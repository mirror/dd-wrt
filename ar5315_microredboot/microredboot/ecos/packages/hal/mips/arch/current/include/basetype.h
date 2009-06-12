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
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:  nickg
// Date:        1998-02-05
// Purpose:     Define architecture base types.
// Usage:       Included by <cyg/infra/cyg_types.h>, do not use directly
//              
//####DESCRIPTIONEND####
//

#include <pkgconf/hal.h>

//-----------------------------------------------------------------------------
// Characterize the architecture

#if defined(CYGPKG_HAL_MIPS_MSBFIRST)
# define CYG_BYTEORDER           CYG_MSBFIRST    // Big endian
#elif defined(CYGPKG_HAL_MIPS_LSBFIRST)
# define CYG_BYTEORDER           CYG_LSBFIRST    // Little endian
#else
# error MIPS endianess not defined by configuration
#endif

#if defined(CYGPKG_HAL_MIPS_DOUBLE_MSBFIRST)
# define CYG_DOUBLE_BYTEORDER    CYG_MSBFIRST    // Big endian
#elif defined(CYGPKG_HAL_MIPS_DOUBLE_LSBFIRST)
# define CYG_DOUBLE_BYTEORDER    CYG_LSBFIRST    // Little endian
#else
# define CYG_DOUBLE_BYTEORDER CYG_BYTEORDER      // default to CPU endianess
#endif

//-----------------------------------------------------------------------------
// MIPS does not usually use labels with undersores.

//#define CYG_LABEL_NAME(_name_) _name_

//-----------------------------------------------------------------------------
// Define the standard variable sizes

// The MIPS architecture uses the default definitions of the base types,
// so we do not need to define any here.

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_BASETYPE_H
// End of basetype.h
