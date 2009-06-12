//==========================================================================
//
//      drv_api.c
//
//      Driver API for non-kernel configurations
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
// Author(s):   Nick Garnett
// Date:        1999-02-24
// Purpose:     Driver API for non-kernel configurations
// Description: These functions are used to support drivers when the kernel
//              is not present.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>

#ifndef CYGPKG_KERNEL

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/cyg_ass.h>

#include <pkgconf/hal.h>
#include <cyg/hal/drv_api.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

//--------------------------------------------------------------------------
// Statics

static volatile cyg_int32 isr_disable_counter = 1;  // ISR disable counter

static CYG_INTERRUPT_STATE isr_disable_state;

volatile cyg_int32 dsr_disable_counter  // DSR disable counter
                      CYGBLD_ATTRIB_ASM_ALIAS( cyg_scheduler_sched_lock );

static cyg_interrupt* volatile dsr_list;        // List of pending DSRs

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

cyg_interrupt *chain_list[CYGNUM_HAL_ISR_COUNT];

#endif

//--------------------------------------------------------------------------
// DSR handling functions.
// post_dsr() places a DSR on the list of DSRs to be called.
// call_dsrs() calls the DSRs.

static void post_dsr( cyg_interrupt *intr )
{
    CYG_INTERRUPT_STATE old_intr;

    CYG_REPORT_FUNCTION();

    HAL_DISABLE_INTERRUPTS(old_intr);

    if( intr->dsr_count++ == 0 )
    {
        intr->next_dsr = dsr_list;
        dsr_list = intr;
    }

    HAL_RESTORE_INTERRUPTS(old_intr);

    CYG_REPORT_RETURN();    
}

static void call_dsrs(void)
{
    CYG_REPORT_FUNCTION();
    
    while( dsr_list != NULL )
    {
        cyg_interrupt *intr;
        cyg_int32 count;
        CYG_INTERRUPT_STATE old_intr;

        HAL_DISABLE_INTERRUPTS(old_intr);
        
        intr = dsr_list;
        dsr_list = intr->next_dsr;
        count = intr->dsr_count;
        intr->dsr_count = 0;
        
        HAL_RESTORE_INTERRUPTS(old_intr);

        intr->dsr( intr->vector, count, (CYG_ADDRWORD)intr->data );
    }

    CYG_REPORT_RETURN();
    
}

//--------------------------------------------------------------------------
// This is referenced from the HAL, although it does not actually get called.

externC void
cyg_interrupt_call_pending_DSRs(void)
{
    call_dsrs();
}

//--------------------------------------------------------------------------
// This is called from springboard ISRs in some HALs.

externC void
cyg_interrupt_post_dsr(CYG_ADDRWORD data)
{
  cyg_interrupt * intr = (cyg_interrupt *)data;
  post_dsr(intr);
}

//--------------------------------------------------------------------------
// Interrupt end function called from HAL VSR to tidy up. This is where
// DSRs will be called if necessary.

externC void
interrupt_end(
    cyg_uint32          isr_ret,
    cyg_interrupt       *intr,
    HAL_SavedRegisters  *regs
    )
{
    CYG_REPORT_FUNCTION();
    
#ifndef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

    // Only do this if we are in a non-chained configuration.
    // If we are chained, then chain_isr will do the DSR
    // posting.
    
    if( isr_ret & CYG_ISR_CALL_DSR && intr != NULL ) post_dsr(intr);

#endif

    if( dsr_disable_counter == 0 ) call_dsrs();

    CYG_REPORT_RETURN();    
}

//--------------------------------------------------------------------------
// ISR for handling chained interrupts.

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

cyg_uint32 chain_isr(cyg_vector_t vector, CYG_ADDRWORD data)
{
    cyg_interrupt *p = *(cyg_interrupt **)data;
    register cyg_uint32 isr_ret = 0;
    register cyg_uint32 isr_chain_ret = 0;

    CYG_REPORT_FUNCTION();

    while( p != NULL )
    {
        if( p->vector == vector )
        {
            isr_ret = p->isr(vector, p->data);

            isr_chain_ret |= isr_ret;

            if( isr_ret & CYG_ISR_CALL_DSR ) post_dsr(p);

            if( isr_ret & CYG_ISR_HANDLED ) break;
        }

        p = p->next;
    }

#ifdef HAL_DEFAULT_ISR
    if( (isr_chain_ret & (CYG_ISR_CALL_DSR|CYG_ISR_HANDLED)) == 0 )
    {
        // If we finished the loop for some reason other than that an
        // ISR has handled the interrupt, call any default ISR to either
        // report the spurious interrupt, or do some other HAL level processing
        // such as GDB interrupt detection etc.

        HAL_DEFAULT_ISR( vector, 0 );
    }
#endif    

    CYG_REPORT_RETURN();
    
    return isr_ret & CYG_ISR_CALL_DSR;
}

#endif

//--------------------------------------------------------------------------
// ISR lock. This disables interrupts and keeps a count of the number
// times it has been called.

externC void cyg_drv_isr_lock()
{
    CYG_REPORT_FUNCTION();

    if( isr_disable_counter == 0 )
        HAL_DISABLE_INTERRUPTS(isr_disable_state);

    CYG_ASSERT( isr_disable_counter >= 0 , "Disable counter negative");
    
    isr_disable_counter++;
    
    CYG_REPORT_RETURN();    
}

//--------------------------------------------------------------------------
// Unlock ISRs. This decrements the count and re-enables interrupts if it
// goes zero.

externC void cyg_drv_isr_unlock()
{
    CYG_REPORT_FUNCTION();
        
    CYG_ASSERT( isr_disable_counter > 0 , "Disable counter not greater than zero");
    
    isr_disable_counter--;

    if ( isr_disable_counter == 0 )
    {
        HAL_RESTORE_INTERRUPTS(isr_disable_state);
    }

    CYG_REPORT_RETURN();
}

//--------------------------------------------------------------------------
// Lock DSR lock. Simply increment the counter.

externC void cyg_drv_dsr_lock()
{
    CYG_REPORT_FUNCTION();

    dsr_disable_counter++;

    CYG_REPORT_RETURN();
}

//--------------------------------------------------------------------------
// Unlock DSR lock. If the counter is about to go zero, call any pending
// DSRs and then zero the counter.

externC void cyg_drv_dsr_unlock()
{
    CYG_REPORT_FUNCTION();

    do
    {
        if( dsr_disable_counter == 1 )
        {
            call_dsrs();
        }

        HAL_REORDER_BARRIER();
        
        dsr_disable_counter = 0;

        HAL_REORDER_BARRIER();

        // Check that no DSRs have been posted between calling
        // call_dsrs() and zeroing dsr_disable_counter. If so,
        // loop back and call them.
        
        if( dsr_list != NULL )
        {
            dsr_disable_counter = 1;
            continue;
        }

        CYG_REPORT_RETURN();
        
        return;
        
    } while(1);

    CYG_FAIL( "Should not be executed" );
}

//--------------------------------------------------------------------------
// Initialize a mutex.

externC void cyg_drv_mutex_init( cyg_drv_mutex_t *mutex )
{
    CYG_REPORT_FUNCTION();
    
    mutex->lock = 0;

    CYG_REPORT_RETURN();    
}

//--------------------------------------------------------------------------
// Destroy a mutex.

externC void cyg_drv_mutex_destroy( cyg_drv_mutex_t *mutex )
{
    CYG_REPORT_FUNCTION();
    
    mutex->lock = -1;

    CYG_REPORT_RETURN();    
}

//--------------------------------------------------------------------------
// Lock a mutex. We check that we are not trying to lock a locked or
// destroyed mutex and if not, set it locked.

externC cyg_bool_t cyg_drv_mutex_lock( cyg_drv_mutex_t *mutex )
{
    CYG_REPORT_FUNCTION();
    
    CYG_ASSERT( mutex->lock == 0 , "Trying to lock locked mutex");

    mutex->lock = 1;

    CYG_REPORT_RETURN();

    return true;
}

//--------------------------------------------------------------------------
// Attempt to claim a mutex, and return if it cannot be.

externC cyg_bool_t cyg_drv_mutex_trylock( cyg_drv_mutex_t *mutex )
{
    cyg_bool_t result = true;
    
    CYG_REPORT_FUNCTION();

    if( mutex->lock == 1 ) result = false;

    mutex->lock = 1;
    
    CYG_REPORT_RETURN();

    return result;
}

//--------------------------------------------------------------------------
// Unlock a mutex. We check that the mutex is actually locked before doing
// this.

externC void cyg_drv_mutex_unlock( cyg_drv_mutex_t *mutex )
{
    CYG_REPORT_FUNCTION();

    CYG_ASSERT( mutex->lock == 1 , "Trying to unlock unlocked mutex");

    mutex->lock = 0;
    
    CYG_REPORT_RETURN();    
}
    
//--------------------------------------------------------------------------
// Release all threads waiting for the mutex.
// This is really for threads, so we do nothing here.

externC void cyg_drv_mutex_release( cyg_drv_mutex_t *mutex )
{
    CYG_REPORT_FUNCTION();


    
    CYG_REPORT_RETURN();    
}
    

//--------------------------------------------------------------------------
// Initialized a condition variable.

externC void cyg_drv_cond_init( cyg_drv_cond_t  *cond, cyg_drv_mutex_t *mutex )
{
    CYG_REPORT_FUNCTION();

    cond->wait = 0;
    cond->mutex = mutex;
    
    CYG_REPORT_RETURN();    
}
    

//--------------------------------------------------------------------------
// Destroy a condition variable.

externC void cyg_drv_cond_destroy( cyg_drv_cond_t  *cond )
{
    CYG_REPORT_FUNCTION();

    cond->wait = -1;
    cond->mutex = NULL;
    
    CYG_REPORT_RETURN();    
}
    
// -------------------------------------------------------------------------
// Wait for a condition variable to be signalled. We simply busy wait
// polling the condition variable's wait member until a DSR sets it to
// 0.  Note that the semantics of condition variables means that the
// wakeup only happens if there is a thread actually waiting on the CV
// when the signal is sent.

externC cyg_bool cyg_drv_cond_wait( cyg_drv_cond_t *cond )
{
    CYG_REPORT_FUNCTION();

    CYG_ASSERT( cond->mutex != NULL, "Uninitialized condition variable");
    CYG_ASSERT( cond->mutex->lock, "Mutex not locked");

    cyg_drv_dsr_lock();
    
    cond->wait = 1;
       
    while( cond->wait == 1 )
    {
        // While looping we call call_dsrs() to service any DSRs that
        // get posted. One of these will make the call to cond_signal
        // to break us out of this loop. If we do not have the DSR
        // lock claimed, then a race condition could occur and keep us
        // stuck here forever.
        
        call_dsrs();
    }

    cyg_drv_dsr_unlock();
    
    CYG_REPORT_RETURN();

    return true;
}

//--------------------------------------------------------------------------
// Signal a condition variable. This sets the wait member to zero, which
// has no effect when there is no waiter, but will wake up any waiting
// thread.

externC void cyg_drv_cond_signal( cyg_drv_cond_t *cond )
{
    CYG_REPORT_FUNCTION();

    cond->wait = 0;
    
    CYG_REPORT_RETURN();    
}
    

//--------------------------------------------------------------------------
// Broadcast to condition variable. This is exactly the same a signal since
// there can only be one waiter.

externC void cyg_drv_cond_broadcast( cyg_drv_cond_t *cond )
{
    CYG_REPORT_FUNCTION();

    cond->wait = 0;
    
    CYG_REPORT_RETURN();    
}

//--------------------------------------------------------------------------
// Spinlock support.
// Since we can only support a single CPU in this version of the API, we only
// set and clear the lock variable to keep track of what's happening.

void cyg_drv_spinlock_init(
    cyg_drv_spinlock_t  *lock,          /* spinlock to initialize            */
    cyg_bool_t          locked          /* init locked or unlocked           */
)
{
    CYG_REPORT_FUNCTION();
    
    lock->lock = locked;

    CYG_REPORT_RETURN();    
}

void cyg_drv_spinlock_destroy( cyg_drv_spinlock_t *lock )
{
    CYG_REPORT_FUNCTION();
    
    lock->lock = -1;

    CYG_REPORT_RETURN();    
}

void cyg_drv_spinlock_spin( cyg_drv_spinlock_t *lock )
{
    CYG_REPORT_FUNCTION();
    
    CYG_ASSERT( lock->lock == 0 , "Trying to lock locked spinlock");

    lock->lock = 1;

    CYG_REPORT_RETURN();
}

void cyg_drv_spinlock_clear( cyg_drv_spinlock_t *lock )
{
    CYG_REPORT_FUNCTION();

    CYG_ASSERT( lock->lock == 1 , "Trying to clear cleared spinlock");

    lock->lock = 0;
    
    CYG_REPORT_RETURN();    
}

cyg_bool_t cyg_drv_spinlock_try( cyg_drv_spinlock_t *lock )
{
    cyg_bool_t result = true;
    
    CYG_REPORT_FUNCTION();

    if( lock->lock == 1 ) result = false;

    lock->lock = 1;
    
    CYG_REPORT_RETURN();

    return result;
}

cyg_bool_t cyg_drv_spinlock_test( cyg_drv_spinlock_t *lock )
{
    cyg_bool_t result = true;
    
    CYG_REPORT_FUNCTION();

    if( lock->lock == 1 ) result = false;

    CYG_REPORT_RETURN();

    return result;
}

void cyg_drv_spinlock_spin_intsave( cyg_drv_spinlock_t *lock,
                                    cyg_addrword_t *istate )
{
    CYG_REPORT_FUNCTION();

    HAL_DISABLE_INTERRUPTS( *istate );

    lock->lock = 1;
    
    CYG_REPORT_RETURN();
}
    

void cyg_drv_spinlock_clear_intsave( cyg_drv_spinlock_t *lock,
                                     cyg_addrword_t istate )
{
    CYG_REPORT_FUNCTION();

    lock->lock = 0;
    
    HAL_RESTORE_INTERRUPTS( istate );
    
    CYG_REPORT_RETURN();
}

//--------------------------------------------------------------------------
// Create an interrupt object.

externC void cyg_drv_interrupt_create(
                     cyg_vector_t        vector,
                     cyg_priority_t      priority,
                     cyg_addrword_t      data,
                     cyg_ISR_t           *isr,
                     cyg_DSR_t           *dsr,
                     cyg_handle_t        *handle,
                     cyg_interrupt       *intr
                     )
{
    CYG_REPORT_FUNCTION();

    intr->vector        = vector;
    intr->priority      = priority;
    intr->isr           = isr;
    intr->dsr           = dsr;
    intr->data          = data;
    intr->next_dsr      = NULL;
    intr->dsr_count     = 0;

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

    intr->next          = NULL;
    
#endif    

    *handle = (cyg_handle_t)intr;
    
    CYG_REPORT_RETURN();
}

//--------------------------------------------------------------------------
// Delete an interrupt object. This merely ensures that it is detached from
// the vector.

externC void cyg_drv_interrupt_delete( cyg_handle_t interrupt )
{
    CYG_REPORT_FUNCTION();

    cyg_drv_interrupt_detach( interrupt );
    
    CYG_REPORT_RETURN();    
}

//--------------------------------------------------------------------------
// 

externC void cyg_drv_interrupt_attach( cyg_handle_t interrupt )
{
    cyg_interrupt *intr = (cyg_interrupt *)interrupt;
    
    CYG_REPORT_FUNCTION();

    CYG_ASSERT( intr->vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");
    CYG_ASSERT( intr->vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    HAL_INTERRUPT_SET_LEVEL( intr->vector, intr->priority );
    
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

    CYG_ASSERT( intr->next == NULL , "cyg_interrupt already on a list");

    {
        cyg_uint32 index;

        HAL_TRANSLATE_VECTOR( intr->vector, index );

        if( chain_list[index] == NULL ) 
        {
            // First Interrupt on this chain, just assign it and
            // register the chain_isr with the HAL.
        
            chain_list[index] = intr;

            HAL_INTERRUPT_ATTACH( intr->vector, chain_isr, 
                                  &chain_list[index], NULL );
        } 
        else
        {
            // There are already interrupts chained, add this one into
            // the chain in priority order.
        
            cyg_interrupt **p = &chain_list[index];

            while( *p != NULL )
            {
                cyg_interrupt *n = *p;
                
                if( n->priority < intr->priority ) break;
            
                p = &n->next;
            }
            intr->next = *p;
            *p = intr;
        }
    }
    
#else
    
    HAL_INTERRUPT_ATTACH( intr->vector, intr->isr, intr->data, intr );

#endif    
    
    CYG_REPORT_RETURN();    
}
   

//--------------------------------------------------------------------------
// Detach an interrupt from its vector.

externC void cyg_drv_interrupt_detach( cyg_handle_t interrupt )
{
    cyg_interrupt *intr = (cyg_interrupt *)interrupt;
    
    CYG_REPORT_FUNCTION();

    CYG_ASSERT( intr->vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( intr->vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

    // Remove the interrupt object from the vector chain.

    {    
        cyg_uint32 index;
        cyg_interrupt **p;

        HAL_TRANSLATE_VECTOR( intr->vector, index );

        p = &chain_list[index];

        while( *p != NULL )
        {
            cyg_interrupt *n = *p;
            
            if( n == intr )
            {
                *p = intr->next;
                break;
            }
            
            p = &n->next;
        }

        // If this was the last one, detach the vector.
    
        if( chain_list[index] == NULL )
            HAL_INTERRUPT_DETACH( intr->vector, chain_isr );
    }
    
#else
    
    HAL_INTERRUPT_DETACH( intr->vector, intr->isr );

#endif
    
    CYG_REPORT_RETURN();    
}
    

//--------------------------------------------------------------------------
// Mask delivery of an interrupt at the interrupt controller.
// (Interrupt safe)

externC void cyg_drv_interrupt_mask( cyg_vector_t vector )
{
    CYG_INTERRUPT_STATE old_ints;

    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("vector=%d", vector);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    HAL_DISABLE_INTERRUPTS(old_ints);
    HAL_INTERRUPT_MASK( vector );
    HAL_RESTORE_INTERRUPTS(old_ints);

    CYG_REPORT_RETURN();
}

//--------------------------------------------------------------------------
// Mask delivery of an interrupt at the interrupt controller.
// (Not interrupt safe)

externC void cyg_drv_interrupt_mask_intunsafe( cyg_vector_t vector )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("vector=%d", vector);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    HAL_INTERRUPT_MASK( vector );

    CYG_REPORT_RETURN();
}

//--------------------------------------------------------------------------
// Unmask delivery of an interrupt at the interrupt controller.
// (Interrupt safe)

externC void cyg_drv_interrupt_unmask( cyg_vector_t vector )
{
    CYG_INTERRUPT_STATE old_ints;
    
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("vector=%d", vector);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");
    
    HAL_DISABLE_INTERRUPTS(old_ints);
    HAL_INTERRUPT_UNMASK( vector );
    HAL_RESTORE_INTERRUPTS(old_ints);

    CYG_REPORT_RETURN();
}

//--------------------------------------------------------------------------
// Unmask delivery of an interrupt at the interrupt controller.
// (Not interrupt safe)

externC void cyg_drv_interrupt_unmask_intunsafe( cyg_vector_t vector )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("vector=%d", vector);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");
    
    HAL_INTERRUPT_UNMASK( vector );

    CYG_REPORT_RETURN();
}

//--------------------------------------------------------------------------
// Acknowledge an interrupt at the controller to allow another interrupt
// to be delivered.

externC void cyg_drv_interrupt_acknowledge( cyg_vector_t vector )
{
//    CYG_REPORT_FUNCTION();

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    HAL_INTERRUPT_ACKNOWLEDGE( vector );

//    CYG_REPORT_RETURN();    
}

//--------------------------------------------------------------------------
// Configure interrupt detection parameters.

externC void cyg_drv_interrupt_configure(
                     cyg_vector_t        vector,
                     cyg_bool_t          level,
                     cyg_bool_t          up
                     )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3("vector = %d, level = %d, up = %d", vector, level,
                        up);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    HAL_INTERRUPT_CONFIGURE( vector, level, up );

    CYG_REPORT_RETURN();
}

//--------------------------------------------------------------------------
// Configure interrupt priority level.

externC void cyg_drv_interrupt_level( cyg_vector_t vector, cyg_priority_t level )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2("vector = %d, level = %d", vector, level);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    HAL_INTERRUPT_SET_LEVEL( vector, level );

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// CPU interrupt routing

externC void cyg_drv_interrupt_set_cpu( cyg_vector_t vector, cyg_cpu_t cpu )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2("vector = %d, cpu = %d", vector, cpu);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

#ifdef CYGPKG_HAL_SMP_SUPPORT    
    HAL_INTERRUPT_SET_CPU( vector, cpu );
#endif
    
    CYG_REPORT_RETURN();
}

externC cyg_cpu_t cyg_drv_interrupt_get_cpu( cyg_vector_t vector )
{
    cyg_cpu_t cpu = 0;
    
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("vector = %d", vector);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

#ifdef CYGPKG_HAL_SMP_SUPPORT    
    HAL_INTERRUPT_GET_CPU( vector, cpu );
#endif
    
    CYG_REPORT_RETURN();

    return cpu;
}

// -------------------------------------------------------------------------
// Exception delivery function called from the HAL as a result of a
// hardware exception being raised.

externC void cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data )
{
    CYG_FAIL(" !!! Exception !!! ");
}


#endif

//--------------------------------------------------------------------------
// EOF drv_api.c
