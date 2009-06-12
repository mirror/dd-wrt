//==========================================================================
//
//        pthread1.cxx
//
//        POSIX pthread test 1
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
// Author(s):     nickg
// Contributors:  nickg
// Date:          2000-04-10
// Description:   Tests POSIX join functionality.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/infra/testcase.h>
#include <pkgconf/posix.h>

#ifndef CYGPKG_POSIX_PTHREAD
#define NA_MSG "POSIX threads not enabled"
#endif

#ifdef NA_MSG
void
cyg_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA(NA_MSG);
}
#else

#include <sys/types.h>
#include <pthread.h>

//--------------------------------------------------------------------------
// Thread stack.

char thread_stack[PTHREAD_STACK_MIN*2];

//--------------------------------------------------------------------------

void *pthread_entry1( void *arg)
{
    CYG_TEST_INFO( "Thread 1 running" );
    
    pthread_exit( (void *)((int)arg+1) );
}

//--------------------------------------------------------------------------

int main(int argc, char **argv)
{
    pthread_t thread;
    pthread_attr_t attr;
    void *retval;

    CYG_TEST_INIT();

    // Create test thread
    pthread_attr_init( &attr );

    pthread_attr_setstackaddr( &attr, (void *)&thread_stack[sizeof(thread_stack)] );
    pthread_attr_setstacksize( &attr, sizeof(thread_stack) );

    pthread_create( &thread,
                    &attr,
                    pthread_entry1,
                    (void *)0x12345678);

    // Now join with it
    pthread_join( thread, &retval );

    // check retval
    
    if( (long)retval == 0x12345679 )
        CYG_TEST_PASS_FINISH( "pthread1" );
    else
        CYG_TEST_FAIL_FINISH( "pthread1" );
}

#endif

//--------------------------------------------------------------------------
// end of pthread1.c
