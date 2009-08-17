/*
 * ip_conntrack_proto_gre.c - Version 1.2 
 *
 * Connection tracking protocol helper module for GRE.
 *
 * GRE is a generic encapsulation protocol, which is generally not very
 * suited for NAT, as it has no protocol-specific part as port numbers.
 *
 * It has an optional key field, which may help us distinguishing two 
 * connections between the same two hosts.
 *
 * GRE is defined in RFC 1701 and RFC 1702, as well as RFC 2784 
 *
 * PPTP is built on top of a modified version of GRE, and has a mandatory
 * field called "CallID", which serves us for the same purpose as the key
 * field in plain GRE.
 *
 * Documentation about PPTP can be found in RFC 2637
 *
 * (C) 2000-2003 by Harald Welte <laforge@gnumonks.org>
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/list.h>

#include <linux/netfilter_ipv4/lockhelp.h>

DECLARE_RWLOCK(ip_ct_gre_lock);
#define ASSERT_READ_LOCK(x) MUST_BE_READ_LOCKED(&ip_ct_gre_lock)
#define ASSERT_WRITE_LOCK(x) MUST_BE_WRITE_LOCKED(&ip_ct_gre_lock)

#include <linux/netfilter_ipv4/listhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>

#include <linux/netfilter_ipv4/ip_conntrack_proto_gre.h>
#include <linux/netfilter_ipv4/ip_conntrack_pptp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harald Welte <laforge@gnumonks.org>");
MODULE_DESCRIPTION("netfilter connection tracking protocol helper for GRE");

/* shamelessly stolen from ip_conntrack_proto_udp.c */
#define GRE_TIMEOUT		(30*HZ)
#define GRE_STREAM_TIMEOUT	(180*HZ)

#if 0
#define DEBUGP(format, args...) printk(KERN_DEBUG __FILE__ ":" __FUNCTION__ \
		                       ": " format, ## args)
#define DUMP_TUPLE_GRE(x) printk("%u.%u.%u.%u:0x%x -> %u.%u.%u.%u:0x%x:%u:0x%x\n", \
			NIPQUAD((x)->src.ip), ntohl((x)->src.u.gre.key), \
			NIPQUAD((x)->dst.ip), ntohl((x)->dst.u.gre.key))
#else
#define DEBUGP(x, args...)
#define DUMP_TUPLE_GRE(x)
#endif
				
/* GRE KEYMAP HANDLING FUNCTIONS */
static LIST_HEAD(gre_keymap_list);

static inline int gre_key_cmpfn(const struct ip_ct_gre_keymap *km,
				const struct ip_conntrack_tuple *t)
{
	return ((km->tuple.src.ip == t->src.ip) &&
		(km->tuple.dst.ip == t->dst.ip) &&
	        (km->tuple.dst.protonum == t->dst.protonum) &&
	        (km->tuple.dst.u.all == t->dst.u.all));
}

/* look up the source key for a given tuple */
static u_int32_t gre_keymap_lookup(struct ip_conntrack_tuple *t)
{
	struct ip_ct_gre_keymap *km;
	u_int32_t key;

	READ_LOCK(&ip_ct_gre_lock);
	km = LIST_FIND(&gre_keymap_list, gre_key_cmpfn,
			struct ip_ct_gre_keymap *, t);
	if (!km) {
		READ_UNLOCK(&ip_ct_gre_lock);
		return 0;
	}

	key = km->tuple.src.u.gre.key;
	READ_UNLOCK(&ip_ct_gre_lock);

	return key;
}

/* add a single keymap entry, associate with specified expect */
int ip_ct_gre_keymap_add(struct ip_conntrack_expect *exp,
			 struct ip_conntrack_tuple *t, int reply)
{
	struct ip_ct_gre_keymap *km;

	km = kmalloc(sizeof(*km), GFP_ATOMIC);
	if (!km)
		return -1;

	/* initializing list head should be sufficient */
	memset(km, 0, sizeof(*km));

	memcpy(&km->tuple, t, sizeof(*t));

	if (!reply)
		exp->proto.gre.keymap_orig = km;
	else
		exp->proto.gre.keymap_reply = km;

	DEBUGP("adding new entry %p: ", km);
	DUMP_TUPLE_GRE(&km->tuple);

	WRITE_LOCK(&ip_ct_gre_lock);
	list_append(&gre_keymap_list, km);
	WRITE_UNLOCK(&ip_ct_gre_lock);

	return 0;
}

/* change the tuple of a keymap entry (used by nat helper) */
void ip_ct_gre_keymap_change(struct ip_ct_gre_keymap *km,
			     struct ip_conntrack_tuple *t)
{
	DEBUGP("changing entry %p to: ", km);
	DUMP_TUPLE_GRE(t);

	WRITE_LOCK(&ip_ct_gre_lock);
	memcpy(&km->tuple, t, sizeof(km->tuple));
	WRITE_UNLOCK(&ip_ct_gre_lock);
}

/* destroy the keymap entries associated with specified expect */
void ip_ct_gre_keymap_destroy(struct ip_conntrack_expect *exp)
{
	DEBUGP("entering for exp %p\n", exp);
	WRITE_LOCK(&ip_ct_gre_lock);
	if (exp->proto.gre.keymap_orig) {
		DEBUGP("removing %p from list\n", exp->proto.gre.keymap_orig);
		list_del(&exp->proto.gre.keymap_orig->list);
		kfree(exp->proto.gre.keymap_orig);
		exp->proto.gre.keymap_orig = NULL;
	}
	if (exp->proto.gre.keymap_reply) {
		DEBUGP("removing %p from list\n", exp->proto.gre.keymap_reply);
		list_del(&exp->proto.gre.keymap_reply->list);
		kfree(exp->proto.gre.keymap_reply);
		exp->proto.gre.keymap_reply = NULL;
	}
	WRITE_UNLOCK(&ip_ct_gre_lock);
}


/* PUBLIC CONNTRACK PROTO HELPER FUNCTIONS */

/* invert gre part of tuple */
static int gre_invert_tuple(struct ip_conntrack_tuple *tuple,
			    const struct ip_conntrack_tuple *orig)
{
	tuple->dst.u.gre.key = orig->src.u.gre.key;
	tuple->src.u.gre.key = orig->dst.u.gre.key;

	return 1;
}

/* gre hdr info to tuple */
static int gre_pkt_to_tuple(const void *datah, size_t datalen,
			    struct ip_conntrack_tuple *tuple)
{
	struct gre_hdr *grehdr = (struct gre_hdr *) datah;
	struct gre_hdr_pptp *pgrehdr = (struct gre_hdr_pptp *) datah;
	u_int32_t srckey;

	/* core guarantees 8 protocol bytes, no need for size check */

	switch (grehdr->version) {
		case GRE_VERSION_1701:
			if (!grehdr->key) {
				DEBUGP("Can't track GRE without key\n");
				return 0;
			}
			tuple->dst.u.gre.key = *(gre_key(grehdr));
			break;

		case GRE_VERSION_PPTP:
			if (ntohs(grehdr->protocol) != GRE_PROTOCOL_PPTP) {
				DEBUGP("GRE_VERSION_PPTP but unknown proto\n");
				return 0;
			}
			tuple->dst.u.gre.key = htonl(ntohs(pgrehdr->call_id));
			break;

		default:
			printk(KERN_WARNING "unknown GRE version %hu\n",
				grehdr->version);
			return 0;
	}

	srckey = gre_keymap_lookup(tuple);

#if 0
	DEBUGP("found src key %x for tuple ", ntohl(srckey));
	DUMP_TUPLE_GRE(tuple);
#endif
	tuple->src.u.gre.key = srckey;

	return 1;
}

/* print gre part of tuple */
static unsigned int gre_print_tuple(char *buffer,
				    const struct ip_conntrack_tuple *tuple)
{
	return sprintf(buffer, "srckey=0x%x dstkey=0x%x ", 
			ntohl(tuple->src.u.gre.key),
			ntohl(tuple->dst.u.gre.key));
}

/* print private data for conntrack */
static unsigned int gre_print_conntrack(char *buffer,
					const struct ip_conntrack *ct)
{
	return sprintf(buffer, "timeout=%u, stream_timeout=%u ",
		       (ct->proto.gre.timeout / HZ),
		       (ct->proto.gre.stream_timeout / HZ));
}

/* Returns verdict for packet, and may modify conntrack */
static int gre_packet(struct ip_conntrack *ct,
		      struct iphdr *iph, size_t len,
		      enum ip_conntrack_info ctinfo)
{
	/* If we've seen traffic both ways, this is a GRE connection.
	 * Extend timeout. */
	if (ct->status & IPS_SEEN_REPLY) {
		ip_ct_refresh_acct(ct, ctinfo, iph, ct->proto.gre.stream_timeout);
		/* Also, more likely to be important, and not a probe. */
		set_bit(IPS_ASSURED_BIT, &ct->status);
	} else
		ip_ct_refresh_acct(ct, ctinfo, iph, ct->proto.gre.timeout);
	
	return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static int gre_new(struct ip_conntrack *ct,
		   struct iphdr *iph, size_t len)
{ 
	DEBUGP(": ");
	DUMP_TUPLE_GRE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);

	/* initialize to sane value.  Ideally a conntrack helper
	 * (e.g. in case of pptp) is increasing them */
	ct->proto.gre.stream_timeout = GRE_STREAM_TIMEOUT;
	ct->proto.gre.timeout = GRE_TIMEOUT;

	return 1;
}

/* Called when a conntrack entry has already been removed from the hashes
 * and is about to be deleted from memory */
static void gre_destroy(struct ip_conntrack *ct)
{
	struct ip_conntrack_expect *master = ct->master;

	DEBUGP(" entering\n");

	if (!master) {
		DEBUGP("no master exp for ct %p\n", ct);
		return;
	}

	ip_ct_gre_keymap_destroy(master);
}

/* protocol helper struct */
static struct ip_conntrack_protocol gre = { { NULL, NULL }, IPPROTO_GRE,
					    "gre", 
					    gre_pkt_to_tuple,
					    gre_invert_tuple,
					    gre_print_tuple,
					    gre_print_conntrack,
					    gre_packet,
					    gre_new,
					    gre_destroy,
					    NULL,
					    THIS_MODULE };

/* ip_conntrack_proto_gre initialization */
static int __init init(void)
{
	int retcode;

	if ((retcode = ip_conntrack_protocol_register(&gre))) {
                printk(KERN_ERR "Unable to register conntrack protocol "
			        "helper for gre: %d\n",	retcode);
		return -EIO;
	}

	return 0;
}

static void __exit fini(void)
{
	struct list_head *pos, *n;

	/* delete all keymap entries */
	WRITE_LOCK(&ip_ct_gre_lock);
	list_for_each_safe(pos, n, &gre_keymap_list) {
		DEBUGP("deleting keymap %p at module unload time\n", pos);
		list_del(pos);
		kfree(pos);
	}
	WRITE_UNLOCK(&ip_ct_gre_lock);

	ip_conntrack_protocol_unregister(&gre); 
}

EXPORT_SYMBOL(ip_ct_gre_keymap_add);
EXPORT_SYMBOL(ip_ct_gre_keymap_change);
EXPORT_SYMBOL(ip_ct_gre_keymap_destroy);

module_init(init);
module_exit(fini);
