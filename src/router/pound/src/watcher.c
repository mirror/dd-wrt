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

unsigned watcher_ttl = 60;

static void watcher_stat (struct watcher *watcher);
static void watcher_read_unlocked (struct watcher *watcher);
static void watcher_clear_unlocked (struct watcher *watcher);

void
watcher_log (int pri, struct watcher *watcher, char const *fmt, ...)
{
  va_list ap;
  struct stringbuf sb;
  struct locus_point pt;

  xstringbuf_init (&sb);
  stringbuf_format_locus_range (&sb, &watcher->locus);
  stringbuf_add_string (&sb, ": ");

  locus_point_init (&pt, watcher->filename,
		    watcher->wd ? watcher->wd->name : NULL);
  stringbuf_add_string (&sb, string_ptr (pt.filename));
  locus_point_unref (&pt);
  stringbuf_add_string (&sb, ": ");

  va_start (ap, fmt);
  stringbuf_vprintf (&sb, fmt, ap);
  va_end (ap);
  logmsg (pri, "%s", stringbuf_value (&sb));
  stringbuf_free (&sb);
}

static inline void
mtim_init (struct timespec *ts)
{
  memset (ts, 0, sizeof *ts);
}

static inline int
mtim_is_zero (struct timespec *ts)
{
  return ts->tv_sec == 0;
}

static inline int
mtim_is_newer (struct timespec *ts, struct timespec *ref)
{
  return timespec_cmp (ts, ref) > 0;
}

static void
job_watcher_check (enum job_ctl ctl, void *arg, const struct timespec *ts)
{
  struct watcher *watcher = arg;
  if (ctl == job_ctl_run)
    {
      struct timespec mtim;

      pthread_rwlock_wrlock (&watcher->rwl);
      mtim = watcher->mtim;
      watcher_stat (watcher);
      if (mtim_is_newer (&watcher->mtim, &mtim))
	{
	  if (mtim_is_zero (&mtim))
	    watcher_log (LOG_INFO, watcher, "file restored");
	  watcher_read_unlocked (watcher);
	}
      else if (mtim_is_zero (&watcher->mtim))
	{
	  if (!mtim_is_zero (&mtim))
	    {
	      watcher_log (LOG_INFO, watcher, "file removed");
	      watcher_clear_unlocked (watcher);
	    }
	}
      pthread_rwlock_unlock (&watcher->rwl);
      job_enqueue_after (watcher_ttl, job_watcher_check, watcher);
    }
}

WATCHPOINT_HEAD watch_head;

void
watchpoint_free (struct watchpoint *wp)
{
  free (wp);
}

void
watchpoint_set_compat_mode (struct watchpoint *wp)
{
  wp->watcher->mode = WATCHER_COMPAT;
  mtim_init (&wp->watcher->mtim);
  watcher_stat (wp->watcher);
  job_enqueue_after (watcher_ttl, job_watcher_check, wp->watcher);
}

static void
watcher_stat (struct watcher *watcher)
{
  struct stat st;
  mtim_init (&watcher->mtim);
  if (fstatat (watcher->wd->fd, watcher->filename, &st, 0))
    {
      if (errno != ENOENT)
	watcher_log (LOG_ERR, watcher, "can't stat: %s", strerror (errno));
    }
  else
#if HAVE_STRUCT_STAT_ST_MTIM
    watcher->mtim = st.st_mtim;
#else
    watcher->mtim.tv_sec = st.st_mtime;
#endif
}

static inline void
watcher_open_error (struct watcher *watcher, int ec)
{
  watcher_log (LOG_ERR, watcher, "can't open file: %s", strerror (ec));
}

static void
watcher_read_unlocked (struct watcher *watcher)
{
  watcher->clear (watcher->obj);
  if (watcher->read (watcher->obj, watcher->filename, watcher->wd) == -1)
    watcher_open_error (watcher, errno);
  else
    watcher_log (LOG_INFO, watcher, "file reloaded");
}

static void
watcher_clear_unlocked (struct watcher *watcher)
{
  watcher->clear (watcher->obj);
  watcher_log (LOG_INFO, watcher, "content cleared");
}

void
watcher_clear (struct watcher *watcher)
{
  pthread_rwlock_wrlock (&watcher->rwl);
  watcher_clear_unlocked (watcher);
  pthread_rwlock_unlock (&watcher->rwl);
}

void
watcher_reread (struct watcher *watcher)
{
  pthread_rwlock_wrlock (&watcher->rwl);
  watcher_read_unlocked (watcher);
  pthread_rwlock_unlock (&watcher->rwl);
}

static char *
filename_catn (char const *dir, char const *file, size_t flen)
{
  char *buf;
  size_t len;
  if (dir)
    {
      len = strlen (dir);
      while (len > 0 && dir[len-1] == '/')
	--len;
    }
  else
    len = 0;
  buf = xmalloc (len + flen + 2);
  if (dir)
    memcpy (buf, dir, len);
  buf[len++] = '/';
  memcpy (buf + len, file, flen);
  buf[len + flen] = 0;
  return buf;
}

char const *
filename_split_wd (char const *filename, WORKDIR **wdp)
{
  char const *name;
  WORKDIR *wd;
  char *dir = NULL;

  if ((name = strrchr (filename, '/')) == NULL)
    {
      name = filename;
      if ((wd = get_include_wd ()) == NULL)
	return NULL;
    }
  else if (filename[0] == '/')
    {
      dir = xstrndup (filename, name - filename);
      name++;
    }
  else
    {
      if ((wd = get_include_wd ()) == NULL)
	return NULL;
      dir = filename_catn (wd->name, filename, name - filename);
      name++;
    }
  if ((wd = workdir_get (dir)) == NULL)
    {
      logmsg (LOG_ERR, "can't open directory %s: %s", dir, strerror (errno));
      name = NULL;
    }
  else
    *wdp = wd;

  free (dir);

  return name;
}

void
watcher_lock (struct watcher *dp)
{
  if (dp)
    pthread_rwlock_rdlock (&dp->rwl);
}

void
watcher_unlock (struct watcher *dp)
{
  if (dp)
    pthread_rwlock_unlock (&dp->rwl);
}

struct watcher *
watcher_register (void *obj, char const *filename,
		  struct locus_range const *loc,
		  int (*read) (void *, char const *, WORKDIR *),
		  void (*clear) (void *))
{
  struct watchpoint *wp;
  int rc;
  enum watcher_mode mode;
  char const *basename;

  XZALLOC (wp);
  wp->wd = -1;
  wp->type = WATCH_FILE;
  XZALLOC (wp->watcher);
  wp->watcher->obj = obj;
  wp->watcher->read = read;
  wp->watcher->clear = clear;

  basename = filename_split_wd (filename, &wp->watcher->wd);
  if (!basename)
    {
      conf_error_at_locus_range (loc, "can't register watcher");
      free (wp);
      return NULL;
    }
  wp->watcher->filename = xstrdup (basename);

  locus_range_init (&wp->watcher->locus);
  locus_range_copy (&wp->watcher->locus, loc);
  pthread_rwlock_init (&wp->watcher->rwl, NULL);

  rc = wp->watcher->read (wp->watcher->obj, wp->watcher->filename,
			  wp->watcher->wd);
  if (rc == -1)
    {
      if (errno == ENOENT)
	{
	  watcher_log (LOG_WARNING, wp->watcher, "file does not exist");
	  mode = WATCHER_NOFILE;
	}
      else
	{
	  watcher_open_error (wp->watcher, errno);
	  watchpoint_free (wp);
	  return NULL;
	}
    }
  else
    mode = WATCHER_EXISTS;

  watchpoint_set_mode (wp, mode);

  DLIST_PUSH (&watch_head, wp, link);

  return wp->watcher;
}
