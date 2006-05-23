/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *   UART code.
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
 * Copyright (C) 2001 MontaVista Software Inc.
 * Author: jsun@mvista.com or jsun@junsun.net
 * based on something similar to arch/mips/vr4181/osprey/dbg_io.c
 *
 * May 2004 rkt
 *
 * Ported to IDT EB434 eval board
 *
 * 
 *
 **************************************************************************
 */

#include <linux/config.h>
#define RC32434_REG_BASE   0xb8000000
#ifdef __MIPSEB__
#define RC32434_UART0_BASE (RC32434_REG_BASE + 0x58003)
#else
#define RC32434_UART0_BASE (RC32434_REG_BASE + 0x58000)
#endif

#define BASE		        RC32434_UART0_BASE
#define MAX_BAUD		(CONFIG_IDT_BOARD_FREQ / 16)
#define REG_OFFSET		0x4


#if (!defined(BASE) || !defined(MAX_BAUD) || !defined(REG_OFFSET))
#error You must define BASE, MAX_BAUD and REG_OFFSET in the Makefile.
#endif

#ifndef INIT_SERIAL_PORT
#define INIT_SERIAL_PORT	1
#endif

#ifndef DEFAULT_BAUD
//#define DEFAULT_BAUD		UART16550_BAUD_115200
#define DEFAULT_BAUD		UART16550_BAUD_9600
#endif
#ifndef DEFAULT_PARITY
#define DEFAULT_PARITY		UART16550_PARITY_NONE
#endif
#ifndef DEFAULT_DATA
#define DEFAULT_DATA		UART16550_DATA_8BIT
#endif
#ifndef DEFAULT_STOP
#define DEFAULT_STOP		UART16550_STOP_1BIT
#endif

/* === END OF CONFIG === */

typedef         unsigned char uint8;
typedef         unsigned int  uint32;

#define         UART16550_BAUD_2400             2400
#define         UART16550_BAUD_4800             4800
#define         UART16550_BAUD_9600             9600
#define         UART16550_BAUD_19200            19200
#define         UART16550_BAUD_38400            38400
#define         UART16550_BAUD_57600            57600
#define         UART16550_BAUD_115200           115200

#define         UART16550_PARITY_NONE           0
#define         UART16550_PARITY_ODD            0x08
#define         UART16550_PARITY_EVEN           0x18
#define         UART16550_PARITY_MARK           0x28
#define         UART16550_PARITY_SPACE          0x38

#define         UART16550_DATA_5BIT             0x0
#define         UART16550_DATA_6BIT             0x1
#define         UART16550_DATA_7BIT             0x2
#define         UART16550_DATA_8BIT             0x3

#define         UART16550_STOP_1BIT             0x0
#define         UART16550_STOP_2BIT             0x4

/* register offset */
#define		OFS_RCV_BUFFER		(0*REG_OFFSET)
#define		OFS_TRANS_HOLD		(0*REG_OFFSET)
#define		OFS_SEND_BUFFER		(0*REG_OFFSET)
#define		OFS_INTR_ENABLE		(1*REG_OFFSET)
#define		OFS_INTR_ID		(2*REG_OFFSET)
#define		OFS_DATA_FORMAT		(3*REG_OFFSET)
#define		OFS_LINE_CONTROL	(3*REG_OFFSET)
#define		OFS_MODEM_CONTROL	(4*REG_OFFSET)
#define		OFS_RS232_OUTPUT	(4*REG_OFFSET)
#define		OFS_LINE_STATUS		(5*REG_OFFSET)
#define		OFS_MODEM_STATUS	(6*REG_OFFSET)
#define		OFS_RS232_INPUT		(6*REG_OFFSET)
#define		OFS_SCRATCH_PAD		(7*REG_OFFSET)

#define		OFS_DIVISOR_LSB		(0*REG_OFFSET)
#define		OFS_DIVISOR_MSB		(1*REG_OFFSET)

#define		UART16550_READ(y)    (*((volatile uint8*)(BASE + y)))
#define		UART16550_WRITE(y, z)  ((*((volatile uint8*)(BASE + y))) = z)

static void Uart16550Init(uint32 baud, uint8 data, uint8 parity, uint8 stop)
{
	/* disable interrupts */
	UART16550_WRITE(OFS_LINE_CONTROL, 0x0);
	UART16550_WRITE(OFS_INTR_ENABLE, 0);
	
	/* set up baud rate */
	{
		uint32 divisor;
		
		/* set DIAB bit */
		UART16550_WRITE(OFS_LINE_CONTROL, 0x80);
		
		/* set divisor */
		divisor = MAX_BAUD / baud;
		UART16550_WRITE(OFS_DIVISOR_LSB, divisor & 0xff);
		UART16550_WRITE(OFS_DIVISOR_MSB, (divisor & 0xff00)>>8);
		
		/* clear DIAB bit */
		UART16550_WRITE(OFS_LINE_CONTROL, 0x0);
	}
	
	/* set data format */
	UART16550_WRITE(OFS_DATA_FORMAT, data | parity | stop);
}


void
putc_init(void)
{
#if INIT_SERIAL_PORT
	Uart16550Init(DEFAULT_BAUD, DEFAULT_DATA, DEFAULT_PARITY, DEFAULT_STOP);
#endif
}

void
putc(unsigned char c)
{
	while ((UART16550_READ(OFS_LINE_STATUS) &0x20) == 0);
	UART16550_WRITE(OFS_SEND_BUFFER, c);
}
