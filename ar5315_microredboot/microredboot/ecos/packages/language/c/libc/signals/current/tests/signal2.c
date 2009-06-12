//========================================================================
//
//      signal2.c
//
//      ISO C signal handling test
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-18
// Purpose:       Test hardware signal functionality
// Description:   This file contains a number of tests for ISO C signal
//                handling when used with hardware exceptions
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/system.h>
#include <pkgconf/libc_signals.h>  // C library signals configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/testcase.h>    // Test infrastructure

#ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS

#include <cyg/hal/hal_intr.h>      // exception ranges &
                                   // HAL_VSR_SET_TO_ECOS_HANDLER

#include <signal.h>                // Signal functions
#include <setjmp.h>                // setjmp(), longjmp()

// STATICS

static int state;
static jmp_buf jbuf;
#endif

// FUNCTIONS

#ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS

static void
myhandler(int sig)
{
    __sighandler_t handler1;

    CYG_TEST_INFO("myhandler() called");
    ++state;

    handler1 = signal(sig, &myhandler);

    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "handler reset itself to default");

    longjmp(jbuf, 1);
} // myhandler()

static void
cause_memerror(void)
{
#ifdef CYGPKG_HAL_I386

    // In the x86 architecture, although we have the DATA_ACCESS
    // exception available, it is not possible to provoke it using the
    // normal code of this test. This is because the normal segments we
    // have installed in the segment registers cover all of memory. Instead we
    // set GS to a descriptor that does not cover 0xF0000000-0xFFFFFFFF and
    // poke at that.

    __asm__ ( "movw     $0x20,%%ax\n"
              "movw     %%ax,%%gs\n"
              "movl     %%gs:0xF0000000,%%eax\n"
              :
              :
              : "eax"
            );
    
#else    
    volatile int x;
    volatile CYG_ADDRESS p=(CYG_ADDRESS) &state;

    do {
        // do a read which prevents us accidentally writing over something
        // important. Make it misaligned to increase the chances of an
        // exception happening
        x = *(volatile int *)(p+1);
        p += (CYG_ADDRESS)0x100000;
    } while(p != 0);
#endif    
} // cause_memerror()

// num must always be 0 - do it this way in case the optimizer tries to
// get smart
static int
cause_fpe(int num)
{
    double a;

    a = 1.0/num;                        // Depending on FPU emulation and/or
                                        // the FPU architecture, this may
                                        // cause an exception.
                                        // (float division by zero)

    return ((int)a)/num;                // This may cause an exception if
                                        // the architecture supports it.
                                        // (integer division by zero).
} // cause_fpe()

volatile int tmp;

#endif


int
main( int argc, char *argv[] )
{
#ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS
    __sighandler_t handler1;

    // special callout to request GDB to alter its handling of signals
    CYG_TEST_GDBCMD("handle SIGBUS nostop");
    CYG_TEST_GDBCMD("handle SIGSEGV nostop");
    CYG_TEST_GDBCMD("handle SIGILL nostop");
    CYG_TEST_GDBCMD("handle SIGFPE nostop");
    CYG_TEST_GDBCMD("handle SIGSYS nostop");
#endif // ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS

    CYG_TEST_INIT();

#ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS
    // Avoid compiler warnings if tests are not applicable.
    if (0) cause_memerror();
    if (0) cause_fpe(0);
#endif
    
    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C "
                  "library signal functions");

// Now reset the exception handlers various to eCos handlers so that we
// have control; this is the target side equivalent of the CYG_TEST_GDBCMD
// lines above:
#ifdef HAL_VSR_SET_TO_ECOS_HANDLER
    // Reclaim the VSR off CygMon possibly
#ifdef CYGNUM_HAL_EXCEPTION_DATA_ACCESS
    HAL_VSR_SET_TO_ECOS_HANDLER( CYGNUM_HAL_EXCEPTION_DATA_ACCESS, NULL );
#endif
#ifdef CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_ACCESS
    HAL_VSR_SET_TO_ECOS_HANDLER( CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_ACCESS, NULL );
#endif
#ifdef CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS
    HAL_VSR_SET_TO_ECOS_HANDLER( CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS, NULL );
#endif
#ifdef CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION
    HAL_VSR_SET_TO_ECOS_HANDLER( CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION, NULL );
#endif
#ifdef CYGNUM_HAL_EXCEPTION_DIV_BY_ZERO
    HAL_VSR_SET_TO_ECOS_HANDLER( CYGNUM_HAL_EXCEPTION_DIV_BY_ZERO, NULL );
#endif
#ifdef CYGNUM_HAL_EXCEPTION_FPU
    HAL_VSR_SET_TO_ECOS_HANDLER( CYGNUM_HAL_EXCEPTION_FPU, NULL );
#endif
#ifdef CYGNUM_HAL_EXCEPTION_FPU_DIV_BY_ZERO
    HAL_VSR_SET_TO_ECOS_HANDLER( CYGNUM_HAL_EXCEPTION_FPU_DIV_BY_ZERO, NULL );
#endif
#endif

#ifdef CYGSEM_LIBC_SIGNALS_HWEXCEPTIONS


    // Test 1

    CYG_TEST_INFO("Test 1");

    handler1 = signal(SIGSEGV, &myhandler);

    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGSEGV handler initialized to default");


    handler1 = signal(SIGBUS, &myhandler);

    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGBUS handler initialized to default");


    handler1 = signal(SIGILL, &myhandler);

    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGILL handler initialized to default");

    handler1 = signal(SIGSYS, &myhandler);

    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGSYS handler initialized to default");

    handler1 = signal(SIGTRAP, &myhandler);

    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGTRAP handler initialized to default");

    handler1 = signal(SIGFPE, &myhandler);

    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGFPE handler initialized to default");

    // Test 2

    CYG_TEST_INFO("Test 2");

    state = 2;

#if defined(CYGPKG_HAL_POWERPC_SIM)
    // The exception generated by the SIM is not recognized by GDB.
    // PR 19945 workaround.
    CYG_TEST_PASS("Test 2 not applicable to PowerPC SIM");
#else
    if (0==setjmp(jbuf)) {
        cause_memerror();
        CYG_TEST_FAIL("Didn't cause exception");
    }
    
    CYG_TEST_PASS_FAIL(3==state, "handler returned correctly");
#endif

    // Test 3

    CYG_TEST_INFO("Test 3");
    
    state = 3;

#ifdef CYGNUM_HAL_EXCEPTION_FPU_DIV_BY_ZERO

    if (0==setjmp(jbuf)) {
        // It is necessary to save the return value in a volatile
        // variable, or GCC will get rid of the call.
        tmp = cause_fpe(0);
        CYG_TEST_FAIL("Didn't cause exception");
    }
    
    CYG_TEST_PASS_FAIL(4==state, "handler returned correctly");

#else

    CYG_TEST_INFO("Test 3 - provoke FP error - not supported");    
    
#endif    

#else
    CYG_TEST_NA("Testing not applicable to this configuration");
#endif

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C "
                    "library signal functions");

    return 0;
} // main()

// EOF signal2.c
