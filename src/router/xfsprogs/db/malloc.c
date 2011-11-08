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
#include "init.h"
#include "malloc.h"
#include "output.h"

static void
badmalloc(void)
{
	dbprintf(_("%s: out of memory\n"), progname);
	exit(4);
}

void *
xcalloc(
	size_t	nelem,
	size_t	elsize)
{
	void	*ptr;

	ptr = calloc(nelem, elsize);
	if (ptr)
		return ptr;
	badmalloc();
	/* NOTREACHED */
	return NULL;
}

void
xfree(
	void	*ptr)
{
	free(ptr);
}

void *
xmalloc(
	size_t	size)
{
	void	*ptr;

	ptr = valloc(size);
	if (ptr)
		return ptr;
	badmalloc();
	/* NOTREACHED */
	return NULL;
}

void *
xrealloc(
	void	*ptr,
	size_t	size)
{
	ptr = realloc(ptr, size);
	if (ptr || !size)
		return ptr;
	badmalloc();
	/* NOTREACHED */
	return NULL;
}

char *
xstrdup(
	const char	*s1)
{
	char		*s;

	s = strdup(s1);
	if (s)
		return s;
	badmalloc();
	/* NOTREACHED */
	return NULL;
}
