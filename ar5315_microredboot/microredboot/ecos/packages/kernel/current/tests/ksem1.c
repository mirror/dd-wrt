/*=================================================================
//
//        ksem1.c
//
//        C API semaphore test 1
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
// Date:          1998-03-20
// Description:   Tests basic semaphore functionality.
//####DESCRIPTIONEND####
*/

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

#ifdef CYGFUN_KERNEL_API_C

#include "testaux.h"

#define NTHREADS 2
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static cyg_handle_t thread[NTHREADS];

static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];


static cyg_sem_t s0, s1, s2;

static volatile cyg_ucount8 q = 0;

static void entry0( cyg_addrword_t data )
{
    cyg_ucount32 val;

    cyg_semaphore_wait(&s0);
    CHECK( 1 == q++ );
    cyg_semaphore_post(&s1);
    cyg_semaphore_wait(&s0);
    CHECK( 3 == q++ );
    cyg_semaphore_peek(&s0, &val);
    CHECK( 0 == val);
    CHECK( ! cyg_semaphore_trywait(&s0) );
    cyg_semaphore_post(&s0);
    CHECK( 4 == q++ );
    cyg_semaphore_peek(&s0, &val);
    CHECK( 1 == val);
    cyg_semaphore_post(&s0);
    cyg_semaphore_peek(&s0, &val);
    CHECK( 2 == val);
    cyg_semaphore_post(&s1);
    cyg_semaphore_peek(&s2, &val);
    CHECK( 0 == val);
    cyg_semaphore_wait(&s2);
    CHECK( 6 == q++ );
    CYG_TEST_PASS_FINISH("Kernel C API Semaphore 1 OK");
}

static void entry1( cyg_addrword_t data )
{
    cyg_count32 val;

    cyg_semaphore_peek(&s1, &val);
    CHECK( 2 == val);
    cyg_semaphore_wait(&s1);
    cyg_semaphore_peek(&s1, &val);
    CHECK( 1 == val);
    cyg_semaphore_wait(&s1);
    CHECK( 0 == q++ );
    cyg_semaphore_peek(&s0, &val);
    CHECK( 0 == val);
    cyg_semaphore_post(&s0);
    cyg_semaphore_wait(&s1);
    CHECK( 2 == q++ );
    cyg_semaphore_post(&s0);
    cyg_semaphore_wait(&s1);
    CHECK( 5 == q++ );
    cyg_semaphore_peek(&s0, &val);
    CHECK( 2 == val);
    CHECK( cyg_semaphore_trywait(&s0) );
    cyg_semaphore_peek(&s0, &val);
    CHECK( 1 == val);
    CHECK( cyg_semaphore_trywait(&s0) );
    cyg_semaphore_peek(&s0, &val);
    CHECK( 0 == val);
    cyg_semaphore_post(&s2);
    cyg_semaphore_wait(&s0);
    CYG_TEST_FAIL_FINISH("Not reached");
}

void ksem1_main( void )
{
    CYG_TEST_INIT();

    cyg_semaphore_init( &s0, 0);
    cyg_semaphore_init( &s1, 2);
    cyg_semaphore_init( &s2, 0);

    cyg_thread_create(4, entry0 , (cyg_addrword_t)0, "ksem1-0",
        (void *)stack[0], STACKSIZE,&thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);

    cyg_thread_create(4, entry1 , (cyg_addrword_t)1, "ksem1-1",
        (void *)stack[1], STACKSIZE, &thread[1], &thread_obj[1]);
    cyg_thread_resume(thread[1]);

    cyg_scheduler_start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{ 
    ksem1_main();
}


#else /* def CYGFUN_KERNEL_API_C */
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel C API layer disabled");
}
#endif /* def CYGFUN_KERNEL_API_C */

/* EOF ksem1.c */
