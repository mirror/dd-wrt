//==========================================================================
//
//        serial5.c
//
//        Test data duplex receive and send.
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
// Date:          1999-03-19
// Description:   Test the duplex receive and send capabilities of 
//                the serial driver.
// Requirements:  This test requires the ser_filter on the host side.
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
// Serial test main function.
void
serial_test( void )
{
    cyg_io_handle_t ser_handle;

    test_open_ser(&ser_handle);

    // We need the filter for this test.
    test_ping(ser_handle);


#if (CYGINT_IO_SERIAL_TEST_SKIP_38400 > 0)
    {
        // The board is too slow to run the driver in interrupt mode
        // at the default 38400 baud when doing this test, so run it
        // at a slower rate.
        cyg_ser_cfg_t slow_cfg = { 
            CYGNUM_SERIAL_BAUD_19200, CYGNUM_SERIAL_WORD_LENGTH_8, 
            CYGNUM_SERIAL_STOP_1, CYGNUM_SERIAL_PARITY_NONE,
            CYGNUM_SERIAL_FLOW_NONE };

        CYG_TEST_INFO("Reducing baud rate to 19200");
        change_config(ser_handle, &slow_cfg);
    }
#endif

    // Start out slow, then go for many tests. Each cycle causes
    // 512 bytes to be sent over the wire.
    test_binary(ser_handle,    1, MODE_DUPLEX_ECHO);
    test_binary(ser_handle,    2, MODE_DUPLEX_ECHO);
    test_binary(ser_handle,    5, MODE_DUPLEX_ECHO);

#if 0 // Disable these until the ser_filter produces status output.
    test_binary(ser_handle,  128, MODE_DUPLEX_ECHO);
    test_binary(ser_handle,  512, MODE_DUPLEX_ECHO);
    test_binary(ser_handle, 1024, MODE_DUPLEX_ECHO);
#endif

    CYG_TEST_PASS_FINISH("serial5 test OK");
}

void
cyg_start(void)
{
    CYG_TEST_INIT();
    cyg_thread_create(10,                   // Priority - just a number
                      (cyg_thread_entry_t*)serial_test,         // entry
                      0,                    // 
                      "serial_thread",     // Name
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
// EOF serial5.c
