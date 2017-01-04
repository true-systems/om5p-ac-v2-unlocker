#include <config.h>
#include <common.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include "ar7240_soc.h"

#define AR9285_BASE     0x10000000

#define AR9285_NUM_GPIO     10

#define AR9285_GPIO_IN_OUT                           0x4048 // GPIO input / output register
#define AR9285_GPIO_IN_VAL                           0x000FFC00
#define AR9285_GPIO_IN_VAL_S                         10

#define AR9285_GPIO_OE_OUT                           0x404c // GPIO output register
#define AR9285_GPIO_OE_OUT_DRV                       0x3    // 2 bit field mask, shifted by 2*bitpos
#define AR9285_GPIO_OE_OUT_DRV_NO                    0x0    // tristate
#define AR9285_GPIO_OE_OUT_DRV_LOW                   0x1    // drive if low
#define AR9285_GPIO_OE_OUT_DRV_HI                    0x2    // drive if high
#define AR9285_GPIO_OE_OUT_DRV_ALL                   0x3    // drive always

#define AR9285_GPIO_IN_ENABLE                        (AR9285_BASE + 0x4054)
#define AR9285_GPIO_JTAG_DISABLE                     (1u << 17)
#define AR9285_GPIO_OUTPUT_MUX1                      0x4060
#define AR9285_GPIO_OUTPUT_MUX2                      0x4064

#define AR9285_GPIO_OUTPUT_MUX_AS_OUTPUT             0
#define AR9285_GPIO_OUTPUT_MUX_AS_PCIE_ATTENTION_LED 1
#define AR9285_GPIO_OUTPUT_MUX_AS_PCIE_POWER_LED     2
#define AR9285_GPIO_OUTPUT_MUX_AS_TX_FRAME           3
#define AR9285_GPIO_OUTPUT_MUX_AS_RX_CLEAR_EXTERNAL  4
#define AR9285_GPIO_OUTPUT_MUX_AS_MAC_NETWORK_LED    5
#define AR9285_GPIO_OUTPUT_MUX_AS_MAC_POWER_LED      6


#define MS(_v, _f)  (((_v) & _f) >> _f##_S)

#define AR9285_GPIO_BIT(_gpio)  (1 << (_gpio))

#define ar9285_reg_wr(_off, _val)   ar7240_reg_wr(((_off) + AR9285_BASE), (_val))
#define ar9285_reg_rd(_off)   (ar7240_reg_rd((_off) + AR9285_BASE))
#define ar9285_reg_rmw(_off, _set, _clr)   do { \
            ar7240_reg_rmw_clear(((_off) + AR9285_BASE), (_clr)); \
            ar7240_reg_rmw_set(((_off) + AR9285_BASE), (_set)); \
} while(0)


/*
 * Configure GPIO Output Mux control
 */
static void
ar9285GpioCfgOutputMux(unsigned int gpio, u32 type)
{
    int          addr;
    u32    gpio_shift;

    // each MUX controls 6 GPIO pins
    if (gpio > 5) {
        addr = AR9285_GPIO_OUTPUT_MUX2;
    } else {
        addr = AR9285_GPIO_OUTPUT_MUX1;
    }

    // 5 bits per GPIO pin. Bits 0..4 for 1st pin in that mux, bits 5..9 for 2nd pin, etc.
    gpio_shift = (gpio % 6) * 5;

    ar9285_reg_rmw(addr, (type << gpio_shift), (0x1f << gpio_shift));
}

/*
 * Configure GPIO Output lines
 */
void
ar9285GpioCfgOutput(unsigned int gpio)
{
    u32    gpio_shift;

    if (gpio >= AR9285_NUM_GPIO) {
        printf("Invalid GPIO\n");
        return;
    }
    // Configure the MUX
    ar9285GpioCfgOutputMux(gpio, AR9285_GPIO_OUTPUT_MUX_AS_OUTPUT);

    // 2 bits per output mode
    gpio_shift = 2*gpio;

    ar9285_reg_rmw(AR9285_GPIO_OE_OUT, 
               (AR9285_GPIO_OE_OUT_DRV_ALL << gpio_shift), 
               (AR9285_GPIO_OE_OUT_DRV << gpio_shift));
}

/*
 * Configure GPIO Input lines
 */
void
ar9285GpioCfgInput(unsigned int gpio)
{
    u32    gpio_shift;

    if (gpio >= AR9285_NUM_GPIO) {
        printf("Invalid GPIO\n");
        return;
    }
    /* TODO: configure input mux for AR5416 */
    /* If configured as input, set output to tristate */
    gpio_shift = 2*gpio;

    ar9285_reg_rmw(AR9285_GPIO_OE_OUT, 
               (AR9285_GPIO_OE_OUT_DRV_NO << gpio_shift), 
               (AR9285_GPIO_OE_OUT_DRV << gpio_shift));
}

/*
 * Once configured for I/O - set output lines
 */
void
ar9285GpioSet(unsigned int gpio, int val)
{
    if (gpio >= AR9285_NUM_GPIO) {
        printf("Invalid GPIO\n");
        return;
    }

    ar9285_reg_rmw(AR9285_GPIO_IN_OUT, ((val&1) << gpio), AR9285_GPIO_BIT(gpio));
}

/*
 * Once configured for I/O - get input lines
 */
u32
ar9285GpioGet(unsigned int gpio)
{
    if (gpio >= AR9285_NUM_GPIO) {
        printf("Invalid GPIO\n");
        return 0xffffffff;
    }

    // Read output value for all gpio's, shift it left, and verify whether a 
    // specific gpio bit is set.
    return (MS(ar9285_reg_rd(AR9285_GPIO_IN_OUT), AR9285_GPIO_IN_VAL) & AR9285_GPIO_BIT(gpio)) != 0;
}

u32
ar9285DisableJtag(void)
{
	/*
	 * Disable ar9285's JTAG access. Else, gpio 0 - 4 will be used
	 * by the jtag. This does NOT affect board's JTAG interface
	 */
	ar7240_reg_rmw_set(AR9285_GPIO_IN_ENABLE, AR9285_GPIO_JTAG_DISABLE);
}
