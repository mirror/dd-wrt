/*
 * Copyright (C) 2017 sigma star gmbh
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Authors: David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 */

#ifndef UBIFS_CRYPTO_H
#define UBIFS_CRYPTO_H

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


struct cipher {
	const char *name;
	unsigned int key_length;

	ssize_t (*encrypt_block)(const void *plaintext, size_t size,
				 const void *key, uint64_t block_index,
				 void *ciphertext);

	ssize_t (*encrypt_fname)(const void *plaintext, size_t size,
				 const void *key, void *ciphertext);

	unsigned int fscrypt_block_mode;
	unsigned int fscrypt_fname_mode;
};

#ifdef WITH_CRYPTO
int crypto_init(void);
void crypto_cleanup(void);
ssize_t derive_key_aes(const void *deriving_key, const void *source_key,
		       size_t source_key_len, void *derived_key);
int derive_key_descriptor(const void *source_key, void *descriptor);
struct cipher *get_cipher(const char *name);
void list_ciphers(FILE *fp);
#else
static inline int crypto_init(void) { return 0;}
static inline void crypto_cleanup(void) {}
#endif /* WITH_CRYPTO */

#endif /* UBIFS_CRYPTO_H */

