#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include "ar7240_soc.h"

extern void ar7240_ddr_initial_config(uint32_t refresh);
extern int ar7240_ddr_find_size(void);

void
ar7240_usb_initial_config(void)
{
    ar7240_reg_wr_nf(AR7240_USB_PLL_CONFIG, 0x0a04081e);
    ar7240_reg_wr_nf(AR7240_USB_PLL_CONFIG, 0x0804081e);
}

void ar7240_gpio_config(void)
{
    /* Disable clock obs */
    ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) & 0xffe7e0ff));
    /* Enable eth Switch LEDs */
    ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) | 0xf8));
}

int
ar7240_mem_config(void)
{
#ifndef COMPRESSED_UBOOT
    unsigned int tap_val1, tap_val2;
#endif
    ar7240_ddr_initial_config(CFG_DDR_REFRESH_VAL);

/* Default tap values for starting the tap_init*/
#ifndef COMPRESSED_UBOOT
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x8);
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0x9);
#else
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x9);
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0xa);
#endif
    ar7240_gpio_config();

#ifndef COMPRESSED_UBOOT
    ar7240_ddr_tap_init();

    tap_val1 = ar7240_reg_rd(0xb800001c);
    tap_val2 = ar7240_reg_rd(0xb8000020);
    printf("#### TAP VALUE 1 = %x, 2 = %x\n",tap_val1, tap_val2);
#endif

    ar7240_usb_initial_config();

    return (ar7240_ddr_find_size());
}

long int initdram(int board_type)
{
    return (ar7240_mem_config());
}

#ifdef COMPRESSED_UBOOT
int checkboard (char *board_string)
{
    strcpy(board_string, "PB9x (ar7242) u-boot");
    return 0;
}
#else
int checkboard (void)
{
    printf("PB9x (ar7242) U-boot\n");
    return 0;
}
#endif /* #ifdef COMPRESSED_UBOOT */
