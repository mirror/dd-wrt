/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (C) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (C) 2001-2011 The ProFTPD Project
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

/* ProFTPD virtual/modular file-system support
 * $Id: fsio.c,v 1.102 2011/05/27 00:38:45 castaglia Exp $
 */

#include "conf.h"

#ifdef HAVE_SYS_STATVFS_H
# include <sys/statvfs.h>
#elif defined(HAVE_SYS_VFS_H)
# include <sys/vfs.h>
#elif defined(HAVE_SYS_MOUNT_H)
# include <sys/mount.h>
#endif

#ifdef AIX3
# include <sys/statfs.h>
#endif

#ifdef HAVE_ACL_LIBACL_H
# include <acl/libacl.h>
#endif

typedef struct fsopendir fsopendir_t;

struct fsopendir {
  fsopendir_t *next,*prev;

  /* pool for this object's use */
  pool *pool;

  pr_fs_t *fsdir;
  DIR *dir;
};

static const char *trace_channel = "fsio";
static pr_fs_t *root_fs = NULL, *fs_cwd = NULL;
static array_header *fs_map = NULL;

#ifdef PR_FS_MATCH
static pr_fs_match_t *fs_match_list = NULL;
#endif /* PR_FS_MATCH */

static fsopendir_t *fsopendir_list;

static void *fs_cache_dir = NULL;
static pr_fs_t *fs_cache_fsdir = NULL;

/* Internal flag set whenever a new pr_fs_t has been added or removed, and
 * cleared once the fs_map has been scanned
 */
static unsigned char chk_fs_map = FALSE;

/* Virtual working directory */
static char vwd[PR_TUNABLE_PATH_MAX + 1] = "/";
static char cwd[PR_TUNABLE_PATH_MAX + 1] = "/";

/* Runtime enabling/disabling of encoding of paths. */
static int use_encoding = TRUE;

/* The following static functions are simply wrappers for system functions
 */

static int sys_stat(pr_fs_t *fs, const char *path, struct stat *sbuf) {
  return stat(path, sbuf);
}

static int sys_fstat(pr_fh_t *fh, int fd, struct stat *sbuf) {
  return fstat(fd, sbuf);
}

static int sys_lstat(pr_fs_t *fs, const char *path, struct stat *sbuf) {
  return lstat(path, sbuf);
}

static int sys_rename(pr_fs_t *fs, const char *rnfm, const char *rnto) {
  return rename(rnfm, rnto);
}

static int sys_unlink(pr_fs_t *fs, const char *path) {
  return unlink(path);
}

static int sys_open(pr_fh_t *fh, const char *path, int flags) {

#ifdef O_BINARY
  /* On Cygwin systems, we need the open(2) equivalent of fopen(3)'s "b"
   * option.  Cygwin defines an O_BINARY flag for this purpose.
   */
  flags |= O_BINARY;
#endif

  return open(path, flags, PR_OPEN_MODE);
}

static int sys_creat(pr_fh_t *fh, const char *path, mode_t mode) {
  return creat(path, mode);
}

static int sys_close(pr_fh_t *fh, int fd) {
  return close(fd);
}

static int sys_read(pr_fh_t *fh, int fd, char *buf, size_t size) {
  return read(fd, buf, size);
}

static int sys_write(pr_fh_t *fh, int fd, const char *buf, size_t size) {
  return write(fd, buf, size);
}

static off_t sys_lseek(pr_fh_t *fh, int fd, off_t offset, int whence) {
  return lseek(fd, offset, whence);
}

static int sys_link(pr_fs_t *fs, const char *path1, const char *path2) {
  return link(path1, path2);
}

static int sys_symlink(pr_fs_t *fs, const char *path1, const char *path2) {
  return symlink(path1, path2);
}

static int sys_readlink(pr_fs_t *fs, const char *path, char *buf,
    size_t buflen) {
  return readlink(path, buf, buflen);
}

static int sys_ftruncate(pr_fh_t *fh, int fd, off_t len) {
  return ftruncate(fd, len);
}

static int sys_truncate(pr_fs_t *fs, const char *path, off_t len) {
  return truncate(path, len);
}

static int sys_chmod(pr_fs_t *fs, const char *path, mode_t mode) {
  return chmod(path, mode);
}

static int sys_fchmod(pr_fh_t *fh, int fd, mode_t mode) {
  return fchmod(fd, mode);
}

static int sys_chown(pr_fs_t *fs, const char *path, uid_t uid, gid_t gid) {
  return chown(path, uid, gid);
}

static int sys_fchown(pr_fh_t *fh, int fd, uid_t uid, gid_t gid) {
  return fchown(fd, uid, gid);
}

/* We provide our own equivalent of access(2) here, rather than using
 * access(2) directly, because access(2) uses the real IDs, rather than
 * the effective IDs, of the process.
 */
static int sys_access(pr_fs_t *fs, const char *path, int mode, uid_t uid,
    gid_t gid, array_header *suppl_gids) {
  mode_t mask;
  struct stat st;

  pr_fs_clear_cache();
  if (pr_fsio_stat(path, &st) < 0)
    return -1;

  /* Root always succeeds. */
  if (uid == PR_ROOT_UID)
    return 0;

  /* Initialize mask to reflect the permission bits that are applicable for
   * the given user. mask contains the user-bits if the user ID equals the
   * ID of the file owner. mask contains the group bits if the group ID
   * belongs to the group of the file. mask will always contain the other
   * bits of the permission bits.
   */
  mask = S_IROTH|S_IWOTH|S_IXOTH;

  if (st.st_uid == uid)
    mask |= S_IRUSR|S_IWUSR|S_IXUSR;

  /* Check the current group, as well as all supplementary groups.
   * Fortunately, we have this information cached, so accessing it is
   * almost free.
   */
  if (st.st_gid == gid) {
    mask |= S_IRGRP|S_IWGRP|S_IXGRP;

  } else {
    if (suppl_gids) {
      register unsigned int i = 0;

      for (i = 0; i < suppl_gids->nelts; i++) {
        if (st.st_gid == ((gid_t *) suppl_gids->elts)[i]) {
          mask |= S_IRGRP|S_IWGRP|S_IXGRP;
          break;
        }
      }
    }
  }

  mask &= st.st_mode;

  /* Perform requested access checks. */
  if (mode & R_OK) {
    if (!(mask & (S_IRUSR|S_IRGRP|S_IROTH))) {
      errno = EACCES;
      return -1;
    }
  }

  if (mode & W_OK) {
    if (!(mask & (S_IWUSR|S_IWGRP|S_IWOTH))) {
      errno = EACCES;
      return -1;
    }
  }

  if (mode & X_OK) {
    if (!(mask & (S_IXUSR|S_IXGRP|S_IXOTH))) {
      errno = EACCES;
      return -1;
    }
  }

  /* F_OK already checked by checking the return value of stat. */
  return 0;
}

static int sys_faccess(pr_fh_t *fh, int mode, uid_t uid, gid_t gid,
    array_header *suppl_gids) {
  return sys_access(fh->fh_fs, fh->fh_path, mode, uid, gid, suppl_gids);
}

static int sys_utimes(pr_fs_t *fs, const char *path, struct timeval *tvs) {
  return utimes(path, tvs);
}

static int sys_futimes(pr_fh_t *fh, int fd, struct timeval *tvs) {
#ifdef HAVE_FUTIMES
  int res;

  /* Check for an ENOSYS errno; if so, fallback to using sys_utimes.  Some
   * platforms will provide a futimes(2) stub which does not actually do
   * anything.
   */
  res = futimes(fd, tvs);
  if (res < 0 &&
      errno == ENOSYS) {
    return sys_utimes(fh->fh_fs, fh->fh_path, tvs);
  }

  return res;
#else
  return sys_utimes(fh->fh_fs, fh->fh_path, tvs);
#endif
}

static int sys_chroot(pr_fs_t *fs, const char *path) {
  if (chroot(path) < 0)
    return -1;

  session.chroot_path = (char *) path;
  return 0;
}

static int sys_chdir(pr_fs_t *fs, const char *path) {
  if (chdir(path) < 0)
    return -1;

  pr_fs_setcwd(path);
  return 0;
}

static void *sys_opendir(pr_fs_t *fs, const char *path) {
  return opendir(path);
}

static int sys_closedir(pr_fs_t *fs, void *dir) {
  return closedir((DIR *) dir);
}

static struct dirent *sys_readdir(pr_fs_t *fs, void *dir) {
  return readdir((DIR *) dir);
}

static int sys_mkdir(pr_fs_t *fs, const char *path, mode_t mode) {
  return mkdir(path, mode);
}

static int sys_rmdir(pr_fs_t *fs, const char *path) {
  return rmdir(path);
}

static int fs_cmp(const void *a, const void *b) {
  pr_fs_t *fsa, *fsb;

  fsa = *((pr_fs_t **) a);
  fsb = *((pr_fs_t **) b);

  return strcmp(fsa->fs_path, fsb->fs_path);
}

/* Statcache stuff */
typedef struct {
  char sc_path[PR_TUNABLE_PATH_MAX+1];
  struct stat sc_stat;
  int sc_errno;
  int sc_retval;

} fs_statcache_t;

static fs_statcache_t statcache;

#define fs_cache_lstat(f, p, s) cache_stat((f), (p), (s), FSIO_FILE_LSTAT)
#define fs_cache_stat(f, p, s) cache_stat((f), (p), (s), FSIO_FILE_STAT)

static int cache_stat(pr_fs_t *fs, const char *path, struct stat *sbuf,
    unsigned int op) {
  int res = -1;
  char pathbuf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  int (*mystat)(pr_fs_t *, const char *, struct stat *) = NULL;

  /* Sanity checks */
  if (!fs) {
    errno = EINVAL;
    return -1;
  }

  if (!path) {
    errno = ENOENT;
    return -1;
  }

  /* Use only absolute path names.  Construct them, if given a relative
   * path, based on cwd.  This obviates the need for something like
   * realpath(3), which only introduces more stat system calls.
   */
  if (*path != '/') {
    sstrcat(pathbuf, cwd, sizeof(pathbuf)-1);

    /* If the cwd is "/", we don't need to duplicate the path separator. 
     * On some systems (e.g. Cygwin), this duplication can cause problems,
     * as the path may then have different semantics.
     */
    if (strncmp(cwd, "/", 2) != 0) {
      sstrcat(pathbuf, "/", sizeof(pathbuf)-1);
    }

    sstrcat(pathbuf, path, sizeof(pathbuf)-1);

  } else
    sstrncpy(pathbuf, path, sizeof(pathbuf)-1);

  /* Determine which filesystem function to use, stat() or lstat() */
  if (op == FSIO_FILE_STAT) {
    mystat = fs->stat ? fs->stat : sys_stat;

  } else {
    mystat = fs->lstat ? fs->lstat : sys_lstat;
  }

  /* Can the last cached stat be used? */
  if (strcmp(pathbuf, statcache.sc_path) == 0) {

    /* Update the given struct stat pointer with the cached info */
    memcpy(sbuf, &statcache.sc_stat, sizeof(struct stat));

    /* Use the cached errno as well */
    errno = statcache.sc_errno;

    return statcache.sc_retval;
  }

  res = mystat(fs, pathbuf, sbuf);

  /* Update the cache */
  memset(statcache.sc_path, '\0', sizeof(statcache.sc_path));
  sstrncpy(statcache.sc_path, pathbuf, sizeof(statcache.sc_path));
  memcpy(&statcache.sc_stat, sbuf, sizeof(struct stat));
  statcache.sc_errno = errno;
  statcache.sc_retval = res;

  return res;
}

/* Lookup routines */

/* Necessary prototype for static function */
static pr_fs_t *lookup_file_canon_fs(const char *, char **, int);

/* lookup_dir_fs() is called when we want to perform some sort of directory
 * operation on a directory or file.  A "closest" match algorithm is used.  If
 * the lookup fails or is not "close enough" (i.e. the final target does not
 * exactly match an existing filesystem handle) scan the list of fs_matches for
 * matchable targets and call any callback functions, then rescan the pr_fs_t
 * list.  The rescan is performed in case any modules registered pr_fs_ts
 * during the hit.
 */
static pr_fs_t *lookup_dir_fs(const char *path, int op) {
  char buf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  char tmp_path[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  pr_fs_t *fs = NULL;
  int exact = FALSE;
  size_t tmp_pathlen = 0;

#ifdef PR_FS_MATCH
  pr_fs_match_t *fsm = NULL;
#endif /* PR_FS_MATCH */

  sstrncpy(buf, path, sizeof(buf));

  /* Check if the given path is an absolute path.  Since there may be
   * alternate fs roots, this is not a simple check.  If the path is
   * not absolute, prepend the current location.
   */
  if (pr_fs_valid_path(path) < 0) {
    if (pr_fs_dircat(tmp_path, sizeof(tmp_path), cwd, buf) < 0) {
      return NULL;
    }

  } else {
    sstrncpy(tmp_path, buf, sizeof(tmp_path));
  }

  /* Make sure that if this is a directory operation, the path being
   * search ends in a trailing slash -- this is how files and directories
   * are differentiated in the fs_map.
   */
  tmp_pathlen = strlen(tmp_path);
  if ((FSIO_DIR_COMMON & op) &&
      tmp_pathlen > 0 &&
      tmp_pathlen < sizeof(tmp_path) &&
      tmp_path[tmp_pathlen - 1] != '/') {
    sstrcat(tmp_path, "/", sizeof(tmp_path));
  }

  fs = pr_get_fs(tmp_path, &exact);

#ifdef PR_FS_MATCH
/* NOTE: what if there is a perfect matching pr_fs_t for the given path,
 *  but an fs_match with pattern of "." is registered?  At present, that
 *  fs_match will never trigger...hmmm...OK.  fs_matches are only scanned
 *  if and only if there is *not* an exactly matching pr_fs_t.
 *
 *  NOTE: this is experimental code, not yet ready for module consumption.
 *  It was present in the older FS code, hence it's presence now.
 */

  /* Is the returned pr_fs_t "close enough"? */
  if (!fs || !exact) {

    /* Look for an fs_match */
    fsm = pr_get_fs_match(tmp_path, op);

    while (fsm) {

      /* Invoke the fs_match's callback function, if set
       *
       * NOTE: what pr_fs_t is being passed to the trigger??
       */
      if (fsm->trigger) {
        if (fsm->trigger(fs, tmp_path, op) <= 0)
          pr_log_pri(PR_LOG_DEBUG, "error: fs_match '%s' trigger failed",
            fsm->name);
      }

      /* Get the next matching fs_match */
      fsm = pr_get_next_fs_match(fsm, tmp_path, op);
    }
  }

  /* Now, check for a new pr_fs_t, if any were registered by fs_match
   * callbacks.  This time, it doesn't matter if it's an exact match --
   * any pr_fs_t will do.
   */
  if (chk_fs_map)
    fs = pr_get_fs(tmp_path, &exact);
#endif /* PR_FS_MATCH */

  return (fs ? fs : root_fs);
}

/* lookup_file_fs() performs the same function as lookup_dir_fs, however
 * because we are performing a file lookup, the target is the subdirectory
 * _containing_ the actual target.  A basic optimization is used here,
 * if the path contains no '/' characters, fs_cwd is returned.
 */
static pr_fs_t *lookup_file_fs(const char *path, char **deref, int op) {

  if (!strchr(path, '/')) {
#ifdef PR_FS_MATCH
    pr_fs_match_t *fsm = NULL;

    fsm = pr_get_fs_match(path, op);

    if (!fsm || fsm->trigger(fs_cwd, path, op) <= 0) {
#else
    if (1) {
#endif /* PR_FS_MATCH */
      pr_fs_t *fs = fs_cwd;
      struct stat sbuf;
      int (*mystat)(pr_fs_t *, const char *, struct stat *) = NULL;

      /* Determine which function to use, stat() or lstat(). */
      if (op == FSIO_FILE_STAT) {
        while (fs && fs->fs_next && !fs->stat)
          fs = fs->fs_next;

        mystat = fs->stat;

      } else {
        while (fs && fs->fs_next && !fs->lstat)
          fs = fs->fs_next;

        mystat = fs->lstat;
      }

      if (mystat(fs, path, &sbuf) == -1 || !S_ISLNK(sbuf.st_mode))
        return fs;

    } else {

      /* The given path is a symbolic link, in which case we need to find
       * the actual path referenced, and return an pr_fs_t for _that_ path
       */
      char linkbuf[PR_TUNABLE_PATH_MAX + 1];
      int i;

      /* Three characters are reserved at the end of linkbuf for some path
       * characters (and a trailing NUL).
       */
      i = pr_fsio_readlink(path, &linkbuf[2], sizeof(linkbuf)-3);
      if (i != -1) {
        linkbuf[i] = '\0';
        if (strchr(linkbuf, '/') == NULL) {
          if (i + 3 > PR_TUNABLE_PATH_MAX)
            i = PR_TUNABLE_PATH_MAX - 3;

          memmove(&linkbuf[2], linkbuf, i + 1);

          linkbuf[i+2] = '\0';
          linkbuf[0] = '.';
          linkbuf[1] = '/';
          return lookup_file_canon_fs(linkbuf, deref, op);
        }
      }

      /* What happens if fs_cwd->readlink is NULL, or readlink() returns -1?
       * I guess, for now, we punt, and return fs_cwd.
       */
      return fs_cwd;
    }
  }

  return lookup_dir_fs(path, op);
}

static pr_fs_t *lookup_file_canon_fs(const char *path, char **deref, int op) {
  static char workpath[PR_TUNABLE_PATH_MAX + 1];

  memset(workpath,'\0',sizeof(workpath));

  if (pr_fs_resolve_partial(path, workpath, sizeof(workpath)-1,
      FSIO_FILE_OPEN) == -1) {
    if (*path == '/' || *path == '~') {
      if (pr_fs_interpolate(path, workpath, sizeof(workpath)-1) != -1)
        sstrncpy(workpath, path, sizeof(workpath));

    } else {
      if (pr_fs_dircat(workpath, sizeof(workpath), cwd, path) < 0)
        return NULL;
    }
  }

  if (deref)
    *deref = workpath;

  return lookup_file_fs(workpath, deref, op);
}

/* FS functions proper */

void pr_fs_clear_cache(void) {
  memset(&statcache, '\0', sizeof(statcache));
}

int pr_fs_copy_file(const char *src, const char *dst) {
  pr_fh_t *src_fh, *dst_fh;
  struct stat src_st, dst_st;
  char *buf;
  size_t bufsz;
  int dst_existed = FALSE, res;

  if (src == NULL ||
      dst == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* Use a nonblocking open() for the path; it could be a FIFO, and we don't
   * want to block forever if the other end of the FIFO is not running.
   */
  src_fh = pr_fsio_open(src, O_RDONLY|O_NONBLOCK);
  if (src_fh == NULL) {
    int xerrno = errno;

    pr_log_pri(PR_LOG_WARNING, "error opening source file '%s' "
      "for copying: %s", src, strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  pr_fsio_set_block(src_fh);

  /* Do not allow copying of directories. open(2) may not fail when
   * opening the source path, since it is only doing a read-only open,
   * which does work on directories.
   */

  /* This should never fail. */
  (void) pr_fsio_fstat(src_fh, &src_st);
  if (S_ISDIR(src_st.st_mode)) {
    int xerrno = EISDIR;

    pr_fsio_close(src_fh);

    pr_log_pri(PR_LOG_WARNING, "warning: cannot copy source '%s': %s", src,
      strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  /* We use stat() here, not lstat(), since open() would follow a symlink
   * to its target, and what we really want to know here is whether the
   * ultimate destination file exists or not.
   */
  if (pr_fsio_stat(dst, &dst_st) == 0) {
    dst_existed = TRUE;
    pr_fs_clear_cache();
  }

  /* Use a nonblocking open() for the path; it could be a FIFO, and we don't
   * want to block forever if the other end of the FIFO is not running.
   */
  dst_fh = pr_fsio_open(dst, O_WRONLY|O_CREAT|O_NONBLOCK);
  if (dst_fh == NULL) {
    int xerrno = errno;

    pr_fsio_close(src_fh);

    pr_log_pri(PR_LOG_WARNING, "error opening destination file '%s' "
      "for copying: %s", dst, strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  pr_fsio_set_block(dst_fh);

  /* Stat the source file to find its optimal copy block size. */
  if (pr_fsio_fstat(src_fh, &src_st) < 0) {
    int xerrno = errno;

    pr_log_pri(PR_LOG_WARNING, "error checking source file '%s' "
      "for copying: %s", src, strerror(xerrno));

    pr_fsio_close(src_fh);
    pr_fsio_close(dst_fh);

    if (!dst_existed) {
      /* Don't unlink the destination file if it already existed. */
      pr_fsio_unlink(dst);
    }

    errno = xerrno;
    return -1;
  }

  if (pr_fsio_fstat(dst_fh, &dst_st) == 0) {

    /* Check to see if the source and destination paths are identical.
     * We wait until now, rather than simply comparing the path strings
     * earlier, in order to do stats on the paths and compare things like
     * file size, mtime, inode, etc.
     */

    if (strcmp(src, dst) == 0 &&
        src_st.st_dev == dst_st.st_dev &&
        src_st.st_ino == dst_st.st_ino &&
        src_st.st_size == dst_st.st_size &&
        src_st.st_mtime == dst_st.st_mtime) {

      pr_fsio_close(src_fh);
      pr_fsio_close(dst_fh);

      /* No need to copy the same file. */
      return 0;
    }
  }

  bufsz = src_st.st_blksize;
  buf = malloc(bufsz);
  if (buf == NULL) {
    pr_log_pri(PR_LOG_CRIT, "Out of memory!");
    exit(1);
  }

#ifdef S_ISFIFO
  if (!S_ISFIFO(dst_st.st_mode)) {
    /* Make sure the destination file starts with a zero size. */
    pr_fsio_truncate(dst, 0);
  }
#endif

  while ((res = pr_fsio_read(src_fh, buf, bufsz)) > 0) {
    size_t datalen;
    off_t offset;

    pr_signals_handle();

    /* Be sure to handle short writes. */
    datalen = res;
    offset = 0;

    while (datalen > 0) {
      res = pr_fsio_write(dst_fh, buf + offset, datalen);
      if (res < 0) {
        int xerrno = errno;

        if (xerrno == EINTR ||
            xerrno == EAGAIN) {
          pr_signals_handle();
          continue;
        }

        pr_fsio_close(src_fh);
        pr_fsio_close(dst_fh);

        if (!dst_existed) {
          /* Don't unlink the destination file if it already existed. */
          pr_fsio_unlink(dst);
        }

        pr_log_pri(PR_LOG_WARNING, "error copying to '%s': %s", dst,
          strerror(xerrno));
        free(buf);

        errno = xerrno;
        return -1;
      }

      if (res == datalen) {
        break;
      }

      offset += res;
      datalen -= res;
    }
  }

  free(buf);

#if defined(HAVE_POSIX_ACL) && defined(PR_USE_FACL)
  {
    /* Copy any ACLs from the source file to the destination file as well. */
# if defined(HAVE_BSD_POSIX_ACL)
    acl_t facl, facl_dup = NULL;
    int have_facl = FALSE, have_dup = FALSE;

    facl = acl_get_fd(PR_FH_FD(src_fh));
    if (facl)
      have_facl = TRUE;

    if (have_facl)
        facl_dup = acl_dup(facl);

    if (facl_dup)
      have_dup = TRUE;

    if (have_dup &&
        acl_set_fd(PR_FH_FD(dst_fh), facl_dup) < 0)
      pr_log_debug(DEBUG3, "error applying ACL to destination file: %s",
        strerror(errno));

    if (have_dup)
      acl_free(facl_dup);

# elif defined(HAVE_LINUX_POSIX_ACL)

#  if defined(HAVE_PERM_COPY_FD)
    /* Linux provides the handy perm_copy_fd(3) function in its libacl
     * library just for this purpose.
     */
    if (perm_copy_fd(src, PR_FH_FD(src_fh), dst, PR_FH_FD(dst_fh), NULL) < 0) {
      pr_log_debug(DEBUG3, "error copying ACL to destination file: %s",
        strerror(errno));
    }

#  else
    acl_t src_acl = acl_get_fd(PR_FH_FD(src_fh));
    if (src_acl == NULL) {
      pr_log_debug(DEBUG3, "error obtaining ACL for fd %d: %s",
        PR_FH_FD(src_fh), strerror(errno));

    } else {
      if (acl_set_fd(PR_FH_FD(dst_fh), src_acl) < 0) {
        pr_log_debug(DEBUG3, "error setting ACL on fd %d: %s",
          PR_FH_FD(dst_fh), strerror(errno));

      } else {
        acl_free(src_acl);
      }
    }

#  endif /* !HAVE_PERM_COPY_FD */

# elif defined(HAVE_SOLARIS_POSIX_ACL)
    int nents;

    nents = facl(PR_FH_FD(src_fh), GETACLCNT, 0, NULL);
    if (nents < 0) {
      pr_log_debug(DEBUG3, "error getting source file ACL count: %s",
        strerror(errno));

    } else {
      aclent_t *acls;

      acls = malloc(sizeof(aclent_t) * nents);
      if (!acls) {
        pr_log_pri(PR_LOG_CRIT, "Out of memory!");
        exit(1);
      }

      if (facl(PR_FH_FD(src_fh), GETACL, nents, acls) < 0) {
        pr_log_debug(DEBUG3, "error getting source file ACLs: %s",
          strerror(errno));

      } else {
        if (facl(PR_FH_FD(dst_fh), SETACL, nents, acls) < 0) {
          pr_log_debug(DEBUG3, "error setting dest file ACLs: %s",
            strerror(errno));
        }
      }

      free(acls);
    }
# endif /* HAVE_SOLARIS_POSIX_ACL && PR_USE_FACL */
  }
#endif /* HAVE_POSIX_ACL */

  pr_fsio_close(src_fh);

  res = pr_fsio_close(dst_fh);
  if (res < 0) {
    int xerrno = errno;

    pr_log_pri(PR_LOG_WARNING, "error closing '%s': %s", dst,
      strerror(xerrno));

    errno = xerrno;
  }

  return res;
}

pr_fs_t *pr_register_fs(pool *p, const char *name, const char *path) {
  pr_fs_t *fs = NULL;

  /* Sanity check */
  if (!p || !name || !path) {
    errno = EINVAL;
    return NULL;
  }

  /* Instantiate an pr_fs_t */
  fs = pr_create_fs(p, name);
  if (fs != NULL) {

    /* Call pr_insert_fs() from here */
    if (!pr_insert_fs(fs, path)) {
      pr_trace_msg(trace_channel, 4, "error inserting FS '%s' at path '%s'",
        name, path);

      destroy_pool(fs->fs_pool);
      return NULL;
    }

  } else
    pr_trace_msg(trace_channel, 6, "error creating FS '%s'", name);

  return fs;
}

pr_fs_t *pr_create_fs(pool *p, const char *name) {
  pr_fs_t *fs = NULL;
  pool *fs_pool = NULL;

  /* Sanity check */
  if (!p || !name) {
    errno = EINVAL;
    return NULL;
  }

  /* Allocate a subpool, then allocate an pr_fs_t object from that subpool */
  fs_pool = make_sub_pool(p);
  pr_pool_tag(fs_pool, "FS Pool");

  fs = pcalloc(fs_pool, sizeof(pr_fs_t));
  if (!fs)
    return NULL;

  fs->fs_pool = fs_pool;
  fs->fs_next = fs->fs_prev = NULL;
  fs->fs_name = pstrdup(fs->fs_pool, name);
  fs->fs_next = root_fs;
  fs->allow_xdev_link = TRUE;
  fs->allow_xdev_rename = TRUE;

  /* This is NULL until set by pr_insert_fs() */
  fs->fs_path = NULL;

  return fs;
}

int pr_insert_fs(pr_fs_t *fs, const char *path) {
  char cleaned_path[PR_TUNABLE_PATH_MAX] = {'\0'};

  if (!fs_map) {
    pool *map_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(map_pool, "FSIO Map Pool");

    fs_map = make_array(map_pool, 0, sizeof(pr_fs_t *));
  }

  /* Clean the path, but only if it starts with a '/'.  Non-local-filesystem
   * paths may not want/need to be cleaned.
   */
  if (*path == '/') {
    pr_fs_clean_path(path, cleaned_path, sizeof(cleaned_path));

    /* Cleaning the path may have removed a trailing slash, which the
     * caller may actually have wanted.  Make sure one is present in
     * the cleaned version, if it was present in the original version and
     * is not present in the cleaned version.
     */
    if (path[strlen(path)-1] == '/') {
      size_t len = strlen(cleaned_path);

      if (len > 1 &&
          len < (PR_TUNABLE_PATH_MAX-3) &&
          cleaned_path[len-1] != '/') {
        cleaned_path[len] = '/';
        cleaned_path[len+1] = '\0';
      }
    }

  } else
    sstrncpy(cleaned_path, path, sizeof(cleaned_path));

  if (!fs->fs_path)
    fs->fs_path = pstrdup(fs->fs_pool, cleaned_path);

  /* Check for duplicates. */
  if (fs_map->nelts > 0) {
    pr_fs_t *fsi = NULL, **fs_objs = (pr_fs_t **) fs_map->elts;
    register int i;

    for (i = 0; i < fs_map->nelts; i++) {
      fsi = fs_objs[i];

      if (strcmp(fsi->fs_path, cleaned_path) == 0) {
        /* An entry for this path already exists.  Make sure the FS being
         * mounted is not the same as the one already present.
         */
        if (strcmp(fsi->fs_name, fs->fs_name) == 0) {
          pr_log_pri(PR_LOG_DEBUG,
            "error: duplicate fs paths not allowed: '%s'", cleaned_path);
          errno = EEXIST;
          return FALSE;
        }

        /* "Push" the given FS on top of the existing one. */
        fs->fs_next = fsi;
        fsi->fs_prev = fs;
        fs_objs[i] = fs;

        chk_fs_map = TRUE;
        return TRUE;
      }
    }
  }

  /* Push the new FS into the container, then resort the contents. */
  *((pr_fs_t **) push_array(fs_map)) = fs;

  /* Sort the FSs in the map according to their paths (if there are
   * more than one element in the array_header.
   */
  if (fs_map->nelts > 1)
    qsort(fs_map->elts, fs_map->nelts, sizeof(pr_fs_t *), fs_cmp);

  /* Set the flag so that the fs wrapper functions know that a new FS
   * has been registered.
   */
  chk_fs_map = TRUE;

  return TRUE;
}

pr_fs_t *pr_unmount_fs(const char *path, const char *name) {
  pr_fs_t *fsi = NULL, **fs_objs = NULL;
  register unsigned int i = 0;

  /* Sanity check */
  if (!path) {
    errno = EINVAL;
    return NULL;
  }

  /* This should never be called before pr_register_fs(), but, just in case...*/
  if (!fs_map) {
    errno = EACCES;
    return NULL;
  }

  fs_objs = (pr_fs_t **) fs_map->elts;

  for (i = 0; i < fs_map->nelts; i++) {
    fsi = fs_objs[i];

    if (strcmp(fsi->fs_path, path) == 0 &&
        (name ? strcmp(fsi->fs_name, name) == 0 : TRUE)) {

      /* Exact match -- remove this FS.  If there is an FS underneath, pop 
       * the top FS off the stack.  Otherwise, allocate a new map.  Then
       * iterate through the old map, pushing all other FSs into the new map.
       * Destroy the old map.  Move the new map into place.
       */

      if (fsi->fs_next == NULL) {
        register unsigned int j = 0;
        pr_fs_t *tmp_fs, **old_objs = NULL;
        pool *map_pool;
        array_header *new_map;

        /* If removing this FS would leave an empty map, don't bother
         * allocating a new one.
         */
        if (fs_map->nelts == 1) {
          destroy_pool(fs_map->pool);
          fs_map = NULL;
          fs_cwd = root_fs;

          chk_fs_map = TRUE;
          return NULL;
        }

        map_pool = make_sub_pool(permanent_pool);
        new_map = make_array(map_pool, 0, sizeof(pr_fs_t *));

        pr_pool_tag(map_pool, "FSIO Map Pool");
        old_objs = (pr_fs_t **) fs_map->elts;

        for (j = 0; j < fs_map->nelts; j++) {
          tmp_fs = old_objs[j];

          if (strcmp(tmp_fs->fs_path, path) != 0)
            *((pr_fs_t **) push_array(new_map)) = old_objs[j];
        }

        destroy_pool(fs_map->pool);
        fs_map = new_map;

        /* Don't forget to set the flag so that wrapper functions scan the
         * new map.
         */
        chk_fs_map = TRUE;

        return fsi;
      }

      /* "Pop" this FS off the stack. */
      if (fsi->fs_next)
        fsi->fs_next->fs_prev = NULL;
      fs_objs[i] = fsi->fs_next;
      fsi->fs_next = fsi->fs_prev = NULL; 

      chk_fs_map = TRUE;
      return fsi;
    }
  }

  return NULL;
}

pr_fs_t *pr_remove_fs(const char *path) {
  return pr_unmount_fs(path, NULL);
}

int pr_unregister_fs(const char *path) {
  pr_fs_t *fs = NULL;

  if (!path) {
    errno = EINVAL;
    return -1;
  }

  /* Call pr_remove_fs() to get the fs for this path removed from the
   * fs_map.
   */
  fs = pr_remove_fs(path);
  if (fs) {
    destroy_pool(fs->fs_pool);
    return 0;
  }

  errno = ENOENT;
  return -1;
}

/* This function returns the best pr_fs_t to handle the given path.  It will
 * return NULL if there are no registered pr_fs_ts to handle the given path,
 * in which case the default root_fs should be used.  This is so that
 * functions can look to see if an pr_fs_t, other than the default, for a
 * given path has been registered, if necessary.  If the return value is
 * non-NULL, that will be a registered pr_fs_t to handle the given path.  In
 * this case, if the exact argument is not NULL, it will either be TRUE,
 * signifying that the returned pr_fs_t is an exact match for the given
 * path, or FALSE, meaning the returned pr_fs_t is a "best match" -- most
 * likely the pr_fs_t that handles the directory in which the given path
 * occurs.
 */
pr_fs_t *pr_get_fs(const char *path, int *exact) {
  pr_fs_t *fs = NULL, **fs_objs = NULL, *best_match_fs = NULL;
  register unsigned int i = 0;

  /* Sanity check */
  if (!path) {
    errno = EINVAL;
    return NULL;
  }

  /* Basic optimization -- if there're no elements in the fs_map,
   * return the root_fs.
   */
  if (!fs_map ||
      fs_map->nelts == 0) {
    return root_fs;
  }

  fs_objs = (pr_fs_t **) fs_map->elts;
  best_match_fs = root_fs;

  /* In order to handle deferred-resolution paths (eg "~" paths), the given
   * path will need to be passed through dir_realpath(), if necessary.
   *
   * The chk_fs_map flag, if TRUE, should be cleared on return of this
   * function -- all that flag says is, if TRUE, that this function _might_
   * return something different than it did on a previous call.
   */

  for (i = 0; i < fs_map->nelts; i++) {
    int res = 0;

    fs = fs_objs[i];

    /* If the current pr_fs_t's path ends in a slash (meaning it is a
     * directory, and it matches the first part of the given path,
     * assume it to be the best pr_fs_t found so far.
     */
    if ((fs->fs_path)[strlen(fs->fs_path) - 1] == '/' &&
        !strncmp(path, fs->fs_path, strlen(fs->fs_path)))
      best_match_fs = fs;

    res = strcmp(fs->fs_path, path);

    if (res == 0) {

      /* Exact match */
      if (exact)
        *exact = TRUE;

      chk_fs_map = FALSE;
      return fs;

    } else if (res > 0) {

      if (exact)
        *exact = FALSE;

      chk_fs_map = FALSE;

      /* Gone too far - return the best-match pr_fs_t */
      return best_match_fs;
    }
  }

  chk_fs_map = FALSE;

  /* Return best-match by default */
  return best_match_fs;
}

#if defined(PR_USE_REGEX) && defined(PR_FS_MATCH)
void pr_associate_fs(pr_fs_match_t *fsm, pr_fs_t *fs) {
  *((pr_fs_t **) push_array(fsm->fsm_fs_objs)) = fs;
}

pr_fs_match_t *pr_create_fs_match(pool *p, const char *name,
    const char *pattern, int opmask) {
  pr_fs_match_t *fsm = NULL;
  pool *match_pool = NULL;
  regex_t *regexp = NULL;
  int res = 0;
  char regerr[80] = {'\0'};

  if (!p || !name || !pattern) {
    errno = EINVAL;
    return NULL;
  }

  match_pool = make_sub_pool(p);
  fsm = (pr_fs_match_t *) pcalloc(match_pool, sizeof(pr_fs_match_t));

  if (!fsm)
    return NULL;

  fsm->fsm_next = NULL;
  fsm->fsm_prev = NULL;

  fsm->fsm_pool = match_pool;
  fsm->fsm_name = pstrdup(fsm->fsm_pool, name);
  fsm->fsm_opmask = opmask;
  fsm->fsm_pattern = pstrdup(fsm->fsm_pool, pattern);

  regexp = pr_regexp_alloc();

  res = pr_regexp_compile(regexp, pattern, REG_EXTENDED|REG_NOSUB);
  if (res != 0) {
    pr_regexp_error(res, regexp, regerr, sizeof(regerr));
    pr_regexp_free(regexp);

    pr_log_pri(PR_LOG_ERR, "unable to compile regex '%s': %s", pattern, regerr);

    /* Destroy the just allocated pr_fs_match_t */
    destroy_pool(fsm->fsm_pool);

    return NULL;

  } else
    fsm->fsm_regex = regexp;

  /* All pr_fs_match_ts start out as null patterns, i.e. no defined callback.
   */
  fsm->trigger = NULL;

  /* Allocate an array_header, used to record the pointers of any pr_fs_ts
   * this pr_fs_match_t may register.  This array_header should be accessed
   * via associate_fs().
   */
  fsm->fsm_fs_objs = make_array(fsm->fsm_pool, 0, sizeof(pr_fs_t *));

  return fsm;
}

int pr_insert_fs_match(pr_fs_match_t *fsm) {
  pr_fs_match_t *fsmi = NULL;

  if (fs_match_list) {

    /* Find the end of the fs_match list */
    fsmi = fs_match_list;

    /* Prevent pr_fs_match_ts with duplicate names */
    if (strcmp(fsmi->fsm_name, fsm->fsm_name) == 0) {
      pr_log_pri(PR_LOG_DEBUG,
        "error: duplicate fs_match names not allowed: '%s'", fsm->fsm_name);
      return FALSE;
    }

    while (fsmi->fsm_next) {
      fsmi = fsmi->fsm_next;

      if (strcmp(fsmi->fsm_name, fsm->fsm_name) == 0) {
        pr_log_pri(PR_LOG_DEBUG,
          "error: duplicate fs_match names not allowed: '%s'", fsm->fsm_name);
        return FALSE;
      }
    }

    fsm->fsm_next = NULL;
    fsm->fsm_prev = fsmi;
    fsmi->fsm_next = fsm;

  } else

    /* This fs_match _becomes_ the start of the fs_match list */
    fs_match_list = fsm;

  return TRUE;
}

pr_fs_match_t *pr_register_fs_match(pool *p, const char *name,
    const char *pattern, int opmask) {
  pr_fs_match_t *fsm = NULL;

  /* Sanity check */
  if (!p || !name || !pattern) {
    errno = EINVAL;
    return NULL;
  }

  /* Instantiate an fs_match */
  if ((fsm = pr_create_fs_match(p, name, pattern, opmask)) != NULL) {

    /* Insert the fs_match into the list */
    if (!pr_insert_fs_match(fsm)) {
      pr_regexp_free(fsm->fsm_regex);
      destroy_pool(fsm->fsm_pool);

      return NULL;
    }
  }

  return fsm;
}

int pr_unregister_fs_match(const char *name) {
  pr_fs_match_t *fsm = NULL;
  pr_fs_t **assoc_fs_objs = NULL, *assoc_fs = NULL;
  int removed = FALSE;

  /* fs_matches are required to have duplicate names, so using the name as
   * the identifier will work.
   */

  /* Sanity check*/
  if (!name) {
    errno = EINVAL;
    return FALSE;
  }

  if (fs_match_list) {
    for (fsm = fs_match_list; fsm; fsm = fsm->fsm_next) {

      /* Search by name */
      if ((name && fsm->fsm_name && strcmp(fsm->fsm_name, name) == 0)) {

        /* Remove this fs_match from the list */
        if (fsm->fsm_prev)
          fsm->fsm_prev->fsm_next = fsm->fsm_next;

        if (fsm->fsm_next)
          fsm->fsm_next->fsm_prev = fsm->fsm_prev;

        /* Check for any pr_fs_ts this pattern may have registered, and
         * remove them as well.
         */
        assoc_fs_objs = (pr_fs_t **) fsm->fsm_fs_objs->elts;

        for (assoc_fs = *assoc_fs_objs; assoc_fs; assoc_fs++)
          pr_unregister_fs(assoc_fs->fs_path);

        pr_regexp_free(fsm->fsm_regex);
        destroy_pool(fsm->fsm_pool);

        /* If this fs_match's prev and next pointers are NULL, it is the
         * last fs_match in the list.  If this is the case, make sure
         * that fs_match_list is set to NULL, signalling that there are
         * no more registered fs_matches.
         */
        if (fsm->fsm_prev == NULL && fsm->fsm_next == NULL) {
          fs_match_list = NULL;
          fsm = NULL;
        }

        removed = TRUE;
      }
    }
  }

  return (removed ? TRUE : FALSE);
}

pr_fs_match_t *pr_get_next_fs_match(pr_fs_match_t *fsm, const char *path,
    int op) {
  pr_fs_match_t *fsmi = NULL;

  /* Sanity check */
  if (!fsm) {
    errno = EINVAL;
    return NULL;
  }

  for (fsmi = fsm->fsm_next; fsmi; fsmi = fsmi->fsm_next) {
    if ((fsmi->fsm_opmask & op) &&
        pr_regexp_exec(fsmi->fsm_regex, path, 0, NULL, 0) == 0)
      return fsmi;
  }

  return NULL;
}

pr_fs_match_t *pr_get_fs_match(const char *path, int op) {
  pr_fs_match_t *fsm = NULL;

  if (!fs_match_list)
    return NULL;

  /* Check the first element in the fs_match_list... */
  fsm = fs_match_list;

  if ((fsm->fsm_opmask & op) &&
      pr_regexp_exec(fsm->fsm_regex, path, 0, NULL, 0) == 0)
    return fsm;

  /* ...otherwise, hand the search off to pr_get_next_fs_match() */
  return pr_get_next_fs_match(fsm, path, op);
}
#endif /* PR_USE_REGEX and PR_FS_MATCH */

void pr_fs_setcwd(const char *dir) {
  pr_fs_resolve_path(dir, cwd, sizeof(cwd)-1, FSIO_DIR_CHDIR);
  sstrncpy(cwd, dir, sizeof(cwd));
  fs_cwd = lookup_dir_fs(cwd, FSIO_DIR_CHDIR);
  cwd[sizeof(cwd) - 1] = '\0';
}

const char *pr_fs_getcwd(void) {
  return cwd;
}

const char *pr_fs_getvwd(void) {
  return vwd;
}

int pr_fs_dircat(char *buf, int buflen, const char *dir1, const char *dir2) {
  /* Make temporary copies so that memory areas can overlap */
  char *_dir1 = NULL, *_dir2 = NULL;
  size_t dir1len = 0;

  if (!dir1 || !dir2) {
    errno = EINVAL;
    return -1;
  }

  /* This is a test to see if we've got reasonable directories to concatenate.
   */
  if ((strlen(dir1) + strlen(dir2) + 1) >= PR_TUNABLE_PATH_MAX) {
    errno = ENAMETOOLONG;
    buf[0] = '\0';  
    return -1;
  }

  _dir1 = strdup(dir1);
  if (!_dir1)
    return -1;

  _dir2 = strdup(dir2);
  if (!_dir2) {
    free(_dir1);
    return -1;
  }

  dir1len = strlen(_dir1) - 1;

  if (*_dir2 == '/') {
    sstrncpy(buf, _dir2, buflen);
    free(_dir1);
    free(_dir2);
    return 0;
  }

  sstrncpy(buf, _dir1, buflen);

  if (buflen && *(_dir1 + dir1len) != '/')
    sstrcat(buf, "/", buflen);

  sstrcat(buf, _dir2, buflen);

  if (!*buf) {
   *buf++ = '/';
   *buf = '\0';
  }

  free(_dir1);
  free(_dir2);

  return 0;
}

/* This function performs any tilde expansion needed and then returns the
 * resolved path, if any.
 *
 * Returns: -1 (errno = ENOENT): user does not exist
 *           0 : no interpolation done (path exists)
 *           1 : interpolation done
 */
int pr_fs_interpolate(const char *path, char *buf, size_t buflen) {
  pool *p = NULL;
  struct passwd *pw = NULL;
  struct stat sbuf;
  char *fname = NULL;
  char user[PR_TUNABLE_LOGIN_MAX + 1] = {'\0'};
  int len;

  if (!path) {
    errno = EINVAL;
    return -1;
  }

  if (path[0] == '~') {
    fname = strchr(path, '/');

    /* Copy over the username.
     */
    if (fname) {
      len = fname - path;
      sstrncpy(user, path + 1, len > sizeof(user) ? sizeof(user) : len);

      /* Advance past the '/'. */
      fname++;

    } else if (pr_fsio_stat(path, &sbuf) == -1) {

      /* Otherwise, this might be something like "~foo" which could be a file
       * or it could be a user.  Let's find out.
       *
       * Must be a user, if anything...otherwise it's probably a typo.
       */
      len = strlen(path);
      sstrncpy(user, path + 1, len > sizeof(user) ? sizeof(user) : len);

    } else {

      /* Otherwise, this _is_ the file in question, perform no interpolation.
       */
      fname = (char *) path;
      return 0;
    }

    /* If the user hasn't been explicitly specified, set it here.  This
     * handles cases such as files beginning with "~", "~/foo" or simply "~".
     */
    if (!*user)
      sstrncpy(user, session.user, sizeof(user));

    /* The permanent pool is used here, rather than session.pool, as path
     * interpolation can occur during startup parsing, when session.pool does
     * not exist.  It does not really matter, since the allocated sub pool
     * is destroyed shortly.
     */
    p = make_sub_pool(permanent_pool);
    pr_pool_tag(p, "pr_fs_interpolate() pool");

    pw = pr_auth_getpwnam(p, user);

    if (!pw) {
      destroy_pool(p);
      errno = ENOENT;
      return -1;
    }

    sstrncpy(buf, pw->pw_dir, buflen);

    /* Done with pw, which means we can destroy the temporary pool now. */
    destroy_pool(p);

    len = strlen(buf);

    if (fname && len < buflen && buf[len - 1] != '/')
      buf[len++] = '/';

    if (fname)
      sstrncpy(&buf[len], fname, buflen - len);

  } else
    sstrncpy(buf, path, buflen);

  return 1;
}

int pr_fs_resolve_partial(const char *path, char *buf, size_t buflen, int op) {
  char curpath[PR_TUNABLE_PATH_MAX + 1]  = {'\0'},
       workpath[PR_TUNABLE_PATH_MAX + 1] = {'\0'},
       namebuf[PR_TUNABLE_PATH_MAX + 1]  = {'\0'},
       *where = NULL, *ptr = NULL, *last = NULL;

  pr_fs_t *fs = NULL;
  int len = 0, fini = 1, link_cnt = 0;
  ino_t last_inode = 0;
  dev_t last_device = 0;
  struct stat sbuf;

  if (!path) {
    errno = EINVAL;
    return -1;
  }

  if (*path != '/') {
    if (*path == '~') {
      switch (pr_fs_interpolate(path, curpath, sizeof(curpath)-1)) {
      case -1:
        return -1;

      case 0:
        sstrncpy(curpath, path, sizeof(curpath));
        sstrncpy(workpath, cwd, sizeof(workpath));
        break;
      }

    } else {
      sstrncpy(curpath, path, sizeof(curpath));
      sstrncpy(workpath, cwd, sizeof(workpath));
    }

  } else
    sstrncpy(curpath, path, sizeof(curpath));

  while (fini--) {
    where = curpath;

    while (*where != '\0') {
      pr_signals_handle();

      /* Handle "." */
      if (strncmp(where, ".", 2) == 0) {
        where++;
        continue;
      }

      /* Handle ".." */
      if (strncmp(where, "..", 3) == 0) {
        where += 2;
        ptr = last = workpath;

        while (*ptr) {
          if (*ptr == '/')
            last = ptr;
          ptr++;
        }

        *last = '\0';
        continue;
      }

      /* Handle "./" */
      if (strncmp(where, "./", 2) == 0) {
        where += 2;
        continue;
      }

      /* Handle "../" */
      if (strncmp(where, "../", 3) == 0) {
        where += 3;
        ptr = last = workpath;

        while (*ptr) {
          if (*ptr == '/')
            last = ptr;
          ptr++;
        }

        *last = '\0';
        continue;
      }

      ptr = strchr(where, '/');
      if (ptr == NULL) {
        size_t wherelen = strlen(where);

        ptr = where;
        if (wherelen >= 1)
          ptr += (wherelen - 1);

      } else {
        *ptr = '\0';
      }

      sstrncpy(namebuf, workpath, sizeof(namebuf));

      if (*namebuf) {
        for (last = namebuf; *last; last++);
        if (*--last != '/')
          sstrcat(namebuf, "/", sizeof(namebuf)-1);

      } else {
        sstrcat(namebuf, "/", sizeof(namebuf)-1);
      }

      sstrcat(namebuf, where, sizeof(namebuf)-1);

      where = ++ptr;

      fs = lookup_dir_fs(namebuf, op);

      if (fs_cache_lstat(fs, namebuf, &sbuf) == -1)
        return -1;

      if (S_ISLNK(sbuf.st_mode)) {
        char linkpath[PR_TUNABLE_PATH_MAX + 1] = {'\0'};

        /* Detect an obvious recursive symlink */
        if (sbuf.st_ino && (ino_t) sbuf.st_ino == last_inode &&
            sbuf.st_dev && (dev_t) sbuf.st_dev == last_device) {
          errno = ELOOP;
          return -1;
        }

        last_inode = (ino_t) sbuf.st_ino;
        last_device = (dev_t) sbuf.st_dev;

        if (++link_cnt > 32) {
          errno = ELOOP;
          return -1;
        }
	
        len = pr_fsio_readlink(namebuf, linkpath, sizeof(linkpath)-1);
        if (len <= 0) {
          errno = ENOENT;
          return -1;
        }

        *(linkpath + len) = '\0';
        if (*linkpath == '/')
          *workpath = '\0';

        /* Trim any trailing slash. */
        if (linkpath[len-1] == '/')
          linkpath[len-1] = '\0';

        if (*linkpath == '~') {
          char tmpbuf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};

          *workpath = '\0';
          sstrncpy(tmpbuf, linkpath, sizeof(tmpbuf));

          if (pr_fs_interpolate(tmpbuf, linkpath, sizeof(linkpath)-1) == -1)
	    return -1;
        }

        if (*where) {
          sstrcat(linkpath, "/", sizeof(linkpath)-1);
          sstrcat(linkpath, where, sizeof(linkpath)-1);
        }

        sstrncpy(curpath, linkpath, sizeof(curpath));
        fini++;
        break; /* continue main loop */
      }

      if (S_ISDIR(sbuf.st_mode)) {
        sstrncpy(workpath, namebuf, sizeof(workpath));
        continue;
      }

      if (*where) {
        errno = ENOENT;
        return -1;               /* path/notadir/morepath */

      } else {
        sstrncpy(workpath, namebuf, sizeof(workpath));
      }
    }
  }

  if (!workpath[0])
    sstrncpy(workpath, "/", sizeof(workpath));

  sstrncpy(buf, workpath, buflen);

  return 0;
}

int pr_fs_resolve_path(const char *path, char *buf, size_t buflen, int op) {
  char curpath[PR_TUNABLE_PATH_MAX + 1]  = {'\0'},
       workpath[PR_TUNABLE_PATH_MAX + 1] = {'\0'},
       namebuf[PR_TUNABLE_PATH_MAX + 1]  = {'\0'},
       *where = NULL, *ptr = NULL, *last = NULL;

  pr_fs_t *fs = NULL;

  int len = 0, fini = 1, link_cnt = 0;
  ino_t last_inode = 0;
  dev_t last_device = 0;
  struct stat sbuf;

  if (!path) {
    errno = EINVAL;
    return -1;
  }

  if (pr_fs_interpolate(path, curpath, sizeof(curpath)-1) != -1)
    sstrncpy(curpath, path, sizeof(curpath));

  if (curpath[0] != '/')
    sstrncpy(workpath, cwd, sizeof(workpath));
  else
    workpath[0] = '\0';

  while (fini--) {
    where = curpath;

    while (*where != '\0') {
      pr_signals_handle();

      if (strncmp(where, ".", 2) == 0) {
        where++;
        continue;
      }

      /* handle "./" */
      if (strncmp(where, "./", 2) == 0) {
        where += 2;
        continue;
      }

      /* handle "../" */
      if (strncmp(where, "../", 3) == 0) {
        where += 3;
        ptr = last = workpath;
        while (*ptr) {
          if (*ptr == '/')
            last = ptr;
          ptr++;
        }

        *last = '\0';
        continue;
      }

      ptr = strchr(where, '/');

      if (!ptr) {
        size_t wherelen = strlen(where);

        ptr = where;
        if (wherelen >= 1)
          ptr += (wherelen - 1);

      } else
        *ptr = '\0';

      sstrncpy(namebuf, workpath, sizeof(namebuf));

      if (*namebuf) {
        for (last = namebuf; *last; last++);

        if (*--last != '/')
          sstrcat(namebuf, "/", sizeof(namebuf)-1);

      } else
        sstrcat(namebuf, "/", sizeof(namebuf)-1);

      sstrcat(namebuf, where, sizeof(namebuf)-1);

      where = ++ptr;

      fs = lookup_dir_fs(namebuf, op);

      if (fs_cache_lstat(fs, namebuf, &sbuf) == -1) {
        errno = ENOENT;
        return -1;
      }

      if (S_ISLNK(sbuf.st_mode)) {
        char linkpath[PR_TUNABLE_PATH_MAX + 1] = {'\0'};

        /* Detect an obvious recursive symlink */
        if (sbuf.st_ino && (ino_t) sbuf.st_ino == last_inode &&
            sbuf.st_dev && (dev_t) sbuf.st_dev == last_device) {
          errno = ELOOP;
          return -1;
        }

        last_inode = (ino_t) sbuf.st_ino;
        last_device = (dev_t) sbuf.st_dev;

        if (++link_cnt > 32) {
          errno = ELOOP;
          return -1;
        }

        len = pr_fsio_readlink(namebuf, linkpath, sizeof(linkpath)-1);
        if (len <= 0) {
          errno = ENOENT;
          return -1;
        }

        *(linkpath+len) = '\0';

        if (*linkpath == '/')
          *workpath = '\0';

        /* Trim any trailing slash. */
        if (linkpath[len-1] == '/')
          linkpath[len-1] = '\0';

        if (*linkpath == '~') {
          char tmpbuf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
          *workpath = '\0';

          sstrncpy(tmpbuf, linkpath, sizeof(tmpbuf));

          if (pr_fs_interpolate(tmpbuf, linkpath, sizeof(linkpath)-1) == -1)
	    return -1;
        }

        if (*where) {
          sstrcat(linkpath, "/", sizeof(linkpath)-1);
          sstrcat(linkpath, where, sizeof(linkpath)-1);
        }

        sstrncpy(curpath, linkpath, sizeof(curpath));
        fini++;
        break; /* continue main loop */
      }

      if (S_ISDIR(sbuf.st_mode)) {
        sstrncpy(workpath, namebuf, sizeof(workpath));
        continue;
      }

      if (*where) {
        errno = ENOENT;
        return -1;               /* path/notadir/morepath */

      } else
        sstrncpy(workpath, namebuf, sizeof(workpath));
    }
  }

  if (!workpath[0])
    sstrncpy(workpath, "/", sizeof(workpath));

  sstrncpy(buf, workpath, buflen);

  return 0;
}

void pr_fs_clean_path(const char *path, char *buf, size_t buflen) {
  char workpath[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  char curpath[PR_TUNABLE_PATH_MAX + 1]  = {'\0'};
  char namebuf[PR_TUNABLE_PATH_MAX + 1]  = {'\0'};
  char *where = NULL, *ptr = NULL, *last = NULL;
  int fini = 1;

  if (!path)
    return;

  sstrncpy(curpath, path, sizeof(curpath));

  /* main loop */
  while (fini--) {
    where = curpath;

    while (*where != '\0') {
      pr_signals_handle();

      if (strncmp(where, ".", 2) == 0) {
        where++;
        continue;
      }

      /* handle "./" */
      if (strncmp(where, "./", 2) == 0) {
        where += 2;
        continue;
      }

      /* handle ".." */
      if (strncmp(where, "..", 3) == 0) {
        where += 2;
        ptr = last = workpath;

        while (*ptr) {
          if (*ptr == '/')
            last = ptr;
          ptr++;
        }

        *last = '\0';
        continue;
      }

      /* handle "../" */
      if (strncmp(where, "../", 3) == 0) {
        where += 3;
        ptr = last = workpath;

        while (*ptr) {
          if (*ptr == '/')
            last = ptr;
          ptr++;
        }
        *last = '\0';
        continue;
      }
      ptr = strchr(where, '/');

      if (!ptr) {
        size_t wherelen = strlen(where);

        ptr = where;
        if (wherelen >= 1)
          ptr += (wherelen - 1);

      } else
        *ptr = '\0';

      sstrncpy(namebuf, workpath, sizeof(namebuf));

      if (*namebuf) {
        for (last = namebuf; *last; last++);
        if (*--last != '/')
          sstrcat(namebuf, "/", sizeof(namebuf)-1);

      } else
        sstrcat(namebuf, "/", sizeof(namebuf)-1);

      sstrcat(namebuf, where, sizeof(namebuf)-1);
      namebuf[sizeof(namebuf)-1] = '\0';

      where = ++ptr;

      sstrncpy(workpath, namebuf, sizeof(workpath));
    }
  }

  if (!workpath[0])
    sstrncpy(workpath, "/", sizeof(workpath));

  sstrncpy(buf, workpath, buflen);
}

int pr_fs_use_encoding(int bool) {
  int curr_setting = use_encoding;
  use_encoding = bool;

  return curr_setting;
}

char *pr_fs_decode_path(pool *p, const char *path) {
#ifdef PR_USE_NLS
  size_t outlen;
  char *res;

  if (!use_encoding) {
    return (char *) path;
  }

  res = pr_decode_str(p, path, strlen(path) + 1, &outlen);
  if (!res) {
    pr_trace_msg("encode", 1, "error decoding path '%s': %s", path,
      strerror(errno));
    return (char *) path;
  }

  pr_trace_msg("encode", 5, "decoded '%s' into '%s'", path, res);
  return res;
#else
  return (char *) path;
#endif /* PR_USE_NLS */
}

char *pr_fs_encode_path(pool *p, const char *path) {
#ifdef PR_USE_NLS
  size_t outlen;
  char *res;

  if (!use_encoding) {
    return (char *) path;
  }

  res = pr_encode_str(p, path, strlen(path) + 1, &outlen);
  if (!res) {
    pr_trace_msg("encode", 1, "error encoding path '%s': %s", path,
      strerror(errno));
    return (char *) path;
  }

  pr_trace_msg("encode", 5, "encoded '%s' into '%s'", path, res);
  return res;
#else
  return (char *) path;
#endif /* PR_USE_NLS */
}

/* This function checks the given path's prefix against the paths that
 * have been registered.  If no matching path prefix has been registered,
 * the path is considered invalid.
 */
int pr_fs_valid_path(const char *path) {

  if (fs_map && fs_map->nelts > 0) {
    pr_fs_t *fsi = NULL, **fs_objs = (pr_fs_t **) fs_map->elts;
    register int i;

    for (i = 0; i < fs_map->nelts; i++) {
      fsi = fs_objs[i];

      if (strncmp(fsi->fs_path, path, strlen(fsi->fs_path)) == 0)
        return 0;
    }
  }

  /* Also check the path against the default '/' path. */
  if (*path == '/')
    return 0;

  errno = EINVAL;
  return -1;
}

void pr_fs_virtual_path(const char *path, char *buf, size_t buflen) {
  char curpath[PR_TUNABLE_PATH_MAX + 1]  = {'\0'},
       workpath[PR_TUNABLE_PATH_MAX + 1] = {'\0'},
       namebuf[PR_TUNABLE_PATH_MAX + 1]  = {'\0'},
       *where = NULL, *ptr = NULL, *last = NULL;

  int fini = 1;

  if (!path)
    return;

  if (pr_fs_interpolate(path, curpath, sizeof(curpath)-1) != -1)
    sstrncpy(curpath, path, sizeof(curpath));

  if (curpath[0] != '/')
    sstrncpy(workpath, vwd, sizeof(workpath));
  else
    workpath[0] = '\0';

  /* curpath is path resolving */
  /* linkpath is path a symlink pointed to */
  /* workpath is the path we've resolved */

  /* main loop */
  while (fini--) {
    where = curpath;
    while (*where != '\0') {
      if (strncmp(where, ".", 2) == 0) {
        where++;
        continue;
      }

      /* handle "./" */
      if (strncmp(where, "./", 2) == 0) {
        where += 2;
        continue;
      }

      /* handle ".." */
      if (strncmp(where, "..", 3) == 0) {
        where += 2;
        ptr = last = workpath;
        while (*ptr) {
          if (*ptr == '/')
            last = ptr;
          ptr++;
        }

        *last = '\0';
        continue;
      }

      /* handle "../" */
      if (strncmp(where, "../", 3) == 0) {
        where += 3;
        ptr = last = workpath;
        while (*ptr) {
          if (*ptr == '/')
            last = ptr;
          ptr++;
        }
        *last = '\0';
        continue;
      }
      ptr = strchr(where, '/');

      if (!ptr) {
        size_t wherelen = strlen(where);

        ptr = where;
        if (wherelen >= 1)
          ptr += (wherelen - 1);

      } else
        *ptr = '\0';

      sstrncpy(namebuf, workpath, sizeof(namebuf));

      if (*namebuf) {
        for (last = namebuf; *last; last++);
        if (*--last != '/')
          sstrcat(namebuf, "/", sizeof(namebuf)-1);

      } else
        sstrcat(namebuf, "/", sizeof(namebuf)-1);

      sstrcat(namebuf, where, sizeof(namebuf)-1);

      where = ++ptr;

      sstrncpy(workpath, namebuf, sizeof(workpath));
    }
  }

  if (!workpath[0])
    sstrncpy(workpath, "/", sizeof(workpath));

  sstrncpy(buf, workpath, buflen);
}

int pr_fsio_chdir_canon(const char *path, int hidesymlink) {
  char resbuf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  pr_fs_t *fs = NULL;
  int res = 0;

  if (pr_fs_resolve_partial(path, resbuf, sizeof(resbuf)-1,
      FSIO_DIR_CHDIR) == -1)
    return -1;

  fs = lookup_dir_fs(resbuf, FSIO_DIR_CHDIR);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom chdir handler.  If there are none,
   * use the system chdir.
   */
  while (fs && fs->fs_next && !fs->chdir)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s chdir() for path '%s'", fs->fs_name,
    path);
  res = (fs->chdir)(fs, resbuf);

  if (res != -1) {
    /* chdir succeeded, so we set fs_cwd for future references. */
     fs_cwd = fs ? fs : root_fs;

     if (hidesymlink) {
       pr_fs_virtual_path(path, vwd, sizeof(vwd)-1);

     } else {
       sstrncpy(vwd, resbuf, sizeof(vwd));
     }
  }

  return res;
}

int pr_fsio_chdir(const char *path, int hidesymlink) {
  char resbuf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};
  pr_fs_t *fs = NULL;
  int res;

  pr_fs_clean_path(path, resbuf, sizeof(resbuf)-1);

  fs = lookup_dir_fs(path, FSIO_DIR_CHDIR);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom chdir handler.  If there are none,
   * use the system chdir.
   */
  while (fs && fs->fs_next && !fs->chdir)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s chdir() for path '%s'", fs->fs_name,
    path);
  res = (fs->chdir)(fs, resbuf);

  if (res != -1) {
    /* chdir succeeded, so we set fs_cwd for future references. */
     fs_cwd = fs;

     if (hidesymlink)
       pr_fs_virtual_path(path, vwd, sizeof(vwd)-1);
     else
       sstrncpy(vwd, resbuf, sizeof(vwd));
  }

  return res;
}

/* fs_opendir, fs_closedir and fs_readdir all use a nifty
 * optimization, caching the last-recently-used pr_fs_t, and
 * avoid future pr_fs_t lookups when iterating via readdir.
 */
void *pr_fsio_opendir(const char *path) {
  pr_fs_t *fs = NULL;
  fsopendir_t *fsod = NULL, *fsodi = NULL;
  pool *fsod_pool = NULL;
  DIR *res = NULL;

  if (strchr(path, '/') == NULL) {
    pr_fs_setcwd(pr_fs_getcwd());
    fs = fs_cwd;

  } else {
    char buf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};

    if (pr_fs_resolve_partial(path, buf, sizeof(buf)-1, FSIO_DIR_OPENDIR) == -1)
      return NULL;

    fs = lookup_dir_fs(buf, FSIO_DIR_OPENDIR);
  }

  /* Find the first non-NULL custom opendir handler.  If there are none,
   * use the system opendir.
   */
  while (fs && fs->fs_next && !fs->opendir)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s opendir() for path '%s'",
    fs->fs_name, path);
  res = (fs->opendir)(fs, path);

  if (res == NULL) {
    return NULL;
  }

  /* Cache it here */
  fs_cache_dir = res;
  fs_cache_fsdir = fs;

  fsod_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(fsod_pool, "fsod subpool");

  fsod = pcalloc(fsod_pool, sizeof(fsopendir_t));

  if (fsod == NULL) {
    if (fs->closedir) {
      (fs->closedir)(fs, res);
      errno = ENOMEM;
      return NULL;

    } else {
      sys_closedir(fs, res);
      errno = ENOMEM;
      return NULL;
    }
  }

  fsod->pool = fsod_pool;
  fsod->dir = res;
  fsod->fsdir = fs;
  fsod->next = NULL;
  fsod->prev = NULL;

  if (fsopendir_list) {

    /* find the end of the fsopendir list */
    fsodi = fsopendir_list;
    while (fsodi->next) {
      pr_signals_handle();
      fsodi = fsodi->next;
    }

    fsod->next = NULL;
    fsod->prev = fsodi;
    fsodi->next = fsod;

  } else {
    /* This fsopendir _becomes_ the start of the fsopendir list */
    fsopendir_list = fsod;
  }

  return res;
}

static pr_fs_t *find_opendir(void *dir, int closing) {
  pr_fs_t *fs = NULL;

  if (fsopendir_list) {
    fsopendir_t *fsod;

    for (fsod = fsopendir_list; fsod; fsod = fsod->next) {
      if (fsod->dir && fsod->dir == dir) {
        fs = fsod->fsdir;
        break;
      }
    }
   
    if (closing && fsod) {
      if (fsod->prev)
        fsod->prev->next = fsod->next;
 
      if (fsod->next)
        fsod->next->prev = fsod->prev;

      if (fsod == fsopendir_list)
        fsopendir_list = fsod->next;

      destroy_pool(fsod->pool);
    }
  }

  if (dir == fs_cache_dir) {
    fs = fs_cache_fsdir;

    if (closing) {
      fs_cache_dir = NULL;
      fs_cache_fsdir = NULL;
    }
  }

  return fs;
}

int pr_fsio_closedir(void *dir) {
  int res;
  pr_fs_t *fs = find_opendir(dir, TRUE);

  if (!fs)
    return -1;

  /* Find the first non-NULL custom closedir handler.  If there are none,
   * use the system closedir.
   */
  while (fs && fs->fs_next && !fs->closedir)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s closedir()", fs->fs_name);
  res = (fs->closedir)(fs, dir);

  return res;
}

struct dirent *pr_fsio_readdir(void *dir) {
  struct dirent *res;
  pr_fs_t *fs = find_opendir(dir, FALSE);

  if (!fs)
    return NULL;

  /* Find the first non-NULL custom readdir handler.  If there are none,
   * use the system readdir.
   */
  while (fs && fs->fs_next && !fs->readdir)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s readdir()", fs->fs_name);
  res = (fs->readdir)(fs, dir);

  return res;
}

int pr_fsio_mkdir(const char *path, mode_t mode) {
  int res;
  pr_fs_t *fs;

  fs = lookup_dir_fs(path, FSIO_DIR_MKDIR);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom mkdir handler.  If there are none,
   * use the system mkdir.
   */
  while (fs && fs->fs_next && !fs->mkdir)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s mkdir() for path '%s'", fs->fs_name,
    path);
  res = (fs->mkdir)(fs, path, mode);

  return res;
}

int pr_fsio_rmdir(const char *path) {
  int res;
  pr_fs_t *fs;

  fs = lookup_dir_fs(path, FSIO_DIR_RMDIR);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom rmdir handler.  If there are none,
   * use the system rmdir.
   */
  while (fs && fs->fs_next && !fs->rmdir)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s rmdir() for path '%s'", fs->fs_name,
    path);
  res = (fs->rmdir)(fs, path);

  return res;
}

int pr_fsio_stat_canon(const char *path, struct stat *sbuf) {
  pr_fs_t *fs = lookup_file_canon_fs(path, NULL, FSIO_FILE_STAT);

  /* Find the first non-NULL custom stat handler.  If there are none,
   * use the system stat.
   */
  while (fs && fs->fs_next && !fs->stat)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s stat() for path '%s'",
    fs ? fs->fs_name : "system", path);
  return fs_cache_stat(fs ? fs : root_fs, path, sbuf);
}

int pr_fsio_stat(const char *path, struct stat *sbuf) {
  pr_fs_t *fs;

  fs = lookup_file_fs(path, NULL, FSIO_FILE_STAT);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom stat handler.  If there are none,
   * use the system stat.
   */
  while (fs && fs->fs_next && !fs->stat)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s stat() for path '%s'", fs->fs_name,
    path);
  return fs_cache_stat(fs ? fs : root_fs, path, sbuf);
}

int pr_fsio_fstat(pr_fh_t *fh, struct stat *sbuf) {
  int res;
  pr_fs_t *fs;

  if (!fh) {
    errno = EINVAL;
    return -1;
  }

  /* Find the first non-NULL custom fstat handler.  If there are none,
   * use the system fstat.
   */
  fs = fh->fh_fs;
  while (fs && fs->fs_next && !fs->fstat)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s fstat() for path '%s'", fs->fs_name,
    fh->fh_path);
  res = (fs->fstat)(fh, fh->fh_fd, sbuf);

  return res;
}

int pr_fsio_lstat_canon(const char *path, struct stat *sbuf) {
  pr_fs_t *fs = lookup_file_canon_fs(path, NULL, FSIO_FILE_LSTAT);

  /* Find the first non-NULL custom lstat handler.  If there are none,
   * use the system lstat.
   */
  while (fs && fs->fs_next && !fs->lstat)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s lstat() for path '%s'",
    fs ? fs->fs_name : "system", path);
  return fs_cache_lstat(fs ? fs : root_fs, path, sbuf);
}

int pr_fsio_lstat(const char *path, struct stat *sbuf) {
  pr_fs_t *fs;

  fs = lookup_file_fs(path, NULL, FSIO_FILE_LSTAT);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom lstat handler.  If there are none,
   * use the system lstat.
   */
  while (fs && fs->fs_next && !fs->lstat)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s lstat() for path '%s'", fs->fs_name,
    path);
  return fs_cache_lstat(fs ? fs : root_fs, path, sbuf);
}

int pr_fsio_readlink_canon(const char *path, char *buf, size_t buflen) {
  int res;
  pr_fs_t *fs = lookup_file_canon_fs(path, NULL, FSIO_FILE_READLINK);

  /* Find the first non-NULL custom readlink handler.  If there are none,
   * use the system readlink.
   */
  while (fs && fs->fs_next && !fs->readlink)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s readlink() for path '%s'",
    fs->fs_name, path);
  res = (fs->readlink)(fs, path, buf, buflen);

  return res;
}

int pr_fsio_readlink(const char *path, char *buf, size_t buflen) {
  int res;
  pr_fs_t *fs;

  fs = lookup_file_fs(path, NULL, FSIO_FILE_READLINK);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom readlink handler.  If there are none,
   * use the system readlink.
   */
  while (fs && fs->fs_next && !fs->readlink)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s readlink() for path '%s'",
    fs->fs_name, path);
  res = (fs->readlink)(fs, path, buf, buflen);

  return res;
}

/* pr_fs_glob() is just a wrapper for glob(3), setting the various gl_
 * callbacks to our fs functions.
 */
int pr_fs_glob(const char *pattern, int flags,
    int (*errfunc)(const char *, int), glob_t *pglob) {

  if (pglob) {
    flags |= GLOB_ALTDIRFUNC;

    pglob->gl_closedir = (void (*)(void *)) pr_fsio_closedir;
    pglob->gl_readdir = pr_fsio_readdir;
    pglob->gl_opendir = pr_fsio_opendir;
    pglob->gl_lstat = pr_fsio_lstat;
    pglob->gl_stat = pr_fsio_stat;
  }

  return glob(pattern, flags, errfunc, pglob);
}

void pr_fs_globfree(glob_t *pglob) {
  globfree(pglob);
}

int pr_fsio_rename_canon(const char *rfrom, const char *rto) {
  int res;
  pr_fs_t *from_fs, *to_fs, *fs;

  from_fs = lookup_file_canon_fs(rfrom, NULL, FSIO_FILE_RENAME);
  to_fs = lookup_file_canon_fs(rto, NULL, FSIO_FILE_RENAME);

  if (from_fs->allow_xdev_rename == FALSE ||
      to_fs->allow_xdev_rename == FALSE) {
    if (from_fs != to_fs) {
      errno = EXDEV;
      return -1;
    }
  }

  fs = to_fs;

  /* Find the first non-NULL custom rename handler.  If there are none,
   * use the system rename.
   */
  while (fs && fs->fs_next && !fs->rename)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s rename() for paths '%s', '%s'",
    fs->fs_name, rfrom, rto);
  res = (fs->rename)(fs, rfrom, rto);

  return res;
}

int pr_fsio_rename(const char *rnfm, const char *rnto) {
  int res;
  pr_fs_t *from_fs, *to_fs, *fs;

  from_fs = lookup_file_fs(rnfm, NULL, FSIO_FILE_RENAME);
  if (from_fs == NULL) {
    return -1;
  }

  to_fs = lookup_file_fs(rnto, NULL, FSIO_FILE_RENAME);
  if (to_fs == NULL) {
    return -1;
  }

  if (from_fs->allow_xdev_rename == FALSE ||
      to_fs->allow_xdev_rename == FALSE) {
    if (from_fs != to_fs) {
      errno = EXDEV;
      return -1;
    }
  }

  fs = to_fs;

  /* Find the first non-NULL custom rename handler.  If there are none,
   * use the system rename.
   */
  while (fs && fs->fs_next && !fs->rename)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s rename() for paths '%s', '%s'",
    fs->fs_name, rnfm, rnto);
  res = (fs->rename)(fs, rnfm, rnto);

  return res;
}

int pr_fsio_unlink_canon(const char *name) {
  int res;
  pr_fs_t *fs = lookup_file_canon_fs(name, NULL, FSIO_FILE_UNLINK);

  /* Find the first non-NULL custom unlink handler.  If there are none,
   * use the system unlink.
   */
  while (fs && fs->fs_next && !fs->unlink)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s unlink() for path '%s'",
    fs->fs_name, name);
  res = (fs->unlink)(fs, name);

  return res;
}
	
int pr_fsio_unlink(const char *name) {
  int res;
  pr_fs_t *fs;

  fs = lookup_file_fs(name, NULL, FSIO_FILE_UNLINK);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom unlink handler.  If there are none,
   * use the system unlink.
   */
  while (fs && fs->fs_next && !fs->unlink)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s unlink() for path '%s'",
    fs->fs_name, name);
  res = (fs->unlink)(fs, name);

  return res;
}

pr_fh_t *pr_fsio_open_canon(const char *name, int flags) {
  char *deref = NULL;
  pool *tmp_pool = NULL;
  pr_fh_t *fh = NULL;

  pr_fs_t *fs = lookup_file_canon_fs(name, &deref, FSIO_FILE_OPEN);

  /* Allocate a filehandle. */
  tmp_pool = make_sub_pool(fs->fs_pool);
  pr_pool_tag(tmp_pool, "pr_fsio_open_canon() subpool");

  fh = pcalloc(tmp_pool, sizeof(pr_fh_t));
  fh->fh_pool = tmp_pool;
  fh->fh_path = pstrdup(fh->fh_pool, name);
  fh->fh_fd = -1;
  fh->fh_buf = NULL;
  fh->fh_fs = fs;

  /* Find the first non-NULL custom open handler.  If there are none,
   * use the system open.
   */
  while (fs && fs->fs_next && !fs->open)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s open() for path '%s'", fs->fs_name,
    name);
  fh->fh_fd = (fs->open)(fh, deref, flags);

  if (fh->fh_fd == -1) {
    destroy_pool(fh->fh_pool);
    return NULL;
  }

  return fh;
}

pr_fh_t *pr_fsio_open(const char *name, int flags) {
  pool *tmp_pool = NULL;
  pr_fh_t *fh = NULL;
  pr_fs_t *fs = NULL;

  if (!name) {
    errno = EINVAL;
    return NULL;
  }

  fs = lookup_file_fs(name, NULL, FSIO_FILE_OPEN);
  if (fs == NULL) {
    return NULL;
  }

  /* Allocate a filehandle. */
  tmp_pool = make_sub_pool(fs->fs_pool);
  pr_pool_tag(tmp_pool, "pr_fsio_open() subpool");

  fh = pcalloc(tmp_pool, sizeof(pr_fh_t));
  fh->fh_pool = tmp_pool;
  fh->fh_path = pstrdup(fh->fh_pool, name);
  fh->fh_fd = -1;
  fh->fh_buf = NULL;
  fh->fh_fs = fs;

  /* Find the first non-NULL custom open handler.  If there are none,
   * use the system open.
   */
  while (fs && fs->fs_next && !fs->open)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s open() for path '%s'", fs->fs_name,
    name);
  fh->fh_fd = (fs->open)(fh, name, flags);

  if (fh->fh_fd == -1) {
    destroy_pool(fh->fh_pool);
    return NULL;
  }

  return fh;
}

pr_fh_t *pr_fsio_creat_canon(const char *name, mode_t mode) {
  char *deref = NULL;
  pool *tmp_pool = NULL;
  pr_fh_t *fh = NULL;
  pr_fs_t *fs;

  fs = lookup_file_canon_fs(name, &deref, FSIO_FILE_CREAT);
  if (fs == NULL) {
    return NULL;
  }

  /* Allocate a filehandle. */
  tmp_pool = make_sub_pool(fs->fs_pool);
  pr_pool_tag(tmp_pool, "pr_fsio_creat_canon() subpool");

  fh = pcalloc(tmp_pool, sizeof(pr_fh_t));
  fh->fh_pool = tmp_pool;
  fh->fh_path = pstrdup(fh->fh_pool, name);
  fh->fh_fd = -1;
  fh->fh_buf = NULL;
  fh->fh_fs = fs;

  /* Find the first non-NULL custom creat handler.  If there are none,
   * use the system creat.
   */
  while (fs && fs->fs_next && !fs->creat)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s creat() for path '%s'", fs->fs_name,
    name);
  fh->fh_fd = (fs->creat)(fh, deref, mode);

  if (fh->fh_fd == -1) {
    destroy_pool(fh->fh_pool);
    return NULL;
  }

  return fh;
}

pr_fh_t *pr_fsio_creat(const char *name, mode_t mode) {
  pool *tmp_pool = NULL;
  pr_fh_t *fh = NULL;
  pr_fs_t *fs;

  fs = lookup_file_fs(name, NULL, FSIO_FILE_CREAT);
  if (fs == NULL) {
    return NULL;
  }

  /* Allocate a filehandle. */
  tmp_pool = make_sub_pool(fs->fs_pool);
  pr_pool_tag(tmp_pool, "pr_fsio_creat() subpool");

  fh = pcalloc(tmp_pool, sizeof(pr_fh_t));
  fh->fh_pool = tmp_pool;
  fh->fh_path = pstrdup(fh->fh_pool, name);
  fh->fh_fd = -1;
  fh->fh_buf = NULL;
  fh->fh_fs = fs;

  /* Find the first non-NULL custom creat handler.  If there are none,
   * use the system creat.
   */
  while (fs && fs->fs_next && !fs->creat)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s creat() for path '%s'", fs->fs_name,
    name);
  fh->fh_fd = (fs->creat)(fh, name, mode);

  if (fh->fh_fd == -1) {
    destroy_pool(fh->fh_pool);
    return NULL;
  }

  return fh;
}

int pr_fsio_close(pr_fh_t *fh) {
  int res = 0;
  pr_fs_t *fs;

  if (!fh) {
    errno = EINVAL;
    return -1;
  }

  /* Find the first non-NULL custom close handler.  If there are none,
   * use the system close.
   */
  fs = fh->fh_fs;
  while (fs && fs->fs_next && !fs->close)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s close() for path '%s'", fs->fs_name,
    fh->fh_path);
  res = (fs->close)(fh, fh->fh_fd);

  destroy_pool(fh->fh_pool);
  return res;
}

int pr_fsio_read(pr_fh_t *fh, char *buf, size_t size) {
  int res;
  pr_fs_t *fs;

  if (!fh) {
    errno = EINVAL;
    return -1;
  }

  /* Find the first non-NULL custom read handler.  If there are none,
   * use the system read.
   */
  fs = fh->fh_fs;
  while (fs && fs->fs_next && !fs->read)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s read() for path '%s' (%lu bytes)",
    fs->fs_name, fh->fh_path, (unsigned long) size);
  res = (fs->read)(fh, fh->fh_fd, buf, size);

  return res;
}

int pr_fsio_write(pr_fh_t *fh, const char *buf, size_t size) {
  int res;
  pr_fs_t *fs;

  if (!fh) {
    errno = EINVAL;
    return -1;
  }

  /* Find the first non-NULL custom write handler.  If there are none,
   * use the system write.
   */
  fs = fh->fh_fs;
  while (fs && fs->fs_next && !fs->write)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s write() for path '%s' (%lu bytes)",
    fs->fs_name, fh->fh_path, (unsigned long) size);
  res = (fs->write)(fh, fh->fh_fd, buf, size);

  return res;
}

off_t pr_fsio_lseek(pr_fh_t *fh, off_t offset, int whence) {
  off_t res;
  pr_fs_t *fs;

  if (!fh) {
    errno = EINVAL;
    return -1;
  }

  /* Find the first non-NULL custom lseek handler.  If there are none,
   * use the system lseek.
   */
  fs = fh->fh_fs;
  while (fs && fs->fs_next && !fs->lseek)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s lseek() for path '%s'", fs->fs_name,
    fh->fh_path);
  res = (fs->lseek)(fh, fh->fh_fd, offset, whence);

  return res;
}

int pr_fsio_link_canon(const char *lfrom, const char *lto) {
  int res;
  pr_fs_t *from_fs, *to_fs, *fs;

  from_fs = lookup_file_fs(lfrom, NULL, FSIO_FILE_LINK);
  if (from_fs == NULL) {
    return -1;
  }

  to_fs = lookup_file_fs(lto, NULL, FSIO_FILE_LINK);
  if (to_fs == NULL) {
    return -1;
  }

  if (from_fs->allow_xdev_link == FALSE ||
      to_fs->allow_xdev_link == FALSE) {
    if (from_fs != to_fs) {
      errno = EXDEV;
      return -1;
    }
  }

  fs = to_fs;

  /* Find the first non-NULL custom link handler.  If there are none,
   * use the system link.
   */
  while (fs && fs->fs_next && !fs->link)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s link() for paths '%s', '%s'",
    fs->fs_name, lfrom, lto);
  res = (fs->link)(fs, lfrom, lto);

  return res;
}

int pr_fsio_link(const char *lfrom, const char *lto) {
  int res;
  pr_fs_t *from_fs, *to_fs, *fs;

  from_fs = lookup_file_fs(lfrom, NULL, FSIO_FILE_LINK);
  if (from_fs == NULL) {
    return -1;
  }

  to_fs = lookup_file_fs(lto, NULL, FSIO_FILE_LINK);
  if (to_fs == NULL) {
    return -1;
  }

  if (from_fs->allow_xdev_link == FALSE ||
      to_fs->allow_xdev_link == FALSE) {
    if (from_fs != to_fs) {
      errno = EXDEV;
      return -1;
    }
  }

  fs = to_fs;

  /* Find the first non-NULL custom link handler.  If there are none,
   * use the system link.
   */
  while (fs && fs->fs_next && !fs->link)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s link() for paths '%s', '%s'",
    fs->fs_name, lfrom, lto);
  res = (fs->link)(fs, lfrom, lto);

  return res;
}

int pr_fsio_symlink_canon(const char *lfrom, const char *lto) {
  int res;
  pr_fs_t *fs = lookup_file_canon_fs(lto, NULL, FSIO_FILE_SYMLINK);

  /* Find the first non-NULL custom symlink handler.  If there are none,
   * use the system symlink
   */
  while (fs && fs->fs_next && !fs->symlink)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s symlink() for path '%s'",
    fs->fs_name, lto);
  res = (fs->symlink)(fs, lfrom, lto);

  return res;
}

int pr_fsio_symlink(const char *lfrom, const char *lto) {
  int res;
  pr_fs_t *fs;

  fs = lookup_file_fs(lto, NULL, FSIO_FILE_SYMLINK);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom symlink handler.  If there are none,
   * use the system symlink.
   */
  while (fs && fs->fs_next && !fs->symlink)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s symlink() for path '%s'",
    fs->fs_name, lto);
  res = (fs->symlink)(fs, lfrom, lto);

  return res;
}

int pr_fsio_ftruncate(pr_fh_t *fh, off_t len) {
  int res;
  pr_fs_t *fs;

  if (!fh) {
    errno = EINVAL;
    return -1;
  }

  /* Find the first non-NULL custom ftruncate handler.  If there are none,
   * use the system ftruncate.
   */
  fs = fh->fh_fs;
  while (fs && fs->fs_next && !fs->ftruncate)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s ftruncate() for path '%s'",
    fs->fs_name, fh->fh_path);
  res = (fs->ftruncate)(fh, fh->fh_fd, len);

  return res;
}

int pr_fsio_truncate_canon(const char *path, off_t len) {
  int res;
  pr_fs_t *fs = lookup_file_canon_fs(path, NULL, FSIO_FILE_TRUNC);

  /* Find the first non-NULL custom truncate handler.  If there are none,
   * use the system truncate.
   */
  while (fs && fs->fs_next && !fs->truncate)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s truncate() for path '%s'",
    fs->fs_name, path);
  res = (fs->truncate)(fs, path, len);

  return res;
}

int pr_fsio_truncate(const char *path, off_t len) {
  int res;
  pr_fs_t *fs;

  fs = lookup_file_fs(path, NULL, FSIO_FILE_TRUNC);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom truncate handler.  If there are none,
   * use the system truncate.
   */
  while (fs && fs->fs_next && !fs->truncate)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s truncate() for path '%s'",
    fs->fs_name, path);
  res = (fs->truncate)(fs, path, len);
  
  return res;
}

int pr_fsio_chmod_canon(const char *name, mode_t mode) {
  int res;
  char *deref = NULL;
  pr_fs_t *fs;

  fs = lookup_file_canon_fs(name, &deref, FSIO_FILE_CHMOD);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom chmod handler.  If there are none,
   * use the system chmod.
   */
  while (fs && fs->fs_next && !fs->chmod)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s chmod() for path '%s'",
    fs->fs_name, name);
  res = (fs->chmod)(fs, deref, mode);

  if (res == 0)
    pr_fs_clear_cache();

  return res;
}

int pr_fsio_chmod(const char *name, mode_t mode) {
  int res;
  pr_fs_t *fs;

  fs = lookup_file_fs(name, NULL, FSIO_FILE_CHMOD);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom chmod handler.  If there are none,
   * use the system chmod.
   */
  while (fs && fs->fs_next && !fs->chmod)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s chmod() for path '%s'",
    fs->fs_name, name);
  res = (fs->chmod)(fs, name, mode);

  if (res == 0)
    pr_fs_clear_cache();

  return res;
}

int pr_fsio_fchmod(pr_fh_t *fh, mode_t mode) {
  int res;
  pr_fs_t *fs;

  if (!fh) {
    errno = EINVAL;
    return -1;
  }

  /* Find the first non-NULL custom fchmod handler.  If there are none, use
   * the system fchmod.
   */
  fs = fh->fh_fs;
  while (fs && fs->fs_next && !fs->fchmod)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s fchmod() for path '%s'",
    fs->fs_name, fh->fh_path);
  res = (fs->fchmod)(fh, fh->fh_fd, mode);

  if (res == 0)
    pr_fs_clear_cache();

  return res;
}

int pr_fsio_chown_canon(const char *name, uid_t uid, gid_t gid) {
  int res;
  pr_fs_t *fs;

  fs = lookup_file_canon_fs(name, NULL, FSIO_FILE_CHOWN);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom chown handler.  If there are none,
   * use the system chown.
   */
  while (fs && fs->fs_next && !fs->chown)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s chown() for path '%s'",
    fs->fs_name, name);
  res = (fs->chown)(fs, name, uid, gid);

  if (res == 0)
    pr_fs_clear_cache();

  return res;
}

int pr_fsio_chown(const char *name, uid_t uid, gid_t gid) {
  int res;
  pr_fs_t *fs;

  fs = lookup_file_fs(name, NULL, FSIO_FILE_CHOWN);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom chown handler.  If there are none,
   * use the system chown.
   */
  while (fs && fs->fs_next && !fs->chown)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s chown() for path '%s'",
    fs->fs_name, name);
  res = (fs->chown)(fs, name, uid, gid);

  if (res == 0)
    pr_fs_clear_cache();

  return res;
}

int pr_fsio_fchown(pr_fh_t *fh, uid_t uid, gid_t gid) {
  int res;
  pr_fs_t *fs;

  if (!fh) {
    errno = EINVAL;
    return -1;
  }

  /* Find the first non-NULL custom fchown handler.  If there are none, use
   * the system fchown.
   */
  fs = fh->fh_fs;
  while (fs && fs->fs_next && !fs->fchown)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s fchown() for path '%s'",
    fs->fs_name, fh->fh_path);
  res = (fs->fchown)(fh, fh->fh_fd, uid, gid);

  if (res == 0)
    pr_fs_clear_cache();

  return res;
}

int pr_fsio_access(const char *path, int mode, uid_t uid, gid_t gid,
    array_header *suppl_gids) {
  pr_fs_t *fs;

  if (!path) {
    errno = EINVAL;
    return -1;
  }

  fs = lookup_file_fs(path, NULL, FSIO_FILE_ACCESS);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom access handler.  If there are none,
   * use the system access.
   */
  while (fs && fs->fs_next && !fs->access)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s access() for path '%s'",
    fs->fs_name, path);
  return (fs->access)(fs, path, mode, uid, gid, suppl_gids);
}

int pr_fsio_faccess(pr_fh_t *fh, int mode, uid_t uid, gid_t gid,
    array_header *suppl_gids) {
  pr_fs_t *fs;

  if (!fh) {
    errno = EINVAL;
    return -1;
  }

  /* Find the first non-NULL custom faccess handler.  If there are none,
   * use the system faccess.
   */
  fs = fh->fh_fs;
  while (fs && fs->fs_next && !fs->faccess)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s faccess() for path '%s'",
    fs->fs_name, fh->fh_path);
  return (fs->faccess)(fh, mode, uid, gid, suppl_gids);
}

int pr_fsio_utimes(const char *path, struct timeval *tvs) {
  int res;
  pr_fs_t *fs;

  if (path == NULL ||
      tvs == NULL) {
    errno = EINVAL;
    return -1;
  }

  fs = lookup_file_fs(path, NULL, FSIO_FILE_UTIMES);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom utimes handler.  If there are none,
   * use the system utimes.
   */
  while (fs && fs->fs_next && !fs->utimes)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s utimes() for path '%s'",
    fs->fs_name, path);
  res = (fs->utimes)(fs, path, tvs);

  if (res == 0)
    pr_fs_clear_cache();

  return res;
}

int pr_fsio_futimes(pr_fh_t *fh, struct timeval *tvs) {
  int res;
  pr_fs_t *fs;

  if (fh == NULL ||
      tvs == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* Find the first non-NULL custom futimes handler.  If there are none,
   * use the system futimes.
   */
  fs = fh->fh_fs;
  while (fs && fs->fs_next && !fs->futimes)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s futimes() for path '%s'",
    fs->fs_name, fh->fh_path);
  res = (fs->futimes)(fh, fh->fh_fd, tvs);

  if (res == 0)
    pr_fs_clear_cache();

  return res;
}

/* If the wrapped chroot() function suceeds (eg returns 0), then all
 * pr_fs_ts currently registered in the fs_map will have their paths
 * rewritten to reflect the new root.
 */
int pr_fsio_chroot(const char *path) {
  int res = 0;
  pr_fs_t *fs;

  fs = lookup_dir_fs(path, FSIO_DIR_CHROOT);
  if (fs == NULL) {
    return -1;
  }

  /* Find the first non-NULL custom chroot handler.  If there are none,
   * use the system chroot.
   */
  while (fs && fs->fs_next && !fs->chroot)
    fs = fs->fs_next;

  pr_trace_msg(trace_channel, 8, "using %s chroot() for path '%s'",
    fs->fs_name, path);
  res = (fs->chroot)(fs, path);
  if (res == 0) {
    unsigned int iter_start = 0;

    /* The filesystem handles in fs_map need to be readjusted to the new root.
     */
    register unsigned int i = 0;
    pool *map_pool = make_sub_pool(permanent_pool);
    array_header *new_map = make_array(map_pool, 0, sizeof(pr_fs_t *));
    pr_fs_t **fs_objs = NULL;

    pr_pool_tag(map_pool, "FSIO Map Pool");

    if (fs_map)
      fs_objs = (pr_fs_t **) fs_map->elts;

    if (fs != root_fs) {
      if (strncmp(fs->fs_path, path, strlen(path)) == 0) {
        memmove(fs->fs_path, fs->fs_path + strlen(path),
          strlen(fs->fs_path) - strlen(path) + 1);
      }

      *((pr_fs_t **) push_array(new_map)) = fs;
      iter_start = 1;
    }

    for (i = iter_start; i < (fs_map ? fs_map->nelts : 0); i++) {
      pr_fs_t *tmpfs = fs_objs[i];

      /* The memory for this field has already been allocated, so futzing
       * with it like this should be fine.  Watch out for any paths that
       * may be different, e.g. added manually, not through pr_register_fs().
       * Any absolute paths that are outside of the chroot path are discarded.
       * Deferred-resolution paths (eg "~" paths) and relative paths are kept.
       */

      if (strncmp(tmpfs->fs_path, path, strlen(path)) == 0) {
        pr_fs_t *next;

        memmove(tmpfs->fs_path, tmpfs->fs_path + strlen(path),
          strlen(tmpfs->fs_path) - strlen(path) + 1);

        /* Need to do this for any stacked FSs as well. */
        next = tmpfs->fs_next;
        while (next) {
          pr_signals_handle();

          memmove(next->fs_path, next->fs_path + strlen(path),
            strlen(next->fs_path) - strlen(path) + 1);

          next = next->fs_next;
        }
      }

      /* Add this FS to the new fs_map. */
      *((pr_fs_t **) push_array(new_map)) = tmpfs;
    }

    /* Sort the new map */
    qsort(new_map->elts, new_map->nelts, sizeof(pr_fs_t *), fs_cmp);

    /* Destroy the old map */
    if (fs_map)
      destroy_pool(fs_map->pool);

    fs_map = new_map;
    chk_fs_map = TRUE;
  }

  return res;
}

char *pr_fsio_gets(char *buf, size_t size, pr_fh_t *fh) {
  char *bp = NULL;
  int toread = 0;
  pr_buffer_t *pbuf = NULL;

  if (!buf || !fh || size <= 0) {
    errno = EINVAL;
    return NULL;
  }

  if (!fh->fh_buf) {
    size_t bufsz;

    /* Conscientious callers who want the optimal IO on the file should
     * set the fh->fh_iosz hint.
     */
    bufsz = fh->fh_iosz ? fh->fh_iosz : PR_TUNABLE_BUFFER_SIZE;

    fh->fh_buf = pcalloc(fh->fh_pool, sizeof(pr_buffer_t));
    fh->fh_buf->buf = fh->fh_buf->current = pcalloc(fh->fh_pool, bufsz);
    fh->fh_buf->remaining = fh->fh_buf->buflen = bufsz;
  }

  pbuf = fh->fh_buf;
  bp = buf;

  while (size) {
    pr_signals_handle();

    if (!pbuf->current ||
        pbuf->remaining == pbuf->buflen) { /* empty buffer */

      toread = pr_fsio_read(fh, pbuf->buf,
        size < pbuf->buflen ? size : pbuf->buflen);

      if (toread <= 0) {
        if (bp != buf) {
          *bp = '\0';
          return buf;

        } else
          return NULL;
      }

      pbuf->remaining = pbuf->buflen - toread;
      pbuf->current = pbuf->buf;

    } else
      toread = pbuf->buflen - pbuf->remaining;

    while (size &&
           toread > 0 &&
           *pbuf->current != '\n' &&
           toread--) {
      pr_signals_handle();

      *bp++ = *pbuf->current++;
      size--;
      pbuf->remaining++;
    }

    if (size &&
        toread &&
        *pbuf->current == '\n') {
      size--;
      toread--;
      *bp++ = *pbuf->current++;
      pbuf->remaining++;
      break;
    }

    if (!toread)
      pbuf->current = NULL;
  }

  *bp = '\0';
  return buf;
}

/* pr_fsio_getline() is an fgets() with backslash-newline stripping, copied from
 * Wietse Venema's tcpwrapppers-7.6 code.  The extra *lineno argument is
 * needed, at the moment, to properly track which line of the configuration
 * file is being read in, so that errors can be reported with line numbers
 * correctly.
 */
char *pr_fsio_getline(char *buf, int buflen, pr_fh_t *fh,
    unsigned int *lineno) {
  int inlen;
  char *start = buf;

  while (pr_fsio_gets(buf, buflen, fh)) {
    pr_signals_handle();

    inlen = strlen(buf);

    if (inlen >= 1) {
      if (buf[inlen - 1] == '\n') {
        (*lineno)++;

        if (inlen >= 2 && buf[inlen - 2] == '\\') {
          char *bufp;

          inlen -= 2;
      
          /* Watch for commented lines when handling line continuations.
           * Advance past any leading whitespace, to see if the first
           * non-whitespace character is the comment character.
           */
          for (bufp = buf; *bufp && isspace((int) *bufp); bufp++);

          if (*bufp == '#')
             continue;
 
        } else
          return start;
      }
    }

    /* Be careful of reading too much. */
    if (buflen - inlen == 0)
      return buf;

    buf += inlen;
    buflen -= inlen;
    buf[0] = 0;
  }

  return (buf > start ? start : 0);
}

/* Be generous in the maximum allowed number of dup fds, in our search for
 * one that is outside the big three.
 *
 * In theory, this should be a runtime lookup using getdtablesize(2), being
 * sure to handle the ENOSYS case (for older systems).
 */
#define FSIO_MAX_DUPFDS		512

/* The main three fds (stdin, stdout, stderr) need to be protected, reserved
 * for use.  This function uses dup(2) to open new fds on the given fd
 * until the new fd is not one of the big three.
 */
int pr_fs_get_usable_fd(int fd) {
  register unsigned int i;
  int fdi, dup_fds[FSIO_MAX_DUPFDS], n; 

  if (fd > STDERR_FILENO) {
    return fd;
  }
 
  memset(dup_fds, -1, sizeof(dup_fds));
  i = 0;
  n = -1;

  fdi = fd;
  while (i < FSIO_MAX_DUPFDS) {
    pr_signals_handle();

    dup_fds[i] = dup(fdi);
    if (dup_fds[i] < 0) {
      register unsigned int j;
      int xerrno  = errno;

      /* Need to clean up any previously opened dups as well. */
      for (j = 0; j <= i; j++) {
        close(dup_fds[j]);
        dup_fds[j] = -1;
      }

      errno = xerrno;
      return -1;
    }

    if (dup_fds[i] <= STDERR_FILENO) {
      /* Continue searching for an open fd that isn't 0, 1, or 2. */
      fdi = dup_fds[i];
      i++;
      continue;
    }

    n = i;
    fdi = dup_fds[n];
    break;
  }

  /* If n is -1, we reached the max number of dups without finding an
   * open one.  Hard to imagine this happening, but catch the case anyway.
   */
  if (n == -1) {
    /* Free up the fds we opened in our search. */
    for (i = 0; i < FSIO_MAX_DUPFDS; i++) {
      if (dup_fds[i] >= 0) {
        close(dup_fds[i]);
        dup_fds[i] = -1;
      }
    }

    errno = EPERM;
    return -1;
  }

  /* Free up the fds we opened in our search. */
  for (i = 0; i < n; i++) {
    close(dup_fds[i]);
    dup_fds[i] = -1;
  }

  return fdi;
}

/* Simple multiplication and division doesn't work with very large
 * filesystems (overflows 32 bits).  This code should handle it.
 */
static off_t calc_fs_size(size_t blocks, size_t bsize) {
  off_t bl_lo, bl_hi;
  off_t res_lo, res_hi, tmp;

  bl_lo = blocks & 0x0000ffff;
  bl_hi = blocks & 0xffff0000;

  tmp = (bl_hi >> 16) * bsize;
  res_hi = tmp & 0xffff0000;
  res_lo = (tmp & 0x0000ffff) << 16;
  res_lo += bl_lo * bsize;

  if (res_hi & 0xfc000000)
    /* Overflow */
    return 0;

  return (res_lo >> 10) | (res_hi << 6);
}

static int fs_getsize(char *path, off_t *fs_size) {
# if defined(HAVE_SYS_STATVFS_H)

#  if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS == 64 && \
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
#  endif /* LFS && !Solaris 2.5.1 && !Solaris 2.6 && !Solaris 2.7 */

  pr_trace_msg(trace_channel, 18, "using statvfs() on '%s'", path);
  if (statvfs(path, &fs) < 0) {
    int xerrno = errno;

    pr_trace_msg(trace_channel, 3, "statvfs() error using '%s': %s",
      path, strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  /* The calc_fs_size() function is only useful for 32-bit numbers;
   * if either of our two values are in datatypes larger than 4 bytes,
   * we'll use typecasting.
   */
  if (sizeof(fs.f_bavail) > 4 ||
      sizeof(fs.f_frsize) > 4) {
    *fs_size = ((off_t) fs.f_bavail * (off_t) fs.f_frsize);

  } else {
    *fs_size = calc_fs_size(fs.f_bavail, fs.f_frsize);
  }

  return 0;

# elif defined(HAVE_SYS_VFS_H)
  struct statfs fs;

  pr_trace_msg(trace_channel, 18, "using statfs() on '%s'", path);
  if (statfs(path, &fs) < 0) {
    int xerrno = errno;

    pr_trace_msg(trace_channel, 3, "statfs() error using '%s': %s",
      path, strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  /* The calc_fs_size() function is only useful for 32-bit numbers;
   * if either of our two values are in datatypes larger than 4 bytes,
   * we'll use typecasting.
   */
  if (sizeof(fs.f_bavail) > 4 ||
      sizeof(fs.f_bsize) > 4) {
    *fs_size = ((off_t) fs.f_bavail * (off_t) fs.f_bsize);

  } else {
    *fs_size = calc_fs_size(fs.f_bavail, fs.f_bsize);
  }

  return 0;

# elif defined(HAVE_STATFS)
  struct statfs fs;

  pr_trace_msg(trace_channel, 18, "using statfs() on '%s'", path);
  if (statfs(path, &fs) < 0) {
    int xerrno = errno;

    pr_trace_msg(trace_channel, 3, "statfs() error using '%s': %s",
      path, strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  /* The calc_fs_size() function is only useful for 32-bit numbers;
   * if either of our two values are in datatypes larger than 4 bytes,
   * we'll use typecasting.
   */
  if (sizeof(fs.f_bavail) > 4 ||
      sizeof(fs.f_bsize) > 4) {
    *fs_size = ((off_t) fs.f_bavail * (off_t) fs.f_bsize);

  } else {
    *fs_size = calc_fs_size(fs.f_bavail, fs.f_bsize);
  }

  return 0;

# endif /* !HAVE_STATFS && !HAVE_SYS_STATVFS && !HAVE_SYS_VFS */
  errno = ENOSYS;
  return -1;
}

#if defined(HAVE_STATFS) || defined(HAVE_SYS_STATVFS_H) || \
  defined(HAVE_SYS_VFS_H)
off_t pr_fs_getsize(char *path) {
  int res;
  off_t fs_size;

  res = pr_fs_getsize2(path, &fs_size);
  if (res < 0) {
    fs_size = 0;
  }

  return fs_size;
}
#endif /* !HAVE_STATFS && !HAVE_SYS_STATVFS && !HAVE_SYS_VFS */

int pr_fs_getsize2(char *path, off_t *fs_size) {
  return fs_getsize(path, fs_size);
}

int pr_fsio_puts(const char *buf, pr_fh_t *fh) {
  if (!fh) {
    errno = EINVAL;
    return -1;
  }

  return pr_fsio_write(fh, buf, strlen(buf));
}

int pr_fsio_set_block(pr_fh_t *fh) {
  int flags, res;

  if (fh == NULL) {
    errno = EINVAL;
    return -1;
  }

  flags = fcntl(fh->fh_fd, F_GETFL);
  res = fcntl(fh->fh_fd, F_SETFL, flags & (U32BITS ^ O_NONBLOCK));

  return res;
}

void pr_resolve_fs_map(void) {
  register unsigned int i = 0;

  if (!fs_map)
    return;

  for (i = 0; i < fs_map->nelts; i++) {
    char *newpath = NULL;
    unsigned char add_slash = FALSE;
    pr_fs_t *tmpfs = ((pr_fs_t **) fs_map->elts)[i];

    /* Skip if this fs is the root fs. */
    if (tmpfs == root_fs)
      continue;

    /* Note that dir_realpath() does _not_ handle "../blah" paths
     * well, so...at least for now, hope that such paths are screened
     * by the code adding such paths into the fs_map.  Check for
     * a trailing slash in the unadjusted path, so that I know if I need
     * to re-add that slash to the adjusted path -- these trailing slashes
     * are important!
     */
    if ((strncmp(tmpfs->fs_path, "/", 2) != 0 &&
        (tmpfs->fs_path)[strlen(tmpfs->fs_path) - 1] == '/'))
      add_slash = TRUE;

    newpath = dir_realpath(tmpfs->fs_pool, tmpfs->fs_path);

    if (add_slash)
      newpath = pstrcat(tmpfs->fs_pool, newpath, "/", NULL);

    /* Note that this does cause a slightly larger memory allocation from
     * the pr_fs_t's pool, as the original path value was also allocated
     * from that pool, and that original pointer is being overwritten.
     * However, as this function is only called once, and that pool
     * is freed later, I think this may be acceptable.
     */
    tmpfs->fs_path = newpath;
  }

  /* Resort the map */
  qsort(fs_map->elts, fs_map->nelts, sizeof(pr_fs_t *), fs_cmp);

  return;
}

int init_fs(void) {
  char cwdbuf[PR_TUNABLE_PATH_MAX + 1] = {'\0'};

  /* Establish the default pr_fs_t that will handle any path */
  root_fs = pr_create_fs(permanent_pool, "system");
  if (root_fs == NULL) {

    /* Do not insert this fs into the FS map.  This will allow other
     * modules to insert filesystems at "/", if they want.
     */
    pr_log_pri(PR_LOG_ERR, "error: unable to initialize default fs");
    exit(1);
  }

  root_fs->fs_path = pstrdup(root_fs->fs_pool, "/");

  /* Set the root FSIO handlers. */
  root_fs->stat = sys_stat;
  root_fs->fstat = sys_fstat;
  root_fs->lstat = sys_lstat;
  root_fs->rename = sys_rename;
  root_fs->unlink = sys_unlink;
  root_fs->open = sys_open;
  root_fs->creat = sys_creat;
  root_fs->close = sys_close;
  root_fs->read = sys_read;
  root_fs->write = sys_write;
  root_fs->lseek = sys_lseek;
  root_fs->link = sys_link;
  root_fs->readlink = sys_readlink;
  root_fs->symlink = sys_symlink;
  root_fs->ftruncate = sys_ftruncate;
  root_fs->truncate = sys_truncate;
  root_fs->chmod = sys_chmod;
  root_fs->fchmod = sys_fchmod;
  root_fs->chown = sys_chown;
  root_fs->fchown = sys_fchown;
  root_fs->access = sys_access;
  root_fs->faccess = sys_faccess;
  root_fs->utimes = sys_utimes;
  root_fs->futimes = sys_futimes;

  root_fs->chdir = sys_chdir;
  root_fs->chroot = sys_chroot;
  root_fs->opendir = sys_opendir;
  root_fs->closedir = sys_closedir;
  root_fs->readdir = sys_readdir;
  root_fs->mkdir = sys_mkdir;
  root_fs->rmdir = sys_rmdir;

  if (getcwd(cwdbuf, sizeof(cwdbuf)-1)) {
    cwdbuf[sizeof(cwdbuf)-1] = '\0';
    pr_fs_setcwd(cwdbuf);

  } else {
    pr_fsio_chdir("/", FALSE);
    pr_fs_setcwd("/");
  }

  return 0;
}

#ifdef PR_USE_DEVEL

static const char *get_fs_hooks_str(pool *p, pr_fs_t *fs) {
  char *hooks = "";

  if (fs->stat)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "stat(2)", NULL);

  if (fs->lstat)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "lstat(2)", NULL);

  if (fs->fstat)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "fstat(2)", NULL);

  if (fs->rename)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "rename(2)", NULL);

  if (fs->link)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "link(2)", NULL);

  if (fs->unlink)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "unlink(2)", NULL);

  if (fs->open)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "open(2)", NULL);

  if (fs->creat)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "creat(2)", NULL);

  if (fs->close)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "close(2)", NULL);

  if (fs->read)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "read(2)", NULL);

  if (fs->lseek)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "lseek(2)", NULL);

  if (fs->readlink)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "readlink(2)", NULL);

  if (fs->symlink)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "symlink(2)", NULL);

  if (fs->ftruncate)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "ftruncate(2)", NULL);

  if (fs->truncate)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "truncate(2)", NULL);

  if (fs->chmod)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "chmod(2)", NULL);

  if (fs->chown)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "chown(2)", NULL);

  if (fs->access)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "access(2)", NULL);

  if (fs->faccess)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "faccess(2)", NULL);

  if (fs->utimes)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "utimes(2)", NULL);

  if (fs->futimes)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "futimes(3)", NULL);

  if (fs->chdir)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "chdir(2)", NULL);

  if (fs->chroot)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "chroot(2)", NULL);

  if (fs->opendir)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "opendir(3)", NULL);

  if (fs->closedir)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "closedir(3)", NULL);

  if (fs->readdir)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "readdir(3)", NULL);

  if (fs->mkdir)
    hooks = pstrcat(p, hooks, *hooks ? ", " : "", "mkdir(2)", NULL);

  if (!*hooks) {
    return pstrdup(p, "(none)");
  }

  return hooks;
}

static void get_fs_info(pool *p, int depth, pr_fs_t *fs,
    void (*dumpf)(const char *, ...)) {

  dumpf("FS#%u: '%s', mounted at '%s', implementing the following hooks:",
    depth, fs->fs_name, fs->fs_path);
  dumpf("FS#%u:    %s", depth, get_fs_hooks_str(p, fs));
}

void pr_fs_dump(void (*dumpf)(const char *, ...)) {
  pool *p;

  dumpf("FS#0: 'system' mounted at '/', implementing the following hooks:");
  dumpf("FS#0:    (all)");

  if (!fs_map ||
      fs_map->nelts == 0)
    return;

  p = make_sub_pool(permanent_pool);

  if (fs_map->nelts > 0) {
    pr_fs_t **fs_objs = (pr_fs_t **) fs_map->elts;
    register int i;

    for (i = 0; i < fs_map->nelts; i++) {
      pr_fs_t *fsi = fs_objs[i];

      for (; fsi->fs_next; fsi = fsi->fs_next) {
        get_fs_info(p, i+1, fsi, dumpf);
      }
    }
  }

  destroy_pool(p);
}
#endif /* PR_USE_DEVEL */
