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

#include <linux/vmalloc.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/etherdevice.h>
#include "ppe_drv.h"

/*
 * ppe_drv_cc_process_v4()
 *	Try flush for a given flow key.
 */
static void ppe_drv_cc_process_v4(ppe_drv_cc_t cc, struct flow_keys *keys)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_flow *flow;
	struct ppe_drv_v4_conn_flow *pcf;
	struct ppe_drv_v4_conn_flow *pcr;
	struct ppe_drv_v4_5tuple v4_5tuple = {0};
	struct ppe_drv_v4_conn_sync *cns;
	struct ppe_drv_v4_conn *cn = NULL;

	v4_5tuple.flow_ip = ntohl(keys->addrs.v4addrs.src);
	v4_5tuple.return_ip = ntohl(keys->addrs.v4addrs.dst);
	v4_5tuple.protocol = keys->basic.ip_proto;

	/*
	 * PPE support only TCP, UDP or UDP_LITE for 5-tuple
	 */
	if ((keys->basic.ip_proto == IPPROTO_TCP)
			|| (keys->basic.ip_proto == IPPROTO_UDP)
			|| (keys->basic.ip_proto == IPPROTO_UDPLITE)) {

		v4_5tuple.flow_ident = ntohs(keys->ports.src);
		v4_5tuple.return_ident = ntohs(keys->ports.dst);
	}

	/*
	 * Get the flow table entry and try to destroy.
	 */
	spin_lock_bh(&p->lock);
	flow = ppe_drv_flow_v4_get(&v4_5tuple);
	if (!flow) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_flush_conn_not_found);
		ppe_drv_info("%p: flow entry not found for cpu_code: %d", keys, cc);
		return;
	}

	pcf = flow->pcf.v4;
	cn = pcf->conn;
	pcr = (pcf == &cn->pcf) ? &cn->pcr : &cn->pcf;

	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_FLOW_RFS_PPE_ASSIST)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_flush_skip_conn_rfs);
		ppe_drv_info("%p: connection flush called for ppe_rfs cpu_code: %d", keys, cc);
		return;
	}

	if (ppe_drv_v4_conn_flow_flags_check(pcr, PPE_DRV_V4_CONN_FLAG_FLOW_RFS_PPE_ASSIST)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_flush_skip_conn_rfs);
		ppe_drv_info("%p: connection flush called for ppe_rfs cpu_code: %d", keys, cc);
		return;
	}

	if (ppe_drv_v4_flush(pcf->conn) != PPE_DRV_RET_SUCCESS) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_flush_conn_not_found);
		ppe_drv_info("%p: connection flush failed for cpu_code: %d", keys, cc);
		return;
	}

	/*
	 * Capture remaining stats.
	 */
	cns = ppe_drv_v4_conn_stats_alloc();
	if (cns) {
		ppe_drv_v4_conn_sync_one(ppe_drv_v4_conn_flow_conn_get(pcf), cns, PPE_DRV_STATS_SYNC_REASON_FLUSH);
	}

	spin_unlock_bh(&p->lock);

	/*
	 * Sync stats with CM
	 */
	if (cns) {
		ppe_drv_v4_conn_stats_sync_invoke_cb(cns);
		ppe_drv_v4_conn_stats_free(cns);
	}

	ppe_drv_v4_conn_free(ppe_drv_v4_conn_flow_conn_get(pcf));
}

/*
 * ppe_drv_cc_process_v6()
 *	Try flush for a given flow key.
 */
static void ppe_drv_cc_process_v6(ppe_drv_cc_t cc, struct flow_keys *keys)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_flow *flow;
	struct ppe_drv_v6_conn_flow *pcf;
	struct ppe_drv_v6_conn_flow *pcr;
	struct ppe_drv_v6_5tuple v6_5tuple = {0};
	struct ppe_drv_v6_conn_sync *cns;
	struct ppe_drv_v6_conn *cn = NULL;

	PPE_DRV_IN6_TO_IPV6(v6_5tuple.flow_ip, keys->addrs.v6addrs.src);
	PPE_DRV_IN6_TO_IPV6(v6_5tuple.return_ip, keys->addrs.v6addrs.dst);
	v6_5tuple.protocol = keys->basic.ip_proto;

	/*
	 * PPE support only TCP, UDP or UDP_LITE
	 */
	if ((keys->basic.ip_proto == IPPROTO_TCP)
			|| (keys->basic.ip_proto == IPPROTO_UDP)
			|| (keys->basic.ip_proto == IPPROTO_UDPLITE)) {

		v6_5tuple.flow_ident = ntohs(keys->ports.src);
		v6_5tuple.return_ident = ntohs(keys->ports.dst);
	}

	/*
	 * Get the flow table entry and try to destroy.
	 */
	spin_lock_bh(&p->lock);
	flow = ppe_drv_flow_v6_get(&v6_5tuple);
	if (!flow) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.v6_flush_conn_not_found);
		ppe_drv_info("%p: flow entry not found for cpu_code: %d", keys, cc);
		return;
	}

	pcf = flow->pcf.v6;
	cn = pcf->conn;
	pcr = (pcf == &cn->pcf) ? &cn->pcr : &cn->pcf;

	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_FLOW_RFS_PPE_ASSIST)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.v6_flush_skip_conn_rfs);
		ppe_drv_info("%p: connection flush called for ppe_rfs cpu_code: %d", keys, cc);
		return;
	}

	if (ppe_drv_v6_conn_flow_flags_check(pcr, PPE_DRV_V6_CONN_FLAG_FLOW_RFS_PPE_ASSIST)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.v6_flush_skip_conn_rfs);
		ppe_drv_info("%p: connection flush called for ppe_rfs cpu_code: %d", keys, cc);
		return;
	}

	if (ppe_drv_v6_flush(pcf->conn) != PPE_DRV_RET_SUCCESS) {
		spin_unlock_bh(&p->lock);
		ppe_drv_info("%p: connection flush failed for cpu_code: %d", keys, cc);
		return;
	}

	/*
	 * Capture remaining stats.
	 */
	cns = ppe_drv_v6_conn_stats_alloc();
	if (cns) {
		ppe_drv_v6_conn_sync_one(ppe_drv_v6_conn_flow_conn_get(pcf), cns, PPE_DRV_STATS_SYNC_REASON_FLUSH);
	}

	spin_unlock_bh(&p->lock);

	/*
	 * Sync stats with CM
	 */
	if (cns) {
		ppe_drv_v6_conn_stats_sync_invoke_cb(cns);
		ppe_drv_v6_conn_stats_free(cns);
	}

	ppe_drv_v6_conn_free(ppe_drv_v6_conn_flow_conn_get(pcf));
}

/*
 * ppe_drv_cc_process_skbuff()
 *	Process skbuff with a non-zero cpu code.
 */
bool ppe_drv_cc_process_skbuff(struct ppe_drv_cc_metadata *cc_info, struct sk_buff *skb)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_cc *pcc;
	struct flow_keys keys = {0};
	ppe_drv_cc_callback_t cb;
	void *app_data;
	bool ret = false;
	uint16_t cc = cc_info->cpu_code;
	struct iphdr *iph;

	ppe_drv_assert((cc > 0) && (cc < PPE_DRV_CC_MAX), "%p: invalid cpu code %u", p, cc);

	/*
	 * Check if this CPU code needs flush.
	 */
	pcc = &p->cc[cc];
	if (!pcc->flush) {
		goto done;
	}

	/*
	 * Extract flow key from skbuff
	 * We are using skb flow dissect and ignoring its performance impact,
	 * since PPE generate explicit CPU code only for first packet which is
	 * responsible for auto flow deceleration in PPE, all subsequent packets
	 * corresponding to decelerated flow would be sent to host without a
	 * specific CPU code which we are monitoring.
	 *
	 * Note:
	 * 1. we need to parse the 1st fragment of an IP fragment chain to flush
	 *    associated rule.
	 * 2. PPE uses 3 tuple rule for all ip protocol other than TCP, UDP & UDP_LITE,
	 *    so we STOP_AT_ENCAP.
	 * 3. For packets with fake MAC header PPE can generate a flow based exception,
	 *    only when it's an IPv4 or IPv6 packets.
	 */
	if (cc_info->fake_mac) {
		/*
		 * Discard L2 header
		 */
		skb_pull_inline(skb, ETH_HLEN);
		iph = (struct iphdr *)skb->data;

		if (iph->version == 4) {
			skb->protocol = htons(ETH_P_IP);
		} else if (iph->version == 6) {
			skb->protocol = htons(ETH_P_IPV6);
		} else {
			ppe_drv_info("%p: Non-IP packet with fake mac set :%p for cc:%u ",
					p, skb, cc);
			goto push;
		}
	} else {
		skb->protocol = eth_type_trans(skb, skb->dev);
	}

	skb_reset_network_header(skb);
	if (!skb_flow_dissect_flow_keys(skb, &keys,
			FLOW_DISSECTOR_F_PARSE_1ST_FRAG | FLOW_DISSECTOR_F_STOP_AT_ENCAP)) {
		ppe_drv_info("%p: dissection failed for skb:%p", p, skb);
		goto push;
	}

	/*
	 * Attempt flush
	 */
	switch (keys.control.addr_type) {
		case FLOW_DISSECTOR_KEY_IPV4_ADDRS:
			ppe_drv_cc_process_v4((ppe_drv_cc_t)cc, &keys);
			break;

		case FLOW_DISSECTOR_KEY_IPV6_ADDRS:
			ppe_drv_cc_process_v6((ppe_drv_cc_t)cc, &keys);
			break;

		default:
			ppe_drv_info("%p: dissection failed for skb:%p", p, skb);
	}
push:
	skb_push(skb, ETH_HLEN);
done:
	spin_lock_bh(&p->lock);
	cb = pcc->cb;
	app_data = pcc->app_data;
	spin_unlock_bh(&p->lock);

	ppe_drv_trace("%p: processing skb:%p cc:%u cb:%p app:%p",
			p, skb, cc, cb, app_data);


	if (cb) {
		ret = cb(app_data, skb, (void *)cc_info);
	}

	return ret;
}
EXPORT_SYMBOL(ppe_drv_cc_process_skbuff);

/*
 * ppe_drv_cc_unregister_cb()
 *	Unregister callback for a give service code
 */
void ppe_drv_cc_unregister_cb(ppe_drv_cc_t cc)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_cc *pcc;

	spin_lock_bh(&p->lock);
	pcc = &p->cc[cc];
	ppe_drv_assert(pcc->cb, "%p: no cb registered for cc: %u", p, cc);
	pcc->cb = NULL;
	pcc->app_data = NULL;
	spin_unlock_bh(&p->lock);

	ppe_drv_info("%p: unregistered cb/app_data for cc:%u", p, cc);
}
EXPORT_SYMBOL(ppe_drv_cc_unregister_cb);

/*
 * ppe_drv_cc_register_cb()
 *	Registers a callback for a given cpu code.
 */
void ppe_drv_cc_register_cb(ppe_drv_cc_t cc, ppe_drv_cc_callback_t cb, void *app_data)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_cc *pcc;

	ppe_drv_assert(cb, "%p: cannot register null cb for cc %u", p, cc);

	spin_lock_bh(&p->lock);
	pcc = &p->cc[cc];
	ppe_drv_assert(!pcc->cb, "%p: multiple registration for cc:%u - "
				"prev cb:%p current cb:%p", p, cc, pcc->cb, cb);
	pcc->cb = cb;
	pcc->app_data = app_data;
	spin_unlock_bh(&p->lock);

	ppe_drv_info("%p: registered cb:%p app_data:%p for cc:%u", p, cb, app_data, cc);
}
EXPORT_SYMBOL(ppe_drv_cc_register_cb);

/*
 * ppe_drv_cc_entries_free()
 *	Free cc instance if it was allocated.
 */
void ppe_drv_cc_entries_free(struct ppe_drv_cc *cc)
{
	vfree(cc);
}

/*
 * ppe_drv_cc_entries_alloc()
 *	Allocates and initializes cpu code table and update which cpu code we plan to maintain.
 */
struct ppe_drv_cc *ppe_drv_cc_entries_alloc(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_cc *cc;
	struct ppe_drv_cc *pcc;
	const uint8_t max_exception = ppe_drv_exception_max();
	uint8_t cpu_code, i;

	cc = vzalloc(sizeof(struct ppe_drv_cc) * PPE_DRV_CC_MAX);
	if (!cc) {
		ppe_drv_warn("%p: Failed to allocate cpu code table entries", p);
		return NULL;
	}

	/*
	 * Enable cpu code for which connection flush would be attempted.
	 *
	 * Note: we start with all the exceptions for which flow-deceleration is enabled.
	 * If this creates problem we can manually enable specific CPU code for which flush
	 * is needed.
	 */
	for (i = 1; i < max_exception; i++) {
		cpu_code = ppe_drv_exception_list[i].code;
		pcc = &cc[cpu_code];
		pcc->flush = true;
	}

	pcc = &cc[PPE_DRV_CC_L2_EXP_MTU_FAIL];
	pcc->flush = true;

	pcc = &cc[PPE_DRV_CC_L3_EXP_MTU_FAIL];
	pcc->flush = true;

	pcc = &cc[PPE_DRV_CC_L3_EXP_FLOW_MTU_FAIL];
	pcc->flush = true;

	return cc;
}
