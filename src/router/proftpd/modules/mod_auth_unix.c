/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2006 The ProFTPD Project team
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

/* Unix authentication module for ProFTPD
 * $Id: mod_auth_unix.c,v 1.31 2006/06/29 17:16:23 castaglia Exp $
 */

#include "conf.h"

/* AIX has some rather stupid function prototype inconsistencies between
 * their crypt.h and stdlib.h's setkey() declarations.  *sigh*
 */
#if defined(HAVE_CRYPT_H) && !defined(AIX4) && !defined(AIX5)
# include <crypt.h>
#endif

#ifdef PR_USE_SHADOW
# include <shadow.h>
#endif

#ifdef HAVE_SYS_SECURITY_H
# include <sys/security.h>
#endif

#ifdef HAVE_KRB_H
# include <krb.h>
#endif

#if defined(HAVE_HPSECURITY_H) || defined(HPUX10) || defined(HPUX11)
# include <hpsecurity.h>
# ifndef COMSEC
#  define COMSEC 1
# endif /* !COMSEC */
#endif /* HAVE_HPSECURITY_H or HPUX10 or HPUX11 */

#if defined(HAVE_PROT_H) || defined(COMSEC)
# include <prot.h>
#endif

#ifdef PR_USE_SIA
# ifdef HAVE_SIA_H
#  include <sia.h>
# endif
# ifdef HAVE_SIAD_H
#  include <siad.h>
# endif
#endif /* PR_USE_SIA */

#ifdef CYGWIN
typedef void *HANDLE;
typedef unsigned long DWORD;
# define INVALID_HANDLE_VALUE (HANDLE)(-1)
# define WINAPI __stdcall
DWORD WINAPI GetVersion(void);
extern HANDLE cygwin_logon_user (const struct passwd *, const char *);
extern void cygwin_set_impersonation_token (const HANDLE);
#endif /* CYGWIN */

#ifdef SETGRENT_VOID
# define RETSETGRENTTYPE	void
#else
# define RETSETGRENTTYPE	int
#endif

#include "privs.h"

static const char *pwdfname = "/etc/passwd";
static const char *grpfname = "/etc/group";

#ifdef HAVE__PW_STAYOPEN
extern int _pw_stayopen;
#endif

#define HASH_TABLE_SIZE		100

typedef union idauth {
  uid_t uid;
  gid_t gid;
} idauth_t;

typedef struct _idmap {
  struct _idmap *next,*prev;

  /* This is a union because different OSs may give different types to UIDs
   * and GIDs.  This presents a far more portable way to deal with this
   * reality.
   */
  idauth_t id;

  char *name;			/* user or group name */
} idmap_t;

module auth_unix_module;

static xaset_t *uid_table[HASH_TABLE_SIZE];
static xaset_t *gid_table[HASH_TABLE_SIZE];

static FILE *pwdf = NULL;
static FILE *grpf = NULL;

extern unsigned char persistent_passwd;

#undef PASSWD
#define PASSWD		pwdfname
#undef GROUP
#define	GROUP		grpfname

#ifdef PR_USE_SHADOW

/* Shadow password entries are stored as number of days, not seconds
 * and are -1 if unused
 */
#define SP_CVT_DAYS(x)	((x) == (time_t)-1 ? (x) : ((x) * 86400))

#endif /* PR_USE_SHADOW */

static void p_setpwent(void) {
  if (pwdf)
    rewind(pwdf);

  else
    if ((pwdf = fopen(PASSWD,"r")) == NULL)
      pr_log_pri(PR_LOG_ERR, "Unable to open password file %s for reading: %s",
        PASSWD, strerror(errno));
}

static void p_endpwent(void) {
  if (pwdf) {
    fclose(pwdf);
    pwdf = NULL;
  }
}

static RETSETGRENTTYPE p_setgrent(void) {
  if (grpf)
    rewind(grpf);

  else
    if ((grpf = fopen(GROUP,"r")) == NULL)
      pr_log_pri(PR_LOG_ERR, "Unable to open group file %s for reading: %s",
        GROUP, strerror(errno));

#ifndef SETGRENT_VOID
  return 0;
#endif
}

static void p_endgrent(void) {
  if (grpf) {
    fclose(grpf);
    grpf = NULL;
  }
}

static struct passwd *p_getpwent(void) {
  if (!pwdf)
    p_setpwent();

  if (!pwdf)
    return NULL;

  return fgetpwent(pwdf);
}

static struct group *p_getgrent(void) {
  struct group *gr = NULL;

  if (!grpf)
    p_setgrent();

  if (!grpf)
    return NULL;

  gr = fgetgrent(grpf);

  return gr;
}

static struct passwd *p_getpwnam(const char *name) {
  struct passwd *pw = NULL;

  p_setpwent();
  while ((pw = p_getpwent()) != NULL)
    if (!strcmp(name,pw->pw_name))
      break;

  return pw;
}

static struct passwd *p_getpwuid(uid_t uid) {
  struct passwd *pw = NULL;

  p_setpwent();
  while ((pw = p_getpwent()) != NULL)
    if (pw->pw_uid == uid)
      break;

  return pw;
}

static struct group *p_getgrnam(const char *name) {
  struct group *gr = NULL;

  p_setgrent();
  while ((gr = p_getgrent()) != NULL)
    if (!strcmp(name,gr->gr_name))
      break;

  return gr;
}

static struct group *p_getgrgid(gid_t gid) {
  struct group *gr = NULL;

  p_setgrent();
  while ((gr = p_getgrent()) != NULL)
    if (gr->gr_gid == gid)
      break;

  return gr;
}

inline static int _compare_uid(idmap_t *m1, idmap_t *m2) {
  if (m1->id.uid < m2->id.uid)
    return -1;

  if (m1->id.uid > m2->id.uid)
    return 1;

  return 0;
}

inline static int _compare_gid(idmap_t *m1, idmap_t *m2) {
  if (m1->id.gid < m2->id.gid)
    return -1;

  if (m1->id.gid > m2->id.gid)
    return 1;

  return 0;
}

inline static int _compare_id(xaset_t **table, idauth_t id, idauth_t idcomp) {
  if (table == uid_table)
    return id.uid == idcomp.uid;
  else
    return id.gid == idcomp.gid;
}

static idmap_t *_auth_lookup_id(xaset_t **id_table, idauth_t id) {
  int hash = ((id_table == uid_table) ? id.uid : id.gid) % HASH_TABLE_SIZE;
  idmap_t *m;

  if (!id_table[hash])
    id_table[hash] = xaset_create(session.pool,
      (id_table == uid_table) ? (XASET_COMPARE)_compare_uid :
      (XASET_COMPARE)_compare_gid);

  for (m = (idmap_t *) id_table[hash]->xas_list; m; m = m->next) {
    if (_compare_id(id_table, m->id, id))
      break;
  }

  if (!m || !_compare_id(id_table, m->id, id)) {
    /* Isn't in the table */
    m = (idmap_t *) pcalloc(id_table[hash]->pool, sizeof(idmap_t));

    if (id_table == uid_table)
      m->id.uid = id.uid;
    else
      m->id.gid = id.gid;

    xaset_insert_sort(id_table[hash], (xasetmember_t *) m, FALSE);
  }

  return m;
}

MODRET pw_setpwent(cmd_rec *cmd) {
  if (persistent_passwd)
    p_setpwent();

  else
    setpwent();

  return PR_DECLINED(cmd);
}

MODRET pw_endpwent(cmd_rec *cmd) {
  if (persistent_passwd)
    p_endpwent();

  else
    endpwent();

  return PR_DECLINED(cmd);
}

MODRET pw_setgrent(cmd_rec *cmd) {
  if (persistent_passwd)
    p_setgrent();

  else
    setgrent();

  return PR_DECLINED(cmd);
}

MODRET pw_endgrent(cmd_rec *cmd) {
  if (persistent_passwd)
    p_endgrent();

  else
    endgrent();

  return PR_DECLINED(cmd);
}

MODRET pw_getgrent(cmd_rec *cmd) {
  struct group *gr;

  if (persistent_passwd)
    gr = p_getgrent();

  else
    gr = getgrent();

  return gr ? mod_create_data(cmd, gr) : PR_DECLINED(cmd);
}

MODRET pw_getpwent(cmd_rec *cmd) {
  struct passwd *pw;

  if (persistent_passwd)
    pw = p_getpwent();

  else
    pw = getpwent();

  return pw ? mod_create_data(cmd, pw) : PR_DECLINED(cmd);
}

MODRET pw_getpwuid(cmd_rec *cmd) {
  struct passwd *pw;
  uid_t uid;

  uid = *((uid_t *) cmd->argv[0]);
  if (persistent_passwd)
    pw = p_getpwuid(uid);

  else
    pw = getpwuid(uid);

  return pw ? mod_create_data(cmd, pw) : PR_DECLINED(cmd);
}

MODRET pw_getpwnam(cmd_rec *cmd) {
  struct passwd *pw;
  const char *name;

  name = cmd->argv[0];
  if (persistent_passwd)
    pw = p_getpwnam(name);

  else
    pw = getpwnam(name);

  return pw ? mod_create_data(cmd, pw) : PR_DECLINED(cmd);
}

MODRET pw_getgrnam(cmd_rec *cmd) {
  struct group *gr;
  const char *name;

  name = cmd->argv[0];
  if (persistent_passwd)
    gr = p_getgrnam(name);

  else
    gr = getgrnam(name);

  return gr ? mod_create_data(cmd, gr) : PR_DECLINED(cmd);
}

MODRET pw_getgrgid(cmd_rec *cmd) {
  struct group *gr;
  gid_t gid;

  gid = *((gid_t *) cmd->argv[0]);
  if (persistent_passwd)
    gr = p_getgrgid(gid);

  else
    gr = getgrgid(gid);

  return gr ? mod_create_data(cmd, gr) : PR_DECLINED(cmd);
}

#ifdef PR_USE_SHADOW
static char *_get_pw_info(pool *p, const char *u, time_t *lstchg, time_t *min,
    time_t *max, time_t *warn, time_t *inact, time_t *expire) {
  struct spwd *sp;
  char *cpw = NULL;

  PRIVS_ROOT
#ifdef HAVE_SETSPENT
  setspent();
#endif /* HAVE_SETSPENT */

  sp = getspnam(u);

  if (sp) {
    cpw = pstrdup(p, sp->sp_pwdp);

    if (lstchg)
      *lstchg = SP_CVT_DAYS(sp->sp_lstchg);

    if (min)
      *min = SP_CVT_DAYS(sp->sp_min);

    if (max)
      *max = SP_CVT_DAYS(sp->sp_max);

#ifdef HAVE_SPWD_SP_WARN
    if (warn)
      *warn = SP_CVT_DAYS(sp->sp_warn);
#endif /* HAVE_SPWD_SP_WARN */

#ifdef HAVE_SPWD_SP_INACT
    if (inact)
      *inact = SP_CVT_DAYS(sp->sp_inact);
#endif /* HAVE_SPWD_SP_INACT */

#ifdef HAVE_SPWD_SP_EXPIRE
    if (expire)
      *expire = SP_CVT_DAYS(sp->sp_expire);
#endif /* HAVE_SPWD_SP_EXPIRE */
  }
#ifdef PR_USE_AUTO_SHADOW
  else {
    struct passwd *pw;

    endspent();
    PRIVS_RELINQUISH

    if ((pw = getpwnam(u)) != NULL) {
      cpw = pstrdup(p, pw->pw_passwd);

      if (lstchg)
        *lstchg = (time_t) -1;

      if (min)
        *min = (time_t) -1;

      if (max)
        *max = (time_t) -1;

      if (warn)
        *warn = (time_t) -1;

      if (inact)
        *inact = (time_t) -1;

      if (expire)
        *expire = (time_t) -1;
    }
  }
#else
  endspent();
  PRIVS_RELINQUISH
#endif /* PR_USE_AUTO_SHADOW */
  return cpw;
}

#else /* PR_USE_SHADOW */

static char *_get_pw_info(pool *p, const char *u, time_t *lstchg, time_t *min,
    time_t *max, time_t *warn, time_t *inact, time_t *expire) {
  char *cpw = NULL;
#if defined(HAVE_GETPRPWENT) || defined(COMSEC)
  struct pr_passwd *prpw;
#endif
#if !defined(HAVE_GETPRPWENT) || defined(COMSEC)
  struct passwd *pw;
#endif

 /* Some platforms (i.e. BSD) provide "transparent" shadowing, which
  * requires that we are root in order to have the password member
  * filled in.
  */

  PRIVS_ROOT
#if !defined(HAVE_GETPRPWENT) || defined(COMSEC)
# ifdef COMSEC
  if (!iscomsec()) {
# endif /* COMSEC */
  endpwent();
#if defined(BSDI3) || defined(BSDI4)
  /* endpwent() seems to be buggy on BSDI3.1 (is this true for 4.0?)
   * setpassent(0) _seems_ to do the same thing, however this conflicts
   * with the man page documented behavior.  Argh, why do all the bsds
   * have to be different in this area (except OpenBSD, grin).
   */
  setpassent(0);
#else /* BSDI3 || BSDI4 */
  setpwent();
#endif /* BSDI3 || BSDI4 */

  pw = getpwnam(u);

  if (pw) {
    cpw = pstrdup(p, pw->pw_passwd);

    if (lstchg)
      *lstchg = (time_t) -1;

    if (min)
      *min = (time_t) -1;

    if (max)
      *max = (time_t) -1;

    if (warn)
      *warn = (time_t) -1;

    if (inact)
      *inact = (time_t) -1;

    if (expire)
      *expire = (time_t) -1;
  }

  endpwent();
#ifdef COMSEC
  } else {
#endif /* COMSEC */
#endif /* !HAVE_GETPRWENT or COMSEC */

#if defined(HAVE_GETPRPWENT) || defined(COMSEC)
  endprpwent();
  setprpwent();

  prpw = getprpwnam((char *) u);

  if (prpw) {
    cpw = pstrdup(p, prpw->ufld.fd_encrypt);

    if (lstchg)
      *lstchg = (time_t) -1;

    if (min)
      *min = prpw->ufld.fd_min;

    if (max)
      *max = (time_t) -1;

    if (warn)
      *warn = (time_t) -1;

    if (inact)
      *inact = (time_t) -1;

    if (expire)
      *expire = prpw->ufld.fd_expire;
  }

  endprpwent();
#ifdef COMSEC
  }
#endif /* COMSEC */
#endif /* HAVE_GETPRPWENT or COMSEC */

  PRIVS_RELINQUISH
#if defined(BSDI3) || defined(BSDI4)
  setpassent(1);
#endif
  return cpw;
}

#endif /* PR_USE_SHADOW */

/* High-level auth handlers
 */

/* cmd->argv[0] : user name
 * cmd->argv[1] : cleartext password
 */

MODRET pw_auth(cmd_rec *cmd) {
  time_t now;
  char *cpw;
  time_t lstchg = -1, max = -1, inact = -1, disable = -1;
  const char *name;

  name = cmd->argv[0];
  time(&now);

  cpw = _get_pw_info(cmd->tmp_pool, name, &lstchg, NULL, &max, NULL, &inact,
    &disable);

  if (!cpw)
    return PR_DECLINED(cmd);

  if (pr_auth_check(cmd->tmp_pool, cpw, cmd->argv[0], cmd->argv[1]))
    return PR_ERROR_INT(cmd, PR_AUTH_BADPWD);

  if (lstchg > (time_t) 0 &&
      max > (time_t) 0 &&
      inact > (time_t)0)
    if (now > lstchg + max + inact)
      return PR_ERROR_INT(cmd, PR_AUTH_AGEPWD);

  if (disable > (time_t) 0 &&
      now > disable)
    return PR_ERROR_INT(cmd, PR_AUTH_DISABLEDPWD);

  session.auth_mech = "mod_auth_unix.c";
  return PR_HANDLED(cmd);
}

/* cmd->argv[0] = hashed password,
 * cmd->argv[1] = user,
 * cmd->argv[2] = cleartext
 */

MODRET pw_check(cmd_rec *cmd) {
  const char *cpw = cmd->argv[0];
  const char *pw = cmd->argv[2];

#ifdef PR_USE_SIA
  SIAENTITY *ent = NULL;
  int res = SIASUCCESS;
  char *info[2];
  struct passwd *pwd;
  char *user = NULL;
#endif

#ifdef COMSEC
  if (iscomsec()) {
    if (strcmp(bigcrypt((char *) pw, (char *) cpw), cpw) != 0) {
      return PR_DECLINED(cmd);
    }

  } else {
#endif /* COMSEC */

#ifdef PR_USE_SIA
  /* Use Tru64's C2 SIA subsystem for authenticating this user. */
  user = cmd->argv[1];

  pr_log_auth(PR_LOG_NOTICE, "using SIA for user '%s'", user);

  info[0] = "ProFTPD";
  info[1] = NULL;

  /* Prepare the SIA subsystem. */
  PRIVS_ROOT
  res = sia_ses_init(&ent, 1, info, NULL, user, NULL, 0, NULL);
  if (res != SIASUCCESS) {
    pr_log_auth(PR_LOG_NOTICE, "sia_ses_init() returned %d for user '%s'", res,
      user);

  } else {

    res = sia_ses_authent(NULL, pw, ent);
    if (res != SIASUCCESS) {
      sia_ses_release(&ent);
      PRIVS_RELINQUISH
      pr_log_auth(PR_LOG_NOTICE, "sia_ses_authent() returned %d for user '%s'",
        res, user);
      return ERROR(cmd);
    }

    res = sia_ses_estab(NULL, ent);
    if (res != SIASUCCESS) {
      PRIVS_RELINQUISH
      pr_log_auth(PR_LOG_NOTICE, "sia_ses_estab() returned %d for user '%s'",
        res, user);
      return ERROR(cmd);
    }

    res = sia_ses_release(&ent);
    if (res != SIASUCCESS) {
      PRIVS_RELINQUISH
      pr_log_auth(PR_LOG_NOTICE, "sia_ses_release() returned %d", res);
      return ERROR(cmd);
    }
  }
  PRIVS_RELINQUISH

  if (res != SIASUCCESS) {
    return PR_DECLINED(cmd);
  }

#else /* !PR_USE_SIA */

# ifdef CYGWIN
  /* We have to do special Windows NT voodoo with Cygwin in order to be
   * able to switch UID/GID. More info at
   * http://cygwin.com/cygwin-ug-net/ntsec.html#NTSEC-SETUID
   */
  if (GetVersion() < 0x80000000) {
    cmd_rec *tmp_cmd = NULL;
    modret_t *mr = NULL;
    struct passwd *pwent = NULL;
    HANDLE token;

    /* A struct passwd * is needed.  To look one up via pw_getpwnam(), though,
     * we'll need a cmd_rec.
     */
    tmp_cmd = pr_cmd_alloc(cmd->tmp_pool, 1, cmd->argv[1]);

    /* pw_getpwnam() returns a MODRET, so we need to handle that.  Yes, this
     * might have been easier if we'd used pr_auth_getpwnam(), but that would
     * dispatch through other auth modules, which is _not_ what we want.
     */
    mr = pw_getpwnam(tmp_cmd);

    /* Note: we don't handle the case where pw_getpwnam() returns anything
     * other than HANDLED at the moment.
     */

    if (MODRET_ISHANDLED(mr) &&
        MODRET_HASDATA(mr)) {
      pwent = mr->data;

      token = cygwin_logon_user((const struct passwd *) pwent, pw);
      if (token == INVALID_HANDLE_VALUE) {
        pr_log_pri(PR_LOG_NOTICE, "error authenticating Cygwin user: %s",
          strerror(errno));
        return PR_DECLINED(cmd);
      }

      cygwin_set_impersonation_token(token);

    } else {
      return PR_DECLINED(cmd);
    }

  } else
# endif /* CYGWIN */

  if (strcmp(crypt(pw, cpw), cpw) != 0) {
    return PR_DECLINED(cmd);
  }
#endif /* PR_USE_SIA */

#ifdef COMSEC
  }
#endif /* COMSEC */

  session.auth_mech = "mod_auth_unix.c";
  return PR_HANDLED(cmd);
}

MODRET pw_uid2name(cmd_rec *cmd) {
  idmap_t *m;
  idauth_t id;
  struct passwd *pw;

  id.uid = *((uid_t *) cmd->argv[0]);
  m = _auth_lookup_id(uid_table, id);

  if (!m->name) {
    /* Wasn't cached, so perform a lookup */

    if (persistent_passwd)
      pw = p_getpwuid(id.uid);

    else
      pw = getpwuid(id.uid);

    if (pw) {
      m->name = pstrdup(session.pool ? session.pool : permanent_pool,
        pw->pw_name);
      return mod_create_data(cmd, m->name);
    }

    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, m->name);
}

MODRET pw_gid2name(cmd_rec *cmd) {
  idmap_t *m;
  idauth_t id;
  struct group *gr;

  id.gid = *((gid_t *) cmd->argv[0]);

  m = _auth_lookup_id(gid_table, id);

  if (!m->name) {
    if (persistent_passwd)
      gr = p_getgrgid(id.gid);

    else
      gr = getgrgid(id.gid);

    if (gr) {
      m->name = pstrdup(session.pool ? session.pool : permanent_pool,
        gr->gr_name);
      return mod_create_data(cmd, m->name);
    }

    return PR_DECLINED(cmd);
  }

  return mod_create_data(cmd, m->name);
}

MODRET pw_name2uid(cmd_rec *cmd) {
  struct passwd *pw;
  const char *name;

  name = cmd->argv[0];

  if (persistent_passwd)
    pw = p_getpwnam(name);

  else
    pw = getpwnam(name);

  return pw ? mod_create_data(cmd, (void *) &pw->pw_uid) : PR_DECLINED(cmd);
}

MODRET pw_name2gid(cmd_rec *cmd) {
  struct group *gr;
  const char *name;

  name = cmd->argv[0];

  if (persistent_passwd)
    gr = p_getgrnam(name);

  else
    gr = getgrnam(name);

  return gr ? mod_create_data(cmd, (void *) &gr->gr_gid) : PR_DECLINED(cmd);
}

/* cmd->argv[0] = name
 * cmd->argv[1] = (array_header **) group_ids
 * cmd->argv[2] = (array_header **) group_names
 */

MODRET pw_getgroups(cmd_rec *cmd) {
  struct passwd *pw = NULL;
  struct group *gr = NULL;
  array_header *gids = NULL, *groups = NULL;
  char **gr_member = NULL, *name = NULL;

  /* function pointers for which lookup functions to use */
  struct passwd *(*my_getpwnam)(const char *) = NULL;
  struct group *(*my_getgrgid)(gid_t) = NULL;
  struct group *(*my_getgrent)(void) = NULL;
  RETSETGRENTTYPE (*my_setgrent)(void) = NULL;

  /* play function pointer games */
  if (persistent_passwd) {
    my_getpwnam = p_getpwnam;
    my_getgrgid = p_getgrgid;
    my_getgrent = p_getgrent;
    my_setgrent = p_setgrent;

  } else {
    my_getpwnam = getpwnam;
    my_getgrgid = getgrgid;
    my_getgrent = getgrent;
    my_setgrent = setgrent;
  }

  name = (char *) cmd->argv[0];

  /* Check for NULL values. */
  if (cmd->argv[1])
    gids = (array_header *) cmd->argv[1];

  if (cmd->argv[2])
    groups = (array_header *) cmd->argv[2];

  /* Retrieve the necessary info. */
  if (!name || !(pw = my_getpwnam(name)))
    return PR_DECLINED(cmd);

  /* Populate the first group ID and name. */
  if (gids)
    *((gid_t *) push_array(gids)) = pw->pw_gid;

  if (groups && (gr = my_getgrgid(pw->pw_gid)) != NULL)
    *((char **) push_array(groups)) = pstrdup(session.pool, gr->gr_name);

  my_setgrent();

  /* This is where things get slow, expensive, and ugly.  Loop through
   * everything, checking to make sure we haven't already added it.
   */
  while ((gr = my_getgrent()) != NULL && gr->gr_mem) {

    /* Loop through each member name listed */
    for (gr_member = gr->gr_mem; *gr_member; gr_member++) {

      /* If it matches the given username... */
      if (!strcmp(*gr_member, pw->pw_name)) {

        /* ...add the GID and name */
        if (gids)
          *((gid_t *) push_array(gids)) = gr->gr_gid;

        if (groups && pw->pw_gid != gr->gr_gid)
          *((char **) push_array(groups)) = pstrdup(session.pool,
            gr->gr_name);
      }
    }
  }

  if (gids && gids->nelts > 0)
    return mod_create_data(cmd, (void *) &gids->nelts);

  else if (groups && groups->nelts > 0)
    return mod_create_data(cmd, (void *) &groups->nelts);

  return PR_DECLINED(cmd);
}

MODRET set_persistentpasswd(cmd_rec *cmd) {
  int bool = -1;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if ((bool = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  persistent_passwd = bool;

  return PR_HANDLED(cmd);
}

/* Events handlers
 */

static void auth_unix_exit_ev(const void *event_data, void *user_data) {
  pr_auth_endpwent(session.pool);
  pr_auth_endgrent(session.pool);

  return;
}

/* Initialization routines
 */

static int auth_unix_init(void) {
  memset(uid_table, 0 ,sizeof(uid_table));
  memset(gid_table, 0, sizeof(gid_table));

#ifdef HAVE__PW_STAYOPEN
  _pw_stayopen = 1;
#endif

  return 0;
}

static int auth_unix_sess_init(void) {
  pr_event_register(&auth_unix_module, "core.exit", auth_unix_exit_ev, NULL);
  return 0;
}

/* Module API tables
 */

static conftable auth_unix_conftab[] = {
  { "PersistentPasswd",		set_persistentpasswd,		NULL },
  { NULL,			NULL,				NULL }
};

static authtable auth_unix_authtab[] = {
  { 0,  "setpwent",	pw_setpwent },
  { 0,  "endpwent",	pw_endpwent },
  { 0,  "setgrent",     pw_setgrent },
  { 0,  "endgrent",	pw_endgrent },
  { 0,	"getpwent",	pw_getpwent },
  { 0,  "getgrent",	pw_getgrent },
  { 0,  "getpwnam",	pw_getpwnam },
  { 0,	"getpwuid",	pw_getpwuid },
  { 0,  "getgrnam",     pw_getgrnam },
  { 0,  "getgrgid",     pw_getgrgid },
  { 0,  "auth",         pw_auth	},
  { 0,  "check",	pw_check },
  { 0,  "uid2name",	pw_uid2name },
  { 0,  "gid2name",	pw_gid2name },
  { 0,  "name2uid",	pw_name2uid },
  { 0,  "name2gid",	pw_name2gid },
  { 0,  "getgroups",	pw_getgroups },
  { 0,  NULL }
};

module auth_unix_module = {
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "auth_unix",

  /* Module configuration handler table */
  auth_unix_conftab,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  auth_unix_authtab,

  /* Module initialization */
  auth_unix_init,

  /* Session initialization */
  auth_unix_sess_init
};
