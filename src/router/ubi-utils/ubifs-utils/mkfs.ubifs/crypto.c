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

#define PROGRAM_NAME "mkfs.ubifs"
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <assert.h>

#include "fscrypt.h"
#include "common.h"

static int do_hash(const EVP_MD *md, const unsigned char *in, size_t len, unsigned char *out)
{
	unsigned int out_len;
	EVP_MD_CTX *mdctx = EVP_MD_CTX_create();

	if (!mdctx)
		return -1;

	if (EVP_DigestInit_ex(mdctx, md, NULL) != 1)
		return -1;

	if(EVP_DigestUpdate(mdctx, in, len) != 1)
		return -1;

	if(EVP_DigestFinal_ex(mdctx, out, &out_len) != 1)
		return -1;

	EVP_MD_CTX_destroy(mdctx);

	return 0;
}

static int check_iv_key_size(const EVP_CIPHER *cipher, size_t key_len,
				size_t iv_len)
{
	if ((size_t)EVP_CIPHER_key_length(cipher) != key_len) {
		errmsg("Cipher key length mismatch. Expected %lu, got %d",
			(unsigned long)key_len, EVP_CIPHER_key_length(cipher));
		return -1;
	}

	if (iv_len && (size_t)EVP_CIPHER_iv_length(cipher) != iv_len) {
		errmsg("Cipher IV length mismatch. Expected %lu, got %d",
			(unsigned long)iv_len, EVP_CIPHER_key_length(cipher));
		return -1;
	}

	return 0;
}

static ssize_t do_encrypt(const EVP_CIPHER *cipher,
				const void *plaintext, size_t size,
				const void *key, size_t key_len,
				const void *iv, size_t iv_len,
				void *ciphertext)
{
	int ciphertext_len, len;
	EVP_CIPHER_CTX *ctx;

	if (check_iv_key_size(cipher, key_len, iv_len))
		return -1;

	if (!(ctx = EVP_CIPHER_CTX_new()))
		goto fail;

	EVP_CIPHER_CTX_set_padding(ctx, 0);

	if (EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv) != 1)
		goto fail_ctx;

	if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, size) != 1)
		goto fail_ctx;

	ciphertext_len = len;

	if (cipher == EVP_aes_256_xts()) {
		if (EVP_EncryptFinal(ctx, ciphertext + ciphertext_len, &len) != 1)
			goto fail_ctx;

		ciphertext_len += len;
	}

	EVP_CIPHER_CTX_free(ctx);
	return ciphertext_len;
fail_ctx:
	ERR_print_errors_fp(stderr);
	EVP_CIPHER_CTX_free(ctx);
	return -1;
fail:
	ERR_print_errors_fp(stderr);
	return -1;
}

static ssize_t gen_essiv_salt(const void *iv, size_t iv_len, const void *key, size_t key_len, void *salt)
{
	size_t ret;
	const EVP_CIPHER *cipher;
	void *sha256 = xzalloc(EVP_MD_size(EVP_sha256()));

	cipher = EVP_aes_256_ecb();
	if (!cipher) {
		errmsg("OpenSSL: Cipher AES-256-ECB is not supported");
		goto fail;
	}

	if (do_hash(EVP_sha256(), key, key_len, sha256) != 0) {
		errmsg("sha256 failed");
		goto fail;
	}

	ret = do_encrypt(cipher, iv, iv_len, sha256, EVP_MD_size(EVP_sha256()), NULL, 0, salt);
	if (ret != iv_len) {
		errmsg("Unable to compute ESSIV salt, return value %zi instead of %zi", ret, iv_len);
		goto fail;
	}

	free(sha256);

	return ret;
fail:
	free(sha256);
	return -1;
}

static ssize_t encrypt_block(const void *plaintext, size_t size,
			     const void *key, uint64_t block_index,
			     void *ciphertext, const EVP_CIPHER *cipher)
{
	size_t key_len, ivsize;
	void *tweak;
	struct {
		uint64_t index;
		uint8_t padding[FS_IV_SIZE - sizeof(uint64_t)];
	} iv;

	ivsize = EVP_CIPHER_iv_length(cipher);
	key_len = EVP_CIPHER_key_length(cipher);

	iv.index = cpu_to_le64(block_index);
	memset(iv.padding, 0, sizeof(iv.padding));

	if (cipher == EVP_aes_128_cbc()) {
		tweak = alloca(ivsize);
		if (gen_essiv_salt(&iv, FS_IV_SIZE, key, key_len, tweak) < 0)
			return -1;
	} else {
		tweak = &iv;
	}

	return do_encrypt(cipher, plaintext, size, key, key_len, tweak,
			  ivsize, ciphertext);
}

static ssize_t encrypt_block_aes128_cbc(const void *plaintext, size_t size,
					const void *key, uint64_t block_index,
					void *ciphertext)
{
	const EVP_CIPHER *cipher = EVP_aes_128_cbc();

	if (!cipher) {
		errmsg("OpenSSL: Cipher AES-128-CBC is not supported");
		return -1;
	}
	return encrypt_block(plaintext, size, key, block_index,
			     ciphertext, cipher);
}

static ssize_t encrypt_block_aes256_xts(const void *plaintext, size_t size,
					const void *key, uint64_t block_index,
					void *ciphertext)
{
	const EVP_CIPHER *cipher = EVP_aes_256_xts();

	if (!cipher) {
		errmsg("OpenSSL: Cipher AES-256-XTS is not supported");
		return -1;
	}
	return encrypt_block(plaintext, size, key, block_index,
			     ciphertext, cipher);
}

static void block_swap(uint8_t *ciphertext, size_t i0, size_t i1,
			size_t size)
{
	uint8_t temp[size], *p0, *p1;

	p0 = ciphertext + i0 * size;
	p1 = ciphertext + i1 * size;

	memcpy(temp, p0, size);
	memcpy(p0, p1, size);
	memcpy(p1, temp, size);
}

static ssize_t encrypt_cbc_cts(const void *plaintext, size_t size,
			       const void *key, void *ciphertext,
			       const EVP_CIPHER *cipher)
{
	size_t diff, padded_size, count, ivsize;
	uint8_t iv[EVP_MAX_IV_LENGTH], *padded;
	ssize_t ret, key_len;

	key_len = EVP_CIPHER_key_length(cipher);
	ivsize = EVP_CIPHER_iv_length(cipher);

	memset(iv, 0, ivsize);

	diff = size % ivsize;

	if (diff) {
		padded_size = size - diff + ivsize;
		padded = size > 256 ? malloc(padded_size) : alloca(padded_size);

		memcpy(padded, plaintext, size);
		memset(padded + size, 0, padded_size - size);

		ret = do_encrypt(cipher, padded, padded_size, key, key_len,
				 iv, ivsize, ciphertext);

		if (size > 256)
			free(padded);
	} else {
		ret = do_encrypt(cipher, plaintext, size, key, key_len,
				 iv, ivsize, ciphertext);
	}

	if (ret < 0)
		return ret;

	count = ret / ivsize;

	if (count > 1)
		block_swap(ciphertext, count - 2, count - 1, ivsize);

	return size;
}

static ssize_t encrypt_aes128_cbc_cts(const void *plaintext, size_t size,
				      const void *key, void *ciphertext)
{
	const EVP_CIPHER *cipher = EVP_aes_128_cbc();
	if (!cipher) {
		errmsg("OpenSSL: Cipher AES-128-CBC is not supported");
		return -1;
	}

	return encrypt_cbc_cts(plaintext, size, key, ciphertext, cipher);
}

static ssize_t encrypt_aes256_cbc_cts(const void *plaintext, size_t size,
				      const void *key, void *ciphertext)
{
	const EVP_CIPHER *cipher = EVP_aes_256_cbc();
	if (!cipher) {
		errmsg("OpenSSL: Cipher AES-256-CBC is not supported");
		return -1;
	}

	return encrypt_cbc_cts(plaintext, size, key, ciphertext, cipher);
}

ssize_t derive_key_aes(const void *deriving_key, const void *source_key,
		       size_t source_key_len, void *derived_key)
{
	const EVP_CIPHER *cipher;
	size_t aes_key_len;

	cipher = EVP_aes_128_ecb();
	if (!cipher) {
		errmsg("OpenSSL: Cipher AES-128-ECB is not supported");
		return -1;
	}
	aes_key_len = EVP_CIPHER_key_length(cipher);

	return do_encrypt(cipher, source_key, source_key_len, deriving_key,
			  aes_key_len, NULL, 0, derived_key);
}

int derive_key_descriptor(const void *source_key, void *descriptor)
{
	int ret = -1;
	void *hash1 = xzalloc(EVP_MD_size(EVP_sha512()));
	void *hash2 = xzalloc(EVP_MD_size(EVP_sha512()));

	if (do_hash(EVP_sha512(), source_key, FS_MAX_KEY_SIZE, hash1) != 0)
		goto out;

	if (do_hash(EVP_sha512(), hash1, EVP_MD_size(EVP_sha512()), hash2) != 0)
		goto out;

	memcpy(descriptor, hash2, FS_KEY_DESCRIPTOR_SIZE);

	ret = 0;
out:
	free(hash1);
	free(hash2);
	return ret;
}

static struct cipher ciphers[] = {
	{
		.name = "AES-128-CBC",
		.key_length = 16,
		.encrypt_block = encrypt_block_aes128_cbc,
		.encrypt_fname = encrypt_aes128_cbc_cts,
		.fscrypt_block_mode = FS_ENCRYPTION_MODE_AES_128_CBC,
		.fscrypt_fname_mode = FS_ENCRYPTION_MODE_AES_128_CTS,
	}, {
		.name = "AES-256-XTS",
		.key_length = 64,
		.encrypt_block = encrypt_block_aes256_xts,
		.encrypt_fname = encrypt_aes256_cbc_cts,
		.fscrypt_block_mode = FS_ENCRYPTION_MODE_AES_256_XTS,
		.fscrypt_fname_mode = FS_ENCRYPTION_MODE_AES_256_CTS,
	}
};

int crypto_init(void)
{
	ERR_load_crypto_strings();
	RAND_poll();
	return 0;
}

void crypto_cleanup(void)
{
	EVP_cleanup();
	ERR_free_strings();
}

struct cipher *get_cipher(const char *name)
{
	size_t i;

	for (i = 0; i < sizeof(ciphers) / sizeof(ciphers[0]); ++i) {
		if (!strcmp(ciphers[i].name, name))
			return ciphers + i;
	}

	return NULL;
}

void list_ciphers(FILE *fp)
{
	size_t i;

	for (i = 0; i < sizeof(ciphers) / sizeof(ciphers[0]); ++i) {
		fprintf(fp, "\t%s\n", ciphers[i].name);
	}
}
