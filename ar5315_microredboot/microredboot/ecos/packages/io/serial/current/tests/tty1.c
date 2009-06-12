//==========================================================================
//
//        tty.c
//
//        Test TTY driver API
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
// Author(s):     jskov
// Contributors:  jskov
// Date:          1999-04-08
// Description:   Test the TTY driver API.
// 
//####DESCRIPTIONEND####

#include <pkgconf/system.h>

#include <cyg/infra/testcase.h>         // test macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

// Package requirements
#if defined(CYGPKG_IO_SERIAL) && defined(CYGPKG_KERNEL)

#include <pkgconf/kernel.h>

// Package option requirements
#if defined(CYGFUN_KERNEL_API_C)

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include <cyg/kernel/kapi.h>
unsigned char stack[CYGNUM_HAL_STACK_SIZE_TYPICAL];
cyg_thread thread_data;
cyg_handle_t thread_handle;

#include "ser_test_protocol.inl"

//---------------------------------------------------------------------------
// TTY test main function.


void
tty_api_test(cyg_io_handle_t* handle)
{
    int res, len;
    unsigned char buffer[16];

    // Always return...
    if (handle)
        return;

    CYG_TEST_FAIL_FINISH("Not reached");

    // read & write
    res = cyg_io_read(handle, &buffer[0], &len);
    res = cyg_io_write(handle, &buffer[0], &len);

    // cyg_io_get_config
    // TTY layer
    cyg_io_get_config(handle, 
                      CYG_IO_GET_CONFIG_TTY_INFO, &buffer[0], &len);
    // Call-throughs to serial layer.
    cyg_io_get_config(handle, 
                      CYG_IO_GET_CONFIG_SERIAL_INFO, &buffer[0], &len);
    cyg_io_get_config(handle, 
                      CYG_IO_GET_CONFIG_SERIAL_OUTPUT_DRAIN, &buffer[0], &len);
    cyg_io_get_config(handle, 
                      CYG_IO_GET_CONFIG_SERIAL_INPUT_FLUSH, &buffer[0], &len);
    cyg_io_get_config(handle, 
                      CYG_IO_GET_CONFIG_SERIAL_ABORT, &buffer[0], &len);
    cyg_io_get_config(handle, 
                      CYG_IO_GET_CONFIG_SERIAL_OUTPUT_FLUSH, &buffer[0], &len);

    // cyg_io_set_config
    // TTY layer.
    cyg_io_set_config(handle, 
                      CYG_IO_SET_CONFIG_TTY_INFO, &buffer[0], &len);
    // Call-throughs to serial layer.
    cyg_io_set_config(handle, 
                      CYG_IO_SET_CONFIG_SERIAL_INFO, &buffer[0], &len);
}


void
tty_test( void )
{
    cyg_io_handle_t tty_handle;

    test_open_tty(&tty_handle);

    tty_api_test(&tty_handle);

    CYG_TEST_PASS_FINISH("tty1 test OK");
}

void
cyg_start(void)
{
    CYG_TEST_INIT();
    cyg_thread_create(10,                   // Priority - just a number
                      (cyg_thread_entry_t*)tty_test,         // entry
                      0,                    // 
                      "tty_thread",         // Name
                      &stack[0],            // Stack
                      CYGNUM_HAL_STACK_SIZE_TYPICAL,           // Size
                      &thread_handle,       // Handle
                      &thread_data          // Thread data structure
        );
    cyg_thread_resume(thread_handle);
    cyg_scheduler_start();
}

#else // CYGFUN_KERNEL_API_C
#define N_A_MSG "Needs kernel C API"
#endif

#else // CYGPKG_IO_SERIAL && CYGPKG_KERNEL
#define N_A_MSG "Needs IO/serial and Kernel"
#endif

#ifdef N_A_MSG
void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG);
}
#endif // N_A_MSG
// EOF tty1.c
