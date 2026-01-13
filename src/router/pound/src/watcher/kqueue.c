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
#include <sys/event.h>

struct fsevmon
{
  int kq;
  struct kevent *evtab;
  int evnum; /* Number of elements in evtab. */
};

static void
process_file_event (struct fsevmon *evmon, struct kevent *evp,
		    struct watchpoint *wp)
{
  if (evp->fflags & NOTE_DELETE)
    sentinel_setup (evmon, wp);
#ifdef NOTE_CLOSE_WRITE
  else if (evp->fflags & NOTE_CLOSE_WRITE)
    watcher_reread (wp->watcher);
#else
  else if (evp->fflags & NOTE_WRITE)
    wp->watcher->flags = 1;
  else if (evp->fflags & NOTE_CLOSE)
    {
      if (wp->watcher->flags)
	{
	  watcher_reread (wp->watcher);
	  wp->watcher->flags = 0;
	}
    }
#endif
}

static void
dir_restore_files (FSEVMON *evmon, struct watchpoint *sentinel)
{
  WORKDIR *wd = sentinel->wdir.wd;
  struct watchpoint *wp;
  struct stat st;
  
  DLIST_FOREACH (wp, &watch_head, link)
    {
      if (wp->type == WATCH_FILE && wp->watcher->wd == wd)
	{
	  if (fstatat (wd->fd, wp->watcher->filename, &st, 0) == 0)
	    {
	      if (!sentinel_wakeup (evmon, sentinel, wp->watcher->filename))
		break;
	    }
	}
    }
}

static void
process_dir_event (struct fsevmon *evmon, struct kevent *evp,
		   struct watchpoint *wp)
{
  if (evp->fflags & NOTE_DELETE)
    {
      logmsg (LOG_NOTICE, "%s: directory removed", wp->wdir.wd->name);
      workdir_set_compat_mode (wp->wdir.wd);
      fsevmon_watchpoint_remove (evmon, wp);
      return;
    }
  if (evp->fflags & (NOTE_WRITE | NOTE_EXTEND))
    dir_restore_files (evmon, wp);
}

static void
process_event (struct fsevmon *evmon, struct kevent *evp)
{
  struct watchpoint *wp = evp->udata;
  switch (wp->type)
    {
    case WATCH_FILE:
      process_file_event (evmon, evp, wp);
      break;

    case WATCH_DIR:
      process_dir_event (evmon, evp, wp);
      break;
    }
}

static size_t
watchpoint_count (void)
{
  struct watchpoint *wp;
  size_t n = 0;
  DLIST_FOREACH (wp, &watch_head, link)
    ++n;
  return n;
}

static struct fsevmon *
fsevmon_init (void)
{
  struct fsevmon *emp;
  size_t n = watchpoint_count ();
  int fd;
  
  if (n == 0)
    {
      errno = 0;
      return NULL;
    }
  
  if ((fd = kqueue ()) == -1)
    return NULL;
  
  XZALLOC (emp);
  emp->kq = fd;
  emp->evtab = xcalloc (n, sizeof (emp->evtab[0]));
  emp->evnum = n;
  return emp;
}

static int
fsevmon_select (struct fsevmon *emp)
{
  int i, n;

  n = kevent (emp->kq, NULL, 0, emp->evtab, emp->evnum, NULL);
  if (n == -1)
    {
      if (errno == EINTR)
	return 0;
      else
	{
	  logmsg (LOG_ERR, "kevent: %s", strerror (errno));
	  return 1;
	}
    }
  for (i = 0; i < n; i++)
    process_event (emp, &emp->evtab[i]);

  return 0;
}

static int
watchpoint_set_internal (struct fsevmon *evmon,
			 char const *filename, WORKDIR *wd,
			 struct watchpoint *wp,
			 int flags,
			 int mask)
{
  struct kevent evt;
  int fd;
  char *msg;
  struct locus_point pt;

  fd = openat (wd ? wd->fd : AT_FDCWD, filename, flags);;
  if (fd != -1)
    {
      EV_SET (&evt, fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, mask,
	      0, wp);
      if (kevent (evmon->kq, &evt, 1, NULL, 0, NULL) != -1)
	{
	  wp->wd = fd;
	  return 0;
	}
      msg = "error adding kevent";
    }
  else
    msg = "can't open file";

  locus_point_init (&pt, filename, wd ? wd->name : NULL);
  logmsg (LOG_ERR, "%s: %s: %s", msg, string_ptr (pt.filename),
	  strerror (errno));
  locus_point_unref (&pt);
  return -1;
}

int
fsevmon_watchpoint_install (struct fsevmon *evmon, struct watchpoint *wp)
{
  char const *filename;
  WORKDIR *wd = NULL;
  int flags = O_RDONLY | O_NONBLOCK;
  int mask = NOTE_DELETE;
  
  switch (wp->type)
    {
    case WATCH_FILE:
      filename = wp->watcher->filename;
      wd = wp->watcher->wd;
#if defined(NOTE_CLOSE_WRITE)
      mask |= NOTE_CLOSE_WRITE;
#else
      mask |= NOTE_CLOSE | NOTE_WRITE;
#endif	
      break;

    case WATCH_DIR:
      filename = wp->wdir.wd->name;
      wd = NULL;
      mask |= NOTE_WRITE | NOTE_EXTEND;
      flags |= O_DIRECTORY;
    }
  return watchpoint_set_internal (evmon, filename, wd, wp, flags, mask);
}

void
fsevmon_watchpoint_uninstall (struct fsevmon *evmon, struct watchpoint *wp)
{
  /* NOTE:
     Events which are attached to file descriptors are automatically deleted
     on the last close of the descriptor. */
  struct kevent ev;

  if (wp->wd == -1)
    return;
  EV_SET (&ev, wp->wd, EVFILT_VNODE, EV_DELETE, 0, 0, NULL);
  if (kevent (evmon->kq, &ev, 1, NULL, 0, NULL) == -1)
    logmsg (LOG_ERR, "error removing kevent: %s", strerror (errno));
  close (wp->wd);
  wp->wd = -1;
}

static void
watcher_cleanup (void *arg)
{
  struct fsevmon *evmon = arg;
  close (evmon->kq);
  free (evmon->evtab);
  free (evmon);
}

static void *
thr_watcher (void *arg)
{
  struct fsevmon *evmon = arg;
  pthread_cleanup_push (watcher_cleanup, evmon);
  while (1)
    fsevmon_select (evmon);
  pthread_cleanup_pop (1);
  return NULL;
}  

void
watchpoint_set_mode (struct watchpoint *wp, enum watcher_mode mode)
{
  wp->watcher->mode = mode;
}

int
watcher_setup (void)
{
  struct fsevmon *evmon;
  
  struct watchpoint *wp;
  pthread_t tid;

  if (DLIST_EMPTY (&watch_head))
    return 0;

  evmon = fsevmon_init ();
  if (!evmon)
    {
      logmsg (LOG_CRIT, "kqueue: %s", strerror (errno));
      return -1;
    }

  DLIST_FOREACH (wp, &watch_head, link)
    {
      if (wp->type == WATCH_DIR)
	continue;
      switch (wp->watcher->mode)
	{
	case WATCHER_EXISTS:
	  fsevmon_watchpoint_install (evmon, wp);
	  break;

	case WATCHER_NOFILE:
	  watchpoint_dir_locate (evmon, wp->watcher->wd);
	  break;

	case WATCHER_COMPAT:
	  /* Shouldn't happen. */
	  break;
	}
    }

  pthread_create (&tid, &thread_attr_detached, thr_watcher, evmon);
  return 0;
}
