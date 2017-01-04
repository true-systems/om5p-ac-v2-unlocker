/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _PHY_H
#define _PHY_H

#include <config.h>

#define ath_gmac_unit2name(_unit) (_unit ?  "eth1" : "eth0")

extern int ath_gmac_miiphy_read(char *devname, uint32_t phaddr, uint8_t reg, uint16_t *data);
extern int ath_gmac_miiphy_write(char *devname, uint32_t phaddr, uint8_t reg, uint16_t data);

#define phy_reg_read(base, addr, reg)	\
	ath_gmac_miiphy_read(ath_gmac_unit2name(base), addr, reg, NULL)

#define phy_reg_write(base, addr, reg, data)	\
	ath_gmac_miiphy_write(ath_gmac_unit2name(base), addr, reg, data)

#endif
