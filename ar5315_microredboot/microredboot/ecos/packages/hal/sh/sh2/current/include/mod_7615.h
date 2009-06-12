#ifndef CYGONCE_HAL_MOD_76xx_H
#define CYGONCE_HAL_MOD_76xx_H

//=============================================================================
//
//      mod_7615.h
//
//      List modules available on CPU
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
// Date:        2002-01-09
// Purpose:     Define modules (and versions) available on this CPU.
// Usage:       Included from <cyg/hal/sh_regs.h>
//
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//-----------------------------------------------------------------------------
// Modules provided by the CPU

#define CYGARC_SH_MOD_BSC  1
#define CYGARC_SH_MOD_CAC  1
#define CYGARC_SH_MOD_BCN  2
#define CYGARC_SH_MOD_CPG  1
#define CYGARC_SH_MOD_INTC 1
#define CYGARC_SH_MOD_WDT  1
#define CYGARC_SH_MOD_IRDA 1
#define CYGARC_SH_MOD_PFC  1
#define CYGARC_SH_MOD_SCIF 1
//#define CYGARC_SH_MOD_UBC  2
#define CYGARC_SH_MOD_FRT  1
#define CYGARC_SH_MOD_DMAC 1
#define CYGARC_SH_MOD_PCM  1
#define CYGARC_SH_MOD_ETH  1

//-----------------------------------------------------------------------------
// Extra details for Cache Module (CAC)

// Cache dimenions - one unified cache
#define CYGARC_SH_MOD_CAC_SIZE        4096  // Size of cache in bytes
#define CYGARC_SH_MOD_CAC_LINE_SIZE   16    // Size of a cache line
#define CYGARC_SH_MOD_CAC_WAYS        4     // Associativity of the cache

//-----------------------------------------------------------------------------
// Extra details for interrupt handling
#undef CYGARC_SH_SOFTWARE_IP_UPDATE

#endif // CYGONCE_HAL_MOD_76xx_H
