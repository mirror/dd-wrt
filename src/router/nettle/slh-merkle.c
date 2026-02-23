/* slh-merkle.c

   Copyright (C) 2025 Niels Möller

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

#include "slh-dsa-internal.h"

/* Computes root hash of a tree.
   Example for height == 2, 4 entries:

   i = 0 ==> stack: [0], i = 1
   i = 1 ==> stack: [0, 1] ==> [0|1], i = 2
   i = 2 ==> stack: [0|1, 2], i = 3
   i = 3 ==> stack: [0|1, 2, 3] ==> [0|1, 2|3] ==> [0|1|2|3], i = 4

   The size of the stack equals popcount(i)
*/
void
_merkle_root (const struct slh_merkle_ctx_secret *ctx,
	      merkle_leaf_hash_func *leaf_hash, merkle_node_hash_func *node_hash,
	      unsigned height, unsigned start, uint8_t *root,
	      /* Must have space for (height + 1) node hashes */
	      uint8_t *stack)
{
  unsigned stack_size = 0;
  unsigned i;
  assert (height > 0);
  assert ( (start & ((1<<height) - 1)) == 0);

  for (i = 0; i < (1<<height); i++)
    {
      /* Leaf index. */
      unsigned idx = start + i;
      unsigned h;
      assert (stack_size <= height);

      leaf_hash (ctx, idx, stack + stack_size++ * _SLH_DSA_128_SIZE);

      for (h = 1; (idx&1); h++)
	{
	  assert (stack_size >= 2);
	  idx >>= 1;
	  stack_size--;
	  if (h == height)
	    {
	      assert (stack_size == 1);
	      node_hash (&ctx->pub, h, idx,
			 stack + (stack_size - 1) * _SLH_DSA_128_SIZE,
			 stack + stack_size * _SLH_DSA_128_SIZE,
			 root);
	      return;
	    }
	  node_hash (&ctx->pub, h, idx,
		     stack + (stack_size - 1) * _SLH_DSA_128_SIZE,
		     stack + stack_size * _SLH_DSA_128_SIZE,
		     stack + (stack_size - 1)* _SLH_DSA_128_SIZE);
	}
    }
}

void
_merkle_sign (const struct slh_merkle_ctx_secret *ctx,
	      merkle_leaf_hash_func *leaf_hash, merkle_node_hash_func *node_hash,
	      unsigned height, unsigned idx, uint8_t *signature)
{
  unsigned h;
  /* By generating the path from the end, we can use the output area
     as the temporary stack. */
  for (h = height; --h > 0;)
    _merkle_root (ctx, leaf_hash, node_hash, h, (idx & -(1 << h)) ^ (1 << h),
		  signature + h*_SLH_DSA_128_SIZE, signature);

  leaf_hash (ctx, idx ^ 1, signature);
}

void
_merkle_verify (const struct slh_merkle_ctx_public *ctx, merkle_node_hash_func *node_hash,
		unsigned height, unsigned idx, const uint8_t *signature, uint8_t *hash)
{
  unsigned h;

  for (h = 1; h <= height; h++, signature += _SLH_DSA_128_SIZE)
    {
      unsigned right = idx & 1;
      idx >>= 1;
      if (right)
	node_hash (ctx, h, idx, signature, hash, hash);
      else
	node_hash (ctx, h, idx, hash, signature, hash);
    }
}
