#include <common.h>
#include <jffs2/jffs2.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include "ar7100_soc.h"
#include "ar7100_flash.h"

/*
 * globals
 */
flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

#define display(x)  ;

#define AR7100_SPI_CMD_WRITE_SR     0x01

/*
 * statics
 */
static void ar7100_spi_write_enable(void);
static void ar7100_spi_poll(void);
static void ar7100_spi_write_page(uint32_t addr, uint8_t *data, int len);
static void ar7100_spi_sector_erase(uint32_t addr);
static void ar7100_spi_flash_unblock();

static void
ar7100_spi_flash_unblock()
{
    ar7100_spi_write_enable();
    ar7100_spi_bit_banger(AR7100_SPI_CMD_WRITE_SR);
    ar7100_spi_bit_banger(0x0);
    ar7100_spi_go();
    ar7100_spi_poll();
}

static void
read_id()
{
    u32 rd = 0x777777;

    ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);
    ar7100_spi_bit_banger(0x9f);
    ar7100_spi_delay_8();
    ar7100_spi_delay_8();
    ar7100_spi_delay_8();
    ar7100_spi_done(); 
    /* rd = ar7100_reg_rd(AR7100_SPI_RD_STATUS); */
    rd = ar7100_reg_rd(AR7100_SPI_READ); 
    printf("id read %#x\n", rd);
}

unsigned long 
flash_init (void)
{
    int i;
    u32 rd = 0x666666;

    ar7100_reg_wr_nf(AR7100_SPI_CLOCK, 0x43);
    ar7100_spi_flash_unblock();
    read_id();
/*
    rd = ar7100_reg_rd(AR7100_SPI_RD_STATUS);
    printf ("rd = %x\n", rd);
    if (rd & 0x80) {
    }
*/

    /*
     * hook into board specific code to fill flash_info
     */
    return (flash_get_geom(&flash_info));
}


void flash_print_info (flash_info_t *info)
{
    printf("The hell do you want flinfo for??\n");
}

int
flash_erase(flash_info_t *info, int s_first, int s_last)
{
    int i, sector_size = info->size/info->sector_count;

    printf("\nFirst %#x last %#x sector size %#x\n",
           s_first, s_last, sector_size);

    for (i = s_first; i <= s_last; i++) {
        printf("\b\b\b\b%4d", i);
        ar7100_spi_sector_erase(i * sector_size);
    }
    ar7100_spi_done();
    printf("\n");

    return 0;
}

/*
 * Write a buffer from memory to flash:
 * 0. Assumption: Caller has already erased the appropriate sectors.
 * 1. call page programming for every 256 bytes
 */
int 
write_buff(flash_info_t *info, uchar *source, ulong addr, ulong len)
{
    int total = 0, len_this_lp, bytes_this_page;
    ulong dst;
    uchar *src;
    
    printf ("write addr: %x\n", addr); 
    addr = addr - CFG_FLASH_BASE;

    while(total < len) {
        src              = source + total;
        dst              = addr   + total;
        bytes_this_page  = AR7100_SPI_PAGE_SIZE - (addr % AR7100_SPI_PAGE_SIZE);
        len_this_lp      = ((len - total) > bytes_this_page) ? bytes_this_page
                                                             : (len - total);
        ar7100_spi_write_page(dst, src, len_this_lp);
        total += len_this_lp;
    }

    ar7100_spi_done();

    return 0;
}

static void
ar7100_spi_write_enable()  
{
    ar7100_reg_wr_nf(AR7100_SPI_FS, 1);                  
    ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);     
    ar7100_spi_bit_banger(AR7100_SPI_CMD_WREN);             
    ar7100_spi_go();
}

static void
ar7100_spi_poll()   
{
    int rd;                                                 

    do {
        ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);     
        ar7100_spi_bit_banger(AR7100_SPI_CMD_RD_STATUS);        
        ar7100_spi_delay_8();
        rd = (ar7100_reg_rd(AR7100_SPI_RD_STATUS) & 1);               
    }while(rd);
}

static void
ar7100_spi_write_page(uint32_t addr, uint8_t *data, int len)
{
    int i;
    uint8_t ch;

    display(0x77);
    ar7100_spi_write_enable();
    ar7100_spi_bit_banger(AR7100_SPI_CMD_PAGE_PROG);
    ar7100_spi_send_addr(addr);

    for(i = 0; i < len; i++) {
        ch = *(data + i);
        ar7100_spi_bit_banger(ch);
    }

    ar7100_spi_go();
    display(0x66);
    ar7100_spi_poll();
    display(0x6d);
}

static void
ar7100_spi_sector_erase(uint32_t addr)
{
    ar7100_spi_write_enable();
    ar7100_spi_bit_banger(AR7100_SPI_CMD_SECTOR_ERASE);
    ar7100_spi_send_addr(addr);
    ar7100_spi_go();
    display(0x7d);
    ar7100_spi_poll();
}


