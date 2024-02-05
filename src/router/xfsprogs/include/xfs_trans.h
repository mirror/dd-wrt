// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#ifndef __XFS_TRANS_H__
#define __XFS_TRANS_H__

struct xfs_mount;
struct xfs_buftarg;
struct xfs_buf;
struct xfs_buf_map;

/*
 * Userspace Transaction interface
 */

struct xfs_item_ops {
	uint64_t (*iop_sort)(struct xfs_log_item *lip);
	int (*iop_precommit)(struct xfs_trans *tp, struct xfs_log_item *lip);
};

typedef struct xfs_log_item {
	struct list_head		li_trans;	/* transaction list */
	xfs_lsn_t			li_lsn;		/* last on-disk lsn */
	struct xfs_mount		*li_mountp;	/* ptr to fs mount */
	uint				li_type;	/* item type */
	unsigned long			li_flags;	/* misc flags */
	struct xfs_buf			*li_buf;	/* real buffer pointer */
	struct list_head		li_bio_list;	/* buffer item list */
	const struct xfs_item_ops	*li_ops;	/* function list */
} xfs_log_item_t;

#define XFS_LI_DIRTY	3	/* log item dirty in transaction */

struct xfs_inode_log_item {
	xfs_log_item_t		ili_item;		/* common portion */
	struct xfs_inode	*ili_inode;		/* inode pointer */
	unsigned short		ili_lock_flags;		/* lock flags */
	unsigned int		ili_dirty_flags;	/* dirty in current tx */
	unsigned int		ili_last_fields;	/* fields when flushed*/
	unsigned int		ili_fields;		/* fields to be logged */
	unsigned int		ili_fsync_fields;	/* ignored by userspace */
	spinlock_t		ili_lock;
};

typedef struct xfs_buf_log_item {
	xfs_log_item_t		bli_item;	/* common item structure */
	struct xfs_buf		*bli_buf;	/* real buffer pointer */
	unsigned int		bli_flags;	/* misc flags */
	unsigned int		bli_recur;	/* recursion count */
	xfs_buf_log_format_t	__bli_format;	/* in-log header */
} xfs_buf_log_item_t;

#define XFS_BLI_DIRTY			(1<<0)
#define XFS_BLI_HOLD			(1<<1)
#define XFS_BLI_STALE			(1<<2)
#define XFS_BLI_INODE_ALLOC_BUF		(1<<3)
#define XFS_BLI_ORDERED			(1<<4)

typedef struct xfs_qoff_logitem {
	xfs_log_item_t		qql_item;	/* common portion */
	struct xfs_qoff_logitem	*qql_start_lip;	/* qoff-start logitem, if any */
	xfs_qoff_logformat_t	qql_format;	/* logged structure */
} xfs_qoff_logitem_t;

typedef struct xfs_trans {
	unsigned int		t_log_res;	/* amt of log space resvd */
	unsigned int		t_log_count;	/* count for perm log res */
	unsigned int		t_blk_res;	/* # of blocks resvd */
	unsigned int		t_blk_res_used;	/* # of resvd blocks used */
	unsigned int		t_rtx_res;	/* # of rt extents resvd */
	unsigned int		t_rtx_res_used;	/* # of resvd rt extents used */
	unsigned int		t_flags;	/* misc flags */
	xfs_agnumber_t		t_highest_agno;	/* highest AGF locked */
	struct xfs_mount 	*t_mountp;	/* ptr to fs mount struct */
	struct xfs_dquot_acct	*t_dqinfo;	/* acctg info for dquots */
	long			t_icount_delta;	/* superblock icount change */
	long			t_ifree_delta;	/* superblock ifree change */
	long			t_fdblocks_delta;/* superblock fdblocks chg */
	long			t_frextents_delta;/* superblock freextents chg*/
	struct list_head	t_items;	/* log item descriptors */
	struct list_head	t_dfops;	/* deferred operations */
} xfs_trans_t;

void	xfs_trans_init(struct xfs_mount *);
int	xfs_trans_roll(struct xfs_trans **);

int	libxfs_trans_alloc(struct xfs_mount *mp, struct xfs_trans_res *resp,
			   uint blocks, uint rtextents, uint flags,
			   struct xfs_trans **tpp);
int	libxfs_trans_alloc_inode(struct xfs_inode *ip, struct xfs_trans_res *resv,
			unsigned int dblocks, unsigned int rblocks, bool force,
			struct xfs_trans **tpp);
int	libxfs_trans_alloc_rollable(struct xfs_mount *mp, uint blocks,
				    struct xfs_trans **tpp);
int	libxfs_trans_alloc_empty(struct xfs_mount *mp, struct xfs_trans **tpp);
int	libxfs_trans_commit(struct xfs_trans *);
void	libxfs_trans_cancel(struct xfs_trans *);

/* cancel dfops associated with a transaction */
void xfs_defer_cancel(struct xfs_trans *);

struct xfs_buf *libxfs_trans_getsb(struct xfs_trans *);

void	libxfs_trans_ijoin(struct xfs_trans *, struct xfs_inode *, uint);
void	libxfs_trans_log_inode (struct xfs_trans *, struct xfs_inode *,
				uint);
int	libxfs_trans_roll_inode (struct xfs_trans **, struct xfs_inode *);

void	libxfs_trans_brelse(struct xfs_trans *, struct xfs_buf *);
void	libxfs_trans_binval(struct xfs_trans *, struct xfs_buf *);
void	libxfs_trans_bjoin(struct xfs_trans *, struct xfs_buf *);
void	libxfs_trans_bhold(struct xfs_trans *, struct xfs_buf *);
void	libxfs_trans_bhold_release(struct xfs_trans *, struct xfs_buf *);
void	libxfs_trans_dirty_buf(struct xfs_trans *, struct xfs_buf *);
void	libxfs_trans_log_buf(struct xfs_trans *, struct xfs_buf *,
				uint, uint);
bool	libxfs_trans_ordered_buf(xfs_trans_t *, struct xfs_buf *);

int	libxfs_trans_get_buf_map(struct xfs_trans *tp, struct xfs_buftarg *btp,
		struct xfs_buf_map *map, int nmaps, xfs_buf_flags_t flags,
		struct xfs_buf **bpp);

int	libxfs_trans_read_buf_map(struct xfs_mount *mp, struct xfs_trans *tp,
				  struct xfs_buftarg *btp,
				  struct xfs_buf_map *map, int nmaps,
				  xfs_buf_flags_t flags, struct xfs_buf **bpp,
				  const struct xfs_buf_ops *ops);
static inline int
libxfs_trans_get_buf(
	struct xfs_trans	*tp,
	struct xfs_buftarg	*btp,
	xfs_daddr_t		blkno,
	int			numblks,
	uint			flags,
	struct xfs_buf		**bpp)
{
	DEFINE_SINGLE_BUF_MAP(map, blkno, numblks);

	return libxfs_trans_get_buf_map(tp, btp, &map, 1, flags, bpp);
}

static inline int
libxfs_trans_read_buf(
	struct xfs_mount	*mp,
	struct xfs_trans	*tp,
	struct xfs_buftarg	*btp,
	xfs_daddr_t		blkno,
	int			numblks,
	xfs_buf_flags_t		flags,
	struct xfs_buf		**bpp,
	const struct xfs_buf_ops *ops)
{
	DEFINE_SINGLE_BUF_MAP(map, blkno, numblks);
	return libxfs_trans_read_buf_map(mp, tp, btp, &map, 1, flags, bpp, ops);
}

#define xfs_log_item_in_current_chkpt(lip)	(false)
#define xfs_trans_item_relog(lip, tp)		(NULL)

/* Contorted mess to make gcc shut up about unused vars. */
#define xlog_grant_push_threshold(log, need)    \
		((log) == (log) ? NULLCOMMITLSN : NULLCOMMITLSN)

/* from xfs_log.h */
/*
 * By comparing each component, we don't have to worry about extra
 * endian issues in treating two 32 bit numbers as one 64 bit number
 */
static inline xfs_lsn_t _lsn_cmp(xfs_lsn_t lsn1, xfs_lsn_t lsn2)
{
	if (CYCLE_LSN(lsn1) != CYCLE_LSN(lsn2))
		return (CYCLE_LSN(lsn1)<CYCLE_LSN(lsn2))? -999 : 999;

	if (BLOCK_LSN(lsn1) != BLOCK_LSN(lsn2))
		return (BLOCK_LSN(lsn1)<BLOCK_LSN(lsn2))? -999 : 999;

	return 0;
}

#define XFS_LSN_CMP(a, b)			_lsn_cmp(a, b)

#endif	/* __XFS_TRANS_H__ */
