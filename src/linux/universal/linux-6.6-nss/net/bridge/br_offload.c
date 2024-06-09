// SPDX-License-Identifier: GPL-2.0-only
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include "br_private.h"
#include "br_private_offload.h"

static DEFINE_SPINLOCK(offload_lock);

struct bridge_flow_key {
	u8 dest[ETH_ALEN];
	u8 src[ETH_ALEN];
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	u16 vlan_tag;
	bool vlan_present;
#endif
};

struct bridge_flow {
	struct net_bridge_port *port;
	struct rhash_head node;
	struct bridge_flow_key key;
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	bool vlan_out_present;
	u16 vlan_out;
#endif

	unsigned long used;
	struct net_bridge_fdb_entry *fdb_in, *fdb_out;
	struct hlist_node fdb_list_in, fdb_list_out;

	struct rcu_head rcu;
};

static const struct rhashtable_params flow_params = {
	.automatic_shrinking = true,
	.head_offset = offsetof(struct bridge_flow, node),
	.key_len = sizeof(struct bridge_flow_key),
	.key_offset = offsetof(struct bridge_flow, key),
};

static struct kmem_cache *offload_cache __read_mostly;

static void
flow_rcu_free(struct rcu_head *head)
{
	struct bridge_flow *flow;

	flow = container_of(head, struct bridge_flow, rcu);
	kmem_cache_free(offload_cache, flow);
}

static void
__br_offload_flow_free(struct bridge_flow *flow)
{
	flow->used = 0;
	hlist_del(&flow->fdb_list_in);
	hlist_del(&flow->fdb_list_out);

	call_rcu(&flow->rcu, flow_rcu_free);
}

static void
br_offload_flow_free(struct bridge_flow *flow)
{
	if (rhashtable_remove_fast(&flow->port->offload.rht, &flow->node,
				   flow_params) != 0)
		return;

	__br_offload_flow_free(flow);
}

static bool
br_offload_flow_fdb_refresh_time(struct bridge_flow *flow,
				 struct net_bridge_fdb_entry *fdb)
{
	if (!time_after(flow->used, fdb->updated))
		return false;

	fdb->updated = flow->used;

	return true;
}


static void
br_offload_flow_refresh_time(struct bridge_flow *flow)
{
	br_offload_flow_fdb_refresh_time(flow, flow->fdb_in);
	br_offload_flow_fdb_refresh_time(flow, flow->fdb_out);
}

static void
br_offload_destroy_cb(void *ptr, void *arg)
{
	struct bridge_flow *flow = ptr;

	__br_offload_flow_free(flow);
}

static bool
br_offload_need_gc(struct net_bridge_port *p)
{
	return (atomic_read(&p->offload.rht.nelems) +
	        p->br->offload_cache_reserved) >= p->br->offload_cache_size;
}

static void
br_offload_gc_work(struct work_struct *work)
{
	struct rhashtable_iter hti;
	struct net_bridge_port *p;
	struct bridge_flow *gc_flow = NULL;
	struct bridge_flow *flow;
	unsigned long gc_used;

	p = container_of(work, struct net_bridge_port, offload.gc_work);

	if (!br_offload_need_gc(p))
		return;

	rhashtable_walk_enter(&p->offload.rht, &hti);
	rhashtable_walk_start(&hti);
	while ((flow = rhashtable_walk_next(&hti)) != NULL) {
		unsigned long used;

		if (IS_ERR(flow))
			continue;

		used = READ_ONCE(flow->used);
		if (!used)
			continue;

		if (gc_flow && !time_before(used, gc_used))
			continue;

		gc_flow = flow;
		gc_used = used;
	}
	rhashtable_walk_stop(&hti);
	rhashtable_walk_exit(&hti);

	if (!gc_flow)
		return;

	spin_lock_bh(&offload_lock);
	if (br_offload_need_gc(p) && gc_flow &&
	    gc_flow->used == gc_used)
		br_offload_flow_free(gc_flow);
	if (p->offload.enabled && br_offload_need_gc(p))
		queue_work(system_long_wq, work);
	spin_unlock_bh(&offload_lock);

}

void br_offload_port_state(struct net_bridge_port *p)
{
	struct net_bridge_port_offload *o = &p->offload;
	bool enabled = true;
	bool flush = false;

	if (p->state != BR_STATE_FORWARDING ||
	    !(p->flags & BR_OFFLOAD))
		enabled = false;

	spin_lock_bh(&offload_lock);
	if (o->enabled == enabled)
		goto out;

	if (enabled) {
		if (!o->gc_work.func)
			INIT_WORK(&o->gc_work, br_offload_gc_work);
		rhashtable_init(&o->rht, &flow_params);
	} else {
		flush = true;
		rhashtable_free_and_destroy(&o->rht, br_offload_destroy_cb, o);
	}

	o->enabled = enabled;

out:
	spin_unlock_bh(&offload_lock);

	if (flush)
		flush_work(&o->gc_work);
}

void br_offload_fdb_update(const struct net_bridge_fdb_entry *fdb)
{
	struct bridge_flow *f;
	struct hlist_node *tmp;

	spin_lock_bh(&offload_lock);

	hlist_for_each_entry_safe(f, tmp, &fdb->offload_in, fdb_list_in)
		br_offload_flow_free(f);

	hlist_for_each_entry_safe(f, tmp, &fdb->offload_out, fdb_list_out)
		br_offload_flow_free(f);

	spin_unlock_bh(&offload_lock);
}

static void
br_offload_prepare_key(struct net_bridge_port *p, struct bridge_flow_key *key,
		       struct sk_buff *skb)
{
	memset(key, 0, sizeof(*key));
	memcpy(key, eth_hdr(skb), 2 * ETH_ALEN);
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	if (!br_opt_get(p->br, BROPT_VLAN_ENABLED))
		return;

	if (!skb_vlan_tag_present(skb) || skb->vlan_proto != p->br->vlan_proto)
		return;

	key->vlan_present = true;
	key->vlan_tag = skb_vlan_tag_get_id(skb);
#endif
}

void br_offload_output(struct sk_buff *skb)
{
	struct net_bridge_port_offload *o;
	struct br_input_skb_cb *cb = (struct br_input_skb_cb *)skb->cb;
	struct net_bridge_port *p, *inp;
	struct net_device *dev;
	struct net_bridge_fdb_entry *fdb_in, *fdb_out;
	struct net_bridge_vlan_group *vg;
	struct bridge_flow_key key;
	struct bridge_flow *flow;
	u16 vlan;

	if (!cb->offload)
		return;

	rcu_read_lock();

	p = br_port_get_rcu(skb->dev);
	if (!p)
		goto out;

	o = &p->offload;
	if (!o->enabled)
		goto out;

	if (atomic_read(&p->offload.rht.nelems) >= p->br->offload_cache_size)
		goto out;

	dev = dev_get_by_index_rcu(dev_net(p->br->dev), cb->input_ifindex);
	if (!dev)
		goto out;

	inp = br_port_get_rcu(dev);
	if (!inp)
		goto out;

	vg = nbp_vlan_group_rcu(inp);
	vlan = cb->input_vlan_present ? cb->input_vlan_tag : br_get_pvid(vg);
	fdb_in = br_fdb_find_rcu(p->br, eth_hdr(skb)->h_source, vlan);
	if (!fdb_in || !fdb_in->dst)
		goto out;

	vg = nbp_vlan_group_rcu(p);
	vlan = skb_vlan_tag_present(skb) ? skb_vlan_tag_get_id(skb) : br_get_pvid(vg);
	fdb_out = br_fdb_find_rcu(p->br, eth_hdr(skb)->h_dest, vlan);
	if (!fdb_out || !fdb_out->dst)
		goto out;

	br_offload_prepare_key(p, &key, skb);
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	key.vlan_present = cb->input_vlan_present;
	key.vlan_tag = cb->input_vlan_tag;
#endif

	flow = kmem_cache_alloc(offload_cache, GFP_ATOMIC);
	flow->port = inp;
	memcpy(&flow->key, &key, sizeof(key));

#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	flow->vlan_out_present = skb_vlan_tag_present(skb);
	flow->vlan_out = skb_vlan_tag_get(skb);
#endif

	flow->fdb_in = fdb_in;
	flow->fdb_out = fdb_out;
	flow->used = jiffies;

	spin_lock_bh(&offload_lock);
	if (!o->enabled ||
	    atomic_read(&p->offload.rht.nelems) >= p->br->offload_cache_size ||
	    rhashtable_insert_fast(&inp->offload.rht, &flow->node, flow_params)) {
		kmem_cache_free(offload_cache, flow);
		goto out_unlock;
	}

	hlist_add_head(&flow->fdb_list_in, &fdb_in->offload_in);
	hlist_add_head(&flow->fdb_list_out, &fdb_out->offload_out);

	if (br_offload_need_gc(p))
		queue_work(system_long_wq, &p->offload.gc_work);

out_unlock:
	spin_unlock_bh(&offload_lock);

out:
	rcu_read_unlock();
}

bool br_offload_input(struct net_bridge_port *p, struct sk_buff *skb)
{
	struct net_bridge_port_offload *o = &p->offload;
	struct br_input_skb_cb *cb = (struct br_input_skb_cb *)skb->cb;
	struct bridge_flow_key key;
	struct net_bridge_port *dst;
	struct bridge_flow *flow;
	unsigned long now = jiffies;
	bool ret = false;

	if (skb->len < sizeof(key))
		return false;

	if (!o->enabled)
		return false;

	if (is_multicast_ether_addr(eth_hdr(skb)->h_dest))
		return false;

	br_offload_prepare_key(p, &key, skb);

	rcu_read_lock();
	flow = rhashtable_lookup(&o->rht, &key, flow_params);
	if (!flow) {
		cb->offload = 1;
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
		cb->input_vlan_present = key.vlan_present != 0;
		cb->input_vlan_tag = key.vlan_tag;
#endif
		cb->input_ifindex = p->dev->ifindex;
		goto out;
	}

	if (flow->fdb_in->dst != p)
		goto out;

	dst = flow->fdb_out->dst;
	if (!dst)
		goto out;

	ret = true;
#ifdef CONFIG_BRIDGE_VLAN_FILTERING
	if (!flow->vlan_out_present && key.vlan_present) {
		__vlan_hwaccel_clear_tag(skb);
	} else if (flow->vlan_out_present) {
		if (skb_vlan_tag_present(skb) &&
		    skb->vlan_proto != p->br->vlan_proto) {
			/* Protocol-mismatch, empty out vlan_tci for new tag */
			skb_push(skb, ETH_HLEN);
			skb = vlan_insert_tag_set_proto(skb, skb->vlan_proto,
							skb_vlan_tag_get(skb));
			if (unlikely(!skb))
				goto out;

			skb_pull(skb, ETH_HLEN);
			skb_reset_mac_len(skb);
		}

		__vlan_hwaccel_put_tag(skb, p->br->vlan_proto,
				       flow->vlan_out);
	}
#endif

	skb->dev = dst->dev;
	skb_push(skb, ETH_HLEN);

	if (skb_warn_if_lro(skb) || !is_skb_forwardable(skb->dev, skb)) {
		kfree_skb(skb);
		goto out;
	}

	if (now - flow->used >= HZ) {
		flow->used = now;
		br_offload_flow_refresh_time(flow);
	}

	skb_forward_csum(skb);
	dev_queue_xmit(skb);

out:
	rcu_read_unlock();
	return ret;
}

static void
br_offload_check_gc(struct net_bridge *br)
{
	struct net_bridge_port *p;

	spin_lock_bh(&br->lock);
	list_for_each_entry(p, &br->port_list, list)
		if (br_offload_need_gc(p))
			queue_work(system_long_wq, &p->offload.gc_work);
	spin_unlock_bh(&br->lock);
}

int br_offload_set_cache_size(struct net_bridge *br, unsigned long val,struct netlink_ext_ack *extack)
{
	br->offload_cache_size = val;
	br_offload_check_gc(br);

	return 0;
}

int br_offload_set_cache_reserved(struct net_bridge *br, unsigned long val,struct netlink_ext_ack *extack)
{
	br->offload_cache_reserved = val;
	br_offload_check_gc(br);

	return 0;
}

int __init br_offload_init(void)
{
	offload_cache = kmem_cache_create("bridge_offload_cache",
					  sizeof(struct bridge_flow),
					  0, SLAB_HWCACHE_ALIGN, NULL);
	if (!offload_cache)
		return -ENOMEM;

	return 0;
}

void br_offload_fini(void)
{
	kmem_cache_destroy(offload_cache);
}
