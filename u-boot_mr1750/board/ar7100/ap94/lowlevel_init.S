/*****************************************************************************/
/*! file lowlevel_init.s
** /brief Low Level initialization routine for AP94
**    
**  
**
**  Copyright (c) 200x Atheros Communications Inc.  All rights reserved.
**
*/

#include <config.h>
#include <version.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <ar7100_soc.h>

/*
 * Helper macros.
 * These Clobber t7, t8 and t9
 */
#define clear_mask(_reg, _mask)                     \
    li  t7, KSEG1ADDR(_reg);                                   \
    lw  t8, 0(t7);                                  \
    li  t9, ~_mask;                                 \
    and t8, t8, t9;                                 \
    sw  t8, 0(t7)            

#define set_val(_reg, _mask, _val)                  \
    li  t7, KSEG1ADDR(_reg);                                   \
    lw  t8, 0(t7);                                  \
    li  t9, ~_mask;                                 \
    and t8, t8, t9;                                 \
    li  t9, _val;                                   \
    or  t8, t8, t9;                                 \
    sw  t8, 0(t7)            

#define get_val(_reg, _mask, _shift, _res_reg)      \
    li  t7, KSEG1ADDR(_reg);                                   \
    lw  t8, 0(t7);                                  \
    li  t9, _mask;                                  \
    and t8, t8, t9;                                 \
    srl _res_reg, t8, _shift                        \

#define pll_clr(_mask)                              \
    clear_mask(AR7100_CPU_PLL_CONFIG, _mask)

#define pll_set(_mask, _val)                        \
    set_val(AR7100_CPU_PLL_CONFIG,  _mask, _val)

#define pll_get(_mask, _shift, _res_reg)            \
    get_val(AR7100_CPU_PLL_CONFIG, _mask, _shift, _res_reg)

#define clk_clr(_mask)                               \
    clear_mask(AR7100_CPU_CLOCK_CONTROL, _mask)

#define clk_set(_mask, _val)                         \
    set_val(AR7100_CPU_CLOCK_CONTROL,  _mask, _val)

#define clk_get(_mask, _shift, _res_reg)              \
    get_val(AR7100_CPU_CLOCK_CONTROL, _mask, _shift, _res_reg)

#define MEMDELAY(count, reg)	\
	li	    reg, count;	\
9:	addi	reg, -1;	\
	bgtz	reg, 9b;	\
	nop


#define PLL_CONFIG_SW_UPDATE_VAL (1 << 31)
#define CLOCK_CTRL_SWITCH_VAL (1 << 1)

/******************************************************************************
 * first level initialization:
 * 
 * 0) If clock cntrl reset switch is already set, we're recovering from 
 *    "divider reset"; goto 3.
 * 1) Setup divide ratios.
 * 2) Reset.
 * 3) Setup pll's, wait for lock.
 * 
 *****************************************************************************/

.globl lowlevel_init

lowlevel_init:

    /*
     * The code below is for the real chip. Wont work on FPGA
     */
    /* jr ra  */

    clk_get(CLOCK_CONTROL_RST_SWITCH_MASK, CLOCK_CONTROL_RST_SWITCH_SHIFT, t6)
    bne zero, t6, initialize_pll
    nop 

    pll_set(PLL_CONFIG_CPU_DIV_MASK,    PLL_CONFIG_CPU_DIV_VAL)
    pll_set(PLL_CONFIG_AHB_DIV_MASK,    PLL_CONFIG_AHB_DIV_VAL)
    pll_set(PLL_CONFIG_DDR_DIV_MASK,    PLL_CONFIG_DDR_DIV_VAL)
	pll_set(PLL_CONFIG_PLL_DIVOUT_MASK, PLL_CONFIG_PLL_DIVOUT_VAL)

    pll_set(PLL_CONFIG_SW_UPDATE_MASK, PLL_CONFIG_SW_UPDATE_VAL)

    /*
     * Will cause a reset
     */
    clk_set(CLOCK_CONTROL_RST_SWITCH_MASK, CLOCK_CTRL_SWITCH_VAL)
    clk_set(CLOCK_CONTROL_CLOCK_SWITCH_MASK, 1)
    
initialize_pll:
    pll_set(PLL_CONFIG_SW_UPDATE_MASK, PLL_CONFIG_SW_UPDATE_VAL)
    clk_clr(CLOCK_CONTROL_RST_SWITCH_MASK)
    pll_set(PLL_CONFIG_PLL_FB_MASK, PLL_CONFIG_PLL_FB_VAL)
    pll_set(PLL_CONFIG_PLL_LOOP_BW_MASK, PLL_CONFIG_PLL_LOOP_BW_VAL)

    pll_clr(PLL_CONFIG_PLL_POWER_DOWN_MASK);
    pll_clr(PLL_CONFIG_PLL_BYPASS_MASK);

wait_for_pll_lock:
    pll_get(PLL_CONFIG_LOCKED_MASK, PLL_CONFIG_LOCKED_SHIFT, t6)
    beq zero, t6, wait_for_pll_lock
    nop 


pll_locked:
    clk_set(CLOCK_CONTROL_CLOCK_SWITCH_MASK, 1)
    
    /*
    ** Now, time to initialize memory.  We'll do the early initialization here
    ** (taken directly from Redboot), and the later initialization will not be
    ** performed.  Better all around.
    **
    ** Since AP-94 is a fixed design, we'll ensure the config value has the correct
    ** CAS latency.
    */
    
    li      a0, KSEG1ADDR(AR7100_DDR_CONFIG)
	li	    t0, CFG_DDR_CONFIG_VAL
	sw	    t0, 0(a0)
	nop
    
    /*
    ** Load all other values
    */

	li      a1, KSEG1ADDR(AR7100_DDR_CONFIG2)
	li	    t8, CFG_DDR_CONFIG2_VAL
	sw	    t8, 0(a1)
    MEMDELAY(30, t2)
    
    /*
    ** Setting the mode register
    */
    
    li      a0, KSEG1ADDR(AR7100_DDR_CONTROL)
    li      a1, KSEG1ADDR(AR7100_DDR_EXT_MODE)

    li      t5, 8
    sw      t5, 0(a0)
    MEMDELAY(30, t2)

    li      t5, 1
    sw      t5, 0(a0)
    MEMDELAY(30, t2)

    li      t5, CFG_DDR_EXT_MODE_VAL
    sw      t5, 0(a1)
    MEMDELAY(30, t2)

    li      t5, 2
    sw      t5, 0(a0)
    MEMDELAY(30, t2)

    li      t5, 8
    sw      t5, 0(a0)
    MEMDELAY(30, t2)

	/*
    **  Want to set the mode value.  since AP-94 only operates at
    ** one speed, we don't need to check the speed setting
    */
    
    li      a2, KSEG1ADDR(AR7100_DDR_MODE)
	li	    t5, CFG_DDR_MODE_VAL
    sw	    t5, 0(a2)
	MEMDELAY(30, t2)

    li      t5, 1
    sw      t5, 0(a0)
    MEMDELAY(30, t2)

    /*
    ** Setting refresh, data this cycle, and tap words
    */
    
    li      a0, KSEG1ADDR(AR7100_DDR_REFRESH)
    li      t5, CFG_DDR_REFRESH_VAL
    sw      t5, 0(a0)
    MEMDELAY(30, t2)

    li      a2, KSEG1ADDR(AR7100_DDR_RD_DATA_THIS_CYCLE)
	li	    t5, CFG_DDR_RD_DATA_THIS_CYCLE_VAL
	sw	    t5, 0(a2)
	
    li      a1, 0x07
    li      a0, KSEG1ADDR(AR7100_DDR_TAP_CONTROL0)
    sw      a1, 0(a0)
    li      a0, KSEG1ADDR(AR7100_DDR_TAP_CONTROL1)
    sw      a1, 0(a0)
    li      a0, KSEG1ADDR(AR7100_DDR_TAP_CONTROL2)
    sw      a1, 0(a0)
    li      a0, KSEG1ADDR(AR7100_DDR_TAP_CONTROL3)
    sw      a1, 0(a0)
    nop

    jr ra

