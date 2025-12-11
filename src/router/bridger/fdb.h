// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#ifndef __BRIDGER_FDB_H
#define __BRIDGER_FDB_H

struct fdb_key {
	uint8_t addr[ETH_ALEN];
	uint16_t vlan;
};

struct fdb_entry {
	struct avl_node node;

	struct fdb_key key;
	uint16_t ndm_state;
	bool updated;
	bool is_local;

	struct device *dev;
	struct list_head dev_list;

	struct list_head flows_in;
	struct list_head flows_out;
};

void fdb_init(struct bridge *br);
struct fdb_entry *fdb_get(struct bridge *br, const struct fdb_key *key);
struct fdb_entry *fdb_create(struct bridge *br, const struct fdb_key *key, struct device *dev);
void fdb_delete(struct bridge *br, struct fdb_entry *f);
void fdb_set_device(struct fdb_entry *f, struct device *dev);
void fdb_clear_flows(struct fdb_entry *f);

#endif
