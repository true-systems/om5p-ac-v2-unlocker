#include <asm/addrspace.h>
#include <asm/types.h>
#include <config.h>
#include <ar7100_soc.h>

#define		REG_OFFSET		4

/* === END OF CONFIG === */

/* register offset */
#define         OFS_RCV_BUFFER          (0*REG_OFFSET)
#define         OFS_TRANS_HOLD          (0*REG_OFFSET)
#define         OFS_SEND_BUFFER         (0*REG_OFFSET)
#define         OFS_INTR_ENABLE         (1*REG_OFFSET)
#define         OFS_INTR_ID             (2*REG_OFFSET)
#define         OFS_DATA_FORMAT         (3*REG_OFFSET)
#define         OFS_LINE_CONTROL        (3*REG_OFFSET)
#define         OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define         OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define         OFS_LINE_STATUS         (5*REG_OFFSET)
#define         OFS_MODEM_STATUS        (6*REG_OFFSET)
#define         OFS_RS232_INPUT         (6*REG_OFFSET)
#define         OFS_SCRATCH_PAD         (7*REG_OFFSET)

#define         OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define         OFS_DIVISOR_MSB         (1*REG_OFFSET)

#define         MY_WRITE(y, z)  ((*((volatile u32*)(y))) = z)
#define         UART16550_READ(y)   ar7100_reg_rd((AR7100_UART_BASE+y))
#define         UART16550_WRITE(x, z)  ar7100_reg_wr((AR7100_UART_BASE+x), z)

void 
ar7100_sys_frequency(u32 *cpu_freq, u32 *ddr_freq, u32 *ahb_freq)
{
#ifndef AR9100
    u32 pll, pll_div, cpu_div, ahb_div, ddr_div, freq;

    pll = ar7100_reg_rd(AR7100_CPU_PLL_CONFIG);

    pll_div = 
        ((pll & PLL_CONFIG_PLL_FB_MASK) >> PLL_CONFIG_PLL_FB_SHIFT) + 1;

    cpu_div = 
        ((pll & PLL_CONFIG_CPU_DIV_MASK) >> PLL_CONFIG_CPU_DIV_SHIFT) + 1;

    ddr_div = 
        ((pll & PLL_CONFIG_DDR_DIV_MASK) >> PLL_CONFIG_DDR_DIV_SHIFT) + 1;

    ahb_div = 
       (((pll & PLL_CONFIG_AHB_DIV_MASK) >> PLL_CONFIG_AHB_DIV_SHIFT) + 1)*2;

    freq = pll_div * 40000000; 

    if (cpu_freq)
        *cpu_freq = freq/cpu_div;

    if (ddr_freq)
        *ddr_freq = freq/ddr_div;

    if (ahb_freq)
        *ahb_freq = (freq/cpu_div)/ahb_div;

    /*
    printf ("cpu_freq = %d\n", freq/cpu_div);
    printf ("ddr_freq = %d\n", freq/ddr_div);
    printf ("ahb_freq = %d\n", (freq/cpu_div)/ahb_div);
    */
#else
    u32 pll, pll_div, ahb_div, ddr_div, freq;

    pll = ar7100_reg_rd(AR7100_CPU_PLL_CONFIG);

    pll_div = 
        ((pll & PLL_CONFIG_PLL_FB_MASK) >> PLL_CONFIG_PLL_FB_SHIFT);

    ddr_div = 
        ((pll & PLL_CONFIG_DDR_DIV_MASK) >> PLL_CONFIG_DDR_DIV_SHIFT) + 1;

    ahb_div = 
       (((pll & PLL_CONFIG_AHB_DIV_MASK) >> PLL_CONFIG_AHB_DIV_SHIFT) + 1)*2;

    freq = pll_div * 5000000; 

    if (cpu_freq)
        *cpu_freq = freq;

    if (ddr_freq)
        *ddr_freq = freq/ddr_div;

    if (ahb_freq)
        *ahb_freq = freq/ahb_div;
#endif
}


void serial_init()
{
    u32 div;
    u32 ahb_freq = 100000000;

    ar7100_sys_frequency  (0, 0, &ahb_freq);  
    div  = ahb_freq/(16 * CONFIG_BAUDRATE);  

    /*
     * undocumented. confirm, why write to GPIO for uart?
     */
#if 0
    ar7100_reg_wr(AR7100_GPIO_OE, 0xcff);
    ar7100_reg_wr(AR7100_GPIO_OUT, 0x3b);

    MY_WRITE(0xb8040028, 0x100);
#else
    MY_WRITE(0xb8040000, 0x47f);
    MY_WRITE(0xb8040008, 0x7f);

    MY_WRITE(0xb8040028, 0x100);
#endif

    /* 
     * set DIAB bit 
     */
    UART16550_WRITE(OFS_LINE_CONTROL, 0x80);
        
    /* set divisor */
    /*UART16550_WRITE(OFS_DIVISOR_LSB, 0x04);
    UART16550_WRITE(OFS_DIVISOR_MSB, 0x01);*/
#if 1
    UART16550_WRITE(OFS_DIVISOR_LSB, (div & 0xff));
    UART16550_WRITE(OFS_DIVISOR_MSB, ((div >> 8) & 0xff));
#else
    UART16550_WRITE(OFS_DIVISOR_LSB, (0x36 & 0xff));
    UART16550_WRITE(OFS_DIVISOR_MSB, 0);
#endif

    /* clear DIAB bit*/ 
    UART16550_WRITE(OFS_LINE_CONTROL, 0x00);

    /* set data format */
    UART16550_WRITE(OFS_DATA_FORMAT, 0x3);

    UART16550_WRITE(OFS_INTR_ENABLE, 0);

    //WRITE(0xb8020008, 0x7);
    //WRITE(0xb8020010, 0x3);
}

int serial_tstc (void)
{
    return(UART16550_READ(OFS_LINE_STATUS) & 0x1);
}

u8 serial_getc()
{
    while(!serial_tstc());

    return UART16550_READ(OFS_RCV_BUFFER);
}


void serial_putc(u8 byte)
{
    if (byte == '\n') serial_putc ('\r');

    while (((UART16550_READ(OFS_LINE_STATUS)) & 0x20) == 0x0);
    UART16550_WRITE(OFS_SEND_BUFFER, byte);
}

void serial_setbrg (void)
{
}

void serial_puts (const char *s)
{
	while (*s)
	{
		serial_putc (*s++);
	}
}
