/*====================================================================*
 *
 *   size_t b64dump (void const * memory, size_t extent, size_t column, FILE *fp);
 *
 *   base64.h
 *
 *   base64 encode a memory region and write to a text file; wrap
 *   the output at a given column; do not wrap when column is 0;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef B64DUMP_SOURCE
#define B64DUMP_SOURCE

#include <stdio.h>
#include <stdint.h>

#include "../tools/base64.h"
#include "../tools/types.h"

void b64dump (void const * memory, size_t extent, size_t column, FILE *fp)

{
	byte * offset = (byte *)(memory);
	unsigned encode = 0;
	while (extent)
	{
		uint32_t word = 0;
		unsigned byte = 0;
		unsigned bits = BASE64_WORDSIZE - BASE64_BYTESIZE;
		while ((bits) && (extent))
		{
			bits -= BASE64_BYTESIZE;
			word |= *offset << bits;
			offset++;
			extent--;
			byte++;
		}
		if (byte++)
		{
			bits = BASE64_WORDSIZE - BASE64_BYTESIZE;
			while ((bits) && (byte))
			{
				bits -= BASE64_CHARSIZE;
				putc (BASE64_CHARSET [(word >> bits) & BASE64_CHARMASK], fp);
				byte--;
				encode++;
			}
			while (bits)
			{
				bits -= BASE64_CHARSIZE;
				putc ('=', fp);
				encode++;
			}
			if ((column) && !(encode%column))
			{
				putc ('\n', fp);
			}
		}
	}
	return;
}


#endif

