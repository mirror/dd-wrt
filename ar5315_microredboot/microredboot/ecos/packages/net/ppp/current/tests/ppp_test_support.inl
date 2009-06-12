//==========================================================================
//
//      tests/ppp_test_support.c
//
//      PPP testing support routines
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
// Date:         2003-07-01
// Purpose:      
// Description:  This file contains routines that are used by the PPP tests.
//
//####DESCRIPTIONEND####
//
//==========================================================================

// PPP test support

#include <network.h>

#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <pkgconf/ppp.h>

#include <cyg/ppp/ppp.h>

#include <cyg/io/io.h>
#include <cyg/io/serialio.h>

#include <stdio.h>

#include <cyg/infra/testcase.h>

//==========================================================================

static cyg_serial_baud_rate_t ppp_test_set_baud( cyg_serial_baud_rate_t baud )
{
#ifdef CYGPKG_PPP_TESTS_AUTOMATE
    cyg_io_handle_t     handle;
    cyg_serial_info_t   cfg;
    int                 err;
    int                 len = sizeof(cfg);
    char                buf[30];
    int                 blen;
    cyg_serial_baud_rate_t old_baud;
    

    CYG_TEST_INFO( "Switching baud rate" );
    
    err = cyg_io_lookup( CYGPKG_PPP_TEST_DEVICE, &handle );

    if( err != 0 ) {
        CYG_TEST_INFO("cyg_io_lookup: failed");
    }

    sprintf( buf, "BAUD:%03d\r\n", baud );
    blen = strlen(buf);
    
    err = cyg_io_write( handle, buf, &blen );

    if( err != 0 ) {
        CYG_TEST_INFO("cyg_io_write: failed");
    }

    // Wait for those characters to reach the remote end
    cyg_thread_delay(200);

    err = cyg_io_get_config( handle,
                             CYG_IO_GET_CONFIG_SERIAL_INFO,
                             &cfg,
                             &len);

    if( err != 0 ) {
        CYG_TEST_INFO("cyg_io_get_config: failed");
    }

    cfg.flags |= CYGNUM_SERIAL_FLOW_RTSCTS_RX|CYGNUM_SERIAL_FLOW_RTSCTS_TX;

    old_baud = cfg.baud;
    cfg.baud = baud;
    
    err = cyg_io_set_config( handle,
                             CYG_IO_SET_CONFIG_SERIAL_INFO,
                             &cfg,
                             &len);

    if( err != 0 ) {
        CYG_TEST_INFO("cyg_io_set_config failed");
    }

    // Wait for other end to switch...
    cyg_thread_delay(100);

    CYG_TEST_INFO( "Baud rate switched" );

    return old_baud;

#else
    return 0;
#endif


}

//==========================================================================

static void ppp_test_announce( char *buf )
{
#ifdef CYGPKG_PPP_TESTS_AUTOMATE    
    cyg_io_handle_t     handle;
    int                 err;
    int                 blen;

//    cyg_thread_delay(200);

    CYG_TEST_INFO( "Sending announce string" );
    
    err = cyg_io_lookup( CYGPKG_PPP_TEST_DEVICE, &handle );

    if( err != 0 ) {
        CYG_TEST_INFO("cyg_io_lookup: failed");
    }

    blen = strlen(buf);
  
    err = cyg_io_write( handle, buf, &blen );

    if( err != 0 ) {
        CYG_TEST_INFO("cyg_io_write: failed");
    }

    blen = 2;
    err = cyg_io_write( handle, "\r\n", &blen );

    if( err != 0 ) {
        CYG_TEST_INFO("cyg_io_write: failed");
    }

    cyg_thread_delay(200);        

    CYG_TEST_INFO( "Announce string sent" );
#endif
}

//==========================================================================

static void ppp_test_finish(void)
{
#ifdef CYGPKG_PPP_TESTS_AUTOMATE    
    cyg_io_handle_t     handle;
    int                 err;
    char                *buf = "FINISH\r\n";
    int                 blen;

//    cyg_thread_delay(200);

    CYG_TEST_INFO( "Sending FINISH" );
    
    err = cyg_io_lookup( CYGPKG_PPP_TEST_DEVICE, &handle );

    if( err != 0 ) {
        CYG_TEST_INFO("cyg_io_lookup: failed");
    }

    blen = strlen(buf);
  
    err = cyg_io_write( handle, buf, &blen );

    if( err != 0 ) {
        CYG_TEST_INFO("cyg_io_write: failed");
    }

    cyg_thread_delay(200);        

    CYG_TEST_INFO( "FINISH sent" );
#endif
}

//==========================================================================
