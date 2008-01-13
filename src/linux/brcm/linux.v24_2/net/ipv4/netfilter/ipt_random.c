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

MODULE_LICENSE("GPL");

static int
ipt_rand_match(const struct sk_buff *pskb,
	       const struct net_device *in,
	       const struct net_device *out,
	       const void *matchinfo,
	       int offset,
	       const void *hdr,
	       u_int16_t datalen,
	       int *hotdrop)
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

static int
ipt_rand_checkentry(const char *tablename,
		   const struct ipt_ip *e,
		   void *matchinfo,
		   unsigned int matchsize,
		   unsigned int hook_mask)
{
	/* Parameters from userspace */
	const struct ipt_rand_info *info = matchinfo;

	if (matchsize != IPT_ALIGN(sizeof(struct ipt_rand_info))) {
		printk("ipt_random: matchsize %u != %u\n", matchsize,
		       IPT_ALIGN(sizeof(struct ipt_rand_info)));
		return 0;
	}

	/* must be  1 <= average % <= 99 */
	/* 1  x 2.55 = 2   */
	/* 99 x 2.55 = 252 */
	if ((info->average < 2) || (info->average > 252)) {
		printk("ipt_random:  invalid average %u\n", info->average);
		return 0;
	}

	return 1;
}

static struct ipt_match ipt_rand_reg = { 
	{NULL, NULL},
	"random",
	ipt_rand_match,
	ipt_rand_checkentry,
	NULL,
	THIS_MODULE };

static int __init init(void)
{
	if (ipt_register_match(&ipt_rand_reg))
		return -EINVAL;

	printk("ipt_random match loaded\n");
	return 0;
}

static void __exit fini(void)
{
	ipt_unregister_match(&ipt_rand_reg);
	printk("ipt_random match unloaded\n");
}

module_init(init);
module_exit(fini);
