//=============================================================================
//
//      plf_stub.c
//
//      Platform specific code for GDB stub support.
//
//=============================================================================

// - pjo, 28 sep 1999
// - Copied ARM version for use with i386/pc.

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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   gthomas (based on the old ARM/AEB hal_stub.c)
// Contributors:gthomas, jskov, pjo, nickg
// Date:        1999-02-15
// Purpose:     Platform specific code for GDB stub support.
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

#include <cyg/hal/hal_stub.h>

#include <cyg/hal/hal_io.h>             // HAL IO macros
#include <cyg/hal/hal_intr.h>           // HAL interrupt macros

#include <cyg/hal/plf_misc.h>

//-----------------------------------------------------------------------------
// Connect our VSR to the exception vectors.

externC void __default_exception_vsr(void);

#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
externC void __syscall_tramp(void);
externC char idtStart[];
#endif // CYGSEM_REDBOOT_BSP_SYSCALLS


void hal_pc_stubs_init(void)
{
#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
    cyg_hal_pc_set_idt_entry((CYG_ADDRESS)__syscall_tramp, (short *)(idtStart + (0x80 * 8)));
#endif // CYGSEM_REDBOOT_BSP_SYSCALLS
}


//-----------------------------------------------------------------------------

#endif // ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
// End of plf_stub.c
