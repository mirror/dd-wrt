/*====================================================================*
 *
 *   size_t ipv4spec (char const *string,  void * memory);
 *
 *   memory.h
 *
 *   encode a 4-byte memory region with the equivalent of an IPv4
 *   dotted decimal string; all field delimiters must be present
 *   but individual fields may have leading zeros or be empty;
 *
 *      0.0.0.0		 0x00, 0x00, 0x00, 0x00
 *      127...1		 0x7F, 0x00, 0x00, 0x01
 *      192.168.099.000  0xC0, 0xA8, 0x63, 0x00
 *
 *.  released 2005 by charles maier associates ltd. for public use;
 *:  compiled on debian gnu/linux with gcc 2.95 compiler;
 *;  licensed under the gnu public license version two;
 *
 *--------------------------------------------------------------------*/

#ifndef IPV4SPEC_SOURCE
#define IPV4SPEC_SOURCE

#include <ctype.h>

#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"

size_t ipv4spec (char const * string, void * memory)

{
	char const * number = string;
	byte * origin = (byte *)(memory);
	byte * offset = (byte *)(memory);
	byte * extent = offset + IPv4_LEN;
	unsigned radix = RADIX_DEC;
	unsigned digit = 0;
	while ((*number) && (offset < extent))
	{
		unsigned value = 0;
		if (offset > origin)
		{
			if (*number == DEC_EXTENDER)
			{
				number++;
			}
		}
		while ((digit = todigit (*number)) < radix)
		{
			value *= radix;
			value += digit;
			if (value >> 8)
			{
				error (1, ERANGE, "IPv4 '%s' octet %d exceeds 8 bits", string, (unsigned)(offset - origin) + 1);
			}
			number++;
		}
		*offset++ = value;
	}

#if defined (WIN32)

	while (isspace (*number))
	{
		number++;
	}

#endif

	if (offset < extent)
	{
		error (1, EINVAL, "IPv4 '%s' has only %d octets", string, (unsigned)(offset - origin));
	}
	if (*number)
	{
		error (1, EINVAL, "IPv4 '%s' contains trash '%s'", string, number);
	}
	return (offset - origin);
}


/*====================================================================*
 *   demo/test program;
 *--------------------------------------------------------------------*/

#if 0

#include <stdio.h>

char const * program_name = "ipv4spec";
int main (int argc, char * argv [])

{
	byte memory [4];
	char string [16];
	while (*++argv)
	{
		ipv4spec (* argv, memory);
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

