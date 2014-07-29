/*
 * ProFTPD - FTP server API testsuite
 * Copyright (c) 2008-2014 The ProFTPD Project team
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

#include "tests.h"

/* Stubs */

session_t session;

char ServerType = SERVER_STANDALONE;
int ServerUseReverseDNS = 1;
server_rec *main_server = NULL;
pid_t mpid = 1;
module *static_modules[] = { NULL };
module *loaded_modules = NULL;

char *dir_realpath(pool *p, const char *path) {
  return NULL;
}

void *get_param_ptr(xaset_t *set, const char *name, int recurse) {
  errno = ENOENT;
  return NULL;
}

struct passwd *pr_auth_getpwnam(pool *p, const char *name) {
  errno = ENOENT;
  return NULL;
}

int pr_config_get_server_xfer_bufsz(int direction) {
  int bufsz = -1;

  switch (direction) {
    case PR_NETIO_IO_RD:
      bufsz = PR_TUNABLE_DEFAULT_RCVBUFSZ;
      break;

    case PR_NETIO_IO_WR:
      bufsz = PR_TUNABLE_DEFAULT_SNDBUFSZ;
      break;

    default:
      errno = EINVAL;
      return -1;
  }

  return bufsz;
}

int pr_ctrls_unregister(module *m, const char *action) {
  return 0;
}

void pr_log_debug(int level, const char *fmt, ...) {
  if (getenv("TEST_VERBOSE") != NULL) {
    va_list msg;

    fprintf(stderr, "DEBUG%d: ", level);

    va_start(msg, fmt);
    vfprintf(stderr, fmt, msg);
    va_end(msg);

    fprintf(stderr, "\n");
  }
}

void pr_log_pri(int prio, const char *fmt, ...) {
  if (getenv("TEST_VERBOSE") != NULL) {
    va_list msg;

    fprintf(stderr, "PRI%d: ", prio);

    va_start(msg, fmt);
    vfprintf(stderr, fmt, msg);
    va_end(msg);

    fprintf(stderr, "\n");
  }
}

void pr_signals_handle(void) {
}

void pr_signals_block(void) {
}

void pr_signals_unblock(void) {
}

int pr_trace_get_level(const char *channel) {
  return 0;
}

int pr_trace_msg(const char *channel, int level, const char *fmt, ...) {

  if (getenv("TEST_VERBOSE") != NULL) {
    va_list msg;

    fprintf(stderr, "TRACE: <%s:%d>: ", channel, level);

    va_start(msg, fmt);
    vfprintf(stderr, fmt, msg);
    va_end(msg);

    fprintf(stderr, "\n");
  }

  return 0;
}

void run_schedule(void) {
}
