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

#ifndef __EIP_TR_IPSEC_H
#define __EIP_TR_IPSEC_H

#include "eip.h"

/*
 * IPsec APIs for tunnel & transport mode.
 */
void eip_tr_ipsec_dec_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
void eip_tr_ipsec_enc_cmn_init(struct eip_tr *tr, struct eip_tr_info *info, const struct eip_svc_entry *algo);
void eip_tr_ipsec_tx_done(struct eip_tr *tr, struct eip_hw_desc *hw, struct eip_sw_desc *sw);
uint8_t eip_tr_ipsec_get_ohdr(uint32_t sa_flags);

#endif /* __EIP_TR_IPSEC_H */
