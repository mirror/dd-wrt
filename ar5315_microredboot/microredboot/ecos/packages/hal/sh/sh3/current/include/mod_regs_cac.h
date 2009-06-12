//=============================================================================
//
//      mod_regs_cac.h
//
//      CAC (cache) Module register definitions
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
// Date:        2000-10-30
//              
//####DESCRIPTIONEND####
//
//=============================================================================

// Besides the below, cache sizes are defined in the CPU variant module
// headers (mod_77xx.h).

//--------------------------------------------------------------------------
// Cache registers
#define CYGARC_REG_CCR                  0xffffffec

#define CYGARC_REG_CCR_RA               0x00000020
#define CYGARC_REG_CCR_CF               0x00000008
#define CYGARC_REG_CCR_CB               0x00000004
#define CYGARC_REG_CCR_WT               0x00000002
#define CYGARC_REG_CCR_CE               0x00000001


//--------------------------------------------------------------------------
// Address array access
// Address part (base/top/step specified in mod file)
#define CYGARC_REG_CACHE_ADDRESS_ADDRESS 0x00000008 // compare address

// Data part
#define CYGARC_REG_CACHE_ADDRESS_TAG_Mask 0x1ffffc00
#define CYGARC_REG_CACHE_ADDRESS_U 0x00000002 // updated (contains dirty data)
#define CYGARC_REG_CACHE_ADDRESS_V 0x00000001 // valid


