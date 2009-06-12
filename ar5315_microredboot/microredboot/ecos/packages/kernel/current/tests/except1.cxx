//=================================================================
//
//        except1.cxx
//
//        Exception test 1
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
// Author(s):     dsm
// Contributors:  dsm, jlarmour
// Date:          1999-02-16
// Description:   Test basic exception functionality
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/infra/testcase.h>

#ifdef CYGPKG_KERNEL_EXCEPTIONS

#include <cyg/kernel/sched.hxx>         // Cyg_Scheduler::start()
#include <cyg/kernel/thread.hxx>        // Cyg_Thread
#include <cyg/kernel/intr.hxx>          // cyg_VSR

#include <cyg/hal/hal_intr.h>           // exception ranges

#include <cyg/kernel/sched.inl>
#include <cyg/kernel/thread.inl>

#define NTHREADS 1
#include "testaux.hxx"

#ifndef CYGPKG_HAL_ARM_PID
#define EXCEPTION_DATA_ACCESS
#endif

static int d0;
#ifdef EXCEPTION_DATA_ACCESS
static cyg_exception_handler handler0;

static void handler0(CYG_ADDRWORD data, cyg_code number, CYG_ADDRWORD info)
{
    CYG_TEST_INFO("handler 0 called");
    
    CYG_TEST_CHECK((CYG_ADDRWORD)123 == data, "handler given wrong data");
    
    // ignore machine specific stuff
    CYG_UNUSED_PARAM(cyg_code, number);
    CYG_UNUSED_PARAM(CYG_ADDRWORD, info);

    CYG_TEST_PASS_FINISH("Except 1 OK");
}
#endif

static void handler1(CYG_ADDRWORD data, cyg_code number, CYG_ADDRWORD info)
{
    CYG_TEST_INFO("handler 1 called");

    CYG_TEST_CHECK((CYG_ADDRWORD)&d0 == data, "handler given wrong data");

#ifdef CYGSEM_KERNEL_EXCEPTIONS_DECODE
    CYG_TEST_CHECK(number == CYGNUM_HAL_EXCEPTION_MAX, "handler given wrong number");
#else
    CYG_UNUSED_PARAM(cyg_code, number);
#endif

    CYG_TEST_CHECK((CYG_ADDRWORD)99 == info, "handler given wrong info");
}

#ifdef EXCEPTION_DATA_ACCESS
// The following function attempts to cause an exception in various
// hacky ways.  It is machine dependent what exception is generated.
// It does reads rather than writes hoping not to corrupt anything
// important.
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

void cause_exception(void)
{
    int x;
    unsigned int p=0;

    // First try for an address exception (unaligned access exception
    // or SEGV/BUS exceptions)
    do {
        x=*(volatile int *)(p-1);
        p+=0x100000;
    } while(p != 0);

    // Next try an integer or floating point divide-by-zero exception.
    cause_fpe(0);
}
#endif

static void entry0( CYG_ADDRWORD data )
{
#ifdef EXCEPTION_DATA_ACCESS
    cyg_code n;
#endif
    cyg_exception_handler *old_handler, *old_handler1;
    CYG_ADDRWORD old_data, old_data1;
    Cyg_Thread *p=Cyg_Thread::self();

    CYG_UNUSED_PARAM(CYG_ADDRESS, data);

    p->register_exception(
        CYGNUM_HAL_EXCEPTION_MAX, 
        &handler1,
        (CYG_ADDRWORD)&d0,
        &old_handler,
        &old_data);

    p->register_exception(
        CYGNUM_HAL_EXCEPTION_MAX, 
        &handler1,
        (CYG_ADDRWORD)&d0,
        &old_handler1,
        &old_data1);
    
    CYG_TEST_CHECK(old_handler1 == &handler1,
        "register exception: old_handler not the one previously registered");
    CYG_TEST_CHECK(old_data1 == (CYG_ADDRWORD)&d0,
        "register exception: old_data not those previously registered");

    p->deliver_exception(CYGNUM_HAL_EXCEPTION_MAX, (CYG_ADDRWORD)99);

    CYG_TEST_INFO("handler 1 returned");

    p->deregister_exception(CYGNUM_HAL_EXCEPTION_MAX);
    p->deregister_exception(CYGNUM_HAL_EXCEPTION_MAX);

#ifdef EXCEPTION_DATA_ACCESS

#if 0
#elif defined(CYGPKG_HAL_POWERPC_SIM)
    // The exception generated by the SIM is not recognized by GDB.
    // PR 19945 workaround.
    CYG_TEST_NA("Not applicable to PowerPC SIM");
#endif

    for(n = CYGNUM_HAL_EXCEPTION_MIN; n <= CYGNUM_HAL_EXCEPTION_MAX; n++) {
        p->register_exception(
            n,
            handler0,
            (CYG_ADDRWORD)123, 
            &old_handler1,
            &old_data1);
    }

    CYG_TEST_PASS("Attempting to provoke exception");

    cause_exception();

    CYG_TEST_FAIL_FINISH("Couldn't cause exception");
#else // EXCEPTION_DATA_ACCESS
    CYG_TEST_NA("Platform does not support data exceptions");
#endif

}

#ifdef CYG_HAL_MIPS_TX39_JMR3904

externC cyg_VSR __default_exception_vsr;
cyg_VSR *old_vsr;

#endif

void except0_main( void )
{
    // Use CYG_TEST_GDBCMD _before_ CYG_TEST_INIT()
    CYG_TEST_GDBCMD("handle SIGBUS nostop");
    CYG_TEST_GDBCMD("handle SIGSEGV nostop");
    CYG_TEST_GDBCMD("handle SIGFPE nostop");

    CYG_TEST_INIT();

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

    new_thread(entry0, 0);

    Cyg_Scheduler::start();

    CYG_TEST_FAIL_FINISH("Not reached");
}
externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    except0_main();
}
#else // def CYGPKG_KERNEL_EXCEPTIONS
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Exceptions disabled");
}
#endif // def CYGPKG_KERNEL_EXCEPTIONS

// EOF except1.cxx
