/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

struct ppe_drv_pppoe;

#include <linux/if_ether.h>

/*
 * ppe_drv_l3_if_type
 *	PPE L3_IF types
 */
enum ppe_drv_l3_if_type {
	PPE_DRV_L3_IF_TYPE_NONE,    /* L3_IF NONE */
	PPE_DRV_L3_IF_TYPE_PORT,    /* L3_IF for a port */
	PPE_DRV_L3_IF_TYPE_PPPOE,   /* L3_IF for pppoe interface */
	PPE_DRV_L3_IF_TYPE_VLAN,	/* L3_IF for VLAN interface */
	PPE_DRV_L3_IF_TYPE_MAX,     /* Max L3_IF types */
};

/*
 * ppe_drv_l3_if
 *	 L3 interface information
 */
struct ppe_drv_l3_if {
	struct list_head list;		/* List of L3_IF instance */
	struct kref ref;		/* Reference count */
	struct ppe_drv_pppoe *pppoe;	/* pppoe interface associated with l3 */
	uint8_t l3_if_index;		/* L3 interface number */
	uint16_t mtu;			/* MTU size of L3 interface */
	enum ppe_drv_l3_if_type type;	/* L3_IF type port or pppoe */

	bool is_mac_set;		/* Mac address already set on L3_IF */

	uint8_t ig_mac_addr[ETH_ALEN];	/* Ingress MAC address of the L3 interface */
	uint16_t ig_mac_ref;		/* Reference on ingress MAC */
	bool is_ig_mac_set;		/* Ingress MAC address set on L3_IF */

	uint8_t eg_mac_addr[ETH_ALEN];	/* Egress MAC address of the L3 interface */
	bool is_eg_mac_set;		/* Egress MAC address set on L3_IF */
};

uint16_t ppe_drv_l3_if_get_index(struct ppe_drv_l3_if *l3_if);
void ppe_drv_l3_if_dmac_check_set(struct ppe_drv_l3_if *l3_if, bool enable);

bool ppe_drv_l3_if_mtu_mru_set(struct ppe_drv_l3_if *l3_if, uint16_t mtu, uint16_t mru);
bool ppe_drv_l3_if_mtu_mru_clear(struct ppe_drv_l3_if *l3_if);
bool ppe_drv_l3_if_mtu_mru_disable(struct ppe_drv_l3_if *l3_if);

bool ppe_drv_l3_if_ig_mac_add_and_ref(struct ppe_drv_l3_if *l3_if);
bool ppe_drv_l3_if_ig_mac_deref(struct ppe_drv_l3_if *l3_if);

bool ppe_drv_l3_if_eg_mac_addr_set(struct ppe_drv_l3_if *l3_if, const uint8_t *mac_addr);
bool ppe_drv_l3_if_eg_mac_addr_clear(struct ppe_drv_l3_if *l3_if);

bool ppe_drv_l3_if_mac_addr_set(struct ppe_drv_l3_if *l3_if, const uint8_t *mac_addr);
bool ppe_drv_l3_if_mac_addr_clear(struct ppe_drv_l3_if *l3_if);

struct ppe_drv_pppoe *ppe_drv_l3_if_pppoe_get(struct ppe_drv_l3_if *l3_if);
void ppe_drv_l3_if_pppoe_clear(struct ppe_drv_l3_if *l3_if);
bool ppe_drv_l3_if_pppoe_match(struct ppe_drv_l3_if *l3_if, uint16_t session_id, uint8_t *smac);
bool ppe_drv_l3_if_pppoe_set(struct ppe_drv_l3_if *l3_if, struct ppe_drv_pppoe *pppoe);

struct ppe_drv_l3_if *ppe_drv_l3_if_alloc(enum ppe_drv_l3_if_type type);
struct ppe_drv_l3_if *ppe_drv_l3_if_ref(struct ppe_drv_l3_if *l3_if);
bool ppe_drv_l3_if_deref(struct ppe_drv_l3_if *l3_if);

bool ppe_drv_l3_if_disable_ttl_dec(struct ppe_drv_l3_if *l3_if, bool disable_ttl_dec);

void ppe_drv_l3_if_entries_free(struct ppe_drv_l3_if *l3_if);
struct ppe_drv_l3_if *ppe_drv_l3_if_entries_alloc(void);
