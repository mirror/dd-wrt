#ifndef CYGONCE_IO_FLASH_FLASH_DEV_H
#define CYGONCE_IO_FLASH_FLASH_DEV_H
//==========================================================================
//
//      flash_dev.h
//
//      Common flash device driver definitions
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
// Author(s):    hmt
// Contributors: hmt, jskov, Jose Pascual <josepascual@almudi.com>
// Date:         2001-02-22
// Purpose:      Define common flash device driver definitions
// Description:  The flash_data_t type is used for accessing
//               devices at the correct width.
//               The FLASHWORD macro must be used to create constants
//               of suitable width.
//               The FLASH_P2V macro can be used to fix up non-linear
//               mappings of flash blocks (defaults to a linear 
//               implementation).
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#ifdef _FLASH_PRIVATE_
#include <cyg/infra/cyg_type.h>

// ------------------------------------------------------------------------
//
// No mapping on this target - but these casts would be needed if some
// manipulation did occur.  An example of this might be:
// // First 4K page of flash at physical address zero is
// // virtually mapped at address 0xa0000000.
// #define FLASH_P2V(x) ((volatile flash_t *)(((unsigned)(x) < 0x1000) ?
//                            ((unsigned)(x) | 0xa0000000) :
//                            (unsigned)(x)))

#ifndef FLASH_P2V
#define FLASH_P2V( _a_ ) ((volatile flash_t *)((CYG_ADDRWORD)(_a_)))
#endif

// ------------------------------------------------------------------------
//
// This generic code is intended to deal with all shapes and orientations
// of Intel StrataFlash.  Trademarks &c belong to their respective owners.
//
// It therefore needs some trickery to define the constants and accessor
// types that we use to interact with the device or devices.
//
// The assumptions are that
//  o Parallel devices, we write to, with the "opcode" replicated per
//    device
//  o The "opcode" and status returns exist only in the low byte of the
//    device's interface regardless of its width.
//  o Hence opcodes and status are only one byte.
// An exception is the test for succesfully erased data.
//
// ------------------------------------------------------------------------

#if 8 == CYGNUM_FLASH_WIDTH

# if 1 == CYGNUM_FLASH_INTERLEAVE
#  define FLASHWORD( k ) ((flash_data_t)(k)) // To narrow a 16-bit constant
typedef cyg_uint8 flash_data_t;
# elif 2 == CYGNUM_FLASH_INTERLEAVE
// 2 devices to make 16-bit
#  define FLASHWORD( k ) ((k)+((k)<<8))
typedef cyg_uint16 flash_data_t;
# elif 4 == CYGNUM_FLASH_INTERLEAVE
// 4 devices to make 32-bit
#  define FLASHWORD( k ) ((k)+((k)<<8)+((k)<<16)+((k)<<24))
typedef cyg_uint32 flash_data_t;
# elif 8 == CYGNUM_FLASH_INTERLEAVE
// 8 devices to make 64-bit - intermediate requires explicit widening
#  define FLASHWORD32( k ) ((flash_data_t)((k)+((k)<<8)+((k)<<16)+((k)<<24)))
#  define FLASHWORD( k ) (FLASHWORD32( k ) + (FLASHWORD32( k ) << 32));
typedef cyg_uint64 flash_data_t;
# else
#  error How many 8-bit flash devices?
# endif

#elif 16 == CYGNUM_FLASH_WIDTH

# if 1 == CYGNUM_FLASH_INTERLEAVE
#  define FLASHWORD( k ) (k)
typedef cyg_uint16 flash_data_t;
# elif 2 == CYGNUM_FLASH_INTERLEAVE
// 2 devices to make 32-bit
#  define FLASHWORD( k ) ((k)+((k)<<16))
typedef cyg_uint32 flash_data_t;
# elif 4 == CYGNUM_FLASH_INTERLEAVE
// 4 devices to make 64-bit - intermediate requires explicit widening
#  define FLASHWORD32( k ) ((flash_data_t)((k)+((k)<<16)))
#  define FLASHWORD( k ) (FLASHWORD32( k ) + (FLASHWORD32( k ) << 32));
typedef cyg_uint64 flash_data_t;
# else
#  error How many 16-bit flash devices?
# endif

#elif 32 == CYGNUM_FLASH_WIDTH

# if 1 == CYGNUM_FLASH_INTERLEAVE
#  define FLASHWORD( k ) (k)
typedef cyg_uint32 flash_data_t;
# elif 2 == CYGNUM_FLASH_INTERLEAVE
// 2 devices to make 64-bit - intermediate requires explicit widening
#  define FLASHWORD32( k ) ((flash_data_t)(k))
#  define FLASHWORD( k ) (FLASHWORD32( k ) + (FLASHWORD32( k ) << 32));
typedef cyg_uint64 flash_data_t;
# else
#  error How many 32-bit flash devices?
# endif

#else
# error What flash width?
#endif

// Data (not) that we read back:
#if 0 == CYGNUM_FLASH_BLANK
# define FLASH_BlankValue ((flash_data_t)0)
#elif 1 == CYGNUM_FLASH_BLANK
# define FLASH_BlankValue ((flash_data_t)(-1ll))
#else
# error What blank value?
#endif

#endif // _FLASH_PRIVATE_

#endif // CYGONCE_IO_FLASH_FLASH_DEV_H
//----------------------------------------------------------------------------
// end of flash_dev.h
