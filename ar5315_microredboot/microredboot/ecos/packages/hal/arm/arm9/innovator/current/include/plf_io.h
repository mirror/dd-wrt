#ifndef CYGONCE_HAL_ARM_ARM9_INNOVATOR_PLF_IO_H
#define CYGONCE_HAL_ARM_ARM9_INNOVATOR_PLF_IO_H

/*=============================================================================
//
//      plf_io.h
//
//      Platform specific support (register layout, etc)
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
// Author(s):    Patrick Doyle <wpd@delcomsys.com>
// Contributors: Patrick Doyle <wpd@delcomsys.com>
// Date:         2003-01-15
// Purpose:      Platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_io.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/hal/innovator.h>

#if 0
#define CYGARC_PHYSICAL_ADDRESS(x) (x)
#else
#define HACK_SDRAM_VIRT_BASE 0x00000000
#define HACK_SDRAM_SIZE      0x02000000
#define HACK_SDRAM_MASK      0x01ffffff
#define HACK_SDRAM_PHYS_BASE 0x10000000

#define HACK_FLASH_VIRT_BASE 0x10000000
#define HACK_FLASH_SIZE      0x00400000
#define HACK_FLASH_MASK      0x003fffff
#define HACK_FLASH_PHYS_BASE 0x00000000

#define CYGARC_PHYSICAL_ADDRESS(_x_)                                    \
({ CYG_ADDRWORD _p = (CYG_ADDRWORD)(_x_);                               \
 if (_p >= HACK_SDRAM_VIRT_BASE &&                                      \
     _p < (HACK_SDRAM_VIRT_BASE+HACK_SDRAM_SIZE))                       \
     _p = (_p & HACK_SDRAM_MASK) + HACK_SDRAM_PHYS_BASE;                \
 else if (_p >= HACK_FLASH_VIRT_BASE &&                                 \
          _p < (HACK_FLASH_VIRT_BASE+HACK_FLASH_SIZE))                  \
     _p = (_p & HACK_FLASH_MASK) + HACK_FLASH_PHYS_BASE;                \
 _p; })
#endif
#endif // CYGONCE_HAL_ARM_ARM9_INNOVATOR_PLF_IO_H
// EOF plf_io.h
