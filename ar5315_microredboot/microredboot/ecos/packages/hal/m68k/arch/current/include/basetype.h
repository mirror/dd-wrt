#ifndef CYGONCE_HAL_BASETYPE_H
#define CYGONCE_HAL_BASETYPE_H

//=============================================================================
//
//      basetype.h
//
//      Standard types for this architecture.
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

// Include variant specific types.
#include <cyg/hal/var_basetype.h>

// Include the standard variable sizes.
#include <cyg/hal/gen_types.h>

//-----------------------------------------------------------------------------
// Characterize the architecture

#define CYG_BYTEORDER   CYG_MSBFIRST    // Big endian

//-----------------------------------------------------------------------------
// 68k does not usually use labels with underscores. Some labels generated
// by the linker do, so add an underscore where required.

#define CYG_LABEL_NAME(_name_) _##_name_

// The 68k architecture uses the default definitions of the base types,
// so we do not need to define any here.

//-----------------------------------------------------------------------------
// Override the alignment definitions from cyg_type.h.
// Most 68k variants use a 4 byte alignment.
// A few variants (68000) may override this setting to use a 2 byte alignment.
#ifndef CYGARC_ALIGNMENT
#define CYGARC_ALIGNMENT 4
#endif
// the corresponding power of two alignment
#ifndef CYGARC_P2ALIGNMENT
#define CYGARC_P2ALIGNMENT 2
#endif


//-----------------------------------------------------------------------------
// Define a compiler-specific rune for saying a function doesn't return

//      WARNING: There is a bug in some versions of gcc for the m68k that does
// not handle the noreturn attribute  correctly.  It is  safe to just  disable
// this attribute.

#ifndef CYGBLD_ATTRIB_NORET
# define CYGBLD_ATTRIB_NORET
#endif

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_BASETYPE_H

