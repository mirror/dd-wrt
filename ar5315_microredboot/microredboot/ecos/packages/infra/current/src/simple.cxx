//==========================================================================
//
//      simple.cxx
//
//      Simple, non formatting trace and assert functions
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-12-04
// Purpose:     Simple Trace and assert functions
// Description: The functions in this file are simple implementations
//              of the standard trace and assert functions. These do not
//              do any printf style formatting, simply printing the string
//              and arguments directly.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/infra.h>

#ifdef CYGDBG_INFRA_DEBUG_TRACE_ASSERT_SIMPLE

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <pkgconf/hal.h>                // HAL configury
#include <cyg/infra/diag.h>             // HAL polled output
#include <cyg/hal/hal_arch.h>           // architectural stuff for...
#include <cyg/hal/hal_intr.h>           // interrupt control

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>             // kernel configury
#include <cyg/kernel/thread.hxx>        // thread id to print
#include <cyg/kernel/sched.hxx>         // ancillaries for above
#include <cyg/kernel/thread.inl>        // ancillaries for above
#endif

// -------------------------------------------------------------------------
// Local Configuration: hack me!

#define CYG_NO_FILENAME 1
#define CYG_NO_THREADID 0
#define CYG_NO_LINENUM  0
#define CYG_NO_FUNCNAME 0
#define CYG_DIAG_PRINTF 1

#ifndef CYGPKG_KERNEL
# undef  CYG_NO_THREADID
# define CYG_NO_THREADID 1
#endif
// -------------------------------------------------------------------------
// Functions to trim file names and function names down to printable lengths
// (these are shared between trace and assert functions)

static const char *trim_file(const char *file)
{
#if !CYG_NO_FILENAME
    if ( NULL == file )
        file = "<nofile>";
    
    const char *f = file;
    
    while( *f ) f++;

    while( *f != '/' && f != file ) f--;

    return f==file?f:(f+1);
#else
    return "";
#endif
}

static const char *trim_func(const char *func)
{
#if !CYG_NO_FUNCNAME
    
    static char fbuf[100];
    int i;
    
    if ( NULL == func )
        func = "<nofunc>";

    for( i = 0; func[i] && func[i] != '(' ; i++ )
        fbuf[i] = func[i];

    fbuf[i++] = '(';
    fbuf[i++] = ')';
    fbuf[i  ] = 0;

    return &fbuf[0];
#else
    return "";
#endif
}

void write_lnum( cyg_uint32 lnum)
{
#if !CYG_NO_LINENUM
    diag_write_char('[');
    diag_write_dec(lnum);
    diag_write_char(']');
#endif
}

void write_thread_id()
{
#if !CYG_NO_THREADID    
    Cyg_Thread *t = Cyg_Thread::self();
    cyg_uint16 tid = 0xFFFF;

    if( t != NULL ) tid = t->get_unique_id();

    diag_write_char('<');
    diag_write_hex(tid);
    diag_write_char('>');
#endif
}


// -------------------------------------------------------------------------
// Trace functions:

#ifdef CYGDBG_USE_TRACING

externC void
cyg_tracenomsg( const char *psz_func, const char *psz_file, cyg_uint32 linenum )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char('\n');

    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
    
};

// provide every other one of these as a space/caller bloat compromise.

externC void
cyg_tracemsg( cyg_uint32 what, 
              const char *psz_func, const char *psz_file, cyg_uint32 linenum,
              const char *psz_msg )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    if ( NULL == psz_msg )
        psz_msg = "<nomsg>";

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
    diag_write_string(psz_msg);
    diag_write_char('\n');

    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);

};

externC void
cyg_tracemsg2( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum,
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1 )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    if ( NULL == psz_msg )
        psz_msg = "<nomsg>";

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
#if CYG_DIAG_PRINTF
    diag_printf( psz_msg, arg0, arg1 );
#else
    diag_write_string(psz_msg);
    diag_write_char(' ');
    diag_write_hex(arg0);
    diag_write_char(' ');
    diag_write_hex(arg1);
#endif    
    diag_write_char('\n');    
    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
};

externC void
cyg_tracemsg4( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum, 
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3 )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    if ( NULL == psz_msg )
        psz_msg = "<nomsg>";

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
#if CYG_DIAG_PRINTF
    diag_printf( psz_msg, arg0, arg1, arg2, arg3 );
#else
    diag_write_string(psz_msg);
    diag_write_char(' ');
    diag_write_hex(arg0);
    diag_write_char(' ');
    diag_write_hex(arg1);
    diag_write_char(' ');
    diag_write_hex(arg2);
    diag_write_char(' ');
    diag_write_hex(arg3);
#endif    
    diag_write_char('\n');    

    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
};

externC void
cyg_tracemsg6( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum,
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3,
               CYG_ADDRWORD arg4,  CYG_ADDRWORD arg5 )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    if ( NULL == psz_msg )
        psz_msg = "<nomsg>";

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
#if CYG_DIAG_PRINTF
    diag_printf( psz_msg, arg0, arg1, arg2, arg3, arg4, arg5 );
#else
    diag_write_string(psz_msg);
    diag_write_char(' ');
    diag_write_hex(arg0);
    diag_write_char(' ');
    diag_write_hex(arg1);
    diag_write_char(' ');
    diag_write_hex(arg2);
    diag_write_char(' ');
    diag_write_hex(arg3);
    diag_write_char(' ');
    diag_write_hex(arg4);
    diag_write_char(' ');
    diag_write_hex(arg5);
#endif    
    diag_write_char('\n');    

    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
};

externC void
cyg_tracemsg8( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum,
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3,
               CYG_ADDRWORD arg4,  CYG_ADDRWORD arg5,
               CYG_ADDRWORD arg6,  CYG_ADDRWORD arg7 )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    if ( NULL == psz_msg )
        psz_msg = "<nomsg>";

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
#if CYG_DIAG_PRINTF
    diag_printf( psz_msg, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7 );
#else
    diag_write_string(psz_msg);
    diag_write_char(' ');
    diag_write_hex(arg0);
    diag_write_char(' ');
    diag_write_hex(arg1);
    diag_write_char(' ');
    diag_write_hex(arg2);
    diag_write_char(' ');
    diag_write_hex(arg3);
    diag_write_char(' ');
    diag_write_hex(arg4);
    diag_write_char(' ');
    diag_write_hex(arg5);
    diag_write_char(' ');
    diag_write_hex(arg6);
    diag_write_char(' ');
    diag_write_hex(arg7);
#endif    
    diag_write_char('\n');    

    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
};

// -------------------------------------------------------------------------

externC void
cyg_trace_dump(void)
{
#if defined(CYGPKG_KERNEL) && defined(CYG_DIAG_PRINTF)

    {
        diag_printf("\nScheduler:\n\n");

        Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;

        diag_printf("Lock:                %d\n",sched->get_sched_lock() );

# ifdef CYGVAR_KERNEL_THREADS_NAME
    
        diag_printf("Current Thread:      %s\n",sched->get_current_thread()->get_name());

# else

        diag_printf("Current Thread:    %d\n",sched->get_current_thread()->get_unique_id());
    
# endif

    }
    
# ifdef CYGVAR_KERNEL_THREADS_LIST

    {
        Cyg_Thread *t = Cyg_Thread::get_list_head();

        diag_printf("\nThreads:\n\n");
    
        while( NULL != t )
        {
            cyg_uint32 state = t->get_state();
            char tstate[7];
            char *tstate1 = "SCUKX";
            static char *(reasons[8]) =
            {
                "NONE",                           // No recorded reason
                "WAIT",                           // Wait with no timeout
                "DELAY",                          // Simple time delay
                "TIMEOUT",                        // Wait with timeout/timeout expired
                "BREAK",                          // forced break out of sleep
                "DESTRUCT",                       // wait object destroyed[note]
                "EXIT",                           // forced termination
                "DONE"                            // Wait/delay complete
            };

            if( 0 != state )
            {
                // Knock out chars that do not correspond to set bits.
                for( int i = 0; i < 6 ; i++ )
                    if( 0 == (state & (1<<i)) )
                        tstate[i] = ' ';
                    else tstate[i] = tstate1[i];
                tstate[6] = 0;
            }
            else tstate[0] = 'R', tstate[1] = 0;

#   ifdef CYGVAR_KERNEL_THREADS_NAME
        
            diag_printf( "%20s pri = %3d state = %6s id = %3d\n",
                         t->get_name(),
                         t->get_priority(),
                         tstate,
                         t->get_unique_id()
                );
#   else

            diag_printf( "Thread %3d        pri = %3d state = %6s\n",
                         t->get_unique_id(),
                         t->get_priority(),
                         tstate
                );

#   endif        
            diag_printf( "%20s stack base = %08x ptr = %08x size = %08x\n",
                         "",
                         t->get_stack_base(),
#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT
                         t->get_saved_context(),
#else
                         0,
#endif
                         t->get_stack_size()
                );

            diag_printf( "%20s sleep reason %8s wake reason %8s\n",
                         "",
                         reasons[t->get_sleep_reason()],
                         reasons[t->get_wake_reason()]
                );

            diag_printf( "%20s queue = %08x      wait info = %08x\n",
                         "",
                         t->get_current_queue(),
                         t->get_wait_info()
                         );

            diag_printf("\n");
            t = t->get_list_next();
        }

    }
# endif // CYGVAR_KERNEL_THREADS_LIST
    
#endif // CYG_DIAG_PRINTF
}

#endif // CYGDBG_USE_TRACING

// -------------------------------------------------------------------------
// Assert functions:

#ifdef CYGDBG_USE_ASSERTS

externC void
cyg_assert_fail( const char *psz_func, const char *psz_file,
                 cyg_uint32 linenum, const char *psz_msg ) __THROW
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    diag_write_string("ASSERT FAIL: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
    diag_write_string(psz_msg);
    diag_write_char('\n');

#ifdef CYGHWR_TEST_PROGRAM_EXIT
    CYGHWR_TEST_PROGRAM_EXIT();
#endif
    for(;;);

//    DIAG_DEVICE_END_SYNC();
//    HAL_RESTORE_INTERRUPTS(old_ints);
};

extern "C"
{
extern unsigned long _stext;
extern unsigned long _etext;

unsigned long stext_addr = (unsigned long)&_stext;
unsigned long etext_addr = (unsigned long)&_etext;
};

externC cyg_bool cyg_check_data_ptr(const void *ptr)
{
    unsigned long p = (unsigned long)ptr;
    
    if( p == 0 ) return false;

    if( p > stext_addr && p < etext_addr ) return false;

    return true;
}

externC cyg_bool cyg_check_func_ptr(const void (*ptr)(void))
{
    unsigned long p = (unsigned long)ptr;
    
    if( p == 0 ) return false;

    if( p < stext_addr && p > etext_addr ) return false;

    return true;
}

#endif // CYGDBG_USE_ASSERTS

#endif // CYGDBG_INFRA_DEBUG_TRACE_ASSERT_SIMPLE

// -------------------------------------------------------------------------
// EOF simple.cxx
