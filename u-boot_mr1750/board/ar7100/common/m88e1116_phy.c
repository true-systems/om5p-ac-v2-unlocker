/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright c 2003 Atheros Communications, Inc.,  All Rights Reserved.
 * modify for M88E1116, changed information as:
 * 1. phy address changed to 0x14 for eap7660d
 * 2. in order to avoid compiler error, changed all athr_* and ather_* to M88E1116_*
 * 3. delete all mdelay()
 * 4. MODULE_NAME
 */

/*
 * Manage the atheros ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */

#ifdef __BDI
#include "bdi.h"
#else
#ifdef __ECOS
#if defined(CYGNUM_USE_ENET_VERBOSE)
#   undef  VERBOSE
#   define VERBOSE CYGNUM_USE_ENET_VERBOSE
#else
#   define VERBOSE 0
#endif
#else
#include <config.h>
#include <linux/types.h>
#include <common.h>
#include <miiphy.h>
#include "phy.h"
#include "ar7100_soc.h"
#define printk printf
#endif
#endif

#ifndef VERBOSE
#define  VERBOSE           0
#endif

#include "m88e1116_phy.h"

#define MODULE_NAME "M88E1116"

typedef struct {
  int              is_enet_port;
  int              mac_unit;
  unsigned int     phy_addr;
}m88e1116_phy_t;

m88e1116_phy_t phy_info[] = {
    {is_enet_port: 1,
     mac_unit    : 0,
     phy_addr    : 0x14},
    {is_enet_port: 1,
     mac_unit    : 1,
     phy_addr    : 0x15}
};

static uint16_t
ag7100_mii_read(uint32_t phybase, uint16_t phyaddr, uint16_t reg)
{
    uint16_t val;

    phy_reg_read(phybase, phyaddr, reg, &val);
    return val;
}

static void
ag7100_mii_write(uint32_t phybase, uint16_t phyaddr, uint16_t reg, uint32_t val)
{
    phy_reg_write(phybase, phyaddr, reg, val);
}

static m88e1116_phy_t *
m88e1116_phy_find(int unit)
{
    int i;
    m88e1116_phy_t *phy;

    for(i = 0; i < sizeof(phy_info)/sizeof(m88e1116_phy_t); i++) {
        phy = &phy_info[i];

        if (phy->is_enet_port && (phy->mac_unit == unit))
            return phy;
    }

    return NULL;
}

int
m88e1116_phy_setup(int unit)
{
    m88e1116_phy_t *phy = m88e1116_phy_find(unit);
    uint16_t  phyHwStatus;
    uint16_t  timeout;

    if (!phy) {
        printk(MODULE_NAME": \nNo phy found for unit %d\n", unit);
        return;
    }


    /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */

    phy_reg_write(unit, phy->phy_addr, M88E1116_AUTONEG_ADVERT,
                  M88E1116_ADVERTISE_ALL);

    phy_reg_write(unit, phy->phy_addr, M88E1116_1000BASET_CONTROL,
                  M88E1116_ADVERTISE_1000FULL);

    /* delay tx_clk */
    phy_reg_write(unit, phy->phy_addr, 0x1D, 0x5);
    phy_reg_write(unit, phy->phy_addr, 0x1E, 0x100);

    /* Reset PHYs*/
    phy_reg_write(unit, phy->phy_addr, M88E1116_PHY_CONTROL,
                  M88E1116_CTRL_AUTONEGOTIATION_ENABLE
                  | M88E1116_CTRL_SOFTWARE_RESET);


    /*
     * Wait up to 3 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
    timeout=20;
    for (;;) {
        phy_reg_read(unit, phy->phy_addr, M88E1116_PHY_CONTROL, &phyHwStatus);

        if (M88E1116_RESET_DONE(phyHwStatus)) {
        printk(MODULE_NAME": Port %d, Neg Success\n", unit);
            break;
        }
        if (timeout == 0) {
        printk(MODULE_NAME": Port %d, Negogiation timeout\n", unit);
            break;
        }
        if (--timeout == 0) {
        printk(MODULE_NAME": Port %d, Negogiation timeout\n", unit);
            break;
        }

    }

    printk(MODULE_NAME": unit %d phy addr %x ", unit, phy->phy_addr);
    printk(MODULE_NAME": reg0 %x\n", ag7100_mii_read(0, phy->phy_addr, 0));
    /*
     * After the phy is setup, the LED control changed.
     */
    phy_reg_write(unit, phy->phy_addr, 0x16, 0x3);
    phy_reg_write(unit, phy->phy_addr, 0x10, 0x177);
    phy_reg_write(unit, phy->phy_addr, 0x11, 0x5);
    phy_reg_write(unit, phy->phy_addr, 0x16, 0x0);
}

int
m88e1116_phy_is_up(int unit)
{
    int status;
    m88e1116_phy_t *phy = m88e1116_phy_find(unit);

    if (!phy)
        return 0;
    status = ag7100_mii_read(0, phy->phy_addr, M88E1116_PHY_SPEC_STATUS);
    if (status & M88E1116_STATUS_LINK_PASS)
        return 1;

    return 0;
}

int
m88e1116_phy_is_fdx(int unit)
{
    int status;
    m88e1116_phy_t *phy = m88e1116_phy_find(unit);
    int ii = 200;
    if (!phy)
        return 0;
    do {
    status = ag7100_mii_read(0, phy->phy_addr, M88E1116_PHY_SPEC_STATUS);
    } while((!(status & M88E1116_STATUS_RESOVLED)) && --ii);
    status = !(!(status & M88E1116_STATUS_FULL_DEPLEX));
    return (status);
}

int
m88e1116_phy_speed(int unit)
{
    int status;
    m88e1116_phy_t *phy = m88e1116_phy_find(unit);
    int ii = 200;

    if (!phy)
        return 0;
    do {
        status = ag7100_mii_read(0, phy->phy_addr, M88E1116_PHY_SPEC_STATUS);
    }while((!(status & M88E1116_STATUS_RESOVLED)) && --ii);
    status = ((status & M88E1116_STATUS_LINK_MASK) >> M88E1116_STATUS_LINK_SHIFT);
    switch(status) {
    case 0:
        return _10BASET;
    case 1:
        return _100BASET;
    case 2:
        return _1000BASET;
    default:
        printk(MODULE_NAME": Unkown speed read!\n");
    }
    return -1;
}
