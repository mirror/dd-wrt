/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2007-2009 The ProFTPD Project team
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
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/*
 * Pidfile management
 * $Id: pidfile.c,v 1.4 2009/08/23 23:58:56 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"

static const char *pidfile_path = PR_PID_FILE_PATH;

void pr_pidfile_write(void) {
  FILE *fh = NULL;
  const char *path = NULL;

  path = get_param_ptr(main_server->conf, "PidFile", FALSE);
  if (path != NULL &&
      *path) {
    pidfile_path = pstrdup(permanent_pool, path);

  } else {
    path = pidfile_path;
  }

  PRIVS_ROOT
  fh = fopen(path, "w");
  PRIVS_RELINQUISH

  if (fh == NULL) {
    fprintf(stderr, "error opening PidFile '%s': %s\n", path, strerror(errno));
    exit(1);
  }

  fprintf(fh, "%lu\n", (unsigned long) getpid());
  if (fclose(fh) < 0) {
    fprintf(stderr, "error writing PidFile '%s': %s\n", path, strerror(errno));
  }
}

int pr_pidfile_remove(void) {
  return unlink(pidfile_path);
}
