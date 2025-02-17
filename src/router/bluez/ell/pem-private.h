/*
 * Embedded Linux library
 * Copyright (C) 2019  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>

struct l_certchain;

struct pem_file_info {
	int fd;
	struct stat st;
	uint8_t *data;
};

int pem_file_open(struct pem_file_info *info, const char *filename);
void pem_file_close(struct pem_file_info *info);

const char *pem_next(const void *buf, size_t buf_len, char **type_label,
				size_t *base64_len,
				const char **endp, bool strict);

uint8_t *pem_load_buffer(const void *buf, size_t buf_len,
				char **out_type_label, size_t *out_len,
				char **out_headers, const char **out_endp);

struct l_key *pem_load_private_key(uint8_t *content, size_t len, char *label,
					const char *passphrase, char *headers,
					bool *encrypted);

int pem_write_certificate_chain(const struct l_certchain *cert,
				const char *filename);
