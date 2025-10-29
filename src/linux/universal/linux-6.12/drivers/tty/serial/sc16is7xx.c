// SPDX-License-Identifier: GPL-2.0+
/*
 * SC16IS7xx tty serial driver - common code
 *
 * Copyright (C) 2014 GridPoint
 * Author: Jon Ringle <jringle@gridpoint.com>
 * Based on max310x.c, by Alexander Shiyan <shc_work@mail.ru>
 */

#undef DEFAULT_SYMBOL_NAMESPACE
#define DEFAULT_SYMBOL_NAMESPACE "SERIAL_NXP_SC16IS7XX"

#include <linux/bits.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/export.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/idr.h>
#include <linux/kthread.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/sched.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/string.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/uaccess.h>
#include <linux/units.h>

#include "sc16is7xx.h"

#define SC16IS7XX_MAX_DEVS		8

/* SC16IS7XX register definitions */
#define SC16IS7XX_RHR_REG		(0x00) /* RX FIFO */
#define SC16IS7XX_THR_REG		(0x00) /* TX FIFO */
#define SC16IS7XX_IER_REG		(0x01) /* Interrupt enable */
#define SC16IS7XX_IIR_REG		(0x02) /* Interrupt Identification */
#define SC16IS7XX_FCR_REG		(0x02) /* FIFO control */
#define SC16IS7XX_LCR_REG		(0x03) /* Line Control */
#define SC16IS7XX_MCR_REG		(0x04) /* Modem Control */
#define SC16IS7XX_LSR_REG		(0x05) /* Line Status */
#define SC16IS7XX_MSR_REG		(0x06) /* Modem Status */
#define SC16IS7XX_SPR_REG		(0x07) /* Scratch Pad */
#define SC16IS7XX_TXLVL_REG		(0x08) /* TX FIFO level */
#define SC16IS7XX_RXLVL_REG		(0x09) /* RX FIFO level */
#define SC16IS7XX_IODIR_REG		(0x0a) /* I/O Direction
						* - only on 75x/76x
						*/
#define SC16IS7XX_IOSTATE_REG		(0x0b) /* I/O State
						* - only on 75x/76x
						*/
#define SC16IS7XX_IOINTENA_REG		(0x0c) /* I/O Interrupt Enable
						* - only on 75x/76x
						*/
#define SC16IS7XX_IOCONTROL_REG		(0x0e) /* I/O Control
						* - only on 75x/76x
						*/
#define SC16IS7XX_EFCR_REG		(0x0f) /* Extra Features Control */

/* TCR/TLR Register set: Only if ((MCR[2] == 1) && (EFR[4] == 1)) */
#define SC16IS7XX_TCR_REG		(0x06) /* Transmit control */
#define SC16IS7XX_TLR_REG		(0x07) /* Trigger level */

/* Special Register set: Only if ((LCR[7] == 1) && (LCR != 0xBF)) */
#define SC16IS7XX_DLL_REG		(0x00) /* Divisor Latch Low */
#define SC16IS7XX_DLH_REG		(0x01) /* Divisor Latch High */

/* Enhanced Register set: Only if (LCR == 0xBF) */
#define SC16IS7XX_EFR_REG		(0x02) /* Enhanced Features */
#define SC16IS7XX_XON1_REG		(0x04) /* Xon1 word */
#define SC16IS7XX_XON2_REG		(0x05) /* Xon2 word */
#define SC16IS7XX_XOFF1_REG		(0x06) /* Xoff1 word */
#define SC16IS7XX_XOFF2_REG		(0x07) /* Xoff2 word */

/* IER register bits */
#define SC16IS7XX_IER_RDI_BIT		BIT(0)   /* Enable RX data interrupt */
#define SC16IS7XX_IER_THRI_BIT		BIT(1)   /* Enable TX holding register
						  * interrupt */
#define SC16IS7XX_IER_RLSI_BIT		BIT(2)   /* Enable RX line status
						  * interrupt */
#define SC16IS7XX_IER_MSI_BIT		BIT(3)   /* Enable Modem status
						  * interrupt */

/* IER register bits - write only if (EFR[4] == 1) */
#define SC16IS7XX_IER_SLEEP_BIT		BIT(4)   /* Enable Sleep mode */
#define SC16IS7XX_IER_XOFFI_BIT		BIT(5)   /* Enable Xoff interrupt */
#define SC16IS7XX_IER_RTSI_BIT		BIT(6)   /* Enable nRTS interrupt */
#define SC16IS7XX_IER_CTSI_BIT		BIT(7)   /* Enable nCTS interrupt */

/* FCR register bits */
#define SC16IS7XX_FCR_FIFO_BIT		BIT(0)   /* Enable FIFO */
#define SC16IS7XX_FCR_RXRESET_BIT	BIT(1)   /* Reset RX FIFO */
#define SC16IS7XX_FCR_TXRESET_BIT	BIT(2)   /* Reset TX FIFO */
#define SC16IS7XX_FCR_RXLVLL_BIT	BIT(6)   /* RX Trigger level LSB */
#define SC16IS7XX_FCR_RXLVLH_BIT	BIT(7)   /* RX Trigger level MSB */

/* FCR register bits - write only if (EFR[4] == 1) */
#define SC16IS7XX_FCR_TXLVLL_BIT	BIT(4)   /* TX Trigger level LSB */
#define SC16IS7XX_FCR_TXLVLH_BIT	BIT(5)   /* TX Trigger level MSB */

/* IIR register bits */
#define SC16IS7XX_IIR_NO_INT_BIT	0x01		/* No interrupts pending */
#define SC16IS7XX_IIR_ID_MASK		GENMASK(5, 1)	/* Mask for the interrupt ID */
#define SC16IS7XX_IIR_THRI_SRC		0x02		/* TX holding register empty */
#define SC16IS7XX_IIR_RDI_SRC		0x04		/* RX data interrupt */
#define SC16IS7XX_IIR_RLSE_SRC		0x06		/* RX line status error */
#define SC16IS7XX_IIR_RTOI_SRC		0x0c		/* RX time-out interrupt */
#define SC16IS7XX_IIR_MSI_SRC		0x00		/* Modem status interrupt
							 * - only on 75x/76x
							 */
#define SC16IS7XX_IIR_INPIN_SRC		0x30		/* Input pin change of state
							 * - only on 75x/76x
							 */
#define SC16IS7XX_IIR_XOFFI_SRC		0x10		/* Received Xoff */
#define SC16IS7XX_IIR_CTSRTS_SRC	0x20		/* nCTS,nRTS change of state
							 * from active (LOW)
							 * to inactive (HIGH)
							 */
/* LCR register bits */
#define SC16IS7XX_LCR_LENGTH0_BIT	BIT(0)   /* Word length bit 0 */
#define SC16IS7XX_LCR_LENGTH1_BIT	BIT(1)   /* Word length bit 1
						  *
						  * Word length bits table:
						  * 00 -> 5 bit words
						  * 01 -> 6 bit words
						  * 10 -> 7 bit words
						  * 11 -> 8 bit words
						  */
#define SC16IS7XX_LCR_STOPLEN_BIT	BIT(2)   /* STOP length bit
						  *
						  * STOP length bit table:
						  * 0 -> 1 stop bit
						  * 1 -> 1-1.5 stop bits if
						  *      word length is 5,
						  *      2 stop bits otherwise
						  */
#define SC16IS7XX_LCR_PARITY_BIT	BIT(3)   /* Parity bit enable */
#define SC16IS7XX_LCR_EVENPARITY_BIT	BIT(4)   /* Even parity bit enable */
#define SC16IS7XX_LCR_FORCEPARITY_BIT	BIT(5)   /* 9-bit multidrop parity */
#define SC16IS7XX_LCR_TXBREAK_BIT	BIT(6)   /* TX break enable */
#define SC16IS7XX_LCR_DLAB_BIT		BIT(7)   /* Divisor Latch enable */
#define SC16IS7XX_LCR_WORD_LEN_5	(0x00)
#define SC16IS7XX_LCR_WORD_LEN_6	(0x01)
#define SC16IS7XX_LCR_WORD_LEN_7	(0x02)
#define SC16IS7XX_LCR_WORD_LEN_8	(0x03)
#define SC16IS7XX_LCR_CONF_MODE_A	SC16IS7XX_LCR_DLAB_BIT /* Special
								* reg set */
#define SC16IS7XX_LCR_CONF_MODE_B	0xBF                   /* Enhanced
								* reg set */

/* MCR register bits */
#define SC16IS7XX_MCR_DTR_BIT		BIT(0)   /* DTR complement
						  * - only on 75x/76x
						  */
#define SC16IS7XX_MCR_RTS_BIT		BIT(1)   /* RTS complement */
#define SC16IS7XX_MCR_TCRTLR_BIT	BIT(2)   /* TCR/TLR register enable */
#define SC16IS7XX_MCR_LOOP_BIT		BIT(4)   /* Enable loopback test mode */
#define SC16IS7XX_MCR_XONANY_BIT	BIT(5)   /* Enable Xon Any
						  * - write enabled
						  * if (EFR[4] == 1)
						  */
#define SC16IS7XX_MCR_IRDA_BIT		BIT(6)   /* Enable IrDA mode
						  * - write enabled
						  * if (EFR[4] == 1)
						  */
#define SC16IS7XX_MCR_CLKSEL_BIT	BIT(7)   /* Divide clock by 4
						  * - write enabled
						  * if (EFR[4] == 1)
						  */

/* LSR register bits */
#define SC16IS7XX_LSR_DR_BIT		BIT(0)   /* Receiver data ready */
#define SC16IS7XX_LSR_OE_BIT		BIT(1)   /* Overrun Error */
#define SC16IS7XX_LSR_PE_BIT		BIT(2)   /* Parity Error */
#define SC16IS7XX_LSR_FE_BIT		BIT(3)   /* Frame Error */
#define SC16IS7XX_LSR_BI_BIT		BIT(4)   /* Break Interrupt */
#define SC16IS7XX_LSR_BRK_ERROR_MASK \
	(SC16IS7XX_LSR_OE_BIT | \
	 SC16IS7XX_LSR_PE_BIT | \
	 SC16IS7XX_LSR_FE_BIT | \
	 SC16IS7XX_LSR_BI_BIT)

#define SC16IS7XX_LSR_THRE_BIT		BIT(5)   /* TX holding register empty */
#define SC16IS7XX_LSR_TEMT_BIT		BIT(6)   /* Transmitter empty */
#define SC16IS7XX_LSR_FIFOE_BIT		BIT(7)   /* Fifo Error */

/* MSR register bits */
#define SC16IS7XX_MSR_DCTS_BIT		BIT(0)   /* Delta CTS Clear To Send */
#define SC16IS7XX_MSR_DDSR_BIT		BIT(1)   /* Delta DSR Data Set Ready
						  * or (IO4)
						  * - only on 75x/76x
						  */
#define SC16IS7XX_MSR_DRI_BIT		BIT(2)   /* Delta RI Ring Indicator
						  * or (IO7)
						  * - only on 75x/76x
						  */
#define SC16IS7XX_MSR_DCD_BIT		BIT(3)   /* Delta CD Carrier Detect
						  * or (IO6)
						  * - only on 75x/76x
						  */
#define SC16IS7XX_MSR_CTS_BIT		BIT(4)   /* CTS */
#define SC16IS7XX_MSR_DSR_BIT		BIT(5)   /* DSR (IO4)
						  * - only on 75x/76x
						  */
#define SC16IS7XX_MSR_RI_BIT		BIT(6)   /* RI (IO7)
						  * - only on 75x/76x
						  */
#define SC16IS7XX_MSR_CD_BIT		BIT(7)   /* CD (IO6)
						  * - only on 75x/76x
						  */

/*
 * TCR register bits
 * TCR trigger levels are available from 0 to 60 characters with a granularity
 * of four.
 * The programmer must program the TCR such that TCR[3:0] > TCR[7:4]. There is
 * no built-in hardware check to make sure this condition is met. Also, the TCR
 * must be programmed with this condition before auto RTS or software flow
 * control is enabled to avoid spurious operation of the device.
 */
#define SC16IS7XX_TCR_RX_HALT(words)	((((words) / 4) & 0x0f) << 0)
#define SC16IS7XX_TCR_RX_RESUME(words)	((((words) / 4) & 0x0f) << 4)

/*
 * TLR register bits
 * If TLR[3:0] or TLR[7:4] are logical 0, the selectable trigger levels via the
 * FIFO Control Register (FCR) are used for the transmit and receive FIFO
 * trigger levels. Trigger levels from 4 characters to 60 characters are
 * available with a granularity of four.
 *
 * When the trigger level setting in TLR is zero, the SC16IS74x/75x/76x uses the
 * trigger level setting defined in FCR. If TLR has non-zero trigger level value
 * the trigger level defined in FCR is discarded. This applies to both transmit
 * FIFO and receive FIFO trigger level setting.
 *
 * When TLR is used for RX trigger level control, FCR[7:6] should be left at the
 * default state, that is, '00'.
 */
#define SC16IS7XX_TLR_TX_TRIGGER(words)	((((words) / 4) & 0x0f) << 0)
#define SC16IS7XX_TLR_RX_TRIGGER(words)	((((words) / 4) & 0x0f) << 4)

/* IOControl register bits (Only 75x/76x) */
#define SC16IS7XX_IOCONTROL_LATCH_BIT	BIT(0)   /* Enable input latching */
#define SC16IS7XX_IOCONTROL_MODEM_A_BIT	BIT(1)   /* Enable GPIO[7:4] as modem A pins */
#define SC16IS7XX_IOCONTROL_MODEM_B_BIT	BIT(2)   /* Enable GPIO[3:0] as modem B pins */
#define SC16IS7XX_IOCONTROL_SRESET_BIT	BIT(3)   /* Software Reset */

/* EFCR register bits */
#define SC16IS7XX_EFCR_9BIT_MODE_BIT	BIT(0)   /* Enable 9-bit or Multidrop
						  * mode (RS485) */
#define SC16IS7XX_EFCR_RXDISABLE_BIT	BIT(1)   /* Disable receiver */
#define SC16IS7XX_EFCR_TXDISABLE_BIT	BIT(2)   /* Disable transmitter */
#define SC16IS7XX_EFCR_AUTO_RS485_BIT	BIT(4)   /* Auto RS485 RTS direction */
#define SC16IS7XX_EFCR_RTS_INVERT_BIT	BIT(5)   /* RTS output inversion */
#define SC16IS7XX_EFCR_IRDA_MODE_BIT	BIT(7)   /* IrDA mode
						  * 0 = rate upto 115.2 kbit/s
						  *   - Only 75x/76x
						  * 1 = rate upto 1.152 Mbit/s
						  *   - Only 76x
						  */

/* EFR register bits */
#define SC16IS7XX_EFR_AUTORTS_BIT	BIT(6)   /* Auto RTS flow ctrl enable */
#define SC16IS7XX_EFR_AUTOCTS_BIT	BIT(7)   /* Auto CTS flow ctrl enable */
#define SC16IS7XX_EFR_XOFF2_DETECT_BIT	BIT(5)   /* Enable Xoff2 detection */
#define SC16IS7XX_EFR_ENABLE_BIT	BIT(4)   /* Enable enhanced functions
						  * and writing to IER[7:4],
						  * FCR[5:4], MCR[7:5]
						  */
#define SC16IS7XX_EFR_SWFLOW3_BIT	BIT(3)
#define SC16IS7XX_EFR_SWFLOW2_BIT	BIT(2)
						 /*
						  * SWFLOW bits 3 & 2 table:
						  * 00 -> no transmitter flow
						  *       control
						  * 01 -> transmitter generates
						  *       XON2 and XOFF2
						  * 10 -> transmitter generates
						  *       XON1 and XOFF1
						  * 11 -> transmitter generates
						  *       XON1, XON2, XOFF1 and
						  *       XOFF2
						  */
#define SC16IS7XX_EFR_SWFLOW1_BIT	BIT(1)
#define SC16IS7XX_EFR_SWFLOW0_BIT	BIT(0)
						 /*
						  * SWFLOW bits 1 & 0 table:
						  * 00 -> no received flow
						  *       control
						  * 01 -> receiver compares
						  *       XON2 and XOFF2
						  * 10 -> receiver compares
						  *       XON1 and XOFF1
						  * 11 -> receiver compares
						  *       XON1, XON2, XOFF1 and
						  *       XOFF2
						  */
#define SC16IS7XX_EFR_FLOWCTRL_BITS	(SC16IS7XX_EFR_AUTORTS_BIT | \
					SC16IS7XX_EFR_AUTOCTS_BIT | \
					SC16IS7XX_EFR_XOFF2_DETECT_BIT | \
					SC16IS7XX_EFR_SWFLOW3_BIT | \
					SC16IS7XX_EFR_SWFLOW2_BIT | \
					SC16IS7XX_EFR_SWFLOW1_BIT | \
					SC16IS7XX_EFR_SWFLOW0_BIT)


/* Misc definitions */
#define SC16IS7XX_FIFO_SIZE		(64)
#define SC16IS7XX_GPIOS_PER_BANK	4

#define SC16IS7XX_RECONF_MD		BIT(0)
#define SC16IS7XX_RECONF_IER		BIT(1)
#define SC16IS7XX_RECONF_RS485		BIT(2)

struct sc16is7xx_one_config {
	unsigned int			flags;
	u8				ier_mask;
	u8				ier_val;
};

struct sc16is7xx_one {
	struct uart_port		port;
	struct regmap			*regmap;
	struct mutex			efr_lock; /* EFR registers access */
	struct kthread_work		tx_work;
	struct kthread_work		reg_work;
	struct kthread_delayed_work	ms_work;
	struct sc16is7xx_one_config	config;
	unsigned char			buf[SC16IS7XX_FIFO_SIZE]; /* Rx buffer. */
	unsigned int			old_mctrl;
	u8				old_lcr; /* Value before EFR access. */
	bool				irda_mode;
};

struct sc16is7xx_port {
	const struct sc16is7xx_devtype	*devtype;
	struct clk			*clk;
#ifdef CONFIG_GPIOLIB
	struct gpio_chip		gpio;
	unsigned long			gpio_valid_mask;
#endif
	u8				mctrl_mask;
	struct kthread_worker		kworker;
	struct task_struct		*kworker_task;
	struct sc16is7xx_one		p[];
};

static DEFINE_IDA(sc16is7xx_lines);

static struct uart_driver sc16is7xx_uart = {
	.owner		= THIS_MODULE,
	.driver_name    = SC16IS7XX_NAME,
	.dev_name	= "ttySC",
	.nr		= SC16IS7XX_MAX_DEVS,
};

#define to_sc16is7xx_one(p,e)	((container_of((p), struct sc16is7xx_one, e)))

static u8 sc16is7xx_port_read(struct uart_port *port, u8 reg)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);
	unsigned int val = 0;

	regmap_read(one->regmap, reg, &val);

	return val;
}

static void sc16is7xx_port_write(struct uart_port *port, u8 reg, u8 val)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	regmap_write(one->regmap, reg, val);
}

static void sc16is7xx_fifo_read(struct uart_port *port, u8 *rxbuf, unsigned int rxlen)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	regmap_noinc_read(one->regmap, SC16IS7XX_RHR_REG, rxbuf, rxlen);
}

static void sc16is7xx_fifo_write(struct uart_port *port, u8 *txbuf, u8 to_send)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	/*
	 * Don't send zero-length data, at least on SPI it confuses the chip
	 * delivering wrong TXLVL data.
	 */
	if (unlikely(!to_send))
		return;

	regmap_noinc_write(one->regmap, SC16IS7XX_THR_REG, txbuf, to_send);
}

static void sc16is7xx_port_update(struct uart_port *port, u8 reg,
				  u8 mask, u8 val)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	regmap_update_bits(one->regmap, reg, mask, val);
}

static void sc16is7xx_power(struct uart_port *port, int on)
{
	sc16is7xx_port_update(port, SC16IS7XX_IER_REG,
			      SC16IS7XX_IER_SLEEP_BIT,
			      on ? 0 : SC16IS7XX_IER_SLEEP_BIT);
}

/*
 * In an amazing feat of design, the Enhanced Features Register (EFR)
 * shares the address of the Interrupt Identification Register (IIR).
 * Access to EFR is switched on by writing a magic value (0xbf) to the
 * Line Control Register (LCR). Any interrupt firing during this time will
 * see the EFR where it expects the IIR to be, leading to
 * "Unexpected interrupt" messages.
 *
 * Prevent this possibility by claiming a mutex while accessing the EFR,
 * and claiming the same mutex from within the interrupt handler. This is
 * similar to disabling the interrupt, but that doesn't work because the
 * bulk of the interrupt processing is run as a workqueue job in thread
 * context.
 */
static void sc16is7xx_efr_lock(struct uart_port *port)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	mutex_lock(&one->efr_lock);

	/* Backup content of LCR. */
	one->old_lcr = sc16is7xx_port_read(port, SC16IS7XX_LCR_REG);

	/* Enable access to Enhanced register set */
	sc16is7xx_port_write(port, SC16IS7XX_LCR_REG, SC16IS7XX_LCR_CONF_MODE_B);

	/* Disable cache updates when writing to EFR registers */
	regcache_cache_bypass(one->regmap, true);
}

static void sc16is7xx_efr_unlock(struct uart_port *port)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	/* Re-enable cache updates when writing to normal registers */
	regcache_cache_bypass(one->regmap, false);

	/* Restore original content of LCR */
	sc16is7xx_port_write(port, SC16IS7XX_LCR_REG, one->old_lcr);

	mutex_unlock(&one->efr_lock);
}

static void sc16is7xx_ier_clear(struct uart_port *port, u8 bit)
{
	struct sc16is7xx_port *s = dev_get_drvdata(port->dev);
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	lockdep_assert_held_once(&port->lock);

	one->config.flags |= SC16IS7XX_RECONF_IER;
	one->config.ier_mask |= bit;
	one->config.ier_val &= ~bit;
	kthread_queue_work(&s->kworker, &one->reg_work);
}

static void sc16is7xx_ier_set(struct uart_port *port, u8 bit)
{
	struct sc16is7xx_port *s = dev_get_drvdata(port->dev);
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	lockdep_assert_held_once(&port->lock);

	one->config.flags |= SC16IS7XX_RECONF_IER;
	one->config.ier_mask |= bit;
	one->config.ier_val |= bit;
	kthread_queue_work(&s->kworker, &one->reg_work);
}

static void sc16is7xx_stop_tx(struct uart_port *port)
{
	sc16is7xx_ier_clear(port, SC16IS7XX_IER_THRI_BIT);
}

static void sc16is7xx_stop_rx(struct uart_port *port)
{
	sc16is7xx_ier_clear(port, SC16IS7XX_IER_RDI_BIT);
}

const struct sc16is7xx_devtype sc16is74x_devtype = {
	.name		= "SC16IS74X",
	.nr_gpio	= 0,
	.nr_uart	= 1,
};
EXPORT_SYMBOL_GPL(sc16is74x_devtype);

const struct sc16is7xx_devtype sc16is750_devtype = {
	.name		= "SC16IS750",
	.nr_gpio	= 8,
	.nr_uart	= 1,
};
EXPORT_SYMBOL_GPL(sc16is750_devtype);

const struct sc16is7xx_devtype sc16is752_devtype = {
	.name		= "SC16IS752",
	.nr_gpio	= 8,
	.nr_uart	= 2,
};
EXPORT_SYMBOL_GPL(sc16is752_devtype);

const struct sc16is7xx_devtype sc16is760_devtype = {
	.name		= "SC16IS760",
	.nr_gpio	= 8,
	.nr_uart	= 1,
};
EXPORT_SYMBOL_GPL(sc16is760_devtype);

const struct sc16is7xx_devtype sc16is762_devtype = {
	.name		= "SC16IS762",
	.nr_gpio	= 8,
	.nr_uart	= 2,
};
EXPORT_SYMBOL_GPL(sc16is762_devtype);

static bool sc16is7xx_regmap_volatile(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case SC16IS7XX_RHR_REG:
	case SC16IS7XX_IIR_REG:
	case SC16IS7XX_LSR_REG:
	case SC16IS7XX_MSR_REG:
	case SC16IS7XX_TXLVL_REG:
	case SC16IS7XX_RXLVL_REG:
	case SC16IS7XX_IOSTATE_REG:
	case SC16IS7XX_IOCONTROL_REG:
		return true;
	default:
		return false;
	}
}

static bool sc16is7xx_regmap_precious(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case SC16IS7XX_RHR_REG:
		return true;
	default:
		return false;
	}
}

static bool sc16is7xx_regmap_noinc(struct device *dev, unsigned int reg)
{
	return reg == SC16IS7XX_RHR_REG;
}

/*
 * Configure programmable baud rate generator (divisor) according to the
 * desired baud rate.
 *
 * From the datasheet, the divisor is computed according to:
 *
 *              XTAL1 input frequency
 *             -----------------------
 *                    prescaler
 * divisor = ---------------------------
 *            baud-rate x sampling-rate
 */
static int sc16is7xx_set_baud(struct uart_port *port, int baud)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);
	u8 lcr;
	unsigned int prescaler = 1;
	unsigned long clk = port->uartclk, div = clk / 16 / baud;

	if (div >= BIT(16)) {
		prescaler = 4;
		div /= prescaler;
	}

	/* If bit MCR_CLKSEL is set, the divide by 4 prescaler is activated. */
	sc16is7xx_port_update(port, SC16IS7XX_MCR_REG,
			      SC16IS7XX_MCR_CLKSEL_BIT,
			      prescaler == 1 ? 0 : SC16IS7XX_MCR_CLKSEL_BIT);

	mutex_lock(&one->efr_lock);

	/* Backup LCR and access special register set (DLL/DLH) */
	lcr = sc16is7xx_port_read(port, SC16IS7XX_LCR_REG);
	sc16is7xx_port_write(port, SC16IS7XX_LCR_REG,
			     SC16IS7XX_LCR_CONF_MODE_A);

	/* Write the new divisor */
	regcache_cache_bypass(one->regmap, true);
	sc16is7xx_port_write(port, SC16IS7XX_DLH_REG, div / 256);
	sc16is7xx_port_write(port, SC16IS7XX_DLL_REG, div % 256);
	regcache_cache_bypass(one->regmap, false);

	/* Restore LCR and access to general register set */
	sc16is7xx_port_write(port, SC16IS7XX_LCR_REG, lcr);

	mutex_unlock(&one->efr_lock);

	return DIV_ROUND_CLOSEST((clk / prescaler) / 16, div);
}

static void sc16is7xx_handle_rx(struct uart_port *port, unsigned int rxlen,
				unsigned int iir)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);
	unsigned int lsr = 0, bytes_read, i;
	bool read_lsr = (iir == SC16IS7XX_IIR_RLSE_SRC) ? true : false;
	u8 ch, flag;

	if (unlikely(rxlen >= sizeof(one->buf))) {
		dev_warn_ratelimited(port->dev,
				     "ttySC%i: Possible RX FIFO overrun: %d\n",
				     port->line, rxlen);
		port->icount.buf_overrun++;
		/* Ensure sanity of RX level */
		rxlen = sizeof(one->buf);
	}

	while (rxlen) {
		/* Only read lsr if there are possible errors in FIFO */
		if (read_lsr) {
			lsr = sc16is7xx_port_read(port, SC16IS7XX_LSR_REG);
			if (!(lsr & SC16IS7XX_LSR_FIFOE_BIT))
				read_lsr = false; /* No errors left in FIFO */
		} else
			lsr = 0;

		if (read_lsr) {
			one->buf[0] = sc16is7xx_port_read(port, SC16IS7XX_RHR_REG);
			bytes_read = 1;
		} else {
			sc16is7xx_fifo_read(port, one->buf, rxlen);
			bytes_read = rxlen;
		}

		lsr &= SC16IS7XX_LSR_BRK_ERROR_MASK;

		port->icount.rx++;
		flag = TTY_NORMAL;

		if (unlikely(lsr)) {
			if (lsr & SC16IS7XX_LSR_BI_BIT) {
				port->icount.brk++;
				if (uart_handle_break(port))
					continue;
			} else if (lsr & SC16IS7XX_LSR_PE_BIT)
				port->icount.parity++;
			else if (lsr & SC16IS7XX_LSR_FE_BIT)
				port->icount.frame++;
			else if (lsr & SC16IS7XX_LSR_OE_BIT)
				port->icount.overrun++;

			lsr &= port->read_status_mask;
			if (lsr & SC16IS7XX_LSR_BI_BIT)
				flag = TTY_BREAK;
			else if (lsr & SC16IS7XX_LSR_PE_BIT)
				flag = TTY_PARITY;
			else if (lsr & SC16IS7XX_LSR_FE_BIT)
				flag = TTY_FRAME;
			else if (lsr & SC16IS7XX_LSR_OE_BIT)
				flag = TTY_OVERRUN;
		}

		for (i = 0; i < bytes_read; ++i) {
			ch = one->buf[i];
			if (uart_handle_sysrq_char(port, ch))
				continue;

			if (lsr & port->ignore_status_mask)
				continue;

			uart_insert_char(port, lsr, SC16IS7XX_LSR_OE_BIT, ch,
					 flag);
		}
		rxlen -= bytes_read;
	}

	tty_flip_buffer_push(&port->state->port);
}

static void sc16is7xx_handle_tx(struct uart_port *port)
{
	struct tty_port *tport = &port->state->port;
	unsigned long flags;
	unsigned int txlen;
	unsigned char *tail;

	if (unlikely(port->x_char)) {
		sc16is7xx_port_write(port, SC16IS7XX_THR_REG, port->x_char);
		port->icount.tx++;
		port->x_char = 0;
		return;
	}

	if (kfifo_is_empty(&tport->xmit_fifo) || uart_tx_stopped(port)) {
		uart_port_lock_irqsave(port, &flags);
		sc16is7xx_stop_tx(port);
		uart_port_unlock_irqrestore(port, flags);
		return;
	}

	/* Limit to space available in TX FIFO */
	txlen = sc16is7xx_port_read(port, SC16IS7XX_TXLVL_REG);
	if (txlen > SC16IS7XX_FIFO_SIZE) {
		dev_err_ratelimited(port->dev,
			"chip reports %d free bytes in TX fifo, but it only has %d",
			txlen, SC16IS7XX_FIFO_SIZE);
		txlen = 0;
	}

	txlen = kfifo_out_linear_ptr(&tport->xmit_fifo, &tail, txlen);
	sc16is7xx_fifo_write(port, tail, txlen);
	uart_xmit_advance(port, txlen);

	uart_port_lock_irqsave(port, &flags);
	if (kfifo_len(&tport->xmit_fifo) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	if (kfifo_is_empty(&tport->xmit_fifo))
		sc16is7xx_stop_tx(port);
	else
		sc16is7xx_ier_set(port, SC16IS7XX_IER_THRI_BIT);
	uart_port_unlock_irqrestore(port, flags);
}

static unsigned int sc16is7xx_get_hwmctrl(struct uart_port *port)
{
	u8 msr = sc16is7xx_port_read(port, SC16IS7XX_MSR_REG);
	unsigned int mctrl = 0;

	mctrl |= (msr & SC16IS7XX_MSR_CTS_BIT) ? TIOCM_CTS : 0;
	mctrl |= (msr & SC16IS7XX_MSR_DSR_BIT) ? TIOCM_DSR : 0;
	mctrl |= (msr & SC16IS7XX_MSR_CD_BIT)  ? TIOCM_CAR : 0;
	mctrl |= (msr & SC16IS7XX_MSR_RI_BIT)  ? TIOCM_RNG : 0;
	return mctrl;
}

static void sc16is7xx_update_mlines(struct sc16is7xx_one *one)
{
	struct uart_port *port = &one->port;
	unsigned long flags;
	unsigned int status, changed;

	lockdep_assert_held_once(&one->efr_lock);

	status = sc16is7xx_get_hwmctrl(port);
	changed = status ^ one->old_mctrl;

	if (changed == 0)
		return;

	one->old_mctrl = status;

	uart_port_lock_irqsave(port, &flags);
	if ((changed & TIOCM_RNG) && (status & TIOCM_RNG))
		port->icount.rng++;
	if (changed & TIOCM_DSR)
		port->icount.dsr++;
	if (changed & TIOCM_CAR)
		uart_handle_dcd_change(port, status & TIOCM_CAR);
	if (changed & TIOCM_CTS)
		uart_handle_cts_change(port, status & TIOCM_CTS);

	wake_up_interruptible(&port->state->port.delta_msr_wait);
	uart_port_unlock_irqrestore(port, flags);
}

static bool sc16is7xx_port_irq(struct sc16is7xx_port *s, int portno)
{
	bool rc = true;
	unsigned int iir, rxlen;
	struct uart_port *port = &s->p[portno].port;
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	mutex_lock(&one->efr_lock);

	iir = sc16is7xx_port_read(port, SC16IS7XX_IIR_REG);
	if (iir & SC16IS7XX_IIR_NO_INT_BIT) {
		rc = false;
		goto out_port_irq;
	}

	iir &= SC16IS7XX_IIR_ID_MASK;

	switch (iir) {
	case SC16IS7XX_IIR_RDI_SRC:
	case SC16IS7XX_IIR_RLSE_SRC:
	case SC16IS7XX_IIR_RTOI_SRC:
	case SC16IS7XX_IIR_XOFFI_SRC:
		rxlen = sc16is7xx_port_read(port, SC16IS7XX_RXLVL_REG);

		/*
		 * There is a silicon bug that makes the chip report a
		 * time-out interrupt but no data in the FIFO. This is
		 * described in errata section 18.1.4.
		 *
		 * When this happens, read one byte from the FIFO to
		 * clear the interrupt.
		 */
		if (iir == SC16IS7XX_IIR_RTOI_SRC && !rxlen)
			rxlen = 1;

		if (rxlen)
			sc16is7xx_handle_rx(port, rxlen, iir);
		break;
		/* CTSRTS interrupt comes only when CTS goes inactive */
	case SC16IS7XX_IIR_CTSRTS_SRC:
	case SC16IS7XX_IIR_MSI_SRC:
		sc16is7xx_update_mlines(one);
		break;
	case SC16IS7XX_IIR_THRI_SRC:
		sc16is7xx_handle_tx(port);
		break;
	default:
		dev_err_ratelimited(port->dev,
				    "ttySC%i: Unexpected interrupt: %x",
				    port->line, iir);
		break;
	}

out_port_irq:
	mutex_unlock(&one->efr_lock);

	return rc;
}

static irqreturn_t sc16is7xx_irq(int irq, void *dev_id)
{
	bool keep_polling;

	struct sc16is7xx_port *s = (struct sc16is7xx_port *)dev_id;

	do {
		int i;

		keep_polling = false;

		for (i = 0; i < s->devtype->nr_uart; ++i)
			keep_polling |= sc16is7xx_port_irq(s, i);
	} while (keep_polling);

	return IRQ_HANDLED;
}

static void sc16is7xx_tx_proc(struct kthread_work *ws)
{
	struct uart_port *port = &(to_sc16is7xx_one(ws, tx_work)->port);
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	if ((port->rs485.flags & SER_RS485_ENABLED) &&
	    (port->rs485.delay_rts_before_send > 0))
		msleep(port->rs485.delay_rts_before_send);

	mutex_lock(&one->efr_lock);
	sc16is7xx_handle_tx(port);
	mutex_unlock(&one->efr_lock);
}

static void sc16is7xx_reconf_rs485(struct uart_port *port)
{
	const u32 mask = SC16IS7XX_EFCR_AUTO_RS485_BIT |
			 SC16IS7XX_EFCR_RTS_INVERT_BIT;
	u32 efcr = 0;
	struct serial_rs485 *rs485 = &port->rs485;
	unsigned long irqflags;

	uart_port_lock_irqsave(port, &irqflags);
	if (rs485->flags & SER_RS485_ENABLED) {
		efcr |=	SC16IS7XX_EFCR_AUTO_RS485_BIT;

		if (rs485->flags & SER_RS485_RTS_AFTER_SEND)
			efcr |= SC16IS7XX_EFCR_RTS_INVERT_BIT;
	}
	uart_port_unlock_irqrestore(port, irqflags);

	sc16is7xx_port_update(port, SC16IS7XX_EFCR_REG, mask, efcr);
}

static void sc16is7xx_reg_proc(struct kthread_work *ws)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(ws, reg_work);
	struct sc16is7xx_one_config config;
	unsigned long irqflags;

	uart_port_lock_irqsave(&one->port, &irqflags);
	config = one->config;
	memset(&one->config, 0, sizeof(one->config));
	uart_port_unlock_irqrestore(&one->port, irqflags);

	if (config.flags & SC16IS7XX_RECONF_MD) {
		u8 mcr = 0;

		/* Device ignores RTS setting when hardware flow is enabled */
		if (one->port.mctrl & TIOCM_RTS)
			mcr |= SC16IS7XX_MCR_RTS_BIT;

		if (one->port.mctrl & TIOCM_DTR)
			mcr |= SC16IS7XX_MCR_DTR_BIT;

		if (one->port.mctrl & TIOCM_LOOP)
			mcr |= SC16IS7XX_MCR_LOOP_BIT;
		sc16is7xx_port_update(&one->port, SC16IS7XX_MCR_REG,
				      SC16IS7XX_MCR_RTS_BIT |
				      SC16IS7XX_MCR_DTR_BIT |
				      SC16IS7XX_MCR_LOOP_BIT,
				      mcr);
	}

	if (config.flags & SC16IS7XX_RECONF_IER)
		sc16is7xx_port_update(&one->port, SC16IS7XX_IER_REG,
				      config.ier_mask, config.ier_val);

	if (config.flags & SC16IS7XX_RECONF_RS485)
		sc16is7xx_reconf_rs485(&one->port);
}

static void sc16is7xx_ms_proc(struct kthread_work *ws)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(ws, ms_work.work);
	struct sc16is7xx_port *s = dev_get_drvdata(one->port.dev);

	if (one->port.state) {
		mutex_lock(&one->efr_lock);
		sc16is7xx_update_mlines(one);
		mutex_unlock(&one->efr_lock);

		kthread_queue_delayed_work(&s->kworker, &one->ms_work, HZ);
	}
}

static void sc16is7xx_enable_ms(struct uart_port *port)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);
	struct sc16is7xx_port *s = dev_get_drvdata(port->dev);

	lockdep_assert_held_once(&port->lock);

	kthread_queue_delayed_work(&s->kworker, &one->ms_work, 0);
}

static void sc16is7xx_start_tx(struct uart_port *port)
{
	struct sc16is7xx_port *s = dev_get_drvdata(port->dev);
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	kthread_queue_work(&s->kworker, &one->tx_work);
}

static void sc16is7xx_throttle(struct uart_port *port)
{
	unsigned long flags;

	/*
	 * Hardware flow control is enabled and thus the device ignores RTS
	 * value set in MCR register. Stop reading data from RX FIFO so the
	 * AutoRTS feature will de-activate RTS output.
	 */
	uart_port_lock_irqsave(port, &flags);
	sc16is7xx_ier_clear(port, SC16IS7XX_IER_RDI_BIT);
	uart_port_unlock_irqrestore(port, flags);
}

static void sc16is7xx_unthrottle(struct uart_port *port)
{
	unsigned long flags;

	uart_port_lock_irqsave(port, &flags);
	sc16is7xx_ier_set(port, SC16IS7XX_IER_RDI_BIT);
	uart_port_unlock_irqrestore(port, flags);
}

static unsigned int sc16is7xx_tx_empty(struct uart_port *port)
{
	unsigned int lsr;

	lsr = sc16is7xx_port_read(port, SC16IS7XX_LSR_REG);

	return (lsr & SC16IS7XX_LSR_TEMT_BIT) ? TIOCSER_TEMT : 0;
}

static unsigned int sc16is7xx_get_mctrl(struct uart_port *port)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	/* Called with port lock taken so we can only return cached value */
	return one->old_mctrl;
}

static void sc16is7xx_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct sc16is7xx_port *s = dev_get_drvdata(port->dev);
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	one->config.flags |= SC16IS7XX_RECONF_MD;
	kthread_queue_work(&s->kworker, &one->reg_work);
}

static void sc16is7xx_break_ctl(struct uart_port *port, int break_state)
{
	sc16is7xx_port_update(port, SC16IS7XX_LCR_REG,
			      SC16IS7XX_LCR_TXBREAK_BIT,
			      break_state ? SC16IS7XX_LCR_TXBREAK_BIT : 0);
}

static void sc16is7xx_set_termios(struct uart_port *port,
				  struct ktermios *termios,
				  const struct ktermios *old)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);
	unsigned int lcr, flow = 0;
	int baud;
	unsigned long flags;

	kthread_cancel_delayed_work_sync(&one->ms_work);

	/* Mask termios capabilities we don't support */
	termios->c_cflag &= ~CMSPAR;

	/* Word size */
	switch (termios->c_cflag & CSIZE) {
	case CS5:
		lcr = SC16IS7XX_LCR_WORD_LEN_5;
		break;
	case CS6:
		lcr = SC16IS7XX_LCR_WORD_LEN_6;
		break;
	case CS7:
		lcr = SC16IS7XX_LCR_WORD_LEN_7;
		break;
	case CS8:
		lcr = SC16IS7XX_LCR_WORD_LEN_8;
		break;
	default:
		lcr = SC16IS7XX_LCR_WORD_LEN_8;
		termios->c_cflag &= ~CSIZE;
		termios->c_cflag |= CS8;
		break;
	}

	/* Parity */
	if (termios->c_cflag & PARENB) {
		lcr |= SC16IS7XX_LCR_PARITY_BIT;
		if (!(termios->c_cflag & PARODD))
			lcr |= SC16IS7XX_LCR_EVENPARITY_BIT;
	}

	/* Stop bits */
	if (termios->c_cflag & CSTOPB)
		lcr |= SC16IS7XX_LCR_STOPLEN_BIT; /* 2 stops */

	/* Set read status mask */
	port->read_status_mask = SC16IS7XX_LSR_OE_BIT;
	if (termios->c_iflag & INPCK)
		port->read_status_mask |= SC16IS7XX_LSR_PE_BIT |
					  SC16IS7XX_LSR_FE_BIT;
	if (termios->c_iflag & (BRKINT | PARMRK))
		port->read_status_mask |= SC16IS7XX_LSR_BI_BIT;

	/* Set status ignore mask */
	port->ignore_status_mask = 0;
	if (termios->c_iflag & IGNBRK)
		port->ignore_status_mask |= SC16IS7XX_LSR_BI_BIT;
	if (!(termios->c_cflag & CREAD))
		port->ignore_status_mask |= SC16IS7XX_LSR_BRK_ERROR_MASK;

	/* Configure flow control */
	port->status &= ~(UPSTAT_AUTOCTS | UPSTAT_AUTORTS);
	if (termios->c_cflag & CRTSCTS) {
		flow |= SC16IS7XX_EFR_AUTOCTS_BIT |
			SC16IS7XX_EFR_AUTORTS_BIT;
		port->status |= UPSTAT_AUTOCTS | UPSTAT_AUTORTS;
	}
	if (termios->c_iflag & IXON)
		flow |= SC16IS7XX_EFR_SWFLOW3_BIT;
	if (termios->c_iflag & IXOFF)
		flow |= SC16IS7XX_EFR_SWFLOW1_BIT;

	/* Update LCR register */
	sc16is7xx_port_write(port, SC16IS7XX_LCR_REG, lcr);

	/* Update EFR registers */
	sc16is7xx_efr_lock(port);
	sc16is7xx_port_write(port, SC16IS7XX_XON1_REG, termios->c_cc[VSTART]);
	sc16is7xx_port_write(port, SC16IS7XX_XOFF1_REG, termios->c_cc[VSTOP]);
	sc16is7xx_port_update(port, SC16IS7XX_EFR_REG,
			      SC16IS7XX_EFR_FLOWCTRL_BITS, flow);
	sc16is7xx_efr_unlock(port);

	/* Get baud rate generator configuration */
	baud = uart_get_baud_rate(port, termios, old,
				  port->uartclk / 16 / 4 / 0xffff,
				  port->uartclk / 16);

	/* Setup baudrate generator */
	baud = sc16is7xx_set_baud(port, baud);

	uart_port_lock_irqsave(port, &flags);

	/* Update timeout according to new baud rate */
	uart_update_timeout(port, termios->c_cflag, baud);

	if (UART_ENABLE_MS(port, termios->c_cflag))
		sc16is7xx_enable_ms(port);

	uart_port_unlock_irqrestore(port, flags);
}

static int sc16is7xx_config_rs485(struct uart_port *port, struct ktermios *termios,
				  struct serial_rs485 *rs485)
{
	struct sc16is7xx_port *s = dev_get_drvdata(port->dev);
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	if (rs485->flags & SER_RS485_ENABLED) {
		/*
		 * RTS signal is handled by HW, it's timing can't be influenced.
		 * However, it's sometimes useful to delay TX even without RTS
		 * control therefore we try to handle .delay_rts_before_send.
		 */
		if (rs485->delay_rts_after_send)
			return -EINVAL;
	}

	one->config.flags |= SC16IS7XX_RECONF_RS485;
	kthread_queue_work(&s->kworker, &one->reg_work);

	return 0;
}

static int sc16is7xx_startup(struct uart_port *port)
{
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);
	unsigned int val;
	unsigned long flags;

	sc16is7xx_power(port, 1);

	/* Reset FIFOs*/
	val = SC16IS7XX_FCR_RXRESET_BIT | SC16IS7XX_FCR_TXRESET_BIT;
	sc16is7xx_port_write(port, SC16IS7XX_FCR_REG, val);
	udelay(5);
	sc16is7xx_port_write(port, SC16IS7XX_FCR_REG,
			     SC16IS7XX_FCR_FIFO_BIT);

	/* Enable TCR/TLR */
	sc16is7xx_port_update(port, SC16IS7XX_MCR_REG,
			      SC16IS7XX_MCR_TCRTLR_BIT,
			      SC16IS7XX_MCR_TCRTLR_BIT);

	/* Configure flow control levels */
	/* Flow control halt level 48, resume level 24 */
	sc16is7xx_port_write(port, SC16IS7XX_TCR_REG,
			     SC16IS7XX_TCR_RX_RESUME(24) |
			     SC16IS7XX_TCR_RX_HALT(48));

	/* Disable TCR/TLR access */
	sc16is7xx_port_update(port, SC16IS7XX_MCR_REG, SC16IS7XX_MCR_TCRTLR_BIT, 0);

	/* Now, initialize the UART */
	sc16is7xx_port_write(port, SC16IS7XX_LCR_REG, SC16IS7XX_LCR_WORD_LEN_8);

	/* Enable IrDA mode if requested in DT */
	/* This bit must be written with LCR[7] = 0 */
	sc16is7xx_port_update(port, SC16IS7XX_MCR_REG,
			      SC16IS7XX_MCR_IRDA_BIT,
			      one->irda_mode ?
				SC16IS7XX_MCR_IRDA_BIT : 0);

	/* Enable the Rx and Tx FIFO */
	sc16is7xx_port_update(port, SC16IS7XX_EFCR_REG,
			      SC16IS7XX_EFCR_RXDISABLE_BIT |
			      SC16IS7XX_EFCR_TXDISABLE_BIT,
			      0);

	/* Enable RX, CTS change and modem lines interrupts */
	val = SC16IS7XX_IER_RDI_BIT | SC16IS7XX_IER_CTSI_BIT |
	      SC16IS7XX_IER_MSI_BIT;
	sc16is7xx_port_write(port, SC16IS7XX_IER_REG, val);

	/* Enable modem status polling */
	uart_port_lock_irqsave(port, &flags);
	sc16is7xx_enable_ms(port);
	uart_port_unlock_irqrestore(port, flags);

	return 0;
}

static void sc16is7xx_shutdown(struct uart_port *port)
{
	struct sc16is7xx_port *s = dev_get_drvdata(port->dev);
	struct sc16is7xx_one *one = to_sc16is7xx_one(port, port);

	kthread_cancel_delayed_work_sync(&one->ms_work);

	/* Disable all interrupts */
	sc16is7xx_port_write(port, SC16IS7XX_IER_REG, 0);
	/* Disable TX/RX */
	sc16is7xx_port_update(port, SC16IS7XX_EFCR_REG,
			      SC16IS7XX_EFCR_RXDISABLE_BIT |
			      SC16IS7XX_EFCR_TXDISABLE_BIT,
			      SC16IS7XX_EFCR_RXDISABLE_BIT |
			      SC16IS7XX_EFCR_TXDISABLE_BIT);

	sc16is7xx_power(port, 0);

	kthread_flush_worker(&s->kworker);
}

static const char *sc16is7xx_type(struct uart_port *port)
{
	struct sc16is7xx_port *s = dev_get_drvdata(port->dev);

	return (port->type == PORT_SC16IS7XX) ? s->devtype->name : NULL;
}

static int sc16is7xx_request_port(struct uart_port *port)
{
	/* Do nothing */
	return 0;
}

static void sc16is7xx_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
		port->type = PORT_SC16IS7XX;
}

static int sc16is7xx_verify_port(struct uart_port *port,
				 struct serial_struct *s)
{
	if ((s->type != PORT_UNKNOWN) && (s->type != PORT_SC16IS7XX))
		return -EINVAL;
	if (s->irq != port->irq)
		return -EINVAL;

	return 0;
}

static void sc16is7xx_pm(struct uart_port *port, unsigned int state,
			 unsigned int oldstate)
{
	sc16is7xx_power(port, (state == UART_PM_STATE_ON) ? 1 : 0);
}

static void sc16is7xx_null_void(struct uart_port *port)
{
	/* Do nothing */
}

static const struct uart_ops sc16is7xx_ops = {
	.tx_empty	= sc16is7xx_tx_empty,
	.set_mctrl	= sc16is7xx_set_mctrl,
	.get_mctrl	= sc16is7xx_get_mctrl,
	.stop_tx	= sc16is7xx_stop_tx,
	.start_tx	= sc16is7xx_start_tx,
	.throttle	= sc16is7xx_throttle,
	.unthrottle	= sc16is7xx_unthrottle,
	.stop_rx	= sc16is7xx_stop_rx,
	.enable_ms	= sc16is7xx_enable_ms,
	.break_ctl	= sc16is7xx_break_ctl,
	.startup	= sc16is7xx_startup,
	.shutdown	= sc16is7xx_shutdown,
	.set_termios	= sc16is7xx_set_termios,
	.type		= sc16is7xx_type,
	.request_port	= sc16is7xx_request_port,
	.release_port	= sc16is7xx_null_void,
	.config_port	= sc16is7xx_config_port,
	.verify_port	= sc16is7xx_verify_port,
	.pm		= sc16is7xx_pm,
};

#ifdef CONFIG_GPIOLIB
static int sc16is7xx_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	unsigned int val;
	struct sc16is7xx_port *s = gpiochip_get_data(chip);
	struct uart_port *port = &s->p[0].port;

	val = sc16is7xx_port_read(port, SC16IS7XX_IOSTATE_REG);

	return !!(val & BIT(offset));
}

static void sc16is7xx_gpio_set(struct gpio_chip *chip, unsigned offset, int val)
{
	struct sc16is7xx_port *s = gpiochip_get_data(chip);
	struct uart_port *port = &s->p[0].port;

	sc16is7xx_port_update(port, SC16IS7XX_IOSTATE_REG, BIT(offset),
			      val ? BIT(offset) : 0);
}

static int sc16is7xx_gpio_direction_input(struct gpio_chip *chip,
					  unsigned offset)
{
	struct sc16is7xx_port *s = gpiochip_get_data(chip);
	struct uart_port *port = &s->p[0].port;

	sc16is7xx_port_update(port, SC16IS7XX_IODIR_REG, BIT(offset), 0);

	return 0;
}

static int sc16is7xx_gpio_direction_output(struct gpio_chip *chip,
					   unsigned offset, int val)
{
	struct sc16is7xx_port *s = gpiochip_get_data(chip);
	struct uart_port *port = &s->p[0].port;
	u8 state = sc16is7xx_port_read(port, SC16IS7XX_IOSTATE_REG);

	if (val)
		state |= BIT(offset);
	else
		state &= ~BIT(offset);

	/*
	 * If we write IOSTATE first, and then IODIR, the output value is not
	 * transferred to the corresponding I/O pin.
	 * The datasheet states that each register bit will be transferred to
	 * the corresponding I/O pin programmed as output when writing to
	 * IOSTATE. Therefore, configure direction first with IODIR, and then
	 * set value after with IOSTATE.
	 */
	sc16is7xx_port_update(port, SC16IS7XX_IODIR_REG, BIT(offset),
			      BIT(offset));
	sc16is7xx_port_write(port, SC16IS7XX_IOSTATE_REG, state);

	return 0;
}

static int sc16is7xx_gpio_init_valid_mask(struct gpio_chip *chip,
					  unsigned long *valid_mask,
					  unsigned int ngpios)
{
	struct sc16is7xx_port *s = gpiochip_get_data(chip);

	*valid_mask = s->gpio_valid_mask;

	return 0;
}

static int sc16is7xx_setup_gpio_chip(struct sc16is7xx_port *s)
{
	struct device *dev = s->p[0].port.dev;

	if (!s->devtype->nr_gpio)
		return 0;

	switch (s->mctrl_mask) {
	case 0:
		s->gpio_valid_mask = GENMASK(7, 0);
		break;
	case SC16IS7XX_IOCONTROL_MODEM_A_BIT:
		s->gpio_valid_mask = GENMASK(3, 0);
		break;
	case SC16IS7XX_IOCONTROL_MODEM_B_BIT:
		s->gpio_valid_mask = GENMASK(7, 4);
		break;
	default:
		break;
	}

	if (s->gpio_valid_mask == 0)
		return 0;

	s->gpio.owner		 = THIS_MODULE;
	s->gpio.parent		 = dev;
	s->gpio.label		 = dev_name(dev);
	s->gpio.init_valid_mask	 = sc16is7xx_gpio_init_valid_mask;
	s->gpio.direction_input	 = sc16is7xx_gpio_direction_input;
	s->gpio.get		 = sc16is7xx_gpio_get;
	s->gpio.direction_output = sc16is7xx_gpio_direction_output;
	s->gpio.set		 = sc16is7xx_gpio_set;
	s->gpio.base		 = -1;
	s->gpio.ngpio		 = s->devtype->nr_gpio;
	s->gpio.can_sleep	 = 1;

	return gpiochip_add_data(&s->gpio, s);
}
#endif

static void sc16is7xx_setup_irda_ports(struct sc16is7xx_port *s)
{
	int i;
	int ret;
	int count;
	u32 irda_port[SC16IS7XX_MAX_PORTS];
	struct device *dev = s->p[0].port.dev;

	count = device_property_count_u32(dev, "irda-mode-ports");
	if (count < 0 || count > ARRAY_SIZE(irda_port))
		return;

	ret = device_property_read_u32_array(dev, "irda-mode-ports",
					     irda_port, count);
	if (ret)
		return;

	for (i = 0; i < count; i++) {
		if (irda_port[i] < s->devtype->nr_uart)
			s->p[irda_port[i]].irda_mode = true;
	}
}

/*
 * Configure ports designated to operate as modem control lines.
 */
static int sc16is7xx_setup_mctrl_ports(struct sc16is7xx_port *s,
				       struct regmap *regmap)
{
	int i;
	int ret;
	int count;
	u32 mctrl_port[SC16IS7XX_MAX_PORTS];
	struct device *dev = s->p[0].port.dev;

	count = device_property_count_u32(dev, "nxp,modem-control-line-ports");
	if (count < 0 || count > ARRAY_SIZE(mctrl_port))
		return 0;

	ret = device_property_read_u32_array(dev, "nxp,modem-control-line-ports",
					     mctrl_port, count);
	if (ret)
		return ret;

	s->mctrl_mask = 0;

	for (i = 0; i < count; i++) {
		/* Use GPIO lines as modem control lines */
		if (mctrl_port[i] == 0)
			s->mctrl_mask |= SC16IS7XX_IOCONTROL_MODEM_A_BIT;
		else if (mctrl_port[i] == 1)
			s->mctrl_mask |= SC16IS7XX_IOCONTROL_MODEM_B_BIT;
	}

	if (s->mctrl_mask)
		regmap_update_bits(
			regmap,
			SC16IS7XX_IOCONTROL_REG,
			SC16IS7XX_IOCONTROL_MODEM_A_BIT |
			SC16IS7XX_IOCONTROL_MODEM_B_BIT, s->mctrl_mask);

	return 0;
}

static const struct serial_rs485 sc16is7xx_rs485_supported = {
	.flags = SER_RS485_ENABLED | SER_RS485_RTS_AFTER_SEND,
	.delay_rts_before_send = 1,
	.delay_rts_after_send = 1,	/* Not supported but keep returning -EINVAL */
};

/* Reset device, purging any pending irq / data */
static int sc16is7xx_reset(struct device *dev, struct regmap *regmap)
{
	struct gpio_desc *reset_gpio;

	/* Assert reset GPIO if defined and valid. */
	reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(reset_gpio))
		return dev_err_probe(dev, PTR_ERR(reset_gpio), "Failed to get reset GPIO\n");

	if (reset_gpio) {
		/* The minimum reset pulse width is 3 us. */
		fsleep(5);
		gpiod_set_value_cansleep(reset_gpio, 0); /* Deassert GPIO */
	} else {
		/* Software reset */
		regmap_write(regmap, SC16IS7XX_IOCONTROL_REG,
			     SC16IS7XX_IOCONTROL_SRESET_BIT);
	}

	return 0;
}

int sc16is7xx_probe(struct device *dev, const struct sc16is7xx_devtype *devtype,
		    struct regmap *regmaps[], int irq)
{
	unsigned long freq = 0, *pfreq = dev_get_platdata(dev);
	unsigned int val;
	u32 uartclk = 0;
	int i, ret;
	struct sc16is7xx_port *s;
	bool port_registered[SC16IS7XX_MAX_PORTS];

	for (i = 0; i < devtype->nr_uart; i++)
		if (IS_ERR(regmaps[i]))
			return PTR_ERR(regmaps[i]);

	/*
	 * This device does not have an identification register that would
	 * tell us if we are really connected to the correct device.
	 * The best we can do is to check if communication is at all possible.
	 *
	 * Note: regmap[0] is used in the probe function to access registers
	 * common to all channels/ports, as it is guaranteed to be present on
	 * all variants.
	 */
	ret = regmap_read(regmaps[0], SC16IS7XX_LSR_REG, &val);
	if (ret < 0)
		return -EPROBE_DEFER;

	/* Alloc port structure */
	s = devm_kzalloc(dev, struct_size(s, p, devtype->nr_uart), GFP_KERNEL);
	if (!s) {
		dev_err(dev, "Error allocating port structure\n");
		return -ENOMEM;
	}

	/* Always ask for fixed clock rate from a property. */
	device_property_read_u32(dev, "clock-frequency", &uartclk);

	s->clk = devm_clk_get_optional(dev, NULL);
	if (IS_ERR(s->clk))
		return PTR_ERR(s->clk);

	ret = clk_prepare_enable(s->clk);
	if (ret)
		return ret;

	freq = clk_get_rate(s->clk);
	if (freq == 0) {
		if (uartclk)
			freq = uartclk;
		if (pfreq)
			freq = *pfreq;
		if (freq)
			dev_dbg(dev, "Clock frequency: %luHz\n", freq);
		else
			return -EINVAL;
	}

	s->devtype = devtype;
	dev_set_drvdata(dev, s);

	kthread_init_worker(&s->kworker);
	s->kworker_task = kthread_run(kthread_worker_fn, &s->kworker,
				      "sc16is7xx");
	if (IS_ERR(s->kworker_task)) {
		ret = PTR_ERR(s->kworker_task);
		goto out_clk;
	}
	sched_set_fifo(s->kworker_task);

	ret = sc16is7xx_reset(dev, regmaps[0]);
	if (ret)
		goto out_kthread;

	/* Mark each port line and status as uninitialised. */
	for (i = 0; i < devtype->nr_uart; ++i) {
		s->p[i].port.line = SC16IS7XX_MAX_DEVS;
		port_registered[i] = false;
	}

	for (i = 0; i < devtype->nr_uart; ++i) {
		ret = ida_alloc_max(&sc16is7xx_lines,
				    SC16IS7XX_MAX_DEVS - 1, GFP_KERNEL);
		if (ret < 0)
			goto out_ports;

		s->p[i].port.line = ret;

		/* Initialize port data */
		s->p[i].port.dev	= dev;
		s->p[i].port.irq	= irq;
		s->p[i].port.type	= PORT_SC16IS7XX;
		s->p[i].port.fifosize	= SC16IS7XX_FIFO_SIZE;
		s->p[i].port.flags	= UPF_FIXED_TYPE | UPF_LOW_LATENCY;
		s->p[i].port.iobase	= i;
		/*
		 * Use all ones as membase to make sure uart_configure_port() in
		 * serial_core.c does not abort for SPI/I2C devices where the
		 * membase address is not applicable.
		 */
		s->p[i].port.membase	= (void __iomem *)~0;
		s->p[i].port.iotype	= UPIO_PORT;
		s->p[i].port.uartclk	= freq;
		s->p[i].port.rs485_config = sc16is7xx_config_rs485;
		s->p[i].port.rs485_supported = sc16is7xx_rs485_supported;
		s->p[i].port.ops	= &sc16is7xx_ops;
		s->p[i].old_mctrl	= 0;
		s->p[i].regmap		= regmaps[i];

		mutex_init(&s->p[i].efr_lock);

		ret = uart_get_rs485_mode(&s->p[i].port);
		if (ret)
			goto out_ports;

		/* Disable all interrupts */
		sc16is7xx_port_write(&s->p[i].port, SC16IS7XX_IER_REG, 0);
		/* Disable TX/RX */
		sc16is7xx_port_write(&s->p[i].port, SC16IS7XX_EFCR_REG,
				     SC16IS7XX_EFCR_RXDISABLE_BIT |
				     SC16IS7XX_EFCR_TXDISABLE_BIT);

		/* Initialize kthread work structs */
		kthread_init_work(&s->p[i].tx_work, sc16is7xx_tx_proc);
		kthread_init_work(&s->p[i].reg_work, sc16is7xx_reg_proc);
		kthread_init_delayed_work(&s->p[i].ms_work, sc16is7xx_ms_proc);

		/* Register port */
		ret = uart_add_one_port(&sc16is7xx_uart, &s->p[i].port);
		if (ret)
			goto out_ports;

		port_registered[i] = true;

		/* Enable EFR */
		sc16is7xx_port_write(&s->p[i].port, SC16IS7XX_LCR_REG,
				     SC16IS7XX_LCR_CONF_MODE_B);

		regcache_cache_bypass(regmaps[i], true);

		/* Enable write access to enhanced features */
		sc16is7xx_port_write(&s->p[i].port, SC16IS7XX_EFR_REG,
				     SC16IS7XX_EFR_ENABLE_BIT);

		regcache_cache_bypass(regmaps[i], false);

		/* Restore access to general registers */
		sc16is7xx_port_write(&s->p[i].port, SC16IS7XX_LCR_REG, 0x00);

		/* Go to suspend mode */
		sc16is7xx_power(&s->p[i].port, 0);
	}

	sc16is7xx_setup_irda_ports(s);

	ret = sc16is7xx_setup_mctrl_ports(s, regmaps[0]);
	if (ret)
		goto out_ports;

#ifdef CONFIG_GPIOLIB
	ret = sc16is7xx_setup_gpio_chip(s);
	if (ret)
		goto out_ports;
#endif

	/*
	 * Setup interrupt. We first try to acquire the IRQ line as level IRQ.
	 * If that succeeds, we can allow sharing the interrupt as well.
	 * In case the interrupt controller doesn't support that, we fall
	 * back to a non-shared falling-edge trigger.
	 */
	ret = devm_request_threaded_irq(dev, irq, NULL, sc16is7xx_irq,
					IRQF_TRIGGER_LOW | IRQF_SHARED |
					IRQF_ONESHOT,
					dev_name(dev), s);
	if (!ret)
		return 0;

	ret = devm_request_threaded_irq(dev, irq, NULL, sc16is7xx_irq,
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					dev_name(dev), s);
	if (!ret)
		return 0;

#ifdef CONFIG_GPIOLIB
	if (s->gpio_valid_mask)
		gpiochip_remove(&s->gpio);
#endif

out_ports:
	for (i = 0; i < devtype->nr_uart; i++) {
		if (s->p[i].port.line < SC16IS7XX_MAX_DEVS)
			ida_free(&sc16is7xx_lines, s->p[i].port.line);
		if (port_registered[i])
			uart_remove_one_port(&sc16is7xx_uart, &s->p[i].port);
	}

out_kthread:
	kthread_stop(s->kworker_task);

out_clk:
	clk_disable_unprepare(s->clk);

	return ret;
}
EXPORT_SYMBOL_GPL(sc16is7xx_probe);

void sc16is7xx_remove(struct device *dev)
{
	struct sc16is7xx_port *s = dev_get_drvdata(dev);
	int i;

#ifdef CONFIG_GPIOLIB
	if (s->gpio_valid_mask)
		gpiochip_remove(&s->gpio);
#endif

	for (i = 0; i < s->devtype->nr_uart; i++) {
		kthread_cancel_delayed_work_sync(&s->p[i].ms_work);
		ida_free(&sc16is7xx_lines, s->p[i].port.line);
		uart_remove_one_port(&sc16is7xx_uart, &s->p[i].port);
		sc16is7xx_power(&s->p[i].port, 0);
	}

	kthread_flush_worker(&s->kworker);
	kthread_stop(s->kworker_task);

	clk_disable_unprepare(s->clk);
}
EXPORT_SYMBOL_GPL(sc16is7xx_remove);

const struct of_device_id __maybe_unused sc16is7xx_dt_ids[] = {
	{ .compatible = "nxp,sc16is740",	.data = &sc16is74x_devtype, },
	{ .compatible = "nxp,sc16is741",	.data = &sc16is74x_devtype, },
	{ .compatible = "nxp,sc16is750",	.data = &sc16is750_devtype, },
	{ .compatible = "nxp,sc16is752",	.data = &sc16is752_devtype, },
	{ .compatible = "nxp,sc16is760",	.data = &sc16is760_devtype, },
	{ .compatible = "nxp,sc16is762",	.data = &sc16is762_devtype, },
	{ }
};
EXPORT_SYMBOL_GPL(sc16is7xx_dt_ids);
MODULE_DEVICE_TABLE(of, sc16is7xx_dt_ids);

const struct regmap_config sc16is7xx_regcfg = {
	.reg_bits = 5,
	.pad_bits = 3,
	.val_bits = 8,
	.cache_type = REGCACHE_MAPLE,
	.volatile_reg = sc16is7xx_regmap_volatile,
	.precious_reg = sc16is7xx_regmap_precious,
	.writeable_noinc_reg = sc16is7xx_regmap_noinc,
	.readable_noinc_reg = sc16is7xx_regmap_noinc,
	.max_raw_read = SC16IS7XX_FIFO_SIZE,
	.max_raw_write = SC16IS7XX_FIFO_SIZE,
	.max_register = SC16IS7XX_EFCR_REG,
};
EXPORT_SYMBOL_GPL(sc16is7xx_regcfg);

const char *sc16is7xx_regmap_name(u8 port_id)
{
	switch (port_id) {
	case 0:	return "port0";
	case 1:	return "port1";
	default:
		WARN_ON(true);
		return NULL;
	}
}
EXPORT_SYMBOL_GPL(sc16is7xx_regmap_name);

unsigned int sc16is7xx_regmap_port_mask(unsigned int port_id)
{
	/* CH1,CH0 are at bits 2:1. */
	return port_id << 1;
}
EXPORT_SYMBOL_GPL(sc16is7xx_regmap_port_mask);

static int __init sc16is7xx_init(void)
{
	return uart_register_driver(&sc16is7xx_uart);
}
module_init(sc16is7xx_init);

static void __exit sc16is7xx_exit(void)
{
	uart_unregister_driver(&sc16is7xx_uart);
}
module_exit(sc16is7xx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jon Ringle <jringle@gridpoint.com>");
MODULE_DESCRIPTION("SC16IS7xx tty serial core driver");
