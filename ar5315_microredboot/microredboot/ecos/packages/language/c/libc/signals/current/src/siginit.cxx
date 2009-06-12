//========================================================================
//
//      siginit.cxx
//
//      ISO C and POSIX 1003.1 signals implementation
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-18
// Purpose:       Provide implementation of ISO C and POSIX 1003.1 signals
// Description:   This file initializes all hardware exceptions,
//                initializes the signal handler table and provides the
//                default action function for signals
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_signals.h>  // libc signals configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <signal.h>                // Main signals definitions
#include <cyg/infra/cyg_ass.h>     // Assertion infrastructure
#include <cyg/infra/cyg_trac.h>    // Tracing infrastructure
#include <stdlib.h>                // exit()

#ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS
# include <pkgconf/kernel.h>       // required for kernel includes
# include <cyg/kernel/except.hxx>  // Kernel exception API
# include <cyg/kernel/thread.hxx>  // Cyg_Thread::self()
# include <cyg/kernel/thread.inl>  // inline definitions for above
# include <cyg/kernel/intr.hxx>    // Interrupt enable
# include <cyg/hal/hal_intr.h>     // HAL interrupt control
#endif

#ifdef CYGSEM_LIBC_SIGNALS_THREAD_SAFE
# include <pkgconf/kernel.h>       // required for kernel includes
# include <cyg/kernel/mutex.hxx>
#endif

// TYPE DEFINITIONS

// define dummy class to allow initialization of cyg_libc_signal_handlers

class Cyg_libc_signals_dummy_init_class {
public:
    Cyg_libc_signals_dummy_init_class();
};

#ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS

struct exception_signal_mapping_t {
    cyg_code exception;
    int signal;
};    

// EXTERNS

extern cyg_exception_handler cyg_null_exception_handler;

#endif // ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS

// GLOBALS

// main signal handlers
__sighandler_t cyg_libc_signal_handlers[CYGNUM_LIBC_SIGNALS];

#ifdef CYGSEM_LIBC_SIGNALS_THREAD_SAFE
Cyg_Mutex cyg_libc_signal_handlers_mutex CYG_INIT_PRIORITY(LIBC);
#endif

// STATICS

// dummy object to invoke constructor
static Cyg_libc_signals_dummy_init_class
cyg_libc_signals_dummy_init_obj CYG_INIT_PRIORITY(LIBC);

#ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS

# ifdef CYGDBG_USE_TRACING
static cyg_uint8 cyg_libc_signals_hwhandler_trace_level =
  CYGNUM_LIBC_SIGNALS_HWHANDLER_TRACE_LEVEL;
# define TL1 (0 < cyg_libc_signals_hwhandler_trace_level)
# endif

# ifdef CYGSEM_LIBC_SIGNALS_CHAIN_HWEXCEPTIONS
// old exception info so we can chain
// FIXME: consider malloced linked list config option
static struct {
    cyg_exception_handler *old_handler;
    CYG_ADDRWORD old_data;
} exception_chain_data[CYGNUM_HAL_EXCEPTION_COUNT];
# endif

// struct that maps exceptions to signals
static const struct exception_signal_mapping_t
exception_signal_mapping[] = {

#ifdef CYGNUM_HAL_EXCEPTION_DATA_ACCESS
    {CYGNUM_HAL_EXCEPTION_DATA_ACCESS, SIGBUS},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_DATA_WRITE
    {CYGNUM_HAL_EXCEPTION_DATA_WRITE, SIGBUS},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_CODE_ACCESS
    {CYGNUM_HAL_EXCEPTION_CODE_ACCESS, SIGBUS},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_CODE_WRITE
    {CYGNUM_HAL_EXCEPTION_CODE_WRITE, SIGBUS},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_CODE_EXECUTE
    {CYGNUM_HAL_EXCEPTION_CODE_EXECUTE, SIGBUS},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_IO_ACCESS
    {CYGNUM_HAL_EXCEPTION_IO_ACCESS, SIGBUS},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_IO_WRITE
    {CYGNUM_HAL_EXCEPTION_IO_ACCESS, SIGBUS},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_ACCESS
    {CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_ACCESS, SIGSEGV},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_WRITE
    {CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_WRITE, SIGSEGV},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_CODE_TLBMISS_ACCESS
    {CYGNUM_HAL_EXCEPTION_CODE_TLBMISS_ACCESS, SIGSEGV},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_CODE_TLBMISS_WRITE
    {CYGNUM_HAL_EXCEPTION_CODE_TLBMISS_WRITE, SIGSEGV},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_DATA_TLBERROR_ACCESS
    {CYGNUM_HAL_EXCEPTION_DATA_TLBERROR_ACCESS, SIGSEGV},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_DATA_TLBERROR_WRITE
    {CYGNUM_HAL_EXCEPTION_DATA_TLBERROR_WRITE, SIGSEGV},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_CODE_TLBERROR_ACCESS
    {CYGNUM_HAL_EXCEPTION_CODE_TLBERROR_ACCESS, SIGSEGV},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_CODE_TLBERROR_WRITE
    {CYGNUM_HAL_EXCEPTION_CODE_TLBERROR_WRITE, SIGSEGV},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS
    {CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS, SIGBUS},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_WRITE
    {CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_WRITE, SIGBUS},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_IO_UNALIGNED_ACCESS
    {CYGNUM_HAL_EXCEPTION_IO_UNALIGNED_ACCESS, SIGBUS},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_IO_UNALIGNED_WRITE
    {CYGNUM_HAL_EXCEPTION_IO_UNALIGNED_WRITE, SIGBUS},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION
    {CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION, SIGILL},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_INTERRUPT
    {CYGNUM_HAL_EXCEPTION_INTERRUPT, SIGINT},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_TRAP
    {CYGNUM_HAL_EXCEPTION_TRAP, SIGTRAP},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_DIV_BY_ZERO
    {CYGNUM_HAL_EXCEPTION_DIV_BY_ZERO, SIGFPE},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_OVERFLOW
    {CYGNUM_HAL_EXCEPTION_OVERFLOW, SIGFPE},
#endif    
#ifdef CYGNUM_HAL_EXCEPTION_BOUNDS
    {CYGNUM_HAL_EXCEPTION_BOUNDS, SIGSEGV},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_SINGLE_STEP
    {CYGNUM_HAL_EXCEPTION_SINGLE_STEP, SIGTRAP},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_INSTRUCTION_BP
    {CYGNUM_HAL_EXCEPTION_INSTRUCTION_BP, SIGTRAP},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_PERIPHERAL_BP
    {CYGNUM_HAL_EXCEPTION_PERIPHERAL_BP, SIGTRAP},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_DATA_BP
    {CYGNUM_HAL_EXCEPTION_DATA_BP, SIGTRAP},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_DEVELOPMENT_BP
    {CYGNUM_HAL_EXCEPTION_DEVELOPMENT_BP, SIGTRAP},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_STACK_OVERFLOW
    {CYGNUM_HAL_EXCEPTION_STACK_OVERFLOW, SIGSEGV},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_STACK_FAULT
    {CYGNUM_HAL_EXCEPTION_STACK_FAULT, SIGSEGV},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_PARITY
    {CYGNUM_HAL_EXCEPTION_PARITY, SIGBUS},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_FPU
    {CYGNUM_HAL_EXCEPTION_FPU, SIGFPE},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_FPU_NOT_AVAIL
    {CYGNUM_HAL_EXCEPTION_FPU_NOT_AVAIL, SIGFPE},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_FPU_OVERFLOW
    {CYGNUM_HAL_EXCEPTION_FPU_OVERFLOW, SIGFPE},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_FPU_UNDERFLOW
    {CYGNUM_HAL_EXCEPTION_FPU_UNDERFLOW, SIGFPE},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_FPU_DIV_BY_ZERO
    {CYGNUM_HAL_EXCEPTION_FPU_DIV_BY_ZERO, SIGFPE},
#endif
#ifdef CYGNUM_HAL_EXCEPTION_SYSTEM_CALL
    {CYGNUM_HAL_EXCEPTION_SYSTEM_CALL, SIGSYS},
#endif
    {0, 0} // dummy value to ensure compiler is happy
};

        
// FUNCTIONS

/////////////////////////////////////////
// cyg_libc_signals_hwexcept_handler() //
/////////////////////////////////////////

// FIXME: should be able to get this work with
// CYGSEM_KERNEL_EXCEPTIONS_DECODE disabled as well as enabled
static void
cyg_libc_signals_hwexcept_handler( CYG_ADDRWORD data, cyg_code exception,
                                   CYG_ADDRWORD info)
{
    int signal = (int)data;
    int ret;

    CYG_REPORT_FUNCNAME("cyg_libc_signals_hwexcept_handler");

    CYG_REPORT_FUNCARG3( "data = %08x, exception = %d, info = %08x",
                         data, exception, info );

#ifdef CYGSEM_LIBC_SIGNALS_BAD_SIGNAL_FATAL 
    CYG_PRECONDITION((signal > 0) && (signal < CYGNUM_LIBC_SIGNALS), 
                     "Signal number not valid!");
#endif

// chain first as it may be more useful more low-level stuff needed
#ifdef CYGSEM_LIBC_SIGNALS_CHAIN_HWEXCEPTIONS
    // map exception to 0..CYGNUM_HAL_EXCEPTION_COUNT
    exception -= CYGNUM_HAL_EXCEPTION_MIN;

    // special case for null handler since it is only for uncaught exceptions

    if (exception_chain_data[exception].old_handler != 
        cyg_null_exception_handler) {
        (*exception_chain_data[exception].old_handler)(
            exception_chain_data[exception].old_data, exception, info);
        CYG_TRACE0(TL1, "Chained exception handler returned");
    } // if
#endif

#ifndef CYGSEM_LIBC_SIGNALS_BAD_SIGNAL_FATAL
    // if not fatal, silently return
    if ((signal <= 0) || (signal >= CYGNUM_LIBC_SIGNALS)) {
        CYG_REPORT_RETURN();
        return;
    }
#endif

    CYG_TRACE0(TL1, "Enabling interrupts");
    HAL_ENABLE_INTERRUPTS();

    CYG_TRACE0(TL1, "Raising signal");
    ret = raise(signal);

    if (ret) {
        CYG_TRACE1(TL1, "raise() returned non-zero value %d!!!", ret);
    } // if

    CYG_REPORT_RETURN();
} // cyg_libc_signals_hwexcept_handler()


//////////////////
// reg_except() //
//////////////////

static void inline
reg_except( cyg_code exception, int signal )
{
    Cyg_Thread::self()->register_exception(
        exception, &cyg_libc_signals_hwexcept_handler, (CYG_ADDRWORD)signal,
#ifdef CYGSEM_LIBC_SIGNALS_CHAIN_HWEXCEPTIONS
        &exception_chain_data[exception - 
                             CYGNUM_HAL_EXCEPTION_MIN].old_handler,
        &exception_chain_data[exception - CYGNUM_HAL_EXCEPTION_MIN].old_data
#else
        NULL, NULL
#endif
        );

} // reg_except();

#endif // ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS

///////////////////////////////////////////////////
// Cyg_libc_signals_dummy_init_class constructor //
///////////////////////////////////////////////////

Cyg_libc_signals_dummy_init_class::Cyg_libc_signals_dummy_init_class()
{
    cyg_ucount8 i;

    CYG_REPORT_FUNCNAME("Cyg_libc_signals_dummy_init_class constructor");
    
    // FIXME: some should be SIG_IGN?
    for (i=0; i<CYGNUM_LIBC_SIGNALS; i++)
        cyg_libc_signal_handlers[i] = SIG_DFL;

#ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS
    // go through the entire array of exceptions, _but_ subtract 1 for
    // the dummy at the end
    for (i=0; i < (sizeof(exception_signal_mapping) / 
                   sizeof(exception_signal_mapping_t) - 1); i++) {
        CYG_ASSERT( (exception_signal_mapping[i].exception <= 
                    CYGNUM_HAL_EXCEPTION_MAX) &&
                    (exception_signal_mapping[i].exception >=
                    CYGNUM_HAL_EXCEPTION_MIN),
                    "Asked to register bad exception");

        CYG_ASSERT( (exception_signal_mapping[i].signal > 0) &&
                    (exception_signal_mapping[i].signal < 
                     CYGNUM_LIBC_SIGNALS), "Asked to register bad signal" );

        reg_except( exception_signal_mapping[i].exception,
                    exception_signal_mapping[i].signal);
    }
#endif

    CYG_REPORT_RETURN();
} // Cyg_libc_signals_dummy_init_class() constructor


////////////////////////////////////////
// cyg_libc_signals_default_handler() //
////////////////////////////////////////

// Default signal handler - SIG_DFL
externC void
cyg_libc_signals_default_handler(int sig)
{
    CYG_REPORT_FUNCNAME( "cyg_libc_signals_default_handler" );

    CYG_REPORT_FUNCARG1( "signal number = %d", sig );

    exit(1000 + sig); // FIXME

    CYG_REPORT_RETURN();
} // cyg_libc_signals_default_handler()

#ifdef CYGSEM_LIBC_SIGNALS_THREAD_SAFE
/////////////////////////////////////
// cyg_libc_signals_lock_do_lock() //
/////////////////////////////////////

externC cyg_bool
cyg_libc_signals_lock_do_lock(void)
{
    cyg_bool ret;
    CYG_REPORT_FUNCNAMETYPE("cyg_libc_signals_lock_do_lock", "returning %d");

    ret = cyg_libc_signal_handlers_mutex.lock();

    CYG_REPORT_RETVAL(ret);

    return ret;
} // cyg_libc_signals_lock_do_lock()

///////////////////////////////////////
// cyg_libc_signals_lock_do_unlock() //
///////////////////////////////////////

externC void
cyg_libc_signals_lock_do_unlock(void)
{
    CYG_REPORT_FUNCNAME("cyg_libc_signals_lock_do_unlock");

    cyg_libc_signal_handlers_mutex.unlock();

    CYG_REPORT_RETURN();
} // cyg_libc_signals_lock_do_unlock()

#endif // ifdef CYGSEM_LIBC_SIGNALS_THREAD_SAFE

// EOF siginit.cxx
