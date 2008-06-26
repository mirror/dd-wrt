/*
 * ProFTPD - FTP server daemon
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
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Controls API routines
 *
 * $Id: ctrls.c,v 1.16 2006/12/16 01:13:52 castaglia Exp $
 */

#include "conf.h"

#ifdef PR_USE_CTRLS

typedef struct ctrls_act_obj {
  struct ctrls_act_obj *prev, *next;
  pool *pool;
  unsigned int id;
  const char *action;
  const char *desc;
  const module *module;
  volatile unsigned int flags;
  int (*action_cb)(pr_ctrls_t *, int, char **);
} ctrls_action_t;

static unsigned char ctrls_blocked = FALSE;

static pool *ctrls_pool = NULL;
static ctrls_action_t *ctrls_action_list = NULL;

static pr_ctrls_t *ctrls_active_list = NULL;
static pr_ctrls_t *ctrls_free_list = NULL;

static int ctrls_use_isfifo = FALSE;

static const char *trace_channel = "ctrls";

/* lookup/lookup_next indices */
static ctrls_action_t *action_lookup_next = NULL;
static const char *action_lookup_action = NULL;
static module *action_lookup_module = NULL;

/* necessary prototypes */
static ctrls_action_t *ctrls_action_new(void);
static pr_ctrls_t *ctrls_new(void);
static pr_ctrls_t *ctrls_lookup_action(module *, const char *, unsigned char);
static pr_ctrls_t *ctrls_lookup_next_action(module *, unsigned char);

static pr_ctrls_t *ctrls_prepare(ctrls_action_t *act) {
  pr_ctrls_t *ctrl = NULL;

  /* Sanity check */
  if (!act)
    return NULL;

  pr_block_ctrls();

  /* Get a blank ctrl object */
  ctrl = ctrls_new();

  /* Fill in the fields from the action object. */
  ctrl->ctrls_id = act->id;
  ctrl->ctrls_module = act->module;
  ctrl->ctrls_action = act->action;
  ctrl->ctrls_desc = act->desc;
  ctrl->ctrls_cb = act->action_cb;
  ctrl->ctrls_flags = act->flags;

  /* Add this to the "in use" list */
  ctrl->ctrls_next = ctrls_active_list;
  ctrls_active_list = ctrl;

  pr_unblock_ctrls();
  return ctrl;
}

static void ctrls_free(pr_ctrls_t *ctrl) {

  /* Make sure that ctrls are blocked while we're doing this */
  pr_block_ctrls();

  /* Remove this object from the active list */
  if (ctrl->ctrls_prev)
    ctrl->ctrls_prev->ctrls_next = ctrl->ctrls_next;

  else
    ctrls_active_list = ctrl->ctrls_next;

  if (ctrl->ctrls_next)
    ctrl->ctrls_next->ctrls_prev = ctrl->ctrls_prev;

  /* Clear its fields, and add it to the free list */
  ctrl->ctrls_next = NULL;
  ctrl->ctrls_prev = NULL;
  ctrl->ctrls_id = 0;
  ctrl->ctrls_module = NULL;
  ctrl->ctrls_action = NULL;
  ctrl->ctrls_cb = NULL;
  ctrl->ctrls_cb_retval = 1;
  ctrl->ctrls_flags = 0;

  if (ctrl->ctrls_tmp_pool) {
    destroy_pool(ctrl->ctrls_tmp_pool);
    ctrl->ctrls_tmp_pool = NULL;
  }

  ctrl->ctrls_cb_args = NULL;
  ctrl->ctrls_cb_resps = NULL;
  ctrl->ctrls_data = NULL;

  ctrl->ctrls_next = ctrls_free_list;
  ctrls_free_list = ctrl;

  pr_unblock_ctrls();
  return;
}

static ctrls_action_t *ctrls_action_new(void) {
  ctrls_action_t *act = NULL;
  pool *sub_pool = make_sub_pool(ctrls_pool);

  pr_pool_tag(sub_pool, "ctrls action subpool");
  act = pcalloc(sub_pool, sizeof(ctrls_action_t));
  act->pool = sub_pool;

  return act;
}

static pr_ctrls_t *ctrls_new(void) {
  pr_ctrls_t *ctrl = NULL;

  /* Check for a free ctrl first */
  if (ctrls_free_list) {

    /* Take one from the top */
    ctrl = ctrls_free_list;
    ctrls_free_list = ctrls_free_list->ctrls_next;

    if (ctrls_free_list)
      ctrls_free_list->ctrls_prev = NULL;

  } else {

    /* Have to allocate a new one. */
    ctrl = (pr_ctrls_t *) pcalloc(ctrls_pool, sizeof(pr_ctrls_t));

    /* It's important that a new ctrl object have the retval initialized
     * to 1; this tells the Controls layer that it is "pending", not yet
     * handled.
     */
    ctrl->ctrls_cb_retval = 1;
  }

  return ctrl;
}

static char *ctrls_sep(char **str) {
  char *ret = NULL, *dst = NULL;
  unsigned char quoted = FALSE;

  /* Sanity checks */
  if (!str || !*str || !**str)
    return NULL;

  while (**str && isspace((int) **str))
    (*str)++;

  if (!**str)
    return NULL;

  ret = dst = *str;

  if (**str == '\"') {
    quoted = TRUE;
    (*str)++;
  }

  while (**str &&
         (quoted ? (**str != '\"') : !isspace((int) **str))) {

    if (**str == '\\' && quoted) {

      /* Escaped char */
      if (*((*str) + 1))
        *dst = *(++(*str));
    }

    *dst++ = **str;
    ++(*str);
  }

  if (**str)
    (*str)++;
  *dst = '\0';

  return ret;
}

int pr_ctrls_register(const module *mod, const char *action,
    const char *desc, int (*cb)(pr_ctrls_t *, int, char **)) {
  ctrls_action_t *act = NULL, *acti = NULL;
  int act_id = -1;

  /* sanity checks */
  if (!action || !desc || !cb) {
    errno = EINVAL;
    return -1;
  }

  pr_trace_msg("ctrls", 3,
    "module '%s' registering handler for ctrl action '%s' (at %p)",
    mod ? mod->name : "(none)", action, cb);

  /* Block ctrls while we're doing this */
  pr_block_ctrls();

  /* Get a ctrl action object */
  act = ctrls_action_new();

  /* Randomly generate a unique random ID for this object */
  while (TRUE) {
    unsigned char have_id = FALSE;
    act_id = rand();

    /* Check the list for this ID */
    for (acti = ctrls_action_list; acti; acti = acti->next) {
      if (acti->id == act_id) {
        have_id = TRUE;
        break;
      }
    }

    if (!have_id)
      break;
  }

  act->next = NULL;
  act->id = act_id;
  act->action = action;
  act->desc = desc;
  act->module = mod;
  act->action_cb = cb;

  /* Add this to the list of "registered" actions */

  if (ctrls_action_list) {
    act->next = ctrls_action_list;
    ctrls_action_list->prev = act;
  }
  
  ctrls_action_list = act;

  pr_unblock_ctrls();

  return act_id;
}

int pr_ctrls_unregister(module *mod, const char *action) {
  ctrls_action_t *act = NULL;
  unsigned char have_action = FALSE;

  /* sanity checks */
  if (!action) {
    errno = EINVAL;
    return -1;
  }

  pr_trace_msg("ctrls", 3,
    "module '%s' unregistering handler for ctrl action '%s'",
    mod ? mod->name : "(none)", action);

  /* Make sure that ctrls are blocked while we're doing this */
  pr_block_ctrls();

  for (act = ctrls_action_list; act; act = act->next) {
    if (strcmp(act->action, action) == 0 &&
        (act->module == mod || mod == ANY_MODULE || mod == NULL)) {
      have_action = TRUE;

      /* Remove this object from the list of registered actions */
      if (act->prev)
        act->prev->next = act->next;

      else
        ctrls_action_list = act->next;

      if (act->next)
        act->next->prev = act->prev;

      /* Destroy this action. */
      destroy_pool(act->pool); 
    }
  }
  
  pr_unblock_ctrls();

  if (!have_action) {
    errno = ENOENT;
    return -1;
  }

  return 0;
}

int pr_ctrls_add_arg(pr_ctrls_t *ctrl, char *ctrls_arg) {

  /* Sanity checks */
  if (!ctrl || !ctrls_arg) {
    errno = EINVAL;
    return -1;
  }

  /* Make sure the pr_ctrls_t has a temporary pool, from which the args will
   * be allocated.
   */
  if (!ctrl->ctrls_tmp_pool) {
    ctrl->ctrls_tmp_pool = make_sub_pool(ctrls_pool);
    pr_pool_tag(ctrl->ctrls_tmp_pool, "ctrls tmp pool");
  }

  if (!ctrl->ctrls_cb_args)
    ctrl->ctrls_cb_args = make_array(ctrl->ctrls_tmp_pool, 0, sizeof(char *));

  /* Add the given argument */
  *((char **) push_array(ctrl->ctrls_cb_args)) = pstrdup(ctrl->ctrls_tmp_pool,
    ctrls_arg);

  return 0;
}

int pr_ctrls_copy_args(pr_ctrls_t *src_ctrl, pr_ctrls_t *dst_ctrl) {

  /* Sanity checks */
  if (!src_ctrl || !dst_ctrl) {
    errno = EINVAL;
    return -1;
  }

  /* If source ctrl has no ctrls_cb_args member, there's nothing to be
   * done.
   */
  if (!src_ctrl->ctrls_cb_args)
    return 0;

  /* Make sure the pr_ctrls_t has a temporary pool, from which the args will
   * be allocated.
   */
  if (!dst_ctrl->ctrls_tmp_pool) {
    dst_ctrl->ctrls_tmp_pool = make_sub_pool(ctrls_pool);
    pr_pool_tag(dst_ctrl->ctrls_tmp_pool, "ctrls tmp pool");
  }

  /* Overwrite any existing dst_ctrl->ctrls_cb_args.  This is OK, as
   * the ctrl will be reset (cleared) once it has been processed.
   */
  dst_ctrl->ctrls_cb_args = copy_array(dst_ctrl->ctrls_tmp_pool,
    src_ctrl->ctrls_cb_args);

  return 0;
}

int pr_ctrls_copy_resps(pr_ctrls_t *src_ctrl, pr_ctrls_t *dst_ctrl) {

  /* sanity checks */
  if (!src_ctrl || !dst_ctrl) {
    errno = EINVAL;
    return -1;
  }

  /* The source ctrl must have a ctrls_cb_resps member, and the destination
   * ctrl must not have a ctrls_cb_resps member.
   */
  if (!src_ctrl->ctrls_cb_resps || dst_ctrl->ctrls_cb_resps) {
    errno = EINVAL;
    return -1;
  }

  dst_ctrl->ctrls_cb_resps = copy_array(dst_ctrl->ctrls_tmp_pool,
    src_ctrl->ctrls_cb_resps);

  return 0;
}

int pr_ctrls_add_response(pr_ctrls_t *ctrl, char *fmt, ...) {
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  va_list resp;

  /* Sanity check */
  if (!ctrl || !fmt) {
    errno = EINVAL;
    return -1;
  }

  /* Make sure the pr_ctrls_t has a temporary pool, from which the responses
   * will be allocated
   */
  if (!ctrl->ctrls_tmp_pool) {
    ctrl->ctrls_tmp_pool = make_sub_pool(ctrls_pool);
    pr_pool_tag(ctrl->ctrls_tmp_pool, "ctrls tmp pool");
  }

  if (!ctrl->ctrls_cb_resps)
    ctrl->ctrls_cb_resps = make_array(ctrl->ctrls_tmp_pool, 0,
      sizeof(char *));

  /* Affix the message */
  va_start(resp, fmt);
  vsnprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), fmt, resp);
  va_end(resp);

  buf[sizeof(buf) - 1] = '\0';

  /* add the given response */
  *((char **) push_array(ctrl->ctrls_cb_resps)) =
    pstrdup(ctrl->ctrls_tmp_pool, buf);

  return 0;
}

int pr_ctrls_flush_response(pr_ctrls_t *ctrl) {

  /* Sanity check */
  if (!ctrl) {
    errno = EINVAL;
    return -1;
  }

  /* Make sure the callback(s) added responses */
  if (ctrl->ctrls_cb_resps) {
    if (pr_ctrls_send_msg(ctrl->ctrls_cl->cl_fd, ctrl->ctrls_cb_retval,
        ctrl->ctrls_cb_resps->nelts,
        (char **) ctrl->ctrls_cb_resps->elts) < 0)
      return -1;
  }

  return 0;
}

int pr_ctrls_parse_msg(pool *msg_pool, char *msg, unsigned int *msgargc,
    char ***msgargv) {
  char *tmp = msg, *str = NULL;
  pool *tmp_pool = NULL;
  array_header *msgs = NULL;

  /* Sanity checks */
  if (!msg_pool || !msgargc || !msgargv) {
    errno = EINVAL;
    return -1;
  }

  tmp_pool = make_sub_pool(msg_pool);

  /* Allocate an array_header, and push each space-delimited string
   * (respecting quotes and escapes) into the array.  Once done,
   * destroy the array.
   */
 
  msgs = make_array(tmp_pool, 0, sizeof(char *));

  while ((str = ctrls_sep(&tmp)) != NULL)
    *((char **) push_array(msgs)) = pstrdup(msg_pool, str);

  *msgargc = msgs->nelts;
  *msgargv = (char **) msgs->elts;

  destroy_pool(tmp_pool);

  return 0;
}

int pr_ctrls_recv_request(pr_ctrls_cl_t *cl) {
  pr_ctrls_t *ctrl = NULL, *next_ctrl = NULL;
  char reqaction[512] = {'\0'}, *reqarg = NULL;
  size_t reqargsz = 0;
  unsigned int nreqargs = 0, reqarglen = 0;
  int status = 0;
  register int i = 0;

  if (!cl) {
    errno = EINVAL;
    return -1;
  }

  if (cl->cl_fd < 0) {
    errno = EBADF;
    return -1;
  }

  /* No interruptions */
  pr_signals_block();

  /* Read in the incoming number of args, including the action. */

  /* First, read the status (but ignore it).  This is necessary because
   * the same function, pr_ctrls_send_msg(), is used to send requests
   * as well as responses, and the status is a necessary part of a response.
   */
  if (read(cl->cl_fd, &status, sizeof(int)) < 0) {
    pr_signals_unblock();
    return -1;
  }
 
  /* Read in the args, length first, then string. */
  if (read(cl->cl_fd, &nreqargs, sizeof(unsigned int)) < 0) {
    pr_signals_unblock();
    return -1;
  }

  /* Next, read in the requested number of arguments.  The client sends
   * the arguments in pairs: first the length of the argument, then the
   * argument itself.  The first argument is the action, so get the first
   * matching pr_ctrls_t (if present), and add the remaining arguments to it.
   */
  
  if (read(cl->cl_fd, &reqarglen, sizeof(unsigned int)) < 0) {
    pr_signals_unblock();
    return -1;
  }

  if (reqarglen >= sizeof(reqaction)) {
    pr_signals_unblock();
    errno = ENOMEM;
    return -1;
  }

  memset(reqaction, '\0', sizeof(reqaction));

  if (read(cl->cl_fd, reqaction, reqarglen) < 0) {
    pr_signals_unblock();
    return -1;
  }

  reqaction[sizeof(reqaction)-1] = '\0';
  nreqargs--;

  /* Find a matching action object, and use it to populate a ctrl object,
   * preparing the ctrl object for dispatching to the action handlers.
   */
  ctrl = ctrls_lookup_action(NULL, reqaction, TRUE);
  if (ctrl == NULL) {
    pr_signals_unblock();
    errno = EINVAL;
    return -1;
  }

  for (i = 0; i < nreqargs; i++) {
    memset(reqarg, '\0', reqargsz);

    if (read(cl->cl_fd, &reqarglen, sizeof(unsigned int)) < 0) {
      pr_signals_unblock();
      return -1;
    }

    /* Make sure reqarg is large enough to handle the given argument.  If
     * it is too small, allocate one of the necessary size.
     */

    if (reqargsz < reqarglen) {
      reqargsz = reqarglen + 1;

      if (!ctrl->ctrls_tmp_pool) {
        ctrl->ctrls_tmp_pool = make_sub_pool(ctrls_pool);
        pr_pool_tag(ctrl->ctrls_tmp_pool, "ctrls tmp pool");
      }

      reqarg = pcalloc(ctrl->ctrls_tmp_pool, reqargsz);
    }

    if (read(cl->cl_fd, reqarg, reqarglen) < 0) {
      pr_signals_unblock();
      return -1;
    }

    if (pr_ctrls_add_arg(ctrl, reqarg)) {
      pr_signals_unblock();
      return -1;
    }
  }

  /* Add this ctrls object to the client object. */
  *((pr_ctrls_t **) push_array(cl->cl_ctrls)) = ctrl;

  /* Set the flag that this control is ready to go */
  ctrl->ctrls_flags |= PR_CTRLS_REQUESTED;
  ctrl->ctrls_cl = cl;

  /* Copy the populated ctrl object args to ctrl objects for all other
   * matching action objects.
   */
  next_ctrl = ctrls_lookup_next_action(NULL, TRUE);

  while (next_ctrl) {
    if (pr_ctrls_copy_args(ctrl, next_ctrl))
      return -1;

    /* Add this ctrl object to the client object. */
    *((pr_ctrls_t **) push_array(cl->cl_ctrls)) = next_ctrl;

    /* Set the flag that this control is ready to go. */ 
    next_ctrl->ctrls_flags |= PR_CTRLS_REQUESTED;
    next_ctrl->ctrls_cl = cl;

    next_ctrl = ctrls_lookup_next_action(NULL, TRUE);
  }

  pr_signals_unblock();
  return 0;
}

int pr_ctrls_recv_response(pool *resp_pool, int ctrls_sockfd,
    int *status, char ***respargv) {
  register int i = 0;
  array_header *resparr = NULL;
  unsigned int respargc = 0, resparglen = 0;
  char response[PR_TUNABLE_BUFFER_SIZE] = {'\0'};

  /* Sanity checks */
  if (!resp_pool || ctrls_sockfd < 0 || !status) {
    errno = EINVAL;
    return -1;
  }

  resparr = make_array(resp_pool, 0, sizeof(char *));

  /* No interruptions. */
  pr_signals_block();

  /* First, read the status, which is the return value of the control handler.
   */
  if (read(ctrls_sockfd, status, sizeof(int)) != sizeof(int)) {
    pr_signals_unblock();
    return -1;
  }

  /* Next, read the number of responses to be received */
  if (read(ctrls_sockfd, &respargc, sizeof(unsigned int)) !=
      sizeof(unsigned int)) {
    pr_signals_unblock();
    return -1;
  }

  /* Read each response, and add it to the array */ 
  for (i = 0; i < respargc; i++) {
    int bread = 0, blen = 0;

    if (read(ctrls_sockfd, &resparglen,
        sizeof(unsigned int)) != sizeof(unsigned int)) {
      pr_signals_unblock();
      return -1;
    }

    /* Make sure resparglen is not too big */
    if (resparglen >= sizeof(response)) {
      pr_signals_unblock();
      errno = ENOMEM;
      return -1;
    }

    memset(response, '\0', sizeof(response));

    bread = read(ctrls_sockfd, response, resparglen);
    while (bread != resparglen) {
      if (bread < 0) {
        pr_signals_unblock(); 
        return -1;
      }

      blen += bread;
      bread = read(ctrls_sockfd, response + blen, resparglen - blen);
    }

    /* Always make sure the buffer is zero-terminated */
    response[sizeof(response)-1] = '\0';

    *((char **) push_array(resparr)) = pstrdup(resp_pool, response);
  }

  if (respargv)
    *respargv = ((char **) resparr->elts);

  pr_signals_unblock(); 
  return respargc;
}

int pr_ctrls_send_msg(int sockfd, int msgstatus, unsigned int msgargc,
    char **msgargv) {
  register int i = 0;
  unsigned int msgarglen = 0;

  /* Sanity checks */
  if (sockfd < 0) {
    errno = EINVAL;
    return -1;
  }

  if (msgargc < 1)
    return 0;    

  if (msgargv == NULL)
    return 0;

  /* No interruptions */
  pr_signals_block();

  /* Send the message status first */
  if (write(sockfd, &msgstatus, sizeof(int)) != sizeof(int)) {
    pr_signals_unblock();
    return -1;
  }

  /* Send the strings, one argument at a time.  First, send the number
   * of arguments to be sent; then send, for each argument, first the
   * length of the argument string, then the argument itself.
   */
  if (write(sockfd, &msgargc, sizeof(unsigned int)) !=
      sizeof(unsigned int)) {
    pr_signals_unblock();
    return -1;
  }

  for (i = 0; i < msgargc; i++) {
    int res = 0;

    msgarglen = strlen(msgargv[i]);

    while (TRUE) {
      res = write(sockfd, &msgarglen, sizeof(unsigned int));

      if (res != sizeof(unsigned int)) {
        if (errno == EAGAIN)
          continue;

        pr_signals_unblock();
        return -1;

      } else
        break;
    }

    while (TRUE) {
      res = write(sockfd, msgargv[i], msgarglen);

      if (res != msgarglen) {
        if (errno == EAGAIN)
          continue;

        pr_signals_unblock();
        return -1;

      } else
        break;
    }
  }

  pr_signals_unblock();
  return 0;
}

static pr_ctrls_t *ctrls_lookup_action(module *mod, const char *action,
    unsigned char skip_disabled) {

  /* (Re)set the current indices */
  action_lookup_next = ctrls_action_list;
  action_lookup_action = action;
  action_lookup_module = mod;

  /* Wrapper around ctrls_lookup_next_action() */
  return ctrls_lookup_next_action(mod, skip_disabled);
}

static pr_ctrls_t *ctrls_lookup_next_action(module *mod,
    unsigned char skip_disabled) {
  ctrls_action_t *act = NULL;

  /* Sanity check */
  if (!action_lookup_action) {
    errno = EINVAL;
    return NULL;
  }

  if (mod != action_lookup_module)
    return ctrls_lookup_action(mod, action_lookup_action, skip_disabled);

  for (act = action_lookup_next; act; act = act->next) {

    if (skip_disabled && (act->flags & PR_CTRLS_ACT_DISABLED))
      continue;

    if (strcmp(act->action, action_lookup_action) == 0 &&
        (act->module == mod || mod == ANY_MODULE || mod == NULL)) {

      action_lookup_next = act->next;

      /* Use this action object to prepare a ctrl object. */
      return ctrls_prepare(act);
    }
  }

  return NULL;
}

int pr_get_registered_actions(pr_ctrls_t *ctrl, int flags) {
  ctrls_action_t *act = NULL;

  /* Are ctrls blocked? */
  if (ctrls_blocked) {
    errno = EPERM;
    return -1;
  }

  for (act = ctrls_action_list; act; act = act->next) {
    switch (flags) {
      case CTRLS_GET_ACTION_ALL:
        pr_ctrls_add_response(ctrl, "%s (mod_%s.c)", act->action,
          act->module->name);
        break;

      case CTRLS_GET_ACTION_ENABLED:
        if (act->flags & PR_CTRLS_ACT_DISABLED)
          break;
        pr_ctrls_add_response(ctrl, "%s (mod_%s.c)", act->action,
          act->module->name);
        break;

      case CTRLS_GET_DESC:
        pr_ctrls_add_response(ctrl, "%s: %s", act->action,
          act->desc);
        break;
    }
  }

  return 0;
}

int pr_set_registered_actions(module *mod, const char *action,
    unsigned char skip_disabled, unsigned int flags) {
  ctrls_action_t *act = NULL;
  unsigned char have_action = FALSE;

  /* Is flags a valid combination of settable flags? */
  if (flags && flags != PR_CTRLS_ACT_DISABLED) {
    errno = EINVAL;
    return -1;
  }

  /* Are ctrls blocked? */
  if (ctrls_blocked) {
    errno = EPERM;
    return -1;
  }

  for (act = ctrls_action_list; act; act = act->next) {
    if (skip_disabled && (act->flags & PR_CTRLS_ACT_DISABLED))
      continue;

    if ((!action ||
         strcmp(action, "all") == 0 ||
         strcmp(act->action, action) == 0) &&
        (act->module == mod || mod == ANY_MODULE || mod == NULL)) {
      have_action = TRUE;
      act->flags = flags;
    }
  }

  if (!have_action) {
    errno = ENOENT;
    return -1;
  }

  return 0;
}

int pr_ctrls_connect(const char *socket_file) {
  int sockfd = -1, len = 0;
  struct sockaddr_un cl_sock, ctrl_sock;

  /* No interruptions */
  pr_signals_block();

  /* Create a Unix domain socket */
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    pr_signals_unblock();
    return -1;
  }

  /* Fill in the socket address */
  memset(&cl_sock, 0, sizeof(cl_sock));

  /* This first part is clever.  First, this process creates a socket in
   * the file system.  It _then_ connect()s to the server.  Upon accept()ing
   * the connection, the server examines the created socket to see that it
   * is indeed a socket, with the proper mode and time.  Clever, but not
   * ideal.
   */

  cl_sock.sun_family = AF_UNIX;
  sprintf(cl_sock.sun_path, "%s%05u", "/tmp/ftp.cl", (unsigned int) getpid());
  len = sizeof(cl_sock);

  /* Make sure the file doesn't already exist */
  unlink(cl_sock.sun_path);

  /* Make it a socket */
  if (bind(sockfd, (struct sockaddr *) &cl_sock, len) < 0) {
    unlink(cl_sock.sun_path);
    pr_signals_unblock();
    return -1;
  }

  /* Set the proper mode */
  if (chmod(cl_sock.sun_path, PR_CTRLS_CL_MODE) < 0) {
    unlink(cl_sock.sun_path);
    pr_signals_unblock();
    return -1;
  }

  /* Now connect to the real server */
  memset(&ctrl_sock, 0, sizeof(ctrl_sock));

  ctrl_sock.sun_family = AF_UNIX;
  strncpy(ctrl_sock.sun_path, socket_file, strlen(socket_file));
  len = sizeof(ctrl_sock);

  if (connect(sockfd, (struct sockaddr *) &ctrl_sock, len) < 0) {
    unlink(cl_sock.sun_path);
    pr_signals_unblock();
    return -1;
  }

  pr_signals_unblock();
  return sockfd;
}

int pr_ctrls_issock_unix(mode_t sock_mode) {

  if (ctrls_use_isfifo) {
#ifdef S_ISFIFO
    if (S_ISFIFO(sock_mode)) {
      return 0;
    }
#endif /* S_ISFIFO */
  } else {
#ifdef S_ISSOCK
    if (S_ISSOCK(sock_mode)) {
      return 0;
    }
#endif /* S_ISSOCK */
  }

  errno = ENOSYS;
  return -1;
}

void pr_block_ctrls(void) {
  ctrls_blocked = TRUE;
}

void pr_unblock_ctrls(void) {
  ctrls_blocked = FALSE;
}

int pr_check_actions(void) {
  ctrls_action_t *act = NULL;

  for (act = ctrls_action_list; act; act = act->next) {

    if (act->flags & PR_CTRLS_ACT_SOLITARY) {
      /* This is a territorial action -- only one instance allowed */
      if (ctrls_lookup_action(NULL, act->action, FALSE)) {
        pr_log_pri(PR_LOG_NOTICE,
          "duplicate controls for '%s' action not allowed",
          act->action);
        return -1;
      }
    }
  }

  return 0;
}

int pr_run_ctrls(module *mod, const char *action) {
  pr_ctrls_t *ctrl = NULL;

  /* Are ctrls blocked? */
  if (ctrls_blocked) {
    errno = EPERM;
    return -1;
  }

  for (ctrl = ctrls_active_list; ctrl; ctrl = ctrl->ctrls_next) {

    /* Be watchful of the various client-side flags.  Note: if
     * ctrl->ctrls_cl is ever NULL, it means there's a bug in the code.
     */
    if (ctrl->ctrls_cl->cl_flags != PR_CTRLS_CL_HAVEREQ)
      continue;

    /* Has this control been disabled? */
    if (ctrl->ctrls_flags & PR_CTRLS_ACT_DISABLED)
      continue;

    /* Is it time to trigger this ctrl? */
    if (!(ctrl->ctrls_flags & PR_CTRLS_REQUESTED))
      continue;

    if (ctrl->ctrls_when > time(NULL)) {
      ctrl->ctrls_flags |= PR_CTRLS_PENDING;
      pr_ctrls_add_response(ctrl, "request pending");
      continue;
    }

    if (action &&
        strcmp(ctrl->ctrls_action, action) == 0) {
      pr_trace_msg(trace_channel, 7, "calling '%s' control handler",
        ctrl->ctrls_action);

      /* Invoke the callback, if the ctrl's action matches.  Unblock
       * ctrls before invoking the callback, then re-block them after the
       * callback returns.  This will allow the action handlers to use some
       * of the Controls API functions correctly.
       */
      pr_unblock_ctrls();
      ctrl->ctrls_cb_retval = ctrl->ctrls_cb(ctrl,
        (ctrl->ctrls_cb_args ? ctrl->ctrls_cb_args->nelts : 0),
        (ctrl->ctrls_cb_args ? (char **) ctrl->ctrls_cb_args->elts : NULL));
      pr_block_ctrls();

      if (ctrl->ctrls_cb_retval < 1) {
        ctrl->ctrls_flags &= ~PR_CTRLS_REQUESTED;
        ctrl->ctrls_flags &= ~PR_CTRLS_PENDING;
        ctrl->ctrls_flags |= PR_CTRLS_HANDLED;
      }

    } else if (!action) {
      pr_trace_msg(trace_channel, 7, "calling '%s' control handler",
        ctrl->ctrls_action);

      /* If no action was given, invoke every callback */
      pr_unblock_ctrls();
      ctrl->ctrls_cb_retval = ctrl->ctrls_cb(ctrl,
        (ctrl->ctrls_cb_args ? ctrl->ctrls_cb_args->nelts : 0),
        (ctrl->ctrls_cb_args ? (char **) ctrl->ctrls_cb_args->elts : NULL));
      pr_block_ctrls();

      if (ctrl->ctrls_cb_retval < 1) {
        ctrl->ctrls_flags &= ~PR_CTRLS_REQUESTED;
        ctrl->ctrls_flags &= ~PR_CTRLS_PENDING;
        ctrl->ctrls_flags |= PR_CTRLS_HANDLED;
      }
    }
  }

  return 0;
}

int pr_reset_ctrls(void) {
  pr_ctrls_t *ctrl = NULL;

  /* NOTE: need a clean_ctrls() or somesuch that will, after sending any
   * responses, iterate through the list and "free" any ctrls whose
   * ctrls_cb_retval is zero.  This feature is used to handle things like
   * shutdown requests in the future -- the request is only considered
   * "processed" when the callback returns zero.  Any non-zero requests are
   * not cleared, and are considered "pending".  However, this brings up the
   * complication of an additional request for that action being issued by the
   * client before the request is processed.  Simplest solution: remove the
   * old request args, and replace them with the new ones.
   *
   * This requires that the return value of the ctrl callback be explicitly
   * documented.
   *
   * How about: ctrls_cb_retval = 1  pending
   *                              0  processed, OK    (reset)
   *                             -1  processed, error (reset)
   */

  for (ctrl = ctrls_active_list; ctrl; ctrl = ctrl->ctrls_next) {
    if (ctrl->ctrls_cb_retval < 1)
      ctrls_free(ctrl);
  }

  return 0;
}

void init_ctrls(void) {
  struct stat st;
  int sockfd;
  struct sockaddr_un sockun;
  size_t socklen;
  char *sockpath = PR_RUN_DIR "/test.sock";

  if (ctrls_pool) {
    destroy_pool(ctrls_pool);
  }

  ctrls_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(ctrls_pool, "Controls Pool");

  /* Make sure all of the lists are zero'd out. */
  ctrls_action_list = NULL;
  ctrls_active_list = NULL;
  ctrls_free_list = NULL;

   /* And that the lookup indices are (re)set as well... */
  action_lookup_next = NULL;
  action_lookup_action = NULL;
  action_lookup_module = NULL;

  /* Run-time check to find out whether this platform identifies a
   * Unix domain socket file descriptor via the S_ISFIFO macro, or
   * the S_ISSOCK macro.
   */

  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    pr_log_pri(PR_LOG_NOTICE, "notice: unable to create Unix domain socket: %s",
      strerror(errno));
    return;
  }

  memset(&sockun, 0, sizeof(sockun));
  sockun.sun_family = AF_UNIX;
  sstrncpy(sockun.sun_path, sockpath, strlen(sockpath) + 1);
  socklen = sizeof(struct sockaddr_un);

  if (bind(sockfd, (struct sockaddr *) &sockun, socklen) < 0) {
    pr_log_pri(PR_LOG_NOTICE,
      "notice: unable to bind to Unix domain socket at '%s': %s",
      sockpath, strerror(errno));
    (void) close(sockfd);
    (void) unlink(sockpath);
    return;
  }

  if (fstat(sockfd, &st) < 0) {
    pr_log_pri(PR_LOG_NOTICE,
      "notice: unable to stat Unix domain socket at '%s': %s",
      sockpath, strerror(errno));
    (void) close(sockfd);
    (void) unlink(sockpath);
    return;
  }

#ifdef S_ISFIFO
  pr_trace_msg(trace_channel, 9, "testing Unix domain socket using S_ISFIFO");
  if (S_ISFIFO(st.st_mode)) {
    ctrls_use_isfifo = TRUE;
  }
#else
  pr_trace_msg(trace_channel, 9, "cannot test Unix domain socket using "
    "S_ISFIFO: macro undefined");
#endif

#ifdef S_ISSOCK
  pr_trace_msg(trace_channel, 9, "testing Unix domain socket using S_ISSOCK");
  if (S_ISSOCK(st.st_mode)) {
    ctrls_use_isfifo = FALSE;
  }
#else
  pr_trace_msg(trace_channel, 9, "cannot test Unix domain socket using "
    "S_ISSOCK: macro undefined");
#endif

  pr_trace_msg(trace_channel, 9, "using %s macro for Unix domain socket "
    "detection", ctrls_use_isfifo ? "S_ISFIFO" : "S_ISSOCK");

  (void) close(sockfd);
  (void) unlink(sockpath);
  return;
}

#endif /* PR_USE_CTRLS */
