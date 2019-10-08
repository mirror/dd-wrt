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
 * Authors: Richard Weinberger <richard@sigma-star.at>
 *          David Oberhollenzer <david.oberhollenzer@sigma-star.at>
 */

#ifndef FSCRYPT_H
#define FSCRYPT_H


#include "mkfs.ubifs.h"
#include <sys/types.h>
#include "crypto.h"

#ifndef FS_KEY_DESCRIPTOR_SIZE
#define FS_KEY_DESCRIPTOR_SIZE  8
#endif
#define FS_ENCRYPTION_CONTEXT_FORMAT_V1 1
#define FS_KEY_DERIVATION_NONCE_SIZE	16

#ifndef FS_ENCRYPTION_MODE_AES_256_XTS
#define FS_ENCRYPTION_MODE_AES_256_XTS 1
#endif

#ifndef FS_ENCRYPTION_MODE_AES_256_CTS
#define FS_ENCRYPTION_MODE_AES_256_CTS 4
#endif

#ifndef FS_ENCRYPTION_MODE_AES_128_CBC
#define FS_ENCRYPTION_MODE_AES_128_CBC 5
#endif

#ifndef FS_ENCRYPTION_MODE_AES_128_CTS
#define FS_ENCRYPTION_MODE_AES_128_CTS 6
#endif

#ifndef FS_POLICY_FLAGS_VALID
#define FS_POLICY_FLAGS_PAD_4		0x00
#define FS_POLICY_FLAGS_PAD_8		0x01
#define FS_POLICY_FLAGS_PAD_16		0x02
#define FS_POLICY_FLAGS_PAD_32		0x03
#define FS_POLICY_FLAGS_PAD_MASK	0x03
#define FS_POLICY_FLAGS_VALID		0x03
#endif

#define FS_CRYPTO_BLOCK_SIZE	16

/**
 * Encryption context for inode
 *
 * Protector format:
 *  1 byte: Protector format (1 = this version)
 *  1 byte: File contents encryption mode
 *  1 byte: File names encryption mode
 *  1 byte: Flags
 *  8 bytes: Master Key descriptor
 *  16 bytes: Encryption Key derivation nonce
 */
struct fscrypt_context {
	__u8 format;
	__u8 contents_encryption_mode;
	__u8 filenames_encryption_mode;
	__u8 flags;
	__u8 master_key_descriptor[FS_KEY_DESCRIPTOR_SIZE];
	__u8 nonce[FS_KEY_DERIVATION_NONCE_SIZE];
} __attribute__((packed));

/**
 * For encrypted symlinks, the ciphertext length is stored at the beginning
 * of the string in little-endian format.
 */
struct fscrypt_symlink_data {
	__le16 len;
	char encrypted_path[1];
} __attribute__((packed));


#ifndef FS_MAX_KEY_SIZE
#define FS_MAX_KEY_SIZE	64
#endif

#ifndef FS_IV_SIZE
#define FS_IV_SIZE 16
#endif

#ifdef WITH_CRYPTO
unsigned char *calc_fscrypt_subkey(struct fscrypt_context *fctx);
struct fscrypt_context *inherit_fscrypt_context(struct fscrypt_context *fctx);
void free_fscrypt_context(struct fscrypt_context *fctx);
unsigned int fscrypt_fname_encrypted_size(struct fscrypt_context *fctx,
					  unsigned int ilen);
int encrypt_path(void **outbuf, void *data, unsigned int data_len,
		 unsigned int max_namelen, struct fscrypt_context *fctx);
int encrypt_data_node(struct fscrypt_context *fctx, unsigned int block_no,
		      struct ubifs_data_node *dn, size_t length);
struct fscrypt_context *init_fscrypt_context(const char *cipher_name,
					     unsigned int flags,
					     const char *key_file,
					     const char *key_descriptor);
#else
static inline struct fscrypt_context *init_fscrypt_context(
					const char *cipher_name,
					unsigned int flags,
					const char *key_file,
					const char *key_descriptor)
{
	(void)cipher_name;
	(void)flags;
	(void)key_file;
	(void)key_descriptor;

	assert(0);
	return NULL;
}

static inline void free_fscrypt_context(struct fscrypt_context *fctx)
{
	(void)fctx;

	assert(0);
}

static inline int encrypt_path(void **outbuf, void *data, unsigned int data_len,
		 unsigned int max_namelen, struct fscrypt_context *fctx)
{
	(void)outbuf;
	(void)data;
	(void)data_len;
	(void)max_namelen;
	(void)fctx;

	assert(0);
	return -1;
}

static inline int encrypt_data_node(struct fscrypt_context *fctx, unsigned int block_no,
		      struct ubifs_data_node *dn, size_t length)
{
	(void)fctx;
	(void)block_no;
	(void)dn;
	(void)length;

	assert(0);
	return -1;
}

static inline struct fscrypt_context *inherit_fscrypt_context(struct fscrypt_context *fctx)
{
	(void)fctx;

	assert(0);
	return NULL;
}
#endif /* WITH_CRYPTO */
#endif /* FSCRYPT_H */

