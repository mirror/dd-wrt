/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <ppe_drv.h>
#include <ppe_drv_v4.h>
#include <ppe_drv_v6.h>
#include <ppe_priority.h>
#include "ppe_priority.h"

struct ppe_priority gbl_ppe_priority;

/*
 * ppe_priority_ipv6_rule_destroy()
 * 	Destroy IPv6 PPE priority rule
 */
enum ppe_priority_ret ppe_priority_ipv6_rule_destroy(struct ppe_priority_ipv6_rule_destroy_msg *destroy_ipv6)
{
	struct ppe_priority *p = &gbl_ppe_priority;
	struct ppe_drv_v6_rule_destroy pd6rd = {0};

	ppe_priority_stats_inc(&p->stats.v6_destroy_ppe_rule_priority);

	memcpy(pd6rd.tuple.flow_ip, destroy_ipv6->tuple.flow_ip, sizeof(destroy_ipv6->tuple.flow_ip));
	pd6rd.tuple.flow_ident = destroy_ipv6->tuple.flow_ident;
	memcpy(pd6rd.tuple.return_ip, destroy_ipv6->tuple.return_ip, sizeof(destroy_ipv6->tuple.return_ip));
	pd6rd.tuple.return_ident = destroy_ipv6->tuple.return_ident;
	pd6rd.tuple.protocol = destroy_ipv6->tuple.protocol;

	if (ppe_drv_v6_assist_rule_destroy(&pd6rd) != PPE_DRV_RET_SUCCESS) {
		ppe_priority_warn("%p: error in pushing Passive PPE priority rules\n", destroy_ipv6);
		ppe_priority_stats_inc(&p->stats.v6_destroy_ppe_rule_priority_fail);
		return PPE_PRIORITY_RET_FAILURE;
	}

	return PPE_PRIORITY_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_priority_ipv6_rule_destroy);

/*
 * ppe_priority_ipv6_rule_create()
 * 	Create IPv6 PPE Priority rule
 */
enum ppe_priority_ret ppe_priority_ipv6_rule_create(struct ppe_priority_ipv6_rule_create_msg *create_ipv6)
{
	struct ppe_drv_v6_rule_create pd6rc = {0};
	struct ppe_priority *p = &gbl_ppe_priority;
	ppe_drv_ret_t ret;

	ppe_priority_stats_inc(&p->stats.v6_create_ppe_rule_priority);

	memcpy(pd6rc.tuple.flow_ip, create_ipv6->tuple.flow_ip, sizeof(create_ipv6->tuple.flow_ip));
	pd6rc.tuple.flow_ident = create_ipv6->tuple.flow_ident;
	memcpy(pd6rc.tuple.return_ip, create_ipv6->tuple.return_ip, sizeof(create_ipv6->tuple.return_ip));
	pd6rc.tuple.return_ident = create_ipv6->tuple.return_ident;
	pd6rc.tuple.protocol = create_ipv6->tuple.protocol;
	pd6rc.conn_rule.flow_mtu = create_ipv6->flow_mtu;
	if (create_ipv6->priority > PPE_DRV_INT_PRI_MAX && create_ipv6->priority < PPE_DRV_INT_PRI_MIN) {
		ppe_priority_warn("incorrect priority value to be set %d [valid priority value [0-15]\n", create_ipv6->priority);
		return PPE_PRIORITY_RET_FAILURE;
	}
	pd6rc.qos_rule.flow_qos_tag = create_ipv6->priority;

	ret = ppe_drv_v6_assist_rule_create(&pd6rc, PPE_DRV_ASSIST_FEATURE_PRIORITY);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_priority_warn("%p: Error in pushing  rules\n", create_ipv6);
		ppe_priority_stats_inc(&p->stats.v6_create_ppe_rule_priority_fail);
		return PPE_PRIORITY_RET_FAILURE;
	}

	return PPE_PRIORITY_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_priority_ipv6_rule_create);

/*
 * ppe_priority_ipv4_rule_destroy()
 * 	Destroy IPv4 PPE PRIORITY rule
 */
enum ppe_priority_ret ppe_priority_ipv4_rule_destroy(struct ppe_priority_ipv4_rule_destroy_msg *destroy_ipv4)
{
	struct ppe_priority *p = &gbl_ppe_priority;
	struct ppe_drv_v4_rule_destroy pd4rd = {0};

	ppe_priority_stats_inc(&p->stats.v4_destroy_ppe_rule_priority);

	pd4rd.tuple.flow_ip = destroy_ipv4->tuple.flow_ip;
	pd4rd.tuple.flow_ident = destroy_ipv4->tuple.flow_ident;
	pd4rd.tuple.return_ip = destroy_ipv4->tuple.return_ip;
	pd4rd.tuple.return_ident = destroy_ipv4->tuple.return_ident;
	pd4rd.tuple.protocol = destroy_ipv4->tuple.protocol;

	if (ppe_drv_v4_assist_rule_destroy(&pd4rd) != PPE_DRV_RET_SUCCESS) {
		ppe_priority_warn("%p: Error deleting flow rule from PPE\n", destroy_ipv4);
		ppe_priority_stats_inc(&p->stats.v4_destroy_ppe_rule_priority_fail);
		return PPE_PRIORITY_RET_FAILURE;
	}

	return PPE_PRIORITY_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_priority_ipv4_rule_destroy);

/*
 * ppe_priority_ipv4_rule_create()
 * 	Create IPv4 PPE priority rule
 */
enum ppe_priority_ret ppe_priority_ipv4_rule_create(struct ppe_priority_ipv4_rule_create_msg *create_ipv4)
{
	struct ppe_drv_v4_rule_create pd4rc = {0};
	struct ppe_priority *p = &gbl_ppe_priority;
	ppe_drv_ret_t ret;

	ppe_priority_stats_inc(&p->stats.v4_create_ppe_rule_priority);

	pd4rc.tuple.flow_ip = create_ipv4->tuple.flow_ip;
	pd4rc.tuple.flow_ident = create_ipv4->tuple.flow_ident;
	pd4rc.tuple.return_ip = create_ipv4->tuple.return_ip;
	pd4rc.tuple.return_ident = create_ipv4->tuple.return_ident;
	pd4rc.tuple.protocol = create_ipv4->tuple.protocol;
	pd4rc.conn_rule.flow_mtu = create_ipv4->flow_mtu;
	if (create_ipv4->priority > PPE_DRV_INT_PRI_MAX && create_ipv4->priority < PPE_DRV_INT_PRI_MIN) {
		ppe_priority_warn("incorrect priority value to be set %d [valid priority value [0-15]\n", create_ipv4->priority);
		return PPE_PRIORITY_RET_FAILURE;
	}

	pd4rc.qos_rule.flow_qos_tag = create_ipv4->priority;

	ret = ppe_drv_v4_assist_rule_create(&pd4rc, PPE_DRV_ASSIST_FEATURE_PRIORITY);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_priority_warn("%p: Error in pushing rule\n", create_ipv4);
		ppe_priority_stats_inc(&p->stats.v4_create_ppe_rule_priority_fail);
		return PPE_PRIORITY_RET_FAILURE;
	}

	return PPE_PRIORITY_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_priority_ipv4_rule_create);

/*
 * ppe_priority_deinit()
 *	priority  deinit API
 */
void ppe_priority_deinit(void)
{
	ppe_priority_stats_debugfs_exit();
}

/*
 * ppe_priority_init()
 *	priority init API
 */
void ppe_priority_init(struct dentry *d_rule)
{
	struct ppe_priority *g_prio = &gbl_ppe_priority;
	spin_lock_init(&g_prio->lock);
	ppe_priority_stats_debugfs_init(d_rule);
}

