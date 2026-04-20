/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <ppe_drv/ppe_drv.h>
#include "ppe_drv_tun_prgm_prsr.h"
#include "ppe_drv_tun.h"

/*
 * ppe_drv_tun_prgm_prsr_l2tp_deconfigure
 *	deconfigure Program parser for L2TP tunnel
 */
bool ppe_drv_tun_prgm_prsr_l2tp_deconfigure(struct ppe_drv_tun_prgm_prsr *program)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_tun_prgm_prsr_l2tp *l2tp_data =  &program->ctx.data.l2tp;

	ppe_drv_assert((program->ctx.mode == PPE_DRV_TUN_PROGRAM_MODE_L2TP_V2), "program mode not L2TP for program type : %d", program->parser_idx);

	if (!ppe_drv_tun_prgm_prsr_deconfigure(&program->ctx.prsr_cfg, program->parser_idx)) {
		ppe_drv_warn("%p: program entry delete failed for L2TP Tunnel", p);
		return false;
	}

	/*
	 * delete Program UDF entry for L2TP IPv4
	 */
	if (!ppe_drv_tun_prgm_prsr_prgm_udf_deconfigure(program->parser_idx, &l2tp_data->ipv4_udf)) {
		ppe_drv_warn("%p: program UDF entry delete failed for L2TP Tunnel", p);
		return false;
	}

	/*
	 * delete Program UDF entry for L2TP IPv6
	 */
	if (!ppe_drv_tun_prgm_prsr_prgm_udf_deconfigure(program->parser_idx, &l2tp_data->ipv6_udf)) {
		ppe_drv_warn("%p: program UDF entry delete failed for L2TP Tunnel", p);
		return false;
	}

	/*
	 * Release udf profile instance
	 */
	ppe_drv_tun_udf_entry_dref(l2tp_data->l2tp_udf);

	return true;
}

/*
 * ppe_drv_tun_l2tp_prgm_prsr_configure
 *	Configure Program parser for L2TP tunnel
 */
bool ppe_drv_tun_l2tp_prgm_prsr_configure(struct ppe_drv_tun_prgm_prsr *program)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint8_t program_type = program->parser_idx;
	struct ppe_drv_tun_prgm_prsr_l2tp *l2tp_data =  &program->ctx.data.l2tp;
	struct ppe_drv_tun_prgm_prsr_cfg *cfg = &program->ctx.prsr_cfg;
	struct ppe_drv_tun_prgm_prsr_decap_key *key = &program->ctx.key;
	struct ppe_drv_tun_prgm_prsr_prgm_udf *ipv4_udf = &l2tp_data->ipv4_udf;
	struct ppe_drv_tun_prgm_prsr_prgm_udf *ipv6_udf = &l2tp_data->ipv6_udf;
	struct ppe_drv_tun_udf_profile udf_pf = {0};

	ppe_drv_assert((program->ctx.mode == PPE_DRV_TUN_PROGRAM_MODE_L2TP_V2), "program mode not L2TP for program type : %d", program_type);

	if (ppe_drv_tun_prgm_prsr_configured(program)) {
		/*
		 *  Programable Parser already configured
		 */
		return true;
	}

	/*
	 * decap key configuration
	 */
	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_SRC_IP);
	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_DEST_IP);
	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_L4_PROTO);
	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_SPORT);
	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_DPORT);
	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_UDF0);
	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_UDF1);
	/*
	 * UDF index configured for Tunnel ID of L2TP tunnel
	 */
	key->udf0_id = PPE_DRV_TUN_L2TP_TUNNEL_ID_UDF;
	key->udf1_id = PPE_DRV_TUN_L2TP_SESSION_ID_UDF;

	/*
	 * UDF index configured for Session ID of L2TP tunnel
	 */
	key->udf0_mask = PPE_DRV_TUN_DECAP_L2TP_UDF_MASK;
	key->udf1_mask = PPE_DRV_TUN_DECAP_L2TP_UDF_MASK;

	/*
	 * L2TPv2 frame is of below format need to configure parser to match this
	 *
	 * ETH HDR | IP HDR | UDP HDR | L2TP HDR | PPP HDR | Inner Payload |
	 * Start the match from UDP HDR within IPv4 header as L2TP v2 supports only IPv4 outer header
	 */
	cfg->ip_ver = PPE_DRV_TUN_PRGM_PRSR_IP_VER_IPV4;
	cfg->outer_hdr = PPE_DRV_TUN_PRGM_PRSR_OHDR_UDP;

	/*
	 * Fill the src port and dest port for UDP header match
	 */
	cfg->protocol = ((uint32_t)p->tun_gbl.tun_l2tp.l2tp_dport << 16) + p->tun_gbl.tun_l2tp.l2tp_sport;
	cfg->protocol_mask = PPE_DRV_TUN_DECAP_L2TP_PROTOCOL_MASK;

	/*
	 * Configure UDF feilds to point to L2TP header and PPP headers
	 * starting from end of UDP header.
	 * pos_mode = 0, indicates end of the last matched header i.e UDP header
	 * inner_type_mode = 1, which represents UDF mode
	 * basic_hdr_len denotes the length after which inner payload starts
	 * i.e sizeof L2TP header(6 bytes) + PPP header(4 Bytes) i.e 6+4 = 10bytes
	 */
	cfg->inner_mode = PPE_DRV_TUN_PRGM_PRSR_INNER_MODE_UDF;
	cfg->pos_mode = PPE_DRV_TUN_PRGM_PRSR_POS_MODE_END;
	cfg->conf.udf.hdr_len = 10;

	/*
	 * Add UDF match settings to match L2TP packet type field in the L2TP header
	 * and Address, Control and Protocol in PPP header
	 * udf_offset [0] = 0, points to the L2TP packet type information
	 * udf_offset [1] = 6, points to Address and control fields in PPP header
	 * udf_offset [2] = 8 points to Protocol feild of PPP header
	 */
	cfg->conf.udf.udf_offset[0] = PPE_DRV_TUN_DECAP_L2TP_PKT_TYPE_OFFSET;
	cfg->conf.udf.udf_offset[1] = PPE_DRV_TUN_DECAP_L2TP_PPP_ADDR_OFFSET;
	cfg->conf.udf.udf_offset[2] = PPE_DRV_TUN_DECAP_L2TP_PPP_CTRL_OFFSET;

	if (!ppe_drv_tun_prgm_prsr_configure(cfg, key, program_type)) {
		ppe_drv_warn("%p: program entry configuration failed for L2TPv2", p);
		return false;
	}

	ppe_drv_tun_prgm_udf_bitmap_set(ipv4_udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_SET_UDF0);
	ppe_drv_tun_prgm_udf_bitmap_set(ipv4_udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_SET_UDF1);
	ppe_drv_tun_prgm_udf_bitmap_set(ipv4_udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_SET_UDF2);

	/*
	 * Set values to match against for the wdf fields configured above
	 * udf_val[0] = 0x2, checks if l2tp version is 2. no other optional fields are supported in data packet
	 * udf_val[1] = 0xff03, PPP feilds: address is 0xff and control is 0x03 for all L2TP frames.
	 * udf_val[2] = 0x0021 for IPv4 inner frames
	 * udf_val[2] = 0x0057 for IPv6 inner frames
	 */
	ipv4_udf->udf_val[0] = PPE_DRV_TUN_L2TP_V2_PACKET_TYPE;
	ipv4_udf->udf_val[1] = (PPE_DRV_TUN_L2TP_PPP_ADDRESS << 8) | PPE_DRV_TUN_L2TP_PPP_CONTROL;
	ipv4_udf->udf_val[2] = PPP_IP;
	ipv4_udf->udf_mask[0] = PPE_DRV_TUN_DECAP_L2TP_UDF_MASK;
	ipv4_udf->udf_mask[1] = PPE_DRV_TUN_DECAP_L2TP_UDF_MASK;
	ipv4_udf->udf_mask[2] = PPE_DRV_TUN_DECAP_L2TP_UDF_MASK;
	ipv4_udf->inner_hdr = PPE_DRV_TUN_PRGM_PRSR_INNER_HDR_IPV4;
	/* ipv4_udf->action_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_CHK_INNER_HDR_TYPE); */
	ppe_drv_tun_prgm_udf_action_bitmap_set(ipv4_udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_CHK_INNER_HDR_TYPE);

	/*
	 * Set UDF header length as zero as the basic header already has taken L2TP and PPP lengths into account
	 */
	ipv4_udf->hdr_len = 0;

	if (!ppe_drv_tun_prgm_prsr_prgm_udf_configure(program_type, ipv4_udf)) {
		ppe_drv_warn("%p: program udf entry configuration failed for L2TP", p);
		return false;
	}

	ppe_drv_tun_prgm_udf_bitmap_set(ipv6_udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_SET_UDF0);
	ppe_drv_tun_prgm_udf_bitmap_set(ipv6_udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_SET_UDF1);
	ppe_drv_tun_prgm_udf_bitmap_set(ipv6_udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_SET_UDF2);

	/*
	 * Add IPv6 udf entry.
	 */
	ipv6_udf->udf_val[0] = PPE_DRV_TUN_L2TP_V2_PACKET_TYPE;
	ipv6_udf->udf_val[1] = (PPE_DRV_TUN_L2TP_PPP_ADDRESS << 8) | PPE_DRV_TUN_L2TP_PPP_CONTROL;
	ipv6_udf->udf_val[2] = PPP_IPV6;
	ipv6_udf->udf_mask[0] = PPE_DRV_TUN_DECAP_L2TP_UDF_MASK;
	ipv6_udf->udf_mask[1] = PPE_DRV_TUN_DECAP_L2TP_UDF_MASK;
	ipv6_udf->udf_mask[2] = PPE_DRV_TUN_DECAP_L2TP_UDF_MASK;
	ipv6_udf->inner_hdr = PPE_DRV_TUN_PRGM_PRSR_INNER_HDR_IPV6;
	/* ipv6_udf->action_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_CHK_INNER_HDR_TYPE); */
	ppe_drv_tun_prgm_udf_action_bitmap_set(ipv6_udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_CHK_INNER_HDR_TYPE);

	/*
	 * Set UDF header length as zero as the basic header already has taken L2TP and PPP lengths into account
	 */
	ipv6_udf->hdr_len = 0;

	if (!ppe_drv_tun_prgm_prsr_prgm_udf_configure(program_type, ipv6_udf)) {
		ppe_drv_warn("%p: program udf entry configuration failed for L2TP", p);
		return false;
	}

	/*
	 * Configure tunnel UDF_CTRL to match l3, l4 and PGM type
	 */
	udf_pf.l3_match = true;
	udf_pf.l3_type = PPE_DRV_TUN_UDF_L3_TYPE_IPV4;
	udf_pf.l4_match = true;
	udf_pf.l4_type = PPE_DRV_TUN_UDF_L4_TYPE_UDP;
	udf_pf.program_match = true;
	udf_pf.program_type = program_type;

	ppe_drv_tun_udf_bitmask_set(&udf_pf, PPE_DRV_TUN_L2TP_TUNNEL_ID_UDF);
	ppe_drv_tun_udf_bitmask_set(&udf_pf, PPE_DRV_TUN_L2TP_SESSION_ID_UDF);

	/*
	 * Configure UDF for Tunnel ID and Session ID offsets
	 * from L4 header
	 */
	udf_pf.udf[PPE_DRV_TUN_L2TP_TUNNEL_ID_UDF].offset_type = PPE_DRV_TUN_UDF_OFFSET_TYPE_L4;
	udf_pf.udf[PPE_DRV_TUN_L2TP_TUNNEL_ID_UDF].offset = PPE_DRV_TUN_DECAP_L2TP_TUNNEL_ID_OFFSET;
	udf_pf.udf[PPE_DRV_TUN_L2TP_SESSION_ID_UDF].offset_type = PPE_DRV_TUN_UDF_OFFSET_TYPE_L4;
	udf_pf.udf[PPE_DRV_TUN_L2TP_SESSION_ID_UDF].offset = PPE_DRV_TUN_DECAP_L2TP_SESSION_ID_OFFSET;

	l2tp_data->l2tp_udf = ppe_drv_tun_udf_entry_configure(&udf_pf);
	if (!l2tp_data->l2tp_udf) {
		ppe_drv_trace("%p: Tunnel UDF entry configure failure for L2TP", p);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_l2tp_port_set
 *       Set L2TP source and destination port.
 */
bool ppe_drv_tun_l2tp_port_set(uint16_t sport, uint16_t dport)
{
	if (!ppe_drv_tun_prgm_prsr_type_allocated(PPE_DRV_TUN_PROGRAM_MODE_L2TP_V2)) {
		ppe_drv_gbl.tun_gbl.tun_l2tp.l2tp_sport = sport;
		ppe_drv_gbl.tun_gbl.tun_l2tp.l2tp_dport = dport;
		return true;
	}

	return false;
}
EXPORT_SYMBOL(ppe_drv_tun_l2tp_port_set);

/*
 * ppe_drv_tun_l2tp_port_get
 *      get L2TP source and destination port.
 */
bool ppe_drv_tun_l2tp_port_get(uint16_t *sport, uint16_t *dport)
{
	*sport = ppe_drv_gbl.tun_gbl.tun_l2tp.l2tp_sport;
	*dport = ppe_drv_gbl.tun_gbl.tun_l2tp.l2tp_dport;

	return true;
}
EXPORT_SYMBOL(ppe_drv_tun_l2tp_port_get);
