/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Defines for volume structures in NTFS Linux kernel driver.
 * Part of the Linux-NTFS project.
 *
 * Copyright (c) 2001-2006 Anton Altaparmakov
 * Copyright (c) 2002 Richard Russon
 * Copyright (c) 2025 LG Electronics Co., Ltd.
 */

#ifndef _LINUX_NTFS_VOLUME_H
#define _LINUX_NTFS_VOLUME_H

#include <linux/rwsem.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/uidgid.h>
#include <linux/workqueue.h>
#include <linux/errseq.h>

#include "layout.h"

#define NTFS_VOL_UID	BIT(1)
#define NTFS_VOL_GID	BIT(2)

/*
 * The NTFS in memory super block structure.
 */
struct ntfs_volume {
	/* Device specifics. */
	struct super_block *sb;		/* Pointer back to the super_block. */
	s64 nr_blocks;			/*
					 * Number of sb->s_blocksize bytes
					 * sized blocks on the device.
					 */
	/* Configuration provided by user at mount time. */
	unsigned long flags;		/* Miscellaneous flags, see below. */
	kuid_t uid;			/* uid that files will be mounted as. */
	kgid_t gid;			/* gid that files will be mounted as. */
	umode_t fmask;			/* The mask for file permissions. */
	umode_t dmask;			/* The mask for directory permissions. */
	u8 mft_zone_multiplier;		/* Initial mft zone multiplier. */
	u8 on_errors;			/* What to do on filesystem errors. */
	errseq_t wb_err;
	/* NTFS bootsector provided information. */
	u16 sector_size;		/* in bytes */
	u8 sector_size_bits;		/* log2(sector_size) */
	u32 cluster_size;		/* in bytes */
	u32 cluster_size_mask;		/* cluster_size - 1 */
	u8 cluster_size_bits;		/* log2(cluster_size) */
	u32 mft_record_size;		/* in bytes */
	u32 mft_record_size_mask;	/* mft_record_size - 1 */
	u8 mft_record_size_bits;	/* log2(mft_record_size) */
	u32 index_record_size;		/* in bytes */
	u32 index_record_size_mask;	/* index_record_size - 1 */
	u8 index_record_size_bits;	/* log2(index_record_size) */
	s64 nr_clusters;		/*
					 * Volume size in clusters == number of
					 * bits in lcn bitmap.
					 */
	s64 mft_lcn;			/* Cluster location of mft data. */
	s64 mftmirr_lcn;		/* Cluster location of copy of mft. */
	u64 serial_no;			/* The volume serial number. */
	/* Mount specific NTFS information. */
	u32 upcase_len;			/* Number of entries in upcase[]. */
	__le16 *upcase;		/* The upcase table. */

	s32 attrdef_size;		/* Size of the attribute definition table in bytes. */
	struct attr_def *attrdef;	/*
					 * Table of attribute definitions.
					 * Obtained from FILE_AttrDef.
					 */

	/* Variables used by the cluster and mft allocators. */
	s64 mft_data_pos;		/*
					 * Mft record number at which to
					 * allocate the next mft record.
					 */
	s64 mft_zone_start;		/* First cluster of the mft zone. */
	s64 mft_zone_end;		/* First cluster beyond the mft zone. */
	s64 mft_zone_pos;		/* Current position in the mft zone. */
	s64 data1_zone_pos;		/* Current position in the first data zone. */
	s64 data2_zone_pos;		/* Current position in the second data zone. */

	struct inode *mft_ino;		/* The VFS inode of $MFT. */

	struct inode *mftbmp_ino;	/* Attribute inode for $MFT/$BITMAP. */
	struct rw_semaphore mftbmp_lock; /*
					  *  Lock for serializing accesses to the
					  * mft record bitmap ($MFT/$BITMAP).
					  */
	struct inode *mftmirr_ino;	/* The VFS inode of $MFTMirr. */
	int mftmirr_size;		/* Size of mft mirror in mft records. */

	struct inode *logfile_ino;	/* The VFS inode of LogFile. */

	struct inode *lcnbmp_ino;	/* The VFS inode of $Bitmap. */
	struct rw_semaphore lcnbmp_lock; /*
					  * Lock for serializing accesses to the
					  * cluster bitmap ($Bitmap/$DATA).
					  */

	struct inode *vol_ino;		/* The VFS inode of $Volume. */
	__le16 vol_flags;			/* Volume flags. */
	u8 major_ver;			/* Ntfs major version of volume. */
	u8 minor_ver;			/* Ntfs minor version of volume. */
	unsigned char *volume_label;

	struct inode *root_ino;		/* The VFS inode of the root directory. */
	struct inode *secure_ino;	/*
					 * The VFS inode of $Secure (NTFS3.0+
					 * only, otherwise NULL).
					 */
	struct inode *extend_ino;	/*
					 * The VFS inode of $Extend (NTFS3.0+
					 * only, otherwise NULL).
					 */
	/* $Quota stuff is NTFS3.0+ specific.  Unused/NULL otherwise. */
	struct inode *quota_ino;	/* The VFS inode of $Quota. */
	struct inode *quota_q_ino;	/* Attribute inode for $Quota/$Q. */
	struct nls_table *nls_map;
	bool nls_utf8;
	wait_queue_head_t free_waitq;

	atomic64_t free_clusters;	/* Track the number of free clusters */
	atomic64_t free_mft_records;		/* Track the free mft records */
	atomic64_t dirty_clusters;
	u8 sparse_compression_unit;
	unsigned int *lcn_empty_bits_per_page;
	struct work_struct precalc_work;
	loff_t preallocated_size;
};

/*
 * Defined bits for the flags field in the ntfs_volume structure.
 */
enum {
	NV_Errors,		/* 1: Volume has errors, prevent remount rw. */
	NV_ShowSystemFiles,	/* 1: Return system files in ntfs_readdir(). */
	NV_CaseSensitive,	/*
				 * 1: Treat file names as case sensitive and
				 *    create filenames in the POSIX namespace.
				 *    Otherwise be case insensitive but still
				 *    create file names in POSIX namespace.
				 */
	NV_LogFileEmpty,	/* 1: LogFile journal is empty. */
	NV_QuotaOutOfDate,	/* 1: Quota is out of date. */
	NV_UsnJrnlStamped,	/* 1: UsnJrnl has been stamped. */
	NV_ReadOnly,
	NV_Compression,
	NV_FreeClusterKnown,
	NV_Shutdown,
	NV_SysImmutable,	/* 1: Protect system files from deletion. */
	NV_ShowHiddenFiles,	/* 1: Return hidden files in ntfs_readdir(). */
	NV_HideDotFiles,
	NV_CheckWindowsNames,
	NV_Discard,
	NV_DisableSparse,
};

/*
 * Macro tricks to expand the NVolFoo(), NVolSetFoo(), and NVolClearFoo()
 * functions.
 */
#define DEFINE_NVOL_BIT_OPS(flag)					\
static inline int NVol##flag(struct ntfs_volume *vol)		\
{								\
	return test_bit(NV_##flag, &(vol)->flags);		\
}								\
static inline void NVolSet##flag(struct ntfs_volume *vol)	\
{								\
	set_bit(NV_##flag, &(vol)->flags);			\
}								\
static inline void NVolClear##flag(struct ntfs_volume *vol)	\
{								\
	clear_bit(NV_##flag, &(vol)->flags);			\
}

/* Emit the ntfs volume bitops functions. */
DEFINE_NVOL_BIT_OPS(Errors)
DEFINE_NVOL_BIT_OPS(ShowSystemFiles)
DEFINE_NVOL_BIT_OPS(CaseSensitive)
DEFINE_NVOL_BIT_OPS(LogFileEmpty)
DEFINE_NVOL_BIT_OPS(QuotaOutOfDate)
DEFINE_NVOL_BIT_OPS(UsnJrnlStamped)
DEFINE_NVOL_BIT_OPS(ReadOnly)
DEFINE_NVOL_BIT_OPS(Compression)
DEFINE_NVOL_BIT_OPS(FreeClusterKnown)
DEFINE_NVOL_BIT_OPS(Shutdown)
DEFINE_NVOL_BIT_OPS(SysImmutable)
DEFINE_NVOL_BIT_OPS(ShowHiddenFiles)
DEFINE_NVOL_BIT_OPS(HideDotFiles)
DEFINE_NVOL_BIT_OPS(CheckWindowsNames)
DEFINE_NVOL_BIT_OPS(Discard)
DEFINE_NVOL_BIT_OPS(DisableSparse)

static inline void ntfs_inc_free_clusters(struct ntfs_volume *vol, s64 nr)
{
	if (!NVolFreeClusterKnown(vol))
		wait_event(vol->free_waitq, NVolFreeClusterKnown(vol));
	atomic64_add(nr, &vol->free_clusters);
}

static inline void ntfs_dec_free_clusters(struct ntfs_volume *vol, s64 nr)
{
	if (!NVolFreeClusterKnown(vol))
		wait_event(vol->free_waitq, NVolFreeClusterKnown(vol));
	atomic64_sub(nr, &vol->free_clusters);
}

static inline void ntfs_inc_free_mft_records(struct ntfs_volume *vol, s64 nr)
{
	if (!NVolFreeClusterKnown(vol))
		return;

	atomic64_add(nr, &vol->free_mft_records);
}

static inline void ntfs_dec_free_mft_records(struct ntfs_volume *vol, s64 nr)
{
	if (!NVolFreeClusterKnown(vol))
		return;

	atomic64_sub(nr, &vol->free_mft_records);
}

static inline void ntfs_set_lcn_empty_bits(struct ntfs_volume *vol, unsigned long index,
		u8 val, unsigned int count)
{
	if (!NVolFreeClusterKnown(vol))
		wait_event(vol->free_waitq, NVolFreeClusterKnown(vol));

	if (val)
		vol->lcn_empty_bits_per_page[index] -= count;
	else
		vol->lcn_empty_bits_per_page[index] += count;
}

static __always_inline void ntfs_hold_dirty_clusters(struct ntfs_volume *vol, s64 nr_clusters)
{
	atomic64_add(nr_clusters, &vol->dirty_clusters);
}

static __always_inline void ntfs_release_dirty_clusters(struct ntfs_volume *vol, s64 nr_clusters)
{
	if (atomic64_read(&vol->dirty_clusters) < nr_clusters)
		atomic64_set(&vol->dirty_clusters, 0);
	else
		atomic64_sub(nr_clusters, &vol->dirty_clusters);
}

s64 ntfs_available_clusters_count(struct ntfs_volume *vol, s64 nr_clusters);
s64 get_nr_free_clusters(struct ntfs_volume *vol);
#endif /* _LINUX_NTFS_VOLUME_H */
