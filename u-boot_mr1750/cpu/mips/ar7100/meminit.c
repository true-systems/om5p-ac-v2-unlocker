/* 
 * Memory controller config:
 * Assumes that the caches are initialized.
 *
 * 0) Figah out the Tap controller settings.
 * 1) Figure out whether the interface is 16bit or 32bit.
 * 2) Size the DRAM
 *
 *  0) Tap controller settings
 *  --------------------------
 * The Table below provides all possible values of TAP controllers. We need to
 * find the extreme left and extreme right of the spectrum (of max_udelay and
 * min_udelay). We then program the TAP to be in the middle.
 * Note for this we would need to be able to read and write memory. So, 
 * initially we assume that a 16bit interface, which will always work unless
 * there is exactly _1_ 32 bit part...for now we assume this is not the case.
 * 
 * The algo:
 * 0) Program the controller in 16bit mode.
 * 1) Start with the extreme left of the table
 * 2) Write 0xa4, 0xb5, 0xc6, 0xd7 to 0, 2, 4, 6
 * 3) Read 0 - this will fetch the entire cacheline.
 * 4) If the value at address 4 is good, record this table entry, goto 6
 * 5) Increment to get the next table entry. Goto 2.
 * 6) Start with extreme right. Do the same as above.
 *
 * 1) 16bit or 32bit
 * -----------------
 *  31st bit of reg 0x1800_0000 will  determine the mode. By default, 
 *  controller is set to 32-bit mode. In 32 bit mode, full data bus DQ [31:0] 
 *  will be used to write 32 bit data. Suppose you have 16bit DDR memory
 *  (it will have 16bit wide data bus). If you try to write 16 bit DDR in 32 
 *  bit mode, you are going to miss upper 16 bits of data. Reading to that 
 *  location will give you only lower 16 bits correctly, upper 16 bits will 
 *  have some junk value. E.g.,
 *
 *  write to 0x0000_0000 0x12345678
 *  write to 0x0000_1000 0x00000000 (just to discharge DQ[31:16] )
 *  read from 0x0000_0000
 *  if u see something like 0x0000_5678 (or XXXX_5678 but not equal to 
 *  0x12345678) - its a 16 bit interface
 *
 *  2) Size the DRAM
 *  -------------------
 *  DDR wraps around. Write a pattern to 0x0000_0000. Write an address 
 *  pattern at 4M, 8M, 16M etc. and check when 0x0000_0000 gets overwritten.
 *
 *
 *  We can use #define's for all these addresses and patterns but its easier
 *  to see what's going on without :)
 */
#include <common.h>
#include <asm/addrspace.h>
#include "ar7100_soc.h"


uint8_t     tap_settings[] = 
            {0x40, 0x41, 0x10, 0x12, 0x13, 0x15, 0x1a, 0x1c, 0x1f, 0x2f, 0x3f};

uint16_t    tap_pattern[] = {0xa5, 0xb6, 0xc7, 0xd8};

void
ar7100_ddr_tap_set(uint8_t set)
{
    ar7100_reg_wr_nf(AR7100_DDR_TAP_CONTROL0, set);
    ar7100_reg_wr_nf(AR7100_DDR_TAP_CONTROL1, set);
    ar7100_reg_wr_nf(AR7100_DDR_TAP_CONTROL2, set);
    ar7100_reg_wr_nf(AR7100_DDR_TAP_CONTROL3, set);
}
/*
 * We use 0xa0003000, because the first way is locked into cache to give
 * us a rudimentary stack
 */
void ar7100_ddr_tap_config(void)
{
    uint8_t set, prev, dummy;
    int i, j, k, floor, ceiling;
    uint16_t *p = 0xa0003000, *p_ca = 0x80003000;

    ar7100_reg_rmw_set(AR7100_DDR_CONFIG, AR7100_DDR_CONFIG_16BIT);

    for(i = 0; i < sizeof(tap_settings); i++) {
        set = tap_settings[i];
        ar7100_ddr_tap_set(set);

#ifndef COMPRESSED_UBOOT
        printf("trying %#x...", set);
#endif /* #ifndef COMPRESSED_UBOOT */

        for(j = 0; j < 4; j++)
            *(p + j) = tap_pattern[j];

        /*
         * get the cacheline
         */
        dummy = *p_ca;

        if ((*(p_ca + 2)) == tap_pattern[2]) {
            floor = i;
#ifndef COMPRESSED_UBOOT
            printf("worked; floor %d\n", i);
#endif /* #ifndef COMPRESSED_UBOOT */
            break;
        }
#ifndef COMPRESSED_UBOOT
        printf("didnt work written %#x read %#x\n", tap_pattern[2], 
                *(p_ca + 2));
#endif /* #ifndef COMPRESSED_UBOOT */
        flush_cache(p_ca, 8);
    }

    for(k = sizeof(tap_settings) - 1; k > i; k--) {
        set = tap_settings[k];
        ar7100_ddr_tap_set(set);

#ifndef COMPRESSED_UBOOT
        printf("trying %#x...", set);
#endif /* #ifndef COMPRESSED_UBOOT */
        for(j = 0; j < 4; j++)
            *(p + j) = tap_pattern[j];

        dummy = *p_ca;

        if ((*(p_ca + 2)) == tap_pattern[2]) {
            ceiling = k;
#ifndef COMPRESSED_UBOOT
            printf("worked; ceiling %d\n", k);
#endif /* #ifndef COMPRESSED_UBOOT */
            break;
        }
#ifndef COMPRESSED_UBOOT
        printf("didnt work written %#x read %#x\n", tap_pattern[2], 
                *(p_ca + 2));
#endif /* #ifndef COMPRESSED_UBOOT */
        flush_cache(p_ca, 8);
    }
    /*
     * If the min and max delay both worked, lets pick a default
     */
#ifndef COMPRESSED_UBOOT
    printf("floor %d ceil %d size-1 %d\n",
            floor, ceiling, (sizeof(tap_settings) - 1));
#endif /* #ifndef COMPRESSED_UBOOT */

    if ((floor == 0) && (ceiling == (sizeof(tap_settings) - 1))) 
        set = AR7100_DDR_TAP_DEFAULT;
    else {
        i = ((ceiling - floor)/2) + floor;
        set = tap_settings[i];
#ifndef COMPRESSED_UBOOT
        printf("i %d\n", i);
#endif /* #ifndef COMPRESSED_UBOOT */
    }

#ifndef COMPRESSED_UBOOT
    printf("Setting Tap to %#x\n", set);
#endif /* #ifndef COMPRESSED_UBOOT */
    ar7100_ddr_tap_set(set);
    udelay(1);
}

ar7100_ddr_width_t
ar7100_ddr_get_width()
{
    volatile uint32_t *p = 0xa0000000, *p1 = 0xa0001000;
    uint32_t pat = 0x12345678, rd;

    *p  = pat;
    *p1 = 0;
    asm("sync");
    /*
     * discharge DQ[31:16]
     */
    rd  = *p;

    if (rd == pat)
        return AR7100_DDR_32B;

    if ((rd & 0xffff) == (pat & 0xffff))
        return AR7100_DDR_16B_LOW;

    if ((rd & 0xffff0000) == (pat & 0xffff0000))
        return AR7100_DDR_16B_HIGH;

    /*
     * If Some'n's hosed this early cant do much; silence the compiler :)
     */
#ifndef COMPRESSED_UBOOT
    printf("something's wrong. rd %#x pat %#x\n", rd, pat);
#endif /* #ifndef COMPRESSED_UBOOT */

    return AR7100_DDR_32B;
}
 
/*
 * We check for size in 4M increments
 */
#define AR7100_DDR_SIZE_INCR    (4*1024*1024)

int sanity_mem_check(int mem_size) 
{
	int data=0xaaaa5555,i,ret=0;
        volatile unsigned int *ptr = KSEG1;

	for(i = 0; i < ((mem_size * AR7100_DDR_SIZE_INCR) / 4096) ; i++) {
		
		ptr[i] = (data ^ (1  << (i % 16)));
		 if(i % 16  == 0) 
			data = ~data;
	} 
        data = 0xaaaa5555;
        for(i = 0; i < ((mem_size * AR7100_DDR_SIZE_INCR) / 4096) ; i++) {

                if(ptr[i] != (data ^ (1  << (i % 16)))) {
			ret = 1;
			break;
		 }
                 if(i % 16 == 0)
                        data = ~data;
        }
	return ret;
}
int
ar7100_ddr_find_size()
{
    uint8_t  *p = KSEG1, pat = 0x77;
    int i, incr = AR7100_DDR_SIZE_INCR;

    *p = pat;

    for(i = 1; ; i++) {
        *(p + i * AR7100_DDR_SIZE_INCR) = (uint8_t)(i);
        if (*p != pat) {
            break;
        }
    }
    if(i < 4) {
	udelay(1000);
        for (;;) {
            ar7100_reg_wr(AR7100_RESET,
                        (AR7100_RESET_FULL_CHIP | AR7100_RESET_DDR));
        }
    }
    if(sanity_mem_check(i)) {
        udelay(1000);
        for (;;) {
            ar7100_reg_wr(AR7100_RESET,
                        (AR7100_RESET_FULL_CHIP | AR7100_RESET_DDR));
        }
    }
    return (i*AR7100_DDR_SIZE_INCR);
}

void
ar7100_ddr_initial_config(uint32_t refresh)
{
#ifndef COMPRESSED_UBOOT
    printf("\nsri . . ");
#endif /* #ifndef COMPRESSED_UBOOT */
#if 0
    ar7100_reg_wr(AR7100_RESET, AR7100_RESET_DDR);
    udelay(10);
#endif
#ifndef COMPRESSED_UBOOT
    ar7100_reg_wr_nf(AR7100_DDR_CONFIG, CFG_DDR_CONFIG_VAL);
    udelay(1000);
    ar7100_reg_wr_nf(AR7100_DDR_CONFIG2, CFG_DDR_CONFIG2_VAL);
    udelay(1000);
    ar7100_reg_wr_nf(AR7100_DDR_CONTROL, 0x8);
    udelay(1000);
#ifdef AR9100
    ar7100_reg_wr_nf(AR7100_DDR_MODE, CFG_DDR_MODE_VAL_INIT);
    udelay(1000);
#endif
    ar7100_reg_wr_nf(AR7100_DDR_CONTROL, 0x1);
    udelay(1000);
    ar7100_reg_wr_nf(AR7100_DDR_EXT_MODE, CFG_DDR_EXT_MODE_VAL);
    udelay(1000);
#endif /* #ifndef COMPRESSED_UBOOT */
    ar7100_reg_wr_nf(AR7100_DDR_CONTROL, 0x2);
    udelay(1000);
    ar7100_reg_wr_nf(AR7100_DDR_CONTROL, 0x8);
    udelay(1000);
    ar7100_reg_wr_nf(AR7100_DDR_MODE, CFG_DDR_MODE_VAL);
    udelay(10000);
    ar7100_reg_wr_nf(AR7100_DDR_CONTROL, 0x1);
    udelay(1000);
    ar7100_reg_wr_nf(AR7100_DDR_REFRESH, refresh);
    udelay(1000);
    ar7100_reg_wr_nf(AR7100_DDR_RD_DATA_THIS_CYCLE,CFG_DDR_RD_DATA_THIS_CYCLE_VAL);
    udelay(1000);
}





