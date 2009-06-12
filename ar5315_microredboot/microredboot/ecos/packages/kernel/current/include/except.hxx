#ifndef CYGONCE_KERNEL_EXCEPT_HXX
#define CYGONCE_KERNEL_EXCEPT_HXX

//==========================================================================
//
//      except.hxx
//
//      Exception handling declarations
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
// Purpose:      Define exception interfaces
// Description:  The classes defined here collectively implement the
//               internal API used to register, manage and deliver
//               exceptions.
// Usage:        #include <cyg/kernel/thread.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_intr.h>           // exception defines

// -------------------------------------------------------------------------
// Exception handler function prototype

typedef void cyg_exception_handler(
    CYG_ADDRWORD        data,                   // user supplied data
    cyg_code            exception_number,       // exception being raised
    CYG_ADDRWORD        exception_info          // any exception specific info
    );

// -------------------------------------------------------------------------
// Exception delivery interface. This function is exported to the HAL which
// invokes it for all exceptions that it is not able to handle itself.

externC void cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data );

// -------------------------------------------------------------------------
// Exception control class. Depending on the configuration there is either
// one of these per thread, or one for the entire system.

#ifdef CYGPKG_KERNEL_EXCEPTIONS

class Cyg_Exception_Control
{

#ifdef CYGSEM_KERNEL_EXCEPTIONS_DECODE
    cyg_exception_handler   *exception_handler[CYGNUM_HAL_EXCEPTION_COUNT];
    
    CYG_ADDRWORD            exception_data[CYGNUM_HAL_EXCEPTION_COUNT];
#else
    cyg_exception_handler   *exception_handler; // Handler function
    
    CYG_ADDRWORD            exception_data;     // Handler data
#endif

public:

    Cyg_Exception_Control();

    // Register an exception handler for either the specific exception
    // or for all exceptions.
    void register_exception(
        cyg_code                exception_number,       // exception number
        cyg_exception_handler   handler,                // handler function
        CYG_ADDRWORD            data,                   // data argument
        cyg_exception_handler   **old_handler,          // handler function
        CYG_ADDRWORD            *old_data               // data argument
        );

    // Remove an exception handler.
    void deregister_exception(
        cyg_code                exception_number        // exception number
        );

    // Deliver the given exception now by invoking the appropriate
    // exception handler.
    void deliver_exception(
        cyg_code            exception_number,       // exception being raised
        CYG_ADDRWORD        exception_info          // exception specific info
        );
};

#endif

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_EXCEPT_HXX
// EOF except.hxx
