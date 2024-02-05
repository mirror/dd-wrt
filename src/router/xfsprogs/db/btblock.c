// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "btblock.h"
#include "print.h"
#include "bit.h"
#include "init.h"
#include "io.h"
#include "output.h"

/*
 * Definition of the possible btree block layouts.
 */
static struct xfs_db_btree {
	uint32_t		magic;
	size_t			block_len;
	size_t			key_len;
	size_t			rec_len;
	size_t			ptr_len;
} btrees[] = {
	{	XFS_BMAP_MAGIC,
		XFS_BTREE_LBLOCK_LEN,
		sizeof(xfs_bmbt_key_t),
		sizeof(xfs_bmbt_rec_t),
		sizeof(__be64),
	},
	{	XFS_ABTB_MAGIC,
		XFS_BTREE_SBLOCK_LEN,
		sizeof(xfs_alloc_key_t),
		sizeof(xfs_alloc_rec_t),
		sizeof(__be32),
	},
	{	XFS_ABTC_MAGIC,
		XFS_BTREE_SBLOCK_LEN,
		sizeof(xfs_alloc_key_t),
		sizeof(xfs_alloc_rec_t),
		sizeof(__be32),
	},
	{	XFS_IBT_MAGIC,
		XFS_BTREE_SBLOCK_LEN,
		sizeof(xfs_inobt_key_t),
		sizeof(xfs_inobt_rec_t),
		sizeof(__be32),
	},
	{	XFS_FIBT_MAGIC,
		XFS_BTREE_SBLOCK_LEN,
		sizeof(xfs_inobt_key_t),
		sizeof(xfs_inobt_rec_t),
		sizeof(__be32),
	},
	{	XFS_BMAP_CRC_MAGIC,
		XFS_BTREE_LBLOCK_CRC_LEN,
		sizeof(xfs_bmbt_key_t),
		sizeof(xfs_bmbt_rec_t),
		sizeof(__be64),
	},
	{	XFS_ABTB_CRC_MAGIC,
		XFS_BTREE_SBLOCK_CRC_LEN,
		sizeof(xfs_alloc_key_t),
		sizeof(xfs_alloc_rec_t),
		sizeof(__be32),
	},
	{	XFS_ABTC_CRC_MAGIC,
		XFS_BTREE_SBLOCK_CRC_LEN,
		sizeof(xfs_alloc_key_t),
		sizeof(xfs_alloc_rec_t),
		sizeof(__be32),
	},
	{	XFS_IBT_CRC_MAGIC,
		XFS_BTREE_SBLOCK_CRC_LEN,
		sizeof(xfs_inobt_key_t),
		sizeof(xfs_inobt_rec_t),
		sizeof(__be32),
	},
	{	XFS_FIBT_CRC_MAGIC,
		XFS_BTREE_SBLOCK_CRC_LEN,
		sizeof(xfs_inobt_key_t),
		sizeof(xfs_inobt_rec_t),
		sizeof(__be32),
	},
	{	XFS_RMAP_CRC_MAGIC,
		XFS_BTREE_SBLOCK_CRC_LEN,
		2 * sizeof(struct xfs_rmap_key),
		sizeof(struct xfs_rmap_rec),
		sizeof(__be32),
	},
	{	XFS_REFC_CRC_MAGIC,
		XFS_BTREE_SBLOCK_CRC_LEN,
		sizeof(struct xfs_refcount_key),
		sizeof(struct xfs_refcount_rec),
		sizeof(__be32),
	},
	{	0,
	},
};

/*
 * Find the right block definition for a given ondisk block.
 */
static struct xfs_db_btree *
block_to_bt(
	struct xfs_btree_block	*bb)
{
	struct xfs_db_btree	*btp;
	uint32_t		magic;
	bool			crc;

	magic = be32_to_cpu((bb)->bb_magic);
	for (btp = &btrees[0]; btp->magic != 0; btp++) {
		if (magic == btp->magic)
			return btp;
	}

	/* Magic is invalid/unknown.  Guess based on iocur type */
	crc = xfs_has_crc(mp);
	switch (iocur_top->typ->typnm) {
	case TYP_BMAPBTA:
	case TYP_BMAPBTD:
		magic = crc ? XFS_BMAP_CRC_MAGIC : XFS_BMAP_MAGIC;
		break;
	case TYP_BNOBT:
		magic = crc ? XFS_ABTB_CRC_MAGIC : XFS_ABTB_MAGIC;
		break;
	case TYP_CNTBT:
		magic = crc ? XFS_ABTC_CRC_MAGIC : XFS_ABTC_MAGIC;
		break;
	case TYP_INOBT:
		magic = crc ? XFS_IBT_CRC_MAGIC : XFS_IBT_MAGIC;
		break;
	case TYP_FINOBT:
		magic = crc ? XFS_FIBT_CRC_MAGIC : XFS_FIBT_MAGIC;
		break;
	case TYP_RMAPBT:
		magic = crc ? XFS_RMAP_CRC_MAGIC : 0;
		break;
	case TYP_REFCBT:
		magic = crc ? XFS_REFC_CRC_MAGIC : 0;
		break;
	default:
		ASSERT(0);
	}

	ASSERT(magic);
	dbprintf(_("Bad btree magic 0x%x; coercing to %s.\n"),
		be32_to_cpu((bb)->bb_magic),
		iocur_top->typ->name);

	for (btp = &btrees[0]; btp->magic != 0; btp++) {
		if (magic == btp->magic)
			return btp;
	}

	return NULL;
}

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

const field_t	bmapbta_crc_hfld[] = {
	{ "", FLDT_BMAPBTA_CRC, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};
const field_t	bmapbtd_crc_hfld[] = {
	{ "", FLDT_BMAPBTD_CRC, OI(0), C1, 0, TYP_NONE },
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
/* crc enabled versions */
const field_t	bmapbta_crc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_DFSBNO, OI(OFF(u.l.bb_leftsib)), C1, 0, TYP_BMAPBTA },
	{ "rightsib", FLDT_DFSBNO, OI(OFF(u.l.bb_rightsib)), C1, 0, TYP_BMAPBTA },
	{ "bno", FLDT_DFSBNO, OI(OFF(u.l.bb_blkno)), C1, 0, TYP_BMAPBTD },
	{ "lsn", FLDT_UINT64X, OI(OFF(u.l.bb_lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(u.l.bb_uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_INO, OI(OFF(u.l.bb_owner)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(u.l.bb_crc)), C1, 0, TYP_NONE },
	{ "recs", FLDT_BMAPBTAREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_BMAPBTAKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_BMAPBTAPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_BMAPBTA },
	{ NULL }
};
const field_t	bmapbtd_crc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_DFSBNO, OI(OFF(u.l.bb_leftsib)), C1, 0, TYP_BMAPBTD },
	{ "rightsib", FLDT_DFSBNO, OI(OFF(u.l.bb_rightsib)), C1, 0, TYP_BMAPBTD },
	{ "bno", FLDT_DFSBNO, OI(OFF(u.l.bb_blkno)), C1, 0, TYP_BMAPBTD },
	{ "lsn", FLDT_UINT64X, OI(OFF(u.l.bb_lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(u.l.bb_uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_INO, OI(OFF(u.l.bb_owner)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(u.l.bb_crc)), C1, 0, TYP_NONE },
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

#define BMBT_EXNTFLAG_BITOFF	0
#define BMBT_STARTOFF_BITOFF	(BMBT_EXNTFLAG_BITOFF + BMBT_EXNTFLAG_BITLEN)
#define BMBT_STARTBLOCK_BITOFF	(BMBT_STARTOFF_BITOFF + BMBT_STARTOFF_BITLEN)
#define BMBT_BLOCKCOUNT_BITOFF	\
	(BMBT_STARTBLOCK_BITOFF + BMBT_STARTBLOCK_BITLEN)

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

const field_t	inobt_crc_hfld[] = {
	{ "", FLDT_INOBT_CRC, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

const field_t	inobt_spcrc_hfld[] = {
	{ "", FLDT_INOBT_SPCRC, OI(0), C1, 0, TYP_NONE },
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
const field_t	inobt_crc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_INOBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_INOBT },
	{ "bno", FLDT_DFSBNO, OI(OFF(u.s.bb_blkno)), C1, 0, TYP_INOBT },
	{ "lsn", FLDT_UINT64X, OI(OFF(u.s.bb_lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(u.s.bb_uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_AGNUMBER, OI(OFF(u.s.bb_owner)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(u.s.bb_crc)), C1, 0, TYP_NONE },
	{ "recs", FLDT_INOBTREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_INOBTKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_INOBTPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_INOBT },
	{ NULL }
};
const field_t	inobt_spcrc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_INOBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_INOBT },
	{ "bno", FLDT_DFSBNO, OI(OFF(u.s.bb_blkno)), C1, 0, TYP_INOBT },
	{ "lsn", FLDT_UINT64X, OI(OFF(u.s.bb_lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(u.s.bb_uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_AGNUMBER, OI(OFF(u.s.bb_owner)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(u.s.bb_crc)), C1, 0, TYP_NONE },
	{ "recs", FLDT_INOBTSPREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_INOBTKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_INOBTPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_INOBT },
	{ NULL }
};

/* free inode btree */

const field_t	finobt_hfld[] = {
	{ "", FLDT_FINOBT, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

const field_t	finobt_crc_hfld[] = {
	{ "", FLDT_FINOBT_CRC, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

const field_t	finobt_spcrc_hfld[] = {
	{ "", FLDT_FINOBT_SPCRC, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

const field_t	finobt_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_INOBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_INOBT },
	{ "recs", FLDT_INOBTREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_INOBTKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_FINOBTPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_FINOBT },
	{ NULL }
};
const field_t	finobt_crc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_INOBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_INOBT },
	{ "bno", FLDT_DFSBNO, OI(OFF(u.s.bb_blkno)), C1, 0, TYP_INOBT },
	{ "lsn", FLDT_UINT64X, OI(OFF(u.s.bb_lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(u.s.bb_uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_AGNUMBER, OI(OFF(u.s.bb_owner)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(u.s.bb_crc)), C1, 0, TYP_NONE },
	{ "recs", FLDT_INOBTREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_INOBTKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_FINOBTPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_FINOBT },
	{ NULL }
};
const field_t	finobt_spcrc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_INOBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_INOBT },
	{ "bno", FLDT_DFSBNO, OI(OFF(u.s.bb_blkno)), C1, 0, TYP_INOBT },
	{ "lsn", FLDT_UINT64X, OI(OFF(u.s.bb_lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(u.s.bb_uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_AGNUMBER, OI(OFF(u.s.bb_owner)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(u.s.bb_crc)), C1, 0, TYP_NONE },
	{ "recs", FLDT_INOBTSPREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_INOBTKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_FINOBTPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_FINOBT },
	{ NULL }
};

#undef OFF

#define	KOFF(f)	bitize(offsetof(xfs_inobt_key_t, ir_ ## f))
const field_t	inobt_key_flds[] = {
	{ "startino", FLDT_AGINO, OI(KOFF(startino)), C1, 0, TYP_INODE },
	{ NULL }
};
#undef KOFF

#define	ROFF(f)	bitize(offsetof(xfs_inobt_rec_t, f))
const field_t	inobt_rec_flds[] = {
	{ "startino", FLDT_AGINO, OI(ROFF(ir_startino)), C1, 0, TYP_INODE },
	{ "freecount", FLDT_INT32D, OI(ROFF(ir_u.f.ir_freecount)), C1, 0, TYP_NONE },
	{ "free", FLDT_INOFREE, OI(ROFF(ir_free)), C1, 0, TYP_NONE },
	{ NULL }
};
/* sparse inode on-disk format */
const field_t	inobt_sprec_flds[] = {
	{ "startino", FLDT_AGINO, OI(ROFF(ir_startino)), C1, 0, TYP_INODE },
	{ "holemask", FLDT_UINT16X, OI(ROFF(ir_u.sp.ir_holemask)), C1, 0,
	  TYP_NONE },
	{ "count", FLDT_UINT8D, OI(ROFF(ir_u.sp.ir_count)), C1, 0, TYP_NONE },
	{ "freecount", FLDT_UINT8D, OI(ROFF(ir_u.sp.ir_freecount)), C1, 0,
	  TYP_NONE },
	{ "free", FLDT_INOFREE, OI(ROFF(ir_free)), C1, 0, TYP_NONE },
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

const field_t	bnobt_crc_hfld[] = {
	{ "", FLDT_BNOBT_CRC, OI(0), C1, 0, TYP_NONE },
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
const field_t	bnobt_crc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_BNOBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_BNOBT },
	{ "bno", FLDT_DFSBNO, OI(OFF(u.s.bb_blkno)), C1, 0, TYP_BNOBT },
	{ "lsn", FLDT_UINT64X, OI(OFF(u.s.bb_lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(u.s.bb_uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_AGNUMBER, OI(OFF(u.s.bb_owner)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(u.s.bb_crc)), C1, 0, TYP_NONE },
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

const field_t	cntbt_crc_hfld[] = {
	{ "", FLDT_CNTBT_CRC, OI(0), C1, 0, TYP_NONE },
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
const field_t	cntbt_crc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_CNTBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_CNTBT },
	{ "bno", FLDT_DFSBNO, OI(OFF(u.s.bb_blkno)), C1, 0, TYP_CNTBT },
	{ "lsn", FLDT_UINT64X, OI(OFF(u.s.bb_lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(u.s.bb_uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_AGNUMBER, OI(OFF(u.s.bb_owner)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(u.s.bb_crc)), C1, 0, TYP_NONE },
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

/* RMAP btree blocks */
const field_t	rmapbt_crc_hfld[] = {
	{ "", FLDT_RMAPBT_CRC, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	OFF(f)	bitize(offsetof(struct xfs_btree_block, bb_ ## f))
const field_t	rmapbt_crc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_RMAPBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_RMAPBT },
	{ "bno", FLDT_DFSBNO, OI(OFF(u.s.bb_blkno)), C1, 0, TYP_RMAPBT },
	{ "lsn", FLDT_UINT64X, OI(OFF(u.s.bb_lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(u.s.bb_uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_AGNUMBER, OI(OFF(u.s.bb_owner)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(u.s.bb_crc)), C1, 0, TYP_NONE },
	{ "recs", FLDT_RMAPBTREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_RMAPBTKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_RMAPBTPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_RMAPBT },
	{ NULL }
};
#undef OFF

#define	KOFF(f)	bitize(offsetof(struct xfs_rmap_key, rm_ ## f))

#define RMAPBK_STARTBLOCK_BITOFF	0
#define RMAPBK_OWNER_BITOFF		(RMAPBK_STARTBLOCK_BITOFF + RMAPBT_STARTBLOCK_BITLEN)
#define RMAPBK_ATTRFLAG_BITOFF		(RMAPBK_OWNER_BITOFF + RMAPBT_OWNER_BITLEN)
#define RMAPBK_BMBTFLAG_BITOFF		(RMAPBK_ATTRFLAG_BITOFF + RMAPBT_ATTRFLAG_BITLEN)
#define RMAPBK_EXNTFLAG_BITOFF		(RMAPBK_BMBTFLAG_BITOFF + RMAPBT_BMBTFLAG_BITLEN)
#define RMAPBK_UNUSED_OFFSET_BITOFF	(RMAPBK_EXNTFLAG_BITOFF + RMAPBT_EXNTFLAG_BITLEN)
#define RMAPBK_OFFSET_BITOFF		(RMAPBK_UNUSED_OFFSET_BITOFF + RMAPBT_UNUSED_OFFSET_BITLEN)

#define HI_KOFF(f)	bitize(sizeof(struct xfs_rmap_key) + offsetof(struct xfs_rmap_key, rm_ ## f))

#define RMAPBK_STARTBLOCKHI_BITOFF	(bitize(sizeof(struct xfs_rmap_key)))
#define RMAPBK_OWNERHI_BITOFF		(RMAPBK_STARTBLOCKHI_BITOFF + RMAPBT_STARTBLOCK_BITLEN)
#define RMAPBK_ATTRFLAGHI_BITOFF	(RMAPBK_OWNERHI_BITOFF + RMAPBT_OWNER_BITLEN)
#define RMAPBK_BMBTFLAGHI_BITOFF	(RMAPBK_ATTRFLAGHI_BITOFF + RMAPBT_ATTRFLAG_BITLEN)
#define RMAPBK_EXNTFLAGHI_BITOFF	(RMAPBK_BMBTFLAGHI_BITOFF + RMAPBT_BMBTFLAG_BITLEN)
#define RMAPBK_UNUSED_OFFSETHI_BITOFF	(RMAPBK_EXNTFLAGHI_BITOFF + RMAPBT_EXNTFLAG_BITLEN)
#define RMAPBK_OFFSETHI_BITOFF		(RMAPBK_UNUSED_OFFSETHI_BITOFF + RMAPBT_UNUSED_OFFSET_BITLEN)

const field_t	rmapbt_key_flds[] = {
	{ "startblock", FLDT_AGBLOCK, OI(KOFF(startblock)), C1, 0, TYP_DATA },
	{ "owner", FLDT_INT64D, OI(KOFF(owner)), C1, 0, TYP_NONE },
	{ "offset", FLDT_RFILEOFFD, OI(RMAPBK_OFFSET_BITOFF), C1, 0, TYP_NONE },
	{ "extentflag", FLDT_REXTFLG, OI(RMAPBK_EXNTFLAG_BITOFF), C1, 0,
	  TYP_NONE },
	{ "attrfork", FLDT_RATTRFORKFLG, OI(RMAPBK_ATTRFLAG_BITOFF), C1, 0,
	  TYP_NONE },
	{ "bmbtblock", FLDT_RBMBTFLG, OI(RMAPBK_BMBTFLAG_BITOFF), C1, 0,
	  TYP_NONE },
	{ "startblock_hi", FLDT_AGBLOCK, OI(HI_KOFF(startblock)), C1, 0, TYP_DATA },
	{ "owner_hi", FLDT_INT64D, OI(HI_KOFF(owner)), C1, 0, TYP_NONE },
	{ "offset_hi", FLDT_RFILEOFFD, OI(RMAPBK_OFFSETHI_BITOFF), C1, 0, TYP_NONE },
	{ "extentflag_hi", FLDT_REXTFLG, OI(RMAPBK_EXNTFLAGHI_BITOFF), C1, 0,
	  TYP_NONE },
	{ "attrfork_hi", FLDT_RATTRFORKFLG, OI(RMAPBK_ATTRFLAGHI_BITOFF), C1, 0,
	  TYP_NONE },
	{ "bmbtblock_hi", FLDT_RBMBTFLG, OI(RMAPBK_BMBTFLAGHI_BITOFF), C1, 0,
	  TYP_NONE },
	{ NULL }
};
#undef HI_KOFF
#undef KOFF

#define RMAPBT_STARTBLOCK_BITOFF	0
#define RMAPBT_BLOCKCOUNT_BITOFF	(RMAPBT_STARTBLOCK_BITOFF + RMAPBT_STARTBLOCK_BITLEN)
#define RMAPBT_OWNER_BITOFF		(RMAPBT_BLOCKCOUNT_BITOFF + RMAPBT_BLOCKCOUNT_BITLEN)
#define RMAPBT_ATTRFLAG_BITOFF		(RMAPBT_OWNER_BITOFF + RMAPBT_OWNER_BITLEN)
#define RMAPBT_BMBTFLAG_BITOFF		(RMAPBT_ATTRFLAG_BITOFF + RMAPBT_ATTRFLAG_BITLEN)
#define RMAPBT_EXNTFLAG_BITOFF		(RMAPBT_BMBTFLAG_BITOFF + RMAPBT_BMBTFLAG_BITLEN)
#define RMAPBT_UNUSED_OFFSET_BITOFF	(RMAPBT_EXNTFLAG_BITOFF + RMAPBT_EXNTFLAG_BITLEN)
#define RMAPBT_OFFSET_BITOFF		(RMAPBT_UNUSED_OFFSET_BITOFF + RMAPBT_UNUSED_OFFSET_BITLEN)

const field_t	rmapbt_rec_flds[] = {
	{ "startblock", FLDT_AGBLOCK, OI(RMAPBT_STARTBLOCK_BITOFF), C1, 0, TYP_DATA },
	{ "blockcount", FLDT_EXTLEN, OI(RMAPBT_BLOCKCOUNT_BITOFF), C1, 0, TYP_NONE },
	{ "owner", FLDT_INT64D, OI(RMAPBT_OWNER_BITOFF), C1, 0, TYP_NONE },
	{ "offset", FLDT_RFILEOFFD, OI(RMAPBT_OFFSET_BITOFF), C1, 0, TYP_NONE },
	{ "extentflag", FLDT_REXTFLG, OI(RMAPBT_EXNTFLAG_BITOFF), C1, 0,
	  TYP_NONE },
	{ "attrfork", FLDT_RATTRFORKFLG, OI(RMAPBT_ATTRFLAG_BITOFF), C1, 0,
	  TYP_NONE },
	{ "bmbtblock", FLDT_RBMBTFLG, OI(RMAPBT_BMBTFLAG_BITOFF), C1, 0,
	  TYP_NONE },
	{ NULL }
};

/* refcount btree blocks */
const field_t	refcbt_crc_hfld[] = {
	{ "", FLDT_REFCBT_CRC, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	OFF(f)	bitize(offsetof(struct xfs_btree_block, bb_ ## f))
const field_t	refcbt_crc_flds[] = {
	{ "magic", FLDT_UINT32X, OI(OFF(magic)), C1, 0, TYP_NONE },
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "leftsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_leftsib)), C1, 0, TYP_REFCBT },
	{ "rightsib", FLDT_AGBLOCK, OI(OFF(u.s.bb_rightsib)), C1, 0, TYP_REFCBT },
	{ "bno", FLDT_DFSBNO, OI(OFF(u.s.bb_blkno)), C1, 0, TYP_REFCBT },
	{ "lsn", FLDT_UINT64X, OI(OFF(u.s.bb_lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(OFF(u.s.bb_uuid)), C1, 0, TYP_NONE },
	{ "owner", FLDT_AGNUMBER, OI(OFF(u.s.bb_owner)), C1, 0, TYP_NONE },
	{ "crc", FLDT_CRC, OI(OFF(u.s.bb_crc)), C1, 0, TYP_NONE },
	{ "recs", FLDT_REFCBTREC, btblock_rec_offset, btblock_rec_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "keys", FLDT_REFCBTKEY, btblock_key_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_REFCBTPTR, btblock_ptr_offset, btblock_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_REFCBT },
	{ NULL }
};
#undef OFF

#define REFCNTBT_COWFLAG_BITOFF		0
#define REFCNTBT_STARTBLOCK_BITOFF	(REFCNTBT_COWFLAG_BITOFF + REFCNTBT_COWFLAG_BITLEN)

const field_t	refcbt_key_flds[] = {
	{ "startblock", FLDT_CAGBLOCK, OI(REFCNTBT_STARTBLOCK_BITOFF), C1, 0, TYP_DATA },
	{ "cowflag", FLDT_CCOWFLG, OI(REFCNTBT_COWFLAG_BITOFF), C1, 0, TYP_DATA },
	{ NULL }
};

#define	ROFF(f)	bitize(offsetof(struct xfs_refcount_rec, rc_ ## f))
const field_t	refcbt_rec_flds[] = {
	{ "startblock", FLDT_CAGBLOCK, OI(REFCNTBT_STARTBLOCK_BITOFF), C1, 0, TYP_DATA },
	{ "blockcount", FLDT_EXTLEN, OI(ROFF(blockcount)), C1, 0, TYP_NONE },
	{ "refcount", FLDT_UINT32D, OI(ROFF(refcount)), C1, 0, TYP_DATA },
	{ "cowflag", FLDT_CCOWFLG, OI(REFCNTBT_COWFLAG_BITOFF), C1, 0, TYP_DATA },
	{ NULL }
};
#undef ROFF
