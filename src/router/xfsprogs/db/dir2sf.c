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
#include "bit.h"
#include "dir2sf.h"

static int	dir2_inou_i4_count(void *obj, int startoff);
static int	dir2_inou_i8_count(void *obj, int startoff);
static int	dir2_sf_entry_inumber_offset(void *obj, int startoff, int idx);
static int	dir2_sf_entry_name_count(void *obj, int startoff);
static int	dir2_sf_list_count(void *obj, int startoff);
static int	dir2_sf_list_offset(void *obj, int startoff, int idx);

#define	OFF(f)	bitize(offsetof(xfs_dir2_sf_t, f))
const field_t	dir2sf_flds[] = {
	{ "hdr", FLDT_DIR2_SF_HDR, OI(OFF(hdr)), C1, 0, TYP_NONE },
	{ "list", FLDT_DIR2_SF_ENTRY, dir2_sf_list_offset, dir2_sf_list_count,
	  FLD_ARRAY|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ NULL }
};

#define UOFF(f)	bitize(offsetof(xfs_dir2_inou_t, f))
const field_t	dir2_inou_flds[] = {
	{ "i8", FLDT_DIR2_INO8, OI(UOFF(i8)), dir2_inou_i8_count, FLD_COUNT,
	  TYP_INODE },
	{ "i4", FLDT_DIR2_INO4, OI(UOFF(i4)), dir2_inou_i4_count, FLD_COUNT,
	  TYP_INODE },
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
	xfs_dir2_sf_t	*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (xfs_dir2_sf_t *)XFS_DFORK_DPTR(obj);
	return sf->hdr.i8count == 0;
}

/*ARGSUSED*/
static int
dir2_inou_i8_count(
	void		*obj,
	int		startoff)
{
	xfs_dir2_sf_t	*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (xfs_dir2_sf_t *)XFS_DFORK_DPTR(obj);
	return sf->hdr.i8count != 0;
}

/*ARGSUSED*/
int
dir2_inou_size(
	void		*obj,
	int		startoff,
	int		idx)
{
	xfs_dir2_sf_t	*sf;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	sf = (xfs_dir2_sf_t *)XFS_DFORK_DPTR(obj);
	return bitize(sf->hdr.i8count ?
		      (uint)sizeof(xfs_dir2_ino8_t) :
		      (uint)sizeof(xfs_dir2_ino4_t));
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

/*ARGSUSED*/
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

int
dir2_sf_entry_size(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_sf_entry_t	*e;
	int			i;
	xfs_dir2_sf_t		*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (xfs_dir2_sf_t *)((char *)obj + byteize(startoff));
	e = xfs_dir2_sf_firstentry(sf);
	for (i = 0; i < idx; i++)
		e = xfs_dir2_sf_nextentry(sf, e);
	return bitize((int)xfs_dir2_sf_entsize_byentry(sf, e));
}

/*ARGSUSED*/
int
dir2_sf_hdr_size(
	void		*obj,
	int		startoff,
	int		idx)
{
	xfs_dir2_sf_t	*sf;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	sf = (xfs_dir2_sf_t *)((char *)obj + byteize(startoff));
	return bitize(xfs_dir2_sf_hdr_size(sf->hdr.i8count));
}

static int
dir2_sf_list_count(
	void			*obj,
	int			startoff)
{
	xfs_dir2_sf_t		*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (xfs_dir2_sf_t *)((char *)obj + byteize(startoff));
	return sf->hdr.count;
}

static int
dir2_sf_list_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_dir2_sf_entry_t	*e;
	int			i;
	xfs_dir2_sf_t		*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (xfs_dir2_sf_t *)((char *)obj + byteize(startoff));
	e = xfs_dir2_sf_firstentry(sf);
	for (i = 0; i < idx; i++)
		e = xfs_dir2_sf_nextentry(sf, e);
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
	xfs_dir2_sf_t		*sf;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	sf = (xfs_dir2_sf_t *)((char *)obj + byteize(startoff));
	e = xfs_dir2_sf_firstentry(sf);
	for (i = 0; i < sf->hdr.count; i++)
		e = xfs_dir2_sf_nextentry(sf, e);
	return bitize((int)((char *)e - (char *)sf));
}
