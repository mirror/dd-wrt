/*
 * ProFTPD: mod_facts -- a module for handling "facts" [RFC3659]
 *
 * Copyright (c) 2007-2009 The ProFTPD Project
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
 * $Id: mod_facts.c,v 1.26 2009/10/14 23:45:28 castaglia Exp $
 */

#include "conf.h"

#define MOD_FACTS_VERSION		"mod_facts/0.1"

#if PROFTPD_VERSION_NUMBER < 0x0001030101
# error "ProFTPD 1.3.1rc1 or later required"
#endif

module facts_module;

static unsigned long facts_opts = 0;
#define FACTS_OPT_SHOW_MODIFY		0x00001
#define FACTS_OPT_SHOW_PERM		0x00002
#define FACTS_OPT_SHOW_SIZE		0x00004
#define FACTS_OPT_SHOW_TYPE		0x00008
#define FACTS_OPT_SHOW_UNIQUE		0x00010
#define FACTS_OPT_SHOW_UNIX_GROUP	0x00020
#define FACTS_OPT_SHOW_UNIX_MODE	0x00040
#define FACTS_OPT_SHOW_UNIX_OWNER	0x00080

struct mlinfo {
  pool *pool;
  struct stat st;
  struct tm *tm;
  const char *type;
  const char *perm;
  const char *path;
};

/* Necessary prototypes */
static void facts_mlinfobuf_flush(void);

/* Support functions
 */

static int facts_filters_allow_path(cmd_rec *cmd, const char *path) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  regex_t *preg = get_param_ptr(CURRENT_CONF, "PathAllowFilter", FALSE);

  if (preg &&
      regexec(preg, path, 0, NULL, 0) != 0) {
    pr_log_debug(DEBUG2, MOD_FACTS_VERSION
      ": %s denied by PathAllowFilter on '%s'", cmd->argv[0], cmd->arg);
    return -1;
  }

  preg = get_param_ptr(CURRENT_CONF, "PathDenyFilter", FALSE);

  if (preg &&
      regexec(preg, path, 0, NULL, 0) == 0) {
    pr_log_debug(DEBUG2, MOD_FACTS_VERSION
      ": %s denied by PathDenyFilter on '%s'", cmd->argv[0], cmd->arg);
    return -1;
  }
#endif

  return 0;
}

static time_t facts_mktime(unsigned int year, unsigned int month,
    unsigned int mday, unsigned int hour, unsigned int min, unsigned int sec) {
  struct tm tm;
  time_t res;
  char *env;

#ifdef HAVE_TZNAME
  char *tzname_dup[2];

  /* The mktime(3) function has a nasty habit of changing the tzname global
   * variable as a side-effect.  This can cause problems, as when the process
   * has become chrooted, and mktime(3) sets/changes tzname wrong.  (For more
   * information on the tzname global variable, see the tzset(3) man page.)
   *
   * The best way to deal with this issue (which is especially prominent
   * on systems running glibc-2.3 or later, which is particularly ill-behaved
   * in a chrooted environment, as it assumes the ability to find system
   * timezone files at paths which are no longer valid within the chroot)
   * is to set the TZ environment variable explicitly, before starting
   * proftpd.  You can also use the SetEnv configuration directive within
   * the proftpd.conf to set the TZ environment variable, e.g.:
   *
   *  SetEnv TZ PST
   *
   * To try to help sites which fail to do this, the tzname global variable
   * will be copied prior to the mktime(3) call, and the copy restored after
   * the call.  (Note that calling the ctime(3) and localtime(3) functions also
   * causes a similar overwriting/setting of the tzname environment variable.)
   */
  memcpy(&tzname_dup, tzname, sizeof(tzname_dup));
#endif /* HAVE_TZNAME */

  env = pr_env_get(session.pool, "TZ");

  /* Set the TZ environment to be GMT, so that mktime(3) treats the timestamp
   * provided by the client as being in GMT/UTC.
   */
  if (pr_env_set(session.pool, "TZ", "GMT") < 0) {
    pr_log_debug(DEBUG8, MOD_FACTS_VERSION
      ": error setting TZ environment variable to 'GMT': %s", strerror(errno));
  }

  tm.tm_sec = sec;
  tm.tm_min = min;
  tm.tm_hour = hour;
  tm.tm_mday = mday;
  tm.tm_mon = (month - 1);
  tm.tm_year = (year - 1900);
  tm.tm_wday = 0;
  tm.tm_yday = 0;
  tm.tm_isdst = -1;

  res = mktime(&tm);

  /* Restore the old TZ setting, if any. */
  if (env) {
    if (pr_env_set(session.pool, "TZ", env) < 0) {
      pr_log_debug(DEBUG8, MOD_FACTS_VERSION
        ": error setting TZ environment variable to '%s': %s", env,
        strerror(errno));
    }
  }

#ifdef HAVE_TZNAME
  /* Restore the old tzname values prior to returning. */
  memcpy(tzname, tzname_dup, sizeof(tzname_dup));
#endif /* HAVE_TZNAME */

  return res;
}

static size_t facts_mlinfo_fmt(struct mlinfo *info, char *buf, size_t bufsz) {
  char *ptr;
  size_t buflen = 0;

  memset(buf, '\0', bufsz);

  ptr = buf + buflen;

  if (facts_opts & FACTS_OPT_SHOW_MODIFY) {
    snprintf(ptr, bufsz - buflen, "modify=%04d%02d%02d%02d%02d%02d;",
      info->tm->tm_year+1900, info->tm->tm_mon+1, info->tm->tm_mday,
      info->tm->tm_hour, info->tm->tm_min, info->tm->tm_sec);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  if (facts_opts & FACTS_OPT_SHOW_PERM) {
    snprintf(ptr, bufsz - buflen, "perm=%s;", info->perm);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  if (!S_ISDIR(info->st.st_mode) &&
      (facts_opts & FACTS_OPT_SHOW_SIZE)) {
    snprintf(ptr, bufsz - buflen, "size=%" PR_LU ";",
      (pr_off_t) info->st.st_size);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  if (facts_opts & FACTS_OPT_SHOW_TYPE) {
    snprintf(ptr, bufsz - buflen, "type=%s;", info->type);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  if (facts_opts & FACTS_OPT_SHOW_UNIQUE) {
    snprintf(ptr, bufsz - buflen, "unique=%lXU%lX;",
      (unsigned long) info->st.st_dev, (unsigned long) info->st.st_ino);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  if (facts_opts & FACTS_OPT_SHOW_UNIX_GROUP) {
    snprintf(ptr, bufsz - buflen, "UNIX.group=%lu;",
      (unsigned long) info->st.st_gid);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  if (facts_opts & FACTS_OPT_SHOW_UNIX_MODE) {
    snprintf(ptr, bufsz - buflen, "UNIX.mode=0%o;",
      (unsigned int) info->st.st_mode & 07777);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  if (facts_opts & FACTS_OPT_SHOW_UNIX_OWNER) {
    snprintf(ptr, bufsz - buflen, "UNIX.owner=%lu;",
      (unsigned long) info->st.st_uid);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  /* MLST entries are not sent via pr_data_xfer(), and thus we do not need
   * to include an LF at the end; it is appended by pr_response_send_raw().
   * But MLSD entries DO need the trailing LF, so that it can be converted
   * into a CRLF sequence by pr_data_xfer().
   */
  if (strcmp(session.curr_cmd, C_MLSD) == 0) {
    snprintf(ptr, bufsz - buflen, " %s\n", info->path);

  } else {
    snprintf(ptr, bufsz - buflen, " %s", info->path);
  }

  buf[bufsz-1] = '\0';
  buflen = strlen(buf);

  return buflen;
}

/* This buffer is used by the MLSD handler, to buffer up the output lines.
 * When all the lines have been added, or when the buffer is full, it will
 * flushed out.
 *
 * This handling is different from the MLST handler's use of
 * facts_mlinfo_add() because MLST gets to send its line back on the control
 * channel, wherease MLSD's output is sent via a data transfer, much like
 * LIST or NLST.
 */
static char *mlinfo_buf = NULL;
static size_t mlinfo_bufsz = 0;
static size_t mlinfo_buflen = 0;

static void facts_mlinfobuf_init(void) {
  if (mlinfo_buf == NULL) {
    mlinfo_bufsz = pr_config_get_xfer_bufsz();
    mlinfo_buf = palloc(session.pool, mlinfo_bufsz);
  }

  memset(mlinfo_buf, '\0', mlinfo_bufsz);
  mlinfo_buflen = 0;
}

static void facts_mlinfobuf_add(struct mlinfo *info) {
  char buf[PR_TUNABLE_BUFFER_SIZE];
  size_t buflen;
 
  buflen = facts_mlinfo_fmt(info, buf, sizeof(buf));

  /* If this buffer will exceed the capacity of mlinfo_buf, then flush
   * mlinfo_buf.
   */
  if (buflen >= (mlinfo_bufsz - mlinfo_buflen)) {
    (void) facts_mlinfobuf_flush();
  }

  sstrcat(mlinfo_buf, buf, mlinfo_bufsz);
  mlinfo_buflen += buflen;
}

static void facts_mlinfobuf_flush(void) {
  if (mlinfo_buflen > 0) {
    int res;

    res = pr_data_xfer(mlinfo_buf, mlinfo_buflen);
    if (res < 0 &&
        errno != 0) {
      pr_log_debug(DEBUG3, MOD_FACTS_VERSION
        ": error transferring data: [%d] %s", errno, strerror(errno));
    }
  }

  facts_mlinfobuf_init();
}

static int facts_mlinfo_get(struct mlinfo *info, const char *path,
    const char *dent_name) {
  char *perm = "";

  if (pr_fsio_stat(path, &(info->st)) < 0) {
    pr_log_debug(DEBUG4, MOD_FACTS_VERSION ": error stat'ing '%s': %s",
      path, strerror(errno));
    return -1;
  }

  info->tm = pr_gmtime(info->pool, &(info->st.st_mtime));

  if (!S_ISDIR(info->st.st_mode)) {
    info->type = "file";

    if (pr_fsio_access(path, R_OK, session.uid, session.gid,
        session.gids) == 0) {

      /* XXX Need to come up with a good way of determining whether 'd'
       * should be listed.  For example, if the parent directory does not
       * allow write privs to the current user/group, then the file cannot
       * be deleted.
       */

      perm = pstrcat(info->pool, perm, "adfr", NULL);
    }

    if (pr_fsio_access(path, W_OK, session.uid, session.gid,
        session.gids) == 0) {
      perm = pstrcat(info->pool, perm, "w", NULL);
    }

  } else {
    info->type = "dir";

    if (dent_name[0] != '.') {
      if (strcmp(path, pr_fs_getcwd()) == 0) {
        info->type = "cdir";
      }

    } else {
      if (dent_name[1] == '\0') {
        info->type = "cdir";
      }

      if (strlen(dent_name) >= 2) {
        if (dent_name[1] == '.' &&
            dent_name[2] == '\0') {
          info->type = "pdir";
        }
      }
    }

    if (pr_fsio_access(path, R_OK, session.uid, session.gid,
        session.gids) == 0) {
      perm = pstrcat(info->pool, perm, "fl", NULL);
    }

    if (pr_fsio_access(path, W_OK, session.uid, session.gid,
        session.gids) == 0) {
      perm = pstrcat(info->pool, perm, "cdmp", NULL);
    }

    if (pr_fsio_access(path, X_OK, session.uid, session.gid,
        session.gids) == 0) {
      perm = pstrcat(info->pool, perm, "e", NULL);
    }
  }

  info->perm = perm;
  return 0;
}

static void facts_mlinfo_add(struct mlinfo *info) {
  char buf[PR_TUNABLE_BUFFER_SIZE];

  (void) facts_mlinfo_fmt(info, buf, sizeof(buf));

  /* The trailing CRLF will be added by pr_response_add(). */
  pr_response_add(R_DUP, "%s", buf);
}

static void facts_mlst_feat_add(pool *p) {
  char *feat_str = "";

  feat_str = pstrcat(p, feat_str, "modify", NULL);
  if (facts_opts & FACTS_OPT_SHOW_MODIFY) {
    feat_str = pstrcat(p, feat_str, "*;", NULL);

  } else {
    feat_str = pstrcat(p, feat_str, ";", NULL);
  }

  feat_str = pstrcat(p, feat_str, "perm", NULL);
  if (facts_opts & FACTS_OPT_SHOW_PERM) {
    feat_str = pstrcat(p, feat_str, "*;", NULL);

  } else {
    feat_str = pstrcat(p, feat_str, ";", NULL);
  }

  feat_str = pstrcat(p, feat_str, "size", NULL);
  if (facts_opts & FACTS_OPT_SHOW_SIZE) {
    feat_str = pstrcat(p, feat_str, "*;", NULL);

  } else {
    feat_str = pstrcat(p, feat_str, ";", NULL);
  }

  feat_str = pstrcat(p, feat_str, "type", NULL);
  if (facts_opts & FACTS_OPT_SHOW_TYPE) {
    feat_str = pstrcat(p, feat_str, "*;", NULL);

  } else {
    feat_str = pstrcat(p, feat_str, ";", NULL);
  }

  feat_str = pstrcat(p, feat_str, "unique", NULL);
  if (facts_opts & FACTS_OPT_SHOW_UNIQUE) {
    feat_str = pstrcat(p, feat_str, "*;", NULL);

  } else {
    feat_str = pstrcat(p, feat_str, ";", NULL);
  }

  feat_str = pstrcat(p, feat_str, "UNIX.group", NULL);
  if (facts_opts & FACTS_OPT_SHOW_UNIX_GROUP) {
    feat_str = pstrcat(p, feat_str, "*;", NULL);

  } else {
    feat_str = pstrcat(p, feat_str, ";", NULL);
  }

  feat_str = pstrcat(p, feat_str, "UNIX.mode", NULL);
  if (facts_opts & FACTS_OPT_SHOW_UNIX_MODE) {
    feat_str = pstrcat(p, feat_str, "*;", NULL);

  } else {
    feat_str = pstrcat(p, feat_str, ";", NULL);
  }

  feat_str = pstrcat(p, feat_str, "UNIX.owner", NULL);
  if (facts_opts & FACTS_OPT_SHOW_UNIX_OWNER) {
    feat_str = pstrcat(p, feat_str, "*;", NULL);

  } else {
    feat_str = pstrcat(p, feat_str, ";", NULL);
  }

  feat_str = pstrcat(p, "MLST ", feat_str, NULL);
  pr_feat_add(feat_str);
}

static void facts_mlst_feat_remove(void) {
  const char *feat, *mlst_feat = NULL;

  feat = pr_feat_get();
  while (feat) {
    pr_signals_handle();

    if (strncmp(feat, C_MLST, 4) == 0) {
      mlst_feat = feat;
      break;
    }

    feat = pr_feat_get_next();
  }

  if (mlst_feat)
    pr_feat_remove(mlst_feat);
}

static int facts_modify_mtime(pool *p, const char *path, char *timestamp) {
  char c, *ptr;
  unsigned int year, month, day, hour, min, sec;
  struct timeval tvs[2];

  (void) p;

  ptr = timestamp;
  c = timestamp[4];
  timestamp[4] = '\0';
  year = atoi(ptr);
  timestamp[4] = c;

  ptr = &(timestamp[4]);
  c = timestamp[6];
  timestamp[6] = '\0';
  month = atoi(ptr);
  timestamp[6] = c;

  if (month < 1 ||
      month > 12) {
    pr_log_debug(DEBUG8, MOD_FACTS_VERSION
      ": bad number of months (%d) in timestamp '%s'", month, timestamp);
    errno = EINVAL;
    return -1;
  }

  ptr = &(timestamp[6]);
  c = timestamp[8];
  timestamp[8] = '\0';
  day = atoi(ptr);
  timestamp[8] = c;

  if (day < 1 ||
      day > 31) {
    pr_log_debug(DEBUG8, MOD_FACTS_VERSION
      ": bad number of days (%d) in timestamp '%s'", day, timestamp);
    errno = EINVAL;
    return -1;
  }

  ptr = &(timestamp[8]);
  c = timestamp[10];
  timestamp[10] = '\0';
  hour = atoi(ptr);
  timestamp[10] = c;

  if (hour < 0 ||
      hour > 24) {
    pr_log_debug(DEBUG8, MOD_FACTS_VERSION
      ": bad number of hours (%d) in timestamp '%s'", hour, timestamp);
    errno = EINVAL;
    return -1;
  }

  ptr = &(timestamp[10]);
  c = timestamp[12];
  timestamp[12] = '\0';
  min = atoi(ptr);
  timestamp[12] = c;

  if (min < 0 ||
      min > 60) {
    pr_log_debug(DEBUG8, MOD_FACTS_VERSION
      ": bad number of minutes (%d) in timestamp '%s'", min, timestamp);
    errno = EINVAL;
    return -1;
  }

  ptr = &(timestamp[12]);
  sec = atoi(ptr);

  if (sec < 0 ||
      sec > 61) {
    pr_log_debug(DEBUG8, MOD_FACTS_VERSION
      ": bad number of seconds (%d) in timestamp '%s'", sec, timestamp);
    errno = EINVAL;
    return -1;
  }

  tvs[0].tv_usec = tvs[1].tv_usec = 0;
  tvs[0].tv_sec = tvs[1].tv_sec = facts_mktime(year, month, day, hour, min,
    sec);

  if (pr_fsio_utimes(path, tvs) < 0) {
    pr_log_debug(DEBUG2, MOD_FACTS_VERSION
      ": error modifying modify fact for '%s': %s", path, strerror(errno));
    return -1;
  }

  return 0;
}

static int facts_modify_unix_group(pool *p, const char *path,
    const char *group) {
  gid_t gid;
  char *tmp = NULL;

  gid = strtoul(group, &tmp, 10);
  if (tmp &&
      *tmp) {
    /* Try to lookup the GID using the value as a name. */
    gid = pr_auth_name2gid(p, group);
    if (gid == (gid_t) -1) {
      pr_log_debug(DEBUG7, MOD_FACTS_VERSION ": no such group '%s'", group);
      errno = EINVAL;
      return -1;
    }
  }

  if (pr_fsio_chown(path, (uid_t) -1, gid) < 0) {
    pr_log_debug(DEBUG5, MOD_FACTS_VERSION
      ": error modifying UNIX.group fact for '%s': %s", path, strerror(errno));
    return -1;
  }

  return 0;
}

static int facts_modify_unix_mode(pool *p, const char *path, char *mode_str) {
  mode_t mode;
  char *tmp = NULL;

  mode = strtoul(mode_str, &tmp, 8);
  if (tmp &&
      *tmp) {
    pr_log_debug(DEBUG3, MOD_FACTS_VERSION
      ": UNIX.mode fact '%s' is not an octal number", mode_str);
    return -1;
  }

  if (pr_fsio_chmod(path, mode) < 0) {
    pr_log_debug(DEBUG5, MOD_FACTS_VERSION
      ": error modifying UNIX.mode fact for '%s': %s", path, strerror(errno));
    return -1;
  }

  return 0;
}

/* Command handlers
 */

MODRET facts_mff(cmd_rec *cmd) {
  const char *path, *decoded_path;
  char *facts, *ptr;

  if (cmd->argc < 3) {
    pr_response_add_err(R_501, _("Invalid number of arguments"));
    return PR_ERROR(cmd);
  }

  facts = cmd->argv[1];

  /* The path can contain spaces.  Thus we need to use cmd->arg, not cmd->argv,
   * to find the path.  But cmd->arg contains the facts as well.  Thus we
   * find the FIRST space in cmd->arg; the path is everything past that space.
   */
  ptr = strchr(cmd->arg, ' ');
  path = pstrdup(cmd->tmp_pool, ptr + 1);

  decoded_path = pr_fs_decode_path(cmd->tmp_pool, path);

  if (!dir_check(cmd->tmp_pool, cmd, cmd->group, (char *) decoded_path, NULL)) {
    pr_log_debug(DEBUG4, MOD_FACTS_VERSION ": %s command denied by <Limit>",
      cmd->argv[0]);
    pr_response_add_err(R_550, _("Unable to handle command"));
    return PR_ERROR(cmd);
  }

  if (facts_filters_allow_path(cmd, decoded_path) < 0) {
    pr_response_add_err(R_550, "%s: %s", path, strerror(EACCES));
    return PR_ERROR(cmd);
  }

  ptr = strchr(facts, ';');
  if (ptr == NULL) {
    pr_response_add_err(R_550, "%s: %s", facts, strerror(EINVAL));
    return PR_ERROR(cmd);
  }

  while (ptr) {
    pr_signals_handle();

    *ptr = '\0';

    if (strncasecmp(facts, "modify", 6) == 0) {
      /* Equivalent to SITE UTIME, or MFMT */

      char *timestamp, *ptr2;

      ptr2 = strchr(facts, '=');
      if (!ptr2) {
        pr_response_add_err(R_501, "%s: %s", cmd->argv[1], strerror(EINVAL));
        return PR_ERROR(cmd);
      }

      timestamp = ptr2 + 1;

      if (strlen(timestamp) < 14) {
        pr_response_add_err(R_501, "%s: %s", timestamp, strerror(EINVAL));
        return PR_ERROR(cmd);
      }

      ptr2 = strchr(timestamp, '.');
      if (ptr2) {
        pr_log_debug(DEBUG7, MOD_FACTS_VERSION
          ": %s: ignoring unsupported timestamp precision in '%s'",
          cmd->argv[0], timestamp);
        *ptr2 = '\0';
      }

      if (facts_modify_mtime(cmd->tmp_pool, decoded_path, timestamp) < 0) {
        pr_response_add_err(errno == ENOENT ? R_550 : R_501, "%s: %s", path,
          strerror(errno));
        return PR_ERROR(cmd);
      }

    } else if (strncasecmp(facts, "UNIX.group", 10) == 0) {
      /* Equivalent to SITE CHGRP */

      char *group, *ptr2;

      ptr2 = strchr(facts, '=');
      if (!ptr2) {
        *ptr = ';';
        pr_response_add_err(R_501, "%s: %s", cmd->argv[1], strerror(EINVAL));
        return PR_ERROR(cmd);
      }

      group = ptr2 + 1;

      if (facts_modify_unix_group(cmd->tmp_pool, decoded_path, group) < 0) {
        pr_response_add_err(errno == ENOENT ? R_550 : R_501, "%s: %s", path,
          strerror(errno));
        return PR_ERROR(cmd);
      }

    } else if (strncasecmp(facts, "UNIX.mode", 9) == 0) {
      /* Equivalent to SITE CHMOD */

      char *mode_str, *ptr2;

      ptr2 = strchr(facts, '=');
      if (!ptr2) {
        *ptr = ';';
        pr_response_add_err(R_501, "%s: %s", cmd->argv[1], strerror(EINVAL));
        return PR_ERROR(cmd);
      }

      mode_str = ptr2 + 1;

      if (facts_modify_unix_mode(cmd->tmp_pool, decoded_path, mode_str) < 0) {
        pr_response_add_err(errno == ENOENT ? R_550 : R_501, "%s: %s", path,
          strerror(errno));
        return PR_ERROR(cmd);
      }

    } else {
      /* Unlike the OPTS MLST handling, if MFF is sent with an unsupported
       * fact, we get to return an error.
       */
      pr_log_debug(DEBUG5, MOD_FACTS_VERSION
        ": %s: fact '%s' unsupported for modification, denying request",
        cmd->argv[0], facts);
      pr_response_add_err(R_504, _("Cannot modify fact '%s'"), facts);

      *ptr = ';';
      return PR_ERROR(cmd);
    }

    *ptr = ';';
    facts = ptr + 1;
    ptr = strchr(facts, ';');
  }

  /* Due to Draft requirements/recommendations, the list of facts that
   * were successfully modified are to be included in the response, for
   * possible client parsing.  This means that the list is NOT localisable.
   */
  pr_response_add(R_213, "%s %s", cmd->argv[1], path);
  return PR_HANDLED(cmd);
}

MODRET facts_mfmt(cmd_rec *cmd) {
  const char *path, *decoded_path;
  char *timestamp, *ptr;
  int res;

  if (cmd->argc < 3) {
    pr_response_add_err(R_501, _("Invalid number of arguments"));
    return PR_ERROR(cmd);
  }

  timestamp = cmd->argv[1];

  /* The path can contain spaces.  Thus we need to use cmd->arg, not cmd->argv,
   * to find the path.  But cmd->arg contains the facts as well.  Thus we
   * find the FIRST space in cmd->arg; the path is everything past that space.
   */
  ptr = strchr(cmd->arg, ' ');
  path = pstrdup(cmd->tmp_pool, ptr + 1);

  decoded_path = pr_fs_decode_path(cmd->tmp_pool, path);

  if (!dir_check(cmd->tmp_pool, cmd, cmd->group, (char *) decoded_path, NULL)) {
    pr_log_debug(DEBUG4, MOD_FACTS_VERSION ": %s command denied by <Limit>",
      cmd->argv[0]);
    pr_response_add_err(R_550, _("Unable to handle command"));
    return PR_ERROR(cmd);
  }

  if (facts_filters_allow_path(cmd, decoded_path) < 0) {
    pr_response_add_err(R_550, "%s: %s", path, strerror(EACCES));
    return PR_ERROR(cmd);
  }

  if (strlen(timestamp) < 14) {
    pr_response_add_err(R_501, "%s: %s", timestamp, strerror(EINVAL));
    return PR_ERROR(cmd);
  }

  ptr = strchr(timestamp, '.');
  if (ptr) {
    pr_log_debug(DEBUG7, MOD_FACTS_VERSION
      ": %s: ignoring unsupported timestamp precision in '%s'", cmd->argv[0],
      timestamp);
    *ptr = '\0';
  }

  res = facts_modify_mtime(cmd->tmp_pool, decoded_path, timestamp);
  if (res < 0) {
    if (ptr) {
      *ptr = '.';
    }

    pr_response_add_err(errno == ENOENT ? R_550 : R_501, "%s: %s", path,
      strerror(errno));
    return PR_ERROR(cmd);
  }

  /* We need to capitalize the 'modify' fact name in the response, as
   * per the Draft, so that clients can parse it to see the actual
   * time used by the server; it is possible for the server to ignore some
   * of the precision requested by the client.
   *
   * This same requirement means that the string is NOT localisable.
   */
  pr_response_add(R_213, "Modify=%s; %s", timestamp, path);

  if (ptr) {
    *ptr = '.';
  }

  return PR_HANDLED(cmd);
}

MODRET facts_mlsd(cmd_rec *cmd) {
  const char *path, *decoded_path, *best_path;
  struct mlinfo info;
  DIR *dirh;
  struct dirent *dent;

  if (cmd->argc != 1) {
    path = pstrdup(cmd->tmp_pool, cmd->arg);
    decoded_path = pr_fs_decode_path(cmd->tmp_pool, path);

  } else {
    decoded_path = path = pr_fs_getcwd();
  }

  if (!dir_check(cmd->tmp_pool, cmd, cmd->group, (char *) decoded_path, NULL)) {
    pr_log_debug(DEBUG4, MOD_FACTS_VERSION ": %s command denied by <Limit>",
      cmd->argv[0]);
    pr_response_add_err(R_550, _("Unable to handle command"));
    return PR_ERROR(cmd);
  }

  /* RFC3659 explicitly does NOT support glob characters.  So warn about
   * this, but let the command continue as is.  We don't actually call
   * glob(3) here, so no expansion will occur.
   */
  if (strpbrk(decoded_path, "{[*?") != NULL) {
    pr_log_debug(DEBUG9, MOD_FACTS_VERSION ": glob characters in MLSD ('%s') "
      "ignored", decoded_path);
  }

  /* Make sure that the given path is actually a directory. */
  if (pr_fsio_stat(decoded_path, &(info.st)) < 0) {
    int xerrno = errno;

    pr_log_debug(DEBUG4, MOD_FACTS_VERSION ": unable to stat '%s' (%s), "
      "denying %s", decoded_path, strerror(xerrno), cmd->argv[0]);

    pr_response_add_err(R_550, "%s: %s", path, strerror(xerrno));
    return PR_ERROR(cmd);
  }

  if (!S_ISDIR(info.st.st_mode)) {
    pr_response_add_err(R_550, _("'%s' is not a directory"), path);
    return PR_ERROR(cmd);
  }

  best_path = dir_best_path(cmd->tmp_pool, decoded_path);

  dirh = pr_fsio_opendir(best_path);
  if (dirh == NULL) {
    int xerrno = errno;

    pr_trace_msg("fileperms", 1, "MLSD, user '%s' (UID %lu, GID %lu): "
      "error opening directory '%s': %s", session.user,
      (unsigned long) session.uid, (unsigned long) session.gid,
      best_path, strerror(xerrno));

    pr_response_add_err(R_550, "%s: %s", path, strerror(xerrno));
    return PR_ERROR(cmd);
  }

  /* Open data connection */
  session.sf_flags |= SF_ASCII_OVERRIDE;
  if (pr_data_open(NULL, C_MLSD, PR_NETIO_IO_WR, 0) < 0) {
    pr_fsio_closedir(dirh);
    return PR_ERROR(cmd);
  }

  pr_fs_clear_cache();
  facts_mlinfobuf_init();

  while ((dent = pr_fsio_readdir(dirh)) != NULL) {
    int hidden = FALSE, res;
    char *rel_path, *abs_path;

    pr_signals_handle();

    rel_path = pdircat(cmd->tmp_pool, best_path, dent->d_name, NULL);

    /* Check that the file can be listed. */
    abs_path = dir_realpath(cmd->tmp_pool, rel_path);
    if (abs_path) {
      res = dir_check(cmd->tmp_pool, cmd, cmd->group, abs_path, &hidden);
      
    } else {
      abs_path = dir_canonical_path(cmd->tmp_pool, rel_path);
      if (abs_path == NULL)
        abs_path = rel_path;

      res = dir_check_canon(cmd->tmp_pool, cmd, cmd->group, abs_path, &hidden);
    }

    if (!res || hidden) {
      continue;
    }

    memset(&info, 0, sizeof(struct mlinfo));

    info.pool = cmd->tmp_pool;
    if (facts_mlinfo_get(&info, rel_path, dent->d_name) < 0) {
      pr_log_debug(DEBUG3, MOD_FACTS_VERSION
        ": MLSD: unable to get info for '%s': %s", abs_path, strerror(errno));
      continue;
    }

    /* As per RFC3659, the directory being listed should not appear as a
     * component in the paths of the directory contents.
     */
    info.path = pr_fs_encode_path(cmd->tmp_pool, dent->d_name);

    facts_mlinfobuf_add(&info);

    if (XFER_ABORTED) {
      pr_data_abort(0, 0);
      break;
    }
  }

  pr_fsio_closedir(dirh);

  if (XFER_ABORTED) {
    pr_data_close(TRUE);

  } else {
    facts_mlinfobuf_flush();
    pr_data_close(FALSE);
  }

  return PR_HANDLED(cmd);
}

MODRET facts_mlst(cmd_rec *cmd) {
  int hidden = FALSE;
  const char *path, *decoded_path;
  struct mlinfo info;

  if (cmd->argc != 1) {
    path = pstrdup(cmd->tmp_pool, cmd->arg);
    decoded_path = pr_fs_decode_path(cmd->tmp_pool, path);

  } else {
    decoded_path = path = pr_fs_getcwd();
  }

  if (!dir_check(cmd->tmp_pool, cmd, cmd->group, (char *) decoded_path,
      &hidden)) {
    pr_log_debug(DEBUG4, MOD_FACTS_VERSION ": %s command denied by <Limit>",
      cmd->argv[0]);
    pr_response_add_err(R_550, _("Unable to handle command"));
    return PR_ERROR(cmd);
  }

  if (hidden) {
    /* Simply send an empty list, much like we do for a STAT command for
     * a hidden file.
     */
    pr_response_add(R_250, _("Start of list for %s"), path);
    pr_response_add(R_250, _("End of list"));

    return PR_HANDLED(cmd);
  }

  info.pool = cmd->tmp_pool;

  pr_fs_clear_cache();
  if (facts_mlinfo_get(&info, decoded_path, decoded_path) < 0) {
    pr_response_add_err(R_550, _("'%s' cannot be listed"), path);
    return PR_ERROR(cmd);
  }

  /* No need to re-encode the path here as UTF8, since 'path' is the
   * original parameter as sent by the client.
   */
  info.path = path;

  pr_response_add(R_250, _("Start of list for %s"), path);
  facts_mlinfo_add(&info);
  pr_response_add(R_250, _("End of list"));

  return PR_HANDLED(cmd);
}

MODRET facts_opts_mlst(cmd_rec *cmd) {
  register unsigned int i;
  char *method, *facts, *ptr, *resp_str = "";

  method = pstrdup(cmd->tmp_pool, cmd->argv[0]);

  /* Convert underscores to spaces in the method name, for prettier logging. */
  for (i = 0; method[i]; i++) {
    if (method[i] == '_')
      method[i] = ' ';
  }

  if (cmd->argc > 2) {
    pr_response_add_err(R_501, _("'%s' not understood"), method);
    return PR_ERROR(cmd);
  }

  if (cmd->argc == 1) {
    facts_opts = 0;

    /* Update MLST FEAT listing to match the showing of no facts. */
    facts_mlst_feat_remove();
    facts_mlst_feat_add(cmd->tmp_pool);

    /* This response is mandated by RFC3659, therefore it is not
     * localisable.
     */
    pr_response_add(R_200, "%s", method);
    return PR_HANDLED(cmd);
  }

  /* Do not show any facts by default at this point; the processing of the
   * facts requested by the client will enable just the ones the client
   * wishes to receive.
   */
  facts_opts = 0;
  facts_mlst_feat_remove();

  facts = cmd->argv[1];
  ptr = strchr(facts, ';');

  while (ptr) {
    pr_signals_handle();

    *ptr = '\0';

    if (strcasecmp(facts, "modify") == 0) {
      facts_opts |= FACTS_OPT_SHOW_MODIFY;
      resp_str = pstrcat(cmd->tmp_pool, resp_str, "modify;", NULL);

    } else if (strcasecmp(facts, "perm") == 0) {
      facts_opts |= FACTS_OPT_SHOW_PERM;
      resp_str = pstrcat(cmd->tmp_pool, resp_str, "perm;", NULL);

    } else if (strcasecmp(facts, "size") == 0) {
      facts_opts |= FACTS_OPT_SHOW_SIZE;
      resp_str = pstrcat(cmd->tmp_pool, resp_str, "size;", NULL);

    } else if (strcasecmp(facts, "type") == 0) {
      facts_opts |= FACTS_OPT_SHOW_TYPE;
      resp_str = pstrcat(cmd->tmp_pool, resp_str, "type;", NULL);

    } else if (strcasecmp(facts, "unique") == 0) {
      facts_opts |= FACTS_OPT_SHOW_UNIQUE;
      resp_str = pstrcat(cmd->tmp_pool, resp_str, "unique;", NULL);

    } else if (strcasecmp(facts, "UNIX.group") == 0) {
      facts_opts |= FACTS_OPT_SHOW_UNIX_GROUP;
      resp_str = pstrcat(cmd->tmp_pool, resp_str, "UNIX.group;", NULL);

    } else if (strcasecmp(facts, "UNIX.mode") == 0) {
      facts_opts |= FACTS_OPT_SHOW_UNIX_MODE;
      resp_str = pstrcat(cmd->tmp_pool, resp_str, "UNIX.mode;", NULL);

    } else if (strcasecmp(facts, "UNIX.owner") == 0) {
      facts_opts |= FACTS_OPT_SHOW_UNIX_OWNER;
      resp_str = pstrcat(cmd->tmp_pool, resp_str, "UNIX.owner;", NULL);

    } else {
      pr_log_debug(DEBUG3, MOD_FACTS_VERSION
        ": %s: client requested unsupported fact '%s'", method, facts);
    }

    *ptr = ';';
    facts = ptr + 1;
    ptr = strchr(facts, ';');
  }

  facts_mlst_feat_add(cmd->tmp_pool);

  /* This response is mandated by RFC3659, therefore it is not localisable. */
  pr_response_add(R_200, "%s %s", method, resp_str);
  return PR_HANDLED(cmd);
}

/* Configuration handlers
 */

/* usage: FactsAdvertise on|off */
MODRET set_factsadvertise(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

/* Initialization functions
 */

static int facts_init(void) {
  pr_help_add("MLSD", "[<sp> pathname]", TRUE);
  pr_help_add("MLST", "[<sp> pathname]", TRUE);

  return 0;
}

static int facts_sess_init(void) {
  config_rec *c;
  int advertise = TRUE;

  c = find_config(main_server->conf, CONF_PARAM, "FactsAdvertise", FALSE);
  if (c) {
    advertise = *((int *) c->argv[0]);
  }

  if (advertise == FALSE)
    return 0;

  facts_opts = FACTS_OPT_SHOW_MODIFY|FACTS_OPT_SHOW_PERM|FACTS_OPT_SHOW_SIZE|
    FACTS_OPT_SHOW_TYPE|FACTS_OPT_SHOW_UNIQUE|FACTS_OPT_SHOW_UNIX_GROUP|
    FACTS_OPT_SHOW_UNIX_MODE|FACTS_OPT_SHOW_UNIX_OWNER;

  /* XXX The media-type fact could be supported if mod_mimetype was available
   * and used.
   */

  pr_feat_add("MFF modify;UNIX.group;UNIX.mode;");
  pr_feat_add("MFMT");
  pr_feat_add("TVFS");

  facts_mlst_feat_add(session.pool);

  return 0;
}

/* Module API tables
 */

static conftable facts_conftab[] = {
  { "FactsAdvertise",	set_factsadvertise,	NULL },
  { NULL }
};

static cmdtable facts_cmdtab[] = {
  { CMD,	"MFF",		G_WRITE,facts_mff,  TRUE, FALSE, CL_WRITE },
  { CMD,	"MFMT",		G_WRITE,facts_mfmt, TRUE, FALSE, CL_WRITE },
  { CMD,	C_MLSD,		G_DIRS,	facts_mlsd, TRUE, FALSE, CL_DIRS },
  { CMD,	C_MLST,		G_DIRS,	facts_mlst, TRUE, FALSE, CL_DIRS },
  { CMD,	C_OPTS "_MLST", G_NONE, facts_opts_mlst, FALSE, FALSE },
  { 0, NULL }
};

module facts_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "facts",

  /* Module configuration handler table */
  facts_conftab,

  /* Module command handler table */
  facts_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  facts_init,

  /* Session initialization function */
  facts_sess_init,

  /* Module version */
  MOD_FACTS_VERSION
};
