/*
 * This header was initially copied in fall 2011 from util-linux.
 */

/*
 * General memory allocation wrappers for malloc, realloc, calloc,
 * strdup, strndup.
 */

#ifndef PROCPS_NG_XALLOC_H
#define PROCPS_NG_XALLOC_H

#include <stdlib.h>
#include <string.h>

#include "c.h"

#ifndef XALLOC_EXIT_CODE
# define XALLOC_EXIT_CODE EXIT_FAILURE
#endif

static inline __ul_alloc_size(1)
void *xmalloc(const size_t size)
{
	void *ret = malloc(size);
	if (!ret && size)
		errx(XALLOC_EXIT_CODE, "cannot allocate %zu bytes", size);
	return ret;
}

static inline __ul_alloc_size(2)
void *xrealloc(void *ptr, const size_t size)
{
	void *ret = realloc(ptr, size);
	if (!ret && size)
		errx(XALLOC_EXIT_CODE, "cannot allocate %zu bytes", size);
	return ret;
}

static inline __ul_calloc_size(1, 2)
void *xcalloc(const size_t nelems, const size_t size)
{
	void *ret = calloc(nelems, size);
	if (!ret && size && nelems)
		errx(XALLOC_EXIT_CODE, "cannot allocate %zu bytes", nelems*size);
	return ret;
}

static inline char *xstrdup(const char *str)
{
	char *ret;
	if (!str)
		return NULL;
	ret = strdup(str);
	if (!ret)
		errx(XALLOC_EXIT_CODE, "cannot duplicate string");
	return ret;
}

static inline char *xstrndup(const char *s, size_t n)
{
	if (! s)
		return NULL;
	char *ret = strndup(s, n);
	if (! ret)
		errx(XALLOC_EXIT_CODE, "cannot duplicate string");
	return ret;
}

#endif /* PROCPS_NG_XALLOC_H */
