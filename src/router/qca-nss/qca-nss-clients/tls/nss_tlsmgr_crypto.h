/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE
 **************************************************************************
 */

/*
 * nss_tlsmgr_crypto.h
 */

#ifndef __NSS_TLSMGR_CRYPTO_H_
#define __NSS_TLSMGR_CRYPTO_H_

/*
 * TLS algorithm information
 */
struct nss_tlsmgr_algo_info {
	char *name;				/* Linux crypto algorithm string. */
	uint32_t rta_key_sz;			/* RTA key attribute size. */
};

/*
 * TLS crypto configuration data; this data structure will be maintained
 * as a list in actual context data. This data structure will hold aead
 * context and crypto index to be used for a particular context.
 */
struct nss_tlsmgr_crypto {
	struct list_head list;                  /* List of crypto data. */
	struct crypto_aead *aead;               /* Linux AEAD context. */
	struct crypto_ahash *ahash;		/* Linux crypto AHASH context */
	uint32_t crypto_idx;			/* List head */
};

nss_tlsmgr_status_t nss_tlsmgr_crypto_update_null(struct net_device *dev, struct nss_tlsmgr_ctx *ctx);
struct nss_tlsmgr_crypto *nss_tlsmgr_crypto_alloc(struct nss_tlsmgr_config *cfg);
void nss_tlsmgr_crypto_free(struct nss_tlsmgr_crypto *crypto);
#endif /* !__NSS_TLSMGR_CRYPTO_H_ */
