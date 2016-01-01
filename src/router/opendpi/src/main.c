/* 
 * main.c
 * Copyright (C) 2010-2012 G. Elian Gidoni <geg@gnu.org>
 *               2012 Ed Wildgoose <lists@wildgooses.com>
 * 
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netfilter/x_tables.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/if_ether.h>
#include <linux/rbtree.h>
#include <linux/kref.h>
#include <linux/time.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <linux/atomic.h>
#include <linux/proc_fs.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>
#include "opendpi/lib/ndpi_main.c"
#define BT_ANNOUNCE 

#include "ndpi_main.h"
#include "xt_ndpi.h"

#include <config.h>

#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF    89
#endif

static char dir_name[]="xt_ndpi";
static char info_name[]="info";
#ifdef NDPI_DETECTION_SUPPORT_IPV6
static char info6_name[]="info6";
#endif
#ifdef BT_ANNOUNCE
static char ann_name[]="announce";
#endif

static char proto_name[]="proto";


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)


#define PROC_REMOVE(pde,net) proc_remove(pde)
#else

#define PROC_REMOVE(pde,net) proc_net_remove(net,dir_name)

/* backport from 3.10 */
static inline struct inode *file_inode(struct file *f)
{
	return f->f_path.dentry->d_inode;
}
static inline void *PDE_DATA(const struct inode *inode)
{
	return PROC_I(inode)->pde->data;
}
#endif

#if defined(CONFIG_NF_CONNTRACK_CUSTOM) && CONFIG_NF_CONNTRACK_CUSTOM > 0
#define NF_CT_CUSTOM
#include <net/netfilter/nf_conntrack_extend.h> 
#else
#undef NF_CT_CUSTOM
#include <net/netfilter/nf_conntrack_labels.h>
#ifndef CONFIG_NF_CONNTRACK_LABELS
#error NF_CONNTRACK_LABELS not defined
#endif
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("G. Elian Gidoni <geg@gnu.org>, Vitaly E. Lavrov <vel21ripn@gmail.com>");
MODULE_DESCRIPTION("nDPI wrapper");
MODULE_ALIAS("ipt_ndpi");
MODULE_ALIAS("ipt_NDPI");


/* id tracking */
struct osdpi_id_node {
        struct rb_node node;
        struct kref refcnt;
	union nf_inet_addr ip;
	struct ndpi_id_struct ndpi_id;
};

struct ndpi_net {
	struct ndpi_detection_module_struct *ndpi_struct;
	struct rb_root osdpi_id_root;
	NDPI_PROTOCOL_BITMASK protocols_bitmask;
	atomic_t	protocols_cnt[NDPI_LAST_IMPLEMENTED_PROTOCOL+1];
	spinlock_t	id_lock;
	spinlock_t	ipq_lock;
	struct proc_dir_entry   *pde,
#ifdef NDPI_DETECTION_SUPPORT_IPV6
				*pe_info6,
#endif
#ifdef BT_ANNOUNCE
				*pe_ann,
#endif
				*pe_info,
				*pe_proto;
	int		n_hash;
	int		gc_count;
	int		gc_index;
        struct timer_list gc;
	struct ndpi_mark {
		u_int32_t	mark,mask;
	} mark[NDPI_LAST_IMPLEMENTED_PROTOCOL+1];
};

struct nf_ct_ext_ndpi {
	u_int8_t		proto,l4_proto; // 2 bytes
	spinlock_t		lock; // 2 bytes
	struct ndpi_flow_struct	*flow;
	struct ndpi_id_struct   *src,*dst;
};

static char *prot_short_str[] = { NDPI_PROTOCOL_SHORT_STRING };

static unsigned long  ndpi_log_debug=0;
static unsigned long  ndpi_log_trace=0;
static unsigned long  ndpi_mtu=1520;
static unsigned long  bt_hash_size=0;
static unsigned long  bt_hash_tmo=1200;

static unsigned long  ndpi_jumbo=0;
static unsigned long  ndpi_falloc=0;
static unsigned long  ndpi_nskb=0;
static unsigned long  ndpi_lskb=0;
static unsigned long  ndpi_flow_c=0;
static unsigned long  ndpi_bt_gc=0;

static unsigned long  ndpi_p0=0;
static unsigned long  ndpi_p1=0;
static unsigned long  ndpi_p2=0;
static unsigned long  ndpi_p3=0;
static unsigned long  ndpi_p4=0;
static unsigned long  ndpi_p5=0;
static unsigned long  ndpi_p6=0;
static unsigned long  ndpi_p7=0;
static unsigned long  ndpi_p8=0;
static unsigned long  ndpi_p9=0;
static unsigned long  ndpi_pa=0;
static unsigned long  ndpi_pb=0;
static unsigned long  ndpi_pc=0;
static unsigned long  ndpi_pd=0;

static unsigned long  ndpi_pl[11]={0,};

module_param_named(log_debug, ndpi_log_debug, ulong, 0600);
module_param_named(log_trace, ndpi_log_trace, ulong, 0600);
module_param_named(mtu, ndpi_mtu, ulong, 0600);

module_param_named(bt_hash_size, bt_hash_size, ulong, 0400);
module_param_named(bt_hash_timeout, bt_hash_tmo, ulong, 0400);

module_param_named(jumbo, ndpi_jumbo, ulong, 0400);
module_param_named(noalloc, ndpi_falloc, ulong, 0400);
module_param_named(skb_sgo, ndpi_nskb, ulong, 0400);
module_param_named(skb_lin, ndpi_lskb, ulong, 0400);
module_param_named(flow_created, ndpi_flow_c, ulong, 0400);
module_param_named(bt_gc_count, ndpi_bt_gc, ulong, 0400);

module_param_named(ipv4,         ndpi_p0, ulong, 0400);
module_param_named(ipv6,         ndpi_pa, ulong, 0400);
module_param_named(nonip,        ndpi_pb, ulong, 0400);
module_param_named(frag_and_len, ndpi_p1, ulong, 0400);
module_param_named(bad_tcp_udp,  ndpi_p2, ulong, 0400);
module_param_named(ct_confirm,   ndpi_p3, ulong, 0400);
module_param_named(add_ndpi_err, ndpi_p4, ulong, 0400);
module_param_named(unsup_proto,  ndpi_p5, ulong, 0400);
module_param_named(proc_pack,    ndpi_p6, ulong, 0400);
module_param_named(non_tcpudp,   ndpi_p7, ulong, 0400);
module_param_named(known,        ndpi_p8, ulong, 0400);
module_param_named(max_parsed_l, ndpi_p9, ulong, 0400);
module_param_named(id_num,	 ndpi_pc, ulong, 0400);
module_param_named(noncached,	 ndpi_pd, ulong, 0400);



module_param_named(bt_pto, ndpi_pto, ulong, 0400);
module_param_named(bt_ptss, ndpi_ptss, ulong, 0400);
module_param_named(bt_ptsd, ndpi_ptsd, ulong, 0400);
module_param_named(bt_ptds, ndpi_ptds, ulong, 0400);
module_param_named(bt_ptdd, ndpi_ptdd, ulong, 0400);
module_param_named(bt_ptussf, ndpi_ptussf, ulong, 0400);
module_param_named(bt_ptussr, ndpi_ptussr, ulong, 0400);
module_param_named(bt_ptusdf, ndpi_ptusdf, ulong, 0400);
module_param_named(bt_ptusdr, ndpi_ptusdr, ulong, 0400);
module_param_named(bt_ptudsf, ndpi_ptudsf, ulong, 0400);
module_param_named(bt_ptudsr, ndpi_ptudsr, ulong, 0400);
module_param_named(bt_ptuddf, ndpi_ptuddf, ulong, 0400);
module_param_named(bt_ptuddr, ndpi_ptuddr, ulong, 0400);

module_param_named(bt_pusr, ndpi_pusr, ulong, 0400);
module_param_named(bt_pusf, ndpi_pusf, ulong, 0400);
module_param_named(bt_pudr, ndpi_pudr, ulong, 0400);
module_param_named(bt_pudf, ndpi_pudf, ulong, 0400);

module_param_named(zpl010,  ndpi_pl[0], ulong, 0400);
module_param_named(zpl020,  ndpi_pl[1], ulong, 0400);
module_param_named(zpl030,  ndpi_pl[2], ulong, 0400);
module_param_named(zpl040,  ndpi_pl[3], ulong, 0400);
module_param_named(zpl050,  ndpi_pl[4], ulong, 0400);
module_param_named(zpl060,  ndpi_pl[5], ulong, 0400);
module_param_named(zpl070,  ndpi_pl[6], ulong, 0400);
module_param_named(zpl080,  ndpi_pl[7], ulong, 0400);
module_param_named(zpl090,  ndpi_pl[8], ulong, 0400);
module_param_named(zpl100,  ndpi_pl[9], ulong, 0400);
module_param_named(zpl100x, ndpi_pl[10], ulong, 0400);

module_param_named(btptm000,  ndpi_btp_tm[0], ulong, 0400);
module_param_named(btptm001,  ndpi_btp_tm[1], ulong, 0400);
module_param_named(btptm002,  ndpi_btp_tm[2], ulong, 0400);
module_param_named(btptm003,  ndpi_btp_tm[3], ulong, 0400);
module_param_named(btptm004,  ndpi_btp_tm[4], ulong, 0400);
module_param_named(btptm005,  ndpi_btp_tm[5], ulong, 0400);
module_param_named(btptm006,  ndpi_btp_tm[6], ulong, 0400);
module_param_named(btptm007,  ndpi_btp_tm[7], ulong, 0400);
module_param_named(btptm008,  ndpi_btp_tm[8], ulong, 0400);
module_param_named(btptm009,  ndpi_btp_tm[9], ulong, 0400);
module_param_named(btptm010,  ndpi_btp_tm[10], ulong, 0400);
module_param_named(btptm011,  ndpi_btp_tm[11], ulong, 0400);
module_param_named(btptm012,  ndpi_btp_tm[12], ulong, 0400);
module_param_named(btptm013,  ndpi_btp_tm[13], ulong, 0400);
module_param_named(btptm014,  ndpi_btp_tm[14], ulong, 0400);
module_param_named(btptm015,  ndpi_btp_tm[15], ulong, 0400);
module_param_named(btptm016,  ndpi_btp_tm[16], ulong, 0400);
module_param_named(btptm017,  ndpi_btp_tm[17], ulong, 0400);
module_param_named(btptm018,  ndpi_btp_tm[18], ulong, 0400);
module_param_named(btptm019,  ndpi_btp_tm[19], ulong, 0400);

static int ndpi_net_id;
static inline struct ndpi_net *ndpi_pernet(struct net *net)
{
	        return net_generic(net, ndpi_net_id);
}

/* detection */
static u32 detection_tick_resolution = 1000;

static	enum nf_ct_ext_id nf_ct_ext_id_ndpi = 0;
static	struct kmem_cache *osdpi_flow_cache = NULL;
static	struct kmem_cache *osdpi_id_cache = NULL;

static	size_t size_id_struct = 0;
static	size_t size_flow_struct = 0;

/* debug functions */

static void debug_printf(u32 protocol, void *id_struct,
                         ndpi_log_level_t log_level, const char *format, ...)
{
        /* do nothing */

        va_list args;
        switch (log_level)
        {
            case NDPI_LOG_ERROR: 
        	va_start(args, format);
                vprintk(format, args);
        	va_end(args);
                break;
            case NDPI_LOG_TRACE:
		if(!ndpi_log_trace) break;
        	va_start(args, format);
                vprintk(format, args);
        	va_end(args);
                break;

            case NDPI_LOG_DEBUG:
		if(!ndpi_log_debug) break;
        	va_start(args, format);
                vprintk(format, args);
        	va_end(args);
                break;
        }
}

static void *malloc_wrapper(unsigned long size)
{
	return kmalloc(size, GFP_KERNEL);
}

static void free_wrapper(void *freeable)
{
	kfree(freeable);
}

static struct ndpi_id_struct *
ndpi_id_search_or_insert(struct ndpi_net *n, 
		union nf_inet_addr *ip)
{
        int res;
        struct osdpi_id_node *this,*id;
	struct rb_root *root;
  	struct rb_node **new, *parent = NULL;

	spin_lock_bh (&n->id_lock);
	root = &n->osdpi_id_root;
	new  = &(root->rb_node);
  	while (*new) {
                this = rb_entry(*new, struct osdpi_id_node, node);
		res = memcmp(ip, &this->ip,sizeof(union nf_inet_addr));

		parent = *new;
  		if (res < 0)
  			new = &((*new)->rb_left);
  		else if (res > 0)
  			new = &((*new)->rb_right);
  		else {
                	kref_get (&this->refcnt);
			spin_unlock_bh (&n->id_lock);
  			return &this->ndpi_id;
		}
  	}
	id = kmem_cache_zalloc (osdpi_id_cache, GFP_ATOMIC);
	if (id == NULL) {
		spin_unlock_bh (&n->id_lock);
		pr_err("xt_ndpi: couldn't allocate new id.\n");
		return NULL;
	}
	(volatile unsigned long int)ndpi_pc++;
	memcpy(&id->ip, ip, sizeof(union nf_inet_addr));
	kref_init (&id->refcnt);

  	rb_link_node(&id->node, parent, new);
  	rb_insert_color(&id->node, root);
	spin_unlock_bh (&n->id_lock);
	return &id->ndpi_id;
}

static void
ndpi_free_id (struct ndpi_net *n, struct osdpi_id_node * id)
{
	if (atomic_sub_and_test((int) 1, &id->refcnt.refcount)) {
	        rb_erase(&id->node, &n->osdpi_id_root);
	        kmem_cache_free (osdpi_id_cache, id);
		(volatile unsigned long int)ndpi_pc--;
	}
}

static inline struct nf_ct_ext_ndpi *nf_ct_ext_find_ndpi(const struct nf_conn * ct)
{
	return (struct nf_ct_ext_ndpi *)__nf_ct_ext_find(ct,nf_ct_ext_id_ndpi);
}
static inline void *nf_ct_ext_add_ndpi(struct nf_conn * ct)
{
#ifdef NF_CT_CUSTOM
  #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	return __nf_ct_ext_add_length(ct,nf_ct_ext_id_ndpi,
		sizeof(struct nf_ct_ext_ndpi),GFP_ATOMIC);
  #else
	return __nf_ct_ext_add(ct,nf_ct_ext_id_ndpi,GFP_ATOMIC);
  #endif
#else
	return __nf_ct_ext_add_length(ct,nf_ct_ext_id_ndpi,
		sizeof(struct nf_ct_ext_ndpi)-sizeof(struct nf_conn_labels),GFP_ATOMIC);
#endif
}

static inline void free_flow_data(struct nf_ct_ext_ndpi *ct_ndpi) {
    if(ct_ndpi->flow != NULL) {
	ndpi_free_flow(ct_ndpi->flow);
	kmem_cache_free (osdpi_flow_cache, ct_ndpi->flow);
	ct_ndpi->flow = NULL;
    }
}

static int
__ndpi_free_flow (struct nf_conn * ct,void *data) {
	struct ndpi_net *n = data;
	struct nf_ct_ext_ndpi *ct_ndpi = nf_ct_ext_find_ndpi(ct);
	
	if(!ct_ndpi) return 1;

	spin_lock_bh (&n->id_lock);
	if(ct_ndpi->src) {
		ndpi_free_id (n, container_of(ct_ndpi->src,struct osdpi_id_node,ndpi_id ));
		ct_ndpi->src = NULL;
	}
	if(ct_ndpi->dst) {
		ndpi_free_id (n, container_of(ct_ndpi->dst,struct osdpi_id_node,ndpi_id ));
		ct_ndpi->dst = NULL;
	}
	spin_unlock_bh (&n->id_lock);
	free_flow_data(ct_ndpi);
	module_put(THIS_MODULE);
	return 1;
}

static void
nf_ndpi_free_flow (struct nf_conn * ct)
{
	struct ndpi_net *n = ndpi_pernet(nf_ct_net(ct));
	struct nf_ct_ext_ndpi *ct_ndpi = nf_ct_ext_find_ndpi(ct);

	if(!ct_ndpi) return;
	spin_lock_bh(&ct_ndpi->lock);
	__ndpi_free_flow(ct,(void *)n);
	spin_unlock_bh(&ct_ndpi->lock);
}

/* must be locked ct_ndpi->lock */
static struct ndpi_flow_struct * 
ndpi_alloc_flow (struct nf_ct_ext_ndpi *ct_ndpi)
{
        struct ndpi_flow_struct *flow;

        flow = kmem_cache_zalloc (osdpi_flow_cache, GFP_ATOMIC);
        if (flow == NULL) {
                pr_err("xt_ndpi: couldn't allocate new flow.\n");
                return flow;
        }

	ct_ndpi->proto = NDPI_PROTOCOL_UNKNOWN;
	ct_ndpi->flow = flow;
	__module_get(THIS_MODULE);
	ndpi_flow_c++;
        return flow;
}
#ifndef NF_CT_CUSTOM
static void (*ndpi_nf_ct_destroy)(struct nf_conntrack *) __rcu __read_mostly;

static void ndpi_destroy_conntrack(struct nf_conntrack *nfct) {
	struct nf_conn *ct = (struct nf_conn *)nfct;
	void (*destroy)(struct nf_conntrack *);

	nf_ndpi_free_flow(ct);

	rcu_read_lock();
        destroy = rcu_dereference(ndpi_nf_ct_destroy);
        if(destroy) destroy(nfct);
        rcu_read_unlock();
}
#endif

/*****************************************************************/

static void
ndpi_enable_protocols (struct ndpi_net *n, const struct xt_ndpi_mtinfo *info)
{
        int i;

        for (i = 1; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
                if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0) {
                        spin_lock_bh (&n->ipq_lock);
			__module_get(THIS_MODULE);

			//Force http (7) or ssl (91) detection for webserver host requests
                        if ((i > 118 && i < 127) || (i > 139 && i < 146) || (i > 175 && i < 182 ) || i == 70 || i == 133) {
                           NDPI_ADD_PROTOCOL_TO_BITMASK(n->protocols_bitmask, 7);
                           NDPI_ADD_PROTOCOL_TO_BITMASK(n->protocols_bitmask, 91);
                        }

                        if(atomic_inc_return(&n->protocols_cnt[i]) == 1) {
				NDPI_ADD_PROTOCOL_TO_BITMASK(n->protocols_bitmask, i);
				ndpi_set_protocol_detection_bitmask2
					(n->ndpi_struct,&n->protocols_bitmask);
			}
                        spin_unlock_bh (&n->ipq_lock);
                }
        }
}


static void
ndpi_disable_protocols (struct ndpi_net *n, const struct xt_ndpi_mtinfo *info)
{
        int i;

        for (i = 1; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
                if (!info || NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0) {
                        spin_lock_bh (&n->ipq_lock);
			while (atomic_read(&n->protocols_cnt[i])) {
				module_put(THIS_MODULE);
				if (atomic_dec_and_test(&n->protocols_cnt[i])) {
					NDPI_DEL_PROTOCOL_FROM_BITMASK(n->protocols_bitmask, i);
					ndpi_set_protocol_detection_bitmask2
						(n->ndpi_struct, &n->protocols_bitmask);
					break;
			    	} 
				if(info) break;
			}
                        spin_unlock_bh (&n->ipq_lock);
                }
        }
}
static void add_stat(unsigned long int n) {

	if(n > ndpi_p9) ndpi_p9 = n;
	n /= 10;
	if(n < 0) n = 0;
	if(n > sizeof(ndpi_pl)/sizeof(ndpi_pl[0])-1)
		n = sizeof(ndpi_pl)/sizeof(ndpi_pl[0])-1;
	ndpi_pl[n]++;
}

static u32
ndpi_process_packet(struct ndpi_net *n, struct nf_conn * ct, struct nf_ct_ext_ndpi *ct_ndpi,
		    const uint64_t time,
                    const struct sk_buff *skb,int dir)
{
	ndpi_protocol  proto = {NDPI_PROTOCOL_UNKNOWN,NDPI_PROTOCOL_UNKNOWN};
        struct ndpi_id_struct *src, *dst;
        struct ndpi_flow_struct * flow;
	u32 low_ip, up_ip, tmp_ip;
	u16 low_port, up_port, tmp_port, protocol;
	const struct iphdr *iph;

	iph = ip_hdr(skb);
	flow = ct_ndpi->flow;
	if (!flow) {
		flow = ndpi_alloc_flow(ct_ndpi);
		if (!flow) return proto.protocol;
                else flow->detection_completed = 0;
	}

        if (flow->detection_completed && (flow->detected_protocol != NDPI_PROTOCOL_UNKNOWN)) {
                return flow->detected_protocol;
        }

	src = ct_ndpi->src;
	if (!src) {
		src = ndpi_id_search_or_insert (n,
			&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3);
		if (!src) return proto.protocol;
		ct_ndpi->src = src;
	}
	dst = ct_ndpi->dst;
	if (!dst) {
		dst = ndpi_id_search_or_insert (n,
			&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3);
		if (!dst) return proto.protocol;
		ct_ndpi->dst = dst;
	}

	/* here the actual detection is performed */
	if(dir) {
		src = ct_ndpi->dst;
		dst = ct_ndpi->src;
	}
	proto = ndpi_detection_process_packet(n->ndpi_struct,flow,
//#ifdef NDPI_DETECTION_SUPPORT_IPV6
//				ip6h->version == 6 ? (uint8_t *)ipv6_hdr:
//#endif
					 (uint8_t *) iph, 
					 skb->len, time,
					  src, dst, dir);

        flow->detection_completed = 0;
	if(proto.protocol) {
		add_stat(flow->packet.parsed_lines);
		if (proto.protocol <= NDPI_LAST_IMPLEMENTED_PROTOCOL) {
			flow->detection_completed = 1;
		}else proto.protocol = NDPI_PROTOCOL_UNKNOWN;
    		flow->detected_protocol = proto.protocol;
		return proto.protocol;
	}
	if(iph->version != 4) {
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		const struct ipv6hdr *ip6h;

		ip6h = ipv6_hdr(skb);
		low_ip = 0;
		up_ip = 0;
		protocol = ip6h->nexthdr;
#else
		return proto.protocol;
#endif
	} else {
		low_ip=ntohl(iph->saddr);
		up_ip=ntohl(iph->daddr);
		protocol = iph->protocol;
	}
	if(protocol == IPPROTO_TCP || protocol == IPPROTO_UDP) {
		low_port = htons(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.tcp.port);
		up_port  = htons(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.tcp.port);
		if(low_ip > up_ip) { tmp_ip = low_ip; low_ip=up_ip; up_ip = tmp_ip; }
		if(low_port > up_port) { tmp_port = low_port; low_port=up_port; up_port = tmp_port; }
	} else {
		low_port = up_port = 0;
	}
	proto = ndpi_guess_undetected_protocol (
			n->ndpi_struct,protocol,low_ip,low_port,up_ip,up_port);
        flow->detected_protocol = proto.protocol;
	if(proto.protocol && proto.protocol > NDPI_LAST_IMPLEMENTED_PROTOCOL)
		proto.protocol = NDPI_PROTOCOL_UNKNOWN;
	else 
		flow->detection_completed = 1;

	return proto.protocol;
}
static inline int can_handle(const struct sk_buff *skb,u8 *l4_proto)
{
	const struct iphdr *iph;
	u32 l4_len;
	u8 proto;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	const struct ipv6hdr *ip6h;

	ip6h = ipv6_hdr(skb);
	if(ip6h->version == 6) {
		ndpi_pa++;
		*l4_proto = ip6h->nexthdr;
		// FIXME!
		return 1;
	}
#endif
	iph = ip_hdr(skb);
        if(!iph) { /* not IP */
		ndpi_pb++; return 0;
	}
	if(iph->version != 4) {
		ndpi_pb++; return 0;
	}
	*l4_proto = proto = iph->protocol;
	ndpi_p0++;

	if(ntohs(iph->frag_off) & 0x3fff) {
		ndpi_p1++; return 0;
	}
	if(skb->len <= (iph->ihl << 2)) {
		ndpi_p1++; return 0; 
	}

	l4_len = skb->len - (iph->ihl << 2);
        if(proto == IPPROTO_TCP) {
		if(l4_len < sizeof(struct tcphdr)) {
			ndpi_p2++; return 0;
		}
		return 1;
	}
        if(proto == IPPROTO_UDP) {
		if(l4_len < sizeof(struct udphdr)) {
			ndpi_p2++; return 0;
		}
		return 1;
	}
#if defined(NDPI_PROTOCOL_IP_IPSEC) || defined(NDPI_PROTOCOL_IP_GRE) || \
	defined(NDPI_PROTOCOL_IP_ICMP) || defined(NDPI_PROTOCOL_IP_IGMP) || \
	defined(NDPI_PROTOCOL_IP_OSPF) || defined(NDPI_PROTOCOL_IP_IP_IN_IP)
        if(proto == IPPROTO_ESP  || proto == IPPROTO_GRE ||
	  proto == IPPROTO_ICMP  || proto == IPPROTO_IGMP ||
	  proto == IPPROTO_OSPF  || proto == IPPROTO_IPIP) {
		ndpi_p7++;
		return 1;
	}
#endif
	ndpi_p5++;
        return 0;
}

#define NDPI_ID 0x44504900ul

static bool
ndpi_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	u32 proto;
	u64 time;
	const struct xt_ndpi_mtinfo *info = par->matchinfo;

	enum ip_conntrack_info ctinfo;
	struct nf_conn * ct;
	struct timeval tv;
	struct sk_buff *linearized_skb = NULL;
	const struct sk_buff *skb_use = NULL;
	struct nf_ct_ext_ndpi *ct_ndpi = NULL;
	u32 *c_proto;
	u8 l4_proto;

	proto = NDPI_PROTOCOL_UNKNOWN;

	if(!can_handle(skb,&l4_proto)) {
		return false;
	}

	c_proto = (void *)&skb->cb[sizeof(skb->cb)-4];
	if((*c_proto & 0xffffff00ul) == NDPI_ID) {
		proto = *c_proto & 0xff;
		ndpi_p8++;
		goto check_rule;
	}

	if( skb->len > ndpi_mtu && skb_is_nonlinear(skb) ) {
		ndpi_jumbo++;
		return false;
	}

	ct = nf_ct_get (skb, &ctinfo);
	if (ct == NULL) return false;

	if (nf_ct_is_untracked(ct)) {
		return false;               
	}
	ct_ndpi = nf_ct_ext_find_ndpi(ct);
	if(!ct_ndpi) {
		if(nf_ct_is_confirmed(ct)) {
			ndpi_p3++;
			return false;               
		}
		ct_ndpi = nf_ct_ext_add_ndpi(ct);
		if(ct_ndpi) {
			memset((char *)ct_ndpi,0,sizeof(struct nf_ct_ext_ndpi));
			spin_lock_init(&ct_ndpi->lock);
			ct_ndpi->l4_proto = l4_proto;
		} else
			ndpi_p4++;
	}
	if(!ct_ndpi) return false;

	spin_lock_bh (&ct_ndpi->lock);
	/* don't pass icmp for TCP/UDP to ndpi_process_packet()  */
	if(l4_proto == IPPROTO_ICMP && ct_ndpi->l4_proto != IPPROTO_ICMP) {
		proto = NDPI_PROTOCOL_IP_ICMP;
		spin_unlock_bh (&ct_ndpi->lock);
		goto check_rule;
	}
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	if(l4_proto == IPPROTO_ICMPV6 && ct_ndpi->l4_proto != IPPROTO_ICMPV6) {
		proto = NDPI_PROTOCOL_IP_ICMPV6;
		spin_unlock_bh (&ct_ndpi->lock);
		goto check_rule;
	}
#endif
	if(ct_ndpi->proto == NDPI_PROTOCOL_UNKNOWN ||
	    (ct_ndpi->flow)) {
		struct ndpi_net *n;

		if (skb_is_nonlinear(skb)) {
			linearized_skb = skb_copy(skb, GFP_ATOMIC);
			if (linearized_skb == NULL) {
				spin_unlock_bh (&ct_ndpi->lock);
				ndpi_falloc++;
				return false;
			}
			skb_use = linearized_skb;
			ndpi_nskb += 1;
		} else {
			skb_use = skb;
			ndpi_lskb += 1;
		}

		do_gettimeofday(&tv);

		time = ((uint64_t) tv.tv_sec) * detection_tick_resolution +
			tv.tv_usec / (1000000 / detection_tick_resolution);
		ndpi_p6++;
		n = ndpi_pernet(nf_ct_net(ct));
		proto = ndpi_process_packet(n, ct,
				ct_ndpi, time, skb_use,
				CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL);

		*c_proto = NDPI_ID | proto;
		if(proto != NDPI_PROTOCOL_UNKNOWN) {
			ct_ndpi->proto = proto;
			if(ct_ndpi->flow) {
					free_flow_data(ct_ndpi);
			}
		}
		spin_unlock_bh (&ct_ndpi->lock);

		if(linearized_skb != NULL)
			kfree_skb(linearized_skb);
	} else {
		proto = ct_ndpi->proto;
		*c_proto = NDPI_ID | proto;
		spin_unlock_bh (&ct_ndpi->lock);
		ndpi_p8++;
	}
    check_rule:
	if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto) != 0)
		return true ^ (info->invert != 0);

	return (info->invert != 0);
}


static int
ndpi_mt_check(const struct xt_mtchk_param *par)
{
	const struct xt_ndpi_mtinfo *info = par->matchinfo;

	if (NDPI_BITMASK_IS_ZERO(info->flags)) {
		pr_info("None selected protocol.\n");
		return -EINVAL;
	}

        ndpi_enable_protocols (ndpi_pernet(par->net), info);
	return nf_ct_l3proto_try_module_get (par->family);
}

static void 
ndpi_mt_destroy (const struct xt_mtdtor_param *par)
{
	const struct xt_ndpi_mtinfo *info = par->matchinfo;

        ndpi_disable_protocols (ndpi_pernet(par->net), info);
	nf_ct_l3proto_module_put (par->family);
}

#ifdef NF_CT_CUSTOM
static unsigned int seq_print_ndpi(struct seq_file *s,
                                  const struct nf_conn *ct,
                                  int dir)
{

       struct nf_ct_ext_ndpi *ct_ndpi;
       if(dir != IP_CT_DIR_REPLY) return 0;

       ct_ndpi = nf_ct_ext_find_ndpi(ct);
       if (ct_ndpi && ct_ndpi->proto)
               seq_printf(s,"ndpi=%s ",prot_short_str[ct_ndpi->proto]);
       return 0;       
}
#endif

static void ndpi_cleanup(struct net *net)
{
        struct rb_node * next;
        struct osdpi_id_node *id;
        struct ndpi_net *n;

	n = ndpi_pernet(net);
	del_timer(&n->gc);
        ndpi_disable_protocols (n, NULL);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 12, 1)
	nf_ct_iterate_cleanup(net, __ndpi_free_flow, n);
#else
	nf_ct_iterate_cleanup(net, __ndpi_free_flow, n, 0 ,0);
#endif
        /* free all objects before destroying caches */
        
        next = rb_first(&n->osdpi_id_root);
        while (next) {
                id = rb_entry(next, struct osdpi_id_node, node);
                next = rb_next(&id->node);
                rb_erase(&id->node, &n->osdpi_id_root);
                kmem_cache_free (osdpi_id_cache, id);
        }
        ndpi_exit_detection_module(n->ndpi_struct, free_wrapper);
	if(n->pde) {
		if(n->pe_info)
			remove_proc_entry(info_name, n->pde);
		if(n->pe_proto)
			remove_proc_entry(proto_name, n->pde);
#ifdef BT_ANNOUNCE
		if(n->pe_ann)
			remove_proc_entry(ann_name, n->pde);
#endif
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		if(n->pe_info6)
			remove_proc_entry(info6_name, n->pde);
#endif
		PROC_REMOVE(n->pde,net);
	}
}

static unsigned int
ndpi_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
        const struct xt_ndpi_tginfo *info = par->targinfo;

	int ndpi_proto = 0;
	u_int32_t mark=0,mask=0xff;
	u32 *c_proto;

	if(info->proto_id) {
		struct ndpi_net *n = ndpi_pernet(dev_net(skb->dev ? : skb_dst(skb)->dev));
		c_proto = (void *)&skb->cb[sizeof(skb->cb)-4];
		if((*c_proto & 0xffffff00ul) == NDPI_ID) {
			ndpi_proto = *c_proto & 0xff;
		} else {
			enum ip_conntrack_info ctinfo;
			struct nf_conn * ct;
			struct nf_ct_ext_ndpi *ct_ndpi;

			ct = nf_ct_get (skb, &ctinfo);
			if (ct && !nf_ct_is_untracked(ct)) {
			    ct_ndpi = nf_ct_ext_find_ndpi(ct);
			    if(ct_ndpi) {
				spin_lock_bh (&ct_ndpi->lock);
				ndpi_proto = ct_ndpi->proto & 0xff;
				spin_unlock_bh (&ct_ndpi->lock);
			    }
			}
			if(!ndpi_proto)
				return XT_CONTINUE;
		}
		mark = n->mark[ndpi_proto].mark;
		mask = n->mark[ndpi_proto].mask;
	}

	if(info->t_mark) {
	        skb->mark = (skb->mark & info->mask) | info->mark;
		if(info->proto_id)
        		skb->mark = (skb->mark & ~mask) | mark;
	}
	if(info->t_clsf) {
	        skb->priority = (skb->priority & info->mask) | info->mark;
		if(info->proto_id)
        		skb->priority = (skb->priority & ~mask) | mark;
	}
        return info->t_accept ? NF_ACCEPT : XT_CONTINUE;
}

static struct xt_match
ndpi_mt_reg __read_mostly = {
	.name = "ndpi",
	.revision = 0,
#ifdef NDPI_DETECTION_SUPPORT_IPV6
        .family = NFPROTO_UNSPEC,
#else
	.family = NFPROTO_IPV4,
#endif
	.match = ndpi_mt,
	.checkentry = ndpi_mt_check,
	.destroy = ndpi_mt_destroy,
	.matchsize = XT_ALIGN(sizeof(struct xt_ndpi_mtinfo)),
	.me = THIS_MODULE,
};

static struct xt_target ndpi_tg_reg __read_mostly = {
        .name           = "NDPI",
        .revision       = 0,
#ifdef NDPI_DETECTION_SUPPORT_IPV6
        .family         = NFPROTO_UNSPEC,
#else
	.family		= NFPROTO_IPV4,
#endif
        .target         = ndpi_tg,
        .targetsize     = sizeof(struct xt_ndpi_tginfo),
        .me             = THIS_MODULE,
};

static void bt_port_gc(unsigned long data) {
        struct ndpi_net *n = (struct ndpi_net *)data;
        struct ndpi_detection_module_struct *ndpi_struct = n->ndpi_struct;
	struct hash_ip4p_table *ht = ndpi_struct->bt_ht;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	struct hash_ip4p_table *ht6 = ndpi_struct->bt6_ht;
#endif
	struct timespec tm;
	int i;

	if(!ht
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		&& !ht6
#endif
		)  return;

	getnstimeofday(&tm);
	spin_lock(&ht->lock);
	/* full period 64 seconds */
	for(i=0; i < ht->size/128;i++) {
		if(n->gc_index < 0 ) n->gc_index = 0;
		if(n->gc_index >= ht->size-1) n->gc_index = 0;

		if(ht && ht->tbl[n->gc_index].len)
			n->gc_count += ndpi_bittorrent_gc(ht,n->gc_index,tm.tv_sec);
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		if(ht6) {
		   if(ht6->tbl[n->gc_index].len)
			n->gc_count += ndpi_bittorrent_gc(ht6,n->gc_index,tm.tv_sec);
		}
#endif
		n->gc_index++;
	}
	spin_unlock(&ht->lock);
	
	ndpi_bt_gc = n->gc_count;

	n->gc.expires = jiffies + HZ/2;
	add_timer(&n->gc);
}

static int inet_ntop(int ipv6,void *ip, u_int16_t port, char *lbuf, size_t bufsize) {
u_int8_t *ipp = (u_int8_t *)ip;
u_int16_t *ip6p = (u_int16_t *)ip;
return  ipv6 ? 	snprintf(lbuf,bufsize-1, "%x:%x:%x:%x:%x:%x:%x:%x.%d",
			htons(ip6p[0]),htons(ip6p[1]),htons(ip6p[2]),htons(ip6p[3]),
			htons(ip6p[4]),htons(ip6p[5]),htons(ip6p[6]),htons(ip6p[7]),
			htons(port))
		     :	snprintf(lbuf,bufsize-1, "%d.%d.%d.%d:%d",
			ipp[0],ipp[1],ipp[2],ipp[3],htons(port));
}

static ssize_t _ninfo_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos,int ipv6)
{
        struct ndpi_net *n = PDE_DATA(file_inode(file));
        struct ndpi_detection_module_struct *ndpi_struct = n->ndpi_struct;
	struct hash_ip4p_table *ht,*ht4 = ndpi_struct->bt_ht;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	struct hash_ip4p_table *ht6 = ndpi_struct->bt6_ht;
#endif
	char lbuf[128];
	struct hash_ip4p *t;
	size_t p;
	int l;
	ht = 
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		ipv6 ? ht6:
#endif
		ht4;
	if(!ht) {
	    if(!*ppos) {
	        l =  snprintf(lbuf,sizeof(lbuf)-1, "hash disabled\n");
		if (!(access_ok(VERIFY_WRITE, buf, l) &&
				! __copy_to_user(buf, lbuf, l))) return -EFAULT;
		(*ppos)++;
		return l;
	    }
	    return 0;
	}
	if(n->n_hash < 0 || n->n_hash >= ht->size-1) {
	    int tmin,tmax,i;

	    if(!*ppos) {
		tmin = 0x7fffffff;
		tmax = 0;
		t = &ht->tbl[0];

		for(i = ht->size-1; i >= 0 ;i--,t++) {
			if(t->len > 0 && t->len < tmin) tmin = t->len;
			if(t->len > tmax) tmax = t->len;
		}
		if(!atomic_read(&ht->count)) tmin = 0;
	        l =  snprintf(lbuf,sizeof(lbuf)-1,
			"hash_size %lu hash timeout %lus count %u min %d max %d gc %d\n",
				bt_hash_size*1024,bt_hash_tmo,
				atomic_read(&ht->count),tmin,tmax,n->gc_count	);

		if (!(access_ok(VERIFY_WRITE, buf, l) &&
				! __copy_to_user(buf, lbuf, l))) return -EFAULT;
		(*ppos)++;
		return l;
	    }
	    /* ppos > 0 */
#define BSS1 144
#define BSS2 12
	    if(*ppos * BSS1 >= bt_hash_size*1024) return 0;

	    t = &ht->tbl[(*ppos-1)*BSS1];
	    p=0;
	    for(i=0; i < BSS1;i++,t++) {
		if(!(i % BSS2)) {
		        l = snprintf(lbuf,sizeof(lbuf)-1, "%d:\t",(int)(i+(*ppos-1)*BSS1));
			if (!(access_ok(VERIFY_WRITE, buf+p, l) && !__copy_to_user(buf+p, lbuf, l)))
				return -EFAULT;
			p += l;
		}
	        l = snprintf(lbuf,sizeof(lbuf)-1, "%5zu%c",
				t->len, (i % BSS2) == (BSS2-1) ? '\n':' ');
		
		if (!(access_ok(VERIFY_WRITE, buf+p, l) &&
				!__copy_to_user(buf+p, lbuf, l)))
			return -EFAULT;
		p += l;
	    }
	    (*ppos)++;
	    return p;
	}
	t = &ht->tbl[n->n_hash];
	if(!*ppos) {
	        l =  snprintf(lbuf,sizeof(lbuf)-1, "index %d len %zu\n",
				n->n_hash,t->len);
		if (!(access_ok(VERIFY_WRITE, buf, l) &&
				!__copy_to_user(buf, lbuf, l))) return -EFAULT;
		(*ppos)++;
		return l;
	}
	if(*ppos > 1) return 0;
	p = 0;
	spin_lock(&t->lock);
	do {
		struct hash_ip4p_node *x = t->top;
	 	struct timespec tm;

	        getnstimeofday(&tm);
		while(x && p < count - 128) {
		        l =  inet_ntop(ipv6,&x->ip,x->port,lbuf,sizeof(lbuf)-2);
			l += snprintf(&lbuf[l],sizeof(lbuf)-l-1, " %d %x %u\n",
				(int)(tm.tv_sec - x->lchg),x->flag,x->count);

			if (!(access_ok(VERIFY_WRITE, buf+p, l) &&
				!__copy_to_user(buf+p, lbuf, l))) return -EFAULT;
			p += l;
			x = x->next;
		}
	} while(0);
	spin_unlock(&t->lock);
	(*ppos)++;
	return p;
}

static int ninfo_proc_open(struct inode *inode, struct file *file)
{
        return 0;
}
static ssize_t ninfo_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
return _ninfo_proc_read(file,buf,count,ppos,0);
}

static ssize_t ninfo6_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
return _ninfo_proc_read(file,buf,count,ppos,1);
}

static ssize_t
ninfo_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff)
{
        struct ndpi_net *n = PDE_DATA(file_inode(file));
	char buf[32];
	int idx;

        if (length > 0) {
		memset(buf,0,sizeof(buf));
		if (!(access_ok(VERIFY_READ, buffer, length) && 
			!__copy_from_user(&buf[0], buffer, min(length,sizeof(buf)-1))))
			        return -EFAULT;
		if(sscanf(buf,"%d",&idx) != 1) return -EINVAL;
		n->n_hash = idx;
        }
        return length;
}

#ifdef BT_ANNOUNCE
static ssize_t nann_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
        struct ndpi_net *n = PDE_DATA(file_inode(file));
        struct ndpi_detection_module_struct *ndpi_struct = n->ndpi_struct;
	struct bt_announce *b = ndpi_struct->bt_ann;
	int  bt_len = ndpi_struct->bt_ann_len;
	char lbuf[512],ipbuf[64];
	int i,l,p;

	for(i = 0,p = 0; i < bt_len; i++,b++) {
		if(!b->time) break;

		if(i < *ppos ) continue;
		if(!b->ip[0] && !b->ip[1] && b->ip[2] == 0xfffffffful)
			inet_ntop(0,&b->ip[3],b->port,ipbuf,sizeof(ipbuf));
		    else
			inet_ntop(1,&b->ip,b->port,ipbuf,sizeof(ipbuf));
	        l =  snprintf(lbuf,sizeof(lbuf)-1, "%08x%08x%08x%08x%08x %s %u '%.*s'\n",
				htonl(b->hash[0]),htonl(b->hash[1]),
				htonl(b->hash[2]),htonl(b->hash[3]),htonl(b->hash[4]),
				ipbuf,b->time,b->name_len,b->name);

		if(count < l) break;

		if (!(access_ok(VERIFY_WRITE, buf+p, l) &&
				!__copy_to_user(buf+p, lbuf, l))) return -EFAULT;
		p += l;
		count -= l;
		(*ppos)++;
	}
	return p;
}
#endif


static ssize_t nproto_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
        struct ndpi_net *n = PDE_DATA(file_inode(file));
	char lbuf[128];
	int i,l,p;

	for(i = 0,p = 0; i < NDPI_LAST_IMPLEMENTED_PROTOCOL+1; i++) {

		if(i < *ppos ) continue;
		l = i ? 0: snprintf(lbuf,sizeof(lbuf),"#id mark     ~mask     name\n");

		l += snprintf(&lbuf[l],sizeof(lbuf)-l,"%02x  %8x/%08x %s\n",
				i,n->mark[i].mark,n->mark[i].mask,prot_short_str[i]);

		if(count < l) break;

		if (!(access_ok(VERIFY_WRITE, buf+p, l) &&
				!__copy_to_user(buf+p, lbuf, l))) return -EFAULT;
		p += l;
		count -= l;
		(*ppos)++;
	}
	return p;
}

static int parse_ndpi_mark(char *cmd,struct ndpi_net *n) {
	char *v,*m;
	v = cmd;
	if(!*v) return 0;

	while(*v && (*v == ' ' || *v == '\t')) v++;
	if(*v == '#') return 0;
	while(*v && !(*v == ' ' || *v == '\t')) v++;
	if(*v) *v++ = '\0';
	while(*v && (*v == ' ' || *v == '\t')) v++;
	m = v;
	while(*m && *m != '/') m++;
	if(*m) {
		char *x;
		*m++ = '\0';
		x = m;
		while(*x && !(*x == ' ' || *x == '\t')) x++;
		if(*x) *x++ = '\0';
	}
	if(*v) {
		u_int32_t mark,mask;
		int id=-1;
		int i,any,all,ok;

		any = !strcmp(cmd,"any");
		all = !strcmp(cmd,"all");
		if(kstrtoint(cmd,16,&id)) {
			id = -1;
		} else {
			if(id < 0 || id > NDPI_LAST_IMPLEMENTED_PROTOCOL) {
				printk("NDPI: bad id %d\n",id);
				id = -1;
			}
		}
		if(kstrtou32(v,16,&mark)) {
			printk("NDPI: bad mark '%s'\n",v);
			return 1;
		}
		if(*m) {
			if(kstrtou32(m,16,&mask)) {
				printk("NDPI: bad mask '%s'\n",m);
				return 1;
			}
		}
//		printk("NDPI: proto %s id %d mark %x mask %s\n",
//				cmd,id,mark,m);
		if(id != -1) {
			n->mark[id].mark = mark;
			if(*m) 	n->mark[id].mask = mask;
			return 0;
		}
		ok = 0;
		for(i=0; i < NDPI_LAST_IMPLEMENTED_PROTOCOL+1; i++) {
			if(!any && !all && strcmp(cmd,prot_short_str[i])) continue;
			if(any && !i) continue;
			n->mark[i].mark = mark;
			if(*m) 	n->mark[i].mask = mask;
			ok++;
//			printk("Proto %s id %02x mark %08x/%08x\n",
//					cmd,i,n->mark[i].mark,n->mark[i].mask);
		}
		if(!ok) {
			printk("NDPI: unknown proto %s\n", cmd);
			return 1;
		}
		return 0;
	}
	if(!strcmp(cmd,"init")) {
		int i;
		for(i=0; i < NDPI_LAST_IMPLEMENTED_PROTOCOL+1; i++) {
			n->mark[i].mark = i;
			n->mark[i].mask = 0xff;
		}
		return 0;
	}
	printk("bad cmd %s\n",cmd);
	return *v ? 0:1;
}

static ssize_t
nproto_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff)
{
        struct ndpi_net *n = PDE_DATA(file_inode(file));
	char buf[1025],cmd[64];
	int pos,cpos,bpos,l;

	if (length <= 0) return length;
	pos = 0; cpos = 0;
	memset(cmd,0,sizeof(cmd));

	while(pos < length) {
		l = min(length,sizeof(buf)-1);
	
		memset(buf,0,sizeof(buf));
		if (!(access_ok(VERIFY_READ, buffer+pos, l) && 
			!__copy_from_user(&buf[0], buffer+pos, l)))
			        return -EFAULT;
		for(bpos = 0; bpos < l; bpos++) {
			if(buf[bpos] == '\n') {
				if(parse_ndpi_mark(cmd,n)) return -EINVAL;
				cpos = 0;
				memset(cmd,0,sizeof(cmd));
				continue;
			}
			if(cpos < sizeof(cmd)-1)
				cmd[cpos++] = buf[bpos];
		}
		pos += l;
        }
	if(cpos) {
		if(parse_ndpi_mark(cmd,n)) return -EINVAL;
	}
        return length;
}

static const struct file_operations nproto_proc_fops = {
        .open    = ninfo_proc_open,
        .read    = nproto_proc_read,
        .write   = nproto_proc_write,
};

static const struct file_operations ninfo_proc_fops = {
        .open    = ninfo_proc_open,
        .read    = ninfo_proc_read,
        .write   = ninfo_proc_write,
};

#ifdef NDPI_DETECTION_SUPPORT_IPV6
static const struct file_operations ninfo6_proc_fops = {
        .open    = ninfo_proc_open,
        .read    = ninfo6_proc_read,
        .write   = ninfo_proc_write,
};
#endif
#ifdef BT_ANNOUNCE
static const struct file_operations nann_proc_fops = {
        .open    = ninfo_proc_open,
        .read    = nann_proc_read,
};
#endif

static int __net_init ndpi_net_init(struct net *net)
{
        int i;
	struct ndpi_net *n;

	/* init global detection structure */

	n = ndpi_pernet(net);
	spin_lock_init(&n->id_lock);
	spin_lock_init(&n->ipq_lock);

	parse_ndpi_mark("init",n);
       	n->osdpi_id_root = RB_ROOT;

	/* init global detection structure */
	n->ndpi_struct = ndpi_init_detection_module(detection_tick_resolution,
                                                     malloc_wrapper,
						     free_wrapper,
						     debug_printf);
	if (n->ndpi_struct == NULL) {
		pr_err("xt_ndpi: global structure initialization failed.\n");
                return -ENOMEM;
	}
        for (i = 0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
                atomic_set (&n->protocols_cnt[i], 0);
        }

	/* disable all protocols */
	NDPI_BITMASK_RESET(n->protocols_bitmask);
	ndpi_set_protocol_detection_bitmask2(n->ndpi_struct, &n->protocols_bitmask);
	if(bt_hash_size > 32) bt_hash_size = 32;
	ndpi_bittorrent_init(n->ndpi_struct,bt_hash_size*1024,bt_hash_tmo,128);
	n->pde = proc_mkdir(dir_name, net->proc_net);
	if(!n->pde) {
		ndpi_exit_detection_module(n->ndpi_struct, free_wrapper);
		pr_err("xt_ndpi: cant create net/%s\n",dir_name);
		return -ENOMEM;
	}
	n->n_hash = -1;
	n->pe_info = proc_create_data(info_name, S_IRUGO | S_IWUSR,
	                         n->pde, &ninfo_proc_fops, n);

	if(!n->pe_info) {
		PROC_REMOVE(n->pde,net);
		ndpi_exit_detection_module(n->ndpi_struct, free_wrapper);
		pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,info_name);
		return -ENOMEM;
	}
	n->pe_proto = proc_create_data(proto_name, S_IRUGO | S_IWUSR,
	                         n->pde, &nproto_proc_fops, n);

	if(!n->pe_proto) {
		remove_proc_entry(info_name, n->pde);
		PROC_REMOVE(n->pde,net);
		ndpi_exit_detection_module(n->ndpi_struct, free_wrapper);
		pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,info_name);
		return -ENOMEM;
	}
#ifdef BT_ANNOUNCE
	n->pe_ann = proc_create_data(ann_name, S_IRUGO,
	                         n->pde, &nann_proc_fops, n);
	if(!n->pe_ann) {
		remove_proc_entry(proto_name, n->pde);
		remove_proc_entry(info_name, n->pde);
		PROC_REMOVE(n->pde,net);
		ndpi_exit_detection_module(n->ndpi_struct, free_wrapper);
		pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,ann_name);
		return -ENOMEM;
	}

#endif
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	n->pe_info6 = proc_create_data(info6_name, S_IRUGO | S_IWUSR,
	                         n->pde, &ninfo6_proc_fops, n);
	if(!n->pe_info6) {
		remove_proc_entry(proto_name, n->pde);
		remove_proc_entry(info_name, n->pde);
#ifdef BT_ANNOUNCE
		remove_proc_entry(ann_name, n->pde);
#endif
		PROC_REMOVE(n->pde,net);
		ndpi_exit_detection_module(n->ndpi_struct, free_wrapper);
		pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,info6_name);
		return -ENOMEM;
	}

#endif
	if(bt_hash_size) {
		init_timer(&n->gc);
		n->gc.data = (unsigned long)n;
		n->gc.function = bt_port_gc;
		n->gc.expires = jiffies + HZ/2;
		add_timer(&n->gc);
	}

	return 0;
}

#ifndef NF_CT_CUSTOM
static void replace_nf_destroy(void)
{
	void (*destroy)(struct nf_conntrack *);
	rcu_read_lock();
	destroy = rcu_dereference(nf_ct_destroy);
	BUG_ON(destroy == NULL);
	rcu_assign_pointer(ndpi_nf_ct_destroy,destroy);
        RCU_INIT_POINTER(nf_ct_destroy, ndpi_destroy_conntrack);
	rcu_read_unlock();
}

static void restore_nf_destroy(void)
{
	void (*destroy)(struct nf_conntrack *);
	rcu_read_lock();
	destroy = rcu_dereference(nf_ct_destroy);
	BUG_ON(destroy != ndpi_destroy_conntrack);
	destroy = rcu_dereference(ndpi_nf_ct_destroy);
	BUG_ON(destroy == NULL);
	rcu_assign_pointer(nf_ct_destroy,destroy);
	rcu_read_unlock();
}
#else
static struct nf_ct_ext_type ndpi_extend = {
       .seq_print = seq_print_ndpi,
       .destroy   = nf_ndpi_free_flow,
       .len    = sizeof(struct nf_ct_ext_ndpi),
       .align  = __alignof__(struct nf_ct_ext_ndpi),
};
#endif

static void __net_exit ndpi_net_exit(struct net *net)
{
        ndpi_cleanup(net);
}

static struct pernet_operations ndpi_net_ops = {
        .init   = ndpi_net_init,
        .exit   = ndpi_net_exit,
        .id     = &ndpi_net_id,
        .size   = sizeof(struct ndpi_net),
};


static int __init ndpi_mt_init(void)
{
        int ret;

/*	size_id_struct = ndpi_detection_get_sizeof_ndpi_id_struct() + sizeof(struct osdpi_id_node); */
	size_id_struct = sizeof(struct osdpi_id_node);
	size_flow_struct = ndpi_detection_get_sizeof_ndpi_flow_struct();
	detection_tick_resolution = HZ;

#ifdef NF_CT_CUSTOM
	ret = nf_ct_extend_custom_register(&ndpi_extend,0x4e445049); /* "NDPI" in hex */
	if(ret < 0) {
		pr_err("xt_ndpi: can't nf_ct_extend_register.\n");
		return -EBUSY;
	}
	nf_ct_ext_id_ndpi = ndpi_extend.id;
#else
	nf_ct_ext_id_ndpi = NF_CT_EXT_LABELS;
#endif

	ret = register_pernet_subsys(&ndpi_net_ops);
	if (ret < 0) {
		pr_err("xt_ndpi: can't register_pernet_subsys.\n");
		goto unreg_ext;
	}

        ret = xt_register_match(&ndpi_mt_reg);
        if (ret) {
                pr_err("xt_ndpi: error registering ndpi match.\n");
		goto unreg_pernet;
        }

        ret = xt_register_target(&ndpi_tg_reg);
        if (ret) {
                pr_err("xt_ndpi: error registering ndpi match.\n");
		goto unreg_match;
        }

	ret = -ENOMEM;

        osdpi_flow_cache = kmem_cache_create("ndpi_flows", size_flow_struct,
                                             0, 0, NULL);
        if (!osdpi_flow_cache) {
                pr_err("xt_ndpi: error creating flow cache.\n");
		goto unreg_target;
        }
        
        osdpi_id_cache = kmem_cache_create("ndpi_ids",
                                           size_id_struct,
                                           0, 0, NULL);
        if (!osdpi_id_cache) {
		pr_err("xt_ndpi: error creating id cache.\n");
		goto free_flow;
	}

        bt_port_cache = kmem_cache_create("ndpi_btport",
				 sizeof(struct hash_ip4p_node)
#ifdef NDPI_DETECTION_SUPPORT_IPV6
				 +12
#endif
				 , 0, 0, NULL);
        if (!bt_port_cache) {
		pr_err("xt_ndpi: error creating port cache.\n");
		goto free_id;
	}
	if(bt_hash_size && bt_hash_size > 32) bt_hash_size = 32;
	if(!bt_hash_tmo) bt_hash_tmo = 900;
	if( bt_hash_tmo > 3600) bt_hash_tmo = 3600;

#ifndef NF_CT_CUSTOM
	replace_nf_destroy();
#endif

	pr_info("xt_ndpi v1.2 ndpi %s %s\n"
		" bt hash size %luk gc timeout %ld sec\n"
		" sizeof hash_ip4p_node  %zu\n"
		" sizeof id_struct %zu\n"
		" sizeof flow_struct %zu\n"
		"  sizeof packet_struct %zu\n"
		"  sizeof flow_tcp_struct %zu\n"
		"  sizeof flow_udp_struct %zu\n"
		"  sizeof int_one_line_struct %zu\n"
		" sizeof ndpi_ip_addr_t %zu\n"
		" ext ID %d\n",
		NDPI_GIT_RELEASE,
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		"with IPv6",
#else
		"without IPv6",
#endif
		bt_hash_size,bt_hash_tmo,
		sizeof(struct hash_ip4p_node)
#ifdef NDPI_DETECTION_SUPPORT_IPV6
				 +12
#endif
		, size_id_struct,size_flow_struct,
		sizeof(struct ndpi_packet_struct),
		sizeof(struct ndpi_flow_tcp_struct),
		sizeof(struct ndpi_flow_udp_struct),
		sizeof(struct ndpi_int_one_line_struct),
		sizeof(ndpi_ip_addr_t),
		nf_ct_ext_id_ndpi);
	return 0;

free_id:
       	kmem_cache_destroy (osdpi_id_cache);
free_flow:
       	kmem_cache_destroy (osdpi_flow_cache);
unreg_target:
	xt_unregister_target(&ndpi_tg_reg);
unreg_match:
	xt_unregister_match(&ndpi_mt_reg);
unreg_pernet:
	unregister_pernet_subsys(&ndpi_net_ops);
unreg_ext:
#ifdef NF_CT_CUSTOM
	nf_ct_extend_unregister(&ndpi_extend);
#endif
       	return ret;
}


static void __exit ndpi_mt_exit(void)
{
	pr_info("xt_ndpi 1.2 unload.\n");

        kmem_cache_destroy (bt_port_cache);
        kmem_cache_destroy (osdpi_id_cache);
        kmem_cache_destroy (osdpi_flow_cache);
	xt_unregister_target(&ndpi_tg_reg);
	xt_unregister_match(&ndpi_mt_reg);
	unregister_pernet_subsys(&ndpi_net_ops);
#ifdef NF_CT_CUSTOM
	nf_ct_extend_unregister(&ndpi_extend);
#else
	restore_nf_destroy();
#endif
}


module_init(ndpi_mt_init);
module_exit(ndpi_mt_exit);
