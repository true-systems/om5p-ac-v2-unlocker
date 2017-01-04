/*
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
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

/*
 * SPI Read/Write Utilities
 */

#include <common.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_ETHREG)

/*-----------------------------------------------------------------------
 * Definitions
 */
#ifdef CFG_ATHRS26_PHY
extern unsigned int s26_rd_phy(unsigned int phy_addr, unsigned int reg_addr);
extern void s26_wr_phy(unsigned int phy_addr, unsigned int reg_addr, unsigned int write_data);
extern uint32_t athrs26_reg_read(uint32_t reg_addr);
extern void athrs26_reg_write(uint32_t reg_addr, uint32_t reg_val);
#endif

#ifdef CFG_ATHRS27_PHY
extern unsigned int s27_rd_phy(unsigned int phy_addr, unsigned int reg_addr);
extern void s27_wr_phy(unsigned int phy_addr, unsigned int reg_addr, unsigned int write_data);
extern uint32_t athrs27_reg_read(uint32_t reg_addr);
extern void athrs27_reg_write(uint32_t reg_addr, uint32_t reg_val);
#endif

#ifdef CONFIG_AR7242_S16_PHY
extern uint32_t athrs16_reg_read(uint32_t reg_addr);
extern void athrs16_reg_write(uint32_t reg_addr, uint32_t reg_val);
extern int  ag7240_miiphy_read(char *devname, uint32_t phaddr,
                uint8_t reg);
extern int  ag7240_miiphy_write(char *devname, uint32_t phaddr,
                uint8_t reg, uint16_t data);
#endif


#define READ_MAC  0x01
#define WRITE_MAC 0x02
#define READ_PHY  0x10
#define WRITE_PHY 0x20

/*
 * Values from last command.
 */
static int   reg;
static int   val,rd_value;
static int phyaddr;
static int portnum;

int do_ethreg (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int   rcode = 0;

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */

	if ((flag & CMD_FLAG_REPEAT) == 0)
	{
		if (argc == 2) {
			reg = simple_strtoul(argv[1], NULL, 10);
			rcode = READ_MAC;
		}
		if (argc == 3) {
			reg = simple_strtoul(argv[1], NULL, 10);
			val = simple_strtoul(argv[2],NULL,10);
			rcode = WRITE_MAC;
		}
		if (argc == 4) {
    	                if(*argv[1] == 'p') {
				portnum = simple_strtoul(argv[2], NULL, 10);
				reg = simple_strtoul(argv[3],NULL,10);
				rcode = READ_PHY; 
			}
			else
			    return 1;
		}
        	if (argc == 5) {
    	                if(*argv[1] == 'p') {
				portnum = simple_strtoul(argv[2], NULL, 10);
				reg = simple_strtoul(argv[3],NULL,10);
				val = simple_strtoul(argv[4],NULL,10);
				rcode = WRITE_PHY; 
			}
			else
			    return 1;
		}

		if ((argc > 4) && (argc < 2))
			return 1;
	}
#ifdef CONFIG_AR7242_S16_PHY
   if(rcode == READ_PHY) {
   	rd_value = ag7240_miiphy_read("eth0",portnum,reg);
        printf("Read Reg: 0x%08x = 0x%08x\n",reg,rd_value);
   }
   else if(rcode == READ_MAC) {
        rd_value = athrs16_reg_read(reg);
        printf("Read Reg: 0x%08x = 0x%08x\n",reg,rd_value);
   }
   else if(rcode  == WRITE_PHY) {
   	rd_value = ag7240_miiphy_read("eth0",portnum,reg);
        ag7240_miiphy_write("eth0",portnum,reg,val);
        printf("Write Reg: 0x%08x: Oldval = 0x%08x Newval = 0x%08x\n", reg, rd_value, val);
   }
   else if(rcode == WRITE_MAC) {
        rd_value = athrs16_reg_read(reg);
        athrs16_reg_write(reg,val);
        printf("Write Reg: 0x%08x: Oldval = 0x%08x Newval = 0x%08x\n", reg, rd_value, val);
   }
   else
	return 1;
#endif
#ifdef CFG_ATHRS26_PHY        
   if(rcode == READ_PHY) {
   	rd_value = s26_rd_phy(portnum,reg);
        printf("Read Reg: 0x%08x = 0x%08x\n",reg,rd_value);
   }
   else if(rcode == READ_MAC) {
        rd_value = athrs26_reg_read(reg);
        printf("Read Reg: 0x%08x = 0x%08x\n",reg,rd_value);
   }
   else if(rcode  == WRITE_PHY) {
   	rd_value = s26_rd_phy(portnum,reg);
        s26_wr_phy(portnum,reg,val);
        printf("Write Reg: 0x%08x: Oldval = 0x%08x Newval = 0x%08x\n", reg, rd_value, val);
   }
   else if(rcode == WRITE_MAC) {
        rd_value = athrs26_reg_read(reg);
        athrs26_reg_write(reg,val);
        printf("Write Reg: 0x%08x: Oldval = 0x%08x Newval = 0x%08x\n", reg, rd_value, val);
   }
   else
	return 1;
#endif
#ifdef CFG_ATHRS27_PHY        
   if(rcode == READ_PHY) {
   	rd_value = s27_rd_phy(portnum,reg);
        printf("Read Reg: 0x%08x = 0x%08x\n",reg,rd_value);
   }
   else if(rcode == READ_MAC) {
        rd_value = athrs27_reg_read(reg);
        printf("Read Reg: 0x%08x = 0x%08x\n",reg,rd_value);
   }
   else if(rcode  == WRITE_PHY) {
   	rd_value = s27_rd_phy(portnum,reg);
        s27_wr_phy(portnum,reg,val);
        printf("Write Reg: 0x%08x: Oldval = 0x%08x Newval = 0x%08x\n", reg, rd_value, val);
   }
   else if(rcode == WRITE_MAC) {
        rd_value = athrs27_reg_read(reg);
        athrs27_reg_write(reg,val);
        printf("Write Reg: 0x%08x: Oldval = 0x%08x Newval = 0x%08x\n", reg, rd_value, val);
   }
   else
	return 1;
#endif
   return 0;
}

/***************************************************/

U_BOOT_CMD(
	ethreg,	6,	1,	do_ethreg,
	"ethreg    - S26 PHY Reg rd/wr  utility\n",
	"<p> <phyaddr> <reg> <value> - Send <bit_len> bits from <dout> out the SPI\n"
	"<p>  - operates on the phy; by default is rd/wr s26 mac registers\n"
	"<phyaddr> - Address of the phy\n"
	"<reg>    - Register offset\n"
	"<value>  - value to write\n"
);

#endif	/* CFG_CMD_SPI */
