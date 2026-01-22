/*
 * fuse2fs.c - FUSE server for e2fsprogs.
 *
 * Copyright (C) 2014 Oracle.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "config.h"
#include <pthread.h>
#ifdef __linux__
# include <linux/fs.h>
# include <linux/falloc.h>
# include <linux/xattr.h>
#endif
#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif
#include <sys/ioctl.h>
#include <unistd.h>
#include <ctype.h>
#define FUSE_DARWIN_ENABLE_EXTENSIONS 0
#ifdef __SET_FOB_FOR_FUSE
# error Do not set magic value __SET_FOB_FOR_FUSE!!!!
#endif
#ifndef _FILE_OFFSET_BITS
/*
 * Old versions of libfuse (e.g. Debian 2.9.9 package) required that the build
 * system set _FILE_OFFSET_BITS explicitly, even if doing so isn't required to
 * get a 64-bit off_t.  AC_SYS_LARGEFILE doesn't set any _FILE_OFFSET_BITS if
 * it's not required (such as on aarch64), so we must inject it here.
 */
# define __SET_FOB_FOR_FUSE
# define _FILE_OFFSET_BITS 64
#endif /* _FILE_OFFSET_BITS */
#include <fuse.h>
#ifdef __SET_FOB_FOR_FUSE
# undef _FILE_OFFSET_BITS
#endif /* __SET_FOB_FOR_FUSE */
#include <inttypes.h>
#include "ext2fs/ext2fs.h"
#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fsP.h"
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
# define FUSE_PLATFORM_OPTS	""
#else
# ifdef __linux__
#  define FUSE_PLATFORM_OPTS	",use_ino,big_writes"
# else
#  define FUSE_PLATFORM_OPTS	",use_ino"
# endif
#endif

#include "../version.h"
#include "uuid/uuid.h"
#include "e2p/e2p.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(a) (gettext(a))
#ifdef gettext_noop
#define N_(a) gettext_noop(a)
#else
#define N_(a) (a)
#endif
#define P_(singular, plural, n) (ngettext(singular, plural, n))
#ifndef NLS_CAT_NAME
#define NLS_CAT_NAME "e2fsprogs"
#endif
#ifndef LOCALEDIR
#define LOCALEDIR "/usr/share/locale"
#endif
#else
#define _(a) (a)
#define N_(a) a
#define P_(singular, plural, n) ((n) == 1 ? (singular) : (plural))
#endif

#ifndef XATTR_NAME_POSIX_ACL_DEFAULT
#define XATTR_NAME_POSIX_ACL_DEFAULT "posix_acl_default"
#endif
#ifndef XATTR_SECURITY_PREFIX
#define XATTR_SECURITY_PREFIX "security."
#define XATTR_SECURITY_PREFIX_LEN (sizeof (XATTR_SECURITY_PREFIX) - 1)
#endif

/*
 * Linux and MacOS implement the setxattr(2) interface, which defines
 * XATTR_CREATE and XATTR_REPLACE.  However, FreeBSD uses
 * extattr_set_file(2), which does not have a flags or options
 * parameter, and does not define XATTR_CREATE and XATTR_REPLACE.
 */
#ifndef XATTR_CREATE
#define XATTR_CREATE 0
#endif

#ifndef XATTR_REPLACE
#define XATTR_REPLACE 0
#endif

#if !defined(EUCLEAN)
#if !defined(EBADMSG)
#define EUCLEAN EBADMSG
#elif !defined(EPROTO)
#define EUCLEAN EPROTO
#else
#define EUCLEAN EIO
#endif
#endif /* !defined(EUCLEAN) */

#if !defined(ENODATA)
#ifdef ENOATTR
#define ENODATA ENOATTR
#else
#define ENODATA ENOENT
#endif
#endif /* !defined(ENODATA) */

static ext2_filsys global_fs; /* Try not to use this directly */

#define dbg_printf(fuse2fs, format, ...) \
	while ((fuse2fs)->debug) { \
		printf("FUSE2FS (%s): " format, (fuse2fs)->shortdev, ##__VA_ARGS__); \
		fflush(stdout); \
		break; \
	}

#define log_printf(fuse2fs, format, ...) \
	do { \
		printf("FUSE2FS (%s): " format, (fuse2fs)->shortdev, ##__VA_ARGS__); \
		fflush(stdout); \
	} while (0)

#define err_printf(fuse2fs, format, ...) \
	do { \
		fprintf(stderr, "FUSE2FS (%s): " format, (fuse2fs)->shortdev, ##__VA_ARGS__); \
		fflush(stderr); \
	} while (0)

#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 8)
# ifdef _IOR
#  ifdef _IOW
#   define SUPPORT_I_FLAGS
#  endif
# endif
#endif

#ifdef FALLOC_FL_KEEP_SIZE
# define FL_KEEP_SIZE_FLAG FALLOC_FL_KEEP_SIZE
# define SUPPORT_FALLOCATE
#else
# define FL_KEEP_SIZE_FLAG (0)
#endif

#ifdef FALLOC_FL_PUNCH_HOLE
# define FL_PUNCH_HOLE_FLAG FALLOC_FL_PUNCH_HOLE
#else
# define FL_PUNCH_HOLE_FLAG (0)
#endif

#ifdef FALLOC_FL_ZERO_RANGE
# define FL_ZERO_RANGE_FLAG FALLOC_FL_ZERO_RANGE
#else
# define FL_ZERO_RANGE_FLAG (0)
#endif

errcode_t ext2fs_run_ext3_journal(ext2_filsys *fs);

#ifdef CONFIG_JBD_DEBUG		/* Enabled by configure --enable-jbd-debug */
int journal_enable_debug = -1;
#endif

/*
 * ext2_file_t contains a struct inode, so we can't leave files open.
 * Use this as a proxy instead.
 */
#define FUSE2FS_FILE_MAGIC	(0xEF53DEAFUL)
struct fuse2fs_file_handle {
	unsigned long magic;
	ext2_ino_t ino;
	int open_flags;
};

/* Main program context */
#define FUSE2FS_MAGIC		(0xEF53DEADUL)
struct fuse2fs {
	unsigned long magic;
	ext2_filsys fs;
	pthread_mutex_t bfl;
	char *device;
	char *shortdev;
	uint8_t ro;
	uint8_t debug;
	uint8_t no_default_opts;
	uint8_t panic_on_error;
	uint8_t minixdf;
	uint8_t fakeroot;
	uint8_t alloc_all_blocks;
	uint8_t norecovery;
	uint8_t kernel;
	uint8_t directio;
	uint8_t acl;

	int blocklog;
	unsigned int blockmask;
	unsigned long offset;
	unsigned int next_generation;
	unsigned long long cache_size;
	char *lockfile;
};

#define FUSE2FS_CHECK_MAGIC(fs, ptr, num) do {if ((ptr)->magic != (num)) \
	return translate_error((fs), 0, EXT2_ET_FILESYSTEM_CORRUPTED); \
} while (0)

#define FUSE2FS_CHECK_CONTEXT(ptr) do {if ((ptr)->magic != FUSE2FS_MAGIC) \
	return translate_error(global_fs, 0, EXT2_ET_FILESYSTEM_CORRUPTED); \
} while (0)

static int __translate_error(ext2_filsys fs, ext2_ino_t ino, errcode_t err,
			     const char *file, int line);
#define translate_error(fs, ino, err) __translate_error((fs), (ino), (err), \
			__FILE__, __LINE__)

/* for macosx */
#ifndef W_OK
#  define W_OK 2
#endif

#ifndef R_OK
#  define R_OK 4
#endif

static inline int u_log2(unsigned int arg)
{
	int	l = 0;

	arg >>= 1;
	while (arg) {
		l++;
		arg >>= 1;
	}
	return l;
}

static inline blk64_t FUSE2FS_B_TO_FSBT(const struct fuse2fs *ff, off_t pos)
{
	return pos >> ff->blocklog;
}

static inline blk64_t FUSE2FS_B_TO_FSB(const struct fuse2fs *ff, off_t pos)
{
	return (pos + ff->blockmask) >> ff->blocklog;
}

static inline unsigned int FUSE2FS_OFF_IN_FSB(const struct fuse2fs *ff,
					      off_t pos)
{
	return pos & ff->blockmask;
}

static inline off_t FUSE2FS_FSB_TO_B(const struct fuse2fs *ff, blk64_t bno)
{
	return bno << ff->blocklog;
}

#define EXT4_EPOCH_BITS 2
#define EXT4_EPOCH_MASK ((1 << EXT4_EPOCH_BITS) - 1)
#define EXT4_NSEC_MASK  (~0UL << EXT4_EPOCH_BITS)

/*
 * Extended fields will fit into an inode if the filesystem was formatted
 * with large inodes (-I 256 or larger) and there are not currently any EAs
 * consuming all of the available space. For new inodes we always reserve
 * enough space for the kernel's known extended fields, but for inodes
 * created with an old kernel this might not have been the case. None of
 * the extended inode fields is critical for correct filesystem operation.
 * This macro checks if a certain field fits in the inode. Note that
 * inode-size = GOOD_OLD_INODE_SIZE + i_extra_isize
 */
#define EXT4_FITS_IN_INODE(ext4_inode, field)		\
	((offsetof(typeof(*ext4_inode), field) +	\
	  sizeof((ext4_inode)->field))			\
	 <= ((size_t) EXT2_GOOD_OLD_INODE_SIZE +		\
	    (ext4_inode)->i_extra_isize))		\

static inline __u32 ext4_encode_extra_time(const struct timespec *time)
{
	__u32 extra = sizeof(time->tv_sec) > 4 ?
			((time->tv_sec - (__s32)time->tv_sec) >> 32) &
			EXT4_EPOCH_MASK : 0;
	return extra | (time->tv_nsec << EXT4_EPOCH_BITS);
}

static inline void ext4_decode_extra_time(struct timespec *time, __u32 extra)
{
	if (sizeof(time->tv_sec) > 4 && (extra & EXT4_EPOCH_MASK)) {
		__u64 extra_bits = extra & EXT4_EPOCH_MASK;
		/*
		 * Prior to kernel 3.14?, we had a broken decode function,
		 * wherein we effectively did this:
		 * if (extra_bits == 3)
		 *     extra_bits = 0;
		 */
		time->tv_sec += extra_bits << 32;
	}
	time->tv_nsec = ((extra) & EXT4_NSEC_MASK) >> EXT4_EPOCH_BITS;
}

#define EXT4_CLAMP_TIMESTAMP(xtime, timespec, raw_inode)		       \
do {									       \
	if ((timespec)->tv_sec < EXT4_TIMESTAMP_MIN)			       \
		(timespec)->tv_sec = EXT4_TIMESTAMP_MIN;		       \
	if ((timespec)->tv_sec < EXT4_TIMESTAMP_MIN)			       \
		(timespec)->tv_sec = EXT4_TIMESTAMP_MIN;		       \
									       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime ## _extra)) {		       \
		if ((timespec)->tv_sec > EXT4_EXTRA_TIMESTAMP_MAX)	       \
			(timespec)->tv_sec = EXT4_EXTRA_TIMESTAMP_MAX;	       \
	} else {							       \
		if ((timespec)->tv_sec > EXT4_NON_EXTRA_TIMESTAMP_MAX)	       \
			(timespec)->tv_sec = EXT4_NON_EXTRA_TIMESTAMP_MAX;     \
	}								       \
} while (0)

#define EXT4_INODE_SET_XTIME(xtime, timespec, raw_inode)		       \
do {									       \
	typeof(*(timespec)) _ts = *(timespec);				       \
									       \
	EXT4_CLAMP_TIMESTAMP(xtime, &_ts, raw_inode);			       \
	(raw_inode)->xtime = _ts.tv_sec;				       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime ## _extra))		       \
		(raw_inode)->xtime ## _extra =				       \
				ext4_encode_extra_time(&_ts);		       \
} while (0)

#define EXT4_EINODE_SET_XTIME(xtime, timespec, raw_inode)		       \
do {									       \
	typeof(*(timespec)) _ts = *(timespec);				       \
									       \
	EXT4_CLAMP_TIMESTAMP(xtime, &_ts, raw_inode);			       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime))			       \
		(raw_inode)->xtime = _ts.tv_sec;			       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime ## _extra))		       \
		(raw_inode)->xtime ## _extra =				       \
				ext4_encode_extra_time(&_ts);		       \
} while (0)

#define EXT4_INODE_GET_XTIME(xtime, timespec, raw_inode)		       \
do {									       \
	(timespec)->tv_sec = (signed)((raw_inode)->xtime);		       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime ## _extra))		       \
		ext4_decode_extra_time((timespec),			       \
				       (raw_inode)->xtime ## _extra);	       \
	else								       \
		(timespec)->tv_nsec = 0;				       \
} while (0)

#define EXT4_EINODE_GET_XTIME(xtime, timespec, raw_inode)		       \
do {									       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime))			       \
		(timespec)->tv_sec =					       \
			(signed)((raw_inode)->xtime);			       \
	if (EXT4_FITS_IN_INODE(raw_inode, xtime ## _extra))		       \
		ext4_decode_extra_time((timespec),			       \
				       raw_inode->xtime ## _extra);	       \
	else								       \
		(timespec)->tv_nsec = 0;				       \
} while (0)

static inline errcode_t fuse2fs_read_inode(ext2_filsys fs, ext2_ino_t ino,
					   struct ext2_inode_large *inode)
{
	memset(inode, 0, sizeof(*inode));
	return ext2fs_read_inode_full(fs, ino, EXT2_INODE(inode),
				      sizeof(*inode));
}

static inline errcode_t fuse2fs_write_inode(ext2_filsys fs, ext2_ino_t ino,
					    struct ext2_inode_large *inode)
{
	return ext2fs_write_inode_full(fs, ino, EXT2_INODE(inode),
				       sizeof(*inode));
}

static void get_now(struct timespec *now)
{
#ifdef CLOCK_REALTIME
	if (!clock_gettime(CLOCK_REALTIME, now))
		return;
#endif

	now->tv_sec = time(NULL);
	now->tv_nsec = 0;
}

static void increment_version(struct ext2_inode_large *inode)
{
	__u64 ver;

	ver = inode->osd1.linux1.l_i_version;
	if (EXT4_FITS_IN_INODE(inode, i_version_hi))
		ver |= (__u64)inode->i_version_hi << 32;
	ver++;
	inode->osd1.linux1.l_i_version = ver;
	if (EXT4_FITS_IN_INODE(inode, i_version_hi))
		inode->i_version_hi = ver >> 32;
}

static void init_times(struct ext2_inode_large *inode)
{
	struct timespec now;

	get_now(&now);
	EXT4_INODE_SET_XTIME(i_atime, &now, inode);
	EXT4_INODE_SET_XTIME(i_ctime, &now, inode);
	EXT4_INODE_SET_XTIME(i_mtime, &now, inode);
	EXT4_EINODE_SET_XTIME(i_crtime, &now, inode);
	increment_version(inode);
}

static int update_ctime(ext2_filsys fs, ext2_ino_t ino,
			struct ext2_inode_large *pinode)
{
	errcode_t err;
	struct timespec now;
	struct ext2_inode_large inode;

	get_now(&now);

	/* If user already has a inode buffer, just update that */
	if (pinode) {
		increment_version(pinode);
		EXT4_INODE_SET_XTIME(i_ctime, &now, pinode);
		return 0;
	}

	/* Otherwise we have to read-modify-write the inode */
	err = fuse2fs_read_inode(fs, ino, &inode);
	if (err)
		return translate_error(fs, ino, err);

	increment_version(&inode);
	EXT4_INODE_SET_XTIME(i_ctime, &now, &inode);

	err = fuse2fs_write_inode(fs, ino, &inode);
	if (err)
		return translate_error(fs, ino, err);

	return 0;
}

static int update_atime(ext2_filsys fs, ext2_ino_t ino)
{
	errcode_t err;
	struct ext2_inode_large inode, *pinode;
	struct timespec atime, mtime, now;

	if (!(fs->flags & EXT2_FLAG_RW))
		return 0;
	err = fuse2fs_read_inode(fs, ino, &inode);
	if (err)
		return translate_error(fs, ino, err);

	pinode = &inode;
	EXT4_INODE_GET_XTIME(i_atime, &atime, pinode);
	EXT4_INODE_GET_XTIME(i_mtime, &mtime, pinode);
	get_now(&now);
	/*
	 * If atime is newer than mtime and atime hasn't been updated in thirty
	 * seconds, skip the atime update.  Same idea as Linux "relatime".
	 */
	if (atime.tv_sec >= mtime.tv_sec && atime.tv_sec >= now.tv_sec - 30)
		return 0;
	EXT4_INODE_SET_XTIME(i_atime, &now, &inode);

	err = fuse2fs_write_inode(fs, ino, &inode);
	if (err)
		return translate_error(fs, ino, err);

	return 0;
}

static int update_mtime(ext2_filsys fs, ext2_ino_t ino,
			struct ext2_inode_large *pinode)
{
	errcode_t err;
	struct ext2_inode_large inode;
	struct timespec now;

	if (pinode) {
		get_now(&now);
		EXT4_INODE_SET_XTIME(i_mtime, &now, pinode);
		EXT4_INODE_SET_XTIME(i_ctime, &now, pinode);
		increment_version(pinode);
		return 0;
	}

	err = fuse2fs_read_inode(fs, ino, &inode);
	if (err)
		return translate_error(fs, ino, err);

	get_now(&now);
	EXT4_INODE_SET_XTIME(i_mtime, &now, &inode);
	EXT4_INODE_SET_XTIME(i_ctime, &now, &inode);
	increment_version(&inode);

	err = fuse2fs_write_inode(fs, ino, &inode);
	if (err)
		return translate_error(fs, ino, err);

	return 0;
}

static int ext2_file_type(unsigned int mode)
{
	if (LINUX_S_ISREG(mode))
		return EXT2_FT_REG_FILE;

	if (LINUX_S_ISDIR(mode))
		return EXT2_FT_DIR;

	if (LINUX_S_ISCHR(mode))
		return EXT2_FT_CHRDEV;

	if (LINUX_S_ISBLK(mode))
		return EXT2_FT_BLKDEV;

	if (LINUX_S_ISLNK(mode))
		return EXT2_FT_SYMLINK;

	if (LINUX_S_ISFIFO(mode))
		return EXT2_FT_FIFO;

	if (LINUX_S_ISSOCK(mode))
		return EXT2_FT_SOCK;

	return 0;
}

static int fs_can_allocate(struct fuse2fs *ff, blk64_t num)
{
	ext2_filsys fs = ff->fs;
	blk64_t reserved;

	dbg_printf(ff, "%s: Asking for %llu; alloc_all=%d total=%llu free=%llu "
		   "rsvd=%llu\n", __func__, num, ff->alloc_all_blocks,
		   ext2fs_blocks_count(fs->super),
		   ext2fs_free_blocks_count(fs->super),
		   ext2fs_r_blocks_count(fs->super));
	if (num > ext2fs_blocks_count(fs->super))
		return 0;

	if (ff->alloc_all_blocks)
		return 1;

	/*
	 * Different meaning for r_blocks -- libext2fs has bugs where the FS
	 * can get corrupted if it totally runs out of blocks.  Avoid this
	 * by refusing to allocate any of the reserve blocks to anybody.
	 */
	reserved = ext2fs_r_blocks_count(fs->super);
	if (reserved == 0)
		reserved = ext2fs_blocks_count(fs->super) / 10;
	return ext2fs_free_blocks_count(fs->super) > reserved + num;
}

static int fs_writeable(ext2_filsys fs)
{
	return (fs->flags & EXT2_FLAG_RW) && (fs->super->s_error_count == 0);
}

static inline int is_superuser(struct fuse2fs *ff, struct fuse_context *ctxt)
{
	if (ff->fakeroot)
		return 1;
	return ctxt->uid == 0;
}

static inline int want_check_owner(struct fuse2fs *ff,
				   struct fuse_context *ctxt)
{
	/*
	 * The kernel is responsible for access control, so we allow anything
	 * that the superuser can do.
	 */
	if (ff->kernel)
		return 0;
	return !is_superuser(ff, ctxt);
}

/* Test for append permission */
#define A_OK	16

static int check_iflags_access(struct fuse2fs *ff, ext2_ino_t ino,
			       const struct ext2_inode *inode, int mask)
{
	ext2_filsys fs = ff->fs;

	EXT2FS_BUILD_BUG_ON((A_OK & (R_OK | W_OK | X_OK | F_OK)) != 0);

	/* no writing or metadata changes to read-only or broken fs */
	if ((mask & (W_OK | A_OK)) && !fs_writeable(fs))
		return -EROFS;

	dbg_printf(ff, "access ino=%d mask=e%s%s%s%s iflags=0x%x\n",
		   ino,
		   (mask & R_OK ? "r" : ""),
		   (mask & W_OK ? "w" : ""),
		   (mask & X_OK ? "x" : ""),
		   (mask & A_OK ? "a" : ""),
		   inode->i_flags);

	/* is immutable? */
	if ((mask & W_OK) &&
	    (inode->i_flags & EXT2_IMMUTABLE_FL))
		return -EPERM;

	/* is append-only? */
	if ((inode->i_flags & EXT2_APPEND_FL) && (mask & W_OK) && !(mask & A_OK))
		return -EPERM;

	return 0;
}

static int check_inum_access(struct fuse2fs *ff, ext2_ino_t ino, int mask)
{
	struct fuse_context *ctxt = fuse_get_context();
	ext2_filsys fs = ff->fs;
	struct ext2_inode inode;
	mode_t perms;
	errcode_t err;
	int ret;

	/* no writing to read-only or broken fs */
	if ((mask & (W_OK | A_OK)) && !fs_writeable(fs))
		return -EROFS;

	err = ext2fs_read_inode(fs, ino, &inode);
	if (err)
		return translate_error(fs, ino, err);
	perms = inode.i_mode & 0777;

	dbg_printf(ff, "access ino=%d mask=e%s%s%s%s perms=0%o iflags=0x%x "
		   "fuid=%d fgid=%d uid=%d gid=%d\n", ino,
		   (mask & R_OK ? "r" : ""),
		   (mask & W_OK ? "w" : ""),
		   (mask & X_OK ? "x" : ""),
		   (mask & A_OK ? "a" : ""),
		   perms, inode.i_flags,
		   inode_uid(inode), inode_gid(inode),
		   ctxt->uid, ctxt->gid);

	/* existence check */
	if (mask == 0)
		return 0;

	ret = check_iflags_access(ff, ino, &inode, mask);
	if (ret)
		return ret;

	/* If kernel is responsible for mode and acl checks, we're done. */
	if (ff->kernel)
		return 0;

	/* Figure out what root's allowed to do */
	if (is_superuser(ff, ctxt)) {
		/* Non-file access always ok */
		if (!LINUX_S_ISREG(inode.i_mode))
			return 0;

		/* R/W access to a file always ok */
		if (!(mask & X_OK))
			return 0;

		/* X access to a file ok if a user/group/other can X */
		if (perms & 0111)
			return 0;

		/* Trying to execute a file that's not executable. BZZT! */
		return -EACCES;
	}

	/* Remove the O_APPEND flag before testing permissions */
	mask &= ~A_OK;

	/* allow owner, if perms match */
	if (inode_uid(inode) == ctxt->uid) {
		if ((mask & (perms >> 6)) == mask)
			return 0;
		return -EACCES;
	}

	/* allow group, if perms match */
	if (inode_gid(inode) == ctxt->gid) {
		if ((mask & (perms >> 3)) == mask)
			return 0;
		return -EACCES;
	}

	/* otherwise check other */
	if ((mask & perms) == mask)
		return 0;
	return -EACCES;
}

static void op_destroy(void *p EXT2FS_ATTR((unused)))
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;

	if (ff->magic != FUSE2FS_MAGIC) {
		translate_error(global_fs, 0, EXT2_ET_BAD_MAGIC);
		return;
	}
	fs = ff->fs;
	dbg_printf(ff, "%s: dev=%s\n", __func__, fs->device_name);
	if (fs->flags & EXT2_FLAG_RW) {
		fs->super->s_state |= EXT2_VALID_FS;
		if (fs->super->s_error_count)
			fs->super->s_state |= EXT2_ERROR_FS;
		ext2fs_mark_super_dirty(fs);
		err = ext2fs_set_gdt_csum(fs);
		if (err)
			translate_error(fs, 0, err);

		err = ext2fs_flush2(fs, 0);
		if (err)
			translate_error(fs, 0, err);
	}

	if (ff->debug && fs->io->manager->get_stats) {
		io_stats stats = NULL;

		fs->io->manager->get_stats(fs->io, &stats);
		dbg_printf(ff, "read: %lluk\n",  stats->bytes_read >> 10);
		dbg_printf(ff, "write: %lluk\n", stats->bytes_written >> 10);
		dbg_printf(ff, "hits: %llu\n",   stats->cache_hits);
		dbg_printf(ff, "misses: %llu\n", stats->cache_misses);
		dbg_printf(ff, "hit_ratio: %.1f%%\n",
				(100.0 * stats->cache_hits) /
				(stats->cache_hits + stats->cache_misses));
	}

	if (ff->kernel) {
		char uuid[UUID_STR_SIZE];

		uuid_unparse(fs->super->s_uuid, uuid);
		log_printf(ff, "%s %s.\n", _("unmounting filesystem"), uuid);
	}
}

static void *op_init(struct fuse_conn_info *conn
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
			, struct fuse_config *cfg EXT2FS_ATTR((unused))
#endif
			)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;

	if (ff->magic != FUSE2FS_MAGIC) {
		translate_error(global_fs, 0, EXT2_ET_BAD_MAGIC);
		return NULL;
	}
	fs = ff->fs;
	dbg_printf(ff, "%s: dev=%s\n", __func__, fs->device_name);
#ifdef FUSE_CAP_IOCTL_DIR
	conn->want |= FUSE_CAP_IOCTL_DIR;
#endif
#ifdef FUSE_CAP_POSIX_ACL
	if (ff->acl)
		conn->want |= FUSE_CAP_POSIX_ACL;
#endif
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
	conn->time_gran = 1;
	cfg->use_ino = 1;
	if (ff->debug)
		cfg->debug = 1;
#endif
	if (fs->flags & EXT2_FLAG_RW) {
		fs->super->s_mnt_count++;
		ext2fs_set_tstamp(fs->super, s_mtime, time(NULL));
		fs->super->s_state &= ~EXT2_VALID_FS;
		ext2fs_mark_super_dirty(fs);
		err = ext2fs_flush2(fs, 0);
		if (err)
			translate_error(fs, 0, err);
	}

	if (ff->kernel) {
		char uuid[UUID_STR_SIZE];

		uuid_unparse(fs->super->s_uuid, uuid);
		log_printf(ff, "%s %s.\n", _("mounted filesystem"), uuid);
	}
	return ff;
}

static int stat_inode(ext2_filsys fs, ext2_ino_t ino, struct stat *statbuf)
{
	struct ext2_inode_large inode;
	dev_t fakedev = 0;
	errcode_t err;
	int ret = 0;
	struct timespec tv;

	err = fuse2fs_read_inode(fs, ino, &inode);
	if (err)
		return translate_error(fs, ino, err);

	memcpy(&fakedev, fs->super->s_uuid, sizeof(fakedev));
	statbuf->st_dev = fakedev;
	statbuf->st_ino = ino;
	statbuf->st_mode = inode.i_mode;
	statbuf->st_nlink = inode.i_links_count;
	statbuf->st_uid = inode_uid(inode);
	statbuf->st_gid = inode_gid(inode);
	statbuf->st_size = EXT2_I_SIZE(&inode);
	statbuf->st_blksize = fs->blocksize;
	statbuf->st_blocks = ext2fs_get_stat_i_blocks(fs,
						EXT2_INODE(&inode));
	EXT4_INODE_GET_XTIME(i_atime, &tv, &inode);
#if HAVE_STRUCT_STAT_ST_ATIM
	statbuf->st_atim = tv;
#else
	statbuf->st_atime = tv.tv_sec;
#endif
	EXT4_INODE_GET_XTIME(i_mtime, &tv, &inode);
#if HAVE_STRUCT_STAT_ST_ATIM
	statbuf->st_mtim = tv;
#else
	statbuf->st_mtime = tv.tv_sec;
#endif
	EXT4_INODE_GET_XTIME(i_ctime, &tv, &inode);
#if HAVE_STRUCT_STAT_ST_ATIM
	statbuf->st_ctim = tv;
#else
	statbuf->st_ctime = tv.tv_sec;
#endif
	if (LINUX_S_ISCHR(inode.i_mode) ||
	    LINUX_S_ISBLK(inode.i_mode)) {
		if (inode.i_block[0])
			statbuf->st_rdev = inode.i_block[0];
		else
			statbuf->st_rdev = inode.i_block[1];
	}

	return ret;
}

static int op_getattr(const char *path, struct stat *statbuf
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
			, struct fuse_file_info *fi EXT2FS_ATTR((unused))
#endif
			)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf(ff, "%s: path=%s\n", __func__, path);
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	ret = stat_inode(fs, ino, statbuf);
out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_readlink(const char *path, char *buf, size_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	struct ext2_inode inode;
	unsigned int got;
	ext2_file_t file;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf(ff, "%s: path=%s\n", __func__, path);
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}

	err = ext2fs_read_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	if (!LINUX_S_ISLNK(inode.i_mode)) {
		ret = -EINVAL;
		goto out;
	}

	len--;
	if (inode.i_size < len)
		len = inode.i_size;
	if (ext2fs_is_fast_symlink(&inode))
		memcpy(buf, (char *)inode.i_block, len);
	else {
		/* big/inline symlink */

		err = ext2fs_file_open(fs, ino, 0, &file);
		if (err) {
			ret = translate_error(fs, ino, err);
			goto out;
		}

		err = ext2fs_file_read(file, buf, len, &got);
		if (err || got != len) {
			ext2fs_file_close(file);
			ret = translate_error(fs, ino, err);
			goto out2;
		}

out2:
		err = ext2fs_file_close(file);
		if (ret)
			goto out;
		if (err) {
			ret = translate_error(fs, ino, err);
			goto out;
		}
	}
	buf[len] = 0;

	if (fs_writeable(fs)) {
		ret = update_atime(fs, ino);
		if (ret)
			goto out;
	}

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int __getxattr(struct fuse2fs *ff, ext2_ino_t ino, const char *name,
		      void **value, size_t *value_len)
{
	ext2_filsys fs = ff->fs;
	struct ext2_xattr_handle *h;
	errcode_t err;
	int ret = 0;

	err = ext2fs_xattrs_open(fs, ino, &h);
	if (err)
		return translate_error(fs, ino, err);

	err = ext2fs_xattrs_read(h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out_close;
	}

	err = ext2fs_xattr_get(h, name, value, value_len);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out_close;
	}

out_close:
	err = ext2fs_xattrs_close(&h);
	if (err && !ret)
		ret = translate_error(fs, ino, err);
	return ret;
}

static int __setxattr(struct fuse2fs *ff, ext2_ino_t ino, const char *name,
		      void *value, size_t valuelen)
{
	ext2_filsys fs = ff->fs;
	struct ext2_xattr_handle *h;
	errcode_t err;
	int ret = 0;

	err = ext2fs_xattrs_open(fs, ino, &h);
	if (err)
		return translate_error(fs, ino, err);

	err = ext2fs_xattrs_read(h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out_close;
	}

	err = ext2fs_xattr_set(h, name, value, valuelen);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out_close;
	}

out_close:
	err = ext2fs_xattrs_close(&h);
	if (err && !ret)
		ret = translate_error(fs, ino, err);
	return ret;
}

static int propagate_default_acls(struct fuse2fs *ff, ext2_ino_t parent,
				  ext2_ino_t child)
{
	void *def;
	size_t deflen;
	int ret;

	if (!ff->acl)
		return 0;

	ret = __getxattr(ff, parent, XATTR_NAME_POSIX_ACL_DEFAULT, &def,
			 &deflen);
	switch (ret) {
	case -ENODATA:
	case -ENOENT:
		/* no default acl */
		return 0;
	case 0:
		break;
	default:
		return ret;
	}

	ret = __setxattr(ff, child, XATTR_NAME_POSIX_ACL_DEFAULT, def, deflen);
	ext2fs_free_mem(&def);
	return ret;
}

static int op_mknod(const char *path, mode_t mode, dev_t dev)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t parent, child;
	char *temp_path;
	errcode_t err;
	char *node_name, a;
	int filetype;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf(ff, "%s: path=%s mode=0%o dev=0x%x\n", __func__, path, mode,
		   (unsigned int)dev);
	temp_path = strdup(path);
	if (!temp_path) {
		ret = -ENOMEM;
		goto out;
	}
	node_name = strrchr(temp_path, '/');
	if (!node_name) {
		ret = -ENOMEM;
		goto out;
	}
	node_name++;
	a = *node_name;
	*node_name = 0;

	pthread_mutex_lock(&ff->bfl);
	if (!fs_can_allocate(ff, 2)) {
		ret = -ENOSPC;
		goto out2;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &parent);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	ret = check_inum_access(ff, parent, A_OK | W_OK);
	if (ret)
		goto out2;

	*node_name = a;

	if (LINUX_S_ISCHR(mode))
		filetype = EXT2_FT_CHRDEV;
	else if (LINUX_S_ISBLK(mode))
		filetype = EXT2_FT_BLKDEV;
	else if (LINUX_S_ISFIFO(mode))
		filetype = EXT2_FT_FIFO;
	else if (LINUX_S_ISSOCK(mode))
		filetype = EXT2_FT_SOCK;
	else {
		ret = -EINVAL;
		goto out2;
	}

	err = ext2fs_new_inode(fs, parent, mode, 0, &child);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	dbg_printf(ff, "%s: create ino=%d/name=%s in dir=%d\n", __func__, child,
		   node_name, parent);
	err = ext2fs_link(fs, parent, node_name, child,
			  filetype | EXT2FS_LINK_EXPAND);
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	ret = update_mtime(fs, parent, NULL);
	if (ret)
		goto out2;

	memset(&inode, 0, sizeof(inode));
	inode.i_mode = mode;

	if (dev & ~0xFFFF)
		inode.i_block[1] = dev;
	else
		inode.i_block[0] = dev;
	inode.i_links_count = 1;
	inode.i_extra_isize = sizeof(struct ext2_inode_large) -
		EXT2_GOOD_OLD_INODE_SIZE;
	inode.i_uid = ctxt->uid;
	ext2fs_set_i_uid_high(inode, ctxt->uid >> 16);
	inode.i_gid = ctxt->gid;
	ext2fs_set_i_gid_high(inode, ctxt->gid >> 16);

	err = ext2fs_write_new_inode(fs, child, EXT2_INODE(&inode));
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	inode.i_generation = ff->next_generation++;
	init_times(&inode);
	err = fuse2fs_write_inode(fs, child, &inode);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	ext2fs_inode_alloc_stats2(fs, child, 1, 0);

	ret = propagate_default_acls(ff, parent, child);
	if (ret)
		goto out2;
out2:
	pthread_mutex_unlock(&ff->bfl);
out:
	free(temp_path);
	return ret;
}

static int op_mkdir(const char *path, mode_t mode)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t parent, child;
	char *temp_path;
	errcode_t err;
	char *node_name, a;
	struct ext2_inode_large inode;
	char *block;
	blk64_t blk;
	int ret = 0;
	mode_t parent_sgid;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf(ff, "%s: path=%s mode=0%o\n", __func__, path, mode);
	temp_path = strdup(path);
	if (!temp_path) {
		ret = -ENOMEM;
		goto out;
	}
	node_name = strrchr(temp_path, '/');
	if (!node_name) {
		ret = -ENOMEM;
		goto out;
	}
	node_name++;
	a = *node_name;
	*node_name = 0;

	pthread_mutex_lock(&ff->bfl);
	if (!fs_can_allocate(ff, 1)) {
		ret = -ENOSPC;
		goto out2;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &parent);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	ret = check_inum_access(ff, parent, A_OK | W_OK);
	if (ret)
		goto out2;

	/* Is the parent dir sgid? */
	err = fuse2fs_read_inode(fs, parent, &inode);
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}
	parent_sgid = inode.i_mode & S_ISGID;

	*node_name = a;

	err = ext2fs_mkdir2(fs, parent, 0, 0, EXT2FS_LINK_EXPAND,
			    node_name, NULL);
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	ret = update_mtime(fs, parent, NULL);
	if (ret)
		goto out2;

	/* Still have to update the uid/gid of the dir */
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &child);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}
	dbg_printf(ff, "%s: created ino=%d/path=%s in dir=%d\n", __func__, child,
		   node_name, parent);

	err = fuse2fs_read_inode(fs, child, &inode);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	inode.i_uid = ctxt->uid;
	ext2fs_set_i_uid_high(inode, ctxt->uid >> 16);
	inode.i_gid = ctxt->gid;
	ext2fs_set_i_gid_high(inode, ctxt->gid >> 16);
	inode.i_mode = LINUX_S_IFDIR | (mode & ~S_ISUID) |
		       parent_sgid;
	inode.i_generation = ff->next_generation++;
	init_times(&inode);

	err = fuse2fs_write_inode(fs, child, &inode);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	/* Rewrite the directory block checksum, having set i_generation */
	if ((inode.i_flags & EXT4_INLINE_DATA_FL) ||
	    !ext2fs_has_feature_metadata_csum(fs->super))
		goto out2;
	err = ext2fs_new_dir_block(fs, child, parent, &block);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}
	err = ext2fs_bmap2(fs, child, EXT2_INODE(&inode), NULL, 0, 0,
			   NULL, &blk);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out3;
	}
	err = ext2fs_write_dir_block4(fs, blk, block, 0, child);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out3;
	}

	ret = propagate_default_acls(ff, parent, child);
	if (ret)
		goto out3;

out3:
	ext2fs_free_mem(&block);
out2:
	pthread_mutex_unlock(&ff->bfl);
out:
	free(temp_path);
	return ret;
}

static int unlink_file_by_name(struct fuse2fs *ff, const char *path)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	ext2_ino_t dir;
	char *filename = strdup(path);
	char *base_name;
	int ret;

	base_name = strrchr(filename, '/');
	if (base_name) {
		*base_name++ = '\0';
		err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, filename,
				   &dir);
		if (err) {
			free(filename);
			return translate_error(fs, 0, err);
		}
	} else {
		dir = EXT2_ROOT_INO;
		base_name = filename;
	}

	ret = check_inum_access(ff, dir, W_OK);
	if (ret) {
		free(filename);
		return ret;
	}

	dbg_printf(ff, "%s: unlinking name=%s from dir=%d\n", __func__,
		   base_name, dir);
	err = ext2fs_unlink(fs, dir, base_name, 0, 0);
	free(filename);
	if (err)
		return translate_error(fs, dir, err);

	return update_mtime(fs, dir, NULL);
}

static errcode_t remove_ea_inodes(struct fuse2fs *ff, ext2_ino_t ino,
				  struct ext2_inode_large *inode)
{
	ext2_filsys fs = ff->fs;
	struct ext2_xattr_handle *h;
	errcode_t err;

	/*
	 * The xattr handle maintains its own private copy of the inode, so
	 * write ours to disk so that we can read it.
	 */
	err = fuse2fs_write_inode(fs, ino, inode);
	if (err)
		return err;

	err = ext2fs_xattrs_open(fs, ino, &h);
	if (err)
		return err;

	err = ext2fs_xattrs_read(h);
	if (err)
		goto out_close;

	err = ext2fs_xattr_remove_all(h);
	if (err)
		goto out_close;

out_close:
	ext2fs_xattrs_close(&h);

	/* Now read the inode back in. */
	return fuse2fs_read_inode(fs, ino, inode);
}

static int remove_inode(struct fuse2fs *ff, ext2_ino_t ino)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	struct ext2_inode_large inode;
	int ret = 0;

	err = fuse2fs_read_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}
	dbg_printf(ff, "%s: put ino=%d links=%d\n", __func__, ino,
		   inode.i_links_count);

	switch (inode.i_links_count) {
	case 0:
		return 0; /* XXX: already done? */
	case 1:
		inode.i_links_count--;
		ext2fs_set_dtime(fs, EXT2_INODE(&inode));
		break;
	default:
		inode.i_links_count--;
	}

	ret = update_ctime(fs, ino, &inode);
	if (ret)
		goto out;

	if (inode.i_links_count)
		goto write_out;

	if (ext2fs_has_feature_ea_inode(fs->super)) {
		err = remove_ea_inodes(ff, ino, &inode);
		if (err)
			goto write_out;
	}

	/* Nobody holds this file; free its blocks! */
	err = ext2fs_free_ext_attr(fs, ino, &inode);
	if (err)
		goto write_out;

	if (ext2fs_inode_has_valid_blocks2(fs, EXT2_INODE(&inode))) {
		err = ext2fs_punch(fs, ino, EXT2_INODE(&inode), NULL,
				   0, ~0ULL);
		if (err) {
			ret = translate_error(fs, ino, err);
			goto write_out;
		}
	}

	ext2fs_inode_alloc_stats2(fs, ino, -1,
				  LINUX_S_ISDIR(inode.i_mode));

write_out:
	err = fuse2fs_write_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}
out:
	return ret;
}

static int __op_unlink(struct fuse2fs *ff, const char *path)
{
	ext2_filsys fs = ff->fs;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}

	ret = check_inum_access(ff, ino, W_OK);
	if (ret)
		goto out;

	ret = unlink_file_by_name(ff, path);
	if (ret)
		goto out;

	ret = remove_inode(ff, ino);
	if (ret)
		goto out;
out:
	return ret;
}

static int op_unlink(const char *path)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	int ret;

	FUSE2FS_CHECK_CONTEXT(ff);
	pthread_mutex_lock(&ff->bfl);
	ret = __op_unlink(ff, path);
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

struct rd_struct {
	ext2_ino_t	parent;
	int		empty;
};

static int rmdir_proc(ext2_ino_t dir EXT2FS_ATTR((unused)),
		      int	entry EXT2FS_ATTR((unused)),
		      struct ext2_dir_entry *dirent,
		      int	offset EXT2FS_ATTR((unused)),
		      int	blocksize EXT2FS_ATTR((unused)),
		      char	*buf EXT2FS_ATTR((unused)),
		      void	*private)
{
	struct rd_struct *rds = (struct rd_struct *) private;

	if (dirent->inode == 0)
		return 0;
	if (((dirent->name_len & 0xFF) == 1) && (dirent->name[0] == '.'))
		return 0;
	if (((dirent->name_len & 0xFF) == 2) && (dirent->name[0] == '.') &&
	    (dirent->name[1] == '.')) {
		rds->parent = dirent->inode;
		return 0;
	}
	rds->empty = 0;
	return 0;
}

static int __op_rmdir(struct fuse2fs *ff, const char *path)
{
	ext2_filsys fs = ff->fs;
	ext2_ino_t child;
	errcode_t err;
	struct ext2_inode_large inode;
	struct rd_struct rds;
	int ret = 0;

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &child);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf(ff, "%s: rmdir path=%s ino=%d\n", __func__, path, child);

	ret = check_inum_access(ff, child, W_OK);
	if (ret)
		goto out;

	rds.parent = 0;
	rds.empty = 1;

	err = ext2fs_dir_iterate2(fs, child, 0, 0, rmdir_proc, &rds);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out;
	}

	/* the kernel checks parent permissions before emptiness */
	if (rds.parent == 0) {
		ret = translate_error(fs, child, EXT2_ET_FILESYSTEM_CORRUPTED);
		goto out;
	}

	ret = check_inum_access(ff, rds.parent, W_OK);
	if (ret)
		goto out;

	if (rds.empty == 0) {
		ret = -ENOTEMPTY;
		goto out;
	}

	ret = unlink_file_by_name(ff, path);
	if (ret)
		goto out;
	/* Directories have to be "removed" twice. */
	ret = remove_inode(ff, child);
	if (ret)
		goto out;
	ret = remove_inode(ff, child);
	if (ret)
		goto out;

	if (rds.parent) {
		dbg_printf(ff, "%s: decr dir=%d link count\n", __func__,
			   rds.parent);
		err = fuse2fs_read_inode(fs, rds.parent, &inode);
		if (err) {
			ret = translate_error(fs, rds.parent, err);
			goto out;
		}
		if (inode.i_links_count > 1)
			inode.i_links_count--;
		ret = update_mtime(fs, rds.parent, &inode);
		if (ret)
			goto out;
		err = fuse2fs_write_inode(fs, rds.parent, &inode);
		if (err) {
			ret = translate_error(fs, rds.parent, err);
			goto out;
		}
	}

out:
	return ret;
}

static int op_rmdir(const char *path)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	int ret;

	FUSE2FS_CHECK_CONTEXT(ff);
	pthread_mutex_lock(&ff->bfl);
	ret = __op_rmdir(ff, path);
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_symlink(const char *src, const char *dest)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t parent, child;
	char *temp_path;
	errcode_t err;
	char *node_name, a;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf(ff, "%s: symlink %s to %s\n", __func__, src, dest);
	temp_path = strdup(dest);
	if (!temp_path) {
		ret = -ENOMEM;
		goto out;
	}
	node_name = strrchr(temp_path, '/');
	if (!node_name) {
		ret = -ENOMEM;
		goto out;
	}
	node_name++;
	a = *node_name;
	*node_name = 0;

	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &parent);
	*node_name = a;
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	ret = check_inum_access(ff, parent, A_OK | W_OK);
	if (ret)
		goto out2;


	/* Create symlink */
	err = ext2fs_symlink(fs, parent, 0, node_name, src);
	if (err == EXT2_ET_DIR_NO_SPACE) {
		err = ext2fs_expand_dir(fs, parent);
		if (err) {
			ret = translate_error(fs, parent, err);
			goto out2;
		}

		err = ext2fs_symlink(fs, parent, 0, node_name, src);
	}
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	/* Update parent dir's mtime */
	ret = update_mtime(fs, parent, NULL);
	if (ret)
		goto out2;

	/* Still have to update the uid/gid of the symlink */
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &child);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}
	dbg_printf(ff, "%s: symlinking ino=%d/name=%s to dir=%d\n", __func__,
		   child, node_name, parent);

	err = fuse2fs_read_inode(fs, child, &inode);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	inode.i_uid = ctxt->uid;
	ext2fs_set_i_uid_high(inode, ctxt->uid >> 16);
	inode.i_gid = ctxt->gid;
	ext2fs_set_i_gid_high(inode, ctxt->gid >> 16);
	inode.i_generation = ff->next_generation++;
	init_times(&inode);

	err = fuse2fs_write_inode(fs, child, &inode);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}
out2:
	pthread_mutex_unlock(&ff->bfl);
out:
	free(temp_path);
	return ret;
}

struct update_dotdot {
	ext2_ino_t new_dotdot;
};

static int update_dotdot_helper(ext2_ino_t dir EXT2FS_ATTR((unused)),
				int entry EXT2FS_ATTR((unused)),
				struct ext2_dir_entry *dirent,
				int offset EXT2FS_ATTR((unused)),
				int blocksize EXT2FS_ATTR((unused)),
				char *buf EXT2FS_ATTR((unused)),
				void *priv_data)
{
	struct update_dotdot *ud = priv_data;

	if (ext2fs_dirent_name_len(dirent) == 2 &&
	    dirent->name[0] == '.' && dirent->name[1] == '.') {
		dirent->inode = ud->new_dotdot;
		return DIRENT_CHANGED | DIRENT_ABORT;
	}

	return 0;
}

static int op_rename(const char *from, const char *to
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
			, unsigned int flags EXT2FS_ATTR((unused))
#endif
			)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t from_ino, to_ino, to_dir_ino, from_dir_ino;
	char *temp_to = NULL, *temp_from = NULL;
	char *cp, a;
	struct ext2_inode inode;
	struct update_dotdot ud;
	int ret = 0;

#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
	/* renameat2 is not supported */
	if (flags)
		return -ENOSYS;
#endif

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf(ff, "%s: renaming %s to %s\n", __func__, from, to);
	pthread_mutex_lock(&ff->bfl);
	if (!fs_can_allocate(ff, 5)) {
		ret = -ENOSPC;
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, from, &from_ino);
	if (err || from_ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, to, &to_ino);
	if (err && err != EXT2_ET_FILE_NOT_FOUND) {
		ret = translate_error(fs, 0, err);
		goto out;
	}

	if (err == EXT2_ET_FILE_NOT_FOUND)
		to_ino = 0;

	/* Already the same file? */
	if (to_ino != 0 && to_ino == from_ino) {
		ret = 0;
		goto out;
	}

	ret = check_inum_access(ff, from_ino, W_OK);
	if (ret)
		goto out;

	if (to_ino) {
		ret = check_inum_access(ff, to_ino, W_OK);
		if (ret)
			goto out;
	}

	temp_to = strdup(to);
	if (!temp_to) {
		ret = -ENOMEM;
		goto out;
	}

	temp_from = strdup(from);
	if (!temp_from) {
		ret = -ENOMEM;
		goto out2;
	}

	/* Find parent dir of the source and check write access */
	cp = strrchr(temp_from, '/');
	if (!cp) {
		ret = -EINVAL;
		goto out2;
	}

	a = *(cp + 1);
	*(cp + 1) = 0;
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_from,
			   &from_dir_ino);
	*(cp + 1) = a;
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}
	if (from_dir_ino == 0) {
		ret = -ENOENT;
		goto out2;
	}

	ret = check_inum_access(ff, from_dir_ino, W_OK);
	if (ret)
		goto out2;

	/* Find parent dir of the destination and check write access */
	cp = strrchr(temp_to, '/');
	if (!cp) {
		ret = -EINVAL;
		goto out2;
	}

	a = *(cp + 1);
	*(cp + 1) = 0;
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_to,
			   &to_dir_ino);
	*(cp + 1) = a;
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}
	if (to_dir_ino == 0) {
		ret = -ENOENT;
		goto out2;
	}

	ret = check_inum_access(ff, to_dir_ino, W_OK);
	if (ret)
		goto out2;

	/* If the target exists, unlink it first */
	if (to_ino != 0) {
		err = ext2fs_read_inode(fs, to_ino, &inode);
		if (err) {
			ret = translate_error(fs, to_ino, err);
			goto out2;
		}

		dbg_printf(ff, "%s: unlinking %s ino=%d\n", __func__,
			   LINUX_S_ISDIR(inode.i_mode) ? "dir" : "file",
			   to_ino);
		if (LINUX_S_ISDIR(inode.i_mode))
			ret = __op_rmdir(ff, to);
		else
			ret = __op_unlink(ff, to);
		if (ret)
			goto out2;
	}

	/* Get ready to do the move */
	err = ext2fs_read_inode(fs, from_ino, &inode);
	if (err) {
		ret = translate_error(fs, from_ino, err);
		goto out2;
	}

	/* Link in the new file */
	dbg_printf(ff, "%s: linking ino=%d/path=%s to dir=%d\n", __func__,
		   from_ino, cp + 1, to_dir_ino);
	err = ext2fs_link(fs, to_dir_ino, cp + 1, from_ino,
			  ext2_file_type(inode.i_mode) | EXT2FS_LINK_EXPAND);
	if (err) {
		ret = translate_error(fs, to_dir_ino, err);
		goto out2;
	}

	/* Update '..' pointer if dir */
	err = ext2fs_read_inode(fs, from_ino, &inode);
	if (err) {
		ret = translate_error(fs, from_ino, err);
		goto out2;
	}

	if (LINUX_S_ISDIR(inode.i_mode)) {
		ud.new_dotdot = to_dir_ino;
		dbg_printf(ff, "%s: updating .. entry for dir=%d\n", __func__,
			   to_dir_ino);
		err = ext2fs_dir_iterate2(fs, from_ino, 0, NULL,
					  update_dotdot_helper, &ud);
		if (err) {
			ret = translate_error(fs, from_ino, err);
			goto out2;
		}

		/* Decrease from_dir_ino's links_count */
		dbg_printf(ff, "%s: moving linkcount from dir=%d to dir=%d\n",
			   __func__, from_dir_ino, to_dir_ino);
		err = ext2fs_read_inode(fs, from_dir_ino, &inode);
		if (err) {
			ret = translate_error(fs, from_dir_ino, err);
			goto out2;
		}
		inode.i_links_count--;
		err = ext2fs_write_inode(fs, from_dir_ino, &inode);
		if (err) {
			ret = translate_error(fs, from_dir_ino, err);
			goto out2;
		}

		/* Increase to_dir_ino's links_count */
		err = ext2fs_read_inode(fs, to_dir_ino, &inode);
		if (err) {
			ret = translate_error(fs, to_dir_ino, err);
			goto out2;
		}
		inode.i_links_count++;
		err = ext2fs_write_inode(fs, to_dir_ino, &inode);
		if (err) {
			ret = translate_error(fs, to_dir_ino, err);
			goto out2;
		}
	}

	/* Update timestamps */
	ret = update_ctime(fs, from_ino, NULL);
	if (ret)
		goto out2;

	ret = update_mtime(fs, to_dir_ino, NULL);
	if (ret)
		goto out2;

	/* Remove the old file */
	ret = unlink_file_by_name(ff, from);
	if (ret)
		goto out2;

	/* Flush the whole mess out */
	err = ext2fs_flush2(fs, 0);
	if (err)
		ret = translate_error(fs, 0, err);

out2:
	free(temp_from);
	free(temp_to);
out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_link(const char *src, const char *dest)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	char *temp_path;
	errcode_t err;
	char *node_name, a;
	ext2_ino_t parent, ino;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf(ff, "%s: src=%s dest=%s\n", __func__, src, dest);
	temp_path = strdup(dest);
	if (!temp_path) {
		ret = -ENOMEM;
		goto out;
	}
	node_name = strrchr(temp_path, '/');
	if (!node_name) {
		ret = -ENOMEM;
		goto out;
	}
	node_name++;
	a = *node_name;
	*node_name = 0;

	pthread_mutex_lock(&ff->bfl);
	if (!fs_can_allocate(ff, 2)) {
		ret = -ENOSPC;
		goto out2;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &parent);
	*node_name = a;
	if (err) {
		err = -ENOENT;
		goto out2;
	}

	ret = check_inum_access(ff, parent, A_OK | W_OK);
	if (ret)
		goto out2;

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, src, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	err = fuse2fs_read_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	ret = check_iflags_access(ff, ino, EXT2_INODE(&inode), W_OK);
	if (ret)
		goto out2;

	inode.i_links_count++;
	ret = update_ctime(fs, ino, &inode);
	if (ret)
		goto out2;

	err = fuse2fs_write_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	dbg_printf(ff, "%s: linking ino=%d/name=%s to dir=%d\n", __func__, ino,
		   node_name, parent);
	err = ext2fs_link(fs, parent, node_name, ino,
			  ext2_file_type(inode.i_mode) | EXT2FS_LINK_EXPAND);
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	ret = update_mtime(fs, parent, NULL);
	if (ret)
		goto out2;

out2:
	pthread_mutex_unlock(&ff->bfl);
out:
	free(temp_path);
	return ret;
}

#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
/* Obtain group ids of the process that sent us a command(?) */
static int get_req_groups(struct fuse2fs *ff, gid_t **gids, size_t *nr_gids)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	gid_t *array;
	int nr = 32;	/* nobody has more than 32 groups right? */
	int ret;

	do {
		err = ext2fs_get_array(nr, sizeof(gid_t), &array);
		if (err)
			return translate_error(fs, 0, err);

		ret = fuse_getgroups(nr, array);
		if (ret < 0)
			return ret;

		if (ret <= nr) {
			*gids = array;
			*nr_gids = ret;
			return 0;
		}

		ext2fs_free_mem(&array);
		nr = ret;
	} while (0);

	/* shut up gcc */
	return -ENOMEM;
}

/*
 * Is this file's group id in the set of groups associated with the process
 * that initiated the fuse request?  Returns 1 for yes, 0 for no, or a negative
 * errno.
 */
static int in_file_group(struct fuse_context *ctxt,
			 const struct ext2_inode_large *inode)
{
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	gid_t *gids = NULL;
	size_t i, nr_gids = 0;
	gid_t gid = inode_gid(*inode);
	int ret;

	ret = get_req_groups(ff, &gids, &nr_gids);
	if (ret < 0)
		return ret;

	for (i = 0; i < nr_gids; i++)
		if (gids[i] == gid)
			return 1;
	return 0;
}
#else
static int in_file_group(struct fuse_context *ctxt,
			 const struct ext2_inode_large *inode)
{
	return ctxt->gid == inode_gid(*inode);
}
#endif

static int op_chmod(const char *path, mode_t mode
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
			, struct fuse_file_info *fi EXT2FS_ATTR((unused))
#endif
			)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf(ff, "%s: path=%s mode=0%o ino=%d\n", __func__, path, mode, ino);

	err = fuse2fs_read_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	ret = check_iflags_access(ff, ino, EXT2_INODE(&inode), W_OK);
	if (ret)
		goto out;

	if (want_check_owner(ff, ctxt) && ctxt->uid != inode_uid(inode)) {
		ret = -EPERM;
		goto out;
	}

	/*
	 * XXX: We should really check that the inode gid is not in /any/
	 * of the user's groups, but FUSE only tells us about the primary
	 * group.
	 */
	if (!is_superuser(ff, ctxt)) {
		ret = in_file_group(ctxt, &inode);
		if (ret < 0)
			goto out;

		if (!ret)
			mode &= ~S_ISGID;
	}

	inode.i_mode &= ~0xFFF;
	inode.i_mode |= mode & 0xFFF;

	dbg_printf(ff, "%s: path=%s new_mode=0%o ino=%d\n", __func__,
		   path, inode.i_mode, ino);

	ret = update_ctime(fs, ino, &inode);
	if (ret)
		goto out;

	err = fuse2fs_write_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_chown(const char *path, uid_t owner, gid_t group
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
			, struct fuse_file_info *fi EXT2FS_ATTR((unused))
#endif
			)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf(ff, "%s: path=%s owner=%d group=%d ino=%d\n", __func__,
		   path, owner, group, ino);

	err = fuse2fs_read_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	ret = check_iflags_access(ff, ino, EXT2_INODE(&inode), W_OK);
	if (ret)
		goto out;

	/* FUSE seems to feed us ~0 to mean "don't change" */
	if (owner != (uid_t) ~0) {
		/* Only root gets to change UID. */
		if (want_check_owner(ff, ctxt) &&
		    !(inode_uid(inode) == ctxt->uid && owner == ctxt->uid)) {
			ret = -EPERM;
			goto out;
		}
		inode.i_uid = owner;
		ext2fs_set_i_uid_high(inode, owner >> 16);
	}

	if (group != (gid_t) ~0) {
		/* Only root or the owner get to change GID. */
		if (want_check_owner(ff, ctxt) &&
		    inode_uid(inode) != ctxt->uid) {
			ret = -EPERM;
			goto out;
		}

		/* XXX: We /should/ check group membership but FUSE */
		inode.i_gid = group;
		ext2fs_set_i_gid_high(inode, group >> 16);
	}

	ret = update_ctime(fs, ino, &inode);
	if (ret)
		goto out;

	err = fuse2fs_write_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int punch_posteof(struct fuse2fs *ff, ext2_ino_t ino, off_t new_size)
{
	ext2_filsys fs = ff->fs;
	struct ext2_inode_large inode;
	blk64_t truncate_block = FUSE2FS_B_TO_FSB(ff, new_size);
	errcode_t err;

	err = fuse2fs_read_inode(fs, ino, &inode);
	if (err)
		return translate_error(fs, ino, err);

	err = ext2fs_punch(fs, ino, EXT2_INODE(&inode), 0, truncate_block,
			   ~0ULL);
	if (err)
		return translate_error(fs, ino, err);

	return 0;
}

static int truncate_helper(struct fuse2fs *ff, ext2_ino_t ino, off_t new_size)
{
	ext2_filsys fs = ff->fs;
	ext2_file_t file;
	__u64 old_isize;
	errcode_t err;
	int ret = 0;

	err = ext2fs_file_open(fs, ino, EXT2_FILE_WRITE, &file);
	if (err)
		return translate_error(fs, ino, err);

	err = ext2fs_file_get_lsize(file, &old_isize);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out_close;
	}

	dbg_printf(ff, "%s: ino=%u isize=0x%llx new_size=0x%llx\n", __func__,
		   ino,
		   (unsigned long long)old_isize,
		   (unsigned long long)new_size);

	err = ext2fs_file_set_size2(file, new_size);
	if (err)
		ret = translate_error(fs, ino, err);

out_close:
	err = ext2fs_file_close(file);
	if (ret)
		return ret;
	if (err)
		return translate_error(fs, ino, err);

	ret = update_mtime(fs, ino, NULL);
	if (ret)
		return ret;

	/*
	 * Truncating to the current size is usually understood to mean that
	 * we should clear out post-EOF preallocations.
	 */
	if (new_size == old_isize)
		return punch_posteof(ff, ino, new_size);

	return 0;
}

static int op_truncate(const char *path, off_t len
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
			, struct fuse_file_info *fi EXT2FS_ATTR((unused))
#endif
			)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	if (!ino) {
		ret = -ESTALE;
		goto out;
	}
	dbg_printf(ff, "%s: ino=%d len=%jd\n", __func__, ino, (intmax_t) len);

	ret = check_inum_access(ff, ino, W_OK);
	if (ret)
		goto out;

	ret = truncate_helper(ff, ino, len);
	if (ret)
		goto out;

out:
	pthread_mutex_unlock(&ff->bfl);
	return err;
}

#ifdef __linux__
static void detect_linux_executable_open(int kernel_flags, int *access_check,
				  int *e2fs_open_flags)
{
	/*
	 * On Linux, execve will bleed __FMODE_EXEC into the file mode flags,
	 * and FUSE is more than happy to let that slip through.
	 */
	if (kernel_flags & 0x20) {
		*access_check = X_OK;
		*e2fs_open_flags &= ~EXT2_FILE_WRITE;
	}
}
#else
static void detect_linux_executable_open(int kernel_flags, int *access_check,
				  int *e2fs_open_flags)
{
	/* empty */
}
#endif /* __linux__ */

static int __op_open(struct fuse2fs *ff, const char *path,
		     struct fuse_file_info *fp)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	struct fuse2fs_file_handle *file;
	int check = 0, ret = 0;

	dbg_printf(ff, "%s: path=%s oflags=0o%o\n", __func__, path, fp->flags);
	err = ext2fs_get_mem(sizeof(*file), &file);
	if (err)
		return translate_error(fs, 0, err);
	file->magic = FUSE2FS_FILE_MAGIC;

	file->open_flags = 0;
	switch (fp->flags & O_ACCMODE) {
	case O_RDONLY:
		check = R_OK;
		break;
	case O_WRONLY:
		check = W_OK;
		file->open_flags |= EXT2_FILE_WRITE;
		break;
	case O_RDWR:
		check = R_OK | W_OK;
		file->open_flags |= EXT2_FILE_WRITE;
		break;
	}
	if (fp->flags & O_APPEND) {
		/* the kernel doesn't allow truncation of an append-only file */
		if (fp->flags & O_TRUNC) {
			ret = -EPERM;
			goto out;
		}

		check |= A_OK;
	}

	detect_linux_executable_open(fp->flags, &check, &file->open_flags);

	if (fp->flags & O_CREAT)
		file->open_flags |= EXT2_FILE_CREATE;

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &file->ino);
	if (err || file->ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf(ff, "%s: ino=%d\n", __func__, file->ino);

	ret = check_inum_access(ff, file->ino, check);
	if (ret) {
		/*
		 * In a regular (Linux) fs driver, the kernel will open
		 * binaries for reading if the user has --x privileges (i.e.
		 * execute without read).  Since the kernel doesn't have any
		 * way to tell us if it's opening a file via execve, we'll
		 * just assume that allowing access is ok if asking for ro mode
		 * fails but asking for x mode succeeds.  Of course we can
		 * also employ undocumented hacks (see above).
		 */
		if (check == R_OK) {
			ret = check_inum_access(ff, file->ino, X_OK);
			if (ret)
				goto out;
		} else
			goto out;
	}

	if (fp->flags & O_TRUNC) {
		ret = truncate_helper(ff, file->ino, 0);
		if (ret)
			goto out;
	}

	fp->fh = (uintptr_t)file;

out:
	if (ret)
		ext2fs_free_mem(&file);
	return ret;
}

static int op_open(const char *path, struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	int ret;

	FUSE2FS_CHECK_CONTEXT(ff);
	pthread_mutex_lock(&ff->bfl);
	ret = __op_open(ff, path, fp);
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_read(const char *path EXT2FS_ATTR((unused)), char *buf,
		   size_t len, off_t offset,
		   struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	ext2_file_t efp;
	errcode_t err;
	unsigned int got = 0;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d off=%jd len=%jd\n", __func__, fh->ino,
		   (intmax_t) offset, len);
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_file_open(fs, fh->ino, fh->open_flags, &efp);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	err = ext2fs_file_llseek(efp, offset, SEEK_SET, NULL);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

	err = ext2fs_file_read(efp, buf, len, &got);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

out2:
	err = ext2fs_file_close(efp);
	if (ret)
		goto out;
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	if (fs_writeable(fs)) {
		ret = update_atime(fs, fh->ino);
		if (ret)
			goto out;
	}
out:
	pthread_mutex_unlock(&ff->bfl);
	return got ? (int) got : ret;
}

static int op_write(const char *path EXT2FS_ATTR((unused)),
		    const char *buf, size_t len, off_t offset,
		    struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	ext2_file_t efp;
	errcode_t err;
	unsigned int got = 0;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d off=%jd len=%jd\n", __func__, fh->ino,
		   (intmax_t) offset, (intmax_t) len);
	pthread_mutex_lock(&ff->bfl);
	if (!fs_writeable(fs)) {
		ret = -EROFS;
		goto out;
	}

	if (!fs_can_allocate(ff, FUSE2FS_B_TO_FSB(ff, len))) {
		ret = -ENOSPC;
		goto out;
	}

	err = ext2fs_file_open(fs, fh->ino, fh->open_flags, &efp);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	err = ext2fs_file_llseek(efp, offset, SEEK_SET, NULL);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

	err = ext2fs_file_write(efp, buf, len, &got);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

	err = ext2fs_file_flush(efp);
	if (err) {
		got = 0;
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

out2:
	err = ext2fs_file_close(efp);
	if (ret)
		goto out;
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	ret = update_mtime(fs, fh->ino, NULL);
	if (ret)
		goto out;

out:
	pthread_mutex_unlock(&ff->bfl);
	return got ? (int) got : ret;
}

static int op_release(const char *path EXT2FS_ATTR((unused)),
		      struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d\n", __func__, fh->ino);
	pthread_mutex_lock(&ff->bfl);
	if (fs_writeable(fs) && fh->open_flags & EXT2_FILE_WRITE) {
		err = ext2fs_flush2(fs, EXT2_FLAG_FLUSH_NO_SYNC);
		if (err)
			ret = translate_error(fs, fh->ino, err);
	}
	fp->fh = 0;
	pthread_mutex_unlock(&ff->bfl);

	ext2fs_free_mem(&fh);

	return ret;
}

static int op_fsync(const char *path EXT2FS_ATTR((unused)),
		    int datasync EXT2FS_ATTR((unused)),
		    struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d\n", __func__, fh->ino);
	/* For now, flush everything, even if it's slow */
	pthread_mutex_lock(&ff->bfl);
	if (fs_writeable(fs) && fh->open_flags & EXT2_FILE_WRITE) {
		err = ext2fs_flush2(fs, 0);
		if (err)
			ret = translate_error(fs, fh->ino, err);
	}
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

static int op_statfs(const char *path EXT2FS_ATTR((unused)),
		     struct statvfs *buf)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	uint64_t fsid, *f;
	blk64_t overhead, reserved, free;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf(ff, "%s: path=%s\n", __func__, path);
	buf->f_bsize = fs->blocksize;
	buf->f_frsize = 0;

	if (ff->minixdf)
		overhead = 0;
	else
		overhead = fs->desc_blocks +
			   (blk64_t)fs->group_desc_count *
			   (fs->inode_blocks_per_group + 2);
	reserved = ext2fs_r_blocks_count(fs->super);
	if (!reserved)
		reserved = ext2fs_blocks_count(fs->super) / 10;
	free = ext2fs_free_blocks_count(fs->super);

	buf->f_blocks = ext2fs_blocks_count(fs->super) - overhead;
	buf->f_bfree = free;
	if (free < reserved)
		buf->f_bavail = 0;
	else
		buf->f_bavail = free - reserved;
	buf->f_files = fs->super->s_inodes_count;
	buf->f_ffree = fs->super->s_free_inodes_count;
	buf->f_favail = fs->super->s_free_inodes_count;
	f = (uint64_t *)fs->super->s_uuid;
	fsid = *f;
	f++;
	fsid ^= *f;
	buf->f_fsid = fsid;
	buf->f_flag = 0;
	if (fs->flags & EXT2_FLAG_RW)
		buf->f_flag |= ST_RDONLY;
	buf->f_namemax = EXT2_NAME_LEN;

	return 0;
}

static const char *valid_xattr_prefixes[] = {
	"user.",
	"trusted.",
	"security.",
	"gnu.",
	"system.",
};

static int validate_xattr_name(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(valid_xattr_prefixes); i++) {
		if (!strncmp(name, valid_xattr_prefixes[i],
					strlen(valid_xattr_prefixes[i])))
			return 1;
	}

	return 0;
}

static int op_getxattr(const char *path, const char *key, char *value,
		       size_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	void *ptr;
	size_t plen;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	if (!validate_xattr_name(key))
		return -ENODATA;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	if (!ext2fs_has_feature_xattr(fs->super)) {
		ret = -ENOTSUP;
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf(ff, "%s: ino=%d name=%s\n", __func__, ino, key);

	ret = check_inum_access(ff, ino, R_OK);
	if (ret)
		goto out;

	ret = __getxattr(ff, ino, key, &ptr, &plen);
	if (ret)
		goto out;

	if (!len) {
		ret = plen;
	} else if (len < plen) {
		ret = -ERANGE;
	} else {
		memcpy(value, ptr, plen);
		ret = plen;
	}

	ext2fs_free_mem(&ptr);
out:
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

static int count_buffer_space(char *name, char *value EXT2FS_ATTR((unused)),
			      size_t value_len EXT2FS_ATTR((unused)),
			      void *data)
{
	unsigned int *x = data;

	*x = *x + strlen(name) + 1;
	return 0;
}

static int copy_names(char *name, char *value EXT2FS_ATTR((unused)),
		      size_t value_len EXT2FS_ATTR((unused)), void *data)
{
	char **b = data;
	size_t name_len = strlen(name);

	memcpy(*b, name, name_len + 1);
	*b = *b + name_len + 1;

	return 0;
}

static int op_listxattr(const char *path, char *names, size_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	struct ext2_xattr_handle *h;
	unsigned int bufsz;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	if (!ext2fs_has_feature_xattr(fs->super)) {
		ret = -ENOTSUP;
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, ino, err);
		goto out;
	}
	dbg_printf(ff, "%s: ino=%d\n", __func__, ino);

	ret = check_inum_access(ff, ino, R_OK);
	if (ret)
		goto out;

	err = ext2fs_xattrs_open(fs, ino, &h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	err = ext2fs_xattrs_read(h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	/* Count buffer space needed for names */
	bufsz = 0;
	err = ext2fs_xattrs_iterate(h, count_buffer_space, &bufsz);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	if (len == 0) {
		ret = bufsz;
		goto out2;
	} else if (len < bufsz) {
		ret = -ERANGE;
		goto out2;
	}

	/* Copy names out */
	memset(names, 0, len);
	err = ext2fs_xattrs_iterate(h, copy_names, &names);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}
	ret = bufsz;
out2:
	err = ext2fs_xattrs_close(&h);
	if (err && !ret)
		ret = translate_error(fs, ino, err);
out:
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

static int op_setxattr(const char *path EXT2FS_ATTR((unused)),
		       const char *key, const char *value,
		       size_t len, int flags)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	struct ext2_xattr_handle *h;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	if (flags & ~(XATTR_CREATE | XATTR_REPLACE))
		return -EOPNOTSUPP;

	if (!validate_xattr_name(key))
		return -EINVAL;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	if (!ext2fs_has_feature_xattr(fs->super)) {
		ret = -ENOTSUP;
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf(ff, "%s: ino=%d name=%s\n", __func__, ino, key);

	ret = check_inum_access(ff, ino, W_OK);
	if (ret == -EACCES) {
		ret = -EPERM;
		goto out;
	} else if (ret)
		goto out;

	err = ext2fs_xattrs_open(fs, ino, &h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	err = ext2fs_xattrs_read(h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	if (flags & (XATTR_CREATE | XATTR_REPLACE)) {
		void *buf;
		size_t buflen;

		err = ext2fs_xattr_get(h, key, &buf, &buflen);
		switch (err) {
		case EXT2_ET_EA_KEY_NOT_FOUND:
			if (flags & XATTR_REPLACE) {
				ret = -ENODATA;
				goto out2;
			}
			break;
		case 0:
			ext2fs_free_mem(&buf);
			if (flags & XATTR_CREATE) {
				ret = -EEXIST;
				goto out2;
			}
			break;
		default:
			ret = translate_error(fs, ino, err);
			goto out2;
		}
	}

	err = ext2fs_xattr_set(h, key, value, len);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	ret = update_ctime(fs, ino, NULL);
out2:
	err = ext2fs_xattrs_close(&h);
	if (!ret && err)
		ret = translate_error(fs, ino, err);
out:
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

static int op_removexattr(const char *path, const char *key)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	struct ext2_xattr_handle *h;
	void *buf;
	size_t buflen;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	/*
	 * Once in a while libfuse gives us a no-name xattr to delete as part
	 * of clearing ACLs.  Just pretend we cleared them.
	 */
	if (key[0] == 0)
		return 0;

	if (!validate_xattr_name(key))
		return -ENODATA;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	if (!ext2fs_has_feature_xattr(fs->super)) {
		ret = -ENOTSUP;
		goto out;
	}

	if (!fs_can_allocate(ff, 1)) {
		ret = -ENOSPC;
		goto out;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf(ff, "%s: ino=%d name=%s\n", __func__, ino, key);

	ret = check_inum_access(ff, ino, W_OK);
	if (ret)
		goto out;

	err = ext2fs_xattrs_open(fs, ino, &h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	err = ext2fs_xattrs_read(h);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	err = ext2fs_xattr_get(h, key, &buf, &buflen);
	switch (err) {
	case EXT2_ET_EA_KEY_NOT_FOUND:
		/*
		 * ACLs are special snowflakes that require a 0 return when
		 * the ACL never existed in the first place.
		 */
		if (!strncmp(XATTR_SECURITY_PREFIX, key,
			     XATTR_SECURITY_PREFIX_LEN))
			ret = 0;
		else
			ret = -ENODATA;
		goto out2;
	case 0:
		ext2fs_free_mem(&buf);
		break;
	default:
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	err = ext2fs_xattr_remove(h, key);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out2;
	}

	ret = update_ctime(fs, ino, NULL);
out2:
	err = ext2fs_xattrs_close(&h);
	if (err && !ret)
		ret = translate_error(fs, ino, err);
out:
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}

struct readdir_iter {
	void *buf;
	ext2_filsys fs;
	fuse_fill_dir_t func;
};

static inline mode_t dirent_fmode(ext2_filsys fs,
				   const struct ext2_dir_entry *dirent)
{
	if (!ext2fs_has_feature_filetype(fs->super))
		return 0;

	switch (ext2fs_dirent_file_type(dirent)) {
	case EXT2_FT_REG_FILE:
		return S_IFREG;
	case EXT2_FT_DIR:
		return S_IFDIR;
	case EXT2_FT_CHRDEV:
		return S_IFCHR;
	case EXT2_FT_BLKDEV:
		return S_IFBLK;
	case EXT2_FT_FIFO:
		return S_IFIFO;
	case EXT2_FT_SOCK:
		return S_IFSOCK;
	case EXT2_FT_SYMLINK:
		return S_IFLNK;
	}

	return 0;
}

static int op_readdir_iter(ext2_ino_t dir EXT2FS_ATTR((unused)),
			   int entry EXT2FS_ATTR((unused)),
			   struct ext2_dir_entry *dirent,
			   int offset EXT2FS_ATTR((unused)),
			   int blocksize EXT2FS_ATTR((unused)),
			   char *buf EXT2FS_ATTR((unused)), void *data)
{
	struct readdir_iter *i = data;
	char namebuf[EXT2_NAME_LEN + 1];
	struct stat stat = {
		.st_ino = dirent->inode,
		.st_mode = dirent_fmode(i->fs, dirent),
	};
	int ret;

	memcpy(namebuf, dirent->name, dirent->name_len & 0xFF);
	namebuf[dirent->name_len & 0xFF] = 0;
	ret = i->func(i->buf, namebuf, &stat, 0
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
			, 0
#endif
			);
	if (ret)
		return DIRENT_ABORT;

	return 0;
}

static int op_readdir(const char *path EXT2FS_ATTR((unused)),
		      void *buf, fuse_fill_dir_t fill_func,
		      off_t offset EXT2FS_ATTR((unused)),
		      struct fuse_file_info *fp
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
			, enum fuse_readdir_flags flags EXT2FS_ATTR((unused))
#endif
			)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	errcode_t err;
	struct readdir_iter i;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	i.fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(i.fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d\n", __func__, fh->ino);
	pthread_mutex_lock(&ff->bfl);
	i.buf = buf;
	i.func = fill_func;
	err = ext2fs_dir_iterate2(i.fs, fh->ino, 0, NULL, op_readdir_iter, &i);
	if (err) {
		ret = translate_error(i.fs, fh->ino, err);
		goto out;
	}

	if (fs_writeable(i.fs)) {
		ret = update_atime(i.fs, fh->ino);
		if (ret)
			goto out;
	}
out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_access(const char *path, int mask)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf(ff, "%s: path=%s mask=0x%x\n", __func__, path, mask);
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err || ino == 0) {
		ret = translate_error(fs, 0, err);
		goto out;
	}

	ret = check_inum_access(ff, ino, mask);
	if (ret)
		goto out;

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_create(const char *path, mode_t mode, struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t parent, child;
	char *temp_path;
	errcode_t err;
	char *node_name, a;
	int filetype;
	struct ext2_inode_large inode;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	dbg_printf(ff, "%s: path=%s mode=0%o\n", __func__, path, mode);
	temp_path = strdup(path);
	if (!temp_path) {
		ret = -ENOMEM;
		goto out;
	}
	node_name = strrchr(temp_path, '/');
	if (!node_name) {
		ret = -ENOMEM;
		goto out;
	}
	node_name++;
	a = *node_name;
	*node_name = 0;

	pthread_mutex_lock(&ff->bfl);
	if (!fs_can_allocate(ff, 1)) {
		ret = -ENOSPC;
		goto out2;
	}

	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, temp_path,
			   &parent);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out2;
	}

	ret = check_inum_access(ff, parent, A_OK | W_OK);
	if (ret)
		goto out2;

	*node_name = a;

	filetype = ext2_file_type(mode);

	err = ext2fs_new_inode(fs, parent, mode, 0, &child);
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	dbg_printf(ff, "%s: creating ino=%d/name=%s in dir=%d\n", __func__, child,
		   node_name, parent);
	err = ext2fs_link(fs, parent, node_name, child,
			  filetype | EXT2FS_LINK_EXPAND);
	if (err) {
		ret = translate_error(fs, parent, err);
		goto out2;
	}

	ret = update_mtime(fs, parent, NULL);
	if (ret)
		goto out2;

	memset(&inode, 0, sizeof(inode));
	inode.i_mode = mode;
	inode.i_links_count = 1;
	inode.i_extra_isize = sizeof(struct ext2_inode_large) -
		EXT2_GOOD_OLD_INODE_SIZE;
	inode.i_uid = ctxt->uid;
	ext2fs_set_i_uid_high(inode, ctxt->uid >> 16);
	inode.i_gid = ctxt->gid;
	ext2fs_set_i_gid_high(inode, ctxt->gid >> 16);
	if (ext2fs_has_feature_extents(fs->super)) {
		ext2_extent_handle_t handle;

		inode.i_flags &= ~EXT4_EXTENTS_FL;
		ret = ext2fs_extent_open2(fs, child,
					  EXT2_INODE(&inode), &handle);
		if (ret) {
			ret = translate_error(fs, child, err);
			goto out2;
		}

		ext2fs_extent_free(handle);
	}

	err = ext2fs_write_new_inode(fs, child, EXT2_INODE(&inode));
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	inode.i_generation = ff->next_generation++;
	init_times(&inode);
	err = fuse2fs_write_inode(fs, child, &inode);
	if (err) {
		ret = translate_error(fs, child, err);
		goto out2;
	}

	ext2fs_inode_alloc_stats2(fs, child, 1, 0);

	ret = propagate_default_acls(ff, parent, child);
	if (ret)
		goto out2;

	ret = __op_open(ff, path, fp);
	if (ret)
		goto out2;
out2:
	pthread_mutex_unlock(&ff->bfl);
out:
	free(temp_path);
	return ret;
}

#if FUSE_VERSION < FUSE_MAKE_VERSION(3, 0)
static int op_ftruncate(const char *path EXT2FS_ATTR((unused)),
			off_t len, struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	ext2_file_t efp;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d len=%jd\n", __func__, fh->ino,
		   (intmax_t) len);
	pthread_mutex_lock(&ff->bfl);
	if (!fs_writeable(fs)) {
		ret = -EROFS;
		goto out;
	}

	err = ext2fs_file_open(fs, fh->ino, fh->open_flags, &efp);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	err = ext2fs_file_set_size2(efp, len);
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out2;
	}

out2:
	err = ext2fs_file_close(efp);
	if (ret)
		goto out;
	if (err) {
		ret = translate_error(fs, fh->ino, err);
		goto out;
	}

	ret = update_mtime(fs, fh->ino, NULL);
	if (ret)
		goto out;

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

static int op_fgetattr(const char *path EXT2FS_ATTR((unused)),
		       struct stat *statbuf,
		       struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d\n", __func__, fh->ino);
	pthread_mutex_lock(&ff->bfl);
	ret = stat_inode(fs, fh->ino, statbuf);
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}
#endif /* FUSE_VERSION < FUSE_MAKE_VERSION(3, 0) */

static int op_utimens(const char *path, const struct timespec ctv[2]
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
			, struct fuse_file_info *fi EXT2FS_ATTR((unused))
#endif
			)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct timespec tv[2];
	ext2_filsys fs;
	errcode_t err;
	ext2_ino_t ino;
	struct ext2_inode_large inode;
	int access = W_OK;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf(ff, "%s: ino=%d atime=%lld.%ld mtime=%lld.%ld\n", __func__,
			ino,
			(long long int)ctv[0].tv_sec, ctv[0].tv_nsec,
			(long long int)ctv[1].tv_sec, ctv[1].tv_nsec);

	/*
	 * ext4 allows timestamp updates of append-only files but only if we're
	 * setting to current time
	 */
	if (ctv[0].tv_nsec == UTIME_NOW && ctv[1].tv_nsec == UTIME_NOW)
		access |= A_OK;
	ret = check_inum_access(ff, ino, access);
	if (ret)
		goto out;

	err = fuse2fs_read_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

	tv[0] = ctv[0];
	tv[1] = ctv[1];
#ifdef UTIME_NOW
	if (tv[0].tv_nsec == UTIME_NOW)
		get_now(tv);
	if (tv[1].tv_nsec == UTIME_NOW)
		get_now(tv + 1);
#endif /* UTIME_NOW */
#ifdef UTIME_OMIT
	if (tv[0].tv_nsec != UTIME_OMIT)
		EXT4_INODE_SET_XTIME(i_atime, &tv[0], &inode);
	if (tv[1].tv_nsec != UTIME_OMIT)
		EXT4_INODE_SET_XTIME(i_mtime, &tv[1], &inode);
#endif /* UTIME_OMIT */
	ret = update_ctime(fs, ino, &inode);
	if (ret)
		goto out;

	err = fuse2fs_write_inode(fs, ino, &inode);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

#define FUSE2FS_MODIFIABLE_IFLAGS \
	(EXT2_FL_USER_MODIFIABLE & ~(EXT4_EXTENTS_FL | EXT4_CASEFOLD_FL | \
				     EXT3_JOURNAL_DATA_FL))

static inline int set_iflags(struct ext2_inode_large *inode, __u32 iflags)
{
	if ((inode->i_flags ^ iflags) & ~FUSE2FS_MODIFIABLE_IFLAGS)
		return -EINVAL;

	inode->i_flags = (inode->i_flags & ~FUSE2FS_MODIFIABLE_IFLAGS) |
			 (iflags & FUSE2FS_MODIFIABLE_IFLAGS);
	return 0;
}

#ifdef SUPPORT_I_FLAGS
static int ioctl_getflags(struct fuse2fs *ff, struct fuse2fs_file_handle *fh,
			  void *data)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	struct ext2_inode_large inode;

	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d\n", __func__, fh->ino);
	err = fuse2fs_read_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	*(__u32 *)data = inode.i_flags & EXT2_FL_USER_VISIBLE;
	return 0;
}

static int ioctl_setflags(struct fuse2fs *ff, struct fuse2fs_file_handle *fh,
			  void *data)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	struct ext2_inode_large inode;
	int ret;
	__u32 flags = *(__u32 *)data;
	struct fuse_context *ctxt = fuse_get_context();

	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d\n", __func__, fh->ino);
	err = fuse2fs_read_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	if (want_check_owner(ff, ctxt) && inode_uid(inode) != ctxt->uid)
		return -EPERM;

	ret = set_iflags(&inode, flags);
	if (ret)
		return ret;

	ret = update_ctime(fs, fh->ino, &inode);
	if (ret)
		return ret;

	err = fuse2fs_write_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	return 0;
}

static int ioctl_getversion(struct fuse2fs *ff, struct fuse2fs_file_handle *fh,
			    void *data)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	struct ext2_inode_large inode;

	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d\n", __func__, fh->ino);
	err = fuse2fs_read_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	*(__u32 *)data = inode.i_generation;
	return 0;
}

static int ioctl_setversion(struct fuse2fs *ff, struct fuse2fs_file_handle *fh,
			    void *data)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	struct ext2_inode_large inode;
	int ret;
	__u32 generation = *(__u32 *)data;
	struct fuse_context *ctxt = fuse_get_context();

	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d\n", __func__, fh->ino);
	err = fuse2fs_read_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	if (want_check_owner(ff, ctxt) && inode_uid(inode) != ctxt->uid)
		return -EPERM;

	inode.i_generation = generation;

	ret = update_ctime(fs, fh->ino, &inode);
	if (ret)
		return ret;

	err = fuse2fs_write_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	return 0;
}
#endif /* SUPPORT_I_FLAGS */

#ifdef FS_IOC_FSGETXATTR
static __u32 iflags_to_fsxflags(__u32 iflags)
{
	__u32 xflags = 0;

	if (iflags & FS_SYNC_FL)
		xflags |= FS_XFLAG_SYNC;
	if (iflags & FS_IMMUTABLE_FL)
		xflags |= FS_XFLAG_IMMUTABLE;
	if (iflags & FS_APPEND_FL)
		xflags |= FS_XFLAG_APPEND;
	if (iflags & FS_NODUMP_FL)
		xflags |= FS_XFLAG_NODUMP;
	if (iflags & FS_NOATIME_FL)
		xflags |= FS_XFLAG_NOATIME;
	if (iflags & FS_DAX_FL)
		xflags |= FS_XFLAG_DAX;
	if (iflags & FS_PROJINHERIT_FL)
		xflags |= FS_XFLAG_PROJINHERIT;
	return xflags;
}

static int ioctl_fsgetxattr(struct fuse2fs *ff, struct fuse2fs_file_handle *fh,
			    void *data)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	struct ext2_inode_large inode;
	struct fsxattr *fsx = data;
	unsigned int inode_size;

	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d\n", __func__, fh->ino);
	err = fuse2fs_read_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	memset(fsx, 0, sizeof(*fsx));
	inode_size = EXT2_GOOD_OLD_INODE_SIZE + inode.i_extra_isize;
	if (ext2fs_inode_includes(inode_size, i_projid))
		fsx->fsx_projid = inode_projid(inode);
	fsx->fsx_xflags = iflags_to_fsxflags(inode.i_flags);
	return 0;
}

static __u32 fsxflags_to_iflags(__u32 xflags)
{
	__u32 iflags = 0;

	if (xflags & FS_XFLAG_IMMUTABLE)
		iflags |= FS_IMMUTABLE_FL;
	if (xflags & FS_XFLAG_APPEND)
		iflags |= FS_APPEND_FL;
	if (xflags & FS_XFLAG_SYNC)
		iflags |= FS_SYNC_FL;
	if (xflags & FS_XFLAG_NOATIME)
		iflags |= FS_NOATIME_FL;
	if (xflags & FS_XFLAG_NODUMP)
		iflags |= FS_NODUMP_FL;
	if (xflags & FS_XFLAG_DAX)
		iflags |= FS_DAX_FL;
	if (xflags & FS_XFLAG_PROJINHERIT)
		iflags |= FS_PROJINHERIT_FL;
	return iflags;
}

static int ioctl_fssetxattr(struct fuse2fs *ff, struct fuse2fs_file_handle *fh,
			    void *data)
{
	ext2_filsys fs = ff->fs;
	errcode_t err;
	struct ext2_inode_large inode;
	int ret;
	struct fuse_context *ctxt = fuse_get_context();
	struct fsxattr *fsx = data;
	__u32 flags = fsxflags_to_iflags(fsx->fsx_xflags);
	unsigned int inode_size;

	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: ino=%d\n", __func__, fh->ino);
	err = fuse2fs_read_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	if (want_check_owner(ff, ctxt) && inode_uid(inode) != ctxt->uid)
		return -EPERM;

	ret = set_iflags(&inode, flags);
	if (ret)
		return ret;

	inode_size = EXT2_GOOD_OLD_INODE_SIZE + inode.i_extra_isize;
	if (ext2fs_inode_includes(inode_size, i_projid))
		inode.i_projid = fsx->fsx_projid;

	ret = update_ctime(fs, fh->ino, &inode);
	if (ret)
		return ret;

	err = fuse2fs_write_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	return 0;
}
#endif /* FS_IOC_FSGETXATTR */

#ifdef FITRIM
static int ioctl_fitrim(struct fuse2fs *ff, struct fuse2fs_file_handle *fh,
			void *data)
{
	ext2_filsys fs = ff->fs;
	struct fstrim_range *fr = data;
	blk64_t start, end, max_blocks, b, cleared, minlen;
	blk64_t max_blks = ext2fs_blocks_count(fs->super);
	errcode_t err = 0;

	if (!fs_writeable(fs))
		return -EROFS;

	start = FUSE2FS_B_TO_FSBT(ff, fr->start);
	end = FUSE2FS_B_TO_FSBT(ff, fr->start + fr->len - 1);
	minlen = FUSE2FS_B_TO_FSBT(ff, fr->minlen);

	if (EXT2FS_NUM_B2C(fs, minlen) > EXT2_CLUSTERS_PER_GROUP(fs->super) ||
	    start >= max_blks ||
	    fr->len < fs->blocksize)
		return -EINVAL;

	dbg_printf(ff, "%s: start=%llu end=%llu minlen=%llu\n", __func__,
		   start, end, minlen);

	if (start < fs->super->s_first_data_block)
		start = fs->super->s_first_data_block;

	if (end < fs->super->s_first_data_block)
		end = fs->super->s_first_data_block;
	if (end >= ext2fs_blocks_count(fs->super))
		end = ext2fs_blocks_count(fs->super) - 1;

	cleared = 0;
	max_blocks = FUSE2FS_B_TO_FSBT(ff, 2048ULL * 1024 * 1024);

	fr->len = 0;
	while (start <= end) {
		err = ext2fs_find_first_zero_block_bitmap2(fs->block_map,
							   start, end, &start);
		switch (err) {
		case 0:
			break;
		case ENOENT:
			/* no free blocks found, so we're done */
			err = 0;
			goto out;
		default:
			return translate_error(fs, fh->ino, err);
		}

		b = start + max_blocks < end ? start + max_blocks : end;
		err =  ext2fs_find_first_set_block_bitmap2(fs->block_map,
							   start, b, &b);
		if (err && err != ENOENT)
			return translate_error(fs, fh->ino, err);
		if (b - start >= minlen) {
			err = io_channel_discard(fs->io, start, b - start);
			if (err)
				return translate_error(fs, fh->ino, err);
			cleared += b - start;
			fr->len = FUSE2FS_FSB_TO_B(ff, cleared);
		}
		start = b + 1;
	}

out:
	fr->len = cleared;
	return err;
}
#endif /* FITRIM */

#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 8)
static int op_ioctl(const char *path EXT2FS_ATTR((unused)),
#if FUSE_VERSION >= FUSE_MAKE_VERSION(3, 0)
		    unsigned int cmd,
#else
		    int cmd,
#endif
		    void *arg EXT2FS_ATTR((unused)),
		    struct fuse_file_info *fp,
		    unsigned int flags EXT2FS_ATTR((unused)), void *data)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	pthread_mutex_lock(&ff->bfl);
	switch ((unsigned long) cmd) {
#ifdef SUPPORT_I_FLAGS
	case EXT2_IOC_GETFLAGS:
		ret = ioctl_getflags(ff, fh, data);
		break;
	case EXT2_IOC_SETFLAGS:
		ret = ioctl_setflags(ff, fh, data);
		break;
	case EXT2_IOC_GETVERSION:
		ret = ioctl_getversion(ff, fh, data);
		break;
	case EXT2_IOC_SETVERSION:
		ret = ioctl_setversion(ff, fh, data);
		break;
#endif
#ifdef FS_IOC_FSGETXATTR
	case FS_IOC_FSGETXATTR:
		ret = ioctl_fsgetxattr(ff, fh, data);
		break;
	case FS_IOC_FSSETXATTR:
		ret = ioctl_fssetxattr(ff, fh, data);
		break;
#endif
#ifdef FITRIM
	case FITRIM:
		ret = ioctl_fitrim(ff, fh, data);
		break;
#endif
	default:
		dbg_printf(ff, "%s: Unknown ioctl %d\n", __func__, cmd);
		ret = -ENOTTY;
	}
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}
#endif /* FUSE 28 */

static int op_bmap(const char *path, size_t blocksize EXT2FS_ATTR((unused)),
		   uint64_t *idx)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs;
	ext2_ino_t ino;
	errcode_t err;
	int ret = 0;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	pthread_mutex_lock(&ff->bfl);
	err = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, &ino);
	if (err) {
		ret = translate_error(fs, 0, err);
		goto out;
	}
	dbg_printf(ff, "%s: ino=%d blk=%"PRIu64"\n", __func__, ino, *idx);

	err = ext2fs_bmap2(fs, ino, NULL, NULL, 0, *idx, 0, (blk64_t *)idx);
	if (err) {
		ret = translate_error(fs, ino, err);
		goto out;
	}

out:
	pthread_mutex_unlock(&ff->bfl);
	return ret;
}

#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 9)
# ifdef SUPPORT_FALLOCATE
static int fallocate_helper(struct fuse_file_info *fp, int mode, off_t offset,
			    off_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	struct ext2_inode_large inode;
	blk64_t start, end;
	__u64 fsize;
	errcode_t err;
	int flags;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	start = FUSE2FS_B_TO_FSBT(ff, offset);
	end = FUSE2FS_B_TO_FSBT(ff, offset + len - 1);
	dbg_printf(ff, "%s: ino=%d mode=0x%x start=%llu end=%llu\n", __func__,
		   fh->ino, mode, start, end);
	if (!fs_can_allocate(ff, FUSE2FS_B_TO_FSB(ff, len)))
		return -ENOSPC;

	err = fuse2fs_read_inode(fs, fh->ino, &inode);
	if (err)
		return err;
	fsize = EXT2_I_SIZE(&inode);

	/* Allocate a bunch of blocks */
	flags = (mode & FL_KEEP_SIZE_FLAG ? 0 :
			EXT2_FALLOCATE_INIT_BEYOND_EOF);
	err = ext2fs_fallocate(fs, flags, fh->ino,
			       EXT2_INODE(&inode),
			       ~0ULL, start, end - start + 1);
	if (err && err != EXT2_ET_BLOCK_ALLOC_FAIL)
		return translate_error(fs, fh->ino, err);

	/* Update i_size */
	if (!(mode & FL_KEEP_SIZE_FLAG)) {
		if ((__u64) offset + len > fsize) {
			err = ext2fs_inode_size_set(fs,
						EXT2_INODE(&inode),
						offset + len);
			if (err)
				return translate_error(fs, fh->ino, err);
		}
	}

	err = update_mtime(fs, fh->ino, &inode);
	if (err)
		return err;

	err = fuse2fs_write_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	return err;
}

static errcode_t clean_block_middle(struct fuse2fs *ff, ext2_ino_t ino,
				    struct ext2_inode_large *inode,
				    off_t offset, off_t len, char **buf)
{
	ext2_filsys fs = ff->fs;
	blk64_t blk;
	off_t residue;
	int retflags;
	errcode_t err;

	residue = FUSE2FS_OFF_IN_FSB(ff, offset);
	if (residue == 0)
		return 0;

	if (!*buf) {
		err = ext2fs_get_mem(fs->blocksize, buf);
		if (err)
			return err;
	}

	err = ext2fs_bmap2(fs, ino, EXT2_INODE(inode), *buf, 0,
			   FUSE2FS_B_TO_FSBT(ff, offset), &retflags, &blk);
	if (err)
		return err;
	if (!blk || (retflags & BMAP_RET_UNINIT))
		return 0;

	err = io_channel_read_blk(fs->io, blk, 1, *buf);
	if (err)
		return err;

	memset(*buf + residue, 0, len);

	return io_channel_write_blk(fs->io, blk, 1, *buf);
}

static errcode_t clean_block_edge(struct fuse2fs *ff, ext2_ino_t ino,
				  struct ext2_inode_large *inode, off_t offset,
				  int clean_before, char **buf)
{
	ext2_filsys fs = ff->fs;
	blk64_t blk;
	int retflags;
	off_t residue;
	errcode_t err;

	residue = FUSE2FS_OFF_IN_FSB(ff, offset);
	if (residue == 0)
		return 0;

	if (!*buf) {
		err = ext2fs_get_mem(fs->blocksize, buf);
		if (err)
			return err;
	}

	err = ext2fs_bmap2(fs, ino, EXT2_INODE(inode), *buf, 0,
			   FUSE2FS_B_TO_FSBT(ff, offset), &retflags, &blk);
	if (err)
		return err;

	err = io_channel_read_blk(fs->io, blk, 1, *buf);
	if (err)
		return err;
	if (!blk || (retflags & BMAP_RET_UNINIT))
		return 0;

	if (clean_before)
		memset(*buf, 0, residue);
	else
		memset(*buf + residue, 0, fs->blocksize - residue);

	return io_channel_write_blk(fs->io, blk, 1, *buf);
}

static int punch_helper(struct fuse_file_info *fp, int mode, off_t offset,
			off_t len)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	struct fuse2fs_file_handle *fh =
		(struct fuse2fs_file_handle *)(uintptr_t)fp->fh;
	ext2_filsys fs;
	struct ext2_inode_large inode;
	blk64_t start, end;
	errcode_t err;
	char *buf = NULL;

	FUSE2FS_CHECK_CONTEXT(ff);
	fs = ff->fs;
	FUSE2FS_CHECK_MAGIC(fs, fh, FUSE2FS_FILE_MAGIC);
	dbg_printf(ff, "%s: offset=%jd len=%jd\n", __func__,
		   (intmax_t) offset, (intmax_t) len);

	/* kernel ext4 punch requires this flag to be set */
	if (!(mode & FL_KEEP_SIZE_FLAG))
		return -EINVAL;

	/* Punch out a bunch of blocks */
	start = FUSE2FS_B_TO_FSB(ff, offset);
	end = (offset + len - fs->blocksize) / fs->blocksize;
	dbg_printf(ff, "%s: ino=%d mode=0x%x start=%llu end=%llu\n", __func__,
		   fh->ino, mode, start, end);

	err = fuse2fs_read_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	/* Zero everything before the first block and after the last block */
	if (FUSE2FS_B_TO_FSBT(ff, offset) == FUSE2FS_B_TO_FSBT(ff, offset + len))
		err = clean_block_middle(ff, fh->ino, &inode, offset,
					 len, &buf);
	else {
		err = clean_block_edge(ff, fh->ino, &inode, offset, 0, &buf);
		if (!err)
			err = clean_block_edge(ff, fh->ino, &inode,
					       offset + len, 1, &buf);
	}
	if (buf)
		ext2fs_free_mem(&buf);
	if (err)
		return translate_error(fs, fh->ino, err);

	/* Unmap full blocks in the middle */
	if (start <= end) {
		err = ext2fs_punch(fs, fh->ino, EXT2_INODE(&inode),
				   NULL, start, end);
		if (err)
			return translate_error(fs, fh->ino, err);
	}

	err = update_mtime(fs, fh->ino, &inode);
	if (err)
		return err;

	err = fuse2fs_write_inode(fs, fh->ino, &inode);
	if (err)
		return translate_error(fs, fh->ino, err);

	return 0;
}

static int zero_helper(struct fuse_file_info *fp, int mode, off_t offset,
		       off_t len)
{
	int ret = punch_helper(fp, mode | FL_KEEP_SIZE_FLAG, offset, len);

	if (!ret)
		ret = fallocate_helper(fp, mode, offset, len);
	return ret;
}

static int op_fallocate(const char *path EXT2FS_ATTR((unused)), int mode,
			off_t offset, off_t len,
			struct fuse_file_info *fp)
{
	struct fuse_context *ctxt = fuse_get_context();
	struct fuse2fs *ff = (struct fuse2fs *)ctxt->private_data;
	ext2_filsys fs = ff->fs;
	int ret;

	/* Catch unknown flags */
	if (mode & ~(FL_ZERO_RANGE_FLAG | FL_PUNCH_HOLE_FLAG | FL_KEEP_SIZE_FLAG))
		return -EOPNOTSUPP;

	pthread_mutex_lock(&ff->bfl);
	if (!fs_writeable(fs)) {
		ret = -EROFS;
		goto out;
	}
	if (mode & FL_ZERO_RANGE_FLAG)
		ret = zero_helper(fp, mode, offset, len);
	else if (mode & FL_PUNCH_HOLE_FLAG)
		ret = punch_helper(fp, mode, offset, len);
	else
		ret = fallocate_helper(fp, mode, offset, len);
out:
	pthread_mutex_unlock(&ff->bfl);

	return ret;
}
# endif /* SUPPORT_FALLOCATE */
#endif /* FUSE 29 */

static struct fuse_operations fs_ops = {
	.init = op_init,
	.destroy = op_destroy,
	.getattr = op_getattr,
	.readlink = op_readlink,
	.mknod = op_mknod,
	.mkdir = op_mkdir,
	.unlink = op_unlink,
	.rmdir = op_rmdir,
	.symlink = op_symlink,
	.rename = op_rename,
	.link = op_link,
	.chmod = op_chmod,
	.chown = op_chown,
	.truncate = op_truncate,
	.open = op_open,
	.read = op_read,
	.write = op_write,
	.statfs = op_statfs,
	.release = op_release,
	.fsync = op_fsync,
	.setxattr = op_setxattr,
	.getxattr = op_getxattr,
	.listxattr = op_listxattr,
	.removexattr = op_removexattr,
	.opendir = op_open,
	.readdir = op_readdir,
	.releasedir = op_release,
	.fsyncdir = op_fsync,
	.access = op_access,
	.create = op_create,
#if FUSE_VERSION < FUSE_MAKE_VERSION(3, 0)
	.ftruncate = op_ftruncate,
	.fgetattr = op_fgetattr,
#endif
	.utimens = op_utimens,
#if (FUSE_VERSION >= FUSE_MAKE_VERSION(2, 9)) && (FUSE_VERSION < FUSE_MAKE_VERSION(3, 0))
# if defined(UTIME_NOW) || defined(UTIME_OMIT)
	.flag_utime_omit_ok = 1,
# endif
#endif
	.bmap = op_bmap,
#ifdef SUPERFLUOUS
	.lock = op_lock,
	.poll = op_poll,
#endif
#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 8)
	.ioctl = op_ioctl,
#if FUSE_VERSION < FUSE_MAKE_VERSION(3, 0)
	.flag_nullpath_ok = 1,
#endif
#endif
#if FUSE_VERSION >= FUSE_MAKE_VERSION(2, 9)
#if FUSE_VERSION < FUSE_MAKE_VERSION(3, 0)
	.flag_nopath = 1,
#endif
# ifdef SUPPORT_FALLOCATE
	.fallocate = op_fallocate,
# endif
#endif
};

static int get_random_bytes(void *p, size_t sz)
{
	int fd;
	ssize_t r;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		perror("/dev/urandom");
		return 0;
	}

	r = read(fd, p, sz);

	close(fd);
	return (size_t) r == sz;
}

enum {
	FUSE2FS_IGNORED,
	FUSE2FS_VERSION,
	FUSE2FS_HELP,
	FUSE2FS_HELPFULL,
	FUSE2FS_CACHE_SIZE,
};

#define FUSE2FS_OPT(t, p, v) { t, offsetof(struct fuse2fs, p), v }

static struct fuse_opt fuse2fs_opts[] = {
	FUSE2FS_OPT("ro",		ro,			1),
	FUSE2FS_OPT("rw",		ro,			0),
	FUSE2FS_OPT("errors=panic",	panic_on_error,		1),
	FUSE2FS_OPT("minixdf",		minixdf,		1),
	FUSE2FS_OPT("bsddf",		minixdf,		0),
	FUSE2FS_OPT("fakeroot",		fakeroot,		1),
	FUSE2FS_OPT("fuse2fs_debug",	debug,			1),
	FUSE2FS_OPT("no_default_opts",	no_default_opts,	1),
	FUSE2FS_OPT("norecovery",	norecovery,		1),
	FUSE2FS_OPT("noload",		norecovery,		1),
	FUSE2FS_OPT("offset=%lu",	offset,			0),
	FUSE2FS_OPT("kernel",		kernel,			1),
	FUSE2FS_OPT("directio",		directio,		1),
	FUSE2FS_OPT("acl",		acl,			1),
	FUSE2FS_OPT("noacl",		acl,			0),

	FUSE_OPT_KEY("user_xattr",	FUSE2FS_IGNORED),
	FUSE_OPT_KEY("noblock_validity", FUSE2FS_IGNORED),
	FUSE_OPT_KEY("nodelalloc",	FUSE2FS_IGNORED),
	FUSE_OPT_KEY("cache_size=%s",	FUSE2FS_CACHE_SIZE),
	FUSE2FS_OPT("lockfile=%s",	lockfile,		0),

	FUSE_OPT_KEY("-V",             FUSE2FS_VERSION),
	FUSE_OPT_KEY("--version",      FUSE2FS_VERSION),
	FUSE_OPT_KEY("-h",             FUSE2FS_HELP),
	FUSE_OPT_KEY("--help",         FUSE2FS_HELP),
	FUSE_OPT_KEY("--helpfull",     FUSE2FS_HELPFULL),
	FUSE_OPT_END
};


static int fuse2fs_opt_proc(void *data, const char *arg,
			    int key, struct fuse_args *outargs)
{
	struct fuse2fs *ff = data;

	switch (key) {
	case FUSE_OPT_KEY_NONOPT:
		if (!ff->device) {
			ff->device = strdup(arg);
			return 0;
		}
		return 1;
	case FUSE2FS_CACHE_SIZE:
		ff->cache_size = parse_num_blocks2(arg + 11, -1);
		if (ff->cache_size < 1 || ff->cache_size > INT32_MAX) {
			fprintf(stderr, "%s: %s\n", arg,
 _("cache size must be between 1 block and 2GB."));
			return -1;
		}

		/* do not pass through to libfuse */
		return 0;
	case FUSE2FS_IGNORED:
		return 0;
	case FUSE2FS_HELP:
	case FUSE2FS_HELPFULL:
		fprintf(stderr,
	"usage: %s device/image mountpoint [options]\n"
	"\n"
	"general options:\n"
	"    -o opt,[opt...]  mount options\n"
	"    -h   --help      print help\n"
	"    -V   --version   print version\n"
	"\n"
	"fuse2fs options:\n"
	"    -o errors=panic        dump core on error\n"
	"    -o minixdf             minix-style df\n"
	"    -o fakeroot            pretend to be root for permission checks\n"
	"    -o no_default_opts     do not include default fuse options\n"
	"    -o offset=<bytes>      similar to mount -o offset=<bytes>, mount the partition starting at <bytes>\n"
	"    -o norecovery          don't replay the journal\n"
	"    -o fuse2fs_debug       enable fuse2fs debugging\n"
	"    -o lockfile=<file>     file to show that fuse is still using the file system image\n"
	"    -o kernel              run this as if it were the kernel, which sets:\n"
	"                           allow_others,default_permissions,suid,dev\n"
	"    -o directio            use O_DIRECT to read and write the disk\n"
	"    -o cache_size=N[KMG]   use a disk cache of this size\n"
	"\n",
			outargs->argv[0]);
		if (key == FUSE2FS_HELPFULL) {
			fuse_opt_add_arg(outargs, "-h");
			fuse_main(outargs->argc, outargs->argv, &fs_ops, NULL);
		} else {
			fprintf(stderr, "Try --helpfull to get a list of "
				"all flags, including the FUSE options.\n");
		}
		exit(1);

	case FUSE2FS_VERSION:
		fprintf(stderr, "fuse2fs %s (%s)\n", E2FSPROGS_VERSION,
			E2FSPROGS_DATE);
		fuse_opt_add_arg(outargs, "--version");
		fuse_main(outargs->argc, outargs->argv, &fs_ops, NULL);
		exit(0);
	}
	return 1;
}

static const char *get_subtype(const char *argv0)
{
	size_t argvlen = strlen(argv0);

	if (argvlen < 4)
		goto out_default;

	if (argv0[argvlen - 4] == 'e' &&
	    argv0[argvlen - 3] == 'x' &&
	    argv0[argvlen - 2] == 't' &&
	    isdigit(argv0[argvlen - 1]))
		return &argv0[argvlen - 4];

out_default:
	return "ext4";
}

/* Figure out a reasonable default size for the disk cache */
static unsigned long long default_cache_size(void)
{
	long pages = 0, pagesize = 0;
	unsigned long long max_cache;
	unsigned long long ret = 32ULL << 20; /* 32 MB */

#ifdef _SC_PHYS_PAGES
	pages = sysconf(_SC_PHYS_PAGES);
#endif
#ifdef _SC_PAGESIZE
	pagesize = sysconf(_SC_PAGESIZE);
#endif
	if (pages > 0 && pagesize > 0) {
		max_cache = (unsigned long long)pagesize * pages / 20;

		if (max_cache > 0 && ret > max_cache)
			ret = max_cache;
	}
	return ret;
}

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse2fs fctx;
	errcode_t err;
	FILE *orig_stderr = stderr;
	char *logfile;
	char extra_args[BUFSIZ];
	int ret = 0;
	int flags = EXT2_FLAG_64BITS | EXT2_FLAG_THREADS | EXT2_FLAG_EXCLUSIVE |
		    EXT2_FLAG_RW;

	memset(&fctx, 0, sizeof(fctx));
	fctx.magic = FUSE2FS_MAGIC;

	fuse_opt_parse(&args, &fctx, fuse2fs_opts, fuse2fs_opt_proc);
	if (fctx.device == NULL) {
		fprintf(stderr, "Missing ext4 device/image\n");
		fprintf(stderr, "See '%s -h' for usage\n", argv[0]);
		exit(1);
	}

	/* /dev/sda -> sda for reporting */
	fctx.shortdev = strrchr(fctx.device, '/');
	if (fctx.shortdev)
		fctx.shortdev++;
	else
		fctx.shortdev = fctx.device;

#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
	set_com_err_gettext(gettext);
#endif
	add_error_table(&et_ext2_error_table);

	/* Set up error logging */
	logfile = getenv("FUSE2FS_LOGFILE");
	if (logfile) {
		FILE *fp = fopen(logfile, "a");
		if (!fp) {
			perror(logfile);
			goto out;
		}
		stderr = fp;
		stdout = fp;
	} else if (fctx.kernel) {
		/* in kernel mode, try to log errors to the kernel log */
		FILE *fp = fopen("/dev/ttyprintk", "a");
		if (fp) {
			stderr = fp;
			stdout = fp;
		}
	}

	/* Will we allow users to allocate every last block? */
	if (getenv("FUSE2FS_ALLOC_ALL_BLOCKS")) {
		log_printf(&fctx, "%s\n",
 _("Allowing users to allocate all blocks. This is dangerous!"));
		fctx.alloc_all_blocks = 1;
	}

	if (fctx.lockfile) {
		FILE *lockfile = fopen(fctx.lockfile, "w");
		char *resolved;

		if (!lockfile) {
			err = errno;
			err_printf(&fctx, "%s: %s: %s\n", fctx.lockfile,
				   _("opening lockfile failed"),
				   strerror(err));
			fctx.lockfile = NULL;
			ret |= 32;
			goto out;
		}
		fclose(lockfile);

		resolved = realpath(fctx.lockfile, NULL);
		if (!resolved) {
			err = errno;
			err_printf(&fctx, "%s: %s: %s\n", fctx.lockfile,
				   _("resolving lockfile failed"),
				   strerror(err));
			unlink(fctx.lockfile);
			fctx.lockfile = NULL;
			ret |= 32;
			goto out;
		}
		free(fctx.lockfile);
		fctx.lockfile = resolved;
	}

	/* Start up the fs (while we still can use stdout) */
	ret = 2;
	char options[50];
	sprintf(options, "offset=%lu", fctx.offset);
	if (fctx.directio)
		flags |= EXT2_FLAG_DIRECT_IO;
	err = ext2fs_open2(fctx.device, options, flags, 0, 0, unix_io_manager,
			   &global_fs);
	if (err) {
		err_printf(&fctx, "%s.\n", error_message(err));
		err_printf(&fctx, "%s\n", _("Please run e2fsck -fy."));
		goto out;
	}
	fctx.fs = global_fs;
	global_fs->priv_data = &fctx;
	fctx.blocklog = u_log2(fctx.fs->blocksize);
	fctx.blockmask = fctx.fs->blocksize - 1;

	if (!fctx.cache_size)
		fctx.cache_size = default_cache_size();
	if (fctx.cache_size) {
		char buf[55];

		snprintf(buf, sizeof(buf), "cache_blocks=%llu",
			 FUSE2FS_B_TO_FSBT(&fctx, fctx.cache_size));
		err = io_channel_set_options(global_fs->io, buf);
		if (err) {
			err_printf(&fctx, "%s %lluk: %s\n",
				   _("cannot set disk cache size to"),
				   fctx.cache_size >> 10,
				   error_message(err));
			goto out;
		}
	}

	ret = 3;

	if (ext2fs_has_feature_quota(global_fs->super)) {
		err_printf(&fctx, "%s", _("quotas not supported."));
		goto out;
	}
	if (ext2fs_has_feature_verity(global_fs->super)) {
		err_printf(&fctx, "%s", _("verity not supported."));
		goto out;
	}
	if (ext2fs_has_feature_encrypt(global_fs->super)) {
		err_printf(&fctx, "%s", _("encryption not supported."));
		goto out;
	}
	if (ext2fs_has_feature_casefold(global_fs->super)) {
		err_printf(&fctx, "%s", _("casefolding not supported."));
		goto out;
	}

	if (ext2fs_has_feature_shared_blocks(global_fs->super))
		fctx.ro = 1;

	if (ext2fs_has_feature_journal_needs_recovery(global_fs->super)) {
		if (fctx.norecovery) {
			log_printf(&fctx, "%s\n",
 _("Mounting read-only without recovering journal."));
			fctx.ro = 1;
			global_fs->flags &= ~EXT2_FLAG_RW;
		} else {
			log_printf(&fctx, "%s\n", _("Recovering journal."));
			err = ext2fs_run_ext3_journal(&global_fs);
			if (err) {
				err_printf(&fctx, "%s.\n", error_message(err));
				err_printf(&fctx, "%s\n",
						_("Please run e2fsck -fy."));
				goto out;
			}
			ext2fs_clear_feature_journal_needs_recovery(global_fs->super);
			ext2fs_mark_super_dirty(global_fs);
		}
	}

	if (global_fs->flags & EXT2_FLAG_RW) {
		if (ext2fs_has_feature_journal(global_fs->super))
			log_printf(&fctx, "%s",
 _("Warning: fuse2fs does not support using the journal.\n"
   "There may be file system corruption or data loss if\n"
   "the file system is not gracefully unmounted.\n"));
		err = ext2fs_read_inode_bitmap(global_fs);
		if (err) {
			translate_error(global_fs, 0, err);
			goto out;
		}
		err = ext2fs_read_block_bitmap(global_fs);
		if (err) {
			translate_error(global_fs, 0, err);
			goto out;
		}
	}

	if (!(global_fs->super->s_state & EXT2_VALID_FS))
		err_printf(&fctx, "%s\n",
 _("Warning: Mounting unchecked fs, running e2fsck is recommended."));
	if (global_fs->super->s_max_mnt_count > 0 &&
	    global_fs->super->s_mnt_count >= global_fs->super->s_max_mnt_count)
		err_printf(&fctx, "%s\n",
 _("Warning: Maximal mount count reached, running e2fsck is recommended."));
	if (global_fs->super->s_checkinterval > 0 &&
	    (time_t) (global_fs->super->s_lastcheck +
		      global_fs->super->s_checkinterval) <= time(0))
		err_printf(&fctx, "%s\n",
 _("Warning: Check time reached; running e2fsck is recommended."));
	if (global_fs->super->s_last_orphan)
		err_printf(&fctx, "%s\n",
 _("Orphans detected; running e2fsck is recommended."));

	if (global_fs->super->s_state & EXT2_ERROR_FS) {
		err_printf(&fctx, "%s\n",
 _("Errors detected; running e2fsck is required."));
		goto out;
	}

	/* Initialize generation counter */
	get_random_bytes(&fctx.next_generation, sizeof(unsigned int));

	/* Set up default fuse parameters */
	snprintf(extra_args, BUFSIZ, "-okernel_cache,subtype=%s,"
		 "fsname=%s,attr_timeout=0" FUSE_PLATFORM_OPTS,
		 get_subtype(argv[0]),
		 fctx.device);
	if (fctx.no_default_opts == 0)
		fuse_opt_add_arg(&args, extra_args);

	if (fctx.ro)
		fuse_opt_add_arg(&args, "-oro");

	if (fctx.fakeroot) {
#ifdef HAVE_MOUNT_NODEV
		fuse_opt_add_arg(&args,"-onodev");
#endif
#ifdef HAVE_MOUNT_NOSUID
		fuse_opt_add_arg(&args,"-onosuid");
#endif
	}

	if (fctx.kernel) {
		/*
		 * ACLs are always enforced when kernel mode is enabled, to
		 * match the kernel ext4 driver which always enables ACLs.
		 */
		fctx.acl = 1;
		fuse_opt_insert_arg(&args, 1,
 "-oallow_other,default_permissions,suid,dev");
	}

	if (fctx.debug) {
		int	i;

		printf("FUSE2FS (%s): fuse arguments:", fctx.shortdev);
		for (i = 0; i < args.argc; i++)
			printf(" '%s'", args.argv[i]);
		printf("\n");
		fflush(stdout);
	}

	pthread_mutex_init(&fctx.bfl, NULL);
	ret = fuse_main(args.argc, args.argv, &fs_ops, &fctx);
	pthread_mutex_destroy(&fctx.bfl);

	switch(ret) {
	case 0:
		/* success */
		ret = 0;
		break;
	case 1:
	case 2:
		/* invalid option or no mountpoint */
		ret = 1;
		break;
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		/* setup or mounting failed */
		ret = 32;
		break;
	default:
		/* fuse started up enough to call op_init */
		ret = 0;
		break;
	}
out:
	if (ret & 1) {
		fprintf(orig_stderr, "%s\n",
 _("Mount failed due to unrecognized options.  Check dmesg(1) for details."));
		fflush(orig_stderr);
	}
	if (ret & 32) {
		fprintf(orig_stderr, "%s\n",
 _("Mount failed while opening filesystem.  Check dmesg(1) for details."));
		fflush(orig_stderr);
	}
	if (global_fs) {
		err = ext2fs_close(global_fs);
		if (err)
			com_err(argv[0], err, "while closing fs");
		global_fs = NULL;
	}
	if (fctx.lockfile) {
		if (unlink(fctx.lockfile)) {
			err = errno;
			err_printf(&fctx, "%s: %s: %s\n", fctx.lockfile,
				   _("removing lockfile failed"),
				   strerror(err));
		}
		free(fctx.lockfile);
	}
	if (fctx.device)
		free(fctx.device);
	fuse_opt_free_args(&args);
	return ret;
}

static int __translate_error(ext2_filsys fs, ext2_ino_t ino, errcode_t err,
			     const char *file, int line)
{
	struct timespec now;
	int ret = err;
	struct fuse2fs *ff = fs->priv_data;
	int is_err = 0;

	/* Translate ext2 error to unix error code */
	switch (err) {
	case 0:
		break;
	case EXT2_ET_NO_MEMORY:
	case EXT2_ET_TDB_ERR_OOM:
		ret = -ENOMEM;
		break;
	case EXT2_ET_INVALID_ARGUMENT:
	case EXT2_ET_LLSEEK_FAILED:
		ret = -EINVAL;
		break;
	case EXT2_ET_NO_DIRECTORY:
		ret = -ENOTDIR;
		break;
	case EXT2_ET_FILE_NOT_FOUND:
		ret = -ENOENT;
		break;
	case EXT2_ET_DIR_NO_SPACE:
		is_err = 1;
		/* fallthrough */
	case EXT2_ET_TOOSMALL:
	case EXT2_ET_BLOCK_ALLOC_FAIL:
	case EXT2_ET_INODE_ALLOC_FAIL:
	case EXT2_ET_EA_NO_SPACE:
		ret = -ENOSPC;
		break;
	case EXT2_ET_SYMLINK_LOOP:
		ret = -EMLINK;
		break;
	case EXT2_ET_FILE_TOO_BIG:
		ret = -EFBIG;
		break;
	case EXT2_ET_TDB_ERR_EXISTS:
	case EXT2_ET_FILE_EXISTS:
		ret = -EEXIST;
		break;
	case EXT2_ET_MMP_FAILED:
	case EXT2_ET_MMP_FSCK_ON:
		ret = -EBUSY;
		break;
	case EXT2_ET_EA_KEY_NOT_FOUND:
		ret = -ENODATA;
		break;
	/* Sometimes fuse returns a garbage file handle pointer to us... */
	case EXT2_ET_MAGIC_EXT2_FILE:
		ret = -EFAULT;
		break;
	case EXT2_ET_UNIMPLEMENTED:
		ret = -EOPNOTSUPP;
		break;
	case EXT2_ET_MAGIC_EXT2FS_FILSYS:
	case EXT2_ET_MAGIC_BADBLOCKS_LIST:
	case EXT2_ET_MAGIC_BADBLOCKS_ITERATE:
	case EXT2_ET_MAGIC_INODE_SCAN:
	case EXT2_ET_MAGIC_IO_CHANNEL:
	case EXT2_ET_MAGIC_UNIX_IO_CHANNEL:
	case EXT2_ET_MAGIC_IO_MANAGER:
	case EXT2_ET_MAGIC_BLOCK_BITMAP:
	case EXT2_ET_MAGIC_INODE_BITMAP:
	case EXT2_ET_MAGIC_GENERIC_BITMAP:
	case EXT2_ET_MAGIC_TEST_IO_CHANNEL:
	case EXT2_ET_MAGIC_DBLIST:
	case EXT2_ET_MAGIC_ICOUNT:
	case EXT2_ET_MAGIC_PQ_IO_CHANNEL:
	case EXT2_ET_MAGIC_E2IMAGE:
	case EXT2_ET_MAGIC_INODE_IO_CHANNEL:
	case EXT2_ET_MAGIC_EXTENT_HANDLE:
	case EXT2_ET_BAD_MAGIC:
	case EXT2_ET_MAGIC_EXTENT_PATH:
	case EXT2_ET_MAGIC_GENERIC_BITMAP64:
	case EXT2_ET_MAGIC_BLOCK_BITMAP64:
	case EXT2_ET_MAGIC_INODE_BITMAP64:
	case EXT2_ET_MAGIC_RESERVED_13:
	case EXT2_ET_MAGIC_RESERVED_14:
	case EXT2_ET_MAGIC_RESERVED_15:
	case EXT2_ET_MAGIC_RESERVED_16:
	case EXT2_ET_MAGIC_RESERVED_17:
	case EXT2_ET_MAGIC_RESERVED_18:
	case EXT2_ET_MAGIC_RESERVED_19:
	case EXT2_ET_MMP_MAGIC_INVALID:
	case EXT2_ET_MAGIC_EA_HANDLE:
	case EXT2_ET_DIR_CORRUPTED:
	case EXT2_ET_CORRUPT_SUPERBLOCK:
	case EXT2_ET_RESIZE_INODE_CORRUPT:
	case EXT2_ET_TDB_ERR_CORRUPT:
	case EXT2_ET_UNDO_FILE_CORRUPT:
	case EXT2_ET_FILESYSTEM_CORRUPTED:
	case EXT2_ET_CORRUPT_JOURNAL_SB:
	case EXT2_ET_INODE_CORRUPTED:
	case EXT2_ET_EA_INODE_CORRUPTED:
		/* same errno that linux uses */
		is_err = 1;
		ret = -EUCLEAN;
		break;
	default:
		is_err = 1;
		ret = (err < 256) ? -err : -EIO;
		break;
	}

	if (!is_err)
		return ret;

	if (ino)
		err_printf(ff, "%s (inode #%d) at %s:%d.\n",
			error_message(err), ino, file, line);
	else
		err_printf(ff, "%s at %s:%d.\n",
			error_message(err), file, line);

	/* Make a note in the error log */
	get_now(&now);
	ext2fs_set_tstamp(fs->super, s_last_error_time, now.tv_sec);
	fs->super->s_last_error_ino = ino;
	fs->super->s_last_error_line = line;
	fs->super->s_last_error_block = err; /* Yeah... */
	strncpy((char *)fs->super->s_last_error_func, file,
		sizeof(fs->super->s_last_error_func));
	if (ext2fs_get_tstamp(fs->super, s_first_error_time) == 0) {
		ext2fs_set_tstamp(fs->super, s_first_error_time, now.tv_sec);
		fs->super->s_first_error_ino = ino;
		fs->super->s_first_error_line = line;
		fs->super->s_first_error_block = err;
		strncpy((char *)fs->super->s_first_error_func, file,
			sizeof(fs->super->s_first_error_func));
	}

	fs->super->s_error_count++;
	ext2fs_mark_super_dirty(fs);
	ext2fs_flush(fs);
	if (ff->panic_on_error)
		abort();

	return ret;
}
