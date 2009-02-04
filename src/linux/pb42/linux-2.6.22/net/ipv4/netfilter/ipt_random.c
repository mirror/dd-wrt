/*
  This is a module which is used for a "random" match support.
  This file is distributed under the terms of the GNU General Public
  License (GPL). Copies of the GPL can be obtained from:
     ftp://prep.ai.mit.edu/pub/gnu/GPL

  2001-10-14 Fabrice MARIE <fabrice@netfilter.org> : initial implementation.
*/

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/random.h>
#include <net/tcp.h>
#include <linux/spinlock.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_random.h>
#include <linux/netfilter/x_tables.h>

MODULE_LICENSE("GPL");


static bool ipt_rand_match(const struct sk_buff *skb,
		  const struct net_device *in, const struct net_device *out,
		  const struct xt_match *match, const void *matchinfo,
		  int offset, unsigned int protoff, bool *hotdrop)
{
	/* Parameters from userspace */
	const struct ipt_rand_info *info = matchinfo;
	u_int8_t random_number;

	/* get 1 random number from the kernel random number generation routine */
	get_random_bytes((void *)(&random_number), 1);

	/* Do we match ? */
	if (random_number <= info->average)
		return 1;
	else
		return 0;
}

static bool ipt_rand_checkentry(const char *tablename,
	   const void *e,
	   const struct xt_match *match,
	   void *matchinfo,
	   unsigned int hook_mask)
{
	/* Parameters from userspace */
	const struct ipt_rand_info *info = matchinfo;

	/* must be  1 <= average % <= 99 */
	/* 1  x 2.55 = 2   */
	/* 99 x 2.55 = 252 */
	if ((info->average < 2) || (info->average > 252)) {
		printk("ipt_random:  invalid average %u\n", info->average);
		return 0;
	}

	return 1;
}

static struct xt_match ipt_rand_reg = { 
	.name = "random",
	.family = AF_INET,
	.match = ipt_rand_match,
	.checkentry = ipt_rand_checkentry,
	.matchsize	= sizeof(struct ipt_rand_info),
	.me = THIS_MODULE 
	};

static int __init init(void)
{
	if (xt_register_match(&ipt_rand_reg))
		return -EINVAL;

	printk("ipt_random match loaded\n");
	return 0;
}

static void __exit fini(void)
{
	xt_unregister_match(&ipt_rand_reg);
	printk("ipt_random match unloaded\n");
}

module_init(init);
module_exit(fini);
