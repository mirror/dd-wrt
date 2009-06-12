//==========================================================================
//
//      cpuload.cxx
//
//      Estimate the current CPU load.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):    Andrew Lunn
// Contributors: Andrew Lunn
// Date:         2002-08-12
// Purpose:      
// Description:  
//    This provides a simple CPU load meter. All loads are returned
//    as a percentage, ie 0-100. This is only a rough measure. Any clever
//    power management, sleep modes etc, will cause these results to be
//    wrong.
//              
// This code is part of eCos (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h> 
#include <cyg/cpuload/cpuload.h>

/* Here we run the idle thread as a high priority thread for 0.1
   second.  We see how much the counter in the idle loop is
   incremented. This will only work if there are no other threads
   running at priority 1 and 2 */

static cyg_thread thread;
externC cyg_uint32 idle_thread_loops;
char idle_stack[CYGNUM_HAL_STACK_SIZE_MINIMUM];

extern void idle_thread_main( CYG_ADDRESS data );

// -------------------------------------------------------------------------
/* We are playing ping-pong with the cpuload_calibrate function and
   the idle thread. First cpuload_calibrate runs. Next the idle
   thread, then this alarm function and then back to the
   cpuload_calibrate function. */

void static 
alarm_func(cyg_handle_t alarm,cyg_addrword_t data) { 
  cyg_handle_t idleH = (cyg_handle_t) data;
  cyg_thread_suspend(idleH); 
}


/* Calibrate the cpuload measurements.*/
externC void 
cyg_cpuload_calibrate(cyg_uint32  *calibration) {
  cyg_handle_t counter;
  cyg_alarm alarm_s;
  cyg_handle_t alarmH;
  cyg_uint32 idle_loops_start;  
  cyg_handle_t idleH;
  cyg_priority_t old_priority;
  
  cyg_thread_create(1,
		    idle_thread_main,
		    0,
		    "Calibration idle thread",
		    idle_stack,
		    sizeof(idle_stack),
		    &idleH,
		    &thread);
  
  cyg_clock_to_counter(cyg_real_time_clock(),&counter);
  cyg_alarm_create(counter,alarm_func,(cyg_addrword_t)idleH,&alarmH,&alarm_s);
  
  cyg_alarm_initialize(alarmH,cyg_current_time()+10,0);
  cyg_alarm_enable(alarmH);
  
  idle_loops_start = idle_thread_loops;
  
  /* Dont be decieved, remember this is a multithreaded system ! */
  old_priority = cyg_thread_get_priority(cyg_thread_self());
  cyg_thread_set_priority(cyg_thread_self(),2);
  cyg_thread_resume(idleH);
  cyg_thread_set_priority(cyg_thread_self(),old_priority);
  
  *calibration = idle_thread_loops - idle_loops_start;
  cyg_alarm_delete(alarmH);
  cyg_thread_kill(idleH);
  cyg_thread_delete(idleH);
}

static void 
cpuload_alarm_func(cyg_handle_t alarm,cyg_addrword_t data) { 
  cyg_cpuload_t * cpuload = (cyg_cpuload_t *)data;
  cyg_uint32 idle_loops_now = idle_thread_loops;
  cyg_uint32 idle_loops;
  cyg_uint32 load;
  
  if (idle_loops_now >= cpuload->last_idle_loops) {
    idle_loops = idle_loops_now - cpuload->last_idle_loops;
  } else {
    idle_loops = ~0 - (cpuload->last_idle_loops - idle_loops_now);
  }
  
  /* We need 64 bit arithmatic to prevent wrap around */ 
  load = (cyg_uint32) (((cyg_uint64) idle_loops * (cyg_uint64)100) / 
		       (cyg_uint64)cpuload->calibration);
  if (load > 100) {
    load = 100;
  }
  load = 100 - load;
  
  cpuload->average_point1s = load;
  cpuload->average_1s = load + ((cpuload->average_1s * 90)/100);
  cpuload->average_10s = load + ((cpuload->average_10s * 99)/100);
  cpuload->last_idle_loops = idle_loops_now;
}

/* Create a CPU load measurements object and start the
   measurements. */
externC void 
cyg_cpuload_create(cyg_cpuload_t *cpuload,
                   cyg_uint32 calibration, 
                   cyg_handle_t *handle) 
{
  cyg_handle_t counter;
  
  cpuload->average_point1s = 0;
  cpuload->average_1s = 0;
  cpuload->average_10s = 0;
  cpuload->calibration = calibration;
  cpuload->last_idle_loops = idle_thread_loops;

  cyg_clock_to_counter(cyg_real_time_clock(),&counter);
  cyg_alarm_create(counter,
		   cpuload_alarm_func,
		   (cyg_addrword_t)cpuload,
		   &cpuload->alarmH,
		   &cpuload->alarm_s);

  cyg_alarm_initialize(cpuload->alarmH,cyg_current_time()+10,10);
  cyg_alarm_enable(cpuload->alarmH);
  
  *handle = (cyg_handle_t) cpuload;
}

/* Stop measurements of the cpuload. The cyg_cpuload_t object can then
   be freed. */
externC void 
cyg_cpuload_delete(cyg_handle_t handle) {
  cyg_cpuload_t * cpuload = (cyg_cpuload_t *) handle;
  
  cyg_alarm_delete(cpuload->alarmH);
}  

/* Return the cpuload for the last 100ms, 1seconds and 10 second */
externC void
cyg_cpuload_get(cyg_handle_t handle,
                cyg_uint32 *average_point1s, 	    
                cyg_uint32 *average_1s, 	    
                cyg_uint32 *average_10s) {

  cyg_cpuload_t * cpuload = (cyg_cpuload_t *) handle;
  *average_point1s = cpuload->average_point1s;
  *average_1s = cpuload->average_1s/10;
  *average_10s = cpuload->average_10s/100;
}




