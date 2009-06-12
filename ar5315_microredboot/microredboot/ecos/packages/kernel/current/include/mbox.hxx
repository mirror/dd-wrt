#ifndef CYGONCE_KERNEL_MBOX_HXX
#define CYGONCE_KERNEL_MBOX_HXX

//==========================================================================
//
//      mbox.hxx
//
//      Plain (void *) Mbox (Message Box/Mailbox) class declarations
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
// Purpose:     Define Mbox class interfaces
// Description: The classes defined here provide the APIs for mboxes.
// Usage:       #include <cyg/kernel/mbox.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>            // assertion macros
#include <cyg/kernel/thread.hxx>

#ifdef CYGIMP_MBOX_USE_MBOXT_PLAIN
#include <cyg/kernel/mboxt.hxx>
#else
#include <cyg/kernel/mboxt2.hxx>
#endif

// -------------------------------------------------------------------------
// Message/Mail Box.  This class implements a queue of void * items using
// the Cyg_Mbox<Type, QSize> template class.

#ifndef CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE
// default is 10 elements
#define CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE (10)
#endif

// Cyg_Mbox has a fixed size array of (void *)s; one size fits all.
// Because of this, we can simplify the API by returning a NULL for
// "failed" conditions.  Ergo a NULL message is illegal.  BFD.

class Cyg_Mbox
{
private:
#ifdef CYGIMP_MBOX_USE_MBOXT_PLAIN
    Cyg_Mboxt<void *, CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE> m;
#else
    Cyg_Mboxt2<void *, CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE> m;
#endif

public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Mbox();                         // Constructor
    ~Cyg_Mbox();                        // Destructor
        
    void *      get();                  // get an item; wait if none
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    void *      get( cyg_tick_count timeout );
#endif
    void *      tryget();               // just one attempt

    void *      peek_item();            // Get next item to be returned
                                        // without removing it

#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT   // then we support it too
    cyg_bool    put( void *item );      // put an item; wait if full
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    cyg_bool    put( void *item, cyg_tick_count timeout );
#endif
#endif
    cyg_bool    tryput( void *item );   // fails if Q full

    inline
    cyg_count32 peek()                  // Get current count value
    {
        return m.peek();
    }

    inline
    cyg_bool    waiting_to_get()        // Any threads waiting to get?
    {
        return m.waiting_to_get();
    }
    inline
    cyg_bool    waiting_to_put()        // Any threads waiting to put?
    {
        return m.waiting_to_put();
    }
};



// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_MBOX_HXX
// End of mbox.hxx
