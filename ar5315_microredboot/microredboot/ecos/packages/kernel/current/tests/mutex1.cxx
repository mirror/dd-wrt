//==========================================================================
//
//        mutex1.cxx
//
//        Mutex test 1
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
// Date:          1998-02-24
// Description:   Tests basic mutex functionality.
// Omissions:     Timed wait.
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/sched.hxx>        // Cyg_Scheduler::start()
#include <cyg/kernel/thread.hxx>       // Cyg_Thread

#include <cyg/kernel/mutex.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/sched.inl>
#include <cyg/kernel/thread.inl>


#define NTHREADS 3
#include "testaux.hxx"

static Cyg_Mutex m0, m1;
static Cyg_Condition_Variable cvar0( m0 ), cvar1( m0 ), cvar2( m1 );

static cyg_ucount8 m0d=0, m1d=0;

static void finish( cyg_ucount8 t )
{
    m1.lock(); {
        m1d |= 1<<t;
        if( 0x7 == m1d )
            CYG_TEST_PASS_FINISH("Mutex 1 OK");
        cvar2.wait();
    }
    CYG_TEST_FAIL_FINISH("Not reached");    
}

static void entry0( CYG_ADDRWORD data )
{
    m0.lock(); {
        CHECK( ! m0.trylock() );
        m1.lock(); {
            CHECK( ! m0.trylock() );            
        } m1.unlock();
    } m0.unlock();

    m0.lock(); {
        while ( 0 == m0d )
            cvar0.wait();
        CHECK( 1 == m0d++ );
        cvar0.signal();
        while ( 4 != m0d )
            cvar1.wait();
        CHECK( 4 == m0d );
    } m0.unlock();

    finish( data );
}

static void entry1( CYG_ADDRWORD data )
{
    m0.lock(); {
        CHECK( m1.trylock() ); {
        } m1.unlock();
    } m0.unlock();

    m0.lock(); {
        CHECK( 0 == m0d++ );
        cvar0.broadcast();
    } m0.unlock();
    
    m0.lock(); {
        while( 1 == m0d )
            cvar0.wait();
        CHECK( 2 == m0d++ );
        cvar0.signal();
        while (3 == m0d )
            cvar1.wait();
    } m0.unlock();

    finish( data );                 // At most 1 finish inside m0 lock
}

static void entry2( CYG_ADDRWORD data )
{
    m0.lock(); {
        while( 3 != m0d ) {
            cvar0.wait();
        }
        CHECK( 3 == m0d++ );
        cvar1.broadcast();
    } m0.unlock();

    finish( data );    
}

void mutex1_main( void )
{
    CYG_TEST_INIT();

    new_thread(entry0, 0);
    new_thread(entry1, 1);
    new_thread(entry2, 2);

    Cyg_Scheduler::start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{ 
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    mutex1_main();
}
// EOF mutex1.cxx
