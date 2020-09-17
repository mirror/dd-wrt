// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
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
