/*
 * Rusty Russell (C)2000 -- This code is GPL.
 * Patrick McHardy (c) 2006-2012
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter_bridge.h>
#include <linux/seq_file.h>
#include <linux/rcupdate.h>
#include <net/protocol.h>
#include <net/netfilter/nf_queue.h>
#include <net/dst.h>

#include "nf_internals.h"

static const struct nf_queue_handler __rcu *nf_queue_handler;

/*
 * Hook for nfnetlink_queue to register its queue handler.
 * We do this so that most of the NFQUEUE code can be modular.
 *
 * Once the queue is registered it must reinject all packets it
 * receives, no matter what.
 */

void nf_register_queue_handler(const struct nf_queue_handler *qh)
{
	/* should never happen, we only have one queueing backend in kernel */
	WARN_ON(rcu_access_pointer(nf_queue_handler));
	rcu_assign_pointer(nf_queue_handler, qh);
}
EXPORT_SYMBOL(nf_register_queue_handler);

/* The caller must flush their queue before this */
void nf_unregister_queue_handler(void)
{
	RCU_INIT_POINTER(nf_queue_handler, NULL);
}
EXPORT_SYMBOL(nf_unregister_queue_handler);

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
static const struct nf_queue_handler __rcu *queue_imq_handler __read_mostly;

void nf_register_queue_imq_handler(const struct nf_queue_handler *qh)
{
	rcu_assign_pointer(queue_imq_handler, qh);
}
EXPORT_SYMBOL_GPL(nf_register_queue_imq_handler);

void nf_unregister_queue_imq_handler(void)
{
	RCU_INIT_POINTER(queue_imq_handler, NULL);
	synchronize_rcu();
}
EXPORT_SYMBOL_GPL(nf_unregister_queue_imq_handler);
#endif

static void nf_queue_sock_put(struct sock *sk)
{
#ifdef CONFIG_INET
	sock_gen_put(sk);
#else
	sock_put(sk);
#endif
}

static void nf_queue_entry_release_refs(struct nf_queue_entry *entry)
{
	struct nf_hook_state *state = &entry->state;

	/* Release those devices we held, or Alexey will kill me. */
	dev_put(state->in);
	dev_put(state->out);
	if (state->sk)
		nf_queue_sock_put(state->sk);

#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)
	dev_put(entry->physin);
	dev_put(entry->physout);
#endif
}

void nf_queue_entry_free(struct nf_queue_entry *entry)
{
	nf_queue_entry_release_refs(entry);
	kfree(entry);
}
EXPORT_SYMBOL_GPL(nf_queue_entry_free);

static void __nf_queue_entry_init_physdevs(struct nf_queue_entry *entry)
{
#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)
	const struct sk_buff *skb = entry->skb;

	if (nf_bridge_info_exists(skb)) {
		entry->physin = nf_bridge_get_physindev(skb, entry->state.net);
		entry->physout = nf_bridge_get_physoutdev(skb);
	} else {
		entry->physin = NULL;
		entry->physout = NULL;
	}
#endif
}

/* Bump dev refs so they don't vanish while packet is out */
bool nf_queue_entry_get_refs(struct nf_queue_entry *entry)
{
	struct nf_hook_state *state = &entry->state;

	if (state->sk && !refcount_inc_not_zero(&state->sk->sk_refcnt))
		return false;

	dev_hold(state->in);
	dev_hold(state->out);

#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)
	dev_hold(entry->physin);
	dev_hold(entry->physout);
#endif
	return true;
}
EXPORT_SYMBOL_GPL(nf_queue_entry_get_refs);

void nf_queue_nf_hook_drop(struct net *net)
{
	const struct nf_queue_handler *qh;

	rcu_read_lock();
	qh = rcu_dereference(nf_queue_handler);
	if (qh)
		qh->nf_hook_drop(net);
	rcu_read_unlock();
}
EXPORT_SYMBOL_GPL(nf_queue_nf_hook_drop);

static void nf_ip_saveroute(const struct sk_buff *skb,
			    struct nf_queue_entry *entry)
{
	struct ip_rt_info *rt_info = nf_queue_entry_reroute(entry);

	if (entry->state.hook == NF_INET_LOCAL_OUT) {
		const struct iphdr *iph = ip_hdr(skb);

		rt_info->tos = iph->tos;
		rt_info->daddr = iph->daddr;
		rt_info->saddr = iph->saddr;
		rt_info->mark = skb->mark;
	}
}

static void nf_ip6_saveroute(const struct sk_buff *skb,
			     struct nf_queue_entry *entry)
{
	struct ip6_rt_info *rt_info = nf_queue_entry_reroute(entry);

	if (entry->state.hook == NF_INET_LOCAL_OUT) {
		const struct ipv6hdr *iph = ipv6_hdr(skb);

		rt_info->daddr = iph->daddr;
		rt_info->saddr = iph->saddr;
		rt_info->mark = skb->mark;
	}
}

static int __nf_queue(struct sk_buff *skb, const struct nf_hook_state *state,
		      unsigned int index, unsigned int verdict)
{
	struct nf_queue_entry *entry = NULL;
	const struct nf_queue_handler *qh;
	unsigned int route_key_size;
	unsigned int queuetype = verdict & NF_VERDICT_MASK;
	unsigned int queuenum  = verdict >> NF_VERDICT_QBITS;
	int status;

	/* QUEUE == DROP if no one is waiting, to be safe. */
	if (queuetype == NF_IMQ_QUEUE) {
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		qh = rcu_dereference(queue_imq_handler);
#else
		BUG();
		return -ESRCH;
#endif
	} else {
		qh = rcu_dereference(nf_queue_handler);
	}

	if (!qh)
		return -ESRCH;

	switch (state->pf) {
	case AF_INET:
		route_key_size = sizeof(struct ip_rt_info);
		break;
	case AF_INET6:
		route_key_size = sizeof(struct ip6_rt_info);
		break;
	default:
		route_key_size = 0;
		break;
	}

	if (skb_sk_is_prefetched(skb)) {
		struct sock *sk = skb->sk;

		if (!sk_is_refcounted(sk)) {
			if (!refcount_inc_not_zero(&sk->sk_refcnt))
				return -ENOTCONN;

			/* drop refcount on skb_orphan */
			skb->destructor = sock_edemux;
		}
	}

	entry = kmalloc(sizeof(*entry) + route_key_size, GFP_ATOMIC);
	if (!entry)
		return -ENOMEM;

	if (skb_dst(skb) && !skb_dst_force(skb)) {
		kfree(entry);
		return -ENETDOWN;
	}

	*entry = (struct nf_queue_entry) {
		.skb	= skb,
		.state	= *state,
		.hook_index = index,
		.size	= sizeof(*entry) + route_key_size,
	};

	__nf_queue_entry_init_physdevs(entry);

	if (!nf_queue_entry_get_refs(entry)) {
		kfree(entry);
		return -ENOTCONN;
	}

	switch (entry->state.pf) {
	case AF_INET:
		nf_ip_saveroute(skb, entry);
		break;
	case AF_INET6:
		nf_ip6_saveroute(skb, entry);
		break;
	}

	status = qh->outfn(entry, queuenum);
	if (status < 0) {
		nf_queue_entry_free(entry);
		return status;
	}

	return 0;
}

/* Packets leaving via this function must come back through nf_reinject(). */
int nf_queue(struct sk_buff *skb, struct nf_hook_state *state,
	     unsigned int index, unsigned int verdict)
{
	int ret;

	ret = __nf_queue(skb, state, index, verdict);
	if (ret < 0) {

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	/* IMQ Bypass */
	if (ret == -ECANCELED && skb->imq_flags == 0) {
		return 1;
	}
#endif

		if (ret == -ESRCH &&
		    (verdict & NF_VERDICT_FLAG_QUEUE_BYPASS))
			return 1;
		kfree_skb(skb);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(nf_queue);
