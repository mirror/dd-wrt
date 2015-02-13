/*====================================================================*
 *
 *   void hexdump (void const * memory, size_t offset, size_t extent, FILE * fp);
 *
 *   memory.h
 *
 *   print memory as a hexadecimal dump showing relative offsets and
 *   an ASCII character display; this function is similar to but not
 *   the same as function hexview;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef HEXDUMP_SOURCE
#define HEXDUMP_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "../tools/memory.h"
#include "../tools/number.h"

void hexdump (void const * memory, size_t offset, size_t extent, FILE * fp)

{
	byte * origin = (byte *)(memory);
	unsigned block = 0x10;
	size_t lower = block * (offset / block);
	size_t upper = block + lower;
	size_t index = 0;
	char buffer [ADDRSIZE + 72];
	char * output;
	while (lower < extent)
	{
		output = buffer + ADDRSIZE;
		for (index = lower; output-- > buffer; index >>= 4)
		{
			*output = DIGITS_HEX [index & 0x0F];
		}
		output = buffer + ADDRSIZE;
		for (index = lower; index < upper; index++)
		{
			*output++ = ' ';
			if (index < offset)
			{
				*output++ = ' ';
				*output++ = ' ';
			}
			else if (index < extent)
			{
				*output++ = DIGITS_HEX [(origin [index] >> 4) & 0x0F];
				*output++ = DIGITS_HEX [(origin [index] >> 0) & 0x0F];
			}
			else
			{
				*output++ = ' ';
				*output++ = ' ';
			}
		}
		*output++ = ' ';
		for (index = lower; index < upper; index++)
		{
			if (index < offset)
			{
				*output++ = ' ';
			}
			else if (index < extent)
			{
				unsigned c = origin [index];
				*output++ = isprint (c)? c: '.';
			}
			else
			{
				*output++ = ' ';
			}
		}
		*output++ = '\n';
		*output++ = '\0';
		fputs (buffer, fp);
		lower += block;
		upper += block;
	}
	if (extent)
	{
		output = buffer;
		*output++ = '\n';
		*output++ = '\0';
		fputs (buffer, fp);
	}
	fflush (fp);
	return;
}


#endif

