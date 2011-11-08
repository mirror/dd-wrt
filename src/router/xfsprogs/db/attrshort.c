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
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "bit.h"
#include "attrshort.h"

static int	attr_sf_entry_name_count(void *obj, int startoff);
static int	attr_sf_entry_value_count(void *obj, int startoff);
static int	attr_sf_entry_value_offset(void *obj, int startoff, int idx);
static int	attr_shortform_list_count(void *obj, int startoff);
static int	attr_shortform_list_offset(void *obj, int startoff, int idx);

#define	OFF(f)	bitize(offsetof(xfs_attr_shortform_t, f))
const field_t	attr_shortform_flds[] = {
	{ "hdr", FLDT_ATTR_SF_HDR, OI(OFF(hdr)), C1, 0, TYP_NONE },
	{ "list", FLDT_ATTR_SF_ENTRY, attr_shortform_list_offset,
	  attr_shortform_list_count, FLD_ARRAY|FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ NULL }
};

#define	HOFF(f)	bitize(offsetof(xfs_attr_sf_hdr_t, f))
const field_t	attr_sf_hdr_flds[] = {
	{ "totsize", FLDT_UINT16D, OI(HOFF(totsize)), C1, 0, TYP_NONE },
	{ "count", FLDT_UINT8D, OI(HOFF(count)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	EOFF(f)	bitize(offsetof(xfs_attr_sf_entry_t, f))
const field_t	attr_sf_entry_flds[] = {
	{ "namelen", FLDT_UINT8D, OI(EOFF(namelen)), C1, 0, TYP_NONE },
	{ "valuelen", FLDT_UINT8D, OI(EOFF(valuelen)), C1, 0, TYP_NONE },
	{ "flags", FLDT_UINT8X, OI(EOFF(flags)), C1, FLD_SKIPALL, TYP_NONE },
	{ "root", FLDT_UINT1,
	  OI(EOFF(flags) + bitsz(__uint8_t) - XFS_ATTR_ROOT_BIT - 1), C1, 0,
	  TYP_NONE },
	{ "secure", FLDT_UINT1,
	  OI(EOFF(flags) + bitsz(__uint8_t) - XFS_ATTR_SECURE_BIT - 1), C1, 0,
	  TYP_NONE },
	{ "name", FLDT_CHARNS, OI(EOFF(nameval)), attr_sf_entry_name_count,
	  FLD_COUNT, TYP_NONE },
	{ "value", FLDT_CHARNS, attr_sf_entry_value_offset,
	  attr_sf_entry_value_count, FLD_COUNT|FLD_OFFSET, TYP_NONE },
	{ NULL }
};

static int
attr_sf_entry_name_count(
	void			*obj,
	int			startoff)
{
	xfs_attr_sf_entry_t	*e;

	ASSERT(bitoffs(startoff) == 0);
	e = (xfs_attr_sf_entry_t *)((char *)obj + byteize(startoff));
	return e->namelen;
}

int
attr_sf_entry_size(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_attr_sf_entry_t	*e;
	int			i;
	xfs_attr_shortform_t	*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (xfs_attr_shortform_t *)((char *)obj + byteize(startoff));
	e = &sf->list[0];
	for (i = 0; i < idx; i++)
		e = XFS_ATTR_SF_NEXTENTRY(e);
	return bitize((int)XFS_ATTR_SF_ENTSIZE(e));
}

static int
attr_sf_entry_value_count(
	void			*obj,
	int			startoff)
{
	xfs_attr_sf_entry_t	*e;

	ASSERT(bitoffs(startoff) == 0);
	e = (xfs_attr_sf_entry_t *)((char *)obj + byteize(startoff));
	return e->valuelen;
}

/*ARGSUSED*/
static int
attr_sf_entry_value_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_attr_sf_entry_t	*e;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	e = (xfs_attr_sf_entry_t *)((char *)obj + byteize(startoff));
	return bitize((int)((char *)&e->nameval[e->namelen] - (char *)e));
}

static int
attr_shortform_list_count(
	void			*obj,
	int			startoff)
{
	xfs_attr_shortform_t	*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (xfs_attr_shortform_t *)((char *)obj + byteize(startoff));
	return sf->hdr.count;
}

static int
attr_shortform_list_offset(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_attr_sf_entry_t	*e;
	int			i;
	xfs_attr_shortform_t	*sf;

	ASSERT(bitoffs(startoff) == 0);
	sf = (xfs_attr_shortform_t *)((char *)obj + byteize(startoff));
	e = &sf->list[0];
	for (i = 0; i < idx; i++)
		e = XFS_ATTR_SF_NEXTENTRY(e);
	return bitize((int)((char *)e - (char *)sf));
}

/*ARGSUSED*/
int
attrshort_size(
	void			*obj,
	int			startoff,
	int			idx)
{
	xfs_attr_sf_entry_t	*e;
	int			i;
	xfs_attr_shortform_t	*sf;

	ASSERT(bitoffs(startoff) == 0);
	ASSERT(idx == 0);
	sf = (xfs_attr_shortform_t *)((char *)obj + byteize(startoff));
	e = &sf->list[0];
	for (i = 0; i < sf->hdr.count; i++)
		e = XFS_ATTR_SF_NEXTENTRY(e);
	return bitize((int)((char *)e - (char *)sf));
}
