#ifndef CYGONCE_HAL_PLF_REGS_H
#define CYGONCE_HAL_PLF_REGS_H

//==========================================================================
//
//      plf_regs.h
//
//      PowerPC 82xx platform CPU definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Gary Thomas
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
// Contributors: gthomas
// Date:         2002-06-27
// Purpose:      
// Description:  Possibly override any platform assumptions
//
// Usage:        Included via the variant+architecture register headers:
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#define _CSB281_PCI_CONFIG_ADDR    0xFEC00000   // PCI configuration cycle address
#define _CSB281_PCI_CONFIG_DATA    0xFEE00000   // PCI configuration cycle data

#define _CSB281_BCSR               0xFF000000   // Board control (16 bit access)
#define _CSB281_BCSR_IDSEL1          0x0001     // Select PCI slot 0
#define _CSB281_BCSR_IDSEL2          0x0002     // Select PCI slot 1
#define _CSB281_BCSR_IDSEL3          0x0004     // Select GD82559 (PCI)
#define _CSB281_BCSR_LED0            0x0008     // 0 => LED0 on
#define _CSB281_BCSR_LED1            0x0010     // 0 => LED1 on
#define _CSB281_BCSR_PRESET          0x0020     // 0 => Reset peripherals (PCI, etc)
#define _CSB281_BCSR_SMI             0x0040     // 1 => enable SMI via SW0
#define _CSB281_BCSR_NMI             0x0080     // 1 => enable SMI via SW1
#define _CSB281_BCSR_USER0           0x0100     // 0 => DIP switch 0 on
#define _CSB281_BCSR_USER1           0x0200     // 0 => DIP switch 1 on
#define _CSB281_BCSR_USER2           0x0400     // 0 => DIP switch 2 on
#define _CSB281_BCSR_USER3           0x0800     // 0 => DIP switch 3 on
#define _CSB281_BCSR_SW0             0x1000     // 0 => SW0 pressed
#define _CSB281_BCSR_SW1             0x2000     // 0 => SW1 pressed

#define _CSB281_2WCSR              0xFF000100   // 2wire controller (32 bit access)
#define _CSB281_2WCSR_CLR_ALL        0x0000     // SDA=0, SCL=0
#define _CSB281_2WCSR_SET_ALL        0x00FF     // SDA=1, SCL=1
#define _CSB281_2WCSR_CLR_SDA        0x0004     // SDA=0
#define _CSB281_2WCSR_SET_SDA        0x0008     // SDA=1
#define _CSB281_2WCSR_CLR_SCL        0x0001     // SCL=0
#define _CSB281_2WCSR_SET_SCL        0x0002     // SCL=1
#define _CSB281_2WCSR_GET_SCL        0x0002     // SCL=?
#define _CSB281_2WCSR_GET_SDA        0x0001     // SDA=?

#define _CSB281_FS6377_DEV         0x58

#define _CSB281_EUMBBAR            0xF0000000   // Internal registers

// Interrupt controller
#define _CSB281_EPIC               (_CSB281_EUMBBAR+0x40000)
#define _CSB281_EPIC_FRR           (_CSB281_EPIC+0x01000)  // Feature reporting register
#define _CSB281_EPIC_GCR           (_CSB281_EPIC+0x01020)  // Global configuration
#define _CSB281_EPIC_GCR_R            0x80000000           // Reset
#define _CSB281_EPIC_GCR_M            0x20000000           // Mode
#define _CSB281_EPIC_EICR          (_CSB281_EPIC+0x01030)  // Interrupt configuration
#define _CSB281_EPIC_EICR_SIE         0x08000000           // Serial interrupt enable
#define _CSB281_EPIC_EVI           (_CSB281_EPIC+0x01080)  // Vendor identification
#define _CSB281_EPIC_PI            (_CSB281_EPIC+0x01090)  // Processor initialization
#define _CSB281_EPIC_SVR           (_CSB281_EPIC+0x010E0)  // Spurious interrupt
#define _CSB281_EPIC_TFRR          (_CSB281_EPIC+0x010F0)  // Timer frequency
#define _CSB281_EPIC_TCR           (_CSB281_EPIC+0x010F4)  // Timer control
#define _CSB281_EPIC_GTCCR0        (_CSB281_EPIC+0x01100)  // Timer 0 - current count
#define _CSB281_EPIC_GTBCR0        (_CSB281_EPIC+0x01110)  // Timer 0 - base  count
#define _CSB281_EPIC_GTVPR0        (_CSB281_EPIC+0x01120)  // Timer 0 - vector/priority
#define _CSB281_EPIC_GTDR0         (_CSB281_EPIC+0x01130)  // Timer 0 - destination
#define _CSB281_EPIC_GTCCR1        (_CSB281_EPIC+0x01140)  // Timer 1 - current count
#define _CSB281_EPIC_GTBCR1        (_CSB281_EPIC+0x01150)  // Timer 1 - base  count
#define _CSB281_EPIC_GTVPR1        (_CSB281_EPIC+0x01160)  // Timer 1 - vector/priority
#define _CSB281_EPIC_GTDR1         (_CSB281_EPIC+0x01170)  // Timer 1 - destination
#define _CSB281_EPIC_GTCCR2        (_CSB281_EPIC+0x01180)  // Timer 2 - current count
#define _CSB281_EPIC_GTBCR2        (_CSB281_EPIC+0x01190)  // Timer 2 - base  count
#define _CSB281_EPIC_GTVPR2        (_CSB281_EPIC+0x011A0)  // Timer 2 - vector/priority
#define _CSB281_EPIC_GTDR2         (_CSB281_EPIC+0x011B0)  // Timer 2 - destination
#define _CSB281_EPIC_GTCCR3        (_CSB281_EPIC+0x011C0)  // Timer 2 - current count
#define _CSB281_EPIC_GTBCR3        (_CSB281_EPIC+0x011D0)  // Timer 2 - base  count
#define _CSB281_EPIC_GTVPR3        (_CSB281_EPIC+0x011E0)  // Timer 2 - vector/priority
#define _CSB281_EPIC_IVPR0         (_CSB281_EPIC+0x10200)  // IRQ 0 - vector/priority
#define _CSB281_EPIC_IDR0          (_CSB281_EPIC+0x10210)  // IRQ 0 - destination
#define _CSB281_EPIC_IVPR1         (_CSB281_EPIC+0x10220)  // IRQ 1 - vector/priority
#define _CSB281_EPIC_IDR1          (_CSB281_EPIC+0x10230)  // IRQ 1 - destination
#define _CSB281_EPIC_IVPR2         (_CSB281_EPIC+0x10240)  // IRQ 2 - vector/priority
#define _CSB281_EPIC_IDR2          (_CSB281_EPIC+0x10250)  // IRQ 2 - destination
#define _CSB281_EPIC_IVPR3         (_CSB281_EPIC+0x10260)  // IRQ 3 - vector/priority
#define _CSB281_EPIC_IDR3          (_CSB281_EPIC+0x10270)  // IRQ 3 - destination
#define _CSB281_EPIC_IVPR4         (_CSB281_EPIC+0x10280)  // IRQ 4 - vector/priority
#define _CSB281_EPIC_IDR4          (_CSB281_EPIC+0x10290)  // IRQ 4 - destination
#define _CSB281_EPIC_I2CVPR        (_CSB281_EPIC+0x11020)  // I2C - vector/priority
#define _CSB281_EPIC_I2CDR         (_CSB281_EPIC+0x11030)  // I2C - destination
#define _CSB281_EPIC_DMA0VPR       (_CSB281_EPIC+0x11040)  // DMA0 - vector/priority
#define _CSB281_EPIC_DMA0DR        (_CSB281_EPIC+0x11050)  // DMA0 - destination
#define _CSB281_EPIC_DMA1VPR       (_CSB281_EPIC+0x11060)  // DMA1 - vector/priority
#define _CSB281_EPIC_DMA1DR        (_CSB281_EPIC+0x11070)  // DMA1 - destination
#define _CSB281_EPIC_MSGVPR        (_CSB281_EPIC+0x110C0)  // MSG - vector/priority
#define _CSB281_EPIC_MSGDR         (_CSB281_EPIC+0x110D0)  // MSG - destination
#define _CSB281_EPIC_UART0VPR      (_CSB281_EPIC+0x11120)  // UART0 - vector/priority
#define _CSB281_EPIC_UART0DR       (_CSB281_EPIC+0x11130)  // UART0 - destination
#define _CSB281_EPIC_UART1VPR      (_CSB281_EPIC+0x11140)  // UART1 - vector/priority
#define _CSB281_EPIC_UART1DR       (_CSB281_EPIC+0x11150)  // UART1 - destination
#define _CSB281_EPIC_PCTPR         (_CSB281_EPIC+0x20080)  // Processor current task priority
#define _CSB281_EPIC_IACK          (_CSB281_EPIC+0x200A0)  // Interrupt ack (vector)
#define _CSB281_EPIC_EOI           (_CSB281_EPIC+0x200B0)  // End of interrupt

#define _CSB281_EPIC_PVR_M            0x80000000           // Interrupt masked
#define _CSB281_EPIC_PVR_A            0x40000000           // Interrupt active
#define _CSB281_EPIC_PVR_P            0x00800000           // Polarity 0 = active low
#define _CSB281_EPIC_PVR_S            0x00400000           // Sense 0 = edge
#define _CSB281_EPIC_PVR_PRIO_SHIFT   16
#define _CSB281_EPIC_PVR_PRIO_MASK    0xF
#define _CSB281_EPIC_PVR_VEC_SHIFT    0
#define _CSB281_EPIC_PVR_VEC_MASK     0xFF

#define _zero_bit(_val_, _bit_) _val_ & ~_bit_
#define _one_bit(_val_, _bit_) _val_ | _bit_

#endif // CYGONCE_HAL_PLF_REGS_H
