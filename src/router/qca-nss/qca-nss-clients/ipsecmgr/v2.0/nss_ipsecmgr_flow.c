/*
 **************************************************************************
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */
#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/of.h>
#include <linux/ipv6.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/etherdevice.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/atomic.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/ip6_route.h>

#include <crypto/aead.h>
#include <crypto/internal/hash.h>

#include <nss_api_if.h>
#include <nss_ipsec_cmn.h>
#include <nss_ipsecmgr.h>

#include "nss_ipsecmgr_ref.h"
#include "nss_ipsecmgr_flow.h"
#include "nss_ipsecmgr_sa.h"
#include "nss_ipsecmgr_ctx.h"
#include "nss_ipsecmgr_tunnel.h"
#include "nss_ipsecmgr_priv.h"

extern struct nss_ipsecmgr_drv *ipsecmgr_drv;

/*
 * Flow tuple print info
 */
static const struct nss_ipsecmgr_print ipsecmgr_print_flow_tuple[] = {
	{"dest_ip", NSS_IPSECMGR_PRINT_IPADDR},
	{"src_ip", NSS_IPSECMGR_PRINT_IPADDR},
	{"spi", NSS_IPSECMGR_PRINT_WORD},
	{"dst_port", NSS_IPSECMGR_PRINT_WORD},
	{"src_port", NSS_IPSECMGR_PRINT_WORD},
	{"user_pattern", NSS_IPSECMGR_PRINT_WORD},
	{"protocol", NSS_IPSECMGR_PRINT_WORD},
	{"ip_version", NSS_IPSECMGR_PRINT_WORD},
};

/*
 * nss_ipsecmgr_flow_print_len()
 * 	Print flow length
 */
static ssize_t nss_ipsecmgr_flow_print_len(struct nss_ipsecmgr_ref *ref)
{
	const struct nss_ipsecmgr_print *prn = ipsecmgr_print_flow_tuple;
	ssize_t len = NSS_IPSECMGR_FLOW_PRINT_EXTRA;
	int i;

	for (i = 0; i < ARRAY_SIZE(ipsecmgr_print_flow_tuple); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	return len;
}

/*
 * nss_ipsecmgr_flow_print()
 * 	Print flow
 */
static ssize_t nss_ipsecmgr_flow_print(struct nss_ipsecmgr_ref *ref, char *buf)
{
	struct nss_ipsecmgr_flow *flow = container_of(ref, struct nss_ipsecmgr_flow, ref);
	const struct nss_ipsecmgr_print *prn = ipsecmgr_print_flow_tuple;
	struct nss_ipsec_cmn_flow_tuple *tuple = &flow->state.tuple;
	uint32_t dest_ip[4], src_ip[4];
	ssize_t max_len, len = 0;

	max_len = nss_ipsecmgr_flow_print_len(&flow->ref);
	len += snprintf(buf + len, max_len - len, "Flow tuple: {");

	switch (tuple->ip_ver) {
	case IPVERSION:
		len += snprintf(buf + len, max_len - len, "%s: %pI4h,", prn->str, tuple->dest_ip);
		prn++;

		len += snprintf(buf + len, max_len - len, "%s: %pI4h,", prn->str, tuple->src_ip);
		prn++;

		break;
	case 6:
		nss_ipsecmgr_hton_v6addr(src_ip, tuple->src_ip);
		nss_ipsecmgr_hton_v6addr(dest_ip, tuple->dest_ip);

		len += snprintf(buf + len, max_len - len, "%s: %pI6c,", prn->str, dest_ip);
		prn++;

		len += snprintf(buf + len, max_len - len, "%s: %pI6c,", prn->str, src_ip);
		prn++;

		break;
	}

	len += snprintf(buf + len, max_len - len, "%s: 0x%x,", prn->str, tuple->spi_index);
	prn++;

	len += snprintf(buf + len, max_len - len, "%s: %u,", prn->str, tuple->dst_port);
	prn++;

	len += snprintf(buf + len, max_len - len, "%s: %u,", prn->str, tuple->src_port);
	prn++;

	len += snprintf(buf + len, max_len - len, "%s: %u,", prn->str, tuple->user_pattern);
	prn++;

	len += snprintf(buf + len, max_len - len, "%s: %u,", prn->str, tuple->protocol);
	prn++;

	len += snprintf(buf + len, max_len - len, "%s: %u", prn->str, tuple->ip_ver);
	prn++;

	len += snprintf(buf + len, max_len - len, "}\n");

	return len;
}

/*
 * nss_ipsecmgr_flow_update_db()
 *	Add flow to DB.
 */
static bool nss_ipsecmgr_flow_update_db(struct nss_ipsecmgr_flow *flow)
{
	struct nss_ipsec_cmn_sa_tuple *sa_tuple = &flow->state.sa;
	struct nss_ipsecmgr_sa *sa;
	uint32_t hash_idx;

	hash_idx = nss_ipsecmgr_flow_tuple2hash(&flow->state.tuple, NSS_IPSECMGR_FLOW_MAX);

	write_lock(&ipsecmgr_drv->lock);
	sa = nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, sa_tuple);
	if (!sa) {
		write_unlock(&ipsecmgr_drv->lock);
		nss_ipsecmgr_trace("%px: failed to find SA during flow update", flow);
		return false;
	}

	flow->sa = sa;
	/*
	 * Add reference of the flow to the SA associated with the outer.
	 */
	nss_ipsecmgr_ref_add(&flow->ref, &sa->ref);
	list_add(&flow->list, &ipsecmgr_drv->flow_db[hash_idx]);
	write_unlock(&ipsecmgr_drv->lock);
	return true;
}

/*
 * nss_ipsecmgr_flow_create_resp()
 *	response for the flow message
 */
static void nss_ipsecmgr_flow_create_resp(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_ipsecmgr_flow *flow = app_data;

	/*
	 * If, NSS rejected the flow add then we will not
	 * add it to our list
	 */
	if (ncm->response != NSS_CMN_RESPONSE_ACK) {
		nss_ipsecmgr_trace("%px: NSS response error (%u)", flow, ncm->error);
		kfree(flow);
		return;
	}

	if (!nss_ipsecmgr_flow_update_db(flow)) {
		nss_ipsecmgr_trace("%px: failed to update flow in DB (%u)", flow, ncm->error);
		kfree(flow);
		return;
	}

	nss_ipsecmgr_trace("%px: Sucessfully updated flow in DB", flow);
}

/*
 * nss_ipsecmgr_flow_free()
 *	Free flow object
 */
static void nss_ipsecmgr_flow_free(struct nss_ipsecmgr_flow *flow)
{
	kfree(flow);
}

/*
 * nss_ipsecmgr_flow_del_ref()
 *	Unlink the flow entry
 */
static void nss_ipsecmgr_flow_del_ref(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_flow *flow = container_of(ref, struct nss_ipsecmgr_flow, ref);

	/*
	 * Write lock needs to be held by the caller since flow db is
	 * getting modified.
	 */
	BUG_ON(write_can_lock(&ipsecmgr_drv->lock));
	list_del_init(&flow->list);
}

/*
 * nss_ipsecmgr_flow_free_ref()
 *	Free the flow entry
 */
static void nss_ipsecmgr_flow_free_ref(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_flow *flow = container_of(ref, struct nss_ipsecmgr_flow, ref);
	enum nss_ipsec_cmn_msg_type type = NSS_IPSEC_CMN_MSG_TYPE_FLOW_DESTROY;
	struct nss_ipsec_cmn_msg nicm;
	nss_tx_status_t status;

	memset(&nicm, 0, sizeof(nicm));

	memcpy(&nicm.msg.flow.flow_tuple, &flow->state.tuple, sizeof(nicm.msg.flow.flow_tuple));
	memcpy(&nicm.msg.flow.sa_tuple, &flow->state.sa, sizeof(nicm.msg.flow.sa_tuple));

	status = nss_ipsec_cmn_tx_msg_sync(flow->nss_ctx, flow->ifnum, type, sizeof(nicm.msg.flow), &nicm);
	if (status != NSS_TX_SUCCESS) {
		nss_ipsecmgr_info("%px: failed to send the flow message(%u)", flow, type);
	}

	if (nicm.cm.error != NSS_IPSEC_CMN_MSG_ERROR_NONE) {
		nss_ipsecmgr_warn("%px: failed to free flow from NSS (%u)", flow, nicm.cm.error);
	}

	nss_ipsecmgr_flow_free(flow);
}

/*
 * nss_ipsecmgr_flow_alloc()
 *	Add a new flow to database
 */
static struct nss_ipsecmgr_flow *nss_ipsecmgr_flow_alloc(struct nss_ipsecmgr_sa *sa,
							struct nss_ipsec_cmn_flow_tuple *flow_tuple,
							struct nss_ipsec_cmn_sa_tuple *sa_tuple)
{
	struct nss_ipsecmgr_flow *flow = NULL;

	flow = kzalloc(sizeof(*flow), GFP_ATOMIC);
	if (!flow) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_info("%px: failed to allocate flow", sa);
		return NULL;
	}

	/*
	 * Initialize the flow entry
	 */
	flow->ifnum = sa->ifnum;
	flow->nss_ctx = sa->nss_ctx;
	flow->tunnel_id = sa->tunnel_id;

	INIT_LIST_HEAD(&flow->list);
	nss_ipsecmgr_ref_init(&flow->ref, nss_ipsecmgr_flow_del_ref, nss_ipsecmgr_flow_free_ref);
	nss_ipsecmgr_ref_init_print(&flow->ref, nss_ipsecmgr_flow_print_len, nss_ipsecmgr_flow_print);

	/*
	 * Copy information received for flow creation
	 */
	memcpy(&flow->state.tuple, flow_tuple, sizeof(flow->state.tuple));
	memcpy(&flow->state.sa, sa_tuple, sizeof(flow->state.sa));

	return flow;
}

/*
 * nss_ipsecmgr_flow_find()
 *	Lookup flow_tuple in the flow data base
 *
 * Note: No locks are taken here; so it needs to be called with a read/write lock held.
 */
struct nss_ipsecmgr_flow *nss_ipsecmgr_flow_find(struct list_head *db, struct nss_ipsec_cmn_flow_tuple *tuple)
{
	uint32_t hash_idx = nss_ipsecmgr_flow_tuple2hash(tuple, NSS_IPSECMGR_FLOW_MAX);
	struct list_head *head = &db[hash_idx];
	struct nss_ipsecmgr_flow *flow;

	/*
	 * Linux does not provide any specific API(s) to test for RW locks. The caller
	 * being internal is assumed to hold write lock before initiating this.
	 */
	list_for_each_entry(flow, head, list) {
		if (nss_ipsecmgr_flow_tuple_match(&flow->state.tuple, tuple))
			return flow;
	}

	return NULL;
}

/*
 * nss_ipsecmgr_flow_get_sa()
 *	Lookup flow_tuple in flow database and return sa_tuple.
 */
nss_ipsecmgr_status_t nss_ipsecmgr_flow_get_sa(struct net_device *tun, struct nss_ipsecmgr_flow_tuple *flow_tuple,
					struct nss_ipsecmgr_sa_tuple *sa_tuple)
{
	struct nss_ipsec_cmn_flow_tuple tuple;
	struct nss_ipsecmgr_flow *flow;
	struct nss_ipsecmgr_sa *sa;

	nss_ipsecmgr_flow2tuple(flow_tuple, &tuple);
	write_lock_bh(&ipsecmgr_drv->lock);

	flow = nss_ipsecmgr_flow_find(ipsecmgr_drv->flow_db, &tuple);
	if (!flow) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_trace("%px: Failed to find flow", tun);
		return NSS_IPSECMGR_FAIL_FLOW;
	}

	sa = flow->sa;
	if (!sa) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_error("%px: SA is not associated with flow", tun);
		return NSS_IPSECMGR_INVALID_SA;
	}

	nss_ipsecmgr_sa_tuple2sa(&sa->state.tuple, sa_tuple);
	write_unlock_bh(&ipsecmgr_drv->lock);

	return NSS_IPSECMGR_OK;
}
EXPORT_SYMBOL(nss_ipsecmgr_flow_get_sa);

/*
 * nss_ipsecmgr_flow_del()
 *	Delete a existing flow from the database
 *
 * Note: We will only delete the flow associated with the requested SA.
 * If we donot find the association then the flow delete will not perform
 * the delete
 */
void nss_ipsecmgr_flow_del(struct net_device *dev, struct nss_ipsecmgr_flow_tuple *f_tuple,
				struct nss_ipsecmgr_sa_tuple *s_tuple)
{
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct list_head *flow_db = ipsecmgr_drv->flow_db;
	struct list_head *sa_db = ipsecmgr_drv->sa_db;
	struct nss_ipsec_cmn_flow_tuple flow_tuple = {0};
	struct nss_ipsec_cmn_sa_tuple sa_tuple = {0};
	struct nss_ipsecmgr_flow *flow;
	struct nss_ipsecmgr_sa *sa;

	nss_ipsecmgr_flow2tuple(f_tuple, &flow_tuple);
	nss_ipsecmgr_sa2tuple(s_tuple, &sa_tuple);

	/*
	 * Write lock needed here since Flow DB is looked up and removed
	 * No one should be accessing flow at this point
	 */
	write_lock_bh(&ipsecmgr_drv->lock);

	sa = nss_ipsecmgr_sa_find(sa_db, &sa_tuple);
	if (!sa) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: failed to find SA during flow_del", tun);
		return;
	}

	flow = nss_ipsecmgr_flow_find(flow_db, &flow_tuple);
	if (!flow) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: failed to find flow for flow_del", tun);
		return;
	}

	/*
	 * Match if the SA provided in indeed associated with the flow. In case this is
	 * not associated due to re-key then we should not remove the flow
	 */
	if (flow->sa != sa) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: failed to match the SA in flow(%px) delete", tun, flow);
		return;
	}

	/*
	 * Free the flow entry and reference.
	 */
	nss_ipsecmgr_ref_del(&flow->ref, &tun->free_refs);
	write_unlock_bh(&ipsecmgr_drv->lock);

	schedule_work(&tun->free_work);
}
EXPORT_SYMBOL(nss_ipsecmgr_flow_del);

/*
 * nss_ipsecmgr_flow_add()
 *	Add a new flow to database asynchronously.
 */
nss_ipsecmgr_status_t nss_ipsecmgr_flow_add(struct net_device *dev, struct nss_ipsecmgr_flow_tuple *f_tuple,
						struct nss_ipsecmgr_sa_tuple *s_tuple)
{
	enum nss_ipsec_cmn_msg_type type = NSS_IPSEC_CMN_MSG_TYPE_FLOW_CREATE;
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct nss_ipsec_cmn_flow_tuple *flow_tuple;
	struct nss_ipsec_cmn_sa_tuple *sa_tuple;
	struct nss_ipsec_cmn_msg nicm = {{0}};
	struct nss_ipsecmgr_flow *flow;
	struct nss_ipsecmgr_ctx *ctx;
	struct nss_ipsecmgr_sa *sa;
	nss_tx_status_t status;

	dev_hold(dev);

	flow_tuple = &nicm.msg.flow.flow_tuple;
	sa_tuple = &nicm.msg.flow.sa_tuple;

	nss_ipsecmgr_sa2tuple(s_tuple, sa_tuple);
	nss_ipsecmgr_flow2tuple(f_tuple, flow_tuple);

	/*
	 * Write lock needed here since Flow DB is looked up and added
	 * No one should be accessing/removing flow at this point
	 */
	write_lock_bh(&ipsecmgr_drv->lock);

	/*
	 * Check if the SA exists
	 */
	sa = nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, sa_tuple);
	if (!sa) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: failed to find SA during flow_add", tun);
		dev_put(dev);
		return NSS_IPSECMGR_INVALID_SA;
	}

	/*
	 * Check if the context exists
	 */
	ctx = nss_ipsecmgr_ctx_find(tun, sa->type);
	if (!ctx) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: Failed to find context (%u)", tun, sa->type);
		dev_put(dev);
		return NSS_IPSECMGR_INVALID_CTX;
	}

	/*
	 * TODO: Why do we need this
	 */
	memcpy(sa_tuple, &sa->state.tuple, sizeof(*sa_tuple));

	/*
	 * At this point we do not know whether the flow is duplicate or not
	 * we try to perform a flow lookup to determine whether the flow already
	 * exists in the data base. Once, we have determined the flow then we check
	 * if the caller is performing a SA switch.
	 */
	flow = nss_ipsecmgr_flow_find(ipsecmgr_drv->flow_db, flow_tuple);
	if (flow) {
		/*
		 * Flow already exists; check if the SA entry is the same. In case
		 * the SA is same then it is surely a duplicate entry. We need to
		 * return from here without doing anything.
		 */
		if (flow->sa == sa) {
			write_unlock_bh(&ipsecmgr_drv->lock);
			nss_ipsecmgr_trace("%px: Duplicate flow in flow_add", ipsecmgr_drv);
			dev_put(dev);
			return NSS_IPSECMGR_DUPLICATE_FLOW;
		}

		/*
		 * Detach the flow from the reference tree and flow database, then free the object.
		 * Here we don't want to inform the NSS as it is not a real flow free.
		 * We are just moving an existing flow to a new SA
		 */
		nss_ipsecmgr_ref_del(&flow->ref, NULL);
		nss_ipsecmgr_flow_free(flow);
	}

	flow = nss_ipsecmgr_flow_alloc(sa, flow_tuple, sa_tuple);
	if (!flow) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_info("%px: failed to allocate flow", tun);
		dev_put(dev);
		return NSS_IPSECMGR_FAIL_FLOW_ALLOC;
	}

	write_unlock_bh(&ipsecmgr_drv->lock);

	/*
	 * Initialize the IPsec message to be sent.
	 */
	nss_ipsec_cmn_msg_init(&nicm, flow->ifnum, type, sizeof(nicm.msg.flow), nss_ipsecmgr_flow_create_resp, flow);

	/*
	 * Note: We don't need to hold the lock for the accessing the flow since
	 * it is not part of the list yet. Now, send a message to firmware for
	 * updating the flow in its table. Once, NSS confirms it then we will
	 * add the flow back in the database
	 */
	status = nss_ipsec_cmn_tx_msg(flow->nss_ctx, &nicm);
	if (status != NSS_TX_SUCCESS) {
		nss_ipsecmgr_info("%px: failed to send the flow create message", dev);
		nss_ipsecmgr_flow_free(flow);
		dev_put(dev);
		return NSS_IPSECMGR_FAIL;
	}

	dev_put(dev);
	return NSS_IPSECMGR_OK;
}
EXPORT_SYMBOL(nss_ipsecmgr_flow_add);

/*
 * nss_ipsecmgr_flow_add()
 *	Add a new flow to database synchronously.
 */
nss_ipsecmgr_status_t nss_ipsecmgr_flow_add_sync(struct net_device *dev, struct nss_ipsecmgr_flow_tuple *f_tuple,
						struct nss_ipsecmgr_sa_tuple *s_tuple)
{

	enum nss_ipsec_cmn_msg_type type = NSS_IPSEC_CMN_MSG_TYPE_FLOW_CREATE;
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct nss_ipsec_cmn_flow_tuple *flow_tuple;
	struct nss_ipsec_cmn_sa_tuple *sa_tuple;
	struct nss_ipsec_cmn_msg nicm = {{0}};
	struct nss_ipsecmgr_flow *flow;
	struct nss_ipsecmgr_ctx *ctx;
	struct nss_ipsecmgr_sa *sa;
	nss_tx_status_t status;

	BUG_ON(in_atomic());

	dev_hold(dev);

	memset(&nicm, 0, sizeof(nicm));

	flow_tuple = &nicm.msg.flow.flow_tuple;
	sa_tuple = &nicm.msg.flow.sa_tuple;

	nss_ipsecmgr_sa2tuple(s_tuple, sa_tuple);
	nss_ipsecmgr_flow2tuple(f_tuple, flow_tuple);

	/*
	 * Write lock needed here since Flow DB is looked up and added
	 * No one should be accessing/removing flow at this point
	 */
	write_lock_bh(&ipsecmgr_drv->lock);

	/*
	 * Check if the SA exists
	 */
	sa = nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, sa_tuple);
	if (!sa) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: failed to find SA during flow_add", tun);
		dev_put(dev);
		return NSS_IPSECMGR_INVALID_SA;
	}

	/*
	 * Check if the context exists
	 */
	ctx = nss_ipsecmgr_ctx_find(tun, sa->type);
	if (!ctx) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_warn("%px: Failed to find context (%u)", tun, sa->type);
		dev_put(dev);
		return NSS_IPSECMGR_INVALID_CTX;
	}

	/*
	 * TODO: Why do we need this
	 */
	memcpy(sa_tuple, &sa->state.tuple, sizeof(*sa_tuple));

	/*
	 * At this point we do not know whether the flow is duplicate or not
	 * we try to perform a flow lookup to determine whether the flow already
	 * exists in the data base. Once, we have determined the flow then we check
	 * if the caller is performing a SA switch.
	 */
	flow = nss_ipsecmgr_flow_find(ipsecmgr_drv->flow_db, flow_tuple);
	if (flow) {
		/*
		 * Flow already exists; check if the SA entry is the same. In case
		 * the SA is same then it is surely a duplicate entry. We need to
		 * return from here without doing anything.
		 */
		if (flow->sa == sa) {
			write_unlock_bh(&ipsecmgr_drv->lock);
			nss_ipsecmgr_trace("%px: Duplicate flow in flow_add", ipsecmgr_drv);
			dev_put(dev);
			return NSS_IPSECMGR_DUPLICATE_FLOW;
		}

		/*
		 * Detach the flow from the reference tree and flow database, then free the object.
		 * Here we don't want to inform the NSS as it is not a real flow free.
		 * We are just moving an existing flow to a new SA
		 */
		nss_ipsecmgr_ref_del(&flow->ref, NULL);
		nss_ipsecmgr_flow_free(flow);
	}

	flow = nss_ipsecmgr_flow_alloc(sa, flow_tuple, sa_tuple);
	if (!flow) {
		write_unlock_bh(&ipsecmgr_drv->lock);
		nss_ipsecmgr_info("%px: failed to allocate flow", tun);
		dev_put(dev);
		return NSS_IPSECMGR_FAIL_FLOW_ALLOC;
	}

	write_unlock_bh(&ipsecmgr_drv->lock);

	/*
	 * Initialize the IPsec message to be sent.
	 */
	nss_ipsec_cmn_msg_init(&nicm, flow->ifnum, type, sizeof(nicm.msg.flow), NULL, NULL);

	/*
	 * Note: We don't need to hold the lock for the accessing the flow since
	 * it is not part of the list yet. Now, send a message to firmware for
	 * updating the flow in its table. Once, NSS confirms it then we will
	 * add the flow back in the database
	 */
	status = nss_ipsec_cmn_tx_msg_sync(flow->nss_ctx, flow->ifnum, type, sizeof(nicm.msg.flow), &nicm);
	if (status != NSS_TX_SUCCESS) {
		nss_ipsecmgr_info("%px: failed to send the flow create message", dev);
		nss_ipsecmgr_flow_free(flow);
		dev_put(dev);
		return NSS_IPSECMGR_FAIL;
	}

	/*
	 * Since, this is a synchronous call add it to the database directly
	 */
	if (!nss_ipsecmgr_flow_update_db(flow)) {
		nss_ipsecmgr_info("%px: failed to add flow to DB", dev);
		nss_ipsecmgr_flow_free(flow);
		dev_put(dev);
		return NSS_IPSECMGR_FAIL_ADD_DB;
	}

	dev_put(dev);
	return NSS_IPSECMGR_OK;
}
EXPORT_SYMBOL(nss_ipsecmgr_flow_add_sync);
