// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)

#ifndef OFFLOAD_H
#define OFFLOAD_H

#include <linux/cfm_bridge.h>

struct mac_addr {
	unsigned char addr[6];
};

struct maid_data {
	unsigned char data[CFM_MAID_LENGTH];
};

int cfm_offload_mep_create(uint32_t br_ifindex, uint32_t instance, uint32_t domain, uint32_t direction,
			   uint32_t ifindex);
int cfm_offload_mep_delete(uint32_t br_ifindex, uint32_t instance);
int cfm_offload_mep_config(uint32_t br_ifindex, uint32_t instance, struct mac_addr *mac, uint32_t level,
			   uint32_t mepid);
int cfm_offload_cc_config(uint32_t br_ifindex, uint32_t instance, uint32_t enable,
			  uint32_t interval, struct maid_data *maid);
int cfm_offload_cc_rdi(uint32_t br_ifindex, uint32_t instance, uint32_t rdi);
int cfm_offload_cc_peer(uint32_t br_ifindex, uint32_t instance, uint32_t remove, uint32_t mepid);
int cfm_offload_cc_ccm_tx(uint32_t br_ifindex, uint32_t instance,
			  struct mac_addr *dmac, uint32_t sequence, uint32_t period, uint32_t iftlv,
			  uint8_t iftlv_value, uint32_t porttlv, uint8_t porttlv_value);

int cfm_offload_init(void);
int cfm_offload_mep_config_show(uint32_t br_ifindex);
int cfm_offload_mep_status_show(uint32_t br_ifindex);

#endif
