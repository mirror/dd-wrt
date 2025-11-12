/*
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include <nftables.h>
#include <utils.h>

void __noreturn __memory_allocation_error(const char *filename, uint32_t line)
{
	fprintf(stderr, "%s:%u: Memory allocation failure\n", filename, line);
	exit(NFT_EXIT_NOMEM);
}

void *xmalloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (ptr == NULL)
		memory_allocation_error();
	return ptr;
}

void *xmalloc_array(size_t nmemb, size_t size)
{
	assert(size != 0);
	assert(nmemb != 0);

	if (nmemb > SIZE_MAX / size)
		memory_allocation_error();

	return xmalloc(nmemb * size);
}

void *xzalloc_array(size_t nmemb, size_t size)
{
	void *ptr;

	ptr = xmalloc_array(nmemb, size);
	memset(ptr, 0, nmemb * size);

	return ptr;
}

void *xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (ptr == NULL && size != 0)
		memory_allocation_error();
	return ptr;
}

void *xzalloc(size_t size)
{
	void *ptr;

	ptr = xmalloc(size);
	memset(ptr, 0, size);
	return ptr;
}

char *xstrdup(const char *s)
{
	char *res;

	assert(s != NULL);
	res = strdup(s);
	if (res == NULL)
		memory_allocation_error();
	return res;
}

void xstrunescape(const char *in, char *out)
{
	unsigned int i, k = 0;

	for (i = 0; i < strlen(in); i++) {
		if (in[i] == '\\')
			continue;

		out[k++] = in[i];
	}
	out[k++] = '\0';
}

int round_pow_2(unsigned int n)
{
	return 1UL << fls(n - 1);
}
