/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <ppe_acl.h>
#include <ppe_policer.h>
#include "ppe_policer.h"

struct ppe_policer_base gbl_ppe_policer = {0};

/*
 * ppe_policer_alloc
 *	Alloc ppe policer alloc
 */
struct ppe_policer *ppe_policer_alloc(void)
{
	/*
	 * Allocate memory for a new rule
	 *
	 * kzalloc with GFP_ATOMIC is used assuming ACL rules can be created
	 * in softirq context as well while processing an SKB in the system.
	 */
	return kzalloc(sizeof(struct ppe_policer), GFP_ATOMIC);
}

/*
 * ppe_policer_free
 *	Alloc ppe policer free
 */
void ppe_policer_free(struct ppe_policer *pol)
{
	kfree(pol);
}

/*
 * ppe_policer_rule_port_find_by_id()
 *	Find a rule corresponding to a rule ID.
 */
static struct ppe_policer *ppe_policer_rule_port_find_by_id(int16_t id)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	struct ppe_policer *pol;

	if (id >= PPE_POLICER_PORT_RULE_MAX) {
		ppe_policer_stats_inc(&g_policer->stats.port_rule_id_invalid);
		ppe_policer_warn("%p: Invalid rule ID: %d", g_policer, id);
		return NULL;
	}

	/*
	 * Get the first available policer rule ID.
	 */
	if (!list_empty(&g_policer->port_active_rules)) {
		list_for_each_entry(pol, &g_policer->port_active_rules, list) {
			int pol_id = ppe_drv_policer_get_port_id(pol->drv_ctx.port_ctx);
			if (pol_id == id) {
				ppe_policer_info("%p: POLICER rule: %p found for ID: %d", g_policer, pol, id);
				return pol;
			}
		}
	}

	ppe_policer_stats_inc(&g_policer->stats.port_rule_not_found);
	ppe_policer_warn("%p: No valid POLICER rule for ID: %d", g_policer, id);
	return NULL;
}

/*
 * ppe_policer_rule_acl_find_by_id()
 *	Find a rule corresponding to a rule ID.
 */
static struct ppe_policer *ppe_policer_rule_acl_find_by_id(int16_t id)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	struct ppe_policer *pol;

	if (id >= PPE_ACL_POLICER_FLOW_RULE_MAX) {
		ppe_policer_stats_inc(&g_policer->stats.acl_rule_id_invalid);
		ppe_policer_warn("%p: Invalid rule ID: %d", g_policer, id);
		return NULL;
	}

	if (!list_empty(&g_policer->acl_active_rules)) {
		list_for_each_entry(pol, &g_policer->acl_active_rules, list) {
			if (pol->rule_id == id) {
				ppe_policer_info("%p: POLICER rule: %p found for ID: %d", g_policer, pol, id);
				return pol;
			}
		}
	}

	ppe_policer_stats_inc(&g_policer->stats.acl_rule_not_found);
	ppe_policer_warn("%p: No valid POLICER rule for ID: %d", g_policer, id);
	return NULL;
}

/*
 * ppe_policer_acl_rule_free()
 *	Destroy ACL policer
 */
static void ppe_policer_acl_rule_free(struct kref *kref)
{
	struct ppe_policer *pol = container_of(kref, struct ppe_policer, kref_cnt);
	struct ppe_policer_base *p = &gbl_ppe_policer;

	if (ppe_acl_rule_destroy(pol->acl_rule_id) != PPE_ACL_RET_SUCCESS) {
		ppe_policer_warn("%p: failed to destroy dummy ACL: %d", p, pol->acl_rule_id);
		return;
	}

	list_del(&pol->list);

	ppe_drv_policer_acl_destroy(pol->drv_ctx.acl_ctx);

	/*
	 * free the acl rule memory.
	*/
	ppe_policer_free(pol);
	ppe_policer_stats_inc(&p->stats.destroy_acl_policer_success);
}

/*
 * ppe_policer_port_rule_free()
 *	Destroy port policer
 */
static void ppe_policer_port_rule_free(struct kref *kref)
{
	struct ppe_policer *pol = container_of(kref, struct ppe_policer, kref_cnt);
	struct ppe_policer_base *p = &gbl_ppe_policer;

	list_del(&pol->list);
	ppe_drv_port_clear_policer_support(pol->dev);

	ppe_drv_policer_port_destroy(pol->drv_ctx.port_ctx);

	/*
	 * free the acl rule memory.
	*/
	ppe_policer_free(pol);
	ppe_policer_stats_inc(&p->stats.destroy_port_policer_success);
}

/*
 * ppe_policer_port_destroy()
 *	Port destroy
 */
static bool ppe_policer_port_destroy(struct ppe_policer_destroy_info *destroy)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	struct ppe_policer *pol;
	struct net_device *dev;
	int32_t port_id;

	dev = dev_get_by_name(&init_net, destroy->name);
	if (!dev) {
		ppe_policer_warn("%p: failed to find valid src for dev\n", destroy);
		return false;
	}

	if (!ppe_drv_port_check_policer_support(dev)) {
		ppe_policer_warn("%p: Policer not configured for dev(%p)\n", destroy, destroy->name);
		return false;
	}

	port_id = ppe_drv_port_num_from_dev(dev);
	if (port_id <= -1) {
		ppe_policer_warn("%p: Invalid port_id for dev(%s)\n", destroy, destroy->name);
		return false;
	}

	pol = ppe_policer_rule_port_find_by_id(port_id);
	if (!pol) {
		ppe_policer_stats_inc(&g_policer->stats.policer_destroy_fail_invalid_id);
		ppe_policer_warn("%p: failed to find the acl rule for ID", g_policer);
		return false;
	}

	if (kref_put(&pol->kref_cnt, ppe_policer_port_rule_free)) {
		ppe_policer_trace("%p: reference goes down to 0 for policer: %p\n", g_policer, pol);
	}

	return true;
}

/*
 * ppe_policer_acl_destroy()
 *	ACL destroy
 */
static bool ppe_policer_acl_destroy(struct ppe_policer_destroy_info *destroy)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	struct ppe_policer *pol = ppe_policer_rule_acl_find_by_id(destroy->rule_id);
	if (!pol) {
		ppe_policer_stats_inc(&g_policer->stats.policer_destroy_fail_invalid_id);
		ppe_policer_warn("%p: failed to find the acl rule for ID", g_policer);
		return false;
	}

	if (kref_put(&pol->kref_cnt, ppe_policer_acl_rule_free)) {
		ppe_policer_trace("%p: reference goes down to 0 for policer: %p\n", g_policer, pol);
	}

	ppe_policer_info("%p: ref dec: %u", g_policer, kref_read(&pol->kref_cnt));
	return true;
}

/*
 * ppe_policer_destroy()
 * 	Destroy IPv4 PPE POLICER rule
 */
ppe_policer_ret_t ppe_policer_destroy(struct ppe_policer_destroy_info *destroy)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;

	/* Port Policer */
	if (destroy->policer_type == PPE_POLICER_TYPE_PORT) {
		spin_lock_bh(&g_policer->lock);
		if (!ppe_policer_port_destroy(destroy)) {
			spin_unlock_bh(&g_policer->lock);
			return PPE_POLICER_PORT_RET_DESTROY_FAIL;
		}

		spin_unlock_bh(&g_policer->lock);
		return PPE_POLICER_SUCCESS;
	}

	spin_lock_bh(&g_policer->lock);
	if (!ppe_policer_acl_destroy(destroy)) {
		spin_unlock_bh(&g_policer->lock);
		return PPE_POLICER_PORT_RET_DESTROY_FAIL;
	}

	spin_unlock_bh(&g_policer->lock);
	return PPE_POLICER_SUCCESS;
}
EXPORT_SYMBOL(ppe_policer_destroy);

/*
 * ppe_policer_create_port()
 *	create port policer
 */
static bool ppe_policer_create_port(struct ppe_policer_create_info *info)
{
	struct ppe_policer_config *cinfo = &info->config;
	struct ppe_drv_policer_rule_create create = {0};
	struct ppe_drv_policer_rule_create_port_info *port_info = &create.msg.port_info;
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	struct ppe_policer *pol;
	struct net_device *dev;

	dev = dev_get_by_name(&init_net, info->name);
	if (!dev) {
		ppe_policer_warn("%p: failed to find valid src for dev\n", info);
		return false;
	}

	if (ppe_drv_port_check_policer_support(dev)) {
		ppe_policer_warn("%p: Policer already configured for dev(%s)\n", info, info->name);
		dev_put(dev);
		return false;
	}

	pol = ppe_policer_alloc();
	if (!pol) {
		ppe_policer_stats_inc(&g_policer->stats.acl_create_fail_oom);
		ppe_policer_warn("%p: failed to allocate acl memory: %p", g_policer, info);
		dev_put(dev);
		return PPE_POLICER_PORT_RET_CREATE_FAIL_OOM;
	}

	port_info->dev = dev;
	port_info->meter_en = cinfo->meter_enable;
	port_info->colour_mode = cinfo->colour_aware;
	port_info->coupling_flag = cinfo->couple_enable;
	port_info->meter_mode = cinfo->mode;
	port_info->meter_unit = cinfo->meter_unit;
	port_info->cbs = cinfo->committed_burst_size;
	port_info->cir = cinfo->committed_rate;
	port_info->ebs = cinfo->peak_burst_size;
	port_info->eir = cinfo->peak_rate;

	memset(&port_info->action, 0, sizeof(struct ppe_drv_policer_rule_create_action));

	if (cinfo->action_info.yellow_pri) {
		port_info->action.exceed_chg_pri = true;
		port_info->action.exceed_pri = cinfo->action_info.yellow_pri;
	}

	if (cinfo->action_info.yellow_dp) {
		port_info->action.exceed_chg_dp = true;
		port_info->action.exceed_dp = cinfo->action_info.yellow_dp;
	}

	if (cinfo->action_info.yellow_pcp) {
		port_info->action.exceed_chg_pcp = true;
		port_info->action.exceed_pcp = cinfo->action_info.yellow_pcp;
	}

	if (cinfo->action_info.yellow_dei) {
		port_info->action.exceed_chg_dei = true;
		port_info->action.exceed_dei = cinfo->action_info.yellow_dei;
	}

	port_info->action.red_drop = true;

	pol->drv_ctx.port_ctx = ppe_drv_policer_port_create(&create);
	if (!pol->drv_ctx.port_ctx) {
		ppe_policer_stats_inc(&g_policer->stats.create_port_policer_failed);
		ppe_policer_warn("Unable to create port policer in HW for dev\n");
		dev_put(dev);
		ppe_policer_free(pol);
		return false;
	}

	list_add(&pol->list, &g_policer->port_active_rules);
	info->ret = PPE_POLICER_SUCCESS;
	ppe_policer_stats_inc(&g_policer->stats.policer_port_create_req);
	pol->dev = dev;

	kref_init(&pol->kref_cnt);

	ppe_drv_port_set_policer_support(dev);
	dev_put(dev);

	ppe_policer_stats_inc(&g_policer->stats.create_port_policer_success);
	ppe_policer_trace("Created PORT policer successfully\n");
	return true;
}

/*
 * ppe_policer_create_acl()
 *	Create acl policer
 */
static bool ppe_policer_create_acl(struct ppe_policer_create_info *info)
{
	struct ppe_policer_config *cinfo = &info->config;
	struct ppe_drv_policer_rule_create create = {0};
	struct ppe_drv_policer_rule_create_acl_info *acl_info = &create.msg.acl_info;
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	struct ppe_policer *pol;

	pol = ppe_policer_rule_acl_find_by_id(info->rule_id);
	if (pol) {
		ppe_policer_stats_inc(&g_policer->stats.policer_acl_already_exists);
		ppe_policer_warn("%p: Policer index already configured: %d", g_policer, info->rule_id);
		return false;
	}

	pol = ppe_policer_alloc();
	if (!pol) {
		ppe_policer_stats_inc(&g_policer->stats.port_create_fail_oom);
		ppe_policer_warn("%p: failed to allocate acl memory for policer: %p", g_policer, info);
		return false;
	}

	/* ACL policer */
	pol->rule_id = info->rule_id;

	acl_info->meter_en = cinfo->meter_enable;
	acl_info->colour_mode = cinfo->colour_aware;
	acl_info->coupling_flag = cinfo->couple_enable;
	acl_info->meter_mode = cinfo->mode;
	acl_info->meter_unit = cinfo->meter_unit;
	acl_info->cbs = cinfo->committed_burst_size;
	acl_info->cir = cinfo->committed_rate;
	acl_info->ebs = cinfo->peak_burst_size;
	acl_info->eir = cinfo->peak_rate;

	memset(&acl_info->action, 0, sizeof(struct ppe_drv_policer_rule_create_action));

	if (cinfo->action_info.yellow_pri) {
		acl_info->action.exceed_chg_pri = true;
		acl_info->action.exceed_pri = cinfo->action_info.yellow_pri;
	}

	if (cinfo->action_info.yellow_dp) {
		acl_info->action.exceed_chg_dp = true;
		acl_info->action.exceed_dp = cinfo->action_info.yellow_dp;
	}

	if (cinfo->action_info.yellow_pcp) {
		acl_info->action.exceed_chg_pcp = true;
		acl_info->action.exceed_pcp = cinfo->action_info.yellow_pcp;
	}

	if (cinfo->action_info.yellow_dei) {
		acl_info->action.exceed_chg_dei = true;
		acl_info->action.exceed_dei = cinfo->action_info.yellow_dei;
	}

	if (cinfo->action_info.yellow_dscp) {
		acl_info->action.exceed_chg_dscp = true;
		acl_info->action.exceed_dscp = cinfo->action_info.yellow_dscp;
	}

	acl_info->action.red_drop = true;

	pol->drv_ctx.acl_ctx = ppe_drv_policer_acl_create(&create);
	if (!pol->drv_ctx.acl_ctx) {
		ppe_policer_stats_inc(&g_policer->stats.create_acl_policer_failed);
		ppe_policer_warn("Unable to create acl policer in HW for dev\n");
		return false;
	}

	ppe_drv_policer_user2hw_id_map(pol->drv_ctx.acl_ctx, info->rule_id);

	list_add(&pol->list, &g_policer->acl_active_rules);

	info->ret = PPE_POLICER_SUCCESS;
	ppe_policer_stats_inc(&g_policer->stats.policer_acl_create_req);
	ppe_policer_warn("%p: ACL policer rule created with rule ID: %d", g_policer, pol->rule_id);

	kref_init(&pol->kref_cnt);

	ppe_policer_stats_inc(&g_policer->stats.create_acl_policer_success);
	return true;
}

/*
 * ppe_policer_id_to_hwidx()
 *	Policer to HW index
 */
int ppe_policer_id_to_hwidx(uint16_t policer_id)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	struct ppe_policer *pol;

	spin_lock_bh(&g_policer->lock);
	pol = ppe_policer_rule_acl_find_by_id(policer_id);
	if (!pol) {
		ppe_policer_stats_inc(&g_policer->stats.policer_destroy_fail_invalid_id);
		ppe_policer_warn("%p: failed to find the acl rule for ID", g_policer);
		spin_unlock_bh(&g_policer->lock);
		return PPE_POLICER_ACL_RET_DESTROY_FAIL_INVALID_ID;

	}

	spin_unlock_bh(&g_policer->lock);

	return ppe_drv_policer_get_policer_id(pol->drv_ctx.acl_ctx);
}
EXPORT_SYMBOL(ppe_policer_id_to_hwidx);

/*
 * ppe_policer_create()
 * 	Create PPE POlicer rule
 */
ppe_policer_ret_t ppe_policer_create(struct ppe_policer_create_info *create)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;

	/*
	 * Port Policer
	 */
	if (create->policer_type == PPE_POLICER_TYPE_PORT) {
		spin_lock_bh(&g_policer->lock);
		if (!ppe_policer_create_port(create)) {
			spin_unlock_bh(&g_policer->lock);
			ppe_policer_warn("Unable to create port policer\n");
			return PPE_POLICER_PORT_RET_CREATE_FAIL;
		}

		spin_unlock_bh(&g_policer->lock);
		return PPE_POLICER_SUCCESS;
	}

	/*
	 * ACL Policer
	 */
	spin_lock_bh(&g_policer->lock);
	if (!ppe_policer_create_acl(create)) {
		spin_unlock_bh(&g_policer->lock);
		ppe_policer_warn("Unable to create ACL policer\n");
		return PPE_POLICER_ACL_RET_CREATE_FAIL;
	}

	spin_unlock_bh(&g_policer->lock);
	return PPE_POLICER_ACL_RET_CREATE_SUCCESS;
}
EXPORT_SYMBOL(ppe_policer_create);

#ifdef NSS_PPE_RULE_IPQ53XX
bool ppe_policer_rule_flow_add_cb(void *app_data, struct ppe_drv_policer_flow *info)
{
	return true;
}

bool ppe_policer_rule_flow_del_cb(void *app_data, struct ppe_drv_policer_flow *info)
{
	return true;
}

#else
/*
 * ppe_policer_rule_flow_del_cb()
 *	 Flow delete callback.
 */
bool ppe_policer_rule_flow_del_cb(void *app_data, struct ppe_drv_policer_flow *info)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	struct ppe_policer *pol;

	/*
	 * Find the matching rule corresponding to the rule ID.
	 */
	spin_lock_bh(&g_policer->lock);
	pol = ppe_policer_rule_acl_find_by_id(info->id);
	if (!pol) {
		ppe_policer_warn("%p: failed to find the rule for ID: %d", g_policer, info->id);
		spin_unlock_bh(&g_policer->lock);
		return false;
	}

	/*
	 * Dereference: during flow_del_cb()
	 * Reference: during flow_add_cb()
	 */
	if (kref_put(&pol->kref_cnt, ppe_policer_acl_rule_free)) {
		ppe_policer_warn("%p: reference goes down to 0 for pol: %p ID: %d\n",
				g_policer, pol, info->id);
	}

	ppe_policer_trace("%p: rule_id: %u ref dec: %u", g_policer, info->id, kref_read(&pol->kref_cnt));
	spin_unlock_bh(&g_policer->lock);
	return true;
}

/*
 * ppe_policer_rule_flow_add_cb()
 *	 Return a service code corresponding to rule ID for n-tuple match.
 */
bool ppe_policer_rule_flow_add_cb(void *app_data, struct ppe_drv_policer_flow *info)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	struct ppe_acl_rule_flow_policer flow_rule = {0};
	struct ppe_policer *pol;

	spin_lock_bh(&g_policer->lock);
	pol = ppe_policer_rule_acl_find_by_id(info->id);
	if (!pol) {
		ppe_policer_warn("%p: failed to find the rule for ID: %d", g_policer, info->id);
		spin_unlock_bh(&g_policer->lock);
		return false;
	}

	flow_rule.hw_policer_idx = ppe_drv_policer_get_policer_id(pol->drv_ctx.acl_ctx);
	flow_rule.pkt_noedit = info->pkt_noedit;

	/*
	 * Find the matching rule corresponding to the rule ID.
	 */
	if (ppe_acl_rule_flow_policer_create(&flow_rule) != PPE_ACL_RET_SUCCESS) {
		ppe_policer_warn("%p: failed to create dummy ACL: %d", g_policer, info->id);
		spin_unlock_bh(&g_policer->lock);
		return false;
	}

	info->sc_valid = true;
	info->sc = flow_rule.sc;

	/*
	 * Fill acl rule id for deletion
	 */
	pol->acl_rule_id = flow_rule.rule_id;

	/*
	 * Reference: during flow_add_cb()
	 * Dereference: during flow_del_cb()
	 */
	kref_get(&pol->kref_cnt);

	ppe_policer_trace("%p: rule_id: %u ref inc: %u acl_id: %d", g_policer, info->id, kref_read(&pol->kref_cnt), pol->rule_id);
	spin_unlock_bh(&g_policer->lock);
	return true;
}
#endif

/*
 * ppe_policer_v6_noedit_flow_create()
 *	Create Policer v6 rule in PPE
 */
ppe_policer_ret_t ppe_policer_v6_noedit_flow_create(struct ppe_drv_v6_rule_create *create)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	ppe_drv_ret_t ret;

	ppe_policer_stats_inc(&g_policer->stats.v6_create_ppe_rule_flow_policer);

	ret = ppe_drv_v6_policer_flow_create(create);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_policer_warn("%p: Error in pushing Passive PPE RFS rules\n", create);
		ppe_policer_stats_inc(&g_policer->stats.v6_create_ppe_rule_fail);
		return PPE_POLICER_CREATE_V6_RULE_FAILURE;
	}

	return PPE_POLICER_SUCCESS;
}
EXPORT_SYMBOL(ppe_policer_v6_noedit_flow_create);

/*
 * ppe_policer_v6_noedit_flow_destroy()
 *	Destroy Policer v6 rule in PPE
 */
ppe_policer_ret_t ppe_policer_v6_noedit_flow_destroy(struct ppe_drv_v6_rule_destroy *destroy)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;

	if (ppe_drv_v6_policer_flow_destroy(destroy) != PPE_DRV_RET_SUCCESS) {
		ppe_policer_warn("%p: error in pushing Passive PPE policer rules\n", destroy);
		ppe_policer_stats_inc(&g_policer->stats.v6_destroy_ppe_rule_fail);
		return PPE_POLICER_DESTROY_V6_RULE_FAILURE;
	}

	return PPE_POLICER_SUCCESS;
}
EXPORT_SYMBOL(ppe_policer_v6_noedit_flow_destroy);

/*
 * ppe_policer_v4_noedit_flow_create()
 *	Create Policer v4 rule in PPE
 */
ppe_policer_ret_t ppe_policer_v4_noedit_flow_create(struct ppe_drv_v4_rule_create *create)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;
	ppe_drv_ret_t ret;

	ppe_policer_stats_inc(&g_policer->stats.v4_create_ppe_rule_flow_policer);

	ret = ppe_drv_v4_policer_flow_create(create);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_policer_warn("%p: Error in pushing Passive PPE RFS rules\n", create);
		ppe_policer_stats_inc(&g_policer->stats.v4_create_ppe_rule_fail);
		return PPE_POLICER_CREATE_V4_RULE_FAILURE;
	}

	return PPE_POLICER_SUCCESS;
}
EXPORT_SYMBOL(ppe_policer_v4_noedit_flow_create);

/*
 * ppe_policer_v4_noedit_flow_destroy()
 *	Destroy Policer v4 rule in PPE
 */
ppe_policer_ret_t ppe_policer_v4_noedit_flow_destroy(struct ppe_drv_v4_rule_destroy *destroy)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;

	if (ppe_drv_v4_policer_flow_destroy(destroy) != PPE_DRV_RET_SUCCESS) {
		ppe_policer_warn("%p: error in pushing Passive PPE policer rules\n", destroy);
		ppe_policer_stats_inc(&g_policer->stats.v4_destroy_ppe_rule_fail);
		return PPE_POLICER_DESTROY_V4_RULE_FAILURE;
	}

	return PPE_POLICER_SUCCESS;
}
EXPORT_SYMBOL(ppe_policer_v4_noedit_flow_destroy);

/*
 * ppe_policer_deinit()
 *	Policer deinit API
 */
void ppe_policer_deinit(void)
{
	ppe_policer_stats_debugfs_exit();
	ppe_drv_policer_flow_unregister_cb();
}

/*
 * ppe_policer_init()
 *	policer init API
 */
void ppe_policer_init(struct dentry *d_rule)
{
	struct ppe_policer_base *g_policer = &gbl_ppe_policer;

	spin_lock_init(&g_policer->lock);
	INIT_LIST_HEAD(&g_policer->port_active_rules);
	INIT_LIST_HEAD(&g_policer->acl_active_rules);

	ppe_drv_policer_flow_register_cb(ppe_policer_rule_flow_add_cb, ppe_policer_rule_flow_del_cb, NULL);
	ppe_policer_stats_debugfs_init(d_rule);
}
