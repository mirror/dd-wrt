//==========================================================================
//
//        signal2.cxx
//
//        POSIX signal test 2
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
// Contributors:  jlarmour
// Date:          2000-04-10
// Description:   Tests POSIX signal functionality.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/isoinfra.h>
#include <cyg/hal/hal_intr.h>   // For exception codes

#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

#include <setjmp.h>

#include <cyg/infra/testcase.h>

#if CYGINT_ISO_SETJMP == 0
# define NA_MSG "Requires setjmp/longjmp implementation"
#elif !defined(CYGPKG_POSIX_SIGNALS)
# define NA_MSG "POSIX signals not enabled"
#endif

#ifdef NA_MSG
void
cyg_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA( NA_MSG );
}
#else

//--------------------------------------------------------------------------
// Local variables

static jmp_buf jbuf;

//--------------------------------------------------------------------------

// PowerPC is a special case as it has the alignment exception, but it
// doesn't trigger for this function unless in little-endian mode (although
// the exception exists for other instructions not used by this function so
// CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS will still be defined

#if defined(CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS) && !(defined(CYGPKG_HAL_POWERPC) && (CYG_BYTEORDER==CYG_MSBFIRST))

static void
cause_unaligned_access(void)
{
    volatile int x;
    volatile CYG_ADDRESS p=(CYG_ADDRESS) &jbuf;

    x = *(volatile int *)(p+1);

} // cause_unaligned_access()

#endif

//--------------------------------------------------------------------------

#ifdef CYGNUM_HAL_EXCEPTION_DATA_ACCESS

static void
cause_illegal_access(void)
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
    volatile CYG_ADDRESS p=(CYG_ADDRESS) &jbuf;

    do
    {
        x = *(volatile int *)(p);
        p += (CYG_ADDRESS)0x100000;
    } while( p != (CYG_ADDRESS)&jbuf );

#endif    
} // cause_illegal_access()

#endif

//--------------------------------------------------------------------------

#ifdef CYGNUM_HAL_EXCEPTION_FPU_DIV_BY_ZERO

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

#endif

//--------------------------------------------------------------------------
// Signal handler functions

static void sigsegv( int signo )
{
    CYG_TEST_INFO( "sigsegv() handler called" );
    CYG_TEST_CHECK( signo == SIGSEGV, "Signal not SIGSEGV");

    longjmp( jbuf, 1 );
}

static void sigbus( int signo )
{
    CYG_TEST_INFO( "sigbus() handler called" );
    CYG_TEST_CHECK( signo == SIGBUS, "Signal not SIGBUS");

    longjmp( jbuf, 1 );
}

static void sigfpe( int signo )
{
    CYG_TEST_INFO( "sigfpe() handler called" );
    CYG_TEST_CHECK( signo == SIGFPE, "Signal not SIGFPE");

    longjmp( jbuf, 1 );
}


//--------------------------------------------------------------------------

int main(int argc, char **argv)
{
    int ret;
    sigset_t mask;
    struct sigaction sa;
    
    CYG_TEST_INIT();

    // Make a full signal set
    sigfillset( &mask );

   
    // Install signal handlers

    sa.sa_mask = mask;
    sa.sa_flags = 0;

    sa.sa_handler = sigsegv;
    ret = sigaction( SIGSEGV, &sa, NULL );
    CYG_TEST_CHECK( ret == 0 , "sigaction returned error");

    sa.sa_handler = sigbus;
    ret = sigaction( SIGBUS, &sa, NULL );
    CYG_TEST_CHECK( ret == 0 , "sigaction returned error");

    sa.sa_handler = sigfpe;
    ret = sigaction( SIGFPE, &sa, NULL );
    CYG_TEST_CHECK( ret == 0 , "sigaction returned error");

    // now make an empty signal set
    sigemptyset( &mask );
    
// Now reset the various exception handlers to eCos handlers so that we
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
#ifdef CYGNUM_HAL_EXCEPTION_FPU_DIV_BY_ZERO
    HAL_VSR_SET_TO_ECOS_HANDLER( CYGNUM_HAL_EXCEPTION_FPU_DIV_BY_ZERO, NULL );
#endif
#endif

    // PowerPC is a special case as it has the alignment exception, but it
    // doesn't trigger for this function unless in little-endian mode (although
    // the exception exists for other instructions not used by this function so
    // CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS will still be defined

#if defined(CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS) && !(defined(CYGPKG_HAL_POWERPC) && (CYG_BYTEORDER==CYG_MSBFIRST))
    
    CYG_TEST_INFO("Test 1 - provoke unaligned access");
    
    if( setjmp( jbuf ) == 0 )
    {
        pthread_sigmask( SIG_SETMASK, &mask, NULL );
        cause_unaligned_access();
        CYG_TEST_FAIL("Didn't cause exception");        
    }

#else

    CYG_TEST_INFO("Test 1 - provoke unaligned access - not supported");

#endif    

#ifdef CYGNUM_HAL_EXCEPTION_DATA_ACCESS
    
    CYG_TEST_INFO("Test 2 - provoke illegal access");
    
    if( setjmp( jbuf ) == 0 )
    {
        pthread_sigmask( SIG_SETMASK, &mask, NULL );
        cause_illegal_access();
        CYG_TEST_FAIL("Didn't cause exception");        
    }

#else

    CYG_TEST_INFO("Test 1 - provoke illegal access - not supported");

#endif    
    
#ifdef CYGNUM_HAL_EXCEPTION_FPU_DIV_BY_ZERO
    
    CYG_TEST_INFO("Test 3 - provoke FP error");    

    if( setjmp( jbuf ) == 0 )
    {
        pthread_sigmask( SIG_SETMASK, &mask, NULL );
        cause_fpe(0);
        CYG_TEST_FAIL("Didn't cause exception");        
    }

#else

    CYG_TEST_INFO("Test 3 - provoke FP error - not supported");    
    
#endif    

    CYG_TEST_PASS_FINISH( "signal2" );
}

#endif // ifndef NA_MSG

//--------------------------------------------------------------------------
// end of signal1.c
