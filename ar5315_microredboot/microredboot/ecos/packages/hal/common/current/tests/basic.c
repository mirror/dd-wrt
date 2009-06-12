//=================================================================
//
//        basic.c
//
//        HAL basic functions test
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
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          2001-10-24
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/infra/testcase.h>         // test case macros
#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/infra/cyg_ass.h>          // assertions

#include <cyg/hal/hal_arch.h>           // context macros

// -------------------------------------------------------------------------

#define BITS(t) (sizeof(t)*8)

void
entry(void)
{
    int res;
    int i, mask, ndx;
    hal_jmp_buf jmpbuf;

    res = 1;

    // Check HAL_MSBIT_NDEX() functions
    mask = 1;  // One bits set
    for (i = 0;  i < BITS(int);  i++) {
        HAL_MSBIT_INDEX(ndx, mask);
        res &= (ndx == i);
        HAL_LSBIT_INDEX(ndx, mask);
        res &= (ndx == i);
        mask <<= 1;
    }

    mask = 3;  // Two bits set
    for (i = 0;  i < BITS(int)-1;  i++) {
        HAL_MSBIT_INDEX(ndx, mask);
        res &= (ndx == (i+1));
        HAL_LSBIT_INDEX(ndx, mask);
        res &= (ndx == i);
        mask <<= 1;
    }
    CYG_TEST_PASS_FAIL(res, "HAL_xSBIT_INDEX() basic functions");

    res = 0;
    if (hal_setjmp(jmpbuf)) {
        res = 1;
    } else {
        hal_longjmp(jmpbuf, 1);
    }
    CYG_TEST_PASS_FAIL(res, "hal_setjmp()/hal_longjmp() basic functions");

    CYG_TEST_FINISH("HAL basic functions test");
}

// -------------------------------------------------------------------------

externC void
cyg_start( void )
{
    CYG_TEST_INIT();

    entry();
}

// -------------------------------------------------------------------------
// EOF basic.c
