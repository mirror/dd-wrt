// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#ifndef __BRIDGER_XDP_H
#define __BRIDGER_XDP_H

#define BRIDGER_VLAN_PRESENT	(1 << 15)
#define BRIDGER_VLAN_TYPE_AD	(1 << 14)

#define BRIDGER_VLAN_FLAGS	(BRIDGER_VLAN_PRESENT | BRIDGER_VLAN_TYPE_AD)

#define BRIDGER_VLAN_ID		((1 << 12) - 1)

#define BRIDGER_PENDING_FLOWS	32
#define BRIDGER_OFFLOAD_FLOWS	256
#define BRIDGER_DEVMAP_SIZE	64

struct bridger_flow_key {
	uint8_t dest[6];
	uint8_t src[6];
	uint16_t vlan;
	uint32_t ifindex;
};

struct bridger_pending_flow {
	uint64_t packets;
};

struct bridger_offload_flow {
	uint64_t packets;
	uint32_t target_port;
	uint16_t vlan;
	uint16_t redirect_flags;
};

struct bridger_policy_flow {
	struct bridger_offload_flow flow;
	uint8_t bridge_mac[6];
};

#endif
