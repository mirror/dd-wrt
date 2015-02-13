/*====================================================================*
 *
 *   SHA256.h - SHA256 encryption declarations and definitions;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef SHA256_HEADER
#define SHA256_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdint.h>
#include <string.h>

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define SHA256_DIGEST_LENGTH 256/8

/*====================================================================*
 *   variables;
 *--------------------------------------------------------------------*/

typedef struct sha256

{
	uint32_t count [2];
	uint32_t state [8];
	uint8_t block [64];
	uint8_t extra [64];
}

SHA256;

/*====================================================================*
 *   functions;
 *--------------------------------------------------------------------*/

void SHA256Reset (struct sha256 * sha256);
void SHA256Write (struct sha256 * sha256, void const * memory, size_t extent);
void SHA256Block (struct sha256 * sha256, void const * memory);
void SHA256Fetch (struct sha256 * sha256, uint8_t digest []);
void SHA256Print (const uint8_t digest [], char const * string);
void SHA256Ident (signed fd, uint8_t digest []);
signed SHA256Match (signed fd, const uint8_t digest []);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

