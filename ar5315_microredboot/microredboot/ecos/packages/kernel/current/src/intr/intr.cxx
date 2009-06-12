//==========================================================================
//
//      intr/intr.cxx
//
//      Interrupt class implementations
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
// Author(s):    nickg
// Contributors: nickg
// Date:         1999-02-17
// Purpose:      Interrupt class implementation
// Description:  This file contains the definitions of the interrupt
//               class.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>             // base kernel types
#include <cyg/infra/cyg_trac.h>           // tracing macros
#include <cyg/infra/cyg_ass.h>            // assertion macros
#include <cyg/kernel/instrmnt.h>           // instrumentation

#include <cyg/kernel/intr.hxx>             // our header

#include <cyg/kernel/sched.hxx>            // scheduler

#include <cyg/kernel/sched.inl>

// -------------------------------------------------------------------------
// Statics

volatile cyg_int32 Cyg_Interrupt::disable_counter[CYGNUM_KERNEL_CPU_MAX];

Cyg_SpinLock Cyg_Interrupt::interrupt_disable_spinlock CYG_INIT_PRIORITY( INTERRUPTS );

CYG_INTERRUPT_STATE Cyg_Interrupt::interrupt_disable_state[CYGNUM_KERNEL_CPU_MAX];

// -------------------------------------------------------------------------

Cyg_Interrupt::Cyg_Interrupt(
    cyg_vector      vec,                // Vector to attach to
    cyg_priority    pri,                // Queue priority
    CYG_ADDRWORD    d,                  // Data pointer
    cyg_ISR         *ir,                // Interrupt Service Routine
    cyg_DSR         *dr                 // Deferred Service Routine
    )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5("vector=%d, priority=%d, data=%08x, isr=%08x, "
                        "dsr=%08x", vec, pri, d, ir, dr);
    
    vector      = vec;
    priority    = pri;
    isr         = ir;
    dsr         = dr;
    data        = d;

#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST

    dsr_count   = 0;
    next_dsr    = NULL;

#endif

#ifdef CYGIMP_KERNEL_INTERRUPTS_CHAIN

    next        = NULL;
    
#endif

    CYG_REPORT_RETURN();
    
};

// -------------------------------------------------------------------------

Cyg_Interrupt::~Cyg_Interrupt()
{
    CYG_REPORT_FUNCTION();
    detach();
    CYG_REPORT_RETURN();
};

// -------------------------------------------------------------------------
// DSR handling statics:

#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_TABLE

Cyg_Interrupt *
Cyg_Interrupt::dsr_table[CYGNUM_KERNEL_CPU_MAX][CYGNUM_KERNEL_INTERRUPTS_DSRS_TABLE_SIZE];

cyg_ucount32 Cyg_Interrupt::dsr_table_head[CYGNUM_KERNEL_CPU_MAX];

volatile cyg_ucount32 Cyg_Interrupt::dsr_table_tail[CYGNUM_KERNEL_CPU_MAX];

#endif

#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST

Cyg_Interrupt* volatile Cyg_Interrupt::dsr_list[CYGNUM_KERNEL_CPU_MAX];

#endif

// -------------------------------------------------------------------------
// Call any pending DSRs

void
Cyg_Interrupt::call_pending_DSRs_inner(void)
{
//    CYG_REPORT_FUNCTION();

    HAL_SMP_CPU_TYPE cpu = CYG_KERNEL_CPU_THIS();
    
#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_TABLE    

    while( dsr_table_head[cpu] != dsr_table_tail[cpu] )
    {
        Cyg_Interrupt *intr = dsr_table[cpu][dsr_table_head[cpu]];

        dsr_table_head[cpu]++;
        if( dsr_table_head[cpu] >= CYGNUM_KERNEL_INTERRUPTS_DSRS_TABLE_SIZE )
            dsr_table_head[cpu] = 0;

        CYG_INSTRUMENT_INTR(CALL_DSR, intr->vector, 0);
        
        CYG_ASSERT( intr->dsr != NULL , "No DSR defined");

        intr->dsr( intr->vector, 1, (CYG_ADDRWORD)intr->data );
    }
    
#endif

#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST

    while( dsr_list[cpu] != NULL )
    {
        Cyg_Interrupt* intr;
        cyg_uint32 old_intr;
        cyg_count32 count;
        
        HAL_DISABLE_INTERRUPTS(old_intr);
        
        intr = dsr_list[cpu];
        dsr_list[cpu] = intr->next_dsr;
        count = intr->dsr_count;
        intr->dsr_count = 0;
        
        HAL_RESTORE_INTERRUPTS(old_intr);
        
        CYG_ASSERT( intr->dsr != NULL , "No DSR defined");

        intr->dsr( intr->vector, count, (CYG_ADDRWORD)intr->data );
        
    }
    
#endif
    
};

externC void
cyg_interrupt_call_pending_DSRs(void)
{
    Cyg_Interrupt::call_pending_DSRs_inner();
}

//
// Use HAL supported function to run through the DSRs, but executing using
// the separate interrupt stack if available.  This function calls back
// into this module via 'cyg_interrupt_call_pending_DSRs' above, to keep
// the whole process as general as possible.

void
Cyg_Interrupt::call_pending_DSRs(void)
{
    CYG_ASSERT( Cyg_Scheduler::get_sched_lock() == 1,
                "DSRs being called with sched_lock not equal to 1");
    HAL_INTERRUPT_STACK_CALL_PENDING_DSRS();
}


// -------------------------------------------------------------------------

void
Cyg_Interrupt::post_dsr(void)
{
//    CYG_REPORT_FUNCTION();
    HAL_SMP_CPU_TYPE cpu = CYG_KERNEL_CPU_THIS();
    
    CYG_INSTRUMENT_INTR(POST_DSR, vector, 0);

    cyg_uint32 old_intr;

    // We need to disable interrupts during this part to
    // guard against nested interrupts.
    
    HAL_DISABLE_INTERRUPTS(old_intr);

#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_TABLE
    
    dsr_table[cpu][dsr_table_tail[cpu]++] = this;
    if( dsr_table_tail[cpu] >= CYGNUM_KERNEL_INTERRUPTS_DSRS_TABLE_SIZE )
        dsr_table_tail[cpu] = 0;

#endif

#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST

    // Only add the interrupt to the dsr list if this is
    // the first DSR call.
    // At present DSRs are pushed onto the list and will be
    // called in reverse order. We do not define the order
    // in which DSRs are called, so this is acceptable.
    
    if( dsr_count++ == 0 )
    {
        next_dsr = dsr_list[cpu];
        dsr_list[cpu] = this;
    }
    
#endif
    
    HAL_RESTORE_INTERRUPTS(old_intr);    
};

// -------------------------------------------------------------------------
// A C callable interface to Cyg_Interrupt::post_dsr() that can be used from
// the HAL.

externC void
cyg_interrupt_post_dsr( CYG_ADDRWORD intr_obj )
{
    Cyg_Interrupt* intr = (Cyg_Interrupt*) intr_obj;
    intr->post_dsr ();
}

// -------------------------------------------------------------------------

// FIXME: should have better name - Jifl
externC void
interrupt_end(
    cyg_uint32          isr_ret,
    Cyg_Interrupt       *intr,
    HAL_SavedRegisters  *regs
    )
{
//    CYG_REPORT_FUNCTION();

#ifdef CYGPKG_KERNEL_SMP_SUPPORT
    Cyg_Scheduler::lock();
#endif
    
    // Sometimes we have a NULL intr object pointer.
    cyg_vector vector = (intr!=NULL)?intr->vector:0;

    CYG_INSTRUMENT_INTR(END, vector, isr_ret);
    
    CYG_UNUSED_PARAM( cyg_vector, vector ); // prevent compiler warning
    
#ifndef CYGIMP_KERNEL_INTERRUPTS_CHAIN

    // Only do this if we are in a non-chained configuration.
    // If we are chained, then chain_isr below will do the DSR
    // posting.
    
    if( isr_ret & Cyg_Interrupt::CALL_DSR && intr != NULL ) intr->post_dsr();

#endif    

#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT

    // If we have GDB support enabled, and there is the possibility
    // that this thread will be context switched as a result of this
    // interrupt, then save the pointer to the saved thread context in
    // the thread object so that GDB can get a meaningful context to
    // look at.
    
    Cyg_Scheduler::get_current_thread()->set_saved_context(regs);
    
#endif    
    
    // Now unlock the scheduler, which may also call DSRs
    // and cause a thread switch to happen.
    
    Cyg_Scheduler::unlock();

#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT

    Cyg_Scheduler::get_current_thread()->set_saved_context(0);
    
#endif    
    
    CYG_INSTRUMENT_INTR(RESTORE, vector, 0);    
}

// -------------------------------------------------------------------------
// Interrupt chaining statics.

#ifdef CYGIMP_KERNEL_INTERRUPTS_CHAIN

Cyg_Interrupt *Cyg_Interrupt::chain_list[CYGNUM_HAL_ISR_TABLE_SIZE];

#endif

// -------------------------------------------------------------------------
// Chaining ISR inserted in HAL vector

#ifdef CYGIMP_KERNEL_INTERRUPTS_CHAIN

cyg_uint32
Cyg_Interrupt::chain_isr(cyg_vector vector, CYG_ADDRWORD data)
{
    Cyg_Interrupt *p = *(Cyg_Interrupt **)data;
    register cyg_uint32 isr_ret = 0;
    register cyg_uint32 isr_chain_ret = 0;

    CYG_INSTRUMENT_INTR(CHAIN_ISR, vector, 0);

    while( p != NULL )
    {
        if( p->vector == vector )
        {
            isr_ret = p->isr(vector, p->data);

            isr_chain_ret |= isr_ret;

            if( isr_ret & Cyg_Interrupt::CALL_DSR ) p->post_dsr();

            if( isr_ret & Cyg_Interrupt::HANDLED ) break;
        }

        p = p->next;
    }

#ifdef HAL_DEFAULT_ISR
    if( (isr_chain_ret & (Cyg_Interrupt::HANDLED|Cyg_Interrupt::CALL_DSR)) == 0 )
    {
        // If we finished the loop for some reason other than that an
        // ISR has handled the interrupt, call any default ISR to either
        // report the spurious interrupt, or do some other HAL level processing
        // such as GDB interrupt detection etc.

        HAL_DEFAULT_ISR( vector, 0 );
    }
#endif    

    return isr_ret & ~Cyg_Interrupt::CALL_DSR;
}

#endif

// -------------------------------------------------------------------------
// Attach an ISR to an interrupt vector.

void
Cyg_Interrupt::attach(void)
{
    CYG_REPORT_FUNCTION();

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    CYG_INSTRUMENT_INTR(ATTACH, vector, 0);

    HAL_INTERRUPT_SET_LEVEL( vector, priority );
    
#ifdef CYGIMP_KERNEL_INTERRUPTS_CHAIN

    CYG_ASSERT( next == NULL , "Cyg_Interrupt already on a list");

    cyg_uint32 index;

    HAL_TRANSLATE_VECTOR( vector, index );

    if( chain_list[index] == NULL )
    {
        int in_use;
        // First Interrupt on this chain, just assign it and register
        // the chain_isr with the HAL.
        
        chain_list[index] = this;

        HAL_INTERRUPT_IN_USE( vector, in_use );
        CYG_ASSERT( 0 == in_use, "Interrupt vector not free.");
        HAL_INTERRUPT_ATTACH( vector, chain_isr, &chain_list[index], NULL );
    }
    else
    {
        // There are already interrupts chained, add this one into the
        // chain in priority order.
        
        Cyg_Interrupt **p = &chain_list[index];

        while( *p != NULL )
        {
            Cyg_Interrupt *n = *p;

            if( n->priority < priority ) break;
            
            p = &n->next;
        }
        next = *p;
        *p = this;
    }
    
#else
    
    {
        int in_use;


        HAL_INTERRUPT_IN_USE( vector, in_use );
        CYG_ASSERT( 0 == in_use, "Interrupt vector not free.");

        HAL_INTERRUPT_ATTACH( vector, isr, data, this );
    }

#endif    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Detach the ISR from the vector

void
Cyg_Interrupt::detach(void)
{
    CYG_REPORT_FUNCTION();

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    CYG_INSTRUMENT_INTR(DETACH, vector, 0);

#ifdef CYGIMP_KERNEL_INTERRUPTS_CHAIN

    // Remove the interrupt object from the vector chain.
    
    cyg_uint32 index;

    HAL_TRANSLATE_VECTOR( vector, index );

    Cyg_Interrupt **p = &chain_list[index];

    while( *p != NULL )
    {
        Cyg_Interrupt *n = *p;

        if( n == this )
        {
            *p = next;
            break;
        }
            
        p = &n->next;
    }

    // If this was the last one, detach the vector.
    
    if( chain_list[index] == NULL )
        HAL_INTERRUPT_DETACH( vector, chain_isr );
    
#else
    
    HAL_INTERRUPT_DETACH( vector, isr );

#endif

    CYG_REPORT_RETURN();
    
}

// -------------------------------------------------------------------------
// Get the current service routine

void
Cyg_Interrupt::get_vsr(cyg_vector vector, cyg_VSR **vsr)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2("vector = %d, mem to put VSR in is at %08x", vector,
                        vsr);

    CYG_ASSERT( vector >= CYGNUM_HAL_VSR_MIN, "Invalid vector");        
    CYG_ASSERT( vector <= CYGNUM_HAL_VSR_MAX, "Invalid vector");

    HAL_VSR_GET( vector, vsr );

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Install a vector service routine

void
Cyg_Interrupt::set_vsr(cyg_vector vector, cyg_VSR *vsr, cyg_VSR **old)
{
    CYG_REPORT_FUNCTION();

    CYG_REPORT_FUNCARG3( "vector = %d, new vsr is at %08x, mem to put "
                         "old VSR in is at %08x", vector, vsr, old);

    CYG_INSTRUMENT_INTR(SET_VSR, vector, vsr);

    CYG_ASSERT( vector >= CYGNUM_HAL_VSR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_VSR_MAX, "Invalid vector");    

    CYG_INTERRUPT_STATE old_ints;
    
    HAL_DISABLE_INTERRUPTS(old_ints);

    HAL_VSR_SET( vector, vsr, old );
    
    HAL_RESTORE_INTERRUPTS(old_ints);

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Disable interrupts at the CPU


void
Cyg_Interrupt::disable_interrupts(void)
{
    CYG_REPORT_FUNCTION();

    CYG_INSTRUMENT_INTR(DISABLE, disable_counter[CYG_KERNEL_CPU_THIS()]+1, 0);

    HAL_SMP_CPU_TYPE cpu_this = CYG_KERNEL_CPU_THIS();

    // If the disable_counter is zero, disable interrupts and claim the spinlock.
    
    if( 0 == disable_counter[cpu_this] )
    {
        // Claim the spinlock and disable interrupts. We save the original interrupt
        // enable state to restore later.
        interrupt_disable_spinlock.spin_intsave(&interrupt_disable_state[cpu_this]);
    }

    // Now increment our disable counter.
    
    disable_counter[cpu_this]++;
    
    CYG_REPORT_RETURN();
}


// -------------------------------------------------------------------------
// Re-enable CPU interrupts

void
Cyg_Interrupt::enable_interrupts(void)
{
    CYG_REPORT_FUNCTION();
        
    CYG_INSTRUMENT_INTR(ENABLE, disable_counter[CYG_KERNEL_CPU_THIS()], 0);

    HAL_SMP_CPU_TYPE cpu_this = CYG_KERNEL_CPU_THIS();

    CYG_ASSERT( disable_counter[cpu_this] > 0 , "Disable counter not greater than zero");
    
    // If the disable counter goes to zero, then release the spinlock and restore
    // the previous interrupt state.
    
    if( --disable_counter[cpu_this] == 0 )
    {
        interrupt_disable_spinlock.clear_intsave(interrupt_disable_state[cpu_this]);
    }

    CYG_REPORT_RETURN();
}
    
// -------------------------------------------------------------------------
// Mask a specific interrupt in a PIC

void
Cyg_Interrupt::mask_interrupt(cyg_vector vector)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("vector=%d", vector);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    CYG_INSTRUMENT_INTR(MASK, vector, 0);

    CYG_INTERRUPT_STATE old_ints;
    
    HAL_DISABLE_INTERRUPTS(old_ints);
    HAL_INTERRUPT_MASK( vector );
    HAL_RESTORE_INTERRUPTS(old_ints);

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Mask a specific interrupt in a PIC (but not interrupt safe)

void
Cyg_Interrupt::mask_interrupt_intunsafe(cyg_vector vector)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("vector=%d", vector);


    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    CYG_INSTRUMENT_INTR(MASK, vector, 0);

    HAL_INTERRUPT_MASK( vector );

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Clear PIC mask

void
Cyg_Interrupt::unmask_interrupt(cyg_vector vector)
{
    CYG_REPORT_FUNCTION();

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");
    
    CYG_INSTRUMENT_INTR(UNMASK, vector, 0);

    CYG_INTERRUPT_STATE old_ints;
    
    HAL_DISABLE_INTERRUPTS(old_ints);
    HAL_INTERRUPT_UNMASK( vector );
    HAL_RESTORE_INTERRUPTS(old_ints);

    CYG_REPORT_RETURN();
}
    

// -------------------------------------------------------------------------
// Clear PIC mask (but not interrupt safe)

void
Cyg_Interrupt::unmask_interrupt_intunsafe(cyg_vector vector)
{
    CYG_REPORT_FUNCTION();

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");
    
    CYG_INSTRUMENT_INTR(UNMASK, vector, 0);

    HAL_INTERRUPT_UNMASK( vector );

    CYG_REPORT_RETURN();
}
    

// -------------------------------------------------------------------------
// Acknowledge interrupt at PIC

void
Cyg_Interrupt::acknowledge_interrupt(cyg_vector vector)
{
//    CYG_REPORT_FUNCTION();

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    CYG_INSTRUMENT_INTR(ACK, vector, 0);

    HAL_INTERRUPT_ACKNOWLEDGE( vector );
}

// -------------------------------------------------------------------------
// Change interrupt detection at PIC

void
Cyg_Interrupt::configure_interrupt(
    cyg_vector vector,              // vector to control
    cyg_bool level,                 // level or edge triggered
    cyg_bool up                     // hi/lo level, rising/falling edge
    )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3("vector = %d, level = %d, up = %d", vector, level,
                        up);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    CYG_INSTRUMENT_INTR(CONFIGURE, vector, (level<<1)|up);

    HAL_INTERRUPT_CONFIGURE( vector, level, up );

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// SMP support for setting/getting interrupt CPU

#ifdef CYGPKG_KERNEL_SMP_SUPPORT

void
Cyg_Interrupt::set_cpu(
    cyg_vector vector,              // vector to control
    HAL_SMP_CPU_TYPE cpu            // CPU to set
    )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2("vector = %d, cpu = %d", vector, cpu );

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    CYG_INSTRUMENT_INTR(SET_CPU, vector, cpu);

    HAL_INTERRUPT_SET_CPU( vector, cpu );

    CYG_REPORT_RETURN();
}

HAL_SMP_CPU_TYPE
Cyg_Interrupt::get_cpu(
    cyg_vector vector              // vector to control
    )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("vector = %d", vector);

    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");    
    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");

    HAL_SMP_CPU_TYPE cpu = 0;
    
    HAL_INTERRUPT_GET_CPU( vector, cpu );

    CYG_INSTRUMENT_INTR(GET_CPU, vector, cpu);
    
    CYG_REPORT_RETURN();

    return cpu;
}

#endif

// -------------------------------------------------------------------------
// EOF intr/intr.cxx
