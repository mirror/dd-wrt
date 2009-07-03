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

void *memmove( void *s1, const void *s2, size_t n )
{
    char *dst = (char *)s1;
    const char *src = (const char *)s2;
    if ((src < dst) && (dst < (src + n)))
    {
        // Have to copy backwards
        src += n;
        dst += n;
        while (n--)
        {
            *--dst = *--src;
        }
    }
    else
    {
        while (n--)
        {
            *dst++ = *src++;
        }
    }

    
    return s1;
} // __memmove()

