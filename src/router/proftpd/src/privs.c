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
 * As a special exemption, The ProFTPD Project and other respective copyright
 * holders give permission to link this program with OpenSSL, and distribute
 * the resulting executable, without including the source code for OpenSSL in
 * the source distribution.
 *
 * $Id: privs.c,v 1.3 2009/09/16 16:32:41 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"

/* Functions for manipulating saved, real and effective UID for easy switching
 * from/to root.
 *
 * Note: In version 1.1.5, all of this changed.  We USED to play games with
 * the saved-UID/GID AND setreuid()/setregid(); however this appears to be
 * slightly non-portable (i.e. w/ BSDs).  Since POSIX.1 saved-UIDs are pretty
 * much useless without setre* (in the case of root), we now use basic UID
 * swapping if we have seteuid(), and setreuid() swapping if not.
 *
 * If seteuid() is present, we set the saved UID/GID using setuid/seteuid().
 * setreuid() is no longer used as it is considered obsolete on many systems.
 * GIDS are also no longer swapped, as they are unnecessary.
 *
 * If run as root, proftpd now normally runs as:
 *   real user            : root
 *   effective user       : <user>
 *   saved user           : root
 *   real/eff/saved group : <group>
 */

/* Porters, please put the most reasonable and secure method of
 * doing this in here.
 */

static const char *trace_channel = "privs";

/* We keep a count of the number of times PRIVS_ROOT/PRIVS_RELINQUISH have
 * been called.  This allows for nesting calls to PRIVS_ROOT/PRIVS_RELINQUISH,
 * so that the last PRIVS_RELINQUISH call actually releases the privs.
 */
static unsigned int root_privs = 0;
static unsigned int user_privs = 0;

int pr_privs_setup(uid_t uid, gid_t gid, const char *file, int lineno) {
  pr_log_debug(DEBUG9, "SETUP PRIVS at %s:%d", file, lineno);

  /* Reset the user/root privs counters. */
  root_privs = user_privs = 0;
  pr_trace_msg(trace_channel, 9, "PRIVS_SETUP called, "
    "resetting user/root privs count");

  pr_signals_block();

  if (getuid() != PR_ROOT_UID) {
    session.ouid = session.uid = getuid();
    session.gid = getgid();

    if (setgid(session.gid) < 0) {
      pr_log_pri(PR_LOG_ERR, "SETUP PRIVS: unable to setgid(): %s",
        strerror(errno));
    }

#if defined(HAVE_SETEUID)
    if (setuid(session.uid) < 0) {
      pr_log_pri(PR_LOG_ERR, "SETUP PRIVS: unable to setuid(): %s", 
        strerror(errno));
    }

    if (seteuid(session.uid) < 0) {
      pr_log_pri(PR_LOG_ERR, "SETUP PRIVS: unable to seteuid(): %s", 
        strerror(errno));
    }
#else
    if (setreuid(session.uid, session.uid) < 0) {
      pr_log_pri(PR_LOG_ERR, "SETUP PRIVS: unable to setreuid(): %s",
        strerror(errno));
    }
#endif /* !HAVE_SETEUID */

  } else {
    session.ouid = getuid();
    session.uid = uid;
    session.gid = gid;

#if defined(HAVE_SETEUID)
    if (setuid(PR_ROOT_UID) < 0) {
      pr_log_pri(PR_LOG_ERR, "SETUP PRIVS: unable to setuid(): %s", 
        strerror(errno));
    }

    if (setgid(gid) < 0) {
      pr_log_pri(PR_LOG_ERR, "SETUP PRIVS: unable to setgid(): %s", 
        strerror(errno));
    }

    if (seteuid(uid) < 0) {
      pr_log_pri(PR_LOG_ERR, "SETUP PRIVS: unable to seteuid(): %s", 
        strerror(errno));
    }
#else
    if (setgid(session.gid) < 0) {
      pr_log_pri(PR_LOG_ERR, "SETUP PRIVS: unable to setgid(): %s", 
        strerror(errno));
    }

    if (setreuid(PR_ROOT_UID, session.uid) < 0) {
      pr_log_pri(PR_LOG_ERR, "SETUP PRIVS: unable to setreuid(): %s",
        strerror(errno));
    }
#endif /* !HAVE_SETEUID */
  }

  pr_signals_unblock();
  return 0;
}

int pr_privs_root(const char *file, int lineno) {
  pr_log_debug(DEBUG9, "ROOT PRIVS at %s:%d", file, lineno);

  if (root_privs > 0) {
    pr_trace_msg(trace_channel, 9, "root privs count = %u, ignoring PRIVS_ROOT",
      root_privs);
    return 0;
  }

  pr_trace_msg(trace_channel, 9, "root privs count = %u, honoring PRIVS_ROOT",
    root_privs);
  root_privs++;

  pr_signals_block();

  if (!session.disable_id_switching) {

#if defined(HAVE_SETEUID)
    if (seteuid(PR_ROOT_UID) < 0) {
      pr_log_pri(PR_LOG_ERR, "ROOT PRIVS: unable to seteuid(): %s", 
        strerror(errno));
    }

    if (setegid(PR_ROOT_GID) < 0) {
      pr_log_pri(PR_LOG_ERR, "ROOT PRIVS: unable to setegid(): %s", 
        strerror(errno));
    }
#else
    if (setreuid(session.uid, PR_ROOT_UID) < 0) {
      pr_log_pri(PR_LOG_ERR, "ROOT PRIVS: unable to setreuid(): %s",
        strerror(errno));
    }

    if (setregid(session.gid, PR_ROOT_GID)) {
      pr_log_pri(PR_LOG_ERR, "ROOT PRIVS: unable to setregid(): %s",
        strerror(errno));
    }
#endif /* !HAVE_SETEUID */

  } else {
    pr_log_debug(DEBUG9, "ROOT PRIVS: ID switching disabled");
  }

  pr_signals_unblock();
  return 0;
}

int pr_privs_user(const char *file, int lineno) {
  pr_log_debug(DEBUG9, "USER PRIVS %lu at %s:%d",
    (unsigned long) session.login_uid, file, lineno);

  if (user_privs > 0) {
    pr_trace_msg(trace_channel, 9, "user privs count = %u, ignoring PRIVS_USER",
      user_privs);
    return 0;
  }

  pr_trace_msg(trace_channel, 9, "user privs count = %u, honoring PRIVS_USER",
    user_privs);
  user_privs++;

  pr_signals_block();

  if (!session.disable_id_switching) {
#if defined(HAVE_SETEUID)
    if (seteuid(PR_ROOT_UID) < 0) {
      pr_log_pri(PR_LOG_ERR, "USER PRIVS: unable to seteuid(PR_ROOT_UID): %s",
        strerror(errno));
    }

    if (setegid(session.login_gid) < 0) {
      pr_log_pri(PR_LOG_ERR, "USER PRIVS: unable to "
        "setegid(session.login_gid): %s", strerror(errno));
    }

    if (seteuid(session.login_uid) < 0) {
      pr_log_pri(PR_LOG_ERR, "USER PRIVS: unable to "
        "seteuid(session.login_uid): %s", strerror(errno));
    }
#else
    if (setreuid(session.uid, PR_ROOT_UID) < 0) {
      pr_log_pri(PR_LOG_ERR,
        "USER PRIVS: unable to setreuid(session.uid, PR_ROOT_UID): %s",
        strerror(errno));
    }

    if (setregid(session.gid, session.login_gid) < 0) {
      pr_log_pri(PR_LOG_ERR, "USER PRIVS: unable to setregid(session.gid, "
        "session.login_gid): %s", strerror(errno));
    }

    if (setreuid(session.uid, session.login_uid) < 0) {
      pr_log_pri(PR_LOG_ERR, "USER PRIVS: unable to setreuid(session.uid, "
        "session.login_uid): %s", strerror(errno));
    }
#endif /* !HAVE_SETEUID */

  } else {
    pr_log_debug(DEBUG9, "USER PRIVS: ID switching disabled");
  }

  pr_signals_unblock();
  return 0;
}

int pr_privs_relinquish(const char *file, int lineno) {
  pr_log_debug(DEBUG9, "RELINQUISH PRIVS at %s:%d", file, lineno);

  if (root_privs == 0 &&
      user_privs == 0) {
    /* No privs to relinquish here. */
    pr_trace_msg(trace_channel, 9,
      "user/root privs count = 0, ignoring PRIVS_RELINQUISH");
    return 0;
  }

  /* We only want to actually relinquish the privs (user or root) when
   * the nesting count reaches 1.
   */
  if (root_privs + user_privs > 1) {
    pr_trace_msg(trace_channel, 9,
      "root privs count = %u, user privs count = %u, ignoring PRIVS_RELINQUISH",
      root_privs, user_privs);
    return 0;

  } else {
    pr_trace_msg(trace_channel, 9, "root privs count = %u, user privs "
      "count = %u, honoring PRIVS_RELINQUISH", root_privs, user_privs);
  }

  pr_signals_block();

  if (!session.disable_id_switching) {
#if defined(HAVE_SETEUID)
    if (geteuid() != PR_ROOT_UID) {
      if (seteuid(PR_ROOT_UID) < 0) {
        pr_log_pri(PR_LOG_ERR, "RELINQUISH PRIVS: unable to "
          "seteuid(PR_ROOT_UID): %s", strerror(errno));
      }

      if (user_privs > 0)
        user_privs--;

    } else {
      if (root_privs > 0)
        root_privs--;
    }

    if (setegid(session.gid) < 0) {
      pr_log_pri(PR_LOG_ERR, "RELINQUISH PRIVS: unable to "
        "setegid(session.gid): %s", strerror(errno));
    }

    if (seteuid(session.uid) < 0) {
      pr_log_pri(PR_LOG_ERR, "RELINQUISH PRIVS: unable to "
        "seteuid(session.uid): %s", strerror(errno));
    }
#else
    if (geteuid() != PR_ROOT_UID) {
      if (setreuid(session.uid, PR_ROOT_UID) < 0) {
        pr_log_pri(PR_LOG_ERR, "RELINQUISH PRIVS: unable to "
          "setreuid(session.uid, PR_ROOT_UID): %s", strerror(errno));
      }

      if (user_privs > 0)
        user_privs--;

    } else {
      if (root_privs > 0)
        root_privs--;
    }

    if (getegid() != PR_ROOT_GID) {
      if (setregid(session.gid, PR_ROOT_GID) < 0) {
        pr_log_pri(PR_LOG_ERR, "RELINQUISH PRIVS: unable to "
          "setregid(session.gid, PR_ROOT_GID): %s", strerror(errno));
      }
    }

    if (setregid(session.gid, session.gid)) {
      pr_log_pri(PR_LOG_ERR, "RELINQUISH PRIVS: unable to "
        "setregid(session.gid, session.gid): %s", strerror(errno));
    }

    if (setreuid(session.uid, session.uid)) {
      pr_log_pri(PR_LOG_ERR, "RELINQUISH PRIVS: unable to "
        "setreuid(session.uid, session.uid): %s", strerror(errno));
    }

#endif /* !HAVE_SETEUID */
  } else {
    pr_log_debug(DEBUG9, "RELINQUISH PRIVS: ID switching disabled");
  }

  pr_signals_unblock();
  return 0;
}

int pr_privs_revoke(const char *file, int lineno) {
  pr_log_debug(DEBUG9, "REVOKE PRIVS at %s:%d", file, lineno);

  root_privs = user_privs = 0;
  pr_trace_msg(trace_channel, 9, "PRIVS_REVOKE called, "
    "clearing user/root privs count");

  pr_signals_block();

#if defined(HAVE_SETEUID)
  if (seteuid(PR_ROOT_UID) < 0) {
    pr_log_pri(PR_LOG_ERR, "REVOKE PRIVS: unable to seteuid(): %s",
      strerror(errno));
  }

  if (setgid(session.gid) < 0) {
    pr_log_pri(PR_LOG_ERR, "REVOKE PRIVS: unable to setgid(): %s",
      strerror(errno));
  }

  if (setuid(session.uid) < 0) {
    pr_log_pri(PR_LOG_ERR, "REVOKE PRIVS: unable to setuid(): %s",
      strerror(errno));
  }
#else
  if (setreuid(PR_ROOT_UID, PR_ROOT_UID) < 0) {
    pr_log_pri(PR_LOG_ERR, "REVOKE PRIVS: unable to "
      "setreuid(PR_ROOT_UID, PR_ROOT_UID): %s", strerror(errno));
  }

  if (setgid(session.gid) < 0) {
    pr_log_pri(PR_LOG_ERR, "REVOKE PRIVS: unable to setgid(): %s",
      strerror(errno));
  }

  if (setuid(session.uid) < 0) {
    pr_log_pri(PR_LOG_ERR, "REVOKE PRIVS: unable to setuid(): %s",
      strerror(errno));
  }
#endif /* !HAVE_SETEUID */

  pr_signals_unblock();
  return 0;
}
