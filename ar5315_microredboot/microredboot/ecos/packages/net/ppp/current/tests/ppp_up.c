//==========================================================================
//
//      tests/ppp_up.c
//
//      Simple test of PPP and networking support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Portions created by Nick Garnett are
// Copyright (C) 2003 eCosCentric Ltd.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg
// Date:         2003-06-01
// Purpose:      
// Description:  This test just brings the PPP link up and waits for the
//               other end to bring it down. Meanwhile the other end can
//               ping us, or talk to the HTTPD if it is configured.
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// PPP test code

#include "ppp_test_support.inl"

//==========================================================================

typedef void pr_fun(char *fmt, ...);

externC void show_network_tables(pr_fun *pr);

//==========================================================================
    

#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

//==========================================================================

static void do_test( cyg_serial_baud_rate_t baud )
{
    cyg_ppp_options_t options;
    cyg_ppp_handle_t ppp_handle;
    int i = 0;

    ppp_test_announce( "PPP_UP" );
    
    cyg_ppp_options_init( &options );

//    options.debug = 1;
//    options.kdebugflag = 1;

    options.baud = baud;
    
    show_network_tables( diag_printf );
    
    ppp_handle = cyg_ppp_up( CYGPKG_PPP_TEST_DEVICE, &options );

    CYG_TEST_INFO( "Waiting for PPP to come up");
    
    cyg_ppp_wait_up( ppp_handle );

    CYG_TEST_INFO( "Waiting until remote end goes down");

    while( cyg_ppp_wait_up( ppp_handle ) == 0 )
    {
        i++;

        if( (i % 60) == 0 )
            show_network_tables( diag_printf );

        cyg_thread_delay(100);
    }
    
    cyg_ppp_wait_down( ppp_handle );
}

//==========================================================================

void
ppp_test(cyg_addrword_t p)
{
    cyg_serial_baud_rate_t old;
    
    CYG_TEST_INIT();
    diag_printf("Start PPP test\n");

    init_all_network_interfaces();

//    old = ppp_test_set_baud( CYGNUM_SERIAL_BAUD_115200 );
//    do_test( CYGNUM_SERIAL_BAUD_115200 );

    old = ppp_test_set_baud( CYGNUM_SERIAL_BAUD_19200 );
    do_test( CYGNUM_SERIAL_BAUD_19200 );

#ifdef CYGPKG_PPP_TESTS_AUTOMATE

    {
        static cyg_serial_baud_rate_t test_rates[] =
            { CYGDAT_PPP_TEST_BAUD_RATES, 0 };

        int i;

        for( i = 0; test_rates[i] != 0; i++ )
        {
            ppp_test_set_baud( test_rates[i] );
            do_test( test_rates[i] );
        }

        ppp_test_set_baud( old );
    
        ppp_test_finish();
        
    }
           
#endif
    
    CYG_TEST_PASS_FINISH("PPP test OK");
}

//==========================================================================

void
cyg_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      ppp_test,          // entry
                      0,                 // entry parameter
                      "PPP test",        // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
}

//==========================================================================
