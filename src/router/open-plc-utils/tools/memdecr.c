/*====================================================================*
 *
 *   signed memdecr (void * memory, size_t extent);
 *
 *   memory.h
 *
 *   decrement a multi-byte memory region; start at 0xff and reset
 *   at 0x00; return -1 if all bytes are 0x00;
 *
 *   for example:
 *
 *      0x00 0x00 0x00 --> 0x00 0x00 0x01
 *      0xFF 0x00 0xFF --> 0xFF 0x01 0x00
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef MEMDECR_SOURCE
#define MEMDECR_SOURCE

#include "../tools/memory.h"

signed memdecr (void * memory, register size_t extent)

{
	register byte * offset = (byte *)(memory);
	while (extent--)
	{
		if (-- offset [extent] != 0xFF)
		{
			return (0);
		}
	}
	return (-1);
}


#endif

