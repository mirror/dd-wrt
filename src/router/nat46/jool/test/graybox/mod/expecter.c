#include "expecter.h"

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/sort.h>
#include "common/types.h"
#include "mod/common/address.h"
#include "diff.h"
#include "log.h"
#include "util.h"

struct expecter_node {
	struct expected_packet pkt;
	struct list_head list_hook;
};

struct netfilter_hook {
	struct net *ns;
	struct list_head nodes;
	struct list_head list_hook;
};

static LIST_HEAD(hooks);
static struct graybox_stats stats;

static int expecter_handle_pkt(struct sk_buff *actual);

unsigned int hook_cb(void *priv, struct sk_buff *skb,
		const struct nf_hook_state *nhs)
{
	log_debug("========= Graybox: Received packet =========");
	return expecter_handle_pkt(skb);
}

static struct nf_hook_ops nfho[] = {
	{
		.hook = hook_cb,
		.pf = PF_INET6,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP6_PRI_FIRST + 25,
	},
	{
		.hook = hook_cb,
		.pf = PF_INET,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FIRST + 25,
	}
};

int expecter_setup(void)
{
	memset(&stats, 0, sizeof(stats));
	return 0;
}

void expecter_teardown(void)
{
	struct list_head *node;
	struct netfilter_hook *hook;

	expecter_flush();

	while (!list_empty(&hooks)) {
		log_info("Deleting hook.");
		node = hooks.next;
		list_del(node);
		hook = list_entry(node, struct netfilter_hook, list_hook);

		nf_unregister_net_hooks(hook->ns, nfho, ARRAY_SIZE(nfho));

		put_net(hook->ns);
		WARN(!list_empty(&hook->nodes), "hook node list is not empty");
		kfree(hook);
	}
}

static void log_intro(struct sk_buff *skb)
{
	struct iphdr *hdr4;
	struct ipv6hdr *hdr6;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		hdr4 = ip_hdr(skb);
		log_debug("Received packet %pI4 -> %pI4",
				&hdr4->saddr, &hdr4->daddr);
		break;

	case htons(ETH_P_IPV6):
		hdr6 = ipv6_hdr(skb);
		log_debug("Received packet %pI6c -> %pI6c",
				&hdr6->saddr, &hdr6->daddr);
		break;

	default:
		log_debug("Received packet. (Unknown protocol.)");
	}
}

static void free_node(struct expecter_node *node)
{
	if (!node)
		return;

	if (node->pkt.bytes)
		kfree(node->pkt.bytes);
	if (node->pkt.filename)
		kfree(node->pkt.filename);
	kfree(node);
}

static int be16_compare(const void *a, const void *b)
{
	return *(__u16 *)a - *(__u16 *)b;
}

static void be16_swap(void *a, void *b, int size)
{
	__u16 t = *(__u16 *)a;
	*(__u16 *)a = *(__u16 *)b;
	*(__u16 *)b = t;
}

static void sort_exceptions(struct expected_packet *pkt)
{
	__u16 *list = pkt->exceptions.values;
	int list_length = pkt->exceptions.count;
	unsigned int i, j;

	/* Sort ascending. */
	sort(list, list_length, sizeof(*list), be16_compare, be16_swap);

	/* Remove duplicates. */
	for (i = 0, j = 1; j < list_length; j++) {
		if (list[i] != list[j]) {
			i++;
			list[i] = list[j];
		}
	}

	pkt->exceptions.count = i + 1;
}

static struct netfilter_hook *__get_hook(struct net *ns)
{
	struct netfilter_hook *hook;

	list_for_each_entry(hook, &hooks, list_hook)
		if (hook->ns == ns)
			return hook;

	return NULL;
}

static struct netfilter_hook *get_hook(void)
{
	struct netfilter_hook *hook;
	struct net *ns;
	int error;

	ns = get_net_ns_by_pid(task_pid_vnr(current));
	if (IS_ERR(ns)) {
		log_err("Could not retrieve the current namespace: %ld",
				PTR_ERR(ns));
		return NULL;
	}

	hook = __get_hook(ns);
	if (hook) {
		put_net(ns);
		return hook;
	}

	hook = kmalloc(sizeof(struct netfilter_hook), GFP_KERNEL);
	if (!hook) {
		put_net(ns);
		return NULL;
	}

	hook->ns = ns;
	INIT_LIST_HEAD(&hook->nodes);
	list_add(&hook->list_hook, &hooks);

	error = nf_register_net_hooks(ns, nfho, ARRAY_SIZE(nfho));
	if (error) {
		log_info("nf_register_net_hooks() error: %d", error);
		list_del(&hook->list_hook);
		kfree(hook);
		put_net(ns);
		return NULL;
	}

	return hook;
}

int expecter_add(struct expected_packet *pkt)
{
	struct netfilter_hook *hook;
	struct expecter_node *node;

	if (pkt->bytes_len == 0) {
		log_err("The packet is zero bytes long.");
		return -EINVAL;
	}

	node = kmalloc(sizeof(struct expecter_node), GFP_KERNEL);
	if (!node)
		goto enomem;
	memset(node, 0, sizeof(*node));

	node->pkt.filename = kmalloc(strlen(pkt->filename) + 1, GFP_KERNEL);
	if (!node->pkt.filename)
		goto enomem;
	strcpy(node->pkt.filename, pkt->filename);

	node->pkt.bytes = kmalloc(pkt->bytes_len, GFP_KERNEL);
	if (!node->pkt.bytes)
		goto enomem;
	memcpy(node->pkt.bytes, pkt->bytes, pkt->bytes_len);
	node->pkt.bytes_len = pkt->bytes_len;

	if (pkt->exceptions.count) {
		node->pkt.exceptions = pkt->exceptions;
		sort_exceptions(&node->pkt);
	}

	hook = get_hook();
	if (!hook)
		goto enomem;

	list_add_tail(&node->list_hook, &hook->nodes);

	log_debug("Stored packet '%s'.", pkt->filename);
	return 0;

enomem:
	free_node(node);
	return -ENOMEM;
}

void expecter_flush(void)
{
	struct netfilter_hook *hook;
	struct expecter_node *node;

	list_for_each_entry(hook, &hooks, list_hook) {
		while (!list_empty(&hook->nodes)) {
			node = list_entry(hook->nodes.next,
					struct expecter_node,
					list_hook);
			list_del(&node->list_hook);

			log_info("Queued: %s", node->pkt.filename);
			switch (get_l3_proto(node->pkt.bytes)) {
			case 4:
				stats.ipv4.queued++;
				break;
			case 6:
				stats.ipv6.queued++;
				break;
			}

			free_node(node);
		}
	}
}

static bool has_same_addr6(struct ipv6hdr *hdr1, struct ipv6hdr *hdr2)
{
	bool result;

	result = addr6_equals(&hdr1->daddr, &hdr2->daddr)
			&& addr6_equals(&hdr1->saddr, &hdr2->saddr);
	log_debug("Incoming packet %s %pI6c -> %pI6c.",
			result ? "matches" : "does not match",
			&hdr1->saddr, &hdr1->daddr);

	return result;
}

static bool has_same_addr4(struct iphdr *hdr1, struct iphdr *hdr2)
{
	bool result;

	result = hdr1->daddr == hdr2->daddr && hdr1->saddr == hdr2->saddr;
	log_debug("Incoming packet %s %pI4 -> %pI4.",
			result ? "matches" : "does not match",
			&hdr1->saddr, &hdr1->daddr);

	return result;
}

static bool has_same_address(struct expected_packet *expected,
		struct sk_buff *actual)
{
	int expected_proto = get_l3_proto(expected->bytes);
	int actual_proto = get_l3_proto(skb_network_header(actual));

	if (expected_proto != actual_proto) {
		log_debug("Incoming packet does not match l3 protocol of expected packet.");
		return false;
	}

	switch (expected_proto) {
	case 4:
		return has_same_addr4((struct iphdr *) expected->bytes,
				ip_hdr(actual));
	case 6:
		return has_same_addr6((struct ipv6hdr *) expected->bytes,
				ipv6_hdr(actual));
	}

	return false;
}

static unsigned int old_algorithm(struct expected_packet *expected,
		struct sk_buff *actual)
{
	unsigned char *actual_ptr;
	size_t i;
	size_t min_len;
	__u16 skip;

	/* BTW: The old algorithm does not account for paging. Weird. */
	actual_ptr = skb_network_header(actual);
	min_len = min(expected->bytes_len, (size_t)actual->len);
	skip = 0;

	for (i = 0; i < min_len; i++) {
		if (skip < expected->exceptions.count
				&& expected->exceptions.values[skip] == i) {
			skip++;
			continue;
		}

		if (expected->bytes[i] != actual_ptr[i])
			return 1;
	}

	return 0;
}

static bool pkt_equals(struct expected_packet *expected, struct sk_buff *actual)
{
	unsigned int errors_new = 0;
	unsigned int errors_old = 0;

	log_info("Packet %s (length %zu):", expected->filename,
			expected->bytes_len);

	if (expected->bytes_len != actual->len) {
		log_info("\tLength:");
		log_info("\t\tExpected: %zu", expected->bytes_len);
		log_info("\t\tActual: %u", actual->len);
		errors_new++;
		errors_old++;
	}

	errors_new += collect_errors(expected, actual);
	errors_old += old_algorithm(expected, actual);

	if (!!errors_new != !!errors_old) {
		log_err("Error: The new algorithm %s errors, the old algorithm did %s errors.",
				errors_new ? "yielded" : "did not yield",
				errors_old ? "yield" : "not yield");
		errors_new++;
	}

	return !errors_new;
}

static struct graybox_proto_stats *get_stats(struct expected_packet *pkt)
{
	switch (get_l3_proto(pkt->bytes)) {
	case 4:
		return &stats.ipv4;
	case 6:
		return &stats.ipv6;
	}

	return NULL;
}

static int expecter_handle_pkt(struct sk_buff *actual)
{
	struct netfilter_hook *hook;
	struct expecter_node *node;
	struct expected_packet *expected;
	struct graybox_proto_stats *stats;

	log_intro(actual);

	hook = __get_hook(dev_net(actual->dev));
	if (!hook || list_empty(&hook->nodes)) {
		log_debug("No packets queued.");
		return NF_ACCEPT;
	}

	node = list_entry(hook->nodes.next, struct expecter_node, list_hook);
	expected = &node->pkt;

	if (!has_same_address(expected, actual))
		return NF_ACCEPT;

	list_del(&node->list_hook);

	log_debug("Received packet matches '%s'.", expected->filename);

	stats = get_stats(expected);
	if (WARN(!stats, "Unreachable code: protocol was already validated."))
		return NF_ACCEPT;

	if (pkt_equals(expected, actual)) {
		stats->successes++;
	} else {
		stats->failures++;
	}

	free_node(node);
	return NF_DROP;
}

void expecter_stat(struct graybox_stats *result)
{
	struct netfilter_hook *hook;
	struct expecter_node *node;

	memcpy(result, &stats, sizeof(stats));

	list_for_each_entry(hook, &hooks, list_hook) {
		list_for_each_entry(node, &hook->nodes, list_hook) {
			switch (get_l3_proto(node->pkt.bytes)) {
			case 4:
				result->ipv4.queued++;
				break;
			case 6:
				result->ipv6.queued++;
				break;
			}
		}
	}
}

void expecter_stat_flush(void)
{
	memset(&stats, 0, sizeof(stats));
}
