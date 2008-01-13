/* Kernel module to match an IP address pool. */

#include <linux/module.h>
#include <linux/ip.h>
#include <linux/skbuff.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_pool.h>
#include <linux/netfilter_ipv4/ipt_pool.h>

static inline int match_pool(
	ip_pool_t index,
	__u32 addr,
	int inv
) {
	if (ip_pool_match(index, ntohl(addr)))
		inv = !inv;
	return inv;
}

static int match(
	const struct sk_buff *skb,
	const struct net_device *in,
	const struct net_device *out,
	const void *matchinfo,
	int offset,
	const void *hdr,
	u_int16_t datalen,
	int *hotdrop
) {
	const struct ipt_pool_info *info = matchinfo;
	const struct iphdr *iph = skb->nh.iph;

	if (info->src != IP_POOL_NONE && !match_pool(info->src, iph->saddr,
						info->flags&IPT_POOL_INV_SRC))
		return 0;

	if (info->dst != IP_POOL_NONE && !match_pool(info->dst, iph->daddr,
						info->flags&IPT_POOL_INV_DST))
		return 0;

	return 1;
}

static int checkentry(
	const char *tablename,
	const struct ipt_ip *ip,
	void *matchinfo,
	unsigned int matchsize,
	unsigned int hook_mask
) {
	if (matchsize != IPT_ALIGN(sizeof(struct ipt_pool_info)))
		return 0;
	return 1;
}

static struct ipt_match pool_match
= { { NULL, NULL }, "pool", &match, &checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	return ipt_register_match(&pool_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&pool_match);
}

module_init(init);
module_exit(fini);
