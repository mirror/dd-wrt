/*
 * ProFTPD: mod_auth_file - file-based authentication module that supports
 *                          restrictions on the file contents
 *
 * Copyright (c) 2002-2005 The ProFTPD Project team
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
 * $Id: mod_auth_file.c,v 1.1 2006/04/24 11:39:28 honor Exp $
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

typedef struct entry_rec {
  struct entry_rec *next, *prev;
  char *name;

} authfile_entry_t;

typedef struct file_rec {
  struct file_rec *af_next;

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

static unsigned char af_handle_pw = FALSE, af_handle_gr = FALSE;

/* List of server-specific AuthUserFiles */
static authfile_file_t *af_user_file_list = NULL;
static authfile_file_t *af_current_user_file = NULL;

/* List of server-specific AuthGroupFiles */
static authfile_file_t *af_group_file_list = NULL;
static authfile_file_t *af_current_group_file = NULL;

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

      if ((new_buf = realloc(*buf, *buflen)) == NULL)
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

  if (!grpbuf)
    grpbuf = malloc(i);
  else
    grpbuf = realloc(grpbuf, i);

  if (!grpbuf)
    return NULL;

  sstrncpy(grpbuf, buf, i);

  if ((cp = strrchr(grpbuf, '\n')))
    *cp = '\0';

  for (cp = grpbuf, i = 0; i < NGRPFIELDS && cp; i++) {
    grpfields[i] = cp;

    if ((cp = strchr(cp, ':')))
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

static unsigned char af_close_file(authfile_file_t *file) {
  if (file && file->af_file) {
    fclose(file->af_file);
    file->af_file = NULL;
  }

  return TRUE;
}

static unsigned char af_open_file(authfile_file_t *file) {
  if (file) {

    /* If already opened, rewind */
    if (file->af_file)
      rewind(file->af_file);

    else if ((file->af_file = fopen(file->af_path, "r")) == NULL)
      return FALSE;

    return TRUE;
  }

  return FALSE;
}

static unsigned char af_allow_grent(authfile_file_t *groupf,
    struct group *grp) {

  /* Check that the grent is within the ID restrictions (if present). */
  if (groupf->af_restricted_ids) {

    if (grp->gr_gid < groupf->af_min_id.gid) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping group '%s': "
        "GID (%u) below the minimum allowed (%u)", grp->gr_name,
        (unsigned int) grp->gr_gid, (unsigned int) groupf->af_min_id.gid);
      return FALSE;
    }

    if (grp->gr_gid > groupf->af_max_id.gid) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping group '%s': "
        "GID (%u) above the maximum allowed (%u)", grp->gr_name,
        (unsigned int) grp->gr_gid, (unsigned int) groupf->af_max_id.gid);
      return FALSE;
    }
  }

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  /* Check if the grent has an acceptable name. */
  if (groupf->af_restricted_names) {
    int res = regexec(groupf->af_name_regex, grp->gr_name, 0, NULL, 0);

    if ((res != 0 && !groupf->af_name_regex_inverted) ||
        (res == 0 && groupf->af_name_regex_inverted)) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping group '%s': "
        "name '%s' does not meet allowed filter '%s'", grp->gr_name,
        grp->gr_name, groupf->af_name_filter);
      return FALSE;
    }
  }
#endif /* !HAVE_REGEX_H and !HAVE_REGCOMP */

  return TRUE;
}

static void af_endgrent(void) {
  af_close_file(af_current_group_file);
  af_current_group_file = NULL;

  return;
}

static struct group *af_getgrent(authfile_file_t *groupf) {
  struct group *grp = NULL;

  while (TRUE) {
#ifdef HAVE_FGETGRENT
    pr_signals_handle();
    grp = fgetgrent(groupf->af_file);
#else
    char *cp = NULL, *buf = NULL;
    int buflen = BUFSIZ;

    pr_signals_handle();

    buf = malloc(BUFSIZ);
    if (!buf)
      return NULL;

    while (af_getgrentline(&buf, &buflen, groupf->af_file) != NULL) {

      /* Ignore comment and empty lines */
      if (buf[0] == '\0' || buf[0] == '#')
        continue;

      if ((cp = strchr(buf, '\n')) != NULL)
        *cp = '\0';

      grp = af_getgrp(buf);
      free(buf);

      break;
    }
#endif /* !HAVE_FGETGRENT */

    /* If grp is NULL now, the file is empty - nothing more to be read. */
    if (!grp)
      break;

    if (!af_allow_grent(groupf, grp))
      continue;

    break;
  }

  return grp;
}

static struct group *af_getgrnam(authfile_file_t *groupf, const char *name) {
  struct group *grp = NULL;

  while ((grp = af_getgrent(groupf)) != NULL)
    if (!strcmp(name, grp->gr_name))

      /* Found the requested group */
      break;

  return grp;
}

static struct group *af_getgrgid(authfile_file_t *groupf, gid_t gid) {
  struct group *grp = NULL;

  while ((grp = af_getgrent(groupf)) != NULL)
    if (grp->gr_gid == gid)

      /* Found the requested GID */
      break;

  return grp;
}

static unsigned char af_setgrent(void) {

  /* If not already present, start at the top of the list. */
  if (!af_current_group_file)
    af_current_group_file = af_group_file_list;

  while (af_current_group_file) {

    if (!af_open_file(af_current_group_file)) {
      /* Log the error */
      pr_log_pri(PR_LOG_ERR, "error: unable to open group file '%s': %s",
        af_current_group_file->af_path, strerror(errno));

      /* Move to the next file in the list. */
      af_current_group_file = af_current_group_file->af_next;
      continue;

    } else {
      pr_log_debug(DEBUG7, MOD_AUTH_FILE_VERSION ": using group file '%s'",
        af_current_group_file->af_path);
      return TRUE;
    }
  }

  return FALSE;
}

static unsigned char af_allow_pwent(authfile_file_t *passwdf,
    struct passwd *pwd) {

  /* Check that the pwent is within the ID restrictions (if present). */
  if (passwdf->af_restricted_ids) {

    if (pwd->pw_uid < passwdf->af_min_id.uid) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping user '%s': "
        "UID (%u) below the minimum allowed (%u)", pwd->pw_name,
        (unsigned int) pwd->pw_uid, (unsigned int) passwdf->af_min_id.uid);
      return FALSE;
    }

    if (pwd->pw_uid > passwdf->af_max_id.gid) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping user '%s': "
        "UID (%u) above the maximum allowed (%u)", pwd->pw_name,
        (unsigned int) pwd->pw_uid, (unsigned int) passwdf->af_max_id.uid);
      return FALSE;
    }
  }

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
  /* Check if the pwent has an acceptable name. */
  if (passwdf->af_restricted_names) {
    int res = regexec(passwdf->af_name_regex, pwd->pw_name, 0, NULL, 0);

    if ((res != 0 && !passwdf->af_name_regex_inverted) ||
        (res == 0 && passwdf->af_name_regex_inverted)) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping user '%s': "
        "name '%s' does not meet allowed filter '%s'", pwd->pw_name,
        pwd->pw_name, passwdf->af_name_filter);
      return FALSE;
    }
  }

  /* Check if the pwent has an acceptable home directory. */
  if (passwdf->af_restricted_homes) {

    int res = regexec(passwdf->af_home_regex, pwd->pw_dir, 0, NULL, 0);

    if ((res != 0 && !passwdf->af_home_regex_inverted) ||
        (res == 0 && passwdf->af_home_regex_inverted)) {
      pr_log_debug(DEBUG3, MOD_AUTH_FILE_VERSION ": skipping user '%s': "
        "home '%s' does not meet allowed filter '%s'", pwd->pw_name,
        pwd->pw_dir, passwdf->af_home_filter);
      return FALSE;
    }
  }
#endif /* !HAVE_REGEX_H and !HAVE_REGCOMP */

  return TRUE;
}

static void af_endpwent(void) {
  af_close_file(af_current_user_file);
  af_current_user_file = NULL;

  return;
}

static struct passwd *af_getpwent(authfile_file_t *passwdf) {
  struct passwd *pwd = NULL;

  while (TRUE) {
#ifdef HAVE_FGETPWENT
    pr_signals_handle();
    pwd = fgetpwent(passwdf->af_file);
#else
    char buf[BUFSIZ] = {'\0'};

    pr_signals_handle();
    while (fgets(buf, sizeof(buf), passwdf->af_file) != (char*) 0) {
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

    if (!af_allow_pwent(passwdf, pwd))
      continue;

    break;
  }

  return pwd;
}

static struct passwd *af_getpwnam(authfile_file_t *passwdf, const char *name) {
  struct passwd *pwd = NULL;

  while ((pwd = af_getpwent(passwdf)) != NULL)
    if (!strcmp(name, pwd->pw_name))

      /* Found the requested user */
      break;

  return pwd;
}

static char *af_getpwpass(authfile_file_t *passwdf, const char *name) {
  struct passwd *pwd = af_getpwnam(passwdf, name);

  return pwd ? pwd->pw_passwd : NULL;
}

static struct passwd *af_getpwuid(authfile_file_t *passwdf, uid_t uid) {
  struct passwd *pwd = NULL;

  while ((pwd = af_getpwent(passwdf)) != NULL)
    if (pwd->pw_uid == uid)

      /* Found the requested UID */
      break;

  return pwd;
}

static unsigned char af_setpwent(void) {

  /* If not already present, start at the top of the list. */
  if (!af_current_user_file)
    af_current_user_file = af_user_file_list;

  while (af_current_user_file) {

    if (!af_open_file(af_current_user_file)) {
      /* Log the error */
      pr_log_pri(PR_LOG_ERR, "error: unable to open passwd file '%s': %s",
        af_current_user_file->af_path, strerror(errno));

      /* Move to the next file in the list. */
      af_current_user_file = af_current_user_file->af_next;
      continue;

    } else {
      pr_log_debug(DEBUG7, MOD_AUTH_FILE_VERSION ": using passwd file '%s'",
        af_current_user_file->af_path);
      return TRUE;
    }
  }

  return FALSE;
}

/* Authentication handlers.
 */

MODRET authfile_endpwent(cmd_rec *cmd) {

  /* Do not handle *pw* requests unless we can do so. */
  if (!af_handle_pw)
    return DECLINED(cmd);

  af_endpwent();

  return DECLINED(cmd);
}

MODRET authfile_getpwent(cmd_rec *cmd) {
  struct passwd *pwd = NULL;

  /* Do not handle *pw* requests unless we can do so. */
  if (!af_handle_pw)
    return DECLINED(cmd);

  if (!af_setpwent())
    return DECLINED(cmd);

  pwd = af_getpwent(af_current_user_file);

  return pwd ? mod_create_data(cmd, pwd) : DECLINED(cmd);
}

MODRET authfile_getpwnam(cmd_rec *cmd) {
  struct passwd *pwd = NULL;
  const char *name = cmd->argv[0];

  /* Do not handle *pw* requests unless we can do so. */
  if (!af_handle_pw)
    return DECLINED(cmd);

  if (!af_setpwent())
    return DECLINED(cmd);

  /* Ugly -- we iterate through the file.  Time-consuming. */
  while ((pwd = af_getpwent(af_current_user_file)) != NULL)
    if (!strcmp(name, pwd->pw_name))

      /* Found the requested name */
      break;

  return pwd ? mod_create_data(cmd, pwd) : DECLINED(cmd);
}

MODRET authfile_getpwuid(cmd_rec *cmd) {
  struct passwd *pwd = NULL;
  uid_t uid = *((uid_t *) cmd->argv[0]);

  /* Do not handle *pw* requests unless we can do so. */
  if (!af_handle_pw)
    return DECLINED(cmd);

  if (!af_setpwent())
    return DECLINED(cmd);

  pwd = af_getpwuid(af_current_user_file, uid);

  return pwd ? mod_create_data(cmd, pwd) : DECLINED(cmd);
}

MODRET authfile_name2uid(cmd_rec *cmd) {
  struct passwd *pwd = NULL;

  /* Do not handle *pw* requests unless we can do so. */
  if (!af_handle_pw)
    return DECLINED(cmd);

  if (!af_setpwent())
    return DECLINED(cmd);

  pwd = af_getpwnam(af_current_user_file, cmd->argv[0]);

  return pwd ? mod_create_data(cmd, (void *) &pwd->pw_uid) : DECLINED(cmd);
}

MODRET authfile_setpwent(cmd_rec *cmd) {

  /* Do not handle *pw* requests unless we can do so. */
  if (!af_handle_pw)
    return DECLINED(cmd);

  if (af_setpwent())
    return DECLINED(cmd);

  pr_log_debug(DEBUG2,
    MOD_AUTH_FILE_VERSION ": unable to find useable AuthUserFile");

  return DECLINED(cmd);
}

MODRET authfile_uid2name(cmd_rec *cmd) {
  struct passwd *pwd = NULL;

  /* Do not handle *pw* requests unless we can do so. */
  if (!af_handle_pw)
    return DECLINED(cmd);

  if (!af_setpwent())
    return DECLINED(cmd);

  pwd = af_getpwuid(af_current_user_file, *((uid_t *) cmd->argv[0]));

  return pwd ? mod_create_data(cmd, pwd->pw_name) : DECLINED(cmd);
}

MODRET authfile_endgrent(cmd_rec *cmd) {

  /* Do not handle *gr* requests unless we can do so. */
  if (!af_handle_gr)
    return DECLINED(cmd);

  af_endgrent();

  return DECLINED(cmd);
}

MODRET authfile_getgrent(cmd_rec *cmd) {
  struct group *grp = NULL;

  /* Do not handle *gr* requests unless we can do so. */
  if (!af_handle_gr)
    return DECLINED(cmd);

  if (!af_setgrent())
    return DECLINED(cmd);

  grp = af_getgrent(af_current_group_file);

  return grp ? mod_create_data(cmd, grp) : DECLINED(cmd);
}

MODRET authfile_getgrgid(cmd_rec *cmd) {
  struct group *grp = NULL;
  gid_t gid = *((gid_t *) cmd->argv[0]);

  /* Do not handle *gr* requests unless we can do so. */
  if (!af_handle_gr)
    return DECLINED(cmd);

  if (!af_setgrent())
    return DECLINED(cmd);

  grp = af_getgrgid(af_current_group_file, gid);

  return grp ? mod_create_data(cmd, grp) : DECLINED(cmd);
}

MODRET authfile_getgrnam(cmd_rec *cmd) {
  struct group *grp = NULL;
  const char *name = cmd->argv[0];

  /* Do not handle *gr* requests unless we can do so. */
  if (!af_handle_gr)
    return DECLINED(cmd);

  if (!af_setgrent())
    return DECLINED(cmd);

  while ((grp = af_getgrent(af_current_group_file)) != NULL)
    if (!strcmp(name, grp->gr_name))

      /* Found the name requested */
      break;

  return grp ? mod_create_data(cmd, grp) : DECLINED(cmd);
}

MODRET authfile_getgroups(cmd_rec *cmd) {
  struct passwd *pwd = NULL;
  struct group *grp = NULL;
  array_header *gids = NULL, *groups = NULL;
  char *name = cmd->argv[0];

  /* Do not handle *gr* requests unless we can do so. */
  if (!af_handle_gr)
    return DECLINED(cmd);

  if (!af_setpwent())
    return DECLINED(cmd);

  if (!af_setgrent())
    return DECLINED(cmd);

  /* Check for NULLs */
  if (cmd->argv[1])
    gids = (array_header *) cmd->argv[1];

  if (cmd->argv[2])
    groups = (array_header *) cmd->argv[2];

  /* Retrieve the necessary info. */
  if (!name || !(pwd = af_getpwnam(af_current_user_file, name)))
    return mod_create_error(cmd, -1);

  /* Populate the first group ID and name. */
  if (gids)
    *((gid_t *) push_array(gids)) = pwd->pw_gid;

  if (groups &&
      (grp = af_getgrgid(af_current_group_file, pwd->pw_gid)) != NULL)
    *((char **) push_array(groups)) = pstrdup(session.pool, grp->gr_name);

  /* The above call to af_getgrgid() will position the file pointer in
   * the AuthGroupFile just after the group with the primary GID.
   * Subsequently, the below af_getgrent() starts from that position, and
   * goes to the end of the file.  The problem is that there may be groups
   * before the primary GID for the current group.  So, ideally, the
   * getgrent() loop would continue until we're back to where we are now,
   * rather than stopping at the end of the file.  Conversely, we could
   * just simply rewind to the start of the AuthGroupFile (which is easier).
   * The core auth code will remove duplicate IDs as needed.
   */
  af_open_file(af_current_group_file);

  /* This is where things get slow, expensive, and ugly.  Loop through
   * everything, checking to make sure we haven't already added it.
   */
  while ((grp = af_getgrent(af_current_group_file)) != NULL &&
      grp->gr_mem) {
    char **gr_mems = NULL;

    /* Loop through each member name listed */
    for (gr_mems = grp->gr_mem; *gr_mems; gr_mems++) {

      /* If it matches the given username... */
      if (!strcmp(*gr_mems, pwd->pw_name)) {

        /* ...add the GID and name */
        if (gids)
          *((gid_t *) push_array(gids)) = grp->gr_gid;

        if (groups)
          *((char **) push_array(groups)) = pstrdup(session.pool, grp->gr_name);
      }
    }
  }

  if (gids && gids->nelts > 0)
    return mod_create_data(cmd, (void *) &gids->nelts);

  else if (groups && groups->nelts > 0)
    return mod_create_data(cmd, (void *) &groups->nelts);

  return DECLINED(cmd);
}

MODRET authfile_gid2name(cmd_rec *cmd) {
  struct group *grp = NULL;

  /* Do not handle *gr* requests unless we can do so. */
  if (!af_handle_gr)
    return DECLINED(cmd);

  if (!af_setgrent())
    return DECLINED(cmd);

  grp = af_getgrgid(af_current_group_file, *((gid_t *) cmd->argv[0]));

  return grp ? mod_create_data(cmd, grp->gr_name) : DECLINED(cmd);
}

MODRET authfile_name2gid(cmd_rec *cmd) {
  struct group *grp = NULL;

  /* Do not handle *gr* requests unless we can do so. */
  if (!af_handle_gr)
    return DECLINED(cmd);

  if (!af_setgrent())
    return DECLINED(cmd);

  grp = af_getgrnam(af_current_group_file, cmd->argv[0]);

  return grp ? mod_create_data(cmd, (void *) &grp->gr_gid) : DECLINED(cmd);
}

MODRET authfile_setgrent(cmd_rec *cmd) {

  /* Do not handle *gr* requests unless we can do so. */
  if (!af_handle_gr)
    return DECLINED(cmd);

  if (af_setgrent())
    return DECLINED(cmd);

  pr_log_debug(DEBUG2,
    MOD_AUTH_FILE_VERSION ": unable to find useable AuthGroupFile");

  return DECLINED(cmd);
}

MODRET authfile_auth(cmd_rec *cmd) {
  time_t now = time(NULL), lstchg = -1, max = -1, inact = -1, disable = -1;
  char *tmp = NULL, *cleartxt_pass = NULL;
  const char *name = cmd->argv[0];

  /* Do not handle *pw* requests unless we can do so. */
  if (!af_handle_pw)
    return DECLINED(cmd);

  if (!af_setpwent())
    return DECLINED(cmd);

  /* Lookup the cleartxt password for this user. */
  if ((tmp = af_getpwpass(af_current_user_file, name)) == NULL) {

    /* For now, return DECLINED.  Ideally, we could stash an auth module
     * identifier in the session structure, so that all auth modules could
     * coordinate/use their methods as long as they matched the auth module
     * used.
     */
    return DECLINED(cmd);

#if 0
    /* When the above is implemented, and if the user being checked was
     * provided by mod_auth_file, we'd return this.
     */
    return ERROR_INT(cmd, PR_AUTH_NOPWD);
#endif
  }

  cleartxt_pass = pstrdup(cmd->tmp_pool, tmp);

  if (pr_auth_check(cmd->tmp_pool, cleartxt_pass, name, cmd->argv[1]))
    return ERROR_INT(cmd, PR_AUTH_BADPWD);

  if (lstchg > 0 && max > 0 && inact > 0 && now > (lstchg + max + inact))
    return ERROR_INT(cmd, PR_AUTH_AGEPWD);

  if (disable > 0 && now > disable)
    return ERROR_INT(cmd, PR_AUTH_DISABLEDPWD);

  session.auth_mech = "mod_auth_file.c";
  return HANDLED(cmd);
}

MODRET authfile_chkpass(cmd_rec *cmd) {
  const char *ciphertxt_pass = cmd->argv[0];
  const char *cleartxt_pass = cmd->argv[2];

  /* Do not handle *pw* requests unless we can do so. */
  if (!af_handle_pw)
    return DECLINED(cmd);

  if (strcmp(crypt(cleartxt_pass, ciphertxt_pass), ciphertxt_pass) == 0) {
    session.auth_mech = "mod_auth_file.c";
    return HANDLED(cmd);
  }  

  return DECLINED(cmd);
}

/* Configuration handlers
 */

/* NOTE: support multiple AuthUserFiles, AuthGroupFiles.  Have optional
 * parameter to restrict ID range in files, min and max, where max >= min.
 *
 * Future rev: incorporate AuthShadowFile into this, and add --shadow
 * capabilities to ftpasswd.
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
      if (!strcmp(cmd->argv[i], "id")) {
        gid_t min, max;
        char *sep = NULL, *tmp = NULL;

        /* The range restriction parameter is of the form "min-max", where max
         * must be >= min.
         */

        if ((sep = strchr(cmd->argv[++i], '-')) == NULL)
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
      } else if (!strcmp(cmd->argv[i], "name")) {
        char *filter = cmd->argv[++i];
        regex_t *preg = NULL;
        int res = 0;

        preg = pr_regexp_alloc();

        /* Check for a ! negation/inversion filter prefix. */
        if (*filter == '!') {
          filter++;
          file->af_name_regex_inverted = TRUE;
        }

        if ((res = regcomp(preg, filter, REG_EXTENDED|REG_NOSUB)) != 0) {
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

  return HANDLED(cmd);
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
      if (!strcmp(cmd->argv[i], "id")) {
        uid_t min, max;
        char *sep = NULL, *tmp = NULL;

        /* The range restriction parameter is of the form "min-max", where max
         * must be >= min.
         */

        if ((sep = strchr(cmd->argv[++i], '-')) == NULL)
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
      } else if (!strcmp(cmd->argv[i], "home")) {
        char *filter = cmd->argv[++i];
        regex_t *preg = NULL;
        int res = 0;

        preg = pr_regexp_alloc();

        /* Check for a ! negation/inversion filter prefix. */
        if (*filter == '!') {
          filter++;
          file->af_home_regex_inverted = TRUE;
        }

        if ((res = regcomp(preg, filter, REG_EXTENDED|REG_NOSUB)) != 0) {
          char errstr[200] = {'\0'};

          regerror(res, preg, errstr, sizeof(errstr));
          pr_regexp_free(preg);

          CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "'", filter, "' failed "
            "regex compilation: ", errstr, NULL));
        }

        file->af_home_filter = pstrdup(c->pool, cmd->argv[i]);
        file->af_home_regex = preg;
        file->af_restricted_homes = TRUE;

      } else if (!strcmp(cmd->argv[i], "name")) {
        char *filter = cmd->argv[++i];
        regex_t *preg = NULL;
        int res = 0;

        preg = pr_regexp_alloc();

        /* Check for a ! negation/inversion filter prefix. */
        if (*filter == '!') {
          filter++;
          file->af_name_regex_inverted = TRUE;
        }

        if ((res = regcomp(preg, filter, REG_EXTENDED|REG_NOSUB)) != 0) {
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

  return HANDLED(cmd);
}

/* Initialization routines
 */

static int authfile_sess_init(void) {
  config_rec *c = NULL;

  af_user_file_list = af_group_file_list = NULL;

  /* Search for all relevant AuthUserFiles for this server. */
  c = find_config(main_server->conf, CONF_PARAM, "AuthUserFile", FALSE);

  while (c) {
    authfile_file_t *file = c->argv[0];

/* NOTE: This is a hack, to prevent these config_recs from being handled by
 * mod_unixpw.  Only necessary until mod_unixpw is transformed into
 * mod_auth_unix.
 */
c->name = "";

    if (!af_user_file_list) {
      file->af_next = af_user_file_list;
      af_user_file_list = file;
    }

    c = find_config_next(c, c->next, CONF_PARAM, "AuthUserFile", FALSE);
  }

  /* Search for all relevant AuthGroupFiles for this server. */
  c = find_config(main_server->conf, CONF_PARAM, "AuthGroupFile", FALSE);

  while (c) {
    authfile_file_t *file = c->argv[0];

/* NOTE: This is a hack, to prevent these config_recs from being handled by
 * mod_unixpw.  Only necessary until mod_unixpw is transformed into
 * mod_auth_unix.
 */
c->name = "";

    if (!af_group_file_list) {
      file->af_next = af_group_file_list;
      af_group_file_list = file;
    }

    c = find_config_next(c, c->next, CONF_PARAM, "AuthGroupFile", FALSE);
  }

  if (af_user_file_list)
    af_handle_pw = TRUE;

  if (af_group_file_list)
    af_handle_gr = TRUE;

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
  authfile_sess_init
};

