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

/*
 * Connection entry falgs.
 */
#define PPE_DRV_V6_CONN_FLAG_RETURN_VALID 0x00000001
					/* Return direction flow is valid. */
#define PPE_DRV_V6_CONN_FLAG_TYPE_MAPT	0x00000002
					/* Connection type is MAP-T */
/*
 * Bit flags for flow entry.
 */
#define PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW 0x00000001
					/* Bridge flow without routing in the path */
#define PPE_DRV_V6_CONN_FLOW_FLAG_DSCP_MARKING 0x00000002
					/* DSCP Marking Flag if IP DSCP value is to be changed */
#define PPE_DRV_V6_CONN_FLOW_FLAG_VLAN_PRI_MARKING 0x00000004
					/* Flow needs 802.1p marking */
#define PPE_DRV_V6_CONN_FLOW_FLAG_PPPOE_FLOW 0x00000008
					/* Flow is a PPPoE flow */
#define PPE_DRV_V6_CONN_FLOW_FLAG_QOS_VALID 0x00000010
					/* QoS valid */
#define PPE_DRV_V6_CONN_FLOW_FLAG_INLINE_IPSEC 0x00000020
					/* Inline IPSec flow */
#define PPE_DRV_V6_CONN_FLAG_FLOW_RFS_PPE_ASSIST 0x00000040
					/* Flow needs PPE assistance for RFS */
#define PPE_DRV_V6_CONN_FLOW_METADATA_TYPE_WIFI_INFO 0x00000080
					/* SAWF marking is valid for the flow */
#define PPE_DRV_V6_CONN_FLOW_FLAG_FSE 0x00000100
					/* Flow is also pushed to FSE HW in Wifi */
#define PPE_DRV_V6_CONN_FLAG_FLOW_VP_VALID 0x00000200
					/* Flow is VP valid when VP rule comes in DS user type */
#define PPE_DRV_V6_CONN_FLAG_BRIDGE_VLAN_NETDEV 0x00000400
					/* Flow is via bridge VLAN netdev */

#ifdef NSS_PPE_IPQ53XX
#define PPE_DRV_V6_CONN_FLOW_FLAG_SRC_INTERFACE_CHECK 0x00000400
					/* source interface check */
#endif
#define PPE_DRV_V6_CONN_FLAG_FLOW_IGMAC_VALID 0x00000800
#define PPE_DRV_V6_CONN_FLAG_FLOW_OFFLOAD_DISABLED 0x00001000
					/* Flow has the PPE offload disabled */

#define PPE_DRV_V6_CONN_FLAG_FLOW_ACL_VALID 0x00001000
					/* Flow + ACL combination match */
#define PPE_DRV_V6_CONN_FLAG_FLOW_POLICER_VALID 0x00002000
#define PPE_DRV_V6_CONN_FLAG_FLOW_PPE_POLICER_ASSIST 0x00004000
					/* Policer for NoEdit */
#define PPE_DRV_V6_CONN_FLAG_FLOW_PRIORITY_PPE_ASSIST 0x00008000
					/* Flow needs PPE assistance for Priority setting */
#define PPE_DRV_V6_CONN_FLAG_FLOW_NO_EDIT_RULE 0x00010000
					/* Flow is noedit rule */
#define PPE_DRV_V6_CONN_FLAG_FLOW_WIFI_DS	0x00020000
					/* Flow + MLO DS node */

/*
 * ppe_drv_v6_conn_flow
 *	Structure for individual flow direction
 */
struct ppe_drv_v6_conn_flow {
	struct ppe_drv_v6_conn *conn;		/* Pointer to parent structure */
	struct ppe_drv_flow *pf;		/* Flow pointer */
	uint16_t xmit_interface_mtu;		/* Interface MTU */
	uint8_t match_protocol;			/* Protocol */
	uint32_t match_src_ip[4];			/* Source IP address */
	uint32_t match_dest_ip[4];			/* Destination IP address */
	uint32_t match_src_ident;		/* Source port/connection ident */
	uint32_t match_dest_ident;		/* Destination port/connection ident */
	uint8_t xmit_dest_mac_addr[ETH_ALEN];	/* Destination MAC address after forwarding */

	/*
	 * Host order
	 */
	uint32_t dump_match_src_ip[4];		/* Source IP address */
	uint32_t dump_match_dest_ip[4];		/* Destination IP address */

	/*
	 * PPE to and from port
	 */
	struct ppe_drv_port *rx_port;		/* Rx ppe port */
	struct ppe_drv_port *tx_port;		/* Tx ppe port */

	/*
	 * VLAN headers
	*/
	struct ppe_drv_vlan ingress_vlan[PPE_DRV_MAX_VLAN];	/* Ingress VLAN headers */
	struct ppe_drv_vlan egress_vlan[PPE_DRV_MAX_VLAN];	/* Egress VLAN headers */
	uint8_t ingress_vlan_cnt;		/* Ingress active vlan headers count */
	uint8_t egress_vlan_cnt;		/* Egress active vlan headers count */

	/*
	 * PPPoE header
	 */
	uint16_t pppoe_session_id;		/**< Session id */
	uint8_t pppoe_server_mac[ETH_ALEN];	/**< Server MAC address */

	/*
	 * QOS info
	 */
	uint32_t int_pri;			/* For QoS */
	uint8_t egress_dscp;			/* Egress DSCP value */
	uint32_t flags;				/* Flags */

	/*
	 * Egress information
	 */
	struct ppe_drv_iface *eg_port_if;
	struct ppe_drv_iface *eg_l3_if;
	struct ppe_drv_iface *eg_vsi_if;
	struct ppe_drv_vsi *eg_top_vsi;

	/*
	 * Flow metadata information
	 */
	struct ppe_drv_flow_metadata flow_metadata;	/* Information about the flow metadata */

	/*
	 * Igress information
	 */
	struct ppe_drv_iface *in_port_if;
	struct ppe_drv_iface *in_l3_if;

	/*
	 * Flow + ACL info
	 */
	ppe_drv_sc_t acl_sc;
	uint16_t acl_id;
	uint16_t policer_hw_id;		/* HW policer index */
	uint16_t policer_id;		/* User policer index */

	uint8_t wifi_rule_ds_metadata;		/* Wi-Fi rule DS metadata */

	/*
	 * Statistics for this flow entry
	 */
	atomic_t rx_packets;			/* Number of Rx packets */
	atomic_t rx_bytes;			/* Number of Rx bytes */
	atomic_t tx_packets;			/* Number of Tx packets */
	atomic_t tx_bytes;			/* Number of Tx bytes */
};

/**
 * ppe_drv_v6_conn
 *	Structure to define a complete connection.
 */
struct ppe_drv_v6_conn {
	struct list_head list;
	struct ppe_drv_v6_conn_flow pcf;	/* flow object for flow direction */
	struct ppe_drv_v6_conn_flow pcr;	/* flow object for return direction */
	uint32_t flags;				/* connection flags */
	bool toggle;				/* Used during stats sync */
};

/*
 * ppe_drv_v6_addr_equal()
 *	Compare ipv6 address
 */
static inline bool ppe_drv_v6_addr_equal(uint32_t *a1, uint32_t *a2)
{
	return ((a1[0] ^ a2[0]) |
		(a1[1] ^ a2[1]) |
		(a1[2] ^ a2[2]) |
		(a1[3] ^ a2[3])) == 0;
}

/**
 * ppe_drv_v6_assist_stats_type
 * 	PPE ASSIST Stats type
 */
enum ppe_drv_v6_assist_stats_type {
	PPE_DRV_V6_ASSIST_CREATE_REQ,
	PPE_DRV_V6_ASSIST_CREATE_MEM_FAIL,
	PPE_DRV_V6_ASSIST_CREATE_CONN_FAIL,
	PPE_DRV_V6_ASSIST_CREATE_FLOW_COLL,
	PPE_DRV_V6_ASSIST_CREATE_FLOW_FAIL,
	PPE_DRV_V6_ASSIST_DESTROY_REQ,
	PPE_DRV_V6_ASSIST_DESTROY_FLOW_NOT_FOUND,
	PPE_DRV_V6_ASSIST_DESTROY_FLOW_FAIL,
};

/*
 * ppe_drv_v6_conn_alloc()
 *      Allocate v6 connections.
 */
static inline struct ppe_drv_v6_conn *ppe_drv_v6_conn_alloc(void)
{
	/*
	 * Allocate a new connection entry
	 *
	 * TODO: kzalloc with GFP_ATOMIC is used while considering sync method, in
	 * that case this API would be called from softirq.
	 *
	 * Revisit if we later handle this in a workqueue in async model.
	 */
	return kzalloc(sizeof(struct ppe_drv_v6_conn), GFP_ATOMIC);
}

/*
 * ppe_drv_v6_conn_free()
 *      Free v6 connections.
 */
static inline void ppe_drv_v6_conn_free(struct ppe_drv_v6_conn *cn)
{
	kfree(cn);
}

/*
 * ppe_drv_v6_conn_stats_alloc()
 *      Allocate v6 stats connections.
 */
static inline struct ppe_drv_v6_conn_sync *ppe_drv_v6_conn_stats_alloc(void)
{
	/*
	 * Allocate a new connection stats entry
	 */
	return kzalloc(sizeof(struct ppe_drv_v6_conn_sync), GFP_ATOMIC);
}

/*
 * ppe_drv_v6_conn_stats_free()
 *      Free v6 stats connections
 */
static inline void ppe_drv_v6_conn_stats_free(struct ppe_drv_v6_conn_sync *cns)
{
	kfree(cns);
}

/*
 * ppe_drv_v6_conn_flags_check()
 *      check the bit flags.
 */
static inline bool ppe_drv_v6_conn_flags_check(struct ppe_drv_v6_conn *cn, uint32_t flags)
{
        return (cn->flags & flags);
}

/*
 * ppe_drv_v6_conn_flags_clear()
 *	Clear a specific bit flag.
 */
static inline void ppe_drv_v6_conn_flags_clear(struct ppe_drv_v6_conn *cn, uint32_t flags)
{
        cn->flags &= ~flags;
}

/*
 * ppe_drv_v6_conn_flags_set()
 *	Set a specific bit flags.
 */
static inline void ppe_drv_v6_conn_flags_set(struct ppe_drv_v6_conn *cn, uint32_t flags)
{
        cn->flags |= flags;
}

/*
 * ppe_drv_v6_conn_flow_conn_get()
 *	Returns connection object.
 */
static inline struct ppe_drv_v6_conn *ppe_drv_v6_conn_flow_conn_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->conn;
}

/*
 * ppe_drv_v6_conn_flow_pf_get()
 *	Returns ppe flow instance.
 */
static inline struct ppe_drv_flow *ppe_drv_v6_conn_flow_pf_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->pf;
}

/*
 * ppe_drv_v6_conn_flow_xmit_interface_mtu_get()
 *	Returns xmit MTU.
 */
static inline uint16_t ppe_drv_v6_conn_flow_xmit_interface_mtu_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->xmit_interface_mtu;
}

/*
 * ppe_drv_v6_conn_flow_match_protocol_get()
 *	Returns IP protocol value.
 */
static inline uint8_t ppe_drv_v6_conn_flow_match_protocol_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->match_protocol;
}

/*
 * ppe_drv_v6_conn_flow_match_src_ip_get()
 *	Returns flow source IP.
 */
static inline void ppe_drv_v6_conn_flow_match_src_ip_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *src_ip)
{
        src_ip[0] = pcf->match_src_ip[0];
        src_ip[1] = pcf->match_src_ip[1];
        src_ip[2] = pcf->match_src_ip[2];
        src_ip[3] = pcf->match_src_ip[3];
}

/*
 * ppe_drv_v6_conn_flow_match_dest_ip_get()
 *	Returns flow destination IP.
 */
static inline void ppe_drv_v6_conn_flow_match_dest_ip_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *dest_ip)
{
        dest_ip[0] = pcf->match_dest_ip[0];
        dest_ip[1] = pcf->match_dest_ip[1];
        dest_ip[2] = pcf->match_dest_ip[2];
        dest_ip[3] = pcf->match_dest_ip[3];
}

/*
 * ppe_drv_v6_conn_flow_match_src_ident_get()
 *	Returns flow source l4 port.
 */
static inline uint32_t ppe_drv_v6_conn_flow_match_src_ident_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->match_src_ident;
}

/*
 * ppe_drv_v6_conn_flow_match_dest_ident_get()
 *	Returns flow destination l4 port.
 */
static inline uint32_t ppe_drv_v6_conn_flow_match_dest_ident_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->match_dest_ident;
}

/*
 * ppe_drv_v6_conn_flow_xmit_dest_mac_addr_get()
 *	Returns destination mac address.
 */
static inline uint8_t *ppe_drv_v6_conn_flow_xmit_dest_mac_addr_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->xmit_dest_mac_addr;
}

/*
 * ppe_drv_v6_conn_flow_rx_port_get()
 *	Returns ingress port.
 */
static inline struct ppe_drv_port *ppe_drv_v6_conn_flow_rx_port_get(struct ppe_drv_v6_conn_flow *pcf)
{
	return pcf->rx_port;
}

/*
 * ppe_drv_v6_conn_flow_tx_port_get()
 *	Returns egress port.
 */
static inline struct ppe_drv_port *ppe_drv_v6_conn_flow_tx_port_get(struct ppe_drv_v6_conn_flow *pcf)
{
	return pcf->tx_port;
}

/*
 * ppe_drv_v6_conn_flow_ingress_vlan_get()
 *	Returns ingress VLAN header(s).
 */
static inline struct ppe_drv_vlan *ppe_drv_v6_conn_flow_ingress_vlan_get(struct ppe_drv_v6_conn_flow *pcf, uint8_t index)
{
        return &pcf->ingress_vlan[index];
}

/*
 * ppe_drv_v6_conn_flow_egress_vlan_get()
 *	Returns egress VLAN header(s).
 */
static inline struct ppe_drv_vlan *ppe_drv_v6_conn_flow_egress_vlan_get(struct ppe_drv_v6_conn_flow *pcf, uint8_t index)
{
        return &pcf->egress_vlan[index];
}

/*
 * ppe_drv_v6_conn_flow_ingress_vlan_cnt_get()
 *	Returns ingress VLAN header count.
 */
static inline uint8_t ppe_drv_v6_conn_flow_ingress_vlan_cnt_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->ingress_vlan_cnt;
}

/*
 * ppe_drv_v6_conn_flow_egress_vlan_cnt_get()
 *	Returns egress VLAN header count.
 */
static inline uint8_t ppe_drv_v6_conn_flow_egress_vlan_cnt_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->egress_vlan_cnt;
}

/*
 * ppe_drv_v6_conn_flow_pppoe_session_id_get()
 *	Returns pppoe session ID.
 */
static inline uint16_t ppe_drv_v6_conn_flow_pppoe_session_id_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->pppoe_session_id;
}

/*
 * ppe_drv_v6_conn_flow_pppoe_server_mac_get()
 *	Returns pppoe server source MAC address.
 */
static inline uint8_t *ppe_drv_v6_conn_flow_pppoe_server_mac_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->pppoe_server_mac;
}

/*
 * ppe_drv_v6_conn_flow_int_pri_get()
 *	Returns internal priority associated with a flow.
 */
static inline uint32_t ppe_drv_v6_conn_flow_int_pri_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->int_pri;

}

/*
 * ppe_drv_v6_conn_flow_egress_dscp_get()
 *	Returns DSCP value associated with flow.
 */
static inline uint8_t ppe_drv_v6_conn_flow_egress_dscp_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->egress_dscp;
}

/*
 * ppe_drv_v6_conn_flow_eg_port_if_get()
 *	Returns egress port interface.
 */
static inline struct ppe_drv_iface *ppe_drv_v6_conn_flow_eg_port_if_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->eg_port_if;
}

/*
 * ppe_drv_v6_conn_flow_in_port_if_get()
 *	Returns ingress port interface.
 */
static inline struct ppe_drv_iface *ppe_drv_v6_conn_flow_in_port_if_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->in_port_if;
}

/*
 * ppe_drv_v6_conn_flow_eg_l3_if_get()
 *	Returns egress L3_IF interface.
 */
static inline struct ppe_drv_iface *ppe_drv_v6_conn_flow_eg_l3_if_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->eg_l3_if;
}

/*
 * ppe_drv_v6_conn_flow_in_l3_if_get()
 *	Returns ingress L3_IF interface.
 */
static inline struct ppe_drv_iface *ppe_drv_v6_conn_flow_in_l3_if_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->in_l3_if;
}

/*
 * ppe_drv_v6_conn_flow_eg_vsi_if_get()
 *	Returns egress VSI interface.
 */
static inline struct ppe_drv_iface *ppe_drv_v6_conn_flow_eg_vsi_if_get(struct ppe_drv_v6_conn_flow *pcf)
{
        return pcf->eg_vsi_if;
}

/*
 * ppe_drv_v6_conn_flow_eg_top_vsi_get()
 *	Returns egress top iface VSI.
 */
static inline struct ppe_drv_vsi *ppe_drv_v6_conn_flow_eg_top_vsi_get(struct ppe_drv_v6_conn_flow *pcf)
{
	return pcf->eg_top_vsi;
}

/*
 * ppe_drv_v6_conn_flow_conn_set()
 *	Sets connection object.
 */
static inline void ppe_drv_v6_conn_flow_conn_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_v6_conn *conn)
{
        pcf->conn = conn;
}

/*
 * ppe_drv_v6_conn_flow_pf_set()
 *	Sets ppe flow instance.
 */
static inline void ppe_drv_v6_conn_flow_pf_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_flow *pf)
{
        pcf->pf = pf;
}

/*
 * ppe_drv_v6_conn_flow_xmit_interface_mtu_set()
 *	Sets xmit MTU.
 */
static inline void ppe_drv_v6_conn_flow_xmit_interface_mtu_set(struct ppe_drv_v6_conn_flow *pcf, uint16_t xmit_interface_mtu)
{
        pcf->xmit_interface_mtu = xmit_interface_mtu;
}

/*
 * ppe_drv_v6_conn_flow_match_protocol_set()
 *	Sets IP protocol value.
 */
static inline void ppe_drv_v6_conn_flow_match_protocol_set(struct ppe_drv_v6_conn_flow *pcf, uint8_t match_protocol)
{
        pcf->match_protocol = match_protocol;
}

/*
 * ppe_drv_v6_conn_flow_match_src_ip_set()
 *	Sets flow source IP.
 */
static inline void ppe_drv_v6_conn_flow_match_src_ip_set(struct ppe_drv_v6_conn_flow *pcf, uint32_t match_src_ip[4])
{
        pcf->match_src_ip[0] = match_src_ip[0];
        pcf->match_src_ip[1] = match_src_ip[1];
        pcf->match_src_ip[2] = match_src_ip[2];
        pcf->match_src_ip[3] = match_src_ip[3];
}

/*
 * ppe_drv_v6_conn_flow_dump_match_src_ip_set()
 *	Sets flow source IP in Host order.
 */
static inline void ppe_drv_v6_conn_flow_dump_match_src_ip_set(struct ppe_drv_v6_conn_flow *pcf, uint32_t match_src_ip[4])
{
        pcf->dump_match_src_ip[0] = htonl(match_src_ip[0]);
        pcf->dump_match_src_ip[1] = htonl(match_src_ip[1]);
        pcf->dump_match_src_ip[2] = htonl(match_src_ip[2]);
        pcf->dump_match_src_ip[3] = htonl(match_src_ip[3]);
}

/*
 * ppe_drv_v6_conn_flow_match_dest_ip_set()
 *	Sets flow destination IP.
 */
static inline void ppe_drv_v6_conn_flow_match_dest_ip_set(struct ppe_drv_v6_conn_flow *pcf, uint32_t match_dest_ip[4])
{
        pcf->match_dest_ip[0] = match_dest_ip[0];
        pcf->match_dest_ip[1] = match_dest_ip[1];
        pcf->match_dest_ip[2] = match_dest_ip[2];
        pcf->match_dest_ip[3] = match_dest_ip[3];
}

/*
 * ppe_drv_v6_conn_flow_dump_match_dest_ip_set()
 *	Sets flow destination IP.
 */
static inline void ppe_drv_v6_conn_flow_dump_match_dest_ip_set(struct ppe_drv_v6_conn_flow *pcf, uint32_t match_dest_ip[4])
{
        pcf->dump_match_dest_ip[0] = htonl(match_dest_ip[0]);
        pcf->dump_match_dest_ip[1] = htonl(match_dest_ip[1]);
        pcf->dump_match_dest_ip[2] = htonl(match_dest_ip[2]);
        pcf->dump_match_dest_ip[3] = htonl(match_dest_ip[3]);
}

/*
 * ppe_drv_v6_conn_flow_match_src_ident_set()
 *	Sets flow source l4 port.
 */
static inline void ppe_drv_v6_conn_flow_match_src_ident_set(struct ppe_drv_v6_conn_flow *pcf, uint16_t match_src_ident)
{
        pcf->match_src_ident = match_src_ident;
}

/*
 * ppe_drv_v6_conn_flow_match_dest_ident_set()
 *	Sets flow destination l4 port.
 */
static inline void ppe_drv_v6_conn_flow_match_dest_ident_set(struct ppe_drv_v6_conn_flow *pcf, uint16_t match_dest_ident)
{
        pcf->match_dest_ident = match_dest_ident;
}

/*
 * ppe_drv_v6_conn_flow_xmit_dest_mac_addr_set()
 *	Sets destination mac address.
 */
static inline void ppe_drv_v6_conn_flow_xmit_dest_mac_addr_set(struct ppe_drv_v6_conn_flow *pcf, uint8_t *xmit_dest_mac_addr)
{
	memcpy(&pcf->xmit_dest_mac_addr, xmit_dest_mac_addr, ETH_ALEN);
}

/*
 * ppe_drv_v6_conn_flow_rx_port_set()
 *	Sets ingress port.
 */
static inline void ppe_drv_v6_conn_flow_rx_port_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_port *rx_port)
{
	pcf->rx_port = rx_port;
}

/*
 * ppe_drv_v6_conn_flow_tx_port_set()
 *	Sets egress port.
 */
static inline void ppe_drv_v6_conn_flow_tx_port_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_port *tx_port)
{
	pcf->tx_port = tx_port;
}

/*
 * ppe_drv_v6_conn_flow_pppoe_session_id_set()
 *	Sets pppoe session ID.
 */
static inline void ppe_drv_v6_conn_flow_pppoe_session_id_set(struct ppe_drv_v6_conn_flow *pcf, uint16_t pppoe_session_id)
{
        pcf->pppoe_session_id = pppoe_session_id;
}

/*
 * ppe_drv_v6_conn_flow_pppoe_server_mac_set()
 *	Sets pppoe server source MAC address.
 */
static inline void ppe_drv_v6_conn_flow_pppoe_server_mac_set(struct ppe_drv_v6_conn_flow *pcf, uint8_t *pppoe_server_mac)
{
	memcpy(&pcf->pppoe_server_mac, pppoe_server_mac, ETH_ALEN);
}

/*
 * ppe_drv_v6_conn_flow_int_pri_set()
 *	Sets internal priority associated with a flow.
 */
static inline void ppe_drv_v6_conn_flow_int_pri_set(struct ppe_drv_v6_conn_flow *pcf, uint32_t int_pri)
{
        pcf->int_pri = int_pri;

}

/*
 * ppe_drv_v6_conn_flow_egress_dscp_set()
 *	Sets DSCP value associated with flow.
 */
static inline void ppe_drv_v6_conn_flow_egress_dscp_set(struct ppe_drv_v6_conn_flow *pcf, uint8_t egress_dscp)
{
        pcf->egress_dscp = egress_dscp;
}

/*
 * ppe_drv_v6_conn_flow_eg_port_if_set()
 *	Sets egress port interface.
 */
static inline void ppe_drv_v6_conn_flow_eg_port_if_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_iface *eg_port_if)
{
        pcf->eg_port_if = eg_port_if;
}

/*
 * ppe_drv_v6_conn_flow_eg_l3_if_set()
 *	Sets egress L3_IF interface.
 */
static inline void ppe_drv_v6_conn_flow_eg_l3_if_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_iface *eg_l3_if)
{
        pcf->eg_l3_if = eg_l3_if;
}

/*
 * ppe_drv_v6_conn_flow_eg_vsi_if_set()
 *	Sets egress VSI interface.
 */
static inline void ppe_drv_v6_conn_flow_eg_vsi_if_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_iface *eg_vsi_if)
{
        pcf->eg_vsi_if = eg_vsi_if;
}

/*
 * ppe_drv_v6_conn_flow_eg_top_vsi_set()
 *	Sets egress top VSI interface.
 */
static inline void ppe_drv_v6_conn_flow_eg_top_vsi_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_vsi *eg_top_vsi)
{
	pcf->eg_top_vsi = eg_top_vsi;
}

/*
 * ppe_drv_v6_conn_flow_in_port_if_set()
 *	Sets ingress port interface.
 */
static inline void ppe_drv_v6_conn_flow_in_port_if_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_iface *in_port_if)
{
        pcf->in_port_if = in_port_if;
}

/*
 * ppe_drv_v6_conn_flow_in_l3_if_set_and_ref()
 *	Sets ingress L3_IF interface and take ref on iface
 */
static inline void ppe_drv_v6_conn_flow_in_l3_if_set_and_ref(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_iface *in_l3_if)
{
	kref_get(&in_l3_if->ref);
	pcf->in_l3_if = in_l3_if;
}

/*
 * ppe_drv_v6_conn_flow_flags_check()
 *      check the bit flags.
 */
static inline bool ppe_drv_v6_conn_flow_flags_check(struct ppe_drv_v6_conn_flow *pcf, uint32_t flags)
{
        return (pcf->flags & flags);
}

/*
 * ppe_drv_v6_conn_flow_flags_clear()
 *	Clear a specific bit flag.
 */
static inline void ppe_drv_v6_conn_flow_flags_clear(struct ppe_drv_v6_conn_flow *pcf, uint32_t flags)
{
        pcf->flags &= ~flags;
}

/*
 * ppe_drv_v6_conn_flow_flags_set()
 *	Set a specific bit flags.
 */
static inline void ppe_drv_v6_conn_flow_flags_set(struct ppe_drv_v6_conn_flow *pcf, uint32_t flags)
{
        pcf->flags |= flags;
}

/*
 * ppe_drv_v6_conn_flow_mc_min_mtu_get()
 *	Find and return the minimum MTU supported by interfaces in multicast interface list.
 */
static inline uint16_t ppe_drv_v6_conn_flow_mc_min_mtu_get(struct ppe_drv_v6_conn_flow *pcf)
{
	/*
	 * TODO: fix this with multiast support.
	 */
	return 1500;
}

/*
 * ppe_drv_v6_conn_flow_rx_stats_add()
 *	Add counters to Rx stats atomically.
 */
static inline void ppe_drv_v6_conn_flow_rx_stats_add(struct ppe_drv_v6_conn_flow *pcf, uint32_t rx_pkts, uint32_t rx_bytes)
{
	atomic_add(rx_pkts, &pcf->rx_packets);
	atomic_add(rx_bytes, &pcf->rx_bytes);
}

/*
 * ppe_drv_v6_conn_flow_tx_stats_add()
 *	Add counters to Tx stats atomically.
 */
static inline void ppe_drv_v6_conn_flow_tx_stats_add(struct ppe_drv_v6_conn_flow *pcf, uint32_t tx_pkts, uint32_t tx_bytes)
{
	atomic_add(tx_pkts, &pcf->tx_packets);
	atomic_add(tx_bytes, &pcf->tx_bytes);
}

/*
 * ppe_drv_v6_conn_flow_rx_stats_sub()
 *	Subtract counters from Rx stats atomically.
 */
static inline void ppe_drv_v6_conn_flow_rx_stats_sub(struct ppe_drv_v6_conn_flow *pcf, uint32_t rx_pkts, uint32_t rx_bytes)
{
	atomic_sub(rx_pkts, &pcf->rx_packets);
	atomic_sub(rx_bytes, &pcf->rx_bytes);
}

/*
 * ppe_drv_v6_conn_flow_tx_stats_sub()
 *	Subtract counters from Tx stats atomically.
 */
static inline void ppe_drv_v6_conn_flow_tx_stats_sub(struct ppe_drv_v6_conn_flow *pcf, uint32_t tx_pkts, uint32_t tx_bytes)
{
	atomic_sub(tx_pkts, &pcf->tx_packets);
	atomic_sub(tx_bytes, &pcf->tx_bytes);
}

/*
 * ppe_drv_v6_conn_flow_rx_stats_get()
 *	Get counters from Rx stats atomically.
 */
static inline void ppe_drv_v6_conn_flow_rx_stats_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *rx_pkts, uint32_t *rx_bytes)
{
	*rx_pkts = atomic_read(&pcf->rx_packets);
	*rx_bytes = atomic_read(&pcf->rx_bytes);
}

/*
 * ppe_drv_v6_conn_flow_tx_stats_sub()
 *	Get counters from Tx stats atomically.
 */
static inline void ppe_drv_v6_conn_flow_tx_stats_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *tx_pkts, uint32_t *tx_bytes)
{
	*tx_pkts = atomic_read(&pcf->tx_packets);
	*tx_bytes = atomic_read(&pcf->tx_bytes);
}

/*
 * IPv6 internal APIs.
 */
ppe_drv_ret_t ppe_drv_v6_conn_fill(struct ppe_drv_v6_rule_create *create, struct ppe_drv_v6_conn *cn,
				   enum ppe_drv_conn_type flow_type);
ppe_drv_ret_t ppe_drv_v6_flush(struct ppe_drv_v6_conn *cn);
void ppe_drv_v6_conn_stats_sync_invoke_cb(struct ppe_drv_v6_conn_sync *cns);
void ppe_drv_v6_conn_sync_one(struct ppe_drv_v6_conn *cn, struct ppe_drv_v6_conn_sync *cns,
								enum ppe_drv_stats_sync_reason reason);
void ppe_drv_v6_flow_vlan_set(struct ppe_drv_v6_conn_flow *pcf,
		uint32_t primary_ingress_vlan_tag, uint32_t primary_egress_vlan_tag,
		uint32_t secondary_ingress_vlan_tag, uint32_t secondary_egress_vlan_tag);
void ppe_drv_v6_if_walk_release(struct ppe_drv_v6_conn_flow *pcf);
bool ppe_drv_v6_if_walk(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_top_if_rule *top_if,
						ppe_drv_iface_t tx_if, ppe_drv_iface_t rx_if);
bool ppe_drv_v6_fse_flow_configure(struct ppe_drv_v6_rule_create *create, struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_v6_conn_flow *pcr);
void ppe_drv_fill_fse_v6_tuple_info(struct ppe_drv_v6_conn_flow *conn, struct ppe_drv_fse_rule_info *fse_info, bool is_ds);
bool ppe_drv_v6_fse_interface_check(struct ppe_drv_v6_conn_flow *pcf);
