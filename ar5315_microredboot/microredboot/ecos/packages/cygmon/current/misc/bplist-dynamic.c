//==========================================================================
//
//      bplist-dynamic.c
//
//      Breakpoint list using dynamic memory.
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
// Purpose:      Breakpoint list using dynamic memory.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include "board.h"

#ifndef USE_ECOS_HAL_BREAKPOINTS

#include <stdlib.h>

#ifndef NO_MALLOC
#ifndef NO_MALLOC_H
#include "malloc.h"
#else
void free ();
char *malloc ();
#endif
#endif

#ifdef __ECOS__
#include <cyg/hal/plf_stub.h>
#endif /* __ECOS__ */

/*
 * A simple target breakpoint list using malloc.
 * To use this package, you must define TRAP_SIZE to be the size
 * in bytes of a trap instruction (max if there's more than one),
 * and export a char array called _breakinst that contains a
 * breakpoint trap.  This package will copy trap instructions
 * from _breakinst into the breakpoint locations.
 */

static struct breakpoint_list {
  target_register_t  addr;
  char old_contents [TRAP_SIZE];
  struct breakpoint_list *next;
  char in_memory;
} *breakpoint_list = NULL;

#ifdef NO_MALLOC
static struct breakpoint_list bp_list [MAX_BP_NUM];
static struct breakpoint_list *free_bp_list = NULL;
static int curr_bp_num = 0;
#endif

#ifndef BREAKINST_DEFINED
#define BREAKINST_DEFINED
extern unsigned char _breakinst[];
#endif

int
__set_breakpoint (target_register_t addr)
{
  struct breakpoint_list **addent = &breakpoint_list;
  struct breakpoint_list *l = breakpoint_list;
  struct breakpoint_list *newent;

  while (l != NULL && l->addr < addr)
    {
      addent = &l->next;
      l =  l->next;
    }

  if (l != NULL && l->addr == addr)
    return 2;

#ifdef NO_MALLOC
  if (free_bp_list != NULL)
    {
      newent = free_bp_list;
      free_bp_list = free_bp_list->next;
    }
  else
    {
      if (curr_bp_num < MAX_BP_NUM)
	{
	  newent = &bp_list[curr_bp_num++];
	}
      else
	{
	  return 1;
	}
    }
#else
  newent = (struct breakpoint_list *) malloc (sizeof (struct breakpoint_list));
#endif
  newent->addr = addr;
  newent->in_memory = 0;
  newent->next = l;
  *addent = newent;
  return 0;
}

int
__remove_breakpoint (target_register_t addr)
{
  struct breakpoint_list *l = breakpoint_list;
  struct breakpoint_list *prev = NULL;

  while (l != NULL && l->addr < addr)
    {
      prev = l;
      l = l->next;
    }

  if (l == NULL)
    return 1;

  if (l->in_memory)
    {
      __write_mem_safe (&l->old_contents[0],
			(void*)l->addr,
			sizeof (l->old_contents));
    }

  if (prev == NULL)
    breakpoint_list = l->next;
  else
    prev->next = l->next;

#ifdef NO_MALLOC
  l->next = free_bp_list;
  free_bp_list = l;
#else
  free (l);
#endif
  return 0;
}

#include <cyg/hal/generic-stub.h>
#include <cyg/hal/hal_stub.h>
void
__cygmon_install_breakpoints (void)
{
  struct breakpoint_list *l = breakpoint_list;

  while (l != NULL)
    {
      if (! l->in_memory)
	{
	  int len = sizeof (l->old_contents);

	  if (__read_mem_safe (&l->old_contents[0], (void*)l->addr, len) == len)
	    {
#ifdef WRITE_MEM_IS_MEMCPY
	      if (__write_mem_safe (_breakinst, (void*)l->addr, len) == (void*)l->addr)
#else
	      if (__write_mem_safe (_breakinst, (void*)l->addr, len) == len)
#endif
		{
		  l->in_memory = 1;
		}
	    }
	}
      l = l->next;
    }
  flush_i_cache ();
}

void
__cygmon_clear_breakpoints (void)
{
  struct breakpoint_list *l = breakpoint_list;

  while (l != NULL)
    {
      if (l->in_memory)
	{
	  int len = sizeof (l->old_contents);

#ifdef WRITE_MEM_IS_MEMCPY
      if (__write_mem_safe (_breakinst, (void*)l->addr, len) == (void*)l->addr)
#else
	  if (__write_mem_safe (&l->old_contents[0], (void*)l->addr, len) == len)
#endif
	    {
	      l->in_memory = 0;
	    }
	}
      l = l->next;
    }
  flush_i_cache ();
}

#endif // USE_ECOS_HAL_BREAKPOINTS
