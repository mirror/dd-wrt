/*
 **************************************************************************
 * Copyright (c) 2014-2020, The Linux Foundation. All rights reserved.
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
 * Magic number
 */
#define ECM_DB_NODE_INSTANCE_MAGIC 0x3312

/*
 * Max size of directional connection count string printed
 * on the dump output.
 */
#define ECM_DB_NODE_CONN_COUNT_STR_SIZE 27

typedef uint32_t ecm_db_node_hash_t;

/*
 * struct ecm_db_node_instance
 */
struct ecm_db_node_instance {
	struct ecm_db_node_instance *next;		/* Next instance in global list */
	struct ecm_db_node_instance *prev;		/* Previous instance in global list */
	struct ecm_db_node_instance *hash_next;		/* Next node in the chain of nodes */
	struct ecm_db_node_instance *hash_prev;		/* previous node in the chain of nodes */
	uint8_t address[ETH_ALEN];			/* RO: MAC Address of this node */

#ifdef ECM_DB_XREF_ENABLE
	/*
	 * For convenience nodes keep lists of connections that have been established from them and to them.
	 * In fact the same connection could be listed as from & to on the same interface (think: WLAN<>WLAN AP function)
	 * Nodes keep this information for rapid iteration of connections e.g. when a node 'goes down' we
	 * can defunct all associated connections or destroy any accel engine rules.
	 */
	struct ecm_db_connection_instance *connections[ECM_DB_OBJ_DIR_MAX];
								/* list of connections made on this node */
	int connections_count[ECM_DB_OBJ_DIR_MAX];		/* Number of connections on this node with the direction specified in the index*/

	/*
	 * Nodes reachable from an interface are stored in a linked list maintained by that interface.
	 * This is so, given an interface, you can examine all nodes reachable from it.
	 */
	struct ecm_db_node_instance *node_next;				/* The next node within the same iface nodes list */
	struct ecm_db_node_instance *node_prev;				/* The previous node within the same iface nodes list */
#endif

	uint32_t time_added;				/* RO: DB time stamp when the node was added into the database */

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	uint64_t from_data_total;			/* Total of data sent by this node */
	uint64_t to_data_total;				/* Total of data sent to this node */
	uint64_t from_packet_total;			/* Total of packets sent by this node */
	uint64_t to_packet_total;			/* Total of packets sent to this node */
	uint64_t from_data_total_dropped;
	uint64_t to_data_total_dropped;
	uint64_t from_packet_total_dropped;
	uint64_t to_packet_total_dropped;
#endif
	struct ecm_db_iface_instance *iface;		/* The interface to which this node relates */

	ecm_db_node_final_callback_t final;		/* Callback to owner when object is destroyed */
	void *arg;					/* Argument returned to owner in callbacks */
	uint8_t flags;
	int refs;					/* Integer to trap we never go negative */
	ecm_db_node_hash_t hash_index;
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

int _ecm_db_node_count_get(void);

void _ecm_db_node_ref(struct ecm_db_node_instance *ni);
void ecm_db_node_ref(struct ecm_db_node_instance *ni);
int ecm_db_node_deref(struct ecm_db_node_instance *ni);

#ifdef ECM_DB_XREF_ENABLE
void ecm_db_traverse_node_connection_list_and_defunct(struct ecm_db_node_instance *node, ecm_db_obj_dir_t dir,
							int ip_version, ecm_db_connection_defunct_type_t event);

#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
void ecm_db_node_ovs_routed_connections_defunct(uint8_t *node_mac,
						struct net_device *ovs_br,
						int ip_version,
						ecm_db_obj_dir_t dir);
void ecm_db_traverse_snode_dnode_connection_list_and_defunct(
	struct ecm_db_node_instance *sni, uint8_t *dmac, int ip_version, ecm_db_obj_dir_t dir);
#endif
#endif

void ecm_db_node_adress_get(struct ecm_db_node_instance *ni, uint8_t *address_buffer);

struct ecm_db_iface_instance *ecm_db_node_iface_get_and_ref(struct ecm_db_node_instance *ni);
struct ecm_db_node_instance *ecm_db_node_find_and_ref(uint8_t *address, struct ecm_db_iface_instance *ii);

bool ecm_db_node_is_mac_addr_equal(struct ecm_db_node_instance *ni, uint8_t *address);

struct ecm_db_node_instance *ecm_db_nodes_get_and_ref_first(void);
struct ecm_db_node_instance *ecm_db_node_get_and_ref_next(struct ecm_db_node_instance *ni);

struct ecm_db_node_instance *ecm_db_node_chain_get_and_ref_first(uint8_t *address);
struct ecm_db_node_instance *ecm_db_node_chain_get_and_ref_next(struct ecm_db_node_instance *ni);

struct ecm_db_node_instance *ecm_db_node_alloc(void);
void ecm_db_node_add(struct ecm_db_node_instance *ni,
		     struct ecm_db_iface_instance *ii,
		     uint8_t *address,
		     ecm_db_node_final_callback_t final, void *arg);

#ifdef ECM_STATE_OUTPUT_ENABLE
int ecm_db_node_state_get(struct ecm_state_file_instance *sfi, struct ecm_db_node_instance *ni);
int ecm_db_node_hash_table_lengths_get(int index);
int ecm_db_node_hash_index_get_next(int index);
int ecm_db_node_hash_index_get_first(void);
#endif

int ecm_db_node_get_connections_count(struct ecm_db_node_instance *ni, ecm_db_obj_dir_t dir);

void ecm_db_node_ovs_connections_masked_defunct(int ip_ver, uint8_t *src_mac, bool src_mac_check, ip_addr_t src_addr_mask,
							uint16_t src_port_mask, uint8_t *dest_mac, bool dest_mac_check,
							ip_addr_t dest_addr_mask, uint16_t dest_port_mask,
							int proto_mask, ecm_db_obj_dir_t dir, bool is_routed);

bool ecm_db_node_init(struct dentry *dentry);
void ecm_db_node_exit(void);
