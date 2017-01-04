#ifndef __BOARD_955X_H
#define __BOARD_955X_H

#include <config.h>

#undef MTDPARTS_DEFAULT

#undef CFG_HZ

#include <atheros.h>
#define CONFIG_GPIO_CUSTOM      1
#define CONFIG_HW_WATCHDOG      1
#ifndef FLASH_SIZE
#define FLASH_SIZE 8
#endif
/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */

#if (FLASH_SIZE == 16)
#define CFG_MAX_FLASH_SECT      256    /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE		0x01000000	/* Total flash size */
#elif (FLASH_SIZE == 8)
#define CFG_MAX_FLASH_SECT      128    /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE          0x00800000 /* Total flash size */
#else
#define CFG_MAX_FLASH_SECT      64     /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE		0x00400000	/* Total flash size */
#endif

#ifndef COMPRESSED_UBOOT
#define ENABLE_DYNAMIC_CONF	1
#endif

#undef CFG_ATHRS26_PHY

#if (CFG_MAX_FLASH_SECT * CFG_FLASH_SECTOR_SIZE) != CFG_FLASH_SIZE
#	error "Invalid flash configuration"
#endif

#define CFG_FLASH_WORD_SIZE	unsigned short

#if defined(CONFIG_ATH_NAND_BR) && defined(COMPRESSED_UBOOT)
#define CFG_FLASH_BASE			0xa0100000
#else
/* NOR Flash start address */
#define CFG_FLASH_BASE			0x9f000000
#endif

#ifdef COMPRESSED_UBOOT
#define BOOTSTRAP_TEXT_BASE		CFG_FLASH_BASE
#define BOOTSTRAP_CFG_MONITOR_BASE	BOOTSTRAP_TEXT_BASE
#endif

#define CONFIG_PCI_CONFIG_DATA_IN_OTP

/*
 * Defines to change flash size on reboot
 */
#ifdef ENABLE_DYNAMIC_CONF
#define UBOOT_FLASH_SIZE		(256 * 1024)
#define UBOOT_ENV_SEC_START		(CFG_FLASH_BASE + UBOOT_FLASH_SIZE)

#define CFG_FLASH_MAGIC			0xaabacada
#define CFG_FLASH_MAGIC_F		(UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x20)
#define CFG_FLASH_SECTOR_SIZE_F		*(volatile int *)(CFG_FLASH_MAGIC_F + 0x4)
#define CFG_FLASH_SIZE_F		*(volatile int *)(CFG_FLASH_MAGIC_F + 0x8) /* Total flash size */
#define CFG_MAX_FLASH_SECT_F		(CFG_FLASH_SIZE / CFG_FLASH_SECTOR_SIZE) /* max number of sectors on one chip */
#else
#define CFG_FLASH_SIZE_F		CFG_FLASH_SIZE
#define CFG_FLASH_SECTOR_SIZE_F		CFG_FLASH_SECTOR_SIZE
#endif


#define CFG_DDR_REFRESH_VAL		0x4138

/*
 * The following #defines are needed to get flash environment right
 */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#undef CONFIG_BOOTARGS

#define __gen_cmd(n, a, f, ec, cc, el)		\
	#n "=tftp 0x80060000 ${dir}" #f "&&"	\
	#ec " " #a " " #el "&&"			\
	#cc " $fileaddr " #a " $filesize\0"

#define gen_cmd(n, a, f)			\
	__gen_cmd(n, a, f, erase, cp.b, +$filesize)

#define gen_cmd_el(n, a, f, el)			\
	__gen_cmd(n, a, f, erase, cp.b, +el)

#define nand_gen_cmd(n, a, f, s)		\
	__gen_cmd(n, a, f, nand erase, nand write, s)

#define __fs_name(x, y)		x ## y
#define _fs_name(x, y)		__fs_name(x, y)
#define fs_name(y)		_fs_name(__CONFIG_BOARD_NAME, y)

#ifdef COMPRESSED_UBOOT
#	define ATH_U_FILE	tuboot.bin
#else
#	define ATH_U_FILE	u-boot.bin
#endif


#ifdef CONFIG_ATH_NAND_SUPPORT
#	ifdef CONFIG_ATH_NAND_BR	// nand boot rom
#		if defined(COMPRESSED_UBOOT)
#			define ATH_U_CMD	nand_gen_cmd(lu, 0x0, 2fw.bin, 0x20000)
#			define MTDPARTS_DEFAULT	"mtdparts=ath-nand:128k(u-boot),384k(free),1280k(uImage),7m(rootfs),128k(dummy),128k(caldata)"
#		else
#		define ATH_U_CMD	nand_gen_cmd(lu, 0x0, 2fw.bin, 0x40000)
#		define MTDPARTS_DEFAULT "mtdparts=ath-nand:256k(u-boot),256k(u-boot-env),1280k(uImage),7m(rootfs),128k(dummy),128k(caldata)"
#		endif
#		define ATH_ROOT_DEV	"31:03"
#		define CFG_ENV_ADDR	0x00040000
#	else //dual flash
#		define ATH_U_CMD	gen_cmd(lu, 0x9f000000, ATH_U_FILE)
#		define MTDPARTS_DEFAULT "mtdparts=ath-nor0:320k(u-boot-and-env),6336k(free);ath-nand:256k(u-boot),256k(u-boot-env),1280k(uImage),7m(rootfs),128k(dummy),128k(caldata)"
#		define ATH_ROOT_DEV	"31:05"
#		define CFG_ENV_ADDR	0x9f040000
#	endif
#	define ATH_F_FILE		fs_name(${bc}-nand-jffs2)
#	define ATH_F_LEN		0x700000
#	define ATH_F_ADDR		0x1c0000
#	define ATH_K_FILE		vmlinux${bc}.lzma.uImage
#	define ATH_K_ADDR		0x80000
#	define ATH_F_CMD		nand_gen_cmd(lf, ATH_F_ADDR, ATH_F_FILE, ATH_F_LEN)
#	define ATH_K_CMD		nand_gen_cmd(lk, ATH_K_ADDR, ATH_K_FILE, 0x140000)
#	define ATH_EXTRA_ENV		"bootdevice=0\0"
#else
#	define ATH_F_FILE		fs_name(${bc}-jffs2)
#	define ATH_F_LEN		0x630000
#	define ATH_F_ADDR		0x9f050000
#	define ATH_K_FILE		vmlinux${bc}.lzma.uImage
#	define ATH_K_ADDR		0x9f680000
#	define MTDPARTS_DEFAULT		"mtdparts=ath-nor0:256k(u-boot),64k(u-boot-env),6336k(rootfs),1408k(uImage)," ATH_MTDPARTS_MIB0 ",64k(ART)"
#endif /*CONFIG_MI124*/

#ifndef ATH_ROOT_DEV
#	define ATH_ROOT_DEV	"31:02"
#endif

#ifndef ATH_EXTRA_ENV
#	define ATH_EXTRA_ENV
#endif

#ifndef ATH_U_CMD
#	define ATH_U_CMD	gen_cmd(lu, 0x9f000000, ATH_U_FILE)
#endif

#ifndef ATH_F_CMD
#	define ATH_F_CMD	gen_cmd_el(lf, ATH_F_ADDR, ATH_F_FILE, ATH_F_LEN)
#endif

#ifndef ATH_K_CMD
#	define ATH_K_CMD	gen_cmd(lk, ATH_K_ADDR, ATH_K_FILE)
#endif

#if 0
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"dir=\0" ATH_U_CMD ATH_F_CMD ATH_K_CMD ""
#endif

/* default mtd partition table */
#undef MTDPARTS_DEFAULT
#ifdef COMPRESSED_UBOOT
#define MTDPARTS_DEFAULT    "mtdparts=spi0.0:64k(u-boot),576k(kernel),1344k(rootfs),64k(ART)"
#else
#if (FLASH_SIZE == 16)
#define MTDPARTS_DEFAULT    "mtdparts=spi0.0:256k(u-boot),64k(u-boot-env),384k(custom),1280k(kernel),6528k(rootfs),7808k(inactive),64k(ART)"
#elif (FLASH_SIZE == 8)
#define MTDPARTS_DEFAULT    "mtdparts=spi0.0:256k(u-boot),64k(u-boot-env),320k(custom),1024k(kernel),4928k(rootfs),1536k(inactive),64k(ART)"
#else
#define MTDPARTS_DEFAULT    "mtdparts=spi0.0:256k(u-boot),64k(u-boot-env),320k(custom),768k(kernel),2624k(rootfs),64k(ART)"
#endif
#endif /* #ifdef COMPRESSED_UBOOT */

//[[Benson]2016/03/01]:Modify BOARD_NAME arg to fix string
//#define	CONFIG_BOOTARGS		"console=ttyS0,115200 rootfstype=squashfs,jffs2 init=/etc/preinit board="BOARD_NAME
#define       CONFIG_BOOTARGS         "console=ttyS0,115200 rootfstype=squashfs,jffs2 init=/etc/preinit board=MR1750v2"
//[[Benson]2016/03/01]: End
#if (FLASH_SIZE == 16)
#define CONFIG_EXTRA_ENV_SETTINGS \
"preboot=flashit 0x80100000 fwupgrade.cfg\0" \
"imagechk=test -n \"${check_skip}\" || check_skip=1 && datachk vmlinux,rootfs\0" \
"bootcmd_0=tftp 0x81000000 vmlinux-initramfs.bin && bootm 0x81000000\0" \
"bootargs_0="CONFIG_BOOTARGS" root=31:04 "MTDPARTS_DEFAULT"\0" \
"bootcmd_1=run imagechk && bootm 0x9f0b0000\0" \
"bootcmd_2=run imagechk && bootm 0x9f850000\0" \
"set_bootargs_1=set bootargs_1 ${bootargs} root=31:04 mtdparts=spi0.0:256k(u-boot),64k(u-boot-env),384k(custom),${kernel_size_1}k(kernel),${rootfs_size_1}k(rootfs),7808k(inactive),64k(ART)\0" \
"set_bootargs_2=set bootargs_2 ${bootargs} root=31:05 mtdparts=spi0.0:256k(u-boot),64k(u-boot-env),384k(custom),7808k(inactive),${kernel_size_2}k(kernel),${rootfs_size_2}k(rootfs),64k(ART)\0" \
"kernel_size_1=1280\0" \
"rootfs_size_1=6528\0" \
"kernel_size_2=1280\0" \
"rootfs_size_2=6528\0"
#elif (FLASH_SIZE == 8)
#define CONFIG_EXTRA_ENV_SETTINGS \
"preboot=flashit 0x80100000 fwupgrade.cfg\0" \
"imagechk=test -n \"${check_skip}\" || check_skip=1 && datachk vmlinux,rootfs\0" \
"bootcmd_0=tftp 0x81000000 vmlinux-initramfs.bin && bootm 0x81000000\0" \
"bootargs_0="CONFIG_BOOTARGS" root=31:04 "MTDPARTS_DEFAULT"\0" \
"bootcmd_1=run imagechk && bootm 0x9f0a0000\0" \
"bootcmd_2=run imagechk && bootm 0x9f670000\0" \
"set_bootargs_1=set bootargs_1 ${bootargs} root=31:04 mtdparts=spi0.0:256k(u-boot),64k(u-boot-env),320k(custom),${kernel_size_1}k(kernel),${rootfs_size_1}k(rootfs),1536k(inactive),64k(ART)\0" \
"set_bootargs_2=set bootargs_2 ${bootargs} root=31:05 mtdparts=spi0.0:256k(u-boot),64k(u-boot-env),320k(custom),5952k(inactive),${kernel_size_2}k(kernel),${rootfs_size_2}k(rootfs),64k(ART)\0" \
"kernel_size_1=1024\0" \
"rootfs_size_1=4928\0" \
"kernel_size_2=576\0" \
"rootfs_size_2=960\0"
 #else
 #define CONFIG_EXTRA_ENV_SETTINGS \
"preboot=flashit 0x80100000 fwupgrade.cfg\0" \
"bootcmd_0=tftp 0x81000000 vmlinux-initramfs.bin && bootm 0x81000000\0" \
"bootargs_0="CONFIG_BOOTARGS" root=31:04 "MTDPARTS_DEFAULT"\0" \
"bootcmd_1=bootm 0x9f0a0000\0" \
"set_bootargs_1=set bootargs_1 ${bootargs} root=31:04 mtdparts=spi0.0:256k(u-boot),64k(u-boot-env),320k(custom),${kernel_size_1}k(kernel),${rootfs_size_1}k(rootfs),64k(ART)\0" \
"kernel_size_1=768\0" \
"rootfs_size_1=2624\0"
#endif
//#define CFG_PLL_FREQ    CFG_PLL_720_600_200

/*
 * timeout values are in ticks
 */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET	0x1000

#ifdef CONFIG_ATH_NAND_SUPPORT
#	define CONFIG_BOOTCOMMAND	"nboot 0x81000000 0 0x80000"
#else
#	define CFG_ENV_ADDR		0x9f040000
#	define CONFIG_BOOTCOMMAND	"bootm 0x9f680000"
#endif


/* DDR value from Flash */
#ifdef ENABLE_DYNAMIC_CONF
#define CFG_DDR_MAGIC		0xaabacada
#define CFG_DDR_MAGIC_F		(UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x30)
#define CFG_DDR_CONFIG_VAL_F	*(volatile int *)(CFG_DDR_MAGIC_F + 4)
#define CFG_DDR_CONFIG2_VAL_F	*(volatile int *)(CFG_DDR_MAGIC_F + 8)
#define CFG_DDR_EXT_MODE_VAL_F	*(volatile int *)(CFG_DDR_MAGIC_F + 12)
#endif

#define CONFIG_NET_MULTI
#define CONFIG_NET_RETRY_COUNT 0
#define CONFIG_MEMSIZE_IN_BYTES
#define CONFIG_PCI 1

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#ifndef COMPRESSED_UBOOT
#define ATH_CFG_COMMANDS	((			\
				CONFIG_CMD_DFL	|	\
				CFG_CMD_DHCP	|	\
				CFG_CMD_ELF	|	\
				CFG_CMD_PCI	|	\
				CFG_CMD_FLS	|	\
				CFG_CMD_MII	|	\
				CFG_CMD_PING	|	\
				CFG_CMD_NET	|	\
				CFG_CMD_ENV	|	\
				CFG_CMD_PLL	|	\
				CFG_CMD_FLASH	|	\
				CFG_CMD_LOADS	|	\
				CFG_CMD_RUN	|	\
				CFG_CMD_LOADB	|	\
				CFG_CMD_ELF	|	\
				CFG_CMD_DDR	|	\
				CFG_CMD_ETHREG		\
				) & ~(			\
				CFG_CMD_IMLS	|	\
				CFG_CMD_FLASH		\
				))
#else
#	ifdef CONFIG_ATH_NAND_BR
#		define ATH_CFG_COMMANDS		((			\
						CONFIG_CMD_DFL	|	\
						CFG_CMD_PING	|	\
						CFG_CMD_NET) & ~(	\
						CFG_CMD_FLASH		\
						))
#	else
#		define ATH_CFG_COMMANDS		(CONFIG_CMD_DFL	|	\
				CFG_CMD_PING	|	\
				CFG_CMD_NET)
#	endif
#endif /* #ifndef COMPRESSED_UBOOT */

#ifdef CONFIG_ATH_NAND_SUPPORT
#	ifdef CONFIG_ATH_NAND_BR
#		define CFG_ENV_IS_IN_NAND	1
#		define CFG_ENV_OFFSET		0x40000u
#		define CFG_ENV_SIZE		0x40000u
#		define ATH_EXTRA_CMD		CFG_CMD_NAND
#	else
#		define CFG_ENV_IS_IN_FLASH	1
#		define CFG_ENV_SIZE		CFG_FLASH_SECTOR_SIZE
#		define ATH_EXTRA_CMD		(CFG_CMD_NAND | CFG_CMD_FLASH)
#	endif
#	define NAND_MAX_CHIPS			1
#	define CFG_MAX_NAND_DEVICE		1
#else
#	define ATH_EXTRA_CMD			CFG_CMD_FLASH
#	define CFG_ENV_IS_IN_FLASH		1
#	define CFG_ENV_SIZE			CFG_FLASH_SECTOR_SIZE
#endif

#undef CONFIG_BOOTCOMMAND
#ifndef COMPRESSED_UBOOT
#if (FLASH_SIZE == 16) || (FLASH_SIZE == 8)
#define CONFIG_BOOTCOMMAND "test -n \"${preboot}\" && run preboot; test -n \"${bootseq}\" || bootseq=1,2; run set_bootargs_1; run set_bootargs_2; boot ${bootseq}"
#else
#define CONFIG_BOOTCOMMAND "test -n \"${preboot}\" && run preboot; test -n \"${bootseq}\" || bootseq=1; run set_bootargs_1; boot ${bootseq}"
#endif
#endif /* #ifndef COMPRESSED_UBOOT */
#define DEBUG

#ifdef COMPRESSED_UBOOT
#undef  CFG_ENV_IS_IN_FLASH
#undef  CFG_ENV_IS_IN_NAND
#define CFG_ENV_IS_NOWHERE		1
#endif

#define CONFIG_COMMANDS			(ATH_CFG_COMMANDS | ATH_EXTRA_CMD)
#define CONFIG_IPADDR			192.168.100.9
#define CONFIG_SERVERIP			192.168.100.8
//[[Benson]2016/03/09]: Modified for Sanity check
//#define CONFIG_ETHADDR			0x00:0xaa:0xbb:0xcc:0xdd:0xee
#define CONFIG_ETHADDR			0x00:0x03:0x7f:0x09:0x0b:0xad
//[[Benson]2016/03/09]: End
#define CONFIG_ENV_OVERWRITE		1
#define CFG_FAULT_ECHO_LINK_DOWN	1

#define CFG_PHY_ADDR			0
#define CFG_ATH_GMAC_NMACS		1
#define CFG_GMII			0
#define CFG_MII0_RMII			1
#define CFG_AG7100_GE0_RMII		1

#define CFG_BOOTM_LEN			(16 << 20) /* 16 MB */
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2		"hush>"

/*
** Parameters defining the location of the calibration/initialization
** information for the two Merlin devices.
** NOTE: **This will change with different flash configurations**
*/

#if (FLASH_SIZE == 16)
#define WLANCAL                         0xbfff1000
#define BOARDCAL                        0xbfff0000
#elif (FLASH_SIZE == 8)
#define WLANCAL                         0x9f7f1000
#define BOARDCAL                        0x9f7f0000
#else
#define WLANCAL                         0x9f3f1000
#define BOARDCAL                        0x9f3f0000
#endif
#define ATHEROS_PRODUCT_ID		137
#define CAL_SECTOR			(CFG_MAX_FLASH_SECT - 1)

/* For Merlin, both PCI, PCI-E interfaces are valid */
#define ATH_ART_PCICFG_OFFSET		12

#include <cmd_confdefs.h>

#endif	/* __BOARD_955X_H */
