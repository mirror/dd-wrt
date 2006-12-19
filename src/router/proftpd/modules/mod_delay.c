/*
 * ProFTPD: mod_delay -- a module for adding arbitrary delays to the FTP
 *                       session lifecycle
 *
 * Copyright (c) 2004 TJ Saunders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * This is mod_delay, contrib software for proftpd 1.2.10 and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * $Id: mod_delay.c,v 1.1 2006/04/24 11:39:28 honor Exp $
 */

#include "conf.h"
#include "privs.h"

#define MOD_DELAY_VERSION		"mod_delay/0.5"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001021001
# error "ProFTPD 1.2.10rc1 or later required"
#endif

#include <signal.h>

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif

/* Number of values to keep in a row. */
#ifndef DELAY_NVALUES
# define DELAY_NVALUES			256
#endif

/* Fraction of total values that are accepted from a single session. */
#ifndef DELAY_SESS_NVALUES
# define DELAY_SESS_NVALUES		16
#endif

extern xaset_t *server_list;

module delay_module;

struct delay_rec {
  unsigned int d_sid;
  char d_addr[80];
  unsigned int d_port;
  unsigned int d_nvals;
  long d_vals[DELAY_NVALUES];
};

struct {
  const char *dt_path;
  int dt_fd;
  off_t dt_size;
  void *dt_data;

} delay_tab;

static unsigned int delay_engine = TRUE;
static unsigned int delay_nuser = 0;
static unsigned int delay_npass = 0;
static struct timeval delay_tv;

static void delay_table_reset(void);

#define delay_swap(a, b) \
  tmp = (a); \
  (a) = (b); \
  (b) = tmp;

static long delay_select_k(unsigned long k, array_header *values) {
  unsigned long l, ir, tmp = 0;
  long *elts = (long *) values->elts;
  int nelts = values->nelts;

  /* This is from "Numeric Recipes in C", Ch. 8.5, as the select()
   * algorithm, an in-place sorting algorithm for finding the Kth
   * element in an array, where all elements to the left of K are
   * smaller than K, and all elements to the right are larger.
   */

  l = 1;
  ir = values->nelts - 1;

  while (TRUE) {
    if (ir <= l+1) {
      if (ir == l+1 &&
          elts[ir] < elts[l])
        delay_swap(elts[l], elts[ir]);

      return elts[k];

    } else {
      unsigned long i, j;
      long p;
      unsigned long mid = (l + ir) >> 1;

      delay_swap(elts[mid], elts[l+1]);
      if (elts[l] > elts[ir])
        delay_swap(elts[l], elts[ir]);

      if (elts[l+1] > elts[ir])
        delay_swap(elts[l+1], elts[ir]);

      if (elts[l] > elts[l+1])
        delay_swap(elts[l], elts[l+1]);

      i = l + 1;
      j = ir;
      p = elts[l+1];

      while (TRUE) {
        do i++;
          while (i < nelts && elts[i] < p);

        do j--;
          while (j >= 0 && elts[j] > p);

        if (j < i)
          break;

        delay_swap(elts[i], elts[j]);
      }

      elts[l+1] = elts[j];
      elts[j] = p;

      if (p >= k)
        ir = j - 1;

      if (p <= k)
        l = i;

      if (l >= (nelts - 1) ||
          ir >= nelts)
        break;
    }
  }

  return -1;
}

static long delay_get_median(pool *p, unsigned int rownum, long interval) {
  register unsigned int i;
  struct delay_rec *row;
  long *tab_vals;
  array_header *list = make_array(p, 1, sizeof(long));
  
  /* Calculate the median value of the current command's recorded values.
   *
   * When calculating the median, we use the current interval as well
   * as the recorded intervals in the table, giving us an odd number of
   * values.
   *
   * If the number of values in this row is less than the watermark
   * value, we'll actually return the maximum value, rather than the
   * median.  Below the watermark, the server is "cold" and has not
   * yet accumulated enough data to make the median a useful value.
   */

  row = &((struct delay_rec *) delay_tab.dt_data)[rownum];
  tab_vals = row->d_vals;

  /* Start at the end of the row and work backward, as values are
   * always added at the end of the row, shifting everything to the left.
   */
  for (i = 1; i < row->d_nvals; i++)
    *((long *) push_array(list)) = tab_vals[DELAY_NVALUES - i];
  *((long *) push_array(list)) = interval;

  pr_log_debug(DEBUG6, MOD_DELAY_VERSION
    ": selecting median interval from %d %s", list->nelts,
    list->nelts != 1 ? "values" : "value");
  return delay_select_k(((list->nelts + 1) / 2), list);
}

static void delay_mask_signals(unsigned char block) {
  static sigset_t mask_sigset;

  if (block) {
    sigemptyset(&mask_sigset);

    sigaddset(&mask_sigset, SIGCHLD);
    sigaddset(&mask_sigset, SIGUSR1);
    sigaddset(&mask_sigset, SIGINT);
    sigaddset(&mask_sigset, SIGQUIT);
    sigaddset(&mask_sigset, SIGALRM);
#ifdef SIGIO
    sigaddset(&mask_sigset, SIGIO);
#endif
#ifdef SIGBUS
    sigaddset(&mask_sigset, SIGBUS);
#endif
    sigaddset(&mask_sigset, SIGHUP);

    sigprocmask(SIG_BLOCK, &mask_sigset, NULL);

  } else
    sigprocmask(SIG_UNBLOCK, &mask_sigset, NULL);
}

static void delay_signals_block(void) {
  delay_mask_signals(TRUE);
}

static void delay_signals_unblock(void) {
  delay_mask_signals(FALSE);
}

static void delay_delay(long interval) {
  struct timeval tv;
  long rand_usec;

  /* Add an additional delay of a random number of usecs, with a 
   * maximum of half of the given interval.
   */
  rand_usec = ((interval / 2.0) * rand()) / RAND_MAX;
  interval += rand_usec;
  pr_log_debug(DEBUG10, MOD_DELAY_VERSION
    ": additional random delay of %ld usecs added", (long int) rand_usec);

  tv.tv_sec = interval / 1000000;
  tv.tv_usec = interval % 1000000;

  pr_log_debug(DEBUG0, MOD_DELAY_VERSION ": delaying for %ld usecs",
    (long int) ((tv.tv_sec * 1000000) + tv.tv_usec));

  delay_signals_block();
  (void) select(0, NULL, NULL, NULL, &tv);
  delay_signals_unblock();
}

static void delay_table_add_interval(unsigned int rownum, long interval) {
  struct delay_rec *row;

  row = &((struct delay_rec *) delay_tab.dt_data)[rownum];

  /* Shift all existing values to the left one position. */
  memmove(&(row->d_vals[0]), &(row->d_vals[1]),
    sizeof(long) * (DELAY_NVALUES - 1));

  /* Add the given value to the end. */
  row->d_vals[DELAY_NVALUES-1] = interval;
  if (row->d_nvals < DELAY_NVALUES)
    row->d_nvals++;
}

static int delay_table_init(void) {
  pr_fh_t *fh;
  struct stat st;
  server_rec *s;
  unsigned int nservers = 0;
  off_t tab_size;
  int flags = O_RDWR|O_CREAT;
  int reset_table = FALSE;

  /* We only want to create the table if it does not already exist.
   *
   * If the ServerType is inetd, we want to leave the current contents
   * alone (don't we want to check to see that it's appropriate, sid-wise,
   * for the current server_list?), otherwse, we reset the table.
   *
   * The size of the table should be:
   *
   *  number of vhosts * 2 * row size
   */

  for (s = (server_rec *) server_list->xas_list; s; s = s->next)
    nservers++;
  tab_size = nservers * 2 * sizeof(struct delay_rec);

  PRIVS_ROOT
  fh = pr_fsio_open(delay_tab.dt_path, flags);
  PRIVS_RELINQUISH

  if (!fh) {
    pr_log_debug(DEBUG0, MOD_DELAY_VERSION
      ": error opening DelayTable '%s': %s", delay_tab.dt_path,
      strerror(errno));
    return -1;
  }

  if (pr_fsio_fstat(fh, &st) < 0) {
    pr_log_debug(DEBUG0, MOD_DELAY_VERSION
      ": error stat'ing DelayTable '%s': %s", delay_tab.dt_path,
      strerror(errno));
    return -1;
  }

  if (st.st_size != tab_size) {
    /* This check is for cases when the ServerType is inetd, and the
     * current DelayTable has the wrong size, which can happen if the
     * configuration has changed by having vhosts added or removed.
     */

    pr_log_debug(DEBUG2, MOD_DELAY_VERSION
      ": expected table size %" PR_LU ", found %" PR_LU ", resetting table",
      (pr_off_t) tab_size, (pr_off_t) st.st_size);
    reset_table = TRUE;
  }

  if (reset_table) {
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = 0;
    lock.l_start = 0;
    lock.l_len = 0;

    pr_log_debug(DEBUG10, MOD_DELAY_VERSION ": write-locking DelayTable '%s'",
      fh->fh_path);
    while (fcntl(fh->fh_fd, F_SETLKW, &lock) < 0) {
      if (errno == EINTR) {
        pr_signals_handle();
        continue;

      } else {
        pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
          "warning: unable to obtain write lock on DelayTable '%s': %s",
          fh->fh_path, strerror(errno));
        pr_fsio_close(fh);
        return -1;
      } 
    } 

    /* Seek to the desired table size (actually, one byte less than the
     * desired size) and write a single byte, so that there's enough
     * allocated backing store on the filesystem to support the ensuing
     * mmap() call.
     */
    lseek(fh->fh_fd, tab_size-1, SEEK_SET);
    write(fh->fh_fd, "", 1);

    /* Truncate the table, in case we're shrinking an existing table. */
    pr_fsio_ftruncate(fh, tab_size);

    lock.l_type = F_UNLCK;

    pr_log_debug(DEBUG10, MOD_DELAY_VERSION ": unlocking DelayTable '%s'",
      fh->fh_path);
    fcntl(fh->fh_fd, F_SETLK, &lock);
  }

  delay_tab.dt_fd = fh->fh_fd;
  delay_tab.dt_size = tab_size;
  delay_tab.dt_data = mmap(NULL, delay_tab.dt_size, PROT_READ|PROT_WRITE,
    MAP_SHARED, delay_tab.dt_fd, 0);

  if (delay_tab.dt_data == MAP_FAILED) {
    pr_log_pri(PR_LOG_ERR, MOD_DELAY_VERSION
      ": error mapping DelayTable '%s' into memory: %s", delay_tab.dt_path,
      strerror(errno));
    pr_fsio_close(fh);
    return -1;
  }

  if (!reset_table) {
    struct delay_rec *row;

    for (s = (server_rec *) server_list->xas_list; s; s = s->next) {
      unsigned int i = s->sid - 1;

      /* Row for USER values */
      row = &((struct delay_rec *) delay_tab.dt_data)[i];
      if (strcmp(pr_netaddr_get_ipstr(s->addr), row->d_addr) != 0) {
        reset_table = TRUE;
        break;
      }

      if (s->ServerPort != row->d_port) {
        reset_table = TRUE;
        break;
      }

      /* Row for PASS values */
      row = &((struct delay_rec *) delay_tab.dt_data)[i + 1];
      if (strcmp(pr_netaddr_get_ipstr(s->addr), row->d_addr) != 0) {
        reset_table = TRUE;
        break;
      }

      if (s->ServerPort != row->d_port) {
        reset_table = TRUE;
        break;
      }
    }
  }

  if (reset_table) {
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = 0;
    lock.l_start = 0;
    lock.l_len = 0;

    pr_log_debug(DEBUG10, MOD_DELAY_VERSION ": write-locking DelayTable '%s'",
      fh->fh_path);
    while (fcntl(fh->fh_fd, F_SETLKW, &lock) < 0) {
      if (errno == EINTR) {
        pr_signals_handle();
        continue;

      } else {
        pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
          "warning: unable to obtain write lock on DelayTable '%s': %s",
          fh->fh_path, strerror(errno));
        pr_fsio_close(fh);
        return -1;
      }
    }

    /* Seek to the desired table size (actually, one byte less than the
     * desired size) and write a single byte, so that there's enough
     * allocated backing store on the filesystem to support the ensuing
     * mmap() call.
     */
    lseek(fh->fh_fd, tab_size-1, SEEK_SET);
    write(fh->fh_fd, "", 1);

    /* Truncate the table, in case we're shrinking an existing table. */
    pr_fsio_ftruncate(fh, tab_size);

    pr_log_debug(DEBUG6, MOD_DELAY_VERSION ": resetting DelayTable '%s'",
      delay_tab.dt_path);
    delay_table_reset();

    lock.l_type = F_UNLCK;

    pr_log_debug(DEBUG10, MOD_DELAY_VERSION ": unlocking DelayTable '%s'",
      fh->fh_path);
    fcntl(fh->fh_fd, F_SETLK, &lock);
  }

  /* Done */
  munmap(delay_tab.dt_data, delay_tab.dt_size);

  delay_tab.dt_data = NULL;

  if (pr_fsio_close(fh) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      ": error closing DelayTable '%s': %s", delay_tab.dt_path,
      strerror(errno));
    return -1;
  }

  delay_tab.dt_fd = -1;
  return 0;
}

static int delay_table_load(int lock_table) {

  if (lock_table) {
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = 0;
    lock.l_start = 0;
    lock.l_len = 0;

    pr_log_debug(DEBUG10, MOD_DELAY_VERSION ": write-locking DelayTable '%s'",
      delay_tab.dt_path);
    while (fcntl(delay_tab.dt_fd, F_SETLKW, &lock) < 0) {
      if (errno == EINTR) {
        pr_signals_handle();
        continue;

     } else
       return -1;
    }
  }

  if (!delay_tab.dt_data) {
    delay_tab.dt_data = mmap(NULL, delay_tab.dt_size, PROT_READ|PROT_WRITE,
      MAP_SHARED, delay_tab.dt_fd, 0);

    if (delay_tab.dt_data == MAP_FAILED)
      return -1;
  }

  return 0;
}

static void delay_table_reset(void) {
  server_rec *s;

  for (s = (server_rec *) server_list->xas_list; s; s = s->next) {
    unsigned int i = s->sid - 1;

    /* Row for USER values */
    struct delay_rec *row = &((struct delay_rec *) delay_tab.dt_data)[i];
    row->d_sid = s->sid;
    sstrncpy(row->d_addr, pr_netaddr_get_ipstr(s->addr), sizeof(row->d_addr));
    row->d_port = s->ServerPort;
    row->d_nvals = 0;
    memset(row->d_vals, 0, sizeof(row->d_vals));

    /* Row for PASS values */
    row = &((struct delay_rec *) delay_tab.dt_data)[i + 1];
    row->d_sid = s->sid;
    sstrncpy(row->d_addr, pr_netaddr_get_ipstr(s->addr), sizeof(row->d_addr));
    row->d_port = s->ServerPort;
    row->d_nvals = 0;
    memset(row->d_vals, 0, sizeof(row->d_vals));
  }

  return;
}

static int delay_table_wlock(unsigned int rownum) {
  struct flock lock;

  lock.l_type = F_WRLCK;
  lock.l_whence = 0;
  lock.l_start = sizeof(struct delay_rec) * rownum;
  lock.l_len = sizeof(struct delay_rec);

  pr_log_debug(DEBUG10, MOD_DELAY_VERSION
    ": write-locking DelayTable '%s', row %u", delay_tab.dt_path, rownum + 1);
  while (fcntl(delay_tab.dt_fd, F_SETLKW, &lock) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    pr_log_debug(DEBUG2, MOD_DELAY_VERSION
       ": error locking table: %s", strerror(errno));
    return -1;
  }

  return 0;
}

static int delay_table_unload(int unlock_table) {

  if (delay_tab.dt_data) {
    munmap(delay_tab.dt_data, delay_tab.dt_size);
    delay_tab.dt_data = NULL;
  }

  if (unlock_table) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    pr_log_debug(DEBUG10, MOD_DELAY_VERSION ": unlocking DelayTable '%s'",
      delay_tab.dt_path);
    while (fcntl(delay_tab.dt_fd, F_SETLK, &lock) < 0) {
      if (errno == EINTR) {
        pr_signals_handle();
        continue;

      } else
        return -1;
    }
  }

  return 0;
}

static int delay_table_unlock(unsigned int rownum) {
  struct flock lock;

  lock.l_type = F_UNLCK;
  lock.l_whence = 0;
  lock.l_start = sizeof(struct delay_rec) * rownum;
  lock.l_len = sizeof(struct delay_rec);

  pr_log_debug(DEBUG10, MOD_DELAY_VERSION
    ": unlocking DelayTable '%s', row %u", delay_tab.dt_path, rownum + 1);
  while (fcntl(delay_tab.dt_fd, F_SETLKW, &lock) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
       ": error unlocking table: %s", strerror(errno));
    return -1;
  }

  return 0;
}

/* Configuration handlers
 */

/* usage: DelayEngine on|off */
MODRET set_delayengine(cmd_rec *cmd) {
  config_rec *c;
  int bool;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = bool;

  return HANDLED(cmd);
}

/* usage: DelayTable path */
MODRET set_delaytable(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (pr_fs_valid_path(cmd->argv[1]) < 0)
    CONF_ERROR(cmd, "must be an absolute path");

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return HANDLED(cmd);
}

/* Command handlers
 */

MODRET delay_post_pass(cmd_rec *cmd) {
  struct timeval tv;
  unsigned int rownum;
  long interval, median;

  if (!delay_engine)
    return DECLINED(cmd);

  /* We use sid-1, since the sid is a server number, and the locking
   * routines want a row index.  However, PASS rows are always after
   * USER rows, so we need to add 1 to the row number, leaving us
   * with just the sid.
   */
  rownum = main_server->sid;

  /* Prepare for manipulating the table. */
  if (delay_table_load(FALSE) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      "warning: unable to load DelayTable '%s' into memory: %s",
      delay_tab.dt_path, strerror(errno));
    return DECLINED(cmd);
  }

  delay_table_wlock(rownum);

  gettimeofday(&tv, NULL);

  interval = (tv.tv_sec - delay_tv.tv_sec) * 1000000 +
    (tv.tv_usec - delay_tv.tv_usec);

  /* Get the median interval value. */
  median = delay_get_median(cmd->tmp_pool, rownum, interval);

  /* Add the interval to the table. Only allow a single session to
   * add a portion of the cache size, to prevent a single client from
   * poisoning the cache.
   */
  if (delay_npass < (DELAY_NVALUES / DELAY_SESS_NVALUES)) {
    pr_log_debug(DEBUG10, MOD_DELAY_VERSION ": adding %ld usecs to PASS row",
      interval);
    delay_table_add_interval(rownum, interval);
    delay_npass++;

  } else
    /* Generate an event, in case a module (i.e. mod_ban) might want
     * to do something appropriate to such an ill-behaved client.
     */
    pr_event_generate("mod_delay.max-pass", session.c);

  /* Done with the table. */
  delay_table_unlock(rownum);
  if (delay_table_unload(FALSE) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      "warning: unable to unload DelayTable '%s' from memory: %s",
      delay_tab.dt_path, strerror(errno));
  }

  /* If the current interval is less than the median interval, we
   * need to delay ourselves a little.
   */
  if (interval < median)
    delay_delay(median - interval);

  return DECLINED(cmd);
}

MODRET delay_pre_pass(cmd_rec *cmd) {
  if (!delay_engine)
    return DECLINED(cmd);

  gettimeofday(&delay_tv, NULL);
  return DECLINED(cmd);
}

MODRET delay_post_user(cmd_rec *cmd) {
  struct timeval tv;
  unsigned int rownum;
  long interval, median;

  if (!delay_engine)
    return DECLINED(cmd);

  /* We use sid-1, since the sid is a server number, and the locking
   * routines want a row index.
   */
  rownum = main_server->sid - 1;

  /* Prepare for manipulating the table. */
  if (delay_table_load(FALSE) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      "warning: unable to load DelayTable '%s' into memory: %s",
      delay_tab.dt_path, strerror(errno));
    return DECLINED(cmd);
  }

  delay_table_wlock(rownum);

  gettimeofday(&tv, NULL);

  interval = (tv.tv_sec - delay_tv.tv_sec) * 1000000 +
    (tv.tv_usec - delay_tv.tv_usec);

  /* Get the median interval value. */
  median = delay_get_median(cmd->tmp_pool, rownum, interval);

  /* Add the interval to the table. Only allow a single session to
   * add a portion of the cache size, to prevent a single client from
   * poisoning the cache.
   */
  if (delay_nuser < (DELAY_NVALUES / DELAY_SESS_NVALUES)) {
    pr_log_debug(DEBUG10, MOD_DELAY_VERSION ": adding %ld usecs to USER row",
      interval);
    delay_table_add_interval(rownum, interval);
    delay_nuser++;

  } else
    /* Generate an event, in case a module (i.e. mod_ban) might want
     * to do something appropriate to such an ill-behaved client.
     */
    pr_event_generate("mod_delay.max-user", session.c);

  /* Done with the table. */
  delay_table_unlock(rownum);
  if (delay_table_unload(FALSE) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      "warning: unable to unload DelayTable '%s' from memory: %s",
      delay_tab.dt_path, strerror(errno));
  }

  /* If the current interval is less than the median interval, we
   * need to delay ourselves a little.
   */
  if (interval < median)
    delay_delay(median - interval);

  return DECLINED(cmd);
}

MODRET delay_pre_user(cmd_rec *cmd) {
  if (!delay_engine)
    return DECLINED(cmd);

  gettimeofday(&delay_tv, NULL);
  return DECLINED(cmd);
}

/* Event handlers
 */

static void delay_exit_ev(const void *event_data, void *user_data) {
  pr_fh_t *fh;

  if (!delay_engine)
    return;

  /* Write out the DelayTable to the filesystem, thus updating the
   * file metadata.
   */

  PRIVS_ROOT
  fh = pr_fsio_open(delay_tab.dt_path, O_RDWR);
  PRIVS_RELINQUISH

  if (!fh) {
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      ": warning: unable to open DelayTable '%s': %s", delay_tab.dt_path,
      strerror(errno));
    return;
  }

  delay_tab.dt_fd = fh->fh_fd;
  delay_tab.dt_data = NULL;

  /* Load the DelayTable into memory. */
  if (delay_table_load(TRUE) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      "warning: unable to load DelayTable '%s' into memory: %s",
      delay_tab.dt_path, strerror(errno));
    pr_fsio_close(fh);
    return;
  }

  if (pr_fsio_write(fh, delay_tab.dt_data, delay_tab.dt_size) < 0)
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      ": warning: error updating DelayTable '%s': %s", delay_tab.dt_path,
      strerror(errno));

  /* Unload the DelayTable from memory. */
  if (delay_table_unload(TRUE) < 0) {
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      ": warning: error unloading DelayTable '%s' from memory: %s",
      delay_tab.dt_path, strerror(errno));
  }

  if (pr_fsio_close(fh) < 0)
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      ": warning: error writing DelayTable '%s': %s", delay_tab.dt_path,
      strerror(errno));

  return;
}

static void delay_postparse_ev(const void *event_data, void *user_data) {
  config_rec *c;

  c = find_config(main_server->conf, CONF_PARAM, "DelayEngine", FALSE);
  if (c && *((unsigned int *) c->argv[0]) == FALSE)
    delay_engine = FALSE;

  if (!delay_engine)
    return;

  c = find_config(main_server->conf, CONF_PARAM, "DelayTable", FALSE);
  if (c)
    delay_tab.dt_path = c->argv[0];

  (void) delay_table_init();
  return;
}

/* Initialization functions
 */

static int delay_init(void) {
  delay_tab.dt_path = PR_RUN_DIR "/proftpd.delay";
  delay_tab.dt_data = NULL;

  pr_event_register(&delay_module, "core.exit", delay_exit_ev, NULL);
  pr_event_register(&delay_module, "core.postparse", delay_postparse_ev, NULL);
  return 0;
}

static int delay_sess_init(void) {
  pr_fh_t *fh;
  config_rec *c;

  pr_event_unregister(&delay_module, "core.exit", delay_exit_ev);

  if (!delay_engine)
    return 0;

  /* Look up DelayEngine again, as it may have been disabled in an
   * <IfClass> section.
   */
  c = find_config(main_server->conf, CONF_PARAM, "DelayEngine", FALSE);
  if (c && *((unsigned int *) c->argv[0]) == FALSE)
    delay_engine = FALSE;

  if (!delay_engine)
    return 0;

  delay_nuser = 0;
  delay_npass = 0;

  pr_log_debug(DEBUG6, MOD_DELAY_VERSION ": opening DelayTable '%s'",
    delay_tab.dt_path);

  PRIVS_ROOT
  fh = pr_fsio_open(delay_tab.dt_path, O_RDWR);
  PRIVS_RELINQUISH

  if (!fh) {
    pr_log_pri(PR_LOG_WARNING, MOD_DELAY_VERSION
      ": warning: unable to open DelayTable '%s': %s", delay_tab.dt_path,
      strerror(errno));
    delay_engine = FALSE;
    return 0;
  }

  delay_tab.dt_fd = fh->fh_fd;
  delay_tab.dt_data = NULL;

  return 0;  
}

/* Module API tables
 */

static conftable delay_conftab[] = {
  { "DelayEngine",	set_delayengine,	NULL },
  { "DelayTable",	set_delaytable,		NULL },
  { NULL }
};

static cmdtable delay_cmdtab[] = {
  { PRE_CMD,		C_PASS,	G_NONE,	delay_pre_pass,		FALSE, FALSE },
  { POST_CMD,		C_PASS,	G_NONE,	delay_post_pass,	FALSE, FALSE },
  { POST_CMD_ERR,	C_PASS,	G_NONE,	delay_post_pass,	FALSE, FALSE },
  { PRE_CMD,		C_USER,	G_NONE,	delay_pre_user,		FALSE, FALSE },
  { POST_CMD,		C_USER,	G_NONE,	delay_post_user,	FALSE, FALSE },
  { POST_CMD_ERR,	C_USER,	G_NONE,	delay_post_user,	FALSE, FALSE },
  { 0, NULL }
};

module delay_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "delay",

  /* Module configuration handler table */
  delay_conftab,

  /* Module command handler table */
  delay_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  delay_init,

  /* Session initialization function */
  delay_sess_init,

  /* Module version */
  MOD_DELAY_VERSION
};
