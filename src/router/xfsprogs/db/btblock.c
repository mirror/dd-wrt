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
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "btblock.h"
#include "print.h"
#include "bit.h"
#include "init.h"


/*
 * Definition of the possible btree block layouts.
 */
struct xfs_db_btree {
	size_t			block_len;
	size_t			key_len;
	size_t			rec_len;
	size_t			ptr_len;
} btrees[] = {
	[/*0x424d415*/0] = { /* BMAP */
		XFS_BTREE_LBLOCK_LEN,
		sizeof(xfs_bmbt_key_t),
		sizeof(xfs_bmbt_rec_t),
		sizeof(__be64),
	},
	[/*0x4142544*/2] = { /* ABTB */
		XFS_BTREE_SBLOCK_LEN,
		sizeof(xfs_alloc_key_t),
		sizeof(xfs_alloc_rec_t),
		sizeof(__be32),
	},
	[/*0x4142544*/3] = { /* ABTC */
		XFS_BTREE_SBLOCK_LEN,
		sizeof(xfs_alloc_key_t),
		sizeof(xfs_alloc_rec_t),
		sizeof(__be32),
	},
	[/*0x4941425*/4] = { /* IABT */
		XFS_BTREE_SBLOCK_LEN,
		sizeof(xfs_inobt_key_t),
		sizeof(xfs_inobt_rec_t),
		sizeof(__be32),
	},
};

/*
 * Find the right block defintion for a given ondisk block.
 *
 * We use the least significant bit of the magic number as index into
 * the array of block defintions.
 */
#define block_to_bt(bb) \
	(&btrees[be32_to_cpu((bb)->bb_magic) & 0xf])

/* calculate max records.  Only for non-leaves. */
static int
btblock_maxrecs(struct xfs_db_btree *bt, int blocksize)
{
	blocksize -= bt->block_len;

	return blocksize / (bt->key_len + bt->ptr_len);
}

/*
 * Get the number of keys in a btree block.
 *
 * Note: can also be used to get the number of ptrs because there are
 * always the same number of keys and ptrs in a block.
 */
static int
btblock_key_count(
	void			*obj,
	int			startoff)
{
	struct xfs_btree_block	*block = obj;

	ASSERT(startoff == 0);

	if (block->bb_level == 0)
		return 0;
	return be16_to_cpu(block->bb_numrecs);
}

/*
 * Get the number of keys in a btree block.
 */
static int
btblock_rec_count(
	void			*obj,
	int			startoff)
{
	struct xfs_btree_block	*block = obj;

	ASSERT(startoff == 0);

	if (block->bb_level != 0)
		return 0;
	return be16_to_cpu(block->bb_numrecs);
}

/*
 * Get the offset of the key at idx in a btree block.
 */
static int
btblock_key_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_btree_block	*block = obj;
	struct xfs_db_btree	*bt = block_to_bt(block);
	int			offset;

	ASSERT(startoff == 0);
	ASSERT(block->bb_level != 0);

	offset = bt->block_len + (idx - 1) * bt->key_len;
	return bitize(offset);
}

/*
 * Get the offset of the ptr at idx in a btree block.
 */
static int
btblock_ptr_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_btree_block	*block = obj;
	struct xfs_db_btree	*bt = block_to_bt(block);
	int			offset;
	int			maxrecs;

	ASSERT(startoff == 0);
	ASSERT(block->bb_level != 0);

	maxrecs = btblock_maxrecs(bt, mp->m_sb.sb_blocksize);
	offset = bt->block_len +
			maxrecs * bt->key_len +
			(idx - 1) * bt->ptr_len;

	return bitize(offset);
}

/*
 * Get the offset of the record at idx in a btree block.
 */
static int
btblock_rec_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_btree_block	*block = obj;
	struct xfs_db_btree	*bt = block_to_bt(block);
	int			offset;

	ASSERT(startoff == 0);
	ASSERT(block->bb_level == 0);

	offset = bt->block_len + (idx - 1) * bt->rec_len;
	return bitize(offset);
}

/*
 * Get the size of a btree block.
 */
int
btblock_size(
	void	*obj,
	int	startoff,
	int	idx)
{
	return bitize(mp->m_sb.sb_blocksize);
}


/*
 * Bmap btree.
 */

const field_t	bmapbta_hfld[] = {
	{ "", FLDT_BMAPBTA, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};
const field_t	bmapbtd_hfld[] = {
	{ "", FLDT_BMAPBTD, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	OFF(f)	bitize(offsetof(struct xfs_btree_block, bb_ ## f))
const field_t	bmapbta_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_DFSBNO, OI(OFF(u.l.bb_leftsib)), C1, 0, TYP_BMAPBTA },
	{ "rightsib", FLDT_DFSBNO, OI(OFF(u.l.bb_rightsib)), C1, 0, TYP_BMAPBTA },
	{ "recs", FLDT_BMAPBTAREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_BMAPBTAKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_BMAPBTAPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_BMAPBTA },
	{ NULL }
};
const field_t	bmapbtd_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_DFSBNO, OI(OFF(u.l.bb_leftsib)), C1, 0, TYP_BMAPBTD },
	{ "rightsib", FLDT_DFSBNO, OI(OFF(u.l.bb_rightsib)), C1, 0, TYP_BMAPBTD },
	{ "recs", FLDT_BMAPBTDREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_BMAPBTDKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_BMAPBTDPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_BMAPBTD },
	{ NULL }
};
#undef OFF

#define	KOFF(f)	bitize(offsetof(xfs_bmbt_key_t, br_ ## f))
const field_t	bmapbta_key_flds[] = {
	{ "startoff", FLDT_DFILOFFA, OI(KOFF(startoff)), C1, 0, TYP_ATTR },
	{ NULL }
};
const field_t	bmapbtd_key_flds[] = {
	{ "startoff", FLDT_DFILOFFD, OI(KOFF(startoff)), C1, 0, TYP_INODATA },
	{ NULL }
};
#undef KOFF

#ifndef XFS_NATIVE_HOST

#define BMBT_EXNTFLAG_BITOFF	0
#define BMBT_STARTOFF_BITOFF	(BMBT_EXNTFLAG_BITOFF + BMBT_EXNTFLAG_BITLEN)
#define BMBT_STARTBLOCK_BITOFF	(BMBT_STARTOFF_BITOFF + BMBT_STARTOFF_BITLEN)
#define BMBT_BLOCKCOUNT_BITOFF	\
	(BMBT_STARTBLOCK_BITOFF + BMBT_STARTBLOCK_BITLEN)

#else

#define BMBT_EXNTFLAG_BITOFF	63
#define BMBT_STARTOFF_BITOFF	(BMBT_EXNTFLAG_BITOFF - BMBT_STARTOFF_BITLEN)
#define BMBT_STARTBLOCK_BITOFF	85 /* 128 - 43 (other 9 is in first word) */
#define BMBT_BLOCKCOUNT_BITOFF	64 /* Start of second 64 bit container */

#endif /* XFS_NATIVE_HOST */

const field_t	bmapbta_rec_flds[] = {
	{ "startoff", FLDT_CFILEOFFA, OI(BMBT_STARTOFF_BITOFF), C1, 0,
	  TYP_ATTR },
	{ "startblock", FLDT_CFSBLOCK, OI(BMBT_STARTBLOCK_BITOFF), C1, 0,
	  TYP_ATTR },
	{ "blockcount", FLDT_CEXTLEN, OI(BMBT_BLOCKCOUNT_BITOFF), C1, 0,
	  TYP_NONE },
	{ "extentflag", FLDT_CEXTFLG, OI(BMBT_EXNTFLAG_BITOFF), C1, 0,
	  TYP_NONE },
	{ NULL }
};
const field_t	bmapbtd_rec_flds[] = {
	{ "startoff", FLDT_CFILEOFFD, OI(BMBT_STARTOFF_BITOFF), C1, 0,
	  TYP_INODATA },
	{ "startblock", FLDT_CFSBLOCK, OI(BMBT_STARTBLOCK_BITOFF), C1, 0,
	  TYP_INODATA },
	{ "blockcount", FLDT_CEXTLEN, OI(BMBT_BLOCKCOUNT_BITOFF), C1, 0,
	  TYP_NONE },
	{ "extentflag", FLDT_CEXTFLG, OI(BMBT_EXNTFLAG_BITOFF), C1, 0,
	  TYP_NONE },
	{ NULL }
};


/*
 * Inode allocation btree.
 */

const field_t	inobt_hfld[] = {
	{ "", FLDT_INOBT, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	OFF(f)	bitize(offsetof(struct xfs_btree_block, bb_ ## f))
const field_t	inobt_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_INOBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_INOBT },
	{ "recs", FLDT_INOBTREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_INOBTKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_INOBTPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_INOBT },
	{ NULL }
};
#undef OFF

#define	KOFF(f)	bitize(offsetof(xfs_inobt_key_t, ir_ ## f))
const field_t	inobt_key_flds[] = {
	{ "startino", FLDT_AGINO, OI(KOFF(startino)), C1, 0, TYP_INODE },
	{ NULL }
};
#undef KOFF

#define	ROFF(f)	bitize(offsetof(xfs_inobt_rec_t, ir_ ## f))
const field_t	inobt_rec_flds[] = {
	{ "startino", FLDT_AGINO, OI(ROFF(startino)), C1, 0, TYP_INODE },
	{ "freecount", FLDT_INT32D, OI(ROFF(freecount)), C1, 0, TYP_NONE },
	{ "free", FLDT_INOFREE, OI(ROFF(free)), C1, 0, TYP_NONE },
	{ NULL }
};
#undef ROFF


/*
 * Allocation btrees.
 */
const field_t	bnobt_hfld[] = {
	{ "", FLDT_BNOBT, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	OFF(f)	bitize(offsetof(struct xfs_btree_block, bb_ ## f))
const field_t	bnobt_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_BNOBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_BNOBT },
	{ "recs", FLDT_BNOBTREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_BNOBTKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_BNOBTPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_BNOBT },
	{ NULL }
};
#undef OFF

#define	KOFF(f)	bitize(offsetof(xfs_alloc_key_t, ar_ ## f))
const field_t	bnobt_key_flds[] = {
	{ "startblock", FLDT_AGBLOCK, OI(KOFF(startblock)), C1, 0, TYP_DATA },
	{ "blockcount", FLDT_EXTLEN, OI(KOFF(blockcount)), C1, 0, TYP_NONE },
	{ NULL }
};
#undef KOFF

#define	ROFF(f)	bitize(offsetof(xfs_alloc_rec_t, ar_ ## f))
const field_t	bnobt_rec_flds[] = {
	{ "startblock", FLDT_AGBLOCK, OI(ROFF(startblock)), C1, 0, TYP_DATA },
	{ "blockcount", FLDT_EXTLEN, OI(ROFF(blockcount)), C1, 0, TYP_NONE },
	{ NULL }
};
#undef ROFF

const field_t	cntbt_hfld[] = {
	{ "", FLDT_CNTBT, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	OFF(f)	bitize(offsetof(struct xfs_btree_block, bb_ ## f))
const field_t	cntbt_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_CNTBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_CNTBT },
	{ "recs", FLDT_CNTBTREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_CNTBTKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_CNTBTPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_CNTBT },
	{ NULL }
};
#undef OFF

#define	KOFF(f)	bitize(offsetof(xfs_alloc_key_t, ar_ ## f))
const field_t	cntbt_key_flds[] = {
	{ "blockcount", FLDT_EXTLEN, OI(KOFF(blockcount)), C1, 0, TYP_NONE },
	{ "startblock", FLDT_AGBLOCK, OI(KOFF(startblock)), C1, 0, TYP_DATA },
	{ NULL }
};
#undef KOFF

#define	ROFF(f)	bitize(offsetof(xfs_alloc_rec_t, ar_ ## f))
const field_t	cntbt_rec_flds[] = {
	{ "startblock", FLDT_AGBLOCK, OI(ROFF(startblock)), C1, 0, TYP_DATA },
	{ "blockcount", FLDT_EXTLEN, OI(ROFF(blockcount)), C1, 0, TYP_NONE },
	{ NULL }
};
#undef ROFF
