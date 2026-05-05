/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#ifndef __EIP_FLOW_H
#define __EIP_FLOW_H

#include <linux/debugfs.h>

#define EIP_FLOW_MAX (1U << (EIP_HW_FLUE_CONFIG_TABLE_SZ + 5))
#define EIP_FLOW_MASK (EIP_FLOW_MAX - 1)
#define EIP_FLOW_HASH_IDX(hash) ((hash.words[0] >> 6) & EIP_FLOW_MASK)
#define EIP_FLOW_HASH_DATA_SZ 13U /* Iterations for hash function */
#define EIP_FLOW_MAX_COLLISION 64U
#define EIP_HASH_EQUAL(H1,H2) ((H1.words[0] == H2.words[0]) && (H1.words[1] == H2.words[1]) && (H1.words[2] == H2.words[2]) && (H1.words[3] == H2.words[3]))
#define EIP_FLOW_TR_DISABLE 0x0
#define EIP_FLOW_VALID_BIT 0x1U

/*
 * eip_flow_hash_t
 * 	Structure to store 128bit hash
 */
typedef struct {
	uint32_t words[4];
} eip_flow_hash_t;

/*
 * eip_flow_hw
 *      Hw flow
 */
struct eip_flow_hw {
	eip_flow_hash_t hashid_1;	/* For storing the hash value */
	eip_flow_hash_t reserved_0;
	eip_flow_hash_t reserved_1;
	uint32_t tr_addr_type_1;	/* Record offset for transform record */
	uint32_t reserved_addr_0;
	uint32_t reserved_addr_1;
	uint32_t next_flow_offset;	/* Bucket offset in the presence of collision */
}__attribute((__packed__));

/*
 * eip_flow
 *      SW flow object
 */
struct eip_flow {
	struct hlist_node h_node;	/* Hlist node to traverse the collision list */
	struct list_head t_node;	/* List node for per-device tunnel flows (encap/decap) */
	struct eip_tr *tr;		/* Transform record associated with this flow */
	struct eip_flow_hw *hflow;      /* Pointer to HW flow */
	dma_addr_t hflow_paddr;         /* Physical address of hardware flow */
	eip_flow_hash_t hash;           /* Hash value calculated from flow tuple */
	struct eip_flow_tuple tuple;    /* Flow tuple */
	bool sentinel;                  /* Sentinel to check if a node is the head node */
	struct eip_hw_stats stats;      /* Accumulated HW stats (sum of deltas from eip_tr_get_hw_stats) */
};

/*
 * eip_flow_tbl
 * 	Flow table
 */
struct eip_flow_tbl {
	struct hlist_head sw_head[EIP_FLOW_MAX];        /* Pointer to SW flow object in the flow table */
	struct eip_flow_hw *hw_head;                    /* Pointer to HW flow in the flow table */
	dma_addr_t hw_head_paddr;                       /* Physical address of the flow table */
	struct eip_flow_tbl_stats {
		uint64_t alloc;                         /* Counter for number of flows allocated */
		uint64_t free;                          /* Counter for number of flows freed */
		uint64_t collision;                     /* Counter for total number of collisions */
		uint64_t active_collision;              /* Counter for total collisions that are active */
		uint64_t fail_collision;		/* Counter for total collosions fail */
	} stats;
	struct dentry *dentry;				/* Debugfs dentry for flow table dump */
};

bool eip_flow_table_init(struct platform_device *pdev);
void eip_flow_table_deinit(struct platform_device *pdev);
int eip_flow_update_by_tr(struct eip_tun *tun, struct eip_tr *tr, struct eip_tr *new_tr);

#endif /* __EIP_FLOW_H */
