#ifndef CYGONCE_HAL_HAL_ARCH_H
#define CYGONCE_HAL_HAL_ARCH_H

//=============================================================================
//
//      hal_arch.h
//
//      Architecture specific abstractions for synthetic target.
//
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
// Author(s):   proven
// Contributors:proven, pjo, nickg,bartv
// Date:        1998-10-05
// Purpose:     Define architecture abstractions
// Usage:       #include <cyg/hal/hal_arch.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

// ----------------------------------------------------------------------------
// Architectural definitions such as processor save state information.
// For the synthetic target most of this information has to be
// provided by the variant HALs.

#include <cyg/hal/var_arch.h>

//-----------------------------------------------------------------------------
// Exception handling function.
// This function is provided by either the kernel or the common HAL to
// deliver an exception to the current thread.
// NOTE: the declaration should move to a common package. 
externC void cyg_hal_deliver_exception( CYG_WORD code, CYG_ADDRWORD data );

//-----------------------------------------------------------------------------
// Execution reorder barrier.
// When optimizing the compiler can reorder code. In multithreaded systems
// where the order of actions is vital, this can sometimes cause problems.
// This macro may be inserted into places where reordering should not happen.

#define HAL_REORDER_BARRIER() asm volatile ( "" : : : "memory" )

//-----------------------------------------------------------------------------
// Idle thread code.
// This macro is called in the idle thread loop, and gives the HAL the
// chance to insert code. In the synthetic target it involves blocking
// until there is a signal.

externC void hal_idle_thread_action(cyg_uint32 loop_count);

#define HAL_IDLE_THREAD_ACTION(_count_) hal_idle_thread_action(_count_)

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_ARCH_H
// End of hal_arch.h
