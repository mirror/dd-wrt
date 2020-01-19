/*
 *  copied from linux/lib/string.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/module.h>
#include <linux/string.h>

#undef memcmp
__visible int memcmp(const void *cs, const void *ct, size_t count)
{
	register const unsigned char *r1 = (const unsigned char *) cs;
	register const unsigned char *r2 = (const unsigned char *) ct;

	int r = 0;
	while (count-- && ((r = ((int)(*r1++)) - *r2++) == 0));

	return r;
}
EXPORT_SYMBOL(memcmp);

