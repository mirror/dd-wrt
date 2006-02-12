/* 
 * netfilter module to enforce network quotas
 *
 * Sam Johnston <samj@samj.net>
 *
 * 30/01/05: Fixed on SMP --Pablo Neira <pablo@eurodev.net>
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_quota.h>

MODULE_LICENSE("GPL");

static spinlock_t quota_lock = SPIN_LOCK_UNLOCKED;

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset, const void *hdr, u_int16_t datalen, int *hotdrop)
{
	struct ipt_quota_info *q =
		((struct ipt_quota_info *) matchinfo)->master;

        spin_lock_bh(&quota_lock);

        if (q->quota >= datalen) {
                /* we can afford this one */
                q->quota -= datalen;
                spin_unlock_bh(&quota_lock);

#ifdef DEBUG_IPT_QUOTA
                printk("IPT Quota OK: %llu datlen %d \n", q->quota, datalen);
#endif
                return 1;
        }

        /* so we do not allow even small packets from now on */
        q->quota = 0;

#ifdef DEBUG_IPT_QUOTA
        printk("IPT Quota Failed: %llu datlen %d \n", q->quota, datalen);
#endif

        spin_unlock_bh(&quota_lock);
        return 0;
}

static int
checkentry(const char *tablename,
           const struct ipt_ip *ip,
           void *matchinfo, unsigned int matchsize, unsigned int hook_mask)
{
        /* TODO: spinlocks? sanity checks? */
	struct ipt_quota_info *q = (struct ipt_quota_info *) matchinfo;

        if (matchsize != IPT_ALIGN(sizeof (struct ipt_quota_info)))
                return 0;
	
	/* For SMP, we only want to use one set of counters. */
	q->master = q;

        return 1;
}

static struct ipt_match quota_match
    = { {NULL, NULL}, "quota", &match, &checkentry, NULL, THIS_MODULE };

static int __init
init(void)
{
        return ipt_register_match(&quota_match);
}

static void __exit
fini(void)
{
        ipt_unregister_match(&quota_match);
}

module_init(init);
module_exit(fini);

