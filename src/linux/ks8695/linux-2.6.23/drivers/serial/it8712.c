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
 *  $Id: it8712.c,v 1.2 2006/06/06 06:36:04 middle Exp $
 *
 */
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/serial.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <asm/hardware.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <asm/sizes.h>

#if defined(CONFIG_SERIAL_IT8712_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/serial_core.h>
#include <asm/arch/sl2312.h>
#include <asm/arch/int_ctrl.h>
#include <asm/arch/it8712.h>
#include "it8712.h"

//#define DEBUG           1
#define UART_NR		1

#define SERIAL_IT8712_NAME	"ttySI"
#define SERIAL_IT8712_MAJOR	204
#define SERIAL_IT8712_MINOR	41      /* Temporary - will change in future */
#define SERIAL_IT8712_NR	UART_NR
#define UART_PORT_SIZE 0x50
#define LPC_HOST_CONTINUE_MODE	0x00000040

#define IT8712_NO_PORTS         UART_NR
#define IT8712_ISR_PASS_LIMIT	256

#define LPC_BUS_CTRL	*(unsigned int*)(IO_ADDRESS(SL2312_LPC_HOST_BASE + 4))
#define LPC_BUS_STATUS	*(unsigned int*)(IO_ADDRESS(SL2312_LPC_HOST_BASE + 4))
#define LPC_SERIAL_IRQ_CTRL	*(unsigned int*)(IO_ADDRESS(SL2312_LPC_HOST_BASE + 8))
#define LPC_SERIAL_IRQ_STATUS	*(unsigned int*)(IO_ADDRESS(SL2312_LPC_HOST_BASE + 0x0c))
#define LPC_SERIAL_IRQ_TRITYPE *(unsigned int*)(IO_ADDRESS(SL2312_LPC_HOST_BASE + 0x10))
#define LPC_SERIAL_IRQ_POLARITY	*(unsigned int*)(IO_ADDRESS(SL2312_LPC_HOST_BASE + 0x14))
#define LPC_SERIAL_IRQ_ENABLE	*(unsigned int*)(IO_ADDRESS(SL2312_LPC_HOST_BASE + 0x18))




/*
 * Access macros for the SL2312 UARTs
 */
#define UART_GET_INT_STATUS(p)	(inb(((p)->membase+UART_IIR)) & 0x0F)  // interrupt identification
#define UART_PUT_IER(p, c)      outb(c,((p)->membase+UART_IER))         // interrupt enable
#define UART_GET_IER(p)         inb(((p)->membase+UART_IER))
#define UART_PUT_CHAR(p, c)     outb(c,((p)->membase+UART_TX))         // transmitter holding
#define UART_GET_CHAR(p)        inb(((p)->membase+UART_RX))            // receive buffer
#define UART_GET_LSR(p)         inb(((p)->membase+UART_LSR))            // line status
#define UART_GET_MSR(p)         inb(((p)->membase+UART_MSR))            // modem status
#define UART_GET_MCR(p)         inb(((p)->membase+UART_MCR))            // modem control
#define UART_PUT_MCR(p, c)      outb(c,((p)->membase+UART_MCR))
#define UART_GET_LCR(p)         inb(((p)->membase+UART_LCR))       // mode control
#define UART_PUT_LCR(p, c)      outb(c,((p)->membase+UART_LCR))
#define UART_PUT_FCR(p, c)      outb(c,((p)->membase+UART_FCR))       // fifo control
#define UART_GET_DIV_HI(p)	inb(((p)->membase+UART_DLM))
#define UART_PUT_DIV_HI(p, c)	outb(c,((p)->membase+UART_DLM))
#define UART_GET_DIV_LO(p)	inb(((p)->membase+UART_DLL))
#define UART_PUT_DIV_LO(p, c)	outb(c,((p)->membase+UART_DLL))
#define UART_PUT_MDR(p, c)      outb(c,UART_MDR((p)->membase))
#define UART_RX_DATA(s)		((s) & UART_LSR_DR)
#define UART_TX_READY(s)	((s) & UART_LSR_THRE)

static void it8712_stop_tx(struct uart_port *port, u_int from_tty)
{
        unsigned int reg;

        //printk("it8712 stop tx : \n");
        reg = UART_GET_IER(port);
        reg &= ~(UART_IER_THRI);
	UART_PUT_IER(port, reg);
}

static void it8712_stop_rx(struct uart_port *port)
{
        unsigned int reg;

        //printk("it8712 stop rx : \n");
        reg = UART_GET_IER(port);
        reg &= ~(UART_IER_RDI);
	UART_PUT_IER(port, reg);

}

static void it8712_enable_ms(struct uart_port *port)
{
        unsigned int reg;

        //printk("it8712 enable ms : \n");

        reg = UART_GET_IER(port);
        reg |= (UART_IER_MSI);
	UART_PUT_IER(port, reg);

}

static void it8712_rx_chars(struct uart_port *port, struct pt_regs *regs)
{
	struct tty_struct *tty = port->info->tty;
	unsigned int status, mask, ch, flg, ignored = 0;

 //       printk("it8712_rx_chars : \n");
	status = UART_GET_LSR(port);
	while (UART_RX_DATA(status)) {

		/*
		 * We need to read rds before reading the
		 * character from the fifo
		 */
		ch = UART_GET_CHAR(port);
		port->icount.rx++;

		if (tty->flip.count >= TTY_FLIPBUF_SIZE)
			goto ignore_char;

		flg = TTY_NORMAL;

		/*
		 * Note that the error handling code is
		 * out of the main execution path
		 */

		if (status & (UART_LSR_OE|UART_LSR_PE|UART_LSR_FE|UART_LSR_BI|UART_LSR_DE))
			goto handle_error;
		if (uart_handle_sysrq_char(port, ch, regs))
			goto ignore_char;

	error_return:
		*tty->flip.flag_buf_ptr++ = flg;
		*tty->flip.char_buf_ptr++ = ch;
		tty->flip.count++;
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
		*tty->flip.flag_buf_ptr++ = flg;
		*tty->flip.char_buf_ptr++ = ch;
		tty->flip.count++;
		if (tty->flip.count >= TTY_FLIPBUF_SIZE)
			goto ignore_char;
		ch = 0;
		flg = TTY_OVERRUN;
	}
#ifdef SUPPORT_SYSRQ
	port->sysrq = 0;
#endif
	goto error_return;
}

static void it8712_tx_chars(struct uart_port *port)
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
		it8712_stop_tx(port, 0);
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
		it8712_stop_tx(port, 0);
}

static void it8712_start_tx(struct uart_port *port, unsigned int tty_start)
{
        unsigned int reg;

        //printk("it8712 start tx : \n");
        reg = UART_GET_IER(port);
        reg |= (UART_IER_THRI);
	UART_PUT_IER(port, reg);
	it8712_tx_chars(port);
}

static void it8712_modem_status(struct uart_port *port)
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

static irqreturn_t  it8712_int(int irq, void *dev_id, struct pt_regs *regs)
{
	struct uart_port *port = dev_id;
	unsigned int status, pass_counter = 0, data;


		data = LPC_SERIAL_IRQ_STATUS;
	if((data&0x10)==0x10)
	{
		status = UART_GET_INT_STATUS(port);
		do {
//			     printk("it8712_int: status %x \n", status);
			switch(status)
			{
			   case UART_IIR_RDI:
			   case UART_IIR_RLSI:
			   case UART_IIR_RCTO:
				it8712_rx_chars(port, regs);
			   break;
			   case UART_IIR_THRI:
				it8712_tx_chars(port);
			   break;
			   case UART_IIR_MSI:
				it8712_modem_status(port);
			   break;
			   default:
			   break;
			}
			if (pass_counter++ > IT8712_ISR_PASS_LIMIT)
				break;

			status = UART_GET_INT_STATUS(port);
		} while (status);
	}

		status = 0;
        status |= (IRQ_LPC_MASK);
        *((volatile unsigned int *)IRQ_CLEAR(IO_ADDRESS(SL2312_INTERRUPT_BASE))) = status;

	//cnt=0;
	//do{
	//	data = LPC_SERIAL_IRQ_STATUS;
		LPC_SERIAL_IRQ_STATUS = data;
	//	cnt++;
	//}while(data);
	//if(cnt>2)
	//	printf("it8712_uart_Isr clear LPC_SERIAL_IRQ_STATUS %x \n", cnt);
        return IRQ_HANDLED;
}

static u_int it8712_tx_empty(struct uart_port *port)
{
//        printk("it8712 tx empty : \n");

	return ((UART_GET_LSR(port) & UART_LSR_THRE)? TIOCSER_TEMT : 0);
}

static u_int it8712_get_mctrl(struct uart_port *port)
{
	unsigned int result = 0;
	unsigned int status;

//        printk("it8712 get mctrl : \n");

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

static void it8712_set_mctrl_null(struct uart_port *port, u_int mctrl)
{
}

static void it8712_break_ctl(struct uart_port *port, int break_state)
{
	unsigned int lcr;

//        printk("it8712 break ctl : \n");

	lcr = UART_GET_LCR(port);
	if (break_state == -1)
		lcr |= UART_LCR_SBC;
	else
		lcr &= ~UART_LCR_SBC;
	UART_PUT_LCR(port, lcr);
}

static inline u_int uart_calculate_quot(struct uart_port *port, u_int baud)
{
	u_int quot;

	/* Special case: B0 rate */
	if (!baud)
		baud = 9600;

	quot = (port->uartclk/(16 * baud)) ;

	return quot;
}
static void it8712_set_termios(struct uart_port *port, struct termios *termios,
                               struct termios *old)
{
	unsigned int  uart_mc, old_ier, baud, quot;
	unsigned long flags;

        termios->c_cflag |= CREAD;
        termios->c_cflag |= CLOCAL;
#ifdef DEBUG
	printk("it8712_set_cflag(0x%x) called\n", cflag);
#endif
        baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk/16);
        quot = uart_get_divisor(port, baud);

	/* byte size and parity */
	switch (termios->c_cflag & CSIZE) {
	case CS5:
              uart_mc = UART_LCR_WLEN5;
              break;
	case CS6:
              uart_mc = UART_LCR_WLEN6;
              break;
	case CS7:
              uart_mc = UART_LCR_WLEN7;
              break;
	default: // CS8
              uart_mc = UART_LCR_WLEN8;
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

	old_ier = UART_GET_IER(port);

        if(UART_ENABLE_MS(port, termios->c_cflag))
             old_ier |= UART_IER_MSI;

	/* Set baud rate */
	quot = quot / 13;
	UART_PUT_LCR(port, UART_LCR_DLAB);
	UART_PUT_DIV_LO(port, (quot & 0xff));
	UART_PUT_DIV_HI(port, ((quot & 0xf00) >> 8));

	UART_PUT_LCR(port, uart_mc);
//	UART_PUT_LCR(port, 0x07); // ???? it is wired
        UART_PUT_MCR(port, 0x08);
        UART_PUT_FCR(port, 0x01);
	UART_PUT_IER(port, 0x07);

	spin_unlock_irqrestore(&port->lock, flags);
}

static int it8712_startup(struct uart_port *port)
{
	int retval, i;
	unsigned int regs;

        //printk("it8712 startup : \n");

	/*
	 * Use iobase to store a pointer to info. We need this to start a
	 * transmission as the tranmittr interrupt is only generated on
	 * the transition to the idle state
	 */

	//	regs = 0;
    //    regs |= (IRQ_LPC_MASK);
    //    *((volatile unsigned int *)IRQ_CLEAR(IO_ADDRESS(SL2312_INTERRUPT_BASE))) = regs;

	/*
	 * Allocate the IRQ
	 */
	retval = request_irq(port->irq, it8712_int, SA_INTERRUPT, "it8712", port);
	if (retval)
		return retval;

	//printk("Init LPC int...........\n");
        /* setup interrupt controller  */
        regs = *((volatile unsigned int *)IRQ_TMODE(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
        regs &= ~(IRQ_LPC_MASK);
        *((volatile unsigned int *)IRQ_TMODE(IO_ADDRESS(SL2312_INTERRUPT_BASE))) = regs;
        regs = *((volatile unsigned int *)IRQ_TLEVEL(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
        regs &= ~(IRQ_LPC_MASK);
        *((volatile unsigned int *)IRQ_TLEVEL(IO_ADDRESS(SL2312_INTERRUPT_BASE))) = regs;
        *((volatile unsigned int *)IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE))) |= (unsigned int)(IRQ_LPC_MASK);

	LPC_SERIAL_IRQ_POLARITY = 0x10; //0x10; //0x02;
	LPC_SERIAL_IRQ_TRITYPE = 0x10; //0x10;//
	LPC_SERIAL_IRQ_ENABLE = 0x10;

	LPC_BUS_CTRL = 0xc0;
	LPC_SERIAL_IRQ_CTRL = 0xc0;
	for(i=0;i<1000;i++) ;
	LPC_SERIAL_IRQ_CTRL = 0x80;
	/*
	 * Finally, enable interrupts. Use the TII interrupt to minimise
	 * the number of interrupts generated. If higher performance is
	 * needed, consider using the TI interrupt with a suitable FIFO
	 * threshold
	 */
	//UART_PUT_IER(port, (UART_IER_RDI|UART_IER_THRI));
	UART_PUT_IER(port, (UART_IER_RDI|UART_IER_THRI|UART_IER_RLSI));//middle

	return 0;
}

static void it8712_shutdown(struct uart_port *port)
{
        //printk("it8712 shutdown : \n");

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

static const char *it8712_type(struct uart_port *port)
{
	return port->type == PORT_IT8712 ? "IT8712" : NULL;
}

/*
 * Release the memory region(s) being used by 'port'
 */
static void it8712_release_port(struct uart_port *port)
{
//        printk("it8712 release port : \n");

	release_mem_region(port->mapbase, UART_PORT_SIZE);
}

/*
 * Request the memory region(s) being used by 'port'
 */
static int it8712_request_port(struct uart_port *port)
{
	return request_mem_region(port->mapbase, UART_PORT_SIZE,
				    "serial_it8712") != NULL ? 0 : -EBUSY;
}

/*
 * Configure/autoconfigure the port.
 */
static void it8712_config_port(struct uart_port *port, int flags)
{

	if (flags & UART_CONFIG_TYPE) {
		if (it8712_request_port(port) == 0)
			port->type = PORT_IT8712;
	}
}

/*
 * verify the new serial_struct (for TIOCSSERIAL).
 */
static int it8712_verify_port(struct uart_port *port, struct serial_struct *ser)
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

static struct uart_ops it8712_pops = {
	.tx_empty	= it8712_tx_empty,
	.set_mctrl	= it8712_set_mctrl_null,
	.get_mctrl	= it8712_get_mctrl,
	.stop_tx	= it8712_stop_tx,
	.start_tx	= it8712_start_tx,
	.stop_rx	= it8712_stop_rx,
	.enable_ms	= it8712_enable_ms,
	.break_ctl	= it8712_break_ctl,
	.startup	= it8712_startup,
	.shutdown	= it8712_shutdown,
	.set_termios	= it8712_set_termios,
	.type		= it8712_type,
	.release_port	= it8712_release_port,
	.request_port	= it8712_request_port,
	.config_port	= it8712_config_port,
	.verify_port	= it8712_verify_port,
};

#ifdef CONFIG_ARCH_SL2312

static struct uart_port it8712_ports[UART_NR] = {
	{
		membase:	(void *)0,
		mapbase:	0,
		iotype:		SERIAL_IO_MEM,
		irq:		0,
		uartclk:	UART_CLK/2,
		fifosize:	16,
		ops:		&it8712_pops,
		flags:		ASYNC_BOOT_AUTOCONF,
	}
};

#endif

#ifdef CONFIG_SERIAL_IT8712_CONSOLE
#ifdef used_and_not_const_char_pointer
static int it8712_console_read(struct uart_port *port, char *s, u_int count)
{
	unsigned int status;
	int c;
#ifdef DEBUG
	printk("it8712_console_read() called\n");
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
static void it8712_console_write(struct console *co, const char *s, unsigned count)
{
#ifdef CONFIG_ARCH_SL2312
	struct uart_port *port = it8712_ports + co->index;
	unsigned int status, old_ies;
	int i;

	/*
	 *	First save the CR then disable the interrupts
	 */
	old_ies = UART_GET_IER(port);
	//if(old_ies!=7)
	//{
	//
	//	printk("old_ies = %x\n",old_ies);
	//	old_ies = 7;
	//}
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
	} while (!(status&UART_LSR_THRE));
	UART_PUT_IER(port, old_ies);
#endif
}

static void /*__init*/ it8712_console_get_options(struct uart_port *port, int *baud, int *parity, int *bits)
{
	//printk("it8712 console get options : \n");

	u_int uart_mc, quot;
	uart_mc= UART_GET_MCR(port);

	*parity = 'n';
	if (uart_mc & UART_LCR_PARITY) {
		if (uart_mc & UART_LCR_EVEN)
			*parity = 'e';
		else
			*parity = 'o';
	}

	switch (uart_mc & UART_LCR_MSK){

	case UART_LCR_WLEN5:
		*bits = 5;
		break;
	case UART_LCR_WLEN6:
		*bits = 6;
		break;
	case UART_LCR_WLEN7:
		*bits = 7;
		break;
	case UART_LCR_WLEN8:
		*bits = 8;
		break;
	}
	UART_PUT_MCR(port,UART_LCR_DLAB);
	quot = UART_GET_DIV_LO(port) | (UART_GET_DIV_HI(port) << 8);
	UART_PUT_MCR(port,uart_mc);
	*baud = (port->uartclk / (16 *quot));
}

static int __init it8712_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = 38400;
	int bits = 8;
	int parity = 'n';
	int flow= 'n';
	int base;//, irq;
	int i ;

	printk("it8712 console setup : \n");

	LPCSetConfig(0, 0x02, 0x01);
        LPCSetConfig(LDN_SERIAL1, 0x30, 0x1);
        LPCSetConfig(LDN_SERIAL1, 0x23, 0x0);
	base = IT8712_IO_BASE;
	base += ((LPCGetConfig(LDN_SERIAL1, 0x60) << 8) + LPCGetConfig(LDN_SERIAL1, 0x61));
	it8712_ports[0].mapbase = base;
	it8712_ports[0].membase = (void *)IO_ADDRESS(base);
	it8712_ports[0].irq = IRQ_LPC_OFFSET;
    //   	irq = LPCGetConfig(LDN_SERIAL1, 0x70);
	//it8712_ports[0].irq += irq;

	//printk("it8712 irq is %x \n", it8712_ports[0].irq);

	// setup LPC Host 'quiet mode'
	//*((volatile unsigned int *)IO_ADDRESS((SL2312_LPC_HOST_BASE+0x04))) |= LPC_HOST_CONTINUE_MODE ;
	//for(i=0;i<1000;i++) ;						// delay
	//*((volatile unsigned int *)IO_ADDRESS((SL2312_LPC_HOST_BASE+0x04))) &= ~(LPC_HOST_CONTINUE_MODE) ;
	LPC_BUS_CTRL = 0xc0;
	LPC_SERIAL_IRQ_CTRL = 0xc0;
	for(i=0;i<1000;i++) ;
	LPC_SERIAL_IRQ_CTRL = 0x80;

#ifdef CONFIG_ARCH_SL2312
	/*
	 * Check whether an invalid uart number has been specified, and
	 * if so, search for the first available port that does have
	 * console support.
	 */
	port = uart_get_console(it8712_ports,IT8712_NO_PORTS,co);
#else
	return -ENODEV;
#endif

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	else
		it8712_console_get_options(port, &baud, &parity, &bits);

	return uart_set_options(port, co, baud, parity, bits, flow);
}

extern struct uart_driver it8712_reg;
static struct console it8712_console = {
	.name           = SERIAL_IT8712_NAME,
	.write		= it8712_console_write,
	.device		= uart_console_device,
        .setup          = it8712_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= 0,
        .data           = &it8712_reg,
};

static int __init it8712_console_init(void)
{
	register_console(&it8712_console);
        return 0;
}

console_initcall(it8712_console_init);

#define IT8712_CONSOLE	&it8712_console
#else
#define IT8712_CONSOLE	NULL
#endif

static struct uart_driver it8712_reg = {
	.owner                  = NULL,
	.driver_name		= SERIAL_IT8712_NAME,
	.dev_name		= SERIAL_IT8712_NAME,
        .major                  = SERIAL_IT8712_MAJOR,
	.minor			= SERIAL_IT8712_MINOR,
	.nr			= UART_NR,
	.cons			= IT8712_CONSOLE,
};

static int __init it8712_init(void)
{
        int result;
	//printk("serial_it8712: it871212_init \n");


        result = uart_register_driver(&it8712_reg);
        if(result)
             return result;
	result = uart_add_one_port(&it8712_reg, &it8712_ports[0]);

        return result;

}


__initcall(it8712_init);
