//==========================================================================
//
//      common/except.cxx
//
//      Exception handling implementation
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
// Contributors: nickg, jlarmour
// Date:         1999-02-16
// Purpose:      Exception handling implementation
// Description:  This file contains the code that registers and delivers
//               exceptions.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/kernel/instrmnt.h>       // instrumentation

#include <cyg/kernel/except.hxx>       // our header

#include <cyg/hal/hal_arch.h>          // architecture definitions
#include <cyg/hal/hal_intr.h>          // vector definitions

#include <cyg/kernel/thread.hxx>       // thread interface

#include <cyg/kernel/thread.inl>       // thread inlines

#ifdef CYGPKG_KERNEL_EXCEPTIONS

// -------------------------------------------------------------------------
// Null exception handler. This is used to capture exceptions that are
// not caught by user supplied handlers.

void
cyg_null_exception_handler(
    CYG_ADDRWORD        data,                   // user supplied data
    cyg_code            exception_number,       // exception being raised
    CYG_ADDRWORD        exception_info          // any exception specific info
    )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3("data=%08x, exception=%d, info=%08x", data,
                        exception_number, exception_info);
    CYG_TRACE1( 1, "Uncaught exception: %d", exception_number);
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Exception Controller constructor.

Cyg_Exception_Control::Cyg_Exception_Control()
{
    CYG_REPORT_FUNCTION();
#ifdef CYGSEM_KERNEL_EXCEPTIONS_DECODE

    for( int i = 0; i < CYGNUM_HAL_EXCEPTION_COUNT ; i++ )
        exception_handler[i] = cyg_null_exception_handler,
            exception_data[i] = 0;
#else

    exception_handler = cyg_null_exception_handler;    
    exception_data = 0;
    
#endif
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Exception registation. Stores the handler function and data to be used
// for handling the given exception number. Where exceptions are not decoded
// only a single handler may be registered for all exceptions. This function
// also returns the old values of the exception handler and data to allow
// chaining to be implemented.

void
Cyg_Exception_Control::register_exception(
    cyg_code                exception_number,       // exception number
    cyg_exception_handler   handler,                // handler function
    CYG_ADDRWORD            data,                   // data argument
    cyg_exception_handler   **old_handler,          // handler function
    CYG_ADDRWORD            *old_data               // data argument
    )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5("exception=%d, handler func=%08x, data=%08x, "
                        "space for old handler=%08x,space for old data=%08x",
                        exception_number, handler, data, old_handler,
                        old_data);

    CYG_ASSERT( exception_number <= CYGNUM_HAL_EXCEPTION_MAX,
                "Out of range exception number");
    CYG_ASSERT( exception_number >= CYGNUM_HAL_EXCEPTION_MIN,
                "Out of range exception number");


    // Should we complain if there is already a registered
    // handler, or should we just replace is silently?
    
#ifdef CYGSEM_KERNEL_EXCEPTIONS_DECODE

    if( old_handler != NULL )
        *old_handler = exception_handler[exception_number -
                                        CYGNUM_HAL_EXCEPTION_MIN];
    if( old_data != NULL )
        *old_data = exception_data[exception_number - 
                                  CYGNUM_HAL_EXCEPTION_MIN];
    exception_handler[exception_number - CYGNUM_HAL_EXCEPTION_MIN] = handler;
    exception_data[exception_number - CYGNUM_HAL_EXCEPTION_MIN] = data;
    
#else
    
    if( old_handler != NULL )
        *old_handler = exception_handler;
    if( old_data != NULL )
        *old_data = exception_data;
    exception_handler = handler;
    exception_data = data;
    
#endif
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Exception deregistation. Revert the handler for the exception number
// to the default.

void
Cyg_Exception_Control::deregister_exception(
    cyg_code                exception_number        // exception number
    )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("exception number=%d", exception_number);

    CYG_ASSERT( exception_number <= CYGNUM_HAL_EXCEPTION_MAX,
                "Out of range exception number");
    CYG_ASSERT( exception_number >= CYGNUM_HAL_EXCEPTION_MIN,
                "Out of range exception number");

#ifdef CYGSEM_KERNEL_EXCEPTIONS_DECODE

    exception_handler[exception_number - CYGNUM_HAL_EXCEPTION_MIN] = 
        cyg_null_exception_handler;
    exception_data[exception_number - CYGNUM_HAL_EXCEPTION_MIN] = 0;
    
#else
    
    exception_handler = cyg_null_exception_handler;
    exception_data = 0;
    
#endif

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Exception delivery. Call the appropriate exception handler.

void
Cyg_Exception_Control::deliver_exception(
    cyg_code            exception_number,       // exception being raised
    CYG_ADDRWORD        exception_info          // exception specific info
    )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2("exception number=%d, exception info=%08x",
                        exception_number, exception_info);

    cyg_exception_handler *handler = NULL;
    CYG_ADDRWORD data = 0;

    CYG_ASSERT( exception_number <= CYGNUM_HAL_EXCEPTION_MAX,
                "Out of range exception number");
    CYG_ASSERT( exception_number >= CYGNUM_HAL_EXCEPTION_MIN,
                "Out of range exception number");
    
#ifdef CYGSEM_KERNEL_EXCEPTIONS_DECODE

    handler = exception_handler[exception_number - CYGNUM_HAL_EXCEPTION_MIN];
    data = exception_data[exception_number - CYGNUM_HAL_EXCEPTION_MIN];
    
#else
    
    handler = exception_handler;
    data = exception_data;
    
#endif

    // The handler will always be a callable function: either the user's
    // registered function or the null handler. So it is always safe to
    // just go ahead and call it.
    
    handler( data, exception_number, exception_info );

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Exception delivery function called from the HAL as a result of a
// hardware exception being raised.

externC void
cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data )
{
    CYG_REPORT_FUNCTION();
    Cyg_Thread::self()->deliver_exception( (cyg_code)code, data );
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Where exceptions are global, there is a single static instance of the
// exception control object. Define it here.

#ifdef CYGSEM_KERNEL_EXCEPTIONS_GLOBAL

Cyg_Exception_Control Cyg_Thread::exception_control 
                                              CYG_INIT_PRIORITY(INTERRUPTS);

#endif

// -------------------------------------------------------------------------

#endif // ifdef CYGPKG_KERNEL_EXCEPTIONS

// -------------------------------------------------------------------------
// EOF common/except.cxx
