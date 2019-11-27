// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (C) 2012-2013 Samsung Electronics Co., Ltd.
 */

#ifndef _EXFAT_H
#define _EXFAT_H

#include <linux/buffer_head.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/nls.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/ratelimit.h>
#include <linux/version.h>
#include <linux/kobject.h>
#include "api.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
#include <linux/iversion.h>
#define INC_IVERSION(x)		(inode_inc_iversion(x))
#define GET_IVERSION(x)		(inode_peek_iversion_raw(x))
#define SET_IVERSION(x,y)	(inode_set_iversion(x, y))
#else
#define INC_IVERSION(x)		(x->i_version++)
#define GET_IVERSION(x)		(x->i_version)
#define SET_IVERSION(x,y)	(x->i_version = y)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
#define timespec_compat	timespec64
#define KTIME_GET_REAL_TS ktime_get_real_ts64
#else
#define timespec_compat	timespec
#define KTIME_GET_REAL_TS ktime_get_real_ts
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
#define EXFAT_IS_SB_RDONLY(sb)	((sb)->s_flags & MS_RDONLY)
#else
#define EXFAT_IS_SB_RDONLY(sb)	((sb)->s_flags & SB_RDONLY)
#endif

/*
 * exfat error flags
 */
#define EXFAT_ERRORS_CONT	(1)    /* ignore error and continue */
#define EXFAT_ERRORS_PANIC	(2)    /* panic on error */
#define EXFAT_ERRORS_RO		(3)    /* remount r/o on error */

/*
 * exfat allocator destination for smart allocation
 */
#define ALLOC_NOWHERE		(0)
#define ALLOC_COLD		(1)
#define ALLOC_HOT		(16)
#define ALLOC_COLD_ALIGNED	(1)
#define ALLOC_COLD_PACKING	(2)
#define ALLOC_COLD_SEQ		(4)

/*
 * exfat nls lossy flag
 */
#define NLS_NAME_NO_LOSSY	(0x00) /* no lossy */
#define NLS_NAME_LOSSY		(0x01) /* just detected incorrect filename(s) */
#define NLS_NAME_OVERLEN	(0x02) /* the length is over than its limit */

/*
 * exfat common MACRO
 */
#define CLUSTER_16(x)	((u16)((x) & 0xFFFFU))
#define CLUSTER_32(x)	((u32)((x) & 0xFFFFFFFFU))
#define CLUS_EOF	CLUSTER_32(~0)
#define CLUS_BAD	(0xFFFFFFF7U)
#define CLUS_FREE	(0)
#define CLUS_BASE	(2)
#define IS_CLUS_EOF(x)	((x) == CLUS_EOF)
#define IS_CLUS_BAD(x)	((x) == CLUS_BAD)
#define IS_CLUS_FREE(x)	((x) == CLUS_FREE)
#define IS_LAST_SECT_IN_CLUS(fsi, sec)				\
	((((sec) - (fsi)->data_start_sector + 1)		\
	& ((1 << (fsi)->sect_per_clus_bits) - 1)) == 0)

#define CLUS_TO_SECT(fsi, x)	\
	((((unsigned long long)(x) - CLUS_BASE) << (fsi)->sect_per_clus_bits) + (fsi)->data_start_sector)

#define SECT_TO_CLUS(fsi, sec)	\
	((u32)((((sec) - (fsi)->data_start_sector) >> (fsi)->sect_per_clus_bits) + CLUS_BASE))

/*
 * exfat mount in-memory data
 */
struct exfat_mount_options {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
	kuid_t fs_uid;
	kgid_t fs_gid;
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0) */
	uid_t fs_uid;
	gid_t fs_gid;
#endif
	unsigned short fs_fmask;
	unsigned short fs_dmask;
	unsigned short allow_utime; /* permission for setting the [am]time */
	unsigned short codepage;    /* codepage for shortname conversions */
	char *iocharset;            /* charset for filename input/display */
	unsigned char quiet;        /* fake return success on setattr(e.g. chmods/chowns) */

	unsigned char utf8;
	unsigned char casesensitive;
	unsigned char tz_utc;
	unsigned char symlink;      /* support symlink operation */
	unsigned char errors;       /* on error: continue, panic, remount-ro */
	unsigned char discard;      /* flag on if -o dicard specified and device support discard() */
	unsigned char delayed_meta; /* delay flushing dirty metadata */
};

#define EXFAT_HASH_BITS    8
#define EXFAT_HASH_SIZE    (1UL << EXFAT_HASH_BITS)

/*
 * EXFAT file system superblock in-memory data
 */
struct exfat_sb_info {
	FS_INFO_T fsi;	/* private filesystem info */

	struct mutex s_vlock;   /* volume lock */
	int use_vmalloc;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0)
	int s_dirt;
	struct mutex s_lock;    /* superblock lock */
	int write_super_queued;			/* Write_super work is pending? */
	struct delayed_work write_super_work;   /* Work_queue data structrue for write_super() */
	spinlock_t work_lock;			/* Lock for WQ */
#endif
	struct super_block *host_sb;		/* sb pointer */
	struct exfat_mount_options options;
	struct nls_table *nls_disk; /* Codepage used on disk */
	struct nls_table *nls_io;   /* Charset used for input and display */
	struct ratelimit_state ratelimit;

	spinlock_t inode_hash_lock;
	struct hlist_head inode_hashtable[EXFAT_HASH_SIZE];
	struct kobject sb_kobj;

	atomic_t stat_n_pages_queued;	/* # of pages in the request queue (approx.) */
};

/*
 * EXFAT file system inode in-memory data
 */
struct exfat_inode_info {
	FILE_ID_T fid;
	char  *target;
	/* NOTE: i_size_ondisk is 64bits, so must hold ->inode_lock to access */
	loff_t i_size_ondisk;         /* physically allocated size */
	loff_t i_size_aligned;          /* block-aligned i_size (used in cont_write_begin) */
	loff_t i_pos;               /* on-disk position of directory entry or 0 */
	struct hlist_node i_hash_fat;    /* hash by i_location */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
	struct rw_semaphore truncate_lock; /* protect bmap against truncate */
#endif
	struct inode vfs_inode;
};

/*
 * FIXME : needs on-disk-slot in-memory data
 */

static inline struct exfat_sb_info *EXFAT_SB(struct super_block *sb)
{
	return (struct exfat_sb_info *)sb->s_fs_info;
}

static inline struct exfat_inode_info *EXFAT_I(struct inode *inode)
{
	return container_of(inode, struct exfat_inode_info, vfs_inode);
}

/*
 * If ->i_mode can't hold S_IWUGO (i.e. ATTR_RO), we use ->i_attrs to
 * save ATTR_RO instead of ->i_mode.
 *
 * If it's directory and !sbi->options.rodir, ATTR_RO isn't read-only
 * bit, it's just used as flag for app.
 */
static inline int exfat_mode_can_hold_ro(struct inode *inode)
{
	struct exfat_sb_info *sbi = EXFAT_SB(inode->i_sb);

	if (S_ISDIR(inode->i_mode))
		return 0;

	if ((~sbi->options.fs_fmask) & S_IWUGO)
		return 1;
	return 0;
}

/*
 * FIXME : needs to check symlink option.
 */
/* Convert attribute bits and a mask to the UNIX mode. */
static inline mode_t exfat_make_mode(struct exfat_sb_info *sbi,
					u32 attr, mode_t mode)
{
	if ((attr & ATTR_READONLY) && !(attr & ATTR_SUBDIR))
		mode &= ~S_IWUGO;

	if (attr & ATTR_SUBDIR)
		return (mode & ~sbi->options.fs_dmask) | S_IFDIR;
	else if (attr & ATTR_SYMLINK)
		return (mode & ~sbi->options.fs_dmask) | S_IFLNK;
	else
		return (mode & ~sbi->options.fs_fmask) | S_IFREG;
}

/* Return the FAT attribute byte for this inode */
static inline u32 exfat_make_attr(struct inode *inode)
{
	u32 attrs = EXFAT_I(inode)->fid.attr;

	if (S_ISDIR(inode->i_mode))
		attrs |= ATTR_SUBDIR;
	if (exfat_mode_can_hold_ro(inode) && !(inode->i_mode & S_IWUGO))
		attrs |= ATTR_READONLY;
	return attrs;
}

static inline void exfat_save_attr(struct inode *inode, u32 attr)
{
	if (exfat_mode_can_hold_ro(inode))
		EXFAT_I(inode)->fid.attr = attr & ATTR_RWMASK;
	else
		EXFAT_I(inode)->fid.attr = attr & (ATTR_RWMASK | ATTR_READONLY);
}

/* exfat/nls.c */
/* NLS management function */
s32  exfat_nls_cmp_uniname(struct super_block *sb, u16 *a, u16 *b);
s32  exfat_nls_sfn_to_uni16s(struct super_block *sb, DOS_NAME_T *p_dosname, UNI_NAME_T *p_uniname);
s32  exfat_nls_uni16s_to_vfsname(struct super_block *sb, UNI_NAME_T *uniname, u8 *p_cstring, s32 len);
s32  exfat_nls_vfsname_to_uni16s(struct super_block *sb, const u8 *p_cstring,
			const s32 len, UNI_NAME_T *uniname, s32 *p_lossy);

/* exfat/xattr.c */
#ifdef CONFIG_EXFAT_VIRTUAL_XATTR
void setup_exfat_xattr_handler(struct super_block *sb);
extern int exfat_setxattr(struct dentry *dentry, const char *name, const void *value, size_t size, int flags);
extern ssize_t exfat_getxattr(struct dentry *dentry, const char *name, void *value, size_t size);
extern ssize_t exfat_listxattr(struct dentry *dentry, char *list, size_t size);
extern int exfat_removexattr(struct dentry *dentry, const char *name);
#else
static inline void setup_exfat_xattr_handler(struct super_block *sb) {};
#endif

/* exfat/misc.c */
#ifdef CONFIG_EXFAT_UEVENT
extern int exfat_uevent_init(struct kset *exfat_kset);
extern void exfat_uevent_uninit(void);
extern void exfat_uevent_ro_remount(struct super_block *sb);
#else
static inline int exfat_uevent_init(struct kset *exfat_kset)
{
	return 0;
}
static inline void exfat_uevent_uninit(void) {};
static inline void exfat_uevent_ro_remount(struct super_block *sb) {};
#endif
extern void
__exfat_fs_error(struct super_block *sb, int report, const char *fmt, ...)
	__printf(3, 4) __cold;
#define exfat_fs_error(sb, fmt, args...)          \
	__exfat_fs_error(sb, 1, fmt, ## args)
#define exfat_fs_error_ratelimit(sb, fmt, args...) \
	__exfat_fs_error(sb, __ratelimit(&EXFAT_SB(sb)->ratelimit), fmt, ## args)
extern void
__exfat_msg(struct super_block *sb, const char *lv, int st, const char *fmt, ...)
	__printf(4, 5) __cold;
#define exfat_msg(sb, lv, fmt, args...)          \
	__exfat_msg(sb, lv, 0, fmt, ## args)
#define exfat_log_msg(sb, lv, fmt, args...)          \
	__exfat_msg(sb, lv, 1, fmt, ## args)
extern void exfat_log_version(void);
extern void exfat_time_fat2unix(struct exfat_sb_info *sbi, struct timespec_compat *ts,
				DATE_TIME_T *tp);
extern void exfat_time_unix2fat(struct exfat_sb_info *sbi, struct timespec_compat *ts,
				DATE_TIME_T *tp);
extern TIMESTAMP_T *exfat_tm_now(struct exfat_sb_info *sbi, TIMESTAMP_T *tm);

#ifdef CONFIG_EXFAT_DEBUG

#ifdef CONFIG_EXFAT_DBG_BUGON
#define exfat_debug_bug_on(expr)        BUG_ON(expr)
#else
#define exfat_debug_bug_on(expr)
#endif

#ifdef CONFIG_EXFAT_DBG_WARNON
#define exfat_debug_warn_on(expr)        WARN_ON(expr)
#else
#define exfat_debug_warn_on(expr)
#endif

#else /* CONFIG_EXFAT_DEBUG */

#define exfat_debug_bug_on(expr)
#define exfat_debug_warn_on(expr)

#endif /* CONFIG_EXFAT_DEBUG */

#define	EXFAT_MSG_LV_NONE	(0x00000000)
#define EXFAT_MSG_LV_ERR	(0x00000001)
#define EXFAT_MSG_LV_INFO	(0x00000002)
#define EXFAT_MSG_LV_DBG	(0x00000003)
#define EXFAT_MSG_LV_MORE	(0x00000004)
#define EXFAT_MSG_LV_TRACE	(0x00000005)
#define EXFAT_MSG_LV_ALL	(0x00000006)

#define EXFAT_MSG_LEVEL		EXFAT_MSG_LV_INFO

#define EXFAT_TAG_NAME	"EXFAT"
#define __S(x) #x
#define _S(x) __S(x)

extern void __exfat_dmsg(int level, const char *fmt, ...) __printf(2, 3) __cold;

#define EXFAT_EMSG_T(level, ...)	\
	__exfat_dmsg(level, KERN_ERR "[" EXFAT_TAG_NAME "] [" _S(__FILE__) "(" _S(__LINE__) ")] " __VA_ARGS__)
#define EXFAT_DMSG_T(level, ...)	\
	__exfat_dmsg(level, KERN_INFO "[" EXFAT_TAG_NAME "] " __VA_ARGS__)

#define EXFAT_EMSG(...) EXFAT_EMSG_T(EXFAT_MSG_LV_ERR, __VA_ARGS__)
#define EXFAT_IMSG(...) EXFAT_DMSG_T(EXFAT_MSG_LV_INFO, __VA_ARGS__)
#define EXFAT_DMSG(...) EXFAT_DMSG_T(EXFAT_MSG_LV_DBG, __VA_ARGS__)
#define EXFAT_MMSG(...) EXFAT_DMSG_T(EXFAT_MSG_LV_MORE, __VA_ARGS__)

#define EMSG(...)
#define IMSG(...)
#define DMSG(...)
#define MMSG(...)

#define EMSG_VAR(exp)
#define IMSG_VAR(exp)
#define DMSG_VAR(exp)
#define MMSG_VAR(exp)

#ifdef CONFIG_EXFAT_DBG_MSG


#if (EXFAT_MSG_LEVEL >= EXFAT_MSG_LV_ERR)
#undef EMSG
#undef EMSG_VAR
#define EMSG(...)	EXFAT_EMSG(__VA_ARGS__)
#define EMSG_VAR(exp)	exp
#endif

#if (EXFAT_MSG_LEVEL >= EXFAT_MSG_LV_INFO)
#undef IMSG
#undef IMSG_VAR
#define IMSG(...)	EXFAT_IMSG(__VA_ARGS__)
#define IMSG_VAR(exp)	exp
#endif

#if (EXFAT_MSG_LEVEL >= EXFAT_MSG_LV_DBG)
#undef DMSG
#undef DMSG_VAR
#define DMSG(...)	EXFAT_DMSG(__VA_ARGS__)
#define DMSG_VAR(exp)	exp
#endif

#if (EXFAT_MSG_LEVEL >= EXFAT_MSG_LV_MORE)
#undef MMSG
#undef MMSG_VAR
#define MMSG(...)	EXFAT_MMSG(__VA_ARGS__)
#define MMSG_VAR(exp)	exp
#endif

#endif /* CONFIG_EXFAT_DBG_MSG */

#define ASSERT(expr)	{					\
	if (!(expr)) {						\
		pr_err("exFAT: Assertion failed! %s\n", #expr);	\
		BUG_ON(1);					\
	}							\
}

#endif /* !_EXFAT_H */

