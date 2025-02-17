/*
 * Embedded Linux library
 * Copyright (C) 2018  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_CERT_H
#define __ELL_CERT_H

#include <stddef.h>
#include <ell/cleanup.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_queue;
struct l_cert;
struct l_certchain;

enum l_cert_key_type {
	L_CERT_KEY_RSA,
	L_CERT_KEY_ECC,
	L_CERT_KEY_UNKNOWN,
};

typedef bool (*l_cert_walk_cb_t)(struct l_cert *cert, void *user_data);

struct l_cert *l_cert_new_from_der(const uint8_t *buf, size_t buf_len);
void l_cert_free(struct l_cert *cert);
DEFINE_CLEANUP_FUNC(l_cert_free);

const uint8_t *l_cert_get_der_data(struct l_cert *cert, size_t *out_len);
const uint8_t *l_cert_get_dn(struct l_cert *cert, size_t *out_len);
bool l_cert_get_valid_times(struct l_cert *cert, uint64_t *out_not_before_time,
				uint64_t *out_not_after_time);
enum l_cert_key_type l_cert_get_pubkey_type(struct l_cert *cert);
struct l_key *l_cert_get_pubkey(struct l_cert *cert);

void l_certchain_free(struct l_certchain *chain);
DEFINE_CLEANUP_FUNC(l_certchain_free);

struct l_cert *l_certchain_get_leaf(struct l_certchain *chain);
void l_certchain_walk_from_leaf(struct l_certchain *chain,
				l_cert_walk_cb_t cb, void *user_data);
void l_certchain_walk_from_ca(struct l_certchain *chain,
				l_cert_walk_cb_t cb, void *user_data);

bool l_certchain_verify(struct l_certchain *chain, struct l_queue *ca_certs,
			const char **error);

bool l_cert_load_container_file(const char *filename, const char *password,
				struct l_certchain **out_certchain,
				struct l_key **out_privkey,
				bool *out_encrypted);

bool l_cert_pkcs5_pbkdf1(enum l_checksum_type type, const char *password,
				const uint8_t *salt, size_t salt_len,
				unsigned int iter_count,
				uint8_t *out_dk, size_t dk_len);
bool l_cert_pkcs5_pbkdf2(enum l_checksum_type type, const char *password,
				const uint8_t *salt, size_t salt_len,
				unsigned int iter_count,
				uint8_t *out_dk, size_t dk_len);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_CERT_H */
