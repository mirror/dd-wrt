#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H
//=============================================================================
//
//      plf_io.h
//
//      Platform specific registers
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
// Author(s):   jskov
// Contributors:jskov
// Date:        2001-03-16
// Purpose:     ARM/E7T platform specific registers
// Description: 
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

// Platform doesn't need address munging even if configures as big-endian

#define HAL_IO_MACROS_NO_ADDRESS_MUNGING

// non-caching by accessing addr|0x04000000

#define E7T_REG_BASE              0x07ff0000

// -----------------------------------------------------------------------------
// System config (register bases and caching)
#define E7T_SYSCFG                (E7T_REG_BASE + 0x0000)

#define E7T_SYSCFG_SDM            0x80000000
#define E7T_SYSCFG_PD_ID_MASK     0x3c000000
#define E7T_SYSCFG_SRBBP_MASK     0x03ff0000 // address/64k
#define E7T_SYSCFG_ISBBP_MASK     0x0000ffc0 // a25-a16
#define E7T_SYSCFG_CM_MASK        0x00000030
#define E7T_SYSCFG_CM_4R_4C       0x00000000
#define E7T_SYSCFG_CM_0R_8C       0x00000010
#define E7T_SYSCFG_CM_8R_0C       0x00000020
#define E7T_SYSCFG_WE             0x00000004 // only KS32C50100?
#define E7T_SYSCFG_CE             0x00000002
#define E7T_SYSCFG_SE             0x00000001

#define E7T_CLKCON                (E7T_REG_BASE + 0x3000)

#define E7T_EXTACON0              (E7T_REG_BASE + 0x3008)
#define E7T_EXTACON1              (E7T_REG_BASE + 0x300c)

//-----------------------------------------------------------------------------
// Memory banks data width
#define E7T_EXTDBWTH              (E7T_REG_BASE + 0x3010)

#define E7T_EXTDBWTH_MASK         3
#define E7T_EXTDBWTH_8BIT         1
#define E7T_EXTDBWTH_16BIT        2
#define E7T_EXTDBWTH_32BIT        3

#define E7T_EXTDBWTH_DSR0_shift   0
#define E7T_EXTDBWTH_DSR1_shift   2
#define E7T_EXTDBWTH_DSR2_shift   4
#define E7T_EXTDBWTH_DSR3_shift   6
#define E7T_EXTDBWTH_DSR4_shift   8
#define E7T_EXTDBWTH_DSR5_shift   10
#define E7T_EXTDBWTH_DSD0_shift   12
#define E7T_EXTDBWTH_DSD1_shift   14
#define E7T_EXTDBWTH_DSD2_shift   16
#define E7T_EXTDBWTH_DSD3_shift   18
#define E7T_EXTDBWTH_DSX0_shift   20
#define E7T_EXTDBWTH_DSX1_shift   22
#define E7T_EXTDBWTH_DSX2_shift   24
#define E7T_EXTDBWTH_DSX3_shift   26

// -----------------------------------------------------------------------------
// Bank locations and timing
#define E7T_ROMCON0               (E7T_REG_BASE + 0x3014)
#define E7T_ROMCON1               (E7T_REG_BASE + 0x3018)
#define E7T_ROMCON2               (E7T_REG_BASE + 0x301c)
#define E7T_ROMCON3               (E7T_REG_BASE + 0x3020)
#define E7T_ROMCON4               (E7T_REG_BASE + 0x3024)
#define E7T_ROMCON5               (E7T_REG_BASE + 0x3028)

#define E7T_ROMCON_PMC_MASK       0x00000003
#define E7T_ROMCON_PMC_ROM        0x00000000
#define E7T_ROMCON_PMC_4W_PAGE    0x00000001
#define E7T_ROMCON_PMC_8W_PAGE    0x00000002
#define E7T_ROMCON_PMC_16W_PAGE   0x00000003

#define E7T_ROMCON_TPA_MASK       0x0000000c
#define E7T_ROMCON_TPA_5C         0x00000000
#define E7T_ROMCON_TPA_2C         0x00000004
#define E7T_ROMCON_TPA_3C         0x00000008
#define E7T_ROMCON_TPA_4C         0x0000000c

#define E7T_ROMCON_TACC_MASK      0x00000070
#define E7T_ROMCON_TACC_DISABLE   0x00000000
#define E7T_ROMCON_TACC_2C        0x00000010
#define E7T_ROMCON_TACC_3C        0x00000020
#define E7T_ROMCON_TACC_4C        0x00000030
#define E7T_ROMCON_TACC_5C        0x00000040
#define E7T_ROMCON_TACC_6C        0x00000050
#define E7T_ROMCON_TACC_7C        0x00000060

#define E7T_ROMCON_BASE_MASK      0x000ffc00
#define E7T_ROMCON_BASE_shift     10

#define E7T_ROMCON_NEXT_MASK      0x3ff00000
#define E7T_ROMCON_NEXT_shift     20



#define E7T_DRAMCON0              (E7T_REG_BASE + 0x302c)
#define E7T_DRAMCON1              (E7T_REG_BASE + 0x3030)
#define E7T_DRAMCON2              (E7T_REG_BASE + 0x3034)
#define E7T_DRAMCON3              (E7T_REG_BASE + 0x3038)
#define E7T_REFEXTCON             (E7T_REG_BASE + 0x303c)

//-----------------------------------------------------------------------------
// INTC

#define E7T_INTMOD                (E7T_REG_BASE + 0x4000)
#define E7T_INTPND                (E7T_REG_BASE + 0x4004)
#define E7T_INTMSK                (E7T_REG_BASE + 0x4008)
#define E7T_INTPRI0               (E7T_REG_BASE + 0x400c)
#define E7T_INTPRI1               (E7T_REG_BASE + 0x4010)
#define E7T_INTPRI2               (E7T_REG_BASE + 0x4014)
#define E7T_INTPRI3               (E7T_REG_BASE + 0x4018)
#define E7T_INTPRI4               (E7T_REG_BASE + 0x401c)
#define E7T_INTPRI5               (E7T_REG_BASE + 0x4020)
#define E7T_INTOFFSET             (E7T_REG_BASE + 0x4024)
#define E7T_PNDPRI                (E7T_REG_BASE + 0x4028)
#define E7T_PNDTEST               (E7T_REG_BASE + 0x402c)
#define E7T_INTOFFSET_FIQ         (E7T_REG_BASE + 0x4030)
#define E7T_INTOFFSET_IRQ         (E7T_REG_BASE + 0x4034)

#define E7T_INTMSK_GLOBAL         (1<<21)

//-----------------------------------------------------------------------------
// PIO

#define E7T_IOPMOD                (E7T_REG_BASE + 0x5000)
#define E7T_IOPCON                (E7T_REG_BASE + 0x5004)
#define E7T_IOPDATA               (E7T_REG_BASE + 0x5008)

//-----------------------------------------------------------------------------
// Timers

#define E7T_TMOD                  (E7T_REG_BASE + 0x6000)
#define E7T_TDATA0                (E7T_REG_BASE + 0x6004)
#define E7T_TDATA1                (E7T_REG_BASE + 0x6008)
#define E7T_TCNT0                 (E7T_REG_BASE + 0x600c)
#define E7T_TCNT1                 (E7T_REG_BASE + 0x6010)

#define E7T_TMOD_TE0              0x00000001
#define E7T_TMOD_TMD0             0x00000002
#define E7T_TMOD_TCLR0            0x00000004
#define E7T_TMOD_TE1              0x00000008
#define E7T_TMOD_TMD1             0x00000010
#define E7T_TMOD_TCLR1            0x00000020


//-----------------------------------------------------------------------------
// UART

#define E7T_UART0_BASE            (E7T_REG_BASE + 0xd000)
#define E7T_UART1_BASE            (E7T_REG_BASE + 0xe000)

#define E7T_UART_LCON             0x0000
#define E7T_UART_CON              0x0004
#define E7T_UART_STAT             0x0008
#define E7T_UART_TXBUF            0x000c
#define E7T_UART_RXBUF            0x0010
#define E7T_UART_BRDIV            0x0014
#define E7T_UART_BRDCNT           0x0018
#define E7T_UART_BRDCLK           0x001c

#define E7T_UART_LCON_5_DBITS     0x00
#define E7T_UART_LCON_6_DBITS     0x01
#define E7T_UART_LCON_7_DBITS     0x02
#define E7T_UART_LCON_8_DBITS     0x03
#define E7T_UART_LCON_1_SBITS     0x00
#define E7T_UART_LCON_2_SBITS     0x04
#define E7T_UART_LCON_NO_PARITY   0x00
#define E7T_UART_LCON_EVEN_PARITY 0x00
#define E7T_UART_LCON_ODD_PARITY  0x28
#define E7T_UART_LCON_1_PARITY    0x30
#define E7T_UART_LCON_0_PARITY    0x38
#define E7T_UART_LCON_SCS         0x40
#define E7T_UART_LCON_IR          0x80

#define E7T_UART_CON_RXM_MASK     0x03
#define E7T_UART_CON_RXM_INT      0x01
#define E7T_UART_CON_TXM_MASK     0x0c
#define E7T_UART_CON_TXM_INT      0x08
#define E7T_UART_CON_RX_ERR_INT   0x04


#define E7T_UART_STAT_DTR         0x10
#define E7T_UART_STAT_RDR         0x20
#define E7T_UART_STAT_TXE         0x40  // tx empty
#define E7T_UART_STAT_TC          0x80  // tx complete

//-----------------------------------------------------------------------------
// Cache
#define E7T_CACHE_SET0_ADDR       0x14000000
#define E7T_CACHE_SET1_ADDR       0x14800000
#define E7T_CACHE_TAG_ADDR        0x15000000

//-----------------------------------------------------------------------------
// Memory map is 1-1

#define CYGARC_PHYSICAL_ADDRESS(_x_) (_x_)

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H
