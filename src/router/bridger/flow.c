// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#include <inttypes.h>
#include <stdlib.h>
#include <linux/neighbour.h>
#include "bridger.h"

static struct uloop_timeout flow_update_timer;

static int flow_key_cmp(const void *k1, const void *k2, void *ptr)
{
	return memcmp(k1, k2, sizeof(struct bridger_flow_key));
}

static int flow_sort_key_cmp(const void *k1, const void *k2, void *ptr)
{
	return memcmp(k1, k2, sizeof(uint64_t));
}

static AVL_TREE(flows, flow_key_cmp, false, NULL);
static AVL_TREE(sorted_flows, flow_sort_key_cmp, true, NULL);

static void
__bridger_flow_delete(struct bridger_flow *flow)
{
	list_del(&flow->fdb_in_list);
	list_del(&flow->fdb_out_list);
	avl_delete(&sorted_flows, &flow->sort_node);
}

static void
flow_debug_msg(struct bridger_flow *flow, const char *type)
{
	char src[20], dest[20];
	D("%s flow %s@%s -> %s@%s vlan=%d cur_packets=%"PRIu64" avg_packets=%"PRIu64" idle=%d\n", type,
	  strcpy(src, format_macaddr(flow->key.src)),
	  flow->fdb_in->dev->ifname,
	  strcpy(dest, format_macaddr(flow->key.dest)),
	  flow->fdb_out->dev->ifname,
	  flow->key.vlan & BRIDGER_VLAN_ID,
	  flow->cur_packets,
	  flow->avg_packets >> BRIDGER_EWMA_SHIFT,
	  flow->idle);
}

void bridger_flow_delete(struct bridger_flow *flow)
{
	flow_debug_msg(flow, "Delete");
	__bridger_flow_delete(flow);
	avl_delete(&flows, &flow->node);
	bridger_bpf_flow_delete(flow);
	bridger_nl_flow_offload_del(flow);
}

void bridger_check_pending_flow(struct bridger_flow_key *key,
				struct bridger_pending_flow *val)
{
	struct fdb_entry *fdb_in, *fdb_out;
	struct bridger_flow *flow;
	struct bridge *br;
	char src[20], dest[20];
	struct device *dev;
	struct fdb_key fkey = {};
	int out_vlan;

	if (!memcmp(key->src, key->dest, ETH_ALEN))
		return;

	dev = device_get(key->ifindex);
	if (!dev)
		return;

	br = device_get_br(dev);
	if (!br)
		return;

	memcpy(fkey.addr, key->src, ETH_ALEN);
	fkey.vlan = device_vlan_get_input(dev, key->vlan);
	if (fkey.vlan == (uint16_t)-1)
		return;

	fdb_in = fdb_get(br, &fkey);

	if (fdb_in && fdb_in->dev != dev) {
		D("Skip pending flow: received on %s, but fdb entry is on %s\n",
		  dev->ifname, fdb_in->dev->ifname);
		return;
	}

	if (dev->redirect_dev) {
		if (!fdb_in) {
			fdb_in = fdb_create(br, &fkey, dev);
			fdb_in->ndm_state = NUD_REACHABLE;
		}

		bridger_nl_fdb_refresh(fdb_in);
	}

	if (key->dest[0] & 1)
		return;

	memcpy(fkey.addr, key->dest, ETH_ALEN);
	fdb_out = fdb_get(br, &fkey);

	if (!fdb_out && fkey.vlan && br->fdb_local_vlan_0) {
		struct fdb_key fkey_vlan0 = fkey;
		fkey_vlan0.vlan = 0;
		fdb_out = fdb_get(br, &fkey_vlan0);
		if (fdb_out && !fdb_out->is_local)
			fdb_out = NULL;
	}

	D("Pending flow on %s: %s -> %s @%d num_packets=%"PRIu64" -> %s\n",
	  dev->ifname,
	  strcpy(src, format_macaddr(key->src)),
	  strcpy(dest, format_macaddr(key->dest)),
	  key->vlan & BRIDGER_VLAN_ID, val->packets,
	  fdb_out ? fdb_out->dev->ifname : "(unknown)");

	if (!fdb_in || !fdb_out)
		return;

	if (device_match_phys_switch(fdb_in->dev, fdb_out->dev))
		return;

	if (fdb_in->dev == fdb_out->dev &&
	    !fdb_in->dev->hairpin_mode)
		return;

	if (dev->redirect_dev &&
	    dev->redirect_dev != device_ifindex(fdb_out->dev))
		return;

	if (fdb_in->dev->isolated && fdb_out->dev->isolated)
		return;

	if (!device_vlan_state_forwarding(fdb_in->dev, fkey.vlan))
		return;

	if (!device_vlan_state_forwarding(fdb_out->dev, fkey.vlan))
		return;

	if (!fdb_in->dev->port_forwarding || !fdb_out->dev->port_forwarding)
		return;

	if (device_vlan_has_tunnel(fdb_in->dev, fkey.vlan))
		return;

	if (device_vlan_has_tunnel(fdb_out->dev, fkey.vlan))
		return;

	out_vlan = device_vlan_get_output(fdb_out->dev, fkey.vlan);
	if (out_vlan < 0)
		return;

	flow = avl_find_element(&flows, key, flow, node);
	if (!flow) {
		flow = calloc(1, sizeof(*flow));
		flow->node.key = &flow->key;
		flow->sort_node.key = &flow->avg_packets;
		memcpy(&flow->key, key, sizeof(flow->key));
		avl_insert(&flows, &flow->node);
	} else {
		__bridger_flow_delete(flow);
	}

	flow->fdb_in = fdb_in;
	list_add(&flow->fdb_in_list, &fdb_in->flows_in);

	flow->fdb_out = fdb_out;
	list_add(&flow->fdb_out_list, &fdb_out->flows_out);

	flow->offload.target_port = device_ifindex(fdb_out->dev);
	flow->offload.vlan = out_vlan;
	flow->offload.redirect_flags = fdb_out->dev->br ? BPF_F_INGRESS : 0;
	if (!bridge_local_rx && fdb_out->dev->br)
		flow->offload.target_port = 0;

	bridger_bpf_flow_upload(flow);
	if (!fdb_in->dev->br && !fdb_out->dev->br)
		bridger_nl_flow_offload_add(flow);

	avl_insert(&sorted_flows, &flow->sort_node);
}

static void
bridger_flow_update_cb(struct uloop_timeout *timeout)
{
	struct bridger_flow *flow, *tmp;

	device_reset_offload_update();

	avl_for_each_element(&flows, flow, node) {
		flow->fdb_in->updated = false;
		flow->fdb_out->updated = false;
	}

	avl_for_each_element_safe(&flows, flow, node, tmp) {
		avl_delete(&sorted_flows, &flow->sort_node);
		bridger_bpf_flow_update(flow);
		bridger_nl_flow_offload_update(flow);
		avl_insert(&sorted_flows, &flow->sort_node);

		flow_debug_msg(flow, "Update");

		if (flow->cur_packets)
			flow->idle = 0;
		else if (++flow->idle >= 30)
			bridger_flow_delete(flow);

		if (flow->idle)
			continue;

		bridger_nl_fdb_refresh(flow->fdb_in);
	}

	uloop_timeout_set(timeout, 1000);
}

int bridger_flow_init(void)
{
	flow_update_timer.cb = bridger_flow_update_cb;
	bridger_flow_update_cb(&flow_update_timer);

	return 0;
}
