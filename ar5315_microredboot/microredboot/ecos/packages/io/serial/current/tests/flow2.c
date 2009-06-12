//==========================================================================
//
//        flow2.c
//
//        Test duplex receive and send with flow control.
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
// Author(s):     jskov,jlarmour
// Contributors:  
// Date:          2000-07-27
// Description:   Test the duplex receive and send capabilities of 
//                the serial driver with flow control.
// Requirements:  This test requires the ser_filter on the host side.
// 
//####DESCRIPTIONEND####

#include <pkgconf/system.h>

#include <cyg/infra/testcase.h>         // test macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif
#ifdef CYGPKG_IO_SERIAL
# include <pkgconf/io_serial.h>
#endif

// Package requirements
#ifndef CYGPKG_IO_SERIAL
# define NA_MSG "Requires I/O serial package"
#elif !defined(CYGFUN_KERNEL_API_C)
# define NA_MSG "Requires kernel C API"
#elif !defined(CYGPKG_IO_SERIAL_FLOW_CONTROL)
# define NA_MSG "Requires serial flow control"
#endif

#ifdef NA_MSG
void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( NA_MSG);
}
#else

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include <cyg/kernel/kapi.h>
unsigned char stack[CYGNUM_HAL_STACK_SIZE_TYPICAL];
cyg_thread thread_data;
cyg_handle_t thread_handle;

#include "ser_test_protocol.inl"


//---------------------------------------------------------------------------
// run the tests

static void
run_tests( cyg_io_handle_t ser_handle )
{
    // Start slowly, then go for max size.
    {
        test_binary(ser_handle,             16, MODE_DUPLEX_ECHO);
        test_binary(ser_handle,            128, MODE_DUPLEX_ECHO);
        test_binary(ser_handle,            256, MODE_DUPLEX_ECHO);
        test_binary(ser_handle,           1024, MODE_DUPLEX_ECHO);
    }
}

//---------------------------------------------------------------------------
// Serial test main function.

void
serial_test( void )
{
    cyg_io_handle_t ser_handle;
    cyg_ser_cfg_t *cfg=&test_configs[0];
    cyg_ser_cfg_t new_cfg;
    int count = sizeof(test_configs) / sizeof(cyg_ser_cfg_t);
    int i;

    test_open_ser(&ser_handle);

    // We need the filter for this test.
    test_ping(ser_handle);

    // Choose the configuration with the fastest baud rate, to be most
    // provocative. Start at 1 coz cfg already points at 0
    for (i=1; i<count; i++) {
        if (cfg->baud_rate < test_configs[i].baud_rate)
            cfg=&test_configs[i];
    }

    // Set flow control from configuration
    // Choose software first

#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_SOFTWARE
    CYG_TEST_INFO("Setting software flow control");

    new_cfg = *cfg;
    new_cfg.flags |= CYGNUM_SERIAL_FLOW_XONXOFF_RX |
                     CYGNUM_SERIAL_FLOW_XONXOFF_TX;
    if (ENOERR == change_config(ser_handle, &new_cfg))
        run_tests( ser_handle );
#endif

    // hardware flow control
#ifdef CYGOPT_IO_SERIAL_FLOW_CONTROL_HW
    CYG_TEST_INFO("Setting RTS/CTS hardware flow control");

    new_cfg = *cfg;
    new_cfg.flags |= CYGNUM_SERIAL_FLOW_RTSCTS_RX|CYGNUM_SERIAL_FLOW_RTSCTS_TX;
    if (ENOERR == change_config(ser_handle, &new_cfg))
        run_tests( ser_handle );

    CYG_TEST_INFO("Setting DSR/DTR hardware flow control");

    new_cfg = *cfg;
    new_cfg.flags |= CYGNUM_SERIAL_FLOW_DSRDTR_RX|CYGNUM_SERIAL_FLOW_DSRDTR_TX;
    if (ENOERR == change_config(ser_handle, &new_cfg))
        run_tests( ser_handle );
#endif
 
    CYG_TEST_PASS_FINISH("flow2 test OK");
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

#endif // ifndef NA_MSG

// EOF flow2.c
