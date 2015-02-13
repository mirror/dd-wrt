/*====================================================================*
 *
 *   char * decstring ( char buffer [], size_t length, void const * memory, size_t extent);
 *
 *   memory.h
 *
 *   decode a memory region into a decimal character buffer and
 *   return the buffer address;
 *
 *   allow four string characters for each memory byte; this means
 *   that the buffer must hold at least four characters or nothing
 *   will be decoded;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef DECSTRING_SOURCE
#define DECSTRING_SOURCE

#include <stdint.h>

#include "../tools/memory.h"

char * decstring (char buffer [], size_t length, void const * memory, size_t extent)

{
	decdecode (memory, extent, buffer, length);
	return ((char *)(buffer));
}


#endif

