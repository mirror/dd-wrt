#ifndef CYGONCE_HAL_PLF_REGS_H
#define CYGONCE_HAL_PLF_REGS_H

//==========================================================================
//
//      plf_regs.h
//
//      TAMS MOAB PowerPC 405GP platform CPU definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002, 2003 Gary Thomas
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
// Author(s):    Gary Thomas <gary@mlbassoc.com>
// Contributors: 
// Date:         2003-09-02
// Purpose:      
// Description:  Possibly override any platform assumptions
//
// Usage:        Included via the variant+architecture register headers:
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Special memory regions
#define _MOAB_NAND  0xC0000000  // Mapped window to main NAND device
#define _MOAB_OCM   0xD0000000  // Mapped window to on-chip memory (4KB only)

// GPIO Layout
#define _MOAB_CLE   0x00010000  // GPIO 15 - asserts CLE to NAND device
#define _MOAB_ALE   0x00020000  // GPIO 14 - asserts ALE to NAND device
#define _MOAB_CE    0x00004000  // GPIO 17 - asserts CE to NAND device (active low)
#define _MOAB_RDY   0x00000200  // GPIO 22 - NAND RDY

#endif // CYGONCE_HAL_PLF_REGS_H
