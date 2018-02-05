/*
 * Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.
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
#include "bit.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "attr.h"
#include "io.h"
#include "init.h"
#include "output.h"

static int	attr_leaf_entries_count(void *obj, int startoff);
static int	attr_leaf_hdr_count(void *obj, int startoff);
static int	attr_leaf_name_local_count(void *obj, int startoff);
static int	attr_leaf_name_local_name_count(void *obj, int startoff);
static int	attr_leaf_name_local_value_count(void *obj, int startoff);
static int	attr_leaf_name_local_value_offset(void *obj, int startoff,
						  int idx);
static int	attr_leaf_name_remote_count(void *obj, int startoff);
static int	attr_leaf_name_remote_name_count(void *obj, int startoff);
static int	attr_leaf_nvlist_count(void *obj, int startoff);
static int	attr_leaf_nvlist_offset(void *obj, int startoff, int idx);
static int	attr_node_btree_count(void *obj, int startoff);
static int	attr_node_hdr_count(void *obj, int startoff);

static int	attr_remote_data_count(void *obj, int startoff);
static int	attr3_remote_hdr_count(void *obj, int startoff);
static int	attr3_remote_data_count(void *obj, int startoff);

const field_t	attr_hfld[] = {
	{ "", FLDT_ATTR, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	LOFF(f)	bitize(offsetof(xfs_attr_leafblock_t, f))
#define	NOFF(f)	bitize(offsetof(xfs_da_intnode_t, f))
const field_t	attr_flds[] = {
	{ "hdr", FLDT_ATTR_LEAF_HDR, OI(LOFF(hdr)), attr_leaf_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "hdr", FLDT_ATTR_NODE_HDR, OI(NOFF(hdr)), attr_node_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "data", FLDT_CHARNS, OI(0), attr_remote_data_count, FLD_COUNT,
	  TYP_NONE },
	{ "entries", FLDT_ATTR_LEAF_ENTRY, OI(LOFF(entries)),
	  attr_leaf_entries_count, FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "btree", FLDT_ATTR_NODE_ENTRY, OI(NOFF(__btree)), attr_node_btree_count,
	  FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "nvlist", FLDT_ATTR_LEAF_NAME, attr_leaf_nvlist_offset,
	  attr_leaf_nvlist_count, FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ NULL }
};

#define	BOFF(f)	bitize(offsetof(xfs_da_blkinfo_t, f))
const field_t	attr_blkinfo_flds[] = {
	{ "forw", FLDT_ATTRBLOCK, OI(BOFF(forw)), C1, 0, TYP_ATTR },
	{ "back", FLDT_ATTRBLOCK, OI(BOFF(back)), C1, 0, TYP_ATTR },
	{ "magic", FLDT_UINT16X, OI(BOFF(magic)), C1, 0, TYP_NONE },
	{ "pad", FLDT_UINT16X, OI(BOFF(pad)), C1, FLD_SKIPALL, TYP_NONE },
	{ NULL }
};

#define	LEOFF(f)	bitize(offsetof(xfs_attr_leaf_entry_t, f))
const field_t	attr_leaf_entry_flds[] = {
	{ "hashval", FLDT_UINT32X, OI(LEOFF(hashval)), C1, 0, TYP_NONE },
	{ "nameidx", FLDT_UINT16D, OI(LEOFF(nameidx)), C1, 0, TYP_NONE },
	{ "flags", FLDT_UINT8X, OI(LEOFF(flags)), C1, FLD_SKIPALL, TYP_NONE },
	{ "incomplete", FLDT_UINT1,
	  OI(LEOFF(flags) + bitsz(uint8_t) - XFS_ATTR_INCOMPLETE_BIT - 1), C1,
	  0, TYP_NONE },
	{ "root", FLDT_UINT1,
	  OI(LEOFF(flags) + bitsz(uint8_t) - XFS_ATTR_ROOT_BIT - 1), C1, 0,
	  TYP_NONE },
	{ "secure", FLDT_UINT1,
	  OI(LEOFF(flags) + bitsz(uint8_t) - XFS_ATTR_SECURE_BIT - 1), C1, 0,
	  TYP_NONE },
	{ "local", FLDT_UINT1,
	  OI(LEOFF(flags) + bitsz(uint8_t) - XFS_ATTR_LOCAL_BIT - 1), C1, 0,
	  TYP_NONE },
	{ "pad2", FLDT_UINT8X, OI(LEOFF(pad2)), C1, FLD_SKIPALL, TYP_NONE },
	{ NULL }
};

#define	LHOFF(f)	bitize(offsetof(xfs_attr_leaf_hdr_t, f))
const field_t	attr_leaf_hdr_flds[] = {
	{ "info", FLDT_ATTR_BLKINFO, OI(LHOFF(info)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT16D, OI(LHOFF(count)), C1, 0, TYP_NONE },
	{ "usedbytes", FLDT_UINT16D, OI(LHOFF(usedbytes)), C1, 0, TYP_NONE },
	{ "firstused", FLDT_UINT16D, OI(LHOFF(firstused)), C1, 0, TYP_NONE },
	{ "holes", FLDT_UINT8D, OI(LHOFF(holes)), C1, 0, TYP_NONE },
	{ "pad1", FLDT_UINT8X, OI(LHOFF(pad1)), C1, FLD_SKIPALL, TYP_NONE },
	{ "freemap", FLDT_ATTR_LEAF_MAP, OI(LHOFF(freemap)),
	  CI(XFS_ATTR_LEAF_MAPSIZE), FLD_ARRAY, TYP_NONE },
	{ NULL }
};

#define	LMOFF(f)	bitize(offsetof(xfs_attr_leaf_map_t, f))
const field_t	attr_leaf_map_flds[] = {
	{ "base", FLDT_UINT16D, OI(LMOFF(base)), C1, 0, TYP_NONE },
	{ "size", FLDT_UINT16D, OI(LMOFF(size)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	LNOFF(f)	bitize(offsetof(xfs_attr_leaf_name_local_t, f))
#define	LVOFF(f)	bitize(offsetof(xfs_attr_leaf_name_remote_t, f))
const field_t	attr_leaf_name_flds[] = {
	{ "valuelen", FLDT_UINT16D, OI(LNOFF(valuelen)),
	  attr_leaf_name_local_count, FLD_COUNT, TYP_NONE },
	{ "namelen", FLDT_UINT8D, OI(LNOFF(namelen)),
	  attr_leaf_name_local_count, FLD_COUNT, TYP_NONE },
	{ "name", FLDT_CHARNS, OI(LNOFF(nameval)),
	  attr_leaf_name_local_name_count, FLD_COUNT, TYP_NONE },
	{ "value", FLDT_CHARNS, attr_leaf_name_local_value_offset,
	  attr_leaf_name_local_value_count, FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "valueblk", FLDT_UINT32X, OI(LVOFF(valueblk)),
	  attr_leaf_name_remote_count, FLD_COUNT, TYP_NONE },
	{ "valuelen", FLDT_UINT32D, OI(LVOFF(valuelen)),
	  attr_leaf_name_remote_count, FLD_COUNT, TYP_NONE },
	{ "namelen", FLDT_UINT8D, OI(LVOFF(namelen)),
	  attr_leaf_name_remote_count, FLD_COUNT, TYP_NONE },
	{ "name", FLDT_CHARNS, OI(LVOFF(name)),
	  attr_leaf_name_remote_name_count, FLD_COUNT, TYP_NONE },
	{ NULL }
};

#define	EOFF(f)	bitize(offsetof(xfs_da_node_entry_t, f))
const field_t	attr_node_entry_flds[] = {
	{ "hashval", FLDT_UINT32X, OI(EOFF(hashval)), C1, 0, TYP_NONE },
	{ "before", FLDT_ATTRBLOCK, OI(EOFF(before)), C1, 0, TYP_ATTR },
	{ NULL }
};

#define	HOFF(f)	bitize(offsetof(xfs_da_node_hdr_t, f))
const field_t	attr_node_hdr_flds[] = {
	{ "info", FLDT_ATTR_BLKINFO, OI(HOFF(info)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT16D, OI(HOFF(__count)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(HOFF(__level)), C1, 0, TYP_NONE },
	{ NULL }
};

static int
attr_leaf_entries_count(
	void			*obj,
	int			startoff)
{
	struct xfs_attr_leafblock *leaf = obj;

	ASSERT(startoff == 0);
	if (be16_to_cpu(leaf->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC)
		return 0;
	return be16_to_cpu(leaf->hdr.count);
}

static int
attr3_leaf_entries_count(
	void			*obj,
	int			startoff)
{
	struct xfs_attr3_leafblock *leaf = obj;

	ASSERT(startoff == 0);
	if (be16_to_cpu(leaf->hdr.info.hdr.magic) != XFS_ATTR3_LEAF_MAGIC)
		return 0;
	return be16_to_cpu(leaf->hdr.count);
}

static int
attr_leaf_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_attr_leafblock *leaf = obj;

	ASSERT(startoff == 0);
	return be16_to_cpu(leaf->hdr.info.magic) == XFS_ATTR_LEAF_MAGIC;
}

static int
attr3_leaf_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_attr3_leafblock *leaf = obj;

	ASSERT(startoff == 0);
	return be16_to_cpu(leaf->hdr.info.hdr.magic) == XFS_ATTR3_LEAF_MAGIC;
}

static int
attr_remote_data_count(
	void				*obj,
	int				startoff)
{
	if (attr_leaf_hdr_count(obj, startoff) == 0 &&
	    attr_node_hdr_count(obj, startoff) == 0)
		return mp->m_sb.sb_blocksize;
	return 0;
}

static int
attr3_remote_data_count(
	void				*obj,
	int				startoff)
{
	struct xfs_attr3_rmt_hdr	*hdr = obj;
	size_t				buf_space;

	ASSERT(startoff == 0);

	if (hdr->rm_magic != cpu_to_be32(XFS_ATTR3_RMT_MAGIC))
		return 0;
	buf_space = XFS_ATTR3_RMT_BUF_SPACE(mp, mp->m_sb.sb_blocksize);
	if (be32_to_cpu(hdr->rm_bytes) > buf_space)
		return buf_space;
	return be32_to_cpu(hdr->rm_bytes);
}

typedef int (*attr_leaf_entry_walk_f)(struct xfs_attr_leafblock *,
				      struct xfs_attr_leaf_entry *, int);
static int
attr_leaf_entry_walk(
	void				*obj,
	int				startoff,
	attr_leaf_entry_walk_f		func)
{
	struct xfs_attr_leafblock	*leaf = obj;
	struct xfs_attr3_icleaf_hdr	leafhdr;
	struct xfs_attr_leaf_entry	*entries;
	struct xfs_attr_leaf_entry	*e;
	int				i;
	int				off;

	ASSERT(bitoffs(startoff) == 0);
	if (be16_to_cpu(leaf->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC &&
	    be16_to_cpu(leaf->hdr.info.magic) != XFS_ATTR3_LEAF_MAGIC)
		return 0;

	off = byteize(startoff);
	xfs_attr3_leaf_hdr_from_disk(mp->m_attr_geo, &leafhdr, leaf);
	entries = xfs_attr3_leaf_entryp(leaf);

	for (i = 0; i < leafhdr.count; i++) {
		e = &entries[i];
		if (be16_to_cpu(e->nameidx) == off)
			return func(leaf, e, i);
	}
	return 0;
}

static int
__attr_leaf_name_local_count(
	struct xfs_attr_leafblock	*leaf,
	struct xfs_attr_leaf_entry      *e,
	int				i)
{
	return (e->flags & XFS_ATTR_LOCAL) != 0;
}

static int
attr_leaf_name_local_count(
	void			*obj,
	int			startoff)
{
	return attr_leaf_entry_walk(obj, startoff,
				    __attr_leaf_name_local_count);
}

static int
__attr_leaf_name_local_name_count(
	struct xfs_attr_leafblock	*leaf,
	struct xfs_attr_leaf_entry      *e,
	int				i)
{
	struct xfs_attr_leaf_name_local	*l;

	if (!(e->flags & XFS_ATTR_LOCAL))
		return 0;

	l = xfs_attr3_leaf_name_local(leaf, i);
	return l->namelen;
}

static int
attr_leaf_name_local_name_count(
	void				*obj,
	int				startoff)
{
	return attr_leaf_entry_walk(obj, startoff,
				    __attr_leaf_name_local_name_count);
}

static int
__attr_leaf_name_local_value_count(
	struct xfs_attr_leafblock	*leaf,
	struct xfs_attr_leaf_entry      *e,
	int				i)
{
	struct xfs_attr_leaf_name_local	*l;

	if (!(e->flags & XFS_ATTR_LOCAL))
		return 0;

	l = xfs_attr3_leaf_name_local(leaf, i);
	return be16_to_cpu(l->valuelen);
}

static int
attr_leaf_name_local_value_count(
	void				*obj,
	int				startoff)
{
	return attr_leaf_entry_walk(obj, startoff,
				    __attr_leaf_name_local_value_count);
}

static int
__attr_leaf_name_local_value_offset(
	struct xfs_attr_leafblock	*leaf,
	struct xfs_attr_leaf_entry      *e,
	int				i)
{
	struct xfs_attr_leaf_name_local	*l;
	char				*vp;

	l = xfs_attr3_leaf_name_local(leaf, i);
	vp = (char *)&l->nameval[l->namelen];

	return (int)bitize(vp - (char *)l);
}

static int
attr_leaf_name_local_value_offset(
	void				*obj,
	int				startoff,
	int				idx)
{
	return attr_leaf_entry_walk(obj, startoff,
				    __attr_leaf_name_local_value_offset);
}

static int
__attr_leaf_name_remote_count(
	struct xfs_attr_leafblock	*leaf,
	struct xfs_attr_leaf_entry      *e,
	int				i)
{
	return (e->flags & XFS_ATTR_LOCAL) == 0;
}

static int
attr_leaf_name_remote_count(
	void				*obj,
	int				startoff)
{
	return attr_leaf_entry_walk(obj, startoff,
				    __attr_leaf_name_remote_count);
}

static int
__attr_leaf_name_remote_name_count(
	struct xfs_attr_leafblock	*leaf,
	struct xfs_attr_leaf_entry      *e,
	int				i)
{
	struct xfs_attr_leaf_name_remote *r;

	if (e->flags & XFS_ATTR_LOCAL)
		return 0;

	r = xfs_attr3_leaf_name_remote(leaf, i);
	return r->namelen;
}

static int
attr_leaf_name_remote_name_count(
	void				*obj,
	int				startoff)
{
	return attr_leaf_entry_walk(obj, startoff,
				    __attr_leaf_name_remote_name_count);
}

int
attr_leaf_name_size(
	void				*obj,
	int				startoff,
	int				idx)
{
	struct xfs_attr_leafblock	*leaf = obj;
	struct xfs_attr_leaf_entry	*e;
	struct xfs_attr_leaf_name_local	*l;
	struct xfs_attr_leaf_name_remote *r;

	ASSERT(startoff == 0);
	if (be16_to_cpu(leaf->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC &&
	    be16_to_cpu(leaf->hdr.info.magic) != XFS_ATTR3_LEAF_MAGIC)
		return 0;
	e = &xfs_attr3_leaf_entryp(leaf)[idx];
	if (e->flags & XFS_ATTR_LOCAL) {
		l = xfs_attr3_leaf_name_local(leaf, idx);
		return (int)bitize(xfs_attr_leaf_entsize_local(l->namelen,
					be16_to_cpu(l->valuelen)));
	} else {
		r = xfs_attr3_leaf_name_remote(leaf, idx);
		return (int)bitize(xfs_attr_leaf_entsize_remote(r->namelen));
	}
}

static int
attr_leaf_nvlist_count(
	void			*obj,
	int			startoff)
{
	struct xfs_attr_leafblock *leaf = obj;

	ASSERT(startoff == 0);
	if (be16_to_cpu(leaf->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC)
		return 0;
	return be16_to_cpu(leaf->hdr.count);
}

static int
attr3_leaf_nvlist_count(
	void			*obj,
	int			startoff)
{
	struct xfs_attr3_leafblock *leaf = obj;

	ASSERT(startoff == 0);
	if (be16_to_cpu(leaf->hdr.info.hdr.magic) != XFS_ATTR3_LEAF_MAGIC)
		return 0;
	return be16_to_cpu(leaf->hdr.count);
}

static int
attr_leaf_nvlist_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_attr_leafblock *leaf = obj;
	struct xfs_attr_leaf_entry *e;

	ASSERT(startoff == 0);
	e = &xfs_attr3_leaf_entryp(leaf)[idx];
	return bitize(be16_to_cpu(e->nameidx));
}

static int
attr_node_btree_count(
	void			*obj,
	int			startoff)
{
	struct xfs_da_intnode	*node = obj;

	ASSERT(startoff == 0);		/* this is a base structure */
	if (be16_to_cpu(node->hdr.info.magic) != XFS_DA_NODE_MAGIC)
		return 0;
	return be16_to_cpu(node->hdr.__count);
}

static int
attr3_node_btree_count(
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
attr_node_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_da_intnode	*node = obj;

	ASSERT(startoff == 0);
	return be16_to_cpu(node->hdr.info.magic) == XFS_DA_NODE_MAGIC;
}

static int
attr3_node_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_da3_intnode	*node = obj;

	ASSERT(startoff == 0);
	return be16_to_cpu(node->hdr.info.hdr.magic) == XFS_DA3_NODE_MAGIC;
}

static int
attr3_remote_hdr_count(
	void			*obj,
	int			startoff)
{
	struct xfs_attr3_rmt_hdr	*node = obj;

	ASSERT(startoff == 0);
	return be32_to_cpu(node->rm_magic) == XFS_ATTR3_RMT_MAGIC;
}

int
attr_size(
	void	*obj,
	int	startoff,
	int	idx)
{
	return bitize(mp->m_sb.sb_blocksize);
}

/*
 * CRC enabled attribute block field definitions
 */
const field_t	attr3_hfld[] = {
	{ "", FLDT_ATTR3, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	L3OFF(f)	bitize(offsetof(struct xfs_attr3_leafblock, f))
#define	N3OFF(f)	bitize(offsetof(struct xfs_da3_intnode, f))
const field_t	attr3_flds[] = {
	{ "hdr", FLDT_ATTR3_LEAF_HDR, OI(L3OFF(hdr)), attr3_leaf_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "hdr", FLDT_ATTR3_NODE_HDR, OI(N3OFF(hdr)), attr3_node_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "hdr", FLDT_ATTR3_REMOTE_HDR, OI(0), attr3_remote_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "data", FLDT_CHARNS, OI(bitize(sizeof(struct xfs_attr3_rmt_hdr))),
	  attr3_remote_data_count, FLD_COUNT, TYP_NONE },
	{ "entries", FLDT_ATTR_LEAF_ENTRY, OI(L3OFF(entries)),
	  attr3_leaf_entries_count, FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "btree", FLDT_ATTR_NODE_ENTRY, OI(N3OFF(__btree)),
	  attr3_node_btree_count, FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "nvlist", FLDT_ATTR_LEAF_NAME, attr_leaf_nvlist_offset,
	  attr3_leaf_nvlist_count, FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ NULL }
};

#define	LH3OFF(f)	bitize(offsetof(struct xfs_attr3_leaf_hdr, f))
const field_t	attr3_leaf_hdr_flds[] = {
	{ "info", FLDT_ATTR3_BLKINFO, OI(LH3OFF(info)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT16D, OI(LH3OFF(count)), C1, 0, TYP_NONE },
	{ "usedbytes", FLDT_UINT16D, OI(LH3OFF(usedbytes)), C1, 0, TYP_NONE },
	{ "firstused", FLDT_UINT16D, OI(LH3OFF(firstused)), C1, 0, TYP_NONE },
	{ "holes", FLDT_UINT8D, OI(LH3OFF(holes)), C1, 0, TYP_NONE },
	{ "pad1", FLDT_UINT8X, OI(LH3OFF(pad1)), C1, FLD_SKIPALL, TYP_NONE },
	{ "freemap", FLDT_ATTR_LEAF_MAP, OI(LH3OFF(freemap)),
	  CI(XFS_ATTR_LEAF_MAPSIZE), FLD_ARRAY, TYP_NONE },
	{ NULL }
};

#define	B3OFF(f)	bitize(offsetof(struct xfs_da3_blkinfo, f))
const field_t	attr3_blkinfo_flds[] = {
	{ "hdr", FLDT_ATTR_BLKINFO, OI(B3OFF(hdr)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(B3OFF(crc)), C1, 0, TYP_NONE },
	{ "bno", FLDT_DFSBNO, OI(B3OFF(blkno)), C1, 0, TYP_BMAPBTD },
	{ "lsn", FLDT_UINT64X, OI(B3OFF(lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(B3OFF(uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_INO, OI(B3OFF(owner)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	H3OFF(f)	bitize(offsetof(struct xfs_da3_node_hdr, f))
const field_t	attr3_node_hdr_flds[] = {
	{ "info", FLDT_ATTR3_BLKINFO, OI(H3OFF(info)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT16D, OI(H3OFF(__count)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(H3OFF(__level)), C1, 0, TYP_NONE },
	{ "pad", FLDT_UINT32D, OI(H3OFF(__pad32)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	RM3OFF(f)	bitize(offsetof(struct xfs_attr3_rmt_hdr, rm_ ## f))
const struct field	attr3_remote_crc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(RM3OFF(magic)), C1, 0, TYP_NONE },
	{ "offset", FLDT_UINT32D, OI(RM3OFF(offset)), C1, 0, TYP_NONE },
	{ "bytes", FLDT_UINT32D, OI(RM3OFF(bytes)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(RM3OFF(crc)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(RM3OFF(uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_INO, OI(RM3OFF(owner)), C1, 0, TYP_NONE },
	{ "bno", FLDT_DFSBNO, OI(RM3OFF(blkno)), C1, 0, TYP_BMAPBTD },
	{ "lsn", FLDT_UINT64X, OI(RM3OFF(lsn)), C1, 0, TYP_NONE },
	{ NULL }
};

/* Set the CRC. */
void
xfs_attr3_set_crc(
	struct xfs_buf		*bp)
{
	__be32			magic32;
	__be16			magic16;

	magic32 = *(__be32 *)bp->b_addr;
	magic16 = ((struct xfs_da_blkinfo *)bp->b_addr)->magic;

	switch (magic16) {
	case cpu_to_be16(XFS_ATTR3_LEAF_MAGIC):
		xfs_buf_update_cksum(bp, XFS_ATTR3_LEAF_CRC_OFF);
		return;
	case cpu_to_be16(XFS_DA3_NODE_MAGIC):
		xfs_buf_update_cksum(bp, XFS_DA3_NODE_CRC_OFF);
		return;
	default:
		break;
	}

	switch (magic32) {
	case cpu_to_be32(XFS_ATTR3_RMT_MAGIC):
		xfs_buf_update_cksum(bp, XFS_ATTR3_RMT_CRC_OFF);
		return;
	default:
		dbprintf(_("Unknown attribute buffer type!\n"));
		break;
	}
}

/*
 * Special read verifier for attribute buffers. Detect the magic number
 * appropriately and set the correct verifier and call it.
 */
static void
xfs_attr3_db_read_verify(
	struct xfs_buf		*bp)
{
	__be32			magic32;
	__be16			magic16;

	magic32 = *(__be32 *)bp->b_addr;
	magic16 = ((struct xfs_da_blkinfo *)bp->b_addr)->magic;

	switch (magic16) {
	case cpu_to_be16(XFS_ATTR3_LEAF_MAGIC):
		bp->b_ops = &xfs_attr3_leaf_buf_ops;
		goto verify;
	case cpu_to_be16(XFS_DA3_NODE_MAGIC):
		bp->b_ops = &xfs_da3_node_buf_ops;
		goto verify;
	default:
		break;
	}

	switch (magic32) {
	case cpu_to_be32(XFS_ATTR3_RMT_MAGIC):
		bp->b_ops = &xfs_attr3_rmt_buf_ops;
		break;
	default:
		dbprintf(_("Unknown attribute buffer type!\n"));
		xfs_buf_ioerror(bp, -EFSCORRUPTED);
		return;
	}
verify:
	bp->b_ops->verify_read(bp);
}

static void
xfs_attr3_db_write_verify(
	struct xfs_buf		*bp)
{
	dbprintf(_("Writing unknown attribute buffer type!\n"));
	xfs_buf_ioerror(bp, -EFSCORRUPTED);
}

const struct xfs_buf_ops xfs_attr3_db_buf_ops = {
	.name = "xfs_attr3",
	.verify_read = xfs_attr3_db_read_verify,
	.verify_write = xfs_attr3_db_write_verify,
};
