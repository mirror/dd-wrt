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

#include <xfs/libxfs.h>
#include "bit.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "dir.h"
#include "io.h"
#include "init.h"

static int	dir_leaf_entries_count(void *obj, int startoff);
static int	dir_leaf_hdr_count(void *obj, int startoff);
static int	dir_leaf_name_count(void *obj, int startoff);
static int	dir_leaf_namelist_count(void *obj, int startoff);
static int	dir_leaf_namelist_offset(void *obj, int startoff, int idx);
static int	dir_node_btree_count(void *obj, int startoff);
static int	dir_node_hdr_count(void *obj, int startoff);

const field_t	dir_hfld[] = {
	{ "", FLDT_DIR, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	LOFF(f)	bitize(offsetof(xfs_dir_leafblock_t, f))
#define	NOFF(f)	bitize(offsetof(xfs_da_intnode_t, f))
const field_t	dir_flds[] = {
	{ "lhdr", FLDT_DIR_LEAF_HDR, OI(LOFF(hdr)), dir_leaf_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "nhdr", FLDT_DIR_NODE_HDR, OI(NOFF(hdr)), dir_node_hdr_count,
	  FLD_COUNT, TYP_NONE },
	{ "entries", FLDT_DIR_LEAF_ENTRY, OI(LOFF(entries)),
	  dir_leaf_entries_count, FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "btree", FLDT_DIR_NODE_ENTRY, OI(NOFF(btree)),
	  dir_node_btree_count, FLD_ARRAY|FLD_COUNT, TYP_NONE },
	{ "namelist", FLDT_DIR_LEAF_NAME, dir_leaf_namelist_offset,
	  dir_leaf_namelist_count, FLD_ARRAY|FLD_OFFSET|FLD_COUNT, TYP_NONE },
	{ NULL }
};

#define	BOFF(f)	bitize(offsetof(xfs_da_blkinfo_t, f))
const field_t	dir_blkinfo_flds[] = {
	{ "forw", FLDT_DIRBLOCK, OI(BOFF(forw)), C1, 0, TYP_INODATA },
	{ "back", FLDT_DIRBLOCK, OI(BOFF(back)), C1, 0, TYP_INODATA },
	{ "magic", FLDT_UINT16X, OI(BOFF(magic)), C1, 0, TYP_NONE },
	{ "pad", FLDT_UINT16X, OI(BOFF(pad)), C1, FLD_SKIPALL, TYP_NONE },
	{ NULL }
};

#define	LEOFF(f)	bitize(offsetof(xfs_dir_leaf_entry_t, f))
const field_t	dir_leaf_entry_flds[] = {
	{ "hashval", FLDT_UINT32X, OI(LEOFF(hashval)), C1, 0, TYP_NONE },
	{ "nameidx", FLDT_UINT16D, OI(LEOFF(nameidx)), C1, 0, TYP_NONE },
	{ "namelen", FLDT_UINT8D, OI(LEOFF(namelen)), C1, 0, TYP_NONE },
	{ "pad2", FLDT_UINT8X, OI(LEOFF(pad2)), C1, FLD_SKIPALL, TYP_NONE },
	{ NULL }
};

#define	LHOFF(f)	bitize(offsetof(xfs_dir_leaf_hdr_t, f))
const field_t	dir_leaf_hdr_flds[] = {
	{ "info", FLDT_DIR_BLKINFO, OI(LHOFF(info)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT16D, OI(LHOFF(count)), C1, 0, TYP_NONE },
	{ "namebytes", FLDT_UINT16D, OI(LHOFF(namebytes)), C1, 0, TYP_NONE },
	{ "firstused", FLDT_UINT16D, OI(LHOFF(firstused)), C1, 0, TYP_NONE },
	{ "holes", FLDT_UINT8D, OI(LHOFF(holes)), C1, 0, TYP_NONE },
	{ "pad1", FLDT_UINT8X, OI(LHOFF(pad1)), C1, FLD_SKIPALL, TYP_NONE },
	{ "freemap", FLDT_DIR_LEAF_MAP, OI(LHOFF(freemap)),
	  CI(XFS_DIR_LEAF_MAPSIZE), FLD_ARRAY, TYP_NONE },
	{ NULL }
};

#define	LMOFF(f)	bitize(offsetof(xfs_dir_leaf_map_t, f))
const field_t	dir_leaf_map_flds[] = {
	{ "base", FLDT_UINT16D, OI(LMOFF(base)), C1, 0, TYP_NONE },
	{ "size", FLDT_UINT16D, OI(LMOFF(size)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	LNOFF(f)	bitize(offsetof(xfs_dir_leaf_name_t, f))
const field_t	dir_leaf_name_flds[] = {
	{ "inumber", FLDT_DIR_INO, OI(LNOFF(inumber)), C1, 0, TYP_INODE },
	{ "name", FLDT_CHARNS, OI(LNOFF(name)), dir_leaf_name_count, FLD_COUNT,
	  TYP_NONE },
	{ NULL }
};

#define	EOFF(f)	bitize(offsetof(xfs_da_node_entry_t, f))
const field_t	dir_node_entry_flds[] = {
	{ "hashval", FLDT_UINT32X, OI(EOFF(hashval)), C1, 0, TYP_NONE },
	{ "before", FLDT_DIRBLOCK, OI(EOFF(before)), C1, 0, TYP_INODATA },
	{ NULL }
};

#define	HOFF(f)	bitize(offsetof(xfs_da_node_hdr_t, f))
const field_t	dir_node_hdr_flds[] = {
	{ "info", FLDT_DIR_BLKINFO, OI(HOFF(info)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT16D, OI(HOFF(count)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(HOFF(level)), C1, 0, TYP_NONE },
	{ NULL }
};

/*ARGSUSED*/
static int
dir_leaf_entries_count(
	void			*obj,
	int			startoff)
{
	xfs_dir_leafblock_t	*block;

	ASSERT(startoff == 0);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_DIR_LEAF_MAGIC)
		return 0;
	return be16_to_cpu(block->hdr.count);
}

/*ARGSUSED*/
static int
dir_leaf_hdr_count(
	void			*obj,
	int			startoff)
{
	xfs_dir_leafblock_t	*block;

	ASSERT(startoff == 0);
	block = obj;
	return be16_to_cpu(block->hdr.info.magic) == XFS_DIR_LEAF_MAGIC;
}

static int
dir_leaf_name_count(
	void			*obj,
	int			startoff)
{
	xfs_dir_leafblock_t	*block;
	xfs_dir_leaf_entry_t	*e;
	int			i;
	int			off;

	ASSERT(bitoffs(startoff) == 0);
	off = byteize(startoff);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_DIR_LEAF_MAGIC)
		return 0;
	for (i = 0; i < be16_to_cpu(block->hdr.count); i++) {
		e = &block->entries[i];
		if (be16_to_cpu(e->nameidx) == off)
			return e->namelen;
	}
	return 0;
}

/*ARGSUSED*/
int
dir_leaf_name_size(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir_leafblock_t	*block;
	xfs_dir_leaf_entry_t	*e;

	ASSERT(startoff == 0);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_DIR_LEAF_MAGIC)
		return 0;
	e = &block->entries[idx];
	return bitize((int)xfs_dir_leaf_entsize_byentry(e));
}

/*ARGSUSED*/
static int
dir_leaf_namelist_count(
	void			*obj,
	int			startoff)
{
	xfs_dir_leafblock_t	*block;

	ASSERT(startoff == 0);
	block = obj;
	if (be16_to_cpu(block->hdr.info.magic) != XFS_DIR_LEAF_MAGIC)
		return 0;
	return be16_to_cpu(block->hdr.count);
}

/*ARGSUSED*/
static int
dir_leaf_namelist_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir_leafblock_t	*block;
	xfs_dir_leaf_entry_t	*e;

	ASSERT(startoff == 0);
	block = obj;
	e = &block->entries[idx];
	return bitize(be16_to_cpu(e->nameidx));
}

/*ARGSUSED*/
static int
dir_node_btree_count(
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
dir_node_hdr_count(
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
dir_size(
	void	*obj,
	int	startoff,
	int	idx)
{
	return bitize(mp->m_sb.sb_blocksize);
}
