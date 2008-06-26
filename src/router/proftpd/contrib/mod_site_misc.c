/*
 * ProFTPD: mod_site_misc -- a module implementing miscellaneous SITE commands
 *
 * Copyright (c) 2004-2006 The ProFTPD Project
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
 * $Id: mod_site_misc.c,v 1.3 2006/06/16 02:33:43 castaglia Exp $
 */

#include "conf.h"

#define MOD_SITE_MISC_VERSION		"mod_site_misc/1.1"

static int site_misc_check_filters(cmd_rec *cmd, const char *path) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  regex_t *preg = get_param_ptr(CURRENT_CONF, "PathAllowFilter", FALSE);

  if (preg &&
      regexec(preg, path, 0, NULL, 0) != 0) {
    pr_log_debug(DEBUG2, MOD_SITE_MISC_VERSION
      ": 'SITE %s' denied by PathAllowFilter", cmd->arg);
    return -1;
  }

  preg = get_param_ptr(CURRENT_CONF, "PathDenyFilter", FALSE);

  if (preg &&
      regexec(preg, path, 0, NULL, 0) == 0) {
    pr_log_debug(DEBUG2, MOD_SITE_MISC_VERSION
      ": 'SITE %s' denied by PathDenyFilter", cmd->arg);
    return -1;
  }
#endif

  return 0;
}

static int site_misc_create_dir(const char *dir) {
  struct stat st;
  int res;

  pr_fs_clear_cache();

  res = pr_fsio_stat(dir, &st);
  if (res == -1 &&
      errno != ENOENT) {
    pr_log_debug(DEBUG2, MOD_SITE_MISC_VERSION ": error checking '%s': %s",
      dir, strerror(errno));
    return -1;
  }

  if (res == 0)
    return 0;

  if (pr_fsio_mkdir(dir, 0777) < 0) {
    pr_log_debug(DEBUG2, MOD_SITE_MISC_VERSION ": error creating '%s': %s",
      dir, strerror(errno));
    return -1;
  }

  return 0;
}

static int site_misc_create_path(pool *p, const char *path) {
  struct stat st;
  char *curr_path, *dup_path;

  pr_fs_clear_cache();

  if (pr_fsio_stat(path, &st) == 0)
    return 0;

  dup_path = pstrdup(p, path);
  curr_path = session.cwd;
 
  while (dup_path && *dup_path) {
    char *curr_dir = strsep(&dup_path, "/");
 
    curr_path = pdircat(p, curr_path, curr_dir, NULL);
   
    if (site_misc_create_dir(curr_path) < 0)
      return -1;

    pr_signals_handle(); 
  }
 
  return 0;
}

static int site_misc_delete_dir(pool *p, const char *dir) {
  void *dirh;
  struct dirent *dent;

  dirh = pr_fsio_opendir(dir);
  if (!dirh)
    return -1;

  while ((dent = pr_fsio_readdir(dirh)) != NULL) {
    struct stat st;
    char *file;

    pr_signals_handle();

    if (strcmp(dent->d_name, ".") == 0 ||
        strcmp(dent->d_name, "..") == 0)
      continue;

    file = pdircat(p, dir, dent->d_name, NULL);

    if (pr_fsio_stat(file, &st) < 0)
      return -1;

    if (S_ISDIR(st.st_mode)) {
      if (site_misc_delete_dir(p, file) < 0)
        return -1;

    } else if (pr_fsio_unlink(file) < 0)
      return -1;
  }

  pr_fsio_closedir(dirh);

  if (pr_fsio_rmdir(dir) < 0)
    return -1;

  return 0;
}

static int site_misc_delete_path(pool *p, const char *path) {
  struct stat st;

  pr_fs_clear_cache();

  if (pr_fsio_stat(path, &st) < 0)
    return -1;

  if (!S_ISDIR(st.st_mode)) {
    errno = EINVAL;
    return -1;
  }

  return site_misc_delete_dir(p, path);
}

static time_t site_misc_mktime(unsigned int year, unsigned int month,
    unsigned int mday, unsigned int hour, unsigned int min) {
  struct tm tm;

  tm.tm_sec = 0;
  tm.tm_min = min;
  tm.tm_hour = hour;
  tm.tm_mday = mday;
  tm.tm_mon = (month - 1);
  tm.tm_year = (year - 1900);
  tm.tm_wday = 0;
  tm.tm_yday = 0;
  tm.tm_isdst = -1;
   
  return mktime(&tm);
}

/* Command handlers
 */

MODRET site_misc_mkdir(cmd_rec *cmd) {
  if (cmd->argc < 2)
    return PR_DECLINED(cmd);

  if (strcasecmp(cmd->argv[1], "MKDIR") == 0) {
    register unsigned int i;
    char *path = "";
    unsigned char *authenticated;

    if (cmd->argc < 3)
      return PR_DECLINED(cmd);

    authenticated = get_param_ptr(cmd->server->conf, "authenticated", FALSE);

    if (!authenticated || *authenticated == FALSE) {
      pr_response_add_err(R_530, "Please login with USER and PASS");
      return PR_ERROR(cmd);
    }

    for (i = 2; i < cmd->argc; i++)
      path = pstrcat(cmd->tmp_pool, path, *path ? " " : "", cmd->argv[i], NULL);

    path = pr_fs_decode_path(cmd->tmp_pool, path);

    if (site_misc_check_filters(cmd, path) < 0) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EPERM));
      return PR_ERROR(cmd);
    }

    if (!dir_check(cmd->tmp_pool, "SITE_MKDIR", G_WRITE, path, NULL)) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EPERM));
      return PR_ERROR(cmd);
    }

    if (site_misc_create_path(cmd->tmp_pool, path) < 0) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
      return PR_ERROR(cmd);
    }

    pr_response_add(R_200, "SITE %s command successful", cmd->argv[1]);
    return PR_HANDLED(cmd);
  }

  if (strcasecmp(cmd->argv[1], "HELP") == 0)
    pr_response_add(R_214, "MKDIR <sp> path");

  return PR_DECLINED(cmd);
}

MODRET site_misc_rmdir(cmd_rec *cmd) {
  if (cmd->argc < 2)
    return PR_DECLINED(cmd);

  if (strcasecmp(cmd->argv[1], "RMDIR") == 0) {
    register unsigned int i;
    char *path = "";
    unsigned char *authenticated;

    if (cmd->argc < 3)
      return PR_DECLINED(cmd);

    authenticated = get_param_ptr(cmd->server->conf, "authenticated", FALSE);

    if (!authenticated || *authenticated == FALSE) {
      pr_response_add_err(R_530, "Please login with USER and PASS");
      return PR_ERROR(cmd);
    }

    for (i = 2; i < cmd->argc; i++)
      path = pstrcat(cmd->tmp_pool, path, *path ? " " : "", cmd->argv[i], NULL);

    path = pr_fs_decode_path(cmd->tmp_pool, path);

    if (!dir_check(cmd->tmp_pool, "SITE_RMDIR", G_WRITE, path, NULL)) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EPERM));
      return PR_ERROR(cmd);
    }

    if (site_misc_delete_path(cmd->tmp_pool, path) < 0) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
      return PR_ERROR(cmd);
    }

    pr_response_add(R_200, "SITE %s command successful", cmd->argv[1]);
    return PR_HANDLED(cmd);
  } 

  if (strcasecmp(cmd->argv[1], "HELP") == 0)
    pr_response_add(R_214, "RMDIR <sp> path");

  return PR_DECLINED(cmd);
}

MODRET site_misc_symlink(cmd_rec *cmd) {
  if (cmd->argc < 2)
    return PR_DECLINED(cmd);

  if (strcasecmp(cmd->argv[1], "SYMLINK") == 0) {
    char *src, *dst;
    unsigned char *authenticated;

    if (cmd->argc < 4)
      return PR_DECLINED(cmd);

    authenticated = get_param_ptr(cmd->server->conf, "authenticated", FALSE);

    if (!authenticated || *authenticated == FALSE) {
      pr_response_add_err(R_530, "Please login with USER and PASS");
      return PR_ERROR(cmd);
    }

    src = pr_fs_decode_path(cmd->tmp_pool, cmd->argv[2]);

    if (!dir_check(cmd->tmp_pool, "SITE_SYMLINK", G_WRITE, src, NULL)) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg[2], strerror(EPERM));
      return PR_ERROR(cmd);
    }

    dst = pr_fs_decode_path(cmd->tmp_pool, cmd->argv[3]);

    if (!dir_check(cmd->tmp_pool, "SITE_SYMLINK", G_WRITE, dst, NULL)) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg[3], strerror(EPERM));
      return PR_ERROR(cmd);
    }

    if (site_misc_check_filters(cmd, dst) < 0) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EPERM));
      return PR_ERROR(cmd);
    }

    if (pr_fsio_symlink(src, dst) < 0) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
      return PR_ERROR(cmd);
    }

    pr_response_add(R_200, "SITE %s command successful", cmd->argv[1]);
    return PR_HANDLED(cmd);
  } 

  if (strcasecmp(cmd->argv[1], "HELP") == 0)
    pr_response_add(R_214, "SYMLINK <sp> source <sp> destination");

  return PR_DECLINED(cmd);
}

MODRET site_misc_utime(cmd_rec *cmd) {
  if (cmd->argc < 2)
    return PR_DECLINED(cmd);

  if (strcasecmp(cmd->argv[1], "UTIME") == 0) {
    register unsigned int i;
    char c, *p, *path = "";
    unsigned int year, month, day, hour, min;
    struct utimbuf tmbuf;
    unsigned char *authenticated;

    if (cmd->argc < 4)
      return PR_DECLINED(cmd);

    authenticated = get_param_ptr(cmd->server->conf, "authenticated", FALSE);

    if (!authenticated || *authenticated == FALSE) {
      pr_response_add_err(R_530, "Please login with USER and PASS");
      return PR_ERROR(cmd);
    }

    if (strlen(cmd->argv[2]) != 12) {
      pr_response_add_err(R_500, "%s: %s", cmd->arg, strerror(EINVAL));
      return PR_ERROR(cmd);
    }

    for (i = 3; i < cmd->argc; i++)
      path = pstrcat(cmd->tmp_pool, path, *path ? " " : "", cmd->argv[i], NULL);

    path = pr_fs_decode_path(cmd->tmp_pool, path);

    if (!dir_check(cmd->tmp_pool, "SITE_UTIME", G_WRITE, path, NULL)) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EPERM));
      return PR_ERROR(cmd);
    }

    if (site_misc_check_filters(cmd, path) < 0) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(EPERM));
      return PR_ERROR(cmd);
    }

    p = cmd->argv[2];
    c = cmd->argv[2][4];
    cmd->argv[2][4] = '\0';
    year = atoi(p);
   
    cmd->argv[2][4] = c;
    p = &(cmd->argv[2][4]);
    c = cmd->argv[2][6];
    cmd->argv[2][6] = '\0';
    month = atoi(p);

    cmd->argv[2][6] = c;
    p = &(cmd->argv[2][6]);
    c = cmd->argv[2][8];
    cmd->argv[2][8] = '\0';
    day = atoi(p);

    cmd->argv[2][8] = c;
    p = &(cmd->argv[2][8]);
    c = cmd->argv[2][10];
    cmd->argv[2][10] = '\0';
    hour = atoi(p);

    cmd->argv[2][10] = c;
    p = &(cmd->argv[2][10]);
    min = atoi(p);

    tmbuf.actime = tmbuf.modtime = site_misc_mktime(year, month, day, hour,
      min);

    if (utime(path, &tmbuf) < 0) {
      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
      return PR_ERROR(cmd);
    }
 
    pr_response_add(R_200, "SITE %s command successful", cmd->argv[1]);
    return PR_HANDLED(cmd);
  }

  if (strcasecmp(cmd->argv[1], "HELP") == 0)
    pr_response_add(R_214, "UTIME <sp> YYYYMMDDhhmm <sp> path");

  return PR_DECLINED(cmd);
}

/* Initialization functions
 */

/* Module API tables
 */

static cmdtable site_misc_cmdtab[] = {
  { CMD, C_SITE, G_WRITE, site_misc_mkdir,	FALSE,	FALSE, CL_MISC },
  { CMD, C_SITE, G_WRITE, site_misc_rmdir,	FALSE,	FALSE, CL_MISC },
  { CMD, C_SITE, G_WRITE, site_misc_symlink,	FALSE,	FALSE, CL_MISC },
  { CMD, C_SITE, G_WRITE, site_misc_utime,	FALSE,	FALSE, CL_MISC },
  { 0, NULL }
};

module site_misc_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "site_misc",

  /* Module configuration handler table */
  NULL,

  /* Module command handler table */
  site_misc_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  NULL,

  /* Session initialization function */
  NULL,

  /* Module version */
  MOD_SITE_MISC_VERSION
};
