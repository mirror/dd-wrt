//==========================================================================
//
//      tests/ppp_auth.c
//
//      Simple test of PPP authentication support
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
// Tests
//
// The following table contains parameters for various tests permuting
// the authentication requirements of the peer, the authentication
// requirements of this end and the user name and password used.
//
// For this test to work the /etc/ppp/pap-secrets file on the remote
// end should contain the following two lines:
//
// eCos       *         secret       *
// eCosPAP    *         secretPAP    *
//
// The /etc/ppp/chap-secrets file should contain:
//
// eCos       *         secret       *
// eCosCHAP   *         secretCHAP   *
//

static struct auth_info
{
    char                        *name;
    cyg_serial_baud_rate_t      baud;
    int                         refuse_pap;
    int                         refuse_chap;
    char	                user[MAXNAMELEN];
    char	                passwd[MAXSECRETLEN];
    cyg_int32                   result;
} tests[] =
{
    // PPP_AUTH_1 just tries the default auth mechanism for the server
    { "PPP_AUTH_1", CYGNUM_SERIAL_BAUD_115200, 0, 0, "eCos", "secret", 0 },
    { "PPP_AUTH_1", CYGNUM_SERIAL_BAUD_115200, 0, 0, "eCos", "secretX", -1 },
    { "PPP_AUTH_1", CYGNUM_SERIAL_BAUD_115200, 0, 0, "eCosX", "secret", -1 },

    // PPP_AUTH_2 requires PAP authentication only
    { "PPP_AUTH_2", CYGNUM_SERIAL_BAUD_115200, 1, 0, "eCosCHAP", "secretCHAP", -1 },
    { "PPP_AUTH_2", CYGNUM_SERIAL_BAUD_115200, 1, 0, "eCosPAP", "secretPAP", -1 },
    { "PPP_AUTH_2", CYGNUM_SERIAL_BAUD_115200, 1, 0, "eCos", "secret", -1 },
    { "PPP_AUTH_2", CYGNUM_SERIAL_BAUD_115200, 0, 1, "eCosCHAP", "secretCHAP", -1 },
    { "PPP_AUTH_2", CYGNUM_SERIAL_BAUD_115200, 0, 1, "eCosPAP", "secretPAP", 0 },
    { "PPP_AUTH_2", CYGNUM_SERIAL_BAUD_115200, 0, 1, "eCos", "secret", 0 },

    // PPP_AUTH_3 requires CHAP authentication only
    { "PPP_AUTH_3", CYGNUM_SERIAL_BAUD_115200, 0, 1, "eCosPAP", "secretPAP", -1 },
    { "PPP_AUTH_3", CYGNUM_SERIAL_BAUD_115200, 0, 1, "eCosCHAP", "secretCHAP", -1 },
    { "PPP_AUTH_3", CYGNUM_SERIAL_BAUD_115200, 0, 1, "eCos", "secret", -1 },
    { "PPP_AUTH_3", CYGNUM_SERIAL_BAUD_115200, 1, 0, "eCosPAP", "secretPAP", -1 },
    { "PPP_AUTH_3", CYGNUM_SERIAL_BAUD_115200, 1, 0, "eCosCHAP", "secretCHAP", 0 },
    { "PPP_AUTH_3", CYGNUM_SERIAL_BAUD_115200, 1, 0, "eCos", "secret", 0 },    
    
    { NULL }
};

//==========================================================================


static int do_test( struct auth_info *test )
{
    cyg_ppp_options_t options;
    cyg_ppp_handle_t ppp_handle;
    int i = 0;
    cyg_int32 result;
    
    CYG_TEST_INFO( test->name );
            
    ppp_test_set_baud( test->baud );
    
    ppp_test_announce( test->name );
    
    cyg_ppp_options_init( &options );

//    options.debug = 1;
//    options.kdebugflag = 1;

    options.baud = test->baud;
    options.refuse_pap = test->refuse_pap;
    options.refuse_chap = test->refuse_chap;
    strncpy( options.user, test->user, MAXNAMELEN );
    strncpy( options.passwd, test->passwd, MAXNAMELEN );
    
//    show_network_tables( diag_printf );
    
    ppp_handle = cyg_ppp_up( CYGPKG_PPP_TEST_DEVICE, &options );

    CYG_TEST_INFO( "Waiting for PPP to come up");
    
    result = cyg_ppp_wait_up( ppp_handle );

    if( result != test->result )
    {
        CYG_TEST_INFO( "Failed" );

        return 0;
    }
    else
    {
        CYG_TEST_INFO( "Waiting until remote end goes down");

        while( cyg_ppp_wait_up( ppp_handle ) == 0 )
        {
            i++;

//            if( (i % 60) == 0 )
//                show_network_tables( diag_printf );

            cyg_thread_delay(100);
        }

        cyg_ppp_wait_down( ppp_handle );

        return 1;
    }
    
}

//==========================================================================

void
ppp_test(cyg_addrword_t p)
{
    cyg_serial_baud_rate_t old;
    struct auth_info *test;
    int failures = 0;
    
    CYG_TEST_INIT();
    diag_printf("Start PPP test\n");

    init_all_network_interfaces();

    old = ppp_test_set_baud( CYGNUM_SERIAL_BAUD_38400 );


    for( test = &tests[0]; test->name != NULL; test++ )
    {
        if( !do_test( test ) )
            failures++;

        cyg_thread_delay(300);
    }

    ppp_test_set_baud( old );
    
    ppp_test_finish();

    if( failures > 0 )
        CYG_TEST_FAIL("Some tests failed");
    else
        CYG_TEST_PASS("OK");
    
    CYG_TEST_FINISH("PPP auth test");
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
