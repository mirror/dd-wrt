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

#ifndef _PPE_DRV_TUN_ENCAP_XLATE_RULE_
#define _PPE_DRV_TUN_ENCAP_XLATE_RULE_

#define PPE_DRV_TUN_ENCAP_XLTE_RULE_MAX_RULES 16

/*
 * ppe_drv_tun_encap_xlate_rule
 *	EG edit rule table information
 */
struct ppe_drv_tun_encap_xlate_rule {
	uint8_t rule_index;	/* encap rule index */
	struct kref ref;	/* Reference counter */
};

bool ppe_drv_tun_encap_xlate_rule_deref(struct ppe_drv_tun_encap_xlate_rule *ptecxr);
void ppe_drv_tun_encap_xlate_rule_entries_free(struct ppe_drv_tun_encap_xlate_rule *encap_xlate_rules);
uint8_t ppe_drv_tun_encap_xlate_rule_get_index(struct ppe_drv_tun_encap_xlate_rule *ptecxr);
bool ppe_drv_tun_encap_xlate_rule_configure(struct ppe_drv_tun_encap_xlate_rule *ptecxr,
		struct ppe_drv_tun_cmn_ctx_xlate_rule *rule, int8_t tun_len, uint32_t l2_flags, bool dmr);
struct ppe_drv_tun_encap_xlate_rule *ppe_drv_tun_encap_xlate_rule_entries_alloc(struct ppe_drv *p);
struct ppe_drv_tun_encap_xlate_rule *ppe_drv_tun_encap_xlate_rule_alloc(struct ppe_drv *p);
struct ppe_drv_tun_encap_xlate_rule *ppe_drv_tun_encap_xlate_rule_ref(struct ppe_drv_tun_encap_xlate_rule *ptecxr);
#endif /* _PPE_DRV_TUN_ENCAP_XLATE_RULE_ */
