#ifndef _HORNET_H
#define _HORNET_H

/*
 * GE0 AR8021 phy address
 */
#define GE0_AR8021_PHY_ADDR             0x0

/*
 * Ethernet config registers
 */
#define AG7240_ETH_CFG                  0x18070000
#define AG7240_ETH_CFG_RGMII_GE0        (1<<0)
#define AG7240_ETH_CFG_MII_GE0          (1<<1)
#define AG7240_ETH_CFG_GMII_GE0         (1<<2)
#define AG7240_ETH_CFG_MII_GE0_MASTER   (1<<3)
#define AG7240_ETH_CFG_MII_GE0_SLAVE    (1<<4)
#define AG7240_ETH_CFG_GE0_ERR_EN       (1<<5)
#define AG7240_ETH_CFG_SW_ONLY_MODE     (1<<6)
#define AG7240_ETH_CFG_SW_PHY_SWAP      (1<<7)
#define AG7240_ETH_CFG_SW_PHY_ADDR_SWAP (1<<8)

/* Phy Specific status fields */
#define ATHER_STATUS_LINK_MASK                0xC000
#define ATHER_STATUS_LINK_SHIFT               14
#define ATHER_STATUS_FULL_DEPLEX              0x2000
#define ATHR_STATUS_LINK_PASS                 0x0400
#define ATHR_LATCH_LINK_PASS                  0x0004

#ifndef ag7240_unit2name
#define ag7240_unit2name(_unit) _unit ?  "eth1" : "eth0"
#endif

#endif
