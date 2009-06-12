/*=================================================================
//
//        mbxtime.cxx
//
//        PowerPC MBX HAL timing tests
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
// Date:          1999-06-01
//####DESCRIPTIONEND####
*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/testcase.h>

#ifdef CYGPKG_KERNEL

#include <pkgconf/kernel.h>

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_diag.h>

#include <pkgconf/infra.h>

#include <cyg/infra/diag.h>

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/sched.inl>

// ------------------------------------------------------------------------

externC void hal_mbx_flash_led( int count );
externC void hal_mbx_set_led( int val );

// -------------------------------------------------------------------------

static void entry1( CYG_ADDRWORD arg )
{
    int i;
    char *s = "tick 00";
    extern Cyg_Thread thread2;
    CYG_TEST_INFO( "Starting measured seconds..." );
    for ( i = 0; i < 20 ; i++ ) {
        Cyg_Thread::self()->delay( 100 ); // units should be centiSeconds

        hal_mbx_set_led( ((~i) & 7)<<1 );

        if ( '9' == s[ 6 ]++ ) {
            s[ 6 ] = '0';
            s[ 5 ] ++;
        }
        CYG_TEST_INFO( s );
    }

    CYG_TEST_PASS( "Done measured seconds..." );

    hal_mbx_flash_led( 5 );

    thread2.resume();
}

//---------------------------------------------------------------

#define LOOPS 30

int loops[ LOOPS ] = { 0,0,0,0,0 , 0,0,0,0,0 ,
                       0,0,0,0,0 , 0,0,0,0,0 ,
                       0,0,0,0,0 , 0,0,0,0,0 };

char s[] = "0 count 00 loops 00000";

static void entry2( CYG_ADDRWORD arg )
{
#if 0 // Do not bother with this on PPC
    int i, t1, t2, a;
    for ( a = 0; a < 10; a++, (*s)++ ) {
        CYG_TEST_INFO( "------------------------------------------" );
        HAL_CLOCK_READ( &t1 );
        do {
            t2 = t1;
            HAL_CLOCK_READ( &t1 );
        } while ( t1 >= t2 );

        for ( i = 0; i < LOOPS; i++ ) {
            register int z = 0;
            while ( t1 <= i ) {
                z++;
                HAL_CLOCK_READ( &t1 );
            }
            loops[ i ] = z;
        }

        s[9] = '0';
        s[8] = '0';
        for ( i = 0; i < LOOPS; i++ ) {
            s[ sizeof( s ) - 2 ] = '0' + loops[ i ] / 1       % 10;
            s[ sizeof( s ) - 3 ] = '0' + loops[ i ] / 10      % 10;
            s[ sizeof( s ) - 4 ] = '0' + loops[ i ] / 100     % 10;
            s[ sizeof( s ) - 5 ] = '0' + loops[ i ] / 1000    % 10;
            s[ sizeof( s ) - 6 ] = '0' + loops[ i ] / 10000   % 10;
            CYG_TEST_INFO( s );
            if ( '9' == s[9]++ ) {
                s[9] = '0';
                s[8]++;
            }        
        }
    }
#endif
    CYG_TEST_PASS( "Counted loops per timer tick" );
    CYG_TEST_EXIT( "All done" );
}

// -------------------------------------------------------------------------

static char stack1[ CYGNUM_HAL_STACK_SIZE_TYPICAL ];
static char stack2[ CYGNUM_HAL_STACK_SIZE_TYPICAL ];

static Cyg_Thread thread1     CYG_INIT_PRIORITY( APPLICATION )
 = Cyg_Thread( 2u, entry1, 0u, "timed minute thread",
               (CYG_ADDRWORD)stack1,
               (CYG_ADDRWORD)CYGNUM_HAL_STACK_SIZE_TYPICAL );

static Cyg_Thread thread2     CYG_INIT_PRIORITY( APPLICATION )
 = Cyg_Thread( 4u, entry2, 0u, "uS clock loops timing thread",
               (CYG_ADDRWORD)stack2,
               (CYG_ADDRWORD)CYGNUM_HAL_STACK_SIZE_TYPICAL );

// -------------------------------------------------------------------------

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
externC void
cyg_hal_invoke_constructors();
#endif

externC void
cyg_user_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    CYG_TEST_INIT();
    CYG_TEST_INFO( "cyg_user_start()" );
    thread1.resume();
}

// -------------------------------------------------------------------------

#else  // ! CYGVAR_KERNEL_COUNTERS_CLOCK
#define N_A_MSG "no kernel clock"
#endif // CYGVAR_KERNEL_COUNTERS_CLOCK
#else  // ! CYGPKG_KERNEL
#define N_A_MSG "no kernel"
#endif // CYGPKG_KERNEL

#ifdef N_A_MSG
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG defined ie. we are N/A.

/* EOF mbxtime.cxx */
