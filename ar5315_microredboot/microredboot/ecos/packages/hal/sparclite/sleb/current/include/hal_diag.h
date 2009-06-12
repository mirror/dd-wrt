#ifndef CYGONCE_HAL_HAL_DIAG_H
#define CYGONCE_HAL_HAL_DIAG_H

/*=============================================================================
//
//      hal_diag.h
//
//      HAL Support for Kernel Diagnostic Routines
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
// Purpose:     HAL Support for Kernel Diagnostic Routines
// Description: Diagnostic routines for use during kernel development.
// Usage:       #include <cyg/hal/hal_diag.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/hal_sparclite.h>
#include <pkgconf/hal_sparclite_sleb.h>

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_hwio.h>

/*---------------------------------------------------------------------------*/
/* SPARClite Evaluation Board - SLEB - 86940 SDTR output (+ the LEDs)        */

#define SLEB_LED (*(volatile char *)(0x02000003))

#define SLEB_86940_SDTR0_OUT
#ifdef  SLEB_86940_SDTR0_OUT


#define HAL_SPARC_86940_FLAG_TXRDY  (0x01)
#define HAL_SPARC_86940_FLAG_RXRDY  (0x02)

#define HAL_DIAG_WRITE_CHAR_DIRECT(_c_) CYG_MACRO_START                     \
    cyg_uint32 status = 0;                                                  \
    while ( 0 == (HAL_SPARC_86940_FLAG_TXRDY & status) ) {                  \
        HAL_SPARC_86940_SDTR0_STAT_READ( status );                          \
    }                                                                       \
    HAL_SPARC_86940_SDTR0_TXDATA_WRITE( _c_ );                              \
CYG_MACRO_END

#define HAL_DIAG_WRITE_CHAR_WAIT_FOR_EMPTY() CYG_MACRO_START                \
    cyg_uint32 status = HAL_SPARC_86940_FLAG_TXRDY;                         \
    while ( 0 != (HAL_SPARC_86940_FLAG_TXRDY & status) ) {                  \
        HAL_SPARC_86940_SDTR0_STAT_READ( status );                          \
    }                                                                       \
CYG_MACRO_END



#ifdef CYG_KERNEL_DIAG_GDB
// then use routines from hal_diag.c:

externC void hal_diag_init(void);
externC void hal_diag_write_char(char c);
#define HAL_DIAG_INIT() hal_diag_init()
#define HAL_DIAG_WRITE_CHAR(_c_) CYG_MACRO_START                    \
    SLEB_LED = (_c_); /* immediately */                             \
    hal_diag_write_char(_c_);                                       \
CYG_MACRO_END


#else // CYG_KERNEL_DIAG_GDB
// else go to the serial line directly (after initialization):

externC void hal_diag_init(void);
#define HAL_DIAG_INIT() hal_diag_init()

#define HAL_DIAG_WRITE_CHAR(_c_) CYG_MACRO_START                    \
    SLEB_LED = (_c_);                                               \
    HAL_DIAG_WRITE_CHAR_DIRECT( _c_ );                              \
CYG_MACRO_END

#endif // CYG_KERNEL_DIAG_GDB



#define HAL_DIAG_READ_CHAR(_c_) (_c_) = 0


#else
/*---------------------------------------------------------------------------*/
/* SPARClite Evaluation Board - SLEB - Just use the LEDs on board            */

#define HAL_DIAG_INIT()

#define HAL_DIAG_WRITE_CHAR(_c_) CYG_MACRO_START                            \
      SLEB_LED = (_c_);                                                     \
CYG_MACRO_END

#define HAL_DIAG_READ_CHAR(_c_) (_c_) = 0


#endif // SLEB_86940_SDTR0_OUT

/*---------------------------------------------------------------------------*/
/* end of hal_diag.h                                                         */
#endif /* CYGONCE_HAL_HAL_DIAG_H */
