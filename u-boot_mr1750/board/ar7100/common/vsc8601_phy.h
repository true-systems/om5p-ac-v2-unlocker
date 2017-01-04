/* vsc8601.h
 *
 * History:
 * Jan 14, 2007 wclewis ready common BDI/ECOS/Linux
 */

#ifndef _VSC8601_PHY_H
#define _VSC8601_PHY_H

#ifndef CEXTERN
#define  CEXTERN static inline
#endif

int vsc8601_phy_setup(int unit);

unsigned int 
vsc8601_phy_get_link_status(int unit, int *link, int *fdx, int *speed, unsigned int *cfg);

#endif
