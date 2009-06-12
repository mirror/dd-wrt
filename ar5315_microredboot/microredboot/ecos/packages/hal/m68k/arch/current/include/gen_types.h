#ifndef _GEN_TYPES_H
#define _GEN_TYPES_H
//==========================================================================
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
//==========================================================================

/*****************************************************************************

                              *********************
                              *                   *
                              *   General Types   *
                              *                   *
                              *********************

     Define some simple variable sizes for this platform.

     Note that these types are useful  for making user threads that  are
not eCos-specific.

*****************************************************************************/

//      Include some architecture variant specific types.

#include <cyg/hal/var_gen_types.h>

//      Define types to use for specific sized variables.  Note that the above
// include file could have defined  some of these  types differently.  If  the
// type is already defined, always use the previous definition.

#ifndef _GEN_TYPES_I8_T
#define _GEN_TYPES_I8_T char
#endif
#ifndef _GEN_TYPES_I16_T
#define _GEN_TYPES_I16_T short
#endif
#ifndef _GEN_TYPES_I32_T
#define _GEN_TYPES_I32_T int
#endif
#ifndef _GEN_TYPES_I64_T
#define _GEN_TYPES_I64_T long long
#endif
#ifndef _GEN_TYPES_S8_T
#define _GEN_TYPES_S8_T signed _GEN_TYPES_I8_T
#endif
#ifndef _GEN_TYPES_S16_T
#define _GEN_TYPES_S16_T signed _GEN_TYPES_I16_T
#endif
#ifndef _GEN_TYPES_S32_T
#define _GEN_TYPES_S32_T signed _GEN_TYPES_I32_T
#endif
#ifndef _GEN_TYPES_S64_T
#define _GEN_TYPES_S64_T signed _GEN_TYPES_I64_T
#endif
#ifndef _GEN_TYPES_U8_T
#define _GEN_TYPES_U8_T unsigned _GEN_TYPES_I8_T
#endif
#ifndef _GEN_TYPES_U16_T
#define _GEN_TYPES_U16_T unsigned _GEN_TYPES_I16_T
#endif
#ifndef _GEN_TYPES_U32_T
#define _GEN_TYPES_U32_T unsigned _GEN_TYPES_I32_T
#endif
#ifndef _GEN_TYPES_U64_T
#define _GEN_TYPES_U64_T unsigned _GEN_TYPES_I64_T
#endif
#ifndef _GEN_TYPES_ADDR_T
#define _GEN_TYPES_ADDR_T void *
#endif

typedef _GEN_TYPES_I8_T   i8_t;
typedef _GEN_TYPES_I16_T  i16_t;
typedef _GEN_TYPES_I32_T  i32_t;
typedef _GEN_TYPES_I64_T  i64_t;
typedef _GEN_TYPES_S8_T   s8_t;
typedef _GEN_TYPES_S16_T  s16_t;
typedef _GEN_TYPES_S32_T  s32_t;
typedef _GEN_TYPES_S64_T  s64_t;
typedef _GEN_TYPES_U8_T   u8_t;
typedef _GEN_TYPES_U16_T  u16_t;
typedef _GEN_TYPES_U32_T  u32_t;
typedef _GEN_TYPES_U64_T  u64_t;
typedef _GEN_TYPES_ADDR_T addr_t;

//      Use int_t and uint_t for  fast counter variables.   The size of  these
// types depends on the architecture but it  will always be at least 16  bits.
// For example, 16-bit numbers are faster on the 68000 but 32-bit numbers  are
// faster on the mcf5200.  If you need to assume the size of the variable  use
// u16_t or u32_t instead.

#ifndef _GEN_TYPES_INT_T
#define _GEN_TYPES_INT_T i32_t
#endif
#ifndef _GEN_TYPES_SINT_T
#define _GEN_TYPES_SINT_T s32_t
#endif
#ifndef _GEN_TYPES_UINT_T
#define _GEN_TYPES_UINT_T u32_t
#endif

typedef _GEN_TYPES_INT_T  int_t;
typedef _GEN_TYPES_SINT_T sint_t;
typedef _GEN_TYPES_UINT_T uint_t;

#endif // _GEN_TYPES_H

