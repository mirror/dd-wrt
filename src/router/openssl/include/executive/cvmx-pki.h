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
 * Interface to the hardware Packet Input Data unit.
 */

#ifndef __CVMX_PKI_H__
#define __CVMX_PKI_H__

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pki-defs.h>
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#else
#include "cvmx-fpa.h"
#include "cvmx-helper-util.h"
#include "cvmx-helper-cfg.h"
#endif

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#define CVMX_PKI_NUM_CHANNEL		(4096)
#define CVMX_PKI_NUM_AURA       	(1024)
#define CVMX_PKI_NUM_BPID 	      	(1024)
#define CVMX_PKI_NUM_PKIND		(64)
#define CVMX_PKI_NUM_INTERNAL_STYLES    (256)
#define CVMX_PKI_NUM_FINAL_STYLES	(64)
#define CVMX_PKI_NUM_QPG_ENTRY		(2048)
#define CVMX_PKI_NUM_FRAME_CHECK_VALUES	(2)
#define CVMX_PKI_NUM_LTYPES		(32)
#define CVMX_PKI_NUM_CLUSTERS		(4)
#define CVMX_PKI_NUM_CLUSTER_GROUP      (4)
#define CVMX_PKI_NUM_PCAM_BANK		(2)
#define CVMX_PKI_NUM_PCAM_ENTRY		(192)
#define CVMX_PKI_NUM_QPG_STYLE_INDEX	(8)
#define CVMX_PKI_NUM_FRAME_SIZE_ID	(2)
#define CVMX_PKI_NUM_CHANNELS		(4096)
#define CVMX_PKI_NUM_BPID		(1024)
#define CVMX_PKI_NUM_SSO_GROUP		(256)
#define CVMX_PKI_MAX_FRAME_SIZE		(65535)
#define CVMX_PKI_FIND_AVAL_ENTRY        (-1)
#define CVMX_PKI_MAX_CLUSTER_PROFILES   (4)
#define CVMX_PKI_MAX_STYLE_PROFILES	(256)
#define CVMX_PKI_MAX_NAME		(16)
#define CVMX_PKI_MAX_POOL_PROFILES	(64) /*modify it later*/
#define CVMX_PKI_MAX_AURA_PROFILES	(256) /*modify it later*/


#ifdef CVMX_SUPPORT_SEPARATE_CLUSTER_CONFIG
#define CVMX_PKI_TOTAL_PCAM_ENTRY	((CVMX_PKI_NUM_CLUSTERS) * (CVMX_PKI_NUM_PCAM_BANK) *\
						(CVMX_PKI_NUM_PCAM_ENTRY))
#else
#define CVMX_PKI_TOTAL_PCAM_ENTRY	(CVMX_PKI_NUM_PCAM_BANK * CVMX_PKI_NUM_PCAM_ENTRY)
#endif

#define CVMX_PKI_MAX_QPG_PROFILES	(2048)
#define CVMX_PKI_NOT_ASSIGNED		(-88)


struct cvmx_pki_cluster_profile {
	char		name[CVMX_PKI_MAX_NAME];
	int 		num_clusters;
	int 		cluster_group;
#define CVMX_PKI_PARSE_DSP		0x01
#define CVMX_PKI_PARSE_FULCRUM		0x02
#define CVMX_PKI_PARSE_MPLS		0x04
#define CVMX_PKI_PARSE_L3		0x08
#define CVMX_PKI_PARSE_IL3		0x10
#define CVMX_PKI_PARSE_L4 		0x20
#define CVMX_PKI_PARSE_CUSTOM_L2	0x40
#define CVMX_PKI_PARSE_CUSTOM_LG	0x80
#define CVMX_PKI_PARSE_VIRTUALIZATION	0x100
#define CVMX_PKI_CLUSTER_ALL            0xf
	uint64_t        parsing_mask;

};

struct cvmx_pki_cluster_list {
	int index;
	struct cvmx_pki_cluster_profile cl_profile[CVMX_PKI_MAX_CLUSTER_PROFILES];
};

struct cvmx_pki_fpa_pool_config {
	int users;
	int pool_num;
	uint64_t buffer_size;
	uint64_t buffer_count;
};

struct cvmx_pki_pool_profile {
	char pool_name[CVMX_PKI_MAX_NAME];
	int64_t pool_num;
	uint64_t buffer_size;
	uint64_t buffer_count;
};

struct cvmx_pki_pool_list {
	int index;
	struct cvmx_pki_pool_profile pool_profile[CVMX_PKI_MAX_POOL_PROFILES];
};

struct cvmx_pki_aura_config {
	int users;
	int pool_num;
	int buffer_count;
};

struct cvmx_pki_aura_profile {
	char aura_name[CVMX_PKI_MAX_NAME];
	int aura_num;
	int pool_num;
	int buffer_count;
};

struct cvmx_pki_aura_list {
	int index;
	struct cvmx_pki_aura_profile aura_profile[CVMX_PKI_MAX_AURA_PROFILES];
};

struct cvmx_pki_sso_grp_config {
	int users;
	int priority;
	int weight;
	int affinity;
	uint64_t core_affinity_mask;
	uint64_t core_affinity_mask_set;
};

struct cvmx_pki_sso_grp_profile {
	char grp_name[CVMX_PKI_MAX_NAME];
	int grp_num;
	int priority;
	int weight;
	int affinity;
	uint64_t core_affinity_mask;
	uint64_t core_affinity_mask_set;
};

struct cvmx_pki_sso_grp_list {
	int index;
	struct cvmx_pki_sso_grp_profile grp_profile[CVMX_PKI_NUM_SSO_GROUP];
};

struct cvmx_pki_qpg_profile {
	char qpg_name[CVMX_PKI_MAX_NAME];
	int base_offset;
	int num_entries;
};

struct cvmx_pki_qpg_list {
	int index;
	struct cvmx_pki_qpg_profile qpg_profile[CVMX_PKI_MAX_QPG_PROFILES];
};

struct cvmx_pki_style_profile {
	char				name[CVMX_PKI_MAX_NAME];
	int				style_num;
};

struct cvmx_pki_style_list {
	int    index;
	struct cvmx_pki_style_profile style_profile[CVMX_PKI_MAX_STYLE_PROFILES];
};

struct cvmx_pki_framelen_chk {
	uint16_t	maxlen;
	uint16_t	minlen;
};

struct cvmx_pki_global_config {
	uint64_t			parsing_mask;
	uint64_t			clusters_in_use_mask;
	struct cvmx_pki_framelen_chk    frame_len_chk[CVMX_PKI_NUM_FRAME_SIZE_ID];
	/*enum cvmx_pki_bel		bel_type_map[CVMX_PKI_MAX_LTYPE];*/
};

struct cvmx_pki_qpg_config {
	int  users;
	int  port_add;
	int  aura;
	int  group_ok;
	int  group_bad;
};

struct cvmx_pki_clustergrp_config {
	int		users;
	uint64_t	cluster_mask;
};

enum cvmx_pki_pkind_parse_mode {
	CVMX_PKI_PARSE_LA_TO_LG = 0,
	CVMX_PKI_PARSE_LB_TO_LG = 1,
	CVMX_PKI_PARSE_LC_TO_LG = 3,
	CVMX_PKI_PARSE_LG = 0x3f,
	CVMX_PKI_PARSE_NOTHING = 0x7f,
};

enum cvmx_pki_parse_mode_chg {
	CVMX_PKI_PARSE_NO_CHG = 0x0,
	CVMX_PKI_PARSE_SKIP_TO_LB = 0x1,
	CVMX_PKI_PARSE_SKIP_TO_LC = 0x3,
	CVMX_PKI_PARSE_SKIP_TO_LD = 0x7,
	CVMX_PKI_PARSE_SKIP_TO_LG = 0x3f,
	CVMX_PKI_PARSE_SKIP_ALL = 0x7f,
};

struct cvmx_pki_port_config {
	int				users;
	enum cvmx_pki_pkind_parse_mode	parsing_mode;
	uint64_t 			cluster_mask;
	uint64_t 			l2_parsing_mask;/*vinita_to_do*/
	int	 			initial_style;
	int				cluster_grp;
	int 				num_clusters;
};

struct cvmx_pki_tag_fields {
	uint64_t layer_G_src:1;
	uint64_t layer_F_src:1;
	uint64_t layer_E_src:1;
	uint64_t layer_D_src:1;
	uint64_t layer_C_src:1;
	uint64_t layer_B_src:1;
	uint64_t layer_G_dst:1;
	uint64_t layer_F_dst:1;
	uint64_t layer_E_dst:1;
	uint64_t layer_D_dst:1;
	uint64_t layer_C_dst:1;
	uint64_t layer_B_dst:1;
	uint64_t input_port:1;
	uint64_t mpls_label:1;
	uint64_t first_vlan:1;
	uint64_t second_vlan:1;
	uint64_t ip_prot_nexthdr:1;
	uint64_t tag_sync:1;
	uint64_t tag_spi:1;
	uint64_t tag_gtp:1;
	uint64_t tag_vni:1;
};

enum cvmx_pki_l2_len_mode {
	PKI_L2_LENCHK_EQUAL_GREATER = 0,
	PKI_L2_LENCHK_EQUAL_ONLY
};

enum cvmx_pki_cache_mode {
	CVMX_PKI_OPC_MODE_STT = 0LL,	/* All blocks write through DRAM,*/
	CVMX_PKI_OPC_MODE_STF = 1LL,	/* All blocks into L2 */
	CVMX_PKI_OPC_MODE_STF1_STT = 2LL,	/* 1st block L2, rest DRAM */
	CVMX_PKI_OPC_MODE_STF2_STT = 3LL	/* 1st, 2nd blocks L2, rest DRAM */
};

/**
 * Tag type definitions
 */
enum cvmx_sso_tag_type {
	CVMX_SSO_TAG_TYPE_ORDERED = 0L,	/**< Tag ordering is maintained */
	CVMX_SSO_TAG_TYPE_ATOMIC = 1L,	/**< Tag ordering is maintained, and at most one PP has the tag */
	CVMX_SSO_TAG_TYPE_UNTAGGED = 2L,	/**< The work queue entry from the order
	- NEVER tag switch from NULL to NULL */
	CVMX_SSO_TAG_TYPE_EMPTY = 3L/**< A tag switch to NULL, and there is no space reserved in POW
			- NEVER tag switch to NULL_NULL
			- NEVER tag switch from NULL_NULL
			- NULL_NULL is entered at the beginning of time and on a deschedule.
			- NULL_NULL can be exited by a new work request. A NULL_SWITCH load can also switch the state to NULL */
};

enum cvmx_pki_qpg_qos {
	CVMX_PKI_QPG_QOS_NONE = 0,
	CVMX_PKI_QPG_QOS_VLAN,
	CVMX_PKI_QPG_QOS_MPLS,
	CVMX_PKI_QPG_QOS_DSA_SRC,
	CVMX_PKI_QPG_QOS_DIFFSERV,
	CVMX_PKI_QPG_QOS_HIGIG,
};

enum cvmx_pki_en_dis {
	CVMX_PKI_DISABLE = 0,
	CVMX_PKI_ENABLE = 1,
};

enum cvmx_pki_wqe_vlan {
	CVMX_PKI_USE_FIRST_VLAN = 0,
	CVMX_PKI_USE_SECOND_VLAN
};

struct cvmx_pki_style_config {
	int				users;
	enum cvmx_pki_en_dis 		en_pkt_le_mode;
	enum cvmx_pki_en_dis		en_l2_lenchk;
	uint64_t			cluster_mask;
	enum cvmx_pki_l2_len_mode	l2_lenchk_mode;
	enum cvmx_pki_en_dis		en_maxframe_errchk;
	enum cvmx_pki_en_dis		en_minframe_errchk;
	int	 			max_min_frame_sel;
	enum cvmx_pki_en_dis		en_strip_l2_fcs;
	enum cvmx_pki_en_dis		en_fcs_check;
	int	 			wqe_header_size;
	int 				wqe_start_offset;
	int 				first_mbuf_skip;
	int	 			later_mbuf_skip;
	int				mbuff_size;
	enum cvmx_pki_cache_mode 	cache_mode;
	enum cvmx_pki_en_dis 		en_data_wqe_buf_diff;
	enum cvmx_pki_wqe_vlan		wqe_vlan_vlptr;
	int				qpg_base_offset;
	enum cvmx_pki_en_dis 		en_qpg_calc_port_addr;
	enum cvmx_pki_en_dis 		en_qpg_calc_aura;
	enum cvmx_pki_en_dis 		en_qpg_calc_group;
	enum cvmx_pki_qpg_qos		qpg_qos;
	int				qpg_port_msb;
	int				qpg_port_shift;
	enum cvmx_sso_tag_type	 	tag_type;
	struct cvmx_pki_tag_fields 	tag_fields;
};

#define CVMX_PKI_PCAM_TERM_E_NONE_M                            (0x0)
#define CVMX_PKI_PCAM_TERM_E_L2_CUSTOM_M                       (0x2)
#define CVMX_PKI_PCAM_TERM_E_HIGIG_M                           (0x5)
#define CVMX_PKI_PCAM_TERM_E_DMACH_M                           (0xA)
#define CVMX_PKI_PCAM_TERM_E_DMACL_M                           (0xB)
#define CVMX_PKI_PCAM_TERM_E_GLORT_M                           (0x12)
#define CVMX_PKI_PCAM_TERM_E_DSA_M                             (0x13)
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE0_M                        (0x18)
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE1_M                        (0x19)
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE2_M                        (0x1A)
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE3_M                        (0x1B)
#define CVMX_PKI_PCAM_TERM_E_MPLS0_M                           (0x1E)
#define CVMX_PKI_PCAM_TERM_E_L3_FLAGS_M                        (0x23)
#define CVMX_PKI_PCAM_TERM_E_LD_VNI_M                          (0x28)
#define CVMX_PKI_PCAM_TERM_E_IL3_FLAGS_M                       (0x2B)
#define CVMX_PKI_PCAM_TERM_E_L4_PORT_M                         (0x30)
#define CVMX_PKI_PCAM_TERM_E_LG_CUSTOM_M                       (0x39)

enum cvmx_pki_term {
	CVMX_PKI_PCAM_TERM_E_NONE                    = CVMX_PKI_PCAM_TERM_E_NONE_M,
	CVMX_PKI_PCAM_TERM_E_L2_CUSTOM               = CVMX_PKI_PCAM_TERM_E_L2_CUSTOM_M,
	CVMX_PKI_PCAM_TERM_E_HIGIG                   = CVMX_PKI_PCAM_TERM_E_HIGIG_M,
	CVMX_PKI_PCAM_TERM_E_DMACH                   = CVMX_PKI_PCAM_TERM_E_DMACH_M,
	CVMX_PKI_PCAM_TERM_E_DMACL                   = CVMX_PKI_PCAM_TERM_E_DMACL_M,
	CVMX_PKI_PCAM_TERM_E_GLORT                   = CVMX_PKI_PCAM_TERM_E_GLORT_M,
	CVMX_PKI_PCAM_TERM_E_DSA                     = CVMX_PKI_PCAM_TERM_E_DSA_M,
	CVMX_PKI_PCAM_TERM_E_ETHTYPE0                = CVMX_PKI_PCAM_TERM_E_ETHTYPE0_M,
	CVMX_PKI_PCAM_TERM_E_ETHTYPE1                = CVMX_PKI_PCAM_TERM_E_ETHTYPE1_M,
	CVMX_PKI_PCAM_TERM_E_ETHTYPE2                = CVMX_PKI_PCAM_TERM_E_ETHTYPE2_M,
	CVMX_PKI_PCAM_TERM_E_ETHTYPE3                = CVMX_PKI_PCAM_TERM_E_ETHTYPE3_M,
	CVMX_PKI_PCAM_TERM_E_MPLS0                   = CVMX_PKI_PCAM_TERM_E_MPLS0_M,
	CVMX_PKI_PCAM_TERM_E_L3_FLAGS                = CVMX_PKI_PCAM_TERM_E_L3_FLAGS_M,
	CVMX_PKI_PCAM_TERM_E_LD_VNI                  = CVMX_PKI_PCAM_TERM_E_LD_VNI_M,
	CVMX_PKI_PCAM_TERM_E_IL3_FLAGS               = CVMX_PKI_PCAM_TERM_E_IL3_FLAGS_M,
	CVMX_PKI_PCAM_TERM_E_L4_PORT                 = CVMX_PKI_PCAM_TERM_E_L4_PORT_M,
	CVMX_PKI_PCAM_TERM_E_LG_CUSTOM               = CVMX_PKI_PCAM_TERM_E_LG_CUSTOM_M
};

struct cvmx_pki_pcam_input {
	uint64_t 		style;
	uint64_t		style_mask;
	enum cvmx_pki_term	field;
	uint32_t 		field_mask;
	uint64_t 		data;
	uint64_t 		data_mask;
};

#define CVMX_PKI_LTYPE_E_NONE_M                                (0x0)
#define CVMX_PKI_LTYPE_E_ENET_M                                (0x1)
#define CVMX_PKI_LTYPE_E_VLAN_M                                (0x2)
#define CVMX_PKI_LTYPE_E_SNAP_PAYLD_M                          (0x5)
#define CVMX_PKI_LTYPE_E_ARP_M                                 (0x6)
#define CVMX_PKI_LTYPE_E_RARP_M                                (0x7)
#define CVMX_PKI_LTYPE_E_IP4_M                                 (0x8)
#define CVMX_PKI_LTYPE_E_IP4_OPT_M                             (0x9)
#define CVMX_PKI_LTYPE_E_IP6_M                                 (0xA)
#define CVMX_PKI_LTYPE_E_IP6_OPT_M                             (0xB)
#define CVMX_PKI_LTYPE_E_IPSEC_ESP_M                           (0xC)
#define CVMX_PKI_LTYPE_E_IPFRAG_M                              (0xD)
#define CVMX_PKI_LTYPE_E_IPCOMP_M                              (0xE)
#define CVMX_PKI_LTYPE_E_TCP_M                                 (0x10)
#define CVMX_PKI_LTYPE_E_UDP_M                                 (0x11)
#define CVMX_PKI_LTYPE_E_SCTP_M                                (0x12)
#define CVMX_PKI_LTYPE_E_UDP_VXLAN_M                           (0x13)
#define CVMX_PKI_LTYPE_E_GRE_M                                 (0x14)
#define CVMX_PKI_LTYPE_E_NVGRE_M                               (0x15)
#define CVMX_PKI_LTYPE_E_GTP_M                                 (0x16)
#define CVMX_PKI_LTYPE_E_SW28_M                                (0x1C)
#define CVMX_PKI_LTYPE_E_SW29_M                                (0x1D)
#define CVMX_PKI_LTYPE_E_SW30_M                                (0x1E)
#define CVMX_PKI_LTYPE_E_SW31_M                                (0x1F)

enum cvmx_pki_layer_type {
	CVMX_PKI_LTYPE_E_NONE                        = CVMX_PKI_LTYPE_E_NONE_M,
	CVMX_PKI_LTYPE_E_ENET                        = CVMX_PKI_LTYPE_E_ENET_M,
	CVMX_PKI_LTYPE_E_VLAN                        = CVMX_PKI_LTYPE_E_VLAN_M,
	CVMX_PKI_LTYPE_E_SNAP_PAYLD                  = CVMX_PKI_LTYPE_E_SNAP_PAYLD_M,
	CVMX_PKI_LTYPE_E_ARP                         = CVMX_PKI_LTYPE_E_ARP_M,
	CVMX_PKI_LTYPE_E_RARP                        = CVMX_PKI_LTYPE_E_RARP_M,
	CVMX_PKI_LTYPE_E_IP4                         = CVMX_PKI_LTYPE_E_IP4_M,
	CVMX_PKI_LTYPE_E_IP4_OPT                     = CVMX_PKI_LTYPE_E_IP4_OPT_M,
	CVMX_PKI_LTYPE_E_IP6                         = CVMX_PKI_LTYPE_E_IP6_M,
	CVMX_PKI_LTYPE_E_IP6_OPT                     = CVMX_PKI_LTYPE_E_IP6_OPT_M,
	CVMX_PKI_LTYPE_E_IPSEC_ESP                   = CVMX_PKI_LTYPE_E_IPSEC_ESP_M,
	CVMX_PKI_LTYPE_E_IPFRAG                      = CVMX_PKI_LTYPE_E_IPFRAG_M,
	CVMX_PKI_LTYPE_E_IPCOMP                      = CVMX_PKI_LTYPE_E_IPCOMP_M,
	CVMX_PKI_LTYPE_E_TCP                         = CVMX_PKI_LTYPE_E_TCP_M,
	CVMX_PKI_LTYPE_E_UDP                         = CVMX_PKI_LTYPE_E_UDP_M,
	CVMX_PKI_LTYPE_E_SCTP                        = CVMX_PKI_LTYPE_E_SCTP_M,
	CVMX_PKI_LTYPE_E_UDP_VXLAN                   = CVMX_PKI_LTYPE_E_UDP_VXLAN_M,
	CVMX_PKI_LTYPE_E_GRE                         = CVMX_PKI_LTYPE_E_GRE_M,
	CVMX_PKI_LTYPE_E_NVGRE                       = CVMX_PKI_LTYPE_E_NVGRE_M,
	CVMX_PKI_LTYPE_E_GTP                         = CVMX_PKI_LTYPE_E_GTP_M,
	CVMX_PKI_LTYPE_E_SW28                        = CVMX_PKI_LTYPE_E_SW28_M,
	CVMX_PKI_LTYPE_E_SW29                        = CVMX_PKI_LTYPE_E_SW29_M,
	CVMX_PKI_LTYPE_E_SW30                        = CVMX_PKI_LTYPE_E_SW30_M,
	CVMX_PKI_LTYPE_E_SW31                        = CVMX_PKI_LTYPE_E_SW31_M
};


struct cvmx_pki_pcam_action {
	enum cvmx_pki_parse_mode_chg	parse_mode_chg;
	enum cvmx_pki_layer_type	layer_type_set;
	int				style_add;
	int				parse_flag_set;
	int				pointer_advance;
};

struct cvmx_pki_pcam_config {
	int				in_use;
	int 				entry_num;
	uint64_t			cluster_mask;
	struct cvmx_pki_pcam_input	pcam_input;
	struct cvmx_pki_pcam_action	pcam_action;
};

struct cvmx_pki_pcam_list {
	int	 			index;
	struct cvmx_pki_pcam_config	pcam_cfg[CVMX_PKI_TOTAL_PCAM_ENTRY];
};

/** PKI block configuration*/
struct cvmx_pki_config {
	struct cvmx_pki_global_config		global_cfg;
	struct cvmx_pki_clustergrp_config	cluster_cfg[CVMX_PKI_NUM_CLUSTER_GROUP];
	struct cvmx_pki_port_config		port_cfg[CVMX_HELPER_MAX_IFACE][CVMX_HELPER_CFG_MAX_PORT_PER_IFACE];
	struct cvmx_pki_style_config		style_cfg[CVMX_PKI_NUM_FINAL_STYLES];
	struct cvmx_pki_qpg_config		qpg_cfg[CVMX_PKI_NUM_QPG_ENTRY];
	struct cvmx_pki_fpa_pool_config		pool_cfg[CVMX_FPA3_NUM_POOLS];
	struct cvmx_pki_aura_config		aura_cfg[CVMX_FPA_AURA_NUM];
	struct cvmx_pki_sso_grp_config		sso_grp_cfg[CVMX_PKI_NUM_SSO_GROUP];
};

/** Mapping of profile names to their respective config number*/
struct cvmx_pki_profiles {
	struct cvmx_pki_pcam_list		pcam_list;
	struct cvmx_pki_cluster_list    	cl_profile_list;
	struct cvmx_pki_style_list		style_profile_list;
	struct cvmx_pki_pool_list		pool_profile_list;
	struct cvmx_pki_aura_list		aura_profile_list;
	struct cvmx_pki_sso_grp_list	        sso_grp_profile_list;
	struct cvmx_pki_qpg_list	        qpg_profile_list;
};

extern CVMX_SHARED struct cvmx_pki_config pki_config[CVMX_MAX_NODES];
extern CVMX_SHARED struct cvmx_pki_profiles pki_profiles[CVMX_MAX_NODES];

/**
 * This function writes qpg entry at specified offset in hardware
 *
 * @param node		node number
 * @param index		offset in qpg entry to write to.
 * @param padd		port address for channel calculation
 * @param aura		aura number to send packet to
 * @param group_ok	group number to send packet to if there is no error
 * @param group_bad	group number to send packet  to if there is error
 */
static inline void cvmx_pki_write_qpg_entry(int node, int index, int padd, int aura,
					      int group_ok, int group_bad)
{
	cvmx_pki_qpg_tblx_t qpg_tbl;
	qpg_tbl.u64 = cvmx_read_csr_node(node, CVMX_PKI_QPG_TBLX(index));
	qpg_tbl.s.padd = padd;
	qpg_tbl.s.laura = aura;
	qpg_tbl.s.grp_ok = group_ok;
	qpg_tbl.s.grp_bad = group_bad;
	cvmx_write_csr_node(node, CVMX_PKI_QPG_TBLX(index), qpg_tbl.u64);
}

/**
 * This function assignes the clusters to a group, later pkind can be
 * configured to use that group depending on number of clusters pkind
 * would use. A given cluster can only be enabled in a single cluster group.
 * Number of clusters assign to that group determines how many engine can work
 * in parallel to process the packet. Eack cluster can process x MPPS.
 *
 * @param cluster_group Group to attach clusters to
 * @param cluster_mask  It is the mask of clusters which needs to be assigned to group.
 * to that group
 *
 */
static inline int cvmx_pki_attach_cluster_to_group(int node, uint64_t cluster_group,
		 uint64_t cluster_mask)
{
	cvmx_pki_icgx_cfg_t pki_cl_grp;

	if (node >= CVMX_MAX_NODES) {
		cvmx_dprintf("Invalid node number %d", node);
		return -1;
	}

	if (cluster_group >= CVMX_PKI_NUM_CLUSTER_GROUP) {
		cvmx_dprintf("ERROR: config cluster group %d", (int)cluster_group);
		return -1;
	}
	pki_cl_grp.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(cluster_group));
	pki_cl_grp.s.clusters = cluster_mask;
	cvmx_write_csr_node(node, CVMX_PKI_ICGX_CFG(cluster_group), pki_cl_grp.u64);
	pki_config[node].global_cfg.clusters_in_use_mask |= cluster_mask;
	pki_config[node].cluster_cfg[cluster_group].cluster_mask = cluster_mask;
	return 0;
}

/**
 * This function enables the cluster group to start parsing
 *
 * @param node          node number
 * @param cl_grp        cluster group to enable parsing
 *
 */
static inline int cvmx_pki_parse_enable(int node, int cl_grp)
{
	cvmx_pki_icgx_cfg_t pki_cl_grp;

	if (cl_grp >= CVMX_PKI_NUM_CLUSTER_GROUP) {
		cvmx_dprintf("ERROR: pki parse en group %d", (int)cl_grp);
		return -1;
	}
	pki_cl_grp.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(cl_grp));
	pki_cl_grp.s.pena = 1;
	cvmx_write_csr_node(node, CVMX_PKI_ICGX_CFG(cl_grp), pki_cl_grp.u64);
	return 0;
}

/**
 * This function enables the PKI to send bpid level backpressure
 * to CN78XX inputs.
 * @param node       node number
 */
static inline void cvmx_pki_enable_backpressure(int node)
{
	cvmx_pki_buf_ctl_t pki_buf_ctl;

	pki_buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	pki_buf_ctl.s.pbp_en = 1;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, pki_buf_ctl.u64);
}

static inline void cvmx_pki_mark_style_in_use(int node, int style)
{
#define CVMX_PKI_MANAGE_RESOURCES 1 /*vinita_to_do, later make it global option*/
#if CVMX_PKI_MANAGE_RESOURCES

	/*vinita_to_do spinlock*/
	pki_config[node].style_cfg[style].users++;
#else
#endif
}


/**
 * This function enables pki
 * @param node	 node to enable pki in.
 */
void cvmx_pki_enable(int node);

/**
 * This function disables pki
 * @param node	 	node to disable pki in.
 */
void cvmx_pki_disable(int node);

/**
 * This function writes per pkind parameters in hardware which defines how
  the incoming packet is processed.
 * @param node	 	      node to which pkind belongs.
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
 */
int cvmx_pki_write_pkind(int node, int pkind, int cluster_group,
			   int initial_parse_mode, int initial_style);

/**
 * This function writes/configures parameters associated with style in hardware.
 * @param node	              node to which style belong.
 * @param style		      style to configure.
 * @param cluster_mask	      Mask of clusters to configure the style for.
 * @param style_cfg 	      parameters to configure for style passed in struct.
 */
void cvmx_pki_write_style(int node, uint64_t style, uint64_t cluster_mask,
			    struct cvmx_pki_style_config style_cfg);

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
				struct cvmx_pki_pcam_input input, struct cvmx_pki_pcam_action action);

int cvmx_pki_setup_clusters(int node);

/**
 * Configures the channel which will receive backpressure
 * from the specified bpid.
 * Each channel listens for backpressure on a specific bpid.
 * Each bpid can backpressure multiple channels.
 * @param node    node number
 * @param bpid    bpid from which, channel will receive backpressure.
 * @param channel channel numner to receive backpressue.
 */
int cvmx_pki_write_channel_bpid(int node, int channel, int bpid);

/**
 * Configures the bpid on which, specified channel will
 * assert backpressure.
 * Each bpid receives backpressure from auras.
 * Multiple auras can backpressure single bpid.
 * @param node   node number
 * @param aura   number which will assert backpressure on that bpid.
 * @param bpid   to assert backpressure on.
 */
int cvmx_pki_write_aura_bpid(int node, int aura, int bpid);

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
			      bool ena_drop, bool ena_bp);

/**
 * This function finds if cluster profile with name already exist
 * @param node  node number
 * @param name  profile name to look for
 * @return 	profile index in cluster list on SUCCESS
 *		-1 if profile not found in cluster list
 */
int cvmx_pki_cluster_profile_exist(int node, char *name);

/**
 * This function finds cluster mask associated with
 * given cluster profile name.
 * @param node  node number
 * @param name  profile name to look for
 * @return 	cluster_mask on SUCCESS
 *		-1 if profile not found in cluster list
 */
int cvmx_pki_find_cluster_mask(int node, char *name);

/**
 * This function finds cluster group associated with
 * given cluster profile name
 * @param node  node number
 * @param name  profile name to look for
 * @return 	cluster group number on SUCCESS
 *		-1 if profile not found in cluster list
 */
int cvmx_pki_find_cluster_group(int node, char *name);

/**
 * This function finds if fpa pool profile with
 * name already exist
 * @param node  node number
 * @param name  profile name to look for
 * @return 	profile index in pool list on SUCCESS
 *		-1 if profile not found in pool list
 */
int cvmx_pki_pool_profile_exist(int node, char *name);

/**
 * This function finds if fpa pool number associated with
 * given profile name
 * @param node  node number
 * @param name  profile name to look for
 * @return 	pool number on SUCCESS
 *		-1 if profile not found in pool list
 */
int cvmx_pki_find_pool(int node, char *name);

/**
 * This function finds if fpa aura with given name
 * exist in aura list
 * @param node  node number
 * @param name  profile name to look for
 * @return 	aura index in aura list on SUCCESS
 *		-1 if profile not found in aura list
 */
int cvmx_pki_aura_profile_exist(int node, char *name);

/**
 * This function finds aura number associated with
 * given aura name.
 * @param node  node number
 * @param name  profile name to look for
 * @return 	aura number in aura list on SUCCESS
 *		-1 if profile not found in aura list
 */
int cvmx_pki_find_aura(int node, char *name);

/**
 * This function finds if group with given name
 * exist in group list
 * @param node  node number
 * @param name  profile name to look for
 * @return 	index in group list on SUCCESS
 *		-1 if profile not found in group list
 */
int cvmx_pki_group_profile_exist(int node, char *name);

/**
 * This function finds group number associated with
 * given group profile name
 * @param node  node number
 * @param name  profile name to look for
 * @return 	group number on SUCCESS
 *		-1 if profile not found in group list
 */
int cvmx_pki_find_group(int node, char *name);

/**
 * This function finds if qpg profile with given name
 * exist in qpg list
 * @param node  node number
 * @param name  profile name to look for
 * @return 	index in qpg list on SUCCESS
 *		-1 if profile not found in qpg list
 */
int cvmx_pki_qpg_profile_exist(int node, char *name);

/**
 * This function finds qpg base offset associated with
 * given qpg profile name.
 * @param node  node number
 * @param name  profile name to look for
 * @return 	qpg base offset on SUCCESS
 *		-1 if profile not found in qpg list
 */
int cvmx_pki_find_qpg_base_offset(int node, char *name);

/**
 * This function get the buffer size of the given pool number
 * @param node  node number
 * @param pool  fpa pool number
 * @return 	buffer size SUCCESS
 *		-1 if pool number is not found in pool list
 */
int cvmx_pki_get_pool_buffer_size(int node, int pool);

/**
 * This function get the buffer size of the given aura number
 * @param node  node number
 * @param pool  fpa aura number
 * @return 	buffer size SUCCESS
 *		-1 if aura number is not found in aura list
 */
int cvmx_pki_get_aura_buffer_size(int node, int aura);

int cvmx_pki_get_mbuff_size (int node, int base_offset);

/**
 * This function finds if style profile with given name
 * exist in style list
 * @param node  node number
 * @param name  profile name to look for
 * @return 	index into style list on SUCCESS
 *		-1 if profile not found in style list
 */
int cvmx_pki_style_profile_exist(int node, char *name);

/**
 * This function finds style number associated with
 * given style profile name.
 * @param node  node number
 * @param name  profile name to look for
 * @return 	style number on SUCCESS
 *		 -1 if profile not found in style list
 */
int cvmx_pki_find_style(int node, char *name);


/**
 * This function stores the cluster configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param name  	name associated with this config
 * @param cl_profile    structure containing cluster profile parameters below
 * 			-cluster_group (-1 if needs to be allocated)
 * 			-num_cluster   (number of cluster in the cluster group)
 * 			-parsing_mask  (parsing mask for the cluster group)
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_cluster_config(int node, struct cvmx_pki_cluster_profile cl_profile);

/**
 * This function stores the pool configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param pool_name  	name associated with this config
 * @param pool_numb     pool number (-1 if need to be allocated)
 * @param buffer_size	size of buffers in specified pool
 * @param num_buffers	numberof buffers in specified pool
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_pool_config(int node, char *pool_name, int pool_num,
			     uint64_t buffer_size, uint64_t num_buffers);

/**
 * This function stores the aura configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param aura_name  	name associated with this config
 * @param aura_num      aura number (-1 if need to be allocated)
 * @param pool  	pool to which aura is mapped
 * @param num_buffers	number of buffers to allocate to aura.
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_aura_config(int node, char *aura_name, int aura_num, int pool,
			     int num_buffers);

/**
 * This function stores the group configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param grp_profile	struct to SSO group profile to configure
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_sso_group_config(int node, struct cvmx_pki_sso_grp_profile grp_profile);

/**
 * This function stores the qpg configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param name  	name associated with this config
 * @param base_offset	offset in QPG table (-1 if needs to be allocated)
 * @param num_entries	total number of indexes needs to be allocated from
 *                      base_offset.
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_qpg_profile(int node, char *name, int base_offset, int num_entries);

/**
 * This function stores the group configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param aura_name  	name associated with this config
 * @param group		SSO group number (-1 if needs to be allocated)
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_qpg_config(int node, char *name, int entry_start,
			    int entry_end, struct cvmx_pki_qpg_config qpg_config);

/**
 * This function stores the style configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param aura_name  	name associated with this config
 * @param style_num	style number (-1 if needs to be allocated)
 * @param style_cfg	pointer to struct which has parameters related
 *                      to style config
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_style_config(int node, char *style_name, int style_num,
			      struct cvmx_pki_style_config *style_cfg);

/**
 * This function stores the pkind style configuration in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param pkind  	pkind number
 * @param style_name	pointer to style name which need to be assigned to pkind
 * @return 		0 on SUCCESS
 *			-1 on failure
 */
int cvmx_pki_set_port_style_cfg(uint64_t node, uint64_t interface, uint64_t port, uint64_t style);

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
					  uint64_t port, enum cvmx_pki_pkind_parse_mode parse_mode);

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
				       int cl_grp, uint64_t cl_mask);

/**
 * This function stores the pcam entry in data structure
 * which is then used to program the hardware.
 * @param node  	node number
 * @param cluster_mask	Mask of clusters on which the entry meeds to be appiled.
 * @param input		structure of pcam input parameter which defines matching creteria.
 * @param action	structure of pcam action parameters which aill be applied if match is found.
 * @return              0 on scuccess
 *			-1 on failure
 */
int cvmx_pki_set_pcam_entry(int node, int pcam_index, uint64_t cl_mask,
			    struct cvmx_pki_pcam_input input,
			    struct cvmx_pki_pcam_action action);

void cvmx_pki_set_default_pool_buffer_count(int node, uint64_t buffer_Count);
void cvmx_pki_set_default_pool_config(int node, int64_t pool,
				      int64_t buffer_size, int64_t buffer_count);
void cvmx_pki_set_default_aura_config(int node, int64_t aura,
				      int64_t pool, uint64_t buffer_count);
int cvmx_pki_initialize_data_structures(int node);
void cvmx_pki_get_style_config(int node, int style, struct cvmx_pki_style_config *style_cfg);

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif
