/*
 * ProFTPD - FTP server daemon
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
 * As a special exemption, The ProFTPD Project and other respective copyright
 * holders give permission to link this program with OpenSSL, and distribute
 * the resulting executable, without including the source code for OpenSSL in
 * the source distribution.
 */

/*
 * ProFTPD scoreboard support.
 *
 * $Id: scoreboard.c,v 1.52 2010/01/11 01:16:50 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"

#include <signal.h>

/* From src/dirtree.c */
extern char ServerType;

static pid_t scoreboard_opener = 0;
static int scoreboard_fd = -1;
static char scoreboard_file[PR_TUNABLE_PATH_MAX] = PR_RUN_DIR "/proftpd.scoreboard";

static off_t current_pos = 0;
static pr_scoreboard_header_t header;
static pr_scoreboard_entry_t entry;
static int have_entry = FALSE;
static struct flock entry_lock;

static unsigned char scoreboard_read_locked = FALSE;
static unsigned char scoreboard_write_locked = FALSE;

static const char *trace_channel = "scoreboard";

/* Internal routines */

static char *handle_score_str(const char *fmt, va_list cmdap) {
  static char buf[PR_TUNABLE_SCOREBOARD_BUFFER_SIZE] = {'\0'};
  memset(buf, '\0', sizeof(buf));
  vsnprintf(buf, sizeof(buf), fmt, cmdap);
  buf[sizeof(buf)-1] = '\0';
  return buf;
}

static int read_scoreboard_header(pr_scoreboard_header_t *sch) {
  int res = 0;

  pr_trace_msg(trace_channel, 7, "reading scoreboard header");

  /* NOTE: reading a struct from a file using read(2) -- bad (in general). */
  while ((res = read(scoreboard_fd, sch, sizeof(pr_scoreboard_header_t))) !=
      sizeof(pr_scoreboard_header_t)) {
    int rd_errno = errno;

    if (res == 0) {
      errno = EIO;
      return -1;
    }

    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    errno = rd_errno;
    return -1;
  }

  /* Note: these errors will most likely occur only for inetd-run daemons.
   * Standalone daemons erase the scoreboard on startup.
   */
 
  if (sch->sch_magic != PR_SCOREBOARD_MAGIC) {
    pr_close_scoreboard();
    return PR_SCORE_ERR_BAD_MAGIC;
  }

  if (sch->sch_version < PR_SCOREBOARD_VERSION) {
    pr_close_scoreboard();
    return PR_SCORE_ERR_OLDER_VERSION;
  }

  if (sch->sch_version > PR_SCOREBOARD_VERSION) {
    pr_close_scoreboard();
    return PR_SCORE_ERR_NEWER_VERSION;
  }

  return 0;
}

static const char *get_lock_type(struct flock *lock) {
  const char *lock_type;

  switch (lock->l_type) {
    case F_RDLCK:
      lock_type = "read";
      break;

    case F_WRLCK:
      lock_type = "write";
      break;

    case F_UNLCK:
      lock_type = "unlock";
      break;

    default:
      lock_type = "[unknown]";
  }

  return lock_type;
}

static int rlock_scoreboard(void) {
  struct flock lock;

  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;

  pr_trace_msg("lock", 9, "attempting to read-lock scoreboard fd %d",
    scoreboard_fd);

  while (fcntl(scoreboard_fd, F_SETLKW, &lock) < 0) {
    int xerrno = errno;

    if (xerrno == EINTR) {
      pr_signals_handle();
      continue;
    }

    pr_trace_msg("lock", 3, "read-lock of scoreboard fd %d failed: %s",
      scoreboard_fd, strerror(xerrno));
    if (xerrno == EACCES) {
      /* Get the PID of the process blocking this lock. */
      if (fcntl(scoreboard_fd, F_GETLK, &lock) == 0) {
        pr_trace_msg("lock", 3, "process ID %lu has blocking %s lock on "
          "scoreboard fd %d", (unsigned long) lock.l_pid, get_lock_type(&lock),
          scoreboard_fd);
      }
    }

    errno = xerrno;
    return -1;
  }

  pr_trace_msg("lock", 9, "read-lock of scoreboard fd %d successful",
    scoreboard_fd);

  scoreboard_read_locked = TRUE;
  return 0;
}

static int unlock_entry(void) {

  entry_lock.l_type = F_UNLCK;
  entry_lock.l_whence = SEEK_CUR;
  entry_lock.l_len = sizeof(pr_scoreboard_entry_t);

  pr_trace_msg("lock", 9, "attempting to unlock scoreboard fd %d entry, "
    "offset %" PR_LU, scoreboard_fd, (pr_off_t) entry_lock.l_start);

  while (fcntl(scoreboard_fd, F_SETLKW, &entry_lock) < 0) {
    int xerrno = errno;

    if (xerrno == EINTR) {
      pr_signals_handle();
      continue;
    }

    pr_trace_msg("lock", 3, "unlock of scoreboard fd %d entry failed: %s",
      scoreboard_fd, strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  pr_trace_msg("lock", 9, "unlock of scoreboard fd %d entry, "
    "offset %" PR_LU " succeeded", scoreboard_fd,
    (pr_off_t) entry_lock.l_start);

  return 0;
}

static int unlock_scoreboard(void) {
  struct flock lock;

  lock.l_type = F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;

  scoreboard_read_locked = scoreboard_write_locked = FALSE;

  pr_trace_msg("lock", 9, "attempting to unlock scoreboard fd %d",
    scoreboard_fd);

  while (fcntl(scoreboard_fd, F_SETLK, &lock) < 0) {
    int xerrno = errno;

    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    pr_trace_msg("lock", 3, "unlock of scoreboard fd %d failed: %s",
      scoreboard_fd, strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  pr_trace_msg("lock", 9, "unlock of scoreboard fd %d successful",
    scoreboard_fd);

  return 0;
}

static int wlock_entry(void) {
  entry_lock.l_type = F_WRLCK;
  entry_lock.l_whence = SEEK_CUR;
  entry_lock.l_len = sizeof(pr_scoreboard_entry_t);

  pr_trace_msg("lock", 9, "attempting to write-lock scoreboard fd %d entry, "
    "offset %" PR_LU, scoreboard_fd, (pr_off_t) entry_lock.l_start);

  while (fcntl(scoreboard_fd, F_SETLKW, &entry_lock) < 0) {
    int xerrno = errno;

    if (xerrno == EINTR) {
      pr_signals_handle();
      continue;
    }

    pr_trace_msg("lock", 3, "write-lock of scoreboard fd %d entry failed: %s",
      scoreboard_fd, strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  pr_trace_msg("lock", 9, "write-lock of scoreboard fd %d entry, "
    "offset %" PR_LU " succeeded", scoreboard_fd,
    (pr_off_t) entry_lock.l_start);

  return 0;
}

static int wlock_scoreboard(void) {
  struct flock lock;

  lock.l_type = F_WRLCK;
  lock.l_whence = 0;
  lock.l_start = 0;
  lock.l_len = 0;

  pr_trace_msg("lock", 9, "attempting to write-lock scoreboard fd %d",
    scoreboard_fd);

  while (fcntl(scoreboard_fd, F_SETLKW, &lock) < 0) {
    int xerrno = errno;

    if (xerrno == EINTR) {
      pr_signals_handle();
      continue;
    }

    pr_trace_msg("lock", 3, "write-lock of scoreboard fd %d failed: %s",
      scoreboard_fd, strerror(xerrno));
    if (xerrno == EACCES) {
      /* Get the PID of the process blocking this lock. */
      if (fcntl(scoreboard_fd, F_GETLK, &lock) == 0) {
        pr_trace_msg("lock", 3, "process ID %lu has blocking %s lock on "
          "scoreboard fd %d", (unsigned long) lock.l_pid, get_lock_type(&lock),
          scoreboard_fd);
      }
    }

    errno = xerrno;
    return -1;
  }

  pr_trace_msg("lock", 9, "write-lock of scoreboard fd %d successful",
    scoreboard_fd);

  scoreboard_write_locked = TRUE;
  return 0;
}

static int write_entry(void) {
  if (scoreboard_fd < 0) {
    errno = EINVAL;
    return -1;
  }

  lseek(scoreboard_fd, entry_lock.l_start, SEEK_SET);

  while (write(scoreboard_fd, &entry, sizeof(entry)) != sizeof(entry)) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    return -1;
  }

  /* Rewind. */
  lseek(scoreboard_fd, entry_lock.l_start, SEEK_SET);

  return 0;
}

/* Public routines */

int pr_close_scoreboard(void) {
  if (scoreboard_fd == -1)
    return 0;

  if (scoreboard_read_locked || scoreboard_write_locked)
    unlock_scoreboard();

  pr_trace_msg(trace_channel, 4, "closing scoreboard fd %d", scoreboard_fd);

  while (close(scoreboard_fd) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    break;
  }

  scoreboard_fd = -1;
  scoreboard_opener = 0;

  return 0;
}

void pr_delete_scoreboard(void) {
  if (scoreboard_fd > -1) {
    while (close(scoreboard_fd) < 0) {
      if (errno == EINTR) {
        pr_signals_handle();
        continue;
      }

      break;
    }
  }

  scoreboard_fd = -1;
  scoreboard_opener = 0;

  if (*scoreboard_file) {
    struct stat st;

    if (stat(scoreboard_file, &st) == 0) {
      pr_log_debug(DEBUG3, "deleting existing scoreboard '%s'",
        scoreboard_file);
    }

    (void) unlink(scoreboard_file);
  }
}

const char *pr_get_scoreboard(void) {
  return scoreboard_file;
}

int pr_open_scoreboard(int flags) {
  int res;
  struct stat st;

  if (flags != O_RDWR) {
    errno = EINVAL;
    return -1;
  }

  /* Try to prevent a file descriptor leak by only opening the scoreboard
   * file if the scoreboard file descriptor is not already positive, i.e.
   * if the scoreboard has not already been opened.
   */
  if (scoreboard_fd >= 0 &&
      scoreboard_opener == getpid()) {
    pr_log_debug(DEBUG7, "scoreboard already opened");
    return 0;
  }

  /* Check for symlinks prior to opening the file. */
  if (lstat(scoreboard_file, &st) == 0) {
    if (S_ISLNK(st.st_mode)) {
      scoreboard_fd = -1;
      errno = EPERM;
      return -1;
    }
  }

  pr_log_debug(DEBUG7, "opening scoreboard '%s'", scoreboard_file);

  while ((scoreboard_fd = open(scoreboard_file, flags|O_CREAT,
      PR_SCOREBOARD_MODE)) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    return -1;
  }

  /* Make certain that the scoreboard mode will be read-only for everyone
   * except the user owner (this allows for non-root-running daemons to
   * still modify the scoreboard).
   */
  while (fchmod(scoreboard_fd, 0644) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    break;
  }

  scoreboard_opener = getpid();

  /* Check the header of this scoreboard file. */
  res = read_scoreboard_header(&header);
  if (res == -1) {

    /* If this file is newly created, it needs to have the header
     * written.
     */
    header.sch_magic = PR_SCOREBOARD_MAGIC;
    header.sch_version = PR_SCOREBOARD_VERSION;

    if (ServerType == SERVER_STANDALONE) {
      header.sch_pid = getpid();
      header.sch_uptime = time(NULL);

    } else {
      header.sch_pid = 0;
      header.sch_uptime = 0;
    }

    /* Write-lock the scoreboard file. */
    if (wlock_scoreboard() < 0)
      return -1;

    pr_trace_msg(trace_channel, 7, "writing scoreboard header");

    while (write(scoreboard_fd, &header, sizeof(header)) != sizeof(header)) {
      int wr_errno = errno;

      if (errno == EINTR) {
        pr_signals_handle();
        continue;
      }

      unlock_scoreboard();

      errno = wr_errno;
      return -1;
    }

    unlock_scoreboard();
    return 0;

  } else
    return res;

  return 0;
}

int pr_restore_scoreboard(void) {

  if (scoreboard_fd < 0) {
    errno = EINVAL;
    return -1;
  }

  /* Position the file position pointer of the scoreboard back to
   * where it was, prior to the last pr_rewind_scoreboard() call.
   */
  lseek(scoreboard_fd, current_pos, SEEK_SET);
  return 0;
}

int pr_rewind_scoreboard(void) {

  if (scoreboard_fd < 0) {
    errno = EINVAL;
    return -1;
  }

  current_pos = lseek(scoreboard_fd, 0, SEEK_CUR);

  /* Position the file position pointer of the scoreboard at the
   * start of the scoreboard (past the header).
   */
  lseek(scoreboard_fd, sizeof(pr_scoreboard_header_t), SEEK_SET);
  return 0;
}

int pr_set_scoreboard(const char *path) {
  char dir[PR_TUNABLE_PATH_MAX] = {'\0'};
  struct stat st;
  char *tmp = NULL;

  if (path == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (*path != '/') {
    errno = EINVAL;
    return -1;
  }

  sstrncpy(dir, path, sizeof(dir));

  tmp = strrchr(dir + 1, '/');
  if (tmp == NULL) {
    errno = EINVAL;
    return -1;
  }

  *tmp = '\0';

  /* Check for the possibility that the '/' just found is at the end
   * of the given string.
   */
  if (*(tmp + 1) == '\0') {
    *tmp = '/';
    errno = EINVAL;
    return -1;
  }

  /* Parent directory must not be world-writable */

  if (stat(dir, &st) < 0)
    return -1;

  if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    return -1;
  }

  if (st.st_mode & S_IWOTH) {
    errno = EPERM;
    return -1;
  }

  sstrncpy(scoreboard_file, path, sizeof(scoreboard_file));
  return 0;
}

int pr_scoreboard_entry_add(void) {
  unsigned char found_slot = FALSE;

  if (scoreboard_fd < 0) {
    errno = EINVAL;
    return -1;
  }

  if (have_entry) {
    errno = EPERM;
    return -1;
  }

  pr_trace_msg(trace_channel, 3, "adding new scoreboard entry");

  /* Write-lock the scoreboard file. */
  if (wlock_scoreboard() < 0)
    return -1;

  /* No interruptions, please. */
  pr_signals_block();

  /* If the scoreboard is open, the file position is already past the
   * header.
   */
  while (TRUE) {
    int res = 0;
    while ((res = read(scoreboard_fd, &entry, sizeof(entry))) ==
        sizeof(entry)) {

      /* If this entry's PID is marked as zero, it means this slot can be
       * reused.
       */
      if (!entry.sce_pid) {
        entry_lock.l_start = lseek(scoreboard_fd, 0, SEEK_CUR) - sizeof(entry);
        found_slot = TRUE;
        break;
      }
    }

    if (res == 0) {
      entry_lock.l_start = lseek(scoreboard_fd, 0, SEEK_CUR);
      found_slot = TRUE;
    }

    if (found_slot)
      break;
  }

  memset(&entry, '\0', sizeof(entry));

  entry.sce_pid = session.pid ? session.pid : getpid();
  entry.sce_uid = geteuid();
  entry.sce_gid = getegid();

  have_entry = TRUE;

  if (write_entry() < 0)
    pr_log_pri(PR_LOG_NOTICE, "error writing scoreboard entry: %s",
      strerror(errno));

  pr_signals_unblock();

  /* We can unlock the scoreboard now. */
  unlock_scoreboard();

  return 0;
}

int pr_scoreboard_entry_del(unsigned char verbose) {

  if (scoreboard_fd < 0) {
    errno = EINVAL;
    return -1;
  }

  if (!have_entry) {
    errno = ENOENT;
    return -1;
  }

  pr_trace_msg(trace_channel, 3, "deleting scoreboard entry");

  memset(&entry, '\0', sizeof(entry));

  /* Write-lock this entry */
  wlock_entry();
  if (write_entry() < 0 &&
      verbose) {
    pr_log_pri(PR_LOG_NOTICE, "error deleting scoreboard entry: %s",
      strerror(errno));
  }

  have_entry = FALSE;
  unlock_entry();

  return 0;
}

pid_t pr_scoreboard_get_daemon_pid(void) {
  return header.sch_pid;
}

time_t pr_scoreboard_get_daemon_uptime(void) {
  return header.sch_uptime;
}

pr_scoreboard_entry_t *pr_scoreboard_entry_read(void) {
  static pr_scoreboard_entry_t scan_entry;
  int res = 0;

  if (scoreboard_fd < 0) {
    errno = EINVAL;
    return NULL;
  }

  /* Make sure the scoreboard file is read-locked. */
  if (!scoreboard_read_locked) {

    /* Do not proceed if we cannot lock the scoreboard. */
    if (rlock_scoreboard() < 0)
      return NULL; 
  }

  pr_trace_msg(trace_channel, 5, "reading scoreboard entry");

  memset(&scan_entry, '\0', sizeof(scan_entry));

  /* NOTE: use readv(2)? */
  while (TRUE) {
    while ((res = read(scoreboard_fd, &scan_entry, sizeof(scan_entry))) <= 0) {
      if (res < 0 && errno == EINTR) {
        pr_signals_handle();
        continue;

      } else {
        unlock_scoreboard();
        return NULL;
      }
    }

    if (scan_entry.sce_pid) {
      unlock_scoreboard();
      return &scan_entry;

    } else
      continue;
  }

  unlock_scoreboard();
  return NULL;
}

/* We get clever with the next functions, so that they can be used for
 * various entry attributes.
 */

const char *pr_scoreboard_entry_get(int field) {

  if (scoreboard_fd < 0) {
    errno = EINVAL;
    return NULL;
  }

  if (!have_entry) {
    errno = EPERM;
    return NULL;
  }

  switch (field) {
    case PR_SCORE_USER:
      return entry.sce_user;

    case PR_SCORE_CLIENT_ADDR:
      return entry.sce_client_addr;

    case PR_SCORE_CLIENT_NAME:
      return entry.sce_client_name;

    case PR_SCORE_CLASS:
      return entry.sce_class;

    case PR_SCORE_CWD:
      return entry.sce_cwd;

    case PR_SCORE_CMD:
      return entry.sce_cmd;

    case PR_SCORE_CMD_ARG:
      return entry.sce_cmd_arg;

    case PR_SCORE_PROTOCOL:
      return entry.sce_protocol;
  }

  errno = ENOENT;
  return NULL;
}

int pr_scoreboard_entry_kill(pr_scoreboard_entry_t *sce, int signo) {
  int res;

  if (sce == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (ServerType == SERVER_STANDALONE) {
#ifdef HAVE_GETPGID
    pid_t curr_pgrp;

# ifdef HAVE_GETPGRP
    curr_pgrp = getpgrp();
# else
    curr_pgrp = getpgid(0);
# endif /* HAVE_GETPGRP */

    if (getpgid(sce->sce_pid) != curr_pgrp) {
      pr_trace_msg(trace_channel, 1, "scoreboard entry PID %lu process group "
        "does not match current process group, refusing to send signal",
        (unsigned long) sce->sce_pid);
      errno = EPERM;
      return -1;
    }
#endif /* HAVE_GETPGID */
  }

  res = kill(sce->sce_pid, signo);
  return res;
}

/* Given a NUL-terminated string -- possibly UTF8-encoded -- and a maximum
 * buffer length, return the number of bytes in the string which can fit in
 * that buffer without truncating a character.  This is needed since UTF8
 * characters are variable-width.
 */
static size_t str_getlen(const char *str, size_t maxsz) {
#ifdef PR_USE_NLS
  register unsigned int i = 0;

  while (str[i] > 0 &&
         i < maxsz) {
ascii:
    pr_signals_handle();
    i++;
  }

  while (str[i] &&
         i < maxsz) {
    size_t len;

    if (str[i] > 0) {
      goto ascii;
    }

    pr_signals_handle();

    len = 0;

    switch (str[i] & 0xF0) {
      case 0xE0:
        len = 3;
        break;

      case 0xF0:
        len = 4;
        break;

      default:
        len = 2;
        break;
    }

    if ((i + len) < maxsz) {
      i += len;

    } else {
      break;
    }
  }

  return i;
#else
  /* No UTF8 support in this proftpd build; just return the max size. */
  return maxsz;
#endif /* !PR_USE_NLS */
}

int pr_scoreboard_entry_update(pid_t pid, ...) {
  va_list ap;
  char *tmp = NULL;
  int entry_tag = 0;

  if (scoreboard_fd < 0) {
    errno = EINVAL;
    return -1;
  }

  if (!have_entry) {
    errno = EPERM;
    return -1;
  }

  pr_trace_msg(trace_channel, 3, "updating scoreboard entry");

  /* If updating some fields, clear the begin_idle field.
   */

  va_start(ap, pid);

  while ((entry_tag = va_arg(ap, int)) != 0) {
    pr_signals_handle();

    switch (entry_tag) {
      case PR_SCORE_USER:
        tmp = va_arg(ap, char *);
        memset(entry.sce_user, '\0', sizeof(entry.sce_user));
        sstrncpy(entry.sce_user, tmp,
          str_getlen(tmp, sizeof(entry.sce_user)-1) + 1);
        break;

      case PR_SCORE_CLIENT_ADDR: {
          pr_netaddr_t *remote_addr = va_arg(ap, pr_netaddr_t *);

          snprintf(entry.sce_client_addr, sizeof(entry.sce_client_addr),
            "%s", remote_addr ? pr_netaddr_get_ipstr(remote_addr) :
            "(unknown)");
          entry.sce_client_addr[sizeof(entry.sce_client_addr) - 1] = '\0';
        }
        break;

      case PR_SCORE_CLIENT_NAME: {
          char *remote_name = va_arg(ap, char *);

          if (remote_name == NULL) {
            remote_name = "(unknown)";
          }

          memset(entry.sce_client_name, '\0', sizeof(entry.sce_client_name));

          snprintf(entry.sce_client_name,
            str_getlen(remote_name, sizeof(entry.sce_client_name)-1),
            "%s", remote_name);
          entry.sce_client_name[sizeof(entry.sce_client_name)-1] = '\0';
        }
        break;

      case PR_SCORE_CLASS:
        tmp = va_arg(ap, char *);
        memset(entry.sce_class, '\0', sizeof(entry.sce_class));
        sstrncpy(entry.sce_class, tmp, sizeof(entry.sce_class));
        break;

      case PR_SCORE_CWD:
        tmp = va_arg(ap, char *);
        memset(entry.sce_cwd, '\0', sizeof(entry.sce_cwd));
        sstrncpy(entry.sce_cwd, tmp,
          str_getlen(tmp, sizeof(entry.sce_cwd)-1) + 1);
        break;

      case PR_SCORE_CMD: {
          char *cmdstr = NULL;
          tmp = va_arg(ap, char *);
          cmdstr = handle_score_str(tmp, ap);

          memset(entry.sce_cmd, '\0', sizeof(entry.sce_cmd));
          sstrncpy(entry.sce_cmd, cmdstr, sizeof(entry.sce_cmd));
          tmp = va_arg(ap, void *);
        }
        break;

      case PR_SCORE_CMD_ARG: {
          char *argstr = NULL;
          tmp = va_arg(ap, char *);
          argstr = handle_score_str(tmp, ap);

          memset(entry.sce_cmd_arg, '\0', sizeof(entry.sce_cmd_arg));
          sstrncpy(entry.sce_cmd_arg, argstr,
            str_getlen(argstr, sizeof(entry.sce_cmd_arg)-1) + 1);
          tmp = va_arg(ap, void *);
        }
        break;

      case PR_SCORE_SERVER_PORT:
        entry.sce_server_port = va_arg(ap, int);
        break;

      case PR_SCORE_SERVER_ADDR: {
          pr_netaddr_t *server_addr = va_arg(ap, pr_netaddr_t *);
          int server_port = va_arg(ap, int);

          snprintf(entry.sce_server_addr, sizeof(entry.sce_server_addr),
            "%s:%d", server_addr ? pr_netaddr_get_ipstr(server_addr) :
            "(unknown)", server_port);
          entry.sce_server_addr[sizeof(entry.sce_server_addr)-1] = '\0';
        }
        break;

      case PR_SCORE_SERVER_LABEL:
        tmp = va_arg(ap, char *);
        memset(entry.sce_server_label, '\0', sizeof(entry.sce_server_label));
        sstrncpy(entry.sce_server_label, tmp, sizeof(entry.sce_server_label));
        break;

      case PR_SCORE_BEGIN_IDLE:
        /* Ignore this */
        (void) va_arg(ap, time_t);

        time(&entry.sce_begin_idle);
        break;

      case PR_SCORE_BEGIN_SESSION:
        /* Ignore this */
        (void) va_arg(ap, time_t);

        time(&entry.sce_begin_session);
        break;

      case PR_SCORE_XFER_DONE:
        entry.sce_xfer_done = va_arg(ap, off_t);
        break;

      case PR_SCORE_XFER_SIZE:
        entry.sce_xfer_size = va_arg(ap, off_t);
        break;

      case PR_SCORE_XFER_LEN:
        entry.sce_xfer_len = va_arg(ap, off_t);
        break;

      case PR_SCORE_XFER_ELAPSED:
        entry.sce_xfer_elapsed = va_arg(ap, unsigned long);
        break;

      case PR_SCORE_PROTOCOL:
        tmp = va_arg(ap, char *);
        memset(entry.sce_protocol, '\0', sizeof(entry.sce_protocol));
        sstrncpy(entry.sce_protocol, tmp, sizeof(entry.sce_protocol));
        break;

      default:
        errno = ENOENT;
        return -1;
    }
  }

  /* Write-lock this entry */
  wlock_entry();
  if (write_entry() < 0) {
    pr_log_pri(PR_LOG_NOTICE, "error writing scoreboard entry: %s",
      strerror(errno));
  }
  unlock_entry();

  return 0;
}

/* Validate the PID in a scoreboard entry.  A PID can be invalid in a couple
 * of ways:
 *
 *  1.  The PID refers to a process no longer present on the system.
 *  2.  The PID refers to a process not in the daemon process group
 *      (for "ServerType standalone" servers only).
 */
static int scoreboard_valid_pid(pid_t pid, pid_t curr_pgrp) {
  int res;

  res = kill(pid, 0);
  if (res < 0 &&
      errno == ESRCH) {
    return -1;
  }

  if (ServerType == SERVER_STANDALONE &&
      curr_pgrp > 0) {
#ifdef HAVE_GETPGID
    if (getpgid(pid) != curr_pgrp) { 
      pr_trace_msg(trace_channel, 1, "scoreboard entry PID %lu process group "
        "does not match current process group, removing entry",
        (unsigned long) pid);
      errno = EPERM;
      return -1;
    }
#endif /* HAVE_GETPGID */
  }

  return 0;
}

int pr_scoreboard_scrub(void) {
  int fd = -1;
  off_t curr_offset = 0;
  struct flock lock;
  pid_t curr_pgrp = 0;
  pr_scoreboard_entry_t sce;

  pr_log_debug(DEBUG9, "scrubbing scoreboard");

  /* Manually open the scoreboard.  It won't hurt if the process already
   * has a descriptor opened on the scoreboard file.
   */
  PRIVS_ROOT
  fd = open(pr_get_scoreboard(), O_RDWR);
  PRIVS_RELINQUISH

  if (fd < 0) {
    pr_log_debug(DEBUG1, "unable to scrub ScoreboardFile '%s': %s",
      pr_get_scoreboard(), strerror(errno));
    return -1;
  }

  /* Lock the entire scoreboard. */
  lock.l_type = F_WRLCK;
  lock.l_whence = 0;
  lock.l_start = 0;
  lock.l_len = 0;

  while (fcntl(fd, F_SETLKW, &lock) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    return -1;
  }

  /* Skip past the scoreboard header. */
  curr_offset = lseek(fd, sizeof(pr_scoreboard_header_t), SEEK_SET);

  memset(&sce, 0, sizeof(sce));
 
#ifdef HAVE_GETPGRP
  curr_pgrp = getpgrp();
#elif HAVE_GETPGID
  curr_pgrp = getpgid(0);
#endif /* !HAVE_GETPGRP and !HAVE_GETPGID */
 
  PRIVS_ROOT
  while (read(fd, &sce, sizeof(sce)) == sizeof(sce)) {
    pr_signals_handle();

    /* Check to see if the PID in this entry is valid.  If not, erase
     * the slot.
     */
    if (sce.sce_pid &&
        scoreboard_valid_pid(sce.sce_pid, curr_pgrp) < 0) {

      /* OK, the recorded PID is no longer valid. */
      pr_log_debug(DEBUG9, "scrubbing scoreboard slot for PID %u",
        (unsigned int) sce.sce_pid);

      /* Rewind to the start of this slot. */
      lseek(fd, curr_offset, SEEK_SET);

      memset(&sce, 0, sizeof(sce));
      while (write(fd, &sce, sizeof(sce)) != sizeof(sce)) {
        if (errno == EINTR) {
          pr_signals_handle();
          continue;
        }

        pr_log_debug(DEBUG0, "error scrubbing scoreboard: %s",
          strerror(errno));
      }
    }

    /* Mark the current offset. */
    curr_offset = lseek(fd, 0, SEEK_CUR);
  }
  PRIVS_RELINQUISH

  /* Release the scoreboard. */
  lock.l_type = F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;

  while (fcntl(fd, F_SETLK, &lock) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }
  }

  /* Don't need the descriptor anymore. */
  (void) close(fd);

  return 0;
}
