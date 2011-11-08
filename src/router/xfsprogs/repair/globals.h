/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _XFS_REPAIR_GLOBAL_H
#define _XFS_REPAIR_GLOBAL_H

#ifndef EXTERN
#define EXTERN extern
#endif

/* useful macros */

#define rounddown(x, y) (((x)/(y))*(y))

/* error flags */

#define XR_OK			0	/* good */
#define XR_BAD_MAGIC		1	/* bad magic number */
#define XR_BAD_BLOCKSIZE	2	/* bad block size */
#define XR_BAD_BLOCKLOG		3	/* bad sb_blocklog field */
#define XR_BAD_VERSION		4	/* bad version number */
#define XR_BAD_INPROGRESS	5	/* in progress set */
#define XR_BAD_FS_SIZE_DATA	6	/* ag sizes, number, fs size mismatch */
#define XR_BAD_INO_SIZE_DATA	7	/* bad inode size or perblock fields */
#define XR_BAD_SECT_SIZE_DATA	8	/* bad sector size info */
#define XR_AGF_GEO_MISMATCH	9	/* agf info conflicts with sb */
#define XR_AGI_GEO_MISMATCH	10	/* agf info conflicts with sb */
#define XR_SB_GEO_MISMATCH	11	/* sb geo conflicts with fs sb geo */
#define XR_EOF			12	/* seeked beyond EOF */
#define XR_BAD_RT_GEO_DATA	13	/* realtime geometry inconsistent */
#define XR_BAD_INO_MAX_PCT	14	/* max % of inodes > 100% */
#define XR_BAD_INO_ALIGN	15	/* bad inode alignment value */
#define XR_INSUFF_SEC_SB	16	/* not enough matching secondary sbs */
#define XR_BAD_SB_UNIT		17	/* bad stripe unit */
#define XR_BAD_SB_WIDTH		18	/* bad stripe width */
#define XR_BAD_SVN		19	/* bad shared version number */
#define XR_BAD_ERR_CODE		20	/* Bad error code */

/* XFS filesystem (il)legal values */

#define XR_LOG2BSIZE_MIN	9	/* min/max fs blocksize (log2) */
#define XR_LOG2BSIZE_MAX	16	/* 2^XR_* == blocksize */

#define	NUM_SBS			8	/* max # of sbs to verify */
#define NUM_AGH_SECTS		4	/* # of components in an ag header */

/*
 * secondary sb mask -- if the secondary sb feature bits has a
 * the partial sb mask bit set, then you depend on the fields
 * in it up to and including sb_inoalignmt but the unused part of the
 * sector may have trash in it.  If the sb has any bits set that are in
 * the good mask, then the entire sb and sector are good (was zero'ed
 * by mkfs).  The third mask is for filesystems made by pre-6.5 campus
 * alpha mkfs's.  Those are rare so we'll check for those under
 * a special option.
 */
#define XR_PART_SECSB_VNMASK	0x0F80	/* >= XFS_SB_VERSION_ALIGNBIT */
#define XR_GOOD_SECSB_VNMASK	0x0F00	/* >= XFS_SB_VERSION_DALIGNBIT */
#define XR_ALPHA_SECSB_VNMASK	0x0180	/* DALIGN|ALIGN bits */

/* global variables for xfs_repair */

/* arguments and argument flag variables */

EXTERN char	*fs_name;		/* name of filesystem */
EXTERN int	verbose;		/* verbose flag, mostly for debugging */


/* for reading stuff in manually (bypassing libsim) */

EXTERN char	*iobuf;			/* large buffer */
EXTERN int	iobuf_size;
EXTERN char	*smallbuf;		/* small (1-4 page) buffer */
EXTERN int	smallbuf_size;
EXTERN char	*sb_bufs[NUM_SBS];	/* superblock buffers */
EXTERN int	sbbuf_size;

/* direct I/O info */

EXTERN int	minio_align;		/* min I/O size and alignment */
EXTERN int	mem_align;		/* memory alignment */
EXTERN int	max_iosize;		/* max I/O size */

/* file descriptors */

EXTERN int	fs_fd;			/* filesystem fd */

/* command-line flags */

EXTERN int	verbose;
EXTERN int	no_modify;
EXTERN int	dangerously;		/* live dangerously ... fix ro mount */
EXTERN int	isa_file;
EXTERN int	zap_log;
EXTERN int	dumpcore;		/* abort, not exit on fatal errs */
EXTERN int	delete_attr_ok;		/* can clear attrs w/o clearing files */
EXTERN int	force_geo;		/* can set geo on low confidence info */
EXTERN int	assume_xfs;		/* assume we have an xfs fs */
EXTERN int	pre_65_beta;		/* fs was mkfs'ed by a version earlier * than 6.5-beta */
EXTERN char	*log_name;		/* Name of log device */
EXTERN int	log_spec;		/* Log dev specified as option */
EXTERN char	*rt_name;		/* Name of realtime device */
EXTERN int	rt_spec;		/* Realtime dev specified as option */
EXTERN int	convert_lazy_count;	/* Convert lazy-count mode on/off */
EXTERN int	lazy_count;		/* What to set if to if converting */

/* misc status variables */

EXTERN int		primary_sb_modified;
EXTERN int		bad_ino_btree;
EXTERN int		clear_sunit;
EXTERN int		fs_is_dirty;

/* for hunting down the root inode */

EXTERN int		need_root_inode;
EXTERN int		need_root_dotdot;

EXTERN int		need_rbmino;
EXTERN int		need_rsumino;

EXTERN int		lost_quotas;
EXTERN int		have_uquotino;
EXTERN int		have_gquotino;
EXTERN int		lost_uquotino;
EXTERN int		lost_gquotino;
EXTERN int		lost_pquotino;

EXTERN xfs_agino_t	first_prealloc_ino;
EXTERN xfs_agino_t	last_prealloc_ino;
EXTERN xfs_agblock_t	bnobt_root;
EXTERN xfs_agblock_t	bcntbt_root;
EXTERN xfs_agblock_t	inobt_root;

/* configuration vars -- fs geometry dependent */

EXTERN int		inodes_per_block;
EXTERN int		inodes_per_cluster;
EXTERN unsigned int	glob_agcount;
EXTERN int		chunks_pblock;	/* # of 64-ino chunks per allocation */
EXTERN int		max_symlink_blocks;
EXTERN __int64_t	fs_max_file_offset;

/* realtime info */

EXTERN xfs_rtword_t	*btmcompute;
EXTERN xfs_suminfo_t	*sumcompute;

/* inode tree records have full or partial backptr fields ? */

EXTERN int		full_ino_ex_data;/*
					  * if 1, use ino_ex_data_t component
					  * of ino_un union, if 0, use
					  * parent_list_t component.  see
					  * incore.h for more details
					  */

#define ORPHANAGE	"lost+found"

/* superblock counters */

EXTERN __uint64_t	sb_icount;	/* allocated (made) inodes */
EXTERN __uint64_t	sb_ifree;	/* free inodes */
EXTERN __uint64_t	sb_fdblocks;	/* free data blocks */
EXTERN __uint64_t	sb_frextents;	/* free realtime extents */

/* superblock geometry info */

EXTERN xfs_extlen_t	sb_inoalignmt;
EXTERN __uint32_t	sb_unit;
EXTERN __uint32_t	sb_width;

extern size_t 		ts_dirbuf_size;
extern size_t 		ts_dir_freemap_size;
extern size_t 		ts_attr_freemap_size;

EXTERN pthread_mutex_t	*ag_locks;

EXTERN int 		report_interval;
EXTERN __uint64_t 	*prog_rpt_done;

EXTERN int		ag_stride;
EXTERN int		thread_count;

#endif /* _XFS_REPAIR_GLOBAL_H */
