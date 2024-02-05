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
#include "bit.h"
#include "dir2.h"
#include "dir2sf.h"
#include "init.h"

static int	dir2_inou_i4_count(void *obj, int startoff);
static int	dir2_inou_i8_count(void *obj, int startoff);
static int	dir2_sf_entry_inumber_offset(void *obj, int startoff, int idx);
static int	dir2_sf_entry_name_count(void *obj, int startoff);
static int	dir2_sf_list_count(void *obj, int startoff);
static int	dir2_sf_list_offset(void *obj, int startoff, int idx);

#define	OFF(f)	bitize(offsetof(struct xfs_dir2_sf_hdr, f))
const field_t	dir2sf_flds[] = {
	{ "hdr", FLDT_DIR2_SF_HDR, OI(OFF(count)), C1, 0, TYP_NONE },
	{ "list", FLDT_DIR2_SF_ENTRY, dir2_sf_list_offset, dir2_sf_list_count,
	  FLD_ARRAY|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ NULL }
};

const field_t	dir2_inou_flds[] = {
	{ "i8", FLDT_DIR2_INO8, NULL, dir2_inou_i8_count, FLD_COUNT, TYP_INODE},
	{ "i4", FLDT_DIR2_INO4, NULL, dir2_inou_i4_count, FLD_COUNT, TYP_INODE},
	{ NULL }
};

#define	HOFF(f)	bitize(offsetof(xfs_dir2_sf_hdr_t, f))
const field_t	dir2_sf_hdr_flds[] = {
	{ "count", FLDT_UINT8D, OI(HOFF(count)), C1, 0, TYP_NONE },
	{ "i8count", FLDT_UINT8D, OI(HOFF(i8count)), C1, 0, TYP_NONE },
	{ "parent", FLDT_DIR2_INOU, OI(HOFF(parent)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	EOFF(f)	bitize(offsetof(xfs_dir2_sf_entry_t, f))
const field_t	dir2_sf_entry_flds[] = {
	{ "namelen", FLDT_UINT8D, OI(EOFF(namelen)), C1, 0, TYP_NONE },
	{ "offset", FLDT_DIR2_SF_OFF, OI(EOFF(offset)), C1, 0, TYP_NONE },
	{ "name", FLDT_CHARNS, OI(EOFF(name)), dir2_sf_entry_name_count,
	  FLD_COUNT, TYP_NONE },
	{ "inumber", FLDT_DIR2_INOU, dir2_sf_entry_inumber_offset, C1,
	  FLD_OFFSET, TYP_NONE },
	{ NULL }
};

/*ARGSUSED*/
static int
dir2_inou_i4_count(
	void		*obj,
	int		startoff)
{
	struct xfs_dinode *dip = obj;
	struct xfs_dir2_sf_hdr	*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (struct xfs_dir2_sf_hdr *)XFS_DFORK_DPTR(dip);
	return sf->i8count == 0;
}

/*ARGSUSED*/
static int
dir2_inou_i8_count(
	void		*obj,
	int		startoff)
{
	struct xfs_dinode *dip = obj;
	struct xfs_dir2_sf_hdr	*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (struct xfs_dir2_sf_hdr *)XFS_DFORK_DPTR(dip);
	return sf->i8count != 0;
}

/*ARGSUSED*/
int
dir2_inou_size(
	void		*obj,
	int		startoff,
	int		idx)
{
	struct xfs_dinode *dip = obj;
	struct xfs_dir2_sf_hdr	*sf;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	sf = (struct xfs_dir2_sf_hdr *)XFS_DFORK_DPTR(dip);
	return bitize(sf->i8count ? XFS_INO64_SIZE : XFS_INO32_SIZE);
}

static int
dir2_sf_entry_name_count(
	void			*obj,
	int			startoff)
{
	xfs_dir2_sf_entry_t	*e;

	ASSERT(bitoffs(startoff) == 0);
	e = (xfs_dir2_sf_entry_t *)((char *)obj + byteize(startoff));
	return e->namelen;
}

static int
dir2_sf_entry_inumber_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_sf_entry_t	*e;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	e = (xfs_dir2_sf_entry_t *)((char *)obj + byteize(startoff));
	return bitize((int)((char *)xfs_dir2_sf_inumberp(e) - (char *)e));
}

static int
dir3_sf_entry_inumber_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_sf_entry_t	*e;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	e = (xfs_dir2_sf_entry_t *)((char *)obj + byteize(startoff));
	/* plus 1 to skip the ftype entry */
	return bitize((int)((char *)xfs_dir2_sf_inumberp(e) + 1 - (char *)e));
}

static int
dir3_sf_entry_ftype_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_sf_entry_t	*e;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	e = (xfs_dir2_sf_entry_t *)((char *)obj + byteize(startoff));
	return bitize((int)((char *)&e->name[e->namelen] - (char *)e));
}

int
dir2_sf_entry_size(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_sf_entry_t	*e;
	int			i;
	struct xfs_dir2_sf_hdr	*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (struct xfs_dir2_sf_hdr *)((char *)obj + byteize(startoff));
	e = xfs_dir2_sf_firstentry(sf);
	for (i = 0; i < idx; i++)
		e = libxfs_dir2_sf_nextentry(mp, sf, e);
	return bitize((int)libxfs_dir2_sf_entsize(mp, sf, e->namelen));
}

/*ARGSUSED*/
int
dir2_sf_hdr_size(
	void		*obj,
	int		startoff,
	int		idx)
{
	struct xfs_dir2_sf_hdr	*sf;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	sf = (struct xfs_dir2_sf_hdr *)((char *)obj + byteize(startoff));
	return bitize(xfs_dir2_sf_hdr_size(sf->i8count));
}

static int
dir2_sf_list_count(
	void			*obj,
	int			startoff)
{
	struct xfs_dir2_sf_hdr	*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (struct xfs_dir2_sf_hdr *)((char *)obj + byteize(startoff));
	return sf->count;
}

static int
dir2_sf_list_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_sf_entry_t	*e;
	int			i;
	struct xfs_dir2_sf_hdr	*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (struct xfs_dir2_sf_hdr *)((char *)obj + byteize(startoff));
	e = xfs_dir2_sf_firstentry(sf);
	for (i = 0; i < idx; i++)
		e = libxfs_dir2_sf_nextentry(mp, sf, e);
	return bitize((int)((char *)e - (char *)sf));
}

/*ARGSUSED*/
int
dir2sf_size(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_sf_entry_t	*e;
	int			i;
	struct xfs_dir2_sf_hdr	*sf;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	sf = (struct xfs_dir2_sf_hdr *)((char *)obj + byteize(startoff));
	e = xfs_dir2_sf_firstentry(sf);
	for (i = 0; i < sf->count; i++)
		e = libxfs_dir2_sf_nextentry(mp, sf, e);
	return bitize((int)((char *)e - (char *)sf));
}

#define	OFF(f)	bitize(offsetof(struct xfs_dir2_sf_hdr, f))
const field_t	dir3sf_flds[] = {
	{ "hdr", FLDT_DIR2_SF_HDR, OI(OFF(count)), C1, 0, TYP_NONE },
	{ "list", FLDT_DIR3_SF_ENTRY, dir2_sf_list_offset, dir2_sf_list_count,
	  FLD_ARRAY|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ NULL }
};

#define	E3OFF(f)	bitize(offsetof(xfs_dir2_sf_entry_t, f))
const field_t	dir3_sf_entry_flds[] = {
	{ "namelen", FLDT_UINT8D, OI(E3OFF(namelen)), C1, 0, TYP_NONE },
	{ "offset", FLDT_DIR2_SF_OFF, OI(E3OFF(offset)), C1, 0, TYP_NONE },
	{ "name", FLDT_CHARNS, OI(E3OFF(name)), dir2_sf_entry_name_count,
	  FLD_COUNT, TYP_NONE },
	{ "inumber", FLDT_DIR2_INOU, dir3_sf_entry_inumber_offset, C1,
	  FLD_OFFSET, TYP_NONE },
	{ "filetype", FLDT_UINT8D, dir3_sf_entry_ftype_offset, C1,
	  FLD_OFFSET, TYP_NONE },
	{ NULL }
};
