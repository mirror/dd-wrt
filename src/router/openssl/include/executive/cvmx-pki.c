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
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-pki-cluster.h>
#include <asm/octeon/cvmx-pki-resources.h>
#else
#include "cvmx.h"
#include "cvmx-version.h"
#include "cvmx-error.h"
#include "cvmx-pki-defs.h"
#include "cvmx-pki.h"
#include "cvmx-fpa.h"
#include "cvmx-pki-resources.h"
#include "cvmx-pki-cluster.h"
#endif


CVMX_SHARED struct cvmx_pki_config pki_config[CVMX_MAX_NODES];
CVMX_SHARED struct cvmx_pki_profiles pki_profiles[CVMX_MAX_NODES];

/**
 * This function enables pki
 * @param node	node to enable pki in.
 */
void cvmx_pki_enable(int node)
{

	cvmx_pki_sft_rst_t pki_sft_rst;
	cvmx_pki_buf_ctl_t pki_en;

	pki_sft_rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);

	while (pki_sft_rst.s.busy != 0)
		pki_sft_rst.u64 = cvmx_read_csr_node(node, CVMX_PKI_SFT_RST);

	pki_en.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	if (pki_en.s.pki_en)
		cvmx_dprintf("Warning: Enabling PKI when PKI already enabled.\n");

	pki_en.s.pki_en = 1;

	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, pki_en.u64);

}
EXPORT_SYMBOL(cvmx_pki_enable);

/**
 * This function disables pki
 * @param node	node to disable pki in.
 */
void cvmx_pki_disable(int node)
{
	cvmx_pki_buf_ctl_t pki_en;
	pki_en.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	pki_en.s.pki_en = 0;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, pki_en.u64);

}
EXPORT_SYMBOL(cvmx_pki_disable);

/**
 * This function sets the clusters in PKI
 * @param node	node to set clusters in.
 */
int cvmx_pki_setup_clusters(int node)
{
	int i;

	for (i = 0; i < cvmx_pki_cluster_code_length; i++)
		cvmx_write_csr_node(node, CVMX_PKI_IMEMX(i), cvmx_pki_cluster_code_default[i]);

	return 0;
}

/**
 * @INTERNAL
 * This function is called by cvmx_helper_shutdown() to extract
 * all FPA buffers out of the PKI. After this function
 * completes, all FPA buffers that were prefetched by PKI
 * wil be in the apropriate FPA pool. This functions does not reset
 * PKI as FPA pool zero must be empty before the reset can
 * be performed. WARNING: It is very important that PKI be
 * reset soon after a call to this function.
 *IT IS STILL TBD IN 78XX_HRM., INPLEMENT ONCE DEFINED
 */
void __cvmx_pki_free_ptr(void)
{
	/*TBD, IMPLEMENT ONCE DEFINED IN HRM*/
}

/**
 * This function writes max and min frame lengths to hardware which can be used
 * to check the size of frame arrived.There are 2 possible combination which are
 * indicated by id field.
 * @param node	 	      node number.
 * @param id		      choose which frame len register to write to
 * @param maxframesize        Byte count for max-sized frame check.
 * @param minframesize        Byte count for min-sized frame check.
 *
 */
int cvmx_pki_write_frame_len(int node, int id, int maxframesize, int minframesize)
{
	cvmx_pki_frm_len_chkx_t frame_len_chk;

	if (maxframesize > CVMX_PKI_MAX_FRAME_SIZE ||
		  minframesize > CVMX_PKI_MAX_FRAME_SIZE){
		cvmx_dprintf("ERROR: invalid frame size maxframe =%d minframe=%d\n",
			     maxframesize, minframesize);
		return -1;
	}
	if (id >= CVMX_PKI_NUM_FRAME_SIZE_ID) {
		cvmx_dprintf("ERROR: invalid id %d in write frame len", id);
		return -1;
	}
	frame_len_chk.u64 = cvmx_read_csr_node(node, CVMX_PKI_FRM_LEN_CHKX(id));
	frame_len_chk.s.maxlen = maxframesize;
	frame_len_chk.s.minlen = minframesize;
	cvmx_write_csr_node(node, CVMX_PKI_FRM_LEN_CHKX(id), frame_len_chk.u64);

	return 0;
}


/**
 * This function writes per pkind parameters in hardware which defines how
  the incoming packet is processed.
 * @param node			node number.
 * @param pkind               PKI supports a large number of incoming interfaces
 *                            and packets arriving on different interfaces or channels
 *                            may want to be processed differently. PKI uses the pkind to
 *                            determine how the incoming packet is processed.
 * @param cluster_group       Which cluster group to use. Application would choose the cluster
 *                            group depending on number of clusters it want to use for that pkind.
 * @param initial_parse_mode  Which initial parsing stage is expected.
 * @param initial_style       Which initial style to assign to this pkind. Style also go as one of
 *                            the inputs to match in the pcam table. If no match is found this initial
 *                            style will be the final style.
 * @param l2_parsing_mask     vinita_to_do
 */
int cvmx_pki_write_pkind(int node, int pkind, int cluster_group,
				     int initial_parse_mode, int initial_style)
{
	uint64_t cluster_mask;
	int cluster = 0;
	cvmx_pki_pkindx_icgsel_t pkind_clsel;
	cvmx_pki_clx_pkindx_style_t pkind_cfg_style;
	cvmx_pki_icgx_cfg_t pki_cl_grp;


	if (pkind >= CVMX_PKI_NUM_PKIND || cluster_group >= CVMX_PKI_NUM_CLUSTER_GROUP
		  || initial_style >= CVMX_PKI_NUM_FINAL_STYLES) {
		cvmx_dprintf("ERROR: Configuring PKIND pkind = %d cluster_group = %d style = %d",
			     pkind, cluster_group, initial_style);
		return -1;
	}
	pkind_clsel.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind));
	pkind_clsel.s.icg = cluster_group;
	cvmx_write_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind), pkind_clsel.u64);

	pki_cl_grp.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(cluster_group));
	cluster_mask = (uint64_t)pki_cl_grp.s.clusters;

	while (cluster < CVMX_PKI_NUM_CLUSTERS) {
		if (cluster_mask & (0x01L << cluster)) {
			pkind_cfg_style.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
			pkind_cfg_style.s.pm = initial_parse_mode;
			pkind_cfg_style.s.style = initial_style;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster), pkind_cfg_style.u64);
		}
		cluster++;
	}
	cvmx_pki_mark_style_in_use(node, initial_style);
	return 0;

}

/**
 * This function writes/configures parameters associated with style in hardware.
 * @param node	              node number.
 * @param style		      style to configure.
 * @param cluster_mask	      Mask of clusters to configure the style for.
 * @param style_cfg		parameters to configure for style passed in struct.
 */
void cvmx_pki_write_style(int node, uint64_t style, uint64_t cluster_mask,
			    struct cvmx_pki_style_config style_cfg)
{
	cvmx_pki_clx_stylex_cfg_t style_cfg_reg;
	cvmx_pki_clx_stylex_cfg2_t style_cfg2_reg;
	cvmx_pki_clx_stylex_alg_t style_alg_reg;
	cvmx_pki_stylex_buf_t     style_buf_reg;
	int cluster = 0;

	/*vinita to_do break it differnt functions*/
	while (cluster < CVMX_PKI_NUM_CLUSTERS) {
		if (cluster_mask & (0x01L << cluster)) {
			style_cfg_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));

			/*style_cfg_reg.s.lenerr_en = style_cfg.en_l2_lenchk;*/
			/*style_cfg_reg.s.lenerr_eqpad = style_cfg.l2_lenchk_mode;*/
			/*style_cfg_reg.s.maxerr_en = style_cfg.en_maxframe_errchk;*/
			/*style_cfg_reg.s.minerr_en = style_cfg.en_minframe_errchk;*/
			/*style_cfg_reg.s.fcs_chk = style_cfg.en_fcs_check;*/
			style_cfg_reg.s.fcs_strip = style_cfg.en_strip_l2_fcs;
			/*style_cfg_reg.s.minmax_sel = style_cfg.max_min_frame_sel;*/
			style_cfg_reg.s.qpg_base = style_cfg.qpg_base_offset;
			style_cfg_reg.s.qpg_dis_padd = style_cfg.en_qpg_calc_port_addr;
			style_cfg_reg.s.qpg_dis_aura = style_cfg.en_qpg_calc_aura;
			style_cfg_reg.s.qpg_dis_grp = style_cfg.en_qpg_calc_group;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster), style_cfg_reg.u64);

			style_alg_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
			style_alg_reg.s.qpg_qos = style_cfg.qpg_qos;
			style_alg_reg.s.tag_vni = style_cfg.tag_fields.tag_vni;
			style_alg_reg.s.tag_gtp = style_cfg.tag_fields.tag_gtp;
			style_alg_reg.s.tag_spi = style_cfg.tag_fields.tag_spi;
			style_alg_reg.s.tag_syn = style_cfg.tag_fields.tag_sync;
			style_alg_reg.s.tag_pctl = style_cfg.tag_fields.ip_prot_nexthdr;
			style_alg_reg.s.tag_vs1 = style_cfg.tag_fields.second_vlan;
			style_alg_reg.s.tag_vs0 = style_cfg.tag_fields.first_vlan;
			style_alg_reg.s.tag_mpls0 = style_cfg.tag_fields.mpls_label;
			style_alg_reg.s.tag_prt = style_cfg.tag_fields.input_port;
			style_alg_reg.s.tt = style_cfg.tag_type;

			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster), style_alg_reg.u64);

			style_cfg2_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
			style_cfg2_reg.s.tag_src_lg = style_cfg.tag_fields.layer_G_src;
			style_cfg2_reg.s.tag_src_lf = style_cfg.tag_fields.layer_F_src;
			style_cfg2_reg.s.tag_src_le = style_cfg.tag_fields.layer_E_src;
			style_cfg2_reg.s.tag_src_ld = style_cfg.tag_fields.layer_D_src;
			style_cfg2_reg.s.tag_src_lc = style_cfg.tag_fields.layer_C_src;
			style_cfg2_reg.s.tag_src_lb = style_cfg.tag_fields.layer_B_src;
			style_cfg2_reg.s.tag_dst_lg = style_cfg.tag_fields.layer_G_dst;
			style_cfg2_reg.s.tag_dst_lf = style_cfg.tag_fields.layer_F_dst;
			style_cfg2_reg.s.tag_dst_le = style_cfg.tag_fields.layer_E_dst;
			style_cfg2_reg.s.tag_dst_ld = style_cfg.tag_fields.layer_D_dst;
			style_cfg2_reg.s.tag_dst_lc = style_cfg.tag_fields.layer_C_dst;
			style_cfg2_reg.s.tag_dst_lb = style_cfg.tag_fields.layer_B_dst;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster), style_cfg2_reg.u64);

		}
		cluster++;
	}
	style_buf_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(style));
	style_buf_reg.s.first_skip = (style_cfg.first_mbuf_skip)/8;
	style_buf_reg.s.later_skip = style_cfg.later_mbuf_skip/8;
	style_buf_reg.s.opc_mode = style_cfg.cache_mode;
	style_buf_reg.s.mb_size = (style_cfg.mbuff_size)/8;
	style_buf_reg.s.dis_wq_dat = 0;
	cvmx_write_csr_node(node, CVMX_PKI_STYLEX_BUF(style), style_buf_reg.u64);
}


/**
 * This function writes pcam entry at given offset in pcam table in hardware
 *
 * @param node	              node number.
 * @param index		      offset in pcam table.
 * @param cluster_mask	      Mask of clusters in which to write pcam entry.
 * @param input 	      input keys to pcam match passed as struct.
 * @param action	      pcam match action passed as struct
 *
 */
int cvmx_pki_pcam_write_entry(int node, int index, uint64_t cluster_mask,
				struct cvmx_pki_pcam_input input,
				struct cvmx_pki_pcam_action action)
{
	int bank;
	int cluster = 0;
	cvmx_pki_clx_pcamx_termx_t	pcam_term;
	cvmx_pki_clx_pcamx_matchx_t	pcam_match;
	cvmx_pki_clx_pcamx_actionx_t	pcam_action;

	if (index >= CVMX_PKI_TOTAL_PCAM_ENTRY) {
		cvmx_dprintf("\nERROR: Invalid pcam entry %d", index);
		return -1;
	}
	bank = (int)(input.field & 0x01);
	while (cluster < CVMX_PKI_NUM_CLUSTERS) {
		if (cluster_mask & (0x01L << cluster)) {
			pcam_match.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index));
			pcam_match.s.data1 = input.data & input.data_mask;
			pcam_match.s.data0 = (~input.data) & input.data_mask;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index), pcam_match.u64);
			pcam_action.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index));
			pcam_action.s.pmc = action.parse_mode_chg;
			pcam_action.s.style_add = action.style_add;
			pcam_action.s.pf = action.parse_flag_set;
			pcam_action.s.setty = action.layer_type_set;
			pcam_action.s.advance = action.pointer_advance;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index), pcam_action.u64);
			pcam_term.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index));
			pcam_term.s.term1 = input.field & input.field_mask;
			pcam_term.s.term0 = (~input.field) & input.field_mask;
			pcam_term.s.style1 = input.style & input.style_mask;
			pcam_term.s.style0 = (~input.style) & input.style_mask;
			pcam_term.s.valid = 1;
			cvmx_write_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index), pcam_term.u64);
		}
		cluster++;
	}
	return 0;

}

/**
 * Enables/Disabled QoS (RED Drop, Tail Drop & backpressure) for the
 * PKI aura.
 * @param node      node number
 * @param aura      to enable/disable QoS on.
 * @param ena_red   Enable/Disable RED drop between pass and drop level
 *                  1-enable 0-disable
 * @param ena_drop  Enable/disable tail drop when max drop level exceeds
 *                  1-enable 0-disable
 * @param ena_red   Enable/Disable asserting backpressure on bpid when
 *                  max DROP level exceeds.
 *                  1-enable 0-disable
 */
int cvmx_pki_enable_aura_qos(int node, int aura, bool ena_red,
			     bool ena_drop, bool ena_bp)
{
	cvmx_pki_aurax_cfg_t pki_aura_cfg;

	if (aura >= CVMX_PKI_NUM_AURA) {
		cvmx_dprintf("ERROR: PKI config aura_qos aura = %d", aura);
		return -1;
	}
	pki_aura_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_AURAX_CFG(aura));
	pki_aura_cfg.s.ena_red = ena_red;
	pki_aura_cfg.s.ena_drop = ena_drop;
	pki_aura_cfg.s.ena_bp = ena_bp;
	cvmx_write_csr_node(node, CVMX_PKI_AURAX_CFG(aura), pki_aura_cfg.u64);
	return 0;
}

/**
 * Configures the bpid on which, specified aura will
 * assert backpressure.
 * Each bpid receives backpressure from auras.
 * Multiple auras can backpressure single bpid.
 * @param node   node number
 * @param aura   number which will assert backpressure on that bpid.
 * @param bpid   to assert backpressure on.
 */
int cvmx_pki_write_aura_bpid(int node, int aura, int bpid)
{
	cvmx_pki_aurax_cfg_t pki_aura_cfg;

	if (aura >= CVMX_PKI_NUM_AURA || bpid >= CVMX_PKI_NUM_BPID) {
		cvmx_dprintf("ERROR: PKI config aura_bp aura = %d bpid = %d", aura, bpid);
		return -1;
	}
	pki_aura_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_AURAX_CFG(aura));
	pki_aura_cfg.s.bpid = bpid;
	cvmx_write_csr_node(node, CVMX_PKI_AURAX_CFG(aura), pki_aura_cfg.u64);
	return 0;
}

/**
 * Configures the channel which will receive backpressure
 * from the specified bpid.
 * Each channel listens for backpressure on a specific bpid.
 * Each bpid can backpressure multiple channels.
 * @param node    node number
 * @param bpid    bpid from which, channel will receive backpressure.
 * @param channel channel numner to receive backpressue.
 */
int cvmx_pki_write_channel_bpid(int node, int channel, int bpid)
{
	cvmx_pki_chanx_cfg_t pki_chan_cfg;

	if (channel >= CVMX_PKI_NUM_CHANNELS || bpid >= CVMX_PKI_NUM_BPID) {
		cvmx_dprintf("ERROR: PKI config channel_bp channel = %d bpid = %d", channel, bpid);
		return -1;
	}

	pki_chan_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CHANX_CFG(channel));
	pki_chan_cfg.s.bpid = bpid;
	cvmx_write_csr_node(node, CVMX_PKI_CHANX_CFG(channel), pki_chan_cfg.u64);
	return 0;
}

int cvmx_pki_frame_len_check(int node, int id, int maxframesize, int minframesize)
{
	cvmx_pki_frm_len_chkx_t frame_len_chk;

	frame_len_chk.u64 = cvmx_read_csr_node(node, CVMX_PKI_FRM_LEN_CHKX(id));
	if (maxframesize && minframesize) {
		if (frame_len_chk.s.maxlen != maxframesize ||
				 frame_len_chk.s.minlen != minframesize)
			return -1;
	} else if (maxframesize && frame_len_chk.s.maxlen != maxframesize)
		return -1;
	else if (minframesize && frame_len_chk.s.minlen != minframesize)
		return -1;
	return 0;
}

int cvmx_pki_set_l2_frame_len(int node, uint64_t maxframesize, uint64_t minframesize)
{
	if (cvmx_pki_frame_len_check(node, 0, maxframesize, minframesize)) {
		if (cvmx_pki_frame_len_check(node, 1, maxframesize, minframesize)) {
			cvmx_dprintf("ERROR: No frame len match");
			return -1;
		}
		return 1;
	} else
		return 0;
}

/**
 * This function finds if cluster profile with name already exist
 * @param node  node number
 * @param name  profile name to look for
 * @return 	profile index in cluster list on SUCCESS
 *		-1 if profile not found in cluster list
 */
int cvmx_pki_cluster_profile_exist(int node, char *name)
{
	int index = pki_profiles[node].cl_profile_list.index;

	while (index--) {
		if (strcmp(name, pki_profiles[node].cl_profile_list.cl_profile[index].name) == 0)
			return index;
	}
	return -1;
}

/**
 * This function finds cluster mask associated with
 * given cluster profile name.
 * @param node  node number
 * @param name  profile name to look for
 * @return 	cluster_mask on SUCCESS
 *		-1 if profile not found in cluster list
 */
int cvmx_pki_find_cluster_mask(int node, char *name)
{
	int index;
	int cl_grp;

	index = cvmx_pki_cluster_profile_exist(node, name);
	if (index == -1)
		return -1;

	cl_grp = pki_profiles[node].cl_profile_list.cl_profile[index].cluster_group;
	return pki_config[node].cluster_cfg[cl_grp].cluster_mask;

}

/**
 * This function finds cluster group associated with
 * given cluster profile name
 * @param node  node number
 * @param name  profile name to look for
 * @return 	cluster group number on SUCCESS
 *		-1 if profile not found in cluster list
 */
int cvmx_pki_find_cluster_group(int node, char *name)
{
	int index;

	index = cvmx_pki_cluster_profile_exist(node, name);
	if (index  ==  -1)
		return -1;
	return pki_profiles[node].cl_profile_list.cl_profile[index].cluster_group;
}

/**
 * This function finds if fpa pool profile with
 * name already exist
 * @param node  node number
 * @param name  profile name to look for
 * @return 	profile index in pool list on SUCCESS
 *		-1 if profile not found in pool list
 */
int cvmx_pki_pool_profile_exist(int node, char *name)
{
	int index = pki_profiles[node].pool_profile_list.index;

	while (index--) {
		if (strcmp(name, pki_profiles[node].pool_profile_list.pool_profile[index].pool_name) == 0) {
			return index;
		}
	}
	return -1;
}

/**
 * This function finds if fpa pool number associated with
 * given profile name
 * @param node  node number
 * @param name  profile name to look for
 * @return 	pool number on SUCCESS
 *		-1 if profile not found in pool list
 */
int cvmx_pki_find_pool(int node, char *name)
{
	int index;

	index = cvmx_pki_pool_profile_exist(node, name);
	if (index == -1)
		return -1;
	return pki_profiles[node].pool_profile_list.pool_profile[index].pool_num;
}

/**
 * This function finds if fpa aura with given name
 * exist in aura list
 * @param node  node number
 * @param name  profile name to look for
 * @return 	aura index in aura list on SUCCESS
 *		-1 if profile not found in aura list
 */
int cvmx_pki_aura_profile_exist(int node, char *name)
{
	int index = pki_profiles[node].aura_profile_list.index;

	while (index--) {
		if (strcmp(name, pki_profiles[node].aura_profile_list.aura_profile[index].aura_name) == 0)
			return index;
	}
	return -1;
}

/**
 * This function finds aura number associated with
 * given aura name.
 * @param node  node number
 * @param name  profile name to look for
 * @return 	aura number in aura list on SUCCESS
 *		-1 if profile not found in aura list
 */
int cvmx_pki_find_aura(int node, char *name)
{
	int index;

	index = cvmx_pki_aura_profile_exist(node, name);
	if (index == -1)
		return -1;
	return pki_profiles[node].aura_profile_list.aura_profile[index].aura_num;
}

/**
 * This function finds if group with given name
 * exist in group list
 * @param node  node number
 * @param name  profile name to look for
 * @return 	index in group list on SUCCESS
 *		-1 if profile not found in group list
 */
int cvmx_pki_group_profile_exist(int node, char *name)
{
	int index = pki_profiles[node].sso_grp_profile_list.index;

	while (index--) {
		if (strcmp(name, pki_profiles[node].sso_grp_profile_list.grp_profile[index].grp_name) == 0)
			return index;
	}
	return -1;
}

/**
 * This function finds group number associated with
 * given group profile name
 * @param node  node number
 * @param name  profile name to look for
 * @return 	group number on SUCCESS
 *		-1 if profile not found in group list
 */
int cvmx_pki_find_group(int node, char *name)
{
	int index;

	index = cvmx_pki_group_profile_exist(node, name);
	if (index == -1)
		return -1;
	return pki_profiles[node].sso_grp_profile_list.grp_profile[index].grp_num;
}

/**
 * This function finds if qpg profile with given name
 * exist in qpg list
 * @param node  node number
 * @param name  profile name to look for
 * @return 	index in qpg list on SUCCESS
 *		-1 if profile not found in qpg list
 */
int cvmx_pki_qpg_profile_exist(int node, char *name)
{
	int index = pki_profiles[node].qpg_profile_list.index;

	while (index--) {
		if (strcmp(name, pki_profiles[node].qpg_profile_list.qpg_profile[index].qpg_name) == 0)
			return index;
	}
	return -1;
}

/**
 * This function finds qpg base offset associated with
 * given qpg profile name.
 * @param node  node number
 * @param name  profile name to look for
 * @return 	qpg base offset on SUCCESS
 *		-1 if profile not found in qpg list
 */
int cvmx_pki_find_qpg_base_offset(int node, char *name)
{
	int index;

	index = cvmx_pki_qpg_profile_exist(node, name);
	if (index == -1)
		return -1;
	return pki_profiles[node].qpg_profile_list.qpg_profile[index].base_offset;
}

/**
 * This function get the buffer size of the given pool number
 * @param node  node number
 * @param pool  fpa pool number
 * @return 	buffer size SUCCESS
 *		-1 if pool number is not found in pool list
 */
int cvmx_pki_get_pool_buffer_size(int node, int pool)
{
	int index = pki_profiles[node].aura_profile_list.index;

	while (index--) {
		if (pki_profiles[node].pool_profile_list.pool_profile[index].pool_num == pool)
			return pki_profiles[node].pool_profile_list.pool_profile[index].buffer_size;
	}
	return -1;
}

/**
 * This function get the buffer size of the given aura number
 * @param node  node number
 * @param pool  fpa aura number
 * @return 	buffer size SUCCESS
 *		-1 if aura number is not found in aura list
 */
int cvmx_pki_get_aura_buffer_size(int node, int aura)
{
	int index = pki_profiles[node].aura_profile_list.index;
	int pool_num;

	while (index--) {
		if (pki_profiles[node].aura_profile_list.aura_profile[index].aura_num == aura) {
			pool_num = pki_profiles[node].aura_profile_list.aura_profile[index].pool_num;
			return cvmx_pki_get_pool_buffer_size(node, pool_num);
		}
	}
	return -1;
}

int cvmx_pki_get_mbuff_size (int node, int base_offset)
{
	int index = pki_profiles[node].qpg_profile_list.index;
	int aura;
	int min_size;
	int aura_size;
	int i;

	while (index--) {
		if (pki_profiles[node].qpg_profile_list.qpg_profile[index].base_offset == base_offset) {
			int num_entry = pki_profiles[node].qpg_profile_list.qpg_profile[index].num_entries;
			aura = pki_config[node].qpg_cfg[base_offset].aura;
			min_size = cvmx_pki_get_aura_buffer_size(node, aura);
			for (i = 1; i < num_entry; i++) {
				aura = pki_config[node].qpg_cfg[base_offset+i].aura;
				aura_size = cvmx_pki_get_aura_buffer_size(node, aura);
				if (min_size > aura_size)
					min_size = aura_size;
			}
			return min_size;
		}
	}
	return -1;
}

/**
 * This function finds if style profile with given name
 * exist in style list
 * @param node  node number
 * @param name  profile name to look for
 * @return 	index into style list on SUCCESS
 *		-1 if profile not found in style list
 */
int cvmx_pki_style_profile_exist(int node, char *name)
{
	int index = pki_profiles[node].style_profile_list.index;

	while (index--) {
		if (strcmp(name, pki_profiles[node].style_profile_list.style_profile[index].name) == 0)
			return index;
	}
	return -1;
}

/**
 * This function finds style number associated with
 * given style profile name.
 * @param node  node number
 * @param name  profile name to look for
 * @return 	style number on SUCCESS
 *		-1 if profile not found in style list
 */
int cvmx_pki_find_style(int node, char *name)
{
	int index;

	index = cvmx_pki_style_profile_exist(node, name);
	if (index == -1)
		return -1;
	return pki_profiles[node].style_profile_list.style_profile[index].style_num;
}

/**
 * This function stores the cluster configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param name  	name associated with this config
 * @param cl_profile    structure containing cluster profile parameters below
 * 			-cluster_group (-1 if needs to be allocated)
 * 			-num_cluster   (number of cluster in the cluster group)
 * 			-parsing_mask  (parsing mask for the cluster group)
 * @return 		cluster group number on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_cluster_config(int node, struct cvmx_pki_cluster_profile cl_profile)
{
	int index;
	int cluster_group;
	uint64_t cluster_mask = 0;

	if (node >= CVMX_MAX_NODES) {
		cvmx_dprintf("Invalid node number %d", node);
		return -1;
	}
	index = cvmx_pki_cluster_profile_exist(node, cl_profile.name);
	if (index >= 0) {
		cvmx_dprintf("Warning: Modifing cluster profile with name %s\n", cl_profile.name);
		cluster_group = 0; /*vinita_to_do*/

	} else {
		cluster_group = cvmx_pki_alloc_cluster_group(node, cl_profile. cluster_group);
		if (cluster_group == -1) {
			cvmx_dprintf("ERROR:allocating cluster_group %d\n", cl_profile. cluster_group);
			return -1;
		}
		if (cvmx_pki_alloc_clusters(node, cl_profile.num_clusters, &cluster_mask)) {
			cvmx_dprintf("ERROR:allocating clusters %d\n", cl_profile. num_clusters);
			return -1;
		}
		cl_profile.cluster_group = cluster_group;
		/*spinlock it*/
		index = pki_profiles[node].cl_profile_list.index;
		if (index >= CVMX_PKI_MAX_CLUSTER_PROFILES) {
			cvmx_dprintf("ERROR: Max cluster profiles %d reached\n", index);
			return -1;
		}
		pki_profiles[node].cl_profile_list.index++;
		pki_profiles[node].cl_profile_list.cl_profile[index] = cl_profile;
		pki_config[node].cluster_cfg[cluster_group].cluster_mask = cluster_mask;
		pki_config[node].cluster_cfg[cluster_group].users++;
		/*spinlock free*/
	}
	return cluster_group;
}

/**
 * This function stores the pool configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param pool_name  	name associated with this config
 * @param pool_numb     pool number (-1 if need to be allocated)
 * @param buffer_size	size of buffers in specified pool
 * @param num_buffers	numberof buffers in specified pool
 * @return 		pool number on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_pool_config(int node, char *pool_name, int pool_num,
			     uint64_t buffer_size, uint64_t num_buffers)
{
	uint64_t index;
	struct cvmx_pki_pool_profile *pool_profile;

	if (node >= CVMX_MAX_NODES) {
		cvmx_dprintf("Invalid node number %d", node);
		return -1;
	}
	if (cvmx_pki_pool_profile_exist(node, pool_name) >= 0) {
		cvmx_dprintf("Warning: Modifing pool profile with name %s\n", pool_name);
		pool_num = cvmx_pki_find_pool(node, pool_name);
	} else {
		if (cvmx_fpa_allocate_fpa_pools(node, &pool_num, 1) == -1) {
			cvmx_dprintf("ERROR:allocating pool for pool_config\n");
			return -1;
		}
		/*spinlock it*/
		index = pki_profiles[node].pool_profile_list.index;
		if (index >= CVMX_PKI_MAX_POOL_PROFILES) {
			cvmx_dprintf("ERROR: Max pool profile %d reached\n", (int)index);
			return -1;
		}
		pki_profiles[node].pool_profile_list.index++;
		/*spinlock free*/
		pool_profile = &pki_profiles[node].pool_profile_list.pool_profile[index];
		strcpy(pool_profile->pool_name, pool_name);
		pool_profile->pool_num = pool_num;
		pool_profile->buffer_size = buffer_size;
		pool_profile->buffer_count = num_buffers;
		pki_config[node].pool_cfg[pool_num].users++;
	}
	pki_config[node].pool_cfg[pool_num].buffer_size = buffer_size;
	pki_config[node].pool_cfg[pool_num].buffer_count = num_buffers;

	return pool_num;
}

/**
 * This function stores the aura configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param aura_name  	name associated with this config
 * @param aura_num      aura number (-1 if need to be allocated)
 * @param pool  	pool to which aura is mapped
 * @param num_buffers	number of buffers to allocate to aura.
 * @return 		aura number on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_aura_config(int node, char *aura_name, int aura_num, int pool,
			     int num_buffers)
{
	int64_t index;
	struct cvmx_pki_aura_profile *aura_profile;

	if (node >= CVMX_MAX_NODES) {
		cvmx_dprintf("Invalid node number %d", node);
		return -1;
	}
	if (cvmx_pki_aura_profile_exist(node, aura_name) >= 0) {
		cvmx_dprintf("Warning: Modifing aura profile with name %s\n", aura_name);
		aura_num = cvmx_pki_find_aura(node, aura_name);
	} else {
		aura_num = cvmx_fpa_allocate_auras(node, &aura_num, 1);
		if (aura_num == -1) {
			cvmx_dprintf("ERROR:allocating aura for aura_config\n");
			return -1;
		}
		/*spinlock it*/
		index = pki_profiles[node].aura_profile_list.index;
		if (index >= CVMX_PKI_MAX_AURA_PROFILES) {
			cvmx_dprintf("ERROR: Max aura profile %d reached\n", (int)index);
			return -1;

		}
		pki_profiles[node].aura_profile_list.index++;
		/*spinlock free*/
		aura_profile = &pki_profiles[node].aura_profile_list.aura_profile[index];
		strcpy(aura_profile->aura_name, aura_name);
		aura_profile->aura_num = aura_num;
		aura_profile->pool_num = pool;
		aura_profile->buffer_count = num_buffers;
		pki_config[node].aura_cfg[aura_num].users++;
	}
	pki_config[node].aura_cfg[aura_num].pool_num = pool;
	pki_config[node].pool_cfg[aura_num].buffer_count = num_buffers;
	return aura_num;
}


/**
 * This function stores the group configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param grp_profile	struct to SSO group profile to configure
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_sso_group_config(int node, struct cvmx_pki_sso_grp_profile grp_profile)
{
	int64_t index;

	if (node >= CVMX_MAX_NODES) {
		cvmx_dprintf("Invalid node number %d", node);
		return -1;
	}
	index = cvmx_pki_group_profile_exist(node, grp_profile. grp_name);
	if (index >= 0) {
		cvmx_dprintf("Warning: Modifing sso_grp profile with name %s\n", grp_profile.grp_name);
	} else {
		#if 0 /*vinita_to_do uncomment when group_alloc is ready*/
		group = cvmx_pki_allocate_group(node, group);
		if (group == -1) {
			cvmx_dprintf("ERROR:allocating group for group_config\n");
			return -1;
		}
		#endif

		/*spinlock it*/
		index = pki_profiles[node].sso_grp_profile_list.index;
		if (index >= CVMX_PKI_NUM_SSO_GROUP) {
			cvmx_dprintf("ERROR: Max group profile %d reached\n", (int)index);
			return -1;
		}
		pki_profiles[node].sso_grp_profile_list.index++;
		/*spinlock free*/
	}
	pki_profiles[node].sso_grp_profile_list.grp_profile[index] = grp_profile;
	return 0;
}

/**
 * This function stores the qpg configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param name  	name associated with this config
 * @param base_offset	offset in QPG table (-1 if needs to be allocated)
 * @param num_entries	total number of indexes needs to be allocated from
 *                      base_offset.
 * @return 		base_offset index on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_qpg_profile(int node, char *name, int base_offset, int num_entries)
{
	int64_t index;
	struct cvmx_pki_qpg_profile *qpg_profile;

	if (node >= CVMX_MAX_NODES) {
		cvmx_dprintf("Invalid node number %d", node);
		return -1;
	}
	index = cvmx_pki_qpg_profile_exist(node, name);
	if (index >= 0)
		qpg_profile = &pki_profiles[node].qpg_profile_list.qpg_profile[index];
	else {
		base_offset = cvmx_pki_alloc_qpg_entry(node, base_offset, num_entries);
		if (base_offset == -1) {
			cvmx_dprintf("ERROR:allocating entry for qpg_table\n");
			return -1;
		}
		/*spinlock it*/
		index = pki_profiles[node].qpg_profile_list.index;
		if (index >= CVMX_PKI_MAX_QPG_PROFILES) {
			cvmx_dprintf("ERROR: Max qpg profile %d reached\n", (int)index);
			return -1;
		}
		pki_profiles[node].qpg_profile_list.index++;
		/*spinlock free*/
		qpg_profile = &pki_profiles[node].qpg_profile_list.qpg_profile[index];
		strcpy(qpg_profile->qpg_name, name);
		qpg_profile->base_offset = base_offset;
	}
	qpg_profile->num_entries = num_entries;
	return base_offset;
}

/**
 * This function stores the qpg configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param name  	name of qpg profile associated with this config
 * @param entry_start   starting entry to configure with given parameters.
 * @param entry_end     ending entry to configure with given parameters.
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_qpg_config(int node, char *name, int entry_start,
			    int entry_end, struct cvmx_pki_qpg_config qpg_config)
{
	int index;
	int base_offset;
	int num_entry;

	if (node >= CVMX_MAX_NODES) {
		cvmx_dprintf("Invalid node number %d", node);
		return -1;
	}
	index = cvmx_pki_qpg_profile_exist(node, name);
	if (index < 0) {
		cvmx_dprintf("ERROR:qpg profile %s not found\n", name);
		return -1;
	}
	base_offset = pki_profiles[node].qpg_profile_list.qpg_profile[index].base_offset;
	if (base_offset < 0) {
		cvmx_dprintf("ERROR: invalid base offset %d in qpg profile %s", base_offset, name);
		return -1;
	}
	num_entry = pki_profiles[node].qpg_profile_list.qpg_profile[index].num_entries;
	if (entry_start > num_entry || entry_end > num_entry) {
		cvmx_dprintf("ERROR: start_entry %llu or end_entry %llu is > %llu for qpg_profile %s\n",
			     (unsigned long long)entry_start, (unsigned long long)entry_end, (unsigned long long)num_entry, name);
	}
	while (entry_start <= entry_end) {
		pki_config[node].qpg_cfg[base_offset + entry_start] = qpg_config;
		pki_config[node].qpg_cfg[base_offset + entry_start].users++;
		entry_start++;
	}
	return 0;
}

/**
 * This function stores the style configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param aura_name  	name associated with this config
 * @param style_num	style number (-1 if needs to be allocated)
 * @param style_cfg	pointer to struct which has parameters related
 *                      to style config
 * @return 		style number on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_style_config(int node, char *style_name, int style_num,
			      struct cvmx_pki_style_config *style_cfg)
{
	int index;
	struct cvmx_pki_style_profile *style_profile;

	if (node >= CVMX_MAX_NODES) {
		cvmx_dprintf("Invalid node number %d", node);
		return -1;
	}
	index = cvmx_pki_style_profile_exist(node, style_name);
	if (index >= 0) {
		style_num = cvmx_pki_find_style(node, style_name);
		cvmx_dprintf("Warning: Modifing style profile with name %s\n", style_name);
	} else {
		style_num = cvmx_pki_alloc_style(node, style_num);
		if (style_num == -1) {
			cvmx_dprintf("ERROR:allocating style for style_config\n");
			return -1;
		}
		/*spinlock it*/
		index = pki_profiles[node].style_profile_list.index;
		if (index >= CVMX_PKI_MAX_STYLE_PROFILES) {
			cvmx_dprintf("ERROR: Max style profile %d reached\n", (int)index);
			return -1;

		}
		pki_profiles[node].style_profile_list.index++;
		/*spinlock free*/
		style_profile = &pki_profiles[node].style_profile_list.style_profile[index];
		strcpy(style_profile->name, style_name);
		style_profile->style_num = style_num;
		pki_config[node].style_cfg[style_num].users++;
	}
	if (style_num >= CVMX_PKI_NUM_FINAL_STYLES)
		return -1;
	memcpy(&pki_config[node].style_cfg[style_num], style_cfg, sizeof(struct cvmx_pki_style_config));
	return style_num;
}

/**
 * This function stores the pkind style configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param pkind  	pkind number
 * @param style		style number which need to be assigned to pkind
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_port_style_cfg(uint64_t node, uint64_t interface, uint64_t port, uint64_t style)
{
	pki_config[node].port_cfg[interface][port].initial_style = style;
	pki_config[node].style_cfg[style].cluster_mask = pki_config[node].port_cfg[interface][port].cluster_mask;
	return 0;
}

/**
 * This function stores the pkind initial parse mode in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param pkind  	pkind number
 * @param parse_mode    parse mode to assign to specified pkind.
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
void cvmx_pki_set_port_parse_mode_cfg(uint64_t node, uint64_t interface,
					  uint64_t port, enum cvmx_pki_pkind_parse_mode parse_mode)
{
	pki_config[node].port_cfg[interface][port].parsing_mode = parse_mode;
}

/**
 * This function stores the pkind cluster configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param pkind  	pkind number
 * @param style_name	pointer to style name which need to be assigned to pkind
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
void cvmx_pki_set_port_cluster_config(int node, uint64_t interface, uint64_t port,
					   int cl_grp, uint64_t cl_mask)
{
	pki_config[node].port_cfg[interface][port].cluster_grp = cl_grp;
	pki_config[node].port_cfg[interface][port].cluster_mask = cl_mask;
}

/**
 * This function stores the pcam entry in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param pcam_index	which pcam entry to configure (-1 to allocate from available entries)
 * @param cluster_mask	Mask of clusters on which the entry meeds to be appiled.
 * @param input		structure of pcam input parameter which defines matching creteria.
 * @param action	structure of pcam action parameters which aill be applied if match is found.
 * @return              0 on scuccess
 *			-1 on failure
 */
int cvmx_pki_set_pcam_entry(int node, int pcam_index, uint64_t cl_mask,
			       struct cvmx_pki_pcam_input input,
			       struct cvmx_pki_pcam_action action)
{
	uint64_t index;

	if (node >= CVMX_MAX_NODES) {
		cvmx_dprintf("Invalid node number %d", node);
		return -1;
	}

	/*spinlock it*/
	index = pki_profiles[node].pcam_list.index;
	if (index >= CVMX_PKI_TOTAL_PCAM_ENTRY) {
		cvmx_dprintf("ERROR: Max pcam lists %d reached\n", (int)index);
		return -1;

	}
	pki_profiles[node].pcam_list.index++;
	/*spinlock free*/

	pki_profiles[node].pcam_list.pcam_cfg[index].cluster_mask = cl_mask;
	pki_profiles[node].pcam_list.pcam_cfg[index].entry_num = pcam_index;
	pki_profiles[node].pcam_list.pcam_cfg[index].pcam_input = input;
	pki_profiles[node].pcam_list.pcam_cfg[index].pcam_action = action;
	return 0;
}

/**
 * This function shows the pcam table in raw format,
 * read directly from hardware.
 * @param node    node number
 */
void cvmx_pki_show_pcam_entries(int node)
{
	int cluster;
	int index;
	int bank;

	for (cluster = 0; cluster < 4; cluster++) {
		for (bank = 0; bank < 2; bank++) {
			cvmx_dprintf("\n--------------Cluster %1d Bank %1d-------------\n", cluster, bank);
			cvmx_dprintf("index         TERM                 DATA,                ACTION");
			for (index = 0; index < CVMX_PKI_NUM_PCAM_ENTRY; index++) {
				cvmx_dprintf("\n%d", index);
				cvmx_dprintf("             %-16lx",
				     (unsigned long)cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index)));
				cvmx_dprintf("     %-16lx",
					     (unsigned long)cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index)));
				cvmx_dprintf("     %-16lx",
					     (unsigned long)cvmx_read_csr_node(node, CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index)));
			}
		}
	}
}

/**
 * This function shows the valid entries in readable format,
 * read directly from hardware.
 * @param node    node number
 */
void cvmx_pki_show_valid_pcam_entries(int node)
{
	int cluster;
	int index;
	int bank;
	cvmx_pki_clx_pcamx_termx_t	pcam_term;
	cvmx_pki_clx_pcamx_matchx_t	pcam_match;
	cvmx_pki_clx_pcamx_actionx_t	pcam_action;

	/*vinita_to_do, later modify to use/t/t etc*/
	for (cluster = 0; cluster < 4; cluster++) {
		for (bank = 0; bank < 2; bank++) {
			cvmx_dprintf("\n--------------Cluster %1d Bank %1d---------------------\n", cluster, bank);
			cvmx_dprintf("%-10s%-17s%-19s%-18s", "index",
				     "TERM1:TERM0", "Style1:Style0", "Data1:Data0");
			cvmx_dprintf("%-6s", "ACTION[pmc:style_add:pf:setty:advance]");
			for (index = 0; index < CVMX_PKI_NUM_PCAM_ENTRY; index++) {
				pcam_term.u64 = cvmx_read_csr_node(node,
						CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index));
				if (pcam_term.s.valid) {
					pcam_match.u64 = cvmx_read_csr_node(node,
							CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank, index));
					pcam_action.u64 = cvmx_read_csr_node(node,
							CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank, index));
					cvmx_dprintf("\n%-13d", index);
					cvmx_dprintf("%-2x:%x", pcam_term.s.term1, pcam_term.s.term0);
					cvmx_dprintf("     	      %-2x:%x", pcam_term.s.style1, pcam_term.s.style0);
					cvmx_dprintf("        %-8x:%x", pcam_match.s.data1, pcam_match.s.data0);
					cvmx_dprintf("            %-2x:%-2x       :%-1x :%2x   :%-2x",
						pcam_action.s.pmc, pcam_action.s.style_add, pcam_action.s.pf, pcam_action.s.setty, pcam_action.s.advance);

				}
			}
		}
	}
}

/**
 * This function shows the pkind attributes in readable format,
 * read directly from hardware.
 * @param node    node number
 */
void cvmx_pki_show_pkind_attributes(int node, int pkind)
{
	int cluster = 0;
	int index;
	cvmx_pki_pkindx_icgsel_t pkind_clsel;
	cvmx_pki_clx_pkindx_style_t pkind_cfg_style;
	cvmx_pki_icgx_cfg_t pki_cl_grp;
	cvmx_pki_clx_stylex_cfg_t style_cfg;
	cvmx_pki_clx_stylex_alg_t style_alg;

	if (pkind >= CVMX_PKI_NUM_PKIND) {
		cvmx_dprintf("ERROR: PKIND %d is beyond range\n", pkind);
		return;
	}
	pkind_clsel.u64 = cvmx_read_csr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind));
	cvmx_dprintf("cluster group:	%d\n", pkind_clsel.s.icg);
	pki_cl_grp.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(pkind_clsel.s.icg));
	cvmx_dprintf("cluster mask of the group:	0x%x\n", pki_cl_grp.s.clusters);

	while (cluster < CVMX_PKI_NUM_CLUSTERS) {
		if (pki_cl_grp.s.clusters & (0x01L << cluster)) {
			/*vinita_to_do later modify in human readble format or now just print register value*/
			pkind_cfg_style.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
			cvmx_dprintf("initial parse Mode: %d\n", pkind_cfg_style.s.pm);
			cvmx_dprintf("initial_style: %d\n", pkind_cfg_style.s.style);
			style_alg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_ALG(pkind_cfg_style.s.style, cluster));
			cvmx_dprintf("style_alg: 0x%llx\n", (unsigned long long)style_alg.u64);
			style_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(pkind_cfg_style.s.style, cluster));
			cvmx_dprintf("style_cfg: 0x%llx\n", (unsigned long long)style_cfg.u64);
			cvmx_dprintf("style_cfg2: 0x%llx\n",
				     (unsigned long long)cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG2(pkind_cfg_style.s.style, cluster)));
			cvmx_dprintf("style_buf: 0x%llx\n",
				     (unsigned long long)cvmx_read_csr_node(node, CVMX_PKI_STYLEX_BUF(pkind_cfg_style.s.style)));
			break;
		}
	}
	cvmx_dprintf("qpg base: %d\n", style_cfg.s.qpg_base);
	cvmx_dprintf("qpg qos: %d\n", style_alg.s.qpg_qos);
	for (index = 0; index < 8; index++) {
		cvmx_dprintf("qpg index %d: 0x%llx\n", (index+style_cfg.s.qpg_base),
			     (unsigned long long)cvmx_read_csr_node(node, CVMX_PKI_QPG_TBLX(style_cfg.s.qpg_base+index)));
	}
}

void cvmx_pki_set_default_pool_config(int node, int64_t pool,
				      int64_t buffer_size, int64_t buffer_count)
{
	pki_profiles[node].pool_profile_list.pool_profile[0].pool_num = pool;
	pki_profiles[node].pool_profile_list.pool_profile[0].buffer_size = buffer_size;
	pki_profiles[node].pool_profile_list.pool_profile[0].buffer_count = buffer_count;
}

void cvmx_pki_set_default_aura_config(int node, int64_t aura,
				      int64_t pool, uint64_t buffer_count)
{
	pki_profiles[node].aura_profile_list.aura_profile[0].aura_num = aura;
	pki_profiles[node].aura_profile_list.aura_profile[0].pool_num = pool;
	pki_profiles[node].aura_profile_list.aura_profile[0].buffer_count = buffer_count;
}

void cvmx_pki_set_default_pool_buffer_count(int node, uint64_t buffer_count)
{
	pki_profiles[node].pool_profile_list.pool_profile[0].buffer_count = buffer_count;
	pki_profiles[node].aura_profile_list.aura_profile[0].buffer_count = buffer_count;
}

void cvmx_pki_get_style_config(int node, int style, struct cvmx_pki_style_config *style_cfg)
{
	*style_cfg = pki_config[node].style_cfg[style];
}

int cvmx_pki_initialize_data_structures(int node)
{
	int intf, index;
	int style;
	int pool, aura, base_offset, cl_grp;
	struct cvmx_pki_style_config style_cfg;
	struct cvmx_pki_cluster_profile cl_profile;
	struct cvmx_pki_qpg_config qpg_cfg;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
	memset(&style_cfg, 0 , sizeof(style_cfg));
	memset(&cl_profile, 0, sizeof(cl_profile));
	memset(&qpg_cfg, 0, sizeof(qpg_cfg));

	/*initialize cluster group 0 to use all cluster engines*/
	strcpy(cl_profile.name, "default_clg");
	cl_profile.num_clusters = 4;
	cl_profile.cluster_group = CVMX_PKI_FIND_AVAL_ENTRY;
	cl_grp = cvmx_pki_set_cluster_config(node, cl_profile);
	if (cl_grp < 0) { /*vinita_to_do check if we can reuse the cluster group*/
		cvmx_dprintf("Error: cluster grp alloc in pki_init_data_struct\n");
		return -1;
	}

	/*initialize pool */
	pool = cvmx_pki_set_pool_config(node, "default_pool", CVMX_PKI_FIND_AVAL_ENTRY, 2048, 1000);
	if (pool < 0) {
		cvmx_dprintf("Error:pool alloc in pki_init_data_struct\n");
		return -1;
	}

	/*initialize aura*/
	aura = cvmx_pki_set_aura_config(node, "default_aura", CVMX_PKI_FIND_AVAL_ENTRY, pool, 1000);
	if (aura < 0) {
		cvmx_dprintf("Error: aura alloc in pki_init_data_struct\n");
		return -1;
	}

	/*initialize first entry in qpg table*/
	base_offset = cvmx_pki_set_qpg_profile(node, "default_qpg", CVMX_PKI_FIND_AVAL_ENTRY, 1);
	qpg_cfg.port_add = 0;
	qpg_cfg.aura = aura;
	qpg_cfg.group_ok = 0;
	qpg_cfg.group_bad = 0;
	base_offset = cvmx_pki_set_qpg_config(node, "default_qpg", base_offset, base_offset, qpg_cfg);
	if (base_offset < 0) { /*vinita_to_do check if we can reuse the cluster group*/
		cvmx_dprintf("Error: qpg alloc in pki_init_data_struct\n");
		return -1;
	}

	/*initialize style */
	style_cfg.en_pkt_le_mode = CVMX_PKI_DISABLE;
	style_cfg.en_l2_lenchk = CVMX_PKI_ENABLE;
	style_cfg.l2_lenchk_mode = PKI_L2_LENCHK_EQUAL_ONLY;
	style_cfg.cluster_mask = 0xf;
	style_cfg.en_maxframe_errchk = CVMX_PKI_ENABLE;
	style_cfg.en_minframe_errchk = CVMX_PKI_ENABLE;
	style_cfg.max_min_frame_sel = 0;
	style_cfg.en_strip_l2_fcs = CVMX_PKI_ENABLE;
	style_cfg.en_fcs_check = CVMX_PKI_DISABLE;
	style_cfg.wqe_header_size = 5;
	style_cfg.wqe_start_offset = 0;
	style_cfg.first_mbuf_skip = 40;
	style_cfg.later_mbuf_skip = 0;
	style_cfg.mbuff_size = 2048;
	style_cfg.cache_mode = CVMX_PKI_OPC_MODE_STT;
	style_cfg.en_data_wqe_buf_diff = CVMX_PKI_DISABLE;
	style_cfg.wqe_vlan_vlptr = CVMX_PKI_USE_FIRST_VLAN;
	style_cfg.qpg_base_offset = base_offset;
	style_cfg.en_qpg_calc_port_addr = ~(CVMX_PKI_ENABLE) ;
	style_cfg.en_qpg_calc_aura = ~(CVMX_PKI_ENABLE);
	style_cfg.en_qpg_calc_group = ~(CVMX_PKI_ENABLE);
	style_cfg.qpg_qos = CVMX_PKI_QPG_QOS_NONE;
	style_cfg.qpg_port_msb = 0;
	style_cfg.qpg_port_shift = 0;
	style_cfg.tag_type = CVMX_SSO_TAG_TYPE_ORDERED;
	style_cfg.tag_fields.input_port = 1;

	style = cvmx_pki_set_style_config(node, "default_style", CVMX_PKI_FIND_AVAL_ENTRY, &style_cfg);
	if (style < 0) {
		cvmx_dprintf("ERROR: failed to set style config\n");
		return -1;
	}

	/*initialize style for no fcs strip ports*/
	style_cfg.en_strip_l2_fcs = CVMX_PKI_DISABLE;
	cvmx_pki_set_style_config(node, "no_fcs_strip", CVMX_PKI_FIND_AVAL_ENTRY, &style_cfg);

	/*initialize pki ports to use cluster group 0 and styles*/
	for (intf = 0; intf < CVMX_HELPER_MAX_IFACE; intf++) {
		for (index = 0; index < CVMX_HELPER_CFG_MAX_PORT_PER_IFACE; index++) {
			cvmx_pki_set_port_parse_mode_cfg(node, intf, index, CVMX_PKI_PARSE_LA_TO_LG);
			cvmx_pki_set_port_cluster_config(node, intf, index, cl_grp, CVMX_PKI_CLUSTER_ALL);
			cvmx_pki_set_port_style_cfg(node, intf, index, style);
		}
	}
	}
	return 0;

	/*vinita_to_do, init loop and npi interfaces to attach style no_fcs_strip*/
}
