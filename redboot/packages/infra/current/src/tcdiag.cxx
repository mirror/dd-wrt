//==========================================================================
//
//        tcdiag.cxx
//
//        Infrastructure diag test harness.
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
// Contributors:  dsm, jlarmour
// Date:          1999-02-16
// Description:   Test harness implementation that uses the infrastructure
//                diag channel.  This is intended for manual testing.
// 
//####DESCRIPTIONEND####

#include <pkgconf/infra.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_TARGET_H           // get initialization for
#include CYGBLD_HAL_PLATFORM_H         //   cyg_test_is_simulator

#include <cyg/infra/cyg_type.h>        // base types
#include <cyg/hal/hal_arch.h>          // any architecture specific stuff
#include <cyg/infra/diag.h>            // HAL polled output
#include <cyg/infra/testcase.h>        // what we implement

#include <cyg/hal/hal_intr.h>          // exit macro, if defined

#ifdef CYGHWR_TARGET_SIMULATOR_NO_GDB_WORKING
int cyg_test_is_simulator = 1;         // set this anyway
#else
int cyg_test_is_simulator = 0;         // infrastructure changes as necessary
#endif

//----------------------------------------------------------------------------
// Functions ensuring we get pretty printed assertion messages in the
// farm - regardless of configuration and GDB capabilities.

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
# include <cyg/kernel/thread.hxx>        // thread id to print
# include <cyg/kernel/thread.inl>        // ancillaries for above
#endif

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
#include <cyg/hal/hal_if.h>
#endif

static inline const char *trim_file(const char *file)
{
    if ( NULL == file )
        file = "<nofile>";
    
    const char *f = file;
    
    while( *f ) f++;

    while( *f != '/' && f != file ) f--;

    return f==file?f:(f+1);
}

static inline const char *trim_func(const char *func)
{
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
}

static inline
void write_lnum( cyg_uint32 lnum)
{
    diag_write_char('[');
    diag_write_dec(lnum);
    diag_write_char(']');
}

static inline
void write_thread_id()
{
#ifdef CYGPKG_KERNEL
    Cyg_Thread *t = Cyg_Thread::self();
    cyg_uint16 tid = 0xFFFF;

    if( t != NULL ) tid = t->get_unique_id();

    diag_write_char('<');
    diag_write_hex(tid);
    diag_write_char('>');
#endif
}

// Called from the CYG_ASSERT_DOCALL macro
externC void
cyg_assert_msg( const char *psz_func, const char *psz_file,
                cyg_uint32 linenum, const char *psz_msg ) __THROW
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

#ifdef CYG_HAL_DIAG_LOCK
    CYG_HAL_DIAG_LOCK();
#endif    
    
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    {
        int cur_console;
        int i;
        struct cyg_fconfig fc;

        cur_console = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
        fc.key = "info_console_force";
        fc.type = CYGNUM_FLASH_CFG_TYPE_CONFIG_BOOL;
        fc.val = (void *)&i;
        if (CYGACC_CALL_IF_FLASH_CFG_OP2(CYGNUM_CALL_IF_FLASH_CFG_GET, &fc)) {
            if (i) {
                fc.key = "info_console_number";
                fc.type = CYGNUM_FLASH_CFG_TYPE_CONFIG_INT;
                if (CYGACC_CALL_IF_FLASH_CFG_OP2(CYGNUM_CALL_IF_FLASH_CFG_GET, &fc)) {
                    // Then i is the console to force it to:
                    CYGACC_CALL_IF_SET_CONSOLE_COMM(i);
                }
            }
        }
#endif
    diag_write_string("ASSERT FAIL: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
    diag_write_string(psz_msg);
    diag_write_char('\n');

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
        CYGACC_CALL_IF_SET_CONSOLE_COMM(cur_console);
    }
#endif
#ifdef CYG_HAL_DIAG_UNLOCK
    CYG_HAL_DIAG_UNLOCK();
#endif    
    
    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
}

externC void
cyg_test_init(void)
{
    // currently nothing
}

externC void
cyg_test_output(Cyg_test_code status, const char *msg, int line,
                const char *file)
{
    char *st;

    switch (status) {
    case CYGNUM_TEST_FAIL:
        st = "FAIL:";
        break;
    case CYGNUM_TEST_PASS:
        st = "PASS:";
        break;
    case CYGNUM_TEST_EXIT:
        st = "EXIT:";
        break;
    case CYGNUM_TEST_INFO:
        st = "INFO:";
        break;
    case CYGNUM_TEST_GDBCMD:
        st = "GDB:";
        break;
    case CYGNUM_TEST_NA:
        st = "NOTAPPLICABLE:";
        break;
    default:
        st = "UNKNOWN STATUS:";
        break;
    }

#ifdef CYG_HAL_DIAG_LOCK
    CYG_HAL_DIAG_LOCK();
#endif    
    
    diag_write_string(st);
    diag_write_char('<');
    diag_write_string(msg);
    diag_write_char('>');
    if( CYGNUM_TEST_FAIL == status ) {
        diag_write_string(" Line: ");
        diag_write_dec(line);
        diag_write_string(", File: ");
        diag_write_string(file);
    }
    diag_write_char('\n');

#ifdef CYG_HAL_DIAG_UNLOCK
    CYG_HAL_DIAG_UNLOCK();
#endif    
    
    
}

// This is an appropriate function to set a breakpoint on
externC void
cyg_test_exit(void)
{
// workaround SH dwarf2 gen problem    
#if defined(CYGPKG_HAL_SH) && (__GNUC__ >= 3)
    static volatile int i;
    i++;
#endif
#ifdef CYGHWR_TEST_PROGRAM_EXIT
    CYGHWR_TEST_PROGRAM_EXIT();
#endif
#ifdef CYGSEM_INFRA_RESET_ON_TEST_EXIT
#ifdef HAL_PLATFORM_RESET
    HAL_PLATFORM_RESET();
#else
#warning "Reset selected for test case exit, but none defined"
#endif
#endif
    // Default behaviour - simply hang in a loop
    for(;;)
        ;
}
// EOF tcdiag.cxx
