/*=================================================================
//
//        cpp1.c
//
//        cpp arithmetic bug regression test
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
// Author(s):     hmt
// Contributors:  hmt
// Date:          2001-04-30
//####DESCRIPTIONEND####
*/

#include <pkgconf/hal.h>

#include <pkgconf/infra.h>

#include <cyg/infra/testcase.h>

// -----------------------------------------------------------------
// This is smaller than 2048.
// Unless the parser binds '+' too strongly because it is after a ket, so
// it is mistaken for unary plus, when (4 * 20 + 2) * 4 * 20 is larger.
#define CYGNUM_HAL_STACK_SIZE_MINIMUM ((4 * 20) + 2 * 4 * 20)

#define CYGNUM_UITRON_STACK_SIZE (2048)

#ifdef CYGNUM_HAL_STACK_SIZE_MINIMUM
# ifdef CYGNUM_UITRON_STACK_SIZE
#  if CYGNUM_UITRON_STACK_SIZE < CYGNUM_HAL_STACK_SIZE_MINIMUM

// then override the configured stack size
#   undef CYGNUM_UITRON_STACK_SIZE
#   define CYGNUM_UITRON_STACK_SIZE CYGNUM_HAL_STACK_SIZE_MINIMUM

#  endif // CYGNUM_UITRON_STACK_SIZE < CYGNUM_HAL_STACK_SIZE_MINIMUM
# endif // CYGNUM_UITRON_STACK_SIZE
#endif // CYGNUM_HAL_STACK_SIZE_MINIMUM


// This tests for the bug per se:
int i = CYGNUM_UITRON_STACK_SIZE;

// This tests the workaround independently of more complex context:
#define MAX(_x_,_y_) ((_x_) > (_y_) ? (_x_) : (_y_))

static char stack1[
    MAX(
        CYGNUM_HAL_STACK_SIZE_MINIMUM,
        2048)
    ];

// Better to report a fully-fledged failure and test the workaround than
// fail early.
#if 0
# if CYGNUM_UITRON_STACK_SIZE != 2048
#  error FAIL: CPP '+' binding bug detected
# endif
#endif

// -------------------------------------------------------------------------
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_PASS("CPP '+' binding test compiled OK");
    CYG_TEST_PASS_FAIL( 2048 == i, "initialized i should be 2048" );
    CYG_TEST_PASS_FAIL( 2048 == sizeof( stack1 ),
                        "workaround: sizeof( stack1 ) should be 2048" );
    CYG_TEST_EXIT("All done");
}

// -------------------------------------------------------------------------
/* EOF cpp1.c */
