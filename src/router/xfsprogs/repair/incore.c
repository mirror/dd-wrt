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

#include <libxfs.h>
#include "avl.h"
#include "btree.h"
#include "globals.h"
#include "incore.h"
#include "agheader.h"
#include "protos.h"
#include "err_protos.h"
#include "threads.h"

/*
 * The following manages the in-core bitmap of the entire filesystem
 * using extents in a btree.
 *
 * The btree items will point to one of the state values below,
 * rather than storing the value itself in the pointer.
 */
static int states[16] =
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

static struct btree_root	**ag_bmap;

static void
update_bmap(
	struct btree_root	*bmap,
	unsigned long		offset,
	xfs_extlen_t		blen,
	void			*new_state)
{
	unsigned long		end = offset + blen;
	int			*cur_state;
	unsigned long		cur_key;
	int			*next_state;
	unsigned long		next_key;
	int			*prev_state;

	cur_state = btree_find(bmap, offset, &cur_key);
	if (!cur_state)
		return;

	if (offset == cur_key) {
		/* if the start is the same as the "item" extent */
		if (cur_state == new_state)
			return;

		/*
		 * Note: this may be NULL if we are updating the map for
		 * the superblock.
		 */
		prev_state = btree_peek_prev(bmap, NULL);

		next_state = btree_peek_next(bmap, &next_key);
		if (next_key > end) {
			/* different end */
			if (new_state == prev_state) {
				/* #1: prev has same state, move offset up */
				btree_update_key(bmap, offset, end);
				return;
			}

			/* #4: insert new extent after, update current value */
			btree_update_value(bmap, offset, new_state);
			btree_insert(bmap, end, cur_state);
			return;
		}

		/* same end (and same start) */
		if (new_state == next_state) {
			/* next has same state */
			if (new_state == prev_state) {
				/* #3: merge prev & next */
				btree_delete(bmap, offset);
				btree_delete(bmap, end);
				return;
			}

			/* #8: merge next */
			btree_update_value(bmap, offset, new_state);
			btree_delete(bmap, end);
			return;
		}

		/* same start, same end, next has different state */
		if (new_state == prev_state) {
			/* #5: prev has same state */
			btree_delete(bmap, offset);
			return;
		}

		/* #6: update value only */
		btree_update_value(bmap, offset, new_state);
		return;
	}

	/* different start, offset is in the middle of "cur" */
	prev_state = btree_peek_prev(bmap, NULL);
	ASSERT(prev_state != NULL);
	if (prev_state == new_state)
		return;

	if (end == cur_key) {
		/* end is at the same point as the current extent */
		if (new_state == cur_state) {
			/* #7: move next extent down */
			btree_update_key(bmap, end, offset);
			return;
		}

		/* #9: different start, same end, add new extent */
		btree_insert(bmap, offset, new_state);
		return;
	}

	/* #2: insert an extent into the middle of another extent */
	btree_insert(bmap, offset, new_state);
	btree_insert(bmap, end, prev_state);
}

void
set_bmap_ext(
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno,
	xfs_extlen_t		blen,
	int			state)
{
	update_bmap(ag_bmap[agno], agbno, blen, &states[state]);
}

int
get_bmap_ext(
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno,
	xfs_agblock_t		maxbno,
	xfs_extlen_t		*blen)
{
	int			*statep;
	unsigned long		key;

	statep = btree_find(ag_bmap[agno], agbno, &key);
	if (!statep)
		return -1;

	if (key == agbno) {
		if (blen) {
			if (!btree_peek_next(ag_bmap[agno], &key))
				return -1;
			*blen = MIN(maxbno, key) - agbno;
		}
		return *statep;
	}

	statep = btree_peek_prev(ag_bmap[agno], NULL);
	if (!statep)
		return -1;
	if (blen)
		*blen = MIN(maxbno, key) - agbno;

	return *statep;
}

static uint64_t		*rt_bmap;
static size_t		rt_bmap_size;

/* block records fit into __uint64_t's units */
#define XR_BB_UNIT	64			/* number of bits/unit */
#define XR_BB		4			/* bits per block record */
#define XR_BB_NUM	(XR_BB_UNIT/XR_BB)	/* number of records per unit */
#define XR_BB_MASK	0xF			/* block record mask */

/*
 * these work in real-time extents (e.g. fsbno == rt extent number)
 */
int
get_rtbmap(
	xfs_drtbno_t	bno)
{
	return (*(rt_bmap + bno /  XR_BB_NUM) >>
		((bno % XR_BB_NUM) * XR_BB)) & XR_BB_MASK;
}

void
set_rtbmap(
	xfs_drtbno_t	bno,
	int		state)
{
	*(rt_bmap + bno / XR_BB_NUM) =
	 ((*(rt_bmap + bno / XR_BB_NUM) &
	  (~((__uint64_t) XR_BB_MASK << ((bno % XR_BB_NUM) * XR_BB)))) |
	 (((__uint64_t) state) << ((bno % XR_BB_NUM) * XR_BB)));
}

static void
reset_rt_bmap(void)
{
	if (rt_bmap)
		memset(rt_bmap, 0x22, rt_bmap_size);	/* XR_E_FREE */
}

static void
init_rt_bmap(
	xfs_mount_t	*mp)
{
	if (mp->m_sb.sb_rextents == 0)
		return;

	rt_bmap_size = roundup(mp->m_sb.sb_rextents / (NBBY / XR_BB),
			       sizeof(__uint64_t));

	rt_bmap = memalign(sizeof(__uint64_t), rt_bmap_size);
	if (!rt_bmap) {
		do_error(
	_("couldn't allocate realtime block map, size = %" PRIu64 "\n"),
			mp->m_sb.sb_rextents);
		return;
	}
}

static void
free_rt_bmap(xfs_mount_t *mp)
{
	free(rt_bmap);
	rt_bmap = NULL;
}


void
reset_bmaps(xfs_mount_t *mp)
{
	xfs_agnumber_t	agno;
	xfs_agblock_t	ag_size;
	int		ag_hdr_block;

	ag_hdr_block = howmany(4 * mp->m_sb.sb_sectsize, mp->m_sb.sb_blocksize);
	ag_size = mp->m_sb.sb_agblocks;

	for (agno = 0; agno < mp->m_sb.sb_agcount; agno++) {
		if (agno == mp->m_sb.sb_agcount - 1)
			ag_size = (xfs_extlen_t)(mp->m_sb.sb_dblocks -
				   (xfs_drfsbno_t)mp->m_sb.sb_agblocks * agno);
#ifdef BTREE_STATS
		if (btree_find(ag_bmap[agno], 0, NULL)) {
			printf("ag_bmap[%d] btree stats:\n", i);
			btree_print_stats(ag_bmap[agno], stdout);
		}
#endif
		/*
		 * We always insert an item for the first block having a
		 * given state.  So the code below means:
		 *
		 *	block 0..ag_hdr_block-1:	XR_E_INUSE_FS
		 *	ag_hdr_block..ag_size:		XR_E_UNKNOWN
		 *	ag_size...			XR_E_BAD_STATE
		 */
		btree_clear(ag_bmap[agno]);
		btree_insert(ag_bmap[agno], 0, &states[XR_E_INUSE_FS]);
		btree_insert(ag_bmap[agno],
				ag_hdr_block, &states[XR_E_UNKNOWN]);
		btree_insert(ag_bmap[agno], ag_size, &states[XR_E_BAD_STATE]);
	}

	if (mp->m_sb.sb_logstart != 0) {
		set_bmap_ext(XFS_FSB_TO_AGNO(mp, mp->m_sb.sb_logstart),
			     XFS_FSB_TO_AGBNO(mp, mp->m_sb.sb_logstart),
			     mp->m_sb.sb_logblocks, XR_E_INUSE_FS);
	}

	reset_rt_bmap();
}

void
init_bmaps(xfs_mount_t *mp)
{
	xfs_agnumber_t i;

	ag_bmap = calloc(mp->m_sb.sb_agcount, sizeof(struct btree_root *));
	if (!ag_bmap)
		do_error(_("couldn't allocate block map btree roots\n"));

	ag_locks = calloc(mp->m_sb.sb_agcount, sizeof(pthread_mutex_t));
	if (!ag_locks)
		do_error(_("couldn't allocate block map locks\n"));

	for (i = 0; i < mp->m_sb.sb_agcount; i++)  {
		btree_init(&ag_bmap[i]);
		pthread_mutex_init(&ag_locks[i], NULL);
	}

	init_rt_bmap(mp);
	reset_bmaps(mp);
}

void
free_bmaps(xfs_mount_t *mp)
{
	xfs_agnumber_t i;

	for (i = 0; i < mp->m_sb.sb_agcount; i++)
		btree_destroy(ag_bmap[i]);
	free(ag_bmap);
	ag_bmap = NULL;

	free_rt_bmap(mp);
}
