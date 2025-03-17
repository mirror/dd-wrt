/*
 * safememory.c
 *
 * Copyright (C) 2005 - 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <stdlib.h>
#include <shutils.h>
#include <utils.h>
#include <stdarg.h>

#define MAGIC 0xcafecafe
#undef dd_malloc

typedef struct memblock {
	size_t len;
	unsigned int magic;
	unsigned int data[0];
} memblock_t;

#define GETMAGIC(mem) (memblock_t *)((char *)mem - sizeof(memblock_t))
static void fillmagic(memblock_t *mem, size_t len)
{
	mem->len = len;
	mem->magic = MAGIC;
	mem->data[len] = MAGIC;
}

static void checktainted(void *src)
{
	memblock_t *mem = GETMAGIC(src);
	if (mem->magic != MAGIC) {
		dd_loginfo("safememory", "found bad allocated memory %p, magic was 0x%08X\n", src, mem->magic);
		return;
	}
	if (mem->data[mem->len] != MAGIC) {
		dd_loginfo("safememory", "heap corruption detected at %p, magic was 0x%08X\n", src, mem->data[mem->len]);
		return;
	}
}
void *dd_malloc(size_t len)
{
	memblock_t *mem;
	mem = malloc(len + sizeof(memblock_t) + sizeof(unsigned int));
	fillmagic(mem, len);
	return mem->data;
}

void dd_free(void *src)
{
	checktainted(src);
	free(GETMAGIC(src));
}

void *dd_realloc(void *src, size_t len)
{
	checktainted(src);
	memblock_t *mem = realloc(GETMAGIC(src), len + sizeof(memblock_t) + sizeof(unsigned int));
	if (!mem) {
		dd_loginfo("safememorx", "cannot allocate %ld bytes\n", len);
		return mem;
	}
	fillmagic(mem, len);
	return mem->data;
}

char *dd_strdup(const char *str)
{
	size_t l = strlen(str);
	char *d = dd_malloc(l + 1);
	if (!d)
		return NULL;
	fillmagic(GETMAGIC(d), l + 1);
	return memcpy(d, str, l + 1);
}

char *dd_strcpy(char *dst, const char *src)
{
	memblock_t *mem = GETMAGIC(dst);
	if (mem->magic == MAGIC) {
		checktainted(dst);
		strlcpy(dst, src, mem->len);
		return dst;
	} else {
		return strcpy(dst, src);
	}
}

size_t dd_strlcpy(char *dst, const char *src, size_t len)
{
	memblock_t *mem = GETMAGIC(dst);
	if (mem->magic == MAGIC) {
		checktainted(dst);
		if ((mem->len - 1) < len)
			dd_loginfo("safememory", "overflow detected in %s, target size is %ld, wanted %ld\n", __func__, mem->len,
				   len + 1);
		return strlcpy(dst, src, len);
	} else {
		return strlcpy(dst, src, len);
	}
}

char *dd_strcat(char *dst, const char *src)
{
	memblock_t *mem = GETMAGIC(dst);
	if (mem->magic == MAGIC) {
		checktainted(dst);
		strlcat(dst, src, mem->len);
		return dst;
	} else {
		return strcat(dst, src);
	}
}

size_t dd_strlcat(char *dst, const char *src, size_t len)
{
	memblock_t *mem = GETMAGIC(dst);
	if (mem->magic == MAGIC) {
		checktainted(dst);
		if ((mem->len - 1) < len)
			dd_loginfo("safememory", "overflow detected in %s, target size is %ld, wanted %ld\n", __func__, mem->len,
				   len + 1);
		return strlcat(dst, src, mem->len);
	} else {
		return strlcat(dst, src, len);
	}
}

char *dd_strncat(char *dst, const char *src, size_t len)
{
	memblock_t *mem = GETMAGIC(dst);
	if (mem->magic == MAGIC) {
		checktainted(dst);
		dd_loginfo("safememory", "overflow detected in %s, target size is %ld, wanted %ld\n", __func__, mem->len, len + 1);
		strlcat(dst, src, mem->len);
		return dst;
	} else {
		strlcat(dst, src, len);
		return dst;
	}
}

char *dd_strncpy(char *dst, const char *src, size_t len)
{
	memblock_t *mem = GETMAGIC(dst);
	if (mem->magic == MAGIC) {
		checktainted(dst);
		if ((mem->len - 1) < len)
			dd_loginfo("safememory", "overflow detected in %s, target size is %ld, wanted %ld\n", __func__, mem->len,
				   len + 1);
		strlcpy(dst, src, len);
		return dst;
	} else {
		return strncpy(dst, src, len);
	}
}

int dd_sprintf(char *str, const char *fmt, ...)
{
	va_list ap;
	int n;
	char *dest;
	if (!str)
		return 0;
	va_start(ap, fmt);
	n = vasprintf(&dest, fmt, ap);
	va_end(ap);
	dd_strcpy(str, dest);
	free(dest);
	return n;
}

int dd_snprintf(char *str, size_t len, const char *fmt, ...)
{
	va_list ap;
	int n;
	char *dest;
	if (len < 1)
		return 0;
	va_start(ap, fmt);
	n = vasprintf(&dest, fmt, ap);
	va_end(ap);
	dd_strlcpy(str, dest, len);
	free(dest);
	return n;
}
