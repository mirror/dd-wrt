/*====================================================================*
 *
 *   void SHA256Write (struct sha256 * sha256, void const * memory, size_t extent);
 *
 *   SHA256.h
 *
 *   write a block of data to an SHA256 digest; this function behaves
 *   like function write() but returns no value and does not fail; an
 *   unlimited amount of data may be written using successive writes;
 *
 *   to start a new digest, use function SHA266Reset(); to read the
 *   digest, use function SHA256Fetch();
 *
 *   Read standard FIPS180-2 sec 5.3.2 for an explanation;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef SHA256WRITE_SOURCE
#define SHA256WRITE_SOURCE

#include "../key/SHA256.h"

void SHA256Write (struct sha256 * sha256, void const * memory, size_t extent)

{
	if (extent)
	{
		uint8_t * buffer = (uint8_t *)(memory);
		unsigned left = sha256->count [0] & 0x3F;
		unsigned fill = sizeof (sha256->block) - left;
		sha256->count [0] += (uint32_t)(extent);
		sha256->count [0] &= 0xFFFFFFFF;
		if (sha256->count [0] < extent)
		{
			sha256->count [1]++;
		}
		if ((left) && (extent >= fill))
		{
			memcpy (sha256->block + left, buffer, fill);
			SHA256Block (sha256, sha256->block);
			extent -= fill;
			buffer += fill;
			left = 0;
		}
		while (extent >= sizeof (sha256->block))
		{
			SHA256Block (sha256, buffer);
			extent -= sizeof (sha256->block);
			buffer += sizeof (sha256->block);
		}
		if (extent)
		{
			memcpy (sha256->block + left, buffer, extent);
		}
	}
	return;
}


#endif

