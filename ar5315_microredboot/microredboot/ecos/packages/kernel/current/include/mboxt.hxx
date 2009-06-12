#ifndef CYGONCE_KERNEL_MBOXT_HXX
#define CYGONCE_KERNEL_MBOXT_HXX

//==========================================================================
//
//      mboxt.hxx
//
//      Mboxt (Message Box/Mailbox) class declarations
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
// Purpose:     Define Mboxt class interfaces
// Description: The classes defined here provide the APIs for mboxtes.
// Usage:       #include <cyg/kernel/mboxt.hxx>
//              #include <cyg/kernel/mboxt.inl>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>            // assertion macros
#include <cyg/kernel/thread.hxx>

// -------------------------------------------------------------------------
// Message/Mail Box.  This template implements a queue of T's.
// Implemented as a template for maximal flexibility; one would hope
// that only one, with T==(void *) and the same number of them,
// is ever used without very good reason.

// Cyg_Mboxt has a fixed size array of T's; one size fits all.

template <class T, cyg_count32 QUEUE_SIZE>
class Cyg_Mboxt
{
private:
    cyg_count32         base;           // index of first used slot
    cyg_count32         count;          // count of used slots
    Cyg_ThreadQueue     get_threadq;    // Queue of waiting threads
#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    Cyg_ThreadQueue     put_threadq;    // Queue of waiting threads
#endif
    static const
    cyg_count32         size = QUEUE_SIZE;
    T                   itemqueue[ QUEUE_SIZE ];
                                        // queue of items
    // private utility functions
    // wake up a thread from some queue
    inline void         wakeup_waiter( Cyg_ThreadQueue &q );

public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Mboxt();                        // Constructor
    ~Cyg_Mboxt();                       // Destructor
        
    cyg_bool    get( T &ritem );        // get an item; wait if none
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    cyg_bool    get( T &ritem, cyg_tick_count abs_timeout );
#endif
    cyg_bool    tryget( T &ritem );     // just one attempt

    cyg_bool    peek_item( T &ritem );  // get next item without
                                        // removing it

#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    cyg_bool    put( const T item );    // put an item; wait if full
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    cyg_bool    put( const T item, cyg_tick_count abs_timeout );
#endif
#endif
    cyg_bool    tryput( const T item ); // fails if Q full

    inline
    cyg_count32 peek()                  // Get current count value
    {
        return count;
    }

    inline
    cyg_bool    waiting_to_get()        // Any threads waiting?
    {
        return ! get_threadq.empty();
    }

    inline
    cyg_bool    waiting_to_put()        // Any threads waiting?
    {
#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
        return ! put_threadq.empty();
#else
        return false;
#endif
    }
};

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_MBOXT_HXX
// EOF mboxt.hxx
