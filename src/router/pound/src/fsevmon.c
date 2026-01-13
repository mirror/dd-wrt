/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2025 Sergey Poznyakoff
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pound.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "pound.h"
#include "extern.h"
#include "watcher.h"

struct watchpoint *
watchpoint_locate (int wd)
{
  struct watchpoint *wp;
  DLIST_FOREACH (wp, &watch_head, link)
    {
      if (wp->wd == wd)
	return wp;
    }
  return NULL;
}

struct watchpoint *
watchpoint_file_locate (WORKDIR *wdir, char const *filename)
{
  struct watchpoint *wp;
  DLIST_FOREACH (wp, &watch_head, link)
    {
      if (wp->type == WATCH_FILE &&
	  wp->watcher->mode == WATCHER_NOFILE &&
	  wp->watcher->wd == wdir &&
	  strcmp (wp->watcher->filename, filename) == 0)
	return wp;
    }
  return NULL;
}

void
fsevmon_disable (FSEVMON *evmon)
{
  struct watchpoint *wp, *tmp;
  DLIST_FOREACH_SAFE (wp, tmp, &watch_head, link)
    {
      switch (wp->type)
	{
	case WATCH_FILE:
	  fsevmon_watchpoint_uninstall (evmon, wp);
	  wp->wd = -1;
	  watchpoint_set_compat_mode (wp);
	  break;

	case WATCH_DIR:
	  fsevmon_watchpoint_remove (evmon, wp);
	}
    }
}

void
fsevmon_watchpoint_remove (FSEVMON *evmon, struct watchpoint *wp)
{
  DLIST_REMOVE (&watch_head, wp, link);
  fsevmon_watchpoint_uninstall (evmon, wp);
  watchpoint_free (wp);
}

struct watchpoint *
watchpoint_dir_locate (FSEVMON *evmon, WORKDIR *wd)
{
  struct watchpoint *wp;
  DLIST_FOREACH (wp, &watch_head, link)
    {
      if (wp->type == WATCH_DIR && wp->wdir.wd == wd)
	{
	  wp->wdir.nref++;
	  return wp;
	}
    }
  XZALLOC (wp);
  wp->type = WATCH_DIR;
  wp->wdir.wd = wd;
  wp->wdir.nref = 1;
  if (fsevmon_watchpoint_install (evmon, wp))
    {
      free (wp);
      return NULL;
    }
  DLIST_PUSH (&watch_head, wp, link);
  return wp;
}

struct watchpoint *
watchpoint_dir_unref (FSEVMON *evmon, struct watchpoint *wp)
{
  if (--wp->wdir.nref == 0)
    {
      fsevmon_watchpoint_remove (evmon, wp);
      return NULL;
    }
  return wp;
}

struct watchpoint *
sentinel_wakeup (FSEVMON *evmon, struct watchpoint *sentinel,
		 char const *name)
{
  struct watchpoint *wp = watchpoint_file_locate (sentinel->wdir.wd, name);
  if (wp)
    {
      watcher_log (LOG_INFO, wp->watcher, "file restored");
      wp->watcher->mode = WATCHER_EXISTS;
      fsevmon_watchpoint_install (evmon, wp);
      sentinel = watchpoint_dir_unref (evmon, sentinel);
      watcher_reread (wp->watcher);
    }
  return sentinel;
}

struct watchpoint *
sentinel_setup (FSEVMON *evmon, struct watchpoint *wp)
{
  WATCHER *watcher = wp->watcher;
  struct watchpoint *sentinel;
  struct stat st;

  watcher_log (LOG_INFO, watcher, "file removed");
  watcher_clear (watcher);
  fsevmon_watchpoint_uninstall (evmon, wp);
  watcher->mode = WATCHER_NOFILE;
  sentinel = watchpoint_dir_locate (evmon, watcher->wd);
  if (sentinel == NULL)
    watchpoint_set_compat_mode (wp);
  else if (fstatat (watcher->wd->fd, watcher->filename, &st, 0) == 0)
    {
      /* The file had been created before the sentinel was installed.
	 No create event will be reported in this case, so trigger the
	 sentinel explicitly. */
      sentinel = sentinel_wakeup (evmon, sentinel, watcher->filename);
    }
  return sentinel;
}

void
workdir_set_compat_mode (WORKDIR *wd)
{
  struct watchpoint *wp;
  DLIST_FOREACH (wp, &watch_head, link)
    {
      if (wp->type == WATCH_FILE && wp->watcher->wd == wd)
	watchpoint_set_compat_mode (wp);
    }
}
