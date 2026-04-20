/*
 **************************************************************************
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 **************************************************************************
 */

/*
 * Maximum MTU value which can be set in ECM's connection rules.
 */
#define ECM_DB_IFACE_MTU_MAX 65535

/*
 * Magic number
 */
#define ECM_DB_IFACE_INSTANCE_MAGIC 0xAEF1

typedef uint32_t ecm_db_iface_hash_t;
typedef uint32_t ecm_db_iface_id_hash_t;

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_db_iface_state_get_method_t
 *	Used to obtain interface state
 */
typedef int (*ecm_db_iface_state_get_method_t)(struct ecm_db_iface_instance *ii, struct ecm_state_file_instance *sfi);
#endif

/*
 * struct ecm_db_iface_instance
 */
struct ecm_db_iface_instance {
	struct ecm_db_iface_instance *next;		/* Next instance in global list */
	struct ecm_db_iface_instance *prev;		/* Previous instance in global list */
	struct ecm_db_iface_instance *hash_next;	/* Next Interface in the chain of Interfaces */
	struct ecm_db_iface_instance *hash_prev;	/* previous Interface in the chain of Interfaces */
	ecm_db_iface_type_t type;			/* RO: Type of interface */
	uint32_t time_added;				/* RO: DB time stamp when the Interface was added into the database */

	int32_t interface_identifier;			/* RO: The operating system dependent identifier of this interface */
	int32_t ae_interface_identifier;		/* RO: The accel engine identifier of this interface */
	char name[IFNAMSIZ];				/* Name of interface */
	int32_t mtu;					/* Interface MTU */

	struct ecm_db_iface_instance *iface_id_hash_next;	/* Next interface in the chain of interface id table */
	struct ecm_db_iface_instance *iface_id_hash_prev;	/* Previous interface in the chain of interface id table */
	ecm_db_iface_id_hash_t iface_id_hash_index;		/* Hash index value of chains */

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	uint64_t from_data_total;			/* Total of data sent by this Interface */
	uint64_t to_data_total;				/* Total of data sent to this Interface */
	uint64_t from_packet_total;			/* Total of packets sent by this Interface */
	uint64_t to_packet_total;			/* Total of packets sent to this Interface */
	uint64_t from_data_total_dropped;
	uint64_t to_data_total_dropped;
	uint64_t from_packet_total_dropped;
	uint64_t to_packet_total_dropped;
#endif

#ifdef ECM_DB_XREF_ENABLE
	/*
	 * For convenience interfaces keep lists of connections that have been established
	 * from them and to them.
	 * In fact the same connection could be listed as from & to on the same interface (think: WLAN<>WLAN AP function)
	 * Interfaces keep this information for rapid iteration of connections e.g. when an interface 'goes down' we
	 * can defunct all associated connections or destroy any accel engine rules.
	 */
	struct ecm_db_connection_instance *connections[ECM_DB_OBJ_DIR_MAX];
							/* list of connections made on this interface */

	/*
	 * Normally only the node refers to the interfaces which it is reachable upon.
	 * The interface  also keeps a list of all nodes that can be reached.
	 */
	struct ecm_db_node_instance *nodes;				/* Nodes associated with this Interface */
	int node_count;							/* Number of Nodes in the nodes list */
#endif

	/*
	 * Interface specific information.
	 * type identifies which information is applicable.
	 */
	union {
		struct ecm_db_interface_info_ethernet ethernet;		/* type == ECM_DB_IFACE_TYPE_ETHERNET */
#ifdef ECM_INTERFACE_VLAN_ENABLE
		struct ecm_db_interface_info_vlan vlan;			/* type == ECM_DB_IFACE_TYPE_VLAN */
#endif
#ifdef ECM_INTERFACE_MACVLAN_ENABLE
		struct ecm_db_interface_info_macvlan macvlan;		/* type == ECM_DB_IFACE_TYPE_MACVLAN */
#endif
#ifdef ECM_INTERFACE_BOND_ENABLE
		struct ecm_db_interface_info_lag lag;			/* type == ECM_DB_IFACE_TYPE_LAG */
#endif
		struct ecm_db_interface_info_bridge bridge;		/* type == ECM_DB_IFACE_TYPE_BRIDGE */
#ifdef ECM_INTERFACE_PPPOE_ENABLE
		struct ecm_db_interface_info_pppoe pppoe;		/* type == ECM_DB_IFACE_TYPE_PPPOE */
#endif
#ifdef ECM_INTERFACE_L2TPV2_ENABLE
		struct ecm_db_interface_info_pppol2tpv2 pppol2tpv2;	/* type == ECM_DB_IFACE_TYPE_PPPOL2TPV2 */
#endif
#ifdef ECM_INTERFACE_PPTP_ENABLE
		struct ecm_db_interface_info_pptp pptp;			/* type == ECM_DB_IFACE_TYPE_PPTP */
#endif
#ifdef ECM_INTERFACE_MAP_T_ENABLE
		struct ecm_db_interface_info_map_t map_t;		/* type == ECM_DB_IFACE_TYPE_MAP_T */
#endif
#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
		struct ecm_db_interface_info_gre_tun gre_tun;		/* type == ECM_DB_IFACE_TYPE_GRE_TUN */
#endif
		struct ecm_db_interface_info_unknown unknown;		/* type == ECM_DB_IFACE_TYPE_UNKNOWN */
		struct ecm_db_interface_info_loopback loopback;		/* type == ECM_DB_IFACE_TYPE_LOOPBACK */
#ifdef ECM_INTERFACE_IPSEC_ENABLE
		struct ecm_db_interface_info_ipsec_tunnel ipsec_tunnel;	/* type == ECM_DB_IFACE_TYPE_IPSEC_TUNNEL */
#endif
#ifdef ECM_INTERFACE_SIT_ENABLE
		struct ecm_db_interface_info_sit sit;			/* type == ECM_DB_IFACE_TYPE_SIT (6-in-4) */
#endif
#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
#ifdef ECM_IPV6_ENABLE
		struct ecm_db_interface_info_tunipip6 tunipip6;		/* type == ECM_DB_IFACE_TYPE_TUNIPIP6 (IPIP v6 Tunnel i.e. TUNNEL6) */
#endif
#endif
#ifdef ECM_INTERFACE_RAWIP_ENABLE
		struct ecm_db_interface_info_rawip rawip;		/* type ECM_DB_IFACE_TYPE_RAWIP */
#endif
#ifdef ECM_INTERFACE_OVPN_ENABLE
		struct ecm_db_interface_info_ovpn ovpn;			/* type == ECM_DB_IFACE_TYPE_OVPN (OpenVPN tunnel - data channel offload interface) */
#endif
#ifdef ECM_INTERFACE_VXLAN_ENABLE
		struct ecm_db_interface_info_vxlan vxlan;			/* type == ECM_DB_IFACE_TYPE_VXLAN */
#endif
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
		struct ecm_db_interface_info_ovs_bridge ovsb;			/* type == ECM_DB_IFACE_TYPE_OVS_BRIDGE */
#endif
	} type_info;

#ifdef ECM_STATE_OUTPUT_ENABLE
	ecm_db_iface_state_get_method_t state_get;	/* Type specific method to return state */
#endif

	ecm_db_iface_final_callback_t final;		/* Callback to owner when object is destroyed */
	void *arg;					/* Argument returned to owner in callbacks */
	uint32_t flags;
	int refs;					/* Integer to trap we never go negative */
	ecm_db_iface_hash_t hash_index;
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

int _ecm_db_iface_count_get(void);

void _ecm_db_iface_ref(struct ecm_db_iface_instance *ii);
void ecm_db_iface_ref(struct ecm_db_iface_instance *ii);
int ecm_db_iface_deref(struct ecm_db_iface_instance *ii);

int32_t ecm_db_iface_mtu_reset(struct ecm_db_iface_instance *ii, int32_t mtu);

void ecm_db_iface_identifier_hash_table_entry_check_and_update(struct ecm_db_iface_instance *ii,
							       int32_t new_interface_identifier);
int32_t ecm_db_iface_ae_interface_identifier_get(struct ecm_db_iface_instance *ii);
void ecm_db_iface_ae_interface_identifier_set(struct ecm_db_iface_instance *ii, uint32_t num);
void ecm_db_iface_update_ae_interface_identifier(struct ecm_db_iface_instance *ii,
						 int32_t ae_interface_identifier);

int32_t ecm_db_iface_interface_identifier_get(struct ecm_db_iface_instance *ii);
void ecm_db_iface_interface_name_get(struct ecm_db_iface_instance *ii, char *name_buffer);

void ecm_db_iface_ethernet_address_get(struct ecm_db_iface_instance *ii, uint8_t *address);
void ecm_db_iface_bridge_address_get(struct ecm_db_iface_instance *ii, uint8_t *address);

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
void ecm_db_iface_ovs_bridge_address_get(struct ecm_db_iface_instance *ii, uint8_t *address);
#endif

#ifdef ECM_INTERFACE_PPPOE_ENABLE
void ecm_db_iface_pppoe_session_info_get(struct ecm_db_iface_instance *ii,
					 struct ecm_db_interface_info_pppoe *pppoe_info);
#endif
#ifdef ECM_INTERFACE_VLAN_ENABLE
void ecm_db_iface_vlan_info_get(struct ecm_db_iface_instance *ii,
				struct ecm_db_interface_info_vlan *vlan_info);
#endif

struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_by_interface_identifier(int32_t interface_id);
struct ecm_db_iface_instance *ecm_db_iface_ifidx_find_and_ref_ethernet(uint8_t *address, int32_t idx, int32_t ae_interface_num);

#ifdef ECM_INTERFACE_RAWIP_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_rawip(uint8_t *address);
#endif

#ifdef ECM_INTERFACE_BOND_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_lag(uint8_t *address);
void ecm_db_iface_add_lag(struct ecm_db_iface_instance *ii,
			  uint8_t *address, char *name, int32_t mtu,
			  int32_t interface_identifier, int32_t ae_interface_identifier,
			  ecm_db_iface_final_callback_t final, void *arg);
void ecm_db_iface_lag_address_get(struct ecm_db_iface_instance *ii, uint8_t *address);
#endif

#ifdef ECM_INTERFACE_VLAN_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_vlan(uint8_t *address,
							     uint16_t vlan_tag,
							     uint16_t vlan_tpid);
void ecm_db_iface_add_vlan(struct ecm_db_iface_instance *ii,
			   uint8_t *address, uint16_t vlan_tag, uint16_t vlan_tpid,
			   char *name, int32_t mtu, int32_t interface_identifier,
			   int32_t ae_interface_identifier,
			   ecm_db_iface_final_callback_t final, void *arg);
#endif

struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_bridge(uint8_t *address, int32_t if_num);
#ifdef ECM_INTERFACE_MACVLAN_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_macvlan(uint8_t *address);
void ecm_db_iface_macvlan_address_get(struct ecm_db_iface_instance *ii, uint8_t *address);
#endif
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_ovs_bridge(uint8_t *address, int32_t if_num);
#endif
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_unknown(uint32_t os_specific_ident);

#ifdef ECM_INTERFACE_PPPOE_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_pppoe(uint16_t pppoe_session_id, uint8_t *remote_mac);
void ecm_db_iface_add_pppoe(struct ecm_db_iface_instance *ii,
			    uint16_t pppoe_session_id, uint8_t *remote_mac, char *name,
			    int32_t mtu, int32_t interface_identifier, int32_t ae_interface_identifier,
			    ecm_db_iface_final_callback_t final, void *arg);
#endif

#ifdef ECM_INTERFACE_L2TPV2_ENABLE
void ecm_db_iface_pppol2tpv2_session_info_get(struct ecm_db_iface_instance *ii,
					      struct ecm_db_interface_info_pppol2tpv2 *pppol2tpv2_info);
struct ecm_db_iface_instance *
ecm_db_iface_find_and_ref_pppol2tpv2(uint32_t pppol2tpv2_tunnel_id,
				     uint32_t pppol2tpv2_session_id);
void ecm_db_iface_add_pppol2tpv2(struct ecm_db_iface_instance *ii,
				 struct ecm_db_interface_info_pppol2tpv2 *pppol2tpv2_info,
				 char *name, int32_t mtu, int32_t interface_identifier,
				 int32_t ae_interface_identifier,
				 ecm_db_iface_final_callback_t final, void *arg);
#endif

#ifdef ECM_INTERFACE_MAP_T_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_map_t(int if_index, int32_t ae_interface_num);
void ecm_db_iface_add_map_t(struct ecm_db_iface_instance *ii,
			    struct ecm_db_interface_info_map_t *map_t_info, char *name,
			    int32_t mtu, int32_t interface_identifier, int32_t ae_interface_identifier,
			    ecm_db_iface_final_callback_t final, void *arg);
#endif

#ifdef ECM_INTERFACE_GRE_TUN_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_gre_tun(int if_index, int32_t ae_interface_num);
void ecm_db_iface_gre_tun_info_get(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_gre_tun *gre_tun_info);
void ecm_db_iface_add_gre_tun(struct ecm_db_iface_instance *ii,
				struct ecm_db_interface_info_gre_tun *gre_tun_info, char *name,
				int32_t mtu, int32_t interface_identifier, int32_t ae_interface_identifier,
				ecm_db_iface_final_callback_t final, void *arg);
#endif

#ifdef ECM_INTERFACE_PPTP_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_pptp(uint32_t pptp_src_call_id,
							     uint32_t pptp_dst_call_id,
							     int32_t ae_interface_num);
void ecm_db_iface_pptp_session_info_get(struct ecm_db_iface_instance *ii,
					struct ecm_db_interface_info_pptp *pptp_info);
void ecm_db_iface_add_pptp(struct ecm_db_iface_instance *ii,
			   struct ecm_db_interface_info_pptp *pptp_info, char *name,
			   int32_t mtu, int32_t interface_identifier,
			   int32_t ae_interface_identifier,
			   ecm_db_iface_final_callback_t final, void *arg);
#endif

struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_loopback(uint32_t os_specific_ident);

#ifdef ECM_INTERFACE_IPSEC_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_ipsec_tunnel(uint32_t os_specific_ident, int32_t ae_interface_num);
void ecm_db_iface_add_ipsec_tunnel(struct ecm_db_iface_instance *ii,
				   uint32_t os_specific_ident, char *name, int32_t mtu,
				   int32_t interface_identifier, int32_t ae_interface_identifier,
				   ecm_db_iface_final_callback_t final, void *arg);
#endif

#ifdef ECM_INTERFACE_SIT_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_sit(ip_addr_t saddr,
							    ip_addr_t daddr,
							    int32_t ae_interface_num);
bool ecm_db_iface_sit_daddr_is_null(struct ecm_db_iface_instance *ii);
void ecm_db_iface_add_sit(struct ecm_db_iface_instance *ii,
			  struct ecm_db_interface_info_sit *type_info, char *name,
			  int32_t mtu, int32_t interface_identifier,
			  int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final, void *arg);
#endif
#ifdef ECM_INTERFACE_TUNIPIP6_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_tunipip6(ip_addr_t saddr,
								 ip_addr_t daddr,
								 int32_t ae_interface_num);
void ecm_db_iface_add_tunipip6(struct ecm_db_iface_instance *ii,
				struct ecm_db_interface_info_tunipip6 *type_info, char *name,
				int32_t mtu, int32_t interface_identifier,
				int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final, void *arg);
#endif
#ifdef ECM_INTERFACE_OVPN_ENABLE
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_ovpn(int32_t tun_ifnum);
void ecm_db_iface_add_ovpn(struct ecm_db_iface_instance *ii,
				struct ecm_db_interface_info_ovpn *type_info, char *name,
				int32_t mtu, int32_t interface_identifier,
				ecm_db_iface_final_callback_t final, void *arg);
#endif

#ifdef ECM_INTERFACE_VXLAN_ENABLE
void ecm_db_iface_vxlan_info_get(struct ecm_db_iface_instance *ii, struct ecm_db_interface_info_vxlan *vxlan_info);
struct ecm_db_iface_instance *ecm_db_iface_find_and_ref_vxlan(uint32_t vni, uint32_t if_type);
void ecm_db_iface_add_vxlan(struct ecm_db_iface_instance *ii,
			   uint32_t vni, uint32_t if_type, char *name,
			   int32_t mtu, int32_t interface_identifier,
			   int32_t ae_interface_identifier,
			   ecm_db_iface_final_callback_t final, void *arg);
#endif

struct ecm_db_iface_instance *ecm_db_interfaces_get_and_ref_first(void);
struct ecm_db_iface_instance *ecm_db_interface_get_and_ref_next(struct ecm_db_iface_instance *ii);

#ifdef ECM_DB_XREF_ENABLE
int ecm_db_iface_node_count_get(struct ecm_db_iface_instance *ii);
struct ecm_db_node_instance *ecm_db_iface_nodes_get_and_ref_first(struct ecm_db_iface_instance *ii);
struct ecm_db_connection_instance *
ecm_db_iface_connections_get_and_ref_first(struct ecm_db_iface_instance *ii,
					   ecm_db_obj_dir_t dir);
#endif

struct ecm_db_iface_instance *ecm_db_iface_alloc(void);

void ecm_db_iface_add_ethernet(struct ecm_db_iface_instance *ii,
				uint8_t *address, char *name, int32_t mtu, int32_t interface_identifier,
				int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final, void *arg);

void ecm_db_iface_add_bridge(struct ecm_db_iface_instance *ii,
				uint8_t *address, char *name, int32_t mtu, int32_t interface_identifier,
				int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final, void *arg);

#ifdef ECM_INTERFACE_MACVLAN_ENABLE
void ecm_db_iface_add_macvlan(struct ecm_db_iface_instance *ii,
				uint8_t *address, char *name, int32_t mtu, int32_t interface_identifier,
				int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final, void *arg);
#endif
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
void ecm_db_iface_add_ovs_bridge(struct ecm_db_iface_instance *ii,
				uint8_t *address, char *name, int32_t mtu, int32_t interface_identifier,
				int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final, void *arg);
#endif

void ecm_db_iface_add_unknown(struct ecm_db_iface_instance *ii,
				uint32_t os_specific_ident, char *name, int32_t mtu, int32_t interface_identifier,
				int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final, void *arg);

void ecm_db_iface_add_loopback(struct ecm_db_iface_instance *ii,
				uint32_t os_specific_ident, char *name, int32_t mtu, int32_t interface_identifier,
				int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final, void *arg);

#ifdef ECM_INTERFACE_RAWIP_ENABLE
void ecm_db_iface_add_rawip(struct ecm_db_iface_instance *ii,
				uint8_t *address, char *name, int32_t mtu, int32_t interface_identifier,
				int32_t ae_interface_identifier, ecm_db_iface_final_callback_t final, void *arg);
#endif

char *ecm_db_interface_type_to_string(ecm_db_iface_type_t type);

ecm_db_iface_type_t ecm_db_iface_type_get(struct ecm_db_iface_instance *ii);

#ifdef ECM_STATE_OUTPUT_ENABLE
int ecm_db_iface_state_get(struct ecm_state_file_instance *sfi, struct ecm_db_iface_instance *ii);
int ecm_db_iface_hash_table_lengths_get(int index);
int ecm_db_iface_hash_index_get_next(int index);
int ecm_db_iface_hash_index_get_first(void);
#endif

bool ecm_db_iface_init(struct dentry *dentry);
