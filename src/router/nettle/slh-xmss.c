/* slh-xmss.c

   The eXtended Merkle Signature Scheme, part of SLH-DSA (FIPS 205)

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

#include "bswap-internal.h"
#include "slh-dsa-internal.h"

static void
xmss_leaf (const struct slh_merkle_ctx_secret *ctx, unsigned idx, uint8_t *leaf)
{
  _wots_gen (ctx->pub.hash, ctx->pub.tree_ctx, ctx->secret_seed, idx, leaf, ctx->scratch_ctx);
}

static void
xmss_node (const struct slh_merkle_ctx_public *ctx, unsigned height, unsigned index,
	   const uint8_t *left, const uint8_t *right, uint8_t *out)
{
  struct slh_address_hash ah =
    {
      bswap32_if_le (SLH_XMSS_TREE),
      0,
      bswap32_if_le (height),
      bswap32_if_le (index),
    };
  ctx->hash->node (ctx->tree_ctx, &ah, left, right, out);
}

void
_xmss_gen (const struct slh_hash *hash,
	   const uint8_t *public_seed, const uint8_t *secret_seed,
	   const struct slh_xmss_params *xmss, uint8_t *root,
	   void *tree_ctx, void *scratch_ctx, uint8_t *scratch)
{
  const struct slh_merkle_ctx_secret ctx =
    {
      { hash, tree_ctx, 0 },
      secret_seed, scratch_ctx,
    };
  hash->init_tree (tree_ctx, public_seed, xmss->d - 1, 0);
  _merkle_root (&ctx, xmss_leaf, xmss_node, xmss->h, 0, root, scratch);
}

void
_xmss_sign (const struct slh_merkle_ctx_secret *ctx, unsigned h,
	    unsigned idx, const uint8_t *msg, uint8_t *signature, uint8_t *pub)
{
  _wots_sign (ctx->pub.hash, ctx->pub.tree_ctx, ctx->secret_seed, idx,
	      msg, signature, pub, ctx->scratch_ctx);
  signature += WOTS_SIGNATURE_SIZE;

  _merkle_sign (ctx, xmss_leaf, xmss_node, h, idx, signature);
  _merkle_verify (&ctx->pub, xmss_node, h, idx, signature, pub);
}

void
_xmss_verify (const struct slh_merkle_ctx_public *ctx, unsigned h,
	      unsigned idx, const uint8_t *msg, const uint8_t *signature, uint8_t *pub,
	      void *scratch_ctx)
{
  _wots_verify (ctx->hash, ctx->tree_ctx, idx, msg, signature, pub, scratch_ctx);
  signature += WOTS_SIGNATURE_SIZE;

  _merkle_verify (ctx, xmss_node, h, idx, signature, pub);
}
