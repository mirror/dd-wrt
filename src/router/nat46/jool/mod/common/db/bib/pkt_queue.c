#include "pkt_queue.h"

#include "common/constants.h"
#include "mod/common/icmp_wrapper.h"
#include "mod/common/log.h"
#include "mod/common/wkmalloc.h"
#include "mod/common/db/rbtree.h"

struct pktqueue {
	/** The stored packets, sorted by expiration date. (oldest to newest) */
	struct list_head node_list;
	/** The same packets, sorted by IPv4 identifiers. */
	struct rb_root node_tree;
};

static unsigned long get_timeout(void)
{
	return msecs_to_jiffies(1000 * TCP_INCOMING_SYN);
}

static void send_icmp_error(struct pktqueue_session *node)
{
	icmp64_send(NULL, node->skb, ICMPERR_PORT_UNREACHABLE, 0);
	kfree_skb(node->skb);
	wkfree(struct pktqueue_session, node);
}

static void rm(struct pktqueue *queue, struct pktqueue_session *node)
{
	list_del(&node->list_hook);
	rb_erase(&node->tree_hook, &queue->node_tree);
}

struct pktqueue *pktqueue_alloc(void)
{
	struct pktqueue *result;

	result = wkmalloc(struct pktqueue, GFP_KERNEL);
	if (!result)
		return NULL;

	INIT_LIST_HEAD(&result->node_list);
	result->node_tree = RB_ROOT;

	return result;
}

void pktqueue_release(struct pktqueue *queue)
{
	struct pktqueue_session *node;
	struct pktqueue_session *tmp;

	list_for_each_entry_safe(node, tmp, &queue->node_list, list_hook)
		send_icmp_error(node);
	wkfree(struct pktqueue, queue);
}

static struct pktqueue_session *pktqueue_entry(const struct rb_node *node)
{
	return rb_entry(node, struct pktqueue_session, tree_hook);
}

static int compare_rbnode(const struct rb_node *n1, const struct rb_node *n2)
{
	return taddr6_compare(&pktqueue_entry(n1)->dst6,
			&pktqueue_entry(n2)->dst6);
}

/**
 * On success, assumes the caller's reference to @pkt's skb is being transferred
 * to @queue.
 *
 * The typical return values are
 * 0: success; packet stored.
 * -EEXIST: SO already exists; @pkt is redundant.
 * -ENOSPC: SO is valid but we're already storing too many packets. Please fail
 * gracefully somehow.
 */
int pktqueue_add(struct pktqueue *queue, struct packet *pkt,
		struct ipv6_transport_addr *dst6, bool too_many)
{
	struct pktqueue_session *new;
	struct rb_node *collision;
	struct tree_slot slot;

	new = wkmalloc(struct pktqueue_session, GFP_ATOMIC);
	if (!new)
		return -ENOMEM;
	new->dst6 = *dst6;

	collision = rbtree_find_slot(&new->tree_hook, &queue->node_tree,
			compare_rbnode, &slot);
	if (collision) {
		wkfree(struct pktqueue_session, new);
		/*
		 * Should we reset the timer of the existing session?
		 * Don't know; the RFC is silent on this.
		 * My guess is no; if we keep updating the timer and the user is
		 * not attempting a SO, the continuous stream of v4 SYN attempts
		 * will prevent the port unreachable from being sent until the
		 * user stops retrying. Since the user will not be listening at
		 * that point, the ICMP error will be pointless.
		 *
		 * Well, the client often increases the time window between SYNs
		 * to make up for congestion, but still. Six seconds flat should
		 * be enough to assemble a SO and too much lag already for
		 * someone who simply missed some port.
		 */
		return -EEXIST;
	}

	/*
	 * This is not handled by the caller because it MUST happen after the
	 * collision check above.
	 * -EEXIST has more priority and must never yield an ICMP error.
	 * -ENOSPC always yields an ICMP error.
	 * So if -ENOSPC is validated before, we would end up fetching lots of
	 * misplaced ICMP errors.
	 */
	if (too_many) {
		wkfree(struct pktqueue_session, new);
		return -ENOSPC;
	}

	new->src4 = pkt->tuple.dst.addr4;
	new->dst4 = pkt->tuple.src.addr4;
	new->skb = pkt_original_pkt(pkt)->skb;
	new->update_time = jiffies;
	treeslot_commit(&slot);
	list_add_tail(&new->list_hook, &queue->node_list);
	return 0;
}

void pktqueue_rm(struct pktqueue *queue, struct ipv4_transport_addr *src4)
{
	struct pktqueue_session *node, *tmp;

	/*
	 * Yes, full traversal. @src4 is not indexed and this is a rare
	 * operation anyway.
	 * Not to mention, the pkt queue is not allowed to grow much.
	 */
	list_for_each_entry_safe(node, tmp, &queue->node_list, list_hook) {
		if (taddr4_equals(&node->src4, src4)) {
			list_del(&node->list_hook);
			rb_erase(&node->tree_hook, &queue->node_tree);
			kfree_skb(node->skb);
			wkfree(struct pktqueue_session, node);
		}
	}
}

/**
 * Returns > 0 if node.session.*4 > session.*4.
 * Returns < 0 if node.session.*4 < session.*4.
 * Returns 0 if node.session.*4 == session.*4.
 */
static int compare_simple(const struct pktqueue_session *node,
		const struct ipv6_transport_addr *addr)
{
	return taddr6_compare(&node->dst6, addr);
}

static struct pktqueue_session *__tree_find(struct pktqueue *queue,
		struct ipv6_transport_addr *addr)
{
	return rbtree_find(addr, &queue->node_tree, compare_simple,
			struct pktqueue_session, tree_hook);
}

struct pktqueue_session *pktqueue_find(struct pktqueue *queue,
		struct ipv6_transport_addr *addr,
		struct mask_domain *masks)
{
	struct pktqueue_session *node;

	node = __tree_find(queue, addr);
	if (!node)
		return NULL;

	if (masks && !mask_domain_matches(masks, &node->src4))
		return NULL;

	rm(queue, node);
	return node;
}

void pktqueue_put_node(struct xlator *jool, struct pktqueue_session *node)
{
	__log_debug(jool, "Deleting stored type 1 packet.");
	kfree_skb(node->skb);
	wkfree(struct pktqueue_session, node);
}

/**
 * This function always succeeds; the return value is the number of packets in
 * @probes.
 */
unsigned int pktqueue_prepare_clean(struct pktqueue *queue,
		struct list_head *probes)
{
	struct pktqueue_session *node, *tmp;
	const unsigned long TIMEOUT = get_timeout();
	unsigned int removed = 0;

	list_for_each_entry_safe(node, tmp, &queue->node_list, list_hook) {
		/*
		 * "list" is sorted by expiration date,
		 * so stop on the first unexpired session.
		 */
		if (time_before(jiffies, node->update_time + TIMEOUT))
			break;

		rm(queue, node);
		list_add(&node->list_hook, probes);
		removed++;
	}

	return removed;
}

void pktqueue_clean(struct list_head *probes)
{
	struct pktqueue_session *node, *tmp;
	list_for_each_entry_safe(node, tmp, probes, list_hook)
		send_icmp_error(node);
}
