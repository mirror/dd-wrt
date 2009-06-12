#ifndef CYGONCE_DEVS_FLASH_TOSHIBA_TC58XXX_PARTS_INL
#define CYGONCE_DEVS_FLASH_TOSHIBA_TC58XXX_PARTS_INL
//==========================================================================
//
//      flash_tc58xxx_parts.inl
//
//      Toshiba Tc58xxx series part descriptions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Jonathan Larmour
// Copyright (C) 2003, 2004 Gary Thomas
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
// Author(s):    Gary Thomas <gary@mlbassoc.com>
// Contributors: 
// Date:         2003-09-02
// Purpose:      Should be included from the flash_tc58xxx.inl file only.
// Description:  Toshiba Tc58xxx part descriptions
//               Roughly based on Atmel AT49xxxx work by Jani Monoses <jani@iv.ro>
//
// FIXME:        Add configury for selecting bottom/top bootblocks
//####DESCRIPTIONEND####
//
//==========================================================================

// Platform code must define the below
// #define CYGNUM_FLASH_INTERLEAVE      : Number of interleaved devices (in parallel)
// #define CYGNUM_FLASH_SERIES          : Number of devices in series
// #define CYGNUM_FLASH_WIDTH           : Width of devices on platform
// #define CYGNUM_FLASH_BASE            : Address of first device
// And select one of the below device variants

#if defined(CYGHWR_DEVS_FLASH_TOSHIBA_TC58256)
{  // 256Mb (32MB)
    device_id  : FLASHWORD(0x75),
    block_size : 0x4000 * CYGNUM_FLASH_INTERLEAVE,
    page_size  : 0x200 * CYGNUM_FLASH_INTERLEAVE,
    block_count: 2048,
    device_size: 0x2000000 * CYGNUM_FLASH_INTERLEAVE,
    base_mask  : ~(0x2000000 * CYGNUM_FLASH_INTERLEAVE - 1),
},
#endif
#if defined(CYGHWR_DEVS_FLASH_TOSHIBA_TC58DVG02)
{  // 1024Mb (128MB)
    device_id  : FLASHWORD(0x79),
    block_size : 0x4000 * CYGNUM_FLASH_INTERLEAVE,
    page_size  : 0x200 * CYGNUM_FLASH_INTERLEAVE,
    block_count: 8192,
    device_size: 0x8000000 * CYGNUM_FLASH_INTERLEAVE,
    base_mask  : ~(0x8000000 * CYGNUM_FLASH_INTERLEAVE - 1),
},
#endif
#endif // ifndef CYGONCE_DEVS_FLASH_TOSHIBA_TC58XXX_PARTS_INL

// EOF flash_tc58xxx_parts.inl
