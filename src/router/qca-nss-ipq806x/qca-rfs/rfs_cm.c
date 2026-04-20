/*
 * Copyright (c) 2014 - 2015, 2017 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * rfs_cm.c
 *	Receiving Flow Streering - Connection Manager
 */

#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/skbuff.h>
#include <linux/jhash.h>
#include <linux/inetdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/proc_fs.h>
#include <net/route.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_core.h>

#include "rfs.h"
#include "rfs_cm.h"
#include "rfs_rule.h"
#include "rfs_ess.h"

/*
 * Definition for RX hash debugging.
 * Debugging only, don't enable it when releasing
 * #define DEBUG_ESS_SKB_RXHASH 1
 */

#define RFS_CONNECTION_HASH_SHIFT 10
#define RFS_CONNECTION_HASH_SIZE (1 << RFS_CONNECTION_HASH_SHIFT)
#define RFS_CONNECTION_HASH_MASK (RFS_CONNECTION_HASH_SIZE - 1)

#define RFS_NAT_IP_HASH_SHIFT 8
#define RFS_NAT_IP_HASH_SIZE (1 << RFS_NAT_IP_HASH_SHIFT)
#define RFS_NAT_IP_HASH_MASK (RFS_NAT_IP_HASH_SIZE - 1)

/*
 * Per-module structure.
 */
struct rfs_cm {
	spinlock_t hash_lock;
	/*
	 * connection hash table
	 */
	struct hlist_head conn_hash[RFS_CONNECTION_HASH_SIZE];
	/*
	 * SNAT/DNAT hash table for fast IP address searching
	 */
	struct hlist_head snat_hash[RFS_NAT_IP_HASH_SIZE];
	struct hlist_head dnat_hash[RFS_NAT_IP_HASH_SIZE];
	/*
	 * proc entry for connection debugging
	 */
	struct proc_dir_entry *proc_cm;
	int is_running;
};

static struct rfs_cm __cm;


/*
 * rfs_cm_connection_hash
 *	calculate connection hash by 5-tuple
 */
static unsigned int rfs_cm_connection_hash(uint8_t protocol,
					__be32 src_ip, __be16 src_port,
					__be32 dest_ip, __be16 dest_port)
{
        uint32_t hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
        return ((hash >> RFS_CONNECTION_HASH_SHIFT) ^ hash) & RFS_CONNECTION_HASH_MASK;
}


/*
 * rfs_cm_ip_hash
 *	calculate connection hash by IP address
 */
static unsigned int rfs_cm_ip_hash(__be32 ipaddr)
{
	return jhash_1word(ipaddr, 0) & RFS_NAT_IP_HASH_MASK;
}


/*
 * rfs_cm_connection_rcu_free
 */
static void rfs_cm_connection_rcu_free(struct rcu_head *head)
{
	struct rfs_cm_ipv4_connection *ce
		= container_of(head, struct rfs_cm_ipv4_connection, rcu);
	kfree(ce);
}


/*
 * rfs_cm_conneciton_create
 */
int rfs_cm_conneciton_create(struct rfs_cm_ipv4_connection *conn)
{
	unsigned int index;
	struct hlist_head *head;
	struct rfs_cm_ipv4_connection *ce;
	uint16_t cpu;
	struct rfs_cm *cm = &__cm;

	spin_lock_bh(&cm->hash_lock);
	/*
	 * Look up the connection by orig 5-tuples
	 */
	index = rfs_cm_connection_hash(conn->protocol,
				       conn->orig_src_ip,
				       conn->orig_src_port,
				       conn->orig_dest_ip,
				       conn->orig_dest_port);

	head = &cm->conn_hash[index];
	hlist_for_each_entry_rcu(ce, head, conn_hlist) {
		if (ce->protocol == conn->protocol &&
		    ce->orig_src_ip == conn->orig_src_ip &&
		    ce->orig_src_port == conn->orig_src_port &&
		    ce->orig_dest_ip == conn->orig_dest_ip &&
		    ce->orig_dest_port == conn->orig_dest_port) {
			break;
		}
	}

	if (ce) {
		spin_unlock_bh(&cm->hash_lock);
		return 0;
	}

	/*
	 * Create a connection entry if it doesn't exist
	 */
	ce = kmalloc(sizeof(struct rfs_cm_ipv4_connection), GFP_ATOMIC);
	if (!ce ) {
		spin_unlock_bh(&cm->hash_lock);
		return -1;
	}

	memcpy(ce, conn, sizeof(struct rfs_cm_ipv4_connection));
	hlist_add_head_rcu(&ce->conn_hlist, head);

	if (ce->flag & RFS_CM_FLAG_SNAT) {
		index = rfs_cm_ip_hash(conn->orig_src_ip);
		head  = &cm->snat_hash[index];
		hlist_add_head_rcu(&ce->snat_hlist, head);
	}

	if (ce->flag & RFS_CM_FLAG_DNAT) {
		index = rfs_cm_ip_hash(conn->reply_src_ip);
		head  = &cm->dnat_hash[index];
		hlist_add_head_rcu(&ce->dnat_hlist, head);
	}

        RFS_DEBUG("New connnection:\n\torig  p: %d s:%pI4:%u, d: %pI4:%u"
		   " reply s:%pI4:%u, d: %pI4:%u flags: %08x, cpu %d\n",
                    ce->protocol,
                    &ce->orig_src_ip, ntohs(ce->orig_src_port),
                    &ce->orig_dest_ip, ntohs(ce->orig_dest_port),
                    &ce->reply_src_ip, ntohs(ce->reply_src_port),
                    &ce->reply_dest_ip, ntohs(ce->reply_dest_port),
		    ce->flag, ce->cpu);

	if (ce->flag && RFS_CM_FLAG_SNAT)
		cpu = rfs_rule_get_cpu_by_ipaddr(conn->orig_src_ip);
	else
		cpu = rfs_rule_get_cpu_by_ipaddr(conn->reply_src_ip);

	if (ce->cpu != RPS_NO_CPU) {
	    rfs_ess_update_tuple_rule(ce->orig_rxhash, ce->reply_rxhash, ce->cpu);
	}
	spin_unlock_bh(&cm->hash_lock);
	return 0;
}


/*
 * rfs_cm_connection_destroy
 */
static int rfs_cm_connection_destroy(struct rfs_cm_ipv4_connection *conn)
{
	unsigned int index;
	struct hlist_head *head;
	struct rfs_cm_ipv4_connection *ce;
	struct rfs_cm *cm = &__cm;

	spin_lock_bh(&cm->hash_lock);
	/*
	 * Look up the connection by orig 5-tuples
	 */
	index = rfs_cm_connection_hash(conn->protocol,
				       conn->orig_src_ip,
				       conn->orig_src_port,
				       conn->orig_dest_ip,
				       conn->orig_dest_port);

	head = &cm->conn_hash[index];
	hlist_for_each_entry_rcu(ce, head, conn_hlist) {
		if (ce->protocol == conn->protocol &&
		    ce->orig_src_ip == conn->orig_src_ip &&
		    ce->orig_src_port == conn->orig_src_port &&
		    ce->orig_dest_ip == conn->orig_dest_ip &&
		    ce->orig_dest_port == conn->orig_dest_port) {
			break;
		}
	}

	if (!ce) {
		spin_unlock_bh(&cm->hash_lock);
		return 0;
	}

	hlist_del_rcu(&ce->conn_hlist);

	if (ce->flag & RFS_CM_FLAG_SNAT) {
		hlist_del_rcu(&ce->snat_hlist);
	}

	if (ce->flag & RFS_CM_FLAG_DNAT ) {
		hlist_del_rcu(&ce->dnat_hlist);
	}

        RFS_DEBUG("Remove connnection:\n\torig  p: %d s:%pI4:%u, d: %pI4:%u"
		   " reply s:%pI4:%u, d: %pI4:%u flags: %08x, cpu %d\n",
                    ce->protocol,
                    &ce->orig_src_ip, ntohs(ce->orig_src_port),
                    &ce->orig_dest_ip, ntohs(ce->orig_dest_port),
                    &ce->reply_src_ip, ntohs(ce->reply_src_port),
                    &ce->reply_dest_ip, ntohs(ce->reply_dest_port),
		    ce->flag, ce->cpu);

	if (ce->cpu != RPS_NO_CPU ) {
	    rfs_ess_update_tuple_rule(ce->orig_rxhash, ce->reply_rxhash, RPS_NO_CPU);
	}


	call_rcu(&ce->rcu, rfs_cm_connection_rcu_free);
	spin_unlock_bh(&cm->hash_lock);
	return 0;
}


/*
 * rfs_cm_connection_find
 */
int rfs_cm_connection_find(struct rfs_cm_ipv4_connection *conn)
{
	unsigned int index;
	struct hlist_head *head;
	struct rfs_cm_ipv4_connection *ce;
	struct rfs_cm *cm = &__cm;

	rcu_read_lock();
	/*
	 * Look up the connection by orig 5-tuples
	 */
	index = rfs_cm_connection_hash(conn->protocol,
				       conn->orig_src_ip,
				       conn->orig_src_port,
				       conn->orig_dest_ip,
				       conn->orig_dest_port);

	head = &cm->conn_hash[index];
	hlist_for_each_entry_rcu(ce, head, conn_hlist) {
		if (ce->protocol == conn->protocol &&
		    ce->orig_src_ip == conn->orig_src_ip &&
		    ce->orig_src_port == conn->orig_src_port &&
		    ce->orig_dest_ip == conn->orig_dest_ip &&
		    ce->orig_dest_port == conn->orig_dest_port) {
			break;
		}
	}

	if (!ce) {
		rcu_read_unlock();
		return 0;
	}

	/*
	 * Ugly, but it's efficient to get all information
	 */
	memcpy(conn, ce, sizeof(struct rfs_cm_ipv4_connection));

	rcu_read_unlock();
	return 1;
}


/*
 * rfs_cm_connection_destroy_all
 */
void rfs_cm_connection_destroy_all(void)
{
	unsigned int index;
	struct hlist_head *head;
	struct rfs_cm_ipv4_connection *ce;
	struct rfs_cm *cm = &__cm;

	spin_lock_bh(&cm->hash_lock);
	for ( index = 0; index < RFS_CONNECTION_HASH_SIZE; index++) {
		struct hlist_node *n;
		head = &cm->conn_hash[index];;
		hlist_for_each_entry_safe(ce, n, head, conn_hlist) {
			hlist_del_rcu(&ce->conn_hlist);

			if (ce->flag & RFS_CM_FLAG_SNAT) {
				hlist_del_rcu(&ce->snat_hlist);
			}

			if (ce->flag & RFS_CM_FLAG_DNAT ) {
				hlist_del_rcu(&ce->dnat_hlist);
			}
			call_rcu(&ce->rcu, rfs_cm_connection_rcu_free);

		}
	}
	spin_unlock_bh(&cm->hash_lock);

}


/*
 * rfs_cm_update_rules
 */
int rfs_cm_update_rules(__be32 ipaddr, uint16_t cpu)
{
	unsigned int index;
	struct hlist_head *head;
	struct rfs_cm_ipv4_connection *ce;
	struct rfs_cm *cm = &__cm;


	spin_lock_bh(&cm->hash_lock);
	index = rfs_cm_ip_hash(ipaddr);
	head  = &cm->snat_hash[index];
	hlist_for_each_entry_rcu(ce, head, snat_hlist) {
		if (ce->orig_src_ip != ipaddr)
			continue;
		if (cpu == ce->cpu)
			continue;
		RFS_DEBUG("Connection cpu change %d --> %d\n", ce->cpu, cpu);
		rfs_ess_update_tuple_rule(ce->orig_rxhash, ce->reply_rxhash, cpu);
		ce->cpu = cpu;
	}


	head  = &cm->dnat_hash[index];
	hlist_for_each_entry_rcu(ce, head, dnat_hlist) {
		if (ce->reply_src_ip != ipaddr)
			continue;
		if (cpu == ce->cpu)
			continue;
		RFS_DEBUG("Connection cpu change %d --> %d\n", ce->cpu, cpu);
		rfs_ess_update_tuple_rule(ce->orig_rxhash, ce->reply_rxhash, cpu);
		ce->cpu = cpu;
	}

	spin_unlock_bh(&cm->hash_lock);
	return 0;
}


/*
 * rfs_cm_proc_show
 *	Show connections in proc file system
 */
static int rfs_cm_proc_show(struct seq_file *m, void *v)
{
	unsigned int index;
	int count = 0;
	struct hlist_head *head;
	struct rfs_cm_ipv4_connection *ce;
	struct rfs_cm *cm = &__cm;

	seq_printf(m, "RFS connection table:\n");

	rcu_read_lock();
	for ( index = 0; index < RFS_CONNECTION_HASH_SIZE; index++) {
		head = &cm->conn_hash[index];
		hlist_for_each_entry_rcu(ce, head, conn_hlist) {
			seq_printf(m, "%03d %04x proto %s flags: %08x cpu %d:\n",
				   ++count, index,
				   ce->protocol==IPPROTO_TCP?"TCP":"UDP",
				   ce->flag, ce->cpu);
			seq_printf(m, "\ts:%pI4:%u d:%pI4:%u / s:%pI4:%u d:%pI4:%u\n",
				   &ce->orig_src_ip, ntohs(ce->orig_src_port),
				   &ce->orig_dest_ip, ntohs(ce->orig_dest_port),
				   &ce->reply_src_ip, ntohs(ce->reply_src_port),
				   &ce->reply_dest_ip, ntohs(ce->reply_dest_port));
		}
	}
	seq_putc(m, '\n');
	rcu_read_unlock();
	return 0;
}


/*
 * rfs_cm_proc_open
 */
static int rfs_cm_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, rfs_cm_proc_show, NULL);
}


/*
 * struct file_operations cm_proc_fops
 */
static const struct file_operations cm_proc_fops = {
	.owner = THIS_MODULE,
	.open  = rfs_cm_proc_open,
	.read  = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


/*
 * rfs_cm_conntrack_event()
 *	Callback event invoked when a conntrack connection's state changes.
 */
#ifdef CONFIG_NF_CONNTRACK_CHAIN_EVENTS
static int rfs_cm_conntrack_event(struct notifier_block *this,
			unsigned long events, void *ptr)
#else
static int rfs_cm_conntrack_event(unsigned int events, struct nf_ct_event *item)
#endif
{
#ifdef CONFIG_NF_CONNTRACK_CHAIN_EVENTS
	struct nf_ct_event *item = ptr;
#endif
	struct nf_conn *ct = item->ct;
	struct nf_conntrack_tuple orig_tuple;
	struct nf_conntrack_tuple reply_tuple;
	struct rfs_cm_ipv4_connection conn;
	int snat, dnat;

	/*
	 * If we don't have a conntrack entry then we're done.
	 */
	if (unlikely(!ct)) {
		RFS_WARN("no ct in conntrack event callback\n");
		return NOTIFY_DONE;
	}

	/*
	 * If this is an untracked connection then we can't have any state either.
	 */
	if (unlikely(ct == &nf_conntrack_untracked)) {
		RFS_TRACE("ignoring untracked conn\n");
		return NOTIFY_DONE;
	}

	/*
	 * Ignore anything other than IPv4 connections.
	 */
	if (unlikely(nf_ct_l3num(ct) != AF_INET)) {
		RFS_TRACE("ignoring non-IPv4 conn\n");
		return NOTIFY_DONE;
	}


	snat = test_bit(IPS_SRC_NAT_BIT, &ct->status);
	dnat = test_bit(IPS_DST_NAT_BIT, &ct->status);

	if (!snat && !dnat ) {
		RFS_TRACE("ignoring non-NAT conn\n");
		return NOTIFY_DONE;
	}

	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	reply_tuple = ct->tuplehash[IP_CT_DIR_REPLY].tuple;

	memset(&conn, 0, sizeof(struct rfs_cm_ipv4_connection));
	conn.protocol = orig_tuple.dst.protonum;
	conn.orig_src_ip = (__be32)orig_tuple.src.u3.ip;
	conn.orig_dest_ip = (__be32)orig_tuple.dst.u3.ip;
	conn.reply_src_ip = (__be32)reply_tuple.src.u3.ip;
	conn.reply_dest_ip = (__be32)reply_tuple.dst.u3.ip;

	if (ipv4_is_loopback(conn.orig_dest_ip) || ipv4_is_multicast(conn.orig_dest_ip))
		return NOTIFY_DONE;

	switch (conn.protocol) {
	case IPPROTO_TCP:
		conn.orig_src_port = orig_tuple.src.u.tcp.port;
		conn.orig_dest_port = orig_tuple.dst.u.tcp.port;
		conn.reply_src_port = reply_tuple.src.u.tcp.port;
		conn.reply_dest_port = reply_tuple.dst.u.tcp.port;

		break;
	case IPPROTO_UDP:
		conn.orig_src_port = orig_tuple.src.u.udp.port;
		conn.orig_dest_port = orig_tuple.dst.u.udp.port;
		conn.reply_src_port = reply_tuple.src.u.udp.port;
		conn.reply_dest_port = reply_tuple.dst.u.udp.port;
		break;
	default:
		return NOTIFY_DONE;
	}


	conn.cpu = RPS_NO_CPU;
	if (snat) {
		conn.flag |= RFS_CM_FLAG_SNAT;
		conn.reply_rxhash =
			rfs_ess_get_rxhash(conn.reply_src_ip,
					   conn.reply_dest_ip,
					   conn.reply_src_port,
					   conn.reply_dest_port);
		conn.cpu = rfs_rule_get_cpu_by_ipaddr(conn.orig_src_ip);
	}

	if (dnat) {
		conn.flag |= RFS_CM_FLAG_DNAT;
		conn.orig_rxhash =
			rfs_ess_get_rxhash(conn.orig_src_ip,
					   conn.orig_dest_ip,
					   conn.orig_src_port,
					   conn.orig_dest_port);
		conn.cpu = rfs_rule_get_cpu_by_ipaddr(conn.reply_src_ip);
	}

	if (events & ((1 << IPCT_NEW) | (1 << IPCT_RELATED))) {
		rfs_cm_conneciton_create(&conn);
	}
	else if (events & (1 << IPCT_DESTROY)) {
		rfs_cm_connection_destroy(&conn);
	}

	return NOTIFY_DONE;
}

/*
 * Netfilter conntrack event system to monitor connection tracking changes
 */
#ifdef CONFIG_NF_CONNTRACK_CHAIN_EVENTS
static struct notifier_block rfs_cm_conntrack_notifier = {
	.notifier_call = rfs_cm_conntrack_event,
};
#else
static struct nf_ct_event_notifier rfs_cm_conntrack_notifier = {
	.fcn = rfs_cm_conntrack_event,
};
#endif



#ifdef DEBUG_ESS_SKB_RXHASH
/*
 * __rfs_cm_ipv4_post_routing_hook
 */
static unsigned int __rfs_cm_ipv4_post_routing_hook(
			const struct nf_hook_ops *ops,
			struct sk_buff *skb,
			const struct net_device *in,
			const struct net_device *out,
			int (*okfn)(struct sk_buff *))
{
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	enum ip_conntrack_dir dir;
	struct nf_conntrack_tuple orig_tuple;
	struct nf_conntrack_tuple reply_tuple;
	struct rfs_cm_ipv4_connection conn;
	int snat, dnat;

	if (!skb->rxhash) {
		return NF_ACCEPT;
	}

	if (unlikely(skb->pkt_type == PACKET_BROADCAST)) {
		return NF_ACCEPT;
	}

	if (unlikely(skb->pkt_type == PACKET_MULTICAST)) {
		return NF_ACCEPT;
	}

	ct = nf_ct_get(skb, &ctinfo);
	if (unlikely(!ct)) {
		return NF_ACCEPT;
	}

	if (unlikely(ct == &nf_conntrack_untracked)) {
		return NF_ACCEPT;
	}


	if (unlikely(nf_ct_l3num(ct) != AF_INET)) {
		return NF_ACCEPT;
	}


	snat = test_bit(IPS_SRC_NAT_BIT, &ct->status);
	dnat = test_bit(IPS_DST_NAT_BIT, &ct->status);
	dir = CTINFO2DIR(ctinfo);

	if(!snat && !dnat) {
		return NF_ACCEPT;
	}

	if (snat && (dir != IP_CT_DIR_REPLY)) {
		RFS_TRACE("ignoring SNAT original\n");
		return NF_ACCEPT;
	}


	if (dnat && (dir != IP_CT_DIR_ORIGINAL)) {
		RFS_TRACE("ignoring DNAT reply\n");
		return NF_ACCEPT;
	}

	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
	reply_tuple = ct->tuplehash[IP_CT_DIR_REPLY].tuple;

	memset(&conn, 0, sizeof(struct rfs_cm_ipv4_connection));
	conn.protocol = orig_tuple.dst.protonum;
	conn.orig_src_ip = (__be32)orig_tuple.src.u3.ip;
	conn.orig_dest_ip = (__be32)orig_tuple.dst.u3.ip;
	conn.reply_src_ip = (__be32)reply_tuple.src.u3.ip;
	conn.reply_dest_ip = (__be32)reply_tuple.dst.u3.ip;

	if (ipv4_is_loopback(conn.orig_dest_ip) || ipv4_is_multicast(conn.orig_dest_ip))
		return NF_ACCEPT;

	switch (conn.protocol) {
	case IPPROTO_TCP:
		conn.orig_src_port = orig_tuple.src.u.tcp.port;
		conn.orig_dest_port = orig_tuple.dst.u.tcp.port;
		conn.reply_src_port = reply_tuple.src.u.tcp.port;
		conn.reply_dest_port = reply_tuple.dst.u.tcp.port;

		break;
	case IPPROTO_UDP:
		conn.orig_src_port = orig_tuple.src.u.udp.port;
		conn.orig_dest_port = orig_tuple.dst.u.udp.port;
		conn.reply_src_port = reply_tuple.src.u.udp.port;
		conn.reply_dest_port = reply_tuple.dst.u.udp.port;
		break;
	default:
		return NF_ACCEPT;

	}

	if (rfs_cm_connection_find(&conn) <= 0) {
		RFS_DEBUG("can't find connnection:\n\torig  p: %d s:%pI4:%u, d: %pI4:%u"
		   " reply s:%pI4:%u, d: %pI4:%u\n",
                    conn.protocol,
                    &conn.orig_src_ip, ntohs(conn.orig_src_port),
                    &conn.orig_dest_ip, ntohs(conn.orig_dest_port),
                    &conn.reply_src_ip, ntohs(conn.reply_src_port),
                    &conn.reply_dest_ip, ntohs(conn.reply_dest_port));

		return NF_ACCEPT;
	}

        RFS_DEBUG("Connnection:\n\torig  p: %d s:%pI4:%u, d: %pI4:%u"
		   " reply s:%pI4:%u, d: %pI4:%u dir: %s\n",
                    conn.protocol,
                    &conn.orig_src_ip, ntohs(conn.orig_src_port),
                    &conn.orig_dest_ip, ntohs(conn.orig_dest_port),
                    &conn.reply_src_ip, ntohs(conn.reply_src_port),
                    &conn.reply_dest_ip, ntohs(conn.reply_dest_port),
		    dir == IP_CT_DIR_ORIGINAL?"original":"reply");

	if (snat) {
		RFS_DEBUG("Rx hash 0x%08x, calc 0x%08x\n", skb->rxhash, conn.reply_rxhash);
	}

	if (dnat) {
		RFS_DEBUG("Rx hash 0x%08x, calc 0x%08x\n", skb->rxhash, conn.orig_rxhash);
	}
	return NF_ACCEPT;
}

static struct nf_hook_ops rfs_cm_nf_hooks[] __read_mostly = {
	{
		.hook = __rfs_cm_ipv4_post_routing_hook,
		.owner = THIS_MODULE,
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_NAT_SRC + 1,
	},

};
#endif

/*
 * rfs_cm_start
 */
int rfs_cm_start(void)
{
	int ret = -1;
	struct rfs_cm *cm = &__cm;

	if (cm->is_running)
		return 0;

	RFS_DEBUG("RFS cm start\n");
#ifdef CONFIG_NF_CONNTRACK_EVENTS
	ret = nf_conntrack_register_notifier(&init_net, &rfs_cm_conntrack_notifier);
	if (ret < 0) {
		RFS_ERROR("can't register nf notifier hook: %d\n", ret);
		return -1;
	}
#endif

#ifdef DEBUG_ESS_SKB_RXHASH
	nf_register_hooks(rfs_cm_nf_hooks, ARRAY_SIZE(rfs_cm_nf_hooks));
#endif
	cm->is_running = 1;
	return 0;
}


/*
 * rfs_cm_stop
 */
int rfs_cm_stop(void)
{
	struct rfs_cm *cm = &__cm;

	if (!cm->is_running)
		return 0;

	RFS_DEBUG("RFS cm stop\n");
#ifdef DEBUG_ESS_SKB_RXHASH
	nf_unregister_hooks(rfs_cm_nf_hooks, ARRAY_SIZE(rfs_cm_nf_hooks));
#endif

#ifdef CONFIG_NF_CONNTRACK_EVENTS
	nf_conntrack_unregister_notifier(&init_net, &rfs_cm_conntrack_notifier);
#endif

	rfs_cm_connection_destroy_all();
	cm->is_running = 0;
	return 0;
}


/*
 * rfs_cm_init()
 */
int rfs_cm_init(void)
{
	struct rfs_cm *cm = &__cm;

	RFS_DEBUG("RFS cm init\n");
	spin_lock_init(&cm->hash_lock);
	cm->proc_cm = proc_create("connection", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
				    rfs_proc_entry, &cm_proc_fops);

	cm->is_running = 0;
	return 0;
}

/*
 * rfs_cm_exit()
 */
void rfs_cm_exit(void)
{

	struct rfs_cm *cm = &__cm;
	RFS_DEBUG("RFS cm exit\n");

	if (cm->proc_cm)
		remove_proc_entry("connection", rfs_proc_entry);

	rfs_cm_stop();
}


