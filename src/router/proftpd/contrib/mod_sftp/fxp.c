/*
 * ProFTPD - mod_sftp sftp
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
 * $Id: fxp.c,v 1.88 2010/02/19 17:47:20 castaglia Exp $
 */

#include "mod_sftp.h"
#include "ssh2.h"
#include "msg.h"
#include "crypto.h"
#include "packet.h"
#include "disconnect.h"
#include "channel.h"
#include "auth.h"
#include "fxp.h"
#include "utf8.h"

/* FXP_NAME file attribute flags */
#define SSH2_FX_ATTR_SIZE		0x00000001
#define SSH2_FX_ATTR_UIDGID		0x00000002
#define SSH2_FX_ATTR_PERMISSIONS	0x00000004 
#define SSH2_FX_ATTR_ACMODTIME		0x00000008
#define SSH2_FX_ATTR_ACCESSTIME         SSH2_FX_ATTR_ACMODTIME
#define SSH2_FX_ATTR_CREATETIME		0x00000010
#define SSH2_FX_ATTR_MODIFYTIME		0x00000020
#define SSH2_FX_ATTR_ACL		0x00000040
#define SSH2_FX_ATTR_OWNERGROUP		0x00000080
#define SSH2_FX_ATTR_SUBSECOND_TIMES	0x00000100
#define SSH2_FX_ATTR_BITS		0x00000200
#define SSH2_FX_ATTR_ALLOCATION_SIZE	0x00000400
#define SSH2_FX_ATTR_TEXT_HINT		0x00000800
#define SSH2_FX_ATTR_MIME_TYPE		0x00001000
#define SSH2_FX_ATTR_LINK_COUNT		0x00002000
#define SSH2_FX_ATTR_UNTRANSLATED_NAME	0x00004000
#define SSH2_FX_ATTR_CTIME		0x00008000
#define SSH2_FX_ATTR_EXTENDED		0x80000000

/* FXP_ATTRS file types */
#define SSH2_FX_ATTR_FTYPE_REGULAR		1
#define SSH2_FX_ATTR_FTYPE_DIRECTORY		2
#define SSH2_FX_ATTR_FTYPE_SYMLINK		3
#define SSH2_FX_ATTR_FTYPE_SPECIAL		4
#define SSH2_FX_ATTR_FTYPE_UNKNOWN		5
#define SSH2_FX_ATTR_FTYPE_SOCKET		6
#define SSH2_FX_ATTR_FTYPE_CHAR_DEVICE		7
#define SSH2_FX_ATTR_FTYPE_BLOCK_DEVICE		8
#define SSH2_FX_ATTR_FTYPE_FIFO			9

/* FXP_LOCK/FXP_UNLOCK flags */
#define SSH2_FXL_READ			0x00000040
#define SSH2_FXL_WRITE			0x00000080
#define SSH2_FXL_DELETE			0x00000100

/* FXP_OPEN flags (prior to version 5) */
#define SSH2_FXF_READ			0x00000001
#define SSH2_FXF_WRITE			0x00000002
#define SSH2_FXF_APPEND			0x00000004
#define SSH2_FXF_CREAT			0x00000008
#define SSH2_FXF_TRUNC			0x00000010
#define SSH2_FXF_EXCL			0x00000020
#define SSH2_FXF_TEXT			0x00000040

/* FXP_OPEN flags (version 5 and higher) */
#define SSH2_FXF_WANT_READ_DATA		0x00000001
#define SSH2_FXF_WANT_WRITE_DATA	0x00000002
#define SSH2_FXF_WANT_APPEND_DATA	0x00000004
#define SSH2_FXF_WANT_READ_NAMED_ATTRS	0x00000008
#define SSH2_FXF_WANT_WRITE_NAMED_ATTRS	0x00000010
#define SSH2_FXF_WANT_READ_ATTRIBUTES	0x00000080
#define SSH2_FXF_WANT_WRITE_ATTRIBUTES	0x00000100
#define SSH2_FXF_WANT_READ_ACL		0x00020000
#define SSH2_FXF_WANT_WRITE_ACL		0x00040000
#define SSH2_FXF_WANT_WRITE_OWNER	0x00080000

#define SSH2_FXF_CREATE_NEW			0x00000000
#define SSH2_FXF_CREATE_TRUNCATE		0x00000001
#define SSH2_FXF_OPEN_EXISTING			0x00000002
#define SSH2_FXF_OPEN_OR_CREATE			0x00000003
#define SSH2_FXF_TRUNCATE_EXISTING		0x00000004
#define SSH2_FXF_ACCESS_APPEND_DATA		0x00000008
#define SSH2_FXF_ACCESS_APPEND_DATA_ATOMIC	0x00000010
#define SSH2_FXF_ACCESS_TEXT_MODE		0x00000020

/* These are the BLOCK_{READ,WRITE,DELETE} values from Section 8.1.1.3 of
 * the SFTP Draft.
 */
#define SSH2_FXF_ACCESS_READ_LOCK		0x00000040
#define SSH2_FXF_ACCESS_WRITE_LOCK		0x00000080
#define SSH2_FXF_ACCESS_DELETE_LOCK		0x00000100

/* FXP_REALPATH control flags */
#define SSH2_FXRP_NO_CHECK		0x00000001
#define SSH2_FXRP_STAT_IF		0x00000002
#define SSH2_FXRP_STAT_ALWAYS		0x00000003

/* FXP_RENAME flags */
#define SSH2_FXR_OVERWRITE		0x00000001
#define SSH2_FXR_ATOMIC			0x00000002
#define SSH2_FXR_NATIVE			0x00000004

/* FXP_STATUS codes */
#define SSH2_FX_OK				0
#define SSH2_FX_EOF				1
#define SSH2_FX_NO_SUCH_FILE			2
#define SSH2_FX_PERMISSION_DENIED		3
#define SSH2_FX_FAILURE				4
#define SSH2_FX_BAD_MESSAGE			5
#define SSH2_FX_NO_CONNECTION			6
#define SSH2_FX_CONNECTION_LOST			7
#define SSH2_FX_OP_UNSUPPORTED			8
#define SSH2_FX_INVALID_HANDLE			9
#define SSH2_FX_NO_SUCH_PATH			10
#define SSH2_FX_FILE_ALREADY_EXISTS		11
#define SSH2_FX_WRITE_PROTECT			12
#define SSH2_FX_NO_MEDIA			13
#define SSH2_FX_NO_SPACE_ON_FILESYSTEM		14
#define SSH2_FX_QUOTA_EXCEEDED			15
#define SSH2_FX_UNKNOWN_PRINCIPAL		16
#define SSH2_FX_LOCK_CONFLICT			17
#define SSH2_FX_DIR_NOT_EMPTY			18
#define SSH2_FX_NOT_A_DIRECTORY			19
#define SSH2_FX_INVALID_FILENAME		20
#define SSH2_FX_LINK_LOOP			21
#define SSH2_FX_CANNOT_DELETE			22
#define SSH2_FX_INVALID_PARAMETER		23
#define SSH2_FX_FILE_IS_A_DIRECTORY		24
#define SSH2_FX_BYTE_RANGE_LOCK_CONFLICT	25
#define SSH2_FX_BYTE_RANGE_LOCK_REFUSED		26
#define SSH2_FX_DELETE_PENDING			27
#define SSH2_FX_FILE_CORRUPT			28
#define SSH2_FX_OWNER_INVALID			29
#define SSH2_FX_GROUP_INVALID			30
#define SSH2_FX_NO_MATCHING_BYTE_RANGE_LOCK	31

/* statvfs@openssh.com extension flags */
#define SSH2_FXE_STATVFS_ST_RDONLY		0x1
#define SSH2_FXE_STATVFS_ST_NOSUID		0x2

extern pr_response_t *resp_list, *resp_err_list;

struct fxp_dirent {
  const char *client_path;
  const char *real_path;
  struct stat *st;
};

struct fxp_handle {
  pool *pool;
  const char *name;

  pr_fh_t *fh;
  int fh_flags;

  /* For supporting the HiddenStores directive */
  char *fh_real_path;

  /* For tracking the number of bytes transferred for this file; for
   * better TransferLog tracking.
   */
  size_t fh_bytes_xferred;

  void *dirh;
  const char *dir;
};

struct fxp_packet {
  pool *pool;
  uint32_t channel_id;
  uint32_t packet_len;
  unsigned char request_type;
  uint32_t request_id;
  uint32_t payload_sz;
  char *payload;
  uint32_t payload_len;

  unsigned int state;
};

#define	FXP_PACKET_HAVE_PACKET_LEN	0x0001
#define	FXP_PACKET_HAVE_REQUEST_TYPE	0x0002
#define	FXP_PACKET_HAVE_REQUEST_ID	0x0004
#define	FXP_PACKET_HAVE_PAYLOAD_SIZE	0x0008
#define	FXP_PACKET_HAVE_PAYLOAD		0x0010

/* After 32K of allocation from the scratch SFTP payload pool, destroy the
 * pool and create a new one.  This will prevent unbounded allocation
 * from the pool.
 */
#define FXP_PACKET_DATA_ALLOC_MAX_SZ		(1024 * 32)
static size_t fxp_packet_data_allocsz = 0;

#define FXP_PACKET_DATA_DEFAULT_SZ		(1024 * 4)
#define FXP_RESPONSE_DATA_DEFAULT_SZ		512

struct fxp_extpair {
  char *ext_name;
  uint32_t ext_datalen;
  char *ext_data;
};

static pool *fxp_pool = NULL;
static int fxp_use_gmt = TRUE;

static unsigned int fxp_min_client_version = 1;
static unsigned int fxp_max_client_version = 6;
static unsigned int fxp_utf8_protocol_version = 4;
static unsigned long fxp_ext_flags = SFTP_FXP_EXT_DEFAULT;

/* For handling "version-select" requests properly (or rejecting them as
 * necessary.
 */
static int allow_version_select = FALSE;

/* Use a struct to maintain the per-channel FXP-specific values. */
struct fxp_session {
  struct fxp_session *next, *prev;

  pool *pool;
  uint32_t channel_id;
  uint32_t client_version;
  pr_table_t *handle_tab;
};

static struct fxp_session *fxp_session = NULL, *fxp_sessions = NULL;

static const char *trace_channel = "sftp";

/* Necessary prototypes */
static struct fxp_handle *fxp_handle_get(const char *);
static struct fxp_packet *fxp_packet_create(pool *, uint32_t);
static int fxp_packet_write(struct fxp_packet *);

/* XXX These two namelist-related functions are the same as the ones
 * in kex.c; the code should be refactored out into a misc.c or utils.c
 * or somesuch.
 */

static array_header *fxp_parse_namelist(pool *p, const char *names) {
  char *ptr;
  array_header *list;

  list = make_array(p, 0, sizeof(const char *));

  ptr = strchr(names, ',');
  while (ptr) {
    char *elt;

    pr_signals_handle();

    elt = pcalloc(p, (ptr - names) + 1);
    memcpy(elt, names, (ptr - names));

    *((const char **) push_array(list)) = elt;
    names = ++ptr;

    ptr = strchr(names, ',');
  }
  *((const char **) push_array(list)) = pstrdup(p, names);

  return list;
}

static const char *fxp_get_shared_name(pool *p, const char *c2s_names,
    const char *s2c_names) {
  register unsigned int i;
  const char *name = NULL, **client_names, **server_names;
  pool *tmp_pool;
  array_header *client_list, *server_list;

  tmp_pool = make_sub_pool(p);
  pr_pool_tag(tmp_pool, "SFTP shared name pool");

  client_list = fxp_parse_namelist(tmp_pool, c2s_names);
  client_names = (const char **) client_list->elts;

  server_list = fxp_parse_namelist(tmp_pool, s2c_names);
  server_names = (const char **) server_list->elts;

  for (i = 0; i < client_list->nelts; i++) {
    register unsigned int j;

    if (name)
      break;

    for (j = 0; j < server_list->nelts; j++) {
      if (strcmp(client_names[i], server_names[j]) == 0) {
        name = client_names[i];
        break;
      }
    }
  }

  name = pstrdup(p, name);
  destroy_pool(tmp_pool);

  return name;
}

static struct fxp_session *fxp_get_session(uint32_t channel_id) {
  struct fxp_session *sess;

  sess = fxp_sessions;
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

static int fxp_timeout_stalled_cb(CALLBACK_FRAME) {
  pr_event_generate("core.timeout-stalled", NULL);

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "Data transfer stalled timeout (%d secs) reached",
    pr_data_get_timeout(PR_DATA_TIMEOUT_STALLED));
  SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION,
    "data stalled timeout reached");

  return 0;
}

static cmd_rec *fxp_cmd_alloc(pool *p, const char *name, char *arg) {
  cmd_rec *cmd;

  cmd = pr_cmd_alloc(p, 2, pstrdup(p, name), arg ? arg : "");
  cmd->arg = arg;
  cmd->tmp_pool = pr_pool_create_sz(p, 64);

  return cmd;
}

static const char *fxp_strerror(uint32_t status) {
    switch (status) {
      case SSH2_FX_OK:
        return "OK";

      case SSH2_FX_EOF:
        return "End of file";

      case SSH2_FX_NO_SUCH_FILE:
        return "No such file";

      case SSH2_FX_PERMISSION_DENIED:
        return "Permission denied";

      case SSH2_FX_BAD_MESSAGE:
        return "Bad message";

      case SSH2_FX_OP_UNSUPPORTED:
        return "Unsupported operation";

      case SSH2_FX_INVALID_HANDLE:
        return "Invalid handle";

      case SSH2_FX_NO_SUCH_PATH:
        return "No such path";

      case SSH2_FX_FILE_ALREADY_EXISTS:
        return "File already exists";

      case SSH2_FX_NO_SPACE_ON_FILESYSTEM:
        return "Out of disk space";

      case SSH2_FX_QUOTA_EXCEEDED:
        return "Quota exceeded";

      case SSH2_FX_UNKNOWN_PRINCIPAL:
        return "Unknown principal";

      case SSH2_FX_LOCK_CONFLICT:
        return "Lock conflict";

      case SSH2_FX_DIR_NOT_EMPTY:
        return "Directory is not empty";

      case SSH2_FX_NOT_A_DIRECTORY:
        return "Not a directory";

      case SSH2_FX_INVALID_FILENAME:
        return "Invalid filename";

      case SSH2_FX_LINK_LOOP:
        return "Link loop";

      case SSH2_FX_INVALID_PARAMETER:
        return "Invalid parameter";

      case SSH2_FX_FILE_IS_A_DIRECTORY:
        return "File is a directory";

      case SSH2_FX_OWNER_INVALID:
        return "Invalid owner";

      case SSH2_FX_GROUP_INVALID:
        return "Invalid group";
    }

    return "Failure";
}

static uint32_t fxp_errno2status(int xerrno, const char **reason) {
  uint32_t status_code = SSH2_FX_FAILURE;

  /* Provide a default reason string; it will be overwritten below by a
   * more appropriate string as necessary.
   */ 
  if (reason) {
    *reason = fxp_strerror(status_code);
  }

  switch (xerrno) {
    case 0:
      status_code = SSH2_FX_OK;
      if (reason) {
        *reason = fxp_strerror(status_code);
      }
      break;

    case EOF:
      status_code = SSH2_FX_EOF;
      if (reason) {
        *reason = fxp_strerror(status_code);
      }
      break;

    case EBADF:
    case ENOENT:
#ifdef ENXIO
    case ENXIO:
#endif
      status_code = SSH2_FX_NO_SUCH_FILE;
      if (reason) {
        *reason = fxp_strerror(status_code);
      }
      break;

    case EACCES:
    case EPERM:
      status_code = SSH2_FX_PERMISSION_DENIED;
      if (reason) {
        *reason = fxp_strerror(status_code);
      }
      break;

    case EIO:
    case EXDEV:
      if (reason) {
        *reason = strerror(xerrno);
      }
      break;

    case ENOSYS:
      status_code = SSH2_FX_OP_UNSUPPORTED;
      if (reason) {
        *reason = fxp_strerror(status_code);
      }
      break;

    case EFAULT:
    case EINVAL:
      if (reason) {
        *reason = fxp_strerror(SSH2_FX_INVALID_PARAMETER);
      }

      if (fxp_session->client_version > 5) {
        status_code = SSH2_FX_INVALID_PARAMETER;

      } else {
        status_code = SSH2_FX_OP_UNSUPPORTED;
      }
      break;

    case EEXIST:
      if (reason) {
        *reason = fxp_strerror(SSH2_FX_FILE_ALREADY_EXISTS);
      }

      if (fxp_session->client_version > 3) {
        status_code = SSH2_FX_FILE_ALREADY_EXISTS;
      }
      break;

#ifdef EDQUOT
    case EDQUOT:
      if (reason) {
        *reason = fxp_strerror(SSH2_FX_QUOTA_EXCEEDED);
      }

      if (fxp_session->client_version > 4) {
        status_code = SSH2_FX_QUOTA_EXCEEDED;
      }
      break;
#endif

#ifdef EFBIG
    case EFBIG:
#endif
#ifdef ENOSPC
    case ENOSPC:
#endif
      if (reason) {
        *reason = fxp_strerror(SSH2_FX_NO_SPACE_ON_FILESYSTEM);
      }

      if (fxp_session->client_version > 4) {
        status_code = SSH2_FX_NO_SPACE_ON_FILESYSTEM;
      }
      break;

    case EISDIR:
      if (reason) {
        *reason = fxp_strerror(SSH2_FX_FILE_IS_A_DIRECTORY);
      }

      if (fxp_session->client_version > 5) {
        status_code = SSH2_FX_FILE_IS_A_DIRECTORY;
      }
      break;

    case ENOTDIR:
      if (reason) {
        *reason = fxp_strerror(SSH2_FX_NOT_A_DIRECTORY);
      }

      if (fxp_session->client_version > 5) {
        status_code = SSH2_FX_NOT_A_DIRECTORY;
      }
      break;

    case ELOOP:
      if (reason) {
        *reason = fxp_strerror(SSH2_FX_LINK_LOOP);
      }

      if (fxp_session->client_version > 5) {
        status_code = SSH2_FX_LINK_LOOP;
      }
      break;

#ifdef ENAMETOOLONG
    case ENAMETOOLONG:
      if (reason) {
        *reason = fxp_strerror(SSH2_FX_INVALID_FILENAME);
      }

      if (fxp_session->client_version > 5) {
        status_code = SSH2_FX_INVALID_FILENAME;
      }
      break;
#endif

     /* On AIX5, ENOTEMPTY and EEXIST are defined to be the same value.
      * And using the same value multiple times in a switch statement
      * causes compiler grief.  See:
      *
      *  http://forums.proftpd.org/smf/index.php/topic,3971.0.html
      *
      * To handle this, then, we only use ENOTEMPTY if it is defined to
      * be a different value than EEXIST.  We'll have an AIX-specific
      * check for this particular error case in the fxp_handle_rmdir()
      * function.
      */
#if defined(ENOTEMPTY) && ENOTEMPTY != EEXIST
    case ENOTEMPTY:
      if (reason) {
        *reason = fxp_strerror(SSH2_FX_DIR_NOT_EMPTY);
      }

      if (fxp_session->client_version > 5) {
        status_code = SSH2_FX_DIR_NOT_EMPTY;
      }
      break;
#endif
  }

  return status_code;
}

/* Map the FXP_OPEN flags to POSIX flags. */
static int fxp_get_v3_open_flags(uint32_t flags) {
  int res = 0;

  if (flags & SSH2_FXF_READ) {
    if (flags & SSH2_FXF_WRITE) {
      res = O_RDWR;

#ifdef O_APPEND
      if (flags & SSH2_FXF_APPEND) {
        res |= O_APPEND;
      }
#endif

    } else {
      res = O_RDONLY;
    }

  } else if (flags & SSH2_FXF_WRITE) {
    res = O_WRONLY;

#ifdef O_APPEND
    if (flags & SSH2_FXF_APPEND) {
      res |= O_APPEND;
    }
#endif
  }

  if (flags & SSH2_FXF_CREAT) {
    res |= O_CREAT;

    if (flags & SSH2_FXF_TRUNC) {
      res |= O_TRUNC;
    }

    if (flags & SSH2_FXF_EXCL) {
      res |= O_EXCL;
    }
  }

  return res;
}

static int fxp_get_v5_open_flags(uint32_t desired_access, uint32_t flags) {

  /* Assume that the desired flag is read-only by default. */
  int res = O_RDONLY;

  /* These mappings are found in draft-ietf-secsh-filexfer-05.txt,
   * section 6.3.1.
   */

  if ((desired_access & SSH2_FXF_WANT_READ_DATA) ||
      (desired_access & SSH2_FXF_WANT_READ_ATTRIBUTES)) {

    if ((desired_access & SSH2_FXF_WANT_WRITE_DATA) ||
        (desired_access & SSH2_FXF_WANT_WRITE_ATTRIBUTES)) {
      res = O_RDWR;

#ifdef O_APPEND
      if ((desired_access & SSH2_FXF_WANT_APPEND_DATA) &&
          ((flags & SSH2_FXF_ACCESS_APPEND_DATA) ||
           (flags & SSH2_FXF_ACCESS_APPEND_DATA_ATOMIC))) {
        res |= O_APPEND;
      }
#endif

    } else {
      res = O_RDONLY;
    }

  } else if ((desired_access & SSH2_FXF_WANT_WRITE_DATA) ||
             (desired_access & SSH2_FXF_WANT_WRITE_ATTRIBUTES)) {
    res = O_WRONLY;

#ifdef O_APPEND
    if ((desired_access & SSH2_FXF_WANT_APPEND_DATA) &&
        ((flags & SSH2_FXF_ACCESS_APPEND_DATA) ||
         (flags & SSH2_FXF_ACCESS_APPEND_DATA_ATOMIC))) {
      res |= O_APPEND;
    }
#endif
  }

  if (flags & SSH2_FXF_OPEN_OR_CREATE) {
    res |= O_CREAT;

    /* If we're creating, then it's a write. */
    if (!(res & O_RDWR)) {
      res |= O_WRONLY;
    }
  }

  if (flags & SSH2_FXF_CREATE_TRUNCATE) {
    res |= O_CREAT|O_TRUNC;

    /* If we're creating and truncating, then it's definitely a write. */
    if (!(res & O_RDWR)) {
      res |= O_WRONLY;
    }
  }

  if (flags & SSH2_FXF_TRUNCATE_EXISTING) {
    res |= O_TRUNC;

    /* If we're truncating, then it's a write. */
    if (!(res & O_RDWR)) {
      res |= O_WRONLY;
    }
  }

  return res;
}

/* Like pr_strtime(), except that it uses pr_gmtime() rather than
 * pr_localtime().
 */
static const char *fxp_strtime(pool *p, time_t t) {
  static char buf[64];
  static char *mons[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
    "Aug", "Sep", "Oct", "Nov", "Dec" };
  static char *days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  struct tm *tr;

  memset(buf, '\0', sizeof(buf));

  tr = pr_gmtime(p, &t);
  if (tr != NULL) {
    snprintf(buf, sizeof(buf), "%s %s %2d %02d:%02d:%02d %d",
      days[tr->tm_wday], mons[tr->tm_mon], tr->tm_mday, tr->tm_hour,
      tr->tm_min, tr->tm_sec, tr->tm_year + 1900);

  } else {
    buf[0] = '\0';
  }

  buf[sizeof(buf)-1] = '\0';

  return buf;
}

static const char *fxp_get_request_type_desc(unsigned char request_type) {
  switch (request_type) {
    case SFTP_SSH2_FXP_INIT:
      return "INIT";

    case SFTP_SSH2_FXP_VERSION:
      /* XXX We should never receive this type of message from the client. */
      return "VERSION";

    case SFTP_SSH2_FXP_OPEN:
      return "OPEN";

    case SFTP_SSH2_FXP_CLOSE:
      return "CLOSE";

    case SFTP_SSH2_FXP_READ:
      return "READ";

    case SFTP_SSH2_FXP_WRITE:
      return "WRITE";

    case SFTP_SSH2_FXP_LSTAT:
      return "LSTAT";

    case SFTP_SSH2_FXP_FSTAT:
      return "FSTAT";

    case SFTP_SSH2_FXP_SETSTAT:
      return "SETSTAT";

    case SFTP_SSH2_FXP_FSETSTAT:
      return "FSETSTAT";

    case SFTP_SSH2_FXP_OPENDIR:
      return "OPENDIR";

    case SFTP_SSH2_FXP_READDIR:
      return "READDIR";

    case SFTP_SSH2_FXP_REMOVE:
      return "REMOVE";

    case SFTP_SSH2_FXP_MKDIR:
      return "MKDIR";

    case SFTP_SSH2_FXP_RMDIR:
      return "RMDIR";

    case SFTP_SSH2_FXP_REALPATH:
      return "REALPATH";

    case SFTP_SSH2_FXP_STAT:
      return "STAT";

    case SFTP_SSH2_FXP_RENAME:
      return "RENAME";

    case SFTP_SSH2_FXP_READLINK:
      return "READLINK";

    case SFTP_SSH2_FXP_LINK:
      return "LINK";

    case SFTP_SSH2_FXP_SYMLINK:
      return "SYMLINK";

    case SFTP_SSH2_FXP_LOCK:
      return "LOCK";

    case SFTP_SSH2_FXP_UNLOCK:
      return "UNLOCK";

    case SFTP_SSH2_FXP_STATUS:
      return "STATUS";

    case SFTP_SSH2_FXP_HANDLE:
      return "HANDLE";

    case SFTP_SSH2_FXP_DATA:
      return "DATA";

    case SFTP_SSH2_FXP_NAME:
      return "NAME";

    case SFTP_SSH2_FXP_ATTRS:
      return "ATTRS";

    case SFTP_SSH2_FXP_EXTENDED:
      return "EXTENDED";

    case SFTP_SSH2_FXP_EXTENDED_REPLY:
      return "EXTENDED_REPLY";
  }

  return "(unknown)";
}

static int fxp_path_pass_regex_filters(pool *p, const char *request,
    const char *path) {
  int res;
  xaset_t *set;

  set = get_dir_ctxt(p, (char *) path);

  res = pr_filter_allow_path(set, path);
  switch (res) {
    case 0:
      break;

    case PR_FILTER_ERR_FAILS_ALLOW_FILTER:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "path '%s' for %s denied by PathAllowFilter", path, request);
      errno = EACCES;
      return -1;

    case PR_FILTER_ERR_FAILS_DENY_FILTER:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "path '%s' for %s denied by PathDenyFilter", path, request);
      errno = EACCES;
      return -1;
  }

  return 0;
}

/* FXP_STATUS messages */
static void fxp_status_write(char **buf, uint32_t *buflen, uint32_t request_id,
    uint32_t status_code, const char *status_msg, const char *extra_data) {
  char num[32];

  /* Add a fake response to the response chain, for use by mod_log's
   * logging, e.g. for supporting the %S/%s LogFormat variables.
   */

  pr_response_clear(&resp_list);
  pr_response_clear(&resp_err_list);

  memset(num, '\0', sizeof(num));
  snprintf(num, sizeof(num), "%lu", (unsigned long) status_code);
  num[sizeof(num)-1] = '\0';
  pr_response_add(pstrdup(fxp_session->pool, num), "%s", status_msg);

  sftp_msg_write_byte(buf, buflen, SFTP_SSH2_FXP_STATUS);
  sftp_msg_write_int(buf, buflen, request_id);
  sftp_msg_write_int(buf, buflen, status_code);

  if (fxp_session->client_version >= 3) {
    sftp_msg_write_string(buf, buflen, status_msg);
    /* XXX localization */
    sftp_msg_write_string(buf, buflen, "en-US");

    if (fxp_session->client_version >= 5 &&
        extra_data) {
      /* Used specifically for UNKNOWN_PRINCIPAL errors */
      sftp_msg_write_string(buf, buflen, extra_data);
    }
  }
}

/* The SFTP subsystem Draft defines a few new data types. */

#if 0
/* XXX Not really used for messages from clients. */
static uint16_t fxp_msg_read_short(pool *p, char **buf, uint32_t *buflen) {
  uint16_t val;

  (void) p;

  if (*buflen < sizeof(uint16_t)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SFTP message format error: unable to read short (buflen = %lu)",
      (unsigned long) *buflen); 
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  memcpy(&val, *buf, sizeof(uint16_t));
  (*buf) += sizeof(uint16_t);
  (*buflen) -= sizeof(uint16_t);

  val = ntohs(val);
  return val;
}
#endif

static uint64_t fxp_msg_read_long(pool *p, char **buf, uint32_t *buflen) {
  uint64_t val;
  unsigned char data[8];

  if (*buflen < sizeof(data)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SFTP message format error: unable to read long (buflen = %lu)",
      (unsigned long) *buflen); 
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  memcpy(data, *buf, sizeof(data));
  (*buf) += sizeof(data);
  (*buflen) -= sizeof(data);

  val = (uint64_t) data[0] << 56;
  val |= (uint64_t) data[1] << 48;
  val |= (uint64_t) data[2] << 40;
  val |= (uint64_t) data[3] << 32;
  val |= (uint64_t) data[4] << 24;
  val |= (uint64_t) data[5] << 16;
  val |= (uint64_t) data[6] << 8;
  val |= (uint64_t) data[7];

  return val;
}

#if 0
/* XXX Not used yet, if ever. */
static struct fxp_extpair *fxp_msg_read_extpair(pool *p, char **buf,
    uint32_t *buflen) {
  uint32_t namelen, datalen;
  char *name, *data;
  struct fxp_extpair *extpair;

  namelen = sftp_msg_read_int(p, buf, buflen);
  if (*buflen < namelen) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SFTP message format error: unable to read %lu bytes of extpair name "
      "data (buflen = %lu)", (unsigned long) namelen, (unsigned long) *buflen);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  name = palloc(p, namelen + 1);
  memcpy(name, *buf, namelen);
  (*buf) += namelen;
  (*buflen) -= namelen;
  name[namelen] = '\0';

  datalen = sftp_msg_read_int(p, buf, buflen);
  if (datalen > 0) {
    data = sftp_msg_read_data(p, buf, buflen, datalen);

  } else {
    data = NULL;
  }

  extpair = palloc(p, sizeof(struct fxp_extpair));
  extpair->ext_name = name;
  extpair->ext_datalen = datalen;
  extpair->ext_data = data;

  return extpair;
}
#endif

static void fxp_msg_write_short(char **buf, uint32_t *buflen, uint16_t val) {
  if (*buflen < sizeof(uint16_t)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SFTP message format error: unable to write short (buflen = %lu)",
      (unsigned long) *buflen);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  val = htons(val);
  memcpy(*buf, &val, sizeof(uint16_t));
  (*buf) += sizeof(uint16_t);
  (*buflen) -= sizeof(uint16_t);
}

static void fxp_msg_write_long(char **buf, uint32_t *buflen, uint64_t val) {
  unsigned char data[8];

  if (*buflen < sizeof(uint64_t)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SFTP message format error: unable to write long (buflen = %lu)",
      (unsigned long) *buflen);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  data[0] = (unsigned char) (val >> 56) & 0xFF;
  data[1] = (unsigned char) (val >> 48) & 0xFF;
  data[2] = (unsigned char) (val >> 40) & 0xFF;
  data[3] = (unsigned char) (val >> 32) & 0xFF;
  data[4] = (unsigned char) (val >> 24) & 0xFF;
  data[5] = (unsigned char) (val >> 16) & 0xFF;
  data[6] = (unsigned char) (val >> 8) & 0xFF;
  data[7] = (unsigned char) val & 0xFF;

  sftp_msg_write_data(buf, buflen, (char *) data, sizeof(data), FALSE);
}

static void fxp_msg_write_extpair(char **buf, uint32_t *buflen,
    struct fxp_extpair *extpair) {
  uint32_t len;

  len = strlen(extpair->ext_name);
  sftp_msg_write_data(buf, buflen, extpair->ext_name, len, TRUE);
  sftp_msg_write_data(buf, buflen, extpair->ext_data, extpair->ext_datalen,
    TRUE);
}

static int fxp_attrs_set(pr_fh_t *fh, const char *path, struct stat *attrs,
    uint32_t attr_flags, char **buf, uint32_t *buflen, struct fxp_packet *fxp) {
  struct stat st;
  int res;

  /* Note: path is never null; it is always passed by the caller.  fh MAY be
   * null, depending on whether the caller already has a file handle or not.
   */

  if (fh != NULL) {
    res = pr_fsio_fstat(fh, &st);

  } else {
    res = pr_fsio_lstat(path, &st);
  }

  if (res < 0) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error checking '%s': %s", path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(buf, buflen, fxp->request_id, status_code, reason, NULL);

    errno = xerrno;
    return -1;
  }

  if (attr_flags & SSH2_FX_ATTR_PERMISSIONS) {
    if (attrs->st_mode &&
        st.st_mode != attrs->st_mode) {
      cmd_rec *cmd;

      cmd = pr_cmd_alloc(fxp->pool, 1, "SITE_CHMOD");
      cmd->arg = pstrdup(fxp->pool, path);
      if (!dir_check(fxp->pool, cmd, "WRITE", (char *) path, NULL)) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "chmod of '%s' blocked by <Limit> configuration", path);

        errno = EACCES;
        res = -1;

      } else {
        if (fh != NULL) {
          res = pr_fsio_fchmod(fh, attrs->st_mode);

        } else {
          res = pr_fsio_chmod(path, attrs->st_mode);
        }
      }

      if (res < 0) {
        uint32_t status_code;
        const char *reason;
        int xerrno = errno;

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error changing permissions of '%s' to 0%o: %s", path,
          (unsigned int) attrs->st_mode, strerror(xerrno));

        status_code = fxp_errno2status(xerrno, &reason);

        pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
          "('%s' [%d])", (unsigned long) status_code, reason,
          xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

        fxp_status_write(buf, buflen, fxp->request_id, status_code, reason,
          NULL);

        errno = xerrno;
        return -1;

      } else {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "client set permissions on '%s' to 0%o", path,
          (unsigned int) attrs->st_mode);
      }
    }
  }

  if ((attr_flags & SSH2_FX_ATTR_UIDGID) ||
      (attr_flags & SSH2_FX_ATTR_OWNERGROUP)) {
    int do_chown = FALSE;

    uid_t client_uid = (uid_t) -1;
    gid_t client_gid = (uid_t) -1;

    if (st.st_uid != attrs->st_uid) {
      client_uid = attrs->st_uid;
      do_chown = TRUE;
    }

    if (st.st_gid != attrs->st_gid) {
      client_gid = attrs->st_gid;
      do_chown = TRUE;
    }

    if (do_chown) {
      if (fh != NULL) {
        res = pr_fsio_fchown(fh, client_uid, client_gid);

      } else {
        res = pr_fsio_chown(path, client_uid, client_gid);
      }

      if (res < 0) {
        uint32_t status_code;
        const char *reason;
        int xerrno = errno;

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error changing ownership of '%s' to UID %lu, GID %lu: %s",
          path, (unsigned long) client_uid, (unsigned long) client_gid,
          strerror(xerrno));

        status_code = fxp_errno2status(xerrno, &reason);

        pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
          "('%s' [%d])", (unsigned long) status_code, reason,
          xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

        fxp_status_write(buf, buflen, fxp->request_id, status_code, reason,
          NULL);

        errno = xerrno;
        return -1;

      } else {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "client set ownership of '%s' to UID %lu, GID %lu",
          path, (unsigned long) client_uid, (unsigned long) client_gid);
      }
    }
  }

  if (attr_flags & SSH2_FX_ATTR_SIZE) {
    if (attrs->st_size &&
        st.st_size != attrs->st_size) {

      /* If we're dealing with a FIFO, just pretend that the truncate(2)
       * succeeded; FIFOs don't handle truncation well.  And it won't
       * necessarily matter to the client, right?
       */
      if (S_ISREG(st.st_mode)) {
        if (fh != NULL) {
          res = pr_fsio_ftruncate(fh, attrs->st_size);

        } else {
          res = pr_fsio_truncate(path, attrs->st_size);
        }

      } else {
        res = 0;
      }

      if (res < 0) {
        uint32_t status_code;
        const char *reason;
        int xerrno = errno;

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error changing size of '%s' from %" PR_LU " bytes to %" PR_LU
          " bytes: %s", path, (pr_off_t) st.st_size, (pr_off_t) attrs->st_size,
          strerror(xerrno));

        status_code = fxp_errno2status(xerrno, &reason);

        pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
          "('%s' [%d])", (unsigned long) status_code, reason,
          xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

        fxp_status_write(buf, buflen, fxp->request_id, status_code, reason,
          NULL);

        errno = xerrno;
        return -1;

      } else {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "client set size of '%s' to %" PR_LU " bytes", path,
          (pr_off_t) attrs->st_size);
      }
    }
  }

  if (fxp_session->client_version <= 3 &&
      attr_flags & SSH2_FX_ATTR_ACMODTIME) {
    if (st.st_atime != attrs->st_atime ||
        st.st_mtime != attrs->st_mtime) {
      struct timeval tvs[2];

      tvs[0].tv_sec = attrs->st_atime;
      tvs[0].tv_usec = 0;

      tvs[1].tv_sec = attrs->st_mtime;
      tvs[1].tv_usec = 0;

      if (fh != NULL) {
        res = pr_fsio_futimes(fh, tvs);

      } else {
        res = pr_fsio_utimes(path, tvs);
      }

      if (res < 0) {
        uint32_t status_code;
        const char *reason;
        int xerrno = errno;

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error changing access/modification times '%s': %s", path,
           strerror(xerrno));

        status_code = fxp_errno2status(xerrno, &reason);

        pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
          "('%s' [%d])", (unsigned long) status_code, reason,
          xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

        fxp_status_write(buf, buflen, fxp->request_id, status_code, reason,
          NULL);

        errno = xerrno;
        return -1;

      } else {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "client set access time of '%s' to %s, modification time to %s",
          path, fxp_strtime(fxp->pool, attrs->st_atime),
          fxp_strtime(fxp->pool, attrs->st_mtime));
      }
    }
  }

  if (fxp_session->client_version > 3) {
    if (attr_flags & SSH2_FX_ATTR_ACCESSTIME) {
      if (st.st_atime != attrs->st_atime) {
        struct timeval tvs[2];

        tvs[0].tv_sec = attrs->st_atime;
        tvs[0].tv_usec = 0;

        tvs[1].tv_sec = st.st_mtime;
        tvs[1].tv_usec = 0;

        if (fh != NULL) {
          res = pr_fsio_futimes(fh, tvs);

        } else {
          res = pr_fsio_utimes(path, tvs);
        }

        if (res < 0) {
          uint32_t status_code;
          const char *reason;
          int xerrno = errno;

          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error changing access time '%s': %s", path, strerror(xerrno));

          status_code = fxp_errno2status(xerrno, &reason);
  
          pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
            "('%s' [%d])", (unsigned long) status_code, reason,
            xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

          fxp_status_write(buf, buflen, fxp->request_id, status_code, reason,
            NULL);

          errno = xerrno;
          return -1;

        } else {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "client set access time of '%s' to %s", path,
            fxp_strtime(fxp->pool, attrs->st_atime));
        }
      }
    }

    if (attr_flags & SSH2_FX_ATTR_MODIFYTIME) {
      if (st.st_mtime != attrs->st_mtime) {
        struct timeval tvs[2];

        tvs[0].tv_sec = st.st_atime;
        tvs[0].tv_usec = 0;

        tvs[1].tv_sec = attrs->st_mtime;
        tvs[1].tv_usec = 0;

        if (fh != NULL) {
          res = pr_fsio_futimes(fh, tvs);

        } else {
          res = pr_fsio_utimes(path, tvs);
        }

        if (res < 0) {
          uint32_t status_code;
          const char *reason;
          int xerrno = errno;

          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error changing modification time '%s': %s", path,
            strerror(xerrno));

          status_code = fxp_errno2status(xerrno, &reason);

          pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
            "('%s' [%d])", (unsigned long) status_code, reason,
            xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

          fxp_status_write(buf, buflen, fxp->request_id, status_code, reason,
            NULL);

          errno = xerrno;
          return -1;

        } else {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "client set modification time of '%s' to %s", path,
            fxp_strtime(fxp->pool, attrs->st_mtime));
        }
      }
    }
  }

  return 0;
}

/* Provide a stringified representation of attributes sent by clients. */

static const char *fxp_strftype(mode_t mode) {
  if (S_ISREG(mode)) {
    return "file";
  }

  if (S_ISDIR(mode)) {
    return "dir";
  }

  if (S_ISLNK(mode)) {
    return "symlink";
  }

  if (S_ISSOCK(mode)) {
    return "socket";
  }

#ifdef S_ISFIFO
  if (S_ISFIFO(mode)) {
    return "fifo";
  }
#endif

#ifdef S_ISCHR
  if (S_ISCHR(mode)) {
    return "dev/char";
  }
#endif

#ifdef S_ISBLK
  if (S_ISBLK(mode)) {
    return "dev/block";
  }
#endif

  return "unknown";
}

static char *fxp_strattrs(pool *p, struct stat *st, uint32_t *attr_flags) {
  char buf[PR_TUNABLE_BUFFER_SIZE], *ptr;
  size_t buflen = 0, bufsz;
  uint32_t flags = 0;

  buflen = 0;
  bufsz = sizeof(buf);
  memset(buf, '\0', bufsz);

  ptr = buf;

  if (attr_flags != NULL) {
    flags = *attr_flags;

  } else {
    if (fxp_session->client_version <= 3) {
      flags = SSH2_FX_ATTR_SIZE|SSH2_FX_ATTR_UIDGID|SSH2_FX_ATTR_PERMISSIONS|
        SSH2_FX_ATTR_ACMODTIME;

    } else {
      flags = SSH2_FX_ATTR_SIZE|SSH2_FX_ATTR_PERMISSIONS|
        SSH2_FX_ATTR_ACCESSTIME|SSH2_FX_ATTR_MODIFYTIME|
        SSH2_FX_ATTR_OWNERGROUP;
    }
  }

  snprintf(ptr, bufsz - buflen, "type=%s;", fxp_strftype(st->st_mode));
  buflen = strlen(buf);
  ptr = buf + buflen;

  if (flags & SSH2_FX_ATTR_SIZE) {
    snprintf(ptr, bufsz - buflen, "size=%" PR_LU ";", (pr_off_t) st->st_size);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  if ((flags & SSH2_FX_ATTR_UIDGID) ||
      (flags & SSH2_FX_ATTR_OWNERGROUP)) {
    snprintf(ptr, bufsz - buflen, "UNIX.owner=%lu;",
      (unsigned long) st->st_uid);
    buflen = strlen(buf);
    ptr = buf + buflen;

    snprintf(ptr, bufsz - buflen, "UNIX.group=%lu;",
      (unsigned long) st->st_gid);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  if (flags & SSH2_FX_ATTR_PERMISSIONS) {
    snprintf(ptr, bufsz - buflen, "UNIX.mode=0%o;",
      (unsigned int) st->st_mode & 07777);
    buflen = strlen(buf);
    ptr = buf + buflen;
  }

  if (fxp_session->client_version <= 3) {
    if (flags & SSH2_FX_ATTR_ACMODTIME) {
      struct tm *tm;

      tm = pr_gmtime(p, &st->st_atime);
      snprintf(ptr, bufsz - buflen, "access=%04d%02d%02d%02d%02d%02d;",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min,
        tm->tm_sec);
      buflen = strlen(buf);
      ptr = buf + buflen;

      tm = pr_gmtime(p, &st->st_mtime);
      snprintf(ptr, bufsz - buflen, "modify=%04d%02d%02d%02d%02d%02d;",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min,
        tm->tm_sec);
      buflen = strlen(buf);
      ptr = buf + buflen;
    }

  } else { 
    if (flags & SSH2_FX_ATTR_ACCESSTIME) {
      struct tm *tm;

      tm = pr_gmtime(p, &st->st_atime);
      snprintf(ptr, bufsz - buflen, "access=%04d%02d%02d%02d%02d%02d;",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min,
        tm->tm_sec);
      buflen = strlen(buf);
      ptr = buf + buflen;
    }

    if (flags & SSH2_FX_ATTR_MODIFYTIME) {
      struct tm *tm;

      tm = pr_gmtime(p, &st->st_mtime);
      snprintf(ptr, bufsz - buflen, "modify=%04d%02d%02d%02d%02d%02d;",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min,
        tm->tm_sec);
      buflen = strlen(buf);
      ptr = buf + buflen;
    }
  }

  return pstrdup(p, buf);
}

static char *fxp_strattrflags(pool *p, uint32_t flags) {
  char *str = "";

  if (flags & SSH2_FX_ATTR_SIZE) {
    str = pstrcat(p, str, *str ? ";" : "", "size", NULL);
  }

  if ((flags & SSH2_FX_ATTR_UIDGID) ||
      (flags & SSH2_FX_ATTR_OWNERGROUP)) {
    str = pstrcat(p, str, *str ? ";" : "", "UNIX.owner", NULL);
    str = pstrcat(p, str, *str ? ";" : "", "UNIX.group", NULL);
  }

  if (flags & SSH2_FX_ATTR_PERMISSIONS) {
    str = pstrcat(p, str, *str ? ";" : "", "UNIX.mode", NULL);
  }

  if (fxp_session->client_version <= 3) {
    if (flags & SSH2_FX_ATTR_ACMODTIME) {
      str = pstrcat(p, str, *str ? ";" : "", "access", NULL);
      str = pstrcat(p, str, *str ? ";" : "", "modify", NULL);
    }

  } else {
    if (flags & SSH2_FX_ATTR_ACCESSTIME) {
      str = pstrcat(p, str, *str ? ";" : "", "access", NULL);
    }

    if (flags & SSH2_FX_ATTR_MODIFYTIME) {
      str = pstrcat(p, str, *str ? ";" : "", "modify", NULL);
    }
  }

  return str;
}

static char *fxp_stroflags(pool *p, int flags) {
  char *str = "";

  if (flags == O_RDONLY) {
    str = pstrcat(p, str, "O_RDONLY", NULL);

  } else if (flags & O_RDWR) {
    str = pstrcat(p, str, "O_RDWR", NULL);

  } else if (flags & O_WRONLY) {
    str = pstrcat(p, str, "O_WRONLY", NULL);
  }

#ifdef O_APPEND
  if (flags & O_APPEND) {
    str = pstrcat(p, str, *str ? "|" : "", "O_APPEND", NULL);
  }
#endif

  if (flags & O_CREAT) {
    str = pstrcat(p, str, *str ? "|" : "", "O_CREAT", NULL);
  }

  if (flags & O_TRUNC) {
    str = pstrcat(p, str, *str ? "|" : "", "O_TRUNC", NULL);
  }

  if (flags & O_EXCL) {
    str = pstrcat(p, str, *str ? "|" : "", "O_EXCL", NULL);
  }

  return str;
}

static struct stat *fxp_attrs_read(struct fxp_packet *fxp, char **buf,
    uint32_t *buflen, uint32_t *flags) {
  struct stat *st;

  st = pcalloc(fxp->pool, sizeof(struct stat));

  *flags = sftp_msg_read_int(fxp->pool, buf, buflen);

  if (fxp_session->client_version <= 3) {
    if (*flags & SSH2_FX_ATTR_SIZE) {
      st->st_size = fxp_msg_read_long(fxp->pool, buf, buflen);
    }

    if (*flags & SSH2_FX_ATTR_UIDGID) {
      st->st_uid = sftp_msg_read_int(fxp->pool, buf, buflen);
      st->st_gid = sftp_msg_read_int(fxp->pool, buf, buflen);
    }

    if (*flags & SSH2_FX_ATTR_PERMISSIONS) {
      st->st_mode = sftp_msg_read_int(fxp->pool, buf, buflen);
    }

    if (*flags & SSH2_FX_ATTR_ACMODTIME) {
      st->st_atime = sftp_msg_read_int(fxp->pool, buf, buflen);
      st->st_mtime = sftp_msg_read_int(fxp->pool, buf, buflen);
    }

    /* XXX Vendor-specific extensions */

  } else {
    char file_type;

/* XXX Use this to create different types of files?  E.g. what if the client
 * wants to OPEN a socket, or a fifo?
 */
    file_type = sftp_msg_read_byte(fxp->pool, buf, buflen);

    if (*flags & SSH2_FX_ATTR_SIZE) {
      st->st_size = fxp_msg_read_long(fxp->pool, buf, buflen);
    }

    if (*flags & SSH2_FX_ATTR_OWNERGROUP) {
      char *name;
      uid_t uid;
      gid_t gid;

      name = sftp_msg_read_string(fxp->pool, buf, buflen);
      uid = pr_auth_name2uid(fxp->pool, name);
      if (uid == (uid_t) -1) {
        char *buf2, *ptr2;
        uint32_t buflen2, bufsz2, status_code;
        struct fxp_packet *resp;

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "unable to translate user name '%s' to UID, UNKNOWN_PRINCIPAL error",
          name);

        buflen2 = bufsz2 = FXP_RESPONSE_DATA_DEFAULT_SZ;
        buf2 = ptr2 = palloc(fxp->pool, bufsz2);

        status_code = SSH2_FX_UNKNOWN_PRINCIPAL;

        pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
          (unsigned long) status_code, fxp_strerror(status_code));

        fxp_status_write(&buf2, &buflen2, fxp->request_id, status_code,
          fxp_strerror(status_code), name);

        resp = fxp_packet_create(fxp->pool, fxp->channel_id);
        resp->payload = ptr2;
        resp->payload_sz = (bufsz2 - buflen2);

        if (fxp_packet_write(resp) < 0) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error sending UNKNOWN_PRINCIPAL status: %s", strerror(errno));
        }

        return NULL;
      }

      st->st_uid = uid;

      name = sftp_msg_read_string(fxp->pool, buf, buflen);
      gid = pr_auth_name2gid(fxp->pool, name);
      if (gid == (gid_t) -1) {
        char *buf2, *ptr2;
        uint32_t buflen2, bufsz2, status_code;
        struct fxp_packet *resp;

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "unable to translate group name '%s' to GID, UNKNOWN_PRINCIPAL error",
          name);

        buflen2 = bufsz2 = FXP_RESPONSE_DATA_DEFAULT_SZ;
        buf2 = ptr2 = palloc(fxp->pool, bufsz2);

        status_code = SSH2_FX_UNKNOWN_PRINCIPAL;

        pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
          (unsigned long) status_code, fxp_strerror(status_code));

        fxp_status_write(&buf2, &buflen2, fxp->request_id, status_code,
          fxp_strerror(status_code), name);

        resp = fxp_packet_create(fxp->pool, fxp->channel_id);
        resp->payload = ptr2;
        resp->payload_sz = (bufsz2 - buflen2);

        if (fxp_packet_write(resp) < 0) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error sending UNKNOWN_PRINCIPAL status: %s", strerror(errno));
        }

        return NULL;
      }

      st->st_gid = gid;
    }

    if (*flags & SSH2_FX_ATTR_PERMISSIONS) {
      st->st_mode = sftp_msg_read_int(fxp->pool, buf, buflen);
    }

    if (*flags & SSH2_FX_ATTR_ACCESSTIME) {
      st->st_atime = fxp_msg_read_long(fxp->pool, buf, buflen);
    }

    if (*flags & SSH2_FX_ATTR_MODIFYTIME) {
      st->st_mtime = fxp_msg_read_long(fxp->pool, buf, buflen);
    }

    /* XXX Vendor-specific extensions */
  }

  return st;
}

static char fxp_get_file_type(mode_t mode) {
  if (S_ISREG(mode)) {
    return SSH2_FX_ATTR_FTYPE_REGULAR;
  }

  if (S_ISDIR(mode)) {
    return SSH2_FX_ATTR_FTYPE_DIRECTORY;
  }

  if (S_ISLNK(mode)) {
    return SSH2_FX_ATTR_FTYPE_SYMLINK;
  }

  if (S_ISSOCK(mode)) {
    if (fxp_session->client_version <= 4) {
      return SSH2_FX_ATTR_FTYPE_SPECIAL;
    }

    return SSH2_FX_ATTR_FTYPE_SOCKET;
  }

#ifdef S_ISFIFO
  if (S_ISFIFO(mode)) {
    if (fxp_session->client_version <= 4) {
      return SSH2_FX_ATTR_FTYPE_SPECIAL;
    }

    return SSH2_FX_ATTR_FTYPE_FIFO;
  }
#endif

#ifdef S_ISCHR
  if (S_ISCHR(mode)) {
    if (fxp_session->client_version <= 4) {
      return SSH2_FX_ATTR_FTYPE_SPECIAL;
    }

    return SSH2_FX_ATTR_FTYPE_CHAR_DEVICE;
  }
#endif

#ifdef S_ISBLK
  if (S_ISBLK(mode)) {
    if (fxp_session->client_version <= 4) {
      return SSH2_FX_ATTR_FTYPE_SPECIAL;
    }

    return SSH2_FX_ATTR_FTYPE_BLOCK_DEVICE;
  }
#endif

  return SSH2_FX_ATTR_FTYPE_UNKNOWN;
}

static void fxp_attrs_write(pool *p, char **buf, uint32_t *buflen,
    struct stat *st) {
  uint32_t flags;
  mode_t perms;

  if (fxp_session->client_version <= 3) {
    flags = SSH2_FX_ATTR_SIZE|SSH2_FX_ATTR_UIDGID|SSH2_FX_ATTR_PERMISSIONS|
      SSH2_FX_ATTR_ACMODTIME;
    perms = st->st_mode;

    sftp_msg_write_int(buf, buflen, flags);
    fxp_msg_write_long(buf, buflen, st->st_size);
    sftp_msg_write_int(buf, buflen, st->st_uid);
    sftp_msg_write_int(buf, buflen, st->st_gid);
    sftp_msg_write_int(buf, buflen, perms);
    sftp_msg_write_int(buf, buflen, st->st_atime);
    sftp_msg_write_int(buf, buflen, st->st_mtime);

  } else {
    char file_type;

    perms = st->st_mode;

    /* Make sure that we do not include the file type bits when sending the
     * permission bits of the st_mode field.
     */
    perms &= ~S_IFMT;

    flags = SSH2_FX_ATTR_SIZE|SSH2_FX_ATTR_PERMISSIONS|SSH2_FX_ATTR_ACCESSTIME|
      SSH2_FX_ATTR_MODIFYTIME|SSH2_FX_ATTR_OWNERGROUP;

    file_type = fxp_get_file_type(st->st_mode);

    sftp_msg_write_int(buf, buflen, flags);
    sftp_msg_write_byte(buf, buflen, file_type);
    fxp_msg_write_long(buf, buflen, st->st_size);
    sftp_msg_write_string(buf, buflen, pr_auth_uid2name(p, st->st_uid));
    sftp_msg_write_string(buf, buflen, pr_auth_gid2name(p, st->st_gid));
    sftp_msg_write_int(buf, buflen, perms);
    fxp_msg_write_long(buf, buflen, st->st_atime);
    fxp_msg_write_long(buf, buflen, st->st_mtime);
  }
}

/* The strmode(3) function appears in some BSDs, but is not portable. */
static char *fxp_strmode(pool *p, mode_t mode) {
  char mode_str[12];

  memset(mode_str, '\0', sizeof(mode_str));
  sstrncpy(mode_str, "?--------- ", sizeof(mode_str));

  switch (mode & S_IFMT) {
    case S_IFREG:
      mode_str[0] = '-';
      break;

    case S_IFDIR:
      mode_str[0] = 'd';
      break;

    case S_IFLNK:
      mode_str[0] = 'l';
      break;

#ifdef S_IFSOCK
    case S_IFSOCK:
      mode_str[0] = 's';
      break;
#endif

    case S_IFIFO:
      mode_str[0] = 'p';
      break;

    case S_IFBLK:
      mode_str[0] = 'b';
      break;

    case S_IFCHR:
      mode_str[0] = 'c';
      break;
  }

  if (mode_str[0] != '?') {
    /* User perms */
    mode_str[1] = (mode & S_IRUSR) ? 'r' : '-';
    mode_str[2] = (mode & S_IWUSR) ? 'w' : '-';
    mode_str[3] = (mode & S_IXUSR) ?
      ((mode & S_ISUID) ? 's' : 'x') : ((mode & S_ISUID) ? 'S' : '-');

    /* Group perms */
    mode_str[4] = (mode & S_IRGRP) ? 'r' : '-';
    mode_str[5] = (mode & S_IWGRP) ? 'w' : '-';
    mode_str[6] = (mode & S_IXGRP) ?
      ((mode & S_ISGID) ? 's' : 'x') : ((mode & S_ISGID) ? 'S' : '-');

    /* World perms */
    mode_str[7] = (mode & S_IROTH) ? 'r' : '-';
    mode_str[8] = (mode & S_IWOTH) ? 'w' : '-';
    mode_str[9] = (mode & S_IXOTH) ?
      ((mode & S_ISVTX) ? 't' : 'x') : ((mode & S_ISVTX) ? 'T' : '-');
  }

  return pstrdup(p, mode_str);
}

static char *fxp_get_path_listing(pool *p, const char *path, struct stat *st) {
  const char *user, *group;
  char listing[256], *mode_str, time_str[64];
  struct tm *t;
  int user_len, group_len;
  size_t time_strlen;
  time_t now = time(NULL);

  memset(listing, '\0', sizeof(listing));
  memset(time_str, '\0', sizeof(time_str));
 
  mode_str = fxp_strmode(p, st->st_mode); 

  if (fxp_use_gmt) {
    t = pr_gmtime(p, (time_t *) &st->st_mtime);

  } else {
    t = pr_localtime(p, (time_t *) &st->st_mtime);
  }

  /* Use strftime(3) to format the time entry for us.  Seems some SFTP clients
   * are *very* particular about this formatting.  Understandable, since
   * the SFTP Drafts for protocol version 3 did not actually define a format;
   * now most clients conform to the format used by OpenSSH.
   */

  if ((now - st->st_mtime) > (180 * 24 * 60 * 60)) {
    time_strlen = strftime(time_str, sizeof(time_str), "%b %e  %Y", t);

  } else {
    time_strlen = strftime(time_str, sizeof(time_str), "%b %e %H:%M", t);
  }

  user = pr_auth_uid2name(p, st->st_uid);
  user_len = MAX(strlen(user), 8);

  group = pr_auth_gid2name(p, st->st_gid);
  group_len = MAX(strlen(group), 8);

  snprintf(listing, sizeof(listing)-1,
    "%s %3u %-*s %-*s %8" PR_LU " %s %s", mode_str,
    (unsigned int) st->st_nlink, user_len, user, group_len, group,
    (pr_off_t) st->st_size, time_str, path);
  return pstrdup(p, listing);
}

static struct fxp_dirent *fxp_get_dirent(pool *p, cmd_rec *cmd,
    const char *real_path, mode_t *fake_mode) {
  struct fxp_dirent *fxd;
  struct stat st;
  int hidden = 0, res;

  if (pr_fsio_lstat(real_path, &st) < 0) {
    return NULL;
  }

  res = dir_check(p, cmd, G_DIRS, real_path, &hidden);
  if (res == 0 ||
      hidden == TRUE) {
    errno = EACCES;
    return NULL;
  }

  if (fake_mode != NULL) {
    mode_t mode;

    mode = *fake_mode;
    mode |= (st.st_mode & S_IFMT);

    if (S_ISDIR(st.st_mode)) {
      if (st.st_mode & S_IROTH) {
        mode |= S_IXOTH;
      }

      if (st.st_mode & S_IRGRP) {
        mode |= S_IXGRP;
      }

      if (st.st_mode & S_IRUSR) {
        mode |= S_IXUSR;
      }
    }

    st.st_mode = mode;
  }

  fxd = pcalloc(p, sizeof(struct fxp_dirent));  
  fxd->real_path = real_path;
  fxd->st = pcalloc(p, sizeof(struct stat));
  memcpy(fxd->st, &st, sizeof(struct stat));

  return fxd;
}

static void fxp_name_write(pool *p, char **buf, uint32_t *buflen,
    const char *path, struct stat *st) {

  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    sftp_msg_write_string(buf, buflen, sftp_utf8_encode_str(p, path));

  } else {
    sftp_msg_write_string(buf, buflen, path);
  }

  if (fxp_session->client_version <= 3) {
    char *path_desc;

    path_desc = fxp_get_path_listing(p, path, st);

    if (fxp_session->client_version >= fxp_utf8_protocol_version) {
      sftp_msg_write_string(buf, buflen, sftp_utf8_encode_str(p, path_desc));

    } else {
      sftp_msg_write_string(buf, buflen, path_desc);
    }
  }

  fxp_attrs_write(p, buf, buflen, st);
}

/* FX Handle Mgmt */

static int fxp_handle_add(uint32_t channel_id, struct fxp_handle *fxh) {
  int res;

  if (fxp_session->handle_tab == NULL) {
    fxp_session->handle_tab = pr_table_alloc(fxp_session->pool, 0);
  }

  res = pr_table_add(fxp_session->handle_tab, fxh->name, fxh, sizeof(void *)); 
  if (res < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error stashing handle: %s", strerror(errno));
  }

  return res;
}

static struct fxp_handle *fxp_handle_create(pool *p) {
  char *handle, *template;
  pool *sub_pool;
  struct fxp_handle *fxh;

  sub_pool = pr_pool_create_sz(p, 64);
  fxh = pcalloc(sub_pool, sizeof(struct fxp_handle));
  fxh->pool = sub_pool;

  /* Use mktemp(3) to generate a random string usable as a handle.  Most
   * mktemp(3) implementations support up to 6 'X' characters in the template,
   * thus we need to allocate a string that is 7 characters long (to include
   * the trailing NUL).
   */
  template = palloc(sub_pool, 7);

  while (1) {
    register unsigned int i;

    /* Keep trying until mktemp(3) returns a string that we haven't used
     * yet.  We need to avoid collisions.
     */
    pr_signals_handle();

    for (i = 0; i < 6; i++) {
      template[i] = 'X';
    }
    template[6] = '\0';

    handle = mktemp(template);
    if (handle == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error using mktemp(3): %s", strerror(errno));
      destroy_pool(sub_pool);
      return NULL;
    }

    if (fxp_handle_get(handle) == NULL) {
      fxh->name = handle;
      break;
    }

    pr_trace_msg(trace_channel, 4,
      "handle '%s' already used, generating another", handle);
  }

  return fxh;
}

/* NOTE: this function is ONLY called when the session is closed, for
 * "aborting" any file handles still left open by the client.
 */
static int fxp_handle_abort(const void *key_data, size_t key_datasz,
    void *value_data, size_t value_datasz, void *user_data) {
  struct fxp_handle *fxh;
  char *abs_path, *curr_path = NULL, *real_path = NULL;
  char direction;
  unsigned char *delete_aborted_stores = NULL;
  cmd_rec *cmd = NULL;

  fxh = value_data;
  delete_aborted_stores = user_data;

  /* Is this a file or a directory handle? */
  if (fxh->dirh != NULL) {
    if (pr_fsio_closedir(fxh->dirh) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error closing aborted directory '%s': %s", fxh->dir, strerror(errno));
    }

    fxh->dirh = NULL;
    return 0;
  }

  curr_path = pstrdup(fxh->pool, fxh->fh->fh_path);
  real_path = curr_path;
  if (fxh->fh_real_path) {
    real_path = fxh->fh_real_path;
  }

  /* Write an 'incomplete' TransferLog entry for this. */
  abs_path = dir_abs_path(fxh->pool, curr_path, TRUE);

  if (fxh->fh_flags == O_RDONLY) {
    direction = 'o';

  } else {
    direction = 'i';
  }

  if (fxh->fh_flags & O_APPEND) {
    cmd = fxp_cmd_alloc(fxh->pool, C_APPE, pstrdup(fxh->pool, curr_path));

  } else if ((fxh->fh_flags & O_WRONLY) ||
             (fxh->fh_flags & O_RDWR)) {
    cmd = fxp_cmd_alloc(fxh->pool, C_STOR, pstrdup(fxh->pool, curr_path));

  } else if (fxh->fh_flags == O_RDONLY) {
    cmd = fxp_cmd_alloc(fxh->pool, C_RETR, pstrdup(fxh->pool, curr_path));
  }

  xferlog_write(0, pr_netaddr_get_sess_remote_name(), fxh->fh_bytes_xferred,
    abs_path, 'b', direction, 'r', session.user, 'i');

  if (cmd) {
    /* Ideally we could provide a real response code/message for any
     * configured ExtendedLogs for these aborted uploads.  Something to
     * refine in the future...
     */
    pr_response_clear(&resp_list);
    pr_response_clear(&resp_err_list);

    (void) pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    (void) pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);
  }

  if (pr_fsio_close(fxh->fh) < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error writing aborted file '%s': %s", fxh->fh->fh_path, strerror(errno));
  }

  fxh->fh = NULL;

  if (fxh->fh_flags != O_RDONLY) {
    if (delete_aborted_stores != NULL &&
        *delete_aborted_stores == TRUE) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "removing aborted uploaded file '%s'", curr_path);

      if (pr_fsio_unlink(curr_path) < 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error unlinking file '%s': %s", curr_path,
          strerror(errno));
      }
    }
  }

  return 0;
}

static int fxp_handle_delete(struct fxp_handle *fxh) {
  if (fxp_session->handle_tab == NULL) {
    errno = EPERM;
    return -1;
  }

  (void) pr_table_remove(fxp_session->handle_tab, fxh->name, NULL);
  return 0;
}

static struct fxp_handle *fxp_handle_get(const char *handle) {
  struct fxp_handle *fxh;

  if (fxp_session->handle_tab == NULL) {
    errno = EPERM;
    return NULL;
  }

  fxh = pr_table_get(fxp_session->handle_tab, handle, NULL);

  return fxh;
}

/* FX Message I/O */

static struct fxp_packet *fxp_packet_create(pool *p, uint32_t channel_id) {
  pool *sub_pool;
  struct fxp_packet *fxp;

  sub_pool = pr_pool_create_sz(p, 64);
  fxp = pcalloc(sub_pool, sizeof(struct fxp_packet));
  fxp->pool = sub_pool;
  fxp->channel_id = channel_id;

  return fxp;
}

static pool *curr_buf_pool = NULL;
static char *curr_buf = NULL;
static uint32_t curr_buflen = 0, curr_bufsz = 0;
static struct fxp_packet *curr_pkt = NULL;

static struct fxp_packet *fxp_packet_get_packet(uint32_t channel_id) {
  struct fxp_packet *fxp;

  if (curr_pkt) {
    return curr_pkt;
  }

  fxp = fxp_packet_create(fxp_pool, channel_id);

  return fxp;
}

static void fxp_packet_set_packet(struct fxp_packet *pkt) {
  curr_pkt = pkt;
}

static void fxp_packet_clear_cache(void) {
  curr_buflen = 0;
}

static uint32_t fxp_packet_get_cache(char **data) {
  *data = curr_buf;
  return curr_buflen;
}

static void fxp_packet_add_cache(char *data, uint32_t datalen) {
  if (!curr_buf_pool) {
    curr_buf_pool = make_sub_pool(fxp_pool);
    pr_pool_tag(curr_buf_pool, "SFTP packet buffer pool");

    curr_buf = palloc(curr_buf_pool, FXP_PACKET_DATA_DEFAULT_SZ);
    curr_bufsz = fxp_packet_data_allocsz = FXP_PACKET_DATA_DEFAULT_SZ;
  }

  if (data == NULL ||
      datalen == 0) {
    curr_buflen = 0;
    return;
  }

  if (curr_buflen == 0) {
    if (curr_bufsz >= datalen) {
      /* We already have a buffer with enough space.  Nice. */

    } else {
      /* We need a larger buffer.  Round up to the nearest 1K size. */
      size_t sz;

      sz = sftp_crypto_get_size(datalen+1, 1024);

      if (fxp_packet_data_allocsz > FXP_PACKET_DATA_ALLOC_MAX_SZ) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "renewing SFTP packet data pool");
        destroy_pool(curr_buf_pool);

        curr_buf_pool = make_sub_pool(fxp_pool);
        pr_pool_tag(curr_buf_pool, "SFTP packet buffer pool");

        curr_bufsz = 0;
      }

      curr_bufsz = sz;
      curr_buf = palloc(curr_buf_pool, curr_bufsz);
      fxp_packet_data_allocsz += sz;
    }

    memcpy(curr_buf, data, datalen);
    curr_buflen = datalen;

    return;
  }

  if (curr_buflen > 0) {
    if (curr_bufsz >= (curr_buflen + datalen)) {
      /* We already have a buffer with enough space.  Nice. */

    } else {
      /* We need a larger buffer.  Round up to the nearest 1K size. */
      size_t sz;

      sz = sftp_crypto_get_size(curr_buflen + datalen + 1, 1024);

      if (fxp_packet_data_allocsz > FXP_PACKET_DATA_ALLOC_MAX_SZ) {
        pool *tmp_pool;
        char *tmp_data;
        uint32_t tmp_datalen;

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "renewing SFTP packet data pool");

        tmp_pool = make_sub_pool(fxp_pool);
        tmp_datalen = curr_buflen;
        tmp_data = palloc(tmp_pool, tmp_datalen);                
        memcpy(tmp_data, curr_buf, tmp_datalen);
        
        destroy_pool(curr_buf_pool);

        curr_buf_pool = make_sub_pool(fxp_pool);
        pr_pool_tag(curr_buf_pool, "SFTP packet buffer pool");

        curr_bufsz = sz;
        curr_buf = palloc(curr_buf_pool, curr_bufsz);
        fxp_packet_data_allocsz += sz;

        memcpy(curr_buf, tmp_data, tmp_datalen);
        curr_buflen = tmp_datalen;

        destroy_pool(tmp_pool);
      }
    }

    /* Append the SSH2 data to the current unconsumed buffer. */
    memcpy(curr_buf + curr_buflen, data, datalen);
    curr_buflen += datalen;
  }

  return;
}

static struct fxp_packet *fxp_packet_read(uint32_t channel_id, char **data,
    uint32_t *datalen, int *have_cache) {
  struct fxp_packet *fxp;
  char *buf;
  uint32_t buflen;

  if (datalen) {
    pr_trace_msg(trace_channel, 9,
      "reading SFTP data from SSH2 packet buffer (%lu bytes)",
      (unsigned long) *datalen);
    fxp_packet_add_cache(*data, *datalen);
  }

  buflen = fxp_packet_get_cache(&buf);

  fxp = fxp_packet_get_packet(channel_id);

  if (!(fxp->state & FXP_PACKET_HAVE_PACKET_LEN)) {
    /* Make sure we have enough data in the buffer to cover the packet len. */
    if (buflen < sizeof(uint32_t)) {
      return NULL;
    }

    fxp->packet_len = sftp_msg_read_int(fxp->pool, &buf, &buflen);
    fxp->state |= FXP_PACKET_HAVE_PACKET_LEN;

    if (buflen == 0) {
      fxp_packet_set_packet(fxp);
      fxp_packet_clear_cache();
      *have_cache = FALSE;

      return NULL;
    }
  }

  if (!(fxp->state & FXP_PACKET_HAVE_REQUEST_TYPE)) {
    /* Make sure we have enough data in the buffer to cover the request type. */
    if (buflen < sizeof(char)) {
      return NULL;
    }

    fxp->request_type = sftp_msg_read_byte(fxp->pool, &buf, &buflen);
    fxp->state |= FXP_PACKET_HAVE_REQUEST_TYPE;

    if (buflen == 0) {
      fxp_packet_set_packet(fxp);
      fxp_packet_clear_cache();
      *have_cache = FALSE;

      return NULL;
    }
  }

  if (!(fxp->state & FXP_PACKET_HAVE_PAYLOAD_SIZE)) {
    /* And take back one byte for whose request_type this is... */
    fxp->payload_sz = fxp->packet_len - 1;
    fxp->state |= FXP_PACKET_HAVE_PAYLOAD_SIZE;
  }

  if (!(fxp->state & FXP_PACKET_HAVE_REQUEST_ID)) {
    if (fxp->request_type != SFTP_SSH2_FXP_INIT) {
      /* Make sure we have enough data in the buffer to cover the request ID. */
      if (buflen < sizeof(uint32_t)) {
        return NULL;
      }

      /* The INIT and VERSION requests do not use request IDs. */
      fxp->request_id = sftp_msg_read_int(fxp->pool, &buf, &buflen);
      fxp->payload_sz -= sizeof(uint32_t);
    }

    fxp->state |= FXP_PACKET_HAVE_REQUEST_ID;

    if (buflen == 0) {
      fxp_packet_set_packet(fxp);
      fxp_packet_clear_cache();
      *have_cache = FALSE;

      return NULL;
    }
  }

  if (!(fxp->state & FXP_PACKET_HAVE_PAYLOAD)) {
    if (fxp->payload_sz == buflen) {
      fxp->payload = buf;
      fxp->payload_len = buflen;
      fxp->state |= FXP_PACKET_HAVE_PAYLOAD;

      fxp_packet_set_packet(NULL);
      fxp_packet_clear_cache();
      *have_cache = FALSE;

      return fxp;

    } else if (fxp->payload_sz > buflen) {
      if (fxp->payload == NULL) {
        /* We don't have any existing payload data; copy the entire buffer
         * into the payload buffer.
         */
        fxp->payload = pcalloc(fxp->pool, fxp->payload_sz);
        memcpy(fxp->payload, buf, buflen);
        fxp->payload_len = buflen;

        buflen = 0;
        buf = NULL;

      } else {
        uint32_t payload_len;

        /* We have existing payload data.  Now we need to see if the
         * packet buffer plus the existing payload buffer will provide
         * the complete payload data.
         */
        if ((fxp->payload_len + buflen) <= fxp->payload_sz) {
          /* The packet buffer data is less than needed, or is just
           * enough to complete the payload.
           */

          payload_len = fxp->payload_len + buflen;
          memcpy(fxp->payload + fxp->payload_len, buf, buflen);

          buflen = 0;
          buf = NULL;

        } else {
          /* The packet buffer contains more than enough data.  Copy into
           * the payload buffer the rest of what's needed.
           */
          memcpy(fxp->payload + fxp->payload_len, buf,
            (fxp->payload_sz - fxp->payload_len));

          buflen -= (fxp->payload_sz - fxp->payload_len);
          buf += (fxp->payload_sz - fxp->payload_len);

          /* Set this to the payload size, for the following checks for
           * payload completeness.
           */
          payload_len = fxp->payload_sz;
        }

        fxp->payload_len = payload_len;

        if (fxp->payload_len == fxp->payload_sz) {
          fxp->state |= FXP_PACKET_HAVE_PAYLOAD;
          fxp_packet_set_packet(NULL);
          fxp_packet_clear_cache();

          fxp_packet_add_cache(buf, buflen);
          *have_cache = buflen > 0 ? TRUE : FALSE;

          return fxp;
        }
      }

      fxp_packet_set_packet(fxp);
      fxp_packet_clear_cache();

      /* We may be adding unconsumed packet buffer data back into the
       * cache (as when the packet buffer contained more data than needed
       * to complete the payload), or the packet buffer might be empty
       * (as when the entire packet buffer was consumed in completing, or
       * attempting to complete, the payload).
       */

      fxp_packet_add_cache(buf, buflen);
      *have_cache = buflen > 0 ? TRUE : FALSE;

      pr_trace_msg(trace_channel, 15, "received %lu bytes of %lu byte payload, "
        "need more data from client", (unsigned long) fxp->payload_len,
        (unsigned long) fxp->payload_sz);

      return NULL;

    } else if (fxp->payload_sz < buflen) {
      if (fxp->payload) {
        /* Append the data to complete this SFTP packet's payload, and stash
         * the remaining data, to be used for the next packet requested.
         */
        memcpy(fxp->payload + fxp->payload_len, buf,
          fxp->payload_sz - fxp->payload_len);

      } else {
        fxp->payload = pcalloc(fxp->pool, fxp->payload_sz);
        memcpy(fxp->payload, buf, fxp->payload_sz);
      }

      buflen -= (fxp->payload_sz - fxp->payload_len);
      buf += (fxp->payload_sz - fxp->payload_len);

      fxp->payload_len = fxp->payload_sz;

      fxp->state |= FXP_PACKET_HAVE_PAYLOAD;
      fxp_packet_set_packet(NULL);

      fxp_packet_clear_cache();
      fxp_packet_add_cache(buf, buflen);
      *have_cache = buflen > 0 ? TRUE : FALSE;

      return fxp;
    }
  }

  fxp_packet_clear_cache();
  *have_cache = FALSE;
  return fxp;
}

static int fxp_packet_write(struct fxp_packet *fxp) {
  char *buf, *ptr;
  uint32_t buflen, bufsz;
  int res;

  /* Use a buffer that's a little larger than the FX packet size */
  buflen = bufsz = fxp->payload_sz + 32;
  buf = ptr = palloc(fxp->pool, bufsz);

  sftp_msg_write_data(&buf, &buflen, fxp->payload, fxp->payload_sz, TRUE);

  res = sftp_channel_write_data(fxp->pool, fxp->channel_id, ptr,
    (bufsz - buflen));
  return res;
}

/* Miscellaneous */

static void fxp_version_add_vendor_id_ext(pool *p, char **buf,
    uint32_t *buflen) {
  char *buf2, *ptr2;
  const char *vendor_name, *product_name, *product_version;
  uint32_t bufsz2, buflen2;
  uint64_t build_number;
  struct fxp_extpair ext;

  if (!(fxp_ext_flags & SFTP_FXP_EXT_VENDOR_ID)) {
    return;
  }

  bufsz2 = buflen2 = 512;
  ptr2 = buf2 = sftp_msg_getbuf(p, bufsz2);

  vendor_name = "ProFTPD Project";
  product_name = "mod_sftp";
  product_version = MOD_SFTP_VERSION;
  build_number = pr_version_get_number();

  sftp_msg_write_string(&buf2, &buflen2, vendor_name);
  sftp_msg_write_string(&buf2, &buflen2, product_name);
  sftp_msg_write_string(&buf2, &buflen2, product_version);
  fxp_msg_write_long(&buf2, &buflen2, build_number);

  ext.ext_name = "vendor-id";
  ext.ext_data = ptr2;
  ext.ext_datalen = (bufsz2 - buflen2);

  pr_trace_msg(trace_channel, 11, "+ SFTP extension: %s = "
     "{ vendorName = '%s', productName = '%s', productVersion = '%s', "
     "buildNumber = %" PR_LU " }", ext.ext_name, vendor_name, product_name,
     product_version, (pr_off_t) build_number);

  fxp_msg_write_extpair(buf, buflen, &ext);
}

static void fxp_version_add_version_ext(pool *p, char **buf, uint32_t *buflen) {
  register unsigned int i;
  struct fxp_extpair ext;
  char *versions_str = "";

  if (!(fxp_ext_flags & SFTP_FXP_EXT_VERSION_SELECT)) {
    return;
  }

  ext.ext_name = "versions";

  /* The versions we report to the client depend on the min/max client
   * versions, which may have been configured differently via SFTPClientMatch.
   */

  for (i = fxp_min_client_version; i <= fxp_max_client_version; i++) {
    switch (i) {
      case 1:
        /* Skip version 1; it is not in the list of version strings defined
         * in Section 4.6 of the SFTP Draft.
         */
        break;

      case 2:
        versions_str = pstrcat(p, versions_str, *versions_str ? "," : "",
          "2", NULL);
        break;

      case 3:
        versions_str = pstrcat(p, versions_str, *versions_str ? "," : "",
          "3", NULL);
        break;

#ifdef PR_USE_NLS
      /* We can only advertise support for these protocol versions if
       * --enable-nls has been used, as they require UTF8 support.
       */
      case 4:
        versions_str = pstrcat(p, versions_str, *versions_str ? "," : "",
          "4", NULL);
        break;

      case 5:
        versions_str = pstrcat(p, versions_str, *versions_str ? "," : "",
          "5", NULL);
        break;

      case 6:
        versions_str = pstrcat(p, versions_str, *versions_str ? "," : "",
          "6", NULL);
        break;
#endif
    }
  }

  ext.ext_data = versions_str;
  ext.ext_datalen = strlen(ext.ext_data);

  pr_trace_msg(trace_channel, 11, "+ SFTP extension: %s = '%s'", ext.ext_name,
    ext.ext_data);
  fxp_msg_write_extpair(buf, buflen, &ext);

  /* The sending of this extension is necessary in order to support any
   * 'version-select' requests from the client, as per Section 4.6 of the
   * SFTP Draft.  That is, if we don't send the 'versions' extension and the
   * client tries to send us a 'version-select', then we MUST close the
   * connection.
   */
  allow_version_select = TRUE;
}

static void fxp_version_add_openssh_exts(pool *p, char **buf,
    uint32_t *buflen) {
  (void) p;

  /* These are OpenSSH-specific SFTP extensions. */

  if (fxp_ext_flags & SFTP_FXP_EXT_POSIX_RENAME) {
    struct fxp_extpair ext;

    ext.ext_name = "posix-rename@openssh.com";
    ext.ext_data = "1";
    ext.ext_datalen = 1;

    pr_trace_msg(trace_channel, 11, "+ SFTP extension: %s = '%s'", ext.ext_name,
      ext.ext_data);
    fxp_msg_write_extpair(buf, buflen, &ext);
  }

#ifdef HAVE_SYS_STATVFS_H
  if (fxp_ext_flags & SFTP_FXP_EXT_STATVFS) {
    struct fxp_extpair ext;

    ext.ext_name = "statvfs@openssh.com";
    ext.ext_data = "2";
    ext.ext_datalen = 1;

    pr_trace_msg(trace_channel, 11, "+ SFTP extension: %s = '%s'", ext.ext_name,
      ext.ext_data);
    fxp_msg_write_extpair(buf, buflen, &ext);

    ext.ext_name = "fstatvfs@openssh.com";
    ext.ext_data = "2";
    ext.ext_datalen = 1;

    pr_trace_msg(trace_channel, 11, "+ SFTP extension: %s = '%s'",
      ext.ext_name, ext.ext_data);
    fxp_msg_write_extpair(buf, buflen, &ext);
  }
#endif
}

static void fxp_version_add_newline_ext(pool *p, char **buf, uint32_t *buflen) {
  struct fxp_extpair ext;

  (void) p;

  ext.ext_name = "newline";
  ext.ext_data = "\n";
  ext.ext_datalen = 1;

  pr_trace_msg(trace_channel, 11, "+ SFTP extension: %s = '\n'", ext.ext_name);
  fxp_msg_write_extpair(buf, buflen, &ext);
}

static void fxp_version_add_supported_ext(pool *p, char **buf,
    uint32_t *buflen) {
  struct fxp_extpair ext;
  uint32_t attrs_len, attrs_sz;
  char *attrs_buf, *attrs_ptr;
  uint32_t file_mask, bits_mask, open_mask, access_mask, max_read_size;

  ext.ext_name = "supported";

  attrs_sz = attrs_len = 1024;
  attrs_ptr = attrs_buf = sftp_msg_getbuf(p, attrs_sz);

  file_mask = SSH2_FX_ATTR_SIZE|SSH2_FX_ATTR_PERMISSIONS|
    SSH2_FX_ATTR_ACCESSTIME|SSH2_FX_ATTR_MODIFYTIME|SSH2_FX_ATTR_OWNERGROUP;

  bits_mask = 0;

  open_mask = SSH2_FXF_WANT_READ_DATA|SSH2_FXF_WANT_WRITE_DATA|
    SSH2_FXF_WANT_APPEND_DATA|SSH2_FXF_WANT_READ_ATTRIBUTES|
    SSH2_FXF_WANT_WRITE_ATTRIBUTES;

  access_mask = SSH2_FXF_CREATE_NEW|SSH2_FXF_CREATE_TRUNCATE|
    SSH2_FXF_OPEN_EXISTING|SSH2_FXF_OPEN_OR_CREATE|
    SSH2_FXF_TRUNCATE_EXISTING|SSH2_FXF_ACCESS_APPEND_DATA|
    SSH2_FXF_ACCESS_APPEND_DATA_ATOMIC;

  max_read_size = 0;

  sftp_msg_write_int(&attrs_buf, &attrs_len, file_mask);
  sftp_msg_write_int(&attrs_buf, &attrs_len, bits_mask);
  sftp_msg_write_int(&attrs_buf, &attrs_len, open_mask);
  sftp_msg_write_int(&attrs_buf, &attrs_len, access_mask);
  sftp_msg_write_int(&attrs_buf, &attrs_len, max_read_size);

  ext.ext_data = attrs_ptr;
  ext.ext_datalen = (attrs_sz - attrs_len);

  pr_trace_msg(trace_channel, 11, "+ SFTP extension: %s", ext.ext_name);
  fxp_msg_write_extpair(buf, buflen, &ext);
}

static void fxp_version_add_supported2_ext(pool *p, char **buf,
    uint32_t *buflen) {
  struct fxp_extpair ext;
  uint32_t attrs_len, attrs_sz;
  char *attrs_buf, *attrs_ptr;
  uint32_t file_mask, bits_mask, open_mask, access_mask, max_read_size;
  uint16_t open_lock_mask, lock_mask;
  int ext_count;

  ext.ext_name = "supported2";

  attrs_sz = attrs_len = 1024;
  attrs_ptr = attrs_buf = sftp_msg_getbuf(p, attrs_sz);

  file_mask = SSH2_FX_ATTR_SIZE|SSH2_FX_ATTR_PERMISSIONS|
    SSH2_FX_ATTR_ACCESSTIME|SSH2_FX_ATTR_MODIFYTIME|SSH2_FX_ATTR_OWNERGROUP;

  bits_mask = 0;

  open_mask = SSH2_FXF_WANT_READ_DATA|SSH2_FXF_WANT_WRITE_DATA|
    SSH2_FXF_WANT_APPEND_DATA|SSH2_FXF_WANT_READ_ATTRIBUTES|
    SSH2_FXF_WANT_WRITE_ATTRIBUTES;

  access_mask = SSH2_FXF_CREATE_NEW|SSH2_FXF_CREATE_TRUNCATE|
    SSH2_FXF_OPEN_EXISTING|SSH2_FXF_OPEN_OR_CREATE|
    SSH2_FXF_TRUNCATE_EXISTING|SSH2_FXF_ACCESS_APPEND_DATA|
    SSH2_FXF_ACCESS_APPEND_DATA_ATOMIC;

  max_read_size = 0;

  /* Set only one bit, to indicate that locking is not supported for
   * OPEN commands.
   */
  open_lock_mask = 0x0001;

  /* Indicate that we support the classic locks: READ+WRITE+ADVISORY and
   * WRITE+ADVISORY.  Note that we do not need to include DELETE, since this
   * mask is only for LOCK commands, not UNLOCK commands.
   */
  lock_mask = 0x0c01;

  sftp_msg_write_int(&attrs_buf, &attrs_len, file_mask);
  sftp_msg_write_int(&attrs_buf, &attrs_len, bits_mask);
  sftp_msg_write_int(&attrs_buf, &attrs_len, open_mask);
  sftp_msg_write_int(&attrs_buf, &attrs_len, access_mask);
  sftp_msg_write_int(&attrs_buf, &attrs_len, max_read_size);
  fxp_msg_write_short(&attrs_buf, &attrs_len, open_lock_mask);
  fxp_msg_write_short(&attrs_buf, &attrs_len, lock_mask);

  /* Attribute extensions */
  sftp_msg_write_int(&attrs_buf, &attrs_len, 0);

  /* The possible extensions to advertise here are:
   *
   *  check-file
   *  copy-file
   *  vendor-id
   *
   * Note that we don't have to advertise the @openssh.com extensions, since
   * they occur for protocol versions which don't support 'supported2'.  And
   * we don't have to list 'version-select', since the sending of the
   * 'versions' extension in our VERSION automatically enables use of this
   * extension by the client.
   */
  ext_count = 3;

  if (!(fxp_ext_flags & SFTP_FXP_EXT_CHECK_FILE)) {
    ext_count--;
  }

  if (!(fxp_ext_flags & SFTP_FXP_EXT_COPY_FILE)) {
    ext_count--;
  }

  /* Additional protocol extensions (why these appear in 'supported2' is
   * confusing to me, too).
   */
  sftp_msg_write_int(&attrs_buf, &attrs_len, ext_count);

  if (fxp_ext_flags & SFTP_FXP_EXT_CHECK_FILE) {
    pr_trace_msg(trace_channel, 11, "%s", "+ SFTP extension: check-file");
    sftp_msg_write_string(&attrs_buf, &attrs_len, "check-file");
  }

  if (fxp_ext_flags & SFTP_FXP_EXT_COPY_FILE) {
    pr_trace_msg(trace_channel, 11, "%s", "+ SFTP extension: copy-file");
    sftp_msg_write_string(&attrs_buf, &attrs_len, "copy-file");
  }

  pr_trace_msg(trace_channel, 11, "%s", "+ SFTP extension: vendor-id");
  sftp_msg_write_string(&attrs_buf, &attrs_len, "vendor-id");
 
  ext.ext_data = attrs_ptr;
  ext.ext_datalen = (attrs_sz - attrs_len);

  pr_trace_msg(trace_channel, 11, "+ SFTP extension: %s", ext.ext_name);
  fxp_msg_write_extpair(buf, buflen, &ext);
}

/* SFTP Extension handlers */

static int fxp_handle_ext_check_file(struct fxp_packet *fxp, char *digest_list,
    char *path, off_t offset, off_t len, uint32_t blocksz) {
  char *buf, *ptr, *supported_digests;
  const char *digest_name, *reason;
  uint32_t buflen, bufsz, status_code;
  struct fxp_packet *resp;
  int data_len, res, xerrno = 0;
  struct stat st;
  pr_fh_t *fh;
  cmd_rec *cmd;
  unsigned int nblocks;
  off_t range_len, total_len = 0;
  void *data;
  BIO *bio, *fd_bio, *md_bio;
  const EVP_MD *md;

  pr_trace_msg(trace_channel, 8, "client sent check-file request: "
    "path = '%s', digests = '%s', offset = %" PR_LU ", len = %" PR_LU
    ", block size = %lu", path, digest_list, (pr_off_t) offset, (pr_off_t) len,
    (unsigned long) blocksz);

  /* We could end up with lots of digests to write, if the file is large
   * and/or the block size is small.  Be prepared.
   */
  buflen = bufsz = (FXP_RESPONSE_DATA_DEFAULT_SZ * 2);
  buf = ptr = palloc(fxp->pool, bufsz);

  /* The minimum block size required by this extension is 256 bytes. */
  if (blocksz != 0 &&
      blocksz < 256) {
    xerrno = EINVAL;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SFTP client check-file request sent invalid block size "
      "(%lu bytes <= 256)", (unsigned long) blocksz);

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  res = pr_fsio_lstat(path, &st);
  if (res < 0) {
    xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to lstat path '%s': %s", path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason, strerror(xerrno),
      xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (S_ISDIR(st.st_mode)) {
    xerrno = EISDIR;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SFTP client check-file requested on a directory, denying");

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason, strerror(xerrno),
      xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (!S_ISREG(st.st_mode) &&
      !S_ISLNK(st.st_mode)) {
    xerrno = EINVAL;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SFTP client check-file request not for file or symlink, denying");

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (offset >= st.st_size) {
    xerrno = EINVAL;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client check-file request sent invalid offset (%" PR_LU
      " >= %" PR_LU " file size)", (pr_off_t) offset, (pr_off_t) st.st_size);

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  cmd = pr_cmd_alloc(fxp->pool, 1, "SITE_DIGEST");
  cmd->arg = pstrdup(fxp->pool, path);
  if (!dir_check(fxp->pool, cmd, "READ", path, NULL)) {
    xerrno = EACCES;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "'check-file' of '%s' blocked by <Limit> configuration", path);

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason, strerror(xerrno),
      xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  supported_digests = "md5,sha1";
#ifdef HAVE_SHA256_OPENSSL
  supported_digests = pstrcat(fxp->pool, supported_digests, ",sha224,sha256",
    NULL);
#endif
#ifdef HAVE_SHA512_OPENSSL
  supported_digests = pstrcat(fxp->pool, supported_digests, ",sha384,sha512",
    NULL);
#endif

  digest_name = fxp_get_shared_name(fxp->pool, digest_list, supported_digests);
  if (digest_name == NULL) {
    xerrno = EINVAL;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no supported digests in client check-file request "
      "(client sent '%s', server supports '%s')", digest_list,
      supported_digests);

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (len == 0) {
    range_len = st.st_size - offset;

  } else {
    range_len = offset + len;
  }

  if (blocksz == 0) {
    nblocks = 1;

  } else {
    nblocks = range_len / blocksz;
    if (range_len % blocksz != 0) {
      nblocks++;
    }
  }

  pr_trace_msg(trace_channel, 15, "for check-file request on '%s', "
    "calculate %s digest of %u %s", path, digest_name, nblocks,
    nblocks == 1 ? "block/checksum" : "nblocks/checksums");

  fh = pr_fsio_open(path, O_RDONLY|O_NONBLOCK);
  if (fh == NULL) {
    xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to open path '%s': %s", path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason, strerror(xerrno),
      xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_fsio_set_block(fh);

  if (pr_fsio_lseek(fh, offset, SEEK_SET) < 0) {
    xerrno = errno;

    pr_fsio_close(fh);

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to seek to offset %" PR_LU " in '%s': %s", (pr_off_t) offset,
      path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason, strerror(xerrno),
      xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  md = EVP_get_digestbyname(digest_name);
  if (md == NULL) {
    xerrno = EINVAL;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to support %s digests: %s", digest_name,
      sftp_crypto_get_errors());

    pr_fsio_close(fh);

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);

    /* Since we already started writing the EXTENDED_REPLY, we have
     * to reset the pointers and overwrite the existing message.
     */
    buf = ptr;
    buflen = bufsz;

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  md_bio = BIO_new(BIO_f_md());

  /* XXX Check the return value here */
  BIO_set_md(md_bio, md);

  fd_bio = BIO_new(BIO_s_fd());
  BIO_set_fd(fd_bio, PR_FH_FD(fh), BIO_NOCLOSE);

  bio = BIO_push(md_bio, fd_bio);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_EXTENDED_REPLY);
  sftp_msg_write_int(&buf, &buflen, fxp->request_id);
  sftp_msg_write_string(&buf, &buflen, digest_name);

  pr_trace_msg(trace_channel, 8,
    "sending response: EXTENDED_REPLY %s digest of %u %s", digest_name,
    nblocks, nblocks == 1 ? "block" : "blocks");

  if (blocksz == 0) {
    data_len = st.st_blksize;

  } else {
    data_len = blocksz;
  }

  data = palloc(fxp->pool, data_len);

  while (TRUE) {
    pr_signals_handle();

    res = BIO_read(bio, data, data_len);
    if (res < 0) {
      if (BIO_should_read(fd_bio)) {
        continue;
      }

      /* error */
      xerrno = errno;

      pr_fsio_close(fh);

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading from '%s': %s", path, strerror(xerrno));

      status_code = fxp_errno2status(xerrno, &reason);

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
        "('%s' [%d])", (unsigned long) status_code, reason,
        strerror(xerrno), xerrno);

      /* Since we already started writing the EXTENDED_REPLY, we have
       * to reset the pointers and overwrite the existing message.
       */
      buf = ptr;
      buflen = bufsz;

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
        reason, NULL);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);

      return fxp_packet_write(resp);

    } else if (res == 0) {
      if (BIO_should_retry(fd_bio) != 0) {
        continue;
      }

      /* EOF */
      break;
    }

    if (blocksz != 0) {
      char digest[EVP_MAX_MD_SIZE];
      unsigned int digest_len;

      (void) BIO_flush(bio);
      digest_len = BIO_gets(md_bio, digest, sizeof(digest));

      sftp_msg_write_data(&buf, &buflen, digest, digest_len, FALSE);

      (void) BIO_reset(md_bio);

      total_len += res; 
      if (len > 0 &&
          total_len >= len) {
        break;
      }
    }
  }

  if (blocksz == 0) {
    char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    (void) BIO_flush(bio);
    digest_len = BIO_gets(md_bio, digest, sizeof(digest));

    sftp_msg_write_data(&buf, &buflen, digest, digest_len, FALSE);
  }

  BIO_free_all(bio);
  pr_fsio_close(fh);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_ext_copy_file(struct fxp_packet *fxp, char *src,
    char *dst, int overwrite) {
  char *abs_path, *buf, *ptr, *args;
  const char *reason;
  uint32_t buflen, bufsz, status_code;
  struct fxp_packet *resp;
  cmd_rec *cmd, *cmd2;
  int res, xerrno;
  struct stat st;

  args = pstrcat(fxp->pool, src, " ", dst, NULL);

  /* We need to provide an actual argv in this COPY cmd_rec, so we can't
   * use fxp_cmd_alloc(); we have to allocate the cmd_rec ourselves.
   */
  cmd = pr_cmd_alloc(fxp->pool, 3, pstrdup(fxp->pool, "COPY"), src, dst);
  cmd->arg = args;
  cmd->tmp_pool = pr_pool_create_sz(fxp->pool, 64);
  cmd->class = CL_WRITE;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "COPY of '%s' to '%s' blocked by '%s' handler", src, dst, cmd->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (strcmp(src, dst) == 0) {
    xerrno = EEXIST;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "COPY of '%s' to same path '%s', rejecting", src, dst);

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  res = pr_fsio_stat(dst, &st);
  if (res == 0) {
    unsigned char *allow_overwrite = NULL;
    int limit_allow;

    allow_overwrite = get_param_ptr(get_dir_ctxt(fxp->pool, dst),
      "AllowOverwrite", FALSE);

    cmd2 = pr_cmd_alloc(fxp->pool, 3, "SITE_COPY", src, dst);
    cmd2->arg = pstrdup(fxp->pool, args);
    limit_allow = dir_check(fxp->pool, cmd2, "WRITE", dst, NULL);

    if (!overwrite ||
        (allow_overwrite == NULL ||
         *allow_overwrite == FALSE) ||
        !limit_allow) {
      xerrno = EACCES;

      if (!overwrite) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "'%s' exists and client did not request COPY overwrites", dst);

      } else if (!limit_allow) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "COPY to '%s' blocked by <Limit> configuration", dst);

      } else {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "AllowOverwrite permission denied for '%s'", dst);
      }

      status_code = fxp_errno2status(xerrno, &reason);

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
        (unsigned long) status_code, reason);

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
        NULL);

      pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);

      return fxp_packet_write(resp);
    }
  }

  if (fxp_path_pass_regex_filters(fxp->pool, "COPY", src) < 0 ||
      fxp_path_pass_regex_filters(fxp->pool, "COPY", dst) < 0) {
    xerrno = errno;

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  cmd2 = pr_cmd_alloc(fxp->pool, 3, "SITE_COPY", src, dst);
  cmd2->arg = pstrdup(fxp->pool, args);
  if (!dir_check(fxp->pool, cmd2, "READ", src, NULL)) {
    xerrno = EACCES;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "COPY of '%s' blocked by <Limit> configuration", src);

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason, strerror(xerrno),
      xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  res = pr_fs_copy_file(src, dst);
  if (res < 0) {
    xerrno = errno;

    status_code = fxp_errno2status(xerrno, &reason);

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error copying '%s' to '%s': %s", src, dst, strerror(xerrno));

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  /* No errors. */
  xerrno = errno = 0;

  pr_fs_clear_cache();
  pr_fsio_stat(dst, &st);

  pr_cmd_dispatch_phase(cmd, POST_CMD, 0);
  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  /* Write a TransferLog entry as well. */
  abs_path = dir_abs_path(fxp->pool, dst, TRUE);
  xferlog_write(0, session.c->remote_name, st.st_size, abs_path, 'b', 'i',
    'r', session.user, 'c');

  status_code = fxp_errno2status(xerrno, &reason);

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
    (unsigned long) status_code, reason);

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_ext_posix_rename(struct fxp_packet *fxp, char *src,
    char *dst) {
  char *buf, *ptr, *args;
  const char *reason;
  uint32_t buflen, bufsz, status_code;
  struct fxp_packet *resp;
  cmd_rec *cmd, *cmd2, *cmd3;
  int res, xerrno;

  args = pstrcat(fxp->pool, src, " ", dst, NULL);

  cmd = fxp_cmd_alloc(fxp->pool, "RENAME", args);
  cmd->class = CL_MISC;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  cmd2 = fxp_cmd_alloc(fxp->pool, C_RNTO, dst);
  if (pr_cmd_dispatch_phase(cmd2, PRE_CMD, 0) < 0) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RENAME to '%s' blocked by '%s' handler", dst, cmd2->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  dst = cmd2->arg;

  cmd3 = fxp_cmd_alloc(fxp->pool, C_RNFR, src);
  if (pr_cmd_dispatch_phase(cmd3, PRE_CMD, 0) < 0) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RENAME from '%s' blocked by '%s' handler", src, cmd3->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd3, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  src = cmd3->arg;

  if (!dir_check(fxp->pool, cmd3, G_DIRS, src, NULL) ||
      !dir_check(fxp->pool, cmd2, G_WRITE, dst, NULL)) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RENAME of '%s' to '%s' blocked by <Limit> configuration",
      src, dst);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd3, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (strcmp(src, dst) == 0) {
    xerrno = EEXIST;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RENAME of '%s' to same path '%s', rejecting", src, dst);

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd3, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxp_path_pass_regex_filters(fxp->pool, "RENAME", src) < 0 ||
      fxp_path_pass_regex_filters(fxp->pool, "RENAME", dst) < 0) {
    status_code = fxp_errno2status(errno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd3, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  res = pr_fsio_rename(src, dst);
  if (res < 0) {
    if (errno != EXDEV) {
      xerrno = errno;

      (void) pr_trace_msg("fileperms", 1, "RENAME, user '%s' (UID %lu, "
        "GID %lu): error renaming '%s' to '%s': %s", session.user,
        (unsigned long) session.uid, (unsigned long) session.gid,
        src, dst, strerror(xerrno));

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error renaming '%s' to '%s': %s", src, dst, strerror(xerrno));

      errno = xerrno;

    } else {
      /* In this case, we should manually copy the file from the source
       * path to the destination path.
       */
      errno = 0;

      res = pr_fs_copy_file(src, dst);
      if (res < 0) {
        xerrno = errno;

        (void) pr_trace_msg("fileperms", 1, "RENAME, user '%s' (UID %lu, "
          "GID %lu): error copying '%s' to '%s': %s", session.user,
          (unsigned long) session.uid, (unsigned long) session.gid,
          src, dst, strerror(xerrno));

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error copying '%s' to '%s': %s", src, dst, strerror(xerrno));

        errno = xerrno;

      } else {
        /* Once copied, remove the original path. */
        if (pr_fsio_unlink(src) < 0) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error deleting '%s': %s", src, strerror(errno));
        }

        xerrno = errno = 0;
      }
    }

  } else {
    /* No errors. */
    xerrno = errno = 0;
  }

  status_code = fxp_errno2status(xerrno, &reason);

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
    "('%s' [%d])", (unsigned long) status_code, reason,
    xerrno != EOF ? strerror(errno) : "End of file", xerrno);

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  pr_cmd_dispatch_phase(cmd3, xerrno == 0 ? POST_CMD : POST_CMD_ERR, 0);
  pr_cmd_dispatch_phase(cmd2, xerrno == 0 ? POST_CMD : POST_CMD_ERR, 0);
  pr_cmd_dispatch_phase(cmd, xerrno == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

#ifdef HAVE_SYS_STATVFS_H
static int fxp_handle_ext_statvfs(struct fxp_packet *fxp, const char *path) {
  char *buf, *ptr;
  const char *reason;
  uint32_t buflen, bufsz, status_code;
  struct fxp_packet *resp;
  uint64_t fs_id = 0, fs_flags = 0;

# if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS == 64 && \
   defined(SOLARIS2) && !defined(SOLARIS2_5_1) && !defined(SOLARIS2_6) && \
   !defined(SOLARIS2_7)
  /* Note: somewhere along the way, Sun decided that the prototype for
   * its statvfs64(2) function would include a statvfs64_t rather than
   * struct statvfs64.  In 2.6 and 2.7, it's struct statvfs64, and
   * in 8, 9 it's statvfs64_t.  This should silence compiler warnings.
   * (The statvfs_t will be redefined to a statvfs64_t as appropriate on
   * LFS systems).
   */
  statvfs_t fs;
#  else
  struct statvfs fs;
# endif /* LFS && !Solaris 2.5.1 && !Solaris 2.6 && !Solaris 2.7 */

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (statvfs(path, &fs) < 0) {
    int xerrno = errno;

    pr_trace_msg(trace_channel, 3, "statvfs() error using '%s': %s",
      path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_trace_msg(trace_channel, 8,
    "sending response: EXTENDED_REPLY <statvfs data of '%s'>", path);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_EXTENDED_REPLY);
  sftp_msg_write_int(&buf, &buflen, fxp->request_id);
  fxp_msg_write_long(&buf, &buflen, fs.f_bsize);
  fxp_msg_write_long(&buf, &buflen, fs.f_frsize);
  fxp_msg_write_long(&buf, &buflen, fs.f_blocks);
  fxp_msg_write_long(&buf, &buflen, fs.f_bfree);
  fxp_msg_write_long(&buf, &buflen, fs.f_bavail);
  fxp_msg_write_long(&buf, &buflen, fs.f_files);
  fxp_msg_write_long(&buf, &buflen, fs.f_ffree);
  fxp_msg_write_long(&buf, &buflen, fs.f_favail);

  /* AIX requires this machination because a) its statvfs struct has
   * non-standard data types for the fsid value:
   *
   *  https://lists.dulug.duke.edu/pipermail/rpm-devel/2006-July/001236.html
   *  https://lists.dulug.duke.edu/pipermail/rpm-devel/2006-July/001264.html
   *  https://lists.dulug.duke.edu/pipermail/rpm-devel/2006-July/001265.html
   *  https://lists.dulug.duke.edu/pipermail/rpm-devel/2006-July/001268.html
   *
   * and b) it does not really matter what value is written; the client is
   * not going to be able to do much with this value anyway.  From that
   * perspective, I'm not sure why the OpenSSH extension even includes the
   * value in the response (*shrug*).
   */
#if !defined(AIX4) && !defined(AIX5)
  memcpy(&fs_id, &(fs.f_fsid), sizeof(fs_id));
#endif
  fxp_msg_write_long(&buf, &buflen, fs_id);

  /* These flags and values are defined by OpenSSH's PROTOCOL document.
   *
   * Other platforms support more fs.f_flag values than just ST_RDONLY
   * and ST_NOSUID, but those are the only two flags handled by OpenSSH;
   * thus we cannot simply send fs.f_flag directly to the client as is.
   */
#ifdef ST_RDONLY
  if (fs.f_flag & ST_RDONLY) {
    fs_flags |= SSH2_FXE_STATVFS_ST_RDONLY;
  }
#endif

#ifdef ST_NOSUID
  if (fs.f_flag & ST_NOSUID) {
    fs_flags |= SSH2_FXE_STATVFS_ST_NOSUID;
  }
#endif

  fxp_msg_write_long(&buf, &buflen, fs_flags);
  fxp_msg_write_long(&buf, &buflen, fs.f_namemax);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}
#endif /* !HAVE_SYS_STATVFS_H */

static int fxp_handle_ext_vendor_id(struct fxp_packet *fxp) {
  char *buf, *ptr, *vendor_name, *product_name, *product_version;
  uint32_t buflen, bufsz, status_code;
  uint64_t build_number;
  const char *reason;
  struct fxp_packet *resp;

  vendor_name = sftp_msg_read_string(fxp->pool, &fxp->payload,
    &fxp->payload_sz);

  product_name = sftp_msg_read_string(fxp->pool, &fxp->payload,
    &fxp->payload_sz);

  product_version = sftp_msg_read_string(fxp->pool, &fxp->payload,
    &fxp->payload_sz);

  build_number = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);

  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    vendor_name = sftp_utf8_decode_str(fxp->pool, vendor_name);
    product_name = sftp_utf8_decode_str(fxp->pool, product_name);
    product_version = sftp_utf8_decode_str(fxp->pool, product_version);
  }

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "client sent 'vendor-id' extension: { vendorName = '%s', "
    "productName = '%s', productVersion = '%s', buildNumber = %" PR_LU " }",
    vendor_name, product_name, product_version, (pr_off_t) build_number);

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  status_code = SSH2_FX_OK;
  reason = "OK";

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
    (unsigned long) status_code, reason);

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_ext_version_select(struct fxp_packet *fxp,
    char *version_str) {
  char *buf, *ptr;
  uint32_t buflen, bufsz, status_code;
  const char *reason;
  struct fxp_packet *resp;
  int res = 0, version = 0;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (!allow_version_select) {
    int xerrno = EACCES;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client sent 'version-select' request at inappropriate time, rejecting");

    status_code = SSH2_FX_FAILURE;
    reason = "Failure";

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason, strerror(xerrno),
      xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    (void) fxp_packet_write(resp);

    errno = EINVAL;
    return -1;
  }

  version = atoi(version_str);

  if (version > fxp_max_client_version) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested SFTP protocol version %d via 'version-select', "
      "which exceeds SFTPClientMatch max SFTP protocol version %u, rejecting",
      version, fxp_max_client_version);
    res = -1;
  }

  if (version < fxp_min_client_version) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested SFTP protocol version %d via 'version-select', "
      "which is less than SFTPClientMatch min SFTP protocol version %u, "
      "rejecting", version, fxp_min_client_version);
    res = -1;
  }

#ifndef PR_USE_NLS
  /* If NLS supported was enabled in the proftpd build, then we can support
   * UTF8, and thus every other version of SFTP.  Otherwise, we can only
   * support up to version 3.
   */
  if (version > 3) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested SFTP protocol version %d via 'version-select', "
      "but we can only support protocol version 3 due to lack of "
      "UTF8 support (requires --enable-nls)", version);
    res = -1;
  }
#endif

  if (res < 0) {
    int xerrno = EINVAL;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client sent 'version-select' request at inappropriate time, rejecting");

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason, strerror(xerrno),
      xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    (void) fxp_packet_write(resp);

    errno = EINVAL;
    return -1;
  }

  pr_trace_msg(trace_channel, 7, "client requested switch to SFTP protocol "
    "version %d via 'version-select'", version);
  fxp_session->client_version = (unsigned long) version;

  status_code = SSH2_FX_OK;
  reason = "OK";

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
    (unsigned long) status_code, reason);

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  allow_version_select = FALSE;
  return fxp_packet_write(resp);
}

/* Request handlers */

static int fxp_handle_close(struct fxp_packet *fxp) {
  int xerrno = 0, res = 0;
  char *buf, *ptr, *name;
  const char *reason;
  uint32_t buflen, bufsz, status_code;
  struct fxp_handle *fxh;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  name = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);

  cmd = fxp_cmd_alloc(fxp->pool, "CLOSE", name);
  cmd->class = CL_READ|CL_WRITE;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "CLOSE", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", name, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: CLOSE %s", name);

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  fxh = fxp_handle_get(name);
  if (fxh == NULL) {
    status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxh->dirh == NULL &&
      fxh->fh == NULL) {
    status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);
 
    fxp_handle_delete(fxh);
    destroy_pool(fxh->pool);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  pr_timer_remove(PR_TIMER_STALLED, ANY_MODULE);

  if (fxh->fh != NULL) {
    char *curr_path = NULL, *real_path = NULL;
    cmd_rec *cmd2 = NULL;

    curr_path = pstrdup(fxp->pool, fxh->fh->fh_path);
    real_path = curr_path;
    if (fxh->fh_real_path) {
      real_path = fxh->fh_real_path;
    }
 
    res = pr_fsio_close(fxh->fh);
    if (res < 0) 
      xerrno = errno;

    pr_scoreboard_entry_update(session.pid,
      PR_SCORE_CMD_ARG, "%s", real_path, NULL, NULL);

    if (fxh->fh_real_path != NULL &&
        res == 0) {
      /* This is a HiddenStores file, and needs to be renamed to the real
       * path.
       */

      pr_trace_msg(trace_channel, 8, "renaming HiddenStores path '%s' to '%s'",
        curr_path, real_path);

      res = pr_fsio_rename(curr_path, real_path);
      if (res < 0) {
        xerrno = errno;

        pr_log_pri(PR_LOG_WARNING, "Rename of %s to %s failed: %s",
          curr_path, real_path, strerror(xerrno));

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "renaming of HiddenStore path '%s' to '%s' failed: %s",
          curr_path, real_path, strerror(xerrno));

        pr_fsio_unlink(curr_path);
      }
    }

    if (fxh->fh_flags & O_APPEND) {
      cmd2 = fxp_cmd_alloc(fxp->pool, C_APPE, pstrdup(fxp->pool, real_path));

    } else if ((fxh->fh_flags & O_WRONLY) ||
               (fxh->fh_flags & O_RDWR)) {
      cmd2 = fxp_cmd_alloc(fxp->pool, C_STOR, pstrdup(fxp->pool, real_path));

    } else if (fxh->fh_flags == O_RDONLY) {
      cmd2 = fxp_cmd_alloc(fxp->pool, C_RETR, pstrdup(fxp->pool, real_path));
    }

    fxh->fh = NULL;

    if (cmd2) {
      int post_phase = POST_CMD, log_phase = LOG_CMD;

      if (res < 0 &&
          xerrno != EOF) {
        post_phase = POST_CMD_ERR;
        log_phase = LOG_CMD_ERR;
      }

      /* XXX We don't really care about the success of this dispatch, since
       * there's not much that we can do, in this code, at this point.
       */
      (void) pr_cmd_dispatch_phase(cmd2, post_phase, 0);
      (void) pr_cmd_dispatch_phase(cmd2, log_phase, 0);
    }

  } else if (fxh->dirh != NULL) {
    pr_scoreboard_entry_update(session.pid,
      PR_SCORE_CMD_ARG, "%s", fxh->dir, NULL, NULL);

    res = pr_fsio_closedir(fxh->dirh);
    if (res < 0)
      xerrno = errno;
    fxh->dirh = NULL;
  }

  /* Clear out any transfer-specific data. */
  if (session.xfer.p) {
    destroy_pool(session.xfer.p);
  }
  memset(&session.xfer, 0, sizeof(session.xfer));

  if (res < 0) {
    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

  } else {
    errno = 0;
    status_code = fxp_errno2status(0, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);
  }

  fxp_handle_delete(fxh);
  destroy_pool(fxh->pool);

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  pr_cmd_dispatch_phase(cmd, res < 0 ? LOG_CMD_ERR : LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);
  
  return fxp_packet_write(resp);
}

static int fxp_handle_extended(struct fxp_packet *fxp) {
  int res;
  char *buf, *ptr, *ext_request_name;
  uint32_t buflen, bufsz, status_code;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  ext_request_name = sftp_msg_read_string(fxp->pool, &fxp->payload,
    &fxp->payload_sz);

  cmd = fxp_cmd_alloc(fxp->pool, "EXTENDED", ext_request_name);
  cmd->class = CL_MISC;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "EXTENDED", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", ext_request_name, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: EXTENDED %s",
    ext_request_name);

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (strcmp(ext_request_name, "vendor-id") == 0) {
    res = fxp_handle_ext_vendor_id(fxp);
    pr_cmd_dispatch_phase(cmd, res == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

    return res;
  }

  if ((fxp_ext_flags & SFTP_FXP_EXT_VERSION_SELECT) &&
      strcmp(ext_request_name, "version-select") == 0) {
    char *version_str;

    version_str = sftp_msg_read_string(fxp->pool, &fxp->payload,
      &fxp->payload_sz);

    res = fxp_handle_ext_version_select(fxp, version_str);
    pr_cmd_dispatch_phase(cmd, res == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

    return res;
  }

  if ((fxp_ext_flags & SFTP_FXP_EXT_CHECK_FILE) &&
      strcmp(ext_request_name, "check-file-name") == 0) {
    char *path, *digest_list;
    off_t offset, len;
    uint32_t blocksz;

    path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
    digest_list = sftp_msg_read_string(fxp->pool, &fxp->payload,
      &fxp->payload_sz);
    offset = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);
    len = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);
    blocksz = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);

    res = fxp_handle_ext_check_file(fxp, digest_list, path, offset, len,
      blocksz);
    pr_cmd_dispatch_phase(cmd, res == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

    return res;
  }

  if ((fxp_ext_flags & SFTP_FXP_EXT_CHECK_FILE) &&
      strcmp(ext_request_name, "check-file-handle") == 0) {
    char *handle, *path, *digest_list;
    off_t offset, len;
    uint32_t blocksz;
    struct fxp_handle *fxh;

    handle = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);

    fxh = fxp_handle_get(handle);
    if (fxh == NULL ||
        fxh->dirh != NULL) {
      status_code = SSH2_FX_INVALID_HANDLE;

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
        (unsigned long) status_code, fxp_strerror(status_code));

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
        fxp_strerror(status_code), NULL);

      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);

      return fxp_packet_write(resp);
    }

    /* Make sure the file was opened with read permissions; if it was opened
     * write-only, for example, we need to return EACCES.
     */
    if (fxh->fh_flags & O_WRONLY) {
      status_code = SSH2_FX_PERMISSION_DENIED;
   
      pr_trace_msg(trace_channel, 9, "file %s opened write-only, "
        "unable to obtain file checksum (%s)", fxh->fh->fh_path,
        strerror(EACCES));
 
      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
        (unsigned long) status_code, fxp_strerror(status_code));

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
        fxp_strerror(status_code), NULL);

      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);

      return fxp_packet_write(resp);
    }

    path = fxh->fh->fh_path;

    digest_list = sftp_msg_read_string(fxp->pool, &fxp->payload,
      &fxp->payload_sz);
    offset = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);
    len = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);
    blocksz = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);

    res = fxp_handle_ext_check_file(fxp, digest_list, path, offset, len,
      blocksz);
    pr_cmd_dispatch_phase(cmd, res == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

    return res;
  }

  if ((fxp_ext_flags & SFTP_FXP_EXT_COPY_FILE) &&
      strcmp(ext_request_name, "copy-file") == 0) {
    char *src, *dst;
    int overwrite;

    src = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
    dst = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
    overwrite = sftp_msg_read_bool(fxp->pool, &fxp->payload, &fxp->payload_sz);

    res = fxp_handle_ext_copy_file(fxp, src, dst, overwrite);
    pr_cmd_dispatch_phase(cmd, res == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

    return res;
  }

  if ((fxp_ext_flags & SFTP_FXP_EXT_POSIX_RENAME) &&
      strcmp(ext_request_name, "posix-rename@openssh.com") == 0) {
    char *src, *dst;

    src = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
    dst = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);

    if (fxp_session->client_version >= fxp_utf8_protocol_version) {
      src = sftp_utf8_decode_str(fxp->pool, src);
      dst = sftp_utf8_decode_str(fxp->pool, dst);
    }

    res = fxp_handle_ext_posix_rename(fxp, src, dst);
    pr_cmd_dispatch_phase(cmd, res == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

    return res;
  }

#ifdef HAVE_SYS_STATVFS_H
  if ((fxp_ext_flags & SFTP_FXP_EXT_STATVFS) &&
      strcmp(ext_request_name, "statvfs@openssh.com") == 0) {
    const char *path;

    path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);

    res = fxp_handle_ext_statvfs(fxp, path);
    pr_cmd_dispatch_phase(cmd, res == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

    return res;
  }

  if ((fxp_ext_flags & SFTP_FXP_EXT_STATVFS) &&
      strcmp(ext_request_name, "fstatvfs@openssh.com") == 0) {
    const char *handle, *path;
    struct fxp_handle *fxh;

    handle = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);

    fxh = fxp_handle_get(handle);
    if (fxh == NULL) {
      status_code = SSH2_FX_INVALID_HANDLE;

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
        (unsigned long) status_code, fxp_strerror(status_code));

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
        fxp_strerror(status_code), NULL);

      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);

      return fxp_packet_write(resp);
    }

    path = fxh->fh ? fxh->fh->fh_path : fxh->dir;

    res = fxp_handle_ext_statvfs(fxp, path);
    pr_cmd_dispatch_phase(cmd, res == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

    return res;
  }
#endif

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "client requested '%s' extension, rejecting", ext_request_name);
  status_code = SSH2_FX_OP_UNSUPPORTED;

  pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
    (unsigned long) status_code, fxp_strerror(status_code));

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
    fxp_strerror(status_code), NULL);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);
  
  return fxp_packet_write(resp);
}

static int fxp_handle_fsetstat(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *name;
  const char *reason;
  uint32_t attr_flags, buflen, bufsz, status_code;
  int res;
  struct stat *attrs;
  struct fxp_handle *fxh;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  name = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);

  cmd = fxp_cmd_alloc(fxp->pool, "FSETSTAT", name);
  cmd->class = CL_WRITE;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "FSETSTAT", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", name, NULL, NULL);

  attrs = fxp_attrs_read(fxp, &fxp->payload, &fxp->payload_sz, &attr_flags);
  if (attrs == NULL) {
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);
    return 0;
  }

  pr_trace_msg(trace_channel, 7, "received request: FSETSTAT %s %s", name,
    fxp_strattrs(fxp->pool, attrs, &attr_flags));

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  fxh = fxp_handle_get(name);
  if (fxh == NULL) {
    status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  cmd_name = cmd->argv[0];
  cmd->argv[0] = "SETSTAT";

  if (!dir_check(fxp->pool, cmd, G_WRITE,
        (char *) (fxh->fh ? fxh->fh->fh_path : fxh->dir), NULL)) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    cmd->argv[0] = cmd_name;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "FSETSTAT of '%s' blocked by <Limit> configuration",
      fxh->fh ? fxh->fh->fh_path : fxh->dir);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  cmd->argv[0] = cmd_name;

  if (fxh->fh != NULL) {
    res = fxp_attrs_set(fxh->fh, fxh->fh->fh_path, attrs, attr_flags, &buf,
      &buflen, fxp);

  } else {
    res = fxp_attrs_set(NULL, fxh->dir, attrs, attr_flags, &buf, &buflen, fxp);
  }

  if (res < 0) {
    int xerrno = errno;

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  status_code = fxp_errno2status(0, &reason);

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
    (unsigned long) status_code, reason);

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_fstat(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *name;
  uint32_t buflen, bufsz;
  struct stat st;
  struct fxp_handle *fxh;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  name = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);

  cmd = fxp_cmd_alloc(fxp->pool, "FSTAT", name);
  cmd->class = CL_READ;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "FSTAT", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", name, NULL, NULL);

  if (fxp_session->client_version > 3) {
    uint32_t attr_flags;

    /* These are hints from the client about what file attributes are
     * of particular interest.  We do not currently honor them.
     */
    attr_flags = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);

    pr_trace_msg(trace_channel, 7, "received request: FSTAT %s %s", name,
      fxp_strattrflags(fxp->pool, attr_flags));

  } else {
    pr_trace_msg(trace_channel, 7, "received request: FSTAT %s", name);
  }

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  fxh = fxp_handle_get(name);
  if (fxh == NULL) {
    uint32_t status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxh->fh == NULL) {
    uint32_t status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  cmd_name = cmd->argv[0];
  cmd->argv[0] = "FSTAT";

  if (!dir_check(fxp->pool, cmd, G_NONE, fxh->fh->fh_path, NULL)) {
    uint32_t status_code = SSH2_FX_PERMISSION_DENIED;

    cmd->argv[0] = cmd_name;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "FSTAT of '%s' blocked by <Limit> configuration", fxh->fh->fh_path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  cmd->argv[0] = cmd_name;

  if (pr_fsio_fstat(fxh->fh, &st) < 0) {
    uint32_t status_code;
    const char *reason;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error checking '%s' for FSTAT: %s", fxh->fh->fh_path, strerror(errno));

    status_code = fxp_errno2status(errno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      errno != EOF ? strerror(errno) : "End of file", errno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  pr_trace_msg(trace_channel, 8, "sending response: ATTRS %s",
    fxp_strattrs(fxp->pool, &st, NULL));

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_ATTRS);
  sftp_msg_write_int(&buf, &buflen, fxp->request_id);
  fxp_attrs_write(fxp->pool, &buf, &buflen, &st);

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);
  
  return fxp_packet_write(resp);
}

static int fxp_handle_init(struct fxp_packet *fxp) {
  char version_str[16];
  char *buf, *ptr;
  uint32_t buflen, bufsz;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  fxp_session->client_version = sftp_msg_read_int(fxp->pool, &fxp->payload,
    &fxp->payload_sz);

  memset(version_str, '\0', sizeof(version_str));
  snprintf(version_str, sizeof(version_str)-1, "%lu",
    (unsigned long) fxp_session->client_version);

  cmd = fxp_cmd_alloc(fxp->pool, "INIT", version_str);
  cmd->class = CL_MISC;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "INIT", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", version_str, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: INIT %lu",
    (unsigned long) fxp_session->client_version);

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_VERSION);

  if (fxp_session->client_version > fxp_max_client_version) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested SFTP protocol version %lu, which exceeds "
      "SFTPClientMatch max SFTP protocol version %u, using protocol version %u",
      (unsigned long) fxp_session->client_version, fxp_max_client_version,
      fxp_max_client_version);
    fxp_session->client_version = fxp_max_client_version;
  }

  if (fxp_session->client_version < fxp_min_client_version) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested SFTP protocol version %lu, which is less than "
      "SFTPClientMatch min SFTP protocol version %u, using protocol version %u",
      (unsigned long) fxp_session->client_version, fxp_min_client_version,
      fxp_min_client_version);
    fxp_session->client_version = fxp_min_client_version;
  }

#ifndef PR_USE_NLS
  /* If NLS supported was enabled in the proftpd build, then we can support
   * UTF8, and thus every other version of SFTP.  Otherwise, we can only
   * support up to version 3.
   */
  if (fxp_session->client_version > 3) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested SFTP protocol version %lu, but we can only support "
      "protocol version 3 due to lack of UTF8 support (requires --enable-nls)",
      (unsigned long) fxp_session->client_version);
    fxp_session->client_version = 3;
  }
#endif

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "using SFTP protocol version %lu for this session (channel ID %lu)",
    (unsigned long) fxp_session->client_version,
    (unsigned long) fxp->channel_id);

  pr_trace_msg(trace_channel, 8, "sending response: VERSION %lu",
    (unsigned long) fxp_session->client_version);

  sftp_msg_write_int(&buf, &buflen, fxp_session->client_version);

  fxp_version_add_vendor_id_ext(fxp->pool, &buf, &buflen);
  fxp_version_add_version_ext(fxp->pool, &buf, &buflen);
  fxp_version_add_openssh_exts(fxp->pool, &buf, &buflen);

  if (fxp_session->client_version >= 4) {
    fxp_version_add_newline_ext(fxp->pool, &buf, &buflen);
  }

  if (fxp_session->client_version == 5) {
    fxp_version_add_supported_ext(fxp->pool, &buf, &buflen);
  }

  if (fxp_session->client_version >= 6) {
    fxp_version_add_supported2_ext(fxp->pool, &buf, &buflen);
  }

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_link(struct fxp_packet *fxp) {
  char *buf, *args, *cmd_name, *ptr, *src_path, *dst_path;
  const char *reason;
  char is_symlink;
  int have_error = FALSE, res;
  uint32_t buflen, bufsz, status_code;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  src_path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    src_path = sftp_utf8_decode_str(fxp->pool, src_path);
  }

  dst_path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    dst_path = sftp_utf8_decode_str(fxp->pool, dst_path);
  }

  args = pstrcat(fxp->pool, src_path, " ", dst_path, NULL);

  cmd = fxp_cmd_alloc(fxp->pool, "LINK", args);
  cmd->class = CL_WRITE;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "LINK", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", args, NULL, NULL);

  is_symlink = sftp_msg_read_byte(fxp->pool, &fxp->payload, &fxp->payload_sz);

  pr_trace_msg(trace_channel, 7, "received request: LINK %s %s %d", src_path,
    dst_path, is_symlink);

  if (strlen(src_path) == 0) {
    /* Use the default directory if the path is empty. */
    src_path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty link path given in LINK request, using '%s'", src_path);
  }

  if (strlen(dst_path) == 0) {
    /* Use the default directory if the path is empty. */
    dst_path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty target path given in LINK request, using '%s'", dst_path);
  }

  /* Make sure we use the full paths. */
  src_path = dir_canonical_vpath(fxp->pool, src_path);
  dst_path = dir_canonical_vpath(fxp->pool, dst_path);

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  cmd_name = cmd->argv[0];
  cmd->argv[0] = "SYMLINK";

  if (!dir_check(fxp->pool, cmd, G_READ, src_path, NULL)) {
    cmd->argv[0] = cmd_name;
    have_error = TRUE;
  }

  if (!have_error &&
      !dir_check(fxp->pool, cmd, G_WRITE, dst_path, NULL)) {
    cmd->argv[0] = cmd_name;
    have_error = TRUE;
  }

  cmd->argv[0] = cmd_name;

  if (have_error) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "LINK of '%s' to '%s' blocked by <Limit> configuration",
      src_path, dst_path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (is_symlink) {
    res = pr_fsio_symlink(src_path, dst_path);

  } else {
    res = pr_fsio_link(src_path, dst_path);
  }

  if (res < 0) {
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error %s symlinking '%s' to '%s': %s",
      is_symlink ? "symlinking" : "linking", src_path, dst_path,
      strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

  } else {
    errno = 0;
    status_code = fxp_errno2status(0, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);

    pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);
  }

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_lock(struct fxp_packet *fxp) {
  char *buf, *ptr, *name;
  const char *lock_type_str = NULL;
  uint32_t buflen, bufsz, lock_flags, status_code;
  uint64_t offset, lock_len;
  struct flock lock;
  struct fxp_handle *fxh;
  struct fxp_packet *resp;
  cmd_rec *cmd;
  
  name = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  offset = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);
  lock_len = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);
  lock_flags = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);

  cmd = fxp_cmd_alloc(fxp->pool, "LOCK", name);
  cmd->class = CL_WRITE;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "LOCK", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", name, NULL, NULL);

  pr_trace_msg(trace_channel, 7,
    "received request: LOCK %s %" PR_LU " %" PR_LU " %lu",
    name, (pr_off_t) offset, (pr_off_t) lock_len, (unsigned long) lock_flags);

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  fxh = fxp_handle_get(name);
  if (fxh == NULL) {
    status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxh->fh == NULL) {
    /* We do not support locking of directory handles, only files. */
    status_code = SSH2_FX_OP_UNSUPPORTED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested unsupported LOCK of a directory, rejecting");

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  if (!dir_check(fxp->pool, cmd, G_WRITE, fxh->fh->fh_path, NULL)) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "LOCK of '%s' blocked by <Limit> configuration", fxh->fh->fh_path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", fxh->fh->fh_path, NULL, NULL);

  if (lock_flags & SSH2_FXL_DELETE) {
    /* The UNLOCK command is used for removing locks, not LOCK. */
    status_code = SSH2_FX_OP_UNSUPPORTED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested lock removal using LOCK, rejecting");

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);

  } else {
    if ((lock_flags & SSH2_FXL_WRITE) &&
        (lock_flags & SSH2_FXL_READ)) {
      /* We do not support simultaneous read and write locking. */
      status_code = SSH2_FX_OP_UNSUPPORTED;

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "client requested unsupported simultaneous read/write LOCK, "
        "rejecting");

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
        (unsigned long) status_code, fxp_strerror(status_code));

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
        fxp_strerror(status_code), NULL);
  
      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);
  
      return fxp_packet_write(resp);
    }

    if (lock_flags & SSH2_FXL_READ) {
      lock.l_type = F_RDLCK;
      lock_type_str = "read";
    }

    if (lock_flags & SSH2_FXL_WRITE) {
      lock.l_type = F_WRLCK;
      lock_type_str = "write";
    }
  }

  lock.l_whence = SEEK_SET;
  lock.l_start = offset;
  lock.l_len = lock_len;

  if (lock_len > 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested %s locking of '%s' from %" PR_LU " for %" PR_LU
      " bytes", lock_type_str, fxh->fh->fh_path, (pr_off_t) offset,
      (pr_off_t) lock_len);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested %s locking of '%s' from %" PR_LU " to end-of-file",
      lock_type_str, fxh->fh->fh_path, (pr_off_t) offset);
  }

  pr_trace_msg("lock", 9, "attemping to %s lock file '%s'", lock_type_str,
    fxh->fh->fh_path);

  while (fcntl(fxh->fh->fh_fd, F_SETLKW, &lock) < 0) {
    int xerrno;
    const char *reason;

    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    xerrno = errno;
    pr_trace_msg("lock", 3, "%s-lock of '%s' failed: %s", lock_type_str,
      fxh->fh->fh_path, strerror(errno)); 

    if (errno == EACCES) { 
      /* Get the PID of the process blocking this lock. */
      if (fcntl(fxh->fh->fh_fd, F_GETLK, &lock) == 0) {
        pr_trace_msg("lock", 3, "process ID %lu has blocking %s lock on '%s'",
          (unsigned long) lock.l_pid, lock.l_type == F_RDLCK ? "read" : "write",
          fxh->fh->fh_path);
      }

      status_code = SSH2_FX_LOCK_CONFLICT;
      reason = fxp_strerror(status_code);

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
        (unsigned long) status_code, reason);

    } else {
      status_code = fxp_errno2status(xerrno, &reason);

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
        "('%s' [%d])", (unsigned long) status_code, reason,
        xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);
    }

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_trace_msg("lock", 9, "%s lock of file '%s' successful", lock_type_str,
    fxh->fh->fh_path);

  status_code = SSH2_FX_OK;

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
    (unsigned long) status_code, fxp_strerror(status_code));

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
    fxp_strerror(status_code), NULL);

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_lstat(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *path;
  uint32_t buflen, bufsz;
  struct stat st;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    path = sftp_utf8_decode_str(fxp->pool, path);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "LSTAT", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", path, NULL, NULL);

  if (fxp_session->client_version > 3) {
    uint32_t attr_flags;

    /* These are hints from the client about what file attributes are
     * of particular interest.  We do not currently honor them.
     */
    attr_flags = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);

    pr_trace_msg(trace_channel, 7, "received request: LSTAT %s %s", path,
      fxp_strattrflags(fxp->pool, attr_flags));

  } else {
    pr_trace_msg(trace_channel, 7, "received request: LSTAT %s", path);
  }

  if (strlen(path) == 0) {
    /* Use the default directory if the path is empty. */
    path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty path given in LSTAT request, using '%s'", path);
  }

  cmd = fxp_cmd_alloc(fxp->pool, "LSTAT", path);
  cmd->class = CL_READ;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    uint32_t status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "LSTAT of '%s' blocked by '%s' handler", path, cmd->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  /* The path may have been changed by any PRE_CMD handlers. */
  path = cmd->arg;

  cmd_name = cmd->argv[0];
  cmd->argv[0] = "LSTAT";

  if (!dir_check(fxp->pool, cmd, G_NONE, path, NULL)) {
    uint32_t status_code = SSH2_FX_PERMISSION_DENIED;

    cmd->argv[0] = cmd_name;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "LSTAT of '%s' blocked by <Limit> configuration", path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  cmd->argv[0] = cmd_name;

  if (pr_fsio_lstat(path, &st) < 0) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error checking '%s' for LSTAT: %s", path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_trace_msg(trace_channel, 8, "sending response: ATTRS %s",
    fxp_strattrs(fxp->pool, &st, NULL));

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_ATTRS);
  sftp_msg_write_int(&buf, &buflen, fxp->request_id);
  fxp_attrs_write(fxp->pool, &buf, &buflen, &st);

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_mkdir(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *path;
  struct stat *attrs;
  int have_error = FALSE;
  mode_t dir_mode;
  uint32_t attr_flags, buflen, bufsz, status_code;
  struct fxp_packet *resp;
  cmd_rec *cmd, *cmd2;

  path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    path = sftp_utf8_decode_str(fxp->pool, path);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "MKDIR", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", path, NULL, NULL);

  attrs = fxp_attrs_read(fxp, &fxp->payload, &fxp->payload_sz, &attr_flags);
  if (attrs == NULL) {
    return 0;
  }

  pr_trace_msg(trace_channel, 7, "received request: MKDIR %s %s", path,
    fxp_strattrs(fxp->pool, attrs, &attr_flags));

  if (strlen(path) == 0) {
    /* Use the default directory if the path is empty. */
    path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty path given in MKDIR request, using '%s'", path);
  }

  cmd = fxp_cmd_alloc(fxp->pool, "MKDIR", path);
  cmd->class = CL_WRITE;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "MKDIR of '%s' blocked by '%s' handler", path, cmd->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  path = cmd->arg;

  cmd2 = fxp_cmd_alloc(fxp->pool, C_MKD, path);
  if (pr_cmd_dispatch_phase(cmd2, PRE_CMD, 0) == -1) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "MKDIR of '%s' blocked by '%s' handler", path, cmd2->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  /* The path may have been changed by any PRE_CMD handlers. */
  path = cmd2->arg;

  cmd_name = cmd->argv[0];
  cmd->argv[0] = C_MKD;

  if (!dir_check(fxp->pool, cmd, G_WRITE, path, NULL)) {
    cmd->argv[0] = cmd_name;
    have_error = TRUE;
  }

  cmd->argv[0] = C_XMKD;

  if (!have_error &&
      !dir_check(fxp->pool, cmd, G_WRITE, path, NULL)) {
    cmd->argv[0] = cmd_name;
    have_error = TRUE;
  }

  cmd->argv[0] = cmd_name;

  if (have_error) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "MKDIR of '%s' blocked by <Limit> configuration", path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxp_path_pass_regex_filters(fxp->pool, "MKDIR", path) < 0) {
    status_code = fxp_errno2status(errno, NULL);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  dir_mode = (attr_flags & SSH2_FX_ATTR_PERMISSIONS) ? attrs->st_mode : 0777;

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "creating directory '%s' with mode 0%o", path, (unsigned int) dir_mode);

  if (pr_fsio_mkdir(path, dir_mode) < 0) {
    const char *reason;
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "MKDIR, user '%s' (UID %lu, GID %lu): "
      "error making directory '%s': %s", session.user,
      (unsigned long) session.uid, (unsigned long) session.gid, path,
      strerror(xerrno));

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "MKDIR of '%s' failed: %s", path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  /* XXX Handle GroupOwner, UserOwner?  Requires root privs, which we
   * dropped.  Thus the admin must be warned that, to use these directives,
   * they must explicitly use "RootRevoke off" for mod_sftp configs, and be
   * warned of the consequences of doing so.
   */

  status_code = SSH2_FX_OK;

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
    (unsigned long) status_code, fxp_strerror(status_code));

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
    fxp_strerror(status_code), NULL);

  pr_cmd_dispatch_phase(cmd2, POST_CMD, 0);
  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_open(struct fxp_packet *fxp) {
  char *buf, *ptr, *path, *hiddenstore_path = NULL;
  uint32_t attr_flags, buflen, bufsz, desired_access = 0, flags;
  int open_flags, res, timeout_stalled;
  pr_fh_t *fh;
  struct stat *attrs;
  struct fxp_handle *fxh;
  struct fxp_packet *resp;
  cmd_rec *cmd, *cmd2 = NULL;

  path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    path = sftp_utf8_decode_str(fxp->pool, path);
  }

  cmd = fxp_cmd_alloc(fxp->pool, "OPEN", path);

  /* Set the command class to MISC for now; we'll change it later to
   * READ or WRITE once we know which it is.
   */
  cmd->class = CL_MISC;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "OPEN", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", path, NULL, NULL);

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (fxp_session->client_version > 4) {
    desired_access = sftp_msg_read_int(fxp->pool, &fxp->payload,
      &fxp->payload_sz);

    /* Check for unsupported flags. */
    if ((desired_access & SSH2_FXF_WANT_READ_NAMED_ATTRS) ||
        (desired_access & SSH2_FXF_WANT_READ_ACL) ||
        (desired_access & SSH2_FXF_WANT_WRITE_NAMED_ATTRS) ||
        (desired_access & SSH2_FXF_WANT_WRITE_ACL) ||
        (desired_access & SSH2_FXF_WANT_WRITE_OWNER)) {
      uint32_t status_code;
      const char *unsupported_str = "";

      if (desired_access & SSH2_FXF_WANT_READ_NAMED_ATTRS) {
        unsupported_str = pstrcat(fxp->pool, unsupported_str,
          *unsupported_str ? "|" : "", "WANT_READ_NAMED_ATTRS", NULL);
      }

      if (desired_access & SSH2_FXF_WANT_READ_ACL) {
        unsupported_str = pstrcat(fxp->pool, unsupported_str,
          *unsupported_str ? "|" : "", "WANT_READ_ACL", NULL);
      }

      if (desired_access & SSH2_FXF_WANT_WRITE_NAMED_ATTRS) {
        unsupported_str = pstrcat(fxp->pool, unsupported_str,
          *unsupported_str ? "|" : "", "WANT_WRITE_NAMED_ATTRS", NULL);
      }

      if (desired_access & SSH2_FXF_WANT_WRITE_ACL) {
        unsupported_str = pstrcat(fxp->pool, unsupported_str,
          *unsupported_str ? "|" : "", "WANT_WRITE_ACL", NULL);
      }

      if (desired_access & SSH2_FXF_WANT_WRITE_OWNER) {
        unsupported_str = pstrcat(fxp->pool, unsupported_str,
          *unsupported_str ? "|" : "", "WANT_WRITE_OWNER", NULL);
      }

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "client requested unsupported access '%s' in OPEN command, rejecting",
        unsupported_str);

      status_code = SSH2_FX_OP_UNSUPPORTED;

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
        (unsigned long) status_code, fxp_strerror(status_code));

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
        fxp_strerror(status_code), NULL);

      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);

      return fxp_packet_write(resp);
    }
  }

  flags = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);

  /* Check for any unsupported flags. */
  if (fxp_session->client_version > 4) {
    /* XXX If O_SHLOCK and O_EXLOCK are defined, as they are on OSX, the
     * ACCESS_READ_LOCK and ACCESS_WRITE_LOCK flags should be supported.
     *
     * Note that IF we support these LOCK flags, we will need to report
     * this support in the VERSION response as well.
     */

    if ((flags & SSH2_FXF_ACCESS_READ_LOCK) ||
        (flags & SSH2_FXF_ACCESS_WRITE_LOCK) ||
        (flags & SSH2_FXF_ACCESS_DELETE_LOCK)) {
      uint32_t status_code;
      const char *unsupported_str = "";

      if (desired_access & SSH2_FXF_ACCESS_READ_LOCK) {
        unsupported_str = pstrcat(fxp->pool, unsupported_str,
          *unsupported_str ? "|" : "", "ACCESS_READ_LOCK", NULL);
      }

      if (desired_access & SSH2_FXF_ACCESS_WRITE_LOCK) {
        unsupported_str = pstrcat(fxp->pool, unsupported_str,
          *unsupported_str ? "|" : "", "ACCESS_WRITE_LOCK", NULL);
      }

      if (desired_access & SSH2_FXF_ACCESS_DELETE_LOCK) {
        unsupported_str = pstrcat(fxp->pool, unsupported_str,
          *unsupported_str ? "|" : "", "ACCESS_DELETE_LOCK", NULL);
      }

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "client requested unsupported flag '%s' in OPEN command, rejecting",
        unsupported_str);

      status_code = SSH2_FX_OP_UNSUPPORTED;

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
        (unsigned long) status_code, fxp_strerror(status_code));

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
        fxp_strerror(status_code), NULL);

      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);

      return fxp_packet_write(resp);
    }

    /* Make sure the requested path exists. */
    if ((flags & SSH2_FXF_OPEN_EXISTING) &&
        !exists(path)) {
      uint32_t status_code;

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "client requested OPEN_EXISTING flag in OPEN command and '%s' does "
        "not exist", path);

      status_code = SSH2_FX_NO_SUCH_FILE;

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
        (unsigned long) status_code, fxp_strerror(status_code));

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
        fxp_strerror(status_code), NULL);

      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);

      return fxp_packet_write(resp);
    }
  }

  if (fxp_session->client_version < 5) {
    open_flags = fxp_get_v3_open_flags(flags);

  } else {
    open_flags = fxp_get_v5_open_flags(desired_access, flags);
  }

  attrs = fxp_attrs_read(fxp, &fxp->payload, &fxp->payload_sz, &attr_flags);
  if (attrs == NULL) {
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);
    return 0;
  }

  pr_trace_msg(trace_channel, 7, "received request: OPEN %s %s (%s)",
    path, fxp_strattrs(fxp->pool, attrs, &attr_flags),
    fxp_stroflags(fxp->pool, open_flags));

  if (open_flags & O_APPEND) {
    cmd->class = CL_WRITE;
    cmd2 = fxp_cmd_alloc(fxp->pool, C_APPE, path);

  } else if ((open_flags & O_WRONLY) ||
             (open_flags & O_RDWR)) {
    cmd2 = fxp_cmd_alloc(fxp->pool, C_STOR, path);

    if (open_flags & O_WRONLY) {
      cmd->class = CL_WRITE;

    } else if (open_flags & O_RDWR) {
      cmd->class = CL_READ|CL_WRITE;
    }

  } else if (open_flags == O_RDONLY) {
    cmd->class = CL_READ;
    cmd2 = fxp_cmd_alloc(fxp->pool, C_RETR, path);
  }

  if (cmd2) {
    if (pr_cmd_dispatch_phase(cmd2, PRE_CMD, 0) < 0) {
      int xerrno = errno;
      const char *reason;
      uint32_t status_code;

      /* One of the PRE_CMD phase handlers rejected the command. */

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "OPEN command for '%s' blocked by '%s' handler", path, cmd2->argv[0]);

      /* Hopefully the command handlers set an appropriate errno value.  If
       * they didn't, however, we need to be prepared with a fallback.
       */
      if (xerrno != ENOENT &&
          xerrno != EACCES &&
          xerrno != EPERM &&
          xerrno != EINVAL) {
        xerrno = EACCES;
      }

      status_code = fxp_errno2status(xerrno, &reason);

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
        "('%s' [%d])", (unsigned long) status_code, reason,
        xerrno != EOF ? strerror(errno) : "End of file", xerrno);

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
        NULL);

      pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
      pr_cmd_dispatch_phase(cmd2, LOG_CMD_ERR, 0);

      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);

      return fxp_packet_write(resp);
    }

    path = cmd2->arg;

    if (session.xfer.xfer_type == STOR_HIDDEN) {
      hiddenstore_path = pr_table_get(cmd2->notes,
        "mod_xfer.store-hidden-path", NULL);

    } else {
      path = dir_best_path(fxp->pool, path);
    }
  }

  /* We automatically add the O_NONBLOCK flag to the set of open() flags
   * in order to deal with writing to a FIFO whose other end may not be
   * open.  Then, after a successful open, we return the file to blocking
   * mode.
   */
  fh = pr_fsio_open(hiddenstore_path ? hiddenstore_path : path,
    open_flags|O_NONBLOCK);
  if (fh == NULL) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "OPEN, user '%s' (UID %lu, GID %lu): "
      "error opening '%s': %s", session.user,
      (unsigned long) session.uid, (unsigned long) session.gid,
      hiddenstore_path ? hiddenstore_path : path, strerror(xerrno));

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error opening '%s': %s", hiddenstore_path ? hiddenstore_path : path,
      strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    if (cmd2) {
      pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
      pr_cmd_dispatch_phase(cmd2, LOG_CMD_ERR, 0);
    }

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_fsio_set_block(fh);

  /* If the SFTPOption for ignoring perms for SFTP uploads is set, handle
   * it by clearing the SSH2_FX_ATTR_PERMISSIONS flag.
   */
  if ((sftp_opts & SFTP_OPT_IGNORE_SFTP_UPLOAD_PERMS) &&
      (attr_flags & SSH2_FX_ATTR_PERMISSIONS)) {
    pr_trace_msg(trace_channel, 7, "SFTPOption 'IgnoreSFTPUploadPerms' "
      "configured, ignoring perms sent by client");
    attr_flags &= ~SSH2_FX_ATTR_PERMISSIONS;
  }

  /* If the client provided a suggested size in the OPEN, ignore it.
   * Trying to honor the suggested size by truncating the file here can
   * cause problems, as when the client is resuming a transfer and the
   * resumption fails; the file would then be worse off than before due to the
   * truncation.  See:
   *
   *  http://winscp.net/tracker/show_bug.cgi?id=351
   *
   * The truncation isn't really needed anyway, since the ensuing READ/WRITE
   * requests will contain the offsets into the file at which to begin
   * reading/write the file contents.
   */

  attr_flags &= ~SSH2_FX_ATTR_SIZE;

  res = fxp_attrs_set(fh, fh->fh_path, attrs, attr_flags, &buf, &buflen, fxp);
  if (res < 0) {
    pr_fsio_close(fh);

    if (cmd2) {
      pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
      pr_cmd_dispatch_phase(cmd2, LOG_CMD_ERR, 0);
    }

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  fxh = fxp_handle_create(fxp_pool);
  fxh->fh = fh;
  fxh->fh_flags = open_flags;

  if (hiddenstore_path) {
    fxh->fh_real_path = pstrdup(fxh->pool, path);
  }

  if (fxp_handle_add(fxp->channel_id, fxh) < 0) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    buf = ptr;
    buflen = bufsz;

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      reason, NULL);

    pr_fsio_close(fh);

    if (cmd2) {
      pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
      pr_cmd_dispatch_phase(cmd2, LOG_CMD_ERR, 0);
    }

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_trace_msg(trace_channel, 8, "sending response: HANDLE %s", fxh->name);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_HANDLE);
  sftp_msg_write_int(&buf, &buflen, fxp->request_id);
  sftp_msg_write_string(&buf, &buflen, fxh->name);

  session.xfer.p = pr_pool_create_sz(fxp_pool, 64);
  session.xfer.path = pstrdup(session.xfer.p, path);
  memset(&session.xfer.start_time, 0, sizeof(session.xfer.start_time));
  gettimeofday(&session.xfer.start_time, NULL);

  if ((open_flags & O_APPEND) ||
      (open_flags & O_WRONLY) ||
      (open_flags & O_RDWR)) {
    session.xfer.direction = PR_NETIO_IO_RD;

  } else if (open_flags == O_RDONLY) {
    session.xfer.direction = PR_NETIO_IO_WR;
  }

  pr_timer_remove(PR_TIMER_STALLED, ANY_MODULE);

  timeout_stalled = pr_data_get_timeout(PR_DATA_TIMEOUT_STALLED);
  if (timeout_stalled > 0) {
    pr_timer_add(timeout_stalled, PR_TIMER_STALLED, NULL,
      fxp_timeout_stalled_cb, "TimeoutStalled");
  }

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_opendir(struct fxp_packet *fxp) {
  char *buf, *ptr, *path, *vpath;
  uint32_t buflen, bufsz;
  int timeout_stalled;
  void *dirh;
  struct fxp_handle *fxh;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    path = sftp_utf8_decode_str(fxp->pool, path);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "OPENDIR", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", path, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: OPENDIR %s", path);

  if (strlen(path) == 0) {
    /* Use the default directory if the path is empty. */
    path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty path given in OPENDIR request, using '%s'", path);
  }

  cmd = fxp_cmd_alloc(fxp->pool, "OPENDIR", path);
  cmd->class = CL_DIRS;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    uint32_t status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "OPENDIR of '%s' blocked by '%s' handler", path, cmd->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  /* The path may have been changed by any PRE_CMD handlers. */
  path = cmd->arg;

  vpath = dir_canonical_vpath(fxp->pool, path);
  if (vpath == NULL) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error resolving '%s': %s", path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  path = vpath;

  dirh = pr_fsio_opendir(path);
  if (dirh == NULL) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "OPENDIR, user '%s' (UID %lu, "
      "GID %lu): error opening '%s': %s", session.user,
      (unsigned long) session.uid, (unsigned long) session.gid, path,
      strerror(xerrno));

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error opening '%s': %s", path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  fxh = fxp_handle_create(fxp_pool);
  fxh->dirh = dirh;

  if (strcmp(path, pr_fs_getvwd()) != 0) {
    fxh->dir = pstrdup(fxh->pool, path);
  } else {
    fxh->dir = "";
  }

  if (fxp_handle_add(fxp->channel_id, fxh) < 0) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    buf = ptr;
    buflen = bufsz;

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_fsio_closedir(dirh);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_trace_msg(trace_channel, 8, "sending response: HANDLE %s",
    fxh->name);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_HANDLE);
  sftp_msg_write_int(&buf, &buflen, fxp->request_id);
  sftp_msg_write_string(&buf, &buflen, fxh->name);

  session.xfer.p = pr_pool_create_sz(fxp_pool, 64);
  memset(&session.xfer.start_time, 0, sizeof(session.xfer.start_time));
  gettimeofday(&session.xfer.start_time, NULL);
  session.xfer.direction = PR_NETIO_IO_WR;

  pr_timer_remove(PR_TIMER_STALLED, ANY_MODULE);

  timeout_stalled = pr_data_get_timeout(PR_DATA_TIMEOUT_STALLED);
  if (timeout_stalled > 0) {
    pr_timer_add(timeout_stalled, PR_TIMER_STALLED, NULL,
      fxp_timeout_stalled_cb, "TimeoutStalled");
  }

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_read(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *data = NULL, *name;
  int res;
  uint32_t buflen, bufsz, datalen;
  uint64_t offset;
  struct stat st;
  struct fxp_handle *fxh;
  struct fxp_packet *resp;
  cmd_rec *cmd, *cmd2;

  name = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  offset = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);
  datalen = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);

#if 0
  /* XXX This doesn't appear to be needed now.  But I'll keep it around,
   * just in case some buggy client needs this treatment.
   */
  if (datalen > max_readsz) {
    pr_trace_msg(trace_channel, 8,
      "READ requested len %lu exceeds max (%lu), truncating",
      (unsigned long) datalen, (unsigned long) max_readsz);
    datalen = max_readsz;
  }
#endif

  cmd = fxp_cmd_alloc(fxp->pool, "READ", name);
  cmd->class = CL_READ;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "READ", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", name, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: READ %s %" PR_LU " %lu",
    name, (pr_off_t) offset, (unsigned long) datalen);

  buflen = bufsz = datalen + 64;
  buf = ptr = palloc(fxp->pool, bufsz);

  fxh = fxp_handle_get(name);
  if (fxh == NULL) {
    uint32_t status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxh->fh == NULL) {
    uint32_t status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", fxh->fh->fh_path, NULL, NULL);

  if (pr_fsio_fstat(fxh->fh, &st) < 0) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error checking '%s' for READ: %s", fxh->fh->fh_path, strerror(xerrno));

    status_code = fxp_errno2status(errno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, xerrno == EOF ? LOG_CMD : LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (offset > st.st_size) {
    uint32_t status_code;
    const char *reason;
    int xerrno = EOF;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "requested read offset (%" PR_LU " bytes) greater than size of "
      "'%s' (%" PR_LU " bytes)", (pr_off_t) offset, fxh->fh->fh_path,
      (pr_off_t) st.st_size);

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason, "End of file",
      xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_XFER_SIZE, st.st_size,
    PR_SCORE_XFER_DONE, (off_t) offset,
    NULL);

  cmd_name = cmd->argv[0];
  cmd->argv[0] = C_RETR;

  if (!dir_check(fxp->pool, cmd, G_READ, fxh->fh->fh_path, NULL)) {
    uint32_t status_code = SSH2_FX_PERMISSION_DENIED;

    cmd->argv[0] = cmd_name;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "READ of '%s' blocked by <Limit> configuration", fxh->fh->fh_path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  cmd->argv[0] = cmd_name;

  /* XXX Check MaxRetrieveFileSize */

  if (fxp_path_pass_regex_filters(fxp->pool, "READ", fxh->fh->fh_path) < 0) {
    uint32_t status_code;
    const char *reason;

    status_code = fxp_errno2status(errno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (S_ISREG(st.st_mode)) {
    if (pr_fsio_lseek(fxh->fh, offset, SEEK_SET) < 0) {
      uint32_t status_code;
      const char *reason;
      int xerrno = errno;

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error seeking to offset (%" PR_LU " bytes) for '%s': %s",
        (pr_off_t) offset, fxh->fh->fh_path, strerror(xerrno));

      status_code = fxp_errno2status(xerrno, &reason);

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
        "('%s' [%d])", (unsigned long) status_code, reason,
        xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
        NULL);
  
      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);
  
      return fxp_packet_write(resp);

    } else {
      /* No error. */
      errno = 0;
    }
  }

  cmd2 = fxp_cmd_alloc(fxp->pool, C_RETR, NULL);
  pr_throttle_init(cmd2);

  if (datalen) {
    data = palloc(fxp->pool, datalen);
  }

  res = pr_fsio_read(fxh->fh, data, datalen);

  if (pr_data_get_timeout(PR_DATA_TIMEOUT_NO_TRANSFER) > 0) {
    pr_timer_reset(PR_TIMER_NOXFER, ANY_MODULE);
  }

  if (pr_data_get_timeout(PR_DATA_TIMEOUT_STALLED) > 0) {
    pr_timer_reset(PR_TIMER_STALLED, ANY_MODULE);
  }

  if (res <= 0) {
    uint32_t status_code;
    const char *reason;
    int xerrno;

    if (res < 0) {
      xerrno = errno;

      (void) pr_trace_msg("fileperms", 1, "READ, user '%s' (UID %lu, GID %lu): "
        "error reading from '%s': %s", session.user,
        (unsigned long) session.uid, (unsigned long) session.gid,
        fxh->fh->fh_path, strerror(xerrno));

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading from '%s': %s", fxh->fh->fh_path, strerror(xerrno));

      errno = xerrno;

    } else {
      /* Assume EOF */
      pr_throttle_pause(offset, TRUE);
      xerrno = EOF;
    }

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, xerrno != EOF ? LOG_CMD_ERR : LOG_CMD, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_throttle_pause(offset, FALSE);

  pr_trace_msg(trace_channel, 8, "sending response: DATA (%lu bytes)",
    (unsigned long) res);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_DATA);
  sftp_msg_write_int(&buf, &buflen, fxp->request_id);
  sftp_msg_write_data(&buf, &buflen, data, res, TRUE);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  fxh->fh_bytes_xferred += res;
  session.xfer.total_bytes += res;
  session.total_bytes += res;
  
  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  res = fxp_packet_write(resp);
  return res;
}

static int fxp_handle_readdir(struct fxp_packet *fxp) {
  register unsigned int i;
  unsigned int max_entries = 30;
  char *buf, *cmd_name, *ptr, *name;
  uint32_t buflen, bufsz;
  struct dirent *dent;
  struct fxp_dirent **paths;
  struct fxp_handle *fxh;
  struct fxp_packet *resp;
  array_header *path_list;
  cmd_rec *cmd;
  int have_error = FALSE;
  mode_t *fake_mode = NULL;

  name = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);

  cmd = fxp_cmd_alloc(fxp->pool, "READDIR", name);
  cmd->class = CL_DIRS;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "READDIR", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", name, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: READDIR %s", name);

  /* XXX What's a good size here?
   * Currently we calculate the max path length for the maximum number of
   * entries, plus 256 bytes of additional data for that path.
   */
  buflen = bufsz = (max_entries * ((PR_TUNABLE_PATH_MAX + 1) + 256));
  buf = ptr = palloc(fxp->pool, bufsz);

  fxh = fxp_handle_get(name);
  if (fxh == NULL) {
    uint32_t status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxh->dirh == NULL) {
    uint32_t status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", fxh->dir, NULL, NULL);

  path_list = make_array(fxp->pool, max_entries, sizeof(struct fxp_dirent *));

  /* If blocked by <Limit LIST>/<Limit NLST>, return EOF immediately. */

  cmd_name = cmd->argv[0];
  cmd->argv[0] = C_LIST;

  if (!dir_check(fxp->pool, cmd, G_DIRS, (char *) fxh->dir, NULL)) {
    cmd->argv[0] = cmd_name;
    have_error = TRUE;
  }

  cmd->argv[0] = C_NLST;

  if (!have_error &&
      !dir_check(fxp->pool, cmd, G_DIRS, (char *) fxh->dir, NULL)) {
    cmd->argv[0] = cmd_name;
    have_error = TRUE;
  }

  cmd->argv[0] = cmd_name;

  if (have_error) {
    uint32_t status_code = SSH2_FX_EOF;
 
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "READDIR of '%s' blocked by <Limit> configuration", fxh->dir);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD, 0); 
    pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  fake_mode = get_param_ptr(get_dir_ctxt(fxp->pool, (char *) fxh->dir),
    "DirFakeMode", FALSE);

  while ((dent = pr_fsio_readdir(fxh->dirh)) != NULL) {
    char *real_path;
    struct fxp_dirent *fxd;

    pr_signals_handle();

    real_path = dir_canonical_vpath(fxp->pool, pdircat(fxp->pool, fxh->dir,
      dent->d_name, NULL));

    fxd = fxp_get_dirent(fxp->pool, cmd, real_path, fake_mode);
    if (fxd == NULL) {
      int xerrno = errno;

      pr_trace_msg(trace_channel, 3,
        "unable to obtain directory listing for '%s': %s", real_path,
        strerror(xerrno));

      continue;
    }

    fxd->client_path = pstrdup(fxp->pool, dent->d_name);
    
    *((struct fxp_dirent **) push_array(path_list)) = fxd;

    /* Make the number of entries returned per READDIR more dynamic. */
    if (path_list->nelts == max_entries) {
      break;
    }
  }

  if (pr_data_get_timeout(PR_DATA_TIMEOUT_STALLED) > 0) {
    pr_timer_reset(PR_TIMER_STALLED, ANY_MODULE);
  }

  if (path_list->nelts == 0) {
    /* We have reached the end of the directory entries; send an EOF. */
    uint32_t status_code = SSH2_FX_EOF;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);
  
    pr_cmd_dispatch_phase(cmd, POST_CMD, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  pr_trace_msg(trace_channel, 8, "sending response: NAME (%lu count)",
    (unsigned long) path_list->nelts);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_NAME);
  sftp_msg_write_int(&buf, &buflen, fxp->request_id);
  sftp_msg_write_int(&buf, &buflen, path_list->nelts);

  paths = path_list->elts;
  for (i = 0; i < path_list->nelts; i++) {
    fxp_name_write(fxp->pool, &buf, &buflen, paths[i]->client_path,
      paths[i]->st);
  }

  if (fxp_session->client_version > 5) {
    sftp_msg_write_bool(&buf, &buflen, path_list->nelts == max_entries ?
      FALSE : TRUE);
  }

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  session.xfer.total_bytes += resp->payload_sz;
  session.total_bytes += resp->payload_sz;
  
  pr_cmd_dispatch_phase(cmd, POST_CMD, 0);
  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);
  return fxp_packet_write(resp);
}

static int fxp_handle_readlink(struct fxp_packet *fxp) {
  char data[PR_TUNABLE_PATH_MAX + 1];
  char *buf, *ptr, *path;
  int res;
  uint32_t buflen, bufsz;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    path = sftp_utf8_decode_str(fxp->pool, path);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "READLINK", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", path, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: READLINK %s", path);

  if (strlen(path) == 0) {
    /* Use the default directory if the path is empty. */
    path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty path given in READLINK request, using '%s'", path);
  }

  cmd = fxp_cmd_alloc(fxp->pool, "READLINK", path);
  cmd->class = CL_READ;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    uint32_t status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "READLINK of '%s' blocked by '%s' handler", path, cmd->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  /* The path may have been changed by any PRE_CMD handlers. */
  path = cmd->arg;

  res = pr_fsio_readlink(path, data, sizeof(data) - 1);
  if (res < 0) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    buf = ptr;
    buflen = bufsz;

    status_code = fxp_errno2status(xerrno, &reason);

    (void) pr_trace_msg("fileperms", 1, "READLINK, user '%s' (UID %lu, "
      "GID %lu): error using readlink() on  '%s': %s", session.user,
      (unsigned long) session.uid, (unsigned long) session.gid, path,
      strerror(xerrno));

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

  } else {
    struct stat st;
    memset(&st, 0, sizeof(struct stat));

    data[res] = '\0';

    pr_trace_msg(trace_channel, 8, "sending response: NAME 1 %s %s",
      data, fxp_strattrs(fxp->pool, &st, NULL));

    sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_NAME);
    sftp_msg_write_int(&buf, &buflen, fxp->request_id);
    sftp_msg_write_int(&buf, &buflen, 1);
    fxp_name_write(fxp->pool, &buf, &buflen, data, &st);

    pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);
  }

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_realpath(struct fxp_packet *fxp) {
  char *buf, *ptr, *path;
  uint32_t buflen, bufsz;
  struct stat st;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version > fxp_utf8_protocol_version) {
    path = sftp_utf8_decode_str(fxp->pool, path);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "REALPATH", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", path, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: REALPATH %s", path);

  if (strlen(path) == 0) {
    /* Use the default directory if the path is empty. */
    path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty path given in REALPATH request, using '%s'", path);
  }

  cmd = fxp_cmd_alloc(fxp->pool, "REALPATH", path);
  cmd->class = CL_INFO;

  if (fxp_session->client_version >= 6 &&
      fxp->payload_sz >= sizeof(char)) {
    char ctrl_flags;
    char *composite_path;

    ctrl_flags = sftp_msg_read_byte(fxp->pool, &fxp->payload,
      &fxp->payload_sz);
(void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION, "handle_realpath: ctrl_flags = %d", ctrl_flags);

    if (fxp->payload_sz > 0) {
      composite_path = sftp_msg_read_string(fxp->pool, &fxp->payload,
        &fxp->payload_sz);
(void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION, "handle_realpath: have composite-path = '%s'", composite_path);

      /* XXX One problem with the most recent SFTP Draft is that it does NOT
       * include a count of the number of composite-paths that the client
       * may send.  The format of the REALPATH request, currently, only allows
       * for one composite-path element; the description of this feature
       * implies that multiple such composite-path elements could be supplied.
       * Sigh.  I'll need to provide feedback on this, I see.
       */
    }
  }

  buflen = bufsz = PR_TUNABLE_PATH_MAX + 32;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    uint32_t status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "REALPATH of '%s' blocked by '%s' handler", path, cmd->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  /* The path may have been changed by any PRE_CMD handlers. */
  path = cmd->arg;

  if (strcmp(path, ".") == 0) {
    /* The client is asking about the current working directory.  Easy. */
    path = (char *) pr_fs_getvwd();

  } else {
    char *vpath;

    vpath = dir_canonical_vpath(fxp->pool, path);
    if (vpath == NULL) {
      uint32_t status_code;
      const char *reason;
      int xerrno = errno;

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error resolving '%s': %s", path, strerror(xerrno));

      status_code = fxp_errno2status(xerrno, &reason);

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
        "('%s' [%d])", (unsigned long) status_code, reason,
        xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
        NULL);

      pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);

      return fxp_packet_write(resp);
    }

    path = vpath;
  }

  if (pr_fsio_lstat(path, &st) < 0) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error checking '%s' for REALPATH: %s", path, strerror(xerrno));

    buf = ptr;
    buflen = bufsz;

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

  } else {
    pr_trace_msg(trace_channel, 8, "sending response: NAME 1 %s %s",
      path, fxp_strattrs(fxp->pool, &st, NULL));

    sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_NAME);
    sftp_msg_write_int(&buf, &buflen, fxp->request_id);
    sftp_msg_write_int(&buf, &buflen, 1);
    fxp_name_write(fxp->pool, &buf, &buflen, path, &st);

    pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);
  }

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_remove(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *path, *real_path;
  const char *reason;
  uint32_t buflen, bufsz, status_code;
  struct stat st;
  struct fxp_packet *resp;
  cmd_rec *cmd, *cmd2;

  path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    path = sftp_utf8_decode_str(fxp->pool, path);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "REMOVE", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", path, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: REMOVE %s", path);

  if (strlen(path) == 0) {
    /* Use the default directory if the path is empty. */
    path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty path given in REMOVE request, using '%s'", path);
  }

  cmd = fxp_cmd_alloc(fxp->pool, "REMOVE", path);
  cmd->class = CL_WRITE;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "REMOVE of '%s' blocked by '%s' handler", path, cmd->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  path = cmd->arg;

  cmd_name = cmd->argv[0];
  cmd->argv[0] = C_DELE;

  if (!dir_check(fxp->pool, cmd, G_WRITE, path, NULL)) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    cmd->argv[0] = cmd_name;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "REMOVE of '%s' blocked by <Limit> configuration", path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  cmd->argv[0] = cmd_name;

  if (fxp_path_pass_regex_filters(fxp->pool, "REMOVE", path) < 0) {
    status_code = fxp_errno2status(errno, NULL);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  real_path = dir_canonical_path(fxp->pool, path);
  if (real_path == NULL) {
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error resolving '%s': %s", path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (pr_fsio_lstat(real_path, &st) < 0) {
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to check '%s': %s", real_path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (S_ISDIR(st.st_mode)) {
    int xerrno = EISDIR;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to remove '%s': %s", real_path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  cmd2 = fxp_cmd_alloc(fxp_pool, "DELE", path);
  pr_cmd_dispatch_phase(cmd2, PRE_CMD, 0);

  if (pr_fsio_unlink(real_path) < 0) {
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "REMOVE, user '%s' (UID %lu, GID %lu): "
      "error deleting '%s': %s", session.user,
      (unsigned long) session.uid, (unsigned long) session.gid, real_path,
      strerror(xerrno));

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error unlinking '%s': %s", real_path, strerror(xerrno));
    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd2, LOG_CMD_ERR, 0);

    errno = xerrno;

  } else {
    char *abs_path;

    /* The TransferLog format wants the full path to the deleted file,
     * regardless of a chroot.
     */
    abs_path = dir_abs_path(fxp->pool, path, TRUE);

    xferlog_write(0, session.c->remote_name, st.st_size, abs_path,
      'b', 'd', 'r', session.user, 'c');

    pr_cmd_dispatch_phase(cmd2, POST_CMD, 0);
    pr_cmd_dispatch_phase(cmd2, LOG_CMD, 0);
    errno = 0;
  }

  status_code = fxp_errno2status(errno, &reason);

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
    "('%s' [%d])", (unsigned long) status_code, reason,
    errno != EOF ? strerror(errno) : "End of file", errno);

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  pr_cmd_dispatch_phase(cmd, errno == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_rename(struct fxp_packet *fxp) {
  char *buf, *args, *ptr, *old_path, *new_path;
  const char *reason;
  uint32_t buflen, bufsz, flags, status_code;
  struct fxp_packet *resp;
  cmd_rec *cmd, *cmd2, *cmd3;
  int xerrno = 0;

  old_path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  new_path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);

  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    old_path = sftp_utf8_decode_str(fxp->pool, old_path);
    new_path = sftp_utf8_decode_str(fxp->pool, new_path);
  }

  /* In protocol version 5 and later, there is a flags int which follows.
   * However, this flags argument is usually used to indicate to servers
   * that they can use the POSIX rename(2) semantics, i.e. that overwriting
   * a file at the new/destination path is OK.
   *
   * At the moment, since we use rename(2) anyway, even for the older protocol
   * versions, we don't read in the flags value.  This does mean, however,
   * that mod_sftp will not properly return an "file already exists" error
   * if the specified new/destination path already exists.
   */

  args = pstrcat(fxp->pool, old_path, " ", new_path, NULL);

  pr_trace_msg(trace_channel, 7, "received request: RENAME %s %s", old_path,
    new_path);

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "RENAME", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", args, NULL, NULL);

  if (strlen(old_path) == 0) {
    /* Use the default directory if the path is empty. */
    old_path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty old path given in RENAME request, using '%s'", old_path);
  }

  if (strlen(new_path) == 0) {
    /* Use the default directory if the path is empty. */
    new_path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty new path given in RENAME request, using '%s'", new_path);
  }

  if (fxp_session->client_version > 4) {
    flags = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);

    if (flags & SSH2_FXR_ATOMIC) {
      /* The ATOMIC flag implies OVERWRITE. */
      flags |= SSH2_FXR_OVERWRITE;
    }

  } else {
    flags = 0;
  }

  cmd = fxp_cmd_alloc(fxp->pool, "RENAME", args);
  cmd->class = CL_MISC;
 
  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  cmd2 = fxp_cmd_alloc(fxp->pool, C_RNTO, new_path);
  if (pr_cmd_dispatch_phase(cmd2, PRE_CMD, 0) < 0) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RENAME to '%s' blocked by '%s' handler", new_path, cmd2->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  new_path = cmd2->arg;

  cmd3 = fxp_cmd_alloc(fxp->pool, C_RNFR, old_path);
  if (pr_cmd_dispatch_phase(cmd3, PRE_CMD, 0) < 0) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RENAME from '%s' blocked by '%s' handler", old_path, cmd3->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd3, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  old_path = cmd3->arg;

  if (!dir_check(fxp->pool, cmd3, G_DIRS, old_path, NULL) ||
      !dir_check(fxp->pool, cmd2, G_WRITE, new_path, NULL)) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RENAME of '%s' to '%s' blocked by <Limit> configuration",
      old_path, new_path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (strcmp(old_path, new_path) == 0) {
    xerrno = EEXIST;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RENAME of '%s' to same path '%s', rejecting", old_path, new_path);

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (!(flags & SSH2_FXR_OVERWRITE) &&
      exists(new_path)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "denying RENAME of '%s' to '%s': '%s' already exists and client did not "
      "specify OVERWRITE flag", old_path, new_path, new_path);

    status_code = SSH2_FX_FILE_ALREADY_EXISTS;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxp_path_pass_regex_filters(fxp->pool, "RENAME", old_path) < 0 ||
      fxp_path_pass_regex_filters(fxp->pool, "RENAME", new_path) < 0) {
    status_code = fxp_errno2status(errno, NULL);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (pr_fsio_rename(old_path, new_path) < 0) {
    if (errno != EXDEV) {
      xerrno = errno;

      (void) pr_trace_msg("fileperms", 1, "RENAME, user '%s' (UID %lu, "
        "GID %lu): error renaming '%s' to '%s': %s", session.user,
        (unsigned long) session.uid, (unsigned long) session.gid,
        old_path, new_path, strerror(xerrno));

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error renaming '%s' to '%s': %s", old_path, new_path,
        strerror(xerrno));

      errno = xerrno;

    } else {
      /* In this case, we should manually copy the file from the source
       * path to the destination path.
       */
      errno = 0;
      if (pr_fs_copy_file(old_path, new_path) < 0) {
        xerrno = errno;

        (void) pr_trace_msg("fileperms", 1, "RENAME, user '%s' (UID %lu, "
          "GID %lu): error copying '%s' to '%s': %s", session.user,
          (unsigned long) session.uid, (unsigned long) session.gid,
          old_path, new_path, strerror(xerrno));

        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error copying '%s' to '%s': %s", old_path, new_path,
          strerror(xerrno));

        errno = xerrno;

      } else {
        /* Once copied, remove the original path. */
        if (pr_fsio_unlink(old_path) < 0) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error deleting '%s': %s", old_path, strerror(errno));
        }

        xerrno = errno = 0;
      }
    }

  } else {
    /* No errors. */
    xerrno = errno = 0;
  }

  status_code = fxp_errno2status(xerrno, &reason);

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
    "('%s' [%d])", (unsigned long) status_code, reason,
    xerrno != EOF ? strerror(errno) : "End of file", xerrno);

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  pr_cmd_dispatch_phase(cmd2, xerrno == 0 ? POST_CMD : POST_CMD_ERR, 0);
  pr_cmd_dispatch_phase(cmd, xerrno == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_rmdir(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *path;
  const char *reason;
  uint32_t buflen, bufsz, status_code;
  struct fxp_packet *resp;
  cmd_rec *cmd, *cmd2;
  int have_error = FALSE;

  path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    path = sftp_utf8_decode_str(fxp->pool, path);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "RMDIR", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", path, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: RMDIR %s", path);

  if (strlen(path) == 0) {
    /* Use the default directory if the path is empty. */
    path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty path given in RMDIR request, using '%s'", path);
  }

  cmd = fxp_cmd_alloc(fxp->pool, "RMDIR", path);
  cmd->class = CL_WRITE;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RMDIR of '%s' blocked by '%s' handler", path, cmd->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  path = cmd->arg;

  cmd2 = fxp_cmd_alloc(fxp->pool, C_RMD, path);
  if (pr_cmd_dispatch_phase(cmd2, PRE_CMD, 0) == -1) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RMDIR of '%s' blocked by '%s' handler", path, cmd2->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  cmd_name = cmd->argv[0];
  cmd->argv[0] = C_RMD;

  if (!dir_check(fxp->pool, cmd, G_WRITE, path, NULL)) {
    cmd->argv[0] = cmd_name;
    have_error = TRUE;
  }

  cmd->argv[0] = C_XRMD;

  if (!have_error &&
      !dir_check(fxp->pool, cmd, G_WRITE, path, NULL)) {
    cmd->argv[0] = cmd_name;
    have_error = TRUE;
  }

  cmd->argv[0] = cmd_name;

  if (have_error) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RMDIR of '%s' blocked by <Limit> configuration", path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxp_path_pass_regex_filters(fxp->pool, "RMDIR", path) < 0) {
    status_code = fxp_errno2status(errno, NULL);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (pr_fsio_rmdir(path) < 0) {
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "RMDIR, user '%s' (UID %lu, GID %lu): "
      "error removing directory '%s': %s", session.user,
      (unsigned long) session.uid, (unsigned long) session.gid, path,
      strerror(xerrno));

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error removing directory '%s': %s", path, strerror(xerrno));

    errno = xerrno;

  } else {
    /* No error. */
    errno = 0;
  }

#if defined(ENOTEMPTY) && ENOTEMPTY != EEXIST
  status_code = fxp_errno2status(errno, &reason);

#else
  /* On AIX5, ENOTEMPTY and EEXIST are defined to the same value.  See:
   *
   *  http://forums.proftpd.org/smf/index.php/topic,3971.0.html
   *
   * We still want to send the proper SFTP error code/string if we see
   * these values, though.  The fix for handling this case in
   * fxp_errno2status() means that we need to do the errno lookup a little
   * more manually here.
   */

  if (errno != ENOTEMPTY) {
    status_code = fxp_errno2status(errno, &reason);

  } else {
    /* Generic failure code, works for all protocol versions. */
    status_code = SSH2_FX_FAILURE;

    if (fxp_session->client_version > 3) {
      status_code = SSH2_FX_FILE_ALREADY_EXISTS;
    }

    if (fxp_session->client_version > 5) {
      status_code = SSH2_FX_DIR_NOT_EMPTY;
    }

    reason = fxp_strerror(status_code);
  }
#endif

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
    "('%s' [%d])", (unsigned long) status_code, reason,
    errno != EOF ? strerror(errno) : "End of file", errno);

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  pr_cmd_dispatch_phase(cmd2, errno == 0 ? POST_CMD : POST_CMD_ERR, 0);
  pr_cmd_dispatch_phase(cmd, errno == 0 ? LOG_CMD : LOG_CMD_ERR, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_setstat(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *path;
  const char *reason;
  uint32_t attr_flags, buflen, bufsz, status_code;
  int res;
  struct stat *attrs;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    path = sftp_utf8_decode_str(fxp->pool, path);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "SETSTAT", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", path, NULL, NULL);

  attrs = fxp_attrs_read(fxp, &fxp->payload, &fxp->payload_sz, &attr_flags);
  if (attrs == NULL) {
    return 0;
  }

  pr_trace_msg(trace_channel, 7, "received request: SETSTAT %s %s", path,
    fxp_strattrs(fxp->pool, attrs, &attr_flags));

  if (strlen(path) == 0) {
    /* Use the default directory if the path is empty. */
    path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty path given in SETSTAT request, using '%s'", path);
  }

  cmd = fxp_cmd_alloc(fxp->pool, "SETSTAT", path);
  cmd->class = CL_WRITE;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SETSTAT of '%s' blocked by '%s' handler", path, cmd->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  /* The path may have been changed by any PRE_CMD handlers. */
  path = cmd->arg;

  cmd_name = cmd->argv[0];
  cmd->argv[0] = "SETSTAT";

  if (!dir_check(fxp->pool, cmd, G_WRITE, path, NULL)) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    cmd->argv[0] = cmd_name;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SETSTAT of '%s' blocked by <Limit> configuration", path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  cmd->argv[0] = cmd_name;

  res = fxp_attrs_set(NULL, path, attrs, attr_flags, &buf, &buflen, fxp);
  if (res < 0) {
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  status_code = fxp_errno2status(0, &reason);

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
    (unsigned long) status_code, reason);

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_stat(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *path;
  uint32_t buflen, bufsz;
  struct stat st;
  struct fxp_packet *resp;
  cmd_rec *cmd;

  path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version > fxp_utf8_protocol_version) {
    path = sftp_utf8_decode_str(fxp->pool, path);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "STAT", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", path, NULL, NULL);

  if (fxp_session->client_version > 3) {
    uint32_t attr_flags;

    /* These are hints from the client about what file attributes are
     * of particular interest.  We do not currently honor them.
     */
    attr_flags = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);

    pr_trace_msg(trace_channel, 7, "received request: STAT %s %s", path,
      fxp_strattrflags(fxp->pool, attr_flags));

  } else {
    pr_trace_msg(trace_channel, 7, "received request: STAT %s", path);
  }

  if (strlen(path) == 0) {
    /* Use the default directory if the path is empty. */
    path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty path given in STAT request, using '%s'", path);
  }

  cmd = fxp_cmd_alloc(fxp->pool, "STAT", path);
  cmd->class = CL_READ;

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  if (pr_cmd_dispatch_phase(cmd, PRE_CMD, 0) < 0) {
    uint32_t status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "STAT of '%s' blocked by '%s' handler", path, cmd->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  /* The path may have been changed by any PRE_CMD handlers. */
  path = cmd->arg;

  cmd_name = cmd->argv[0];
  cmd->argv[0] = "STAT";

  if (!dir_check(fxp->pool, cmd, G_NONE, path, NULL)) {
    uint32_t status_code = SSH2_FX_PERMISSION_DENIED;

    cmd->argv[0] = cmd_name;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "STAT of '%s' blocked by <Limit> configuration", path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  cmd->argv[0] = cmd_name;

  if (pr_fsio_stat(path, &st) < 0) {
    uint32_t status_code;
    const char *reason;
    int xerrno = errno;

    if (xerrno != ENOENT) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error checking '%s' for STAT: %s", path, strerror(xerrno));
    }

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_trace_msg(trace_channel, 8, "sending response: ATTRS %s",
    fxp_strattrs(fxp->pool, &st, NULL));

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_FXP_ATTRS);
  sftp_msg_write_int(&buf, &buflen, fxp->request_id);
  fxp_attrs_write(fxp->pool, &buf, &buflen, &st);

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_symlink(struct fxp_packet *fxp) {
  char *buf, *args, *args2, *cmd_name, *ptr, *src_path, *dst_path, *vpath;
  const char *reason;
  int have_error = FALSE, res;
  uint32_t buflen, bufsz, status_code;
  struct fxp_packet *resp;
  cmd_rec *cmd, *cmd2;

  src_path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    src_path = sftp_utf8_decode_str(fxp->pool, src_path);
  }

  dst_path = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  if (fxp_session->client_version >= fxp_utf8_protocol_version) {
    dst_path = sftp_utf8_decode_str(fxp->pool, dst_path);
  }

  args = pstrcat(fxp->pool, src_path, " ", dst_path, NULL);

  cmd = fxp_cmd_alloc(fxp->pool, "SYMLINK", args);
  cmd->class = CL_WRITE;

  pr_trace_msg(trace_channel, 7, "received request: SYMLINK %s %s", src_path,
    dst_path);

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "SYMLINK", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", args, NULL, NULL);

  if (strlen(src_path) == 0) {
    /* Use the default directory if the path is empty. */
    src_path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty link path given in SYMLINK request, using '%s'", src_path);
  }

  if (strlen(dst_path) == 0) {
    /* Use the default directory if the path is empty. */
    dst_path = sftp_auth_get_default_dir();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "empty target path given in SYMLINK request, using '%s'", dst_path);
  }

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  /* Make sure we use the full paths. */
  vpath = dir_canonical_vpath(fxp->pool, src_path);
  if (vpath == NULL) {
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error resolving '%s': %s", src_path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  src_path = vpath;

  vpath = dir_canonical_vpath(fxp->pool, dst_path);
  if (vpath == NULL) {
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error resolving '%s': %s", dst_path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  dst_path = vpath;

  /* We use a slightly different cmd_rec here, for the benefit of PRE_CMD
   * handlers such as mod_rewrite.  It is impossible for a client to
   * send a tab ('\t') in SFTP, so we use that as our delimiter in the
   * single-string args argument in the cmd_rec.
   *
   * If the PRE_CMD dispatch is successful, we can then check to see
   * if the args string changed, and if so, parse back out the individual
   * paths.
   */

  args2 = pstrcat(fxp->pool, src_path, "\t", dst_path, NULL);
  cmd2 = fxp_cmd_alloc(fxp->pool, "SYMLINK", args2);
  cmd2->class = CL_WRITE;

  if (pr_cmd_dispatch_phase(cmd2, PRE_CMD, 0) < 0) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SYMLINK of '%s' to '%s' blocked by '%s' handler", src_path, dst_path,
      cmd2->argv[0]);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd2, POST_CMD_ERR, 0);
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  /* The paths may have been changed by any PRE_CMD handlers. */
  if (strcmp(args2, cmd2->arg) != 0) {
    ptr = strchr(cmd2->arg, '\t');
    if (ptr) {
      *ptr = '\0';
      src_path = cmd2->arg;
      dst_path = ptr + 1;
    }
  }

  cmd_name = cmd->argv[0];
  cmd->argv[0] = "SYMLINK";

  if (!dir_check(fxp->pool, cmd, G_READ, src_path, NULL)) {
    cmd->argv[0] = cmd_name;
    have_error = TRUE;
  }

  if (!have_error &&
      !dir_check(fxp->pool, cmd, G_WRITE, dst_path, NULL)) {
    cmd->argv[0] = cmd_name;
    have_error = TRUE;
  }

  cmd->argv[0] = cmd_name;

  if (have_error) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SYMLINK of '%s' to '%s' blocked by <Limit> configuration",
      src_path, dst_path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  res = pr_fsio_symlink(src_path, dst_path);

  if (res < 0) {
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error symlinking '%s' to '%s': %s", src_path, dst_path,
      strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

  } else {
    errno = 0;
    status_code = fxp_errno2status(0, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, reason);

    pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);
  }

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason, NULL);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_write(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *data, *name;
  int res;
  uint32_t buflen, bufsz, datalen, status_code;
  uint64_t offset;
  struct stat st;
  struct fxp_handle *fxh;
  struct fxp_packet *resp;
  cmd_rec *cmd, *cmd2;

  name = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  offset = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);
  datalen = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);
  data = sftp_msg_read_data(fxp->pool, &fxp->payload, &fxp->payload_sz,
    datalen);

  session.xfer.total_bytes += datalen;
  session.total_bytes += datalen;
  
  cmd = fxp_cmd_alloc(fxp->pool, "WRITE", name);
  cmd->class = CL_WRITE;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "WRITE", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", name, NULL, NULL);

  pr_trace_msg(trace_channel, 7, "received request: WRITE %s %" PR_LU " %lu",
    name, (pr_off_t) offset, (unsigned long) datalen);

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  fxh = fxp_handle_get(name);
  if (fxh == NULL) {
    status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxh->fh == NULL) {
    status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", fxh->fh->fh_path, NULL, NULL);
  fxh->fh_bytes_xferred += datalen;

  if (pr_fsio_fstat(fxh->fh, &st) < 0) {
    const char *reason;
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error checking '%s' for WRITE: %s", fxh->fh->fh_path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  /* It would be nice to check the requested offset against the size of
   * the file.  However, the protocol specifically allows for sparse files,
   * where the requested offset is far beyond the end of the file.
   *
   * XXX Perhaps this should be configurable?
   */
#if 0
  if (offset > st.st_size) {
    const char *reason;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "requested write offset (%" PR_LU " bytes) greater than size of "
      "'%s' (%" PR_LU " bytes)", (pr_off_t) offset, fxh->fh->fh_path,
      (pr_off_t) st.st_size);

    status_code = fxp_errno2status(EINVAL, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason, strerror(EINVAL),
      EINVAL);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }
#endif

  cmd_name = cmd->argv[0];
  cmd->argv[0] = C_STOR;

  if (!dir_check(fxp->pool, cmd, G_WRITE, fxh->fh->fh_path, NULL)) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    cmd->argv[0] = cmd_name;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "WRITE of '%s' blocked by <Limit> configuration", fxh->fh->fh_path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  cmd->argv[0] = cmd_name;

  /* XXX Check MaxStoreFileSize */

  if (fxp_path_pass_regex_filters(fxp->pool, "WRITE", fxh->fh->fh_path) < 0) {
    status_code = fxp_errno2status(errno, NULL);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (S_ISREG(st.st_mode)) {
    if (pr_fsio_lseek(fxh->fh, offset, SEEK_SET) < 0) {
      const char *reason;
      int xerrno = errno;

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error seeking to offset (%" PR_LU " bytes) for '%s': %s",
        (pr_off_t) offset, fxh->fh->fh_path, strerror(xerrno));

      status_code = fxp_errno2status(xerrno, &reason);

      pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
        "('%s' [%d])", (unsigned long) status_code, reason,
        xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

      fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
        NULL);
  
      pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

      resp = fxp_packet_create(fxp->pool, fxp->channel_id);
      resp->payload = ptr;
      resp->payload_sz = (bufsz - buflen);
  
      return fxp_packet_write(resp);
    }
  }

  /* If the open flags have O_APPEND, treat this as an APPE command, rather
   * than a STOR command.
   */
  if (!(fxh->fh_flags & O_APPEND)) {
    cmd2 = fxp_cmd_alloc(fxp->pool, C_STOR, NULL);

  } else {
    cmd2 = fxp_cmd_alloc(fxp->pool, C_APPE, NULL);
  }

  pr_throttle_init(cmd2);
  
  res = pr_fsio_write(fxh->fh, data, datalen);

  if (pr_data_get_timeout(PR_DATA_TIMEOUT_NO_TRANSFER) > 0) {
    pr_timer_reset(PR_TIMER_NOXFER, ANY_MODULE);
  }

  if (pr_data_get_timeout(PR_DATA_TIMEOUT_STALLED) > 0) {
    pr_timer_reset(PR_TIMER_STALLED, ANY_MODULE);
  }

  pr_throttle_pause(offset, FALSE);

  if (res < 0) {
    const char *reason;
    int xerrno = errno;

    (void) pr_trace_msg("fileperms", 1, "WRITE, user '%s' (UID %lu, GID %lu): "
      "error writing to '%s': %s", session.user,
      (unsigned long) session.uid, (unsigned long) session.gid,
      fxh->fh->fh_path, strerror(xerrno));

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error writing to '%s': %s", fxh->fh->fh_path, strerror(xerrno));

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  status_code = SSH2_FX_OK;

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
    (unsigned long) status_code, fxp_strerror(status_code));

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
    fxp_strerror(status_code), NULL);

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

static int fxp_handle_unlock(struct fxp_packet *fxp) {
  char *buf, *cmd_name, *ptr, *name;
  uint32_t buflen, bufsz, lock_flags, status_code;
  uint64_t offset, lock_len;
  struct flock lock;
  struct fxp_handle *fxh;
  struct fxp_packet *resp;
  cmd_rec *cmd;
  
  name = sftp_msg_read_string(fxp->pool, &fxp->payload, &fxp->payload_sz);
  offset = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);
  lock_len = fxp_msg_read_long(fxp->pool, &fxp->payload, &fxp->payload_sz);
  lock_flags = sftp_msg_read_int(fxp->pool, &fxp->payload, &fxp->payload_sz);

  cmd = fxp_cmd_alloc(fxp->pool, "UNLOCK", name);
  cmd->class = CL_WRITE;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD, "%s", "UNLOCK", NULL, NULL);
  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", name, NULL, NULL);

  pr_trace_msg(trace_channel, 7,
    "received request: UNLOCK %s %" PR_LU " %" PR_LU " %lu", name,
    (pr_off_t) offset, (pr_off_t) lock_len, (unsigned long) lock_flags);

  buflen = bufsz = FXP_RESPONSE_DATA_DEFAULT_SZ;
  buf = ptr = palloc(fxp->pool, bufsz);

  fxh = fxp_handle_get(name);
  if (fxh == NULL) {
    status_code = SSH2_FX_INVALID_HANDLE;

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  if (fxh->fh == NULL) {
    /* We do not support locking of directory handles, only files. */
    status_code = SSH2_FX_OP_UNSUPPORTED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested unsupported UNLOCK of a directory, rejecting");

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  cmd_name = cmd->argv[0];
  cmd->argv[0] = "LOCK";

  if (!dir_check(fxp->pool, cmd, G_WRITE, fxh->fh->fh_path, NULL)) {
    status_code = SSH2_FX_PERMISSION_DENIED;

    cmd->argv[0] = cmd_name;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "UNLOCK of '%s' blocked by <Limit> configuration", fxh->fh->fh_path);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }
  cmd->argv[0] = cmd_name;

  pr_scoreboard_entry_update(session.pid,
    PR_SCORE_CMD_ARG, "%s", fxh->fh->fh_path, NULL, NULL);

  if (lock_flags & SSH2_FXL_DELETE) {
    lock.l_type = F_UNLCK;

  } else {
    /* The LOCK command is used for adding locks, not UNLOCK. */
    status_code = SSH2_FX_OP_UNSUPPORTED;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested locking using UNLOCK, rejecting");

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
      (unsigned long) status_code, fxp_strerror(status_code));

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
      fxp_strerror(status_code), NULL);
  
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);
  
    return fxp_packet_write(resp);
  }

  lock.l_whence = SEEK_SET;
  lock.l_start = offset;
  lock.l_len = lock_len;

  if (lock_len > 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested unlocking of '%s' from %" PR_LU " for %" PR_LU
      " bytes", fxh->fh->fh_path, (pr_off_t) offset, (pr_off_t) lock_len);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "client requested unlocking of '%s' from %" PR_LU " to end-of-file",
      fxh->fh->fh_path, (pr_off_t) offset);
  }

  pr_trace_msg("lock", 9, "attemping to unlock file '%s'", fxh->fh->fh_path);

  while (fcntl(fxh->fh->fh_fd, F_SETLK, &lock) < 0) {
    int xerrno;
    const char *reason;

    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    xerrno = errno;
    pr_trace_msg("lock", 3, "unlock of '%s' failed: %s", fxh->fh->fh_path,
      strerror(errno)); 

    status_code = fxp_errno2status(xerrno, &reason);

    pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s' "
      "('%s' [%d])", (unsigned long) status_code, reason,
      xerrno != EOF ? strerror(xerrno) : "End of file", xerrno);

    fxp_status_write(&buf, &buflen, fxp->request_id, status_code, reason,
      NULL);

    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    resp = fxp_packet_create(fxp->pool, fxp->channel_id);
    resp->payload = ptr;
    resp->payload_sz = (bufsz - buflen);

    return fxp_packet_write(resp);
  }

  pr_trace_msg("lock", 9, "unlock of file '%s' successful", fxh->fh->fh_path);

  status_code = SSH2_FX_OK;

  pr_trace_msg(trace_channel, 8, "sending response: STATUS %lu '%s'",
    (unsigned long) status_code, fxp_strerror(status_code));

  fxp_status_write(&buf, &buflen, fxp->request_id, status_code,
    fxp_strerror(status_code), NULL);

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  resp = fxp_packet_create(fxp->pool, fxp->channel_id);
  resp->payload = ptr;
  resp->payload_sz = (bufsz - buflen);

  return fxp_packet_write(resp);
}

/* Main entry point */
int sftp_fxp_handle_packet(pool *p, void *ssh2, uint32_t channel_id,
    char *data, uint32_t datalen) {
  struct ssh2_packet *pkt;
  struct fxp_packet *fxp;
  int have_cache, res;

  if (fxp_pool == NULL) {
    fxp_pool = make_sub_pool(sftp_pool);
    pr_pool_tag(fxp_pool, "SFTP Pool");
  }

  pkt = ssh2;

  fxp = fxp_packet_read(channel_id, &data, &datalen, &have_cache);
  while (fxp) {
    pr_signals_handle();

    /* This is a bit of a hack, for playing along better with mod_vroot,
     * which pays attention to the session.curr_phase value.
     *
     * I'm not sure which is better here, PRE_CMD vs CMD.  Let's go with
     * PRE_CMD for now.
     */
    session.curr_phase = PRE_CMD;

    if (fxp->request_id) {
      pr_trace_msg(trace_channel, 6,
        "received %s (%d) SFTP request (request ID %lu, channel ID %lu)",
        fxp_get_request_type_desc(fxp->request_type), fxp->request_type,
        (unsigned long) fxp->request_id, (unsigned long) channel_id);

    } else {
      pr_trace_msg(trace_channel, 6,
        "received %s (%d) SFTP request (channel ID %lu)",
        fxp_get_request_type_desc(fxp->request_type), fxp->request_type,
        (unsigned long) channel_id);
    }

    fxp_session = fxp_get_session(channel_id);
    if (fxp_session == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "no existing SFTP session for channel ID %lu, rejecting request",
        (unsigned long) channel_id);
      destroy_pool(fxp->pool);
      return -1;
    }

    pr_response_set_pool(fxp->pool);

    switch (fxp->request_type) {
      case SFTP_SSH2_FXP_INIT:
        /* If we already know the version, then the client has sent
         * FXP_INIT before, and should NOT be sending it again.
         */
        if (fxp_session->client_version == 0) {
          res = fxp_handle_init(fxp);

        } else {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "already received SFTP INIT request from client");
          destroy_pool(fxp->pool);
          fxp_session = NULL;
          return -1;
        }

        break;

      case SFTP_SSH2_FXP_CLOSE:
        allow_version_select = FALSE;
        res = fxp_handle_close(fxp);
        break;

      case SFTP_SSH2_FXP_EXTENDED:
        res = fxp_handle_extended(fxp);
        break;

      case SFTP_SSH2_FXP_FSETSTAT:
        allow_version_select = FALSE;
        res = fxp_handle_fsetstat(fxp);
        break;

      case SFTP_SSH2_FXP_FSTAT:
        allow_version_select = FALSE;
        res = fxp_handle_fstat(fxp);
        break;

      case SFTP_SSH2_FXP_LINK:
        allow_version_select = FALSE;
        res = fxp_handle_link(fxp);
        break;

      case SFTP_SSH2_FXP_LOCK:
        allow_version_select = FALSE;
        res = fxp_handle_lock(fxp);
        break;

      case SFTP_SSH2_FXP_LSTAT:
        allow_version_select = FALSE;
        res = fxp_handle_lstat(fxp);
        break;

      case SFTP_SSH2_FXP_MKDIR:
        allow_version_select = FALSE;
        res = fxp_handle_mkdir(fxp);
        break;

      case SFTP_SSH2_FXP_OPEN:
        allow_version_select = FALSE;
        res = fxp_handle_open(fxp);
        break;

      case SFTP_SSH2_FXP_OPENDIR:
        allow_version_select = FALSE;
        res = fxp_handle_opendir(fxp);
        break;

      case SFTP_SSH2_FXP_READ:
        allow_version_select = FALSE;
        res = fxp_handle_read(fxp);
        break;

      case SFTP_SSH2_FXP_READDIR:
        allow_version_select = FALSE;
        res = fxp_handle_readdir(fxp);
        break;

      case SFTP_SSH2_FXP_READLINK:
        allow_version_select = FALSE;
        res = fxp_handle_readlink(fxp);
        break;

      case SFTP_SSH2_FXP_REALPATH:
        allow_version_select = FALSE;
        res = fxp_handle_realpath(fxp);
        break;

      case SFTP_SSH2_FXP_REMOVE:
        allow_version_select = FALSE;
        res = fxp_handle_remove(fxp);
        break;

      case SFTP_SSH2_FXP_RENAME:
        allow_version_select = FALSE;
        res = fxp_handle_rename(fxp);
        break;

      case SFTP_SSH2_FXP_RMDIR:
        allow_version_select = FALSE;
        res = fxp_handle_rmdir(fxp);
        break;

      case SFTP_SSH2_FXP_SETSTAT:
        allow_version_select = FALSE;
        res = fxp_handle_setstat(fxp);
        break;

      case SFTP_SSH2_FXP_STAT:
        allow_version_select = FALSE;
        res = fxp_handle_stat(fxp);
        break;

      case SFTP_SSH2_FXP_SYMLINK:
        allow_version_select = FALSE;
        res = fxp_handle_symlink(fxp);
        break;

      case SFTP_SSH2_FXP_WRITE:
        allow_version_select = FALSE;
        res = fxp_handle_write(fxp);
        break;

      case SFTP_SSH2_FXP_UNLOCK:
        allow_version_select = FALSE;
        res = fxp_handle_unlock(fxp);
        break;

      default:
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "unhandled SFTP request type %d", fxp->request_type);
        destroy_pool(fxp->pool);
        fxp_session = NULL;
        return -1;
    }

    destroy_pool(fxp->pool);

    if (res < 0) {
      fxp_session = NULL;
      return res;
    }

    if (have_cache) {
      fxp = fxp_packet_read(channel_id, NULL, NULL, &have_cache);
      continue;
    }

    fxp_session = NULL;
    return res;
  }

  fxp_session = NULL;
  return 0;
}

int sftp_fxp_set_extensions(unsigned long ext_flags) {
  fxp_ext_flags = ext_flags;
  return 0;
}

int sftp_fxp_set_protocol_version(unsigned int min_version,
    unsigned int max_version) {
  if ((min_version < 1 || min_version > 6) ||
      (max_version < 1 || max_version > 6)) {
    errno = EINVAL;
    return -1;
  }

  if (min_version > max_version) {
    errno = EINVAL;
    return -1;
  }

  fxp_min_client_version = min_version;
  fxp_max_client_version = max_version;
  return 0;
}

int sftp_fxp_set_utf8_protocol_version(unsigned int version) {
  if (version < 1 || version > 6) {
    errno = EINVAL;
    return -1;
  }

  fxp_utf8_protocol_version = version;
  return 0;
}

void sftp_fxp_use_gmt(int use_gmt) {
  fxp_use_gmt = use_gmt;
}

int sftp_fxp_open_session(uint32_t channel_id) {
  pool *sub_pool;
  struct fxp_session *sess, *last;

  /* Check to see if we already have an SFTP session opened for the given
   * channel ID.
   */
  sess = last = fxp_sessions;
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

  /* Looks like we get to allocate a new one. */
  sub_pool = make_sub_pool(fxp_pool);
  pr_pool_tag(sub_pool, "SFTP session pool");

  sess = pcalloc(sub_pool, sizeof(struct fxp_session));
  sess->pool = sub_pool;
  sess->channel_id = channel_id;

  if (last) {
    last->next = sess;
    sess->prev = last;

  } else {
    fxp_sessions = sess;
  }

  pr_session_set_protocol("sftp");
  return 0;
}

int sftp_fxp_close_session(uint32_t channel_id) {
  struct fxp_session *sess;

  /* Check to see if we have an SFTP session opened for the given channel ID.
   */
  sess = fxp_sessions;
  while (sess) {
    pr_signals_handle();

    if (sess->channel_id == channel_id) {

      if (sess->next)
        sess->next->prev = sess->prev;

      if (sess->prev) {
        sess->prev->next = sess->next;

      } else {
        /* This is the start of the session list. */
        fxp_sessions = sess->next;
      }

      if (sess->handle_tab) {
        int count;

        count = pr_table_count(sess->handle_tab);
        if (count > 0) {
          int res;
          config_rec *c;
          void *callback_data = NULL;

          c = find_config(main_server->conf, CONF_PARAM, "DeleteAbortedStores",
            FALSE);
          if (c) {
            callback_data = c->argv[0];
          }

          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "aborting %d unclosed file %s", count,
            count != 1 ? "handles" : "handle");

          res = pr_table_do(sess->handle_tab, fxp_handle_abort, callback_data,
            PR_TABLE_DO_FL_ALL);
          if (res < 0) {
            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "error doing session filehandle table: %s", strerror(errno));
          }
        }

        (void) pr_table_empty(sess->handle_tab);
        (void) pr_table_free(sess->handle_tab);
        sess->handle_tab = NULL;
      }

      destroy_pool(sess->pool);

      pr_session_set_protocol("ssh2");
      return 0;
    }

    sess = sess->next;
  }

  errno = ENOENT;
  return -1;
}
