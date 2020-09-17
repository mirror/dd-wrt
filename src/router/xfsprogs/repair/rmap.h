// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef RMAP_H_
#define RMAP_H_

extern bool collect_rmaps;

extern bool rmap_needs_work(struct xfs_mount *);

extern void rmaps_init(struct xfs_mount *);
extern void rmaps_free(struct xfs_mount *);

extern int rmap_add_rec(struct xfs_mount *, xfs_ino_t, int, struct xfs_bmbt_irec *);
extern int rmap_finish_collecting_fork_recs(struct xfs_mount *mp,
		xfs_agnumber_t agno);
extern int rmap_add_ag_rec(struct xfs_mount *, xfs_agnumber_t agno,
		xfs_agblock_t agbno, xfs_extlen_t len, uint64_t owner);
extern int rmap_add_bmbt_rec(struct xfs_mount *, xfs_ino_t, int, xfs_fsblock_t);
extern int rmap_fold_raw_recs(struct xfs_mount *mp, xfs_agnumber_t agno);
extern bool rmaps_are_mergeable(struct xfs_rmap_irec *r1, struct xfs_rmap_irec *r2);

extern int rmap_add_fixed_ag_rec(struct xfs_mount *, xfs_agnumber_t);
extern int rmap_store_ag_btree_rec(struct xfs_mount *, xfs_agnumber_t);

extern size_t rmap_record_count(struct xfs_mount *, xfs_agnumber_t);
extern int rmap_init_cursor(xfs_agnumber_t, struct xfs_slab_cursor **);
extern void rmap_avoid_check(void);
extern int rmaps_verify_btree(struct xfs_mount *, xfs_agnumber_t);

extern int64_t rmap_diffkeys(struct xfs_rmap_irec *kp1,
		struct xfs_rmap_irec *kp2);
extern void rmap_high_key_from_rec(struct xfs_rmap_irec *rec,
		struct xfs_rmap_irec *key);

extern int compute_refcounts(struct xfs_mount *, xfs_agnumber_t);
extern size_t refcount_record_count(struct xfs_mount *, xfs_agnumber_t);
extern int init_refcount_cursor(xfs_agnumber_t, struct xfs_slab_cursor **);
extern void refcount_avoid_check(void);
extern int check_refcounts(struct xfs_mount *, xfs_agnumber_t);

extern void record_inode_reflink_flag(struct xfs_mount *, struct xfs_dinode *,
	xfs_agnumber_t, xfs_agino_t, xfs_ino_t);
extern int fix_inode_reflink_flags(struct xfs_mount *, xfs_agnumber_t);

extern void fix_freelist(struct xfs_mount *, xfs_agnumber_t, bool);
extern void rmap_store_agflcount(struct xfs_mount *, xfs_agnumber_t, int);

#endif /* RMAP_H_ */
