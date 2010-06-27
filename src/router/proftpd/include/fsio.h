/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2009 The ProFTPD Project
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

/* ProFTPD virtual/modular filesystem support.
 *
 * $Id: fsio.h,v 1.24 2009/11/05 17:46:54 castaglia Exp $
 */

#ifndef PR_FSIO_H
#define PR_FSIO_H

#include "conf.h"

#ifdef HAVE_REGEX_H
#include <regex.h>
#endif

/* This is a Tru64-specific hack, to work around some macro funkiness
 * in their /usr/include/sys/mount.h header.
 */
#ifdef OSF5
# undef fh_data
#endif

/* Operation codes */
#define FSIO_FILE_STAT		(1 << 0)
#define FSIO_FILE_LSTAT		(1 << 1)
#define FSIO_FILE_RENAME	(1 << 2)
#define FSIO_FILE_UNLINK	(1 << 3)
#define FSIO_FILE_OPEN		(1 << 4)
#define FSIO_FILE_CREAT		(1 << 5)
#define FSIO_FILE_CLOSE		(1 << 6)
#define FSIO_FILE_READ		(1 << 7)
#define FSIO_FILE_WRITE		(1 << 8)
#define FSIO_FILE_LINK		(1 << 9)
#define FSIO_FILE_SYMLINK	(1 << 10)
#define FSIO_FILE_READLINK	(1 << 11)
#define FSIO_FILE_TRUNC		(1 << 12)
#define FSIO_FILE_CHMOD		(1 << 13)
#define FSIO_FILE_CHOWN		(1 << 14)
#define FSIO_FILE_ACCESS	(1 << 15)
#define FSIO_FILE_UTIMES	(1 << 23)

/* Macro that defines the most common file ops */
#define FSIO_FILE_COMMON	(FSIO_FILE_OPEN|FSIO_FILE_READ|FSIO_FILE_WRITE|\
                                 FSIO_FILE_CLOSE|FSIO_FILE_CREAT)

#define FSIO_DIR_CHROOT		(1 << 16)
#define FSIO_DIR_CHDIR		(1 << 17)
#define FSIO_DIR_OPENDIR	(1 << 18)
#define FSIO_DIR_CLOSEDIR	(1 << 19)
#define FSIO_DIR_READDIR	(1 << 20)
#define FSIO_DIR_MKDIR		(1 << 21)
#define FSIO_DIR_RMDIR		(1 << 22)

/* Macro that defines directory operations */
#define FSIO_DIR_COMMON		(FSIO_DIR_CHROOT|FSIO_DIR_CHDIR|\
                                 FSIO_DIR_OPENDIR|FSIO_DIR_READDIR|\
                                 FSIO_DIR_CLOSEDIR|FSIO_DIR_MKDIR|\
                                 FSIO_DIR_RMDIR)

/* Default mode used when creating files */
#define PR_OPEN_MODE		0666

/* Modular filesystem object */

typedef struct fs_rec pr_fs_t;
typedef struct fh_rec pr_fh_t;

struct fs_rec {

  /* These pointers will be effective once layered FS modules are
   * supported
   */
  pr_fs_t *fs_next, *fs_prev;

  /* Descriptive tag for this fs object */
  char *fs_name;

  char *fs_path;

  /* Slot for module-specific data */
  void *fs_data;

  /* Pool for this object's use */
  pool *fs_pool;

  /* FS function pointers */
  int (*stat)(pr_fs_t *, const char *, struct stat *);
  int (*fstat)(pr_fh_t *, int, struct stat *);
  int (*lstat)(pr_fs_t *, const char *, struct stat *);
  int (*rename)(pr_fs_t *, const char *, const char *);
  int (*unlink)(pr_fs_t *, const char *);
  int (*open)(pr_fh_t *, const char *, int);
  int (*creat)(pr_fh_t *, const char *, mode_t);
  int (*close)(pr_fh_t *, int);
  int (*read)(pr_fh_t *, int, char *, size_t);
  int (*write)(pr_fh_t *, int, const char *, size_t);
  off_t (*lseek)(pr_fh_t *, int, off_t, int);
  int (*link)(pr_fs_t *, const char *, const char *);
  int (*readlink)(pr_fs_t *, const char *, char *, size_t);
  int (*symlink)(pr_fs_t *, const char *, const char *);
  int (*ftruncate)(pr_fh_t *, int, off_t);
  int (*truncate)(pr_fs_t *, const char *, off_t);
  int (*chmod)(pr_fs_t *, const char *, mode_t);
  int (*fchmod)(pr_fh_t *, int, mode_t);
  int (*chown)(pr_fs_t *, const char *, uid_t, gid_t);
  int (*fchown)(pr_fh_t *, int, uid_t, gid_t);
  int (*access)(pr_fs_t *, const char *, int, uid_t, gid_t, array_header *);
  int (*faccess)(pr_fh_t *, int, uid_t, gid_t, array_header *);
  int (*utimes)(pr_fs_t *, const char *, struct timeval *);
  int (*futimes)(pr_fh_t *, int, struct timeval *);

  /* For actual operations on the directory (or subdirs)
   * we cast the return from opendir to DIR* in src/fs.c, so
   * modules can use their own data type
   */

  int (*chdir)(pr_fs_t *, const char *);
  int (*chroot)(pr_fs_t *, const char *);
  void *(*opendir)(pr_fs_t *, const char *);
  int (*closedir)(pr_fs_t *, void *);
  struct dirent *(*readdir)(pr_fs_t *, void *);
  int (*mkdir)(pr_fs_t *, const char *, mode_t);
  int (*rmdir)(pr_fs_t *, const char *);

  /* This flag determines whether this FS handler allows cross-FS hardlinks,
   * either from this FS to another FS, or from another FS to this FS.
   *
   * If the flag is set to FALSE by the FS registrant, then a hardlink
   * across FS handlers will fail, with errno set to EXDEV.  The caller
   * will then have to handle the EXDEV error appropriately.
   */
  int allow_xdev_link;

  /* This flag determines whether this FS handler allows cross-FS renames,
   * either from this FS to another FS, or from another FS to this FS.
   *
   * If the flag is set to FALSE by the FS registrant, then a rename
   * across FS handlers will fail, with errno set to EXDEV.  The caller
   * will then have to handle the EXDEV error appropriately.
   *
   * In the core engine, a RNFR/RNTO sequence which encounters an EXDEV
   * error will cause a copy/delete of the file.  This can be more IO
   * intensive than expected, and lead to longer times for the RNTO
   * command to complete.
   */
  int allow_xdev_rename;
};

struct fh_rec {

  /* Pool for this object's use */
  pool *fh_pool;

  int fh_fd;
  char *fh_path;

  /* Arbitrary data associated with this file. */
  void *fh_data;

  /* Pointer to the filesystem in which this file is located. */
  pr_fs_t *fh_fs;

  /* For buffer I/O on this file, should anything choose to use it. */
  pr_buffer_t *fh_buf;

  /* Hint of the optimal buffer size for IO on this file. */
  size_t fh_iosz;
};

/* Macros for that code that needs to get into the internals of pr_fs_t.
 * (These will help keep the internals as opaque as possible).
 */
#define PR_FH_FD(f)	((f)->fh_fd)

#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
#ifdef PR_FS_MATCH
typedef struct fs_match_rec pr_fs_match_t;
struct fs_match_rec {

  pr_fs_match_t *fsm_next, *fsm_prev;

  /* pool for this object's use */
  pool *fsm_pool;

  /* descriptive tag for this fs regex */
  char *fsm_name;

  /* mask of the fs operations to which this regex should apply */
  int fsm_opmask;

  /* string containing the match pattern */
  char *fsm_pattern;

  /* compiled pattern (regex) */
  regex_t *fsm_regex;

  /* "trigger" function to be called whenever a path that matches the
   * compiled regex is given.
   */
  int (*trigger)(pr_fh_t *, const char *, int);

  /* NOTE: need some way of keeping track of the pr_fs_t registered by
   *  an fs_match's trigger function, such that when an fs_match is
   *  removed, its registered pr_fs_t's are removed as well.
   */
  array_header *fsm_fs_objs;
};
#endif /* PR_FS_MATCH */
#endif /* HAVE_REGEX_H && HAVE_REGCOMP */

int pr_fsio_stat(const char *, struct stat *);
int pr_fsio_stat_canon(const char *, struct stat *);
int pr_fsio_fstat(pr_fh_t *, struct stat *);
int pr_fsio_lstat(const char *, struct stat *);
int pr_fsio_lstat_canon(const char *, struct stat *);
int pr_fsio_readlink(const char *, char *, size_t);
int pr_fsio_readlink_canon(const char *, char *, size_t);
int pr_fsio_chdir(const char *, int);
int pr_fsio_chdir_canon(const char *, int);
void *pr_fsio_opendir(const char *);
int pr_fsio_closedir(void *);
struct dirent *pr_fsio_readdir(void *);
int pr_fsio_mkdir(const char *, mode_t);
int pr_fsio_rmdir(const char *);
int pr_fsio_rename(const char *, const char *);
int pr_fsio_rename_canon(const char *, const char *);
int pr_fsio_unlink(const char *);
int pr_fsio_unlink_canon(const char *);
pr_fh_t *pr_fsio_open(const char *, int);
pr_fh_t *pr_fsio_open_canon(const char *, int);
pr_fh_t *pr_fsio_creat(const char *, mode_t);
pr_fh_t *pr_fsio_creat_canon(const char *, mode_t);
int pr_fsio_close(pr_fh_t *);
int pr_fsio_read(pr_fh_t *, char *, size_t);
int pr_fsio_write(pr_fh_t *, const char *, size_t);
int pr_fsio_link(const char *, const char *);
int pr_fsio_link_canon(const char *, const char *);
int pr_fsio_symlink(const char *, const char *);
int pr_fsio_symlink_canon(const char *, const char *);
int pr_fsio_ftruncate(pr_fh_t *, off_t);
int pr_fsio_truncate(const char *, off_t);
int pr_fsio_truncate_canon(const char *, off_t);
int pr_fsio_chmod(const char *, mode_t);
int pr_fsio_fchmod(pr_fh_t *, mode_t);
int pr_fsio_chmod_canon(const char *, mode_t);
int pr_fsio_chown(const char *, uid_t, gid_t);
int pr_fsio_fchown(pr_fh_t *, uid_t, gid_t);
int pr_fsio_chown_canon(const char *, uid_t, gid_t);
int pr_fsio_chroot(const char *);
int pr_fsio_access(const char *, int, uid_t, gid_t, array_header *);
int pr_fsio_faccess(pr_fh_t *, int, uid_t, gid_t, array_header *);
int pr_fsio_utimes(const char *, struct timeval *);
int pr_fsio_futimes(pr_fh_t *, struct timeval *);
off_t pr_fsio_lseek(pr_fh_t *, off_t, int);

/* FS-related functions */

char *pr_fsio_getline(char *, int, pr_fh_t *, unsigned int *);
char *pr_fsio_gets(char *, size_t, pr_fh_t *);
int pr_fsio_puts(const char *, pr_fh_t *);
int pr_fsio_set_block(pr_fh_t *);

pr_fs_t *pr_register_fs(pool *, const char *, const char *);
pr_fs_t *pr_create_fs(pool *, const char *);
pr_fs_t *pr_get_fs(const char *, int *);
int pr_insert_fs(pr_fs_t *, const char *);
pr_fs_t *pr_remove_fs(const char *);
pr_fs_t *pr_unmount_fs(const char *, const char *);
int pr_unregister_fs(const char *);

#if defined(HAVE_REGEX_H) && defined(HAVE_RECOMP)
#ifdef PR_FS_MATCH
pr_fs_match_t *pr_register_fs_match(pool *, const char *, const char *, int);
void pr_associate_fs(pr_fs_match_t *, pr_fs_t *);
pr_fs_match_t *pr_create_fs_match(pool *, const char *, const char *, int);
pr_fs_match_t *pr_get_fs_match(const char *, int);
pr_fs_match_t *pr_get_next_fs_match(pr_fs_match_t *, const char *, int);
int pr_insert_fs_match(pr_fs_match_t *);
int pr_unregister_fs_match(const char *);
#endif /* PR_FS_MATCH */
#endif /* HAVE_REGEX_H && HAVE_REGCOMP */

void pr_fs_clear_cache(void);
int pr_fs_copy_file(const char *, const char *);
void pr_fs_setcwd(const char *);
const char *pr_fs_getcwd(void);
const char *pr_fs_getvwd(void);
int pr_fs_dircat(char *, int, const char *, const char *);
int pr_fs_interpolate(const char *, char *, size_t);
int pr_fs_resolve_partial(const char *, char *, size_t, int);
int pr_fs_resolve_path(const char *, char *, size_t, int);
char *pr_fs_decode_path(pool *, const char *);
char *pr_fs_encode_path(pool *, const char *);
int pr_fs_use_encoding(int bool);
int pr_fs_valid_path(const char *);
void pr_fs_virtual_path(const char *, char *, size_t);
void pr_fs_clean_path(const char *, char *, size_t);
int pr_fs_glob(const char *, int, int (*errfunc)(const char *, int), glob_t *);
void pr_fs_globfree(glob_t *);
void pr_resolve_fs_map(void);

/* The main three fds (stdin, stdout, stderr) need to be protected, reserved
 * for use.  This function uses dup(2) to open new fds on the given fd
 * until the new fd is not one of the big three.
 */
int pr_fs_get_usable_fd(int);

#if defined(HAVE_STATFS) || defined(HAVE_SYS_STATVFS_H) || \
  defined(HAVE_SYS_VFS_H)
off_t pr_fs_getsize(char *);
#endif

/* For internal use only. */
int init_fs(void);

#ifdef PR_USE_DEVEL
void pr_fs_dump(void (*)(const char *, ...));
#endif /* PR_USE_DEVEL */

#endif /* PR_FSIO_H */
