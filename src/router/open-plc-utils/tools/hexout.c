/*====================================================================*
 *
 *   void hexout (void const * memory, size_t extent, char c, char e, FILE * fp);
 *
 *   memory.h
 *
 *   print memory as a series of hexadecimal octets seperated by
 *   character c; normally, character c will be HEX_EXTENDER as
 *   defined in number.h;
 *
 *   for example, hexout (memory, 6, ':', ';', stdout) would print:
 *
 *      00:B0:52:00:00:01;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef HEXOUT_SOURCE
#define HEXOUT_SOURCE

#include <stdio.h>
#include <ctype.h>

#include "../tools/memory.h"
#include "../tools/number.h"

void hexout (void const * memory, size_t extent, char c, char e, FILE * fp)

{
	byte * offset = (byte *)(memory);
	while (extent--)
	{
		putc (DIGITS_HEX [(* offset >> 4) & 0x0F], fp);
		putc (DIGITS_HEX [(* offset >> 0) & 0x0F], fp);
		if ((extent) && (c))
		{
			putc (c, fp);
		}
		offset++;
	}
	if (e)
	{
		putc (e, fp);
	}
	return;
}


#endif

