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

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>

/*---------------------------------------------------------------------------*/

#define CYG_DIAG_USE_ERC32

/*---------------------------------------------------------------------------*/

#ifdef  CYG_DIAG_USE_ERC32

/*---------------------------------------------------------------------------*/
/* Register addresses                                                        */

#define SPARC_MEC_UART              (0x01f800e0)

/* These must be accessed word-wide to work! */
#define SPARC_MEC_UART_IO( x )      ((cyg_uint32)(x))

#define SPARC_MEC_UART_A_RX         ((volatile cyg_uint32 *)(SPARC_MEC_UART + 0))
#define SPARC_MEC_UART_A_TX         ((volatile cyg_uint32 *)(SPARC_MEC_UART + 0))
#define SPARC_MEC_UART_B_RX         ((volatile cyg_uint32 *)(SPARC_MEC_UART + 4))
#define SPARC_MEC_UART_B_TX         ((volatile cyg_uint32 *)(SPARC_MEC_UART + 4))
#define SPARC_MEC_UART_STATUS       ((volatile cyg_uint32 *)(SPARC_MEC_UART + 8))
#define SPARC_MEC_UART_TXAMASK      (0x00004)
#define SPARC_MEC_UART_TXBMASK      (0x40000)
#define SPARC_MEC_UART_RXAMASK      (0x00001)
#define SPARC_MEC_UART_RXBMASK      (0x10000)

  
/*---------------------------------------------------------------------------*/

#define HAL_DIAG_INIT()

#define HAL_DIAG_WRITE_CHAR(_c_)                                            \
{                                                                           \
    if( 1 || _c_ != '\r' )                                                  \
    {                                                                       \
        while( (SPARC_MEC_UART_TXAMASK & *SPARC_MEC_UART_STATUS) == 0 )     \
            continue;                                                       \
        *SPARC_MEC_UART_A_TX = SPARC_MEC_UART_IO(_c_);                      \
    }                                                                       \
}

#define HAL_DIAG_READ_CHAR(_c_)                                             \
{                                                                           \
    while( (SPARC_MEC_UART_RXAMASK & *SPARC_MEC_UART_STATUS) == 0 )         \
            continue;                                                       \
    _c_ = (char)*SPARC_MEC_UART_A_TX;                                       \
}

#define XHAL_DIAG_WRITE_CHAR(_c_)                                            \
{                                                                           \
    if( _c_ != '\r' )                                                       \
    {                                                                       \
        *SPARC_MEC_UART_A_TX = SPARC_MEC_UART_IO(_c_);                      \
    }                                                                       \
}

#define XHAL_DIAG_READ_CHAR(_c_)                                             \
{                                                                           \
    _c_ = (char)*SPARC_MEC_UART_A_TX;                                       \
}

#else
/*---------------------------------------------------------------------------*/
/* There is no diagnostic output on SPARC simulator                      */

#define HAL_DIAG_INIT()

#define HAL_DIAG_WRITE_CHAR(_c_)

#define HAL_DIAG_READ_CHAR(_c_) (_c_) = 0

#endif

/*---------------------------------------------------------------------------*/
/* end of hal_diag.h                                                         */
#endif /* CYGONCE_HAL_HAL_DIAG_H */
