// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "globals.h"

/* global variables for xfs_repair */

/* arguments and argument flag variables */

char	*fs_name;		/* name of filesystem */
int	verbose;		/* verbose flag, mostly for debugging */


/* for reading stuff in manually (bypassing libsim) */

char	*iobuf;			/* large buffer */
int	iobuf_size;
char	*smallbuf;		/* small (1-4 page) buffer */
int	smallbuf_size;
int	sbbuf_size;

/* direct I/O info */

int	minio_align;		/* min I/O size and alignment */
int	mem_align;		/* memory alignment */
int	max_iosize;		/* max I/O size */

/* file descriptors */

int	fs_fd;			/* filesystem fd */

/* command-line flags */

int	verbose;
int	no_modify;
int	dangerously;		/* live dangerously ... fix ro mount */
int	isa_file;
int	zap_log;
int	dumpcore;		/* abort, not exit on fatal errs */
int	force_geo;		/* can set geo on low confidence info */
int	assume_xfs;		/* assume we have an xfs fs */
char	*log_name;		/* Name of log device */
int	log_spec;		/* Log dev specified as option */
char	*rt_name;		/* Name of realtime device */
int	rt_spec;		/* Realtime dev specified as option */
int	convert_lazy_count;	/* Convert lazy-count mode on/off */
int	lazy_count;		/* What to set if to if converting */
bool	features_changed;	/* did we change superblock feature bits? */
bool	add_inobtcount;		/* add inode btree counts to AGI */
bool	add_bigtime;		/* add support for timestamps up to 2486 */
bool	add_nrext64;

/* misc status variables */

int	primary_sb_modified;
int	bad_ino_btree;
int	copied_sunit;
int	fs_is_dirty;

/* for hunting down the root inode */

int	need_root_inode;
int	need_root_dotdot;

int	need_rbmino;
int	need_rsumino;

int	lost_quotas;
int	have_uquotino;
int	have_gquotino;
int	have_pquotino;
int	lost_uquotino;
int	lost_gquotino;
int	lost_pquotino;

/* configuration vars -- fs geometry dependent */

int		inodes_per_block;
unsigned int	glob_agcount;
int		chunks_pblock;	/* # of 64-ino chunks per allocation */
int		max_symlink_blocks;
int64_t		fs_max_file_offset;

/* realtime info */

xfs_rtword_t	*btmcompute;
xfs_suminfo_t	*sumcompute;

/* inode tree records have full or partial backptr fields ? */

int	full_ino_ex_data;	/*
				 * if 1, use ino_ex_data_t component
				 * of ino_un union, if 0, use
				 * parent_list_t component.  see
				 * incore.h for more details
				 */

#define ORPHANAGE	"lost+found"

/* superblock counters */

uint64_t	sb_icount;	/* allocated (made) inodes */
uint64_t	sb_ifree;	/* free inodes */
uint64_t	sb_fdblocks;	/* free data blocks */
uint64_t	sb_frextents;	/* free realtime extents */

/* superblock geometry info */

xfs_extlen_t	sb_inoalignmt;
uint32_t	sb_unit;
uint32_t	sb_width;

struct aglock	*ag_locks;
struct aglock	rt_lock;

int		report_interval;
uint64_t	*prog_rpt_done;

int		ag_stride;
int		thread_count;

/* If nonzero, simulate failure after this phase. */
int		fail_after_phase;
