//=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
//
//=============================================================================
//####UNSUPPORTEDBEGIN####
//
// -------------------------------------------
// This source file has been contributed to eCos/Red Hat. It may have been
// changed slightly to provide an interface consistent with those of other 
// files.
//
// The functionality and contents of this file is supplied "AS IS"
// without any form of support and will not necessarily be kept up
// to date by Red Hat.
//
// All inquiries about this file, or the functionality provided by it,
// should be directed to the 'ecos-discuss' mailing list (see
// http://sourceware.cygnus.com/ecos/intouch.html for details).
//
// Contributed by: Kevin Hester <khester@opticworks.com>
// Maintained by:  <Unmaintained>
// -------------------------------------------
//
//####UNSUPPORTEDEND####
//=============================================================================
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
// Author(s):   nickg, jskov
// Contributors:nickg
// Date:        1998-03-02
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt macros

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#include <cyg/hal/hal_stub.h>           // hal_output_gdb_string
#endif

#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/quicc_smc2.h>

//-----------------------------------------------------------------------------
// Select default diag channel to use

//#define CYG_KERNEL_DIAG_ROMART
//#define CYG_KERNEL_DIAG_SERIAL

#if !defined(CYG_KERNEL_DIAG_SERIAL)
#define CYG_KERNEL_DIAG_SERIAL
#endif

//-----------------------------------------------------------------------------
// Fads board specific serial

#if defined(CYG_KERNEL_DIAG_SERIAL)

void hal_diag_init(void)
{
#if !defined(CYGDBG_KERNEL_DEBUG_GDB_INCLUDE_STUBS)
    // init the actual serial port
    cyg_smc2_init(9600);
#endif
}

void hal_diag_write_char(char c)
{
#if !defined(CYGDBG_KERNEL_DEBUG_GDB_INCLUDE_STUBS)
    unsigned long __state;
    HAL_DISABLE_INTERRUPTS(__state);
    cyg_smc2_putchar(c);
    HAL_RESTORE_INTERRUPTS(__state);
#else
    hal_output_gdb_string (&c, 1);
#endif
}

void hal_diag_read_char(char *c)
{
    *c = cyg_smc2_getchar();
}
#endif

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG
