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

#include <libxfs.h>
#include "avl.h"
#include "globals.h"
#include "incore.h"
#include "agheader.h"
#include "protos.h"
#include "threads.h"
#include "err_protos.h"

static pthread_mutex_t	ino_flist_lock;
extern avlnode_t	*avl_firstino(avlnode_t *root);

/*
 * array of inode tree ptrs, one per ag
 */
avltree_desc_t	**inode_tree_ptrs;

/*
 * ditto for uncertain inodes
 */
static avltree_desc_t	**inode_uncertain_tree_ptrs;

#define ALLOC_NUM_INOS		100

/* free lists -- inode nodes and extent nodes */

typedef struct ino_flist_s  {
	ino_tree_node_t		*list;
	ino_tree_node_t		*last;
	long long		cnt;
} ino_flist_t;

static ino_flist_t ino_flist;	/* free list must be initialized before use */

/* memory optimised nlink counting for all inodes */

static void nlink_grow_8_to_16(ino_tree_node_t *irec);
static void nlink_grow_16_to_32(ino_tree_node_t *irec);

static void
disk_nlink_32_set(ino_tree_node_t *irec, int ino_offset, __uint32_t nlinks)
{
	((__uint32_t*)irec->disk_nlinks)[ino_offset] = nlinks;
}

static __uint32_t
disk_nlink_32_get(ino_tree_node_t *irec, int ino_offset)
{
	return ((__uint32_t*)irec->disk_nlinks)[ino_offset];
}

static __uint32_t
counted_nlink_32_get(ino_tree_node_t *irec, int ino_offset)
{
	return ((__uint32_t*)irec->ino_un.ex_data->counted_nlinks)[ino_offset];
}

static __uint32_t
counted_nlink_32_inc(ino_tree_node_t *irec, int ino_offset)
{
	return ++(((__uint32_t*)irec->ino_un.ex_data->counted_nlinks)[ino_offset]);
}

static __uint32_t
counted_nlink_32_dec(ino_tree_node_t *irec, int ino_offset)
{
	__uint32_t *nlinks = (__uint32_t*)irec->ino_un.ex_data->counted_nlinks;

	ASSERT(nlinks[ino_offset] > 0);
	return --(nlinks[ino_offset]);
}


static void
disk_nlink_16_set(ino_tree_node_t *irec, int ino_offset, __uint32_t nlinks)
{
	if (nlinks >= 0x10000) {
		nlink_grow_16_to_32(irec);
		disk_nlink_32_set(irec, ino_offset, nlinks);
	} else
		((__uint16_t*)irec->disk_nlinks)[ino_offset] = nlinks;
}

static __uint32_t
disk_nlink_16_get(ino_tree_node_t *irec, int ino_offset)
{
	return ((__uint16_t*)irec->disk_nlinks)[ino_offset];
}

static __uint32_t
counted_nlink_16_get(ino_tree_node_t *irec, int ino_offset)
{
	return ((__uint16_t*)irec->ino_un.ex_data->counted_nlinks)[ino_offset];
}

static __uint32_t
counted_nlink_16_inc(ino_tree_node_t *irec, int ino_offset)
{
	__uint16_t *nlinks = (__uint16_t*)irec->ino_un.ex_data->counted_nlinks;

	if (nlinks[ino_offset] == 0xffff) {
		nlink_grow_16_to_32(irec);
		return counted_nlink_32_inc(irec, ino_offset);
	}
	return ++(nlinks[ino_offset]);
}

static __uint32_t
counted_nlink_16_dec(ino_tree_node_t *irec, int ino_offset)
{
	__uint16_t *nlinks = (__uint16_t*)irec->ino_un.ex_data->counted_nlinks;

	ASSERT(nlinks[ino_offset] > 0);
	return --(nlinks[ino_offset]);
}


static void
disk_nlink_8_set(ino_tree_node_t *irec, int ino_offset, __uint32_t nlinks)
{
	if (nlinks >= 0x100) {
		nlink_grow_8_to_16(irec);
		disk_nlink_16_set(irec, ino_offset, nlinks);
	} else
		irec->disk_nlinks[ino_offset] = nlinks;
}

static __uint32_t
disk_nlink_8_get(ino_tree_node_t *irec, int ino_offset)
{
	return irec->disk_nlinks[ino_offset];
}

static __uint32_t
counted_nlink_8_get(ino_tree_node_t *irec, int ino_offset)
{
	return irec->ino_un.ex_data->counted_nlinks[ino_offset];
}

static __uint32_t
counted_nlink_8_inc(ino_tree_node_t *irec, int ino_offset)
{
	if (irec->ino_un.ex_data->counted_nlinks[ino_offset] == 0xff) {
		nlink_grow_8_to_16(irec);
		return counted_nlink_16_inc(irec, ino_offset);
	}
	return ++(irec->ino_un.ex_data->counted_nlinks[ino_offset]);
}

static __uint32_t
counted_nlink_8_dec(ino_tree_node_t *irec, int ino_offset)
{
	ASSERT(irec->ino_un.ex_data->counted_nlinks[ino_offset] > 0);
	return --(irec->ino_un.ex_data->counted_nlinks[ino_offset]);
}


static nlink_ops_t nlinkops[] = {
	{sizeof(__uint8_t) * XFS_INODES_PER_CHUNK,
		disk_nlink_8_set, disk_nlink_8_get,
		counted_nlink_8_get, counted_nlink_8_inc, counted_nlink_8_dec},
	{sizeof(__uint16_t) * XFS_INODES_PER_CHUNK,
		disk_nlink_16_set, disk_nlink_16_get,
		counted_nlink_16_get, counted_nlink_16_inc, counted_nlink_16_dec},
	{sizeof(__uint32_t) * XFS_INODES_PER_CHUNK,
		disk_nlink_32_set, disk_nlink_32_get,
		counted_nlink_32_get, counted_nlink_32_inc, counted_nlink_32_dec},
};

static void
nlink_grow_8_to_16(ino_tree_node_t *irec)
{
	__uint16_t	*new_nlinks;
	int		i;

	new_nlinks = malloc(sizeof(__uint16_t) * XFS_INODES_PER_CHUNK);
	if (new_nlinks == NULL)
		do_error(_("could not allocate expanded nlink array\n"));
	for (i = 0; i < XFS_INODES_PER_CHUNK; i++)
		new_nlinks[i] = irec->disk_nlinks[i];
	free(irec->disk_nlinks);
	irec->disk_nlinks = (__uint8_t*)new_nlinks;

	if (full_ino_ex_data) {
		new_nlinks = malloc(sizeof(__uint16_t) * XFS_INODES_PER_CHUNK);
		if (new_nlinks == NULL)
			do_error(_("could not allocate expanded nlink array\n"));
		for (i = 0; i < XFS_INODES_PER_CHUNK; i++)
			new_nlinks[i] = irec->ino_un.ex_data->counted_nlinks[i];
		free(irec->ino_un.ex_data->counted_nlinks);
		irec->ino_un.ex_data->counted_nlinks = (__uint8_t*)new_nlinks;
	}
	irec->nlinkops = &nlinkops[1];
}

static void
nlink_grow_16_to_32(ino_tree_node_t *irec)
{
	__uint32_t	*new_nlinks;
	int		i;

	new_nlinks = malloc(sizeof(__uint32_t) * XFS_INODES_PER_CHUNK);
	if (new_nlinks == NULL)
		do_error(_("could not allocate expanded nlink array\n"));
	for (i = 0; i < XFS_INODES_PER_CHUNK; i++)
		new_nlinks[i] = ((__int16_t*)&irec->disk_nlinks)[i];
	free(irec->disk_nlinks);
	irec->disk_nlinks = (__uint8_t*)new_nlinks;

	if (full_ino_ex_data) {
		new_nlinks = malloc(sizeof(__uint32_t) * XFS_INODES_PER_CHUNK);
		if (new_nlinks == NULL)
			do_error(_("could not allocate expanded nlink array\n"));
		for (i = 0; i < XFS_INODES_PER_CHUNK; i++)
			new_nlinks[i] = ((__int16_t*)&irec->ino_un.ex_data->counted_nlinks)[i];
		free(irec->ino_un.ex_data->counted_nlinks);
		irec->ino_un.ex_data->counted_nlinks = (__uint8_t*)new_nlinks;
	}
	irec->nlinkops = &nlinkops[2];
}

/*
 * next is the uncertain inode list -- a sorted (in ascending order)
 * list of inode records sorted on the starting inode number.  There
 * is one list per ag.
 */

/*
 * common code for creating inode records for use by trees and lists.
 * called only from add_inodes and add_inodes_uncertain
 *
 * IMPORTANT:  all inodes (inode records) start off as free and
 *		unconfirmed.
 */
/* ARGSUSED */
static ino_tree_node_t *
mk_ino_tree_nodes(
	xfs_agino_t		starting_ino)
{
	int 			i;
	ino_tree_node_t 	*ino_rec;
	avlnode_t 		*node;

	pthread_mutex_lock(&ino_flist_lock);
	if (ino_flist.cnt == 0)  {
		ASSERT(ino_flist.list == NULL);

		if ((ino_rec = malloc(sizeof(ino_tree_node_t[ALLOC_NUM_INOS])))
					== NULL)
			do_error(_("inode map malloc failed\n"));

		for (i = 0; i < ALLOC_NUM_INOS; i++)  {
			ino_rec->avl_node.avl_nextino =
				(avlnode_t *) ino_flist.list;
			ino_flist.list = ino_rec;
			ino_flist.cnt++;
			ino_rec++;
		}
	}

	ASSERT(ino_flist.list != NULL);

	ino_rec = ino_flist.list;
	ino_flist.list = (ino_tree_node_t *) ino_rec->avl_node.avl_nextino;
	ino_flist.cnt--;
	node = &ino_rec->avl_node;
	node->avl_nextino = node->avl_forw = node->avl_back = NULL;
	pthread_mutex_unlock(&ino_flist_lock);

	/* initialize node */

	ino_rec->ino_startnum = 0;
	ino_rec->ino_confirmed = 0;
	ino_rec->ino_isa_dir = 0;
	ino_rec->ir_free = (xfs_inofree_t) - 1;
	ino_rec->ino_un.ex_data = NULL;
	ino_rec->nlinkops = &nlinkops[0];
	ino_rec->disk_nlinks = calloc(1, nlinkops[0].nlink_size);
	if (ino_rec->disk_nlinks == NULL)
		do_error(_("could not allocate nlink array\n"));

	return(ino_rec);
}

/*
 * return inode record to free list, will be initialized when
 * it gets pulled off list
 */
static void
free_ino_tree_node(ino_tree_node_t *ino_rec)
{
	ino_rec->avl_node.avl_nextino = NULL;
	ino_rec->avl_node.avl_forw = NULL;
	ino_rec->avl_node.avl_back = NULL;

	pthread_mutex_lock(&ino_flist_lock);
	if (ino_flist.list != NULL)  {
		ASSERT(ino_flist.cnt > 0);
		ino_rec->avl_node.avl_nextino = (avlnode_t *) ino_flist.list;
	} else  {
		ASSERT(ino_flist.cnt == 0);
		ino_rec->avl_node.avl_nextino = NULL;
	}

	ino_flist.list = ino_rec;
	ino_flist.cnt++;

	free(ino_rec->disk_nlinks);

	if (ino_rec->ino_un.ex_data != NULL)  {
		if (full_ino_ex_data) {
			free(ino_rec->ino_un.ex_data->parents);
			free(ino_rec->ino_un.ex_data->counted_nlinks);
		}
		free(ino_rec->ino_un.ex_data);

	}
	pthread_mutex_unlock(&ino_flist_lock);
}

/*
 * last referenced cache for uncertain inodes
 */
static ino_tree_node_t **last_rec;

/*
 * ok, the uncertain inodes are a set of trees just like the
 * good inodes but all starting inode records are (arbitrarily)
 * aligned on XFS_CHUNK_PER_INODE boundaries to prevent overlaps.
 * this means we may have partials records in the tree (e.g. records
 * without 64 confirmed uncertain inodes).  Tough.
 *
 * free is set to 1 if the inode is thought to be free, 0 if used
 */
void
add_aginode_uncertain(xfs_agnumber_t agno, xfs_agino_t ino, int free)
{
	ino_tree_node_t		*ino_rec;
	xfs_agino_t		s_ino;
	int			offset;

	ASSERT(agno < glob_agcount);
	ASSERT(last_rec != NULL);

	s_ino = rounddown(ino, XFS_INODES_PER_CHUNK);

	/*
	 * check for a cache hit
	 */
	if (last_rec[agno] != NULL && last_rec[agno]->ino_startnum == s_ino)  {
		offset = ino - s_ino;
		if (free)
			set_inode_free(last_rec[agno], offset);
		else
			set_inode_used(last_rec[agno], offset);

		return;
	}

	/*
	 * check to see if record containing inode is already in the tree.
	 * if not, add it
	 */
	if ((ino_rec = (ino_tree_node_t *)
			avl_findrange(inode_uncertain_tree_ptrs[agno],
				s_ino)) == NULL)  {
		ino_rec = mk_ino_tree_nodes(s_ino);
		ino_rec->ino_startnum = s_ino;

		if (avl_insert(inode_uncertain_tree_ptrs[agno],
				(avlnode_t *) ino_rec) == NULL)  {
			do_error(_("add_aginode_uncertain - "
				   "duplicate inode range\n"));
		}
	}

	if (free)
		set_inode_free(ino_rec, ino - s_ino);
	else
		set_inode_used(ino_rec, ino - s_ino);

	/*
	 * set cache entry
	 */
	last_rec[agno] = ino_rec;
}

/*
 * like add_aginode_uncertain() only it needs an xfs_mount_t *
 * to perform the inode number conversion.
 */
void
add_inode_uncertain(xfs_mount_t *mp, xfs_ino_t ino, int free)
{
	add_aginode_uncertain(XFS_INO_TO_AGNO(mp, ino),
				XFS_INO_TO_AGINO(mp, ino), free);
}

/*
 * pull the indicated inode record out of the uncertain inode tree
 */
void
get_uncertain_inode_rec(struct xfs_mount *mp, xfs_agnumber_t agno,
			ino_tree_node_t *ino_rec)
{
	ASSERT(inode_tree_ptrs != NULL);
	ASSERT(agno < mp->m_sb.sb_agcount);
	ASSERT(inode_tree_ptrs[agno] != NULL);

	avl_delete(inode_uncertain_tree_ptrs[agno], &ino_rec->avl_node);

	ino_rec->avl_node.avl_nextino = NULL;
	ino_rec->avl_node.avl_forw = NULL;
	ino_rec->avl_node.avl_back = NULL;
}

ino_tree_node_t *
findfirst_uncertain_inode_rec(xfs_agnumber_t agno)
{
	return((ino_tree_node_t *)
		inode_uncertain_tree_ptrs[agno]->avl_firstino);
}

ino_tree_node_t *
find_uncertain_inode_rec(xfs_agnumber_t agno, xfs_agino_t ino)
{
	return((ino_tree_node_t *)
		avl_findrange(inode_uncertain_tree_ptrs[agno], ino));
}

void
clear_uncertain_ino_cache(xfs_agnumber_t agno)
{
	last_rec[agno] = NULL;
}


/*
 * next comes the inode trees.  One per ag.  AVL trees
 * of inode records, each inode record tracking 64 inodes
 */
/*
 * set up an inode tree record for a group of inodes that will
 * include the requested inode.
 *
 * does NOT error-check for duplicate records.  Caller is
 * responsible for checking that.
 *
 * ino must be the start of an XFS_INODES_PER_CHUNK (64) inode chunk
 *
 * Each inode resides in a 64-inode chunk which can be part
 * one or more chunks (MAX(64, inodes-per-block).  The fs allocates
 * in chunks (as opposed to 1 chunk) when a block can hold more than
 * one chunk (inodes per block > 64).  Allocating in one chunk pieces
 * causes us problems when it takes more than one fs block to contain
 * an inode chunk because the chunks can start on *any* block boundary.
 * So we assume that the caller has a clue because at this level, we
 * don't.
 */
static ino_tree_node_t *
add_inode(struct xfs_mount *mp, xfs_agnumber_t agno, xfs_agino_t ino)
{
	ino_tree_node_t *ino_rec;

	/* no record exists, make some and put them into the tree */

	ino_rec = mk_ino_tree_nodes(ino);
	ino_rec->ino_startnum = ino;

	if (avl_insert(inode_tree_ptrs[agno],
			(avlnode_t *) ino_rec) == NULL)  {
		do_warn(_("add_inode - duplicate inode range\n"));
	}

	return(ino_rec);
}

/*
 * pull the indicated inode record out of the inode tree
 */
void
get_inode_rec(struct xfs_mount *mp, xfs_agnumber_t agno, ino_tree_node_t *ino_rec)
{
	ASSERT(inode_tree_ptrs != NULL);
	ASSERT(agno < mp->m_sb.sb_agcount);
	ASSERT(inode_tree_ptrs[agno] != NULL);

	avl_delete(inode_tree_ptrs[agno], &ino_rec->avl_node);

	ino_rec->avl_node.avl_nextino = NULL;
	ino_rec->avl_node.avl_forw = NULL;
	ino_rec->avl_node.avl_back = NULL;
}

/*
 * free the designated inode record (return it to the free pool)
 */
/* ARGSUSED */
void
free_inode_rec(xfs_agnumber_t agno, ino_tree_node_t *ino_rec)
{
	free_ino_tree_node(ino_rec);
}

void
find_inode_rec_range(struct xfs_mount *mp, xfs_agnumber_t agno,
			xfs_agino_t start_ino, xfs_agino_t end_ino,
			ino_tree_node_t **first, ino_tree_node_t **last)
{
	*first = *last = NULL;

	/*
	 * Is the AG inside the file system ?
	 */
	if (agno < mp->m_sb.sb_agcount)
		avl_findranges(inode_tree_ptrs[agno], start_ino,
			end_ino, (avlnode_t **) first, (avlnode_t **) last);
}

/*
 * if ino doesn't exist, it must be properly aligned -- on a
 * filesystem block boundary or XFS_INODES_PER_CHUNK boundary,
 * whichever alignment is larger.
 */
ino_tree_node_t *
set_inode_used_alloc(struct xfs_mount *mp, xfs_agnumber_t agno, xfs_agino_t ino)
{
	ino_tree_node_t *ino_rec;

	/*
	 * check alignment -- the only way to detect this
	 * is too see if the chunk overlaps another chunk
	 * already in the tree
	 */
	ino_rec = add_inode(mp, agno, ino);

	ASSERT(ino_rec != NULL);
	ASSERT(ino >= ino_rec->ino_startnum &&
		ino - ino_rec->ino_startnum < XFS_INODES_PER_CHUNK);

	set_inode_used(ino_rec, ino - ino_rec->ino_startnum);

	return(ino_rec);
}

ino_tree_node_t *
set_inode_free_alloc(struct xfs_mount *mp, xfs_agnumber_t agno, xfs_agino_t ino)
{
	ino_tree_node_t *ino_rec;

	ino_rec = add_inode(mp, agno, ino);

	ASSERT(ino_rec != NULL);
	ASSERT(ino >= ino_rec->ino_startnum &&
		ino - ino_rec->ino_startnum < XFS_INODES_PER_CHUNK);

	set_inode_free(ino_rec, ino - ino_rec->ino_startnum);

	return(ino_rec);
}

void
print_inode_list_int(xfs_agnumber_t agno, int uncertain)
{
	ino_tree_node_t *ino_rec;

	if (!uncertain)  {
		fprintf(stderr, _("good inode list is --\n"));
		ino_rec = findfirst_inode_rec(agno);
	} else  {
		fprintf(stderr, _("uncertain inode list is --\n"));
		ino_rec = findfirst_uncertain_inode_rec(agno);
	}

	if (ino_rec == NULL)  {
		fprintf(stderr, _("agno %d -- no inodes\n"), agno);
		return;
	}

	printf(_("agno %d\n"), agno);

	while(ino_rec != NULL)  {
		fprintf(stderr,
	_("\tptr = %lx, start = 0x%x, free = 0x%llx, confirmed = 0x%llx\n"),
			(unsigned long)ino_rec,
			ino_rec->ino_startnum,
			(unsigned long long)ino_rec->ir_free,
			(unsigned long long)ino_rec->ino_confirmed);
		if (ino_rec->ino_startnum == 0)
			ino_rec = ino_rec;
		ino_rec = next_ino_rec(ino_rec);
	}
}

void
print_inode_list(xfs_agnumber_t agno)
{
	print_inode_list_int(agno, 0);
}

void
print_uncertain_inode_list(xfs_agnumber_t agno)
{
	print_inode_list_int(agno, 1);
}

/*
 * set parent -- use a bitmask and a packed array.  The bitmask
 * indicate which inodes have an entry in the array.  An inode that
 * is the Nth bit set in the mask is stored in the Nth location in
 * the array where N starts at 0.
 */

void
set_inode_parent(
	ino_tree_node_t		*irec,
	int			offset,
	xfs_ino_t		parent)
{
	parent_list_t		*ptbl;
	int			i;
	int			cnt;
	int			target;
	__uint64_t		bitmask;
	parent_entry_t		*tmp;

	if (full_ino_ex_data)
		ptbl = irec->ino_un.ex_data->parents;
	else
		ptbl = irec->ino_un.plist;

	if (ptbl == NULL)  {
		ptbl = (parent_list_t *)malloc(sizeof(parent_list_t));
		if (!ptbl)
			do_error(_("couldn't malloc parent list table\n"));

		if (full_ino_ex_data)
			irec->ino_un.ex_data->parents = ptbl;
		else
			irec->ino_un.plist = ptbl;

		ptbl->pmask = 1LL << offset;
		ptbl->pentries = (xfs_ino_t*)memalign(sizeof(xfs_ino_t),
							sizeof(xfs_ino_t));
		if (!ptbl->pentries)
			do_error(_("couldn't memalign pentries table\n"));
#ifdef DEBUG
		ptbl->cnt = 1;
#endif
		ptbl->pentries[0] = parent;

		return;
	}

	if (ptbl->pmask & (1LL << offset))  {
		bitmask = 1LL;
		target = 0;

		for (i = 0; i < offset; i++)  {
			if (ptbl->pmask & bitmask)
				target++;
			bitmask <<= 1;
		}
#ifdef DEBUG
		ASSERT(target < ptbl->cnt);
#endif
		ptbl->pentries[target] = parent;

		return;
	}

	bitmask = 1LL;
	cnt = target = 0;

	for (i = 0; i < XFS_INODES_PER_CHUNK; i++)  {
		if (ptbl->pmask & bitmask)  {
			cnt++;
			if (i < offset)
				target++;
		}

		bitmask <<= 1;
	}

#ifdef DEBUG
	ASSERT(cnt == ptbl->cnt);
#endif
	ASSERT(cnt >= target);

	tmp = (xfs_ino_t*)memalign(sizeof(xfs_ino_t), (cnt + 1) * sizeof(xfs_ino_t));
	if (!tmp)
		do_error(_("couldn't memalign pentries table\n"));

	memmove(tmp, ptbl->pentries, target * sizeof(parent_entry_t));

	if (cnt > target)
		memmove(tmp + target + 1, ptbl->pentries + target,
				(cnt - target) * sizeof(parent_entry_t));

	free(ptbl->pentries);

	ptbl->pentries = tmp;

#ifdef DEBUG
	ptbl->cnt++;
#endif
	ptbl->pentries[target] = parent;
	ptbl->pmask |= (1LL << offset);
}

xfs_ino_t
get_inode_parent(ino_tree_node_t *irec, int offset)
{
	__uint64_t	bitmask;
	parent_list_t	*ptbl;
	int		i;
	int		target;

	if (full_ino_ex_data)
		ptbl = irec->ino_un.ex_data->parents;
	else
		ptbl = irec->ino_un.plist;

	if (ptbl->pmask & (1LL << offset))  {
		bitmask = 1LL;
		target = 0;

		for (i = 0; i < offset; i++)  {
			if (ptbl->pmask & bitmask)
				target++;
			bitmask <<= 1;
		}
#ifdef DEBUG
		ASSERT(target < ptbl->cnt);
#endif
		return(ptbl->pentries[target]);
	}

	return(0LL);
}

static void
alloc_ex_data(ino_tree_node_t *irec)
{
	parent_list_t 	*ptbl;

	ptbl = irec->ino_un.plist;
	irec->ino_un.ex_data  = (ino_ex_data_t *)calloc(1, sizeof(ino_ex_data_t));
	if (irec->ino_un.ex_data == NULL)
		do_error(_("could not malloc inode extra data\n"));

	irec->ino_un.ex_data->parents = ptbl;
	irec->ino_un.ex_data->counted_nlinks = calloc(1, irec->nlinkops->nlink_size);

	if (irec->ino_un.ex_data->counted_nlinks == NULL)
		do_error(_("could not malloc inode extra data\n"));
}

void
add_ino_ex_data(xfs_mount_t *mp)
{
	ino_tree_node_t	*ino_rec;
	xfs_agnumber_t	i;

	for (i = 0; i < mp->m_sb.sb_agcount; i++)  {
		ino_rec = findfirst_inode_rec(i);

		while (ino_rec != NULL)  {
			alloc_ex_data(ino_rec);
			ino_rec = next_ino_rec(ino_rec);
		}
	}
	full_ino_ex_data = 1;
}

static __psunsigned_t
avl_ino_start(avlnode_t *node)
{
	return((__psunsigned_t) ((ino_tree_node_t *) node)->ino_startnum);
}

static __psunsigned_t
avl_ino_end(avlnode_t *node)
{
	return((__psunsigned_t) (
		((ino_tree_node_t *) node)->ino_startnum +
		XFS_INODES_PER_CHUNK));
}

avlops_t avl_ino_tree_ops = {
	avl_ino_start,
	avl_ino_end
};

void
incore_ino_init(xfs_mount_t *mp)
{
	int i;
	int agcount = mp->m_sb.sb_agcount;

	pthread_mutex_init(&ino_flist_lock, NULL);
	if ((inode_tree_ptrs = malloc(agcount *
					sizeof(avltree_desc_t *))) == NULL)
		do_error(_("couldn't malloc inode tree descriptor table\n"));
	if ((inode_uncertain_tree_ptrs = malloc(agcount *
					sizeof(avltree_desc_t *))) == NULL)
		do_error(
		_("couldn't malloc uncertain ino tree descriptor table\n"));

	for (i = 0; i < agcount; i++)  {
		if ((inode_tree_ptrs[i] =
				malloc(sizeof(avltree_desc_t))) == NULL)
			do_error(_("couldn't malloc inode tree descriptor\n"));
		if ((inode_uncertain_tree_ptrs[i] =
				malloc(sizeof(avltree_desc_t))) == NULL)
			do_error(
			_("couldn't malloc uncertain ino tree descriptor\n"));
	}
	for (i = 0; i < agcount; i++)  {
		avl_init_tree(inode_tree_ptrs[i], &avl_ino_tree_ops);
		avl_init_tree(inode_uncertain_tree_ptrs[i], &avl_ino_tree_ops);
	}

	ino_flist.cnt = 0;
	ino_flist.list = NULL;

	if ((last_rec = malloc(sizeof(ino_tree_node_t *) * agcount)) == NULL)
		do_error(_("couldn't malloc uncertain inode cache area\n"));

	memset(last_rec, 0, sizeof(ino_tree_node_t *) * agcount);

	full_ino_ex_data = 0;
}

#ifdef XR_INO_REF_DEBUG
void
add_inode_refchecked(xfs_ino_t ino, ino_tree_node_t *ino_rec, int ino_offset)
{
	XFS_INOPROC_SET_PROC((ino_rec), (ino_offset));

	ASSERT(is_inode_refchecked(ino, ino_rec, ino_offset));
}

int
is_inode_refchecked(xfs_ino_t ino, ino_tree_node_t *ino_rec, int ino_offset)
{
	return(XFS_INOPROC_IS_PROC(ino_rec, ino_offset) == 0LL ? 0 : 1);
}
#endif /* XR_INO_REF_DEBUG */
