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
#include <sys/inotify.h>

struct fsevmon
{
  int ifd;
};

void
watchpoint_set_mode (struct watchpoint *wp, enum watcher_mode mode)
{
  wp->watcher->mode = mode;
}

int
fsevmon_watchpoint_install (FSEVMON *evmon, struct watchpoint *wp)
{
  struct locus_point pt;

  switch (wp->type)
    {
    case WATCH_FILE:
      locus_point_init (&pt, wp->watcher->filename, wp->watcher->wd->name);
      wp->wd = inotify_add_watch (evmon->ifd, string_ptr (pt.filename),
				  IN_CLOSE_WRITE | IN_MOVE_SELF);
      locus_point_unref (&pt);
      break;

    case WATCH_DIR:
      wp->wd = inotify_add_watch (evmon->ifd, wp->wdir.wd->name,
				  IN_CREATE | IN_MOVED_TO);
      break;
    }
  return wp->wd == -1;
}

void
fsevmon_watchpoint_uninstall (FSEVMON *evmon, struct watchpoint *wp)
{
  if (wp->wd != -1)
    {
      inotify_rm_watch (evmon->ifd, wp->wd);
      wp->wd = -1;
    }
}

static void
process_event (FSEVMON *evmon, struct inotify_event *ep)
{
  struct watchpoint *wp;

  if (ep->mask & IN_Q_OVERFLOW)
    logmsg (LOG_NOTICE, "event queue overflow");

  wp = watchpoint_locate (ep->wd);
  if (!wp)
    {
      if (!(ep->mask & IN_IGNORED))
	{
	  if (ep->len > 0)
	    logmsg (LOG_NOTICE, "ignoring unrecognized event %#x for %s",
		    ep->mask, ep->name);
	  else
	    logmsg (LOG_NOTICE, "ignoring unrecognized event %#x", ep->mask);
	}
      return;
    }

#if 0
  if (ep->len > 0)
    logmsg (LOG_DEBUG, "event %#x, for %s", ep->mask, ep->name);
  else
    logmsg (LOG_DEBUG, "event %#x, %s, %s", ep->mask, wp->watcher->filename, wp->watcher->wd->name);
#endif

  if (ep->mask & IN_IGNORED)
    {
      wp->wd = -1;
      switch (wp->type)
	{
	case WATCH_FILE:
	  sentinel_setup (evmon, wp);
	  break;

	case WATCH_DIR:
	  logmsg (LOG_NOTICE, "%s: directory removed", wp->wdir.wd->name);
	  workdir_set_compat_mode (wp->wdir.wd);
	  fsevmon_watchpoint_remove (evmon, wp);
	  break;
	}
      return;
    }

  if (ep->mask & IN_MOVE_SELF)
    sentinel_setup (evmon, wp);
  else if (ep->mask & (IN_CREATE | IN_MOVED_TO))
    sentinel_wakeup (evmon, wp, ep->name);
  else if (ep->mask & IN_CLOSE_WRITE)
    watcher_reread (wp->watcher);
}

static void
watcher_cleanup (void *arg)
{
  FSEVMON *evmon = arg;
  fsevmon_disable (evmon);
  close (evmon->ifd);
  free (evmon);
}

static void *
thr_watcher (void *arg)
{
  FSEVMON *evmon = arg;
  char buffer[4096];
  struct inotify_event *ep;
  size_t size;
  ssize_t rdbytes;

  pthread_cleanup_push (watcher_cleanup, arg);
  while (1)
    {
      rdbytes = read (evmon->ifd, buffer, sizeof (buffer));
      if (rdbytes == -1)
	{
	  if (errno == EINTR)
	    continue;
	  logmsg (LOG_CRIT, "inotify read failed: %s", strerror (errno));
	  break;
	}
      ep = (struct inotify_event *) buffer;

      while (rdbytes)
	{
	  if (ep->wd >= 0)
	    process_event (evmon, ep);
	  size = sizeof (*ep) + ep->len;
	  ep = (struct inotify_event *) ((char*) ep + size);
	  rdbytes -= size;
	}
    }
  pthread_cleanup_pop (1);
  return NULL;
}

int
watcher_setup (void)
{
  FSEVMON *evmon;
  struct watchpoint *wp;
  pthread_t tid;
  int ifd;

  if (DLIST_EMPTY (&watch_head))
    return 0;

  ifd = inotify_init ();
  if (ifd == -1)
    {
      logmsg (LOG_CRIT, "inotify_init: %s", strerror (errno));
      return -1;
    }
  XZALLOC (evmon);
  evmon->ifd = ifd;
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
