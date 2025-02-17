/*
 * Embedded Linux library
 * Copyright (C) 2018  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

#include "util.h"
#include "tls.h"
#include "cipher.h"
#include "checksum.h"
#include "cert.h"
#include "tls-private.h"
#include "key.h"
#include "random.h"
#include "ecc.h"
#include "ecdh.h"
#include "missing.h"

enum signature_algorithm {
	SIGNATURE_ALGORITHM_ANONYMOUS = 0,
	SIGNATURE_ALGORITHM_RSA = 1,
	SIGNATURE_ALGORITHM_DSA = 2,
	SIGNATURE_ALGORITHM_ECDSA = 3,
};

static enum handshake_hash_type find_hash_by_id(uint8_t id)
{
	enum handshake_hash_type hash;

	for (hash = 0; hash < __HANDSHAKE_HASH_COUNT; hash++)
		if (tls_handshake_hash_data[hash].tls_id == id)
			break;

	return hash;
}

/*
 * Sanitize DigitallySigned struct input, making sure the lengths
 * are valid and correspond to what we expect.
 *
 * Returns: start of the opaque portion
 */
static const uint8_t *validate_digitally_signed(struct l_tls *tls,
					const uint8_t *in, size_t in_len,
					enum signature_algorithm expected_alg,
					uint16_t *opaque_len)
{
	size_t offset = 2;
	uint16_t len;

	if (tls->negotiated_version < L_TLS_V12)
		offset = 0;

	if (in_len < offset + 2)
		goto size_error;

	len = l_get_be16(in + offset);
	if (len != in_len - offset - 2)
		goto size_error;

	if (tls->negotiated_version >= L_TLS_V12) {
		if (in[1] != expected_alg) {
			TLS_DISCONNECT(TLS_ALERT_DECRYPT_ERROR, 0,
					"Unknown signature algorithm %i",
					in[1]);

			return NULL;
		}
	}

	*opaque_len = len;
	return in + offset + 2;

size_error:
	TLS_DISCONNECT(TLS_ALERT_DECODE_ERROR, 0, "Signature msg too "
			"short (%zi) or signature length doesn't match",
			in_len);
	return NULL;
}

static bool tls_rsa_validate_cert_key(struct l_cert *cert)
{
	return l_cert_get_pubkey_type(cert) == L_CERT_KEY_RSA;
}

static ssize_t tls_rsa_sign(struct l_tls *tls, uint8_t *out, size_t out_len,
				tls_get_hash_t get_hash,
				const uint8_t *data, size_t data_len)
{
	ssize_t result = -EMSGSIZE;
	enum l_checksum_type sign_checksum_type;
	uint8_t sign_input[HANDSHAKE_HASH_MAX_SIZE + 36];
	size_t sign_input_len;
	uint8_t *ptr = out;

	if (!tls->priv_key || !tls->priv_key_size) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, TLS_ALERT_BAD_CERT,
				"No private key loaded");

		return -ENOKEY;
	}

	if (tls->negotiated_version >= L_TLS_V12) {
		const struct tls_hash_algorithm *hash_type =
			&tls_handshake_hash_data[tls->signature_hash];

		/* Build the DigitallySigned struct */
		if (out_len < 2) /* Is there space for the algorithm IDs */
			goto error;

		get_hash(tls, tls->signature_hash, data, data_len,
				sign_input, &sign_input_len);
		sign_checksum_type = hash_type->l_id;

		*ptr++ = hash_type->tls_id;
		*ptr++ = 1;	/* RSA_sign */
		out_len -= 2;
	} else {
		get_hash(tls, HANDSHAKE_HASH_MD5, data, data_len,
				sign_input + 0, NULL);
		get_hash(tls, HANDSHAKE_HASH_SHA1, data, data_len,
				sign_input + 16, NULL);
		sign_checksum_type = L_CHECKSUM_NONE;
		sign_input_len = 36;
	}

	if (out_len < tls->priv_key_size + 2)
		goto error;

	l_put_be16(tls->priv_key_size, ptr);
	result = l_key_sign(tls->priv_key, L_KEY_RSA_PKCS1_V1_5,
				sign_checksum_type, sign_input, ptr + 2,
				sign_input_len, tls->priv_key_size);
	ptr += tls->priv_key_size + 2;

	if (result == (ssize_t) tls->priv_key_size)
		return ptr - out; /* Success */

error:
	TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
			"Signing the hash failed: %s",
			strerror(-result));
	return result;
}

static bool tls_rsa_verify(struct l_tls *tls, const uint8_t *in, size_t in_len,
				tls_get_hash_t get_hash,
				const uint8_t *data, size_t data_len)
{
	enum l_checksum_type sign_checksum_type;
	uint8_t expected[HANDSHAKE_HASH_MAX_SIZE + 36];
	size_t expected_len;
	const uint8_t *opaque;
	uint16_t opaque_len;
	bool success;

	opaque = validate_digitally_signed(tls, in, in_len,
				SIGNATURE_ALGORITHM_RSA, &opaque_len);
	if (!opaque)
		return false;

	/* Only the default hash type supported */
	if (opaque_len != tls->peer_pubkey_size) {
		TLS_DISCONNECT(TLS_ALERT_DECODE_ERROR, 0,
				"Signature length %hu not equal %zi",
				opaque_len, tls->peer_pubkey_size);

		return false;
	}

	if (tls->negotiated_version >= L_TLS_V12) {
		enum handshake_hash_type hash = find_hash_by_id(in[0]);

		if (hash == __HANDSHAKE_HASH_COUNT) {
			TLS_DISCONNECT(TLS_ALERT_DECRYPT_ERROR, 0,
					"Unknown hash type %i", in[0]);
			return false;
		}

		get_hash(tls, hash, data, data_len, expected, &expected_len);
		sign_checksum_type = tls_handshake_hash_data[hash].l_id;

		/*
		 * Note: Next we let the l_key_verify's underlying kernel
		 * operation prepend the OID to the hash to build the
		 * DigestInfo struct.  However according to 4.7 we need to
		 * support at least two forms of the signed content in the
		 * verification:
		 *  - DigestInfo with NULL AlgorithmIdentifier.parameters,
		 *  - DigestInfo with empty AlgorithmIdentifier.parameters,
		 *
		 * while the kernel only understands the former encoding.
		 * Note PKCS#1 versions 2.0 and later section A.2.4 do
		 * mandate NULL AlgorithmIdentifier.parameters.
		 *
		 * Additionally PKCS#1 v1.5 said BER is used in place of DER
		 * for DigestInfo encoding which adds more ambiguity in the
		 * encoding.
		 */
	} else {
		get_hash(tls, HANDSHAKE_HASH_MD5, data, data_len,
				expected + 0, NULL);
		get_hash(tls, HANDSHAKE_HASH_SHA1, data, data_len,
				expected + 16, NULL);
		expected_len = 36;
		sign_checksum_type = L_CHECKSUM_NONE;

		/*
		 * Note: Within the RSA padding for signatures PKCS#1 1.5
		 * allows the block format to be either 0 or 1, while PKCS#1
		 * v2.0+ mandates block type 1 making the signatures
		 * unambiguous.  TLS 1.0 doesn't additionally specify which
		 * block type is to be used (TLS 1.2 does) meaning that both
		 * PKCS#1 v1.5 types are allowed.  The l_key_verify's
		 * underlying kernel implementation only accepts block type
		 * 1.  If this ever becomes an issue we'd need to go back to
		 * using L_KEY_RSA_RAW and our own PKCS#1 v1.5 verify logic.
		 */
	}

	success = l_key_verify(tls->peer_pubkey, L_KEY_RSA_PKCS1_V1_5,
				sign_checksum_type, expected, opaque,
				expected_len, tls->peer_pubkey_size);

	if (!success)
		TLS_DISCONNECT(TLS_ALERT_DECRYPT_ERROR, 0,
				"Peer signature verification failed");
	else
		TLS_DEBUG("Peer signature verified");

	return success;
}

static struct tls_signature_algorithm tls_rsa_signature = {
	.id = 1, /* SignatureAlgorithm.rsa */
	.validate_cert_key_type = tls_rsa_validate_cert_key,
	.sign = tls_rsa_sign,
	.verify = tls_rsa_verify,
};

static bool tls_ecdsa_validate_cert_key(struct l_cert *cert)
{
	return l_cert_get_pubkey_type(cert) == L_CERT_KEY_ECC;
}

static bool tls_ecdsa_verify(struct l_tls *tls,
				const uint8_t *in, size_t in_len,
				tls_get_hash_t get_hash,
				const uint8_t *data, size_t data_len)
{
	/* RFC 8422, Section 5.10: "SHA-1 is used in TLS 1.1 and earlier" */
	enum handshake_hash_type hash = HANDSHAKE_HASH_SHA1;
	enum l_checksum_type sign_checksum_type;
	const uint8_t *opaque;
	uint16_t opaque_len;
	uint8_t expected[HANDSHAKE_HASH_MAX_SIZE];
	size_t expected_len;
	bool success;

	opaque = validate_digitally_signed(tls, in, in_len,
				SIGNATURE_ALGORITHM_ECDSA, &opaque_len);
	if (!opaque)
		return false;

	if (tls->negotiated_version >= L_TLS_V12) {
		hash = find_hash_by_id(in[0]);
		if (hash == __HANDSHAKE_HASH_COUNT) {
			TLS_DISCONNECT(TLS_ALERT_DECRYPT_ERROR, 0,
					"Unknown hash type %i", in[0]);
			return false;
		}

		/* Hash should match the curve, refer to RFC 5480, Section 4 */
		switch (tls->peer_pubkey_size) {
		case 32:
			if (hash != HANDSHAKE_HASH_SHA256 &&
					hash != HANDSHAKE_HASH_SHA384)
				goto bad_hash;

			break;
		case 48:
			if (hash != HANDSHAKE_HASH_SHA384)
				goto bad_hash;

			break;
		bad_hash:
		default:
			TLS_DISCONNECT(TLS_ALERT_DECRYPT_ERROR, 0,
					"Invalid hash %i",
					in[0]);
		}
	}

	get_hash(tls, hash, data, data_len, expected, &expected_len);
	sign_checksum_type = tls_handshake_hash_data[hash].l_id;

	success = l_key_verify(tls->peer_pubkey, L_KEY_ECDSA_X962,
				sign_checksum_type, expected, opaque,
				expected_len, opaque_len);

	if (!success)
		TLS_DISCONNECT(TLS_ALERT_DECRYPT_ERROR, 0,
				"Peer signature verification failed");
	else
		TLS_DEBUG("Peer signature verified");

	return success;
}

static struct tls_signature_algorithm tls_ecdsa_signature = {
	.id = 3, /* SignatureAlgorithm.ecdsa */
	.validate_cert_key_type = tls_ecdsa_validate_cert_key,
	.verify = tls_ecdsa_verify,
};

static bool tls_send_rsa_client_key_xchg(struct l_tls *tls)
{
	uint8_t buf[1024 + 32];
	uint8_t *ptr = buf + TLS_HANDSHAKE_HEADER_SIZE;
	uint8_t pre_master_secret[48];
	ssize_t bytes_encrypted;

	if (!tls->peer_pubkey) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"Peer public key not received");

		return false;
	}

	/* Must match the version in tls_send_client_hello */
	pre_master_secret[0] = (uint8_t) (tls->client_version >> 8);
	pre_master_secret[1] = (uint8_t) (tls->client_version >> 0);

	l_getrandom(pre_master_secret + 2, 46);

	if (tls->peer_pubkey_size + 32 > (int) sizeof(buf)) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"Peer public key too big: %zi",
				tls->peer_pubkey_size);

		return false;
	}

	l_put_be16(tls->peer_pubkey_size, ptr);
	bytes_encrypted = l_key_encrypt(tls->peer_pubkey,
					L_KEY_RSA_PKCS1_V1_5, L_CHECKSUM_NONE,
					pre_master_secret, ptr + 2, 48,
					tls->peer_pubkey_size);
	ptr += tls->peer_pubkey_size + 2;

	if (bytes_encrypted != (ssize_t) tls->peer_pubkey_size) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"Encrypting PreMasterSecret failed: %s",
				strerror(-bytes_encrypted));

		return false;
	}

	tls_tx_handshake(tls, TLS_CLIENT_KEY_EXCHANGE, buf, ptr - buf);

	tls_generate_master_secret(tls, pre_master_secret, 48);
	explicit_bzero(pre_master_secret, 48);

	return true;
}

static void tls_handle_rsa_client_key_xchg(struct l_tls *tls,
						const uint8_t *buf, size_t len)
{
	uint8_t pre_master_secret[48], random_secret[46];
	ssize_t bytes_decrypted;

	if (!tls->priv_key || !tls->priv_key_size) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, TLS_ALERT_BAD_CERT,
				"No private key");

		return;
	}

	if (len != tls->priv_key_size + 2) {
		TLS_DISCONNECT(TLS_ALERT_DECODE_ERROR, 0,
				"ClientKeyExchange len %zi not %zi", len,
				tls->priv_key_size + 2);

		return;
	}

	len = l_get_be16(buf);

	if (len != tls->priv_key_size) {
		TLS_DISCONNECT(TLS_ALERT_DECODE_ERROR, 0,
				"EncryptedPreMasterSecret len %zi not %zi",
				len, tls->priv_key_size);

		return;
	}

	bytes_decrypted = l_key_decrypt(tls->priv_key, L_KEY_RSA_PKCS1_V1_5,
					L_CHECKSUM_NONE, buf + 2,
					pre_master_secret, tls->priv_key_size,
					48);

	/*
	 * Assume correct premaster secret client version which according
	 * to the TLS1.2 spec is unlikely in client implementations SSLv3
	 * and prior.  Spec suggests either not supporting them or adding
	 * a configurable override for <= SSLv3 clients.  For now we have
	 * no need to support them.
	 *
	 * On any decode error randomise the Pre Master Secret as per the
	 * countermeasures in 7.4.7.1 and don't generate any alerts.
	 */
	l_getrandom(random_secret, 46);

	pre_master_secret[0] = tls->client_version >> 8;
	pre_master_secret[1] = tls->client_version >> 0;

	if (bytes_decrypted != 48) {
		memcpy(pre_master_secret + 2, random_secret, 46);

		TLS_DEBUG("Error decrypting PreMasterSecret: %s",
				strerror(-bytes_decrypted));
	}

	tls_generate_master_secret(tls, pre_master_secret, 48);
	explicit_bzero(pre_master_secret, 48);
	explicit_bzero(random_secret, 46);
}

static struct tls_key_exchange_algorithm tls_rsa_key_xchg = {
	.send_client_key_exchange = tls_send_rsa_client_key_xchg,
	.handle_client_key_exchange = tls_handle_rsa_client_key_xchg,
};

/* Used by both DHE and ECDHE */
static bool tls_get_dh_params_hash(struct l_tls *tls,
					enum handshake_hash_type type,
					const uint8_t *data, size_t data_len,
					uint8_t *out, size_t *out_len)
{
	struct l_checksum *checksum;
	ssize_t ret;

	/*
	 * The ServerKeyExchange signature hash input format for RSA_sign is
	 * not really specified in either RFC 8422 or RFC 5246 explicitly
	 * but we use this format by analogy to DHE_RSA which uses RSA_sign
	 * as well.  Also matches ecdsa, ed25519 and ed448 formats.
	 */
	struct iovec iov[] = {
		{ .iov_base = tls->pending.client_random, .iov_len = 32 },
		{ .iov_base = tls->pending.server_random, .iov_len = 32 },
		{ .iov_base = (void *) data, .iov_len = data_len },
	};

	checksum = l_checksum_new(tls_handshake_hash_data[type].l_id);
	if (!checksum)
		return false;

	l_checksum_updatev(checksum, iov, L_ARRAY_SIZE(iov));
	ret = l_checksum_get_digest(checksum, out, HANDSHAKE_HASH_MAX_SIZE);
	l_checksum_free(checksum);

	if (ret < 0)
		return false;

	if (out_len)
		*out_len = ret;

	return true;
}

struct tls_ecdhe_params {
	const struct l_ecc_curve *curve;
	struct l_ecc_scalar *private;
	struct l_ecc_point *public;
};

static void tls_free_ecdhe_params(struct l_tls *tls)
{
	struct tls_ecdhe_params *params = tls->pending.key_xchg_params;

	if (!params)
		return;

	tls->pending.key_xchg_params = NULL;

	l_ecc_scalar_free(params->private);
	l_ecc_point_free(params->public);
	l_free(params);
}

static size_t tls_write_ecpoint(uint8_t *buf, size_t len,
				const struct tls_named_group *curve,
				const struct l_ecc_point *point)
{
	size_t point_bytes;

	/* RFC 8422, Section 5.4.1 */
	point_bytes = l_ecc_point_get_data(point, buf + 2, len - 2);
	buf[0] = 1 + point_bytes;		/* length */
	buf[1] = 4;				/* form: uncompressed */
	return 2 + point_bytes;
}

static size_t tls_write_server_ecdh_params(struct l_tls *tls, uint8_t *buf, size_t len)
{
	struct tls_ecdhe_params *params = tls->pending.key_xchg_params;

	/* RFC 8422, Section 5.4 */
	buf[0] = 3;				/* curve_type: named_curve */
	l_put_be16(tls->negotiated_curve->id, buf + 1);
	return 3 + tls_write_ecpoint(buf + 3, len - 3, tls->negotiated_curve,
					params->public);
}

static bool tls_send_ecdhe_server_key_xchg(struct l_tls *tls)
{
	uint8_t buf[1024];
	uint8_t *ptr = buf + TLS_HANDSHAKE_HEADER_SIZE;
	struct tls_ecdhe_params *params;
	ssize_t sign_len;
	const uint8_t *server_ecdh_params_ptr;

	/*
	 * RFC 8422, Section 5.4
	 *
	 * If we're getting here we can assume that tls->pending.key_xchg_params
	 * is NULL, tls->priv_key is our signing key and tls->negotiated_curve
	 * is non-NULL.
	 */

	params = l_new(struct tls_ecdhe_params, 1);
	params->curve = l_ecc_curve_from_tls_group(tls->negotiated_curve->id);
	tls->pending.key_xchg_params = params;

	if (!l_ecdh_generate_key_pair(params->curve,
					&params->private, &params->public)) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"Generating ECDH key pair failed");
		return false;
	}

	server_ecdh_params_ptr = ptr;
	ptr += tls_write_server_ecdh_params(tls, ptr, buf + sizeof(buf) - ptr);

	if (tls->pending.cipher_suite->signature) {
		sign_len = tls->pending.cipher_suite->signature->sign(tls, ptr,
						buf + sizeof(buf) - ptr,
						tls_get_dh_params_hash,
						server_ecdh_params_ptr,
						ptr - server_ecdh_params_ptr);
		if (sign_len < 0)
			return false;

		ptr += sign_len;
	}

	tls_tx_handshake(tls, TLS_SERVER_KEY_EXCHANGE, buf, ptr - buf);

	return true;
}

static void tls_handle_ecdhe_server_key_xchg(struct l_tls *tls,
						const uint8_t *buf, size_t len)
{
	struct tls_ecdhe_params *params;
	uint16_t namedcurve;
	const uint8_t *server_ecdh_params_ptr = buf;
	size_t point_bytes;

	/* RFC 8422, Section 5.4 */

	if (len < 5)
		goto decode_error;

	if (*buf != 3) {
		TLS_DISCONNECT(TLS_ALERT_ILLEGAL_PARAM, 0,
				"Unsupported (deprecated?) ECCurveType %u",
				*buf);
		return;
	}

	namedcurve = l_get_be16(buf + 1);
	buf += 3;
	len -= 3;

	tls->negotiated_curve = tls_find_group(namedcurve);

	if (!tls->negotiated_curve ||
			tls->negotiated_curve->type != TLS_GROUP_TYPE_EC) {
		TLS_DISCONNECT(TLS_ALERT_ILLEGAL_PARAM, 0,
				"Unsupported NamedCurve %u", namedcurve);
		return;
	}

	TLS_DEBUG("Negotiated %s", tls->negotiated_curve->name);

	if (*buf < 1)
		goto decode_error;

	point_bytes = *buf++ - 1;

	if (*buf != 4) {	/* uncompressed */
		TLS_DISCONNECT(TLS_ALERT_ILLEGAL_PARAM, 0,
				"Unsupported (deprecated?) PointConversionForm "
				"%u", *buf);
		return;
	}

	buf++;
	len -= 2;

	if (len < point_bytes)
		goto decode_error;

	/*
	 * RFC 8422, Section 5.11: "A receiving party MUST check that the
	 * x and y parameters from the peer's public value satisfy the
	 * curve equation, y^2 = x^3 + ax + b mod p."
	 * This happens in l_ecc_point_from_data when the L_ECC_POINT_TYPE_FULL
	 * format is used.
	 */
	params = l_new(struct tls_ecdhe_params, 1);
	params->curve = l_ecc_curve_from_tls_group(tls->negotiated_curve->id);
	params->public = l_ecc_point_from_data(params->curve,
						L_ECC_POINT_TYPE_FULL,
						buf, point_bytes);
	tls->pending.key_xchg_params = params;
	buf += point_bytes;
	len -= point_bytes;

	if (!params->public || point_bytes !=
			2 * l_ecc_curve_get_scalar_bytes(params->curve)) {
		TLS_DISCONNECT(TLS_ALERT_DECODE_ERROR, 0,
				"ServerKeyExchange.params.public decode error");
		return;
	}

	if (tls->pending.cipher_suite->signature) {
		if (!tls->pending.cipher_suite->signature->verify(tls, buf, len,
						tls_get_dh_params_hash,
						server_ecdh_params_ptr,
						buf - server_ecdh_params_ptr))
			return;
	} else {
		if (len)
			goto decode_error;
	}

	TLS_SET_STATE(TLS_HANDSHAKE_WAIT_HELLO_DONE);

	return;

decode_error:
	TLS_DISCONNECT(TLS_ALERT_DECODE_ERROR, 0,
			"ServerKeyExchange decode error");
}

static bool tls_send_ecdhe_client_key_xchg(struct l_tls *tls)
{
	uint8_t buf[1024];
	uint8_t *ptr = buf + TLS_HANDSHAKE_HEADER_SIZE;
	uint8_t pre_master_secret[128];
	ssize_t pre_master_secret_len;
	struct tls_ecdhe_params *params = tls->pending.key_xchg_params;
	struct l_ecc_point *our_public;
	struct l_ecc_scalar *secret;

	/* RFC 8422, Section 5.7 */

	if (!l_ecdh_generate_key_pair(params->curve,
					&params->private, &our_public)) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"Generating ECDH key pair failed");
		return false;
	}

	ptr += tls_write_ecpoint(ptr, buf + sizeof(buf) - ptr,
					tls->negotiated_curve, our_public);
	l_ecc_point_free(our_public);

	/*
	 * Neither 5.4 or 5.7 "Actions" paragraphs say when the ECDH shared
	 * secret is calculated but we can either do this in
	 * tls_handle_ecdhe_server_key_xchg or here.  In both cases we only
	 * need to store the public key in the client's key_xchg_params and
	 * can free all of the params after sending the ClientKeyExchange.
	 * By doing this calculation here we're aligned with RSA and also
	 * with the server mode where the shared secret can only be
	 * calculated after the ClientKeyExchange is received.
	 */
	if (!l_ecdh_generate_shared_secret(params->private, params->public,
						&secret)) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"Generating ECDH shared-secret failed");
		return false;
	}

	tls_free_ecdhe_params(tls);
	pre_master_secret_len = l_ecc_scalar_get_data(secret,
						pre_master_secret,
						sizeof(pre_master_secret));
	l_ecc_scalar_free(secret);

	if (pre_master_secret_len < 0) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"l_ecc_scalar_get_data(secret) failed");
		return false;
	}

	tls_tx_handshake(tls, TLS_CLIENT_KEY_EXCHANGE, buf, ptr - buf);

	tls_generate_master_secret(tls, pre_master_secret,
					pre_master_secret_len);
	explicit_bzero(pre_master_secret, pre_master_secret_len);

	return true;
}

static void tls_handle_ecdhe_client_key_xchg(struct l_tls *tls,
						const uint8_t *buf, size_t len)
{
	struct tls_ecdhe_params *params = tls->pending.key_xchg_params;
	uint8_t pre_master_secret[128];
	ssize_t pre_master_secret_len;
	struct l_ecc_point *other_public;
	struct l_ecc_scalar *secret;
	size_t point_bytes = 2 * l_ecc_curve_get_scalar_bytes(params->curve);

	/* RFC 8422, Section 5.7 */

	if (len < 2)
		goto decode_error;

	if (*buf++ != 1 + point_bytes)
		goto decode_error;

	if (*buf != 4) {	/* uncompressed */
		TLS_DISCONNECT(TLS_ALERT_ILLEGAL_PARAM, 0,
				"Unsupported (deprecated?) PointConversionForm "
				"%u", *buf);
		return;
	}

	buf++;
	len -= 2;

	if (len != point_bytes)
		goto decode_error;

	/*
	 * RFC 8422, Section 5.11: "A receiving party MUST check that the
	 * x and y parameters from the peer's public value satisfy the
	 * curve equation, y^2 = x^3 + ax + b mod p."
	 * This happens in l_ecc_point_from_data when the L_ECC_POINT_TYPE_FULL
	 * format is used.
	 */
	other_public = l_ecc_point_from_data(params->curve,
						L_ECC_POINT_TYPE_FULL,
						buf, len);
	if (!other_public) {
		TLS_DISCONNECT(TLS_ALERT_DECODE_ERROR, 0,
				"ClientKeyExchange.exchange_keys.ecdh_Yc "
				"decode error");
		return;
	}

	if (!l_ecdh_generate_shared_secret(params->private, other_public,
						&secret)) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"Generating ECDH shared-secret failed");
		return;
	}

	tls_free_ecdhe_params(tls);
	l_ecc_point_free(other_public);
	pre_master_secret_len = l_ecc_scalar_get_data(secret,
						pre_master_secret,
						sizeof(pre_master_secret));
	l_ecc_scalar_free(secret);

	if (pre_master_secret_len < 0) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"l_ecc_scalar_get_data(secret) failed");
		return;
	}

	tls_generate_master_secret(tls, pre_master_secret,
					pre_master_secret_len);
	explicit_bzero(pre_master_secret, pre_master_secret_len);

	return;

decode_error:
	TLS_DISCONNECT(TLS_ALERT_DECODE_ERROR, 0,
			"ClientKeyExchange decode error");
}

static struct tls_key_exchange_algorithm tls_ecdhe = {
	.need_ecc = true,
	.send_server_key_exchange = tls_send_ecdhe_server_key_xchg,
	.handle_server_key_exchange = tls_handle_ecdhe_server_key_xchg,
	.send_client_key_exchange = tls_send_ecdhe_client_key_xchg,
	.handle_client_key_exchange = tls_handle_ecdhe_client_key_xchg,
	.free_params = tls_free_ecdhe_params,
};

/* Maximum FF DH prime modulus size in bytes */
#define TLS_DHE_MAX_SIZE 1024

struct tls_dhe_params {
	size_t prime_len;
	struct l_key *prime;
	struct l_key *generator;
	struct l_key *private;
	struct l_key *public;
};

static void tls_free_dhe_params(struct l_tls *tls)
{
	struct tls_dhe_params *params = tls->pending.key_xchg_params;

	if (!params)
		return;

	tls->pending.key_xchg_params = NULL;

	l_key_free(params->prime);
	l_key_free(params->generator);
	l_key_free(params->private);
	l_key_free(params->public);
	l_free(params);
}

static bool tls_send_dhe_server_key_xchg(struct l_tls *tls)
{
	uint8_t buf[1024 + TLS_DHE_MAX_SIZE * 3];
	uint8_t *ptr = buf + TLS_HANDSHAKE_HEADER_SIZE;
	struct tls_dhe_params *params;
	const uint8_t *prime_buf;
	uint8_t generator_buf = tls->negotiated_ff_group->ff.generator;
	uint8_t public_buf[TLS_DHE_MAX_SIZE];
	size_t public_len;
	unsigned int zeros = 0;
	ssize_t sign_len;
	const uint8_t *server_dh_params_ptr;

	params = l_new(struct tls_dhe_params, 1);
	prime_buf = tls->negotiated_ff_group->ff.prime;
	params->prime_len = tls->negotiated_ff_group->ff.prime_len;

	params->prime = l_key_new(L_KEY_RAW, prime_buf, params->prime_len);
	params->generator = l_key_new(L_KEY_RAW, &generator_buf, 1);

	if (!params->prime || !params->generator) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0, "l_key_new failed");
		goto free_params;
	}

	params->private = l_key_generate_dh_private(prime_buf, params->prime_len);
	if (!params->private) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"l_key_generate_dh_private failed");
		goto free_params;
	}

	memset(public_buf, 0, sizeof(public_buf));
	public_len = params->prime_len;

	if (!l_key_compute_dh_public(params->generator, params->private,
					params->prime, public_buf,
					&public_len)) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"l_key_compute_dh_public failed");
		goto free_params;
	}

	while (zeros < public_len && public_buf[zeros] == 0x00)
		zeros++;

	server_dh_params_ptr = ptr;

	/* RFC 5246, Section 7.4.3 */

	l_put_be16(params->prime_len, ptr);
	memcpy(ptr + 2, prime_buf, params->prime_len);
	ptr += 2 + params->prime_len;

	l_put_be16(1, ptr);
	memcpy(ptr + 2, &generator_buf, 1);
	ptr += 2 + 1;

	l_put_be16(public_len - zeros, ptr);
	memcpy(ptr + 2, public_buf + zeros, public_len - zeros);
	ptr += 2 + public_len - zeros;

	if (tls->pending.cipher_suite->signature) {
		sign_len = tls->pending.cipher_suite->signature->sign(tls, ptr,
						buf + sizeof(buf) - ptr,
						tls_get_dh_params_hash,
						server_dh_params_ptr,
						ptr - server_dh_params_ptr);
		if (sign_len < 0)
			goto free_params;

		ptr += sign_len;
	}

	tls->pending.key_xchg_params = params;

	tls_tx_handshake(tls, TLS_SERVER_KEY_EXCHANGE, buf, ptr - buf);
	return true;

free_params:
	l_key_free(params->prime);
	l_key_free(params->generator);
	l_key_free(params->private);
	l_free(params);
	return false;
}

static void tls_handle_dhe_server_key_xchg(struct l_tls *tls,
						const uint8_t *buf, size_t len)
{
	struct tls_dhe_params *params = NULL;
	const uint8_t *prime_buf;
	const uint8_t *generator_buf;
	size_t generator_len;
	const uint8_t *public_buf;
	size_t public_len;
	const uint8_t *server_dh_params_ptr = buf;

	if (len < 2)
		goto decode_error;

	params = l_new(struct tls_dhe_params, 1);
	params->prime_len = l_get_be16(buf);
	if (len < 2 + params->prime_len + 2)
		goto decode_error;

	prime_buf = buf + 2;
	buf += 2 + params->prime_len;
	len -= 2 + params->prime_len;

	/* Strip leading zeros for the length checks later */
	while (params->prime_len && prime_buf[0] == 0x00) {
		prime_buf++;
		params->prime_len--;
	}

	generator_len = l_get_be16(buf);
	if (len < 2 + generator_len + 2)
		goto decode_error;

	generator_buf = buf + 2;
	buf += 2 + generator_len;
	len -= 2 + generator_len;

	public_len = l_get_be16(buf);
	if (len < 2 + public_len)
		goto decode_error;

	public_buf = buf + 2;
	buf += 2 + public_len;
	len -= 2 + public_len;

	/*
	 * Validate the values received.  Without requiring RFC 7919 from
	 * the server, and there are many servers that don't implement it
	 * yet, we basically have to blindly accept the provided prime value.
	 * We have no way to confirm that it's actually prime or that it's a
	 * "safe prime" or that it forms a group without small sub-groups.
	 * There's also no way to whitelist all valid values.  But we do a
	 * basic sanity check and require it to be 1536-bit or longer, the
	 * minimum length required by the Linux kernel for keyctl_dh_compute().
	 * The generator must also be at least within the min & max interval
	 * for the private/public values.
	 */

	if (params->prime_len > TLS_DHE_MAX_SIZE || params->prime_len < 192 ||
			!(prime_buf[params->prime_len - 1] & 1)) {
		TLS_DISCONNECT(TLS_ALERT_HANDSHAKE_FAIL, 0,
				"Server DH prime modulus invalid");
		goto free_params;
	}

	if (!l_key_validate_dh_payload(generator_buf, generator_len,
					prime_buf, params->prime_len)) {
		TLS_DISCONNECT(TLS_ALERT_HANDSHAKE_FAIL, 0,
				"Server DH generator value invalid");
		goto free_params;
	}

	/*
	 * Just output a warning if the server sent group parameters not
	 * offered in our RFC 7919 Supported Groups extension.
	 */
	if (!tls_find_ff_group(prime_buf, params->prime_len,
				generator_buf, generator_len))
		TLS_DEBUG("Warning: using server's custom %i-bit FF DH group",
				(int) (params->prime_len * 8));

	/*
	 * RFC 7919 Section 3.0:
	 * "the client MUST verify that dh_Ys is in the range
	 * 1 < dh_Ys < dh_p - 1.  If dh_Ys is not in this range, the client
	 * MUST terminate the connection with a fatal handshake_failure(40)
	 * alert."
	 */
	if (!l_key_validate_dh_payload(public_buf, public_len,
					prime_buf, params->prime_len)) {
		TLS_DISCONNECT(TLS_ALERT_HANDSHAKE_FAIL, 0,
				"Server DH public value invalid");
		goto free_params;
	}

	params->prime = l_key_new(L_KEY_RAW, prime_buf, params->prime_len);
	params->generator = l_key_new(L_KEY_RAW, generator_buf, generator_len);
	params->public = l_key_new(L_KEY_RAW, public_buf, public_len);

	if (!params->prime || !params->generator || !params->public) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0, "l_key_new failed");
		goto free_params;
	}

	/* Do this now so we don't need prime_buf in send_client_key_xchg */
	params->private = l_key_generate_dh_private(prime_buf, params->prime_len);
	if (!params->private) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"l_key_generate_dh_private failed");
		goto free_params;
	}

	tls->pending.key_xchg_params = params;

	if (tls->pending.cipher_suite->signature) {
		if (!tls->pending.cipher_suite->signature->verify(tls, buf, len,
						tls_get_dh_params_hash,
						server_dh_params_ptr,
						buf - server_dh_params_ptr))
			return;
	} else {
		if (len)
			goto decode_error;
	}

	TLS_SET_STATE(TLS_HANDSHAKE_WAIT_HELLO_DONE);
	return;

decode_error:
	TLS_DISCONNECT(TLS_ALERT_DECODE_ERROR, 0,
			"ServerKeyExchange decode error");

free_params:
	if (params) {
		l_key_free(params->prime);
		l_key_free(params->generator);
		l_key_free(params->public);
		l_free(params);
	}
}

static bool tls_send_dhe_client_key_xchg(struct l_tls *tls)
{
	struct tls_dhe_params *params = tls->pending.key_xchg_params;
	uint8_t buf[128 + params->prime_len];
	uint8_t *ptr = buf + TLS_HANDSHAKE_HEADER_SIZE;
	uint8_t public_buf[params->prime_len];
	size_t public_len;
	unsigned int zeros = 0;
	uint8_t pre_master_secret[params->prime_len];
	size_t pre_master_secret_len;

	public_len = params->prime_len;
	memset(public_buf, 0, sizeof(public_buf));

	if (!l_key_compute_dh_public(params->generator, params->private,
					params->prime, public_buf,
					&public_len)) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"l_key_compute_dh_public failed");
		return false;
	}

	while (zeros < public_len && public_buf[zeros] == 0x00)
		zeros++;

	l_put_be16(public_len - zeros, ptr);
	memcpy(ptr + 2, public_buf + zeros, public_len - zeros);
	ptr += 2 + public_len - zeros;

	pre_master_secret_len = params->prime_len;
	zeros = 0;

	if (!l_key_compute_dh_secret(params->public, params->private,
					params->prime, pre_master_secret,
					&pre_master_secret_len)) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"Generating DH shared-secret failed");
		return false;
	}

	while (zeros < pre_master_secret_len &&
			pre_master_secret[zeros] == 0x00)
		zeros++;

	tls_tx_handshake(tls, TLS_CLIENT_KEY_EXCHANGE, buf, ptr - buf);

	tls_free_dhe_params(tls);
	tls_generate_master_secret(tls, pre_master_secret + zeros,
					pre_master_secret_len - zeros);
	explicit_bzero(pre_master_secret, pre_master_secret_len);
	return true;
}

static void tls_handle_dhe_client_key_xchg(struct l_tls *tls,
						const uint8_t *buf, size_t len)
{
	struct tls_dhe_params *params = tls->pending.key_xchg_params;
	uint8_t pre_master_secret[params->prime_len];
	size_t pre_master_secret_len;
	size_t public_len;
	unsigned int zeros = 0;

	if (len < 2)
		goto decode_error;

	public_len = l_get_be16(buf);
	buf += 2;
	len -= 2;

	if (public_len != len)
		goto decode_error;

	/*
	 * RFC 7919 Section 4:
	 * "the server MUST verify that 1 < dh_Yc < dh_p - 1.  If dh_Yc is
	 * out of range, the server MUST terminate the connection with
	 * a fatal handshake_failure(40) alert."
	 */
	if (!l_key_validate_dh_payload(buf, public_len,
					tls->negotiated_ff_group->ff.prime,
					params->prime_len)) {
		TLS_DISCONNECT(TLS_ALERT_HANDSHAKE_FAIL, 0,
				"Client DH public value invalid");
		return;
	}

	params->public = l_key_new(L_KEY_RAW, buf, public_len);
	if (!params->public) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0, "l_key_new failed");
		return;
	}

	pre_master_secret_len = params->prime_len;

	if (!l_key_compute_dh_secret(params->public, params->private,
					params->prime, pre_master_secret,
					&pre_master_secret_len)) {
		TLS_DISCONNECT(TLS_ALERT_INTERNAL_ERROR, 0,
				"Generating DH shared-secret failed");
		return;
	}

	while (zeros < pre_master_secret_len &&
			pre_master_secret[zeros] == 0x00)
		zeros++;

	tls_free_dhe_params(tls);
	tls_generate_master_secret(tls, pre_master_secret + zeros,
					pre_master_secret_len - zeros);
	explicit_bzero(pre_master_secret, pre_master_secret_len);
	return;

decode_error:
	TLS_DISCONNECT(TLS_ALERT_DECODE_ERROR, 0,
			"ClientKeyExchange decode error");
}

static struct tls_key_exchange_algorithm tls_dhe = {
	.need_ecc = true,
	.send_server_key_exchange = tls_send_dhe_server_key_xchg,
	.handle_server_key_exchange = tls_handle_dhe_server_key_xchg,
	.send_client_key_exchange = tls_send_dhe_client_key_xchg,
	.handle_client_key_exchange = tls_handle_dhe_client_key_xchg,
	.free_params = tls_free_dhe_params,
};

static struct tls_bulk_encryption_algorithm tls_aes128 = {
	.cipher_type = TLS_CIPHER_BLOCK,
	.l_id = L_CIPHER_AES_CBC,
	.key_length = 16,
	.iv_length = 16,
	.block_length = 16,
}, tls_aes256 = {
	.cipher_type = TLS_CIPHER_BLOCK,
	.l_id = L_CIPHER_AES_CBC,
	.key_length = 32,
	.iv_length = 16,
	.block_length = 16,
}, tls_3des_ede = {
	.cipher_type = TLS_CIPHER_BLOCK,
	.l_id = L_CIPHER_DES3_EDE_CBC,
	.key_length = 24,
	.iv_length = 8,
	.block_length = 8,
}, tls_aes128_gcm = {
	.cipher_type = TLS_CIPHER_AEAD,
	.l_aead_id = L_AEAD_CIPHER_AES_GCM,
	.key_length = 16,
	.iv_length = 12,
	.fixed_iv_length = 4,
	.auth_tag_length = 16,
}, tls_aes256_gcm = {
	.cipher_type = TLS_CIPHER_AEAD,
	.l_aead_id = L_AEAD_CIPHER_AES_GCM,
	.key_length = 32,
	.iv_length = 12,
	.fixed_iv_length = 4,
	.auth_tag_length = 16,
};

static struct tls_mac_algorithm tls_sha = {
	.id = 2,
	.hmac_type = L_CHECKSUM_SHA1,
	.mac_length = 20,
}, tls_sha256 = {
	.id = 4,
	.hmac_type = L_CHECKSUM_SHA256,
	.mac_length = 32,
}, tls_sha384 = {
	.id = 5,
	.hmac_type = L_CHECKSUM_SHA384,
	.mac_length = 48,
};

static struct tls_cipher_suite tls_rsa_with_3des_ede_cbc_sha = {
	.id = { 0x00, 0x0a },
	.name = "TLS_RSA_WITH_3DES_EDE_CBC_SHA",
	.encryption = &tls_3des_ede,
	.mac = &tls_sha,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_rsa_key_xchg,
}, tls_dhe_rsa_with_3des_ede_cbc_sha = {
	.id = { 0x00, 0x16 },
	.name = "TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA",
	.encryption = &tls_3des_ede,
	.mac = &tls_sha,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_dhe,
}, tls_rsa_with_aes_128_cbc_sha = {
	.id = { 0x00, 0x2f },
	.name = "TLS_RSA_WITH_AES_128_CBC_SHA",
	.encryption = &tls_aes128,
	.mac = &tls_sha,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_rsa_key_xchg,
}, tls_dhe_rsa_with_aes_128_cbc_sha = {
	.id = { 0x00, 0x33 },
	.name = "TLS_DHE_RSA_WITH_AES_128_CBC_SHA",
	.encryption = &tls_aes128,
	.mac = &tls_sha,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_dhe,
}, tls_rsa_with_aes_256_cbc_sha = {
	.id = { 0x00, 0x35 },
	.name = "TLS_RSA_WITH_AES_256_CBC_SHA",
	.encryption = &tls_aes256,
	.mac = &tls_sha,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_rsa_key_xchg,
}, tls_dhe_rsa_with_aes_256_cbc_sha = {
	.id = { 0x00, 0x39 },
	.name = "TLS_DHE_RSA_WITH_AES_256_CBC_SHA",
	.encryption = &tls_aes256,
	.mac = &tls_sha,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_dhe,
}, tls_rsa_with_aes_128_cbc_sha256 = {
	.id = { 0x00, 0x3c },
	.name = "TLS_RSA_WITH_AES_128_CBC_SHA256",
	.encryption = &tls_aes128,
	.mac = &tls_sha256,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_rsa_key_xchg,
}, tls_rsa_with_aes_256_cbc_sha256 = {
	.id = { 0x00, 0x3d },
	.name = "TLS_RSA_WITH_AES_256_CBC_SHA256",
	.encryption = &tls_aes256,
	.mac = &tls_sha256,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_rsa_key_xchg,
}, tls_dhe_rsa_with_aes_128_cbc_sha256 = {
	.id = { 0x00, 0x67 },
	.name = "TLS_DHE_RSA_WITH_AES_128_CBC_SHA256",
	.encryption = &tls_aes128,
	.mac = &tls_sha256,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_dhe,
}, tls_dhe_rsa_with_aes_256_cbc_sha256 = {
	.id = { 0x00, 0x6b },
	.name = "TLS_DHE_RSA_WITH_AES_256_CBC_SHA256",
	.encryption = &tls_aes256,
	.mac = &tls_sha256,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_dhe,
}, tls_rsa_with_aes_128_gcm_sha256 = {
	.id = { 0x00, 0x9c },
	.name = "TLS_RSA_WITH_AES_128_GCM_SHA256",
	.encryption = &tls_aes128_gcm,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_rsa_key_xchg,
}, tls_rsa_with_aes_256_gcm_sha384 = {
	.id = { 0x00, 0x9d },
	.name = "TLS_RSA_WITH_AES_256_GCM_SHA384",
	.encryption = &tls_aes256_gcm,
	.prf_hmac = L_CHECKSUM_SHA384,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_rsa_key_xchg,
}, tls_dhe_rsa_with_aes_128_gcm_sha256 = {
	.id = { 0x00, 0x9e },
	.name = "TLS_DHE_RSA_WITH_AES_128_GCM_SHA256",
	.encryption = &tls_aes128_gcm,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_dhe,
}, tls_dhe_rsa_with_aes_256_gcm_sha384 = {
	.id = { 0x00, 0x9f },
	.name = "TLS_DHE_RSA_WITH_AES_256_GCM_SHA384",
	.encryption = &tls_aes256_gcm,
	.prf_hmac = L_CHECKSUM_SHA384,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_dhe,
}, tls_ecdhe_rsa_with_3des_ede_cbc_sha = {
	.id = { 0xc0, 0x12 },
	.name = "TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA",
	.encryption = &tls_3des_ede,
	.mac = &tls_sha,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_rsa_with_aes_128_cbc_sha = {
	.id = { 0xc0, 0x13 },
	.name = "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA",
	.encryption = &tls_aes128,
	.mac = &tls_sha,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_rsa_with_aes_256_cbc_sha = {
	.id = { 0xc0, 0x14 },
	.name = "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA",
	.encryption = &tls_aes256,
	.mac = &tls_sha,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_rsa_with_aes_128_cbc_sha256 = {
	.id = { 0xc0, 0x27 },
	.name = "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256",
	.encryption = &tls_aes128,
	.mac = &tls_sha256,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_rsa_with_aes_256_cbc_sha384 = {
	.id = { 0xc0, 0x28 },
	.name = "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384",
	.encryption = &tls_aes256,
	.mac = &tls_sha384,
	.prf_hmac = L_CHECKSUM_SHA384,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_rsa_with_aes_128_gcm_sha256 = {
	.id = { 0xc0, 0x2f },
	.name = "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256",
	.encryption = &tls_aes128_gcm,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_rsa_with_aes_256_gcm_sha384 = {
	.id = { 0xc0, 0x30 },
	.name = "TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384",
	.encryption = &tls_aes256_gcm,
	.prf_hmac = L_CHECKSUM_SHA384,
	.signature = &tls_rsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_ecdsa_with_3des_ede_cbc_sha = {
	.id = { 0xc0, 0x08 },
	.name = "TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA",
	.encryption = &tls_3des_ede,
	.mac = &tls_sha,
	.signature = &tls_ecdsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_ecdsa_with_aes_128_cbc_sha = {
	.id = { 0xc0, 0x09 },
	.name = "TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA",
	.encryption = &tls_aes128,
	.mac = &tls_sha,
	.signature = &tls_ecdsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_ecdsa_with_aes_256_cbc_sha = {
	.id = { 0xc0, 0x0a },
	.name = "TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA",
	.encryption = &tls_aes256,
	.mac = &tls_sha,
	.signature = &tls_ecdsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_ecdsa_with_aes_128_gcm_sha256 = {
	.id = { 0xc0, 0x2b },
	.name = "TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256",
	.encryption = &tls_aes128_gcm,
	.signature = &tls_ecdsa_signature,
	.key_xchg = &tls_ecdhe,
}, tls_ecdhe_ecdsa_with_aes_256_gcm_sha384 = {
	.id = { 0xc0, 0x2c },
	.name = "TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384",
	.encryption = &tls_aes256_gcm,
	.prf_hmac = L_CHECKSUM_SHA384,
	.signature = &tls_ecdsa_signature,
	.key_xchg = &tls_ecdhe,
};

struct tls_cipher_suite *tls_cipher_suite_pref[] = {
	&tls_ecdhe_rsa_with_aes_256_cbc_sha,
	&tls_ecdhe_ecdsa_with_aes_256_cbc_sha,
	&tls_ecdhe_rsa_with_aes_128_cbc_sha,
	&tls_ecdhe_ecdsa_with_aes_128_cbc_sha,
	&tls_dhe_rsa_with_aes_256_cbc_sha,
	&tls_dhe_rsa_with_aes_128_cbc_sha,
	&tls_rsa_with_aes_256_cbc_sha,
	&tls_rsa_with_aes_128_cbc_sha,
	&tls_ecdhe_rsa_with_aes_256_cbc_sha384,
	&tls_ecdhe_rsa_with_aes_128_cbc_sha256,
	&tls_dhe_rsa_with_aes_256_cbc_sha256,
	&tls_dhe_rsa_with_aes_128_cbc_sha256,
	&tls_rsa_with_aes_256_cbc_sha256,
	&tls_rsa_with_aes_128_cbc_sha256,
	&tls_ecdhe_rsa_with_aes_256_gcm_sha384,
	&tls_ecdhe_rsa_with_aes_128_gcm_sha256,
	&tls_ecdhe_ecdsa_with_aes_256_gcm_sha384,
	&tls_ecdhe_ecdsa_with_aes_128_gcm_sha256,
	&tls_dhe_rsa_with_aes_256_gcm_sha384,
	&tls_dhe_rsa_with_aes_128_gcm_sha256,
	&tls_rsa_with_aes_256_gcm_sha384,
	&tls_rsa_with_aes_128_gcm_sha256,
	&tls_ecdhe_rsa_with_3des_ede_cbc_sha,
	&tls_ecdhe_ecdsa_with_3des_ede_cbc_sha,
	&tls_dhe_rsa_with_3des_ede_cbc_sha,
	&tls_rsa_with_3des_ede_cbc_sha,
	NULL,
};
