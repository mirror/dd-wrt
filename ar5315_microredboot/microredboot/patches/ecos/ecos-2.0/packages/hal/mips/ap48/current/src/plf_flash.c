//==========================================================================
//
//      plf_flash.c
//
//      Flash programming for MX Flash device on AP48 reference board
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): Atheros Communications, Inc.
// Contributors:
// Date:         2003-12-02
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

//--------------------------------------------------------------------------
#include <pkgconf/hal_mips_ap48.h>

// The AP48 has an MX29LV160AT flash part, which is AMD-compatible.
#define CYGNUM_FLASH_INTERLEAVE (1)
#define CYGNUM_FLASH_SERIES     (1)
#define CYGNUM_FLASH_WIDTH      (8)
#define CYGNUM_FLASH_16AS8      1
#define CYGNUM_FLASH_BASE       (0xbe000000)

//--------------------------------------------------------------------------
// Platform specific extras

//--------------------------------------------------------------------------
// Now include the driver code.
#ifdef CYGINT_DEVS_FLASH_AMD_AM29XXXXX_REQUIRED
#include "cyg/io/flash_am29xxxxx.inl"
#endif
// ------------------------------------------------------------------------
// EOF plf_flash.c
