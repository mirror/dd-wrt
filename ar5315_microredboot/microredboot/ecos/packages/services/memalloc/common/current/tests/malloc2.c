//=================================================================
//
//        malloc2.c
//
//        Stress testcase for C library malloc(), calloc() and free()
//
//=================================================================
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-30
// Description:   Contains testcode to stress-test C library malloc(),
//                calloc() and free() functions
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <pkgconf/system.h> // Overall system configuration
#include <pkgconf/memalloc.h>  // config header so we can know size of malloc pool
#ifdef CYGPKG_ISOINFRA
# include <pkgconf/isoinfra.h>
# include <stdlib.h>
#endif

#include <cyg/infra/testcase.h>

#if !defined(CYGPKG_ISOINFRA)
# define NA_MSG "Requires isoinfra package"
#elif !CYGINT_ISO_MAIN_STARTUP
# define NA_MSG "Requires main() to be called"
#elif !CYGINT_ISO_MALLOC
# define NA_MSG "Requires malloc"
#elif !CYGINT_ISO_MALLINFO
# define NA_MSG "Requires mallinfo"
#endif

#ifdef NA_MSG
void
cyg_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA( NA_MSG );
    CYG_TEST_FINISH("Done");
}
#else

// CONSTANTS

#define NUM_ITERATIONS 1000

// GLOBALS

static int problem=0;

// FUNCTIONS

extern int
cyg_memalloc_maxalloc( void );

static void *
safe_malloc( size_t size )
{
    void *ptr;

    ptr=malloc(size);

    if (ptr==NULL)
    {
        CYG_TEST_FAIL( "malloc returned NULL! "
                       "Perhaps the allocator doesn't coalesce?" );
        problem++;
    } // if

    return ptr;
} // safe_malloc()


static void *
safe_calloc( size_t size )
{
    void *ptr;
    int i;

    ptr=calloc(size,1);

    if (ptr==NULL)
    {
        CYG_TEST_FAIL( "calloc returned NULL! "
                       "Perhaps the allocator doesn't coalesce" );
        problem++;
    } // if
    else
    {
        for (i=0; i < size; i++)
        {
            if (((char *)ptr)[i] != 0)
            {
                CYG_TEST_FAIL("calloc didn't clear data completely");
                problem++;
                return ptr;
            } // if
        } // for
    } // else

    return ptr;
} // safe_calloc()


static void
fill_with_data( char *buf, int size )
{
    int i;

    for (i=0; i < size; i++)
        buf[i] = 'f';

    for (i=0; i < size; i++)
        if (buf[i] != 'f')
        {
            CYG_TEST_FAIL( "data written to buffer does not compare "
                           "correctly! #1" );
            problem++;
            return;
        } // if


    for (i=0; i < size; i++)
        buf[i] = 'z';

    for (i=0; i < size; i++)
        if (buf[i] != 'z')
        {
            CYG_TEST_FAIL( "data written to buffer does not compare "
                           "correctly! #2" );
            problem++;
            return;
        } // if

} // fill_with_data()


int
main( int argc, char *argv[] )
{
    char *str1, *str2, *str3;
    int j;
    int poolmax;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting stress tests from testcase " __FILE__ " for C "
                  "library malloc(), calloc() and free() functions");

    poolmax = mallinfo().maxfree;
    
    if ( poolmax <= 0 ) {
        CYG_TEST_FAIL_FINISH( "Can't determine allocation size to use" );
    }

    if ( poolmax < 300 )
    {
        CYG_TEST_FAIL_FINISH("This testcase cannot safely be used with a "
                             "memory pool for malloc less than 300 bytes");
    } // if


    for ( j=1; j < NUM_ITERATIONS; j++)
    {
//        if ((j % 100) == 0)
//            CYG_TEST_STILL_ALIVE( j, "Multiple mallocs and frees continuing" );

        
        str1 = (char *)safe_malloc(50);
        fill_with_data( str1, 50 );
        str2 = (char *)safe_calloc(11);
        fill_with_data( str2, 11 );
        str3 = (char *)safe_malloc(32);
        fill_with_data( str3, 32 );

        free(str2);
        free(str1);

        str2 = (char *)safe_calloc(11);
        fill_with_data( str2, 11 );
        free(str2);

        str1 = (char *)safe_calloc(50);
        fill_with_data( str1, 50 );
        free(str3);

        str3 = (char *)safe_malloc(32);
        fill_with_data( str3, 32 );
        free(str1);

        str2 = (char *)safe_calloc(11);
        fill_with_data( str2, 11 );
        str1 = (char *)safe_malloc(50);
        fill_with_data( str1, 50 );

        free(str3);
        free(str1);
        free(str2);

        if (problem != 0)
            break;
    } // for

    // Did it completely successfully?
    if (j==NUM_ITERATIONS)
        CYG_TEST_PASS("Stress test completed successfully");

    CYG_TEST_FINISH("Finished stress tests from testcase " __FILE__ " for C "
                    "library malloc(), calloc() and free() functions");

    return 0;
} // main()

#endif // ifndef NA_MSG

// EOF malloc2.c
