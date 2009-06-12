//==========================================================================
//
//      bplist-dynamic.c
//
//      Dynamic breakpoint list.
//      Currently only statically allocated.  (ie NO_MALLOC is assumed)
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
// Contributors: dmoseley
// Date:         2000-07-11
// Purpose:      Dynamic breakpoint list.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#if defined(CYGNUM_HAL_BREAKPOINT_LIST_SIZE) && (CYGNUM_HAL_BREAKPOINT_LIST_SIZE > 0) && defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_cache.h>

#ifdef TARGET_HAS_HARVARD_MEMORY
#define __read_mem_safe __read_progmem_safe
#define __write_mem_safe __write_progmem_safe
#endif

/*
 * A simple target breakpoint list without using malloc.
 * To use this package, you must define HAL_BREAKINST_SIZE to be the size
 * in bytes of a trap instruction (max if there's more than one),
 * HAL_BREAKINST to the opcode value of the instruction, and
 * HAL_BREAKINST_TYPE to be the type necessary to hold the opcode value.
 */

struct breakpoint_list {
  target_register_t  addr;
  char old_contents [HAL_BREAKINST_SIZE];
  struct breakpoint_list *next;
  char in_memory;
  char length;
} *breakpoint_list = NULL;

#ifndef HAL_BREAKINST_ADDR
static HAL_BREAKINST_TYPE break_inst = HAL_BREAKINST;
#define HAL_BREAKINST_ADDR(x) ((void*)&break_inst)
#endif

static struct breakpoint_list bp_list [CYGNUM_HAL_BREAKPOINT_LIST_SIZE];
static struct breakpoint_list *free_bp_list = NULL;
static int curr_bp_num = 0;

int
__set_breakpoint (target_register_t addr, target_register_t len)
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

  if (free_bp_list != NULL)
    {
      newent = free_bp_list;
      free_bp_list = free_bp_list->next;
    }
  else
    {
      if (curr_bp_num < CYGNUM_HAL_BREAKPOINT_LIST_SIZE)
	{
	  newent = &bp_list[curr_bp_num++];
	}
      else
	{
	  return 1;
	}
    }

  newent->addr = addr;
  newent->in_memory = 0;
  newent->next = l;
  newent->length = len;
  *addent = newent;

  return 0;
}

int
__remove_breakpoint (target_register_t addr, target_register_t len)
{
  struct breakpoint_list *l = breakpoint_list;
  struct breakpoint_list *prev = NULL;

  while (l != NULL && l->addr < addr)
    {
      prev = l;
      l = l->next;
    }

  if ((l == NULL) || (l->addr != addr))
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

  l->next = free_bp_list;
  free_bp_list = l;

  return 0;
}

#if defined(HAL_STUB_HW_BREAKPOINT_LIST_SIZE) && (HAL_STUB_HW_BREAKPOINT_LIST_SIZE > 0)
#ifndef HAL_STUB_HW_BREAKPOINT
#error "Must define HAL_STUB_HW_BREAKPOINT"
#endif
struct hw_breakpoint_list {
  target_register_t  addr;
  target_register_t  len;
  char used;
  char installed;
};
static struct hw_breakpoint_list hw_bp_list [HAL_STUB_HW_BREAKPOINT_LIST_SIZE];

int
__set_hw_breakpoint (target_register_t addr, target_register_t len)
{
  int i;

  for (i = 0; i < HAL_STUB_HW_BREAKPOINT_LIST_SIZE; i++)
    {
      if (hw_bp_list[i].used == 0)
	{
	  hw_bp_list[i].addr = addr;
	  hw_bp_list[i].len = len;
	  hw_bp_list[i].used = 1;
	  hw_bp_list[i].installed = 0;
	  return 0;
	}
    }
  return -1;
}

int
__remove_hw_breakpoint (target_register_t addr, target_register_t len)
{
  int i;

  for (i = 0; i < HAL_STUB_HW_BREAKPOINT_LIST_SIZE; i++)
    {
      if (hw_bp_list[i].used && hw_bp_list[i].addr == addr
	  && hw_bp_list[i].len == len)
	{
	  if (hw_bp_list[i].installed)
	    HAL_STUB_HW_BREAKPOINT(0, (void *)addr, (int)len);
	  hw_bp_list[i].used = 0;
	  return 0;
	}
    }
  return -1;
}

static void
__install_hw_breakpoint_list (void)
{
  int i;

  for (i = 0; i < HAL_STUB_HW_BREAKPOINT_LIST_SIZE; i++)
    {
      if (hw_bp_list[i].used && hw_bp_list[i].installed == 0)
	{
	  HAL_STUB_HW_BREAKPOINT(1, (void *)hw_bp_list[i].addr,
				 (int)hw_bp_list[i].len);
	  hw_bp_list[i].installed = 1;
	}
    }
}

static void
__clear_hw_breakpoint_list (void)
{
  int i;

  for (i = 0; i < HAL_STUB_HW_BREAKPOINT_LIST_SIZE; i++)
    {
      if (hw_bp_list[i].used && hw_bp_list[i].installed)
	{
	  HAL_STUB_HW_BREAKPOINT(0, (void *)hw_bp_list[i].addr,
				 (int)hw_bp_list[i].len);
	  hw_bp_list[i].installed = 0;
	}
    }
}
#endif // HAL_STUB_HW_BREAKPOINT_LIST_SIZE

#if defined(HAL_STUB_HW_WATCHPOINT_LIST_SIZE) && (HAL_STUB_HW_WATCHPOINT_LIST_SIZE > 0)
#ifndef HAL_STUB_HW_WATCHPOINT
#error "Must define HAL_STUB_HW_WATCHPOINT"
#endif
struct hw_watchpoint_list {
  target_register_t  addr;
  target_register_t  len;
  int ztype;
  char used;
  char installed;
};
static struct hw_watchpoint_list hw_wp_list [HAL_STUB_HW_WATCHPOINT_LIST_SIZE];

int
__set_hw_watchpoint (target_register_t addr, target_register_t len, int ztype)
{
  int i;

  for (i = 0; i < HAL_STUB_HW_WATCHPOINT_LIST_SIZE; i++)
    {
      if (hw_wp_list[i].used == 0)
	{
	  hw_wp_list[i].addr = addr;
	  hw_wp_list[i].len = len;
	  hw_wp_list[i].ztype = ztype;
	  hw_wp_list[i].used = 1;
	  hw_wp_list[i].installed = 0;
	  return 0;
	}
    }
  return -1;
}

int
__remove_hw_watchpoint (target_register_t addr, target_register_t len, int ztype)
{
  int i;

  for (i = 0; i < HAL_STUB_HW_WATCHPOINT_LIST_SIZE; i++)
    {
      if (hw_wp_list[i].used && hw_wp_list[i].addr == addr
	  && hw_wp_list[i].len == len && hw_wp_list[i].ztype == ztype )
	{
	  if (hw_wp_list[i].installed)
	    HAL_STUB_HW_WATCHPOINT(0, (void *)addr, (int)len, ztype);
	  hw_wp_list[i].used = 0;
	  return 0;
	}
    }
  return -1;
}

static void
__install_hw_watchpoint_list (void)
{
  int i;

  for (i = 0; i < HAL_STUB_HW_WATCHPOINT_LIST_SIZE; i++)
    {
      if (hw_wp_list[i].used && hw_wp_list[i].installed == 0)
	{
	  HAL_STUB_HW_WATCHPOINT(1, (void *)hw_wp_list[i].addr,
				 (int)hw_wp_list[i].len, hw_wp_list[i].ztype);
	  hw_wp_list[i].installed = 1;
	}
    }
}

static void
__clear_hw_watchpoint_list (void)
{
  int i;

  for (i = 0; i < HAL_STUB_HW_WATCHPOINT_LIST_SIZE; i++)
    {
      if (hw_wp_list[i].used && hw_wp_list[i].installed)
	{
	  HAL_STUB_HW_WATCHPOINT(0, (void *)hw_wp_list[i].addr,
				 (int)hw_wp_list[i].len, hw_wp_list[i].ztype);
	  hw_wp_list[i].installed = 0;
	}
    }
}
#endif // HAL_STUB_HW_WATCHPOINT_LIST_SIZE



void
__install_breakpoint_list (void)
{
  struct breakpoint_list *l = breakpoint_list;

  while (l != NULL)
    {
      if (! l->in_memory)
	{
	  int len = sizeof (l->old_contents);
	  if (__read_mem_safe (&l->old_contents[0], (void*)l->addr, len) == len)
	    {
	      if (__write_mem_safe (HAL_BREAKINST_ADDR(l->length),
				    (void*)l->addr, l->length) == l->length)
		{
		  l->in_memory = 1;
		}
	    }
	}
      l = l->next;
    }
#if defined(HAL_STUB_HW_BREAKPOINT_LIST_SIZE) && (HAL_STUB_HW_BREAKPOINT_LIST_SIZE > 0)
  __install_hw_breakpoint_list();
#endif
#if defined(HAL_STUB_HW_WATCHPOINT_LIST_SIZE) && (HAL_STUB_HW_WATCHPOINT_LIST_SIZE > 0)
  __install_hw_watchpoint_list();
#endif
  HAL_ICACHE_SYNC();
}

void
__clear_breakpoint_list (void)
{
  struct breakpoint_list *l = breakpoint_list;

  while (l != NULL)
    {
      if (l->in_memory)
	{
	  int len = sizeof (l->old_contents);
	  if (__write_mem_safe (&l->old_contents[0], (void*)l->addr, len) == len)
	    {
	      l->in_memory = 0;
	    }
	}
      l = l->next;
    }
#if defined(HAL_STUB_HW_BREAKPOINT_LIST_SIZE) && (HAL_STUB_HW_BREAKPOINT_LIST_SIZE > 0)
  __clear_hw_breakpoint_list();
#endif
#if defined(HAL_STUB_HW_WATCHPOINT_LIST_SIZE) && (HAL_STUB_HW_WATCHPOINT_LIST_SIZE > 0)
  __clear_hw_watchpoint_list();
#endif
  HAL_ICACHE_INVALIDATE_ALL();
}

int
__display_breakpoint_list (void (*print_func)(target_register_t))
{
  struct breakpoint_list *l = breakpoint_list;

  while (l != NULL)
    {
      print_func(l->addr);
      l = l->next;
    }

  return 0;
}
#else  // (CYGNUM_HAL_BREAKPOINT_LIST_SIZE == 0) or UNDEFINED

#include <cyg/hal/hal_stub.h>           // Our header

#ifndef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
// We don't know that type target_register_t is yet.
// Let's just pick a type so we can compile.  Since
// these versions of the functions don't actually do
// anything with the parameters, the actualy types
// don't matter.
typedef unsigned long target_register_t;
#endif

int
__set_breakpoint (target_register_t addr, target_register_t len)
{
  return 1;
}

int
__remove_breakpoint (target_register_t addr, target_register_t len)
{
  return 1;
}

void
__install_breakpoint_list (void)
{
}

void
__clear_breakpoint_list (void)
{
}

int
__display_breakpoint_list (void (*print_func)(target_register_t))
{
    return 0;
}
#endif // (CYGNUM_HAL_BREAKPOINT_LIST_SIZE > 0)

