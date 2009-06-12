/*=================================================================
//
//        slebintr.c
//
//        SPARClite HAL interrupt manipulation test
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

#define DELAY(x) \
    CYG_MACRO_START int i; for (i = 0; i < x; i++); CYG_MACRO_END

#define DLA 100

static int l, u, m, j;

int levels[ 16 ];
int    ups[ 16 ];
int  masks[ 16 ];
int   reqs[ 16 ];

#define XDIGIT( q ) (q + ((q < 10) ? '0' : ('A'-10) ))

#define SETSTR( x, y, str ) CYG_MACRO_START \
        str[0] = XDIGIT( x ); \
        str[1] = XDIGIT( y ); \
CYG_MACRO_END

static char lstr[] = "xy Bad level";
static char mstr[] = "xy Bad  mask";
static char ustr[] = "xy Bad    up";


static void checkallbut( int z )
{
    int i;
    for ( i = 1; i < 16; i++ ) {
        int level, up, hipri, mask, req;
        if ( z == i )
            continue;

        SETSTR( i, z, lstr );
        SETSTR( i, z, mstr );
        SETSTR( i, z, ustr );

        HAL_INTERRUPT_QUERY_INFO( i, level, up, hipri, mask, req);
        l = level;
        u = up;
        m = mask;
        j = i;

#if 0 // for manual testing really...
        if ( level != levels[i] )
            CYG_TEST_INFO( lstr );
        if (    up !=    ups[i] )
            CYG_TEST_INFO( ustr );
        if (  mask !=  masks[i] )
            CYG_TEST_INFO( mstr );
        if ( (level != levels[i] ) 
        |    (   up !=    ups[i] ) 
        |    ( mask !=  masks[i] ) ) {
            CYG_TEST_INFO( "Re-reading" );
            HAL_INTERRUPT_QUERY_INFO( i, level, up, hipri, mask, req);
        }
#endif
        CYG_TEST_CHECK( level == levels[i], lstr );
        CYG_TEST_CHECK(    up ==    ups[i], ustr );
        CYG_TEST_CHECK(  mask ==  masks[i], mstr );
    }
}

// input is the active phase of the chosen interrupt.  It is assumed that
// the source is normally level-sensititve rather than edge-sensitive.
static void interferewith( int which, int input )
{
    int level, up, hipri, mask, req;

    // Interfere with interrupt 'which'
    HAL_INTERRUPT_CONFIGURE( which, 1, input ); // should be no change

    checkallbut( 0 ); // so don't exclude any of them

    HAL_INTERRUPT_CONFIGURE( which, 1, !input ); // make it other-sensitive
    DELAY( DLA );
    HAL_INTERRUPT_ACKNOWLEDGE( which );
    DELAY( DLA );
    HAL_INTERRUPT_QUERY_INFO( which, level, up, hipri, mask, req);
    CYG_TEST_CHECK( 0 != level , "Int not level-sensitive (-ve level)" );
    if ( input )
        CYG_TEST_CHECK( 0 == up, "Int high level (-ve level)" );
    else
        CYG_TEST_CHECK( 0 != up, "Int low level (-ve level)" );
    CYG_TEST_CHECK( 0 !=  mask , "Int unmasked (-ve level)" );
    CYG_TEST_CHECK( 0 !=   req , "Int not requesting (-ve level)" );
    checkallbut( which ); // don't check #which, we're messing with it
    
    HAL_INTERRUPT_CONFIGURE( which, 0, input ); // edge, default sense
    DELAY( DLA );
    HAL_INTERRUPT_ACKNOWLEDGE( which );
    DELAY( DLA );
    HAL_INTERRUPT_QUERY_INFO( which, level, up, hipri, mask, req);
    CYG_TEST_CHECK( 0 == level , "Int not edge-sensitive (+ve edge)" );
    if ( input )
        CYG_TEST_CHECK( 0 != up, "Int low edge (+ve edge)" );
    else
        CYG_TEST_CHECK( 0 == up, "Int high edge (+ve edge)" );
    CYG_TEST_CHECK( 0 !=  mask , "Int unmasked (+ve edge)" );
    CYG_TEST_CHECK( 0 ==   req , "Int requesting (+ve edge)" );
    checkallbut( which ); // don't check #which, we're messing with it
    
    HAL_INTERRUPT_CONFIGURE( which, 0, !input ); // edge, opposite sense
    DELAY( DLA );
    HAL_INTERRUPT_ACKNOWLEDGE( which );
    DELAY( DLA );
    HAL_INTERRUPT_QUERY_INFO( which, level, up, hipri, mask, req);
    CYG_TEST_CHECK( 0 == level , "Int not edge-sensitive (-ve edge)" );
    if ( input )
        CYG_TEST_CHECK( 0 == up, "Int high edge (-ve edge)" );
    else
        CYG_TEST_CHECK( 0 != up, "Int low edge (-ve edge)" );
    CYG_TEST_CHECK( 0 !=  mask , "Int unmasked (-ve edge)" );
    CYG_TEST_CHECK( 0 ==   req , "Int requesting (-ve edge)" );
    checkallbut( which ); // don't check #which, we're messing with it
    
    HAL_INTERRUPT_CONFIGURE( which, 1, input ); // back to original value
    DELAY( DLA );
    HAL_INTERRUPT_ACKNOWLEDGE( which );
    DELAY( DLA );
    checkallbut( 0 ); // so don't exclude any of them
}

// ------------------------------------------------------------------------

#ifndef CYGPKG_KERNEL
// then the clock is not initialized!
#undef HAL_CLOCK_READ
// so provide a dumb counter, so we do the test a number of times anyway.
static int pseudotime = 0;
#define HAL_CLOCK_READ( _pval_ ) *(_pval_) = \
   ((pseudotime > 10) ? (pseudotime = 0) : ++pseudotime)
#endif

static int start( void )
{
    // We'll mess about with these interrupt sources:
    // 13   : EX_IRQ13 from expansion board, active HIGH
    // 12   : EX_IRQ12 from expansion board, active LOW
    //  4   : EX_IRQ4 from expansion board, active LOW
    //  3   : EX_IRQ3 from expansion board, active HIGH

    int i, hipri;
    for ( i = 1; i < 16; i++ ) {
        HAL_INTERRUPT_QUERY_INFO( i, levels[i], ups[i],
                                  hipri, masks[i], reqs[i]);
    }
    CYG_TEST_CHECK( 0 !=  masks[13], "Int 13 unmasked initially" );
    CYG_TEST_CHECK( 0 !=  masks[12], "Int 12 unmasked initially" );
    CYG_TEST_CHECK( 0 !=  masks[ 4], "Int  4 unmasked initially" );
    CYG_TEST_CHECK( 0 !=  masks[ 3], "Int  3 unmasked initially" );
    CYG_TEST_CHECK( 0 ==   reqs[13], "Int 13 requests initially" );
    CYG_TEST_CHECK( 0 ==   reqs[12], "Int 12 requests initially" );
    CYG_TEST_CHECK( 0 ==   reqs[ 4], "Int  4 requests initially" );
    CYG_TEST_CHECK( 0 ==   reqs[ 3], "Int  3 requests initially" );
    CYG_TEST_CHECK( 0 != levels[13], "Int 13 edgetrig initially" );
    CYG_TEST_CHECK( 0 != levels[12], "Int 12 edgetrig initially" );
    CYG_TEST_CHECK( 0 != levels[ 4], "Int  4 edgetrig initially" );
    CYG_TEST_CHECK( 0 != levels[ 3], "Int  3 edgetrig initially" );
    CYG_TEST_CHECK( 0 !=    ups[13], "Int 13 not up initially" );
    CYG_TEST_CHECK( 0 ==    ups[12], "Int 12 is up initially" );
    CYG_TEST_CHECK( 0 ==    ups[ 4], "Int  4 is up initially" );
    CYG_TEST_CHECK( 0 !=    ups[ 3], "Int  3 not up initially" );

    checkallbut( 0 ); // don't exclude any of them

    // I want to run this loop for 100 centiSeconds, so that 100 clock
    // interrupts have occurred whilst interfering with the other interrupt
    // state, to provoke possible problems with interactions there.  On a
    // 100MHz 86832 with default configuration, this usually gives 1200
    // loops in total, 12 per centiSecond.  But with config variance and
    // caching behaviour, it's quite possible for this loop to take much
    // longer, if it takes over 1/2 a centiSecond, aliasing effects could
    // cause the timing to fail completely, causing test timeouts.  Hence
    // the additional loop limit of 20 times round the inner loop, aiming
    // for a maximum elapsed time of 20 S maximum, plus extra chances to
    // break out part way through the loop if a tick has passed.

    hipri = 0;
    CYG_TEST_INFO( "About to configure interrupts" );
    for ( i = 0; i < 100; i++ ) {
        int t1, t2, j;
        HAL_CLOCK_READ( &t1 );
        // Do this while/until there is a clock tick
        // ie. some interrupt activity:
        for ( j = 0; j < 20; j++ ) {
            t2 = t1;
            hipri++;
            interferewith( 13, 1 );
            interferewith( 3, 1 );
            interferewith( 4, 0 );
            HAL_CLOCK_READ( &t1 );
            if ( t1 < t2 )
                break;                  // clock has wrapped
            t2 = t1;
            interferewith( 12, 0 );
            interferewith( 3, 1 );
            interferewith( 3, 1 );
            HAL_CLOCK_READ( &t1 );
            if ( t1 < t2 )
                break;                  // clock has wrapped
            t2 = t1;
            interferewith( 4, 0 );
            interferewith( 13, 1 );
            interferewith( 12, 0 );
            interferewith( 12, 0 );
            HAL_CLOCK_READ( &t1 );
            if ( t1 < t2 )
                break;                  // clock has wrapped
        }
    }
    CYG_TEST_PASS( "Configured interrupts 3,4,12 & 13 a great deal" );
    return hipri;
}

// -------------------------------------------------------------------------

externC void
#ifdef CYGPKG_KERNEL
cyg_user_start( void )
#else
cyg_start( void )
#endif
{
    int loops;
    CYG_TEST_INIT();
    CYG_TEST_INFO( "cyg_user_start()" );
    HAL_ENABLE_INTERRUPTS();
    loops = start();
    // Typically about 1200 loops execute on the 33MHz 86832;
    // I hoped to put in a check that we hadn't wasted time in
    // spurious interrupts, but kernel instrumentation, for example,
    // is more than enough to slow the world down... so keep this
    // very weak test in, as a placeholder.
    CYG_TEST_CHECK( 100 <= loops, "Not enough tests executed" );
    CYG_TEST_EXIT( "All done" );
}

// -------------------------------------------------------------------------

/* EOF slebintr.c */
