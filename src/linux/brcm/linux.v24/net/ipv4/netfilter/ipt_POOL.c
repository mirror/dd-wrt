/* ipt_POOL.c - netfilter target to manipulate IP pools
 *
 * This target can be used almost everywhere. It acts on some specified
 * IP pool, adding or deleting some IP address in the pool. The address
 * can be either the source (--addsrc, --delsrc), or destination (--add/deldst)
 * of the packet under inspection.
 *
 * The target normally returns IPT_CONTINUE.
 */

#include <linux/types.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/inetdevice.h>
#include <net/protocol.h>
#include <net/checksum.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ipt_pool.h>

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

/*** NOTE NOTE NOTE NOTE ***
**
** By sheer luck, I get away with using the "struct ipt_pool_info", as defined
** in <linux/netfilter_ipv4/ipt_pool.h>, both as the match and target info.
** Here, in the target implementation, ipt_pool_info.src, if not IP_POOL_NONE,
** is modified for the source IP address of the packet under inspection.
** The same way, the ipt_pool_info.dst pool is modified for the destination.
**
** The address is added to the pool normally. However, if IPT_POOL_DEL_dir
** flag is set in ipt_pool_info.flags, the address is deleted from the pool.
**
** If a modification was done to the pool, we possibly return ACCEPT or DROP,
** if the right IPT_POOL_MOD_dir_ACCEPT or _MOD_dir_DROP flags are set.
** The IPT_POOL_INV_MOD_dir flag inverts the sense of the check (i.e. the
** ACCEPT and DROP flags are evaluated when the pool was not modified.)
*/

static int
do_check(const char *tablename,
	       const struct ipt_entry *e,
	       void *targinfo,
	       unsigned int targinfosize,
	       unsigned int hook_mask)
{
	const struct ipt_pool_info *ipi = targinfo;

	if (targinfosize != IPT_ALIGN(sizeof(*ipi))) {
		DEBUGP("POOL_check: size %u.\n", targinfosize);
		return 0;
	}
	DEBUGP("ipt_POOL:do_check(%d,%d,%d)\n",ipi->src,ipi->dst,ipi->flags);
	return 1;
}

static unsigned int
do_target(struct sk_buff **pskb,
		unsigned int hooknum,
		const struct net_device *in,
		const struct net_device *out,
		const void *targinfo,
		void *userinfo)
{
	const struct ipt_pool_info *ipi = targinfo;
	int modified;
	unsigned int verdict = IPT_CONTINUE;

	if (ipi->src != IP_POOL_NONE) {
		modified = ip_pool_mod(ipi->src, ntohl((*pskb)->nh.iph->saddr),
					ipi->flags & IPT_POOL_DEL_SRC);
		if (!!modified ^ !!(ipi->flags & IPT_POOL_INV_MOD_SRC)) {
			if (ipi->flags & IPT_POOL_MOD_SRC_ACCEPT)
				verdict = NF_ACCEPT;
			else if (ipi->flags & IPT_POOL_MOD_SRC_DROP)
				verdict = NF_DROP;
		}
	}
	if (verdict == IPT_CONTINUE && ipi->dst != IP_POOL_NONE) {
		modified = ip_pool_mod(ipi->dst, ntohl((*pskb)->nh.iph->daddr),
					ipi->flags & IPT_POOL_DEL_DST);
		if (!!modified ^ !!(ipi->flags & IPT_POOL_INV_MOD_DST)) {
			if (ipi->flags & IPT_POOL_MOD_DST_ACCEPT)
				verdict = NF_ACCEPT;
			else if (ipi->flags & IPT_POOL_MOD_DST_DROP)
				verdict = NF_DROP;
		}
	}
	return verdict;
}

static struct ipt_target pool_reg
= { { NULL, NULL }, "POOL", do_target, do_check, NULL, THIS_MODULE };

static int __init init(void)
{
	DEBUGP("init ipt_POOL\n");
	return ipt_register_target(&pool_reg);
}

static void __exit fini(void)
{
	DEBUGP("fini ipt_POOL\n");
	ipt_unregister_target(&pool_reg);
}

module_init(init);
module_exit(fini);
