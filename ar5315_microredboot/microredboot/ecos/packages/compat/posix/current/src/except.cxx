//==========================================================================
//
//      except.cxx
//
//      POSIX exception translation
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
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-03-27
// Purpose:             POSIX exception translation
// Description:         This file contains code to translate eCos hardware
//                      exceptions into POSIX signals.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/posix.h>

#include <cyg/kernel/ktypes.h>          // base kernel types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/infra/diag.h>

#include "pprivate.h"                   // POSIX private header

#include <signal.h>                     // our header

#include <cyg/kernel/thread.inl>

//==========================================================================
// Translation table from eCos exceptions to POSIX signals.

static const struct
{
    cyg_code    exception;
    int         signal;
} exception_signal_mapping[] =
{
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

//==========================================================================
// POSIX exception handler

static void cyg_posix_exception_handler(
    CYG_ADDRWORD        data,                   // user supplied data == signal number
    cyg_code            exception_number,       // exception being raised
    CYG_ADDRWORD        exception_info          // any exception specific info
    )
{
    int signo = 0;

    pthread_info *self = pthread_self_info();

    if( self == NULL )
    {
        // Not a POSIX thread, just return
        return;
    }
    
#ifdef CYGSEM_KERNEL_EXCEPTIONS_DECODE

    signo = data;

#else

    for( int i = 0; exception_signal_mapping[i].signal != 0; i++ )    
    {
        if( exception_signal_mapping[i].exception == exception_number )
        {
            signo = exception_signal_mapping[i].signal;
            break;
        }
    }

#endif

    if( sigismember( &self->sigmask, signo ) )
    {
        // The signal is masked in the current thread. POSIX says that
        // the behaviour is undefined here. We choose to ignore it.

        return;
    }

    // The kernel exception handler may have disabled interrupts, so
    // we (re-)enable them here. From this point on we are running in
    // a context that is effectively just pushed onto the stack of the
    // current thread. If we return we will unwind and resume
    // execution from the excepting code. We can also, in theory,
    // longjump out of the signal handler, and although that is
    // deprecated, we make sure in cyg_deliver_signals() that it is
    // possible to do it.
    
    HAL_ENABLE_INTERRUPTS();
    
    struct sigevent sev;

    sev.sigev_notify           = SIGEV_SIGNAL;
    sev.sigev_signo            = signo;
    sev.sigev_value.sival_ptr  = (void *)exception_info;

    // Generate the signal
    cyg_sigqueue( &sev, SI_EXCEPT );

    // And try to deliver it
    cyg_deliver_signals();
}

//==========================================================================
// Install all the exception handlers

static void install_handlers( Cyg_Thread *thread)
{
#ifdef CYGSEM_KERNEL_EXCEPTIONS_DECODE

    // With decoded exceptions, we must install a separate exception
    // handler for each supported exception.

    for( int i = 0; exception_signal_mapping[i].signal != 0; i++ )
    {
        thread->register_exception( exception_signal_mapping[i].exception,
                                    cyg_posix_exception_handler,
                                    exception_signal_mapping[i].signal,,
                                    NULL,
                                    NULL);
    }
    
#else

    // Otherwise there is just one exception handler for all exceptions.
    
    thread->register_exception( CYGNUM_HAL_EXCEPTION_MIN,
                                cyg_posix_exception_handler,
                                0,
                                NULL,
                                NULL);
    
#endif    
    
}

//==========================================================================
// Initialization

externC void cyg_posix_exception_start()
{
#ifdef CYGSEM_KERNEL_EXCEPTIONS_GLOBAL

    // With global exceptions, we only need to install a single static
    // set of exception handlers. Note that by this point in system
    // initialization the idle thread should be installed as the
    // current thread, so we pass a pointer to that to
    // install_handlers(). The identity of the thread passed is
    // actually irrelevant in this case and is just used as a handle
    // into the thread class.

    install_handlers( Cyg_Thread::self() );
    
#endif    
}

//==========================================================================
// Per thread exception initialization and destruction

externC void cyg_pthread_exception_init(pthread_info *thread)
{
#ifndef CYGSEM_KERNEL_EXCEPTIONS_GLOBAL

    // With non-global exceptions we must install a new set of handlers
    // for each thread.

    install_handlers( thread->thread );
    
#endif
}

externC void cyg_pthread_exception_destroy(pthread_info *thread)
{
    // Nothing to do at present.
}

// -------------------------------------------------------------------------
// EOF except.cxx
