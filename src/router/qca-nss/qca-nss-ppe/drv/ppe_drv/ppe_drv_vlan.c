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

#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <fal/fal_rss_hash.h>
#include <fal/fal_ip.h>
#include <fal/fal_init.h>
#include <fal/fal_pppoe.h>
#include <fal/fal_tunnel.h>
#include <fal/fal_api.h>
#include <fal/fal_vsi.h>
#include <ref/ref_vsi.h>
#include <fal/fal_portvlan.h>
#include "ppe_drv.h"
#include "ppe_drv_stats.h"

/*
 * ppe_drv_vlan_del_untag_ingress_rule()
 *	Delete ingress VLAN translation rule for untagged frames
 */
bool ppe_drv_vlan_del_untag_ingress_rule(struct ppe_drv_port *port, struct ppe_drv_l3_if *src_l3_if)
{
	fal_vlan_trans_adv_rule_t xlt_rule = {0};
	fal_vlan_trans_adv_action_t xlt_action = {0};
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_port_t fal_port;
	sw_error_t err;

	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(port->port) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, port->port)
			: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, port->port);

	/*
	 * Fields for match
	 */
	xlt_rule.s_tagged = 0x1;				/* Accept untagged svlan */
	xlt_rule.c_tagged = 0x1;				/* Accept untagged cvlan */

	/*
	 * Fields for action
	 */
	xlt_action.src_info_enable = A_TRUE;
	xlt_action.src_info_type = 1;
	xlt_action.src_info = src_l3_if->l3_if_index;

	err = fal_port_vlan_trans_adv_del(PPE_DRV_SWITCH_ID, fal_port, FAL_PORT_VLAN_INGRESS, &xlt_rule,
				&xlt_action);
	if (err != SW_OK) {
		ppe_drv_warn("%px: Failed to delete ingress translation rule for untagged VLAN port: %d, error: %d\n",
				p, fal_port, err);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_ingress_untag_vlan_del);
		return false;
	}

	ppe_drv_info("%p: Deleted untag vlan rule for port: %d, with src_l3_if: %d", p, port->port, src_l3_if->l3_if_index);
	return true;
}

/*
 * ppe_drv_vlan_add_untag_ingress_rule()
 *	Add Ingress VLAN translation rule for untagged frames
 */
bool ppe_drv_vlan_add_untag_ingress_rule(struct ppe_drv_port *port, struct ppe_drv_l3_if *src_l3_if)
{
	fal_vlan_trans_adv_rule_t xlt_rule = {0};
	fal_vlan_trans_adv_action_t xlt_action = {0};
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_port_t fal_port;
	sw_error_t err;

	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(port->port) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, port->port)
			: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, port->port);

	/*
	 * Fields for match
	 */
	xlt_rule.s_tagged = 0x1;				/* Accept untagged svlan */
	xlt_rule.c_tagged = 0x1;				/* Accept untagged cvlan */

	/*
	 * Fields for action
	 */
	xlt_action.src_info_enable = A_TRUE;
	xlt_action.src_info_type = 1;
	xlt_action.src_info = src_l3_if->l3_if_index;

	err = fal_port_vlan_trans_adv_add(PPE_DRV_SWITCH_ID, fal_port, FAL_PORT_VLAN_INGRESS, &xlt_rule,
				&xlt_action);
	if (err != SW_OK) {
		ppe_drv_warn("%px: Failed to update ingress translation rule for untagged VLAN port: %d, error: %d\n",
				p, fal_port, err);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_ingress_untag_vlan_add);
		return false;
	}

	ppe_drv_info("%p: Added untag vlan rule for port: %d, with src_l3_if: %d", p, port->port, src_l3_if->l3_if_index);
	return true;
}

/*
 * ppe_drv_vlan_tpid_set()
 *	Set PPE vlan TPID
 */
ppe_drv_ret_t ppe_drv_vlan_tpid_set(uint16_t ctpid, uint16_t stpid, uint32_t mask)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	fal_tpid_t tpid;

	tpid.mask = mask;
	tpid.ctpid = ctpid;
	tpid.stpid = stpid;
	tpid.tunnel_ctpid = ctpid;
	tpid.tunnel_stpid = stpid;

	spin_lock_bh(&p->lock);
	if ((fal_ingress_tpid_set(PPE_DRV_SWITCH_ID, &tpid) != SW_OK) || (fal_egress_tpid_set(PPE_DRV_SWITCH_ID, &tpid) != SW_OK)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("failed to set ctpid %d stpid %d\n", tpid.ctpid, tpid.stpid);
		return PPE_DRV_RET_VLAN_TPID_FAIL;
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;

}
EXPORT_SYMBOL(ppe_drv_vlan_tpid_set);

/*
 * ppe_drv_vlan_port_role_set()
 *	Set VLAN port role
 */
ppe_drv_ret_t ppe_drv_vlan_port_role_set(struct ppe_drv_iface *iface, uint32_t port_id, fal_port_qinq_role_t *mode)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_port_t fal_port;

	spin_lock_bh(&p->lock);
	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(port_id) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, port_id)
			: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, port_id);

	if (fal_port_qinq_mode_set(PPE_DRV_SWITCH_ID, fal_port, mode) != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("failed to set %d as edge port\n", fal_port);
		return PPE_DRV_RET_PORT_ROLE_FAIL;
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_port_role_set);

/*
 * ppe_drv_vlan_ingress_rule_action_set_vp()
 *	Set xlt_rule and xlt_action structure for ingress.
 */
void ppe_drv_vlan_ingress_rule_action_set_vp(fal_vlan_trans_adv_rule_t *xlt_rule,
					fal_vlan_trans_adv_action_t *xlt_action, struct ppe_drv_vlan_xlate_info *info)
{
	/*
         * Field for ingress match.
	 * Accept tagged/untagged/priority tagged svlan and cvlan.
         */
	xlt_rule->s_tagged = (FAL_PORT_VLAN_XLT_MATCH_UNTAGGED | FAL_PORT_VLAN_XLT_MATCH_TAGGED
			| FAL_PORT_VLAN_XLT_MATCH_PRIO_TAG);
	xlt_rule->c_tagged = (FAL_PORT_VLAN_XLT_MATCH_UNTAGGED | FAL_PORT_VLAN_XLT_MATCH_TAGGED
			| FAL_PORT_VLAN_XLT_MATCH_PRIO_TAG);
	xlt_rule->c_vid = (info->cvid == 0xFFFF) ? 0 : info->cvid;
	xlt_rule->c_vid_enable = (info->cvid == 0xFFFF) ? A_FALSE : A_TRUE;
	xlt_rule->s_vid = (info->svid == 0xFFFF) ? 0 : info->svid;
	xlt_rule->s_vid_enable = (info->svid == 0xFFFF) ? A_FALSE : A_TRUE;

	/*
	 * field for ingress action.
	 */
	xlt_action->cvid_xlt_cmd = (info->cvid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_DELETE;
	xlt_action->cvid_xlt = (info->cvid == 0xFFFF) ? 0 : info->cvid;
	xlt_action->svid_xlt_cmd = (info->svid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_DELETE;
	xlt_action->svid_xlt = (info->svid == 0xFFFF) ? 0 : info->svid;
	xlt_action->src_info_enable = A_TRUE;
	xlt_action->src_info = info->port_id;
	xlt_action->src_info_type = FAL_CHG_SRC_TYPE_VP;
}

/*
 * ppe_drv_vlan_egress_rule_action_set_vp()
 *	Set xlt_rule and xlt_action structure for egress.
 */
void ppe_drv_vlan_egress_rule_action_set_vp(fal_vlan_trans_adv_rule_t *xlt_rule,
					fal_vlan_trans_adv_action_t *xlt_action, struct ppe_drv_vlan_xlate_info *info)
{
	/*
	 * Fields for egress match.
	 * Accept tagged/untagged/priority tagged svlan and cvlan.
	 */
	xlt_rule->s_tagged = (FAL_PORT_VLAN_XLT_MATCH_UNTAGGED | FAL_PORT_VLAN_XLT_MATCH_TAGGED
			| FAL_PORT_VLAN_XLT_MATCH_PRIO_TAG);
	xlt_rule->c_tagged = (FAL_PORT_VLAN_XLT_MATCH_UNTAGGED | FAL_PORT_VLAN_XLT_MATCH_TAGGED
			| FAL_PORT_VLAN_XLT_MATCH_PRIO_TAG);

	/*
	 * Fields for egress action.
	 */
	xlt_action->cvid_xlt_cmd = (info->cvid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	xlt_action->cvid_xlt = (info->cvid == 0xFFFF) ? 0 : info->cvid;
	xlt_action->svid_xlt_cmd = (info->svid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	xlt_action->svid_xlt = (info->svid == 0xFFFF) ? 0 : info->svid;
}

/*
 * ppe_drv_vlan_as_vp_del_xlate_rules()
 *	Delete Ingress and Egress VLAN translation rules for VP.
 */
ppe_drv_ret_t ppe_drv_vlan_as_vp_del_xlate_rules(struct ppe_drv_iface *iface, struct ppe_drv_vlan_xlate_info *info)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_vlan_trans_adv_rule_t xlt_rule = {0};
	fal_vlan_trans_adv_action_t xlt_action = {0};
	fal_port_t fal_port, base_f_port;
	sw_error_t rc;
	struct net_device *base_dev;
	struct ppe_drv_iface *base_if;
	uint8_t b_port;

	base_dev = vlan_dev_priv(iface->dev)->real_dev;
	if (base_dev && is_vlan_dev(base_dev)) {
		base_dev = vlan_dev_priv(base_dev)->real_dev;
	}

	if (!base_dev) {
		ppe_drv_warn("%s: failed to obtain base_dev", iface->dev->name);
		return PPE_DRV_RET_BASE_IFACE_NOT_FOUND;
	}

	spin_lock_bh(&p->lock);
	base_if = ppe_drv_iface_get_by_dev_internal(base_dev);
	if (!base_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: NULL base interface for iface\n", base_dev);
		return PPE_DRV_RET_BASE_IFACE_NOT_FOUND;
	}

	b_port = ppe_drv_iface_port_idx_get(base_if);
	if (b_port == -1) {
		spin_unlock_bh(&p->lock);
		ppe_drv_info("%px: %s:%d is invalid port\n", base_dev, base_dev->name, b_port);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	ppe_drv_iface_base_set(iface, base_if);

	ppe_drv_vlan_ingress_rule_action_set_vp(&xlt_rule, &xlt_action, info);
	base_f_port = PPE_DRV_VIRTUAL_PORT_CHK(b_port) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, b_port)
		: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, b_port);

	ppe_drv_trace("%px: rule cvid: %d, rule svid: %d, act cvid: %d, act svid: %d, act src info: %d, port: %d\n",
			iface, xlt_rule.c_vid, xlt_rule.s_vid, xlt_action.cvid_xlt,
			xlt_action.svid_xlt, xlt_action.src_info, b_port);
	/*
	 * Delete ingress vlan translation rule.
	 */
	rc = fal_port_vlan_trans_adv_del(PPE_DRV_SWITCH_ID, base_f_port, FAL_PORT_VLAN_INGRESS,
			&xlt_rule, &xlt_action);
	if (rc != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Failed to delete old ingress vlan translation of port %d, error: %d\n"
				, base_f_port, rc);
		return PPE_DRV_RET_VLAN_INGRESS_DEL_FAIL;
	}

	memset(&xlt_rule, 0, sizeof(xlt_rule));
	memset(&xlt_action, 0, sizeof(xlt_action));

	ppe_drv_vlan_egress_rule_action_set_vp(&xlt_rule, &xlt_action, info);
	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(info->port_id) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, info->port_id)
		: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, info->port_id);

	ppe_drv_trace("%px: act cvid: %d, act svid: %d, port: %d\n", iface, xlt_action.cvid_xlt,
			xlt_action.svid_xlt, info->port_id);
	/*
	 * Delete egress vlan translation rule.
	 */
	rc = fal_port_vlan_trans_adv_del(PPE_DRV_SWITCH_ID, fal_port, FAL_PORT_VLAN_EGRESS,
			&xlt_rule, &xlt_action);
	if (rc != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Failed to delete old egress vlan translation of port %d, error: %d\n", fal_port, rc);
		return PPE_DRV_RET_VLAN_EGRESS_DEL_FAIL;
	}

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_as_vp_del_xlate_rules);

/*
 * ppe_drv_vlan_as_vp_add_xlate_rules()
 *	Add Ingress and Egress VLAN translation rules for VLAN created as VP.
 */
ppe_drv_ret_t ppe_drv_vlan_as_vp_add_xlate_rules(struct ppe_drv_iface *iface, struct ppe_drv_vlan_xlate_info *info)
{
	fal_vlan_trans_adv_rule_t xlt_rule = {0};
	fal_vlan_trans_adv_action_t xlt_action = {0};
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_port_t fal_port, base_f_port;
	struct ppe_drv_iface *base_if;
	struct net_device *base_dev;
	struct vlan_dev_priv *dev_priv;
	uint8_t b_port;
	int ret;

	dev_priv = (iface->dev? vlan_dev_priv(iface->dev): NULL);
	base_dev = dev_priv ? dev_priv->real_dev: NULL;
	if (base_dev && is_vlan_dev(base_dev)) {
		base_dev = vlan_dev_priv(base_dev)->real_dev;
	}

	if (!base_dev) {
		ppe_drv_warn("%s: failed to obtain base_dev", iface->dev->name);
		return PPE_DRV_RET_BASE_DEV_NOT_FOUND;
	}

	spin_lock_bh(&p->lock);
	base_if = ppe_drv_iface_get_by_dev_internal(base_dev);
	if (!base_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: NULL base interface for iface\n", base_dev);
		return PPE_DRV_RET_BASE_IFACE_NOT_FOUND;
	}

	b_port = ppe_drv_iface_port_idx_get(base_if);
	if (b_port == -1) {
		spin_unlock_bh(&p->lock);
		ppe_drv_info("%px: %s:%d is invalid port\n", base_dev, base_dev->name, b_port);
		return PPE_DRV_RET_BASE_PORT_NOT_FOUND;
	}

	ppe_drv_iface_base_set(iface, base_if);

	ppe_drv_vlan_ingress_rule_action_set_vp(&xlt_rule, &xlt_action, info);
	base_f_port = PPE_DRV_VIRTUAL_PORT_CHK(b_port) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, b_port)
		: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, b_port);

	ppe_drv_trace("%px: rule cvid: %d, rule svid: %d, act cvid: %d, act svid: %d, act src info: %d, port: %d\n",
			iface, xlt_rule.c_vid, xlt_rule.s_vid, xlt_action.cvid_xlt,
			xlt_action.svid_xlt, xlt_action.src_info, b_port);

	/*
	 * Add ingress vlan translation rule.
	 * For adding ingress rule we are using base physical port number
	 * becasue packet rx happen on gmac, so we need to add a ingress
	 * rule/action to match tagged packet, which untag the packet and
	 * give it to src_info port, which is VP.
	 */
	ret = fal_port_vlan_trans_adv_add(PPE_DRV_SWITCH_ID, base_f_port,
			FAL_PORT_VLAN_INGRESS, &xlt_rule,
			&xlt_action);
	if (ret != SW_OK) {
		ppe_drv_warn("%px: Failed to update ingress translation rule for port: %d, error: %d\n"
				, iface, base_f_port, ret);
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_ingress_vlan_add);
		return PPE_DRV_RET_INGRESS_VLAN_FAIL;
	}

	memset(&xlt_rule, 0, sizeof(xlt_rule));
	memset(&xlt_action, 0, sizeof(xlt_action));

	ppe_drv_vlan_egress_rule_action_set_vp(&xlt_rule, &xlt_action, info);
	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(info->port_id) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, info->port_id)
			: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, info->port_id);

	ppe_drv_trace("%px: act cvid: %d, act svid: %d, port: %d\n", iface, xlt_action.cvid_xlt,
			xlt_action.svid_xlt, info->port_id);

	/*
	 * Add egress vlan translation rule.
	 */
	ret = fal_port_vlan_trans_adv_add(PPE_DRV_SWITCH_ID, fal_port, FAL_PORT_VLAN_EGRESS, &xlt_rule,
				&xlt_action);
	if (ret != SW_OK) {
		ppe_drv_warn("%px: Failed to update egress translation rule for port: %d, error: %d\n",
				iface, fal_port, ret);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_egress_vlan_add);

		/*
		 * Delete ingress vlan translation rule.
		 */
		if (ret != SW_ALREADY_EXIST) {
			memset(&xlt_rule, 0, sizeof(xlt_rule));
			memset(&xlt_action, 0, sizeof(xlt_action));
			ppe_drv_vlan_ingress_rule_action_set_vp(&xlt_rule, &xlt_action, info);
			fal_port_vlan_trans_adv_del(PPE_DRV_SWITCH_ID, base_f_port, FAL_PORT_VLAN_INGRESS,
												&xlt_rule, &xlt_action);
		}

		spin_unlock_bh(&p->lock);
		return PPE_DRV_RET_EGRESS_VLAN_FAIL;
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_as_vp_add_xlate_rules);

/*
 * ppe_drv_vlan_del_xlate_rule()
 *	Delete Ingress and Egress VLAN translation rules
 */
ppe_drv_ret_t ppe_drv_vlan_del_xlate_rule(struct ppe_drv_iface *iface, struct ppe_drv_vlan_xlate_info *info)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_vlan_trans_adv_rule_t xlt_rule;	/* VLAN Translation Rule */
	fal_vlan_trans_adv_action_t xlt_action;	/* VLAN Translation Action */
	struct ppe_drv_vsi *vsi;
	fal_port_t fal_port;
	int vsi_idx, rc;

	/*
	 * Check with vlan device created under bridge
	 */
	spin_lock_bh(&p->lock);
	if (info->br) {
		vsi = ppe_drv_iface_vsi_get(info->br);
	} else {
		vsi = ppe_drv_iface_vsi_get(iface);
	}

	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Invalid VSI for given iface\n", iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	vsi_idx = vsi->index;

	/*
	 * Delete old ingress vlan translation rule
	 */
	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(info->port_id) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, info->port_id)
			: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, info->port_id);


	rc = ppe_port_vlan_vsi_set(PPE_DRV_SWITCH_ID, fal_port, info->svid, info->cvid, PPE_VSI_INVALID);
	if (rc != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Failed to delete old ingress vlan translation rule of port %d, error: %d\n", fal_port, rc);
		return PPE_DRV_RET_VLAN_INGRESS_DEL_FAIL;
	}

	/*
	 * Add egress vlan translation rule
	 */
	memset(&xlt_rule, 0, sizeof(xlt_rule));
	memset(&xlt_action, 0, sizeof(xlt_action));

	/*
	 * Fields for match
	 */
	xlt_rule.vsi_valid = A_TRUE;				/* Use vsi as search key */
	xlt_rule.vsi_enable = A_TRUE;				/* Use vsi as search key */
	xlt_rule.vsi = vsi_idx;					/* Use vsi as search key */
	xlt_rule.s_tagged = 0x7;				/* Accept tagged/untagged/priority tagged svlan */
	xlt_rule.c_tagged = 0x7;				/* Accept tagged/untagged/priority tagged cvlan */

	/*
	 * Fields for action
	 */
	xlt_action.cvid_xlt_cmd = (info->cvid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	xlt_action.cvid_xlt = (info->cvid == 0xFFFF) ? 0 : info->cvid;
	xlt_action.svid_xlt_cmd = (info->svid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	xlt_action.svid_xlt = (info->svid == 0xFFFF) ? 0 : info->svid;

	/*
	 * Delete old egress vlan translation rule
	 */
	rc = fal_port_vlan_trans_adv_del(PPE_DRV_SWITCH_ID, fal_port, FAL_PORT_VLAN_EGRESS,
			&xlt_rule, &xlt_action);
	if (rc != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Failed to delete old egress vlan translation of port %d, error: %d\n", fal_port, rc);
		return PPE_DRV_RET_VLAN_EGRESS_DEL_FAIL;
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_del_xlate_rule);

/*
 * ppe_drv_vlan_add_xlate_rule()
 *	Add Ingress and Egress VLAN translation rules
 */
ppe_drv_ret_t ppe_drv_vlan_add_xlate_rule(struct ppe_drv_iface *iface, struct ppe_drv_vlan_xlate_info *info)
{
	fal_vlan_trans_adv_rule_t xlt_rule;
	fal_vlan_trans_adv_action_t xlt_action;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_vsi *vsi;
	int vsi_idx, ret, rc;
	fal_port_t fal_port;

	/*
	 * Check with vlan device created under bridge
	 */
	spin_lock_bh(&p->lock);
	if (info->br) {
		vsi = ppe_drv_iface_vsi_get(info->br);
	} else {
		vsi = ppe_drv_iface_vsi_get(iface);
	}

	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Invalid VSI for given iface\n", iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	vsi_idx = vsi->index;

	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(info->port_id) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, info->port_id)
			: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, info->port_id);

	/*
	 * Add new ingress vlan translation rule
	 */
	rc = ppe_port_vlan_vsi_set(PPE_DRV_SWITCH_ID, fal_port, info->svid, info->cvid, vsi_idx);
	if (rc != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_ingress_vlan_add);
		ppe_drv_warn("Failed to update ingress vlan translation of port %d, error: %d\n", fal_port, rc);
		return PPE_DRV_RET_INGRESS_VLAN_FAIL;
	}

	/*
	 * Add egress vlan translation rule
	 */
	memset(&xlt_rule, 0, sizeof(xlt_rule));
	memset(&xlt_action, 0, sizeof(xlt_action));

	/*
	 * Fields for match
	 */
	xlt_rule.vsi_valid = A_TRUE;				/* Use vsi as search key */
	xlt_rule.vsi_enable = A_TRUE;				/* Use vsi as search key */
	xlt_rule.vsi = vsi_idx;					/* Use vsi as search key */
	xlt_rule.s_tagged = 0x7;				/* Accept tagged/untagged/priority tagged svlan */
	xlt_rule.c_tagged = 0x7;				/* Accept tagged/untagged/priority tagged cvlan */

	/*
	 * Fields for action
	 */
	xlt_action.cvid_xlt_cmd = (info->cvid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	xlt_action.cvid_xlt = (info->cvid == 0xFFFF) ? 0 : info->cvid;
	xlt_action.svid_xlt_cmd = (info->svid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	xlt_action.svid_xlt = (info->svid == 0xFFFF) ? 0 : info->svid;

	ret = fal_port_vlan_trans_adv_add(PPE_DRV_SWITCH_ID, fal_port, FAL_PORT_VLAN_EGRESS, &xlt_rule,
				&xlt_action);
	if (ret != SW_OK) {
		ppe_drv_warn("%px: Failed to update egress translation rule for port: %d, error: %d\n",
				iface, fal_port, ret);

		/*
		 * Delete ingress vlan translation rule
		 */
		if (ret != SW_ALREADY_EXIST) {
			ppe_port_vlan_vsi_set(PPE_DRV_SWITCH_ID, fal_port, FAL_VLAN_INVALID, info->cvid, PPE_VSI_INVALID);
		}

		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_egress_vlan_add);

		return PPE_DRV_RET_EGRESS_VLAN_FAIL;
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_add_xlate_rule);


/*
 * ppe_drv_vlan_over_bridge_del_ig_rule
 *	Delete ingress xlate rule for the iface
 */
ppe_drv_ret_t ppe_drv_vlan_over_bridge_del_ig_rule(struct ppe_drv_iface *slave_iface,
						   struct ppe_drv_iface *vlan_iface)
{
	uint32_t port_id, ret = 0;
	fal_port_t fal_port;
	fal_vlan_trans_adv_action_t xlt_action = {0};
	fal_vlan_trans_adv_rule_t xlt_rule =  {0};
	struct ppe_drv_vsi *vsi;
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_iface_vsi_get(vlan_iface);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Invalid VSI for given iface\n", vlan_iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	port_id = ppe_drv_iface_port_idx_get(slave_iface);

	if (port_id == -1) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("PortId is invalid for %s\n", slave_iface->dev->name);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(port_id) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, port_id)
			: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, port_id);

	/*
	 * Field for match
	 */
	if (vsi->vlan.outer_vlan == PPE_DRV_VLAN_HDR_VLAN_NOT_CONFIGURED) {
		xlt_rule.s_tagged = FAL_PORT_VLAN_XLT_MATCH_UNTAGGED;
		xlt_rule.s_vid_enable = A_FALSE;
	} else {
		xlt_rule.s_tagged = FAL_PORT_VLAN_XLT_MATCH_TAGGED;
		xlt_rule.s_vid_enable = A_TRUE;
		xlt_rule.s_vid = vsi->vlan.outer_vlan;
	}
	xlt_rule.c_tagged = FAL_PORT_VLAN_XLT_MATCH_TAGGED;
	xlt_rule.c_vid_enable = A_TRUE;
	xlt_rule.c_vid = vsi->vlan.inner_vlan;

	/*
	 * Field for action
	 */
	xlt_action.vsi_xlt_enable = A_TRUE;
	xlt_action.vsi_xlt = vsi->index;

	ret = fal_port_vlan_trans_adv_del(PPE_DRV_SWITCH_ID, fal_port, FAL_PORT_VLAN_INGRESS, &xlt_rule,
					  &xlt_action);

	if (ret != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_ingress_vlan_over_bridge_del);
		ppe_drv_warn("Delete rule failed for %s portid %d ret %d\n", slave_iface->dev->name, port_id, ret);
		return PPE_DRV_RET_VLAN_INGRESS_DEL_FAIL;
	}

	spin_unlock_bh(&p->lock);

	ppe_drv_trace("Delete ingress success rule Outer VID %d Inner VID %d fal_port %d dev %s port_id %d\n",
		       vsi->vlan.outer_vlan, vsi->vlan.inner_vlan, fal_port, slave_iface->dev->name, port_id);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_over_bridge_del_ig_rule);

/*
 * ppe_drv_vlan_over_bridge_add_ig_rule
 *	Installing ingress xlate rule for the iface
 */
ppe_drv_ret_t ppe_drv_vlan_over_bridge_add_ig_rule(struct ppe_drv_iface *slave_iface,
						   struct ppe_drv_iface *vlan_iface)
{
	uint32_t port_id, ret = 0;
	fal_port_t fal_port;
	fal_vlan_trans_adv_action_t xlt_action = {0};
	fal_vlan_trans_adv_rule_t xlt_rule = {0};
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_vsi *vsi;

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_iface_vsi_get(vlan_iface);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Invalid VSI for given iface\n", vlan_iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	port_id = ppe_drv_iface_port_idx_get(slave_iface);

	if (port_id == -1) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("PortId is invalid for %s\n", slave_iface->dev->name);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(port_id) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, port_id)
		: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, port_id);

	/*
	 * Field for match
	 */
	if (vsi->vlan.outer_vlan == PPE_DRV_VLAN_HDR_VLAN_NOT_CONFIGURED) {
		xlt_rule.s_tagged = FAL_PORT_VLAN_XLT_MATCH_UNTAGGED;
		xlt_rule.s_vid_enable = A_FALSE;
	} else {
		xlt_rule.s_tagged = FAL_PORT_VLAN_XLT_MATCH_TAGGED;
		xlt_rule.s_vid_enable = A_TRUE;
		xlt_rule.s_vid = vsi->vlan.outer_vlan;
	}
	xlt_rule.c_tagged = FAL_PORT_VLAN_XLT_MATCH_TAGGED;
	xlt_rule.c_vid_enable = A_TRUE;
	xlt_rule.c_vid = vsi->vlan.inner_vlan;

	/*
	 * Field for action
	 */
	xlt_action.vsi_xlt_enable = A_TRUE;
	xlt_action.vsi_xlt = vsi->index;

	ret = fal_port_vlan_trans_adv_add(PPE_DRV_SWITCH_ID, fal_port, FAL_PORT_VLAN_INGRESS, &xlt_rule,
					  &xlt_action);

	if (ret != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_ingress_vlan_over_bridge_add);
		ppe_drv_warn("Add ingress rule failed for %s portid %d ret %d\n", slave_iface->dev->name, port_id, ret);
		return PPE_DRV_RET_INGRESS_VLAN_FAIL;
	}

	spin_unlock_bh(&p->lock);

	ppe_drv_trace("Add ingress rule success Outer VID %d Inner VID %d fal_port %d dev %s port_id %d\n",
		      vsi->vlan.outer_vlan, vsi->vlan.inner_vlan, fal_port, slave_iface->dev->name, port_id);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_over_bridge_add_ig_rule);

/*
 * ppe_drv_vlan_deinit()
 *	De-Initialize VLAN interfaces
 */
void ppe_drv_vlan_deinit(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *port_if;
	struct ppe_drv_vsi *vsi;
	struct ppe_drv_port *pp;

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_iface_vsi_get(iface);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Invalid VSI vlan iface", iface);
		return;
	}

	/*
	 * Detach VLAN vsi to port.
	 * Use base interface for double vlan
	 */
	port_if = ppe_drv_iface_base_get(iface);
	if (port_if->type == PPE_DRV_IFACE_TYPE_VLAN) {
		port_if = ppe_drv_iface_base_get(port_if);
	}

	pp = ppe_drv_iface_port_get(port_if);
	if (pp) {
		ppe_drv_port_vsi_detach(pp, vsi);
	}

	if (vsi) {
		ppe_drv_l3_if_deref(vsi->l3_if);
		ppe_drv_vsi_deref(vsi);
	}

	ppe_drv_iface_base_clear(iface);
	ppe_drv_iface_vsi_clear(iface);
	ppe_drv_iface_l3_if_clear(iface);
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_vlan_deinit);

/*
 * ppe_drv_vlan_init()
 *	VLAN init
 */
ppe_drv_ret_t ppe_drv_vlan_init(struct ppe_drv_iface *ppe_iface, struct net_device *base_dev, uint32_t vlan_id, bool vlan_over_bridge)
{
	struct ppe_drv_iface *base_if, *port_if;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv_vsi *vsi;
	struct ppe_drv_port *pp;

	spin_lock_bh(&p->lock);
	base_if = ppe_drv_iface_get_by_dev_internal(base_dev);
	if (!base_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: NULL base interface for iface\n", ppe_iface);
		return PPE_DRV_RET_BASE_IFACE_NOT_FOUND;
	}

	ppe_drv_iface_base_set(ppe_iface, base_if);

	vsi = ppe_drv_vsi_alloc(PPE_DRV_VSI_TYPE_VLAN);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: NULL VSI for iface\n", ppe_iface);
		return PPE_DRV_RET_VSI_ALLOC_FAIL;
	}

	if (vlan_over_bridge) {
		ppe_drv_trace("VLAN over bridge baseif dev %s iface dev %s base_dev %s vlan_id %d type %d\n",
			      base_if->dev->name, ppe_iface->dev->name, base_dev->name, vlan_id, base_if->type);
		if (!is_vlan_dev(base_dev)) {
			base_if->flags |= PPE_DRV_IFACE_VLAN_OVER_BRIDGE;
		}
	}

	ppe_drv_iface_vsi_set(ppe_iface, vsi);
	/*
	 * Set inner and outer vlan for a given VSI
	 */
	if (!ppe_drv_vsi_set_vlan(vsi, vlan_id, base_if)) {
		ppe_drv_vsi_deref(vsi);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Set vlan failed for iface\n", ppe_iface);
		return PPE_DRV_RET_VSI_ALLOC_FAIL;
	}

	l3_if = ppe_drv_vsi_l3_if_get_and_ref(vsi);
	if (!l3_if) {
		ppe_drv_vsi_deref(vsi);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: L3_IF not found for vsi(%p)\n", ppe_iface, vsi);
		return PPE_DRV_RET_FAILURE_NO_RESOURCE;
	}

	ppe_drv_iface_l3_if_set(ppe_iface, l3_if);

	/*
	 * Attach VLAN vsi to port.
	 * Use base interface for double vlan
	 */
	port_if = base_if;
	if (port_if->type == PPE_DRV_IFACE_TYPE_VLAN) {
		port_if = ppe_drv_iface_base_get(port_if);
	}

	pp = ppe_drv_iface_port_get(port_if);
	if (pp) {
		ppe_drv_port_vsi_attach(pp, vsi);
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_init);

/*
 * ppe_drv_vlan_lag_slave_leave()
 *	lag slaves leave vlan
 */
ppe_drv_ret_t ppe_drv_vlan_lag_slave_leave(struct ppe_drv_iface *vlan_iface, struct net_device *slave_dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;
	struct ppe_drv_port *port;
	struct ppe_drv_vsi *vsi;

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_iface_vsi_get(vlan_iface);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Invalid VSI for given iface\n", vlan_iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	iface = ppe_drv_iface_get_by_dev_internal(slave_dev);
	if (!iface) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px: PPE interface cannot be found for slave %s\n", vlan_iface, slave_dev->name);
		return PPE_DRV_RET_IFACE_INVALID;
	}

	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Unable to get port from iface of slave %s\n", iface, slave_dev->name);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	/*
	 * Detach slave dev to vsi.
	 * This API is needed to attach port's l3_if of slave port back so that PPE
	 * can use l3_if associated with slave port instead of bond vlan interface.
	 */
	ppe_drv_port_vsi_detach(port, vsi);
	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_lag_slave_leave);

/*
 * ppe_drv_vlan_lag_slave_join()
 *	lag slaves join vlan
 */
ppe_drv_ret_t ppe_drv_vlan_lag_slave_join(struct ppe_drv_iface *vlan_iface, struct net_device *slave_dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;
	struct ppe_drv_port *port;
	struct ppe_drv_vsi *vsi;

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_iface_vsi_get(vlan_iface);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Invalid VSI for given iface\n", vlan_iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	iface = ppe_drv_iface_get_by_dev_internal(slave_dev);
	if (!iface) {
		ppe_drv_warn("%px: PPE interface cannot be found for slave %s\n", vlan_iface, slave_dev->name);
		spin_unlock_bh(&p->lock);
		return PPE_DRV_RET_IFACE_INVALID;
	}

	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get port from iface of slave %s\n", iface, slave_dev->name);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	/*
	 * Attach slave dev to vsi.
	 * This API is needed to detach port's l3_if of slave port so that PPE
	 * can use l3_if associated with bond vlan interface.
	 */
	ppe_drv_port_vsi_attach(port, vsi);
	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_lag_slave_join);

