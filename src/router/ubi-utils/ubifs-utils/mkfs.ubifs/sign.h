/*
 * Copyright (C) 2018 Pengutronix
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
 * Author: Sascha Hauer
 */

#ifndef __UBIFS_SIGN_H__
#define __UBIFS_SIGN_H__

#ifdef WITH_CRYPTO
#include <openssl/evp.h>

void ubifs_node_calc_hash(const void *node, uint8_t *hash);
void mst_node_calc_hash(const void *node, uint8_t *hash);
void hash_digest_init(void);
void hash_digest_update(const void *buf, int len);
void hash_digest_final(void *hash, unsigned int *len);
int init_authentication(void);
int sign_superblock_node(void *node);
int authenticated(void);

extern EVP_MD_CTX *hash_md;
extern const EVP_MD *md;

#else
static inline void ubifs_node_calc_hash(__attribute__((unused)) const void *node,
					__attribute__((unused)) uint8_t *hash)
{
}

static inline void mst_node_calc_hash(__attribute__((unused)) const void *node,
				      __attribute__((unused)) uint8_t *hash)
{
}

static inline void hash_digest_init(void)
{
}

static inline void hash_digest_update(__attribute__((unused)) const void *buf,
				      __attribute__((unused)) int len)
{
}

static inline void hash_digest_final(__attribute__((unused)) void *hash,
				     __attribute__((unused)) unsigned int *len)
{
}

static inline int init_authentication(void)
{
	return 0;
}

static inline int sign_superblock_node(__attribute__((unused)) void *node)
{
	return 0;
}

static inline int authenticated(void)
{
	return 0;
}

#endif

#endif /* __UBIFS_SIGN_H__ */
