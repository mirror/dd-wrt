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

#include <xfs/libxfs.h>
#include "bit.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "attr.h"
#include "io.h"
#include "init.h"

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
	{ "entries", FLDT_ATTR_LEAF_ENTRY, OI(LOFF(entries)),
	  attr_leaf_entries_count, FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "btree", FLDT_ATTR_NODE_ENTRY, OI(NOFF(btree)), attr_node_btree_count,
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
	  OI(LEOFF(flags) + bitsz(__uint8_t) - XFS_ATTR_INCOMPLETE_BIT - 1), C1,
	  0, TYP_NONE },
	{ "root", FLDT_UINT1,
	  OI(LEOFF(flags) + bitsz(__uint8_t) - XFS_ATTR_ROOT_BIT - 1), C1, 0,
	  TYP_NONE },
	{ "secure", FLDT_UINT1,
	  OI(LEOFF(flags) + bitsz(__uint8_t) - XFS_ATTR_SECURE_BIT - 1), C1, 0,
	  TYP_NONE },
	{ "local", FLDT_UINT1,
	  OI(LEOFF(flags) + bitsz(__uint8_t) - XFS_ATTR_LOCAL_BIT - 1), C1, 0,
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
	{ "count", FLDT_UINT16D, OI(HOFF(count)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(HOFF(level)), C1, 0, TYP_NONE },
	{ NULL }
};

/*ARGSUSED*/
static int
attr_leaf_entries_count(
	void			*obj,
	int			startoff)
{
	xfs_attr_leafblock_t	*block;

	ASSERT(startoff == 0);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC) 
		return 0;
	return be16_to_cpu(block->hdr.count);
}

/*ARGSUSED*/
static int
attr_leaf_hdr_count(
	void			*obj,
	int			startoff)
{
	xfs_attr_leafblock_t	*block;

	ASSERT(startoff == 0);
	block = obj;
	return be16_to_cpu(block->hdr.info.magic) == XFS_ATTR_LEAF_MAGIC;
}

static int
attr_leaf_name_local_count(
	void			*obj,
	int			startoff)
{
	xfs_attr_leafblock_t	*block;
	xfs_attr_leaf_entry_t	*e;
	int			i;
	int			off;

	ASSERT(bitoffs(startoff) == 0);
	off = byteize(startoff);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC)
		return 0;
	for (i = 0; i < be16_to_cpu(block->hdr.count); i++) {
		e = &block->entries[i];
		if (be16_to_cpu(e->nameidx) == off)
			return (e->flags & XFS_ATTR_LOCAL) != 0;
	}
	return 0;
}

static int
attr_leaf_name_local_name_count(
	void				*obj,
	int				startoff)
{
	xfs_attr_leafblock_t		*block;
	xfs_attr_leaf_entry_t		*e;
	int				i;
	xfs_attr_leaf_name_local_t	*l;
	int				off;

	ASSERT(bitoffs(startoff) == 0);
	off = byteize(startoff);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC)
		return 0;
	for (i = 0; i < be16_to_cpu(block->hdr.count); i++) {
		e = &block->entries[i];
		if (be16_to_cpu(e->nameidx) == off) {
			if (e->flags & XFS_ATTR_LOCAL) {
				l = xfs_attr_leaf_name_local(block, i);
				return l->namelen;
			} else
				return 0;
		}
	}
	return 0;
}

static int
attr_leaf_name_local_value_count(
	void				*obj,
	int				startoff)
{
	xfs_attr_leafblock_t		*block;
	xfs_attr_leaf_entry_t		*e;
	int				i;
	xfs_attr_leaf_name_local_t	*l;
	int				off;

	ASSERT(bitoffs(startoff) == 0);
	off = byteize(startoff);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC)
		return 0;
	for (i = 0; i < be16_to_cpu(block->hdr.count); i++) {
		e = &block->entries[i];
		if (be16_to_cpu(e->nameidx) == off) {
			if (e->flags & XFS_ATTR_LOCAL) {
				l = xfs_attr_leaf_name_local(block, i);
				return be16_to_cpu(l->valuelen);
			} else
				return 0;
		}
	}
	return 0;
}

/*ARGSUSED*/
static int
attr_leaf_name_local_value_offset(
	void				*obj,
	int				startoff,
	int				idx)
{
	xfs_attr_leafblock_t		*block;
	xfs_attr_leaf_name_local_t	*l;
	char				*vp;
	int				off;
	xfs_attr_leaf_entry_t		*e;
	int				i;

	ASSERT(bitoffs(startoff) == 0);
	off = byteize(startoff);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC)
		return 0;

	for (i = 0; i < be16_to_cpu(block->hdr.count); i++) {
		e = &block->entries[i];
		if (be16_to_cpu(e->nameidx) == off)
			break;
	}
	if (i >= be16_to_cpu(block->hdr.count)) 
		return 0;

	l = xfs_attr_leaf_name_local(block, i);
	vp = (char *)&l->nameval[l->namelen];
	return (int)bitize(vp - (char *)l);
}

static int
attr_leaf_name_remote_count(
	void			*obj,
	int			startoff)
{
	xfs_attr_leafblock_t	*block;
	xfs_attr_leaf_entry_t	*e;
	int			i;
	int			off;

	ASSERT(bitoffs(startoff) == 0);
	off = byteize(startoff);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC)
		return 0;
	for (i = 0; i < be16_to_cpu(block->hdr.count); i++) {
		e = &block->entries[i];
		if (be16_to_cpu(e->nameidx) == off)
			return (e->flags & XFS_ATTR_LOCAL) == 0;
	}
	return 0;
}

static int
attr_leaf_name_remote_name_count(
	void				*obj,
	int				startoff)
{
	xfs_attr_leafblock_t		*block;
	xfs_attr_leaf_entry_t		*e;
	int				i;
	int				off;
	xfs_attr_leaf_name_remote_t	*r;

	ASSERT(bitoffs(startoff) == 0);
	off = byteize(startoff);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC)
		return 0;
	for (i = 0; i < be16_to_cpu(block->hdr.count); i++) {
		e = &block->entries[i];
		if (be16_to_cpu(e->nameidx) == off) {
			if (!(e->flags & XFS_ATTR_LOCAL)) {
				r = xfs_attr_leaf_name_remote(block, i);
				return r->namelen;
			} else
				return 0;
		}
	}
	return 0;
}

/*ARGSUSED*/
int
attr_leaf_name_size(
	void				*obj,
	int				startoff,
	int				idx)
{
	xfs_attr_leafblock_t		*block;
	xfs_attr_leaf_entry_t		*e;
	xfs_attr_leaf_name_local_t	*l;
	xfs_attr_leaf_name_remote_t	*r;

	ASSERT(startoff == 0);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC)
		return 0;
	e = &block->entries[idx];
	if (e->flags & XFS_ATTR_LOCAL) {
		l = xfs_attr_leaf_name_local(block, idx);
		return (int)bitize(xfs_attr_leaf_entsize_local(l->namelen,
					be16_to_cpu(l->valuelen)));
	} else {
		r = xfs_attr_leaf_name_remote(block, idx);
		return (int)bitize(xfs_attr_leaf_entsize_remote(r->namelen));
	}
}

/*ARGSUSED*/
static int
attr_leaf_nvlist_count(
	void			*obj,
	int			startoff)
{
	xfs_attr_leafblock_t	*block;

	ASSERT(startoff == 0);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_ATTR_LEAF_MAGIC)
		return 0;
	return be16_to_cpu(block->hdr.count);
}

/*ARGSUSED*/
static int
attr_leaf_nvlist_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_attr_leafblock_t	*block;
	xfs_attr_leaf_entry_t	*e;

	ASSERT(startoff == 0);
	block = obj;
	e = &block->entries[idx];
	return bitize(be16_to_cpu(e->nameidx));
}

/*ARGSUSED*/
static int
attr_node_btree_count(
	void			*obj,
	int			startoff)
{
	xfs_da_intnode_t	*block;

	ASSERT(startoff == 0);		/* this is a base structure */
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_DA_NODE_MAGIC)
		return 0;
	return be16_to_cpu(block->hdr.count);
}

/*ARGSUSED*/
static int
attr_node_hdr_count(
	void			*obj,
	int			startoff)
{
	xfs_da_intnode_t	*block;

	ASSERT(startoff == 0);
	block = obj;
	return be16_to_cpu(block->hdr.info.magic) == XFS_DA_NODE_MAGIC;
}

/*ARGSUSED*/
int
attr_size(
	void	*obj,
	int	startoff,
	int	idx)
{
	return bitize(mp->m_sb.sb_blocksize);
}
