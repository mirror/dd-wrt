/*
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

#define PPE_DRV_PORT_SRC_PROFILE_MAX	4	/* Source profile for a port can be between 0-3 */
#define PPE_DRV_PHY_PORT_CHK(n) ((n) >= PPE_DRV_PHYSICAL_START && (n) < PPE_DRV_PHYSICAL_MAX)
#define PPE_DRV_VIRTUAL_PORT_CHK(n) ((n) >= PPE_DRV_VIRTUAL_START && (n) < PPE_DRV_PORTS_MAX)
#define PPE_DRV_PORT_VIRTUAL_L2_TUN		0x01	/* Port is L2 tunnel virtual port */
#define PPE_DRV_PORT_VIRTUAL_L3_TUN		0x02	/* Port is L3 tunnel virtual port */
#define PPE_DRV_PORT_SRC_PROFILE 0

/**
 * ppe_port_user_type
 *	User's VP type
 */
enum ppe_port_user_type {
	PPE_DRV_PORT_USER_TYPE_PASSIVE_VP = 1,		/**< VP for Passive use-case */
	PPE_DRV_PORT_USER_TYPE_ACTIVE_VP,		/**< VP for Active use-case */
	PPE_DRV_PORT_USER_TYPE_DS,			/**< VP for Direct-Switch use-case */
	PPE_DRV_PORT_USER_TYPE_MAX,			/**< Maximum VP User types */
};

/**
 * ppe_port_netdev_type
 *	Port's netdev type
 */
enum ppe_port_netdev_type {
	PPE_DRV_PORT_NETDEV_TYPE_WIFI = 1,		/**< Port netdev is Wi-Fi */
	PPE_DRV_PORT_NETDEV_TYPE_MAX,			/**< Maximum port netdev types */
};

/*
 * ppe_drv_port_flag
 *	Port flags
 */
typedef enum ppe_drv_port_flag {
	PPE_DRV_PORT_FLAG_IIPSEC = 0x1,
	PPE_DRV_PORT_FLAG_IDTLS = 0x2,
	PPE_DRV_PORT_FLAG_DS = 0x4,
	PPE_DRV_PORT_FLAG_MAX = 0x8,
	PPE_DRV_PORT_RFS_ENABLED = 0x10,
	PPE_DRV_PORT_FLAG_WIFI_DEV = 0x20,
	PPE_DRV_PORT_POLICER_ENABLED = 0x40,
	PPE_DRV_PORT_FLAG_OFFLOAD_ENABLED = 0x80,
	PPE_DRV_PORT_FLAG_REDIR_ENABLED = 0x100,
	PPE_DRV_PORT_FLAG_TUN_ENDPOINT_DS = 0x200,
} ppe_drv_port_flag_t;

/*
 * ppe_drv_port
 *	Port information
 */
struct ppe_drv_port {
	struct list_head l3_list;		/* List head of associated L3 interface */
	struct ppe_drv_l3_if *port_l3_if;	/* Port L3_IF */
	struct ppe_drv_l3_if *active_l3_if;	/* Active L3_IF */
	struct ppe_drv_vsi *port_vsi;		/* Pointer to Port's VSI */
	struct ppe_drv_vsi *br_vsi;		/* Pointer to Bridge's VSI TODO: To be added to the vsi_list*/
	struct ppe_l2_vp *l2_vp;		/* Pointer to L2 VP instance. */
	struct net_device *dev;			/* Associated netdev */
	struct kref ref_cnt;			/* Reference count object */
	struct ppe_drv_tun *port_tun;		/* PPE drv tun object associated with port */
	struct ppe_drv_tun_l3_if *tl_l3_if;	/* Tunnel L3 interface corresponding to this port entry */
	enum ppe_drv_port_type type;		/* Port type */
	uint32_t flags;				/* Port flags */
	bool ingress_untag_vlan;		/* Ingress VLAN rule for untag packets configured? */
	bool active_l3_if_attached;		/* Port L3_IF attached? */
	bool is_fdb_learn_enabled;		/* Port FDB learning enabled */
	uint16_t mtu;				/* MTU value of port */
	uint16_t mru;				/* MRU value of port */
	uint8_t mac_addr[ETH_ALEN];		/* MAC address of port */
	uint8_t port;				/* Port number */
	uint8_t mac_valid;			/* 1 if MAC address is valid */
	uint8_t src_profile;			/* Source profile of the port */
	uint8_t ucast_queue;			/* Base queue ID for the port */
	uint8_t tunnel_vp_cfg;			/* Port is of type tunnel VP */
	uint8_t active_vlan;			/* Number active VLAN configured on the port */
	struct ppe_drv_port_hw_stats stats;	/* PPE HW port statistics */
	uint8_t core_mask;			/* Core mask for VP flow */
	uint8_t shadow_core_mask;		/* Shadow Core mask for VP flow */
	uint8_t user_type;			/* PPE VP user type */
	uint8_t next_core;			/* Next core to pick for RFS */
	uint8_t xmit_port;			/* Physical port attached to virtual port */
};

void ppe_drv_port_ucast_queue_update(struct ppe_drv_port *pp, uint8_t queue_id);
uint8_t ppe_drv_port_ucast_queue_get(struct ppe_drv_port *pp);
bool ppe_drv_port_ucast_queue_set(struct ppe_drv_port *pp, uint8_t queue_id);
uint8_t ppe_drv_port_is_tunnel_vp(struct ppe_drv_port *pp);

struct net_device *ppe_drv_port_to_dev(struct ppe_drv_port *pp);
struct ppe_drv_port *ppe_drv_port_from_dev(struct net_device *dev);
struct ppe_drv_port *ppe_drv_port_from_tl_l3_if(struct ppe_drv_tun_l3_if *tl_l3_if);

void ppe_drv_port_mac_addr_set(struct ppe_drv_port *pp, const uint8_t *mac_addr);
void ppe_drv_port_mac_addr_clear(struct ppe_drv_port *pp);

bool ppe_drv_port_pp_mtu_cfg(struct ppe_drv_port *pp, bool enable);
bool ppe_drv_port_mtu_mru_set(struct ppe_drv_port *pp, uint16_t mtu, uint16_t mru);
void ppe_drv_port_mtu_mru_clear(struct ppe_drv_port *pp);
bool ppe_drv_port_mtu_cfg_update(struct ppe_drv_port *pp, uint16_t extra_hdr_len);
bool ppe_drv_port_mtu_mru_disable(struct ppe_drv_port *pp);
bool ppe_drv_port_mtu_disable(struct ppe_drv_port *pp);

struct ppe_drv_vsi *ppe_drv_port_find_vlan_vsi(struct ppe_drv_port *pp, uint32_t in_vlan, uint32_t out_vlan);
struct ppe_drv_vsi *ppe_drv_port_find_bridge_vsi(struct ppe_drv_port *pp);

struct ppe_drv_l3_if *ppe_drv_port_find_port_l3_if(struct ppe_drv_port *pp);
struct ppe_drv_l3_if *ppe_drv_port_find_pppoe_l3_if(struct ppe_drv_port *pp, uint16_t session_id, uint8_t *smac);

void ppe_drv_port_l3_if_detach(struct ppe_drv_port *pp, struct ppe_drv_l3_if *l3_if);
bool ppe_drv_port_l3_if_attach(struct ppe_drv_port *pp, struct ppe_drv_l3_if *l3_if);

void ppe_drv_port_vsi_detach(struct ppe_drv_port *pp, struct ppe_drv_vsi *vsi);
void ppe_drv_port_vsi_attach(struct ppe_drv_port *pp, struct ppe_drv_vsi *vsi);

bool ppe_drv_port_deref(struct ppe_drv_port *pp);
struct ppe_drv_port *ppe_drv_port_ref(struct ppe_drv_port *pp);
struct ppe_drv_port *ppe_drv_port_alloc(enum ppe_drv_port_type type, struct net_device *dev, uint8_t tunnel_vp_cfg);
struct ppe_drv_port *ppe_drv_port_phy_alloc(uint8_t port_num, struct net_device *dev);

void ppe_drv_port_entries_free(struct ppe_drv_port *port);
struct ppe_drv_port *ppe_drv_port_entries_alloc(void);
uint16_t ppe_drv_port_num_get(struct ppe_drv_port *pp);
bool ppe_drv_port_is_physical(struct ppe_drv_port *pp);
struct ppe_drv_port *ppe_drv_port_from_port_num(uint16_t port_num);

void ppe_drv_port_tl_l3_if_detach(struct ppe_drv_port *pp);
struct ppe_drv_tun_l3_if *ppe_drv_port_tl_l3_if_get_n_ref(struct ppe_drv_port *pp);
void ppe_drv_port_tl_l3_if_attach(struct ppe_drv_port *pp, struct ppe_drv_tun_l3_if *tl_l3_if);

void  ppe_drv_port_tun_set(struct ppe_drv_port *pp, struct ppe_drv_tun *ptun);
struct ppe_drv_tun *ppe_drv_port_tun_get(struct ppe_drv_port *pp);
bool ppe_drv_port_check_flow_offload_enabled(struct ppe_drv_port *drv_port);
bool ppe_drv_is_wlan_vp_port_type(uint8_t user_type);

/*
 * ppe_drv_port_flags_check()
 *      check the bit flags.
 */
static inline bool ppe_drv_port_flags_check(struct ppe_drv_port *pp, uint32_t flags)
{
	return (pp->flags & flags);
}

/*
 * ppe_drv_port_flags_clear()
 *	Clear a specific bit flag.
 */
static inline void ppe_drv_port_flags_clear(struct ppe_drv_port *pp, uint32_t flags)
{
	pp->flags &= ~flags;
}

/*
 * ppe_drv_port_flags_set()
 *	Set a specific bit flags.
 */
static inline void ppe_drv_port_flags_set(struct ppe_drv_port *pp, uint32_t flags)
{
	pp->flags |= flags;
}
