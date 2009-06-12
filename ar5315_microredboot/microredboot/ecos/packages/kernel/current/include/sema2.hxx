#ifndef CYGONCE_KERNEL_SEMA2_HXX
#define CYGONCE_KERNEL_SEMA2_HXX

//==========================================================================
//
//      sema2.hxx
//
//      Semaphore class declarations
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-09-09
// Purpose:     Define Semaphore class interfaces
// Description: The classes defined here provide the APIs for binary
//              and counting semaphores.
// Usage:       #include <cyg/kernel/sema2.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/kernel/thread.hxx>

#if 0
// THERE IS NO BINARY-SEMAPHORE-2

// -------------------------------------------------------------------------
// Binary semaphore. This has only two states: posted and not-posted.

class Cyg_Binary_Semaphore2
{
    cyg_bool            state;          // The binary semaphore state

    Cyg_ThreadQueue     queue;          // Queue of waiting threads

public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Binary_Semaphore2(              // Constructor
        cyg_bool init_state = false     // Initial state value
        );

    ~Cyg_Binary_Semaphore2();           // Destructor
        
    cyg_bool    wait();                 // Wait until state == true

    cyg_bool    trywait();              // Set state false if possible
        
    void        post();                 // Increment count

    cyg_bool    posted();               // Get current state
    
};
#endif

// -------------------------------------------------------------------------
// Counting semaphore. This implements the usual counter based semaphore.

class Cyg_Counting_Semaphore2
{
    cyg_count32         count;          // The semaphore count

    Cyg_ThreadQueue     queue;          // Queue of waiting threads

public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Counting_Semaphore2(            // Constructor
        cyg_count32 init_count = 0      // Initial count value
        );

    ~Cyg_Counting_Semaphore2();         // Destructor
        
    cyg_bool    wait();                 // Wait until decrement

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    cyg_bool    wait( cyg_tick_count abs_timeout );
#endif                                  // Wait until decrement or timeout

    cyg_bool    trywait();              // Try to decrement
        
    void        post();                 // Increment count

    cyg_count32 peek() const;           // Get current count value
    
    inline
    cyg_bool    waiting()               // Is anyone waiting?
    {
        return !queue.empty();
    }

};

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_SEMA2_HXX
// EOF sema2.hxx
