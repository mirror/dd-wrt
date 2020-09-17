// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "avl.h"
#include "globals.h"
#include "incore.h"
#include "agheader.h"
#include "protos.h"
#include "threads.h"
#include "err_protos.h"

/*
 * array of inode tree ptrs, one per ag
 */
avltree_desc_t	**inode_tree_ptrs;

/*
 * ditto for uncertain inodes
 */
static avltree_desc_t	**inode_uncertain_tree_ptrs;

/* memory optimised nlink counting for all inodes */

static void *
alloc_nlink_array(uint8_t nlink_size)
{
	void *ptr;

	ptr = calloc(XFS_INODES_PER_CHUNK, nlink_size);
	if (!ptr)
		do_error(_("could not allocate nlink array\n"));
	return ptr;
}

static void
nlink_grow_8_to_16(ino_tree_node_t *irec)
{
	uint16_t	*new_nlinks;
	int		i;

	irec->nlink_size = sizeof(uint16_t);

	new_nlinks = alloc_nlink_array(irec->nlink_size);
	for (i = 0; i < XFS_INODES_PER_CHUNK; i++)
		new_nlinks[i] = irec->disk_nlinks.un8[i];
	free(irec->disk_nlinks.un8);
	irec->disk_nlinks.un16 = new_nlinks;

	if (full_ino_ex_data) {
		new_nlinks = alloc_nlink_array(irec->nlink_size);
		for (i = 0; i < XFS_INODES_PER_CHUNK; i++) {
			new_nlinks[i] =
				irec->ino_un.ex_data->counted_nlinks.un8[i];
		}
		free(irec->ino_un.ex_data->counted_nlinks.un8);
		irec->ino_un.ex_data->counted_nlinks.un16 = new_nlinks;
	}
}

static void
nlink_grow_16_to_32(ino_tree_node_t *irec)
{
	uint32_t	*new_nlinks;
	int		i;

	irec->nlink_size = sizeof(uint32_t);

	new_nlinks = alloc_nlink_array(irec->nlink_size);
	for (i = 0; i < XFS_INODES_PER_CHUNK; i++)
		new_nlinks[i] = irec->disk_nlinks.un16[i];
	free(irec->disk_nlinks.un16);
	irec->disk_nlinks.un32 = new_nlinks;

	if (full_ino_ex_data) {
		new_nlinks = alloc_nlink_array(irec->nlink_size);

		for (i = 0; i < XFS_INODES_PER_CHUNK; i++) {
			new_nlinks[i] =
				irec->ino_un.ex_data->counted_nlinks.un16[i];
		}
		free(irec->ino_un.ex_data->counted_nlinks.un16);
		irec->ino_un.ex_data->counted_nlinks.un32 = new_nlinks;
	}
}

void add_inode_ref(struct ino_tree_node *irec, int ino_offset)
{
	ASSERT(irec->ino_un.ex_data != NULL);

	switch (irec->nlink_size) {
	case sizeof(uint8_t):
		if (irec->ino_un.ex_data->counted_nlinks.un8[ino_offset] < 0xff) {
			irec->ino_un.ex_data->counted_nlinks.un8[ino_offset]++;
			break;
		}
		nlink_grow_8_to_16(irec);
		/*FALLTHRU*/
	case sizeof(uint16_t):
		if (irec->ino_un.ex_data->counted_nlinks.un16[ino_offset] < 0xffff) {
			irec->ino_un.ex_data->counted_nlinks.un16[ino_offset]++;
			break;
		}
		nlink_grow_16_to_32(irec);
		/*FALLTHRU*/
	case sizeof(uint32_t):
		irec->ino_un.ex_data->counted_nlinks.un32[ino_offset]++;
		break;
	default:
		ASSERT(0);
	}
}

void drop_inode_ref(struct ino_tree_node *irec, int ino_offset)
{
	uint32_t	refs = 0;

	ASSERT(irec->ino_un.ex_data != NULL);

	switch (irec->nlink_size) {
	case sizeof(uint8_t):
		ASSERT(irec->ino_un.ex_data->counted_nlinks.un8[ino_offset] > 0);
		refs = --irec->ino_un.ex_data->counted_nlinks.un8[ino_offset];
		break;
	case sizeof(uint16_t):
		ASSERT(irec->ino_un.ex_data->counted_nlinks.un16[ino_offset] > 0);
		refs = --irec->ino_un.ex_data->counted_nlinks.un16[ino_offset];
		break;
	case sizeof(uint32_t):
		ASSERT(irec->ino_un.ex_data->counted_nlinks.un32[ino_offset] > 0);
		refs = --irec->ino_un.ex_data->counted_nlinks.un32[ino_offset];
		break;
	default:
		ASSERT(0);
	}

	if (refs == 0)
		irec->ino_un.ex_data->ino_reached &= ~IREC_MASK(ino_offset);
}

uint32_t num_inode_references(struct ino_tree_node *irec, int ino_offset)
{
	ASSERT(irec->ino_un.ex_data != NULL);

	switch (irec->nlink_size) {
	case sizeof(uint8_t):
		return irec->ino_un.ex_data->counted_nlinks.un8[ino_offset];
	case sizeof(uint16_t):
		return irec->ino_un.ex_data->counted_nlinks.un16[ino_offset];
	case sizeof(uint32_t):
		return irec->ino_un.ex_data->counted_nlinks.un32[ino_offset];
	default:
		ASSERT(0);
	}
	return 0;
}

void set_inode_disk_nlinks(struct ino_tree_node *irec, int ino_offset,
		uint32_t nlinks)
{
	switch (irec->nlink_size) {
	case sizeof(uint8_t):
		if (nlinks < 0xff) {
			irec->disk_nlinks.un8[ino_offset] = nlinks;
			break;
		}
		nlink_grow_8_to_16(irec);
		/*FALLTHRU*/
	case sizeof(uint16_t):
		if (nlinks < 0xffff) {
			irec->disk_nlinks.un16[ino_offset] = nlinks;
			break;
		}
		nlink_grow_16_to_32(irec);
		/*FALLTHRU*/
	case sizeof(uint32_t):
		irec->disk_nlinks.un32[ino_offset] = nlinks;
		break;
	default:
		ASSERT(0);
	}
}

uint32_t get_inode_disk_nlinks(struct ino_tree_node *irec, int ino_offset)
{
	switch (irec->nlink_size) {
	case sizeof(uint8_t):
		return irec->disk_nlinks.un8[ino_offset];
	case sizeof(uint16_t):
		return irec->disk_nlinks.un16[ino_offset];
	case sizeof(uint32_t):
		return irec->disk_nlinks.un32[ino_offset];
	default:
		ASSERT(0);
	}
	return 0;
}

static uint8_t *
alloc_ftypes_array(
	struct xfs_mount *mp)
{
	uint8_t		*ptr;

	if (!xfs_sb_version_hasftype(&mp->m_sb))
		return NULL;

	ptr = calloc(XFS_INODES_PER_CHUNK, sizeof(*ptr));
	if (!ptr)
		do_error(_("could not allocate ftypes array\n"));
	return ptr;
}

/*
 * Next is the uncertain inode list -- a sorted (in ascending order)
 * list of inode records sorted on the starting inode number.  There
 * is one list per ag.
 */

/*
 * Common code for creating inode records for use by trees and lists.
 * called only from add_inodes and add_inodes_uncertain
 *
 * IMPORTANT:  all inodes (inode records) start off as free and
 *		unconfirmed.
 */
static struct ino_tree_node *
alloc_ino_node(
	struct xfs_mount	*mp,
	xfs_agino_t		starting_ino)
{
	struct ino_tree_node 	*irec;

	irec = malloc(sizeof(*irec));
	if (!irec)
		do_error(_("inode map malloc failed\n"));

	irec->avl_node.avl_nextino = NULL;
	irec->avl_node.avl_forw = NULL;
	irec->avl_node.avl_back = NULL;

	irec->ino_startnum = starting_ino;
	irec->ino_confirmed = 0;
	irec->ino_isa_dir = 0;
	irec->ino_was_rl = 0;
	irec->ino_is_rl = 0;
	irec->ir_free = (xfs_inofree_t) - 1;
	irec->ir_sparse = 0;
	irec->ino_un.ex_data = NULL;
	irec->nlink_size = sizeof(uint8_t);
	irec->disk_nlinks.un8 = alloc_nlink_array(irec->nlink_size);
	irec->ftypes = alloc_ftypes_array(mp);
	return irec;
}

static void
free_nlink_array(union ino_nlink nlinks, uint8_t nlink_size)
{
	switch (nlink_size) {
	case sizeof(uint8_t):
		free(nlinks.un8);
		break;
	case sizeof(uint16_t):
		free(nlinks.un16);
		break;
	case sizeof(uint32_t):
		free(nlinks.un32);
		break;
	default:
		ASSERT(0);
	}
}

static void
free_ino_tree_node(
	struct ino_tree_node	*irec)
{
	irec->avl_node.avl_nextino = NULL;
	irec->avl_node.avl_forw = NULL;
	irec->avl_node.avl_back = NULL;

	free_nlink_array(irec->disk_nlinks, irec->nlink_size);
	if (irec->ino_un.ex_data != NULL)  {
		if (full_ino_ex_data) {
			free(irec->ino_un.ex_data->parents);
			free_nlink_array(irec->ino_un.ex_data->counted_nlinks,
					 irec->nlink_size);
		}
		free(irec->ino_un.ex_data);

	}

	free(irec->ftypes);
	free(irec);
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
add_aginode_uncertain(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	xfs_agino_t		ino,
	int			free)
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
	ino_rec = (ino_tree_node_t *)
		avl_findrange(inode_uncertain_tree_ptrs[agno], s_ino);
	if (!ino_rec) {
		ino_rec = alloc_ino_node(mp, s_ino);

		if (!avl_insert(inode_uncertain_tree_ptrs[agno],
				&ino_rec->avl_node))
			do_error(
	_("add_aginode_uncertain - duplicate inode range\n"));
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
	add_aginode_uncertain(mp, XFS_INO_TO_AGNO(mp, ino),
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
 * Next comes the inode trees.  One per AG,  AVL trees of inode records, each
 * inode record tracking 64 inodes
 */

/*
 * Set up an inode tree record for a group of inodes that will include the
 * requested inode.
 *
 * This does NOT do error-check for duplicate records.  The caller is
 * responsible for checking that. Ino must be the start of an
 * XFS_INODES_PER_CHUNK (64) inode chunk
 *
 * Each inode resides in a 64-inode chunk which can be part one or more chunks
 * (max(64, inodes-per-block).  The fs allocates in chunks (as opposed to 1
 * chunk) when a block can hold more than one chunk (inodes per block > 64).
 * Allocating in one chunk pieces causes us problems when it takes more than
 * one fs block to contain an inode chunk because the chunks can start on
 * *any* block boundary. So we assume that the caller has a clue because at
 * this level, we don't.
 */
static struct ino_tree_node *
add_inode(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	xfs_agino_t		agino)
{
	struct ino_tree_node	*irec;

	irec = alloc_ino_node(mp, agino);
	if (!avl_insert(inode_tree_ptrs[agno],	&irec->avl_node))
		do_warn(_("add_inode - duplicate inode range\n"));
	return irec;
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

static void
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
	uint64_t		bitmask;
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

		ptbl->pmask = 1ULL << offset;
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

	if (ptbl->pmask & (1ULL << offset))  {
		bitmask = 1ULL;
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

	bitmask = 1ULL;
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
	ptbl->pmask |= (1ULL << offset);
}

xfs_ino_t
get_inode_parent(ino_tree_node_t *irec, int offset)
{
	uint64_t	bitmask;
	parent_list_t	*ptbl;
	int		i;
	int		target;

	if (full_ino_ex_data)
		ptbl = irec->ino_un.ex_data->parents;
	else
		ptbl = irec->ino_un.plist;

	if (ptbl->pmask & (1ULL << offset))  {
		bitmask = 1ULL;
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

void
alloc_ex_data(ino_tree_node_t *irec)
{
	parent_list_t 	*ptbl;

	ptbl = irec->ino_un.plist;
	irec->ino_un.ex_data  = (ino_ex_data_t *)calloc(1, sizeof(ino_ex_data_t));
	if (irec->ino_un.ex_data == NULL)
		do_error(_("could not malloc inode extra data\n"));

	irec->ino_un.ex_data->parents = ptbl;

	switch (irec->nlink_size) {
	case sizeof(uint8_t):
		irec->ino_un.ex_data->counted_nlinks.un8 =
			alloc_nlink_array(irec->nlink_size);
		break;
	case sizeof(uint16_t):
		irec->ino_un.ex_data->counted_nlinks.un16 =
			alloc_nlink_array(irec->nlink_size);
		break;
	case sizeof(uint32_t):
		irec->ino_un.ex_data->counted_nlinks.un32 =
			alloc_nlink_array(irec->nlink_size);
		break;
	default:
		ASSERT(0);
	}
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

static uintptr_t
avl_ino_start(avlnode_t *node)
{
	return((uintptr_t) ((ino_tree_node_t *) node)->ino_startnum);
}

static uintptr_t
avl_ino_end(avlnode_t *node)
{
	return((uintptr_t) (
		((ino_tree_node_t *) node)->ino_startnum +
		XFS_INODES_PER_CHUNK));
}

static avlops_t avl_ino_tree_ops = {
	avl_ino_start,
	avl_ino_end
};

void
incore_ino_init(xfs_mount_t *mp)
{
	int i;
	int agcount = mp->m_sb.sb_agcount;

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

	if ((last_rec = malloc(sizeof(ino_tree_node_t *) * agcount)) == NULL)
		do_error(_("couldn't malloc uncertain inode cache area\n"));

	memset(last_rec, 0, sizeof(ino_tree_node_t *) * agcount);

	full_ino_ex_data = 0;
}
