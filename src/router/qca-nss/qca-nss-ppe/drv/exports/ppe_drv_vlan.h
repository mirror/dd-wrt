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
#ifndef _PPE_DRV_VLAN_H_
#define _PPE_DRV_VLAN_H_

#include <fal/fal_portvlan.h>

struct ppe_drv_iface;

#define PPE_DRV_MAX_VLAN 2

/*
 * ppe_drv_vlan_type
 *	Flag indicating packet is vlan tagged or untagged.
 */
enum ppe_drv_vlan_type {
	PPE_DRV_VLAN_UNTAGGED,	/**< VLAN untagged. */
	PPE_DRV_VLAN_TAGGED	/**< VLAN tagged. */
};

/*
 * ppe_drv_vlan
 *	VLAN information
 */
struct ppe_drv_vlan {
	uint16_t tpid;		/**< TPID in VLAN header. */
	uint16_t tci;		/**< TCI in VLAN header. */
};

/*
 * ppe_drv_vlan_xlate_info()
 *	VLAN translation info
 */
struct ppe_drv_vlan_xlate_info {
	struct ppe_drv_iface *br;				/**< Bridge PPE interface */
	uint32_t port_id;					/**< Port-ID */
	uint32_t cvid;						/**< CVID to program in XLATE tables */
	uint32_t svid;						/**< SVID to program in XLATE tables */
};

/**
 * ppe_drv_vlan_tpid_set
 *	Set VLAN tpid in PPE vlan tables.
 *
 * @param[in] ctpid   C-tag TPID.
 * @param[in] stpid   S-tag TPID.
 * @param[in] mask    Mask for Egress and Ingress vlan enable.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_tpid_set(uint16_t ctpid, uint16_t stpid, uint32_t mask);

/**
 * ppe_drv_vlan_port_role_set
 *	Set VLAN port role.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface  PPE interface for vlan device.
 * @param[in] port_id  Port index number.
 * @param[in] mode    Ingress and/or egress mode.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_port_role_set(struct ppe_drv_iface *iface, uint32_t port_id, fal_port_qinq_role_t *mode);

/**
 * ppe_drv_vlan_del_xlate_rule
 *	Delete vlan translation rules.
 *
 * @datatypes
 * ppe_drv_iface
 * ppe_drv_vlan_xlate_info
 *
 * @param[in] iface  PPE interface for vlan device.
 * @param[in] ppe_drv_vlan_xlate_info	Translation info.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_del_xlate_rule(struct ppe_drv_iface *iface, struct ppe_drv_vlan_xlate_info *info);

/*
 * ppe_drv_vlan_over_bridge_del_ig_rule
 * 	Deleting ingress xlate rules for the given iface
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface PPE interface of the slave
 * @param[in] iface PPE interface of the VLAN
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_over_bridge_del_ig_rule(struct ppe_drv_iface *slave_iface,
						   struct ppe_drv_iface *vlan_iface);
/*
 * ppe_drv_vlan_over_bridge_add_ig_rule
 * 	Installing ingress xlate rules for the given iface
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface PPE interface of the slave
 * @param[in] iface PPE interface of the VLAN
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_over_bridge_add_ig_rule(struct ppe_drv_iface *slave_iface,
						   struct ppe_drv_iface *vlan_iface);
/**
 * ppe_drv_vlan_as_vp_del_xlate_rules
 *	Delete vlan translation rules with VP.
 *
 * @datatypes
 * ppe_drv_iface
 * ppe_drv_vlan_xlate_info
 *
 * @param[in] iface  PPE interface for vlan device.
 * @param[in] ppe_drv_vlan_xlate_info Translation info.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_as_vp_del_xlate_rules(struct ppe_drv_iface *iface, struct ppe_drv_vlan_xlate_info *info);

/**
 * ppe_drv_vlan_add_xlate_rule
 *	Add vlan translation rules.
 *
 * @datatypes
 * ppe_drv_iface
 * ppe_drv_vlan_xlate_info
 *
 * @param[in] iface  PPE interface for vlan device.
 * @param[in] ppe_drv_vlan_xlate_info Translation info.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_add_xlate_rule(struct ppe_drv_iface *iface, struct ppe_drv_vlan_xlate_info *info);

/**
 * ppe_drv_vlan_as_vp_add_xlate_rules
 *	Add vlan translation rules with VP.
 *
 * @datatypes
 * ppe_drv_iface
 * ppe_drv_vlan_xlate_info
 *
 * @param[in] iface  PPE interface for vlan device.
 * @param[in] ppe_drv_vlan_xlate_info Translation info.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_as_vp_add_xlate_rules(struct ppe_drv_iface *iface, struct ppe_drv_vlan_xlate_info *info);

/**
 * ppe_drv_vlan_deinit
 *	Deinitialize vlan.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface  PPE interface for vlan device.
 *
 * @return
 * Status of the operation.
 */
void ppe_drv_vlan_deinit(struct ppe_drv_iface *iface);

/**
 * ppe_drv_vlan_init
 *	Initialize vlan.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 *
 * @param[in] iface  PPE interface for vlan device.
 * @param[in] base_dev  Base net device on which vlan is created.
 * @param[in] vlan_id  vlan_id.
 * @param[in] vlan_over_bridge VLAN interface is created over bridge
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_init(struct ppe_drv_iface *iface, struct net_device *base_dev, uint32_t vlan_id,
				bool vlan_over_bridge);

/**
 * ppe_drv_vlan_lag_slave_join
 *	slave dev inside lag join vlan.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 *
 * @param[in] vlan_iface  PPE interface for vlan device.
 * @param[in] slave_dev  Slave net device
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_lag_slave_join(struct ppe_drv_iface *vlan_iface, struct net_device *slave_dev);

/**
 * ppe_drv_vlan_lag_slave_leave
 *	slave dev inside lag leave vlan.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 *
 * @param[in] vlan_iface  PPE interface for vlan device.
 * @param[in] slave_dev  Slave net device
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_vlan_lag_slave_leave(struct ppe_drv_iface *vlan_iface, struct net_device *slave_dev);

#endif /* _PPE_DRV_VLAN_H_ */
