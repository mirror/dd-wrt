/*========================================================================
//
//      heapgen.cpp
//
//      Helper file for heapgen.tcl
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-06-13
// Purpose:       Helper file for heapgen.tcl
// Description:   Exports macros derived from the configuration so that
//                they are visible to heapgen.tcl. This file is
//                preprocessed by a make rule in the CDL to generate
//                "heapgeninc.tcl"
//                Note, this isn't a real C file. It is only to be
//                preprocessed, not compiled
// Usage:         
//
//####DESCRIPTIONEND####
//
//======================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/memalloc.h>

#define STRINGIFY1(_x_) #_x_
#define STRINGIFY(_x_) STRINGIFY1(_x_)

set memlayout_h   STRINGIFY(CYGHWR_MEMORY_LAYOUT_H)
set memlayout_ldi STRINGIFY(CYGHWR_MEMORY_LAYOUT_LDI)
set malloc_impl_h   STRINGIFY(CYGBLD_MEMALLOC_MALLOC_IMPLEMENTATION_HEADER)
#define __MALLOC_IMPL_WANTED
#include CYGBLD_MEMALLOC_MALLOC_IMPLEMENTATION_HEADER
set malloc_impl_class   STRINGIFY(CYGCLS_MEMALLOC_MALLOC_IMPL)

/* EOF heapgen.cpp */
