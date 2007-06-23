/* cipher.h
 *	Copyright (C) 1998, 2002, 2003 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifndef G10_CIPHER_H
#define G10_CIPHER_H

#include <gcrypt.h>

#define DBG_CIPHER _gcry_get_debug_flag( 1 )

#include "../cipher/random.h"

#define PUBKEY_FLAG_NO_BLINDING (1 << 0)

/*-- rmd160.c --*/
void _gcry_rmd160_hash_buffer (char *outbuf, const char *buffer, size_t length);
/*-- sha1.c --*/
void _gcry_sha1_hash_buffer (char *outbuf, const char *buffer, size_t length);

/*-- dsa.c --*/
void _gcry_register_pk_dsa_progress (gcry_handler_progress_t cbc, void *cb_data);
/*-- elgamal.c --*/
void _gcry_register_pk_elg_progress (gcry_handler_progress_t cb, void *cb_data);
/*-- primegen.c --*/
void _gcry_register_primegen_progress (gcry_handler_progress_t cb, void *cb_data);

/*-- pubkey.c --*/
const char * _gcry_pk_aliased_algo_name (int algorithm);

/* Declarations for the cipher specifications.  */
extern gcry_cipher_spec_t _gcry_cipher_spec_blowfish;
extern gcry_cipher_spec_t _gcry_cipher_spec_des;
extern gcry_cipher_spec_t _gcry_cipher_spec_tripledes;
extern gcry_cipher_spec_t _gcry_cipher_spec_arcfour;
extern gcry_cipher_spec_t _gcry_cipher_spec_cast5;
extern gcry_cipher_spec_t _gcry_cipher_spec_aes;
extern gcry_cipher_spec_t _gcry_cipher_spec_aes192;
extern gcry_cipher_spec_t _gcry_cipher_spec_aes256;
extern gcry_cipher_spec_t _gcry_cipher_spec_twofish;
extern gcry_cipher_spec_t _gcry_cipher_spec_twofish128;
extern gcry_cipher_spec_t _gcry_cipher_spec_serpent128;
extern gcry_cipher_spec_t _gcry_cipher_spec_serpent192;
extern gcry_cipher_spec_t _gcry_cipher_spec_serpent256;
extern gcry_cipher_spec_t _gcry_cipher_spec_rfc2268_40;

/* Declarations for the digest specifications.  */
extern gcry_md_spec_t _gcry_digest_spec_crc32;
extern gcry_md_spec_t _gcry_digest_spec_crc32_rfc1510;
extern gcry_md_spec_t _gcry_digest_spec_crc24_rfc2440;
extern gcry_md_spec_t _gcry_digest_spec_md4;
extern gcry_md_spec_t _gcry_digest_spec_md5;
extern gcry_md_spec_t _gcry_digest_spec_rmd160;
extern gcry_md_spec_t _gcry_digest_spec_sha1;
extern gcry_md_spec_t _gcry_digest_spec_sha256;
extern gcry_md_spec_t _gcry_digest_spec_sha512;
extern gcry_md_spec_t _gcry_digest_spec_sha384;
extern gcry_md_spec_t _gcry_digest_spec_tiger;

/* Declarations for the pubkey cipher specifications.  */
extern gcry_pk_spec_t _gcry_pubkey_spec_rsa;
extern gcry_pk_spec_t _gcry_pubkey_spec_elg;
extern gcry_pk_spec_t _gcry_pubkey_spec_dsa;

#endif /*G10_CIPHER_H*/
