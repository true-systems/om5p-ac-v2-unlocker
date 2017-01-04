/*
 * This file contains the configuration parameters for the dbau1x00 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ar7100.h>

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS     1	    /* max number of memory banks */
#define CFG_MAX_FLASH_SECT      128    /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE          0x00800000    /* Flash size */

/* 
 * We boot from this flash
 */
#define CFG_FLASH_BASE		    0xbf000000

/* 
 * The following #defines are needed to get flash environment right 
 */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#undef CFG_HZ

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
#endif


/* 
 * timeout values are in ticks 
 */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET	0x1000

#define	CFG_ENV_IS_IN_FLASH    1
#undef CFG_ENV_IS_NOWHERE  

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_ADDR		0xbf040000
#define CFG_ENV_SIZE		0x10000

//#define CONFIG_FLASH_16BIT

#define CONFIG_NR_DRAM_BANKS	2

#define CFG_DDR_REFRESH_VAL     0x461b
#define CFG_DDR_TRTW_VAL        0x1f
#define CFG_DDR_TWTR_VAL        0x1e


#define CONFIG_NET_MULTI

#define CONFIG_MEMSIZE_IN_BYTES


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_COMMANDS	(( CONFIG_CMD_DFL | CFG_CMD_DHCP | CFG_CMD_ELF | \
            CFG_CMD_MII | CFG_CMD_PING | CFG_CMD_NET |\
   CFG_CMD_ENV | CFG_CMD_FLASH | CFG_CMD_LOADS | CFG_CMD_RUN | CFG_CMD_LOADB \
   | CFG_CMD_ELF ))


#define CONFIG_IPADDR   192.168.1.10
#define CONFIG_SERVERIP 192.168.1.11
#define CONFIG_ETHADDR 0x00:0xaa:0xbb:0xcc:0xdd:0xee
#define CFG_FAULT_ECHO_LINK_DOWN    1

#define CONFIG_ADMTEK_PHY 
#define CFG_PHY_ADDR 0x14  /* Port 4 */

#define CFG_GMII     0
#define CFG_MII0_MII             0

#define DEBUG
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "hush>"

#include <cmd_confdefs.h>

/* Ported from
 * redboot/ecos/packages/hal/mips/pb42/current/cdl/hal_mips_pb42.cdl
 */

#define CFG_DDR_CONFIG_VAL			0xefbc8cd0
#define CFG_DDR_CONFIG2_VAL			0x8e7156a2
#define CFG_DDR_MODE_VAL			      0x61
#define CFG_DDR_EXT_MODE_VAL			  0x0
#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL		    0xffff

#endif	/* __CONFIG_H */
