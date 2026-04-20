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

#include "ppe_drv.h"
#include <fal/fal_fdb.h>
#include <fal/fal_mirror.h>

/*
 * ppe_drv_dp_set_mirror_if()
 *	set ingress/egress mirror interface
 */
ppe_drv_ret_t ppe_drv_dp_set_mirror_if(struct ppe_drv_iface *iface,
		ppe_drv_dp_mirror_direction_t direction, bool enable)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *port;
	sw_error_t err;

	spin_lock_bh(&p->lock);
	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get port from iface\n", iface);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	switch (direction) {
	case PPE_DRV_DP_MIRR_DI_IN:
		err = fal_mirr_port_in_set(PPE_DRV_SWITCH_ID, port->port, enable);
		if (err != SW_OK) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("Failed to %s ingress mirror for port %u\n",
					(enable ? "enable" : "disable"), port->port);
			return PPE_DRV_RET_SET_MIRROR_IN_FAIL;
		}

		break;

	case PPE_DRV_DP_MIRR_DI_EG:
		err = fal_mirr_port_eg_set(PPE_DRV_SWITCH_ID, port->port, enable);
		if (err != SW_OK) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("Failed to set %s egress mirror for port %u\n",
					(enable ? "enable" : "disable"), port->port);
			return PPE_DRV_RET_SET_MIRROR_EG_FAIL;
		}

		break;

	default:
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Failed to set Mirror direction: %u direction \
				is not supported\n", direction);
		return PPE_DRV_RET_SET_MIRROR_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("Set %s %s mirror for port %u\n",
			(enable ? "enable" : "disable"),
			(direction == PPE_DRV_DP_MIRR_DI_IN ? "ingress" : "egress"),
			port->port);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_dp_set_mirror_if);

/*
 * ppe_drv_dp_set_mirr_analysis_port()
 *	set ingress/egress mirror analysis interface
 */
ppe_drv_ret_t ppe_drv_dp_set_mirr_analysis_port(struct ppe_drv_iface *iface,
		ppe_drv_dp_mirror_direction_t direction, bool enable, uint8_t priority)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *port;
	sw_error_t err;
	fal_mirr_analysis_config_t analysis_cfg = {0};

	spin_lock_bh(&p->lock);
	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get port from iface\n", iface);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	/* if api called for disable port then set default/invalid port */
	if (enable) {
		analysis_cfg.port_id = port->port;
	} else {
		analysis_cfg.port_id = PPE_DRV_MIRR_INVAL_PORT;
	}

	analysis_cfg.priority = priority;
	err = fal_mirr_analysis_config_set(PPE_DRV_SWITCH_ID, (fal_mirr_direction_t)direction,
			&analysis_cfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Failed to set %s mirror analysis port for port num %u\n",
				(direction == PPE_DRV_DP_MIRR_DI_IN ? "ingress" : "egress"),
				port->port);
		return PPE_DRV_RET_SET_MIRROR_ANALYSIS_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%s mirror analysis port is set for port num %u\n",
			(direction == PPE_DRV_DP_MIRR_DI_IN ? "Ingress" : "Egress"),
			port->port);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_dp_set_mirr_analysis_port);

/*
 * ppe_drv_dp_get_mirr_analysis_port()
 *	get ingress/egress mirror analysis interface
 */
ppe_drv_ret_t ppe_drv_dp_get_mirr_analysis_port(
		ppe_drv_dp_mirror_direction_t direction, uint8_t *port_num)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	sw_error_t err;
	fal_mirr_analysis_config_t analysis_cfg = {0};

	spin_lock_bh(&p->lock);
	err = fal_mirr_analysis_config_get(PPE_DRV_SWITCH_ID, (fal_mirr_direction_t)direction,
			&analysis_cfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Failed to get %s mirror analysis port for port num\n",
				(direction == PPE_DRV_DP_MIRR_DI_IN ? "ingress" : "egress"));
		return PPE_DRV_RET_GET_MIRROR_ANALYSIS_FAIL;
	}

	spin_unlock_bh(&p->lock);

	/* if port number is PPE_DRV_MIRR_INVAL_PORT then return mo port set */
	if (analysis_cfg.port_id == PPE_DRV_MIRR_INVAL_PORT) {
		ppe_drv_info("No %s mirror analysis port is set\n",
				(direction == PPE_DRV_DP_MIRR_DI_IN ? "Ingress" : "Egress"));
		return PPE_DRV_RET_GET_MIRROR_ANALYSIS_NO_PORT;
	}

	*port_num = (uint8_t)analysis_cfg.port_id;
	return PPE_DRV_RET_SUCCESS;

}
EXPORT_SYMBOL(ppe_drv_dp_get_mirr_analysis_port);

/*
 * ppe_drv_dp_deinit()
 *	De-initialize a physical port.
 */
ppe_drv_ret_t ppe_drv_dp_deinit(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv_port *port;

	spin_lock_bh(&p->lock);
	l3_if = ppe_drv_iface_l3_if_get(iface);
	if (!l3_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get l3_if from iface\n", iface);
		return PPE_DRV_RET_L3_IF_NOT_FOUND;
	}

	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get port from iface\n", iface);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	/*
	 * Detach l3_if from port
	 * Dereference l3_if
	 * Dereference port
	 */
	ppe_drv_port_l3_if_detach(port, l3_if);
	port->port_l3_if = NULL;
	ppe_drv_l3_if_deref(l3_if);

	ppe_drv_port_deref(port);

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_dp_deinit);

/*
 * ppe_drv_dp_init()
 *	Initialize a physical port.
 */
ppe_drv_ret_t ppe_drv_dp_init(struct ppe_drv_iface *iface, uint32_t macid,
				bool mht_dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv_port *port;

	spin_lock_bh(&p->lock);
	port = ppe_drv_port_phy_alloc(macid, iface->dev);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get a valid port of type(%d)\n", iface, PPE_DRV_PORT_PHYSICAL);
		return PPE_DRV_RET_PORT_ALLOC_FAIL;
	}

	l3_if = ppe_drv_l3_if_alloc(PPE_DRV_L3_IF_TYPE_PORT);
	if (!l3_if) {
		ppe_drv_port_deref(port);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get a valid l3_if of type(%d)\n", iface, PPE_DRV_L3_IF_TYPE_PORT);
		return PPE_DRV_RET_L3_IF_ALLOC_FAIL;
	}

	/*
	 * Attach l3_if to port
	 */
	if (!ppe_drv_port_l3_if_attach(port, l3_if)) {
		ppe_drv_port_deref(port);
		ppe_drv_l3_if_deref(l3_if);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to attach valid l3_if(%p) to port(%p)\n", iface, l3_if, port);
		return PPE_DRV_RET_L3_IF_PORT_ATTACH_FAIL;
	}

	port->port_l3_if = l3_if;

	ppe_drv_iface_port_set(iface, port);
	ppe_drv_iface_l3_if_set(iface, l3_if);

	/*
	 * If MHT device is set, configure iface with mht flag.
	 */
	if (mht_dev)
		iface->flags |= PPE_DRV_IFACE_FLAG_MHT_SWITCH_VALID;

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_dp_init);

/*
 * ppe_drv_dp_set_ppe_offload_enable_flag()
 *	API to set PPE offload enable flag in PPE port
 */
ppe_drv_ret_t ppe_drv_dp_set_ppe_offload_enable_flag(struct ppe_drv_iface *iface,
		bool disable)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *port;

	spin_lock_bh(&p->lock);
	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get port from iface\n", iface);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	/*
	 * Set the OFFLOAD enabled flag in the PPE port if the input
	 * flag indicates so.
	 */
	if (!disable) {
		port->flags |= PPE_DRV_PORT_FLAG_OFFLOAD_ENABLED;
		if_bm_to_offload |= (1 << (port->port - 1));
	}

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_dp_set_ppe_offload_enable_flag);
