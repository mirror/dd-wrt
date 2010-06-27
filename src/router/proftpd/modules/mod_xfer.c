/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2010 The ProFTPD Project team
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

/* Data transfer module for ProFTPD
 *
 * $Id: mod_xfer.c,v 1.269 2010/02/10 21:45:17 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"

#include <signal.h>

#ifdef HAVE_SYS_SENDFILE_H
# include <sys/sendfile.h>
#endif

#ifdef HAVE_REGEX_H
# include <regex.h>
#endif

/* Minimum priority a process can have. */
#ifndef PRIO_MIN
# define PRIO_MIN	-20
#endif

/* Maximum priority a process can have.  */
#ifndef PRIO_MAX
# define PRIO_MAX	20
#endif

extern module auth_module;
extern pid_t mpid;

/* Variables for this module */
static pr_fh_t *retr_fh = NULL;
static pr_fh_t *stor_fh = NULL;
static pr_fh_t *displayfilexfer_fh = NULL;

static unsigned char have_prot = FALSE;
static unsigned char have_zmode = FALSE;
static unsigned char use_sendfile = TRUE;

static int xfer_check_limit(cmd_rec *);

/* Transfer priority */
static int xfer_prio_config = 0;
static int xfer_prio_curr = 0;
static unsigned long xfer_prio_flags = 0;
#define PR_XFER_PRIO_FL_APPE  0x0001
#define PR_XFER_PRIO_FL_RETR  0x0002
#define PR_XFER_PRIO_FL_STOR  0x0004
#define PR_XFER_PRIO_FL_STOU  0x0008

static int xfer_prio_adjust(void);
static int xfer_prio_restore(void);

/* Used for MaxTransfersPerHost and TransferRate */
static int xfer_parse_cmdlist(const char *, config_rec *, char *);

module xfer_module;

static int xfer_errno;
static int xfer_logged_sendfile_decline_msg = FALSE;

static const char *trace_channel = "xfer";

static unsigned long find_max_nbytes(char *directive) {
  config_rec *c = NULL;
  unsigned int ctxt_precedence = 0;
  unsigned char have_user_limit, have_group_limit, have_class_limit,
    have_all_limit;
  unsigned long max_nbytes = 0UL;

  have_user_limit = have_group_limit = have_class_limit =
    have_all_limit = FALSE;

  c = find_config(CURRENT_CONF, CONF_PARAM, directive, FALSE);

  while (c) {

    /* This check is for more than three arguments: one argument is the
     * classifier (i.e. "user", "group", or "class"), one argument is
     * the precedence, one is the number of bytes; the remaining arguments
     * are the individual items in the configured expression.
     */

    if (c->argc > 3) {
      if (strcmp(c->argv[2], "user") == 0) {

        if (pr_expr_eval_user_or((char **) &c->argv[3]) == TRUE) {
          if (*((unsigned int *) c->argv[1]) > ctxt_precedence) {

            /* Set the context precedence */
            ctxt_precedence = *((unsigned int *) c->argv[1]);

            max_nbytes = *((unsigned long *) c->argv[0]);

            have_group_limit = have_class_limit = have_all_limit = FALSE;
            have_user_limit = TRUE;
          }
        }

      } else if (strcmp(c->argv[2], "group") == 0) {

        if (pr_expr_eval_group_or((char **) &c->argv[3]) == TRUE) {
          if (*((unsigned int *) c->argv[1]) > ctxt_precedence) {

            /* Set the context precedence */
            ctxt_precedence = *((unsigned int *) c->argv[1]);

            max_nbytes = *((unsigned long *) c->argv[0]);

            have_user_limit = have_class_limit = have_all_limit = FALSE;
            have_group_limit = TRUE;
          }
        }

      } else if (strcmp(c->argv[2], "class") == 0) {

        if (pr_expr_eval_class_or((char **) &c->argv[3]) == TRUE) {
          if (*((unsigned int *) c->argv[1]) > ctxt_precedence) {

            /* Set the context precedence */
            ctxt_precedence = *((unsigned int *) c->argv[1]);

            max_nbytes = *((unsigned long *) c->argv[0]);

            have_user_limit = have_group_limit = have_all_limit = FALSE;
            have_class_limit = TRUE;
          }
        }
      }

    } else {

      if (*((unsigned int *) c->argv[1]) > ctxt_precedence) {

        /* Set the context precedence. */
        ctxt_precedence = *((unsigned int *) c->argv[1]);

        max_nbytes = *((unsigned long *) c->argv[0]);

        have_user_limit = have_group_limit = have_class_limit = FALSE;
        have_all_limit = TRUE;
      }
    }

    c = find_config_next(c, c->next, CONF_PARAM, directive, FALSE);
  }

  /* Print out some nice debugging information. */
  if (max_nbytes > 0UL &&
      (have_user_limit || have_group_limit ||
       have_class_limit || have_all_limit)) {
    pr_log_debug(DEBUG5, "%s (%lu bytes) in effect for %s",
      directive, max_nbytes,
      have_user_limit ? "user " : have_group_limit ? "group " :
      have_class_limit ? "class " : "all");
  }

  return max_nbytes;
}

static unsigned long parse_max_nbytes(char *nbytes_str, char *units_str) {
  long res;
  unsigned long nbytes;
  char *endp = NULL;
  float units_factor = 0.0;

  /* clear any previous local errors */
  xfer_errno = 0;

  /* first, check the given units to determine the correct mulitplier
   */
  if (!strcasecmp("Gb", units_str)) {
    units_factor = 1024.0 * 1024.0 * 1024.0;

  } else if (!strcasecmp("Mb", units_str)) {
    units_factor = 1024.0 * 1024.0;

  } else if (!strcasecmp("Kb", units_str)) {
    units_factor = 1024.0;

  } else if (!strcasecmp("b", units_str)) {
    units_factor = 1.0;

  } else {
    xfer_errno = EINVAL;
    return 0;
  }

  /* make sure a number was given */
  if (!isdigit((int) *nbytes_str)) {
    xfer_errno = EINVAL;
    return 0;
  }

  /* knowing the factor, now convert the given number string to a real
   * number
   */
  res = strtol(nbytes_str, &endp, 10);

  if (errno == ERANGE) {
    xfer_errno = ERANGE;
    return 0;
  }

  if (endp && *endp) {
    xfer_errno = EINVAL;
    return 0;
  }

  /* don't bother to apply the factor if that will cause the number to
   * overflow
   */
  if (res > (ULONG_MAX / units_factor)) {
    xfer_errno = ERANGE;
    return 0;
  }

  nbytes = (unsigned long) res * units_factor;
  return nbytes;
}

static void _log_transfer(char direction, char abort_flag) {
  struct timeval end_time;
  char *fullpath = NULL;

  memset(&end_time, '\0', sizeof(end_time));

  if (session.xfer.start_time.tv_sec != 0) {
    gettimeofday(&end_time, NULL);
    end_time.tv_sec -= session.xfer.start_time.tv_sec;

    if (end_time.tv_usec >= session.xfer.start_time.tv_usec)
      end_time.tv_usec -= session.xfer.start_time.tv_usec;

    else {
      end_time.tv_usec = 1000000L - (session.xfer.start_time.tv_usec -
        end_time.tv_usec);
      end_time.tv_sec--;
    }
  }

  fullpath = dir_abs_path(session.xfer.p, session.xfer.path, TRUE);

  if ((session.sf_flags & SF_ANON) != 0) {
    xferlog_write(end_time.tv_sec, pr_netaddr_get_sess_remote_name(),
      session.xfer.total_bytes, fullpath,
      (session.sf_flags & SF_ASCII ? 'a' : 'b'), direction,
      'a', session.anon_user, abort_flag);

  } else {
    xferlog_write(end_time.tv_sec, pr_netaddr_get_sess_remote_name(),
      session.xfer.total_bytes, fullpath,
      (session.sf_flags & SF_ASCII ? 'a' : 'b'), direction,
      'r', session.user, abort_flag);
  }

  pr_log_debug(DEBUG1, "Transfer %s %" PR_LU " bytes in %ld.%02lu seconds",
    abort_flag == 'c' ? "completed:" : "aborted after",
    (pr_off_t) session.xfer.total_bytes, (long) end_time.tv_sec,
    (unsigned long)(end_time.tv_usec / 10000));
}

/* Code borrowed from src/dirtree.c's get_word() -- modified to separate
 * words on commas as well as spaces.
 */
static char *get_cmd_from_list(char **list) {
  char *res = NULL, *dst = NULL;
  unsigned char quote_mode = FALSE;

  while (**list && isspace((int) **list))
    (*list)++;

  if (!**list)
    return NULL;

  res = dst = *list;

  if (**list == '\"') {
    quote_mode = TRUE;
    (*list)++;
  }

  while (**list && **list != ',' &&
      (quote_mode ? (**list != '\"') : (!isspace((int) **list)))) {

    if (**list == '\\' && quote_mode) {

      /* escaped char */
      if (*((*list) + 1))
        *dst = *(++(*list));
    }

    *dst++ = **list;
    ++(*list);
  }

  if (**list)
    (*list)++;

  *dst = '\0';

  return res;
}

static int xfer_check_limit(cmd_rec *cmd) {
  config_rec *c = NULL;
  const char *client_addr = pr_netaddr_get_ipstr(session.c->remote_addr);
  char server_addr[128];

  memset(server_addr, '\0', sizeof(server_addr));
  snprintf(server_addr, sizeof(server_addr)-1, "%s:%d",
    pr_netaddr_get_ipstr(main_server->addr), main_server->ServerPort);
  server_addr[sizeof(server_addr)-1] = '\0';

  c = find_config(CURRENT_CONF, CONF_PARAM, "MaxTransfersPerHost", FALSE);
  while (c) {
    char *xfer_cmd = NULL, **cmdlist = (char **) c->argv[0];
    unsigned char matched_cmd = FALSE;
    unsigned int curr = 0, max = 0;
    pr_scoreboard_entry_t *score = NULL;

    pr_signals_handle();

    /* Does this MaxTransfersPerHost apply to the current command?  Note: this
     * could be made more efficient by using bitmasks rather than string
     * comparisons.
     */
    for (xfer_cmd = *cmdlist; xfer_cmd; xfer_cmd = *(cmdlist++)) {
      if (strcasecmp(xfer_cmd, cmd->argv[0]) == 0) {
        matched_cmd = TRUE;
        break;
      }
    }

    if (!matched_cmd) {
      c = find_config_next(c, c->next, CONF_PARAM, "MaxTransfersPerHost",
        FALSE);
      continue;
    }

    max = *((unsigned int *) c->argv[1]);

    /* Count how many times the current IP address is logged in, AND how
     * many of those other logins are currently using this command.
     */

    (void) pr_rewind_scoreboard();
    while ((score = pr_scoreboard_entry_read()) != NULL) {
      pr_signals_handle();

      /* Scoreboard entry must match local server address and remote client
       * address to be counted.
       */
      if (strcmp(score->sce_server_addr, server_addr) != 0)
        continue;

      if (strcmp(score->sce_client_addr, client_addr) != 0)
        continue;

      if (strcmp(score->sce_cmd, xfer_cmd) == 0)
        curr++;
    }

    pr_restore_scoreboard();

    if (curr >= max) {
      char maxn[20];

      char *maxstr = "Sorry, the maximum number of data transfers (%m) from "
        "your host are currently being used.";

      if (c->argv[2] != NULL)
        maxstr = c->argv[2];

      pr_event_generate("mod_xfer.max-transfers-per-host", session.c);

      memset(maxn, '\0', sizeof(maxn));
      snprintf(maxn, sizeof(maxn)-1, "%u", max);
      pr_response_send(R_451, "%s", sreplace(cmd->tmp_pool, maxstr, "%m",
        maxn, NULL));
      pr_log_debug(DEBUG4, "MaxTransfersPerHost %u exceeded for %s for "
        "client '%s'", max, xfer_cmd, client_addr);

      return -1;
    }

    c = find_config_next(c, c->next, CONF_PARAM, "MaxTransfersPerHost", FALSE);
  }

  c = find_config(CURRENT_CONF, CONF_PARAM, "MaxTransfersPerUser", FALSE);
  while (c) {
    char *xfer_cmd = NULL, **cmdlist = (char **) c->argv[0];
    unsigned char matched_cmd = FALSE;
    unsigned int curr = 0, max = 0;
    pr_scoreboard_entry_t *score = NULL;

    pr_signals_handle();

    /* Does this MaxTransfersPerUser apply to the current command?  Note: this
     * could be made more efficient by using bitmasks rather than string
     * comparisons.
     */
    for (xfer_cmd = *cmdlist; xfer_cmd; xfer_cmd = *(cmdlist++)) {
      if (strcasecmp(xfer_cmd, cmd->argv[0]) == 0) {
        matched_cmd = TRUE;
        break;
      }
    }

    if (!matched_cmd) {
      c = find_config_next(c, c->next, CONF_PARAM, "MaxTransfersPerUser",
        FALSE);
      continue;
    }

    max = *((unsigned int *) c->argv[1]);

    /* Count how many times the current user is logged in, AND how many of
     * those other logins are currently using this command.
     */

    (void) pr_rewind_scoreboard();
    while ((score = pr_scoreboard_entry_read()) != NULL) {
      pr_signals_handle();

      if (strcmp(score->sce_server_addr, server_addr) != 0)
        continue;

      if (strcmp(score->sce_user, session.user) != 0)
        continue;

      if (strcmp(score->sce_cmd, xfer_cmd) == 0)
        curr++;
    }

    pr_restore_scoreboard();

    if (curr >= max) {
      char maxn[20];

      char *maxstr = "Sorry, the maximum number of data transfers (%m) from "
        "this user are currently being used.";

      if (c->argv[2] != NULL)
        maxstr = c->argv[2];

      pr_event_generate("mod_xfer.max-transfers-per-user", session.user);

      memset(maxn, '\0', sizeof(maxn));
      snprintf(maxn, sizeof(maxn)-1, "%u", max);
      pr_response_send(R_451, "%s", sreplace(cmd->tmp_pool, maxstr, "%m",
        maxn, NULL));
      pr_log_debug(DEBUG4, "MaxTransfersPerUser %u exceeded for %s for "
        "user '%s'", max, xfer_cmd, session.user);

      return -1;
    }

    c = find_config_next(c, c->next, CONF_PARAM, "MaxTransfersPerUser", FALSE);
  }

  return 0;
}

static int xfer_displayfile(void) {
  int res = -1;

  if (displayfilexfer_fh) {
    if (pr_display_fh(displayfilexfer_fh, session.vwd, R_226) < 0) {
      pr_log_debug(DEBUG6, "unable to display DisplayFileTransfer "
        "file '%s': %s", displayfilexfer_fh->fh_path, strerror(errno));
    }

    /* Rewind the filehandle, so that it can be used again. */
    if (pr_fsio_lseek(displayfilexfer_fh, 0, SEEK_SET) < 0) {
      pr_log_debug(DEBUG6, "error rewinding DisplayFileTransfer "
        "file '%s': %s", displayfilexfer_fh->fh_path, strerror(errno));
    }

    res = 0;

  } else {
    char *displayfilexfer = get_param_ptr(main_server->conf,
      "DisplayFileTransfer", FALSE);
    if (displayfilexfer) {
      if (pr_display_file(displayfilexfer, session.vwd, R_226) < 0) {
        pr_log_debug(DEBUG6, "unable to display DisplayFileTransfer "
          "file '%s': %s", displayfilexfer, strerror(errno));
      }

      res = 0;
    }
  }

  return res;
}

static int xfer_prio_adjust(void) {
  int res;

  if (xfer_prio_config == 0) {
    return 0;
  }

  errno = 0;
  res = getpriority(PRIO_PROCESS, 0);
  if (res < 0 &&
      errno != 0) {
    pr_trace_msg(trace_channel, 7, "unable to get current process priority: %s",
      strerror(errno));
    return -1;
  }

  /* No need to adjust anything if the current priority already is the
   * configured priority.
   */
  if (res == xfer_prio_config) {
    pr_trace_msg(trace_channel, 10,
      "current process priority matches configured priority");
    return 0;
  }

  xfer_prio_curr = res;

  pr_trace_msg(trace_channel, 10, "adjusting process priority to be %d",
    xfer_prio_config);
  if (xfer_prio_config > 0) {
    res = setpriority(PRIO_PROCESS, 0, xfer_prio_config);

  } else {
    /* Increasing the process priority requires root privs. */
    PRIVS_ROOT
    res = setpriority(PRIO_PROCESS, 0, xfer_prio_config);
    PRIVS_RELINQUISH
  }

  if (res < 0) {
    pr_trace_msg(trace_channel, 0, "error adjusting process priority: %s",
      strerror(errno));
    return -1;
  }

  return 0;
}

static int xfer_prio_restore(void) {
  int res;

  if (xfer_prio_config == 0 ||
      xfer_prio_curr == 0) {
    return 0;
  }

  pr_trace_msg(trace_channel, 10, "restoring process priority to %d",
    xfer_prio_curr);

  PRIVS_ROOT
  res = setpriority(PRIO_PROCESS, 0, xfer_prio_curr);
  PRIVS_RELINQUISH

  if (res < 0) {
    pr_trace_msg(trace_channel, 0, "error restoring process priority: %s",
      strerror(errno));
  }

  xfer_prio_curr = 0;
  return 0;
}

static int xfer_parse_cmdlist(const char *name, config_rec *c,
    char *cmdlist) {
  char *cmd = NULL;
  array_header *cmds = NULL;

  /* Allocate an array_header. */
  cmds = make_array(c->pool, 0, sizeof(char *));

  /* Add each command to the array, checking for invalid commands or
   * duplicates.
   */
  while ((cmd = get_cmd_from_list(&cmdlist)) != NULL) {

    /* Is the given command a valid one for this directive? */
    if (strcasecmp(cmd, C_APPE) != 0 &&
        strcasecmp(cmd, C_RETR) != 0 &&
        strcasecmp(cmd, C_STOR) != 0 &&
        strcasecmp(cmd, C_STOU) != 0) {
      pr_log_debug(DEBUG0, "invalid %s command: %s", name, cmd);
      errno = EINVAL;
      return -1;
    }

    *((char **) push_array(cmds)) = pstrdup(c->pool, cmd);
  }

  /* Terminate the array with a NULL. */
  *((char **) push_array(cmds)) = NULL;

  /* Store the array of commands in the config_rec. */
  c->argv[0] = (void *) cmds->elts;

  return 0;
}

static int transmit_normal(char *buf, long bufsz) {
  long sz = pr_fsio_read(retr_fh, buf, bufsz);

  if (sz < 0) {
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "RETR, user '%s' (UID %lu, GID %lu): "
      "error reading from '%s': %s", session.user,
      (unsigned long) session.uid, (unsigned long) session.gid,
      retr_fh->fh_path, strerror(xerrno));

    errno = xerrno;
    return 0;
  }

  if (sz == 0) {
    return 0;
  }

  return pr_data_xfer(buf, sz);
}

#ifdef HAVE_SENDFILE
static int transmit_sendfile(off_t count, off_t *offset,
    pr_sendfile_t *sentlen) {

  /* We don't use sendfile() if:
   * - We're using bandwidth throttling.
   * - We're transmitting an ASCII file.
   * - We're using RFC2228 data channel protection
   * - We're using MODE Z compression
   * - There's no data left to transmit.
   * - UseSendfile is set to off.
   */
  if (pr_throttle_have_rate() ||
     !(session.xfer.file_size - count) ||
     (session.sf_flags & (SF_ASCII|SF_ASCII_OVERRIDE)) ||
     have_prot || have_zmode ||
     !use_sendfile) {

    if (!xfer_logged_sendfile_decline_msg) {
      if (!use_sendfile) {
        pr_log_debug(DEBUG10, "declining use of sendfile due to UseSendfile "
          "configuration setting");

      } else if (pr_throttle_have_rate()) {
        pr_log_debug(DEBUG10, "declining use of sendfile due to TransferRate "
          "restrictions");
    
      } else if (session.sf_flags & (SF_ASCII|SF_ASCII_OVERRIDE)) {
        pr_log_debug(DEBUG10, "declining use of sendfile for ASCII data");

      } else if (have_prot) {
        pr_log_debug(DEBUG10, "declining use of sendfile due to RFC2228 data "
          "channel protections");

      } else if (have_zmode) {
        pr_log_debug(DEBUG10, "declining use of sendfile due to MODE Z "
          "restrictions");

      } else {
        pr_log_debug(DEBUG10, "declining use of sendfile due to lack of data "
          "to transmit");
      }

      xfer_logged_sendfile_decline_msg = TRUE;
    }

    return 0;
  }

  pr_log_debug(DEBUG10, "using sendfile capability for transmitting data");

 retry:
  *sentlen = pr_data_sendfile(PR_FH_FD(retr_fh), offset,
    session.xfer.file_size - count);

  if (*sentlen == -1) {
    switch (errno) {
      case EAGAIN:
      case EINTR:
        if (XFER_ABORTED) {
          pr_log_pri(PR_LOG_NOTICE, "sendfile transmission aborted");
          return -1;
        }

        /* Interrupted call, or the other side wasn't ready yet. */
        pr_signals_handle();
        goto retry;

      case EPIPE:
      case ECONNRESET:
      case ETIMEDOUT:
      case EHOSTUNREACH:
        /* Other side broke the connection. */
        break;

#ifdef ENOSYS
      case ENOSYS:
#endif /* ENOSYS */

#ifdef EOVERFLOW
      case EOVERFLOW:
#endif /* EOVERFLOW */

      case EINVAL:
        /* No sendfile support, apparently.  Try it the normal way. */
        return 0;
        break;

    default:
      pr_log_pri(PR_LOG_ERR, "error using sendfile(): [%d] %s", errno,
        strerror(errno));
      return -1;
    }
  }

  return 1;
}
#endif /* HAVE_SENDFILE */

/* Note: the count and offset arguments are only for the benefit of
 * transmit_sendfile(), if sendfile support is enabled.  The transmit_normal()
 * function only needs/uses buf and bufsz.
 */
static long transmit_data(off_t count, off_t *offset, char *buf, long bufsz) {
  long res;

#ifdef TCP_CORK
  int on = 1;
  socklen_t len = sizeof(int);

# ifdef SOL_TCP
  int tcp_level = SOL_TCP;
# else
  int tcp_level = IPPROTO_TCP;
# endif /* SOL_TCP */
#endif /* TCP_CORK */

#ifdef HAVE_SENDFILE
  pr_sendfile_t sentlen;
  int ret;
#endif /* HAVE_SENDFILE */

#ifdef TCP_CORK
  /* Note: TCP_CORK is a Linuxism, introduced with the 2.4 kernel.  It
   * has effects similar to BSD's TCP_NOPUSH option.
   */
  if (setsockopt(PR_NETIO_FD(session.d->outstrm), tcp_level, TCP_CORK, &on,
      len) < 0)
    pr_log_pri(PR_LOG_NOTICE, "error setting TCP_CORK: %s", strerror(errno));
#endif /* TCP_CORK */

#ifdef HAVE_SENDFILE
  ret = transmit_sendfile(count, offset, &sentlen);
  if (ret > 0) {
    /* sendfile() was used, so return the value of sentlen. */
    res = (long) sentlen;

  } else if (ret == 0) {
    /* sendfile() should not be used for some reason, fallback to using
     * normal data transmission methods.
     */
    res = transmit_normal(buf, bufsz);

  } else {
    /* There was an error with sendfile(); do NOT attempt to re-send the
     * data using normal data transmission methods, unless the cause
     * of the error is one of an accepted few cases.
     */
# ifdef EOVERFLOW
    pr_log_debug(DEBUG10, "use of sendfile(2) failed due to %s (%d), "
      "falling back to normal data transmission", strerror(errno),
      errno);
    res = transmit_normal(buf, bufsz);
# else
    errno = EIO;
    res = -1;
# endif
  }
#else
  res = transmit_normal(buf, bufsz);
#endif /* HAVE_SENDFILE */

#ifdef TCP_CORK
  if (session.d) {
    /* The session.d struct can become null after transmit_normal() if the
     * client aborts the transfer, thus we need to check for this.
     */
    on = 0;
    if (setsockopt(PR_NETIO_FD(session.d->outstrm), tcp_level, TCP_CORK, &on,
        len) < 0) {
      pr_log_pri(PR_LOG_NOTICE, "error setting TCP_CORK: %s", strerror(errno));
    }
  }
#endif /* TCP_CORK */

  return res;
}

static void stor_chown(void) {
  struct stat st;
  char *xfer_path = NULL;

  if (session.xfer.xfer_type == STOR_HIDDEN)
    xfer_path = session.xfer.path_hidden;
  else
    xfer_path = session.xfer.path;

  /* session.fsgid defaults to -1, so chown(2) won't chgrp unless specifically
   * requested via GroupOwner.
   */
  if ((session.fsuid != (uid_t) -1) && xfer_path) {
    int err = 0, iserr = 0;

    PRIVS_ROOT
    if (pr_fsio_chown(xfer_path, session.fsuid, session.fsgid) == -1) {
      iserr++;
      err = errno;
    }
    PRIVS_RELINQUISH

    if (iserr)
      pr_log_pri(PR_LOG_WARNING, "chown(%s) as root failed: %s", xfer_path,
        strerror(err));

    else {

      if (session.fsgid != (gid_t) -1)
        pr_log_debug(DEBUG2, "root chown(%s) to uid %lu, gid %lu successful",
          xfer_path, (unsigned long) session.fsuid,
          (unsigned long) session.fsgid);

      else
        pr_log_debug(DEBUG2, "root chown(%s) to uid %lu successful", xfer_path,
          (unsigned long) session.fsuid);

      pr_fs_clear_cache();
      pr_fsio_stat(xfer_path, &st);

      /* The chmod happens after the chown because chown will remove
       * the S{U,G}ID bits on some files (namely, directories); the subsequent
       * chmod is used to restore those dropped bits.  This makes it
       * necessary to use root privs when doing the chmod as well (at least
       * in the case of chown'ing the file via root privs) in order to ensure
       * that the mode can be set (a file might be being "given away", and if
       * root privs aren't used, the chmod() will fail because the old owner/
       * session user doesn't have the necessary privileges to do so).
       */
      iserr = 0;
      PRIVS_ROOT
      if (pr_fsio_chmod(xfer_path, st.st_mode) < 0)
        iserr++;
      PRIVS_RELINQUISH

      if (iserr)
        pr_log_debug(DEBUG0, "root chmod(%s) to %04o failed: %s", xfer_path,
          (unsigned int) st.st_mode, strerror(errno));
      else
        pr_log_debug(DEBUG2, "root chmod(%s) to %04o successful", xfer_path,
          (unsigned int) st.st_mode);
    }

  } else if ((session.fsgid != (gid_t) -1) && xfer_path) {
    register unsigned int i;
    int res, use_root_privs = TRUE;

    /* Check if session.fsgid is in session.gids.  If not, use root privs. */
    for (i = 0; i < session.gids->nelts; i++) {
      gid_t *group_ids = session.gids->elts;

      if (group_ids[i] == session.fsgid) {
        use_root_privs = FALSE;
        break;
      }
    }

    if (use_root_privs) {
      PRIVS_ROOT
    }

    res = pr_fsio_chown(xfer_path, (uid_t) -1, session.fsgid);

    if (use_root_privs) {
      PRIVS_RELINQUISH
    }

    if (res == -1)
      pr_log_pri(PR_LOG_WARNING, "%schown(%s) failed: %s",
        use_root_privs ? "root " : "", xfer_path, strerror(errno));

    else {
      pr_log_debug(DEBUG2, "%schown(%s) to gid %lu successful",
        use_root_privs ? "root " : "", xfer_path,
        (unsigned long) session.fsgid);

      pr_fs_clear_cache();
      pr_fsio_stat(xfer_path, &st);

      if (use_root_privs) {
        PRIVS_ROOT
      }

      res = pr_fsio_chmod(xfer_path, st.st_mode);

      if (use_root_privs) {
        PRIVS_RELINQUISH
      }

      if (res < 0)
        pr_log_debug(DEBUG0, "%schmod(%s) to %04o failed: %s",
          use_root_privs ? "root " : "", xfer_path, (unsigned int) st.st_mode,
          strerror(errno));
    }
  }
}

static void retr_abort(void) {
  /* Isn't necessary to send anything here, just cleanup */

  if (retr_fh) {
    pr_fsio_close(retr_fh);
    retr_fh = NULL;
  }

  _log_transfer('o', 'i');
}

static void retr_complete(void) {
  pr_fsio_close(retr_fh);
  retr_fh = NULL;
}

static void stor_abort(void) {
  unsigned char *delete_stores = NULL;

  if (stor_fh) {
    if (pr_fsio_close(stor_fh) < 0) {
      int xerrno = errno;

      pr_log_pri(PR_LOG_NOTICE, "notice: error closing '%s': %s",
        stor_fh->fh_path, strerror(xerrno));
 
      errno = xerrno;
    }

    stor_fh = NULL;
  }

  if (session.xfer.xfer_type == STOR_HIDDEN) {
    delete_stores = get_param_ptr(CURRENT_CONF, "DeleteAbortedStores", FALSE);
    if (delete_stores != NULL &&
        *delete_stores == TRUE) {
      /* If a hidden store was aborted, remove only hidden file, not real
       * one.
       */
      if (session.xfer.path_hidden) {
        pr_log_debug(DEBUG5, "removing aborted HiddenStores file '%s'",
          session.xfer.path_hidden);
        pr_fsio_unlink(session.xfer.path_hidden);
      }
    }

  } else if (session.xfer.path) {
    delete_stores = get_param_ptr(CURRENT_CONF, "DeleteAbortedStores", FALSE);
    if (delete_stores != NULL &&
        *delete_stores == TRUE) {
      pr_log_debug(DEBUG5, "removing aborted file '%s'", session.xfer.path);
      pr_fsio_unlink(session.xfer.path);
    }
  }

  _log_transfer('i', 'i');
}

static int stor_complete(void) {
  int res = 0;

  if (pr_fsio_close(stor_fh) < 0) {
    int xerrno = errno;

    pr_log_pri(PR_LOG_NOTICE, "notice: error closing '%s': %s",
      stor_fh->fh_path, strerror(xerrno));

    /* We will unlink failed writes, but only if it's a HiddenStores file.
     * Other files will need to be explicitly deleted/removed by the client.
     */
    if (session.xfer.xfer_type == STOR_HIDDEN) {
      if (session.xfer.path_hidden) {
        pr_log_debug(DEBUG5, "failed to close HiddenStores file '%s', removing",
          session.xfer.path_hidden);
        pr_fsio_unlink(session.xfer.path_hidden);
      }
    }

    errno = xerrno;
    res = -1;
  }

  stor_fh = NULL;
  return res;
}

static int get_hidden_store_path(cmd_rec *cmd, char *path, char *prefix) {
  char *c = NULL, *hidden_path;
  int dotcount = 0, foundslash = 0, basenamestart = 0, maxlen;

  /* We have to also figure out the temporary hidden file name for receiving
   * this transfer.  Length is +(N+1) due to prepended prefix and "." at end.
   */

  /* Figure out where the basename starts */
  for (c = path; *c; ++c) {

    if (*c == '/') {
      foundslash = 1;
      basenamestart = dotcount = 0;

    } else if (*c == '.') {
      ++dotcount;

      /* Keep track of leading dots, ... is normal, . and .. are special.
       * So if we exceed ".." it becomes a normal file. Retroactively consider
       * this the possible start of the basename.
       */
      if ((dotcount > 2) &&
          !basenamestart)
        basenamestart = ((unsigned long) c - (unsigned long) path) - dotcount;

    } else {

      /* We found a nonslash, nondot character; if this is the first time
       * we found one since the last slash, remember this as the possible
       * start of the basename.
       */
      if (!basenamestart)
        basenamestart = ((unsigned long) c - (unsigned long) path) - dotcount;
    }
  }

  if (!basenamestart) {
    session.xfer.xfer_type = STOR_DEFAULT;

    pr_log_debug(DEBUG6, "could not determine HiddenStores path for '%s'",
      path);

    /* This probably shouldn't happen */
    pr_response_add_err(R_451, _("%s: Bad file name"), path);
    return -1;
  }

  /* Add N+1 for the prefix and "." characters, plus one for a terminating
   * NUL.
   */
  maxlen = strlen(path) + strlen(prefix) + 2;

  if (maxlen > PR_TUNABLE_PATH_MAX) {
    session.xfer.xfer_type = STOR_DEFAULT;

    pr_log_pri(PR_LOG_NOTICE, "making path '%s' a hidden path exceeds max "
      "path length (%u)", path, PR_TUNABLE_PATH_MAX);

    /* This probably shouldn't happen */
    pr_response_add_err(R_451, _("%s: File name too long"), path);
    return -1;
  }

  if (pr_table_add(cmd->notes, "mod_xfer.store-hidden-path", NULL, 0) < 0) {
    pr_log_pri(PR_LOG_NOTICE,
      "notice: error adding 'mod_xfer.store-hidden-path': %s",
      strerror(errno));
  }

  if (!foundslash) {

    /* Simple local file name */
    hidden_path = pstrcat(cmd->tmp_pool, prefix, path, ".", NULL);

    pr_log_pri(PR_LOG_DEBUG, "HiddenStore: local path, will rename %s to %s",
      hidden_path, path);

  } else {

    /* Complex relative path or absolute path */
    hidden_path = pstrndup(cmd->pool, path, maxlen);
    hidden_path[basenamestart] = '\0';

    hidden_path = pstrcat(cmd->pool, hidden_path, prefix,
      path + basenamestart, ".", NULL);

    pr_log_pri(PR_LOG_DEBUG, "HiddenStore: complex path, will rename %s to %s",
      hidden_path, path);
  }

  if (file_mode(hidden_path)) {
    session.xfer.xfer_type = STOR_DEFAULT;

    pr_log_debug(DEBUG3, "HiddenStore path '%s' already exists",
      hidden_path);

    pr_response_add_err(R_550, _("%s: Temporary hidden file %s already exists"),
      cmd->arg, hidden_path);
    return -1;
  }

  if (pr_table_set(cmd->notes, "mod_xfer.store-hidden-path",
      hidden_path, 0) < 0) {
    pr_log_pri(PR_LOG_NOTICE,
      "notice: error setting 'mod_xfer.store-hidden-path': %s",
      strerror(errno));
  }

  session.xfer.xfer_type = STOR_HIDDEN;
  return 0;
}

MODRET xfer_post_prot(cmd_rec *cmd) {
  CHECK_CMD_ARGS(cmd, 2);

  if (strcmp(cmd->argv[1], "C") != 0)
    have_prot = TRUE;
  else
    have_prot = FALSE;

  return PR_DECLINED(cmd);
}

MODRET xfer_post_mode(cmd_rec *cmd) {
  CHECK_CMD_ARGS(cmd, 2);

  if (strcmp(cmd->argv[1], "Z") == 0)
    have_zmode = TRUE;
  else
    have_zmode = FALSE;

  return PR_DECLINED(cmd);
}

/* This is a PRE_CMD handler that checks security, etc, and places the full
 * filename to receive in cmd->notes, under the key 'mod_xfer.store-path'.
 * Note that we CANNOT use cmd->tmp_pool for this, as tmp_pool only lasts for
 * the duration of this function.
 */
MODRET xfer_pre_stor(cmd_rec *cmd) {
  char *path;
  mode_t fmode;
  unsigned char *allow_overwrite = NULL, *allow_restart = NULL;
  config_rec *c;

  if (cmd->argc < 2) {
    pr_response_add_err(R_500, _("'%s' not understood"), get_full_cmd(cmd));
    errno = EINVAL;
    return PR_ERROR(cmd);
  }

  path = dir_best_path(cmd->tmp_pool,
    pr_fs_decode_path(cmd->tmp_pool, cmd->arg));

  if (!path ||
      !dir_check(cmd->tmp_pool, cmd, cmd->group, path, NULL)) {
    int xerrno = errno;

    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(xerrno));

    errno = xerrno;
    return PR_ERROR(cmd);
  }

  if (xfer_check_limit(cmd) < 0) {
    pr_response_add_err(R_451, _("%s: Too many transfers"), cmd->arg);
    errno = EPERM;
    return PR_ERROR(cmd);
  }

  fmode = file_mode(path);

  allow_overwrite = get_param_ptr(CURRENT_CONF, "AllowOverwrite", FALSE);

  if (fmode && (session.xfer.xfer_type != STOR_APPEND) &&
      (!allow_overwrite || *allow_overwrite == FALSE)) {
    pr_log_debug(DEBUG6, "AllowOverwrite denied permission for %s", cmd->arg);
    pr_response_add_err(R_550, _("%s: Overwrite permission denied"), cmd->arg);
    errno = EACCES;
    return PR_ERROR(cmd);
  }

  if (fmode &&
      !S_ISREG(fmode) &&
      !S_ISFIFO(fmode)) {

    /* Make an exception for the non-regular /dev/null file.  This will allow
     * network link testing by uploading as much data as necessary directly
     * to /dev/null.
     *
     * On Linux, allow another exception for /dev/full; this is useful for
     * tests which want to simulate running out-of-space scenarios.
     */
    if (strcasecmp(path, "/dev/null") != 0
#ifdef LINUX
        && strcasecmp(path, "/dev/full") != 0
#endif
       ) {
      pr_response_add_err(R_550, _("%s: Not a regular file"), cmd->arg);
      errno = EPERM;
      return PR_ERROR(cmd);
    }
  }

  /* If restarting, check permissions on this directory, if
   * AllowStoreRestart is set, permit it
   */
  allow_restart = get_param_ptr(CURRENT_CONF, "AllowStoreRestart", FALSE);

  if (fmode &&
     (session.restart_pos || (session.xfer.xfer_type == STOR_APPEND)) &&
     (!allow_restart || *allow_restart == FALSE)) {

    pr_response_add_err(R_451, _("%s: Append/Restart not permitted, try again"),
      cmd->arg);
    session.restart_pos = 0L;
    session.xfer.xfer_type = STOR_DEFAULT;
    errno = EPERM;
    return PR_ERROR(cmd);
  }

  /* Otherwise everthing is good */
  if (pr_table_add(cmd->notes, "mod_xfer.store-path",
      pstrdup(cmd->pool, path), 0) < 0)
    pr_log_pri(PR_LOG_NOTICE, "notice: error adding 'mod_xfer.store-path': %s",
      strerror(errno));

  c = find_config(CURRENT_CONF, CONF_PARAM, "HiddenStores", FALSE);
  if (c &&
      *((int *) c->argv[0]) == TRUE) {
    char *prefix = c->argv[1];

    /* If we're using HiddenStores, then REST won't work. */
    if (session.restart_pos) {
      pr_response_add_err(R_501,
        _("REST not compatible with server configuration"));
      errno = EINVAL;
      return PR_ERROR(cmd);
    }

    if (get_hidden_store_path(cmd, path, prefix) < 0) {
      return PR_ERROR(cmd);
    }
  }

  (void) xfer_prio_adjust();
  return PR_HANDLED(cmd);
}

/* xfer_pre_stou() is a PRE_CMD handler that changes the uploaded filename
 * to a unique one, after making the requisite security and authorization
 * checks.
 */
MODRET xfer_pre_stou(cmd_rec *cmd) {
  config_rec *c = NULL;
  char *prefix = "ftp", *filename = NULL;
  int tmpfd;
  mode_t mode;
  unsigned char *allow_overwrite = NULL;

  session.xfer.xfer_type = STOR_DEFAULT;

  /* Some FTP clients are "broken" in that they will send a filename
   * along with STOU.  Technically this violates RFC959, but for now, just
   * ignore that filename.  Stupid client implementors.
   */

  if (cmd->argc > 2) {
    pr_response_add_err(R_500, _("'%s' not understood"), get_full_cmd(cmd));
    return PR_ERROR(cmd);
  }

  if (xfer_check_limit(cmd) < 0) {
    pr_response_add_err(R_451, _("%s: Too many transfers"), cmd->arg);
    return PR_ERROR(cmd);
  }

  /* Watch for STOU preceded by REST, which makes no sense.
   *
   *   REST: session.restart_pos > 0
   */
  if (session.restart_pos) {
    pr_response_add_err(R_550, _("STOU incompatible with REST"));
    return PR_ERROR(cmd);
  }

  /* Generate the filename to be stored, depending on the configured
   * unique filename prefix.
   */
  c = find_config(CURRENT_CONF, CONF_PARAM, "StoreUniquePrefix", FALSE);
  if (c != NULL)
    prefix = c->argv[0];

  /* Now, construct the unique filename using the cmd_rec's pool, the
   * prefix, and mkstemp().
   */
  filename = pstrcat(cmd->pool, prefix, "XXXXXX", NULL);

  tmpfd = mkstemp(filename);
  if (tmpfd < 0) {
    pr_log_pri(PR_LOG_ERR, "error: unable to use mkstemp(): %s",
      strerror(errno));

    /* If we can't guarantee a unique filename, refuse the command. */
    pr_response_add_err(R_450, _("%s: unable to generate unique filename"),
      cmd->argv[0]);
    return PR_ERROR(cmd);

  } else {
    cmd->arg = filename;

    /* Close the unique file.  This introduces a small race condition
     * between the time this function returns, and the STOU CMD handler
     * opens the unique file, but this may have to do, as closing that
     * race would involve some major restructuring.
     */
    close(tmpfd);
  }

  /* It's OK to reuse the char * pointer for filename. */
  filename = dir_best_path(cmd->tmp_pool, cmd->arg);

  if (!filename ||
      !dir_check(cmd->tmp_pool, cmd, cmd->group, filename, NULL)) {
    int xerrno = errno;

    /* Do not forget to delete the file created by mkstemp(3) if there is
     * an error.
     */
    (void) pr_fsio_unlink(cmd->arg);

    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(xerrno));
    errno = xerrno;
    return PR_ERROR(cmd);
  }

  mode = file_mode(filename);

  /* Note: this case should never happen: how one can be appending to
   * a supposedly unique filename?  Should probably be removed...
   */
  allow_overwrite = get_param_ptr(CURRENT_CONF, "AllowOverwrite", FALSE);

  if (mode && session.xfer.xfer_type != STOR_APPEND &&
      (!allow_overwrite || *allow_overwrite == FALSE)) {
    pr_log_debug(DEBUG6, "AllowOverwrite denied permission for %s", cmd->arg);
    pr_response_add_err(R_550, _("%s: Overwrite permission denied"), cmd->arg);
    return PR_ERROR(cmd);
  }

  /* Not likely to _not_ be a regular file, but just to be certain... */
  if (mode &&
      !S_ISREG(mode)) {
    (void) pr_fsio_unlink(cmd->arg);
    pr_response_add_err(R_550, _("%s: Not a regular file"), cmd->arg);
    return PR_ERROR(cmd);
  }

  /* Otherwise everthing is good */
  if (pr_table_add(cmd->notes, "mod_xfer.store-path",
      pstrdup(cmd->pool, filename), 0) < 0)
    pr_log_pri(PR_LOG_NOTICE, "notice: error adding 'mod_xfer.store-path': %s",
      strerror(errno));

  session.xfer.xfer_type = STOR_UNIQUE;

  (void) xfer_prio_adjust();
  return PR_HANDLED(cmd);
}

/* xfer_post_stou() is a POST_CMD handler that changes the mode of the
 * STOU file from 0600, which is what mkstemp() makes it, to 0666,
 * the default for files uploaded via STOR.  This is to prevent users
 * from being surprised.
 */
MODRET xfer_post_stou(cmd_rec *cmd) {

  /* This is the same mode as used in src/fs.c.  Should probably be
   * available as a macro.
   */
  mode_t mode = 0666;

  if (pr_fsio_chmod(cmd->arg, mode) < 0) {

    /* Not much to do but log the error. */
    pr_log_pri(PR_LOG_ERR, "error: unable to chmod '%s': %s", cmd->arg,
      strerror(errno));
  }

  return PR_DECLINED(cmd);
}

/* xfer_pre_appe() is the PRE_CMD handler for the APPE command, which
 * simply sets xfer_type to STOR_APPEND and calls xfer_pre_stor().
 */
MODRET xfer_pre_appe(cmd_rec *cmd) {
  session.xfer.xfer_type = STOR_DEFAULT;

  if (xfer_check_limit(cmd) < 0) {
    pr_response_add_err(R_451, _("%s: Too many transfers"), cmd->arg);
    errno = EPERM;
    return PR_ERROR(cmd);
  }

  session.xfer.xfer_type = STOR_APPEND;
  return xfer_pre_stor(cmd);
}

MODRET xfer_stor(cmd_rec *cmd) {
  char *path;
  char *lbuf;
  int bufsz, len, ferrno = 0, res;
  off_t nbytes_stored, nbytes_max_store = 0;
  unsigned char have_limit = FALSE;
  struct stat st;
  off_t curr_pos = 0;

  /* Prepare for any potential throttling. */
  pr_throttle_init(cmd);

  session.xfer.path = pr_table_get(cmd->notes, "mod_xfer.store-path", NULL);
  session.xfer.path_hidden = pr_table_get(cmd->notes,
    "mod_xfer.store-hidden-path", NULL);

  path = session.xfer.path;

  res = pr_filter_allow_path(CURRENT_CONF, path);
  switch (res) {
    case 0:
      break;

    case PR_FILTER_ERR_FAILS_ALLOW_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathAllowFilter", cmd->argv[0],
        path);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd);

    case PR_FILTER_ERR_FAILS_DENY_FILTER:
      pr_log_debug(DEBUG2, "'%s %s' denied by PathDenyFilter", cmd->argv[0],
        path);
      pr_response_add_err(R_550, _("%s: Forbidden filename"), cmd->arg);
      return PR_ERROR(cmd);
  }

  /* Make sure the proper current working directory is set in the FSIO
   * layer, so that the proper FS can be used for the open().
   */
  pr_fs_setcwd(pr_fs_getcwd());

  if (session.xfer.xfer_type == STOR_HIDDEN) {
    stor_fh = pr_fsio_open(session.xfer.path_hidden,
      O_WRONLY|(session.restart_pos ? 0 : O_CREAT|O_EXCL));
    if (stor_fh == NULL) {
      ferrno = errno;

      (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
        "error opening '%s': %s", cmd->argv[0], session.user,
        (unsigned long) session.uid, (unsigned long) session.gid,
        session.xfer.path_hidden, strerror(ferrno));
    }

  } else if (session.xfer.xfer_type == STOR_APPEND) {
    stor_fh = pr_fsio_open(session.xfer.path, O_CREAT|O_WRONLY);

    if (stor_fh) {
      if (pr_fsio_lseek(stor_fh, 0, SEEK_END) == (off_t) -1) {
        pr_log_debug(DEBUG4, "unable to seek to end of '%s' for appending: %s",
          cmd->arg, strerror(errno));
        (void) pr_fsio_close(stor_fh);
        stor_fh = NULL;
      }

    } else {
      ferrno = errno;

      (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
        "error opening '%s': %s", cmd->argv[0], session.user,
        (unsigned long) session.uid, (unsigned long) session.gid,
        session.xfer.path, strerror(ferrno));
    }

  } else {
    /* Normal session */
    stor_fh = pr_fsio_open(path,
        O_WRONLY|(session.restart_pos ? 0 : O_TRUNC|O_CREAT));
    if (stor_fh == NULL) {
      ferrno = errno;

      (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
        "error opening '%s': %s", cmd->argv[0], session.user,
        (unsigned long) session.uid, (unsigned long) session.gid, path,
        strerror(ferrno));
    }
  }

  if (stor_fh &&
      session.restart_pos) {
    int xerrno = 0;

    if (pr_fsio_lseek(stor_fh, session.restart_pos, SEEK_SET) == -1) {
      pr_log_debug(DEBUG4, "unable to seek to position %" PR_LU " of '%s': %s",
        (pr_off_t) session.restart_pos, cmd->arg, strerror(errno));
      xerrno = errno;

    } else if (pr_fsio_stat(path, &st) == -1) {
      pr_log_debug(DEBUG4, "unable to stat '%s': %s", cmd->arg,
        strerror(errno));
      xerrno = errno;
    }

    if (xerrno) {
      (void) pr_fsio_close(stor_fh);
      errno = xerrno;
      stor_fh = NULL;
    }

    /* Make sure that the requested offset is valid (within the size of the
     * file being resumed).
     */
    if (stor_fh &&
        session.restart_pos > st.st_size) {
      pr_response_add_err(R_554, _("%s: invalid REST argument"), cmd->arg);
      (void) pr_fsio_close(stor_fh);
      stor_fh = NULL;
      return PR_ERROR(cmd);
    }

    curr_pos = session.restart_pos;
    session.restart_pos = 0L;
  }

  if (!stor_fh) {
    pr_log_debug(DEBUG4, "unable to open '%s' for writing: %s", cmd->arg,
      strerror(ferrno));
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(ferrno));
    return PR_ERROR(cmd);
  }

  /* Perform the actual transfer now */
  pr_data_init(cmd->arg, PR_NETIO_IO_RD);

  /* Note that we have to re-populate the session.xfer variables here,
   * AFTER the pr_data_init() call.  pr_data_init() ensures that there is
   * no leftover information in session.xfer, as from aborted tranfers.
   */
  session.xfer.path = pr_table_get(cmd->notes, "mod_xfer.store-path", NULL);
  session.xfer.path_hidden = pr_table_get(cmd->notes,
    "mod_xfer.store-hidden-path", NULL);
  session.xfer.file_size = curr_pos;

  /* First, make sure the uploaded file has the requested ownership. */
  stor_chown();

  if (pr_data_open(cmd->arg, NULL, PR_NETIO_IO_RD, 0) < 0) {
    stor_abort();
    pr_data_abort(0, TRUE);
    return PR_ERROR(cmd);
  }

  /* Initialize the number of bytes stored */
  nbytes_stored = 0;

  /* Retrieve the number of bytes to store, maximum, if present.
   * This check is needed during the pr_data_xfer() loop, below, because
   * the size of the file being uploaded isn't known in advance
   */
  if ((nbytes_max_store = find_max_nbytes("MaxStoreFileSize")) == 0UL)
    have_limit = FALSE;
  else
    have_limit = TRUE;

  /* Check the MaxStoreFileSize, and abort now if zero. */
  if (have_limit &&
      nbytes_max_store == 0) {

    pr_log_pri(PR_LOG_INFO, "MaxStoreFileSize (%" PR_LU " %s) reached: "
      "aborting transfer of '%s'", (pr_off_t) nbytes_max_store,
      nbytes_max_store != 1 ? "bytes" : "byte", path);

    /* Abort the transfer. */
    stor_abort();

    /* Set errno to EDQOUT (or the most appropriate alternative). */
#if defined(EDQUOT)
    pr_data_abort(EDQUOT, FALSE);
#elif defined(EFBIG)
    pr_data_abort(EFBIG, FALSE);
#else
    pr_data_abort(EPERM, FALSE);
#endif
    return PR_ERROR(cmd);
  }

  bufsz = (main_server->tcp_rcvbuf_len > 0 ?  main_server->tcp_rcvbuf_len :
    pr_config_get_xfer_bufsz());
  lbuf = (char *) palloc(cmd->tmp_pool, bufsz);

  while ((len = pr_data_xfer(lbuf, bufsz)) > 0) {
    pr_signals_handle();

    if (XFER_ABORTED)
      break;

    nbytes_stored += len;

    /* Double-check the current number of bytes stored against the
     * MaxStoreFileSize, if configured.
     */
    if (have_limit &&
        nbytes_stored > nbytes_max_store) {

      pr_log_pri(PR_LOG_INFO, "MaxStoreFileSize (%" PR_LU " bytes) reached: "
        "aborting transfer of '%s'", (pr_off_t) nbytes_max_store, path);

      /* Abort the transfer. */
      stor_abort();

    /* Set errno to EDQOUT (or the most appropriate alternative). */
#if defined(EDQUOT)
      pr_data_abort(EDQUOT, FALSE);
#elif defined(EFBIG)
      pr_data_abort(EFBIG, FALSE);
#else
      pr_data_abort(EPERM, FALSE);
#endif
      return PR_ERROR(cmd);
    }

    res = pr_fsio_write(stor_fh, lbuf, len);
    if (res != len) {
      int xerrno = EIO;

      if (res < 0)
        xerrno = errno;

      (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
        "error writing to '%s': %s", cmd->argv[0], session.user,
        (unsigned long) session.uid, (unsigned long) session.gid,
        stor_fh->fh_path, strerror(xerrno));

      stor_abort();
      pr_data_abort(xerrno, FALSE);
      return PR_ERROR(cmd);
    }

    /* If no throttling is configured, this does nothing. */
    pr_throttle_pause(nbytes_stored, FALSE);
  }

  if (XFER_ABORTED) {
    stor_abort();
    pr_data_abort(0, 0);
    return PR_ERROR(cmd);

  } else if (len < 0) {

    /* default abort errno, in case session.d et al has already gone away */
    int xerrno = ECONNABORTED;

    stor_abort();

    if (session.d != NULL &&
        session.d->instrm != NULL) {
      xerrno = PR_NETIO_ERRNO(session.d->instrm);
    }

    pr_data_abort(xerrno, FALSE);
    return PR_ERROR(cmd);

  } else {

    /* If no throttling is configured, this does nothing. */
    pr_throttle_pause(nbytes_stored, TRUE);

    if (stor_complete() < 0) {
      /* Check errno for EDQOUT (or the most appropriate alternative).
       * (I hate the fact that FTP has a special response code just for
       * this, and that clients actually expect it.  Special cases are
       * stupid.)
       */
#if defined(EDQUOT)
      if (errno == EDQUOT) {
        pr_response_add_err(R_552, "%s: %s", cmd->arg, strerror(errno));
        return PR_ERROR(cmd);
      }
#elif defined(EFBIG)
      if (errno == EFBIG) {
        pr_response_add_err(R_552, "%s: %s", cmd->arg, strerror(errno));
        return PR_ERROR(cmd);
      }
#endif

      pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
      return PR_ERROR(cmd);
    }

    if (session.xfer.path &&
        session.xfer.path_hidden) {
      if (pr_fsio_rename(session.xfer.path_hidden, session.xfer.path) != 0) {

        /* This should only fail on a race condition with a chmod/chown
         * or if STOR_APPEND is on and the permissions are squirrely.
         * The poor user will have to re-upload, but we've got more important
         * problems to worry about and this failure should be fairly rare.
         */
        pr_log_pri(PR_LOG_WARNING, "Rename of %s to %s failed: %s.",
          session.xfer.path_hidden, session.xfer.path, strerror(errno));

        pr_response_add_err(R_550, _("%s: Rename of hidden file %s failed: %s"),
          session.xfer.path, session.xfer.path_hidden, strerror(errno));

        pr_fsio_unlink(session.xfer.path_hidden);
        return PR_ERROR(cmd);
      }
    }

    if (xfer_displayfile() < 0) {
      pr_data_close(FALSE);

    } else {
      pr_data_close(TRUE);
    } 
  }

  return PR_HANDLED(cmd);
}

MODRET xfer_rest(cmd_rec *cmd) {
  off_t pos;
  char *endp = NULL;

  if (cmd->argc != 2) {
    pr_response_add_err(R_500, _("'%s' not understood"), get_full_cmd(cmd));
    return PR_ERROR(cmd);
  }

  /* Don't allow negative numbers.  strtoul()/strtoull() will silently
   * handle them.
   */
  if (*cmd->argv[1] == '-') {
    pr_response_add_err(R_501,
      _("REST requires a value greater than or equal to 0"));
    return PR_ERROR(cmd);
  }

#ifdef HAVE_STRTOULL
  pos = strtoull(cmd->argv[1], &endp, 10);
#else
  pos = strtoul(cmd->argv[1], &endp, 10);
#endif /* HAVE_STRTOULL */

  if (endp &&
      *endp) {
    pr_response_add_err(R_501,
      _("REST requires a value greater than or equal to 0"));
    return PR_ERROR(cmd);
  }

  /* Refuse the command if we're in ASCII mode, and the restart position
   * is anything other than zero.
   *
   * Ideally, we would refuse the REST command when in ASCII mode regardless
   * of position.  However, some (IMHO, stupid) clients "test" the FTP
   * server by sending "REST 0" to see if the server supports REST, without
   * regard to the transfer type.  This, then, is a hack to handle such
   * clients.
   */
  if ((session.sf_flags & SF_ASCII) &&
      pos != 0) {
    pr_log_debug(DEBUG5, "%s not allowed in ASCII mode", cmd->argv[0]);
    pr_response_add_err(R_501,
      _("%s: Resuming transfers not allowed in ASCII mode"), cmd->argv[0]);
    return PR_ERROR(cmd);
  } 

  session.restart_pos = pos;

  pr_response_add(R_350, _("Restarting at %" PR_LU
    ". Send STORE or RETRIEVE to initiate transfer"), (pr_off_t) pos);
  return PR_HANDLED(cmd);
}

/* This is a PRE_CMD handler that checks security, etc, and places the full
 * filename to send in cmd->notes (note that we CANNOT use cmd->tmp_pool
 * for this, as tmp_pool only lasts for the duration of this function).
 */
MODRET xfer_pre_retr(cmd_rec *cmd) {
  char *dir = NULL;
  mode_t fmode;
  unsigned char *allow_restart = NULL;

  xfer_logged_sendfile_decline_msg = FALSE;

  if (cmd->argc < 2) {
    pr_response_add_err(R_500, _("'%s' not understood"), get_full_cmd(cmd));
    errno = EINVAL;
    return PR_ERROR(cmd);
  }

  dir = dir_realpath(cmd->tmp_pool,
    pr_fs_decode_path(cmd->tmp_pool, cmd->arg));

  if (!dir ||
      !dir_check(cmd->tmp_pool, cmd, cmd->group, dir, NULL)) {
    int xerrno = errno;

    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(xerrno));

    errno = xerrno;
    return PR_ERROR(cmd);
  }

  if (xfer_check_limit(cmd) < 0) {
    pr_response_add_err(R_451, _("%s: Too many transfers"), cmd->arg);
    errno = EPERM;
    return PR_ERROR(cmd);
  }

  fmode = file_mode(dir);
  if (fmode == 0) {
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
    return PR_ERROR(cmd);
  }

  if (!S_ISREG(fmode)
#ifdef S_ISFIFO
      && !S_ISFIFO(fmode)
#endif
     ) {
    pr_response_add_err(R_550, _("%s: Not a regular file"), cmd->arg);
    errno = EPERM;
    return PR_ERROR(cmd);
  }

  /* If restart is on, check to see if AllowRestartRetrieve is off, in
   * which case we disallow the transfer and clear restart_pos.
   */
  allow_restart = get_param_ptr(CURRENT_CONF, "AllowRetrieveRestart", FALSE);

  if (session.restart_pos &&
     (allow_restart && *allow_restart == FALSE)) {
    pr_response_add_err(R_451, _("%s: Restart not permitted, try again"),
      cmd->arg);
    session.restart_pos = 0L;
    errno = EPERM;
    return PR_ERROR(cmd);
  }

  /* Otherwise everthing is good */
  if (pr_table_add(cmd->notes, "mod_xfer.retr-path",
      pstrdup(cmd->pool, dir), 0) < 0)
    pr_log_pri(PR_LOG_NOTICE, "notice: error adding 'mod_xfer.retr-path': %s",
      strerror(errno));

  (void) xfer_prio_adjust();
  return PR_HANDLED(cmd);
}

MODRET xfer_retr(cmd_rec *cmd) {
  char *dir = NULL, *lbuf;
  struct stat st;
  off_t nbytes_max_retrieve = 0;
  unsigned char have_limit = FALSE;
  long bufsz, len = 0;
  off_t curr_pos = 0, nbytes_sent = 0, cnt_steps = 0, cnt_next = 0;

  /* Prepare for any potential throttling. */
  pr_throttle_init(cmd);

  dir = pr_table_get(cmd->notes, "mod_xfer.retr-path", NULL);

  retr_fh = pr_fsio_open(dir, O_RDONLY);
  if (retr_fh == NULL) {
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
      "error opening '%s': %s", cmd->argv[0], session.user,
      (unsigned long) session.uid, (unsigned long) session.gid,
      dir, strerror(xerrno));

    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(xerrno));
    return PR_ERROR(cmd);
  }

  if (pr_fsio_stat(dir, &st) < 0) {
    /* Error stat'ing the file. */
    int xerrno = errno;
    pr_fsio_close(retr_fh);
    errno = xerrno;

    retr_fh = NULL;
    pr_response_add_err(R_550, "%s: %s", cmd->arg, strerror(errno));
    return PR_ERROR(cmd);
  }

  if (session.restart_pos) {

    /* Make sure that the requested offset is valid (within the size of the
     * file being resumed).
     */
    if (session.restart_pos > st.st_size) {
      pr_response_add_err(R_554, _("%s: invalid REST argument"), cmd->arg);
      pr_fsio_close(retr_fh);
      retr_fh = NULL;

      return PR_ERROR(cmd);
    }

    if (pr_fsio_lseek(retr_fh, session.restart_pos,
        SEEK_SET) == (off_t) -1) {
      int xerrno = errno;
      pr_fsio_close(retr_fh);
      errno = xerrno;
      retr_fh = NULL;

      (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
        "error seeking to byte %" PR_LU " of '%s': %s", cmd->argv[0],
        session.user, (unsigned long) session.uid, (unsigned long) session.gid,
        (pr_off_t) session.restart_pos, dir, strerror(xerrno));

      pr_log_debug(DEBUG0, "error seeking to offset %" PR_LU
        " for file %s: %s", (pr_off_t) session.restart_pos, dir,
        strerror(xerrno));
      pr_response_add_err(R_554, _("%s: invalid REST argument"), cmd->arg);
      return PR_ERROR(cmd);
    }

    curr_pos = session.restart_pos;
    session.restart_pos = 0L;
  }

  /* Send the data */
  pr_data_init(cmd->arg, PR_NETIO_IO_WR);

  session.xfer.path = dir;
  session.xfer.file_size = st.st_size;

  cnt_steps = session.xfer.file_size / 100;
  if (cnt_steps == 0)
    cnt_steps = 1;

  if (pr_data_open(cmd->arg, NULL, PR_NETIO_IO_WR, st.st_size - curr_pos) < 0) {
    retr_abort();
    pr_data_abort(0, TRUE);
    return PR_ERROR(cmd);
  }

  /* Retrieve the number of bytes to retrieve, maximum, if present */
  nbytes_max_retrieve = find_max_nbytes("MaxRetrieveFileSize");
  if (nbytes_max_retrieve == 0UL)
    have_limit = FALSE;
  else
    have_limit = TRUE;

  /* Check the MaxRetrieveFileSize.  If it is zero, or if the size
   * of the file being retrieved is greater than the MaxRetrieveFileSize,
   * then signal an error and abort the transfer now.
   */
  if (have_limit &&
      ((nbytes_max_retrieve == 0) || (st.st_size > nbytes_max_retrieve))) {

    pr_log_pri(PR_LOG_INFO, "MaxRetrieveFileSize (%" PR_LU " %s) reached: "
      "aborting transfer of '%s'", (pr_off_t) nbytes_max_retrieve,
      nbytes_max_retrieve != 1 ? "bytes" : "byte", dir);

    /* Abort the transfer. */
    retr_abort();

    /* Set errno to EPERM ("Operation not permitted") */
    pr_data_abort(EPERM, FALSE);
    return PR_ERROR(cmd);
  }

  bufsz = (main_server->tcp_sndbuf_len > 0 ?  main_server->tcp_sndbuf_len :
    pr_config_get_xfer_bufsz());
  lbuf = (char *) palloc(cmd->tmp_pool, bufsz);

  nbytes_sent = curr_pos;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_XFER_SIZE, session.xfer.file_size,
    PR_SCORE_XFER_DONE, (off_t) 0,
    NULL);

  while (nbytes_sent != session.xfer.file_size) {
    pr_signals_handle();

    if (XFER_ABORTED)
      break;

    len = transmit_data(nbytes_sent, &curr_pos, lbuf, bufsz);
    if (len == 0)
      break;

    if (len < 0) {
      /* Make sure that the errno value, needed for the pr_data_abort() call,
       * is preserved; errno itself might be overwritten in retr_abort().
       */
      int xerrno = errno;

      retr_abort();

      pr_data_abort(xerrno, FALSE);
      return PR_ERROR(cmd);
    }

    nbytes_sent += len;

    if ((nbytes_sent / cnt_steps) != cnt_next) {
      cnt_next = nbytes_sent / cnt_steps;

      pr_scoreboard_entry_update(session.pid,
        PR_SCORE_XFER_DONE, nbytes_sent,
        NULL);
    }

    /* If no throttling is configured, this simply updates the scoreboard.
     * In this case, we want to use session.xfer.total_bytes, rather than
     * nbytes_sent, as the latter incorporates a REST position and the
     * former does not.  (When handling STOR, this is not an issue: different
     * end-of-loop conditions).
     */
    pr_throttle_pause(session.xfer.total_bytes, FALSE);
  }

  if (XFER_ABORTED) {
    retr_abort();
    pr_data_abort(0, FALSE);
    return PR_ERROR(cmd);

  } else {
    /* If no throttling is configured, this simply updates the scoreboard.
     * In this case, we want to use session.xfer.total_bytes, rather than
     * nbytes_sent, as the latter incorporates a REST position and the
     * former does not.  (When handling STOR, this is not an issue: different
     * end-of-loop conditions).
     */
    pr_throttle_pause(session.xfer.total_bytes, TRUE);

    retr_complete();

    if (xfer_displayfile() < 0) {
      pr_data_close(FALSE);

    } else {
      pr_data_close(TRUE);
    }
  }

  return PR_HANDLED(cmd);
}

MODRET xfer_abor(cmd_rec *cmd) {
  if (cmd->argc != 1) {
    pr_response_add_err(R_500, _("'%s' not understood"), get_full_cmd(cmd));
    return PR_ERROR(cmd);
  }

  if (session.xfer.direction == PR_NETIO_IO_RD) {
    stor_abort();

  } else if (session.xfer.direction == PR_NETIO_IO_WR) {
    retr_abort();
  }

  pr_data_abort(0, FALSE);
  pr_data_reset();
  pr_data_cleanup();

  pr_response_add(R_226, _("Abort successful"));
  return PR_HANDLED(cmd);
}

MODRET xfer_type(cmd_rec *cmd) {
  if (cmd->argc < 2 || cmd->argc > 3) {
    pr_response_add_err(R_500, _("'%s' not understood"), get_full_cmd(cmd));
    return PR_ERROR(cmd);
  }

  cmd->argv[1][0] = toupper(cmd->argv[1][0]);

  if (!strcmp(cmd->argv[1], "A") ||
      (cmd->argc == 3 && !strcmp(cmd->argv[1], "L") &&
       !strcmp(cmd->argv[2], "7"))) {

    /* TYPE A(SCII) or TYPE L 7. */
    session.sf_flags |= SF_ASCII;

  } else if (!strcmp(cmd->argv[1], "I") ||
      (cmd->argc == 3 && !strcmp(cmd->argv[1], "L") &&
       !strcmp(cmd->argv[2], "8"))) {

    /* TYPE I(MAGE) or TYPE L 8. */
    session.sf_flags &= (SF_ALL^(SF_ASCII|SF_ASCII_OVERRIDE));

  } else {
    pr_response_add_err(R_500, _("'%s' not understood"), get_full_cmd(cmd));
    return PR_ERROR(cmd);
  }

  pr_response_add(R_200, _("Type set to %s"), cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET xfer_stru(cmd_rec *cmd) {
  if (cmd->argc != 2) {
    pr_response_add_err(R_501, _("'%s' not understood"), get_full_cmd(cmd));
    return PR_ERROR(cmd);
  }

  cmd->argv[1][0] = toupper(cmd->argv[1][0]);

  switch ((int) cmd->argv[1][0]) {
    case 'F':
      /* Should 202 be returned instead??? */
      pr_response_add(R_200, _("Structure set to F"));
      return PR_HANDLED(cmd);
      break;

    case 'R':
      /* Accept R but with no operational difference from F???
       * R is required in minimum implementations by RFC-959, 5.1.
       * RFC-1123, 4.1.2.13, amends this to only apply to servers whose file
       * systems support record structures, but also suggests that such a
       * server "may still accept files with STRU R, recording the byte stream
       * literally." Another configurable choice, perhaps?
       *
       * NOTE: wu-ftpd does not so accept STRU R.
       */

       /* FALLTHROUGH */

    case 'P':
      /* RFC-1123 recommends against implementing P. */
      pr_response_add_err(R_504, _("'%s' unsupported structure type"),
        get_full_cmd(cmd));
      return PR_ERROR(cmd);
      break;

    default:
      pr_response_add_err(R_501, _("'%s' unrecognized structure type"),
        get_full_cmd(cmd));
      return PR_ERROR(cmd);
      break;
  }
}

MODRET xfer_mode(cmd_rec *cmd) {
  if (cmd->argc != 2) {
    pr_response_add_err(R_501, _("'%s' not understood"), get_full_cmd(cmd));
    return PR_ERROR(cmd);
  }

  cmd->argv[1][0] = toupper(cmd->argv[1][0]);

  switch ((int) cmd->argv[1][0]) {
    case 'S':
      /* Should 202 be returned instead??? */
      pr_response_add(R_200, _("Mode set to S"));
      return PR_HANDLED(cmd);
      break;

    case 'B':
      /* FALLTHROUGH */

    case 'C':
      pr_response_add_err(R_504, _("'%s' unsupported transfer mode"),
        get_full_cmd(cmd));
      return PR_ERROR(cmd);
      break;

    default:
      pr_response_add_err(R_501, _("'%s' unrecognized transfer mode"),
        get_full_cmd(cmd));
      return PR_ERROR(cmd);
      break;
  }
}

MODRET xfer_allo(cmd_rec *cmd) {
  pr_response_add(R_202, _("No storage allocation necessary"));
  return PR_HANDLED(cmd);
}

MODRET xfer_smnt(cmd_rec *cmd) {
  pr_response_add(R_502, _("SMNT command not implemented"));
  return PR_HANDLED(cmd);
}

MODRET xfer_err_cleanup(cmd_rec *cmd) {
  pr_data_clear_xfer_pool();

  memset(&session.xfer, '\0', sizeof(session.xfer));

  /* Don't forget to clear any possible REST parameter as well. */
  session.restart_pos = 0;

  (void) xfer_prio_restore();
  return PR_DECLINED(cmd);
}

MODRET xfer_log_stor(cmd_rec *cmd) {
  _log_transfer('i', 'c');

  /* Increment the file counters. */
  session.total_files_in++;
  session.total_files_xfer++;

  pr_data_cleanup();

  /* Don't forget to clear any possible REST parameter as well. */
  session.restart_pos = 0;

  (void) xfer_prio_restore();
  return PR_DECLINED(cmd);
}

MODRET xfer_log_retr(cmd_rec *cmd) {

  _log_transfer('o', 'c');

  /* Increment the file counters. */
  session.total_files_out++;
  session.total_files_xfer++;

  pr_data_cleanup();

  /* Don't forget to clear any possible REST parameter as well. */
  session.restart_pos = 0;

  (void) xfer_prio_restore();
  return PR_DECLINED(cmd);
}

static int noxfer_timeout_cb(CALLBACK_FRAME) {
  if (session.sf_flags & SF_XFER)
    /* Transfer in progress, ignore this timeout */
    return 1;

  pr_event_generate("core.timeout-no-transfer", NULL);
  pr_response_send_async(R_421,
    _("No transfer timeout (%d seconds): closing control connection"),
    pr_data_get_timeout(PR_DATA_TIMEOUT_NO_TRANSFER));

  pr_timer_remove(PR_TIMER_IDLE, ANY_MODULE);
  pr_timer_remove(PR_TIMER_LOGIN, ANY_MODULE);

  /* If this timeout is encountered and we are expecting a passive transfer,
   * add some logging that suggests things to check and possibly fix
   * (e.g. network/firewall rules).
   */
  if (session.sf_flags & SF_PASSIVE) {
    pr_log_pri(PR_LOG_INFO,
      "Passive data transfer failed, possibly due to network issues");
    pr_log_pri(PR_LOG_INFO,
      "Check your PassivePorts and MasqueradeAddress settings,");
    pr_log_pri(PR_LOG_INFO,
       "and any router, NAT, and firewall rules in the network path.");
  }

  session_exit(PR_LOG_NOTICE, "FTP no transfer timeout, disconnected", 0, NULL);
  return 0;
}

MODRET xfer_post_pass(cmd_rec *cmd) {
  config_rec *c;

  c = find_config(TOPLEVEL_CONF, CONF_PARAM, "TimeoutNoTransfer", FALSE);
  if (c != NULL) {
    int timeout = *((int *) c->argv[0]);
    pr_data_set_timeout(PR_DATA_TIMEOUT_NO_TRANSFER, timeout);

    /* Setup timer */
    if (timeout > 0) {
      pr_timer_add(timeout, PR_TIMER_NOXFER, &xfer_module, noxfer_timeout_cb,
        "TimeoutNoTransfer");
    }
  }

  c = find_config(TOPLEVEL_CONF, CONF_PARAM, "TimeoutStalled", FALSE);
  if (c != NULL) {
    int timeout = *((int *) c->argv[0]);
    pr_data_set_timeout(PR_DATA_TIMEOUT_STALLED, timeout);

    /* Note: timers for handling TimeoutStalled timeouts are handled in the
     * data transfer routines, not here.
     */
  }

  /* Check for TransferPriority. */
  c = find_config(TOPLEVEL_CONF, CONF_PARAM, "TransferPriority", FALSE);
  if (c) {
    xfer_prio_flags = *((unsigned long *) c->argv[0]);
    xfer_prio_config = *((int *) c->argv[1]);
  }

  return PR_DECLINED(cmd);
}

/* Configuration handlers
 */

MODRET set_allowoverwrite(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR|CONF_DYNDIR);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = (unsigned char) bool;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_allowrestart(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR|CONF_DYNDIR);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_deleteabortedstores(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR|CONF_DYNDIR);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

/* usage: DisplayFileTransfer path */
MODRET set_displayfiletransfer(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  (void) add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

MODRET set_hiddenstores(cmd_rec *cmd) {
  int bool = -1, add_periods = TRUE;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|CONF_DIR);

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);

  /* Handle the case where the admin may, for some reason, want a custom
   * prefix which could also be construed to be a Boolean value by
   * get_boolean(): if the value begins AND ends with a period, then treat
   * it as a custom prefix.
   */
  if ((cmd->argv[1])[0] == '.' &&
      (cmd->argv[1])[strlen(cmd->argv[1])-1] == '.') {
    add_periods = FALSE;
    bool = -1;

  } else {
    bool = get_boolean(cmd, 1);
  }

  if (bool == -1) {
    /* If the parameter is not a Boolean parameter, assume that the
     * admin is configuring a specific prefix to use instead of the
     * default ".in.".
     */

    c->argv[0] = pcalloc(c->pool, sizeof(int));
    *((int *) c->argv[0]) = TRUE;

    if (add_periods) {
      /* Automatically add the leading and trailing periods. */
      c->argv[1] = pstrcat(c->pool, ".", cmd->argv[1], ".", NULL);

    } else {
      c->argv[1] = pstrdup(c->pool, cmd->argv[1]);
    }

  } else {
    c->argv[0] = pcalloc(c->pool, sizeof(int));
    *((int *) c->argv[0]) = bool;

    if (bool) {
      /* The default HiddenStore prefix */
      c->argv[1] = pstrdup(c->pool, ".in.");
    }
  }

  c->flags |= CF_MERGEDOWN;
  return PR_HANDLED(cmd);
}

MODRET set_maxfilesize(cmd_rec *cmd) {
  config_rec *c = NULL;
  unsigned long nbytes;
  unsigned int precedence = 0;

  int ctxt = (cmd->config && cmd->config->config_type != CONF_PARAM ?
     cmd->config->config_type : cmd->server->config_type ?
     cmd->server->config_type : CONF_ROOT);

  if (cmd->argc-1 == 1) {
    if (strcmp(cmd->argv[1], "*") != 0)
      CONF_ERROR(cmd, "incorrect number of parameters");

  } else if (cmd->argc-1 != 2 && cmd->argc-1 != 4)
    CONF_ERROR(cmd, "incorrect number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_ANON|CONF_VIRTUAL|CONF_GLOBAL|CONF_DIR|
    CONF_DYNDIR);

  /* Set the precedence for this config_rec based on its configuration
   * context.
   */
  if (ctxt & CONF_GLOBAL)
    precedence = 1;

  /* These will never appear simultaneously */
  else if (ctxt & CONF_ROOT || ctxt & CONF_VIRTUAL)
    precedence = 2;

  else if (ctxt & CONF_ANON)
    precedence = 3;

  else if (ctxt & CONF_DIR)
    precedence = 4;

  else if (ctxt & CONF_DYNDIR)
    precedence = 5;

  /* If the directive was used with four arguments, it means the optional
   * classifiers and expression were used.  Make sure the classifier is a valid
   * one.
   */
  if (cmd->argc-1 == 4) {
    if (!strcmp(cmd->argv[3], "user") ||
        !strcmp(cmd->argv[3], "group") ||
        !strcmp(cmd->argv[3], "class")) {

       /* no-op */

     } else
       CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unknown classifier used: '",
         cmd->argv[3], "'", NULL));
  }

  if (cmd->argc-1 == 1) {

    /* Do nothing here -- the "*" (the only parameter allowed if there is
     * only a single parameter given) signifies an unlimited size, which is
     * what the server provides by default.
     */
    nbytes = 0UL;

  } else {

    /* Pass the cmd_rec off to see what number of bytes was
     * requested/configured.
     */
    nbytes = parse_max_nbytes(cmd->argv[1], cmd->argv[2]);
    if (nbytes == 0) {
      if (xfer_errno == EINVAL)
        CONF_ERROR(cmd, "invalid parameters");

      if (xfer_errno == ERANGE) {
        char ulong_max[80];

        memset(ulong_max, '\0', sizeof(ulong_max));
        snprintf(ulong_max, sizeof(ulong_max)-1, "%lu",
          (unsigned long) ULONG_MAX);

        CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
         "number of bytes must be between 0 and ", ulong_max, NULL));
      }
    }
  }

  if (cmd->argc-1 == 1 || cmd->argc-1 == 2) {
    c = add_config_param(cmd->argv[0], 2, NULL, NULL);
    c->argv[0] = pcalloc(c->pool, sizeof(unsigned long));
    *((unsigned long *) c->argv[0]) = nbytes;
    c->argv[1] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[1]) = precedence;

  } else {
    array_header *acl = NULL;
    int argc = cmd->argc - 4;
    char **argv = cmd->argv + 3;

    acl = pr_expr_create(cmd->tmp_pool, &argc, argv);

    c = add_config_param(cmd->argv[0], 0);
    c->argc = argc + 3;
    c->argv = pcalloc(c->pool, ((argc + 4) * sizeof(char *)));

    argv = (char **) c->argv;

    /* Copy in the configured bytes */
    *argv = pcalloc(c->pool, sizeof(unsigned long));
    *((unsigned long *) *argv++) = nbytes;

    /* Copy in the precedence */
    *argv = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) *argv++) = precedence;

    /* Copy in the classifier. */
    *argv++ = pstrdup(c->pool, cmd->argv[3]);

    if (argc && acl) {
      while (argc--) {
        *argv++ = pstrdup(c->pool, *((char **) acl->elts));
        acl->elts = ((char **) acl->elts) + 1;
      }
    }

    /* Don't forget the terminating NULL */
    *argv = NULL;
  }

  c->flags |= CF_MERGEDOWN_MULTI;

  return PR_HANDLED(cmd);
}

/* usage: MaxTransfersPerHost cmdlist count [msg] */
MODRET set_maxtransfersperhost(cmd_rec *cmd) {
  config_rec *c = NULL;
  int count = 0;

  if (cmd->argc-1 < 2 ||
      cmd->argc-1 > 3)
    CONF_ERROR(cmd, "bad number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR|CONF_DYNDIR);

  count = atoi(cmd->argv[2]);
  if (count < 1)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "count must be greater than zero: '",
      cmd->argv[2], "'", NULL));

  c = add_config_param(cmd->argv[0], 3, NULL, NULL, NULL);

  /* Parse the command list. */
  if (xfer_parse_cmdlist(cmd->argv[0], c, cmd->argv[1]) < 0)
    CONF_ERROR(cmd, "error with command list");

  c->argv[1] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[1]) = count;

  if (cmd->argc-1 == 3)
    c->argv[2] = pstrdup(c->pool, cmd->argv[3]);

  c->flags |= CF_MERGEDOWN_MULTI;

  return PR_HANDLED(cmd);
}

/* usage: MaxTransfersPerUser cmdlist count [msg] */
MODRET set_maxtransfersperuser(cmd_rec *cmd) {
  config_rec *c = NULL;
  int count = 0;

  if (cmd->argc-1 < 2 ||
      cmd->argc-1 > 3) 
    CONF_ERROR(cmd, "bad number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR|CONF_DYNDIR);

  count = atoi(cmd->argv[2]);
  if (count < 1)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "count must be greater than zero: '",
      cmd->argv[2], "'", NULL));

  c = add_config_param(cmd->argv[0], 3, NULL, NULL, NULL);

  /* Parse the command list. */
  if (xfer_parse_cmdlist(cmd->argv[0], c, cmd->argv[1]) < 0)
    CONF_ERROR(cmd, "error with command list");

  c->argv[1] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[1]) = count;

  if (cmd->argc-1 == 3)
    c->argv[2] = pstrdup(c->pool, cmd->argv[3]);

  c->flags |= CF_MERGEDOWN_MULTI;

  return PR_HANDLED(cmd);
}

MODRET set_storeuniqueprefix(cmd_rec *cmd) {
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR|CONF_DYNDIR);

  /* make sure there are no slashes in the prefix */
  if (strchr(cmd->argv[1], '/') != NULL)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "no slashes allowed in prefix: '",
      cmd->argv[1], "'", NULL));

  c = add_config_param_str(cmd->argv[0], 1, (void *) cmd->argv[1]);
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_timeoutnoxfer(cmd_rec *cmd) {
  int timeout = -1;
  char *endp = NULL;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  timeout = (int) strtol(cmd->argv[1], &endp, 10);

  if ((endp && *endp) || timeout < 0 || timeout > 65535)
    CONF_ERROR(cmd, "timeout values must be between 0 and 65535");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = timeout;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

MODRET set_timeoutstalled(cmd_rec *cmd) {
  int timeout = -1;
  char *endp = NULL;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  timeout = (int) strtol(cmd->argv[1], &endp, 10);

  if ((endp && *endp) || timeout < 0 || timeout > 65535)
    CONF_ERROR(cmd, "timeout values must be between 0 and 65535");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = timeout;
  c->flags |= CF_MERGEDOWN;

  return PR_HANDLED(cmd);
}

/* usage: TransferPriority cmds "low"|"medium"|"high"|number
 */
MODRET set_transferpriority(cmd_rec *cmd) {
  config_rec *c;
  int prio;
  char *str;
  unsigned long flags = 0;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  if (strcasecmp(cmd->argv[2], "low") == 0) {
    prio = 15;

  } else if (strcasecmp(cmd->argv[2], "medium") == 0) {
    prio = 0;

  } else if (strcasecmp(cmd->argv[2], "high") == 0) {
    prio = -15;

  } else {
    int res = atoi(cmd->argv[2]);

    if (res < PRIO_MIN ||
        res > PRIO_MAX) {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": invalid priority '",
        cmd->argv[2], "'", NULL));
    }

    prio = res;
  }

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);

  /* Parse the command list. */
  while ((str = get_cmd_from_list(&cmd->argv[1])) != NULL) {

    if (strcmp(str, C_APPE) == 0) {
      flags |= PR_XFER_PRIO_FL_APPE;

    } else if (strcmp(str, C_RETR) == 0) {
      flags |= PR_XFER_PRIO_FL_RETR;

    } else if (strcmp(str, C_STOR) == 0) {
      flags |= PR_XFER_PRIO_FL_STOR;

    } else if (strcmp(str, C_STOU) == 0) {
      flags |= PR_XFER_PRIO_FL_STOU;

    } else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
        ": invalid FTP transfer command: '", str, "'", NULL));
    }
  }

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned long));
  *((unsigned long *) c->argv[0]) = flags;
  c->argv[1] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[1]) = prio;
  c->flags |= CF_MERGEDOWN;

  return HANDLED(cmd);
}

/* usage: TransferRate cmds kbps[:free-bytes] ["user"|"group"|"class"
 *          expression]
 */
MODRET set_transferrate(cmd_rec *cmd) {
  config_rec *c = NULL;
  char *tmp = NULL, *endp = NULL;
  long double rate = 0.0;
  off_t freebytes = 0;
  unsigned int precedence = 0;

  int ctxt = (cmd->config && cmd->config->config_type != CONF_PARAM ?
     cmd->config->config_type : cmd->server->config_type ?
     cmd->server->config_type : CONF_ROOT);

  /* Must have two or four parameters */
  if (cmd->argc-1 != 2 && cmd->argc-1 != 4)
    CONF_ERROR(cmd, "wrong number of parameters");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR|CONF_DYNDIR);

  /* Set the precedence for this config_rec based on its configuration
   * context.
   */
  if (ctxt & CONF_GLOBAL)
    precedence = 1;

  /* These will never appear simultaneously */
  else if (ctxt & CONF_ROOT || ctxt & CONF_VIRTUAL)
    precedence = 2;

  else if (ctxt & CONF_ANON)
    precedence = 3;

  else if (ctxt & CONF_DIR)
    precedence = 4;

  /* Note: by tweaking this value to be lower than the precedence for
   * <Directory> appearances of this directive, I can effectively cause
   * any .ftpaccess appearances not to override...
   */
  else if (ctxt & CONF_DYNDIR)
    precedence = 5;

  /* Check for a valid classifier. */
  if (cmd->argc-1 > 2) {
    if (!strcmp(cmd->argv[3], "user") ||
        !strcmp(cmd->argv[3], "group") ||
        !strcmp(cmd->argv[3], "class"))
      /* do nothing */
      ;
    else
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unknown classifier requested: '",
        cmd->argv[3], "'", NULL));
  }

  if ((tmp = strchr(cmd->argv[2], ':')) != NULL)
    *tmp = '\0';

  /* Parse the 'kbps' part.  Ideally, we'd be using strtold(3) rather than
   * strtod(3) here, but FreeBSD doesn't have strtold(3).  Yay.  Portability.
   */
  rate = (long double) strtod(cmd->argv[2], &endp);

  if (rate < 0.0)
    CONF_ERROR(cmd, "rate must be greater than zero");

  if (endp && *endp)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "invalid number: '",
      cmd->argv[2], "'", NULL));

  /* Parse any 'free-bytes' part */
  if (tmp) {
    cmd->argv[2] = ++tmp;

    freebytes = strtoul(cmd->argv[2], &endp, 10);
    if (endp && *endp) {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "invalid number: '",
        cmd->argv[2], "'", NULL));
    }
  }

  /* Construct the config_rec */
  if (cmd->argc-1 == 2) {
    c = add_config_param(cmd->argv[0], 4, NULL, NULL, NULL, NULL);

    /* Parse the command list. */
    if (xfer_parse_cmdlist(cmd->argv[0], c, cmd->argv[1]) < 0)
      CONF_ERROR(cmd, "error with command list");

    c->argv[1] = pcalloc(c->pool, sizeof(long double));
    *((long double *) c->argv[1]) = rate;
    c->argv[2] = pcalloc(c->pool, sizeof(off_t));
    *((off_t *) c->argv[2]) = freebytes;
    c->argv[3] = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) c->argv[3]) = precedence;

  } else {
    array_header *acl = NULL;
    int argc = cmd->argc - 4;
    char **argv = cmd->argv + 3;

    acl = pr_expr_create(cmd->tmp_pool, &argc, argv);

    c = add_config_param(cmd->argv[0], 0);

    /* Parse the command list.
     *
     * The five additional slots are for: cmd-list, bps, free-bytes,
     * precedence, user/group/class.
     */
    c->argc = argc + 5;

    c->argv = pcalloc(c->pool, ((c->argc + 1) * sizeof(char *)));
    argv = (char **) c->argv;

    if (xfer_parse_cmdlist(cmd->argv[0], c, cmd->argv[1]) < 0)
      CONF_ERROR(cmd, "error with command list");

    /* Note: the command list is at index 0, hence this increment. */
    argv++;

    *argv = pcalloc(c->pool, sizeof(long double));
    *((long double *) *argv++) = rate;
    *argv = pcalloc(c->pool, sizeof(off_t));
    *((unsigned long *) *argv++) = freebytes;
    *argv = pcalloc(c->pool, sizeof(unsigned int));
    *((unsigned int *) *argv++) = precedence;

    *argv++ = pstrdup(c->pool, cmd->argv[3]);

    if (argc && acl) {
      while (argc--) {
        *argv++ = pstrdup(c->pool, *((char **) acl->elts));
        acl->elts = ((char **) acl->elts) + 1;
      }
    }

    /* don't forget the terminating NULL */
    *argv = NULL;
  }

  c->flags |= CF_MERGEDOWN_MULTI;
  return PR_HANDLED(cmd);
}

/* usage: UseSendfile on|off */
MODRET set_usesendfile(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(unsigned char));
  *((unsigned char *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

/* Event handlers
 */

static void xfer_sigusr2_ev(const void *event_data, void *user_data) {

  /* Only do this if we're currently involved in a data transfer.
   * This is a hack put in to support mod_shaper's antics.
   */
  if (strcmp(session.curr_cmd, C_APPE) == 0 ||
      strcmp(session.curr_cmd, C_RETR) == 0 ||
      strcmp(session.curr_cmd, C_STOR) == 0 ||
      strcmp(session.curr_cmd, C_STOU) == 0) {
    pool *p = make_sub_pool(session.pool);
    cmd_rec *cmd = pr_cmd_alloc(p, 1, session.curr_cmd);

    /* Rescan the config tree for TransferRates, picking up any possible
     * changes.
     */
    pr_log_debug(DEBUG2, "rechecking TransferRates");
    pr_throttle_init(cmd);

    destroy_pool(p);
  }

  return;
}

/* Events handlers
 */

static void xfer_exit_ev(const void *event_data, void *user_data) {

  if (session.sf_flags & SF_XFER) {
    if (session.xfer.direction == PR_NETIO_IO_RD) {
       /* An upload is occurring... */
      pr_trace_msg(trace_channel, 6, "session exiting, aborting upload");
      stor_abort();

    } else {
      /* A download is occurring... */
      pr_trace_msg(trace_channel, 6, "session exiting, aborting download");
      retr_abort();
    }
  }

  return;
}

static void xfer_xfer_stalled_ev(const void *event_data, void *user_data) {
  if (!(session.sf_flags & SF_XFER)) {
    if (session.xfer.direction == PR_NETIO_IO_RD) {
      pr_trace_msg(trace_channel, 6, "transfer stalled, aborting upload");
      stor_abort();

    } else {
      pr_trace_msg(trace_channel, 6, "transfer stalled, aborting download");
      retr_abort();
    }
  }

  /* The "else" case, for a stalled transfer, will be handled by the
   * 'core.exit' event handler above.  In that case, a data transfer
   * _will_ have actually been in progress, whereas in the !SF_XFER
   * case, the client requested a transfer, but never actually opened
   * the data connection.
   */

  return;
}

/* Initialization routines
 */

static int xfer_init(void) {

  /* Add the commands handled by this module to the HELP list. */
  pr_help_add(C_TYPE, "<sp> type-code (A, I, L 7, L 8)", TRUE);
  pr_help_add(C_STRU, "is not implemented (always F)", TRUE);
  pr_help_add(C_MODE, "is not implemented (always S)", TRUE);
  pr_help_add(C_RETR, "<sp> pathname", TRUE);
  pr_help_add(C_STOR, "<sp> pathname", TRUE);
  pr_help_add(C_STOU, "(store unique filename)", TRUE);
  pr_help_add(C_APPE, "<sp> pathname", TRUE);
  pr_help_add(C_REST, "<sp> byte-count", TRUE);
  pr_help_add(C_ABOR, "(abort current operation)", TRUE);

  return 0;
}

static int xfer_sess_init(void) {
  char *displayfilexfer = NULL;
  config_rec *c = NULL;

  /* Check for UseSendfile. */
  c = find_config(main_server->conf, CONF_PARAM, "UseSendfile", FALSE);
  if (c) {
    use_sendfile = *((unsigned char *) c->argv[0]);
  }

  /* Exit handlers for HiddenStores cleanup */
  pr_event_register(&xfer_module, "core.exit", xfer_exit_ev, NULL);
  pr_event_register(&xfer_module, "core.timeout-stalled",
    xfer_xfer_stalled_ev, NULL);

  pr_event_register(&xfer_module, "core.signal.USR2", xfer_sigusr2_ev,
    NULL);

  /* Look for a DisplayFileTransfer file which has an absolute path.  If we
   * find one, open a filehandle, such that that file can be displayed
   * even if the session is chrooted.  DisplayFileTransfer files with
   * relative paths will be handled after chroot, preserving the old
   * behavior.
   */
  displayfilexfer = get_param_ptr(main_server->conf, "DisplayFileTransfer",
    FALSE);
  if (displayfilexfer &&
      *displayfilexfer == '/') {

    displayfilexfer_fh = pr_fsio_open(displayfilexfer, O_RDONLY);
    if (displayfilexfer_fh == NULL)
      pr_log_debug(DEBUG6, "unable to open DisplayFileTransfer file '%s': %s",
        displayfilexfer, strerror(errno));
  }

  return 0;
}

/* Module API tables
 */

static conftable xfer_conftab[] = {
  { "AllowOverwrite",		set_allowoverwrite,		NULL },
  { "AllowRetrieveRestart",	set_allowrestart,		NULL },
  { "AllowStoreRestart",	set_allowrestart,		NULL },
  { "DeleteAbortedStores",	set_deleteabortedstores,	NULL },
  { "DisplayFileTransfer",	set_displayfiletransfer,	NULL },
  { "HiddenStores",		set_hiddenstores,		NULL },
  { "MaxRetrieveFileSize",	set_maxfilesize,		NULL },
  { "MaxStoreFileSize",		set_maxfilesize,		NULL },
  { "MaxTransfersPerHost",	set_maxtransfersperhost,	NULL },
  { "MaxTransfersPerUser",	set_maxtransfersperuser,	NULL },
  { "StoreUniquePrefix",	set_storeuniqueprefix,		NULL },
  { "TimeoutNoTransfer",	set_timeoutnoxfer,		NULL },
  { "TimeoutStalled",		set_timeoutstalled,		NULL },
  { "TransferPriority",		set_transferpriority,		NULL },
  { "TransferRate",		set_transferrate,		NULL },
  { "UseSendfile",		set_usesendfile,		NULL },

  { NULL }
};

static cmdtable xfer_cmdtab[] = {
  { CMD,     C_TYPE,	G_NONE,	 xfer_type,	FALSE,	FALSE, CL_MISC },
  { CMD,     C_STRU,	G_NONE,	 xfer_stru,	TRUE,	FALSE, CL_MISC },
  { CMD,     C_MODE,	G_NONE,	 xfer_mode,	TRUE,	FALSE, CL_MISC },
  { POST_CMD,C_MODE,	G_NONE,  xfer_post_mode,FALSE,	FALSE },
  { CMD,     C_ALLO,	G_NONE,	 xfer_allo,	TRUE,	FALSE, CL_MISC },
  { CMD,     C_SMNT,	G_NONE,	 xfer_smnt,	TRUE,	FALSE, CL_MISC },
  { PRE_CMD, C_RETR,	G_READ,	 xfer_pre_retr,	TRUE,	FALSE },
  { CMD,     C_RETR,	G_READ,	 xfer_retr,	TRUE,	FALSE, CL_READ },
  { LOG_CMD, C_RETR,	G_NONE,	 xfer_log_retr,	FALSE,  FALSE },
  { LOG_CMD_ERR, C_RETR,G_NONE,  xfer_err_cleanup,  FALSE,  FALSE },
  { PRE_CMD, C_STOR,	G_WRITE, xfer_pre_stor,	TRUE,	FALSE },
  { CMD,     C_STOR,	G_WRITE, xfer_stor,	TRUE,	FALSE, CL_WRITE },
  { LOG_CMD, C_STOR,    G_NONE,	 xfer_log_stor,	FALSE,  FALSE },
  { LOG_CMD_ERR, C_STOR,G_NONE,  xfer_err_cleanup,  FALSE,  FALSE },
  { PRE_CMD, C_STOU,	G_WRITE, xfer_pre_stou,	TRUE,	FALSE },
  { CMD,     C_STOU,	G_WRITE, xfer_stor,	TRUE,	FALSE, CL_WRITE },
  { POST_CMD,C_STOU,	G_WRITE, xfer_post_stou,FALSE,	FALSE },
  { LOG_CMD, C_STOU,	G_NONE,  xfer_log_stor,	FALSE,	FALSE },
  { LOG_CMD_ERR, C_STOU,G_NONE,  xfer_err_cleanup,  FALSE,  FALSE },
  { PRE_CMD, C_APPE,	G_WRITE, xfer_pre_appe,	TRUE,	FALSE },
  { CMD,     C_APPE,	G_WRITE, xfer_stor,	TRUE,	FALSE, CL_WRITE },
  { LOG_CMD, C_APPE,	G_NONE,  xfer_log_stor,	FALSE,  FALSE },
  { LOG_CMD_ERR, C_APPE,G_NONE,  xfer_err_cleanup,  FALSE,  FALSE },
  { CMD,     C_ABOR,	G_NONE,	 xfer_abor,	TRUE,	TRUE,  CL_MISC  },
  { CMD,     C_REST,	G_NONE,	 xfer_rest,	TRUE,	FALSE, CL_MISC  },
  { POST_CMD,C_PROT,	G_NONE,  xfer_post_prot,	FALSE,	FALSE },
  { POST_CMD,C_PASS,	G_NONE,	 xfer_post_pass,	FALSE, FALSE },
  { 0, NULL }
};

module xfer_module = {
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "xfer",

  /* Module configuration directive table */
  xfer_conftab,

  /* Module command handler table */
  xfer_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  xfer_init,

  /* Session initialization function */
  xfer_sess_init
};
