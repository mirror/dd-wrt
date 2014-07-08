#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/jhash.h>
#include <linux/init.h>
#include <linux/ddtb.h>
#include <linux/rculist.h>
#include <linux/seq_file.h>

#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,N,...) N

#define macro_dispatcher(func, ...) \
	macro_dispatcher_(func, VA_NUM_ARGS(__VA_ARGS__))
#define macro_dispatcher_(func, nargs) \
	macro_dispatcher__(func, nargs)
#define macro_dispatcher__(func, nargs) \
	func ## nargs


#undef hlist_entry_safe
#define hlist_entry_safe(ptr, type, member) \
	(ptr) ? hlist_entry(ptr, type, member) : NULL

#define hlist_for_each_entry4(tpos, pos, head, member)			\
	for (pos = (head)->first;					\
	     pos &&							\
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;});\
	     pos = pos->next)

#define hlist_for_each_entry_safe5(tpos, pos, n, head, member)		\
	for (pos = (head)->first;					\
	     pos && ({ n = pos->next; 1; }) &&				\
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;});\
	     pos = n)

#define hlist_for_each_entry3(pos, head, member)				\
	for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member);	\
	     pos;								\
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define hlist_for_each_entry_safe4(pos, n, head, member) 			\
	for (pos = hlist_entry_safe((head)->first, typeof(*pos), member);	\
	     pos && ({ n = pos->member.next; 1; });				\
	     pos = hlist_entry_safe(n, typeof(*pos), member))

#undef hlist_for_each_entry
#define hlist_for_each_entry(...) \
	macro_dispatcher(hlist_for_each_entry, __VA_ARGS__)(__VA_ARGS__)
#undef hlist_for_each_entry_safe
#define hlist_for_each_entry_safe(...) \
	macro_dispatcher(hlist_for_each_entry_safe, __VA_ARGS__)(__VA_ARGS__)


#define hlist_for_each_entry_rcu4(tpos, pos, head, member)		\
	for (pos = rcu_dereference_raw(hlist_first_rcu(head));		\
	     pos &&							\
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1; });\
	     pos = rcu_dereference_raw(hlist_next_rcu(pos)))

#define hlist_for_each_entry_rcu3(pos, head, member)				\
	for (pos = hlist_entry_safe (rcu_dereference_raw(hlist_first_rcu(head)),\
			typeof(*(pos)), member);				\
		pos;								\
		pos = hlist_entry_safe(rcu_dereference_raw(hlist_next_rcu(	\
			&(pos)->member)), typeof(*(pos)), member))

#undef hlist_for_each_entry_rcu
#define hlist_for_each_entry_rcu(...) \
	macro_dispatcher(hlist_for_each_entry_rcu, __VA_ARGS__)(__VA_ARGS__)


#define BUCKET_SIZE	128
static struct hlist_head rs[BUCKET_SIZE];
static unsigned int ddtb_dbg;
static unsigned int ddtb_stats;
static struct spinlock lock;

#ifdef CONFIG_DEBUG_FS

static struct dentry *debugfs;

static int
ddtb_cache_show_ip(struct seq_file *s, char *prefix, u32 _ip, int port)
{
	u32 ip = be32_to_cpu(_ip);

	return seq_printf(s, "%s%d.%d.%d.%d:%d", prefix, ip >> 24, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff, port);
}

static int ddtb_seq_show(struct seq_file *s, void *v)
{
	struct hlist_node *node = v;
	struct ddtb_dir *d = hlist_entry_safe(node, struct ddtb_dir, h);
	struct ddtb_conn *c;

	if (d->flags & DDTB_F_REPLY)
		return 0;

	c = container_of(d, struct ddtb_conn, request);
	seq_printf(s, "tx_pkts:%d tx_bytes:%d rx_pkts:%d rx_bytes:%d\n",
		   c->rx_pkts, c->rx_bytes, c->tx_pkts, c->tx_bytes);
	seq_printf(s, "-> dev:%s", c->request.tuple.dev->name);
	if (c->request.tuple.vid >= 0)
		seq_printf(s, ".%d", c->request.tuple.vid);

	seq_printf(s, " flags:%x mac: %pM", c->request.flags, c->request.tuple.mac);
	ddtb_cache_show_ip(s, " src:", c->request.tuple.sip[0], be16_to_cpu(c->request.tuple.sp));
	ddtb_cache_show_ip(s, " dst:", c->request.tuple.dip[0], be16_to_cpu(c->request.tuple.dp));

	seq_printf(s, "\n<- dev:%s", c->reply.tuple.dev->name);
	if (c->reply.tuple.vid >= 0)
		seq_printf(s, ".%d", c->reply.tuple.vid);

	seq_printf(s, " flags:%x mac: %pM", c->reply.flags, c->reply.tuple.mac);
	ddtb_cache_show_ip(s, " src:", c->reply.tuple.sip[0], be16_to_cpu(c->reply.tuple.sp));
	ddtb_cache_show_ip(s, " dst:", c->reply.tuple.dip[0], be16_to_cpu(c->reply.tuple.dp));

	seq_printf(s, "\n");

	return 0;
}

static struct hlist_node *
ddtb_get_next(struct seq_file *seq, struct hlist_node *node)
{
	int *bucket = seq->private;

	if (node)
		node = rcu_dereference(hlist_next_rcu(node));

	while (!node) {
		if (++(*bucket) >= BUCKET_SIZE)
			return NULL;
		node = rcu_dereference(hlist_first_rcu(&rs[*bucket]));
	}

	return node;
}

static void *ddtb_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	return ddtb_get_next(s, v);
}

static void *ddtb_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(RCU)
{
	struct hlist_node *node;
	int offset = *pos;

	rcu_read_lock();

	node = rcu_dereference(hlist_first_rcu(&rs[0]));
	if (!node)
		node = ddtb_get_next(seq, node);

	while (node && offset-- > 0)
		node = ddtb_get_next(seq, node);

	return node;
}

static void ddtb_seq_stop(struct seq_file *seq, void *v)
	__releases(RCU)
{
	rcu_read_unlock();
}

static const struct seq_operations ddtb_seq_ops = {
	.start = ddtb_seq_start,
	.next = ddtb_seq_next,
	.stop = ddtb_seq_stop,
	.show = ddtb_seq_show,
};

static int ddtb_cache_open(struct inode *inode, struct file *file)
{
	return seq_open_private(file, &ddtb_seq_ops, sizeof(int));
}

static const struct file_operations ddtb_cache_ops = {
	.owner	= THIS_MODULE,
	.open	= ddtb_cache_open,
	.read	= seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int
ddtb_debugfs_init(void)
{
	debugfs = debugfs_create_dir("ddtb", NULL);
	if (!debugfs)
		return -ENOMEM;

	debugfs_create_file("cache", S_IFREG | S_IRUGO,
		debugfs, NULL, &ddtb_cache_ops);

	return 0;
}

#else

static inline int ddtb_debugfs_init(void)
{
	return 0;
}

#endif

static inline void
ddtb_dump_tuple(struct ddtb_tuple *t)
{
	u32 sip = be32_to_cpu(t->sip[0]);
	u32 dip = be32_to_cpu(t->dip[0]);

	pr_info("dev:%p %s vid:%d proto:%d\n", t->dev, t->dev->name, t->vid, t->proto);
	pr_info("mac:%pM\n", t->mac);
	pr_info("src:%d.%d.%d.%d:%d\n", sip >> 24, (sip >> 16) & 0xff, (sip >> 8) & 0xff, sip & 0xff, be16_to_cpu(t->sp));
	pr_info("src:%d.%d.%d.%d:%d\n", dip >> 24, (dip >> 16) & 0xff, (dip >> 8) & 0xff, dip & 0xff, be16_to_cpu(t->dp));
}

static inline int
ddtb_ip_bucket(struct ddtb_tuple *tuple)
{
	u32 h = jhash2((u32 *) tuple, sizeof(struct ddtb_tuple) / 4, 0xdeadbeef);
	u32 b = h & (BUCKET_SIZE - 1);

	return b;
}

static struct ddtb_dir*
ddtb_ip_lookup(struct ddtb_tuple *tuple, int bucket)
{
	struct ddtb_dir *d;

	hlist_for_each_entry_rcu(d, &rs[bucket], h)
		if (!memcmp(tuple, &d->tuple, sizeof(struct ddtb_tuple)))
			return d;

	return NULL;
}

struct ddtb_conn*
ddtb_ip_add(struct ddtb_conn *_c, struct net_device *orig, int v6)
{
	u32 b = ddtb_ip_bucket(&_c->request.tuple);
	struct ddtb_conn *c;
	struct ddtb_dir *d;

	rcu_read_lock();
	spin_lock_bh(&lock);
	d = ddtb_ip_lookup(&_c->request.tuple, b);
	if (d) {
		d->tuple.dev = orig;
		if (d->flags & DDTB_F_REPLY)
			c = container_of(d, struct ddtb_conn, reply);
		else
			c = container_of(d, struct ddtb_conn, request);

		hlist_del_rcu(&c->request.h);
		b = ddtb_ip_bucket(&c->request.tuple);
		hlist_add_head(&c->request.h, &rs[b]);

		hlist_del_rcu(&c->reply.h);
		b = ddtb_ip_bucket(&c->reply.tuple);
		hlist_add_head(&c->reply.h, &rs[b]);

		if (ddtb_dbg) {
			pr_ddtb("rehashing\n");
			pr_ddtb("->\n");
			ddtb_dump_tuple(&c->request.tuple);
			pr_ddtb("<-\n");
			ddtb_dump_tuple(&c->reply.tuple);
		}

		spin_unlock_bh(&lock);
		rcu_read_unlock();

		return c;
	}

	_c->request.tuple.dev = orig;
	b = ddtb_ip_bucket(&_c->request.tuple);
	d = ddtb_ip_lookup(&_c->request.tuple, b);
	spin_unlock_bh(&lock);
	rcu_read_unlock();
	if (d) {
		pr_ddtb("duplicate conn entry\n");
		return NULL;
	}

	c = kmalloc(sizeof(*c), GFP_KERNEL);
	if (!c)
		return NULL;

	memcpy(c, _c, sizeof(*c));
	c->request.flags |= DDTB_F_REQUEST;
	spin_lock_bh(&lock);
	hlist_add_head(&c->request.h, &rs[b]);

	c->reply.flags |= DDTB_F_REPLY;
	b = ddtb_ip_bucket(&c->reply.tuple);
	hlist_add_head(&c->reply.h, &rs[b]);
	spin_unlock_bh(&lock);

	if (ddtb_dbg) {
		pr_ddtb("->\n");
		ddtb_dump_tuple(&c->request.tuple);
		pr_ddtb("<-\n");
		ddtb_dump_tuple(&c->reply.tuple);
	}

	return c;
}
EXPORT_SYMBOL(ddtb_ip_add);

int
ddtb_ip_delete(struct ddtb_conn *_c, int v6)
{
	u32 b = ddtb_ip_bucket(&_c->request.tuple);
	struct ddtb_conn *c;
	struct ddtb_dir *d;

	spin_lock_bh(&lock);
	d = ddtb_ip_lookup(&_c->request.tuple, b);

	if (!d) {
		spin_unlock_bh(&lock);
		return -1;
	}

	c = container_of(d, struct ddtb_conn, request);

	if (ddtb_dbg) {
		ddtb_dump_tuple(&c->request.tuple);
		ddtb_dump_tuple(&c->reply.tuple);
	}

	hlist_del_rcu(&c->request.h);
	hlist_del_rcu(&c->reply.h);
	kfree_rcu(c, rcu);
	spin_unlock_bh(&lock);

	return 0;
}
EXPORT_SYMBOL(ddtb_ip_delete);

static inline void
ddtb_ip_setup_skb(struct ddtb_tuple *t, struct sk_buff *skb,
		  unsigned char *iph, unsigned char *th)
{
	void *data = skb->data;
	struct ethhdr *eh;

	skb->dev = t->dev;

	if (skb->protocol == htons(ETH_P_8021Q)) {
		struct vlan_hdr *vh = data;

		if (t->vid < 0) {
			skb->protocol = vh->h_vlan_encapsulated_proto;
			__skb_pull(skb, VLAN_HLEN);
		} else {
			vh->h_vlan_TCI = htons(t->vid);
		}
	} else if (t->vid >= 0) {
		struct vlan_hdr *vh = (void *) __skb_push(skb, VLAN_HLEN);

		vh->h_vlan_encapsulated_proto = skb->protocol;
		vh->h_vlan_TCI = htons(t->vid);
		skb->protocol = htons(ETH_P_8021Q);
	}

	if (skb->dev->priv_flags & (IFF_FAST_PATH | IFF_EBRIDGE)) {
		eh = (struct ethhdr *) __skb_push(skb, sizeof(struct ethhdr));
		eh->h_proto = skb->protocol;
		memcpy(eh->h_source, skb->dev->dev_addr, ETH_ALEN);
		memcpy(eh->h_dest, t->mac, ETH_ALEN);
	}

	skb_set_network_header(skb, iph - skb->data);
	skb_set_transport_header(skb, th - skb->data);
	skb_reset_mac_header(skb);
}

static void ddtb_nat_packet(struct ddtb_tuple *tuple, struct iphdr *iph, void *trans, bool src)
{
	struct tcphdr *th;
	struct udphdr *uh;
	__sum16 *check;
	__be16 *hdr_port;
	__be32 *hdr_ip;
	__be32 new_ip;
	__be16 new_port;

	if (src) {
		hdr_ip = &iph->daddr;
		new_ip = tuple->sip[0];
		new_port = tuple->sp;

		switch (iph->protocol) {
		case IPPROTO_TCP:
			th = trans;
			hdr_port = &th->dest;
			check = &th->check;
			break;
		case IPPROTO_UDP:
			uh = trans;
			hdr_port = &uh->dest;
			check = &uh->check;
			break;
		default:
			return;
		}
	} else {
		hdr_ip = &iph->saddr;
		new_ip = tuple->dip[0];
		new_port = tuple->dp;

		switch (iph->protocol) {
		case IPPROTO_TCP:
			th = trans;
			hdr_port = &th->source;
			check = &th->check;
			break;
		case IPPROTO_UDP:
			uh = trans;
			hdr_port = &uh->source;
			check = &uh->check;
			break;
		default:
			return;
		}
	}

	csum_replace4(check, *hdr_ip, new_ip);
	csum_replace2(check, *hdr_port, new_port);
	*hdr_port = new_port;

	csum_replace4(&iph->check, *hdr_ip, new_ip);
	*hdr_ip = new_ip;
}

static int
ddtb_packet_xmit(struct ddtb_conn *c, struct sk_buff *skb)
{
#ifdef CONFIG_NF_CONNTRACK_MARK
	if (c->mark)
		skb->mark = c->mark;
#endif

	dev_queue_xmit(skb);
}

static int
ddtb_rewrite_request_v4(struct ddtb_conn *c, struct sk_buff *skb, struct iphdr *iph, void *trans)
{
	if (c->request.flags & DDTB_F_SNAT)
		ddtb_nat_packet(&c->reply.tuple, iph, trans, false);

	if (c->request.flags & DDTB_F_DNAT)
		ddtb_nat_packet(&c->reply.tuple, iph, trans, true);

	ddtb_ip_setup_skb(&c->reply.tuple, skb, (void *) iph, trans);

	if (ddtb_stats) {
		c->tx_pkts++;
		c->tx_bytes += skb->len;
	}

	ddtb_packet_xmit(c, skb);

	return 0;
}

static int
ddtb_rewrite_reply_v4(struct ddtb_conn *c, struct sk_buff *skb, struct iphdr *iph, void *trans)
{
	if (c->request.flags & DDTB_F_SNAT)
		ddtb_nat_packet(&c->request.tuple, iph, trans, true);

	if (c->request.flags & DDTB_F_DNAT)
		ddtb_nat_packet(&c->request.tuple, iph, trans, false);

	ddtb_ip_setup_skb(&c->request.tuple, skb, (void *) iph, trans);

	if (ddtb_stats) {
		c->rx_pkts++;
		c->rx_bytes += skb->len;
	}

	ddtb_packet_xmit(c, skb);

	return 0;
}

static int
ddtb_forward_v4(struct sk_buff *skb, struct ethhdr *eth, struct iphdr *iph, s16 vid)
{
	struct ddtb_tuple tuple;
	struct ddtb_conn *c;
	struct ddtb_dir *d;
	void *trans = iph;
	struct tcphdr *th;
	struct udphdr *uh;
	int ret, delete = 0;
	u32 b;

	memset(&tuple, 0, sizeof(struct ddtb_tuple));

	memcpy(tuple.sip, &iph->saddr, 4);
	memcpy(tuple.dip, &iph->daddr, 4);

	trans += iph->ihl << 2;

	switch (iph->protocol) {
	case IPPROTO_TCP:
		th = trans;
		if (th->fin || th->rst)
			delete = 1;

		tuple.proto = IPPROTO_TCP;
		tuple.sp = th->source;
		tuple.dp = th->dest;
		break;

	case IPPROTO_UDP:
		uh = trans;
		tuple.proto = IPPROTO_UDP;
		tuple.sp = uh->source;
		tuple.dp = uh->dest;
		tuple.dp = uh->dest;
		break;

	default:
		return -1;
	}

	tuple.dev = skb->orig_dev;
	tuple.vid = vid;
	if (eth)
		memcpy(tuple.mac, eth->h_source, 6);

	b = ddtb_ip_bucket(&tuple);

	rcu_read_lock();
	d = ddtb_ip_lookup(&tuple, b);

	if (!d)
		ret = -1;
	else if (delete) {
		if (d->flags & DDTB_F_REPLY)
			c = container_of(d, struct ddtb_conn, reply);
		else
			c = container_of(d, struct ddtb_conn, request);
		ddtb_ip_delete(c, 0);
	} else if (d->flags & DDTB_F_REPLY) {
		c = container_of(d, struct ddtb_conn, reply);
		ret = ddtb_rewrite_reply_v4(c, skb, iph, trans);
	} else {
		c = container_of(d, struct ddtb_conn, request);
		ret = ddtb_rewrite_request_v4(c, skb, iph, trans);
	}

	rcu_read_unlock();

	return ret;
}

int
ddtb_intercept(struct sk_buff *skb)
{
	struct ethhdr *eth = NULL;
	__be16 proto = skb->protocol;
	void *data = skb->data;
	struct iphdr *iph;
	__be16 vlan = -1;
	unsigned int pflags;

	if (!skb->dev)
		return -1;

	pflags = skb->dev->priv_flags;
	if ((pflags & IFF_EBRIDGE) == 0)
		skb->orig_dev = skb->dev;

	if (!skb->orig_dev)
		return -1;

	pflags = skb->orig_dev->priv_flags;
	if (pflags & IFF_FAST_PATH) {
		eth = eth_hdr(skb);
		if (proto == htons(ETH_P_8021Q)) {
			vlan = be16_to_cpu(*((__be16 *) data));
			data += 2;
			proto = *((__be16 *) data);
			data += 2;
		}
	} else if (!(skb->orig_dev->flags & IFF_POINTOPOINT)) {
		return -1;
	}

	switch (be16_to_cpu(proto)) {
	case ETH_P_IP:
		iph = data;
		if (iph->version == 4)
			return ddtb_forward_v4(skb, eth, iph, vlan);
		break;
	}

	return -1;
}

static int __init
ddtb_init(void)
{
	ddtb_debugfs_init();
	spin_lock_init(&lock);

	return 0;
}

core_initcall_sync(ddtb_init);

module_param_named(debug_enable, ddtb_dbg, uint, 0600);
MODULE_PARM_DESC(debug_enable, "Enable debugging");

module_param_named(stats_enable, ddtb_stats, uint, 0600);
MODULE_PARM_DESC(stats_enable, "Enable statistics");
