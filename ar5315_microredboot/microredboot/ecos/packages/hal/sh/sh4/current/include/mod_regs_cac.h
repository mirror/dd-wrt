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
// Copyright (C) 2003 Nick Garnett 
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
// Contributors:jskov, nickg
// Date:        2000-10-30
//              
//####DESCRIPTIONEND####
//
//=============================================================================

// Besides the below, cache sizes are defined in the CPU variant module
// headers (mod_77xx.h).

//--------------------------------------------------------------------------
// Cache registers
#define CYGARC_REG_CCR                  0xFF00001C

#define CYGARC_REG_CCR_IIX              0x00008000  // IC index enable
#define CYGARC_REG_CCR_ICI              0x00000800  // IC invalidation
#define CYGARC_REG_CCR_ICE              0x00000100  // IC Enable
#define CYGARC_REG_CCR_OIX              0x00000080  // OC index enable
#define CYGARC_REG_CCR_ORA              0x00000020  // OC RAM enable
#define CYGARC_REG_CCR_OCI              0x00000008  // OC Invalidation
#define CYGARC_REG_CCR_CB               0x00000004  // Copy-Back enable
#define CYGARC_REG_CCR_WT               0x00000002  // Write Through enable
#define CYGARC_REG_CCR_OCE              0x00000001  // OC Enable

#ifdef CYGPKG_HAL_SH_202
#define CYGARC_REG_CCR_EMODE            0x80000000  // Enhanced mode
#else
#define CYGARC_REG_CCR_EMODE            0x00000000  // No enhanced mode
#endif

#define CYGARC_REG_CCR_CE               0x00000101  // IC, OC Enable
#define CYGARC_REG_CCR_CF               0x00000808  // IC, OC Invalidation

