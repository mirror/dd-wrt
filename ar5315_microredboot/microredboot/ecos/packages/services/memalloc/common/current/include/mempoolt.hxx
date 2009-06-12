#ifndef CYGONCE_KERNEL_MEMPOOLT_HXX
#define CYGONCE_KERNEL_MEMPOOLT_HXX

//==========================================================================
//
//      mempoolt.hxx
//
//      Mempoolt (Memory pool template) class declarations
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
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-02-10
// Purpose:     Define Mempoolt class interface

// Description: The class defined here provides the APIs for thread-safe,
//              kernel-savvy memory managers; make a class with the
//              underlying allocator as the template parameter.
// Usage:       #include <cyg/kernel/mempoolt.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/kernel/thread.hxx>

template <class T>
class Cyg_Mempoolt
{
private:
    T pool;                             // underlying memory manager
    Cyg_ThreadQueue queue;              // queue of waiting threads

public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Mempoolt(
        cyg_uint8 *base,
        cyg_int32 size,
        CYG_ADDRWORD arg_thru );        // Constructor
    ~Cyg_Mempoolt();                    // Destructor
        
    // get some memory; wait if none available; return NULL if failed
    // due to interrupt
    cyg_uint8 *alloc( cyg_int32 size );
    
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    // get some memory with a timeout; return NULL if failed
    // due to interrupt or timeout
    cyg_uint8 *alloc( cyg_int32 size, cyg_tick_count abs_timeout );
#endif

    // get some memory, return NULL if none available
    cyg_uint8 *try_alloc( cyg_int32 size );
    
    // free the memory back to the pool
    cyg_bool free( cyg_uint8 *p, cyg_int32 size );

    // if applicable: return -1 if not fixed size
    cyg_int32 get_blocksize();

    // is anyone waiting for memory?
    cyg_bool waiting() { return ! queue.empty(); }

    // these two are obvious and generic
    cyg_int32 get_totalmem();
    cyg_int32 get_freemem();

    // get information about the construction parameters for external
    // freeing after the destruction of the holding object.
    void get_arena(
        cyg_uint8 * &base,
        cyg_int32 &size,
        CYG_ADDRWORD &arg_thru );

    // Return the size of the memory allocation (previously returned 
    // by alloc() or try_alloc() ) at ptr. Returns -1 if not found
    cyg_int32
    get_allocation_size( cyg_uint8 * /* ptr */ );
};

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_MEMPOOLT_HXX
// EOF mempoolt.hxx
