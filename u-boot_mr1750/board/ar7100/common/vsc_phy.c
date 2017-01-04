#include <config.h>
#include "phy.h"

void
vsc_phy_setup(int unit)
{
    phy_reg_write(0, CFG_PHY_ADDR, 0, 0x8000);
    phy_reg_write(0, CFG_PHY_ADDR, 0x1c, 0x4);
    phy_reg_write(0, CFG_PHY_ADDR, 0x17, 0x1000);
    //ag7100_mii_write(0, CFG_PHY_ADDR, 0, 0x2100);
    phy_reg_write(0, CFG_PHY_ADDR, 31, 0x2a30);
    phy_reg_write(0, CFG_PHY_ADDR, 8, 0x10);
    phy_reg_write(0, CFG_PHY_ADDR, 31, 0);
    phy_reg_write(0, CFG_PHY_ADDR, 0x12, 0x0008);
}
