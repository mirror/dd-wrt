/* sha3.c

   The sha3 hash function.

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

#include <string.h>

#include "sha3.h"
#include "sha3-internal.h"

#include "bswap-internal.h"
#include "macros.h"
#include "memxor.h"
#include "nettle-write.h"

void
sha3_init (struct sha3_ctx *ctx)
{
  memset (ctx, 0, offsetof (struct sha3_ctx, block));
}

void
_nettle_sha3_update (struct sha3_ctx *ctx, unsigned block_size,
		     size_t length, const uint8_t *data)
{
#if WORDS_BIGENDIAN
  unsigned byte_index = ctx->index & 7;
  unsigned index = ctx->index >> 3;

  if (byte_index > 0)
    {
      unsigned left = sizeof (ctx->block) - byte_index;
      if (length < left)
	{
	  memcpy (ctx->block.b + byte_index, data, length);
	  ctx->index += length;
	  return;
	}
      memcpy (ctx->block.b + byte_index, data, left);
      data += left; length -= left;

      ctx->state.a[index++] ^= nettle_bswap64 (ctx->block.u64);
      if (index == block_size)
	{
	  sha3_permute (&ctx->state);
	  index = 0;
	}
    }

  for (; length >= 8; length -= 8, data += 8)
    {
      ctx->state.a[index++] ^= LE_READ_UINT64 (data);
      if (index == block_size)
	{
	  sha3_permute (&ctx->state);
	  index = 0;
	}
    }
  memcpy (ctx->block.b, data, length);
  ctx->index = (index << 3) + length;
#else /* !WORDS_BIGENDIAN */
  /* Switch to units of bytes. */
  block_size <<= 3;
  if (ctx->index > 0)
    {
      unsigned left = block_size - ctx->index;
      if (length < left)
	{
	  memxor ((uint8_t *) ctx->state.a + ctx->index, data, length);
	  ctx->index += length;
	  return;
	}
      memxor ((uint8_t *) ctx->state.a + ctx->index, data, left);
      data += left; length -= left;
      ctx->index = 0;
      sha3_permute (&ctx->state);
    }
  for (; length >= block_size; length -= block_size, data += block_size)
    {
      memxor ((uint8_t *) ctx->state.a, data, block_size);
      sha3_permute (&ctx->state);
    }
  memxor ((uint8_t *) ctx->state.a, data, length);
  ctx->index = length;
#endif /* !WORDS_BIGENDIAN */
}

void
_nettle_sha3_pad (struct sha3_ctx *ctx, unsigned block_size, uint8_t magic)
{
#if WORDS_BIGENDIAN
  unsigned byte_index = ctx->index & 7;
  unsigned word_index = ctx->index >> 3;

  ctx->block.b[byte_index++] = magic;
  memset (ctx->block.b + byte_index, 0, 8 - byte_index);
  ctx->state.a[word_index] ^= nettle_bswap64 (ctx->block.u64);
#else /* !WORDS_BIGENDIAN */
  ((uint8_t *) ctx->state.a)[ctx->index] ^= magic;
#endif /* !WORDS_BIGENDIAN */
  ctx->state.a[block_size - 1] ^= (uint64_t) 1 << 63;
}

void
_nettle_sha3_digest (struct sha3_ctx *ctx, unsigned block_size,
		     unsigned digest_size, uint8_t *digest)
{
  _nettle_sha3_pad (ctx, block_size, SHA3_HASH_MAGIC);
  sha3_permute (&ctx->state);
  _nettle_write_le64 (digest_size, digest, ctx->state.a);
  sha3_init (ctx);
}
