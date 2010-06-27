/*
 * ProFTPD: mod_auth_file - file-based authentication module that supports
 *                          restrictions on the file contents
 *
 * Copyright (c) 2002-2007 The ProFTPD Project team
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
 * As a special exemption, the ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 *
 * $Id: mod_auth_file.c,v 1.34 2009/12/10 17:40:01 castaglia Exp $
 */

#include "conf.h"

/* AIX has some rather stupid function prototype inconsistencies between
 * their crypt.h and stdlib.h's setkey() declarations.
 */
#if defined(HAVE_CRYPT_H) && !defined(AIX4) && !defined(AIX5)
# include <crypt.h>
#endif

#define MOD_AUTH_FILE_VERSION	"mod_auth_file/0.8.3"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001020702
# error "ProFTPD 1.2.7rc2 or later required"
#endif

#ifndef BUFSIZ
# define BUFSIZ          PR_TUNABLE_BUFFER_SIZE
#endif /* !BUFSIZ */

typedef union {
  uid_t uid;
  gid_t gid;

} authfile_id_t;

typedef struct file_rec {
  char *af_path;
  FILE *af_file;

  unsigned char af_restricted_ids;
  authfile_id_t af_min_id;
  authfile_id_t af_max_id;

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  unsigned char af_restricted_names;
  char *af_name_filter;
  regex_t *af_name_regex;
  unsigned char af_name_regex_inverted;

  /* These are AuthUserFile-specific */
  unsigned char af_restricted_homes;
  char *af_home_filter;
  regex_t *af_home_regex;
  unsigned char af_home_regex_inverted;

#endif /* !HAVE_REGEX_H and !HAVE_REGCOMP */

} authfile_file_t;

/* List of server-specific Authiles */
static authfile_file_t *af_user_file = NULL;
static authfile_file_t *af_group_file = NULL;

extern unsigned char persistent_passwd;

static int af_setpwent(void);
static int af_setgrent(void);

/* Support routines.  Move the passwd/group functions out of lib/ into here.
 */

#ifndef HAVE_FGETPWENT

#define NPWDFIELDS      7

static char pwdbuf[BUFSIZ];
static char *pwdfields[NPWDFIELDS];
static struct passwd pwent;

static struct passwd *af_getpasswd(const char *buf) {
  register unsigned int i;
  register char *cp = NULL;
  char *ep = NULL, *buffer = NULL;
  char **fields = NULL;
  struct passwd *pwd = NULL;

  fields = pwdfields;
  buffer = pwdbuf;
  pwd = &pwent;

  strncpy(buffer, buf, BUFSIZ-1);
  buffer[BUFSIZ-1] = '\0';

  for (cp = buffer, i = 0; i < NPWDFIELDS && cp; i++) {
    fields[i] = cp;
    while (*cp && *cp != ':')
      ++cp;

    if (*cp)
      *cp++ = '\0';

    else
      cp = 0;
  }

  if (i != NPWDFIELDS || *fields[2] == '\0' || *fields[3] == '\0')
    return NULL;

  pwd->pw_name = fields[0];
  pwd->pw_passwd = fields[1];

  if (fields[2][0] == '\0' ||
     ((pwd->pw_uid = strtol(fields[2], &ep, 10)) == 0 && *ep))
       return NULL;

  if (fields[3][0] == '\0' ||
     ((pwd->pw_gid = strtol(fields[3], &ep, 10)) == 0 && *ep))
       return NULL;

  pwd->pw_gecos = fields[4];
  pwd->pw_dir = fields[5];
  pwd->pw_shell = fields[6];

  return pwd;
}
#endif /* !HAVE_FGETPWENT */

#ifndef HAVE_FGETGRENT

#define MAXMEMBERS	4096
#define NGRPFIELDS      4

static char *grpbuf = NULL;
static struct group grent;
static char *grpfields[NGRPFIELDS];
static char *members[MAXMEMBERS+1];

static char *af_getgrentline(char **buf, int *buflen, FILE *fp) {
  char *cp = *buf;

  while (fgets(cp, (*buflen) - (cp - *buf), fp) != NULL) {
    pr_signals_handle();

    /* Is this a full line? */
    if (strchr(cp, '\n'))
      return *buf;

    /* No -- allocate a larger buffer, doubling buflen. */
    *buflen += *buflen;

    {
      char *new_buf;

      new_buf = realloc(*buf, *buflen);
      if (new_buf == NULL)
        break;

      *buf = new_buf;
    }

    cp = *buf + (cp - *buf);
    cp = strchr(cp, '\0');
  }

  free(*buf);
  *buf = NULL;
  *buflen = 0;

  return NULL;
}

static char **af_getgrmems(char *s) {
  int nmembers = 0;

  while (s && *s && nmembers < MAXMEMBERS) {
    members[nmembers++] = s;
    while (*s && *s != ',')
      s++;

    if (*s)
      *s++ = '\0';
  }

  members[nmembers] = NULL;
  return members;
}

static struct group *af_getgrp(const char *buf) {
  int i;
  char *cp;

  i = strlen(buf) + 1;

  if (!grpbuf) {
    grpbuf = malloc(i);

  } else {
    char *new_buf;

    new_buf = realloc(grpbuf, i);
    if (new_buf == NULL)
      return NULL;

    grpbuf = new_buf;
  }

  if (!grpbuf)
    return NULL;

  sstrncpy(grpbuf, buf, i);

  cp = strrchr(grpbuf, '\n');
  if (cp)
    *cp = '\0';

  for (cp = grpbuf, i = 0; i < NGRPFIELDS && cp; i++) {
    grpfields[i] = cp;

    cp = strchr(cp, ':');
    if (cp)
      *cp++ = 0;
  }

  if (i < (NGRPFIELDS - 1)) {
    pr_log_pri(PR_LOG_ERR, "Malformed entry in group file: %s", buf);
    return NULL;
  }

  if (*grpfields[2] == '\0')
    return NULL;

  grent.gr_name = grpfields[0];
  grent.gr_passwd = grpfields[1];
  grent.gr_gid = atoi(grpfields[2]);
  grent.gr_mem = af_getgrmems(grpfields[3]);

  return &grent;
}
#endif /* !HAVE_FGETGRENT */

static int af_allow_grent(struct group *grp) {
  if (!af_group_file) {
    errno = EPERM;
    return -1;
  }

  /* Check that the grent is within the ID restrictions (if present). */
  if (af_group_file->af_restricted_ids) {

    if (grp->gr_gid < af_group_file->af_min_id.gid) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping group '%s': "
        "GID (%u) below the minimum allowed (%u)", grp->gr_name,
        (unsigned int) grp->gr_gid,
        (unsigned int) af_group_file->af_min_id.gid);
      errno = EINVAL;
      return -1;
    }

    if (grp->gr_gid > af_group_file->af_max_id.gid) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping group '%s': "
        "GID (%u) above the maximum allowed (%u)", grp->gr_name,
        (unsigned int) grp->gr_gid,
        (unsigned int) af_group_file->af_max_id.gid);
      errno = EINVAL;
      return -1;
    }
  }

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  /* Check if the grent has an acceptable name. */
  if (af_group_file->af_restricted_names) {
    int res = regexec(af_group_file->af_name_regex, grp->gr_name, 0, NULL, 0);

    if ((res != 0 && !af_group_file->af_name_regex_inverted) ||
        (res == 0 && af_group_file->af_name_regex_inverted)) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping group '%s': "
        "name '%s' does not meet allowed filter '%s'", grp->gr_name,
        grp->gr_name, af_group_file->af_name_filter);
      errno = EINVAL;
      return -1;
    }
  }
#endif /* !HAVE_REGEX_H and !HAVE_REGCOMP */

  return 0;
}

static void af_endgrent(void) {
  if (af_group_file &&
      af_group_file->af_file) {
    fclose(af_group_file->af_file);
    af_group_file->af_file = NULL;
  }

  return;
}

static struct group *af_getgrent(void) {
  struct group *grp = NULL;

  if (!af_group_file ||
      !af_group_file->af_file) {
    errno = EINVAL;
    return NULL;
  }

  while (TRUE) {
#ifdef HAVE_FGETGRENT
    pr_signals_handle();
    grp = fgetgrent(af_group_file->af_file);
#else
    char *cp = NULL, *buf = NULL;
    int buflen = BUFSIZ;

    pr_signals_handle();

    buf = malloc(BUFSIZ);
    if (!buf)
      return NULL;

    while (af_getgrentline(&buf, &buflen, af_group_file->af_file) != NULL) {

      /* Ignore comment and empty lines */
      if (buf[0] == '\0' || buf[0] == '#')
        continue;

      cp = strchr(buf, '\n');
      if (cp != NULL)
        *cp = '\0';

      grp = af_getgrp(buf);
      free(buf);

      break;
    }
#endif /* !HAVE_FGETGRENT */

    /* If grp is NULL now, the file is empty - nothing more to be read. */
    if (!grp)
      break;

    if (af_allow_grent(grp) < 0)
      continue;

    break;
  }

  return grp;
}

static struct group *af_getgrnam(const char *name) {
  struct group *grp = NULL;

  if (af_setgrent() < 0)
    return NULL;

  while ((grp = af_getgrent()) != NULL) {
    if (strcmp(name, grp->gr_name) == 0) {

      /* Found the requested group */
      break;
    }
  }

  return grp;
}

static struct group *af_getgrgid(gid_t gid) {
  struct group *grp = NULL;

  if (af_setgrent() < 0)
    return NULL;

  while ((grp = af_getgrent()) != NULL) {
    if (grp->gr_gid == gid) {

      /* Found the requested GID */
      break;
    }
  }

  return grp;
}

static int af_setgrent(void) {

  if (af_group_file) {
    if (af_group_file->af_file) {
      /* If already opened, rewind */
      rewind(af_group_file->af_file);
      return 0;

    } else {
      af_group_file->af_file = fopen(af_group_file->af_path, "r");
      if (af_group_file->af_file == NULL) {
        pr_log_pri(PR_LOG_ERR, "error: unable to open group file '%s': %s",
          af_group_file->af_path, strerror(errno));
        return -1;
      }

      pr_log_debug(DEBUG7, MOD_AUTH_FILE_VERSION ": using group file '%s'",
        af_group_file->af_path);
      return 0;
    }
  }

  errno = EPERM;
  return -1;
}

static int af_allow_pwent(struct passwd *pwd) {
  if (!af_user_file) {
    errno = EPERM;
    return -1;
  }

  /* Check that the pwent is within the ID restrictions (if present). */
  if (af_user_file->af_restricted_ids) {

    if (pwd->pw_uid < af_user_file->af_min_id.uid) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping user '%s': "
        "UID (%u) below the minimum allowed (%u)", pwd->pw_name,
        (unsigned int) pwd->pw_uid, (unsigned int) af_user_file->af_min_id.uid);
      errno = EINVAL;
      return -1;
    }

    if (pwd->pw_uid > af_user_file->af_max_id.gid) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping user '%s': "
        "UID (%u) above the maximum allowed (%u)", pwd->pw_name,
        (unsigned int) pwd->pw_uid, (unsigned int) af_user_file->af_max_id.uid);
      errno = EINVAL;
      return -1;
    }
  }

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  /* Check if the pwent has an acceptable name. */
  if (af_user_file->af_restricted_names) {
    int res = regexec(af_user_file->af_name_regex, pwd->pw_name, 0, NULL, 0);

    if ((res != 0 && !af_user_file->af_name_regex_inverted) ||
        (res == 0 && af_user_file->af_name_regex_inverted)) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping user '%s': "
        "name '%s' does not meet allowed filter '%s'", pwd->pw_name,
        pwd->pw_name, af_user_file->af_name_filter);
      errno = EINVAL;
      return -1;
    }
  }

  /* Check if the pwent has an acceptable home directory. */
  if (af_user_file->af_restricted_homes) {

    int res = regexec(af_user_file->af_home_regex, pwd->pw_dir, 0, NULL, 0);

    if ((res != 0 && !af_user_file->af_home_regex_inverted) ||
        (res == 0 && af_user_file->af_home_regex_inverted)) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping user '%s': "
        "home '%s' does not meet allowed filter '%s'", pwd->pw_name,
        pwd->pw_dir, af_user_file->af_home_filter);
      errno = EINVAL;
      return -1;
    }
  }
#endif /* !HAVE_REGEX_H and !HAVE_REGCOMP */

  return 0;
}

static void af_endpwent(void) {
  if (af_user_file &&
      af_user_file->af_file) {
    fclose(af_user_file->af_file);
    af_user_file->af_file = NULL;
  }

  return;
}

static struct passwd *af_getpwent(void) {
  struct passwd *pwd = NULL;

  if (!af_user_file ||
      !af_user_file->af_file) {
    errno = EINVAL;
    return NULL;
  }

  while (TRUE) {
#ifdef HAVE_FGETPWENT
    pr_signals_handle();
    pwd = fgetpwent(af_user_file->af_file);
#else
    char buf[BUFSIZ] = {'\0'};

    pr_signals_handle();
    while (fgets(buf, sizeof(buf), af_user_file->af_file) != (char*) 0) {
      pr_signals_handle();

      /* Ignore empty and comment lines */
      if (buf[0] == '\0' || buf[0] == '#')
        continue;

      buf[strlen(buf)-1] = '\0';
      pwd = af_getpasswd(buf);
      break;
    }
#endif /* !HAVE_FGETPWENT */

    /* If pwd is NULL now, the file is empty - nothing more to be read. */
    if (!pwd)
      break;

    if (af_allow_pwent(pwd) < 0)
      continue;

    break;
  }

  return pwd;
}

static struct passwd *af_getpwnam(const char *name) {
  struct passwd *pwd = NULL;

  if (af_setpwent() < 0)
    return NULL;

  while ((pwd = af_getpwent()) != NULL) {
    if (strcmp(name, pwd->pw_name) == 0) {

      /* Found the requested user */
      break;
    }
  }

  return pwd;
}

static char *af_getpwpass(const char *name) {
  struct passwd *pwd = af_getpwnam(name);
  return pwd ? pwd->pw_passwd : NULL;
}

static struct passwd *af_getpwuid(uid_t uid) {
  struct passwd *pwd = NULL;

  if (af_setpwent() < 0)
    return NULL;

  while ((pwd = af_getpwent()) != NULL) {
    if (pwd->pw_uid == uid) {

      /* Found the requested UID */
      break;
    }
  }

  return pwd;
}

static int af_setpwent(void) {

  if (af_user_file) {
    if (af_user_file->af_file) {
      /* If already opened, rewind */
      rewind(af_user_file->af_file);
      return 0;

    } else {
      af_user_file->af_file = fopen(af_user_file->af_path, "r");
      if (af_user_file->af_file == NULL) {
        pr_log_pri(PR_LOG_ERR, "error: unable to open passwd file '%s': %s",
          af_user_file->af_path, strerror(errno));
        return -1;
      }

      pr_log_debug(DEBUG7, MOD_AUTH_FILE_VERSION ": using passwd file '%s'",
        af_user_file->af_path);
      return 0;
    }
  }

  errno = EPERM;
  return -1;
}

/* Authentication handlers.
 */

MODRET authfile_endpwent(cmd_rec *cmd) {
  af_endpwent();
  return PR_DECLINED(cmd);
}

MODRET authfile_getpwent(cmd_rec *cmd) {
  struct passwd *pwd = NULL;

  pwd = af_getpwent();

  return pwd ? mod_create_data(cmd, pwd) : PR_DECLINED(cmd);
}

MODRET authfile_getpwnam(cmd_rec *cmd) {
  struct passwd *pwd = NULL;
  const char *name = cmd->argv[0];

  if (af_setpwent() < 0)
    return PR_DECLINED(cmd);

  /* Ugly -- we iterate through the file.  Time-consuming. */
  while ((pwd = af_getpwent()) != NULL) {
    if (strcmp(name, pwd->pw_name) == 0) {

      /* Found the requested name */
      break;
    }
  }

  return pwd ? mod_create_data(cmd, pwd) : PR_DECLINED(cmd);
}

MODRET authfile_getpwuid(cmd_rec *cmd) {
  struct passwd *pwd = NULL;
  uid_t uid = *((uid_t *) cmd->argv[0]);

  if (af_setpwent() < 0)
    return PR_DECLINED(cmd);

  pwd = af_getpwuid(uid);

  return pwd ? mod_create_data(cmd, pwd) : PR_DECLINED(cmd);
}

MODRET authfile_name2uid(cmd_rec *cmd) {
  struct passwd *pwd = NULL;

  if (af_setpwent() < 0)
    return PR_DECLINED(cmd);

  pwd = af_getpwnam(cmd->argv[0]);

  return pwd ? mod_create_data(cmd, (void *) &pwd->pw_uid) : PR_DECLINED(cmd);
}

MODRET authfile_setpwent(cmd_rec *cmd) {
  if (af_setpwent() == 0)
    return PR_DECLINED(cmd);

  return PR_DECLINED(cmd);
}

MODRET authfile_uid2name(cmd_rec *cmd) {
  struct passwd *pwd = NULL;

  if (af_setpwent() < 0)
    return PR_DECLINED(cmd);

  pwd = af_getpwuid(*((uid_t *) cmd->argv[0]));

  return pwd ? mod_create_data(cmd, pwd->pw_name) : PR_DECLINED(cmd);
}

MODRET authfile_endgrent(cmd_rec *cmd) {
  af_endgrent();
  return PR_DECLINED(cmd);
}

MODRET authfile_getgrent(cmd_rec *cmd) {
  struct group *grp = NULL;

  grp = af_getgrent();

  return grp ? mod_create_data(cmd, grp) : PR_DECLINED(cmd);
}

MODRET authfile_getgrgid(cmd_rec *cmd) {
  struct group *grp = NULL;
  gid_t gid = *((gid_t *) cmd->argv[0]);

  if (af_setgrent() < 0)
    return PR_DECLINED(cmd);

  grp = af_getgrgid(gid);

  return grp ? mod_create_data(cmd, grp) : PR_DECLINED(cmd);
}

MODRET authfile_getgrnam(cmd_rec *cmd) {
  struct group *grp = NULL;
  const char *name = cmd->argv[0];

  if (af_setgrent() < 0)
    return PR_DECLINED(cmd);

  while ((grp = af_getgrent()) != NULL) {
    if (strcmp(name, grp->gr_name) == 0) {

      /* Found the name requested */
      break;
    }
  }

  return grp ? mod_create_data(cmd, grp) : PR_DECLINED(cmd);
}

MODRET authfile_getgroups(cmd_rec *cmd) {
  struct passwd *pwd = NULL;
  struct group *grp = NULL;
  array_header *gids = NULL, *groups = NULL;
  char *name = cmd->argv[0];

  if (name == NULL) {
    return PR_DECLINED(cmd);
  }

  if (af_setpwent() < 0) {
    return PR_DECLINED(cmd);
  }

  if (af_setgrent() < 0) {
    return PR_DECLINED(cmd);
  }

  /* Check for NULLs */
  if (cmd->argv[1])
    gids = (array_header *) cmd->argv[1];

  if (cmd->argv[2])
    groups = (array_header *) cmd->argv[2];

  /* Retrieve the necessary info. */
  pwd = af_getpwnam(name);
  if (pwd == NULL) {
    return PR_DECLINED(cmd);
  }

  /* Populate the first group ID and name. */
  if (gids) {
    *((gid_t *) push_array(gids)) = pwd->pw_gid;
  }

  if (groups &&
      (grp = af_getgrgid(pwd->pw_gid)) != NULL) {
    *((char **) push_array(groups)) = pstrdup(session.pool, grp->gr_name);
  }

  af_setgrent();

  /* This is where things get slow, expensive, and ugly.  Loop through
   * everything, checking to make sure we haven't already added it.
   */
  while ((grp = af_getgrent()) != NULL &&
      grp->gr_mem) {
    char **gr_mems = NULL;

    pr_signals_handle();

    /* Loop through each member name listed */
    for (gr_mems = grp->gr_mem; *gr_mems; gr_mems++) {

      /* If it matches the given username... */
      if (strcmp(*gr_mems, pwd->pw_name) == 0) {

        /* ...add the GID and name */
        if (gids)
          *((gid_t *) push_array(gids)) = grp->gr_gid;

        if (groups)
          *((char **) push_array(groups)) = pstrdup(session.pool, grp->gr_name);
      }
    }
  }

  if (gids && gids->nelts > 0) {
    return mod_create_data(cmd, (void *) &gids->nelts);

  } else if (groups && groups->nelts > 0) {
    return mod_create_data(cmd, (void *) &groups->nelts);
  }

  return PR_DECLINED(cmd);
}

MODRET authfile_gid2name(cmd_rec *cmd) {
  struct group *grp = NULL;

  if (af_setgrent() < 0)
    return PR_DECLINED(cmd);

  grp = af_getgrgid(*((gid_t *) cmd->argv[0]));

  return grp ? mod_create_data(cmd, grp->gr_name) : PR_DECLINED(cmd);
}

MODRET authfile_name2gid(cmd_rec *cmd) {
  struct group *grp = NULL;

  if (af_setgrent() < 0)
    return PR_DECLINED(cmd);

  grp = af_getgrnam(cmd->argv[0]);

  return grp ? mod_create_data(cmd, (void *) &grp->gr_gid) : PR_DECLINED(cmd);
}

MODRET authfile_setgrent(cmd_rec *cmd) {
  if (af_setgrent() == 0)
    return PR_DECLINED(cmd);

  return PR_DECLINED(cmd);
}

MODRET authfile_auth(cmd_rec *cmd) {
  char *tmp = NULL, *cleartxt_pass = NULL;
  const char *name = cmd->argv[0];

  if (af_setpwent() < 0)
    return PR_DECLINED(cmd);

  /* Lookup the cleartxt password for this user. */
  tmp = af_getpwpass(name);
  if (tmp == NULL) {

    /* For now, return DECLINED.  Ideally, we could stash an auth module
     * identifier in the session structure, so that all auth modules could
     * coordinate/use their methods as long as they matched the auth module
     * used.
     */
    return PR_DECLINED(cmd);

#if 0
    /* When the above is implemented, and if the user being checked was
     * provided by mod_auth_file, we'd return this.
     */
    return PR_ERROR_INT(cmd, PR_AUTH_NOPWD);
#endif
  }

  cleartxt_pass = pstrdup(cmd->tmp_pool, tmp);

  if (pr_auth_check(cmd->tmp_pool, cleartxt_pass, name, cmd->argv[1]))
    return PR_ERROR_INT(cmd, PR_AUTH_BADPWD);

  session.auth_mech = "mod_auth_file.c";
  return PR_HANDLED(cmd);
}

MODRET authfile_chkpass(cmd_rec *cmd) {
  const char *ciphertxt_pass = cmd->argv[0];
  const char *cleartxt_pass = cmd->argv[2];
  char *crypted_pass = NULL;

  if (!ciphertxt_pass) {
    pr_log_debug(DEBUG2, MOD_AUTH_FILE_VERSION
      ": missing ciphertext password for comparison");
    return PR_DECLINED(cmd);
  }

  if (!cleartxt_pass) {
    pr_log_debug(DEBUG2, MOD_AUTH_FILE_VERSION
      ": missing client-provided password for comparison");
    return PR_DECLINED(cmd);
  }

  /* Even though the AuthUserFile is not used here, there must be one
   * configured before this function should attempt to check the password.
   * Otherwise, it could be checking a password retrieved by some other
   * auth module.
   */
  if (!af_user_file)
    return PR_DECLINED(cmd);

  crypted_pass = crypt(cleartxt_pass, ciphertxt_pass);
  if (!crypted_pass) {
    pr_log_debug(DEBUG0, MOD_AUTH_FILE_VERSION
      ": error using crypt(3): %s", strerror(errno));
    return PR_DECLINED(cmd);
  }

  if (strcmp(crypted_pass, ciphertxt_pass) == 0) {
    session.auth_mech = "mod_auth_file.c";
    return PR_HANDLED(cmd);
  }  

  return PR_DECLINED(cmd);
}

/* Configuration handlers
 */

/* usage: AuthGroupFile path [id <min-max>] [name <regex>] */
MODRET set_authgroupfile(cmd_rec *cmd) {
  config_rec *c = NULL;
  authfile_file_t *file = NULL;

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  if (cmd->argc-1 < 1 || cmd->argc-1 > 5)
#else
  if (cmd->argc-1 < 1 || cmd->argc-1 > 2)
#endif /* !HAVE_REGEX_H and !HAVE_REGCOMP */
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (*(cmd->argv[1]) != '/')
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "unable to use relative path for ", cmd->argv[0], " '",
      cmd->argv[1], "'.", NULL));

  c = add_config_param(cmd->argv[0], 1, NULL);

  file = pcalloc(c->pool, sizeof(authfile_file_t));
  file->af_path = pstrdup(c->pool, cmd->argv[1]);
  c->argv[0] = (void *) file;

  /* Check for restrictions */
  if (cmd->argc-1 != 1) {
    register unsigned int i = 0;

    for (i = 2; i < cmd->argc; i++) {
      if (strcmp(cmd->argv[i], "id") == 0) {
        gid_t min, max;
        char *sep = NULL, *tmp = NULL;

        /* The range restriction parameter is of the form "min-max", where max
         * must be >= min.
         */

        sep = strchr(cmd->argv[++i], '-');
        if (sep == NULL)
          CONF_ERROR(cmd, "badly formatted ID restriction parameter");

        *sep = '\0';

        min = strtol(cmd->argv[i], &tmp, 10);

        if (tmp && *tmp)
          CONF_ERROR(cmd, "badly formatted minimum ID");

        tmp = NULL;

        max = strtol(sep+1, &tmp, 10);

        if (tmp && *tmp)
          CONF_ERROR(cmd, "badly formatted maximum ID");

        if (min > max)
          CONF_ERROR(cmd, "minimum cannot be larger than maximum");

        file->af_min_id.gid = min;
        file->af_max_id.gid = max;
        file->af_restricted_ids = TRUE;

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
      } else if (strcmp(cmd->argv[i], "name") == 0) {
        char *filter = cmd->argv[++i];
        regex_t *preg = NULL;
        int res = 0;

        preg = pr_regexp_alloc();

        /* Check for a ! negation/inversion filter prefix. */
        if (*filter == '!') {
          filter++;
          file->af_name_regex_inverted = TRUE;
        }

        res = regcomp(preg, filter, REG_EXTENDED|REG_NOSUB);
        if (res != 0) {
          char errstr[200] = {'\0'};

          regerror(res, preg, errstr, sizeof(errstr));
          pr_regexp_free(preg);

          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", filter, "' failed "
            "regex compilation: ", errstr, NULL));
        }

        file->af_name_filter = pstrdup(c->pool, cmd->argv[i]);
        file->af_name_regex = preg;
        file->af_restricted_names = TRUE;

#endif /* !HAVE_REGEX_H && !HAVE_REGCOMP */

      } else
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown restriction '",
          cmd->argv[i], "'", NULL));
    }
  }

  return PR_HANDLED(cmd);
}

/* usage: AuthUserFile path [home <regexp>] [id <min-max>] [name <regex>] */
MODRET set_authuserfile(cmd_rec *cmd) {
  config_rec *c = NULL;
  authfile_file_t *file = NULL;

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  if (cmd->argc-1 < 1 || cmd->argc-1 > 7)
#else
  if (cmd->argc-1 < 1 || cmd->argc-1 > 2)
#endif /* !HAVE_REGEX_H and !HAVE_REGCOMP */
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (*(cmd->argv[1]) != '/')
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "unable to use relative path for ", cmd->argv[0], " '",
      cmd->argv[1], "'.", NULL));

  c = add_config_param(cmd->argv[0], 1, NULL);

  file = pcalloc(c->pool, sizeof(authfile_file_t));
  file->af_path = pstrdup(c->pool, cmd->argv[1]);
  c->argv[0] = (void *) file;

  /* Check for restrictions */
  if (cmd->argc-1 != 1) {
    register unsigned int i = 0;

    for (i = 2; i < cmd->argc; i++) {
      if (strcmp(cmd->argv[i], "id") == 0) {
        uid_t min, max;
        char *sep = NULL, *tmp = NULL;

        /* The range restriction parameter is of the form "min-max", where max
         * must be >= min.
         */

        sep = strchr(cmd->argv[++i], '-');
        if (sep == NULL)
          CONF_ERROR(cmd, "badly formatted ID restriction parameter");

        *sep = '\0';

        min = strtol(cmd->argv[i], &tmp, 10);

        if (tmp && *tmp)
          CONF_ERROR(cmd, "badly formatted minimum ID");

        tmp = NULL;

        max = strtol(sep+1, &tmp, 10);

        if (tmp && *tmp)
          CONF_ERROR(cmd, "badly formatted maximum ID");

        if (min > max)
          CONF_ERROR(cmd, "minimum cannot be larger than maximum");

        file->af_min_id.uid = min;
        file->af_max_id.uid = max;
        file->af_restricted_ids = TRUE;

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
      } else if (strcmp(cmd->argv[i], "home") == 0) {
        char *filter = cmd->argv[++i];
        regex_t *preg = NULL;
        int res = 0;

        preg = pr_regexp_alloc();

        /* Check for a ! negation/inversion filter prefix. */
        if (*filter == '!') {
          filter++;
          file->af_home_regex_inverted = TRUE;
        }

        res = regcomp(preg, filter, REG_EXTENDED|REG_NOSUB);
        if (res != 0) {
          char errstr[200] = {'\0'};

          regerror(res, preg, errstr, sizeof(errstr));
          pr_regexp_free(preg);

          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", filter, "' failed "
            "regex compilation: ", errstr, NULL));
        }

        file->af_home_filter = pstrdup(c->pool, cmd->argv[i]);
        file->af_home_regex = preg;
        file->af_restricted_homes = TRUE;

      } else if (strcmp(cmd->argv[i], "name") == 0) {
        char *filter = cmd->argv[++i];
        regex_t *preg = NULL;
        int res = 0;

        preg = pr_regexp_alloc();

        /* Check for a ! negation/inversion filter prefix. */
        if (*filter == '!') {
          filter++;
          file->af_name_regex_inverted = TRUE;
        }

        res = regcomp(preg, filter, REG_EXTENDED|REG_NOSUB);
        if (res != 0) {
          char errstr[200] = {'\0'};

          regerror(res, preg, errstr, sizeof(errstr));
          pr_regexp_free(preg);

          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", filter, "' failed "
            "regex compilation: ", errstr, NULL));
        }

        file->af_name_filter = pstrdup(c->pool, cmd->argv[i]);
        file->af_name_regex = preg;
        file->af_restricted_names = TRUE;

#endif /* !HAVE_REGEX_H && !HAVE_REGCOMP */

      } else
        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown restriction '",
          cmd->argv[i], "'", NULL));
    }
  }

  return PR_HANDLED(cmd);
}

/* Initialization routines
 */

static int authfile_sess_init(void) {
  config_rec *c = NULL;

  c = find_config(main_server->conf, CONF_PARAM, "AuthUserFile", FALSE);
  if (c) {
    af_user_file = c->argv[0];
  }

  c = find_config(main_server->conf, CONF_PARAM, "AuthGroupFile", FALSE);
  if (c) {
    af_group_file = c->argv[0];
  }

  return 0;
}

/* Module API tables
 */

static conftable authfile_conftab[] = {
  { "AuthGroupFile",	set_authgroupfile,	NULL },
  { "AuthUserFile",	set_authuserfile,	NULL },
  { NULL }
};

static authtable authfile_authtab[] = {

  /* User information callbacks */
  { 0, "endpwent",	authfile_endpwent },
  { 0, "getpwent",	authfile_getpwent },
  { 0, "getpwnam",	authfile_getpwnam },
  { 0, "getpwuid",	authfile_getpwuid },
  { 0, "name2uid",	authfile_name2uid },
  { 0, "setpwent",	authfile_setpwent },
  { 0, "uid2name",	authfile_uid2name },

  /* Group information callbacks */
  { 0, "endgrent",	authfile_endgrent },
  { 0, "getgrent",	authfile_getgrent },
  { 0, "getgrgid",	authfile_getgrgid },
  { 0, "getgrnam",	authfile_getgrnam },
  { 0, "getgroups",	authfile_getgroups },
  { 0, "gid2name",	authfile_gid2name },
  { 0, "name2gid",	authfile_name2gid },
  { 0, "setgrent",	authfile_setgrent },

  /* Miscellaneous callbacks */
  { 0, "auth",		authfile_auth },
  { 0, "check",		authfile_chkpass },

  { 0, NULL, NULL }
};

module auth_file_module = {
  /* Always NULL */
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "auth_file",

  /* Module configuration handler table */
  authfile_conftab,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  authfile_authtab,

  /* Module initialization function */
  NULL,

  /* Session initialization function */
  authfile_sess_init,

  /* Module version */
  MOD_AUTH_FILE_VERSION
};

