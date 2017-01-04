/*
 * Copyright (c) 2009, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */
/*
 * This file contains the configuration parameters for the dbau1x00 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ar7100.h>

#define CFG_BOARD_PB44 1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#ifndef COMPRESSED_UBOOT
#define CFG_MAX_FLASH_BANKS     1           /* max number of memory banks */
#define CFG_MAX_FLASH_SECT      128         /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE          0x00800000  /* Flash size */
#else
/* For 2 MB flash */
#define CFG_MAX_FLASH_BANKS     1           /* max number of memory banks */
#define CFG_MAX_FLASH_SECT      128         /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE          0x00800000  /* 0x00200000 Total flash size */
#endif  /* #ifndef COMPRESSED_UBOOT */

/* 
 * We boot from this flash
 */
#ifndef COMPRESSED_UBOOT
#define CFG_FLASH_BASE          0xbf000000
#else
#define CFG_FLASH_BASE          0xbf000000
#endif  /* #ifndef COMPRESSED_UBOOT */
#ifdef COMPRESSED_UBOOT
#define BOOTSTRAP_TEXT_BASE         CFG_FLASH_BASE
#define BOOTSTRAP_CFG_MONITOR_BASE  BOOTSTRAP_TEXT_BASE
#endif  /* #ifdef COMPRESSED_UBOOT */


#undef CONFIG_BOOTARGS_FL
#undef CONFIG_BOOTARGS_RD
#undef CONFIG_BOOTARGS
#undef  MTDPARTS_DEFAULT
#undef  MTDIDS_DEFAULT

#define CONFIG_ROOTFS_FLASH
#undef CONFIG_BOOTARGS

/* XXX - putting rootfs in last partition results in jffs errors */

#ifdef COMPRESSED_UBOOT
#define CONFIG_BOOTARGS     "console=ttyS0,115200 root=31:01 rootfstype=jffs2 init=/sbin/init mtdparts=ar7100-nor0:64k(u-boot),6144k(rootfs),1728k(uImage),64k(ART)"
#else
#define CONFIG_BOOTARGS     "console=ttyS0,115200 root=31:01 rootfstype=jffs2 init=/sbin/init mtdparts=ar7100-nor0:64k(u-boot),6144k(rootfs),1728k(uImage),64k(ART)"
#endif /* #ifdef COMPRESSED_UBOOT */

#define CONFIG_EXTRA_ENV_SETTINGS \
"loadUboot=tftp 0x80060000 tuboot.bin; erase 0xbf000000 +$filesize; cp.b $fileaddr 0xbf000000 $filesize\0" \
"loadLinux=tftpboot 0x80010000 vmlinux.lzma.uImage;erase 0xbf640000 +$filesize; cp.b $fileaddr 0xbf640000 $filesize\0" \
"loadFiles=tftpboot 0x80010000 pb44-small-jffs2;erase 0xbf040000 +$filesize; cp.b $fileaddr 0xbf040000 $filesize\0" \
"loadAll=run loadUboot;run loadLinux;run loadFiles\0"

/* 
 * The following #defines are needed to get flash environment right 
 */
#define CFG_MONITOR_BASE    TEXT_BASE
#define CFG_MONITOR_LEN     (192 << 10)

#undef CFG_HZ
#define CFG_PLL_FREQ    CFG_PLL_680_340_170

/*
 * MIPS32 24K Processor Core Family Software User's Manual
 *
 * 6.2.9 Count Register (CP0 Register 9, Select 0)
 * The Count register acts as a timer, incrementing at a constant
 * rate, whether or not an instruction is executed, retired, or
 * any forward progress is made through the pipeline.  The counter
 * increments every other clock, if the DC bit in the Cause register
 * is 0.
 */
/* Since the count is incremented every other tick, divide by 2 */
/* XXX derive this from CFG_PLL_FREQ */
#if (CFG_PLL_FREQ == CFG_PLL_200_200_100)
#	define CFG_HZ          200000000
#elif (CFG_PLL_FREQ == CFG_PLL_300_300_150)
#	define CFG_HZ          200000000
#elif (CFG_PLL_FREQ == CFG_PLL_333_333_166)
#	define CFG_HZ          222000000
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_133)
#	define CFG_HZ          266000000
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_66)
#	define CFG_HZ          266000000
#elif (CFG_PLL_FREQ == CFG_PLL_400_400_200)
#	define CFG_HZ          400000000
#elif (CFG_PLL_FREQ == CFG_PLL_680_340_170)
#   define CFG_HZ  (680000000/2)
#endif


/* 
 * timeout values are in ticks 
 */
#define CFG_FLASH_ERASE_TOUT    (2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT    (2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET  0x1000

#ifndef COMPRESSED_UBOOT
#define CFG_ENV_IS_IN_FLASH    1
#undef CFG_ENV_IS_NOWHERE  
#else
#undef  CFG_ENV_IS_IN_FLASH
#define CFG_ENV_IS_NOWHERE  1
#endif /* #ifndef COMPRESSED_UBOOT */


/* Address and size of Primary Environment Sector   */
#define CFG_ENV_ADDR		0xbf040000
#define CFG_ENV_SIZE        0x10000

#ifndef COMPRESSED_UBOOT
#define CONFIG_BOOTCOMMAND "bootm 0xbf640000"
#else
#define CONFIG_BOOTCOMMAND "bootm 0xbf640000"
#endif /* #ifndef COMPRESSED_UBOOT */


#define CONFIG_NR_DRAM_BANKS    		2

#define ATHEROS_PRODUCT_ID	136
/* The calibration sector is used to store MAC addresses and CPU PLL setting */
#define BOARDCAL                        0xbfff0000
#define CAL_SECTOR                      127

/*
** Configure the CPU clock settings
*/

#define CFG_PLL_FREQ	CFG_PLL_680_340_170
/*
** These values for DDR configuration came from the
** BDM configuration script
*/

#define CFG_DDR_REFRESH_VAL     		0x461b
#define CFG_DDR_CONFIG_VAL      		0x6fb8884e
#define CFG_DDR_CONFIG2_VAL            	0x812cd6a8
#define CFG_DDR_MODE_VAL_INIT   		0x131
#define CFG_DDR_EXT_MODE_VAL    		0x0
#define CFG_DDR_MODE_VAL        		0x63
#define CFG_DDR_TRTW_VAL        		0x1f
#define CFG_DDR_TWTR_VAL        		0x1e
#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL  0xff


#define CONFIG_NET_MULTI
#define CONFIG_MEMSIZE_IN_BYTES

#ifndef COMPRESSED_UBOOT
#define CONFIG_PCI
#endif /* #ifndef COMPRESSED_UBOOT */


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#ifndef COMPRESSED_UBOOT
#define CONFIG_COMMANDS (( CONFIG_CMD_DFL        | CFG_CMD_MII   | CFG_CMD_PING  \
   | CFG_CMD_NET | CFG_CMD_PCI     | CFG_CMD_ENV | CFG_CMD_FLASH | CFG_CMD_LOADS \
   | CFG_CMD_RUN | CFG_CMD_LOADB   | CFG_CMD_ELF | CFG_CMD_BSP ))
#else
#define CONFIG_COMMANDS (( CONFIG_CMD_DFL | CFG_CMD_PING | CFG_CMD_NET))
#endif /* #ifndef COMPRESSED_UBOOT */


/*
** Ethernet Configuration
** Set default values, and select the PHY to use
*/

#define CONFIG_IPADDR       		192.168.1.2
#define CONFIG_SERVERIP     		192.168.1.200
//#define CONFIG_ETHADDR      		00:03:7f:ff:ff:fe

#define CFG_FAULT_ECHO_LINK_DOWN    1

#define CONFIG_PHY_GIGE       		1              /* GbE speed/duplex detect */

#define CFG_VITESSE_8601_7395_PHY   1
//#define CFG_ATHRF1_PHY              1
#define CFG_AG7100_NMACS 			2

#define CFG_PHY_ADDR				0 
#define CFG_AG7100_GE0_RGMII 1 /* XXX: on pb44, GE0 can be RGMII or GMII depending magpie's interface */
#define CFG_AG7100_GE1_RGMII 1

/*
** Configure Parser
*/

#define CFG_BOOTM_LEN   (16 << 20) /* 16 MB */
#define DEBUG
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "hush>"

#include <cmd_confdefs.h>

#endif  /* __CONFIG_H */
