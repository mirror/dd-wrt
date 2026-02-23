/* hmac.h

   HMAC message authentication code (RFC-2104).

   Copyright (C) 2001, 2002, 2025 Niels Möller

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

#ifndef NETTLE_HMAC_H_INCLUDED
#define NETTLE_HMAC_H_INCLUDED

#include <stdalign.h>

#include "nettle-meta.h"

#include "gosthash94.h"
#include "md5.h"
#include "ripemd160.h"
#include "sha1.h"
#include "sha2.h"
#include "streebog.h"
#include "sm3.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Namespace mangling */
#define hmac_md5_set_key nettle_hmac_md5_set_key
#define hmac_md5_update nettle_hmac_md5_update
#define hmac_md5_digest nettle_hmac_md5_digest
#define hmac_ripemd160_set_key nettle_hmac_ripemd160_set_key
#define hmac_ripemd160_update nettle_hmac_ripemd160_update
#define hmac_ripemd160_digest nettle_hmac_ripemd160_digest
#define hmac_sha1_set_key nettle_hmac_sha1_set_key
#define hmac_sha1_update nettle_hmac_sha1_update
#define hmac_sha1_digest nettle_hmac_sha1_digest
#define hmac_sha224_set_key nettle_hmac_sha224_set_key
#define hmac_sha224_digest nettle_hmac_sha224_digest
#define hmac_sha256_set_key nettle_hmac_sha256_set_key
#define hmac_sha256_update nettle_hmac_sha256_update
#define hmac_sha256_digest nettle_hmac_sha256_digest
#define hmac_sha384_set_key nettle_hmac_sha384_set_key
#define hmac_sha384_digest nettle_hmac_sha384_digest
#define hmac_sha512_set_key nettle_hmac_sha512_set_key
#define hmac_sha512_update nettle_hmac_sha512_update
#define hmac_sha512_digest nettle_hmac_sha512_digest
#define hmac_gosthash94_set_key nettle_hmac_gosthash94_set_key
#define hmac_gosthash94_update nettle_hmac_gosthash94_update
#define hmac_gosthash94_digest nettle_hmac_gosthash94_digest
#define hmac_gosthash94cp_set_key nettle_hmac_gosthash94cp_set_key
#define hmac_gosthash94cp_update nettle_hmac_gosthash94cp_update
#define hmac_gosthash94cp_digest nettle_hmac_gosthash94cp_digest
#define hmac_streebog256_set_key nettle_hmac_streebog256_set_key
#define hmac_streebog256_digest nettle_hmac_streebog256_digest
#define hmac_streebog512_set_key nettle_hmac_streebog512_set_key
#define hmac_streebog512_update nettle_hmac_streebog512_update
#define hmac_streebog512_digest nettle_hmac_streebog512_digest
#define hmac_sm3_set_key nettle_hmac_sm3_set_key
#define hmac_sm3_update nettle_hmac_sm3_update
#define hmac_sm3_digest nettle_hmac_sm3_digest

#define _NETTLE_HMAC_CTX(type) {			\
    alignas(type) char outer[offsetof (type, index)];	\
    alignas(type) char inner[offsetof (type, index)];	\
    type state;						\
  }

/* hmac-md5 */
struct hmac_md5_ctx _NETTLE_HMAC_CTX (struct md5_ctx);

void
hmac_md5_set_key(struct hmac_md5_ctx *ctx,
		 size_t key_length, const uint8_t *key);

void
hmac_md5_update(struct hmac_md5_ctx *ctx,
		size_t length, const uint8_t *data);

void
hmac_md5_digest(struct hmac_md5_ctx *ctx,
		uint8_t *digest);


/* hmac-ripemd160 */
struct hmac_ripemd160_ctx _NETTLE_HMAC_CTX (struct ripemd160_ctx);

void
hmac_ripemd160_set_key(struct hmac_ripemd160_ctx *ctx,
		       size_t key_length, const uint8_t *key);

void
hmac_ripemd160_update(struct hmac_ripemd160_ctx *ctx,
		      size_t length, const uint8_t *data);

void
hmac_ripemd160_digest(struct hmac_ripemd160_ctx *ctx,
		      uint8_t *digest);


/* hmac-sha1 */
struct hmac_sha1_ctx _NETTLE_HMAC_CTX (struct sha1_ctx);

void
hmac_sha1_set_key(struct hmac_sha1_ctx *ctx,
		  size_t key_length, const uint8_t *key);

void
hmac_sha1_update(struct hmac_sha1_ctx *ctx,
		 size_t length, const uint8_t *data);

void
hmac_sha1_digest(struct hmac_sha1_ctx *ctx,
		 uint8_t *digest);

/* hmac-sha256 */
struct hmac_sha256_ctx _NETTLE_HMAC_CTX (struct sha256_ctx);

void
hmac_sha256_set_key(struct hmac_sha256_ctx *ctx,
		    size_t key_length, const uint8_t *key);

void
hmac_sha256_update(struct hmac_sha256_ctx *ctx,
		   size_t length, const uint8_t *data);

void
hmac_sha256_digest(struct hmac_sha256_ctx *ctx,
		   uint8_t *digest);

/* hmac-sha224 */
#define hmac_sha224_ctx hmac_sha256_ctx

void
hmac_sha224_set_key(struct hmac_sha224_ctx *ctx,
		    size_t key_length, const uint8_t *key);

#define hmac_sha224_update nettle_hmac_sha256_update

void
hmac_sha224_digest(struct hmac_sha224_ctx *ctx,
		   uint8_t *digest);

/* hmac-sha512 */
struct hmac_sha512_ctx _NETTLE_HMAC_CTX (struct sha512_ctx);

void
hmac_sha512_set_key(struct hmac_sha512_ctx *ctx,
		    size_t key_length, const uint8_t *key);

void
hmac_sha512_update(struct hmac_sha512_ctx *ctx,
		   size_t length, const uint8_t *data);

void
hmac_sha512_digest(struct hmac_sha512_ctx *ctx,
		   uint8_t *digest);

/* hmac-sha384 */
#define hmac_sha384_ctx hmac_sha512_ctx

void
hmac_sha384_set_key(struct hmac_sha512_ctx *ctx,
		    size_t key_length, const uint8_t *key);

#define hmac_sha384_update nettle_hmac_sha512_update

void
hmac_sha384_digest(struct hmac_sha512_ctx *ctx,
		   uint8_t *digest);

/* hmac-gosthash94 */
struct hmac_gosthash94_ctx _NETTLE_HMAC_CTX (struct gosthash94_ctx);

void
hmac_gosthash94_set_key(struct hmac_gosthash94_ctx *ctx,
			size_t key_length, const uint8_t *key);

void
hmac_gosthash94_update(struct hmac_gosthash94_ctx *ctx,
		       size_t length, const uint8_t *data);

  void
hmac_gosthash94_digest(struct hmac_gosthash94_ctx *ctx,
		       uint8_t *digest);

#define hmac_gosthash94cp_ctx hmac_gosthash94_ctx

void
hmac_gosthash94cp_set_key(struct hmac_gosthash94cp_ctx *ctx,
			  size_t key_length, const uint8_t *key);

void
hmac_gosthash94cp_update(struct hmac_gosthash94cp_ctx *ctx,
			 size_t length, const uint8_t *data);

void
hmac_gosthash94cp_digest(struct hmac_gosthash94cp_ctx *ctx,
			 uint8_t *digest);


/* hmac-streebog */
struct hmac_streebog512_ctx _NETTLE_HMAC_CTX (struct streebog512_ctx);

void
hmac_streebog512_set_key(struct hmac_streebog512_ctx *ctx,
		    size_t key_length, const uint8_t *key);

void
hmac_streebog512_update(struct hmac_streebog512_ctx *ctx,
		   size_t length, const uint8_t *data);

void
hmac_streebog512_digest(struct hmac_streebog512_ctx *ctx,
			uint8_t *digest);

#define hmac_streebog256_ctx hmac_streebog512_ctx

void
hmac_streebog256_set_key(struct hmac_streebog256_ctx *ctx,
		    size_t key_length, const uint8_t *key);

#define hmac_streebog256_update hmac_streebog512_update

void
hmac_streebog256_digest(struct hmac_streebog256_ctx *ctx,
			uint8_t *digest);

/* hmac-sm3 */
struct hmac_sm3_ctx _NETTLE_HMAC_CTX (struct sm3_ctx);

void
hmac_sm3_set_key(struct hmac_sm3_ctx *ctx,
		 size_t key_length, const uint8_t *key);

void
hmac_sm3_update(struct hmac_sm3_ctx *ctx,
		size_t length, const uint8_t *data);

void
hmac_sm3_digest(struct hmac_sm3_ctx *ctx,
		uint8_t *digest);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_HMAC_H_INCLUDED */
