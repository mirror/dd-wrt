/* Find Elf file and cache via Dwflst_Process_Tracker.
   Copyright (C) 2025, Red Hat, Inc.
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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/stat.h>
#include "../libelf/libelfP.h"
/* XXX: Private header needed for Elf * ref_count field. */
/* TODO: Consider dup_elf() rather than direct ref_count access. */

#include "libdwfl_stacktraceP.h"

unsigned long int
__libdwfl_stacktrace_elftab_hash_st (const char *module_name,
				     dev_t st_dev,
				     ino_t st_ino)
{
  unsigned long int hval = elf_hash(module_name);
  hval ^= (unsigned long int)st_dev;
  hval ^= (unsigned long int)st_ino;
  return hval;
}

unsigned long int
__libdwfl_stacktrace_elftab_hash (const char *module_name,
				  const char *module_path,
				  int fd)
{
  struct stat sb;
  int rc = -1;
  if (fd >= 0)
    rc = fstat(fd, &sb);
  else if (module_path != NULL)
    rc = stat(module_path, &sb);
  if (rc < 0)
    return elf_hash(module_name);
  return __libdwfl_stacktrace_elftab_hash_st
    (module_name, sb.st_dev, sb.st_ino);
}

int
dwflst_tracker_find_cached_elf (Dwflst_Process_Tracker *tracker,
				const char *module_name,
				const char *module_path,
				char **file_name, Elf **elfp)
{
  dwflst_tracker_elf_info *ent = NULL;
  int rc = -1;
  struct stat sb;

  if (module_path == NULL)
    module_path = module_name;
  unsigned long int hval =
    __libdwfl_stacktrace_elftab_hash (module_name, module_path, -1/* no fd */);

  rwlock_rdlock(tracker->elftab_lock);
  ent = dwflst_tracker_elftab_find(&tracker->elftab, hval);
  rwlock_unlock(tracker->elftab_lock);

  /* Guard against collisions.
     TODO: Need proper chaining, dynamicsizehash_concurrent isn't really
     equipped for it. */
  if (ent != NULL)
    rc = fstat(ent->fd, &sb);
  if (rc < 0 || strcmp (module_name, ent->module_name) != 0
      || ent->dev != sb.st_dev || ent->ino != sb.st_ino)
    return -1;

  /* Verify that ent->fd has not been updated: */
  if (rc < 0 || ent->dev != sb.st_dev || ent->ino != sb.st_ino
      || ent->last_mtime != sb.st_mtime)
    return -1;

  if (ent->elf != NULL)
    ent->elf->ref_count++;
  *elfp = ent->elf;
  *file_name = strdup(ent->module_name);
  return ent->fd;
}
INTDEF(dwflst_tracker_find_cached_elf)

bool
dwflst_tracker_cache_elf (Dwflst_Process_Tracker *tracker,
			  const char *module_name,
			  const char *file_name __attribute__((unused)),
			  Elf *elf, int fd)
{
  dwflst_tracker_elf_info *ent = NULL;
  int rc = -1;
  struct stat sb;

  if (fd >= 0)
    rc = fstat(fd, &sb);
  if (rc < 0)
    return false;
  unsigned long int hval =
    __libdwfl_stacktrace_elftab_hash_st (module_name, sb.st_dev, sb.st_ino);

  rwlock_wrlock(tracker->elftab_lock);
  ent = dwflst_tracker_elftab_find(&tracker->elftab, hval);
  /* Guard against collisions.
     TODO: Need proper chaining, dynamicsizehash_concurrent isn't really
     equipped for it. */
  if (ent != NULL && (strcmp (module_name, ent->module_name) != 0
		      || ent->dev != sb.st_dev || ent->ino != sb.st_ino))
    {
      rwlock_unlock(tracker->elftab_lock);
      return false;
    }
  if (ent == NULL)
    {
      ent = calloc (1, sizeof (dwflst_tracker_elf_info));
      if (ent == NULL)
	{
	  rwlock_unlock(tracker->elftab_lock);
	  __libdwfl_seterrno (DWFL_E_NOMEM);
	  return false;
	}
      ent->module_name = strdup(module_name);

      if (dwflst_tracker_elftab_insert(&tracker->elftab, hval, ent) != 0)
	{
	  free(ent->module_name);
	  free(ent);
	  rwlock_unlock(tracker->elftab_lock);
	  assert(false); /* Should not occur due to the wrlock on elftab. */
	}
    }
  else
    {
      /* TODO: The following assertions are still triggered on certain
	 code paths that acquire fds or create Elf structs without
	 checking the caching mechanism first.  This is not a serious
	 problem, and can be fixed incrementally. */

      /* assert(ent->elf == NULL || ent->elf == elf); */ /* Guard against redundant/leaked Elf *. */
      /* assert(ent->fd == fd); */ /* Guard against redundant open. */

      /* For now, correct behaviour (from dwfl_module_getdwarf.c open_elf)
         is to replace the existing elf, keep module_name. */
      if (ent->elf != NULL && ent->elf != elf)
	elf_end(ent->elf);
    }
  if (elf != NULL && ent->elf != elf)
    elf->ref_count++;
  ent->elf = elf;
  ent->fd = fd;
  if (rc == 0)
    {
      ent->dev = sb.st_dev;
      ent->ino = sb.st_ino;
      ent->last_mtime = sb.st_mtime;
    }
  /* else create a cache entry with 0 values for dev/ino/mtime;
     since dev/ino are hashed, this will not conflict with entries
     for which fstat was successful */
  rwlock_unlock(tracker->elftab_lock);
  return true;
}
INTDEF(dwflst_tracker_cache_elf)

Dwflst_Process_Tracker *
dwflst_module_gettracker (Dwfl_Module *mod)
{
  if (mod == NULL)
    return NULL;
  if (mod->dwfl == NULL)
    return NULL;
  return mod->dwfl->tracker;
}
INTDEF(dwflst_module_gettracker)

int
dwflst_tracker_linux_proc_find_elf (Dwfl_Module *mod,
				    void **userdata __attribute__ ((unused)),
				    const char *module_name, Dwarf_Addr base,
				    char **file_name, Elf **elfp)
{
  Dwflst_Process_Tracker *tracker = INTUSE(dwflst_module_gettracker) (mod);
  int fd;

  if (tracker != NULL)
    {
      fd = INTUSE(dwflst_tracker_find_cached_elf)
	(tracker, module_name, module_name, file_name, elfp);
      if (fd >= 0)
	return fd;
    }

  fd = INTUSE(dwfl_linux_proc_find_elf) (mod, userdata, module_name,
					 base, file_name, elfp);

  if (tracker != NULL && fd >= 0 && *file_name != NULL)
    {
      INTUSE(dwflst_tracker_cache_elf)
	(tracker, module_name, *file_name, *elfp, fd);
    }
  return fd;
}
