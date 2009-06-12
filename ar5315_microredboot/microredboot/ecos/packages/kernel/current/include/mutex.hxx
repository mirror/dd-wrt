#ifndef CYGONCE_KERNEL_MUTEX_HXX
#define CYGONCE_KERNEL_MUTEX_HXX

//==========================================================================
//
//      mutex.hxx
//
//      Mutex class declarations
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
// Purpose:     Define Mutex class interfaces
// Description: The classes defined here provide the APIs for mutexes
//              and condition variables.
// Usage:       #include <cyg/kernel/mutex.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <cyg/kernel/thread.hxx>

// -------------------------------------------------------------------------
// Mutex.

class Cyg_Mutex
{
    friend class Cyg_Condition_Variable;
    
    cyg_atomic          locked;         // true if locked. This may seem
                                        // redundant due to "owner" below,
                                        // but is intentionally present for
                                        // future SMP support.

    Cyg_Thread          *owner;         // Current locking thread

    Cyg_ThreadQueue     queue;          // Queue of waiting threads

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC

public:    
    enum cyg_protcol
    {
        NONE = 0,                       // no inversion protocol
        INHERIT,                        // priority inheritance protocol
        CEILING                         // priority ceiling protocol
    };

private:    
    cyg_protcol protocol;               // this mutex's protocol
    
#endif    
    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING

private:    
    cyg_priority        ceiling;        // mutex priority ceiling
    
#endif
    
public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Mutex();                        // Create in unlocked state

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC
    
    Cyg_Mutex( cyg_protcol protocol );  // Create with defined protocol

#endif
    
    ~Cyg_Mutex();                       // Destructor
        
    cyg_bool    lock();                 // lock and/or wait

    cyg_bool    trylock();              // try to lock and return success

    void        unlock();               // unlock

    void        release();              // release all waiting threads

    // Get the current owning thread
    inline Cyg_Thread *get_owner() { return owner; }
    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING

    // set ceiling priority for priority ceiling protocol
    void        set_ceiling( cyg_priority priority );

    cyg_priority get_ceiling(void) { return ceiling; };
    
#endif

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC

    // set inversion protocol
    void        set_protocol( cyg_protcol new_protocol );
#endif
    
};

// -------------------------------------------------------------------------
// Condition variable.

class Cyg_Condition_Variable
{
    Cyg_Mutex           *mutex;         // Associated mutex

    Cyg_ThreadQueue     queue;          // Queue of waiting threads

    // Private internal implementation function for wait operations
    cyg_bool wait_inner( Cyg_Mutex *mutex );

#ifdef CYGMFN_KERNEL_SYNCH_CONDVAR_TIMED_WAIT

    // Private internal implementation function for timed wait operations
    cyg_bool wait_inner( Cyg_Mutex *mutex, cyg_tick_count timeout );

#endif
    
public:

    CYGDBG_DEFINE_CHECK_THIS

    Cyg_Condition_Variable();           // simple constructor
    
    Cyg_Condition_Variable(
        Cyg_Mutex &mutex                // linked mutex
        );

    ~Cyg_Condition_Variable();          // Destructor
        

    void     signal();                  // Set cond true, wake one thread

    void     broadcast();               // Set cond true, wake all threads

    // Wait for condition to be true
    inline cyg_bool wait() { return wait_inner( mutex ); }
    
#ifdef CYGMFN_KERNEL_SYNCH_CONDVAR_TIMED_WAIT

    // Wait until a signal or timeout expiry
    inline cyg_bool wait( cyg_tick_count timeout )
    { return wait_inner( mutex, timeout ); }

#endif

#ifdef CYGMFN_KERNEL_SYNCH_CONDVAR_WAIT_MUTEX

    // Wait for condition to be true using the supplied mutex    
    inline cyg_bool wait( Cyg_Mutex &mx ) { return wait_inner( &mx ); }
    

#ifdef CYGMFN_KERNEL_SYNCH_CONDVAR_TIMED_WAIT

    // Wait until a signal or timeout expiry, using the supplied mutex
    inline cyg_bool wait( Cyg_Mutex &mx, cyg_tick_count timeout )
    { return wait_inner( &mx, timeout ); }        
    
#endif    
#endif    

    // Return a pointer to this variables thread queue. Used mainly
    // for testing whether a thread is on the queue for a particular
    // cv.
    inline Cyg_ThreadQueue *get_queue() { return &queue; };
    
};


// -------------------------------------------------------------------------

#endif // ifndef CYGONCE_KERNEL_MUTEX_HXX
// EOF mutex.hxx
