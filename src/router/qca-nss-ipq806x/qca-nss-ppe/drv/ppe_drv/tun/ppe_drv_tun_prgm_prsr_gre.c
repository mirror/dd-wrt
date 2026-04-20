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
 * ppe_drv_tun_prgm_prsr_gre_deconfigure
 *     deconfigure Programmable tunnel parser for GRE decap without key
 */
bool ppe_drv_tun_prgm_prsr_gre_deconfigure(struct ppe_drv_tun_prgm_prsr *pgm_psr)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	ppe_drv_assert((pgm_psr->ctx.mode == PPE_DRV_TUN_PROGRAM_MODE_GRE), "program mode not GRE for program type : %d", pgm_psr->parser_idx);

	if (!ppe_drv_tun_prgm_prsr_deconfigure(&pgm_psr->ctx.prsr_cfg, pgm_psr->parser_idx)) {
		ppe_drv_warn("%p: program entry delete failed for GRE Tunnel", p);
		return false;
	}
	return true;
}

/*
 * ppe_drv_tun_prgm_prsr_gre_configure
 *     Configure Program tunnel parser for GRE decap without key
 */
bool ppe_drv_tun_prgm_prsr_gre_configure(struct ppe_drv_tun_prgm_prsr *program_parser)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint8_t parser_idx = program_parser->parser_idx;
	struct ppe_drv_tun_prgm_prsr_cfg *cfg = &program_parser->ctx.prsr_cfg;
	struct ppe_drv_tun_prgm_prsr_decap_key *key = &program_parser->ctx.key;

	ppe_drv_assert((program_parser->ctx.mode == PPE_DRV_TUN_PROGRAM_MODE_GRE), "program mode not GRE for program type : %d", parser_idx);

	if (ppe_drv_tun_prgm_prsr_configured(program_parser)) {
		ppe_drv_trace("%p: Tunnel Programable Parser is already configured for GRE", p);
		return true;
	}

	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_SRC_IP);
	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_DEST_IP);
	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_L4_PROTO);
	key->key_bitmap |= PPE_DRV_TUN_BIT(PPE_DRV_TUN_PRGM_PRSR_DECAP_KEY_TLINFO);
	key->tunnel_info_mask = ~FAL_TUNNEL_DECAP_TUNNEL_INFO_MASK;

	/*
	 * Configure tunnel program entry for GRETAP without key
	 */
	cfg->ip_ver = PPE_DRV_TUN_PRGM_PRSR_IP_VER_IPV4_IPV6;
	cfg->outer_hdr = PPE_DRV_TUN_PRGM_PRSR_OHDR_GRE;
	cfg->protocol = ETH_P_TEB;
	cfg->protocol_mask = FAL_TUNNEL_PROGRAM_GRE_PROTOCOL_MASK;

	/*
	 * Configure the program registers for GRE.
	 * position mode = 0, points to end of outer header
	 * inner_mode = 0, configure in fix mode as its fixed outer header type
	 * inner header type is ETHernet for a GRE packet
	 */
	cfg->inner_mode = PPE_DRV_TUN_PRGM_PRSR_INNER_MODE_FIX;
	cfg->pos_mode = PPE_DRV_TUN_PRGM_PRSR_POS_MODE_END;
	cfg->conf.inner_hdr = PPE_DRV_TUN_PRGM_PRSR_INNER_HDR_ETH;

	if (!ppe_drv_tun_prgm_prsr_configure(cfg, key, parser_idx)) {
		ppe_drv_warn("%p: program entry configuration failed for GRE", p);
		return false;
	}
	return true;
}
