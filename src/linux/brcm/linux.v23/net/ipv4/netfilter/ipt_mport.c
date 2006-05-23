/* Kernel module to match one of a list of TCP/UDP ports: ports are in
   the same place so we can treat them as equal. */
#include <linux/module.h>
#include <linux/types.h>
#include <linux/udp.h>
#include <linux/skbuff.h>

#include <linux/netfilter_ipv4/ipt_mport.h>
#include <linux/netfilter_ipv4/ip_tables.h>

MODULE_LICENSE("GPL");

#define duprintf(format, args...)

/* Returns 1 if the port is matched by the test, 0 otherwise. */
static inline int
ports_match(const struct ipt_mport *minfo, u_int16_t src, u_int16_t dst)
{
	unsigned int i;
        unsigned int m;
        u_int16_t pflags = minfo->pflags;
	for (i=0, m=1; i<IPT_MULTI_PORTS; i++, m<<=1) {
                u_int16_t s, e;

                if (pflags & m
                    && minfo->ports[i] == 65535)
                        return 0;

                s = minfo->ports[i];

                if (pflags & m) {
                        e = minfo->ports[++i];
                        m <<= 1;
                } else
                        e = s;

                if (minfo->flags & IPT_MPORT_SOURCE
                    && src >= s && src <= e)
                        return 1;

		if (minfo->flags & IPT_MPORT_DESTINATION
		    && dst >= s && dst <= e)
			return 1;
	}

	return 0;
}

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset,
      const void *hdr,
      u_int16_t datalen,
      int *hotdrop)
{
	const struct udphdr *udp = hdr;
	const struct ipt_mport *minfo = matchinfo;

	/* Must be big enough to read ports. */
	if (offset == 0 && datalen < sizeof(struct udphdr)) {
		/* We've been asked to examine this packet, and we
		   can't.  Hence, no choice but to drop. */
			duprintf("ipt_mport:"
				 " Dropping evil offset=0 tinygram.\n");
			*hotdrop = 1;
			return 0;
	}

	/* Must not be a fragment. */
	return !offset
		&& ports_match(minfo, ntohs(udp->source), ntohs(udp->dest));
}

/* Called when user tries to insert an entry of this type. */
static int
checkentry(const char *tablename,
	   const struct ipt_ip *ip,
	   void *matchinfo,
	   unsigned int matchsize,
	   unsigned int hook_mask)
{
	if (matchsize != IPT_ALIGN(sizeof(struct ipt_mport)))
		return 0;

	/* Must specify proto == TCP/UDP, no unknown flags or bad count */
	return (ip->proto == IPPROTO_TCP || ip->proto == IPPROTO_UDP)
		&& !(ip->invflags & IPT_INV_PROTO)
		&& matchsize == IPT_ALIGN(sizeof(struct ipt_mport));
}

static struct ipt_match mport_match
= { { NULL, NULL }, "mport", &match, &checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	return ipt_register_match(&mport_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&mport_match);
}

module_init(init);
module_exit(fini);
