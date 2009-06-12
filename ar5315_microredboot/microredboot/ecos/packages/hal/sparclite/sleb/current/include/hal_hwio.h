#ifndef CYGONCE_HAL_HAL_HWIO_H
#define CYGONCE_HAL_HAL_HWIO_H

/*=============================================================================
//
//      hal_hwio.h
//
//      HAL Support for IO to platform-specific devices
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
// Author(s):   hmt
// Contributors:        hmt
// Date:        1999-01-11
// Purpose:     HAL Support for IO to platform-specific devices
// Description: Macros to access the 86940 SPARClite companion chip
// Usage:       #include <cyg/hal/hal_hwio.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/hal_sparclite.h>
#include <pkgconf/hal_sparclite_sleb.h>

#include <cyg/infra/cyg_type.h>

/*---------------------------------------------------------------------------*/
/* MB86940 flags and the like                                                */

/* Interrupt trigger modes. */
#define HAL_SPARC_86940_TRIG_LEVEL_H   0   /* trigger on high level   */
#define HAL_SPARC_86940_TRIG_LEVEL_L   1   /* trigger on low level    */
#define HAL_SPARC_86940_TRIG_EDGE_H    2   /* trigger on rising edge  */
#define HAL_SPARC_86940_TRIG_EDGE_L    3   /* trigger on falling edge */

/* Timer prescaler register values */
#define HAL_SPARC_86940_PRS_EXTCLK    0x8000
#define HAL_SPARC_86940_PRS_ODIV1     (0<<8)
#define HAL_SPARC_86940_PRS_ODIV2     (1<<8)
#define HAL_SPARC_86940_PRS_ODIV4     (2<<8)
#define HAL_SPARC_86940_PRS_ODIV8     (3<<8)
#define HAL_SPARC_86940_PRS_ODIV16    (4<<8)
#define HAL_SPARC_86940_PRS_ODIV32    (5<<8)
#define HAL_SPARC_86940_PRS_ODIV64    (6<<8)
#define HAL_SPARC_86940_PRS_ODIV128   (7<<8)

/* Timer control register values */
#define HAL_SPARC_86940_TCR_CE          (1<<11)
#define HAL_SPARC_86940_TCR_CLKINT      (0<<9)
#define HAL_SPARC_86940_TCR_CLKEXT      (1<<9)
#define HAL_SPARC_86940_TCR_CLKPRS      (2<<9)
#define HAL_SPARC_86940_TCR_CLKRSVD     (3<<9)
#define HAL_SPARC_86940_TCR_OUTSAME     (0<<7)
#define HAL_SPARC_86940_TCR_OUTHIGH     (1<<7)
#define HAL_SPARC_86940_TCR_OUTLOW      (2<<7)
#define HAL_SPARC_86940_TCR_OUTC3       (3<<7)
#define HAL_SPARC_86940_TCR_INV         (1<<6)
#define HAL_SPARC_86940_TCR_PER_INT     (0<<3)
#define HAL_SPARC_86940_TCR_TO_INT      (1<<3)
#define HAL_SPARC_86940_TCR_SQWAVE      (2<<3)
#define HAL_SPARC_86940_TCR_SW_WATCH    (3<<3)
#define HAL_SPARC_86940_TCR_HW_WATCH    (4<<3)
#define HAL_SPARC_86940_TCR_LEVEL_L     0
#define HAL_SPARC_86940_TCR_LEVEL_H     1
#define HAL_SPARC_86940_TCR_EDGE_H      2
#define HAL_SPARC_86940_TCR_EDGE_L      3
#define HAL_SPARC_86940_TCR_EDGE        4

/* serial mode register values */
#define HAL_SPARC_86940_SER_STOP0       (0<<6)
#define HAL_SPARC_86940_SER_STOP1       (1<<6)
#define HAL_SPARC_86940_SER_STOP1_5     (2<<6)
#define HAL_SPARC_86940_SER_STOP2       (3<<6)
#define HAL_SPARC_86940_SER_NO_PARITY   (0<<4)
#define HAL_SPARC_86940_SER_ODD_PARITY  (1<<4)
#define HAL_SPARC_86940_SER_EVEN_PARITY (3<<4)
#define HAL_SPARC_86940_SER_5BITS       (0<<2)
#define HAL_SPARC_86940_SER_6BITS       (1<<2)
#define HAL_SPARC_86940_SER_7BITS       (2<<2)
#define HAL_SPARC_86940_SER_8BITS       (3<<2)
#define HAL_SPARC_86940_SER_MODE_SYNCH  0
#define HAL_SPARC_86940_SER_DIV1_CLK    1
#define HAL_SPARC_86940_SER_DIV16_CLK   2
#define HAL_SPARC_86940_SER_DIV64_CLK   3

/* serial command register (asynch) */
#define HAL_SPARC_86940_SER_CMD_EHM     (1<<7)
#define HAL_SPARC_86940_SER_CMD_IRST    (1<<6)
#define HAL_SPARC_86940_SER_CMD_RTS     (1<<5)
#define HAL_SPARC_86940_SER_CMD_EFR     (1<<4)
#define HAL_SPARC_86940_SER_CMD_BREAK   (1<<3)
#define HAL_SPARC_86940_SER_CMD_RXEN    (1<<2)
#define HAL_SPARC_86940_SER_CMD_DTR     (1<<1)
#define HAL_SPARC_86940_SER_CMD_TXEN    (1<<0)

/* serial status register */
#define HAL_SPARC_86940_SER_STAT_DSR    (1<<7)
#define HAL_SPARC_86940_SER_STAT_BREAK  (1<<6)
#define HAL_SPARC_86940_SER_STAT_FERR   (1<<5)
#define HAL_SPARC_86940_SER_STAT_OERR   (1<<4)
#define HAL_SPARC_86940_SER_STAT_PERR   (1<<3)
#define HAL_SPARC_86940_SER_STAT_TXEMP  (1<<2)
#define HAL_SPARC_86940_SER_STAT_RXRDY  (1<<1)
#define HAL_SPARC_86940_SER_STAT_TXRDY  (1<<0)

#define HAL_SPARC_86940_CHIP_ASI        4
#define HAL_SPARC_86940_CHIP_BASE       0x10000000
#define HAL_SPARC_86940_REGADDR_SHIFT   2
#define HAL_SPARC_86940_REGVAL_SHIFT    16

/*---------------------------------------------------------------------------*/
/* Register addresses                                                        */

// The "interesting" IO parts are in Address Space Four:
#define HAL_SPARC_ASI_4_READ( addr, res )                                   \
    asm volatile(                                                           \
        "lda [ %1 ] 4, %0"                                                  \
        : "=r"(res)                                                         \
        : "r"(addr)                                                         \
    );

#define HAL_SPARC_ASI_4_WRITE( addr, val )                                  \
    asm volatile(                                                           \
        "sta %0, [ %1 ] 4"                                                  \
        :                                                                   \
        : "r"(val),"r"(addr)                                                \
    );

#define HAL_SPARC_86940_BASE (0x10000000)  // in ASI 4

// The 86940 is connected to the upper 16 bits!
#define HAL_SPARC_86940_READ( reg, result ) CYG_MACRO_START                 \
        cyg_uint32 hires;                                                   \
        HAL_SPARC_ASI_4_READ( reg + HAL_SPARC_86940_BASE, hires );          \
        hires >>= 16;                                                       \
        result = hires;                                                     \
CYG_MACRO_END

// THIS IS ONLY HERE FOR DEBUGGING:
// The 86940 is connected to the upper 16 bits!
// And seems to be an unreliable deprecated thing...
// so read it 3 times and believe the majority.
#define HAL_SPARC_86940_READ3( reg, result ) CYG_MACRO_START                \
        cyg_uint32 hires1;                                                  \
        cyg_uint32 hires2;                                                  \
        cyg_uint32 hires3;                                                  \
        HAL_SPARC_ASI_4_READ( reg + HAL_SPARC_86940_BASE, hires1 );         \
        HAL_SPARC_ASI_4_READ( reg + HAL_SPARC_86940_BASE, hires2 );         \
        HAL_SPARC_ASI_4_READ( reg + HAL_SPARC_86940_BASE, hires3 );         \
        result = ((hires1&hires2)|(hires1&hires3)|(hires2&hires3)) >> 16;   \
CYG_MACRO_END

#define HAL_SPARC_86940_WRITE( reg, value ) CYG_MACRO_START                 \
        cyg_uint32 hival = (value) << 16;                                   \
        HAL_SPARC_ASI_4_WRITE( reg + HAL_SPARC_86940_BASE, hival );         \
CYG_MACRO_END

// Registers are at word offsets
#define HAL_SPARC_86940_REG_SDTR0_TXDATA ( 0x08 * 4 )
#define HAL_SPARC_86940_REG_SDTR0_RXDATA ( 0x08 * 4 )
#define HAL_SPARC_86940_REG_SDTR0_STAT   ( 0x09 * 4 )
#define HAL_SPARC_86940_REG_SDTR0_CTRL   ( 0x09 * 4 )
#define HAL_SPARC_86940_REG_SDTR1_TXDATA ( 0x0c * 4 )
#define HAL_SPARC_86940_REG_SDTR1_RXDATA ( 0x0c * 4 )
#define HAL_SPARC_86940_REG_SDTR1_STAT   ( 0x0d * 4 )
#define HAL_SPARC_86940_REG_SDTR1_CTRL   ( 0x0d * 4 )

#define HAL_SPARC_86940_REG_PRS0         ( 0x10 * 4 )
#define HAL_SPARC_86940_REG_TCR0         ( 0x11 * 4 )
#define HAL_SPARC_86940_REG_RELOAD0      ( 0x12 * 4 )
#define HAL_SPARC_86940_REG_CNT0         ( 0x13 * 4 )
#define HAL_SPARC_86940_REG_PRS1         ( 0x14 * 4 )
#define HAL_SPARC_86940_REG_TCR1         ( 0x15 * 4 )
#define HAL_SPARC_86940_REG_RELOAD1      ( 0x16 * 4 )
#define HAL_SPARC_86940_REG_CNT1         ( 0x17 * 4 )
#define HAL_SPARC_86940_REG_TCR2         ( 0x19 * 4 )
#define HAL_SPARC_86940_REG_RELOAD2      ( 0x1A * 4 )
#define HAL_SPARC_86940_REG_CNT2         ( 0x1B * 4 )
#define HAL_SPARC_86940_REG_TCR3         ( 0x1D * 4 )
#define HAL_SPARC_86940_REG_RELOAD3      ( 0x1E * 4 )
#define HAL_SPARC_86940_REG_CNT3         ( 0x1F * 4 )

// Glue together to access them neatly
#define HAL_SPARC_86940_SDTR0_TXDATA_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_SDTR0_TXDATA, v )
#define HAL_SPARC_86940_SDTR0_RXDATA_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_SDTR0_RXDATA, r )

#define HAL_SPARC_86940_SDTR0_STAT_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_SDTR0_STAT, r )
#define HAL_SPARC_86940_SDTR0_CTRL_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_SDTR0_CTRL, v )

#define HAL_SPARC_86940_SDTR1_TXDATA_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_SDTR1_TXDATA, v )
#define HAL_SPARC_86940_SDTR1_RXDATA_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_SDTR1_RXDATA, r )

#define HAL_SPARC_86940_SDTR1_STAT_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_SDTR1_STAT, r )
#define HAL_SPARC_86940_SDTR1_CTRL_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_SDTR1_CTRL, v )

#define HAL_SPARC_86940_PRS0_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_PRS0, r )
#define HAL_SPARC_86940_PRS0_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_PRS0, v )
    
#define HAL_SPARC_86940_TCR0_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_TCR0, r )
#define HAL_SPARC_86940_TCR0_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_TCR0, v )

#define HAL_SPARC_86940_RELOAD0_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_RELOAD0, r )
#define HAL_SPARC_86940_RELOAD0_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_RELOAD0, v )

#define HAL_SPARC_86940_CNT0_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_CNT0, r )
#define HAL_SPARC_86940_CNT0_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_CNT0, v )

#define HAL_SPARC_86940_PRS1_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_PRS1, r )
#define HAL_SPARC_86940_PRS1_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_PRS1, v )

#define HAL_SPARC_86940_TCR1_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_TCR1, r )
#define HAL_SPARC_86940_TCR1_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_TCR1, v )

#define HAL_SPARC_86940_RELOAD1_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_RELOAD1, r )
#define HAL_SPARC_86940_RELOAD1_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_RELOAD1, v )

#define HAL_SPARC_86940_CNT1_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_CNT1, r )
#define HAL_SPARC_86940_CNT1_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_CNT1, v )

#define HAL_SPARC_86940_TCR2_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_TCR2, r )
#define HAL_SPARC_86940_TCR2_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_TCR2, v )

#define HAL_SPARC_86940_RELOAD2_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_RELOAD2, r )
#define HAL_SPARC_86940_RELOAD2_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_RELOAD2, v )

#define HAL_SPARC_86940_CNT2_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_CNT2, r )
#define HAL_SPARC_86940_CNT2_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_CNT2, v )

#define HAL_SPARC_86940_TCR3_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_TCR3, r )
#define HAL_SPARC_86940_TCR3_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_TCR3, v )

#define HAL_SPARC_86940_RELOAD3_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_RELOAD3, r )
#define HAL_SPARC_86940_RELOAD3_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_RELOAD3, v )

#define HAL_SPARC_86940_CNT3_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_CNT3, r )
#define HAL_SPARC_86940_CNT3_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_CNT3, v )

/*---------------------------------------------------------------------------*/
/* end of hal_hwio.h                                                         */
#endif /* CYGONCE_HAL_HAL_HWIO_H */
