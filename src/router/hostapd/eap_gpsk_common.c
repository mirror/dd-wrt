/*
 * EAP server/peer: EAP-GPSK shared routines
 * Copyright (c) 2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#include "common.h"
#include "eap_defs.h"
#include "aes_wrap.h"
#include "sha256.h"
#include "eap_gpsk_common.h"


/**
 * eap_gpsk_supported_ciphersuite - Check whether ciphersuite is supported
 * @vendor: CSuite/Vendor
 * @specifier: CSuite/Specifier
 * Returns: 1 if ciphersuite is support, or 0 if not
 */
int eap_gpsk_supported_ciphersuite(int vendor, int specifier)
{
	if (vendor == EAP_GPSK_VENDOR_IETF &&
	    specifier == EAP_GPSK_CIPHER_AES)
		return 1;
#ifdef EAP_GPSK_SHA256
	if (vendor == EAP_GPSK_VENDOR_IETF &&
	    specifier == EAP_GPSK_CIPHER_SHA256)
		return 1;
#endif /* EAP_GPSK_SHA256 */
	return 0;
}


static int eap_gpsk_gkdf_aes(const u8 *psk, const u8 *data, size_t data_len,
			     u8 *buf, size_t len)
{
	u8 *tmp, *pos, *opos;
	size_t i, n, size, left, clen;

	size = 16;

	tmp = malloc(size + data_len + 2 + 2);
	if (tmp == NULL) {
		wpa_printf(MSG_DEBUG, "EAP-GPSK: Failed to allocate memory "
			   "for GKDF");
		return -1;
	}

	opos = buf;
	left = len;
	n = (len + size - 1) / size;
	for (i = 1; i <= n; i++) {
		pos = tmp;
		if (i > 1)
			pos += size;
		memcpy(pos, data, data_len);
		pos += data_len;
		WPA_PUT_BE16(pos, i);
		pos += 2;
		WPA_PUT_BE16(pos, len);
		pos += 2;

		if (omac1_aes_128(psk, tmp, pos - tmp, tmp) < 0) {
			free(tmp);
			return -1;
		}
		clen = left > size ? size : left;
		memcpy(opos, tmp, clen);
		opos += clen;
		left -= clen;
	}

	free(tmp);

	return 0;
}


static int eap_gpsk_derive_keys_aes(const u8 *psk, size_t psk_len,
				    const u8 *seed, size_t seed_len,
				    u8 *msk, u8 *sk, size_t *sk_len,
				    u8 *pk, size_t *pk_len)
{
#define EAP_GPSK_SK_LEN_AES 16
#define EAP_GPSK_PK_LEN_AES 16
	u8 mk[16], *pos;
	u8 kdf_out[EAP_MSK_LEN + EAP_EMSK_LEN + EAP_GPSK_SK_LEN_AES +
		   EAP_GPSK_PK_LEN_AES];

	/*
	 * MK = GKDF-16(PSK[0..15], seed)
	 * KDF_out = GKDF-160(MK, seed)
	 * MSK = KDF_out[0..63]
	 * EMSK = KDF_out[64..127]
	 * SK = KDF_out[128..143]
	 * PK = KDF_out[144..159]
	 * MID = GKDF-16(Zero-String, "Method ID" || EAP_Method_Type || seed)
	 */

	if (psk_len < 16) {
		wpa_printf(MSG_DEBUG, "EAP-GPSK: Too short PSK (len %d) for "
			   "AES-128 ciphersuite", psk_len);
		return -1;
	}

	if (eap_gpsk_gkdf_aes(psk, seed, seed_len, mk, sizeof(mk)) < 0)
		return -1;
	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: MK", mk, sizeof(mk));

	if (eap_gpsk_gkdf_aes(mk, seed, seed_len, kdf_out, sizeof(kdf_out)) <
	    0)
		return -1;

	pos = kdf_out;
	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: MSK", pos, EAP_MSK_LEN);
	memcpy(msk, pos, EAP_MSK_LEN);
	pos += EAP_MSK_LEN;

	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: EMSK", pos, EAP_EMSK_LEN);
	pos += EAP_EMSK_LEN;

	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: SK", pos, EAP_GPSK_SK_LEN_AES);
	memcpy(sk, pos, EAP_GPSK_SK_LEN_AES);
	*sk_len = EAP_GPSK_SK_LEN_AES;
	pos += EAP_GPSK_SK_LEN_AES;

	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: PK", pos, EAP_GPSK_PK_LEN_AES);
	memcpy(pk, pos, EAP_GPSK_PK_LEN_AES);
	*pk_len = EAP_GPSK_PK_LEN_AES;

	return 0;
}


#ifdef EAP_GPSK_SHA256
static int eap_gpsk_gkdf_sha256(const u8 *psk, size_t psk_len,
				const u8 *data, size_t data_len,
				u8 *buf, size_t len)
{
	u8 *tmp, *pos, *opos;
	size_t i, n, size, left, clen;
	u8 ibuf[2], lenbuf[2], hash[SHA256_MAC_LEN];
	const u8 *addr[4];
	size_t vlen[4];

	size = SHA256_MAC_LEN;
	addr[0] = hash;
	vlen[0] = size;
	addr[1] = data;
	vlen[1] = data_len;
	addr[2] = ibuf;
	vlen[2] = sizeof(ibuf);
	addr[3] = lenbuf;
	vlen[3] = sizeof(lenbuf);

	opos = buf;
	left = len;
	n = (len + size - 1) / size;
	for (i = 1; i <= n; i++) {
		pos = tmp;
		if (i > 1)
			hmac_sha256_vector(psk, psk_len, 4, addr, vlen, hash);
		else
			hmac_sha256_vector(psk, psk_len, 3, addr + 1, vlen + 1,
					   hash);

		clen = left > size ? size : left;
		memcpy(opos, hash, clen);
		opos += clen;
		left -= clen;
	}

	return 0;
}


static int eap_gpsk_derive_keys_sha256(const u8 *psk, size_t psk_len,
				       const u8 *seed, size_t seed_len,
				       u8 *msk, u8 *sk, size_t *sk_len,
				       u8 *pk, size_t *pk_len)
{
#define EAP_GPSK_SK_LEN_SHA256 SHA256_MAC_LEN
#define EAP_GPSK_PK_LEN_SHA256 SHA256_MAC_LEN
	u8 mk[SHA256_MAC_LEN], *pos;
	u8 kdf_out[EAP_MSK_LEN + EAP_EMSK_LEN + EAP_GPSK_SK_LEN_SHA256 +
		   EAP_GPSK_PK_LEN_SHA256];

	/*
	 * MK = GKDF-32(PSK, seed || CSuite_Sel)
	 * KDF_out = GKDF-192(MK, seed)
	 * MSK = KDF_out[0..63]
	 * EMSK = KDF_out[64..127]
	 * SK = KDF_out[128..159]
	 * PK = KDF_out[160..191]
	 * MID = GKDF-16(Zero-String, "Method ID" || EAP_Method_Type || seed)
	 */

	if (eap_gpsk_gkdf_sha256(psk, psk_len, seed, seed_len, mk, sizeof(mk))
	    < 0)
		return -1;
	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: MK", mk, sizeof(mk));

	if (eap_gpsk_gkdf_sha256(mk, sizeof(mk), seed, seed_len,
				 kdf_out, sizeof(kdf_out)) < 0)
		return -1;

	pos = kdf_out;
	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: MSK", pos, EAP_MSK_LEN);
	memcpy(msk, pos, EAP_MSK_LEN);
	pos += EAP_MSK_LEN;

	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: EMSK", pos, EAP_EMSK_LEN);
	pos += EAP_EMSK_LEN;

	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: SK",
			pos, EAP_GPSK_SK_LEN_SHA256);
	memcpy(sk, pos, EAP_GPSK_SK_LEN_SHA256);
	*sk_len = EAP_GPSK_SK_LEN_AES;
	pos += EAP_GPSK_SK_LEN_AES;

	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: PK",
			pos, EAP_GPSK_PK_LEN_SHA256);
	memcpy(pk, pos, EAP_GPSK_PK_LEN_SHA256);
	*pk_len = EAP_GPSK_PK_LEN_SHA256;

	return 0;
}
#endif /* EAP_GPSK_SHA256 */


/**
 * eap_gpsk_derive_keys - Derive EAP-GPSK keys
 * @psk: Pre-shared key (at least 16 bytes if AES is used)
 * @psk_len: Length of psk in bytes
 * @vendor: CSuite/Vendor
 * @specifier: CSuite/Specifier
 * @rand_client: 32-byte RAND_Client
 * @rand_server: 32-byte RAND_Server
 * @id_client: ID_Client
 * @id_client_len: Length of ID_Client
 * @id_server: ID_Server
 * @id_server_len: Length of ID_Server
 * @msk: Buffer for 64-byte MSK
 * @sk: Buffer for SK (at least EAP_GPSK_MAX_SK_LEN bytes)
 * @sk_len: Buffer for returning length of SK
 * @pk: Buffer for SK (at least EAP_GPSK_MAX_PK_LEN bytes)
 * @pk_len: Buffer for returning length of PK
 * Returns: 0 on success, -1 on failure
 */
int eap_gpsk_derive_keys(const u8 *psk, size_t psk_len, int vendor,
			 int specifier,
			 const u8 *rand_client, const u8 *rand_server,
			 const u8 *id_client, size_t id_client_len,
			 const u8 *id_server, size_t id_server_len,
			 u8 *msk, u8 *sk, size_t *sk_len,
			 u8 *pk, size_t *pk_len)
{
	u8 *seed, *pos;
	size_t seed_len;
	int ret;

	wpa_printf(MSG_DEBUG, "EAP-GPSK: Deriving keys (%d:%d)",
		   vendor, specifier);

	if (vendor != EAP_GPSK_VENDOR_IETF)
		return -1;

	wpa_hexdump_key(MSG_DEBUG, "EAP-GPSK: PSK", psk, psk_len);

	/* Seed = RAND_Client || RAND_Server || ID_Server || IN_Client */
	seed_len = 2 * EAP_GPSK_RAND_LEN + id_server_len + id_client_len;
	seed = malloc(seed_len);
	if (seed == NULL) {
		wpa_printf(MSG_DEBUG, "EAP-GPSK: Failed to allocate memory "
			   "for key derivation");
		return -1;
	}

	pos = seed;
	memcpy(pos, rand_client, EAP_GPSK_RAND_LEN);
	pos += EAP_GPSK_RAND_LEN;
	memcpy(pos, rand_server, EAP_GPSK_RAND_LEN);
	pos += EAP_GPSK_RAND_LEN;
	memcpy(pos, id_server, id_server_len);
	pos += id_server_len;
	memcpy(pos, id_client, id_client_len);
	pos += id_client_len;
	wpa_hexdump(MSG_DEBUG, "EAP-GPSK: Seed", seed, seed_len);

	switch (specifier) {
	case EAP_GPSK_CIPHER_AES:
		ret = eap_gpsk_derive_keys_aes(psk, psk_len, seed, seed_len,
					       msk, sk, sk_len, pk, pk_len);
		break;
#ifdef EAP_GPSK_SHA256
	case EAP_GPSK_CIPHER_SHA256:
		ret = eap_gpsk_derive_keys_sha256(psk, psk_len, seed, seed_len,
						  msk, sk, sk_len, pk, pk_len);
		break;
#endif /* EAP_GPSK_SHA256 */
	default:
		wpa_printf(MSG_DEBUG, "EAP-GPSK: Unknown cipher %d:%d used in "
			   "key derivation", vendor, specifier);
		ret = -1;
		break;
	}

	free(seed);

	return ret;
}


/**
 * eap_gpsk_mic_len - Get the length of the MIC
 * @vendor: CSuite/Vendor
 * @specifier: CSuite/Specifier
 * Returns: MIC length in bytes
 */
size_t eap_gpsk_mic_len(int vendor, int specifier)
{
	if (vendor != EAP_GPSK_VENDOR_IETF)
		return 0;

	switch (specifier) {
	case EAP_GPSK_CIPHER_AES:
		return 16;
#ifdef EAP_GPSK_SHA256
	case EAP_GPSK_CIPHER_SHA256:
		return 32;
#endif /* EAP_GPSK_SHA256 */
	default:
		return 0;
	}
}


static int eap_gpsk_compute_mic_aes(const u8 *sk, size_t sk_len,
				    const u8 *data, size_t len, u8 *mic)
{
	if (sk_len != 16) {
		wpa_printf(MSG_DEBUG, "EAP-GPSK: Invalid SK length %d for "
			   "AES-CMAC MIC", sk_len);
		return -1;
	}

	return omac1_aes_128(sk, data, len, mic);
}


/**
 * eap_gpsk_compute_mic - Compute EAP-GPSK MIC for an EAP packet
 * @sk: Session key SK from eap_gpsk_derive_keys()
 * @sk_len: SK length in bytes from eap_gpsk_derive_keys()
 * @vendor: CSuite/Vendor
 * @specifier: CSuite/Specifier
 * @data: Input data to MIC
 * @len: Input data length in bytes
 * @mic: Buffer for the computed MIC, eap_gpsk_mic_len(cipher) bytes
 * Returns: 0 on success, -1 on failure
 */
int eap_gpsk_compute_mic(const u8 *sk, size_t sk_len, int vendor,
			 int specifier, const u8 *data, size_t len, u8 *mic)
{
	int ret;

	if (vendor != EAP_GPSK_VENDOR_IETF)
		return -1;

	switch (specifier) {
	case EAP_GPSK_CIPHER_AES:
		ret = eap_gpsk_compute_mic_aes(sk, sk_len, data, len, mic);
		break;
#ifdef EAP_GPSK_SHA256
	case EAP_GPSK_CIPHER_SHA256:
		hmac_sha256(sk, sk_len, data, len, mic);
		ret = 0;
		break;
#endif /* EAP_GPSK_SHA256 */
	default:
		wpa_printf(MSG_DEBUG, "EAP-GPSK: Unknown cipher %d:%d used in "
			   "MIC computation", vendor, specifier);
		ret = -1;
		break;
	}

	return ret;
}
