/* sha3-internal.h

   The sha3 hash function (aka Keccak).

   Copyright (C) 2012 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#ifndef NETTLE_SHA3_INTERNAL_H_INCLUDED
#define NETTLE_SHA3_INTERNAL_H_INCLUDED

#include "nettle-types.h"

#define SHA3_HASH_MAGIC 6
#define SHA3_SHAKE_MAGIC 0x1f

void
_nettle_sha3_init (struct sha3_ctx *ctx);

/* For all functions, the block_size is in units of uint64_t words. */
void
_nettle_sha3_update (struct sha3_ctx *ctx, unsigned block_size,
		     size_t length, const uint8_t *data);

void
_nettle_sha3_pad (struct sha3_ctx *ctx, unsigned block_size, uint8_t magic);

void
_nettle_sha3_digest (struct sha3_ctx *ctx, unsigned block_size,
		     unsigned digest_size, uint8_t *digest);

void
_nettle_sha3_shake (struct sha3_ctx *ctx, unsigned block_size,
		    size_t length, uint8_t *dst);

void
_nettle_sha3_shake_output (struct sha3_ctx *ctx, unsigned block_size,
			   size_t length, uint8_t *dst);

#define _NETTLE_SHA3_HASH(name, NAME) {		\
 #name,						\
 sizeof(struct sha3_ctx),			\
 NAME##_DIGEST_SIZE,				\
 NAME##_BLOCK_SIZE,				\
 (nettle_hash_init_func *) sha3_init,		\
 (nettle_hash_update_func *) name##_update,	\
 (nettle_hash_digest_func *) name##_digest	\
}

#endif
