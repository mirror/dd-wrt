//==========================================================================
//
//      wallclock_hs7729pci.inl
//
//      HS7729PCI wallclock details
//
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jskov
// Contributors:  jskov
// Date:          2001-07-06
// Purpose:       Wallclock driver details for HS7729PCI
//
//####DESCRIPTIONEND####
//
//==========================================================================

#define nDS_LINEAR
#define DS_ADDR    0xa80000e0
#define DS_DATA    0xa80000e2

#define DS_READ_UINT8(_a_, _d_)               \
    CYG_MACRO_START                           \
    cyg_uint16 t;                             \
    HAL_READ_UINT16((_a_), t);                \
    (_d_) = (t >> 8) & 0xff;                  \
    CYG_MACRO_END

#define DS_WRITE_UINT8(_a_, _d_)              \
    CYG_MACRO_START                           \
    HAL_WRITE_UINT16((_a_), (_d_)<<8);        \
    CYG_MACRO_END


//-----------------------------------------------------------------------------
// End of wallclock_hs7729pci.inl
