/*****************************************************************************/
/*! file pb44.c
** /brief Boot support for PB44 board. First version identical to pb45.c
**    
**  This provides the support code required for the AP94 board in the U-Boot
**  environment.  This board is a Hydra based system with two Merlin WLAN
**  interfaces.
**
**  Copyright (c) 2008 Atheros Communications Inc.  All rights reserved.
**
*/

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include "ar7100_soc.h"

 
/******************************************************************************/
/*!
**  \brief ar7100_mem_config
**
**  This is a "C" level implementation of the memory configuration interface
**  for the PB44.  
**
**  \return RAM size in bytes
*/

int
ar7100_mem_config(void)
{

    /* XXX - should be set based board configuration */
    *(volatile unsigned int *)0xb8050004 = 0x400050C0;
    udelay(10);
    *(volatile unsigned int *)0xb8050018 = 0x1313;
    udelay(10);
    *(volatile unsigned int *)0xb805001c = 0x76; /* 66MHz at 1GHz PLL */
    udelay(10);
    *(volatile unsigned int *)0xb8050010 = 0x1099;
    udelay(10);

    return (ar7100_ddr_find_size());
}

/******************************************************************************/
/*!
**  \brief ar7100_usb_initial_config
**
**  -- Enter Detailed Description --
**
**  \param param1 Describe Parameter 1
**  \param param2 Describe Parameter 2
**  \return Describe return value, or N/A for void
*/

long int initdram(int board_type)
{
#ifndef COMPRESSED_UBOOT
    printf("b8050000: 0x%x\n",* (unsigned long *)(0xb8050000));
#endif /* #ifndef COMPRESSED_UBOOT */
    return (ar7100_mem_config());
}

/******************************************************************************/
/*!
**  \brief ar7100_usb_initial_config
**
**  -- Enter Detailed Description --
**
**  \param param1 Describe Parameter 1
**  \param param2 Describe Parameter 2
**  \return Describe return value, or N/A for void
*/

#ifdef COMPRESSED_UBOOT
int checkboard (char *board_string)
{
    strcpy(board_string, "PB44 (ar7100) U-boot");
    return 0;
}
#else
int checkboard (void)
{
    unsigned long pll, pll_div, cpu_div, ahb_div, ddr_div, freq;

    printf("PB44 (ar7100) U-boot \n");

    pll = ar7100_reg_rd(AR7100_CPU_PLL_CONFIG);

    pll_div  = ((pll & PLL_CONFIG_PLL_FB_MASK) >> PLL_CONFIG_PLL_FB_SHIFT) + 1;
    freq     = pll_div * 40;
    cpu_div  = ((pll & PLL_CONFIG_CPU_DIV_MASK) >> PLL_CONFIG_CPU_DIV_SHIFT) + 1;
    ddr_div  = ((pll & PLL_CONFIG_DDR_DIV_MASK) >> PLL_CONFIG_DDR_DIV_SHIFT) + 1;
    ahb_div  = (((pll & PLL_CONFIG_AHB_DIV_MASK) >> PLL_CONFIG_AHB_DIV_SHIFT) + 1)*2;

    printf("Frequency: CPU %d MHz, DDR %d MHz, AHB %dMHz \n", freq/cpu_div, freq/ddr_div, (freq/cpu_div)/ahb_div);

    return 0;
}
#endif /* #ifdef COMPRESSED_UBOOT */

/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long 
flash_get_geom (flash_info_t *flash_info)
{
    int i;
    
    /* XXX this is hardcoded until we figure out how to read flash id */

    flash_info->flash_id  = FLASH_M25P64;
    flash_info->size = 8*1024*1024; /* bytes */
    flash_info->sector_count = flash_info->size/CFG_FLASH_SECTOR_SIZE;

    for (i = 0; i < flash_info->sector_count; i++) {
        flash_info->start[i] = CFG_FLASH_BASE + (i * CFG_FLASH_SECTOR_SIZE);
        flash_info->protect[i] = 0;
    }

#ifndef COMPRESSED_UBOOT
    printf ("flash size 8MB, sector count = %d\n", flash_info->sector_count);
#endif /* #ifndef COMPRESSED_UBOOT */
    return (flash_info->size);

}

extern flash_info_t flash_info[];

#if (CONFIG_COMMANDS & CFG_CMD_PLL)
int do_ar7100_pll (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    char    sectorBuff[65536];
    int     index;

    /*
    ** read the flash sector
    */
    memcpy(sectorBuff,(void *)BOARDCAL, 65536);
    if (argc == 1)  {
        printf("current PLL setting %d\n", sectorBuff[19]);
        return 0;
    }

    /*
    ** Argv[1] contains the value string.  Convert to binary
    */

    index = simple_strtoul(argv[1],0,10);

    /*
    ** If the serial number is less than 0, or greater than 8, it's out of range
    */

    if(index < 0 || index > 8)
    {
        printf("index %d out of range\n", index);
        return 1;
    }

    /* store a u32 at offset 16. we are on big-endian arch, so ... */
    sectorBuff[16] = 0;
    sectorBuff[17] = 0;
    sectorBuff[18] = 0;
    sectorBuff[19] = index;

    flash_erase(flash_info,CAL_SECTOR,CAL_SECTOR);
    write_buff(flash_info,sectorBuff, BOARDCAL, 65536);
    
    return 0;
}

/* N.B.: These values should match CFG_PLL_* values in ar7100.h */
U_BOOT_CMD(
    ar7100_pll, 2, 0, do_ar7100_pll,
    "ar7100_pll - Set CPU/DDR/AHB frequencies\n",
    "ar7100_pll <index> - Set CPU/DDR/AHB frequencies based on the list below:\n"
    "\t0 - 200/200/100\n"
    "\t1 - 300/300/150\n"
    "\t2 - 333/333/166\n"
    "\t3 - 266/266/133\n"
    "\t4 - 266/266/66\n"
    "\t5 - 400/400/200\n"
    "\t6 - 360/360/180\n"
    "\t7 - 400/400/100\n"
    "\t8 - 680/340/170\n"
);
#endif /* #if (CONFIG_COMMANDS & CFG_CMD_PLL) */

#ifdef AG7100_DEBUG
extern int ag7100_rxdbg;
int do_rxdbg_ctl (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    uint16_t val;
    if (argc != 2) {
        printf("incorrect format\n");
        return 1;
    }
    val = simple_strtoul (argv[1], NULL, 16);
    if (val) ag7100_rxdbg = 1;
    else ag7100_rxdbg = 0;
    return 0;
}
U_BOOT_CMD(
    rxdbg, 2, 0, do_rxdbg_ctl,
    "rxdbg - 1 to turn on rx pkt prints, 0 to turn off\n",
    "rxdbg - 1 to turn on rx pkt prints, 0 to turn off\n"
);

extern int ag7100_txdbg;
int do_txdbg_ctl (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{   
    uint16_t val;
    if (argc != 2) {
        printf("incorrect format\n");
        return 1;
    }
    val = simple_strtoul (argv[1], NULL, 16);
    if (val) ag7100_txdbg = 1;
    else ag7100_txdbg = 0;
    return 0;
}
U_BOOT_CMD(
    txdbg, 2, 0, do_txdbg_ctl,
    "txdbg - 1 to turn on rx pkt prints, 0 to turn off\n",
    "txdbg - 1 to turn on rx pkt prints, 0 to turn off\n"
);

#endif /* AG7100_DEBUG */

