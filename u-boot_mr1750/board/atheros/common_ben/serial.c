#include <asm/addrspace.h>
#include <asm/types.h>
#include <config.h>
#include <atheros.h>

int serial_init(void)
{
//#if !defined(CONFIG_ATH_EMULATION)
	uint32_t div, val;

	div = ath_uart_freq() / (16 * CONFIG_BAUDRATE);
#ifdef CONFIG_SCO_SLAVE_CONNECTED 
	val = ath_reg_rd(GPIO_OE_ADDRESS) & (~0xcbf410u);
#else
	val = ath_reg_rd(GPIO_OE_ADDRESS) & (~0xcffc10u);
#endif
	ath_reg_wr(GPIO_OE_ADDRESS, val);

	ath_reg_rmw_set(GPIO_OUT_FUNCTION2_ADDRESS,
			GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_SET(0x16));

	ath_reg_rmw_clear(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0xff));

	ath_reg_rmw_set(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0x9));

	val = ath_reg_rd(GPIO_OUT_ADDRESS) | 0xcffc10u;
	ath_reg_wr(GPIO_OUT_ADDRESS, val);

	val = ath_reg_rd(GPIO_SPARE_ADDRESS);
	ath_reg_wr(GPIO_SPARE_ADDRESS, (val | 0x8402));

	ath_reg_wr(GPIO_OUT_ADDRESS, 0x2f);

	/*
	 * set DIAB bit
	 */
	ath_uart_wr(OFS_LINE_CONTROL, 0x80);

	/* set divisor */
	ath_uart_wr(OFS_DIVISOR_LSB, (div & 0xff));
	ath_uart_wr(OFS_DIVISOR_MSB, ((div >> 8) & 0xff));

	/* clear DIAB bit*/
	ath_uart_wr(OFS_LINE_CONTROL, 0x00);

	/* set data format */
	ath_uart_wr(OFS_DATA_FORMAT, 0x3);

	ath_uart_wr(OFS_INTR_ENABLE, 0);
//#endif
	return 0;
}

int serial_tstc (void)
{
	return(ath_uart_rd(OFS_LINE_STATUS) & 0x1);
}

u8 serial_getc(void)
{
	while(!serial_tstc());

	return ath_uart_rd(OFS_RCV_BUFFER);
}


void serial_putc(u8 byte)
{
	if (byte == '\n') serial_putc ('\r');

	while (((ath_uart_rd(OFS_LINE_STATUS)) & 0x20) == 0x0);

	ath_uart_wr(OFS_SEND_BUFFER, byte);
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
