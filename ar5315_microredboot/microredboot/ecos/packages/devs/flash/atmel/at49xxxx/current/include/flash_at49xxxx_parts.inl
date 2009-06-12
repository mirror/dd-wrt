#ifndef CYGONCE_DEVS_FLASH_ATMEL_AT49XXXX_PARTS_INL
#define CYGONCE_DEVS_FLASH_ATMEL_AT49XXXX_PARTS_INL
//==========================================================================
//
//      at49xxxx_parts.inl
//
//      Atmel AT49xxxx series part descriptions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Jonathan Larmour
// Copyright (C) 2003 Gary Thomas
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####

//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour,Jani Monoses <jani@iv.ro>
// Contributors: Jani Monoses <jani@iv.ro>, Cristian Vlasin <cris@iv.ro>, tdrury
// Date:         2003-07-14
// Purpose:      Should be included from the flash_at49xxxx.inl file only.
// Description:  Atmel AT49xxxx part descriptions
//
// FIXME:        Add configury for selecting bottom/top bootblocks
//####DESCRIPTIONEND####
//
//==========================================================================

//
// Note: 'bootblocks' are a set of blocks which are treated by
// the driver as a single larger block.  This simplifies the driver
// so as to only have to deal with single size blocks (even though
// the physical device may differ).  The data structure is laid out as:
//    <address of start of boot block area 1>
//    <size of sub-block 1>
//    <size of sub-block 2>
//    ...
//    <size of sub-block n>
//    <address of start of boot block area 2>
//    <size of sub-block 1>
//    <size of sub-block 2>
//    ...
//    <size of sub-block n>
//    _LAST_BOOTBLOCK
//
// Finally, when specifying a device with bootblocks, the total number
// of blocks should reflect this collapse, i.e. if the device has 15
// full size blocks and 8 blocks which are 1/8 each, then the total
// should be 16 blocks.

#define _LAST_BOOTBLOCK (-1)

// Platform code must define the below
// #define CYGNUM_FLASH_INTERLEAVE      : Number of interleaved devices (in parallel)
// #define CYGNUM_FLASH_SERIES          : Number of devices in series
// #define CYGNUM_FLASH_WIDTH           : Width of devices on platform
// #define CYGNUM_FLASH_BASE            : Address of first device
// And select one of the below device variants

#if defined(CYGPKG_DEVS_FLASH_ATMEL_AT49LV040)
//
// Note: this device is not terribly useful for anything other than a bootstrap device
// as it is only 512KB and has only one erase block.
//
    {   // AT49LV040
        device_id  : FLASHWORD(0x13),
        block_size : 0x80000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 1,
        device_size: 0x80000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x80000 * CYGNUM_FLASH_INTERLEAVE - 1),
        bootblock  : false,
        chip_erase : true
    },
#endif

#if defined(CYGHWR_DEVS_FLASH_ATMEL_AT49LV8011) || \
    defined(CYGHWR_DEVS_FLASH_ATMEL_AT49BV8011)
    {   // AT49BV/LV8011
        // the following ID is true for both 8 and 16 bit CYGNUM_FLASH_WIDTH
        device_id  : FLASHWORD(0xCB),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 16,
        device_size: 0x100000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x100000 * CYGNUM_FLASH_INTERLEAVE - 1),
        bootblock  : true,
        bootblocks : { 0x000000 * CYGNUM_FLASH_INTERLEAVE,
                       0x004000 * CYGNUM_FLASH_INTERLEAVE,
                       0x008000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x008000 * CYGNUM_FLASH_INTERLEAVE,
                       0x004000 * CYGNUM_FLASH_INTERLEAVE,
                       _LAST_BOOTBLOCK
                     }
    },
#endif
#if defined(CYGHWR_DEVS_FLASH_ATMEL_AT49LV8011T) || \
    defined(CYGHWR_DEVS_FLASH_ATMEL_AT49BV8011T)
    {   // AT49BV/LV8011
        // the following ID is true for both 8 and 16 bit CYGNUM_FLASH_WIDTH
        device_id  : FLASHWORD(0x4A),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 16,
        device_size: 0x100000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x100000 * CYGNUM_FLASH_INTERLEAVE - 1),
        bootblock  : true,
        bootblocks : { 0x0E0000 * CYGNUM_FLASH_INTERLEAVE,
                       0x004000 * CYGNUM_FLASH_INTERLEAVE,
                       0x008000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,
                       0x008000 * CYGNUM_FLASH_INTERLEAVE,
                       0x004000 * CYGNUM_FLASH_INTERLEAVE,
                       _LAST_BOOTBLOCK
                     }
    },
#endif
#if defined(CYGHWR_DEVS_FLASH_ATMEL_AT49BV1604A) || \
    defined(CYGHWR_DEVS_FLASH_ATMEL_AT49BV1614A) || \
    defined(CYGHWR_DEVS_FLASH_ATMEL_AT49LV1614A)
    {   // AT49BV/LV8011
        // the following ID is true for both 8 and 16 bit CYGNUM_FLASH_WIDTH
        device_id  : FLASHWORD(0xC0),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 32,
        device_size: 0x200000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x200000 * CYGNUM_FLASH_INTERLEAVE - 1),
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
                       _LAST_BOOTBLOCK
                     }
    },
#endif
#if defined(CYGHWR_DEVS_FLASH_ATMEL_AT29LV200BB)
    {   // AT29LV200BB
        device_id  : FLASHWORD(0x22BF),
        block_size : 0x10000 * CYGNUM_FLASH_INTERLEAVE,
        block_count: 4,
        device_size: 0x40000 * CYGNUM_FLASH_INTERLEAVE,
        base_mask  : ~(0x40000 * CYGNUM_FLASH_INTERLEAVE - 1),
        bootblock  : true,
        bootblocks : { 0x000000 * CYGNUM_FLASH_INTERLEAVE,  // 0x00000..0x03FFF
                       0x004000 * CYGNUM_FLASH_INTERLEAVE,  // 0x04000..0x05FFF
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,  // 0x06000..0x07FFF
                       0x002000 * CYGNUM_FLASH_INTERLEAVE,  // 0x08000..0x0FFFF
                       0x008000 * CYGNUM_FLASH_INTERLEAVE,
                       _LAST_BOOTBLOCK
                     }
    },
#endif

#endif // ifndef CYGONCE_DEVS_FLASH_ATMEL_AT49XXXX_PARTS_INL

// EOF flash_at49xxxx_parts.inl
