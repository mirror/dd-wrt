#ifndef CYGONCE_DEVS_FLASH_INTEL_28FXXX_PARTS_INL
#define CYGONCE_DEVS_FLASH_INTEL_28FXXX_PARTS_INL
//==========================================================================
//
//      flash_28fxxx_parts.inl
//
//      Intel 28Fxxx part descriptors
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Author(s):    jskov
// Contributors: jskov, gthomas
// Date:         2001-08-07
// Purpose:
// Description:  Intel 28Fxxx part descriptors
// Usage:        Should be included from the flash_28fxxx.inl file only.
//
// FIXME:        Add configury for selecting bottom/top bootblocks
//####DESCRIPTIONEND####
//
//==========================================================================

#if CYGNUM_FLASH_WIDTH == 8
#ifdef CYGHWR_DEVS_FLASH_SHARP_LH28F016SCT_Z4
    {   // LH28F016SCT_Z4
        device_id  : FLASHWORD(0xA0),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 32,
        device_size: 0x200000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x200000 * CYGNUM_FLASH_INTERLEAVE - 1),
        buffered_w : false,
        locking    : true,
        bootblock  : false,
        banked     : false
    },
#endif

#ifdef CYGHWR_DEVS_FLASH_SHARP_LH28F016SCT_95
    {   // LH28F016SCT_95
        device_id  : FLASHWORD(0xAA),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 32,
        device_size: 0x200000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x200000 * CYGNUM_FLASH_INTERLEAVE - 1),
        buffered_w : false,
        locking    : true,
        bootblock  : false,
        banked     : false
    },
#endif

#else // 16 bit devices

#ifdef CYGHWR_DEVS_FLASH_INTEL_28F320C3
    {   // 28F320C3-T
        device_id  : FLASHWORD(0x88c4),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 64,
        device_size: 0x400000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x400000 * CYGNUM_FLASH_INTERLEAVE - 1),
        locking    : true,
        buffered_w : false,
        bootblock  : true,
        bootblocks : { 0x3f0000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0
                     },
        banked     : false
    },
    {   // 28F320C3-B
        device_id  : FLASHWORD(0x88c5),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 64,
        device_size: 0x400000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x400000 * CYGNUM_FLASH_INTERLEAVE - 1),
        locking    : true,
        buffered_w : false,
        bootblock  : true,
        bootblocks : { 0x000000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0
                     },
        banked     : false
    },
#endif

#ifdef CYGHWR_DEVS_FLASH_INTEL_28F320B3
    {   // 28F320B3-T
        device_id  : FLASHWORD(0x8896),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 64,
        device_size: 0x400000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x400000 * CYGNUM_FLASH_INTERLEAVE - 1),
        locking    : false,
        buffered_w : false,
        bootblock  : true,
        bootblocks : { 0x3f0000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0
                     },
        banked     : false
    },
    {   // 28F320B3-B
        device_id  : FLASHWORD(0x8897),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 64,
        device_size: 0x400000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x400000 * CYGNUM_FLASH_INTERLEAVE - 1),
        locking    : false,
        buffered_w : false,
        bootblock  : true,
        bootblocks : { 0x000000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0
                     },
        banked     : false
    },
#endif


#ifdef CYGHWR_DEVS_FLASH_INTEL_28F320S3
    {   // 28F320S3
        device_id  : FLASHWORD(0x00d4),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 64,
        device_size: 0x400000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x400000 * CYGNUM_FLASH_INTERLEAVE - 1),
        locking    : true,
        buffered_w : false,
        bootblock  : false,
        banked     : false
    },
#endif
	
#ifdef CYGHWR_DEVS_FLASH_INTEL_28F160S5
    {   // 28F160S5
        device_id  : FLASHWORD(0x00d0),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 32,
        device_size: 0x200000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x200000 * CYGNUM_FLASH_INTERLEAVE - 1),
        buffered_w : true,
        locking    : false,
        bootblock  : false,
        banked     : false
    },
#endif

#endif // 16 bit devices


#endif // CYGONCE_DEVS_FLASH_INTEL_28FXXX_PARTS_INL
