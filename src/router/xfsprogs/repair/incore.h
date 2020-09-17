// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#ifndef XFS_REPAIR_INCORE_H
#define XFS_REPAIR_INCORE_H

#include "avl.h"


/*
 * contains definition information.  implementation (code)
 * is spread out in separate files.
 */

/*
 * block map -- track state of each filesystem block.
 */

void		init_bmaps(xfs_mount_t *mp);
void		reset_bmaps(xfs_mount_t *mp);
void		free_bmaps(xfs_mount_t *mp);

void		set_bmap_ext(xfs_agnumber_t agno, xfs_agblock_t agbno,
			     xfs_extlen_t blen, int state);
int		get_bmap_ext(xfs_agnumber_t agno, xfs_agblock_t agbno,
			     xfs_agblock_t maxbno, xfs_extlen_t *blen);

void		set_rtbmap(xfs_rtblock_t bno, int state);
int		get_rtbmap(xfs_rtblock_t bno);

static inline void
set_bmap(xfs_agnumber_t agno, xfs_agblock_t agbno, int state)
{
	set_bmap_ext(agno, agbno, 1, state);
}

static inline int
get_bmap(xfs_agnumber_t agno, xfs_agblock_t agbno)
{
	return get_bmap_ext(agno, agbno, agbno + 1, NULL);
}

/*
 * extent tree definitions
 * right now, there are 3 trees per AG, a bno tree, a bcnt tree
 * and a tree for dup extents.  If the code is modified in the
 * future to use an extent tree instead of a bitmask for tracking
 * fs blocks, then we could lose the dup extent tree if we labelled
 * each extent with the inode that owned it.
 */

typedef unsigned char extent_state_t;

typedef struct extent_tree_node  {
	avlnode_t		avl_node;
	xfs_agblock_t		ex_startblock;	/* starting block (agbno) */
	xfs_extlen_t		ex_blockcount;	/* number of blocks in extent */
	extent_state_t		ex_state;	/* see state flags below */

	struct extent_tree_node		*next;	/* for bcnt extent lists */
	struct extent_tree_node		*last;	/* for bcnt extent list anchors */
#if 0
	xfs_ino_t		ex_inode;	/* owner, NULL if free or  */
						/*	multiply allocated */
#endif
} extent_tree_node_t;

typedef struct rt_extent_tree_node  {
	avlnode_t		avl_node;
	xfs_rtblock_t		rt_startblock;	/* starting realtime block */
	xfs_extlen_t		rt_blockcount;	/* number of blocks in extent */
	extent_state_t		rt_state;	/* see state flags below */

#if 0
	xfs_ino_t		ex_inode;	/* owner, NULL if free or  */
						/*	multiply allocated */
#endif
} rt_extent_tree_node_t;

/* extent states, prefix with XR_ to avoid conflict with buffer cache defines */

#define XR_E_UNKNOWN	0	/* unknown state */
#define XR_E_FREE1	1	/* free block (marked by one fs space tree) */
#define XR_E_FREE	2	/* free block (marked by both fs space trees) */
#define XR_E_INUSE	3	/* extent used by file/dir data or metadata */
#define XR_E_INUSE_FS	4	/* extent used by fs ag header or log */
#define XR_E_MULT	5	/* extent is multiply referenced */
#define XR_E_INO	6	/* extent used by inodes (inode blocks) */
#define XR_E_FS_MAP	7	/* extent used by fs space/inode maps */
#define XR_E_INUSE1	8	/* used block (marked by rmap btree) */
#define XR_E_INUSE_FS1	9	/* used by fs ag header or log (rmap btree) */
#define XR_E_INO1	10	/* used by inodes (marked by rmap btree) */
#define XR_E_FS_MAP1	11	/* used by fs space/inode maps (rmap btree) */
#define XR_E_REFC	12	/* used by fs ag reference count btree */
#define XR_E_COW	13	/* leftover cow extent */
#define XR_E_BAD_STATE	14

/* separate state bit, OR'ed into high (4th) bit of ex_state field */

#define XR_E_WRITTEN	0x8	/* extent has been written out, can't reclaim */
#define written(state)		((state) & XR_E_WRITTEN)
#define set_written(state)	(state) &= XR_E_WRITTEN

/*
 * bno extent tree functions
 */
void
add_bno_extent(xfs_agnumber_t agno, xfs_agblock_t startblock,
		xfs_extlen_t blockcount);

extent_tree_node_t *
findfirst_bno_extent(xfs_agnumber_t agno);

extent_tree_node_t *
find_bno_extent(xfs_agnumber_t agno, xfs_agblock_t agbno);

extent_tree_node_t *
findfirst_bno_extent(xfs_agnumber_t agno);

#define findnext_bno_extent(exent_ptr)	\
		((extent_tree_node_t *) ((exent_ptr)->avl_node.avl_nextino))

void
get_bno_extent(xfs_agnumber_t agno, extent_tree_node_t *ext);

/*
 * bcnt tree functions
 */
void
add_bcnt_extent(xfs_agnumber_t agno, xfs_agblock_t startblock,
		xfs_extlen_t blockcount);

extent_tree_node_t *
findfirst_bcnt_extent(xfs_agnumber_t agno);

extent_tree_node_t *
find_bcnt_extent(xfs_agnumber_t agno, xfs_agblock_t agbno);

extent_tree_node_t *
findbiggest_bcnt_extent(xfs_agnumber_t agno);

extent_tree_node_t *
findnext_bcnt_extent(xfs_agnumber_t agno, extent_tree_node_t *ext);

extent_tree_node_t *
get_bcnt_extent(xfs_agnumber_t agno, xfs_agblock_t startblock,
		xfs_extlen_t blockcount);

/*
 * duplicate extent tree functions
 */

int		add_dup_extent(xfs_agnumber_t agno, xfs_agblock_t startblock,
			xfs_extlen_t blockcount);
int		search_dup_extent(xfs_agnumber_t agno,
			xfs_agblock_t start_agbno, xfs_agblock_t end_agbno);
void		add_rt_dup_extent(xfs_rtblock_t	startblock,
				xfs_extlen_t	blockcount);

int		search_rt_dup_extent(xfs_mount_t	*mp,
					xfs_rtblock_t	bno);

/*
 * extent/tree recyling and deletion routines
 */

/*
 * return an extent node to the extent node free list
 */
void		release_extent_tree_node(extent_tree_node_t *node);

/*
 * recycle all the nodes in the per-AG tree
 */
void		release_dup_extent_tree(xfs_agnumber_t agno);
void		release_agbno_extent_tree(xfs_agnumber_t agno);
void		release_agbcnt_extent_tree(xfs_agnumber_t agno);

/*
 * realtime duplicate extent tree - this one actually frees the memory
 */
void		free_rt_dup_extent_tree(xfs_mount_t *mp);

void		incore_ext_init(xfs_mount_t *);
/*
 * per-AG extent trees shutdown routine -- all (bno, bcnt and dup)
 * at once.  this one actually frees the memory instead of just recyling
 * the nodes.
 */
void		incore_ext_teardown(xfs_mount_t *mp);
void		incore_ino_init(xfs_mount_t *);

int		count_bno_extents(xfs_agnumber_t);
int		count_bno_extents_blocks(xfs_agnumber_t, uint *);
int		count_bcnt_extents(xfs_agnumber_t);

/*
 * inode definitions
 */

/* inode types */

#define XR_INO_UNKNOWN	0		/* unknown */
#define XR_INO_DIR	1		/* directory */
#define XR_INO_RTDATA	2		/* realtime file */
#define XR_INO_RTBITMAP	3		/* realtime bitmap inode */
#define XR_INO_RTSUM	4		/* realtime summary inode */
#define XR_INO_DATA	5		/* regular file */
#define XR_INO_SYMLINK	6		/* symlink */
#define XR_INO_CHRDEV	7		/* character device */
#define XR_INO_BLKDEV	8		/* block device */
#define XR_INO_SOCK	9		/* socket */
#define XR_INO_FIFO	10		/* fifo */
#define XR_INO_MOUNTPOINT 11		/* mountpoint */
#define XR_INO_UQUOTA	12		/* user quota inode */
#define XR_INO_GQUOTA	13		/* group quota inode */
#define XR_INO_PQUOTA	14		/* project quota inode */

/* inode allocation tree */

/*
 * Inodes in the inode allocation trees are allocated in chunks.
 * Those groups can be easily duplicated in our trees.
 * Disconnected inodes are harder.  We can do one of two
 * things in that case:  if we know the inode allocation btrees
 * are good, then we can disallow directory references to unknown
 * inode chunks.  If the inode allocation trees have been trashed or
 * we feel like being aggressive, then as we hit unknown inodes,
 * we can search on the disk for all contiguous inodes and see if
 * they fit into chunks.  Before putting them into the inode tree,
 * we can scan each inode starting at the earliest inode to see which
 * ones are good.  This protects us from the pathalogical case of
 * inodes appearing in user-data.  We still may have to mark the
 * inodes as "possibly fake" so that if a file claims the blocks,
 * we decide to believe the inodes, especially if they're not
 * connected.
 */

#define PLIST_CHUNK_SIZE	4

typedef xfs_ino_t parent_entry_t;

struct nlink_ops;

typedef struct parent_list  {
	uint64_t		pmask;
	parent_entry_t		*pentries;
#ifdef DEBUG
	short			cnt;
#endif
} parent_list_t;

union ino_nlink {
	uint8_t		*un8;
	uint16_t	*un16;
	uint32_t	*un32;
};

typedef struct ino_ex_data  {
	uint64_t		ino_reached;	/* bit == 1 if reached */
	uint64_t		ino_processed;	/* reference checked bit mask */
	parent_list_t		*parents;
	union ino_nlink		counted_nlinks;/* counted nlinks in P6 */
} ino_ex_data_t;

typedef struct ino_tree_node  {
	avlnode_t		avl_node;
	xfs_agino_t		ino_startnum;	/* starting inode # */
	xfs_inofree_t		ir_free;	/* inode free bit mask */
	uint64_t		ir_sparse;	/* sparse inode bitmask */
	uint64_t		ino_confirmed;	/* confirmed bitmask */
	uint64_t		ino_isa_dir;	/* bit == 1 if a directory */
	uint64_t		ino_was_rl;	/* bit == 1 if reflink flag set */
	uint64_t		ino_is_rl;	/* bit == 1 if reflink flag should be set */
	uint8_t			nlink_size;
	union ino_nlink		disk_nlinks;	/* on-disk nlinks, set in P3 */
	union  {
		ino_ex_data_t	*ex_data;	/* phases 6,7 */
		parent_list_t	*plist;		/* phases 2-5 */
	} ino_un;
	uint8_t			*ftypes;	/* phases 3,6 */
} ino_tree_node_t;

#define INOS_PER_IREC	(sizeof(uint64_t) * NBBY)
#define	IREC_MASK(i)	((uint64_t)1 << (i))

void		add_ino_ex_data(xfs_mount_t *mp);

/*
 * return an inode record to the free inode record pool
 */
void		free_inode_rec(xfs_agnumber_t agno, ino_tree_node_t *ino_rec);

/*
 * get pulls the inode record from the good inode tree
 */
void		get_inode_rec(struct xfs_mount *mp, xfs_agnumber_t agno,
			      ino_tree_node_t *ino_rec);

extern avltree_desc_t     **inode_tree_ptrs;

static inline int
get_inode_offset(struct xfs_mount *mp, xfs_ino_t ino, ino_tree_node_t *irec)
{
	return XFS_INO_TO_AGINO(mp, ino) - irec->ino_startnum;
}
static inline ino_tree_node_t *
findfirst_inode_rec(xfs_agnumber_t agno)
{
	return((ino_tree_node_t *) inode_tree_ptrs[agno]->avl_firstino);
}
static inline ino_tree_node_t *
find_inode_rec(struct xfs_mount *mp, xfs_agnumber_t agno, xfs_agino_t ino)
{
	/*
	 * Is the AG inside the file system
	 */
	if (agno >= mp->m_sb.sb_agcount)
		return NULL;
	return((ino_tree_node_t *)
		avl_findrange(inode_tree_ptrs[agno], ino));
}
void		find_inode_rec_range(struct xfs_mount *mp, xfs_agnumber_t agno,
			xfs_agino_t start_ino, xfs_agino_t end_ino,
			ino_tree_node_t **first, ino_tree_node_t **last);

/*
 * set inode states -- setting an inode to used or free also
 * automatically marks it as "existing".  Note -- all the inode
 * add/set/get routines assume a valid inode number.
 */
ino_tree_node_t	*set_inode_used_alloc(struct xfs_mount *mp, xfs_agnumber_t agno,
				      xfs_agino_t ino);
ino_tree_node_t	*set_inode_free_alloc(struct xfs_mount *mp, xfs_agnumber_t agno,
				      xfs_agino_t ino);

void		print_inode_list(xfs_agnumber_t agno);
void		print_uncertain_inode_list(xfs_agnumber_t agno);

/*
 * separate trees for uncertain inodes (they may not exist).
 */
ino_tree_node_t		*findfirst_uncertain_inode_rec(xfs_agnumber_t agno);
ino_tree_node_t		*find_uncertain_inode_rec(xfs_agnumber_t agno,
						xfs_agino_t ino);
void			add_inode_uncertain(xfs_mount_t *mp,
						xfs_ino_t ino, int free);
void			add_aginode_uncertain(struct xfs_mount *mp,
						xfs_agnumber_t agno,
						xfs_agino_t agino, int free);
void			get_uncertain_inode_rec(struct xfs_mount *mp,
						xfs_agnumber_t agno,
						ino_tree_node_t *ino_rec);
void			clear_uncertain_ino_cache(xfs_agnumber_t agno);

/*
 * return next in-order inode tree node.  takes an "ino_tree_node_t *"
 */
#define next_ino_rec(ino_node_ptr)	\
		((ino_tree_node_t *) ((ino_node_ptr)->avl_node.avl_nextino))
/*
 * return the next linked inode (forward avl tree link)-- meant to be used
 * by linked list routines (uncertain inode routines/records)
 */
#define next_link_rec(ino_node_ptr)	\
		((ino_tree_node_t *) ((ino_node_ptr)->avl_node.avl_forw))

/*
 * finobt helpers
 */

static inline bool
inode_rec_has_free(struct ino_tree_node *ino_rec)
{
	/* must have real, allocated inodes for finobt */
	return ino_rec->ir_free & ~ino_rec->ir_sparse;
}

static inline ino_tree_node_t *
findfirst_free_inode_rec(xfs_agnumber_t agno)
{
	ino_tree_node_t *ino_rec;

	ino_rec = findfirst_inode_rec(agno);

	while (ino_rec && !inode_rec_has_free(ino_rec))
		ino_rec = next_ino_rec(ino_rec);

	return ino_rec;
}

static inline ino_tree_node_t *
next_free_ino_rec(ino_tree_node_t *ino_rec)
{
	ino_rec = next_ino_rec(ino_rec);

	while (ino_rec && !inode_rec_has_free(ino_rec))
		ino_rec = next_ino_rec(ino_rec);

	return ino_rec;
}

/*
 * Has an inode been processed for phase 6 (reference count checking)?
 *
 * add_inode_refchecked() is set on an inode when it gets traversed
 * during the reference count phase (6).  It's set so that if the inode
 * is a directory, it's traversed (and it's links counted) only once.
 */
static inline void add_inode_refchecked(struct ino_tree_node *irec, int offset)
{
	irec->ino_un.ex_data->ino_processed |= IREC_MASK(offset);
}

static inline int is_inode_refchecked(struct ino_tree_node *irec, int offset)
{
	return (irec->ino_un.ex_data->ino_processed & IREC_MASK(offset)) != 0;
}

/*
 * set/test is inode known to be valid (although perhaps corrupt)
 */
static inline void set_inode_confirmed(struct ino_tree_node *irec, int offset)
{
	irec->ino_confirmed |= IREC_MASK(offset);
}

static inline int is_inode_confirmed(struct ino_tree_node *irec, int offset)
{
	return (irec->ino_confirmed & IREC_MASK(offset)) != 0;
}

/*
 * set/clear/test is inode a directory inode
 */
static inline void set_inode_isadir(struct ino_tree_node *irec, int offset)
{
	irec->ino_isa_dir |= IREC_MASK(offset);
}

static inline void clear_inode_isadir(struct ino_tree_node *irec, int offset)
{
	irec->ino_isa_dir &= ~IREC_MASK(offset);
}

static inline int inode_isadir(struct ino_tree_node *irec, int offset)
{
	return (irec->ino_isa_dir & IREC_MASK(offset)) != 0;
}

/*
 * set/clear/test is inode free or used
 */
static inline void set_inode_free(struct ino_tree_node *irec, int offset)
{
	set_inode_confirmed(irec, offset);
	irec->ir_free |= XFS_INOBT_MASK(offset);

}

static inline void set_inode_used(struct ino_tree_node *irec, int offset)
{
	set_inode_confirmed(irec, offset);
	irec->ir_free &= ~XFS_INOBT_MASK(offset);
}

static inline int is_inode_free(struct ino_tree_node *irec, int offset)
{
	return (irec->ir_free & XFS_INOBT_MASK(offset)) != 0;
}

/*
 * set/test is inode sparse (not physically allocated)
 */
static inline void set_inode_sparse(struct ino_tree_node *irec, int offset)
{
	irec->ir_sparse |= XFS_INOBT_MASK(offset);
}

static inline bool is_inode_sparse(struct ino_tree_node *irec, int offset)
{
	return irec->ir_sparse & XFS_INOBT_MASK(offset);
}

/*
 * set/clear/test was inode marked as reflinked
 */
static inline void set_inode_was_rl(struct ino_tree_node *irec, int offset)
{
	irec->ino_was_rl |= IREC_MASK(offset);
}

static inline void clear_inode_was_rl(struct ino_tree_node *irec, int offset)
{
	irec->ino_was_rl &= ~IREC_MASK(offset);
}

static inline int inode_was_rl(struct ino_tree_node *irec, int offset)
{
	return (irec->ino_was_rl & IREC_MASK(offset)) != 0;
}

/*
 * set/clear/test should inode be marked as reflinked
 */
static inline void set_inode_is_rl(struct ino_tree_node *irec, int offset)
{
	irec->ino_is_rl |= IREC_MASK(offset);
}

static inline void clear_inode_is_rl(struct ino_tree_node *irec, int offset)
{
	irec->ino_is_rl &= ~IREC_MASK(offset);
}

static inline int inode_is_rl(struct ino_tree_node *irec, int offset)
{
	return (irec->ino_is_rl & IREC_MASK(offset)) != 0;
}

/*
 * add_inode_reached() is set on inode I only if I has been reached
 * by an inode P claiming to be the parent and if I is a directory,
 * the .. link in the I says that P is I's parent.
 *
 * add_inode_ref() is called every time a link to an inode is
 * detected and drop_inode_ref() is called every time a link to
 * an inode that we've counted is removed.
 */
void add_inode_ref(struct ino_tree_node *irec, int offset);
void drop_inode_ref(struct ino_tree_node *irec, int offset);
uint32_t num_inode_references(struct ino_tree_node *irec, int offset);

void set_inode_disk_nlinks(struct ino_tree_node *irec, int offset, uint32_t nlinks);
uint32_t get_inode_disk_nlinks(struct ino_tree_node *irec, int offset);

static inline int is_inode_reached(struct ino_tree_node *irec, int offset)
{
	ASSERT(irec->ino_un.ex_data != NULL);
	return (irec->ino_un.ex_data->ino_reached & IREC_MASK(offset)) != 0;
}

static inline void add_inode_reached(struct ino_tree_node *irec, int offset)
{
	add_inode_ref(irec, offset);
	irec->ino_un.ex_data->ino_reached |= IREC_MASK(offset);
}

/*
 * get/set inode filetype. Only used if the superblock feature bit is set
 * which allocates irec->ftypes.
 */
static inline void
set_inode_ftype(struct ino_tree_node *irec,
	int		ino_offset,
	uint8_t		ftype)
{
	if (irec->ftypes)
		irec->ftypes[ino_offset] = ftype;
}

static inline uint8_t
get_inode_ftype(
	struct ino_tree_node *irec,
	int		ino_offset)
{
	if (!irec->ftypes)
		return XFS_DIR3_FT_UNKNOWN;
	return irec->ftypes[ino_offset];
}

/*
 * set/get inode number of parent -- works for directory inodes only
 */
void		set_inode_parent(ino_tree_node_t *irec, int ino_offset,
					xfs_ino_t ino);
xfs_ino_t	get_inode_parent(ino_tree_node_t *irec, int ino_offset);

/*
 * Allocate extra inode data
 */
void		alloc_ex_data(ino_tree_node_t *irec);

/*
 * bmap cursor for tracking and fixing bmap btrees.  All xfs btrees number
 * the levels with 0 being the leaf and every level up being 1 greater.
 */

#define XR_MAX_BMLEVELS		10	/* XXX - rcc need to verify number */

typedef struct bm_level_state  {
	xfs_fsblock_t		fsbno;
	xfs_fsblock_t		left_fsbno;
	xfs_fsblock_t		right_fsbno;
	uint64_t		first_key;
	uint64_t		last_key;
/*
	int			level;
	uint64_t		prev_last_key;
	xfs_buf_t		*bp;
	xfs_bmbt_block_t	*block;
*/
} bm_level_state_t;

typedef struct bm_cursor  {
	int			num_levels;
	xfs_ino_t		ino;
	xfs_dinode_t		*dip;
	bm_level_state_t	level[XR_MAX_BMLEVELS];
} bmap_cursor_t;

void init_bm_cursor(bmap_cursor_t *cursor, int num_level);

/*
 * On-disk inobt record helpers. The sparse inode record format has a single
 * byte freecount. The older format has a 32-bit freecount and thus byte
 * conversion is necessary.
 */

static inline int
inorec_get_freecount(
	struct xfs_mount	*mp,
	struct xfs_inobt_rec	*rp)
{
	if (xfs_sb_version_hassparseinodes(&mp->m_sb))
		return rp->ir_u.sp.ir_freecount;
	return be32_to_cpu(rp->ir_u.f.ir_freecount);
}

static inline void
inorec_set_freecount(
	struct xfs_mount	*mp,
	struct xfs_inobt_rec	*rp,
	int			freecount)
{
	if (xfs_sb_version_hassparseinodes(&mp->m_sb))
		rp->ir_u.sp.ir_freecount = freecount;
	else
		rp->ir_u.f.ir_freecount = cpu_to_be32(freecount);
}

#endif /* XFS_REPAIR_INCORE_H */
