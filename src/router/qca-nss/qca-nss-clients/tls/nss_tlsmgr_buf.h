/*
 **************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
 * nss_tlsmgr_buf.h
 */

#ifndef __NSS_TLSMGR_BUF_H_
#define __NSS_TLSMGR_BUF_H_

#define NSS_TLSMGR_BUF_MAGIC 0x884e

/**
 * NSS tls manager buffer stucture
 */
struct nss_tlsmgr_buf {
	struct sk_buff *skb;		/**< SKB container pointer. */
	struct nss_tlsmgr_tun *tun;	/**< Pointer to TLS tunnel. */
	void *priv; 			/**< Per buffer private data pointer. */
	void *app_data;			/**< Callback application data. */
	nss_tlsmgr_data_callback_t cb;	/**< Callback routine. */
	uint16_t rec_cnt;		/**< Number of records available in buffer. */
	uint16_t magic;			/**< Magic number protection. */
	struct nss_tlsmgr_rec rec[];	/**< Records starts after buffer. */
};

void nss_tlsmgr_buf_rx(struct nss_tlsmgr_buf *buf, nss_tlsmgr_status_t status);

struct nss_tlsmgr_rec *nss_tlsmgr_buf_get_rec_start(struct nss_tlsmgr_buf *buf);

#endif /* !__NSS_TLSMGR_BUF_H_ */
