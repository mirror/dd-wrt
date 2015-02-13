/*====================================================================*
 *
 *   void set32bitmap (uint32_t * map, unsigned bit);
 *
 *   flags.h
 *
 *   set a given bit in a 32-bit flagword array; macro _bits () is
 *   defined in flags.h;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef SET32BITMAP_SOURCE
#define SET32BITMAP_SOURCE

#include "../tools/flags.h"
#include "../tools/endian.h"

void set32bitmap (uint32_t * map, unsigned bit)

{
	map [bit / _bits (* map)] |= HTOLE32 (1 << (bit % _bits (* map)));
	return;
}


#endif

