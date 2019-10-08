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

#include "mkfs.ubifs.h"
#include "common.h"

#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/cms.h>
#include <openssl/conf.h>
#include <err.h>

static struct ubifs_info *c = &info_;

EVP_MD_CTX *hash_md;
const EVP_MD *md;

int authenticated(void)
{
	return c->hash_algo_name != NULL;
}

static int match_string(const char * const *array, size_t n, const char *string)
{
	int index;
	const char *item;

	for (index = 0; index < n; index++) {
		item = array[index];
		if (!item)
			break;
		if (!strcmp(item, string))
			return index;
	}

	return -EINVAL;
}

#include <linux/hash_info.h>

const char *const hash_algo_name[HASH_ALGO__LAST] = {
	[HASH_ALGO_MD4]		= "md4",
	[HASH_ALGO_MD5]		= "md5",
	[HASH_ALGO_SHA1]	= "sha1",
	[HASH_ALGO_RIPE_MD_160]	= "rmd160",
	[HASH_ALGO_SHA256]	= "sha256",
	[HASH_ALGO_SHA384]	= "sha384",
	[HASH_ALGO_SHA512]	= "sha512",
	[HASH_ALGO_SHA224]	= "sha224",
	[HASH_ALGO_RIPE_MD_128]	= "rmd128",
	[HASH_ALGO_RIPE_MD_256]	= "rmd256",
	[HASH_ALGO_RIPE_MD_320]	= "rmd320",
	[HASH_ALGO_WP_256]	= "wp256",
	[HASH_ALGO_WP_384]	= "wp384",
	[HASH_ALGO_WP_512]	= "wp512",
	[HASH_ALGO_TGR_128]	= "tgr128",
	[HASH_ALGO_TGR_160]	= "tgr160",
	[HASH_ALGO_TGR_192]	= "tgr192",
	[HASH_ALGO_SM3_256]	= "sm3-256",
};

static void display_openssl_errors(int l)
{
	const char *file;
	char buf[120];
	int e, line;

	if (ERR_peek_error() == 0)
		return;
	fprintf(stderr, "At main.c:%d:\n", l);

	while ((e = ERR_get_error_line(&file, &line))) {
		ERR_error_string(e, buf);
		fprintf(stderr, "- SSL %s: %s:%d\n", buf, file, line);
	}
}

static void drain_openssl_errors(void)
{
	const char *file;
	int line;

	if (ERR_peek_error() == 0)
		return;
	while (ERR_get_error_line(&file, &line)) {}
}

#define ssl_err_msg(fmt, ...) ({			\
	display_openssl_errors(__LINE__);		\
	err_msg(fmt, ## __VA_ARGS__);			\
	-1;						\
})

static const char *key_pass;

static int pem_pw_cb(char *buf, int len, __attribute__((unused)) int w,
		     __attribute__((unused)) void *v)
{
	int pwlen;

	if (!key_pass)
		return -1;

	pwlen = strlen(key_pass);
	if (pwlen >= len)
		return -1;

	strcpy(buf, key_pass);

	/* If it's wrong, don't keep trying it. */
	key_pass = NULL;

	return pwlen;
}

static EVP_PKEY *read_private_key(const char *private_key_name, X509 **cert)
{
	EVP_PKEY *private_key = NULL;
	int err;

	*cert = NULL;

	if (!strncmp(private_key_name, "pkcs11:", 7)) {
		ENGINE *e;
		struct {
			const char *url;
			X509 *cert;
		} parms = {
			.url = private_key_name,
		};

		ENGINE_load_builtin_engines();
		drain_openssl_errors();
		e = ENGINE_by_id("pkcs11");
		if (!e) {
			ssl_err_msg("Load PKCS#11 ENGINE");
			return NULL;
		}

		if (ENGINE_init(e)) {
			drain_openssl_errors();
		} else {
			ssl_err_msg("ENGINE_init");
			return NULL;
		}

		if (key_pass)
			if (!ENGINE_ctrl_cmd_string(e, "PIN", key_pass, 0)) {
				ssl_err_msg("Set PKCS#11 PIN");
				return NULL;
			}

		private_key = ENGINE_load_private_key(e, private_key_name,
						      NULL, NULL);

		err = ENGINE_ctrl_cmd(e, "LOAD_CERT_CTRL", 0, &parms, NULL, 0);
		if (!err || !parms.cert) {
			ssl_err_msg("Load certificate");
		}
		*cert = parms.cert;
		fprintf(stderr, "Using cert %p\n", *cert);
	} else {
		BIO *b;

		b = BIO_new_file(private_key_name, "rb");
		if (!b)
			goto out;

		private_key = PEM_read_bio_PrivateKey(b, NULL, pem_pw_cb,
						      NULL);
		BIO_free(b);
	}
out:
	if (!private_key)
		ssl_err_msg("failed opening private key %s", private_key_name);

	return private_key;
}

static X509 *read_x509(const char *x509_name)
{
	unsigned char buf[2];
	X509 *x509 = NULL;
	BIO *b;
	int n;

	b = BIO_new_file(x509_name, "rb");
	if (!b)
		goto out;

	/* Look at the first two bytes of the file to determine the encoding */
	n = BIO_read(b, buf, 2);
	if (n != 2) {
		if (BIO_should_retry(b))
			err_msg("%s: Read wanted retry", x509_name);
		if (n >= 0)
			err_msg("%s: Short read", x509_name);
		goto out;
	}

	if (BIO_reset(b))
		goto out;

	if (buf[0] == 0x30 && buf[1] >= 0x81 && buf[1] <= 0x84)
		/* Assume raw DER encoded X.509 */
		x509 = d2i_X509_bio(b, NULL);
	else
		/* Assume PEM encoded X.509 */
		x509 = PEM_read_bio_X509(b, NULL, NULL, NULL);

	BIO_free(b);

out:
	if (!x509) {
		ssl_err_msg("%s", x509_name);
		return NULL;
	}

	return x509;
}

int sign_superblock_node(void *node)
{
	EVP_PKEY *private_key;
	CMS_ContentInfo *cms = NULL;
	X509 *cert = NULL;
	BIO *bd, *bm;
	void *obuf;
	long len;
	int ret;
	void *pret;
	struct ubifs_sig_node *sig = node + UBIFS_SB_NODE_SZ;

	if (!authenticated())
		return 0;

	ERR_load_crypto_strings();
	ERR_clear_error();

	key_pass = getenv("MKFS_UBIFS_SIGN_PIN");

	bm = BIO_new_mem_buf(node, UBIFS_SB_NODE_SZ);

	private_key = read_private_key(c->auth_key_filename, &cert);
	if (!private_key)
		return -1;

	if (!cert) {
		if (!c->auth_cert_filename)
			return err_msg("authentication certificate not provided (--auth-cert)");
		cert = read_x509(c->auth_cert_filename);
	}

	if (!cert)
		return -1;

	OpenSSL_add_all_digests();
	display_openssl_errors(__LINE__);

	cms = CMS_sign(NULL, NULL, NULL, NULL,
		       CMS_NOCERTS | CMS_PARTIAL | CMS_BINARY |
		       CMS_DETACHED | CMS_STREAM);
	if (!cms)
		return err_msg("CMS_sign failed");

	pret = CMS_add1_signer(cms, cert, private_key, md,
			      CMS_NOCERTS | CMS_BINARY |
			      CMS_NOSMIMECAP | CMS_NOATTR);
	if (!pret)
		return err_msg("CMS_add1_signer failed");

	ret = CMS_final(cms, bm, NULL, CMS_NOCERTS | CMS_BINARY);
	if (!ret)
		return err_msg("CMS_final failed");

	bd = BIO_new(BIO_s_mem());

	ret = i2d_CMS_bio_stream(bd, cms, NULL, 0);
	if (!ret)
		return err_msg("i2d_CMS_bio_stream failed");

	len = BIO_get_mem_data(bd, &obuf);

	sig->type = UBIFS_SIGNATURE_TYPE_PKCS7;
	sig->len = cpu_to_le32(len);
	sig->ch.node_type  = UBIFS_SIG_NODE;

	memcpy(sig + 1, obuf, len);

	BIO_free(bd);
	BIO_free(bm);

	return 0;
}

/**
 * ubifs_node_calc_hash - calculate the hash of a UBIFS node
 * @c: UBIFS file-system description object
 * @node: the node to calculate a hash for
 * @hash: the returned hash
 */
void ubifs_node_calc_hash(const void *node, uint8_t *hash)
{
	const struct ubifs_ch *ch = node;
	unsigned int md_len;

	if (!authenticated())
		return;

	EVP_DigestInit_ex(hash_md, md, NULL);
	EVP_DigestUpdate(hash_md, node, le32_to_cpu(ch->len));
	EVP_DigestFinal_ex(hash_md, hash, &md_len);
}

/**
 * mst_node_calc_hash - calculate the hash of a UBIFS master node
 * @c: UBIFS file-system description object
 * @node: the node to calculate a hash for
 * @hash: the returned hash
 */
void mst_node_calc_hash(const void *node, uint8_t *hash)
{
	unsigned int md_len;

	if (!authenticated())
		return;

	EVP_DigestInit_ex(hash_md, md, NULL);
	EVP_DigestUpdate(hash_md, node + sizeof(struct ubifs_ch),
			 UBIFS_MST_NODE_SZ - sizeof(struct ubifs_ch));
	EVP_DigestFinal_ex(hash_md, hash, &md_len);
}

void hash_digest_init(void)
{
	if (!authenticated())
		return;

	EVP_DigestInit_ex(hash_md, md, NULL);
}

void hash_digest_update(const void *buf, int len)
{
	if (!authenticated())
		return;

	EVP_DigestUpdate(hash_md, buf, len);
}

void hash_digest_final(void *hash, unsigned int *len)
{
	if (!authenticated())
		return;

	EVP_DigestFinal_ex(hash_md, hash, len);
}

int init_authentication(void)
{
	int hash_algo;

	if (!c->auth_key_filename && !c->auth_cert_filename && !c->hash_algo_name)
		return 0;

	if (!c->auth_key_filename)
		return err_msg("authentication key not given (--auth-key)");

	if (!c->hash_algo_name)
		return err_msg("Hash algorithm not given (--hash-algo)");

	OPENSSL_no_config();
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();

	md = EVP_get_digestbyname(c->hash_algo_name);
	if (!md)
		return err_msg("Unknown message digest %s", c->hash_algo_name);

	hash_md = EVP_MD_CTX_create();
	c->hash_len = EVP_MD_size(md);

	hash_algo = match_string(hash_algo_name, HASH_ALGO__LAST, c->hash_algo_name);
	if (hash_algo < 0)
		return err_msg("Unsupported message digest %s", c->hash_algo_name);

	c->hash_algo = hash_algo;

	return 0;
}
