/* nettle-types.h

   Copyright (C) 2005, 2014 Niels Möller

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

#ifndef NETTLE_TYPES_H
#define NETTLE_TYPES_H

/* For size_t */
#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>

/* Attributes we want to use in installed header files, and hence
   can't rely on config.h. */
#ifdef __GNUC__

#define _NETTLE_ATTRIBUTE_PURE __attribute__((pure))
#ifndef _NETTLE_ATTRIBUTE_DEPRECATED
/* Variant without message is supported since gcc-3.1 or so. */
#define _NETTLE_ATTRIBUTE_DEPRECATED __attribute__((deprecated))
#endif

#else /* !__GNUC__ */

#define _NETTLE_ATTRIBUTE_PURE
#define _NETTLE_ATTRIBUTE_DEPRECATED

#endif /* !__GNUC__ */

#ifdef __cplusplus
extern "C" {
#endif

/* On 64-bit platforms where uint64_t requires 8 byte alignment, use
   twice the alignment. To work for both C and C++, needs to be placed
   before the type, see example for nettle_block16 below. */
#define _NETTLE_ALIGN16 alignas(alignof(uint64_t) == 8 ? 16 : 0)

/* An aligned 16-byte block. */
union nettle_block16
{
  uint8_t b[16];
  _NETTLE_ALIGN16 uint64_t u64[2];
};

union nettle_block8
{
  uint8_t b[8];
  uint64_t u64;
};


/* Used for generating randomness, as well as for extendable output
   functions like shake. */
typedef void nettle_output_func(void *ctx,
				size_t length, uint8_t *dst);
/* Old name used for key generation and (ec)dsa signature creation. */
typedef nettle_output_func nettle_random_func;

/* Progress report function, mainly for key generation. */
typedef void nettle_progress_func(void *ctx, int c);

/* Realloc function, used by struct nettle_buffer. */
typedef void *nettle_realloc_func(void *ctx, void *p, size_t length);

/* Ciphers */
typedef void nettle_set_key_func(void *ctx, const uint8_t *key);

/* For block ciphers, const context. */
typedef void nettle_cipher_func(const void *ctx,
				size_t length, uint8_t *dst,
				const uint8_t *src);


/* Uses a void * for cipher contexts. Used for crypt operations where
   the internal state changes during the encryption. */
typedef void nettle_crypt_func(void *ctx,
			       size_t length, uint8_t *dst,
			       const uint8_t *src);

/* Hash algorithms */
typedef void nettle_hash_init_func(void *ctx);
typedef void nettle_hash_update_func(void *ctx,
				     size_t length,
				     const uint8_t *src);
typedef void nettle_hash_digest_func(void *ctx, uint8_t *dst);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_TYPES_H */
