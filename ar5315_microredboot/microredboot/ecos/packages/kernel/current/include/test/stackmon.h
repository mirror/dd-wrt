#ifndef CYGONCE_KERNEL_TEST_STACKMON_H
#define CYGONCE_KERNEL_TEST_STACKMON_H

/*=================================================================
//
//        stackmon.h
//
//        Auxiliary test header file
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
// Date:          1999-05-20
// Description:
//     Defines some convenience functions for stack use output.
// Note:
//     The functions are defined for both C and C++ usage - with different
//     argument types.
//
//####DESCRIPTIONEND####
*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_type.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
# if defined(CYGFUN_KERNEL_API_C)
#  include <cyg/kernel/kapi.h>
# endif
# if defined(__cplusplus)
#  include <cyg/kernel/sched.hxx>
#  include <cyg/kernel/thread.hxx>
#  include <cyg/kernel/thread.inl>
# endif
# include <cyg/kernel/smp.hxx>
#endif

#ifndef STACKMON_PRINTF
#include <cyg/infra/diag.h>
#define STACKMON_PRINTF diag_printf
#endif

// ------------------------------------------------------------------------
// Utility function for actually counting a stack

inline void cyg_test_size_a_stack( char *comment, char *format,
                                   char *base, char *top )
{
#ifdef CYGFUN_KERNEL_THREADS_STACK_CHECKING
    cyg_uint32* cur32   = (cyg_uint32*) ((((CYG_ADDRWORD)&(base[CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE])) + 3) & ~3);
    cyg_uint32* top32   = (cyg_uint32*) ((((CYG_ADDRWORD)top) + 3) & ~3);
    for ( ; cur32 < top32; cur32++) {
        if (*cur32 != 0xDEADBEEF) {
            break;
        }
    }
    STACKMON_PRINTF( format, comment, (CYG_ADDRWORD)top32 - (CYG_ADDRWORD)cur32, top - base );
#else    
    register char *p;
    for ( p = base; p < top; p++ )
        if ( *p )
            break;
    STACKMON_PRINTF( format, comment, top - p, top - base );
#endif    
}

// ------------------------------------------------------------------------

inline void cyg_test_dump_stack_stats( char *comment,
                                       char *base, char *top )
{
    cyg_test_size_a_stack( comment, "%31s : stack used %5d size %5d\n",
                           base, top );
}

// ------------------------------------------------------------------------

#ifdef __cplusplus

inline void cyg_test_dump_thread_stack_stats( char *comment,
                                              Cyg_Thread *p )
{
#if defined(CYGPKG_KERNEL)
    char *base, *top;
    base = (char *)p->get_stack_base();
    top =   base + p->get_stack_size();
    cyg_test_dump_stack_stats( comment, base, top );
#endif
}

#else // __cplusplus

inline void cyg_test_dump_thread_stack_stats( char *comment,
                                              cyg_handle_t p )
{
#if defined(CYGPKG_KERNEL) && defined(CYGFUN_KERNEL_API_C)
    char *base, *top;
    base = (char *) cyg_thread_get_stack_base( p );
    top =   base + cyg_thread_get_stack_size( p );
    cyg_test_dump_stack_stats( comment, base, top );
#endif
}

#endif // __cplusplus

// ------------------------------------------------------------------------
// Print out size of idle thread stack usage since start-of-time.  Only
// meaningful if there is a scheduler.

#ifdef __cplusplus

inline void cyg_test_dump_idlethread_stack_stats( char *comment )
{
#if defined(CYGPKG_KERNEL)
    int i;
    extern Cyg_Thread idle_thread[CYGNUM_KERNEL_CPU_MAX];
    for( i = 0; i < CYGNUM_KERNEL_CPU_MAX; i++ )
    {
        // idle thread is not really a plain CygThread; danger.
        char *ibase  = (char *)idle_thread[i].get_stack_base();
        char *istack = ibase + idle_thread[i].get_stack_size();
        cyg_test_size_a_stack( comment,
                               "%20s : Idlethread stack used %5d size %5d\n",
                               ibase, istack );
    }
#endif
}

#else // __cplusplus

inline void cyg_test_dump_idlethread_stack_stats( char *comment )
{
#if defined(CYGPKG_KERNEL) && defined(CYGFUN_KERNEL_API_C)
    cyg_handle_t idle_thread = cyg_thread_idle_thread();

    char *ibase  = (char *)cyg_thread_get_stack_base( idle_thread );
    char *istack = ibase + cyg_thread_get_stack_size( idle_thread );
    cyg_test_size_a_stack( comment,
              "%20s : Idlethread stack used %5d size %5d\n",
              ibase, istack );
#endif
}

#endif // __cplusplus

// ------------------------------------------------------------------------
// Print out size of interrupt stack usage since start-of-time or since it
// was last cleared.  NB on some architectures and configurations, the
// interrupt stack is the same as the bootup stack, so clear it in the
// first first thread to execute.  Clearing it before scheduler start would
// be fatal!

#if defined(HAL_INTERRUPT_STACK_BASE) && defined(HAL_INTERRUPT_STACK_TOP)
externC char HAL_INTERRUPT_STACK_BASE[];
externC char HAL_INTERRUPT_STACK_TOP[];
#endif

inline void cyg_test_dump_interrupt_stack_stats( char *comment )
{
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
#if defined(HAL_INTERRUPT_STACK_BASE) && defined(HAL_INTERRUPT_STACK_TOP)
    cyg_test_size_a_stack( comment,
              "%20s :  Interrupt stack used %5d size %5d\n",
              HAL_INTERRUPT_STACK_BASE, HAL_INTERRUPT_STACK_TOP );
#endif
#endif
}

// Clear interrupt stack to reset stats - only after sched has started.

inline void cyg_test_clear_interrupt_stack( void )
{
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
#if defined(HAL_INTERRUPT_STACK_BASE) && defined(HAL_INTERRUPT_STACK_TOP)
    cyg_uint32  old_intr;
    HAL_DISABLE_INTERRUPTS(old_intr);
# ifdef CYGFUN_KERNEL_THREADS_STACK_CHECKING
    {
        cyg_uint32* cur32   = (cyg_uint32*) ((((CYG_ADDRWORD)HAL_INTERRUPT_STACK_BASE) + 3) & ~3);
        cyg_uint32* top32   = (cyg_uint32*) ((((CYG_ADDRWORD)HAL_INTERRUPT_STACK_TOP) + 3) & ~3);
        for ( ; cur32 < top32; cur32++) {
            *cur32 = 0xDEADBEEF;
        }
    }
# else
    {
        register char *p;
        for ( p = HAL_INTERRUPT_STACK_BASE; p < HAL_INTERRUPT_STACK_TOP; p++ )
            *p = 0;                         // zero it for checking later
    }
# endif    
    HAL_RESTORE_INTERRUPTS(old_intr);
#endif
#endif
}

// ------------------------------------------------------------------------

#endif // ifndef CYGONCE_KERNEL_TEST_STACKMON_H

// EOF stackmon.h
