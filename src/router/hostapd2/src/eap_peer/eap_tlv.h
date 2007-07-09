/*
 * EAP peer method: EAP-TLV (draft-josefsson-pppext-eap-tls-eap-07.txt)
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef EAP_TLV_H
#define EAP_TLV_H

#include "eap_common/eap_tlv_common.h"

u8 * eap_tlv_build_nak(int id, u16 nak_type, size_t *resp_len);
u8 * eap_tlv_build_result(int id, u16 status, size_t *resp_len);
int eap_tlv_process(struct eap_sm *sm, struct eap_method_ret *ret,
		    const struct eap_hdr *hdr, u8 **resp, size_t *resp_len);

#endif /* EAP_TLV_H */
