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
#include "strvec.h"
#include "output.h"
#include "malloc.h"

static int	count_strvec(char **vec);

void
add_strvec(
	char	***vecp,
	char	*str)
{
	char	*dup;
	int	i;
	char	**vec;

	dup = xstrdup(str);
	vec = *vecp;
	i = count_strvec(vec);
	vec = xrealloc(vec, sizeof(*vec) * (i + 2));
	vec[i] = dup;
	vec[i + 1] = NULL;
	*vecp = vec;
}

char **
copy_strvec(
	char	**vec)
{
	int	i;
	char	**rval;

	i = count_strvec(vec);
	rval = new_strvec(i);
	for (i = 0; vec[i] != NULL; i++)
		rval[i] = xstrdup(vec[i]);
	return rval;
}

static int
count_strvec(
	char	**vec)
{
	int	i;

	for (i = 0; vec[i] != NULL; i++)
		continue;
	return i;
}

void
free_strvec(
	char	**vec)
{
	int	i;

	for (i = 0; vec[i] != NULL; i++)
		xfree(vec[i]);
	xfree(vec);
}

char **
new_strvec(
	int	count)
{
	char	**rval;

	rval = xmalloc(sizeof(*rval) * (count + 1));
	rval[count] = NULL;
	return rval;
}

void
print_strvec(
	char	**vec)
{
	int	i;

	for (i = 0; vec[i] != NULL; i++)
		dbprintf("%s", vec[i]);
}
