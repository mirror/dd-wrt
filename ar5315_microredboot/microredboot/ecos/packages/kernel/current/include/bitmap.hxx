#ifndef CYGONCE_KERNEL_BITMAP_HXX
#define CYGONCE_KERNEL_BITMAP_HXX

//==========================================================================
//
//      bitmap.hxx
//
//      Bitmap scheduler class declaration(s)
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
// Date:        1997-09-10
// Purpose:     Define bitmap scheduler implementation
// Description: The classes defined here are used as base classes
//              by the common classes that define schedulers and thread
//              things.
// Usage:       Included according to configuration by
//              <cyg/kernel/sched.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>

// -------------------------------------------------------------------------
// The macro CYGNUM_KERNEL_SCHED_BITMAP_SIZE contains the number of bits 
// that the scheduler bitmap should contain. It is derived from the number
// of threads that the system is allowed to use during configuration.

#ifndef CYGNUM_KERNEL_SCHED_BITMAP_SIZE
#define CYGNUM_KERNEL_SCHED_BITMAP_SIZE 32
#endif

#if CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 8
typedef cyg_ucount8 cyg_sched_bitmap;
#elif CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 16
typedef cyg_ucount16 cyg_sched_bitmap;
#elif CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 32
typedef cyg_ucount32 cyg_sched_bitmap;
#else
#error Bitmaps greater than 32 bits not currently allowed
#endif

// -------------------------------------------------------------------------
// Customize the scheduler

#define CYGIMP_THREAD_PRIORITY  1

#define CYG_THREAD_MIN_PRIORITY (CYGNUM_KERNEL_SCHED_BITMAP_SIZE-1)
#define CYG_THREAD_MAX_PRIORITY 0

// set default scheduling info value for thread constructors.
#define CYG_SCHED_DEFAULT_INFO  CYG_THREAD_MAX_PRIORITY

// -------------------------------------------------------------------------
// This class contains the implementation details of the scheduler, and
// provides a standard API for accessing it.

class Cyg_Scheduler_Implementation
    : public Cyg_Scheduler_Base
{
    friend class Cyg_ThreadQueue_Implementation;
    friend class Cyg_SchedThread_Implementation;
    
    cyg_sched_bitmap    run_queue;

    Cyg_Thread          *thread_table[CYGNUM_KERNEL_SCHED_BITMAP_SIZE];

    
protected:

    Cyg_Scheduler_Implementation();     // Constructor
    
    // The following functions provide the scheduler implementation
    // interface to the Cyg_Scheduler class. These are protected
    // so that only the scheduler can call them.
    
    // choose a new thread
    Cyg_Thread  *schedule();

    // make thread schedulable
    void        add_thread(Cyg_Thread *thread);

    // make thread un-schedulable
    void        rem_thread(Cyg_Thread *thread);

    // register thread with scheduler
    void        register_thread(Cyg_Thread *thread);

    // deregister thread
    void        deregister_thread(Cyg_Thread *thread);
    
    // Test the given priority for uniqueness
    cyg_bool    unique( cyg_priority priority);

public:
    void set_idle_thread( Cyg_Thread *thread, HAL_SMP_CPU_TYPE cpu );
    
};

// -------------------------------------------------------------------------
// Scheduler thread implementation.
// This class provides the implementation of the scheduler specific parts
// of each thread.

class Cyg_SchedThread_Implementation
{
    friend class Cyg_Scheduler_Implementation;
    friend class Cyg_ThreadQueue_Implementation;
    
protected:

    cyg_priority        priority;       // current thread priority

    Cyg_SchedThread_Implementation(CYG_ADDRWORD sched_info);

    void yield();                       // Yield CPU to next thread

    // These are not applicable in a bitmap scheduler; placeholders:
    inline void rotate_queue( cyg_priority pri ) { };
    inline void to_queue_head( void ) { };
};

// -------------------------------------------------------------------------
// Thread queue implementation.
// This class provides the (scheduler specific) implementation of the
// thread queue class.

class Cyg_ThreadQueue_Implementation
{
    cyg_sched_bitmap    wait_queue;

protected:

    // API used by Cyg_ThreadQueue

    Cyg_ThreadQueue_Implementation();   // Constructor
    
                                        // Add thread to queue
    void                enqueue(Cyg_Thread *thread);

                                        // return first thread on queue
    Cyg_Thread          *highpri();

                                        // remove first thread on queue    
    Cyg_Thread          *dequeue();

                                        // remove specified thread from queue    
    void                remove(Cyg_Thread *thread);

                                        // test if queue is empty
    cyg_bool            empty();

};

inline cyg_bool Cyg_ThreadQueue_Implementation::empty()
{
    return wait_queue == 0;
}

// -------------------------------------------------------------------------

#endif // ifndef CYGONCE_KERNEL_BITMAP_HXX
// EOF bitmap.hxx
