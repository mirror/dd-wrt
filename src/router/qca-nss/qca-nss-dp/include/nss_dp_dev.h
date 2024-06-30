/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2021-2023, Qualcomm Innovation Center, Inc. All rights reserved
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __NSS_DP_DEV_H__
#define __NSS_DP_DEV_H__

#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#if defined(NSS_DP_NETSTANDBY)
#include <linux/netstandby.h>
#endif
#include <linux/platform_device.h>
#include <linux/switch.h>
#include <linux/version.h>
#include <linux/ethtool.h>

#include "nss_dp_api_if.h"
#include "nss_dp_hal_if.h"
#include "nss_dp_hal_info.h"
#ifdef NSS_DP_PPEDS_SUPPORT
#include "nss_dp_ppeds.h"
#endif

#define NSS_DP_ACL_DEV_ID 0

#if defined(NSS_DP_MAC_POLL_SUPPORT)
#define NSS_DP_EDMA_SWITCH_DEV_ID	0
#endif

/*
 * Number of TX/RX queue supported
 */
#define NSS_DP_NETDEV_TX_QUEUE_NUM	NSS_DP_QUEUE_NUM
#define NSS_DP_NETDEV_RX_QUEUE_NUM	NSS_DP_QUEUE_NUM

/*
 * Maximum supported GSO segments
 */
#define NSS_DP_GSO_MAX_SEGS		NSS_DP_HAL_GSO_MAX_SEGS

/*
 * Rx buffer allocation size as per memory profile
 */
#if (defined(NSS_DP_MEM_PROFILE_LOW) || defined(NSS_DP_MEM_PROFILE_MEDIUM)) && !defined(__LP64__)
#define NSS_DP_RX_BUFFER_SIZE		1856
#else
#define NSS_DP_RX_BUFFER_SIZE		1984
#endif

#if defined(NSS_DP_EDMA_V2)
/*
 * Rx rings flow control threshold values
 *
 * The Rx flow control has the X-OFF and the X-ON threshold values.
 * Whenever the free Rx ring descriptor count falls below the X-OFF value, the
 * ring level flow control will kick in and the mapped PPE queues will be backpressured.
 * Similarly, whenever the free Rx ring descriptor count crosses the X-ON value,
 * the ring level flow control will be disabled.
 */
#define NSS_DP_RX_FC_XOFF_DEF		32
#define NSS_DP_RX_FC_XON_DEF		64

/*
 * Rx ring's mapped AC FC threshold value.
 *
 * This value is picked by running four uni-UDPv4 traffic which are mapped
 * to 4 different rings (and hence processed by 4 different cores) and then
 * increase one flow's rate to more than what the core can process and observe
 * whether this congestion in one flow is influencing other flows or not.
 * Below is the maximum threshold value which is invoking the congestion's queue
 * tail drop and hence not trigerring the Rx port's back-pressure.
 */
#define NSS_DP_RX_AC_FC_THRES_DEF	0x104

/*
 * Tx/Rx Mitigation values
 *
 * Packet count indicates number of packets received or transmitted after which
 * the interrupt is triggered. Stale timer (in microseconds) indicates time after
 * which interrupt is triggered in case number of packets have not been accumulated.
 *
 * These values are picked by running the single large packet uni-UDPv4 traffic
 * at moderate speed and observing the CPU utilization for the flow's processing
 * core.
 * Below values gave the best combination of the idle CPU percentage left for the
 * above mentioned scenario and the minimum impact of the single and multi core
 * small packet's PPS numbers.
 */
#define NSS_DP_TX_MITIGATION_TIMER_DEF		250
#define NSS_DP_TX_MITIGATION_PKT_CNT_DEF	16
#define NSS_DP_RX_MITIGATION_TIMER_DEF		25
#define NSS_DP_RX_MITIGATION_PKT_CNT_DEF	16

/*
 * Virtual Port dummy MAC ID
 */
#define NSS_DP_VP_MAC_ID		(NSS_DP_HAL_MAX_PORTS + 2)
#endif

#if defined(NSS_DP_NETSTANDBY) && defined(NSS_DP_IPQ53XX)
#define NSS_DP_EDMA_SWITCH_MHT_DEV_ID	1
#endif

/*
 * TODO - move NSS_DP_ETHTOOL_MRR_OPS section to nss_dp_ethtool_priv.h
 */
#ifdef NSS_DP_ETHTOOL_MRR_OPS
/*
 * nss_dp_priv_flags_bit_no
 *	ethtool private flags
 */
enum nss_dp_priv_flags_bit_no {
	NSS_DP_MIRR_IN_FLG_BIT = 0,
	NSS_DP_MIRR_EG_FLG_BIT,
	NSS_DP_MIRR_ANALYSIS_IN_FLG_BIT,
	NSS_DP_MIRR_ANALYSIS_EG_FLG_BIT,
	NSS_DP_MAX_ETHTOOL_PRIV_FLAGS,
};

/* No change in ethtool private flag */
#define NSS_DP_NO_PRIV_FLAG_CHANGE	0

/* Mirror ingress priv flag bit */
#define NSS_DP_MIRR_IN_ENABLE	(1 << NSS_DP_MIRR_IN_FLG_BIT)

/* Mirror egress priv flag bit */
#define NSS_DP_MIRR_EG_ENABLE	(1 << NSS_DP_MIRR_EG_FLG_BIT)

/* Analysis mirror ingress priv flag bit */
#define NSS_DP_MIRR_ANALYSIS_IN_ENABLE	(1 << NSS_DP_MIRR_ANALYSIS_IN_FLG_BIT)

/* Analysis mirror egress priv flag bit */
#define NSS_DP_MIRR_ANALYSIS_EG_ENABLE	(1 << NSS_DP_MIRR_ANALYSIS_EG_FLG_BIT)

/*
 * nss-dp ethtool private flags
 */
static const char nss_dp_priv_flg_str[][ETH_GSTRING_LEN] = {
	"Mirror-set-ingress",
	"Mirror-set-egress",
	"Mirror-set-analysis-ingress",
	"Mirror-set-analysis-egress",
};

/**
 * __nss_dp_set_priv_flags()
 *	set ethtool private flags
 */
int __nss_dp_set_priv_flags(struct net_device *dev, u32 flags);

/**
 * __nss_dp_get_priv_flags()
 *	get ethtool private flags
 */
u32 __nss_dp_get_priv_flags(struct net_device *dev);

#else

#define NSS_DP_MAX_ETHTOOL_PRIV_FLAGS	0

/**
 * __nss_dp_set_priv_flags()
 *	set ethtool private flags
 */
static inline int __nss_dp_set_priv_flags(struct net_device *dev, u32 flags)
{
	return -EOPNOTSUPP;
}

/**
 * __nss_dp_get_priv_flags()
 *	get ethtool private flags
 */
static inline u32 __nss_dp_get_priv_flags(struct net_device *dev)
{
	return 0;
}

static const char nss_dp_priv_flg_str[][ETH_GSTRING_LEN] = {
	"",
};

#endif /* NSS_DP_ETHTOOL_MRR_OPS */

struct nss_dp_global_ctx;

#if defined(NSS_DP_NETSTANDBY)
/*
 * nss_dp_netstandby_gbl_ctx
 *	Global structure to be used as APP data with netstandby module
 */
struct nss_dp_netstandby_gbl_ctx {
	struct nss_dp_global_ctx *ctx;	/* Global NSS DP context */

	/*
	 * ErP completion callbacks
	 */
	netstandby_event_compl_cb_t enter_cmp_cb;       /**< Callback enter completion event */
	netstandby_event_compl_cb_t exit_cmp_cb;        /**< Callback exit completion event */
};
#endif

/*
 * nss data plane device structure
 */
struct nss_dp_dev {
	uint32_t macid;			/* Sequence# of Mac on the platform */
	uint32_t vsi;			/* vsi number */
	unsigned long flags;		/* Status flags */
	unsigned long drv_flags;	/* Driver specific feature flags */

	/* Phy related stuff */
	struct device_node *phy_node;	/* Phy device OF node */
	struct phy_device *phydev;	/* Phy device */
	struct mii_bus *miibus;		/* MII bus */
	uint32_t phy_mii_type;		/* RGMII/SGMII/QSGMII */
	uint32_t phy_mdio_addr;		/* Mdio address */
	bool link_poll;			/* Link polling enable? */
	uint32_t forced_speed;		/* Forced speed? */
	uint32_t forced_duplex;		/* Forced duplex? */
	uint32_t link_state;		/* Current link state */
	uint32_t pause;			/* Current flow control settings */

	struct net_device *netdev;
	struct platform_device *pdev;
	struct napi_struct napi;

	struct nss_dp_data_plane_ctx *dpc;
					/* context when NSS owns GMACs */
	struct nss_dp_data_plane_ops *data_plane_ops;
					/* ops for each data plane */
	struct nss_dp_global_ctx *ctx;	/* Global NSS DP context */
	struct nss_gmac_hal_dev *gmac_hal_ctx;
					/* context of gmac hal */
	struct nss_gmac_hal_ops *gmac_hal_ops;
					/* GMAC HAL OPS */
	struct nss_dp_hal_info dp_info;
					/* SoC specific data plane information */

	/* switchdev related attributes */
#ifdef CONFIG_NET_SWITCHDEV
	u8 stp_state;			/* STP state of this physical port */
	unsigned long brport_flags;	/* bridge port flags */
#endif
	uint32_t rx_page_mode;		/* page mode for Rx processing */
	uint32_t rx_jumbo_mru;		/* Jumbo mru value for Rx processing */
#ifdef NSS_DP_ETHTOOL_MRR_OPS
	uint32_t ethtool_priv_flags;	/* Ethtool private flags */
#endif /* NSS_DP_ETHTOOL_MRR_OPS */
	bool ppe_offload_disabled;
	bool is_switch_connected;	/* If there is an additional Switch connected */
#ifdef NSS_DP_MHT_SW_PORT_MAP
	bool nss_dp_mht_dev;		/* Netdevice belongs to MHT switch */
#endif
	uint32_t fixed_link_speed;	/* Fixed link speed for the port connected to the switch */
};

/*
 * nss data plane global context
 */
struct nss_dp_global_ctx {
	struct nss_dp_dev *nss_dp[NSS_DP_MAX_PORTS];
	struct nss_gmac_hal_ops *gmac_hal_ops[GMAC_HAL_TYPE_MAX];
					/* GMAC HAL OPS */
	bool common_init_done;		/* Flag to hold common init state */
	uint8_t slowproto_acl_bm;	/* Port bitmap to allow slow protocol packets */
	uint32_t rx_buf_size;		/* Buffer size to allocate */
	uint32_t jumbo_mru;		/* Jumbo mru value for Rx processing */
	bool overwrite_mode;		/* Overwrite mode for Rx processing */
	bool page_mode;			/* Page mode for Rx processing */
	bool tx_requeue_stop;		/* Disable queue stop for Tx processing */
#ifdef NSS_DP_MHT_SW_PORT_MAP
	bool is_mht_dev;		/* MHT switch ports tx ring mapping flag */
#endif
#if defined(NSS_DP_MAC_POLL_SUPPORT)
	bool enable_polling_task;	/* enable SSDK PHY polling task */
#endif
};

/* Global data */
extern struct nss_dp_global_ctx dp_global_ctx;
extern struct nss_dp_data_plane_ctx dp_global_data_plane_ctx[NSS_DP_MAX_PORTS];
extern int nss_dp_rx_napi_budget;
extern int nss_dp_tx_napi_budget;

#if defined(NSS_DP_EDMA_V2)
extern int nss_dp_rx_fc_xon;
extern int nss_dp_rx_fc_xoff;
extern int nss_dp_rx_ac_fc_threshold;
extern int nss_dp_tx_mitigation_timer;
extern int nss_dp_tx_mitigation_pkt_cnt;
extern int nss_dp_rx_mitigation_timer;
extern int nss_dp_rx_mitigation_pkt_cnt;
extern uint8_t nss_dp_pri_map[EDMA_PRI_MAX];
extern int nss_dp_recovery_en;
#endif

#if defined(NSS_DP_NETSTANDBY)
extern struct nss_dp_standby_gbl_ctx gbl_ctx;
#endif

/*
 * nss data plane link state
 */
enum nss_dp_link_state {
	__NSS_DP_LINK_UP,	/* Indicate link is UP */
	__NSS_DP_LINK_DOWN	/* Indicate link is down */
};

/*
 * nss data plane status
 */
enum nss_dp_state {
	__NSS_DP_UP,		/* set to indicate the interface is UP	*/
	__NSS_DP_RXCSUM,	/* Rx checksum enabled			*/
	__NSS_DP_AUTONEG,	/* Autonegotiation Enabled		*/
	__NSS_DP_LINKPOLL,	/* Poll link status			*/
};

/*
 * nss data plane private flags
 */
enum nss_dp_priv_flags {
	__NSS_DP_PRIV_FLAG_INIT_DONE,
	__NSS_DP_PRIV_FLAG_IRQ_REQUESTED,
	__NSS_DP_PRIV_FLAG_INIT_OVERRIDE,
	__NSS_DP_PRIV_FLAG_MAX,
};
#define NSS_DP_PRIV_FLAG(x)	(1 << __NSS_DP_PRIV_FLAG_ ## x)

/*
 * nss_dp_set_ethtool_ops()
 */
void nss_dp_set_ethtool_ops(struct net_device *netdev);

/*
 * nss data plane switchdev helpers
 */
#ifdef CONFIG_NET_SWITCHDEV
void nss_dp_switchdev_setup(struct net_device *dev);
void nss_dp_switchdev_remove(struct net_device *dev);
bool nss_dp_is_phy_dev(struct net_device *dev);
#endif

/*
 * nss_dp_get_idx_from_macid()
 *	Get array index in data plane project from macid
 *
 * Note: We are not using MACID 7 for indexing.
 */
#if defined(NSS_DP_EDMA_V2)
static inline uint32_t nss_dp_get_idx_from_macid(uint32_t macid)
{
	if (likely(macid < NSS_DP_VP_MAC_ID)) {
		return (macid - 1);
	}

	return (macid - 2);
}

#else
static inline uint32_t nss_dp_get_idx_from_macid(uint32_t macid)
{
	return (macid - 1);
}
#endif

#endif	/* __NSS_DP_DEV_H__ */
