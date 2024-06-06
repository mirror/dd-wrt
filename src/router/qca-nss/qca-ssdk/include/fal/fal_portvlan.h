/*
 * Copyright (c) 2012, 2016-2018, 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */



/**
 * @defgroup fal_port_vlan FAL_PORT_VLAN
 * @{
 */
#ifndef _FAL_PORT_VLAN_H_
#define _FAL_PORT_VLAN_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "sw.h"
#include "fal_type.h"

#if defined(SW_API_LOCK) && (!defined(HSL_STANDALONG))
#define FAL_PORTVLAN_API_LOCK FAL_API_LOCK
#define FAL_PORTVLAN_API_UNLOCK FAL_API_UNLOCK
#else
#define FAL_PORTVLAN_API_LOCK
#define FAL_PORTVLAN_API_UNLOCK
#endif

#define FAL_DEF_VLAN_STPID 0x88a8
#define FAL_DEF_VLAN_CTPID 0x8100

/*
* Vlan frame type match in vlan translation
*/
#define FAL_PORT_VLAN_XLT_MATCH_UNTAGGED    0x1
#define FAL_PORT_VLAN_XLT_MATCH_PRIO_TAG    0x2
#define FAL_PORT_VLAN_XLT_MATCH_TAGGED      0x4

/**
  @brief This enum defines 802.1q mode type.
  */
typedef enum {
	FAL_1Q_DISABLE = 0, /* 802.1q mode disbale, port based vlan */
	FAL_1Q_SECURE,      /* secure mode, packets which vid isn't in vlan table or
			     * source port isn't in vlan port member will be discarded.
			     */
	FAL_1Q_CHECK,       /* check mode, packets which vid isn't in vlan table will be
			     * discarded, packets which source port isn't in vlan port member
			     * will forward base on vlan port member
			     */
	FAL_1Q_FALLBACK,    /* fallback mode, packets which vid isn't in vlan table will
			     * forwarded base on port vlan, packet's which source port isn't
			     * in vlan port member will forward base on vlan port member.
			     */
	FAL_1Q_MODE_BUTT
} fal_pt_1qmode_t;

/**
  @brief This enum defines receive packets tagged mode.
  */
typedef enum
{
	FAL_INVLAN_ADMIT_ALL = 0,  /**<  receive all packets include tagged and untagged */
	FAL_INVLAN_ADMIT_TAGGED,   /**<  only receive tagged packets*/
	FAL_INVLAN_ADMIT_UNTAGGED, /**<  only receive untagged packets include priority tagged */
	FAL_INVLAN_MODE_BUTT
} fal_pt_invlan_mode_t;

/**
  @brief This enum defines vlan propagation mode.
  */
typedef enum
{
	FAL_VLAN_PROPAGATION_DISABLE = 0, /**<  vlan propagation disable */
	FAL_VLAN_PROPAGATION_CLONE,       /**<  vlan paopagation mode is clone */
	FAL_VLAN_PROPAGATION_REPLACE,     /**<  vlan paopagation mode is repalce */
	FAL_VLAN_PROPAGATION_MODE_BUTT
} fal_vlan_propagation_mode_t;

typedef enum
{
	FAL_FRAMETYPE_ETHERNET = 0,
	FAL_FRAMETYPE_RFC_1024 = 1,
	FAL_FRAMETYPE_LLC_OTHER = 2,
	FAL_FRAMETYPE_ETHORRFC1024 = 3,
} fal_frametype_t;

typedef enum
{
	FAL_VID_XLT_CMD_UNCHANGED = 0,
	FAL_VID_XLT_CMD_ADDORREPLACE = 1,
	FAL_VID_XLT_CMD_DELETE = 2,
} fal_vid_xlt_cmd_t;

typedef enum
{
	FAL_PORT_VLAN_ALL = 0,
	FAL_PORT_VLAN_INGRESS = 1,
	FAL_PORT_VLAN_EGRESS = 2,
} fal_port_vlan_direction_t;


/**
  @details  Fields description:

  o_vid - original vlan id
  s_vid - service vid id
  c_vid - custom vid id
  bi_dir - entry search direction
  forward_dir - entry search direction only be forward
  reverse_dir - entry search direction only be reverse
  o_vid_is_cvid - o_vid in entry means c_vid not s_vid
  s_vid_enable  - s_vid in entry is valid
  c_vid_enable  - c_vid in entry is valid
  one_2_one_vlan- the entry used for 1:1 vlan
  @brief This structure defines the vlan translation entry.

*/

typedef struct
{
	a_uint32_t o_vid;        /* original vid */
	a_uint32_t s_vid;        /* service vid */
	a_uint32_t c_vid;        /* customer vid */
	a_bool_t   bi_dir;       /* lookup can be forward and reverse */
	a_bool_t   forward_dir;  /* lookup direction only can be from o_vid to s_vid and/or c_vid*/
	a_bool_t   reverse_dir;  /* lookup direction only can be from s_vid and/or c_vid to o_vid*/
	a_bool_t   o_vid_is_cvid;
	a_bool_t   s_vid_enable; /* enable svid value or not */
	a_bool_t   c_vid_enable; /* enable cvid value or not */
	a_bool_t   one_2_one_vlan;

	/*direction check*/
	a_uint32_t trans_direction; /* 0 is for ingress, 1 is for egress */

	/*vsi check*/
	a_bool_t   vsi_valid; /* check if rule will include vsi value valid */
	a_bool_t   vsi_enable; /* check if rule will include vsi value */
	a_uint32_t   vsi; /* vsi value */

	/*protocol and ethernet type*/
	a_bool_t   protocol_enable; /* enable protocol value or not */
	a_uint16_t   protocol; /* protocol value */
	a_bool_t   frmtype_enable; /* enable frame type value or not */
	fal_frametype_t   frmtype; /* frame type value */

	/*tagged mode, bit 0 for untagged, bit 1 for priority tagged and bit 2 for tagged*/
	a_uint8_t   s_tagged; /* stag type is untagged/pri tagged/tagged */
	a_uint8_t   c_tagged; /* ctag type is untagged/pri tagged/tagged */

	/*cpcp and cdei*/
	a_bool_t   c_pcp_enable; /* check if rule will include customer pcp value */
	a_uint8_t   c_pcp; /* customer pcp value */
	a_bool_t   c_dei_enable; /* check if rule will include customer dei value */
	a_uint8_t   c_dei; /* customer dei value */
	/*spcp and sdei*/
	a_bool_t   s_pcp_enable; /* check if rule will include service pcp value */
	a_uint8_t   s_pcp; /* service pcp value */
	a_bool_t   s_dei_enable; /* check if rule will include service dei value */
	a_uint8_t   s_dei; /* service dei value */

	/*translation action*/
	/*counter action*/
	a_bool_t   counter_enable; /* check if action will enable counter_id */
	a_uint8_t   counter_id;  /* counter id */
	/*vsi action*/
	a_bool_t   vsi_action_enable; /* check if action will enable vsi action */
	a_uint8_t   vsi_action; /* vsi action */
	/*svid action*/
	fal_vid_xlt_cmd_t   svid_xlt_cmd; /* check if action will do svid xlt */
	a_uint16_t   svid_xlt; /* svid xlt operation */
	/*cvid action*/
	fal_vid_xlt_cmd_t   cvid_xlt_cmd; /* check if action will do cvid xlt */
	a_uint16_t   cvid_xlt; /* cvid xlt operation */
	/*swap svid and cvid action*/
	a_bool_t   swap_svid_cvid; /* check if action will do svid and cvid swap operation */
	/*spcp action*/
	a_bool_t   spcp_xlt_enable; /* check if action will enable spcp xlt */
	a_uint8_t   spcp_xlt; /* spcp xlt operation */
	/*cpcp action*/
	a_bool_t   cpcp_xlt_enable; /* check if action will enable cpcp xlt */
	a_uint8_t   cpcp_xlt; /* cpcp xlt operation */
	/*swap spcp and cpcp action*/
	a_bool_t   swap_spcp_cpcp; /* check if action will do spcp and cpcp swap operation */
	/*sdei action*/
	a_bool_t   sdei_xlt_enable; /* check if action will enable sdei xlt */
	a_uint8_t   sdei_xlt; /* sdei xlt operation */
	/*cdei action*/
	a_bool_t   cdei_xlt_enable; /* check if action will enable cdei xlt */
	a_uint8_t   cdei_xlt; /* cdei xlt operation */
	/*swap sdei and cdei action*/
	a_bool_t   swap_sdei_cdei; /* check if action will do sdei and cdei swap operation */

	/*port bitmap for this entry*/
	a_uint32_t port_bitmap; /* rule must be include port_bitmap */
} fal_vlan_trans_entry_t;

/**
  @brief This enum defines qinq working mode.
  */
typedef enum
{
	FAL_QINQ_CTAG_MODE = 0,
	FAL_QINQ_STAG_MODE,
	FAL_QINQ_MODE_BUTT
} fal_qinq_mode_t;

/**
  @brief This enum defines port role in qinq mode.
  */
typedef enum
{
	FAL_QINQ_EDGE_PORT = 0,
	FAL_QINQ_CORE_PORT,
	FAL_QINQ_PORT_ROLE_BUTT
} fal_qinq_port_role_t;

/**
  @brief This enum defines which port to use for qinq role.
  */
typedef enum
{
	FAL_QINQ_SEL_TNL_DECAP_SRC_VP = 0,
	FAL_QINQ_SEL_ORG_SRC_PORT,
	FAL_QINQ_SEL_PORT_TYPE_BUTT
} fal_qinq_port_sel_t;

#define FAL_FLG_TST(flag, field) \
    (((flag) & (field)) ? 1 : 0)

#define FAL_GLOBAL_QINQ_MODE_INGRESS_EN (0x1UL << 0)
#define FAL_GLOBAL_QINQ_MODE_EGRESS_EN (0x1UL << 1)
#define FAL_GLOBAL_QINQ_MODE_EGRESS_UNTOUCHED_FOR_CPU_CODE (0x1UL << 2)
	typedef struct {
	    a_uint32_t mask;/*bit 0 for ingress and bit 1 for egress*/
	    fal_qinq_mode_t ingress_mode; /* ingress direction mode */
	    fal_qinq_mode_t egress_mode; /* egress direction mode */
	    a_bool_t untouched_for_cpucode; /*egress untouched with cpu_code!=0 to cpu port 0 */
	} fal_global_qinq_mode_t;

#define FAL_PORT_QINQ_ROLE_INGRESS_EN (0x1UL << 0)
#define FAL_PORT_QINQ_ROLE_EGRESS_EN (0x1UL << 1)
#define FAL_PORT_QINQ_ROLE_TUNNEL_EN (0x1UL << 2)
typedef struct {
	a_uint32_t mask;/*bit 0 for ingress and bit 1 for egress*/
	fal_qinq_port_role_t ingress_port_role; /* port inress direction role */
	fal_qinq_port_role_t egress_port_role; /* port egress direction role */
	fal_qinq_port_role_t tunnel_port_role; /* tunnel ingress parse role
						* added for ipq95xx */
	fal_qinq_port_sel_t ingress_port_sel; /*which port used for ingress_port_role in tunnel decap scenario.
						* 0 is used to selcet srcvp of tunnel decap, 1 always selet original source port.
						* default: 0, will use srcvp of tunnel decap.
						* added for ipq5332*/
} fal_port_qinq_role_t;

#define FAL_TPID_CTAG_EN (0x1UL << 0)
#define FAL_TPID_STAG_EN (0x1UL << 1)
#define FAL_TUNNEL_TPID_CTAG_EN (0x1UL << 2)
#define FAL_TUNNEL_TPID_STAG_EN (0x1UL << 3)
typedef struct
{
	a_uint32_t mask; /* bit 0 for ctpid and bit 1 for stpid
			  * bit 2 for tunnel ctpid, bit 3 for tunnel stpid
			  * */
	a_uint16_t ctpid; /* customer tpid value */
	a_uint16_t stpid; /* service tpid value */
	a_uint16_t tunnel_ctpid; /* tunnel customer tpid value, added for ipq95xx */
	a_uint16_t tunnel_stpid; /* tunnel service tpid value, added for ipq95xx */
} fal_tpid_t;

typedef struct {
	a_bool_t membership_filter; /* check if ingress will filter port in vlan filter */
	a_bool_t tagged_filter; /* check if ingress will filter tagged packet */
	a_bool_t untagged_filter; /* check if ingress will filter untagged packet */
	a_bool_t priority_filter; /* check if ingress will filter priority packet */
	a_bool_t ctag_tagged_filter; /* ingress filter ctag tagged packet or not,
				      * added for ipq95xx */
	a_bool_t ctag_untagged_filter; /* ingress filter ctag untagged packet or not,
					* added for ipq95xx */
	a_bool_t ctag_priority_filter; /* ingress filter ctag priority packet or not,
					* added for ipq95xx */
} fal_ingress_vlan_filter_t;

#define FAL_PORT_VLAN_TAG_CVID_EN (0x1UL << 0)
#define FAL_PORT_VLAN_TAG_SVID_EN (0x1UL << 1)
#define FAL_PORT_VLAN_TAG_CPCP_EN (0x1UL << 2)
#define FAL_PORT_VLAN_TAG_SPCP_EN (0x1UL << 3)
#define FAL_PORT_VLAN_TAG_CDEI_EN (0x1UL << 4)
#define FAL_PORT_VLAN_TAG_SDEI_EN (0x1UL << 5)
typedef struct
{
	a_uint32_t mask;	/*bit 0 for ctag vid;
				 *bit 1 for stag vid;
				 *bit 2 for ctag priority;
				 *bit 3 for stag priority;
				 *bit 4 for ctag dei;
				 *bit 5 for stag dei*/
	a_uint16_t cvid; /* customer vid value */
	a_uint16_t svid; /* service vid value */
	a_uint16_t cpri; /* customer pri value */
	a_uint16_t spri; /* service pri value */
	a_uint16_t cdei; /* customer dei value */
	a_uint16_t sdei; /* service dei value */
} fal_port_vlan_tag_t;

typedef struct {
	a_bool_t default_cvid_en; /* enable default cvid or not */
	a_bool_t default_svid_en; /* enable default svid or not */
} fal_port_default_vid_enable_t;

#define FAL_PORT_PROPAGATION_VID_EN (0x1UL << 0)
#define FAL_PORT_PROPAGATION_PCP_EN (0x1UL << 1)
#define FAL_PORT_PROPAGATION_DEI_EN (0x1UL << 2)
typedef struct
{
	a_uint32_t mask; /*bit 0 for vid;
			  *bit 1 for priority;
			  *bit 2 for dei*/
	fal_vlan_propagation_mode_t vid_propagation; /* enable vid propagation or not */
	fal_vlan_propagation_mode_t pri_propagation; /* enable pri propagation or not */
	fal_vlan_propagation_mode_t dei_propagation; /* enable dei propagation or not */
} fal_vlantag_propagation_t;

#define FAL_EGRESSMODE_CTAG_EN (0x1UL << 0)
#define FAL_EGRESSMODE_STAG_EN (0x1UL << 1)
typedef struct
{
	a_uint32_t mask; /*bit 0 for ctag and bit 1 for stag*/
	fal_pt_1q_egmode_t stag_mode; /* stag mode */
	fal_pt_1q_egmode_t ctag_mode; /* ctag mode */
} fal_vlantag_egress_mode_t;

typedef struct
{
	fal_pbmp_t port_bitmap; /* rule need know which ports matched this rule */
	a_uint8_t s_tagged; /* rule need know stag type(untagged/pri_tagged/tagged) */
	a_bool_t s_vid_enable; /* check if rule will include service vid value */
	a_uint32_t s_vid; /* service vid */
	a_bool_t s_pcp_enable; /* check if rule will include service pcp value */
	a_uint8_t s_pcp; /* service pcp value */
	a_bool_t s_dei_enable; /* check if rule will include service dei value */
	a_uint8_t s_dei; /* service dei value */
	a_uint8_t c_tagged; /* rule need know ctag type(untagged/pri_tagged/tagged) */
	a_bool_t c_vid_enable; /* check if rule will include customer vid value */
	a_uint32_t c_vid; /* customer vid */
	a_bool_t c_pcp_enable; /* check if rule will include customer pcp value */
	a_uint8_t c_pcp; /* customer pcp value */
	a_bool_t c_dei_enable; /* check if rule will include customer dei value */
	a_uint8_t c_dei; /* customert dei value */

	/* these four fields just for vlan ingress rule */
	a_bool_t frmtype_enable; /* check if rule will include frame type value */
	fal_frametype_t	frmtype; /* frame type value */
	a_bool_t protocol_enable; /* check if rule will include protocol value */
	a_uint16_t protocol; /* protocol value */

	/* these three fields just for vlan egress rule */
	a_bool_t vsi_valid; /* check if rule will include vsi value valid */
	a_bool_t vsi_enable; /* check if rule will include vsi value */
	a_uint32_t vsi; /* vsi value */

	/*vni fields match for ingress added by appe*/
	a_bool_t vni_resv_enable; /* check vni resv or not, added for ipq95xx */
	a_uint8_t vni_resv_type;  /* 0 for vni only, 1 for vni and reserver,
				   * added for ipq95xx
				   * */
	a_uint32_t vni_resv; /* vni or gre key filed value, added for ipq95xx*/
} fal_vlan_trans_adv_rule_t;

typedef struct
{
	a_bool_t swap_svid_cvid; /* check if action will do svid and cvid swap operation */
	fal_vid_xlt_cmd_t svid_xlt_cmd; /* check if action will do svid xlt operation */
	a_uint16_t svid_xlt; /* service vid xlt value */
	fal_vid_xlt_cmd_t cvid_xlt_cmd; /* check if action will do cvid xlt operation */
	a_uint16_t cvid_xlt; /* customer vid xlt value */
	a_bool_t swap_spcp_cpcp; /* check if action will do spcp and cpcp swap operation */
	a_bool_t spcp_xlt_enable; /* check if action will enable spcp xlt */
	a_uint8_t spcp_xlt; /* service pcp xlt value */
	a_bool_t cpcp_xlt_enable; /* check if action will enable cpcp xlt */
	a_uint8_t cpcp_xlt; /* customer pcp xlt value */
	a_bool_t swap_sdei_cdei; /* check if action will do sdei and cdei swap operation */
	a_bool_t sdei_xlt_enable; /* check if action will enable sdei xlt */
	a_uint8_t sdei_xlt; /* service dei xlt value */
	a_bool_t cdei_xlt_enable; /* check if action will enable cdei xlt */
	a_uint8_t cdei_xlt; /* customer dei xlt value */
	a_bool_t counter_enable; /* check if action will enable counter_id */
	a_uint8_t counter_id;  /* counter id */
	/* these two fields just for vlan ingress action */
	a_bool_t vsi_xlt_enable; /* check if action will enable vsi xlt */
	a_uint8_t vsi_xlt; /* vsi xlt value */
	/*src info fields for ingress added by appe*/
	a_bool_t src_info_enable; /* src info xlt or not, added for ipq95xx*/
	a_uint8_t src_info_type; /* 0 virtual port, 1 l3_if for tunnel payload,
				  * added for ipq95xx */
	a_uint32_t src_info; /* src info value, added for ipq95xx */
	/*vni fields for egress added by appe*/
	a_bool_t vni_resv_enable; /* vni xlat or not, added for ipq95xx*/
	a_uint32_t vni_resv;	/* vni xlt value, added for ipq95xx*/
} fal_vlan_trans_adv_action_t;

typedef struct
{
	a_uint32_t rx_packet_counter; /* ingress vlan translation pkt counter */
	a_uint64_t rx_byte_counter; /* ingress vlan translation byte counter */
	a_uint32_t tx_packet_counter; /* egress vlan translation pkt counter */
	a_uint64_t tx_byte_counter; /* egress vlan translation byte counter */
} fal_port_vlan_counter_t;

typedef struct {
	a_bool_t enable; /*enable or not */
	a_uint8_t group_id; /* isolation group id */
} fal_portvlan_isol_ctrl_t;

typedef struct {
	a_bool_t membership_filter; /* membership filter or not for vport */
} fal_egress_vlan_filter_t;

#ifndef IN_PORTVLAN_MINI
sw_error_t
fal_port_force_default_vid_set(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t enable);

sw_error_t
fal_port_force_portvlan_set(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t enable);

sw_error_t
fal_port_force_default_vid_get(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t * enable);

sw_error_t
fal_port_force_portvlan_get(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t * enable);

sw_error_t
fal_port_tls_get(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t * enable);

sw_error_t
fal_port_pri_propagation_set(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t enable);

sw_error_t
fal_port_pri_propagation_get(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t * enable);

sw_error_t
fal_port_vlan_propagation_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_vlan_propagation_mode_t * mode);

sw_error_t
fal_port_vlan_trans_add(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry);

sw_error_t
fal_port_vlan_trans_del(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry);

sw_error_t
fal_port_vlan_trans_get(a_uint32_t dev_id, fal_port_t port_id, fal_vlan_trans_entry_t *entry);

sw_error_t
fal_port_vlan_trans_iterate(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t * iterator, fal_vlan_trans_entry_t * entry);

sw_error_t
fal_port_mac_vlan_xlt_set(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t enable);

sw_error_t
fal_port_mac_vlan_xlt_get(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t * enable);

sw_error_t
fal_netisolate_set(a_uint32_t dev_id, a_uint32_t enable);

sw_error_t
fal_netisolate_get(a_uint32_t dev_id, a_uint32_t * enable);

sw_error_t
fal_eg_trans_filter_bypass_en_set(a_uint32_t dev_id, a_uint32_t enable);

sw_error_t
fal_eg_trans_filter_bypass_en_get(a_uint32_t dev_id, a_uint32_t * enable);

sw_error_t
fal_port_vrf_id_set(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t vrf_id);

sw_error_t
fal_port_vrf_id_get(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t * vrf_id);

sw_error_t
fal_port_egress_vlan_filter_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_egress_vlan_filter_t *filter);
sw_error_t
fal_port_egress_vlan_filter_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_egress_vlan_filter_t *filter);
sw_error_t
fal_portvlan_isol_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_portvlan_isol_ctrl_t *isol_ctrl);
sw_error_t
fal_portvlan_isol_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_portvlan_isol_ctrl_t *isol_ctrl);
sw_error_t
fal_portvlan_isol_group_set(a_uint32_t dev_id,
		a_uint8_t isol_group_id, a_uint64_t *isol_group_bmp);
sw_error_t
fal_portvlan_isol_group_get(a_uint32_t dev_id,
		a_uint8_t isol_group_id, a_uint64_t *isol_group_bmp);
sw_error_t
fal_port_vlan_counter_get(a_uint32_t dev_id, a_uint32_t cnt_index,
		fal_port_vlan_counter_t * counter);
sw_error_t
fal_port_vlan_counter_cleanup(a_uint32_t dev_id, a_uint32_t cnt_index);

sw_error_t
fal_port_tag_propagation_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_port_vlan_direction_t direction,
		fal_vlantag_propagation_t *prop);

sw_error_t
fal_port_tag_propagation_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_port_vlan_direction_t direction,
		fal_vlantag_propagation_t *prop);

sw_error_t
fal_port_vlan_vpgroup_set(a_uint32_t dev_id, a_uint32_t vport,
		fal_port_vlan_direction_t direction, a_uint32_t vpgroup_id);

sw_error_t
fal_port_vlan_vpgroup_get(a_uint32_t dev_id, a_uint32_t vport,
		fal_port_vlan_direction_t direction, a_uint32_t *vpgroup_id);
#endif

sw_error_t
fal_port_vlan_trans_adv_add(a_uint32_t dev_id, fal_port_t port_id,
		fal_port_vlan_direction_t direction,
		fal_vlan_trans_adv_rule_t * rule, fal_vlan_trans_adv_action_t * action);

sw_error_t
fal_port_vlan_trans_adv_del(a_uint32_t dev_id, fal_port_t port_id,
		fal_port_vlan_direction_t direction,
		fal_vlan_trans_adv_rule_t * rule, fal_vlan_trans_adv_action_t * action);

sw_error_t
fal_port_vlan_trans_adv_getfirst(a_uint32_t dev_id, fal_port_t port_id,
		fal_port_vlan_direction_t direction,
		fal_vlan_trans_adv_rule_t * rule, fal_vlan_trans_adv_action_t * action);

sw_error_t
fal_port_vlan_trans_adv_getnext(a_uint32_t dev_id, fal_port_t port_id,
		fal_port_vlan_direction_t direction,
		fal_vlan_trans_adv_rule_t * rule, fal_vlan_trans_adv_action_t * action);

sw_error_t
fal_portvlan_member_update(a_uint32_t dev_id, fal_port_t port_id,
		fal_pbmp_t mem_port_map);

sw_error_t
fal_portvlan_member_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_pbmp_t * mem_port_map);

sw_error_t
fal_port_vlantag_egmode_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_vlantag_egress_mode_t *port_egvlanmode);

sw_error_t
fal_port_vlantag_egmode_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_vlantag_egress_mode_t *port_egvlanmode);

sw_error_t
fal_port_vlantag_vsi_egmode_enable(a_uint32_t dev_id,
		fal_port_t port_id, a_bool_t enable);

sw_error_t
fal_port_vlantag_vsi_egmode_status_get(a_uint32_t dev_id,
		fal_port_t port_id, a_bool_t * enable);

sw_error_t
fal_port_vlan_xlt_miss_cmd_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_fwd_cmd_t cmd);

sw_error_t
fal_port_vlan_xlt_miss_cmd_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_fwd_cmd_t *cmd);

sw_error_t
fal_port_qinq_mode_set(a_uint32_t dev_id, fal_port_t port_id, fal_port_qinq_role_t *mode);

sw_error_t
fal_port_qinq_mode_get(a_uint32_t dev_id, fal_port_t port_id, fal_port_qinq_role_t *mode);

sw_error_t
fal_ingress_tpid_set(a_uint32_t dev_id, fal_tpid_t *tpid);

sw_error_t
fal_ingress_tpid_get(a_uint32_t dev_id, fal_tpid_t *tpid);

sw_error_t
fal_egress_tpid_set(a_uint32_t dev_id, fal_tpid_t *tpid);

sw_error_t
fal_egress_tpid_get(a_uint32_t dev_id, fal_tpid_t *tpid);

sw_error_t
fal_port_vlan_propagation_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_vlan_propagation_mode_t mode);

sw_error_t
fal_port_egvlanmode_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_pt_1q_egmode_t port_egvlanmode);

sw_error_t
fal_port_egvlanmode_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_pt_1q_egmode_t * pport_egvlanmode);

sw_error_t
fal_global_qinq_mode_set(a_uint32_t dev_id, fal_global_qinq_mode_t *mode);

sw_error_t
fal_global_qinq_mode_get(a_uint32_t dev_id, fal_global_qinq_mode_t *mode);

sw_error_t
fal_port_vsi_egmode_set(a_uint32_t dev_id, a_uint32_t vsi,
		a_uint32_t port_id, fal_pt_1q_egmode_t egmode);

sw_error_t
fal_port_vsi_egmode_get(a_uint32_t dev_id, a_uint32_t vsi,
		a_uint32_t port_id, fal_pt_1q_egmode_t * egmode);

sw_error_t
fal_port_1qmode_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_pt_1qmode_t port_1qmode);

sw_error_t
fal_port_1qmode_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_pt_1qmode_t * pport_1qmode);

sw_error_t
fal_port_default_svid_set(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t vid);

sw_error_t
fal_port_default_svid_get(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t * vid);

sw_error_t
fal_port_default_cvid_set(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t vid);

sw_error_t
fal_port_default_cvid_get(a_uint32_t dev_id, fal_port_t port_id,
		a_uint32_t * vid);

sw_error_t
fal_port_tls_set(a_uint32_t dev_id, fal_port_t port_id,
		a_bool_t enable);

sw_error_t
fal_qinq_mode_set(a_uint32_t dev_id, fal_qinq_mode_t mode);

sw_error_t
fal_qinq_mode_get(a_uint32_t dev_id, fal_qinq_mode_t * mode);

sw_error_t
fal_port_qinq_role_set(a_uint32_t dev_id, fal_port_t port_id, fal_qinq_port_role_t role);

sw_error_t
fal_port_qinq_role_get(a_uint32_t dev_id, fal_port_t port_id, fal_qinq_port_role_t * role);

sw_error_t
fal_port_default_vlantag_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_port_vlan_direction_t direction,
		fal_port_default_vid_enable_t *default_vid_en,
		fal_port_vlan_tag_t *default_tag);

sw_error_t
fal_port_default_vlantag_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_port_vlan_direction_t direction,
		fal_port_default_vid_enable_t *default_vid_en,
		fal_port_vlan_tag_t *default_tag);

sw_error_t
fal_portvlan_member_add(a_uint32_t dev_id, fal_port_t port_id,
		fal_port_t mem_port_id);

sw_error_t
fal_portvlan_member_del(a_uint32_t dev_id, fal_port_t port_id,
		fal_port_t mem_port_id);

sw_error_t
fal_nestvlan_tpid_set(a_uint32_t dev_id, a_uint32_t tpid);

sw_error_t
fal_nestvlan_tpid_get(a_uint32_t dev_id, a_uint32_t * tpid);

sw_error_t
fal_port_invlan_mode_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_pt_invlan_mode_t mode);

sw_error_t
fal_port_invlan_mode_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_pt_invlan_mode_t * mode);

sw_error_t
fal_port_ingress_vlan_filter_set(a_uint32_t dev_id, fal_port_t port_id,
		fal_ingress_vlan_filter_t *filter);

sw_error_t
fal_port_ingress_vlan_filter_get(a_uint32_t dev_id, fal_port_t port_id,
		fal_ingress_vlan_filter_t *filter);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _PORT_VLAN_H_ */
/**
 * @}
 */
