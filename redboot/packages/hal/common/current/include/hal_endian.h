#ifndef CYGONCE_HAL_HAL_ENDIAN_H
#define CYGONCE_HAL_HAL_ENDIAN_H

//=============================================================================
//
//      hal_endian.h
//
//      Endian conversion macros
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
// Author(s):   jskov
// Contributors:jskov
// Date:        2001-10-04
// Purpose:     Endian conversion macros
// Usage:       #include <cyg/hal/hal_endian.h>
//                           
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>                // CYGBLD_HAL_ENDIAN_H
#include <cyg/infra/cyg_type.h>         // endian setting

// Allow HALs to override the generic implementation of swap macros
#ifdef CYGBLD_HAL_ENDIAN_H
# include CYGBLD_HAL_ENDIAN_H
#endif

#ifndef CYG_SWAP16
# define CYG_SWAP16(_x_)                                        \
    ({ cyg_uint16 _x = (_x_); ((_x << 8) | (_x >> 8)); })
#endif

#ifndef CYG_SWAP32
# define CYG_SWAP32(_x_)                        \
    ({ cyg_uint32 _x = (_x_);                   \
       ((_x << 24) |                            \
       ((0x0000FF00UL & _x) <<  8) |            \
       ((0x00FF0000UL & _x) >>  8) |            \
       (_x  >> 24)); })
#endif


#if (CYG_BYTEORDER == CYG_LSBFIRST)
# define CYG_CPU_TO_BE16(_x_) CYG_SWAP16((_x_))
# define CYG_CPU_TO_BE32(_x_) CYG_SWAP32((_x_))
# define CYG_BE16_TO_CPU(_x_) CYG_SWAP16((_x_))
# define CYG_BE32_TO_CPU(_x_) CYG_SWAP32((_x_))

# define CYG_CPU_TO_LE16(_x_) (_x_)
# define CYG_CPU_TO_LE32(_x_) (_x_)
# define CYG_LE16_TO_CPU(_x_) (_x_)
# define CYG_LE32_TO_CPU(_x_) (_x_)

#elif (CYG_BYTEORDER == CYG_MSBFIRST)

# define CYG_CPU_TO_BE16(_x_) (_x_)
# define CYG_CPU_TO_BE32(_x_) (_x_)
# define CYG_BE16_TO_CPU(_x_) (_x_)
# define CYG_BE32_TO_CPU(_x_) (_x_)

# define CYG_CPU_TO_LE16(_x_) CYG_SWAP16((_x_))
# define CYG_CPU_TO_LE32(_x_) CYG_SWAP32((_x_))
# define CYG_LE16_TO_CPU(_x_) CYG_SWAP16((_x_))
# define CYG_LE32_TO_CPU(_x_) CYG_SWAP32((_x_))

#else

# error "Endian mode not selected"

#endif

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_ENDIAN_H
// End of hal_endian.h
