/*====================================================================*
 *
 *   uint64_t basespec (char const * string, unsigned base, unsigned size);
 *
 *   number.h
 *
 *   convert a character string to an equivalent unsigned integer and
 *   return the result; terminate the program on failure;
 *
 *   the base argument is the number base to be used for conversion;
 *   base 0 permits the number base to be determined by the string
 *   string prefix; 0b, 0d or 0x for binary, decimal or hex;
 *
 *   this implementation accepts a minus sign in order to negate any
 *   number in any base;
 *
 *   the size argument is the maximum number of bytes permitted in the
 *   result;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef BASESPEC_SOURCE
#define BASESPEC_SOURCE

#include <stdlib.h>
#include <ctype.h>

#include "../tools/number.h"
#include "../tools/error.h"

uint64_t basespec (char const * string, unsigned base, unsigned size)

{
	char const * number = string;
	unsigned radix = RADIX_DEC;
	signed scale = 1;
	uint64_t limit = 0;
	uint64_t value = 0;
	unsigned digit = 0;
	limit = ~limit;
	if (size < sizeof (limit))
	{
		limit <<= size << 3;
		limit = ~limit;
	}
	if (base)
	{
		radix = base;
	}
	if (* number == '=')
	{
		number++;
	}
	else if (* number == '+')
	{
		number++;
	}
	else if (* number == '-')
	{
		number++;
		scale = -1;
	}
	if (*number == '0')
	{
		number++;
		if ((*number == 'b') || (*number == 'B'))
		{
			radix = RADIX_BIN;
			number++;
		}
		else if ((*number == 'd') || (*number == 'D'))
		{
			radix = RADIX_DEC;
			number++;
		}
		else if ((*number == 'x') || (*number == 'X'))
		{
			radix = RADIX_HEX;
			number++;
		}
	}
	if ((base) && (base != radix))
	{
		error (1, EINVAL, "%s is not base %d notation", string, base);
	}
	while ((digit = todigit (*number)) < radix)
	{
		value *= radix;
		value += digit;
		if (value > limit)
		{
			error (1, ERANGE, "%s exceeds %d bits", string, (size << 3));
		}
		number++;
	}

#ifdef WIN32

	while (isspace (*number))
	{
		number++;
	}

#endif

	if (*number)
	{
		error (1, EINVAL, "%s is not base %d notation", string, radix);
	}
	return (scale * value);
}


#endif

