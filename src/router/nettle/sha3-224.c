/* sha3-224.c

   The sha3 hash function, 224 bit output.

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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stddef.h>
#include <string.h>

#include "sha3.h"
#include "sha3-internal.h"

#include "nettle-write.h"

void
sha3_224_init (struct sha3_224_ctx *ctx)
{
  memset (ctx, 0, offsetof (struct sha3_224_ctx, block));
}

void
sha3_224_update (struct sha3_224_ctx *ctx,
		 size_t length,
		 const uint8_t *data)
{
  ctx->index = _nettle_sha3_update (&ctx->state,
				    SHA3_224_BLOCK_SIZE, ctx->block,
				    ctx->index, length, data);
}

void
sha3_224_digest(struct sha3_224_ctx *ctx,
		size_t length,
		uint8_t *digest)
{
  _sha3_pad_hash (&ctx->state, SHA3_224_BLOCK_SIZE, ctx->block, ctx->index);
  _nettle_write_le64 (length, digest, ctx->state.a);
  sha3_224_init (ctx);
}
