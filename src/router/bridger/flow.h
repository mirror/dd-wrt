// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#ifndef __BRIDGER_FLOW_H
#define __BRIDGER_FLOW_H

struct bridger_flow {
	struct avl_node node;
	struct avl_node sort_node;
	struct avl_node offload_node;
	struct bridger_flow_key key;
	struct bridger_offload_flow offload;

	uint64_t avg_packets;
	uint64_t cur_packets;
	uint64_t offload_packets;
	int idle;

	uint16_t offload_ifindex;
	uint16_t offload_id;

	struct fdb_entry *fdb_in, *fdb_out;
	struct list_head fdb_in_list, fdb_out_list;
};

int bridger_flow_init(void);
void bridger_flow_delete(struct bridger_flow *flow);
void bridger_check_pending_flow(struct bridger_flow_key *key, struct bridger_pending_flow *val);

#endif
