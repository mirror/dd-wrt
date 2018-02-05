/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
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

#include "libxfs.h"
#include "avl.h"
#include "globals.h"
#include "agheader.h"
#include "incore.h"
#include "protos.h"
#include "err_protos.h"
#include "dinode.h"
#include "rt.h"
#include "versions.h"
#include "threads.h"
#include "progress.h"
#include "slab.h"
#include "rmap.h"

/*
 * we maintain the current slice (path from root to leaf)
 * of the btree incore.  when we need a new block, we ask
 * the block allocator for the address of a block on that
 * level, map the block in, and set up the appropriate
 * pointers (child, silbing, etc.) and keys that should
 * point to the new block.
 */
typedef struct bt_stat_level  {
	/*
	 * set in setup_cursor routine and maintained in the tree-building
	 * routines
	 */
	xfs_buf_t		*buf_p;		/* 2 buffer pointers to ... */
	xfs_buf_t		*prev_buf_p;
	xfs_agblock_t		agbno;		/* current block being filled */
	xfs_agblock_t		prev_agbno;	/* previous block */
	/*
	 * set in calculate/init cursor routines for each btree level
	 */
	int			num_recs_tot;	/* # tree recs in level */
	int			num_blocks;	/* # tree blocks in level */
	int			num_recs_pb;	/* num_recs_tot / num_blocks */
	int			modulo;		/* num_recs_tot % num_blocks */
} bt_stat_level_t;

typedef struct bt_status  {
	int			init;		/* cursor set up once? */
	int			num_levels;	/* # of levels in btree */
	xfs_extlen_t		num_tot_blocks;	/* # blocks alloc'ed for tree */
	xfs_extlen_t		num_free_blocks;/* # blocks currently unused */

	xfs_agblock_t		root;		/* root block */
	/*
	 * list of blocks to be used to set up this tree
	 * and pointer to the first unused block on the list
	 */
	xfs_agblock_t		*btree_blocks;		/* block list */
	xfs_agblock_t		*free_btree_blocks;	/* first unused block */
	/*
	 * per-level status info
	 */
	bt_stat_level_t		level[XFS_BTREE_MAXLEVELS];
	uint64_t		owner;		/* owner */
} bt_status_t;

/*
 * extra metadata for the agi
 */
struct agi_stat {
	xfs_agino_t		first_agino;
	xfs_agino_t		count;
	xfs_agino_t		freecount;
};

static uint64_t	*sb_icount_ag;		/* allocated inodes per ag */
static uint64_t	*sb_ifree_ag;		/* free inodes per ag */
static uint64_t	*sb_fdblocks_ag;	/* free data blocks per ag */

static int
mk_incore_fstree(xfs_mount_t *mp, xfs_agnumber_t agno)
{
	int			in_extent;
	int			num_extents;
	xfs_agblock_t		extent_start;
	xfs_extlen_t		extent_len;
	xfs_agblock_t		agbno;
	xfs_agblock_t		ag_end;
	uint			free_blocks;
	xfs_extlen_t		blen;
	int			bstate;

	/*
	 * scan the bitmap for the ag looking for continuous
	 * extents of free blocks.  At this point, we know
	 * that blocks in the bitmap are either set to an
	 * "in use" state or set to unknown (0) since the
	 * bmaps were zero'ed in phase 4 and only blocks
	 * being used by inodes, inode bmaps, ag headers,
	 * and the files themselves were put into the bitmap.
	 *
	 */
	ASSERT(agno < mp->m_sb.sb_agcount);

	extent_start = extent_len = 0;
	in_extent = 0;
	num_extents = free_blocks = 0;

	if (agno < mp->m_sb.sb_agcount - 1)
		ag_end = mp->m_sb.sb_agblocks;
	else
		ag_end = mp->m_sb.sb_dblocks -
			(xfs_rfsblock_t)mp->m_sb.sb_agblocks *
                       (mp->m_sb.sb_agcount - 1);

	/*
	 * ok, now find the number of extents, keep track of the
	 * largest extent.
	 */
	for (agbno = 0; agbno < ag_end; agbno += blen) {
		bstate = get_bmap_ext(agno, agbno, ag_end, &blen);
		if (bstate < XR_E_INUSE)  {
			free_blocks += blen;
			if (in_extent == 0)  {
				/*
				 * found the start of a free extent
				 */
				in_extent = 1;
				num_extents++;
				extent_start = agbno;
				extent_len = blen;
			} else  {
				extent_len += blen;
			}
		} else   {
			if (in_extent)  {
				/*
				 * free extent ends here, add extent to the
				 * 2 incore extent (avl-to-be-B+) trees
				 */
				in_extent = 0;
#if defined(XR_BLD_FREE_TRACE) && defined(XR_BLD_ADD_EXTENT)
				fprintf(stderr, "adding extent %u [%u %u]\n",
					agno, extent_start, extent_len);
#endif
				add_bno_extent(agno, extent_start, extent_len);
				add_bcnt_extent(agno, extent_start, extent_len);
			}
		}
	}
	if (in_extent)  {
		/*
		 * free extent ends here
		 */
#if defined(XR_BLD_FREE_TRACE) && defined(XR_BLD_ADD_EXTENT)
		fprintf(stderr, "adding extent %u [%u %u]\n",
			agno, extent_start, extent_len);
#endif
		add_bno_extent(agno, extent_start, extent_len);
		add_bcnt_extent(agno, extent_start, extent_len);
	}

	return(num_extents);
}

static xfs_agblock_t
get_next_blockaddr(xfs_agnumber_t agno, int level, bt_status_t *curs)
{
	ASSERT(curs->free_btree_blocks < curs->btree_blocks +
						curs->num_tot_blocks);
	ASSERT(curs->num_free_blocks > 0);

	curs->num_free_blocks--;
	return(*curs->free_btree_blocks++);
}

/*
 * set up the dynamically allocated block allocation data in the btree
 * cursor that depends on the info in the static portion of the cursor.
 * allocates space from the incore bno/bcnt extent trees and sets up
 * the first path up the left side of the tree.  Also sets up the
 * cursor pointer to the btree root.   called by init_freespace_cursor()
 * and init_ino_cursor()
 */
static void
setup_cursor(xfs_mount_t *mp, xfs_agnumber_t agno, bt_status_t *curs)
{
	int			j;
	unsigned int		u;
	xfs_extlen_t		big_extent_len;
	xfs_agblock_t		big_extent_start;
	extent_tree_node_t	*ext_ptr;
	extent_tree_node_t	*bno_ext_ptr;
	xfs_extlen_t		blocks_allocated;
	xfs_agblock_t		*agb_ptr;
	int			error;

	/*
	 * get the number of blocks we need to allocate, then
	 * set up block number array, set the free block pointer
	 * to the first block in the array, and null the array
	 */
	big_extent_len = curs->num_tot_blocks;
	blocks_allocated = 0;

	ASSERT(big_extent_len > 0);

	if ((curs->btree_blocks = malloc(sizeof(xfs_agblock_t)
					* big_extent_len)) == NULL)
		do_error(_("could not set up btree block array\n"));

	agb_ptr = curs->free_btree_blocks = curs->btree_blocks;

	for (j = 0; j < curs->num_free_blocks; j++, agb_ptr++)
		*agb_ptr = NULLAGBLOCK;

	/*
	 * grab the smallest extent and use it up, then get the
	 * next smallest.  This mimics the init_*_cursor code.
	 */
	ext_ptr =  findfirst_bcnt_extent(agno);

	agb_ptr = curs->btree_blocks;

	/*
	 * set up the free block array
	 */
	while (blocks_allocated < big_extent_len)  {
		if (!ext_ptr)
			do_error(
_("error - not enough free space in filesystem\n"));
		/*
		 * use up the extent we've got
		 */
		for (u = 0; u < ext_ptr->ex_blockcount &&
				blocks_allocated < big_extent_len; u++)  {
			ASSERT(agb_ptr < curs->btree_blocks
					+ curs->num_tot_blocks);
			*agb_ptr++ = ext_ptr->ex_startblock + u;
			blocks_allocated++;
		}

		error = rmap_add_ag_rec(mp, agno, ext_ptr->ex_startblock, u,
				curs->owner);
		if (error)
			do_error(_("could not set up btree rmaps: %s\n"),
				strerror(-error));

		/*
		 * if we only used part of this last extent, then we
		 * need only to reset the extent in the extent
		 * trees and we're done
		 */
		if (u < ext_ptr->ex_blockcount)  {
			big_extent_start = ext_ptr->ex_startblock + u;
			big_extent_len = ext_ptr->ex_blockcount - u;

			ASSERT(big_extent_len > 0);

			bno_ext_ptr = find_bno_extent(agno,
						ext_ptr->ex_startblock);
			ASSERT(bno_ext_ptr != NULL);
			get_bno_extent(agno, bno_ext_ptr);
			release_extent_tree_node(bno_ext_ptr);

			ext_ptr = get_bcnt_extent(agno, ext_ptr->ex_startblock,
					ext_ptr->ex_blockcount);
			release_extent_tree_node(ext_ptr);
#ifdef XR_BLD_FREE_TRACE
			fprintf(stderr, "releasing extent: %u [%u %u]\n",
				agno, ext_ptr->ex_startblock,
				ext_ptr->ex_blockcount);
			fprintf(stderr, "blocks_allocated = %d\n",
				blocks_allocated);
#endif

			add_bno_extent(agno, big_extent_start, big_extent_len);
			add_bcnt_extent(agno, big_extent_start, big_extent_len);

			return;
		}
		/*
		 * delete the used-up extent from both extent trees and
		 * find next biggest extent
		 */
#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, "releasing extent: %u [%u %u]\n",
			agno, ext_ptr->ex_startblock, ext_ptr->ex_blockcount);
#endif
		bno_ext_ptr = find_bno_extent(agno, ext_ptr->ex_startblock);
		ASSERT(bno_ext_ptr != NULL);
		get_bno_extent(agno, bno_ext_ptr);
		release_extent_tree_node(bno_ext_ptr);

		ext_ptr = get_bcnt_extent(agno, ext_ptr->ex_startblock,
				ext_ptr->ex_blockcount);
		ASSERT(ext_ptr != NULL);
		release_extent_tree_node(ext_ptr);

		ext_ptr = findfirst_bcnt_extent(agno);
	}
#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "blocks_allocated = %d\n",
		blocks_allocated);
#endif
}

static void
write_cursor(bt_status_t *curs)
{
	int i;

	for (i = 0; i < curs->num_levels; i++)  {
#if defined(XR_BLD_FREE_TRACE) || defined(XR_BLD_INO_TRACE)
		fprintf(stderr, "writing bt block %u\n", curs->level[i].agbno);
#endif
		if (curs->level[i].prev_buf_p != NULL)  {
			ASSERT(curs->level[i].prev_agbno != NULLAGBLOCK);
#if defined(XR_BLD_FREE_TRACE) || defined(XR_BLD_INO_TRACE)
			fprintf(stderr, "writing bt prev block %u\n",
						curs->level[i].prev_agbno);
#endif
			libxfs_writebuf(curs->level[i].prev_buf_p, 0);
		}
		libxfs_writebuf(curs->level[i].buf_p, 0);
	}
}

static void
finish_cursor(bt_status_t *curs)
{
	ASSERT(curs->num_free_blocks == 0);
	free(curs->btree_blocks);
}

/*
 * We need to leave some free records in the tree for the corner case of
 * setting up the AGFL. This may require allocation of blocks, and as
 * such can require insertion of new records into the tree (e.g. moving
 * a record in the by-count tree when a long extent is shortened). If we
 * pack the records into the leaves with no slack space, this requires a
 * leaf split to occur and a block to be allocated from the free list.
 * If we don't have any blocks on the free list (because we are setting
 * it up!), then we fail, and the filesystem will fail with the same
 * failure at runtime. Hence leave a couple of records slack space in
 * each block to allow immediate modification of the tree without
 * requiring splits to be done.
 *
 * XXX(hch): any reason we don't just look at mp->m_alloc_mxr?
 */
#define XR_ALLOC_BLOCK_MAXRECS(mp, level) \
	(libxfs_allocbt_maxrecs((mp), (mp)->m_sb.sb_blocksize, (level) == 0) - 2)

/*
 * this calculates a freespace cursor for an ag.
 * btree_curs is an in/out.  returns the number of
 * blocks that will show up in the AGFL.
 */
static int
calculate_freespace_cursor(xfs_mount_t *mp, xfs_agnumber_t agno,
			xfs_agblock_t *extents, bt_status_t *btree_curs)
{
	xfs_extlen_t		blocks_needed;		/* a running count */
	xfs_extlen_t		blocks_allocated_pt;	/* per tree */
	xfs_extlen_t		blocks_allocated_total;	/* for both trees */
	xfs_agblock_t		num_extents;
	int			i;
	int			extents_used;
	int			extra_blocks;
	bt_stat_level_t		*lptr;
	bt_stat_level_t		*p_lptr;
	extent_tree_node_t	*ext_ptr;
	int			level;

	num_extents = *extents;
	extents_used = 0;

	ASSERT(num_extents != 0);

	lptr = &btree_curs->level[0];
	btree_curs->init = 1;

	/*
	 * figure out how much space we need for the leaf level
	 * of the tree and set up the cursor for the leaf level
	 * (note that the same code is duplicated further down)
	 */
	lptr->num_blocks = howmany(num_extents, XR_ALLOC_BLOCK_MAXRECS(mp, 0));
	lptr->num_recs_pb = num_extents / lptr->num_blocks;
	lptr->modulo = num_extents % lptr->num_blocks;
	lptr->num_recs_tot = num_extents;
	level = 1;

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "%s 0 %d %d %d %d\n", __func__,
			lptr->num_blocks,
			lptr->num_recs_pb,
			lptr->modulo,
			lptr->num_recs_tot);
#endif
	/*
	 * if we need more levels, set them up.  # of records
	 * per level is the # of blocks in the level below it
	 */
	if (lptr->num_blocks > 1)  {
		for (; btree_curs->level[level - 1].num_blocks > 1
				&& level < XFS_BTREE_MAXLEVELS;
				level++)  {
			lptr = &btree_curs->level[level];
			p_lptr = &btree_curs->level[level - 1];
			lptr->num_blocks = howmany(p_lptr->num_blocks,
					XR_ALLOC_BLOCK_MAXRECS(mp, level));
			lptr->modulo = p_lptr->num_blocks
					% lptr->num_blocks;
			lptr->num_recs_pb = p_lptr->num_blocks
					/ lptr->num_blocks;
			lptr->num_recs_tot = p_lptr->num_blocks;
#ifdef XR_BLD_FREE_TRACE
			fprintf(stderr, "%s %d %d %d %d %d\n", __func__,
					level,
					lptr->num_blocks,
					lptr->num_recs_pb,
					lptr->modulo,
					lptr->num_recs_tot);
#endif
		}
	}

	ASSERT(lptr->num_blocks == 1);
	btree_curs->num_levels = level;

	/*
	 * ok, now we have a hypothetical cursor that
	 * will work for both the bno and bcnt trees.
	 * now figure out if using up blocks to set up the
	 * trees will perturb the shape of the freespace tree.
	 * if so, we've over-allocated.  the freespace trees
	 * as they will be *after* accounting for the free space
	 * we've used up will need fewer blocks to to represent
	 * than we've allocated.  We can use the AGFL to hold
	 * XFS_AGFL_SIZE (sector/xfs_agfl_t) blocks but that's it.
	 * Thus we limit things to XFS_AGFL_SIZE/2 for each of the 2 btrees.
	 * if the number of extra blocks is more than that,
	 * we'll have to be called again.
	 */
	for (blocks_needed = 0, i = 0; i < level; i++)  {
		blocks_needed += btree_curs->level[i].num_blocks;
	}

	/*
	 * record the # of blocks we've allocated
	 */
	blocks_allocated_pt = blocks_needed;
	blocks_needed *= 2;
	blocks_allocated_total = blocks_needed;

	/*
	 * figure out how many free extents will be used up by
	 * our space allocation
	 */
	if ((ext_ptr = findfirst_bcnt_extent(agno)) == NULL)
		do_error(_("can't rebuild fs trees -- not enough free space "
			   "on ag %u\n"), agno);

	while (ext_ptr != NULL && blocks_needed > 0)  {
		if (ext_ptr->ex_blockcount <= blocks_needed)  {
			blocks_needed -= ext_ptr->ex_blockcount;
			extents_used++;
		} else  {
			blocks_needed = 0;
		}

		ext_ptr = findnext_bcnt_extent(agno, ext_ptr);

#ifdef XR_BLD_FREE_TRACE
		if (ext_ptr != NULL)  {
			fprintf(stderr, "got next extent [%u %u]\n",
				ext_ptr->ex_startblock, ext_ptr->ex_blockcount);
		} else  {
			fprintf(stderr, "out of extents\n");
		}
#endif
	}
	if (blocks_needed > 0)
		do_error(_("ag %u - not enough free space to build freespace "
			   "btrees\n"), agno);

	ASSERT(num_extents >= extents_used);

	num_extents -= extents_used;

	/*
	 * see if the number of leaf blocks will change as a result
	 * of the number of extents changing
	 */
	if (howmany(num_extents, XR_ALLOC_BLOCK_MAXRECS(mp, 0))
			!= btree_curs->level[0].num_blocks)  {
		/*
		 * yes -- recalculate the cursor.  If the number of
		 * excess (overallocated) blocks is < XFS_AGFL_SIZE/2, we're ok.
		 * we can put those into the AGFL.  we don't try
		 * and get things to converge exactly (reach a
		 * state with zero excess blocks) because there
		 * exist pathological cases which will never
		 * converge.  first, check for the zero-case.
		 */
		if (num_extents == 0)  {
			/*
			 * ok, we've used up all the free blocks
			 * trying to lay out the leaf level. go
			 * to a one block (empty) btree and put the
			 * already allocated blocks into the AGFL
			 */
			if (btree_curs->level[0].num_blocks != 1)  {
				/*
				 * we really needed more blocks because
				 * the old tree had more than one level.
				 * this is bad.
				 */
				 do_warn(_("not enough free blocks left to "
					   "describe all free blocks in AG "
					   "%u\n"), agno);
			}
#ifdef XR_BLD_FREE_TRACE
			fprintf(stderr,
				"ag %u -- no free extents, alloc'ed %d\n",
				agno, blocks_allocated_pt);
#endif
			lptr->num_blocks = 1;
			lptr->modulo = 0;
			lptr->num_recs_pb = 0;
			lptr->num_recs_tot = 0;

			btree_curs->num_levels = 1;

			/*
			 * don't reset the allocation stats, assume
			 * they're all extra blocks
			 * don't forget to return the total block count
			 * not the per-tree block count.  these are the
			 * extras that will go into the AGFL.  subtract
			 * two for the root blocks.
			 */
			btree_curs->num_tot_blocks = blocks_allocated_pt;
			btree_curs->num_free_blocks = blocks_allocated_pt;

			*extents = 0;

			return(blocks_allocated_total - 2);
		}

		lptr = &btree_curs->level[0];
		lptr->num_blocks = howmany(num_extents,
					XR_ALLOC_BLOCK_MAXRECS(mp, 0));
		lptr->num_recs_pb = num_extents / lptr->num_blocks;
		lptr->modulo = num_extents % lptr->num_blocks;
		lptr->num_recs_tot = num_extents;
		level = 1;

		/*
		 * if we need more levels, set them up
		 */
		if (lptr->num_blocks > 1)  {
			for (level = 1; btree_curs->level[level-1].num_blocks
					> 1 && level < XFS_BTREE_MAXLEVELS;
					level++)  {
				lptr = &btree_curs->level[level];
				p_lptr = &btree_curs->level[level-1];
				lptr->num_blocks = howmany(p_lptr->num_blocks,
					XR_ALLOC_BLOCK_MAXRECS(mp, level));
				lptr->modulo = p_lptr->num_blocks
						% lptr->num_blocks;
				lptr->num_recs_pb = p_lptr->num_blocks
						/ lptr->num_blocks;
				lptr->num_recs_tot = p_lptr->num_blocks;
			}
		}
		ASSERT(lptr->num_blocks == 1);
		btree_curs->num_levels = level;

		/*
		 * now figure out the number of excess blocks
		 */
		for (blocks_needed = 0, i = 0; i < level; i++)  {
			blocks_needed += btree_curs->level[i].num_blocks;
		}
		blocks_needed *= 2;

		ASSERT(blocks_allocated_total >= blocks_needed);
		extra_blocks = blocks_allocated_total - blocks_needed;
	} else  {
		if (extents_used > 0) {
			/*
			 * reset the leaf level geometry to account
			 * for consumed extents.  we can leave the
			 * rest of the cursor alone since the number
			 * of leaf blocks hasn't changed.
			 */
			lptr = &btree_curs->level[0];

			lptr->num_recs_pb = num_extents / lptr->num_blocks;
			lptr->modulo = num_extents % lptr->num_blocks;
			lptr->num_recs_tot = num_extents;
		}

		extra_blocks = 0;
	}

	btree_curs->num_tot_blocks = blocks_allocated_pt;
	btree_curs->num_free_blocks = blocks_allocated_pt;

	*extents = num_extents;

	return(extra_blocks);
}

static void
prop_freespace_cursor(xfs_mount_t *mp, xfs_agnumber_t agno,
		bt_status_t *btree_curs, xfs_agblock_t startblock,
		xfs_extlen_t blockcount, int level, xfs_btnum_t btnum)
{
	struct xfs_btree_block	*bt_hdr;
	xfs_alloc_key_t		*bt_key;
	xfs_alloc_ptr_t		*bt_ptr;
	xfs_agblock_t		agbno;
	bt_stat_level_t		*lptr;

	ASSERT(btnum == XFS_BTNUM_BNO || btnum == XFS_BTNUM_CNT);

	level++;

	if (level >= btree_curs->num_levels)
		return;

	lptr = &btree_curs->level[level];
	bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);

	if (be16_to_cpu(bt_hdr->bb_numrecs) == 0)  {
		/*
		 * only happens once when initializing the
		 * left-hand side of the tree.
		 */
		prop_freespace_cursor(mp, agno, btree_curs, startblock,
				blockcount, level, btnum);
	}

	if (be16_to_cpu(bt_hdr->bb_numrecs) ==
				lptr->num_recs_pb + (lptr->modulo > 0))  {
		/*
		 * write out current prev block, grab us a new block,
		 * and set the rightsib pointer of current block
		 */
#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, " %d ", lptr->prev_agbno);
#endif
		if (lptr->prev_agbno != NULLAGBLOCK) {
			ASSERT(lptr->prev_buf_p != NULL);
			libxfs_writebuf(lptr->prev_buf_p, 0);
		}
		lptr->prev_agbno = lptr->agbno;;
		lptr->prev_buf_p = lptr->buf_p;
		agbno = get_next_blockaddr(agno, level, btree_curs);

		bt_hdr->bb_u.s.bb_rightsib = cpu_to_be32(agbno);

		lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, agbno),
					XFS_FSB_TO_BB(mp, 1));
		lptr->agbno = agbno;

		if (lptr->modulo)
			lptr->modulo--;

		/*
		 * initialize block header
		 */
		lptr->buf_p->b_ops = &xfs_allocbt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, btnum, level,
					0, agno, 0);

		bt_hdr->bb_u.s.bb_leftsib = cpu_to_be32(lptr->prev_agbno);

		/*
		 * propagate extent record for first extent in new block up
		 */
		prop_freespace_cursor(mp, agno, btree_curs, startblock,
				blockcount, level, btnum);
	}
	/*
	 * add extent info to current block
	 */
	be16_add_cpu(&bt_hdr->bb_numrecs, 1);

	bt_key = XFS_ALLOC_KEY_ADDR(mp, bt_hdr,
				be16_to_cpu(bt_hdr->bb_numrecs));
	bt_ptr = XFS_ALLOC_PTR_ADDR(mp, bt_hdr,
				be16_to_cpu(bt_hdr->bb_numrecs),
				mp->m_alloc_mxr[1]);

	bt_key->ar_startblock = cpu_to_be32(startblock);
	bt_key->ar_blockcount = cpu_to_be32(blockcount);
	*bt_ptr = cpu_to_be32(btree_curs->level[level-1].agbno);
}

/*
 * rebuilds a freespace tree given a cursor and type
 * of tree to build (bno or bcnt).  returns the number of free blocks
 * represented by the tree.
 */
static xfs_extlen_t
build_freespace_tree(xfs_mount_t *mp, xfs_agnumber_t agno,
		bt_status_t *btree_curs, xfs_btnum_t btnum)
{
	xfs_agnumber_t		i;
	xfs_agblock_t		j;
	struct xfs_btree_block	*bt_hdr;
	xfs_alloc_rec_t		*bt_rec;
	int			level;
	xfs_agblock_t		agbno;
	extent_tree_node_t	*ext_ptr;
	bt_stat_level_t		*lptr;
	xfs_extlen_t		freeblks;

	ASSERT(btnum == XFS_BTNUM_BNO || btnum == XFS_BTNUM_CNT);

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "in build_freespace_tree, agno = %d\n", agno);
#endif
	level = btree_curs->num_levels;
	freeblks = 0;

	ASSERT(level > 0);

	/*
	 * initialize the first block on each btree level
	 */
	for (i = 0; i < level; i++)  {
		lptr = &btree_curs->level[i];

		agbno = get_next_blockaddr(agno, i, btree_curs);
		lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, agbno),
					XFS_FSB_TO_BB(mp, 1));

		if (i == btree_curs->num_levels - 1)
			btree_curs->root = agbno;

		lptr->agbno = agbno;
		lptr->prev_agbno = NULLAGBLOCK;
		lptr->prev_buf_p = NULL;
		/*
		 * initialize block header
		 */
		lptr->buf_p->b_ops = &xfs_allocbt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, btnum, i, 0, agno, 0);
	}
	/*
	 * run along leaf, setting up records.  as we have to switch
	 * blocks, call the prop_freespace_cursor routine to set up the new
	 * pointers for the parent.  that can recurse up to the root
	 * if required.  set the sibling pointers for leaf level here.
	 */
	if (btnum == XFS_BTNUM_BNO)
		ext_ptr = findfirst_bno_extent(agno);
	else
		ext_ptr = findfirst_bcnt_extent(agno);

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "bft, agno = %d, start = %u, count = %u\n",
		agno, ext_ptr->ex_startblock, ext_ptr->ex_blockcount);
#endif

	lptr = &btree_curs->level[0];

	for (i = 0; i < btree_curs->level[0].num_blocks; i++)  {
		/*
		 * block initialization, lay in block header
		 */
		lptr->buf_p->b_ops = &xfs_allocbt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, btnum, 0, 0, agno, 0);

		bt_hdr->bb_u.s.bb_leftsib = cpu_to_be32(lptr->prev_agbno);
		bt_hdr->bb_numrecs = cpu_to_be16(lptr->num_recs_pb +
							(lptr->modulo > 0));
#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, "bft, bb_numrecs = %d\n",
				be16_to_cpu(bt_hdr->bb_numrecs));
#endif

		if (lptr->modulo > 0)
			lptr->modulo--;

		/*
		 * initialize values in the path up to the root if
		 * this is a multi-level btree
		 */
		if (btree_curs->num_levels > 1)
			prop_freespace_cursor(mp, agno, btree_curs,
					ext_ptr->ex_startblock,
					ext_ptr->ex_blockcount,
					0, btnum);

		bt_rec = (xfs_alloc_rec_t *)
			  ((char *)bt_hdr + XFS_ALLOC_BLOCK_LEN(mp));
		for (j = 0; j < be16_to_cpu(bt_hdr->bb_numrecs); j++) {
			ASSERT(ext_ptr != NULL);
			bt_rec[j].ar_startblock = cpu_to_be32(
							ext_ptr->ex_startblock);
			bt_rec[j].ar_blockcount = cpu_to_be32(
							ext_ptr->ex_blockcount);
			freeblks += ext_ptr->ex_blockcount;
			if (btnum == XFS_BTNUM_BNO)
				ext_ptr = findnext_bno_extent(ext_ptr);
			else
				ext_ptr = findnext_bcnt_extent(agno, ext_ptr);
#if 0
#ifdef XR_BLD_FREE_TRACE
			if (ext_ptr == NULL)
				fprintf(stderr, "null extent pointer, j = %d\n",
					j);
			else
				fprintf(stderr,
				"bft, agno = %d, start = %u, count = %u\n",
					agno, ext_ptr->ex_startblock,
					ext_ptr->ex_blockcount);
#endif
#endif
		}

		if (ext_ptr != NULL)  {
			/*
			 * get next leaf level block
			 */
			if (lptr->prev_buf_p != NULL)  {
#ifdef XR_BLD_FREE_TRACE
				fprintf(stderr, " writing fst agbno %u\n",
					lptr->prev_agbno);
#endif
				ASSERT(lptr->prev_agbno != NULLAGBLOCK);
				libxfs_writebuf(lptr->prev_buf_p, 0);
			}
			lptr->prev_buf_p = lptr->buf_p;
			lptr->prev_agbno = lptr->agbno;
			lptr->agbno = get_next_blockaddr(agno, 0, btree_curs);
			bt_hdr->bb_u.s.bb_rightsib = cpu_to_be32(lptr->agbno);

			lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, lptr->agbno),
					XFS_FSB_TO_BB(mp, 1));
		}
	}

	return(freeblks);
}

/*
 * XXX(hch): any reason we don't just look at mp->m_inobt_mxr?
 */
#define XR_INOBT_BLOCK_MAXRECS(mp, level) \
			libxfs_inobt_maxrecs((mp), (mp)->m_sb.sb_blocksize, \
						(level) == 0)

/*
 * we don't have to worry here about how chewing up free extents
 * may perturb things because inode tree building happens before
 * freespace tree building.
 */
static void
init_ino_cursor(xfs_mount_t *mp, xfs_agnumber_t agno, bt_status_t *btree_curs,
		uint64_t *num_inos, uint64_t *num_free_inos, int finobt)
{
	uint64_t		ninos;
	uint64_t		nfinos;
	int			rec_nfinos;
	int			rec_ninos;
	ino_tree_node_t		*ino_rec;
	int			num_recs;
	int			level;
	bt_stat_level_t		*lptr;
	bt_stat_level_t		*p_lptr;
	xfs_extlen_t		blocks_allocated;
	int			i;

	*num_inos = *num_free_inos = 0;
	ninos = nfinos = 0;

	lptr = &btree_curs->level[0];
	btree_curs->init = 1;
	btree_curs->owner = XFS_RMAP_OWN_INOBT;

	/*
	 * build up statistics
	 */
	ino_rec = findfirst_inode_rec(agno);
	for (num_recs = 0; ino_rec != NULL; ino_rec = next_ino_rec(ino_rec))  {
		rec_ninos = 0;
		rec_nfinos = 0;
		for (i = 0; i < XFS_INODES_PER_CHUNK; i++)  {
			ASSERT(is_inode_confirmed(ino_rec, i));
			/*
			 * sparse inodes are not factored into superblock (free)
			 * inode counts
			 */
			if (is_inode_sparse(ino_rec, i))
				continue;
			if (is_inode_free(ino_rec, i))
				rec_nfinos++;
			rec_ninos++;
		}

		/*
		 * finobt only considers records with free inodes
		 */
		if (finobt && !rec_nfinos)
			continue;

		nfinos += rec_nfinos;
		ninos += rec_ninos;
		num_recs++;
	}

	if (num_recs == 0) {
		/*
		 * easy corner-case -- no inode records
		 */
		lptr->num_blocks = 1;
		lptr->modulo = 0;
		lptr->num_recs_pb = 0;
		lptr->num_recs_tot = 0;

		btree_curs->num_levels = 1;
		btree_curs->num_tot_blocks = btree_curs->num_free_blocks = 1;

		setup_cursor(mp, agno, btree_curs);

		return;
	}

	blocks_allocated = lptr->num_blocks = howmany(num_recs,
					XR_INOBT_BLOCK_MAXRECS(mp, 0));

	lptr->modulo = num_recs % lptr->num_blocks;
	lptr->num_recs_pb = num_recs / lptr->num_blocks;
	lptr->num_recs_tot = num_recs;
	level = 1;

	if (lptr->num_blocks > 1)  {
		for (; btree_curs->level[level-1].num_blocks > 1
				&& level < XFS_BTREE_MAXLEVELS;
				level++)  {
			lptr = &btree_curs->level[level];
			p_lptr = &btree_curs->level[level - 1];
			lptr->num_blocks = howmany(p_lptr->num_blocks,
				XR_INOBT_BLOCK_MAXRECS(mp, level));
			lptr->modulo = p_lptr->num_blocks % lptr->num_blocks;
			lptr->num_recs_pb = p_lptr->num_blocks
					/ lptr->num_blocks;
			lptr->num_recs_tot = p_lptr->num_blocks;

			blocks_allocated += lptr->num_blocks;
		}
	}
	ASSERT(lptr->num_blocks == 1);
	btree_curs->num_levels = level;

	btree_curs->num_tot_blocks = btree_curs->num_free_blocks
			= blocks_allocated;

	setup_cursor(mp, agno, btree_curs);

	*num_inos = ninos;
	*num_free_inos = nfinos;

	return;
}

static void
prop_ino_cursor(xfs_mount_t *mp, xfs_agnumber_t agno, bt_status_t *btree_curs,
	xfs_agino_t startino, int level)
{
	struct xfs_btree_block	*bt_hdr;
	xfs_inobt_key_t		*bt_key;
	xfs_inobt_ptr_t		*bt_ptr;
	xfs_agblock_t		agbno;
	bt_stat_level_t		*lptr;

	level++;

	if (level >= btree_curs->num_levels)
		return;

	lptr = &btree_curs->level[level];
	bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);

	if (be16_to_cpu(bt_hdr->bb_numrecs) == 0)  {
		/*
		 * this only happens once to initialize the
		 * first path up the left side of the tree
		 * where the agbno's are already set up
		 */
		prop_ino_cursor(mp, agno, btree_curs, startino, level);
	}

	if (be16_to_cpu(bt_hdr->bb_numrecs) ==
				lptr->num_recs_pb + (lptr->modulo > 0))  {
		/*
		 * write out current prev block, grab us a new block,
		 * and set the rightsib pointer of current block
		 */
#ifdef XR_BLD_INO_TRACE
		fprintf(stderr, " ino prop agbno %d ", lptr->prev_agbno);
#endif
		if (lptr->prev_agbno != NULLAGBLOCK)  {
			ASSERT(lptr->prev_buf_p != NULL);
			libxfs_writebuf(lptr->prev_buf_p, 0);
		}
		lptr->prev_agbno = lptr->agbno;;
		lptr->prev_buf_p = lptr->buf_p;
		agbno = get_next_blockaddr(agno, level, btree_curs);

		bt_hdr->bb_u.s.bb_rightsib = cpu_to_be32(agbno);

		lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, agbno),
					XFS_FSB_TO_BB(mp, 1));
		lptr->agbno = agbno;

		if (lptr->modulo)
			lptr->modulo--;

		/*
		 * initialize block header
		 */
		lptr->buf_p->b_ops = &xfs_inobt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, XFS_BTNUM_INO,
					level, 0, agno, 0);

		bt_hdr->bb_u.s.bb_leftsib = cpu_to_be32(lptr->prev_agbno);

		/*
		 * propagate extent record for first extent in new block up
		 */
		prop_ino_cursor(mp, agno, btree_curs, startino, level);
	}
	/*
	 * add inode info to current block
	 */
	be16_add_cpu(&bt_hdr->bb_numrecs, 1);

	bt_key = XFS_INOBT_KEY_ADDR(mp, bt_hdr,
				    be16_to_cpu(bt_hdr->bb_numrecs));
	bt_ptr = XFS_INOBT_PTR_ADDR(mp, bt_hdr,
				    be16_to_cpu(bt_hdr->bb_numrecs),
				    mp->m_inobt_mxr[1]);

	bt_key->ir_startino = cpu_to_be32(startino);
	*bt_ptr = cpu_to_be32(btree_curs->level[level-1].agbno);
}

/*
 * XXX: yet more code that can be shared with mkfs, growfs.
 */
static void
build_agi(xfs_mount_t *mp, xfs_agnumber_t agno, bt_status_t *btree_curs,
		bt_status_t *finobt_curs, struct agi_stat *agi_stat)
{
	xfs_buf_t	*agi_buf;
	xfs_agi_t	*agi;
	int		i;

	agi_buf = libxfs_getbuf(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGI_DADDR(mp)),
			mp->m_sb.sb_sectsize/BBSIZE);
	agi_buf->b_ops = &xfs_agi_buf_ops;
	agi = XFS_BUF_TO_AGI(agi_buf);
	memset(agi, 0, mp->m_sb.sb_sectsize);

	agi->agi_magicnum = cpu_to_be32(XFS_AGI_MAGIC);
	agi->agi_versionnum = cpu_to_be32(XFS_AGI_VERSION);
	agi->agi_seqno = cpu_to_be32(agno);
	if (agno < mp->m_sb.sb_agcount - 1)
		agi->agi_length = cpu_to_be32(mp->m_sb.sb_agblocks);
	else
		agi->agi_length = cpu_to_be32(mp->m_sb.sb_dblocks -
			(xfs_rfsblock_t) mp->m_sb.sb_agblocks * agno);
	agi->agi_count = cpu_to_be32(agi_stat->count);
	agi->agi_root = cpu_to_be32(btree_curs->root);
	agi->agi_level = cpu_to_be32(btree_curs->num_levels);
	agi->agi_freecount = cpu_to_be32(agi_stat->freecount);
	agi->agi_newino = cpu_to_be32(agi_stat->first_agino);
	agi->agi_dirino = cpu_to_be32(NULLAGINO);

	for (i = 0; i < XFS_AGI_UNLINKED_BUCKETS; i++)
		agi->agi_unlinked[i] = cpu_to_be32(NULLAGINO);

	if (xfs_sb_version_hascrc(&mp->m_sb))
		platform_uuid_copy(&agi->agi_uuid, &mp->m_sb.sb_meta_uuid);

	if (xfs_sb_version_hasfinobt(&mp->m_sb)) {
		agi->agi_free_root = cpu_to_be32(finobt_curs->root);
		agi->agi_free_level = cpu_to_be32(finobt_curs->num_levels);
	}

	libxfs_writebuf(agi_buf, 0);
}

/*
 * rebuilds an inode tree given a cursor.  We're lazy here and call
 * the routine that builds the agi
 */
static void
build_ino_tree(xfs_mount_t *mp, xfs_agnumber_t agno,
		bt_status_t *btree_curs, xfs_btnum_t btnum,
		struct agi_stat *agi_stat)
{
	xfs_agnumber_t		i;
	xfs_agblock_t		j;
	xfs_agblock_t		agbno;
	xfs_agino_t		first_agino;
	struct xfs_btree_block	*bt_hdr;
	xfs_inobt_rec_t		*bt_rec;
	ino_tree_node_t		*ino_rec;
	bt_stat_level_t		*lptr;
	xfs_agino_t		count = 0;
	xfs_agino_t		freecount = 0;
	int			inocnt;
	uint8_t			finocnt;
	int			k;
	int			level = btree_curs->num_levels;
	int			spmask;
	uint64_t		sparse;
	uint16_t		holemask;

	ASSERT(btnum == XFS_BTNUM_INO || btnum == XFS_BTNUM_FINO);

	for (i = 0; i < level; i++)  {
		lptr = &btree_curs->level[i];

		agbno = get_next_blockaddr(agno, i, btree_curs);
		lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, agbno),
					XFS_FSB_TO_BB(mp, 1));

		if (i == btree_curs->num_levels - 1)
			btree_curs->root = agbno;

		lptr->agbno = agbno;
		lptr->prev_agbno = NULLAGBLOCK;
		lptr->prev_buf_p = NULL;
		/*
		 * initialize block header
		 */

		lptr->buf_p->b_ops = &xfs_inobt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, btnum, i, 0, agno, 0);
	}

	/*
	 * run along leaf, setting up records.  as we have to switch
	 * blocks, call the prop_ino_cursor routine to set up the new
	 * pointers for the parent.  that can recurse up to the root
	 * if required.  set the sibling pointers for leaf level here.
	 */
	if (btnum == XFS_BTNUM_FINO)
		ino_rec = findfirst_free_inode_rec(agno);
	else
		ino_rec = findfirst_inode_rec(agno);

	if (ino_rec != NULL)
		first_agino = ino_rec->ino_startnum;
	else
		first_agino = NULLAGINO;

	lptr = &btree_curs->level[0];

	for (i = 0; i < lptr->num_blocks; i++)  {
		/*
		 * block initialization, lay in block header
		 */
		lptr->buf_p->b_ops = &xfs_inobt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, btnum, 0, 0, agno, 0);

		bt_hdr->bb_u.s.bb_leftsib = cpu_to_be32(lptr->prev_agbno);
		bt_hdr->bb_numrecs = cpu_to_be16(lptr->num_recs_pb +
							(lptr->modulo > 0));

		if (lptr->modulo > 0)
			lptr->modulo--;

		if (lptr->num_recs_pb > 0)
			prop_ino_cursor(mp, agno, btree_curs,
					ino_rec->ino_startnum, 0);

		bt_rec = (xfs_inobt_rec_t *)
			  ((char *)bt_hdr + XFS_INOBT_BLOCK_LEN(mp));
		for (j = 0; j < be16_to_cpu(bt_hdr->bb_numrecs); j++) {
			ASSERT(ino_rec != NULL);
			bt_rec[j].ir_startino =
					cpu_to_be32(ino_rec->ino_startnum);
			bt_rec[j].ir_free = cpu_to_be64(ino_rec->ir_free);

			inocnt = finocnt = 0;
			for (k = 0; k < sizeof(xfs_inofree_t)*NBBY; k++)  {
				ASSERT(is_inode_confirmed(ino_rec, k));

				if (is_inode_sparse(ino_rec, k))
					continue;
				if (is_inode_free(ino_rec, k))
					finocnt++;
				inocnt++;
			}

			/*
			 * Set the freecount and check whether we need to update
			 * the sparse format fields. Otherwise, skip to the next
			 * record.
			 */
			inorec_set_freecount(mp, &bt_rec[j], finocnt);
			if (!xfs_sb_version_hassparseinodes(&mp->m_sb))
				goto nextrec;

			/*
			 * Convert the 64-bit in-core sparse inode state to the
			 * 16-bit on-disk holemask.
			 */
			holemask = 0;
			spmask = (1 << XFS_INODES_PER_HOLEMASK_BIT) - 1;
			sparse = ino_rec->ir_sparse;
			for (k = 0; k < XFS_INOBT_HOLEMASK_BITS; k++) {
				if (sparse & spmask) {
					ASSERT((sparse & spmask) == spmask);
					holemask |= (1 << k);
				} else
					ASSERT((sparse & spmask) == 0);
				sparse >>= XFS_INODES_PER_HOLEMASK_BIT;
			}

			bt_rec[j].ir_u.sp.ir_count = inocnt;
			bt_rec[j].ir_u.sp.ir_holemask = cpu_to_be16(holemask);

nextrec:
			freecount += finocnt;
			count += inocnt;

			if (btnum == XFS_BTNUM_FINO)
				ino_rec = next_free_ino_rec(ino_rec);
			else
				ino_rec = next_ino_rec(ino_rec);
		}

		if (ino_rec != NULL)  {
			/*
			 * get next leaf level block
			 */
			if (lptr->prev_buf_p != NULL)  {
#ifdef XR_BLD_INO_TRACE
				fprintf(stderr, "writing inobt agbno %u\n",
					lptr->prev_agbno);
#endif
				ASSERT(lptr->prev_agbno != NULLAGBLOCK);
				libxfs_writebuf(lptr->prev_buf_p, 0);
			}
			lptr->prev_buf_p = lptr->buf_p;
			lptr->prev_agbno = lptr->agbno;
			lptr->agbno = get_next_blockaddr(agno, 0, btree_curs);
			bt_hdr->bb_u.s.bb_rightsib = cpu_to_be32(lptr->agbno);

			lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, lptr->agbno),
					XFS_FSB_TO_BB(mp, 1));
		}
	}

	if (agi_stat) {
		agi_stat->first_agino = first_agino;
		agi_stat->count = count;
		agi_stat->freecount = freecount;
	}
}

/* rebuild the rmap tree */

/*
 * we don't have to worry here about how chewing up free extents
 * may perturb things because rmap tree building happens before
 * freespace tree building.
 */
static void
init_rmapbt_cursor(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_status	*btree_curs)
{
	size_t			num_recs;
	int			level;
	struct bt_stat_level	*lptr;
	struct bt_stat_level	*p_lptr;
	xfs_extlen_t		blocks_allocated;
	int			maxrecs;

	if (!xfs_sb_version_hasrmapbt(&mp->m_sb)) {
		memset(btree_curs, 0, sizeof(struct bt_status));
		return;
	}

	lptr = &btree_curs->level[0];
	btree_curs->init = 1;
	btree_curs->owner = XFS_RMAP_OWN_AG;

	/*
	 * build up statistics
	 */
	num_recs = rmap_record_count(mp, agno);
	if (num_recs == 0) {
		/*
		 * easy corner-case -- no rmap records
		 */
		lptr->num_blocks = 1;
		lptr->modulo = 0;
		lptr->num_recs_pb = 0;
		lptr->num_recs_tot = 0;

		btree_curs->num_levels = 1;
		btree_curs->num_tot_blocks = btree_curs->num_free_blocks = 1;

		setup_cursor(mp, agno, btree_curs);

		return;
	}

	/*
	 * Leave enough slack in the rmapbt that we can insert the
	 * metadata AG entries without too many splits.
	 */
	maxrecs = mp->m_rmap_mxr[0];
	if (num_recs > maxrecs)
		maxrecs -= 10;
	blocks_allocated = lptr->num_blocks = howmany(num_recs, maxrecs);

	lptr->modulo = num_recs % lptr->num_blocks;
	lptr->num_recs_pb = num_recs / lptr->num_blocks;
	lptr->num_recs_tot = num_recs;
	level = 1;

	if (lptr->num_blocks > 1)  {
		for (; btree_curs->level[level-1].num_blocks > 1
				&& level < XFS_BTREE_MAXLEVELS;
				level++)  {
			lptr = &btree_curs->level[level];
			p_lptr = &btree_curs->level[level - 1];
			lptr->num_blocks = howmany(p_lptr->num_blocks,
				mp->m_rmap_mxr[1]);
			lptr->modulo = p_lptr->num_blocks % lptr->num_blocks;
			lptr->num_recs_pb = p_lptr->num_blocks
					/ lptr->num_blocks;
			lptr->num_recs_tot = p_lptr->num_blocks;

			blocks_allocated += lptr->num_blocks;
		}
	}
	ASSERT(lptr->num_blocks == 1);
	btree_curs->num_levels = level;

	btree_curs->num_tot_blocks = btree_curs->num_free_blocks
			= blocks_allocated;

	setup_cursor(mp, agno, btree_curs);
}

static void
prop_rmap_cursor(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_status	*btree_curs,
	struct xfs_rmap_irec	*rm_rec,
	int			level)
{
	struct xfs_btree_block	*bt_hdr;
	struct xfs_rmap_key	*bt_key;
	xfs_rmap_ptr_t		*bt_ptr;
	xfs_agblock_t		agbno;
	struct bt_stat_level	*lptr;

	level++;

	if (level >= btree_curs->num_levels)
		return;

	lptr = &btree_curs->level[level];
	bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);

	if (be16_to_cpu(bt_hdr->bb_numrecs) == 0)  {
		/*
		 * this only happens once to initialize the
		 * first path up the left side of the tree
		 * where the agbno's are already set up
		 */
		prop_rmap_cursor(mp, agno, btree_curs, rm_rec, level);
	}

	if (be16_to_cpu(bt_hdr->bb_numrecs) ==
				lptr->num_recs_pb + (lptr->modulo > 0))  {
		/*
		 * write out current prev block, grab us a new block,
		 * and set the rightsib pointer of current block
		 */
#ifdef XR_BLD_INO_TRACE
		fprintf(stderr, " rmap prop agbno %d ", lptr->prev_agbno);
#endif
		if (lptr->prev_agbno != NULLAGBLOCK)  {
			ASSERT(lptr->prev_buf_p != NULL);
			libxfs_writebuf(lptr->prev_buf_p, 0);
		}
		lptr->prev_agbno = lptr->agbno;
		lptr->prev_buf_p = lptr->buf_p;
		agbno = get_next_blockaddr(agno, level, btree_curs);

		bt_hdr->bb_u.s.bb_rightsib = cpu_to_be32(agbno);

		lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, agbno),
					XFS_FSB_TO_BB(mp, 1));
		lptr->agbno = agbno;

		if (lptr->modulo)
			lptr->modulo--;

		/*
		 * initialize block header
		 */
		lptr->buf_p->b_ops = &xfs_rmapbt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, XFS_BTNUM_RMAP,
					level, 0, agno, 0);

		bt_hdr->bb_u.s.bb_leftsib = cpu_to_be32(lptr->prev_agbno);

		/*
		 * propagate extent record for first extent in new block up
		 */
		prop_rmap_cursor(mp, agno, btree_curs, rm_rec, level);
	}
	/*
	 * add rmap info to current block
	 */
	be16_add_cpu(&bt_hdr->bb_numrecs, 1);

	bt_key = XFS_RMAP_KEY_ADDR(bt_hdr,
				    be16_to_cpu(bt_hdr->bb_numrecs));
	bt_ptr = XFS_RMAP_PTR_ADDR(bt_hdr,
				    be16_to_cpu(bt_hdr->bb_numrecs),
				    mp->m_rmap_mxr[1]);

	bt_key->rm_startblock = cpu_to_be32(rm_rec->rm_startblock);
	bt_key->rm_owner = cpu_to_be64(rm_rec->rm_owner);
	bt_key->rm_offset = cpu_to_be64(rm_rec->rm_offset);

	*bt_ptr = cpu_to_be32(btree_curs->level[level-1].agbno);
}

static void
prop_rmap_highkey(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_status	*btree_curs,
	struct xfs_rmap_irec	*rm_highkey)
{
	struct xfs_btree_block	*bt_hdr;
	struct xfs_rmap_key	*bt_key;
	struct bt_stat_level	*lptr;
	struct xfs_rmap_irec	key = {0};
	struct xfs_rmap_irec	high_key;
	int			level;
	int			i;
	int			numrecs;

	high_key = *rm_highkey;
	for (level = 1; level < btree_curs->num_levels; level++) {
		lptr = &btree_curs->level[level];
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		numrecs = be16_to_cpu(bt_hdr->bb_numrecs);
		bt_key = XFS_RMAP_HIGH_KEY_ADDR(bt_hdr, numrecs);

		bt_key->rm_startblock = cpu_to_be32(high_key.rm_startblock);
		bt_key->rm_owner = cpu_to_be64(high_key.rm_owner);
		bt_key->rm_offset = cpu_to_be64(
				libxfs_rmap_irec_offset_pack(&high_key));

		for (i = 1; i < numrecs - 1; i++) {
			bt_key = XFS_RMAP_HIGH_KEY_ADDR(bt_hdr, i);
			key.rm_startblock = be32_to_cpu(bt_key->rm_startblock);
			key.rm_owner = be64_to_cpu(bt_key->rm_owner);
			key.rm_offset = be64_to_cpu(bt_key->rm_offset);
			if (rmap_diffkeys(&key, &high_key) > 0)
				high_key = key;
		}
	}
}

/*
 * rebuilds a rmap btree given a cursor.
 */
static void
build_rmap_tree(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_status	*btree_curs)
{
	xfs_agnumber_t		i;
	xfs_agblock_t		j;
	xfs_agblock_t		agbno;
	struct xfs_btree_block	*bt_hdr;
	struct xfs_rmap_irec	*rm_rec;
	struct xfs_slab_cursor	*rmap_cur;
	struct xfs_rmap_rec	*bt_rec;
	struct xfs_rmap_irec	highest_key = {0};
	struct xfs_rmap_irec	hi_key = {0};
	struct bt_stat_level	*lptr;
	int			numrecs;
	int			level = btree_curs->num_levels;
	int			error;

	highest_key.rm_flags = 0;
	for (i = 0; i < level; i++)  {
		lptr = &btree_curs->level[i];

		agbno = get_next_blockaddr(agno, i, btree_curs);
		lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, agbno),
					XFS_FSB_TO_BB(mp, 1));

		if (i == btree_curs->num_levels - 1)
			btree_curs->root = agbno;

		lptr->agbno = agbno;
		lptr->prev_agbno = NULLAGBLOCK;
		lptr->prev_buf_p = NULL;
		/*
		 * initialize block header
		 */

		lptr->buf_p->b_ops = &xfs_rmapbt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, XFS_BTNUM_RMAP,
					i, 0, agno, 0);
	}

	/*
	 * run along leaf, setting up records.  as we have to switch
	 * blocks, call the prop_rmap_cursor routine to set up the new
	 * pointers for the parent.  that can recurse up to the root
	 * if required.  set the sibling pointers for leaf level here.
	 */
	error = rmap_init_cursor(agno, &rmap_cur);
	if (error)
		do_error(
_("Insufficient memory to construct reverse-map cursor."));
	rm_rec = pop_slab_cursor(rmap_cur);
	lptr = &btree_curs->level[0];

	for (i = 0; i < lptr->num_blocks; i++)  {
		numrecs = lptr->num_recs_pb + (lptr->modulo > 0);
		ASSERT(rm_rec != NULL || numrecs == 0);

		/*
		 * block initialization, lay in block header
		 */
		lptr->buf_p->b_ops = &xfs_rmapbt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, XFS_BTNUM_RMAP,
					0, 0, agno, 0);

		bt_hdr->bb_u.s.bb_leftsib = cpu_to_be32(lptr->prev_agbno);
		bt_hdr->bb_numrecs = cpu_to_be16(numrecs);

		if (lptr->modulo > 0)
			lptr->modulo--;

		if (lptr->num_recs_pb > 0) {
			ASSERT(rm_rec != NULL);
			prop_rmap_cursor(mp, agno, btree_curs, rm_rec, 0);
		}

		bt_rec = (struct xfs_rmap_rec *)
			  ((char *)bt_hdr + XFS_RMAP_BLOCK_LEN);
		highest_key.rm_startblock = 0;
		highest_key.rm_owner = 0;
		highest_key.rm_offset = 0;
		for (j = 0; j < be16_to_cpu(bt_hdr->bb_numrecs); j++) {
			ASSERT(rm_rec != NULL);
			bt_rec[j].rm_startblock =
					cpu_to_be32(rm_rec->rm_startblock);
			bt_rec[j].rm_blockcount =
					cpu_to_be32(rm_rec->rm_blockcount);
			bt_rec[j].rm_owner = cpu_to_be64(rm_rec->rm_owner);
			bt_rec[j].rm_offset = cpu_to_be64(
					libxfs_rmap_irec_offset_pack(rm_rec));
			rmap_high_key_from_rec(rm_rec, &hi_key);
			if (rmap_diffkeys(&hi_key, &highest_key) > 0)
				highest_key = hi_key;

			rm_rec = pop_slab_cursor(rmap_cur);
		}

		/* Now go set the parent key */
		prop_rmap_highkey(mp, agno, btree_curs, &highest_key);

		if (rm_rec != NULL)  {
			/*
			 * get next leaf level block
			 */
			if (lptr->prev_buf_p != NULL)  {
#ifdef XR_BLD_RL_TRACE
				fprintf(stderr, "writing rmapbt agbno %u\n",
					lptr->prev_agbno);
#endif
				ASSERT(lptr->prev_agbno != NULLAGBLOCK);
				libxfs_writebuf(lptr->prev_buf_p, 0);
			}
			lptr->prev_buf_p = lptr->buf_p;
			lptr->prev_agbno = lptr->agbno;
			lptr->agbno = get_next_blockaddr(agno, 0, btree_curs);
			bt_hdr->bb_u.s.bb_rightsib = cpu_to_be32(lptr->agbno);

			lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, lptr->agbno),
					XFS_FSB_TO_BB(mp, 1));
		}
	}
	free_slab_cursor(&rmap_cur);
}

/* rebuild the refcount tree */

/*
 * we don't have to worry here about how chewing up free extents
 * may perturb things because reflink tree building happens before
 * freespace tree building.
 */
static void
init_refc_cursor(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_status	*btree_curs)
{
	size_t			num_recs;
	int			level;
	struct bt_stat_level	*lptr;
	struct bt_stat_level	*p_lptr;
	xfs_extlen_t		blocks_allocated;

	if (!xfs_sb_version_hasreflink(&mp->m_sb)) {
		memset(btree_curs, 0, sizeof(struct bt_status));
		return;
	}

	lptr = &btree_curs->level[0];
	btree_curs->init = 1;
	btree_curs->owner = XFS_RMAP_OWN_REFC;

	/*
	 * build up statistics
	 */
	num_recs = refcount_record_count(mp, agno);
	if (num_recs == 0) {
		/*
		 * easy corner-case -- no refcount records
		 */
		lptr->num_blocks = 1;
		lptr->modulo = 0;
		lptr->num_recs_pb = 0;
		lptr->num_recs_tot = 0;

		btree_curs->num_levels = 1;
		btree_curs->num_tot_blocks = btree_curs->num_free_blocks = 1;

		setup_cursor(mp, agno, btree_curs);

		return;
	}

	blocks_allocated = lptr->num_blocks = howmany(num_recs,
					mp->m_refc_mxr[0]);

	lptr->modulo = num_recs % lptr->num_blocks;
	lptr->num_recs_pb = num_recs / lptr->num_blocks;
	lptr->num_recs_tot = num_recs;
	level = 1;

	if (lptr->num_blocks > 1)  {
		for (; btree_curs->level[level-1].num_blocks > 1
				&& level < XFS_BTREE_MAXLEVELS;
				level++)  {
			lptr = &btree_curs->level[level];
			p_lptr = &btree_curs->level[level - 1];
			lptr->num_blocks = howmany(p_lptr->num_blocks,
					mp->m_refc_mxr[1]);
			lptr->modulo = p_lptr->num_blocks % lptr->num_blocks;
			lptr->num_recs_pb = p_lptr->num_blocks
					/ lptr->num_blocks;
			lptr->num_recs_tot = p_lptr->num_blocks;

			blocks_allocated += lptr->num_blocks;
		}
	}
	ASSERT(lptr->num_blocks == 1);
	btree_curs->num_levels = level;

	btree_curs->num_tot_blocks = btree_curs->num_free_blocks
			= blocks_allocated;

	setup_cursor(mp, agno, btree_curs);
}

static void
prop_refc_cursor(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_status	*btree_curs,
	xfs_agblock_t		startbno,
	int			level)
{
	struct xfs_btree_block	*bt_hdr;
	struct xfs_refcount_key	*bt_key;
	xfs_refcount_ptr_t	*bt_ptr;
	xfs_agblock_t		agbno;
	struct bt_stat_level	*lptr;

	level++;

	if (level >= btree_curs->num_levels)
		return;

	lptr = &btree_curs->level[level];
	bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);

	if (be16_to_cpu(bt_hdr->bb_numrecs) == 0)  {
		/*
		 * this only happens once to initialize the
		 * first path up the left side of the tree
		 * where the agbno's are already set up
		 */
		prop_refc_cursor(mp, agno, btree_curs, startbno, level);
	}

	if (be16_to_cpu(bt_hdr->bb_numrecs) ==
				lptr->num_recs_pb + (lptr->modulo > 0))  {
		/*
		 * write out current prev block, grab us a new block,
		 * and set the rightsib pointer of current block
		 */
#ifdef XR_BLD_INO_TRACE
		fprintf(stderr, " ino prop agbno %d ", lptr->prev_agbno);
#endif
		if (lptr->prev_agbno != NULLAGBLOCK)  {
			ASSERT(lptr->prev_buf_p != NULL);
			libxfs_writebuf(lptr->prev_buf_p, 0);
		}
		lptr->prev_agbno = lptr->agbno;
		lptr->prev_buf_p = lptr->buf_p;
		agbno = get_next_blockaddr(agno, level, btree_curs);

		bt_hdr->bb_u.s.bb_rightsib = cpu_to_be32(agbno);

		lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, agbno),
					XFS_FSB_TO_BB(mp, 1));
		lptr->agbno = agbno;

		if (lptr->modulo)
			lptr->modulo--;

		/*
		 * initialize block header
		 */
		lptr->buf_p->b_ops = &xfs_refcountbt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, XFS_BTNUM_REFC,
					level, 0, agno, 0);

		bt_hdr->bb_u.s.bb_leftsib = cpu_to_be32(lptr->prev_agbno);

		/*
		 * propagate extent record for first extent in new block up
		 */
		prop_refc_cursor(mp, agno, btree_curs, startbno, level);
	}
	/*
	 * add inode info to current block
	 */
	be16_add_cpu(&bt_hdr->bb_numrecs, 1);

	bt_key = XFS_REFCOUNT_KEY_ADDR(bt_hdr,
				    be16_to_cpu(bt_hdr->bb_numrecs));
	bt_ptr = XFS_REFCOUNT_PTR_ADDR(bt_hdr,
				    be16_to_cpu(bt_hdr->bb_numrecs),
				    mp->m_refc_mxr[1]);

	bt_key->rc_startblock = cpu_to_be32(startbno);
	*bt_ptr = cpu_to_be32(btree_curs->level[level-1].agbno);
}

/*
 * rebuilds a refcount btree given a cursor.
 */
static void
build_refcount_tree(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_status	*btree_curs)
{
	xfs_agnumber_t		i;
	xfs_agblock_t		j;
	xfs_agblock_t		agbno;
	struct xfs_btree_block	*bt_hdr;
	struct xfs_refcount_irec	*refc_rec;
	struct xfs_slab_cursor	*refc_cur;
	struct xfs_refcount_rec	*bt_rec;
	struct bt_stat_level	*lptr;
	int			numrecs;
	int			level = btree_curs->num_levels;
	int			error;

	for (i = 0; i < level; i++)  {
		lptr = &btree_curs->level[i];

		agbno = get_next_blockaddr(agno, i, btree_curs);
		lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, agbno),
					XFS_FSB_TO_BB(mp, 1));

		if (i == btree_curs->num_levels - 1)
			btree_curs->root = agbno;

		lptr->agbno = agbno;
		lptr->prev_agbno = NULLAGBLOCK;
		lptr->prev_buf_p = NULL;
		/*
		 * initialize block header
		 */

		lptr->buf_p->b_ops = &xfs_refcountbt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, XFS_BTNUM_REFC,
					i, 0, agno, 0);
	}

	/*
	 * run along leaf, setting up records.  as we have to switch
	 * blocks, call the prop_refc_cursor routine to set up the new
	 * pointers for the parent.  that can recurse up to the root
	 * if required.  set the sibling pointers for leaf level here.
	 */
	error = init_refcount_cursor(agno, &refc_cur);
	if (error)
		do_error(
_("Insufficient memory to construct refcount cursor."));
	refc_rec = pop_slab_cursor(refc_cur);
	lptr = &btree_curs->level[0];

	for (i = 0; i < lptr->num_blocks; i++)  {
		numrecs = lptr->num_recs_pb + (lptr->modulo > 0);
		ASSERT(refc_rec != NULL || numrecs == 0);

		/*
		 * block initialization, lay in block header
		 */
		lptr->buf_p->b_ops = &xfs_refcountbt_buf_ops;
		bt_hdr = XFS_BUF_TO_BLOCK(lptr->buf_p);
		memset(bt_hdr, 0, mp->m_sb.sb_blocksize);
		libxfs_btree_init_block(mp, lptr->buf_p, XFS_BTNUM_REFC,
					0, 0, agno, 0);

		bt_hdr->bb_u.s.bb_leftsib = cpu_to_be32(lptr->prev_agbno);
		bt_hdr->bb_numrecs = cpu_to_be16(numrecs);

		if (lptr->modulo > 0)
			lptr->modulo--;

		if (lptr->num_recs_pb > 0)
			prop_refc_cursor(mp, agno, btree_curs,
					refc_rec->rc_startblock, 0);

		bt_rec = (struct xfs_refcount_rec *)
			  ((char *)bt_hdr + XFS_REFCOUNT_BLOCK_LEN);
		for (j = 0; j < be16_to_cpu(bt_hdr->bb_numrecs); j++) {
			ASSERT(refc_rec != NULL);
			bt_rec[j].rc_startblock =
					cpu_to_be32(refc_rec->rc_startblock);
			bt_rec[j].rc_blockcount =
					cpu_to_be32(refc_rec->rc_blockcount);
			bt_rec[j].rc_refcount = cpu_to_be32(refc_rec->rc_refcount);

			refc_rec = pop_slab_cursor(refc_cur);
		}

		if (refc_rec != NULL)  {
			/*
			 * get next leaf level block
			 */
			if (lptr->prev_buf_p != NULL)  {
#ifdef XR_BLD_RL_TRACE
				fprintf(stderr, "writing refcntbt agbno %u\n",
					lptr->prev_agbno);
#endif
				ASSERT(lptr->prev_agbno != NULLAGBLOCK);
				libxfs_writebuf(lptr->prev_buf_p, 0);
			}
			lptr->prev_buf_p = lptr->buf_p;
			lptr->prev_agbno = lptr->agbno;
			lptr->agbno = get_next_blockaddr(agno, 0, btree_curs);
			bt_hdr->bb_u.s.bb_rightsib = cpu_to_be32(lptr->agbno);

			lptr->buf_p = libxfs_getbuf(mp->m_dev,
					XFS_AGB_TO_DADDR(mp, agno, lptr->agbno),
					XFS_FSB_TO_BB(mp, 1));
		}
	}
	free_slab_cursor(&refc_cur);
}

/*
 * build both the agf and the agfl for an agno given both
 * btree cursors.
 *
 * XXX: yet more common code that can be shared with mkfs/growfs.
 */
static void
build_agf_agfl(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_status	*bno_bt,
	struct bt_status	*bcnt_bt,
	xfs_extlen_t		freeblks,	/* # free blocks in tree */
	int			lostblocks,	/* # blocks that will be lost */
	struct bt_status	*rmap_bt,
	struct bt_status	*refcnt_bt,
	struct xfs_slab		*lost_fsb)
{
	struct extent_tree_node	*ext_ptr;
	struct xfs_buf		*agf_buf, *agfl_buf;
	int			i;
	struct xfs_agfl		*agfl;
	struct xfs_agf		*agf;
	xfs_fsblock_t		fsb;
	__be32			*freelist;
	int			error;

	agf_buf = libxfs_getbuf(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGF_DADDR(mp)),
			mp->m_sb.sb_sectsize/BBSIZE);
	agf_buf->b_ops = &xfs_agf_buf_ops;
	agf = XFS_BUF_TO_AGF(agf_buf);
	memset(agf, 0, mp->m_sb.sb_sectsize);

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "agf = 0x%p, agf_buf->b_addr = 0x%p\n",
		agf, agf_buf->b_addr);
#endif

	/*
	 * set up fixed part of agf
	 */
	agf->agf_magicnum = cpu_to_be32(XFS_AGF_MAGIC);
	agf->agf_versionnum = cpu_to_be32(XFS_AGF_VERSION);
	agf->agf_seqno = cpu_to_be32(agno);

	if (agno < mp->m_sb.sb_agcount - 1)
		agf->agf_length = cpu_to_be32(mp->m_sb.sb_agblocks);
	else
		agf->agf_length = cpu_to_be32(mp->m_sb.sb_dblocks -
			(xfs_rfsblock_t) mp->m_sb.sb_agblocks * agno);

	agf->agf_roots[XFS_BTNUM_BNO] = cpu_to_be32(bno_bt->root);
	agf->agf_levels[XFS_BTNUM_BNO] = cpu_to_be32(bno_bt->num_levels);
	agf->agf_roots[XFS_BTNUM_CNT] = cpu_to_be32(bcnt_bt->root);
	agf->agf_levels[XFS_BTNUM_CNT] = cpu_to_be32(bcnt_bt->num_levels);
	agf->agf_roots[XFS_BTNUM_RMAP] = cpu_to_be32(rmap_bt->root);
	agf->agf_levels[XFS_BTNUM_RMAP] = cpu_to_be32(rmap_bt->num_levels);
	agf->agf_freeblks = cpu_to_be32(freeblks);
	agf->agf_rmap_blocks = cpu_to_be32(rmap_bt->num_tot_blocks -
			rmap_bt->num_free_blocks);
	agf->agf_refcount_root = cpu_to_be32(refcnt_bt->root);
	agf->agf_refcount_level = cpu_to_be32(refcnt_bt->num_levels);
	agf->agf_refcount_blocks = cpu_to_be32(refcnt_bt->num_tot_blocks -
			refcnt_bt->num_free_blocks);

	/*
	 * Count and record the number of btree blocks consumed if required.
	 */
	if (xfs_sb_version_haslazysbcount(&mp->m_sb)) {
		unsigned int blks;
		/*
		 * Don't count the root blocks as they are already
		 * accounted for.
		 */
		blks = (bno_bt->num_tot_blocks - bno_bt->num_free_blocks) +
			(bcnt_bt->num_tot_blocks - bcnt_bt->num_free_blocks) -
			2;
		if (xfs_sb_version_hasrmapbt(&mp->m_sb))
			blks += rmap_bt->num_tot_blocks - rmap_bt->num_free_blocks - 1;
		agf->agf_btreeblks = cpu_to_be32(blks);
#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, "agf->agf_btreeblks = %u\n",
				be32_to_cpu(agf->agf_btreeblks));
#endif
	}

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "bno root = %u, bcnt root = %u, indices = %u %u\n",
			be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNO]),
			be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNT]),
			XFS_BTNUM_BNO,
			XFS_BTNUM_CNT);
#endif

	if (xfs_sb_version_hascrc(&mp->m_sb))
		platform_uuid_copy(&agf->agf_uuid, &mp->m_sb.sb_meta_uuid);

	/* initialise the AGFL, then fill it if there are blocks left over. */
	agfl_buf = libxfs_getbuf(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGFL_DADDR(mp)),
			mp->m_sb.sb_sectsize/BBSIZE);
	agfl_buf->b_ops = &xfs_agfl_buf_ops;
	agfl = XFS_BUF_TO_AGFL(agfl_buf);

	/* setting to 0xff results in initialisation to NULLAGBLOCK */
	memset(agfl, 0xff, mp->m_sb.sb_sectsize);
	if (xfs_sb_version_hascrc(&mp->m_sb)) {
		agfl->agfl_magicnum = cpu_to_be32(XFS_AGFL_MAGIC);
		agfl->agfl_seqno = cpu_to_be32(agno);
		platform_uuid_copy(&agfl->agfl_uuid, &mp->m_sb.sb_meta_uuid);
		for (i = 0; i < XFS_AGFL_SIZE(mp); i++)
			agfl->agfl_bno[i] = cpu_to_be32(NULLAGBLOCK);
	}
	freelist = XFS_BUF_TO_AGFL_BNO(mp, agfl_buf);

	/*
	 * do we have left-over blocks in the btree cursors that should
	 * be used to fill the AGFL?
	 */
	if (bno_bt->num_free_blocks > 0 || bcnt_bt->num_free_blocks > 0)  {
		/*
		 * yes, now grab as many blocks as we can
		 */
		i = 0;
		while (bno_bt->num_free_blocks > 0 && i < XFS_AGFL_SIZE(mp))  {
			freelist[i] = cpu_to_be32(
					get_next_blockaddr(agno, 0, bno_bt));
			i++;
		}

		while (bcnt_bt->num_free_blocks > 0 && i < XFS_AGFL_SIZE(mp))  {
			freelist[i] = cpu_to_be32(
					get_next_blockaddr(agno, 0, bcnt_bt));
			i++;
		}
		/*
		 * now throw the rest of the blocks away and complain
		 */
		while (bno_bt->num_free_blocks > 0) {
			fsb = XFS_AGB_TO_FSB(mp, agno,
					get_next_blockaddr(agno, 0, bno_bt));
			error = slab_add(lost_fsb, &fsb);
			if (error)
				do_error(
_("Insufficient memory saving lost blocks.\n"));
		}
		while (bcnt_bt->num_free_blocks > 0) {
			fsb = XFS_AGB_TO_FSB(mp, agno,
					get_next_blockaddr(agno, 0, bcnt_bt));
			error = slab_add(lost_fsb, &fsb);
			if (error)
				do_error(
_("Insufficient memory saving lost blocks.\n"));
		}

		agf->agf_flfirst = 0;
		agf->agf_fllast = cpu_to_be32(i - 1);
		agf->agf_flcount = cpu_to_be32(i);
		rmap_store_agflcount(mp, agno, i);

#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, "writing agfl for ag %u\n", agno);
#endif

	} else  {
		agf->agf_flfirst = 0;
		agf->agf_fllast = cpu_to_be32(XFS_AGFL_SIZE(mp) - 1);
		agf->agf_flcount = 0;
	}

	libxfs_writebuf(agfl_buf, 0);

	ext_ptr = findbiggest_bcnt_extent(agno);
	agf->agf_longest = cpu_to_be32((ext_ptr != NULL) ?
						ext_ptr->ex_blockcount : 0);

	ASSERT(be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNOi]) !=
		be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNTi]));
	ASSERT(be32_to_cpu(agf->agf_refcount_root) !=
		be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNOi]));
	ASSERT(be32_to_cpu(agf->agf_refcount_root) !=
		be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNTi]));

	libxfs_writebuf(agf_buf, 0);

	/*
	 * now fix up the free list appropriately
	 */
	fix_freelist(mp, agno, true);

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "wrote agf for ag %u\n", agno);
#endif
}

/*
 * update the superblock counters, sync the sb version numbers and
 * feature bits to the filesystem, and sync up the on-disk superblock
 * to match the incore superblock.
 */
static void
sync_sb(xfs_mount_t *mp)
{
	xfs_buf_t	*bp;

	bp = libxfs_getsb(mp, 0);
	if (!bp)
		do_error(_("couldn't get superblock\n"));

	mp->m_sb.sb_icount = sb_icount;
	mp->m_sb.sb_ifree = sb_ifree;
	mp->m_sb.sb_fdblocks = sb_fdblocks;
	mp->m_sb.sb_frextents = sb_frextents;

	update_sb_version(mp);

	libxfs_sb_to_disk(XFS_BUF_TO_SBP(bp), &mp->m_sb);
	libxfs_writebuf(bp, 0);
}

/*
 * make sure the root and realtime inodes show up allocated
 * even if they've been freed.  they get reinitialized in phase6.
 */
static void
keep_fsinos(xfs_mount_t *mp)
{
	ino_tree_node_t		*irec;
	int			i;

	irec = find_inode_rec(mp, XFS_INO_TO_AGNO(mp, mp->m_sb.sb_rootino),
			XFS_INO_TO_AGINO(mp, mp->m_sb.sb_rootino));

	for (i = 0; i < 3; i++)
		set_inode_used(irec, i);
}

static void
phase5_func(
	xfs_mount_t	*mp,
	xfs_agnumber_t	agno,
	struct xfs_slab	*lost_fsb)
{
	uint64_t	num_inos;
	uint64_t	num_free_inos;
	uint64_t	finobt_num_inos;
	uint64_t	finobt_num_free_inos;
	bt_status_t	bno_btree_curs;
	bt_status_t	bcnt_btree_curs;
	bt_status_t	ino_btree_curs;
	bt_status_t	fino_btree_curs;
	bt_status_t	rmap_btree_curs;
	bt_status_t	refcnt_btree_curs;
	int		extra_blocks = 0;
	uint		num_freeblocks;
	xfs_extlen_t	freeblks1;
#ifdef DEBUG
	xfs_extlen_t	freeblks2;
#endif
	xfs_agblock_t	num_extents;
	struct agi_stat	agi_stat = {0,};
	int		error;

	if (verbose)
		do_log(_("        - agno = %d\n"), agno);

	{
		/*
		 * build up incore bno and bcnt extent btrees
		 */
		num_extents = mk_incore_fstree(mp, agno);

#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, "# of bno extents is %d\n",
				count_bno_extents(agno));
#endif

		if (num_extents == 0)  {
			/*
			 * XXX - what we probably should do here is pick an
			 * inode for a regular file in the allocation group
			 * that has space allocated and shoot it by traversing
			 * the bmap list and putting all its extents on the
			 * incore freespace trees, clearing the inode,
			 * and clearing the in-use bit in the incore inode
			 * tree.  Then try mk_incore_fstree() again.
			 */
			do_error(_("unable to rebuild AG %u.  "
				  "Not enough free space in on-disk AG.\n"),
				agno);
		}

		/*
		 * ok, now set up the btree cursors for the
		 * on-disk btrees (includs pre-allocating all
		 * required blocks for the trees themselves)
		 */
		init_ino_cursor(mp, agno, &ino_btree_curs, &num_inos,
				&num_free_inos, 0);

		if (xfs_sb_version_hasfinobt(&mp->m_sb))
			init_ino_cursor(mp, agno, &fino_btree_curs,
					&finobt_num_inos, &finobt_num_free_inos,
					1);

		sb_icount_ag[agno] += num_inos;
		sb_ifree_ag[agno] += num_free_inos;

		/*
		 * Set up the btree cursors for the on-disk rmap btrees,
		 * which includes pre-allocating all required blocks.
		 */
		init_rmapbt_cursor(mp, agno, &rmap_btree_curs);

		/*
		 * Set up the btree cursors for the on-disk refcount btrees,
		 * which includes pre-allocating all required blocks.
		 */
		init_refc_cursor(mp, agno, &refcnt_btree_curs);

		num_extents = count_bno_extents_blocks(agno, &num_freeblocks);
		/*
		 * lose two blocks per AG -- the space tree roots
		 * are counted as allocated since the space trees
		 * always have roots
		 */
		sb_fdblocks_ag[agno] += num_freeblocks - 2;

		if (num_extents == 0)  {
			/*
			 * XXX - what we probably should do here is pick an
			 * inode for a regular file in the allocation group
			 * that has space allocated and shoot it by traversing
			 * the bmap list and putting all its extents on the
			 * incore freespace trees, clearing the inode,
			 * and clearing the in-use bit in the incore inode
			 * tree.  Then try mk_incore_fstree() again.
			 */
			do_error(
			_("unable to rebuild AG %u.  No free space.\n"), agno);
		}

#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, "# of bno extents is %d\n", num_extents);
#endif

		/*
		 * track blocks that we might really lose
		 */
		extra_blocks = calculate_freespace_cursor(mp, agno,
					&num_extents, &bno_btree_curs);

		/*
		 * freespace btrees live in the "free space" but
		 * the filesystem treats AGFL blocks as allocated
		 * since they aren't described by the freespace trees
		 */

		/*
		 * see if we can fit all the extra blocks into the AGFL
		 */
		extra_blocks = (extra_blocks - XFS_AGFL_SIZE(mp) > 0)
				? extra_blocks - XFS_AGFL_SIZE(mp)
				: 0;

		if (extra_blocks > 0)
			sb_fdblocks_ag[agno] -= extra_blocks;

		bcnt_btree_curs = bno_btree_curs;

		bno_btree_curs.owner = XFS_RMAP_OWN_AG;
		bcnt_btree_curs.owner = XFS_RMAP_OWN_AG;
		setup_cursor(mp, agno, &bno_btree_curs);
		setup_cursor(mp, agno, &bcnt_btree_curs);

#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, "# of bno extents is %d\n",
				count_bno_extents(agno));
		fprintf(stderr, "# of bcnt extents is %d\n",
				count_bcnt_extents(agno));
#endif

		/*
		 * now rebuild the freespace trees
		 */
		freeblks1 = build_freespace_tree(mp, agno,
					&bno_btree_curs, XFS_BTNUM_BNO);
#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, "# of free blocks == %d\n", freeblks1);
#endif
		write_cursor(&bno_btree_curs);

#ifdef DEBUG
		freeblks2 = build_freespace_tree(mp, agno,
					&bcnt_btree_curs, XFS_BTNUM_CNT);
#else
		(void) build_freespace_tree(mp, agno,
					&bcnt_btree_curs, XFS_BTNUM_CNT);
#endif
		write_cursor(&bcnt_btree_curs);

		ASSERT(freeblks1 == freeblks2);

		if (xfs_sb_version_hasrmapbt(&mp->m_sb)) {
			build_rmap_tree(mp, agno, &rmap_btree_curs);
			write_cursor(&rmap_btree_curs);
			sb_fdblocks_ag[agno] += (rmap_btree_curs.num_tot_blocks -
					rmap_btree_curs.num_free_blocks) - 1;
		}

		if (xfs_sb_version_hasreflink(&mp->m_sb)) {
			build_refcount_tree(mp, agno, &refcnt_btree_curs);
			write_cursor(&refcnt_btree_curs);
		}

		/*
		 * set up agf and agfl
		 */
		build_agf_agfl(mp, agno, &bno_btree_curs,
				&bcnt_btree_curs, freeblks1, extra_blocks,
				&rmap_btree_curs, &refcnt_btree_curs, lost_fsb);
		/*
		 * build inode allocation tree.
		 */
		build_ino_tree(mp, agno, &ino_btree_curs, XFS_BTNUM_INO,
				&agi_stat);
		write_cursor(&ino_btree_curs);

		/*
		 * build free inode tree
		 */
		if (xfs_sb_version_hasfinobt(&mp->m_sb)) {
			build_ino_tree(mp, agno, &fino_btree_curs,
					XFS_BTNUM_FINO, NULL);
			write_cursor(&fino_btree_curs);
		}

		/* build the agi */
		build_agi(mp, agno, &ino_btree_curs, &fino_btree_curs,
			  &agi_stat);

		/*
		 * tear down cursors
		 */
		finish_cursor(&bno_btree_curs);
		finish_cursor(&ino_btree_curs);
		if (xfs_sb_version_hasrmapbt(&mp->m_sb))
			finish_cursor(&rmap_btree_curs);
		if (xfs_sb_version_hasreflink(&mp->m_sb))
			finish_cursor(&refcnt_btree_curs);
		if (xfs_sb_version_hasfinobt(&mp->m_sb))
			finish_cursor(&fino_btree_curs);
		finish_cursor(&bcnt_btree_curs);

		/*
		 * Put the per-AG btree rmap data into the rmapbt
		 */
		error = rmap_store_ag_btree_rec(mp, agno);
		if (error)
			do_error(
_("unable to add AG %u reverse-mapping data to btree.\n"), agno);

		/*
		 * release the incore per-AG bno/bcnt trees so
		 * the extent nodes can be recycled
		 */
		release_agbno_extent_tree(agno);
		release_agbcnt_extent_tree(agno);
	}
	PROG_RPT_INC(prog_rpt_done[agno], 1);
}

/* Inject lost blocks back into the filesystem. */
static int
inject_lost_blocks(
	struct xfs_mount	*mp,
	struct xfs_slab		*lost_fsbs)
{
	struct xfs_trans	*tp = NULL;
	struct xfs_slab_cursor	*cur = NULL;
	xfs_fsblock_t		*fsb;
	struct xfs_trans_res	tres = {0};
	struct xfs_owner_info	oinfo;
	int			error;

	libxfs_rmap_ag_owner(&oinfo, XFS_RMAP_OWN_AG);
	error = init_slab_cursor(lost_fsbs, NULL, &cur);
	if (error)
		return error;

	while ((fsb = pop_slab_cursor(cur)) != NULL) {
		error = -libxfs_trans_alloc(mp, &tres, 16, 0, 0, &tp);
		if (error)
			goto out_cancel;

		error = -libxfs_free_extent(tp, *fsb, 1, &oinfo,
					    XFS_AG_RESV_NONE);
		if (error)
			goto out_cancel;

		error = -libxfs_trans_commit(tp);
		if (error)
			goto out_cancel;
		tp = NULL;
	}

out_cancel:
	if (tp)
		libxfs_trans_cancel(tp);
	free_slab_cursor(&cur);
	return error;
}

void
phase5(xfs_mount_t *mp)
{
	struct xfs_slab		*lost_fsb;
	xfs_agnumber_t		agno;
	int			error;

	do_log(_("Phase 5 - rebuild AG headers and trees...\n"));
	set_progress_msg(PROG_FMT_REBUILD_AG, (uint64_t)glob_agcount);

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "inobt level 1, maxrec = %d, minrec = %d\n",
		libxfs_inobt_maxrecs(mp, mp->m_sb.sb_blocksize, 0),
		libxfs_inobt_maxrecs(mp, mp->m_sb.sb_blocksize, 0) / 2);
	fprintf(stderr, "inobt level 0 (leaf), maxrec = %d, minrec = %d\n",
		libxfs_inobt_maxrecs(mp, mp->m_sb.sb_blocksize, 1),
		libxfs_inobt_maxrecs(mp, mp->m_sb.sb_blocksize, 1) / 2);
	fprintf(stderr, "xr inobt level 0 (leaf), maxrec = %d\n",
		XR_INOBT_BLOCK_MAXRECS(mp, 0));
	fprintf(stderr, "xr inobt level 1 (int), maxrec = %d\n",
		XR_INOBT_BLOCK_MAXRECS(mp, 1));
	fprintf(stderr, "bnobt level 1, maxrec = %d, minrec = %d\n",
		libxfs_allocbt_maxrecs(mp, mp->m_sb.sb_blocksize, 0),
		libxfs_allocbt_maxrecs(mp, mp->m_sb.sb_blocksize, 0) / 2);
	fprintf(stderr, "bnobt level 0 (leaf), maxrec = %d, minrec = %d\n",
		libxfs_allocbt_maxrecs(mp, mp->m_sb.sb_blocksize, 1),
		libxfs_allocbt_maxrecs(mp, mp->m_sb.sb_blocksize, 1) / 2);
#endif
	/*
	 * make sure the root and realtime inodes show up allocated
	 */
	keep_fsinos(mp);

	/* allocate per ag counters */
	sb_icount_ag = calloc(mp->m_sb.sb_agcount, sizeof(uint64_t));
	if (sb_icount_ag == NULL)
		do_error(_("cannot alloc sb_icount_ag buffers\n"));

	sb_ifree_ag = calloc(mp->m_sb.sb_agcount, sizeof(uint64_t));
	if (sb_ifree_ag == NULL)
		do_error(_("cannot alloc sb_ifree_ag buffers\n"));

	sb_fdblocks_ag = calloc(mp->m_sb.sb_agcount, sizeof(uint64_t));
	if (sb_fdblocks_ag == NULL)
		do_error(_("cannot alloc sb_fdblocks_ag buffers\n"));

	error = init_slab(&lost_fsb, sizeof(xfs_fsblock_t));
	if (error)
		do_error(_("cannot alloc lost block slab\n"));

	for (agno = 0; agno < mp->m_sb.sb_agcount; agno++)
		phase5_func(mp, agno, lost_fsb);

	print_final_rpt();

	/* aggregate per ag counters */
	for (agno = 0; agno < mp->m_sb.sb_agcount; agno++)  {
		sb_icount += sb_icount_ag[agno];
		sb_ifree += sb_ifree_ag[agno];
		sb_fdblocks += sb_fdblocks_ag[agno];
	}
	free(sb_icount_ag);
	free(sb_ifree_ag);
	free(sb_fdblocks_ag);

	if (mp->m_sb.sb_rblocks)  {
		do_log(
		_("        - generate realtime summary info and bitmap...\n"));
		rtinit(mp);
		generate_rtinfo(mp, btmcompute, sumcompute);
	}

	do_log(_("        - reset superblock...\n"));

	/*
	 * sync superblock counter and set version bits correctly
	 */
	sync_sb(mp);

	error = inject_lost_blocks(mp, lost_fsb);
	if (error)
		do_error(_("Unable to reinsert lost blocks into filesystem.\n"));
	free_slab(&lost_fsb);

	bad_ino_btree = 0;

}
