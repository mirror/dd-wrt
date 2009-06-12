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

#define	SYS_exit	1
#define SYS_interrupt   1000

int
hal_syscall_handler(void)
{
    CYG_ADDRWORD func, arg1, arg2, arg3, arg4;
    CYG_ADDRWORD err, sig;

    func = get_register(D0);
    arg1 = get_register(D1);
    arg2 = get_register(D2);
    arg3 = get_register(D3);
    arg4 = 0;
 
    if (func == SYS_interrupt) {
	//  A console interrupt landed us here.
	//  Invoke the debug agent so as to cause a SIGINT.
        return SIGINT;
    }

    if (__do_syscall(func, arg1, arg2, arg3, arg4, &err, &sig)) {
        put_register(D0, err);
	return (int)sig;
    }

    return SIGTRAP;
}

#endif // CYGSEM_REDBOOT_BSP_SYSCALLS
