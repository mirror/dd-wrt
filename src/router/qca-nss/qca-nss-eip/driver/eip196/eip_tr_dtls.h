/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __EIP_TR_DTLS_H
#define __EIP_TR_DTLS_H

#include "eip.h"

#define EIP_TR_DTLS_SKB_CB(__skb) ((struct eip_tr_dtls_skb_cb *)&((__skb)->cb[0]))
#define EIP_TR_DTLS_TRAILER 0x1U
#define EIP_TR_DTLS_VERSION(x) ((x) << 16)
#define EIP_TR_DTLS_EPOCH(x) ((x) << 16)
#define EIP_TR_DTLS_SEQ_START 0x1U

/*
 * eip_tr_dtls_skb_cb
 * 	DTLS sepcific information in skb control block
 */
struct eip_tr_dtls_skb_cb {
	uint8_t pad_len;	/* Padding length required for encryption */
};

/*
 * DTLS transformation APIs.
 */
void eip_tr_dtls_dec_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
void eip_tr_dtls_enc_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
const struct eip_svc_entry *eip_tr_dtls_get_svc(void);
size_t eip_tr_dtls_get_svc_len(void);

#endif /* __EIP_TR_DTLS_H */
