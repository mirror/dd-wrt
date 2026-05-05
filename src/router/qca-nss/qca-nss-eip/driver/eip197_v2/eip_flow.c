/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/dma-mapping.h>
#include <linux/crypto.h>
#include <linux/cache.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/dmapool.h>
#include <linux/dma-direct.h>
#include <linux/spinlock.h>
#include <linux/seq_file.h>
#include <linux/log2.h>
#include <linux/errno.h>
#include <eip.h>

#include "eip_priv.h"
#include "eip_flow.h"

/*
 * Name for the debugfs entry for the flow table
 */
#define FLOW_TBL_DUMP_DEBUF_FS "eip_flow_table"

/*
 * eip_flow_dump_entry()
 *	Helper function to print a single flow entry.
 */
static void eip_flow_dump_entry(struct seq_file *s, struct eip_flow *flow,
				 uint32_t index, uint32_t flow_count)
{
	const char *ip_version;
	const char *ip_str;

	ip_version = (flow->tuple.ip_ver == 4) ? "IPv4" : "IPv6";

	seq_printf(s, "================================================================================\n");
	seq_printf(s, "Flow Entry #%u (Bucket: %u)\n", flow_count, index);
	seq_printf(s, "================================================================================\n");

	/*
	 * Flow identification and hardware information
	 */
	seq_printf(s, "Hardware Information:\n");
	seq_printf(s, "  Physical Address    : %pad\n", &flow->hflow_paddr);

	seq_printf(s, "  Hash ID [0]         : [0x%08x] [0x%08x] [0x%08x] [0x%08x]\n",
		   flow->hflow->hashid_1.words[0], flow->hflow->hashid_1.words[1],
		   flow->hflow->hashid_1.words[2], flow->hflow->hashid_1.words[3]);

	seq_printf(s, "  Hash ID [1]         : [0x%08x] [0x%08x] [0x%08x] [0x%08x]\n",
		   flow->hflow->reserved_0.words[0], flow->hflow->reserved_0.words[1],
		   flow->hflow->reserved_0.words[2], flow->hflow->reserved_0.words[3]);

	seq_printf(s, "  Hash ID [2]         : [0x%08x] [0x%08x] [0x%08x] [0x%08x]\n",
		   flow->hflow->reserved_1.words[0], flow->hflow->reserved_1.words[1],
		   flow->hflow->reserved_1.words[2], flow->hflow->reserved_1.words[3]);

	seq_printf(s, "  Address Type [0]    : 0x%08x\n", flow->hflow->tr_addr_type_1);
	seq_printf(s, "  Address Type [1]    : 0x%08x\n", flow->hflow->reserved_addr_0);
	seq_printf(s, "  Address Type [2]    : 0x%08x\n", flow->hflow->reserved_addr_1);

	/*
	 * Network tuple information
	 */
	seq_printf(s, "\nNetwork Tuple (%s):\n", ip_version);

	ip_str = (flow->tuple.ip_ver == 4) ? "%pI4\n" : "%pI6\n";

	seq_printf(s, "  Source Address      : ");
	seq_printf(s, ip_str, &flow->tuple.src_ip[0]);

	seq_printf(s, "  Destination Address : ");
	seq_printf(s, ip_str, &flow->tuple.dst_ip[0]);

	seq_printf(s, "  Protocol            : %u\n", flow->tuple.ip_proto);
	seq_printf(s, "  Source Port         : %u\n", flow->tuple.src_port);
	seq_printf(s, "  Destination Port    : %u\n", flow->tuple.dst_port);
	seq_printf(s, "  SPI Index           : 0x%08x\n", flow->tuple.spi);
	seq_printf(s, "  Epoch               : %u\n", flow->tuple.epoch);

	seq_printf(s, "================================================================================\n\n");
}

/*
 * eip_flow_dump()
 *	dump all flow entries.
 */
static int eip_flow_dump(struct seq_file *s, void *data)
{
	struct eip_pdev *ep = platform_get_drvdata(eip_drv_g.pdev);
	struct eip_flow_tbl *flow_db = &ep->flow_table;
	struct eip_drv *drv = &eip_drv_g;
	struct hlist_head *head;
	struct eip_flow *flow;
	uint32_t flow_cnt = 0;
	uint32_t index;

	/*
	 * Print header information
	 */
	seq_printf(s, "EIP Flow Table Dump\n");
	seq_printf(s, "===================\n");
	seq_printf(s, "Maximum Buckets: %u\n\n", drv->flow_max);

	/*
	 * Iterate through all buckets in the flow table.
	 * Lock is acquired and released per bucket to avoid holding
	 * the lock for too long when dumping large number of flows.
	 */
	for (index = 0; index < drv->flow_max; index++) {
		spin_lock_bh(&ep->lock);

		head = &flow_db->sw_head[index];

		if (hlist_empty(head)) {
			spin_unlock_bh(&ep->lock);
			continue;
		}

		/*
		 * Iterate through all flows in this bucket
		 */
		hlist_for_each_entry(flow, head, h_node) {
			flow_cnt++;
			eip_flow_dump_entry(s, flow, index, flow_cnt);
		}

		spin_unlock_bh(&ep->lock);

		/*
		 * Yield CPU to prevent monopolizing it during large dumps
		 */
		cond_resched();
	}

	/*
	 * Print summary information
	 */
	seq_printf(s, "================================================================================\n");
	seq_printf(s, "Summary: Total Active Flows = %u\n", flow_cnt);
	seq_printf(s, "================================================================================\n");

	return 0;
}

/*
 * eip_flow_dump_open()
 *	open operation for debugfs.
 */
static int eip_flow_dump_open(struct inode *inode, struct file *file)
{
	return single_open(file, eip_flow_dump, NULL);
}

/*
 * File operations for the debugfs.
 */
static struct file_operations flow_dump_ops = {
	.owner = THIS_MODULE,
	.open = eip_flow_dump_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/*
 * eip_flow_dump_init()
 *	Initialize the debugfs interface for flow table dumping.
 */
static int eip_flow_dump_init(struct eip_flow_tbl *tbl, struct eip_pdev *ep)
{
	tbl->dentry = debugfs_create_file(FLOW_TBL_DUMP_DEBUF_FS, S_IRUGO, ep->dentry, NULL, &flow_dump_ops);
	if (!tbl->dentry) {
		pr_info("Falied to allocate debugfd : %s\n", FLOW_TBL_DUMP_DEBUF_FS);
		return -ENOMEM;
	}

	return 0;
}

/*
 * eip_flow_dump_deinit()
 *	Remove debugfs interface.
 */
static void eip_flow_dump_deinit(struct eip_flow_tbl *tbl)
{
	if (tbl->dentry)
		debugfs_remove_recursive(tbl->dentry);
}

/*
 * eip_flow_get_tail
 * 	Obtain the flow at the tail
 */
static struct eip_flow *eip_flow_get_tail(struct hlist_head *flow_head)
{
	struct hlist_node *entry;
	hlist_for_each(entry, flow_head) {
		if (entry->next == NULL) {
			return hlist_entry(entry, struct eip_flow, h_node);
		}
	}
	return NULL;
}

/*
 * eip_flow_get_next
 * 	Obtain the next flow
 */
static struct eip_flow *eip_flow_get_next(struct eip_flow *flow)
{
	return hlist_entry(flow->h_node.next, struct eip_flow, h_node);

}

/*
 * eip_flow_get_prev
 * 	Obtain the previous flow
 */
static struct eip_flow *eip_flow_get_prev(struct eip_flow *flow)
{
	return hlist_entry(flow->h_node.pprev, struct eip_flow, h_node.next);
}

/*
 * eip_flow_get_hash()
 * 	Calculate hash for flow tuple
 */
static eip_flow_hash_t eip_flow_get_hash(struct eip_flow_tuple *flow)
{
	struct eip_pdev *ep = platform_get_drvdata(eip_drv_g.pdev);
	uint32_t data[EIP_FLOW_HASH_DATA_SZ];
	eip_flow_hash_t hash;
	void __iomem *base_addr = ep->dev_vaddr;
	int i;

	memset(data, 0, sizeof(data));

	data[0] = 0;
	data[1] = flow->ip_proto << 8;

	if (flow->ip_ver == 6) {
		data[1] |= (1 << 25);
	}

	/*
	 * TODO: Write the port field for inner flow.
	 */
	data[2] = ntohl(flow->spi);
	data[3] = flow->epoch;
	data[4] = (flow->dst_port << 16);
	data[4] |= flow->src_port;

	if (flow->ip_ver == IPVERSION) {
		data[5] = flow->dst_ip[0];
		data[6] = data[7] = data[8] = 0;
		data[9] = flow->src_ip[0];
		data[10] = data[11] = data[12] = 0;
	} else if (flow->ip_ver == 6) {
		data[5] = flow->dst_ip[0];
		data[6] = flow->dst_ip[1];
		data[7] = flow->dst_ip[2];
		data[8] = flow->dst_ip[3];
		data[9] = flow->src_ip[0];
		data[10] = flow->src_ip[1];
		data[11] = flow->src_ip[2];
		data[12] = flow->src_ip[3];
	}

	hash.words[0] = ioread32(base_addr + EIP_HW_FHASH_IV0);
	hash.words[1] = ioread32(base_addr + EIP_HW_FHASH_IV1);
	hash.words[2] = ioread32(base_addr + EIP_HW_FHASH_IV2);
	hash.words[3] = ioread32(base_addr + EIP_HW_FHASH_IV3);

	/*
	 * Algorithm referred from Security-IP-197_HW3.3_Programmer-Manual_RevC
	 */
	for (i = 0; i < EIP_FLOW_HASH_DATA_SZ; i++) {
		uint32_t *hash_words = hash.words;

		hash_words[0] += data[i];
		hash_words[0] += (hash_words[0] << 10);
		hash_words[0] ^= (hash_words[0] >> 6);
		hash_words[1+(i % 3)] ^= data[i];

		if (((i % 3)==2) && (i<(EIP_FLOW_HASH_DATA_SZ - 1))) {
			hash_words[1] -= hash_words[2]; hash_words[1] -= hash_words[3]; hash_words[1] ^= (hash_words[3] >> 13);
			hash_words[2] -= hash_words[3]; hash_words[2] -= hash_words[1]; hash_words[2] ^= (hash_words[1] << 8);
			hash_words[3] -= hash_words[1]; hash_words[3] -= hash_words[2]; hash_words[3] ^= (hash_words[2] >> 13);
			hash_words[1] -= hash_words[2]; hash_words[1] -= hash_words[3]; hash_words[1] ^= (hash_words[3] >> 12);
			hash_words[2] -= hash_words[3]; hash_words[2] -= hash_words[1]; hash_words[2] ^= (hash_words[1] << 16);
			hash_words[3] -= hash_words[1]; hash_words[3] -= hash_words[2]; hash_words[3] ^= (hash_words[2] >> 5);
			hash_words[1] -= hash_words[2]; hash_words[1] -= hash_words[3]; hash_words[1] ^= (hash_words[3] >> 3);
			hash_words[2] -= hash_words[3]; hash_words[2] -= hash_words[1]; hash_words[2] ^= (hash_words[1] << 10);
			hash_words[3] -= hash_words[1]; hash_words[3] -= hash_words[2]; hash_words[3] ^= (hash_words[2] >> 15);
		}
	}
	return hash;
}

/*
 * _eip_flow_lookup()
 *      Lookup the Flow in table using the provided hash.
 */
static struct eip_flow *_eip_flow_lookup(struct eip_flow_tbl *flow_table, eip_flow_hash_t hash)
{
	struct eip_pdev *ep = platform_get_drvdata(eip_drv_g.pdev);
	struct eip_drv *drv = &eip_drv_g;
	struct hlist_head *flow_head;
	struct hlist_node *entry;
	struct eip_flow *flow;
	uint32_t idx;

	assert_spin_locked(&ep->lock);

	idx = EIP_FLOW_HASH_IDX(hash, drv->flow_max - 1);
	flow_head = &flow_table->sw_head[idx];

	if (hlist_empty(flow_head)) {
		return NULL;
	}

	hlist_for_each(entry, flow_head) {
		flow = hlist_entry (entry, struct eip_flow, h_node);
		if (EIP_HASH_EQUAL(flow->hash,hash))
			return flow;
	}

	return NULL;
}

/*
 * eip_flow_alloc
 * 	Allocate flow
 */
static struct eip_flow *eip_flow_alloc(struct eip_tun *tun, struct eip_flow_tuple *flow_tuple, struct eip_tr *tr)
{
	struct eip_pdev *ep = platform_get_drvdata(eip_drv_g.pdev);
	struct hlist_head *flow_head = NULL;
	struct eip_drv *drv = &eip_drv_g;
	struct eip_flow_hw *hflow = NULL;
	struct eip_flow_tbl *tbl = NULL;
	struct eip_flow *flow = NULL;
	eip_flow_hash_t hash;
	uint32_t idx;
	int i;

	hash = eip_flow_get_hash(flow_tuple);
	idx = EIP_FLOW_HASH_IDX(hash, drv->flow_max - 1);
	tbl = &ep->flow_table;
	flow_head = &tbl->sw_head[idx];

	assert_spin_locked(&ep->lock);
	if (_eip_flow_lookup(tbl, hash) != NULL) {
		pr_warn("%px: Flow already exists \n", tr);
		return NULL;
	}

	/*
	 * Allocate the software flow.
	 */
	flow = kmem_cache_alloc(ep->flow_swcache, GFP_ATOMIC);
	if (!flow) {
		return NULL;
	}

	memset(flow, 0, sizeof(struct eip_flow));
	INIT_HLIST_NODE(&flow->h_node);
	INIT_LIST_HEAD(&flow->t_node);
	flow->hash = hash;
	flow->tuple = *flow_tuple;
	flow->tr = eip_tr_ref(tr);

	/*
	 * Check if hardware flow is available in the hash table
	 */
	if(hlist_empty(flow_head)) {
		hflow = tbl->hw_head + idx;
		memset(hflow, 0, sizeof(struct eip_flow_hw));
		hflow->tr_addr_type_1 = tr->tr_addr_type;
		hflow->next_flow_offset = 0;
		hflow->hashid_1 = hash;

		/*
		 * Update SW flow object
		 */
		flow->hflow = hflow ;
		flow->hflow_paddr = tbl->hw_head_paddr + (idx * sizeof(*hflow));
		flow->sentinel = true;
		pr_debug("%px: Hardware flow allocated %pad\n", tr, &flow->hflow_paddr);

		return flow;
	}

	/*
	 * Look up the available hardware flow memory from the collision pool.
	 */
	for (i = 0; i < drv->collision_max; i++) {
		hflow = ep->flow_hwcache + i;
		if (hflow->tr_addr_type_1 == 0) {
			hflow->tr_addr_type_1 = tr->tr_addr_type;
			hflow->next_flow_offset = 0;
			hflow->hashid_1 = hash;

			flow->hflow = hflow;
			flow->hflow_paddr = ep->hwcache_paddr + (i * sizeof(*hflow));
			pr_debug("%px: Hardware flow allocated from collision memory %pad\n", tr, &flow->hflow_paddr);

			return flow;
		}
	}

	/*
	 * Failure condition that occurs when the collision list reaches
	 * its maximum capacity.
	 */
	tbl->stats.fail_collision++;
	eip_tr_deref(flow->tr);
	kmem_cache_free(ep->flow_swcache, flow);
	pr_err("%px: Failed to allocate flow from collision\n", tun);
	return NULL;
}

/*
 * eip_flow_free()
 *      Reset and free the flow
 */
static void eip_flow_free(struct eip_flow *flow, struct eip_pdev *ep)
{
	hlist_del_init(&flow->h_node);

	flow->hflow->tr_addr_type_1 = EIP_FLOW_TR_DISABLE;
	eip_tr_deref(flow->tr);
	memset(flow->hflow, 0, sizeof(*(flow->hflow)));
	memset(flow, 0, sizeof(struct eip_flow));
	kmem_cache_free(ep->flow_swcache, flow);
}

/*
 * eip_flow_swap_hflow()
 *      copy and swap hflow memory for next_flow->hflow to flow->hflow.
 */
static void eip_flow_swap_hflow(struct eip_flow *flow, struct eip_flow *next_flow)
{
	flow->hflow->tr_addr_type_1 = EIP_FLOW_TR_DISABLE;
	flow->hflow->hashid_1 = next_flow->hflow->hashid_1;
	flow->hflow->tr_addr_type_1 = next_flow->hflow->tr_addr_type_1;
	flow->hflow->next_flow_offset = next_flow->hflow->next_flow_offset;

	flow->hflow = xchg(&next_flow->hflow, flow->hflow);
	flow->hflow_paddr = xchg(&next_flow->hflow_paddr, flow->hflow_paddr);
	next_flow->sentinel = flow->sentinel;
	flow->sentinel =  false;
}

/*
 * eip_flow_add()
 */
int eip_flow_add(struct eip_tun *tun, struct eip_flow_info *info)
{
	struct eip_pdev *ep = platform_get_drvdata(eip_drv_g.pdev);
	struct eip_flow_tuple *flow_tuple = &info->tuple;
	struct eip_flow *flow, *tail_flow;
	struct eip_drv *drv = &eip_drv_g;
	struct list_head *tun_flow_head;
	struct hlist_head *flow_head;
	struct eip_tr *tr = info->tr;
	struct eip_flow_tbl *tbl;
	eip_features_t feature;
	uint32_t idx;


	feature = !!(tr->tr_flags & EIP_TR_IPSEC_FLAG_ENC) ? EIP_OFFLOAD_INNER_FLOW : EIP_OFFLOAD_OUTER_FLOW;

	if (!eip_feature_check(tun->ctx, feature)) {
		pr_err("%px: Feature (%x) not supported \n", tun, feature);
		return -ENOTSUPP;
	}

	tbl = &ep->flow_table;
	tun_flow_head = &tun->flows;

	spin_lock_bh(&ep->lock);
	flow = eip_flow_alloc(tun, flow_tuple, tr);
	if (!flow) {
		spin_unlock_bh(&ep->lock);
		pr_err("%px: Failed to allocate EIP flow\n", tr);
		return -ENOMEM;
	}

	idx = EIP_FLOW_HASH_IDX(flow->hash, drv->flow_max - 1);
	flow_head = &tbl->sw_head[idx];

	/*
	 * Fast addition: Addition of a new node at the begining of hlist
	 */
	if (flow->sentinel) {
		hlist_add_head(&flow->h_node, flow_head);
		tbl->stats.alloc++;

		/*
		 * Add sentinel flow to tun flow list
		 */
		list_add_tail(&flow->t_node, tun_flow_head);

		spin_unlock_bh(&ep->lock);
		if (flow_tuple->ip_ver == 6) {
			pr_debug("%px Flow (src:%pI6n dst:%pI6n spi:0x%X) added at the head at index %u\n", flow, flow_tuple->src_ip, flow_tuple->dst_ip, ntohl(flow_tuple->spi), idx);
		} else {
			pr_debug("%px Flow (src:%pI4n dst:%pI4n spi:0x%X) added at the head at index %u\n", flow, flow_tuple->src_ip, flow_tuple->dst_ip, ntohl(flow_tuple->spi), idx);
		}

		return 0;
	}

	/*
	 * Slow addition : Allocate a new hardware flow at tail.
	 * Obtain the tail of the current chain. Here, atleast one node will be already present,hence tail can't be null.
	 */
	tail_flow = eip_flow_get_tail(flow_head);
	BUG_ON(tail_flow == NULL);

	pr_debug("%px: Physical address of hardware head: %pad\n", tr, &tbl->hw_head_paddr);
	tail_flow->hflow->next_flow_offset = (flow->hflow_paddr - tbl->hw_head_paddr) | EIP_FLOW_VALID_BIT;

	hlist_add_behind(&flow->h_node, &tail_flow->h_node);
	tbl->stats.alloc++;
	tbl->stats.collision++;
	tbl->stats.active_collision++;

	/*
	 * Add in to tun flow list;
	 */
	list_add_tail(&flow->t_node, tun_flow_head);

	spin_unlock_bh(&ep->lock);
	if (flow_tuple->ip_ver == 6) {
		pr_debug("%px Flow (src:%pI6n dst:%pI6n spi:0x%X) added after collision at index %u\n", flow, flow_tuple->src_ip, flow_tuple->dst_ip, ntohl(flow_tuple->spi), idx);
	} else {
		pr_debug("%px Flow (src:%pI4n dst:%pI4n spi:0x%X) added after collision at index %u\n", flow, flow_tuple->src_ip, flow_tuple->dst_ip, ntohl(flow_tuple->spi), idx);
	}

	return 0;
}
EXPORT_SYMBOL(eip_flow_add);

/*
 * eip_flow_update_by_tr
 *	Update flows TR mapping.
 */
int eip_flow_update_by_tr(struct eip_tun *tun, struct eip_tr *tr, struct eip_tr *new_tr)
{
	struct eip_pdev *ep = platform_get_drvdata(eip_drv_g.pdev);
	struct list_head *head = &tun->flows;
	struct eip_flow *flow;

	spin_lock_bh(&ep->lock);

	list_for_each_entry(flow, head, t_node) {
		/* Compare and update only matching TR */
		if (flow->tr == tr) {
			flow->tr =  eip_tr_ref(new_tr);
			WARN_ON(!flow->tr);

			flow->hflow->tr_addr_type_1 = new_tr->tr_addr_type;
			eip_tr_deref(tr);
		}
	}
	spin_unlock_bh(&ep->lock);
	return 0;
}

/*
 * eip_flow_del()
 * 	Deletion of flow from the hardware table
 */
void eip_flow_del(struct eip_tun *tun, struct eip_flow_info *info)
{
	struct eip_pdev *ep = platform_get_drvdata(eip_drv_g.pdev);
	struct eip_flow_tuple *flow_tuple = &info->tuple;
	struct eip_flow *flow, *prev_flow;
	struct eip_drv *drv = &eip_drv_g;
	struct hlist_head *flow_head;
	struct eip_flow_tbl *tbl;
	eip_flow_hash_t hash;
	uint32_t idx;

	hash = eip_flow_get_hash(flow_tuple);
	idx = EIP_FLOW_HASH_IDX(hash, drv->flow_max - 1);
	tbl = &ep->flow_table;
	flow_head = &tbl->sw_head[idx];

	spin_lock_bh(&ep->lock);
	flow = _eip_flow_lookup(tbl, hash);
	if (flow == NULL) {
		spin_unlock_bh(&ep->lock);
		pr_err("%px: Flow doesn't exist \n", tun);
		return;
	}

	/*
	 * Remove from node from flow list
	 */
	list_del(&flow->t_node);

	/*
	 * This the only flow in the chain, Reset & free the flow.
	 */
	if (hlist_is_singular_node(&flow->h_node, flow_head)) {
		ASSERT(flow->sentinel);
		pr_debug("%px Flow (src:%pI4n dst:%pI4n spi:0x%X) deleted at head at index %u\n",
				flow, flow_tuple->src_ip, flow_tuple->dst_ip, ntohl(flow_tuple->spi), idx);
		goto free;
	}

	/*
	 * There are more flows in the chain, But we are deleting the head flow.
	 * Move the second hardware flow to the head. Then, reset & free the flow.
	 */
	if (flow->sentinel) {
		struct eip_flow *next_flow;

		next_flow = eip_flow_get_next(flow);
		eip_flow_swap_hflow(flow, next_flow);
		pr_debug("%px Flow (src:%pI4n dst:%pI4n spi:0x%X) deleted at head (with collision) at index %u\n",
				flow, flow_tuple->src_ip, flow_tuple->dst_ip, ntohl(flow_tuple->spi), idx);
		tbl->stats.active_collision--;
		goto free;
	}

	/*
	 * We are deleting the non-sentinel (not at the Head) flow.
	 * Make previous hflow to point the next flow (or NULL). Then reset and free the flow.
	 */
	prev_flow = eip_flow_get_prev(flow);
	prev_flow->hflow->next_flow_offset = flow->hflow->next_flow_offset;
	tbl->stats.active_collision--;
	pr_debug("%px Flow (src:%pI4n dst:%pI4n spi:0x%X) deleted (after collision) at index %u\n",
			flow, flow_tuple->src_ip, flow_tuple->dst_ip, ntohl(flow_tuple->spi), idx);

free:
	tbl->stats.free++;
	eip_flow_free(flow, ep);
	spin_unlock_bh(&ep->lock);
}
EXPORT_SYMBOL(eip_flow_del);

/*
 * eip_flow_table_init()
 * 	Allocate flow entries for hardware table
 */
bool eip_flow_table_init(struct platform_device *pdev)
{
	struct eip_pdev *ep = platform_get_drvdata(pdev);
	void __iomem *base_addr = ep->dev_vaddr;
	struct eip_drv *drv = &eip_drv_g;
	size_t tbl_sz, collision_sz;
	struct eip_flow_tbl *tbl;
	u32 hw_tbl_sz = 0;
	u32 tbl_cnt = 0;
	dma_addr_t paddr;
	void *addr;
	int i;

	/*
	 * Derive HW table size by converting the maximum supported flows
	 * (normalized by minimum bucket size) into a log2 exponent.
	 */
	tbl_cnt = drv->flow_max/EIP_FLOW_HW_MIN;
	hw_tbl_sz = ilog2(tbl_cnt);

	ep->flow_swcache = kmem_cache_create("eip_sw_flow", sizeof(struct eip_flow), 0, 0, NULL);
	if (!ep->flow_swcache) {
		pr_err("%px: Failed to allocate flow swcache\n",pdev);
		return false;
	}

	tbl = &ep->flow_table;
	tbl->sw_head = kcalloc(drv->flow_max, sizeof(struct hlist_head), GFP_KERNEL);
	if (!tbl->sw_head) {
		pr_err("%px: Failed to allocate sw flow head\n", pdev);
		goto fail_sw;
	}

	tbl_sz = drv->flow_max * sizeof(struct eip_flow_hw);
	collision_sz = drv->collision_max * sizeof(struct eip_flow_hw);
	tbl->pool_sz = tbl_sz + collision_sz;

	/*
	 * Allocate contiguous memory for hash table and collision list
	 */
	addr = dma_alloc_coherent(&pdev->dev, tbl->pool_sz, &paddr, GFP_DMA);
	if (!addr) {
		pr_err("%px: Failed to allocate entries\n",pdev);
		goto fail_dma;
	}

	memset(addr, 0, (tbl_sz + collision_sz));
	tbl->hw_head = addr;
	tbl->hw_head_paddr = paddr;
	ep->flow_hwcache = addr + tbl_sz;
	ep->hwcache_paddr = paddr + tbl_sz;

	pr_debug("%px: DMA Address %px\n", pdev, tbl->hw_head);
	pr_debug("%px: DMA Physical address %pad\n", pdev, &tbl->hw_head_paddr);

	iowrite32(EIP_HW_FLUE_CONFIG_VAL(hw_tbl_sz), base_addr + EIP_HW_FLUE_CONFIG(0));
	iowrite32(0x0, base_addr + EIP_HW_FLUE_CACHEBASE_LO(0));
	iowrite32(0x0, base_addr + EIP_HW_FLUE_CACHEBASE_HI(0));
	iowrite32(tbl->hw_head_paddr, base_addr + EIP_HW_FLUE_HASHBASE_LO(0));
	iowrite32(0x0, base_addr + EIP_HW_FLUE_HASHBASE_HI(0));

	for (i = 0; i < drv->flow_max; i++){
		INIT_HLIST_HEAD(&tbl->sw_head[i]);
	}

	if (eip_flow_dump_init(tbl, ep)) {
		pr_err("%px: Failed to create flow dump debug fs\n",pdev);
	}

	return true;

fail_dma:
	kfree(tbl->sw_head);
	tbl->sw_head = NULL;
fail_sw:
	kmem_cache_destroy(ep->flow_swcache);
	return false;
}

/*
 * eip_flow_table_deinit()
 * 	Free the flow entries
 */
void eip_flow_table_deinit(struct platform_device *pdev)
{
	struct eip_pdev *ep = platform_get_drvdata(pdev);
	void __iomem *base_addr = ep->dev_vaddr;
	struct eip_flow_tbl *tbl;

	tbl = &ep->flow_table;
	eip_flow_dump_deinit(tbl);
	iowrite32(0x0, base_addr + EIP_HW_FLUE_HASHBASE_LO(0));
	kmem_cache_destroy(ep->flow_swcache);
	dma_free_coherent(&pdev->dev, tbl->pool_sz, tbl->hw_head, tbl->hw_head_paddr);
	kfree(tbl->sw_head);
	tbl->sw_head = NULL;
}
