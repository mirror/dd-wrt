/*
 * ProFTPD - FTP server API testsuite
 * Copyright (c) 2008-2011 The ProFTPD Project team
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
module *static_modules[] = { NULL };
module *loaded_modules = NULL;

int pr_ctrls_unregister(module *m, const char *action) {
  return 0;
}

char *pr_fs_decode_path(pool *p, const char *path) {
  return (char *) path;
}

char *pr_fs_encode_path(pool *p, const char *path) {
  return (char *) path;
}

void pr_log_debug(int level, const char *fmt, ...) {
}

void pr_log_pri(int prio, const char *fmt, ...) {
}

int pr_netio_printf(pr_netio_stream_t *strm, const char *fmt, ...) {
  errno = ENOSYS;
  return -1;
}

int pr_netio_printf_async(pr_netio_stream_t *strm, char *fmt, ...) {
  errno = ENOSYS;
  return -1;
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
  return 0;
}
