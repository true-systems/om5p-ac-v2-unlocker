#ifndef _AG7240_PHY_H
#define _AG7240_PHY_H

static inline void ag7240_phy_setup(int unit)
{
#ifdef CONFIG_AR7242_S16_PHY
    if ((is_ar7242() || is_wasp()) && (unit==0)) {
        athrs16_phy_setup(unit);
    } else
#endif
#ifdef CONFIG_AR7242_RGMII_PHY
    if (is_ar7242() && (unit==0)) {
        athr_phy_setup(unit, 0);
    } else
#endif
    {
        athrs26_phy_setup(unit);
#ifdef CONFIG_F1E_PHY
        athr_phy_setup(unit);
#endif
    }
}

static inline void ag7240_phy_link(int unit, int *link)
{
#ifdef CONFIG_AR7242_S16_PHY
    if ((is_ar7242() || is_wasp()) && (unit==0)) {
         *link = athrs16_phy_is_up(unit);
    } else
#endif
#ifdef CONFIG_AR7242_RGMII_PHY
    if (is_ar7242() && (unit==0)) {
         *link = athr_phy_is_up(unit, 0);
    } else
#endif
    {
#ifdef CFG_ATHRS27_PHY
         *link = athrs27_phy_is_up(unit);
#endif
#ifdef CFG_ATHRS26_PHY
         *link = athrs26_phy_is_up(unit);
#endif
#ifdef CONFIG_F1E_PHY
         *link = athr_phy_is_up(unit);
#endif
    }
}

static inline void ag7240_phy_duplex(int unit, int *duplex)
{
#ifdef CONFIG_AR7242_S16_PHY
    if ((is_ar7242() || is_wasp()) && (unit==0)) {
        *duplex = athrs16_phy_is_fdx(unit);
    } else
#endif
#ifdef CONFIG_AR7242_RGMII_PHY
    if (is_ar7242() && (unit==0)) {
        *duplex = athr_phy_is_fdx(unit, 0);
    } else
#endif
    {
        *duplex = athrs26_phy_is_fdx(unit);
#ifdef CONFIG_F1E_PHY
        *duplex = athr_phy_is_fdx(unit);
#endif
    }
}

static inline void ag7240_phy_speed(int unit, int *speed)
{
#ifdef CONFIG_AR7242_S16_PHY
    if ((is_ar7242() || is_wasp()) && (unit==0)) {
        *speed = athrs16_phy_speed(unit);
    } else
#endif
#ifdef CONFIG_AR7242_RGMII_PHY
    if (is_ar7242() && (unit==0)) {
        *speed = athr_phy_speed(unit, 0);
    } else
#endif
    {
        *speed = athrs26_phy_speed(unit);
#ifdef CONFIG_F1E_PHY
        *speed = athr_phy_speed(unit);
#endif
    }
}

#endif /*_AG7240_PHY_H*/
