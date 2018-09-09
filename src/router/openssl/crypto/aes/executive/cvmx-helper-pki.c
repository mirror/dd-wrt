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
 * PKI helper functions.
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pki-defs.h>
#include <asm/octeon/cvmx-pki.h>
#include <asm/octeon/cvmx-pow.h>
#include <asm/octeon/cvmx-pki-resources.h>
#include <asm/octeon/cvmx-helper-util.h>
#else
#include "cvmx.h"
#include "cvmx-version.h"
#include "cvmx-error.h"
#include "cvmx-pki-defs.h"
#include "cvmx-pki.h"
#include "cvmx-fpa.h"
#include "cvmx-helper-pki.h"
#include "cvmx-pki-resources.h"
#include "cvmx-pow.h"
#include "cvmx-helper-util.h"
#endif

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * This function sets up pools/auras to be used by PKI
 * @param node    node number
 */
int __cvmx_helper_pki_setup_fpa_pools(int node)
{
	int index;
	int pool;
	int aura;
	uint64_t buffer_count;
	uint64_t buffer_size;

	index = pki_profiles[node].pool_profile_list.index;
	while (index--) {
		pool = pki_profiles[node].pool_profile_list.pool_profile[index].pool_num;
		buffer_count = pki_profiles[node].pool_profile_list.pool_profile[index].buffer_count;
		buffer_size = pki_profiles[node].pool_profile_list.pool_profile[index].buffer_size;
		if (buffer_count == 0)
			continue;
		cvmx_fpa_pool_stack_init(node, pool, "PKI Pools", 0,
				buffer_count, FPA_NATURAL_ALIGNMENT,
				buffer_size);
	}
	index = pki_profiles[node].aura_profile_list.index;
	while (index--) {
		aura = pki_profiles[node].aura_profile_list.aura_profile[index].aura_num;
		pool = pki_profiles[node].aura_profile_list.aura_profile[index].pool_num;
		buffer_count = pki_profiles[node].aura_profile_list.aura_profile[index].buffer_count;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
		cvmx_fpa_assign_aura(node, aura, pool);
		cvmx_fpa_aura_init(node, aura, "PKI Aura", 0, NULL, buffer_count,0);
#endif
	}
	return 0;
}
#endif

/**
 * This function sets up pools/auras to be used by PKI
 * @param node    node number
 */
void __cvmx_helper_pki_setup_sso_groups(int node)
{
	int index;
	int grp;
	int priority;
	int weight;
	int affinity;
	uint64_t modify_mask;
	uint64_t core_mask;
	uint64_t core_mask_set;

	index = pki_profiles[node].sso_grp_profile_list.index;
	while (index--) {
		grp = pki_profiles[node].sso_grp_profile_list.grp_profile[index].grp_num;
		priority = pki_profiles[node].sso_grp_profile_list.grp_profile[index].priority;
		weight = pki_profiles[node].sso_grp_profile_list.grp_profile[index].weight;
		affinity = pki_profiles[node].sso_grp_profile_list.grp_profile[index].affinity;
		core_mask = pki_profiles[node].sso_grp_profile_list.grp_profile[index].core_affinity_mask;
		core_mask_set = pki_profiles[node].sso_grp_profile_list.grp_profile[index].core_affinity_mask_set;
		modify_mask = CVMX_SSO_MODIFY_GROUP_PRIORITY | CVMX_SSO_MODIFY_GROUP_WEIGHT |
				CVMX_SSO_MODIFY_GROUP_AFFINITY;
		cvmx_sso_set_group_priority(node, grp, priority, weight, affinity, modify_mask);
		cvmx_sso_set_group_core_affinity(node, grp, core_mask, core_mask_set);
	}
}

int __cvmx_helper_setup_pki_cluster_groups(int node)
{
	uint64_t cl_mask;
	uint64_t cl_group;
	int index;

	index = pki_profiles[node].cl_profile_list.index;
	while (index--) {
		cl_group = pki_profiles[node].cl_profile_list.cl_profile[index].cluster_group;
		cl_mask = pki_config[node].cluster_cfg[cl_group].cluster_mask;
		cvmx_pki_attach_cluster_to_group(node, cl_group, cl_mask);
	}
	return 0;
}

int __cvmx_helper_setup_pki_styles(int node)
{
	int cluster_mask;
	int index;
	int style;
	struct cvmx_pki_style_config style_cfg;

	memset(&style_cfg, 0, sizeof(style_cfg));
	index = pki_profiles[node].style_profile_list.index;
	while (index--) {
		style = pki_profiles[node].style_profile_list.style_profile[index].style_num;
		style_cfg = pki_config[node].style_cfg[style];
		cluster_mask = pki_config[node].style_cfg[style].cluster_mask;
		style_cfg.mbuff_size = cvmx_pki_get_mbuff_size(node,
				style_cfg.qpg_base_offset);
		if (style_cfg.mbuff_size == -1) {
			cvmx_dprintf("ERROR: didn't find base_offset in qpg_profile\n");
			return -1;
		}
	cvmx_pki_write_style(node, style, cluster_mask, style_cfg);
	}
	return 0;
}

int __cvmx_helper_setup_pki_qpg_table(int node)
{
	uint64_t num_entry;
	uint64_t index;
	uint64_t offset;

	index = pki_profiles[node].qpg_profile_list.index;

	while (index--) {
		num_entry = pki_profiles[node].qpg_profile_list.qpg_profile[index].num_entries;
		offset = pki_profiles[node].qpg_profile_list.qpg_profile[index].base_offset;
		while (num_entry--) {
			cvmx_pki_write_qpg_entry(node, offset, pki_config[node].qpg_cfg[offset].port_add,
					pki_config[node].qpg_cfg[offset].aura, pki_config[node].qpg_cfg[offset].group_ok,
					pki_config[node].qpg_cfg[offset].group_bad);
			offset++;
		}
	}
	return 0;
}

int __cvmx_helper_setup_pki_pcam_table(int node)
{
	uint64_t index;
	int bank;

	struct cvmx_pki_pcam_config *pcam_cfg;
	index = pki_profiles[node].pcam_list.index;


	while (index--) {
		pcam_cfg = &pki_profiles[node].pcam_list.pcam_cfg[index];
		bank = pcam_cfg->pcam_input.field % 2;
		pcam_cfg->entry_num = cvmx_pki_pcam_alloc_entry(node, pcam_cfg->entry_num, bank, pcam_cfg->cluster_mask);
		if (pcam_cfg->entry_num == -1) {
			cvmx_dprintf("ERROR: Allocating pcam entry\n");
			return -1;
		}
		cvmx_pki_pcam_write_entry(node, pcam_cfg->entry_num,
					  pcam_cfg->cluster_mask, pcam_cfg->pcam_input,
					  pcam_cfg->pcam_action);
	}
	return 0;
}

/**
 * This function installs the default VLAN entries to identify
 * the VLAN and set WQE[vv], WQE[vs] if VLAN is found. In 78XX
 * hardware (PKI) is not hardwired to recognize any 802.1Q VLAN
 * Ethertypes
 *
 * @param node    node number
 */
int __cvmx_helper_pki_install_default_vlan(int node)
{
	struct cvmx_pki_pcam_input pcam_input;
	struct cvmx_pki_pcam_action pcam_action;
	enum cvmx_pki_term field;
	int index;
	int bank;
	uint64_t cl_mask = CVMX_PKI_CLUSTER_ALL; /*vinita_to_do*/

	for (field = CVMX_PKI_PCAM_TERM_E_ETHTYPE0; field < CVMX_PKI_PCAM_TERM_E_ETHTYPE2; field++) {
		bank = field & 0x01;

		index = cvmx_pki_pcam_alloc_entry(node, CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			cvmx_dprintf("ERROR: Allocating pcam entry node=%d bank=%d", node, bank);
			return -1;
		}
		pcam_input.style = 0;
		pcam_input.style_mask = 0;
		pcam_input.field = field;
		pcam_input.field_mask = 0xfd;
		pcam_input.data = 0x81000000;
		pcam_input.data_mask = 0xffff0000;
		pcam_action.parse_mode_chg = CVMX_PKI_PARSE_NO_CHG;
		pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_VLAN;
		pcam_action.style_add = 0;
		pcam_action.pointer_advance = 4;
		cvmx_pki_pcam_write_entry(node, index, cl_mask, pcam_input, pcam_action);/*vinita_to_do, cluster_mask*/

		index = cvmx_pki_pcam_alloc_entry(node, CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			cvmx_dprintf("ERROR: Allocating pcam entry node=%d bank=%d", node, bank);
			return -1;
		}
		pcam_input.data = 0x88a80000;
		cvmx_pki_pcam_write_entry(node, index, cl_mask, pcam_input, pcam_action);/*vinita_to_do, cluster_mask*/

		index = cvmx_pki_pcam_alloc_entry(node, CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			cvmx_dprintf("ERROR: Allocating pcam entry node=%d bank=%d", node, bank);
			return -1;
		}
		pcam_input.data = 0x92000000;
		cvmx_pki_pcam_write_entry(node, index, cl_mask, pcam_input, pcam_action);/*vinita_to_do, cluster_mask*/

		index = cvmx_pki_pcam_alloc_entry(node, CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			cvmx_dprintf("ERROR: Allocating pcam entry node=%d bank=%d", node, bank);
			return -1;
		}
		pcam_input.data = 0x91000000;
		cvmx_pki_pcam_write_entry(node, index, cl_mask, pcam_input, pcam_action);/*vinita_to_do, cluster_mask*/
	}
	return 0;
}


int __cvmx_helper_global_setup_pki(int node)
{
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* Setup the packet pools*/
	__cvmx_helper_pki_setup_fpa_pools(node);
#endif
	/*vinita_to_do, if defined linux,init structures since there is no
	config file.*/
	/*set up clusters*/
	__cvmx_helper_setup_pki_cluster_groups(node);
	cvmx_pki_setup_clusters(node);
	__cvmx_helper_pki_setup_sso_groups(node);
	__cvmx_helper_setup_pki_qpg_table(node);
	__cvmx_helper_setup_pki_styles(node);
	__cvmx_helper_setup_pki_pcam_table(node);
	cvmx_pki_enable_backpressure(node);
	return 0;
#if 0
	/*Vinita_to_do setup global config after parsing here*/
	cvmx_pki_setup_clusters(node);
	cvmx_pki_enable_backpressure(node);
	return 0;
#endif
}

int cvmx_helper_pki_get_num_qos_entry(enum cvmx_pki_qpg_qos qpg_qos)
{
	if (qpg_qos == CVMX_PKI_QPG_QOS_NONE)
		return 1;
	else if (qpg_qos == CVMX_PKI_QPG_QOS_VLAN || qpg_qos == CVMX_PKI_QPG_QOS_DSA_SRC
		       || qpg_qos == CVMX_PKI_QPG_QOS_MPLS)
		return 8;
	else if (qpg_qos == CVMX_PKI_QPG_QOS_HIGIG) /*vinita_to_do for higig2*/
		return 32;
	else if (qpg_qos == CVMX_PKI_QPG_QOS_DIFFSERV)
		return 64;
	else {
		cvmx_dprintf("ERROR: unrecognized qpg_qos = %d", qpg_qos);
		return 0;
	}
}

int __cvmx_helper_port_setup_pki(int node, int ipd_port)
{
	uint64_t initial_style;
	int cluster_group;
	enum cvmx_pki_pkind_parse_mode initial_parse_mode;
	int interface, index, pknd;

	interface = cvmx_helper_get_interface_num(ipd_port);
	index = cvmx_helper_get_interface_index_num(ipd_port);
	pknd = cvmx_helper_get_pknd(interface, index);

	initial_style = pki_config[node].port_cfg[interface][index].initial_style;
	cluster_group = pki_config[node].port_cfg[interface][index].cluster_grp;
	initial_parse_mode = CVMX_PKI_PARSE_LA_TO_LG;/*vinita_to_do*/
	cvmx_pki_write_pkind(node, pknd, cluster_group,
			     initial_parse_mode, initial_style);
	return 0;
#if 0
	int initial_style;
	int cl_group, num_cl;
	uint64_t cl_mask;
	uint64_t num_entry;
	int num_ports;
	uint64_t port_shift;
	int offset;
	enum cvmx_pki_qpg_qos qpg_qos;
	struct cvmx_pki_style_config style_cfg;
	enum cvmx_pki_pkind_parse_mode initial_parse_mode;
	int grp_ok, grp_bad;
	int priority, weight, affinity;
	int pool, aura;
	uint64_t buffer_count, buffer_size;
	uint64_t modify_mask;
	uint64_t core_mask;
	uint64_t core_mask_set;

	/*parameters used by this pkind*/
	cl_group = pki_config[node].pkind_cfg[pknd].cluster_grp;
	initial_style = pki_config[node].pkind_cfg[pknd].initial_style;
	initial_parse_mode = pki_config[node].pkind_cfg[pknd].parsing_mode;

	if (!pki_config[node].cluster_cfg[cl_group].users) {
		cl_group = cvmx_pki_alloc_cluster_group(node, cl_group);
		if (cl_group == -1) {
			cvmx_dprintf("ERROR:allocating cluster_group\n");
			return -1;
		}
		cl_mask = pki_config[node].cluster_cfg[cl_group].cluster_mask;
		num_cl = pki_config[node].pkind_cfg[pknd].num_clusters;
		if (cvmx_pki_alloc_clusters(node, num_cl, &cl_mask) == -1) {
			cvmx_dprintf("ERROR:allocating clusters\n");
			return -1;
		}
	}
	cvmx_pki_attach_cluster_to_group(node, cl_group, cl_mask);
	pki_config[node].cluster_cfg[cl_group].users++;

	/*setup pool, aura and sso_group used by this pkind, they are configured via qpg table*/
	/*find offset in qpg table for this pkind and total number of entries*/
	qpg_qos = pki_config[node].style_cfg[initial_style].qpg_qos;
	num_entry = cvmx_helper_pki_get_num_qos_entry(qpg_qos);
	num_ports = pki_config[node].style_cfg[initial_style].qpg_port_msb;
	port_shift = pki_config[node].style_cfg[initial_style].qpg_port_shift;
	/*check to make sure that enough qpg entries are allocated*/
	if (num_ports) {
		if (!(port_shift >= __builtin_ffsll(num_entry))) {
			cvmx_dprintf("WARNING! qpg_port_shift incorrect in style %d, making amend\n",
				     initial_style);
			port_shift = __builtin_ffsll(num_entry);
		}
	}
	num_entry *= (uint64_t)(num_ports << port_shift);
	offset = pki_config[node].style_cfg[initial_style].qpg_base_offset;
	/*now check every entry of pools, aura and group used by them
	and initialize pool aura and group*/
	while (num_entry--) {
		/*setup pool used by this entry*/
		aura = pki_config[node].qpg_cfg[offset].aura;
		pool = pki_config[node].aura_cfg[aura].pool_num;
		if (!pki_config[node].pool_cfg[pool].users) {
			if (cvmx_fpa_allocate_fpa_pools(node, &pool, 1) == -1) {
				cvmx_dprintf("ERROR:allocating pool for pool_config\n");
				return -1;
			}
		}
		buffer_count = pki_config[node].pool_cfg[pool].buffer_count;
		buffer_size = pki_config[node].pool_cfg[pool].buffer_size;
		cvmx_fpa_pool_stack_init(node, pool, "PKI Pools", 0,
				buffer_count, FPA_NATURAL_ALIGNMENT,
				buffer_size);
		pki_config[node].pool_cfg[pool].users++;

		/*setup aura used by this entry*/
		if (!pki_config[node].aura_cfg[pool].users) {
			if (cvmx_fpa_allocate_auras(node, &aura, 1) == -1) {
				cvmx_dprintf("ERROR:allocating aura\n");
				return -1;
			}
		}
		buffer_count = pki_config[node].aura_cfg[aura].buffer_count;
		cvmx_fpa_assign_aura(node, aura, pool);
		cvmx_fpa_aura_init(node, aura, "PKI Aura", 0, buffer_count, 0);
		pki_config[node].aura_cfg[pool].users++;

		/*setup sso group used by this entry*/
		grp_ok = pki_config[node].qpg_cfg[offset].group_ok;
		#if 0 /*vinita_to_do uncomment when group_alloc is ready*/
		group = cvmx_pki_allocate_group(node, group);
		if (group == -1) {
			cvmx_dprintf("ERROR:allocating group for group_config\n");
			return -1;
		}
		if (!pki_config[node].sso_grp_cfg[pool].users) {
			priority = pki_config[node].sso_cfg[grp].priority;
			weight = pki_config[node].sso_cfg[grp].weight;
			affinity = pki_config[node].sso_cfg[grp].affinity;
			core_mask = pki_profiles[node].sso_cfg[grp].core_affinity_mask;
			core_mask_set = pki_config[node].sso_cfg[grp].core_affinity_mask_set;
			modify_mask = CVMX_SSO_MODIFY_GROUP_PRIORITY | CVMX_SSO_MODIFY_GROUP_WEIGHT |
			CVMX_SSO_MODIFY_GROUP_AFFINITY;
			cvmx_sso_set_group_priority(node, grp, priority, weight, affinity, modify_mask);
			cvmx_sso_set_group_core_affinity(node, grp, core_mask, core_mask_set);
		}
		#endif
		/*vinita_to_do, grp_bad*/
		grp_bad = pki_config[node].qpg_cfg[offset].group_bad;

		/*setup qpg table used by this pkind*/
		if (!pki_config[node].qpg_cfg[offset].users) {
			offset = cvmx_pki_alloc_qpg_entry(node, offset, 1);
			if (offset == -1) {
				cvmx_dprintf("ERROR:allocating entry for qpg_table\n");
				return -1;
			}
		}
		cvmx_pki_write_qpg_entry(node, offset, pki_config[node].qpg_cfg[offset].port_add,
					 aura, grp_ok, grp_bad);
		pki_config[node].qpg_cfg[offset].users++;
		offset++;
	}

	/*setup style used by this pkind*/
	if (!pki_config[node].style_cfg[initial_style].users) {
		initial_style = cvmx_pki_alloc_style(node, initial_style);
		if (initial_style == -1) {
			cvmx_dprintf("ERROR:allocating style for style_config\n");
			return -1;
		}
		style_cfg = pki_config[node].style_cfg[initial_style];
		cl_mask = pki_config[node].style_cfg[initial_style].cluster_mask;
		style_cfg.mbuff_size = cvmx_pki_get_mbuff_size(node, style_cfg.qpg_base_offset);
		if (style_cfg.mbuff_size == -1) {
			cvmx_dprintf("ERROR: didn't find base_offset in qpg_cfg\n");
			return -1;
		}
	}
	cvmx_pki_write_style(node, initial_style, cl_mask, style_cfg);
	pki_config[node].style_cfg[initial_style].users++;

	/* Now finally setup the pkind to use the initial style configured in step above
	and initial parse mode*/
	cvmx_pki_write_pkind(node, pknd, cl_group,
			     initial_parse_mode, initial_style);
	return 0;
#endif
}

#if 0
int cvmx_helper_setup_pki_port_oldcfg(int node, int pknd)
{
	int initial_style, cluster_group, cluster_mask;
	enum cvmx_pki_pkind_parse_mode initial_parse_mode;
	int padd = 0;
	struct cvmx_pki_style_config style_cfg;

	memset(&style_cfg, 0, sizeof(style_cfg));
	initial_style = pknd;/*vinita_to_do ask hw perf*/
	cluster_group = 0x0;
	cluster_mask = 0xf;

	initial_style = cvmx_pki_alloc_style(node, -1);
	if (initial_style == -1) {
		cvmx_dprintf("ERROR: allocating style %d\n", initial_style);
		return -1;
	}
	/*style_cfg.en_L2_lencheck = 1;*/
	/*style_cfg.l2_lenchk_mode = PKI_L2_LENCHK_EQUAL_GREATER;*/
	style_cfg.cache_mode = cvmx_ipd_cfg.cache_mode;
	style_cfg.first_mbuf_skip = cvmx_ipd_cfg.first_mbuf_skip;
	style_cfg.later_mbuf_skip = cvmx_ipd_cfg.not_first_mbuf_skip;
	style_cfg.mbuff_size = cvmx_ipd_cfg.packet_pool.buffer_size;
	style_cfg.tag_type = cvmx_ipd_cfg.port_config.tag_type;
	style_cfg.tag_fields.layer_c_src = cvmx_ipd_cfg.port_config.tag_fields.ipv6_src_ip |
			cvmx_ipd_cfg.port_config.tag_fields.ipv4_src_ip;
	style_cfg.tag_fields.layer_c_dst = cvmx_ipd_cfg.port_config.tag_fields.ipv6_dst_ip |
			cvmx_ipd_cfg.port_config.tag_fields.ipv4_dst_ip;
	style_cfg.tag_fields.input_port = cvmx_ipd_cfg.port_config.tag_fields.input_port;

#if 0
	/* Default BPID to use for packets on this port-kind */
	prt_cfgbx.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGBX(pknd));
	prt_cfgbx.s.bpid = pknd;
	cvmx_write_csr(CVMX_PIP_PRT_CFGBX(pknd), prt_cfgbx.u64);
#endif
	style_cfg.qpg_base_offset = pknd;

	cvmx_pki_write_qpg_entry(node, style_cfg.qpg_base_offset, padd, cvmx_ipd_cfg.packet_pool.pool_num,
				 pknd & 0x7, pknd & 0x7);

	cvmx_pki_write_style(node, initial_style, cluster_mask, style_cfg);

	initial_parse_mode = CVMX_PKI_PARSE_LA_TO_LG;/*vinita_to_do*/

	cvmx_pki_write_pkind(node, pknd, cluster_group,
			     initial_parse_mode, initial_style);
	return 0;
}
#endif

int cvmx_helper_pki_setup_qpg_table(int node, int num_entries, int port_addr[],
				    uint64_t aura[], uint64_t sso_grp_ok[], uint64_t sso_grp_bad[])
{
	int base_offset;
	int entry;

	base_offset = cvmx_pki_alloc_qpg_entry(node, CVMX_PKI_FIND_AVAL_ENTRY, num_entries);
	if (base_offset == -1) {
		cvmx_dprintf("ERROR:setup_qpg_table: entries not available in qpg table\n");
		return -1;
	}
	for (entry = 0; entry < num_entries; entry++, base_offset++) {
		cvmx_pki_write_qpg_entry(node, base_offset, port_addr[entry], aura[entry],
					 sso_grp_ok[entry], sso_grp_bad[entry]);
	}
	return base_offset - num_entries;
}

void cvmx_helper_pki_set_fcs_op(int node, int interface, int nports, int has_fcs)
{
	int index;
	int pknd;
	int style;
	int cluster = 0;
	cvmx_pki_clx_pkindx_style_t pkind_cfg_style;
	cvmx_pki_clx_stylex_cfg_t style_cfg_reg;

	for (index = 0; index < nports; index++) {
		pknd = cvmx_helper_get_pknd(interface, index);
		/*vinita_to_do; find the cluster in use*/
		pkind_cfg_style.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pknd, cluster));
		style = pkind_cfg_style.s.style;
		style_cfg_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
		style_cfg_reg.s.fcs_strip = has_fcs;
		cvmx_write_csr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster), style_cfg_reg.u64);
	}
}
