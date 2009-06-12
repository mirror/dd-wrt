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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// PPP test code

#if 0
#include <network.h>

#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <pkgconf/ppp.h>

#include <cyg/ppp/ppp.h>

#include <cyg/io/io.h>
#include <cyg/io/serialio.h>

#include <cyg/infra/testcase.h>
#endif

#include "ppp_test_support.inl"



//==========================================================================
    

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

//==========================================================================

static const char *script1[] =
{
    "ABORT"             ,       "BUSY"          ,
    "ABORT"             ,       "NO CARRIER"    ,
    ""                  ,       "Chat Test"     ,
    "CONNECT"           ,       "\\c"           ,
    "TIMEOUT"           ,       "10"            ,
    "ogin:--ogin:"      ,       "ppp"           ,
    "TIMEOUT"           ,       "5"             ,
    "assword:"          ,       "hithere"       ,
    0
};

//==========================================================================

static struct test_info
{
    char        *name;
    const char  **script;
    cyg_int32   result;
} tests[] =
{
    { "CHAT_TEST_1"     , script1       , 1 },          // Simple test to completion
    { "CHAT_TEST_2"     , script1       , 0 },          // Expects an ABORT
    { "CHAT_TEST_3"     , script1       , 0 },          // Expects a timeout
    { NULL              , NULL          , 0 }
};

//==========================================================================

void
chat_test(cyg_addrword_t p)
{
    cyg_serial_baud_rate_t old;
    cyg_int32 failures = 0;
    cyg_int32 result;
    struct test_info *test;
    
    CYG_TEST_INIT();

    CYG_TEST_INFO("Start CHAT test");

    old = ppp_test_set_baud( CYGNUM_SERIAL_BAUD_115200 );

    for( test = &tests[0]; test->name != NULL; test++ )
    {
        CYG_TEST_INFO( test->name );
        ppp_test_announce( test->name );

        result = cyg_ppp_chat( CYGPKG_PPP_TEST_DEVICE, test->script );

        diag_printf("chat result %d expected %d\n",result,test->result );
        
        if( result != test->result )
        {
            CYG_TEST_FAIL( test->name );
            failures++;
        }
        else
            CYG_TEST_PASS( test->name );

        cyg_thread_delay( 300 );
    }

    ppp_test_set_baud( old );
    
    ppp_test_finish();
    
//    ppp_test_announce( "CHAT_TEST_1" );
    
//    success = cyg_ppp_chat( CYGPKG_PPP_TEST_DEVICE, script );

//    if( !success )
//        CYG_TEST_INFO("Chat script failed");
    
    CYG_TEST_FINISH("CHAT test done");
}

//==========================================================================

void
cyg_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      chat_test,          // entry
                      0,                 // entry parameter
                      "CHAT test",        // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
}

//==========================================================================
