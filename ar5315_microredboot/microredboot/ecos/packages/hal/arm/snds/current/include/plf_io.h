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
// Purpose:     ARM/KS32C platform specific registers
// Description: 
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

// Platform doesn't need address munging even if configures as big-endian

#define HAL_IO_MACROS_NO_ADDRESS_MUNGING

// non-caching by accessing addr|0x04000000

#define KS32C_REG_BASE              0x07ff0000

// -----------------------------------------------------------------------------
// System config (register bases and caching)
#define KS32C_SYSCFG                (KS32C_REG_BASE + 0x0000)

#define KS32C_SYSCFG_SDM            0x80000000
#define KS32C_SYSCFG_PD_ID_MASK     0x3c000000
#define KS32C_SYSCFG_SRBBP_MASK     0x03ff0000 // address/64k
#define KS32C_SYSCFG_ISBBP_MASK     0x0000ffc0 // a25-a16
#define KS32C_SYSCFG_CM_MASK        0x00000030
#define KS32C_SYSCFG_CM_4R_4C       0x00000000
#define KS32C_SYSCFG_CM_0R_8C       0x00000010
#define KS32C_SYSCFG_CM_8R_0C       0x00000020
#define KS32C_SYSCFG_WE             0x00000004 // only KS32C50100?
#define KS32C_SYSCFG_CE             0x00000002
#define KS32C_SYSCFG_SE             0x00000001

#define KS32C_CLKCON                (KS32C_REG_BASE + 0x3000)

#define KS32C_EXTACON0              (KS32C_REG_BASE + 0x3008)
#define KS32C_EXTACON1              (KS32C_REG_BASE + 0x300c)

//-----------------------------------------------------------------------------
// Memory banks data width
#define KS32C_EXTDBWTH              (KS32C_REG_BASE + 0x3010)

#define KS32C_EXTDBWTH_MASK         3
#define KS32C_EXTDBWTH_8BIT         1
#define KS32C_EXTDBWTH_16BIT        2
#define KS32C_EXTDBWTH_32BIT        3

#define KS32C_EXTDBWTH_DSR0_shift   0
#define KS32C_EXTDBWTH_DSR1_shift   2
#define KS32C_EXTDBWTH_DSR2_shift   4
#define KS32C_EXTDBWTH_DSR3_shift   6
#define KS32C_EXTDBWTH_DSR4_shift   8
#define KS32C_EXTDBWTH_DSR5_shift   10
#define KS32C_EXTDBWTH_DSD0_shift   12
#define KS32C_EXTDBWTH_DSD1_shift   14
#define KS32C_EXTDBWTH_DSD2_shift   16
#define KS32C_EXTDBWTH_DSD3_shift   18
#define KS32C_EXTDBWTH_DSX0_shift   20
#define KS32C_EXTDBWTH_DSX1_shift   22
#define KS32C_EXTDBWTH_DSX2_shift   24
#define KS32C_EXTDBWTH_DSX3_shift   26

// -----------------------------------------------------------------------------
// Bank locations and timing
#define KS32C_ROMCON0               (KS32C_REG_BASE + 0x3014)
#define KS32C_ROMCON1               (KS32C_REG_BASE + 0x3018)
#define KS32C_ROMCON2               (KS32C_REG_BASE + 0x301c)
#define KS32C_ROMCON3               (KS32C_REG_BASE + 0x3020)
#define KS32C_ROMCON4               (KS32C_REG_BASE + 0x3024)
#define KS32C_ROMCON5               (KS32C_REG_BASE + 0x3028)

#define KS32C_ROMCON_PMC_MASK       0x00000003
#define KS32C_ROMCON_PMC_ROM        0x00000000
#define KS32C_ROMCON_PMC_4W_PAGE    0x00000001
#define KS32C_ROMCON_PMC_8W_PAGE    0x00000002
#define KS32C_ROMCON_PMC_16W_PAGE   0x00000003

#define KS32C_ROMCON_TPA_MASK       0x0000000c
#define KS32C_ROMCON_TPA_5C         0x00000000
#define KS32C_ROMCON_TPA_2C         0x00000004
#define KS32C_ROMCON_TPA_3C         0x00000008
#define KS32C_ROMCON_TPA_4C         0x0000000c

#define KS32C_ROMCON_TACC_MASK      0x00000070
#define KS32C_ROMCON_TACC_DISABLE   0x00000000
#define KS32C_ROMCON_TACC_2C        0x00000010
#define KS32C_ROMCON_TACC_3C        0x00000020
#define KS32C_ROMCON_TACC_4C        0x00000030
#define KS32C_ROMCON_TACC_5C        0x00000040
#define KS32C_ROMCON_TACC_6C        0x00000050
#define KS32C_ROMCON_TACC_7C        0x00000060

#define KS32C_ROMCON_BASE_MASK      0x000ffc00
#define KS32C_ROMCON_BASE_shift     10

#define KS32C_ROMCON_NEXT_MASK      0x3ff00000
#define KS32C_ROMCON_NEXT_shift     20



#define KS32C_DRAMCON0              (KS32C_REG_BASE + 0x302c)
#define KS32C_DRAMCON1              (KS32C_REG_BASE + 0x3030)
#define KS32C_DRAMCON2              (KS32C_REG_BASE + 0x3034)
#define KS32C_DRAMCON3              (KS32C_REG_BASE + 0x3038)

#define KS32C_DRAMCON_CAN_8         0x00000000
#define KS32C_DRAMCON_CAN_9         0x40000000
#define KS32C_DRAMCON_CAN_10        0x80000000
#define KS32C_DRAMCON_CAN_11        0xc0000000
#define KS32C_DRAMCON_TRP_1C        0x00000000
#define KS32C_DRAMCON_TRP_2C        0x00000100
#define KS32C_DRAMCON_TRP_3C        0x00000200
#define KS32C_DRAMCON_TRP_4C        0x00000300
#define KS32C_DRAMCON_TRC_1C        0x00000000
#define KS32C_DRAMCON_TRC_2C        0x00000080
#define KS32C_DRAMCON_RESERVED      0x00000010
#define KS32C_DRAMCON_TCP_1C        0x00000000
#define KS32C_DRAMCON_TCP_2C        0x00000008
#define KS32C_DRAMCON_TCS_1C        0x00000000
#define KS32C_DRAMCON_TCS_2C        0x00000002
#define KS32C_DRAMCON_TCS_3C        0x00000004
#define KS32C_DRAMCON_TCS_4C        0x00000006
#define KS32C_DRAMCON_EDO           0x00000001

#define KS32C_DRAMCON_BASE_MASK      0x000ffc00
#define KS32C_DRAMCON_BASE_shift     10

#define KS32C_DRAMCON_NEXT_MASK      0x3ff00000
#define KS32C_DRAMCON_NEXT_shift     20


#define KS32C_REFEXTCON             (KS32C_REG_BASE + 0x303c)

// DRAM
#define KS32C_REFEXTCON_TCSR_1C     0x00000000
#define KS32C_REFEXTCON_TCHR_1C     0x00000000
// SDRAM
#define KS32C_REFEXTCON_TRC_4C      0x00060000
// DRAM+SDRAM
#define KS32C_REFEXTCON_REN         0x00010000
#define KS32C_REFEXTCON_VSF         0x00008000
#define KS32C_REFEXTCON_BASE        0x00000360

#define KS32C_REFEXTCON_RCV_shift   21

//-----------------------------------------------------------------------------
// INTC

#define KS32C_INTMOD                (KS32C_REG_BASE + 0x4000)
#define KS32C_INTPND                (KS32C_REG_BASE + 0x4004)
#define KS32C_INTMSK                (KS32C_REG_BASE + 0x4008)
#define KS32C_INTPRI0               (KS32C_REG_BASE + 0x400c)
#define KS32C_INTPRI1               (KS32C_REG_BASE + 0x4010)
#define KS32C_INTPRI2               (KS32C_REG_BASE + 0x4014)
#define KS32C_INTPRI3               (KS32C_REG_BASE + 0x4018)
#define KS32C_INTPRI4               (KS32C_REG_BASE + 0x401c)
#define KS32C_INTPRI5               (KS32C_REG_BASE + 0x4020)
#define KS32C_INTOFFSET             (KS32C_REG_BASE + 0x4024)
#define KS32C_PNDPRI                (KS32C_REG_BASE + 0x4028)
#define KS32C_PNDTEST               (KS32C_REG_BASE + 0x402c)
#define KS32C_INTOFFSET_FIQ         (KS32C_REG_BASE + 0x4030)
#define KS32C_INTOFFSET_IRQ         (KS32C_REG_BASE + 0x4034)

#define KS32C_INTMSK_GLOBAL         (1<<21)

//-----------------------------------------------------------------------------
// PIO

#define KS32C_IOPMOD                (KS32C_REG_BASE + 0x5000)
#define KS32C_IOPCON                (KS32C_REG_BASE + 0x5004)
#define KS32C_IOPDATA               (KS32C_REG_BASE + 0x5008)

//-----------------------------------------------------------------------------
// Timers

#define KS32C_TMOD                  (KS32C_REG_BASE + 0x6000)
#define KS32C_TDATA0                (KS32C_REG_BASE + 0x6004)
#define KS32C_TDATA1                (KS32C_REG_BASE + 0x6008)
#define KS32C_TCNT0                 (KS32C_REG_BASE + 0x600c)
#define KS32C_TCNT1                 (KS32C_REG_BASE + 0x6010)

#define KS32C_TMOD_TE0              0x00000001
#define KS32C_TMOD_TMD0             0x00000002
#define KS32C_TMOD_TCLR0            0x00000004
#define KS32C_TMOD_TE1              0x00000008
#define KS32C_TMOD_TMD1             0x00000010
#define KS32C_TMOD_TCLR1            0x00000020


//-----------------------------------------------------------------------------
// UART

#define KS32C_UART0_BASE            (KS32C_REG_BASE + 0xd000)
#define KS32C_UART1_BASE            (KS32C_REG_BASE + 0xe000)

#define KS32C_UART_LCON             0x0000
#define KS32C_UART_CON              0x0004
#define KS32C_UART_STAT             0x0008
#define KS32C_UART_TXBUF            0x000c
#define KS32C_UART_RXBUF            0x0010
#define KS32C_UART_BRDIV            0x0014
#define KS32C_UART_BRDCNT           0x0018
#define KS32C_UART_BRDCLK           0x001c

#define KS32C_UART_LCON_5_DBITS     0x00
#define KS32C_UART_LCON_6_DBITS     0x01
#define KS32C_UART_LCON_7_DBITS     0x02
#define KS32C_UART_LCON_8_DBITS     0x03
#define KS32C_UART_LCON_1_SBITS     0x00
#define KS32C_UART_LCON_2_SBITS     0x04
#define KS32C_UART_LCON_NO_PARITY   0x00
#define KS32C_UART_LCON_EVEN_PARITY 0x00
#define KS32C_UART_LCON_ODD_PARITY  0x28
#define KS32C_UART_LCON_1_PARITY    0x30
#define KS32C_UART_LCON_0_PARITY    0x38
#define KS32C_UART_LCON_SCS         0x40
#define KS32C_UART_LCON_IR          0x80

#define KS32C_UART_CON_RXM_MASK     0x03
#define KS32C_UART_CON_RXM_INT      0x01
#define KS32C_UART_CON_TXM_MASK     0x0c
#define KS32C_UART_CON_TXM_INT      0x08
#define KS32C_UART_CON_RX_ERR_INT   0x04


#define KS32C_UART_STAT_DTR         0x10
#define KS32C_UART_STAT_RDR         0x20
#define KS32C_UART_STAT_TXE         0x40  // tx empty
#define KS32C_UART_STAT_TC          0x80  // tx complete

//-----------------------------------------------------------------------------
// Cache
#define KS32C_CACHE_SET0_ADDR       0x14000000
#define KS32C_CACHE_SET1_ADDR       0x14800000
#define KS32C_CACHE_TAG_ADDR        0x15000000

//-----------------------------------------------------------------------------
// Memory map is 1-1

#define CYGARC_PHYSICAL_ADDRESS(_x_) (_x_)

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H
