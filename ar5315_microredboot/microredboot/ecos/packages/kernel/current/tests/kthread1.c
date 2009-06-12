/*=================================================================
//
//        kthread1.c
//
//        Kernel C API Thread test 1
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
// Author(s):     dsm
// Contributors:    dsm
// Date:          1998-03-18
// Description:   Tests some basic thread functions.
//####DESCRIPTIONEND####
*/

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

#ifdef CYGFUN_KERNEL_API_C

#include "testaux.h"

#include <cyg/hal/hal_arch.h>           // for CYGNUM_HAL_STACK_SIZE_TYPICAL

#ifdef CYGNUM_HAL_STACK_SIZE_TYPICAL
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
#else
#define STACKSIZE 2000
#endif

static char stack[2][STACKSIZE];

static cyg_thread thread[2];

static cyg_handle_t pt0,pt1;


static void entry0( cyg_addrword_t data )
{
    CHECK( 222 == (int)data );

    cyg_thread_suspend(pt1);       
    cyg_thread_resume(pt1);

    cyg_thread_delay(1);

    cyg_thread_resume(pt1);

    cyg_thread_delay(1);

    CYG_TEST_PASS_FINISH("Kernel C API Thread 1 OK");
}

static void entry1( cyg_addrword_t data )
{
    cyg_handle_t self;
    CHECK( 333 == (int)data );

    self = cyg_thread_self();
   
    CHECK( self == pt1 );

    cyg_thread_suspend(pt1);

    cyg_thread_exit();          // no guarantee this will be called
}

void kthread1_main( void )
{
    CYG_TEST_INIT();

    cyg_thread_create(4, entry0, (cyg_addrword_t)222, "kthread1-0",
        (void *)stack[0], STACKSIZE, &pt0, &thread[0] );
    cyg_thread_create(4, entry1, (cyg_addrword_t)333, "kthread1-1",
        (void *)stack[1], STACKSIZE, &pt1, &thread[1] );

    cyg_thread_resume(pt0);
    cyg_thread_resume(pt1);

    cyg_scheduler_start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{
    kthread1_main();
}


#else /* def CYGFUN_KERNEL_API_C */
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel C API layer disabled");
}
#endif /* def CYGFUN_KERNEL_API_C */

/* EOF kthread1.c */
