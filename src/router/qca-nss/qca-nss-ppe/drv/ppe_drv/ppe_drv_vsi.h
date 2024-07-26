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

#define PPE_DRV_VLAN_HDR_VLAN_NOT_CONFIGURED	0xFFF

#define PPE_DRV_VSI_PORT_BITMAP_SHIFT	0
#define PPE_DRV_VSI_UUC_BITMAP_SHIFT	8
#define PPE_DRV_VSI_UMC_BITMAP_SHIFT	16
#define PPE_DRV_VSI_BC_BITMAP_SHIFT	24

#define PPE_DRV_VSI_PORT_BITMAP_MASK	0xFF
#define PPE_DRV_VSI_UUC_BITMAP_MASK	0xFF
#define PPE_DRV_VSI_UMC_BITMAP_MASK	0xFF
#define PPE_DRV_VSI_BC_BITMAP_MASK	0xFF

#define PPE_DRV_VSI_PORT_BITMAP(i)	(((i) << PPE_DRV_VSI_PORT_BITMAP_SHIFT) && PPE_DRV_VSI_PORT_BITMAP_MASK)
#define PPE_DRV_VSI_UUC_BITMAP(i)	(((i) << PPE_DRV_VSI_UUC_BITMAP_SHIFT) && PPE_DRV_VSI_UUC_BITMAP_MASK)
#define PPE_DRV_VSI_UMC_BITMAP(i)	(((i) << PPE_DRV_VSI_UMC_BITMAP_SHIFT) && PPE_DRV_VSI_UMC_BITMAP_MASK)
#define PPE_DRV_VSI_BC_BITMAP(i)	(((i) << PPE_DRV_VSI_BC_BITMAP_SHIFT) && PPE_DRV_VSI_BC_BITMAP_MASK)

/*
 * ppe_drv_vsi_type
 *	PPE VSI types
 */
enum ppe_drv_vsi_type {
	PPE_DRV_VSI_TYPE_PORT,		/* VSI for a single port */
	PPE_DRV_VSI_TYPE_BRIDGE,	/* VSI for bridge interface */
	PPE_DRV_VSI_TYPE_VLAN,		/* VSI for VLAN interface */
	PPE_DRV_VSI_TYPE_MAX,		/* Max VSI types */
};

/*
 * ppe_drv_vsi_vlan
 *	Vlan information
 */
struct ppe_drv_vsi_vlan {
	uint32_t inner_vlan;	/* Inner vlan id */
	uint32_t outer_vlan;	/* Outer vlan id */
};

/*
 * ppe_drv_vsi
 *	VSI information
 */
struct ppe_drv_vsi {
	struct ppe_drv_l3_if *l3_if;	/* Pointer to associated L3 interface */
	struct ppe_drv_vsi_vlan vlan;	/* VLAN information for a given VSI */
	struct kref ref;		/* Reference count */
	bool is_fdb_learn_enabled;	/* FDB learning enabled */
	uint8_t index;			/* vsi number */
	uint8_t type;			/* vsi type */
};

bool ppe_drv_vlan_del_untag_ingress_rule(struct ppe_drv_port *port, struct ppe_drv_l3_if *src_l3_if);
bool ppe_drv_vlan_add_untag_ingress_rule(struct ppe_drv_port *port, struct ppe_drv_l3_if *src_l3_if);


bool ppe_drv_vsi_set_vlan(struct ppe_drv_vsi *vsi, uint32_t vlan_id, struct ppe_drv_iface *nh_iface);
bool ppe_drv_vsi_match_vlan(struct ppe_drv_vsi *vsi, uint32_t inner_vlan, uint32_t outer_vlan);

void ppe_drv_vsi_mc_disable(struct ppe_drv_vsi *vsi);
void ppe_drv_vsi_mc_enable(struct ppe_drv_vsi *vsi);

void ppe_drv_vsi_l3_if_detach(struct ppe_drv_vsi *vsi);
void ppe_drv_vsi_l3_if_attach(struct ppe_drv_vsi *vsi, struct ppe_drv_l3_if *l3_if);

struct ppe_drv_vsi *ppe_drv_vsi_ref(struct ppe_drv_vsi *vsi);
bool ppe_drv_vsi_deref(struct ppe_drv_vsi *vsi);

void ppe_drv_vsi_l3_if_deref(struct ppe_drv_vsi *vsi);
struct ppe_drv_l3_if *ppe_drv_vsi_l3_if_get_and_ref(struct ppe_drv_vsi *vsi);

struct ppe_drv_vsi *ppe_drv_vsi_alloc(enum ppe_drv_vsi_type type);

void ppe_drv_vsi_entries_free(struct ppe_drv_vsi *vsi);
struct ppe_drv_vsi *ppe_drv_vsi_entries_alloc(void);
