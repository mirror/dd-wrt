/*=================================================================
//
//        context.c
//
//        HAL Thread context handling test
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
// Date:          1998-10-07
//####DESCRIPTIONEND####
*/

#include <pkgconf/hal.h>

#include <pkgconf/infra.h>

#include <cyg/infra/testcase.h>

#include <cyg/infra/cyg_trac.h>
#include <cyg/hal/hal_arch.h>

#define CYG_TRACE_USER_BOOL 1

// -------------------------------------------------------------------------

#define THREADS         4
#define STACKSIZE       (2*1024)

char stack[THREADS][STACKSIZE];

CYG_ADDRWORD sp[THREADS];

cyg_count32 switches = 0;

// -------------------------------------------------------------------------

void entry0( CYG_ADDRWORD arg )
{
    CYG_TRACE1B("Thread %d started\n", arg );

    while( switches < 1000 )
    {
        HAL_THREAD_SWITCH_CONTEXT( &sp[arg], &sp[(arg+1) % THREADS] );

        CYG_TRACE1B("Thread %d resumed\n", arg );

        switches++;
    }

    CYG_TEST_PASS_FINISH("HAL Context test");
    
}

// -------------------------------------------------------------------------

void context_main(void)
{
    int i;
    
    CYG_TEST_INIT();

    // Init all thread contexts:
    
    for( i = 0 ; i < THREADS; i++ )
    {
        sp[i] = (CYG_ADDRWORD)stack[i]+STACKSIZE;
        
        HAL_THREAD_INIT_CONTEXT( sp[i], i, entry0, i*0x01010000 );
    }

    // Load the first thread.
    
    HAL_THREAD_LOAD_CONTEXT( &sp[0] );
}

// -------------------------------------------------------------------------

externC void
cyg_start( void )
{
    context_main();
}

// -------------------------------------------------------------------------
/* EOF context.c */
