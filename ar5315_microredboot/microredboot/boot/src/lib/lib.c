/*
 * lib.c
 * copyright 2009 Sebastian Gottschall / NewMedia-NET GmbH / DD-WRT.COM
 * licensed under GPL conditions
 * this code is based on linux sources
 */
#include <linux/kernel.h>

#include <asm/uaccess.h>

char *strchr(const char *s, int c)
{
	for (; *s != (char)c; ++s)
		if (*s == '\0')
			return NULL;
	return (char *)s;
}

size_t strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */ ;
	return sc - s;
}
