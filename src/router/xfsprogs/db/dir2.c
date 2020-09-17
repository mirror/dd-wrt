// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "bit.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "dir2.h"
#include "init.h"
#include "output.h"

static int	dir2_block_hdr_count(void *obj, int startoff);
static int	dir2_block_leaf_count(void *obj, int startoff);
static int	dir2_block_leaf_offset(void *obj, int startoff, int idx);
static int	dir2_block_tail_count(void *obj, int startoff);
static int	dir2_block_tail_offset(void *obj, int startoff, int idx);
static int	dir2_block_u_count(void *obj, int startoff);
static int	dir2_block_u_offset(void *obj, int startoff, int idx);
static int	dir2_data_union_freetag_count(void *obj, int startoff);
static int	dir2_data_union_inumber_count(void *obj, int startoff);
static int	dir2_data_union_length_count(void *obj, int startoff);
static int	dir2_data_union_name_count(void *obj, int startoff);
static int	dir2_data_union_namelen_count(void *obj, int startoff);
static int	dir2_data_union_tag_count(void *obj, int startoff);
static int	dir2_data_union_tag_offset(void *obj, int startoff, int idx);
static int	dir2_data_hdr_count(void *obj, int startoff);
static int	dir2_data_u_count(void *obj, int startoff);
static int	dir2_data_u_offset(void *obj, int startoff, int idx);
static int	dir2_free_bests_count(void *obj, int startoff);
static int	dir2_free_hdr_count(void *obj, int startoff);
static int	dir2_leaf_bests_count(void *obj, int startoff);
static int	dir2_leaf_bests_offset(void *obj, int startoff, int idx);
static int	dir2_leaf_ents_count(void *obj, int startoff);
static int	dir2_leaf_hdr_count(void *obj, int startoff);
static int	dir2_leaf_tail_count(void *obj, int startoff);
static int	dir2_leaf_tail_offset(void *obj, int startoff, int idx);
static int	dir2_node_btree_count(void *obj, int startoff);
static int	dir2_node_hdr_count(void *obj, int startoff);

const field_t	dir2_hfld[] = {
	{ "", FLDT_DIR2, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	BOFF(f)	bitize(offsetof(struct xfs_dir2_data_hdr, f))
#define	DOFF(f)	bitize(offsetof(struct xfs_dir2_data_hdr, f))
#define	FOFF(f)	bitize(offsetof(struct xfs_dir2_free, f))
#define	LOFF(f)	bitize(offsetof(struct xfs_dir2_leaf, f))
#define	NOFF(f)	bitize(offsetof(struct xfs_da_intnode, f))
const field_t	dir2_flds[] = {
	{ "bhdr", FLDT_DIR2_DATA_HDR, OI(BOFF(magic)), dir2_block_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "bu", FLDT_DIR2_DATA_UNION, dir2_block_u_offset, dir2_block_u_count,
	  FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "bleaf", FLDT_DIR2_LEAF_ENTRY, dir2_block_leaf_offset,
	  dir2_block_leaf_count, FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "btail", FLDT_DIR2_BLOCK_TAIL, dir2_block_tail_offset,
	  dir2_block_tail_count, FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "dhdr", FLDT_DIR2_DATA_HDR, OI(DOFF(magic)), dir2_data_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "du", FLDT_DIR2_DATA_UNION, dir2_data_u_offset, dir2_data_u_count,
	  FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "lhdr", FLDT_DIR2_LEAF_HDR, OI(LOFF(hdr)), dir2_leaf_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "lbests", FLDT_DIR2_DATA_OFF, dir2_leaf_bests_offset,
	  dir2_leaf_bests_count, FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "lents", FLDT_DIR2_LEAF_ENTRY, OI(LOFF(__ents)), dir2_leaf_ents_count,
	  FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "ltail", FLDT_DIR2_LEAF_TAIL, dir2_leaf_tail_offset,
	  dir2_leaf_tail_count, FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "nhdr", FLDT_DA_NODE_HDR, OI(NOFF(hdr)), dir2_node_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "nbtree", FLDT_DA_NODE_ENTRY, OI(NOFF(__btree)), dir2_node_btree_count,
	  FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "fhdr", FLDT_DIR2_FREE_HDR, OI(FOFF(hdr)), dir2_free_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "fbests", FLDT_DIR2_DATA_OFFNZ, OI(FOFF(bests)),
	  dir2_free_bests_count, FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ NULL }
};

#define	BTOFF(f)	bitize(offsetof(xfs_dir2_block_tail_t, f))
const field_t	dir2_block_tail_flds[] = {
	{ "count", FLDT_UINT32D, OI(BTOFF(count)), C1, 0, TYP_NONE },
	{ "stale", FLDT_UINT32D, OI(BTOFF(stale)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	DFOFF(f)	bitize(offsetof(xfs_dir2_data_free_t, f))
const field_t	dir2_data_free_flds[] = {
	{ "offset", FLDT_DIR2_DATA_OFF, OI(DFOFF(offset)), C1, 0, TYP_NONE },
	{ "length", FLDT_DIR2_DATA_OFF, OI(DFOFF(length)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	DHOFF(f)	bitize(offsetof(xfs_dir2_data_hdr_t, f))
const field_t	dir2_data_hdr_flds[] = {
	{ "magic", FLDT_UINT32X, OI(DHOFF(magic)), C1, 0, TYP_NONE },
	{ "bestfree", FLDT_DIR2_DATA_FREE, OI(DHOFF(bestfree)),
	  CI(XFS_DIR2_DATA_FD_COUNT), FLD_ARRAY, TYP_NONE },
	{ NULL }
};

#define	DEOFF(f)	bitize(offsetof(xfs_dir2_data_entry_t, f))
#define	DUOFF(f)	bitize(offsetof(xfs_dir2_data_unused_t, f))
const field_t	dir2_data_union_flds[] = {
	{ "freetag", FLDT_UINT16X, OI(DUOFF(freetag)),
	  dir2_data_union_freetag_count, FLD_COUNT, TYP_NONE },
	{ "inumber", FLDT_INO, OI(DEOFF(inumber)),
	  dir2_data_union_inumber_count, FLD_COUNT, TYP_INODE },
	{ "length", FLDT_DIR2_DATA_OFF, OI(DUOFF(length)),
	  dir2_data_union_length_count, FLD_COUNT, TYP_NONE },
	{ "namelen", FLDT_UINT8D, OI(DEOFF(namelen)),
	  dir2_data_union_namelen_count, FLD_COUNT, TYP_NONE },
	{ "name", FLDT_CHARNS, OI(DEOFF(name)), dir2_data_union_name_count,
	  FLD_COUNT, TYP_NONE },
	{ "tag", FLDT_DIR2_DATA_OFF, dir2_data_union_tag_offset,
	  dir2_data_union_tag_count, FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ NULL }
};

#define	LEOFF(f)	bitize(offsetof(xfs_dir2_leaf_entry_t, f))
const field_t	dir2_leaf_entry_flds[] = {
	{ "hashval", FLDT_UINT32X, OI(LEOFF(hashval)), C1, 0, TYP_NONE },
	{ "address", FLDT_UINT32X, OI(LEOFF(address)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	LHOFF(f)	bitize(offsetof(xfs_dir2_leaf_hdr_t, f))
const field_t	dir2_leaf_hdr_flds[] = {
	{ "info", FLDT_DA_BLKINFO, OI(LHOFF(info)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT16D, OI(LHOFF(count)), C1, 0, TYP_NONE },
	{ "stale", FLDT_UINT16D, OI(LHOFF(stale)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	LTOFF(f)	bitize(offsetof(xfs_dir2_leaf_tail_t, f))
const field_t	dir2_leaf_tail_flds[] = {
	{ "bestcount", FLDT_UINT32D, OI(LTOFF(bestcount)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	FHOFF(f)	bitize(offsetof(xfs_dir2_free_hdr_t, f))
const field_t	dir2_free_hdr_flds[] = {
	{ "magic", FLDT_UINT32X, OI(FHOFF(magic)), C1, 0, TYP_NONE },
	{ "firstdb", FLDT_INT32D, OI(FHOFF(firstdb)), C1, 0, TYP_NONE },
	{ "nvalid", FLDT_INT32D, OI(FHOFF(nvalid)), C1, 0, TYP_NONE },
	{ "nused", FLDT_INT32D, OI(FHOFF(nused)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	DBOFF(f)	bitize(offsetof(xfs_da_blkinfo_t, f))
const field_t	da_blkinfo_flds[] = {
	{ "forw", FLDT_DIRBLOCK, OI(DBOFF(forw)), C1, 0, TYP_INODATA },
	{ "back", FLDT_DIRBLOCK, OI(DBOFF(back)), C1, 0, TYP_INODATA },
	{ "magic", FLDT_UINT16X, OI(DBOFF(magic)), C1, 0, TYP_NONE },
	{ "pad", FLDT_UINT16X, OI(DBOFF(pad)), C1, FLD_SKIPALL, TYP_NONE },
	{ NULL }
};

#define	EOFF(f)	bitize(offsetof(xfs_da_node_entry_t, f))
const field_t	da_node_entry_flds[] = {
	{ "hashval", FLDT_UINT32X, OI(EOFF(hashval)), C1, 0, TYP_NONE },
	{ "before", FLDT_DIRBLOCK, OI(EOFF(before)), C1, 0, TYP_INODATA },
	{ NULL }
};

#define	HOFF(f)	bitize(offsetof(xfs_da_node_hdr_t, f))
const field_t	da_node_hdr_flds[] = {
	{ "info", FLDT_DA_BLKINFO, OI(HOFF(info)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT16D, OI(HOFF(__count)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(HOFF(__level)), C1, 0, TYP_NONE },
	{ NULL }
};

/*
 * Worker functions shared between either dir2/dir3 or block/data formats
 */
static int
__dir2_block_tail_offset(
	struct xfs_dir2_data_hdr *block,
	int			startoff,
	int			idx)
{
	struct xfs_dir2_block_tail *btp;

	ASSERT(startoff == 0);
	ASSERT(idx == 0);
	btp = xfs_dir2_block_tail_p(mp->m_dir_geo, block);
	return bitize((int)((char *)btp - (char *)block));
}

static int
__dir2_data_entries_count(
	char	*ptr,
	char	*endptr)
{
	int	i;

	for (i = 0; ptr < endptr; i++) {
		struct xfs_dir2_data_entry *dep;
		struct xfs_dir2_data_unused *dup;

		dup = (xfs_dir2_data_unused_t *)ptr;
		if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG)
			ptr += be16_to_cpu(dup->length);
		else {
			dep = (xfs_dir2_data_entry_t *)ptr;
			ptr += libxfs_dir2_data_entsize(mp, dep->namelen);
		}
	}
	return i;
}

static char *
__dir2_data_entry_offset(
	char	*ptr,
	char	*endptr,
	int	idx)
{
	int	i;

	for (i = 0; i < idx; i++) {
		struct xfs_dir2_data_entry *dep;
		struct xfs_dir2_data_unused *dup;

		ASSERT(ptr < endptr);
		dup = (xfs_dir2_data_unused_t *)ptr;
		if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG)
			ptr += be16_to_cpu(dup->length);
		else {
			dep = (xfs_dir2_data_entry_t *)ptr;
			ptr += libxfs_dir2_data_entsize(mp, dep->namelen);
		}
	}
	return ptr;
}

/*
 * Block format functions
 */
static int
dir2_block_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_data_hdr *block = obj;

	ASSERT(startoff == 0);
	return be32_to_cpu(block->magic) == XFS_DIR2_BLOCK_MAGIC;
}

static int
dir3_block_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_data_hdr *block = obj;

	ASSERT(startoff == 0);
	return be32_to_cpu(block->magic) == XFS_DIR3_BLOCK_MAGIC;
}

static int
dir2_block_leaf_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_data_hdr *block = obj;
	struct xfs_dir2_block_tail *btp;

	ASSERT(startoff == 0);
	if (be32_to_cpu(block->magic) != XFS_DIR2_BLOCK_MAGIC &&
	    be32_to_cpu(block->magic) != XFS_DIR3_BLOCK_MAGIC)
		return 0;
	btp = xfs_dir2_block_tail_p(mp->m_dir_geo, block);
	return be32_to_cpu(btp->count);
}

static int
dir2_block_leaf_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_dir2_data_hdr *block = obj;
	struct xfs_dir2_block_tail *btp;
	struct xfs_dir2_leaf_entry *lep;

	ASSERT(startoff == 0);
	ASSERT(be32_to_cpu(block->magic) == XFS_DIR2_BLOCK_MAGIC ||
	       be32_to_cpu(block->magic) == XFS_DIR3_BLOCK_MAGIC);
	btp = xfs_dir2_block_tail_p(mp->m_dir_geo, block);
	lep = xfs_dir2_block_leaf_p(btp) + idx;
	return bitize((int)((char *)lep - (char *)block));
}

static int
dir2_block_tail_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_data_hdr *block = obj;

	ASSERT(startoff == 0);
	return be32_to_cpu(block->magic) == XFS_DIR2_BLOCK_MAGIC;
}

static int
dir3_block_tail_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_data_hdr *block = obj;

	ASSERT(startoff == 0);
	return be32_to_cpu(block->magic) == XFS_DIR3_BLOCK_MAGIC;
}

static int
dir2_block_tail_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_dir2_data_hdr *block = obj;

	ASSERT(be32_to_cpu(block->magic) == XFS_DIR2_BLOCK_MAGIC ||
	       be32_to_cpu(block->magic) == XFS_DIR3_BLOCK_MAGIC);
	return __dir2_block_tail_offset(block, startoff, idx);
}

static int
dir2_block_u_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_data_hdr *block = obj;
	struct xfs_dir2_block_tail *btp;

	ASSERT(startoff == 0);
	if (be32_to_cpu(block->magic) != XFS_DIR2_BLOCK_MAGIC &&
	    be32_to_cpu(block->magic) != XFS_DIR3_BLOCK_MAGIC)
		return 0;

	btp = xfs_dir2_block_tail_p(mp->m_dir_geo, block);
	return __dir2_data_entries_count(
			(char *)obj + mp->m_dir_geo->data_entry_offset,
			(char *)xfs_dir2_block_leaf_p(btp));
}

static int
dir2_block_u_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_dir2_data_hdr *block = obj;
	struct xfs_dir2_block_tail *btp;
	char			*ptr;

	ASSERT(startoff == 0);
	ASSERT(be32_to_cpu(block->magic) == XFS_DIR2_BLOCK_MAGIC ||
	       be32_to_cpu(block->magic) == XFS_DIR3_BLOCK_MAGIC);
	btp = xfs_dir2_block_tail_p(mp->m_dir_geo, block);
	ptr = __dir2_data_entry_offset(
			(char *)obj + mp->m_dir_geo->data_entry_offset,
			(char *)xfs_dir2_block_leaf_p(btp), idx);
	return bitize((int)(ptr - (char *)block));
}

/*
 * Data block format functions
 */
static int
dir2_data_union_freetag_count(
	void			*obj,
	int			startoff)
{
	xfs_dir2_data_unused_t	*dup;
	char			*end;

	ASSERT(bitoffs(startoff) == 0);
	dup = (xfs_dir2_data_unused_t *)((char *)obj + byteize(startoff));
	end = (char *)&dup->freetag + sizeof(dup->freetag);
	return end <= (char *)obj + mp->m_dir_geo->blksize &&
	       be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG;
}

static int
dir2_data_union_inumber_count(
	void			*obj,
	int			startoff)
{
	xfs_dir2_data_entry_t	*dep;
	xfs_dir2_data_unused_t	*dup;
	char			*end;

	ASSERT(bitoffs(startoff) == 0);
	dup = (xfs_dir2_data_unused_t *)((char *)obj + byteize(startoff));
	dep = (xfs_dir2_data_entry_t *)dup;
	end = (char *)&dep->inumber + sizeof(dep->inumber);
	return end <= (char *)obj + mp->m_dir_geo->blksize &&
	       be16_to_cpu(dup->freetag) != XFS_DIR2_DATA_FREE_TAG;
}

static int
dir2_data_union_length_count(
	void			*obj,
	int			startoff)
{
	xfs_dir2_data_unused_t	*dup;
	char			*end;

	ASSERT(bitoffs(startoff) == 0);
	dup = (xfs_dir2_data_unused_t *)((char *)obj + byteize(startoff));
	end = (char *)&dup->length + sizeof(dup->length);
	return end <= (char *)obj + mp->m_dir_geo->blksize &&
	       be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG;
}

static int
dir2_data_union_name_count(
	void			*obj,
	int			startoff)
{
	xfs_dir2_data_entry_t	*dep;
	xfs_dir2_data_unused_t	*dup;
	char			*end;

	ASSERT(bitoffs(startoff) == 0);
	dup = (xfs_dir2_data_unused_t *)((char *)obj + byteize(startoff));
	dep = (xfs_dir2_data_entry_t *)dup;
	end = (char *)&dep->namelen + sizeof(dep->namelen);
	if (end >= (char *)obj + mp->m_dir_geo->blksize ||
	    be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG)
		return 0;
	end = (char *)&dep->name[0] + dep->namelen;
	return end <= (char *)obj + mp->m_dir_geo->blksize ? dep->namelen : 0;
}

static int
dir2_data_union_namelen_count(
	void			*obj,
	int			startoff)
{
	xfs_dir2_data_entry_t	*dep;
	xfs_dir2_data_unused_t	*dup;
	char			*end;

	ASSERT(bitoffs(startoff) == 0);
	dup = (xfs_dir2_data_unused_t *)((char *)obj + byteize(startoff));
	dep = (xfs_dir2_data_entry_t *)dup;
	end = (char *)&dep->namelen + sizeof(dep->namelen);
	return end <= (char *)obj + mp->m_dir_geo->blksize &&
	       be16_to_cpu(dup->freetag) != XFS_DIR2_DATA_FREE_TAG;
}

static int
dir2_data_union_tag_count(
	void			*obj,
	int			startoff)
{
	xfs_dir2_data_entry_t	*dep;
	xfs_dir2_data_unused_t	*dup;
	char			*end;
	__be16			*tagp;

	ASSERT(bitoffs(startoff) == 0);
	dup = (xfs_dir2_data_unused_t *)((char *)obj + byteize(startoff));
	dep = (xfs_dir2_data_entry_t *)dup;
	end = (char *)&dup->freetag + sizeof(dup->freetag);
	if (end > (char *)obj + mp->m_dir_geo->blksize)
		return 0;
	if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG) {
		end = (char *)&dup->length + sizeof(dup->length);
		if (end > (char *)obj + mp->m_dir_geo->blksize)
			return 0;
		tagp = xfs_dir2_data_unused_tag_p(dup);
	} else {
		end = (char *)&dep->namelen + sizeof(dep->namelen);
		if (end > (char *)obj + mp->m_dir_geo->blksize)
			return 0;
		tagp = libxfs_dir2_data_entry_tag_p(mp, dep);
	}
	end = (char *)tagp + sizeof(*tagp);
	return end <= (char *)obj + mp->m_dir_geo->blksize;
}

static int
dir2_data_union_tag_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_data_entry_t	*dep;
	xfs_dir2_data_unused_t	*dup;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	dup = (xfs_dir2_data_unused_t *)((char *)obj + byteize(startoff));
	if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG)
		return bitize((int)((char *)xfs_dir2_data_unused_tag_p(dup) -
				    (char *)dup));
	dep = (xfs_dir2_data_entry_t *)dup;
	return bitize((int)((char *)libxfs_dir2_data_entry_tag_p(mp, dep) -
			    (char *)dep));
}

static int
dir2_data_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_data_hdr *data = obj;

	ASSERT(startoff == 0);
	return be32_to_cpu(data->magic) == XFS_DIR2_DATA_MAGIC;
}

static int
dir3_data_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_data_hdr *data = obj;

	ASSERT(startoff == 0);
	return be32_to_cpu(data->magic) == XFS_DIR3_DATA_MAGIC;
}

static int
dir2_data_u_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_data_hdr *data = obj;

	ASSERT(startoff == 0);
	if (be32_to_cpu(data->magic) != XFS_DIR2_DATA_MAGIC &&
	    be32_to_cpu(data->magic) != XFS_DIR3_DATA_MAGIC)
		return 0;

	return __dir2_data_entries_count(
				(char *)data + mp->m_dir_geo->data_entry_offset,
				(char *)data + mp->m_dir_geo->blksize);
}

static int
dir2_data_u_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_dir2_data_hdr *data = obj;
	char			*ptr;

	ASSERT(startoff == 0);
	ASSERT(be32_to_cpu(data->magic) == XFS_DIR2_DATA_MAGIC ||
	       be32_to_cpu(data->magic) == XFS_DIR3_DATA_MAGIC);
	ptr = __dir2_data_entry_offset(
				(char *)data + mp->m_dir_geo->data_entry_offset,
				(char *)data + mp->m_dir_geo->blksize, idx);
	return bitize((int)(ptr - (char *)data));
}

int
dir2_data_union_size(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_data_entry_t	*dep;
	xfs_dir2_data_unused_t	*dup;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	dup = (xfs_dir2_data_unused_t *)((char *)obj + byteize(startoff));
	if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG)
		return bitize(be16_to_cpu(dup->length));
	else {
		dep = (xfs_dir2_data_entry_t *)dup;
		return bitize(libxfs_dir2_data_entsize(mp, dep->namelen));
	}
}

static int
dir3_data_union_ftype_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_data_entry_t	*dep;
	xfs_dir2_data_unused_t	*dup;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	dup = (xfs_dir2_data_unused_t *)((char *)obj + byteize(startoff));
	if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG)
		return bitize((int)((char *)xfs_dir2_data_unused_tag_p(dup) -
				    (char *)dup));
	dep = (xfs_dir2_data_entry_t *)dup;
	return bitize((int)((char *)&dep->name[dep->namelen] - (char *)dep));
}

/*
 * Free block functions
 */
static int
dir2_free_bests_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_free	*free = obj;

	ASSERT(startoff == 0);
	if (be32_to_cpu(free->hdr.magic) != XFS_DIR2_FREE_MAGIC)
		return 0;
	return be32_to_cpu(free->hdr.nvalid);
}

static int
dir3_free_bests_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir3_free	*free = obj;

	ASSERT(startoff == 0);
	if (be32_to_cpu(free->hdr.hdr.magic) != XFS_DIR3_FREE_MAGIC)
		return 0;
	return be32_to_cpu(free->hdr.nvalid);
}

static int
dir2_free_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_free	*free = obj;

	ASSERT(startoff == 0);
	return be32_to_cpu(free->hdr.magic) == XFS_DIR2_FREE_MAGIC;
}

static int
dir3_free_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir3_free	*free = obj;

	ASSERT(startoff == 0);
	return be32_to_cpu(free->hdr.hdr.magic) == XFS_DIR3_FREE_MAGIC;
}

/*
 * Leaf block functions
 */
static int
dir2_leaf_bests_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_leaf	*leaf = obj;
	struct xfs_dir2_leaf_tail *ltp;

	ASSERT(startoff == 0);
	if (be16_to_cpu(leaf->hdr.info.magic) != XFS_DIR2_LEAF1_MAGIC &&
	    be16_to_cpu(leaf->hdr.info.magic) != XFS_DIR3_LEAF1_MAGIC)
		return 0;
	ltp = xfs_dir2_leaf_tail_p(mp->m_dir_geo, leaf);
	return be32_to_cpu(ltp->bestcount);
}

static int
dir2_leaf_bests_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_dir2_leaf	*leaf = obj;
	struct xfs_dir2_leaf_tail *ltp;
	__be16			*lbp;

	ASSERT(startoff == 0);
	ASSERT(be16_to_cpu(leaf->hdr.info.magic) == XFS_DIR2_LEAF1_MAGIC ||
	       be16_to_cpu(leaf->hdr.info.magic) == XFS_DIR3_LEAF1_MAGIC);
	ltp = xfs_dir2_leaf_tail_p(mp->m_dir_geo, leaf);
	lbp = xfs_dir2_leaf_bests_p(ltp) + idx;
	return bitize((int)((char *)lbp - (char *)leaf));
}

static int
dir2_leaf_ents_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_leaf	*leaf = obj;

	ASSERT(startoff == 0);
	if (be16_to_cpu(leaf->hdr.info.magic) != XFS_DIR2_LEAF1_MAGIC &&
	    be16_to_cpu(leaf->hdr.info.magic) != XFS_DIR2_LEAFN_MAGIC)
		return 0;
	return be16_to_cpu(leaf->hdr.count);
}

static int
dir3_leaf_ents_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir3_leaf	*leaf = obj;

	ASSERT(startoff == 0);
	if (be16_to_cpu(leaf->hdr.info.hdr.magic) != XFS_DIR3_LEAF1_MAGIC &&
	    be16_to_cpu(leaf->hdr.info.hdr.magic) != XFS_DIR3_LEAFN_MAGIC)
		return 0;
	return be16_to_cpu(leaf->hdr.count);
}

static int
dir2_leaf_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_leaf	*leaf = obj;

	ASSERT(startoff == 0);
	return be16_to_cpu(leaf->hdr.info.magic) == XFS_DIR2_LEAF1_MAGIC ||
	       be16_to_cpu(leaf->hdr.info.magic) == XFS_DIR2_LEAFN_MAGIC;
}

static int
dir3_leaf_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir3_leaf	*leaf = obj;

	ASSERT(startoff == 0);
	return be16_to_cpu(leaf->hdr.info.hdr.magic) == XFS_DIR3_LEAF1_MAGIC ||
	       be16_to_cpu(leaf->hdr.info.hdr.magic) == XFS_DIR3_LEAFN_MAGIC;
}

static int
dir2_leaf_tail_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_leaf	*leaf = obj;

	ASSERT(startoff == 0);
	return be16_to_cpu(leaf->hdr.info.magic) == XFS_DIR2_LEAF1_MAGIC;
}

static int
dir3_leaf_tail_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir3_leaf	*leaf = obj;

	ASSERT(startoff == 0);
	return be16_to_cpu(leaf->hdr.info.hdr.magic) == XFS_DIR3_LEAF1_MAGIC;
}

static int
dir2_leaf_tail_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_dir2_leaf	*leaf = obj;
	struct xfs_dir2_leaf_tail *ltp;

	ASSERT(startoff == 0);
	ASSERT(idx == 0);
	ASSERT(be16_to_cpu(leaf->hdr.info.magic) == XFS_DIR2_LEAF1_MAGIC ||
	       be16_to_cpu(leaf->hdr.info.magic) == XFS_DIR3_LEAF1_MAGIC);
	ltp = xfs_dir2_leaf_tail_p(mp->m_dir_geo, leaf);
	return bitize((int)((char *)ltp - (char *)leaf));
}

/*
 * Node format functions
 */
static int
dir2_node_btree_count(
	void			*obj,
	int			startoff)
{
	xfs_da_intnode_t	*node = obj;

	ASSERT(startoff == 0);
	if (be16_to_cpu(node->hdr.info.magic) != XFS_DA_NODE_MAGIC)
		return 0;
	return be16_to_cpu(node->hdr.__count);
}

static int
dir3_node_btree_count(
	void			*obj,
	int			startoff)
{
	struct xfs_da3_intnode	*node = obj;

	ASSERT(startoff == 0);
	if (be16_to_cpu(node->hdr.info.hdr.magic) != XFS_DA3_NODE_MAGIC)
		return 0;
	return be16_to_cpu(node->hdr.__count);
}

static int
dir2_node_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_da_intnode	*node = obj;

	ASSERT(startoff == 0);
	return be16_to_cpu(node->hdr.info.magic) == XFS_DA_NODE_MAGIC;
}

static int
dir3_node_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_da3_intnode	*node = obj;

	ASSERT(startoff == 0);
	return be16_to_cpu(node->hdr.info.hdr.magic) == XFS_DA3_NODE_MAGIC;
}

int
dir2_size(
	void	*obj,
	int	startoff,
	int	idx)
{
	return bitize(mp->m_dir_geo->blksize);
}

/*
 * CRC enabled structure definitions
 */
const field_t	dir3_hfld[] = {
	{ "", FLDT_DIR3, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	B3OFF(f)	bitize(offsetof(struct xfs_dir3_data_hdr, f))
#define	D3OFF(f)	bitize(offsetof(struct xfs_dir3_data_hdr, f))
#define	F3OFF(f)	bitize(offsetof(struct xfs_dir3_free, f))
#define	L3OFF(f)	bitize(offsetof(struct xfs_dir3_leaf, f))
#define	N3OFF(f)	bitize(offsetof(struct xfs_da3_intnode, f))
const field_t	dir3_flds[] = {
	{ "bhdr", FLDT_DIR3_DATA_HDR, OI(B3OFF(hdr)), dir3_block_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "bu", FLDT_DIR3_DATA_UNION, dir2_block_u_offset, dir2_block_u_count,
	  FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "bleaf", FLDT_DIR2_LEAF_ENTRY, dir2_block_leaf_offset,
	  dir2_block_leaf_count, FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "btail", FLDT_DIR2_BLOCK_TAIL, dir2_block_tail_offset,
	  dir3_block_tail_count, FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "dhdr", FLDT_DIR3_DATA_HDR, OI(D3OFF(hdr)), dir3_data_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "du", FLDT_DIR3_DATA_UNION, dir2_data_u_offset, dir2_data_u_count,
	  FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "lhdr", FLDT_DIR3_LEAF_HDR, OI(L3OFF(hdr)), dir3_leaf_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "lbests", FLDT_DIR2_DATA_OFF, dir2_leaf_bests_offset,
	  dir2_leaf_bests_count, FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "lents", FLDT_DIR2_LEAF_ENTRY, OI(L3OFF(__ents)), dir3_leaf_ents_count,
	  FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "ltail", FLDT_DIR2_LEAF_TAIL, dir2_leaf_tail_offset,
	  dir3_leaf_tail_count, FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ "nhdr", FLDT_DA3_NODE_HDR, OI(N3OFF(hdr)), dir3_node_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "nbtree", FLDT_DA_NODE_ENTRY, OI(N3OFF(__btree)), dir3_node_btree_count,
	  FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "fhdr", FLDT_DIR3_FREE_HDR, OI(F3OFF(hdr)), dir3_free_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "fbests", FLDT_DIR2_DATA_OFFNZ, OI(F3OFF(bests)),
	  dir3_free_bests_count, FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ NULL }
};

#define	D3EOFF(f)	bitize(offsetof(xfs_dir2_data_entry_t, f))
#define	D3UOFF(f)	bitize(offsetof(xfs_dir2_data_unused_t, f))
const field_t	dir3_data_union_flds[] = {
	{ "freetag", FLDT_UINT16X, OI(D3UOFF(freetag)),
	  dir2_data_union_freetag_count, FLD_COUNT, TYP_NONE },
	{ "inumber", FLDT_INO, OI(D3EOFF(inumber)),
	  dir2_data_union_inumber_count, FLD_COUNT, TYP_INODE },
	{ "length", FLDT_DIR2_DATA_OFF, OI(D3UOFF(length)),
	  dir2_data_union_length_count, FLD_COUNT, TYP_NONE },
	{ "namelen", FLDT_UINT8D, OI(D3EOFF(namelen)),
	  dir2_data_union_namelen_count, FLD_COUNT, TYP_NONE },
	{ "name", FLDT_CHARNS, OI(D3EOFF(name)), dir2_data_union_name_count,
	  FLD_COUNT, TYP_NONE },
	{ "filetype", FLDT_UINT8D, dir3_data_union_ftype_offset, C1,
	  FLD_OFFSET, TYP_NONE },
	{ "tag", FLDT_DIR2_DATA_OFF, dir2_data_union_tag_offset,
	  dir2_data_union_tag_count, FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ NULL }
};

#define	DBH3OFF(f)	bitize(offsetof(struct xfs_dir3_blk_hdr, f))
const field_t	dir3_blkhdr_flds[] = {
	{ "magic", FLDT_UINT32X, OI(DBH3OFF(magic)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(DBH3OFF(crc)), C1, 0, TYP_NONE },
	{ "bno", FLDT_DFSBNO, OI(DBH3OFF(blkno)), C1, 0, TYP_BMAPBTD },
	{ "lsn", FLDT_UINT64X, OI(DBH3OFF(lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(DBH3OFF(uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_INO, OI(DBH3OFF(owner)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	DH3OFF(f)	bitize(offsetof(struct xfs_dir3_data_hdr, f))
const field_t	dir3_data_hdr_flds[] = {
	{ "hdr", FLDT_DIR3_BLKHDR, OI(DH3OFF(hdr)), C1, 0, TYP_NONE },
	{ "bestfree", FLDT_DIR2_DATA_FREE, OI(DH3OFF(best_free)),
	  CI(XFS_DIR2_DATA_FD_COUNT), FLD_ARRAY, TYP_NONE },
	{ "pad", FLDT_UINT32X, OI(DH3OFF(pad)), C1, FLD_SKIPALL, TYP_NONE },
	{ NULL }
};

#define	LH3OFF(f)	bitize(offsetof(struct xfs_dir3_leaf_hdr, f))
const field_t	dir3_leaf_hdr_flds[] = {
	{ "info", FLDT_DA3_BLKINFO, OI(LH3OFF(info)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT16D, OI(LH3OFF(count)), C1, 0, TYP_NONE },
	{ "stale", FLDT_UINT16D, OI(LH3OFF(stale)), C1, 0, TYP_NONE },
	{ "pad", FLDT_UINT32X, OI(LH3OFF(pad)), C1, FLD_SKIPALL, TYP_NONE },
	{ NULL }
};

#define	FH3OFF(f)	bitize(offsetof(struct xfs_dir3_free_hdr, f))
const field_t	dir3_free_hdr_flds[] = {
	{ "hdr", FLDT_DIR3_BLKHDR, OI(FH3OFF(hdr)), C1, 0, TYP_NONE },
	{ "firstdb", FLDT_INT32D, OI(FH3OFF(firstdb)), C1, 0, TYP_NONE },
	{ "nvalid", FLDT_INT32D, OI(FH3OFF(nvalid)), C1, 0, TYP_NONE },
	{ "nused", FLDT_INT32D, OI(FH3OFF(nused)), C1, 0, TYP_NONE },
	{ "pad", FLDT_UINT32X, OI(FH3OFF(pad)), C1, FLD_SKIPALL, TYP_NONE },
	{ NULL }
};


#define	DB3OFF(f)	bitize(offsetof(struct xfs_da3_blkinfo, f))
const field_t	da3_blkinfo_flds[] = {
	{ "hdr", FLDT_DA_BLKINFO, OI(DB3OFF(hdr)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(DB3OFF(crc)), C1, 0, TYP_NONE },
	{ "bno", FLDT_DFSBNO, OI(DB3OFF(blkno)), C1, 0, TYP_BMAPBTD },
	{ "lsn", FLDT_UINT64X, OI(DB3OFF(lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(DB3OFF(uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_INO, OI(DB3OFF(owner)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	H3OFF(f)	bitize(offsetof(struct xfs_da3_node_hdr, f))
const field_t	da3_node_hdr_flds[] = {
	{ "info", FLDT_DA3_BLKINFO, OI(H3OFF(info)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT16D, OI(H3OFF(__count)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(H3OFF(__level)), C1, 0, TYP_NONE },
	{ "pad", FLDT_UINT32X, OI(H3OFF(__pad32)), C1, FLD_SKIPALL, TYP_NONE },
	{ NULL }
};

/* Set the CRC. */
void
xfs_dir3_set_crc(
	struct xfs_buf		*bp)
{
	__be32			magic32;
	__be16			magic16;

	magic32 = *(__be32 *)bp->b_addr;
	magic16 = ((struct xfs_da_blkinfo *)bp->b_addr)->magic;

	switch (magic32) {
	case cpu_to_be32(XFS_DIR3_BLOCK_MAGIC):
	case cpu_to_be32(XFS_DIR3_DATA_MAGIC):
		xfs_buf_update_cksum(bp, XFS_DIR3_DATA_CRC_OFF);
		return;
	case cpu_to_be32(XFS_DIR3_FREE_MAGIC):
		xfs_buf_update_cksum(bp, XFS_DIR3_FREE_CRC_OFF);
		return;
	default:
		break;
	}

	switch (magic16) {
	case cpu_to_be16(XFS_DIR3_LEAF1_MAGIC):
	case cpu_to_be16(XFS_DIR3_LEAFN_MAGIC):
		xfs_buf_update_cksum(bp, XFS_DIR3_LEAF_CRC_OFF);
		return;
	case cpu_to_be16(XFS_DA3_NODE_MAGIC):
		xfs_buf_update_cksum(bp, XFS_DA3_NODE_CRC_OFF);
		return;
	default:
		dbprintf(_("Unknown directory buffer type! %x %x\n"), magic32, magic16);
		break;
	}
}

/*
 * Special read verifier for directory buffers. Detect the magic number
 * appropriately and set the correct verifier and call it.
 */
static void
xfs_dir3_db_read_verify(
	struct xfs_buf		*bp)
{
	__be32			magic32;
	__be16			magic16;

	magic32 = *(__be32 *)bp->b_addr;
	magic16 = ((struct xfs_da_blkinfo *)bp->b_addr)->magic;

	switch (magic32) {
	case cpu_to_be32(XFS_DIR3_BLOCK_MAGIC):
		bp->b_ops = &xfs_dir3_block_buf_ops;
		goto verify;
	case cpu_to_be32(XFS_DIR3_DATA_MAGIC):
		bp->b_ops = &xfs_dir3_data_buf_ops;
		goto verify;
	case cpu_to_be32(XFS_DIR3_FREE_MAGIC):
		bp->b_ops = &xfs_dir3_free_buf_ops;
		goto verify;
	default:
		break;
	}

	switch (magic16) {
	case cpu_to_be16(XFS_DIR3_LEAF1_MAGIC):
		bp->b_ops = &xfs_dir3_leaf1_buf_ops;
		break;
	case cpu_to_be16(XFS_DIR3_LEAFN_MAGIC):
		bp->b_ops = &xfs_dir3_leafn_buf_ops;
		break;
	case cpu_to_be16(XFS_DA3_NODE_MAGIC):
		bp->b_ops = &xfs_da3_node_buf_ops;
		break;
	default:
		dbprintf(_("Unknown directory buffer type!\n"));
		xfs_buf_ioerror(bp, -EFSCORRUPTED);
		return;
	}
verify:
	bp->b_ops->verify_read(bp);
}

static void
xfs_dir3_db_write_verify(
	struct xfs_buf		*bp)
{
	dbprintf(_("Writing unknown directory buffer type!\n"));
	xfs_buf_ioerror(bp, -EFSCORRUPTED);
}

const struct xfs_buf_ops xfs_dir3_db_buf_ops = {
	.name = "xfs_dir3",
	.verify_read = xfs_dir3_db_read_verify,
	.verify_write = xfs_dir3_db_write_verify,
};
