#ifndef CYGONCE_VAR_INTR_H
#define CYGONCE_VAR_INTR_H
//=============================================================================
//
//      var_intr.h
//
//      Variant HAL interrupt and clock support
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Author(s):   nickg
// Contributors:nickg, jskov, jlarmour, hmt, gthomas
// Date:        2000-04-02
// Purpose:     Variant interrupt support
// Description: The macros defined here provide the HAL APIs for handling
//              interrupts and the clock on the PPC60x variant CPUs.
// Usage:       Is included via the architecture interrupt header:
//              #include <cyg/hal/hal_intr.h>
//              ...
//
//####DESCRIPTIONEND####
//
//=============================================================================

// Additional trap/exceptions on PPC60x
#define CYGNUM_HAL_VECTOR_ITLB_MISS        0x10
#define CYGNUM_HAL_VECTOR_DTLB_LOAD_MISS   0x11
#define CYGNUM_HAL_VECTOR_DTLB_STORE_MISS  0x12
#define CYGNUM_HAL_VECTOR_SMI              0x13

#define CYGNUM_HAL_VSR_MAX                 CYGNUM_HAL_VECTOR_SMI

// No builtin interrupt controller in the PPC60x CPUs.

#include <cyg/hal/plf_intr.h>  // Maybe something on the platform though

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_VAR_INTR_H
// End of var_intr.h
