/* slh-dsa-sha2-128s.c

   SLH-DSA (FIPS 205) signatures.

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

#include "slh-dsa.h"
#include "slh-dsa-internal.h"

#include "sha2.h"

#define SLH_DSA_M 30

#define XMSS_H 9

void
slh_dsa_sha2_128s_root (const uint8_t *public_seed, const uint8_t *private_seed,
			uint8_t *root)
{
  struct sha256_ctx tree_ctx, scratch_ctx;
  uint8_t scratch[(XMSS_H + 1)*_SLH_DSA_128_SIZE];
  _xmss_gen (&_slh_hash_sha256, public_seed, private_seed,
	     &_slh_dsa_128s_params.xmss, root,
	     &tree_ctx, &scratch_ctx, scratch);
}

void
slh_dsa_sha2_128s_generate_keypair (uint8_t *pub, uint8_t *priv,
				    void *random_ctx, nettle_random_func *random)
{
  random (random_ctx, SLH_DSA_128_SEED_SIZE, pub);
  random (random_ctx, 2*SLH_DSA_128_SEED_SIZE, priv);
  slh_dsa_sha2_128s_root (pub, priv, pub + SLH_DSA_128_SEED_SIZE);
}

/* Only the "pure" and deterministic variant. */
void
slh_dsa_sha2_128s_sign (const uint8_t *pub, const uint8_t *priv,
			size_t length, const uint8_t *msg,
			uint8_t *signature)
{
  struct sha256_ctx tree_ctx, scratch_ctx;
  uint8_t digest[SLH_DSA_M];
  _slh_dsa_pure_rdigest (&_slh_hash_sha256,
			 pub, priv + _SLH_DSA_128_SIZE, length, msg,
			 signature, sizeof (digest), digest);
  _slh_dsa_sign (&_slh_dsa_128s_params, &_slh_hash_sha256,
		 pub, priv, digest, signature + _SLH_DSA_128_SIZE,
		 &tree_ctx, &scratch_ctx);
}

int
slh_dsa_sha2_128s_verify (const uint8_t *pub,
			  size_t length, const uint8_t *msg,
			  const uint8_t *signature)
{
  struct sha256_ctx tree_ctx, scratch_ctx;
  uint8_t digest[SLH_DSA_M];
  _slh_dsa_pure_digest (&_slh_hash_sha256,
			pub, length, msg, signature, sizeof (digest), digest);
  return _slh_dsa_verify (&_slh_dsa_128s_params, &_slh_hash_sha256,
			  pub, digest, signature + _SLH_DSA_128_SIZE,
			  &tree_ctx, &scratch_ctx);
}
