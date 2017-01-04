/*
 * This file contains the configuration parameters for the dbau1x00 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ar7100.h>

#define CONFIG_GPIO_CUSTOM      1
/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS     1       /* max number of memory banks */
#if (FLASH_SIZE == 16)
#define CFG_MAX_FLASH_SECT      64     /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (256*1024)
#define CFG_FLASH_SIZE          0x01000000 /* Total flash size */
#elif (FLASH_SIZE == 8)
#define CFG_MAX_FLASH_SECT      128    /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE          0x00800000 /* Total flash size */
#else
#define CFG_MAX_FLASH_SECT      64     /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE          0x00400000 /* Total flash size */
#endif

#define ENABLE_DYNAMIC_CONF 1

#if (CFG_MAX_FLASH_SECT * CFG_FLASH_SECTOR_SIZE) != CFG_FLASH_SIZE
#	error "Invalid flash configuration"
#endif

#define CFG_FLASH_WORD_SIZE     unsigned short 
#define CFG_FLASH_ADDR0         (0x5555)        
#define CFG_FLASH_ADDR1         (0x2AAA)

/* 
 * We boot from this flash
 */
#define CFG_FLASH_BASE          0xbf000000

/*
 * Defines to change flash size on reboot
 */
#ifdef ENABLE_DYNAMIC_CONF
#define UBOOT_FLASH_SIZE          (256 * 1024)
#define UBOOT_ENV_SEC_START       (CFG_FLASH_BASE + UBOOT_FLASH_SIZE)

#define CFG_FLASH_MAGIC           0xaabacada
#define CFG_FLASH_MAGIC_F         (UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x20)
#define CFG_FLASH_SECTOR_SIZE_F   *(volatile int *)(CFG_FLASH_MAGIC_F + 0x4)
#define CFG_FLASH_SIZE_F          *(volatile int *)(CFG_FLASH_MAGIC_F + 0x8) /* Total flash size */
#define CFG_MAX_FLASH_SECT_F      (CFG_FLASH_SIZE / CFG_FLASH_SECTOR_SIZE) /* max number of sectors on one chip */
#endif

#undef CONFIG_ROOTFS_RD
#undef CONFIG_ROOTFS_FLASH
#undef CONFIG_BOOTARGS_FL
#undef CONFIG_BOOTARGS_RD
#undef CONFIG_BOOTARGS
#undef  MTDPARTS_DEFAULT
#undef  MTDIDS_DEFAULT

#define CONFIG_ROOTFS_FLASH

/* XXX - putting rootfs in last partition results in jffs errors */

#define CONFIG_BOOTARGS     "console=ttyS0,115200 root=31:04 rootfstype=squashfs,jffs2 init=/etc/preinit "MTDPARTS_DEFAULT" board="BOARD_NAME

/* default mtd partition table */
#if (FLASH_SIZE == 16)
#define MTDPARTS_DEFAULT    "mtdparts=ar7100-nor0:256k(u-boot),256k(u-boot-env),1280k(custom),1536k(kernel),10752k(rootfs),640k(failsafe_k),1408k(failsafe_r),256k(ART)"
#elif (FLASH_SIZE == 8)
#define MTDPARTS_DEFAULT    "mtdparts=ar7100-nor0:256k(u-boot),64k(u-boot-env),320k(custom),1024k(kernel),4928k(rootfs),576k(failsafe_k),960k(failsafe_r),64k(ART)"
#else
#define MTDPARTS_DEFAULT    "mtdparts=ar7100-nor0:256k(u-boot),64k(u-boot-env),320k(custom),768k(kernel),2624k(rootfs),64k(ART)"
#endif
#define MTDIDS_DEFAULT      "nor0=ar7100-nor0"

/* 
 * The following #defines are needed to get flash environment right 
 */
#define CFG_MONITOR_BASE    TEXT_BASE
#define CFG_MONITOR_LEN     (192 << 10)

#undef CFG_HZ
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

#define CFG_HZ	(680000000/2)

/* 
 * timeout values are in ticks 
 */
#define CFG_FLASH_ERASE_TOUT    (2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT    (2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET  0x1000

#define CFG_ENV_IS_IN_FLASH    1
#undef CFG_ENV_IS_NOWHERE  

/* Address and size of Primary Environment Sector   */
#define CFG_ENV_ADDR        0xbf040000
#define CFG_ENV_SIZE        CFG_FLASH_SECTOR_SIZE 

#define CONFIG_BOOTCOMMAND "test -n \"${preboot}\" && run preboot; test -n \"${bootseq}\" || bootseq=1,2; boot ${bootseq}"

/*
** Extra environmental variables useful for loading the board
*/

#if (FLASH_SIZE == 16)
#define CONFIG_EXTRA_ENV_SETTINGS \
"imagechk=test -n \"${check_skip}\" || check_skip=1 && datachk vmlinux,rootfs\0" \
"bootcmd_0=tftp 0x81000000 vmlinux-initramfs.bin && bootm 0x81000000\0" \
"bootcmd_1=run imagechk && bootm 0xbf1c0000\0" \
"bootcmd_2=run imagechk && bootm 0xbfdc0000\0" \
"bootargs_2=console=ttyS0,115200 root=31:06 rootfstype=squashfs,jffs2 init=/etc/preinit "MTDPARTS_DEFAULT" board="BOARD_NAME"\0"
#elif (FLASH_SIZE == 8)
#define CONFIG_EXTRA_ENV_SETTINGS \
"imagechk=test -n \"${check_skip}\" || check_skip=1 && datachk vmlinux,rootfs\0" \
"bootcmd_0=tftp 0x81000000 vmlinux-initramfs.bin && bootm 0x81000000\0" \
"bootcmd_1=run imagechk && bootm 0xbf0a0000\0" \
"bootcmd_2=run imagechk && bootm 0xbf670000\0" \
"bootargs_2=console=ttyS0,115200 root=31:06 rootfstype=squashfs,jffs2 init=/etc/preinit "MTDPARTS_DEFAULT" board="BOARD_NAME"\0"
#else
#define CONFIG_EXTRA_ENV_SETTINGS \
"bootcmd_0=tftp 0x81000000 vmlinux-initramfs.bin && bootm 0x81000000\0" \
"bootcmd_1=bootm 0xbf0a0000\0"
#endif

#define CONFIG_NR_DRAM_BANKS    		2

#define CFG_BOARD_AP96 1

/*
** Parameters defining the location of the calibration/initialization
** information for the two Merlin devices.
** NOTE: **This will change with different flash configurations**
*/

#if (FLASH_SIZE == 16)
#define MERLIN24CAL                     0xbffc1000
#define MERLIN50CAL                     0xbffc5000
#define BOARDCAL                        0xbffc0000
#elif (FLASH_SIZE == 8)
#define MERLIN24CAL                     0xbf7f1000
#define MERLIN50CAL                     0xbf7f5000
#define BOARDCAL                        0xbf7f0000
#else
#define MERLIN24CAL                     0xbf3f1000
#define MERLIN50CAL                     0xbf3f5000
#define BOARDCAL                        0xbf3f0000
#endif
#define ATHEROS_PRODUCT_ID              145
#define CAL_SECTOR                      (CFG_MAX_FLASH_SECT - 1)

/*
** Configure the CPU clock settings
*/

#define CFG_PLL_FREQ	CFG_PLL_680_340_170
/*
** These values for DDR configuration came from the
** BDM configuration script
*/

#define CFG_DDR_REFRESH_VAL     		0x461b
#define CFG_DDR_CONFIG_VAL      		0x77b8884e
#define CFG_DDR_CONFIG2_VAL             0x812cd6a8
#define CFG_DDR_MODE_VAL_INIT   		0x131
#define CFG_DDR_EXT_MODE_VAL    		0x0
#define CFG_DDR_MODE_VAL        		0x33
#define CFG_DDR_TRTW_VAL        		0x1f
#define CFG_DDR_TWTR_VAL        		0x1e
#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL  	0xff

/* DDR value from Flash */
#ifdef ENABLE_DYNAMIC_CONF
#define CFG_DDR_MAGIC           0xaabacada
#define CFG_DDR_MAGIC_F         (UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x30)
#define CFG_DDR_CONFIG_VAL_F    *(volatile int *)(CFG_DDR_MAGIC_F + 4)
#define CFG_DDR_CONFIG2_VAL_F   *(volatile int *)(CFG_DDR_MAGIC_F + 8)
#define CFG_DDR_EXT_MODE_VAL_F  *(volatile int *)(CFG_DDR_MAGIC_F + 12)
#endif

#define CONFIG_NET_MULTI
#define CONFIG_MEMSIZE_IN_BYTES
#define CONFIG_PCI


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_COMMANDS (( CONFIG_CMD_DFL | CFG_CMD_DHCP | CFG_CMD_ELF | CFG_CMD_PCI |	\
	CFG_CMD_MII | CFG_CMD_PING | CFG_CMD_NET | CFG_CMD_ENV |	\
	CFG_CMD_FLASH | CFG_CMD_LOADS | CFG_CMD_RUN | CFG_CMD_LOADB | CFG_CMD_ELF | CFG_CMD_ETHREG | CFG_CMD_BSP ))

/*
** Ethernet Configuration
** Set default values, and select the PHY to use
*/

#define CONFIG_IPADDR   192.168.1.1
#define CONFIG_SERVERIP 192.168.1.101
#define CONFIG_ETHADDR 0x00:0xaa:0xbb:0xcc:0xdd:0xee
#define CONFIG_ENV_OVERWRITE        1
#define CFG_FAULT_ECHO_LINK_DOWN    1
#define CONFIG_PHY_GIGE       		1              /* GbE speed/duplex detect */

#define CFG_M88E1116_PHY  			1
#define CFG_AG7100_NMACS 			1

#define CFG_PHY_ADDR				0  /* Port 4 */
#define CFG_GMII 1
#define CFG_MII0_RGMII 0
#define CFG_AG7100_GE0_RGMII 1
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
