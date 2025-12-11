/* Interfaces for libdwfl_stacktrace.

   XXX: This is an experimental initial version of the API, and is
   liable to change in future releases of elfutils, especially as
   we figure out how to generalize the work to other sample data
   formats in addition to perf_events.

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

#ifndef _LIBDWFL_STACKTRACE_H
#define _LIBDWFL_STACKTRACE_H  1

#include "libdwfl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Keeps track of and caches Elf structs across multiple libdwfl
   sessions corresponding to different processes.  */
typedef struct Dwflst_Process_Tracker Dwflst_Process_Tracker;


/* Initialize a new tracker for multiple libdwfl sessions.  Since Elf
   data will shared between the libdwfl sessions, each Dwfl must use
   the same Dwfl_Callbacks CALLBACKS provided when the tracker is
   created.  */
extern Dwflst_Process_Tracker *dwflst_tracker_begin (const Dwfl_Callbacks *callbacks)
  __nonnull_attribute__ (1);

/* Create a new Dwfl linked to this tracker.  */
extern Dwfl *dwflst_tracker_dwfl_begin (Dwflst_Process_Tracker *tracker)
  __nonnull_attribute__ (1);

/* Try to find a cached Elf corresponding to MODULE_NAME.  Verifies
   that the cached Elf has dev/ino/mtime matching the file on disk.
   Non-NULL MODULE_PATH specifies an alternate location for the module
   e.g. /proc/PID/root/MODULE_NAME.  Stores FILE_NAME and ELFP values.
   Returns fd similar to the find_elf callbacks, or -1 if cached Elf
   was not found.  */
extern int dwflst_tracker_find_cached_elf (Dwflst_Process_Tracker *tracker,
					   const char *module_name,
					   const char *module_path,
					   char **file_name, Elf **elfp)
  __nonnull_attribute__ (1, 2, 4, 5);

/* Store an Elf corresponding to MODULE_NAME in the tracker's cache.
   FILE_NAME and FD values must be provided, similar to the output of
   a find_elf callback.  Returns TRUE iff the Elf was successfully
   stored in the cache.  The tracker will retain the Elf* via libelf's
   reference counting mechanism, and release its reference during
   dwflst_tracker_end.  */
extern bool dwflst_tracker_cache_elf (Dwflst_Process_Tracker *tracker,
				      const char *module_name,
				      const char *file_name,
				      Elf *elf, int fd)
  __nonnull_attribute__ (1, 2);

/* Find the Dwfl corresponding to PID.  If CALLBACK is non-NULL and
   the Dwfl has not been created, invoke CALLBACK, which should return
   a Dwfl linked to the tracker (or NULL if Dwfl creation also fails).
   The Dwfl will be automatically added to the tracker's internal
   table when its pid is confirmed by calling dwfl_attach_state.
   Returns NULL if Dwfl was not found and callback failed.  */
extern Dwfl *dwflst_tracker_find_pid (Dwflst_Process_Tracker *tracker,
				      pid_t pid,
				      Dwfl *(*callback) (Dwflst_Process_Tracker *tracker,
							 pid_t pid,
							 void *arg),
				      void *arg)
  __nonnull_attribute__ (1);

/* For implementing a find_elf callback based on the prior two functions.
   Returns the Dwflst_Process_Tracker corresponding to MOD.  */
extern Dwflst_Process_Tracker *dwflst_module_gettracker (Dwfl_Module *mod);

/* End all libdwfl sessions with this tracker.  */
extern void dwflst_tracker_end (Dwflst_Process_Tracker *tracker);


/* Adaptation of the dwfl_linux_proc_find_elf callback from libdwfl,
   except this first attempts to look up a cached Elf* and fd from the
   Dwfl_Module's Dwflst_Process_Tracker (if any).  If a new Elf* is
   created, this callback saves it to the tracker's cache.
   The cache will retain the Elf* via libelf's reference counting
   mechanism, and release its reference during dwflst_tracker_end.  */
extern int dwflst_tracker_linux_proc_find_elf (Dwfl_Module *mod, void **userdata,
					       const char *module_name, Dwarf_Addr base,
					       char **file_name, Elf **);

/* Like dwfl_thread_getframes, but iterates through the frames for a
   stack sample rather than a live thread.  Register file for the stack
   sample is specified by REGS and N_REGS.  For each item in REGS, the
   REGS_MAPPING array specifies its position in the full register file
   expected by the DWARF infrastructure.  Calls dwfl_attach_state on
   DWFL, with architecture specified by ELF, ELF must remain vaild
   during Dwfl lifetime.  Returns zero if all frames have been
   processed by the callback, returns -1 on error, or the value of the
   callback when not DWARF_CB_OK. -1 returned on error will set
   dwfl_errno (). */
int dwflst_sample_getframes (Dwfl *dwfl, Elf *elf, pid_t pid, pid_t tid,
				  const void *stack, size_t stack_size,
				  const Dwarf_Word *regs, uint32_t n_regs,
				  const int *regs_mapping, size_t n_regs_mapping,
				  int (*callback) (Dwfl_Frame *state, void *arg),
				  void *arg)
    __nonnull_attribute__ (1, 5, 7, 9, 11);

/* Adapts dwflst_sample_getframes to linux perf_events stack sample
   and register file data format.  Calls dwfl_attach_state on DWFL,
   with architecture specified by ELF, ELF must remain valid during
   Dwfl lifetime.  Returns zero if all frames have been processed by
   the callback, returns -1 on error, or the value of the callback
   when not DWARF_CB_OK. -1 returned on error will set dwfl_errno
   (). */
int dwflst_perf_sample_getframes (Dwfl *dwfl, Elf *elf, pid_t pid, pid_t tid,
				  const void *stack, size_t stack_size,
				  const Dwarf_Word *regs, uint32_t n_regs,
				  uint64_t perf_regs_mask, uint32_t abi,
				  int (*callback) (Dwfl_Frame *state, void *arg),
				  void *arg)
  __nonnull_attribute__ (1, 5, 7, 11);

/* Returns the linux perf_events register mask describing a set of
   registers sufficient for unwinding on MACHINE, or 0 if libdwfl does
   not handle perf_events samples for MACHINE.  Does not take a Dwfl*
   or Elf* since this is meant to allow a profiling tool to configure
   perf_events to produce meaningful data for a libdwfl session to be
   opened later.  */
uint64_t dwflst_perf_sample_preferred_regs_mask (GElf_Half machine);

#ifdef __cplusplus
}
#endif

#endif  /* libdwfl_stacktrace.h */
