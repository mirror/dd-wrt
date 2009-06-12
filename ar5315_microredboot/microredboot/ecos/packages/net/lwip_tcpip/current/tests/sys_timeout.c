//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 eCosCentric
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================

/* Simple test-case for the lwip system timeout functionality  */

#include "lwip/sys.h"

#define TIMERS 100
sys_timeout_handler timers[TIMERS];
int intervals[TIMERS];

static void
timeout_fn ( void * arg)
{
	int i, j = (int)arg;
//	diag_printf("Timeout #%d called\n", (int)arg);
	for (i = 0; i < j; i++)
		diag_printf(" ");
	diag_printf("%d\n", j);
//	sys_timeout( intervals[j], timers[j], arg);	
	sys_timeout( 1000, timers[j], arg);	
}

static void
timeout_thread(void *arg)
{
  int i;
  
  for (i = 0; i < TIMERS; i++) {
	  diag_printf("Adding timer #%d\n", i);
	  timers[i] = timeout_fn;
	  intervals[i] = (i+1)*1000;	
	  sys_timeout(intervals[i],  timers[i], (void *)i);
  }
  sys_msleep(0); 
  diag_printf("Done\n");
}

void
tmain(cyg_addrword_t p)
{
  lwip_init();	
  sys_thread_new(timeout_thread, (void*)"timeout",7);
}

#define STACK_SIZE 0x1000
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

void
cyg_user_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      tmain,          // entry
                      0,                 // entry parameter
                      "timeout test",        // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
}

