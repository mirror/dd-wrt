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
 * struct ecm_db_listener_instance
 *	listener instances
 */
struct ecm_db_listener_instance {
	struct ecm_db_listener_instance *next;
	struct ecm_db_listener_instance *event_next;
	uint32_t flags;
	void *arg;
	int refs;							/* Integer to trap we never go negative */
	ecm_db_mapping_final_callback_t final;				/* Final callback for this instance */

	ecm_db_iface_listener_added_callback_t iface_added;
	ecm_db_iface_listener_removed_callback_t iface_removed;
	ecm_db_node_listener_added_callback_t node_added;
	ecm_db_node_listener_removed_callback_t node_removed;
	ecm_db_host_listener_added_callback_t host_added;
	ecm_db_host_listener_removed_callback_t host_removed;
	ecm_db_mapping_listener_added_callback_t mapping_added;
	ecm_db_mapping_listener_removed_callback_t mapping_removed;
	ecm_db_connection_listener_added_callback_t connection_added;
	ecm_db_connection_listener_removed_callback_t connection_removed;
#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

/*
 * Listener flags
 */
#define ECM_DB_LISTENER_FLAGS_INSERTED 1			/* Is inserted into database */

void ecm_db_listener_ref(struct ecm_db_listener_instance *li);
int ecm_db_listener_deref(struct ecm_db_listener_instance *li);

struct ecm_db_listener_instance *ecm_db_listener_alloc(void);

void ecm_db_listener_add(struct ecm_db_listener_instance *li,
			 ecm_db_iface_listener_added_callback_t iface_added,
			 ecm_db_iface_listener_removed_callback_t iface_removed,
			 ecm_db_node_listener_added_callback_t node_added,
			 ecm_db_node_listener_removed_callback_t node_removed,
			 ecm_db_host_listener_added_callback_t host_added,
			 ecm_db_host_listener_removed_callback_t host_removed,
			 ecm_db_mapping_listener_added_callback_t mapping_added,
			 ecm_db_mapping_listener_removed_callback_t mapping_removed,
			 ecm_db_connection_listener_added_callback_t connection_added,
			 ecm_db_connection_listener_removed_callback_t connection_removed,
			 ecm_db_listener_final_callback_t final, void *arg);

struct ecm_db_listener_instance *ecm_db_listeners_get_and_ref_first(void);
struct ecm_db_listener_instance *ecm_db_listener_get_and_ref_next(struct ecm_db_listener_instance *li);
