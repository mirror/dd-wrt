//=============================================================================
//
//      mod_regs_cac_2.h
//
//      CAC (cache) Module (type 2) register definitions
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
// Date:        2002-04-18
//              
//####DESCRIPTIONEND####
//
//=============================================================================

// Besides the below, cache sizes are defined in the CPU variant module
// headers (mod_704x.h).

//--------------------------------------------------------------------------
// Cache registers
#define CYGARC_REG_CCR                  0xffff8740

#define CYGARC_REG_CCR_CE_DRAM          0x10 // DRAM cache enabled
#define CYGARC_REG_CCR_CE_CS3           0x08 // CS3 cache enabled
#define CYGARC_REG_CCR_CE_CS2           0x04 // CS2 cache enabled
#define CYGARC_REG_CCR_CE_CS1           0x02 // CS1 cache enabled
#define CYGARC_REG_CCR_CE_CS0           0x01 // CS0 cache enabled

#define CYGARC_REG_CCR_CE (CYGARC_REG_CCR_CE_DRAM|CYGARC_REG_CCR_CE_CS3|CYGARC_REG_CCR_CE_CS2|CYGARC_REG_CCR_CE_CS1|CYGARC_REG_CCR_CE_CS0)
