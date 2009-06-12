#ifndef CYGONCE_HAL_PLF_Z8530_H
#define CYGONCE_HAL_PLF_Z8530_H

//=============================================================================
//
//      plf_z8530.h
//
//      Platform header for Z8530 support.
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
// Author(s):   nickg
// Contributors:nickg
// Date:        1999-05-21
// Purpose:     Platform HAL Z8530 support.
// Usage:       #include <cyg/hal/plf_z8530.h>
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

/*---------------------------------------------------------------------------*/
/* Zilog Z8530 access macros.                                                */

#if defined(CYGPKG_HAL_MIPS_LSBFIRST)
# define DUART_BASE      0xc1000000
#elif defined(CYGPKG_HAL_MIPS_MSBFIRST)
# define DUART_BASE      0xc1000003
#else
#error MIPS endianness not defined by configuration
#endif

// Address offsets for DUART channels
#define DUART_A         4
#define DUART_B         0

/* Require Delay between Zilog chip (Z8530 and Z8536) register access */

//#define ZDEL    66              /* 66 Instructions for 66Mhz */
#define ZDEL    200              /* 200 Instructions for 133Mhz */

#define DELAY_ZACC\
	{ register int N = ZDEL; while (--N > 0); }


#define HAL_DUART_READ_CR( _chan_, _reg_, _val_)        \
CYG_MACRO_START                                         \
    if( (_reg_) != 0 )                                  \
    {                                                   \
        DELAY_ZACC;                                     \
        HAL_WRITE_UINT8( DUART_BASE+(_chan_), _reg_);   \
    }                                                   \
    DELAY_ZACC;                                         \
    HAL_READ_UINT8( DUART_BASE+(_chan_), _val_ );       \
CYG_MACRO_END

#define HAL_DUART_WRITE_CR( _chan_, _reg_, _val_ )      \
CYG_MACRO_START                                         \
    if( (_reg_) != 0 )                                  \
    {                                                   \
        DELAY_ZACC;                                     \
        HAL_WRITE_UINT8( DUART_BASE+(_chan_), _reg_);   \
    }                                                   \
    DELAY_ZACC;                                         \
    HAL_WRITE_UINT8( DUART_BASE+(_chan_), _val_ );      \
CYG_MACRO_END

#define HAL_DUART_WRITE_TR( _chan_, _val_ )             \
CYG_MACRO_START                                         \
    DELAY_ZACC;                                         \
    HAL_WRITE_UINT8( DUART_BASE+(_chan_)+8, _val_ );    \
CYG_MACRO_END

#define HAL_DUART_READ_RR( _chan_, _val_ )              \
CYG_MACRO_START                                         \
    DELAY_ZACC;                                         \
    HAL_READ_UINT8( DUART_BASE+(_chan_)+8, _val_ );     \
CYG_MACRO_END

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_PLF_Z8530_H
// End of plf_z8530.h
