/*=================================================================
//
//        slebstak.c
//
//        SPARClite HAL exception and register manipulation test
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
// Contributors:    dsm, nickg
// Date:          1998-06-18
//####DESCRIPTIONEND####
*/

#include <pkgconf/system.h>

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>

#include <cyg/infra/cyg_ass.h>

#include <cyg/infra/testcase.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_clock.h>

#include <pkgconf/infra.h>

#include <cyg/infra/diag.h>

// -------------------------------------------------------------------------

cyg_uint32 fact( cyg_uint32 arg )
{
    cyg_uint32 ret = (arg < 2) ? 1 : (arg * fact( arg - 1 ));
    return ret;
}


// -------------------------------------------------------------------------

int i, j=0, bit;
int level[8];
#define led (*(volatile char *)(0x02000003))
int lval = 0;

int sequence[] = { 0, 0, 1, 1, 2, 3, 4, 5, 6, 6, 7, 7, 7, 7, 6, 6, 5, 4, 3, 2, 1, 1, 0, 0,
 };

int nseq=(sizeof(sequence)/sizeof(sequence[0])-1);

void set_leds( void ) // and decay them too
{
  int i, j;
  for (j=0; j<50; j++)
    {
      for (i=0; i<256; i++)
        {
          lval = 0;
          for (bit=0; bit<8; bit++)
            if (i >= level[bit])
              lval |= 1<<bit;
          led = lval;
        }
#define N 1
    for (i=0; i<8; i++)
      if (level[i] != 256)
        {
          if (level[i] > 0)
            level[i] -= N;
          else
            level[i] = 0;
        }
    }
}

void start( void )
{
    int op = 0;
    int bright = 255;
    cyg_uint32 f0;

#if 0
    while ( 1 ) {
#else
    int z;
    for ( z = 0; z < 100; z++ ) {
#endif
        f0 = 1;
        for (op=0; op<nseq; op++) {
            HAL_DIAG_WRITE_CHAR( 'A' + op );
            level[sequence[op]] = bright;
            set_leds();
            if ( op ) {
                int f1 = fact( op );
                f0 *= op;
                if ( f0 != f1 ) {
                    while ( 1 ) {
                        led = op;
                        CYG_TEST_FAIL_EXIT( "Factorial wrong" );
                    }
                }
            }
        }
        HAL_DIAG_WRITE_CHAR( '\n' );
        HAL_DIAG_WRITE_CHAR( '\r' );

        CYG_TEST_PASS( "Stack thrashed OK" );
    }
}

// -------------------------------------------------------------------------

externC void
#ifdef CYGPKG_KERNEL
cyg_user_start( void )
#else
cyg_start( void )
#endif
{
    CYG_TEST_INIT();
    CYG_TEST_INFO( "cyg_user_start()" );
    HAL_ENABLE_INTERRUPTS();
    start();
    CYG_TEST_EXIT( "LED bibbling and stack thrashing test" );
}

// -------------------------------------------------------------------------

/* EOF slebstak.c */
