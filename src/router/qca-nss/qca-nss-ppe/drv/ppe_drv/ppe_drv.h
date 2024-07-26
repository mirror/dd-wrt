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

#include <linux/module.h>
#include <ppe_drv_public.h>
#include <ppe_drv_tun_cmn_ctx.h>
#include <ppe_drv_tun_public.h>
#include "ppe_drv_acl.h"
#include "ppe_drv_exception.h"
#include "ppe_drv_cc.h"
#include "ppe_drv_flow.h"
#include "ppe_drv_host.h"
#include "ppe_drv_iface.h"
#include "ppe_drv_l3_if.h"
#include "ppe_drv_nexthop.h"
#include "ppe_drv_policer.h"
#include "ppe_drv_port.h"
#include "ppe_drv_pppoe.h"
#include "ppe_drv_pub_ip.h"
#include "ppe_drv_sc.h"
#include "ppe_drv_stats.h"
#include "ppe_drv_vsi.h"
#include "ppe_drv_v4.h"
#include "ppe_drv_v6.h"
#include "ppe_drv_flow_dump.h"
#include "ppe_drv_if_map.h"

extern uint32_t static_dbg_level;

/*
 * ppe_drv_static_dbg_level
 *	PPE static debug level
 */
enum ppe_drv_static_dbg_level {
	PPE_DRV_STATIC_DBG_LEVEL_NONE,
	PPE_DRV_STATIC_DBG_LEVEL_WARN,
	PPE_DRV_STATIC_DBG_LEVEL_INFO,
	PPE_DRV_STATIC_DBG_LEVEL_TRACE,
};

/*
 * PPE debug macros
 */
#if (PPE_DRV_DEBUG_LEVEL == 3)
#define ppe_drv_assert(c, s, ...)
#else
#define ppe_drv_assert(c, s, ...) if (!(c)) { printk(KERN_CRIT "%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG_ON(!(c)); }
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * If dynamic debug is enabled, use pr_debug.
 */
#define ppe_drv_warn(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_drv_info(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ppe_drv_trace(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels, when dynamic debug is disabled.
 */
#if (PPE_DRV_DEBUG_LEVEL < 2)
#define ppe_drv_warn(s, ...)
#else
#define ppe_drv_warn(s, ...) \
	if (static_dbg_level >= PPE_DRV_STATIC_DBG_LEVEL_WARN) \
		pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_DRV_DEBUG_LEVEL < 3)
#define ppe_drv_info(s, ...)
#else
#define ppe_drv_info(s, ...) \
	if (static_dbg_level >= PPE_DRV_STATIC_DBG_LEVEL_INFO) \
		pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_DRV_DEBUG_LEVEL < 4)
#define ppe_drv_trace(s, ...)
#else
#define ppe_drv_trace(s, ...) \
	if (static_dbg_level >= PPE_DRV_STATIC_DBG_LEVEL_TRACE) \
		pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 * Default switch ID
 */
#define PPE_DRV_SWITCH_ID		0

/*
 * Profile ID
 */
#define PPE_DRV_COMMON_PROFILE_ID	FAL_QM_PROFILE_COMMON_ID
#define PPE_DRV_PO_PROFILE_ID		FAL_QM_PROFILE_PO_ID
#define PPE_DRV_REDIR_PROFILE_ID	9

/*
 * MAX queue priority
 */
#define PPE_DRV_MAX_PRIORITY		16

/*
 * Maximum queue priority supported per core
 */
#define PPE_DRV_MAX_PRIORITY_PER_CORE	8

/*
 * PPE Hash seed and mask
 *
 * Note: we don't initialize the seed value with a random value
 * to keep the hash calculation persistent across reboots.
 */
#define PPE_DRV_HASH_SEED_DEFAULT	0xabbcdefa
#define PPE_DRV_HASH_MASK		0xfff
#define PPE_DRV_HASH_MIX_V4_SIP		0x13
#define PPE_DRV_HASH_MIX_V4_DIP		0xb
#define PPE_DRV_HASH_MIX_V4_PROTO	0x13
#define PPE_DRV_HASH_MIX_V4_DPORT	0xb
#define PPE_DRV_HASH_MIX_V4_SPORT	0x13
#define PPE_DRV_HASH_FIN_MASK		0x1f
#define PPE_DRV_HASH_FIN_INNER_OUTER_0		0x205
#define PPE_DRV_HASH_FIN_INNER_OUTER_1		0x264
#define PPE_DRV_HASH_FIN_INNER_OUTER_2		0x227
#define PPE_DRV_HASH_FIN_INNER_OUTER_3		0x245
#define PPE_DRV_HASH_FIN_INNER_OUTER_4		0x201

#define PPE_DRV_HASH_SIPV6_MIX_0		0x13
#define PPE_DRV_HASH_SIPV6_MIX_1		0xb
#define PPE_DRV_HASH_SIPV6_MIX_2		0x13
#define PPE_DRV_HASH_SIPV6_MIX_3		0xb
#define PPE_DRV_HASH_DIPV6_MIX_0		0x13
#define PPE_DRV_HASH_DIPV6_MIX_1		0xb
#define PPE_DRV_HASH_DIPV6_MIX_2		0x13
#define PPE_DRV_HASH_DIPV6_MIX_3		0xb

#define PPE_DRV_JUMBO_MAX 9216

/*
 * DSCP macros
 */
#define PPE_DRV_DSCP_SHIFT 2
#define PPE_DRV_DSCP_MASK 0xFC

/*
 * VLAN macros
 */
#define PPE_DRV_VLAN_NOT_CONFIGURED	0xFFF
#define PPE_DRV_VLAN_ID_MASK		0xFFF
#define PPE_DRV_VLAN_TPID_MASK		0xFFFF0000
#define PPE_DRV_VLAN_TCI_MASK		0xFFFF
#define PPE_DRV_VLAN_PRIORITY_MASK	0xE000
#define PPE_DRV_VLAN_PRIORITY_SHIFT	13

/*
 * SAWF macros
 */
#define PPE_DRV_SAWF_TAG_SHIFT				24
#define PPE_DRV_SAWF_VALID_TAG				0xAA
#define PPE_DRV_SAWF_SERVICE_CLASS_SHIFT		16
#define PPE_DRV_SAWF_SERVICE_CLASS_MASK			0xFF
#define PPE_DRV_SAWF_PEER_ID_SHIFT			6
#define PPE_DRV_SAWF_PEER_ID_MASK			0x3ff
#define PPE_DRV_SAWF_MSDUQ_MASK				0x3f
#define PPE_DRV_SAWF_TAG_GET(x)				(x >> PPE_DRV_SAWF_TAG_SHIFT)
#define PPE_DRV_SAWF_PEER_ID_GET(x)			((x >> PPE_DRV_SAWF_PEER_ID_SHIFT) & \
							PPE_DRV_SAWF_PEER_ID_MASK)
#define PPE_DRV_SAWF_MSDUQ_GET(x)			(x & PPE_DRV_SAWF_MSDUQ_MASK)

/*
 * MLO macros
 */
#define PPE_DRV_MLO_MARK_SHIFT				6
#define PPE_DRV_MLO_MARK_MASK				0x3FFFF
#define PPE_DRV_MLO_MARK_GET(x)				((x >> PPE_DRV_MLO_MARK_SHIFT) & PPE_DRV_TREE_ID_MLO_MARK_MASK)

#define PPE_DRV_MLO_MSDUQ_MASK				0x3F
#define PPE_DRV_MLO_MSDUQ_GET(x)			(x & PPE_DRV_MLO_MSDUQ_MASK)

/*
 * Tree ID macros
 */
#define PPE_DRV_TREE_ID_TYPE_SHIFT			20
#define PPE_DRV_TREE_ID_TYPE_MASK			0x00F00000
#define PPE_DRV_TREE_ID_TYPE_SET(w, x)			((*w) |= (((x) << PPE_DRV_TREE_ID_TYPE_SHIFT) & PPE_DRV_TREE_ID_TYPE_MASK))

#define PPE_DRV_TREE_ID_SERVICE_CLASS_SHIFT		10
#define PPE_DRV_TREE_ID_SERVICE_CLASS_MASK		0x0003FC00
#define PPE_DRV_TREE_ID_SERVICE_CLASS_SET(w, x)		((*w) |= (((x) << PPE_DRV_TREE_ID_SERVICE_CLASS_SHIFT) & PPE_DRV_TREE_ID_SERVICE_CLASS_MASK))

#define PPE_DRV_TREE_ID_PEER_ID_MASK			0x000003FF
#define PPE_DRV_TREE_ID_PEER_ID_SET(w, x)		((*w) |= ((x) & PPE_DRV_TREE_ID_PEER_ID_MASK))

/*
 * Setting MLO mark into Tree ID
 */
#define PPE_DRV_TREE_ID_MLO_MARK_MASK			0x0003FFFF
#define PPE_DRV_TREE_ID_MLO_MARK_SET(w, x)		((*w) |= ((x) & PPE_DRV_TREE_ID_MLO_MARK_MASK))

/*
 * HW flow stats sync timer frequency in milliseconds
 */
#define PPE_DRV_HW_FLOW_STATS_MS	1000

/*
 * Core to service code mapping
 */
#define PPE_DRV_CORE2SC_NOEDIT(core_id) (PPE_DRV_SC_NOEDIT_REDIR_CORE0 + core_id)
#define PPE_DRV_CORE2SC_EDIT(core_id) (PPE_DRV_SC_EDIT_REDIR_CORE0 + core_id)

/*
 * Default port number return from ssdk is 0xF
 * Which indicates no port is set for mirror analysis
 */
#define PPE_DRV_MIRR_INVAL_PORT 0xF

#if defined(NSS_PPE_IPQ53XX)
#define PPE_DRV_PORT_OFFLOAD_MAX_VAL		0x3
#elif defined(NSS_PPE_IPQ95XX)
#define PPE_DRV_PORT_OFFLOAD_MAX_VAL		0x3f
#else
#define PPE_DRV_PORT_OFFLOAD_MAX_VAL		0xff
#endif

/*
 * L2TP Tunnel default UDP Port
 */
#define PPE_DRV_L2TP_DEFAULT_UDP_PORT	1701

/*
 * Maximum number of MLO link IDs
 */
#define PPE_DRV_DS_MLO_LINK_NODE_ID_MAX	3

/*
 * ppe_drv_entry_valid
 *	PPE entry validity
 */
enum ppe_drv_entry_valid {
	PPE_DRV_ENTRY_INVALID,	/* Entry invalid. */
	PPE_DRV_ENTRY_VALID,	/* Entry valid. */
};

/*
 * ppe_drv_tun_l2tp
 *	l2tp tunnel specific global data
 */
struct ppe_drv_tun_l2tp {
	uint16_t l2tp_sport;				/* L2TP Source port */
	uint16_t l2tp_dport;				/* L2TP Destination port */
	struct ppe_drv_tun_encap_xlate_rule *l2tp_encap_rule; 	/* PPE L2TP EG translate rule entry */
};

/*
 * ppe_drv_tun_gbl
 *	PPE tunnel specific global context in ppe drv
 */
struct ppe_drv_tun_gbl {
	struct ppe_drv_tun_l2tp tun_l2tp;
};

/*
 * ppe_drv
 *	PPE DRV base structure
 */
struct ppe_drv {
	spinlock_t lock;				/* PPE lock */
	spinlock_t stats_lock;				/* PPE statistics lock */
	spinlock_t notifier_lock;			/* PPE notifiers lock */

	uint32_t iface_num;				/* Number of PPE interface */
	uint32_t l3_if_num;				/* Number of entries in PPE L3_IF table */
	uint32_t port_num;				/* Number of entries in PPE Port table */
	uint32_t vsi_num;				/* Number of entries in PPE VSI table */
	uint32_t pub_ip_num;				/* Number of entries in PPE Public IP table */
	uint32_t host_num;				/* Number of entries in PPE Host table */
	uint32_t flow_num;				/* Number of entries in PPE Flow table */
	uint32_t pppoe_session_max;			/* Number of entries in PPE PPPoe Session table */
	uint32_t nexthop_num;				/* Number of entries in PPE Nexthop table */
	uint32_t sc_num;				/* Number of entries in PPE Service Code table */
	uint32_t queue_num;				/* Number of entries in PPE Service Code table */

	/*
	 * Timer
	 */
	unsigned long hw_flow_stats_ticks;		/* Ticks to re-arm the hardware stats timer */
	struct timer_list hw_flow_stats_timer;		/* Timer used to poll for stats from PPE_HW */
	struct ppe_drv_stats stats;			/* PPE statistics */

	/*
	 * Pointer to memory pool for different PPE tables
	 */
	uint8_t core2queue[NR_CPUS];			/* Core to queue mapping */
	struct ppe_drv_iface *iface;			/* Memory for PPE interface shadow table */
	struct ppe_drv_flow *flow;			/* Memory for PPE Flow table */
	struct ppe_drv_host *host;			/* Memory for PPE Host table */
	struct ppe_drv_nexthop *nexthop;		/* Memory for PPE nexthop table */
	struct ppe_drv_pub_ip *pub_ip;			/* Memory for PPE Public IP table */
	struct ppe_drv_vsi *vsi;			/* Memory for PPE VSI shadow table */
	struct ppe_drv_port *port;			/* Memory for PPE Port table */
	struct ppe_drv_l3_if *l3_if;			/* Memory for PPE L3_IF shadow table */
	struct ppe_drv_pppoe *pppoe;			/* Memory for PPE PPPoe table */
	struct ppe_drv_queue *queue;			/* Memory for PPE queue table */
	struct ppe_drv_policer_ctx *pol_ctx;		/* Policer global context */
	struct ppe_drv_tun_encap *ptun_ec;	/* PPE EG tunnel/translate control entries */
	struct ppe_drv_tun_decap *ptun_dc;	/* PPE tunnel decap control entries */
	struct ppe_drv_tun_l3_if *ptun_l3_if;	/* PPE tunnel L3 interface info */
	struct ppe_drv_tun_decap *decap_map_entries;	/* PPE EG tunnel/translate control entries */
	struct ppe_drv_tun_encap_xlate_rule *encap_xlate_rules; 	/* PPE EG translate rule entries */
	struct ppe_drv_tun_decap_xlate_rule *decap_xlate_rules; 	/* PPE Tunnel decap xlate rules */
	struct ppe_drv_sc *sc;				/* Memory for PPE Service Code table */
	struct ppe_drv_cc *cc;				/* Memory for PPE CPU Code table */
	struct ppe_drv_acl *acl;			/* Memory for PPE ACL entries */
	struct dentry *dentry;				/* Debugfs entry */
	struct dentry *stats_dentry;				/* Debugfs entry */
	struct ctl_table_header *ppe_drv_header;	/* PPE DRV sysctl */
	ppe_drv_v4_sync_callback_t ipv4_stats_sync_cb;		/* Callback to call to sync ipv4 statistics */
	void *ipv4_stats_sync_data;				/* Argument for above callback: ipv4_stats_sync_cb */
	ppe_drv_v6_sync_callback_t ipv6_stats_sync_cb;		/* Callback to call to sync ipv6 statistics */
	void *ipv6_stats_sync_data;				/* Argument for above callback: ipv6_stats_sync_cb */
	ppe_drv_qos_int_pri_callback_t int_pri_get_cb;		/* Callback to call to get INT-PRI value */
	struct list_head nh_active;			/* List of active nexthops */
	struct list_head nh_free;			/* List of free nexthops */
	struct kref ref;				/* Reference count */

	/*
	 * v4 and v6 connection list
	 */
	struct list_head conn_v4;			/* List of v4 connection in PPE */
	struct list_head conn_v6;			/* List of v6 connection in PPE */
	struct list_head conn_tun_v4;		/* List of v4 tunnel connection in PPE */
	struct list_head conn_tun_v6;		/* List of v6 tunnel connection in PPE */
	struct ppe_drv_fse_ops *fse_ops;        /* Wi-Fi FSE block operations */
	struct kref fse_ops_ref;		/* FSE Reference count */
	int vxlan_dport;			/* VXLAN destination port */
	bool fse_enable;			/* FSE enabled */
	bool is_wifi_fse_up;			/* Wi-FI FSE ops registered with PPE */

	bool toggled_v4;			/* Toggled bit for v4 sync during a particular iteration */
	bool toggled_v6;			/* Toggled bit for v6 sync during a particular iteration */
	bool tun_toggled_v4;		        /* Tunnel specific Toggled bit for v4 sync during a particular iteration*/
	bool tun_toggled_v6;		        /* Tunnel specific Toggled bit for v6 sync during a particular iteration*/
	struct list_head notifier_list_head;	/* List of event notifier operations in PPE */
	struct ppe_drv_tun_prgm_prsr *pgm;	/* Program Parser entries list */
	struct ppe_drv_tun_udf *pgm_udf;	/* Program Parser udf entries list */
	struct ppe_drv_tun_encap_hdr_ctrl *ecap_hdr_ctrl;	/* header control protomap data */
	bool disable_port_mtu_check;			/* Flag to disable MTU check for all the ports */
	bool eth2eth_offload_if_bitmap;		/* Flag to enable if bitmap check for eth to eth flows */
	struct ppe_drv_tun_gbl tun_gbl;		/* ppe tunnel global context */
};

/*
 * ppe_drv_tree_id_type_get()
 *	Returns the tree_id type.
 */
static inline ppe_drv_tree_id_type_t ppe_drv_tree_id_type_get(struct ppe_drv_flow_metadata *flow_metadata)
{
	return flow_metadata->tree_id_data.type;
}

/*
 * ppe_drv_assist_feature_type_check()
 *      Checks assist feature type.
 */
static inline bool ppe_drv_assist_feature_type_check(uint32_t feature, uint32_t flag)
{
	return !!(feature & flag);
}

/*
 * ppe_drv_sawf_metadata
 *	SAWF information from create rule
 */
struct ppe_drv_sawf_metadata {
	uint32_t sawf_mark;	/* SAWF mark from create rule. */
	uint8_t service_class;	/* SAWF service class from create rule. */
};

/*
 * ppe_drv_scs_metadata
 *      SCS information from create rule
 */
struct ppe_drv_scs_metadata {
	uint32_t scs_mark;	/* SCS mark from create rule. */
};

/*
 * ppe_drv_flow_cookie_metadata
 *	Flow Cookie information from the create rule
 */
struct ppe_drv_flow_cookie_metadata {
	union {
		uint32_t mark;				/* Mark value for tree id type none from create rule message */
		struct ppe_drv_sawf_metadata sawf;	/* SAWF metadata from create rule message */
		struct ppe_drv_scs_metadata scs;	/* SCS metadata from create rule message */
	}type;
};

extern int ppe_drv_get_vxlan_dport(void);
void ppe_drv_fse_ops_free(struct kref *kref);
extern struct ppe_drv ppe_drv_gbl;
extern uint32_t if_bm_to_offload;
