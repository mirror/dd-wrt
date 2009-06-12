//==========================================================================
//
//      generic_mem.c
//
//      Routines for reading and writing memory.
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
// Author(s):    
// Contributors: gthomas, dmoseley
// Date:         1999-10-20
// Purpose:      
// Description:  It may be appropriate to relay directly into the stubs
//               implementation of read memory, without board specific
//               considerations such as checking the allowed ranges of
//               addresses. Perhaps there needs to be some consideration
//               of address spaces or, masking addresses.
//
//               This file implements a default version of reading and writing
//               memory when none of this is an issue
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#ifdef HAVE_BSP
#include <bsp/bsp.h>
#include "cpu_info.h"
#endif
#include "board.h"
#include "monitor.h"
#include "tservice.h"

#ifndef HAVE_BSP
#include "generic-stub.h"
#endif


int
read_memory (mem_addr_t *src, int size, int amt, char *dst)
{
#if defined(HAVE_BSP) && !defined(USE_ECOS_HAL_SAFE_MEMORY)
    return (bsp_memory_read((unsigned char *)src->addr, MEM_ADDR_ASI(src),
			    size << 3, amt, dst) != amt);
#else
  int totamt = size * amt;
  return (totamt != __read_mem_safe (dst, (void*)src->addr, totamt));
#endif
}

int
write_memory (mem_addr_t *dst, int size, int amt, char *src)
{
#if defined(HAVE_BSP) && !defined(USE_ECOS_HAL_SAFE_MEMORY)
    return (bsp_memory_write((unsigned char *)dst->addr, MEM_ADDR_ASI(dst),
			     size << 3, amt, src) != amt);
#else
  int totamt = size * amt;
  return (totamt != __write_mem_safe (src, (void*)dst->addr, totamt));
#endif
}


   
