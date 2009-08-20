/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2005 The ProFTPD Project team
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

/* $Id: privs.h,v 1.31 2006/03/12 00:16:29 castaglia Exp $
 */

#ifndef PR_PRIVS_H
#define PR_PRIVS_H

/* Macros for manipulating saved, real and effective uid for easy
 * switching from/to root.
 *
 * Note: In version 1.1.5, all of this changed.  We USED to play games
 * with the saved-uid/gid _and_ setreuid()/setregid(), however this
 * appears to be slightly non-portable (i.e. w/ BSDs).  However, since
 * POSIX.1 saved-uids are pretty much useless without setre* (in the
 * case of root), so we now use basic uid swapping if we have seteuid(),
 * and setreuid() swapping if not.
 */

/* Porters, please put the most reasonable and secure method of
 * doing this in here:
 */

#if defined(HPUX10) || defined(HPUX11)
# define setreuid(r, e) setresuid((r), (e), 0)
#endif /* HPUX */

#ifdef PR_DEVEL_COREDUMP
/* Unix kernels can be notoriously picky about dumping the core for
 * processes that have fiddled with their effective/actual UID and GID.
 * So, to make it possible for people to have their proftpd processes
 * actually be able to coredump, these PRIVS macros, which switch
 * privileges, are effectively disabled.
 *
 * Hence it is not a Good Idea to run a proftpd built with PR_DEVEL_COREDUMP
 * defined in production.
 */

# define PRIVS_SETUP(u, g)
# define PRIVS_ROOT
# define PRIVS_USER
# define PRIVS_RELINQUISH
# define PRIVS_REVOKE

#else

# if !defined(HAVE_SETEUID)

/* Use setreuid() to perform uid swapping.
 */

#  define PRIVS_SETUP(u, g) { \
    pr_log_debug(DEBUG9, "SETUP PRIVS at %s:%d", __FILE__, __LINE__); \
    pr_signals_block(); \
    if (getuid() != PR_ROOT_UID) { \
      session.ouid = session.uid = getuid(); \
      session.gid = getgid(); \
      if (setgid(session.gid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_SETUP: unable to setgid(): %s", \
          strerror(errno)); \
      if (setreuid(session.uid, session.uid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_SETUP: unable to setreuid(): %s", \
          strerror(errno)); \
    } else {  \
      session.ouid = getuid(); \
      session.uid = (u); \
      session.gid = (g); \
      if (setgid(session.gid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_SETUP: unable to setgid(): %s", \
          strerror(errno)); \
      if (setreuid(PR_ROOT_UID, session.uid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_SETUP: unable to setreuid(): %s", \
          strerror(errno)); \
    } \
    pr_signals_unblock(); \
  }

#  define PRIVS_ROOT { \
    pr_log_debug(DEBUG9, "ROOT PRIVS at %s:%d", __FILE__, __LINE__); \
    pr_signals_block(); \
    if (!session.disable_id_switching) { \
      if (setreuid(session.uid, PR_ROOT_UID)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_ROOT: unable to setreuid(): %s", \
          strerror(errno)); \
      if (setregid(session.gid, PR_ROOT_GID)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_ROOT: unable to setregid(): %s", \
          strerror(errno)); \
    } else { \
      pr_log_debug(DEBUG9, "PRIVS_ROOT: ID switching disabled"); \
    } \
    pr_signals_unblock(); \
  }

#  define PRIVS_USER { \
    pr_log_debug(DEBUG9, "USER PRIVS %d at %s:%d", (int) session.login_uid, \
      __FILE__, __LINE__); \
    pr_signals_block(); \
    if (!session.disable_id_switching) { \
      if (setreuid(session.uid, PR_ROOT_UID)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_USER: unable to setreuid(session.uid, PR_ROOT_UID): %s", \
          strerror(errno)); \
      if (setregid(session.gid, session.login_gid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_USER: unable to setregid(session.gid, " \
          "session.login_gid): %s", strerror(errno)); \
      if (setreuid(session.uid, session.login_uid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_USER: unable to setreuid(session.uid, " \
          "session.login_uid): %s", strerror(errno)); \
    } else { \
      pr_log_debug(DEBUG9, "PRIVS_USER: ID switching disabled"); \
    } \
    pr_signals_unblock(); \
  }

#  define PRIVS_RELINQUISH  { \
    pr_signals_block(); \
    if (!session.disable_id_switching) { \
      pr_log_debug(DEBUG9, "RELINQUISH PRIVS at %s:%d", __FILE__, __LINE__); \
      if (geteuid() != PR_ROOT_UID) { \
        if (setreuid(session.uid, PR_ROOT_UID)) \
          pr_log_pri(PR_LOG_ERR, "PRIVS_RELINQUISH: unable to " \
            "setreuid(session.uid, PR_ROOT_UID): %s", strerror(errno)); \
      } \
      if (getegid() != PR_ROOT_GID) { \
        if (setregid(session.gid, PR_ROOT_GID)) \
          pr_log_pri(PR_LOG_ERR, "PRIVS_RELINQUISH: unable to " \
            "setregid(session.gid, PR_ROOT_GID): %s", strerror(errno)); \
      } \
      if (setregid(session.gid, session.gid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_RELINQUISH: unable to setregid(session.gid, " \
          "session.gid): %s", strerror(errno)); \
      if (setreuid(session.uid, session.uid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_RELINQUISH: unable to setreuid(session.uid, " \
          "session.uid): %s", strerror(errno)); \
    } else { \
      pr_log_debug(DEBUG9, "PRIVS_RELINQUISH: ID switching disabled"); \
    } \
    pr_signals_unblock(); \
  }

#  define PRIVS_REVOKE { \
    pr_log_debug(DEBUG9, "REVOKE PRIVS at %s:%d", __FILE__, __LINE__); \
    pr_signals_block(); \
    if (setreuid(PR_ROOT_UID, PR_ROOT_UID)) \
      pr_log_pri(PR_LOG_ERR, "PRIVS_REVOKE: unable to setreuid(PR_ROOT_UID, PR_ROOT_UID): %s", \
        strerror(errno)); \
    if (setgid(session.gid)) \
      pr_log_pri(PR_LOG_ERR, "PRIVS_REVOKE: unable to setgid(): %s", \
        strerror(errno)); \
    if (setuid(session.uid)) \
      pr_log_pri(PR_LOG_ERR, "PRIVS_REVOKE: unable to setuid(): %s", \
        strerror(errno)); \
    pr_signals_unblock(); \
  }

# else /* HAVE_SETEUID */

/* Set the saved uid/gid using setuid/seteuid().  setreuid() is
 * no longer used as it is considered obsolete on many systems.
 * gids are also no longer swapped, as they are unnecessary.
 * If run as root, proftpd now normally runs as:
 *   real user            : root
 *   effective user       : <user>
 *   saved user           : root
 *   real/eff/saved group : <group>
 */

#  define PRIVS_SETUP(u, g) { \
    pr_log_debug(DEBUG9, "SETUP PRIVS at %s:%d", __FILE__, __LINE__); \
    pr_signals_block(); \
    if (getuid() != PR_ROOT_UID) { \
      session.ouid = session.uid = getuid(); \
      session.gid = getgid(); \
      if (setgid(session.gid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_SETUP: unable to setgid(): %s", \
          strerror(errno)); \
      if (setuid(session.uid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_SETUP: unable to setuid(): %s", \
          strerror(errno)); \
      if (seteuid(session.uid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_SETUP: unable to seteuid(): %s", \
          strerror(errno)); \
    } else { \
      session.ouid = getuid(); \
      session.uid = (u); \
      session.gid = (g); \
      if (setuid(PR_ROOT_UID)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_SETUP: unable to setuid(): %s", \
          strerror(errno)); \
      if (setgid((g))) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_SETUP: unable to setgid(): %s", \
          strerror(errno)); \
      if (seteuid((u))) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_SETUP: unable to seteuid(): %s", \
          strerror(errno)); \
    } \
    pr_signals_unblock(); \
  }

/* Switch back to root privs.
 */
#  define PRIVS_ROOT { \
    pr_log_debug(DEBUG9, "ROOT PRIVS at %s:%d", __FILE__, __LINE__); \
    pr_signals_block(); \
    if (!session.disable_id_switching) { \
      if (seteuid(PR_ROOT_UID)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_ROOT: unable to seteuid(): %s", \
          strerror(errno)); \
      if (setegid(PR_ROOT_GID)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_ROOT: unable to setegid(): %s", \
          strerror(errno)); \
    } else { \
      pr_log_debug(DEBUG9, "ROOT PRIVS: ID switching disabled"); \
    } \
    pr_signals_unblock(); \
  }

/* Switch to the privs of the login user.
 */
#  define PRIVS_USER { \
    pr_log_debug(DEBUG9, "USER PRIVS %d at %s:%d", (int) session.login_uid, \
      __FILE__, __LINE__); \
    pr_signals_block(); \
    if (!session.disable_id_switching) { \
      if (seteuid(PR_ROOT_UID)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_USER: unable to seteuid(PR_ROOT_UID): %s", \
          strerror(errno)); \
      if (setegid(session.login_gid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_USER: unable to setegid(session.login_gid): " \
          "%s", strerror(errno)); \
      if (seteuid(session.login_uid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_USER: unable to seteuid(session.login_uid): " \
          "%s", strerror(errno)); \
    } else { \
      pr_log_debug(DEBUG9, "PRIVS_USER: ID switching disabled"); \
    } \
    pr_signals_unblock(); \
  }

/* Relinquish privs granted by PRIVS_ROOT or PRIVS_USER.
 */
#  define PRIVS_RELINQUISH { \
    pr_signals_block(); \
    if (!session.disable_id_switching) { \
      pr_log_debug(DEBUG9, "RELINQUISH PRIVS at %s:%d", __FILE__, __LINE__); \
      if (geteuid() != PR_ROOT_UID) { \
        if (seteuid(PR_ROOT_UID)) \
          pr_log_pri(PR_LOG_ERR, "PRIVS_RELINQUISH: unable to seteuid(PR_ROOT_UID): %s", strerror(errno)); \
      } \
      if (setegid(session.gid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_RELINQUISH: unable to setegid(session.gid): %s", strerror(errno)); \
      if (seteuid(session.uid)) \
        pr_log_pri(PR_LOG_ERR, "PRIVS_RELINQUISH: unable to seteuid(session.uid): %s", strerror(errno)); \
    } else { \
      pr_log_debug(DEBUG9, "PRIVS_RELINQUISH: ID switching disabled"); \
    } \
    pr_signals_unblock(); \
  }

/* Revoke all privs.
 */
#  define PRIVS_REVOKE { \
    pr_log_debug(DEBUG9, "REVOKE PRIVS at %s:%d", __FILE__, __LINE__); \
    pr_signals_block(); \
    if (seteuid(PR_ROOT_UID)) \
      pr_log_pri(PR_LOG_ERR, "PRIVS_REVOKE: unable to seteuid(): %s", \
        strerror(errno)); \
    if (setgid(session.gid)) \
      pr_log_pri(PR_LOG_ERR, "PRIVS_REVOKE: unable to setgid(): %s", \
        strerror(errno)); \
    if (setuid(session.uid)) \
      pr_log_pri(PR_LOG_ERR, "PRIVS_REVOKE: unable to setuid(): %s", \
        strerror(errno)); \
    pr_signals_unblock(); \
  }

# endif /* HAVE_SETEUID */
#endif /* PR_DEVEL_COREDUMP */

#endif /* PR_PRIVS_H */
