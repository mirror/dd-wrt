/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>

#ifndef _PPE_DRV_TUN_DECAP_H_
#define _PPE_DRV_TUN_DECAP_H_

#define PPE_DRV_TUN_DECAP_MAX_ENTRY             FAL_TUNNEL_DECAP_ENTRY_MAX
#define PPE_DRV_TUN_DECAP_MAP_MAX_ENTRY         8
#define PPE_DRV_TUN_DECAP_MAP_ENTRY_PAIR_MAX    2
#define PPE_DRV_TUN_DECAP_INVALID_IDX           0xFFFF

#define PPE_DRV_TUN_DECAP_L2TP_UDF_MASK 0xffff	/* Common 16 Bit UDF feild Mask used for L2TP tunnel */
#define PPE_DRV_TUN_DECAP_L2TP_TUNNEL_ID_OFFSET 10 /* Tunnel ID offset from start of UDP header */
#define PPE_DRV_TUN_DECAP_L2TP_SESSION_ID_OFFSET 12 /* Session  ID offset from start of UDP Header */
#define PPE_DRV_TUN_DECAP_L2TP_PKT_TYPE_OFFSET 0 /* L2TP offset from end of UDP header */
#define PPE_DRV_TUN_DECAP_L2TP_PPP_ADDR_OFFSET 6 /* PPP Address field offset from end of UDP header */
#define PPE_DRV_TUN_DECAP_L2TP_PPP_CTRL_OFFSET 8 /* PPP Control field offset from end of UDP header */
#define PPE_DRV_TUN_DECAP_L2TP_PROTOCOL_MASK 0xffffffff /* Protocol mask for L2TP program entry config */

/*
 * ppe_drv_tun_decap
 *	tun decap module
 */
struct ppe_drv_tun_decap {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	struct kref ref;	/* Reference counter */
	uint8_t index;		/* Decap entry index */
	uint32_t tl_l3_if_idx;	/* Tunnel l3 interface index */
	uint16_t tl_index;	/* Decap entry table index in HW */
	uint8_t rule_id;	/* Edit rule index associated to this decap for MAP-T */
	struct ppe_drv_tun_prgm_prsr *pgm_prsr;	/* Programable parser instance */
	/* end */
#else
	struct_group(ppe_drv_tun_decap_group,
		struct kref ref;	/* Reference counter */
		uint8_t index;		/* Decap entry index */
		uint32_t tl_l3_if_idx;	/* Tunnel l3 interface index */
		uint16_t tl_index;	/* Decap entry table index in HW */
		uint8_t rule_id;	/* Edit rule index associated to this decap for MAP-T */
		struct ppe_drv_tun_prgm_prsr *pgm_prsr;	/* Programable parser instance */
	);			        /* end of ppe_drv_tun_decap_group group */
#endif
};

uint16_t ppe_drv_tun_decap_configure(struct ppe_drv_tun_decap *ptde, struct ppe_drv_port *pp, struct ppe_drv_tun_cmn_ctx *pth);
struct ppe_drv_tun_decap *ppe_drv_tun_decap_alloc(struct ppe_drv *p);
bool ppe_drv_tun_decap_deref(struct ppe_drv_tun_decap *ptdc);
struct ppe_drv_tun_decap *ppe_drv_tun_decap_ref(struct ppe_drv_tun_decap *ptdc);
bool ppe_drv_tun_decap_activate(struct ppe_drv_tun_decap *ptdc, struct ppe_drv_tun_cmn_ctx_l2 *l2_hdr);
bool ppe_drv_tun_decap_enable(struct ppe_drv_tun_decap *ptdc);
bool ppe_drv_tun_decap_disable(struct ppe_drv_tun_decap *ptdc);
void ppe_drv_tun_decap_set_tl_index(struct ppe_drv_tun_decap *ptdc, uint32_t hwidx);
void ppe_drv_tun_decap_set_tl_l3_idx(struct ppe_drv_tun_decap *ptdc, uint8_t tl_l3_idx);
void ppe_drv_tun_decap_entries_free(struct ppe_drv_tun_decap *ptun_dc);
struct ppe_drv_tun_decap *ppe_drv_tun_decap_entries_alloc(struct ppe_drv *p);

/*
 * MAP decap APIs
 */
bool ppe_drv_tun_decap_map_configure(struct ppe_drv_tun_decap *ptdcm, uint32_t *ipv6_prefix, uint8_t prefix_len, struct ppe_drv_tun_cmn_ctx_l2 *l2_hdr,
				bool src_info_update, uint16_t port_num, uint16_t rule_id, uint8_t ip_to_me);
void ppe_drv_tun_decap_map_deconfigure(struct kref *kref);
bool ppe_drv_tun_decap_map_entry_deref(struct ppe_drv_tun_decap *ptdcm);
void ppe_drv_tun_decap_map_set_rule_id(struct ppe_drv_tun_decap *ptdcm, uint16_t rule_idx);
void ppe_drv_tun_decap_map_entries_free(struct ppe_drv *p);
struct ppe_drv_tun_decap * ppe_drv_tun_decap_map_entries_alloc(struct ppe_drv *p);
struct ppe_drv_tun_decap *ppe_drv_tun_decap_map_alloc(struct ppe_drv *p);
#endif /* _PPE_DRV_TUN_DECAP_H_ */
