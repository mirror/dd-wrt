//=============================================================================
//
//      synth_intr.c
//
//      Interrupt and clock code for the Linux synthetic target.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Bart Veer
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    bartv
// Contributors: bartv, asl
// Date:         2001-03-30
// Purpose:      Implement the interrupt subsystem for the synthetic target
//####DESCRIPTIONEND####
//=============================================================================

// sigprocmask handling.
//
// In the synthetic target interrupts and exceptions are based around
// POSIX sighandlers. When the clock ticks a SIGALRM signal is raised.
// When the I/O auxiliary wants to raise some other interrupt, it
// sends a SIGIO signal. When an exception occurs this results in
// signals like SIGILL and SIGSEGV. This implies an implementation
// where the VSR is the signal handler. Disabling interrupts would
// then mean using sigprocmask() to block certain signals, and
// enabling interrupts means unblocking those signals.
//
// However there are a few problems. One of these is performance: some
// bits of the system such as buffered tracing make very extensive use
// of enabling and disabling interrupts, so making a sigprocmask
// system call each time adds a lot of overhead. More seriously, there
// is a subtle discrepancy between POSIX signal handling and hardware
// interrupts. Signal handlers are expected to return, and then the
// system automatically passes control back to the foreground thread.
// In the process, the sigprocmask is manipulated before invoking the
// signal handler and restored afterwards. Interrupt handlers are
// different: it is quite likely that an interrupt results in another
// eCos thread being activated, so the signal handler does not
// actually return until the interrupted thread gets another chance to
// run. 
//
// The second problem can be addressed by making the sigprocmask part
// of the thread state, saving and restoring it as part of a context
// switch (c.f. siglongjmp()). This matches quite nicely onto typical
// real hardware, where there might be a flag inside some control
// register that controls whether or not interrupts are enabled.
// However this adds more system calls to context switch overhead.
//
// The alternative approach is to implement interrupt enabling and
// disabling in software. The sigprocmask is manipulated only once,
// during initialization, such that certain signals are allowed
// through and others are blocked. When a signal is raised the signal
// handler will always be invoked, but it will decide in software
// whether or not the signal should be processed immediately. This
// alternative approach does not correspond particularly well with
// real hardware: effectively the VSR is always allowed to run.
// However for typical applications this will not really matter, and
// the performance gains outweigh the discrepancy.
//
// Nested interrupts and interrupt priorities can be implemented in
// software, specifically by manipulating the current mask of blocked
// interrupts. This is not currently implemented.
//
// At first glance it might seem that an interrupt stack could be
// implemented trivially using sigaltstack. This does not quite work:
// signal handlers do not always return immediately, so the system
// does not get a chance to clean up the signal handling stack. A
// separate interrupt stack is possible but would have to be
// implemented here, in software, e.g. by having the signal handler
// invoke the VSR on that stack. Unfortunately the system may have
// pushed quite a lot of state on to the current stack already when
// raising the signal, so things could get messy.

// ----------------------------------------------------------------------------
#include <pkgconf/hal.h>
#include <pkgconf/hal_synth.h>

// There are various dependencies on the kernel, e.g. how exceptions
// should be handled.
#include <pkgconf/system.h>
#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_io.h>
#include <cyg/infra/cyg_ass.h>          // Assertions are safe in the synthetic target

#include "synth_protocol.h"

// ----------------------------------------------------------------------------
// Statics.

// Are interrupts currently enabled?
volatile cyg_bool_t hal_interrupts_enabled = false;

// These flags are updated by the signal handler when a signal comes in
// and interrupts are disabled.
static volatile cyg_bool_t  synth_sigio_pending         = false;
static volatile cyg_bool_t  synth_sigalrm_pending       = false;

// The current VSR, to be invoked by the signal handler. This allows
// application code to install an alternative VSR, without that VSR
// having to check for interrupts being disabled and updating the
// pending flags. Effectively, the VSR is only invoked when interrupts
// are enabled.
static void (*synth_VSR)(void)                          = (void (*)(void)) 0;

// The current ISR status and mask registers, or rather software
// emulations thereof. These are not static since application-specific
// VSRs may want to examine/manipulate these. They are also not
// exported in any header file, forcing people writing such VSRs to
// know what they are doing.
volatile cyg_uint32 synth_pending_isrs      = 0;
volatile cyg_uint32 synth_masked_isrs       = 0xFFFFFFFF;

// The vector of interrupt handlers.
typedef struct synth_isr_handler {
    cyg_ISR_t*          isr;
    CYG_ADDRWORD        data;
    CYG_ADDRESS         obj;
    cyg_priority_t      pri;
} synth_isr_handler;
static synth_isr_handler synth_isr_handlers[CYGNUM_HAL_ISR_COUNT];

static void  synth_alrm_sighandler(int);
static void  synth_io_sighandler(int);

// ----------------------------------------------------------------------------
// Basic ISR and VSR handling.

// The default ISR handler. The system should never receive an interrupt it
// does not know how to handle.
static cyg_uint32
synth_default_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    CYG_UNUSED_PARAM(cyg_vector_t, vector);
    CYG_UNUSED_PARAM(cyg_addrword_t, data);
    CYG_FAIL("Default isr handler should never get invoked");
    return CYG_ISR_HANDLED;
}

// The VSR is invoked
//  1) directly by a SIGALRM or SIGIO signal handler, if interrupts
//     were enabled.
//  2) indirectly by hal_enable_interrupts(), if a signal happened
//     while interrupts were disabled. hal_enable_interrupts()
//     will have re-invoked the signal handler.
//
// On entry interrupts are disabled, and there should be one or more
// pending ISRs which are not masked off.
//
// The implementation is as per the HAL specification, where
// applicable.

static void
synth_default_vsr(void)
{
    int         isr_vector;
    cyg_uint32  isr_result;

    CYG_ASSERT(!hal_interrupts_enabled, "VSRs should only be invoked when interrupts are disabled");
    CYG_ASSERT(0 != (synth_pending_isrs & ~synth_masked_isrs), "VSRs should only be invoked when an interrupt is pending");

    // No need to save the cpu state. Either we are in a signal
    // handler and the system has done that for us, or we are called
    // synchronously via enable_interrupts.

    // Increment the kernel scheduler lock, if the kernel is present.
    // This prevents context switching while interrupt handling is in
    // progress.
#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
    cyg_scheduler_lock();
#endif

    // Do not switch to an interrupt stack - functionality is not
    // implemented

    // Do not allow nested interrupts - functionality is not
    // implemented.

    // Decode the actual external interrupt being delivered. This is
    // determined from the pending and masked variables. Only one isr
    // source can be handled here, since interrupt_end must be invoked
    // with details of that interrupt. Multiple pending interrupts
    // will be handled by a recursive call 
    HAL_LSBIT_INDEX(isr_vector, (synth_pending_isrs & ~synth_masked_isrs));
    CYG_ASSERT((CYGNUM_HAL_ISR_MIN <= isr_vector) && (isr_vector <= CYGNUM_HAL_ISR_MAX), "ISR vector must be valid");

    isr_result = (*synth_isr_handlers[isr_vector].isr)(isr_vector, synth_isr_handlers[isr_vector].data);

    // Do not switch back from the interrupt stack, there isn't one.
    
    // Interrupts were not enabled before, so they must be enabled
    // now. This may result in a recursive invocation if other IRQs
    // are still pending. The ISR should have either acknowledged or
    // masked the current interrupt source, to prevent a recursive
    // call for the current interrupt.
    hal_enable_interrupts();

    // Now call interrupt_end() with the result of the isr and the
    // ISR's object This may return straightaway, or it may result in
    // a context switch to another thread. In the latter case, when
    // the current thread is reactivated we end up back here. The
    // third argument should be a pointer to the saved state, but that
    // is only relevant for thread-aware debugging which is not yet
    // supported by the synthetic target.
    {
        extern void interrupt_end(cyg_uint32, CYG_ADDRESS, HAL_SavedRegisters*);
        interrupt_end(isr_result, synth_isr_handlers[isr_vector].obj, (HAL_SavedRegisters*) 0);
    }

    // Restore machine state and return to the interrupted thread.
    // That requires no effort here.
}

// Enabling interrupts. If a SIGALRM or SIGIO arrived at an inconvenient
// time, e.g. when already interacting with the auxiliary, then these
// will have been left pending and must be serviced now. Next, enabling
// interrupts means checking the interrupt pending and mask registers
// and seeing if the VSR should be invoked.
void
hal_enable_interrupts(void)
{
    hal_interrupts_enabled = true;
    if (synth_sigalrm_pending) {
        synth_sigalrm_pending = false;
        synth_alrm_sighandler(CYG_HAL_SYS_SIGALRM);
    }
    if (synth_sigio_pending) {
        synth_sigio_pending = false;
        synth_io_sighandler(CYG_HAL_SYS_SIGIO);
    }

    // The interrupt mask "register" may have been modified while
    // interrupts were disabled. If there are pending interrupts,
    // invoke the VSR. The VSR must be invoked with interrupts
    // disabled, and will return with interrupts enabled.
    // An alternative implementation that might be more accurate
    // is to raise a signal, e.g. SIGUSR1. That way all interrupts
    // come in via the system's signal handling mechanism, and
    // it might be possible to do something useful with saved contexts
    // etc., facilitating thread-aware debugging.
    if (0 != (synth_pending_isrs & ~synth_masked_isrs)) {
        hal_interrupts_enabled = false;
        (*synth_VSR)();
        CYG_ASSERT( hal_interrupts_enabled, "Interrupts should still be enabled on return from the VSR");
    }
}

// ----------------------------------------------------------------------------
// Other interrupt-related routines. Mostly these just involve
// updating some of the statics, but they may be called while
// interrupts are still enabled so care has to be taken.

cyg_bool_t
hal_interrupt_in_use(cyg_vector_t vec)
{
    CYG_ASSERT( (CYGNUM_HAL_ISR_MIN <= vec) && (vec <= CYGNUM_HAL_ISR_MAX), "Can only attach to valid ISR vectors");
    return synth_default_isr != synth_isr_handlers[vec].isr;
}

void
hal_interrupt_attach(cyg_vector_t vec, cyg_ISR_t* isr, CYG_ADDRWORD data, CYG_ADDRESS obj)
{
    CYG_ASSERT( (CYGNUM_HAL_ISR_MIN <= vec) && (vec <= CYGNUM_HAL_ISR_MAX), "Can only attach to valid ISR vectors");
    CYG_CHECK_FUNC_PTR( isr, "A valid ISR must be supplied");
    // The object cannot be validated, it may be NULL if chained
    // interrupts are enabled.
    CYG_ASSERT( synth_isr_handlers[vec].isr == &synth_default_isr, "Only one ISR can be attached to a vector at the HAL level");
    CYG_ASSERT( (false == hal_interrupts_enabled) || (0 != (synth_masked_isrs & (0x01 << vec))), "ISRs should only be attached when it is safe");

    // The priority will have been installed shortly before this call.
    synth_isr_handlers[vec].isr     = isr;
    synth_isr_handlers[vec].data    = data;
    synth_isr_handlers[vec].obj     = obj;
}

void
hal_interrupt_detach(cyg_vector_t vec, cyg_ISR_t* isr)
{
    CYG_ASSERT( (CYGNUM_HAL_ISR_MIN <= vec) && (vec <= CYGNUM_HAL_ISR_MAX), "Can only detach from valid ISR vectors");
    CYG_CHECK_FUNC_PTR( isr, "A valid ISR must be supplied");
    CYG_ASSERT( isr != &synth_default_isr, "An ISR must be attached before it can be detached");
    CYG_ASSERT( (false == hal_interrupts_enabled) || (0 != (synth_masked_isrs & (0x01 << vec))), "ISRs should only be detached when it is safe");

    // The Cyg_Interrupt destructor does an unconditional detach, even if the
    // isr is not currently attached.
    if (isr == synth_isr_handlers[vec].isr) {
        synth_isr_handlers[vec].isr     = &synth_default_isr;
        synth_isr_handlers[vec].data    = (CYG_ADDRWORD) 0;
        synth_isr_handlers[vec].obj     = (CYG_ADDRESS) 0;
    }
    
    // The priority is not updated here. This should be ok, if another
    // isr is attached then the appropriate priority will be installed
    // first.
}

void (*hal_vsr_get(cyg_vector_t vec))(void)
{
    CYG_ASSERT( (CYGNUM_HAL_VSR_MIN <= vec) && (vec <= CYGNUM_HAL_VSR_MAX), "Can only get valid VSR vectors");
    return synth_VSR;
}

void
hal_vsr_set(cyg_vector_t vec, void (*new_vsr)(void), void (**old_vsrp)(void))
{
    cyg_bool_t  old;

    CYG_ASSERT( (CYGNUM_HAL_VSR_MIN <= vec) && (vec <= CYGNUM_HAL_VSR_MAX), "Can only get valid VSR vectors");
    CYG_CHECK_FUNC_PTR( new_vsr, "A valid VSR must be supplied");

    // There is a theoretical possibility of two hal_vsr_set calls at
    // the same time. The old and new VSRs must be kept in synch.
    HAL_DISABLE_INTERRUPTS(old);
    if (0 != old_vsrp) {
        *old_vsrp = synth_VSR;
    }
    synth_VSR = new_vsr;
    HAL_RESTORE_INTERRUPTS(old);
}

void
hal_interrupt_mask(cyg_vector_t which)
{
    CYG_PRECONDITION( !hal_interrupts_enabled, "Interrupts should be disabled on entry to hal_interrupt_mask");
    CYG_ASSERT((CYGNUM_HAL_ISR_MIN <= which) && (which <= CYGNUM_HAL_ISR_MAX), "A valid ISR vector must be supplied");
    synth_masked_isrs |= (0x01 << which);
}

void
hal_interrupt_unmask(cyg_vector_t which)
{
    CYG_PRECONDITION( !hal_interrupts_enabled, "Interrupts should be disabled on entry to hal_interrupt_unmask");
    CYG_ASSERT((CYGNUM_HAL_ISR_MIN <= which) && (which <= CYGNUM_HAL_ISR_MAX), "A valid ISR vector must be supplied");
    synth_masked_isrs &= ~(0x01 << which);
}

void
hal_interrupt_acknowledge(cyg_vector_t which)
{
    cyg_bool_t old;
    CYG_ASSERT((CYGNUM_HAL_ISR_MIN <= which) && (which <= CYGNUM_HAL_ISR_MAX), "A valid ISR vector must be supplied");

    // Acknowledging an interrupt means clearing the bit in the
    // interrupt pending "register".
    // NOTE: does the auxiliary need to keep track of this? Probably
    // not, the auxiliary can just raise SIGIO whenever a device wants
    // attention. There may be a trade off here between additional
    // communication and unnecessary SIGIOs.
    HAL_DISABLE_INTERRUPTS(old);
    synth_pending_isrs &= ~(0x01 << which);
    HAL_RESTORE_INTERRUPTS(old);
}

void
hal_interrupt_configure(cyg_vector_t which, cyg_bool_t level, cyg_bool_t up)
{
    CYG_ASSERT((CYGNUM_HAL_ISR_MIN <= which) && (which <= CYGNUM_HAL_ISR_MAX), "A valid ISR vector must be supplied");
    // The synthetic target does not currently distinguish between
    // level and edge interrupts. Possibly this information will have
    // to be passed on to the auxiliary in future.
    CYG_UNUSED_PARAM(cyg_vector_t, which);
    CYG_UNUSED_PARAM(cyg_bool_t, level);
    CYG_UNUSED_PARAM(cyg_bool_t, up);
}

void
hal_interrupt_set_level(cyg_vector_t which, cyg_priority_t level)
{
    CYG_ASSERT((CYGNUM_HAL_ISR_MIN <= which) && (which <= CYGNUM_HAL_ISR_MAX), "A valid ISR vector must be supplied");
    // The legal values for priorities are not defined at this time.
    // Manipulating the interrupt priority level currently has no
    // effect. The information is stored anyway, for future use.
    synth_isr_handlers[which].pri = level;
}

// ----------------------------------------------------------------------------
// Exception handling. Typically this involves calling into the kernel,
// translating the POSIX signal number into a HAL exception number. In
// practice these signals will generally be caught in the debugger and
// will not have to be handled by eCos itself.

static void
synth_exception_sighandler(int sig)
{
    CYG_WORD    ecos_exception_number = 0;
    cyg_bool_t  old;

    // There is no need to save state, that will have been done by the
    // system as part of the signal delivery process.
    
    // Disable interrupts. Performing e.g. an interaction with the
    // auxiliary after a SIGSEGV is dubious.
    HAL_DISABLE_INTERRUPTS(old);

    // Now decode the signal and turn it into an eCos exception.
    switch(sig) {
      case CYG_HAL_SYS_SIGILL:
        ecos_exception_number = CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION;
        break;
      case CYG_HAL_SYS_SIGBUS:
      case CYG_HAL_SYS_SIGSEGV:
        ecos_exception_number = CYGNUM_HAL_EXCEPTION_DATA_ACCESS;
        break;
      case CYG_HAL_SYS_SIGFPE:
        ecos_exception_number = CYGNUM_HAL_EXCEPTION_FPU;
        break;
      default:
        CYG_FAIL("Unknown signal");
        break;
    }

#ifdef CYGPKG_KERNEL_EXCEPTIONS
    // Deliver the signal, usually to the kernel, possibly to the
    // common HAL. The second argument should be the current
    // savestate, but that is not readily accessible.
    cyg_hal_deliver_exception(ecos_exception_number, (CYG_ADDRWORD) 0);

    // It is now necessary to restore the machine state, including
    // interrupts. In theory higher level code may have manipulated
    // the machine state to prevent any recurrence of the exception.
    // In practice the machine state is not readily accessible.
    HAL_RESTORE_INTERRUPTS(old);
#else
    CYG_FAIL("Exception!!!");
    for (;;);
#endif    
}

// ----------------------------------------------------------------------------
// The clock support. This can be implemented using the setitimer()
// and getitimer() calls. The kernel will install a suitable interrupt
// handler for CYGNUM_HAL_INTERRUPT_RTC, but it depends on the HAL
// for low-level manipulation of the clock hardware.
//
// There is a problem with HAL_CLOCK_READ(). The obvious
// implementation would use getitimer(), but that has the wrong
// behaviour: it is intended for fairly coarse intervals and works in
// terms of system clock ticks, as opposed to a fine-grained
// implementation that actually examines the system clock. Instead use
// gettimeofday().

static struct cyg_hal_sys_timeval synth_clock   = { 0, 0 };

void
hal_clock_initialize(cyg_uint32 period)
{
    struct cyg_hal_sys_itimerval    timer;

    // Needed for hal_clock_read(), if HAL_CLOCK_READ() is used before
    // the first clock interrupt.
    cyg_hal_sys_gettimeofday(&synth_clock, (struct cyg_hal_sys_timezone*) 0);
    
    // The synthetic target clock resolution is in microseconds. A typical
    // value for the period will be 10000, corresponding to one timer
    // interrupt every 10ms. Set up a timer to interrupt in period us,
    // and again every period us after that.
    CYG_ASSERT( period < 1000000, "Clock interrupts should happen at least once per second");
    timer.hal_it_interval.hal_tv_sec    = 0;
    timer.hal_it_interval.hal_tv_usec   = period;
    timer.hal_it_value.hal_tv_sec       = 0;
    timer.hal_it_value.hal_tv_usec      = period;
    
    if (0 != cyg_hal_sys_setitimer(CYG_HAL_SYS_ITIMER_REAL, &timer, (struct cyg_hal_sys_itimerval*) 0)) {
        CYG_FAIL("Failed to initialize the clock itimer");
    }
}

static void
synth_alrm_sighandler(int sig)
{
    CYG_PRECONDITION((CYG_HAL_SYS_SIGALRM == sig), "Only SIGALRM should be handled here");
    
    if (!hal_interrupts_enabled) {
        synth_sigalrm_pending = true;
        return;
    }

    // Interrupts were enabled, but must be blocked before any further processing.
    hal_interrupts_enabled = false;

    // Update the cached value of the clock for hal_clock_read()
    cyg_hal_sys_gettimeofday(&synth_clock, (struct cyg_hal_sys_timezone*) 0);

    // Update the interrupt status "register" to match pending interrupts
    // A timer signal means that IRQ 0 needs attention.
    synth_pending_isrs |= 0x01;

    // If any of the pending interrupts are not masked, invoke the
    // VSR. That will reenable interrupts.
    if (0 != (synth_pending_isrs & ~synth_masked_isrs)) {
        (*synth_VSR)();
    } else {
        hal_interrupts_enabled = true;
    }

    // The VSR will have invoked interrupt_end() with interrupts
    // enabled, and they should still be enabled.
    CYG_ASSERT( hal_interrupts_enabled, "Interrupts should still be enabled on return from the VSR");
}

// Implementing hal_clock_read(). gettimeofday() in conjunction with
// synth_clock gives the time since the last clock tick in
// microseconds, the correct unit for the synthetic target.
cyg_uint32
hal_clock_read(void)
{
    int elapsed;
    struct cyg_hal_sys_timeval  now;
    cyg_hal_sys_gettimeofday(&now, (struct cyg_hal_sys_timezone*) 0);

    elapsed = (1000000 * (now.hal_tv_sec - synth_clock.hal_tv_sec)) + (now.hal_tv_usec - synth_clock.hal_tv_usec);
    return elapsed;
}

// ----------------------------------------------------------------------------
// The signal handler for SIGIO. This can also be invoked by
// hal_enable_interrupts() to catch up with any signals that arrived
// while interrupts were disabled. SIGIO is raised by the auxiliary
// when it requires attention, i.e. when one or more of the devices
// want to raise an interrupt. Finding out exactly which interrupt(s)
// are currently pending in the auxiliary requires communication with
// the auxiliary.
//
// If interrupts are currently disabled then the signal cannot be
// handled immediately. In particular SIGIO cannot be handled because
// there may already be ongoing communication with the auxiliary.
// Instead some volatile flags are used to keep track of which signals
// were raised while interrupts were disabled. 
//
// It might be better to perform the interaction with the auxiliary
// as soon as possible, i.e. either in the SIGIO handler or when the
// current communication completes. That way the mask of pending
// interrupts would remain up to date even when interrupts are
// disabled, thus allowing applications to run in polled mode.

// A little utility called when the auxiliary has been asked to exit,
// implicitly affecting this application as well. The sole purpose
// of this function is to put a suitably-named function on the stack
// to make it more obvious from inside gdb what is happening.
static void
synth_io_handle_shutdown_request_from_auxiliary(void)
{
    cyg_hal_sys_exit(0);
}

static void
synth_io_sighandler(int sig)
{
    CYG_PRECONDITION((CYG_HAL_SYS_SIGIO == sig), "Only SIGIO should be handled here");
    
    if (!hal_interrupts_enabled) {
        synth_sigio_pending = true;
        return;
    }
    
    // Interrupts were enabled, but must be blocked before any further processing.
    hal_interrupts_enabled = false;

    // Update the interrupt status "register" to match pending interrupts
    // Contact the auxiliary to find out what interrupts are currently pending there.
    // If there is no auxiliary at present, e.g. because it has just terminated
    // and things are generally somewhat messy, ignore it.
    //
    // This code also deals with the case where the user has requested program
    // termination. It would be wrong for the auxiliary to just exit, since the
    // application could not distinguish that case from a crash. Instead the
    // auxiliary can optionally return an additional byte of data, and if that
    // byte actually gets sent then that indicates pending termination.
    if (synth_auxiliary_running) {
        int             result;
        int             actual_len;
        unsigned char   dummy[1];
        synth_auxiliary_xchgmsg(SYNTH_DEV_AUXILIARY, SYNTH_AUXREQ_GET_IRQPENDING, 0, 0,
                                (const unsigned char*) 0, 0,        // The auxiliary does not need any additional data
                                &result, dummy, &actual_len, 1);
        synth_pending_isrs |= result;
        if (actual_len) {
            // The auxiliary has been asked to terminate by the user. This
            // request has now been passed on to the eCos application.
            synth_io_handle_shutdown_request_from_auxiliary();
        }
    }

    // If any of the pending interrupts are not masked, invoke the VSR
    if (0 != (synth_pending_isrs & ~synth_masked_isrs)) {
        (*synth_VSR)();
    } else {
        hal_interrupts_enabled = true;
    }

    // The VSR will have invoked interrupt_end() with interrupts
    // enabled, and they should still be enabled.
    CYG_ASSERT( hal_interrupts_enabled, "Interrupts should still be enabled on return from the VSR");
}

// ----------------------------------------------------------------------------
// Here we define an action to do in the idle thread. For the
// synthetic target it makes no sense to spin eating processor time
// that other processes could make use of. Instead we call select. The
// itimer will still go off and kick the scheduler back into life,
// giving us an escape path from the select. There is one problem: in
// some configurations, e.g. when preemption is disabled, the idle
// thread must yield continuously rather than blocking.
void
hal_idle_thread_action(cyg_uint32 loop_count)
{
#ifndef CYGIMP_HAL_IDLE_THREAD_SPIN
    cyg_hal_sys__newselect(0,
                           (struct cyg_hal_sys_fd_set*) 0,
                           (struct cyg_hal_sys_fd_set*) 0,
                           (struct cyg_hal_sys_fd_set*) 0,
                           (struct cyg_hal_sys_timeval*) 0);
#endif
    CYG_UNUSED_PARAM(cyg_uint32, loop_count);
}

// ----------------------------------------------------------------------------
// The I/O auxiliary.
//
// I/O happens via an auxiliary process. During startup this code attempts
// to locate and execute a program ecosynth which should be installed in
// ../libexec/ecosynth relative to some directory on the search path.
// Subsequent I/O operations involve interacting with this auxiliary.

#define MAKESTRING1(a) #a
#define MAKESTRING2(a) MAKESTRING1(a)
#define AUXILIARY       "../libexec/ecos/hal/synth/arch/" MAKESTRING2(CYGPKG_HAL_SYNTH) "/ecosynth"

// Is the auxiliary up and running?
cyg_bool    synth_auxiliary_running   = false;

// The pipes to and from the auxiliary.
static int  to_aux      = -1;
static int  from_aux    = -1;

// Attempt to start up the auxiliary. Note that this happens early on
// during system initialization so it is "known" that the world is
// still simple, e.g. that no other files have been opened.
static void
synth_start_auxiliary(void)
{
#define BUFSIZE 256
    char        filename[BUFSIZE];
    const char* path = 0;
    int         i, j;
    cyg_bool    found   = false;
    int         to_aux_pipe[2];
    int         from_aux_pipe[2];
    int         child;
    int         aux_version;
#if 1
    // Check for a command line argument -io. Only run the auxiliary if this
    // argument is provided, i.e. default to traditional behaviour.
    for (i = 1; i < cyg_hal_sys_argc; i++) {
        const char* tmp = cyg_hal_sys_argv[i];
        if ('-' == *tmp) {
            // Arguments beyond -- are reserved for use by the application,
            // and should not be interpreted by the HAL itself or by ecosynth.
            if (('-' == tmp[1]) && ('\0' == tmp[2])) {
                break;
            }
            tmp++;
            if ('-' == *tmp) {
                // Do not distinguish between -io and --io
                tmp++;
            }
            if (('i' == tmp[0]) && ('o' == tmp[1]) && ('\0' == tmp[2])) {
                found = 1;
                break;
            }
        }
    }
    if (!found) {
        return;
    }
#else
    // Check for a command line argument -ni or -nio. Always run the
    // auxiliary unless this argument is given, i.e. default to full
    // I/O support.
    for (i = 1; i < cyg_hal_sys_argc; i++) {
        const char* tmp = cyg_hal_sys_argv[i];
        if ('-' == *tmp) {
            if (('-' == tmp[1]) && ('\0' == tmp[2])) {
                break;
            }
            tmp++;
            if ('-' == *tmp) {
                tmp++;
            }
            if ('n' == *tmp) {
                tmp++;
                if ('i' == *tmp) {
                    tmp++;
                    if ('\0' == *tmp) {
                        found = 1;  // -ni or --ni
                        break;
                    }
                    if (('o' == *tmp) && ('\0' == tmp[1])) {
                        found = 1;  // -nio or --nio
                        break;
                    }
                }
            }
        }
    }
    if (found) {
        return;
    }
#endif
    
    // The auxiliary must be found relative to the current search path,
    // so look for a PATH= environment variable.
    for (i = 0; (0 == path) && (0 != cyg_hal_sys_environ[i]); i++) {
        const char *var = cyg_hal_sys_environ[i];
        if (('P' == var[0]) && ('A' == var[1]) && ('T' == var[2]) && ('H' == var[3]) && ('=' == var[4])) {
            path = var + 5;
        }
    }
    if (0 == path) {
        // Very unlikely, but just in case.
        path = ".:/bin:/usr/bin";
    }

    found = 0;
    while (!found && ('\0' != *path)) {         // for every entry in the path
        char *tmp = AUXILIARY;
        
        j = 0;

        // As a special case, an empty string in the path corresponds to the
        // current directory.
        if (':' == *path) {
            filename[j++] = '.';
            path++;
        } else {
            while ((j < BUFSIZE) && ('\0' != *path) && (':' != *path)) {
                filename[j++] = *path++;
            }
            // If not at the end of the search path, move on to the next entry.
            if ('\0' != *path) {
                while ((':' != *path) && ('\0' != *path)) {
                    path++;
                }
                if (':' == *path) {
                    path++;
                }
            }
        }
        // Now append a directory separator, and then the name of the executable.
        if (j < BUFSIZE) {
            filename[j++] = '/';
        }
        while ((j < BUFSIZE) && ('\0' != *tmp)) {
            filename[j++] = *tmp++;
        }
        // If there has been a buffer overflow, skip this entry.
        if (j == BUFSIZE) {
            filename[BUFSIZE-1] = '\0';
            diag_printf("Warning: buffer limit reached while searching PATH for the I/O auxiliary.\n");
            diag_printf("       : skipping current entry.\n");
        } else {
            // filename now contains a possible match for the auxiliary.
            filename[j++]    = '\0';
            if (0 == cyg_hal_sys_access(filename, CYG_HAL_SYS_X_OK)) {
                found = true;
            }
        }
    }
#undef BUFSIZE

    if (!found) {
        diag_printf("Error: unable to find the I/O auxiliary program on the current search PATH\n");
        diag_printf("     : please install the appropriate host-side tools.\n");
        cyg_hal_sys_exit(1);
    }

    // An apparently valid executable exists (or at the very least it existed...),
    // so create the pipes that will be used for communication.
    if ((0 != cyg_hal_sys_pipe(to_aux_pipe)) ||
        (0 != cyg_hal_sys_pipe(from_aux_pipe))) {
        diag_printf("Error: unable to set up pipes for communicating with the I/O auxiliary.\n");
        cyg_hal_sys_exit(1);
    }
    
    // Time to fork().
    child = cyg_hal_sys_fork();
    if (child < 0) {
        diag_printf("Error: failed to fork() process for the I/O auxiliary.\n");
        cyg_hal_sys_exit(1);
    } else if (child == 0) {
        cyg_bool    found_dotdot;
        // There should not be any problems rearranging the file descriptors as desired...
        cyg_bool    unexpected_error = 0;
        
        // In the child process. Close unwanted file descriptors, then some dup2'ing,
        // and execve() the I/O auxiliary. The auxiliary will inherit stdin,
        // stdout and stderr from the eCos application, so that functions like
        // printf() work as expected. In addition fd 3 will be the pipe from
        // the eCos application and fd 4 the pipe to the application. It is possible
        // that the eCos application was run from some process other than a shell
        // and hence that file descriptors 3 and 4 are already in use, but that is not
        // supported. One possible workaround would be to close all file descriptors
        // >= 3, another would be to munge the argument vector passing the file
        // descriptors actually being used.
        unexpected_error |= (0 != cyg_hal_sys_close(to_aux_pipe[1]));
        unexpected_error |= (0 != cyg_hal_sys_close(from_aux_pipe[0]));
        
        if (3 != to_aux_pipe[0]) {
            if (3 == from_aux_pipe[1]) {
                // Because to_aux_pipe[] was set up first it should have received file descriptors 3 and 4.
                diag_printf("Internal error: file descriptors have been allocated in an unusual order.\n");
                cyg_hal_sys_exit(1);
            } else {
                unexpected_error |= (3 != cyg_hal_sys_dup2(to_aux_pipe[0], 3));
                unexpected_error |= (0 != cyg_hal_sys_close(to_aux_pipe[0]));
            }
        }
        if (4 != from_aux_pipe[1]) {
            unexpected_error |= (4 != cyg_hal_sys_dup2(from_aux_pipe[1], 4));
            unexpected_error |= (0 != cyg_hal_sys_close(from_aux_pipe[1]));
        }
        if (unexpected_error) {
            diag_printf("Error: internal error in auxiliary process, failed to manipulate pipes.\n");
            cyg_hal_sys_exit(1);
        }
        // The arguments passed to the auxiliary are mostly those for
        // the synthetic target application, except for argv[0] which
        // is replaced with the auxiliary's pathname. The latter
        // currently holds at least one ../, and cleaning this up is
        // useful.
        //
        // If the argument vector contains -- then that and subsequent
        // arguments are not passed to the auxiliary. Instead such
        // arguments are reserved for use by the application.
        do {
            int len;
            for (len = 0; '\0' != filename[len]; len++)
                ;
            found_dotdot = false;
            for (i = 0; i < (len - 4); i++) {
                if (('/' == filename[i]) && ('.' == filename[i+1]) && ('.' == filename[i+2]) && ('/' == filename[i+3])) {
                    j = i + 3;
                    for ( --i; (i >= 0) && ('/' != filename[i]); i--) {
                        CYG_EMPTY_STATEMENT;
                    }
                    if (i >= 0) {
                        found_dotdot = true;
                        do {
                            i++; j++;
                            filename[i] = filename[j];
                        } while ('\0' != filename[i]);
                    }
                }
            }
        } while(found_dotdot);
        
        cyg_hal_sys_argv[0] = filename;

        for (i = 1; i < cyg_hal_sys_argc; i++) {
            const char* tmp = cyg_hal_sys_argv[i];
            if (('-' == tmp[0]) && ('-' == tmp[1]) && ('\0' == tmp[2])) {
                cyg_hal_sys_argv[i] = (const char*) 0;
                break;
            }
        }
        cyg_hal_sys_execve(filename, cyg_hal_sys_argv, cyg_hal_sys_environ);

        // An execute error has occurred. Report this here, then exit. The
        // parent will detect a close on the pipe without having received
        // any data, and it will assume that a suitable diagnostic will have
        // been displayed already.
        diag_printf("Error: failed to execute the I/O auxiliary.\n");
        cyg_hal_sys_exit(1);
    } else {
        int     rc;
        char    buf[1];
        
        // Still executing the eCos application.
        // Do some cleaning-up.
        to_aux      = to_aux_pipe[1];
        from_aux    = from_aux_pipe[0];
        if ((0 != cyg_hal_sys_close(to_aux_pipe[0]))  ||
            (0 != cyg_hal_sys_close(from_aux_pipe[1]))) {
            diag_printf("Error: internal error in main process, failed to manipulate pipes.\n");
            cyg_hal_sys_exit(1);
        }

        // It is now a good idea to block until the auxiliary is
        // ready, i.e. give it a chance to read in its configuration
        // files, load appropriate scripts, pop up windows, ... This
        // may take a couple of seconds or so. Once the auxiliary is
        // ready it will write a single byte down the pipe. This is
        // the only time that the auxiliary will write other than when
        // responding to a request.
        do {
            rc = cyg_hal_sys_read(from_aux, buf, 1);
        } while (-CYG_HAL_SYS_EINTR == rc);

        if (1 != rc) {
            // The auxiliary has not started up successfully, so exit
            // immediately. It should have generated appropriate
            // diagnostics.
            cyg_hal_sys_exit(1);
        }
    }

    // At this point the auxiliary is up and running. It should not
    // generate any interrupts just yet since none of the devices have
    // been initialized. Remember that the auxiliary is now running,
    // so that the initialization routines for those devices can
    // figure out that they should interact with the auxiliary rather
    // than attempt anything manually.
    synth_auxiliary_running   = true;

    // Make sure that the auxiliary is the right version.
    synth_auxiliary_xchgmsg(SYNTH_DEV_AUXILIARY, SYNTH_AUXREQ_GET_VERSION, 0, 0,
                            (const unsigned char*) 0, 0,
                            &aux_version, (unsigned char*) 0, (int*) 0, 0);
    if (SYNTH_AUXILIARY_PROTOCOL_VERSION != aux_version) {
        synth_auxiliary_running = false;
        diag_printf("Error: an incorrect version of the I/O auxiliary is installed\n"
                    "    Expected version %d, actual version %d\n"
                    "    Installed binary is %s\n",
                    SYNTH_AUXILIARY_PROTOCOL_VERSION, aux_version, filename);
        cyg_hal_sys_exit(1);
    }
}

// Write a request to the I/O auxiliary, and optionally get back a
// reply. The dev_id is 0 for messages intended for the auxiliary
// itself, for example a device instantiation or a request for the
// current interrupt sate. Otherwise it identifies a specific device.
// The request code is specific to the device, and the two optional
// arguments are specific to the request.
void
synth_auxiliary_xchgmsg(int devid, int reqcode, int arg1, int arg2,
                        const unsigned char* txdata, int txlen,
                        int* result, 
                        unsigned char* rxdata, int* actual_rxlen, int rxlen)
{
    unsigned char   request[SYNTH_REQUEST_LENGTH];
    unsigned char   reply[SYNTH_REPLY_LENGTH];
    int             rc;
    int             reply_rxlen;
    cyg_bool_t      old_isrstate;

    CYG_ASSERT(devid >= 0, "A valid device id should be supplied");
    CYG_ASSERT((0 == txlen) || ((const unsigned char*)0 != txdata), "Data transmits require a transmit buffer");
    CYG_ASSERT((0 == rxlen) || ((unsigned char*)0 != rxdata), "Data receives require a receive buffer");
    CYG_ASSERT((0 == rxlen) || ((int*)0 != result), "If a reply is expected then space must be allocated");

    // I/O interactions with the auxiliary must be atomic: a context switch in
    // between sending the header and the actual data would be bad.
    HAL_DISABLE_INTERRUPTS(old_isrstate);

    // The auxiliary should be running for the duration of this
    // exchange. However the auxiliary can disappear asynchronously,
    // so it is not possible for higher-level code to be sure that the
    // auxiliary is still running.
    //
    // If the auxiliary disappears during this call then usually this
    // will cause a SIGCHILD or SIGPIPE, both of which result in
    // termination. The exception is when the auxiliary decides to
    // shut down stdout for some reason without exiting - that has to
    // be detected in the read loop.
    if (synth_auxiliary_running) {
        request[SYNTH_REQUEST_DEVID_OFFSET + 0]     = (devid >>  0) & 0x0FF;
        request[SYNTH_REQUEST_DEVID_OFFSET + 1]     = (devid >>  8) & 0x0FF;
        request[SYNTH_REQUEST_DEVID_OFFSET + 2]     = (devid >> 16) & 0x0FF;
        request[SYNTH_REQUEST_DEVID_OFFSET + 3]     = (devid >> 24) & 0x0FF;
        request[SYNTH_REQUEST_REQUEST_OFFSET + 0]   = (reqcode >>  0) & 0x0FF;
        request[SYNTH_REQUEST_REQUEST_OFFSET + 1]   = (reqcode >>  8) & 0x0FF;
        request[SYNTH_REQUEST_REQUEST_OFFSET + 2]   = (reqcode >> 16) & 0x0FF;
        request[SYNTH_REQUEST_REQUEST_OFFSET + 3]   = (reqcode >> 24) & 0x0FF;
        request[SYNTH_REQUEST_ARG1_OFFSET + 0]      = (arg1 >>  0) & 0x0FF;
        request[SYNTH_REQUEST_ARG1_OFFSET + 1]      = (arg1 >>  8) & 0x0FF;
        request[SYNTH_REQUEST_ARG1_OFFSET + 2]      = (arg1 >> 16) & 0x0FF;
        request[SYNTH_REQUEST_ARG1_OFFSET + 3]      = (arg1 >> 24) & 0x0FF;
        request[SYNTH_REQUEST_ARG2_OFFSET + 0]      = (arg2 >>  0) & 0x0FF;
        request[SYNTH_REQUEST_ARG2_OFFSET + 1]      = (arg2 >>  8) & 0x0FF;
        request[SYNTH_REQUEST_ARG2_OFFSET + 2]      = (arg2 >> 16) & 0x0FF;
        request[SYNTH_REQUEST_ARG2_OFFSET + 3]      = (arg2 >> 24) & 0x0FF;
        request[SYNTH_REQUEST_TXLEN_OFFSET + 0]     = (txlen >>  0) & 0x0FF;
        request[SYNTH_REQUEST_TXLEN_OFFSET + 1]     = (txlen >>  8) & 0x0FF;
        request[SYNTH_REQUEST_TXLEN_OFFSET + 2]     = (txlen >> 16) & 0x0FF;
        request[SYNTH_REQUEST_TXLEN_OFFSET + 3]     = (txlen >> 24) & 0x0FF;
        request[SYNTH_REQUEST_RXLEN_OFFSET + 0]     = (rxlen >>  0) & 0x0FF;
        request[SYNTH_REQUEST_RXLEN_OFFSET + 1]     = (rxlen >>  8) & 0x0FF;
        request[SYNTH_REQUEST_RXLEN_OFFSET + 2]     = (rxlen >> 16) & 0x0FF;
        request[SYNTH_REQUEST_RXLEN_OFFSET + 3]     = ((rxlen >> 24) & 0x0FF) | (((int*)0 != result) ? 0x080 : 0);

        // sizeof(synth_auxiliary_request) < PIPE_BUF (4096) so a single write should be atomic,
        // subject only to incoming clock or SIGIO or child-related signals.
        do {
            rc = cyg_hal_sys_write(to_aux, (const void*) &request, SYNTH_REQUEST_LENGTH);
        } while (-CYG_HAL_SYS_EINTR == rc);

        // Is there any more data to be sent?
        if (0 < txlen) {
            int sent    = 0;
            CYG_LOOP_INVARIANT(synth_auxiliary_running, "The auxiliary cannot just disappear");
        
            while (sent < txlen) {
                rc = cyg_hal_sys_write(to_aux, (const void*) &(txdata[sent]), txlen - sent);
                if (-CYG_HAL_SYS_EINTR == rc) {
                    continue;
                } else if (rc < 0) {
                    diag_printf("Internal error: unexpected result %d when sending buffer to auxiliary.\n", rc);
                    diag_printf("              : this application is exiting immediately.\n");
                    cyg_hal_sys_exit(1);
                } else {
                    sent += rc;
                }
            }
            CYG_ASSERT(sent <= txlen, "Amount of data sent should not exceed requested size");
        }

        // The auxiliary can now process this entire request. Is a reply expected?
        if ((int*)0 != result) {
            // The basic reply is also only a small number of bytes, so should be atomic.
            do {
                rc = cyg_hal_sys_read(from_aux, (void*) &reply, SYNTH_REPLY_LENGTH);
            } while (-CYG_HAL_SYS_EINTR == rc);
            if (rc <= 0) {
                if (rc < 0) {
                    diag_printf("Internal error: unexpected result %d when receiving data from auxiliary.\n", rc);
                } else {
                    diag_printf("Internal error: EOF detected on pipe from auxiliary.\n");
                }
                diag_printf("              : this application is exiting immediately.\n");
                cyg_hal_sys_exit(1);
            }
            CYG_ASSERT(SYNTH_REPLY_LENGTH == rc, "The correct amount of data should have been read");

            // Replies are packed in Tcl and assumed to be two 32-bit
            // little-endian integers.
            *result   = (reply[SYNTH_REPLY_RESULT_OFFSET + 3] << 24) |
                (reply[SYNTH_REPLY_RESULT_OFFSET + 2] << 16) |
                (reply[SYNTH_REPLY_RESULT_OFFSET + 1] <<  8) |
                (reply[SYNTH_REPLY_RESULT_OFFSET + 0] <<  0);
            reply_rxlen = (reply[SYNTH_REPLY_RXLEN_OFFSET + 3] << 24) |
                (reply[SYNTH_REPLY_RXLEN_OFFSET + 2] << 16) |
                (reply[SYNTH_REPLY_RXLEN_OFFSET + 1] <<  8) |
                (reply[SYNTH_REPLY_RXLEN_OFFSET + 0] <<  0);
        
            CYG_ASSERT(reply_rxlen <= rxlen, "The auxiliary should not be sending more data than was requested.");
        
            if ((int*)0 != actual_rxlen) {
                *actual_rxlen  = reply_rxlen;
            }
            if (reply_rxlen) {
                int received = 0;
            
                while (received < reply_rxlen) {
                    rc = cyg_hal_sys_read(from_aux, (void*) &(rxdata[received]), reply_rxlen - received);
                    if (-CYG_HAL_SYS_EINTR == rc) {
                        continue;
                    } else if (rc <= 0) {
                        if (rc < 0) {
                            diag_printf("Internal error: unexpected result %d when receiving data from auxiliary.\n", rc);
                        } else {
                            diag_printf("Internal error: EOF detected on pipe from auxiliary.\n");
                        }
                        diag_printf("              : this application is exiting immediately.\n");
                    } else {
                        received += rc;
                    }
                }
                CYG_ASSERT(received == reply_rxlen, "Amount received should be exact");
            }
        }
    }

    HAL_RESTORE_INTERRUPTS(old_isrstate);
}

// Instantiate a device. This takes arguments such as
// devs/eth/synth/ecosynth, current, ethernet, eth0, and 200x100 If
// the package and version are NULL strings then the device being
// initialized is application-specific and does not belong to any
// particular package.
int
synth_auxiliary_instantiate(const char* pkg, const char* version, const char* devtype, const char* devinst, const char* devdata)
{
    int         result = -1;
    char        buf[512 + 1];
    const char* str;
    int         index;

    CYG_ASSERT((const char*)0 != devtype, "Device instantiations must specify a valid device type");
    CYG_ASSERT((((const char*)0 != pkg) && ((const char*)0 != version)) || \
               (((const char*)0 == pkg) && ((const char*)0 == version)), "If a package is specified then the version must be supplied as well");
    
    index = 0;
    str = pkg;
    if ((const char*)0 == str) {
        str = "";
    }
    while ( (index < 512) && ('\0' != *str) ) {
        buf[index++] = *str++;
    }
    if (index < 512) buf[index++] = '\0';
    str = version;
    if ((const char*)0 == str) {
        str = "";
    }
    while ((index < 512) && ('\0' != *str) ) {
        buf[index++] = *str++;
    }
    if (index < 512) buf[index++] = '\0';
    for (str = devtype; (index < 512) && ('\0' != *str); ) {
        buf[index++] = *str++;
    }
    if (index < 512) buf[index++] = '\0';
    if ((const char*)0 != devinst) {
        for (str = devinst; (index < 512) && ('\0' != *str); ) {
            buf[index++] = *str++;
        }
    }
    if (index < 512) buf[index++] = '\0';
    if ((const char*)0 != devdata) {
        for (str = devdata; (index < 512) && ('\0' != *str); ) {
            buf[index++] = *str++;
        }
    }
    if (index < 512) {
        buf[index++] = '\0';
    } else {
        diag_printf("Internal error: buffer overflow constructing instantiate request for auxiliary.\n");
        diag_printf("              : this application is exiting immediately.\n");
        cyg_hal_sys_exit(1);
    }

    if (synth_auxiliary_running) {
        synth_auxiliary_xchgmsg(SYNTH_DEV_AUXILIARY, SYNTH_AUXREQ_INSTANTIATE, 0, 0,
                                buf, index,
                                &result, 
                                (unsigned char*) 0, (int *) 0, 0);
    }
    return result;
}

// ----------------------------------------------------------------------------
// SIGPIPE and SIGCHLD are special, related to the auxiliary process.
//
// A SIGPIPE can only happen when the application is writing to the
// auxiliary, which only happens inside synth_auxiliary_xchgmsg() (this
// assumes that no other code is writing down a pipe, e.g. to interact
// with a process other than the standard I/O auxiliary). Either the
// auxiliary has explicitly closed the pipe, which it is not supposed
// to do, or more likely it has exited. Either way, there is little
// point in continuing - unless we already know that the system is
// shutting down.
static void
synth_pipe_sighandler(int sig)
{
    CYG_ASSERT(CYG_HAL_SYS_SIGPIPE == sig, "The right signal handler should be invoked");
    if (synth_auxiliary_running) {
        synth_auxiliary_running   = false;
        diag_printf("Internal error: communication with the I/O auxiliary has been lost.\n");
        diag_printf("              : this application is exiting immediately.\n");
        cyg_hal_sys_exit(1);
    }
}

// Similarly it is assumed that there will be no child processes other than
// the auxiliary. Therefore a SIGCHLD indicates that the auxiliary has
// terminated unexpectedly. This is bad: normal termination involves
// the application exiting and the auxiliary terminating in response,
// not the other way around.
//
// As a special case, if it is known that the auxiliary is not currently
// usable then the signal is ignored. This copes with the situation where
// the auxiliary has just been fork()'d but has failed to initialize, or
// alternatively where the whole system is in the process of shutting down
// cleanly and it happens that the auxiliary got there first.
static void
synth_chld_sighandler(int sig)
{
    CYG_ASSERT(CYG_HAL_SYS_SIGCHLD == sig, "The right signal handler should be invoked");
    if (synth_auxiliary_running) {
        synth_auxiliary_running   = false;
        diag_printf("Internal error: the I/O auxiliary has terminated unexpectedly.\n");
        diag_printf("              : this application is exiting immediately.\n");
        cyg_hal_sys_exit(1);
    }
}

// ----------------------------------------------------------------------------
// Initialization

void
synth_hardware_init(void)
{
    struct cyg_hal_sys_sigaction action;
    struct cyg_hal_sys_sigset_t  blocked;
    int i;

    // Set up a sigprocmask to block all signals except the ones we
    // particularly want to handle. However do not block the tty
    // signals - the ability to ctrl-C a program is important.
    CYG_HAL_SYS_SIGFILLSET(&blocked);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGILL);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGBUS);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGFPE);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGSEGV);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGPIPE);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGCHLD);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGALRM);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGIO);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGHUP);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGINT);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGQUIT);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGTERM);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGCONT);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGSTOP);
    CYG_HAL_SYS_SIGDELSET(&blocked, CYG_HAL_SYS_SIGTSTP);
        
    if (0 != cyg_hal_sys_sigprocmask(CYG_HAL_SYS_SIG_SETMASK, &blocked, (cyg_hal_sys_sigset_t*) 0)) {
        CYG_FAIL("Failed to initialize sigprocmask");
    }

    // Now set up the VSR and ISR statics
    synth_VSR = &synth_default_vsr;
    for (i = 0; i < CYGNUM_HAL_ISR_COUNT; i++) {
        synth_isr_handlers[i].isr       = &synth_default_isr;
        synth_isr_handlers[i].data      = (CYG_ADDRWORD) 0;
        synth_isr_handlers[i].obj       = (CYG_ADDRESS) 0;
        synth_isr_handlers[i].pri       = CYGNUM_HAL_ISR_COUNT;
    }

    // Install signal handlers for SIGIO and SIGALRM, the two signals
    // that may cause the VSR to run. SA_NODEFER is important: it
    // means that the current signal will not be blocked while the
    // signal handler is running. Combined with a mask of 0, it means
    // that the sigprocmask does not change when a signal handler is
    // invoked, giving eCos the flexibility to switch to other threads
    // instead of having the signal handler return immediately.
    action.hal_mask     = 0;
    action.hal_flags    = CYG_HAL_SYS_SA_NODEFER;
    action.hal_bogus    = (void (*)(int)) 0;
    action.hal_handler  = &synth_alrm_sighandler;
    if (0 != cyg_hal_sys_sigaction(CYG_HAL_SYS_SIGALRM, &action, (struct cyg_hal_sys_sigaction*) 0)) {
        CYG_FAIL("Failed to install signal handler for SIGALRM");
    }
    action.hal_handler  = &synth_io_sighandler;
    if (0 != cyg_hal_sys_sigaction(CYG_HAL_SYS_SIGIO, &action, (struct cyg_hal_sys_sigaction*) 0)) {
        CYG_FAIL("Failed to install signal handler for SIGIO");
    }

    // Install handlers for the various exceptions. For now these also
    // operate with unchanged sigprocmasks, allowing nested
    // exceptions. It is not clear that this is entirely a good idea,
    // but in practice these exceptions will usually be handled by gdb
    // anyway.
    action.hal_handler  = &synth_exception_sighandler;
    if (0 != cyg_hal_sys_sigaction(CYG_HAL_SYS_SIGILL,  &action, (struct cyg_hal_sys_sigaction*) 0)) {
        CYG_FAIL("Failed to install signal handler for SIGILL");
    }
    if (0 != cyg_hal_sys_sigaction(CYG_HAL_SYS_SIGBUS,  &action, (struct cyg_hal_sys_sigaction*) 0)) {
        CYG_FAIL("Failed to install signal handler for SIGBUS");
    }
    if (0 != cyg_hal_sys_sigaction(CYG_HAL_SYS_SIGFPE,  &action, (struct cyg_hal_sys_sigaction*) 0)) {
        CYG_FAIL("Failed to install signal handler for SIGFPE");
    }
    if (0 != cyg_hal_sys_sigaction(CYG_HAL_SYS_SIGSEGV,  &action, (struct cyg_hal_sys_sigaction*) 0)) {
        CYG_FAIL("Failed to install signal handler for SIGSEGV");
    }

    // Also cope with SIGCHLD and SIGPIPE. SIGCHLD indicates that the
    // auxiliary has terminated, which is a bad thing. SIGPIPE
    // indicates that a write to the auxiliary has terminated, but
    // the error condition was caught at a different stage.
    action.hal_handler = &synth_pipe_sighandler;
    if (0 != cyg_hal_sys_sigaction(CYG_HAL_SYS_SIGPIPE,  &action, (struct cyg_hal_sys_sigaction*) 0)) {
        CYG_FAIL("Failed to install signal handler for SIGPIPE");
    }
    action.hal_handler = &synth_chld_sighandler;
    action.hal_flags  |= CYG_HAL_SYS_SA_NOCLDSTOP | CYG_HAL_SYS_SA_NOCLDWAIT;
    if (0 != cyg_hal_sys_sigaction(CYG_HAL_SYS_SIGCHLD,  &action, (struct cyg_hal_sys_sigaction*) 0)) {
        CYG_FAIL("Failed to install signal handler for SIGCHLD");
    }

    // Start up the auxiliary process.
    synth_start_auxiliary();
    
    // All done. At this stage interrupts are still disabled, no ISRs
    // have been installed, and the clock is not yet ticking.
    // Exceptions can come in and will be processed normally. SIGIO
    // and SIGALRM could come in, but nothing has yet been done
    // to make that happen.
}

// Second-stage hardware init. This is called after all C++ static
// constructors have been run, which should mean that all device
// drivers have been initialized and will have performed appropriate
// interactions with the I/O auxiliary. There should now be a
// message exchange with the auxiliary to let it know that there will
// not be any more devices, allowing it to remove unwanted frames,
// run the user's mainrc.tcl script, and so on. Also this is the
// time that the various toplevels get mapped on to the display.
//
// This request blocks until the auxiliary is ready. The return value
// indicates whether or not any errors occurred on the auxiliary side,
// and that those errors have not been suppressed using --keep-going

void
synth_hardware_init2(void)
{
    if (synth_auxiliary_running) {
        int result;
        synth_auxiliary_xchgmsg(SYNTH_DEV_AUXILIARY, SYNTH_AUXREQ_CONSTRUCTORS_DONE,
                               0, 0, (const unsigned char*) 0, 0,
                               &result,
                               (unsigned char*) 0, (int*) 0, 0);
        if ( !result ) {
            cyg_hal_sys_exit(1);
        }
    }
}
