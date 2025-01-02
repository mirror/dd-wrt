/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * PKI Support.
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pki-defs.h>
#include <asm/octeon/cvmx-pki.h>
#include <asm/octeon/cvmx-fpa3.h>
#include <asm/octeon/cvmx-pki-cluster.h>
#include <asm/octeon/cvmx-pki-resources.h>
#else
#include "cvmx.h"
#include "cvmx-version.h"
#include "cvmx-error.h"
#include "cvmx-pki-defs.h"
#include "cvmx-pki.h"
#include "cvmx-fpa3.h"
#include "cvmx-pki-cluster.h"
#include "cvmx-pki-resources.h"
#endif

/**
 * This function enables PKI
 *
 * @param node  Node to enable PKI.
 */
void cvmx_pki_enable(int node)
{
	cvmx_pki_sft_rst_t sft_rst;
	cvmx_pki_buf_ctl_t buf_ctl;

	sft_rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);
	while (sft_rst.s.busy != 0)
		sft_rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);

	buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	if (buf_ctl.s.pki_en)
		cvmx_dprintf("Warning: Enabling PKI when PKI already enabled.\n");

	buf_ctl.s.pki_en = 1;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, buf_ctl.u64);

}
EXPORT_SYMBOL(cvmx_pki_enable);

/**
 * This function disables PKI.
 *
 * @param node  Node to disable PKI.
 */
void cvmx_pki_disable(int node)
{
	cvmx_pki_buf_ctl_t buf_ctl;
	buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	buf_ctl.s.pki_en = 0;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, buf_ctl.u64);
}
EXPORT_SYMBOL(cvmx_pki_disable);

/**
 * This function soft resets PKI.
 *
 * @param node  Node to enable PKI.
 */
void cvmx_pki_reset(int node)
{
	cvmx_pki_sft_rst_t sft_rst;

	sft_rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);
	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_PKI_SFT_RST,
					cvmx_pki_sft_rst_t, active, ==, 0, 10000)) {
		cvmx_dprintf("PKI_SFT_RST is not active\n");
	}

	sft_rst.s.rst = 1;
	cvmx_write_csr_node(node, CVMX_PKI_SFT_RST, sft_rst.u64);
	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_PKI_SFT_RST,
					cvmx_pki_sft_rst_t, busy, ==, 0, 10000)) {
		cvmx_dprintf("PKI_SFT_RST is busy\n");
	}
}

/**
 * This function sets the clusters in PKI.
 *
 * @param node  Node to set clusters.
 */
int cvmx_pki_setup_clusters(int node)
{
	int i;

	for (i = 0; i < cvmx_pki_cluster_code_length; i++)
		cvmx_write_csr_node(node, CVMX_PKI_IMEMX(i), cvmx_pki_cluster_code_default[i]);

	return 0;
}
EXPORT_SYMBOL(cvmx_pki_setup_clusters);

/**
 * @INTERNAL
 * This function is called by cvmx_helper_shutdown() to extract all FPA buffers
 * out of the PKI. After this function completes, all FPA buffers that were
 * prefetched by PKI will be in the apropriate FPA pool. This functions does
 * not reset PKI.
 * WARNING: It is very important that PKI be reset soon after a call to this function.
 */
void __cvmx_pki_free_ptr(int node)
{
	cvmx_pki_buf_ctl_t buf_ctl;

	buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	/* Disable buffering any data. */
	buf_ctl.s.pkt_off = 1;
	/* Disable caching of any data and return all the prefetched buffers to FPA. */
	buf_ctl.s.fpa_cac_dis = 1;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, buf_ctl.u64);
	buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
}

/**
 * This function reads global configuration of PKI block.
 *
 * @param node  Node number.
 * @param gbl_cfg  Pointer to struct to read global configuration.
 */
void cvmx_pki_read_global_config(int node, struct cvmx_pki_global_config *gbl_cfg)
{
	cvmx_pki_stat_ctl_t stat_ctl;
	cvmx_pki_icgx_cfg_t icg_cfg;
	cvmx_pki_gbl_pen_t gbl_pen;
	cvmx_pki_tag_secret_t tag_secret;
	cvmx_pki_frm_len_chkx_t frm_len_chk;
	cvmx_pki_buf_ctl_t buf_ctl;
	unsigned cl_grp;
	int id;

	stat_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_STAT_CTL);
	gbl_cfg->stat_mode = stat_ctl.s.mode;

	for (cl_grp = 0; cl_grp < CVMX_PKI_NUM_CLUSTER_GROUP; cl_grp++) {
		icg_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(cl_grp));
		gbl_cfg->cluster_mask[cl_grp] = icg_cfg.s.clusters;
	}
	gbl_pen.u64 = cvmx_read_csr_node(node, CVMX_PKI_GBL_PEN);
	gbl_cfg->gbl_pen.virt_pen = gbl_pen.s.virt_pen;
	gbl_cfg->gbl_pen.clg_pen = gbl_pen.s.clg_pen;
	gbl_cfg->gbl_pen.cl2_pen = gbl_pen.s.cl2_pen;
	gbl_cfg->gbl_pen.l4_pen = gbl_pen.s.l4_pen;
	gbl_cfg->gbl_pen.il3_pen = gbl_pen.s.il3_pen;
	gbl_cfg->gbl_pen.l3_pen = gbl_pen.s.l3_pen;
	gbl_cfg->gbl_pen.mpls_pen = gbl_pen.s.mpls_pen;
	gbl_cfg->gbl_pen.fulc_pen = gbl_pen.s.fulc_pen;
	gbl_cfg->gbl_pen.dsa_pen = gbl_pen.s.dsa_pen;
	gbl_cfg->gbl_pen.hg_pen = gbl_pen.s.hg_pen;

	tag_secret.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_SECRET);
	gbl_cfg->tag_secret.dst6 = tag_secret.s.dst6;
	gbl_cfg->tag_secret.src6 = tag_secret.s.src6;
	gbl_cfg->tag_secret.dst = tag_secret.s.dst;
	gbl_cfg->tag_secret.src = tag_secret.s.src;

	for (id = 0; id < CVMX_PKI_NUM_FRAME_CHECK; id++) {
		frm_len_chk.u64 = cvmx_read_csr_node(node, CVMX_PKI_FRM_LEN_CHKX(id));
		gbl_cfg->frm_len[id].maxlen = frm_len_chk.s.maxlen;
		gbl_cfg->frm_len[id].minlen = frm_len_chk.s.minlen;
	}
	buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	gbl_cfg->fpa_wait = buf_ctl.s.fpa_wait;
}

/**
 * This function writes max and min frame lengths to hardware which can be used
 * to check the size of frame arrived.There are 2 possible combination which are
 * indicated by id field.
 *
 * @param node  Node number.
 * @param id  Choose which frame len register to write to
 * @param len_chk  Struct containing byte count for max-sized/min-sized frame check.
 */
static void cvmx_pki_write_frame_len(int node, int id,
	struct cvmx_pki_frame_len len_chk)
{
	cvmx_pki_frm_len_chkx_t frm_len_chk;
	frm_len_chk.u64 = cvmx_read_csr_node(node, CVMX_PKI_FRM_LEN_CHKX(id));
	frm_len_chk.s.maxlen = len_chk.maxlen;
	frm_len_chk.s.minlen = len_chk.minlen;
	cvmx_write_csr_node(node, CVMX_PKI_FRM_LEN_CHKX(id), frm_len_chk.u64);
}

/**
 * This function writes global configuration of PKI into hw.
 *
 * @param node  Node number.
 * @param gbl_cfg  Pointer to struct to global configuration.
 */
void cvmx_pki_write_global_config(int node, struct cvmx_pki_global_config *gbl_cfg)
{
	cvmx_pki_stat_ctl_t stat_ctl;
	cvmx_pki_buf_ctl_t buf_ctl;
	unsigned cl_grp;

	for (cl_grp = 0; cl_grp < CVMX_PKI_NUM_CLUSTER_GROUP; cl_grp++)
		cvmx_pki_attach_cluster_to_group(node, cl_grp, gbl_cfg->cluster_mask[cl_grp]);

	stat_ctl.u64 = 0;
	stat_ctl.s.mode = gbl_cfg->stat_mode;
	cvmx_write_csr_node(node, CVMX_PKI_STAT_CTL, stat_ctl.u64);

	buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	buf_ctl.s.fpa_wait = gbl_cfg->fpa_wait;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, buf_ctl.u64);

	cvmx_pki_write_global_parse(node, gbl_cfg->gbl_pen);
	cvmx_pki_write_tag_secret(node, gbl_cfg->tag_secret);
	cvmx_pki_write_frame_len(node, 0, gbl_cfg->frm_len[0]);
	cvmx_pki_write_frame_len(node, 1, gbl_cfg->frm_len[1]);
}

/**
 * This function reads per pkind parameters in hardware which defines how
 * the incoming packet is processed.
 *
 * @param node  Node number.
 * @param pkind  PKI supports a large number of incoming interfaces and packets
 *     arriving on different interfaces or channels may want to be processed
 *     differently. PKI uses the pkind to determine how the incoming packet
 *     is processed.
 * @param pkind_cfg  Pointer to struct conatining pkind configuration read
 *     from the hardware.
 */
int cvmx_pki_read_pkind_config(int node, int pkind,
	struct cvmx_pki_pkind_config *pkind_cfg)
{
	int cluster = 0;
	uint64_t cl_mask;
	cvmx_pki_pkindx_icgsel_t icgsel;
	cvmx_pki_clx_pkindx_style_t pstyle;
	cvmx_pki_icgx_cfg_t icg_cfg;
	cvmx_pki_clx_pkindx_cfg_t pcfg;
	cvmx_pki_clx_pkindx_skip_t skip;

	icgsel.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind));
	icg_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(icgsel.s.icg));
	pkind_cfg->cluster_grp = (uint8_t)icgsel.s.icg;
	cl_mask = (uint64_t)icg_cfg.s.clusters;
	cluster = __builtin_ffsll(cl_mask) - 1;

	pstyle.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
	pkind_cfg->initial_parse_mode = pstyle.s.pm;
	pkind_cfg->initial_style = pstyle.s.style;

	pcfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster));
	pkind_cfg->fcs_pres = pcfg.s.fcs_pres;
	pkind_cfg->parse_en.inst_hdr = pcfg.s.inst_hdr;
	pkind_cfg->parse_en.mpls_en = pcfg.s.mpls_en;
	pkind_cfg->parse_en.lg_custom = pcfg.s.lg_custom;
	pkind_cfg->parse_en.fulc_en = pcfg.s.fulc_en;
	pkind_cfg->parse_en.dsa_en = pcfg.s.dsa_en;
	pkind_cfg->parse_en.hg2_en = pcfg.s.hg2_en;
	pkind_cfg->parse_en.hg_en = pcfg.s.hg_en;

	skip.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_SKIP(pkind, cluster));
	pkind_cfg->fcs_skip = skip.s.fcs_skip;
	pkind_cfg->inst_skip = skip.s.inst_skip;
	return 0;
}

/**
 * This function writes per pkind parameters in hardware which defines how
 * the incoming packet is processed.
 *
 * @param node  Node number.
 * @param pkind  PKI supports a large number of incoming interfaces and packets
 *     arriving on different interfaces or channels may want to be processed
 *     differently. PKI uses the pkind to determine how the incoming
 *     packet is processed.
 * @param pkind_cfg  Pointer to struct conatining pkind configuration need
 *     to be written in the hardware.
 */
int cvmx_pki_write_pkind_config(int node, int pkind,
	struct cvmx_pki_pkind_config *pkind_cfg)
{
	unsigned cluster = 0;
	uint64_t cluster_mask;
	cvmx_pki_pkindx_icgsel_t icgsel;
	cvmx_pki_clx_pkindx_style_t pstyle;
	cvmx_pki_icgx_cfg_t icg_cfg;
	cvmx_pki_clx_pkindx_cfg_t pcfg;
	cvmx_pki_clx_pkindx_skip_t skip;

	if (pkind >= CVMX_PKI_NUM_PKIND ||
		pkind_cfg->cluster_grp >= CVMX_PKI_NUM_CLUSTER_GROUP ||
		pkind_cfg->initial_style >= CVMX_PKI_NUM_FINAL_STYLE) {
		cvmx_dprintf("ERROR: Configuring PKIND pkind = %d cluster_group = %d style = %d\n",
			     pkind, pkind_cfg->cluster_grp, pkind_cfg->initial_style);
		return -1;
	}
	icgsel.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind));
	icgsel.s.icg = pkind_cfg->cluster_grp;
	cvmx_write_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind), icgsel.u64);

	icg_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(pkind_cfg->cluster_grp));
	cluster_mask = (uint64_t)icg_cfg.s.clusters;
	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			pstyle.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
			pstyle.s.pm = pkind_cfg->initial_parse_mode;
			pstyle.s.style = pkind_cfg->initial_style;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster), pstyle.u64);

			pcfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster));
			pcfg.s.fcs_pres = pkind_cfg->fcs_pres;
			pcfg.s.inst_hdr = pkind_cfg->parse_en.inst_hdr;
			pcfg.s.mpls_en = pkind_cfg->parse_en.mpls_en;
			pcfg.s.lg_custom = pkind_cfg->parse_en.lg_custom;
			pcfg.s.fulc_en = pkind_cfg->parse_en.fulc_en;
			pcfg.s.dsa_en = pkind_cfg->parse_en.dsa_en;
			pcfg.s.hg2_en = pkind_cfg->parse_en.hg2_en;
			pcfg.s.hg_en = pkind_cfg->parse_en.hg_en;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster), pcfg.u64);

			skip.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_SKIP(pkind, cluster));
			skip.s.fcs_skip = pkind_cfg->fcs_skip;
			skip.s.inst_skip = pkind_cfg->inst_skip;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PKINDX_SKIP(pkind, cluster), skip.u64);
		}
		cluster++;
	}
	return 0;
}

/**
 * This function reads parameters associated with tag configuration in hardware.
 * Only first cluster in the group is used.
 *
 * @param node  Node number.
 * @param style  Style to configure tag for.
 * @param cluster_mask	Mask of clusters to configure the style for.
 * @param tag_cfg  Pointer to tag configuration struct.
 */
void cvmx_pki_read_tag_config(int node, int style, uint64_t cluster_mask,
	struct cvmx_pki_style_tag_cfg *tag_cfg)
{
	int mask, tag_idx, index;
	cvmx_pki_clx_stylex_cfg2_t style_cfg2;
	cvmx_pki_clx_stylex_alg_t style_alg;
	cvmx_pki_stylex_tag_sel_t tag_sel;
	cvmx_pki_tag_incx_ctl_t tag_ctl;
	cvmx_pki_tag_incx_mask_t tag_mask;
	int cluster = __builtin_ffsll(cluster_mask) - 1;

	style_cfg2.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
	style_alg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster));

	/* 7-Tuple Tag: */
	tag_cfg->tag_fields.layer_g_src = style_cfg2.s.tag_src_lg;
	tag_cfg->tag_fields.layer_f_src = style_cfg2.s.tag_src_lf;
	tag_cfg->tag_fields.layer_e_src = style_cfg2.s.tag_src_le;
	tag_cfg->tag_fields.layer_d_src = style_cfg2.s.tag_src_ld;
	tag_cfg->tag_fields.layer_c_src = style_cfg2.s.tag_src_lc;
	tag_cfg->tag_fields.layer_b_src = style_cfg2.s.tag_src_lb;
	tag_cfg->tag_fields.layer_g_dst = style_cfg2.s.tag_dst_lg;
	tag_cfg->tag_fields.layer_f_dst = style_cfg2.s.tag_dst_lf;
	tag_cfg->tag_fields.layer_e_dst = style_cfg2.s.tag_dst_le;
	tag_cfg->tag_fields.layer_d_dst = style_cfg2.s.tag_dst_ld;
	tag_cfg->tag_fields.layer_c_dst = style_cfg2.s.tag_dst_lc;
	tag_cfg->tag_fields.layer_b_dst = style_cfg2.s.tag_dst_lb;
	tag_cfg->tag_fields.tag_vni = style_alg.s.tag_vni;
	tag_cfg->tag_fields.tag_gtp = style_alg.s.tag_gtp;
	tag_cfg->tag_fields.tag_spi = style_alg.s.tag_spi;
	tag_cfg->tag_fields.tag_sync = style_alg.s.tag_syn;
	tag_cfg->tag_fields.ip_prot_nexthdr = style_alg.s.tag_pctl;
	tag_cfg->tag_fields.second_vlan = style_alg.s.tag_vs1;
	tag_cfg->tag_fields.first_vlan = style_alg.s.tag_vs0;
	tag_cfg->tag_fields.mpls_label = style_alg.s.tag_mpls0;
	tag_cfg->tag_fields.input_port = style_alg.s.tag_prt;

	/* Custom-Mask Tag: */
	tag_sel.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_TAG_SEL(style));
	for (mask = 0; mask < 4; mask++) {
		tag_cfg->mask_tag[mask].enable = (style_cfg2.s.tag_inc & (1 << mask)) != 0;
		switch (mask) {
		case 0: tag_idx = tag_sel.s.tag_idx0; break;
		case 1: tag_idx = tag_sel.s.tag_idx1; break;
		case 2: tag_idx = tag_sel.s.tag_idx2; break;
		case 3: tag_idx = tag_sel.s.tag_idx3; break;
		}
		index = tag_idx * 4 + mask;
		tag_mask.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_INCX_MASK(index));
		tag_cfg->mask_tag[mask].val = tag_mask.s.en;
 		tag_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_INCX_CTL(index));
		tag_cfg->mask_tag[mask].base = tag_ctl.s.ptr_sel;
		tag_cfg->mask_tag[mask].offset = tag_ctl.s.offset;
	}
}

/**
 * This function writes/configures parameters associated with tag configuration in
 * hardware. In Custom-Mask Tagging, all four masks use the same base index
 * to access Tag Control and Tag Mask registers.
 *
 * @param node  Node number.
 * @param style  Style to configure tag for.
 * @param cluster_mask  Mask of clusters to configure the style for.
 * @param tag_cfg  Pointer to taf configuration struct.
 */
void cvmx_pki_write_tag_config(int node, int style, uint64_t cluster_mask,
			       struct cvmx_pki_style_tag_cfg *tag_cfg)
{
	int mask, index, tag_idx, mtag_en = 0;
	unsigned cluster = 0;
	cvmx_pki_clx_stylex_cfg2_t scfg2;
	cvmx_pki_clx_stylex_alg_t style_alg;
	cvmx_pki_tag_incx_ctl_t tag_ctl;
	cvmx_pki_tag_incx_mask_t tag_mask;
	cvmx_pki_stylex_tag_sel_t tag_sel;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			/* 7-Tuple Tag: */
			scfg2.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
			scfg2.s.tag_src_lg = tag_cfg->tag_fields.layer_g_src;
			scfg2.s.tag_src_lf = tag_cfg->tag_fields.layer_f_src;
			scfg2.s.tag_src_le = tag_cfg->tag_fields.layer_e_src;
			scfg2.s.tag_src_ld = tag_cfg->tag_fields.layer_d_src;
			scfg2.s.tag_src_lc = tag_cfg->tag_fields.layer_c_src;
			scfg2.s.tag_src_lb = tag_cfg->tag_fields.layer_b_src;
			scfg2.s.tag_dst_lg = tag_cfg->tag_fields.layer_g_dst;
			scfg2.s.tag_dst_lf = tag_cfg->tag_fields.layer_f_dst;
			scfg2.s.tag_dst_le = tag_cfg->tag_fields.layer_e_dst;
			scfg2.s.tag_dst_ld = tag_cfg->tag_fields.layer_d_dst;
			scfg2.s.tag_dst_lc = tag_cfg->tag_fields.layer_c_dst;
			scfg2.s.tag_dst_lb = tag_cfg->tag_fields.layer_b_dst;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster), scfg2.u64);

			style_alg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
			style_alg.s.tag_vni = tag_cfg->tag_fields.tag_vni;
			style_alg.s.tag_gtp = tag_cfg->tag_fields.tag_gtp;
			style_alg.s.tag_spi = tag_cfg->tag_fields.tag_spi;
			style_alg.s.tag_syn = tag_cfg->tag_fields.tag_sync;
			style_alg.s.tag_pctl = tag_cfg->tag_fields.ip_prot_nexthdr;
			style_alg.s.tag_vs1 = tag_cfg->tag_fields.second_vlan;
			style_alg.s.tag_vs0 = tag_cfg->tag_fields.first_vlan;
			style_alg.s.tag_mpls0 = tag_cfg->tag_fields.mpls_label;
			style_alg.s.tag_prt = tag_cfg->tag_fields.input_port;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster), style_alg.u64);

			/* Custom-Mask Tag (Part 1): */
			for (mask = 0; mask < 4; mask++) {
				if (tag_cfg->mask_tag[mask].enable)
					mtag_en++;
			}
			if (mtag_en) {
				scfg2.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
				scfg2.s.tag_inc = 0;
				for (mask = 0; mask < 4; mask++) {
					if (tag_cfg->mask_tag[mask].enable)
						scfg2.s.tag_inc |= 1 << mask;
				}
				cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster), scfg2.u64);
			}
		}
		cluster++;
	}
	/* Custom-Mask Tag (Part 2): */
	if (mtag_en) {
		if ((tag_idx = cvmx_pki_mtag_idx_alloc(node, -1)) < 0)
			return;

		tag_sel.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_TAG_SEL(style));
		for (mask = 0; mask < 4; mask++) {
			if (tag_cfg->mask_tag[mask].enable) {
				switch (mask) {
				case 0: tag_sel.s.tag_idx0 = tag_idx; break;
				case 1: tag_sel.s.tag_idx1 = tag_idx; break;
				case 2: tag_sel.s.tag_idx2 = tag_idx; break;
				case 3: tag_sel.s.tag_idx3 = tag_idx; break;
				}
				index = tag_idx * 4 + mask;
				tag_mask.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_INCX_MASK(index));
				tag_mask.s.en = tag_cfg->mask_tag[mask].val;
				cvmx_write_csr_node(node, CVMX_PKI_TAG_INCX_MASK(index), tag_mask.u64);

				tag_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_INCX_CTL(index));
				tag_ctl.s.ptr_sel = tag_cfg->mask_tag[mask].base;
				tag_ctl.s.offset = tag_cfg->mask_tag[mask].offset;
				cvmx_write_csr_node(node, CVMX_PKI_TAG_INCX_CTL(index), tag_ctl.u64);
			}
		}
		cvmx_write_csr_node(node, CVMX_PKI_STYLEX_TAG_SEL(style), tag_sel.u64);
	}
}

/**
 * This function reads parameters associated with style in hardware.
 *
 * @param node  Node number.
 * @param style 	Style to read from.
 * @param cluster_mask	Mask of clusters style belongs to.
 * @param style_cfg	 Pointer to style config struct.
 */
void cvmx_pki_read_style_config(int node, int style, uint64_t cluster_mask,
	struct cvmx_pki_style_config *style_cfg)
{
	cvmx_pki_clx_stylex_cfg_t scfg;
	cvmx_pki_clx_stylex_cfg2_t scfg2;
	cvmx_pki_clx_stylex_alg_t style_alg;
	cvmx_pki_stylex_buf_t style_buf;
	int cluster = __builtin_ffsll(cluster_mask) - 1;

	scfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
	scfg2.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
	style_alg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
	style_buf.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(style));

	style_cfg->parm_cfg.ip6_udp_opt = scfg.s.ip6_udp_opt;
	style_cfg->parm_cfg.lenerr_en = scfg.s.lenerr_en;
	style_cfg->parm_cfg.lenerr_eqpad = scfg.s.lenerr_eqpad;
	style_cfg->parm_cfg.maxerr_en = scfg.s.maxerr_en;
	style_cfg->parm_cfg.minerr_en = scfg.s.minerr_en;
	style_cfg->parm_cfg.fcs_chk = scfg.s.fcs_chk;
	style_cfg->parm_cfg.fcs_strip = scfg.s.fcs_strip;
	style_cfg->parm_cfg.minmax_sel = scfg.s.minmax_sel;
	style_cfg->parm_cfg.qpg_base = scfg.s.qpg_base;
	style_cfg->parm_cfg.qpg_dis_padd = scfg.s.qpg_dis_padd;
	style_cfg->parm_cfg.qpg_dis_aura = scfg.s.qpg_dis_aura;
	style_cfg->parm_cfg.qpg_dis_grp = scfg.s.qpg_dis_grp;
	style_cfg->parm_cfg.qpg_dis_grptag = scfg.s.qpg_dis_grptag;
	style_cfg->parm_cfg.rawdrp = scfg.s.rawdrp;
	style_cfg->parm_cfg.force_drop = scfg.s.drop;
	style_cfg->parm_cfg.nodrop = scfg.s.nodrop;

	style_cfg->parm_cfg.len_lg = scfg2.s.len_lg;
	style_cfg->parm_cfg.len_lf = scfg2.s.len_lf;
	style_cfg->parm_cfg.len_le = scfg2.s.len_le;
	style_cfg->parm_cfg.len_ld = scfg2.s.len_ld;
	style_cfg->parm_cfg.len_lc = scfg2.s.len_lc;
	style_cfg->parm_cfg.len_lb = scfg2.s.len_lb;
	style_cfg->parm_cfg.csum_lg = scfg2.s.csum_lg;
	style_cfg->parm_cfg.csum_lf = scfg2.s.csum_lf;
	style_cfg->parm_cfg.csum_le = scfg2.s.csum_le;
	style_cfg->parm_cfg.csum_ld = scfg2.s.csum_ld;
	style_cfg->parm_cfg.csum_lc = scfg2.s.csum_lc;
	style_cfg->parm_cfg.csum_lb = scfg2.s.csum_lb;

	style_cfg->parm_cfg.qpg_qos = style_alg.s.qpg_qos;
	style_cfg->parm_cfg.tag_type = style_alg.s.tt;
	style_cfg->parm_cfg.apad_nip = style_alg.s.apad_nip;
	style_cfg->parm_cfg.qpg_port_sh = style_alg.s.qpg_port_sh;
	style_cfg->parm_cfg.qpg_port_msb = style_alg.s.qpg_port_msb;
	style_cfg->parm_cfg.wqe_vs = style_alg.s.wqe_vs;

	style_cfg->parm_cfg.pkt_lend = style_buf.s.pkt_lend;
	style_cfg->parm_cfg.wqe_hsz = style_buf.s.wqe_hsz;
	style_cfg->parm_cfg.wqe_skip = style_buf.s.wqe_skip * 128;
	style_cfg->parm_cfg.first_skip = style_buf.s.first_skip * 8;
	style_cfg->parm_cfg.later_skip = style_buf.s.later_skip * 8;
	style_cfg->parm_cfg.cache_mode = style_buf.s.opc_mode;
	style_cfg->parm_cfg.mbuff_size = style_buf.s.mb_size * 8;
	style_cfg->parm_cfg.dis_wq_dat = style_buf.s.dis_wq_dat;

	cvmx_pki_read_tag_config(node, style, cluster_mask, &style_cfg->tag_cfg);
}

/**
 * This function writes/configures parameters associated with style in hardware.
 *
 * @param node  Node number.
 * @param style  Style to configure.
 * @param cluster_mask  Mask of clusters to configure the style for.
 * @param style_cfg	 Pointer to style config struct.
 */
void cvmx_pki_write_style_config(int node, uint64_t style, uint64_t cluster_mask,
	struct cvmx_pki_style_config *style_cfg)
{
	cvmx_pki_clx_stylex_cfg_t scfg;
	cvmx_pki_clx_stylex_cfg2_t scfg2;
	cvmx_pki_clx_stylex_alg_t style_alg;
	cvmx_pki_stylex_buf_t style_buf;
	unsigned cluster = 0;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			scfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
			scfg.s.ip6_udp_opt = style_cfg->parm_cfg.ip6_udp_opt;
			scfg.s.lenerr_en = style_cfg->parm_cfg.lenerr_en;
			scfg.s.lenerr_eqpad = style_cfg->parm_cfg.lenerr_eqpad;
			scfg.s.maxerr_en = style_cfg->parm_cfg.maxerr_en;
			scfg.s.minerr_en = style_cfg->parm_cfg.minerr_en;
			scfg.s.fcs_chk = style_cfg->parm_cfg.fcs_chk;
			scfg.s.fcs_strip = style_cfg->parm_cfg.fcs_strip;
			scfg.s.minmax_sel = style_cfg->parm_cfg.minmax_sel;
			scfg.s.qpg_base = style_cfg->parm_cfg.qpg_base;
			scfg.s.qpg_dis_padd = style_cfg->parm_cfg.qpg_dis_padd;
			scfg.s.qpg_dis_aura = style_cfg->parm_cfg.qpg_dis_aura;
			scfg.s.qpg_dis_grp = style_cfg->parm_cfg.qpg_dis_grp;
			scfg.s.qpg_dis_grptag = style_cfg->parm_cfg.qpg_dis_grptag;
			scfg.s.rawdrp = style_cfg->parm_cfg.rawdrp;
			scfg.s.drop = style_cfg->parm_cfg.force_drop;
			scfg.s.nodrop = style_cfg->parm_cfg.nodrop;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster), scfg.u64);

			scfg2.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
			scfg2.s.len_lg = style_cfg->parm_cfg.len_lg;
			scfg2.s.len_lf = style_cfg->parm_cfg.len_lf;
			scfg2.s.len_le = style_cfg->parm_cfg.len_le;
			scfg2.s.len_ld = style_cfg->parm_cfg.len_ld;
			scfg2.s.len_lc = style_cfg->parm_cfg.len_lc;
			scfg2.s.len_lb = style_cfg->parm_cfg.len_lb;
			scfg2.s.csum_lg = style_cfg->parm_cfg.csum_lg;
			scfg2.s.csum_lf = style_cfg->parm_cfg.csum_lf;
			scfg2.s.csum_le = style_cfg->parm_cfg.csum_le;
			scfg2.s.csum_ld = style_cfg->parm_cfg.csum_ld;
			scfg2.s.csum_lc = style_cfg->parm_cfg.csum_lc;
			scfg2.s.csum_lb = style_cfg->parm_cfg.csum_lb;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster), scfg2.u64);

			style_alg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
			style_alg.s.qpg_qos = style_cfg->parm_cfg.qpg_qos;
			style_alg.s.tt = style_cfg->parm_cfg.tag_type;
			style_alg.s.apad_nip = style_cfg->parm_cfg.apad_nip;
			style_alg.s.qpg_port_sh = style_cfg->parm_cfg.qpg_port_sh;
			style_alg.s.qpg_port_msb = style_cfg->parm_cfg.qpg_port_msb;
			style_alg.s.wqe_vs = style_cfg->parm_cfg.wqe_vs;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster), style_alg.u64);
		}
		cluster++;
	}
	style_buf.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(style));
	style_buf.s.pkt_lend = style_cfg->parm_cfg.pkt_lend;
	style_buf.s.wqe_hsz = style_cfg->parm_cfg.wqe_hsz;
	style_buf.s.wqe_skip = (style_cfg->parm_cfg.wqe_skip)/128;
	style_buf.s.first_skip = (style_cfg->parm_cfg.first_skip)/8;
	style_buf.s.later_skip = style_cfg->parm_cfg.later_skip/8;
	style_buf.s.opc_mode = style_cfg->parm_cfg.cache_mode;
	style_buf.s.mb_size = (style_cfg->parm_cfg.mbuff_size)/8;
	style_buf.s.dis_wq_dat = style_cfg->parm_cfg.dis_wq_dat;
	cvmx_write_csr_node(node, CVMX_PKI_STYLEX_BUF(style), style_buf.u64);

	cvmx_pki_write_tag_config(node, style, cluster_mask, &style_cfg->tag_cfg);
}

/**
 * This function reads qpg entry at specified offset from qpg table.
 *
 * @param node  Node number.
 * @param offset  Offset in qpg table to read from.
 * @param qpg_cfg  Pointer to structure containing qpg values.
 */
int cvmx_pki_read_qpg_entry(int node, int offset, struct cvmx_pki_qpg_config *qpg_cfg)
{
	cvmx_pki_qpg_tblx_t qpg_tbl;
	if (offset >= CVMX_PKI_NUM_QPG_ENTRY) {
		cvmx_dprintf("ERROR: qpg offset %d is >= 2048\n", offset);
		return -1;
	}
	qpg_tbl.u64 = cvmx_read_csr_node(node, CVMX_PKI_QPG_TBLX(offset));
	qpg_cfg->aura_num = qpg_tbl.s.laura;
	qpg_cfg->port_add = qpg_tbl.s.padd;
	qpg_cfg->grp_ok = qpg_tbl.s.grp_ok;
	qpg_cfg->grp_bad = qpg_tbl.s.grp_bad;
	qpg_cfg->grptag_ok = qpg_tbl.s.grptag_ok;
	qpg_cfg->grptag_bad = qpg_tbl.s.grptag_bad;
	return 0;
}

/**
 * This function writes qpg entry at specified offset in qpg table.
 *
 * @param node  Node number.
 * @param offset  Offset in qpg table to read from.
 * @param qpg_cfg  Pointer to structure containing qpg values.
 */
void cvmx_pki_write_qpg_entry(int node, int offset, struct cvmx_pki_qpg_config *qpg_cfg)
{
	cvmx_pki_qpg_tblx_t qpg_tbl;
	qpg_tbl.u64 = cvmx_read_csr_node(node, CVMX_PKI_QPG_TBLX(offset));
	qpg_tbl.s.padd = qpg_cfg->port_add;
	qpg_tbl.s.laura = qpg_cfg->aura_num;
	qpg_tbl.s.grp_ok = qpg_cfg->grp_ok;
	qpg_tbl.s.grp_bad = qpg_cfg->grp_bad;
	qpg_tbl.s.grptag_ok = qpg_cfg->grptag_ok;
	qpg_tbl.s.grptag_bad = qpg_cfg->grptag_bad;
	cvmx_write_csr_node(node, CVMX_PKI_QPG_TBLX(offset), qpg_tbl.u64);
}

/**
 * This function writes pcam entry at given offset in pcam table in hardware
 *
 * @param node  Node number.
 * @param index  Offset in pcam table.
 * @param cluster_mask	Mask of clusters in which to write pcam entry.
 * @param input  Input keys to pcam match passed as struct.
 * @param action  PCAM match action passed as struct.
 */
int cvmx_pki_pcam_write_entry(int node, int index, uint64_t cluster_mask,
	struct cvmx_pki_pcam_input input, struct cvmx_pki_pcam_action action)
{
	int bank;
	unsigned cluster = 0;
	cvmx_pki_clx_pcamx_termx_t	term;
	cvmx_pki_clx_pcamx_matchx_t	match;
	cvmx_pki_clx_pcamx_actionx_t act;

	if (index >= CVMX_PKI_TOTAL_PCAM_ENTRY) {
		cvmx_dprintf("\nERROR: Invalid pcam entry %d\n", index);
		return -1;
	}
	bank = (int)(input.field & 0x01);
	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			term.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index));
			term.s.valid = 0;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index), term.u64);
			match.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index));
			match.s.data1 = input.data & input.data_mask;
			match.s.data0 = (~input.data) & input.data_mask;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index), match.u64);

			act.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index));
			act.s.pmc = action.parse_mode_chg;
			act.s.style_add = action.style_add;
			act.s.pf = action.parse_flag_set;
			act.s.setty = action.layer_type_set;
			act.s.advance = action.pointer_advance;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index), act.u64);

			term.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index));
			term.s.term1 = input.field & input.field_mask;
			term.s.term0 = (~input.field) & input.field_mask;
			term.s.style1 = input.style & input.style_mask;
			term.s.style0 = (~input.style) & input.style_mask;
			term.s.valid = 1;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index), term.u64);
		}
		cluster++;
	}
	return 0;
}

/**
 * Enables/Disabled QoS (RED Drop, Tail Drop & backpressure) for the PKI aura.
 *
 * @param node  Node number
 * @param aura  To enable/disable QoS on.
 * @param ena_red  Enable/Disable RED drop between pass and drop level
 *    1-enable 0-disable
 * @param ena_drop  Enable/disable tail drop when max drop level exceeds
 *    1-enable 0-disable
 * @param ena_bp  Enable/Disable asserting backpressure on bpid when max
 *    DROP level exceeds.
 *    1-enable 0-disable
 */
int cvmx_pki_enable_aura_qos(int node, int aura, bool ena_red,
	bool ena_drop, bool ena_bp)
{
	cvmx_pki_aurax_cfg_t aura_cfg;

	if (aura >= CVMX_PKI_NUM_AURA) {
		cvmx_dprintf("ERROR: %s aura = %d\n", __func__, aura);
		return -1;
	}
	aura_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_AURAX_CFG(aura));
	aura_cfg.s.ena_red = ena_red;
	aura_cfg.s.ena_drop = ena_drop;
	aura_cfg.s.ena_bp = ena_bp;
	cvmx_write_csr_node(node, CVMX_PKI_AURAX_CFG(aura), aura_cfg.u64);
	return 0;
}

/**
 * Configures the bpid on which, specified aura will assert backpressure.
 * Each bpid receives backpressure from auras.
 * Multiple auras can backpressure single bpid.
 *
 * @param node  Node number.
 * @param aura  Number which will assert backpressure on that bpid.
 * @param bpid  To assert backpressure on.
 */
int cvmx_pki_write_aura_bpid(int node, int aura, int bpid)
{
	int i, cnt, ena_bp;
	cvmx_pki_aurax_cfg_t aura_cfg;

	if (aura >= CVMX_PKI_NUM_AURA || bpid >= CVMX_PKI_NUM_BPID) {
		cvmx_dprintf("ERROR: aura=%d or bpid=%d is out or range\n", aura, bpid);
		return -1;
	}
	if(OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		/* Workaround for Errata PKI-24364:
		 * Inform about assigning the same BPID to multiple auras
		 * having different ENA_BP.
		 */
		aura_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_AURAX_CFG(aura));
		ena_bp = aura_cfg.s.ena_bp;
		for (i = 0, cnt = 1; i < CVMX_PKI_NUM_AURA; i++) {
			if (i == aura)
				continue;
			aura_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_AURAX_CFG(i));
			if (aura_cfg.s.bpid == bpid && aura_cfg.s.ena_bp != ena_bp)
				cnt++;
		}
		if (cnt > 1)
			cvmx_dprintf("WARNING: BPID(%d) is used by %d AURAs.\n"
				"\tEnable|disable backpressure for all AURAs on this BPID.\n",
				bpid, cnt);
	}
	aura_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_AURAX_CFG(aura));
	aura_cfg.s.bpid = bpid;
	cvmx_write_csr_node(node, CVMX_PKI_AURAX_CFG(aura), aura_cfg.u64);
	return 0;
}

/**
 * Configures the channel which will receive backpressure
 * from the specified bpid.
 * Each channel listens for backpressure on a specific bpid.
 * Each bpid can backpressure multiple channels.
 *
 * @param node  Node number.
 * @param bpid  BPID from which, channel will receive backpressure.
 * @param channel  Channel number to receive backpressue.
 */
int cvmx_pki_write_channel_bpid(int node, int channel, int bpid)
{
	cvmx_pki_chanx_cfg_t chan_cfg;

	if (channel >= CVMX_PKI_NUM_CHANNEL || bpid >= CVMX_PKI_NUM_BPID) {
		cvmx_dprintf("ERROR: %s channel = %d bpid = %d\n", __func__, channel, bpid);
		return -1;
	}
	chan_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CHANX_CFG(channel));
	chan_cfg.s.bpid = bpid;
	cvmx_write_csr_node(node, CVMX_PKI_CHANX_CFG(channel), chan_cfg.u64);
	return 0;
}

/**
 * This function gives the initial style used by that pkind.
 *
 * @param node	Node number.
 * @param pkind	PKIND number.
 */
int cvmx_pki_get_pkind_style(int node, int pkind)
{
	int cluster = 0;
	cvmx_pki_clx_pkindx_style_t style;

	style.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
	return style.s.style;
}

/**
 * This function sets the wqe buffer mode. First packet data buffer can
 * reside either in same buffer as wqe OR it can go in separate buffer.
 * If used the later mode, make sure software allocate enough buffers to
 * now have wqe separate from packet data.
 *
 * @param node  Node number.
 * @param style	 Style to configure.
 * @param pkt_outside_wqe	0 = The packet link pointer will be at word [FIRST_SKIP]
 *    immediately followed by packet data, in the same buffer as the work queue entry.
 *    1 = The packet link pointer will be at word [FIRST_SKIP] in a new buffer
 *    separate from the work queue entry. Words following the WQE in the same
 *    cache line will be zeroed, other lines in the buffer will not be modified
 *    and will retain stale data (from the buffer’s previous use). This setting may
 *    decrease the peak PKI performance by up to half on small packets.
 */
void cvmx_pki_set_wqe_mode(int node, uint64_t style, bool pkt_outside_wqe)
{
	cvmx_pki_stylex_buf_t style_buf;

	style_buf.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(style));
	style_buf.s.dis_wq_dat = pkt_outside_wqe;
	cvmx_write_csr_node(node, CVMX_PKI_STYLEX_BUF(style), style_buf.u64);
}

/**
 * This function sets the Packet mode of all ports and styles to little-endian.
 * It Changes write operations of packet data to L2C to be in little-endian.
 * Does not change the WQE header format, which is properly endian neutral.
 *
 * @param node  Node number.
 * @param style  Style to configure.
 */
void cvmx_pki_set_little_endian(int node, uint64_t style)
{
	cvmx_pki_stylex_buf_t style_buf;

	style_buf.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(style));
	style_buf.s.pkt_lend = 1;
	cvmx_write_csr_node(node, CVMX_PKI_STYLEX_BUF(style), style_buf.u64);
}

/**
 * Enables/Disables fcs check and fcs stripping on the pkind.
 *
 * @param node  Node number
 * @param pknd  PKIND to apply settings on.
 * @param fcs_chk  Enable/disable fcs check.
 *    1 = enable fcs error check.
 *    0 = disable fcs error check.
 * @param fcs_strip	 Strip L2 FCS bytes from packet, decrease WQE[LEN] by 4 bytes
 *    1 = strip L2 FCS.
 *    0 = Do not strip L2 FCS.
 */
void cvmx_pki_endis_fcs_check(int node, int pknd, bool fcs_chk, bool fcs_strip)
{
	int style;
	unsigned cluster = 0;
	cvmx_pki_clx_pkindx_style_t pstyle;
	cvmx_pki_clx_stylex_cfg_t style_cfg;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		pstyle.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pknd, cluster));
		style = pstyle.s.style;
		style_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
		style_cfg.s.fcs_chk = fcs_chk;
		style_cfg.s.fcs_strip = fcs_strip;
		cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster), style_cfg.u64);
		cluster++;
	}
}

/**
 * Enables/Disables l2 length error check and max & min frame length checks
 *
 * @param node  Node number
 * @param pknd  PKIND to disable error for.
 * @param l2len_err  L2 length error check enable.
 * @param maxframe_err  Max frame error check enable.
 * @param minframe_err  Min frame error check enable.
 *    1 = Enabel err checks
 *    0 = Disable error checks
 */
void cvmx_pki_endis_l2_errs(int node, int pknd, bool l2len_err,
			 bool maxframe_err, bool minframe_err)
{
	int style;
	unsigned cluster = 0;
	cvmx_pki_clx_pkindx_style_t pstyle;
	cvmx_pki_clx_stylex_cfg_t style_cfg;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		pstyle.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pknd, cluster));
		style = pstyle.s.style;
		style_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
		style_cfg.s.lenerr_en = l2len_err;
		style_cfg.s.maxerr_en = maxframe_err;
		style_cfg.s.minerr_en = minframe_err;
		cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster), style_cfg.u64);
		cluster++;
	}
}

/**
 * Disables maximum & minimum frame length checks
 *
 * @param node  Node number.
 * @param pknd  PKIND to disable error for.
 */
void cvmx_pki_dis_frame_len_chk(int node, int pknd)
{
	int style;
	unsigned cluster = 0;
	cvmx_pki_clx_pkindx_style_t pstyle;
	cvmx_pki_clx_stylex_cfg_t style_cfg;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		pstyle.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pknd, cluster));
		style = pstyle.s.style;
		style_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
		style_cfg.s.maxerr_en = 0;
		style_cfg.s.minerr_en = 0;
		cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster), style_cfg.u64);
		cluster++;
	}
}

/**
 * This function shows the qpg table entries, read directly from hardware.
 *
 * @param node	Node number
 * @param num_entry  Number of entries to show
 */
void cvmx_pki_show_qpg_entries(int node, uint16_t num_entry)
{
	int index;
	cvmx_pki_qpg_tblx_t qpg_tbl;

	if (num_entry > CVMX_PKI_NUM_QPG_ENTRY)
		num_entry = CVMX_PKI_NUM_QPG_ENTRY;
	for (index = 0; index < num_entry; index++) {
		qpg_tbl.u64 = cvmx_read_csr_node(node, CVMX_PKI_QPG_TBLX(index));
		cvmx_dprintf("\n%d	", index);
		cvmx_dprintf("PADD %-16lu", (unsigned long)qpg_tbl.s.padd);
		cvmx_dprintf("GRP_OK %-16lu", (unsigned long)qpg_tbl.s.grp_ok);
		cvmx_dprintf("GRP_BAD %-16lu", (unsigned long)qpg_tbl.s.grp_bad);
		cvmx_dprintf("LAURA %-16lu", (unsigned long)qpg_tbl.s.laura);
	}
}

/**
 * This function shows the pcam table in raw format,
 * read directly from hardware.
 *
 * @param node  Node number
 */
void cvmx_pki_show_pcam_entries(int node)
{
	int cluster;
	int index;
	int bank;

	for (cluster = 0; cluster < (int)CVMX_PKI_NUM_CLUSTER; cluster++) {
		for (bank = 0; bank < 2; bank++) {
			cvmx_dprintf("\n--------------Cluster %1d Bank %1d-------------\n",
				cluster, bank);
			cvmx_dprintf("index         TERM                 DATA,                ACTION");
			for (index = 0; index < CVMX_PKI_NUM_PCAM_ENTRY; index++) {
				cvmx_dprintf("\n%d", index);
				cvmx_dprintf("             %-16lx",
					(unsigned long)cvmx_read_csr_node(node,
						CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index)));
				cvmx_dprintf("     %-16lx",
					(unsigned long)cvmx_read_csr_node(node,
						CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index)));
				cvmx_dprintf("     %-16lx",
					(unsigned long)cvmx_read_csr_node(node,
						CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index)));
			}
		}
	}
}

/**
 * This function shows the valid entries in readable format,
 * read directly from hardware.
 *
 * @param node  Node number.
 */
void cvmx_pki_show_valid_pcam_entries(int node)
{
	int cluster;
	int index;
	int bank;
	cvmx_pki_clx_pcamx_termx_t	term;
	cvmx_pki_clx_pcamx_matchx_t	match;
	cvmx_pki_clx_pcamx_actionx_t act;

	for (cluster = 0; cluster < (int)CVMX_PKI_NUM_CLUSTER; cluster++) {
		for (bank = 0; bank < 2; bank++) {
			cvmx_dprintf("\n--------------Cluster %1d Bank %1d---------------------\n",
				cluster, bank);
			cvmx_dprintf("%-10s%-17s%-19s%-18s", "index",
				"TERM1:TERM0", "Style1:Style0", "Data1:Data0");
			cvmx_dprintf("%-6s", "ACTION[pmc:style_add:pf:setty:advance]");
			for (index = 0; index < CVMX_PKI_NUM_PCAM_ENTRY; index++) {
				term.u64 = cvmx_read_csr_node(node,
						CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index));
				if (term.s.valid) {
					match.u64 = cvmx_read_csr_node(node,
							CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index));
					act.u64 = cvmx_read_csr_node(node,
							CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index));
					cvmx_dprintf("\n%-13d", index);
					cvmx_dprintf("%-2x:%x", term.s.term1, term.s.term0);
					cvmx_dprintf("     	      %-2x:%x",
						term.s.style1, term.s.style0);
					cvmx_dprintf("        %-8x:%x", match.s.data1, match.s.data0);
					cvmx_dprintf("            %-2x:%-2x       :%-1x :%2x   :%-2x",
						act.s.pmc, act.s.style_add, act.s.pf, act.s.setty, act.s.advance);

				}
			}
		}
	}
}

/**
 * This function shows the pkind attributes in readable format,
 * read directly from hardware.
 *
 * @param node  Node number
 * @param pkind  PKIND info to print
 */
void cvmx_pki_show_pkind_attributes(int node, int pkind)
{
	unsigned cluster = 0;
	int index;
	cvmx_pki_pkindx_icgsel_t icgsel;
	cvmx_pki_clx_pkindx_style_t pstyle;
	cvmx_pki_icgx_cfg_t icg_cfg;
	cvmx_pki_clx_stylex_cfg_t style_cfg;
	cvmx_pki_clx_stylex_alg_t style_alg;

	if (pkind >= CVMX_PKI_NUM_PKIND) {
		cvmx_dprintf("ERROR: PKIND %d is beyond range\n", pkind);
		return;
	}
	cvmx_dprintf("Showing stats for pkind %d------------------\n", pkind);
	icgsel.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind));
	cvmx_dprintf("cluster group:	%d\n", icgsel.s.icg);
	icg_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(icgsel.s.icg));
	cvmx_dprintf("cluster mask of the group:	0x%x\n", icg_cfg.s.clusters);

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (icg_cfg.s.clusters & (0x01L << cluster)) {
			cvmx_dprintf("pkind %d config 0x%llx\n", pkind,
				(unsigned long long)cvmx_read_csr_node(node,
					CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster)));
			pstyle.u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
			cvmx_dprintf("initial parse Mode: %d\n", pstyle.s.pm);
			cvmx_dprintf("initial_style: %d\n", pstyle.s.style);
			style_alg.u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_STYLEX_ALG(pstyle.s.style, cluster));
			cvmx_dprintf("style_alg: 0x%llx\n", (unsigned long long)style_alg.u64);
			style_cfg.u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_STYLEX_CFG(pstyle.s.style, cluster));
			cvmx_dprintf("style_cfg: 0x%llx\n", (unsigned long long)style_cfg.u64);
			cvmx_dprintf("style_cfg2: 0x%llx\n",
				(unsigned long long)cvmx_read_csr_node(node,
					CVMX_PKI_CLX_STYLEX_CFG2(pstyle.s.style, cluster)));
			cvmx_dprintf("style_buf: 0x%llx\n",
				(unsigned long long)cvmx_read_csr_node(node,
					CVMX_PKI_STYLEX_BUF(pstyle.s.style)));
			break;
		}
	}
	cvmx_dprintf("qpg base: %d\n", style_cfg.s.qpg_base);
	cvmx_dprintf("qpg qos: %d\n", style_alg.s.qpg_qos);
	for (index = 0; index < 8; index++) {
		cvmx_dprintf("qpg index %d: 0x%llx\n", (index+style_cfg.s.qpg_base),
			(unsigned long long)cvmx_read_csr_node(node,
				CVMX_PKI_QPG_TBLX(style_cfg.s.qpg_base+index)));
	}
}

#define READCORRECT(cnt, node, value, addr)	\
	{cnt = 0;	\
	while (value >= (1ull << 48) && cnt++ < 20) \
		value = cvmx_read_csr_node(node, addr); \
	if (cnt >= 20)  \
		cvmx_dprintf("count stuck for 0x%llx\n", (long long unsigned int)addr); }

/**
 * Get the status counters for index from PKI.
 *
 * @param node  Node number
 * @param index  PKIND number (if PKI_STATS_CTL:mode=0) or
 *    style(flow) number (if PKI_STATS_CTL:mode=1)
 * @param status   Where to put the results.
 */
void cvmx_pki_get_stats(int node, int index, struct cvmx_pki_port_stats *status)
{
	cvmx_pki_statx_stat0_t stat0;
	cvmx_pki_statx_stat1_t stat1;
	cvmx_pki_statx_stat2_t stat2;
	cvmx_pki_statx_stat3_t stat3;
	cvmx_pki_statx_stat4_t stat4;
	cvmx_pki_statx_stat5_t stat5;
	cvmx_pki_statx_stat6_t stat6;
	cvmx_pki_statx_stat7_t stat7;
	cvmx_pki_statx_stat8_t stat8;
	cvmx_pki_statx_stat9_t stat9;
	cvmx_pki_statx_stat10_t stat10;
	cvmx_pki_statx_stat11_t stat11;
	cvmx_pki_statx_stat14_t stat14;
	cvmx_pki_statx_stat15_t stat15;
	cvmx_pki_statx_stat16_t stat16;
	cvmx_pki_statx_stat17_t stat17;
	cvmx_pki_statx_hist0_t hist0;
	cvmx_pki_statx_hist1_t hist1;
	cvmx_pki_statx_hist2_t hist2;
	cvmx_pki_statx_hist3_t hist3;
	cvmx_pki_statx_hist4_t hist4;
	cvmx_pki_statx_hist5_t hist5;
	cvmx_pki_statx_hist6_t hist6;
	cvmx_pki_pkndx_inb_stat0_t inb_stat0;
	cvmx_pki_pkndx_inb_stat1_t inb_stat1;
	cvmx_pki_pkndx_inb_stat2_t inb_stat2;
	int cnt;

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* Accessing PKI stat registers can timeout based on the Errata
	   PKI-20775, disable SLI_INT_SUM[RML_TO] before reading the stats
	   enable back after clearing the interrupt. */
	cvmx_error_intsn_disable_v3(node, 0x1f000);
#endif
	stat0.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT0(index));
	READCORRECT(cnt, node, stat0.u64, CVMX_PKI_STATX_STAT0(index));

	stat1.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT1(index));
	READCORRECT(cnt, node, stat1.u64, CVMX_PKI_STATX_STAT1(index));

	stat2.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT2(index));
	READCORRECT(cnt, node, stat2.u64, CVMX_PKI_STATX_STAT2(index));

	stat3.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT3(index));
	READCORRECT(cnt, node, stat3.u64, CVMX_PKI_STATX_STAT3(index));

	stat4.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT4(index));
	READCORRECT(cnt, node, stat4.u64, CVMX_PKI_STATX_STAT4(index));

	stat5.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT5(index));
	READCORRECT(cnt, node, stat5.u64, CVMX_PKI_STATX_STAT5(index));

	stat6.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT6(index));
	READCORRECT(cnt, node, stat6.u64, CVMX_PKI_STATX_STAT6(index));

	stat7.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT7(index));
	READCORRECT(cnt, node, stat7.u64, CVMX_PKI_STATX_STAT7(index));

	stat8.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT8(index));
	READCORRECT(cnt, node, stat8.u64, CVMX_PKI_STATX_STAT8(index));

	stat9.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT9(index));
	READCORRECT(cnt, node, stat9.u64, CVMX_PKI_STATX_STAT9(index));

	stat10.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT10(index));
	READCORRECT(cnt, node, stat10.u64, CVMX_PKI_STATX_STAT10(index));

	stat11.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT11(index));
	READCORRECT(cnt, node, stat11.u64, CVMX_PKI_STATX_STAT11(index));

	stat14.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT14(index));
	READCORRECT(cnt, node, stat14.u64, CVMX_PKI_STATX_STAT14(index));

	stat15.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT15(index));
	READCORRECT(cnt, node, stat15.u64, CVMX_PKI_STATX_STAT15(index));

	stat16.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT16(index));
	READCORRECT(cnt, node, stat16.u64, CVMX_PKI_STATX_STAT16(index));

	stat17.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT17(index));
	READCORRECT(cnt, node, stat17.u64, CVMX_PKI_STATX_STAT17(index));

	hist0.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST0(index));
	READCORRECT(cnt, node, hist0.u64, CVMX_PKI_STATX_HIST0(index));

	hist1.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST1(index));
	READCORRECT(cnt, node, hist1.u64, CVMX_PKI_STATX_HIST1(index));

	hist2.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST2(index));
	READCORRECT(cnt, node, hist2.u64, CVMX_PKI_STATX_HIST2(index));

	hist3.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST3(index));
	READCORRECT(cnt, node, hist3.u64, CVMX_PKI_STATX_HIST3(index));

	hist4.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST4(index));
	READCORRECT(cnt, node, hist4.u64, CVMX_PKI_STATX_HIST4(index));

	hist5.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST5(index));
	READCORRECT(cnt, node, hist5.u64, CVMX_PKI_STATX_HIST5(index));

	hist6.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST6(index));
	READCORRECT(cnt, node, hist6.u64, CVMX_PKI_STATX_HIST6(index));

	inb_stat0.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKNDX_INB_STAT0(index));
	inb_stat1.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKNDX_INB_STAT1(index));
	inb_stat2.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKNDX_INB_STAT2(index));

	status->dropped_octets = stat4.s.drp_octs;
	status->dropped_packets = stat3.s.drp_pkts;
	status->octets = stat1.s.octs;
	status->pci_raw_packets = stat2.s.raw;
	status->packets = stat0.s.pkts;
	status->multicast_packets = stat6.s.mcast;
	status->broadcast_packets = stat5.s.bcast;
	status->len_64_packets = hist0.s.h1to63;
	status->len_65_127_packets = hist1.s.h64to127;
	status->len_128_255_packets = hist2.s.h128to255;
	status->len_256_511_packets = hist3.s.h256to511;
	status->len_512_1023_packets = hist4.s.h512to1023;
	status->len_1024_1518_packets = hist5.s.h1024to1518;
	status->len_1519_max_packets = hist6.s.h1519;
	status->fcs_align_err_packets = stat7.s.fcs;
	status->runt_packets = stat9.s.undersz;
	status->runt_crc_packets = stat8.s.frag;
	status->oversize_packets = stat11.s.oversz;
	status->oversize_crc_packets = stat10.s.jabber;
	status->mcast_l2_red_packets = stat15.s.drp_mcast;
	status->bcast_l2_red_packets = stat14.s.drp_bcast;
	status->mcast_l3_red_packets = stat17.s.drp_mcast;
	status->bcast_l3_red_packets = stat16.s.drp_bcast;
	status->inb_packets = inb_stat0.s.pkts;
	status->inb_octets = inb_stat1.s.octs;
	status->inb_errors = inb_stat2.s.errs;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* Enable SLI_INT_SUM[RML_TO] interrupt after clear the pending interrupt. */
	cvmx_write_csr_node(node, CVMX_CIU3_ISCX_W1C(0x1f000), 1);
	cvmx_error_intsn_enable_v3(node, 0x1f000);
#endif
}
EXPORT_SYMBOL(cvmx_pki_get_stats);

/**
 * Clear the statistics counters for a port.
 *
 * @param node  Node number
 * @param port  Port number (ipd_port) to get statistics for.
 *    Make sure PKI_STATS_CTL:mode is set to 0 for collecting per port/pkind stats.
 */
void cvmx_pki_clear_port_stats(int node, uint64_t port)
{
	int xipd = cvmx_helper_node_to_ipd_port(node, port);
	int xiface = cvmx_helper_get_interface_num(xipd);
	int index = cvmx_helper_get_interface_index_num(port);
	int pknd = cvmx_helper_get_pknd(xiface, index);

	cvmx_pki_statx_stat0_t stat0;
	cvmx_pki_statx_stat1_t stat1;
	cvmx_pki_statx_stat2_t stat2;
	cvmx_pki_statx_stat3_t stat3;
	cvmx_pki_statx_stat4_t stat4;
	cvmx_pki_statx_stat5_t stat5;
	cvmx_pki_statx_stat6_t stat6;
	cvmx_pki_statx_stat7_t stat7;
	cvmx_pki_statx_stat8_t stat8;
	cvmx_pki_statx_stat9_t stat9;
	cvmx_pki_statx_stat10_t stat10;
	cvmx_pki_statx_stat11_t stat11;
	cvmx_pki_statx_stat14_t stat14;
	cvmx_pki_statx_stat15_t stat15;
	cvmx_pki_statx_stat16_t stat16;
	cvmx_pki_statx_stat17_t stat17;
	cvmx_pki_statx_hist0_t hist0;
	cvmx_pki_statx_hist1_t hist1;
	cvmx_pki_statx_hist2_t hist2;
	cvmx_pki_statx_hist3_t hist3;
	cvmx_pki_statx_hist4_t hist4;
	cvmx_pki_statx_hist5_t hist5;
	cvmx_pki_statx_hist6_t hist6;
	cvmx_pki_pkndx_inb_stat0_t inb_stat0;
	cvmx_pki_pkndx_inb_stat1_t inb_stat1;
	cvmx_pki_pkndx_inb_stat2_t inb_stat2;

	stat0.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT0(pknd));
	stat1.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT1(pknd));
	stat2.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT2(pknd));
	stat3.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT3(pknd));
	stat4.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT4(pknd));
	stat5.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT5(pknd));
	stat6.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT6(pknd));
	stat7.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT7(pknd));
	stat8.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT8(pknd));
	stat9.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT9(pknd));
	stat10.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT10(pknd));
	stat11.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT11(pknd));
	stat14.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT14(pknd));
	stat15.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT15(pknd));
	stat16.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT16(pknd));
	stat17.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_STAT17(pknd));
	hist0.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST0(pknd));
	hist1.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST1(pknd));
	hist2.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST2(pknd));
	hist3.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST3(pknd));
	hist4.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST4(pknd));
	hist5.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST5(pknd));
	hist6.u64 = cvmx_read_csr_node(node, CVMX_PKI_STATX_HIST6(pknd));
	inb_stat0.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKNDX_INB_STAT0(pknd));
	inb_stat1.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKNDX_INB_STAT1(pknd));
	inb_stat2.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKNDX_INB_STAT2(pknd));

	stat4.s.drp_octs = 0;
	stat3.s.drp_pkts = 0;
	stat1.s.octs = 0;
	stat2.s.raw = 0;
	stat0.s.pkts = 0;
	stat6.s.mcast = 0;
	stat5.s.bcast = 0;
	hist0.s.h1to63 = 0;
	hist1.s.h64to127 = 0;
	hist2.s.h128to255 = 0;
	hist3.s.h256to511 = 0;
	hist4.s.h512to1023 = 0;
	hist5.s.h1024to1518 = 0;
	hist6.s.h1519 = 0;
	stat7.s.fcs = 0;
	stat9.s.undersz = 0;
	stat8.s.frag = 0;
	stat11.s.oversz = 0;
	stat10.s.jabber = 0;
	stat15.s.drp_mcast = 0;
	stat14.s.drp_bcast = 0;
	stat17.s.drp_mcast = 0;
	stat16.s.drp_bcast = 0;
	inb_stat0.s.pkts = 0;
	inb_stat1.s.octs = 0;
	inb_stat2.s.errs = 0;

	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT0(pknd), stat0.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT1(pknd), stat1.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT2(pknd), stat2.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT3(pknd), stat3.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT4(pknd), stat4.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT5(pknd), stat5.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT6(pknd), stat6.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT7(pknd), stat7.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT8(pknd), stat8.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT9(pknd), stat9.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT10(pknd), stat10.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT11(pknd), stat11.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT14(pknd), stat14.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT15(pknd), stat15.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT16(pknd), stat16.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT17(pknd), stat17.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST0(pknd), hist0.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST1(pknd), hist1.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST2(pknd), hist2.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST3(pknd), hist3.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST4(pknd), hist4.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST5(pknd), hist5.u64);
	cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST6(pknd), hist6.u64);
	cvmx_write_csr_node(node, CVMX_PKI_PKNDX_INB_STAT0(pknd), inb_stat0.u64);
	cvmx_write_csr_node(node, CVMX_PKI_PKNDX_INB_STAT1(pknd), inb_stat1.u64);
	cvmx_write_csr_node(node, CVMX_PKI_PKNDX_INB_STAT2(pknd), inb_stat2.u64);
}
EXPORT_SYMBOL(cvmx_pki_clear_port_stats);

#ifdef CVMX_DUMP_PKI

#define PKI_PRN_HEADLEN 28
#define PKI_PRN_DATALEN 52
#define PKI_PRN_LINELEN (PKI_PRN_HEADLEN + PKI_PRN_DATALEN)
#define PKI_PRN_DPLEN(__n) (PKI_PRN_DATALEN / __n)

#ifndef MAX
#define MAX(__a, __b) (((__a) > (__b)) ? (__a) : (__b))
#endif

static int pki_find_pkind_chans(int node, int pkind, char *buf, int blen);

static int pki_get_num_bgxs(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 6;
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return 3;
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 1;
	return 0;
}

static void pki_flprint(int level, char *name, const char *format, ...)
{
	char dbuf[PKI_PRN_DATALEN + 1];
	int offs;
	va_list args;
	va_start(args, format);
	offs = level * 2;
	vsnprintf(dbuf, PKI_PRN_DATALEN + 1, format, args);
	cvmx_printf("%*s%-*s%*s\n",
		offs, "", PKI_PRN_HEADLEN - offs, name, PKI_PRN_DATALEN, dbuf);
	va_end(args);
}
/*
 * Show PKI integrated configuration.
 * See function prototype in cvmx-pki.h
 */
int cvmx_pki_config_dump(unsigned node)
{
#define DLMPRINT(__format, ...) \
	do { \
		int __n; \
		sprintf(lines[1], __format, ## __VA_ARGS__); \
		__n = PKI_PRN_LINELEN - strlen(lines[1]) - 1; \
		memset(lines[0], '-', __n);  lines[0][__n] = '\0'; \
		cvmx_printf("%s %s\n", lines[0], lines[1]); \
	} while (0)

#define NMPRINT(__num, __mask, __index, __level, __header, __format, __arg) \
	do { \
		int __step = PKI_PRN_DATALEN / __num; \
		int __offs = __level * 2; \
		cvmx_printf("%*s%-*s", __offs, "", PKI_PRN_HEADLEN - __offs, __header); \
		for (__index = 0; __index < __num; __index++) { \
			if ((1 << __index) & __mask) \
				cvmx_printf(__format, __step, __arg); \
			else \
				cvmx_printf("%*s", __step, "--"); \
		} \
		cvmx_printf("\n"); \
	} while (0)

#define NSPRINT(__num, __index, __action) \
	do { \
		for (__index = 0; __index < __num; __index++) \
			(__action); \
	} while (0)

#define NMCMPEQ(__cmp, __data, __num, __mask, __index) ({ \
	int __rc = 0; \
	uint64_t __prev = __cmp; \
	do { \
		for (__index = 0; __index < __num; __index++) { \
			if ((1 << __index) & __mask) \
				if (__data != __prev) __rc += 1; \
		} \
	} while (0); \
	__rc; \
})

	void int2cstr(char *buf, int data, int nbits) {
		char *lbits[8] = {"G","F", "E", "D", "C", "B", "?", "?"};
		buf[0] = '\0';
		for (nbits--; nbits >= 0; nbits--, data >>= 1)
			strcat(buf, (data & 1) ? lbits[nbits & 0x7] : "-");
	}
	char *qpgqos_map[8] = {
		[0x0] = "NONE",
		[0x1] = "VLAN",
		[0x2] = "MPLS",
		[0x3] = "DSA_SRC",
		[0x4] = "DIFF",
		[0x5] = "HIGIG",
		[0x6] = "Undef",
		[0x7] = "Undef"
	};
	char *tagtype_map[4] = {
		[0x0] = "Ordered",
		[0x1] = "Atomic",
		[0x2] = "Untagged",
		[0x3] = "Empty"
	};
	char *mtagptr_map[16] = {
		[0]  = "SOP",
		[1]  = "Undef",
		[2]  = "Undef",
		[3]  = "Undef",
		[4]  = "Undef",
		[5]  = "Undef",
		[6]  = "Undef",
		[7]  = "Undef",
		[8]  = "LA",
		[9]  = "LB",
		[10] = "LC",
		[11] = "LD",
		[12] = "LE",
		[13] = "LF",
		[14] = "LG",
		[15] = "VL"
	};
	int nclusters = CVMX_PKI_NUM_CLUSTER;
	cvmx_pki_tag_secret_t secret;
	cvmx_pki_buf_ctl_t ctl;
	cvmx_pki_gbl_pen_t pen;
	cvmx_pki_icgx_cfg_t cgcfg;
	cvmx_pki_sft_rst_t rst;
	int mask, ibase, n, k, i, __i;
	int pkind, cluster, style, group;
	uint32_t crc32, pcrc32;
	char chans[128];
	char lines[4][128];

	/* Show Global Configuration. */
	ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);
	pen.u64 = cvmx_read_csr_node(node, CVMX_PKI_GBL_PEN);
	secret.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_SECRET);
	memset(lines[0], '*', PKI_PRN_LINELEN);  lines[0][PKI_PRN_LINELEN] = '\0';
	cvmx_printf("\n%s\n", lines[0]);
	cvmx_printf("   PKI Configuration (Node %d)\n", node);
	cvmx_printf("%s\n", lines[0]);
	pki_flprint(0, "PKI Enabled/Active", "%d/%d", ctl.s.pki_en, rst.s.active);
	pki_flprint(0, "Packet buffering", "%*s",
		PKI_PRN_DATALEN, ctl.s.pkt_off ? "Disabled" : "Enabled");
	pki_flprint(0, "FPA buffer policy", "%*s",
		PKI_PRN_DATALEN, ctl.s.fpa_wait ? "Wait" : "Drop");
	pki_flprint(0, "BPID backpressure", "%*s",
		PKI_PRN_DATALEN, ctl.s.pbp_en ? "Enabled" : "Disabled");
	pki_flprint(0, "", "%*s%*s%*s%*s", PKI_PRN_DPLEN(4), "DST6",
		PKI_PRN_DPLEN(4), "SRC6", PKI_PRN_DPLEN(4), "DST", PKI_PRN_DPLEN(4), "SRC");
	pki_flprint(0, "Tag secret words (hex)", "%*x%*x%*x%*x",
		PKI_PRN_DPLEN(4), secret.s.dst6, PKI_PRN_DPLEN(4), secret.s.src6,
		PKI_PRN_DPLEN(4), secret.s.dst, PKI_PRN_DPLEN(4), secret.s.src);
	cvmx_printf("%-30s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s\n", "",
		"VIRT", "CLG", "CL2", "L4", "IL3", "L3", "MPLS", "FULC", "DSA", "HG");
	cvmx_printf("%-30s %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d\n", "Parsing enabled",
		pen.s.virt_pen, pen.s.clg_pen, pen.s.cl2_pen, pen.s.l4_pen, pen.s.il3_pen,
		pen.s.l3_pen, pen.s.mpls_pen, pen.s.fulc_pen, pen.s.dsa_pen, pen.s.hg_pen);

	/* Show PKINDs.*/
	for (pkind = 0, pcrc32 = 0, ibase = 0; pkind < CVMX_PKI_NUM_PKIND; pkind++) {
		cvmx_pki_pkindx_icgsel_t cgsel;
		cvmx_pki_clx_pkindx_style_t pkstyle[CVMX_PKI_NUM_CLUSTER];
		cvmx_pki_clx_pkindx_cfg_t pkcfg[CVMX_PKI_NUM_CLUSTER];
		cvmx_pki_clx_pkindx_skip_t pkskip[CVMX_PKI_NUM_CLUSTER];
		cvmx_pki_clx_pkindx_l2_custom_t pkl2cust[CVMX_PKI_NUM_CLUSTER];
		cvmx_pki_clx_pkindx_lg_custom_t pklgcust[CVMX_PKI_NUM_CLUSTER];

		cgsel.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind));
		cgcfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(cgsel.s.icg));
		mask = cgcfg.s.clusters;
		if (mask == 0)
			continue;

		CVMX_MT_CRC_POLYNOMIAL(0x1edc6f41);
		CVMX_MT_CRC_IV(0xffffffff);
		CVMX_MT_CRC_DWORD(cgsel.u64 & 0x3ull);
		for (cluster = 0; cluster < nclusters; cluster++) {
			if (((1 << cluster) & mask) == 0)
				continue;
			pkstyle[cluster].u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
			pkcfg[cluster].u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster));
			pkskip[cluster].u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_PKINDX_SKIP(pkind, cluster));
			pkl2cust[cluster].u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_PKINDX_L2_CUSTOM(pkind, cluster));
			pklgcust[cluster].u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_PKINDX_LG_CUSTOM(pkind, cluster));
			CVMX_MT_CRC_DWORD(pkstyle[cluster].u64 & ((1ull << 16) - 1));
			CVMX_MT_CRC_DWORD(pkcfg[cluster].u64 & ((1ull << 16) - 1));
			CVMX_MT_CRC_DWORD(pkskip[cluster].u64 & ((1ull << 16) - 1));
			CVMX_MT_CRC_DWORD(pkl2cust[cluster].u64 & ((1ull << 16) - 1));
			CVMX_MT_CRC_DWORD(pklgcust[cluster].u64 & ((1ull << 8) - 1));
		}
		if (pki_find_pkind_chans(node, pkind, chans, PKI_PRN_DATALEN))
			CVMX_MT_CRC_DWORD(pkind);
		CVMX_MF_CRC_IV(crc32);
		if (crc32 == pcrc32)
			continue;
		if (pkind > 0 && (pkind - 1) != ibase)
			cvmx_printf("\nPKIND(s) %02d-%02d -- same as PKIND %02d\n",
				pkind - 1, ibase + 1, ibase);
		pcrc32 = crc32;
		ibase = pkind;

		DLMPRINT("PKIND %02d:", pkind);
		pki_flprint(0, "Channel", "%s", (chans[0] != 0) ? chans : "--");
		NSPRINT(nclusters, __i, sprintf(lines[__i], "Cluster%d", __i));
		NMPRINT(nclusters, ((1 << nclusters) - 1), __i, 0, "", "%*s", lines[__i]);
		cvmx_printf("Mapping:\n");
		pki_flprint(1, "Cluster Group", "%*d", PKI_PRN_DATALEN, cgsel.s.icg);
		cvmx_printf("Parsing:\n");
		NMPRINT(nclusters, mask, __i, 1, "Initial Style", "%*d", pkstyle[__i].s.style);
		NSPRINT(nclusters, __i, sprintf(lines[__i], "%c%c%c%c%c%c%c",
			(pkstyle[__i].s.pm & (1 << 0)) ? '-' : 'A',
			(pkstyle[__i].s.pm & (1 << 1)) ? '-' : 'B',
			(pkstyle[__i].s.pm & (1 << 2)) ? '-' : 'C',
			(pkstyle[__i].s.pm & (1 << 3)) ? '-' : 'D',
			(pkstyle[__i].s.pm & (1 << 4)) ? '-' : 'E',
			(pkstyle[__i].s.pm & (1 << 5)) ? '-' : 'F',
			(pkstyle[__i].s.pm & (1 << 6)) ? '-' : 'G'));
		NMPRINT(nclusters, mask, __i, 1, "Initial Parse Mode", "%*s", lines[__i]);
		NMPRINT(nclusters, mask, __i, 1, "INST skip (bytes)", "%*d", pkskip[__i].s.inst_skip);
		if (NMCMPEQ(0, pkcfg[__i].s.inst_hdr, nclusters, mask, __i) != 0)
			NMPRINT(nclusters, mask, __i, 1, "INST Header present", "%*s",
				pkcfg[__i].s.inst_hdr ? "Yes" : "No");
		NMPRINT(nclusters, mask, __i, 1, "FCS skip (bytes)", "%*d", pkskip[__i].s.fcs_skip);
		NMPRINT(nclusters, mask, __i, 1, "FCS present", "%*s",
			pkcfg[__i].s.fcs_pres ? "Yes" : "No");
		if (NMCMPEQ(0, pkl2cust[__i].s.valid, nclusters, mask, __i) != 0) {
			NMPRINT(nclusters, mask, __i, 1, "L2 custom match", "%*s",
				pkl2cust[__i].s.valid ? "On" : "Off");
			NMPRINT(nclusters, mask, __i, 1, "L2 Custom offset", "%*d", pkl2cust[__i].s.offset);
		}
		if (NMCMPEQ(0, pkcfg[__i].s.lg_custom, nclusters, mask, __i) != 0) {
			NMPRINT(nclusters, mask, __i, 1, "LG custom match", "%*s",
				pkcfg[__i].s.lg_custom ? "On" : "Off");
			NMPRINT(nclusters, mask, __i, 1, "LG Custom offset", "%*d", pklgcust[__i].s.offset);
		}
		if (NMCMPEQ(0, pkcfg[__i].s.mpls_en, nclusters, mask, __i) != 0)
			NMPRINT(nclusters, mask, __i, 1, "MPLS parsing", "%*s",
				pkcfg[__i].s.mpls_en ? "On" : "Off");
		if (NMCMPEQ(0, pkcfg[__i].s.dsa_en, nclusters, mask, __i) != 0)
			NMPRINT(nclusters, mask, __i, 1, "DSA parsing", "%*s",
				pkcfg[__i].s.dsa_en ? "On" : "Off");
		if (NMCMPEQ(0, pkcfg[__i].s.hg_en, nclusters, mask, __i) != 0)
			NMPRINT(nclusters, mask, __i, 1, "HG parsing", "%*s",
				pkcfg[__i].s.hg_en ? "On" : "Off");
		if (NMCMPEQ(0, pkcfg[__i].s.hg2_en, nclusters, mask, __i) != 0)
			NMPRINT(nclusters, mask, __i, 1, "HG2 parsing", "%*s",
				pkcfg[__i].s.hg2_en ? "On" : "Off");
		if (NMCMPEQ(0, pkcfg[__i].s.fulc_en, nclusters, mask, __i) != 0)
			NMPRINT(nclusters, mask, __i, 1, "Fulcrum Header parsing", "%*s",
				pkcfg[__i].s.fulc_en ? "On" : "Off");
	}
	if ((pkind - 1) != ibase)
		cvmx_printf("\nPKIND(s) %02d-%02d -- same as PKIND %02d\n", pkind - 1, ibase + 1, ibase);

	/* Show Styles.*/
	for (style = 0, pcrc32 = 0, ibase = 0; style < CVMX_PKI_NUM_FINAL_STYLE; style++) {
		cvmx_pki_stylex_buf_t stbuf;
		cvmx_pki_stylex_tag_sel_t tagsel;
		cvmx_pki_stylex_tag_mask_t tagmask;
		cvmx_pki_clx_stylex_cfg_t stcfg[CVMX_PKI_NUM_CLUSTER];
		cvmx_pki_clx_stylex_cfg2_t stcfg2[CVMX_PKI_NUM_CLUSTER];
		cvmx_pki_clx_stylex_alg_t stalg[CVMX_PKI_NUM_CLUSTER];
		cvmx_pki_qpg_tblx_t qpgtx[CVMX_PKI_NUM_CLUSTER];

		CVMX_MT_CRC_POLYNOMIAL(0x1edc6f41);
		CVMX_MT_CRC_IV(0xffffffff);
		stbuf.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(style));
		CVMX_MT_CRC_DWORD(stbuf.u64 & ((1ull << 33) - 1));
		tagsel.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_TAG_SEL(style));
		CVMX_MT_CRC_DWORD(tagsel.u64 & (0x7ull << 24 | 0x7ull << 16 | 0x7ull << 8 | 0x7ull));
		tagmask.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_TAG_MASK(style));
		CVMX_MT_CRC_DWORD(tagmask.u64 & ((1ull << 16) - 1));
		mask = 0;
		for (cluster = 0; cluster < nclusters; cluster++) {
			stcfg[cluster].u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
			stcfg2[cluster].u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
			stalg[cluster].u64 = cvmx_read_csr_node(node,
				CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
			qpgtx[cluster].u64 = cvmx_read_csr_node(node,
				CVMX_PKI_QPG_TBLX(stcfg[cluster].s.qpg_base));
			CVMX_MT_CRC_DWORD(stcfg[cluster].u64 & 0x7FFF07FF);
			CVMX_MT_CRC_DWORD(stcfg2[cluster].u64 & ((1ull << 32) - 1));
			CVMX_MT_CRC_DWORD(stalg[cluster].u64 & ((1ull << 32) - 1));
			mask |= 1 << cluster;
		}
		CVMX_MF_CRC_IV(crc32);
		if (crc32 == pcrc32)
			continue;
		if (style > 0 && (style - 1) != ibase)
			cvmx_printf("\nSTYLE(s) %02d-%02d -- same as STYLE %02d\n",
				style - 1, ibase + 1, ibase);
		pcrc32 = crc32;
		ibase = style;

		DLMPRINT("STYLE %02d:", style);
		NSPRINT(nclusters, __i, sprintf(lines[__i], "Cluster%d", __i));
		NMPRINT(nclusters, ((1 << nclusters) - 1), __i, 0, "", "%*s", lines[__i]);
		cvmx_printf("Buffering:\n");
		stbuf.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(style));
		pki_flprint(1, "WQE header", "%s", stbuf.s.wqe_hsz ? "WORD0..5" : "WORD0..4");
		pki_flprint(1, "WQE and Data buffers", "%s", stbuf.s.dis_wq_dat ? "Separate" : "Same");
		pki_flprint(1, "WQE Skip (bytes)", "%d", stbuf.s.wqe_skip * 128);
		pki_flprint(1, "First Skip (bytes)", "%d", stbuf.s.first_skip * 8);
		pki_flprint(1, "Later Skip (bytes)", "%d", stbuf.s.later_skip * 8);
		pki_flprint(1, "MB Size (bytes)", "%d", stbuf.s.mb_size * 8);
		pki_flprint(1, "OPC Mode", "%d", stbuf.s.opc_mode);
		NMPRINT(nclusters, mask, __i, 0, "Strip FCS", "%*s", stcfg[__i].s.fcs_strip ? "Yes":"No");
		NMPRINT(nclusters, mask, __i, 0, "Drop", "%*d", stcfg[__i].s.drop);
		NMPRINT(nclusters, mask, __i, 0, "No Drop", "%*d", stcfg[__i].s.nodrop);
		NMPRINT(nclusters, mask, __i, 0, "Raw Drop", "%*d", stcfg[__i].s.rawdrp);
		cvmx_printf("Tag:\n");
		NMPRINT(nclusters, mask, __i, 1, "Tag Type", "%*s", tagtype_map[stalg[__i].s.tt]);
		if (NMCMPEQ(0, ((stcfg2[__i].u64 >> 18) & 0x3F), 4, mask, __i) != 0) {
			NSPRINT(nclusters, __i, int2cstr(lines[__i], (stcfg2[__i].u64 >> 18) & 0x3F, 6));
			NMPRINT(nclusters, mask, __i, 1, "T-Tag <= Source", "%*s", lines[__i]);
		}
		if (NMCMPEQ(0, ((stcfg2[__i].u64 >> 12) & 0x3F), 4, mask, __i) != 0) {
			NSPRINT(nclusters, __i, int2cstr(lines[__i], (stcfg2[__i].u64 >> 12) & 0x3F, 6));
			NMPRINT(nclusters, mask, __i, 1, "T-Tag <= Dest", "%*s", lines[__i]);
		}
		if (NMCMPEQ(0, stalg[__i].s.tag_pctl, 4, mask, __i) != 0)
			NMPRINT(nclusters, mask, __i, 1, "T-Tag <= Proto", "%*s",
				stalg[__i].s.tag_pctl ? "On":"Off");
		if (NMCMPEQ(0, stalg[__i].s.tag_vs0, 4, mask, __i) != 0)
			NMPRINT(nclusters, mask, __i, 1, "T-Tag <= VLAN0", "%*s",
				stalg[__i].s.tag_vs0 ? "On":"Off");
		if (NMCMPEQ(0, stalg[__i].s.tag_vs1, 4, mask, __i) != 0)
			NMPRINT(nclusters, mask, __i, 1, "T-Tag <= VLAN1", "%*s",
				stalg[__i].s.tag_vs1 ? "On":"Off");
		if (NMCMPEQ(0, stalg[__i].s.tag_prt, 4, mask, __i) != 0)
			NMPRINT(nclusters, mask, __i, 1, "T-Tag <= Port", "%*s",
				stalg[__i].s.tag_prt ? "On":"Off");
		if (NMCMPEQ(0, stcfg2[__i].s.tag_inc, 4, mask, __i) != 0) {
			cvmx_pki_tag_incx_ctl_t tagctl;
			cvmx_pki_tag_incx_mask_t incmask;
			int tagx, clx = __builtin_ffsll(mask) - 1;
			cvmx_printf("%*s%s\n", 2, "", "M-Tag = Base:Offset:Mask (name:dec:hex)");
			for (i = 0; i < 4; i++) {
				if ((stcfg2[clx].s.tag_inc & (1 << i)) == 0)
					continue;
				switch (i) {
				case 0: tagx = tagsel.s.tag_idx0 * 4 + 0; break;
				case 1: tagx = tagsel.s.tag_idx1 * 4 + 1; break;
				case 2: tagx = tagsel.s.tag_idx2 * 4 + 2; break;
				case 3: tagx = tagsel.s.tag_idx3 * 4 + 3; break;
				}
				tagctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_INCX_CTL(tagx));
				incmask.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_INCX_MASK(tagx));
				sprintf(lines[0], "M-Tag%d", i);
				pki_flprint(2, lines[0], "%s:%d:%016llx",
					mtagptr_map[tagctl.s.ptr_sel], tagctl.s.offset, incmask.s.en);
			}
		}
		if (NMCMPEQ(0, stcfg2[__i].s.tag_masken, 4, mask, __i) != 0) {
			pki_flprint(1, "Tag Mask (hex)", "%*x", PKI_PRN_DATALEN, tagmask.s.mask);
		}
		cvmx_printf("QPG:\n");
		NMPRINT(nclusters, mask, __i, 1, "QOS Algo", "%*s", qpgqos_map[stalg[__i].s.qpg_qos]);
		NMPRINT(nclusters, mask, __i, 1, "QPG Base (dec)", "%*d", stcfg[__i].s.qpg_base);
		if (NMCMPEQ(0, stalg[__i].s.qpg_port_msb, 4, mask, __i) != 0) {
			NMPRINT(nclusters, mask, __i, 1, "Port MSB (hex)", "%*x", stalg[__i].s.qpg_port_msb);
			NMPRINT(nclusters, mask, __i, 1, "Port Shift (dec)", "%*d", stalg[__i].s.qpg_port_sh);
		}
		NSPRINT(nclusters, __i, stcfg[__i].s.qpg_dis_padd ? \
				sprintf(lines[__i], "Dis") : sprintf(lines[__i], "%d",qpgtx[__i].s.padd));
		NMPRINT(nclusters, mask, __i, 1, "QPG PortAdder", "%*s", lines[__i]);
		NSPRINT(nclusters, __i, stcfg[__i].s.qpg_dis_aura ? \
				sprintf(lines[__i], "Dis") : sprintf(lines[__i], "%d",qpgtx[__i].s.laura));
		NMPRINT(nclusters, mask, __i, 1, "QPG Aura", "%*s", lines[__i]);
		NSPRINT(nclusters, __i, stcfg[__i].s.qpg_dis_grp ? \
				sprintf(lines[__i], "Dis") : sprintf(lines[__i], "%d",qpgtx[__i].s.grp_ok));
		NMPRINT(nclusters, mask, __i, 1, "QPG Group OK", "%*s", lines[__i]);
		NSPRINT(nclusters, __i, stcfg[__i].s.qpg_dis_grp ? \
				sprintf(lines[__i], "Dis") : sprintf(lines[__i], "%d",qpgtx[__i].s.grp_bad));
		NMPRINT(nclusters, mask, __i, 1, "QPG Group Bad", "%*s", lines[__i]);
		NSPRINT(nclusters, __i, stcfg[__i].s.qpg_dis_grptag ? \
				sprintf(lines[__i], "Dis") : sprintf(lines[__i], "%d",qpgtx[__i].s.grptag_ok));
		NMPRINT(nclusters, mask, __i, 1, "QPG GrpTag OK", "%*s", lines[__i]);
		NSPRINT(nclusters, __i, stcfg[__i].s.qpg_dis_grptag ? \
				sprintf(lines[__i], "Dis") : sprintf(lines[__i], "%d",qpgtx[__i].s.grptag_bad));
		NMPRINT(nclusters, mask, __i, 1, "QPG GrpTag Bad", "%*s", lines[__i]);
	}
	if (style > 0 && (style - 1) != ibase)
		cvmx_printf("\nSTYLE(s) %02d-%02d -- same as STYLE %02d\n", style - 1, ibase + 1, ibase);

	/* Show Cluster Groups. */
	for (group = 0; group < (int)CVMX_PKI_NUM_CLUSTER_GROUP; group++) {
		cgcfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(group));
		mask = cgcfg.s.clusters;
		if (mask == 0)
			continue;

		DLMPRINT("CLUSTER GROUP %d (mask = 0x%x):", group, mask);
		cvmx_printf("Action = {PMC(hex) : +STYLE(dec) : PF(hex) : SETTY(dec) : ADVANCE(dec)}\n");
		pki_flprint(0, "Parsing", "%s", cgcfg.s.pena ? "Enabled" : "Disabled");
		pki_flprint(0, "Entry", "%*s%*s", PKI_PRN_DPLEN(2), "PCAM0", PKI_PRN_DPLEN(2), "PCAM1");
		for (cluster = 0; cluster < nclusters; cluster++) {
			cvmx_pki_clx_pcamx_termx_t term[CVMX_PKI_NUM_PCAM_BANK];
			cvmx_pki_clx_pcamx_matchx_t	match[CVMX_PKI_NUM_PCAM_BANK];
			cvmx_pki_clx_pcamx_actionx_t action[CVMX_PKI_NUM_PCAM_BANK];
			int pcam, entry;

			if (((1 << cluster) & mask) == 0)
				continue;

			cvmx_printf("---------------- Cluster %d\n", cluster);
			for (entry = 0; entry < CVMX_PKI_NUM_PCAM_ENTRY; entry++) {
				for (pcam = 0; pcam < CVMX_PKI_NUM_PCAM_BANK; pcam++) {
					term[pcam].u64 = cvmx_read_csr_node(node,
						CVMX_PKI_CLX_PCAMX_TERMX(cluster, pcam, entry));
					match[pcam].u64 = cvmx_read_csr_node(node,
						CVMX_PKI_CLX_PCAMX_MATCHX(cluster, pcam, entry));
					action[pcam].u64 = cvmx_read_csr_node(node,
						CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, pcam, entry));
				}
				if ((term[0].s.valid + term[1].s.valid) == 0)
					continue;
				sprintf(lines[0], "[%d]", entry);
				pki_flprint(0, lines[0], "");
				NSPRINT(2, __i, sprintf(lines[__i], "%x:%x", term[__i].s.term1,
					term[__i].s.term1 ^ term[__i].s.term0));
				NMPRINT(2, 0x3, __i, 1, "Term:Mask (hex)", "%*s", lines[__i]);
				NSPRINT(2, __i, sprintf(lines[__i], "%x:%x", term[__i].s.style1,
					term[__i].s.style1 ^ term[__i].s.style0));
				NMPRINT(2, 0x3, __i, 1, "Style:Mask (hex)", "%*s", lines[__i]);
				NSPRINT(2, __i, sprintf(lines[__i], "%x:%x", match[__i].s.data1,
					match[__i].s.data1 ^ match[__i].s.data0));
				NMPRINT(2, 0x3, __i, 1, "Data:Mask (hex)", "%*s", lines[__i]);
				NSPRINT(2, __i, sprintf(lines[__i], "%x:%d:%x:%d:%d", action[__i].s.pmc,
					action[__i].s.style_add, action[__i].s.pf,
					action[__i].s.setty, action[__i].s.advance));
				NMPRINT(2, 0x3, __i, 1, "Action", "%*s", lines[__i]);
			}
		}
	}
	DLMPRINT("QPG TABLE:");
	cvmx_printf("(Only entries with non-zero values are shown.)\n");
	n = (PKI_PRN_LINELEN) / 8;
	cvmx_printf("%-*s%*s%*s%*s%*s%*s%*s%*s\n", n, "QPG", n, "PADD", n, "GRP_OK",
		n, "GRP_BAD", n, "GTAG_OK", n, "GTAG_BAD", n, "NODE", n, "LAURA");
	for (i = 0; i < CVMX_PKI_NUM_QPG_ENTRY; i++) {
		cvmx_pki_qpg_tblx_t qpgt, qpgt2;
		qpgt2.u64 = qpgt.u64 = cvmx_read_csr_node(node, CVMX_PKI_QPG_TBLX(i));
		qpgt2.s.aura_node = 0;
		if (qpgt2.u64 != 0) {
			sprintf(lines[0], "%d", i);
			cvmx_printf("%-*s%*d%*d%*d%*d%*d%*d%*d\n", n, lines[0], n, qpgt.s.padd,
				n, qpgt.s.grp_ok, n, qpgt.s.grp_bad, n, qpgt.s.grptag_ok,
				n, qpgt.s.grptag_bad, n, qpgt.s.aura_node, n, qpgt.s.laura);
		}
	}
	DLMPRINT("AURA TABLE:");
	cvmx_printf("(Only entries with non-zero values are shown.)\n");
	n = (PKI_PRN_LINELEN) / 8;
	cvmx_printf("%-*s%*s%*s%*s%*s%*s%*s%*s\n", n, "Aura", n, "", n, "",
		n, "PKT_ADD", n, "ENA_RED", n, "ENA_DROP", n, "ENA_BP", n, "BPID");
	for (i = 0; i < CVMX_PKI_NUM_AURA; i++) {
		cvmx_pki_aurax_cfg_t acfg;
		acfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_AURAX_CFG(i));
		if (acfg.u64 != 0) {
			sprintf(lines[0], "%d", i);
			cvmx_printf("%-*s%*s%*s%*d%*d%*d%*d%*d\n", n, lines[0], n, "", n, "",
				n, acfg.s.pkt_add, n, acfg.s.ena_red, n, acfg.s.ena_drop,
				n, acfg.s.ena_bp, n, acfg.s.bpid);
		}
	}
	DLMPRINT("CHANNEL TABLE:");
	cvmx_printf("(Only entries with non-zero values are shown.)\n");
	n = (PKI_PRN_LINELEN) / 8;
	cvmx_printf("%-*s%*s%*s\n", n * 6, "Channel", n, "", n, "BPID");
	for (i = 0; i < CVMX_PKI_NUM_CHANNEL; i++) {
		cvmx_pki_chanx_cfg_t chan;
		int nbgxs = pki_get_num_bgxs();

		chan.u64 = cvmx_read_csr_node(node, CVMX_PKI_CHANX_CFG(i));
		if (chan.s.imp == 1 && chan.s.bpid != 0) {

			if (i >= 0 && i < 64) /* LBK*/
				sprintf(lines[1], "LBK:%d", i);
			else if (i >= 0x100 && i < (0x100 + 128)) { /* DPI*/
				k = i - 0x100;
				if (OCTEON_IS_MODEL(OCTEON_CN78XX) && i >= (0x100 + 64))
					continue;
				sprintf(lines[1], "DPI:%d", k);
			}
			else if (i >= 0x240 && i < (0x240 + 2 * 2)) { /* SRIO*/
				k = i - 0x240;
				sprintf(lines[1], "SRIO%d:%d", k / 2, k % 2);
			}
			else if (i >= 0x400 && i < (0x400 + 2 * 256)) { /* ILK */
				k = i - 0x400;
				sprintf(lines[1], "ILK%d:%d", k / 256, k % 256);
			}
			else if (i >= 0x800 && i < (0x800 + nbgxs * 4 * 16)) { /* BGX */
				k = i - 0x800;
				sprintf(lines[1], "BGX%d:Port%d:Chan%d", k / 0x100,
					(k % (4 * 16)) / 16, (k % (4 * 16)) % 16);
			}
			else
				continue;
			sprintf(lines[0], "%d(0x%x)", i, i);
			cvmx_printf("%-*s%-*s%*s%*d\n",
				n * 2, lines[0], n * 2, lines[1], n * 3, "", n, chan.s.bpid);
		}
	}
	return 0;
}

/*
 * Show PKI integrated statistics.
 * See function prototype in cvmx-pki.h
 */
int cvmx_pki_stats_dump(unsigned node)
{
	int pkind;
	int64_t count;
	cvmx_pki_stat_ctl_t ctl;
	struct cvmx_pki_port_stats stats;
	char chans[128];
	char line[256];

	memset(line, '*', PKI_PRN_LINELEN); line[PKI_PRN_LINELEN] = '\0';
	cvmx_printf("\n%s\n", line);
	cvmx_printf("   PKI Statistics (Node %d)\n", node);
	cvmx_printf("%s\n", line);
	ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_STAT_CTL);

	for (pkind = 0; pkind < CVMX_PKI_NUM_PKIND; pkind++) {
		cvmx_pki_get_stats(node, pkind, &stats);

		if ((stats.inb_octets + stats.inb_errors) > 0) {
			pki_find_pkind_chans(node, pkind, chans, PKI_PRN_DATALEN);
			cvmx_printf("PKIND %d  <= %s\n", pkind, (chans[0] != 0) ? chans : "??");
			if (stats.inb_octets) {
				pki_flprint(2, "Total good packets:", "%lld", stats.inb_packets);
				pki_flprint(2, "Total good bytes:", "%lld", stats.inb_octets);
			}
			if (stats.inb_errors)
				pki_flprint(2, "Total err.packets:", "%lld", stats.inb_errors);
		}
		count = stats.packets;
		count += stats.len_64_packets;
		count += stats.len_65_127_packets;
		count += stats.len_128_255_packets;
		count += stats.len_256_511_packets;
		count += stats.len_512_1023_packets;
		count += stats.len_1024_1518_packets;
		count += stats.len_1519_max_packets;
		count += stats.pci_raw_packets;
		count += stats.dropped_packets;
		count += stats.fcs_align_err_packets;
		count += stats.runt_crc_packets;
		count += stats.runt_packets;
		count += stats.oversize_crc_packets;
		count += stats.oversize_packets;

		if (count > 0) {
			if (ctl.s.mode == 0x1)
				cvmx_printf("STYLE %d\n", pkind);
			if (stats.packets > 0) {
				pki_flprint(2, "Non-dropped packets:", "%lld", stats.packets);
				pki_flprint(2, "Non-dropped bytes:", "%lld", stats.octets);
			}
			if (stats.len_64_packets > 0)
				pki_flprint(2, "64-byte packets:", "%lld", stats.len_64_packets);
			if (stats.len_65_127_packets > 0)
				pki_flprint(2, "65-128-byte packets:", "%lld", stats.len_65_127_packets);
			if (stats.len_128_255_packets > 0)
				pki_flprint(2, "128-255-byte packets:", "%lld", stats.len_128_255_packets);
			if (stats.len_256_511_packets > 0)
				pki_flprint(2, "256-511-byte packets:", "%lld", stats.len_256_511_packets);
			if (stats.len_512_1023_packets > 0)
				pki_flprint(2, "512-1023-byte packets:", "%lld", stats.len_512_1023_packets);
			if (stats.len_1024_1518_packets > 0)
				pki_flprint(2, "1024-1518-byte packets:", "%lld", stats.len_1024_1518_packets);
			if (stats.len_1519_max_packets > 0)
				pki_flprint(2, ">1519-byte packets:", "%lld", stats.len_1519_max_packets);
			if (stats.pci_raw_packets > 0)
				pki_flprint(2, "Raw packets:", "%lld", stats.pci_raw_packets);
			if (stats.dropped_packets > 0) {
				pki_flprint(2, "Dropped packets:", "%lld", stats.dropped_packets);
				pki_flprint(2, "Dropped bytes:", "%lld", stats.dropped_octets);
			}
			if (stats.fcs_align_err_packets > 0)
				pki_flprint(2, "FCS errors:", "%lld", stats.fcs_align_err_packets);
			if (stats.runt_crc_packets > 0)
				pki_flprint(2, "Runt&FCS errors:", "%lld", stats.runt_crc_packets);
			if (stats.runt_packets > 0)
				pki_flprint(2, "Runt errors:", "%lld", stats.runt_packets);
			if (stats.oversize_crc_packets > 0)
				pki_flprint(2, "Oversize&FCS errors:", "%lld", stats.oversize_crc_packets);
			if (stats.oversize_packets > 0)
				pki_flprint(2, "Oversize packets:", "%lld", stats.oversize_packets);
		}
	}
	return 0;
}

/*
 * Clear PKI statistics.
 * See function prototype in cvmx-pki.h
 */
void cvmx_pki_stats_clear(unsigned node)
{
	int i;

	for (i = 0; i < CVMX_PKI_NUM_PKIND; i++) {
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT0(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT1(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT2(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT3(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT4(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT5(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT6(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT7(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT8(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT9(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT10(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT11(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT12(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT13(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT14(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT15(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT16(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT17(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_STAT18(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST0(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST1(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST2(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST3(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST4(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST5(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_STATX_HIST6(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_PKNDX_INB_STAT0(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_PKNDX_INB_STAT1(i), 0);
		cvmx_write_csr_node(node, CVMX_PKI_PKNDX_INB_STAT2(i), 0);
	}
	cvmx_write_csr_node(node, CVMX_PKI_GEN_INT, ~0ull);
	cvmx_write_csr_node(node, CVMX_PKI_ECC_INT0, ~0ull);
	cvmx_write_csr_node(node, CVMX_PKI_ECC_INT1, ~0ull);
	cvmx_write_csr_node(node, CVMX_PKI_ECC_INT2, ~0ull);

	for (i = 0; i < (int)cvmx_pki_num_clusters(); i++) {
		cvmx_write_csr_node(node, CVMX_PKI_CLX_INT(i), ~0ull);
		cvmx_write_csr_node(node, CVMX_PKI_CLX_ECC_INT(i), ~0ull);
	}
}

int pki_find_pkind_chans(int node, int pkind, char *buf, int blen)
{
	int bi, block, port, nbgxs, nchans;
	cvmx_bgxx_cmrx_rx_id_map_t bgxmap;
	cvmx_lbk_chx_pkind_t lbkmap;
	int bcheck_term(int bi, int blen) {
		if ((bi + 8/*name max lendth*/) >= blen) {
			sprintf(&buf[bi], " ..."); 
			return 1;
		}
		return 0;
	}

	memset(buf, 0, blen);
	if ((nbgxs = pki_get_num_bgxs()) == 0)
		return 0;
	/* BGX */
	bi = nchans = 0;
	for (block = 0; block < nbgxs; block++) {
		for (port = 0; port < 4; port++) {
			bgxmap.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_RX_ID_MAP(port, block));
			if (bgxmap.s.pknd == pkind) {
				nchans++;
				bi += sprintf(&buf[bi], " BGX%d:%d", block, port); 
				if (bcheck_term(bi, blen) != 0)
					return nchans;
			}
		}
	}
#if 0
//TODO: Somehow, the register read causes the CPU crash.
	// cvmx_sli_pkt_gbl_control_t dpimap;
	/* DPI */
	dpimap.u64 = cvmx_read_csr_node(node, CVMX_SLI_PKT_GBL_CONTROL);
	(void)dpimap;
	if (dpimap.s.bpkind == pkind) {
		nchans++;
		bi += sprintf(&buf[bi], " DPI"); 
		if (bcheck_term(bi, blen) != 0)
			return nchans;
	}
#endif
	/* LBK */
	for (block = 0; block < 64; block++) {
		lbkmap.u64 = cvmx_read_csr_node(node, CVMX_LBK_CHX_PKIND(block));
		if (lbkmap.s.pkind == pkind) {
			nchans++;
			bi += sprintf(&buf[bi], " LBK:%d", block); 
			if (bcheck_term(bi, blen) != 0)
				return nchans;
		}
	}
	return nchans;
}

#endif /* CVMX_DUMP_PKI */

