/*=================================================================
//
//        kthread0.c
//
//        Kernel C API Thread test 0
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
// Description:   Limited to checking constructors/destructors
//####DESCRIPTIONEND####
*/

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

#ifdef CYGFUN_KERNEL_API_C

#include "testaux.h"

#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static char stack[STACKSIZE];

static cyg_thread_entry_t entry;

static void entry( cyg_addrword_t data )
{
}

static int *p;

#if 0
static cyg_handle_t t0,t1;
static cyg_thread thread0, thread1;
#endif

static cyg_handle_t t2;
static cyg_thread thread2;

static bool flash( void )
{
#if 0 // no facility to allocate stack exists yet.
    cyg_thread_create( entry, 0x111, NULL, 0, &t0, &thread0 );

    cyg_thread_create( entry, (cyg_addrword_t)&t0, STACKSIZE, 0, &t1, &thread0 );
#endif

    cyg_thread_create(4, entry, (cyg_addrword_t)p, "kthread0",
        (void *)stack, STACKSIZE, &t2, &thread2 );

    return true;
}

void kthread0_main( void )
{
    CYG_TEST_INIT();

    CHECK(flash());
    CHECK(flash());
    
    CYG_TEST_PASS_FINISH("Kernel C API Thread 0 OK");
    
}

externC void
cyg_start( void )
{ 
    kthread0_main();
}

#else /* def CYGFUN_KERNEL_API_C */
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel C API layer disabled");
}
#endif /* def CYGFUN_KERNEL_API_C */

/* EOF kthread0.c */
