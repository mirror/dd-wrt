#ifndef CYGONCE_MEMALLOC_MEMJOIN_HXX
#define CYGONCE_MEMALLOC_MEMJOIN_HXX

//==========================================================================
//
//      memjoin.hxx
//
//      Pseudo memory pool used to join together other memory pools
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
// Purpose:      Define joined up memory pool class interface
// Description:  Inline class for constructing a pseudo allocator that contains
//               multiple other allocators. It caters solely to the requirements
//               of the malloc implementation.
// Usage:        #include <cyg/memalloc/memjoin.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// CONFIGURATION

#include <pkgconf/memalloc.h>

// INCLUDES

#include <cyg/infra/cyg_type.h>        // types
//#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <cyg/memalloc/common.hxx>     // Common memory allocator infra


// TYPE DEFINITIONS

template <class T>
class Cyg_Mempool_Joined
{
protected:
    struct pooldesc {
        const cyg_uint8 *startaddr;
        const cyg_uint8 *endaddr;
        T *pool;
    };
    struct pooldesc *pools;
    cyg_uint8 poolcount;

    T *
    find_pool_for_ptr( const cyg_uint8 * /* ptr */ );

public:
    // Constructor
    Cyg_Mempool_Joined( cyg_uint8 /* num_heaps */, T * /* heaps */[] );

    // Destructor
    ~Cyg_Mempool_Joined();

    // get some memory, return NULL if none available
    cyg_uint8 *
    try_alloc( cyg_int32 /* size */ );
    
    // resize existing allocation, if oldsize is non-NULL, previous
    // allocation size is placed into it. If previous size not available,
    // it is set to 0. NB previous allocation size may have been rounded up.
    // Occasionally the allocation can be adjusted *backwards* as well as,
    // or instead of forwards, therefore the address of the resized
    // allocation is returned, or NULL if no resizing was possible.
    // Note that this differs from ::realloc() in that no attempt is
    // made to call malloc() if resizing is not possible - that is left
    // to higher layers. The data is copied from old to new though.
    // The effects of alloc_ptr==NULL or newsize==0 are undefined
    cyg_uint8 *
    resize_alloc( cyg_uint8 * /* alloc_ptr */, cyg_int32 /* newsize */,
                  cyg_int32 * /* oldsize */ =NULL );

    // free the memory back to the pool
    // returns true on success
    cyg_bool
    free( cyg_uint8 * /* ptr */, cyg_int32 /* size */ =0 );

    // Get memory pool status
    // flags is a bitmask of requested fields to fill in. The flags are
    // defined in common.hxx
    void
    get_status( cyg_mempool_status_flag_t /* flags */,
                Cyg_Mempool_Status & /* status */ );

};

#include <cyg/memalloc/memjoin.inl>

#endif // ifndef CYGONCE_MEMALLOC_MEMJOIN_HXX
// EOF memjoin.hxx
