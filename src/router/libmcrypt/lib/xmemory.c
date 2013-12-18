/*
 * Copyright (C) 1998,1999 Nikos Mavroyanopoulos
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* $Id: xmemory.c,v 1.3 2001/05/17 18:58:20 nmav Exp $ */

#ifndef LIBDEFS_H
# define LIBDEFS_H
# include <libdefs.h>
#endif
#include <bzero.h>

#ifdef HAVE_MLOCK
# ifdef HAVE_SYS_MMAN_H
#  include <sys/mman.h>
# endif
#endif

#ifdef HAVE_MLOCK
void LOCKMEM(void *x, size_t size)
{
	mlock(x, size);
}

# define UNLOCKMEM(x,y) munlock(x,y)
#else
# define LOCKMEM(x,y)
# define UNLOCKMEM(x,y)
#endif

#include <bzero.h>

/* memory allocation */

void *mxmalloc(size_t size)
{
	char *x;

	x = malloc(size);

	if (x != NULL) {
		LOCKMEM(x, size);
		return x;
	}
	return NULL;
}

void *mxcalloc(size_t nmemb, size_t size)
{
	char *x;

	x = calloc(nmemb, size);
	if (x != NULL) {
		LOCKMEM(x, size);
		return x;
	}
	return NULL;
}

void *mxrealloc(void *ptr, size_t size)
{
	char *x;

	x = realloc(ptr, size);
	if (x != NULL) {
		LOCKMEM(x, size);
		return x;
	}
	return NULL;
}


void mxfree(void *ptr, size_t size)
{

	Bzero(ptr, size);
	UNLOCKMEM(ptr, size);
	free(ptr);

}
