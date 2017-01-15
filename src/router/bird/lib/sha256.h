/*
 *	BIRD Library -- SHA-256 and SHA-224 Hash Functions
 *
 *	(c) 2015 CZ.NIC z.s.p.o.
 *
 *	Based on the code from libgcrypt-1.6.0, which is
 *	(c) 2003, 2006, 2008, 2009 Free Software Foundation, Inc.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_SHA256_H_
#define _BIRD_SHA256_H_

#include "nest/bird.h"


#define SHA224_SIZE 		28
#define SHA224_HEX_SIZE		57
#define SHA224_BLOCK_SIZE 	64

#define SHA256_SIZE 		32
#define SHA256_HEX_SIZE		65
#define SHA256_BLOCK_SIZE 	64


struct hash_context;

struct sha256_context {
  u32  h0, h1, h2, h3, h4, h5, h6, h7;
  byte buf[SHA256_BLOCK_SIZE];
  uint nblocks;
  uint count;
};

#define sha224_context sha256_context


void sha256_init(struct hash_context *ctx);
void sha224_init(struct hash_context *ctx);

void sha256_update(struct hash_context *ctx, const byte *buf, uint len);
#define sha224_update sha256_update

byte *sha256_final(struct hash_context *ctx);
#define sha224_final sha256_final


#endif /* _BIRD_SHA256_H_ */
