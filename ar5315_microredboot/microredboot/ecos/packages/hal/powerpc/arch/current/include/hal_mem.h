#ifndef CYGONCE_HAL_MEM_H
#define CYGONCE_HAL_MEM_H

//=============================================================================
//
//      hal_mem.h
//
//      HAL memory control API
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:nickg, jskov, hmt
// Date:        2000-02-11
// Purpose:     Memory control API
// Description: The macros defined here provide the HAL APIs for handling
//              simple memory management operations.
// Usage:
//              #include <cyg/hal/hal_mem.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

//=============================================================================
// Memory mapping
typedef struct {
    CYG_ADDRESS  virtual_addr;
    CYG_ADDRESS  physical_addr;
    cyg_int32    size;
    cyg_uint8    flags;
} cyg_memdesc_t;

// each variant HAL must provide these functions for mapping/clearing
// (simple BAT/TLB) memory mappings.
externC int
cyg_hal_map_memory (int id,CYG_ADDRESS virt, CYG_ADDRESS phys, 
                    cyg_int32 size, cyg_uint8 flags);
externC void
cyg_hal_clear_MMU (void);

// each platform HAL must provide one of these to describe how memory
// should be mapped/cached, ideally weak aliased so that apps can override:
externC cyg_memdesc_t cyg_hal_mem_map[];

#define CYGARC_MEMDESC_CI       1       // cache inhibit
#define CYGARC_MEMDESC_GUARDED  2       // guarded

// these macros should ease that task, and ease any future extension of the
// structure (physical == virtual addresses):
#define CYGARC_MEMDESC_CACHE( _va_, _sz_ ) \
        { (_va_), (_va_), (_sz_), 0 }

#define CYGARC_MEMDESC_NOCACHE( _va_, _sz_ ) \
        { (_va_), (_va_), (_sz_), CYGARC_MEMDESC_CI }

#define CYGARC_MEMDESC_NOCACHE_PA( _va_, _pa_, _sz_ ) \
        { (_va_), (_pa_), (_sz_), CYGARC_MEMDESC_CI }

#define CYGARC_MEMDESC_CACHEGUARD( _va_, _sz_ ) \
        { (_va_), (_va_), (_sz_), CYGARC_MEMDESC_GUARDED }

#define CYGARC_MEMDESC_NOCACHEGUARD( _va_, _sz_ ) \
        { (_va_), (_va_), (_sz_), CYGARC_MEMDESC_GUARDED|CYGARC_MEMDESC_CI }

#define CYGARC_MEMDESC_TABLE_END      {0, 0, 0, 0}
#define CYGARC_MEMDESC_TABLE          cyg_memdesc_t cyg_hal_mem_map[]
#define CYGARC_MEMDESC_EMPTY_TABLE    { CYGARC_MEMDESC_TABLE_END }

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_MEM_H
// End of hal_mem.h
