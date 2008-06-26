/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2003 The ProFTPD Project team
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
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* ProFTPD xferlog(5) logging support.
 * $Id: xferlog.c,v 1.3 2005/06/14 01:23:45 castaglia Exp $
 */

#include "conf.h"

#define LOGBUFFER_SIZE	2048

static int xferlogfd = -1;

void xferlog_close(void) {
  if (xferlogfd != -1)
    close(xferlogfd);

  xferlogfd = -1;
}

int xferlog_open(const char *path) {

  if (!path) {
    if (xferlogfd != -1)
      xferlog_close();

    return 0;
  }

  if (xferlogfd == -1) {
    pr_log_debug(DEBUG6, "opening TransferLog '%s'", path);
    pr_log_openfile(path, &xferlogfd, PR_LOG_XFER_MODE);
  }

  return xferlogfd;
}

int xferlog_write(long xfertime, const char *remhost, off_t fsize, char *fname,
    char xfertype, char direction, char access_mode, char *user,
    char abort_flag) {

  char buf[LOGBUFFER_SIZE] = {'\0'}, fbuf[LOGBUFFER_SIZE] = {'\0'};
  register unsigned int i = 0;

  if (xferlogfd == -1 || !remhost || !user || !fname)
    return 0;

  for (i = 0; (i + 1 < sizeof(fbuf)) && fname[i] != '\0'; i++) {
    fbuf[i] = (isspace((int) fname[i]) || iscntrl((int) fname[i])) ? '_' :
      fname[i];
  }
  fbuf[i] = '\0';

  snprintf(buf, sizeof(buf),
    "%s %ld %s %" PR_LU " %s %c _ %c %c %s ftp %c %s %c\n",
      pr_strtime(time(NULL)),
      xfertime,
      remhost,
      (pr_off_t) fsize,
      fbuf,
      xfertype,
      direction,
      access_mode,
      user,
      session.ident_lookups == TRUE ? '1' : '0',
      (session.ident_lookups == TRUE && strcmp(session.ident_user,
        "UNKNOWN")) ? session.ident_user : "*",
      abort_flag);

  buf[sizeof(buf)-1] = '\0';

  return write(xferlogfd, buf, strlen(buf));
}
