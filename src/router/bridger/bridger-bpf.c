// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#define KBUILD_MODNAME "bridger"
#include <uapi/linux/bpf.h>
#include <uapi/linux/pkt_cls.h>
#include <uapi/linux/if_ether.h>
#include <uapi/linux/ipv6.h>
#include <uapi/linux/in6.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include "bridger-bpf.h"

#ifndef ETH_P_CFM
#define ETH_P_CFM	0x8902
#endif

struct {
	__uint(type, BPF_MAP_TYPE_LRU_HASH);
	__uint(pinning, 1);
	__type(key, struct bridger_flow_key);
	__type(value, struct bridger_pending_flow);
	__uint(max_entries, BRIDGER_PENDING_FLOWS);
} pending_flows SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(pinning, 1);
	__type(key, struct bridger_flow_key);
	__type(value, struct bridger_offload_flow);
	__uint(max_entries, BRIDGER_OFFLOAD_FLOWS);
	__uint(map_flags, BPF_F_NO_PREALLOC);
} offload_flows SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(pinning, 1);
	__type(key, unsigned int);
	__type(value, struct bridger_policy_flow);
	__uint(max_entries, BRIDGER_DEVMAP_SIZE);
	__uint(map_flags, BPF_F_NO_PREALLOC);
} dev_policy SEC(".maps");

struct vlanhdr {
	__be16 tci;
	__be16 encap_proto;
};

#define memcpy __builtin_memcpy

static __always_inline int proto_is_vlan(__u16 h_proto)
{
	return !!(h_proto == bpf_htons(ETH_P_8021Q) ||
		  h_proto == bpf_htons(ETH_P_8021AD));
}

static __always_inline void *__skb_data(struct __sk_buff *skb)
{
	return (void *)(long)skb->data;
}

static int ether_addr_cmp(const void *a1, const void *a2)
{
	return (*(__u32 *)a1 ^ *(__u32 *)a2) | (*(__u16 *)(a1 + 4) ^ *(__u16 *)(a2 + 4));
}

static __always_inline void *
skb_ptr(struct __sk_buff *skb, int offset, int len)
{
	void *ptr = __skb_data(skb) + offset;
	void *end = (void *)(long)(skb->data_end);

	if (ptr + len >= end)
		return NULL;

	return ptr;
}

static __always_inline int
bridger_offload(struct __sk_buff *skb, struct bridger_offload_flow *offload,
		struct bridger_flow_key *key, unsigned long *flags)
{
	struct vlanhdr *vlan;
	struct ethhdr eth;
	void *data;
	u16 diff;

	data = skb_ptr(skb, 0, sizeof(eth) + sizeof(*vlan));
	if (!data)
		return -1;

	diff = key->vlan ^ offload->vlan;
	if (diff & (BRIDGER_VLAN_FLAGS | BRIDGER_VLAN_ID)) {
		if ((key->vlan & BRIDGER_VLAN_PRESENT) &&
			 bpf_skb_vlan_pop(skb))
				return -1;

		if (offload->vlan & BRIDGER_VLAN_PRESENT) {
			__be16 proto;

			if (offload->vlan & BRIDGER_VLAN_TYPE_AD)
				proto = bpf_htons(ETH_P_8021AD);
			else
				proto = bpf_htons(ETH_P_8021Q);

			if (bpf_skb_vlan_push(skb, proto, offload->vlan & BRIDGER_VLAN_ID))
				return -1;
		}
	}

	offload->packets++;
	*flags = offload->redirect_flags;
	return offload->target_port;
}

static __always_inline void
bridger_update_pending_flow(struct __sk_buff *skb, struct bridger_flow_key *key)
{
	struct bridger_pending_flow *pending;
	struct bridger_pending_flow data = {
		.packets = 1
	};

	pending = bpf_map_lookup_elem(&pending_flows, key);
	if (pending) {
		pending->packets++;
		return;
	}

	bpf_map_update_elem(&pending_flows, key, &data, BPF_ANY);
}

static void
bridger_key_set_vlan(struct bridger_flow_key *key, __be16 proto, u16 val)
{
	key->vlan = BRIDGER_VLAN_PRESENT;
	key->vlan |= val & BRIDGER_VLAN_ID;
	if (proto == bpf_htons(ETH_P_8021AD))
		key->vlan |= BRIDGER_VLAN_TYPE_AD;
}

SEC("tc")
int bridger_output(struct __sk_buff *skb)
{
	struct bridger_policy_flow *offload;
	struct ethhdr *eth;
	u32 ifindex = skb->ifindex;

	eth = skb_ptr(skb, 0, sizeof(*eth) + sizeof(struct vlanhdr));
	if (!eth)
		return TC_ACT_UNSPEC;

	offload = bpf_map_lookup_elem(&dev_policy, &ifindex);
	if (!offload)
		return TC_ACT_UNSPEC;

	if (skb->ingress_ifindex == offload->flow.target_port)
		return TC_ACT_UNSPEC;

	if (!ether_addr_cmp(eth->h_source, offload->bridge_mac))
		return TC_ACT_UNSPEC;

	return TC_ACT_SHOT;
}

SEC("tc")
int bridger_input(struct __sk_buff *skb)
{
	struct bridger_offload_flow *offload, odata = {};
	struct bridger_pending_flow pending;
	struct bridger_flow_key key = {};
	bool update_pending = false;
	unsigned long flags = 0;
	struct ethhdr *eth;
	__be16 proto;
	bool vlan_hdr = false;
	int ret = -1;

	bpf_skb_pull_data(skb, sizeof(*eth) + sizeof(struct vlanhdr));
	eth = skb_ptr(skb, 0, sizeof(*eth) + sizeof(struct vlanhdr));
	if (!eth)
		return TC_ACT_UNSPEC;

	memcpy(&key, eth, 2 * ETH_ALEN);
	key.ifindex = skb->ifindex;
	proto = eth->h_proto;
	if (skb->vlan_present) {
		bridger_key_set_vlan(&key, skb->vlan_proto, skb->vlan_tci);
	} else if (proto_is_vlan(proto)) {
		struct vlanhdr *vlan = (void *)(eth + 1);

		bridger_key_set_vlan(&key, eth->h_proto, bpf_ntohs(vlan->tci));
		proto = vlan->encap_proto;
		vlan_hdr = true;
	}

	if (proto == bpf_htons(ETH_P_CFM) || proto == bpf_htons(ETH_P_MRP) ||
	    proto == bpf_htons(ETH_P_ARP) || proto == bpf_htons(ETH_P_802_2))
		return TC_ACT_UNSPEC;

	if ((eth->h_source[0] | eth->h_dest[0]) & 1)
		return TC_ACT_UNSPEC;

	if (proto == bpf_htons(ETH_P_IPV6)) {
		struct ipv6hdr *ip6hdr;
		int ofs;

		ofs = sizeof(*eth);
		if (vlan_hdr)
			ofs += sizeof(struct vlanhdr);

		ip6hdr = skb_ptr(skb, ofs, sizeof(*ip6hdr));
		if (!ip6hdr || ip6hdr->nexthdr == IPPROTO_ICMPV6)
			return TC_ACT_UNSPEC;
	}


	offload = bpf_map_lookup_elem(&offload_flows, &key);
	if (offload) {
		if (!offload->target_port)
			return TC_ACT_UNSPEC;

		ret = bridger_offload(skb, offload, &key, &flags);
		goto out;
	}

dev_lookup:
	offload = bpf_map_lookup_elem(&dev_policy, &key.ifindex);
	if (offload) {
		ret = bridger_offload(skb, offload, &key, &flags);
		update_pending = true;
	}

out:
	if (ret < 0 || update_pending)
		bridger_update_pending_flow(skb, &key);
	if (ret >= 0)
		return bpf_redirect(ret, flags);

	return TC_ACT_UNSPEC;
}

char _license[] SEC("license") = "GPL";
