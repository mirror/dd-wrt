//==========================================================================
//
//        mmap_test.c
//
//        Memory mapping test for ARM SA11x0 platforms
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
// Date:          2000-01-04
// Description:   Basic test of MMAP macros and functions
//                
//####DESCRIPTIONEND####

#include <pkgconf/system.h>

#include <cyg/infra/testcase.h>

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>

extern int diag_printf( char *, ... );

static int staticmem = 0;

static void somefunc( void )
{
    staticmem++;
}

static int vpcount = 0, uncachedcount = 0, loopcount = 0;

externC void
#ifdef CYGPKG_KERNEL
cyg_user_start( void )
#else
cyg_start( void )
#endif
{ 
    int stackmem = 127;

    unsigned int p1, p2, v1, v2;

    CYG_TEST_INIT();
    CYG_TEST_INFO( "Starting MMap test" );

    // First check the pagesize macro for various objects.
    p1 = 0;
    HAL_MM_PAGESIZE( &stackmem, p1 );
    CYG_TEST_CHECK( SZ_1M == p1, "Pagesize bad for stackmem" );

    p1 = 1;
    HAL_MM_PAGESIZE( &staticmem, p1 );
    CYG_TEST_CHECK( SZ_1M == p1, "Pagesize bad for staticmem" );

    p1 = 2;
    HAL_MM_PAGESIZE( &somefunc, p1 );
    CYG_TEST_CHECK( SZ_1M == p1, "Pagesize bad for somefunc" );

    CYG_TEST_PASS( "Pagesize macro OK" );

    // Test the macros with directly quoted "&thing" input args,
    // with things being static, on-stack, and code area:

    HAL_VIRT_TO_PHYS_ADDRESS( &stackmem, p2 );
    HAL_PHYS_TO_VIRT_ADDRESS( p2, v2 );
    CYG_TEST_CHECK( (unsigned int)(&stackmem) == v2,
                    "Stackmem translation failed" );

    HAL_VIRT_TO_PHYS_ADDRESS( &staticmem, p2 );
    HAL_PHYS_TO_VIRT_ADDRESS( p2, v2 );
    CYG_TEST_CHECK( (unsigned int)(&staticmem) == v2,
                    "Staticmem translation failed" );

    HAL_VIRT_TO_PHYS_ADDRESS( &somefunc, p2 );
    HAL_PHYS_TO_VIRT_ADDRESS( p2, v2 );
    CYG_TEST_CHECK( (unsigned int)(&somefunc) == v2,
                    "Somefunc translation failed" );


    // Test the macros with variable pointer input args:

    v1 = (unsigned int)&stackmem;
    HAL_VIRT_TO_PHYS_ADDRESS( v1, p2 );
    HAL_PHYS_TO_VIRT_ADDRESS( p2, v2 );
    CYG_TEST_CHECK( v1 == v2,
                    "Ptr-to-stackmem translation failed" );

    v1 = (unsigned int)&staticmem;
    HAL_VIRT_TO_PHYS_ADDRESS( v1, p2 );
    HAL_PHYS_TO_VIRT_ADDRESS( p2, v2 );
    CYG_TEST_CHECK( v1 == v2,
                    "Ptr-to-staticmem translation failed" );

    v1 = (unsigned int)&somefunc;
    HAL_VIRT_TO_PHYS_ADDRESS( v1, p2 );
    HAL_PHYS_TO_VIRT_ADDRESS( p2, v2 );
    CYG_TEST_CHECK( v1 == v2,
                    "Ptr-to-somefunc translation failed" );


    // Test the UNCACHED address macros similarly:

    v1 = (unsigned int)&stackmem;
    HAL_VIRT_TO_UNCACHED_ADDRESS( v1, p2 );
    CYG_TEST_CHECK( v1 != p2, "Stack mem already uncached!" );
    *(int *)v1 = 17;
    HAL_DCACHE_STORE( v1, 4 );
    CYG_TEST_CHECK( *(int *)p2 == 17, "Uncached stack data not 17" );
    *(int *)v1 = 23;
    HAL_DCACHE_STORE( v1, 4 );
    CYG_TEST_CHECK( *(int *)p2 == 23, "Uncached stack data not 23" );

    v1 = (unsigned int)&staticmem;
    HAL_VIRT_TO_UNCACHED_ADDRESS( v1, p2 );
    CYG_TEST_CHECK( v1 != p2, "Static mem already uncached!" );
    *(int *)v1 = 117;
    HAL_DCACHE_STORE( v1, 4 );
    CYG_TEST_CHECK( *(int *)p2 == 117, "Uncached static data not 117" );
    *(int *)v1 = 123;
    HAL_DCACHE_STORE( v1, 4 );
    CYG_TEST_CHECK( *(int *)p2 == 123, "Uncached static data not 123" );

#ifdef CYG_HAL_STARTUP_RAM // then somefunc is in RAM, and this is valid
    v1 = (unsigned int)&somefunc;
    HAL_VIRT_TO_UNCACHED_ADDRESS( v1, p2 );
    CYG_TEST_CHECK( v1 != p2, "Somefunc already uncached!" );
    CYG_TEST_CHECK( *(int *)p2 == *(int *)v1, "Uncached instruction not the same" );
#else
    CYG_TEST_INFO( "Skipping code cachability test, not RAM start" );
#endif

    // Now check via the routines that actually read the MMAP table:

    v1 = (unsigned int)&stackmem;
    p1 = hal_virt_to_phys_address( v1 );
    HAL_VIRT_TO_PHYS_ADDRESS( v1, p2 );
    CYG_TEST_CHECK( p1 == p2, "Physical address of stackmem mismatch" );
    v1 = hal_phys_to_virt_address( p2 );
    HAL_PHYS_TO_VIRT_ADDRESS( p2, v2 );
    CYG_TEST_CHECK( &stackmem == (int *)v1,
                    "Func : Virtual address of stackmem wrong" );
    CYG_TEST_CHECK( &stackmem == (int *)v2,
                    "Macro: Virtual address of stackmem wrong" );

    v1 = (unsigned int)&staticmem;
    p1 = hal_virt_to_phys_address( v1 );
    HAL_VIRT_TO_PHYS_ADDRESS( v1, p2 );
    CYG_TEST_CHECK( p1 == p2, "Physical address of staticmem mismatch" );
    v1 = hal_phys_to_virt_address( p2 );
    HAL_PHYS_TO_VIRT_ADDRESS( p2, v2 );
    CYG_TEST_CHECK( &staticmem == (int *)v1,
                    "Func : Virtual address of staticmem wrong" );
    CYG_TEST_CHECK( &staticmem == (int *)v2,
                    "Macro: Virtual address of staticmem wrong" );

    v1 = (unsigned int)&somefunc;
    p1 = hal_virt_to_phys_address( v1 );
    HAL_VIRT_TO_PHYS_ADDRESS( v1, p2 );
    CYG_TEST_CHECK( p1 == p2, "Physical address of somefunc mismatch" );
    v1 = hal_phys_to_virt_address( p2 );
    HAL_PHYS_TO_VIRT_ADDRESS( p2, v2 );
    CYG_TEST_CHECK( (unsigned int)&somefunc ==v1,
                    "Func : Virtual address of somefunc wrong" );
    CYG_TEST_CHECK( (unsigned int)&somefunc == v2,
                    "Macro: Virtual address of somefunc wrong" );


    // And check the uncached-address version of the routines that actually
    // read the MMAP table:

    v1 = (unsigned int)&stackmem;
    p1 = hal_virt_to_uncached_address( v1 );
    HAL_VIRT_TO_UNCACHED_ADDRESS( v1, p2 );
    CYG_TEST_CHECK( p1 == p2, "Uncached address of stackmem mismatch" );

    v1 = (unsigned int)&staticmem;
    p1 = hal_virt_to_uncached_address( v1 );
    HAL_VIRT_TO_UNCACHED_ADDRESS( v1, p2 );
    CYG_TEST_CHECK( p1 == p2, "Uncached address of staticmem mismatch" );

#ifdef CYG_HAL_STARTUP_RAM // then somefunc is in RAM, and this is valid
    v1 = (unsigned int)&somefunc;
    p1 = hal_virt_to_uncached_address( v1 );
    HAL_VIRT_TO_UNCACHED_ADDRESS( v1, p2 );
    CYG_TEST_CHECK( p1 == p2, "Uncached address of somefunc mismatch" );
#else
    CYG_TEST_INFO( "Skipping code cachability test 2, not RAM start" );
#endif

    CYG_TEST_PASS( "Initial explicit tests AOK" );

    // ---------------------------------------------------------------
    // 
    // We have now tested the macros and routines for some example objects
    // that we know are in RAM and which therefore should be dual-mapped.
    //
    // We can now whizz through all of the address space, checking as we go
    // that sensible things happen.  We must NOT use the addresses in
    // question, just pass them through the macros and routines.
    
    // Start from some random address,
    //    go up until we have covered all of memory
    //        increment by about 0.8 of a Mb,
    for ( v1 = 0x12345; v1 ; v1 += 0xffff7 ) {
        unsigned int v3;

        loopcount++;

        p1 = hal_virt_to_phys_address( v1 );
        HAL_VIRT_TO_PHYS_ADDRESS( v1, p2 );
        if ( p1 ) {
            vpcount++;
            // Then there is a physical address for this virtual address
            if (p1 != p2) diag_printf("v1: %p, p1: %p, p2: %p\n", v1, p1, p2);
            CYG_TEST_CHECK( p1 == p2,
                            "Scan: truly mapped physical address mismatch" );

            v3 = hal_phys_to_virt_address( p2 );
            HAL_PHYS_TO_VIRT_ADDRESS( p2, v2 );
            if (v2 != v3) diag_printf("v1: %p, p1: %p, v2: %p, v3: %p\n", v1, p1, v2, v3);
            CYG_TEST_CHECK( v3 == v2,
                            "Scan: backmapped virtual address mismatch" );
            // But the virtual address might be elsewhere, ie. a cached
            // nonphysical address.
            if ( v3 != v1 ) {
                // Then it is [also] elsewhere, apparently.  Check that its
                // otherness maps right back to this physical address.
                p1 = hal_virt_to_phys_address( v2 );
                if (p1 != p2) diag_printf("v1: %p, p1: %p, p2: %p\n", v1, p1, p2);
                CYG_TEST_CHECK( p1 == p2,
                                "Scan: phys(virt(phys(x))) mismatch" );
            }
        }

        p1 = hal_virt_to_uncached_address( v1 );
        HAL_VIRT_TO_UNCACHED_ADDRESS( v1, p2 );
        if ( p1 ) {
            uncachedcount++;
            // Then there is an uncached address for this virtual address
            if (p1 != p2) diag_printf("v1: %p, p1: %p, p2: %p\n", v1, p1, p2);
            CYG_TEST_CHECK( p1 == p2, "Uncached address of stackmem mismatch" );
        }

        if ( v1 > 0xfff00000u )
            break;
    }

    diag_printf( "INFO:<%d addresses tested>\n", loopcount );
    diag_printf( "INFO:<%d virt-phys mappings checked>\n", vpcount );
    diag_printf( "INFO:<%d uncachable addresses checked>\n", uncachedcount );

    CYG_TEST_PASS( "MMap memory scan test OK" );

    // ---------------------------------------------------------------
    CYG_TEST_EXIT( "Done" );
}

// EOF mmap_test.c
