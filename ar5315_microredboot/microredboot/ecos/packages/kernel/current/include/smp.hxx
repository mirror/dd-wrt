#ifndef CYGONCE_KERNEL_SMP_HXX
#define CYGONCE_KERNEL_SMP_HXX

//==========================================================================
//
//      smp.hxx
//
//      SMP kernel support
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
// Contributors:nickg
// Date:        2001-02-10
// Purpose:     Kernel SMP support
// Description: If SMP support is configured into the kernel, then this file
//              translates HAL defined macros into C and C++ classes and methods
//              that can be called from the rest of the kernel. If SMP is not
//              configured in, then the same classes and methods are defined here
//              to operate correctly in a single CPU configuration.
//              
// Usage:       #include <cyg/kernel/smp.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_intr.h>           // HAL_DISABLE_INTERRUPTS() etc.

#include <cyg/kernel/instrmnt.h>

//==========================================================================

#if defined(CYGPKG_KERNEL_SMP_SUPPORT) && (CYGPKG_HAL_SMP_SUPPORT)

//==========================================================================
// SMP support is included

#define CYG_KERNEL_SMP_ENABLED

// -------------------------------------------------------------------------
// Get HAL support

#include <cyg/hal/hal_smp.h>

// -------------------------------------------------------------------------
// Defined values
// These all just map straight through to the HAL.

#define CYGNUM_KERNEL_CPU_MAX           HAL_SMP_CPU_MAX

#define CYG_KERNEL_CPU_COUNT()          HAL_SMP_CPU_COUNT()

#define CYG_KERNEL_CPU_THIS()           HAL_SMP_CPU_THIS()

#define CYG_KERNEL_CPU_NONE             HAL_SMP_CPU_NONE

// -------------------------------------------------------------------------
// CPU control

#define CYG_KERNEL_CPU_START( __cpu ) HAL_SMP_CPU_START( __cpu )

#define CYG_KERNEL_CPU_RESCHEDULE_INTERRUPT( __cpu, __wait ) \
        HAL_SMP_CPU_RESCHEDULE_INTERRUPT( __cpu, __wait )

#define CYG_KERNEL_CPU_TIMESLICE_INTERRUPT( __cpu, __wait ) \
        HAL_SMP_CPU_TIMESLICE_INTERRUPT( __cpu, __wait )

// -------------------------------------------------------------------------
// Scheduler lock default implementation.

// This implementation should serve for most targets. However, some
// targets may have hardware or other features that make simple
// spinlocks impossible, or allow us to implement the scheduler lock
// in a more efficient manner. If that is the case then the HAL will
// implement these macros itself.

#ifndef HAL_SMP_SCHEDLOCK_DATA_TYPE

#define HAL_SMP_SCHEDLOCK_DATA_TYPE struct hal_smp_schedlock_data_type

struct hal_smp_schedlock_data_type {
    HAL_SPINLOCK_TYPE           spinlock;
    volatile HAL_SMP_CPU_TYPE   holder;
};

#define HAL_SMP_SCHEDLOCK_INIT( __lock, __data )        \
CYG_MACRO_START                                         \
{                                                       \
    __lock = 1;                                         \
    HAL_SPINLOCK_CLEAR(__data.spinlock);                \
    HAL_SPINLOCK_SPIN(__data.spinlock);                 \
    __data.holder = HAL_SMP_CPU_THIS();                 \
}                                                       \
CYG_MACRO_END


#define HAL_SMP_SCHEDLOCK_INC( __lock, __data )                 \
CYG_MACRO_START                                                 \
{                                                               \
    CYG_INTERRUPT_STATE __state;                                \
    HAL_DISABLE_INTERRUPTS(__state);                            \
    if( __data.holder == HAL_SMP_CPU_THIS() )                   \
        __lock++;                                               \
    else                                                        \
    {                                                           \
        CYG_INSTRUMENT_SMP(LOCK_WAIT,CYG_KERNEL_CPU_THIS(),0);  \
        HAL_SPINLOCK_SPIN(__data.spinlock);                     \
        __data.holder = HAL_SMP_CPU_THIS();                     \
        __lock++;                                               \
        CYG_INSTRUMENT_SMP(LOCK_GOT,CYG_KERNEL_CPU_THIS(),0);   \
    }                                                           \
    HAL_RESTORE_INTERRUPTS(__state);                            \
}                                                               \
CYG_MACRO_END

#define HAL_SMP_SCHEDLOCK_ZERO( __lock, __data )                                                 \
CYG_MACRO_START                                                                                  \
{                                                                                                \
    CYG_INTERRUPT_STATE __state;                                                                 \
    HAL_DISABLE_INTERRUPTS(__state);                                                             \
    CYG_ASSERT( __data.holder == HAL_SMP_CPU_THIS(), "Zeroing schedlock not owned by me!");      \
    __lock = 0;                                                                                  \
    __data.holder = HAL_SMP_CPU_NONE;                                                            \
    HAL_SPINLOCK_CLEAR(__data.spinlock);                                                         \
    HAL_RESTORE_INTERRUPTS(__state);                                                             \
}                                                                                                \
CYG_MACRO_END

#define HAL_SMP_SCHEDLOCK_SET( __lock, __data, __new )                                          \
CYG_MACRO_START                                                                                 \
{                                                                                               \
    CYG_ASSERT( __data.holder == HAL_SMP_CPU_THIS(), "Setting schedlock not owned by me!");     \
    __lock = __new;                                                                             \
}                                                                                               \
CYG_MACRO_END

#endif

// -------------------------------------------------------------------------
// SpinLock class
// This class supplies a C++ wrapper for the HAL spinlock API.

#ifdef __cplusplus

#ifdef HAL_SPINLOCK_SPIN

class Cyg_SpinLock
{
    HAL_SPINLOCK_TYPE   lock;

public:

    // Constructor, initialize the lock to clear
    Cyg_SpinLock() { lock = HAL_SPINLOCK_INIT_CLEAR; };

    ~Cyg_SpinLock()
    {
//        CYG_ASSERT( !test(), "spinlock still claimed");
    };
    
    // Spin on the lock.
    void spin()
    {
        HAL_SPINLOCK_SPIN(lock);
    };

    // Clear the lock.
    void clear()
    {
        HAL_SPINLOCK_CLEAR(lock);
    };

    // Try to claim the lock. Return true if successful, false if not.
    cyg_bool trylock()
    {
        cyg_bool testval;
        HAL_SPINLOCK_TRY(lock,testval);
        return testval;
    };

    // Test the current value of the lock
    cyg_bool test()
    {
        cyg_bool testval;
        HAL_SPINLOCK_TEST(lock, testval);
        return testval;
    };


    // The following two member functions are only necessary if the
    // spinlock is to be used in an ISR. 
    
    // Claim the spinlock, but also mask this CPU's interrupts while
    // we have it.
    void spin_intsave(CYG_INTERRUPT_STATE *state)
    {
        CYG_INTERRUPT_STATE s;
        HAL_DISABLE_INTERRUPTS(s);
        *state = s;
        spin();
    };

    // Clear the lock, and restore the interrupt state saved in
    // spin_intsave().
    void clear_intsave(CYG_INTERRUPT_STATE state)
    {
        clear();
        HAL_RESTORE_INTERRUPTS(state);
    };
};

#endif

// -------------------------------------------------------------------------
// Scheduler lock class
// This uses the scheduler lock API defined by the HAL, or the defaults
// defined above.

class Cyg_Scheduler_SchedLock
{
    static volatile cyg_ucount32 sched_lock         // lock counter
                    CYGBLD_ATTRIB_ASM_ALIAS( cyg_scheduler_sched_lock )
                    CYGBLD_ANNOTATE_VARIABLE_SCHED
                    ;
    
    static HAL_SMP_SCHEDLOCK_DATA_TYPE lock_data
                                       CYGBLD_ANNOTATE_VARIABLE_SCHED;
    
protected:

    Cyg_Scheduler_SchedLock()
    {
        HAL_SMP_SCHEDLOCK_INIT( sched_lock, lock_data );
    };
    
    // Increment the scheduler lock. If this takes the lock from zero
    // to one then this code must also do whatever is necessary to
    // serialize CPUs through the scheduler.
    static void inc_sched_lock()
    {
        CYG_INSTRUMENT_SMP(LOCK_INC,CYG_KERNEL_CPU_THIS(),0);
        HAL_SMP_SCHEDLOCK_INC( sched_lock, lock_data );
    };

    // Zero the scheduler lock. This will release the CPU serializing
    // lock and allow another CPU in.
    static void zero_sched_lock()
    {
        CYG_INSTRUMENT_SMP(LOCK_ZERO,CYG_KERNEL_CPU_THIS(),0);
        CYG_ASSERT( sched_lock != 0, "Scheduler lock already zero");
        HAL_SMP_SCHEDLOCK_ZERO( sched_lock, lock_data );
    };
    
    // Set the scheduler lock to a non-zero value. Both the scheduler
    // lock and the new value must be non-zero.
    static void set_sched_lock(cyg_uint32 new_lock)
    {
        CYG_INSTRUMENT_SMP(LOCK_SET,CYG_KERNEL_CPU_THIS(),new_lock);        
        CYG_ASSERT( new_lock > 0, "New scheduler lock value == 0");
        CYG_ASSERT( sched_lock > 0, "Scheduler lock == 0");
        HAL_SMP_SCHEDLOCK_SET( sched_lock, lock_data, new_lock );        
    };

    static cyg_ucount32 get_sched_lock()
    {
        return sched_lock;
    };
};

#define CYGIMP_KERNEL_SCHED_LOCK_DEFINITIONS                    \
volatile cyg_ucount32 Cyg_Scheduler_SchedLock::sched_lock = 1;  \
HAL_SMP_SCHEDLOCK_DATA_TYPE Cyg_Scheduler_SchedLock::lock_data;

#endif // __cplusplus

// -------------------------------------------------------------------------

#else // defined(CYGSEM_KERNEL_SMP_SUPPORT) && (CYGSEM_HAL_SMP_SUPPORT)

//==========================================================================
// SMP support is NOT included.

#undef CYG_KERNEL_SMP_ENABLED

// -------------------------------------------------------------------------
// Defined values
// Supply a set of values that describe a single CPU system.

#ifndef HAL_SMP_CPU_TYPE
#define HAL_SMP_CPU_TYPE                cyg_uint32
#endif

#define CYGNUM_KERNEL_CPU_MAX           1

#define CYG_KERNEL_CPU_COUNT()          1

#define CYG_KERNEL_CPU_THIS()           0

#define CYG_KERNEL_CPU_NONE             -1

#define CYG_KERNEL_CPU_LOWPRI()         CYG_KERNEL_CPU_THIS()

// -------------------------------------------------------------------------
// SpinLock class
// This single CPU version simply goes through the motions of setting
// and clearing the lock variable for debugging purposes. 

#ifdef __cplusplus

class Cyg_SpinLock
{
    volatile cyg_uint32 lock;

public:

    // Constructor, initialize the lock to clear
    Cyg_SpinLock() { lock = 0; };

    ~Cyg_SpinLock()
    {
        CYG_ASSERT( lock == 0, "spinlock still claimed");
    };
    
    // Spin on the lock. In this case we just set it to 1 and proceed.
    void spin()
    {
        CYG_ASSERT( lock == 0, "spinlock already claimed!");
        lock = 1;
    };

    // Clear the lock. Again, just set the value.
    void clear()
    {
        CYG_ASSERT( lock != 0, "spinlock already cleared!");
        lock = 0;
    };

    // Try to claim the lock. Return true if successful, false if not.
    cyg_bool trylock()
    {
        if( lock ) return false;
        else { lock = 1; return true; }
    };

    // Test the current value of the lock
    cyg_bool test() { return lock; };


    // The following two member functions are only necessary if the
    // spinlock is to be used in an ISR. 
    
    // Claim the spinlock, but also mask this CPU's interrupts while
    // we have it.
    void spin_intsave(CYG_INTERRUPT_STATE *state)
    {
        CYG_INTERRUPT_STATE s;
        HAL_DISABLE_INTERRUPTS(s);
        *state = s;
        spin();
    };

    // Clear the lock, and restore the interrupt state saved in
    // spin_intsave().
    void clear_intsave(CYG_INTERRUPT_STATE state)
    {
        clear();
        HAL_RESTORE_INTERRUPTS(state);
    };

};

// -------------------------------------------------------------------------
// Scheduler lock class

class Cyg_Scheduler_SchedLock
{
    static volatile cyg_ucount32 sched_lock         // lock counter
                    CYGBLD_ATTRIB_ASM_ALIAS( cyg_scheduler_sched_lock )
                    CYGBLD_ANNOTATE_VARIABLE_SCHED
                    ;
    
    // For non-SMP versions, the code here does the basic and obvious things.
protected:

    Cyg_Scheduler_SchedLock()
    {
        sched_lock = 1;
    };
    
    // Increment the scheduler lock, possibly taking it from zero to
    // one.
    static void inc_sched_lock()
    {
        sched_lock++;
    };

    static void zero_sched_lock()
    {
        CYG_ASSERT( sched_lock != 0, "Scheduler lock already zero");
        sched_lock = 0;
    };
    
    // Set the scheduler lock to a non-zero value. Both the scheduler
    // lock and the new value must be non-zero.
    static void set_sched_lock(cyg_uint32 new_lock)
    {
        CYG_ASSERT( new_lock > 0, "New scheduler lock value == 0");
        CYG_ASSERT( sched_lock > 0, "Scheduler lock == 0");
        sched_lock = new_lock;
    };

    static cyg_ucount32 get_sched_lock()
    {
        return sched_lock;
    };
};

#define CYGIMP_KERNEL_SCHED_LOCK_DEFINITIONS                    \
volatile cyg_ucount32 Cyg_Scheduler_SchedLock::sched_lock = 1;

#endif // __cplusplus

#endif // defined(CYGSEM_KERNEL_SMP_SUPPORT) && (CYGSEM_HAL_SMP_SUPPORT)

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_SMP_HXX

// EOF smp.hxx
