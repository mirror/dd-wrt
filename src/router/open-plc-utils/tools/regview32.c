/*====================================================================*
 *
 *   void regview32 (void const * memory, size_t offset, size_t length, FILE *fp);
 *
 *   memory.h
 *
 *   print memory as a binary dump showing absolute memory offsets
 *   and 32-bit binary register maps; sample output looks like:
 *
 *      REGISTER CONTENTS 31----24 23----16 15----08 07----00
 *
 *      00183000 00000F7F 00000000-00000000-00001111-01111111
 *      00183004 00000A00 00000000-00000000-00001010-00000000
 *      00183008 00000FF8 00000000-00000000-00001111-11111000
 *      0018300C 00000000 00000000-00000000-00000000-00000000
 *      00183010 00000075 00000000-00000000-00000000-01110101
 *
 *   the header line shown above is not part of the output;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef REGVIEW_SOURCE
#define REGVIEW_SOURCE

#include <stdio.h>

#include "../tools/memory.h"
#include "../tools/endian.h"

void regview32 (void const * memory, size_t offset, size_t extent, FILE *fp)

{
	uint32_t * origin = (uint32_t *)(memory);
	while (extent >= sizeof (* origin))
	{
		signed bit = sizeof (* origin) << 3;
		printf ("%08X ", (uint32_t)(offset));
		printf ("%08X ", LE32TOH (* origin));
		while (bit--)
		{
			putc (((LE32TOH (* origin) >> bit) & 1)? '1': '0', fp);
			if ((bit) && !(bit%8))
			{
				putc ('-', fp);
			}
		}
		putc ('\n', fp);
		offset += sizeof (* origin);
		extent -= sizeof (* origin);
		origin++;
	}
	return;
}


#endif

