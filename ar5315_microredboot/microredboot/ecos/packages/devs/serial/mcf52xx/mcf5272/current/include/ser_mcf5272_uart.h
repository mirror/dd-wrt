#ifndef _SER_MCF5272_H_
#define _SER_MCF5272_H_
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================

#include <pkgconf/io_serial_mcf5272_uart.h>

/* Bit level definitions and macros */
#define MCF5272_UART_UMR1_RXRTS			(0x80)
#define MCF5272_UART_UMR1_RXIRQ			(0x40)
#define MCF5272_UART_UMR1_ERR			(0x20)
#define MCF5272_UART_UMR1_PM_MULTI_ADDR	(0x1C)
#define MCF5272_UART_UMR1_PM_MULTI_DATA	(0x18)
#define MCF5272_UART_UMR1_PM_NONE		(0x10)
#define MCF5272_UART_UMR1_PM_FORCE_HI	(0x0C)
#define MCF5272_UART_UMR1_PM_FORCE_LO	(0x08)
#define MCF5272_UART_UMR1_PM_ODD		(0x04)
#define MCF5272_UART_UMR1_PM_EVEN		(0x00)
#define MCF5272_UART_UMR1_BC_5			(0x00)
#define MCF5272_UART_UMR1_BC_6			(0x01)
#define MCF5272_UART_UMR1_BC_7			(0x02)
#define MCF5272_UART_UMR1_BC_8			(0x03)

#define MCF5272_UART_UMR2_CM_NORMAL	  	(0x00)
#define MCF5272_UART_UMR2_CM_ECHO	  	(0x40)
#define MCF5272_UART_UMR2_CM_LOCAL_LOOP	(0x80)
#define MCF5272_UART_UMR2_CM_REMOTE_LOOP	(0xC0)
#define MCF5272_UART_UMR2_TXRTS		 	(0x20)
#define MCF5272_UART_UMR2_TXCTS		 	(0x10)
#define MCF5272_UART_UMR2_STOP_BITS_1 	(0x07)
#define MCF5272_UART_UMR2_STOP_BITS_15	(0x08)
#define MCF5272_UART_UMR2_STOP_BITS_2 	(0x0F)
#define MCF5272_UART_UMR2_STOP_BITS(a)   ((a)&0x0f)	/* Stop Bit Length */

//#define MCF5272_UART_USR_RB				(0x80)
//#define MCF5272_UART_USR_FE				(0x40)
//#define MCF5272_UART_USR_PE				(0x20)
//#define MCF5272_UART_USR_OE				(0x10)
//#define MCF5272_UART_USR_TXEMP			(0x08)
//#define MCF5272_UART_USR_TXRDY			(0x04)
#define MCF5272_UART_USR_FFULL			(0x02)
#define MCF5272_UART_USR_RXRDY			(0x01)

#define MCF5272_UART_UCSR_RCS(a)	(((a)&0x0f)<<4)	/* Rx Clk Select */
#define MCF5272_UART_UCSR_TCS(a)		((a)&0x0f)	/* Tx Clk Select */


#define MCF5272_UART_UCR_NONE			(0x00)
#define MCF5272_UART_UCR_ENAB           (0x80)
#define MCF5272_UART_UCR_STOP_BREAK		(0x70)
#define MCF5272_UART_UCR_START_BREAK	(0x60)
#define MCF5272_UART_UCR_RESET_BKCHGINT	(0x50)
#define MCF5272_UART_UCR_RESET_ERROR	(0x40)
#define MCF5272_UART_UCR_RESET_TX		(0x30)
#define MCF5272_UART_UCR_RESET_RX		(0x20)
#define MCF5272_UART_UCR_RESET_MR		(0x10)
#define MCF5272_UART_UCR_TX_DISABLED	(0x08)
#define MCF5272_UART_UCR_TX_ENABLED		(0x04)
#define MCF5272_UART_UCR_RX_DISABLED	(0x02)
#define MCF5272_UART_UCR_RX_ENABLED		(0x01)

#define MCF5272_UART_UCCR_COS			(0x10)
#define MCF5272_UART_UCCR_CTS			(0x01)

#define MCF5272_UART_UACR_BRG			(0x80)
#define MCF5272_UART_UACR_CTMS_TIMER	(0x60)
#define MCF5272_UART_UACR_IEC			(0x01)

#define MCF5272_UART_UISR_COS			(0x80)
#define MCF5272_UART_UISR_ABC           (0x40)
#define MCF5272_UART_UISR_DB			(0x04)
#define MCF5272_UART_UISR_RXRDY			(0x02)
#define MCF5272_UART_UISR_TXRDY			(0x01)

#define MCF5272_UART_UIMR_COS			(0x80)
#define MCF5272_UART_UIMR_ABC           (0x40)
#define MCF5272_UART_UIMR_DB			(0x04)
#define MCF5272_UART_UIMR_FFULL			(0x02)
#define MCF5272_UART_UIMR_TXRDY			(0x01)

typedef unsigned char		uint8;  /*  8 bits */
typedef unsigned short int	uint16; /* 16 bits */
typedef unsigned long int	uint32; /* 32 bits */

typedef signed char			int8;   /*  8 bits */
typedef signed short int	int16;  /* 16 bits */
typedef signed long int		int32;  /* 32 bits */

#ifdef CYGPKG_IO_SERIAL_MCF5272_UART_CHANNEL0
unsigned long MCF5272_uart_get_channel_0_baud_rate(void);
#endif /* CYGPKG_IO_SERIAL_MCF5272_UART_CHANNEL0 */

#ifdef CYGPKG_IO_SERIAL_MCF5272_UART_CHANNEL1
unsigned long MCF5272_uart_get_channel_1_baud_rate(void);
#endif /* CYGPKG_IO_SERIAL_MCF5272_UART_CHANNEL1 */

#endif /* _SER_MCF5272_H_ */

