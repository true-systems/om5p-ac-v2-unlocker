/*****************************************************************************/
/*! file ap94_pci.c
** /brief PCI support for PB45 board derived from AP94. plat_dev_init()
** is removed as PB45 has two mini-PCI slots and no calibration data in
** Flash.
**    
**  This provides the support code required for PCI support on the PB45
**  board in the U-Boot environment.  This board is a Hydra based system.
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
#include <pci.h>
#include "ar7100_soc.h"

/*
** PCI controller "hose" value
*/

static struct pci_controller hose;

/*
 * Mask table, bits to mask for quantity of size 1, 2 or 4 bytes.
 * 0 and 3 are not valid indexes...
 */
static uint32_t bytemask[] = {
	/*0*/	0,
	/*1*/	0xff,
	/*2*/	0xffff,
	/*3*/	0,
	/*4*/	0xffffffff,
};

static void crp_read(uint32_t ad_cbe, uint32_t *data);
static void crp_write(uint32_t ad_cbe, uint32_t data);
static int  ar7100_check_error(int verbose);
static int  ar7100_pci_read(uint32_t addr, uint32_t cmd, uint32_t* data);
static int  ar7100_pci_write(uint32_t addr, uint32_t cmd, uint32_t data);
static uint32_t ar7100_config_addr(uint8_t bus_num, uint16_t devfn, int where);
static uint32_t local_byte_lane_enable_bits(uint32_t n, int size);
static int  ar7100_local_read_config(int where, int size, uint32_t *value);
static int  ar7100_local_write_config(int where, int size, uint32_t value);


/*
 * Read from PCI config space (host ctrl)
 */
static void crp_read(uint32_t ad_cbe, uint32_t *data)
{
    ar7100_reg_wr(AR7100_PCI_CRP_AD_CBE, ad_cbe);
	*data     =  ar7100_reg_rd(AR7100_PCI_CRP_RDDATA);
}

/*
 * Write to PCI config space (host ctrl)
 */
static void crp_write(uint32_t ad_cbe, uint32_t data)
{ 
    ar7100_reg_wr(AR7100_PCI_CRP_AD_CBE, (AR7100_CRP_CMD_WRITE | ad_cbe));
	ar7100_reg_wr(AR7100_PCI_CRP_WRDATA, data);
}


/*
 * Check for PCI errors (aborts, parity etc.), for configuration cycles
 * PCI error reg: 1:0
 * AHB error reg: 0
 * Both write-back-1 to clear.
 */
static int 
ar7100_check_error(int verbose)
{
    uint32_t error = 0, trouble = 0, status;

    error = ar7100_reg_rd(AR7100_PCI_ERROR) & 3;

    if (error) {
        ar7100_local_read_config(PCI_STATUS, 2, &status);

        if (verbose) {
            printf("PCI error %d at PCI addr 0x%x status %#x\n", 
                    error, ar7100_reg_rd(AR7100_PCI_ERROR_ADDRESS),
                    status);
        }

        ar7100_reg_wr(AR7100_PCI_ERROR, error);
        ar7100_local_write_config(PCI_STATUS, 2, status);
        /*
         * flush
         */
        ar7100_local_read_config(PCI_STATUS, 2, &status);
        trouble = 1;
    }

    error = 0;
    error = ar7100_reg_rd(AR7100_PCI_AHB_ERROR) & 1;

    if (error) {
        ar7100_local_read_config(PCI_STATUS, 2, &status);

        if (verbose) {
            printf("AHB error %d at AHB address 0x%x status %#x\n", 
                   ar7100_reg_rd(AR7100_PCI_AHB_ERROR_ADDRESS), status);
        }

        ar7100_reg_wr(AR7100_PCI_AHB_ERROR, error);
        trouble = 1;
    }

    return trouble;
}

static int 
ar7100_pci_read(uint32_t addr, uint32_t cmd, uint32_t* data)
{
	int retval = 0;

    ar7100_reg_wr(AR7100_PCI_CFG_AD, addr);
    ar7100_reg_wr(AR7100_PCI_CFG_CBE, cmd);

	/* 
     * the result of the read is now in CFG_RDATA 
     */
    if (!ar7100_check_error(0))
        *data = ar7100_reg_rd(AR7100_PCI_CFG_RDDATA);
    else {
        *data = 0xffffffff;
        retval = 1;
    }

	return retval;
}

static int 
ar7100_pci_write(uint32_t addr, uint32_t cmd, uint32_t data)
{    
	int retval = 0;

    ar7100_reg_wr(AR7100_PCI_CFG_AD, addr);
    ar7100_reg_wr(AR7100_PCI_CFG_CBE, cmd);

	/* 
     * execute the write by writing to CFG_WDATA 
     */ 
    ar7100_reg_wr(AR7100_PCI_CFG_WRDATA, data);

    if (ar7100_check_error(0))
        retval = 1;

	return retval;
}

/*
 * This is assuming idsel of device 0 is connected to Address line 17
 * Address for type 0 config is as follows:
 * AD: 
 *  1:0 00 indicates type zero transaction
 *  7:2    indicates the target config dword
 *  10:8   indicates the target function within the physical device
 *  31:11  are reserved (and most probably used to connect idsels)
 */
static uint32_t 
ar7100_config_addr(uint8_t bus_num, uint16_t devfn, int where)
{
	uint32_t addr;

	if (!bus_num) {
		/* type 0 */
		addr = (1 << (AR7100_PCI_IDSEL_ADLINE_START + PCI_DEV(devfn))) |
               ((PCI_FUNC(devfn)) << 8)                                 |
		       (where & ~3);	
	} else {
		/* type 1 */
		addr = (bus_num << 16) | ((PCI_DEV(devfn)) << 11) | 
			((PCI_FUNC(devfn)) << 8) | (where & ~3) | 1;
	}

	return addr;
}


static uint32_t 
local_byte_lane_enable_bits(uint32_t n, int size)
{
	if (size == 1)
		return (0xf & ~BIT(n)) << 20;
	if (size == 2)
		return (0xf & ~(BIT(n) | BIT(n+1))) << 20;
	if (size == 4)
		return 0;
	return 0xffffffff;
}

static int 
ar7100_local_read_config(int where, int size, uint32_t *value)
{ 
	uint32_t n, data;

	/*printk("ar7100_local_read_config from %d size %d\n", where, size);*/
	n = where % 4;
	crp_read(where & ~3, &data);
	*value = (data >> (8*n)) & bytemask[size];
	/*printk("ar7100_local_read_config read %#x\n", *value);*/

	return 0;
}

static int 
ar7100_local_write_config(int where, int size, uint32_t value)
{
	uint32_t n, byte_enables, data;

	n = where % 4;
	byte_enables = local_byte_lane_enable_bits(n, size);
	if (byte_enables == 0xffffffff)
		return -1;
	data = value << (8*n);
	crp_write((where & ~3) | byte_enables, data);

	return 0;
}


static int ar7100_pci_read_config(struct pci_controller *hose,
			      pci_dev_t dev, int offset, u32* value)
{
	uint32_t    addr;
	uint8_t     bus_num = PCI_BUS(dev);
    uint16_t    devfn = dev & 0xffff;
    

	addr = ar7100_config_addr(bus_num, devfn, offset);
	ar7100_pci_read(addr, AR7100_CFG_CMD_READ, value);

	return 0;
}


static int 
ar7100_pci_write_config(struct pci_controller *hose, pci_dev_t dev, int offset, u32 value)
{
	uint32_t    addr;
	uint8_t     bus_num = PCI_BUS(dev);
    uint16_t    devfn = dev & 0xffff;

	addr = ar7100_config_addr(bus_num, devfn, offset);
	ar7100_pci_write(addr, AR7100_CFG_CMD_WRITE, value);
	return 0;
}

/******************************************************************************/
/*!
**  \brief pci host initialization
**
**  Sets up the PCI controller on the host.  For AR7100 this may not be necessary,
**  but this function is required for board support.
**
** We want a 1:1 mapping between PCI and DDR for inbound and outbound.
** The PCI<---AHB decoding works as follows:
**
** 8 registers in the DDR unit provide software configurable 32 bit offsets
** for each of the eight 16MB PCI windows in the 128MB. The offsets will be 
** added to any address in the 16MB segment before being sent to the PCI unit.
**
** Essentially  for any AHB address generated by the CPU,
** 1. the MSB  four bits are stripped off, [31:28],
** 2. Bit 27 is used to decide between the lower 128Mb (PCI) or the rest of 
**    the AHB space
** 3. Bits 26:24 are used to access one of the 8 window registers and are 
**    masked off.
** 4. If it is a PCI address, then the WINDOW offset in the WINDOW register 
**    corresponding to the next 3 bits (bit 26:24) is ADDED to the address, 
**    to generate the address to PCI unit.
**
**     eg. CPU address = 0x100000ff
**         window 0 offset = 0x10000000
**         This points to lowermost 16MB window in PCI space.
**         So the resulting address would be 0x000000ff+0x10000000
**         = 0x100000ff
**
**         eg2. CPU address = 0x120000ff
**         WINDOW 2 offset = 0x12000000
**         resulting address would be 0x000000ff+0x12000000
**                         = 0x120000ff 
**
** There is no translation for inbound access (PCI device as a master)
**
**  \return N/A
*/

void pci_init_board (void)
{
    uint32_t cmd, val;

    ar7100_reg_rmw_set(AR7100_RESET, 
                      (AR7100_RESET_PCI_BUS|AR7100_RESET_PCI_CORE));
    udelay(1000 * 1000);

    ar7100_reg_rmw_clear(AR7100_RESET, 
                      (AR7100_RESET_PCI_BUS|AR7100_RESET_PCI_CORE));
    udelay(1000 * 1000);

    ar7100_write_pci_window(0);
    ar7100_write_pci_window(1);
    ar7100_write_pci_window(2);
    ar7100_write_pci_window(3);
    ar7100_write_pci_window(4);
    ar7100_write_pci_window(5);
    ar7100_write_pci_window(6);
    ar7100_write_pci_window(7);

    udelay(1000 * 1000);


	cmd = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_INVALIDATE |
          PCI_COMMAND_PARITY|PCI_COMMAND_SERR|PCI_COMMAND_FAST_BACK;

    ar7100_local_write_config(PCI_COMMAND, 4, cmd);

    /*
     * clear any lingering errors and register core error IRQ
     */
    ar7100_check_error(0);
    
    /*
    ** Now, configure for u-boot tools
    */
    
	hose.first_busno = 0;
	hose.last_busno = 0xff;

	/* System space */
	pci_set_region( &hose.regions[0],
		            0x80000000,
		            0x00000000,
		            32 * 1024 * 1024,
		            PCI_REGION_MEM | PCI_REGION_MEMORY);

	/* PCI memory space */
	pci_set_region( &hose.regions[1],
		            0x10000000,
		            0x10000000,
		            128 * 1024 * 1024,
		            PCI_REGION_MEM);

	hose.region_count = 2;

	pci_register_hose(&hose);

	pci_set_ops(&hose,
		pci_hose_read_config_byte_via_dword,
		pci_hose_read_config_word_via_dword,
		ar7100_pci_read_config,
		pci_hose_write_config_byte_via_dword,
		pci_hose_write_config_word_via_dword,
		ar7100_pci_write_config);

    return 0;
}


