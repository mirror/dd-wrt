/*====================================================================*
 *
 *   size_t decdecode (void const * memory, size_t extent,  char buffer [], size_t length);
 *
 *   memory.h
 *
 *   decode a memory block of given length in bytes as a string of
 *   separated hexadecimal bytes; terminate once the string fills
 *   or the memory ends; terminate the string and return the actual
 *   string bytes;
 *
 *   allow three string characters for each memory byte; this means
 *   that the buffer must have at least three characters or nothing
 *   will be decoded; the maximum number of bytes is the lesser of
 *   chars/3 and bytes;;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef DECDECODE_SOURCE
#define DECDECODE_SOURCE

#include "../tools/memory.h"
#include "../tools/number.h"

size_t decdecode (void const * memory, register size_t extent, char buffer [], register size_t length)

{
	register char * string = (char *)(buffer);
	register byte * offset = (byte *)(memory);
	if (length)
	{
		length /= DEC_DIGITS + 1;
		while ((length--) && (extent--))
		{
			unsigned octet = *offset;
			unsigned digit = DEC_DIGITS;
			while (digit--)
			{
				string [digit] = '0' + octet % RADIX_DEC;
				octet /= RADIX_DEC;
			}
			string += DEC_DIGITS;
			if ((length) && (extent))
			{
				*string++ = DEC_EXTENDER;
			}
			offset++;
		}
		*string = (char) (0);
	}
	return (string - buffer);
}


#endif

