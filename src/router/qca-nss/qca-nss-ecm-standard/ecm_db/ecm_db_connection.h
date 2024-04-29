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

#include <net/netfilter/nf_conntrack.h>
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
#include <ovsmgr.h>
#endif

/*
 * Magic number
 */
#define ECM_DB_CONNECTION_INSTANCE_MAGIC 0xff23

typedef uint32_t ecm_db_connection_hash_t;
typedef uint32_t ecm_db_connection_serial_hash_t;

/*
 * Events triggering a connection defunct.
 * This gives flexibillity to handle connection defunct process according to
 * the event that is triggering the defunct.
 */
enum ecm_db_connection_defunct_type {
	ECM_DB_CONNECTION_DEFUNCT_TYPE_STA_JOIN,		/* Defunct connection on STA join notification */
	ECM_DB_CONNECTION_DEFUNCT_TYPE_IGNORE,			/* Ignore the defunct type while defuncting the connection */
	ECM_DB_CONNECTION_DEFUNCT_TYPE_MAX,
};
typedef enum ecm_db_connection_defunct_type ecm_db_connection_defunct_type_t;

#ifdef ECM_DB_CTA_TRACK_ENABLE
/*
 * struct ecm_db_connection_classifier_type_assignment
 *	List linkage
 */
struct ecm_db_connection_classifier_type_assignment {
	struct ecm_db_connection_instance *next;	/* Next connection assigned to a classifier of this type */
	struct ecm_db_connection_instance *prev;	/* Previous connection assigned to a classifier of this type */
	int iteration_count;				/* >0 if something is examining this list entry and it may not be unlinked.  The connection will persist. */
	bool pending_unassign;				/* True when the connection has been unassigned from the type, when iteration_count drops to 0 it may be removed from the list */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};
#endif

/*
 * struct ecm_db_connection_defunct_info
 *	Information about event causing connection defunct based
 *	on a node.
 *	TODO: can be further expanded with more fields based on event causing
 *	node connection defunct.
 */
struct ecm_db_connection_defunct_info {
	ecm_db_connection_defunct_type_t type;			/* Connection defunct event type */
	uint8_t mac[ETH_ALEN];					/* MAC address */
	bool should_keep_connection;				/* should keep connection decision of classifer */
};

/*
 * struct ecm_db_connection_instance
 */
struct ecm_db_connection_instance {
	struct ecm_db_connection_instance *next;		/* Next instance in global list */
	struct ecm_db_connection_instance *prev;		/* Previous instance in global list */

	struct ecm_db_connection_instance *hash_next;		/* Next connection in chain */
	struct ecm_db_connection_instance *hash_prev;		/* Previous connection in chain */
	ecm_db_connection_hash_t hash_index;			/* The hash table slot whose chain of connections this is inserted into */

	struct ecm_db_connection_instance *serial_hash_next;	/* Next connection in serial hash chain */
	struct ecm_db_connection_instance *serial_hash_prev;	/* Previous connection in serial hash chain */
	ecm_db_connection_hash_t serial_hash_index;		/* The hash table slot whose chain of connections this is inserted into */

	uint32_t time_added;					/* RO: DB time stamp when the connection was added into the database */

	int ip_version;						/* RO: The version of IP protocol this connection was established for */
	int protocol;						/* RO: Protocol of the connection */
	ecm_db_direction_t direction;				/* RO: 'Direction' of connection establishment. */
	bool is_routed;						/* RO: True when connection is routed, false when not */
	bool timer_no_touch;					/* RO: Do no update timer when this flag is set */
	uint16_t l2_encap_proto;				/* L2 encap protocol of the flow of this connection */
	uint32_t mark;						/* The result value of mark classifier on this connection */

	/*
	 * Connection endpoint mapping
	 * NOTE: For non-NAT connections mapping[ECM_DB_OBJ_DIR_FROM_NAT] and mapping[ECM_DB_OBJ_DIR_TO_NAT]
	 * would be identical to the endpoint mappings.
	 */
	struct ecm_db_mapping_instance *mapping[ECM_DB_OBJ_DIR_MAX];	/* The connection was established on this mapping in the specified direction */

	/*
	 * From / To Node (NAT and non-NAT).
	 * Connections keep references to the nodes upon which they operate.
	 * Gut feeling would tell us this is unusual since it should be the case that
	 * the HOST refer to the node, e.g. IP address to a MAC address.
	 * However there are some 'interesting' usage models where the same IP address may appear
	 * from different nodes / MAC addresses because of this the unique element here is the connection
	 * and so we record the node information directly here.
	 */
	struct ecm_db_node_instance *node[ECM_DB_OBJ_DIR_MAX];	/* Node which this connection was established in the specified direction */

#ifdef ECM_DB_XREF_ENABLE
	/*
	 * The connection has references to the mappings (both nat and non-nat) as required above.
	 * Also mappings keep lists of connections made to/from them so that they may be iterated
	 * to determine associated connections in each direction/situation (e.g. "defuncting all connections made to/from a mapping").
	 */
	struct ecm_db_connection_instance *mapping_next[ECM_DB_OBJ_DIR_MAX];	/* Next connection made on the same mapping in the specified direction */
	struct ecm_db_connection_instance *mapping_prev[ECM_DB_OBJ_DIR_MAX];	/* Previous connection made on the same mapping in the specified direction */

	/*
	 * Connection endpoint interface
	 * NOTE: For non-NAT connections from/to would be identical to the endpoint interface.
	 * GGG TODO Deprecated - use interface lists instead.
	 * To be removed when interface heirarchies are implemented to provide the same functionality.
	 */
	struct ecm_db_connection_instance *iface_next[ECM_DB_OBJ_DIR_MAX];	/* Next connection made on the same interface with the specified direction*/
	struct ecm_db_connection_instance *iface_prev[ECM_DB_OBJ_DIR_MAX];	/* Previous connection made on the same interface with the specified direction */

	/*
	 * As well as keeping a reference to the node which this connection uses the nodes
	 * also keep lists of connections made from/to them.
	 */
	struct ecm_db_connection_instance *node_next[ECM_DB_OBJ_DIR_MAX];	/* Next connection in the nodes specified direction list */
	struct ecm_db_connection_instance *node_prev[ECM_DB_OBJ_DIR_MAX];	/* Prev connection in the nodes specified direction list */
#endif
	/*
	 * From / To interfaces list
	 * From NAT / To NAT interfaces list
	 * GGG TODO Not sure if NAT interface lists are necessary or appropriate or practical.
	 * Needs to be assessed if it gives any clear benefit and possibly remove these if not.
	 */
	struct ecm_db_iface_instance *interfaces[ECM_DB_OBJ_DIR_MAX][ECM_DB_IFACE_HEIRARCHY_MAX];
								/* The outermost to innnermost interface this connection is using in the specified direction
								 * which is defined in the first dimension of the array.
								 * Relationships are recorded from [ECM_DB_IFACE_HEIRARCHY_MAX - 1] to [0]
								 */
	int32_t interface_first[ECM_DB_OBJ_DIR_MAX];	/* The index of the first interface in the list */
	bool interface_set[ECM_DB_OBJ_DIR_MAX];		/* True when a list has been set - even if there is NO list, it's still deliberately set that way. */

#ifdef ECM_MULTICAST_ENABLE
	/*
	 * Destination Multicast interfaces list
	 */
	struct ecm_db_iface_instance *to_mcast_interfaces;
								/* The outermost to innnermost interfaces this connection is using in multicast path.
								 * The size of the buffer allocated for the to_mcast_interfaces heirarchies is as large as
								 * sizeof(struct ecm_db_iface_instance *) * ECM_DB_MULTICAST_IF_MAX * ECM_DB_IFACE_HEIRARCHY_MAX. */
	int32_t to_mcast_interface_first[ECM_DB_MULTICAST_IF_MAX];
								/* The indexes of the first interfaces in the destinaiton interface list */
	struct ecm_db_multicast_tuple_instance *ti; 		/* Multicast Connection instance */
	bool to_mcast_interfaces_set;				/* Flag to indicate if the destination interface list is currently empty or not */
#endif
	/*
	 * Time values in seconds
	 */
	struct ecm_db_timer_group_entry defunct_timer;		/* Used to defunct the connection on inactivity */

	/*
	 * Byte and packet counts
	 */
	uint64_t from_data_total;				/* Totals of data as sent by the 'from' side of this connection */
	uint64_t to_data_total;					/* Totals of data as sent by the 'to' side of this connection */
	uint64_t from_packet_total;				/* Totals of packets as sent by the 'from' side of this connection */
	uint64_t to_packet_total;				/* Totals of packets as sent by the 'to' side of this connection */
	uint64_t from_data_total_dropped;			/* Total data sent by the 'from' side that we purposely dropped - the 'to' side has not seen this data */
	uint64_t to_data_total_dropped;				/* Total data sent by the 'to' side that we purposely dropped - the 'from' side has not seen this data */
	uint64_t from_packet_total_dropped;			/* Total packets sent by the 'from' side that we purposely dropped - the 'to' side has not seen this data */
	uint64_t to_packet_total_dropped;			/* Total packets sent by the 'to' side that we purposely dropped - the 'from' side has not seen this data */

	/*
	 * Classifiers attached to this connection
	 */
	struct ecm_classifier_instance *assignments;		/* A list of all classifiers that are still assigned to this connection.
								 * When a connection is created, one instance of every type of classifier is assigned to the connection.
								 * Classifiers are added in ascending order of priority - so the most important processes a packet last.
								 * Classifiers may drop out of this list (become unassigned) at any time.
								 */
	struct ecm_classifier_instance *assignments_by_type[ECM_CLASSIFIER_TYPES];
								/* All assignments are also recorded in this array, since there can be only one of each type, this array allows
								 * rapid retrieval of a classifier type, saving having to iterate the assignments list.
								 */

#ifdef ECM_DB_CTA_TRACK_ENABLE
	struct ecm_db_connection_classifier_type_assignment type_assignment[ECM_CLASSIFIER_TYPES];
								/*
								 * Each classifier TYPE has a list of connections that are assigned to it.
								 * This permits a classifier TYPE to rapidly retrieve all connections associated with it.
								 */
#endif

	/*
	 * Re-generation.
	 * When system or classifier state changes, affected connections may need to have their state re-generated.
	 * This ensures that a connection does not continue to operate on stale state which could affect the sanity of acceleration rules.
	 * A connection needs to be re-generated when its regen_required is > 0.
	 * When a re-generation is completed successfully the counter is decremented.
	 * The counter ensures that any further changes of state while re-generation is under way is not missed.
	 * While a connection needs re-generation (regen_required > 0), acceleration should not be permitted.
	 * It may not always be practical to flag individual connections for re-generation (time consuming with large numbers of connections).
	 * The "generation" is a numerical counter comparison against the global "ecm_db_connection_generation".
	 * This ecm_db_connection_generation can be incremented causing a numerical difference between the connections counter and this global.
	 * This is enough to flag that a re-generation is needed.
	 * Further, it is possible that re-generation may be required DURING a rule construction.  Since constructing a rule
	 * can require lengthy non-atomic processes there needs to be a way to ensure that changes during construction of a rule are caught.
	 * The regen_occurances is a counter that is incremented whenever regen_required is also incremented.
	 * However it is never decremented.  This permits the caller to obtain this count before a non-atomic procedure and then afterwards.
	 * If there is any change in the counter value there is a change of generation!  And the operation should be aborted.
	 */
	bool regen_in_progress;					/* The connection is under regeneration right now and is used to provide atomic re-generation in SMP */
	uint16_t regen_required;				/* The connection needs to be re-generated when > 0 */
	uint16_t regen_occurances;				/* Total number of regens required */
	uint16_t generation;					/* Used to detect when a re-evaluation of this connection is necessary by comparing with ecm_db_connection_generation */
	uint32_t regen_success;					/* Tracks how many times re-generation was successfully completed */
	uint32_t regen_fail;					/* Tracks how many times re-generation failed */

	struct ecm_front_end_connection_instance *feci;		/* Front end instance specific to this connection */

	ecm_db_connection_final_callback_t final;		/* Callback to owner when object is destroyed */
	void *arg;						/* Argument returned to owner in callbacks */

	uint32_t serial;					/* RO: Serial number for the connection - unique for run lifetime */
	uint32_t flags;
	int refs;						/* Integer to trap we never go negative */
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

/*
 * Connection flags
 */
#define ECM_DB_CONNECTION_FLAGS_INSERTED 0x1			/* Connection is inserted into connection database tables */
#define ECM_DB_CONNECTION_FLAGS_PPPOE_BRIDGE 0x2		/* Connection is PPPoE bridge entry */
#define ECM_DB_CONNECTION_FLAGS_DEFUNCT_CT_DESTROYED 0x4	/* Connection is defuncted because of conntarck Destroyed */

int _ecm_db_connection_count_get(void);

int ecm_db_connection_count_get(void);
int ecm_db_connection_count_by_protocol_get(int protocol);
struct ecm_front_end_connection_instance *ecm_db_connection_front_end_get_and_ref(struct ecm_db_connection_instance *ci);

int ecm_db_connection_elapsed_defunct_timer(struct ecm_db_connection_instance *ci);
bool ecm_db_connection_defunct_timer_reset(struct ecm_db_connection_instance *ci, ecm_db_timer_group_t tg);
bool ecm_db_connection_defunct_timer_touch(struct ecm_db_connection_instance *ci);
void ecm_db_connection_defunct_timer_no_touch_set(struct ecm_db_connection_instance *ci);
bool ecm_db_connection_defunct_timer_no_touch_get(struct ecm_db_connection_instance *ci);
ecm_db_timer_group_t ecm_db_connection_timer_group_get(struct ecm_db_connection_instance *ci);
void ecm_db_connection_defunct_timer_remove_and_set(struct ecm_db_connection_instance *ci, ecm_db_timer_group_t tg);
void ecm_db_connection_make_defunct(struct ecm_db_connection_instance *ci);
void ecm_db_connection_data_totals_update(struct ecm_db_connection_instance *ci,
					  bool is_from, uint64_t size, uint64_t packets);
void ecm_db_connection_data_totals_update_dropped(struct ecm_db_connection_instance *ci,
						  bool is_from, uint64_t size, uint64_t packets);
void ecm_db_connection_data_stats_get(struct ecm_db_connection_instance *ci,
				      uint64_t *from_data_total, uint64_t *to_data_total,
				      uint64_t *from_packet_total, uint64_t *to_packet_total,
				      uint64_t *from_data_total_dropped, uint64_t *to_data_total_dropped,
				      uint64_t *from_packet_total_dropped, uint64_t *to_packet_total_dropped);

uint32_t ecm_db_connection_serial_get(struct ecm_db_connection_instance *ci);

void ecm_db_connection_address_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir, ip_addr_t addr);

int ecm_db_connection_port_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir);

void ecm_db_connection_node_address_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir, uint8_t *address_buffer);

void ecm_db_connection_iface_name_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir, char *name_buffer);

int ecm_db_connection_iface_mtu_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir);

ecm_db_iface_type_t ecm_db_connection_iface_type_get(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir);

uint16_t ecm_db_connection_regeneration_occurrances_get(struct ecm_db_connection_instance *ci);
void ecm_db_connection_regeneration_completed(struct ecm_db_connection_instance *ci);
void ecm_db_connection_regeneration_failed(struct ecm_db_connection_instance *ci);
bool ecm_db_connection_regeneration_required_check(struct ecm_db_connection_instance *ci);
bool ecm_db_connection_regeneration_required_peek(struct ecm_db_connection_instance *ci);
void ecm_db_connection_regeneration_needed(struct ecm_db_connection_instance *ci);
void ecm_db_regeneration_needed(void);
void ecm_db_connection_regenerate(struct ecm_db_connection_instance *ci);

ecm_db_direction_t ecm_db_connection_direction_get(struct ecm_db_connection_instance *ci);

int ecm_db_connection_protocol_get(struct ecm_db_connection_instance *ci);
int ecm_db_connection_ip_version_get(struct ecm_db_connection_instance *ci);
bool ecm_db_connection_is_routed_get(struct ecm_db_connection_instance *ci);
bool ecm_db_connection_is_pppoe_bridged_get(struct ecm_db_connection_instance *ci);

void _ecm_db_connection_ref(struct ecm_db_connection_instance *ci);
void ecm_db_connection_ref(struct ecm_db_connection_instance *ci);
int ecm_db_connection_deref(struct ecm_db_connection_instance *ci);

struct ecm_db_connection_instance *ecm_db_connections_get_and_ref_first(void);
struct ecm_db_connection_instance *ecm_db_connection_get_and_ref_next(struct ecm_db_connection_instance *ci);

void ecm_db_connection_defunct_all(void);
void ecm_db_connection_defunct_by_port(int port, ecm_db_obj_dir_t dir);
void ecm_db_connection_defunct_by_protocol(int protocol);
void ecm_db_connection_defunct_ip_version(int ip_version);

struct ecm_db_connection_instance *ecm_db_connection_serial_find_and_ref(uint32_t serial);
struct ecm_db_connection_instance *ecm_db_connection_find_and_ref(ip_addr_t host1_addr,
								  ip_addr_t host2_addr,
								  int protocol,
								  int host1_port,
								  int host2_port);

struct ecm_db_node_instance *ecm_db_connection_node_get_and_ref(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir);

struct ecm_db_mapping_instance *ecm_db_connection_mapping_get_and_ref(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir);

struct ecm_db_connection_instance *ecm_db_connection_iface_get_and_ref_next(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir);

struct ecm_db_connection_instance *ecm_db_connection_mapping_get_and_ref_next(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir);

void ecm_db_connection_interfaces_deref(struct ecm_db_iface_instance *interfaces[], int32_t first);

void ecm_db_connection_interfaces_reset(struct ecm_db_connection_instance *ci,
					struct ecm_db_iface_instance *interfaces[], int32_t new_first, ecm_db_obj_dir_t dir);

void ecm_db_connection_interfaces_clear(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir);

bool ecm_db_connection_interfaces_set_check(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir);

int32_t ecm_db_connection_interfaces_get_count(struct ecm_db_connection_instance *ci, ecm_db_obj_dir_t dir);

int32_t ecm_db_connection_interfaces_get_and_ref(struct ecm_db_connection_instance *ci,
						 struct ecm_db_iface_instance *interfaces[],
						 ecm_db_obj_dir_t dir);

void ecm_db_connection_classifier_assign(struct ecm_db_connection_instance *ci,
					 struct ecm_classifier_instance *new_ca);
int ecm_db_connection_classifier_assignments_get_and_ref(struct ecm_db_connection_instance *ci,
							 struct ecm_classifier_instance *assignments[]);
void ecm_db_connection_classifier_unassign(struct ecm_db_connection_instance *ci,
					   struct ecm_classifier_instance *cci);
void ecm_db_connection_assignments_release(int assignment_count,
					   struct ecm_classifier_instance *assignments[]);
struct ecm_classifier_instance *
ecm_db_connection_assigned_classifier_find_and_ref(struct ecm_db_connection_instance *ci,
						   ecm_classifier_type_t type);

#ifdef ECM_DB_CTA_TRACK_ENABLE
struct ecm_db_connection_instance *
ecm_db_connection_by_classifier_type_assignment_get_and_ref_first(ecm_classifier_type_t ca_type);
struct ecm_db_connection_instance *
ecm_db_connection_by_classifier_type_assignment_get_and_ref_next(struct ecm_db_connection_instance *ci,
								 ecm_classifier_type_t ca_type);
void ecm_db_connection_by_classifier_type_assignment_deref(struct ecm_db_connection_instance *ci,
							   ecm_classifier_type_t ca_type);
void ecm_db_connection_regenerate_by_assignment_type(ecm_classifier_type_t ca_type);
void ecm_db_connection_make_defunct_by_assignment_type(ecm_classifier_type_t ca_type);
#endif

struct ecm_db_connection_instance *ecm_db_connection_alloc(void);
void ecm_db_connection_add(struct ecm_db_connection_instance *ci,
			   struct ecm_db_mapping_instance *mapping[],
			   struct ecm_db_node_instance *node[],
			   int ip_version, int protocol, ecm_db_direction_t dir,
			   ecm_db_connection_final_callback_t final,
			   ecm_db_timer_group_t tg, bool is_routed, void *arg);

#ifdef ECM_STATE_OUTPUT_ENABLE
int ecm_db_connection_heirarchy_state_get(struct ecm_state_file_instance *sfi,
					  struct ecm_db_iface_instance *interfaces[],
					  int32_t first_interface);
int ecm_db_connection_state_get(struct ecm_state_file_instance *sfi,
				struct ecm_db_connection_instance *ci);
int ecm_db_connection_hash_table_lengths_get(int index);
int ecm_db_connection_hash_index_get_next(int index);
int ecm_db_connection_hash_index_get_first(void);
int ecm_db_protocol_get_next(int protocol);
int ecm_db_protocol_get_first(void);
#endif

struct ecm_db_connection_instance *ecm_db_connection_ipv4_from_ct_get_and_ref(struct nf_conn *ct);
struct ecm_db_connection_instance *ecm_db_connection_ipv6_from_ct_get_and_ref(struct nf_conn *ct);
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
struct ecm_db_connection_instance *ecm_db_connection_from_ovs_flow_get_and_ref(struct ovsmgr_dp_flow *flow);
#endif
bool ecm_db_connection_decel_v4(__be32 src_ip, int src_port,
				__be32 dest_ip, int dest_port, int protocol);
bool ecm_db_connection_decel_v6(struct in6_addr *src_ip, int src_port,
				struct in6_addr *dest_ip, int dest_port, int protocol);
void ecm_db_front_end_instance_ref_and_set(struct ecm_db_connection_instance *ci,
					   struct ecm_front_end_connection_instance *feci);

void ecm_db_connection_flag_set(struct ecm_db_connection_instance *ci, uint32_t flag);

void ecm_db_connection_l2_encap_proto_set(struct ecm_db_connection_instance *ci, uint16_t l2_encap_proto);
uint16_t ecm_db_connection_l2_encap_proto_get(struct ecm_db_connection_instance *ci);
void ecm_db_connection_mark_set(struct ecm_db_connection_instance *ci, uint32_t mark);
uint32_t ecm_db_connection_mark_get(struct ecm_db_connection_instance *ci);

void ecm_db_connection_defunct_by_classifier(int ip_ver, ip_addr_t src_addr, uint16_t src_port, ip_addr_t dest_addr,
						uint16_t dest_port, int proto, bool is_routed, ecm_classifier_type_t ca_type);

bool ecm_db_connection_init(struct dentry *dentry);
void ecm_db_connection_exit(void);
