#include <linux/module.h>
#include <linux/version.h>
#include <linux/config.h>
#include <linux/socket.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/netdevice.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_NETLINK.h>
#include <net/sock.h>

MODULE_AUTHOR("Gianni Tedesco <gianni@ecsc.co.uk>");
MODULE_DESCRIPTION("Provides iptables NETLINK target similar to ipchains -o");
MODULE_LICENSE("GPL");

#if 0
#define DEBUGP	printk
#else
#define DEBUGP(format, args...)
#endif

static struct sock *ipfwsk;

static unsigned int ipt_netlink_target(struct sk_buff **pskb,
				    unsigned int hooknum,
				    const struct net_device *in,
				    const struct net_device *out,
				    const void *targinfo, void *userinfo)
{
	struct ipt_nldata *nld = (struct ipt_nldata *)targinfo;
	struct iphdr *ip = (*pskb)->nh.iph;
	struct sk_buff *outskb;
	struct netlink_t nlhdr;
	size_t len=0;

	/* Allocate a socket buffer */
	if ( MASK(nld->flags, USE_SIZE) )
		len = nld->size+sizeof(nlhdr);
	else
		len = ntohs(ip->tot_len)+sizeof(nlhdr);	

	outskb=alloc_skb(len, GFP_ATOMIC);

	if (outskb) {
		nlhdr.len=len;
		
		if ( MASK(nld->flags, USE_MARK) )
			nlhdr.mark=(*pskb)->nfmark=nld->mark;
		else
			nlhdr.mark=(*pskb)->nfmark;
		
		if ( in && in->name ) {
			strncpy((char *)&nlhdr.iface, in->name, IFNAMSIZ);
		}else if ( out && out->name ){
			strncpy((char *)&nlhdr.iface, out->name, IFNAMSIZ);
		}

		skb_put(outskb, len);
		memcpy(outskb->data, &nlhdr, sizeof(nlhdr));
		memcpy((outskb->data)+sizeof(nlhdr), ip, len-sizeof(nlhdr));
		netlink_broadcast(ipfwsk, outskb, 0, ~0, GFP_ATOMIC);
	}else{
		if (net_ratelimit())
			printk(KERN_WARNING "ipt_NETLINK: packet drop due to netlink failure\n");
	}

	if ( MASK(nld->flags, USE_DROP) )
		return NF_DROP;

	return IPT_CONTINUE;
}

static int ipt_netlink_checkentry(const char *tablename,
			       const struct ipt_entry *e,
			       void *targinfo,
			       unsigned int targinfosize,
			       unsigned int hookmask)
{
	//struct ipt_nldata *nld = (struct ipt_nldata *)targinfo;

	return 1;
}

static struct ipt_target ipt_netlink_reg = { 
	{NULL, NULL},
	"NETLINK",
	ipt_netlink_target,
	ipt_netlink_checkentry,
	NULL,
	THIS_MODULE
};

static int __init init(void)
{
	DEBUGP("ipt_NETLINK: init module\n");	

	if (ipt_register_target(&ipt_netlink_reg) != 0) {
		return -EINVAL;
	}

	if ( !(ipfwsk=netlink_kernel_create(NETLINK_FIREWALL, NULL)) ){
		return -EINVAL;
	}

	return 0;
}

static void __exit fini(void)
{
	DEBUGP("ipt_NETLINK: cleanup_module\n");
	ipt_unregister_target(&ipt_netlink_reg);
	if(ipfwsk->socket) sock_release(ipfwsk->socket);
}

module_init(init);
module_exit(fini);
