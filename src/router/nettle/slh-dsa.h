/* slh-dsa.h

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

#ifndef NETTLE_SLH_DSA_H
#define NETTLE_SLH_DSA_H

#include "nettle-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define slh_dsa_shake_128s_root nettle_slh_dsa_shake_128s_root
#define slh_dsa_shake_128f_root nettle_slh_dsa_shake_128f_root
#define slh_dsa_sha2_128s_root nettle_slh_dsa_sha2_128s_root
#define slh_dsa_sha2_128f_root nettle_slh_dsa_sha2_128f_root
#define slh_dsa_shake_128s_generate_keypair nettle_slh_dsa_shake_128s_generate_keypair
#define slh_dsa_shake_128f_generate_keypair nettle_slh_dsa_shake_128f_generate_keypair
#define slh_dsa_sha2_128s_generate_keypair nettle_slh_dsa_sha2_128s_generate_keypair
#define slh_dsa_sha2_128f_generate_keypair nettle_slh_dsa_sha2_128f_generate_keypair
#define slh_dsa_shake_128s_sign nettle_slh_dsa_shake_128s_sign
#define slh_dsa_shake_128f_sign nettle_slh_dsa_shake_128f_sign
#define slh_dsa_sha2_128s_sign nettle_slh_dsa_sha2_128s_sign
#define slh_dsa_sha2_128f_sign nettle_slh_dsa_sha2_128f_sign
#define slh_dsa_shake_128s_verify nettle_slh_dsa_shake_128s_verify
#define slh_dsa_shake_128f_verify nettle_slh_dsa_shake_128f_verify
#define slh_dsa_sha2_128s_verify nettle_slh_dsa_sha2_128s_verify
#define slh_dsa_sha2_128f_verify nettle_slh_dsa_sha2_128f_verify

/* Key layout:
   private:
     secret_seed
     prf
   public:
     public_seed
     root
*/

#define SLH_DSA_128_SEED_SIZE 16
#define SLH_DSA_128_KEY_SIZE 32
#define SLH_DSA_128S_SIGNATURE_SIZE 7856
#define SLH_DSA_128F_SIGNATURE_SIZE 17088

/* Computes public key root, from the two seeds. */
void
slh_dsa_shake_128s_root (const uint8_t *public_seed, const uint8_t *private_seed,
			 uint8_t *root);
void
slh_dsa_shake_128f_root (const uint8_t *public_seed, const uint8_t *private_seed,
			 uint8_t *root);
void
slh_dsa_sha2_128s_root (const uint8_t *public_seed, const uint8_t *private_seed,
			uint8_t *root);
void
slh_dsa_sha2_128f_root (const uint8_t *public_seed, const uint8_t *private_seed,
			uint8_t *root);

void
slh_dsa_shake_128s_generate_keypair (uint8_t *pub, uint8_t *key,
				     void *random_ctx, nettle_random_func *random);
void
slh_dsa_shake_128f_generate_keypair (uint8_t *pub, uint8_t *key,
				     void *random_ctx, nettle_random_func *random);
void
slh_dsa_sha2_128s_generate_keypair (uint8_t *pub, uint8_t *key,
				    void *random_ctx, nettle_random_func *random);
void
slh_dsa_sha2_128f_generate_keypair (uint8_t *pub, uint8_t *key,
				    void *random_ctx, nettle_random_func *random);

/* Only the "pure" and deterministic variant. */
void
slh_dsa_shake_128s_sign (const uint8_t *pub, const uint8_t *priv,
			 size_t length, const uint8_t *msg,
			 uint8_t *signature);
void
slh_dsa_shake_128f_sign (const uint8_t *pub, const uint8_t *priv,
			 size_t length, const uint8_t *msg,
			 uint8_t *signature);
void
slh_dsa_sha2_128s_sign (const uint8_t *pub, const uint8_t *priv,
			size_t length, const uint8_t *msg,
			uint8_t *signature);
void
slh_dsa_sha2_128f_sign (const uint8_t *pub, const uint8_t *priv,
			size_t length, const uint8_t *msg,
			uint8_t *signature);

int
slh_dsa_shake_128s_verify (const uint8_t *pub,
			   size_t length, const uint8_t *msg,
			   const uint8_t *signature);
int
slh_dsa_shake_128f_verify (const uint8_t *pub,
			   size_t length, const uint8_t *msg,
			   const uint8_t *signature);
int
slh_dsa_sha2_128s_verify (const uint8_t *pub,
			  size_t length, const uint8_t *msg,
			  const uint8_t *signature);
int
slh_dsa_sha2_128f_verify (const uint8_t *pub,
			  size_t length, const uint8_t *msg,
			  const uint8_t *signature);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_SLH_DSA_H */
