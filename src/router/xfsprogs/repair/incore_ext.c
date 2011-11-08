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
#include "avl64.h"
#include "threads.h"
#define ALLOC_NUM_EXTS		100

/*
 * paranoia -- account for any weird padding, 64/32-bit alignment, etc.
 */
typedef struct extent_alloc_rec  {
	struct list_head	list;
	extent_tree_node_t	extents[ALLOC_NUM_EXTS];
} extent_alloc_rec_t;

typedef struct rt_extent_alloc_rec  {
	struct list_head	list;
	rt_extent_tree_node_t	extents[ALLOC_NUM_EXTS];
} rt_extent_alloc_rec_t;

/*
 * note:  there are 4 sets of incore things handled here:
 * block bitmaps, extent trees, uncertain inode list,
 * and inode tree.  The tree-based code uses the AVL
 * tree package used by the IRIX kernel VM code
 * (sys/avl.h).  The inode list code uses the same records
 * as the inode tree code for convenience.  The bitmaps
 * and bitmap operators are mostly macros defined in incore.h.
 * There are one of everything per AG except for extent
 * trees.  There's one duplicate extent tree, one bno and
 * one bcnt extent tree per AG.  Not all of the above exist
 * through all phases.  The duplicate extent tree gets trashed
 * at the end of phase 4.  The bno/bcnt trees don't appear until
 * phase 5.  The uncertain inode list goes away at the end of
 * phase 3.  The inode tree and bno/bnct trees go away after phase 5.
 */
typedef struct ext_flist_s  {
	extent_tree_node_t	*list;
	int			cnt;
} ext_flist_t;

static ext_flist_t ext_flist;

typedef struct rt_ext_flist_s  {
	rt_extent_tree_node_t	*list;
	int			cnt;
} rt_ext_flist_t;

static rt_ext_flist_t rt_ext_flist;

static avl64tree_desc_t	*rt_ext_tree_ptr;	/* dup extent tree for rt */

static struct btree_root **dup_extent_trees;	/* per ag dup extent trees */
static pthread_mutex_t *dup_extent_tree_locks;

static avltree_desc_t	**extent_bno_ptrs;	/*
						 * array of extent tree ptrs
						 * one per ag for free extents
						 * sorted by starting block
						 * number
						 */
static avltree_desc_t	**extent_bcnt_ptrs;	/*
						 * array of extent tree ptrs
						 * one per ag for free extents
						 * sorted by size
						 */

/*
 * list of allocated "blocks" for easy freeing later
 */
static struct list_head	ba_list;
static struct list_head	rt_ba_list;

/*
 * locks.
 */
static pthread_mutex_t	ext_flist_lock;
static pthread_mutex_t	rt_ext_tree_lock;
static pthread_mutex_t	rt_ext_flist_lock;

/*
 * duplicate extent tree functions
 */

void
release_dup_extent_tree(
	xfs_agnumber_t		agno)
{
	pthread_mutex_lock(&dup_extent_tree_locks[agno]);
	btree_clear(dup_extent_trees[agno]);
	pthread_mutex_unlock(&dup_extent_tree_locks[agno]);
}

int
add_dup_extent(
	xfs_agnumber_t		agno,
	xfs_agblock_t		startblock,
	xfs_extlen_t		blockcount)
{
	int	ret;
#ifdef XR_DUP_TRACE
	fprintf(stderr, "Adding dup extent - %d/%d %d\n", agno, startblock,
		blockcount);
#endif
	pthread_mutex_lock(&dup_extent_tree_locks[agno]);
	ret = btree_insert(dup_extent_trees[agno], startblock,
				(void *)(uintptr_t)(startblock + blockcount));
	pthread_mutex_unlock(&dup_extent_tree_locks[agno]);
	return ret;
}

int
search_dup_extent(
	xfs_agnumber_t		agno,
	xfs_agblock_t		start_agbno,
	xfs_agblock_t		end_agbno)
{
	unsigned long	bno;
	int		ret;

	pthread_mutex_lock(&dup_extent_tree_locks[agno]);
	if (!btree_find(dup_extent_trees[agno], start_agbno, &bno)) {
		ret = 0;
		goto out;	/* this really shouldn't happen */
	}
	if (bno < end_agbno) {
		ret = 1;
		goto out;
	}
	ret = (uintptr_t)btree_peek_prev(dup_extent_trees[agno], NULL) >
								start_agbno;
out:
	pthread_mutex_unlock(&dup_extent_tree_locks[agno]);
	return ret;
}


/*
 * extent tree stuff is avl trees of duplicate extents,
 * sorted in order by block number.  there is one tree per ag.
 */

static extent_tree_node_t *
mk_extent_tree_nodes(xfs_agblock_t new_startblock,
	xfs_extlen_t new_blockcount, extent_state_t new_state)
{
	int i;
	extent_tree_node_t *new;
	extent_alloc_rec_t *rec;

	pthread_mutex_lock(&ext_flist_lock);
	if (ext_flist.cnt == 0)  {
		ASSERT(ext_flist.list == NULL);

		if ((rec = malloc(sizeof(extent_alloc_rec_t))) == NULL)
			do_error(
			_("couldn't allocate new extent descriptors.\n"));

		list_add(&rec->list, &ba_list);

		new = &rec->extents[0];

		for (i = 0; i < ALLOC_NUM_EXTS; i++)  {
			new->avl_node.avl_nextino = (avlnode_t *)
							ext_flist.list;
			ext_flist.list = new;
			ext_flist.cnt++;
			new++;
		}
	}

	ASSERT(ext_flist.list != NULL);

	new = ext_flist.list;
	ext_flist.list = (extent_tree_node_t *) new->avl_node.avl_nextino;
	ext_flist.cnt--;
	new->avl_node.avl_nextino = NULL;
	pthread_mutex_unlock(&ext_flist_lock);

	/* initialize node */

	new->ex_startblock = new_startblock;
	new->ex_blockcount = new_blockcount;
	new->ex_state = new_state;
	new->next = NULL;
	new->last = NULL;

	return(new);
}

void
release_extent_tree_node(extent_tree_node_t *node)
{
	pthread_mutex_lock(&ext_flist_lock);
	node->avl_node.avl_nextino = (avlnode_t *) ext_flist.list;
	ext_flist.list = node;
	ext_flist.cnt++;
	pthread_mutex_unlock(&ext_flist_lock);

	return;
}

/*
 * routines to recycle all nodes in a tree.  it walks the tree
 * and puts all nodes back on the free list so the nodes can be
 * reused.  the duplicate and bno/bcnt extent trees for each AG
 * are recycled after they're no longer needed to save memory
 */
void
release_extent_tree(avltree_desc_t *tree)
{
	extent_tree_node_t	*ext;
	extent_tree_node_t	*tmp;
	extent_tree_node_t	*lext;
	extent_tree_node_t	*ltmp;

	if (tree->avl_firstino == NULL)
		return;

	ext = (extent_tree_node_t *) tree->avl_firstino;

	while (ext != NULL)  {
		tmp = (extent_tree_node_t *) ext->avl_node.avl_nextino;

		/*
		 * ext->next is guaranteed to be set only in bcnt trees
		 */
		if (ext->next != NULL)  {
			lext = ext->next;
			while (lext != NULL)  {
				ltmp = lext->next;
				release_extent_tree_node(lext);
				lext = ltmp;
			}
		}

		release_extent_tree_node(ext);
		ext = tmp;
	}

	tree->avl_root = tree->avl_firstino = NULL;

	return;
}

/*
 * top-level (visible) routines
 */
void
release_agbno_extent_tree(xfs_agnumber_t agno)
{
	release_extent_tree(extent_bno_ptrs[agno]);

	return;
}

void
release_agbcnt_extent_tree(xfs_agnumber_t agno)
{
	release_extent_tree(extent_bcnt_ptrs[agno]);

	return;
}

/*
 * the next 4 routines manage the trees of free extents -- 2 trees
 * per AG.  The first tree is sorted by block number.  The second
 * tree is sorted by extent size.  This is the bno tree.
 */
void
add_bno_extent(xfs_agnumber_t agno, xfs_agblock_t startblock,
		xfs_extlen_t blockcount)
{
	extent_tree_node_t *ext;

	ASSERT(extent_bno_ptrs != NULL);
	ASSERT(extent_bno_ptrs[agno] != NULL);

	ext = mk_extent_tree_nodes(startblock, blockcount, XR_E_FREE);

	if (avl_insert(extent_bno_ptrs[agno], (avlnode_t *) ext) == NULL)  {
		do_error(_("duplicate bno extent range\n"));
	}
}

extent_tree_node_t *
findfirst_bno_extent(xfs_agnumber_t agno)
{
	ASSERT(extent_bno_ptrs != NULL);
	ASSERT(extent_bno_ptrs[agno] != NULL);

	return((extent_tree_node_t *) extent_bno_ptrs[agno]->avl_firstino);
}

extent_tree_node_t *
find_bno_extent(xfs_agnumber_t agno, xfs_agblock_t startblock)
{
	ASSERT(extent_bno_ptrs != NULL);
	ASSERT(extent_bno_ptrs[agno] != NULL);

	return((extent_tree_node_t *) avl_find(extent_bno_ptrs[agno],
						startblock));
}

/*
 * delete a node that's in the tree (pointer obtained by a find routine)
 */
void
get_bno_extent(xfs_agnumber_t agno, extent_tree_node_t *ext)
{
	ASSERT(extent_bno_ptrs != NULL);
	ASSERT(extent_bno_ptrs[agno] != NULL);

	avl_delete(extent_bno_ptrs[agno], &ext->avl_node);

	return;
}

/*
 * normalizing constant for bcnt size -> address conversion (see avl ops)
 * used by the AVL tree code to convert sizes and must be used when
 * doing an AVL search in the tree (e.g. avl_findrange(s))
 */
#define MAXBCNT		0xFFFFFFFF
#define BCNT_ADDR(cnt)	((unsigned int) MAXBCNT - (cnt))

/*
 * the next 4 routines manage the trees of free extents -- 2 trees
 * per AG.  The first tree is sorted by block number.  The second
 * tree is sorted by extent size.  This is the bcnt tree.
 */
void
add_bcnt_extent(xfs_agnumber_t agno, xfs_agblock_t startblock,
		xfs_extlen_t blockcount)
{
	extent_tree_node_t *ext, *prev, *current, *top;
	xfs_agblock_t		tmp_startblock;
	xfs_extlen_t		tmp_blockcount;
	extent_state_t		tmp_state;

	ASSERT(extent_bcnt_ptrs != NULL);
	ASSERT(extent_bcnt_ptrs[agno] != NULL);

	ext = mk_extent_tree_nodes(startblock, blockcount, XR_E_FREE);

	ASSERT(ext->next == NULL);

#ifdef XR_BCNT_TRACE
	fprintf(stderr, "adding bcnt: agno = %d, start = %u, count = %u\n",
			agno, startblock, blockcount);
#endif
	if ((current = (extent_tree_node_t *) avl_find(extent_bcnt_ptrs[agno],
							blockcount)) != NULL)  {
		/*
		 * avl tree code doesn't handle dups so insert
		 * onto linked list in increasing startblock order
		 *
		 * when called from mk_incore_fstree,
		 * startblock is in increasing order.
		 * current is an "anchor" node.
		 * quick check if the new ext goes to the end.
		 * if so, append at the end, using the last field
		 * of the "anchor".
		 */
		ASSERT(current->last != NULL);
		if (startblock > current->last->ex_startblock) {
			current->last->next = ext;
			current->last = ext;
			return;
		}

		/*
		 * scan, to find the proper location for new entry.
		 * this scan is *very* expensive and gets worse with
		 * with increasing entries.
		 */
		top = prev = current;
		while (current != NULL &&
				startblock > current->ex_startblock)  {
			prev = current;
			current = current->next;
		}

		if (top == current)  {
			ASSERT(top == prev);
			/*
			 * new entry should be ahead of current.
			 * to keep the avl tree intact,
			 * swap the values of to-be-inserted element
			 * and the values of the head of the list.
			 * then insert as the 2nd element on the list.
			 *
			 * see the comment in get_bcnt_extent()
			 * as to why we have to do this.
			 */
			tmp_startblock = top->ex_startblock;
			tmp_blockcount = top->ex_blockcount;
			tmp_state = top->ex_state;

			top->ex_startblock = ext->ex_startblock;
			top->ex_blockcount = ext->ex_blockcount;
			top->ex_state = ext->ex_state;

			ext->ex_startblock = tmp_startblock;
			ext->ex_blockcount = tmp_blockcount;
			ext->ex_state = tmp_state;

			current = top->next;
			prev = top;
		}

		prev->next = ext;
		ext->next = current;

		return;
	}

	if (avl_insert(extent_bcnt_ptrs[agno], (avlnode_t *) ext) == NULL)  {
		do_error(_(":  duplicate bno extent range\n"));
	}

	ext->last = ext;	/* ext is an "anchor" node */

	return;
}

extent_tree_node_t *
findfirst_bcnt_extent(xfs_agnumber_t agno)
{
	ASSERT(extent_bcnt_ptrs != NULL);
	ASSERT(extent_bcnt_ptrs[agno] != NULL);

	return((extent_tree_node_t *) extent_bcnt_ptrs[agno]->avl_firstino);
}

extent_tree_node_t *
findbiggest_bcnt_extent(xfs_agnumber_t agno)
{
	extern avlnode_t *avl_lastino(avlnode_t *root);

	ASSERT(extent_bcnt_ptrs != NULL);
	ASSERT(extent_bcnt_ptrs[agno] != NULL);

	return((extent_tree_node_t *) avl_lastino(extent_bcnt_ptrs[agno]->avl_root));
}

extent_tree_node_t *
findnext_bcnt_extent(xfs_agnumber_t agno, extent_tree_node_t *ext)
{
	avlnode_t *nextino;

	if (ext->next != NULL)  {
		ASSERT(ext->ex_blockcount == ext->next->ex_blockcount);
		ASSERT(ext->ex_startblock < ext->next->ex_startblock);
		return(ext->next);
	} else  {
		/*
		 * have to look at the top of the list to get the
		 * correct avl_nextino pointer since that pointer
		 * is maintained and altered by the AVL code.
		 */
		nextino = avl_find(extent_bcnt_ptrs[agno], ext->ex_blockcount);
		ASSERT(nextino != NULL);
		if (nextino->avl_nextino != NULL)  {
			ASSERT(ext->ex_blockcount < ((extent_tree_node_t *)
					nextino->avl_nextino)->ex_blockcount);
		}
		return((extent_tree_node_t *) nextino->avl_nextino);
	}
}

/*
 * this is meant to be called after you walk the bno tree to
 * determine exactly which extent you want (so you'll know the
 * desired value for startblock when you call this routine).
 */
extent_tree_node_t *
get_bcnt_extent(xfs_agnumber_t agno, xfs_agblock_t startblock,
		xfs_extlen_t blockcount)
{
	extent_tree_node_t	*ext, *prev, *top;
	xfs_agblock_t		tmp_startblock;
	xfs_extlen_t		tmp_blockcount;
	extent_state_t		tmp_state;

	prev = NULL;
	ASSERT(extent_bcnt_ptrs != NULL);
	ASSERT(extent_bcnt_ptrs[agno] != NULL);

	if ((ext = (extent_tree_node_t *) avl_find(extent_bcnt_ptrs[agno],
							blockcount)) == NULL)
		return(NULL);

	top = ext;

	if (ext->next != NULL)  {
		/*
		 * pull it off the list
		 */
		while (ext != NULL && startblock != ext->ex_startblock)  {
			prev = ext;
			ext = ext->next;
		}
		ASSERT(ext != NULL);
		if (ext == top)  {
			/*
			 * this node is linked into the tree so we
			 * swap the core values so we can delete
			 * the next item on the list instead of
			 * the head of the list.  This is because
			 * the rest of the tree undoubtedly has
			 * pointers to the piece of memory that
			 * is the head of the list so pulling
			 * the item out of the list and hence
			 * the avl tree would be a bad idea.
			 *
			 * (cheaper than the alternative, a tree
			 * delete of this node followed by a tree
			 * insert of the next node on the list).
			 */
			tmp_startblock = ext->next->ex_startblock;
			tmp_blockcount = ext->next->ex_blockcount;
			tmp_state = ext->next->ex_state;

			ext->next->ex_startblock = ext->ex_startblock;
			ext->next->ex_blockcount = ext->ex_blockcount;
			ext->next->ex_state = ext->ex_state;

			ext->ex_startblock = tmp_startblock;
			ext->ex_blockcount = tmp_blockcount;
			ext->ex_state = tmp_state;

			ext = ext->next;
			prev = top;
		}
		/*
		 * now, a simple list deletion
		 */
		prev->next = ext->next;
		ext->next = NULL;
	} else  {
		/*
		 * no list, just one node.  simply delete
		 */
		avl_delete(extent_bcnt_ptrs[agno], &ext->avl_node);
	}

	ASSERT(ext->ex_startblock == startblock);
	ASSERT(ext->ex_blockcount == blockcount);
	return(ext);
}

static __psunsigned_t
avl_ext_start(avlnode_t *node)
{
	return((__psunsigned_t)
		((extent_tree_node_t *) node)->ex_startblock);
}

static __psunsigned_t
avl_ext_end(avlnode_t *node)
{
	return((__psunsigned_t) (
		((extent_tree_node_t *) node)->ex_startblock +
		((extent_tree_node_t *) node)->ex_blockcount));
}

/*
 * convert size to an address for the AVL tree code -- the bigger the size,
 * the lower the address so the biggest extent will be first in the tree
 */
static __psunsigned_t
avl_ext_bcnt_start(avlnode_t *node)
{
/*
	return((__psunsigned_t) (BCNT_ADDR(((extent_tree_node_t *)
						node)->ex_blockcount)));
*/
	return((__psunsigned_t) ((extent_tree_node_t *)node)->ex_blockcount);
}

static __psunsigned_t
avl_ext_bcnt_end(avlnode_t *node)
{
/*
	return((__psunsigned_t) (BCNT_ADDR(((extent_tree_node_t *)
						node)->ex_blockcount)));
*/
	return((__psunsigned_t) ((extent_tree_node_t *)node)->ex_blockcount);
}

avlops_t avl_extent_bcnt_tree_ops = {
	avl_ext_bcnt_start,
	avl_ext_bcnt_end
};

avlops_t avl_extent_tree_ops = {
	avl_ext_start,
	avl_ext_end
};

/*
 * for real-time extents -- have to dup code since realtime extent
 * startblocks can be 64-bit values.
 */
static rt_extent_tree_node_t *
mk_rt_extent_tree_nodes(xfs_drtbno_t new_startblock,
	xfs_extlen_t new_blockcount, extent_state_t new_state)
{
	int i;
	rt_extent_tree_node_t *new;
	rt_extent_alloc_rec_t *rec;

	pthread_mutex_lock(&rt_ext_flist_lock);
	if (rt_ext_flist.cnt == 0)  {
		ASSERT(rt_ext_flist.list == NULL);

		if ((rec = malloc(sizeof(rt_extent_alloc_rec_t))) == NULL)
			do_error(
			_("couldn't allocate new extent descriptors.\n"));

		list_add(&rec->list, &rt_ba_list);

		new = &rec->extents[0];

		for (i = 0; i < ALLOC_NUM_EXTS; i++)  {
			new->avl_node.avl_nextino = (avlnode_t *)
							rt_ext_flist.list;
			rt_ext_flist.list = new;
			rt_ext_flist.cnt++;
			new++;
		}
	}

	ASSERT(rt_ext_flist.list != NULL);

	new = rt_ext_flist.list;
	rt_ext_flist.list = (rt_extent_tree_node_t *) new->avl_node.avl_nextino;
	rt_ext_flist.cnt--;
	new->avl_node.avl_nextino = NULL;
	pthread_mutex_unlock(&rt_ext_flist_lock);

	/* initialize node */

	new->rt_startblock = new_startblock;
	new->rt_blockcount = new_blockcount;
	new->rt_state = new_state;

	return(new);
}

#if 0
void
release_rt_extent_tree_node(rt_extent_tree_node_t *node)
{
	node->avl_node.avl_nextino = (avlnode_t *) rt_ext_flist.list;
	rt_ext_flist.list = node;
	rt_ext_flist.cnt++;

	return;
}

void
release_rt_extent_tree()
{
	extent_tree_node_t	*ext;
	extent_tree_node_t	*tmp;
	extent_tree_node_t	*lext;
	extent_tree_node_t	*ltmp;
	avl64tree_desc_t	*tree;

	tree = rt_extent_tree_ptr;

	if (tree->avl_firstino == NULL)
		return;

	ext = (extent_tree_node_t *) tree->avl_firstino;

	while (ext != NULL)  {
		tmp = (extent_tree_node_t *) ext->avl_node.avl_nextino;
		release_rt_extent_tree_node(ext);
		ext = tmp;
	}

	tree->avl_root = tree->avl_firstino = NULL;

	return;
}
#endif

/*
 * don't need release functions for realtime tree teardown
 * since we only have one tree, not one per AG
 */
/* ARGSUSED */
void
free_rt_dup_extent_tree(xfs_mount_t *mp)
{
	rt_extent_alloc_rec_t *cur, *tmp;

	ASSERT(mp->m_sb.sb_rblocks != 0);

	list_for_each_entry_safe(cur, tmp, &rt_ba_list, list)
		free(cur);

	free(rt_ext_tree_ptr);

	rt_ext_tree_ptr = NULL;

	return;
}

/*
 * add a duplicate real-time extent
 */
void
add_rt_dup_extent(xfs_drtbno_t startblock, xfs_extlen_t blockcount)
{
	rt_extent_tree_node_t *first, *last, *ext, *next_ext;
	xfs_drtbno_t new_startblock;
	xfs_extlen_t new_blockcount;

	pthread_mutex_lock(&rt_ext_tree_lock);
	avl64_findranges(rt_ext_tree_ptr, startblock - 1,
		startblock + blockcount + 1,
		(avl64node_t **) &first, (avl64node_t **) &last);
	/*
	 * find adjacent and overlapping extent blocks
	 */
	if (first == NULL && last == NULL)  {
		/* nothing, just make and insert new extent */

		ext = mk_rt_extent_tree_nodes(startblock,
				blockcount, XR_E_MULT);

		if (avl64_insert(rt_ext_tree_ptr,
				(avl64node_t *) ext) == NULL)  {
			do_error(_("duplicate extent range\n"));
		}

		pthread_mutex_unlock(&rt_ext_tree_lock);
		return;
	}

	ASSERT(first != NULL && last != NULL);

	/*
	 * find the new composite range, delete old extent nodes
	 * as we go
	 */
	new_startblock = startblock;
	new_blockcount = blockcount;

	for (ext = first;
		ext != (rt_extent_tree_node_t *) last->avl_node.avl_nextino;
		ext = next_ext)  {
		/*
		 * preserve the next inorder node
		 */
		next_ext = (rt_extent_tree_node_t *) ext->avl_node.avl_nextino;
		/*
		 * just bail if the new extent is contained within an old one
		 */
		if (ext->rt_startblock <= startblock &&
				ext->rt_blockcount >= blockcount) {
			pthread_mutex_unlock(&rt_ext_tree_lock);
			return;
		}
		/*
		 * now check for overlaps and adjacent extents
		 */
		if (ext->rt_startblock + ext->rt_blockcount >= startblock
			|| ext->rt_startblock <= startblock + blockcount)  {

			if (ext->rt_startblock < new_startblock)
				new_startblock = ext->rt_startblock;

			if (ext->rt_startblock + ext->rt_blockcount >
					new_startblock + new_blockcount)
				new_blockcount = ext->rt_startblock +
							ext->rt_blockcount -
							new_startblock;

			avl64_delete(rt_ext_tree_ptr, (avl64node_t *) ext);
			continue;
		}
	}

	ext = mk_rt_extent_tree_nodes(new_startblock,
				new_blockcount, XR_E_MULT);

	if (avl64_insert(rt_ext_tree_ptr, (avl64node_t *) ext) == NULL)  {
		do_error(_("duplicate extent range\n"));
	}

	pthread_mutex_unlock(&rt_ext_tree_lock);
	return;
}

/*
 * returns 1 if block is a dup, 0 if not
 */
/* ARGSUSED */
int
search_rt_dup_extent(xfs_mount_t *mp, xfs_drtbno_t bno)
{
	int ret;

	pthread_mutex_lock(&rt_ext_tree_lock);
	if (avl64_findrange(rt_ext_tree_ptr, bno) != NULL)
		ret = 1;
	else
		ret = 0;
	pthread_mutex_unlock(&rt_ext_tree_lock);
	return(ret);
}

static __uint64_t
avl64_rt_ext_start(avl64node_t *node)
{
	return(((rt_extent_tree_node_t *) node)->rt_startblock);
}

static __uint64_t
avl64_ext_end(avl64node_t *node)
{
	return(((rt_extent_tree_node_t *) node)->rt_startblock +
		((rt_extent_tree_node_t *) node)->rt_blockcount);
}

avl64ops_t avl64_extent_tree_ops = {
	avl64_rt_ext_start,
	avl64_ext_end
};

void
incore_ext_init(xfs_mount_t *mp)
{
	int i;
	xfs_agnumber_t agcount = mp->m_sb.sb_agcount;

	list_head_init(&ba_list);
	list_head_init(&rt_ba_list);
	pthread_mutex_init(&ext_flist_lock, NULL);
	pthread_mutex_init(&rt_ext_tree_lock, NULL);
	pthread_mutex_init(&rt_ext_flist_lock, NULL);

	dup_extent_trees = calloc(agcount, sizeof(struct btree_root *));
	if (!dup_extent_trees)
		do_error(_("couldn't malloc dup extent tree descriptor table\n"));

	dup_extent_tree_locks = calloc(agcount, sizeof(pthread_mutex_t));
	if (!dup_extent_tree_locks)
		do_error(_("couldn't malloc dup extent tree descriptor table\n"));

	if ((extent_bno_ptrs = malloc(agcount *
					sizeof(avltree_desc_t *))) == NULL)
		do_error(
	_("couldn't malloc free by-bno extent tree descriptor table\n"));

	if ((extent_bcnt_ptrs = malloc(agcount *
					sizeof(avltree_desc_t *))) == NULL)
		do_error(
	_("couldn't malloc free by-bcnt extent tree descriptor table\n"));

	for (i = 0; i < agcount; i++)  {
		if ((extent_bno_ptrs[i] =
				malloc(sizeof(avltree_desc_t))) == NULL)
			do_error(
			_("couldn't malloc bno extent tree descriptor\n"));
		if ((extent_bcnt_ptrs[i] =
				malloc(sizeof(avltree_desc_t))) == NULL)
			do_error(
			_("couldn't malloc bcnt extent tree descriptor\n"));
	}

	for (i = 0; i < agcount; i++)  {
		btree_init(&dup_extent_trees[i]);
		pthread_mutex_init(&dup_extent_tree_locks[i], NULL);
		avl_init_tree(extent_bno_ptrs[i], &avl_extent_tree_ops);
		avl_init_tree(extent_bcnt_ptrs[i], &avl_extent_bcnt_tree_ops);
	}

	if ((rt_ext_tree_ptr = malloc(sizeof(avltree_desc_t))) == NULL)
		do_error(_("couldn't malloc dup rt extent tree descriptor\n"));

	avl64_init_tree(rt_ext_tree_ptr, &avl64_extent_tree_ops);

	ext_flist.cnt = 0;
	ext_flist.list = NULL;

	return;
}

/*
 * this routine actually frees all the memory used to track per-AG trees
 */
void
incore_ext_teardown(xfs_mount_t *mp)
{
	extent_alloc_rec_t *cur, *tmp;
	xfs_agnumber_t i;

	list_for_each_entry_safe(cur, tmp, &ba_list, list)
		free(cur);

	for (i = 0; i < mp->m_sb.sb_agcount; i++)  {
		btree_destroy(dup_extent_trees[i]);
		free(extent_bno_ptrs[i]);
		free(extent_bcnt_ptrs[i]);
	}

	free(dup_extent_trees);
	free(extent_bcnt_ptrs);
	free(extent_bno_ptrs);

	dup_extent_trees = NULL;
	extent_bcnt_ptrs = NULL;
	extent_bno_ptrs = NULL;
}

int
count_extents(xfs_agnumber_t agno, avltree_desc_t *tree, int whichtree)
{
	extent_tree_node_t *node;
	int i = 0;

	node = (extent_tree_node_t *) tree->avl_firstino;

	while (node != NULL)  {
		i++;
		if (whichtree)
			node = findnext_bcnt_extent(agno, node);
		else
			node = findnext_bno_extent(node);
	}

	return(i);
}

int
count_bno_extents_blocks(xfs_agnumber_t agno, uint *numblocks)
{
	__uint64_t nblocks;
	extent_tree_node_t *node;
	int i = 0;

	ASSERT(agno < glob_agcount);

	nblocks = 0;

	node = (extent_tree_node_t *) extent_bno_ptrs[agno]->avl_firstino;

	while (node != NULL) {
		nblocks += node->ex_blockcount;
		i++;
		node = findnext_bno_extent(node);
	}

	*numblocks = nblocks;
	return(i);
}

int
count_bno_extents(xfs_agnumber_t agno)
{
	ASSERT(agno < glob_agcount);
	return(count_extents(agno, extent_bno_ptrs[agno], 0));
}

int
count_bcnt_extents(xfs_agnumber_t agno)
{
	ASSERT(agno < glob_agcount);
	return(count_extents(agno, extent_bcnt_ptrs[agno], 1));
}
