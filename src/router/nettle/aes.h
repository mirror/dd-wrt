/* aes.h

   The aes/rijndael block cipher.

   Copyright (C) 2001, 2013 Niels Möller

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
 
#ifndef NETTLE_AES_H_INCLUDED
#define NETTLE_AES_H_INCLUDED

#include "nettle-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define aes128_set_encrypt_key nettle_aes128_set_encrypt_key
#define aes128_set_decrypt_key nettle_aes128_set_decrypt_key
#define aes128_invert_key nettle_aes128_invert_key
#define aes128_encrypt nettle_aes128_encrypt
#define aes128_decrypt nettle_aes128_decrypt
#define aes192_set_encrypt_key nettle_aes192_set_encrypt_key
#define aes192_set_decrypt_key nettle_aes192_set_decrypt_key
#define aes192_invert_key nettle_aes192_invert_key
#define aes192_encrypt nettle_aes192_encrypt
#define aes192_decrypt nettle_aes192_decrypt
#define aes256_set_encrypt_key nettle_aes256_set_encrypt_key
#define aes256_set_decrypt_key nettle_aes256_set_decrypt_key
#define aes256_invert_key nettle_aes256_invert_key
#define aes256_encrypt nettle_aes256_encrypt
#define aes256_decrypt nettle_aes256_decrypt

#define AES_BLOCK_SIZE 16

#define AES128_KEY_SIZE 16
#define AES192_KEY_SIZE 24
#define AES256_KEY_SIZE 32
#define _AES128_ROUNDS 10
#define _AES192_ROUNDS 12
#define _AES256_ROUNDS 14

struct aes128_ctx
{
  _NETTLE_ALIGN16 uint32_t keys[4 * (_AES128_ROUNDS + 1)];
};

void
aes128_set_encrypt_key(struct aes128_ctx *ctx, const uint8_t *key);
void
aes128_set_decrypt_key(struct aes128_ctx *ctx, const uint8_t *key);
void
aes128_invert_key(struct aes128_ctx *dst,
		  const struct aes128_ctx *src);
void
aes128_encrypt(const struct aes128_ctx *ctx,
	       size_t length, uint8_t *dst,
	       const uint8_t *src);
void
aes128_decrypt(const struct aes128_ctx *ctx,
	       size_t length, uint8_t *dst,
	       const uint8_t *src);

struct aes192_ctx
{
  _NETTLE_ALIGN16 uint32_t keys[4 * (_AES192_ROUNDS + 1)];
};

void
aes192_set_encrypt_key(struct aes192_ctx *ctx, const uint8_t *key);
void
aes192_set_decrypt_key(struct aes192_ctx *ctx, const uint8_t *key);
void
aes192_invert_key(struct aes192_ctx *dst,
		  const struct aes192_ctx *src);
void
aes192_encrypt(const struct aes192_ctx *ctx,
	       size_t length, uint8_t *dst,
	       const uint8_t *src);
void
aes192_decrypt(const struct aes192_ctx *ctx,
	       size_t length, uint8_t *dst,
	       const uint8_t *src);

struct aes256_ctx
{
  _NETTLE_ALIGN16 uint32_t keys[4 * (_AES256_ROUNDS + 1)];
};

void
aes256_set_encrypt_key(struct aes256_ctx *ctx, const uint8_t *key);
void
aes256_set_decrypt_key(struct aes256_ctx *ctx, const uint8_t *key);
void
aes256_invert_key(struct aes256_ctx *dst,
		  const struct aes256_ctx *src);
void
aes256_encrypt(const struct aes256_ctx *ctx,
	       size_t length, uint8_t *dst,
	       const uint8_t *src);
void
aes256_decrypt(const struct aes256_ctx *ctx,
	       size_t length, uint8_t *dst,
	       const uint8_t *src);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_AES_H_INCLUDED */
