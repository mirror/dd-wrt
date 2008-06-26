/*
 * ProFTPD: mod_ctrls -- a module implementing the ftpdctl local socket
 *          server, as well as several utility functions for other Controls
 *          modules
 *
 * Copyright (c) 2000-2007 TJ Saunders
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
 * This is mod_ctrls, contrib software for proftpd 1.2 and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * $Id: mod_ctrls.c,v 1.36 2007/02/15 17:01:19 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"
#include "mod_ctrls.h"

#define MOD_CTRLS_VERSION "mod_ctrls/0.9.4"

/* Master daemon in standalone mode? (from src/main.c) */
extern unsigned char is_master;

module ctrls_module;
static ctrls_acttab_t ctrls_acttab[];

static const char *trace_channel = "ctrls";

/* Hard-coded Controls timer IDs.  Need two, one for the initial timer, one
 * to identify the user-configured-interval timer
 */
#define CTRLS_TIMER_ID       24075

static unsigned int ctrls_interval = 10;

/* Controls listening socket fd */
static int ctrls_sockfd = -1;

#define MOD_CTRLS_DEFAULT_SOCK		PR_RUN_DIR "/proftpd.sock"
static char *ctrls_sock_file = MOD_CTRLS_DEFAULT_SOCK;

/* User/group ownership of the control socket */
static uid_t ctrls_sock_uid = 0;
static gid_t ctrls_sock_gid = 0;

/* Pool for this module's use */
static pool *ctrls_pool = NULL;

/* Required "freshness" of client credential sockets */
static unsigned int ctrls_cl_freshness = 10;

/* Start of the client list */
static pr_ctrls_cl_t *cl_list = NULL;
static unsigned int cl_listlen = 0;
static unsigned int cl_maxlistlen = 5;

/* Controls access control list.  This is for ACLs on the control socket
 * itself, rather than on individual actions.
 */
static ctrls_acl_t ctrls_sock_acl;

static unsigned char ctrls_engine = TRUE;

/* Logging */
static int ctrls_logfd = -1;
static char *ctrls_logname = NULL;

/* Necessary prototypes */
static int ctrls_setblock(int sockfd);
static int ctrls_setnonblock(int sockfd);

/* Support routines
 */

char *ctrls_argsep(char **arg) {
  char *ret = NULL, *dst = NULL;
  char quote_mode = 0;

  if (!arg || !*arg || !**arg)
    return NULL;

  while (**arg && isspace((int) **arg))
    (*arg)++;

  if (!**arg)
    return NULL;

  ret = dst = *arg;

  if (**arg == '\"') {
    quote_mode++;
    (*arg)++;
  }

  while (**arg && **arg != ',' &&
      (quote_mode ? (**arg != '\"') : (!isspace((int) **arg)))) {

    if (**arg == '\\' && quote_mode) {

      /* escaped char */
      if (*((*arg) + 1))
        *dst = *(++(*arg));
    }

    *dst++ = **arg;
    ++(*arg);
  }

  if (**arg)
    (*arg)++;

  *dst = '\0';
  return ret;
}

/* Returns TRUE if the given cl_gid is allowed by the group ACL, FALSE
 * otherwise. Note that the default is to deny everyone, unless an ACL has
 * been configured. 
 */
static unsigned char ctrls_check_group_acl(gid_t cl_gid,
    const ctrls_grp_acl_t *grp_acl) {
  register int i = 0;
  unsigned char res = FALSE;

  /* Note: the special condition of ngids of 1 and gids of NULL signals
   * that all groups are to be treated according to the allow member.
   */
  if (grp_acl->gids) {
    for (i = 0; i < grp_acl->ngids; i++)
      if ((grp_acl->gids)[i] == cl_gid)
        res = TRUE;

  } else if (grp_acl->ngids == 1)
    res = TRUE;

  if (!grp_acl->allow)
    res = !res;

  return res;
}

/* Returns TRUE if the given cl_uid is allowed by the user ACL, FALSE
 * otherwise. Note that the default is to deny everyone, unless an ACL has
 * been configured.
 */
static unsigned char ctrls_check_user_acl(uid_t cl_uid,
    const ctrls_usr_acl_t *usr_acl) {
  register int i = 0;
  unsigned char res = FALSE;

  /* Note: the special condition of nuids of 1 and uids of NULL signals
   * that all users are to be treated according to the allow member.
   */
  if (usr_acl->uids) {
    for (i = 0; i < usr_acl->nuids; i++)
      if ((usr_acl->uids)[i] == cl_uid)
        res = TRUE;

  } else if (usr_acl->nuids == 1)
    res = TRUE;
 
  if (!usr_acl->allow)
    res = !res;

  return res;
}

/* Returns TRUE for allowed, FALSE for denied. */
unsigned char ctrls_check_acl(const pr_ctrls_t *ctrl,
    const ctrls_acttab_t *acttab, const char *action) {
  register unsigned int i = 0;

  for (i = 0; acttab[i].act_action; i++) {
    if (strcmp(acttab[i].act_action, action) == 0) {

      if (!ctrls_check_user_acl(ctrl->ctrls_cl->cl_uid,
            &(acttab[i].act_acl->acl_usrs)) &&
          !ctrls_check_group_acl(ctrl->ctrls_cl->cl_gid,
            &(acttab[i].act_acl->acl_grps))) {

        /* Access denied */
        return FALSE;
      }
    }
  }

  return TRUE;
}

void ctrls_init_acl(ctrls_acl_t *acl) {

  /* Sanity check */
  if (!acl)
    return;

  memset(acl, '\0', sizeof(ctrls_acl_t));
  acl->acl_usrs.allow = acl->acl_grps.allow = TRUE;
}

char **ctrls_parse_acl(pool *acl_pool, char *acl_str) {
  char *name = NULL, *acl_str_dup = NULL, **acl_list = NULL;
  array_header *acl_arr = NULL;
  pool *tmp_pool = NULL;

  /* Sanity checks */
  if (!acl_pool || !acl_str)
    return NULL;

  tmp_pool = make_sub_pool(acl_pool);
  acl_str_dup = pstrdup(tmp_pool, acl_str);
 
  /* Allocate an array */
  acl_arr = make_array(acl_pool, 0, sizeof(char **));

  /* Add each name to the array */
  while ((name = ctrls_argsep(&acl_str_dup)) != NULL) {
    char *tmp = pstrdup(acl_pool, name);

    /* Push the name into the ACL array */
    *((char **) push_array(acl_arr)) = tmp;
  }

  /* Terminate the temp array with a NULL, as is proper. */
  *((char **) push_array(acl_arr)) = NULL;

  acl_list = (char **) acl_arr->elts;
  destroy_pool(tmp_pool);

  /* return the array of names */
  return acl_list;
}

static void ctrls_set_group_acl(pool *grp_acl_pool, ctrls_grp_acl_t *grp_acl,
    const char *allow, char *grouplist) {
  char *group = NULL, **groups = NULL;
  array_header *gidarr = NULL;
  gid_t gid = 0;
  pool *tmp_pool = NULL;

  if (!grp_acl_pool || !grp_acl || !allow || !grouplist)
    return;

  tmp_pool = make_sub_pool(grp_acl_pool);

  if (strcmp(allow, "allow") == 0)
    grp_acl->allow = TRUE;
  else
    grp_acl->allow = FALSE;

  /* Parse the given expression into an array, then retrieve the GID
   * for each given name.
   */
  groups = ctrls_parse_acl(grp_acl_pool, grouplist);

  /* Allocate an array of gid_t's */
  gidarr = make_array(grp_acl_pool, 0, sizeof(gid_t));

  for (group = *groups; group != NULL; group = *++groups) {

    /* Handle a group name of "*" differently. */
    if (strcmp("*", group) == 0) {
      grp_acl->ngids = 1;
      grp_acl->gids = NULL;
      destroy_pool(tmp_pool);
      return;

    } else {
      gid = pr_auth_name2gid(tmp_pool, group);
      if (gid == (gid_t) -1)
        continue;
    }

    *((gid_t *) push_array(gidarr)) = gid;
  }

  grp_acl->ngids = gidarr->nelts;
  grp_acl->gids = (gid_t *) gidarr->elts;

  destroy_pool(tmp_pool);
}

static void ctrls_set_user_acl(pool *usr_acl_pool, ctrls_usr_acl_t *usr_acl,
    const char *allow, char *userlist) {
  char *user = NULL, **users = NULL;
  array_header *uidarr = NULL;
  uid_t uid = 0;
  pool *tmp_pool = NULL;

  /* Sanity checks */
  if (!usr_acl_pool || !usr_acl || !allow || !userlist)
    return;

  tmp_pool = make_sub_pool(usr_acl_pool);

  if (strcmp(allow, "allow") == 0)
    usr_acl->allow = TRUE;
  else
    usr_acl->allow = FALSE;

  /* Parse the given expression into an array, then retrieve the UID
   * for each given name.
   */
  users = ctrls_parse_acl(usr_acl_pool, userlist);

  /* Allocate an array of uid_t's */
  uidarr = make_array(usr_acl_pool, 0, sizeof(uid_t));

  for (user = *users; user != NULL; user = *++users) {

    /* Handle a user name of "*" differently. */
    if (strcmp("*", user) == 0) {
      usr_acl->nuids = 1;
      usr_acl->uids = NULL;
      destroy_pool(tmp_pool);
      return;

    } else {
      uid = pr_auth_name2uid(tmp_pool, user);
      if (uid == (uid_t) -1)
        continue;
    }

    *((uid_t *) push_array(uidarr)) = uid;
  }

  usr_acl->nuids = uidarr->nelts;
  usr_acl->uids = (uid_t *) uidarr->elts;

  destroy_pool(tmp_pool);
}

char *ctrls_set_module_acls(ctrls_acttab_t *acttab, pool *acl_pool,
    char **actions, const char *allow, const char *type, char *list) {
  register unsigned int i = 0;
  unsigned char all_actions = FALSE;

  /* First, sanity check the given list of actions against the actions
   * in the given table.
   */
  for (i = 0; actions[i]; i++) {
    register unsigned int j = 0;
    unsigned char valid_action = FALSE;

    if (strcmp(actions[i], "all") == 0)
      continue;

    for (j = 0; acttab[j].act_action; j++) {
      if (strcmp(actions[i], acttab[j].act_action) == 0) {
        valid_action = TRUE;
        break;
      }
    }

    if (!valid_action)
      return actions[i];
  }

  for (i = 0; actions[i]; i++) {
    register unsigned int j = 0;

    if (!all_actions && strcmp(actions[i], "all") == 0)
      all_actions = TRUE;

    for (j = 0; acttab[j].act_action; j++) {
      if (all_actions || strcmp(actions[i], acttab[j].act_action) == 0) {

        /* Use the type parameter to determine whether the list is of users or
         * of groups.
         */
        if (strcmp(type, "user") == 0)
          ctrls_set_user_acl(acl_pool, &(acttab[j].act_acl->acl_usrs),
            allow, list);

        else if (strcmp(type, "group") == 0)
          ctrls_set_group_acl(acl_pool, &(acttab[j].act_acl->acl_grps),
            allow, list);
      }
    }
  }

  return NULL;
}

char *ctrls_unregister_module_actions(ctrls_acttab_t *acttab,
    char **actions, module *mod) {
  register unsigned int i = 0;

  /* First, sanity check the given actions against the actions supported by
   * this module.
   */
  for (i = 0; actions[i]; i++) {
    register unsigned int j = 0;
    unsigned char valid_action = FALSE;

    for (j = 0; acttab[j].act_action; j++) {
      if (strcmp(actions[i], acttab[j].act_action) == 0) {
        valid_action = TRUE;
        break;
      }
    }

    if (!valid_action)
      return actions[i];
  }

  /* Next, iterate through both lists again, looking for actions of the
   * module _not_ in the given list.
   */
  for (i = 0; acttab[i].act_action; i++) {
    register unsigned int j = 0;
    unsigned char have_action = FALSE;

    for (j = 0; actions[j]; j++) {
      if (strcmp(acttab[i].act_action, actions[j]) == 0) {
        have_action = TRUE;
        break;
      }
    }

    if (have_action) {
      pr_trace_msg(trace_channel, 4, "mod_%s.c: removing '%s' control",
        mod->name, acttab[i].act_action);
      pr_ctrls_unregister(mod, acttab[i].act_action);
      destroy_pool(acttab[i].act_acl->acl_pool);
    }
  }

  return NULL;
}

/* Controls logging routines
 */

static int ctrls_closelog(void) {
  /* sanity check */
  if (ctrls_logfd != -1) {
    close(ctrls_logfd);
    ctrls_logfd = -1;

    ctrls_logname = NULL;
  }

  return 0;
}

int ctrls_log(const char *module_version, const char *fmt, ...) {
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  time_t timestamp = time(NULL);
  struct tm *t = NULL;
  va_list msg;

  /* sanity check */
  if (!ctrls_logname)
    return 0;

  t = pr_localtime(NULL, &timestamp);

  /* prepend the timestamp */ 
  strftime(buf, sizeof(buf), "%b %d %H:%M:%S ", t);
  buf[sizeof(buf) - 1] = '\0';

  /* prepend a small header */
  snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
           "%s[%u]: ", module_version, (unsigned int) getpid());

  buf[sizeof(buf) - 1] = '\0';

  /* affix the message */
  va_start(msg, fmt);
  vsnprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), fmt, msg);
  va_end(msg);
 
  buf[strlen(buf)] = '\n'; 
  buf[sizeof(buf) - 1] = '\0';
   
  while (write(ctrls_logfd, buf, strlen(buf)) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    return -1;
  }

  return 0;
}

static int ctrls_openlog(void) {
  int res = 0;

  /* Sanity check */
  if (ctrls_logname == NULL)
    return 0;

  res = pr_log_openfile(ctrls_logname, &ctrls_logfd, 0640);

  if (res == -1) {
    pr_log_pri(PR_LOG_NOTICE, MOD_CTRLS_VERSION
      ": unable to open ControlsLog '%s': %s", ctrls_logname,
      strerror(errno));

  } else if (res == PR_LOG_WRITABLE_DIR) {
    pr_log_pri(PR_LOG_NOTICE, MOD_CTRLS_VERSION
      ": unable to open ControlsLog '%s': "
      "containing directory is world writable", ctrls_logname);

  } else if (res == PR_LOG_SYMLINK) {
    pr_log_pri(PR_LOG_NOTICE, MOD_CTRLS_VERSION
      ": unable to open ControlsLog '%s': %s is a symbolic link",
      ctrls_logname, ctrls_logname);
  }

  return res;
}

/* Controls client routines
 */

static pr_ctrls_cl_t *ctrls_new_cl(void) {
  pool *cl_pool = NULL;

  if (!cl_list) {

    /* Our first client */
    cl_pool = make_sub_pool(ctrls_pool);
    pr_pool_tag(cl_pool, "Controls client pool");

    cl_list = (pr_ctrls_cl_t *) pcalloc(cl_pool, sizeof(pr_ctrls_cl_t));

    cl_list->cl_pool = cl_pool;
    cl_list->cl_fd = -1;
    cl_list->cl_uid = 0;
    cl_list->cl_user = NULL;
    cl_list->cl_gid = 0;
    cl_list->cl_group = NULL;
    cl_list->cl_pid = 0;
    cl_list->cl_ctrls = make_array(cl_pool, 0, sizeof(pr_ctrls_t *));

    cl_list->cl_next = NULL;
    cl_list->cl_prev = NULL;

    cl_listlen = 1;

  } else {
    pr_ctrls_cl_t *cl = NULL;

    /* Add another victim to the list */
    cl_pool = make_sub_pool(ctrls_pool);
    pr_pool_tag(cl_pool, "Controls client pool");

    cl = (pr_ctrls_cl_t *) pcalloc(cl_pool, sizeof(pr_ctrls_cl_t));

    cl->cl_pool = cl_pool;
    cl->cl_fd = -1;
    cl->cl_uid = 0;
    cl->cl_user = NULL;
    cl->cl_gid = 0;
    cl->cl_group = NULL;
    cl->cl_pid = 0;
    cl->cl_ctrls = make_array(cl->cl_pool, 0, sizeof(pr_ctrls_t *));

    cl->cl_next = cl_list;
    cl->cl_prev = NULL;

    cl_list->cl_prev = cl;
    cl_list = cl;

    cl_listlen++;
  }

  return cl_list;
}

/* Add a new client to the set */
static pr_ctrls_cl_t *ctrls_add_cl(int cl_fd, uid_t cl_uid, gid_t cl_gid,
    pid_t cl_pid, unsigned long cl_flags) {
  pr_ctrls_cl_t *cl = NULL;

  /* Make sure there's an empty entry available */
  cl = ctrls_new_cl();

  cl->cl_fd = cl_fd;
  cl->cl_uid = cl_uid;
  cl->cl_user = pr_auth_uid2name(cl->cl_pool, cl->cl_uid);
  cl->cl_gid = cl_gid;
  cl->cl_group = pr_auth_gid2name(cl->cl_pool, cl->cl_gid);
  cl->cl_pid = cl_pid;
  cl->cl_flags = cl_flags;

  ctrls_log(MOD_CTRLS_VERSION,
    "accepted connection from %s/%s client", cl->cl_user, cl->cl_group);
 
  return cl;
}

/* Remove a client from the set */
static void ctrls_del_cl(pr_ctrls_cl_t *cl) {

  /* Remove this ctr_cl_t from the list, and free it */
  if (cl->cl_next)
    cl->cl_next->cl_prev = cl->cl_prev;

  if (cl->cl_prev)
    cl->cl_prev->cl_next = cl->cl_next;

  else
    cl_list = cl->cl_next;

  close(cl->cl_fd);
  cl->cl_fd = -1;

  destroy_pool(cl->cl_pool);

  cl_listlen--;

  return;
}

/* Controls socket routines
 */

/* Accept a client connection */
static int ctrls_accept(int sockfd, uid_t *uid, gid_t *gid, pid_t *pid) {
  pid_t cl_pid = 0;
  int cl_fd = -1;
  socklen_t len = 0;
  char *tmp = NULL;
  time_t stale_time;
  struct sockaddr_un sock;
  struct stat st;

  len = sizeof(sock);

  while ((cl_fd = accept(sockfd, (struct sockaddr *) &sock, &len)) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    ctrls_log(MOD_CTRLS_VERSION,
      "error: unable to accept on local socket: %s", strerror(errno));

    return -1;
  }

  len -= sizeof(sock.sun_family);

  /* NULL terminate the name */
  sock.sun_path[len] = '\0';

  /* Check the path -- hmmm... */
  PRIVS_ROOT
  while (stat(sock.sun_path, &st) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    PRIVS_RELINQUISH
    ctrls_log(MOD_CTRLS_VERSION,
      "error: unable to stat %s: %s", sock.sun_path, strerror(errno));
    (void) close(cl_fd);
    return -1;
  }
  PRIVS_RELINQUISH

  /* Is it a socket? */
  if (pr_ctrls_issock_unix(st.st_mode) < 0) {
    (void) close(cl_fd);
    errno = ENOTSOCK;
    return -1;
  }

  /* Are the perms _not_ rwx------? */
  if (st.st_mode & (S_IRWXG|S_IRWXO) ||
      ((st.st_mode & S_IRWXU) != PR_CTRLS_CL_MODE)) {
    ctrls_log(MOD_CTRLS_VERSION,
      "error: unable to accept connection: incorrect mode");
    (void) close(cl_fd);
    errno = EPERM;
    return -1;
  }

  /* Is it new enough? */
  stale_time = time(NULL) - ctrls_cl_freshness;

  if (st.st_atime < stale_time ||
      st.st_ctime < stale_time ||
      st.st_mtime < stale_time) {
    char *msg = "error: stale connection";

    ctrls_log(MOD_CTRLS_VERSION,
      "unable to accept connection: stale connection");

    /* Log the times being compared, to aid in debugging this situation. */
    if (st.st_atime < stale_time) {
      time_t age = stale_time - st.st_atime;

      ctrls_log(MOD_CTRLS_VERSION,
         "last access time of '%s' is %lu seconds old (must be less than %u)",
         sock.sun_path, (unsigned long) age, ctrls_cl_freshness);
    }

    if (st.st_ctime < stale_time) {
      time_t age = stale_time - st.st_ctime;

      ctrls_log(MOD_CTRLS_VERSION,
         "last change time of '%s' is %lu seconds old (must be less than %u)",
         sock.sun_path, (unsigned long) age, ctrls_cl_freshness);
    }

    if (st.st_mtime < stale_time) {
      time_t age = stale_time - st.st_mtime;

      ctrls_log(MOD_CTRLS_VERSION,
         "last modified time of '%s' is %lu seconds old (must be less than %u)",
         sock.sun_path, (unsigned long) age, ctrls_cl_freshness);
    }
 
    if (pr_ctrls_send_msg(cl_fd, -1, 1, &msg) < 0)
      ctrls_log("error sending message: %s", strerror(errno));
    close(cl_fd);
    cl_fd = -1;

    /* Done with the path now */
    PRIVS_ROOT
    unlink(sock.sun_path);
    PRIVS_RELINQUISH

    errno = ETIMEDOUT;
    return -1;
  }

  /* Parse the PID out of the path */
  tmp = sock.sun_path;
  tmp += strlen("/tmp/ftp.cl");
  cl_pid = atol(tmp);

  /* Return the IDs of the caller */
  if (uid)
    *uid = st.st_uid;

  if (gid)
    *gid = st.st_gid;

  if (pid)
    *pid = cl_pid;

  /* Done with the path now */
  PRIVS_ROOT
  unlink(sock.sun_path);
  PRIVS_RELINQUISH

  return cl_fd;
}

/* Iterate through any readable descriptors, reading each into appropriate
 * client objects
 */
static void ctrls_cls_read(void) {
  pr_ctrls_cl_t *cl = cl_list;

  while (cl) {
    if (pr_ctrls_recv_request(cl) < 0) {

      if (errno == EOF) {
        ;
 
      } else if (errno == EINVAL) {

        /* Unsupported action requested */
        if (!cl->cl_flags)
          cl->cl_flags = PR_CTRLS_CL_NOACTION;

        ctrls_log(MOD_CTRLS_VERSION,
          "recvd from %s/%s client: (invalid action)", cl->cl_user,
          cl->cl_group);

      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {

        /* Malicious/blocked client */
        if (!cl->cl_flags)
          cl->cl_flags = PR_CTRLS_CL_BLOCKED;

      } else {
        
        ctrls_log(MOD_CTRLS_VERSION,
          "error: unable to receive client request: %s", strerror(errno)); 
      }

    } else {
      pr_ctrls_t *ctrl = *((pr_ctrls_t **) cl->cl_ctrls->elts);
      char *request = (char *) ctrl->ctrls_action;

      /* Request successfully read.  Flag this client as being in such a
       * state.
       */
      if (!cl->cl_flags)
        cl->cl_flags = PR_CTRLS_CL_HAVEREQ;

      if (ctrl->ctrls_cb_args) {
        int reqargc = ctrl->ctrls_cb_args->nelts;
        char **reqargv = ctrl->ctrls_cb_args->elts;

        /* Reconstruct the original request string from the client for
         * logging.
         */
        while (reqargc--)
          request = pstrcat(cl->cl_pool, request, " ", *reqargv++, NULL);

        ctrls_log(MOD_CTRLS_VERSION,
          "recvd from %s/%s client: '%s'", cl->cl_user, cl->cl_group,
          request);
      }
    }

    cl = cl->cl_next;
  }

  return;
}

/* Iterate through any writable descriptors, writing out the responses to the
 * appropriate client objects
 */
static int ctrls_cls_write(void) {
  pr_ctrls_cl_t *cl = cl_list;

  while (cl) {
    /* Necessary to keep track of the next client in the list while
     * the list is being modified.
     */
    pr_ctrls_cl_t *tmpcl = cl->cl_next;

    /* This client has something to hear */
    if (cl->cl_flags == PR_CTRLS_CL_NOACCESS) {
      char *msg = "access denied";

      /* ACL-denied access */
      if (pr_ctrls_send_msg(cl->cl_fd, -1, 1, &msg) < 0)
        ctrls_log(MOD_CTRLS_VERSION,
          "error: unable to send response to %s/%s client: %s",
          cl->cl_user, cl->cl_group, strerror(errno));

      else
        ctrls_log(MOD_CTRLS_VERSION, "sent to %s/%s client: '%s'",
          cl->cl_user, cl->cl_group, msg);

    } else if (cl->cl_flags == PR_CTRLS_CL_NOACTION) {
      char *msg = "unsupported action requested";

      /* Unsupported action -- no matching controls */
      if (pr_ctrls_send_msg(cl->cl_fd, -1, 1, &msg) < 0)
        ctrls_log(MOD_CTRLS_VERSION,
          "error: unable to send response to %s/%s client: %s",
          cl->cl_user, cl->cl_group, strerror(errno));

      else
        ctrls_log(MOD_CTRLS_VERSION, "sent to %s/%s client: '%s'",
          cl->cl_user, cl->cl_group, msg);

    } else if (cl->cl_flags == PR_CTRLS_CL_BLOCKED) {
      char *msg = "blocked connection";

      if (pr_ctrls_send_msg(cl->cl_fd, -1, 1, &msg) < 0)
        ctrls_log(MOD_CTRLS_VERSION,
          "error: unable to send response to %s/%s client: %s",
          cl->cl_user, cl->cl_group, strerror(errno));

      else
        ctrls_log(MOD_CTRLS_VERSION, "sent to %s/%s client: '%s'",
          cl->cl_user, cl->cl_group, msg);

    } else if (cl->cl_flags == PR_CTRLS_CL_HAVEREQ) {

      if (cl->cl_ctrls->nelts > 0) {
        register int i = 0;
        pr_ctrls_t **ctrlv = NULL;

        ctrlv = (pr_ctrls_t **) cl->cl_ctrls->elts;

        if (cl->cl_ctrls) {
          for (i = 0; i < cl->cl_ctrls->nelts; i++) {
            if ((ctrlv[i])->ctrls_cb_retval < 1) {

              /* Make sure the callback(s) added responses */
              if ((ctrlv[i])->ctrls_cb_resps) {
                if (pr_ctrls_send_msg(cl->cl_fd, (ctrlv[i])->ctrls_cb_retval,
                    (ctrlv[i])->ctrls_cb_resps->nelts,
                    (char **) (ctrlv[i])->ctrls_cb_resps->elts) < 0) {
                  ctrls_log(MOD_CTRLS_VERSION,
                    "error: unable to send response to %s/%s "
                    "client: %s", cl->cl_user, cl->cl_group, strerror(errno));

                } else {

                  /* For logging/accounting purposes */
                  register int j = 0;
                  int respval = (ctrlv[i])->ctrls_cb_retval;
                  int respargc = (ctrlv[i])->ctrls_cb_resps->nelts;
                  char **respargv = (ctrlv[i])->ctrls_cb_resps->elts;

                  ctrls_log(MOD_CTRLS_VERSION,
                    "sent to %s/%s client: return value: %d",
                    cl->cl_user, cl->cl_group, respval);

                  for (j = 0; j < respargc; j++) 
                    ctrls_log(MOD_CTRLS_VERSION,
                      "sent to %s/%s client: '%s'", cl->cl_user,
                      cl->cl_group, respargv[j]);
                }

              } else {

                /* No responses added by callbacks */
                ctrls_log(MOD_CTRLS_VERSION,
                  "notice: no responses given for %s/%s client: "
                  "check controls handlers", cl->cl_user, cl->cl_group);
              }
            }
          }
        }
      }
    }

    ctrls_log(MOD_CTRLS_VERSION,
      "closed connection to %s/%s client", cl->cl_user, cl->cl_group);

    /* Remove the client from the list */
    ctrls_del_cl(cl);
    cl = tmpcl;
  }

  return 0;
}

/* Create a listening local socket */
static int ctrls_listen(const char *sock_file) {
  int sockfd = -1, len = 0;
  struct sockaddr_un sock;

  /* No interruptions */
  pr_signals_block();

  /* Create the Unix domain socket */
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    int xerrno = errno;

    pr_signals_unblock();
    errno = xerrno;
    ctrls_log(MOD_CTRLS_VERSION,
      "error: unable to create local socket: %s", strerror(errno));
    return -1;
  }

  /* Make sure the path to which we want to bind this socket doesn't already
   * exist.
   */
  unlink(sock_file);

  /* Fill in the socket structure fields */
  memset(&sock, 0, sizeof(sock));

  sock.sun_family = AF_UNIX;
  strncpy(sock.sun_path, sock_file, strlen(sock_file));

  len = sizeof(sock);

  /* Bind the name to the descriptor */
  pr_trace_msg(trace_channel, 1, "binding ctrls socket to '%s'", sock.sun_path);
  if (bind(sockfd, (struct sockaddr *) &sock, len) < 0) {
    int xerrno = errno;

    pr_signals_unblock();
    (void) close(sockfd);
    errno = xerrno;
    ctrls_log(MOD_CTRLS_VERSION,
      "error: unable to bind to local socket: %s", strerror(xerrno));
    pr_trace_msg(trace_channel, 1, "unable to bind to local socket: %s",
      strerror(xerrno));
    return -1;
  }

  /* Start listening to the socket */
  if (listen(sockfd, 5) < 0) {
    int xerrno = errno;

    pr_signals_unblock();
    (void) close(sockfd);
    errno = xerrno;
    ctrls_log(MOD_CTRLS_VERSION,
      "error: unable to listen on local socket: %s", strerror(xerrno));
    pr_trace_msg(trace_channel, 1, "unable to listen on local socket: %s",
      strerror(xerrno));
    return -1;
  }

  /* Change the permissions on the socket, so that users can connect */
  if (chmod(sock.sun_path, (mode_t) PR_CTRLS_MODE) < 0) {
    int xerrno = errno;

    pr_signals_unblock();
    (void) close(sockfd);
    errno = xerrno;
    ctrls_log(MOD_CTRLS_VERSION,
      "error: unable to chmod local socket: %s", strerror(xerrno));
    pr_trace_msg(trace_channel, 1, "unable to chmod local socket: %s",
      strerror(xerrno));
    return -1;
  }

  pr_signals_unblock();
  return sockfd;
}

static int ctrls_recv_cl_reqs(void) {
  fd_set cl_rset;
  struct timeval timeout;
  uid_t cl_uid;
  gid_t cl_gid;
  pid_t cl_pid;
  unsigned long cl_flags = 0;
  int cl_fd, max_fd;

  timeout.tv_usec = 500L;
  timeout.tv_sec = 0L;

  /* look for any pending client connections */
  while (cl_listlen < cl_maxlistlen) {
    int res = 0;

    pr_signals_handle();

    if (ctrls_sockfd < 0)
      break;

    FD_ZERO(&cl_rset);
    FD_SET(ctrls_sockfd, &cl_rset);
    max_fd = ctrls_sockfd + 1;

    res = select(max_fd + 1, &cl_rset, NULL, NULL, &timeout);
    if (res == 0) {

      /* Go through the client list */
      ctrls_cls_read();

      return 0;
    }

    if (res < 0) {
      if (errno == EINTR) {
        pr_signals_handle();
        continue;
      }

      ctrls_log(MOD_CTRLS_VERSION,
        "error: unable to select on local socket: %s", strerror(errno));
      return res;
    }
 
    if (FD_ISSET(ctrls_sockfd, &cl_rset)) {

      /* Make sure the ctrl socket is non-blocking */
      if (ctrls_setnonblock(ctrls_sockfd) < 0) {
        ctrls_log(MOD_CTRLS_VERSION,
          "error: unable to set nonblocking on local socket: %s",
          strerror(errno));
        return -1;
      }

      /* Accept pending connections */
      if ((cl_fd = ctrls_accept(ctrls_sockfd, &cl_uid, &cl_gid,
          &cl_pid)) < 0) {
        if (errno != ETIMEDOUT)
          ctrls_log(MOD_CTRLS_VERSION,
            "error: unable to accept connection: %s", strerror(errno));
        continue;
      }

      /* Restore blocking mode to the ctrl socket */
      if (ctrls_setblock(ctrls_sockfd) < 0) {
        ctrls_log(MOD_CTRLS_VERSION,
          "error: unable to set blocking on local socket: %s",
          strerror(errno));
      }

      /* Set this socket as non-blocking */
      if (ctrls_setnonblock(cl_fd) < 0) {
        ctrls_log(MOD_CTRLS_VERSION,
          "error: unable to set nonblocking on client socket: %s",
          strerror(errno));
        continue;
      }

      if (!ctrls_check_user_acl(cl_uid, &ctrls_sock_acl.acl_usrs) &&
          !ctrls_check_group_acl(cl_gid, &ctrls_sock_acl.acl_grps))
        cl_flags = PR_CTRLS_CL_NOACCESS;

      /* Add the client to the list */
      ctrls_add_cl(cl_fd, cl_uid, cl_gid, cl_pid, cl_flags);
    }
  }

  /* Go through the client list */
  ctrls_cls_read();

  return 0; 
}

static int ctrls_send_cl_resps(void) {

  /* Go through the client list */
  ctrls_cls_write();

  return 0;
}

static int ctrls_setblock(int sockfd) {
  int flags = 0;
  int res = -1;

  /* default error */
  errno = EBADF;

  flags = fcntl(sockfd, F_GETFL);
  res = fcntl(sockfd, F_SETFL, flags & (U32BITS ^ O_NONBLOCK));

  return res;
}

static int ctrls_setnonblock(int sockfd) {
  int flags = 0;
  int res = -1;

  /* default error */
  errno = EBADF;

  flags = fcntl(sockfd, F_GETFL);
  res = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

  return res;
}

static int ctrls_timer_cb(CALLBACK_FRAME) {
  static unsigned char first = TRUE;

  /* If the ControlsEngine is not to run, do nothing from here on out */
  if (!ctrls_engine) {
    close(ctrls_sockfd);
    ctrls_sockfd = -1;

    if (is_master)
      /* Remove the local socket path as well */
      unlink(ctrls_sock_file);

    return 0;
  }

  if (first) {
    /* Change the ownership on the socket to that configured by the admin */
    PRIVS_ROOT
    if (chown(ctrls_sock_file, ctrls_sock_uid, ctrls_sock_gid) < 0)
      pr_log_pri(PR_LOG_INFO, "mod_ctrls: unable to chown local socket: %s",
        strerror(errno));
    PRIVS_RELINQUISH

    first = FALSE;
  }

  /* Please no alarms while doing this. */
  pr_alarms_block();

  /* Process pending requests. */
  ctrls_recv_cl_reqs();

  /* Run through the controls */
  pr_run_ctrls(NULL, NULL);

  /* Process pending responses */
  ctrls_send_cl_resps();

  /* Reset controls */
  pr_reset_ctrls();

  pr_alarms_unblock();
  return 1;
}

/* Controls handlers
 */

static int respcmp(const void *a, const void *b) {
  return strcmp(*((char **) a), *((char **) b));
}

static int ctrls_handle_help(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {

  /* Run through the list of registered controls, and add them to the
   * response, including the module in which they appear.
   */

  if (reqargc != 0) {
    pr_ctrls_add_response(ctrl, "wrong number of parameters");
    return -1;
  }

  if (pr_get_registered_actions(ctrl, CTRLS_GET_DESC) < 0)
    pr_ctrls_add_response(ctrl, "unable to get actions: %s", strerror(errno));

  else {

    /* Be nice, and sort the directives lexicographically */
    qsort(ctrl->ctrls_cb_resps->elts, ctrl->ctrls_cb_resps->nelts,
      sizeof(char *), respcmp);
  }

  return 0;
}

static int ctrls_handle_insctrl(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  module *m = ANY_MODULE;

  /* Enable a control into the registered controls list. This requires the
   * action and, optionally, the module of the control to be enabled.
   */

  /* Check the insctrl ACL */
  if (!ctrls_check_acl(ctrl, ctrls_acttab, "insctrl")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  if (reqargc < 1 || reqargc > 2) {
    pr_ctrls_add_response(ctrl, "wrong number of parameters");
    return -1;
  }

  /* If the optional second parameter, a module name, is used, lookup
   * the module pointer matching the name.
   */
  if (reqargc == 2)
    m = pr_module_get(reqargv[1]);

  if (pr_set_registered_actions(m, reqargv[0], FALSE, 0) < 0) {

    if (errno == ENOENT)
      pr_ctrls_add_response(ctrl, "no such control: '%s'", reqargv[0]);
    else
      pr_ctrls_add_response(ctrl, "unable to enable '%s': %s", reqargv[0],
        strerror(errno));

  } else
    pr_ctrls_add_response(ctrl, "'%s' control enabled", reqargv[0]);

  return 0;
}

static int ctrls_handle_lsctrl(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {

  /* Run through the list of registered controls, and add them to the
   * response, including the module in which they appear.
   */

  /* Check the lsctrl ACL */
  if (!ctrls_check_acl(ctrl, ctrls_acttab, "lsctrl")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  if (reqargc != 0) {
    pr_ctrls_add_response(ctrl, "wrong number of parameters");
    return -1;
  }

  if (pr_get_registered_actions(ctrl, CTRLS_GET_ACTION_ENABLED) < 0)
    pr_ctrls_add_response(ctrl, "unable to get actions: %s", strerror(errno));

  else {

    /* Be nice, and sort the actions lexicographically */
    qsort(ctrl->ctrls_cb_resps->elts, ctrl->ctrls_cb_resps->nelts,
      sizeof(char *), respcmp);

  }

  return 0;
}

static int ctrls_handle_rmctrl(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  module *m = ANY_MODULE;
  
  /* Disable a control from the registered controls list. This requires the
   * action and, optionally, the module of the control to be removed.
   */

  /* Check the rmctrl ACL */
  if (!ctrls_check_acl(ctrl, ctrls_acttab, "rmctrl")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  if (reqargc < 1 || reqargc > 2) {
    pr_ctrls_add_response(ctrl, "wrong number of parameters");
    return -1;
  }

  /* The three controls added by this module _cannot_ be removed (at least
   * not via this control handler).
   */
  if (strcmp(reqargv[0], "insctrl") == 0 ||
      strcmp(reqargv[0], "lsctrl") == 0 ||
      strcmp(reqargv[0], "rmctrl") == 0) {
    pr_ctrls_add_response(ctrl, "'%s' control cannot be removed", reqargv[0]);
    return -1;
  }

  /* If the optional second parameter, a module name, is used, lookup
   * the module pointer matching the name.
   */
  if (reqargc == 2)
    m = pr_module_get(reqargv[1]);

  if (pr_set_registered_actions(m, reqargv[0], FALSE,
      PR_CTRLS_ACT_DISABLED) < 0) {

    if (errno == ENOENT)
      pr_ctrls_add_response(ctrl, "no such control: '%s'", reqargv[0]);
    else
      pr_ctrls_add_response(ctrl, "unable to disable '%s': %s", reqargv[0],
        strerror(errno));

  } else {
    if (strcmp(reqargv[0], "all") != 0) 
      pr_ctrls_add_response(ctrl, "'%s' control disabled", reqargv[0]);

    else {

      /* If all actions have been disabled, stop listening on the local
       * socket, and turn off this module's engine.
       */
      pr_ctrls_add_response(ctrl, "all controls disabled");
      pr_ctrls_add_response(ctrl, "restart the daemon to re-enable controls");

      close(ctrls_sockfd);
      ctrls_sockfd = -1;

      ctrls_engine = FALSE;
    }
  }

  return 0;
}

/* Configuration handlers
 */

/* Default behavior is to deny everyone unless an ACL has been configured */
MODRET set_ctrlsacls(cmd_rec *cmd) {
  char *bad_action = NULL, **actions = NULL;

  CHECK_ARGS(cmd, 4);
  CHECK_CONF(cmd, CONF_ROOT);

  /* Parse the given string of actions into a char **.  Then iterate
   * through the acttab, checking to see if a given control is _not_ in
   * the list.  If not in the list, unregister that control.
   */

  /* We can cheat here, and use the ctrls_parse_acl() routine to
   * separate the given string...
   */
  actions = ctrls_parse_acl(cmd->tmp_pool, cmd->argv[1]);

  /* Check the second parameter to make sure it is "allow" or "deny" */
  if (strcmp(cmd->argv[2], "allow") != 0 &&
      strcmp(cmd->argv[2], "deny") != 0)
    CONF_ERROR(cmd, "second parameter must be 'allow' or 'deny'");

  /* Check the third parameter to make sure it is "user" or "group" */
  if (strcmp(cmd->argv[3], "user") != 0 &&
      strcmp(cmd->argv[3], "group") != 0)
    CONF_ERROR(cmd, "third parameter must be 'user' or 'group'");

  bad_action = ctrls_set_module_acls(ctrls_acttab, ctrls_pool, actions,
    cmd->argv[2], cmd->argv[3], cmd->argv[4]);
  if (bad_action != NULL)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown action: '",
      bad_action, "'", NULL));

  return PR_HANDLED(cmd);
}

/* default: 10 secs */
MODRET set_ctrlsauthfreshness(cmd_rec *cmd) {
  int freshness = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  freshness = atoi(cmd->argv[1]);
  if (freshness <= 0)
    CONF_ERROR(cmd, "must be a positive number");

  ctrls_cl_freshness = freshness;

  return PR_HANDLED(cmd);
}

MODRET set_ctrlsengine(cmd_rec *cmd) {
  int bool = -1;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  ctrls_engine = bool;
  return PR_HANDLED(cmd);
}

/* default: 10 secs */
MODRET set_ctrlsinterval(cmd_rec *cmd) {
  int nsecs = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if ((nsecs = atoi(cmd->argv[1])) <= 0)
    CONF_ERROR(cmd, "must be a positive number");

  /* Remove the existing timer, and re-install it with this new interval. */
  ctrls_interval = nsecs;

  pr_timer_remove(CTRLS_TIMER_ID, &ctrls_module);
  pr_timer_add(ctrls_interval, CTRLS_TIMER_ID, &ctrls_module, ctrls_timer_cb);

  return PR_HANDLED(cmd);
}

MODRET set_ctrlslog(cmd_rec *cmd) {
  int res = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  ctrls_logname = pstrdup(ctrls_pool, cmd->argv[1]);

  res = ctrls_openlog();
  if (res < 0) {
    if (res == -1)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unable to open '",
        cmd->argv[1], "': ", strerror(errno), NULL));

    if (res == -2)
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
        "unable to log to a world-writable directory", NULL));
  }

  return PR_HANDLED(cmd);
}

/* Default: 5 max clients */
MODRET set_ctrlsmaxclients(cmd_rec *cmd) {
  int nclients = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  nclients = atoi(cmd->argv[1]);
  if (nclients <= 0)
    CONF_ERROR(cmd, "must be a positive number");

  cl_maxlistlen = nclients;
  return PR_HANDLED(cmd);
}

/* Default: var/run/proftpd.sock */
MODRET set_ctrlssocket(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (*cmd->argv[1] != '/')
    CONF_ERROR(cmd, "must be an absolute path");

  /* Close the socket. */
  if (ctrls_sockfd >= 0) {
    pr_trace_msg(trace_channel, 3, "closing ctrls socket '%s' (fd %d)",
      ctrls_sock_file, ctrls_sockfd);
    close(ctrls_sockfd);
    ctrls_sockfd = -1;
  }

  /* Change the path. */
  if (strcmp(cmd->argv[1], ctrls_sock_file) != 0)
    ctrls_sock_file = pstrdup(ctrls_pool, cmd->argv[1]);

  return PR_HANDLED(cmd);
}

/* Default behavior is to deny everyone unless an ACL has been configured */
MODRET set_ctrlssocketacl(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 3);
  CHECK_CONF(cmd, CONF_ROOT);

  ctrls_init_acl(&ctrls_sock_acl);

  /* Check the first argument to make sure it either "allow" or "deny" */
  if (strcmp(cmd->argv[1], "allow") != 0 &&
      strcmp(cmd->argv[1], "deny") != 0)
    CONF_ERROR(cmd, "first parameter must be either 'allow' or 'deny'");

  /* Check the second argument to see how to handle the directive */
  if (strcmp(cmd->argv[2], "user") == 0)
    ctrls_set_user_acl(ctrls_pool, &ctrls_sock_acl.acl_usrs, cmd->argv[1],
      cmd->argv[3]);

  else if (strcmp(cmd->argv[2], "group") == 0)
    ctrls_set_group_acl(ctrls_pool, &ctrls_sock_acl.acl_grps, cmd->argv[1],
      cmd->argv[3]);

  else
    CONF_ERROR(cmd, "second parameter must be either 'user' or 'group'");

  return PR_HANDLED(cmd);
}

/* Default: root root */
MODRET set_ctrlssocketowner(cmd_rec *cmd) {
  gid_t gid = 0;
  uid_t uid = 0;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT);

  uid = pr_auth_name2uid(cmd->tmp_pool, cmd->argv[1]);
  if (uid == (uid_t) -1) {
    if (errno != EINVAL)
      pr_log_debug(DEBUG0, "%s: %s has UID of -1", cmd->argv[0],
        cmd->argv[1]);

    else
      pr_log_debug(DEBUG0, "%s: no such user '%s'", cmd->argv[0],
        cmd->argv[1]);

  } else
    ctrls_sock_uid = uid;

  gid = pr_auth_name2gid(cmd->tmp_pool, cmd->argv[2]);
  if (gid == (gid_t) -1) {
    if (errno != EINVAL)
      pr_log_debug(DEBUG0, "%s: %s has GID of -1", cmd->argv[0],
        cmd->argv[2]);

    else
      pr_log_debug(DEBUG0, "%s: no such group '%s'", cmd->argv[0],
        cmd->argv[2]);

  } else
    ctrls_sock_gid = gid;

  return PR_HANDLED(cmd);
}

/* Event handlers
 */

static void ctrls_exit_ev(const void *event_data, void *user_data) {
  if (!is_master || !ctrls_engine)
    return;

  /* Close any connected clients */
  if (cl_list) {
    pr_ctrls_cl_t *cl = NULL;

    for (cl = cl_list; cl; cl = cl->cl_next) {
      close(cl->cl_fd);
      cl->cl_fd = -1;
    }
  }

  return;
}

static void ctrls_postparse_ev(const void *event_data, void *user_data) {

  /* Start listening on the ctrl socket */
  PRIVS_ROOT
  ctrls_sockfd = ctrls_listen(ctrls_sock_file);
  PRIVS_RELINQUISH
  if (ctrls_sockfd < 0) {
    pr_log_pri(PR_LOG_NOTICE, "notice: unable to listen to local socket: %s",
      strerror(errno));

  } else {
    /* Ensure that the listen socket used is not one of the major three
     * (stdin, stdout, or stderr).
     */
    if (ctrls_sockfd < 3) {
      if (dup2(ctrls_sockfd, 3) < 0) {
        pr_log_pri(PR_LOG_NOTICE, MOD_CTRLS_VERSION
          ": error duplicating listen socket: %s", strerror(errno));
        (void) close(ctrls_sockfd);
        ctrls_sockfd = -1;

      } else {
        (void) close(ctrls_sockfd);
        ctrls_sockfd = 3;
      }
    }
  }

}

static void ctrls_restart_ev(const void *event_data, void *user_data) {
  register unsigned int i;

  /* Block alarms while we're preparing for the restart. */
  pr_alarms_block();

  /* Close any connected clients */
  if (cl_list) {
    pr_ctrls_cl_t *cl = NULL;

    for (cl = cl_list; cl; cl = cl->cl_next) {
      close(cl->cl_fd);
      cl->cl_fd = -1;
    }
  }

  /* Reset the client list */
  cl_list = NULL;
  cl_listlen = 0;

  pr_trace_msg(trace_channel, 3, "closing ctrls socket '%s' (fd %d)",
    ctrls_sock_file, ctrls_sockfd);
  close(ctrls_sockfd);
  ctrls_sockfd = -1;

  ctrls_closelog();

  /* Clear the existing pool */
  if (ctrls_pool) {
    destroy_pool(ctrls_pool);

    ctrls_logname = NULL;
    ctrls_sock_file = MOD_CTRLS_DEFAULT_SOCK;
  }

  /* Allocate the pool for this module's use */
  ctrls_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(ctrls_pool, MOD_CTRLS_VERSION);

  /* Register the control handlers */
  for (i = 0; ctrls_acttab[i].act_action; i++) {

    /* Allocate and initialize the ACL for this control. */
    ctrls_acttab[i].act_acl = pcalloc(ctrls_pool, sizeof(ctrls_acl_t));
    ctrls_init_acl(ctrls_acttab[i].act_acl);
  }

  pr_alarms_unblock();
  return;
}

static void ctrls_startup_ev(const void *event_data, void *user_data) {

  /* Start a timer for the checking/processing of the ctrl socket.  */
  pr_timer_remove(CTRLS_TIMER_ID, &ctrls_module);
  pr_timer_add(ctrls_interval, CTRLS_TIMER_ID, &ctrls_module, ctrls_timer_cb);
}

/* Initialization routines
 */

static int ctrls_init(void) {
  register unsigned int i = 0; 

  /* Allocate the pool for this module's use */
  ctrls_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(ctrls_pool, MOD_CTRLS_VERSION);

  /* Register the control handlers */
  for (i = 0; ctrls_acttab[i].act_action; i++) {

    /* Allocate and initialize the ACL for this control. */
    ctrls_acttab[i].act_acl = pcalloc(ctrls_pool, sizeof(ctrls_acl_t));
    ctrls_init_acl(ctrls_acttab[i].act_acl);

    if (pr_ctrls_register(&ctrls_module, ctrls_acttab[i].act_action,
        ctrls_acttab[i].act_desc, ctrls_acttab[i].act_cb) < 0)
      pr_log_pri(PR_LOG_INFO, MOD_CTRLS_VERSION
        ": error registering '%s' control: %s",
        ctrls_acttab[i].act_action, strerror(errno));
  }

  /* Make certain the socket ACL is initialized. */
  memset(&ctrls_sock_acl, '\0', sizeof(ctrls_acl_t));
  ctrls_sock_acl.acl_usrs.allow = ctrls_sock_acl.acl_grps.allow = FALSE;

  /* Start listening on the ctrl socket */
  ctrls_sockfd = ctrls_listen(ctrls_sock_file);
  if (ctrls_sockfd < 0)
    pr_log_pri(PR_LOG_NOTICE, "notice: unable to listen to local socket: %s",
      strerror(errno));

  pr_event_register(&ctrls_module, "core.exit", ctrls_exit_ev, NULL);
  pr_event_register(&ctrls_module, "core.postparse", ctrls_postparse_ev, NULL);
  pr_event_register(&ctrls_module, "core.restart", ctrls_restart_ev, NULL);
  pr_event_register(&ctrls_module, "core.startup", ctrls_startup_ev, NULL);

  return 0;
}

static int ctrls_sess_init(void) {

  /* Children are not to listen for or handle control requests */
  ctrls_engine = FALSE;
  pr_timer_remove(CTRLS_TIMER_ID, &ctrls_module);

  pr_event_unregister(&ctrls_module, "core.exit", ctrls_exit_ev);
  pr_event_unregister(&ctrls_module, "core.restart", ctrls_restart_ev);

  /* Close the inherited socket */
  close(ctrls_sockfd);
  ctrls_sockfd = -1;
 
  return 0;
}

static ctrls_acttab_t ctrls_acttab[] = {
  { "help",	"describe all registered controls", NULL,
    ctrls_handle_help },
  { "insctrl",	"enable a disabled control", NULL, 
    ctrls_handle_insctrl },
  { "lsctrl",	"list all registered controls", NULL, 
    ctrls_handle_lsctrl },
  { "rmctrl",	"disable a registered control", NULL,
    ctrls_handle_rmctrl },
  { NULL, NULL, NULL, NULL }
};

/* Module API tables
 */

static conftable ctrls_conftab[] = {
  { "ControlsACLs",		set_ctrlsacls,		NULL },
  { "ControlsAuthFreshness",	set_ctrlsauthfreshness,	NULL },
  { "ControlsEngine",		set_ctrlsengine,	NULL },
  { "ControlsInterval",		set_ctrlsinterval,	NULL },
  { "ControlsLog",		set_ctrlslog,		NULL },
  { "ControlsMaxClients",	set_ctrlsmaxclients,	NULL },
  { "ControlsSocket",		set_ctrlssocket,	NULL },
  { "ControlsSocketACL",	set_ctrlssocketacl,	NULL },
  { "ControlsSocketOwner",	set_ctrlssocketowner,	NULL },
  { NULL }
};

module ctrls_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "ctrls",

  /* Module configuration handler table */
  ctrls_conftab,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  ctrls_init,

  /* Session initialization function */
  ctrls_sess_init,

  /* Module version */
  MOD_CTRLS_VERSION
};
