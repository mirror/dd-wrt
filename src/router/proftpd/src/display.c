/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2004-2009 The ProFTPD Project team
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

/*
 * Display of files
 * $Id: display.c,v 1.16 2009/12/02 03:59:28 castaglia Exp $
 */

#include "conf.h"

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

static int display_fh(pr_fh_t *fh, const char *fs, const char *code) {
  struct stat st;
  char buf[PR_TUNABLE_BUFFER_SIZE] = {'\0'};
  int len;
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
  char *first = NULL, *prev = NULL;
  int sent_first = FALSE;

  /* Stat the opened file to determine the optimal buffer size for IO. */
  memset(&st, 0, sizeof(st));
  pr_fsio_fstat(fh, &st);
  fh->fh_iosz = st.st_blksize;

#if defined(HAVE_STATFS) || defined(HAVE_SYS_STATVFS_H) || \
   defined(HAVE_SYS_VFS_H)
  fs_size = pr_fs_getsize((fs ? (char *) fs : (char *) fh->fh_path));
  snprintf(mg_size, sizeof(mg_size), "%" PR_LU, (pr_off_t) fs_size);
  format_size_str(mg_size_units, sizeof(mg_size_units), fs_size);
#else
  snprintf(mg_size, sizeof(mg_size), "%" PR_LU, (pr_off_t) fs_size);
  format_size_str(mg_size_units, sizeof(mg_size_units), fs_size);
#endif

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

    while(len && (buf[len-1] == '\r' || buf[len-1] == '\n')) {
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

    /* Handle the case where the Display file might contain only one
     * line.
     *
     * We _could_ just use pr_response_add(), and let the response code
     * automatically handle all of the multiline response formatting.
     * However, some of the Display files are at times waiting for the
     * response chains to be flushed won't work (e.g. login, logout).
     * Thus we have to deal with multiline files appropriately here.
     */

    if (first == NULL &&
        sent_first == FALSE) {
      first = pstrdup(p, outs);
      continue;
    }

    if (first) {
      pr_response_send_raw("%s-%s", code, first);
      first = NULL;
      sent_first = TRUE;

      prev = pstrdup(p, outs);
      continue;
    }

    if (prev) {
      if (MultilineRFC2228) {
        pr_response_send_raw("%s-%s", code, prev);

      } else {
        pr_response_send_raw(" %s", prev);
      }
    }

    prev = pstrdup(p, outs);
  }

  /* Do we have any remaining lines to send? */
  if (prev) {
    if (MultilineRFC2228) {
      pr_response_send_raw("%s-%s", code, prev);

    } else {
      pr_response_send_raw(" %s", prev);
    }
  }

  if (first != NULL) {
    if (session.auth_mech != NULL) {
      pr_response_send_raw("%s %s", code, first);

    } else {
      /* There is a special case if the client has not yet authenticated; it
       * means we are handling a DisplayConnect file.  The server will send
       * a banner as well, so we need to treat this is the start of a multiline
       * response.
       */
      pr_response_send_raw("%s-%s", code, first);
    }

  } else {
    if (prev) {
      if (MultilineRFC2228) {
        pr_response_send_raw("%s-%s", code, prev);

      } else {
        if (session.auth_mech != NULL) {
          /* Special case handling for when the client has not yet
           * authenticated (i.e. session.auth_mech is null).  This means
           * we're handling a DisplayConnect file.  The server will send
           * a banner string as well, thus if auth_mech is null, we do NOT
           * want to send this line; if not null, we DO want to send it.
           *
           * Without this check, we would end up sending an extra 220 response 
           * code to the client, which would confuse it.
           */
          pr_response_send_raw("%s %s", code, prev);
        }
      }
    }
  }

  destroy_pool(p);
  return 0;
}

int pr_display_fh(pr_fh_t *fh, const char *fs, const char *code) {
  if (!fh || !code) {
    errno = EINVAL;
    return -1;
  }

  return display_fh(fh, fs, code);
}

int pr_display_file(const char *path, const char *fs, const char *code) {
  pr_fh_t *fh = NULL;
  int res, xerrno;

  if (!path || !code) {
    errno = EINVAL;
    return -1;
  }

  fh = pr_fsio_open_canon(path, O_RDONLY);
  if (fh == NULL)
    return -1;

  res = display_fh(fh, fs, code);
  xerrno = errno;

  pr_fsio_close(fh);
 
  errno = xerrno;
  return res; 
}
