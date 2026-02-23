/* slh-shake.c

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

#include "slh-dsa-internal.h"

#include "bswap-internal.h"
#include "sha3.h"

/* Fields always big-endian */
struct slh_address_tree
{
  uint32_t layer;
  uint32_t pad; /* Always zero */
  uint64_t tree_idx;
};

static void
slh_shake_init_tree (struct sha3_ctx *ctx, const uint8_t *public_seed,
		     uint32_t layer, uint64_t tree_idx)
{
  struct slh_address_tree at = { bswap32_if_le (layer), 0, bswap64_if_le (tree_idx) };

  sha3_init (ctx);
  sha3_256_update (ctx, _SLH_DSA_128_SIZE, public_seed);
  sha3_256_update (ctx, sizeof (at), (const uint8_t *) &at);
}

static void
slh_shake_init_hash (const struct sha3_ctx *tree_ctx, struct sha3_ctx *ctx,
		     const struct slh_address_hash *ah)
{
  *ctx = *tree_ctx;
  sha3_256_update (ctx, sizeof (*ah), (const uint8_t *) ah);
}

static void
slh_shake_secret (const struct sha3_ctx *tree_ctx, const struct slh_address_hash *ah,
		  const uint8_t *secret, uint8_t *out)
{
  struct sha3_ctx ctx;
  slh_shake_init_hash (tree_ctx, &ctx, ah);;
  sha3_256_update (&ctx, _SLH_DSA_128_SIZE, secret);
  sha3_256_shake (&ctx, _SLH_DSA_128_SIZE, out);
}

static void
slh_shake_node (const struct sha3_ctx *tree_ctx, const struct slh_address_hash *ah,
		const uint8_t *left, const uint8_t *right, uint8_t *out)
{
  struct sha3_ctx ctx;
  slh_shake_init_hash (tree_ctx, &ctx, ah);;
  sha3_256_update (&ctx, _SLH_DSA_128_SIZE, left);
  sha3_256_update (&ctx, _SLH_DSA_128_SIZE, right);
  sha3_256_shake (&ctx, _SLH_DSA_128_SIZE, out);
}

static void
slh_shake_digest (struct sha3_ctx *ctx, uint8_t *out)
{
  sha3_256_shake (ctx, _SLH_DSA_128_SIZE, out);
}

static void
slh_shake_randomizer (const uint8_t *public_seed, const uint8_t *secret_prf,
		      size_t prefix_length, const uint8_t *prefix,
		      size_t msg_length, const uint8_t *msg,
		      uint8_t *randomizer)
{
  struct sha3_ctx ctx;

  sha3_init (&ctx);
  sha3_256_update (&ctx, _SLH_DSA_128_SIZE, secret_prf);
  sha3_256_update (&ctx, _SLH_DSA_128_SIZE, public_seed);
  sha3_256_update (&ctx, prefix_length, prefix);
  sha3_256_update (&ctx, msg_length, msg);
  sha3_256_shake (&ctx, _SLH_DSA_128_SIZE, randomizer);
}

static void
slh_shake_msg_digest (const uint8_t *randomizer, const uint8_t *pub,
		      size_t prefix_length, const uint8_t *prefix,
		      size_t msg_length, const uint8_t *msg,
		      size_t digest_size, uint8_t *digest)
{
  struct sha3_ctx ctx;

  sha3_init (&ctx);
  sha3_256_update (&ctx, _SLH_DSA_128_SIZE, randomizer);
  sha3_256_update (&ctx, 2*_SLH_DSA_128_SIZE, pub);
  sha3_256_update (&ctx, prefix_length, prefix);
  sha3_256_update (&ctx, msg_length, msg);
  sha3_256_shake (&ctx, digest_size, digest);
}

const struct slh_hash
_slh_hash_shake =
  {
    (slh_hash_init_tree_func *) slh_shake_init_tree,
    (slh_hash_init_hash_func *) slh_shake_init_hash,
    (nettle_hash_update_func *) sha3_256_update,
    (nettle_hash_digest_func *) slh_shake_digest,
    (slh_hash_secret_func *) slh_shake_secret,
    (slh_hash_node_func *) slh_shake_node,
    slh_shake_randomizer,
    slh_shake_msg_digest
  };
