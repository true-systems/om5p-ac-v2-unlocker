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

#define set_val_reg(_reg, _mask, _valreg)           \
    li  t7, _reg;                                   \
    lw  t8, 0(t7);                                  \
    li  t9, ~_mask;                                 \
    and t8, t8, t9;                                 \
    or  t8, t8, _valreg;                            \
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

#define pll_set_reg(_mask, _reg)                    \
    set_val_reg(AR7100_CPU_PLL_CONFIG,  _mask, _reg)

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

/* at 16 bytes offset from the beginning of last sector of 8MB flash part */
#define CFG_FLASH_PLL_SETTING_ADDR  0x1fff0010
#define AR7100_SPI_CLOCK (AR7100_SPI_BASE + 4)

/* t0 - t5 will have the return results. a0 clobbered */
get_flash_pll_setting:
    /* Disable SPI remap */
    li      a0, KSEG1ADDR(AR7100_SPI_CLOCK)
    li      t0, 0x43
    sw      t0, 0(a0)

    /* Now read pll setting fm flash */
    li      t0, KSEG1ADDR(CFG_FLASH_PLL_SETTING_ADDR)
    lw      t1, 0(t0)

    /* Enable SPI remap */
    li      a0, KSEG1ADDR(AR7100_SPI_CLOCK)
    li      t0, 0x3
    sw      t0, 0(a0)

    /*
    ** check the range of values
    */

    li      t0, CFG_PLL_200_200_100
    blt     t1, t0, 1f
    nop
    li      t0, CFG_PLL_680_340_170
    ble     t1, t0, 2f
    nop

1:
        /* Value is bad use compiled value */

#if (CFG_PLL_FREQ == CFG_PLL_680_340_170)
    li      t1, CFG_PLL_680_340_170
#elif (CFG_PLL_FREQ == CFG_PLL_400_400_100)
    li      t1, CFG_PLL_400_400_100
#elif (CFG_PLL_FREQ == CFG_PLL_360_360_180)
    li      t1, CFG_PLL_360_360_180
#elif (CFG_PLL_FREQ == CFG_PLL_400_400_200)
    li      t1, CFG_PLL_400_400_200
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_66)
    li      t1, CFG_PLL_266_266_66
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_133)
    li      t1, CFG_PLL_266_266_133
#elif (CFG_PLL_FREQ == CFG_PLL_333_333_166)
    li      t1, CFG_PLL_333_333_166
#elif (CFG_PLL_FREQ == CFG_PLL_300_300_150)
    li      t1, CFG_PLL_300_300_150
#elif (CFG_PLL_FREQ == CFG_PLL_200_200_100)
    li      t1, CFG_PLL_200_200_100
#else
#error: must define CFG_PLL_FREQ
#endif
2:
    li      t2, CFG_PLL_680_340_170
    beq     t1, t2, pll_680_340_170
    nop
    li      t2, CFG_PLL_400_400_100
    beq     t1, t2, pll_400_400_100
    nop
    li      t2, CFG_PLL_360_360_180
    beq     t1, t2, pll_360_360_180
    nop
    li      t2, CFG_PLL_400_400_200
    beq     t1, t2, pll_400_400_200
    nop
    li      t2, CFG_PLL_266_266_66
    beq     t1, t2, pll_266_266_66
    nop
    li      t2, CFG_PLL_266_266_133
    beq     t1, t2, pll_266_266_133
    nop
    li      t2, CFG_PLL_333_333_166
    beq     t1, t2, pll_333_333_166
    nop
    li      t2, CFG_PLL_300_300_150
    beq     t1, t2, pll_300_300_150
    nop
    li      t2, CFG_PLL_200_200_100
    b       pll_200_200_100
    nop

pll_680_340_170:
    li      t0, (0x0  << PLL_CONFIG_CPU_DIV_SHIFT)
    li      t1, (0x1  << PLL_CONFIG_DDR_DIV_SHIFT)
    li      t2, (0x1  << PLL_CONFIG_AHB_DIV_SHIFT)
    li      t3, (0x10 << PLL_CONFIG_PLL_FB_SHIFT)
    li      t4, (0x0  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
    li      t5, (0x1  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    b       got_setting
    nop

pll_400_400_100:
    li      t0, (0x1  << PLL_CONFIG_CPU_DIV_SHIFT)
    li      t1, (0x1  << PLL_CONFIG_DDR_DIV_SHIFT)
    li      t2, (0x1  << PLL_CONFIG_AHB_DIV_SHIFT)
    li      t3, (0x13 << PLL_CONFIG_PLL_FB_SHIFT)
    li      t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
    li      t5, (0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    b       got_setting
    nop

pll_360_360_180:
    li      t0, (0x2  << PLL_CONFIG_CPU_DIV_SHIFT)
    li      t1, (0x2  << PLL_CONFIG_DDR_DIV_SHIFT)
    li      t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    li      t3, (0x1b << PLL_CONFIG_PLL_FB_SHIFT)
    li      t4, (0x5  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
    li      t5, (0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    b       got_setting
    nop     

pll_400_400_200:
    li      t0, (0x1  << PLL_CONFIG_CPU_DIV_SHIFT)
    li      t1, (0x1  << PLL_CONFIG_DDR_DIV_SHIFT)
    li      t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    li      t3, (0x13 << PLL_CONFIG_PLL_FB_SHIFT)
    li      t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
    li      t5, (0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    b       got_setting
    nop

pll_266_266_66:
    li      t0, (0x2  << PLL_CONFIG_CPU_DIV_SHIFT)
    li      t1, (0x2  << PLL_CONFIG_DDR_DIV_SHIFT)
    li      t2, (0x1  << PLL_CONFIG_AHB_DIV_SHIFT)
    li      t3, (0x13 << PLL_CONFIG_PLL_FB_SHIFT)
    li      t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
    li      t5, (0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    b       got_setting
    nop

pll_266_266_133:
    li      t0, (0x2  << PLL_CONFIG_CPU_DIV_SHIFT)
    li      t1, (0x2  << PLL_CONFIG_DDR_DIV_SHIFT)
    li      t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    li      t3, (0x13 << PLL_CONFIG_PLL_FB_SHIFT)
    li      t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
    li      t5, (0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    b       got_setting
    nop

pll_333_333_166:
    li      t0, (0x2  << PLL_CONFIG_CPU_DIV_SHIFT)
    li      t1, (0x2  << PLL_CONFIG_DDR_DIV_SHIFT)
    li      t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    li      t3, (0x18 << PLL_CONFIG_PLL_FB_SHIFT)
    li      t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
    li      t5, (0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    b       got_setting
    nop     

pll_300_300_150:
    li      t0, (0x3  << PLL_CONFIG_CPU_DIV_SHIFT)
    li      t1, (0x3  << PLL_CONFIG_DDR_DIV_SHIFT)
    li      t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    li      t3, (0x1d << PLL_CONFIG_PLL_FB_SHIFT)
    li      t4, (0x0  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
    li      t5, (0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    b       got_setting
    nop     

pll_200_200_100:
    li      t0, (0x3  << PLL_CONFIG_CPU_DIV_SHIFT)
    li      t1, (0x3  << PLL_CONFIG_DDR_DIV_SHIFT)
    li      t2, (0x0  << PLL_CONFIG_AHB_DIV_SHIFT)
    li      t3, (0x13 << PLL_CONFIG_PLL_FB_SHIFT)
    li      t4, (0x3  << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
    li      t5, (0x0  << PLL_CONFIG_PLL_DIVOUT_SHIFT)
    b       got_setting
    nop

got_setting:
    jr ra


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

#if 1
    /* store return address to return to caller function */
    move s0, ra
    /* regs t0-t5 will have the settings */
    la   t9, get_flash_pll_setting
    jalr t9
    nop
    /* restore return address */
    move ra, s0
    pll_set_reg(PLL_CONFIG_CPU_DIV_MASK, t0)
    pll_set_reg(PLL_CONFIG_AHB_DIV_MASK, t2)
    pll_set_reg(PLL_CONFIG_DDR_DIV_MASK, t1)
    pll_set_reg(PLL_CONFIG_PLL_DIVOUT_MASK, t5)

#else
    pll_set(PLL_CONFIG_CPU_DIV_MASK,    PLL_CONFIG_CPU_DIV_VAL)
    pll_set(PLL_CONFIG_AHB_DIV_MASK,    PLL_CONFIG_AHB_DIV_VAL)
    pll_set(PLL_CONFIG_DDR_DIV_MASK,    PLL_CONFIG_DDR_DIV_VAL)
    pll_set(PLL_CONFIG_PLL_DIVOUT_MASK, PLL_CONFIG_PLL_DIVOUT_VAL)
#endif

    pll_set(PLL_CONFIG_SW_UPDATE_MASK, PLL_CONFIG_SW_UPDATE_VAL)

    /*
     * Will cause a reset
     */
    clk_set(CLOCK_CONTROL_RST_SWITCH_MASK, CLOCK_CTRL_SWITCH_VAL)
    clk_set(CLOCK_CONTROL_CLOCK_SWITCH_MASK, 1)
    
initialize_pll:
    pll_set(PLL_CONFIG_SW_UPDATE_MASK, PLL_CONFIG_SW_UPDATE_VAL)
    clk_clr(CLOCK_CONTROL_RST_SWITCH_MASK)
#if 1
    pll_set_reg(PLL_CONFIG_PLL_FB_MASK, t3)
    pll_set_reg(PLL_CONFIG_PLL_LOOP_BW_MASK, t4)
#else
    pll_set(PLL_CONFIG_PLL_FB_MASK, PLL_CONFIG_PLL_FB_VAL)
    pll_set(PLL_CONFIG_PLL_LOOP_BW_MASK, PLL_CONFIG_PLL_LOOP_BW_VAL)
#endif

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

