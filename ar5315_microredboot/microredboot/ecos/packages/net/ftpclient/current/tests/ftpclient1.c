//==========================================================================
//
//      tests/dns1.c
//
//      Simple test of DNS client support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Andrew Lunn.
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
// Author(s):    andrew.lunn
// Contributors: andrew.lunn, jskov
// Date:         2001-09-18
// Purpose:      
// Description:  Test FTPClient functions. Note that the _XXX defines below
//               control what addresses the test uses. These must be
//               changed to match the particular testing network in which
//               the test is to be run.
//              
//####DESCRIPTIONEND####
//
//==========================================================================
 
#include <network.h>
#include <ftpclient.h>
#include <cyg/infra/testcase.h>
 
#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;
 
#define __string(_x) #_x
#define __xstring(_x) __string(_x)
 
#define _FTP_SRV           __xstring(172.16.19.254) // farmnet dns0 address
#define _FTP_SRV_V6         __xstring(fec0:0:0:2::1)
#define FTPBUFSIZE (1024 * 64)
char ftpbuf[FTPBUFSIZE];
char ftpbuf1[FTPBUFSIZE];

void
ftp_test(cyg_addrword_t p)
{
  int ret;

  CYG_TEST_INIT();
  
  init_all_network_interfaces();
 
  CYG_TEST_INFO("Getting /etc/passwd from " _FTP_SRV);
  ret = ftp_get(_FTP_SRV,"anonymous","ftpclient1",
                 "/etc/passwd",ftpbuf,FTPBUFSIZE,
                ftpclient_printf);

  if (ret > 0) {
    diag_printf("PASS:< %d bytes received>\n",ret);
  } else {
    diag_printf("FAIL:< ftp_get returned %d>\n",ret);
  }

  CYG_TEST_INFO("Putting passwd file back in /incoming/passwd\n");
  ret = ftp_put(_FTP_SRV,"anonymous","ftpclient1",
                "/incoming/passwd",ftpbuf,ret,
                ftpclient_printf);
  
  if (ret > 0) {
    diag_printf("PASS:\n");
  } else {
    diag_printf("FAIL:< ftp_get returned %d>\n",ret);
  }

  CYG_TEST_INFO("Reading back /incoming/passwd\n");
  ret = ftp_get(_FTP_SRV,"anonymous","ftpclient1",
                 "/incoming/passwd",ftpbuf1,FTPBUFSIZE,
                ftpclient_printf);
  
  if (ret > 0) {
    diag_printf("PASS:< %d bytes received>\n",ret);
  } else {
    diag_printf("FAIL:< ftp_get returned %d>\n",ret);
  }

  CYG_TEST_PASS_FAIL(!memcmp(ftpbuf,ftpbuf1,ret),"Transfer integrity");

#ifdef CYGPKG_NET_INET6
  CYG_TEST_INFO("Getting /etc/passwd from " _FTP_SRV_V6);
  ret = ftp_get(_FTP_SRV_V6,"anonymous","ftpclient1",
                 "/etc/passwd",ftpbuf,FTPBUFSIZE,
                ftpclient_printf);

  if (ret > 0) {
    diag_printf("PASS:< %d bytes received>\n",ret);
  } else {
    diag_printf("FAIL:< ftp_get returned %d>\n",ret);
  }

  CYG_TEST_INFO("Putting passwd file back in /incoming/passwd\n");
  ret = ftp_put(_FTP_SRV_V6,"anonymous","ftpclient1",
                "/incoming/passwd",ftpbuf,ret,
                ftpclient_printf);
  
  if (ret > 0) {
    diag_printf("PASS:\n");
  } else {
    diag_printf("FAIL:< ftp_get returned %d>\n",ret);
  }

  CYG_TEST_INFO("Reading back /incoming/passwd\n");
  ret = ftp_get(_FTP_SRV_V6,"anonymous","ftpclient1",
                 "/incoming/passwd",ftpbuf1,FTPBUFSIZE,
                ftpclient_printf);
  
  if (ret > 0) {
    diag_printf("PASS:< %d bytes received>\n",ret);
  } else {
    diag_printf("FAIL:< ftp_get returned %d>\n",ret);
  }

  CYG_TEST_PASS_FAIL(!memcmp(ftpbuf,ftpbuf1,ret),"Transfer integrity");

#endif

  CYG_TEST_INFO("ftp_Get'ing with a bad username\n");
  ret = ftp_get(_FTP_SRV,"nosuchuser","ftpclient1",
                "/incoming/passwd",ftpbuf1,FTPBUFSIZE,
                ftpclient_printf);
  CYG_TEST_PASS_FAIL(ret==FTP_BADUSER,"Bad Username");

  CYG_TEST_INFO("ftp_get'ting with a bad passwd\n");
  ret = ftp_get(_FTP_SRV,"nobody","ftpclient1",
                "/incoming/passwd",ftpbuf1,FTPBUFSIZE,
                ftpclient_printf);
  CYG_TEST_PASS_FAIL(ret==FTP_BADUSER,"Bad passwd");

  CYG_TEST_INFO("ftp_get'ing from a with a bad passwd\n");
  ret = ftp_get(_FTP_SRV,"nobody","ftpclient1",
                "/incoming/passwd",ftpbuf1,FTPBUFSIZE,
                ftpclient_printf);
  CYG_TEST_PASS_FAIL(ret==FTP_BADUSER,"Bad passwd");

  CYG_TEST_INFO("ftp_get'ing from a bad server\n");
  ret = ftp_get("127.0.0.1","nobody","ftpclient1",
                "/incoming/passwd",ftpbuf1,FTPBUFSIZE,
                ftpclient_printf);
  CYG_TEST_PASS_FAIL(ret==FTP_NOSUCHHOST,"Bad server");

  CYG_TEST_INFO("ftp_get'ing a file which is too big");
  ret = ftp_get(_FTP_SRV,"anonymous","ftpclient1",
                "/incoming/passwd",ftpbuf,2,
                ftpclient_printf);
  CYG_TEST_PASS_FAIL(ret==FTP_TOOBIG,"File too big");
}

void
cyg_start(void)
{
  /* Create a main thread, so we can run the scheduler and have time 'pass' */
  cyg_thread_create(10,                // Priority - just a number
                    ftp_test,          // entry
                    0,                 // entry parameter
                    "FTP test",        // Name
                    &stack[0],         // Stack
                    STACK_SIZE,        // Size
                    &thread_handle,    // Handle
                    &thread_data       // Thread data structure
                    );
  cyg_thread_resume(thread_handle);  /* Start it */
  cyg_scheduler_start();
}
