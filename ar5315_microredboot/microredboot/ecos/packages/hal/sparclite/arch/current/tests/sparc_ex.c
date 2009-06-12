/*=================================================================
//
//        sparc_ex.c
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

#include <cyg/infra/testcase.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

#include <pkgconf/infra.h>

#ifdef CYGDBG_USE_TRACING
#define OUTER_REPEATS 1
#define SIM_REPEATS  10
#define HW_REPEATS 1000 
#else
#define OUTER_REPEATS 10
#define SIM_REPEATS  100
#define HW_REPEATS 10000 
#endif // using tracing to slow everything down

// -------------------------------------------------------------------------
// These routines are used to cause an alignment trap; the compiler is too
// darned clever by half, if you try to inline this stuff as macros it uses
// different instruction sequences and register pairs.  This makes for a
// less thorough test, but there's no option other than writing a LOT of
// assembler code.

// Further, with -O3, the compiler inlines these anyway and so makes
// non-trapping code.  So they are now at the end, to prevent this.

extern cyg_uint64 get_ll( cyg_uint64 *p );

extern cyg_uint64 get_llplus( cyg_uint64 *p );

extern cyg_uint32 get_i( cyg_uint32 *p );

extern cyg_uint32 get_iplus( cyg_uint32 *p );

extern cyg_uint16 get_s( cyg_uint16 *p );

extern cyg_uint16 get_splus( cyg_uint16 *p );

// -------------------------------------------------------------------------
// Some memory to read in more-or-less aligned manners.

#define L1 (0x123456789abcdef0l)
#define L2 (0xfedcba9876543210l)
static cyg_uint64 a[ 2 ] = {
    L1,
    L2
};

#define M32 (0x00000000ffffffffl)
#define M16 (0x000000000000ffffl)
#define M8  (0x00000000000000ffl)

volatile cyg_uint32 trap = 0;
volatile cyg_uint32 same = 0;
volatile cyg_uint32 tcount = 0;

// -------------------------------------------------------------------------
// This macro invokes a routine then checks that a suitable trap occurred.
// It expects the instruction simply to be skipped, rather than the trap
// _handled_ or the unaligned access to be _emulated_ in any way.  This
// test is just a proof that we could write such a handler.

#define TRAPPROC( _var_, _type_, _align_, _proc_ )                         \
CYG_MACRO_START                                                            \
    otrap = trap;                                                          \
    otcount = tcount;                                                      \
    _var_ = _proc_( (_type_ *)(cp + (_align_)) );                          \
    CYG_TEST_CHECK( trap != otrap || same > 0,                             \
                    "No trap [" #_type_ ":" #_align_ "," #_proc_ "]" );    \
    CYG_TEST_CHECK( same < 20,                                             \
        "Undetected trap loop[" #_type_ ":" #_align_ "," #_proc_ "]" );    \
    CYG_TEST_CHECK( tcount > otcount,                                      \
        "No trap counted [" #_type_ ":" #_align_ "," #_proc_ "]" );        \
    CYG_TEST_CHECK( (tcount - 1) <= otcount,                               \
        "Tcount overinc [" #_type_ ":" #_align_ "," #_proc_ "]" );         \
CYG_MACRO_END

// and this one expects no trap to occur:
#define SAFEPROC( _var_, _type_, _align_, _proc_ )                         \
CYG_MACRO_START                                                            \
    trap = 0;                                                              \
    otcount = tcount;                                                      \
    _var_ = _proc_( (_type_ *)(cp + (_align_)) );                          \
    CYG_TEST_CHECK( 0 == trap,                                             \
                        "Trap [" #_type_ ":" #_align_ "," #_proc_ "]" );   \
    CYG_TEST_CHECK( tcount == otcount,                                     \
            "Trap counted [" #_type_ ":" #_align_ "," #_proc_ "]" );       \
CYG_MACRO_END

static void do_test( void )
{
    cyg_uint32 *ip = (cyg_uint32 *)a;
    cyg_uint16 *sp = (cyg_uint16 *)a;
    cyg_uint8  *cp = (cyg_uint8  *)a;
    
    cyg_uint64 l;
    cyg_uint32 i;
    cyg_uint16 s;
    cyg_uint8  c;

    cyg_int32 z, repeats;

    cyg_uint32 otrap;
    cyg_uint32 otcount;

    otrap = trap = 0;
    otcount = tcount;

    // First test interestingly aligned accesses that are legal.

    l = a[0];  CYG_TEST_CHECK( L1 == l, "a[0] read bad" );
    l = a[1];  CYG_TEST_CHECK( L2 == l, "a[1] read bad" );

    i = ip[0];  CYG_TEST_CHECK( ((L1 >> 32) & M32) == i, "ip[0]" );
    i = ip[1];  CYG_TEST_CHECK( ((L1      ) & M32) == i, "ip[1]" );
    i = ip[2];  CYG_TEST_CHECK( ((L2 >> 32) & M32) == i, "ip[2]" );
    i = ip[3];  CYG_TEST_CHECK( ((L2      ) & M32) == i, "ip[3]" );
                                   
    s = sp[0];  CYG_TEST_CHECK( ((L1 >> 48) & M16) == s, "sp[0]" );
    s = sp[1];  CYG_TEST_CHECK( ((L1 >> 32) & M16) == s, "sp[1]" );
    s = sp[2];  CYG_TEST_CHECK( ((L1 >> 16) & M16) == s, "sp[2]" );
    s = sp[3];  CYG_TEST_CHECK( ((L1      ) & M16) == s, "sp[3]" );
    s = sp[4];  CYG_TEST_CHECK( ((L2 >> 48) & M16) == s, "sp[4]" );
    s = sp[5];  CYG_TEST_CHECK( ((L2 >> 32) & M16) == s, "sp[5]" );
    s = sp[6];  CYG_TEST_CHECK( ((L2 >> 16) & M16) == s, "sp[6]" );
    s = sp[7];  CYG_TEST_CHECK( ((L2      ) & M16) == s, "sp[7]" );
                                   
    c = cp[0];  CYG_TEST_CHECK( ((L1 >> 56) & M8) == c, "cp[0]" );
    c = cp[1];  CYG_TEST_CHECK( ((L1 >> 48) & M8) == c, "cp[1]" );
    c = cp[2];  CYG_TEST_CHECK( ((L1 >> 40) & M8) == c, "cp[2]" );
    c = cp[3];  CYG_TEST_CHECK( ((L1 >> 32) & M8) == c, "cp[3]" );
    c = cp[4];  CYG_TEST_CHECK( ((L1 >> 24) & M8) == c, "cp[4]" );
    c = cp[5];  CYG_TEST_CHECK( ((L1 >> 16) & M8) == c, "cp[5]" );
    c = cp[6];  CYG_TEST_CHECK( ((L1 >>  8) & M8) == c, "cp[6]" );
    c = cp[7];  CYG_TEST_CHECK( ((L1      ) & M8) == c, "cp[7]" );
    c = cp[8];  CYG_TEST_CHECK( ((L2 >> 56) & M8) == c, "cp[8]" );
    c = cp[9];  CYG_TEST_CHECK( ((L2 >> 48) & M8) == c, "cp[9]" );
    c = cp[10]; CYG_TEST_CHECK( ((L2 >> 40) & M8) == c, "cp[10]" );
    c = cp[11]; CYG_TEST_CHECK( ((L2 >> 32) & M8) == c, "cp[11]" );
    c = cp[12]; CYG_TEST_CHECK( ((L2 >> 24) & M8) == c, "cp[12]" );
    c = cp[13]; CYG_TEST_CHECK( ((L2 >> 16) & M8) == c, "cp[13]" );
    c = cp[14]; CYG_TEST_CHECK( ((L2 >>  8) & M8) == c, "cp[14]" );
    c = cp[15]; CYG_TEST_CHECK( ((L2      ) & M8) == c, "cp[15]" );

    CYG_TEST_CHECK( 0 == trap, "Traps occurred (legal accesses)" );
    CYG_TEST_CHECK( tcount == otcount, "Traps counted (legal accesses)" );

    CYG_TEST_PASS( "Aligned accesses OK" );

    for ( z = OUTER_REPEATS; z > 0; z-- ) {

        for ( repeats = (cyg_test_is_simulator ? SIM_REPEATS : HW_REPEATS) ;
              repeats > 0 ; repeats-- ) {

            TRAPPROC( l, cyg_uint64, 4, get_llplus );
            TRAPPROC( l, cyg_uint64, 5, get_llplus );
            TRAPPROC( l, cyg_uint64, 6, get_llplus );
            TRAPPROC( l, cyg_uint64, 7, get_llplus );

            TRAPPROC( l, cyg_uint64, 4, get_ll );
            TRAPPROC( l, cyg_uint64, 1, get_ll );
            TRAPPROC( l, cyg_uint64, 2, get_ll );
            TRAPPROC( l, cyg_uint64, 3, get_ll );

            TRAPPROC( i, cyg_uint32, 1, get_iplus );
            TRAPPROC( i, cyg_uint32, 2, get_iplus );
            TRAPPROC( i, cyg_uint32, 3, get_iplus );

            TRAPPROC( i, cyg_uint32, 5, get_i );
            TRAPPROC( i, cyg_uint32, 6, get_i );
            TRAPPROC( i, cyg_uint32, 7, get_i );

            TRAPPROC( s, cyg_uint16, 1, get_splus );
            TRAPPROC( s, cyg_uint16, 3, get_splus );

            TRAPPROC( s, cyg_uint16, 5, get_s );
            TRAPPROC( s, cyg_uint16, 7, get_s );
        }

        CYG_TEST_PASS( "Unaligned accesses OK" );

        // Now test some legal and illegal accesses intermingled.

        for ( repeats = (cyg_test_is_simulator ? SIM_REPEATS : HW_REPEATS) ;
              repeats > 0 ; repeats-- ) {

            SAFEPROC( l, cyg_uint64, 0, get_llplus );
            TRAPPROC( l, cyg_uint64, 5, get_llplus );
            TRAPPROC( l, cyg_uint64, 6, get_llplus );
            SAFEPROC( l, cyg_uint64, 8, get_llplus );

            TRAPPROC( i, cyg_uint32, 1, get_iplus );
            SAFEPROC( i, cyg_uint32, 4, get_iplus );
            TRAPPROC( i, cyg_uint32, 2, get_iplus );
            SAFEPROC( i, cyg_uint32, 8, get_iplus );
            SAFEPROC( i, cyg_uint32, 12, get_iplus );
            SAFEPROC( i, cyg_uint32, 16, get_iplus );
            TRAPPROC( i, cyg_uint32, 3, get_iplus );

            TRAPPROC( s, cyg_uint16, 5, get_s );
            SAFEPROC( s, cyg_uint16, 6, get_s );
            TRAPPROC( s, cyg_uint16, 7, get_s );
            SAFEPROC( s, cyg_uint16, 8, get_s );
 
            TRAPPROC( i, cyg_uint32, 5, get_i );
            SAFEPROC( i, cyg_uint32, 4, get_i );
            TRAPPROC( i, cyg_uint32, 6, get_i );
            TRAPPROC( i, cyg_uint32, 7, get_i );
            SAFEPROC( i, cyg_uint32, 0, get_i );

            TRAPPROC( l, cyg_uint64, 4, get_ll );
            SAFEPROC( l, cyg_uint64, 0, get_ll );
            TRAPPROC( l, cyg_uint64, 1, get_ll );
            SAFEPROC( l, cyg_uint64, 8, get_ll );

            TRAPPROC( s, cyg_uint16, 1, get_splus );
            SAFEPROC( s, cyg_uint16, 2, get_splus );
            TRAPPROC( s, cyg_uint16, 3, get_splus );
            SAFEPROC( s, cyg_uint16, 4, get_splus );
        }

        CYG_TEST_PASS( "Mixture of accesses OK" );
    }
}

// -------------------------------------------------------------------------

externC void
skip_exception_handler(CYG_ADDRWORD vector, CYG_ADDRWORD data,
                                   CYG_ADDRWORD stackpointer);

externC void
fail_exception_handler(CYG_ADDRWORD vector, CYG_ADDRWORD data,
                                   CYG_ADDRWORD stackpointer);

// -------------------------------------------------------------------------

void sparc_ex_main( void )
{
    int i;

    CYG_TEST_INIT();

    for ( i = CYGNUM_HAL_EXCEPTION_MIN; i <= CYGNUM_HAL_EXCEPTION_MAX; i++ ){
        int j;
        HAL_TRANSLATE_VECTOR( i, j );
        HAL_INTERRUPT_ATTACH( j, &fail_exception_handler, j, 0 );
        // we must also ensure that eCos handles the exception;
        // do not drop into CygMon or equivalent.
        // Leave USER_TRAP undisturbed so that breakpoints work.
        if ( CYGNUM_HAL_VECTOR_USER_TRAP != i ) {
            extern void hal_default_exception_vsr( void );
            HAL_VSR_SET( i, (CYG_ADDRESS)hal_default_exception_vsr, NULL );
        }
    }

    HAL_TRANSLATE_VECTOR( CYGNUM_HAL_VECTOR_UNALIGNED, i );
    HAL_INTERRUPT_DETACH( i, &fail_exception_handler );
    HAL_INTERRUPT_ATTACH( i, &skip_exception_handler, i, 0 );

    CYG_TEST_INFO( "Vectors attached OK; calling do_test" );

    do_test();

    CYG_TEST_EXIT( "Done" );
}

// -------------------------------------------------------------------------
externC void
skip_exception_handler(CYG_ADDRWORD vector, CYG_ADDRWORD data,
                                   CYG_ADDRWORD stackpointer)
{
    HAL_SavedRegisters *save;
    HAL_FrameStructure *frame;

    HAL_FLUSH_REGISTERS_TO_STACK();

    save = (HAL_SavedRegisters *) stackpointer;
    frame = (HAL_FrameStructure *) (save + 1); // immediately after

    // now, this is the invokation environment when this saved regset
    // was created (copied from hal_xvsr.S):
    //  ! here,locals have been set up as follows:
    //  ! %l0 = psr (with this CWP/window-level in it)
    //  ! %l1 = pc
    //  ! %l2 = npc
    //  ! %l3 = vector number (16-25 for traps)
    //  ! and we are in our own register window, though it is likely that
    //  ! the next one will need to be saved before we can use it:
    //  ! ie. this one is the invalid register window.
    // and the intention is that we can mess therewith:

    // Check we're not in a trap loop
    if ( trap == save->li.l[1] ) {
        same++;
        if ( 10 < same )
            CYG_TEST_FAIL_EXIT( "Repeated trap" );
    }
    else // restart the count
       same = 0;

    // and record it
    trap = save->li.l[1];
    tcount++;

    // now step on so that we return to the instruction after:
    save->li.l[1] = save->li.l[2]; // PC := NPC
    save->li.l[2] += 4;            // NPC += 4

    // that's all.
}

externC void
fail_exception_handler(CYG_ADDRWORD vector, CYG_ADDRWORD data,
                                   CYG_ADDRWORD stackpointer)
{
    HAL_FLUSH_REGISTERS_TO_STACK();
    CYG_TEST_FAIL_EXIT( "Other exception handler called" );
}

// -------------------------------------------------------------------------

externC void
#ifdef CYGPKG_KERNEL
cyg_user_start( void )
#else
cyg_start( void )
#endif
{
    sparc_ex_main();
}

// -------------------------------------------------------------------------

cyg_uint64 get_ll( cyg_uint64 *p )
{
    return *p;
}

cyg_uint64 get_llplus( cyg_uint64 *p )
{
    cyg_uint64 ll = 0l, v;
    ll = (cyg_uint32)p;
    ll++;
    v = *p;
    v^= ll;
    return v;
}

cyg_uint32 get_i( cyg_uint32 *p )
{
    return *p;
}

cyg_uint32 get_iplus( cyg_uint32 *p )
{
    cyg_uint32 i = 0, v;
    i = (cyg_uint32)p;
    i++;
    v = *p;
    v^= i;
    return v;
}

cyg_uint16 get_s( cyg_uint16 *p )
{
    return *p;
}

cyg_uint16 get_splus( cyg_uint16 *p )
{
    cyg_uint16 s = 0, v;
    s = (cyg_uint16)(0xffff & (cyg_uint32)p);
    s++;
    v = *p;
    v^= s;
    return v;
}

// -------------------------------------------------------------------------

/* EOF sparc_ex.c */
