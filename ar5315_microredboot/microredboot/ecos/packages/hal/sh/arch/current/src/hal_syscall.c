//=============================================================================
//
//      hal_syscall.c
//
//      
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett 
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   msalter
// Contributors:msalter, jskov, nickg
// Date:        2002-02-28
// Purpose:     
// Description: 
//              
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_stub.h>           // Our header
#include <cyg/hal/hal_arch.h>           // HAL_BREAKINST
#include <cyg/hal/hal_cache.h>          // HAL_xCACHE_x
#include <cyg/hal/hal_intr.h>           // interrupt disable/restore

#include <cyg/hal/hal_if.h>             // ROM calling interface
#include <cyg/hal/hal_misc.h>           // Helper functions

#include <redboot.h>

extern int __do_syscall(int func,		// syscall function number
			long arg1, long arg2,	// up to four args.
			long arg3, long arg4,
			int *retval,		// syscall return value
			int *sig);              // signal to return (or 0)

#define _shnewlib_SYS_exit        1
#define _shnewlib_SYS_fork        2

#define _shnewlib_SYS_read        3
#define _shnewlib_SYS_write       4
#define _shnewlib_SYS_open        5
#define _shnewlib_SYS_close       6
#define _shnewlib_SYS_wait4       7
#define _shnewlib_SYS_creat       8
#define _shnewlib_SYS_link        9
#define _shnewlib_SYS_unlink      10
#define _shnewlib_SYS_execv       11
#define _shnewlib_SYS_chdir       12
#define _shnewlib_SYS_mknod       14
#define _shnewlib_SYS_chmod       15
#define _shnewlib_SYS_chown       16
#define _shnewlib_SYS_lseek       19
#define _shnewlib_SYS_getpid      20
#define _shnewlib_SYS_isatty      21
#define _shnewlib_SYS_fstat       22
#define _shnewlib_SYS_time        23

#define _shnewlib_SYS_ARG         24
#define _shnewlib_SYS_stat        38

#define _shnewlib_SYS_pipe        42
#define _shnewlib_SYS_execve      59

#define _shnewlib_SYS_argc        172 /* == 0xAC, for Argument Count :-) */
#define _shnewlib_SYS_argnlen     173
#define _shnewlib_SYS_argn        174

#define _shnewlib_SYS_utime       201 /* not really a system call */
#define _shnewlib_SYS_wait        202 /* nor is this */

int
hal_syscall_handler(void)
{
    int func, arg1, arg2, arg3, arg4;
    int err, sig;

    func = get_register(R4);
    arg1 = get_register(R5);
    arg2 = get_register(R6);
    arg3 = get_register(R7);
    arg4 = *(unsigned int *)(get_register(SP));

    switch (func) {
    case _shnewlib_SYS_exit:
        func = SYS_exit;
        break;
    case _shnewlib_SYS_read:
        func = SYS_read;
        break;
    case _shnewlib_SYS_write:
        func = SYS_write;
        break;
    case _shnewlib_SYS_open:
        func = SYS_open;
        break;
    case _shnewlib_SYS_close:
        func = SYS_close;
        break;
    case _shnewlib_SYS_lseek:
        func = SYS_lseek;
        break;
    case _shnewlib_SYS_unlink:
        func = SYS_unlink;
        break;
    case _shnewlib_SYS_getpid:
        func = SYS_getpid;
        break;
    case _shnewlib_SYS_fstat:
        func = SYS_fstat;
        break;
    case _shnewlib_SYS_chdir:
        func = SYS_chdir;
        break;
    case _shnewlib_SYS_stat:
        func = SYS_stat;
        break;
    case _shnewlib_SYS_chmod:
        func = SYS_chmod;
        break;
    case _shnewlib_SYS_time:
        func = SYS_time;
        break;
    case _shnewlib_SYS_isatty:
        func = SYS_isatty;
        break;
    case _shnewlib_SYS_utime:
        func = SYS_utime;
    case _shnewlib_SYS_argc:
        put_register( R0, 0 );
        put_register( R1, 0 );
        return 0;
    default:
        return SIGTRAP;
    }

    if (func == SYS_interrupt) {
	//  A console interrupt landed us here.
	//  Invoke the debug agent so as to cause a SIGINT.
        return SIGINT;
    }

    if (__do_syscall(func, arg1, arg2, arg3, arg4, &err, &sig)) {
        // R0 is normally result register, but newlib's trap
        // code looks in R1 for the return value
        put_register(R1, err);
	return sig;
    }

    return SIGTRAP;
}
