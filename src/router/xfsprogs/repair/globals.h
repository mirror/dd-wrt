// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#ifndef _XFS_REPAIR_GLOBAL_H
#define _XFS_REPAIR_GLOBAL_H

#include "libxfs.h"

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
#define XR_BAD_CRC		20	/* Bad CRC */
#define XR_BAD_DIR_SIZE_DATA	21	/* Bad directory geometry */
#define XR_BAD_LOG_GEOMETRY	22	/* Bad log geometry */
#define XR_BAD_ERR_CODE		23	/* Bad error code */

/* XFS filesystem (il)legal values */

#define XR_LOG2BSIZE_MIN	9	/* min/max fs blocksize (log2) */
#define XR_LOG2BSIZE_MAX	16	/* 2^XR_* == blocksize */

#define NUM_AGH_SECTS		4	/* # of components in an ag header */

/* global variables for xfs_repair */

/* arguments and argument flag variables */

extern char	*fs_name;		/* name of filesystem */
extern int	verbose;		/* verbose flag, mostly for debugging */


/* for reading stuff in manually (bypassing libsim) */

extern char	*iobuf;			/* large buffer */
extern int	iobuf_size;
extern char	*smallbuf;		/* small (1-4 page) buffer */
extern int	smallbuf_size;
extern int	sbbuf_size;

/* direct I/O info */

extern int	minio_align;		/* min I/O size and alignment */
extern int	mem_align;		/* memory alignment */
extern int	max_iosize;		/* max I/O size */

/* file descriptors */

extern int	fs_fd;			/* filesystem fd */

/* command-line flags */

extern int	verbose;
extern int	no_modify;
extern int	dangerously;		/* live dangerously ... fix ro mount */
extern int	isa_file;
extern int	zap_log;
extern int	dumpcore;		/* abort, not exit on fatal errs */
extern int	force_geo;		/* can set geo on low confidence info */
extern int	assume_xfs;		/* assume we have an xfs fs */
extern char	*log_name;		/* Name of log device */
extern int	log_spec;		/* Log dev specified as option */
extern char	*rt_name;		/* Name of realtime device */
extern int	rt_spec;		/* Realtime dev specified as option */
extern int	convert_lazy_count;	/* Convert lazy-count mode on/off */
extern int	lazy_count;		/* What to set if to if converting */

/* misc status variables */

extern int		primary_sb_modified;
extern int		bad_ino_btree;
extern int		copied_sunit;
extern int		fs_is_dirty;

/* for hunting down the root inode */

extern int		need_root_inode;
extern int		need_root_dotdot;

extern int		need_rbmino;
extern int		need_rsumino;

extern int		lost_quotas;
extern int		have_uquotino;
extern int		have_gquotino;
extern int		have_pquotino;
extern int		lost_uquotino;
extern int		lost_gquotino;
extern int		lost_pquotino;

/* configuration vars -- fs geometry dependent */

extern int		inodes_per_block;
extern unsigned int	glob_agcount;
extern int		chunks_pblock;	/* # of 64-ino chunks per allocation */
extern int		max_symlink_blocks;
extern int64_t		fs_max_file_offset;

/* realtime info */

extern xfs_rtword_t	*btmcompute;
extern xfs_suminfo_t	*sumcompute;

/* inode tree records have full or partial backptr fields ? */

extern int		full_ino_ex_data;/*
					  * if 1, use ino_ex_data_t component
					  * of ino_un union, if 0, use
					  * parent_list_t component.  see
					  * incore.h for more details
					  */

#define ORPHANAGE	"lost+found"

/* superblock counters */

extern uint64_t	sb_icount;	/* allocated (made) inodes */
extern uint64_t	sb_ifree;	/* free inodes */
extern uint64_t	sb_fdblocks;	/* free data blocks */
extern uint64_t	sb_frextents;	/* free realtime extents */

/* superblock geometry info */

extern xfs_extlen_t	sb_inoalignmt;
extern uint32_t	sb_unit;
extern uint32_t	sb_width;

struct aglock {
	pthread_mutex_t	lock __attribute__((__aligned__(64)));
};
extern struct aglock	*ag_locks;

extern int		report_interval;
extern uint64_t		*prog_rpt_done;

extern int		ag_stride;
extern int		thread_count;

#endif /* _XFS_REPAIR_GLOBAL_H */
