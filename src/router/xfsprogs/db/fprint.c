/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
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
#include <ctype.h>
#include <time.h>
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "inode.h"
#include "btblock.h"
#include "bit.h"
#include "print.h"
#include "output.h"
#include "sig.h"
#include "malloc.h"

int
fp_charns(
	void	*obj,
	int	bit,
	int	count,
	char	*fmtstr,
	int	size,
	int	arg,
	int	base,
	int	array)
{
	int	i;
	char	*p;

	ASSERT(bitoffs(bit) == 0);
	ASSERT(size == bitsz(char));
	dbprintf("\"");
	for (i = 0, p = (char *)obj + byteize(bit);
	     i < count && !seenint();
	     i++, p++) {
		if (*p == '\\' || *p == '\'' || *p == '"' || *p == '\?')
			dbprintf("\\%c", *p);
		else if (isgraph((int)*p) || *p == ' ')
			dbprintf("%c", *p);
		else if (*p == '\a' || *p == '\b' || *p == '\f' || *p == '\n' ||
			 *p == '\r' || *p == '\t' || *p == '\v')
			dbprintf("\\%c", *p + ('a' - '\a'));
		else
			dbprintf("\\%03o", *p & 0xff);
	}
	dbprintf("\"");
	return 1;
}

int
fp_num(
	void		*obj,
	int		bit,
	int		count,
	char		*fmtstr,
	int		size,
	int		arg,
	int		base,
	int		array)
{
	int		bitpos;
	int		i;
	int		isnull;
	__int64_t	val;

	for (i = 0, bitpos = bit;
	     i < count && !seenint();
	     i++, bitpos += size) {
		val = getbitval(obj, bitpos, size,
			(arg & FTARG_SIGNED) ? BVSIGNED : BVUNSIGNED);
		if ((arg & FTARG_SKIPZERO) && val == 0)
			continue;
		isnull = (arg & FTARG_SIGNED) || size == 64 ?
			val == -1LL : val == ((1LL << size) - 1LL);
		if ((arg & FTARG_SKIPNULL) && isnull)
			continue;
		if (array)
			dbprintf("%d:", i + base);
		if ((arg & FTARG_DONULL) && isnull)
			dbprintf(_("null"));
		else if (size > 32)
			dbprintf(fmtstr, val);
		else
			dbprintf(fmtstr, (__int32_t)val);
		if (i < count - 1)
			dbprintf(" ");
	}
	return 1;
}

/*ARGSUSED*/
int
fp_sarray(
	void	*obj,
	int	bit,
	int	count,
	char	*fmtstr,
	int	size,
	int	arg,
	int	base,
	int	array)
{
	print_sarray(obj, bit, count, size, base, array,
		(const field_t *)fmtstr, (arg & FTARG_SKIPNMS) != 0);
	return 1;
}

/*ARGSUSED*/
int
fp_time(
	void	*obj,
	int	bit,
	int	count,
	char	*fmtstr,
	int	size,
	int	arg,
	int	base,
	int	array)
{
	int	bitpos;
	char	*c;
	int	i;
	time_t  t;

	ASSERT(bitoffs(bit) == 0);
	for (i = 0, bitpos = bit;
	     i < count && !seenint();
	     i++, bitpos += size) {
		if (array)
			dbprintf("%d:", i + base);
		t=(time_t)getbitval((char *)obj + byteize(bitpos), 0, sizeof(int32_t)*8, 0);
		c = ctime(&t);
		dbprintf("%24.24s", c);
		if (i < count - 1)
			dbprintf(" ");
	}
	return 1;
}

/*ARGSUSED*/
int
fp_uuid(
	void	*obj,
	int	bit,
	int	count,
	char	*fmtstr,
	int	size,
	int	arg,
	int	base,
	int	array)
{
	char	bp[40];	/* UUID string is 36 chars + trailing '\0' */
	int	i;
	uuid_t	*p;

	ASSERT(bitoffs(bit) == 0);
	for (p = (uuid_t *)((char *)obj + byteize(bit)), i = 0;
	     i < count && !seenint();
	     i++, p++) {
		if (array)
			dbprintf("%d:", i + base);
		platform_uuid_unparse(p, bp);
		dbprintf("%s", bp);
		if (i < count - 1)
			dbprintf(" ");
	}
	return 1;
}
