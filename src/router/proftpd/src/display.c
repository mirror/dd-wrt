/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2004-2011 The ProFTPD Project team
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Display of files
 * $Id: display.c,v 1.26 2011/05/23 21:22:24 castaglia Exp $
 */

#include "conf.h"

static int first_msg_sent = FALSE;
static const char *first_msg = NULL;
static const char *prev_msg = NULL;

static void format_size_str(char *buf, size_t buflen, off_t size) {
  char units[] = {'K', 'M', 'G', 'T', 'P'};
  register unsigned int i = 0;

  /* Determine the appropriate units label to use. */
  while (size > 1024) {
    size /= 1024;
    i++;
  }

  /* Now, prepare the buffer. */
  snprintf(buf, buflen, "%.3" PR_LU "%cB", (pr_off_t) size, units[i]);
}

static int display_add_line(pool *p, const char *resp_code,
    const char *resp_msg) {

  /* Handle the case where the data to Display might contain only one line. */

  if (first_msg_sent == FALSE &&
      first_msg == NULL) {
      first_msg = pstrdup(p, resp_msg);
    return 0;
  }

  if (first_msg != NULL) {
    pr_response_send_raw("%s-%s", resp_code, first_msg);
    first_msg = NULL;
    first_msg_sent = TRUE;

    prev_msg = pstrdup(p, resp_msg);
    return 0; 
  }

  if (prev_msg != NULL) {
    if (session.multiline_rfc2228) {
      pr_response_send_raw("%s-%s", resp_code, prev_msg);

    } else {
      pr_response_send_raw(" %s", prev_msg);
    }
  }

  prev_msg = pstrdup(p, resp_msg);
  return 0;
}

static int display_flush_lines(pool *p, const char *resp_code, int flags) {
  if (first_msg != NULL) {
    if (session.auth_mech != NULL) {
      if (flags & PR_DISPLAY_FL_NO_EOM) {
        pr_response_send_raw("%s-%s", resp_code, first_msg);

      } else {
        pr_response_send_raw("%s %s", resp_code, first_msg);
      }

    } else {
      /* There is a special case if the client has not yet authenticated; it
       * means we are handling a DisplayConnect file.  The server will send
       * a banner as well, so we need to treat this is the start of a multiline
       * response.
       */
      pr_response_send_raw("%s-%s", resp_code, first_msg);
    }

  } else {
    if (prev_msg) {
      if (session.multiline_rfc2228) {
        pr_response_send_raw("%s-%s", resp_code, prev_msg);

      } else {
        if (flags & PR_DISPLAY_FL_NO_EOM) {
          pr_response_send_raw(" %s", prev_msg);

        } else {
          pr_response_send_raw("%s %s", resp_code, prev_msg);
        }
      }
    }
  }

  /* Reset state for the next set of lines. */
  first_msg_sent = FALSE;
  first_msg = NULL;
  prev_msg = NULL;

  return 0;
}

static int display_fh(pr_fh_t *fh, const char *fs, const char *code,
    int flags) {
  struct stat st;
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  int len, res;
  unsigned int *current_clients = NULL;
  unsigned int *max_clients = NULL;
  off_t fs_size = 0;
  pool *p;
  void *v;
  xaset_t *s;
  config_rec *c = NULL;
  const char *serverfqdn = main_server->ServerFQDN;
  char *outs, mg_size[12] = {'\0'}, mg_size_units[12] = {'\0'},
    mg_max[12] = "unlimited";
  char total_files_in[12] = {'\0'}, total_files_out[12] = {'\0'},
    total_files_xfer[12] = {'\0'};
  char mg_class_limit[12] = {'\0'}, mg_cur[12] = {'\0'},
    mg_xfer_bytes[12] = {'\0'}, mg_cur_class[12] = {'\0'};
  char mg_xfer_units[12] = {'\0'}, *user;
  const char *mg_time;
  char *rfc1413_ident = NULL;

  /* Stat the opened file to determine the optimal buffer size for IO. */
  memset(&st, 0, sizeof(st));
  pr_fsio_fstat(fh, &st);
  fh->fh_iosz = st.st_blksize;

  res = pr_fs_getsize2(fh->fh_path, &fs_size);
  if (res < 0 &&
      errno != ENOSYS) {
    (void) pr_log_debug(DEBUG7, "error getting filesystem size for '%s': %s",
      fh->fh_path, strerror(errno));
    fs_size = 0;
  }

  snprintf(mg_size, sizeof(mg_size), "%" PR_LU, (pr_off_t) fs_size);
  format_size_str(mg_size_units, sizeof(mg_size_units), fs_size);

  p = make_sub_pool(session.pool);
  pr_pool_tag(p, "Display Pool");

  s = (session.anon_config ? session.anon_config->subset : main_server->conf);

  mg_time = pr_strtime(time(NULL));

  max_clients = get_param_ptr(s, "MaxClients", FALSE);

  v = pr_table_get(session.notes, "client-count", NULL);
  if (v) {
    current_clients = v;
  }

  snprintf(mg_cur, sizeof(mg_cur), "%u", current_clients ? *current_clients: 1);

  if (session.class && session.class->cls_name) {
    unsigned int *class_clients = NULL;
    config_rec *maxc = NULL;
    unsigned int maxclients = 0;

    v = pr_table_get(session.notes, "class-client-count", NULL);
    if (v) {
      class_clients = v;
    }

    snprintf(mg_cur_class, sizeof(mg_cur_class), "%u",
      class_clients ? *class_clients : 0);

    /* For the %z variable, first we scan through the MaxClientsPerClass,
     * and use the first applicable one.  If none are found, look for
     * any MaxClients set.
     */

    maxc = find_config(main_server->conf, CONF_PARAM, "MaxClientsPerClass",
      FALSE);

    while (maxc) {
      pr_signals_handle();

      if (strcmp(maxc->argv[0], session.class->cls_name) != 0) {
        maxc = find_config_next(maxc, maxc->next, CONF_PARAM,
          "MaxClientsPerClass", FALSE);
        continue;
      }

      maxclients = *((unsigned int *) maxc->argv[1]);
      break;
    }

    if (maxclients == 0) {
      maxc = find_config(main_server->conf, CONF_PARAM, "MaxClients", FALSE);

      if (maxc)
        maxclients = *((unsigned int *) maxc->argv[0]);
    }

    snprintf(mg_class_limit, sizeof(mg_class_limit), "%u", maxclients);

  } else {
    snprintf(mg_class_limit, sizeof(mg_class_limit), "%u",
      max_clients ? *max_clients : 0);
    snprintf(mg_cur_class, sizeof(mg_cur_class), "%u", 0);
  }

  snprintf(mg_xfer_bytes, sizeof(mg_xfer_bytes), "%" PR_LU,
    (pr_off_t) session.total_bytes >> 10);
  snprintf(mg_xfer_units, sizeof(mg_xfer_units), "%" PR_LU "B",
    (pr_off_t) session.total_bytes);

  if (session.total_bytes >= 10240) {
    snprintf(mg_xfer_units, sizeof(mg_xfer_units), "%" PR_LU "kB",
      (pr_off_t) session.total_bytes >> 10);

  } else if ((session.total_bytes >> 10) >= 10240) {
    snprintf(mg_xfer_units, sizeof(mg_xfer_units), "%" PR_LU "MB",
      (pr_off_t) session.total_bytes >> 20);

  } else if ((session.total_bytes >> 20) >= 10240) {
    snprintf(mg_xfer_units, sizeof(mg_xfer_units), "%" PR_LU "GB",
      (pr_off_t) session.total_bytes >> 30);
  }

  snprintf(mg_max, sizeof(mg_max), "%u", max_clients ? *max_clients : 0);

  user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);
  if (user == NULL)
    user = "";

  c = find_config(main_server->conf, CONF_PARAM, "MasqueradeAddress", FALSE);
  if (c) {
    pr_netaddr_t *masq_addr = (pr_netaddr_t *) c->argv[0];
    serverfqdn = pr_netaddr_get_dnsstr(masq_addr);
  }

  /* "Stringify" the file number for this session. */
  snprintf(total_files_in, sizeof(total_files_in), "%u",
    session.total_files_in);
  total_files_in[sizeof(total_files_in)-1] = '\0';

  snprintf(total_files_out, sizeof(total_files_out), "%u",
    session.total_files_out);
  total_files_out[sizeof(total_files_out)-1] = '\0';

  snprintf(total_files_xfer, sizeof(total_files_xfer), "%u",
    session.total_files_xfer);
  total_files_xfer[sizeof(total_files_xfer)-1] = '\0';

  rfc1413_ident = pr_table_get(session.notes, "mod_ident.rfc1413-ident", NULL);
  if (rfc1413_ident == NULL) {
    rfc1413_ident = "UNKNOWN";
  }

  while (pr_fsio_gets(buf, sizeof(buf), fh) != NULL) {
    char *tmp;

    pr_signals_handle();

    buf[sizeof(buf)-1] = '\0';
    len = strlen(buf);

    while (len &&
           (buf[len-1] == '\r' || buf[len-1] == '\n')) {
      buf[len-1] = '\0';
      len--;
    }

    /* Check for any Variable-type strings. */
    tmp = strstr(buf, "%{");
    while (tmp) {
      char *key, *tmp2;
      const char *val;

      pr_signals_handle();

      tmp2 = strchr(tmp, '}');
      if (!tmp2) {
        tmp = strstr(tmp + 1, "%{");
        continue;
      }

      key = pstrndup(p, tmp, tmp2 - tmp + 1);

      /* There are a couple of special-case keys to watch for:
       *
       *   env:$var
       *   time:$fmt
       *
       * The Var API does not easily support returning values for keys
       * where part of the value depends on part of the key.  That's why
       * these keys are handled here, instead of in pr_var_get().
       */

      if (strncmp(key, "%{time:", 7) == 0) {
        char time_str[128], *fmt;
        time_t now;
        struct tm *time_info;

        fmt = pstrndup(p, key + 7, strlen(key) - 8);

        now = time(NULL);
        time_info = pr_localtime(NULL, &now);

        memset(time_str, 0, sizeof(time_str));
        strftime(time_str, sizeof(time_str), fmt, time_info);

        val = pstrdup(p, time_str);

      } else if (strncmp(key, "%{env:", 6) == 0) {
        char *env_var;

        env_var = pstrndup(p, key + 6, strlen(key) - 7);
        val = pr_env_get(p, env_var);
        if (val == NULL) {
          pr_trace_msg("var", 4,
            "no value set for environment variable '%s', using \"(none)\"",
            env_var);
          val = "(none)";
        }

      } else {
        val = pr_var_get(key);
        if (val == NULL) {
          pr_trace_msg("var", 4,
            "no value set for name '%s', using \"(none)\"", key);
          val = "(none)";
        }
      }

      outs = sreplace(p, buf, key, val, NULL);
      sstrncpy(buf, outs, sizeof(buf));

      tmp = strstr(outs, "%{");
    }

    outs = sreplace(p, buf,
      "%C", (session.cwd[0] ? session.cwd : "(none)"),
      "%E", main_server->ServerAdmin,
      "%F", mg_size,
      "%f", mg_size_units,
      "%i", total_files_in,
      "%K", mg_xfer_bytes,
      "%k", mg_xfer_units,
      "%L", serverfqdn,
      "%M", mg_max,
      "%N", mg_cur,
      "%o", total_files_out,
      "%R", (session.c && session.c->remote_name ?
        session.c->remote_name : "(unknown)"),
      "%T", mg_time,
      "%t", total_files_xfer,
      "%U", user,
      "%u", rfc1413_ident,
      "%V", main_server->ServerName,
      "%x", session.class ? session.class->cls_name : "(unknown)",
      "%y", mg_cur_class,
      "%z", mg_class_limit,
      NULL);

    sstrncpy(buf, outs, sizeof(buf));

    if (flags & PR_DISPLAY_FL_SEND_NOW) {
      /* Normally we use pr_response_add(), and let the response code
       * automatically handle all of the multiline response formatting.
       * However, some of the Display files are at times waiting for the
       * response chains to be flushed, which won't work (i.e. DisplayConnect
       * and DisplayQuit).
       */
      display_add_line(p, code, outs);

    } else {
      pr_response_add(code, "%s", outs);
    }
  }

  if (flags & PR_DISPLAY_FL_SEND_NOW) {
    display_flush_lines(p, code, flags);
  }

  destroy_pool(p);
  return 0;
}

int pr_display_fh(pr_fh_t *fh, const char *fs, const char *code, int flags) {
  if (!fh || !code) {
    errno = EINVAL;
    return -1;
  }

  return display_fh(fh, fs, code, flags);
}

int pr_display_file(const char *path, const char *fs, const char *code,
    int flags) {
  pr_fh_t *fh = NULL;
  int res, xerrno;

  if (!path || !code) {
    errno = EINVAL;
    return -1;
  }

  fh = pr_fsio_open_canon(path, O_RDONLY);
  if (fh == NULL)
    return -1;

  res = display_fh(fh, fs, code, flags);
  xerrno = errno;

  pr_fsio_close(fh);
 
  errno = xerrno;
  return res; 
}
