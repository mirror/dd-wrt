/*
 * ProFTPD: mod_redis -- a module for managing Redis data
 * Copyright (c) 2017 The ProFTPD Project
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
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * -----DO NOT EDIT BELOW THIS LINE-----
 * $Libraries: -lhiredis$
 */

#include "conf.h"
#include "privs.h"
#include "logfmt.h"
#include "json.h"

#define MOD_REDIS_VERSION		"mod_redis/0.1"

#if PROFTPD_VERSION_NUMBER < 0x0001030605
# error "ProFTPD 1.3.6rc5 or later required"
#endif

#include <hiredis/hiredis.h>

extern xaset_t *server_list;

module redis_module;

#define REDIS_DEFAULT_PORT		6379

static int redis_engine = FALSE;
static int redis_logfd = -1;
static pool *redis_pool = NULL;

static void redis_exit_ev(const void *, void *);
static int redis_sess_init(void);

static const char *trace_channel = "redis";

/* Logging
 */

#define REDIS_LOG_EVENT_FL_CONNECT		1
#define REDIS_LOG_EVENT_FL_DISCONNECT		2
#define REDIS_LOG_EVENT_FL_COMMAND		3

#define REDIS_EVENT_ALL				-1
#define REDIS_EVENT_CONNECT_ID			-2
#define REDIS_EVENT_DISCONNECT_ID		-3

/* The LogFormat "meta" values are in the unsigned char range; for our
 * specific "meta" values, then, choose something greater than 256.
 */
#define REDIS_META_CONNECT		427
#define REDIS_META_DISCONNECT		428

/* For tracking the size of deleted files. */
static off_t redis_dele_filesz = 0;

static pr_table_t *redis_field_idtab = NULL;

/* Entries in the field table identify the field name, and the data type:
 * Boolean, number, or string.
 *
 * Note: This idea of a table of names to JSON types is just what a core
 * JSON API for ProFTPD would include.
 */
struct field_info {
  unsigned int field_type;
  const char *field_name;
  size_t field_namelen;
};

/* Key comparison for the ID/name table. */
static int field_idcmp(const void *k1, size_t ksz1, const void *k2,
  size_t ksz2) {

  /* Return zero to indicate a match, non-zero otherwise. */
  return (*((unsigned int *) k1) == *((unsigned int *) k2) ? 0 : 1);
}

/* Key "hash" callback for ID/name table. */
static unsigned int field_idhash(const void *k, size_t ksz) {
  unsigned int c;
  unsigned int res;

  memcpy(&c, k, ksz);
  res = (c << 8);

  return res;
}

static int field_add(pool *p, unsigned int id, const char *name,
    unsigned int field_type) {
  unsigned int *k;
  struct field_info *fi;
  int res;

  k = palloc(p, sizeof(unsigned int));
  *k = id;

  fi = palloc(p, sizeof(struct field_info));
  fi->field_type = field_type;
  fi->field_name = name;
  fi->field_namelen = strlen(name) + 1;

  res = pr_table_kadd(redis_field_idtab, (const void *) k, sizeof(unsigned int),
    fi, sizeof(struct field_info *));
  return res;
}

static int make_fieldtab(pool *p) {
  int res, xerrno;

  redis_field_idtab = pr_table_alloc(p, 0);

  res = pr_table_ctl(redis_field_idtab, PR_TABLE_CTL_SET_KEY_CMP,
    (void *) field_idcmp);
  xerrno = errno;

  /* Since this function is only called once, at module startup, the logging
   * SHOULD use pr_log_pri().
   */

  if (res < 0) {
    pr_log_pri(PR_LOG_INFO, MOD_REDIS_VERSION
      ": error setting key comparison callback for field ID/names: %s",
      strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  res = pr_table_ctl(redis_field_idtab, PR_TABLE_CTL_SET_KEY_HASH,
    (void *) field_idhash);
  xerrno = errno;

  if (res < 0) {
    pr_log_pri(PR_LOG_INFO, MOD_REDIS_VERSION
      ": error setting key hash callback for field ID/names: %s",
      strerror(errno));

    errno = xerrno;
    return -1;
  }

  /* Now populate the table with the ID/name values.  The key is the
   * LogFormat "meta" ID, and the value is the corresponding name string,
   * for use e.g. as JSON object member names.
   */

  field_add(p, LOGFMT_META_BYTES_SENT, "bytes_sent", PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_FILENAME, "file", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_ENV_VAR, "ENV:", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_REMOTE_HOST, "remote_dns", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_REMOTE_IP, "remote_ip", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_IDENT_USER, "identd_user", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_PID, "pid", PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_TIME, "local_time", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_SECONDS, "transfer_secs", PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_COMMAND, "raw_command", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_LOCAL_NAME, "server_name", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_LOCAL_PORT, "local_port", PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_LOCAL_IP, "local_ip", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_LOCAL_FQDN, "server_dns", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_USER, "user", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_ORIGINAL_USER, "original_user", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_RESPONSE_CODE, "response_code", PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_CLASS, "connection_class", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_ANON_PASS, "anon_password", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_METHOD, "command", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_XFER_PATH, "transfer_path", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_DIR_NAME, "dir_name", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_DIR_PATH, "dir_path", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_CMD_PARAMS, "command_params", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_RESPONSE_STR, "response_msg", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_PROTOCOL, "protocol", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_VERSION, "server_version", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_RENAME_FROM, "rename_from", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_FILE_MODIFIED, "file_modified", PR_JSON_TYPE_BOOL);
  field_add(p, LOGFMT_META_UID, "uid", PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_GID, "gid", PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_RAW_BYTES_IN, "session_bytes_rcvd",
    PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_RAW_BYTES_OUT, "session_bytes_sent",
    PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_EOS_REASON, "session_end_reason",
    PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_VHOST_IP, "server_ip", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_NOTE_VAR, "NOTE:", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_XFER_STATUS, "transfer_status", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_XFER_FAILURE, "transfer_failure",
    PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_MICROSECS, "microsecs", PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_MILLISECS, "millisecs", PR_JSON_TYPE_NUMBER);
  field_add(p, LOGFMT_META_ISO8601, "timestamp", PR_JSON_TYPE_STRING);
  field_add(p, LOGFMT_META_GROUP, "group", PR_JSON_TYPE_STRING);

  field_add(p, REDIS_META_CONNECT, "connecting", PR_JSON_TYPE_BOOL);
  field_add(p, REDIS_META_DISCONNECT, "disconnecting", PR_JSON_TYPE_BOOL);

  return 0;
}

static void encode_json(pool *p, void *json, const char *field_name,
    size_t field_namelen, unsigned int field_type, const void *field_value) {

  switch (field_type) {
    case PR_JSON_TYPE_STRING:
      (void) pr_json_object_set_string(p, json, field_name,
        (const char *) field_value);
      break;

    case PR_JSON_TYPE_NUMBER:
      (void) pr_json_object_set_number(p, json, field_name,
        *((double *) field_value));
      break;

    case PR_JSON_TYPE_BOOL:
      (void) pr_json_object_set_bool(p, json, field_name,
        *((int *) field_value));
      break;

    default:
      (void) pr_log_writefile(redis_logfd, MOD_REDIS_VERSION,
        "unsupported JSON field type: %u", field_type);
  }
}

static char *get_meta_arg(pool *p, unsigned char *meta, size_t *arg_len) {
  char buf[PR_TUNABLE_PATH_MAX+1], *ptr;
  size_t len;

  ptr = buf;
  len = 0;

  while (*meta != LOGFMT_META_ARG_END) {
    pr_signals_handle();
    *ptr++ = (char) *meta++;
    len++;
  }

  *ptr = '\0';
  *arg_len = len;

  return pstrdup(p, buf);
}

static const char *get_meta_dir_name(cmd_rec *cmd) {
  const char *dir_name = NULL;
  pool *p;

  p = cmd->tmp_pool;

  if (pr_cmd_cmp(cmd, PR_CMD_CDUP_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_CWD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_LIST_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_MLSD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_MKD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_NLST_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_RMD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_XCWD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_XCUP_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_XMKD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_XRMD_ID) == 0) {
    char *path, *ptr;

    path = pr_fs_decode_path(p, cmd->arg);
    ptr = strrchr(path, '/');

    if (ptr != NULL) {
      if (ptr != path) {
        dir_name = ptr + 1;

      } else if (*(ptr + 1) != '\0') {
        dir_name = ptr + 1;

      } else {
        dir_name = path;
      }

    } else {
      dir_name = path;
    }

  } else {
    dir_name = pr_fs_getvwd();
  }

  return dir_name;
}

static const char *get_meta_dir_path(cmd_rec *cmd) {
  const char *dir_path = NULL;
  pool *p;

  p = cmd->tmp_pool;

  if (pr_cmd_cmp(cmd, PR_CMD_CDUP_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_LIST_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_MLSD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_MKD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_NLST_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_RMD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_XCUP_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_XMKD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_XRMD_ID) == 0) {
    dir_path = dir_abs_path(p, pr_fs_decode_path(p, cmd->arg), TRUE);

  } else if (pr_cmd_cmp(cmd, PR_CMD_CWD_ID) == 0 ||
             pr_cmd_cmp(cmd, PR_CMD_XCWD_ID) == 0) {

    /* Note: by this point in the dispatch cycle, the current working
     * directory has already been changed.  For the CWD/XCWD commands, this
     * means that dir_abs_path() may return an improper path, with the target
     * directory being reported twice.  To deal with this, do not use
     * dir_abs_path(), and use pr_fs_getvwd()/pr_fs_getcwd() instead.
     */

    if (session.chroot_path != NULL) {
      /* Chrooted session. */
      if (strncmp(pr_fs_getvwd(), "/", 2) == 0) {
        dir_path = session.chroot_path;

      } else {
        dir_path = pdircat(p, session.chroot_path, pr_fs_getvwd(), NULL);
      }

    } else {
      /* Non-chrooted session. */
      dir_path = pr_fs_getcwd();
    }
  }

  return dir_path;
}

static const char *get_meta_filename(cmd_rec *cmd) {
  const char *filename = NULL;
  pool *p;

  p = cmd->tmp_pool;

  if (pr_cmd_cmp(cmd, PR_CMD_RNTO_ID) == 0) {
    filename = dir_abs_path(p, pr_fs_decode_path(p, cmd->arg), TRUE);

  } else if (pr_cmd_cmp(cmd, PR_CMD_RETR_ID) == 0) {
    const char *path;

    path = pr_table_get(cmd->notes, "mod_xfer.retr-path", NULL);
    if (path != NULL) {
      filename = dir_abs_path(p, path, TRUE);
    }

  } else if (pr_cmd_cmp(cmd, PR_CMD_APPE_ID) == 0 ||
             pr_cmd_cmp(cmd, PR_CMD_STOR_ID) == 0) {
    const char *path;

    path = pr_table_get(cmd->notes, "mod_xfer.store-path", NULL);
    if (path != NULL) {
      filename = dir_abs_path(p, path, TRUE);
    }

  } else if (session.xfer.p != NULL &&
             session.xfer.path != NULL) {
    filename = dir_abs_path(p, session.xfer.path, TRUE);

  } else if (pr_cmd_cmp(cmd, PR_CMD_CDUP_ID) == 0 ||
             pr_cmd_cmp(cmd, PR_CMD_PWD_ID) == 0 ||
             pr_cmd_cmp(cmd, PR_CMD_XCUP_ID) == 0 ||
             pr_cmd_cmp(cmd, PR_CMD_XPWD_ID) == 0) {
    filename = dir_abs_path(p, pr_fs_getcwd(), TRUE);

  } else if (pr_cmd_cmp(cmd, PR_CMD_CWD_ID) == 0 ||
             pr_cmd_cmp(cmd, PR_CMD_XCWD_ID) == 0) {

    /* Note: by this point in the dispatch cycle, the current working
     * directory has already been changed.  For the CWD/XCWD commands, this
     * means that dir_abs_path() may return an improper path, with the target
     * directory being reported twice.  To deal with this, do not use
     * dir_abs_path(), and use pr_fs_getvwd()/pr_fs_getcwd() instead.
     */
    if (session.chroot_path != NULL) {
      /* Chrooted session. */
      if (strncmp(pr_fs_getvwd(), "/", 2) == 0) {
        filename = session.chroot_path;

      } else {
        filename = pdircat(p, session.chroot_path, pr_fs_getvwd(), NULL);
      }

    } else {
      /* Non-chrooted session. */
      filename = pr_fs_getcwd();
    }

  } else if (pr_cmd_cmp(cmd, PR_CMD_SITE_ID) == 0 &&
             (strncasecmp(cmd->argv[1], "CHGRP", 6) == 0 ||
              strncasecmp(cmd->argv[1], "CHMOD", 6) == 0 ||
              strncasecmp(cmd->argv[1], "UTIME", 6) == 0)) {
    register unsigned int i;
    char *ptr = "";

    for (i = 3; i <= cmd->argc-1; i++) {
      ptr = pstrcat(p, ptr, *ptr ? " " : "",
        pr_fs_decode_path(p, cmd->argv[i]), NULL);
    }

    filename = dir_abs_path(p, ptr, TRUE);

  } else {
    /* Some commands (i.e. DELE, MKD, RMD, XMKD, and XRMD) have associated
     * filenames that are not stored in the session.xfer structure; these
     * should be expanded properly as well.
     */
    if (pr_cmd_cmp(cmd, PR_CMD_DELE_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_LIST_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_MDTM_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_MKD_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_MLSD_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_MLST_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_NLST_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_RMD_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_XMKD_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_XRMD_ID) == 0) {
      filename = dir_abs_path(p, pr_fs_decode_path(p, cmd->arg), TRUE);

    } else if (pr_cmd_cmp(cmd, PR_CMD_MFMT_ID) == 0) {
      /* MFMT has, as its filename, the second argument. */
      filename = dir_abs_path(p, pr_fs_decode_path(p, cmd->argv[2]), TRUE);
    }
  }

  return filename;
}

static const char *get_meta_transfer_failure(cmd_rec *cmd) {
  const char *transfer_failure = NULL;

  /* If the current command is one that incurs a data transfer, then we
   * need to do more work.  If not, it's an easy substitution.
   */
  if (pr_cmd_cmp(cmd, PR_CMD_APPE_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_LIST_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_MLSD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_NLST_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_RETR_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_STOR_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_STOU_ID) == 0) {
    const char *proto;

    proto = pr_session_get_protocol(0);

    if (strncmp(proto, "ftp", 4) == 0 ||
        strncmp(proto, "ftps", 5) == 0) {

      if (!(XFER_ABORTED)) {
        int res;
        const char *resp_code = NULL, *resp_msg = NULL;

        /* Get the last response code/message.  We use heuristics here to
         * determine when to use "failed" versus "success".
         */
        res = pr_response_get_last(cmd->tmp_pool, &resp_code, &resp_msg);
        if (res == 0 &&
            resp_code != NULL) {
          if (*resp_code != '2' &&
              *resp_code != '1') {
            char *ptr;

            /* Parse out/prettify the resp_msg here */
            ptr = strchr(resp_msg, '.');
            if (ptr != NULL) {
              transfer_failure = ptr + 2;

            } else {
              transfer_failure = resp_msg;
            }
          }
        }
      }
    }
  }

  return transfer_failure;
}

static const char *get_meta_transfer_path(cmd_rec *cmd) {
  const char *transfer_path = NULL;

  if (pr_cmd_cmp(cmd, PR_CMD_RNTO_ID) == 0) {
    transfer_path = dir_best_path(cmd->tmp_pool,
      pr_fs_decode_path(cmd->tmp_pool, cmd->arg));

  } else if (session.xfer.p != NULL &&
             session.xfer.path != NULL) {
    transfer_path = session.xfer.path;

  } else {
    /* Some commands (i.e. DELE, MKD, XMKD, RMD, XRMD) have associated
     * filenames that are not stored in the session.xfer structure; these
     * should be expanded properly as well.
     */
    if (pr_cmd_cmp(cmd, PR_CMD_DELE_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_MKD_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_XMKD_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_RMD_ID) == 0 ||
        pr_cmd_cmp(cmd, PR_CMD_XRMD_ID) == 0) {
      transfer_path = dir_best_path(cmd->tmp_pool,
        pr_fs_decode_path(cmd->tmp_pool, cmd->arg));
    }
  }

  return transfer_path;
}

static int get_meta_transfer_secs(cmd_rec *cmd, double *transfer_secs) {
  if (session.xfer.p == NULL) {
    return -1;
  }

  /* Make sure that session.xfer.start_time actually has values (which is
   * not always the case).
   */
  if (session.xfer.start_time.tv_sec != 0 ||
      session.xfer.start_time.tv_usec != 0) {
    struct timeval end_time;

    gettimeofday(&end_time, NULL);
    end_time.tv_sec -= session.xfer.start_time.tv_sec;

    if (end_time.tv_usec >= session.xfer.start_time.tv_usec) {
      end_time.tv_usec -= session.xfer.start_time.tv_usec;

    } else {
      end_time.tv_usec = 1000000L - (session.xfer.start_time.tv_usec -
        end_time.tv_usec);
      end_time.tv_sec--;
    }

    *transfer_secs = end_time.tv_sec;
    *transfer_secs += (double) ((double) end_time.tv_usec / (double) 1000);

    return 0;
  }

  return -1;
}

static const char *get_meta_transfer_status(cmd_rec *cmd) {
  const char *transfer_status = NULL;

  /* If the current command is one that incurs a data transfer, then we need
   * to do more work.  If not, it's an easy substitution.
   */
  if (pr_cmd_cmp(cmd, PR_CMD_ABOR_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_APPE_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_LIST_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_MLSD_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_NLST_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_RETR_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_STOR_ID) == 0 ||
      pr_cmd_cmp(cmd, PR_CMD_STOU_ID) == 0) {
    const char *proto;

    proto = pr_session_get_protocol(0);

    if (strncmp(proto, "ftp", 4) == 0 ||
        strncmp(proto, "ftps", 5) == 0) {
      if (!(XFER_ABORTED)) {
        int res;
        const char *resp_code = NULL, *resp_msg = NULL;

        /* Get the last response code/message.  We use heuristics here to
         * determine when to use "failed" versus "success".
         */
        res = pr_response_get_last(cmd->tmp_pool, &resp_code, &resp_msg);
        if (res == 0 &&
            resp_code != NULL) {
          if (*resp_code == '2') {
            if (pr_cmd_cmp(cmd, PR_CMD_ABOR_ID) != 0) {
              transfer_status = "success";

            } else {
              /* We're handling the ABOR command, so obviously the value
               * should be 'cancelled'.
               */
              transfer_status = "cancelled";
            }

          } else if (*resp_code == '1') {

            /* If the first digit of the response code is 1, then the
             * response code (for a data transfer command) is probably 150,
             * which means that the transfer was still in progress (didn't
             * complete with a 2xx/4xx response code) when we are called here,
             * which in turn means a timeout kicked in.
             */

            transfer_status = "timeout";

          } else {
            transfer_status = "failed";
          }

        } else {
          transfer_status = "success";
        }

      } else {
        transfer_status = "cancelled";
      }

    } else {
      /* mod_sftp stashes a note for us in the command notes if the transfer
       * failed.
       */
      const char *sftp_status;

      sftp_status = pr_table_get(cmd->notes, "mod_sftp.file-status", NULL);
      if (sftp_status == NULL) {
        transfer_status = "success";

      } else {
        transfer_status = "failed";
      }
    }
  }

  return transfer_status;
}

static int find_next_meta(pool *p, cmd_rec *cmd, int flags,
    unsigned char **log_fmt, void *obj,
    void (*encode_field)(pool *, void *, const char *, size_t, unsigned int,
      const void *)) {
  const struct field_info *fi;
  unsigned char *m;
  unsigned int meta;

  m = (*log_fmt) + 1;

  meta = *m;
  fi = pr_table_kget(redis_field_idtab, (const void *) &meta,
    sizeof(unsigned int), NULL);

  switch (meta) {
    case LOGFMT_META_BYTES_SENT: {
      if (session.xfer.p != NULL) {
        double bytes_sent;

        bytes_sent = session.xfer.total_bytes;
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          &bytes_sent);

      } else if (pr_cmd_cmp(cmd, PR_CMD_DELE_ID) == 0) {
        double bytes_sent;

        bytes_sent = redis_dele_filesz;
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          &bytes_sent);
      }

      m++;
      break;
    }

    case LOGFMT_META_FILENAME: {
      const char *filename;

      filename = get_meta_filename(cmd);
      if (filename != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          filename);
      }

      m++;
      break;
    }

    case LOGFMT_META_ENV_VAR: {
      m++;

      if (*m == LOGFMT_META_START &&
          *(m + 1) == LOGFMT_META_ARG) {
        char *key, *env = NULL;
        size_t key_len = 0;

        key = get_meta_arg(p, (m + 2), &key_len);
        m += key_len;

        env = pr_env_get(p, key);
        if (env != NULL) {
          char *field_name;
          size_t field_namelen;

          field_name = pstrcat(p, fi->field_name, key, NULL);
          field_namelen = strlen(field_name);

          encode_field(p, obj, field_name, field_namelen, fi->field_type, env);
        }
      }

      break;
    }

    case LOGFMT_META_REMOTE_HOST: {
      const char *name;

      name = pr_netaddr_get_sess_remote_name();
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        name);

      m++;
      break;
    }

    case LOGFMT_META_REMOTE_IP: {
      const char *ipstr;

      ipstr = pr_netaddr_get_ipstr(pr_netaddr_get_sess_local_addr());
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        ipstr);

      m++;
      break;
    }

    case LOGFMT_META_IDENT_USER: {
      const char *ident_user;

      ident_user = pr_table_get(session.notes, "mod_ident.rfc1413-ident", NULL);
      if (ident_user != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          ident_user);
      }

      m++;
      break;
    }

    case LOGFMT_META_PID: {
      double sess_pid;

      sess_pid = session.pid;
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        &sess_pid);

      m++;
      break;
    }

    case LOGFMT_META_TIME: {
      char *time_fmt = "%Y-%m-%d %H:%M:%S %z", ts[128];
      struct tm *tm;
      time_t now;

      m++;

      now = time(NULL);
      tm = pr_gmtime(NULL, &now);

      if (*m == LOGFMT_META_START &&
          *(m+1) == LOGFMT_META_ARG) {
        size_t fmt_len = 0;

        time_fmt = get_meta_arg(p, (m + 2), &fmt_len);
      }

      strftime(ts, sizeof(ts)-1, time_fmt, tm);
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        ts);

      break;
    }

    case LOGFMT_META_SECONDS: {
      double transfer_secs;

      if (get_meta_transfer_secs(cmd, &transfer_secs) == 0) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          &transfer_secs);
      }

      m++;
      break;
    }

    case LOGFMT_META_COMMAND: {
      if (pr_cmd_cmp(cmd, PR_CMD_PASS_ID) == 0 &&
          session.hide_password) {
        const char *full_cmd = "PASS (hidden)";

        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          full_cmd);

      } else if (pr_cmd_cmp(cmd, PR_CMD_ADAT_ID) == 0) {
        const char *full_cmd = "ADAT (hidden)";

        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          full_cmd);

      } else if (flags == REDIS_LOG_EVENT_FL_COMMAND) {
        const char *full_cmd;

        full_cmd = get_full_cmd(cmd);
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          full_cmd);
      }

      m++;
      break;
    }

    case LOGFMT_META_LOCAL_NAME: {
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        cmd->server->ServerName);
      m++;
      break;
    }

    case LOGFMT_META_LOCAL_PORT: {
      double server_port;

      server_port = cmd->server->ServerPort;
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        &server_port);

      m++;
      break;
    }

    case LOGFMT_META_LOCAL_IP: {
      const char *ipstr;

      ipstr = pr_netaddr_get_ipstr(pr_netaddr_get_sess_local_addr());
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        ipstr);

      m++;
      break;
    }

    case LOGFMT_META_LOCAL_FQDN: {
      const char *dnsstr;

      dnsstr = pr_netaddr_get_dnsstr(pr_netaddr_get_sess_local_addr());
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        dnsstr);

      m++;
      break;
    }

    case LOGFMT_META_USER: {
      if (session.user != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          session.user);
      }

      m++;
      break;
    }

    case LOGFMT_META_ORIGINAL_USER: {
      const char *orig_user = NULL;

      orig_user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);
      if (orig_user != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          orig_user);
      }

      m++;
      break;
    }

    case LOGFMT_META_RESPONSE_CODE: {
      const char *resp_code = NULL;
      int res;

      res = pr_response_get_last(cmd->tmp_pool, &resp_code, NULL);
      if (res == 0 &&
          resp_code != NULL) {
        double code;

        code = atoi(resp_code);
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          &code);

      /* Hack to add return code for proper logging of QUIT command. */
      } else if (pr_cmd_cmp(cmd, PR_CMD_QUIT_ID) == 0) {
        double code = 221;

        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          &code);
      }

      m++;
      break;
    }

    case LOGFMT_META_CLASS: {
      if (session.conn_class != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          session.conn_class);
      }

      m++;
      break;
    }

    case LOGFMT_META_ANON_PASS: {
      const char *anon_pass;

      anon_pass = pr_table_get(session.notes, "mod_auth.anon-passwd", NULL);
      if (anon_pass == NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          anon_pass);
      }

      m++;
      break;
    }

    case LOGFMT_META_METHOD: {
      if (flags == REDIS_LOG_EVENT_FL_COMMAND) {
        if (pr_cmd_cmp(cmd, PR_CMD_SITE_ID) != 0) {
          encode_field(p, obj, fi->field_name, fi->field_namelen,
            fi->field_type, cmd->argv[0]);

        } else {
          char buf[32], *ptr;

          /* Make sure that the SITE command used is all in uppercase,
           * for logging purposes.
           */
          for (ptr = cmd->argv[1]; *ptr; ptr++) {
            *ptr = toupper((int) *ptr);
          }

          memset(buf, '\0', sizeof(buf));
          snprintf(buf, sizeof(buf)-1, "%s %s", (char *) cmd->argv[0],
            (char *) cmd->argv[1]);

          encode_field(p, obj, fi->field_name, fi->field_namelen,
            fi->field_type, buf);
        }
      }

      m++;
      break;
    }

    case LOGFMT_META_XFER_PATH: {
      const char *transfer_path;

      transfer_path = get_meta_transfer_path(cmd);
      if (transfer_path != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          transfer_path);
      }

      m++;
      break;
    }

    case LOGFMT_META_DIR_NAME: {
      const char *dir_name;

      dir_name = get_meta_dir_name(cmd);
      if (dir_name != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          dir_name);
      }

      m++;
      break;
    }

    case LOGFMT_META_DIR_PATH: {
      const char *dir_path;

      dir_path = get_meta_dir_path(cmd);
      if (dir_path != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          dir_path);
      }

      m++;
      break;
    }

    case LOGFMT_META_CMD_PARAMS: {
      if (pr_cmd_cmp(cmd, PR_CMD_ADAT_ID) == 0 ||
          pr_cmd_cmp(cmd, PR_CMD_PASS_ID) == 0) {
        const char *params = "(hidden)";

        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          params);

      } else if (REDIS_LOG_EVENT_FL_COMMAND &&
                 cmd->argc > 1) {
        const char *params;

        params = pr_fs_decode_path(p, cmd->arg);
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          params);
      }

      m++;
      break;
    }

    case LOGFMT_META_RESPONSE_STR: {
      const char *resp_msg = NULL;
      int res;

      res = pr_response_get_last(p, NULL, &resp_msg);
      if (res == 0 &&
          resp_msg != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          resp_msg);
      }

      m++;
      break;
    }

    case LOGFMT_META_PROTOCOL: {
      const char *proto;

      proto = pr_session_get_protocol(0);
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        proto);

      m++;
      break;
    }

    case LOGFMT_META_VERSION: {
      const char *version;

      version = PROFTPD_VERSION_TEXT;
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        version);

      m++;
      break;
    }

    case LOGFMT_META_RENAME_FROM: {
      if (pr_cmd_cmp(cmd, PR_CMD_RNTO_ID) == 0) {
        const char *rnfr_path;

        rnfr_path = pr_table_get(session.notes, "mod_core.rnfr-path", NULL);
        if (rnfr_path != NULL) {
          encode_field(p, obj, fi->field_name, fi->field_namelen,
            fi->field_type, rnfr_path);
        }
      }

      m++;
      break;
    }

    case LOGFMT_META_FILE_MODIFIED: {
      int modified = FALSE;
      const char *val;

      val = pr_table_get(cmd->notes, "mod_xfer.file-modified", NULL);
      if (val != NULL) {
        if (strncmp(val, "true", 5) == 0) {
          modified = TRUE;
        }
      }

      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        &modified);

      m++;
      break;
    }

    case LOGFMT_META_UID: {
      double sess_uid;

      sess_uid = session.login_uid;
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        &sess_uid);

      m++;
      break;
    }

    case LOGFMT_META_GID: {
      double sess_gid;

      sess_gid = session.login_gid;
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        &sess_gid);

      m++;
      break;
    }

    case LOGFMT_META_RAW_BYTES_IN: {
      double bytes_rcvd;

      bytes_rcvd = session.total_raw_in;
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        &bytes_rcvd);

      m++;
      break;
    }

    case LOGFMT_META_RAW_BYTES_OUT: {
      double bytes_sent;

      bytes_sent = session.total_raw_out;
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        &bytes_sent);

      m++;
      break;
    }

    case LOGFMT_META_EOS_REASON: {
      const char *reason = NULL, *details = NULL;

      reason = pr_session_get_disconnect_reason(&details);
      if (reason != NULL) {
        if (details != NULL) {
          char buf[256];

          memset(buf, '\0', sizeof(buf));
          snprintf(buf, sizeof(buf)-1, "%s: %s", reason, details);
          encode_field(p, obj, fi->field_name, fi->field_namelen,
            fi->field_type, buf);

        } else {
          encode_field(p, obj, fi->field_name, fi->field_namelen,
            fi->field_type, reason);
        }
      }

      m++;
      break;
    }

    case LOGFMT_META_VHOST_IP:
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        cmd->server->ServerAddress);
      m++;
      break;

    case LOGFMT_META_NOTE_VAR: {
      m++;

      if (*m == LOGFMT_META_START &&
          *(m + 1) == LOGFMT_META_ARG) {
        const char *note = NULL;
        char *key;
        size_t key_len = 0;

        key = get_meta_arg(p, (m + 2), &key_len);
        m += key_len;

        /* Check in the cmd->notes table first. */
        note = pr_table_get(cmd->notes, key, NULL);
        if (note == NULL) {
          /* If not there, check in the session.notes table. */
          note = pr_table_get(session.notes, key, NULL);
        }

        if (note != NULL) {
          char *field_name;
          size_t field_namelen;

          field_name = pstrcat(p, fi->field_name, note, NULL);
          field_namelen = strlen(field_name);

          encode_field(p, obj, field_name, field_namelen, fi->field_type, note);
        }
      }

      break;
    }

    case LOGFMT_META_XFER_STATUS: {
      const char *transfer_status;

      transfer_status = get_meta_transfer_status(cmd);
      if (transfer_status != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          transfer_status);
      }

      m++;
      break;
    }

    case LOGFMT_META_XFER_FAILURE: {
      const char *transfer_failure;

      transfer_failure = get_meta_transfer_failure(cmd);
      if (transfer_failure != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          transfer_failure);
      }

      m++;
      break;
    }

    case LOGFMT_META_MICROSECS: {
      double sess_usecs;
      struct timeval now;

      gettimeofday(&now, NULL);
      sess_usecs = now.tv_usec;
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        &sess_usecs);

      m++;
      break;
    }

    case LOGFMT_META_MILLISECS: {
      double sess_msecs;
      struct timeval now;

      gettimeofday(&now, NULL);

      /* Convert microsecs to millisecs. */
      sess_msecs = (now.tv_usec / 1000);

      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        &sess_msecs);

      m++;
      break;
    }

    case LOGFMT_META_ISO8601: {
      char ts[128];
      struct tm *tm;
      struct timeval now;
      unsigned long millis;
      size_t len;

      gettimeofday(&now, NULL);
      tm = pr_localtime(NULL, (const time_t *) &(now.tv_sec));

      len = strftime(ts, sizeof(ts)-1, "%Y-%m-%d %H:%M:%S", tm);

      /* Convert microsecs to millisecs. */
      millis = now.tv_usec / 1000;

      snprintf(ts + len, sizeof(ts) - len - 1, ",%03lu", millis);
      encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
        ts);

      m++;
      break;
    }

    case LOGFMT_META_GROUP: {
      if (session.group != NULL) {
        encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
          session.group);
      }
      m++;
      break;
    }

    default:
      (void) pr_log_writefile(redis_logfd, MOD_REDIS_VERSION,
        "skipping unsupported LogFormat meta %u", meta);
      break;
  }

  *log_fmt = m;
  return 0;
}

static int encode_fields(pool *p, cmd_rec *cmd, int flags,
    unsigned char *log_fmt, void *obj,
    void (*encode_field)(pool *, void *, const char *, size_t, unsigned int,
      const void *)) {

  if (flags == REDIS_LOG_EVENT_FL_CONNECT &&
      session.prev_server == NULL) {
    unsigned int meta = REDIS_META_CONNECT;
    const struct field_info *fi;
    int connecting = TRUE;

    fi = pr_table_kget(redis_field_idtab, (const void *) &meta,
      sizeof(unsigned int), NULL);

    encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
      &connecting);

  } else if (flags == REDIS_LOG_EVENT_FL_DISCONNECT) {
    unsigned int meta = REDIS_META_DISCONNECT;
    const struct field_info *fi;
    int disconnecting = TRUE;

    fi = pr_table_kget(redis_field_idtab, (const void *) &meta,
      sizeof(unsigned int), NULL);

    encode_field(p, obj, fi->field_name, fi->field_namelen, fi->field_type,
      &disconnecting);
  }

  while (*log_fmt) {
    pr_signals_handle();

    if (*log_fmt == LOGFMT_META_START) {
      find_next_meta(cmd->tmp_pool, cmd, flags, &log_fmt, obj, encode_field);

    } else {
      log_fmt++;
    }
  }

  return 0;
}

static int encode_log_fmt(pool *p, cmd_rec *cmd, int flags,
    unsigned char *log_fmt, char **payload, size_t *payload_len) {
  pr_json_object_t *json;
  char *text = NULL;

  json = pr_json_object_alloc(p);
  (void) encode_fields(p, cmd, flags, log_fmt, json, encode_json);

  text = pr_json_object_to_text(p, json, "");
  pr_trace_msg(trace_channel, 3, "generated JSON payload: %s", text);

  *payload_len = strlen(text);
  *payload = text;

  pr_json_object_free(json);
  return 0;
}

static int is_loggable_event(array_header *logged_events, cmd_rec *cmd,
    int flags) {
  register unsigned int i;
  int *event_ids, loggable = FALSE;

  event_ids = logged_events->elts;

  for (i = 0; i < logged_events->nelts; i++) {
    switch (event_ids[i]) {
      case REDIS_EVENT_ALL:
        loggable = TRUE;
         break;

      case REDIS_EVENT_CONNECT_ID:
        if (flags == REDIS_LOG_EVENT_FL_CONNECT) {
          loggable = TRUE;
        }
        break;

      case REDIS_EVENT_DISCONNECT_ID:
        if (flags == REDIS_LOG_EVENT_FL_DISCONNECT) {
          loggable = TRUE;
        }
        break;

      default:
        if (flags == REDIS_LOG_EVENT_FL_COMMAND &&
            pr_cmd_cmp(cmd, event_ids[i]) == 0) {
          loggable = TRUE;
        }
        break;
    }
  }

  return loggable;
}

static void log_event(cmd_rec *cmd, int flags) {
  pr_redis_t *redis;
  config_rec *c;

  redis = pr_redis_conn_get(session.pool);
  if (redis == NULL) {
    (void) pr_log_writefile(redis_logfd, MOD_REDIS_VERSION,
      "error connecting to Redis: %s", strerror(errno));
    return;
  }

  c = find_config(main_server->conf, CONF_PARAM, "RedisLogOnCommand", FALSE);
  while (c != NULL) {
    int res;
    array_header *logged_events;
    const char *fmt_name = NULL;
    char *payload = NULL;
    size_t payload_len = 0;
    unsigned char *log_fmt;

    pr_signals_handle();

    logged_events = c->argv[0];

    if (is_loggable_event(logged_events, cmd, flags) == FALSE) {
      c = find_config_next(c, c->next, CONF_PARAM, "RedisLogOnCommand", FALSE);
      continue;
    }

    fmt_name = c->argv[1];
    log_fmt = c->argv[2];
    res = encode_log_fmt(cmd->tmp_pool, cmd, flags, log_fmt, &payload,
      &payload_len);
    if (res < 0) {
      (void) pr_log_writefile(redis_logfd, MOD_REDIS_VERSION,
        "error generating JSON formatted log message: %s", strerror(errno));
      payload = NULL;
      payload_len = 0;

    } else {
      res = pr_redis_list_append(redis, &redis_module, fmt_name, payload,
        payload_len);
      if (res < 0) {
        (void) pr_log_writefile(redis_logfd, MOD_REDIS_VERSION,
          "error appending log message to '%s': %s", log_fmt, strerror(errno));
      }
    }

    c = find_config_next(c, c->next, CONF_PARAM, "RedisLogOnCommand", FALSE);
  }
}

/* Configuration handlers
 */

/* usage: RedisEngine on|off */
MODRET set_redisengine(cmd_rec *cmd) {
  int engine = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  engine = get_boolean(cmd, 1);
  if (engine == -1) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = engine;

  return PR_HANDLED(cmd);
}

/* usage: RedisLog path|"none" */
MODRET set_redislog(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcasecmp(cmd->argv[1], "none") != 0 &&
      pr_fs_valid_path(cmd->argv[1]) < 0) {
    CONF_ERROR(cmd, "must be an absolute path");
  }

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* XXX Would be nice to a pr_str_csv2array() function */
static array_header *csv2array(pool *p, char *csv) {
  array_header *event_names;
  char *ptr, *name;

  event_names = make_array(p, 1, sizeof(char *));

  ptr = csv;
  name = pr_str_get_word(&ptr, 0);
  while (name != NULL) {
    pr_signals_handle();

    *((char **) push_array(event_names)) = pstrdup(p, name);

    /* Skip commas. */
    while (*ptr == ',') {
      ptr++;
    }

    name = pr_str_get_word(&ptr, 0);
  }

  return event_names;
}

static array_header *event_names2ids(pool *p, const char *directive,
    array_header *event_names) {
  register unsigned int i;
  array_header *event_ids;

  event_ids = make_array(p, event_names->nelts, sizeof(int));
  for (i = 0; i < event_names->nelts; i++) {
    const char *name;
    int event_id, valid = TRUE;

    name = ((const char **) event_names->elts)[i];

    event_id = pr_cmd_get_id(name);
    if (event_id < 0) {
      if (strcmp(name, "ALL") == 0) {
        event_id = REDIS_EVENT_ALL;

      } else if (strcmp(name, "CONNECT") == 0) {
        event_id = REDIS_EVENT_CONNECT_ID;

      } else if (strcmp(name, "DISCONNECT") == 0) {
        event_id = REDIS_EVENT_DISCONNECT_ID;

      } else {
        pr_log_debug(DEBUG0, "%s: skipping unsupported event '%s'", directive,
          name);
        valid = FALSE;
      }
    }

    if (valid == TRUE) {
      *((int *) push_array(event_ids)) = event_id;
    }
  }

  return event_ids;
}

/* usage: RedisLogOnCommand commands log-fmt */
MODRET set_redislogoncommand(cmd_rec *cmd) {
  config_rec *c;
  const char *fmt_name;
  unsigned char *log_fmt = NULL;
  array_header *event_names;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_GLOBAL|CONF_VIRTUAL);

  event_names = csv2array(cmd->tmp_pool, cmd->argv[1]);
  fmt_name = cmd->argv[2];

  /* Make sure that the given LogFormat name is known. */
  c = find_config(cmd->server->conf, CONF_PARAM, "LogFormat", FALSE);
  while (c != NULL) {
    if (strcmp(fmt_name, c->argv[0]) == 0) {
      log_fmt = c->argv[1];
      break;
    }

    c = find_config_next(c, c->next, CONF_PARAM, "LogFormat", FALSE);
  }

  if (log_fmt == NULL) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "no LogFormat '", fmt_name,
      "' configured", NULL));
  }

  c = add_config_param(cmd->argv[0], 4, NULL, NULL, NULL);
  c->argv[0] = event_names2ids(c->pool, cmd->argv[0], event_names);
  c->argv[1] = pstrdup(c->pool, fmt_name);
  c->argv[2] = log_fmt;

  return PR_HANDLED(cmd);
}

/* usage: RedisServer host[:port] [password] */
MODRET set_redisserver(cmd_rec *cmd) {
  config_rec *c;
  char *server, *password = NULL, *ptr;
  size_t server_len;
  int ctx, port = REDIS_DEFAULT_PORT;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  server = pstrdup(cmd->tmp_pool, cmd->argv[1]);
  server_len = strlen(server);

  ptr = strrchr(server, ':');
  if (ptr != NULL) {
    /* We also need to check for IPv6 addresses, e.g. "[::1]" or "[::1]:6379",
     * before assuming that the text following our discovered ':' is indeed
     * a port number.
     */

    if (*server == '[') {
      if (*(ptr-1) == ']') {
        /* We have an IPv6 address with an explicit port number. */
        server = pstrndup(cmd->tmp_pool, server + 1, (ptr - 1) - (server + 1));
        *ptr = '\0';
        port = atoi(ptr + 1);

      } else if (server[server_len-1] == ']') {
        /* We have an IPv6 address without an explicit port number. */
        server = pstrndup(cmd->tmp_pool, server + 1, server_len - 2);
        port = REDIS_DEFAULT_PORT;
      }

    } else {
      *ptr = '\0';
      port = atoi(ptr + 1);
    }
  }

  if (cmd->argc == 3) {
    password = cmd->argv[2];
  }

  c = add_config_param(cmd->argv[0], 3, NULL, NULL, NULL);
  c->argv[0] = pstrdup(c->pool, server);
  c->argv[1] = palloc(c->pool, sizeof(int));
  *((int *) c->argv[1]) = port;
  c->argv[2] = pstrdup(c->pool, password);

  ctx = (cmd->config && cmd->config->config_type != CONF_PARAM ?
    cmd->config->config_type : cmd->server->config_type ?
    cmd->server->config_type : CONF_ROOT);

  if (ctx == CONF_ROOT) {
    /* If we're the "server config" context, set the server now.  This
     * would let mod_redis talk to those servers for e.g. ftpdctl actions.
     */
    (void) redis_set_server(c->argv[0], port, c->argv[2]);
  }

  return PR_HANDLED(cmd);
}

/* usage: RedisTimeouts conn-timeout io-timeout */
MODRET set_redistimeouts(cmd_rec *cmd) {
  config_rec *c;
  unsigned long connect_millis, io_millis;
  char *ptr = NULL;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  connect_millis = strtoul(cmd->argv[1], &ptr, 10);
  if (ptr && *ptr) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "badly formatted connect timeout value: ", cmd->argv[1], NULL));
  }

  ptr = NULL;
  io_millis = strtoul(cmd->argv[2], &ptr, 10);
  if (ptr && *ptr) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "badly formatted IO timeout value: ", cmd->argv[2], NULL));
  }

#if 0
  /* XXX If we're the "server config" context, set the timeouts now.
   * This would let mod_redis talk to those servers for e.g. ftpdctl
   * actions.
   */
  redis_set_timeouts(conn_timeout, io_timeout);
#endif

  c = add_config_param(cmd->argv[0], 2, NULL, NULL);
  c->argv[0] = palloc(c->pool, sizeof(unsigned long));
  *((unsigned long *) c->argv[0]) = connect_millis;
  c->argv[1] = palloc(c->pool, sizeof(unsigned long));
  *((unsigned long *) c->argv[1]) = io_millis;

  return PR_HANDLED(cmd);
}

/* Command handlers
 */

MODRET redis_log_any(cmd_rec *cmd) {
  if (redis_engine == FALSE) {
    return PR_DECLINED(cmd);
  }

  log_event(cmd, REDIS_LOG_EVENT_FL_COMMAND);
  return PR_DECLINED(cmd);
}

MODRET redis_pre_dele(cmd_rec *cmd) {
  const char *path;

  if (redis_engine == FALSE) {
    return PR_DECLINED(cmd);
  }

  path = dir_canonical_path(cmd->tmp_pool,
    pr_fs_decode_path(cmd->tmp_pool, cmd->arg));
  if (path != NULL) {
    struct stat st;

    /* Briefly cache the size of the file being deleted, so that it can be
     * logged properly using %b.
     */
    pr_fs_clear_cache2(path);
    if (pr_fsio_stat(path, &st) == 0) {
      redis_dele_filesz = st.st_size;
    }
  }

  redis_dele_filesz = 0;

  return PR_DECLINED(cmd);
}

/* Event handlers
 */

static void redis_exit_ev(const void *event_data, void *user_data) {
  cmd_rec *cmd;
  pool *tmp_pool;

  tmp_pool = make_sub_pool(session.pool);
  cmd = pr_cmd_alloc(tmp_pool, 1, "DISCONNECT");
  log_event(cmd, REDIS_LOG_EVENT_FL_DISCONNECT);
  destroy_pool(tmp_pool);

  redis_clear();
}

static void redis_restart_ev(const void *event_data, void *user_data) {
  destroy_pool(redis_pool);
  redis_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(redis_pool, MOD_REDIS_VERSION);
}

static void redis_sess_reinit_ev(const void *event_data, void *user_data) {
  int res;

  /* A HOST command changed the main_server pointer, reinitialize ourselves. */

  pr_event_unregister(&redis_module, "core.exit", redis_exit_ev);
  pr_event_unregister(&redis_module, "core.session-reinit",
    redis_sess_reinit_ev);

  (void) close(redis_logfd);
  redis_logfd = -1;

  /* XXX Restore other Redis settings? */
  /* reset RedisTimeouts */

  res = redis_sess_init();
  if (res < 0) {
    pr_session_disconnect(&redis_module,
      PR_SESS_DISCONNECT_SESSION_INIT_FAILED, NULL);
  }
}

static void redis_shutdown_ev(const void *event_data, void *user_data) {
  destroy_pool(redis_pool);
  redis_field_idtab = NULL;
}

/* Initialization functions
 */

static int redis_module_init(void) {
  redis_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(redis_pool, MOD_REDIS_VERSION);

  redis_init();
  pr_event_register(&redis_module, "core.restart", redis_restart_ev, NULL);
  pr_event_register(&redis_module, "core.shutdown", redis_shutdown_ev, NULL);

  pr_log_debug(DEBUG2, MOD_REDIS_VERSION ": using hiredis-%d.%d.%d",
    HIREDIS_MAJOR, HIREDIS_MINOR, HIREDIS_PATCH);

  if (make_fieldtab(redis_pool) < 0) {
    return -1;
  }

  return 0;
}

static int redis_sess_init(void) {
  config_rec *c;
  cmd_rec *cmd;
  pool *tmp_pool;

  pr_event_register(&redis_module, "core.session-reinit",
    redis_sess_reinit_ev, NULL);

  c = find_config(main_server->conf, CONF_PARAM, "RedisEngine", FALSE);
  if (c != NULL) {
    int engine;

    engine = *((int *) c->argv[0]);
    if (engine == FALSE) {
      return 0;
    }

    redis_engine = engine;
  }

  pr_event_register(&redis_module, "core.exit", redis_exit_ev, NULL);

  c = find_config(main_server->conf, CONF_PARAM, "RedisLog", FALSE);
  if (c != NULL) {
    const char *path;

    path = c->argv[0];
    if (strcasecmp(path, "none") != 0) {
      int res, xerrno;

      pr_signals_block();
      PRIVS_ROOT
      res = pr_log_openfile(path, &redis_logfd, PR_LOG_SYSTEM_MODE);
      xerrno = errno;
      PRIVS_RELINQUISH
      pr_signals_unblock();

      switch (res) {
        case 0:
          break;

        case -1:
          pr_log_pri(PR_LOG_NOTICE, MOD_REDIS_VERSION
            ": notice: unable to open RedisLog '%s': %s", path,
            strerror(xerrno));
          break;

        case PR_LOG_WRITABLE_DIR:
          pr_log_pri(PR_LOG_WARNING, MOD_REDIS_VERSION
            ": notice: unable to use RedisLog '%s': parent directory is "
              "world-writable", path);
          break;

        case PR_LOG_SYMLINK:
          pr_log_pri(PR_LOG_WARNING, MOD_REDIS_VERSION
            ": notice: unable to use RedisLog '%s': cannot log to a symlink",
            path);
          break;
      }
    }
  }

  c = find_config(main_server->conf, CONF_PARAM, "RedisServer", FALSE);
  if (c != NULL) {
    const char *server, *password;
    int port;

    server = c->argv[0];
    port = *((int *) c->argv[1]);
    password = c->argv[2];

    (void) redis_set_server(server, port, password);
  }

  c = find_config(main_server->conf, CONF_PARAM, "RedisTimeouts", FALSE);
  if (c) {
    unsigned long connect_millis, io_millis;

    connect_millis = *((unsigned long *) c->argv[0]);
    io_millis = *((unsigned long *) c->argv[1]);

    if (redis_set_timeouts(connect_millis, io_millis) < 0) {
      (void) pr_log_writefile(redis_logfd, MOD_REDIS_VERSION,
        "error setting Redis timeouts: %s", strerror(errno));
    }
  }

  tmp_pool = make_sub_pool(session.pool);
  cmd = pr_cmd_alloc(tmp_pool, 1, "CONNECT");
  log_event(cmd, REDIS_LOG_EVENT_FL_CONNECT);
  destroy_pool(tmp_pool);

  return 0;
}

/* Module API tables
 */

static conftable redis_conftab[] = {
  { "RedisEngine",		set_redisengine,	NULL },
  { "RedisLog",			set_redislog,		NULL },
  { "RedisLogOnCommand",	set_redislogoncommand,	NULL },
  { "RedisServer",		set_redisserver,	NULL },
  { "RedisTimeouts",		set_redistimeouts,	NULL },
 
  { NULL }
};

static cmdtable redis_cmdtab[] = {
  { LOG_CMD,		C_ANY,	G_NONE,	redis_log_any,	FALSE,	FALSE },
  { LOG_CMD_ERR,	C_ANY,	G_NONE,	redis_log_any,	FALSE,	FALSE },
  { PRE_CMD,		C_DELE,	G_NONE,	redis_pre_dele,	FALSE,	FALSE },

  { 0, NULL }
};

module redis_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "redis",

  /* Module configuration handler table */
  redis_conftab,

  /* Module command handler table */
  redis_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  redis_module_init,

  /* Session initialization function */
  redis_sess_init,

  /* Module version */
  MOD_REDIS_VERSION
};
