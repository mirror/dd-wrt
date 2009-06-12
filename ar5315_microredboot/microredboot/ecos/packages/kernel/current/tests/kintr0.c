/*=================================================================
//
//        kintr0.c
//
//        Kernel C API Intr test 0
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
// Contributors:  dsm, jlarmour
// Date:          1999-02-16
// Description:   Very basic test of interrupt objects
// Options:
//     CYGIMP_KERNEL_INTERRUPTS_DSRS_TABLE
//     CYGIMP_KERNEL_INTERRUPTS_DSRS_TABLE_MAX
//     CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST
//####DESCRIPTIONEND####
*/

#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_intr.h>

#include <cyg/infra/testcase.h>

#ifdef CYGFUN_KERNEL_API_C

#include "testaux.h"

static cyg_interrupt intr_obj[2];

static cyg_handle_t intr0, intr1;


static cyg_ISR_t isr0, isr1;
static cyg_DSR_t dsr0, dsr1;

static cyg_uint32 isr0(cyg_vector_t vector, cyg_addrword_t data)
{
    CYG_UNUSED_PARAM(cyg_addrword_t, data);

    cyg_interrupt_acknowledge(vector);
    return 0;
}

static void dsr0(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    CYG_UNUSED_PARAM(cyg_vector_t, vector);
    CYG_UNUSED_PARAM(cyg_ucount32, count);
    CYG_UNUSED_PARAM(cyg_addrword_t, data);
}

static cyg_uint32 isr1(cyg_vector_t vector, cyg_addrword_t data)
{
    CYG_UNUSED_PARAM(cyg_vector_t, vector);
    CYG_UNUSED_PARAM(cyg_addrword_t, data);
    return 0;
}

static void dsr1(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    CYG_UNUSED_PARAM(cyg_vector_t, vector);
    CYG_UNUSED_PARAM(cyg_ucount32, count);
    CYG_UNUSED_PARAM(cyg_addrword_t, data);
}

static bool flash( void )
{
    cyg_handle_t handle;
    cyg_interrupt intr;

    cyg_interrupt_create(CYGNUM_HAL_ISR_MIN, 0, (cyg_addrword_t)333, 
                         isr0, dsr0, &handle, &intr );
    cyg_interrupt_delete(handle);

    return true;
}

/* IMPORTANT: The calling convention for VSRs is target dependent.  It is
 * unlikely that a plain C or C++ routine would function correctly on any
 * particular platform, even if it could correctly access the system
 * resources necessary to handle the event that caused it to be called.
 * VSRs usually must be written in assembly language.
 * 
 * This is just a test program.  The routine vsr0() below is defined simply
 * to define an address that will be in executable memory.  If an event
 * causes this VSR to be called, all bets are off.  If it is accidentally
 * installed in the vector for the realtime clock, the system will likely
 * freeze.
 */

static cyg_VSR_t vsr0;

static void vsr0()
{
}

void kintr0_main( void )
{
    cyg_vector_t v = (CYGNUM_HAL_VSR_MIN + 11) % CYGNUM_HAL_VSR_COUNT;
    cyg_vector_t v1;
    cyg_vector_t lvl1 = CYGNUM_HAL_ISR_MIN + (1 % CYGNUM_HAL_ISR_COUNT);
    cyg_vector_t lvl2 = CYGNUM_HAL_ISR_MIN + (15 % CYGNUM_HAL_ISR_COUNT);
    int in_use;

    cyg_VSR_t *old_vsr, *new_vsr;

    CYG_TEST_INIT();
 
#ifdef CYGPKG_HAL_MIPS_TX39    
    // This can be removed when PR 17831 is fixed
    if ( cyg_test_is_simulator )
        v1 = 12 % CYGNUM_HAL_ISR_COUNT;
    else /* NOTE TRAILING ELSE... */
#endif
    v1 = CYGNUM_HAL_ISR_MIN + ( 6 % CYGNUM_HAL_ISR_COUNT);

    CHECK(flash());
    CHECK(flash());

    // Make sure the chosen levels are not already in use.
    HAL_INTERRUPT_IN_USE( lvl1, in_use );
    intr0 = 0;
    if (!in_use)
        cyg_interrupt_create(lvl1, 1, (cyg_addrword_t)777, isr0, dsr0, 
                             &intr0, &intr_obj[0]);
    
    HAL_INTERRUPT_IN_USE( lvl2, in_use );
    intr1 = 0;
    if (!in_use && lvl1 != lvl2)
        cyg_interrupt_create(lvl2, 1, 888, isr1, dsr1, &intr1, &intr_obj[1]);

    // Check these functions at least exist

    cyg_interrupt_disable();
    cyg_interrupt_enable();

    if (intr0)
        cyg_interrupt_attach(intr0);
    if (intr1)
        cyg_interrupt_attach(intr1);
    if (intr0)
        cyg_interrupt_detach(intr0);
    if (intr1)
        cyg_interrupt_detach(intr1);

    // If this attaching interrupt replaces the previous interrupt
    // instead of adding to it we could be in a big mess if the
    // vector is being used by something important.
        
    cyg_interrupt_get_vsr( v, &old_vsr );
    cyg_interrupt_set_vsr( v, vsr0 );
    cyg_interrupt_get_vsr( v, &new_vsr );
    CHECK( vsr0 == new_vsr );

    new_vsr = NULL;
    cyg_interrupt_get_vsr( v, &new_vsr );
    cyg_interrupt_set_vsr( v, old_vsr );
    CHECK( new_vsr == vsr0 );

    cyg_interrupt_set_vsr( v, new_vsr );
    new_vsr = NULL;
    cyg_interrupt_get_vsr( v, &new_vsr );       
    CHECK( vsr0 == new_vsr );

    cyg_interrupt_set_vsr( v, old_vsr );
    CHECK( vsr0 == new_vsr );
    new_vsr = NULL;
    cyg_interrupt_get_vsr( v, &new_vsr );
    CHECK( old_vsr == new_vsr );
        
    CHECK( NULL != vsr0 );

    cyg_interrupt_mask(v1);
    cyg_interrupt_unmask(v1);
        
    cyg_interrupt_configure(v1, true, true);

    CYG_TEST_PASS_FINISH("Kernel C API Intr 0 OK");
}

externC void
cyg_start( void )
{ 
    kintr0_main();
}

#else /* def CYGFUN_KERNEL_API_C */
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel C API layer disabled");
}
#endif /* def CYGFUN_KERNEL_API_C */

/* EOF kintr0.c */
