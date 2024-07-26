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

/**
 * @addtogroup ppe_drv_subsystem
 * @{
 */

#ifndef _PPE_DRV_SC_H_
#define _PPE_DRV_SC_H_

struct ppe_drv;
struct ppe_drv_nsm_stats;

/*
 * FLOW ACL rule service code range.
 */
#define PPE_DRV_SC_FLOW_ACL_MAX 128
#define PPE_DRV_SC_FLOW_ACL_START 128
#define PPE_DRV_SC_FLOW_ACL_END (PPE_DRV_SC_FLOW_ACL_START + PPE_DRV_SC_FLOW_ACL_MAX - 1)

/*
 * ppe_drv_sc_type
 *	Service code types
 */
typedef enum ppe_drv_sc_type {
	PPE_DRV_SC_NONE,		/* Normal PPE processing */
	PPE_DRV_SC_BYPASS_ALL,		/* Bypasses all stages in PPE */
	PPE_DRV_SC_ADV_QOS_BRIDGED,	/* Adv QoS redirection for bridged flow */
	PPE_DRV_SC_LOOPBACK_QOS,	/* Bridge or IGS QoS redirection */
	PPE_DRV_SC_BNC_0,		/* QoS bounce */
	PPE_DRV_SC_BNC_CMPL_0,		/* QoS bounce complete */
	PPE_DRV_SC_ADV_QOS_ROUTED,	/* Adv QoS redirection for routed flow */
	PPE_DRV_SC_IPSEC_PPE2EIP,	/* Inline IPsec redirection from PPE TO EIP */
	PPE_DRV_SC_IPSEC_EIP2PPE,	/* Inline IPsec redirection from EIP to PPE */
	PPE_DRV_SC_PTP,			/* Service Code for PTP packets */
	PPE_DRV_SC_VLAN_FILTER_BYPASS,	/* VLAN filter bypass for bridge flows between 2 different VSIs */
	PPE_DRV_SC_L3_EXCEPT,		/* Indicate exception post tunnel/tap operation */
	PPE_DRV_SC_SPF_BYPASS,		/* Source port filtering bypass */
	PPE_DRV_SC_NOEDIT_REDIR_CORE0,	/* Service code to re-direct packets to core 0 without editing the packet */
	PPE_DRV_SC_NOEDIT_REDIR_CORE1,	/* Service code to re-direct packets to core 1 without editing the packet */
	PPE_DRV_SC_NOEDIT_REDIR_CORE2,	/* Service code to re-direct packets to core 2 without editing the packet */
	PPE_DRV_SC_NOEDIT_REDIR_CORE3,	/* Service code to re-direct packets to core 3 without editing the packet */
	PPE_DRV_SC_EDIT_REDIR_CORE0,	/* Service code to re-direct packets to core 0 with editing required for regular forwarding */
	PPE_DRV_SC_EDIT_REDIR_CORE1,	/* Service code to re-direct packets to core 1 with editing required for regular forwarding */
	PPE_DRV_SC_EDIT_REDIR_CORE2,	/* Service code to re-direct packets to core 2 with editing required for regular forwarding */
	PPE_DRV_SC_EDIT_REDIR_CORE3,	/* Service code to re-direct packets to core 3 with editing required for regular forwarding */
	PPE_DRV_SC_VP_RPS,		/* Service code to allow RPS for special VP flows when user type is DS and core_mask is 0 */
	PPE_DRV_SC_NOEDIT_ACL_POLICER,  /* Service code to allow Policing but no packet editing */
	PPE_DRV_SC_L2_TUNNEL_EXCEPTION,	/* Service code to allow decapsulated VXLAN/GRE tunnel exception. */
	PPE_DRV_SC_NOEDIT_PRIORITY_SET, /* Service code to prioritize packets without editing and redirection */
	PPE_DRV_SC_NOEDIT_RULE, 	/* Service code to redirect packets without editing */

	PPE_DRV_SC_DS_MLO_LINK_BR_NODE0, /* Service code when bridge flow in DS with PPEDS Node 0 allocated for MLO Link */
	PPE_DRV_SC_DS_MLO_LINK_BR_NODE1, /* Service code when bridge flow in DS with PPEDS Node 1 allocated for MLO Link */
	PPE_DRV_SC_DS_MLO_LINK_BR_NODE2, /* Service code when bridge flow in DS with PPEDS Node 2 allocated for MLO Link */
	PPE_DRV_SC_DS_MLO_LINK_BR_NODE3, /* Service code when bridge flow in DS with PPEDS Node 3 allocated for MLO Link */
	PPE_DRV_SC_DS_MLO_LINK_RO_NODE0, /* Service code when routed flow in DS with PPEDS Node 0 allocated for MLO Link */
	PPE_DRV_SC_DS_MLO_LINK_RO_NODE1, /* Service code when routed flow in DS with PPEDS Node 1 allocated for MLO Link */
	PPE_DRV_SC_DS_MLO_LINK_RO_NODE2, /* Service code when routed flow in DS with PPEDS Node 2 allocated for MLO Link */
	PPE_DRV_SC_DS_MLO_LINK_RO_NODE3, /* Service code when routed flow in DS with PPEDS Node 3 allocated for MLO Link */
	PPE_DRV_SC_FLOW_ACL_FIRST = PPE_DRV_SC_FLOW_ACL_START,
					/* First service code for combining flow and ACL rule */
	PPE_DRV_SC_FLOW_ACL_LAST = PPE_DRV_SC_FLOW_ACL_END,
					/* Last service code for combining flow and ACL rule */
	PPE_DRV_SC_MAX = 256,		/* Max service code */
} ppe_drv_sc_t;

/*
 * ppe_drv_sc_metadata
 *	metadata for service codes.
 */
struct ppe_drv_sc_metadata {
	uint32_t tree_id;		/* Tree id from EDMA descriptor */
	uint32_t wifi_qos;		/* WiFi-qos from EDMA descriptor*/
	uint32_t int_pri;		/* Priority from EDMA descriptor*/
	uint8_t vp_num;			/* Destination VP number */
	uint8_t service_code;		/* Service code from EDMA descriptor*/
};

typedef bool (*ppe_drv_sc_callback_t)(void *app_data, struct sk_buff *skb, void *sc_data);

/*
 * ppe_drv_sc_process_skbuff()
 *	Register callback for a specific service code
 *
 * @param[IN] sc   Service code related metadata.
 * @param[IN] skb  Socket buffer with service code.
 *
 * @return
 * true if packet is consumed by the API or false if the packet is not consumed.
 */
extern bool ppe_drv_sc_process_skbuff(struct ppe_drv_sc_metadata *sc, struct sk_buff *skb);

/*
 * ppe_drv_sc_unregister_cb()
 *	Unregister callback for a specific service code
 *
 * @param[IN] sc   Service code number.
 *
 * @return
 * void
 */
extern void ppe_drv_sc_unregister_cb(ppe_drv_sc_t sc);

/*
 * ppe_drv_sc_register_cb()
 *	Register callback for a specific service code
 *
 * @param[IN] sc   Service code number.
 * @param[IN] cb   Callback API.
 * @param[IN] app_data   Application data to be passed to callback.
 *
 * @return
 * void
 */
extern void ppe_drv_sc_register_cb(ppe_drv_sc_t sc, ppe_drv_sc_callback_t cb, void *app_data);

/*
 * ppe_drv_sc_unregister_vp_cb()
 *	Unregister vp callback for a specific service code
 *
 * @param[IN] sc   Service code number.
 * @param[IN] vp_num   Virtual port number unregistering the callback.
 *
 * @return
 * void
 */
extern void ppe_drv_sc_unregister_vp_cb(ppe_drv_sc_t sc, uint16_t vp_num);

/*
 * ppe_drv_sc_register_vp_cb()
 *	Register vp callback for a specific service code
 *
 * @param[IN] sc   Service code number.
 * @param[IN] cb   Callback API.
 * @param[IN] app_data   Application data to be passed to callback.
 * @param[IN] vp_num     Virtual port number registering the callback.
 *
 * @return
 * void
 */
extern bool ppe_drv_sc_register_vp_cb(ppe_drv_sc_t sc, ppe_drv_sc_callback_t cb, void *app_data, uint16_t vp_num);

/** @} */ /* end_addtogroup ppe_drv_sc_subsystem */

#endif /* _PPE_DRV_SC_H_ */

