//==========================================================================
//
//      generic_fmt32.c
//
//      Generic address conversion routines
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
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      Generic address conversion routines
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <string.h>
#ifdef HAVE_BSP
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#endif
#include "monitor.h"
#include "fmt_util.h"


int
str2addr (char *string, mem_addr_t *res)
{
#ifdef HAVE_ASI
  if (string[0] == '[')
    {
      char *ptr = ++string;
      while (*ptr && *ptr != ']')
	ptr++;

      if (*ptr == 0)
	return -1;

      *(ptr++) = 0;
      res->asi = str2int (string, 16);
      string = ptr;
    }
  else
    res->asi = ASI_DEFAULT;
#endif
  res->addr = str2int (string, 16);
  return 0;
}

void
addr2str (mem_addr_t *addr, char *dest)
{
#ifdef HAVE_ASI
  if (addr->asi != ASI_DEFAULT)
    xsprintf(dest, "[%x]", addr->asi);
  else
#endif
    dest[0] = 0;
  strcat (dest, int2str (addr->addr, 16, sizeof (void *) * 2));
}
