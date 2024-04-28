/*
 **************************************************************************
 * Copyright (c) 2014-2018, The Linux Foundation. All rights reserved.
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
 **************************************************************************
 */

/*
 * Magic number
 */
#define ECM_DB_MAPPING_INSTANCE_MAGIC 0x8765

typedef uint32_t ecm_db_mapping_hash_t;

/*
 * struct ecm_db_mapping_instance
 */
struct ecm_db_mapping_instance {
	struct ecm_db_mapping_instance *next;				/* Next instance in global list */
	struct ecm_db_mapping_instance *prev;				/* Previous instance in global list */

	struct ecm_db_mapping_instance *hash_next;			/* Next mapping in the chain of mappings */
	struct ecm_db_mapping_instance *hash_prev;			/* previous mapping in the chain of mappings */

	uint32_t time_added;						/* RO: DB time stamp when the connection was added into the database */
	struct ecm_db_host_instance *host;				/* The host to which this mapping relates */
	int port;							/* RO: The port number on the host - only applicable for mapping protocols that are port based */

#ifdef ECM_DB_XREF_ENABLE
	/*
	 * For convenience mappings keep lists of connections that have been established from them and to them.
	 * In fact the same connection could be listed as from & to on the same interface (think: WLAN<>WLAN AP function)
	 * Mappings keep this information for rapid iteration of connections e.g. given a mapping we
	 * can defunct all associated connections or destroy any accel engine rules.
	 */
	struct ecm_db_connection_instance *connections[ECM_DB_OBJ_DIR_MAX];	/* list of connections made on this host mapping in the specified direction*/

	/*
	 * While a mapping refers to the host it requires.
	 * The host also keeps a list of all mappings that are associated with it, this is that list linkage.
	 */
	struct ecm_db_mapping_instance *mapping_next;			/* Next mapping in the list of mappings for the host */
	struct ecm_db_mapping_instance *mapping_prev;			/* previous mapping in the list of mappings for the host */
#endif

	/*
	 * Connection counts
	 */
	int tcp_count[ECM_DB_OBJ_DIR_MAX];	/* Number of TCP connections made in the specified direction */
	int udp_count[ECM_DB_OBJ_DIR_MAX];	/* Number of UDP connections made in the specified direction */
	int conn_count[ECM_DB_OBJ_DIR_MAX];	/* Number of connections made in the specified direction */

#ifdef ECM_DB_ADVANCED_STATS_ENABLE
	/*
	 * Data totals
	 */
	uint64_t from_data_total;					/* Total of data sent by this mapping */
	uint64_t to_data_total;						/* Total of data sent to this mapping */
	uint64_t from_packet_total;					/* Total of packets sent by this mapping */
	uint64_t to_packet_total;					/* Total of packets sent to this mapping */
	uint64_t from_data_total_dropped;
	uint64_t to_data_total_dropped;
	uint64_t from_packet_total_dropped;
	uint64_t to_packet_total_dropped;
#endif

	ecm_db_mapping_final_callback_t final;				/* Callback to owner when object is destroyed */
	void *arg;							/* Argument returned to owner in callbacks */
	uint32_t flags;
	int refs;							/* Integer to trap we never go negative */
	ecm_db_mapping_hash_t hash_index;
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

int _ecm_db_mapping_count_get(void);

void _ecm_db_mapping_ref(struct ecm_db_mapping_instance *mi);
void ecm_db_mapping_ref(struct ecm_db_mapping_instance *mi);
int ecm_db_mapping_deref(struct ecm_db_mapping_instance *mi);

#ifdef ECM_STATE_OUTPUT_ENABLE
int ecm_db_mapping_state_get(struct ecm_state_file_instance *sfi, struct ecm_db_mapping_instance *mi);
int ecm_db_mapping_hash_table_lengths_get(int index);
int ecm_db_mapping_hash_index_get_next(int index);
int ecm_db_mapping_hash_index_get_first(void);
#endif

void ecm_db_mapping_adress_get(struct ecm_db_mapping_instance *mi, ip_addr_t addr);
int ecm_db_mapping_port_get(struct ecm_db_mapping_instance *mi);

int ecm_db_mapping_connections_total_count_get(struct ecm_db_mapping_instance *mi);
struct ecm_db_host_instance *ecm_db_mapping_host_get_and_ref(struct ecm_db_mapping_instance *mi);
struct ecm_db_mapping_instance *ecm_db_mapping_find_and_ref(ip_addr_t address, int port);
struct ecm_db_mapping_instance *ecm_db_mappings_get_and_ref_first(void);
struct ecm_db_mapping_instance *ecm_db_mapping_get_and_ref_next(struct ecm_db_mapping_instance *mi);

struct ecm_db_connection_instance *ecm_db_mapping_connections_get_and_ref_first(struct ecm_db_mapping_instance *mi, ecm_db_obj_dir_t dir);

struct ecm_db_mapping_instance *ecm_db_mapping_alloc(void);
void ecm_db_mapping_add(struct ecm_db_mapping_instance *mi, struct ecm_db_host_instance *hi, int port, ecm_db_mapping_final_callback_t final, void *arg);

bool ecm_db_mapping_init(struct dentry *dentry);
void ecm_db_mapping_exit(void);
