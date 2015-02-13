/*====================================================================*
 *
 *   void clr32bitmap (uint32_t * map, unsigned bit);
 *
 *   flags.h
 *
 *   clear the given bit in an 32-bit flagword array; macro _bits ()
 *   is defined in flags.h;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef CLR32BITMAP_SOURCE
#define CLR32BITMAP_SOURCE

#include "../tools/flags.h"
#include "../tools/endian.h"

void clr32bitmap (uint32_t * map, unsigned bit)

{
	map [bit / _bits (* map)] &= ~ HTOLE32 (1 << (bit % _bits (* map)));
	return;
}


#endif

