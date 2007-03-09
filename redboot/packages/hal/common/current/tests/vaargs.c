//=================================================================
//
//        vaargs.c
//
//        HAL variable argument calls test
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
// Author(s):     jskov
// Contributors:  jskov
// Date:          2001-08-03
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/infra/testcase.h>         // test case macros
#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/infra/cyg_ass.h>          // assertions

#include <cyg/hal/hal_arch.h>           // context macros

#include <stdarg.h>                     // vaargs magic

// -------------------------------------------------------------------------

int
function(int n, ...)
{
    va_list args;
    int c = 11 * n;
    int i = 1;
    int res = 1;

    CYG_ASSERT(n >= 0 && n < 8, "Invalid count argument");

    va_start(args, n);

    for (i = 1; i <= n; c++, i++) {
        int v = va_arg(args, int);
        if (v != c) {
            diag_printf("FAIL:<Bad argument: index %d expected %d got %d>\n", i, c, v);
            res = 0;
        }
    }       

    va_end(args);

    return res;
}

int
function_proxy(int n, va_list args)
{
    int c = 11 * n;
    int i = 1;
    int res = 1;

    CYG_ASSERT(n >= 0 && n < 8, "Invalid count argument");

    for (i = 1; i <= n; c++, i++) {
        int v = va_arg(args, int);
        if (v != c) {
            diag_printf("FAIL:<Bad argument: index %d expected %d got %d>\n", i, c, v);
            res = 0;
        }
    }       

    return res;
}

int
proxy(int n, ...)
{
    int res;
    va_list args;

    va_start(args, n);
    res = function_proxy(n, args);
    va_end(args);

    return res;
}


void
entry(void)
{
    int res;

    res =  function(0);
    res &= function(1, 11);
    res &= function(2, 22, 23);
    res &= function(3, 33, 34, 35);
    res &= function(4, 44, 45, 46, 47);
    res &= function(5, 55, 56, 57, 58, 59);
    res &= function(6, 66, 67, 68, 69, 70, 71);
    res &= function(7, 77, 78, 79, 80, 81, 82, 83);
    CYG_TEST_PASS_FAIL(res, "Direct vaargs calls");

    res =  proxy(0);
    res &= proxy(1, 11);
    res &= proxy(2, 22, 23);
    res &= proxy(3, 33, 34, 35);
    res &= proxy(4, 44, 45, 46, 47);
    res &= proxy(5, 55, 56, 57, 58, 59);
    res &= proxy(6, 66, 67, 68, 69, 70, 71);
    res &= proxy(7, 77, 78, 79, 80, 81, 82, 83);
    CYG_TEST_PASS_FAIL(res, "Proxy vaargs calls");

    CYG_TEST_FINISH("HAL vaargs test");
}

// -------------------------------------------------------------------------

externC void
cyg_start( void )
{
    CYG_TEST_INIT();

    entry();
}

// -------------------------------------------------------------------------
// EOF vaargs.c
