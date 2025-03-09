/*
  This is a module which is used for PSD (portscan detection)
  Derived from scanlogd v2.1 written by Solar Designer <solar@false.com>
  and LOG target module.

  Copyright (C) 2000,2001 astaro AG

  This file is distributed under the terms of the GNU General Public
  License (GPL). Copies of the GPL can be obtained from:
     ftp://prep.ai.mit.edu/pub/gnu/GPL

  2000-05-04 Markus Hennig <hennig@astaro.de> : initial
  2000-08-18 Dennis Koslowski <koslowski@astaro.de> : first release
  2000-12-01 Dennis Koslowski <koslowski@astaro.de> : UDP scans detection added
  2001-01-02 Dennis Koslowski <koslowski@astaro.de> : output modified
  2001-02-04 Jan Rekorajski <baggins@pld.org.pl> : converted from target to match
  2004-05-05 Martijn Lievaart <m@rtij.nl> : ported to 2.6
  2007-04-05 Mohd Nawawi Mohamad Jamili <nawawi@tracenetworkcorporation.com> : ported to 2.6.18
  2008-03-21 Mohd Nawawi Mohamad Jamili <nawawi@tracenetworkcorporation.com> : ported to 2.6.24
  2009-08-07 Mohd Nawawi Mohamad Jamili <nawawi@tracenetworkcorporation.com> : ported to xtables-addons
*/

#define pr_fmt(x) KBUILD_MODNAME ": " x
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/tcp.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include "xt_psd.h"
#include "compat_xtables.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dennis Koslowski <koslowski@astaro.com>");
MODULE_AUTHOR("Martijn Lievaart <m@rtij.nl>");
MODULE_AUTHOR("Jan Rekorajski <baggins@pld.org.pl>");
MODULE_AUTHOR(" Mohd Nawawi Mohamad Jamili <nawawi@tracenetworkcorporation.com>");
MODULE_DESCRIPTION("Xtables: PSD - portscan detection");
MODULE_ALIAS("ipt_psd");
MODULE_ALIAS("ip6t_psd");

/*
 * Keep track of up to LIST_SIZE source addresses, using a hash table of
 * PSD_HASH_SIZE entries for faster lookups, but limiting hash collisions to
 * HASH_MAX source addresses per the same hash value.
 */
#define LIST_SIZE			0x100
#define HASH_LOG			9
#define PSD_HASH_SIZE			(1 << HASH_LOG)
#define HASH_MAX			0x10
#if IS_ENABLED(CONFIG_IP6_NF_IPTABLES)
#	define WITH_IPV6 1
#endif

/*
 * Information we keep per each target port
 */
struct port {
	u_int16_t number;      /* port number */
	u_int8_t proto;        /* protocol number */
};

/**
 * Information we keep per each source address.
 * @next:	next entry with the same hash
 * @timestamp:	last update time
 * @count:	number of ports in the list
 * @weight:	total weight of ports in the list
 */
struct host {
	struct host *next;
	unsigned long timestamp;
	__be16 src_port;
	uint16_t count;
	uint8_t weight;
	struct port ports[SCAN_MAX_COUNT-1];
};

/**
 * Information we keep per ipv4 source address.
 */
struct host4 {
	struct host host;
	__be32 saddr;
};

static struct host4 *host_to_host4(const struct host *h)
{
	return (struct host4 *)h;
}

struct host6 {
	struct host host;
	struct in6_addr saddr;
};

/**
 * State information for IPv4 portscan detection.
 * @list:	list of source addresses
 * @hash:	pointers into the list
 * @index:	oldest entry to be replaced
 */
static struct {
	spinlock_t lock;
	struct host4 list[LIST_SIZE];
	struct host *hash[PSD_HASH_SIZE];
	int index;
} state;

#ifdef WITH_IPV6
/**
 * State information for IPv6 portscan detection.
 * @list:	list of source addresses
 * @hash:	pointers into the list
 * @index:	oldest entry to be replaced
 */
static struct {
	spinlock_t lock;
	struct host6 *list;
	struct host **hash;
	int index;
} state6;

static struct host6 *host_to_host6(const struct host *h)
{
	return (struct host6 *) h;
}

/**
 * allocate state6 memory only when needed
 */
static bool state6_alloc_mem(void)
{
	if (state6.hash != NULL)
		return true;

	state6.list = vmalloc(LIST_SIZE * sizeof(struct host6));
	if (state6.list == NULL)
		return false;
	memset(state6.list, 0, LIST_SIZE * sizeof(struct host6));
	state6.hash = vmalloc(PSD_HASH_SIZE * sizeof(struct host *));
	if (state6.hash == NULL) {
		vfree(state6.list);
		return false;
	}
	memset(state6.hash, 0, PSD_HASH_SIZE * sizeof(struct host *));
	return true;
}
#endif

/*
 * Convert an IP address into a hash table index.
 */
static unsigned int hashfunc(__be32 addr)
{
	unsigned int value;
	unsigned int hash;

	value = addr;
	hash = 0;
	do {
		hash ^= value;
	} while ((value >>= HASH_LOG) != 0);
	return hash & (PSD_HASH_SIZE - 1);
}

static inline unsigned int hashfunc6(const struct in6_addr *addr)
{
	__be32 h = addr->s6_addr32[0] ^ addr->s6_addr32[1];
	return hashfunc(h ^ addr->s6_addr32[2] ^ addr->s6_addr32[3]);
}

static bool port_in_list(struct host *host, uint8_t proto, uint16_t port)
{
	unsigned int i;

	for (i = 0; i < host->count; ++i) {
		if (host->ports[i].proto != proto)
			continue;
		if (host->ports[i].number == port)
			return true;
	}
	return false;
}

static uint16_t get_port_weight(const struct xt_psd_info *psd, __be16 port)
{
	return ntohs(port) < 1024 ? psd->lo_ports_weight : psd->hi_ports_weight;
}

static bool
is_portscan(struct host *host, const struct xt_psd_info *psdinfo,
            const struct tcphdr *tcph, uint8_t proto)
{
	if (port_in_list(host, proto, tcph->dest))
		return false;

	/*
	 * TCP/ACK and/or TCP/RST to a new port? This could be an
	 * outgoing connection.
	 */
	if (proto == IPPROTO_TCP && (tcph->ack || tcph->rst))
		return false;

	host->timestamp = jiffies;

	if (host->weight >= psdinfo->weight_threshold) /* already matched */
		return true;

	/* Update the total weight */
	host->weight += get_port_weight(psdinfo, tcph->dest);

	/* Got enough destination ports to decide that this is a scan? */
	if (host->weight >= psdinfo->weight_threshold)
		return true;

	/* Remember the new port */
	if (host->count < ARRAY_SIZE(host->ports)) {
		host->ports[host->count].number = tcph->dest;
		host->ports[host->count].proto = proto;
		host->count++;
	}
	return false;
}

static struct host *host_get_next(struct host *h, struct host **last)
{
	if (h->next != NULL)
		*last = h;
	return h->next;
}

static void ht_unlink(struct host **head, struct host *last)
{
	if (last != NULL)
		last->next = last->next->next;
	else if (*head != NULL)
		*head = (*head)->next;
}

static bool
entry_is_recent(const struct host *h, unsigned long delay_threshold,
                unsigned long now)
{
	return now - h->timestamp <= (delay_threshold * HZ) / 100 &&
	       time_after_eq(now, h->timestamp);
}

static void remove_oldest(struct host **head, struct host *curr)
{
	struct host *h, *last = NULL;

	/*
	 * We are going to reuse the oldest list entry, so remove it from the
	 * hash table first, if it is really already in use.
	 */
	h = *head;
	while (h != NULL) {
		if (curr == h)
			break;
		last = h;
		h = h->next;
	}

	/* Then, remove it */
	if (h != NULL)
		ht_unlink(head, last);
}

static void *
get_header_pointer4(const struct sk_buff *skb, unsigned int thoff, void *mem)
{
	const struct iphdr *iph = ip_hdr(skb);
	int hdrlen;

	switch (iph->protocol) {
	case IPPROTO_TCP:
		hdrlen = sizeof(struct tcphdr);
		break;
	case IPPROTO_UDP:
	case IPPROTO_UDPLITE:
		hdrlen = sizeof(struct udphdr);
		break;
	default:
		return NULL;
	}

	return skb_header_pointer(skb, thoff, hdrlen, mem);
}

static bool
handle_packet4(const struct iphdr *iph, const struct tcphdr *tcph,
               const struct xt_psd_info *psdinfo, unsigned int hash)
{
	unsigned long now;
	struct host *curr, *last = NULL, **head;
	struct host4 *curr4;
	int count = 0;

	now = jiffies;
	head = &state.hash[hash];

	/* Do we know this source address already? */
	curr = *head;
	while (curr != NULL) {
		curr4 = host_to_host4(curr);
		if (curr4->saddr == iph->saddr)
			break;
		count++;
		curr = host_get_next(curr, &last);
	}

	if (curr != NULL) {
		/* We know this address, and the entry isn't too old. Update it. */
		if (entry_is_recent(curr, psdinfo->delay_threshold, now))
			return is_portscan(curr, psdinfo, tcph, iph->protocol);

		/* We know this address, but the entry is outdated. Mark it unused, and
		 * remove from the hash table. We'll allocate a new entry instead since
		 * this one might get reused too soon. */
		curr4 = host_to_host4(curr);
		curr4->saddr = 0;
		ht_unlink(head, last);
		last = NULL;
	}

	/* We don't need an ACK from a new source address */
	if (iph->protocol == IPPROTO_TCP && tcph->ack)
		return false;

	/* Got too many source addresses with the same hash value? Then remove the
	 * oldest one from the hash table, so that they can't take too much of our
	 * CPU time even with carefully chosen spoofed IP addresses. */
	if (count >= HASH_MAX && last != NULL)
		last->next = NULL;

	if (state.list[state.index].saddr != 0)
		head = &state.hash[hashfunc(state.list[state.index].saddr)];
	else
		head = &last;

	/* Get our list entry */
	curr4 = &state.list[state.index++];
	curr = &curr4->host;
	remove_oldest(head, curr);
	if (state.index >= LIST_SIZE)
		state.index = 0;

	/* Link it into the hash table */
	head = &state.hash[hash];
	curr->next = *head;
	*head = curr;

	/* And fill in the fields */
	curr4 = host_to_host4(curr);
	curr4->saddr = iph->saddr;
	curr->timestamp = now;
	curr->count = 1;
	curr->weight = get_port_weight(psdinfo, tcph->dest);
	curr->ports[0].number = tcph->dest;
	curr->ports[0].proto = iph->protocol;
	return false;
}

static bool
xt_psd_match(const struct sk_buff *pskb, struct xt_action_param *match)
{
	struct iphdr *iph = ip_hdr(pskb);
	struct tcphdr _tcph;
	struct tcphdr *tcph;
	bool matched;
	unsigned int hash;
	/* Parameters from userspace */
	const struct xt_psd_info *psdinfo = match->matchinfo;

	if (iph->frag_off & htons(IP_OFFSET)) {
		pr_debug("sanity check failed\n");
		return false;
	}

	/*
	 * We are using IP address 0.0.0.0 for a special purpose here, so do
	 * not let them spoof us. [DHCP needs this feature - HW]
	 */
	if (iph->saddr == 0) {
		pr_debug("spoofed source address (0.0.0.0)\n");
		return false;
	}

	tcph = get_header_pointer4(pskb, match->thoff, &_tcph);
	if (tcph == NULL)
		return false;

	hash = hashfunc(iph->saddr);

	spin_lock(&state.lock);
	matched = handle_packet4(iph, tcph, psdinfo, hash);
	spin_unlock(&state.lock);
	return matched;
}

#ifdef WITH_IPV6
static bool
handle_packet6(const struct ipv6hdr *ip6h, const struct tcphdr *tcph,
	       const struct xt_psd_info *psdinfo, uint8_t proto, int hash)
{
	unsigned long now;
	struct host *curr, *last = NULL, **head;
	struct host6 *curr6;
	int count = 0;

	now = jiffies;
	head = &state6.hash[hash];

	curr = *head;
	while (curr != NULL) {
		curr6 = host_to_host6(curr);
		if (ipv6_addr_equal(&curr6->saddr, &ip6h->saddr))
			break;
		count++;
		curr = host_get_next(curr, &last);
	}

	if (curr != NULL) {
		if (entry_is_recent(curr, psdinfo->delay_threshold, now))
			return is_portscan(curr, psdinfo, tcph, proto);
		curr6 = host_to_host6(curr);
		memset(&curr6->saddr, 0, sizeof(curr6->saddr));
		ht_unlink(head, last);
		last = NULL;
	}

	if (proto == IPPROTO_TCP && tcph->ack)
		return false;

	if (count >= HASH_MAX && last != NULL)
		last->next = NULL;

	if (!ipv6_addr_any(&state6.list[state6.index].saddr))
		head = &state6.hash[hashfunc6(&state6.list[state6.index].saddr)];
	else
		head = &last;

	curr6 = &state6.list[state6.index++];
	curr = &curr6->host;
	remove_oldest(head, curr);
	if (state6.index >= LIST_SIZE)
		state6.index = 0;

	head = &state6.hash[hash];
	curr->next = *head;
	*head = curr;

	curr6 = host_to_host6(curr);
	curr6->saddr = ip6h->saddr;
	curr->timestamp = now;
	curr->count = 1;
	curr->weight = get_port_weight(psdinfo, tcph->dest);
	curr->ports[0].number = tcph->dest;
	curr->ports[0].proto = proto;
	return false;
}

static void *
get_header_pointer6(const struct sk_buff *skb, void *mem, uint8_t *proto)
{
	static const uint8_t types[] = {IPPROTO_TCP,
				        IPPROTO_UDP, IPPROTO_UDPLITE};
	unsigned int i, offset = 0;
	int err;
	size_t hdrlen;

	for (i = 0; i < ARRAY_SIZE(types); ++i) {
		err = ipv6_find_hdr(skb, &offset, types[i], NULL, NULL);
		if (err < 0)
			continue;

		switch (types[i]) {
		case IPPROTO_TCP:
			hdrlen = sizeof(struct tcphdr);
			break;
		case IPPROTO_UDP:
		case IPPROTO_UDPLITE:
			hdrlen = sizeof(struct udphdr);
			break;
		default:
			return NULL;
		}
		*proto = types[i];
		return skb_header_pointer(skb, offset, hdrlen, mem);
	}
	return NULL;
}

static bool
xt_psd_match6(const struct sk_buff *pskb, struct xt_action_param *match)
{
	const struct ipv6hdr *ip6h = ipv6_hdr(pskb);
	struct tcphdr _tcph;
	struct tcphdr *tcph;
	uint8_t proto = 0;
	bool matched;
	int hash;
	const struct xt_psd_info *psdinfo = match->matchinfo;

	if (ipv6_addr_any(&ip6h->saddr))
		return false;

	tcph = get_header_pointer6(pskb, &_tcph, &proto);
	if (tcph == NULL)
		return false;

	hash = hashfunc6(&ip6h->saddr);

	spin_lock(&state6.lock);
	matched = handle_packet6(ip6h, tcph, psdinfo, proto, hash);
	spin_unlock(&state6.lock);
	return matched;
}
#endif

static int psd_mt_check(const struct xt_mtchk_param *par)
{
	const struct xt_psd_info *info = par->matchinfo;

	if (info->weight_threshold == 0)
		/* 0 would match on every 1st packet */
		return -EINVAL;

	if ((info->lo_ports_weight | info->hi_ports_weight) == 0)
		/* would never match */
		return -EINVAL;

	if (info->delay_threshold > PSD_MAX_RATE ||
	    info->weight_threshold > PSD_MAX_RATE ||
	    info->lo_ports_weight > PSD_MAX_RATE ||
	    info->hi_ports_weight > PSD_MAX_RATE)
		return -EINVAL;

	return 0;
}

#ifdef WITH_IPV6
static int psd_mt_check6(const struct xt_mtchk_param *par)
{
	if (!state6_alloc_mem())
		return -ENOMEM;
	return psd_mt_check(par);
}
#endif

static struct xt_match xt_psd_reg[] __read_mostly = {
	{
		.name       = "psd",
		.family     = NFPROTO_IPV4,
		.revision   = 1,
		.checkentry = psd_mt_check,
		.match      = xt_psd_match,
		.matchsize  = sizeof(struct xt_psd_info),
		.me         = THIS_MODULE,
#ifdef WITH_IPV6
	}, {
		.name       = "psd",
		.family     = NFPROTO_IPV6,
		.revision   = 1,
		.checkentry = psd_mt_check6,
		.match      = xt_psd_match6,
		.matchsize  = sizeof(struct xt_psd_info),
		.me         = THIS_MODULE,
#endif
	}
};

static int __init xt_psd_init(void)
{
	spin_lock_init(&(state.lock));
#ifdef WITH_IPV6
	spin_lock_init(&(state6.lock));
#endif
	return xt_register_matches(xt_psd_reg, ARRAY_SIZE(xt_psd_reg));
}

static void __exit xt_psd_exit(void)
{
        xt_unregister_matches(xt_psd_reg, ARRAY_SIZE(xt_psd_reg));
#ifdef WITH_IPV6
	vfree(state6.list);
	vfree(state6.hash);
#endif
}

module_init(xt_psd_init);
module_exit(xt_psd_exit);

