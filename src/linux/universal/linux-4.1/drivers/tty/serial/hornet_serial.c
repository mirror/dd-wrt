/*
 *  linux/drivers/serial/hornet_serial.c
 *
 *  Driver for hornet serial ports
 *
 *  Based on drivers/char/serial.c, by Linus Torvalds, Theodore Ts'o.
 *
 *  Copyright (C) 2010 Ryan Hsu.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *  $Id$
 *
 * A note about mapbase / membase
 *
 *  mapbase is the physical address of the IO port.
 *  membase is an 'ioremapped' cookie.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_reg.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/nmi.h>
#include <linux/mutex.h>

#include <asm/io.h>
#include <asm/irq.h>

#include "8250.h"
#include "ar7240.h"

/*
 * uncomment below to enable WAR for EV81847.
 */ 
//#define AR9330_EV81847_WAR

extern void serial_print(char *fmt, ...); 

#if 0
#define DEBUG_STEP()    serial_print("<<%s, %d>>\n\r", __FUNCTION__, __LINE__)
#else
#define DEBUG_STEP()
#endif

/*
 * Configuration:
 *   share_irqs - whether we pass SA_SHIRQ to request_irq().  This option
 *                is unsafe when used on edge-triggered interrupts.
 */
static unsigned int share_irqs = 0;

static unsigned int nr_uarts = CONFIG_SERIAL_AR933X_NR_UARTS;

static struct uart_driver serial8250_reg;

static int serial_index(struct uart_port *port)
{
	return (serial8250_reg.minor - 64) + port->line;
}

/*
 * Debugging.
 */
#if 0
#define DEBUG_AUTOCONF(fmt...)	printk(fmt)
#else
#define DEBUG_AUTOCONF(fmt...)	do { } while (0)
#endif

#if 0
#define DEBUG_INTR(fmt...)	printk(fmt)
#else
#define DEBUG_INTR(fmt...)	do { } while (0)
#endif

#define PASS_LIMIT	65536

/*
 * We default to IRQ0 for the "no irq" hack.   Some
 * machine types want others as well - they're free
 * to redefine this in their header file.
 */
#define is_real_interrupt(irq)	((irq) != 0)

#ifdef CONFIG_SERIAL_8250_DETECT_IRQ
#define CONFIG_SERIAL_DETECT_IRQ 1
#endif
#ifdef CONFIG_SERIAL_8250_MANY_PORTS
#define CONFIG_SERIAL_MANY_PORTS 1
#endif

/*
 * HUB6 is always on.  This will be removed once the header
 * files have been cleaned.
 */
#define CONFIG_HUB6 1

#include <asm/serial.h>
/*
 * SERIAL_PORT_DFNS tells us about built-in ports that have no
 * standard enumeration mechanism.   Platforms that can find all
 * serial ports via mechanisms like ACPI or PCI need not supply it.
 */
#ifndef SERIAL_PORT_DFNS
#define SERIAL_PORT_DFNS
#endif

static const struct old_serial_port old_serial_port[] = {
	SERIAL_PORT_DFNS /* defined in asm/serial.h */
};

#define UART_NR	CONFIG_SERIAL_AR933X_NR_UARTS

#ifdef CONFIG_SERIAL_8250_RSA

#define PORT_RSA_MAX 4
static unsigned long probe_rsa[PORT_RSA_MAX];
static unsigned int probe_rsa_count;
#endif /* CONFIG_SERIAL_8250_RSA  */

struct uart_8250_port {
	struct uart_port	port;
	struct timer_list	timer;		/* "no irq" timer */
	struct list_head	list;		/* ports on this IRQ */
	unsigned short		capabilities;	/* port capabilities */
	unsigned short		bugs;		/* port bugs */
	unsigned int		tx_loadsz;	/* transmit fifo load size */
	unsigned char		acr;
	unsigned char		ier;
	unsigned char		lcr;
	unsigned char		mcr;
	unsigned char		mcr_mask;	/* mask of user bits */
	unsigned char		mcr_force;	/* mask of forced bits */
	unsigned char		cur_iotype;	/* Running I/O type */

	/*
	 * Some bits in registers are cleared on a read, so they must
	 * be saved whenever the register is read but the bits will not
	 * be immediately processed.
	 */
#define LSR_SAVE_FLAGS UART_LSR_BRK_ERROR_BITS
	unsigned char		lsr_saved_flags;
#define MSR_SAVE_FLAGS UART_MSR_ANY_DELTA
	unsigned char		msr_saved_flags;

	/*
	 * We provide a per-port pm hook.
	 */
	void			(*pm)(struct uart_port *port,
				      unsigned int state, unsigned int old);
};

struct irq_info {
	spinlock_t		lock;
	struct list_head	*head;
};

static struct irq_info irq_lists[NR_IRQS];

static void wait_for_xmitr(struct uart_8250_port *up);

/*
 * Here we define the default xmit fifo size used for each type of UART.
 */
static const struct serial8250_config uart_config[] = {
	[PORT_UNKNOWN] = {
		.name		= "unknown",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_8250] = {
		.name		= "8250",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16450] = {
		.name		= "16450",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16550] = {
		.name		= "16550",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16550A] = {
		.name		= "16550A",
		.fifo_size	= 16,
		.tx_loadsz	= 16,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_CIRRUS] = {
		.name		= "Cirrus",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16650] = {
		.name		= "ST16650",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
		.flags		= UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP,
	},
	[PORT_16650V2] = {
		.name		= "ST16650V2",
		.fifo_size	= 32,
		.tx_loadsz	= 16,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_01 |
				  UART_FCR_T_TRIG_00,
		.flags		= UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP,
	},
	[PORT_16750] = {
		.name		= "TI16750",
		.fifo_size	= 64,
		.tx_loadsz	= 64,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10 |
				  UART_FCR7_64BYTE,
		.flags		= UART_CAP_FIFO | UART_CAP_SLEEP | UART_CAP_AFE,
	},
	[PORT_STARTECH] = {
		.name		= "Startech",
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16C950] = {
		.name		= "16C950/954",
		.fifo_size	= 128,
		.tx_loadsz	= 128,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_16654] = {
		.name		= "ST16654",
		.fifo_size	= 64,
		.tx_loadsz	= 32,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_01 |
				  UART_FCR_T_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP,
	},
	[PORT_16850] = {
		.name		= "XR16850",
		.fifo_size	= 128,
		.tx_loadsz	= 128,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP,
	},
	[PORT_RSA] = {
		.name		= "RSA",
		.fifo_size	= 2048,
		.tx_loadsz	= 2048,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_11,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_NS16550A] = {
		.name		= "NS16550A",
		.fifo_size	= 16,
		.tx_loadsz	= 16,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_NATSEMI,
	},
	[PORT_XSCALE] = {
		.name		= "XScale",
		.fifo_size	= 32,
		.tx_loadsz	= 32,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO | UART_CAP_UUE,
	},
	[PORT_RM9000] = {
		.name		= "RM9000",
		.fifo_size	= 16,
		.tx_loadsz	= 16,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_OCTEON] = {
		.name		= "OCTEON",
		.fifo_size	= 64,
		.tx_loadsz	= 64,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_AR7] = {
		.name		= "AR7",
		.fifo_size	= 16,
		.tx_loadsz	= 16,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_00,
		.flags		= UART_CAP_FIFO | UART_CAP_AFE,
	},
};

#ifdef CONFIG_SERIAL_8250_AU1X00

/* Au1x00 UART hardware has a weird register layout */
static const u8 au_io_in_map[] = {
	[UART_RX]  = 0,
	[UART_IER] = 2,
	[UART_IIR] = 3,
	[UART_LCR] = 5,
	[UART_MCR] = 6,
	[UART_LSR] = 7,
	[UART_MSR] = 8,
};

static const u8 au_io_out_map[] = {
	[UART_TX]  = 1,
	[UART_IER] = 2,
	[UART_FCR] = 4,
	[UART_LCR] = 5,
	[UART_MCR] = 6,
};

/* sane hardware needs no mapping */
static inline int map_8250_in_reg(struct uart_8250_port *up, int offset)
{
	if (up->port.iotype != UPIO_AU)
		return offset;
	return au_io_in_map[offset];
}

static inline int map_8250_out_reg(struct uart_8250_port *up, int offset)
{
	if (up->port.iotype != UPIO_AU)
		return offset;
	return au_io_out_map[offset];
}

#else

/* sane hardware needs no mapping */
#define map_8250_in_reg(up, offset) (offset)
#define map_8250_out_reg(up, offset) (offset)

#endif

#if 1
static  unsigned int serial_in(struct uart_8250_port *up, int offset)
{
	switch (up->port.iotype) {
	case UPIO_HUB6:
		outb(up->port.hub6 - 1 + offset, up->port.iobase);
		return inb(up->port.iobase + 1);

	case UPIO_MEM:
		return readb(up->port.membase + offset);

	case UPIO_MEM32:
		return readl(up->port.membase + offset);

#ifdef CONFIG_SERIAL_8250_AU1X00
	case UPIO_AU:
		return __raw_readl(up->port.membase + offset);
#endif

	default:
		return inb(up->port.iobase + offset);
	}
}

static  void
serial_out(struct uart_8250_port *up, int offset, int value)
{
	switch (up->port.iotype) {
	case UPIO_HUB6:
		outb(up->port.hub6 - 1 + offset, up->port.iobase);
		outb(value, up->port.iobase + 1);
		break;

	case UPIO_MEM:
		writeb(value, up->port.membase + offset);
		break;

	case UPIO_MEM32:
		writel(value, up->port.membase + offset);
		break;

#ifdef CONFIG_SERIAL_8250_AU1X00
	case UPIO_AU:
		__raw_writel(value, up->port.membase + offset);
		break;
#endif

	default:
		outb(value, up->port.iobase + offset);
	}
}
#else

static unsigned int io_serial_in(struct uart_port *port, int offset)
{
	switch (port->iotype) {
	case UPIO_HUB6:
		outb(port->hub6 - 1 + offset, port->iobase);
		return inb(port->iobase + 1);

	case UPIO_MEM:
		return readb(port->membase + offset);

	case UPIO_MEM32:
		return readl(port->membase + offset);

#ifdef CONFIG_SERIAL_8250_AU1X00
	case UPIO_AU:
		return __raw_readl(port->membase + offset);
#endif

	default:
		return inb(port->iobase + offset);
	}
}

static void io_serial_out(struct uart_port *port, int offset, int value)
{
	switch (port->iotype) {
	case UPIO_HUB6:
		outb(port->hub6 - 1 + offset, port->iobase);
		outb(value, port->iobase + 1);
		break;

	case UPIO_MEM:
		writeb(value, port->membase + offset);
		break;

	case UPIO_MEM32:
		writel(value, port->membase + offset);
		break;

#ifdef CONFIG_SERIAL_8250_AU1X00
	case UPIO_AU:
		__raw_writel(value, port->membase + offset);
		break;
#endif

	default:
		outb(value, port->iobase + offset);
	}
}
#define serial_in(up, offset)		\
	(up->port.serial_in(&(up)->port, (offset)))
#define serial_out(up, offset, value)	\
	(up->port.serial_out(&(up)->port, (offset), (value)))
#endif

static void set_io_from_upio(struct uart_port *p)
{
	struct uart_8250_port *up = (struct uart_8250_port *)p;

#if 0
    p->serial_in = io_serial_in;
	p->serial_out = io_serial_out;
#else
    p->serial_in = serial_in;
	p->serial_out = serial_out;

#endif
	/* Remember loaded iotype */
	up->cur_iotype = p->iotype;
}

static void
serial_out_sync(struct uart_8250_port *up, int offset, int value)
{
	struct uart_port *p = &up->port;
	switch (p->iotype) {
	default:
		p->serial_out(p, offset, value);
	}
}

/*
 * We used to support using pause I/O for certain machines.  We
 * haven't supported this for a while, but just in case it's badly
 * needed for certain old 386 machines, I've left these #define's
 * in....
 */
#define serial_inp(up, offset)		serial_in(up, offset)
#define serial_outp(up, offset, value)	serial_out(up, offset, value)


/*
 * For the 16C950
 */
static void serial_icr_write(struct uart_8250_port *up, int offset, int value)
{
	serial_out(up, UART_SCR, offset);
	serial_out(up, UART_ICR, value);
}

static unsigned int serial_icr_read(struct uart_8250_port *up, int offset)
{
	unsigned int value;

	serial_icr_write(up, UART_ACR, up->acr | UART_ACR_ICRRD);
	serial_out(up, UART_SCR, offset);
	value = serial_in(up, UART_ICR);
	serial_icr_write(up, UART_ACR, up->acr);

	return value;
}

/*
 * FIFO support.
 */
static inline void serial8250_clear_fifos(struct uart_8250_port *p)
{
//    printk("%s, %d\n\r", __FUNCTION__, __LINE__);
}

/*
 * IER sleep support.  UARTs which have EFRs need the "extended
 * capability" bit enabled.  Note that on XR16C850s, we need to
 * reset LCR to write to IER.
 */
static inline void serial8250_set_sleep(struct uart_8250_port *p, int sleep)
{
//    printk("%s, %d\n\r", __FUNCTION__, __LINE__);
}

#ifdef CONFIG_SERIAL_8250_RSA
/*
 * Attempts to turn on the RSA FIFO.  Returns zero on failure.
 * We set the port uart clock rate if we succeed.
 */
static int __enable_rsa(struct uart_8250_port *up)
{
	unsigned char mode;
	int result;

	mode = serial_inp(up, UART_RSA_MSR);
	result = mode & UART_RSA_MSR_FIFO;

	if (!result) {
		serial_outp(up, UART_RSA_MSR, mode | UART_RSA_MSR_FIFO);
		mode = serial_inp(up, UART_RSA_MSR);
		result = mode & UART_RSA_MSR_FIFO;
	}

	if (result)
		up->port.uartclk = SERIAL_RSA_BAUD_BASE * 16;

	return result;
}

static void enable_rsa(struct uart_8250_port *up)
{
	if (up->port.type == PORT_RSA) {
		if (up->port.uartclk != SERIAL_RSA_BAUD_BASE * 16) {
			spin_lock_irq(&up->port.lock);
			__enable_rsa(up);
			spin_unlock_irq(&up->port.lock);
		}
		if (up->port.uartclk == SERIAL_RSA_BAUD_BASE * 16)
			serial_outp(up, UART_RSA_FRR, 0);
	}
}

/*
 * Attempts to turn off the RSA FIFO.  Returns zero on failure.
 * It is unknown why interrupts were disabled in here.  However,
 * the caller is expected to preserve this behaviour by grabbing
 * the spinlock before calling this function.
 */
static void disable_rsa(struct uart_8250_port *up)
{
	unsigned char mode;
	int result;

	if (up->port.type == PORT_RSA &&
	    up->port.uartclk == SERIAL_RSA_BAUD_BASE * 16) {
		spin_lock_irq(&up->port.lock);

		mode = serial_inp(up, UART_RSA_MSR);
		result = !(mode & UART_RSA_MSR_FIFO);

		if (!result) {
			serial_outp(up, UART_RSA_MSR, mode & ~UART_RSA_MSR_FIFO);
			mode = serial_inp(up, UART_RSA_MSR);
			result = !(mode & UART_RSA_MSR_FIFO);
		}

		if (result)
			up->port.uartclk = SERIAL_RSA_BAUD_BASE_LO * 16;
		spin_unlock_irq(&up->port.lock);
	}
}
#endif /* CONFIG_SERIAL_8250_RSA */

/*
 * This is a quickie test to see how big the FIFO is.
 * It doesn't work at all the time, more's the pity.
 */
static int size_fifo(struct uart_8250_port *up)
{
	unsigned char old_fcr, old_mcr, old_dll, old_dlm, old_lcr;
	int count;

	old_lcr = serial_inp(up, UART_LCR);
	serial_outp(up, UART_LCR, 0);
	old_fcr = serial_inp(up, UART_FCR);
	old_mcr = serial_inp(up, UART_MCR);
	serial_outp(up, UART_FCR, UART_FCR_ENABLE_FIFO |
		    UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);
	serial_outp(up, UART_MCR, UART_MCR_LOOP);
	serial_outp(up, UART_LCR, UART_LCR_DLAB);
	old_dll = serial_inp(up, UART_DLL);
	old_dlm = serial_inp(up, UART_DLM);
	serial_outp(up, UART_DLL, 0x01);
	serial_outp(up, UART_DLM, 0x00);
	serial_outp(up, UART_LCR, 0x03);
	for (count = 0; count < 256; count++)
		serial_outp(up, UART_TX, count);
	mdelay(20);/* FIXME - schedule_timeout */
	for (count = 0; (serial_inp(up, UART_LSR) & UART_LSR_DR) &&
	     (count < 256); count++)
		serial_inp(up, UART_RX);
	serial_outp(up, UART_FCR, old_fcr);
	serial_outp(up, UART_MCR, old_mcr);
	serial_outp(up, UART_LCR, UART_LCR_DLAB);
	serial_outp(up, UART_DLL, old_dll);
	serial_outp(up, UART_DLM, old_dlm);
	serial_outp(up, UART_LCR, old_lcr);

	return count;
}

/*
 * Read UART ID using the divisor method - set DLL and DLM to zero
 * and the revision will be in DLL and device type in DLM.  We
 * preserve the device state across this.
 */
static unsigned int autoconfig_read_divisor_id(struct uart_8250_port *p)
{
	unsigned char old_dll, old_dlm, old_lcr;
	unsigned int id;

	old_lcr = serial_inp(p, UART_LCR);
	serial_outp(p, UART_LCR, UART_LCR_DLAB);

	old_dll = serial_inp(p, UART_DLL);
	old_dlm = serial_inp(p, UART_DLM);

	serial_outp(p, UART_DLL, 0);
	serial_outp(p, UART_DLM, 0);

	id = serial_inp(p, UART_DLL) | serial_inp(p, UART_DLM) << 8;

	serial_outp(p, UART_DLL, old_dll);
	serial_outp(p, UART_DLM, old_dlm);
	serial_outp(p, UART_LCR, old_lcr);

	return id;
}

/*
 * This is a helper routine to autodetect StarTech/Exar/Oxsemi UART's.
 * When this function is called we know it is at least a StarTech
 * 16650 V2, but it might be one of several StarTech UARTs, or one of
 * its clones.  (We treat the broken original StarTech 16650 V1 as a
 * 16550, and why not?  Startech doesn't seem to even acknowledge its
 * existence.)
 * 
 * What evil have men's minds wrought...
 */
static void autoconfig_has_efr(struct uart_8250_port *up)
{
	unsigned int id1, id2, id3, rev;

	/*
	 * Everything with an EFR has SLEEP
	 */
	up->capabilities |= UART_CAP_EFR | UART_CAP_SLEEP;

	/*
	 * First we check to see if it's an Oxford Semiconductor UART.
	 *
	 * If we have to do this here because some non-National
	 * Semiconductor clone chips lock up if you try writing to the
	 * LSR register (which serial_icr_read does)
	 */

	/*
	 * Check for Oxford Semiconductor 16C950.
	 *
	 * EFR [4] must be set else this test fails.
	 *
	 * This shouldn't be necessary, but Mike Hudson (Exoray@isys.ca)
	 * claims that it's needed for 952 dual UART's (which are not
	 * recommended for new designs).
	 */
	up->acr = 0;
	serial_out(up, UART_LCR, 0xBF);
	serial_out(up, UART_EFR, UART_EFR_ECB);
	serial_out(up, UART_LCR, 0x00);
	id1 = serial_icr_read(up, UART_ID1);
	id2 = serial_icr_read(up, UART_ID2);
	id3 = serial_icr_read(up, UART_ID3);
	rev = serial_icr_read(up, UART_REV);

	DEBUG_AUTOCONF("950id=%02x:%02x:%02x:%02x ", id1, id2, id3, rev);

	if (id1 == 0x16 && id2 == 0xC9 &&
	    (id3 == 0x50 || id3 == 0x52 || id3 == 0x54)) {
		up->port.type = PORT_16C950;

		/*
		 * Enable work around for the Oxford Semiconductor 952 rev B
		 * chip which causes it to seriously miscalculate baud rates
		 * when DLL is 0.
		 */
		if (id3 == 0x52 && rev == 0x01)
			up->bugs |= UART_BUG_QUOT;
		return;
	}
	
	/*
	 * We check for a XR16C850 by setting DLL and DLM to 0, and then
	 * reading back DLL and DLM.  The chip type depends on the DLM
	 * value read back:
	 *  0x10 - XR16C850 and the DLL contains the chip revision.
	 *  0x12 - XR16C2850.
	 *  0x14 - XR16C854.
	 */
	id1 = autoconfig_read_divisor_id(up);
	DEBUG_AUTOCONF("850id=%04x ", id1);

	id2 = id1 >> 8;
	if (id2 == 0x10 || id2 == 0x12 || id2 == 0x14) {
		up->port.type = PORT_16850;
		return;
	}

	/*
	 * It wasn't an XR16C850.
	 *
	 * We distinguish between the '654 and the '650 by counting
	 * how many bytes are in the FIFO.  I'm using this for now,
	 * since that's the technique that was sent to me in the
	 * serial driver update, but I'm not convinced this works.
	 * I've had problems doing this in the past.  -TYT
	 */
	if (size_fifo(up) == 64)
		up->port.type = PORT_16654;
	else
		up->port.type = PORT_16650V2;
}

/*
 * We detected a chip without a FIFO.  Only two fall into
 * this category - the original 8250 and the 16450.  The
 * 16450 has a scratch register (accessible with LCR=0)
 */
static void autoconfig_8250(struct uart_8250_port *up)
{
	unsigned char scratch, status1, status2;

	up->port.type = PORT_8250;

	scratch = serial_in(up, UART_SCR);
	serial_outp(up, UART_SCR, 0xa5);
	status1 = serial_in(up, UART_SCR);
	serial_outp(up, UART_SCR, 0x5a);
	status2 = serial_in(up, UART_SCR);
	serial_outp(up, UART_SCR, scratch);

	if (status1 == 0xa5 && status2 == 0x5a)
		up->port.type = PORT_16450;
}

static int broken_efr(struct uart_8250_port *up)
{
	/*
	 * Exar ST16C2550 "A2" devices incorrectly detect as
	 * having an EFR, and report an ID of 0x0201.  See
	 * http://www.exar.com/info.php?pdf=dan180_oct2004.pdf
	 */
	if (autoconfig_read_divisor_id(up) == 0x0201 && size_fifo(up) == 16)
		return 1;

	return 0;
}

/*
 * We know that the chip has FIFOs.  Does it have an EFR?  The
 * EFR is located in the same register position as the IIR and
 * we know the top two bits of the IIR are currently set.  The
 * EFR should contain zero.  Try to read the EFR.
 */
static void autoconfig_16550a(struct uart_8250_port *up)
{
	unsigned char status1, status2;
	unsigned int iersave;

	up->port.type = PORT_16550A;
	up->capabilities |= UART_CAP_FIFO;

	/*
	 * Check for presence of the EFR when DLAB is set.
	 * Only ST16C650V1 UARTs pass this test.
	 */
	serial_outp(up, UART_LCR, UART_LCR_DLAB);
	if (serial_in(up, UART_EFR) == 0) {
		serial_outp(up, UART_EFR, 0xA8);
		if (serial_in(up, UART_EFR) != 0) {
			DEBUG_AUTOCONF("EFRv1 ");
			up->port.type = PORT_16650;
			up->capabilities |= UART_CAP_EFR | UART_CAP_SLEEP;
		} else {
			DEBUG_AUTOCONF("Motorola 8xxx DUART ");
		}
		serial_outp(up, UART_EFR, 0);
		return;
	}

	/*
	 * Maybe it requires 0xbf to be written to the LCR.
	 * (other ST16C650V2 UARTs, TI16C752A, etc)
	 */
	serial_outp(up, UART_LCR, 0xBF);
	if (serial_in(up, UART_EFR) == 0 && !broken_efr(up)) {
		DEBUG_AUTOCONF("EFRv2 ");
		autoconfig_has_efr(up);
		return;
	}

	/*
	 * Check for a National Semiconductor SuperIO chip.
	 * Attempt to switch to bank 2, read the value of the LOOP bit
	 * from EXCR1. Switch back to bank 0, change it in MCR. Then
	 * switch back to bank 2, read it from EXCR1 again and check
	 * it's changed. If so, set baud_base in EXCR2 to 921600. -- dwmw2
	 */
	serial_outp(up, UART_LCR, 0);
	status1 = serial_in(up, UART_MCR);
	serial_outp(up, UART_LCR, 0xE0);
	status2 = serial_in(up, 0x02); /* EXCR1 */

	if (!((status2 ^ status1) & UART_MCR_LOOP)) {
		serial_outp(up, UART_LCR, 0);
		serial_outp(up, UART_MCR, status1 ^ UART_MCR_LOOP);
		serial_outp(up, UART_LCR, 0xE0);
		status2 = serial_in(up, 0x02); /* EXCR1 */
		serial_outp(up, UART_LCR, 0);
		serial_outp(up, UART_MCR, status1);

		if ((status2 ^ status1) & UART_MCR_LOOP) {
			unsigned short quot;

			serial_outp(up, UART_LCR, 0xE0);

			quot = serial_inp(up, UART_DLM) << 8;
			quot += serial_inp(up, UART_DLL);
			quot <<= 3;

			status1 = serial_in(up, 0x04); /* EXCR1 */
			status1 &= ~0xB0; /* Disable LOCK, mask out PRESL[01] */
			status1 |= 0x10;  /* 1.625 divisor for baud_base --> 921600 */
			serial_outp(up, 0x04, status1);
			
			serial_outp(up, UART_DLL, quot & 0xff);
			serial_outp(up, UART_DLM, quot >> 8);

			serial_outp(up, UART_LCR, 0);

			up->port.uartclk = 921600*16;
			up->port.type = PORT_NS16550A;
			up->capabilities |= UART_NATSEMI;
			return;
		}
	}

	/*
	 * No EFR.  Try to detect a TI16750, which only sets bit 5 of
	 * the IIR when 64 byte FIFO mode is enabled when DLAB is set.
	 * Try setting it with and without DLAB set.  Cheap clones
	 * set bit 5 without DLAB set.
	 */
	serial_outp(up, UART_LCR, 0);
	serial_outp(up, UART_FCR, UART_FCR_ENABLE_FIFO | UART_FCR7_64BYTE);
	status1 = serial_in(up, UART_IIR) >> 5;
	serial_outp(up, UART_FCR, UART_FCR_ENABLE_FIFO);
	serial_outp(up, UART_LCR, UART_LCR_DLAB);
	serial_outp(up, UART_FCR, UART_FCR_ENABLE_FIFO | UART_FCR7_64BYTE);
	status2 = serial_in(up, UART_IIR) >> 5;
	serial_outp(up, UART_FCR, UART_FCR_ENABLE_FIFO);
	serial_outp(up, UART_LCR, 0);

	DEBUG_AUTOCONF("iir1=%d iir2=%d ", status1, status2);

	if (status1 == 6 && status2 == 7) {
		up->port.type = PORT_16750;
		up->capabilities |= UART_CAP_AFE | UART_CAP_SLEEP;
		return;
	}

	/*
	 * Try writing and reading the UART_IER_UUE bit (b6).
	 * If it works, this is probably one of the Xscale platform's
	 * internal UARTs.
	 * We're going to explicitly set the UUE bit to 0 before
	 * trying to write and read a 1 just to make sure it's not
	 * already a 1 and maybe locked there before we even start start.
	 */
	iersave = serial_in(up, UART_IER);
	serial_outp(up, UART_IER, iersave & ~UART_IER_UUE);
	if (!(serial_in(up, UART_IER) & UART_IER_UUE)) {
		/*
		 * OK it's in a known zero state, try writing and reading
		 * without disturbing the current state of the other bits.
		 */
		serial_outp(up, UART_IER, iersave | UART_IER_UUE);
		if (serial_in(up, UART_IER) & UART_IER_UUE) {
			/*
			 * It's an Xscale.
			 * We'll leave the UART_IER_UUE bit set to 1 (enabled).
			 */
			DEBUG_AUTOCONF("Xscale ");
			up->port.type = PORT_XSCALE;
			up->capabilities |= UART_CAP_UUE;
			return;
		}
	} else {
		/*
		 * If we got here we couldn't force the IER_UUE bit to 0.
		 * Log it and continue.
		 */
		DEBUG_AUTOCONF("Couldn't force IER_UUE to 0 ");
	}
	serial_outp(up, UART_IER, iersave);
}

/*
 * This routine is called by rs_init() to initialize a specific serial
 * port.  It determines what type of UART chip this serial port is
 * using: 8250, 16450, 16550, 16550A.  The important question is
 * whether or not this UART is a 16550A or not, since this will
 * determine whether or not we can use its FIFO features or not.
 */
static void autoconfig(struct uart_8250_port *up, unsigned int probeflags)
{
	unsigned char status1, scratch, scratch2, scratch3;
	unsigned char save_lcr, save_mcr;
	unsigned long flags;

    DEBUG_STEP();

	if (!up->port.iobase && !up->port.mapbase && !up->port.membase) {
    DEBUG_STEP();
		return;
	}

	DEBUG_AUTOCONF("ttyS%d: autoconf (0x%04x, 0x%p): ",
			up->port.line, up->port.iobase, up->port.membase);

    up->port.type = PORT_16550A;

	/*
	 * We really do need global IRQs disabled here - we're going to
	 * be frobbing the chips IRQ enable register to see if it exists.
	 */
	spin_lock_irqsave(&up->port.lock, flags);
//	save_flags(flags); cli();

	up->capabilities = 0;
	up->bugs = 0;

#ifdef CONFIG_AR9100
    up->bugs |= UART_BUG_NOMSR;
#endif

	if (up->capabilities != uart_config[up->port.type].flags) {
		printk(KERN_WARNING
		       "ttyS%d: detected caps %08x should be %08x\n",
			up->port.line, up->capabilities,
			uart_config[up->port.type].flags);
	}

	up->port.fifosize = uart_config[up->port.type].fifo_size;
	up->capabilities = uart_config[up->port.type].flags;
	up->tx_loadsz = uart_config[up->port.type].tx_loadsz;
	
	if (up->port.type == PORT_UNKNOWN)
		goto out;

 out:   
{
    unsigned int rdata;

    DEBUG_STEP();
 
    rdata = serial_in(up, UARTCS_ADDRESS);
    rdata |= UARTCS_UARTHOSTINTEN_SET(1);
    serial_out(up, UARTCS_ADDRESS, rdata);

    DEBUG_STEP();
    //rdata = serial_in(up, UARTINTEN_ADDRESS);
    rdata = (UARTINTEN_UARTRXVALIDINTEN_SET(1));// | UARTINTEN_UARTTXREADYINTEN_SET(1));
    serial_out(up, UARTINTEN_ADDRESS, rdata); 
}
	spin_unlock_irqrestore(&up->port.lock, flags);
    DEBUG_STEP();

	DEBUG_AUTOCONF("type=%s\n", uart_config[up->port.type].name);	
}

static void autoconfig_irq(struct uart_8250_port *up)
{
	unsigned char save_mcr, save_ier;
	unsigned char save_ICP = 0;
	unsigned int ICP = 0;
	unsigned long irqs;
	int irq;
	unsigned int rdata;

	if (up->port.flags & UPF_FOURPORT) {
		ICP = (up->port.iobase & 0xfe0) | 0x1f;
		save_ICP = inb_p(ICP);
		outb_p(0x80, ICP);
		(void) inb_p(ICP);
	}

	/* forget possible initially masked and pending IRQ */
	probe_irq_off(probe_irq_on());
	save_ier = serial_inp(up, UARTINTEN_ADDRESS);

	irqs = probe_irq_on();
	udelay (10);

	serial_outp(up, UARTINTEN_ADDRESS, UARTINTEN_HW_MASK);	/* enable all intrs */

    rdata = UARTDATA_UARTTXRXDATA_SET(0xFF);
    rdata |= UARTDATA_UARTTXCSR_SET(1);        
    serial_outp(up, UARTDATA_ADDRESS, rdata);  
		
	udelay (20);
	irq = probe_irq_off(irqs);

	serial_outp(up, UARTINTEN_ADDRESS, save_ier);

	if (up->port.flags & UPF_FOURPORT)
		outb_p(save_ICP, ICP);

	up->port.irq = (irq > 0) ? irq : 0;
}

static inline void __start_tx_interrupt()
{
    unsigned int rdata;
    rdata = uart_reg_read(UARTINTEN_ADDRESS) | (UARTINTEN_UARTTXEMPTYINTEN_SET(1));
    uart_reg_write(UARTINTEN_ADDRESS, rdata);
}

static inline void __stop_tx(struct uart_8250_port *p)
{
    unsigned int rdata;
    
	if (p->ier & UART_IER_THRI) {
		p->ier &= ~UART_IER_THRI;
		
        rdata = serial_in(p, UARTINTEN_ADDRESS);
        rdata = rdata & ~(UARTINTEN_UARTRXVALIDINTEN_SET(1));// | UARTINTEN_UARTTXREADYINTEN_SET(1));
        serial_out(p, UARTINTEN_ADDRESS, rdata);  
	}

}

static void serial8250_stop_tx(struct uart_port *port)
{
	struct uart_8250_port *up = (struct uart_8250_port *)port;

	__stop_tx(up);
}

static void transmit_chars(struct uart_8250_port *up);

static void serial8250_start_tx(struct uart_port *port)
{
    __start_tx_interrupt();
}

static void serial8250_stop_rx(struct uart_port *port)
{
	struct uart_8250_port *up = (struct uart_8250_port *)port;
	unsigned int rdata;

	up->ier &= ~UART_IER_RLSI;
	up->port.read_status_mask &= ~UART_LSR_DR;

    rdata = serial_in(up, UARTINTEN_ADDRESS);
    rdata = rdata & ~(UARTINTEN_UARTRXVALIDINTEN_SET(1));// | UARTINTEN_UARTTXREADYINTEN_SET(1));
    serial_out(up, UARTINTEN_ADDRESS, rdata);	
}

static void serial8250_enable_ms(struct uart_port *port)
{
//    printk("%s, %d\n\r", __FUNCTION__, __LINE__);
}

static  void
receive_chars(struct uart_8250_port *up, int *status)
{
	struct tty_struct *tty = up->port.state->port.tty;
	unsigned int lsr = *status;
	unsigned char ch;
	int max_count = 256;
	char flag;

	do {
#if 0
		if (likely(lsr & UART_LSR_DR))
			ch = serial_inp(up, UART_RX);
		else
			/*
			 * Intel 82571 has a Serial Over Lan device that will
			 * set UART_LSR_BI without setting UART_LSR_DR when
			 * it receives a break. To avoid reading from the
			 * receive buffer without UART_LSR_DR bit set, we
			 * just force the read character to be 0
			 */
			ch = 0;
#endif
		ch = (unsigned char)UARTDATA_UARTTXRXDATA_GET(lsr);
		flag = TTY_NORMAL;
		up->port.icount.rx++;

        lsr = UARTDATA_UARTRXCSR_SET(1);
        serial_outp(up, UARTDATA_ADDRESS, lsr);

		if (unlikely(lsr & (UART_LSR_BI | UART_LSR_PE |
				    UART_LSR_FE | UART_LSR_OE))) {
			/*
			 * For statistics only
			 */
			if (lsr & UART_LSR_BI) {
				lsr &= ~(UART_LSR_FE | UART_LSR_PE);
				up->port.icount.brk++;
				/*
				 * We do the SysRQ and SAK checking
				 * here because otherwise the break
				 * may get masked by ignore_status_mask
				 * or read_status_mask.
				 */
				if (uart_handle_break(&up->port))
					goto ignore_char;
			} else if (lsr & UART_LSR_PE)
				up->port.icount.parity++;
			else if (lsr & UART_LSR_FE)
				up->port.icount.frame++;
			if (lsr & UART_LSR_OE)
				up->port.icount.overrun++;

			/*
			 * Mask off conditions which should be ignored.
			 */
			lsr &= up->port.read_status_mask;

			if (lsr & UART_LSR_BI) {
				DEBUG_INTR("handling break....");
				flag = TTY_BREAK;
			} else if (lsr & UART_LSR_PE)
				flag = TTY_PARITY;
			else if (lsr & UART_LSR_FE)
				flag = TTY_FRAME;
		}
		if (uart_handle_sysrq_char(&up->port, ch))
			goto ignore_char;

		uart_insert_char(&up->port, lsr, UART_LSR_OE, ch, flag);

	ignore_char:
		lsr = serial_inp(up, UARTDATA_ADDRESS);
	} while ((lsr & UARTDATA_UARTRXCSR_MASK) && (max_count-- > 0));
	spin_unlock(&up->port.lock);
	tty_flip_buffer_push(tty);
	spin_lock(&up->port.lock);
	*status = lsr; 	
}

static  void transmit_chars(struct uart_8250_port *up)
{
	struct circ_buf *xmit = &up->port.state->xmit;
	int count;
	unsigned int rdata;

    if (UARTDATA_UARTTXCSR_GET(serial_in(up, UARTDATA_ADDRESS)) == 0) {
        __start_tx_interrupt();
        return;
    }

    if (up->port.x_char) {
        rdata = UARTDATA_UARTTXRXDATA_SET((unsigned int)(up->port.x_char));
        rdata |= UARTDATA_UARTTXCSR_SET(1);        
        serial_outp(up, UARTDATA_ADDRESS, rdata);
        up->port.icount.tx++;
        up->port.x_char = 0;
        __start_tx_interrupt();
        return;
    }
	if (uart_tx_stopped(&up->port)) {
		serial8250_stop_tx(&up->port);
		return;
	}

	if (uart_circ_empty(xmit)) {
		__stop_tx(up);
		return;
	}

	count = up->tx_loadsz;
	do {
        if (UARTDATA_UARTTXCSR_GET(serial_in(up, UARTDATA_ADDRESS)) == 0) {
            __start_tx_interrupt();
            return;
        }

        rdata = UARTDATA_UARTTXRXDATA_SET((unsigned int)(xmit->buf[xmit->tail]));
        rdata |= UARTDATA_UARTTXCSR_SET(1);        
		serial_out(up, UARTDATA_ADDRESS, rdata);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		up->port.icount.tx++;
		if (uart_circ_empty(xmit)){
			break;
        }

    } while (--count > 0); 


    if (UARTDATA_UARTTXCSR_GET(serial_in(up, UARTDATA_ADDRESS)) == 0) {
        __start_tx_interrupt();
        return;
    }

    /* Re-enable TX Empty Interrupt to transmit pending chars */
    if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS) {
        uart_write_wakeup(&up->port);
        __start_tx_interrupt();
    }

	DEBUG_INTR("THRE...");

	if (uart_circ_empty(xmit)) {
		__stop_tx(up);
    } else {
        __start_tx_interrupt();
    }
}

static  void check_modem_status(struct uart_8250_port *up)
{
    printk("%s, %d\n\r", __FUNCTION__, __LINE__);
}


/*! Hornet's interrupt status is not read clear, so that we have to... 
 * a. read out the interrupt status
 * b. clear the interrupt mask to reset the interrupt status
 * c. enable the interrupt to reactivate interrupt
 *
 * Disable and clear the interrupt status
 */
static inline void serial8250_clear_int(struct uart_8250_port *up)
{
#define BIT3 (0x1>>3)

    volatile unsigned int uart_tcs_s = 0x0;
    
    /* 1. clear MISC interrupt mask */
    //ar7240_reg_rmw_clear(AR7240_MISC_INT_MASK, BIT3);
    
    /* 2. clear uartcs hostinten mask, bit13 */
    uart_tcs_s = uart_reg_read(UARTCS_ADDRESS) & ~UARTCS_UARTHOSTINTEN_SET(1);
    uart_reg_write(UARTCS_ADDRESS, uart_tcs_s);
    
    /* 3. clear rx uartint */
    uart_reg_write(UARTINT_ADDRESS, (UARTINT_UARTRXVALIDINT_SET(1)));

    /* 4. clear misc interrupt status  */
    ar7240_reg_rmw_clear(AR7240_MISC_INT_STATUS, BIT3);
    
    /* 5. clear rx uartinten*/
    uart_tcs_s = uart_reg_read(UARTINTEN_ADDRESS) & ~UARTINTEN_UARTRXVALIDINTEN_SET(1);
    uart_reg_write(UARTINTEN_ADDRESS, uart_tcs_s);

    /* 6. enable rx int*/
    uart_tcs_s = uart_reg_read(UARTINTEN_ADDRESS) | (UARTINTEN_UARTRXVALIDINTEN_SET(1));

    uart_reg_write(UARTINTEN_ADDRESS, uart_tcs_s);
    
    /* 7. set uartcs hostinten mask */
    uart_tcs_s = uart_reg_read(UARTCS_ADDRESS) | UARTCS_UARTHOSTINTEN_SET(1);
    uart_reg_write(UARTCS_ADDRESS, uart_tcs_s);
    
    /* 8. set misc int mask */
    //ar7240_reg_wr(AR7240_MISC_INT_MASK, BIT3);
}

/*
 * This handles the interrupt from one port.
 */
static inline void
serial8250_handle_port(struct uart_8250_port *up)
{
    unsigned int status = serial_inp(up, UARTDATA_ADDRESS);
    unsigned int int_status = serial_inp(up, UARTINT_ADDRESS);
    unsigned int en_status = serial_inp(up, UARTINTEN_ADDRESS);
	unsigned long flags;

	spin_lock_irqsave(&up->port.lock, flags);

	if( (int_status&en_status) & UARTINT_UARTRXVALIDINT_MASK ) {
		receive_chars(up, &status);
	}


    if (((int_status & en_status) & UARTINT_UARTTXEMPTYINT_MASK)) {
        unsigned int rdata;
        /* Clear and disable TX Empty Interrupt */
        uart_reg_write(UARTINT_ADDRESS, (UARTINT_UARTTXEMPTYINT_SET(1)));
        rdata = uart_reg_read(UARTINTEN_ADDRESS);
        rdata = rdata & ~(UARTINTEN_UARTTXEMPTYINTEN_SET(1));
        uart_reg_write(UARTINTEN_ADDRESS, rdata);

        if(!uart_circ_empty(&up->port.state->xmit) ) {
            transmit_chars(up);
        }
    }

	spin_unlock_irqrestore(&up->port.lock, flags);
}

/*
 * This is the serial driver's interrupt routine.
 *
 * Arjan thinks the old way was overly complex, so it got simplified.
 * Alan disagrees, saying that need the complexity to handle the weird
 * nature of ISA shared interrupts.  (This is a special exception.)
 *
 * In order to handle ISA shared interrupts properly, we need to check
 * that all ports have been serviced, and therefore the ISA interrupt
 * line has been de-asserted.
 *
 * This means we need to loop through all ports. checking that they
 * don't have an interrupt pending.
 */
static irqreturn_t serial8250_interrupt(int irq, void *dev_id)
{
	struct irq_info *i = dev_id;
	struct list_head *l, *end = NULL;
	int pass_counter = 0, handled = 0;

	DEBUG_INTR("serial8250_interrupt(%d)...", irq);

	spin_lock(&i->lock);

	l = i->head;
	do {
		struct uart_8250_port *up;
		unsigned int iir;

		up = list_entry(l, struct uart_8250_port, list);

		iir = serial_in(up, UARTCS_ADDRESS);
		if (iir & UARTCS_UARTHOSTINT_MASK) {
			spin_lock(&up->port.lock);
			serial8250_handle_port(up);
        		serial8250_clear_int(up);
			spin_unlock(&up->port.lock);

			handled = 1;

			end = NULL;
		} else if (end == NULL)
			end = l;

		l = l->next;

		if (l == i->head && pass_counter++ > PASS_LIMIT) {
			/* If we hit this, we're dead. */
			printk(KERN_ERR "serial8250: too much work for irq%d\n", irq);
			break;
		}
	} while (l != end);

	spin_unlock(&i->lock);

	DEBUG_INTR("end.\n");

	return IRQ_RETVAL(handled);
}

/*
 * To support ISA shared interrupts, we need to have one interrupt
 * handler that ensures that the IRQ line has been deasserted
 * before returning.  Failing to do this will result in the IRQ
 * line being stuck active, and, since ISA irqs are edge triggered,
 * no more IRQs will be seen.
 */
static void serial_do_unlink(struct irq_info *i, struct uart_8250_port *up)
{
	spin_lock_irq(&i->lock);

	if (!list_empty(i->head)) {
		if (i->head == &up->list)
			i->head = i->head->next;
		list_del(&up->list);
	} else {
		BUG_ON(i->head != &up->list);
		i->head = NULL;
	}

	spin_unlock_irq(&i->lock);
}

static int serial_link_irq_chain(struct uart_8250_port *up)
{
	struct irq_info *i = irq_lists + up->port.irq;
	int ret, irq_flags = up->port.flags & UPF_SHARE_IRQ ? IRQF_SHARED : 0;

	spin_lock_irq(&i->lock);

	if (i->head) {
		list_add(&up->list, i->head);
		spin_unlock_irq(&i->lock);

		ret = 0;
	} else {
		INIT_LIST_HEAD(&up->list);
		i->head = &up->list;
		spin_unlock_irq(&i->lock);

		ret = request_irq(up->port.irq, serial8250_interrupt,
				  irq_flags, "serial", i);
		if (ret < 0)
			serial_do_unlink(i, up);
	}

	return ret;
}

static void serial_unlink_irq_chain(struct uart_8250_port *up)
{
	struct irq_info *i = irq_lists + up->port.irq;

	BUG_ON(i->head == NULL);

	if (list_empty(i->head))
		free_irq(up->port.irq, i);

	serial_do_unlink(i, up);
}

/*
 * This function is used to handle ports that do not have an
 * interrupt.  This doesn't work very well for 16450's, but gives
 * barely passable results for a 16550A.  (Although at the expense
 * of much CPU overhead).
 */
static void serial8250_timeout(unsigned long data)
{
	struct uart_8250_port *up = (struct uart_8250_port *)data;
	unsigned int timeout;
	unsigned int iir;
#ifdef AR9330_EV81847_WAR
    struct circ_buf *xmit = &up->port.state->xmit;
    unsigned long flags;

    if(!uart_circ_empty(xmit)) {
		spin_lock_irqsave(&up->port.lock, flags);
		transmit_chars(up);
		spin_unlock_irqrestore(&up->port.lock, flags);
    }
#else
	iir = serial_in(up, UARTCS_ADDRESS);
	if (iir & UARTCS_UARTHOSTINT_MASK) {
		spin_lock(&up->port.lock);
		serial8250_handle_port(up);
		spin_unlock(&up->port.lock);
	}
#endif

	timeout = up->port.timeout;
	timeout = timeout > 6 ? (timeout / 2 - 2) : 1;
	mod_timer(&up->timer, jiffies + timeout);
}

static unsigned int serial8250_tx_empty(struct uart_port *port)
{
	struct uart_8250_port *up = (struct uart_8250_port *)port;
	unsigned long flags;
	unsigned int ret;

	spin_lock_irqsave(&up->port.lock, flags);
    ret = serial_in(up, UARTDATA_ADDRESS) & UARTDATA_UARTTXCSR_MASK ? 0 : TIOCSER_TEMT;
	spin_unlock_irqrestore(&up->port.lock, flags);

	return ret;
}

static unsigned int serial8250_get_mctrl(struct uart_port *port)
{
//    printk("%s, %d\n\r", __FUNCTION__, __LINE__);
    return TIOCM_CAR;
}

static void serial8250_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
//    printk("%s, %d\n\r", __FUNCTION__, __LINE__);
}

static void serial8250_break_ctl(struct uart_port *port, int break_state)
{
	struct uart_8250_port *up = (struct uart_8250_port *)port;
	unsigned long flags;
	unsigned long rdata;

	spin_lock_irqsave(&up->port.lock, flags);
	if (break_state == -1)
		up->lcr |= UART_LCR_SBC;
	else
		up->lcr &= ~UART_LCR_SBC;

    rdata = serial_in(up, UARTCS_ADDRESS);
	if (up->lcr & UART_LCR_SBC)
    	rdata |= UARTCS_UARTTXBREAK_SET(1);
    else 
        rdata &= ~UARTCS_UARTTXBREAK_SET(1);
        
    serial_out(up, UARTCS_ADDRESS, rdata);    
	
	spin_unlock_irqrestore(&up->port.lock, flags);	
}

static int serial8250_startup(struct uart_port *port)
{
	struct uart_8250_port *up = (struct uart_8250_port *)port;
	unsigned long flags;
	unsigned char lsr, iir;
	int retval;
	unsigned int rdata;

    DEBUG_STEP();

	up->capabilities = uart_config[up->port.type].flags;
	up->mcr = 0;

	/*
	 * Clear the FIFO buffers and disable them.
	 * (they will be reeanbled in set_termios())
	 */
	serial8250_clear_fifos(up);

	/*
	 * Clear the interrupt registers.
	 */
    serial_in(up, UARTCS_ADDRESS);
    serial_in(up, UARTINT_ADDRESS);	

	/*
	 * If the "interrupt" for this port doesn't correspond with any
	 * hardware interrupt, we use a timer-based system.  The original
	 * driver used to do this with IRQ0.
	 */
	if (!is_real_interrupt(up->port.irq)) {
		unsigned int timeout = up->port.timeout;

		timeout = timeout > 6 ? (timeout / 2 - 2) : 1;
		up->timer.data = (unsigned long)up;
		mod_timer(&up->timer, jiffies + timeout);
	} else {
#ifdef AR9330_EV81847_WAR
		unsigned int timeout = up->port.timeout;

		timeout = timeout > 6 ? (timeout / 2 - 2) : 1;
		up->timer.data = (unsigned long)up;
        mod_timer(&up->timer, jiffies + timeout);
#endif

		retval = serial_link_irq_chain(up);
		if (retval)
			return retval;
	}

	/*
	 * Now, initialize the UART
	 */
	spin_lock_irqsave(&up->port.lock, flags);
	if (up->port.flags & UPF_FOURPORT) {
		if (!is_real_interrupt(up->port.irq))
			up->port.mctrl |= TIOCM_OUT1;
	} else
		/*
		 * Most PC uarts need OUT2 raised to enable interrupts.
		 */
		if (is_real_interrupt(up->port.irq))
			up->port.mctrl |= TIOCM_OUT2;

	serial8250_set_mctrl(&up->port, up->port.mctrl);

    up->bugs &= ~UART_BUG_TXEN;

	spin_unlock_irqrestore(&up->port.lock, flags);

	/*
	 * Finally, enable interrupts.  Note: Modem status interrupts
	 * are set via set_termios(), which will be occurring imminently
	 * anyway, so we don't enable them here.
	 */
	up->ier = UART_IER_RLSI | UART_IER_RDI;

	//rdata = serial_in(up, UARTINTEN_ADDRESS);
	rdata = UARTINTEN_UARTRXVALIDINTEN_SET(1);// | UARTINTEN_UARTTXREADYINTEN_SET(1);
    serial_out(up, UARTINTEN_ADDRESS, rdata); 	

	/*
	 * And clear the interrupt registers again for luck.
	 */
    serial_in(up, UARTINT_ADDRESS);

	return 0;
}

static void serial8250_shutdown(struct uart_port *port)
{
	struct uart_8250_port *up = (struct uart_8250_port *)port;
	unsigned long flags;
	unsigned int rdata;

	/*
	 * Disable interrupts from this port
	 */
	up->ier = 0;
    serial_out(up, UARTINTEN_ADDRESS, 0);  	

	spin_lock_irqsave(&up->port.lock, flags);
	if (up->port.flags & UPF_FOURPORT) {
		/* reset interrupts on the AST Fourport board */
		inb((up->port.iobase & 0xfe0) | 0x1f);
		up->port.mctrl |= TIOCM_OUT1;
	} else
		up->port.mctrl &= ~TIOCM_OUT2;

	serial8250_set_mctrl(&up->port, up->port.mctrl);
	spin_unlock_irqrestore(&up->port.lock, flags);

	/*
	 * Disable break condition and FIFOs
	 */
	rdata = serial_in(up, UARTCS_ADDRESS);
    rdata &= ~UARTCS_UARTTXBREAK_SET(1);        
    serial_out(up, UARTCS_ADDRESS, rdata);  
    	
	serial8250_clear_fifos(up);

	/*
	 * Read data port to reset things, and then unlink from
	 * the IRQ chain.
	 */
	if (!is_real_interrupt(up->port.irq))
		del_timer_sync(&up->timer);
	else {
#ifdef AR9330_EV81847_WAR
        del_timer_sync(&up->timer);
#endif
		serial_unlink_irq_chain(up);
    }
}

static unsigned int serial8250_get_divisor(struct uart_port *port, unsigned int baud)
{
	unsigned int quot;
	
	quot = ( port->uartclk / (16*baud) ) - 1;
	
	return quot;
}

static void
serial8250_set_termios(struct uart_port *port, struct ktermios *termios,
		       struct ktermios *old)
{
	struct uart_8250_port *up = (struct uart_8250_port *)port;
	unsigned char cval, fcr = 0;
	unsigned long flags;
	unsigned int baud, quot;
	unsigned int rdata;

	switch (termios->c_cflag & CSIZE) {
	case CS5:
		cval = UART_LCR_WLEN5;
		break;
	case CS6:
		cval = UART_LCR_WLEN6;
		break;
	case CS7:
		cval = UART_LCR_WLEN7;
		break;
	default:
	case CS8:
		cval = UART_LCR_WLEN8;
		break;
	}

	if (termios->c_cflag & CSTOPB)
		cval |= UART_LCR_STOP;
	if (termios->c_cflag & PARENB)
		cval |= UART_LCR_PARITY;
	if (!(termios->c_cflag & PARODD))
		cval |= UART_LCR_EPAR;
#ifdef CMSPAR
	if (termios->c_cflag & CMSPAR)
		cval |= UART_LCR_SPAR;
#endif

	/*
	 * Ask the core to calculate the divisor for us.
	 */
	baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk/16);
	quot = serial8250_get_divisor(port, baud);

	/*
	 * Oxford Semi 952 rev B workaround
	 */
	if (up->bugs & UART_BUG_QUOT && (quot & 0xff) == 0)
		quot ++;

	if (up->capabilities & UART_CAP_FIFO && up->port.fifosize > 1) {
		if (baud < 2400)
			fcr = UART_FCR_ENABLE_FIFO | UART_FCR_TRIGGER_1;
		else
			fcr = uart_config[up->port.type].fcr;
	}

	/*
	 * MCR-based auto flow control.  When AFE is enabled, RTS will be
	 * deasserted when the receive FIFO contains more characters than
	 * the trigger, or the MCR RTS bit is cleared.  In the case where
	 * the remote UART is not using CTS auto flow control, we must
	 * have sufficient FIFO entries for the latency of the remote
	 * UART to respond.  IOW, at least 32 bytes of FIFO.
	 */
	if (up->capabilities & UART_CAP_AFE && up->port.fifosize >= 32) {
		up->mcr &= ~UART_MCR_AFE;
		if (termios->c_cflag & CRTSCTS)
			up->mcr |= UART_MCR_AFE;
	}

	/*
	 * Ok, we're now changing the port state.  Do it with
	 * interrupts disabled.
	 */
	spin_lock_irqsave(&up->port.lock, flags);

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	up->port.read_status_mask = UART_LSR_OE | UART_LSR_THRE | UART_LSR_DR;
	if (termios->c_iflag & INPCK)
		up->port.read_status_mask |= UART_LSR_FE | UART_LSR_PE;
	if (termios->c_iflag & (BRKINT | PARMRK))
		up->port.read_status_mask |= UART_LSR_BI;

	/*
	 * Characteres to ignore
	 */
	up->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		up->port.ignore_status_mask |= UART_LSR_PE | UART_LSR_FE;
	if (termios->c_iflag & IGNBRK) {
		up->port.ignore_status_mask |= UART_LSR_BI;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			up->port.ignore_status_mask |= UART_LSR_OE;
	}

	/*
	 * ignore all characters if CREAD is not set
	 */
	if ((termios->c_cflag & CREAD) == 0)
		up->port.ignore_status_mask |= UART_LSR_DR;

	/*
	 * CTS flow control flag and modem status interrupts
	 */
	up->ier &= ~UART_IER_MSI;
	if (!(up->bugs & UART_BUG_NOMSR) &&
			UART_ENABLE_MS(&up->port, termios->c_cflag))
		up->ier |= UART_IER_MSI;
	if (up->capabilities & UART_CAP_UUE)
		up->ier |= UART_IER_UUE | UART_IER_RTOIE;

	rdata = serial_in(up, UARTCS_ADDRESS);
	rdata |= UARTCS_UARTHOSTINTEN_SET(1);
    serial_out(up, UARTCS_ADDRESS, rdata);

	up->lcr = cval;					/* Save LCR */

	serial8250_set_mctrl(&up->port, up->port.mctrl);
	spin_unlock_irqrestore(&up->port.lock, flags);
}

static void
serial8250_pm(struct uart_port *port, unsigned int state,
	      unsigned int oldstate)
{
	struct uart_8250_port *p = (struct uart_8250_port *)port;

	serial8250_set_sleep(p, state != 0);

	if (p->pm)
		p->pm(port, state, oldstate);
}

static unsigned int serial8250_port_size(struct uart_8250_port *pt)
{
	if (pt->port.iotype == UPIO_AU)
		return 0x100000;
#ifdef CONFIG_ARCH_OMAP
	if (is_omap_port(pt))
		return 0x16 << pt->port.regshift;
#endif
	return 8 << pt->port.regshift;
}

/*
 * Resource handling.
 */
static int serial8250_request_std_resource(struct uart_8250_port *up)
{
	unsigned int size = serial8250_port_size(up);
	int ret = 0;

	switch (up->port.iotype) {
	case UPIO_MEM:
		if (!up->port.mapbase)
			break;

		if (!request_mem_region(up->port.mapbase, size, "serial")) {
			ret = -EBUSY;
			break;
		}

		if (up->port.flags & UPF_IOREMAP) {
			up->port.membase = ioremap(up->port.mapbase, size);
			if (!up->port.membase) {
				release_mem_region(up->port.mapbase, size);
				ret = -ENOMEM;
			}
		}
		break;

	case UPIO_HUB6:
	case UPIO_PORT:
		if (!request_region(up->port.iobase, size, "serial"))
			ret = -EBUSY;
		break;
	}
	return ret;
}

static void serial8250_release_std_resource(struct uart_8250_port *up)
{
	unsigned int size = serial8250_port_size(up);
	switch (up->port.iotype) {
	case UPIO_MEM:
		if (!up->port.mapbase)
			break;

		if (up->port.flags & UPF_IOREMAP) {
			iounmap(up->port.membase);
			up->port.membase = NULL;
		}

		release_mem_region(up->port.mapbase, size);
		break;

	case UPIO_HUB6:
	case UPIO_PORT:
		release_region(up->port.iobase, size);
		break;
	}
}

static int serial8250_request_rsa_resource(struct uart_8250_port *up)
{
	unsigned long start = UART_RSA_BASE << up->port.regshift;
	unsigned int size = 8 << up->port.regshift;
	int ret = -EINVAL;

	switch (up->port.iotype) {
	case UPIO_MEM:
		ret = -EINVAL;
		break;

	case UPIO_HUB6:
	case UPIO_PORT:
		start += up->port.iobase;
		if (request_region(start, size, "serial-rsa"))
			ret = 0;
		else
			ret = -EBUSY;
		break;
	}

	return ret;
}

static void serial8250_release_rsa_resource(struct uart_8250_port *up)
{
	unsigned long offset = UART_RSA_BASE << up->port.regshift;
	unsigned int size = 8 << up->port.regshift;

	switch (up->port.iotype) {
	case UPIO_MEM:
		break;

	case UPIO_HUB6:
	case UPIO_PORT:
		release_region(up->port.iobase + offset, size);
		break;
	}
}

static void serial8250_release_port(struct uart_port *port)
{
	struct uart_8250_port *up = (struct uart_8250_port *)port;

	serial8250_release_std_resource(up);
	if (up->port.type == PORT_RSA)
		serial8250_release_rsa_resource(up);
}

static int serial8250_request_port(struct uart_port *port)
{
	struct uart_8250_port *up = (struct uart_8250_port *)port;
	int ret = 0;

	ret = serial8250_request_std_resource(up);
	if (ret == 0 && up->port.type == PORT_RSA) {
		ret = serial8250_request_rsa_resource(up);
		if (ret < 0)
			serial8250_release_std_resource(up);
	}

	return ret;
}

static void serial8250_config_port(struct uart_port *port, int flags)
{
	struct uart_8250_port *up = (struct uart_8250_port *)port;
	int probeflags = PROBE_ANY;
	int ret;

    DEBUG_STEP();

	/*
	 * Find the region that we can probe for.  This in turn
	 * tells us whether we can probe for the type of port.
	 */
	ret = serial8250_request_std_resource(up);
	if (ret < 0) {
	DEBUG_STEP();
		return;
	}

	ret = serial8250_request_rsa_resource(up);
	if (ret < 0)
		probeflags &= ~PROBE_RSA;

	if (flags & UART_CONFIG_TYPE)
		autoconfig(up, probeflags);
	if (up->port.type != PORT_UNKNOWN && flags & UART_CONFIG_IRQ)
		autoconfig_irq(up);

	if (up->port.type != PORT_RSA && probeflags & PROBE_RSA)
		serial8250_release_rsa_resource(up);
	if (up->port.type == PORT_UNKNOWN)
		serial8250_release_std_resource(up);
}

static int
serial8250_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	if (ser->irq >= nr_irqs || ser->irq < 0 ||
	    ser->baud_base < 9600 || ser->type < PORT_UNKNOWN ||
	    ser->type >= ARRAY_SIZE(uart_config) || ser->type == PORT_CIRRUS ||
	    ser->type == PORT_STARTECH)
		return -EINVAL;
	return 0;
}

static const char *
serial8250_type(struct uart_port *port)
{
	int type = port->type;

	if (type >= ARRAY_SIZE(uart_config))
		type = 0;
	return uart_config[type].name;
}

static struct uart_ops serial8250_pops = {
	.tx_empty	= serial8250_tx_empty,
	.set_mctrl	= serial8250_set_mctrl,
	.get_mctrl	= serial8250_get_mctrl,
	.stop_tx	= serial8250_stop_tx,
	.start_tx	= serial8250_start_tx,
	.stop_rx	= serial8250_stop_rx,
	.enable_ms	= serial8250_enable_ms,
	.break_ctl	= serial8250_break_ctl,
	.startup	= serial8250_startup,
	.shutdown	= serial8250_shutdown,
	.set_termios	= serial8250_set_termios,
	.pm		= serial8250_pm,
	.type		= serial8250_type,
	.release_port	= serial8250_release_port,
	.request_port	= serial8250_request_port,
	.config_port	= serial8250_config_port,
	.verify_port	= serial8250_verify_port,
};

static struct uart_8250_port serial8250_ports[UART_NR];

static void __init serial8250_isa_init_ports(void)
{
	struct uart_8250_port *up;
	static int first = 1;
	int i;

    DEBUG_STEP();
	if (!first) {
	    DEBUG_STEP();
		return;
	}
	first = 0;

	for (i = 0; i < nr_uarts; i++) {
		up = &serial8250_ports[i];

		up->port.line = i;
		spin_lock_init(&up->port.lock);

		init_timer(&up->timer);
		up->timer.function = serial8250_timeout;

		/*
		 * ALPHA_KLUDGE_MCR needs to be killed.
		 */
		up->mcr_mask = ~ALPHA_KLUDGE_MCR;
		up->mcr_force = ALPHA_KLUDGE_MCR;

		up->port.ops = &serial8250_pops;
	}

	for (i = 0, up = serial8250_ports;
	     i < ARRAY_SIZE(old_serial_port) && i < nr_uarts;
	     i++, up++) {

		up->port.iobase   = old_serial_port[i].port;
		up->port.irq      = irq_canonicalize(old_serial_port[i].irq);
		up->port.uartclk  = old_serial_port[i].baud_base * 16;
		up->port.flags    = old_serial_port[i].flags;
		up->port.hub6     = old_serial_port[i].hub6;
		up->port.membase  = old_serial_port[i].iomem_base;
		up->port.iotype   = old_serial_port[i].io_type;
		up->port.regshift = old_serial_port[i].iomem_reg_shift;

		set_io_from_upio(&up->port);
		if (share_irqs)
			up->port.flags |= UPF_SHARE_IRQ;
	}
}

static void __init
serial8250_register_ports(struct uart_driver *drv, struct device *dev)
{
	int i;

    DEBUG_STEP();
    
	for (i = 0; i < nr_uarts; i++) {
	    DEBUG_STEP();
		struct uart_8250_port *up = &serial8250_ports[i];
		up->cur_iotype = 0xFF;
	}

    DEBUG_STEP();
	serial8250_isa_init_ports();

	for (i = 0; i < nr_uarts; i++) {
		struct uart_8250_port *up = &serial8250_ports[i];

	    DEBUG_STEP();
		up->port.dev = dev;
		uart_add_one_port(drv, &up->port);
	}
}

#ifdef CONFIG_SERIAL_AR933X_CONSOLE

#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)

/*
 *	Wait for transmitter & holding register to empty
 */

static inline void wait_for_xmitr(struct uart_8250_port *up)
{
	unsigned int status, tmout = 60000;

	/* Wait up to 10ms for the character(s) to be sent. */
	do {
		status = serial_in(up, UARTDATA_ADDRESS);

		if (--tmout == 0)
			break;
		udelay(1);
	} while (UARTDATA_UARTTXCSR_GET(status) == 0);
}

/*
 *	Print a string to the serial port trying not to disturb
 *	any possible real use of the port...
 *
 *	The console_lock must be held when we get here.
 */
static void
serial8250_console_write(struct console *co, const char *s, unsigned int count)
{
	struct uart_8250_port *up = &serial8250_ports[co->index];
	unsigned int ier;
	int i;
    unsigned int rdata;
	int locked = 1;
	unsigned long flags;

	touch_nmi_watchdog();

	local_irq_save(flags);
	if (up->port.sysrq) {
		/* serial8250_handle_port() already took the lock */
		locked = 0;
	} else if (oops_in_progress) {
		locked = spin_trylock(&up->port.lock);
	} else
		spin_lock(&up->port.lock);

	/*
	 *	First save the IER then disable the interrupts
	 */
	ier = serial_in(up, UARTINTEN_ADDRESS);
    serial_out(up, UARTINTEN_ADDRESS, 0);

	/*
	 *	Now, do each character
	 */
	for (i = 0; i < count; i++, s++) {
		wait_for_xmitr(up);

		/*
		 *	Send the character out.
		 *	If a LF, also do CR...
		 */
        rdata = UARTDATA_UARTTXRXDATA_SET((unsigned int)(*s));
        rdata |= UARTDATA_UARTTXCSR_SET(1);        
		serial_out(up, UARTDATA_ADDRESS, rdata);
		
		if (*s == 10) {
			wait_for_xmitr(up);
            rdata = UARTDATA_UARTTXRXDATA_SET(13);
            rdata |= UARTDATA_UARTTXCSR_SET(1);			
			serial_out(up, UARTDATA_ADDRESS, rdata);
		}
	}

	/*
	 *	Finally, wait for transmitter to become empty
	 *	and restore the IER
	 */
	wait_for_xmitr(up);
	serial_out(up, UARTINTEN_ADDRESS, ier);
    serial_out(up, UARTINT_ADDRESS, UARTINT_RSTMASK);

	if (locked)
		spin_unlock(&up->port.lock);
	local_irq_restore(flags);
}

int serial8250_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	/*
	 * Check whether an invalid uart number has been specified, and
	 * if so, search for the first available port that does have
	 * console support.
	 */
	if (co->index >= nr_uarts)
		co->index = 0;
	port = &serial8250_ports[co->index].port;
	if (!port->iobase && !port->membase)
		return -ENODEV;

	if (options) {
		uart_parse_options(options, &baud, &parity, &bits, &flow);
    }

	return uart_set_options(port, co, baud, parity, bits, flow);
}

static int serial8250_console_early_setup(void)
{
//	return serial8250_find_port_for_earlycon();
}

static struct console serial8250_console = {
	.name		= "ttyS",
	.write		= serial8250_console_write,
	.device		= uart_console_device,
	.setup		= serial8250_console_setup,
	.early_setup	= serial8250_console_early_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &serial8250_reg,
};

static int __init serial8250_console_init(void)
{
	if (nr_uarts > UART_NR)
		nr_uarts = UART_NR;
printk(KERN_EMERG "init serial console\n");
	serial8250_isa_init_ports();
	register_console(&serial8250_console);
	
	DEBUG_STEP();

	return 0;
}
console_initcall(serial8250_console_init);

int serial8250_find_port(struct uart_port *p)
{
	int line;
	struct uart_port *port;

	for (line = 0; line < nr_uarts; line++) {
		port = &serial8250_ports[line].port;
		if (p->iotype == port->iotype &&
		    p->iobase == port->iobase &&
		    p->membase == port->membase)
			return line;
	}
	return -ENODEV;
}

#if 0
int __init serial8250_start_console(struct uart_port *port, char *options)
{
	int line;

	line = find_port(port);
	if (line < 0)
		return -ENODEV;

	add_preferred_console("ttyS", line, options);
	printk("Adding console on ttyS%d at %s 0x%lx (options '%s')\n",
		line, port->iotype == UPIO_MEM ? "MMIO" : "I/O port",
		port->iotype == UPIO_MEM ? (unsigned long) port->mapbase :
		    (unsigned long) port->iobase, options);
	if (!(serial8250_console.flags & CON_ENABLED)) {
		serial8250_console.flags &= ~CON_PRINTBUFFER;
		register_console(&serial8250_console);
	}
	return line;
}
#endif

#define SERIAL8250_CONSOLE	&serial8250_console
#else
#define SERIAL8250_CONSOLE	NULL
#endif

static struct uart_driver serial8250_reg = {
	.owner			= THIS_MODULE,
	.driver_name		= "serial",
	.dev_name		= "ttyS",
	.major			= TTY_MAJOR,
	.minor			= 64,
	.cons			= SERIAL8250_CONSOLE,
};

int __init early_serial_setup(struct uart_port *port)
{
	struct uart_port *p;

	if (port->line >= ARRAY_SIZE(serial8250_ports))
		return -ENODEV;

	serial8250_isa_init_ports();
	p = &serial8250_ports[port->line].port;
	p->iobase       = port->iobase;
	p->membase      = port->membase;
	p->irq          = port->irq;
	p->uartclk      = port->uartclk;
	p->fifosize     = port->fifosize;
	p->regshift     = port->regshift;
	p->iotype       = port->iotype;
	p->flags        = port->flags;
	p->mapbase      = port->mapbase;
	p->private_data = port->private_data;
	p->type		= port->type;
	p->line		= port->line;

	set_io_from_upio(p);
	if (port->serial_in)
		p->serial_in = port->serial_in;
	if (port->serial_out)
		p->serial_out = port->serial_out;

	return 0;
}

/**
 *	serial8250_suspend_port - suspend one serial port
 *	@line:  serial line number
 *      @level: the level of port suspension, as per uart_suspend_port
 *
 *	Suspend one serial port.
 */
void serial8250_suspend_port(int line)
{
	uart_suspend_port(&serial8250_reg, &serial8250_ports[line].port);
}

/**
 *	serial8250_resume_port - resume one serial port
 *	@line:  serial line number
 *      @level: the level of port resumption, as per uart_resume_port
 *
 *	Resume one serial port.
 */
void serial8250_resume_port(int line)
{
	struct uart_8250_port *up = &serial8250_ports[line];

	uart_resume_port(&serial8250_reg, &up->port);
}

/*
 * Register a set of serial devices attached to a platform device.  The
 * list is terminated with a zero flags entry, which means we expect
 * all entries to have at least UPF_BOOT_AUTOCONF set.
 */
static int serial8250_probe(struct platform_device *dev)
{
	struct plat_serial8250_port *p = dev->dev.platform_data;
	struct uart_port port;
	int ret, i;

	memset(&port, 0, sizeof(struct uart_port));

	for (i = 0; p && p->flags != 0; p++, i++) {
		port.iobase	= p->iobase;
		port.membase	= p->membase;
		port.irq	= p->irq;
		port.uartclk	= p->uartclk;
		port.regshift	= p->regshift;
		port.iotype	= p->iotype;
		port.flags	= p->flags;
		port.mapbase	= p->mapbase;
		port.hub6	= p->hub6;
		port.private_data	= p->private_data;
		port.type		= p->type;
		port.serial_in		= p->serial_in;
		port.serial_out		= p->serial_out;
		port.dev		= &dev->dev;
		if (share_irqs)
			port.flags |= UPF_SHARE_IRQ;
		ret = serial8250_register_port(&port);
		if (ret < 0) {
		    DEBUG_STEP();
			dev_err(&dev->dev, "unable to register port at index %d "
				"(IO%lx MEM%lx IRQ%d): %d\n", i,
				p->iobase, p->mapbase, p->irq, ret);
		}
	}
	return 0;
}

/*
 * Remove serial ports registered against a platform device.
 */
static int serial8250_remove(struct platform_device *dev)
{
	int i;

	for (i = 0; i < nr_uarts; i++) {
		struct uart_8250_port *up = &serial8250_ports[i];

		if (up->port.dev == &dev->dev)
			serial8250_unregister_port(i);
	}
	return 0;
}

static int serial8250_suspend(struct platform_device *dev, pm_message_t state)
{
	int i;

	for (i = 0; i < UART_NR; i++) {
		struct uart_8250_port *up = &serial8250_ports[i];

		if (up->port.type != PORT_UNKNOWN && up->port.dev == &dev->dev)
			uart_suspend_port(&serial8250_reg, &up->port);
	}

	return 0;
}

static int serial8250_resume(struct platform_device *dev)
{
	int i;

	for (i = 0; i < UART_NR; i++) {
		struct uart_8250_port *up = &serial8250_ports[i];

		if (up->port.type != PORT_UNKNOWN && up->port.dev == &dev->dev)
			uart_resume_port(&serial8250_reg, &up->port);
	}

	return 0;
}

static struct platform_driver serial8250_isa_driver = {
	.probe		= serial8250_probe,
	.remove		= serial8250_remove,
	.suspend	= serial8250_suspend,
	.resume		= serial8250_resume,
	.driver		= {
	.name	= "serial8250",
	.owner	= THIS_MODULE,
	},
};

/*
 * This "device" covers _all_ ISA 8250-compatible serial devices listed
 * in the table in include/asm/serial.h
 */
static struct platform_device *serial8250_isa_devs;

/*
 * serial8250_register_port and serial8250_unregister_port allows for
 * 16x50 serial ports to be configured at run-time, to support PCMCIA
 * modems and PCI multiport cards.
 */
static DEFINE_MUTEX(serial_mutex);

static struct uart_8250_port *serial8250_find_match_or_unused(struct uart_port *port)
{
	int i;

	/*
	 * First, find a port entry which matches.
	 */
	for (i = 0; i < nr_uarts; i++)
		if (uart_match_port(&serial8250_ports[i].port, port))
			return &serial8250_ports[i];

	/*
	 * We didn't find a matching entry, so look for the first
	 * free entry.  We look for one which hasn't been previously
	 * used (indicated by zero iobase).
	 */
	for (i = 0; i < nr_uarts; i++)
		if (serial8250_ports[i].port.type == PORT_UNKNOWN &&
		    serial8250_ports[i].port.iobase == 0)
			return &serial8250_ports[i];

	/*
	 * That also failed.  Last resort is to find any entry which
	 * doesn't have a real port associated with it.
	 */
	for (i = 0; i < nr_uarts; i++)
		if (serial8250_ports[i].port.type == PORT_UNKNOWN)
			return &serial8250_ports[i];

	return NULL;
}

/**
 *	serial8250_register_port - register a serial port
 *	@port: serial port template
 *
 *	Configure the serial port specified by the request. If the
 *	port exists and is in use, it is hung up and unregistered
 *	first.
 *
 *	The port is then probed and if necessary the IRQ is autodetected
 *	If this fails an error is returned.
 *
 *	On success the port is ready to use and the line number is returned.
 */
int serial8250_register_port(struct uart_port *port)
{
	struct uart_8250_port *uart;
	int ret = -ENOSPC;

    if(!port) {
		return -EINVAL;
    }

	if (port->uartclk == 0)
		return -EINVAL;

	mutex_lock(&serial_mutex);

	uart = serial8250_find_match_or_unused(port);

	if (uart) {
		uart_remove_one_port(&serial8250_reg, &uart->port);

		uart->port.iobase   = port->iobase;
		uart->port.membase  = port->membase;
		uart->port.irq      = port->irq;
		uart->port.uartclk  = port->uartclk;
		uart->port.fifosize = port->fifosize;
		uart->port.regshift = port->regshift;
		uart->port.iotype   = port->iotype;
		uart->port.flags    = port->flags | UPF_BOOT_AUTOCONF;
		uart->port.mapbase  = port->mapbase;
		uart->port.private_data = port->private_data;
		if (port->dev)
			uart->port.dev = port->dev;

		ret = uart_add_one_port(&serial8250_reg, &uart->port);

		if (ret == 0) {
			ret = uart->port.line;
		}
	}
	mutex_unlock(&serial_mutex);

	return ret;
}
EXPORT_SYMBOL(serial8250_register_port);

/**
 *	serial8250_unregister_port - remove a 16x50 serial port at runtime
 *	@line: serial line number
 *
 *	Remove one serial port.  This may not be called from interrupt
 *	context.  We hand the port back to the our control.
 */
void serial8250_unregister_port(int line)
{
	struct uart_8250_port *uart = &serial8250_ports[line];

	mutex_lock(&serial_mutex);
	uart_remove_one_port(&serial8250_reg, &uart->port);
	if (serial8250_isa_devs) {
		uart->port.flags &= ~UPF_BOOT_AUTOCONF;
		uart->port.type = PORT_UNKNOWN;
		uart->port.dev = &serial8250_isa_devs->dev;
		uart_add_one_port(&serial8250_reg, &uart->port);
	} else {
		uart->port.dev = NULL;
	}
	mutex_unlock(&serial_mutex);
}
EXPORT_SYMBOL(serial8250_unregister_port);

static int __init serial8250_init(void)
{
	int ret;

	if (nr_uarts > UART_NR)
		nr_uarts = UART_NR;

	printk(KERN_INFO "Serial: 8250/16550 driver, "
		"%d ports, IRQ sharing %sabled\n", nr_uarts,
		share_irqs ? "en" : "dis");

#ifdef CONFIG_SPARC
	ret = sunserial_register_minors(&serial8250_reg, UART_NR);
#else
	serial8250_reg.nr = UART_NR;
	ret = uart_register_driver(&serial8250_reg);
#endif
	if (ret) {
		goto out;
		}

	serial8250_isa_devs = platform_device_alloc("serial8250",
						    PLAT8250_DEV_LEGACY);
	if (!serial8250_isa_devs) {
		ret = -ENOMEM;
		goto unreg_uart_drv;
	}

	ret = platform_device_add(serial8250_isa_devs);
	if (ret) {
		goto put_dev;
	}

	serial8250_register_ports(&serial8250_reg, &serial8250_isa_devs->dev);
	ret = platform_driver_register(&serial8250_isa_driver);

	if (ret == 0) {
		goto out;
    }

	platform_device_del(serial8250_isa_devs);
put_dev:
	platform_device_put(serial8250_isa_devs);
unreg_uart_drv:
#ifdef CONFIG_SPARC
	sunserial_unregister_minors(&serial8250_reg, UART_NR);
#else
	uart_unregister_driver(&serial8250_reg);
#endif
out:
	return ret;
}

static void __exit serial8250_exit(void)
{
	struct platform_device *isa_dev = serial8250_isa_devs;

	/*
	 * This tells serial8250_unregister_port() not to re-register
	 * the ports (thereby making serial8250_isa_driver permanently
	 * in use.)
	 */
	serial8250_isa_devs = NULL;

	platform_driver_unregister(&serial8250_isa_driver);
	platform_device_unregister(isa_dev);

#ifdef CONFIG_SPARC
	sunserial_unregister_minors(&serial8250_reg, UART_NR);
#else
	uart_unregister_driver(&serial8250_reg);
#endif
}

module_init(serial8250_init);
module_exit(serial8250_exit);

EXPORT_SYMBOL(serial8250_suspend_port);
EXPORT_SYMBOL(serial8250_resume_port);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Generic 8250/16x50 serial driver");

module_param(share_irqs, uint, 0644);
MODULE_PARM_DESC(share_irqs, "Share IRQs with other non-8250/16x50 devices"
	" (unsafe)");

module_param(nr_uarts, uint, 0644);
MODULE_PARM_DESC(nr_uarts, "Maximum number of UARTs supported. (1-" __MODULE_STRING(CONFIG_SERIAL_8250_NR_UARTS) ")");

#ifdef CONFIG_SERIAL_8250_RSA
module_param_array(probe_rsa, ulong, &probe_rsa_count, 0444);
MODULE_PARM_DESC(probe_rsa, "Probe I/O ports for RSA");
#endif
MODULE_ALIAS_CHARDEV_MAJOR(TTY_MAJOR);
