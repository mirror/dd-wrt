/*
 * netfilter module to limit the number of parallel tcp
 * connections per IP address.
 *   (c) 2000 Gerd Knorr <kraxel@bytesex.org>
 *   Nov 2002: Martin Bene <martin.bene@icomedias.com>:
 *		only ignore TIME_WAIT or gone connections
 *
 * based on ...
 *
 * Kernel module to match connection tracking information.
 * GPL (C) 1999  Rusty Russell (rusty@rustcorp.com.au).
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/ip_conntrack_tcp.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_connlimit.h>

#define DEBUG 0

MODULE_LICENSE("GPL");

/* we'll save the tuples of all connections we care about */
struct ipt_connlimit_conn
{
        struct list_head list;
	struct ip_conntrack_tuple tuple;
};

struct ipt_connlimit_data {
	spinlock_t lock;
	struct list_head iphash[256];
};

static inline unsigned ipt_iphash(const unsigned addr)
{
	return ((addr ^ (addr >> 8) ^ (addr >> 16) ^ (addr >> 24)) & 0xff);
}

static int count_them(struct ipt_connlimit_data *data,
		      u_int32_t addr, u_int32_t mask,
		      struct ip_conntrack *ct)
{
#if DEBUG
	const static char *tcp[] = { "none", "established", "syn_sent", "syn_recv",
				     "fin_wait", "time_wait", "close", "close_wait",
				     "last_ack", "listen" };
#endif
	int addit = 1, matches = 0;
	struct ip_conntrack_tuple tuple;
	struct ip_conntrack_tuple_hash *found;
	struct ipt_connlimit_conn *conn;
	struct list_head *hash,*lh;

	spin_lock(&data->lock);
	tuple = ct->tuplehash[0].tuple;
	hash = &data->iphash[ipt_iphash(addr & mask)];

	/* check the saved connections */
	for (lh = hash->next; lh != hash; lh = lh->next) {
		conn = list_entry(lh,struct ipt_connlimit_conn,list);
		found = ip_conntrack_find_get(&conn->tuple,ct);
		if (0 == memcmp(&conn->tuple,&tuple,sizeof(tuple)) &&
		    found != NULL &&
		    found->ctrack->proto.tcp.state != TCP_CONNTRACK_TIME_WAIT) {
			/* Just to be sure we have it only once in the list.
			   We should'nt see tuples twice unless someone hooks this
			   into a table without "-p tcp --syn" */
			addit = 0;
		}
#if DEBUG
		printk("ipt_connlimit [%d]: src=%u.%u.%u.%u:%d dst=%u.%u.%u.%u:%d %s\n",
		       ipt_iphash(addr & mask),
		       NIPQUAD(conn->tuple.src.ip), ntohs(conn->tuple.src.u.tcp.port),
		       NIPQUAD(conn->tuple.dst.ip), ntohs(conn->tuple.dst.u.tcp.port),
		       (NULL != found) ? tcp[found->ctrack->proto.tcp.state] : "gone");
#endif
		if (NULL == found) {
			/* this one is gone */
			lh = lh->prev;
			list_del(lh->next);
			kfree(conn);
			continue;
		}
		if (found->ctrack->proto.tcp.state == TCP_CONNTRACK_TIME_WAIT) {
			/* we don't care about connections which are
			   closed already -> ditch it */
			lh = lh->prev;
			list_del(lh->next);
			kfree(conn);
			nf_conntrack_put(&found->ctrack->infos[0]);
			continue;
		}
		if ((addr & mask) == (conn->tuple.src.ip & mask)) {
			/* same source IP address -> be counted! */
			matches++;
		}
		nf_conntrack_put(&found->ctrack->infos[0]);
	}
	if (addit) {
		/* save the new connection in our list */
#if DEBUG
		printk("ipt_connlimit [%d]: src=%u.%u.%u.%u:%d dst=%u.%u.%u.%u:%d new\n",
		       ipt_iphash(addr & mask),
		       NIPQUAD(tuple.src.ip), ntohs(tuple.src.u.tcp.port),
		       NIPQUAD(tuple.dst.ip), ntohs(tuple.dst.u.tcp.port));
#endif
		conn = kmalloc(sizeof(*conn),GFP_ATOMIC);
		if (NULL == conn)
			return -1;
		memset(conn,0,sizeof(*conn));
		INIT_LIST_HEAD(&conn->list);
		conn->tuple = tuple;
		list_add(&conn->list,hash);
		matches++;
	}
	spin_unlock(&data->lock);
	return matches;
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
	const struct ipt_connlimit_info *info = matchinfo;
	int connections, match;
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;

	ct = ip_conntrack_get((struct sk_buff *)skb, &ctinfo);
	if (NULL == ct) {
		printk("ipt_connlimit: Oops: invalid ct state ?\n");
		*hotdrop = 1;
		return 0;
	}
	connections = count_them(info->data,skb->nh.iph->saddr,info->v4_mask,ct);
	if (-1 == connections) {
		printk("ipt_connlimit: Hmm, kmalloc failed :-(\n");
		*hotdrop = 1; /* let's free some memory :-) */
		return 0;
	}
        match = (info->inverse) ? (connections <= info->limit) : (connections > info->limit);
#if DEBUG
	printk("ipt_connlimit: src=%u.%u.%u.%u mask=%u.%u.%u.%u "
	       "connections=%d limit=%d match=%s\n",
	       NIPQUAD(skb->nh.iph->saddr), NIPQUAD(info->v4_mask),
	       connections, info->limit, match ? "yes" : "no");
#endif

	return match;
}

static int check(const char *tablename,
		 const struct ipt_ip *ip,
		 void *matchinfo,
		 unsigned int matchsize,
		 unsigned int hook_mask)
{
	struct ipt_connlimit_info *info = matchinfo;
	int i;

	/* verify size */
	if (matchsize != IPT_ALIGN(sizeof(struct ipt_connlimit_info)))
		return 0;

	/* refuse anything but tcp */
	if (ip->proto != IPPROTO_TCP)
		return 0;

	/* init private data */
	info->data = kmalloc(sizeof(struct ipt_connlimit_data),GFP_KERNEL);
	spin_lock_init(&(info->data->lock));
	for (i = 0; i < 256; i++)
		INIT_LIST_HEAD(&(info->data->iphash[i]));
	
	return 1;
}

static void destroy(void *matchinfo, unsigned int matchinfosize)
{
	struct ipt_connlimit_info *info = matchinfo;
	struct ipt_connlimit_conn *conn;
	struct list_head *hash;
	int i;

	/* cleanup */
	for (i = 0; i < 256; i++) {
		hash = &(info->data->iphash[i]);
		while (hash != hash->next) {
			conn = list_entry(hash->next,struct ipt_connlimit_conn,list);
			list_del(hash->next);
			kfree(conn);
		}
	}
	kfree(info->data);
}

static struct ipt_match connlimit_match
= { { NULL, NULL }, "connlimit", &match, &check, &destroy, THIS_MODULE };

static int __init init(void)
{
	return ipt_register_match(&connlimit_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&connlimit_match);
}

module_init(init);
module_exit(fini);
