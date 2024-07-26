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
#ifndef _PPE_DRV_TUN_ENCAP_H_
#define _PPE_DRV_TUN_ENCAP_H_

#define PPE_DRV_TUN_ENCAP_ENTRIES	FAL_TUNNEL_DECAP_ENTRY_MAX
#define PPE_DRV_TUN_ENCAP_HDR_DATA_SIZE	128

/*
 * Header control configuration flags
 */
#define PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_ID_SEED		0
#define PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_DF_SET		1
#define PPE_DRV_TUN_ENCAP_HDR_CTRL_UDP_SPORT_BASE	2
#define PPE_DRV_TUN_ENCAP_HDR_CTRL_UDP_SPORT_MASK	3
#define PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_ADR_MAP		4
#define PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_PROTO_MAP	5
#define PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV6_ADR_MAP		6
#define PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV6_PROTO_MAP	7

/*
 * L2TP tunnel definitions
 */
#define PPE_DRV_TUN_ENCAP_L2TP_TUN_OFFSET	42
#define PPE_DRV_TUN_ENCAP_L2TP_SRC_START	0
#define PPE_DRV_TUN_ENCAP_L2TP_SRC_WIDTH	8
#define PPE_DRV_TUN_ENCAP_L2TP_DEST_POS		48

/*
 * ppe_drv_tun_encap
 *	EG tunnel control table information
 */
struct ppe_drv_tun_encap {
	struct  ppe_drv_port *port;	/* Pointer to associated port structure */
	uint32_t hdr[PPE_DRV_TUN_ENCAP_HDR_DATA_SIZE/sizeof(uint32_t)];
					/* Index into the TL table */
	struct kref ref;		/* Reference counter */
	uint8_t tun_idx;		/* Tunnel index */
	uint8_t tun_len;		/* Tunnel header length */
	uint8_t l3_offset;		/* Tunnel L3 offset */
	uint8_t l4_offset;		/* Tunnel L4 offset */
	uint8_t rule_id;		/* EG edit rule index for MAP-T/L2TP */
	uint8_t l4_offset_valid;	/* is L4 offset valid in header */
};

/*
 * ppe_drv_tun_encap_ppp_hdr
 * 	ppp header structure
 */
struct ppe_drv_tun_encap_ppp_hdr {
	uint8_t address;	/* address feild */
	uint8_t control;	/* Control feild */
	uint16_t protocol;	/* Protocol value */
}__attribute__((packed));

/*
 * ppe_drv_tun_encap_l2tp_hdr
 * 	l2tp header structure
 */
struct ppe_drv_tun_encap_l2tp_hdr {
	uint16_t flags;		/* Type flags */
	uint16_t tunnel_id;	/* tunnel id */
	uint16_t session_id;	/* Session id */
}__attribute__((packed));

/*
 * ppe_drv_tun_encap_hdr_ctrl
 *	Tunnel header control global configuration data
 */
struct ppe_drv_tun_encap_hdr_ctrl {
	uint16_t udp_sport_base;	/* udp source port hash base */
	struct kref udp_sport_base_ref;	/* Reference count for udp sport base */
	uint16_t udp_sport_mask;	/* udp spource port mask */
	struct kref udp_sport_mask_ref;	/* Reference count for udp sport mask */
	uint32_t ipv4_addr_map_data;	/* ipv4 address map data */
	struct kref ipv4_addr_map_ref;	/* Reference count for IPv4 address map */
	uint32_t ipv4_proto_map_data;	/* ipv4 protocol map data */
	struct kref ipv4_proto_map_ref;	/* Reference count for IPv4 IPv4 protocol map */
	uint32_t ipv6_addr_map_data;	/* ipv6 address map data */
	struct kref ipv6_addr_map_ref;	/* Reference count for IPv6 address map */
	uint32_t ipv6_proto_map_data;	/* ipv6 protocol map data */
	struct kref ipv6_proto_map_ref;	/* Reference count for IPv6 protocol map */
};

/*
 * ppe_drv_tun_encap_header_ctrl
 * 	encap header control configuration data
 */
struct ppe_drv_tun_encap_header_ctrl {
	uint16_t ipv4_id_seed;		/* ipv4 id random seed */
	uint16_t ipv4_df_set;		/* ipv4 DF value */
	uint16_t udp_sport_base;	/* udp source port hash base */
	uint16_t udp_sport_mask;	/* udp spource port mask */
	uint32_t ipv4_addr_map_data;	/* ipv4 address map data */
	uint32_t ipv4_proto_map_data;	/* ipv4 protocol map data */
	uint32_t ipv6_addr_map_data;	/* ipv6 address map data */
	uint32_t ipv6_proto_map_data;	/* ipv6 protocol map data */
	uint8_t flags;			/* flag bitmap set*/
};

/*
 * ppe_drv_tun_encap_hdr_ctrl_flag_check()
 * 	check encap header control bitmap
 */
static inline bool ppe_drv_tun_encap_hdr_ctrl_flag_check(uint8_t field, uint8_t flag)
{
	return !!(field & (1 << flag));
}

/*
 * ppe_drv_tun_encap_hdr_ctrl_flag_set()
 * 	set encap header control bitmap
 */
static inline void ppe_drv_tun_encap_hdr_ctrl_flag_set(uint8_t *field, uint8_t flag)
{
	*field |= (1 << flag);
}

/*
 * ppe_drv_tun_encap_hdr_ctrl_flag_clear()
 * 	clear encap header control bitmap
 */
static inline void ppe_drv_tun_encap_hdr_ctrl_flag_clear(uint8_t *field, uint8_t flag)
{
	*field &= ~(1 << flag);
}

uint16_t ppe_drv_tun_encap_get_len(struct ppe_drv_tun_encap *ptec);
void ppe_drv_tun_encap_set_l3_offset(struct ppe_drv_tun_encap *ptec, uint8_t l3_offset);
void ppe_drv_tun_encap_set_l4_offset(struct ppe_drv_tun_encap *ptec, uint8_t l4_offset);
void ppe_drv_tun_encap_set_rule_id(struct ppe_drv_tun_encap *ptec, uint8_t rule_id);
bool ppe_drv_tun_encap_tun_idx_configure(struct ppe_drv_tun_encap *ptec, uint32_t port_num,
		bool tunnel_id_valid);
uint8_t ppe_drv_tun_encap_get_tun_idx(struct ppe_drv_tun_encap *ptec);
bool ppe_drv_tun_encap_configure(struct ppe_drv_tun_encap *ptec, struct ppe_drv_tun_cmn_ctx *th,
		struct ppe_drv_tun_cmn_ctx_l2 *l2_hdr);
bool ppe_drv_tun_encap_deref(struct ppe_drv_tun_encap *ptec);
struct ppe_drv_tun_encap *ppe_drv_tun_encap_ref(struct ppe_drv_tun_encap *ptec);
struct ppe_drv_tun_encap *ppe_drv_tun_encap_alloc(struct ppe_drv *p);
void ppe_drv_tun_encap_entries_free(struct ppe_drv_tun_encap *ptun_ec);
struct ppe_drv_tun_encap *ppe_drv_tun_encap_entries_alloc(struct ppe_drv *p);

/*
 * Encap header control configuration API's
 */
bool ppe_drv_tun_encap_hdr_ctrl_init(struct ppe_drv *p);
void ppe_drv_tun_encap_hdr_ctrl_free(struct ppe_drv_tun_encap_hdr_ctrl *hdr_ctrl);
bool ppe_drv_tun_encap_hdr_ctrl_set(struct ppe_drv_tun_encap_header_ctrl hdr_ctrl);
bool ppe_drv_tun_encap_hdr_ctrl_reset(uint8_t flags);
bool ppe_drv_tun_encap_hdr_ctrl_vxlan_configure(struct ppe_drv *p, struct ppe_drv_tun *tun);
bool ppe_drv_tun_encap_hdr_ctrl_l2tp_configure(struct ppe_drv *p, struct ppe_drv_tun *tun);
#endif /* _PPE_DRV_TUN_ENCAP_H_ */
