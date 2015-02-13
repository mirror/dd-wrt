/*====================================================================*
 *
 *   size_t hexencode (void * memory, size_t extent, char const * string);
 *
 *   memory.h
 *
 *   encode a hexadecimal string into a fixed length memory region;
 *   return the number of bytes encoded or 0 on error; an error will
 *   occur of the entire region cannot be encoded or the entire
 *   string cannot be converted due to illegal or excessive digits;
 *
 *   permit an optional HEX_EXTENDER character between successive
 *   octets; constant character HEX_EXTENDER is defined in number.h;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef HEXENCODE_SOURCE
#define HEXENCODE_SOURCE

#include <errno.h>
#include <ctype.h>

#include "../tools/memory.h"
#include "../tools/number.h"

size_t hexencode (void * memory, register size_t extent, register char const * string)

{
	register byte * origin = (byte *)(memory);
	register byte * offset = (byte *)(memory);
	unsigned radix = RADIX_HEX;
	unsigned digit = 0;
	while ((extent) && (*string))
	{
		unsigned field = HEX_DIGITS;
		unsigned value = 0;
		if ((offset > origin) && (*string == HEX_EXTENDER))
		{
			string++;
		}
		while (field--)
		{
			if ((digit = todigit (*string)) < radix)
			{
				value *= radix;
				value += digit;
				string++;
				continue;
			}
			errno = EINVAL;
			return (0);
		}
		*offset = value;
		offset++;
		extent--;
	}

#if defined (WIN32)

	while (isspace (*string))
	{
		string++;
	}

#endif

	if ((extent) || (*string))
	{
		errno = EINVAL;
		return (0);
	}
	return (offset - origin);
}


#endif

