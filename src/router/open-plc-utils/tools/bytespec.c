/*====================================================================*
 *
 *   size_t bytespec (char const *string,  void * memory, size_t extent);
 *
 *   memory.h
 *
 *   encode a memory region with a fixed-length hexadecimal string;
 *   return the number of bytes encoded or terminate the program on
 *   error;
 *
 *   the number of octets in string must equal the memory extent or
 *   an error will occur; octets may be seperated by colons; empty
 *   octets are illegal;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef BYTESPEC_SOURCE
#define BYTESPEC_SOURCE

#include <ctype.h>
#include <errno.h>

#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"

size_t bytespec (char const * string, void * memory, size_t extent)

{
	char const * number = string;
	byte * origin = (byte *)(memory);
	byte * offset = (byte *)(memory);
	if (!number)
	{
		error (1, EINVAL, "bytespec");
	}
	while (isspace (*number))
	{
		number++;
	}
	while ((*number) && (extent))
	{
		unsigned digit;
		if ((offset > origin) && (*number == HEX_EXTENDER))
		{
			number++;
		}
		if ((digit = todigit (*number++)) >= RADIX_HEX)
		{
			error (1, EINVAL, "You said '%s' but I want a hex digit", string);
		}
		*offset = digit << 4;
		if ((digit = todigit (*number++)) >= RADIX_HEX)
		{
			error (1, EINVAL, "You said '%s' but I want a hex digit", string);
		}
		*offset |= digit;
		offset++;
		extent--;
	}
	while (isspace (*number))
	{
		number++;
	}
	if ((*number) || (extent))
	{
		error (1, EINVAL, "%s is not %d bytes", string, (unsigned)(offset - origin + extent));
	}
	return (offset - origin);
}


#endif

