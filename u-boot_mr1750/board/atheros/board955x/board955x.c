#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include <atheros.h>

extern int ath_ddr_initial_config(uint32_t refresh);
extern int ath_ddr_find_size(void);

static ulong last_wdt_ticks = 0;

#ifdef COMPRESSED_UBOOT
#	define prmsg(...)
#	define args		char *s
#	define board_str(a)	do {			\
	char ver[] = "0";				\
	strcpy(s, a " - Scorpion 1.");			\
	ver[0] += ath_reg_rd(RST_REVISION_ID_ADDRESS)	\
						& 0xf;	\
	strcat(s, ver);					\
} while (0)
#else
#	define prmsg	printf
#	define args		void
#	define board_str(a)				\
	printf(a " - Scorpion 1.%d", ath_reg_rd		\
			(RST_REVISION_ID_ADDRESS) & 0xf)
#endif

void
ath_usb1_initial_config(void)
{
#define unset(a)	(~(a))

	ath_reg_wr_nf(SWITCH_CLOCK_SPARE_ADDRESS,
		ath_reg_rd(SWITCH_CLOCK_SPARE_ADDRESS) |
		SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_SET(5));
	udelay(1000);

	ath_reg_rmw_set(RST_RESET_ADDRESS,
				RST_RESET_USB_PHY_SUSPEND_OVERRIDE_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_PHY_RESET_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_PHY_ARESET_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_HOST_RESET_SET(1));
	udelay(1000);

	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_PHY_PLL_PWD_EXT_SET(1));
	udelay(10);

	ath_reg_rmw_set(RST_RESET2_ADDRESS, RST_RESET2_USB1_EXT_PWR_SEQ_SET(1));
	udelay(10);
}

void
ath_usb2_initial_config(void)
{
	if (is_drqfn()) {
		return;
	}

	ath_reg_rmw_set(RST_RESET2_ADDRESS, RST_RESET2_USB2_MODE_SET(1));
	udelay(10);
	ath_reg_rmw_set(RST_RESET2_ADDRESS,
				RST_RESET2_USB_PHY2_SUSPEND_OVERRIDE_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET2_ADDRESS, RST_RESET2_USB_PHY2_RESET_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET2_ADDRESS, RST_RESET2_USB_PHY2_ARESET_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET2_ADDRESS, RST_RESET2_USB_HOST2_RESET_SET(1));
	udelay(1000);

	ath_reg_rmw_clear(RST_RESET2_ADDRESS, RST_RESET2_USB_PHY2_PLL_PWD_EXT_SET(1));
	udelay(10);

	ath_reg_rmw_set(RST_RESET2_ADDRESS, RST_RESET2_USB2_EXT_PWR_SEQ_SET(1));
	udelay(10);
}

void ath_gpio_config(void)
{
	/* disable the CLK_OBS on GPIO_4 and set GPIO4 as input */
	ath_reg_rmw_clear(GPIO_OE_ADDRESS, (1 << 4));
	ath_reg_rmw_clear(GPIO_OUT_FUNCTION1_ADDRESS, GPIO_OUT_FUNCTION1_ENABLE_GPIO_4_MASK);
	ath_reg_rmw_set(GPIO_OUT_FUNCTION1_ADDRESS, GPIO_OUT_FUNCTION1_ENABLE_GPIO_4_SET(0x80));
	ath_reg_rmw_set(GPIO_OE_ADDRESS, (1 << 4));
	/* Set GPIO 13 as input for LED functionality to be OFF during bootup */
//	ath_reg_rmw_set(GPIO_OE_ADDRESS, (1 << 13));
	/* Turn off JUMPST_LED and 5Gz LED during bootup */

	ath_reg_rmw_set(GPIO_OE_ADDRESS, (1 << 17));	//	Reset button(SWRST)

	ath_reg_rmw_set(0x18040008, (1 << 13));
	ath_reg_rmw_set(0x18040008, (1 << 19));
	ath_reg_rmw_set(0x18040008, (1 << 23));
#ifdef CONFIG_GPIO_CUSTOM
	ath_reg_rmw_clear(0x18040034, 0xff000000);
	ath_reg_rmw_clear(0x18040038, 0x0000ffff);
	ath_reg_rmw_clear(0x1804003c, 0xff00ffff);
	ath_reg_rmw_clear(0x18040040, 0xffffffff);
	ath_reg_rmw_clear(GPIO_OE_ADDRESS, 0xfb3800);
	ath_reg_rmw_clear(GPIO_OUT_ADDRESS, 0x400000);
	ath_reg_rmw_set(GPIO_OUT_ADDRESS, 0xbb3800);
#endif
}

int
ath_mem_config(void)
{
	unsigned int type, reg32, *tap;
	extern uint32_t *ath_ddr_tap_cal(void);

#if !defined(CONFIG_SKIP_LOWLEVEL_INIT)
#if !defined(CONFIG_ATH_EMULATION)

#if !defined(CONFIG_ATH_NAND_BR)
	type = ath_ddr_initial_config(CFG_DDR_REFRESH_VAL);

	tap = ath_ddr_tap_cal();
	prmsg("tap = 0x%p\n", tap);

	tap = (uint32_t *)0xbd007f10;
	prmsg("Tap (low, high) = (0x%x, 0x%x)\n", tap[0], tap[1]);

	tap = (uint32_t *)TAP_CONTROL_0_ADDRESS;
	prmsg("Tap values = (0x%x, 0x%x, 0x%x, 0x%x)\n",
		tap[0], tap[2], tap[2], tap[3]);

	/* Take WMAC out of reset */
	reg32 = ath_reg_rd(RST_RESET_ADDRESS);
	reg32 = reg32 & ~RST_RESET_RTC_RESET_SET(1);
	ath_reg_wr_nf(RST_RESET_ADDRESS, reg32);
#endif

	ath_usb1_initial_config();
	ath_usb2_initial_config();

	ath_gpio_config();
#endif /* !defined(CONFIG_ATH_EMULATION) */
#endif /* !defined(CONFIG_SKIP_LOWLEVEL_INIT) */

#if defined(CONFIG_OM5PACV2_UNLOCKER)
	/* Setup GPIO12 for watchdog */
	ath_reg_rmw_clear(0x18040008, 0x1000);
	ath_reg_rmw_clear(0x18040038, 0x00ff);
	ath_reg_rmw_clear(0x18040000, 0x1000);

	/*
	 * Setup GPIOs for LEDs:
	 *   RED: GPIO23 (active low)
	 * GREEN: GPIO13 (active low)
	 */
	ath_reg_rmw_set(0x18040008, 0x802000);
	ath_reg_rmw_clear(0x18040038, 0x0000ff00);
	ath_reg_rmw_clear(0x18040040, 0xff000000);
	ath_reg_rmw_clear(0x18040000, 0x802000);
#endif /* defined(CONFIG_OM5PACV2_UNLOCKER) */

	return ath_ddr_find_size();
}

long int initdram(int board_type)
{
	return (ath_mem_config());
}

int	checkboard(args)
{
	board_str("MR1750v2 (ar955x) U-boot\n");

	return 0;
}

void hw_watchdog_reset (void)
{
#if defined(CONFIG_OM5PACV2_UNLOCKER)
	#define _HW_WDT_MASK	0x1000
#else
	#define _HW_WDT_MASK	0x10000
#endif /* defined(CONFIG_OM5PACV2_UNLOCKER) */
	ulong ticks = get_timer(last_wdt_ticks);

	/* Don't feed wdt often than every 100 ms */
	if (ticks < (CFG_HZ / 10))
		return;

	last_wdt_ticks = get_timer(0);

	if ((ath_reg_rd (GPIO_OUT_ADDRESS) & _HW_WDT_MASK) == 0) {
		ath_reg_rmw_set (GPIO_OUT_ADDRESS, _HW_WDT_MASK);
	} else {
		ath_reg_rmw_clear (GPIO_OUT_ADDRESS, _HW_WDT_MASK);
	}
}

#ifdef CONFIG_GPIO_CUSTOM
int gpio_custom (int opcode)
{
	int rcode = 0;
	static int cmd = -1;

	switch (opcode) {
		case 0:
			if ((ath_reg_rd (GPIO_IN_ADDRESS) & 0x20000) == 0) {
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
					int led[] = {19, 21};
					int i = -1, j = 0;
					if ((s = getenv (key)) != NULL && (val = malloc (strlen (s) + 1)) != NULL) {
						strcpy (val, s);
					}
					setenv (key, "no");
					s = "boot 0";
					for (i = -1; ++i < 48; udelay (82500)) {
						if ((j = i % 4) < 2) {
							ath_reg_rmw_set (GPIO_OUT_ADDRESS, 1 << led[j]);
						}
						else {
							ath_reg_rmw_clear (GPIO_OUT_ADDRESS, 1 << led[3 - j]);
						}
					}
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
