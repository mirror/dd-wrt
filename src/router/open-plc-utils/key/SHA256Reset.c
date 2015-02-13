/*====================================================================*
 *
 *   SHA256Reset (struct sha256 * sha256)
 *
 *   SHA256.h
 *
 *   initialize the SHA256 hash state; this effectively erases the
 *   previous hash state;
 *
 *   Read standard FIPS180-2 sec 5.3.2 for an explanation;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef SHA256RESET_SOURCE
#define SHA256RESET_SOURCE

#include "../key/SHA256.h"

void SHA256Reset (struct sha256 * sha256)

{
	memset (sha256, 0, sizeof (struct sha256));
	sha256->state [0] = 0x6A09E667;
	sha256->state [1] = 0xBB67AE85;
	sha256->state [2] = 0x3C6EF372;
	sha256->state [3] = 0xA54FF53A;
	sha256->state [4] = 0x510E527F;
	sha256->state [5] = 0x9B05688C;
	sha256->state [6] = 0x1F83D9AB;
	sha256->state [7] = 0x5BE0CD19;
	sha256->extra [0] = 0x80;
	return;
}


#endif

