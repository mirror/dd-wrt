/* slh-wots.c

   WOTS+ one-time signatures, part of SLH-DSA (FIPS 205)

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

/* If s == 0, returns src and leaves dst unchanged. Otherwise, returns
   dst. For the ah argument, leaves ah->keypair and ah->height_chain
   unchanged, but overwrites the other fields. */
static const uint8_t *
wots_chain (const struct slh_hash *hash, const void *ctx,
	    struct slh_address_hash *ah,
	    unsigned i, unsigned s,
	    const uint8_t *src, uint8_t *dst)
{
  unsigned j;

  if (s == 0)
    return src;

  ah->type = bswap32_if_le (SLH_WOTS_HASH);
  ah->index_hash = bswap32_if_le (i);

  hash->secret (ctx, ah, src, dst);

  for (j = 1; j < s; j++)
    {
      ah->index_hash = bswap32_if_le (i + j);
      hash->secret (ctx, ah, dst, dst);
    }

  return dst;
}

static void
wots_pk_init (const struct slh_hash *hash, const void *tree_ctx,
	      unsigned keypair, struct slh_address_hash *ah,
	      void *ctx)
{
  ah->type = bswap32_if_le (SLH_WOTS_PK);
  ah->keypair = bswap32_if_le (keypair);
  ah->height_chain = 0;
  ah->index_hash = 0;
  hash->init_hash (tree_ctx, ctx, ah);
}

void
_wots_gen (const struct slh_hash *hash, const void *tree_ctx,
	   const uint8_t *secret_seed,
	   uint32_t keypair, uint8_t *pub, void *pub_ctx)
{
  struct slh_address_hash ah;
  unsigned i;

  wots_pk_init (hash, tree_ctx, keypair, &ah, pub_ctx);

  for (i = 0; i < _WOTS_SIGNATURE_LENGTH; i++)
    {
      uint8_t out[_SLH_DSA_128_SIZE];

      /* Generate secret value. */
      ah.type = bswap32_if_le (SLH_WOTS_PRF);
      ah.height_chain = bswap32_if_le (i);
      ah.index_hash = 0;
      hash->secret (tree_ctx, &ah, secret_seed, out);

      /* Hash chain. */
      wots_chain (hash, tree_ctx, &ah, 0, 15, out, out);

      hash->update (pub_ctx, _SLH_DSA_128_SIZE, out);
    }
  hash->digest (pub_ctx, pub);
}

/* Produces signature hash corresponding to the ith message nybble. Modifies addr. */
static void
wots_sign_one (const struct slh_hash *hash, const void *tree_ctx,
	       const uint8_t *secret_seed, uint32_t keypair,
	       unsigned i, uint8_t msg,
	       uint8_t *signature, void *pub_ctx)
{
  struct slh_address_hash ah;
  uint8_t pub[_SLH_DSA_128_SIZE];
  signature += i*_SLH_DSA_128_SIZE;

  /* Generate secret value. */
  ah.type = bswap32_if_le (SLH_WOTS_PRF);
  ah.keypair = bswap32_if_le (keypair);
  ah.height_chain = bswap32_if_le (i);
  ah.index_hash = 0;
  hash->secret (tree_ctx, &ah, secret_seed, signature);

  /* Hash chain. */
  wots_chain (hash, tree_ctx, &ah, 0, msg, signature, signature);

  hash->update (pub_ctx, _SLH_DSA_128_SIZE,
		wots_chain (hash, tree_ctx, &ah, msg, 15 - msg, signature, pub));
}

void
_wots_sign (const struct slh_hash *hash, const void *tree_ctx,
	    const uint8_t *secret_seed, unsigned keypair, const uint8_t *msg,
	    uint8_t *signature, uint8_t *pub, void *pub_ctx)
{
  struct slh_address_hash ah;
  unsigned i;
  uint32_t csum;

  wots_pk_init (hash, tree_ctx, keypair, &ah, pub_ctx);

  for (i = 0, csum = 15*32; i < _SLH_DSA_128_SIZE; i++)
    {
      uint8_t m0, m1;
      m0 = msg[i] >> 4;
      csum -= m0;
      wots_sign_one (hash, tree_ctx, secret_seed, keypair, 2*i, m0, signature, pub_ctx);

      m1 = msg[i] & 0xf;
      csum -= m1;
      wots_sign_one (hash, tree_ctx, secret_seed, keypair, 2*i + 1, m1, signature, pub_ctx);
    }

  wots_sign_one (hash, tree_ctx, secret_seed, keypair, 32, csum >> 8, signature, pub_ctx);
  wots_sign_one (hash, tree_ctx, secret_seed, keypair, 33, (csum >> 4) & 0xf, signature, pub_ctx);
  wots_sign_one (hash, tree_ctx, secret_seed, keypair, 34, csum & 0xf, signature, pub_ctx);

  hash->digest (pub_ctx, pub);
}

static void
wots_verify_one (const struct slh_hash *hash, const void *tree_ctx,
		 uint32_t keypair, unsigned i, uint8_t msg,
		 const uint8_t *signature, void *pub_ctx)
{
  struct slh_address_hash ah;
  uint8_t out[_SLH_DSA_128_SIZE];
  signature += i*_SLH_DSA_128_SIZE;

  ah.keypair = bswap32_if_le (keypair);
  ah.height_chain = bswap32_if_le (i);

  hash->update (pub_ctx, _SLH_DSA_128_SIZE,
		wots_chain (hash, tree_ctx, &ah, msg, 15 - msg, signature, out));
}

void
_wots_verify (const struct slh_hash *hash, const void *tree_ctx,
	      unsigned keypair, const uint8_t *msg, const uint8_t *signature, uint8_t *pub,
	      void *pub_ctx)
{
  struct slh_address_hash ah;
  unsigned i;
  uint32_t csum;

  wots_pk_init (hash, tree_ctx, keypair, &ah, pub_ctx);

  for (i = 0, csum = 15*32; i < _SLH_DSA_128_SIZE; i++)
    {
      uint8_t m0, m1;
      m0 = msg[i] >> 4;
      csum -= m0;
      wots_verify_one (hash, tree_ctx, keypair, 2*i, m0, signature, pub_ctx);

      m1 = msg[i] & 0xf;
      csum -= m1;
      wots_verify_one (hash, tree_ctx, keypair, 2*i + 1, m1, signature, pub_ctx);
    }

  wots_verify_one (hash, tree_ctx, keypair, 32, csum >> 8, signature, pub_ctx);
  wots_verify_one (hash, tree_ctx, keypair, 33, (csum >> 4) & 0xf, signature, pub_ctx);
  wots_verify_one (hash, tree_ctx, keypair, 34, csum & 0xf, signature, pub_ctx);

  hash->digest (pub_ctx, pub);
}
