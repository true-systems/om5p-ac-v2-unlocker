#ifndef _AG7100_PHY_H
#define _AG7100_PHY_H

#ifdef CFG_ATHRS26_PHY
#ifndef AR9100
#include "../board/ar7100/ap94/athrs26_phy.h"

#define ag7100_phy_setup(unit)          athrs26_phy_setup (unit)
#define ag7100_phy_is_up(unit)          athrs26_phy_is_up (unit)
#define ag7100_phy_speed(unit)          athrs26_phy_speed (unit)
#define ag7100_phy_is_fdx(unit)         athrs26_phy_is_fdx (unit)

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  *link=ag7100_phy_is_up(unit);
  *fdx=ag7100_phy_is_fdx(unit);
  *speed=ag7100_phy_speed(unit);
  return 0;
}

#else
#define ag7100_phy_setup(unit) do { \
if(!unit) \
        athrs26_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif // Hydra or Howl
#endif

#ifdef CFG_ATHRS16_PHY

#include "../board/ar7100/common/athrs16_phy.h"

#define ag7100_phy_setup(unit)          athrs16_phy_setup (unit)
#define ag7100_phy_is_up(unit)          athrs16_phy_is_up (unit)
#define ag7100_phy_speed(unit)          athrs16_phy_speed (unit)
#define ag7100_phy_is_fdx(unit)         athrs16_phy_is_fdx (unit)
/*
#define ag7100_phy_ioctl(unit, args)    athr_ioctl(unit,args)
#define ag7100_phy_is_lan_pkt           athr_is_lan_pkt
#define ag7100_phy_set_pkt_port         athr_set_pkt_port
#define ag7100_phy_tag_len              ATHR_VLAN_TAG_SIZE
#define ag7100_phy_get_counters         athrs16_get_counters
*/

static inline unsigned int
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  *link=ag7100_phy_is_up(unit);
  *fdx=ag7100_phy_is_fdx(unit);
  *speed=ag7100_phy_speed(unit);
  return 0;
}

static inline int
ag7100_print_link_status(int unit)
{
  return -1;
}

#endif /* CFG_ATHRS16_PHY */

#ifdef CFG_VSC8201_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        vsc_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif

#ifdef CFG_VSC8601_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        vsc8601_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif

#ifdef CFG_VITESSE_8601_7395_PHY

#include "../board/ar7100/common/vsc73xx.h"

#define ag7100_phy_setup(unit) do { \
if(unit) \
	vsc73xx_setup(unit); \
else \
	vsc8601_phy_setup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(unit) \
	vsc73xx_get_link_status(unit, &link, &fdx, &speed,0); \
else \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(unit) \
	vsc73xx_get_link_status(unit, 0, &duplex, 0,0); \
else \
	duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(unit) \
	vsc73xx_get_link_status(unit, 0, 0, &speed,0); \
else \
	speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

static inline unsigned int
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
    if (0==unit) {
        ag7100_phy_link(unit,*link,*fdx,*speed);
        ag7100_phy_duplex(unit,*fdx);
        ag7100_phy_speed(unit,*speed);
    } else
        return vsc73xx_get_link_status(unit, link, fdx, speed, 0);

    return -1;
}

#endif


#ifdef CFG_IP175B_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        ip_phySetup(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=ip_phyIsUp(unit); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = ip_phyIsFullDuplex(unit); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = ip_phySpeed(unit); \
} while (0);

#endif

#ifdef CONFIG_ADMTEK_PHY

#define ag7100_phy_setup(unit) do { \
if(!unit) \
        miiphy_reset("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
if(!unit) \
        link=miiphy_link("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
if(!unit) \
        duplex = miiphy_duplex("eth0", CFG_PHY_ADDR); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
if(!unit) \
        speed = miiphy_speed("eth0", CFG_PHY_ADDR); \
} while (0);

#endif


#ifdef CFG_ATHRF1_PHY

#include "../board/ar7100/common/athr_phy.h"

#define ag7100_phy_setup(unit) do { \
	athr_phy_setup(unit); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
	duplex = athr_phy_is_fdx(unit); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
	speed = athr_phy_speed(unit); \
} while (0);

static inline unsigned int 
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  *link=athr_phy_is_up(unit); 
  if (*link==0)
    return 0;   
  *fdx=athr_phy_is_fdx(unit);
  *speed=athr_phy_speed(unit);
  return 0;
}

#define ag7100_phy_link(unit,link,fdx,speed) do { \
	ag7100_get_link_status(unit, &link, &fdx, &speed); \
} while (0);

#endif


#ifdef CFG_M88E1116_PHY

#include "../board/ar7100/common/m88e1116_phy.h"

#define ag7100_phy_setup(unit) do { \
        m88e1116_phy_setup(unit); \
} while (0);

#define ag7100_phy_duplex(unit,duplex) do { \
        duplex = m88e1116_phy_is_fdx(unit); \
} while (0);

#define ag7100_phy_speed(unit,speed) do { \
        speed = m88e1116_phy_speed(unit); \
} while (0);

#define ag7100_phy_link(unit,link,fdx,speed) do { \
        ag7100_get_link_status(unit, &link, &fdx, &speed); \
} while (0);

static inline unsigned int
ag7100_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
  *link=m88e1116_phy_is_up(unit);
  if (*link==0)
    return 0;
  *fdx=m88e1116_phy_is_fdx(unit);
  *speed=m88e1116_phy_speed(unit);
  return 0;
}

#endif

#endif /*_AG7100_PHY_H*/
