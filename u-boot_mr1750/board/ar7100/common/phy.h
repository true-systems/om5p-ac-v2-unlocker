#ifndef _PHY_H
#define _PHY_H

#include <config.h>
/*
 * This file defines the interface between MAC and various phy switches.
 */
#define ag7100_unit2name(_unit) _unit ?  "eth1" : "eth0"
extern int  ag7100_miiphy_read(char *devname, unsigned char phaddr,
		unsigned char reg, unsigned short *data);
extern int  ag7100_miiphy_write(char *devname, unsigned char phaddr,
		unsigned char reg, unsigned short data);

#ifdef CFG_ATHRS16_PHY
inline unsigned short s16_phy_reg_read(unsigned int base, unsigned char addr, unsigned char reg)
{   unsigned short data;
    ag7100_miiphy_read(ag7100_unit2name(base), addr, reg, &data);
    return data;
}
#define phy_reg_read(base, addr, reg) s16_phy_reg_read(base, addr, reg)
#else
#define phy_reg_read(base, addr, reg, datap)                    \
    ag7100_miiphy_read(ag7100_unit2name(base), addr, reg, datap);
#endif
#define phy_reg_write(base, addr, reg, data)                   \
        ag7100_miiphy_write(ag7100_unit2name(base), addr, reg, data);

#endif
