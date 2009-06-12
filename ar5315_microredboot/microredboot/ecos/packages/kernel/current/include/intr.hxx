#ifndef CYGONCE_KERNEL_INTR_HXX
#define CYGONCE_KERNEL_INTR_HXX

//==========================================================================
//
//      intr.hxx
//
//      Interrupt class declaration(s)
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
// Purpose:     Define Interrupt class interfaces
// Description: The classes defined here provide the APIs for handling
//              interrupts.
// Usage:       #include "intr.hxx"
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>

#include <cyg/kernel/smp.hxx>

// -------------------------------------------------------------------------
// Default definitions

// Some HALs define the ISR table to be a different size to the number
// of ISR vectors. These HALs will define CYGNUM_HAL_ISR_TABLE_SIZE. All
// other HALs will have the table size equal to the number of vectors.

#ifndef CYGNUM_HAL_ISR_TABLE_SIZE
# define CYGNUM_HAL_ISR_TABLE_SIZE      CYGNUM_HAL_ISR_COUNT
#endif

// -------------------------------------------------------------------------
// Function prototype typedefs


// VSR = Vector Service Routine. This is the code attached directly to an
// interrupt vector. It is very architecture/platform specific and usually
// must be written in assembler.

typedef void  cyg_VSR();

// ISR = Interrupt Service Routine. This is called from the default
// VSR in response to an interrupt. It may access shared data but may
// not call kernel routines. The return value may be
// Cyg_Interrupt::HANDLED and/or Cyg_Interrupt::CALL_DSR.

typedef cyg_uint32 cyg_ISR(cyg_vector vector, CYG_ADDRWORD data);

// DSR = Deferred Service Routine. This is called if the ISR returns
// the Cyg_Interrupt::CALL_DSR bit. It is called at a "safe" point in
// the kernel where it may make calls on kernel routines. The count
// argument indicates how many times the ISR has asked for the DSR to
// be posted since the last time the DSR ran.

typedef void cyg_DSR(cyg_vector vector, cyg_ucount32 count, CYG_ADDRWORD data);

// -------------------------------------------------------------------------
// Include HAL definitions

class Cyg_Interrupt;

#include <cyg/hal/hal_arch.h>

#include <cyg/hal/hal_intr.h>

#ifndef HAL_INTERRUPT_STACK_CALL_PENDING_DSRS
#define HAL_INTERRUPT_STACK_CALL_PENDING_DSRS()    \
      Cyg_Interrupt::call_pending_DSRs_inner()
#endif

externC void interrupt_end(
    cyg_uint32          isr_ret,
    Cyg_Interrupt       *intr,
    HAL_SavedRegisters  *ctx
    );

externC void cyg_interrupt_post_dsr( CYG_ADDRWORD intr_obj );
externC void cyg_interrupt_call_pending_DSRs( void );

// -------------------------------------------------------------------------
// Interrupt class. This both represents each interrupt and provides a static
// interface for controlling the interrupt hardware.

class Cyg_Interrupt
{

    friend class Cyg_Scheduler;
    friend void interrupt_end( cyg_uint32,
                               Cyg_Interrupt *,
                               HAL_SavedRegisters *);
    friend void cyg_interrupt_post_dsr( CYG_ADDRWORD intr_obj );
    friend void cyg_interrupt_call_pending_DSRs( void );
    
    cyg_vector          vector;         // Interrupt vector

    cyg_priority        priority;       // Queuing priority
    
    cyg_ISR             *isr;           // Pointer to ISR

    cyg_DSR             *dsr;           // Pointer to DSR

    CYG_ADDRWORD        data;           // Data pointer


    
    // DSR handling interface called by the scheduler

                                        // Check for pending DSRs
    static cyg_bool     DSRs_pending();

                                        // Call any pending DSRs
    static void         call_pending_DSRs();
    static void         call_pending_DSRs_inner();

    // DSR handling interface called by the scheduler and HAL 
    // interrupt arbiters.

    void                post_dsr();     // Post the DSR for this interrupt


    
    // Data structures for handling DSR calls.  We implement two DSR
    // handling mechanisms, a list based one and a table based
    // one. The list based mechanism is safe with respect to temporary
    // overloads and will not run out of resource. However it requires
    // extra data per interrupt object, and interrupts must be turned
    // off briefly when delivering the DSR. The table based mechanism
    // does not need unnecessary interrupt switching, but may be prone
    // to overflow on overload. However, since a correctly programmed
    // real time application should not experience such a condition,
    // the table based mechanism is more efficient for real use. The
    // list based mechainsm is enabled by default since it is safer to
    // use during development.

#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_TABLE
    
    static Cyg_Interrupt *dsr_table[CYGNUM_KERNEL_CPU_MAX]
                                   [CYGNUM_KERNEL_INTERRUPTS_DSRS_TABLE_SIZE]
                                   CYGBLD_ANNOTATE_VARIABLE_INTR;

    static cyg_ucount32 dsr_table_head[CYGNUM_KERNEL_CPU_MAX]
                                      CYGBLD_ANNOTATE_VARIABLE_INTR;

    static volatile cyg_ucount32 dsr_table_tail[CYGNUM_KERNEL_CPU_MAX]
                                               CYGBLD_ANNOTATE_VARIABLE_INTR;

#endif
#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST

    // Number of DSR posts made
    volatile cyg_ucount32 dsr_count CYGBLD_ANNOTATE_VARIABLE_INTR; 

    // next DSR in list
    Cyg_Interrupt* volatile next_dsr CYGBLD_ANNOTATE_VARIABLE_INTR; 

    // static list of pending DSRs
    static Cyg_Interrupt* volatile dsr_list[CYGNUM_KERNEL_CPU_MAX]
                                           CYGBLD_ANNOTATE_VARIABLE_INTR;
    
#endif

#ifdef CYGIMP_KERNEL_INTERRUPTS_CHAIN

    // The default mechanism for handling interrupts is to attach just
    // one Interrupt object to each vector. In some cases, and on some
    // hardware, this is not possible, and each vector must carry a chain
    // of interrupts.

    Cyg_Interrupt       *next;          // Next Interrupt in list

    // Chaining ISR inserted in HAL vector
    static cyg_uint32 chain_isr(cyg_vector vector, CYG_ADDRWORD data);    

    // Table of interrupt chains
    static Cyg_Interrupt *chain_list[CYGNUM_HAL_ISR_TABLE_SIZE];
    
#endif

    // Interrupt disable data. Interrupt disable can be nested. On
    // each CPU this is controlled by disable_counter[cpu]. When the
    // counter is first incremented from zero to one, the
    // interrupt_disable_spinlock is claimed using spin_intsave(), the
    // original interrupt enable state being saved in
    // interrupt_disable_state[cpu].  When the counter is decremented
    // back to zero the spinlock is cleared using clear_intsave().

    // The spinlock is necessary in SMP systems since a thread
    // accessing data shared with an ISR may be scheduled on a
    // different CPU to the one that handles the interrupt. So, merely
    // blocking local interrupts would be ineffective. SMP aware
    // device drivers should either use their own spinlocks to protect
    // data, or use the API supported by this class, via
    // cyg_drv_isr_lock()/_unlock(). Note that it now becomes
    // essential that ISRs do this if they are to be SMP-compatible.

    // In a single CPU system, this mechanism reduces to just
    // disabling/enabling interrupts.

    // Disable level counter. This counts the number of times
    // interrupts have been disabled.
    static volatile cyg_int32 disable_counter[CYGNUM_KERNEL_CPU_MAX]
                                              CYGBLD_ANNOTATE_VARIABLE_INTR;

    // Interrupt disable spinlock. This is claimed by any CPU that has
    // disabled interrupts via the Cyg_Interrupt API.
    static Cyg_SpinLock interrupt_disable_spinlock CYGBLD_ANNOTATE_VARIABLE_INTR;

    // Saved interrupt state. When each CPU first disables interrupts
    // the original state of the interrupts are saved here to be
    // restored later.
    static CYG_INTERRUPT_STATE interrupt_disable_state[CYGNUM_KERNEL_CPU_MAX]
                                                       CYGBLD_ANNOTATE_VARIABLE_INTR;

    
public:

    Cyg_Interrupt                       // Initialize interrupt
    (
        cyg_vector      vector,         // Vector to attach to
        cyg_priority    priority,       // Queue priority
        CYG_ADDRWORD    data,           // Data pointer
        cyg_ISR         *isr,           // Interrupt Service Routine
        cyg_DSR         *dsr            // Deferred Service Routine
        );

    ~Cyg_Interrupt();
        
    // ISR return values
    enum {
        HANDLED  = 1,                   // Interrupt was handled
        CALL_DSR = 2                    // Schedule DSR
    };

    // Interrupt management
        
    void        attach();               // Attach to vector


    void        detach();               // Detach from vector
        
    
    // Static Interrupt management functions

    // Get the current service routine
    static void get_vsr(cyg_vector vector, cyg_VSR **vsr);

    // Install a vector service routine
    static void set_vsr(
        cyg_vector vector,              // hardware vector to replace
        cyg_VSR *vsr,                   // my new service routine
        cyg_VSR **old = NULL            // pointer to old vsr, if required
        );


    // Static interrupt masking functions

    // Disable interrupts at the CPU
    static void disable_interrupts();

    // Re-enable CPU interrupts
    static void enable_interrupts();

    // Are interrupts enabled at the CPU?
    static inline cyg_bool interrupts_enabled()
    {
        return (0 == disable_counter[CYG_KERNEL_CPU_THIS()]);
    }
    
    // Get the vector for the following calls
    inline cyg_vector get_vector() 
    {
        return vector;
    }
    
    // Static PIC control functions
    
    // Mask a specific interrupt in a PIC
    static void mask_interrupt(cyg_vector vector);
    // The same but not interrupt safe
    static void mask_interrupt_intunsafe(cyg_vector vector);

    // Clear PIC mask
    static void unmask_interrupt(cyg_vector vector);
    // The same but not interrupt safe
    static void unmask_interrupt_intunsafe(cyg_vector vector);

    // Acknowledge interrupt at PIC
    static void acknowledge_interrupt(cyg_vector vector);

    // Change interrupt detection at PIC
    static void configure_interrupt(
        cyg_vector vector,              // vector to control
        cyg_bool level,                 // level or edge triggered
        cyg_bool up                     // hi/lo level, rising/falling edge
        );

#ifdef CYGPKG_KERNEL_SMP_SUPPORT

    // SMP support for associating an interrupt with a specific CPU.
    
    static void set_cpu( cyg_vector, HAL_SMP_CPU_TYPE cpu );
    static HAL_SMP_CPU_TYPE get_cpu( cyg_vector );
    
#endif    
};

#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS
// -------------------------------------------------------------------------
// Check for pending DSRs

inline cyg_bool Cyg_Interrupt::DSRs_pending()
{
    HAL_SMP_CPU_TYPE cpu = CYG_KERNEL_CPU_THIS();
#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_TABLE
    
    return dsr_table_head[cpu] != dsr_table_tail[cpu];

#endif
#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST
    
    return dsr_list[cpu] != NULL;

#endif
};
#endif // CYGIMP_KERNEL_INTERRUPTS_DSRS

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_INTR_HXX
// EOF intr.hxx
