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

#define PROGRAM_NAME "mkfs.ubifs"
#include "fscrypt.h"


static __u8 fscrypt_masterkey[FS_MAX_KEY_SIZE];
static struct cipher *fscrypt_cipher;


unsigned char *calc_fscrypt_subkey(struct fscrypt_context *fctx)
{
	int ret;
	unsigned char *new_key = xmalloc(FS_MAX_KEY_SIZE);

	ret = derive_key_aes(fctx->nonce, fscrypt_masterkey, fscrypt_cipher->key_length, new_key);
	if (ret < 0) {
		err_msg("derive_key_aes failed: %i\n", ret);

		free(new_key);
		new_key = NULL;
	}

	return new_key;
}

struct fscrypt_context *inherit_fscrypt_context(struct fscrypt_context *fctx)
{
	struct fscrypt_context *new_fctx = NULL;

	if (fctx) {
		new_fctx = xmalloc(sizeof(*new_fctx));
		new_fctx->format = fctx->format;
		new_fctx->contents_encryption_mode = fctx->contents_encryption_mode;
		new_fctx->filenames_encryption_mode = fctx->filenames_encryption_mode;
		new_fctx->flags = fctx->flags;
		memcpy(new_fctx->master_key_descriptor, fctx->master_key_descriptor,
		       FS_KEY_DESCRIPTOR_SIZE);
		RAND_bytes((void *)&new_fctx->nonce, FS_KEY_DERIVATION_NONCE_SIZE);
	}

	return new_fctx;
}

void free_fscrypt_context(struct fscrypt_context *fctx)
{
	free(fctx);
}

static void print_fscrypt_master_key_descriptor(__u8 *master_key_descriptor)
{
	int i;

	normsg_cont("fscrypt master key descriptor: 0x");
	for (i = 0; i < FS_KEY_DESCRIPTOR_SIZE; i++) {
		printf("%02x", master_key_descriptor[i]);
	}
	printf("\n");
}

unsigned int fscrypt_fname_encrypted_size(struct fscrypt_context *fctx,
					  unsigned int ilen)
{
	int padding;

	padding = 4 << (fctx->flags & FS_POLICY_FLAGS_PAD_MASK);
	ilen = max_t(unsigned int, ilen, FS_CRYPTO_BLOCK_SIZE);
	return round_up(ilen, padding);
}

int encrypt_path(void **outbuf, void *data, unsigned int data_len,
		unsigned int max_namelen, struct fscrypt_context *fctx)
{
	void *inbuf, *crypt_key;
	unsigned int padding = 4 << (fctx->flags & FS_POLICY_FLAGS_PAD_MASK);
	unsigned int cryptlen;
	int ret;

	cryptlen = max_t(unsigned int, data_len, FS_CRYPTO_BLOCK_SIZE);
	cryptlen = round_up(cryptlen, padding);
	cryptlen = min(cryptlen, max_namelen);

	inbuf = xmalloc(cryptlen);
	/* CTS mode needs a block size aligned buffer */
	*outbuf = xmalloc(round_up(cryptlen, FS_CRYPTO_BLOCK_SIZE));

	memset(inbuf, 0, cryptlen);
	memcpy(inbuf, data, data_len);

	crypt_key = calc_fscrypt_subkey(fctx);
	if (!crypt_key) {
		free(inbuf);
		free(*outbuf);
		return err_msg("could not compute subkey");
	}

	ret = fscrypt_cipher->encrypt_fname(inbuf, cryptlen,
					    crypt_key, *outbuf);
	if (ret < 0) {
		free(inbuf);
		free(*outbuf);
		return err_msg("could not encrypt filename");
	}

	free(crypt_key);
	free(inbuf);
	return cryptlen;
}

int encrypt_data_node(struct fscrypt_context *fctx, unsigned int block_no,
		      struct ubifs_data_node *dn, size_t length)
{
	void *inbuf, *outbuf, *crypt_key;
	size_t ret, pad_len = round_up(length, FS_CRYPTO_BLOCK_SIZE);

	dn->compr_size = cpu_to_le16(length);

	inbuf = xzalloc(pad_len);
	outbuf = xzalloc(pad_len);

	memcpy(inbuf, &dn->data, length);

	crypt_key = calc_fscrypt_subkey(fctx);
	if (!crypt_key) {
		free(inbuf);
		free(outbuf);
		return err_msg("could not compute subkey");
	}

	ret = fscrypt_cipher->encrypt_block(inbuf, pad_len,
					    crypt_key, block_no,
					    outbuf);
	if (ret != pad_len) {
		free(inbuf);
		free(outbuf);
		free(crypt_key);
		return err_msg("encrypt_block returned %zi "
				"instead of %zi", ret, pad_len);
	}

	memcpy(&dn->data, outbuf, pad_len);

	free(inbuf);
	free(outbuf);
	free(crypt_key);
	return pad_len;
}

static int xdigit(int x)
{
	if (isupper(x))
		return x - 'A' + 0x0A;
	if (islower(x))
		return x - 'a' + 0x0A;
	return x - '0';
}

static int parse_key_descriptor(const char *desc, __u8 *dst)
{
	int i, hi, lo;

	if (desc[0] == '0' && (desc[1] == 'x' || desc[1] == 'X'))
		desc += 2;

	for (i = 0; i < FS_KEY_DESCRIPTOR_SIZE; ++i) {
		if (!desc[i * 2] || !desc[i * 2 + 1]) {
			err_msg("key descriptor '%s' is too short", desc);
			return -1;
		}
		if (!isxdigit(desc[i * 2]) || !isxdigit(desc[i * 2 + 1])) {
			err_msg("invalid key descriptor '%s'", desc);
			return -1;
		}

		hi = xdigit(desc[i * 2]);
		lo = xdigit(desc[i * 2 + 1]);

		dst[i] = (hi << 4) | lo;
	}

	if (desc[i * 2]) {
		err_msg("key descriptor '%s' is too long", desc);
		return -1;
	}
	return 0;
}

static int load_master_key(const char *key_file, struct cipher *fsc)
{
	int kf;
	ssize_t keysize;

	kf = open(key_file, O_RDONLY);
	if (kf < 0) {
		sys_errmsg("open '%s'", key_file);
		return -1;
	}

	keysize = read(kf, fscrypt_masterkey, FS_MAX_KEY_SIZE);
	if (keysize < 0) {
		sys_errmsg("read '%s'", key_file);
		goto fail;
	}
	if (keysize == 0) {
		err_msg("loading key from '%s': file is empty", key_file);
		goto fail;
	}
	if (keysize < fsc->key_length) {
		err_msg("key '%s' is too short (at least %u bytes required)",
			key_file, fsc->key_length);
		goto fail;
	}

	close(kf);
	return 0;
fail:
	close(kf);
	return -1;
}

struct fscrypt_context *init_fscrypt_context(const char *cipher_name,
					     unsigned int flags,
					     const char *key_file,
					     const char *key_descriptor)
{
	__u8 master_key_descriptor[FS_KEY_DESCRIPTOR_SIZE];
	__u8 nonce[FS_KEY_DERIVATION_NONCE_SIZE];
	struct fscrypt_context *new_fctx;

	fscrypt_cipher = get_cipher(cipher_name);

	if (!fscrypt_cipher) {
		fprintf(stderr, "Cannot find cipher '%s'\n"
			"Try `%s --help' for more information\n",
			cipher_name, PROGRAM_NAME);
		return NULL;
	}

	if (load_master_key(key_file, fscrypt_cipher))
		return NULL;

	if (!key_descriptor) {
		if (derive_key_descriptor(fscrypt_masterkey, master_key_descriptor))
			return NULL;
		print_fscrypt_master_key_descriptor(master_key_descriptor);
	} else {
		if (parse_key_descriptor(key_descriptor, master_key_descriptor))
			return NULL;
	}

	RAND_bytes((void *)nonce, FS_KEY_DERIVATION_NONCE_SIZE);

	new_fctx = xmalloc(sizeof(*new_fctx));

	new_fctx->format = FS_ENCRYPTION_CONTEXT_FORMAT_V1;
	new_fctx->contents_encryption_mode = fscrypt_cipher->fscrypt_block_mode;
	new_fctx->filenames_encryption_mode = fscrypt_cipher->fscrypt_fname_mode;
	new_fctx->flags = flags;

	memcpy(&new_fctx->nonce, nonce, FS_KEY_DERIVATION_NONCE_SIZE);
	memcpy(&new_fctx->master_key_descriptor, master_key_descriptor,
		FS_KEY_DESCRIPTOR_SIZE);
	return new_fctx;
}
