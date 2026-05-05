/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
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
