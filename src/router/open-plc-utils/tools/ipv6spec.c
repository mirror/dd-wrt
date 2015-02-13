/*====================================================================*
 *
 *   size_t ipv6spec (char const * string,  byte memory []);
 *
 *   memory.h
 *
 *   encode a 16-byte memory region with the binary equivalent of an
 *   ipv6 address string; ipv6 addresses are defined as 8 16-bit hex
 *   numbers separated with colons; two consecutive colons represent
 *   one or more 0000 fields;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef IPV6SPEC_SOURCE
#define IPV6SPEC_SOURCE

#include <memory.h>
#include <ctype.h>
#include <errno.h>

#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"

size_t ipv6spec (char const * string, void * memory)

{
	char const * number = string;
	byte * origin = (byte *)(memory);
	byte * offset = (byte *)(memory);
	byte * extent = offset + IPv6_LEN;
	byte * marker = offset + IPv6_LEN;
	unsigned radix = RADIX_HEX;
	unsigned digit = 0;
	while ((*number) && (offset < extent))
	{
		uint32_t value = 0;
		if (offset > origin)
		{
			if (*number == HEX_EXTENDER)
			{
				number++;
			}
			if (*number == HEX_EXTENDER)
			{
				marker = offset;
			}
		}
		while ((digit = todigit (*number)) < radix)
		{
			value *= radix;
			value += digit;
			if (value >> 16)
			{
				error (1, ERANGE, "IPv6 '%s' field %d exceeds 16 bits", string, 1 + ((unsigned)(offset - origin) >> 1));
			}
			number++;
		}
		*offset++ = (byte)(value >> 8);
		*offset++ = (byte)(value >> 0);
	}

#if defined (WIN32)

	while (isspace (*number))
	{
		number++;
	}

#endif

	if (*number)
	{
		error (1, EINVAL, "IPv6 '%s' includes trash '%s'", string, number);
	}
	if (offset < extent)
	{
		while (offset > marker)
		{
			*--extent = *--offset;
		}
		while (extent > offset)
		{
			*--extent = 0;
		}
	}
	if (offset < marker)
	{
		error (1, EINVAL, "IPv6 '%s' has only %d fields", string, (unsigned)(offset - origin) >> 1);
	}
	return (offset - origin);
}


/*====================================================================*
 *   demo/test program;
 *--------------------------------------------------------------------*/

#if 0
#include <stdio.h>

char const * program_name = "ipv6spec";
int main (int argc, char * argv [])

{
	byte memory [16];
	char string [48];
	while (*++argv)
	{
		ipv6spec (* argv, memory);
		hexdecode (memory, sizeof (memory), string, sizeof (string));
		printf ("%s %s\n", string, * argv);
	}
	return (0);
}


#endif

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

