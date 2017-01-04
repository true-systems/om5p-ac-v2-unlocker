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

void ar7240_gpio_config()
{
    /* Disable clock obs */
    ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) & 0xffe7e0ff));
    /* Enable eth Switch LEDs */
    ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) | 0xf8));
}

int
ar7240_mem_config(void)
{
    ar7240_ddr_initial_config(CFG_DDR_REFRESH_VAL);

    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x8);
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0x9);

    ar7240_usb_initial_config();
    ar7240_gpio_config();

    return (ar7240_ddr_find_size());
}

long int initdram(int board_type)
{
    return (ar7240_mem_config());
}

#ifdef COMPRESSED_UBOOT
int checkboard (char *board_string)
{
    strcpy(board_string, "AP91 Router (ar7240) U-boot");
    return 0;
}
#else
int checkboard (void)
{
    printf("AP91 Router (ar7240) U-boot\n");
    return 0;
}
#endif /* #ifdef COMPRESSED_UBOOT */
