#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include "ar7100_soc.h"

int
ar7100_mem_config()
{
    uint32_t  ddr_config, ddr_config2;
    ar7100_ddr_width_t width;

    ar7100_ddr_initial_config(CFG_DDR_REFRESH_VAL);
#if 0
    ar7100_ddr_tap_config();
#else
    ar7100_reg_wr (AR7100_DDR_TAP_CONTROL0, 0x7);
    ar7100_reg_wr (AR7100_DDR_TAP_CONTROL1, 0x7);
    ar7100_reg_wr (AR7100_DDR_TAP_CONTROL2, 0x7);
    ar7100_reg_wr (AR7100_DDR_TAP_CONTROL3, 0x7);
#endif
#if 0
    ddr_config   = ar7100_reg_rd(AR7100_DDR_CONFIG);
    ddr_config2  = ar7100_reg_rd(AR7100_DDR_CONFIG2);
    width        = ar7100_ddr_get_width();

    if (width != AR7100_DDR_32B)
        ddr_config |= AR7100_DDR_CONFIG_16BIT;
    if (width == AR7100_DDR_16B_HIGH)
        ddr_config2 &= ~AR7100_DDR_CONFIG2_HALF_WIDTH_L;

    ddr_config2 &= ~((0x1f << AR7100_DDR_CONFIG2_TRTW_SHIFT) |
                     (0x1f << AR7100_DDR_CONFIG2_TWTR_SHIFT) |
                      0xf);

    ddr_config2 |= ((CFG_DDR_TRTW_VAL << AR7100_DDR_CONFIG2_TRTW_SHIFT) |
                    (CFG_DDR_TWTR_VAL << AR7100_DDR_CONFIG2_TWTR_SHIFT) |
                    AR7100_DDR_CONFIG2_BL2);
    /*
     * XXX These bits are reserved...
     */
    ddr_config2 |= (1 << 26)|(1 << 27)|(1 << 30);

    printf("programming config1 %#x, config2 %#x\n", ddr_config, ddr_config2);

    ar7100_reg_wr(AR7100_DDR_CONFIG, ddr_config);
    ar7100_reg_wr(AR7100_DDR_CONFIG2, ddr_config2);
#endif
    /* XXX - these don't really belong here! */
    *(volatile unsigned int *)0xb8050004 = 0x50C0;
    udelay(10);
    *(volatile unsigned int *)0xb8050018 = 0x1313;
    udelay(10);
    *(volatile unsigned int *)0xb805001c = 0xee;
    udelay(10);
    *(volatile unsigned int *)0xb8050010 = 0x1099;
    udelay(10);

    return (ar7100_ddr_find_size());
}

long int initdram(int board_type)
{
    return (ar7100_mem_config());
}

int checkboard (void)
{

    printf("PB42 (ar7100) U-boot\n");
	return 0;
}
