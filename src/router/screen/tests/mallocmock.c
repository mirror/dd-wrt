/* Copyright (c) 2013
 *      Mike Gerwitz (mtg@gnu.org)
 *
 * This file is part of GNU screen.
 *
 * GNU screen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * <http://www.gnu.org/licenses>.
 *
 ****************************************************************
 */

#include <stdlib.h>
#include <stdbool.h>

extern void *__libc_malloc(size_t);
extern void *__libc_realloc(void *, size_t);

/* Total number of bytes requested via *alloc */
size_t _mallocmock_malloc_size = 0;
size_t _mallocmock_realloc_size = 0;

/* Cause calls to return NULL */
bool _mallocmock_fail = false;


#ifdef _GNU_SOURCE
/* glibc declares this as a weak symbol, so we can override it */
void *malloc(size_t size)
{
	if (_mallocmock_fail)
		return NULL;

	_mallocmock_malloc_size += size;
	return __libc_malloc(size);
}

void *realloc(void *ptr, size_t size)
{
	if (_mallocmock_fail)
		return NULL;

	_mallocmock_realloc_size += size;
	return __libc_realloc(ptr, size);
}

void mallocmock_reset()
{
	_mallocmock_malloc_size = 0;
	_mallocmock_realloc_size = 0;
}
#endif
