#ifndef CYGONCE_MEMALLOC_KAPIDATA_H
#define CYGONCE_MEMALLOC_KAPIDATA_H

/*==========================================================================
//
//      kapidata.h
//
//      Memory allocator portion of kernel C API
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-06-12
// Purpose:      Memory allocator data for kernel C API
// Description:  This is intentionally only to be included via
//               <cyg/kernel/kapi.h>
// Usage:        This file should not be used directly - instead it should
//               be used via <cyg/kernel/kapi.h>
//              
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/memalloc.h>

/*---------------------------------------------------------------------------*/

/* This corresponds to the extra fields provided by the mempoolt template
   not the actual size of the template in any given instance. */
typedef struct cyg_mempoolt {
    cyg_threadqueue queue;
} cyg_mempoolt;


struct cyg_mempool_var_memdq {
    struct cyg_mempool_var_memdq *prev, *next;
    cyg_int32 size;
};

struct cyg_mempool_var {
    struct cyg_mempool_var_memdq head;
    cyg_uint8  *obase;
    cyg_int32  osize;
    cyg_uint8  *bottom;
    cyg_uint8  *top;
    cyg_int32  alignment;
    cyg_int32  freemem;
    cyg_mempoolt mempoolt;
};

struct cyg_mempool_fix {
    cyg_uint32 *bitmap;
    cyg_int32 maptop;
    cyg_uint8  *mempool;
    cyg_int32 numblocks;
    cyg_int32 freeblocks;
    cyg_int32 blocksize;
    cyg_int32 firstfree;
    cyg_uint8  *top;
    cyg_mempoolt mempoolt;
};

#endif /* ifndef CYGONCE_MEMALLOC_KAPIDATA_H */
/* EOF kapidata.h */
