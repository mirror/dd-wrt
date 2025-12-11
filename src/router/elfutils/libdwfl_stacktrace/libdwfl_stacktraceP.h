/* Internal definitions for libdwfl_stacktrace.
   Copyright (C) 2025 Red Hat, Inc.
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of either

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at
       your option) any later version

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at
       your option) any later version

   or both in parallel, as here.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef _LIBDWFL_STACKTRACEP_H
#define _LIBDWFL_STACKTRACEP_H 1

#include <libdwfl_stacktrace.h>

#include "libdwflP.h"

/* Hash table for Elf *. */
typedef struct
{
  char *module_name; /* dwfltracker_elftab_ent is used iff non-NULL.  */
  int fd;
  Elf *elf;
  dev_t dev;
  ino_t ino;
  time_t last_mtime;
} dwflst_tracker_elf_info;
#include "dwflst_tracker_elftab.h"

/* Hash table for Dwfl *. */
typedef struct
{
  Dwfl *dwfl;
  bool invalid; /* Mark when the dwfl has been removed.  */
} dwflst_tracker_dwfl_info;
#include "dwflst_tracker_dwfltab.h"

struct Dwflst_Process_Tracker
{
  const Dwfl_Callbacks *callbacks;

  /* Table of cached Elf * including fd, path, fstat info.  */
  dwflst_tracker_elftab elftab;
  rwlock_define(, elftab_lock);

  /* Table of cached Dwfl * including pid.  */
  dwflst_tracker_dwfltab dwfltab;
  rwlock_define(, dwfltab_lock);
};


/* Called when dwfl->process->pid becomes known to add the dwfl to its
   Dwflst_Process_Tracker's dwfltab:  */
extern void __libdwfl_stacktrace_add_dwfl_to_tracker (Dwfl *dwfl)
  internal_function;

/* Called from dwfl_end() to remove the dwfl from its
   Dwfl_Process_Tracker's dwfltab:  */
extern void __libdwfl_stacktrace_remove_dwfl_from_tracker (Dwfl *dwfl)
  internal_function;


/* Avoid PLT entries.  */
INTDECL (dwflst_module_gettracker)
INTDECL (dwflst_tracker_find_cached_elf)
INTDECL (dwflst_tracker_cache_elf)


#endif  /* libdwfl_stacktraceP.h */
