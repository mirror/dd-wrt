/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#if defined(CONFIG_CPU_BIG_ENDIAN)
#include "appe_portvlan_reg_be.h"
#else

/**
 * @defgroup
 * @{
 */
#ifndef APPE_VLAN_REG_H
#define APPE_VLAN_REG_H

/*[register] IPR_VP_PARSING*/
#define IPR_VP_PARSING
#define IPR_VP_PARSING_ADDRESS 0x100
#define IPR_VP_PARSING_NUM     192
#define IPR_VP_PARSING_INC     0x4
#define IPR_VP_PARSING_TYPE    REG_TYPE_RW
#define IPR_VP_PARSING_DEFAULT 0x0
	/*[field] PORT_ROLE*/
	#define IPR_VP_PARSING_PORT_ROLE
	#define IPR_VP_PARSING_PORT_ROLE_OFFSET  0
	#define IPR_VP_PARSING_PORT_ROLE_LEN     1
	#define IPR_VP_PARSING_PORT_ROLE_DEFAULT 0x0
#if defined(MPPE)
	/*[field] SRC_PORT_SEL*/
	#define IPR_VP_PARSING_SRC_PORT_SEL
	#define IPR_VP_PARSING_SRC_PORT_SEL_OFFSET  1
	#define IPR_VP_PARSING_SRC_PORT_SEL_LEN     1
	#define IPR_VP_PARSING_SRC_PORT_SEL_DEFAULT 0x0
#endif

struct ipr_vp_parsing {
	a_uint32_t  port_role:1;
#if defined(MPPE)
	a_uint32_t  src_port_sel:1;
	a_uint32_t  _reserved0:30;
#else
	a_uint32_t  _reserved0:31;
#endif
};

union ipr_vp_parsing_u {
	a_uint32_t val;
	struct ipr_vp_parsing bf;
};

/*[table] VLAN_PORT_VP_TBL*/
#define VLAN_PORT_VP_TBL
#define VLAN_PORT_VP_TBL_ADDRESS 0xa000
#define VLAN_PORT_VP_TBL_NUM     256
#define VLAN_PORT_VP_TBL_INC     0x10
#define VLAN_PORT_VP_TBL_TYPE    REG_TYPE_RW
#define VLAN_PORT_VP_TBL_DEFAULT 0x0
	/*[field] PORT_DEF_SVID*/
	#define VLAN_PORT_VP_TBL_PORT_DEF_SVID
	#define VLAN_PORT_VP_TBL_PORT_DEF_SVID_OFFSET  0
	#define VLAN_PORT_VP_TBL_PORT_DEF_SVID_LEN     12
	#define VLAN_PORT_VP_TBL_PORT_DEF_SVID_DEFAULT 0x0
	/*[field] PORT_DEF_SVID_EN*/
	#define VLAN_PORT_VP_TBL_PORT_DEF_SVID_EN
	#define VLAN_PORT_VP_TBL_PORT_DEF_SVID_EN_OFFSET  12
	#define VLAN_PORT_VP_TBL_PORT_DEF_SVID_EN_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_DEF_SVID_EN_DEFAULT 0x0
	/*[field] PORT_DEF_CVID*/
	#define VLAN_PORT_VP_TBL_PORT_DEF_CVID
	#define VLAN_PORT_VP_TBL_PORT_DEF_CVID_OFFSET  13
	#define VLAN_PORT_VP_TBL_PORT_DEF_CVID_LEN     12
	#define VLAN_PORT_VP_TBL_PORT_DEF_CVID_DEFAULT 0x0
	/*[field] PORT_DEF_CVID_EN*/
	#define VLAN_PORT_VP_TBL_PORT_DEF_CVID_EN
	#define VLAN_PORT_VP_TBL_PORT_DEF_CVID_EN_OFFSET  25
	#define VLAN_PORT_VP_TBL_PORT_DEF_CVID_EN_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_DEF_CVID_EN_DEFAULT 0x0
	/*[field] PORT_DEF_SPCP*/
	#define VLAN_PORT_VP_TBL_PORT_DEF_SPCP
	#define VLAN_PORT_VP_TBL_PORT_DEF_SPCP_OFFSET  26
	#define VLAN_PORT_VP_TBL_PORT_DEF_SPCP_LEN     3
	#define VLAN_PORT_VP_TBL_PORT_DEF_SPCP_DEFAULT 0x0
	/*[field] PORT_DEF_SDEI*/
	#define VLAN_PORT_VP_TBL_PORT_DEF_SDEI
	#define VLAN_PORT_VP_TBL_PORT_DEF_SDEI_OFFSET  29
	#define VLAN_PORT_VP_TBL_PORT_DEF_SDEI_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_DEF_SDEI_DEFAULT 0x0
	/*[field] PORT_DEF_CPCP*/
	#define VLAN_PORT_VP_TBL_PORT_DEF_CPCP
	#define VLAN_PORT_VP_TBL_PORT_DEF_CPCP_OFFSET  30
	#define VLAN_PORT_VP_TBL_PORT_DEF_CPCP_LEN     3
	#define VLAN_PORT_VP_TBL_PORT_DEF_CPCP_DEFAULT 0x0
	/*[field] PORT_DEF_CDEI*/
	#define VLAN_PORT_VP_TBL_PORT_DEF_CDEI
	#define VLAN_PORT_VP_TBL_PORT_DEF_CDEI_OFFSET  33
	#define VLAN_PORT_VP_TBL_PORT_DEF_CDEI_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_DEF_CDEI_DEFAULT 0x0
	/*[field] PORT_IN_PCP_PROP_CMD*/
	#define VLAN_PORT_VP_TBL_PORT_IN_PCP_PROP_CMD
	#define VLAN_PORT_VP_TBL_PORT_IN_PCP_PROP_CMD_OFFSET  34
	#define VLAN_PORT_VP_TBL_PORT_IN_PCP_PROP_CMD_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_IN_PCP_PROP_CMD_DEFAULT 0x0
	/*[field] PORT_IN_DEI_PROP_CMD*/
	#define VLAN_PORT_VP_TBL_PORT_IN_DEI_PROP_CMD
	#define VLAN_PORT_VP_TBL_PORT_IN_DEI_PROP_CMD_OFFSET  35
	#define VLAN_PORT_VP_TBL_PORT_IN_DEI_PROP_CMD_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_IN_DEI_PROP_CMD_DEFAULT 0x0
	/*[field] PORT_UNTAG_FLTR_CMD*/
	#define VLAN_PORT_VP_TBL_PORT_UNTAG_FLTR_CMD
	#define VLAN_PORT_VP_TBL_PORT_UNTAG_FLTR_CMD_OFFSET  36
	#define VLAN_PORT_VP_TBL_PORT_UNTAG_FLTR_CMD_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_UNTAG_FLTR_CMD_DEFAULT 0x0
	/*[field] PORT_PRI_TAG_FLTR_CMD*/
	#define VLAN_PORT_VP_TBL_PORT_PRI_TAG_FLTR_CMD
	#define VLAN_PORT_VP_TBL_PORT_PRI_TAG_FLTR_CMD_OFFSET  37
	#define VLAN_PORT_VP_TBL_PORT_PRI_TAG_FLTR_CMD_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_PRI_TAG_FLTR_CMD_DEFAULT 0x0
	/*[field] PORT_TAG_FLTR_CMD*/
	#define VLAN_PORT_VP_TBL_PORT_TAG_FLTR_CMD
	#define VLAN_PORT_VP_TBL_PORT_TAG_FLTR_CMD_OFFSET  38
	#define VLAN_PORT_VP_TBL_PORT_TAG_FLTR_CMD_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_TAG_FLTR_CMD_DEFAULT 0x0
	/*[field] PORT_CVLAN_UNTAG_FLTR_CMD*/
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_UNTAG_FLTR_CMD
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_UNTAG_FLTR_CMD_OFFSET  39
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_UNTAG_FLTR_CMD_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_UNTAG_FLTR_CMD_DEFAULT 0x0
	/*[field] PORT_CVLAN_PRI_TAG_FLTR_CMD*/
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_PRI_TAG_FLTR_CMD
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_PRI_TAG_FLTR_CMD_OFFSET  40
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_PRI_TAG_FLTR_CMD_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_PRI_TAG_FLTR_CMD_DEFAULT 0x0
	/*[field] PORT_CVLAN_TAG_FLTR_CMD*/
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_TAG_FLTR_CMD
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_TAG_FLTR_CMD_OFFSET  41
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_TAG_FLTR_CMD_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_CVLAN_TAG_FLTR_CMD_DEFAULT 0x0
	/*[field] PORT_VLAN_XLT_MISS_FWD_CMD*/
	#define VLAN_PORT_VP_TBL_PORT_VLAN_XLT_MISS_FWD_CMD
	#define VLAN_PORT_VP_TBL_PORT_VLAN_XLT_MISS_FWD_CMD_OFFSET  42
	#define VLAN_PORT_VP_TBL_PORT_VLAN_XLT_MISS_FWD_CMD_LEN     2
	#define VLAN_PORT_VP_TBL_PORT_VLAN_XLT_MISS_FWD_CMD_DEFAULT 0x0
	/*[field] PORT_IN_VLAN_FLTR_CMD*/
	#define VLAN_PORT_VP_TBL_PORT_IN_VLAN_FLTR_CMD
	#define VLAN_PORT_VP_TBL_PORT_IN_VLAN_FLTR_CMD_OFFSET  44
	#define VLAN_PORT_VP_TBL_PORT_IN_VLAN_FLTR_CMD_LEN     1
	#define VLAN_PORT_VP_TBL_PORT_IN_VLAN_FLTR_CMD_DEFAULT 0x0
	/*[field] VLAN_PROFILE*/
	#define VLAN_PORT_VP_TBL_VLAN_PROFILE
	#define VLAN_PORT_VP_TBL_VLAN_PROFILE_OFFSET  45
	#define VLAN_PORT_VP_TBL_VLAN_PROFILE_LEN     8
	#define VLAN_PORT_VP_TBL_VLAN_PROFILE_DEFAULT 0x0

struct vlan_port_vp_tbl {
	a_uint32_t  port_def_svid:12;
	a_uint32_t  port_def_svid_en:1;
	a_uint32_t  port_def_cvid:12;
	a_uint32_t  port_def_cvid_en:1;
	a_uint32_t  port_def_spcp:3;
	a_uint32_t  port_def_sdei:1;
	a_uint32_t  port_def_cpcp_0:2;
	a_uint32_t  port_def_cpcp_1:1;
	a_uint32_t  port_def_cdei:1;
	a_uint32_t  port_in_pcp_prop_cmd:1;
	a_uint32_t  port_in_dei_prop_cmd:1;
	a_uint32_t  port_untag_fltr_cmd:1;
	a_uint32_t  port_pri_tag_fltr_cmd:1;
	a_uint32_t  port_tag_fltr_cmd:1;
	a_uint32_t  port_cvlan_untag_fltr_cmd:1;
	a_uint32_t  port_cvlan_pri_tag_fltr_cmd:1;
	a_uint32_t  port_cvlan_tag_fltr_cmd:1;
	a_uint32_t  port_vlan_xlt_miss_fwd_cmd:2;
	a_uint32_t  port_in_vlan_fltr_cmd:1;
	a_uint32_t  vlan_profile:8;
	a_uint32_t  _reserved0:11;
};

union vlan_port_vp_tbl_u {
	a_uint32_t val[2];
	struct vlan_port_vp_tbl bf;
};

/*[table] EG_VP_TBL*/
#define EG_VP_TBL
#define EG_VP_TBL_ADDRESS 0x5000
#define EG_VP_TBL_NUM     256
#define EG_VP_TBL_INC     0x10
#define EG_VP_TBL_TYPE    REG_TYPE_RW
#define EG_VP_TBL_DEFAULT 0x0
	/*[field] PORT_DEF_SVID*/
	#define EG_VP_TBL_PORT_DEF_SVID
	#define EG_VP_TBL_PORT_DEF_SVID_OFFSET  0
	#define EG_VP_TBL_PORT_DEF_SVID_LEN     12
	#define EG_VP_TBL_PORT_DEF_SVID_DEFAULT 0x0
	/*[field] PORT_DEF_SVID_EN*/
	#define EG_VP_TBL_PORT_DEF_SVID_EN
	#define EG_VP_TBL_PORT_DEF_SVID_EN_OFFSET  12
	#define EG_VP_TBL_PORT_DEF_SVID_EN_LEN     1
	#define EG_VP_TBL_PORT_DEF_SVID_EN_DEFAULT 0x0
	/*[field] PORT_DEF_CVID*/
	#define EG_VP_TBL_PORT_DEF_CVID
	#define EG_VP_TBL_PORT_DEF_CVID_OFFSET  13
	#define EG_VP_TBL_PORT_DEF_CVID_LEN     12
	#define EG_VP_TBL_PORT_DEF_CVID_DEFAULT 0x0
	/*[field] PORT_DEF_CVID_EN*/
	#define EG_VP_TBL_PORT_DEF_CVID_EN
	#define EG_VP_TBL_PORT_DEF_CVID_EN_OFFSET  25
	#define EG_VP_TBL_PORT_DEF_CVID_EN_LEN     1
	#define EG_VP_TBL_PORT_DEF_CVID_EN_DEFAULT 0x0
	/*[field] PORT_VLAN_TYPE*/
	#define EG_VP_TBL_PORT_VLAN_TYPE
	#define EG_VP_TBL_PORT_VLAN_TYPE_OFFSET  26
	#define EG_VP_TBL_PORT_VLAN_TYPE_LEN     1
	#define EG_VP_TBL_PORT_VLAN_TYPE_DEFAULT 0x0
	/*[field] PORT_EG_VLAN_CTAG_MODE*/
	#define EG_VP_TBL_PORT_EG_VLAN_CTAG_MODE
	#define EG_VP_TBL_PORT_EG_VLAN_CTAG_MODE_OFFSET  27
	#define EG_VP_TBL_PORT_EG_VLAN_CTAG_MODE_LEN     2
	#define EG_VP_TBL_PORT_EG_VLAN_CTAG_MODE_DEFAULT 0x0
	/*[field] PORT_EG_VLAN_STAG_MODE*/
	#define EG_VP_TBL_PORT_EG_VLAN_STAG_MODE
	#define EG_VP_TBL_PORT_EG_VLAN_STAG_MODE_OFFSET  29
	#define EG_VP_TBL_PORT_EG_VLAN_STAG_MODE_LEN     2
	#define EG_VP_TBL_PORT_EG_VLAN_STAG_MODE_DEFAULT 0x0
	/*[field] VSI_TAG_MODE_EN*/
	#define EG_VP_TBL_VSI_TAG_MODE_EN
	#define EG_VP_TBL_VSI_TAG_MODE_EN_OFFSET  31
	#define EG_VP_TBL_VSI_TAG_MODE_EN_LEN     1
	#define EG_VP_TBL_VSI_TAG_MODE_EN_DEFAULT 0x0
	/*[field] PORT_EG_PCP_PROP_CMD*/
	#define EG_VP_TBL_PORT_EG_PCP_PROP_CMD
	#define EG_VP_TBL_PORT_EG_PCP_PROP_CMD_OFFSET  32
	#define EG_VP_TBL_PORT_EG_PCP_PROP_CMD_LEN     1
	#define EG_VP_TBL_PORT_EG_PCP_PROP_CMD_DEFAULT 0x0
	/*[field] PORT_EG_DEI_PROP_CMD*/
	#define EG_VP_TBL_PORT_EG_DEI_PROP_CMD
	#define EG_VP_TBL_PORT_EG_DEI_PROP_CMD_OFFSET  33
	#define EG_VP_TBL_PORT_EG_DEI_PROP_CMD_LEN     1
	#define EG_VP_TBL_PORT_EG_DEI_PROP_CMD_DEFAULT 0x0
	/*[field] TX_COUNTING_EN*/
	#define EG_VP_TBL_TX_COUNTING_EN
	#define EG_VP_TBL_TX_COUNTING_EN_OFFSET  34
	#define EG_VP_TBL_TX_COUNTING_EN_LEN     1
	#define EG_VP_TBL_TX_COUNTING_EN_DEFAULT 0x0
	/*[field] XLAT_PROFILE*/
	#define EG_VP_TBL_XLAT_PROFILE
	#define EG_VP_TBL_XLAT_PROFILE_OFFSET  35
	#define EG_VP_TBL_XLAT_PROFILE_LEN     8
	#define EG_VP_TBL_XLAT_PROFILE_DEFAULT 0x0
	/*[field] TUNNEL_VALID*/
	#define EG_VP_TBL_TUNNEL_VALID
	#define EG_VP_TBL_TUNNEL_VALID_OFFSET  43
	#define EG_VP_TBL_TUNNEL_VALID_LEN     1
	#define EG_VP_TBL_TUNNEL_VALID_DEFAULT 0x0
	/*[field] TUNNEL_ID*/
	#define EG_VP_TBL_TUNNEL_ID
	#define EG_VP_TBL_TUNNEL_ID_OFFSET  44
	#define EG_VP_TBL_TUNNEL_ID_LEN     7
	#define EG_VP_TBL_TUNNEL_ID_DEFAULT 0x0
	/*[field] CNT_MODE*/
	#define EG_VP_TBL_CNT_MODE
	#define EG_VP_TBL_CNT_MODE_OFFSET  51
	#define EG_VP_TBL_CNT_MODE_LEN     1
	#define EG_VP_TBL_CNT_MODE_DEFAULT 0x0
#if defined(MPPE)
	/*[field] ATH_HDR_INSERT*/
	#define EG_VP_TBL_ATH_HDR_INSERT
	#define EG_VP_TBL_ATH_HDR_INSERT_OFFSET  52
	#define EG_VP_TBL_ATH_HDR_INSERT_LEN     1
	#define EG_VP_TBL_ATH_HDR_INSERT_DEFAULT 0x0
	/*[field] ATH_HDR_VER*/
	#define EG_VP_TBL_ATH_HDR_VER
	#define EG_VP_TBL_ATH_HDR_VER_OFFSET  53
	#define EG_VP_TBL_ATH_HDR_VER_LEN     2
	#define EG_VP_TBL_ATH_HDR_VER_DEFAULT 0x0
	/*[field] ATH_HDR_DEFAULT_TYPE*/
	#define EG_VP_TBL_ATH_HDR_DEFAULT_TYPE
	#define EG_VP_TBL_ATH_HDR_DEFAULT_TYPE_OFFSET  55
	#define EG_VP_TBL_ATH_HDR_DEFAULT_TYPE_LEN     3
	#define EG_VP_TBL_ATH_HDR_DEFAULT_TYPE_DEFAULT 0x0
	/*[field] ATH_PORT_BITMAP*/
	#define EG_VP_TBL_ATH_PORT_BITMAP
	#define EG_VP_TBL_ATH_PORT_BITMAP_OFFSET  58
	#define EG_VP_TBL_ATH_PORT_BITMAP_LEN     7
	#define EG_VP_TBL_ATH_PORT_BITMAP_DEFAULT 0x0
	/*[field] ATH_HDR_DISABLE_BIT*/
	#define EG_VP_TBL_ATH_HDR_DISABLE_BIT
	#define EG_VP_TBL_ATH_HDR_DISABLE_BIT_OFFSET  65
	#define EG_VP_TBL_ATH_HDR_DISABLE_BIT_LEN     1
	#define EG_VP_TBL_ATH_HDR_DISABLE_BIT_DEFAULT 0x0
	/*[field] ATH_HDR_FROM_CPU*/
	#define EG_VP_TBL_ATH_HDR_FROM_CPU
	#define EG_VP_TBL_ATH_HDR_FROM_CPU_OFFSET  66
	#define EG_VP_TBL_ATH_HDR_FROM_CPU_LEN     1
	#define EG_VP_TBL_ATH_HDR_FROM_CPU_DEFAULT 0x0
#endif

struct eg_vp_tbl {
	a_uint32_t  port_def_svid:12;
	a_uint32_t  port_def_svid_en:1;
	a_uint32_t  port_def_cvid:12;
	a_uint32_t  port_def_cvid_en:1;
	a_uint32_t  port_vlan_type:1;
	a_uint32_t  port_eg_vlan_ctag_mode:2;
	a_uint32_t  port_eg_vlan_stag_mode:2;
	a_uint32_t  vsi_tag_mode_en:1;
	a_uint32_t  port_eg_pcp_prop_cmd:1;
	a_uint32_t  port_eg_dei_prop_cmd:1;
	a_uint32_t  tx_counting_en:1;
	a_uint32_t  xlat_profile:8;
	a_uint32_t  tunnel_valid:1;
	a_uint32_t  tunnel_id:7;
	a_uint32_t  cnt_mode:1;
#if defined(MPPE)
	a_uint32_t  ath_hdr_insert:1;
	a_uint32_t  ath_hdr_ver:2;
	a_uint32_t  ath_hdr_default_type:3;
	a_uint32_t  ath_port_bitmap_0:6;
	a_uint32_t  ath_port_bitmap_1:1;
	a_uint32_t  ath_hdr_disable_bit:1;
	a_uint32_t  ath_hdr_from_cpu:1;
	a_uint32_t  _reserved0:29;
#else
	a_uint32_t  _reserved0:12;
#endif
};

union eg_vp_tbl_u {
#if defined(MPPE)
	a_uint32_t val[3];
#else
	a_uint32_t val[2];
#endif
	struct eg_vp_tbl bf;
};

/*[table] EG_VSI_VP_TAG*/
#define EG_VSI_VP_TAG
#define EG_VSI_VP_TAG_ADDRESS 0x8000
#define EG_VSI_VP_TAG_NUM     2048
#define EG_VSI_VP_TAG_INC     0x10
#define EG_VSI_VP_TAG_TYPE    REG_TYPE_RW
#define EG_VSI_VP_TAG_DEFAULT 0x0
	/*[field] TAGGED_MODE_VP_BITMAP*/
	#define EG_VSI_VP_TAG_TAGGED_MODE_VP_BITMAP
	#define EG_VSI_VP_TAG_TAGGED_MODE_VP_BITMAP_OFFSET  0
	#define EG_VSI_VP_TAG_TAGGED_MODE_VP_BITMAP_LEN     16
	#define EG_VSI_VP_TAG_TAGGED_MODE_VP_BITMAP_DEFAULT 0x0

struct eg_vsi_vp_tag {
	a_uint32_t  tagged_mode_vp_bitmap:16;
	a_uint32_t  _reserved0:16;
};

union eg_vsi_vp_tag_u {
	a_uint32_t val;
	struct eg_vsi_vp_tag bf;
};

/*[register] TPR_PORT_PARSING*/
#define TPR_PORT_PARSING
#define TPR_PORT_PARSING_ADDRESS 0x0
#define TPR_PORT_PARSING_NUM     8
#define TPR_PORT_PARSING_INC     0x4
#define TPR_PORT_PARSING_TYPE    REG_TYPE_RW
#define TPR_PORT_PARSING_DEFAULT 0x0
	/*[field] PORT_ROLE*/
	#define TPR_PORT_PARSING_PORT_ROLE
	#define TPR_PORT_PARSING_PORT_ROLE_OFFSET  0
	#define TPR_PORT_PARSING_PORT_ROLE_LEN     1
	#define TPR_PORT_PARSING_PORT_ROLE_DEFAULT 0x0

struct tpr_port_parsing {
	a_uint32_t  port_role:1;
	a_uint32_t  _reserved0:31;
};

union tpr_port_parsing_u {
	a_uint32_t val;
	struct tpr_port_parsing bf;
};

/*[register] TPR_VLAN_TPID*/
#define TPR_VLAN_TPID
#define TPR_VLAN_TPID_ADDRESS 0x20
#define TPR_VLAN_TPID_NUM     1
#define TPR_VLAN_TPID_INC     0x4
#define TPR_VLAN_TPID_TYPE    REG_TYPE_RW
#define TPR_VLAN_TPID_DEFAULT 0x88a88100
	/*[field] CTAG_TPID*/
	#define TPR_VLAN_TPID_CTAG_TPID
	#define TPR_VLAN_TPID_CTAG_TPID_OFFSET  0
	#define TPR_VLAN_TPID_CTAG_TPID_LEN     16
	#define TPR_VLAN_TPID_CTAG_TPID_DEFAULT 0x8100
	/*[field] STAG_TPID*/
	#define TPR_VLAN_TPID_STAG_TPID
	#define TPR_VLAN_TPID_STAG_TPID_OFFSET  16
	#define TPR_VLAN_TPID_STAG_TPID_LEN     16
	#define TPR_VLAN_TPID_STAG_TPID_DEFAULT 0x88a8

struct tpr_vlan_tpid {
	a_uint32_t  ctag_tpid:16;
	a_uint32_t  stag_tpid:16;
};

union tpr_vlan_tpid_u {
	a_uint32_t val;
	struct tpr_vlan_tpid bf;
};

/*[table] VP_ISOL_TBL*/
#define VP_ISOL_TBL
#define VP_ISOL_TBL_ADDRESS 0x3c000
#define VP_ISOL_TBL_NUM     64
#define VP_ISOL_TBL_INC     0x10
#define VP_ISOL_TBL_TYPE    REG_TYPE_RW
#define VP_ISOL_TBL_DEFAULT 0x0
	/*[field] VP_PROFILE_MAP*/
	#define VP_ISOL_TBL_VP_PROFILE_MAP
	#define VP_ISOL_TBL_VP_PROFILE_MAP_OFFSET  0
	#define VP_ISOL_TBL_VP_PROFILE_MAP_LEN     64
	#define VP_ISOL_TBL_VP_PROFILE_MAP_DEFAULT 0x0

struct vp_isol_tbl {
	a_uint32_t  vp_profile_map_0:32;
	a_uint32_t  vp_profile_map_1:32;
};

union vp_isol_tbl_u {
	a_uint32_t val[2];
	struct vp_isol_tbl bf;
};
#endif
#endif
