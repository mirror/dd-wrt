//==========================================================================
//
//      breakpoints.c
//
//      Support aribtrary set of breakpoints.
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
// Purpose:      
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include "board.h"

#ifndef USE_ECOS_HAL_BREAKPOINTS

#include <stdlib.h>
#ifdef HAVE_BSP
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#endif

#include "monitor.h"
#include "tservice.h"
#include "stub-tservice.h"

#include "fmt_util.h"


static struct bp *last_bp_ptr;
static struct bp *first_bp_ptr;

#ifdef NO_MALLOC
static struct bp *free_bp_list;
static struct bp bp_list[MAX_NUM_BP];
static int       curr_bp_num;
#endif

int
add_mon_breakpoint (mem_addr_t location)
{
  struct bp *ptr;
  struct bp *new_bp_ptr;

  for (ptr = first_bp_ptr; ptr != NULL; ptr = ptr->next)
    {
      if (MEM_ADDR_EQ_P (ptr->address, location))
	return 1;
    }
#ifdef NO_MALLOC
  if (free_bp_list != NULL)
    {
      new_bp_ptr = free_bp_list;
      free_bp_list = new_bp_ptr->next;
    }
  else
    {
      if (curr_bp_num < MAX_NUM_BP)
	{
	  new_bp_ptr = &bp_list[curr_bp_num++];
	}
      else
	{
	  xprintf ("No more breakpoints\n");
	  return 1;
	}
    }
#else
  new_bp_ptr = (struct bp *)malloc (sizeof (struct bp));
#endif

  if (first_bp_ptr == NULL)
    {
      first_bp_ptr = new_bp_ptr;
    }
  else
    {
      last_bp_ptr->next = new_bp_ptr;
    }
  last_bp_ptr = new_bp_ptr;

  last_bp_ptr->next = NULL;
  last_bp_ptr->address = location;
  last_bp_ptr->in_memory = 0;
  return 0;
}


void
install_breakpoints (void)
{
  struct bp *ptr = first_bp_ptr;
  while (ptr != NULL)
    {
      set_breakpoint (ptr);
      ptr = ptr->next;
    }
}


void
clear_breakpoints (void)
{
  struct bp *ptr = first_bp_ptr;

  while (ptr != NULL)
    {
      clear_breakpoint (ptr);
      ptr = ptr->next;
    }
}

int
show_breakpoints (void)
{
  struct bp *ptr;

  for (ptr = first_bp_ptr; ptr != NULL; ptr = ptr->next)
    {
      char buf[20];

      addr2str (&ptr->address, buf);
      xprintf ("%s\n", buf);
    }
  
  return 0;
}



int 
clear_mon_breakpoint (mem_addr_t location)
{
  int error = 0;
  struct bp *ptr = first_bp_ptr;
  struct bp *prev_ptr = NULL;

  /* Scan the list looking for the address to clear */
  while (ptr != NULL && !MEM_ADDR_EQ_P (ptr->address, location))
    {
      /* keep a pointer one behind the current position */
      prev_ptr = ptr;
      ptr = ptr->next;
    }
  if (ptr == NULL)
    {
      xprintf ("That address has no breakpoint on it.\n");
      error = 1;
    }
  else
    {
      /* Just in case it's still in memory. */
      clear_breakpoint (ptr);

      /* now we'll point the previous bp->next at the one after the one 
	 we're deleting, unless there is no previous bp. */
      if (prev_ptr != NULL)
	{
	  prev_ptr->next = ptr->next;
	}

      if (first_bp_ptr == ptr)
	first_bp_ptr = ptr->next;

      if (last_bp_ptr == ptr)
	last_bp_ptr = prev_ptr;

      /* eliminate the offending bp struct */
#ifdef NO_MALLOC
      ptr->next = free_bp_list;
      free_bp_list = ptr;
#else
      free (ptr);
#endif
    }
  return error;
}	

#endif /* USE_ECOS_HAL_BREAKPOINTS */
