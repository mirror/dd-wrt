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
struct watchpoint;

enum watcher_mode
  {
    WATCHER_EXISTS,    /* File exists and is monitored. */
    WATCHER_NOFILE,    /* File does not exist, its directory is monitored. */
    WATCHER_COMPAT     /* Compatibility mode. */
  };

struct watcher
{
  enum watcher_mode mode;
  void *obj;
  int (*read) (void *, char const *, WORKDIR *);
  void (*clear) (void *);
  WORKDIR *wd;                /* Working directory. */
  char *filename;             /* Filename relative to wd. */
  struct locus_range locus;
  pthread_rwlock_t rwl;       /* Locker. */
  struct timespec mtim;       /* File mtime.  Used if inotify is
				 not available. */
  int flags;
};

enum
  {
    WATCH_FILE,
    WATCH_DIR
  };

struct watchpoint
{
  int type;
  int wd;
  DLIST_ENTRY (watchpoint) link;
  union
  {
    struct watcher *watcher;
    struct
    {
      WORKDIR *wd;
      size_t nref;
    } wdir;
  };
};

typedef struct fsevmon FSEVMON;

typedef DLIST_HEAD (,watchpoint) WATCHPOINT_HEAD;

extern WATCHPOINT_HEAD watch_head;

void workdir_set_compat_mode (WORKDIR *wd);

void watcher_reread (struct watcher *watcher);
void watcher_clear (struct watcher *watcher);
void watcher_log (int pri, struct watcher *watcher, char const *fmt, ...)
  ATTR_PRINTFLIKE(3,4);

int fsevmon_watchpoint_install (FSEVMON *evmon, struct watchpoint *wp);
void fsevmon_watchpoint_uninstall (FSEVMON *evmon, struct watchpoint *wp);

void fsevmon_disable (FSEVMON *evmon);
void fsevmon_watchpoint_remove (FSEVMON *evmon, struct watchpoint *wp);

void watchpoint_set_compat_mode (struct watchpoint *wp);
void watchpoint_free (struct watchpoint *wp);
struct watchpoint *watchpoint_locate (int wd);
struct watchpoint *watchpoint_file_locate (WORKDIR *wdir, char const *filename);

struct watchpoint *watchpoint_dir_locate (FSEVMON *evmon, WORKDIR *wd);
struct watchpoint *watchpoint_dir_unref (FSEVMON *evmon, struct watchpoint *wp);

struct watchpoint *sentinel_setup (FSEVMON *evmon, struct watchpoint *wp);
struct watchpoint *sentinel_wakeup (FSEVMON *evmon,
				    struct watchpoint *sentinel,
				    char const *name);

void watchpoint_set_mode (struct watchpoint *wp, enum watcher_mode);
