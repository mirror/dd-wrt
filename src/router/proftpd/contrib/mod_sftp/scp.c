/*
 * ProFTPD - mod_sftp SCP
 * Copyright (c) 2008-2010 TJ Saunders
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
 * $Id: scp.c,v 1.37 2010/02/04 04:02:12 castaglia Exp $
 */

#include "mod_sftp.h"
#include "ssh2.h"
#include "packet.h"
#include "msg.h"
#include "channel.h"
#include "scp.h"

#define SFTP_SCP_ST_MODE_MASK	(S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)

struct scp_path {
  char *path;
  pr_fh_t *fh;

  /* Points to the directory "context" path, if we're receiving a file
   * from a client sub-directory and we're in recursive mode.
   */
  struct scp_path *recv_dir;

  /* Points to the previous directory "context" path, if any.  For handling
   * the push/pop approach that SCP uses for receiving directories.
   */
  struct scp_path *prev_dir;

  /* Track state of file metadata we've received. */
  int checked_errors;

  int have_mode;
  mode_t st_mode;

  struct timeval times[2];
  int recvd_timeinfo;

  mode_t perms;
  off_t filesz;
  const char *filename;
  const char *best_path;
  int recvd_finfo;
  int recvd_data;

  /* For the reading of bytes of files. */
  off_t recvlen;

  int wrote_errors;

  /* Track state of how much file metadata we've sent. */
  int sent_timeinfo;
  int sent_dirinfo;
  int sent_finfo;
  int sent_data;

  /* For sending the bytes of files. */
  off_t sentlen;

  /* For directories. */
  void *dirh;
  struct scp_path *dir_spi;

  /* For supporting the HiddenStores directive. */
  int hiddenstore;
};

static pool *scp_pool = NULL;

/* Use a struct to maintain the per-channel SCP-specific values. */
struct scp_session {
  struct scp_session *next, *prev;

  pool *pool;
  uint32_t channel_id;
  array_header *dirs;
  array_header *paths;
  unsigned int path_idx;
};

static struct scp_session *scp_session = NULL, *scp_sessions = NULL;

/* This structure is a container, for holding the paths and index until
 * the session object for the channel is opened.  sftp_scp_set_params(),
 * which parses out the paths, is called _before_ sftp_scp_open_session(),
 * hence why we need to track these separately.
 */

struct scp_paths {
  struct scp_paths *next, *prev;

  pool *pool;
  uint32_t channel_id;
  array_header *paths;
  unsigned int path_idx;
};

static struct scp_paths *scp_paths = NULL;

static unsigned int scp_opts = 0;
#define SFTP_SCP_OPT_ISSRC	0x0001
#define SFTP_SCP_OPT_ISDST	0x0002
#define SFTP_SCP_OPT_DIR	0x0004
#define SFTP_SCP_OPT_VERBOSE	0x0008
#define SFTP_SCP_OPT_PRESERVE	0x0010
#define SFTP_SCP_OPT_RECURSE	0x0020

/* Boolean flag indicating whether we need to wait for the confirmation
 * response (byte) from the client before proceeding.
 */
static int need_confirm; 

static const char *trace_channel = "scp";

static int send_path(pool *, uint32_t, struct scp_path *);

static cmd_rec *scp_cmd_alloc(pool *p, const char *name, const char *arg) {
  cmd_rec *cmd;

  cmd = pr_cmd_alloc(p, 2, pstrdup(p, name), arg ? arg : "");
  cmd->arg = (char *) arg;
  cmd->tmp_pool = pr_pool_create_sz(p, 64);

  return cmd;
}

static int scp_destroy_paths(struct scp_paths *paths) {
  if (paths == NULL) {
    return 0;
  }

  if (paths->next)
    paths->next->prev = paths->prev;

  if (paths->prev) {
    paths->prev->next = paths->next;

  } else {
    scp_paths = paths->next;
  }

  destroy_pool(paths->pool);
  return 0;
}

static struct scp_paths *scp_new_paths(uint32_t channel_id) {
  pool *sub_pool;
  struct scp_paths *paths, *last;

  /* Check to see if we already have an paths object for this channel ID. */
  paths = last = scp_paths;
  while (paths) {
    pr_signals_handle();

    if (paths->channel_id == channel_id) {
      errno = EEXIST;
      return NULL;
    }

    if (paths->next == NULL) {
      /* This is the last item in the list. */
      last = paths;
    }

    paths = paths->next;
  }

  /* Looks like we get to allocate a new one. */
  sub_pool = make_sub_pool(scp_pool);
  pr_pool_tag(sub_pool, "SCP paths pool");

  paths = pcalloc(sub_pool, sizeof(struct scp_paths));
  paths->pool = sub_pool;
  paths->channel_id = channel_id;

  if (last) {
    last->next = paths;
    paths->prev = last;

  } else {
    scp_paths = paths;
  }

  return paths;
}

static struct scp_paths *scp_get_paths(uint32_t channel_id) {
  struct scp_paths *paths;

  paths = scp_paths;
  while (paths) {
    pr_signals_handle();

    if (paths->channel_id == channel_id) {
      return paths;
    }

    paths = paths->next;
  }

  errno = ENOENT;
  return NULL;
}

static struct scp_session *scp_get_session(uint32_t channel_id) {
  struct scp_session *sess;

  sess = scp_sessions;
  while (sess) {
    pr_signals_handle();

    if (sess->channel_id == channel_id) {
      return sess;
    }

    sess = sess->next;
  }

  errno = ENOENT;
  return NULL;
}

static void reset_path(struct scp_path *sp) {
  if (sp->fh) {
    pr_fsio_close(sp->fh);
    sp->fh = NULL;
  }

  /* XXX Should clear/reset the sent fields as well, but this function
   * is mainly for use when receiving files, not sending files.
   */

  sp->checked_errors = FALSE;

  sp->st_mode = 0;
  sp->have_mode = FALSE;
  sp->recvd_timeinfo = FALSE;

  sp->perms = 0;
  sp->filesz = 0;
  sp->filename = NULL;
  sp->best_path = NULL;
  sp->recvd_finfo = FALSE;
  sp->recvd_data = FALSE;

  sp->recvlen = 0;
  sp->hiddenstore = FALSE;

  sp->wrote_errors = FALSE;
}

static int read_confirm(struct ssh2_packet *pkt, char **buf,
    uint32_t *buflen) {
  char code;

  code = sftp_msg_read_byte(pkt->pool, buf, buflen);
  pr_trace_msg(trace_channel, 9, "recvd confirmation/error code = %d", code);

  switch (code) {
    case 0:
      break;

    case 1: {
      register unsigned int i;
      char *msg;

      /* Error; message to follow. Since it won't be encoded as an SSH2 string,
       * we will need to read it character by character.  Whee.
       */

      msg = pcalloc(pkt->pool, *buflen + 1);
      for (i = 0; *buflen; ) {
        char c;

        c = sftp_msg_read_byte(pkt->pool, buf, buflen);
        if (c == '\n') {
          break;
        }

        msg[i++] = c;
      }

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error from client: %s", msg);
      return -1;
    }

    case 2:
      /* Fatal error, no message. */
      return -1;
  }

  need_confirm = FALSE;
  return 0;
}

static int write_confirm(pool *p, uint32_t channel_id, int code,
    const char *msg) {
  char *buf, *ptr;
  uint32_t buflen, bufsz;

  /* XXX Is this big enough?  Too big? */
  buflen = bufsz = 128;
  buf = ptr = palloc(p, bufsz);

  if (code == 0) {
    pr_trace_msg(trace_channel, 9, "sending confirmation/error code = %d",
      code);

  } else {
    pr_trace_msg(trace_channel, 9, "sending confirmation/error code = %d (%s)",
      code, msg ? msg : "null");
  }

  if (code == 0) {
    sftp_msg_write_byte(&buf, &buflen, code);

  } else {
    char *errstr;
    size_t errlen;

    errstr = pstrcat(p, msg, "\n", NULL);
    errlen = strlen(errstr);

    sftp_msg_write_byte(&buf, &buflen, code);
    sftp_msg_write_data(&buf, &buflen, errstr, errlen, FALSE);
  }

  return sftp_channel_write_data(p, channel_id, ptr, (bufsz - buflen));
}

/* Functions for receiving files from the client. */

static int recv_errors(pool *p, uint32_t channel_id, struct scp_path *sp,
    char *data, uint32_t datalen) {

  /* Check for error messages from the client first. */
  if (data[0] == '\01') {
    register unsigned int i;
    char *msg;

    for (i = 1; i < datalen; i++) {
      if (data[i] == '\n') {
        break;
      }
    }

    if (i < datalen) {
      msg = pstrndup(p, &(data[1]), i + 1);

    } else {
      msg = pcalloc(p, i + 1);
        memcpy(msg, &(data[1]), i);
    }

    pr_trace_msg(trace_channel, 3,
      "received error '%s' from client while receiving path '%s', skipping",
      msg, sp->path);

    sp->checked_errors = TRUE;
    return 1;
  }

  if (data[0] == '\02') {
    pr_trace_msg(trace_channel, 3,
      "received fatal error from client while receiving path '%s', skipping",
      sp->path);

    sp->checked_errors = TRUE;
    return 1;
  }

  sp->checked_errors = TRUE;
  return 0;
}

static int recv_timeinfo(pool *p, uint32_t channel_id, struct scp_path *sp,
    char *data, uint32_t datalen) {
  register unsigned int i;
  char *msg, *ptr = NULL, *tmp = NULL;

  if (data[0] != 'T') {
    pr_trace_msg(trace_channel, 2, "expected T control message, got '%c'",
      data[0]);
    write_confirm(p, channel_id, 1,
      pstrcat(p, sp->path, ": expected control message", NULL));
    return 1;
  }

  for (i = 1; i < datalen; i++) {
    if (data[i] == '\n') {
      ptr = &data[i++];
      break;
    }
  }

  msg = data + 1;

  if (ptr)
    *ptr = '\0';

  pr_trace_msg(trace_channel, 5, "'%s' control message: T%s", sp->path, msg);

  sp->times[1].tv_sec = strtoul(msg, &tmp, 10);
  if (tmp == NULL ||
      *tmp != ' ') {
    write_confirm(p, channel_id, 1, "mtime secs not delimited");
    return 1;
  }

  msg = tmp + 1;
  sp->times[1].tv_usec = strtoul(msg, &tmp, 10);
  if (tmp == NULL ||
      *tmp != ' ') {
    write_confirm(p, channel_id, 1, "mtime usecs not delimited");
    return 1;
  }

  msg = tmp + 1;
  sp->times[0].tv_sec = strtoul(msg, &tmp, 10);
  if (tmp == NULL ||
      *tmp != ' ') {
    write_confirm(p, channel_id, 1, "atime secs not delimited");
    return 1;
  }

  msg = tmp + 1;
  sp->times[0].tv_usec = strtoul(msg, &tmp, 10);
  if (tmp == NULL ||
      *tmp != '\0') {
    write_confirm(p, channel_id, 1, "atime usecs not delimited");
    return 1;
  }

  sp->recvd_timeinfo = TRUE;
  write_confirm(p, channel_id, 0, NULL);
  return 0;
}

static int recv_perms(pool *p, uint32_t channel_id, char *mode_str,
    mode_t *perms) {
  register unsigned int i;

  if (strlen(mode_str) < 5) {
    /* This needs to be at least 5 characters: 4 mode digits, and space. */
    pr_trace_msg(trace_channel, 2, "mode string too short: '%s'", mode_str);
    write_confirm(p, channel_id, 1, "bad mode");
    return -1;
  }

  for (i = 0; i < 4; i++) {
    /* Make sure the characters are numeric, and in the octal range. */
    if (mode_str[i] < '0' ||
        mode_str[i] > '7') {
      pr_trace_msg(trace_channel, 2, "non-octal mode character in '%s'",
        mode_str);
      *perms = 0;
      write_confirm(p, channel_id, 1, "bad mode");
      return -1;
    }

    *perms = (*perms << 3) | (mode_str[i] - '0');
  }

  /* Make sure the next character in the string is a space. */
  if (mode_str[i] != ' ') {
    pr_trace_msg(trace_channel, 2, "mode not followed by space delimiter");
    write_confirm(p, channel_id, 1, "mode not delimited");
    return -1;
  }

  pr_trace_msg(trace_channel, 8, "client sent file perms: %04o",
    (unsigned int) *perms);
  return 0;
}

static int recv_filesz(pool *p, uint32_t channel_id, char *size_str,
    off_t *filesz) {
  register unsigned int i;
  size_t size_strlen;

  size_strlen = strlen(size_str);

  /* The file size field could be of arbitrary length. */
  for (i = 0, *filesz = 0; isdigit((int) size_str[i]); i++) {
    pr_signals_handle();

    *filesz = (*filesz * 10) + (size_str[i] - '0');
  }

  if (size_str[i] != ' ') {
    pr_trace_msg(trace_channel, 2, "file size not followed by space delimiter");
    write_confirm(p, channel_id, 1, "file size not delimited");
    return -1;
  }

  pr_trace_msg(trace_channel, 8, "client sent file size: %" PR_LU " bytes",
    (pr_off_t) *filesz);
  return 0;
}

static int recv_filename(pool *p, uint32_t channel_id, char *name_str,
    struct scp_path *sp) {

  if (strchr(name_str, '/') != NULL ||
      strcmp(name_str, "..") == 0) {
    pr_trace_msg(trace_channel, 2, "bad filename: '%s'", name_str);
    write_confirm(p, channel_id, 1,
      pstrcat(p, "unexpected filename: ", name_str, NULL));
    return -1;
  }

  if (sp->recv_dir == NULL) {
    if (!S_ISDIR(sp->st_mode)) {
      sp->filename = pstrdup(scp_pool, name_str); 

    } else {
      sp->filename = pdircat(scp_pool, sp->path, name_str, NULL);
    }

  } else {
    sp->filename = pdircat(scp_pool, sp->recv_dir->path, name_str, NULL);
  }

  if (sp->filename) {
    char *ptr;

    ptr = strrchr(sp->path, '/');
    if (ptr == NULL) {
      sp->best_path = dir_canonical_vpath(scp_pool, sp->filename);

    } else {

      /* What if the client initially sent an scp command like:
       *
       *  scp -t dir/subdir/
       *
       * for an incoming 'test.txt' file?  To handle this, we append
       * the given name to the path sent in the initial scp command.
       *
       * However, we only want to do this if the given name_str differs
       * from the last path component in sp->path.  Otherwise the client
       * could send:
       *
       *  scp -t dir/subdir/file
       *
       * and the file info control message might include a filename of
       * "file".  If we simply append, then we get "dir/subdir/file/file"
       * which is obviously not correct.
       */

      if (strcmp(name_str, ptr + 1) != 0) {
        sp->best_path = dir_canonical_vpath(scp_pool,
          pdircat(p, sp->path, name_str, NULL));

      } else {
        sp->best_path = dir_canonical_vpath(scp_pool, sp->path);
      }
    }

    /* Update the session.xfer.path value with this better, fuller path. */
    session.xfer.path = pstrdup(session.xfer.p, sp->best_path);
  }

  pr_trace_msg(trace_channel, 8,
    "client sent filename '%s' (path '%s')", name_str, sp->best_path);
  return 0;
}

static int recv_finfo(pool *p, uint32_t channel_id, struct scp_path *sp,
    char *data, uint32_t datalen) {
  register unsigned int i;
  char *hiddenstore_path = NULL, *msg, *ptr = NULL;
  int have_dir = FALSE;
  cmd_rec *cmd = NULL;

  switch (data[0]) {
    case 'C':
      break;
 
    case 'D':
      if (!(scp_opts & SFTP_SCP_OPT_RECURSE)) {
        pr_trace_msg(trace_channel, 3,
          "received D control message for '%s' without RECURSE set, "
          "rejecting", sp->path);
        write_confirm(p, channel_id, 1,
          pstrcat(p, sp->path, ": cannot use directory (no -r option)", NULL));
        return 1;
      }

      have_dir = TRUE;
      break;

    default:
      pr_trace_msg(trace_channel, 3,
        "expected file info control message for '%s', got '%c'",
        sp->path, data[0]);
      write_confirm(p, channel_id, 1,
        pstrcat(p, sp->path, ": expected control message", NULL));
      return 1;
  }

  for (i = 1; i < datalen; i++) {
    if (data[i] == '\n') {
      ptr = &data[i++];
      break;
    }
  }

  msg = data + 1;
  *ptr = '\0';

  pr_trace_msg(trace_channel, 5, "'%s' control message: %c%s", sp->path,
    !have_dir ? 'C' : 'D', msg);

  ptr = msg;
  if (recv_perms(p, channel_id, ptr, &sp->perms) < 0)
    return 1;

  ptr = strchr(ptr, ' ');
  if (ptr == NULL) {
    pr_trace_msg(trace_channel, 3,
      "bad control message (undelimited mode)");
    write_confirm(p, channel_id, 1,
      pstrcat(p, sp->path, ": bad control message (undelimited mode)", NULL));
    return 1;
  }

  /* Advance past the space delimiter. */
  ptr++;
  if (recv_filesz(p, channel_id, ptr, &sp->filesz) < 0)
    return 1;

  ptr = strchr(ptr, ' ');
  if (ptr == NULL) {
    pr_trace_msg(trace_channel, 3,
      "bad control message (undelimited file size)");
    write_confirm(p, channel_id, 1,
      pstrcat(p, sp->path, ": bad control message (undelimited file size)",
      NULL));
    return 1;
  }

  /* Advance past the space delimiter. */
  ptr++;
  if (recv_filename(p, channel_id, ptr, sp) < 0)
    return 1;

  if (i != datalen) {
    pr_trace_msg(trace_channel, 1, "more data in this message!");
  }

  sp->recvd_finfo = TRUE;

  if (have_dir) {
    struct stat st;
    struct scp_path *sp2;

    sp2 = pcalloc(scp_pool, sizeof(struct scp_path));

    if (pr_fsio_stat(sp->filename, &st) < 0) {
      pr_trace_msg(trace_channel, 5, "creating directory '%s'", sp->filename);

      if (pr_fsio_mkdir(sp->filename, 0777) < 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "scp: error creating directory '%s': %s", sp->filename,
          strerror(errno));
        write_confirm(p, channel_id, 1,
          pstrcat(p, sp->filename, ": ", strerror(errno), NULL));
        return 1;
      }

    } else {
      /* Make sure that the path actually is a directory. */
      if (!S_ISDIR(st.st_mode)) {
        pr_trace_msg(trace_channel, 2, "error handling '%s': %s",
          sp->best_path, strerror(ENOTDIR));
        write_confirm(p, channel_id, 1,
          pstrcat(p, sp->filename, ": ", strerror(ENOTDIR), NULL));
        return 1;
      }
    }

    if (scp_session->dirs == NULL) {
      scp_session->dirs = make_array(scp_pool, 1, sizeof(struct scp_path *));
    }

    *((struct scp_path **) push_array(scp_session->dirs)) = sp2;
    sp2->path = pstrdup(scp_pool, sp->filename);
    sp2->filename = pstrdup(scp_pool, sp->filename);
    sp2->best_path = pstrdup(scp_pool, sp->best_path);

    /* Copy any timeinfo as well. */
    sp2->times[0].tv_sec = sp->times[0].tv_sec;
    sp2->times[0].tv_usec = sp->times[0].tv_usec;
    sp2->times[1].tv_sec = sp->times[1].tv_sec;
    sp2->times[1].tv_usec = sp->times[1].tv_usec;
    sp2->recvd_timeinfo = sp->recvd_timeinfo;

    /* And the perms. */
    sp2->perms = sp->perms;

    sp->prev_dir = sp->recv_dir;
    sp->recv_dir = sp2;

    /* Reset sp, for re-use for the next file coming in. */
    reset_path(sp);

    write_confirm(p, channel_id, 0, NULL);
    return 0;
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "scp upload", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", sp->best_path, NULL, NULL);

  cmd = scp_cmd_alloc(p, C_STOR, sp->best_path);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "scp upload to '%s' blocked by '%s' handler", sp->path,
      cmd->argv[0]);

    (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    write_confirm(p, channel_id, 1,
      pstrcat(p, sp->filename, ": ", strerror(EACCES), NULL));

    return 1;
  }

  if (strcmp(sp->filename, cmd->arg) != 0) {
    sp->filename = cmd->arg;
    sp->best_path = dir_canonical_vpath(scp_pool, sp->filename);
  }

  if (session.xfer.xfer_type == STOR_HIDDEN) {
    hiddenstore_path = pr_table_get(cmd->notes, "mod_xfer.store-hidden-path",
      NULL);
  }

  if (!dir_check(p, cmd, G_WRITE, (char *) sp->best_path, NULL)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "scp upload to '%s' blocked by <Limit> configuration", sp->best_path);

    (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    write_confirm(p, channel_id, 1,
      pstrcat(p, sp->filename, ": ", strerror(EACCES), NULL));

    return 1;
  }

  /* We automatically add the O_NONBLOCK flag to the set of open() flags
   * in order to deal with writing to a FIFO whose other end may not be
   * open.  Then, after a successful open, we return the file to blocking
   * mode.
   */

  sp->fh = pr_fsio_open(hiddenstore_path ? hiddenstore_path : sp->best_path,
    O_WRONLY|O_CREAT|O_NONBLOCK);
  if (sp->fh == NULL) {
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
      "error opening '%s': %s", "scp upload", session.user,
      (unsigned long) session.uid, (unsigned long) session.gid,
      hiddenstore_path ? hiddenstore_path : sp->best_path, strerror(xerrno));

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "scp: error opening '%s': %s",
      hiddenstore_path ? hiddenstore_path : sp->best_path, strerror(xerrno));

    (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    write_confirm(p, channel_id, 1,
      pstrcat(p, sp->filename, ": ", strerror(xerrno), NULL));

    errno = xerrno;
    return 1;
  }

  if (hiddenstore_path) {
    sp->hiddenstore = TRUE;
  }

  pr_fsio_set_block(sp->fh);

  write_confirm(p, channel_id, 0, NULL);
  return 0;
}

static int recv_data(pool *p, uint32_t channel_id, struct scp_path *sp,
    char *data, uint32_t datalen) {
  uint32_t writelen;

  writelen = datalen;
  if (writelen > (sp->filesz - sp->recvlen)) {
    writelen = (sp->filesz - sp->recvlen);
  }

  if (writelen > 0) {
    while (TRUE) {
      if (pr_fsio_write(sp->fh, data, writelen) != writelen) {
        if (errno == EAGAIN) {
          pr_signals_handle();
          continue;
        }

        pr_trace_msg(trace_channel, 2, "error writing to '%s': %s",
          sp->best_path, strerror(errno));
        write_confirm(p, channel_id, 1,
          pstrcat(p, sp->filename, ": write error: ", strerror(errno), NULL));

        pr_fsio_close(sp->fh);
        sp->fh = NULL;
        return 1;
      }

      break;
    }

    sp->recvlen += writelen;

    session.xfer.total_bytes += writelen;
    session.total_bytes += writelen;

    if (writelen < datalen) {
      if (data[writelen] != '\0') {
        pr_trace_msg(trace_channel, 2, "expected end-of-data marker when "
          "receiving file data, received '%c'", data[writelen]);
      }

      pr_throttle_pause(sp->recvlen, TRUE);

      sp->recvd_data = TRUE;
      return 1;
    }

    pr_throttle_pause(sp->recvlen, FALSE);

  } else {
    /* We should have just one extra end-of-stream byte. */
    if (data[writelen] != '\0') {
      pr_trace_msg(trace_channel, 2, "expected end-of-data marker when "
        "receiving file data, received '%c'", data[writelen]);
    }

    pr_throttle_pause(sp->recvlen, TRUE);

    sp->recvd_data = TRUE;
    return 1;
  }

  return 0;
}

static int recv_eod(pool *p, uint32_t channel_id, struct scp_path *sp,
    char *data, uint32_t datalen) {
  struct scp_path *dir_sp;
  int ok = TRUE;

  if (data[0] != 'E') {
    return 0;
  }

  pr_trace_msg(trace_channel, 5, "'%s' control message: E", sp->path);

  if (sp->recv_dir == NULL) {
    pr_trace_msg(trace_channel, 1, "received E message for path, but it has no receiving directory context!");
    return 0;
  }

  dir_sp = sp->recv_dir;

  pr_trace_msg(trace_channel, 9, "setting perms %04o on directory '%s'",
    (unsigned int) dir_sp->perms, dir_sp->path);
  if (pr_fsio_chmod(dir_sp->path, dir_sp->perms) < 0) {
    pr_trace_msg(trace_channel, 2, "error setting mode %04o on '%s': %s",
      (unsigned int) dir_sp->perms, dir_sp->path, strerror(errno));
    write_confirm(p, channel_id, 1,
      pstrcat(p, dir_sp->path, ": error setting mode: ", strerror(errno),
      NULL));
    ok = FALSE;
  }

  if (dir_sp->recvd_timeinfo) {
    pr_trace_msg(trace_channel, 9, "setting times on directory '%s'",
      dir_sp->filename);

    if (pr_fsio_utimes(dir_sp->filename, dir_sp->times) < 0) {
      pr_trace_msg(trace_channel, 2,
        "error setting atime %lu, mtime %lu on '%s': %s",
        (unsigned long) sp->times[0].tv_sec,
        (unsigned long) sp->times[1].tv_sec, dir_sp->filename, strerror(errno));
      write_confirm(p, channel_id, 1,
        pstrcat(p, dir_sp->filename, ": error setting times: ",
        strerror(errno), NULL));
      ok = FALSE;
    }
  }

  if (ok)
    write_confirm(p, channel_id, 0, NULL);

  sp->recv_dir = sp->prev_dir;
  return 1;
}

/* Return 1 when the we should skip to the next path in the list, either
 * because we have received all the data for this path, or because we can
 * never receive it (due to some error).
 */
static int recv_path(pool *p, uint32_t channel_id, struct scp_path *sp,
    char *data, uint32_t datalen) {
  int res;
  cmd_rec *cmd = NULL;

  if (!sp->checked_errors) {
    res = recv_errors(p, channel_id, sp, data, datalen);
    if (res == 1)
      return res;
  }

  /* Check for end-of-directory control messages. */
  res = recv_eod(p, channel_id, sp, data, datalen);
  if (res == 1)
    return res;

  if (!sp->have_mode) {
    struct stat st;

    res = pr_fsio_stat(sp->path, &st);
    if (res == 0) {
      sp->st_mode = st.st_mode;
      sp->have_mode = TRUE;
    }

    if (scp_opts & SFTP_SCP_OPT_DIR) {
      /* If the path should be a directory, stat it and make sure that
       * is the case.  If not, we have a problem.
       */
      if (res == 0) {
        if (!S_ISDIR(st.st_mode)) {
          write_confirm(p, channel_id, 1,
            pstrcat(p, sp->path, ": ", strerror(ENOTDIR), NULL));
          return 1;
        }

      } else {
        write_confirm(p, channel_id, 1,
          pstrcat(p, sp->path, ": ", strerror(errno), NULL));
        return 1;
      }

    } else {
      char *ptr;

      /* If the given path contains a directory component, make sure that the
       * directory exists.
       */
      ptr = strrchr(sp->path, '/');
      if (ptr != NULL) {
        *ptr = '\0';

        res = pr_fsio_stat(sp->path, &st);
        *ptr = '/';

        if (res < 0) {
          write_confirm(p, channel_id, 1,
            pstrcat(p, sp->path, ": ", strerror(errno), NULL));
          return 1;
        }
      }
    }
  }

  if ((scp_opts & SFTP_SCP_OPT_PRESERVE) &&
      !sp->recvd_timeinfo) {
    return recv_timeinfo(p, channel_id, sp, data, datalen);
  }

  if (!sp->recvd_finfo) {
    return recv_finfo(p, channel_id, sp, data, datalen);
  }

  if (!sp->recvd_data &&
      sp->recvlen != sp->filesz) {
    if (cmd == NULL)
      cmd = scp_cmd_alloc(p, C_STOR, sp->best_path);

    pr_throttle_init(cmd);

    /* recv_data() indicates that it has received all of the data, including
     * the end-of-data marker, by returning 1.  If that happens, we need
     * to continue one to the end-of-path processing.
     */
    res = recv_data(p, channel_id, sp, data, datalen);
    if (res != 1)
      return res;
  }

  /* The uploaded file may be smaller than an existing file; call
   * pr_fsio_truncate() to ensure proper file size.
   */
  if (S_ISREG(sp->st_mode)) {
    pr_trace_msg(trace_channel, 9, "truncating file '%s' to %" PR_LU " bytes",
      sp->fh->fh_path, (pr_off_t) sp->filesz);

    if (pr_fsio_ftruncate(sp->fh, sp->filesz) < 0) {
      int xerrno = errno;

      pr_trace_msg(trace_channel, 2, "error truncating '%s' to %" PR_LU
        " bytes: %s", sp->best_path, (pr_off_t) sp->filesz, strerror(xerrno));
      write_confirm(p, channel_id, 1,
        pstrcat(p, sp->filename, ": error truncating file: ", strerror(xerrno),
        NULL));

      sp->wrote_errors = TRUE;
    }
  }

  /* If the SFTPOption for ignoring perms for SCP uploads is set, then
   * skip the chmod on the upload file.
   */
  if (!(sftp_opts & SFTP_OPT_IGNORE_SCP_UPLOAD_PERMS)) { 
    pr_trace_msg(trace_channel, 9, "setting perms %04o on file '%s'",
      (unsigned int) sp->perms, sp->fh->fh_path);

    if (pr_fsio_fchmod(sp->fh, sp->perms) < 0) {
      int xerrno = errno;

      pr_trace_msg(trace_channel, 2, "error setting mode %04o on '%s': %s",
        (unsigned int) sp->perms, sp->best_path, strerror(xerrno));
      write_confirm(p, channel_id, 1,
        pstrcat(p, sp->filename, ": error setting mode: ", strerror(xerrno),
        NULL));

      sp->wrote_errors = TRUE;
    }

  } else {
    pr_trace_msg(trace_channel, 7, "SFTPOption 'IgnoreSCPUploadPerms' "
      "configured, ignoring perms sent by client");
  }

  if (sp->fh) {
    char *curr_path;

    curr_path = pstrdup(scp_pool, sp->fh->fh_path);

    res = pr_fsio_close(sp->fh);
    if (res < 0) {
      int xerrno = errno;

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "scp: error closing '%s': %s", sp->best_path, strerror(xerrno));
      write_confirm(p, channel_id, 1,
        pstrcat(p, sp->filename, ": ", strerror(xerrno), NULL));

      sp->wrote_errors = TRUE;
    }

    sp->fh = NULL;

    if (sp->hiddenstore == TRUE &&
        res == 0) {
      /* This is a HiddenStores file, and needs to be renamed to the real
       * path (i.e. sp->best_path).
       */

      pr_trace_msg(trace_channel, 8, "renaming HiddenStores path '%s' to '%s'",
        curr_path, sp->best_path);

      res = pr_fsio_rename(curr_path, sp->best_path);
      if (res < 0) {
        int xerrno = errno;

        pr_log_pri(PR_LOG_WARNING, "Rename of %s to %s failed: %s",
          curr_path, sp->best_path, strerror(xerrno));

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "renaming of HiddenStore path '%s' to '%s' failed: %s",
          curr_path, sp->best_path, strerror(xerrno));

        pr_fsio_unlink(curr_path);
      }
    }
  }

  /* After receiving all the data and metadata, we need to make sure that
   * the requested times and mode/perms are enforced on the uploaded file.
   */
  if (sp->recvd_timeinfo) {
    pr_trace_msg(trace_channel, 9, "setting times on file '%s'", sp->filename);

    if (pr_fsio_utimes(sp->filename, sp->times) < 0) {
      int xerrno = errno;

      pr_trace_msg(trace_channel, 2,
        "error setting atime %lu, mtime %lu on '%s': %s",
        (unsigned long) sp->times[0].tv_sec,
        (unsigned long) sp->times[1].tv_sec, sp->best_path, strerror(xerrno));
      write_confirm(p, channel_id, 1,
        pstrcat(p, sp->filename, ": error setting times: ", strerror(xerrno),
        NULL));

      sp->wrote_errors = TRUE;
    }
  }

  if (!sp->wrote_errors) {
    /* We only send this if there were no end-of-path handling errors. */
    write_confirm(p, channel_id, 0, NULL);

    if (cmd == NULL)
      cmd = scp_cmd_alloc(p, C_STOR, sp->best_path);

    (void) pr_cmd_dispatch_phase(cmd, POST_CMD, 0);
    (void) pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  } else {
    if (cmd == NULL)
      cmd = scp_cmd_alloc(p, C_STOR, sp->best_path);

    (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);
  }

  return 1;
}

/* Functions for sending files to the client. */

static int send_timeinfo(pool *p, uint32_t channel_id, struct scp_path *sp,
    struct stat *st) {
  int res;
  char ctrl_msg[64];
  size_t ctrl_msglen;

  memset(ctrl_msg, '\0', sizeof(ctrl_msg));

  /* The field of this message are:
   *
   *  T       (time info)
   *  number  (mtime, in secs)
   *  0       (future proof field for sending mtime usecs)
   *  number  (atime, in secs)
   *  0       (future proof field for sending atime usecs)
   */

  snprintf(ctrl_msg, sizeof(ctrl_msg), "T%lu 0 %lu 0",
    (unsigned long) (st->st_mtime > 0 ? st->st_mtime : 0),
    (unsigned long) (st->st_atime > 0 ? st->st_atime : 0));

  pr_trace_msg(trace_channel, 3, "sending '%s' T (timestamps): %s", sp->path,
    ctrl_msg);

  ctrl_msg[strlen(ctrl_msg)] = '\n';
  ctrl_msglen = strlen(ctrl_msg);

  need_confirm = TRUE;

  res = sftp_channel_write_data(p, channel_id, ctrl_msg, ctrl_msglen);
  if (res < 0)
    return -1;

  sp->sent_timeinfo = TRUE;
  return 0;
}

static int send_dirinfo(pool *p, uint32_t channel_id, struct scp_path *sp,
    struct stat *st) {
  int res;
  char ctrl_msg[1536];
  size_t ctrl_msglen;
  char *tmp;

  /* We need to find the last path component, if any; no path separators
   * in the control messages.
   */
  tmp = strrchr(sp->path, '/');
  if (tmp == NULL) {
    tmp = sp->path;

  } else {
    tmp++;
  }

  memset(ctrl_msg, '\0', sizeof(ctrl_msg));
  snprintf(ctrl_msg, sizeof(ctrl_msg), "D%04o 0 %.1024s",
    (unsigned int) (st->st_mode & SFTP_SCP_ST_MODE_MASK), tmp);

  pr_trace_msg(trace_channel, 3, "sending '%s' D (directory): %s", sp->path,
    ctrl_msg);

  ctrl_msg[strlen(ctrl_msg)] = '\n';
  ctrl_msglen = strlen(ctrl_msg);

  need_confirm = TRUE;

  res = sftp_channel_write_data(p, channel_id, ctrl_msg, ctrl_msglen);
  if (res < 0) 
    return -1;

  sp->sent_dirinfo = TRUE;
  return 0;
}

static int send_finfo(pool *p, uint32_t channel_id, struct scp_path *sp,
    struct stat *st) {
  int res;
  char ctrl_msg[1536];
  size_t ctrl_msglen;
  char *tmp;

  /* We need to find the last path component, if any; no path separators
   * in the control messages.
   */
  tmp = strrchr(sp->path, '/');
  if (tmp == NULL) {
    tmp = sp->path;

  } else {
    tmp++;
  }

  memset(ctrl_msg, '\0', sizeof(ctrl_msg));
  snprintf(ctrl_msg, sizeof(ctrl_msg), "C%04o %" PR_LU " %.1024s",
    (unsigned int) (st->st_mode & SFTP_SCP_ST_MODE_MASK),
    (pr_off_t) st->st_size, tmp);

  pr_trace_msg(trace_channel, 3, "sending '%s' C (info): %s", sp->path,
    ctrl_msg);

  ctrl_msg[strlen(ctrl_msg)] = '\n';
  ctrl_msglen = strlen(ctrl_msg);

  need_confirm = TRUE;

  res = sftp_channel_write_data(p, channel_id, ctrl_msg, ctrl_msglen);
  if (res < 0)
    return -1;

  sp->sent_finfo = TRUE;
  return 0;
}

static int send_data(pool *p, uint32_t channel_id, struct scp_path *sp,
    struct stat *st) {
  int res;
  char *chunk;
  size_t chunksz;
  long chunklen;

  /* Include space for one more character, i.e. for the terminating NUL
   * character that indicates the last chunk of the file.
   */
  chunksz = (main_server->tcp_rcvbuf_len > 0 ? main_server->tcp_rcvbuf_len :
    pr_config_get_xfer_bufsz()) + 1;
  chunk = palloc(p, chunksz);

  /* Keep sending chunks until we have sent the entire file, or until the
   * channel window closes.
   */
  while (1) {
    pr_signals_handle();

    if (S_ISREG(st->st_mode)) {
      /* Seek to where we last left off with this file. */
      if (pr_fsio_lseek(sp->fh, sp->sentlen, SEEK_SET) < 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error seeking to offset %" PR_LU " in '%s': %s",
          (pr_off_t) sp->sentlen, sp->path, strerror(errno));
        return 1;
      }

      pr_trace_msg(trace_channel, 15, "at %.2f%% (%" PR_LU " of %" PR_LU
        " bytes) of '%s'",
        (float) (((float) sp->sentlen / (float) st->st_size) * 100),
        (pr_off_t) sp->sentlen, (pr_off_t) st->st_size, sp->path);
    }

    chunklen = pr_fsio_read(sp->fh, chunk, chunksz - 1);
    if (chunklen < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading from '%s': %s", sp->path, strerror(errno));
      return 1;
    }

    session.xfer.total_bytes += chunklen;
    session.total_bytes += chunklen;

    /* If this was the last chunk of the file, write one more space
     * character.
     */
    if (sp->sentlen + chunklen == st->st_size) {
      chunk[chunklen++] = '\0';
      need_confirm = TRUE;

      pr_throttle_pause(sp->sentlen, TRUE);

    } else {
      pr_throttle_pause(sp->sentlen, FALSE);
    }

    pr_trace_msg(trace_channel, 3, "sending '%s' data (%lu bytes)", sp->path,
      (unsigned long) chunklen);

    res = sftp_channel_write_data(p, channel_id, chunk, chunklen);
    if (res < 0) {
      return 1;
    }

    /* If our channel window has closed, try handling some packets; hopefully
     * some of them are WINDOW_ADJUST messages.
     *
     * XXX I wonder if this can be more efficient by waiting until we
     * have a certain amount of data buffered up (N * transfer data size?)
     * AND the window is closed before handling incoming packets?  That way
     * we can handle more WINDOW_ADJUSTS at a whack, at the cost of buffering
     * more data in memory.  Hmm.
     *
     * We also need to watch for when rekeying is occurring; handle packets
     * until that state clears.
     */
    while ((sftp_sess_state & SFTP_SESS_STATE_REKEYING) ||
           sftp_channel_get_windowsz(channel_id) == 0) {
      pr_signals_handle();

      if (sftp_ssh2_packet_handle() < 0) {
        return 1;
      }
    }

    sp->sentlen += chunklen;
    if (sp->sentlen >= st->st_size) {
      sp->sent_data = TRUE;
      break;
    }
  }

  return 0;
}

static int send_dir(pool *p, uint32_t channel_id, struct scp_path *sp,
    struct stat *st) {
  struct dirent *dent;
  int res = 0;

  if (sp->dirh == NULL) {
    sp->dirh = pr_fsio_opendir(sp->path);
    if (sp->dirh == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading directory '%s': %s", sp->path, strerror(errno));
      return -1;
    }

    /* If we're a directory, send a D control message. */
    if (!sp->sent_dirinfo) {
      return send_dirinfo(p, channel_id, sp, st);
    }
  }

  /* If we were already in the middle of sending a path from this
   * directory, continue with it.  Otherwise, read the next dent from the
   * directory handle.
   */

  if (sp->dir_spi) { 
    if (session.xfer.p == NULL) {
      session.xfer.p = pr_pool_create_sz(scp_pool, 64);
      session.xfer.path = pstrdup(session.xfer.p, sp->dir_spi->path);
      memset(&session.xfer.start_time, 0, sizeof(session.xfer.start_time));
      gettimeofday(&session.xfer.start_time, NULL);
      session.xfer.direction = PR_NETIO_IO_WR;
    }

    res = send_path(p, channel_id, sp->dir_spi);
    if (res <= 0)
      return res;

    /* Clear out any transfer-specific data. */
    if (session.xfer.p) {
      destroy_pool(session.xfer.p);
    }
    memset(&session.xfer, 0, sizeof(session.xfer));

    sp->dir_spi = NULL;
  }

  while ((dent = pr_fsio_readdir(sp->dirh)) != NULL) {
    struct scp_path *spi;
    size_t pathlen;

    pr_signals_handle();

    /* Skip "." and "..". */
    if (strcmp(dent->d_name, ".") == 0 ||
        strcmp(dent->d_name, "..") == 0) {
      continue;
    }

    /* Add these to the list of paths that need to be sent. */
    spi = pcalloc(scp_pool, sizeof(struct scp_path));
    spi->path = pdircat(p, sp->path, dent->d_name, NULL);
    pathlen = strlen(spi->path);

    /* Trim any trailing path separators.  It's important. */
    while (pathlen > 1 &&
           spi->path[pathlen-1] == '/') {
      pr_signals_handle();
      spi->path[--pathlen] = '\0';
    }

    if (pathlen > 0) {
      sp->dir_spi = spi;

      if (session.xfer.p == NULL) {
        session.xfer.p = pr_pool_create_sz(scp_pool, 64);
        session.xfer.path = pstrdup(session.xfer.p, sp->dir_spi->path);
        memset(&session.xfer.start_time, 0, sizeof(session.xfer.start_time));
        gettimeofday(&session.xfer.start_time, NULL);
        session.xfer.direction = PR_NETIO_IO_WR;
      }

      res = send_path(p, channel_id, sp->dir_spi);
      if (res == 1) {
        /* Clear out any transfer-specific data. */
        if (session.xfer.p) {
          destroy_pool(session.xfer.p);
        }
        memset(&session.xfer, 0, sizeof(session.xfer));
      }

      return res;
    }
  }

  if (sp->dirh) {
    pr_fsio_closedir(sp->dirh);
    sp->dirh = NULL;

    /* Send end-of-directory control message */

    need_confirm = TRUE;
    res = sftp_channel_write_data(p, channel_id, "E\n", 2);
    if (res < 0)
      return res;
  }

  return 1;
}

/* Return 1 when the we should skip to the next path in the list, either
 * because we have sent all the data for this path, or because we can
 * never send it (due to some error).
 */
static int send_path(pool *p, uint32_t channel_id, struct scp_path *sp) {
  int res;
  struct stat st;
  cmd_rec *cmd = NULL;

  if (sp->sent_data) {
    /* Already sent everything for this path. */
    return 1;
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "scp download", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", sp->path, NULL, NULL);

  cmd = scp_cmd_alloc(p, C_RETR, sp->path);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "scp download of '%s' blocked by '%s' handler", sp->path,
      cmd->argv[0]);

    (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    return 1;
  }

  sp->path = cmd->arg;

  if (pr_fsio_stat(sp->path, &st) < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error stat'ing '%s': %s", sp->path, strerror(errno));

    if (sp->fh) {
      pr_fsio_close(sp->fh);
      sp->fh = NULL;

      (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
      (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);
    }

    return 1;
  }

  if (!S_ISREG(st.st_mode)
#ifdef S_ISFIFO
      && !S_ISFIFO(st.st_mode)
#endif
     ) {
    if (S_ISDIR(st.st_mode)) {
      if (scp_opts & SFTP_SCP_OPT_RECURSE) {
        res = send_dir(p, channel_id, sp, &st);
        return res;

      } else {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "cannot send directory '%s' (no -r option)", sp->path);

        (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
        (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

        return 1;
      }

    } else {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "cannot send '%s': Not a regular file", sp->path);

      (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
      (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      return 1;
    }
  }

  if (!sp->fh) {
    if (!dir_check(p, cmd, G_READ, sp->best_path, NULL)) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "scp download of '%s' blocked by <Limit> configuration", sp->best_path);

      (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
      (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      return 1;
    }

    sp->fh = pr_fsio_open(sp->path, O_RDONLY|O_NONBLOCK);
    if (sp->fh == NULL) {
      int xerrno = errno;

      (void) pr_trace_msg("fileperms", 1, "%s, user '%s' (UID %lu, GID %lu): "
        "error opening '%s': %s", "scp download", session.user,
        (unsigned long) session.uid, (unsigned long) session.gid,
        sp->path, strerror(xerrno));

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading '%s': %s", sp->path, strerror(xerrno));

      (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
      (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      errno = xerrno;
      return 1;
    }
  }

  /* If the PRESERVE flag is set, then we need to send a T control message
   * that includes the file timestamps.
   */
  if ((scp_opts & SFTP_SCP_OPT_PRESERVE) &&
      !sp->sent_timeinfo) {
    res = send_timeinfo(p, channel_id, sp, &st);
    if (res == 1) {
      (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
      (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);
    }

    return res;
  }

  if (!sp->sent_finfo) {
    res = send_finfo(p, channel_id, sp, &st);
    if (res == 1) {
      (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
      (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);
    }

    return res;
  }

  if (!sp->sent_data) {
    pr_throttle_init(cmd);

    res = send_data(p, channel_id, sp, &st);
    if (res == 1) {
      (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
      (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);
      return res;
    }
  }

  pr_fsio_close(sp->fh);
  sp->fh = NULL;

  (void) pr_cmd_dispatch_phase(cmd, POST_CMD, 0);
  (void) pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  return 1;
}

/* Main entry point */
int sftp_scp_handle_packet(pool *p, void *ssh2, uint32_t channel_id,
    char *data, uint32_t datalen) {
  int res = -1;
  struct ssh2_packet *pkt;

  scp_session = scp_get_session(channel_id);
  if (scp_session == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no existing SCP session for channel ID %lu, rejecting request",
      (unsigned long) channel_id);
    return -1;
  }

  pkt = ssh2;

  /* This is a bit of a hack, for playing along better with mod_vroot,
   * which pays attention to the session.curr_phase value.
   *
   * I'm not sure which is better here, PRE_CMD vs CMD.  Let's go with
   * PRE_CMD for now.
   */
  session.curr_phase = PRE_CMD;

  if (need_confirm) {
    /* Handle the confirmation/response from the client. */
    if (read_confirm(pkt, &data, &datalen) < 0) {
      return 1;
    }
  }

  if (scp_opts & SFTP_SCP_OPT_ISSRC) {
    while (scp_session->path_idx < scp_session->paths->nelts) {
      struct scp_path **paths;

      pr_signals_handle();

      paths = scp_session->paths->elts;

      if (session.xfer.p == NULL) {
        session.xfer.p = pr_pool_create_sz(scp_pool, 64);
        session.xfer.path = pstrdup(session.xfer.p,
          paths[scp_session->path_idx]->path);
        memset(&session.xfer.start_time, 0, sizeof(session.xfer.start_time));
        gettimeofday(&session.xfer.start_time, NULL);
        session.xfer.direction = PR_NETIO_IO_WR;
      }

      res = send_path(pkt->pool, channel_id, paths[scp_session->path_idx]);
      if (res == 1) {
        /* If send_path() returns 1, it means we've finished that path,
         * and are ready for another.
         */
        scp_session->path_idx++;

        /* Clear out any transfer-specific data. */
        if (session.xfer.p) {
          destroy_pool(session.xfer.p);
        }
        memset(&session.xfer, 0, sizeof(session.xfer));

        continue;
      }

      break;
    }

    if (res < 0) {
      if (scp_session->path_idx == scp_session->paths->nelts) {
        /* If we've sent all the paths, and we're here, assume that everything
         * is OK.  We may just have received the final "OK" ACK byte from the
         * scp client, and have nothing more to do.
         */
        return 1;
      }

      return -1;
    }

    if (scp_session->path_idx != scp_session->paths->nelts)
      return 0;

    return 1;

  } else if (scp_opts & SFTP_SCP_OPT_ISDST) {
    struct scp_path **paths;

    paths = scp_session->paths->elts;

    if (session.xfer.p == NULL) {
      session.xfer.p = pr_pool_create_sz(scp_pool, 64);
      session.xfer.path = pstrdup(session.xfer.p,
        paths[scp_session->path_idx]->path);
      memset(&session.xfer.start_time, 0, sizeof(session.xfer.start_time));
      gettimeofday(&session.xfer.start_time, NULL);
      session.xfer.direction = PR_NETIO_IO_RD;
    }

    res = recv_path(pkt->pool, channel_id, paths[scp_session->path_idx], data,
      datalen);
    if (res == 1) {
      /* If recv_path() returns 1, it means we've finished that path,
       * and are ready for another.
       */
      reset_path(paths[scp_session->path_idx]);

      /* Clear out any transfer-specific data. */
      if (session.xfer.p) {
        destroy_pool(session.xfer.p);
      }
      memset(&session.xfer, 0, sizeof(session.xfer));

      return 0;
    }

    if (res < 0)
      return -1;
  }

  return 0;
}

int sftp_scp_set_params(pool *p, uint32_t channel_id, array_header *req) {
  register unsigned int i;
  int optc, use_glob = TRUE;
  char **reqargv;
  const char *opts = "dfprtv";
  config_rec *c;
  struct scp_paths *paths;

  /* Possible options are:
   *
   *  -d (target should be a directory)
   *  -f (copying data from the server)
   *  -p (preserve times, mode using ctrl messages)
   *  -r (recursive)
   *  -t (copying data to the server)
   *  -v (verbose)
   */

#if defined(FREEBSD4) || defined(FREEBSD5) || defined(FREEBSD6) || \
    defined(FREEBSD7) || \
    defined(DARWIN7) || defined(DARWIN8) || defined(DARWIN9)
  optreset = 1;
  opterr = 1;
  optind = 1;

#elif defined(SOLARIS2)
  opterr = 0;
  optind = 1;

#else
  opterr = 0;
  optind = 0;
#endif /* !FreeBSD and !Solaris2 */

  reqargv = (char **) req->elts;

  for (i = 0; i < req->nelts; i++) {
    if (reqargv[i]) {
      pr_trace_msg(trace_channel, 5, "reqargv[%u] = '%s'", i, reqargv[i]);
    }
  }

  c = find_config(main_server->conf, CONF_PARAM, "UseGlobbing", FALSE);
  if (c) {
    use_glob = *((unsigned char *) c->argv[0]);
  }

  need_confirm = FALSE;
  scp_pool = make_sub_pool(sftp_pool);
  pr_pool_tag(scp_pool, "SSH2 SCP Pool");

  if (pr_env_get(permanent_pool, "POSIXLY_CORRECT") == NULL) {
    pr_env_set(permanent_pool, "POSIXLY_CORRECT", "1");
  }

  while ((optc = getopt(req->nelts, reqargv, opts)) != -1) {
    switch (optc) {
      case 'd':
        scp_opts |= SFTP_SCP_OPT_DIR;
        break;

      case 'f':
        scp_opts |= SFTP_SCP_OPT_ISSRC;
        need_confirm = TRUE;
        break;

      case 'p':
        scp_opts |= SFTP_SCP_OPT_PRESERVE;
        break;

      case 'r':
        scp_opts |= SFTP_SCP_OPT_RECURSE;
        break;

      case 't':
        scp_opts |= SFTP_SCP_OPT_ISDST;
        write_confirm(p, channel_id, 0, NULL);
        break;

      case 'v':
        scp_opts |= SFTP_SCP_OPT_VERBOSE;
        break;
    }
  }

  /* If we don't have paths, then it's an error. */
  if (reqargv[optind] == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "'scp' request provided no paths, ignoring");
    return -1;
  }

  paths = scp_new_paths(channel_id);
  if (paths == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to handle paths for 'scp' request: %s", strerror(errno));
    return -1;
  }

  /* Make a copy of the remaining paths, for later handling. */
  paths->paths = make_array(paths->pool, 1, sizeof(struct scp_path *));
  paths->path_idx = 0;

  for (i = optind; i < req->nelts; i++) {
    pr_signals_handle();

    if (reqargv[i]) {
      struct scp_path *sp;
      size_t pathlen;
      char *glob_path;

      if (use_glob &&
          (scp_opts & SFTP_SCP_OPT_ISSRC) &&
          strpbrk(reqargv[i], "{[*?") != NULL) {
        int res, xerrno;
        glob_t gl;

        /* Whee, glob characters.  Need to expand the pattern to the
         * list of matching files, just as the shell would do.
         */

        memset(&gl, 0, sizeof(gl));

        glob_path = pstrdup(paths->pool, reqargv[i]);
        pathlen = strlen(glob_path);

        /* Remove any enclosing shell quotations, e.g. single and double
         * quotation marks.  Some SCP clients (i.e. newer libssh2) will
         * quote the paths, assuming that the handling server (us) uses
         * a shell to handle the command.  Sigh.
         */
        if ((glob_path[0] == '\'' &&
             glob_path[pathlen-1] == '\'') ||
            (glob_path[0] == '"' &&
             glob_path[pathlen-1] == '"')) {
          glob_path[pathlen-1] = '\0';
          glob_path = (glob_path + 1);
          pathlen -= 2;
        }

        res = pr_fs_glob(glob_path, GLOB_NOSORT|GLOB_BRACE, NULL, &gl);
        switch (res) {
          case 0: {
            register unsigned int j;

            for (j = 0; j < gl.gl_pathc; j++) {
              pr_signals_handle();

              sp = pcalloc(paths->pool, sizeof(struct scp_path));
              sp->path = pstrdup(paths->pool, gl.gl_pathv[j]);
              pathlen = strlen(sp->path);

              /* Trim any trailing path separators.  It's important. */
              while (pathlen > 1 &&
                     sp->path[pathlen-1] == '/') {
                pr_signals_handle();
                sp->path[--pathlen] = '\0';
              }

              if (pathlen > 0) {
                *((struct scp_path **) push_array(paths->paths)) = sp;
              }
            }

            break;
          }

          case GLOB_NOSPACE:
            xerrno = errno;
            pr_trace_msg(trace_channel, 1, "error globbing '%s': Not "
              "enough memory (%s)", reqargv[i], strerror(xerrno));
            errno = xerrno;
            break;

          case GLOB_NOMATCH:
            xerrno = errno;
            pr_trace_msg(trace_channel, 1, "error globbing '%s': No "
              "matches found (%s)", reqargv[i], strerror(xerrno));
            errno = xerrno;
            break;
        }

        pr_fs_globfree(&gl);

      } else {
        sp = pcalloc(paths->pool, sizeof(struct scp_path));
        sp->path = pstrdup(paths->pool, reqargv[i]);
        pathlen = strlen(sp->path);

        /* Remove any enclosing shell quotations, e.g. single and double
         * quotation marks.  Some SCP clients (i.e. newer libssh2) will
         * quote the paths, assuming that the handling server (us) uses
         * a shell to handle the command.  Sigh.
         */
        if ((sp->path[0] == '\'' &&
             sp->path[pathlen-1] == '\'') ||
            (sp->path[0] == '"' &&
             sp->path[pathlen-1] == '"')) {
          sp->path[pathlen-1] = '\0';
          sp->path = (sp->path + 1);
          pathlen -= 2;
        } 

        /* Trim any trailing path separators.  It's important. */
        while (pathlen > 1 &&
               sp->path[pathlen-1] == '/') {
          pr_signals_handle();
          sp->path[--pathlen] = '\0';
        }

        if (pathlen > 0) {
          *((struct scp_path **) push_array(paths->paths)) = sp;
        }
      }
    }
  }

  /* If we're receiving files, and the client provided more than one
   * path, then it's ambiguous -- we don't know which of the files
   * the client will be sending should be written to which path.
   */
  if ((scp_opts & SFTP_SCP_OPT_ISDST) &&
      paths->paths->nelts != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "'scp' request provided more than one destination path, ignoring");
    return -1;
  }

  for (i = 0; i < paths->paths->nelts; i++) {
    struct scp_path *sp;

    sp = ((struct scp_path **) paths->paths->elts)[i];
    if (sp) {
      pr_trace_msg(trace_channel, 5, "scp_path[%u] = '%s'", i, sp->path);
    }
  }

  return 0;
}

int sftp_scp_open_session(uint32_t channel_id) {
  register unsigned int i;
  pool *sub_pool;
  struct scp_paths *paths;
  struct scp_session *sess, *last;

  /* Check to see if we already have an SCP session opened for the given
   * channel ID.
   */
  sess = last = scp_sessions;
  while (sess) {
    pr_signals_handle();

    if (sess->channel_id == channel_id) {
      errno = EEXIST;
      return -1;
    }

    if (sess->next == NULL) {
      /* This is the last item in the list. */
      last = sess;
    }

    sess = sess->next;
  }

  paths = scp_get_paths(channel_id);
  if (paths == NULL) {
    pr_trace_msg(trace_channel, 1, "missing paths for SCP channel ID %lu",
      (unsigned long) channel_id);
    errno = EACCES;
    return -1;
  }

  /* Looks like we get to allocate a new one. */
  sub_pool = make_sub_pool(scp_pool);
  pr_pool_tag(sub_pool, "SCP session pool");

  sess = pcalloc(sub_pool, sizeof(struct scp_session));
  sess->pool = sub_pool;
  sess->channel_id = channel_id;

  /* Now copy all of the struct scp_path elements from the paths list into
   * the session object.
   */

  sess->paths = make_array(sess->pool, paths->paths->nelts,
    sizeof(struct scp_path *));

  for (i = 0; i < paths->paths->nelts; i++) {
    struct scp_path *src_sp, *dst_sp;

    src_sp = ((struct scp_path **) paths->paths->elts)[i];

    dst_sp = pcalloc(sess->pool, sizeof(struct scp_path));
    dst_sp->path = pstrdup(sess->pool, src_sp->path);

    *((struct scp_path **) push_array(sess->paths)) = dst_sp;
  }

  sess->path_idx = paths->path_idx;

  scp_destroy_paths(paths);

  if (last) {
    last->next = sess;
    sess->prev = last;

  } else {
    scp_sessions = sess;
  }

  pr_session_set_protocol("scp");
  return 0;
}

int sftp_scp_close_session(uint32_t channel_id) {
  struct scp_session *sess;

  /* Check to see if we have an SCP session opened for this channel ID. */
  sess = scp_sessions;
  while (sess) {
    pr_signals_handle();

    if (sess->channel_id == channel_id) {

      if (sess->next)
        sess->next->prev = sess->prev;

      if (sess->prev) {
        sess->prev->next = sess->next;

      } else {
        /* This is the start of the session list. */
        scp_sessions = sess->next;
      }

      if (sess->dirs != NULL) {
        /* XXX How to handle dangling directory lists?? */
      }

      if (sess->paths != NULL) {
        if (sess->paths != NULL &&
            sess->paths->nelts > 0) {
          register unsigned int i;
          int count = 0;
          struct scp_path **elts;

          elts = sess->paths->elts;
          for (i = 0; i < sess->paths->nelts; i++) {
            struct scp_path *elt = elts[i];

            if (elt->fh != NULL) {
              count++;
            }
          }

          if (count > 0) {
            config_rec *c;
            unsigned char delete_aborted_stores = FALSE;

            c = find_config(main_server->conf, CONF_PARAM,
              "DeleteAbortedStores", FALSE);
            if (c) {
              delete_aborted_stores = *((unsigned char *) c->argv[0]);
            }

            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "aborting %d unclosed file %s", count,
              count != 1 ? "handles" : "handle");

            for (i = 0; i < sess->paths->nelts; i++) {
              struct scp_path *elt = elts[i];

              if (elt->fh != NULL) {
                char *abs_path, *curr_path;

                curr_path = pstrdup(scp_pool, elt->fh->fh_path);

                /* Write out an 'incomplete' TransferLog entry for this. */
                abs_path = dir_abs_path(scp_pool, elt->best_path, TRUE);
            
                if (elt->recvlen > 0) {
                  xferlog_write(0, pr_netaddr_get_sess_remote_name(),
                    elt->recvlen, abs_path, 'b', 'i', 'r', session.user, 'i');
            
                } else {
                  xferlog_write(0, pr_netaddr_get_sess_remote_name(),
                    elt->sentlen, abs_path, 'b', 'o', 'r', session.user, 'i');
                }

                if (pr_fsio_close(elt->fh) < 0) {
                  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
                    "error writing aborted file '%s': %s", elt->best_path,
                    strerror(errno));
                }

                elt->fh = NULL;

                if (delete_aborted_stores == TRUE &&
                    elt->recvlen > 0) {
                  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
                    "removing aborted uploaded file '%s'", curr_path);

                  if (pr_fsio_unlink(curr_path) < 0) {
                    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
                      "error unlinking file '%s': %s", curr_path,
                      strerror(errno));
                  }
                }
              }
            }
          }
        }
      }

      sess->dirs = NULL;
      sess->paths = NULL;
      destroy_pool(sess->pool);

      pr_session_set_protocol("ssh2");
      return 0;
    }

    sess = sess->next;
  }

  errno = ENOENT;
  return -1;
}
