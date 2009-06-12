//=================================================================
//
//        cpuload.c
//
//        test case for the cpuload code.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Andrew Lunn
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
// Author(s):     asl
// Contributors:  asl
// Date:          2002-08-06
// Description:   Tests the calculation code
//####DESCRIPTIONEND####

#include <cyg/hal/hal_arch.h>
#include <cyg/kernel/kapi.h>
#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>
#include <cyg/cpuload/cpuload.h>

void
do_test(cyg_addrword_t data) {
  cyg_uint32 calibration;
  cyg_cpuload_t cpuload;
  cyg_handle_t handle;
  cyg_uint32 average_point1s;
  cyg_uint32 average_1s;
  cyg_uint32 average_10s;
  cyg_uint32 i,j;
  
  CYG_TEST_INFO("About to calibrate cpuload");
  
  cyg_cpuload_calibrate(&calibration);
  
  CYG_TEST_INFO("Performing 100% load test");
  
  cyg_cpuload_create(&cpuload,calibration,&handle);
  
  /* Busy loop doing useless work. This will cause the cpu load to be
     100%. After all these iterations, check the measured load really
     is 100%. */

  for (i=250; i--; ) {
    for (j=calibration*10; j-- ; ) 
      ;
    cyg_cpuload_get(handle,&average_point1s,&average_1s,&average_10s);
    /* diag_printf("%d %d %d\n",average_point1s,average_1s,average_10s);*/
  }

  /* Rounding errors mean it will probably never reach 100% */
  if ((average_point1s == 100) &&
      (average_1s >= 99) && (average_1s <=100) &&
      (average_10s >= 98) && (average_10s <= 100))
    CYG_TEST_PASS("100% load o.k.");
  else {
    CYG_TEST_FAIL("100% load");
  }
  
  /* Sleeping doing nothing. This will cause the cpuload to be
     0%. After all these iterations, check the measured load really is
     0%. */
  CYG_TEST_INFO("Performing 0% load test");
  for (i=350; i-- ; ) {
    cyg_thread_delay(10);
    cyg_cpuload_get(handle,&average_point1s,&average_1s,&average_10s);
    /* diag_printf("%d %d %d\n",average_point1s,average_1s,average_10s);*/
  }
  
  if ((average_point1s == 0) &&
      (average_1s == 0) &&
      (average_10s >= 0) && (average_10s <= 2))
    CYG_TEST_PASS_FINISH("0% load o.k.");
  else {
    CYG_TEST_FAIL_FINISH("0% load");
  }
}


externC void
cyg_start(void) {

#ifdef CYGPKG_HAL_SYNTH

  CYG_TEST_NA("Not applicable to Synth target");
#else
  static char stack[CYGNUM_HAL_STACK_SIZE_MINIMUM];
  static cyg_handle_t handle;
  static cyg_thread thread;
  
  CYG_TEST_INIT();
  
  cyg_thread_create(4,do_test,0,"cpuload",
                    stack,sizeof(stack),&handle,&thread);
  cyg_thread_resume(handle);
  
  cyg_scheduler_start();
#endif
}
