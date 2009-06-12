//===========================================================================
//
//      longjmp.cxx
//
//      ISO C standard longjmp() function
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-04-30
// Purpose:     
// Description:  Implements ISO standard non-local jump function longjmp()
//               per ISO C para 7.6.2.1. This is the "real" alternative to
//               the inline version of longjmp()
//             
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_setjmp.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Common tracing code
#include <cyg/infra/cyg_ass.h>     // Common assertion code
#include <cyg/hal/hal_arch.h>      // HAL architecture specific implementation

// We don't want the inline version of longjmp() defined here
#ifdef CYGIMP_LIBC_SETJMP_INLINES
# undef CYGIMP_LIBC_SETJMP_INLINES
#endif

#include <setjmp.h>                // Header for setjmp/longjmp


// FUNCTIONS

void
longjmp( jmp_buf cyg_buf, int cyg_val)
{
    CYG_REPORT_FUNCNAME( "longjmp" );
    CYG_REPORT_FUNCARG2( "&cyg_buf=%08x, cyg_val=%d", &cyg_buf, cyg_val );

    // ANSI says that if we are passed cyg_val==0, then we change it to 1
    if (cyg_val == 0)
        ++cyg_val;

    // we let the HAL do the work

    HAL_REORDER_BARRIER(); // prevent any chance of optimisation re-ordering
    hal_longjmp( cyg_buf, cyg_val );
    HAL_REORDER_BARRIER(); // prevent any chance of optimisation re-ordering

#ifdef CYGDBG_USE_ASSERTS
    CYG_ASSERT( 0, "longjmp should not have reached this point!" );
#else
    for (;;)
        CYG_EMPTY_STATEMENT;
#endif
} // longjmp()

// EOF longjmp.cxx
