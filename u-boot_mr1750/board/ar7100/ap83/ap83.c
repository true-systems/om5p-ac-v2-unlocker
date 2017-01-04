#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include "ar7100_soc.h"

int ap83_board_version;

void
ar7100_usb_initial_config(void)
{
    ar7100_reg_wr_nf(AR7100_USB_PLL_CONFIG, 0x00001030);
}

int
ar7100_mem_config()
{
    uint32_t  ddr_config, ddr_config2;
    int i;
#if 0
    ar7100_ddr_width_t width;
#endif

    ar7100_ddr_initial_config(CFG_DDR_REFRESH_VAL);
#if 0
    ar7100_ddr_tap_config();
#else
    ar7100_reg_wr (AR7100_DDR_TAP_CONTROL0, 0x0);
    ar7100_reg_wr (AR7100_DDR_TAP_CONTROL1, 0x0);
    ar7100_reg_wr (AR7100_DDR_TAP_CONTROL2, 0x0);
    ar7100_reg_wr (AR7100_DDR_TAP_CONTROL3, 0x0);
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

    /* XXX - these don't really belong here!
    *(volatile unsigned int *)0xb8050004 = 0x00001032;
    udelay(100);
*/
#ifndef AR9100
    *(volatile unsigned int *)0xb8050018 = 0x1313;
    udelay(10);
#endif
#if 0
    *(volatile unsigned int *)0xb805001c = 0x00000909;
    udelay(100);

    *(volatile unsigned int *)0xb8050014 = 0x14000044;
    udelay(100);
#endif

    i = *(volatile int *)0xb8050004;
    i = i & (~(1 << 25));
    *(volatile int *)0xb8050004 = i;
    while ((*(volatile int *)0xb8050004) & (1 << 17));

    i = *(volatile int *)0xb8050004;
    i = i & (~(1 << 16));
    *(volatile int *)0xb8050004 = i;
    while ((*(volatile int *)0xb8050004) & (1 << 17));

    i = *(volatile int *)0xb8050004;
    i = i | (0x3f << 19);
    *(volatile int *)0xb8050004 = i;
    udelay(100);

    *(volatile int *)0xb8050014 = 0x13000a44;
/*
    *(volatile int *)0xb8050014 = 0x13000044;
    *(volatile int *)0xb8050014 = 0x13111321;
    *(volatile int *)0xb8050014 = 0x00111321;
    *(volatile int *)0xb8050014 = 0x00001344;
    *(volatile int *)0xb8050014 = 0x14000044;
    *(volatile int *)0xb8050014 = 0x14000f44;
    *(volatile int *)0xb8050014 = 0x00001044;
    *(volatile int *)0xb8050014 = 0x14001044;
    *(volatile int *)0xb8050014 = 0x14001f44;
    *(volatile int *)0xb8050014 = 0x1f001044;
    *(volatile int *)0xb8050014 = 0x1f001f44;
*/

    *(volatile int *)0xb805001c = 0x00000909;
    udelay(100);

    i = *(volatile int *)0xb8050004;
    i = i & (~(0x3b << 19));
    *(volatile int *)0xb8050004 = i;
    udelay(100);

    i = *(volatile int *)0xb8050004;
    i = i | (0x3 << 20);
    *(volatile int *)0xb8050004 = i;
    udelay(100);

    i = *(volatile int *)0xb8050004;
    i = i & (~(0x3 << 20));
    *(volatile int *)0xb8050004 = i;
    udelay(100);

    /* Temp addition - check with Mani */
    *(volatile unsigned int *)0xb8080008 = 0x00000060;
    udelay(100);

    ar7100_usb_initial_config();

    return (ar7100_ddr_find_size());
}

long int initdram(int board_type)
{
    return (ar7100_mem_config());
}

int checkboard (void)
{
#define AP83_BOARD_NUM_ADDR ((char *)0xbf7f1244)

    ap83_board_version = (AP83_BOARD_NUM_ADDR[0] - '0') +
                    ((AP83_BOARD_NUM_ADDR[1] - '0') * 10);

    printf("AP83-%d (ar9100) U-boot " ATH_AP83_UBOOT_VERSION "\n",
           ap83_board_version);

    return 0;
}
