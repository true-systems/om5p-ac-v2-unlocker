/*
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <environment.h>
#include <command.h>
#include <config.h>
#ifndef CONFIG_ATHEROS
#include <ar7240_soc.h>
#endif

#ifndef CONFIG_ATH_EMULATION
#if (CONFIG_COMMANDS & CFG_CMD_PLL)
#ifdef CONFIG_MACH_QCA955x
int do_pll (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern env_t	*env_ptr;
	extern int saveenv(void);
	extern void env_crc_update (void);

	unsigned	*val;

	if (argc != 2 && argc != 5) {
		printf(cmdtp->usage);
		return -1;
	}

	val = (unsigned *)env_ptr->data +
		(sizeof(env_ptr->data) / sizeof(*val)) - (32 / sizeof(*val));

	if (strcmp(argv[1], "erase") == 0) {
		printf("Erasing flash setting\n");
		val[0] = val[1] =
		val[2] = val[3] =
		val[4] = val[5] = 0xffffffffu;
	} else if (strcmp(argv[1], "get") == 0) {
		printf("pll 0x%x 0x%x 0x%x 0x%x\n", val[1], val[2], val[3], val[4]);
		return 0;
	} else if (argc > 2) {
		val[0] = PLL_MAGIC;

		val[1] = simple_strtoul(argv[1], NULL, 16);
		val[2] = simple_strtoul(argv[2], NULL, 16);
		val[3] = simple_strtoul(argv[3], NULL, 16);
		val[4] = simple_strtoul(argv[4], NULL, 16);
		printf("Setting 0x%x 0x%x 0x%x 0x%x\n", val[1], val[2], val[3], val[4]);
	} else {
		printf(cmdtp->usage);
		return -1;
	}

	env_crc_update();
	saveenv();

	return 0;

}

U_BOOT_CMD(
	pll,	5,	0,	do_pll,
#ifdef COMPRESSED_UBOOT
	NULL, NULL
#else
	"pll cpu-pll dither ddr-pll dither - Set to change CPU & DDR speed\npll erase\npll get\n", NULL
#endif
);

#elif defined(CONFIG_WASP_SUPPORT)
#include <asm/addrspace.h>

int do_srifpll (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern env_t	*env_ptr;
	extern int saveenv(void);
	extern void env_crc_update (void);

	unsigned	*val;

	if (argc != 2 && argc != 3) {
		printf(cmdtp->usage);
		return -1;
	}

	val = env_ptr->data + sizeof(env_ptr->data) - 0x20 - 0xc;

	if (strcmp(argv[1], "erase") == 0) {
		printf("Erasing flash setting\n");
		val[0] = val[1] = val[2] = 0xffffffffu;
	} else if (strcmp(argv[1], "get") == 0) {
		printf("srifpll 0x%x 0x%x\n", val[1], val[2]);
		return 0;
	} else if (argc > 2) {
		val[0] = SRIF_PLL_MAGIC;

		val[1] = simple_strtoul(argv[1], NULL, 16);
		val[2] = simple_strtoul(argv[2], NULL, 16);
		printf("Setting 0x%x 0x%x\n", val[1], val[2]);
	} else {
		printf(cmdtp->usage);
		return -1;
	}

	env_crc_update();
	saveenv();

	return 0;

}

int do_pll (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern env_t	*env_ptr;
	extern int saveenv(void);
	extern void env_crc_update (void);

	unsigned	*val;

	if (argc != 2 && argc != 6) {
		printf(cmdtp->usage);
		return -1;
	}

	val = (unsigned *)env_ptr->data +
		(sizeof(env_ptr->data) / sizeof(*val)) - (32 / sizeof(*val));

	if (strcmp(argv[1], "erase") == 0) {
		printf("Erasing flash setting\n");
		val[0] = val[1] =
		val[2] = val[3] =
		val[4] = val[5] = 0xffffffffu;
	} else if (strcmp(argv[1], "get") == 0) {
		printf("pll 0x%x 0x%x 0x%x 0x%x 0x%x\n", val[1], val[2], val[3], val[4], val[5]);
		return 0;
	} else if (argc > 2) {
		val[0] = PLL_MAGIC;

		val[1] = simple_strtoul(argv[1], NULL, 16);
		val[2] = simple_strtoul(argv[2], NULL, 16);
		val[3] = simple_strtoul(argv[3], NULL, 16);
		val[4] = simple_strtoul(argv[4], NULL, 16);
		val[5] = simple_strtoul(argv[5], NULL, 16);
		printf("Setting 0x%x 0x%x 0x%x 0x%x 0x%x\n", val[1], val[2], val[3], val[4], val[5]);
	} else {
		printf(cmdtp->usage);
		return -1;
	}

	env_crc_update();
	saveenv();

	return 0;

}

U_BOOT_CMD(
	pll,	6,	0,	do_pll,
#ifdef COMPRESSED_UBOOT
	NULL, NULL
#else
	"pll cpu-pll dither ddr-pll dither - Set to change CPU & DDR speed\npll erase\npll get\n", NULL
#endif
);

U_BOOT_CMD(
	srifpll,	3,	0,	do_srifpll,
#ifdef COMPRESSED_UBOOT
	NULL, NULL
#else
	"srifpll cpu-pll ddr-pll - To change CPU & DDR speed through srif\nsrifpll erase\nsrifpll get\n", NULL
#endif
);
#else	/* CONFIG_WASP_SUPPORT */
typedef struct {
	char		*freq;
	unsigned	ddr,
			ahb,
			pll,
			ref;
} plldef_t;

static plldef_t plldef[] = {
	{ "400_400_200", 0x0u, 0x0u, 0x28u, 0x2u },
	{ "400_400_100", 0x0u, 0x1u, 0x28u, 0x2u },
	{ "360_360_180", 0x0u, 0x0u, 0x24u, 0x2u },
	{ "350_350_175", 0x0u, 0x0u, 0x23u, 0x2u },
	{ "340_340_170", 0x0u, 0x0u, 0x22u, 0x2u },
	{ "320_320_160", 0x0u, 0x0u, 0x20u, 0x2u },
	{ "320_320_80",  0x0u, 0x1u, 0x20u, 0x2u },
	{ "300_300_150", 0x0u, 0x0u, 0x1eu, 0x2u },
	{ "300_300_75",  0x0u, 0x1u, 0x1eu, 0x2u },
	{ "200_200_100", 0x0u, 0x0u, 0x14u, 0x2u },
	{ "370_370_185", 0x0u, 0x0u, 0x25u, 0x2u },
	{ "380_380_190", 0x0u, 0x0u, 0x26u, 0x2u },
	{ "390_390_195", 0x0u, 0x0u, 0x27u, 0x2u },
	{ "410_410_205", 0x0u, 0x0u, 0x29u, 0x2u },
	{ "420_420_210", 0x0u, 0x0u, 0x2au, 0x2u },
	{ "430_430_215", 0x0u, 0x0u, 0x2bu, 0x2u },
	{ NULL, 0x0u, 0x0u, 0x0u, 0x0u }	/* Terminator */
};

int do_pll (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	extern env_t	*env_ptr;
	extern int saveenv(void);
	extern void env_crc_update (void);

	plldef_t	*p;
	unsigned	*val;

#ifndef COMPRESSED_UBOOT
	if (argc == 1) {
		/* list known frequencies */
		printf("List of known frequencies:\n");
		for (p = plldef; p->freq; p ++) {
			printf("\t%s\n", p->freq);
		}
		return 0;
	}
#endif

	if (argc != 2) {
		printf(cmdtp->usage);
		return -1;
	}

	val = (unsigned *)env_ptr->data;

	/*
	 * We use the last 16 bytes of envmt data.
	 * Position the pointer appropriately.
	 */
	val += (sizeof(env_ptr->data) / sizeof(*val))
		- (16 / sizeof(*val));

	val[0] = PLL_MAGIC;

	/*
	 * XXX XXX XXX WARNING XXX XXX XXX
	 * Doesn't detect arguments like aaa_bbb_ccc that
	 * are not present in the above table. They will
	 * get processed in strtoul, and an incorrect
	 * value will be set, instead of flagging an error
	 *
	 * This does not take care of redefining CFG_HZ
	 */
	for (p = plldef; p->freq; p ++) {
		if (strcmp(argv[1], p->freq) == 0) {
			val[1] = (unsigned)(
					(p->ddr << PLL_CONFIG_DDR_DIV_SHIFT) |
					(p->ahb << PLL_CONFIG_AHB_DIV_SHIFT) |
					(p->pll << PLL_CONFIG_PLL_DIV_SHIFT) |
					(p->ref << PLL_CONFIG_PLL_REF_DIV_SHIFT));
			break;
		}
	}
	if (!p->freq) {
		if (strcmp(argv[1], "erase") == 0) {
			val[0] = 0xffffffffu;
			val[1] = 0xffffffffu;
		} else {
			val[1] = simple_strtoul(argv[1], NULL, 10);
		}
	}

#ifndef COMPRESSED_UBOOT
	printf("magic: 0x%08x	value: 0x%08x\n", val[0], val[1]);
#endif

	env_crc_update();
	saveenv();

	return 0;
}

/***************************************************/

U_BOOT_CMD(
	pll,	2,	0,	do_pll,
#ifdef COMPRESSED_UBOOT
	NULL, NULL
#else
	"pll [<val>] - Set to change CPU/AHB/DDR speeds\n",
	"<val> - 300_300_150\n"
#endif
);
#endif /* CONFIG_WASP_SUPPORT */

#endif	/* CFG_CMD_SPI */
#endif // CONFIG_ATH_EMULATION
