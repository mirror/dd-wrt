/* SPDX-License-Identifier: LGPL-2.1 */
/*
 * Copyright (c) 1995-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef __XFS_FS_COMPAT_H__
#define __XFS_FS_COMPAT_H__

/*
 * Backwards-compatible definitions and structures for public kernel interfaces
 */

/*
 * Flags for the bs_xflags/fsx_xflags field in XFS_IOC_FS[GS]ETXATTR[A]
 * These are for backwards compatibility only. New code should
 * use the kernel [4.5 onwards] defined FS_XFLAG_* definitions directly.
 */
#define	XFS_XFLAG_REALTIME	FS_XFLAG_REALTIME
#define	XFS_XFLAG_PREALLOC	FS_XFLAG_PREALLOC
#define	XFS_XFLAG_IMMUTABLE	FS_XFLAG_IMMUTABLE
#define	XFS_XFLAG_APPEND	FS_XFLAG_APPEND
#define	XFS_XFLAG_SYNC		FS_XFLAG_SYNC
#define	XFS_XFLAG_NOATIME	FS_XFLAG_NOATIME
#define	XFS_XFLAG_NODUMP	FS_XFLAG_NODUMP
#define	XFS_XFLAG_RTINHERIT	FS_XFLAG_RTINHERIT
#define	XFS_XFLAG_PROJINHERIT	FS_XFLAG_PROJINHERIT
#define	XFS_XFLAG_NOSYMLINKS	FS_XFLAG_NOSYMLINKS
#define	XFS_XFLAG_EXTSIZE	FS_XFLAG_EXTSIZE
#define	XFS_XFLAG_EXTSZINHERIT	FS_XFLAG_EXTSZINHERIT
#define	XFS_XFLAG_NODEFRAG	FS_XFLAG_NODEFRAG
#define	XFS_XFLAG_FILESTREAM	FS_XFLAG_FILESTREAM
#define	XFS_XFLAG_HASATTR	FS_XFLAG_HASATTR

/*
 * Don't use this.
 * Use struct file_clone_range
 */
struct xfs_clone_args {
	__s64 src_fd;
	__u64 src_offset;
	__u64 src_length;
	__u64 dest_offset;
};

/*
 * Don't use these.
 * Use FILE_DEDUPE_RANGE_SAME / FILE_DEDUPE_RANGE_DIFFERS
 */
#define XFS_EXTENT_DATA_SAME	0
#define XFS_EXTENT_DATA_DIFFERS	1

/* Don't use this. Use file_dedupe_range_info */
struct xfs_extent_data_info {
	__s64 fd;		/* in - destination file */
	__u64 logical_offset;	/* in - start of extent in destination */
	__u64 bytes_deduped;	/* out - total # of bytes we were able
				 * to dedupe from this file */
	/* status of this dedupe operation:
	 * < 0 for error
	 * == XFS_EXTENT_DATA_SAME if dedupe succeeds
	 * == XFS_EXTENT_DATA_DIFFERS if data differs
	 */
	__s32 status;		/* out - see above description */
	__u32 reserved;
};

/*
 * Don't use this.
 * Use struct file_dedupe_range
 */
struct xfs_extent_data {
	__u64 logical_offset;	/* in - start of extent in source */
	__u64 length;		/* in - length of extent */
	__u16 dest_count;	/* in - total elements in info array */
	__u16 reserved1;
	__u32 reserved2;
	struct xfs_extent_data_info info[0];
};

/*
 * Don't use these.
 * Use FICLONE/FICLONERANGE/FIDEDUPERANGE
 */
#define XFS_IOC_CLONE		 _IOW (0x94, 9, int)
#define XFS_IOC_CLONE_RANGE	 _IOW (0x94, 13, struct xfs_clone_args)
#define XFS_IOC_FILE_EXTENT_SAME _IOWR(0x94, 54, struct xfs_extent_data)

/* 64-bit seconds counter that works independently of the C library time_t. */
typedef long long int time64_t;

struct timespec64 {
	time64_t	tv_sec;			/* seconds */
	long		tv_nsec;		/* nanoseconds */
};

#define U32_MAX		((uint32_t)~0U)
#define S32_MAX		((int32_t)(U32_MAX >> 1))
#define S32_MIN		((int32_t)(-S32_MAX - 1))

#endif	/* __XFS_FS_COMPAT_H__ */
