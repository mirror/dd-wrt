//==========================================================================
//
//      nmitest.c
//
//      V850 NMI test program
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
// Author(s):    jlarmour
// Contributors: jlarmour
// Date:         2001-02-01
// Purpose:      Test NMI function
// Description:  This test allows the NMI functionality to be tested.
//               It is not built by default.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <cyg/hal/hal_intr.h>
#include <cyg/infra/diag.h>

#include <cyg/kernel/kapi.h>

int nmicount;
char stack[4096];
static cyg_handle_t thr0h;
static cyg_thread thr0;

void
nmi_handler(cyg_addrword_t data, cyg_code_t exception_number,
            cyg_addrword_t info)
{
    nmicount++;
}

static void
entry0( cyg_addrword_t data )
{
    int oldnmicount=0;

    cyg_exception_set_handler( CYGNUM_HAL_VECTOR_NMI,
                               &nmi_handler,
                               0,
                               NULL,
                               NULL );
    for (;;) {
        if ( oldnmicount != nmicount ) {
            diag_printf("Caught NMI\n");
            oldnmicount = nmicount;
        }
    }
}

void
cyg_user_start(void)
{
    cyg_thread_create(4, entry0, (cyg_addrword_t)0, "thread 0",
                      (void *)&stack[0], sizeof(stack), &thr0h, &thr0 );
    cyg_thread_resume(thr0h);
}

// EOF nmitest.c
