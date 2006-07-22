/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *      EB434 specific polling driver for 16550 UART.
 *
 *  Copyright 2004 IDT Inc. (rischelp@idt.com)
 *         
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * Copyright (C) 2000 by Lineo, Inc.
 * Written by Quinn Jensen (jensenq@lineo.com)
 **************************************************************************
 * P. Sadik  Oct 20, 2003
 *
 * DIVISOR is made a function of idt_cpu_freq
 **************************************************************************
 * P. Sadik  Oct 30, 2003
 *
 * added reset_cons_port
 **************************************************************************
 */

#include <linux/serial_reg.h>

/* turn this on to watch the debug protocol echoed on the console port */
#define DEBUG_REMOTE_DEBUG

#define CONS_BAUD 115200

extern unsigned int idt_cpu_freq;

#define EXT_FREQ    24000000
#define INT_FREQ    idt_cpu_freq

#define EXT_PORT    0xb9800000u
#define EXT_SHIFT   0

#ifdef __MIPSEB__
#define INT_PORT    0xb8058003u
#else
#define INT_PORT    0xb8058000u
#endif
#define INT_SHIFT   2

#define INT_FCR     UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_14
#define EXT_FCR     UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT

typedef struct
{
  volatile unsigned char *base;
  unsigned int shift;
  unsigned int freq;
  unsigned int fcr;
} ser_port;
           
ser_port ports[2] = 
{
  { (volatile unsigned char *)INT_PORT, INT_SHIFT, 0, INT_FCR},
  { (volatile unsigned char *)EXT_PORT, EXT_SHIFT, EXT_FREQ, EXT_FCR}
};

#define CONS_PORT   0

void cons_putc(char c);
int port_getc(int port);
void port_putc(int port, char c);

int cons_getc(void)
{
	return port_getc(CONS_PORT);
}

void cons_putc(char c)
{
	port_putc(CONS_PORT, c);
}

void cons_puts(char *s)
{
	while(*s) {
		if(*s == '\n') cons_putc('\r');
		cons_putc(*s);
		s++;
	}
}

void cons_do_putn(int n)
{
	if(n) {
		cons_do_putn(n / 10);
		cons_putc(n % 10 + '0');
	}
}

void cons_putn(int n)
{
	if(n < 0) {
		cons_putc('-');
		n = -n;
	}

	if (n == 0) {
		cons_putc('0');
	} else {
		cons_do_putn(n);
	}
}

int port_getc(int p)
{
	volatile unsigned char *port = ports[p].base;
	int s = ports[p].shift;
	int c;

	while((*(port + (UART_LSR << s)) & UART_LSR_DR) == 0) {
		continue;
	}       	

	c = *(port + (UART_RX << s));

	return c;
}

int port_getc_ready(int p)
{
	volatile unsigned char *port = ports[p].base;
	int s = ports[p].shift;

	return *(port + (UART_LSR << s)) & UART_LSR_DR;
}

#define OK_TO_XMT (UART_LSR_TEMT | UART_LSR_THRE)

void port_putc(int p, char c)
{
	volatile unsigned char *port = ports[p].base;
	int s = ports[p].shift;
	volatile unsigned char *lsr = port + (UART_LSR << s);

	while((*lsr & OK_TO_XMT) != OK_TO_XMT) {
		continue;
	}

	*(port + (UART_TX << s)) = c;
}

void reset_cons_port(void)
{
  volatile unsigned char *port = ports[CONS_PORT].base;
  unsigned int s = ports[CONS_PORT].shift;
  unsigned int DIVISOR;

  if (ports[CONS_PORT].freq) 
    DIVISOR = (ports[CONS_PORT].freq / 16 / CONS_BAUD);
  else
    DIVISOR = (idt_cpu_freq / 16 / CONS_BAUD);

  /* reset the port */
  *(port + (UART_CSR << s)) = 0;

  /* clear and enable the FIFOs */
  *(port + (UART_FCR << s)) = ports[CONS_PORT].fcr;

  /* set the baud rate */
  *(port + (UART_LCR << s)) = UART_LCR_DLAB;         /* enable DLL, DLM registers */

  *(port + (UART_DLL << s)) = DIVISOR;
  *(port + (UART_DLM << s)) = DIVISOR >> 8;
  /* set the line control stuff and disable DLL, DLM regs */

  *(port + (UART_LCR << s)) = UART_LCR_STOP |        /* 2 stop bits */
    UART_LCR_WLEN8;                         /* 8 bit word length */
        
  /* leave interrupts off */
  *(port + (UART_IER << s)) = 0;

  /* the modem controls don't leave the chip on this port, so leave them alone */
  *(port + (UART_MCR << s)) = 0;
}
