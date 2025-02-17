/*
 * Embedded Linux library
 * Copyright (C) 2015  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_PEM_H
#define __ELL_PEM_H

#ifdef __cplusplus
extern "C" {
#endif

struct l_queue;
struct l_key;
struct l_cert;
struct l_certchain;

uint8_t *l_pem_load_buffer(const void *buf, size_t buf_len, char **type_label,
				size_t *out_len);
uint8_t *l_pem_load_file(const char *filename, char **type_label, size_t *len);

struct l_certchain *l_pem_load_certificate_chain(const char *filename);
struct l_certchain *l_pem_load_certificate_chain_from_data(const void *buf,
								size_t len);
struct l_queue *l_pem_load_certificate_list(const char *filename);
struct l_queue *l_pem_load_certificate_list_from_data(const void *buf,
							size_t len);

struct l_key *l_pem_load_private_key(const char *filename,
					const char *passphrase,
					bool *encrypted);
struct l_key *l_pem_load_private_key_from_data(const void *buf, size_t len,
						const char *passphrase,
						bool *encrypted);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_PEM_H */
