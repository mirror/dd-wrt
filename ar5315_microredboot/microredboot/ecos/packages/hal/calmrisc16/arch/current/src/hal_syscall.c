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
// Contributors:msalter
// Date:        2000-11-5
// Purpose:     
// Description: 
//              
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)

#include <cyg/hal/hal_stub.h>           // Our header
#include <cyg/hal/hal_arch.h>           // HAL_BREAKINST
#include <cyg/hal/hal_cache.h>          // HAL_xCACHE_x
#include <cyg/hal/hal_intr.h>           // interrupt disable/restore

#include <cyg/hal/hal_if.h>             // ROM calling interface
#include <cyg/hal/hal_misc.h>           // Helper functions

extern CYG_ADDRWORD __do_syscall(CYG_ADDRWORD func,		// syscall function number
			CYG_ADDRWORD arg1, CYG_ADDRWORD arg2,	// up to four args.
			CYG_ADDRWORD arg3, CYG_ADDRWORD arg4,
			CYG_ADDRWORD *retval,		// syscall return value
			CYG_ADDRWORD *sig);             // signal to return (or 0)

// These are required by the ANSI C part of newlib (excluding system() of
// course).
#define	SYS_exit	1
#define	SYS_open	2
#define	SYS_close	3
#define	SYS_read	4
#define	SYS_write	5
#define	SYS_lseek	6
#define	SYS_unlink	7
#define	SYS_getpid	8
#define	SYS_kill	9
#define SYS_fstat       10
//#define SYS_sbrk	11 - not currently a system call, but reserved.

// ARGV support.
#define SYS_argvlen	12
#define SYS_argv	13

// These are extras added for one reason or another.
#define SYS_chdir	14
#define SYS_stat	15
#define SYS_chmod 	16
#define SYS_utime 	17
#define SYS_time 	18

#define SYS_interrupt   1000
#define SYS_meminfo     1001

int
hal_syscall_handler(void)
{
    int func, lret = 0;
    unsigned short *sp = (unsigned short *)get_register(REG_SP);
    CYG_ADDRWORD arg1, arg2, arg3, arg4;
    long err, sig;

    // Shift this right because GDB wants 16-bit register values
    // to be left-justified in a 32-bit field. Yuck.
    func = get_register(REG_R0) >> 16;
    arg1 = get_register(REG_A8);
    arg2 = get_register(REG_A9);
    arg3 = get_register(REG_A10);
    arg4 = get_register(REG_A11);

    if (func == SYS_interrupt) {
	//  A console interrupt landed us here.
	//  Invoke the debug agent so as to cause a SIGINT.
        return SIGINT;
    }

    if (__do_syscall(func, arg1, arg2, arg3, arg4, &err, &sig)) {
	put_register(REG_R2, ((err < 0) ? -err : 0) << 16);
	put_register(REG_A8, err);
	return (int)sig;
    }

    return SIGTRAP;
}

#endif // CYGSEM_REDBOOT_BSP_SYSCALLS
