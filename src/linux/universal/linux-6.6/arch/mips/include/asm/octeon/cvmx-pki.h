/***********************license start***************
 * Copyright (c) 2003-2017  Cavium Inc. (support@cavium.com). All rights
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
#include <asm/octeon/cvmx-fpa3.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#else
#include "cvmx-fpa3.h"
#include "cvmx-helper-util.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-error.h"
#endif

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* PKI AURA and BPID count are equal to FPA AURA count */
#define CVMX_PKI_NUM_AURA              (cvmx_fpa3_num_auras())
#define CVMX_PKI_NUM_BPID              (cvmx_fpa3_num_auras())
#define CVMX_PKI_NUM_SSO_GROUP         (cvmx_sso_num_xgrp())
#define CVMX_PKI_NUM_CLUSTER_GROUP_MAX 1
#define CVMX_PKI_NUM_CLUSTER_GROUP     (cvmx_pki_num_cl_grp())
#define CVMX_PKI_NUM_CLUSTER           (cvmx_pki_num_clusters())

/* FIXME: Reduce some of these values, convert to routines XXX */
#define CVMX_PKI_NUM_CHANNEL        4096
#define CVMX_PKI_NUM_PKIND          64
#define CVMX_PKI_NUM_INTERNAL_STYLE 256
#define CVMX_PKI_NUM_FINAL_STYLE    64
#define CVMX_PKI_NUM_QPG_ENTRY      2048
#define CVMX_PKI_NUM_MTAG_IDX       (32 / 4) /* 32 registers grouped by 4*/
#define CVMX_PKI_NUM_LTYPE          32
#define CVMX_PKI_NUM_PCAM_BANK      2
#define CVMX_PKI_NUM_PCAM_ENTRY     192
#define CVMX_PKI_NUM_FRAME_CHECK    2
#define CVMX_PKI_NUM_BELTYPE        32
#define CVMX_PKI_MAX_FRAME_SIZE     65535
#define CVMX_PKI_FIND_AVAL_ENTRY    (-1)
#define CVMX_PKI_CLUSTER_ALL        0xf

#ifdef CVMX_SUPPORT_SEPARATE_CLUSTER_CONFIG
#define CVMX_PKI_TOTAL_PCAM_ENTRY \
	((CVMX_PKI_NUM_CLUSTER) * (CVMX_PKI_NUM_PCAM_BANK) * (CVMX_PKI_NUM_PCAM_ENTRY))
#else
#define CVMX_PKI_TOTAL_PCAM_ENTRY (CVMX_PKI_NUM_PCAM_BANK * CVMX_PKI_NUM_PCAM_ENTRY)
#endif

static inline unsigned cvmx_pki_num_clusters(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 2;
	return 4;
}
static inline unsigned cvmx_pki_num_cl_grp(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
		OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 1;
	return 0;
}

enum cvmx_pki_pkind_parse_mode {
	CVMX_PKI_PARSE_LA_TO_LG = 0, /* Parse LA(L2) to LG */
	CVMX_PKI_PARSE_LB_TO_LG = 1, /* Parse LB(custom) to LG */
	CVMX_PKI_PARSE_LC_TO_LG = 3, /* Parse LC(L3) to LG */
	CVMX_PKI_PARSE_LG       = 0x3f, /* Parse LG */
	CVMX_PKI_PARSE_NOTHING  = 0x7f /* Parse nothing */
};

enum cvmx_pki_parse_mode_chg {
	CVMX_PKI_PARSE_NO_CHG     = 0x0,
	CVMX_PKI_PARSE_SKIP_TO_LB = 0x1,
	CVMX_PKI_PARSE_SKIP_TO_LC = 0x3,
	CVMX_PKI_PARSE_SKIP_TO_LD = 0x7,
	CVMX_PKI_PARSE_SKIP_TO_LG = 0x3f,
	CVMX_PKI_PARSE_SKIP_ALL   = 0x7f,
};

enum cvmx_pki_l2_len_mode {
	PKI_L2_LENCHK_EQUAL_GREATER = 0,
	PKI_L2_LENCHK_EQUAL_ONLY
};

enum cvmx_pki_cache_mode {
	CVMX_PKI_OPC_MODE_STT      = 0LL, /* All blocks write through DRAM,*/
	CVMX_PKI_OPC_MODE_STF      = 1LL, /* All blocks into L2 */
	CVMX_PKI_OPC_MODE_STF1_STT = 2LL, /* 1st block L2, rest DRAM */
	CVMX_PKI_OPC_MODE_STF2_STT = 3LL /* 1st, 2nd blocks L2, rest DRAM */
};

/**
 * Tag type definitions
 */
enum cvmx_sso_tag_type {
	CVMX_SSO_TAG_TYPE_ORDERED  = 0L, /**< Tag ordering is maintained */
	CVMX_SSO_TAG_TYPE_ATOMIC   = 1L,
		/**< Tag ordering is maintained, and at most one PP has the tag */
	CVMX_SSO_TAG_TYPE_UNTAGGED = 2L, /**< The work queue entry from the order
		- NEVER tag switch from NULL to NULL */
	CVMX_SSO_TAG_TYPE_EMPTY    = 3L
		/**< A tag switch to NULL, and there is no space reserved in POW
		- NEVER tag switch to NULL_NULL
		- NEVER tag switch from NULL_NULL
		- NULL_NULL is entered at the beginning of time and on a deschedule.
		- NULL_NULL can be exited by a new work request.
		A NULL_SWITCH load can also switch the state to NULL */
};

enum cvmx_pki_qpg_qos {
	CVMX_PKI_QPG_QOS_NONE = 0,
	CVMX_PKI_QPG_QOS_VLAN,
	CVMX_PKI_QPG_QOS_MPLS,
	CVMX_PKI_QPG_QOS_DSA_SRC,
	CVMX_PKI_QPG_QOS_DIFFSERV,
	CVMX_PKI_QPG_QOS_HIGIG,
};

enum cvmx_pki_wqe_vlan {
	CVMX_PKI_USE_FIRST_VLAN = 0,
	CVMX_PKI_USE_SECOND_VLAN
};

/**
 * Controls how the PKI statistics counters are handled
 * The PKI_STAT*_X registers can be indexed either by port kind (pkind), or
 * final style. (Does not apply to the PKI_STAT_INB* registers.)
 *    0 = X represents the packet’s pkind
 *    1 = X represents the low 6-bits of packet’s final style
 */
enum cvmx_pki_stats_mode {
	CVMX_PKI_STAT_MODE_PKIND,
	CVMX_PKI_STAT_MODE_STYLE
};

enum cvmx_pki_fpa_wait {
	CVMX_PKI_DROP_PKT,
	CVMX_PKI_WAIT_PKT
};

#define PKI_BELTYPE_E__NONE_M 0x0
#define PKI_BELTYPE_E__MISC_M 0x1
#define PKI_BELTYPE_E__IP4_M  0x2
#define PKI_BELTYPE_E__IP6_M  0x3
#define PKI_BELTYPE_E__TCP_M  0x4
#define PKI_BELTYPE_E__UDP_M  0x5
#define PKI_BELTYPE_E__SCTP_M 0x6
#define PKI_BELTYPE_E__SNAP_M 0x7

/* PKI_BELTYPE_E_t */
enum cvmx_pki_beltype {
	CVMX_PKI_BELTYPE_NONE = PKI_BELTYPE_E__NONE_M,
	CVMX_PKI_BELTYPE_MISC = PKI_BELTYPE_E__MISC_M,
	CVMX_PKI_BELTYPE_IP4  = PKI_BELTYPE_E__IP4_M,
	CVMX_PKI_BELTYPE_IP6  = PKI_BELTYPE_E__IP6_M,
	CVMX_PKI_BELTYPE_TCP  = PKI_BELTYPE_E__TCP_M,
	CVMX_PKI_BELTYPE_UDP  = PKI_BELTYPE_E__UDP_M,
	CVMX_PKI_BELTYPE_SCTP = PKI_BELTYPE_E__SCTP_M,
	CVMX_PKI_BELTYPE_SNAP = PKI_BELTYPE_E__SNAP_M,
	CVMX_PKI_BELTYPE_MAX  = CVMX_PKI_BELTYPE_SNAP
};

struct cvmx_pki_frame_len {
	uint16_t maxlen;
	uint16_t minlen;
};

struct cvmx_pki_tag_fields {
	uint64_t layer_g_src :1;
	uint64_t layer_f_src :1;
	uint64_t layer_e_src :1;
	uint64_t layer_d_src :1;
	uint64_t layer_c_src :1;
	uint64_t layer_b_src :1;
	uint64_t layer_g_dst :1;
	uint64_t layer_f_dst :1;
	uint64_t layer_e_dst :1;
	uint64_t layer_d_dst :1;
	uint64_t layer_c_dst :1;
	uint64_t layer_b_dst :1;
	uint64_t input_port :1;
	uint64_t mpls_label :1;
	uint64_t first_vlan :1;
	uint64_t second_vlan :1;
	uint64_t ip_prot_nexthdr :1;
	uint64_t tag_sync :1;
	uint64_t tag_spi :1;
	uint64_t tag_gtp :1;
	uint64_t tag_vni :1;
};

struct cvmx_pki_pkind_parse {
	uint64_t mpls_en :1;	/**< Enable MPLS parsing.
		0 = Any MPLS labels are ignored, but may be handled by custom
		Ethertype PCAM matchers.
		1 = MPLS label stacks are parsed and skipped over.
		PKI_GBL_PEN[MPLS_PEN] must be set.*/
	uint64_t inst_hdr :1; /**< INST header.
		When set, the eight-byte INST_HDR is present on all packets
		(except incoming packets on the DPI ports). */
	uint64_t lg_custom :1; /**< Layer G Custom Match Enable.
		0 = Disable custom LG header extraction
		1 = Enable custom LG header extraction.*/
	uint64_t fulc_en :1;	/**< Enable Fulcrum tag parsing.
		0 = Any Fulcrum header is ignored.
		1 = Fulcrum header is parsed.*/
	uint64_t dsa_en :1; /**< Enable DSA parsing.
		This field should not be set for DPI ports.
		0 = Any DSA header is ignored.
		1 = DSA is parsed. */
	uint64_t hg2_en :1; /**< Enable HiGig 2 parsing.
		This field should not be set for DPI ports.
		0 = Any HiGig2 header is ignored.
		1 = HiGig2 is parsed. PKI_GBL_PEN[HG_PEN] must be set. */
	uint64_t hg_en :1; /**< Enable HiGig parsing.
		This field should not be set for DPI ports.
		0 = Any HiGig header is ignored.
		1 = HiGig is parsed. PKI_GBL_PEN[HG_PEN] must be set.
		At most one of FULC_EN, DSA_EN or HG_EN may be set. */
};

struct cvmx_pki_pool_config {
	int pool_num;
	cvmx_fpa3_pool_t pool;
	uint64_t buffer_size;
	uint64_t buffer_count;
};

struct cvmx_pki_qpg_config {
	int qpg_base;
	int port_add;
	int aura_num;
	int grp_ok;
	int grp_bad;
	int grptag_ok;
	int grptag_bad;
};

struct cvmx_pki_aura_config {
	int aura_num;
	int pool_num;
	cvmx_fpa3_pool_t pool;
	cvmx_fpa3_gaura_t aura;
	int buffer_count;
};

struct cvmx_pki_cluster_grp_config {
	int grp_num;
	uint64_t cluster_mask; /* Bit mask of cluster assigned to this cluster group */
};

struct cvmx_pki_sso_grp_config {
	int group;
	int priority;
	int weight;
	int affinity;
	uint64_t core_mask;
	uint8_t core_mask_set;
};

/* This is per style structure for configuring port parameters,
 * it is kind of of profile which can be assigned to any port.
 * If multiple ports are assigned same style be aware that modifying
 * that style will modify the respective parameters for all the ports
 * which are using this style
 */
struct cvmx_pki_style_parm {

	bool ip6_udp_opt; /**< IPv6/UDP checksum is optional.
		IPv4 allows an optional UDP checksum by sending the all-0s patterns.
		IPv6 outlaws this and the spec says to always check UDP checksum.
		0 = Spec compliant, do not allow optional code.
		1 = Treat IPv6 as IPv4; */
	bool lenerr_en; /**< L2 length error check enable.
		Check if frame was received with L2 length error. */
	bool maxerr_en; /**< Max frame error check enable. */
	bool minerr_en; /**< Min frame error check enable. */
	uint8_t lenerr_eqpad; /**< L2 length checks exact pad size.
		0 = Length check uses greater then or equal comparison.
		1 = Length check uses equal comparison.*/
	uint8_t minmax_sel;	/**< Selects which PKI_FRM_LEN_CHK(0..1) register is used
		for this pkind for MINERR and MAXERR */
	bool qpg_dis_grptag; /**< Disable computing group using WQE[TAG].
		1 -- Disable 0 -- Enable */
	bool fcs_strip; /**< Strip L2 FCS bytes from packet, decrease WQE[LEN] by 4 bytes.*/
	bool fcs_chk; /**< FCS checking enabled.*/
	bool rawdrp; /**< Allow RAW packet drop
		0 = Never drop packets with WQE[RAW] set.
		1 = Allow the PKI to drop RAW packets based on
		PKI_AURA(0..1023)_CFG[ENA_RED/ENA_DROP]. */
	bool force_drop; /**< Force packet dropping.
		0 = Drop packet based on PKI_AURA(0..1023)_CFG[ENA_RED/ENA_DROP].
		1 = Always drop the packet. Overrides NODROP, RAWDRP. */
	bool nodrop; /**< Disable QoS packet drop.
		0 = Allowed to drop packet based on PKI_AURA(0..1023)_CFG[ENA_RED/ENA_DROP].
		1 = Never drop the packet. Overrides RAWDRP. */
	bool qpg_dis_padd; /**< Disable computing port adder by QPG algorithm. */
	bool qpg_dis_grp; /**< Disable computing group by QPG algorithm. */
	bool qpg_dis_aura; /**< Disable computing aura by QPG algorithm. */
	uint16_t qpg_base; /**< Base index into PKI_QPG_TBL(0..2047)*/
	enum cvmx_pki_qpg_qos qpg_qos; /**< Algorithm to select QoS field in QPG calculation */
	uint8_t qpg_port_sh; /**< MSB to take from port number in QPG calculation
		0 = Exclude port number from QPG.
		4 = Include port<3:0>.
		8 = Include port<7:0>.*/
	uint8_t qpg_port_msb; /**< Number of bits to shift port number in QPG */
	uint8_t apad_nip; /**< Value for WQE[APAD] when packet is not IP. */
	uint8_t wqe_vs; /**< Which VLAN to put into WQE[VLPTR] when VLAN stacking.
		0 = Use the first (in network order) VLAN or DSA VID.
		1 = Use the second (in network order) VLAN. */
	enum cvmx_sso_tag_type tag_type; /**< SSO tag type to schedule to */
	bool pkt_lend; /**< Packet little-endian.write operations of packet data
		to L2C to be in LE */
	uint8_t wqe_hsz; /**< Work queue header size:
		0x0 = WORD0..4, standard WQE_S.
		Note FIRST_SKIP may be set to not include WORD4 in memory.
		0x1 = WORD0..5
		0x2 = WORD0..6
		0x3 = WORD0..7
		else = Reserved */
	uint16_t wqe_skip; /**< in bytes, WQE start offset.
		The number of bytes to skip between the buffer
		Pointer and WORD0 of the work-queue entry.*/
	uint16_t first_skip; /**< in bytes, The number of bytes from the top
		of the first MBUF, that the PKI stores the next pointer.*/
	uint16_t later_skip; /**< in bytes, The number of bytes from the top
		of any MBUF, that is not the first MBUF that PKI writes
		next-pointer to.*/
	enum cvmx_pki_cache_mode cache_mode; /**< Select the style of write to the L2C.
		0 = all packet data and next-buffer pointers are written through to memory.
		1 = all packet data and next-buffer pointers are written into the cache.
		2 = the first aligned cache block holding the WQE and/or front packet data
		are written to the L2 cache. All remaining cache blocks are not written to
		the L2 cache.
		3 = the first two aligned cache blocks holding the WQE and front packet data
		are written to the L2 cache. All remaining cache blocks are not written to
		the L2 cache. */
	uint8_t dis_wq_dat; /**< Separate first data buffer from the work queue entry.
		0 = The packet link pointer will be at word [FIRST_SKIP] immediately followed
		by packet data, in the same buffer as the work queue entry.
		1 = The packet link pointer will be at word [FIRST_SKIP] in a new buffer
		separate from the work queue entry.*/
	uint64_t mbuff_size; /**< in bytes, The number of bytes to store into a buffer.
		This must be even, in the range of 32 to 4096 and this must be
		less than or equal to the maximum size of every free page in every FPA pool
		this style may use. */
	bool len_lg; /**< Check length of Layer G. */
	bool len_lf; /**< Check length of Layer F. */
	bool len_le; /**< Check length of Layer E. */
	bool len_ld; /**< Check length of Layer D. */
	bool len_lc; /**< Check length of Layer C. */
	bool len_lb; /**< Check length of Layer B. */
	bool csum_lg; /**< Compute checksum on Layer G. */
	bool csum_lf; /**< Compute checksum on Layer F. */
	bool csum_le; /**< Compute checksum on Layer E. */
	bool csum_ld; /**< Compute checksum on Layer D. */
	bool csum_lc; /**< Compute checksum on Layer C. */
	bool csum_lb; /**< Compute checksum on Layer B. */
};

/* This is per style structure for configuring port's tag configuration,
 * it is kind of of profile which can be assigned to any port.
 * If multiple ports are assigned same style be aware that modiying that style
 * will modify the respective parameters for all the ports which are
 * using this style */
enum cvmx_pki_mtag_ptrsel {
	CVMX_PKI_MTAG_PTRSEL_SOP = 0,
	CVMX_PKI_MTAG_PTRSEL_LA = 8,
	CVMX_PKI_MTAG_PTRSEL_LB = 9,
	CVMX_PKI_MTAG_PTRSEL_LC = 10,
	CVMX_PKI_MTAG_PTRSEL_LD = 11,
	CVMX_PKI_MTAG_PTRSEL_LE = 12,
	CVMX_PKI_MTAG_PTRSEL_LF = 13,
	CVMX_PKI_MTAG_PTRSEL_LG = 14,
	CVMX_PKI_MTAG_PTRSEL_VL = 15,
};

struct cvmx_pki_mask_tag {
	bool enable;
	int base; /* CVMX_PKI_MTAG_PTRSEL_XXX */
	int offset; /* Offset from base. */
	uint64_t val; /* Bitmask:
		1 = enable, 0 = disabled for each byte in the 64-byte array.*/
};

struct cvmx_pki_style_tag_cfg {
	struct cvmx_pki_tag_fields tag_fields;
	struct cvmx_pki_mask_tag mask_tag[4];
};

struct cvmx_pki_style_config {
	struct cvmx_pki_style_parm parm_cfg; /**< General parameter configuration. */
	struct cvmx_pki_style_tag_cfg  tag_cfg; /**< Tag parameter configuration. */
};

struct cvmx_pki_pkind_config {
	uint8_t cluster_grp; /**< Cluster group that will service traffic on this pkind */
	bool fcs_pres; /**< FCS present.
		0 = FCS not present. FCS may not be checked nor stripped.
		1 = FCS present; the last four bytes of the packet are part of the FCS
		and may not be considered part of a IP, TCP or other header for length
		error checks.*/
	struct cvmx_pki_pkind_parse parse_en;
	enum cvmx_pki_pkind_parse_mode initial_parse_mode;
	uint8_t fcs_skip;
	uint8_t inst_skip; /**< Skip amount from front of packet to first byte covered
		by FCS start. The skip must be even. If PTP_MODE, the 8-byte timestamp is
		prepended to the packet, and FCS_SKIP must be at least 8.*/
	int initial_style; /**< Skip amount from front of packet to begin parsing at.
		If PKI_CL(0..3)_PKIND(0..63)_CFG[INST_HDR] is set, points at the first
		byte of the instruction header. If INST_HDR is clear, points at the first
		byte to begin parsing at. The skip must be even. If PTP_MODE, the 8-byte
		timestamp is prepended to the packet, and INST_SKIP must be at least 8. */
	bool custom_l2_hdr; /**< Valid.custom L2 hesder extraction
		0 = Disable custom L2 header extraction.
		1 = Enable custom L2 header extraction.
		PKI_GBL_PEN[CLG_PEN] must be set. */
	uint8_t l2_scan_offset; /**< Scan offset for custom L2 header.
		Pointer to first byte of 32-bit custom extraction header, as absolute
		number of bytes from beginning of packet. If PTP_MODE, the 8-byte timestamp
		is prepended to the packet, and must be included in counting offset bytes. */
	uint64_t lg_scan_offset; /**< Scan offset for custom lg header.
		Pointer to first byte of 32-bit custom extraction header, as relative number
		of bytes from WQE[LFPTR]. */
};

struct cvmx_pki_port_config {
	struct cvmx_pki_pkind_config pkind_cfg; /**< Parameters can be configure per pkind */
	struct cvmx_pki_style_config style_cfg; /**< Parameters are configured per style,
		style is a profile, which can be applied to multiple ports which have same
		configuration and packet processing */
};

struct cvmx_pki_global_parse {
	uint64_t virt_pen :1; /**< Virtualization parsing enable.*/
	uint64_t clg_pen :1;	/**< Custom LG parsing enable. */
	uint64_t cl2_pen :1;	/**< Custom L2 parsing enable.*/
	uint64_t l4_pen :1; /**< L4 parsing enable.*/
	uint64_t il3_pen :1; /**< L3 inner parsing enable. Must be zero if L3_PEN is zero. */
	uint64_t l3_pen :1; /**< L3 parsing enable.*/
	uint64_t mpls_pen :1; /**< MPLS parsing enable.*/
	uint64_t fulc_pen :1; /**< Fulcrum parsing enable.*/
	uint64_t dsa_pen :1; /**< DSA parsing enable. Must be zero if HG_PEN is set.*/
	uint64_t hg_pen :1; /**< HiGig parsing enable. Must be zero if DSA_PEN is set.*/
};

struct cvmx_pki_tag_sec {
	uint16_t dst6; /**< Secret for the destination tuple IPv6 tag CRC calculation.
		Typically identical to SRC6 to insure tagging is symmetric between
		source/destination flows. Typically different from DST for maximum security. */
	uint16_t src6; /**< Secret for the source tuple IPv6 tag CRC calculation.
		Typically different from SRC for maximum security. */
	uint16_t dst; /**< Secret for the destination tuple tag CRC calculation.
		Typically identical to DST6 to insure tagging is symmetric between
		source/destination flows. */
	uint16_t src; /**< Secret for the source tuple tag CRC calculation. */
};

struct cvmx_pki_global_config {
	uint64_t cluster_mask[CVMX_PKI_NUM_CLUSTER_GROUP_MAX]; /**< Mask of clusters
		associated with that cluster group, there are 4 cluster groups and 4 clusters
		which can be assigned to cluster groups */
	enum cvmx_pki_stats_mode stat_mode;	/**< The PKI_STAT*_X registers can be indexed
		either by pkind or final style.
		(Does not apply to the PKI_STAT_INB* registers.)
		0 = X represents the packet's pkind
		1 = X represents the low 6-bits of packet's final style */
	enum cvmx_pki_fpa_wait fpa_wait; /**< Policy when FPA runs out of buffers:
		0 = Drop the remainder of the packet requesting the buffer, and
		set WQE[OPCODE] to RE_MEMOUT.
		1 = Wait until buffers become available, only dropping packets if
		buffering ahead of PKI fills. This may lead to head-of-line
		blocking of packets on other Auras */
	struct cvmx_pki_global_parse gbl_pen; /**< Controls Global parsing options for chip */
	struct cvmx_pki_tag_sec tag_secret; /**< Secret value for src/dst tag tuple to
		reduce cache collision attacks */
	struct cvmx_pki_frame_len frm_len[CVMX_PKI_NUM_FRAME_CHECK]; /**< Values for max
		and min frame length to check against,2 combination */
	enum cvmx_pki_beltype ltype_map[CVMX_PKI_NUM_BELTYPE]; /**< Map of which protocol
		maps to what layer */
	/* struct cvmx_pki_tag_ctl  tag_ctl[32]; */
	/* cvmx_pki_tag_incx_mask_t tag_mask[32]; */
	int pki_enable;
};

#define CVMX_PKI_PCAM_TERM_E_NONE_M      0x0
#define CVMX_PKI_PCAM_TERM_E_L2_CUSTOM_M 0x2
#define CVMX_PKI_PCAM_TERM_E_HIGIGD_M    0x4
#define CVMX_PKI_PCAM_TERM_E_HIGIG_M     0x5
#define CVMX_PKI_PCAM_TERM_E_SMACH_M     0x8
#define CVMX_PKI_PCAM_TERM_E_SMACL_M     0x9
#define CVMX_PKI_PCAM_TERM_E_DMACH_M     0xA
#define CVMX_PKI_PCAM_TERM_E_DMACL_M     0xB
#define CVMX_PKI_PCAM_TERM_E_GLORT_M     0x12
#define CVMX_PKI_PCAM_TERM_E_DSA_M       0x13
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE0_M  0x18
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE1_M  0x19
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE2_M  0x1A
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE3_M  0x1B
#define CVMX_PKI_PCAM_TERM_E_MPLS0_M     0x1E
#define CVMX_PKI_PCAM_TERM_E_L3_SIPHH_M  0x1F
#define CVMX_PKI_PCAM_TERM_E_L3_SIPMH_M  0x20
#define CVMX_PKI_PCAM_TERM_E_L3_SIPML_M  0x21
#define CVMX_PKI_PCAM_TERM_E_L3_SIPLL_M  0x22
#define CVMX_PKI_PCAM_TERM_E_L3_FLAGS_M  0x23
#define CVMX_PKI_PCAM_TERM_E_L3_DIPHH_M  0x24
#define CVMX_PKI_PCAM_TERM_E_L3_DIPMH_M  0x25
#define CVMX_PKI_PCAM_TERM_E_L3_DIPML_M  0x26
#define CVMX_PKI_PCAM_TERM_E_L3_DIPLL_M  0x27
#define CVMX_PKI_PCAM_TERM_E_LD_VNI_M    0x28
#define CVMX_PKI_PCAM_TERM_E_IL3_FLAGS_M 0x2B
#define CVMX_PKI_PCAM_TERM_E_LF_SPI_M    0x2E
#define CVMX_PKI_PCAM_TERM_E_L4_SPORT_M  0x2f
#define CVMX_PKI_PCAM_TERM_E_L4_PORT_M   0x30
#define CVMX_PKI_PCAM_TERM_E_LG_CUSTOM_M 0x39

enum cvmx_pki_term {
	CVMX_PKI_PCAM_TERM_NONE      = CVMX_PKI_PCAM_TERM_E_NONE_M,
	CVMX_PKI_PCAM_TERM_L2_CUSTOM = CVMX_PKI_PCAM_TERM_E_L2_CUSTOM_M,
	CVMX_PKI_PCAM_TERM_HIGIGD    = CVMX_PKI_PCAM_TERM_E_HIGIGD_M,
	CVMX_PKI_PCAM_TERM_HIGIG     = CVMX_PKI_PCAM_TERM_E_HIGIG_M,
	CVMX_PKI_PCAM_TERM_SMACH     = CVMX_PKI_PCAM_TERM_E_SMACH_M,
	CVMX_PKI_PCAM_TERM_SMACL     = CVMX_PKI_PCAM_TERM_E_SMACL_M,
	CVMX_PKI_PCAM_TERM_DMACH     = CVMX_PKI_PCAM_TERM_E_DMACH_M,
	CVMX_PKI_PCAM_TERM_DMACL     = CVMX_PKI_PCAM_TERM_E_DMACL_M,
	CVMX_PKI_PCAM_TERM_GLORT     = CVMX_PKI_PCAM_TERM_E_GLORT_M,
	CVMX_PKI_PCAM_TERM_DSA       = CVMX_PKI_PCAM_TERM_E_DSA_M,
	CVMX_PKI_PCAM_TERM_ETHTYPE0  = CVMX_PKI_PCAM_TERM_E_ETHTYPE0_M,
	CVMX_PKI_PCAM_TERM_ETHTYPE1  = CVMX_PKI_PCAM_TERM_E_ETHTYPE1_M,
	CVMX_PKI_PCAM_TERM_ETHTYPE2  = CVMX_PKI_PCAM_TERM_E_ETHTYPE2_M,
	CVMX_PKI_PCAM_TERM_ETHTYPE3  = CVMX_PKI_PCAM_TERM_E_ETHTYPE3_M,
	CVMX_PKI_PCAM_TERM_MPLS0     = CVMX_PKI_PCAM_TERM_E_MPLS0_M,
	CVMX_PKI_PCAM_TERM_L3_SIPHH  = CVMX_PKI_PCAM_TERM_E_L3_SIPHH_M,
	CVMX_PKI_PCAM_TERM_L3_SIPMH  = CVMX_PKI_PCAM_TERM_E_L3_SIPMH_M,
	CVMX_PKI_PCAM_TERM_L3_SIPML  = CVMX_PKI_PCAM_TERM_E_L3_SIPML_M,
	CVMX_PKI_PCAM_TERM_L3_SIPLL  = CVMX_PKI_PCAM_TERM_E_L3_SIPLL_M,
	CVMX_PKI_PCAM_TERM_L3_FLAGS  = CVMX_PKI_PCAM_TERM_E_L3_FLAGS_M,
	CVMX_PKI_PCAM_TERM_L3_DIPHH  = CVMX_PKI_PCAM_TERM_E_L3_DIPHH_M,
	CVMX_PKI_PCAM_TERM_L3_DIPMH  = CVMX_PKI_PCAM_TERM_E_L3_DIPMH_M,
	CVMX_PKI_PCAM_TERM_L3_DIPML  = CVMX_PKI_PCAM_TERM_E_L3_DIPML_M,
	CVMX_PKI_PCAM_TERM_L3_DIPLL  = CVMX_PKI_PCAM_TERM_E_L3_DIPLL_M,
	CVMX_PKI_PCAM_TERM_LD_VNI    = CVMX_PKI_PCAM_TERM_E_LD_VNI_M,
	CVMX_PKI_PCAM_TERM_IL3_FLAGS = CVMX_PKI_PCAM_TERM_E_IL3_FLAGS_M,
	CVMX_PKI_PCAM_TERM_LF_SPI    = CVMX_PKI_PCAM_TERM_E_LF_SPI_M,
	CVMX_PKI_PCAM_TERM_L4_PORT   = CVMX_PKI_PCAM_TERM_E_L4_PORT_M,
	CVMX_PKI_PCAM_TERM_L4_SPORT  = CVMX_PKI_PCAM_TERM_E_L4_SPORT_M,
	CVMX_PKI_PCAM_TERM_LG_CUSTOM = CVMX_PKI_PCAM_TERM_E_LG_CUSTOM_M
};

#define CVMX_PKI_DMACH_SHIFT	  32
#define CVMX_PKI_DMACH_MASK	      cvmx_build_mask(16)
#define CVMX_PKI_DMACL_MASK	      CVMX_PKI_DATA_MASK_32
#define CVMX_PKI_DATA_MASK_32	  cvmx_build_mask(32)
#define CVMX_PKI_DATA_MASK_16	  cvmx_build_mask(16)
#define CVMX_PKI_DMAC_MATCH_EXACT cvmx_build_mask(48)

struct cvmx_pki_pcam_input {
	uint64_t style;
	uint64_t style_mask; /* bits: 1-match, 0-dont care */
	enum cvmx_pki_term field;
	uint32_t field_mask; /* bits: 1-match, 0-dont care */
	uint64_t data;
	uint64_t data_mask; /* bits: 1-match, 0-dont care */
};

struct cvmx_pki_pcam_action {
	enum cvmx_pki_parse_mode_chg parse_mode_chg;
	enum cvmx_pki_layer_type layer_type_set;
	int style_add;
	int parse_flag_set;
	int pointer_advance;
};

struct cvmx_pki_pcam_config {
	int in_use;
	int entry_num;
	uint64_t cluster_mask;
	struct cvmx_pki_pcam_input pcam_input;
	struct cvmx_pki_pcam_action pcam_action;
};

/**
 * Status statistics for a port
 */
struct cvmx_pki_port_stats {
	uint64_t dropped_octets; /**< Inbound octets marked to be dropped by the IPD */
	uint64_t dropped_packets; /**< Inbound packets marked to be dropped by the IPD */
	uint64_t pci_raw_packets; /**< RAW PCI Packets received by PKI per port */
	uint64_t octets; /**< Number of octets processed by PKI */
	uint64_t packets; /**< Number of packets processed by PKI */
	uint64_t multicast_packets;	/**< Number of indentified L2 multicast packets.
		Does not include broadcast packets.
		Only includes packets whose parse mode is SKIP_TO_L2 */
	uint64_t broadcast_packets;	/**< Number of indentified L2 broadcast packets.
		Does not include multicast packets.
		Only includes packets whose parse mode is SKIP_TO_L2 */
	uint64_t len_64_packets; /**< Number of 64B packets */
	uint64_t len_65_127_packets; /**< Number of 65-127B packets */
	uint64_t len_128_255_packets; /**< Number of 128-255B packets */
	uint64_t len_256_511_packets; /**< Number of 256-511B packets */
	uint64_t len_512_1023_packets; /**< Number of 512-1023B packets */
	uint64_t len_1024_1518_packets; /**< Number of 1024-1518B packets */
	uint64_t len_1519_max_packets; /**< Number of 1519-max packets */
	uint64_t fcs_align_err_packets;	/**< Number of packets with FCS or Align opcode errors */
	uint64_t runt_packets; /**< Number of packets with length < min */
	uint64_t runt_crc_packets; /**< Number of packets with length < min and FCS error */
	uint64_t oversize_packets; /**< Number of packets with length > max */
	uint64_t oversize_crc_packets; /**< Number of packets with length > max and FCS error */
	uint64_t inb_packets; /**< Number of packets without GMX/SPX/PCI errors received by PKI */
	uint64_t inb_octets; /**< Total number of octets from all packets received by PKI,
		including CRC */
	uint64_t inb_errors; /**< Number of packets with GMX/SPX/PCI errors received by PKI */
	uint64_t mcast_l2_red_packets; /**< Number of packets with L2 Multicast DMAC
		that were dropped due to RED.
		The HW will consider a packet to be an L2 multicast packet, when
		the least-significant bit of the first byte of the DMAC is set and the packet
		is not an L2 broadcast packet.
		Only applies when the parse mode for the packets is SKIP-TO-L2 */
	uint64_t bcast_l2_red_packets; /**< Number of packets with L2 Broadcast DMAC
		that were dropped due to RED.
		The HW will consider a packet to be an L2 broadcast packet when the 48-bit
		DMAC is all 1's.
		Only applies when the parse mode for the packets is SKIP-TO-L2 */
	uint64_t mcast_l3_red_packets; /**< Number of packets with L3 Multicast Dest Address
		that were dropped due to RED.
		The HW considers an IPv4 packet to be multicast when the most-significant
		nibble of the 32-bit destination address is 0xE (i.e it is a class D address).
		The HW considers an IPv6 packet to be multicast when the most-significant byte
		of the 128-bit destination address is all 1's.
		Only applies when the parse mode for the packets is SKIP-TO-L2 and the packet
		is IP or the parse mode for the packet is SKIP-TO-IP */
	uint64_t bcast_l3_red_packets; /**< Number of packets with L3 Broadcast Dest Address
		that were dropped due to RED.
		The HW considers an IPv4 packet to be broadcast when all bits are set in
		the MSB of the destination address. IPv6 does not have the concept of
		broadcast packets.
		Only applies when the parse mode for the packet is SKIP-TO-L2 and the packet
		is IP or the parse mode for the packet is SKIP-TO-IP */
};

/**
 * PKI Packet Instruction Header Structure (PKI_INST_HDR_S)
 */
typedef union {
	uint64_t u64;
	struct {
		uint64_t w     : 1; /* INST_HDR size: 0 = 2 bytes, 1 = 4 or 8 bytes */
		uint64_t raw   : 1; /* RAW packet indicator in WQE[RAW]: 1 = enable */
		uint64_t utag  : 1; /* Use INST_HDR[TAG] to compute WQE[TAG]: 1 = enable */
		uint64_t uqpg  : 1; /* Use INST_HDR[QPG] to compute QPG: 1 = enable */
		uint64_t rsvd1 : 1;
		uint64_t pm    : 3; /* Packet parsing mode. Legal values = 0x0..0x7 */
		uint64_t sl    : 8; /* Number of bytes in INST_HDR. */
		/* The following fields are not present, if INST_HDR[W] = 0: */
		uint64_t utt   : 1; /* Use INST_HDR[TT] to compute WQE[TT]: 1 = enable */
		uint64_t tt    : 2; /* INST_HDR[TT] => WQE[TT], if INST_HDR[UTT] = 1 */
		uint64_t rsvd2 : 2;
		uint64_t qpg   :11; /* INST_HDR[QPG] => QPG, if INST_HDR[UQPG] = 1 */
		uint64_t tag   :32; /* INST_HDR[TAG] => WQE[TAG], if INST_HDR[UTAG] = 1 */
	} s;
} cvmx_pki_inst_hdr_t;

/**
 * This function assignes the clusters to a group, later pkind can be
 * configured to use that group depending on number of clusters pkind
 * would use. A given cluster can only be enabled in a single cluster group.
 * Number of clusters assign to that group determines how many engine can work
 * in parallel to process the packet. Eack cluster can process x MPPS.
 *
 * @param node	Node
 * @param cluster_group Group to attach clusters to.
 * @param cluster_mask The mask of clusters which needs to be assigned to the group.
 */
static inline int cvmx_pki_attach_cluster_to_group(int node, uint64_t cluster_group,
		 uint64_t cluster_mask)
{
	cvmx_pki_icgx_cfg_t pki_cl_grp;

	if (cluster_group >= CVMX_PKI_NUM_CLUSTER_GROUP) {
		cvmx_dprintf("ERROR: config cluster group %d", (int)cluster_group);
		return -1;
	}
	pki_cl_grp.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(cluster_group));
	pki_cl_grp.s.clusters = cluster_mask;
	cvmx_write_csr_node(node, CVMX_PKI_ICGX_CFG(cluster_group), pki_cl_grp.u64);
	return 0;
}

static inline void cvmx_pki_write_global_parse(int node,
	struct cvmx_pki_global_parse gbl_pen)
{
	cvmx_pki_gbl_pen_t gbl_pen_reg;
	gbl_pen_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_GBL_PEN);
	gbl_pen_reg.s.virt_pen = gbl_pen.virt_pen;
	gbl_pen_reg.s.clg_pen = gbl_pen.clg_pen;
	gbl_pen_reg.s.cl2_pen = gbl_pen.cl2_pen;
	gbl_pen_reg.s.l4_pen = gbl_pen.l4_pen;
	gbl_pen_reg.s.il3_pen = gbl_pen.il3_pen;
	gbl_pen_reg.s.l3_pen = gbl_pen.l3_pen;
	gbl_pen_reg.s.mpls_pen = gbl_pen.mpls_pen;
	gbl_pen_reg.s.fulc_pen = gbl_pen.fulc_pen;
	gbl_pen_reg.s.dsa_pen = gbl_pen.dsa_pen;
	gbl_pen_reg.s.hg_pen = gbl_pen.hg_pen;
	cvmx_write_csr_node(node, CVMX_PKI_GBL_PEN, gbl_pen_reg.u64);
}

static inline void cvmx_pki_write_tag_secret(int node,
	struct cvmx_pki_tag_sec tag_secret)
{
	cvmx_pki_tag_secret_t tag_secret_reg;
	tag_secret_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_SECRET);
	tag_secret_reg.s.dst6 = tag_secret.dst6;
	tag_secret_reg.s.src6 = tag_secret.src6;
	tag_secret_reg.s.dst = tag_secret.dst;
	tag_secret_reg.s.src = tag_secret.src;
	cvmx_write_csr_node(node, CVMX_PKI_TAG_SECRET, tag_secret_reg.u64);
}

static inline void cvmx_pki_write_ltype_map(int node,
		enum cvmx_pki_layer_type layer,
		enum cvmx_pki_beltype backend)
{
	cvmx_pki_ltypex_map_t ltype_map;
	if (layer > CVMX_PKI_LTYPE_E_MAX || backend > CVMX_PKI_BELTYPE_MAX) {
		cvmx_dprintf("ERROR: invalid ltype beltype mapping\n");
		return;
	}
	ltype_map.u64 = cvmx_read_csr_node(node, CVMX_PKI_LTYPEX_MAP(layer));
	ltype_map.s.beltype = backend;
	cvmx_write_csr_node(node, CVMX_PKI_LTYPEX_MAP(layer), ltype_map.u64);
}

/**
 * This function enables the cluster group to start parsing.
 *
 * @param node    Node number.
 * @param cl_grp  Cluster group to enable parsing.
 */
static inline int cvmx_pki_parse_enable(int node, unsigned cl_grp)
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
 * This function enables the PKI to send bpid level backpressure to CN78XX inputs.
 *
 * @param node Node number.
 */
static inline void cvmx_pki_enable_backpressure(int node)
{
	cvmx_pki_buf_ctl_t pki_buf_ctl;

	pki_buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	pki_buf_ctl.s.pbp_en = 1;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, pki_buf_ctl.u64);
}

/**
 * Clear the statistics counters for a port.
 *
 * @param node Node number.
 * @param port Port number (ipd_port) to get statistics for.
 *    Make sure PKI_STATS_CTL:mode is set to 0 for collecting per port/pkind stats.
 */
void cvmx_pki_clear_port_stats(int node, uint64_t port);

/**
 * Get the status counters for index from PKI.
 *
 * @param node	  Node number.
 * @param index   PKIND number, if PKI_STATS_CTL:mode = 0 or
 *     style(flow) number, if PKI_STATS_CTL:mode = 1
 * @param status  Where to put the results.
 */
void cvmx_pki_get_stats(int node, int index, struct cvmx_pki_port_stats *status);

/**
 * Get the statistics counters for a port.
 *
 * @param node	 Node number
 * @param port   Port number (ipd_port) to get statistics for.
 *    Make sure PKI_STATS_CTL:mode is set to 0 for collecting per port/pkind stats.
 * @param status Where to put the results.
 */
static inline void cvmx_pki_get_port_stats(int node, uint64_t port,
	struct cvmx_pki_port_stats *status)
{
	int xipd = cvmx_helper_node_to_ipd_port(node, port);
	int xiface = cvmx_helper_get_interface_num(xipd);
	int index = cvmx_helper_get_interface_index_num(port);
	int pknd = cvmx_helper_get_pknd(xiface, index);
	cvmx_pki_get_stats(node, pknd, status);
}

/**
 * Get the statistics counters for a flow represented by style in PKI.
 *
 * @param node Node number.
 * @param style_num Style number to get statistics for.
 *    Make sure PKI_STATS_CTL:mode is set to 1 for collecting per style/flow stats.
 * @param status Where to put the results.
 */
static inline void cvmx_pki_get_flow_stats(int node, uint64_t style_num,
	struct cvmx_pki_port_stats *status)
{
	cvmx_pki_get_stats(node, style_num, status);
}

/**
 * Show integrated PKI configuration.
 *
 * @param node	   node number
 */
int cvmx_pki_config_dump(unsigned node);

/**
 * Show integrated PKI statistics.
 *
 * @param node	   node number
 */
int cvmx_pki_stats_dump(unsigned node);

/**
 * Clear PKI statistics.
 *
 * @param node	   node number
 */
void cvmx_pki_stats_clear(unsigned node);

/**
 * This function enables PKI.
 *
 * @param node	 node to enable pki in.
 */
void cvmx_pki_enable(int node);

/**
 * This function disables PKI.
 *
 * @param node	node to disable pki in.
 */
void cvmx_pki_disable(int node);

/**
 * This function soft resets PKI.
 *
 * @param node	node to enable pki in.
 */
void cvmx_pki_reset(int node);

/**
 * This function sets the clusters in PKI.
 *
 * @param node	node to set clusters in.
 */
int cvmx_pki_setup_clusters(int node);

/**
 * This function reads global configuration of PKI block.
 *
 * @param node    Node number.
 * @param gbl_cfg Pointer to struct to read global configuration
 */
void cvmx_pki_read_global_config(int node, struct cvmx_pki_global_config *gbl_cfg);

/**
 * This function writes global configuration of PKI into hw.
 *
 * @param node    Node number.
 * @param gbl_cfg Pointer to struct to global configuration
 */
void cvmx_pki_write_global_config(int node, struct cvmx_pki_global_config *gbl_cfg);

/**
 * This function reads per pkind parameters in hardware which defines how
 * the incoming packet is processed.
 * 
 * @param node   Node number.
 * @param pkind  PKI supports a large number of incoming interfaces and packets
 *     arriving on different interfaces or channels may want to be processed
 *     differently. PKI uses the pkind to determine how the incoming packet
 *     is processed.
 * @param pkind_cfg	Pointer to struct conatining pkind configuration read
 *     from hardware.
 */
int cvmx_pki_read_pkind_config(int node, int pkind,
	struct cvmx_pki_pkind_config *pkind_cfg);

/**
 * This function writes per pkind parameters in hardware which defines how
 * the incoming packet is processed.
 *
 * @param node   Node number.
 * @param pkind  PKI supports a large number of incoming interfaces and packets
 *     arriving on different interfaces or channels may want to be processed
 *     differently. PKI uses the pkind to determine how the incoming packet
 *     is processed.
 * @param pkind_cfg	Pointer to struct conatining pkind configuration need
 *     to be written in hardware.
 */
int cvmx_pki_write_pkind_config(int node, int pkind,
	struct cvmx_pki_pkind_config *pkind_cfg);

/**
 * This function reads parameters associated with tag configuration in hardware.
 *
 * @param node	 Node number.
 * @param style  Style to configure tag for.
 * @param cluster_mask  Mask of clusters to configure the style for.
 * @param tag_cfg  Pointer to tag configuration struct.
 */
void cvmx_pki_read_tag_config(int node, int style, uint64_t cluster_mask,
	struct cvmx_pki_style_tag_cfg *tag_cfg);

/**
 * This function writes/configures parameters associated with tag
 * configuration in hardware.
 *
 * @param node  Node number.
 * @param style  Style to configure tag for.
 * @param cluster_mask  Mask of clusters to configure the style for.
 * @param tag_cfg  Pointer to taf configuration struct.
 */
void cvmx_pki_write_tag_config(int node, int style, uint64_t cluster_mask,
	struct cvmx_pki_style_tag_cfg *tag_cfg);

/**
 * This function reads parameters associated with style in hardware.
 *
 * @param node	Node number.
 * @param style  Style to read from.
 * @param cluster_mask  Mask of clusters style belongs to.
 * @param style_cfg  Pointer to style config struct.
 */
void cvmx_pki_read_style_config(int node, int style, uint64_t cluster_mask,
	struct cvmx_pki_style_config *style_cfg);

/**
 * This function writes/configures parameters associated with style in hardware.
 *
 * @param node  Node number.
 * @param style  Style to configure.
 * @param cluster_mask  Mask of clusters to configure the style for.
 * @param style_cfg  Pointer to style config struct.
 */
void cvmx_pki_write_style_config(int node, uint64_t style, uint64_t cluster_mask,
	struct cvmx_pki_style_config *style_cfg);
/**
 * This function reads qpg entry at specified offset from qpg table
 *
 * @param node  Node number.
 * @param offset  Offset in qpg table to read from.
 * @param qpg_cfg  Pointer to structure containing qpg values
 */
int cvmx_pki_read_qpg_entry(int node, int offset, struct cvmx_pki_qpg_config *qpg_cfg);

/**
 * This function writes qpg entry at specified offset in qpg table
 *
 * @param node  Node number.
 * @param offset  Offset in qpg table to write to.
 * @param qpg_cfg  Pointer to stricture containing qpg values.
 */
void cvmx_pki_write_qpg_entry(int node, int offset, struct cvmx_pki_qpg_config *qpg_cfg);

/**
 * This function writes pcam entry at given offset in pcam table in hardware
 *
 * @param node  Node number.
 * @param index	 Offset in pcam table.
 * @param cluster_mask  Mask of clusters in which to write pcam entry.
 * @param input  Input keys to pcam match passed as struct.
 * @param action  PCAM match action passed as struct
 */
int cvmx_pki_pcam_write_entry(int node, int index, uint64_t cluster_mask,
	struct cvmx_pki_pcam_input input, struct cvmx_pki_pcam_action action);
/**
 * Configures the channel which will receive backpressure from the specified bpid.
 * Each channel listens for backpressure on a specific bpid.
 * Each bpid can backpressure multiple channels.
 * @param node  Node number.
 * @param bpid  BPID from which channel will receive backpressure.
 * @param channel  Channel numner to receive backpressue.
 */
int cvmx_pki_write_channel_bpid(int node, int channel, int bpid);

/**
 * Configures the bpid on which, specified channel will
 * assert backpressure.
 * Each bpid receives backpressure from auras.
 * Multiple auras can backpressure single bpid.
 * @param node  Node number.
 * @param aura  Number which will assert backpressure on that bpid.
 * @param bpid  To assert backpressure on.
 */
int cvmx_pki_write_aura_bpid(int node, int aura, int bpid);

/**
 * Enables/Disabled QoS (RED Drop, Tail Drop & backpressure) for the* PKI aura.
 *
 * @param node  Node number
 * @param aura  To enable/disable QoS on.
 * @param ena_red  Enable/Disable RED drop between pass and drop level
 *    1-enable 0-disable
 * @param ena_drop  Enable/disable tail drop when max drop level exceeds
 *    1-enable 0-disable
 * @param ena_bp  Enable/Disable asserting backpressure on bpid when
 *    max DROP level exceeds.
 *    1-enable 0-disable
 */
int cvmx_pki_enable_aura_qos(int node, int aura, bool ena_red,
		bool ena_drop, bool ena_bp);

/**
 * This function gives the initial style used by that pkind.
 *
 * @param node  Node number.
 * @param pkind  PKIND number.
 */
int cvmx_pki_get_pkind_style(int node, int pkind);

/**
 * This function sets the wqe buffer mode. First packet data buffer can reside
 * either in same buffer as wqe OR it can go in separate buffer. If used the later mode,
 * make sure software allocate enough buffers to now have wqe separate from packet data.
 *
 * @param node  Node number.
 * @param style  Style to configure.
 * @param pkt_outside_wqe 
 *    0 = The packet link pointer will be at word [FIRST_SKIP] immediately
 *    followed by packet data, in the same buffer as the work queue entry.
 *    1 = The packet link pointer will be at word [FIRST_SKIP] in a new
 *    buffer separate from the work queue entry. Words following the
 *    WQE in the same cache line will be zeroed, other lines in the
 *    buffer will not be modified and will retain stale data (from the
 *    buffer’s previous use). This setting may decrease the peak PKI
 *    performance by up to half on small packets.
 */
void cvmx_pki_set_wqe_mode(int node, uint64_t style, bool pkt_outside_wqe);

/**
 * This function sets the Packet mode of all ports and styles to little-endian.
 * It Changes write operations of packet data to L2C to
 * be in little-endian. Does not change the WQE header format, which is
 * properly endian neutral.
 *
 * @param node  Node number.
 * @param style  Style to configure.
 */
void cvmx_pki_set_little_endian(int node, uint64_t style);

/**
 * Enables/Disables L2 length error check and max & min frame length checks.
 *
 * @param node  Node number.
 * @param pknd  PKIND to disable error for.
 * @param l2len_err	 L2 length error check enable.
 * @param maxframe_err	Max frame error check enable.
 * @param minframe_err	Min frame error check enable.
 *    1 -- Enabel err checks
 *    0 -- Disable error checks
 */
void cvmx_pki_endis_l2_errs(int node, int pknd, bool l2len_err,
			bool maxframe_err, bool minframe_err);

/**
 * Enables/Disables fcs check and fcs stripping on the pkind.
 *
 * @param node  Node number.
 * @param pknd  PKIND to apply settings on.
 * @param fcs_chk  Enable/disable fcs check.
 *    1 -- enable fcs error check.
 *    0 -- disable fcs error check.
 * @param fcs_strip	 Strip L2 FCS bytes from packet, decrease WQE[LEN] by 4 bytes
 *    1 -- strip L2 FCS.
 *    0 -- Do not strip L2 FCS.
 */
void cvmx_pki_endis_fcs_check(int node, int pknd, bool fcs_chk, bool fcs_strip);

/**
 * This function shows the qpg table entries, read directly from hardware.
 *
 * @param node  Node number.
 * @param num_entry  Number of entries to print.
 */
void cvmx_pki_show_qpg_entries(int node, uint16_t num_entry);

/**
 * This function shows the pcam table in raw format read directly from hardware.
 *
 * @param node  Node number.
 */
void cvmx_pki_show_pcam_entries(int node);

/**
 * This function shows the valid entries in readable format,
 * read directly from hardware.
 *
 * @param node  Node number.
 */
void cvmx_pki_show_valid_pcam_entries(int node);

/**
 * This function shows the pkind attributes in readable format,
 * read directly from hardware.
 * @param node  Node number.
 * @param pkind  PKIND number to print.
 */
void cvmx_pki_show_pkind_attributes(int node, int pkind);

/**
 * @INTERNAL
 * This function is called by cvmx_helper_shutdown() to extract all FPA buffers
 * out of the PKI. After this function completes, all FPA buffers that were
 * prefetched by PKI will be in the apropriate FPA pool.
 * This functions does not reset the PKI.
 * WARNING: It is very important that PKI be reset soon after a call to this function.
 *
 * @param node  Node number.
 */
void __cvmx_pki_free_ptr(int node);

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif

