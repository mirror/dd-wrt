/*====================================================================*
 *
 *   void SHA256Fetch (struct sha256 * sha256,  uint8_t digest []);
 *
 *   SHA256.h
 *
 *   read the SHA256 digest; this function works like function read()
 *   but the function returns no value and the digest buffer is fixed
 *   length;
 *
 *   to start a digest, use function SHA256Reset(); to write data to
 *   the digest use function SHA256Write();
 *
 *   Read standard FIPS180-2 sec 5.3.2 for an explanation;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef SHA256FETCH_SOURCE
#define SHA256FETCH_SOURCE

#include "../key/SHA256.h"

static void encode (uint8_t memory [], uint32_t number)

{
	*memory++ = (uint8_t)(number >> 24);
	*memory++ = (uint8_t)(number >> 16);
	*memory++ = (uint8_t)(number >> 8);
	*memory++ = (uint8_t)(number >> 0);
	return;
}

void SHA256Fetch (struct sha256 * sha256, uint8_t digest [])

{
	unsigned word;
	uint8_t bits [8];
	uint32_t upper = (sha256->count [0] >> 29) | (sha256->count [1] << 3);
	uint32_t lower = (sha256->count [0] << 3);
	uint32_t final = (sha256->count [0] & 0x3F);
	uint32_t extra = (final < 56)? (56 - final): (120 - final);
	encode (&bits [0], upper);
	encode (&bits [4], lower);
	SHA256Write (sha256, sha256->extra, extra);
	SHA256Write (sha256, bits, sizeof (bits));
	for (word = 0; word < sizeof (sha256->state) / sizeof (uint32_t); word++)
	{
		encode (digest, sha256->state [word]);
		digest += sizeof (uint32_t);
	}
	memset (sha256, 0, sizeof (struct sha256));
	return;
}


#endif

