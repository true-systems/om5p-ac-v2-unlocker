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
#ifdef CONFIG_GPIO_CUSTOM
#endif
}

int
ar7240_mem_config(void)
{
    unsigned int tap_val1, tap_val2;
    ar7240_ddr_initial_config(CFG_DDR_REFRESH_VAL);

    /* Default tap values for starting the tap_init*/
    if (!(is_ar7241() || is_ar7242()))  {
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x8);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0x9);
#ifndef COMPRESSED_UBOOT
        ar7240_ddr_tap_init();
#endif
    }
    else {
	ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, 0x2);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, 0x2);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL2, 0x0);
        ar7240_reg_wr (AR7240_DDR_TAP_CONTROL3, 0x0); 
    }

    tap_val1 = ar7240_reg_rd(0xb800001c);
    tap_val2 = ar7240_reg_rd(0xb8000020);

#ifndef COMPRESSED_UBOOT
    printf("#### TAP VALUE 1 = 0x%x, 2 = 0x%x [0x%x: 0x%x]\n",
                tap_val1, tap_val2, *(unsigned *)0x80500000,
                *(unsigned *)0x80500004);
#endif
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
    if ((is_ar7241() || is_ar7242()))
        strcpy(board_string, BOARD_NAME" (ar7241 - Virian) U-boot\n");
    else
        strcpy(board_string, BOARD_NAME" (ar7240 - Python) U-boot\n");
    return 0;
}
#else
int checkboard (void)
{
    if ((is_ar7241() || is_ar7242()))
        printf(BOARD_NAME" (ar7241 - Virian) U-boot\n");
    else
        printf(BOARD_NAME" (ar7240 - Python) U-boot\n");

    return 0;
}
#endif

#ifdef CONFIG_GPIO_CUSTOM
int gpio_custom (int opcode)
{
    int rcode = 0;
    static int cmd = -1;

    switch (opcode) {
        case 0:
            if (0) {
                cmd = 0;
            }
            else {
                cmd = -1;
            }
            rcode = cmd >= 0;
            break;
        case 1:
            switch (cmd) {
                case 0:
                {
                    char key[] = "netretry", *val = NULL, *s = NULL;
                    int i = -1;

                    if ((s = getenv (key)) != NULL && (val = malloc (strlen (s) + 1)) != NULL) {
                        strcpy (val, s);
                    }
                    setenv (key, "no");
                    s = getenv ("factory_boot") != NULL? "run factory_boot": "boot 0";
                    for (i = 3; i-- > 0 && run_command (s, 0) == -1;) {
                        printf ("Retry%s\n", i > 0? "...": " count exceeded!");
                        if (i <= 0) {
                            run_command ("boot", 0);
                        }
                    }
                    if (val != NULL) {
                        setenv (key, val);
                    }
                    break;
                }
                default:
                    break;
            }
            cmd = -1;
            break;
        default:
            break;
    }

    return rcode;
}
#endif
