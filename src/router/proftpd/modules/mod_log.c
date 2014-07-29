/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2013 The ProFTPD Project team
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
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* Flexible logging module for proftpd
 * $Id: mod_log.c,v 1.155 2013/11/11 01:34:04 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"
#include "mod_log.h"

module log_module;

/* Max path length plus 128 bytes for additional info. */
#define EXTENDED_LOG_BUFFER_SIZE		(PR_TUNABLE_PATH_MAX + 128)

#define EXTENDED_LOG_MODE			0644

typedef struct logformat_struc	logformat_t;
typedef struct logfile_struc 	logfile_t;

struct logformat_struc {
  logformat_t		*next,*prev;

  char			*lf_nickname;
  unsigned char		*lf_format;
};

struct logfile_struc {
  logfile_t		*next,*prev;

  char			*lf_filename;
  int			lf_fd;
  int			lf_syslog_level;

  logformat_t		*lf_format;

  int			lf_classes;

  /* Pointer to the "owning" configuration */
  config_rec		*lf_conf;
};

/* Value for lf_fd signalling that data should be logged via syslog, rather
 * than written to a file.
 */
#define EXTENDED_LOG_SYSLOG	-4

/* For tracking the size of deleted files. */
static off_t log_dele_filesz = 0;

static pool *log_pool = NULL;
static logformat_t *formats = NULL;
static xaset_t *format_set = NULL;
static logfile_t *logs = NULL;
static xaset_t *log_set = NULL;

/* format string args:
   %A			- Anonymous username (password given)
   %a			- Remote client IP address
   %b			- Bytes sent for request
   %{basename}		- Basename of path
   %c			- Class
   %D			- full directory path
   %d			- directory (for client)
   %E			- End-of-session reason
   %{FOOBAR}e		- Contents of environment variable FOOBAR
   %F			- Transfer path (filename for client)
   %f			- Filename
   %g			- Local user's primary group name
   %H                   - Local IP address of server handling session
   %h			- Remote client DNS name
   %I                   - Total number of "raw" bytes read in from network
   %J                   - Request (command) arguments (file.txt, etc)
   %L                   - Local IP address contacted by client
   %l			- Remote logname (from identd)
   %m			- Request (command) method (RETR, etc)
   %O                   - Total number of "raw" bytes written out to network
   %P			- Process ID of child serving request
   %p			- Port of server serving request
   %r			- Full request (command)
   %s			- Response code (status)
   %S                   - Response string
   %T			- Time taken to serve request, in seconds
   %t			- Time
   %{format}t		- Formatted time (strftime(3) format)
   %U                   - Original username sent by client
   %u			- Local user
   %V                   - DNS name of server serving request
   %v			- ServerName of server serving request
   %w                   - RNFR path ("whence" a rename comes, i.e. the source)
   %{file-modified}     - Indicates whether a file is being modified
                          (i.e. already exists) or not.
   %{iso8601}           - ISO-8601 timestamp: YYYY-MM-dd HH:mm:ss,SSS
                            for example: "1999-11-27 15:49:37,459"
   %{microsecs}         - 6 digits of microseconds of current time
   %{millisecs}         - 3 digits of milliseconds of current time
   %{protocol}          - Current protocol (e.g. "ftp", "sftp", etc)
   %{uid}               - UID of logged-in user
   %{gid}               - Primary GID of logged-in user
   %{transfer-status}   - "success", "failed", "cancelled", "timeout", or "-"
   %{transfer-failure}  - reason, or "-"
   %{version}           - ProFTPD version
*/

/* Necessary prototypes */
static int log_sess_init(void);

static void add_meta(unsigned char **s, unsigned char meta, int args, ...) {
  int arglen;
  char *arg;

  **s = LOGFMT_META_START;
  (*s) = (*s) + 1;
  **s = meta;
  (*s) = (*s) + 1;

  if (args) {
    va_list ap;
    va_start(ap, args);

    while (args--) {
      arglen = va_arg(ap, int);
      arg = va_arg(ap, char *);

      memcpy(*s, arg, arglen);
      (*s) = (*s) + arglen;
      **s = LOGFMT_META_ARG_END;
      (*s) = (*s) + 1;
    }

    va_end(ap);
  }
}

static char *preparse_arg(char **s) {
  char *ret = (*s) + 1;

  (*s) = (*s) + 1;
  while (**s && **s != '}')
    (*s) = (*s) + 1;

  **s = '\0';
  (*s) = (*s) + 1;
  return ret;
}

static void logformat(const char *directive, char *nickname, char *fmts) {
  char *tmp, *arg;
  unsigned char format[4096] = {'\0'}, *outs;
  logformat_t *lf;

  /* This function can cause potential problems.  Custom LogFormats
   * might overrun the format buffer.  Fixing this problem involves a
   * rewrite of most of this module.  This will happen post 1.2.0.
   */

  outs = format;
  for (tmp = fmts; *tmp; ) {
    if (*tmp == '%') {
      arg = NULL;
      tmp++;
      for (;;) {
        pr_signals_handle();

        if (strncmp(tmp, "{basename}", 10) == 0) {
          add_meta(&outs, LOGFMT_META_BASENAME, 0);
          tmp += 10;
          continue;
        }
 
        if (strncmp(tmp, "{file-modified}", 15) == 0) {
          add_meta(&outs, LOGFMT_META_FILE_MODIFIED, 0);
          tmp += 15;
          continue;
        }

        if (strncmp(tmp, "{gid}", 5) == 0) {
          add_meta(&outs, LOGFMT_META_GID, 0);
          tmp += 5;
          continue;
        }

        if (strncasecmp(tmp, "{iso8601}", 9) == 0) {
          add_meta(&outs, LOGFMT_META_ISO8601, 0);
          tmp += 9;
          continue;
        }

        if (strncmp(tmp, "{microsecs}", 11) == 0) {
          add_meta(&outs, LOGFMT_META_MICROSECS, 0);
          tmp += 11;
          continue;
        }

        if (strncmp(tmp, "{millisecs}", 11) == 0) {
          add_meta(&outs, LOGFMT_META_MILLISECS, 0);
          tmp += 11;
          continue;
        }

        if (strncmp(tmp, "{protocol}", 10) == 0) {
          add_meta(&outs, LOGFMT_META_PROTOCOL, 0);
          tmp += 10;
          continue;
        }

        if (strncmp(tmp, "{uid}", 5) == 0) {
          add_meta(&outs, LOGFMT_META_UID, 0);
          tmp += 5;
          continue;
        }

        if (strncmp(tmp, "{transfer-failure}", 18) == 0) {
          add_meta(&outs, LOGFMT_META_XFER_FAILURE, 0);
          tmp += 18;
          continue;
        }

        if (strncmp(tmp, "{transfer-status}", 17) == 0) {
          add_meta(&outs, LOGFMT_META_XFER_STATUS, 0);
          tmp += 17;
          continue;
        }

        if (strncmp(tmp, "{version}", 9) == 0) {
          add_meta(&outs, LOGFMT_META_VERSION, 0);
          tmp += 9;
          continue;
        }

        if (strncmp(tmp, "{note:", 6) == 0) {
          char *ptr;

          ptr = strchr(tmp + 6, '}');
          if (ptr != NULL) {
            char *note;
            size_t notelen;

            note = tmp + 6;
            notelen = (ptr - note);

            add_meta(&outs, LOGFMT_META_NOTE_VAR, 0);
            add_meta(&outs, LOGFMT_META_ARG, 1, (int) notelen, note);

            /* Advance 6 for the leading '{note:', and one more for the
             * trailing '}' character.
             */
            tmp += (notelen + 6 + 1);
            continue;
          }
        }

        switch (*tmp) {
          case '{':
            arg = preparse_arg(&tmp);
            continue;

          case 'a':
            add_meta(&outs, LOGFMT_META_REMOTE_IP, 0);
            break;

          case 'A':
            add_meta(&outs, LOGFMT_META_ANON_PASS, 0);
            break;

          case 'b':
            add_meta(&outs, LOGFMT_META_BYTES_SENT, 0);
            break;

          case 'c':
            add_meta(&outs, LOGFMT_META_CLASS, 0);
            break;

          case 'D':
            add_meta(&outs, LOGFMT_META_DIR_PATH, 0);
            break;

          case 'd':
            add_meta(&outs, LOGFMT_META_DIR_NAME, 0);
            break;

          case 'E':
            add_meta(&outs, LOGFMT_META_EOS_REASON, 0);
            break;

          case 'e':
            if (arg) {
              add_meta(&outs, LOGFMT_META_ENV_VAR, 0);
              add_meta(&outs, LOGFMT_META_ARG, 1, (int) strlen(arg), arg);
            }
            break;

          case 'f':
            add_meta(&outs, LOGFMT_META_FILENAME, 0);
            break;

          case 'F':
            add_meta(&outs, LOGFMT_META_XFER_PATH, 0);
            break;

          case 'g':
            add_meta(&outs, LOGFMT_META_GROUP, 0);
            break;

          case 'H':
            add_meta(&outs, LOGFMT_META_VHOST_IP, 0);
            break;

          case 'h':
            add_meta(&outs, LOGFMT_META_REMOTE_HOST, 0);
            break;

          case 'I':
            add_meta(&outs, LOGFMT_META_RAW_BYTES_IN, 0);
            break;

          case 'J':
            add_meta(&outs, LOGFMT_META_CMD_PARAMS, 0);
            break;

          case 'l':
            add_meta(&outs, LOGFMT_META_IDENT_USER, 0);
            break;

          case 'L':
            add_meta(&outs, LOGFMT_META_LOCAL_IP, 0);
            break;

          case 'm':
            add_meta(&outs, LOGFMT_META_METHOD, 0);
            break;

          case 'O':
            add_meta(&outs, LOGFMT_META_RAW_BYTES_OUT, 0);
            break;

          case 'p':
            add_meta(&outs, LOGFMT_META_LOCAL_PORT, 0);
            break;

          case 'P':
            add_meta(&outs, LOGFMT_META_PID, 0);
            break;

          case 'r':
            add_meta(&outs, LOGFMT_META_COMMAND, 0);
            break;

          case 's':
            add_meta(&outs, LOGFMT_META_RESPONSE_CODE, 0);
            break;

          case 'S':
            add_meta(&outs, LOGFMT_META_RESPONSE_STR, 0);
            break;

          case 't':
            add_meta(&outs, LOGFMT_META_TIME, 0);
            if (arg)
              add_meta(&outs, LOGFMT_META_ARG, 1, (int) strlen(arg), arg);
            break;

          case 'T':
            add_meta(&outs, LOGFMT_META_SECONDS, 0);
            break;

          case 'u':
            add_meta(&outs, LOGFMT_META_USER, 0);
            break;

          case 'U':
            add_meta(&outs, LOGFMT_META_ORIGINAL_USER, 0);
            break;

          case 'v':
            add_meta(&outs, LOGFMT_META_LOCAL_NAME, 0);
            break;

          case 'V':
            add_meta(&outs, LOGFMT_META_LOCAL_FQDN, 0);
            break;

          case 'w':
            add_meta(&outs, LOGFMT_META_RENAME_FROM, 0);
            break;

          case '%':
            *outs++ = '%';
            break;

          default:
            *outs++ = *tmp;
            break;
        }

        tmp++;
        break;
      }

    } else {
      *outs++ = *tmp++;
    }
  }

  *outs++ = '\0';

  lf = (logformat_t *) pcalloc(log_pool, sizeof(logformat_t));
  lf->lf_nickname = pstrdup(log_pool, nickname);
  lf->lf_format = palloc(log_pool, outs - format);
  memcpy(lf->lf_format, format, outs - format);

  if (!format_set)
    format_set = xaset_create(log_pool, NULL);

  xaset_insert_end(format_set, (xasetmember_t *) lf);
  formats = (logformat_t *) format_set->xas_list;

  if (directive != NULL) {
    config_rec *c;

    /* Store the parsed format in the config tree as well, for use by other
     * logging-related modules.
     */
    c = add_config_param(directive, 2, NULL, NULL);
    c->argv[0] = pstrdup(c->pool, nickname);
    c->argv[1] = palloc(c->pool, outs - format);

    tmp = c->argv[1];
    memcpy(tmp, format, outs - format); 
  }
}

/* Syntax: LogFormat nickname "format string" */
MODRET set_logformat(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT);

  logformat(cmd->argv[0], cmd->argv[1], cmd->argv[2]);
  return PR_HANDLED(cmd);
}

static int parse_classes(char *s) {
  int classes = CL_NONE;
  char *nextp = NULL;

  do {
    pr_signals_handle();

    nextp = strchr(s, ',');
    if (nextp != NULL) {
      *nextp++ = '\0';

    } else {
      nextp = strchr(s, '|');
      if (nextp != NULL) {
        *nextp++ = '\0';
      }
    }

    if (strcasecmp(s, "NONE") == 0) {
      classes = CL_NONE;
      break;
    }

    if (strcasecmp(s, "ALL") == 0) {
      classes = CL_ALL;
      break;

    } else if (strcasecmp(s, "AUTH") == 0) {
      classes |= CL_AUTH;

    } else if (strcasecmp(s, "INFO") == 0) {
      classes |= CL_INFO;

    } else if (strcasecmp(s, "DIRS") == 0) {
      classes |= CL_DIRS;

    } else if (strcasecmp(s, "READ") == 0) {
      classes |= CL_READ;

    } else if (strcasecmp(s, "WRITE") == 0) {
      classes |= CL_WRITE;

    } else if (strcasecmp(s, "MISC") == 0) {
      classes |= CL_MISC;

    } else if (strcasecmp(s, "SEC") == 0 ||
               strcasecmp(s, "SECURE") == 0) {
      classes |= CL_SEC;

    } else if (strcasecmp(s, "EXIT") == 0) {
      classes |= CL_EXIT;

    } else {
      pr_log_pri(PR_LOG_NOTICE, "ExtendedLog class '%s' is not defined", s);
      return -1;
    }

    /* Advance to the next class in the list. */
    s = nextp;

  } while (s);

  return classes;
}

/* Syntax: ExtendedLog file [<cmd-classes> [<nickname>]] */
MODRET set_extendedlog(cmd_rec *cmd) {
  config_rec *c = NULL;
  int argc;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON);

  argc = cmd->argc;

  if (argc < 2)
    CONF_ERROR(cmd, "Syntax: ExtendedLog file [<cmd-classes> [<nickname>]]");

  c = add_config_param(cmd->argv[0], 3, NULL, NULL, NULL);

  if (strncasecmp(cmd->argv[1], "syslog:", 7) == 0) {
    char *tmp = strchr(cmd->argv[1], ':');

    if (pr_log_str2sysloglevel(++tmp) < 0) {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unknown syslog level: '",
        tmp, "'", NULL));

    } else {
      c->argv[0] = pstrdup(log_pool, cmd->argv[1]);
    }

  } else if (cmd->argv[1][0] != '/') {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "relative paths not allowed: '",
        cmd->argv[1], "'", NULL));

  } else {
    c->argv[0] = pstrdup(log_pool, cmd->argv[1]);
  }

  if (argc > 2) {
    int res;

    /* Parse the given class names, to make sure that they are all valid. */
    res = parse_classes(cmd->argv[2]);
    if (res < 0) {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "invalid log class in '",
        cmd->argv[2], "'", NULL));    
    }

    c->argv[1] = palloc(c->pool, sizeof(int));
    *((int *) c->argv[1]) = res;
  }

  if (argc > 3) {
    c->argv[2] = pstrdup(log_pool, cmd->argv[3]);
  }

  c->argc = argc-1;
  return PR_HANDLED(cmd);
}

/* Syntax: AllowLogSymlinks <on|off> */
MODRET set_allowlogsymlinks(cmd_rec *cmd) {
  int bool = -1;
  config_rec *c = NULL;

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

/* Syntax: ServerLog <filename> */
MODRET set_serverlog(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);

  return PR_HANDLED(cmd);
}

/* Syntax: SystemLog <filename> */
MODRET set_systemlog(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  (void) add_config_param_str(cmd->argv[0], 1, cmd->argv[1]); 
  return PR_HANDLED(cmd);
}

static struct tm *_get_gmtoff(int *tz) {
  time_t tt = time(NULL);
  struct tm gmt;
  struct tm *t;
  int days, hours, minutes;

  gmt = *gmtime(&tt);
  t = pr_localtime(NULL, &tt);

  days = t->tm_yday - gmt.tm_yday;
  hours = ((days < -1 ? 24 : 1 < days ? -24 : days * 24)
          + t->tm_hour - gmt.tm_hour);
  minutes = hours * 60 + t->tm_min - gmt.tm_min;
  *tz = minutes;
  return t;
}

static char *get_next_meta(pool *p, cmd_rec *cmd, unsigned char **f) {
  unsigned char *m;
  char arg[PR_TUNABLE_PATH_MAX+1] = {'\0'}, *argp = NULL, *pass;

  /* This function can cause potential problems.  Custom logformats
   * might overrun the arg buffer.  Fixing this problem involves a
   * rewrite of most of this module.  This will happen post 1.2.0.
   */

  m = (*f) + 1;
  switch (*m) {
    case LOGFMT_META_ARG:
      m++;
      argp = arg;
      while (*m != LOGFMT_META_ARG_END)
        *argp++ = (char) *m++;

      *argp = 0;
      argp = arg;
      m++;
      break;

    case LOGFMT_META_ANON_PASS:
      argp = arg;

      pass = pr_table_get(session.notes, "mod_auth.anon-passwd", NULL);
      if (!pass)
        pass = "UNKNOWN";

      sstrncpy(argp, pass, sizeof(arg));

      m++;
      break;

    case LOGFMT_META_BYTES_SENT:
      argp = arg;
      if (session.xfer.p) {
        snprintf(argp, sizeof(arg), "%" PR_LU,
          (pr_off_t) session.xfer.total_bytes);

      } else if (pr_cmd_cmp(cmd, PR_CMD_DELE_ID) == 0) {
        snprintf(argp, sizeof(arg), "%" PR_LU, (pr_off_t) log_dele_filesz);

      } else
        sstrncpy(argp, "-", sizeof(arg));

      m++;
      break;

    case LOGFMT_META_CLASS:
      argp = arg;
      sstrncpy(argp, session.conn_class ? session.conn_class->cls_name : "-",
        sizeof(arg));
      m++;
      break;

    case LOGFMT_META_DIR_NAME:
      argp = arg;

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
        char *path, *tmp;

        path = pr_fs_decode_path(p, cmd->arg);
        tmp = strrchr(path, '/');

        if (tmp != NULL) {
          if (tmp != path) {
            sstrncpy(argp, tmp + 1, sizeof(arg));

          } else if (*(tmp+1) != '\0') {
            sstrncpy(argp, tmp + 1, sizeof(arg));

          } else {
            sstrncpy(argp, path, sizeof(arg));
          }

        } else {
          sstrncpy(argp, path, sizeof(arg));
        }

      } else {
        sstrncpy(argp, pr_fs_getvwd(), sizeof(arg));
      }

      m++;
      break;

    case LOGFMT_META_DIR_PATH:
      argp = arg;

      if (pr_cmd_cmp(cmd, PR_CMD_CDUP_ID) == 0 ||
          pr_cmd_cmp(cmd, PR_CMD_LIST_ID) == 0 ||
          pr_cmd_cmp(cmd, PR_CMD_MLSD_ID) == 0 ||
          pr_cmd_cmp(cmd, PR_CMD_MKD_ID) == 0 ||
          pr_cmd_cmp(cmd, PR_CMD_NLST_ID) == 0 ||
          pr_cmd_cmp(cmd, PR_CMD_RMD_ID) == 0 ||
          pr_cmd_cmp(cmd, PR_CMD_XCUP_ID) == 0 ||
          pr_cmd_cmp(cmd, PR_CMD_XMKD_ID) == 0 ||
          pr_cmd_cmp(cmd, PR_CMD_XRMD_ID) == 0) {
        sstrncpy(argp, dir_abs_path(p, pr_fs_decode_path(p, cmd->arg), TRUE),
          sizeof(arg));

      } else if (pr_cmd_cmp(cmd, PR_CMD_CWD_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_XCWD_ID) == 0) {

        /* Note: by this point in the dispatch cycle, the current working
         * directory has already been changed.  For the CWD/XCWD commands,
         * this means that dir_abs_path() may return an improper path,
         * with the target directory being reported twice.  To deal with this,
         * don't use dir_abs_path(), and use pr_fs_getvwd()/pr_fs_getcwd()
         * instead.
         */

        if (session.chroot_path) { 
          /* Chrooted session. */
          sstrncpy(arg, strcmp(pr_fs_getvwd(), "/") ?
            pdircat(p, session.chroot_path, pr_fs_getvwd(), NULL) :
            session.chroot_path, sizeof(arg));

        } else {

          /* Non-chrooted session. */
          sstrncpy(arg, pr_fs_getcwd(), sizeof(arg));
        }

      } else {
        sstrncpy(argp, "", sizeof(arg));
      }

      m++;
      break;

    case LOGFMT_META_EOS_REASON: {
      const char *reason_str;
      char *details = NULL;

      argp = arg;

      reason_str = pr_session_get_disconnect_reason(&details);
      sstrncpy(argp, reason_str, sizeof(arg));
      if (details != NULL) {
        sstrcat(argp, ": ", sizeof(arg));
        sstrcat(argp, details, sizeof(arg));
      }

      m++;
      break;
    }

    case LOGFMT_META_FILENAME:
      argp = arg;

      if (pr_cmd_cmp(cmd, PR_CMD_RNTO_ID) == 0) {
        sstrncpy(argp, dir_abs_path(p, pr_fs_decode_path(p, cmd->arg), TRUE),
          sizeof(arg));

      } else if (pr_cmd_cmp(cmd, PR_CMD_RETR_ID) == 0) {
        char *path;

        path = pr_table_get(cmd->notes, "mod_xfer.retr-path", NULL);
        sstrncpy(arg, dir_abs_path(p, path, TRUE), sizeof(arg));

      } else if (pr_cmd_cmp(cmd, PR_CMD_APPE_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_STOR_ID) == 0) {
        char *path;
      
        path = pr_table_get(cmd->notes, "mod_xfer.store-path", NULL);
        sstrncpy(arg, dir_abs_path(p, path, TRUE), sizeof(arg));

      } else if (session.xfer.p &&
                 session.xfer.path) {
        sstrncpy(argp, dir_abs_path(p, session.xfer.path, TRUE), sizeof(arg));

      } else if (pr_cmd_cmp(cmd, PR_CMD_CDUP_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_PWD_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_XCUP_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_XPWD_ID) == 0) {
        sstrncpy(argp, dir_abs_path(p, pr_fs_getcwd(), TRUE), sizeof(arg));

      } else if (pr_cmd_cmp(cmd, PR_CMD_CWD_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_XCWD_ID) == 0) {

        /* Note: by this point in the dispatch cycle, the current working
         * directory has already been changed.  For the CWD/XCWD commands,
         * this means that dir_abs_path() may return an improper path,
         * with the target directory being reported twice.  To deal with this,
         * don't use dir_abs_path(), and use pr_fs_getvwd()/pr_fs_getcwd()
         * instead.
         */
        if (session.chroot_path) {
          /* Chrooted session. */
          sstrncpy(arg, strcmp(pr_fs_getvwd(), "/") ?
            pdircat(p, session.chroot_path, pr_fs_getvwd(), NULL) :
            session.chroot_path, sizeof(arg));

        } else {
          /* Non-chrooted session. */
          sstrncpy(arg, pr_fs_getcwd(), sizeof(arg));
        }

      } else if (pr_cmd_cmp(cmd, PR_CMD_SITE_ID) == 0 &&
                 (strncasecmp(cmd->argv[1], "CHGRP", 6) == 0 ||
                  strncasecmp(cmd->argv[1], "CHMOD", 6) == 0 ||
                  strncasecmp(cmd->argv[1], "UTIME", 6) == 0)) {
        register unsigned int i;
        char *tmp = "";

        for (i = 3; i <= cmd->argc-1; i++) {
          tmp = pstrcat(cmd->tmp_pool, tmp, *tmp ? " " : "",
            pr_fs_decode_path(cmd->tmp_pool, cmd->argv[i]), NULL);
        }

        sstrncpy(argp, dir_abs_path(p, tmp, TRUE), sizeof(arg));

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
          sstrncpy(arg, dir_abs_path(p, pr_fs_decode_path(p, cmd->arg), TRUE),
            sizeof(arg));

        } else if (pr_cmd_cmp(cmd, PR_CMD_MFMT_ID) == 0) {
          /* MFMT has, as its filename, the second argument. */
          sstrncpy(arg, dir_abs_path(p, pr_fs_decode_path(p, cmd->argv[2]),
            TRUE), sizeof(arg));
 
        } else {
          /* All other situations get a "-".  */
          sstrncpy(argp, "-", sizeof(arg));
        }
      }

      m++;
      break;

    case LOGFMT_META_BASENAME: {
      const char *path = NULL;
      argp = arg;

      if (pr_cmd_cmp(cmd, PR_CMD_RNTO_ID) == 0) {
        path = pr_fs_decode_path(p, cmd->arg);

      } else if (pr_cmd_cmp(cmd, PR_CMD_RETR_ID) == 0) {
        path = pr_table_get(cmd->notes, "mod_xfer.retr-path", NULL);

      } else if (pr_cmd_cmp(cmd, PR_CMD_APPE_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_STOR_ID) == 0) {
        path = pr_table_get(cmd->notes, "mod_xfer.store-path", NULL);

      } else if (session.xfer.p &&
                 session.xfer.path) {
        path = session.xfer.path;

      } else if (pr_cmd_cmp(cmd, PR_CMD_CDUP_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_PWD_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_XCUP_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_XPWD_ID) == 0) {
        path = pr_fs_getcwd();

      } else if (pr_cmd_cmp(cmd, PR_CMD_CWD_ID) == 0 ||
                 pr_cmd_cmp(cmd, PR_CMD_XCWD_ID) == 0) {

        /* Note: by this point in the dispatch cycle, the current working
         * directory has already been changed.  For the CWD/XCWD commands,
         * this means that dir_abs_path() may return an improper path,
         * with the target directory being reported twice.  To deal with this,
         * don't use dir_abs_path(), and use pr_fs_getvwd()/pr_fs_getcwd()
         * instead.
         */
        if (session.chroot_path) {
          /* Chrooted session. */
          path = strcmp(pr_fs_getvwd(), "/") ?
            pr_fs_getvwd() : session.chroot_path;

        } else {
          /* Non-chrooted session. */
          path = pr_fs_getcwd();
        }

      } else if (pr_cmd_cmp(cmd, PR_CMD_SITE_ID) == 0 &&
                 (strncasecmp(cmd->argv[1], "CHGRP", 6) == 0 ||
                  strncasecmp(cmd->argv[1], "CHMOD", 6) == 0 ||
                  strncasecmp(cmd->argv[1], "UTIME", 6) == 0)) {
        register unsigned int i;
        char *tmp = "";

        for (i = 3; i <= cmd->argc-1; i++) {
          tmp = pstrcat(cmd->tmp_pool, tmp, *tmp ? " " : "",
            pr_fs_decode_path(cmd->tmp_pool, cmd->argv[i]), NULL);
        }

        path = tmp;

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
          path = pr_fs_decode_path(p, cmd->arg);

        } else if (pr_cmd_cmp(cmd, PR_CMD_MFMT_ID) == 0) {
          /* MFMT has, as its filename, the second argument. */
          path = pr_fs_decode_path(p, cmd->argv[2]);
        }
      }

      if (path != NULL) { 
        char *ptr = NULL;

        ptr = strrchr(path, '/');
        if (ptr != NULL) {
          if (ptr != path) {
            sstrncpy(argp, ptr + 1, sizeof(arg));

          } else if (*(ptr+1) != '\0') {
            sstrncpy(argp, ptr + 1, sizeof(arg));

          } else {
            sstrncpy(argp, path, sizeof(arg));
          }

        } else {
          sstrncpy(argp, path, sizeof(arg));
        }

      } else {
        /* All other situations get a "-".  */
        sstrncpy(argp, "-", sizeof(arg));
      }

      m++;
      break;
    }

    case LOGFMT_META_XFER_PATH:
      argp = arg;

      if (pr_cmd_cmp(cmd, PR_CMD_RNTO_ID) == 0) {
        char *path;

        path = dir_best_path(cmd->tmp_pool,
          pr_fs_decode_path(cmd->tmp_pool, cmd->arg));
        sstrncpy(arg, path, sizeof(arg));

      } else if (session.xfer.p &&
                 session.xfer.path) {
        sstrncpy(argp, session.xfer.path, sizeof(arg));

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
          char *path;

          path = dir_best_path(cmd->tmp_pool,
            pr_fs_decode_path(cmd->tmp_pool, cmd->arg));
          sstrncpy(arg, path, sizeof(arg));

        } else {
          sstrncpy(argp, "-", sizeof(arg));
        }
      }

      m++;
      break;

    case LOGFMT_META_ENV_VAR:
      argp = arg;
      m++;

      if (*m == LOGFMT_META_START &&
          *(m+1) == LOGFMT_META_ARG) {
        char *key = get_next_meta(p, cmd, &m);
        if (key) {
          char *env = pr_env_get(cmd->tmp_pool, key);
          if (env) {
            sstrncpy(argp, env, sizeof(arg));
          }
        }
      }

      break;

    case LOGFMT_META_NOTE_VAR:
      argp = arg;
      m++;

      if (*m == LOGFMT_META_START &&
          *(m+1) == LOGFMT_META_ARG) {
        char *key = get_next_meta(p, cmd, &m);
        if (key) {
          char *note = NULL;

          /* Check in the cmd->notes table first. */
          note = pr_table_get(cmd->notes, key, NULL);
          if (note == NULL) {
            /* If not there, check the session.notes table. */
            note = pr_table_get(session.notes, key, NULL);
          }
 
          if (note) {
            sstrncpy(argp, note, sizeof(arg));
          }
        }
      }

      break;

    case LOGFMT_META_REMOTE_HOST:
      argp = arg;
      sstrncpy(argp, pr_netaddr_get_sess_remote_name(), sizeof(arg));
      m++;
      break;

    case LOGFMT_META_REMOTE_IP:
      argp = arg;
      sstrncpy(argp, pr_netaddr_get_ipstr(pr_netaddr_get_sess_remote_addr()),
        sizeof(arg));
      m++;
      break;

    case LOGFMT_META_RENAME_FROM: {
      char *rnfr_path = "-";

      argp = arg;
      if (pr_cmd_cmp(cmd, PR_CMD_RNTO_ID) == 0) {
        rnfr_path = pr_table_get(session.notes, "mod_core.rnfr-path", NULL);
        if (rnfr_path != NULL) {
          rnfr_path = dir_abs_path(p, pr_fs_decode_path(p, rnfr_path), TRUE);

        } else {
          rnfr_path = "-";
        }
      }

      sstrncpy(argp, rnfr_path, sizeof(arg));
      m++;
      break;
    }

    case LOGFMT_META_IDENT_USER: {
      char *rfc1413_ident;

      argp = arg;
      rfc1413_ident = pr_table_get(session.notes, "mod_ident.rfc1413-ident",
        NULL);
      if (rfc1413_ident == NULL)
        rfc1413_ident = "UNKNOWN";

      sstrncpy(argp, rfc1413_ident, sizeof(arg));
      m++;
      break;
    }

    case LOGFMT_META_METHOD:
      argp = arg;

      if (pr_cmd_cmp(cmd, PR_CMD_SITE_ID) != 0) {
        sstrncpy(argp, cmd->argv[0], sizeof(arg));

      } else {
        char *ptr;

        /* Make sure that the SITE command used is all in uppercase,
         * for logging purposes.
         */

        for (ptr = cmd->argv[1]; *ptr; ptr++) {
          *ptr = toupper((int) *ptr);
        }

        snprintf(argp, sizeof(arg), "%s %s", cmd->argv[0], cmd->argv[1]);
      }

      m++;
      break;

    case LOGFMT_META_LOCAL_PORT:
      argp = arg;
      snprintf(argp, sizeof(arg), "%d", cmd->server->ServerPort);
      m++;
      break;

    case LOGFMT_META_LOCAL_IP:
      argp = arg;
      sstrncpy(argp, pr_netaddr_get_ipstr(pr_netaddr_get_sess_local_addr()),
        sizeof(arg));
      m++;
      break;

    case LOGFMT_META_LOCAL_FQDN:
      argp = arg;
      sstrncpy(argp, pr_netaddr_get_dnsstr(pr_netaddr_get_sess_local_addr()),
        sizeof(arg));
      m++;
      break;

    case LOGFMT_META_PID:
      argp = arg;
      snprintf(argp, sizeof(arg), "%u",(unsigned int)getpid());
      m++;
      break;

    case LOGFMT_META_MICROSECS: {
      struct timeval now;

      argp = arg;
      gettimeofday(&now, NULL);

      snprintf(argp, sizeof(arg), "%06lu", (unsigned long) now.tv_usec);
      m++;
      break;
    }

    case LOGFMT_META_MILLISECS: {
      struct timeval now;
      unsigned long millis;

      argp = arg;

      gettimeofday(&now, NULL);

      /* Convert microsecs to millisecs. */
      millis = now.tv_usec / 1000;

      snprintf(argp, sizeof(arg), "%03lu", millis);
      m++;
      break;
    }

    case LOGFMT_META_TIME:
      {
        char *time_fmt = "[%d/%b/%Y:%H:%M:%S ";
        struct tm t;
        int internal_fmt = 1;
        int timz;
        char sign;

        argp = arg;
        m++;

        if (*m == LOGFMT_META_START &&
            *(m+1) == LOGFMT_META_ARG) {
          time_fmt = get_next_meta(p, cmd, &m);
          internal_fmt = 0;
        }

        t = *_get_gmtoff(&timz);
        sign = (timz < 0 ? '-' : '+');
        if (timz < 0)
          timz = -timz;

        if (time_fmt) {
          strftime(argp, 80, time_fmt, &t);
          if (internal_fmt) {
            if (strlen(argp) < sizeof(arg)) {
              snprintf(argp + strlen(argp), sizeof(arg) - strlen(argp),
                "%c%.2d%.2d]", sign, timz/60, timz%60);

            } else {
              pr_log_pri(PR_LOG_NOTICE,
                "notice: %%t expansion yields excessive string, ignoring");
            }
          }
        }
      }
      break;

    case LOGFMT_META_ISO8601: {
      struct tm *tm;
      struct timeval now;
      unsigned long millis;
      size_t len;

      argp = arg;

      gettimeofday(&now, NULL);
      tm = pr_localtime(NULL, (const time_t *) &(now.tv_sec));

      len = strftime(argp, sizeof(arg), "%Y-%m-%d %H:%M:%S", tm);

      /* Convert microsecs to millisecs. */
      millis = now.tv_usec / 1000;

      snprintf(argp + len, sizeof(arg), ",%03lu", millis);
      m++;
      break;
    }

    case LOGFMT_META_SECONDS:
      argp = arg;
      if (session.xfer.p) {
        /* Make sure that session.xfer.start_time actually has values (which
         * is not always the case).
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

          snprintf(argp, sizeof(arg), "%ld.%03ld", (long) end_time.tv_sec,
            (long) (end_time.tv_usec / 1000));

        } else {
          sstrncpy(argp, "-", sizeof(arg));
        }

      } else {
        sstrncpy(argp, "-", sizeof(arg));
      }

      m++;
      break;

    case LOGFMT_META_COMMAND:
      argp = arg;

      if (pr_cmd_cmp(cmd, PR_CMD_PASS_ID) == 0 &&
          session.hide_password) {
        sstrncpy(argp, "PASS (hidden)", sizeof(arg));

      } else {
        sstrncpy(argp, pr_cmd_get_displayable_str(cmd, NULL), sizeof(arg));
      }

      m++;
      break;

    case LOGFMT_META_CMD_PARAMS:
      argp = arg;
      if (pr_cmd_cmp(cmd, PR_CMD_PASS_ID) == 0 &&
          session.hide_password) {
        sstrncpy(argp, "(hidden)", sizeof(arg));

      } else {
        sstrncpy(argp, pr_fs_decode_path(p, cmd->arg), sizeof(arg));
      }

      m++;
      break;

    case LOGFMT_META_LOCAL_NAME:
      argp = arg;
      sstrncpy(argp, cmd->server->ServerName, sizeof(arg));
      m++;
      break;

    case LOGFMT_META_USER:
      argp = arg;

      if (session.user != NULL) {
        sstrncpy(argp, session.user, sizeof(arg));

      } else {
        sstrncpy(argp, "-", sizeof(arg));
      }

      m++;
      break;

    case LOGFMT_META_ORIGINAL_USER: {
      char *login_user;

      argp = arg;

      login_user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);
      if (login_user) {
        sstrncpy(argp, login_user, sizeof(arg));

      } else {
        sstrncpy(argp, "-", sizeof(arg));
      }

      m++;
      break;
    }

    case LOGFMT_META_GROUP:
      argp = arg;

      if (session.group != NULL) {
        sstrncpy(argp, session.group, sizeof(arg));

      } else {
        sstrncpy(argp, "-", sizeof(arg));
      }

      m++;
      break;

    case LOGFMT_META_RESPONSE_CODE: {
      char *resp_code = NULL;
      int res;

      argp = arg;

      res = pr_response_get_last(cmd->tmp_pool, &resp_code, NULL);
      if (res == 0 &&
          resp_code != NULL) {
        sstrncpy(argp, resp_code, sizeof(arg));

      /* Hack to add return code for proper logging of QUIT command. */
      } else if (pr_cmd_cmp(cmd, PR_CMD_QUIT_ID) == 0) {
        sstrncpy(argp, R_221, sizeof(arg));

      } else {
        sstrncpy(argp, "-", sizeof(arg));
      }

      m++;
      break;
    }

    case LOGFMT_META_RESPONSE_STR: {
      char *resp_msg = NULL;
      int res;

      argp = arg;

      res = pr_response_get_last(cmd->tmp_pool, NULL, &resp_msg);
      if (res == 0 &&
          resp_msg != NULL) {
        sstrncpy(argp, resp_msg, sizeof(arg));

      } else {
        sstrncpy(argp, "-", sizeof(arg));
      }

      m++;
      break;
    }

    case LOGFMT_META_PROTOCOL:
      argp = arg;
      sstrncpy(argp, pr_session_get_protocol(0), sizeof(arg));
      m++;
      break;

    case LOGFMT_META_UID:
      argp = arg;
      snprintf(argp, sizeof(arg), "%lu", (unsigned long) session.login_uid);
      m++;
      break;

    case LOGFMT_META_GID:
      argp = arg;
      snprintf(argp, sizeof(arg), "%lu", (unsigned long) session.login_gid);
      m++;
      break;

    case LOGFMT_META_XFER_FAILURE: {
      argp = arg;

      /* If the current command is one that incurs a data transfer, then we
       * need to do more work.  If not, it's an easy substitution.
       */
      if (session.curr_cmd_id == PR_CMD_APPE_ID ||
          session.curr_cmd_id == PR_CMD_LIST_ID ||
          session.curr_cmd_id == PR_CMD_MLSD_ID ||
          session.curr_cmd_id == PR_CMD_NLST_ID ||
          session.curr_cmd_id == PR_CMD_RETR_ID ||
          session.curr_cmd_id == PR_CMD_STOR_ID ||
          session.curr_cmd_id == PR_CMD_STOU_ID) {
        const char *proto;

        proto = pr_session_get_protocol(0);

        if (strncmp(proto, "ftp", 4) == 0 ||
            strncmp(proto, "ftps", 5) == 0) {

          if (XFER_ABORTED) {
            sstrncpy(argp, "-", sizeof(arg));

          } else {
            int res;
            char *resp_code = NULL, *resp_msg = NULL;

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
                  sstrncpy(argp, ptr + 2, sizeof(arg));

                } else {
                  sstrncpy(argp, resp_msg, sizeof(arg));
                }

              } else {
                sstrncpy(argp, "-", sizeof(arg));
              }

            } else {
              sstrncpy(argp, "-", sizeof(arg));
            }
          }

        } else {
          /* Currently, for failed SFTP/SCP transfers, we can't properly
           * populate the failure reason.  Maybe in the future.
           */
          sstrncpy(argp, "-", sizeof(arg));
        }

      } else {
        sstrncpy(argp, "-", sizeof(arg));
      }

      m++;
      break;
    }

    case LOGFMT_META_XFER_STATUS: {
      argp = arg;

      /* If the current command is one that incurs a data transfer, then we
       * need to do more work.  If not, it's an easy substitution.
       */
      if (session.curr_cmd_id == PR_CMD_ABOR_ID ||
          session.curr_cmd_id == PR_CMD_APPE_ID ||
          session.curr_cmd_id == PR_CMD_LIST_ID ||
          session.curr_cmd_id == PR_CMD_MLSD_ID ||
          session.curr_cmd_id == PR_CMD_NLST_ID ||
          session.curr_cmd_id == PR_CMD_RETR_ID ||
          session.curr_cmd_id == PR_CMD_STOR_ID ||
          session.curr_cmd_id == PR_CMD_STOU_ID) {
        const char *proto;

        proto = pr_session_get_protocol(0);

        if (strncmp(proto, "ftp", 4) == 0 ||
            strncmp(proto, "ftps", 5) == 0) {

          if (!(XFER_ABORTED)) {
            int res;
            char *resp_code = NULL, *resp_msg = NULL;

            /* Get the last response code/message.  We use heuristics here to
             * determine when to use "failed" versus "success".
             */
            res = pr_response_get_last(cmd->tmp_pool, &resp_code, &resp_msg);
            if (res == 0 &&
                resp_code != NULL) {
              if (*resp_code == '2') {

                if (pr_cmd_cmp(cmd, PR_CMD_ABOR_ID) != 0) {
                  sstrncpy(argp, "success", sizeof(arg));

                } else {
                  /* We're handling the ABOR command, so obviously the value
                   * should be 'cancelled'.
                   */
                  sstrncpy(argp, "cancelled", sizeof(arg));
                }

              } else if (*resp_code == '1') {
                /* If the first digit of the response code is 1, then the
                 * response code (for a data transfer command) is probably 150,
                 * which means that the transfer was still in progress (didn't
                 * complete with a 2xx/4xx response code) when we are called
                 * here, which in turn means a timeout kicked in.
                 */
                sstrncpy(argp, "timeout", sizeof(arg));

              } else {
                sstrncpy(argp, "failed", sizeof(arg));
              }

            } else {
              sstrncpy(argp, "success", sizeof(arg));
            }

          } else {
            sstrncpy(argp, "cancelled", sizeof(arg));
          }

        } else {
          /* mod_sftp stashes a note for us in the command notes if the
           * transfer failed.
           */
          char *status;

          status = pr_table_get(cmd->notes, "mod_sftp.file-status", NULL);
          if (status == NULL) {
            sstrncpy(argp, "success", sizeof(arg));

          } else {
            sstrncpy(argp, "failed", sizeof(arg));
          }
        }

      } else {
        sstrncpy(argp, "-", sizeof(arg));
      }

      m++;
      break;
    }

    case LOGFMT_META_VERSION:
      argp = arg;
      sstrncpy(argp, PROFTPD_VERSION_TEXT, sizeof(arg));
      m++;
      break;

    case LOGFMT_META_FILE_MODIFIED: {
      char *modified;

      argp = arg;

      modified = pr_table_get(cmd->notes, "mod_xfer.file-modified", NULL);
      if (modified) {
        sstrncpy(argp, modified, sizeof(arg));

      } else {
        sstrncpy(argp, "false", sizeof(arg));
      }

      m++;
      break;
    }

    case LOGFMT_META_RAW_BYTES_IN:
      argp = arg;
      snprintf(argp, sizeof(arg), "%" PR_LU, (pr_off_t) session.total_raw_in);
      m++;
      break;

    case LOGFMT_META_RAW_BYTES_OUT:
      argp = arg;
      snprintf(argp, sizeof(arg), "%" PR_LU, (pr_off_t) session.total_raw_out);
      m++;
      break;

    case LOGFMT_META_VHOST_IP:
      argp = arg;
      sstrncpy(argp, cmd->server->ServerAddress, sizeof(arg));
      m++;
      break;
  }
 
  *f = m;
  if (argp) {
    return pstrdup(p, argp);
  }

  return NULL;
}

/* from src/log.c */
extern int syslog_sockfd;

static void do_log(cmd_rec *cmd, logfile_t *lf) {
  unsigned char *f = NULL;
  size_t size = EXTENDED_LOG_BUFFER_SIZE-2;
  char logbuf[EXTENDED_LOG_BUFFER_SIZE] = {'\0'};
  logformat_t *fmt = NULL;
  char *s, *bp;
  size_t logbuflen;

  fmt = lf->lf_format;
  f = fmt->lf_format;
  bp = logbuf;

  while (*f && size) {
    pr_signals_handle();

    if (*f == LOGFMT_META_START) {
      s = get_next_meta(cmd->tmp_pool, cmd, &f);
      if (s) {
        size_t tmp;

        tmp = strlen(s);
        if (tmp > size)
          tmp = size;

        memcpy(bp, s, tmp);
        size -= tmp;
        bp += tmp;
      }

    } else {
      *bp++ = (char) *f++;
      size--;
    }
  }

  *bp++ = '\n';
  *bp = '\0';

  logbuflen = (bp - logbuf);

  if (lf->lf_fd != EXTENDED_LOG_SYSLOG) {
    pr_log_event_generate(PR_LOG_TYPE_EXTLOG, lf->lf_fd, -1, logbuf, logbuflen);

    if (write(lf->lf_fd, logbuf, logbuflen) < 0) {
      pr_log_pri(PR_LOG_ALERT, "error: cannot write ExtendedLog to fd %d: %s",
        lf->lf_fd, strerror(errno));
    }

  } else {
    pr_log_event_generate(PR_LOG_TYPE_EXTLOG, syslog_sockfd,
      lf->lf_syslog_level, logbuf, logbuflen);
    pr_syslog(syslog_sockfd, lf->lf_syslog_level, "%s", logbuf);
  }
}

MODRET log_any(cmd_rec *cmd) {
  logfile_t *lf = NULL;

  /* If not in anon mode, only handle logs for main servers */
  for (lf = logs; lf; lf = lf->next) {
    if (lf->lf_fd != -1 &&

        /* If the logging class of this command is one of the classes
         * configured for this ExtendedLog...
         */ 
        ((cmd->cmd_class & lf->lf_classes) ||

         /* ...or if the logging class of this command is unknown (defaults to
          * zero), and this ExtendedLog is configured to log ALL commands, then
          * log it.
          */
         (cmd->cmd_class == 0 && lf->lf_classes == CL_ALL))) {

      if (!session.anon_config &&
          lf->lf_conf &&
          lf->lf_conf->config_type == CONF_ANON)
        continue;

      do_log(cmd, lf);
    }
  }

  return PR_DECLINED(cmd);
}

/* Event handlers
 */

static void log_exit_ev(const void *event_data, void *user_data) {
  cmd_rec *cmd;

  cmd = pr_cmd_alloc(session.pool, 1, "EXIT");
  cmd->cmd_class |= CL_EXIT;

  (void) log_any(cmd);
}

static void log_postparse_ev(const void *event_data, void *user_data) {
  config_rec *c;

  c = find_config(main_server->conf, CONF_PARAM, "SystemLog", FALSE);
  if (c != NULL) {
    char *path;

    path = c->argv[0];
    log_closesyslog();

    if (strncasecmp(path, "none", 5) != 0) {
      int res, xerrno;

      path = dir_canonical_path(main_server->pool, path);

      pr_signals_block();
      PRIVS_ROOT
      res = log_opensyslog(path);
      xerrno = errno;
      PRIVS_RELINQUISH
      pr_signals_unblock();

      if (res < 0) {
        if (res == PR_LOG_WRITABLE_DIR) {
          pr_log_pri(PR_LOG_ERR,
            "unable to open SystemLog '%s': %s is a world-writable directory",
            path, path);

        } else if (res == PR_LOG_SYMLINK) {
          pr_log_pri(PR_LOG_ERR,
            "unable to open SystemLog '%s': %s is a symbolic link", path, path);

        } else {
          if (xerrno != ENXIO) {
            pr_log_pri(PR_LOG_ERR,
              "unable to open SystemLog '%s': %s", path, strerror(xerrno));

          } else {
            pr_log_pri(PR_LOG_ERR,
              "unable to open SystemLog '%s': "
              "FIFO reader process must be running first", path);
          }
        }

        exit(1);
      }

    } else {
      log_discard();
    }
  }
}

static void log_restart_ev(const void *event_data, void *user_data) {
  destroy_pool(log_pool);

  formats = NULL;
  format_set = NULL;
  logs = NULL;
  log_set = NULL;

  log_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(log_pool, "mod_log pool");

  logformat(NULL, "", "%h %l %u %t \"%r\" %s %b");
  return;
}

static void log_xfer_stalled_ev(const void *event_data, void *user_data) {
  if (session.curr_cmd_rec != NULL) {
    /* Automatically dispatch the current command, at the LOG_CMD_ERR phase,
     * so that the ExtendedLog entry for the command gets written out.  This
     * should handle any LIST/MLSD/NLST commands as well (Bug#3696).
     */
    (void) log_any(session.curr_cmd_rec);
  }
}

/* Initialization handlers
 */

static int log_init(void) {
  log_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(log_pool, "mod_log pool");

  /* Add the "default" extendedlog format */
  logformat(NULL, "", "%h %l %u %t \"%r\" %s %b");

  pr_event_register(&log_module, "core.postparse", log_postparse_ev, NULL);
  pr_event_register(&log_module, "core.restart", log_restart_ev, NULL);
  return 0;
}

static void find_extendedlogs(void) {
  config_rec *c;
  char *logfname, *logfmt_s;
  int logclasses = CL_ALL;
  logformat_t *logfmt;
  logfile_t *extlog = NULL;
  unsigned long config_flags = (PR_CONFIG_FIND_FL_SKIP_DIR|PR_CONFIG_FIND_FL_SKIP_LIMIT|PR_CONFIG_FIND_FL_SKIP_DYNDIR);

  /* We _do_ actually want the recursion here.  The reason is that we want
   * to find _all_ ExtendedLog directives in the configuration, including
   * those in <Anonymous> sections.  We have the ability to use root privs
   * now, to make sure these files can be opened, but after the user has
   * authenticated (and we know for sure whether they're anonymous or not),
   * root privs may be permanently revoked.
   *
   * We mitigate the cost of the recursive search (especially for configs
   * with thousands of <Directory>/<Limit> sections) by specifying the
   * find_config() flags to skip those sections; we are only interested
   * in the top-level (CONF_ROOT, CONF_VIRTUAL) and <Anonymous> sections.
   */

  c = find_config2(main_server->conf, CONF_PARAM, "ExtendedLog", TRUE,
    config_flags);
  while (c != NULL) {
    pr_signals_handle();

    logfname = c->argv[0];
    logfmt_s = NULL;

    if (c->argc > 1) {
      logclasses = *((int *) c->argv[1]);

      if (c->argc > 2) {
        logfmt_s = c->argv[2];
      }
    }

    /* No logging for this round.  If, however, this was found in an
     * <Anonymous> section, add a logfile entry for it anyway; the anonymous
     * directive might be trying to override a higher-level config; see
     * Bug#1908.
     */
    if (logclasses == CL_NONE &&
        (c->parent != NULL && c->parent->config_type != CONF_ANON)) {
      goto loop_extendedlogs;
    }

    if (logfmt_s) {
      /* search for the format-nickname */
      for (logfmt = formats; logfmt; logfmt = logfmt->next) {
        if (strcmp(logfmt->lf_nickname, logfmt_s) == 0) {
          break;
        }
      }

      if (logfmt == NULL) {
        pr_log_pri(PR_LOG_NOTICE,
          "ExtendedLog '%s' uses unknown format nickname '%s'", logfname,
          logfmt_s);
        goto loop_extendedlogs;
      }

    } else {
      logfmt = formats;
    }

    extlog = (logfile_t *) pcalloc(session.pool, sizeof(logfile_t));

    extlog->lf_filename = pstrdup(session.pool, logfname);
    extlog->lf_fd = -1;
    extlog->lf_syslog_level = -1;
    extlog->lf_classes = logclasses;
    extlog->lf_format = logfmt;
    extlog->lf_conf = c->parent;
    if (log_set == NULL) {
      log_set = xaset_create(session.pool, NULL);
    }

    xaset_insert(log_set, (xasetmember_t *) extlog);
    logs = (logfile_t *) log_set->xas_list;

loop_extendedlogs:
    c = find_config_next2(c, c->next, CONF_PARAM, "ExtendedLog", TRUE,
      config_flags);
  }
}

MODRET log_pre_dele(cmd_rec *cmd) {
  char *path;

  log_dele_filesz = 0;

  path = dir_canonical_path(cmd->tmp_pool,
    pr_fs_decode_path(cmd->tmp_pool, cmd->arg));
  if (path) {
    struct stat st;

    /* Briefly cache the size of the file being deleted, so that it can be
     * logged properly using %b.
     */
    pr_fs_clear_cache();
    if (pr_fsio_stat(path, &st) == 0)
      log_dele_filesz = st.st_size;
  }

  return PR_DECLINED(cmd);
}

MODRET log_post_host(cmd_rec *cmd) {

  /* If the HOST command changed the main_server pointer, reinitialize
   * ourselves.
   */
  if (session.prev_server != NULL) {
    int res;
    logfile_t *lf = NULL;

    pr_event_unregister(&log_module, "core.exit", log_exit_ev);
    pr_event_unregister(&log_module, "core.timeout-stalled",
      log_xfer_stalled_ev);

    /* XXX If ServerLog configured, close/reopen syslog? */

    /* XXX Close all ExtendedLog files, to prevent duplicate fds. */
    for (lf = logs; lf; lf = lf->next) {
      if (lf->lf_fd > -1) {
        /* No need to close the special EXTENDED_LOG_SYSLOG (i.e. fake) fd. */
        if (lf->lf_fd != EXTENDED_LOG_SYSLOG) {
          (void) close(lf->lf_fd);
        }

        lf->lf_fd = -1;
      }
    }

    res = log_sess_init();
    if (res < 0) {
      pr_session_disconnect(&log_module,
        PR_SESS_DISCONNECT_SESSION_INIT_FAILED, NULL);
    }
  }

  return PR_DECLINED(cmd);
}

MODRET log_post_pass(cmd_rec *cmd) {
  logfile_t *lf;

  /* Authentication is complete, if we aren't in anon-mode, close
   * all extendedlogs opened inside <Anonymous> blocks.
   */
  if (!session.anon_config) {
    for (lf = logs; lf; lf = lf->next) {
      if (lf->lf_fd != -1 &&
          lf->lf_fd != EXTENDED_LOG_SYSLOG &&
          lf->lf_conf &&
          lf->lf_conf->config_type == CONF_ANON) {
        pr_log_debug(DEBUG7, "mod_log: closing ExtendedLog '%s' (fd %d)",
          lf->lf_filename, lf->lf_fd);
        close(lf->lf_fd);
        lf->lf_fd = -1;
      }
    }

  } else {
    /* Close all logs which were opened inside a _different_ anonymous
     * context.
     */
    for (lf = logs; lf; lf = lf->next) {
      if (lf->lf_fd != -1 &&
          lf->lf_fd != EXTENDED_LOG_SYSLOG &&
          lf->lf_conf &&
          lf->lf_conf != session.anon_config) {
        pr_log_debug(DEBUG7, "mod_log: closing ExtendedLog '%s' (fd %d)",
          lf->lf_filename, lf->lf_fd);
        close(lf->lf_fd);
        lf->lf_fd = -1;
      }
    }

    /* If any ExtendedLogs set inside our context match an outer log,
     * close the outer (this allows overriding inside <Anonymous>).
     */
    for (lf = logs; lf; lf = lf->next) {
      if (lf->lf_conf &&
          lf->lf_conf == session.anon_config) {
        /* This should "override" any lower-level extendedlog with the
         * same filename.
         */
        logfile_t *lfi = NULL;

        for (lfi = logs; lfi; lfi = lfi->next) {
          if (lfi->lf_fd != -1 &&
              lfi->lf_fd != EXTENDED_LOG_SYSLOG &&
              !lfi->lf_conf &&
              strcmp(lfi->lf_filename, lf->lf_filename) == 0) {
            pr_log_debug(DEBUG7, "mod_log: closing ExtendedLog '%s' (fd %d)",
              lf->lf_filename, lfi->lf_fd);
            close(lfi->lf_fd);
            lfi->lf_fd = -1;
          }
        }

        /* Go ahead and close the log if it's CL_NONE */
        if (lf->lf_fd != -1 &&
            lf->lf_fd != EXTENDED_LOG_SYSLOG &&
            lf->lf_classes == CL_NONE) {
          pr_log_debug(DEBUG7, "mod_log: closing ExtendedLog '%s' (fd %d)",
            lf->lf_filename, lf->lf_fd);
          close(lf->lf_fd);
          lf->lf_fd = -1;
        }
      }
    }
  }

  return PR_DECLINED(cmd);
}

/* Open all the log files */
static int log_sess_init(void) {
  char *serverlog_name = NULL;
  logfile_t *lf = NULL;

  /* Open the ServerLog, if present. */
  serverlog_name = get_param_ptr(main_server->conf, "ServerLog", FALSE);
  if (serverlog_name != NULL) {
    log_closesyslog();

    if (strncasecmp(serverlog_name, "none", 5) != 0) {
      int res, xerrno;

      PRIVS_ROOT
      res = log_opensyslog(serverlog_name);
      xerrno = errno;
      PRIVS_RELINQUISH

      if (res < 0) {
        if (xerrno != ENXIO) {
          pr_log_debug(DEBUG4, "unable to open ServerLog '%s': %s",
            serverlog_name, strerror(xerrno));

        } else {
          pr_log_debug(DEBUG4,
            "unable to open ServerLog '%s': "
            "FIFO reader process must be running first", serverlog_name);
        }
      }
    }

  } else {
    config_rec *c;

    c = find_config(main_server->conf, CONF_PARAM, "SystemLog", FALSE);
    if (c != NULL) {
      char *path;

      path = c->argv[0];
      log_closesyslog();

      if (strncasecmp(path, "none", 5) != 0) {
        int res, xerrno;

        path = dir_canonical_path(main_server->pool, path);

        pr_signals_block();
        PRIVS_ROOT
        res = log_opensyslog(path);
        xerrno = errno;
        PRIVS_RELINQUISH
        pr_signals_unblock();

        if (res < 0) {
          if (res == PR_LOG_WRITABLE_DIR) {
            pr_log_pri(PR_LOG_ERR,
              "unable to open SystemLog '%s': %s is a world-writable directory",
              path, path);

          } else if (res == PR_LOG_SYMLINK) {
            pr_log_pri(PR_LOG_ERR,
              "unable to open SystemLog '%s': %s is a symbolic link", path,
              path);

          } else {
            if (xerrno != ENXIO) {
              pr_log_pri(PR_LOG_ERR,
                "unable to open SystemLog '%s': %s", path, strerror(xerrno));

            } else {
              pr_log_pri(PR_LOG_ERR,
                "unable to open SystemLog '%s': "
                "FIFO reader process must be running first", path);
            }
          }
        }

      } else {
        log_discard();
      }
    }
  }

  /* Open all the ExtendedLog files. */
  find_extendedlogs();

  for (lf = logs; lf; lf = lf->next) {
    if (lf->lf_fd == -1) {

      /* Is this ExtendedLog to be written to a file, or to syslog? */
      if (strncasecmp(lf->lf_filename, "syslog:", 7) != 0) {
        int res = 0, xerrno;

        pr_log_debug(DEBUG7, "mod_log: opening ExtendedLog '%s'",
          lf->lf_filename);

        pr_signals_block();
        PRIVS_ROOT
        res = pr_log_openfile(lf->lf_filename, &(lf->lf_fd), EXTENDED_LOG_MODE);
        xerrno = errno;
        PRIVS_RELINQUISH
        pr_signals_unblock();

        if (res < 0) {
          if (res == -1) {
            if (xerrno != ENXIO) {
              pr_log_pri(PR_LOG_NOTICE, "unable to open ExtendedLog '%s': %s",
                lf->lf_filename, strerror(xerrno));

            } else {
              pr_log_pri(PR_LOG_NOTICE, "unable to open ExtendedLog '%s': "
                "FIFO reader process must be running first", lf->lf_filename);
            }

          } else if (res == PR_LOG_WRITABLE_DIR) {
            pr_log_pri(PR_LOG_WARNING, "unable to open ExtendedLog '%s': "
              "parent directory is world-writable", lf->lf_filename);

          } else if (res == PR_LOG_SYMLINK) {
            pr_log_pri(PR_LOG_WARNING, "unable to open ExtendedLog '%s': "
              "%s is a symbolic link", lf->lf_filename, lf->lf_filename);
          }
        }

      } else {
        char *tmp = strchr(lf->lf_filename, ':');

        lf->lf_syslog_level = pr_log_str2sysloglevel(++tmp);
        lf->lf_fd = EXTENDED_LOG_SYSLOG;
      }
    }
  }

  /* Register event handlers for the session. */
  pr_event_register(&log_module, "core.exit", log_exit_ev, NULL);
  pr_event_register(&log_module, "core.timeout-stalled", log_xfer_stalled_ev,
    NULL);

  return 0;
}

/* Module API tables
 */

static conftable log_conftab[] = {
  { "AllowLogSymlinks",	set_allowlogsymlinks,			NULL },
  { "ExtendedLog",	set_extendedlog,			NULL },
  { "LogFormat",	set_logformat,				NULL },
  { "ServerLog",	set_serverlog,				NULL },
  { "SystemLog",	set_systemlog,				NULL },
  { NULL,		NULL,					NULL }
};

static cmdtable log_cmdtab[] = {
  { PRE_CMD,		C_DELE,	G_NONE,	log_pre_dele,	FALSE, FALSE },
  { LOG_CMD,		C_ANY,	G_NONE,	log_any,	FALSE, FALSE },
  { LOG_CMD_ERR,	C_ANY,	G_NONE,	log_any,	FALSE, FALSE },
  { POST_CMD,		C_HOST,	G_NONE,	log_post_host,	FALSE, FALSE },
  { POST_CMD,		C_PASS,	G_NONE,	log_post_pass,	FALSE, FALSE },
  { 0, NULL }
};

module log_module = {
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "log",

  /* Module configuration handler table */
  log_conftab,

  /* Module command handler table */
  log_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization */
  log_init,

  /* Session initialization */
  log_sess_init
};
