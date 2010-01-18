/*
 *  linux/drivers/char/serial_uart00.c
 *
 *  Driver for UART00 serial ports
 *
 *  Based on drivers/char/serial_amba.c, by ARM Limited &
 *                                          Deep Blue Solutions Ltd.
 *  Copyright 2001 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id: serial_sl2312.c,v 1.1.1.1 2006/04/03 08:41:00 amos_lee Exp $
 *
 */
#include <linux/module.h>

#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/circ_buf.h>
#include <linux/serial.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/serial_core.h>

#include <asm/system.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <asm/sizes.h>
#include <linux/spinlock.h>
#include <linux/irq.h>


#if defined(CONFIG_SERIAL_SL2312_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <asm/arch/sl2312.h>
#define UART_TYPE (volatile unsigned int*)
#include <asm/arch/uart.h>
#include <asm/arch/int_ctrl.h>

// #define DEBUG           1
#define UART_NR		1


#define SERIAL_SL2312_NAME	"ttyS"
#define SERIAL_SL2312_MAJOR	204
#define SERIAL_SL2312_MINOR	40      /* Temporary - will change in future */
#define SERIAL_SL2312_NR	UART_NR
#define UART_PORT_SIZE 0x50

#define SL2312_NO_PORTS         UART_NR
#define SL2312_ISR_PASS_LIMIT	256

/*
 * Access macros for the SL2312 UARTs
 */
#define UART_GET_INT_STATUS(p)	(inl(UART_IIR((p)->membase)) & 0x0F)      // interrupt identification
#define UART_PUT_IER(p, c)      outl(c,UART_IER((p)->membase))    // interrupt enable
#define UART_GET_IER(p)         inl(UART_IER((p)->membase))
#define UART_PUT_CHAR(p, c)     outl(c,UART_THR((p)->membase))    // transmitter holding
#define UART_GET_CHAR(p)        inl(UART_RBR((p)->membase))       // receive buffer
#define UART_GET_LSR(p)         inl(UART_LSR((p)->membase))       // line status
#define UART_GET_MSR(p)         inl(UART_MSR((p)->membase))       // modem status
#define UART_GET_MCR(p)         inl(UART_MCR((p)->membase))       // modem control
#define UART_PUT_MCR(p, c)      outl(c,UART_MCR((p)->membase))
#define UART_GET_LCR(p)         inl(UART_LCR((p)->membase))       // mode control
#define UART_PUT_LCR(p, c)      outl(c,UART_LCR((p)->membase))
#define UART_GET_DIV_HI(p)	inl(UART_DIV_HI((p)->membase))
#define UART_PUT_DIV_HI(p, c)	outl(c,UART_DIV_HI((p)->membase))
#define UART_GET_DIV_LO(p)	inl(UART_DIV_LO((p)->membase))
#define UART_PUT_DIV_LO(p, c)	outl(c,UART_DIV_LO((p)->membase))
#define UART_PUT_MDR(p, c)      outl(c,UART_MDR((p)->membase))
#define UART_RX_DATA(s)		((s) & UART_LSR_DR)
#define UART_TX_READY(s)	((s) & UART_LSR_THRE)


static void sl2312_stop_tx(struct uart_port *port)
{
        unsigned int reg;

//        printk("sl2312 stop tx : \n");
        reg = UART_GET_IER(port);
        reg &= ~(UART_IER_TE);
	UART_PUT_IER(port, reg);
}

static void sl2312_stop_rx(struct uart_port *port)
{
        unsigned int reg;

//        printk("sl2312 stop rx : \n");
        reg = UART_GET_IER(port);
        reg &= ~(UART_IER_DR);
	UART_PUT_IER(port, reg);

}

static void sl2312_enable_ms(struct uart_port *port)
{
        unsigned int reg;

//        printk("sl2312 enable ms : \n");

        reg = UART_GET_IER(port);
        reg |= (UART_IER_MS);
	UART_PUT_IER(port, reg);

}

static void
sl2312_rx_chars(struct uart_port *port)
{
	struct tty_struct *tty = port->info->tty;
	unsigned int status, mask, ch, flg, ignored = 0;


 //       printk("sl2312_rx_chars : \n");
	status = UART_GET_LSR(port);
	while (UART_RX_DATA(status)) {

		/*
		 * We need to read rds before reading the
		 * character from the fifo
		 */
		ch = UART_GET_CHAR(port);
		port->icount.rx++;

		//if (tty->flip.count >= TTY_FLIPBUF_SIZE)
		if (tty && !tty_buffer_request_room(tty, 1))
			goto ignore_char;

		flg = TTY_NORMAL;

		/*
		 * Note that the error handling code is
		 * out of the main execution path
		 */

		if (status & (UART_LSR_OE|UART_LSR_PE|UART_LSR_FE|UART_LSR_BI|UART_LSR_DE))
			goto handle_error;
		if (uart_handle_sysrq_char(port, ch))
			goto ignore_char;

	error_return:
		//*tty->flip.flag_buf_ptr++ = flg;
		//*tty->flip.char_buf_ptr++ = ch;
		//tty->flip.count++;
		tty_insert_flip_char(tty, ch, flg);
	ignore_char:
		status = UART_GET_LSR(port);
	} // end of while
out:
	tty_flip_buffer_push(tty);
	return;

handle_error:
	if (status & UART_LSR_BI) {
		status &= ~(UART_LSR_FE);
		port->icount.brk++;

#ifdef SUPPORT_SYSRQ
		if (uart_handle_break(port))
			goto ignore_char;
#endif
	} else if (status & UART_LSR_PE)
		port->icount.parity++;
	else if (status & UART_LSR_FE)
		port->icount.frame++;

	if (status & UART_LSR_OE)
		port->icount.overrun++;

	if (status & port->ignore_status_mask) {
		if (++ignored > 100)
			goto out;
		goto ignore_char;
	}

	mask = status & port->read_status_mask;

	if (mask & UART_LSR_BI)
		flg = TTY_BREAK;
	else if (mask & UART_LSR_PE)
		flg = TTY_PARITY;
	else if (mask & UART_LSR_FE)
		flg = TTY_FRAME;

	if (status & UART_LSR_OE) {
		/*
		 * CHECK: does overrun affect the current character?
		 * ASSUMPTION: it does not.
		 */
		//*tty->flip.flag_buf_ptr++ = flg;
		//*tty->flip.char_buf_ptr++ = ch;
		//tty->flip.count++;

		tty_insert_flip_char(tty, 0, TTY_BREAK);

		// if (tty->flip.count >= TTY_FLIPBUF_SIZE)
		if (tty_buffer_request_room(tty, 1))
			goto ignore_char;
		ch = 0;
		flg = TTY_OVERRUN;
	}
#ifdef SUPPORT_SYSRQ
	port->sysrq = 0;
#endif
	goto error_return;
}

static void sl2312_tx_chars(struct uart_port *port)
{
	struct circ_buf *xmit = &port->info->xmit;
	int count;


	if (port->x_char) {
		while(!(UART_GET_LSR(port)&UART_LSR_THRE));
		UART_PUT_CHAR(port, port->x_char);
		port->icount.tx++;
		port->x_char = 0;

		return;
	}
	if (uart_circ_empty(xmit) || uart_tx_stopped(port)) {
		sl2312_stop_tx(port);

		return;
	}

	count = port->fifosize >> 1;
	do {
		while(!(UART_GET_LSR(port)&UART_LSR_THRE));
		UART_PUT_CHAR(port, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	} while (--count > 0);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	if (uart_circ_empty(xmit))
		sl2312_stop_tx(port);

}

static void sl2312_start_tx(struct uart_port *port)
{
        unsigned int reg;

//        printk("sl2312 start tx : \n");
        reg = UART_GET_IER(port);
        reg |= (UART_IER_TE);
	UART_PUT_IER(port, reg);

	sl2312_tx_chars(port);
}

static void sl2312_modem_status(struct uart_port *port)
{
	unsigned int status;

//        printk("it8712 modem status : \n");

	status = UART_GET_MSR(port);

	if (!(status & (UART_MSR_DCTS | UART_MSR_DDSR |
		       UART_MSR_TERI | UART_MSR_DDCD)))
		return;

        if (status & UART_MSR_DDCD)
                uart_handle_dcd_change(port, status & UART_MSR_DCD);

        if (status & UART_MSR_DDSR)
                port->icount.dsr++;

        if (status & UART_MSR_DCTS)
                uart_handle_cts_change(port, status & UART_MSR_CTS);

	wake_up_interruptible(&port->info->delta_msr_wait);

}

static irqreturn_t sl2312_int(int irq, void *dev_id)
{
	struct uart_port *port = dev_id;
	unsigned int status, pass_counter = 0;

	status = UART_GET_INT_STATUS(port);
	do {
		switch(status)
		{
		   case UART_IIR_DR:
		   case UART_IIR_RLS:
			sl2312_rx_chars(port);
		   break;
		   case UART_IIR_TE:
			sl2312_tx_chars(port);
		   break;
		   case UART_IIR_MODEM:
			sl2312_modem_status(port);
		   break;
		   default:
		   break;
		}
		if (pass_counter++ > SL2312_ISR_PASS_LIMIT)
			break;

		status = UART_GET_INT_STATUS(port);
	} while (status);

	return IRQ_HANDLED;
}

static u_int sl2312_tx_empty(struct uart_port *port)
{
//        printk("sl2312 tx empty : \n");

	return ((UART_GET_LSR(port) & UART_LSR_TE)? TIOCSER_TEMT : 0);
}

static u_int sl2312_get_mctrl(struct uart_port *port)
{
	unsigned int result = 0;
	unsigned int status;

//        printk("sl2312 get mctrl : \n");

	status = UART_GET_MSR(port);
	if (status & UART_MSR_DCD)
		result |= TIOCM_CAR;
	if (status & UART_MSR_DSR)
		result |= TIOCM_DSR;
	if (status & UART_MSR_CTS)
		result |= TIOCM_CTS;
	if (status & UART_MSR_RI)
		result |= TIOCM_RI;

	return result;
}

static void sl2312_set_mctrl_null(struct uart_port *port, u_int mctrl)
{
}

static void sl2312_break_ctl(struct uart_port *port, int break_state)
{
	unsigned int lcr;

//        printk("sl2312 break ctl : \n");

	lcr = UART_GET_LCR(port);
	if (break_state == -1)
		lcr |= UART_LCR_SETBREAK;
	else
		lcr &= ~UART_LCR_SETBREAK;
	UART_PUT_LCR(port, lcr);
}

static inline u_int uart_calculate_quot(struct uart_port *port, u_int baud)
{
	u_int quot;

	/* Special case: B0 rate */
	if (!baud)
		baud = 9600;

	quot = (port->uartclk / (16 * baud)-1)  ;

	return quot;
}

static void sl2312_set_termios(struct uart_port *port, struct ktermios *termios,
                               struct ktermios *old)
{
	unsigned int  uart_mc, old_ier, baud, quot;
	unsigned long flags;

        termios->c_cflag |= CREAD;
#ifdef DEBUG
	printk("it8712_set_cflag(0x%x) called\n", cflag);
#endif
        baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk/16);
        quot = (port->uartclk / (16 * baud))  ;
        //uart_get_divisor(port, baud);

	/* byte size and parity */
	switch (termios->c_cflag & CSIZE) {
	case CS5:
              uart_mc = UART_LCR_LEN5;
              break;
	case CS6:
              uart_mc = UART_LCR_LEN6;
              break;
	case CS7:
              uart_mc = UART_LCR_LEN7;
              break;
	default: // CS8
              uart_mc = UART_LCR_LEN8;
              break;
	}

	if (termios->c_cflag & CSTOPB)
		uart_mc|= UART_LCR_STOP;
	if (termios->c_cflag & PARENB) {
		uart_mc |= UART_LCR_EVEN;
		if (!(termios->c_cflag & PARODD))
			uart_mc |= UART_LCR_ODD;
	}

    spin_lock_irqsave(&port->lock, flags);
        /*
         * Update the per-port timeout
         */
        uart_update_timeout(port, termios->c_cflag, baud);
	port->read_status_mask = UART_LSR_OE;
	if (termios->c_iflag & INPCK)
		port->read_status_mask |= UART_LSR_FE | UART_LSR_PE;
	if (termios->c_iflag & (BRKINT | PARMRK))
		port->read_status_mask |= UART_LSR_BI;

	/*
	 * Characters to ignore
	 */
	port->ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		port->ignore_status_mask |= UART_LSR_FE | UART_LSR_PE;
	if (termios->c_iflag & IGNBRK) {
		port->ignore_status_mask |= UART_LSR_BI;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns to (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			port->ignore_status_mask |= UART_LSR_OE;
	}

	//save_flags(flags); cli();
	old_ier = UART_GET_IER(port);

        if(UART_ENABLE_MS(port, termios->c_cflag))
             old_ier |= UART_IER_MS;

	/* Set baud rate */
	UART_PUT_LCR(port, UART_LCR_DLAB);
	UART_PUT_DIV_LO(port, (quot & 0xff));
	UART_PUT_DIV_HI(port, ((quot & 0xf00) >> 8));

	UART_PUT_LCR(port, uart_mc);
	UART_PUT_IER(port, old_ier);

	//restore_flags(flags);
	spin_unlock_irqrestore(&port->lock, flags);
}



static int sl2312_startup(struct uart_port *port)
{
	int retval;
	unsigned int regs;

//        printk("sl2312 startup : \n");

	/*
	 * Use iobase to store a pointer to info. We need this to start a
	 * transmission as the tranmittr interrupt is only generated on
	 * the transition to the idle state
	 */

	/*
	 * Allocate the IRQ
	 */
	retval = request_irq(port->irq, sl2312_int, IRQF_DISABLED, "sl2312", port);
	if (retval)
		return retval;

        /* setup interrupt controller  */
        regs = *((volatile unsigned int *)IRQ_TMODE(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
        regs &= ~(IRQ_UART_MASK);
        *((volatile unsigned int *)IRQ_TMODE(IO_ADDRESS(SL2312_INTERRUPT_BASE))) = regs;
        regs = *((volatile unsigned int *)IRQ_TLEVEL(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
        regs &= ~(IRQ_UART_MASK);
        *((volatile unsigned int *)IRQ_TLEVEL(IO_ADDRESS(SL2312_INTERRUPT_BASE))) = regs;
        *((volatile unsigned int *)IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE))) |= (unsigned int)(IRQ_UART_MASK);

	/*
	 * Finally, enable interrupts. Use the TII interrupt to minimise
	 * the number of interrupts generated. If higher performance is
	 * needed, consider using the TI interrupt with a suitable FIFO
	 * threshold
	 */
	UART_PUT_IER(port, (UART_IER_DR|UART_IER_TE));

	return 0;
}

static void sl2312_shutdown(struct uart_port *port)
{
//        printk("sl2312 shutdown : \n");

	/*
	 * disable all interrupts, disable the port
	 */
	UART_PUT_IER(port, 0x0);

	/* disable break condition and fifos */
//	UART_PUT_MCR(port, (UART_GET_MCR(port)&UART_MCR_MASK));

	/*
	 * Free the interrupt
	 */
	free_irq(port->irq, port);
}

static const char *sl2312_type(struct uart_port *port)
{
	return port->type == PORT_SL2312 ? "SL2312" : NULL;
}

/*
 * Release the memory region(s) being used by 'port'
 */
static void sl2312_release_port(struct uart_port *port)
{
//        printk("sl2312 release port : \n");

	release_mem_region(port->mapbase, UART_PORT_SIZE);
}

/*
 * Request the memory region(s) being used by 'port'
 */
static int sl2312_request_port(struct uart_port *port)
{
	return request_mem_region(port->mapbase, UART_PORT_SIZE,
				    "serial_sl2312") != NULL ? 0 : -EBUSY;
}

/*
 * Configure/autoconfigure the port.
 */
static void sl2312_config_port(struct uart_port *port, int flags)
{

	if (flags & UART_CONFIG_TYPE) {
		if (sl2312_request_port(port) == 0)
			port->type = PORT_SL2312;
	}
}

/*
 * verify the new serial_struct (for TIOCSSERIAL).
 */
static int sl2312_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	int ret = 0;

	if (ser->type != PORT_UNKNOWN && ser->type != PORT_UART00)
		ret = -EINVAL;
	if (ser->irq < 0 || ser->irq >= NR_IRQS)
		ret = -EINVAL;
	if (ser->baud_base < 9600)
		ret = -EINVAL;
	return ret;
}

static struct uart_ops sl2312_pops = {
	.tx_empty		=sl2312_tx_empty,
	.set_mctrl		=sl2312_set_mctrl_null,
	.get_mctrl		=sl2312_get_mctrl,
	.stop_tx		=sl2312_stop_tx,
	.start_tx		=sl2312_start_tx,
	.stop_rx		=sl2312_stop_rx,
	.enable_ms		=sl2312_enable_ms,
	.break_ctl		=sl2312_break_ctl,
	.startup		=sl2312_startup,
	.shutdown		=sl2312_shutdown,
	.set_termios	=sl2312_set_termios,
	.type			=sl2312_type,
	.release_port	=sl2312_release_port,
	.request_port	=sl2312_request_port,
	.config_port	=sl2312_config_port,
	.verify_port	=sl2312_verify_port,
};

#ifdef CONFIG_ARCH_SL2312

static struct uart_port sl2312_ports[UART_NR] = {
	{
		membase:	(void *)IO_ADDRESS(SL2312_UART_BASE),
		mapbase:	SL2312_UART_BASE,
		iotype:		SERIAL_IO_MEM,
		irq:		IRQ_UART,
		uartclk:	UART_CLK,
		fifosize:	16,
		ops:		&sl2312_pops,
		flags:		ASYNC_BOOT_AUTOCONF,
	}
};

#endif

#ifdef CONFIG_SERIAL_SL2312_CONSOLE
#ifdef used_and_not_const_char_pointer
static int sl2312_console_read(struct uart_port *port, char *s, u_int count)
{
	unsigned int status;
	int c;
#ifdef DEBUG
	printk("sl2312_console_read() called\n");
#endif

	c = 0;
	while (c < count) {
		status = UART_GET_LSR(port);
 		if (UART_RX_DATA(status)) {
			*s++ = UART_GET_CHAR(port);
			c++;
		} else {
			// nothing more to get, return
			return c;
		}
	}
	// return the count
	return c;
}
#endif
static void sl2312_console_write(struct console *co, const char *s, unsigned count)
{
#ifdef CONFIG_ARCH_SL2312
	struct uart_port *port = sl2312_ports + co->index;
	unsigned int status, old_ies;
	int i;

	/*
	 *	First save the CR then disable the interrupts
	 */
	old_ies = UART_GET_IER(port);
	UART_PUT_IER(port,0x0);

	/*
	 *	Now, do each character
	 */
	for (i = 0; i < count; i++) {
		do {
			status = UART_GET_LSR(port);
		} while (!UART_TX_READY(status));
		UART_PUT_CHAR(port, s[i]);
		if (s[i] == '\n') {
			do {
				status = UART_GET_LSR(port);
			} while (!UART_TX_READY(status));
			UART_PUT_CHAR(port, '\r');
		}
	}

	/*
	 *	Finally, wait for transmitter to become empty
	 *	and restore the IES
	 */
	do {
		status = UART_GET_LSR(port);
	} while (!(status&UART_LSR_TE));
	UART_PUT_IER(port, old_ies);
#endif
}

#if 0
static void sl2312_console_device(struct console *co,int *index)
{

	struct uart_driver *p = co->data;
    *index = co->index;
    return p->tty_driver;

}
#endif

static void /*__init*/ sl2312_console_get_options(struct uart_port *port, int *baud, int *parity, int *bits)
{
//	printk("sl2312 console get options : \n");

	u_int uart_mc, quot;
	uart_mc= UART_GET_MCR(port);

	*parity = 'n';
	if (uart_mc & UART_LCR_PE) {
		if (uart_mc & UART_LCR_EVEN)
			*parity = 'e';
		else
			*parity = 'o';
	}

	switch (uart_mc & UART_LCR_MSK){

	case UART_LCR_LEN5:
		*bits = 5;
		break;
	case UART_LCR_LEN6:
		*bits = 6;
		break;
	case UART_LCR_LEN7:
		*bits = 7;
		break;
	case UART_LCR_LEN8:
		*bits = 8;
		break;
	}
	UART_PUT_MCR(port,UART_LCR_DLAB);
	quot = UART_GET_DIV_LO(port) | (UART_GET_DIV_HI(port) << 8);
	UART_PUT_MCR(port,uart_mc);
	*baud = port->uartclk / (16 *quot );
}

static int __init sl2312_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = 19200;
	int bits = 8;
	int parity = 'n';
	int flow= 'n';

	printk("sl2312 console setup : \n");

#ifdef CONFIG_ARCH_SL2312
	/*
	 * Check whether an invalid uart number has been specified, and
	 * if so, search for the first available port that does have
	 * console support.
	 */
	port = uart_get_console(sl2312_ports,SL2312_NO_PORTS,co);
#else
	return -ENODEV;
#endif

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	else
		sl2312_console_get_options(port, &baud, &parity, &bits);

	return uart_set_options(port, co, baud, parity, bits, flow);
}

extern struct uart_driver sl2312_reg;
static struct console sl2312_console = {
	.name      = SERIAL_SL2312_NAME,
	.write		= sl2312_console_write,
	.device		= uart_console_device,
//	.device		= sl2312_console_device,
	.setup		= sl2312_console_setup,
//	.flags		= (CON_PRINTBUFFER|CON_ENABLED),
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data       = &sl2312_reg,
};

static int __init sl2312_console_init(void)
{
	register_console(&sl2312_console);
	return 0;

}

console_initcall(sl2312_console_init);

#define SL2312_CONSOLE	&sl2312_console
#else
#define SL2312_CONSOLE	NULL
#endif

// static
struct uart_driver sl2312_reg = {
	.owner         = NULL,
	.driver_name	= SERIAL_SL2312_NAME,
	.dev_name		= SERIAL_SL2312_NAME,
	.devfs_name		= "tts/",
	.major          = SERIAL_SL2312_MAJOR,
	.minor			= SERIAL_SL2312_MINOR,
	.nr				= UART_NR,
	.cons			= SL2312_CONSOLE,
};

static int __init sl2312_init(void)
{
       int result;
	//printk("serial_it8712: it871212_init \n");

        result = uart_register_driver(&sl2312_reg);
        if(result)
             return result;
	result = uart_add_one_port(&sl2312_reg, &sl2312_ports[0]);

        return result;
}


__initcall(sl2312_init);
