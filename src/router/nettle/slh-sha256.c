/* slh-sha256.c

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

#include <string.h>

#include "slh-dsa-internal.h"

#include "bswap-internal.h"
#include "hmac.h"
#include "sha2.h"

/* Uses a "compressed" address,

     uint8_t layer
     uint64_t tree_idx
     uint8_t type

   (packed, no padding).
*/

/* All hashing but H_{msg} and PRF_{msg} use plain sha256: */
static void
slh_sha256_init_tree (struct sha256_ctx *ctx, const uint8_t *public_seed,
		      uint32_t layer, uint64_t tree_idx)
{
  static const uint8_t pad[48];
  uint8_t addr_layer;
  uint64_t addr_tree;

  sha256_init (ctx);
  sha256_update (ctx, _SLH_DSA_128_SIZE, public_seed);
  /* This padding completes a sha256 block. */
  sha256_update (ctx, sizeof (pad), pad);
  /* Compressed address. */
  addr_layer = layer;
  sha256_update (ctx, 1, &addr_layer);
  addr_tree = bswap64_if_le (tree_idx);
  sha256_update (ctx, sizeof (addr_tree), (const uint8_t *) &addr_tree);
}

static void
slh_sha256_init_hash (const struct sha256_ctx *tree_ctx, struct sha256_ctx *ctx,
		      const struct slh_address_hash *ah)
{
  *ctx = *tree_ctx;
  /* For compressed addr, hash only last byte of the type. */
  sha256_update (ctx, sizeof (*ah) - 3, (const uint8_t *) ah + 3);
}

static void
slh_sha256_digest (struct sha256_ctx *ctx, uint8_t *out)
{
  uint8_t digest[SHA256_DIGEST_SIZE];
  sha256_digest (ctx, digest);
  memcpy (out, digest, _SLH_DSA_128_SIZE);
}

static void
slh_sha256_secret (const struct sha256_ctx *tree_ctx,
		   const struct slh_address_hash *ah,
		   const uint8_t *secret, uint8_t *out)
{
  struct sha256_ctx ctx;
  slh_sha256_init_hash (tree_ctx, &ctx, ah);
  sha256_update (&ctx, _SLH_DSA_128_SIZE, secret);
  slh_sha256_digest (&ctx, out);
}

static void
slh_sha256_node (const struct sha256_ctx *tree_ctx,
		 const struct slh_address_hash *ah,
		 const uint8_t *left, const uint8_t *right, uint8_t *out)
{
  struct sha256_ctx ctx;
  slh_sha256_init_hash (tree_ctx, &ctx, ah);
  sha256_update (&ctx, _SLH_DSA_128_SIZE, left);
  sha256_update (&ctx, _SLH_DSA_128_SIZE, right);
  slh_sha256_digest (&ctx, out);
}

static void
slh_sha256_randomizer (const uint8_t *public_seed, const uint8_t *secret_prf,
		       size_t prefix_length, const uint8_t *prefix,
		       size_t msg_length, const uint8_t *msg,
		       uint8_t *randomizer)
{
  struct hmac_sha256_ctx ctx;
  uint8_t digest[SHA256_DIGEST_SIZE];
  hmac_sha256_set_key (&ctx, _SLH_DSA_128_SIZE, secret_prf);
  hmac_sha256_update (&ctx, _SLH_DSA_128_SIZE, public_seed);
  hmac_sha256_update (&ctx, prefix_length, prefix);
  hmac_sha256_update (&ctx, msg_length, msg);
  hmac_sha256_digest (&ctx, digest);
  memcpy (randomizer, digest, _SLH_DSA_128_SIZE);
}

static void
slh_sha256_msg_digest (const uint8_t *randomizer, const uint8_t *pub,
		       size_t prefix_length, const uint8_t *prefix,
		       size_t length, const uint8_t *msg,
		       size_t digest_size, uint8_t *digest)
{
  struct sha256_ctx ctx;
  uint8_t inner[SHA256_DIGEST_SIZE];
  uint32_t i;
  sha256_init (&ctx);
  sha256_update (&ctx, _SLH_DSA_128_SIZE, randomizer);
  sha256_update (&ctx, 2*_SLH_DSA_128_SIZE, pub);
  sha256_update (&ctx, prefix_length, prefix);
  sha256_update (&ctx, length, msg);
  sha256_digest (&ctx, inner);

  /* mgf1 with inner digest as the seed. */
  for (i = 0; digest_size > 0; i++)
    {
      uint32_t i_be = bswap32_if_le (i);
      sha256_update (&ctx, _SLH_DSA_128_SIZE, randomizer);
      sha256_update (&ctx, _SLH_DSA_128_SIZE, pub);
      sha256_update (&ctx, sizeof (inner), inner);
      sha256_update (&ctx, sizeof (i_be), (const uint8_t *) &i_be);
      if (digest_size < SHA256_DIGEST_SIZE)
	{
	  sha256_digest (&ctx, inner);
	  memcpy (digest, inner, digest_size);
	  break;
	}
      sha256_digest (&ctx, digest);
      digest += SHA256_DIGEST_SIZE;
      digest_size -= SHA256_DIGEST_SIZE;
    }
}

const struct slh_hash
_slh_hash_sha256 =
  {
    (slh_hash_init_tree_func *) slh_sha256_init_tree,
    (slh_hash_init_hash_func *) slh_sha256_init_hash,
    (nettle_hash_update_func *) sha256_update,
    (nettle_hash_digest_func *) slh_sha256_digest,
    (slh_hash_secret_func *) slh_sha256_secret,
    (slh_hash_node_func *) slh_sha256_node,
    slh_sha256_randomizer,
    slh_sha256_msg_digest
  };
