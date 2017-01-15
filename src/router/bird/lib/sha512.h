/*
 *	BIRD Library -- SHA-512 and SHA-384 Hash Functions
 *
 *	(c) 2015 CZ.NIC z.s.p.o.
 *
 *	Based on the code from libgcrypt-1.6.0, which is
 *	(c) 2003, 2006, 2008, 2009 Free Software Foundation, Inc.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_SHA512_H_
#define _BIRD_SHA512_H_

#include "nest/bird.h"


#define SHA384_SIZE 		48
#define SHA384_HEX_SIZE		97
#define SHA384_BLOCK_SIZE	128

#define SHA512_SIZE 		64
#define SHA512_HEX_SIZE		129
#define SHA512_BLOCK_SIZE	128


struct hash_context;

struct sha512_context {
  u64  h0, h1, h2, h3, h4, h5, h6, h7;
  byte buf[SHA512_BLOCK_SIZE];
  uint nblocks;
  uint count;
};

#define sha384_context sha512_context


void sha512_init(struct hash_context *ctx);
void sha384_init(struct hash_context *ctx);

void sha512_update(struct hash_context *ctx, const byte *buf, uint len);
#define sha384_update sha512_update

byte *sha512_final(struct hash_context *ctx);
#define sha384_final sha512_final


#endif /* _BIRD_SHA512_H_ */
