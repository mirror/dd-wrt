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
#include "bmroot.h"
#include "io.h"
#include "print.h"
#include "bit.h"
#include "init.h"

static int	bmroota_key_count(void *obj, int startoff);
static int	bmroota_key_offset(void *obj, int startoff, int idx);
static int	bmroota_ptr_count(void *obj, int startoff);
static int	bmroota_ptr_offset(void *obj, int startoff, int idx);
static int	bmrootd_key_count(void *obj, int startoff);
static int	bmrootd_key_offset(void *obj, int startoff, int idx);
static int	bmrootd_ptr_count(void *obj, int startoff);
static int	bmrootd_ptr_offset(void *obj, int startoff, int idx);

#define	OFF(f)	bitize(offsetof(xfs_bmdr_block_t, bb_ ## f))
const field_t	bmroota_flds[] = {
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "keys", FLDT_BMROOTAKEY, bmroota_key_offset, bmroota_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_BMROOTAPTR, bmroota_ptr_offset, bmroota_ptr_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_BMAPBTA },
	{ NULL }
};
const field_t	bmrootd_flds[] = {
	{ "level", FLDT_UINT16D, OI(OFF(level)), C1, 0, TYP_NONE },
	{ "numrecs", FLDT_UINT16D, OI(OFF(numrecs)), C1, 0, TYP_NONE },
	{ "keys", FLDT_BMROOTDKEY, bmrootd_key_offset, bmrootd_key_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ "ptrs", FLDT_BMROOTDPTR, bmrootd_ptr_offset, bmrootd_ptr_count,
	  FLD_ARRAY|FLD_ABASE1|FLD_COUNT|FLD_OFFSET, TYP_BMAPBTD },
	{ NULL }
};

#define	KOFF(f)	bitize(offsetof(xfs_bmdr_key_t, br_ ## f))
const field_t	bmroota_key_flds[] = {
	{ "startoff", FLDT_DFILOFFA, OI(KOFF(startoff)), C1, 0, TYP_NONE },
	{ NULL }
};
const field_t	bmrootd_key_flds[] = {
	{ "startoff", FLDT_DFILOFFD, OI(KOFF(startoff)), C1, 0, TYP_NONE },
	{ NULL }
};

static int
bmroota_key_count(
	void			*obj,
	int			startoff)
{
	xfs_bmdr_block_t	*block;
#ifdef DEBUG
	struct xfs_dinode	*dip = obj;
#endif

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(obj == iocur_top->data);
	block = (xfs_bmdr_block_t *)((char *)obj + byteize(startoff));
	ASSERT(dip->di_forkoff != 0 && (char *)block == XFS_DFORK_APTR(dip));
	ASSERT(be16_to_cpu(block->bb_level) > 0);
	return be16_to_cpu(block->bb_numrecs);
}

static int
bmroota_key_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_bmdr_block_t	*block;
#ifdef DEBUG
	struct xfs_dinode	*dip = obj;
#endif
	xfs_bmdr_key_t		*kp;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(obj == iocur_top->data);
	block = (xfs_bmdr_block_t *)((char *)obj + byteize(startoff));
	ASSERT(dip->di_forkoff != 0 && (char *)block == XFS_DFORK_APTR(dip));
	ASSERT(be16_to_cpu(block->bb_level) > 0);
	kp = XFS_BMDR_KEY_ADDR(block, idx);
	return bitize((int)((char *)kp - (char *)block));
}

static int
bmroota_ptr_count(
	void			*obj,
	int			startoff)
{
	xfs_bmdr_block_t	*block;
#ifdef DEBUG
	struct xfs_dinode	*dip = obj;
#endif

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(obj == iocur_top->data);
	block = (xfs_bmdr_block_t *)((char *)obj + byteize(startoff));
	ASSERT(dip->di_forkoff != 0 && (char *)block == XFS_DFORK_APTR(dip));
	ASSERT(be16_to_cpu(block->bb_level) > 0);
	return be16_to_cpu(block->bb_numrecs);
}

static int
bmroota_ptr_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_bmdr_block_t	*block;
	struct xfs_dinode	*dip;
	xfs_bmdr_ptr_t		*pp;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(obj == iocur_top->data);
	dip = obj;
	block = (xfs_bmdr_block_t *)((char *)obj + byteize(startoff));
	ASSERT(dip->di_forkoff != 0 && (char *)block == XFS_DFORK_APTR(dip));
	ASSERT(be16_to_cpu(block->bb_level) > 0);
	pp = XFS_BMDR_PTR_ADDR(block, idx,
		libxfs_bmdr_maxrecs(XFS_DFORK_ASIZE(dip, mp), 0));
	return bitize((int)((char *)pp - (char *)block));
}

int
bmroota_size(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_dinode	*dip;
#ifdef DEBUG
	xfs_bmdr_block_t	*block;
#endif

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(obj == iocur_top->data);
	ASSERT(idx == 0);
	dip = obj;
#ifdef DEBUG
	block = (xfs_bmdr_block_t *)((char *)obj + byteize(startoff));
	ASSERT(dip->di_forkoff != 0 && (char *)block == XFS_DFORK_APTR(dip));
#endif
	return bitize((int)XFS_DFORK_ASIZE(dip, mp));
}

static int
bmrootd_key_count(
	void			*obj,
	int			startoff)
{
	xfs_bmdr_block_t	*block;
#ifdef DEBUG
	struct xfs_dinode	*dip = obj;
#endif

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(obj == iocur_top->data);
	block = (xfs_bmdr_block_t *)((char *)obj + byteize(startoff));
	ASSERT((char *)block == XFS_DFORK_DPTR(dip));
	ASSERT(be16_to_cpu(block->bb_level) > 0);
	return be16_to_cpu(block->bb_numrecs);
}

static int
bmrootd_key_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_bmdr_block_t	*block;
	xfs_bmdr_key_t		*kp;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(obj == iocur_top->data);
	block = (xfs_bmdr_block_t *)((char *)obj + byteize(startoff));
	ASSERT(be16_to_cpu(block->bb_level) > 0);
	kp = XFS_BMDR_KEY_ADDR(block, idx);
	return bitize((int)((char *)kp - (char *)block));
}

static int
bmrootd_ptr_count(
	void			*obj,
	int			startoff)
{
	xfs_bmdr_block_t	*block;
#ifdef DEBUG
	struct xfs_dinode	*dip = obj;
#endif

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(obj == iocur_top->data);
	block = (xfs_bmdr_block_t *)((char *)obj + byteize(startoff));
	ASSERT((char *)block == XFS_DFORK_DPTR(dip));
	ASSERT(be16_to_cpu(block->bb_level) > 0);
	return be16_to_cpu(block->bb_numrecs);
}

static int
bmrootd_ptr_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_bmdr_block_t	*block;
	xfs_bmdr_ptr_t		*pp;
	struct xfs_dinode	*dip;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(obj == iocur_top->data);
	dip = obj;
	block = (xfs_bmdr_block_t *)((char *)obj + byteize(startoff));
	ASSERT(be16_to_cpu(block->bb_level) > 0);
	pp = XFS_BMDR_PTR_ADDR(block, idx,
		libxfs_bmdr_maxrecs(XFS_DFORK_DSIZE(dip, mp), 0));
	return bitize((int)((char *)pp - (char *)block));
}

int
bmrootd_size(
	void			*obj,
	int			startoff,
	int			idx)
{
	struct xfs_dinode	*dip;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(obj == iocur_top->data);
	ASSERT(idx == 0);
	dip = obj;
	return bitize((int)XFS_DFORK_DSIZE(dip, mp));
}
