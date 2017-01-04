/*
 * This file contains the configuration parameters for the dbau1x00 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ar7100.h>

/*
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	     1	     /* max number of memory banks */
#define CFG_MAX_FLASH_SECT	    (128)	 /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
/* 
 * We boot from this flash, selected with dip switch 
 */
#define CFG_FLASH_BASE		    0xbfc00000

/* 
 * The following #defines are needed to get flash environment right 
 */
#define	CFG_MONITOR_BASE	    TEXT_BASE
#define	CFG_MONITOR_LEN		    (192 << 10)

/*
 * cache lock for stack
 */
#define CFG_INIT_SP_OFFSET	    0x1000


/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

//#define	CFG_ENV_IS_IN_FLASH    1
#define CFG_ENV_IS_NOWHERE  1

/* Address and size of Primary Environment Sector	*/
//#define CFG_ENV_ADDR		0xbfc30000
#define CFG_ENV_SIZE		0x10000

//#define CONFIG_FLASH_16BIT

#define CONFIG_NR_DRAM_BANKS	2

#define CONFIG_NET_MULTI

#define CONFIG_MEMSIZE_IN_BYTES

#define CFG_HZ          40000000

#define CFG_DDR_REFRESH_VAL 0x4139
#define CFG_DDR_TRTW_VAL    0x12
#define CFG_DDR_TWTR_VAL    0x12


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_COMMANDS	(( CONFIG_CMD_DFL | CFG_CMD_DHCP | CFG_CMD_ELF | \
            CFG_CMD_MII | CFG_CMD_PING | CFG_CMD_NET |\
   CFG_CMD_ENV | CFG_CMD_FLASH | CFG_CMD_LOADS | CFG_CMD_RUN | CFG_CMD_LOADB \
   | CFG_CMD_ELF ) & ~CFG_CMD_ENV)

#define CFG_PHY_ADDR    0x18

#define CONFIG_IPADDR   192.168.1.10
#define CONFIG_SERVERIP 192.168.1.11
#define CONFIG_ETHADDR 0x00:0xaa:0xbb:0xcc:0xdd:0xee
#define CFG_FAULT_ECHO_LINK_DOWN    1
//#define CFG_VSC8201_PHY             1
#define CFG_IP175B_PHY             1

#define CFG_MII0_RGMII             0

#define DEBUG
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "hush>"

#include <cmd_confdefs.h>

#endif	/* __CONFIG_H */
