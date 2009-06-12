//=============================================================================
//
//      cycduart.h - Cyclone Diagnostics
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors:
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/


/* Control/status register offsets from base address */

#define RBR 0x00
#define THR 0x00
#define DLL 0x00
#define IER 0x01
#define DLM 0x01
#define IIR 0x02
#define FCR 0x02
#define LCR 0x03
#define MCR 0x04
#define LSR 0x05
#define MSR 0x06
#define SCR 0x07

/* 16550A Line Control Register */

#define LCR_5BITS 0x00
#define LCR_6BITS 0x01
#define LCR_7BITS 0x02
#define LCR_8BITS 0x03
#define LCR_NSB 0x04
#define LCR_PEN 0x08
#define LCR_EPS 0x10
#define LCR_SP 0x20
#define LCR_SB 0x40
#define LCR_DLAB 0x80

/* 16550A Line Status Register */

#define LSR_DR 0x01
#define LSR_OE 0x02
#define LSR_PE 0x04
#define LSR_FE 0x08
#define LSR_BI 0x10
#define LSR_THRE 0x20
#define LSR_TSRE 0x40
#define LSR_FERR 0x80

/* 16550A Interrupt Identification Register */

#define IIR_IP 0x01
#define IIR_ID 0x0e
#define IIR_RLS 0x06
#define IIR_RDA 0x04
#define IIR_THRE 0x02
#define IIR_MSTAT 0x00
#define IIR_TIMEOUT 0x0c

/* 16550A interrupt enable register bits */

#define IER_DAV 0x01
#define IER_TXE 0x02
#define IER_RLS 0x04
#define IER_MS 0x08

/* 16550A Modem control register */

#define MCR_DTR 0x01
#define MCR_RTS 0x02
#define MCR_OUT1 0x04
#define MCR_OUT2 0x08
#define MCR_LOOP 0x10

/* 16550A Modem Status Register */

#define MSR_DCTS 0x01
#define MSR_DDSR 0x02
#define MSR_TERI 0x04
#define MSR_DRLSD 0x08
#define MSR_CTS 0x10
#define MSR_DSR 0x20
#define MSR_RI 0x40
#define MSR_RLSD 0x80

/* (*) 16550A FIFO Control Register */

#define FCR_EN 0x01
#define FCR_RXCLR 0x02
#define FCR_TXCLR 0x04
#define FCR_DMA 0x08
#define FCR_RES1 0x10
#define FCR_RES2 0x20
#define FCR_RXTRIG_L 0x40
#define FCR_RXTRIG_H 0x80


#define CHAN1 0x8
#define CHAN2 0x0

#define DataIn		0x00		/* data input port  */
#define DataOut		0x00		/* data output port  */
#define BaudLsb		0x00		/* baud rate divisor least significant byte  */
#define BaudMsb		0x01		/* baud rate divisor most significant byte  */


/*
 * Enable receive and transmit FIFOs.
 *
 * FCR<7:6>     00      trigger level = 1 byte
 * FCR<5:4>     00      reserved
 * FCR<3>       0       mode 1 - interrupt on fifo threshold
 * FCR<2>       1       clear xmit fifo
 * FCR<1>       1       clear recv fifo
 * FCR<0>       1       turn on fifo mode
 */
#define FIFO_ENABLE 0x07
#define INT_ENABLE  (IER_RLS)   /* default interrupt mask */



