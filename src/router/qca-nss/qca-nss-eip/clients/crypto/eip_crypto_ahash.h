/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "eip_crypto.h"

#ifndef __EIP_CRYPTO_AHASH_H
#define __EIP_CRYPTO_AHASH_H

#define EIP_CRYPTO_AHASH_MAX_SIZE U16_MAX /* MAX data size supported by EIP */
#define EIP_CRYPTO_AHASH_MAX_PAGES DIV_ROUND_UP(EIP_CRYPTO_AHASH_MAX_SIZE, PAGE_SIZE)	/* MAX pages HW can support */

/*
 * eip_crypto_ahash_tfm_ctx
 * 	Ahash context structure which includs state and common context
 */
struct eip_crypto_ahash_tfm_ctx {
	struct eip_crypto_tfm_ctx ctx;	/* Parent context */
	struct scatterlist *sg_user;	/* Placeholder for user's req->src */
	uint32_t sg_user_len;	/* Placeholder for user's req->nbytes */
	struct sg_table sgt;	/* sg table to send data to HW */
	size_t data_len;	/*  Length of data stored in temp storage */
	void *data;	/* Temp storage for aggregating the data */
};

#endif
