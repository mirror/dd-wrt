//==========================================================================
//
//      ks5000_regs.h
//
//      
//
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

/*------------------------------------------------------------------------
   name		: ks5000.h
   purpose	: This is the header file that will define all the
                  registers for the ks5000 processor.  It will contain
		  the addresses for the registers and some values for
		  those registers.
========================================================================*/
#ifndef _KS5000_REGS_H_
#define _KS5000_REGS_H_

#include "std.h"

#define BD_LAN_STOP     {}
#define DEBUG   0             /* DEBUG mode */

#define VPint(a) (*((volatile unsigned int*)(a)))
#define VPshort(a) (*((volatile unsigned short int*)(a)))
#define VPchar(a) (*((volatile unsigned char*)(a)))

#define Base_Addr       0x7ff0000

#define INTADDR         (Reset_Addr+0x20)
#define SPSTR           (VPint(Base_Addr))

// System Manager Register
#define SYSCFG          (VPint(Base_Addr+0x0000))
#define CLKCON          (VPint(Base_Addr+0x3000))
#define EXTACON0        (VPint(Base_Addr+0x3008))
#define EXTACON1        (VPint(Base_Addr+0x300c))
#define EXTDBWTH        (VPint(Base_Addr+0x3010))
#define ROMCON0         (VPint(Base_Addr+0x3014))
#define ROMCON1         (VPint(Base_Addr+0x3018))
#define ROMCON2         (VPint(Base_Addr+0x301c))
#define ROMCON3         (VPint(Base_Addr+0x3020))
#define ROMCON4         (VPint(Base_Addr+0x3024))
#define ROMCON5         (VPint(Base_Addr+0x3028))
#define DRAMCON0        (VPint(Base_Addr+0x302c))
#define DRAMCON1        (VPint(Base_Addr+0x3030))
#define DRAMCON2        (VPint(Base_Addr+0x3034))
#define DRAMCON3        (VPint(Base_Addr+0x3038))
#define REFEXTCON       (VPint(Base_Addr+0x303c))

// Ethernet BDMA Register
#define BDMATXCON       (VPint(Base_Addr+0x9000))
#define BDMARXCON       (VPint(Base_Addr+0x9004))
#define BDMATXPTR       (VPint(Base_Addr+0x9008))
#define BDMARXPTR       (VPint(Base_Addr+0x900c))
#define BDMARXLSZ       (VPint(Base_Addr+0x9010))
#define BDMASTAT        (VPint(Base_Addr+0x9014))

#define CAM_BASE        (VPint(Base_Addr+0x9100))
#define BDMATXBUF       (VPint(Base_Addr+0x9200))
#define BDMARXBUF       (VPint(Base_Addr+0x9800))
#define CAM_BaseAddr	(Base_Addr+0x9100)

// Ethernet MAC Register
#define MACCON          (VPint(Base_Addr+0xa000))
#define CAMCON          (VPint(Base_Addr+0xa004))
#define MACTXCON        (VPint(Base_Addr+0xa008))
#define MACTXSTAT       (VPint(Base_Addr+0xa00c))
#define MACRXCON        (VPint(Base_Addr+0xa010))
#define MACRXSTAT       (VPint(Base_Addr+0xa014))
#define STADATA         (VPint(Base_Addr+0xa018))
#define STACON          (VPint(Base_Addr+0xa01c))
#define CAMEN           (VPint(Base_Addr+0xa028))
#define EMISSCNT        (VPint(Base_Addr+0xa03c))
#define EPZCNT          (VPint(Base_Addr+0xa040))
#define ERMPZCNT        (VPint(Base_Addr+0xa044))
#define ETXSTAT         (VPint(Base_Addr+0xa048))
#define MACRXDESTR      (VPint(Base_Addr+0xa064))
#define MACRXSTATEM     (VPint(Base_Addr+0xa090))

// HDLC Channel A
#define HCON0A          (VPint(Base_Addr+0x7000))
#define HCON1A          (VPint(Base_Addr+0x7004))
#define HSTATA          (VPint(Base_Addr+0x7008))
#define HINTENA         (VPint(Base_Addr+0x700c))
#define HTXFIFOCA       (VPint(Base_Addr+0x7010))
#define HTXFIFOTA       (VPint(Base_Addr+0x7014))
#define HRXFIFOA        (VPint(Base_Addr+0x7018))
#define HSADRA          (VPint(Base_Addr+0x701c))
#define HBRGTCA         (VPint(Base_Addr+0x7020))
#define HPRMBA          (VPint(Base_Addr+0x7024))
#define HDMATXMAA       (VPint(Base_Addr+0x7028))
#define HDMARXMAA       (VPint(Base_Addr+0x702c))
#define HDMATXCNTA      (VPint(Base_Addr+0x7030))
#define HDMARXCNTA      (VPint(Base_Addr+0x7034))
#define HDMARXBCNTA     (VPint(Base_Addr+0x7038))

// HDLC Channel B
#define HCON0B          (VPint(Base_Addr+0x8000))
#define HCON1B          (VPint(Base_Addr+0x8004))
#define HSTATB          (VPint(Base_Addr+0x8008))
#define HINTENB         (VPint(Base_Addr+0x800c))
#define HTXFIFOCB       (VPint(Base_Addr+0x8010))
#define HTXFIFOTB       (VPint(Base_Addr+0x8014))
#define HRXFIFOB        (VPint(Base_Addr+0x8018))
#define HSADRB          (VPint(Base_Addr+0x801c))
#define HBRGTCB         (VPint(Base_Addr+0x8020))
#define HPRMBB          (VPint(Base_Addr+0x8024))
#define HDMATXMAB       (VPint(Base_Addr+0x8028))
#define HDMARXMAB       (VPint(Base_Addr+0x802c))
#define HDMATXCNTB      (VPint(Base_Addr+0x8030))
#define HDMARXCNTB      (VPint(Base_Addr+0x8034))
#define HDMARXBCNTB 	(VPint(Base_Addr+0x8038))

// I2C Bus Register
#define IICCON          (VPint(Base_Addr+0xf000))
#define IICBUF          (VPint(Base_Addr+0xf004))
#define IICPS           (VPint(Base_Addr+0xf008))
#define IICCOUNT        (VPint(Base_Addr+0xf00c))

// GDMA 0
#define GDMACON0        (VPint(Base_Addr+0xb000))
#define GDMASRC0        (VPint(Base_Addr+0xb004))
#define GDMADST0        (VPint(Base_Addr+0xb008))
#define GDMACNT0        (VPint(Base_Addr+0xb00c))

// GDMA 1
#define GDMACON1        (VPint(Base_Addr+0xc000))
#define GDMASRC1        (VPint(Base_Addr+0xc004))
#define GDMADST1        (VPint(Base_Addr+0xc008))
#define GDMACNT1        (VPint(Base_Addr+0xc00c))

// UART 0
#define UART0_LCR       (VPint(Base_Addr+0xd000))   // line control register
#define UART0_CTRL      (VPint(Base_Addr+0xd004))   // uart control register
#define UART0_LST       (VPint(Base_Addr+0xd008))   // line status register
#define UART0_THR       (VPint(Base_Addr+0xd00c))   // transmit holding reg.
#define UART0_RDR       (VPint(Base_Addr+0xd010))   // receive data register
#define UART0_BRD       (VPint(Base_Addr+0xd014))   // baud rate divisor

// UART 1
#define UART1_LCR       (VPint(Base_Addr+0xe000))   // line control register
#define UART1_CTRL      (VPint(Base_Addr+0xe004))   // uart control register
#define UART1_LST       (VPint(Base_Addr+0xe008))   // line status register
#define UART1_THR       (VPint(Base_Addr+0xe00c))   // transmit holding reg.
#define UART1_RDR       (VPint(Base_Addr+0xe010))   // receive data register
#define UART1_BRD       (VPint(Base_Addr+0xe014))   // baud rate divisor

// Timer Register
#define TMOD            (VPint(Base_Addr+0x6000))
#define TDATA0          (VPint(Base_Addr+0x6004))
#define TDATA1          (VPint(Base_Addr+0x6008))
#define TCNT0           (VPint(Base_Addr+0x600c))
#define TCNT1           (VPint(Base_Addr+0x6010))

// Timer Mode Register
#define  TM0_RUN      0x01  /* Timer 0 enable */
#define  TM0_TOGGLE   0x02  /* 0, interval mode */
#define  TM0_OUT_1    0x04  /* Timer 0 Initial TOUT0 value */
#define  TM1_RUN      0x08  /* Timer 1 enable */
#define  TM1_TOGGLE   0x10  /* 0, interval mode */
#define  TM1_OUT_1    0x20  /* Timer 0 Initial TOUT0 value */

// I/O Port Interface
#define IOPMOD          (VPint(Base_Addr+0x5000))
#define IOPCON          (VPint(Base_Addr+0x5004))
#define IOPDATA         (VPint(Base_Addr+0x5008))

// Interrupt Controller Register
#define INTMODE         (VPint(Base_Addr+0x4000))
#define INTPEND         (VPint(Base_Addr+0x4004))
#define INTMASK         (VPint(Base_Addr+0x4008))

#define INTPRI0         (VPint(Base_Addr+0x400c))
#define INTPRI1         (VPint(Base_Addr+0x4010))
#define INTPRI2         (VPint(Base_Addr+0x4014))
#define INTPRI3         (VPint(Base_Addr+0x4018))
#define INTPRI4         (VPint(Base_Addr+0x401c))
#define INTPRI5         (VPint(Base_Addr+0x4020))
#define INTOFFSET       (VPint(Base_Addr+0x4024))

#endif
