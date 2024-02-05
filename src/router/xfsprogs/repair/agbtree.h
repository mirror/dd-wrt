/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef __XFS_REPAIR_AG_BTREE_H__
#define __XFS_REPAIR_AG_BTREE_H__

/* Context for rebuilding a per-AG btree. */
struct bt_rebuild {
	/* Fake root for staging and space preallocations. */
	struct bulkload	newbt;

	/* Geometry of the new btree. */
	struct xfs_btree_bload	bload;

	/* Staging btree cursor for the new tree. */
	struct xfs_btree_cur	*cur;

	/* Tree-specific data. */
	union {
		struct xfs_slab_cursor	*slab_cursor;
		struct {
			struct extent_tree_node	*bno_rec;
			unsigned int		freeblks;
		};
		struct {
			struct ino_tree_node	*ino_rec;
			xfs_agino_t		first_agino;
			xfs_agino_t		count;
			xfs_agino_t		freecount;
		};
	};
};

void finish_rebuild(struct xfs_mount *mp, struct bt_rebuild *btr,
		struct bitmap *lost_blocks);
void init_freespace_cursors(struct repair_ctx *sc, struct xfs_perag *pag,
		unsigned int free_space, unsigned int *nr_extents,
		int *extra_blocks, struct bt_rebuild *btr_bno,
		struct bt_rebuild *btr_cnt);
void build_freespace_btrees(struct repair_ctx *sc, xfs_agnumber_t agno,
		struct bt_rebuild *btr_bno, struct bt_rebuild *btr_cnt);

void init_ino_cursors(struct repair_ctx *sc, struct xfs_perag *pag,
		unsigned int free_space, uint64_t *num_inos,
		uint64_t *num_free_inos, struct bt_rebuild *btr_ino,
		struct bt_rebuild *btr_fino);
void build_inode_btrees(struct repair_ctx *sc, xfs_agnumber_t agno,
		struct bt_rebuild *btr_ino, struct bt_rebuild *btr_fino);

void init_rmapbt_cursor(struct repair_ctx *sc, struct xfs_perag *pag,
		unsigned int free_space, struct bt_rebuild *btr);
void build_rmap_tree(struct repair_ctx *sc, xfs_agnumber_t agno,
		struct bt_rebuild *btr);

void init_refc_cursor(struct repair_ctx *sc, struct xfs_perag *pag,
		unsigned int free_space, struct bt_rebuild *btr);
void build_refcount_tree(struct repair_ctx *sc, xfs_agnumber_t agno,
		struct bt_rebuild *btr);

xfs_extlen_t estimate_agbtree_blocks(struct xfs_perag *pag,
		unsigned int free_extents);

#endif /* __XFS_REPAIR_AG_BTREE_H__ */
