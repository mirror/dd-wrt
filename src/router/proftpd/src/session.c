/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2009 The ProFTPD Project team
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
 *
 * $Id: session.c,v 1.6 2009/09/14 20:44:12 castaglia Exp $
 */

#include "conf.h"

const char *pr_session_get_protocol(int flags) {
  const char *sess_proto;

  sess_proto = pr_table_get(session.notes, "protocol", NULL);
  if (sess_proto == NULL) {
    sess_proto = "ftp";
  }

  if (!(flags & PR_SESS_PROTO_FL_LOGOUT)) {
    /* Return the protocol as is. */
    return sess_proto;
  }

  /* Otherwise, we need to return either "FTP" or "SSH2", for consistency. */
  if (strcmp(sess_proto, "ftp") == 0 ||
      strcmp(sess_proto, "ftps") == 0) {
    return "FTP";
  
  } else if (strcmp(sess_proto, "ssh2") == 0 ||
             strcmp(sess_proto, "sftp") == 0 ||
             strcmp(sess_proto, "scp") == 0 ||
             strcmp(sess_proto, "publickey") == 0) {
    return "SSH2";
  }

  /* Should never reach here, but just in case... */
  return "unknown";
}

int pr_session_set_idle(void) {
  char *user = NULL;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_BEGIN_IDLE, time(NULL),
    PR_SCORE_CMD, "%s", "idle", NULL, NULL);

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", "", NULL, NULL);

  if (session.user) {
    user = session.user;

  } else {
    user = "(authenticating)";
  }

  pr_proctitle_set("%s - %s: IDLE", user, session.proc_prefix);
  return 0;
}

int pr_session_set_protocol(const char *sess_proto) {
  int count, res;

  if (sess_proto == NULL) {
    errno = EINVAL;
    return -1;
  }

  count = pr_table_exists(session.notes, "protocol");
  if (count > 0) {
    res = pr_table_set(session.notes, pstrdup(session.pool, "protocol"),
      pstrdup(session.pool, sess_proto), 0);

    if (res == 0) {
      /* Update the scoreboard entry for this session with the protocol. */
      pr_scoreboard_entry_update(session.pid, PR_SCORE_PROTOCOL, sess_proto,
        NULL);
    }

    return res;
  }

  res = pr_table_add(session.notes, pstrdup(session.pool, "protocol"),
    pstrdup(session.pool, sess_proto), 0);

  if (res == 0) {
    /* Update the scoreboard entry for this session with the protocol. */
    pr_scoreboard_entry_update(session.pid, PR_SCORE_PROTOCOL, sess_proto,
      NULL);
  }

  return res;
}

static const char *sess_ttyname = NULL;

const char *pr_session_get_ttyname(pool *p) {
  char ttybuf[32];
  const char *sess_proto, *tty_proto = NULL;

  if (p == NULL) {
    errno = EINVAL;
    return NULL;
  }

  if (sess_ttyname) {
    /* Return the cached name. */
    return pstrdup(p, sess_ttyname);
  }

  sess_proto = pr_table_get(session.notes, "protocol", NULL);
  if (sess_proto) {
    if (strcmp(sess_proto, "ftp") == 0 ||
        strcmp(sess_proto, "ftps") == 0) {
#if (defined(BSD) && (BSD >= 199103))
      tty_proto = "ftp";
#else
      tty_proto = "ftpd";
#endif

    } else if (strcmp(sess_proto, "ssh2") == 0 ||
               strcmp(sess_proto, "sftp") == 0 ||
               strcmp(sess_proto, "scp") == 0 ||
               strcmp(sess_proto, "publickey") == 0) {

      /* Just use the plain "ssh" string for the tty name for these cases. */
      tty_proto = "ssh";

      /* Cache the originally constructed tty name for any later retrievals. */
      sess_ttyname = pstrdup(session.pool, tty_proto);
      return pstrdup(p, sess_ttyname);
    }
  }

  if (tty_proto == NULL) {
#if (defined(BSD) && (BSD >= 199103))
    tty_proto = "ftp";
#else
    tty_proto = "ftpd";
#endif
  }

  memset(ttybuf, '\0', sizeof(ttybuf));
#if (defined(BSD) && (BSD >= 199103))
  snprintf(ttybuf, sizeof(ttybuf), "%s%ld", tty_proto,
    (long) (session.pid ? session.pid : getpid()));
#else
  snprintf(ttybuf, sizeof(ttybuf), "%s%d", tty_proto,
    (int) (session.pid ? session.pid : getpid()));
#endif

  /* Cache the originally constructed tty name for any later retrievals. */
  sess_ttyname = pstrdup(session.pool, ttybuf);

  return pstrdup(p, sess_ttyname);
}
