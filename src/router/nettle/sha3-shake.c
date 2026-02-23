/* sha3-shake.c

   Copyright (C) 2017, 2024 Daiki Ueno
   Copyright (C) 2017 Red Hat, Inc.
   Copyright (C) 2024 Niels Möller

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

#include <assert.h>
#include <string.h>

#include "sha3.h"
#include "sha3-internal.h"

#include "bswap-internal.h"
#include "macros.h"
#include "nettle-write.h"

void
_nettle_sha3_shake (struct sha3_ctx *ctx, unsigned block_size,
		    size_t length, uint8_t *dst)
{
  _nettle_sha3_pad (ctx, block_size, SHA3_SHAKE_MAGIC);

  /* Use byte units. */
  block_size <<= 3;

  while (length > block_size)
    {
      sha3_permute (&ctx->state);
      _nettle_write_le64 (block_size, dst, ctx->state.a);
      length -= block_size; dst += block_size;
    }

  sha3_permute (&ctx->state);
  _nettle_write_le64 (length, dst, ctx->state.a);
  sha3_init (ctx);
}

void
_nettle_sha3_shake_output (struct sha3_ctx *ctx, unsigned block_size,
			   size_t length, uint8_t *dst)
{
  unsigned index = ctx->index;
  unsigned block_bytes = block_size << 3;

  if (!ctx->shake_flag)
    {
      /* This is the first call of _shake_output.  */
      _nettle_sha3_pad (ctx, block_size, SHA3_SHAKE_MAGIC);
      /* Point at the end of block to trigger fill in of the buffer.  */
      index = block_bytes;
      ctx->shake_flag = 1;
    }

  assert (index <= block_bytes);
  {
#if WORDS_BIGENDIAN
    unsigned byte_index = index & 7;

    if (byte_index)
      {
	unsigned left = 8 - byte_index;
	if (length <= left)
	  {
	    memcpy (dst, ctx->block.b + byte_index, length);
	    ctx->index = index + length;
	    return;
	  }
	memcpy (dst, ctx->block.b + byte_index, left);
	dst += left; length -= left; index += left;
      }

    /* Switch to units of words */
    index >>= 3;

    for (; length > 0; length -= 8, dst += 8, index++)
      {
	if (index == block_size)
	  {
	    sha3_permute (&ctx->state);
	    index = 0;
	  }
	if (length < 8)
	  {
	    ctx->block.u64 = nettle_bswap64 (ctx->state.a[index]);
	    memcpy (dst, ctx->block.b, length);
	    break;
	  }
	LE_WRITE_UINT64(dst, ctx->state.a[index]);
      }
    ctx->index = (index << 3) + length;
#else /* !WORDS_BIGENDIAN */
    size_t left = block_bytes - index;
    if (length <= left)
      {
	memcpy (dst, ((const uint8_t *) ctx->state.a) + index, length);
	ctx->index = index + length;
	return;
      }
    else
      {
	memcpy (dst, ((const uint8_t *) ctx->state.a) + index, left);
	length -= left;
	dst += left;
      }

    /* Write full blocks.  */
    for (; length > block_bytes; length -= block_bytes, dst += block_bytes)
      {
	sha3_permute (&ctx->state);
	_nettle_write_le64 (block_bytes, dst, ctx->state.a);
      }

    sha3_permute (&ctx->state);
    memcpy (dst, ctx->state.a, length);
    ctx->index = length;
#endif /* !WORDS_BIGENDIAN */
  }
}
