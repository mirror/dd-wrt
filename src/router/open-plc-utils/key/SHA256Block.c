/*====================================================================*
 *
 *   void SHA256Block (struct sha256 * sha256, void const * memory);
 *
 *   SHA256.h
 *
 *   encode a fixed-length block of memory into the hash buffer and
 *   update the hash state; the length is implicit in the algorithm
 *   and need not be specified as an argument;
 *
 *   Read standard FIPS180-2 sec 5.3.2 for an explanation;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef SHA256MERGE_SOURCE
#define SHA256MERGE_SOURCE

#include <stdio.h>

#include "../key/SHA256.h"

#define  SHR(word,bits) ((word & 0xFFFFFFFF) >> bits)
#define ROTR(word,bits) (SHR(word,bits) | (word << (32 - bits)))

void SHA256Block (struct sha256 * sha256, void const * memory)

{
	static const uint32_t K [sizeof (sha256->block)] =
	{
		0x428A2F98,
		0x71374491,
		0xB5C0FBCF,
		0xE9B5DBA5,
		0x3956C25B,
		0x59F111F1,
		0x923F82A4,
		0xAB1C5ED5,
		0xD807AA98,
		0x12835B01,
		0x243185BE,
		0x550C7DC3,
		0x72BE5D74,
		0x80DEB1FE,
		0x9BDC06A7,
		0xC19BF174,
		0xE49B69C1,
		0xEFBE4786,
		0x0FC19DC6,
		0x240CA1CC,
		0x2DE92C6F,
		0x4A7484AA,
		0x5CB0A9DC,
		0x76F988DA,
		0x983E5152,
		0xA831C66D,
		0xB00327C8,
		0xBF597FC7,
		0xC6E00BF3,
		0xD5A79147,
		0x06CA6351,
		0x14292967,
		0x27B70A85,
		0x2E1B2138,
		0x4D2C6DFC,
		0x53380D13,
		0x650A7354,
		0x766A0ABB,
		0x81C2C92E,
		0x92722C85,
		0xA2BFE8A1,
		0xA81A664B,
		0xC24B8B70,
		0xC76C51A3,
		0xD192E819,
		0xD6990624,
		0xF40E3585,
		0x106AA070,
		0x19A4C116,
		0x1E376C08,
		0x2748774C,
		0x34B0BCB5,
		0x391C0CB3,
		0x4ED8AA4A,
		0x5B9CCA4F,
		0x682E6FF3,
		0x748F82EE,
		0x78A5636F,
		0x84C87814,
		0x8CC70208,
		0x90BEFFFA,
		0xA4506CEB,
		0xBEF9A3F7,
		0xC67178F2
	};
	unsigned pass;
	unsigned word;
	uint32_t H [sizeof (sha256->state)/sizeof (uint32_t)];
	uint32_t W [sizeof (sha256->block)];
	uint8_t * buffer = (uint8_t *)(memory);
	for (word = 0; word < 16; word++)
	{
		W [word] = 0;
		W [word] |= (uint32_t)(*buffer++) << 24;
		W [word] |= (uint32_t)(*buffer++) << 16;
		W [word] |= (uint32_t)(*buffer++) << 8;
		W [word] |= (uint32_t)(*buffer++) << 0;;
	}
	for ( ; word < sizeof (sha256->block); word++)
	{
		uint32_t s0 = ROTR (W [word-15], 7) ^ ROTR (W [word-15], 18) ^ SHR (W [word-15], 3);
		uint32_t s1 = ROTR (W [word- 2], 17) ^ ROTR (W [word- 2], 19) ^ SHR (W [word- 2], 10);
		W [word] = W [word - 16] + s0 + W [word - 7] + s1;
	}
	for (word = 0; word < (sizeof (sha256->state) / sizeof (uint32_t)); word++)
	{
		H [word] = sha256->state [word];
	}
	for (pass = 0; pass < sizeof (sha256->block); pass++)
	{
		uint32_t s2 = ROTR (H [0], 2) ^ ROTR (H [0], 13) ^ ROTR (H [0], 22);
		uint32_t maj = (H [0] & H [1]) ^ (H [0] & H [2]) ^ (H [1] & H [2]);
		uint32_t t2 = s2 + maj;
		uint32_t s3 = ROTR (H [4], 6) ^ ROTR (H [4], 11) ^ ROTR (H [4], 25);
		uint32_t ch = (H [4] & H [5]) ^ ((~ H [4]) & H [6]);
		uint32_t t1 = H [7] + s3 + ch + K [pass] + W [pass];
		for (word = (sizeof (sha256->state) / sizeof (uint32_t)) - 1; word > 0; word--)
		{
			H [word] = H [word-1];
		}
		H [0] = t1 + t2;
		H [4] += t1;
	}
	for (word = 0; word < (sizeof (sha256->state) / sizeof (uint32_t)); word++)
	{
		sha256->state [word] += H [word];
	}
	return;
}


#endif

