/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2006-2010 The ProFTPD Project team
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
 * As a special exemption, the ProFTPD Project and other respective copyright
 * holders give permission to link this program with OpenSSL, and distribute
 * the resulting executable, without including the source code for OpenSSL
 * in the source distribution.
 */

/* Trace functions
 * $Id: trace.c,v 1.26 2010/02/10 20:54:29 castaglia Exp $
 */


#include "conf.h"
#include "privs.h"

#ifdef PR_USE_TRACE

static int trace_logfd = -1;
static pool *trace_pool = NULL;
static pr_table_t *trace_tab = NULL;

static const char *trace_channels[] = {
  "auth",
  "binding",
  "command",
  "config",
  "ctrls",
  "data",
  "delay",
  "dns",
  "dso",
  "encode",
  "event",
  "facl",
  "fsio",
  "ident",
  "inet",
  "lock",
  "netacl",
  "netio",
  "pam",
  "pool",
  "regexp",
  "response",
  "signal",
  "site",
  "timer",
  "var",
  "xfer",
  NULL
};

static void trace_restart_ev(const void *event_data, void *user_data) {
  close(trace_logfd);
  trace_logfd = -1;

  if (trace_pool) {
    destroy_pool(trace_pool);
    trace_pool = NULL;
    trace_tab = NULL;

    pr_event_unregister(NULL, "core.restart", trace_restart_ev);
  }

  return;
}

static int trace_write(const char *channel, int level, const char *msg) {
  char buf[PR_TUNABLE_BUFFER_SIZE];
  size_t buflen;
  time_t now;
  struct tm *t;

  if (trace_logfd < 0)
    return 0;

  now = time(NULL);
  t = pr_localtime(NULL, &now);

  memset(buf, '\0', sizeof(buf));
  strftime(buf, sizeof(buf), "%b %d %H:%M:%S", t);
  buf[sizeof(buf)-1] = '\0';

  snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
    " [%u] <%s:%d>: %s",
    (unsigned int) (session.pid ? session.pid : getpid()), channel, level, msg);

  buf[sizeof(buf)-1] = '\0';

  buflen = strlen(buf);
  if (buflen < (sizeof(buf) - 1)) {
    buf[buflen] = '\n';

  } else {
    buf[sizeof(buf)-2] = '\n';
  }

  return write(trace_logfd, buf, strlen(buf));
}

pr_table_t *pr_trace_get_table(void) {
  if (!trace_tab) {
    errno = EPERM;
    return NULL;
  }

  return trace_tab;
}

int pr_trace_get_level(const char *channel) {
  int level;
  void *value;

  if (!channel) {
    errno = EINVAL;
    return -1;
  }

  if (!trace_tab ||
      trace_logfd < 0) {
    errno = EPERM;
    return -1;
  }

  value = pr_table_get(trace_tab, channel, NULL);
  if (!value) {
    errno = ENOENT;
    return -1;
  }

  memcpy(&level, value, sizeof(int));
  return level;
}

int pr_trace_set_file(const char *path) {
  int res;

  if (!path) {
    if (trace_logfd < 0) {
      errno = EINVAL;
      return -1;
    }

    (void) close(trace_logfd);
    trace_logfd = -1;
    return 0;
  }

  pr_signals_block();
  PRIVS_ROOT
  res = pr_log_openfile(path, &trace_logfd, 0660);
  PRIVS_RELINQUISH
  pr_signals_unblock();

  if (res < 0) {
    if (res == -1) {
      pr_log_debug(DEBUG1, "unable to open TraceLog '%s': %s", path,
        strerror(errno));

    } else if (res == PR_LOG_WRITABLE_DIR) {
      pr_log_debug(DEBUG1,
        "unable to open TraceLog '%s': parent directory is world-writable",
        path);

    } else if (res == PR_LOG_SYMLINK) {
      pr_log_debug(DEBUG1,
        "unable to open TraceLog '%s': cannot log to a symbolic link",
        path);
    }

    return res;
  }

  return 0;
}

int pr_trace_set_level(const char *channel, int level) {

  if (!channel) {
    errno = EINVAL;
    return -1;
  }

  if (!trace_tab &&
      level < 0)
    return 0;

  if (!trace_pool) {
    trace_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(trace_pool, "Trace API");

    trace_tab = pr_table_alloc(trace_pool, 0);

    /* Register a handler for churning the log pool during HUP. */
    pr_event_register(NULL, "core.restart", trace_restart_ev, NULL);
  }

  if (level >= 0) {
    void *value = palloc(trace_pool, sizeof(int));
    memcpy(value, &level, sizeof(int));

    if (strcmp(channel, PR_TRACE_DEFAULT_CHANNEL) != 0) {
      int count = pr_table_exists(trace_tab, channel);

      if (count <= 0) {
        if (pr_table_add(trace_tab, pstrdup(trace_pool, channel), value,
            sizeof(int)) < 0) {
          return -1;
        }

      } else {
        if (pr_table_set(trace_tab, pstrdup(trace_pool, channel), value,
            sizeof(int)) < 0)
          return -1;
      }

    } else {
      register unsigned int i;

      for (i = 0; trace_channels[i]; i++) {
        (void) pr_trace_set_level(trace_channels[i], level);
      }
    }

  } else {
    if (strcmp(channel, PR_TRACE_DEFAULT_CHANNEL) != 0) {
      (void) pr_table_remove(trace_tab, channel, NULL);

    } else {
      register unsigned int i;

      for (i = 0; trace_channels[i]; i++) {
        (void) pr_table_remove(trace_tab, trace_channels[i], NULL);
      }
    }
  }

  return 0;
}

int pr_trace_msg(const char *channel, int level, const char *fmt, ...) {
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  size_t buflen;
  va_list msg;
  int res;

  if (!channel ||
      !fmt ||
      level < 0) {
    errno = EINVAL;
    return -1;
  }

  if (!trace_tab) {
    errno = EPERM;
    return -1;
  }

  res = pr_trace_get_level(channel);
  if (res < 0)
    return -1;

  if (res < level)
    return 0;

  va_start(msg, fmt);
  vsnprintf(buf, sizeof(buf), fmt, msg);
  va_end(msg);

  /* Always make sure the buffer is NUL-terminated. */
  buf[sizeof(buf)-1] = '\0';

  /* Trim trailing newlines. */
  buflen = strlen(buf);
  while (buflen >= 1 &&
         buf[buflen-1] == '\n') {
    pr_signals_handle();
    buf[buflen-1] = '\0';
    buflen = strlen(buf);
  }

  return trace_write(channel, level, buf);
}

#else

pr_table_t *pr_trace_get_table(void) {
  errno = ENOSYS;
  return NULL;
}

int pr_trace_get_level(const char *channel) {
  errno = ENOSYS;
  return -1;
}

int pr_trace_set_file(const char *path) {
  errno = ENOSYS;
  return -1;
}

int pr_trace_set_level(const char *channel, int level) {
  errno = ENOSYS;
  return -1;
}

int pr_trace_msg(const char *channel, int level, const char *fmt, ...) {
  errno = ENOSYS;
  return -1;
}

#endif /* PR_USE_TRACE */
