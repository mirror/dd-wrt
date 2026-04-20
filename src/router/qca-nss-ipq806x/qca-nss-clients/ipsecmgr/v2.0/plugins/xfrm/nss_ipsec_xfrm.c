/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/if.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/xfrm.h>
#include <net/protocol.h>
#include <linux/inetdevice.h>
#include <linux/icmp.h>
#include <net/addrconf.h>
#include <linux/netfilter.h>
#include <linux/debugfs.h>

#include <nss_api_if.h>
#include <nss_ipsec.h>
#include <nss_ipsecmgr.h>
#include <ecm_interface_ipsec.h>
#include <ecm_notifier.h>
#if defined(NSS_L2TPV2_ENABLED)
#include <nss_l2tpmgr.h>
#endif
#if defined(NSS_VXLAN_ENABLED)
#include <nss_vxlanmgr.h>
#endif
#include "nss_ipsec_xfrm_tunnel.h"
#include "nss_ipsec_xfrm_sa.h"
#include "nss_ipsec_xfrm_flow.h"
#include "nss_ipsec_xfrm.h"

/*
 * Global NSS IPsec xfrm pluging instance
 */
static struct nss_ipsec_xfrm_drv g_ipsec_xfrm;

/*
 * Statistics print info
 */
static const struct nss_ipsec_xfrm_print nss_ipsec_xfrm_drv_stats[] = {
	{"\tencap_nss", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tencap_drop", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tdecap_nss", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tdecap_drop", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tinner_except", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tinner_drop", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\touter_except", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\touter_drop", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\ttun_alloced", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\ttun_dealloced", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\ttun_freed", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tsa_alloced", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tsa_dealloced", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tsa_freed", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tflow_alloced", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tflow_dealloced", NSS_IPSEC_XFRM_PRINT_DWORD},
	{"\tflow_freed", NSS_IPSEC_XFRM_PRINT_DWORD},
};

/*
 * nss_ipsec_xfrm_drv_stats_size()
 *	Calculate size of drv stats.
 */
static ssize_t nss_ipsec_xfrm_drv_stats_size(void)
{
	const struct nss_ipsec_xfrm_print *prn = nss_ipsec_xfrm_drv_stats;
	ssize_t len = NSS_IPSEC_XFRM_PRINT_EXTRA;
	int i;

	for (i = 0; i < ARRAY_SIZE(nss_ipsec_xfrm_drv_stats); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	return len;
}

/*
 * nss_ipsec_xfrm_drv_print()
 *	Print drv stats.
 */
static ssize_t nss_ipsec_xfrm_drv_print(struct nss_ipsec_xfrm_drv *drv , char *buf, ssize_t max_len)
{
	const struct nss_ipsec_xfrm_print *prn = nss_ipsec_xfrm_drv_stats;
	atomic64_t *stats_dword = (atomic64_t *)&drv->stats;
	ssize_t len = 0;
	int i;

	/*
	 * This expects a strict order as per the stats structure
	 */
	len += snprintf(buf + len, max_len - len, "stats: {\n");

	for (i = 0; i < ARRAY_SIZE(nss_ipsec_xfrm_drv_stats); i++, prn++, stats_dword++)
		len += snprintf(buf + len, max_len - len, "%s: %llu\n", prn->str, (uint64_t)atomic64_read(stats_dword));

	len += snprintf(buf + len, max_len - len, "}\n");

	return len;
}

/*
 * nss_ipsec_xfrm_drv_read()
 *	Read nss ipsec xfrm drv info
 */
ssize_t nss_ipsec_xfrm_drv_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_ipsec_xfrm_drv *drv = fp->private_data;
	ssize_t print_len = drv->stats_len;
	ssize_t len = 0;
	char *buf;

	buf = vzalloc(print_len);
	if (!buf) {
		nss_ipsec_xfrm_warn("%px: failed to allocate print buffer (req:%zd)", drv, print_len);
		return 0;
	}

	len = nss_ipsec_xfrm_drv_print(drv, buf, print_len);

	len = simple_read_from_buffer(ubuf, sz, ppos, buf, len);
	vfree(buf);

	return len;
}

/*
 * file operation structure instance
 */
const struct file_operations nss_ipsec_xfrm_drv_file_ops = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_ipsec_xfrm_drv_read,
};

/*
 * nss_ipsec_xfrm_debugfs_deinit()
 *	Remove debugfs.
 */
static void nss_ipsec_xfrm_debugfs_deinit(struct nss_ipsec_xfrm_drv *drv)
{
	debugfs_remove_recursive(drv->dentry);
}

/*
 * nss_ipsec_xfrm_debugfs_init()
 *	Init debugfs.
 */
static void nss_ipsec_xfrm_debugfs_init(struct nss_ipsec_xfrm_drv *drv)
{
	if (!debugfs_initialized())
		return;

	drv->dentry = debugfs_create_dir("qca-nss-ipsec-xfrm", NULL);
	if (!drv->dentry) {
		nss_ipsec_xfrm_err("%p: Failed to create debugfs\n", drv);
		return;
	}

	drv->stats_len = nss_ipsec_xfrm_drv_stats_size();

	debugfs_create_file("stats", S_IRUGO, drv->dentry, drv, &nss_ipsec_xfrm_drv_file_ops);
}

/*
 * nss_ipsec_xfrm_hash_flow()
 *	Computes hash for a given nss_ipsecmgr_flow_tuple.
 */
static inline uint32_t nss_ipsec_xfrm_hash_flow(struct nss_ipsecmgr_flow_tuple *tuple, uint32_t seed, uint16_t size)
{
	uint32_t hash;

	BUILD_BUG_ON(!IS_ALIGNED(sizeof(struct nss_ipsecmgr_flow_tuple), sizeof(uint32_t)));
	hash = nss_ipsec_xfrm_get_hash((uint32_t *)tuple, sizeof(struct nss_ipsecmgr_flow_tuple), seed);
	return (hash & (size -1));
}

/*
 * nss_ipsec_xfrm_init_flow_db()
 *	Initializes the flow db.
 */
static void nss_ipsec_xfrm_init_flow_db(struct nss_ipsec_xfrm_drv *drv)
{
	uint32_t i;

	for (i = 0; i < NSS_IPSEC_XFRM_FLOW_DB_MAX; i++) {
		INIT_LIST_HEAD(&g_ipsec_xfrm.flow_db[i]);
	}
}

/*
 * nss_ipsec_xfrm_flush_flow()
 *	Flushes the provided list of flows.
 */
void nss_ipsec_xfrm_flush_flow(struct list_head *head)
{
	struct nss_ipsec_xfrm_flow *flow, *tmp;

	list_for_each_entry_safe(flow, tmp, head, list_entry) {
		list_del_init(&flow->list_entry);
		nss_ipsec_xfrm_flow_dealloc(flow);
	}
}

/*
 * nss_ipsec_xfrm_flush_flow_all()
 *	Flush the flow DB. Caller needs to hold the write lock.
 */
void nss_ipsec_xfrm_flush_flow_all(struct nss_ipsec_xfrm_drv *drv)
{
	struct list_head free_head;
	uint32_t i;

	INIT_LIST_HEAD(&free_head);

	/*
	 * Splice the database and create list of flows to be freed
	 */
	write_lock_bh(&drv->lock);
	for (i = 0; i < NSS_IPSEC_XFRM_FLOW_DB_MAX; i++) {
		list_splice_init(&drv->flow_db[i], &free_head);
	}

	write_unlock_bh(&drv->lock);

	nss_ipsec_xfrm_flush_flow(&free_head);
}

/*
 * nss_ipsec_xfrm_flush_flow_by_sa()
 *	Delete flows corrosponding to an xfrm_state. Caller needs to hold the write lock.
 */
static void nss_ipsec_xfrm_flush_flow_by_sa(struct nss_ipsec_xfrm_drv *drv, struct nss_ipsec_xfrm_sa *sa)
{
	struct list_head *db_head = drv->flow_db;
	struct nss_ipsec_xfrm_flow *flow, *tmp;
	struct list_head free_head;
	uint32_t count;

	INIT_LIST_HEAD(&free_head);
	write_lock_bh(&drv->lock);

	for (count = NSS_IPSEC_XFRM_FLOW_DB_MAX; count--; db_head++) {
		list_for_each_entry_safe(flow, tmp, db_head, list_entry) {
			if (READ_ONCE(flow->sa) == sa) {
				list_del_init(&flow->list_entry);
				list_add(&flow->list_entry, &free_head);
			}
		}
	}

	write_unlock_bh(&drv->lock);
	nss_ipsec_xfrm_flush_flow(&free_head);
}

/*
 * nss_ipsec_xfrm_del_flow()
 *	Delete the specified flow object. Caller needs to hold the write lock.
 */
static void nss_ipsec_xfrm_del_flow(struct nss_ipsec_xfrm_drv *drv, struct nss_ipsec_xfrm_flow *flow)
{
	write_lock_bh(&drv->lock);
	list_del_init(&flow->list_entry);
	write_unlock_bh(&drv->lock);

	nss_ipsec_xfrm_trace("%p: Flow deleted from database\n", flow);

	/*
	 * Delete any other objects that could be holding reference to the flow object.
	 * There is no such object for now. So we directly dealloc the flow object.
	 */
	nss_ipsec_xfrm_flow_dealloc(flow);
}

/*
 * nss_ipsec_xfrm_add_flow()
 *	Add a flow object to the SA, and insert into flow db. The caller needs to hold the write lock.
 */
static void nss_ipsec_xfrm_add_flow(struct nss_ipsec_xfrm_drv *drv, struct nss_ipsec_xfrm_flow *flow)
{
	uint32_t hash_idx;

	write_lock_bh(&drv->lock);
	hash_idx = nss_ipsec_xfrm_hash_flow(&flow->tuple, drv->hash_nonce, NSS_IPSEC_XFRM_FLOW_DB_MAX);
	list_add(&flow->list_entry, &drv->flow_db[hash_idx]);
	write_unlock_bh(&drv->lock);

	nss_ipsec_xfrm_trace("%p: Flow added to database (hash_idx:%d)\n", flow, hash_idx);
}

/*
 * nss_ipsec_xfrm_ref_flow()
 *	Get flow and acquire reference to it.
 *
 * Note: The caller needs to hold the lock. This function holds the reference to the object
 */
static struct nss_ipsec_xfrm_flow *nss_ipsec_xfrm_ref_flow(struct nss_ipsec_xfrm_drv *drv,
					struct nss_ipsecmgr_flow_tuple *tuple)
{
	struct nss_ipsec_xfrm_flow *flow;
	uint32_t hash_idx;

	hash_idx = nss_ipsec_xfrm_hash_flow(tuple, drv->hash_nonce, NSS_IPSEC_XFRM_FLOW_DB_MAX);
	nss_ipsec_xfrm_trace("%p: flow hash_idx (%u)", drv, hash_idx);

	read_lock_bh(&drv->lock);
	list_for_each_entry(flow, &drv->flow_db[hash_idx], list_entry) {
		if (nss_ipsec_xfrm_flow_match(flow, tuple)) {
			flow = nss_ipsec_xfrm_flow_ref(flow);
			read_unlock_bh(&drv->lock);
			return flow;
		}
	}

	read_unlock_bh(&drv->lock);

	return NULL;
}

/*
 * nss_ipsec_xfrm_hash_tun()
 *	Computes hash for a given SIP/DIP pair.
 */
static inline uint32_t nss_ipsec_xfrm_hash_tun(const xfrm_address_t *dest_ip, const xfrm_address_t *src_ip,
		uint32_t seed, uint16_t size)
{
	uint32_t hash;

	hash = nss_ipsec_xfrm_get_hash((uint32_t *)dest_ip, 16, seed);
	hash = nss_ipsec_xfrm_get_hash((uint32_t *)src_ip, 16, hash);
	return (hash & (size -1));
}

/*
 * nss_ipsec_xfrm_init_tun_db()
 *	Initializes the tunnel db.
 */
static void nss_ipsec_xfrm_init_tun_db(struct nss_ipsec_xfrm_drv *drv)
{
	uint32_t i;

	for (i = 0; i < NSS_IPSEC_XFRM_TUN_DB_MAX; i++) {
		INIT_LIST_HEAD(&g_ipsec_xfrm.tun_db[i]);
	}
}

/*
 * nss_ipsec_xfrm_del_tun()
 *	Remove tunnel from tunnel db.
 */
static void nss_ipsec_xfrm_del_tun(struct nss_ipsec_xfrm_drv *drv, struct nss_ipsec_xfrm_tunnel *tun)
{
	write_lock_bh(&drv->lock);
	list_del_init(&tun->list_entry);
	write_unlock_bh(&drv->lock);

	nss_ipsec_xfrm_trace("%p: Tunnel deleted from database\n", tun);
	nss_ipsec_xfrm_tunnel_dealloc(tun);
}

/*
 * nss_ipsec_xfrm_add_tun()
 *	Insert tunnel into tunnel db.
 */
static void nss_ipsec_xfrm_add_tun(struct nss_ipsec_xfrm_drv *drv, struct nss_ipsec_xfrm_tunnel *tun)
{
	uint32_t hash_idx;

	write_lock_bh(&drv->lock);
	hash_idx = nss_ipsec_xfrm_hash_tun(&tun->remote, &tun->local, drv->hash_nonce, NSS_IPSEC_XFRM_TUN_DB_MAX);
	list_add(&tun->list_entry, &drv->tun_db[hash_idx]);
	write_unlock_bh(&drv->lock);
}

/*
 * nss_ipsec_xfrm_has_tun()
 *	Returns true if the given net_dev corrosponds to a ipsec xfrm tunnel.
 *	The caller needs to hold the read lock.
 */
bool nss_ipsec_xfrm_has_tun(struct nss_ipsec_xfrm_drv *drv, struct net_device *net_dev)
{
	struct list_head *db_head = drv->tun_db;
	struct nss_ipsec_xfrm_tunnel *tun, *tmp;
	bool found = false;
	uint32_t count;

	read_lock_bh(&drv->lock);
	for (count = NSS_IPSEC_XFRM_TUN_DB_MAX; count--; db_head++) {
		list_for_each_entry_safe(tun, tmp, db_head, list_entry) {
			if (tun->dev == net_dev) {
				found = true;
				break;
			}
		}
	}

	read_unlock_bh(&drv->lock);
	return found;
}

/*
 * nss_ipsec_xfrm_ref_tun()
 *	Get tunnel and acquire reference to it.
 *
 * Note: The caller needs to hold the lock. This function holds the reference to the object
 */
struct nss_ipsec_xfrm_tunnel *nss_ipsec_xfrm_ref_tun(struct nss_ipsec_xfrm_drv *drv, xfrm_address_t *local,
				xfrm_address_t *remote, uint16_t family)
{
	struct nss_ipsec_xfrm_tunnel *tun;
	uint32_t hash_idx;

	BUG_ON((family != AF_INET) && (family != AF_INET6));

	hash_idx = nss_ipsec_xfrm_hash_tun(remote, local, drv->hash_nonce, NSS_IPSEC_XFRM_TUN_DB_MAX);
	nss_ipsec_xfrm_trace("%p: Tunnel add hash_idx %d", drv, hash_idx);

	read_lock_bh(&drv->lock);
	list_for_each_entry(tun, &drv->tun_db[hash_idx], list_entry) {
		if (nss_ipsec_xfrm_tunnel_match(tun, remote, local, family)) {
			tun = nss_ipsec_xfrm_tunnel_ref(tun);
			read_unlock_bh(&drv->lock);
			return tun;
		}
	}

	read_unlock_bh(&drv->lock);
	return NULL;
}

/*
 * nss_ipsec_xfrm_verify_offload()
 *	Verify if the XFRM state can be offloaded or not.
 */
static bool nss_ipsec_xfrm_verify_offload(struct xfrm_state *x)
{
	/*
	 * We only support Tunnel and Transport Mode ESP
	 */
	if ((x->props.mode != XFRM_MODE_TUNNEL) && (x->props.mode != XFRM_MODE_TRANSPORT)) {
		nss_ipsec_xfrm_warn("%p: xfrm_state offload not allowed: Non-Transport and Non-Tunnel\n", x);
		return false;
	}

	/*
	 * IPv6 transport mode is not supported.
	 */
	if ((x->props.family == AF_INET6) && (x->props.mode == XFRM_MODE_TRANSPORT)) {
		nss_ipsec_xfrm_warn("%p: xfrm_state offload not allowed: Non-Transport and Non-Tunnel\n", x);
		return false;
	}

	/*
	 * Unsupported ESP-over-UDP encap type
	 */
	if (x->encap && (x->encap->encap_type != UDP_ENCAP_ESPINUDP)) {
		nss_ipsec_xfrm_warn("%p: xfrm_state offload not allowed: Non ESP-over-UDP encap\n", x);
		return false;
	}

	return true;
}

/*
 * nss_ipsec_xfrm_get_dev_n_type()
 *	Get ipsecmgr tunnel netdevice and flow type.
 */
static struct net_device *nss_ipsec_xfrm_get_dev_n_type(struct net_device *kdev, struct sk_buff *skb, int32_t *type)
{
	struct nss_ipsec_xfrm_drv * drv = &g_ipsec_xfrm;
	uint8_t ip_ver = ip_hdr(skb)->version;
	struct net_device *dev = NULL;
	struct xfrm_state *x = NULL;

	nss_ipsec_xfrm_trace("%px", skb);

	BUG_ON((ip_ver != IPVERSION) && (ip_ver != 6));

	/*
	 * This is a plain text packet undergoing encap.
	 */
	x = skb_dst(skb)->xfrm;
	if (x) {
		struct nss_ipsec_xfrm_sa *sa = nss_ipsec_xfrm_sa_ref_by_state(x);
		if (!sa) {
			nss_ipsec_xfrm_trace("%p: Outbound Plain text packet(%px); not offloaded to NSS\n", sa, skb);
			return NULL;
		}

		dev = sa->tun->dev;
		dev_hold(dev);

		*type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_INNER;
		nss_ipsec_xfrm_trace("%p: Outbound packet(%px) for XFRM encap(%s)\n", sa, skb, dev->name);

		nss_ipsec_xfrm_sa_deref(sa);
		return dev;
	}

	/*
	 * dst is not xfrm. This means packet could be:-
	 * 1. Outbound encapsulated (skb->iif = ipsectunX)
	 * 2. Inbound decapsulated (skb->iif = ipsectunX)
	 * 3. Inbound encapsulated (ECM never sees such a packet)
	 * 4. None of the above(don't care for us)
	 */
	dev = dev_get_by_index(&init_net, skb->skb_iif);
	if (!dev) {
		return NULL;
	}

	/*
	 * It is an IPsec flow, but is it really bound to a tunnel managed by us.
	 */
	if (!nss_ipsec_xfrm_has_tun(drv, dev)) {
		nss_ipsec_xfrm_trace("%p: IPsec Flow not offloaded to NSS, (%px); skb_iif (%s)\n", drv, skb, dev->name);
		dev_put(dev);
		return NULL;
	}

	/*
	 * Outbound and encapuslated
	 */
	if ((ip_ver == IPVERSION) && (IPCB(skb)->flags & IPSKB_XFRM_TRANSFORMED)) {
		*type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_OUTER;
	} else if ((ip_ver == 6) && IP6CB(skb)->flags & IP6SKB_XFRM_TRANSFORMED) {
		*type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_OUTER;
	} else {
		*type = NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_INNER;
		BUG_ON(!secpath_exists(skb));
	}

	nss_ipsec_xfrm_trace("%p: Packet(%px) from ECM ip_ver %d type %d\n", drv, skb, ip_ver, *type);

	return dev;
}

/*
 * nss_ipsec_xfrm_sa_tuple2ecm_tuple()
 * 	Converts nss_ipsecmgr_sa_tuple tuple to ecm_notifier_connection_tuple tuple.
 */
static inline void nss_ipsec_xfrm_sa2ecm_tuple(struct nss_ipsecmgr_sa_tuple *sa, struct ecm_notifier_connection_tuple *tuple)
{
	tuple->dst_port = sa->dport;
	tuple->src_port = sa->sport;
	tuple->ip_ver = sa->ip_version;
	tuple->protocol = sa->proto_next_hdr;

	if (sa->ip_version == IPVERSION) {
		tuple->src.in.s_addr = sa->src_ip[0];
		tuple->dest.in.s_addr = sa->dest_ip[0];
		return;
	}

	BUG_ON(sa->ip_version != 6);

	tuple->src.in6.s6_addr32[0] = sa->src_ip[0];
	tuple->src.in6.s6_addr32[1] = sa->src_ip[1];
	tuple->src.in6.s6_addr32[2] = sa->src_ip[2];
	tuple->src.in6.s6_addr32[3] = sa->src_ip[3];

	tuple->dest.in6.s6_addr32[0] = sa->dest_ip[0];
	tuple->dest.in6.s6_addr32[1] = sa->dest_ip[1];
	tuple->dest.in6.s6_addr32[2] = sa->dest_ip[2];
	tuple->dest.in6.s6_addr32[3] = sa->dest_ip[3];
}

/*
 * nss_ipsec_xfrm_ecm_conn2flow_tuple()
 * 	Converts ecm_notifier_connection_data to nss_ipsecmgr_flow_tuple.
 */
static void nss_ipsec_xfrm_ecm_conn2tuple(struct ecm_notifier_connection_data *c, struct nss_ipsecmgr_flow_tuple *t,
					bool is_return)
{
	struct in6_addr *sip6, *dip6;
	struct in_addr *sip, *dip;

	memset(t, 0, sizeof(*t));
	t->ip_version = c->tuple.ip_ver;
	t->proto_next_hdr = c->tuple.protocol;

	if (is_return) {
		sip = &c->tuple.dest.in;
		dip = &c->tuple.src.in;
		sip6 = &c->tuple.dest.in6;
		dip6 = &c->tuple.src.in6;
		t->sport = c->tuple.dst_port;
		t->dport = c->tuple.src_port;
	} else {
		dip = &c->tuple.dest.in;
		sip = &c->tuple.src.in;
		dip6 = &c->tuple.dest.in6;
		sip6 = &c->tuple.src.in6;
		t->dport = c->tuple.dst_port;
		t->sport = c->tuple.src_port;
	}

	if (t->ip_version == IPVERSION) {
		t->dest_ip[0] = dip->s_addr;
		t->src_ip[0] = sip->s_addr;
		return;
	}

	BUG_ON(t->ip_version != 6);

	t->dest_ip[0] = dip6->s6_addr32[0];
	t->dest_ip[1] = dip6->s6_addr32[1];
	t->dest_ip[2] = dip6->s6_addr32[2];
	t->dest_ip[3] = dip6->s6_addr32[3];

	t->src_ip[0] = sip6->s6_addr32[0];
	t->src_ip[1] = sip6->s6_addr32[1];
	t->src_ip[2] = sip6->s6_addr32[2];
	t->src_ip[3] = sip6->s6_addr32[3];
}


/*
 * nss_ipsec_xfrm_ecm_conn_notify()
 *         Notifier function for ECM connection events.
 */
static int nss_ipsec_xfrm_ecm_conn_notify(struct notifier_block *nb, unsigned long event, void *ptr)
{
	struct ecm_notifier_connection_data *conn = ptr;
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;
	struct nss_ipsecmgr_flow_tuple tuple = {0};
	struct nss_ipsec_xfrm_flow *flow;
	bool is_return = false;

	nss_ipsec_xfrm_info("ECM connection event %ld\n", event);

	/*
	 * We only need to handle REMOVE events
	 */
	if (event != ECM_NOTIFIER_ACTION_CONNECTION_REMOVED) {
		nss_ipsec_xfrm_trace("%px: Unhandled event\n", nb);
		return NOTIFY_OK;
	}

	nss_ipsec_xfrm_trace("%px: Conn_tuple: ip_version %d Proto %d sport %d dport %d from_dev %s to_dev %s\n", nb,
			conn->tuple.ip_ver, conn->tuple.protocol, conn->tuple.src_port, conn->tuple.dst_port,
			conn->from_dev ? conn->from_dev->name : "", conn->to_dev ? conn->to_dev->name : "");
	/*
	 * Connection belongs to outer flow and it will be deleted
	 * when parent SA gets deref.
	 */
	switch (conn->tuple.protocol) {
	case IPPROTO_ESP:
		nss_ipsec_xfrm_trace("%px: Connection protocol is ESP, no action required\n", nb);
		return NOTIFY_OK;
	case IPPROTO_UDP:
		if (conn->tuple.src_port == NSS_IPSECMGR_NATT_PORT_DATA) {
			return NOTIFY_OK;
		}

		if (conn->tuple.dst_port == NSS_IPSECMGR_NATT_PORT_DATA) {
			return NOTIFY_OK;
		}

		break;
	default:
		break;
	}

	/*
	 * Find the direction of the flow; it is possible that this flow is
	 * not accelerated through the NSS
	 */
	is_return = !!nss_ipsec_xfrm_has_tun(drv, conn->from_dev);
	if (!is_return && !nss_ipsec_xfrm_has_tun(drv, conn->to_dev)) {
		return NOTIFY_OK;
	}

	nss_ipsec_xfrm_ecm_conn2tuple(conn, &tuple, is_return);

	flow = nss_ipsec_xfrm_ref_flow(drv, &tuple);
	if (!flow) {
		nss_ipsec_xfrm_err("%p: Flow object doesn't exist for the provided tuple", nb);
		return false;
	}

	nss_ipsec_xfrm_del_flow(drv, flow);
	nss_ipsec_xfrm_flow_deref(flow);
	return NOTIFY_OK;
}

#if defined(NSS_L2TPV2_ENABLED) || defined(NSS_VXLAN_ENABLED)
/*
 * nss_ipsec_xfrm_get_encap_ifnum_by_ip()
 *»       Get ipsec tunnel NSS inner interface number.
 */
static int32_t nss_ipsec_xfrm_get_inner_ifnum_by_ip(uint8_t ip_version, uint32_t *src_ip, uint32_t *dest_ip)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;
	struct nss_ipsec_xfrm_tunnel *tun;
	xfrm_address_t remote = {0};
	xfrm_address_t local = {0};
	int32_t if_num;
	uint8_t family;

	switch (ip_version) {
	case IPVERSION:
		family = AF_INET;
		remote.a4 = *dest_ip;
		local.a4 = *src_ip;
		nss_ipsec_xfrm_trace("%px: src_ip %pxI4 %pxI4", drv, src_ip, dest_ip);
		break;

	case 6:
		family = AF_INET6;
		memcpy(&remote.in6, dest_ip, sizeof(struct in6_addr));
		memcpy(&local.in6, src_ip, sizeof(struct in6_addr));
		nss_ipsec_xfrm_trace("%px: src_ip %pxI6 %pxI6", drv, src_ip, dest_ip);
		break;

	default:
		nss_ipsec_xfrm_warn("%px: invalid IP version:%u ", drv, ip_version);
		return -1;
	}

	tun = nss_ipsec_xfrm_ref_tun(drv, &local, &remote, family);
	if (!tun) {
		nss_ipsec_xfrm_warn("%px: No IPSec tunnel for the given SIP/DIP", drv);
		return -1;
	}

	if_num = nss_cmn_get_interface_number_by_dev_and_type(tun->dev, NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_INNER);
	nss_ipsec_xfrm_trace("%px: Tunnel %s Inner ifnum %d", tun, tun->dev->name, if_num);

	nss_ipsec_xfrm_tunnel_deref(tun);
	return if_num;
}
#endif

/*
 * nss_ipsec_xfrm_state_acquire()
 *	xfrm km acquire operation function. No-op for the plugin.
 */
static int nss_ipsec_xfrm_state_acquire(struct xfrm_state *x, struct xfrm_tmpl *t, struct xfrm_policy *xp)
{
	nss_ipsec_xfrm_info("%p: No-Op\n", x);
	return -1;
}

/*
 * nss_ipsec_xfrm_policy_compile()
 *	xfrm km compile_policy operation function. No-op for the plugin.
 */
static struct xfrm_policy *nss_ipsec_xfrm_policy_compile(struct sock *sk, int opt, u8 *data, int len, int *dir)
{
	nss_ipsec_xfrm_info("No-Op\n");
	return NULL;
}

/*
 * nss_ipsec_xfrm_policy_notify()
 *	xfrm km xfrm_policy database change notification function.
 */
static int nss_ipsec_xfrm_policy_notify(struct xfrm_policy *xp, int dir, const struct km_event *c)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	if (xp && (xp->type != XFRM_POLICY_TYPE_MAIN)) {
		return 0;
	}

	nss_ipsec_xfrm_info("%p: XFRM event %d hard %d\n", xp, c->event, c->data.hard);

	if (c->event == XFRM_MSG_FLUSHPOLICY) {
		nss_ipsec_xfrm_flush_flow_all(drv);
	}

        return 0;
}

/*
 * nss_ipsec_xfrm_state_notify()
 *	xfrm km xfrm_state database change notification function.
 */
static int nss_ipsec_xfrm_state_notify(struct xfrm_state *x, const struct km_event *c)
{
	int refcnt = -1;

	if (x) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		refcnt = atomic_read(&x->refcnt);
#else
		refcnt = refcount_read(&x->refcnt);
#endif
	}

	nss_ipsec_xfrm_info("%p(%d): event %d hard %d\n", x, refcnt, c->event, c->data.hard);
	return 0;
}

/*
 * nss_ipsec_xfrm_offload_encap()
 *	Sends an outbound packet to NSS for encapsulation and encryption.
 */
static int nss_ipsec_xfrm_offload_encap(struct nss_ipsec_xfrm_sa *sa, struct net *net, struct xfrm_state *x,
					struct sk_buff *skb, uint16_t family)
{
	struct nss_ipsec_xfrm_tunnel *tun = sa->tun;
	struct nss_ipsecmgr_flow_tuple tuple = {0};
	struct nss_ipsec_xfrm_drv *drv = sa->drv;
	struct nss_ipsec_xfrm_flow *flow = NULL;
	struct net_device *dev = tun->dev;
	nss_ipsecmgr_status_t status;
	struct sk_buff *skb2;
	int err = -EINVAL;

	/*
	 * Linearization is by-product of skb_copy_expand()
	 */
	skb2 = skb_copy_expand(skb, dev->needed_headroom, dev->needed_tailroom, GFP_ATOMIC);
	if (!skb2) {
		nss_ipsec_xfrm_err("%px: Failed to offload encap; copy_expand failed\n", skb);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTERROR);
		return -ENOMEM;
	}

	nss_ipsec_xfrm_flow_hdr2tuple(skb2, 0, &tuple);

	/*
	 * Check if ECM has pushed the rule. If present then push the inner flow rule.
	 * Enable the IPsec inner flow once ECM has accelerated the outer routing
	 * flow rule. This will limit the packet rate in encap direction till ECM
	 * pushes the rule and should minimize the out-of-order issue when ECM
	 * flow is not present.
	 */
	if (!atomic_read(&sa->ecm_accel_outer)) {
		struct ecm_notifier_connection_tuple ecm_tuple = {0};
		enum ecm_notifier_connection_state ecm_state;

		nss_ipsec_xfrm_sa2ecm_tuple(&sa->tuple, &ecm_tuple);

		ecm_state = ecm_notifier_connection_state_get(&ecm_tuple);
		if (ecm_state != ECM_NOTIFIER_CONNECTION_STATE_ACCEL) {
			goto done;
		}

		atomic_set(&sa->ecm_accel_outer, 1);
		nss_ipsec_xfrm_trace("%px/%px: SA %p, Get ecm connection state(%u)\n", skb, skb2, sa, ecm_state);
	}

	flow = nss_ipsec_xfrm_ref_flow(drv, &tuple);
	if (flow) {
		nss_ipsec_xfrm_flow_update(flow, sa);
		nss_ipsec_xfrm_flow_deref(flow);
		goto done;
	}

	/*
	 * Flow does not exist: Allocate and add a new flow
	 */
	flow = nss_ipsec_xfrm_flow_alloc(drv, &tuple, sa);
	if (!flow) {
		nss_ipsec_xfrm_err("%px/%px: Failed to allocate new flow object\n", skb, skb2);
		dev_kfree_skb_any(skb2);
		return -EINVAL;
	}

	nss_ipsec_xfrm_add_flow(drv, flow);

done:
	/*
	 * Manually compute the checksum.
	 */
	if (!skb_is_gso(skb2) && (skb2->ip_summed == CHECKSUM_PARTIAL)) {
		err = skb_checksum_help(skb2);
		if (err) {
			nss_ipsec_xfrm_err("%px/%px: Failed to offload encap; no csum\n", skb, skb2);
			goto drop;
		}
	}

	skb_scrub_packet(skb2, true);

	status = nss_ipsecmgr_sa_tx_inner(sa->tun->dev, &sa->tuple, skb2);
	if (status != NSS_IPSECMGR_OK) {
		nss_ipsec_xfrm_err("%px: Failed to offload encap: unable to tx(%px) IPsec manager\n", sa, skb);
		goto drop;
	}

	/*
	 * Success; consume the original skb.
	 */
	consume_skb(skb);
	return 0;
drop:
	XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTERROR);
	dev_kfree_skb_any(skb2);
	return err;
}

/*
 * nss_ipsec_xfrm_offload_decap()
 *	Sends a inbound packet to NSS for decapsulation and decryption.
 *	Returns 0 if the packet is queued to NSS.
 */
static bool nss_ipsec_xfrm_offload_decap(struct nss_ipsec_xfrm_sa *sa, struct net *net, struct xfrm_state *x,
				struct sk_buff *skb, uint16_t family, bool natt)
{
	struct nss_ipsecmgr_flow_tuple tuple = {0};
	struct nss_ipsec_xfrm_drv *drv = sa->drv;
	struct nss_ipsec_xfrm_flow *flow = NULL;
	nss_ipsecmgr_status_t status;
	struct iphdr *iph = NULL;

	/*
	 * In case of ESP-over-UDP, xfrm4_udp_encap_rcv() reduces the iph total length
	 * by size of the udp header. Undo the same.
	 */
	if (natt) {
		iph = ip_hdr(skb);
		iph->tot_len = htons(ntohs(iph->tot_len) + sizeof(struct udphdr));
	}

	/*
	 * In case of Decap, skb->data points to ESP header or UDP header for NATT.
	 * Set the skb->data to outer IP header.
	 */
	skb_push(skb, skb->data - skb_network_header(skb));

	/*
	 * Set the transport header
	 */
	if (family == AF_INET) {
		skb_set_transport_header(skb, sizeof(struct iphdr));
	} else {
		skb_set_transport_header(skb, sizeof(struct ipv6hdr));
	}

	/*
	 * If skb was cloned (likely due to a packet sniffer), Make a copy.
	 * skb_cow() has check for skb_cloned().
	 */
	if (skb_cow(skb, skb_headroom(skb))) {
		nss_ipsec_xfrm_err("%px: Failed to offload; create writable copy failed: Drop", skb);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMINERROR);
		return false;
	}

	nss_ipsec_xfrm_flow_hdr2tuple(skb, natt, &tuple);

	flow = nss_ipsec_xfrm_ref_flow(drv, &tuple);
	if (flow) {
		nss_ipsec_xfrm_flow_update(flow, sa);
		nss_ipsec_xfrm_flow_deref(flow);
		goto done;
	}

	/*
	 * Flow does not exist: Allocate and add a new flow
	 */
	flow = nss_ipsec_xfrm_flow_alloc(drv, &tuple, sa);
	if (!flow) {
		nss_ipsec_xfrm_err("%px: Failed to allocate new flow object\n", skb);
		return false;
	}

	nss_ipsec_xfrm_add_flow(drv, flow);

done:
	/*
	 * We need to release all resources hold by SKB before sending it to NSS.
	 * Reset the SKB and make it orphan.
	 */
	skb_scrub_packet(skb, true);

	/*
	 * Offload further processing to NSS.
	 * Note: This can fail if the queue is full
	 */
	status = nss_ipsecmgr_sa_tx_outer(sa->tun->dev, &sa->tuple, skb);
	if (status != NSS_IPSECMGR_OK) {
		nss_ipsec_xfrm_err("%px: Failed to offload; IPsec manager tx_outer failed\n", sa);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMINERROR);
		return false;
	}

	return true;
}

/*
 * nss_ipsec_xfrm_esp_rcv()
 * 	Handle a recieved ESP packet.
 */
static int nss_ipsec_xfrm_esp_rcv(struct nss_ipsec_xfrm_drv *drv, struct sk_buff *skb, xfrm_address_t *daddr,
				unsigned int family, bool natt)
{
	struct net *net = dev_net(skb->dev);
	struct nss_ipsec_xfrm_sa *sa = NULL;
	struct xfrm_state *x;
	 __be32 seq = 0;
	 __be32 spi;
	int err;

	err = xfrm_parse_spi(skb, IPPROTO_ESP, &spi, &seq);
	if (err) {
		nss_ipsec_xfrm_err("%px: Failed to handle esp packet; Cannot parse esp hdr: Drop\n", skb);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMINHDRERROR);
		goto drop;
	}

	/*
	 * If, XFRM state is found then we will proceed with the receive processing of the packet
	 */
	x = xfrm_state_lookup(net, skb->mark, daddr, spi, IPPROTO_ESP, family);
	if (!x) {
		nss_ipsec_xfrm_err("%px: Failed to handle esp packet; xfrm_state lookup failed: Drop\n", skb);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMINNOSTATES);
		goto drop;
	}

	/*
	 * Check if the XFRM state is mapped for offload; if not then drop.
	 * Otherwise, consume the packet for decapsulation
	 */
	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	if (!sa) {
		nss_ipsec_xfrm_trace("%px: xfrm_state is not managed by NSS; Drop\n", skb);
		xfrm_state_put(x);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMINSTATEINVALID);
		goto drop;
	}

	nss_ipsec_xfrm_trace("%p: Inbound packet(%px) for xfrm_state=%p\n", sa, skb, x);

	if (!nss_ipsec_xfrm_offload_decap(sa, net, x, skb, family, natt)) {
		nss_ipsec_xfrm_err("%p: Failed to decap ESP packet(%px); xfrm_state=%p\n", sa, skb, x);
		nss_ipsec_xfrm_sa_deref(sa);
		xfrm_state_put(x);
		goto drop;
	}

	nss_ipsec_xfrm_sa_deref(sa);
	xfrm_state_put(x);

	atomic64_inc(&drv->stats.decap_nss);
	return 0;
drop:
	atomic64_inc(&drv->stats.decap_drop);
	dev_kfree_skb_any(skb);
	return 0;
}

/*
 * nss_ipsec_xfrm_encap_v4()
 *	This is called for IPv4 pakcets that are to be transformed.
 */
static int nss_ipsec_xfrm_encap_v4(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct xfrm_state *x = skb_dst(skb)->xfrm;
	struct nss_ipsec_xfrm_sa *sa;
	int ret = 0;

#if defined CONFIG_NETFILTER
	if (!x) {
		IPCB(skb)->flags |= IPSKB_REROUTED;
		return dst_output(net, sk, skb);
	}
#endif
	/*
	 * This skb is going to NSS, hold an additional ref to xfrm_state
	 */
	xfrm_state_hold(x);

	/*
	 * This was already checked in nss_ipsec_xfrm_v6_encap()
	 */
	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	BUG_ON(!sa);

	ret = nss_ipsec_xfrm_offload_encap(sa, net, x, skb, AF_INET);
	if (ret < 0) {
		atomic64_inc(&sa->drv->stats.encap_drop);
		nss_ipsec_xfrm_sa_deref(sa);
		xfrm_state_put(x);
		dev_kfree_skb_any(skb);
		return ret;
	}

	atomic64_inc(&sa->drv->stats.encap_nss);
	nss_ipsec_xfrm_sa_deref(sa);
	xfrm_state_put(x);
	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
/*
 * nss_ipsec_xfrm_v4_init_flags()
 *	Initialize xfrm state flags
 */
static int nss_ipsec_xfrm_v4_init_flags(struct xfrm_state *x)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", x);
	return drv->xsa.v4->init_flags(x);
}

/*
 * nss_ipsec_xfrm_v4_init_sel()
 *	Initialize xfrm state selector
 */
static void nss_ipsec_xfrm_v4_init_sel(struct xfrm_selector *sel, const struct flowi *fl)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", sel);
	return drv->xsa.v4->init_tempsel(sel, fl);
}

/*
 * nss_ipsec_xfrm_v4_init_param()
 *	Initialize xfrm state parameters
 */
static void nss_ipsec_xfrm_v4_init_param(struct xfrm_state *x, const struct xfrm_tmpl *tmpl,
		const xfrm_address_t *daddr, const xfrm_address_t *saddr)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", x);
	return drv->xsa.v4->init_temprop(x, tmpl, daddr, saddr);
}

/*
 * nss_ipsec_xfrm_v6_init_sel()
 *	Initialize xfrm state selector
 */
static void nss_ipsec_xfrm_v6_init_sel(struct xfrm_selector *sel, const struct flowi *fl)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", sel);
	return drv->xsa.v6->init_tempsel(sel, fl);
}

/*
 * nss_ipsec_xfrm_v6_init_param()
 *	Initialize xfrm state parameters
 */
static void nss_ipsec_xfrm_v6_init_param(struct xfrm_state *x, const struct xfrm_tmpl *tmpl,
		const xfrm_address_t *daddr, const xfrm_address_t *saddr)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", x);
	return drv->xsa.v6->init_temprop(x, tmpl, daddr, saddr);
}

/*
 * nss_ipsec_xfrm_v6_sort_tmpl()
 *	Distribution couting sort for xfrm state template
 */
static int nss_ipsec_xfrm_v6_sort_tmpl(struct xfrm_tmpl **dst, struct xfrm_tmpl **src, int n)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", dst);
	return drv->xsa.v6->tmpl_sort(dst, src, n);
}

/*
 * nss_ipsec_xfrm_v6_sort_state()
 *	Distribution couting sort for xfrm state
 */
static int nss_ipsec_xfrm_v6_sort_state(struct xfrm_state **dst, struct xfrm_state **src, int n)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", dst);
	return drv->xsa.v6->state_sort(dst, src, n);
}
#endif

/*
 * nss_ipsec_xfrm_v4_output()
 *	Called for IPv4 Plain text packets submitted for IPSec transformation.
 */
static int nss_ipsec_xfrm_v4_output(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;
	struct xfrm_state *x = skb_dst(skb)->xfrm;
	struct nss_ipsec_xfrm_sa *sa = NULL;
	struct nss_ipsec_xfrm_tunnel *tun;
	int ret = 0;

	/*
	 * No xfrm_state associated; Drop
	 */
	if (!x) {
		nss_ipsec_xfrm_warn("%px: Failed to offload; No xfrm_state associated: drop\n", skb);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTNOSTATES);
		goto drop;
	}

	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	if (!sa) {
		nss_ipsec_xfrm_warn("%px: Failed to offload; xfrm_state %p, Flow not offloaded: drop\n", skb, x);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEINVALID);
		goto drop;
	}

	tun = sa->tun;
	BUG_ON(!tun);

	/*
	 * No falling back now. xfrm_state is offloaded to NSS.
	 */
	nss_ipsec_xfrm_trace("%px: Outbound packet trapped for encap; xfrm_state %p SA %p\n", skb, x, sa);

	/*
	 * Call the Post routing hooks.
	 */
	ret = NF_HOOK_COND(NFPROTO_IPV4, NF_INET_POST_ROUTING, net, sk, skb, NULL, skb_dst(skb)->dev,
			nss_ipsec_xfrm_encap_v4, !(IPCB(skb)->flags & IPSKB_REROUTED));

	nss_ipsec_xfrm_sa_deref(sa);
	return ret;

drop:
	atomic64_inc(&drv->stats.encap_drop);
	dev_kfree_skb_any(skb);
	return -EINVAL;
}

/*
 * nss_ipsec_xfrm_v4_output_finish()
 *	This is called for non-offloaded transformations after the NF_POST routing hooks
 *	for packets that are to-be transformed with an outer IPv4 header.
 *	Redirect to the native function.
 */
static int nss_ipsec_xfrm_v4_output_finish(struct sock *sk, struct sk_buff *skb)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", skb);
	return drv->xsa.v4->output_finish(sk, skb);
}

/*
 * nss_ipsec_xfrm_v4_extract_input()
 *	This is called for non-offloaded transformations, for an inbound transformed packet to
 *	extract the outer IPv4 header. Redirect to the native function.
 */
static int nss_ipsec_xfrm_v4_extract_input(struct xfrm_state *x, struct sk_buff *skb)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", skb);
	return drv->xsa.v4->extract_input(x, skb);
}

/*
 * nss_ipsec_xfrm_v4_extract_output()
 *	This is called for non-offloaded transformations, for an outbound to-be
 *	transformed packet for extracting the inner IPv4 header.
 *	Redirect to the native function.
 */
static int nss_ipsec_xfrm_v4_extract_output(struct xfrm_state *x, struct sk_buff *skb)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", skb);
	return drv->xsa.v4->extract_output(x, skb);
}

/*
 * nss_ipsec_xfrm_v4_transport_finish()
 * 	This is called for non-offloaded transformations, for transport mode packets decapsulated
 * 	with an inner IPv4 header. Redirect to native function
 */
static int nss_ipsec_xfrm_v4_transport_finish(struct sk_buff *skb, int async)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", skb);
	return drv->xsa.v4->transport_finish(skb, async);
}

/*
 * nss_ipsec_xfrm_v4_local_error()
 * 	This is called for non-offloaded transformations to generate local pmtu errors.
 * 	Redirect to native function
 */
void nss_ipsec_xfrm_v4_local_error(struct sk_buff *skb, u32 mtu)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", skb);
	drv->xsa.v4->local_error(skb, mtu);

}

/*
 * nss_ipsec_xfrm_esp_get_mtu()
 * 	Get mtu for inner packet.
 */
uint32_t nss_ipsec_xfrm_esp_get_mtu(struct xfrm_state *x, int mtu)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;
	uint32_t (*fallback_fn)(struct xfrm_state *, int);
	struct nss_ipsec_xfrm_sa *sa;
	size_t ip_len;

	if (x->props.family == AF_INET) {
		ip_len = sizeof(struct iphdr);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		fallback_fn = drv->xsa.v4->type_map[IPPROTO_ESP]->get_mtu;
#else
		fallback_fn = drv->xsa.v4->type_esp->get_mtu;
#endif
	} else {
		ip_len = sizeof(struct ipv6hdr);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		fallback_fn = drv->xsa.v6->type_map[IPPROTO_ESP]->get_mtu;
#else
		fallback_fn = drv->xsa.v6->type_esp->get_mtu;
#endif
	}

	/*
	 * We are not managing this xfrm_state
	 * Fallback
	 */
	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	if (!sa) {
		nss_ipsec_xfrm_info("%p: xfrm_state is not owned by NSS: Fallback\n", x);
		goto fallback;
	}

	/*
	 * We don't need ip_len for tunnel mode to compute the effective MTU
	 */
	if (x->props.mode != XFRM_MODE_TRANSPORT) {
		ip_len = 0;
	}

	mtu -= sa->sa_info.hdr_len;
	mtu -= sa->sa_info.hash_len;
	mtu -= ip_len;

	mtu = round_down(mtu, sa->sa_info.blk_len);
	mtu += ip_len;
	mtu -= 2; /* Trailer length is fixed to 2 bytes */

	nss_ipsec_xfrm_sa_deref(sa);

	nss_ipsec_xfrm_trace("mtu %d", mtu);
	return mtu;

fallback:
	if (fallback_fn) {
		return fallback_fn(x, mtu);
	}

	nss_ipsec_xfrm_err("%p: Fallback failed", x);
	return mtu;
}

/*
 * nss_ipsec_xfrm_esp_init_state()
 * 	Initialize IPsec xfrm state of type ESP.
 */
static int nss_ipsec_xfrm_esp_init_state(struct xfrm_state *x)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;
	struct nss_ipsec_xfrm_tunnel *tun = NULL;
	struct nss_ipsec_xfrm_sa *sa = NULL;
	xfrm_address_t remote = {0};
	xfrm_address_t local = {0};
	struct net_device *local_dev;
	bool new_tun = 0;
	size_t ip_addr_len;

	if (x->props.family == AF_INET) {
		local_dev = ip_dev_find(&init_net, x->id.daddr.a4);
		ip_addr_len = sizeof(local.a4);
	} else {
		local_dev = ipv6_dev_find(&init_net, &x->id.daddr.in6, 1);
		ip_addr_len = sizeof(local.a6);
	}

	/*
	 * Local device is found for the decapsulation direction;
	 * Since, we store the tunnel object using the encap direction
	 * we have to swap the address before the tunnel allocation
	 */
	if (local_dev) {
		memcpy(&local, &x->id.daddr, ip_addr_len);
		memcpy(&remote, &x->props.saddr, ip_addr_len);
		dev_put(local_dev);
	} else {
		memcpy(&local, &x->props.saddr, ip_addr_len);
		memcpy(&remote, &x->id.daddr, ip_addr_len);
	}

	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	if (sa) {
		nss_ipsec_xfrm_warn("%p: xfrm_state is already offloaded %p\n", sa, x);
		nss_ipsec_xfrm_sa_deref(sa);
		WARN_ON(1);
		return -EEXIST;
	}

	if (!nss_ipsec_xfrm_verify_offload(x)) {
		nss_ipsec_xfrm_warn("%p: XFRM state offloaded is not allowed\n", x);
		return -ENOTSUPP;

	}

	tun = nss_ipsec_xfrm_ref_tun(drv, &local, &remote, x->props.family);
	if (!tun) {
		nss_ipsec_xfrm_info("%p: Tunnel doesn't exist for this xfrm_state; create one\n", x);

		/*
		 * We are having the rtnl_lock here(). Hence, we can do the below
		 * without worying about locking, as there is no parallel entity
		 * that could create a tunnel.
		 */
		tun = nss_ipsec_xfrm_tunnel_alloc(drv, &local, &remote, x->props.family);
		if (!tun) {
			nss_ipsec_xfrm_err("%p: Failed to offload xfrm_state; Tunnel alloc failed: Fail\n", x);
			return -EINVAL;
		}

		new_tun = true;
		nss_ipsec_xfrm_tunnel_ref(tun);
	}

	/*
	 * Allocate the new SA for the corresponding the XFRM state
	 * Note: We store the SA inside the XFRM state and it holds a
	 * reference to our SA object
	 */
	sa = nss_ipsec_xfrm_sa_alloc(tun, x);
	if (!sa) {
		nss_ipsec_xfrm_err("%p: Failed to offload xfrm_state; SA alloc failed: Fail\n", x);
		goto error;
	}

	if (new_tun) {
		nss_ipsec_xfrm_add_tun(drv, tun);
	}

	atomic_inc(&tun->num_sa);
	nss_ipsec_xfrm_tunnel_deref(tun);
	return 0;

error:
	if (new_tun) {
		nss_ipsec_xfrm_tunnel_dealloc(tun);
	}

	nss_ipsec_xfrm_tunnel_deref(tun);
	return -EINVAL;
}

/*
 * nss_ipsec_xfrm_esp_deinit_state()
 * 	Destroy IPsec xfrm state of type ESP.
 */
static void nss_ipsec_xfrm_esp_deinit_state(struct xfrm_state *x)
{
	nss_ipsec_xfrm_trace("%p: xfrm_state destroyed\n", x);
	nss_ipsec_xfrm_sa_deinit(x);
	return;
}

/*
 * nss_ipsec_xfrm_esp_input()
 * 	Invoked by stack to handover the packet for decryption.
 * 	This should not be called for NSS managed ESP xfrm_states.
 * 	Redirect to the native version.
 */
static int nss_ipsec_xfrm_esp_input(struct xfrm_state *x, struct sk_buff *skb)
{
	int (*fallback_fn)(struct xfrm_state *, struct sk_buff *);
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	if (x->props.family == AF_INET) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		fallback_fn = drv->xsa.v4->type_map[IPPROTO_ESP]->input;
#else
		fallback_fn = drv->xsa.v4->type_esp->input;
#endif
	} else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		fallback_fn = drv->xsa.v6->type_map[IPPROTO_ESP]->input;
#else
		fallback_fn = drv->xsa.v6->type_esp->input;
#endif
	}

	nss_ipsec_xfrm_trace("%px: Redirect to native stack\n", skb);
	return fallback_fn(x, skb);
}

/*
 * nss_ipsec_xfrm_esp_output()
 * 	Invoked by stack to handover the packet for encryption,
 * 	after it has ben encapsulated.
 * 	This should not be called for NSS managed ESP xfrm_states.
 * 	Redirect to the native version.
 */
static int nss_ipsec_xfrm_esp_output(struct xfrm_state *x, struct sk_buff *skb)
{
	int (*fallback_fn)(struct xfrm_state *, struct sk_buff *);
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	if (x->props.family == AF_INET) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		fallback_fn = drv->xsa.v4->type_map[IPPROTO_ESP]->output;
#else
		fallback_fn = drv->xsa.v4->type_esp->output;
#endif
	} else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		fallback_fn = drv->xsa.v6->type_map[IPPROTO_ESP]->output;
#else
		fallback_fn = drv->xsa.v6->type_esp->output;
#endif
	}

	nss_ipsec_xfrm_trace("%px: Redirect to native stack\n", skb);
	return fallback_fn(x, skb);
}

/*
 * nss_ipsec_xfrm_esp4_rcv()
 * 	Handle a received IPv4 ESP received packet.
 */
static int nss_ipsec_xfrm_esp4_rcv(struct sk_buff *skb)
{
	struct iphdr *iph = (struct iphdr *)skb_network_header(skb);
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	return nss_ipsec_xfrm_esp_rcv(drv, skb, (xfrm_address_t *)&iph->daddr, AF_INET, false);
}

/*
 * nss_ipsec_xfrm_udp_rcv()
 * 	Handle a received IPv4 ESPinUDP received packet.
 */
static int nss_ipsec_xfrm_udp_rcv(struct sk_buff *skb, int nexthdr, __be32 spi, int encap_type)
{
	struct iphdr *iph = (struct iphdr *)skb_network_header(skb);
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	return nss_ipsec_xfrm_esp_rcv(drv, skb, (xfrm_address_t *)&iph->daddr, AF_INET, true);
}

/*
 * nss_ipsec_xfrm_esp4_rcv_err()
 * 	Invoked after an ESP packet has been decrypted.
 * 	This should not be called for NSS managed ESP xfrm_states.
 * 	Redirect to the native handler.
 */
static int nss_ipsec_xfrm_esp4_rcv_err(struct sk_buff *skb, int err)
{
	nss_ipsec_xfrm_trace("%px: Redirect to Native ESP handler\n", skb);
	return 1;
}

/*
 * nss_ipsec_xfrm_esp4_err()
 * 	Invoked for a received ICMP error for ESP.
 */
static int nss_ipsec_xfrm_esp4_err(struct sk_buff *skb, u32 info)
{
	struct net *net = dev_net(skb->dev);
	struct nss_ipsec_xfrm_sa *sa;
	struct ip_esp_hdr *esph;
	struct xfrm_state *x;
	struct iphdr *iph;

	nss_ipsec_xfrm_trace("%px:\n", skb);

	iph = (struct iphdr *)skb->data;
	esph = (struct ip_esp_hdr *) ((uint8_t *)iph + sizeof(*iph));

	x = xfrm_state_lookup(net, skb->mark, (xfrm_address_t *)&iph->daddr, esph->spi, IPPROTO_ESP, AF_INET);
	if (!x) {
		nss_ipsec_xfrm_err("%px: xfrm_lookup failed: Done\n", skb);
		return 0;
	}

	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	if (!sa) {
		nss_ipsec_xfrm_trace("%px: xfrm_state %p is not owned by NSS: Fallback\n", skb, x);
		xfrm_state_put(x);
		return -EINVAL;
	}

	nss_ipsec_xfrm_sa_deref(sa);

	switch (icmp_hdr(skb)->type) {
	case ICMP_DEST_UNREACH:
		if (icmp_hdr(skb)->code != ICMP_FRAG_NEEDED) {
			xfrm_state_put(x);
			return 0;
		}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		ipv4_update_pmtu(skb, net, info, 0, 0, IPPROTO_ESP, 0);
#else
		ipv4_update_pmtu(skb, net, info, 0, IPPROTO_ESP);
#endif
		break;

	case ICMP_REDIRECT:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		ipv4_redirect(skb, net, 0, 0, IPPROTO_ESP, 0);
#else
		ipv4_redirect(skb, net, 0, IPPROTO_ESP);
#endif
		break;

	default:
		xfrm_state_put(x);
		return 0;
	}

	xfrm_state_put(x);
	return 0;
}

/*
 * nss_ipsec_xfrm_encap_v6()
 *	This is called for IPv6 pakcets that are to be transformed.
 */
static int nss_ipsec_xfrm_encap_v6(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct xfrm_state *x = skb_dst(skb)->xfrm;
	struct nss_ipsec_xfrm_sa *sa;
	int ret = 0;

#if defined CONFIG_NETFILTER
	if (!x) {
		IP6CB(skb)->flags |= IP6SKB_REROUTED;
		return dst_output(net, sk, skb);
	}
#endif
	/*
	 * This skb is going to NSS, hold an additional ref to xfrm_state
	 */
	xfrm_state_hold(x);

	/*
	 * This was already checked in nss_ipsec_xfrm_encap_v4()
	 */
	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	BUG_ON(!sa);

	ret = nss_ipsec_xfrm_offload_encap(sa, net, x, skb, AF_INET6);
	if (ret < 0) {
		atomic64_inc(&sa->drv->stats.encap_drop);
		nss_ipsec_xfrm_sa_deref(sa);
		xfrm_state_put(x);
		dev_kfree_skb_any(skb);
		return ret;
	}

	atomic64_inc(&sa->drv->stats.encap_nss);
	nss_ipsec_xfrm_sa_deref(sa);
	xfrm_state_put(x);
	return 0;
}

/*
 * nss_ipsec_xfrm_v6_output()
 *	Called for IPv6 Plain text packets submitted for IPSec transformation.
 */
static int nss_ipsec_xfrm_v6_output(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;
	struct xfrm_state *x = skb_dst(skb)->xfrm;
	struct nss_ipsec_xfrm_sa *sa = NULL;
	struct nss_ipsec_xfrm_tunnel *tun;
	int ret = 0;

	/*
	 * No xfrm_state associated; Drop
	 */
	if (!x) {
		nss_ipsec_xfrm_warn("%px: Failed to offload; No xfrm_state associated: drop\n", skb);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTNOSTATES);
		goto drop;
	}

	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	if (!sa) {
		nss_ipsec_xfrm_warn("%px: Failed to offload; xfrm_state %p, Flow not offloaded: drop\n", skb, x);
		XFRM_INC_STATS(net, LINUX_MIB_XFRMOUTSTATEINVALID);
		goto drop;
	}

	tun = sa->tun;
	BUG_ON(!tun);

	/*
	 * No falling back now. xfrm_state is offloaded to NSS.
	 */
	nss_ipsec_xfrm_trace("%px: Outbound packet trapped for encap; xfrm_state %p SA %p\n", skb, x, sa);

	/*
	 * Call the Post routing hooks.
	 */
	ret = NF_HOOK_COND(NFPROTO_IPV6, NF_INET_POST_ROUTING, net, sk, skb, NULL, skb_dst(skb)->dev,
			nss_ipsec_xfrm_encap_v6, !(IP6CB(skb)->flags & IP6SKB_REROUTED));

	nss_ipsec_xfrm_sa_deref(sa);
	return ret;

drop:
	atomic64_inc(&drv->stats.encap_drop);
	dev_kfree_skb_any(skb);
	return -EINVAL;
}

/*
 * nss_ipsec_xfrm_v6_output_finish()
 *	This is called for non-offloaded transformations after the NF_POST routing hooks
 *	for packets that are to-be transformed with an outer IPv6 header.
 *	Redirect to the native function.
 */
static int nss_ipsec_xfrm_v6_output_finish(struct sock *sk, struct sk_buff *skb)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", skb);
	return drv->xsa.v6->output_finish(sk, skb);
}

/*
 * nss_ipsec_xfrm_v6_extract_input()
 *	This is called for non-offloaded transformations, for an inbound transformed packet to
 *	extract the outer IPv6 header. Redirect to the native function.
 */
static int nss_ipsec_xfrm_v6_extract_input(struct xfrm_state *x, struct sk_buff *skb)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", skb);
	return drv->xsa.v6->extract_input(x, skb);
}

/*
 * nss_ipsec_xfrm_v6_extract_output()
 *	This is called for non-offloaded transformations, for an outbound to-be
 *	transformed packet for extracting the inner IPv6 header.
 *	Redirect to the native function.
 */
static int nss_ipsec_xfrm_v6_extract_output(struct xfrm_state *x, struct sk_buff *skb)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", skb);
	return drv->xsa.v6->extract_output(x, skb);
}

/*
 * nss_ipsec_xfrm_v6_transport_finish()
 * 	This is called for non-offloaded transformations, for transport mode packets decapsulated
 * 	with an inner IPv6 header. Redirect to native function
 */
static int nss_ipsec_xfrm_v6_transport_finish(struct sk_buff *skb, int async)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", skb);
	return drv->xsa.v6->transport_finish(skb, async);
}

/*
 * nss_ipsec_xfrm_v6_local_error()
 * 	This is called for non-offloaded transformations to generate local pmtu errors.
 * 	Redirect to native function
 */
void nss_ipsec_xfrm_v6_local_error(struct sk_buff *skb, u32 mtu)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native xfrm stack\n", skb);
	return drv->xsa.v6->local_error(skb, mtu);
}

/*
 * nss_ipsec_xfrm_v6_esp_hdr_offset()
 * 	Invoked by stack for IPv6 transport mode in encap.
 * 	Redirect to the native version.
 */
static  int nss_ipsec_xfrm_v6_esp_hdr_offset(struct xfrm_state *x, struct sk_buff *skb, u8 **prevhdr)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	nss_ipsec_xfrm_trace("%px: Redirect to native esp6 stack\n", skb);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	return drv->xsa.v6->type_map[IPPROTO_ESP]->hdr_offset(x, skb, prevhdr);
#else
	return drv->xsa.v6->type_esp->hdr_offset(x, skb, prevhdr);
#endif
}

/*
 * nss_ipsec_xfrm_esp6_rcv()
 * 	Handle a received IPv6 ESP received packet.
 */
static int nss_ipsec_xfrm_esp6_rcv(struct sk_buff *skb)
{
	struct ipv6hdr *ip6h = (struct ipv6hdr *)skb_network_header(skb);
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;

	return nss_ipsec_xfrm_esp_rcv(drv, skb, (xfrm_address_t *)&ip6h->daddr, AF_INET6, false);
}

/*
 * nss_ipsec_xfrm_esp6_rcv_err()
 * 	Invoked after an ESP packet has been decrypted.
 * 	This should not be called for NSS managed ESP xfrm_states.
 * 	Redirect to the native handler.
 */
static int nss_ipsec_xfrm_esp6_rcv_err(struct sk_buff *skb, int err)
{
	nss_ipsec_xfrm_trace("%px: Redirect to Native ESP handler; err(%d)\n", skb, err);
	return 1;
}

/*
 * nss_ipsec_xfrm_esp6_err()
 * 	Invoked for a received ICMP error for ESP.
 */
static int nss_ipsec_xfrm_esp6_err(struct sk_buff *skb, struct inet6_skb_parm *opt, u8 type, u8 code, int offset,
					__be32 info)
{
	const struct ipv6hdr *iph = (const struct ipv6hdr *)skb->data;
	struct ip_esp_hdr *esph = (struct ip_esp_hdr *)(skb->data + offset);
	struct net *net = dev_net(skb->dev);
	struct nss_ipsec_xfrm_sa *sa;
	struct xfrm_state *x;

	nss_ipsec_xfrm_trace("%px:\n", skb);

	if (type != ICMPV6_PKT_TOOBIG && type != NDISC_REDIRECT) {
		return 0;
	}

	x = xfrm_state_lookup(net, skb->mark, (const xfrm_address_t *)&iph->daddr, esph->spi, IPPROTO_ESP, AF_INET6);
	if (!x) {
		nss_ipsec_xfrm_err("%px: xfrm_lookup failed: Done\n", skb);
		return 0;
	}

	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	if (!sa) {
		nss_ipsec_xfrm_trace("%px: xfrm_state %p is not owned by NSS: Fallback\n", skb, x);
		xfrm_state_put(x);
		return -EINVAL;
	}

	if (type == NDISC_REDIRECT) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		ip6_redirect(skb, net, skb->dev->ifindex, 0);
#else
		ip6_redirect(skb, net, skb->dev->ifindex, 0, sock_net_uid(net, NULL));
#endif
	} else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		ip6_update_pmtu(skb, net, info, 0, 0);
#else
		ip6_update_pmtu(skb, net, info, 0, 0, sock_net_uid(net, NULL));
#endif
	}

	nss_ipsec_xfrm_sa_deref(sa);
	xfrm_state_put(x);
	return 0;
}

/*
 * nss_ipsec_xfrm_update_stats()
 *	Update statistics to XFRM stack
 */
void nss_ipsec_xfrm_update_stats(struct nss_ipsec_xfrm_drv *drv, struct nss_ipsecmgr_sa_stats *stats)
{
	uint16_t family = AF_INET;
	struct xfrm_state *x;

	if (stats->sa.ip_version == 6) {
		family = AF_INET6;
	}

	/*
	 * Lookup XFRM state by SPI index
	 */
	x = xfrm_state_lookup_byspi(&init_net, htonl(stats->sa.spi_index), family);
	if (!x) {
		return;
	}

	spin_lock(&x->lock);
	x->curlft.packets += stats->pkt_count;
	x->curlft.bytes += stats->pkt_bytes;
	x->stats.replay_window += stats->fail_replay_win;
	x->stats.replay += stats->fail_replay_dup;
	x->stats.integrity_failed += stats->fail_auth;

	/*
	 * XXX: Do we need to do anything for overflow of curlft.packets
	 * and curlft.bytes ?
	 */
	xfrm_state_check_expire(x);
	spin_unlock(&x->lock);

	xfrm_state_put(x);
}

/*
 * nss_ipsec_xfrm_state_delete()
 *	xfrm_state delete notification handler. Deallocate the associated NSS SA.
 */
static void nss_ipsec_xfrm_state_delete(struct xfrm_state *x)
{
	struct nss_ipsec_xfrm_drv *drv = &g_ipsec_xfrm;
	struct nss_ipsec_xfrm_tunnel *tun;
	struct nss_ipsec_xfrm_sa *sa;

	/*
	 * If we are not managing this xfrm_state, then return
	 */
	sa = nss_ipsec_xfrm_sa_ref_by_state(x);
	if (!sa) {
		nss_ipsec_xfrm_info("%p: xfrm_state is not owned by NSS: return\n", x);
		return;
	}

	tun = sa->tun;

	/*
	 * First delete any other objects that could be holding reference to the SA object.
	 * We delete all the flows mapped to this SA.
	 */
	nss_ipsec_xfrm_flush_flow_by_sa(drv, sa);
	nss_ipsec_xfrm_sa_dealloc(sa, x);
	nss_ipsec_xfrm_sa_deref(sa);

	/*
	 * We need to release the tunnel when all SA(s) associated with are gone
	 */
	if (atomic_dec_and_test(&tun->num_sa)) {
		nss_ipsec_xfrm_del_tun(drv, tun);
	}

	return;
}

/*
 * nss_ipsec_xfrm_state_event_notify()
 *	xfrm_state change notification handler.
 */
void nss_ipsec_xfrm_state_event_notify(struct xfrm_state *x, enum xfrm_event_type event)
{
	nss_ipsec_xfrm_info("%p: xfrm state change event %d\n", x, event);

	switch (event) {
		case XFRM_EVENT_STATE_ADD:
			nss_ipsec_xfrm_trace("%p: xfrm state added\n", x);
			return;
		case XFRM_EVENT_STATE_DEL:
			nss_ipsec_xfrm_trace("%p: xfrm state removed\n", x);
			nss_ipsec_xfrm_state_delete(x);
			return;
		default:
			break;
	}
}

static struct ecm_interface_ipsec_callback xfrm_ecm_ipsec_cb =  {
	.tunnel_get_and_hold = nss_ipsec_xfrm_get_dev_n_type
};

static struct notifier_block xfrm_ecm_notifier = {
	.notifier_call = nss_ipsec_xfrm_ecm_conn_notify,
};

static struct xfrm_event_notifier xfrm_evt_notifier = {
	.state_notify = nss_ipsec_xfrm_state_event_notify,
};

#if defined(NSS_L2TPV2_ENABLED)
static struct l2tpmgr_ipsecmgr_cb xfrm_l2tp =  {
	.get_ifnum_by_dev = NULL,
	.get_ifnum_by_ip_addr = nss_ipsec_xfrm_get_inner_ifnum_by_ip,
};
#endif

#if defined(NSS_VXLAN_ENABLED)
static struct nss_vxlanmgr_get_ipsec_if_num nss_ipsec_xfrm_vxlan_cb =  {
	.get_ifnum_by_ip = nss_ipsec_xfrm_get_inner_ifnum_by_ip
};
#endif

/*
 * xfrm_mgr template to listen to state and policy notifications.
 */
static struct xfrm_mgr nss_ipsec_xfrm_mgr = {
	.notify         = nss_ipsec_xfrm_state_notify,
	.acquire        = nss_ipsec_xfrm_state_acquire,
	.compile_policy = nss_ipsec_xfrm_policy_compile,
	.new_mapping    = NULL,
	.notify_policy  = nss_ipsec_xfrm_policy_notify,
	.migrate        = NULL,
	.is_alive       = NULL,
};

/*
 * IPv4 xfrm_state afinfo object.
 */
static struct xfrm_state_afinfo xfrm_v4_afinfo = {
	.family = AF_INET,
	.proto = IPPROTO_IPIP,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	.eth_proto = htons(ETH_P_IP),
	.init_flags = nss_ipsec_xfrm_v4_init_flags,
	.init_tempsel = nss_ipsec_xfrm_v4_init_sel,
	.init_temprop = nss_ipsec_xfrm_v4_init_param,
#endif
	.output = nss_ipsec_xfrm_v4_output,
	.output_finish = nss_ipsec_xfrm_v4_output_finish,
	.extract_input = nss_ipsec_xfrm_v4_extract_input,
	.extract_output = nss_ipsec_xfrm_v4_extract_output,
	.transport_finish = nss_ipsec_xfrm_v4_transport_finish,
	.local_error = nss_ipsec_xfrm_v4_local_error,
};

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
/*
 * IPv4 xfrm_mode object
 */
struct xfrm_mode xfrm_v4_mode_map[XFRM_MODE_MAX] = {
	[XFRM_MODE_TRANSPORT] = {
		.owner = THIS_MODULE,
		.encap = XFRM_MODE_TRANSPORT,
	},
	[XFRM_MODE_TUNNEL] = {
		.owner = THIS_MODULE,
		.encap = XFRM_MODE_TUNNEL,
		.flags = XFRM_MODE_FLAG_TUNNEL,
	},
};

/*
 * IPv6 xfrm_mode object
 */
struct xfrm_mode xfrm_v6_mode_map[XFRM_MODE_MAX] = {
	[XFRM_MODE_ROUTEOPTIMIZATION] = {
		.owner = THIS_MODULE,
		.encap = XFRM_MODE_ROUTEOPTIMIZATION,
	},
	[XFRM_MODE_TRANSPORT] = {
		.owner = THIS_MODULE,
		.encap = XFRM_MODE_TRANSPORT,
	},
	[XFRM_MODE_TUNNEL] = {
		.owner = THIS_MODULE,
		.encap = XFRM_MODE_TUNNEL,
		.flags = XFRM_MODE_FLAG_TUNNEL,
	},
};
#endif

/*
 * IPv4 xfrm_type ESP object.
 */
static const struct xfrm_type xfrm_v4_type = {
	.description = "NSS ESP4",
	.owner = THIS_MODULE,
	.proto = IPPROTO_ESP,
	.flags = XFRM_TYPE_REPLAY_PROT,
	.init_state = nss_ipsec_xfrm_esp_init_state,
	.destructor = nss_ipsec_xfrm_esp_deinit_state,
	.get_mtu = nss_ipsec_xfrm_esp_get_mtu,
	.input = nss_ipsec_xfrm_esp_input,
	.output = nss_ipsec_xfrm_esp_output,
};

/*
 * xfrm IPv4 ESP proto handler.
 */
static struct xfrm4_protocol xfrm4_proto = {
	.handler = nss_ipsec_xfrm_esp4_rcv,
	.input_handler = nss_ipsec_xfrm_udp_rcv,
	.cb_handler = nss_ipsec_xfrm_esp4_rcv_err,
	.err_handler = nss_ipsec_xfrm_esp4_err,
	.priority = INT_MAX,
};

/*
 * IPv6 xfrm_state afinfo object.
 */
static struct xfrm_state_afinfo xfrm_v6_afinfo = {
	.family = AF_INET6,
	.proto = IPPROTO_IPV6,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	.eth_proto = htons(ETH_P_IPV6),
	.init_tempsel = nss_ipsec_xfrm_v6_init_sel,
	.init_temprop = nss_ipsec_xfrm_v6_init_param,
	.tmpl_sort = nss_ipsec_xfrm_v6_sort_tmpl,
	.state_sort = nss_ipsec_xfrm_v6_sort_state,
#endif
	.output = nss_ipsec_xfrm_v6_output,
	.output_finish = nss_ipsec_xfrm_v6_output_finish,
	.extract_input = nss_ipsec_xfrm_v6_extract_input,
	.extract_output = nss_ipsec_xfrm_v6_extract_output,
	.transport_finish = nss_ipsec_xfrm_v6_transport_finish,
	.local_error = nss_ipsec_xfrm_v6_local_error,
};

/*
 * IPv6 xfrm_type ESP object.
 */
static const struct xfrm_type xfrm_v6_type = {
	.description = "NSS ESP6",
	.owner = THIS_MODULE,
	.proto = IPPROTO_ESP,
	.flags = XFRM_TYPE_REPLAY_PROT,
	.init_state = nss_ipsec_xfrm_esp_init_state,
	.destructor = nss_ipsec_xfrm_esp_deinit_state,
	.get_mtu = nss_ipsec_xfrm_esp_get_mtu,
	.input = nss_ipsec_xfrm_esp_input,
	.output = nss_ipsec_xfrm_esp_output,
	.hdr_offset = nss_ipsec_xfrm_v6_esp_hdr_offset,
};

/*
 * xfrm IPv6 ESP proto handler.
 */
static struct xfrm6_protocol xfrm6_proto = {
	.handler = nss_ipsec_xfrm_esp6_rcv,
	.cb_handler = nss_ipsec_xfrm_esp6_rcv_err,
	.err_handler = nss_ipsec_xfrm_esp6_err,
	.priority = INT_MAX,
};

static void nss_ipsec_xfrm_restore_afinfo(struct nss_ipsec_xfrm_drv *drv, uint16_t family)
{
	const struct xfrm_type *type_dstopts, *type_routing;
	const struct xfrm_type *type_ipip, *type_ipv6;
	const struct xfrm_type *type_ah, *type_comp;
	struct xfrm_state_afinfo *afinfo;
	const struct xfrm_type *base;

	/*
	 * Restore the native afinfo object
	 */
	if (family == AF_INET) {
		base = &xfrm_v4_type;
		afinfo = drv->xsa.v4;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		xfrm_unregister_mode(&xfrm_v4_mode_map[XFRM_MODE_TUNNEL], AF_INET);
		xfrm_unregister_mode(&xfrm_v4_mode_map[XFRM_MODE_TRANSPORT], AF_INET);
#endif
	} else {
		base = &xfrm_v6_type;
		afinfo = drv->xsa.v6;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		xfrm_unregister_mode(&xfrm_v6_mode_map[XFRM_MODE_TUNNEL], AF_INET6);
		xfrm_unregister_mode(&xfrm_v6_mode_map[XFRM_MODE_TRANSPORT], AF_INET6);
		xfrm_unregister_mode(&xfrm_v6_mode_map[XFRM_MODE_ROUTEOPTIMIZATION], AF_INET6);
#endif
	}

	BUG_ON(!afinfo);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	type_ah = afinfo->type_map[IPPROTO_AH];
	type_comp = afinfo->type_map[IPPROTO_COMP];
	type_ipip = afinfo->type_map[IPPROTO_IPIP];
	type_ipv6 = afinfo->type_map[IPPROTO_IPV6];
	type_dstopts = afinfo->type_map[IPPROTO_DSTOPTS];
	type_routing = afinfo->type_map[IPPROTO_ROUTING];
#else
	type_ah = afinfo->type_ah;
	type_comp = afinfo->type_comp;
	type_ipip = afinfo->type_ipip;
	type_ipv6 = afinfo->type_ipip6;
	type_dstopts = afinfo->type_dstopts;
	type_routing = afinfo->type_routing;
#endif
	/*
	 * Unregister types
	 */
	if (type_routing) {
		xfrm_unregister_type(type_routing, family);
	}

	if (type_dstopts) {
		xfrm_unregister_type(type_dstopts, family);
	}

	if (type_ipv6) {
		xfrm_unregister_type(type_ipv6, family);
	}

	if (type_ipip) {
		xfrm_unregister_type(type_ipip, family);
	}

	if (type_comp) {
		xfrm_unregister_type(type_comp, family);
	}

	if (type_ah) {
		xfrm_unregister_type(type_ah, family);
	}

	xfrm_unregister_type(base, family);

	xfrm_state_update_afinfo(family, afinfo);
}

static void nss_ipsec_xfrm_override_afinfo(struct nss_ipsec_xfrm_drv *drv, uint16_t family)
{
	const struct xfrm_type *type_dstopts, *type_routing;
	const struct xfrm_type *type_ipip, *type_ipv6;
	const struct xfrm_type *type_ah, *type_comp;
	struct xfrm_state_afinfo *afinfo;
	const struct xfrm_type *base;

	/*
	 * Override ESP type.
	 */
	if (family == AF_INET) {
		base = &xfrm_v4_type;
		afinfo = drv->xsa.v4 = xfrm_state_update_afinfo(AF_INET, &xfrm_v4_afinfo);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		xfrm_register_mode(&xfrm_v4_mode_map[XFRM_MODE_TRANSPORT], AF_INET);
		xfrm_register_mode(&xfrm_v4_mode_map[XFRM_MODE_TUNNEL], AF_INET);
#endif
	} else {
		base = &xfrm_v6_type;
		afinfo = drv->xsa.v6 = xfrm_state_update_afinfo(AF_INET6, &xfrm_v6_afinfo);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
		xfrm_register_mode(&xfrm_v6_mode_map[XFRM_MODE_ROUTEOPTIMIZATION], AF_INET6);
		xfrm_register_mode(&xfrm_v6_mode_map[XFRM_MODE_TRANSPORT], AF_INET6);
		xfrm_register_mode(&xfrm_v6_mode_map[XFRM_MODE_TUNNEL], AF_INET6);
#endif
	}

	BUG_ON(!afinfo);

	xfrm_register_type(base, family);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	type_ah = afinfo->type_map[IPPROTO_AH];
	type_comp = afinfo->type_map[IPPROTO_COMP];
	type_ipip = afinfo->type_map[IPPROTO_IPIP];
	type_ipv6 = afinfo->type_map[IPPROTO_IPV6];
	type_dstopts = afinfo->type_map[IPPROTO_DSTOPTS];
	type_routing = afinfo->type_map[IPPROTO_ROUTING];
#else
	type_ah = afinfo->type_ah;
	type_comp = afinfo->type_comp;
	type_ipip = afinfo->type_ipip;
	type_ipv6 = afinfo->type_ipip6;
	type_dstopts = afinfo->type_dstopts;
	type_routing = afinfo->type_routing;
#endif
	/*
	 * Register types
	 */
	if (type_ah) {
		xfrm_register_type(type_ah, family);
	}

	if (type_comp) {
		xfrm_register_type(type_comp, family);
	}

	if (type_ipip) {
		xfrm_register_type(type_ipip, family);
	}

	if (type_ipv6) {
		xfrm_register_type(type_ipv6, family);
	}

	if (type_dstopts) {
		xfrm_register_type(type_dstopts, family);
	}

	if (type_routing) {
		xfrm_register_type(type_routing, family);
	}
}

/*
 * nss_ipsec_xfrm_init_module()
 *	module_init function
 */
int __init nss_ipsec_xfrm_init_module(void)
{

	rwlock_init(&g_ipsec_xfrm.lock);

	nss_ipsec_xfrm_init_tun_db(&g_ipsec_xfrm);
	nss_ipsec_xfrm_init_flow_db(&g_ipsec_xfrm);

	init_completion(&g_ipsec_xfrm.complete);

	net_get_random_once(&g_ipsec_xfrm.hash_nonce, sizeof(g_ipsec_xfrm.hash_nonce));

	/*
	 * Listen to xfrm event notifcations
	 */
	xfrm_event_register_notifier(&init_net, &xfrm_evt_notifier);

	/*
	 * Register own esp handlers
	 */
	if (xfrm4_protocol_register(&xfrm4_proto, IPPROTO_ESP) < 0) {
		nss_ipsec_xfrm_info_always("can't add ESP protocol for IPv4\n");
		return -EAGAIN;
	}

	if (xfrm6_protocol_register(&xfrm6_proto, IPPROTO_ESP) < 0) {
		nss_ipsec_xfrm_info_always("can't add ESP protocol for IPv6\n");
		goto unreg_v4_handler;
	}

	/*
	 * Override xfrm_state afinfo
	 */
	nss_ipsec_xfrm_override_afinfo(&g_ipsec_xfrm, AF_INET);
	nss_ipsec_xfrm_override_afinfo(&g_ipsec_xfrm, AF_INET6);

	ecm_interface_ipsec_register_callbacks(&xfrm_ecm_ipsec_cb);
	ecm_notifier_register_connection_notify(&xfrm_ecm_notifier);

#if defined(NSS_L2TPV2_ENABLED)
	l2tpmgr_register_ipsecmgr_callback_by_ipaddr(&xfrm_l2tp);
#endif

#if defined(NSS_VXLAN_ENABLED)
	nss_vxlanmgr_register_ipsecmgr_callback_by_ip(&nss_ipsec_xfrm_vxlan_cb);
#endif

	/*
	 * Register for xfrm events
	 */
	xfrm_register_km(&nss_ipsec_xfrm_mgr);

	/*
	 * Initialize debugfs.
	 */
	nss_ipsec_xfrm_debugfs_init(&g_ipsec_xfrm);
	nss_ipsec_xfrm_info_always("NSS IPSec xfrm plugin module loaded %s\n", NSS_CLIENT_BUILD_ID);
	return 0;

unreg_v4_handler:
	xfrm6_protocol_deregister(&xfrm6_proto, IPPROTO_ESP);
	return -EAGAIN;
}

/*
 * nss_ipsec_xfrm_exit_module()
 *	Unregister callbacks/notifiers and clear all stale data
 */
void __exit nss_ipsec_xfrm_exit_module(void)
{
	int ret;

	xfrm4_protocol_deregister(&xfrm4_proto, IPPROTO_ESP);
	xfrm6_protocol_deregister(&xfrm6_proto, IPPROTO_ESP);

	nss_ipsec_xfrm_restore_afinfo(&g_ipsec_xfrm, AF_INET);
	nss_ipsec_xfrm_restore_afinfo(&g_ipsec_xfrm, AF_INET6);

	ecm_notifier_unregister_connection_notify(&xfrm_ecm_notifier);
	ecm_interface_ipsec_unregister_callbacks();

#if defined(NSS_L2TPV2_ENABLED)
	l2tpmgr_unregister_ipsecmgr_callback_by_ipaddr();
#endif

#if defined(NSS_VXLAN_ENABLED)
	nss_vxlanmgr_unregister_ipsecmgr_callback_by_ip();
#endif
	xfrm_unregister_km(&nss_ipsec_xfrm_mgr);

	/*
	 * Wait till the active tunnel count reaches zero.
	 */
	if (atomic_read(&g_ipsec_xfrm.num_tunnels)) {
		ret = wait_for_completion_timeout(&g_ipsec_xfrm.complete, NSS_IPSEC_XFRM_TUNNEL_FREE_TIMEOUT);
		WARN_ON(!ret);
	}

	/*
	 * Remove debugfs.
	 */
	nss_ipsec_xfrm_debugfs_deinit(&g_ipsec_xfrm);

	/*
	 * Unregister xfrm event notifcations
	 */
	xfrm_event_unregister_notifier(&init_net, &xfrm_evt_notifier);

	nss_ipsec_xfrm_info_always("NSS IPSec xfrm plugin module unloaded\n");
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS IPsec XFRM offload glue");

module_init(nss_ipsec_xfrm_init_module);
module_exit(nss_ipsec_xfrm_exit_module);
