/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * PPE Iface valid handle flag.
 */
#define PPE_DRV_IFACE_FLAG_VALID	0x1
#define PPE_DRV_IFACE_FLAG_PORT_VALID	0x2
#define PPE_DRV_IFACE_FLAG_VSI_VALID	0x4
#define PPE_DRV_IFACE_FLAG_L3_IF_VALID	0x8
#define PPE_DRV_IFACE_VLAN_OVER_BRIDGE  0x10
#define PPE_DRV_IFACE_FLAG_WAN_IF_VALID 0x20
#define PPE_DRV_IFACE_FLAG_MHT_SWITCH_VALID 0x40

/*
 * ppe-iface cleanup function callback
 */
typedef ppe_drv_ret_t (*ppe_drv_iface_cleanup_cb)(struct ppe_drv_iface *iface);

/*
 * ppe_drv_iface
 *	PPE interface information
 */
struct ppe_drv_iface {
	struct ppe_drv_iface *base_if;		/* Base list for hierarchy creation */
	struct ppe_drv_iface *parent;		/* Pointer to parent ppe-if– used for bridge/lag slaves */
	struct ppe_drv_port *port;		/* Pointer to port structure */
	struct ppe_drv_vsi *vsi;		/* Pointer to vsi structure */
	struct ppe_drv_l3_if *l3;		/* Pointer to l3_if structure */
	struct kref ref;			/* Reference count */
	struct net_device *dev;			/* Corresponding net-device */
	uint16_t flags;				/* Flag to indicate valid handles */
	uint16_t index;				/* Interface index */
	enum ppe_drv_iface_type type;		/* Interface type */
	ppe_drv_iface_cleanup_cb cleanup_cb;	/* cleanup callback */
};

bool ppe_drv_iface_deref_internal(struct ppe_drv_iface *iface);
struct ppe_drv_iface *ppe_drv_iface_ref(struct ppe_drv_iface *iface);

struct ppe_drv_iface *ppe_drv_iface_get_by_dev_internal(struct net_device *dev);
struct ppe_drv_iface *ppe_drv_iface_get_by_idx(ppe_drv_iface_t idx);
int32_t ppe_drv_iface_vsi_idx_get(struct ppe_drv_iface *iface);
int32_t ppe_drv_iface_l3_if_idx_get(struct ppe_drv_iface *iface);

bool ppe_drv_iface_parent_set(struct ppe_drv_iface *iface, struct ppe_drv_iface *parent);
struct ppe_drv_iface *ppe_drv_iface_parent_get(struct ppe_drv_iface *iface);
void ppe_drv_iface_parent_clear(struct ppe_drv_iface *iface);

void ppe_drv_iface_base_set(struct ppe_drv_iface *iface, struct ppe_drv_iface *base);
struct ppe_drv_iface *ppe_drv_iface_base_get(struct ppe_drv_iface *iface);
void ppe_drv_iface_base_clear(struct ppe_drv_iface *iface);

bool ppe_drv_iface_port_set(struct ppe_drv_iface *iface, struct ppe_drv_port *port);
struct ppe_drv_port *ppe_drv_iface_port_get(struct ppe_drv_iface *iface);
void ppe_drv_iface_port_clear(struct ppe_drv_iface *iface);

bool ppe_drv_iface_vsi_set(struct ppe_drv_iface *iface, struct ppe_drv_vsi *vsi);
struct ppe_drv_vsi *ppe_drv_iface_vsi_get(struct ppe_drv_iface *iface);
void ppe_drv_iface_vsi_clear(struct ppe_drv_iface *iface);

bool ppe_drv_iface_l3_if_set(struct ppe_drv_iface *iface, struct ppe_drv_l3_if *l3_if);
struct ppe_drv_l3_if * ppe_drv_iface_l3_if_get(struct ppe_drv_iface *iface);
void ppe_drv_iface_l3_if_clear(struct ppe_drv_iface *iface);

void ppe_drv_iface_entries_free(struct ppe_drv_iface *iface);
struct ppe_drv_iface *ppe_drv_iface_entries_alloc(void);
