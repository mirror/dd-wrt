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

/*
 * ppe_drv_nexthop_type
 *	PPE nexthop entry type
 */
enum ppe_drv_nexthop_type {
	PPE_DRV_NEXTHOP_TYPE_L3_PPPOE,	/* Nexthop entry provide egress VSI */
	PPE_DRV_NEXTHOP_TYPE_VP,	/* Nexthop entry provide egress port */
};

/*
 * ppe_drv_nexthop
 *	Nexthop information
 */
struct ppe_drv_nexthop {
	struct list_head list;		/* list head to chain nexthop entries in free or active list */
	struct ppe_drv_pub_ip *pub_ip;	/* Pointer to associated pub ip entry */
	struct ppe_drv_port *dest_port;	/* Destination port */
	struct ppe_drv_l3_if *l3_if;	/* Egress l3_if */
	struct kref ref;		/* Reference count object */
	uint32_t dnat_ip;		/* Destination NAT ip */
	uint32_t inner_vlan;		/* Inner vlan ID associated with this entry */
	uint32_t outer_vlan;		/* Outer vlan ID associated with this entry */
	uint16_t index;			/* Index pointed to by this element */
	uint8_t mac_addr[ETH_ALEN];	/* MAC address of nexthop entity */
	uint8_t snat_en;		/* NEXTHOP has Source NAT */
	uint8_t dnat_en;		/* NEXTHOP has Destination NAT */
};

void ppe_drv_nexthop_dump(struct ppe_drv_nexthop *nh);
bool ppe_drv_nexthop_deref(struct ppe_drv_nexthop *nh);
#ifdef NSS_PPE_IPQ53XX
struct ppe_drv_nexthop *ppe_drv_nexthop_v4_bridge_flow_get_and_ref(struct ppe_drv_v4_conn_flow *pcf);
struct ppe_drv_nexthop *ppe_drv_nexthop_v6_bridge_flow_get_and_ref(struct ppe_drv_v6_conn_flow *pcf);
#endif
struct ppe_drv_nexthop *ppe_drv_nexthop_v4_get_and_ref(struct ppe_drv_v4_conn_flow *pcf);
struct ppe_drv_nexthop *ppe_drv_nexthop_v6_get_and_ref(struct ppe_drv_v6_conn_flow *pcf);

void ppe_drv_nexthop_entries_free(struct ppe_drv_nexthop *nexthop);
struct ppe_drv_nexthop *ppe_drv_nexthop_entries_alloc(void);
