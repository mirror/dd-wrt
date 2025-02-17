/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_CHECKSUM_H
#define __ELL_CHECKSUM_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/uio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_checksum;

enum l_checksum_type {
	L_CHECKSUM_NONE,
	L_CHECKSUM_MD4,
	L_CHECKSUM_MD5,
	L_CHECKSUM_SHA1,
	L_CHECKSUM_SHA224,
	L_CHECKSUM_SHA256,
	L_CHECKSUM_SHA384,
	L_CHECKSUM_SHA512,
};

struct l_checksum *l_checksum_new(enum l_checksum_type type);
struct l_checksum *l_checksum_new_cmac_aes(const void *key, size_t key_len);
struct l_checksum *l_checksum_new_hmac(enum l_checksum_type type,
					const void *key, size_t key_len);
struct l_checksum *l_checksum_clone(struct l_checksum *checksum);

void l_checksum_free(struct l_checksum *checksum);

void l_checksum_reset(struct l_checksum *checksum);

bool l_checksum_update(struct l_checksum *checksum,
					const void *data, size_t len);
bool l_checksum_updatev(struct l_checksum *checksum,
					const struct iovec *iov,
					size_t iov_len);
ssize_t l_checksum_get_digest(struct l_checksum *checksum,
					void *digest, size_t len);
char *l_checksum_get_string(struct l_checksum *checksum);

bool l_checksum_is_supported(enum l_checksum_type type, bool check_hmac);
bool l_checksum_cmac_aes_supported(void);

ssize_t l_checksum_digest_length(enum l_checksum_type type);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_CHECKSUM_H */
