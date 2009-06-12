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
// Date:        2001-10-31
// Purpose:     ARM/AAED2000 platform specific registers
// Description: 
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/hal/aaed2000.h>

//-----------------------------------------------------------------------------
// Address space translation macros
// FIXME: The SDRAM translation is only correct for the first block
#define CYGARC_PHYSICAL_ADDRESS(_x_)                                    \
({ CYG_ADDRWORD _p = (CYG_ADDRWORD)(_x_);                               \
 if (_p >= AAED2000_SDRAM_VIRT_BASE &&                                  \
     _p < (AAED2000_SDRAM_VIRT_BASE+AAED2000_SDRAM_SIZE))               \
     _p = (_p & AAED2000_SDRAM_MASK) + AAED2000_SDRAM_PHYS_BASE;        \
 else if (_p >= AAED2000_FLASH_VIRT_BASE &&                             \
          _p < (AAED2000_FLASH_VIRT_BASE+AAED2000_FLASH_SIZE))          \
     _p = (_p & AAED2000_FLASH_MASK) + AAED2000_FLASH_PHYS_BASE;        \
 _p; })

#define CYGARC_VIRTUAL_ADDRESS(_x_)                                     \
({ CYG_ADDRWORD _p = (CYG_ADDRWORD)(_x_);                               \
 if (_p >= AAED2000_SDRAM_PHYS_BASE &&                                  \
     _p < (AAED2000_SDRAM_PHYS_BASE+AAED2000_SDRAM_SIZE))               \
     _p = (_p & AAED2000_SDRAM_MASK) + AAED2000_SDRAM_VIRT_BASE;        \
 else if (_p >= AAED2000_FLASH_PHYS_BASE &&                             \
          _p < (AAED2000_FLASH_PHYS_BASE+AAED2000_FLASH_SIZE))          \
     _p = (_p & AAED2000_FLASH_MASK) + AAED2000_FLASH_VIRT_BASE;        \
 _p; })



//-----------------------------------------------------------------------------
// Macro used for translating from flash address used by RedBoot to that
// used by ARM Boot monitor.
#define _ADDR_REDBOOT_TO_ARM(x) (((CYG_ADDRESS)x & ~0x60000000) | 0x04000000)

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H
