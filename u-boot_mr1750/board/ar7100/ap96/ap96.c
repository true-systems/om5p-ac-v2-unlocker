/*****************************************************************************/
/*! file ap96.c
** /brief Boot support for AP96 board
**    
**  This provides the support code required for the AP96 board in the U-Boot
**  environment.  This board is a Hydra based system with two Merlin WLAN
**  interfaces and S16 GigE interfaces.
** 
**  Code derived from AP94
**
**  AP96 is similar to AP94 in all respects except the ethernet interfaces. 
**  AP94 has S26, while AP96 uses S16. PB45 also uses S16, but PB45 has a
**  32 bit wide memory bus and two mini-PCI slots whereas AP94 and AP96
**  have a 16 bit wide memory bus and WLAN interfaces on the board. So,
**  while PB45 doesn't have a caldata partition in flash, AP94 and AP96
**  will have the radio calibration of the WLAN interfaces in Flash ROM.
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

extern flash_info_t flash_info[];
 
/******************************************************************************/
/*!
**  \brief ar7100_mem_config
**
**  This is a "C" level implementation of the memory configuration interface
**  for the PB45.  
**
**  \return RAM size in bytes
*/

void ar7100_gpio_config(void)
{
#ifdef CONFIG_GPIO_CUSTOM
#endif
}

int
ar7100_mem_config(void)
{

    /* XXX - should be set based board configuration */
    *(volatile unsigned int *)0xb8050004 = 0x50C0;
    udelay(10);
    *(volatile unsigned int *)0xb8050018 = 0x1313;
    udelay(10);
    *(volatile unsigned int *)0xb805001c = 0xee;
    udelay(10);
    *(volatile unsigned int *)0xb8050010 = 0x1099;
    udelay(10);

    ar7100_gpio_config();

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
    printf("b8050000: 0x%x\n",* (unsigned long *)(0xb8050000));
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

int checkboard (void)
{

    printf(BOARD_NAME" (ar7100) U-boot\n");
    return 0;
}

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

/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long 
flash_get_geom (flash_info_t *flash_info)
{
    int i;
    
    /* XXX this is hardcoded until we figure out how to read flash id */

    flash_info->flash_id  = FLASH_M25P64;
    flash_info->size = CFG_FLASH_SIZE; /* bytes */
    flash_info->sector_count = flash_info->size/CFG_FLASH_SECTOR_SIZE;

    for (i = 0; i < flash_info->sector_count; i++) {
        flash_info->start[i] = CFG_FLASH_BASE + (i * CFG_FLASH_SECTOR_SIZE);
        flash_info->protect[i] = 0;
    }

    printf ("flash size %dMB, sector count = %d\n", FLASH_SIZE, flash_info->sector_count);
    return (flash_info->size);

}

#if(0) // (CONFIG_COMMANDS & CFG_CMD_BSP)

/**********************************************************************************
** do_mac_setting
**
** This is the executable portion of the progmac command.  This will process the
** MAC address strings, and program them into the appropriate flash sector..
**
*/

int do_mac (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    char    sectorBuff[65536];
    int     serno;

    /*
    ** Argv[1] contains the value string.  Convert to binary, and program the
    ** values in flash
    */
    
    serno = simple_strtoul(argv[1],0,10);
    
    /*
    ** If the serial number is less than 0, or greater than 0x1fff, it's out of range
    */
    
    if(serno < 0 || serno > 0x1fff)
    {
        printf("Serno out of range\n",serno);
        return 1;
    }
    
    /*
    ** Create the 24 bit number that composes the lower 3 bytes of the MAC address
    */
    
    serno = 0xFFFFFF & ( (ATHEROS_PRODUCT_ID << 13) | (serno & 0x1fff));
    
    /*
    ** Get the values from flash, and program into the MAC address registers
    */
    
    memcpy(sectorBuff,(void *)BOARDCAL, 65536);
    
    /*
    ** Set the first and second values
    */
    
    sectorBuff[0] = 0x00;
    sectorBuff[1] = 0x03;
    sectorBuff[2] = 0x7f;

    sectorBuff[3] = 0xFF & (serno >> 16);
    sectorBuff[4] = 0xFF & (serno >> 8);
    sectorBuff[5] = 0xFF &  serno;
    
    /*
    ** Increment by 1 for the second MAC address
    */

    serno++;    
    memcpy(&sectorBuff[6],&sectorBuff[0],3);
    sectorBuff[3] = 0xFF & (serno >> 16);
    sectorBuff[4] = 0xFF & (serno >> 8);
    sectorBuff[5] = 0xFF &  serno;
    
    flash_erase(flash_info,CAL_SECTOR,CAL_SECTOR);
    write_buff(flash_info,sectorBuff, BOARDCAL, 65536);
    
    return 0;
}

U_BOOT_CMD(none, 2, 0, do_mac,NULL,NULL);
    

U_BOOT_CMD(
    progmac, 2, 0, do_mac,
    "progmac - Set ethernet MAC addresses\n",
    "progmac <serno> - Program the MAC addresses\n"
    "                <serno> is the value of the last\n"
    "                4 digits (decimal) of the serial number\n"
);

#endif
