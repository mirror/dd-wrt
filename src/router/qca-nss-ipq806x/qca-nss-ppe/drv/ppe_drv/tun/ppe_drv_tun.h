/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/ipv6.h>
#include <linux/ppp_defs.h>
#include <ppe_drv_tun_cmn_ctx.h>
#include <ppe_drv_tun_public.h>
#include "ppe_drv_tun_encap.h"
#include "ppe_drv_tun_decap.h"
#include "ppe_drv_tun_encap_xlate_rule.h"
#include "ppe_drv_tun_decap_xlate_rule.h"
#include "ppe_drv_tun_l3_if.h"
#include "ppe_drv_tun_prgm_prsr.h"

#define PPE_DRV_TUN_BIT(x)	(1UL << x)
#define PPE_DRV_TUN_MAX_CTX	128
#define PPE_DRV_TUN_PORT_STATS_RESERVED_COUNT 10  /* Number of slots reserved for tunnel statistics */
#define PPE_DRV_TUN_MAPT_V6_LEN_ADJUST (sizeof(struct ipv6hdr) - sizeof (struct iphdr)) /* IP6 header length difference to be added for MAPT */

/*
 * ppe_drv_tun_tl_action
 *	PPE TL action.
 */
enum ppe_drv_tun_tl_action {
	PPE_DRV_TUN_TL_DEACCEL_DISABLE,	/**< Disable TT deacceleration in PPE >*/
	PPE_DRV_TUN_TL_DEACCEL_ENABLE	/**< Enable TT deacceleration in PPE >*/
};

/*
 * ppe_drv_tun_tl_hash_mode
 *	PPE TL hash mode.
 */
enum ppe_drv_tun_tl_hash_mode {
	PPE_DRV_TUN_TL_HASH_MODE_CRC10,	/**< Hash mode CRC10 >*/
	PPE_DRV_TUN_TL_HASH_MODE_XOR,	/**< Hash mode XOR >*/
	PPE_DRV_TUN_TL_HASH_MODE_CRC16	/**< Hash mode CRC16 >*/
};

/*
 * ppe_drv_tun_tl_ipv4_df_mode
 *	PPE TL IPv4 DF mode.
 */
enum ppe_drv_tun_tl_ipv4_df_mode {
	PPE_DRV_TUN_TL_IPV4_DF_MODE_0,		/**< IPv4 DF bit is always set to 0 >*/
	PPE_DRV_TUN_TL_IPV4_DF_MODE_RFC7915,	/**< IPv4 DF bit is set as per RFC7915 >*/
	PPE_DRV_TUN_TL_IPV4_DF_MODE_1		/**< IPv4 DF bit is always set to 1, for MAP-T only >*/
};

/*
 * ppe_drv_tun_tl_hdr_param_mode
 *	PPE TL_KEY_GEN header parameters mode.
 */
enum ppe_drv_tun_tl_hdr_param_mode {
	PPE_DRV_TUN_TL_HDR_PARAM_MODE_XLD,	/**< Exclude header parameter from key search >*/
	PPE_DRV_TUN_TL_HDR_PARAM_MODE_INC	/**< Include header parameter from key search >*/
};

/*
 * ppe_drv_tun_tl_tbl_op_type
 *	PPE TL_TBL operation types.
 */
enum ppe_drv_tun_tl_tbl_op_type {
	PPE_DRV_TUN_TL_TBL_OP_TYPE_ADD,		/**< Add new entry in TL_TBL >*/
	PPE_DRV_TUN_TL_TBL_OP_TYPE_DEL,		/**< Delete entry from TL_TBL >*/
	PPE_DRV_TUN_TL_TBL_OP_TYPE_GET,		/**< Read entry from TL_TBL >*/
	PPE_DRV_TUN_TL_TBL_OP_TYPE_Flush		/**< Flush TL_TBL >*/
};

/*
 * ppe_drv_tun_tl_tbl_op_mode
 *	PPE TL_TBL operation modes.
 */
enum ppe_drv_tun_tl_tbl_op_mode {
	PPE_DRV_TUN_TL_TBL_OP_MODE_KEY,		/**< Add entry in TL_TBL based on key hash >*/
	PPE_DRV_TUN_TL_TBL_OP_MODE_INDEX		/**< Add entry in TL_TBL at index >*/
};

/*
 * ppe_drv_tun_tl_tbl_op_rslt
 *	PPE TL_TBL operation result.
 */
enum ppe_drv_tun_tl_tbl_op_rslt {
	PPE_DRV_TUN_TL_TBL_OP_RSLT_SUCCESS,		/**< TL_TBL entry operation result is successful >*/
	PPE_DRV_TUN_TL_TBL_OP_RSLT_FAILED		/**< TL_TBL entry operation result is failed >*/
};

/*
 * ppe_drv_tun_tl_tbl_src_info_type
 *	PPE TL_TBL source info type.
 */
enum ppe_drv_tun_tl_tbl_src_info_type {
	PPE_DRV_TUN_TL_TBL_SRC_INFO_TYPE_VP,	/**< TL_TBL entry source info type is VP >*/
	PPE_DRV_TUN_TL_TBL_SRC_INFO_TYPE_L3		/**< TL_TBL entry source info type is L3 >*/
};

/*
 * ppe_drv_tun_tl_tbl_entry_type
 *	PPE TL_TBL entry type.
 */
enum ppe_drv_tun_tl_tbl_entry_type {
	PPE_DRV_TUN_TL_TBL_ENTRY_TYPE_IPV4,	/**< TL_TBL entry type is IPv4 >*/
	PPE_DRV_TUN_TL_TBL_ENTRY_TYPE_IPV6	/**< TL_TBL entry type is IPv6 >*/
};

/*
 * ppe_drv_tun_field
 *	PPE TL_TBL source info valid/invalid.
 */
enum ppe_drv_tun_field {
	PPE_DRV_TUN_FIELD_INVALID,	/**< TL_TBL entry source info invalid >*/
	PPE_DRV_TUN_FIELD_VALID		/**< TL_TBL entry source info valid >*/
};

/*
 * ppe_drv_tun_decap_entry_type
 * 	Translation Decap entry type for MAP-T
 */
enum ppe_drv_tun_decap_entry_type {
	PPE_DRV_TUN_DECAP_LOCAL_ENTRY,	/**< MAP-T local IPV6 address decap entry >*/
	PPE_DRV_TUN_DECAP_REMOTE_ENTRY	/**< MAP-T remote IPV6 address decap entry >*/
};

/* ppe_drv_tun
 *	PPE driver tunnel context
 */
struct ppe_drv_tun {
	struct ppe_drv_tun_cmn_ctx th;				/**< Tunnel header >*/
	struct ppe_drv_port *pp;				/**< Assigned PPE port >*/
	struct ppe_drv_pppoe *pppoe;				/**< Assigned PPPoE port >*/
	struct ppe_drv_tun_l3_if *pt_l3_if;			/**< TL L3 interface instance >*/
	struct ppe_drv_tun_encap *ptec;				/**< EG tunnel encapsulation >*/
	struct ppe_drv_tun_decap *ptdc;				/**< TL decap entry >*/
	struct ppe_drv_tun_encap_xlate_rule *ptecxr;             /* EG edit rule instance */
	struct ppe_drv_tun_decap *ptdcm[PPE_DRV_TUN_DECAP_MAP_ENTRY_PAIR_MAX];
	struct ppe_drv_tun_decap_xlate_rule *ptdcxr[PPE_DRV_TUN_DECAP_MAP_ENTRY_PAIR_MAX];
	ppe_drv_tun_add_ce_callback_t add_cb;			/**< Callback for activating tunnel >*/
	ppe_drv_tun_del_ce_callback_t del_cb;			/**< Callback for deactivating tunnel >*/
	struct kref ref;					/**< Reference count >*/
	uint8_t vp_num;						/**< Tunnel VP number >*/
	uint8_t tun_idx;					/**< Tunnel context ID >*/
	uint8_t xmit_port;					/**< Egress I/O port for tunnel> */
	atomic_t flow_count;					/**< Number of active flows >*/
	uint8_t encap_hdr_bitmap;				/**< Bitmap of feilds enabled in encap header control >**/
};

/*
 * ppe_drv_tun_dp_port_ds
 * 	Check if destination port is wifi DS vp port
 */
static inline bool ppe_drv_tun_dp_port_ds(struct ppe_drv_port *pp)
{
	if (pp->user_type == PPE_DRV_PORT_USER_TYPE_DS) {
		return true;
	}

	return false;
}

/*
 * ppe_drv_tun_dp_port_active_vp
 *	Check if destination port is wifi Active vp port
 */
static inline bool ppe_drv_tun_dp_port_active_vp(struct ppe_drv_port *pp)
{
	if (pp->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP) {
		return true;
	}

	return false;
}

/*
 * ppe_drv_tun_is_dest_port_wifi
 *	Check if destination port of a tunnel is a DS/Active wifi port
 */
static inline bool ppe_drv_tun_is_dest_port_wifi(uint16_t dest_port)
{
	struct ppe_drv_port *pp = NULL;

	pp = ppe_drv_port_from_port_num(dest_port);

	if (pp && (pp->user_type == PPE_DRV_PORT_USER_TYPE_DS
				|| pp->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP)) {
		return true;
	}

	return false;
}

bool ppe_drv_tun_global_init(struct ppe_drv *p);
bool ppe_drv_tun_check_support(uint8_t protocol);
void ppe_drv_tun_vxlan_deconfigure(struct ppe_drv *p);
void ppe_drv_tun_v4_port_stats_update(struct ppe_drv_v4_conn *cn);
void ppe_drv_tun_v6_port_stats_update(struct ppe_drv_v6_conn *cn);
bool ppe_drv_tun_attach_mapt_v6_to_v4(struct ppe_drv_v6_conn *cn);
bool ppe_drv_tun_attach_mapt_v4_to_v6(struct ppe_drv_v4_conn *cn);
bool ppe_drv_tun_detach_mapt_v4_to_v6(struct ppe_drv_v4_conn *cn);
struct ppe_drv_tun *ppe_drv_tun_mapt_port_tun_get(struct ppe_drv_port *tx_port, struct ppe_drv_port *rx_port);
