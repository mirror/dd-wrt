/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/if_tunnel.h>
#include <fal_tunnel.h>
#include <ppe_drv/ppe_drv.h>
#include "ppe_drv_tun_prgm_prsr.h"
#include "ppe_drv_tun.h"

/*
 * ppe_drv_tun_prgm_prsr_free
 *	free program parser instance
 */
static void ppe_drv_tun_prgm_prsr_entry_free(struct kref *kref)
{
	struct ppe_drv_tun_prgm_prsr *pgm =  container_of(kref, struct ppe_drv_tun_prgm_prsr, ref);

	switch (pgm->ctx.mode) {
	case PPE_DRV_TUN_PROGRAM_MODE_GRE:
		if (!ppe_drv_tun_prgm_prsr_gre_deconfigure(pgm)) {
			ppe_drv_warn("%p: error deleting tunnel progamable parser entry for type: %d, mode %d", pgm, pgm->parser_idx, pgm->ctx.mode);
		}
		break;

	case PPE_DRV_TUN_PROGRAM_MODE_L2TP_V2:
		if (!ppe_drv_tun_prgm_prsr_l2tp_deconfigure(pgm)){
			ppe_drv_warn("%p: error deleting tunnel progamable parser entry for type: %d, mode %d",
					pgm, pgm->parser_idx, pgm->ctx.mode);
		}
		break;

	default:
		ppe_drv_warn("%p: unknown programable parser mode %d", pgm, pgm->ctx.mode);
		break;
	}

	memset(&pgm->ctx, 0, sizeof(pgm->ctx));
}

/*
 * ppe_drv_tun_prgm_prsr_deref
 *	Release reference taken on Program Parser Instance
 */
bool ppe_drv_tun_prgm_prsr_deref(struct ppe_drv_tun_prgm_prsr *pgm)
{
	uint8_t parser_idx = pgm->parser_idx;

	ppe_drv_assert(kref_read(&pgm->ref), "ref count under run for program%d tunnel parser", pgm->parser_idx);
	if (kref_put(&pgm->ref, ppe_drv_tun_prgm_prsr_entry_free)) {
		ppe_drv_warn("reference count is 0 for programable parser index: %d", parser_idx);
		return true;
	}
	ppe_drv_trace("%p: mode: %u ref dec:%u", pgm, pgm->ctx.mode, kref_read(&pgm->ref));

	return true;
}

/*
 * ppe_drv_tun_prgm_prsr_ref
 *	Take reference of Program Parser Instance
 */
void ppe_drv_tun_prgm_prsr_ref(struct ppe_drv_tun_prgm_prsr *pgm)
{
	kref_get(&pgm->ref);

	ppe_drv_assert(kref_read(&pgm->ref), "%p: ref count rollover for program type:%d", pgm, pgm->parser_idx);
	ppe_drv_trace("%p: mode: %u ref inc:%u", pgm, pgm->ctx.mode, kref_read(&pgm->ref));
}

/*
 * ppe_drv_tun_prgm_prsr_deconfigure
 * 	Configure program parser instance
 */
bool ppe_drv_tun_prgm_prsr_deconfigure(struct ppe_drv_tun_prgm_prsr_cfg *prsr_cfg, uint8_t parser_idx)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_tunnel_decap_key_t ptdkcfg =  {0};
	fal_tunnel_program_entry_t pgm = {0};
	fal_tunnel_program_cfg_t cfg = {0};
	fal_tunnel_type_t tunnel_type;
	sw_error_t err;

	/*
	 * reset  decap key configuration as the same program can be used for other
	 * tunnels after free
	 */
	tunnel_type = PPE_DRV_TUN_GET_TUNNEL_TYPE_FROM_PGM_TYPE(parser_idx);
	err = fal_tunnel_decap_key_set(PPE_DRV_SWITCH_ID, tunnel_type, &ptdkcfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Tunnel Decap key reset failed for Program%d with error %d", p, parser_idx, err);
		return false;
	}

	/*
	 * Delete program entry
	 */
	pgm.ip_ver = prsr_cfg->ip_ver;
	pgm.outer_hdr_type = PPE_DRV_TUN_PRGM_PRSR_OUT_HDR_TO_FAL_OUT_HDR(prsr_cfg->outer_hdr);
	pgm.protocol = prsr_cfg->protocol;
	pgm.protocol_mask = prsr_cfg->protocol_mask;

	err = fal_tunnel_program_entry_del(PPE_DRV_SWITCH_ID, parser_idx, &pgm);
	if (err != SW_OK) {
		ppe_drv_warn("%p: program entry delete failed for Program%d with error %d", p, parser_idx, err);
		return false;
	}

	/*
	 * Reset Program configurations set for tunnel
	 */
	err = fal_tunnel_program_cfg_set(PPE_DRV_SWITCH_ID, parser_idx, &cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: program entry configuration reset failed for Program%d with error %d", p, parser_idx, err);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_prgm_prsr_configure
 * 	Configure program parser instance
 */
bool ppe_drv_tun_prgm_prsr_configure(struct ppe_drv_tun_prgm_prsr_cfg *prsr_cfg, struct ppe_drv_tun_prgm_prsr_decap_key *key, uint8_t parser_idx)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_tunnel_decap_key_t ptdkcfg =  {0};
	fal_tunnel_program_entry_t pgm = {0};
	fal_tunnel_program_cfg_t cfg = {0};
	fal_tunnel_type_t tunnel_type;
	sw_error_t err;

	/*
	 * Set decap key configurations
	 */
	ptdkcfg.key_bmp = key->key_bitmap;
	ptdkcfg.tunnel_info_mask = key->tunnel_info_mask;
	ptdkcfg.udf0_idx = key->udf0_id;
	ptdkcfg.udf1_idx = key->udf1_id;
	ptdkcfg.udf0_mask = key->udf0_mask;
	ptdkcfg.udf1_mask = key->udf1_mask;

	tunnel_type = PPE_DRV_TUN_GET_TUNNEL_TYPE_FROM_PGM_TYPE(parser_idx);

	err = fal_tunnel_decap_key_set(PPE_DRV_SWITCH_ID, tunnel_type, &ptdkcfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Tunnel Decap key set failed for Program%d with error %d", p, parser_idx, err);
		return false;
	}

	/*
	 * Configure tunnel program entry
	 */
	pgm.ip_ver = prsr_cfg->ip_ver;
	pgm.outer_hdr_type = PPE_DRV_TUN_PRGM_PRSR_OUT_HDR_TO_FAL_OUT_HDR(prsr_cfg->outer_hdr);
	pgm.protocol = prsr_cfg->protocol;
	pgm.protocol_mask = prsr_cfg->protocol_mask;

	err = fal_tunnel_program_entry_add(PPE_DRV_SWITCH_ID, parser_idx, &pgm);
	if (err != SW_OK) {
		ppe_drv_warn("%p: program entry add failed for Program%d with error %d", p, parser_idx, err);
		return false;
	}

	/*
	 * Set program Configuration
	 */
	cfg.inner_type_mode = prsr_cfg->inner_mode;
	cfg.program_pos_mode = prsr_cfg->pos_mode;
	if (prsr_cfg->inner_mode == PPE_DRV_TUN_PRGM_PRSR_INNER_MODE_FIX) {
		cfg.inner_hdr_type = (fal_hdr_type_t)prsr_cfg->conf.inner_hdr;
	} else {
		/*
		 * UDF mode configurations.
		 */
		cfg.basic_hdr_len = prsr_cfg->conf.udf.hdr_len;
		cfg.opt_len_unit = prsr_cfg->conf.udf.len_unit;
		cfg.opt_len_mask = prsr_cfg->conf.udf.len_mask;
		cfg.udf_offset[0] = prsr_cfg->conf.udf.udf_offset[0];
		cfg.udf_offset[1] = prsr_cfg->conf.udf.udf_offset[1];
		cfg.udf_offset[2] = prsr_cfg->conf.udf.udf_offset[2];
	}

	err = fal_tunnel_program_cfg_set(PPE_DRV_SWITCH_ID, parser_idx, &cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: program entry configuration failed for Program%d with error %d", p, parser_idx, err);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_prgm_prsr_prgm_udf_fill
 *     Fill FAL udf structure based on UDF configuration
 */
bool ppe_drv_tun_prgm_prsr_prgm_udf_fill(fal_tunnel_program_udf_t *fal_udf, struct ppe_drv_tun_prgm_prsr_prgm_udf *udf)
{
	int i;

	fal_udf->field_flag = udf->udf_bitmap;

	for (i = 0; i < PPE_DRV_TUN_PRGM_UDF_MAX; i++) {
		if (ppe_drv_tun_prgm_udf_bitmap_check(udf, i)) {
			fal_udf->udf_val[i] = udf->udf_val[i];
			fal_udf->udf_mask[i] = udf->udf_mask[i];
		}
	}

	fal_udf->action_flag = udf->action_bitmap;

	if (ppe_drv_tun_prgm_udf_action_bitmap_check(udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_CHK_INNER_HDR_TYPE)) {
		fal_udf->action_flag |= FAL_TUNNEL_PROGRAM_UDF_ACTION_INNER_HDR_TYPE;
		fal_udf->inner_hdr_type = (fal_hdr_type_t)udf->inner_hdr;
	}

	if (ppe_drv_tun_prgm_udf_action_bitmap_check(udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_CHK_HDR_LEN)) {
		fal_udf->action_flag |= FAL_TUNNEL_PROGRAM_UDF_ACTION_UDF_HDR_LEN;
		fal_udf->udf_hdr_len = udf->hdr_len;
	}

	if (ppe_drv_tun_prgm_udf_action_bitmap_check(udf, PPE_DRV_TUN_PRGM_PRSR_PRGM_UDF_EXCEPTION_EN)) {
		fal_udf->action_flag |= FAL_TUNNEL_PROGRAM_UDF_ACTION_EXCEPTION_EN;
	}

	return true;
}

/*
 * ppe_drv_tun_prgm_prsr_prgm_udf_deconfigure
 *     Deonfigure Program UDF entry
 */
bool ppe_drv_tun_prgm_prsr_prgm_udf_deconfigure(uint8_t parser_idx, struct ppe_drv_tun_prgm_prsr_prgm_udf *udf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_tunnel_program_udf_t fal_udf = {0};
	sw_error_t err;

	ppe_drv_tun_prgm_prsr_prgm_udf_fill(&fal_udf, udf);

	err = fal_tunnel_program_udf_del(PPE_DRV_SWITCH_ID, parser_idx, &fal_udf);
	if (err != SW_OK) {
		ppe_drv_warn("%p: program udf entry del failed for Program%d with error %d", p, parser_idx, err);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_prgm_prsr_prgm_udf_configure
 *     Configure Program UDF entry
 */
bool ppe_drv_tun_prgm_prsr_prgm_udf_configure(uint8_t parser_idx, struct ppe_drv_tun_prgm_prsr_prgm_udf *udf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_tunnel_program_udf_t fal_udf = {0};
	sw_error_t err;

	ppe_drv_tun_prgm_prsr_prgm_udf_fill(&fal_udf, udf);

	err = fal_tunnel_program_udf_add(PPE_DRV_SWITCH_ID, parser_idx, &fal_udf);
	if (err != SW_OK) {
		ppe_drv_warn("%p: program udf entry configuration failed for Program%d with error %d", p, parser_idx, err);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_prgm_prsr_configured
 *	Check if the program parser is already configured
 */
bool ppe_drv_tun_prgm_prsr_configured(struct ppe_drv_tun_prgm_prsr *pgm)
{
	return ((kref_read(&pgm->ref) > 1) ?  true : false);
}

/*
 * ppe_drv_tun_prgm_prsr_type_configured
 *	Check if the program parser is already configured
 */
bool ppe_drv_tun_prgm_prsr_type_allocated(enum ppe_drv_tun_prgm_prsr_mode prsr_mode)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_tun_prgm_prsr *pgm = p->pgm;
	int i;

	if (prsr_mode == PPE_DRV_TUN_PROGRAM_MODE_NONE) {
		ppe_drv_trace("%p: invalid parser type search\n", p);
		return false;
	}

	for (i = 0; i < PPE_DRV_TUN_PRGM_PRSR_MAX; i++) {
		if (pgm[i].ctx.mode == prsr_mode) {
			return true;
		}
	}

	return false;
}

/*
 * ppe_drv_tun_prgm_prsr_entry_alloc
 *	Get free instance of program parser
 */
struct ppe_drv_tun_prgm_prsr *ppe_drv_tun_prgm_prsr_entry_alloc(enum ppe_drv_tun_prgm_prsr_mode mode)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_tun_prgm_prsr *pgm = p->pgm;
	int i, free_index = -1;

	/*
	 * check if the required tunnel is already configured in any program mode.
	 * If configured take a reference and return.
	 * Else assign a free programable parser instance.
	 */
	for (i = 0; i < PPE_DRV_TUN_PRGM_PRSR_MAX; i++) {
		if (pgm[i].ctx.mode == mode) {
			ppe_drv_tun_prgm_prsr_ref(&pgm[i]);
			return &pgm[i];
		}

		if (free_index == -1 &&
				pgm[i].ctx.mode == PPE_DRV_TUN_PROGRAM_MODE_NONE) {
			free_index = i;
		}
	}

	if (free_index == -1) {
		ppe_drv_warn("No free programable Parser index found\n");
		return NULL;
	}

	/*
	 * Take reference on the programable parser instance
	 * and return the same
	 */
	kref_init(&pgm[free_index].ref);
	pgm[free_index].ctx.mode = mode;
	ppe_drv_trace("%p: mode: %u ref inc:%u", &pgm[free_index], pgm[free_index].ctx.mode, kref_read(&pgm[free_index].ref));
	return &pgm[free_index];
}

/*
 * ppe_drv_tun_prgm_prsr_free
 *	Free memory allocated for tunnel Program parser
 */
void ppe_drv_tun_prgm_prsr_free(struct ppe_drv_tun_prgm_prsr *program_parser)
{
	vfree(program_parser);
}

/*
 * ppe_drv_tun_prgm_prsr_alloc
 *	Initialize tunnel program parser structure
 */
struct ppe_drv_tun_prgm_prsr *ppe_drv_tun_prgm_prsr_alloc(struct ppe_drv *p)
{
	struct ppe_drv_tun_prgm_prsr *pgm;
	int index;

	ppe_drv_assert(!p->pgm, "%p: tunnel program parser entries already allocated", p);

	pgm = vzalloc(sizeof(struct ppe_drv_tun_prgm_prsr) * PPE_DRV_TUN_PRGM_PRSR_MAX);
	if (!pgm) {
		ppe_drv_warn("%p: failed to allocate program parser entries", p);
		return NULL;
	}

	for (index = 0; index < PPE_DRV_TUN_PRGM_PRSR_MAX; index++) {
		pgm[index].parser_idx = index;
		pgm[index].ctx.mode = PPE_DRV_TUN_PROGRAM_MODE_NONE;
	}

	return pgm;
}
