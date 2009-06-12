//=================================================================
//
//        sntp1.c
//
//        Simple Network Time Protocol test 1
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Andrew Lunn
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
// Author(s):     Andrew Lunn
// Contributors:  
// Date:          2003-02-11
// Description:   Tests the sntp client
//####DESCRIPTIONEND####

#include <pkgconf/isoinfra.h>
#include <cyg/infra/testcase.h>

#if defined(CYGINT_ISO_STDIO_FORMATTED_IO) && defined(CYGINT_ISO_STRING_STRFUNCS) 
#include <network.h>
#include <time.h>
#include <cyg/sntp/sntp.h>
#include <stdio.h>

#define SECONDSPERMINUTE (cyg_uint32)60
#define SECONDSPERHOUR   (cyg_uint32)(SECONDSPERMINUTE * 60)
#define SECONDSPERDAY    (cyg_uint32)(SECONDSPERHOUR * 24)
#define SECONDSPERYEAR   (cyg_uint32)(SECONDSPERDAY * 365)

void
net_test(cyg_addrword_t param)
{
  int seconds;
  time_t now, build_time;
  struct tm tm={ 0,0,0,0,0,0,0,0,0 };
  int i, loop, waittime;
  char month[4];
  char months[12][4] = { "Jan", "Feb", "Mar", 
			 "Apr", "May", "Jun", 
			 "Jul", "Aug", "Sep",
			 "Oct", "Nov", "Dec" };
  char time_info[32];
  
  CYG_TEST_INIT();

  CYG_TEST_INFO("sntp1 test build " __DATE__);

  init_all_network_interfaces();

  cyg_sntp_start();

  /* The SNTP client will try to obtain NTP time updates by
   * listening for multicasts.  It can also be configured
   * to send unicast requests to specific NTP servers.  By
   * default, unicast NTP servers are obtained from DHCP.
   *
   * If unicast mode is enabled, the run the test loop twice.
   * The first time, unicast requests will be sent.  The
   * second time, the unicast list will be unconfigured and
   * the client will listen only for multicasts.
   *
   * Note that this test is somewhat bogus since multicast
   * NTP packets will actually allow both test loops to
   * pass.  But it is the best we can do for the automated
   * test, so consider this more of a usage example.
   */
#ifdef CYGPKG_NET_SNTP_UNICAST
  loop = 2;
#else
  loop = 1;
#endif
  while (loop-- > 0)
  {
    if (loop == 1) {
      CYG_TEST_INFO("Testing SNTP unicast mode.");
      waittime=14;
    } else {
      CYG_TEST_INFO("Testing SNTP multicast mode.");
      waittime=20;
    }
    for (seconds = waittime; seconds > 0; seconds--) {
      now = time(NULL);
      ctime_r(&now, time_info);
      time_info[strlen(time_info)-1] = '\0';  // Strip \n
      CYG_TEST_INFO(time_info);
      cyg_thread_delay(100);
    }
  
    now = time(NULL);

    if ( now < (5 * 60)) {
      CYG_TEST_FAIL_FINISH("Nothing recieved from the SNTP server");
    } else {
    
      i=sscanf(__DATE__, "%s %d %d",month,&tm.tm_mday,&tm.tm_year);
      CYG_ASSERT(3==i,"sscanf did not return enough results");
      for (i=0; i < 12; i++) {
        if (!strcmp(month,months[i])) 
          break;
      }
      tm.tm_mon = i;
      tm.tm_year -= 1900;

      build_time = mktime(&tm);
      CYG_ASSERT(-1 != build_time,"mktime returned -1");

      if (build_time > time(NULL)) {
        CYG_TEST_FAIL_FINISH("Build time is ahead of SNTP time");
      } else {
        if ((build_time + 60 * 60 * 24 * 90) < time(NULL)) {
  	      CYG_TEST_FAIL_FINISH("Build time is more than 90 days old");
        }
      }
    }

#ifdef CYGPKG_NET_SNTP_UNICAST
    /* For the second pass of the test, we set the time
     * back to epoch and unconfigure the list of SNTP
     * unicast servers.  This will test non-unicast mode.
     */
    cyg_sntp_set_servers(NULL, 0);
    cyg_libc_time_settime(0);
#endif
  }
  CYG_TEST_PASS_FINISH("sntp1 test is complete");
}

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL*2)
static char thread_stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

void
cyg_user_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                  // Priority - just a number
                      net_test,            // entry
                      0,                   // entry parameter
                      "Network test",      // Name
                      thread_stack,        // Stack
                      STACK_SIZE,          // Size
                      &thread_handle,      // Handle
                      &thread_data         // Thread data structure
            );
    cyg_thread_resume(thread_handle);      // Start it
}

#else //defined(CYGINT_ISO_STDIO_FORMATTED_IO) && defined(CYGINT_ISO_STRING_STRFUNCS)

void cyg_user_start(void) 
{
  CYG_TEST_NA("Not all the required packages are available");
}

#endif
