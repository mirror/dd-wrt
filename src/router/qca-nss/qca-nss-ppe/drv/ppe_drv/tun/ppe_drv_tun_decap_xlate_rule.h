/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _PPE_DRV_TUN_DECAP_XLATE_RULE_H_
#define _PPE_DRV_TUN_DECAP_XLATE_RULE_H_

/*
 * IPV4/IPV6 address size in bytes
 */
#define IPV4_HDR_ADDR_SIZE 4
#define IPV6_HDR_ADDR_SIZE 16

#define PPE_DRV_TUN_DCAP_XLTE_RULE_IPV4_ADDR_LEN 32
#define PPE_DRV_TUN_DCAP_XLTE_RULE_IPV6_HDR_ADDR_SIZE 128

/*
 * L4 port length in bits
 */
#define PPE_DRV_TUN_DCAP_XLTE_L4_PORT_LEN 16
#define PPE_DRV_TUN_DCAP_XLTE_RULE_MAX_ENTRIES 8
/*
 * ppe_drv_tun_decap_xlate_rule
 *	TL L3 interface control table information
 */
struct ppe_drv_tun_decap_xlate_rule {
	uint8_t index;		/* table index */
	struct kref ref;	/* Reference counter */
};

struct ppe_drv_tun_decap_xlate_rule *ppe_drv_tun_decap_xlate_rule_entries_alloc(struct ppe_drv *p);
uint8_t ppe_drv_tun_decap_xlate_rule_get_index(struct ppe_drv_tun_decap_xlate_rule *ptdxrule);
bool ppe_drv_tun_decap_xlate_rule_configure(struct ppe_drv_tun_decap_xlate_rule *decap_xlate_rule,
		struct ppe_drv_tun_cmn_ctx_xlate_rule *rule, bool src_ipv6, bool dmr);
struct ppe_drv_tun_decap_xlate_rule *ppe_drv_tun_decap_xlate_rule_alloc(struct ppe_drv *p);
void ppe_drv_tun_decap_xlate_rule_entries_free(struct ppe_drv_tun_decap_xlate_rule *decap_xlate_rules);
bool ppe_drv_tun_decap_xlate_rule_deref(struct ppe_drv_tun_decap_xlate_rule *ptdxrule);
#endif /* _PPE_DRV_TUN_DECAP_XLATE_RULE_H_ */
