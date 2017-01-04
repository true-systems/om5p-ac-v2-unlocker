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

/*
 * SPI Read/Write Utilities
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <config.h>
#include <ar7240_soc.h>

#if (CONFIG_COMMANDS & CFG_CMD_DDR)

/*-----------------------------------------------------------------------
 * Definitions
 */

static void prepare_flash(char **buff_ptr,long **val) 
{
	if((long)*buff_ptr % 4)
		*buff_ptr = *buff_ptr + ((long)*buff_ptr % 4);
        memcpy(*buff_ptr,(void *)((long)UBOOT_ENV_SEC_START),CFG_FLASH_SECTOR_SIZE);
	flash_sect_erase(UBOOT_ENV_SEC_START,UBOOT_ENV_SEC_START + (CFG_FLASH_SECTOR_SIZE - 1));
	*val = (long *)(*buff_ptr + CFG_FLASH_SECTOR_SIZE - 0x30);
}

int do_ddr (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char  *buff,*buff_ptr = NULL;
	int ddr_config,ddr_config2,ext_mod;
	long *val;

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */

	/* last sector of uboot 0xbf040000 - 0xbf050000
	 * write MAGIC at 0xbf04ffe0 
	 * write sector at 0xbf04ffe4
	 * write size at   0xbf04ffe8
         */

	if ((flag & CMD_FLAG_REPEAT) == 0)
	{
		if (argc == 2) {
			ddr_config = simple_strtoul(argv[1], NULL, 10);
			ddr_config2 = CFG_DDR_CONFIG2_VAL;
			ext_mod = CFG_DDR_EXT_MODE_VAL;
		}
		else if (argc == 3) {
			ddr_config = simple_strtoul(argv[1], NULL, 10);
			ddr_config2 = simple_strtoul(argv[2], NULL, 10);
			ext_mod = CFG_DDR_EXT_MODE_VAL;
		}
		else if (argc == 4) {
			ddr_config = simple_strtoul(argv[1], NULL, 10);
			ddr_config2 = simple_strtoul(argv[2], NULL, 10);
			ext_mod = simple_strtoul(argv[3], NULL, 10);
		}
		else {
			printf("Invalid number of arguments:%d\n",argc);
			return -1;
		}
	}

	buff = (char *) malloc(CFG_FLASH_SECTOR_SIZE + 4);
	buff_ptr = buff;

	prepare_flash(&buff_ptr,&val);
	*val++ = (long)CFG_DDR_MAGIC;
	*val++ = ddr_config;
	*val++ = ddr_config2;
	*val   = ext_mod;

        flash_write(buff_ptr,(long)(UBOOT_ENV_SEC_START),CFG_FLASH_SECTOR_SIZE);

	printf("Programed values ext_mod:0x%0.8x ",*val--);
	printf(" ddr_config2:0x%0.8x ddr_config:0x%0.8x \n",*val--,*val);
	free(buff);

	return 0;
}

/***************************************************/

U_BOOT_CMD(
	ddr,	6,	1,	do_ddr,
	"fls    - Set to change DDR settings on reboot\n",
	"<value>  - DDR CONFIG\n"
	"<value>  - DDR CONFIG2\n"
	"<value>  - EXT MOD\n"
);

#endif	/* CFG_CMD_SPI */
