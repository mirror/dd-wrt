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

#include <linux/rbtree.h>
#include <linux/time.h>
#include <linux/atomic.h>
#include <linux/proc_fs.h>
#include <linux/jiffies.h>

#include <net/net_namespace.h>
#include <net/netns/generic.h>

#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/inetdevice.h>
#include <linux/if_ether.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
#include <linux/vmalloc.h>
#endif


#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>
#include <net/netfilter/nf_nat.h>

#define BT_ANNOUNCE 

#include "ndpi_config.h"
#undef HAVE_HYPERSCAN
#include "ndpi_main.h"

#include "xt_ndpi.h"

#include "../lib/third_party/include/ndpi_patricia.h"
#include "../lib/third_party/include/ahocorasick.h"


static ndpi_protocol_match host_match[];

/* Only for debug! */
//#define NDPI_IPPORT_DEBUG


#define COUNTER(a) (volatile unsigned long int)(a)++

#define NDPI_PROCESS_ERROR (NDPI_NUM_BITS+1)
#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF    89
#endif

static char dir_name[]="xt_ndpi";
static char info_name[]="info";
static char ipdef_name[]="ip_proto";
static char hostdef_name[]="host_proto";
static char flow_name[]="flows";
#ifdef NDPI_DETECTION_SUPPORT_IPV6
static char info6_name[]="info6";
#endif
#ifdef BT_ANNOUNCE
static char ann_name[]="announce";
#endif

static char proto_name[]="proto";


#if 1
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
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
static inline struct net *xt_net(const struct xt_action_param *par)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0)
        const struct net_device *dev = par->in;
        if(!dev) dev = par->out;
        return dev ? dev_net(dev): NULL;
#else
        return par->net;
#endif
}
static inline u_int8_t xt_family(const struct xt_action_param *par)
{
        return par->family;
}
static inline unsigned int xt_hooknum(const struct xt_action_param *par)
{
        return par->hooknum;
}
static inline const struct net_device *xt_in(const struct xt_action_param *par)
{
        return par->in;
}
static inline const struct net_device *xt_out(const struct xt_action_param *par)
{
        return par->out;
}
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0)
#define nf_register_net_hooks(net,a,s) nf_register_hooks(a,s)
#define nf_unregister_net_hooks(net,a,s) nf_unregister_hooks(a,s)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
#define refcount_dec_and_test(a) atomic_sub_and_test((int) 1,(a))
#endif

// for testing only!
// #define USE_CONNLABELS

#if !defined(USE_CONNLABELS) && defined(CONFIG_NF_CONNTRACK_CUSTOM) && CONFIG_NF_CONNTRACK_CUSTOM > 0
#define NF_CT_CUSTOM
#else
#undef NF_CT_CUSTOM
#include <net/netfilter/nf_conntrack_labels.h>
#ifndef CONFIG_NF_CONNTRACK_LABELS
#error NF_CONNTRACK_LABELS not defined
#endif
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0)
#define nf_ct_l3proto_try_module_get(a) 0
#define nf_ct_l3proto_module_put(a)
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vitaly E. Lavrov <vel21ripn@gmail.com>, G. Elian Gidoni <geg@gnu.org>");
MODULE_DESCRIPTION("nDPI wrapper");
MODULE_ALIAS("ipt_ndpi");
MODULE_ALIAS("ipt_NDPI");

/* id tracking */
struct osdpi_id_node {
        struct rb_node node;
        struct kref refcnt;
        union  nf_inet_addr ip;
        struct ndpi_id_struct ndpi_id;
};


#include "ndpi_strcol.h"
#include "ndpi_flow_info.h"
#include "ndpi_main_netfilter.h"
#include "ndpi_main_common.h"
#include "ndpi_proc_generic.h"
#include "ndpi_proc_parsers.h"
#include "ndpi_proc_info.h"
#include "ndpi_proc_flow.h"
#include "ndpi_proc_hostdef.h"
#include "ndpi_proc_ipdef.h"

#include "../libre/regexp.h"

#define MAGIC_CT 0xa55a
struct nf_ct_ext_labels { /* max size 128 bit */
	/* words must be first byte for compatible with NF_CONNLABELS
	 * kernels 3.8-4.7 has variable size of nf_ext_labels
	 * kernels 4.8 has fixed size of nf_ext_labels
	 * 32bit - 8 bytes, 64bit - 16 bytes
	 */
	uint8_t			words,pad1;
	uint16_t		magic;
#if __SIZEOF_LONG__ != 4
	uint8_t			pad2[4];
#endif
	struct nf_ct_ext_ndpi	*ndpi_ext;
};

struct ndpi_cb {
	unsigned long 	last_ct; // sizeof(unsigned long) == sizeof(void *)
	uint32_t	proto;
	uint32_t	magic;
};

static struct nf_ct_ext_ndpi dummy_struct1;
static struct flow_info dummy_struct2;

static inline struct ndpi_cb *skb_get_cproto(const struct sk_buff *skb)
{
	return (struct ndpi_cb *)&skb->cb[sizeof(skb->cb)-sizeof(struct ndpi_cb)];
}

static inline struct nf_conn *ct_proto_last(struct ndpi_cb *cb)
{
	return (struct nf_conn *)(READ_ONCE(cb->last_ct) & ~0x7ul);
}
static inline unsigned int ct_proto_get_flow_nat(struct ndpi_cb *cb)
{
	return READ_ONCE(cb->last_ct) & 7;
}
#define FLOW_NAT_START 1
#define FLOW_NAT_END 2

static inline void ct_proto_set_flow_nat(struct ndpi_cb *cb,uint8_t v) {
	unsigned long o,n;
	do {
		o = READ_ONCE(cb->last_ct);
		n = o | v;
	} while( o != n && cmpxchg(&cb->last_ct,o,n) != o);
}

static inline void ct_proto_set_flow(struct ndpi_cb *cb,void *ct, uint8_t v)
{
	WRITE_ONCE(cb->last_ct,(unsigned long int)ct | v);
}

static inline int flow_have_info( struct nf_ct_ext_ndpi *c) {
	unsigned long int m = (1 << f_flow_yes) | (1 << f_flow_info);
	return (READ_ONCE(c->flags) & m) == m;
}

static ndpi_protocol_nf proto_null = NDPI_PROTOCOL_NULL;

static unsigned long int ndpi_flow_limit=10000000; // 4.3Gb
static unsigned long int ndpi_enable_flow=0;
static unsigned long int ndpi_log_debug=0;
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
static unsigned long  ndpi_lib_trace=0;
#endif
static unsigned long  ndpi_mtu=48000;
static unsigned long  bt_log_size=128;
static unsigned long int bt_hash_size=2;
static unsigned long int bt6_hash_size=0;
static unsigned long int bt_hash_tmo=1200;

static unsigned long  max_packet_unk_tcp=20;
static unsigned long  max_packet_unk_udp=20;
static unsigned long  max_packet_unk_other=20;

static unsigned long  flow_read_debug=0;

static unsigned long  ndpi_size_flow_struct=0;
static unsigned long  ndpi_size_id_struct=0;
static unsigned long  ndpi_size_hash_ip4p_node=0;

static unsigned long  ndpi_jumbo=0;
static unsigned long  ndpi_falloc=0;
static unsigned long  ndpi_nskb=0;
static unsigned long  ndpi_lskb=0;
static unsigned long  ndpi_flow_c=0;
static unsigned long  ndpi_flow_d=0;
static unsigned long  ndpi_bt_gc=0;

static unsigned long  ndpi_p0=0;
static unsigned long  ndpi_p1=0;
static unsigned long  ndpi_p2=0;
static unsigned long  ndpi_p31=0;
static unsigned long  ndpi_p34=0;
static unsigned long  ndpi_p7=0;
static unsigned long  ndpi_p9=0;
static unsigned long  ndpi_pa=0;
static unsigned long  ndpi_pb=0;
static unsigned long  ndpi_pc=0;
static unsigned long  ndpi_pd=0;
static unsigned long  ndpi_pe=0;
static unsigned long  ndpi_pf=0;
static unsigned long  ndpi_pg=0;
static unsigned long  ndpi_ph=0;
static unsigned long  ndpi_pi=0;
static unsigned long  ndpi_pi2=0;
static unsigned long  ndpi_pi3=0;
static unsigned long  ndpi_pi4=0;
static unsigned long  ndpi_pj=0;
static unsigned long  ndpi_pjc=0;
static unsigned long  ndpi_pk=0;

static unsigned long  ndpi_pl[11]={0,};
static unsigned long  ndpi_btp_tm[20]={0,};

module_param_named(xt_debug,   ndpi_log_debug, ulong, 0600);
MODULE_PARM_DESC(xt_debug,"Debug level for xt_ndpi (0-3).");
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
module_param_named(lib_trace,  ndpi_lib_trace, ulong, 0600);
MODULE_PARM_DESC(lib_trace,"Debug level for nDPI library (0-off, 1-error, 2-trace, 3-debug, 4->extra debug");
#endif
module_param_named(mtu, ndpi_mtu, ulong, 0600);
MODULE_PARM_DESC(mtu,"Skip checking nonlinear skbuff larger than MTU");

module_param_named(bt_log_size, bt_log_size, ulong, 0400);
MODULE_PARM_DESC(bt_log_size,"Keep information about the lastes N bt-hash. default 0, range: 32 - 512");
module_param_named(bt_hash_size, bt_hash_size, ulong, 0400);
MODULE_PARM_DESC(bt_hash_size,"BT hash table size ( *1000 ). default 0, range: 8-512");
#ifdef NDPI_DETECTION_SUPPORT_IPV6
module_param_named(bt6_hash_size, bt6_hash_size, ulong, 0400);
MODULE_PARM_DESC(bt6_hash_size,"BTv6 hash table size ( *1000 ). default 0, range: 8-32");
#endif
module_param_named(bt_hash_timeout, bt_hash_tmo, ulong, 0400);
MODULE_PARM_DESC(bt_hash_timeout,"The expiration time for inactive records in BT-hash (sec). default 1200 range: 900-3600");

module_param_named(ndpi_enable_flow, ndpi_enable_flow, ulong, 0400);
MODULE_PARM_DESC(ndpi_enable_flow,"Enable netflow info");

module_param_named(ndpi_flow_limit, ndpi_flow_limit, ulong, 0400);
MODULE_PARM_DESC(ndpi_flow_limit,"Limit netflow records. Default 10000000 (~4.3Gb RAM)");

module_param_named(max_unk_tcp,max_packet_unk_tcp,ulong, 0600);
module_param_named(max_unk_udp,max_packet_unk_udp,ulong, 0600);
module_param_named(max_unk_other,max_packet_unk_other,ulong, 0600);
module_param_named(flow_read_debug,flow_read_debug,ulong, 0600);

module_param_named(ndpi_size_flow_struct,ndpi_size_flow_struct,ulong, 0400);
module_param_named(ndpi_size_id_struct,ndpi_size_id_struct,ulong, 0400);
module_param_named(ndpi_size_hash_ip4p_node,ndpi_size_hash_ip4p_node,ulong, 0400);

module_param_named(err_oversize, ndpi_jumbo, ulong, 0400);
MODULE_PARM_DESC(err_oversize,"Counter nonlinear packets bigger than MTU. [info]");
module_param_named(err_skb_linear, ndpi_falloc, ulong, 0400);
MODULE_PARM_DESC(err_skb_linear,"Counter of unsuccessful conversions of nonlinear packets. [error]");

module_param_named(skb_seg,	 ndpi_nskb, ulong, 0400);
MODULE_PARM_DESC(skb_seg,"Counter nonlinear packets. [info]");
module_param_named(skb_lin,	 ndpi_lskb, ulong, 0400);
MODULE_PARM_DESC(skb_lin,"Counter linear packets. [info]");

module_param_named(flow_created, ndpi_flow_c, ulong, 0400);
MODULE_PARM_DESC(flow_created,"Counter of created flows. [info]");
module_param_named(flow_deleted, ndpi_flow_d, ulong, 0400);
MODULE_PARM_DESC(flow_deleted,"Counter of destroyed flows. [info]");
module_param_named(bt_gc_count,  ndpi_bt_gc, ulong, 0400);

module_param_named(ipv4,         ndpi_p0, ulong, 0400);
module_param_named(ipv6,         ndpi_pa, ulong, 0400);
module_param_named(nonip,        ndpi_pb, ulong, 0400);
module_param_named(err_ip_frag_len, ndpi_p1, ulong, 0400);
module_param_named(err_bad_tcp_udp, ndpi_p2, ulong, 0400);
module_param_named(ct_confirm,   ndpi_p31, ulong, 0400);
module_param_named(err_add_ndpi, ndpi_p34, ulong, 0400);
module_param_named(non_tcpudp,   ndpi_p7, ulong, 0400);
module_param_named(max_parsed_lines, ndpi_p9, ulong, 0400);
module_param_named(id_num,	 ndpi_pc, ulong, 0400);
module_param_named(noncached,	 ndpi_pd, ulong, 0400);
module_param_named(err_prot_err, ndpi_pe, ulong, 0400);
module_param_named(err_prot_err1, ndpi_pf, ulong, 0400);
module_param_named(err_alloc_flow, ndpi_pg, ulong, 0400);
module_param_named(err_alloc_id, ndpi_ph, ulong, 0400);
module_param_named(cached,	 ndpi_pi,  ulong, 0400);
module_param_named(c_skb_not,	 ndpi_pi2, ulong, 0400);
module_param_named(c_last_ct_not, ndpi_pi3, ulong, 0400);
module_param_named(c_magic_not,	 ndpi_pi4, ulong, 0400);
module_param_named(l4mismatch,	 ndpi_pj,  ulong, 0400);
module_param_named(l4mis_size,	 ndpi_pjc, ulong, 0400);
module_param_named(ndpi_match,	 ndpi_pk,  ulong, 0400);

static unsigned long  ndpi_pto=0,
	       ndpi_ptss=0, ndpi_ptsd=0,
	       ndpi_ptds=0, ndpi_ptdd=0,
	       ndpi_ptussf=0,ndpi_ptusdr=0,
	       ndpi_ptussr=0,ndpi_ptusdf=0,
	       ndpi_ptudsf=0,ndpi_ptuddr=0,
	       ndpi_ptudsr=0,ndpi_ptuddf=0 ;
static unsigned long 
	       ndpi_pusf=0,ndpi_pusr=0,
	       ndpi_pudf=0,ndpi_pudr=0,
	       ndpi_puo=0;


static int net_ns_id=0;
static int ndpi_net_id;
static inline struct ndpi_net *ndpi_pernet(struct net *net)
{
	        return net_generic(net, ndpi_net_id);
}

/* detection */
static uint32_t detection_tick_resolution = 1000;

static	enum nf_ct_ext_id nf_ct_ext_id_ndpi = 0;
static	struct kmem_cache *osdpi_flow_cache = NULL;
static	struct kmem_cache *osdpi_id_cache = NULL;
static struct kmem_cache *ct_info_cache = NULL;
static struct kmem_cache *bt_port_cache = NULL;

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
static char *dbl_lvl_txt[5] = {
	"ERR",
	"TRACE",
	"DEBUG",
	"DEBUG2",
	NULL
};
/* debug functions */
NDPI_STATIC void debug_printf(u_int32_t protocol, void *id_struct, ndpi_log_level_t log_level,
	const char *file_name, const char *func_name, unsigned line_number, const char * format, ...)
{
	struct ndpi_net *n = id_struct ? ((struct ndpi_detection_module_struct *)id_struct)->user_data : NULL;
	if(!n || protocol >= NDPI_NUM_BITS)
		pr_info("ndpi_debug n=%d, p=%u, l=%s\n",n != NULL,protocol,
				log_level < 5 ? dbl_lvl_txt[log_level]:"???");
	if(!n || protocol >= NDPI_NUM_BITS) return;
	
	if(log_level+1 <= ( ndpi_lib_trace < n->debug_level[protocol] ?
				ndpi_lib_trace : n->debug_level[protocol]))  {
		char buf[256];
		const char *short_fn;
        	va_list args;

		memset(buf, 0, sizeof(buf));
        	va_start(args, format);
		vsnprintf(buf, sizeof(buf)-1, format, args);
       		va_end(args);
		short_fn = strrchr(file_name,'/');
		if(!short_fn)
			short_fn = file_name;
		    else
			short_fn++;

		switch(log_level) {
		case NDPI_LOG_ERROR:
                	pr_err("E: P=%d %s:%d:%s %s",protocol, short_fn, line_number, func_name, buf);
			break;
		case NDPI_LOG_TRACE:
                	pr_info("T: P=%d %s:%d:%s %s",protocol, short_fn, line_number, func_name, buf);
			break;
		case NDPI_LOG_DEBUG:
                	pr_info("D: P=%d %s:%d:%s %s",protocol, short_fn, line_number, func_name, buf);
			break;
		case NDPI_LOG_DEBUG_EXTRA:
                	pr_info("D2: P=%d %s:%d:%s %s",protocol, short_fn, line_number, func_name, buf);
			break;
		default:
			;
		}
        }
}

void set_debug_trace( struct ndpi_net *n) {
	int i;
	const char *t_proto;
	ndpi_debug_function_ptr dbg_printf = (ndpi_debug_function_ptr)NULL;
	if(ndpi_lib_trace)
	    for(i=0; i < NDPI_NUM_BITS; i++) {
		if(!n->mark[i].mark && !n->mark[i].mask) continue;
		t_proto = ndpi_get_proto_by_id(n->ndpi_struct,i);
		if(t_proto) {
			if(!n->debug_level[i]) continue;
			dbg_printf = debug_printf;
			break;
		}
	    }
	if(n->ndpi_struct->ndpi_debug_printf != dbg_printf) {
		pr_info("ndpi:%s debug message %s\n",n->ns_name,
			dbg_printf != NULL ? "ON":"OFF");
		set_ndpi_debug_function(n->ndpi_struct, dbg_printf);
	} else {
		if(ndpi_log_debug)
		  pr_info("ndpi:%s debug %s (not changed)\n",n->ns_name,
			n->ndpi_struct->ndpi_debug_printf != NULL ? "on":"off");
	}
}
#endif

NDPI_STATIC char *ct_info(const struct nf_conn * ct,char *buf,size_t buf_size,int dir);


#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)

NDPI_STATIC void kvfree(const void *addr)
{
	if (is_vmalloc_addr(addr))
		vfree(addr);
	else
		kfree(addr);
}

#ifndef __GFP_REPEAT
#define		__GFP_REPEAT __GFP_RETRY_MAYFAIL
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
static void *kvmalloc_node(size_t size, gfp_t flags, int node)
{
	gfp_t kmalloc_flags = flags;
	void *ret;

	/*
	 * vmalloc uses GFP_KERNEL for some internal allocations (e.g page tables)
	 * so the given set of flags has to be compatible.
	 */
	WARN_ON_ONCE((flags & GFP_KERNEL) != GFP_KERNEL);

	/*
	 * Make sure that larger requests are not too disruptive - no OOM
	 * killer and no allocation failure warnings as we have a fallback
	 */
	if (size > PAGE_SIZE) {
		kmalloc_flags |= __GFP_NOWARN;

		if (!(kmalloc_flags & __GFP_REPEAT))
			kmalloc_flags |= __GFP_NORETRY;
	}

	ret = kmalloc_node(size, kmalloc_flags, node);

	/*
	 * It doesn't really make sense to fallback to vmalloc for sub page
	 * requests
	 */
	if (ret || size <= PAGE_SIZE)
		return ret;

	return (__vmalloc(size, flags | __GFP_HIGHMEM, PAGE_KERNEL));
}

static inline void *kvmalloc(size_t size, gfp_t flags)
{
	return kvmalloc_node(size, flags, NUMA_NO_NODE);
}

#endif

NDPI_STATIC void *malloc_wrapper(size_t size)
{
	if(size > 32*1024) {
		/*
		 * Workarround for 32bit systems
		 * Large memory areas (more than 128 KB) are requested 
		 * only during initial initialization. 
		 * In this case, we can use kvmalloc() instead of kmalloc().
		 */
		return kvmalloc(size,GFP_KERNEL);
	}
	return kmalloc(size,(in_atomic() || irqs_disabled()) ? GFP_ATOMIC : GFP_KERNEL);
}

NDPI_STATIC void free_wrapper(void *freeable)
{
	kvfree(freeable);
}

NDPI_STATIC void fill_prefix_any(prefix_t *p, union nf_inet_addr *ip,int family) {
	memset(p, 0, sizeof(prefix_t));
	p->ref_count = 0;
	if(family == AF_INET) {
		memcpy(&p->add.sin, ip, 4);
		p->family = AF_INET;
		p->bitlen = 32;
		return;
	}
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	if(family == AF_INET6) {
		memcpy(&p->add.sin, ip, 16);
		p->family = AF_INET6;
		p->bitlen = 128;
	}
#endif
}

NDPI_STATIC struct ndpi_id_struct *
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

NDPI_STATIC void
ndpi_free_id (struct ndpi_net *n, struct osdpi_id_node * id)
{
	if (refcount_dec_and_test(&id->refcnt.refcount)) {
	        rb_erase(&id->node, &n->osdpi_id_root);
	        kmem_cache_free (osdpi_id_cache, id);
		(volatile unsigned long int)ndpi_pc--;
	}
}

#ifdef NF_CT_CUSTOM
NDPI_STATIC inline void *nf_ct_ext_add_ndpi(struct nf_conn * ct)
{
  #if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
	return nf_ct_ext_add(ct,nf_ct_ext_id_ndpi,GFP_ATOMIC);
  #elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	return __nf_ct_ext_add_length(ct,nf_ct_ext_id_ndpi,
		sizeof(struct nf_ct_ext_ndpi),GFP_ATOMIC);
  #else
	return __nf_ct_ext_add(ct,nf_ct_ext_id_ndpi,GFP_ATOMIC);
  #endif
}
#endif

NDPI_STATIC inline struct nf_ct_ext_ndpi *nf_ct_get_ext_ndpi(struct nf_ct_ext_labels *ext_l) {
	return ext_l && ext_l->magic == MAGIC_CT ? ext_l->ndpi_ext:NULL;
}

NDPI_STATIC inline struct nf_ct_ext_ndpi *nf_ct_ext_find_ndpi(const struct nf_conn * ct)
{
struct nf_ct_ext_labels *l = (struct nf_ct_ext_labels *)__nf_ct_ext_find(ct,nf_ct_ext_id_ndpi);
return nf_ct_get_ext_ndpi(l);
}

NDPI_STATIC inline struct nf_ct_ext_labels *nf_ct_ext_find_label(const struct nf_conn * ct)
{
	return (struct nf_ct_ext_labels *)__nf_ct_ext_find(ct,nf_ct_ext_id_ndpi);
}


NDPI_STATIC void ndpi_ct_list_add(struct ndpi_net *n,
			struct nf_ct_ext_ndpi *ct_ndpi) {

	struct nf_ct_ext_ndpi *h;

	do {
	    h = READ_ONCE(n->flow_h);
	    WRITE_ONCE(ct_ndpi->next,h);
	} while(cmpxchg(&n->flow_h,h,ct_ndpi) != h);

	set_flow_yes(ct_ndpi);
	atomic_inc(&n->acc_work);
}

NDPI_STATIC void ndpi_init_ct_struct(struct ndpi_net *n, 
		struct nf_ct_ext_ndpi *ct_ndpi,
		uint8_t l4_proto, struct nf_conn * ct,
		int is_ipv6, uint32_t s_time) {

	size_t addr_size = is_ipv6 ? 16:4;
	const struct nf_conntrack_tuple *tuple;

	spin_lock_init(&ct_ndpi->lock);
	ct_ndpi->l4_proto = l4_proto;
	if(is_ipv6)
		set_ipv6(ct_ndpi);

	if(!ndpi_enable_flow) return;

	tuple = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;

	memcpy(&ct_ndpi->flinfo.ip_s, &tuple->src.u3,addr_size);
	memcpy(&ct_ndpi->flinfo.ip_d, &tuple->dst.u3,addr_size);
	if(l4_proto == IPPROTO_TCP || l4_proto == IPPROTO_UDP ||
	   l4_proto == IPPROTO_UDPLITE || l4_proto == IPPROTO_SCTP) {
		ct_ndpi->flinfo.sport = tuple->src.u.tcp.port;
		ct_ndpi->flinfo.dport = tuple->dst.u.tcp.port;
	}
	if(l4_proto == IPPROTO_ICMP && (
		tuple->dst.u.icmp.type == ICMP_ECHO || 
		tuple->dst.u.icmp.type == ICMP_ECHOREPLY)) {
		ct_ndpi->flinfo.sport = (tuple->dst.u.icmp.type << 8) | tuple->dst.u.icmp.code;
		ct_ndpi->flinfo.dport = tuple->src.u.icmp.id;
	}

	ct_ndpi->flinfo.time_start = s_time;
	if(ndpi_log_debug > 1)
	  pr_info("ndpi:%s init_ct_struct ct_ndpi %pK ct %pK prot %u\n",n->ns_name,
			ct_ndpi,ct,l4_proto);
}

NDPI_STATIC void ndpi_nat_detect(struct nf_ct_ext_ndpi *ct_ndpi, struct nf_conn * ct) {

	size_t addr_size = test_ipv6(ct_ndpi) ? 16:4;
	const uint8_t l4_proto = ct_ndpi->l4_proto;
	int ns_ip,nd_ip,ns_port=0,nd_port=0;
	const struct nf_conntrack_tuple *tuple;

	tuple = &ct->tuplehash[IP_CT_DIR_REPLY].tuple;
	ns_ip = memcmp(&ct_ndpi->flinfo.ip_d, &tuple->src.u3,addr_size);
	nd_ip = memcmp(&ct_ndpi->flinfo.ip_s, &tuple->dst.u3,addr_size);
	if(l4_proto == IPPROTO_TCP || l4_proto == IPPROTO_UDP ||
	   l4_proto == IPPROTO_UDPLITE) {
		ns_port = ct_ndpi->flinfo.dport != tuple->src.u.tcp.port;
		nd_port = ct_ndpi->flinfo.sport != tuple->dst.u.tcp.port;
	}
	if(!test_ipv6(ct_ndpi)) {
	    if(ns_ip || ns_port) {
		set_dnat(ct_ndpi);
		memcpy(&ct_ndpi->flinfo.ip_dnat, &tuple->src.u3,addr_size);
		ct_ndpi->flinfo.dport_nat = tuple->src.u.tcp.port;
	    }
	    if(nd_ip || nd_port) {
		set_snat(ct_ndpi);
		memcpy(&ct_ndpi->flinfo.ip_snat, &tuple->dst.u3,addr_size);
		ct_ndpi->flinfo.sport_nat = tuple->dst.u.tcp.port;
	    }
	}
	if(ndpi_log_debug > 1)
		pr_info("ndpi: %s ct_ndpi %pK\n",__func__,ct_ndpi);
}

NDPI_STATIC inline void ndpi_ct_counters_add(struct nf_ct_ext_ndpi *ct_ndpi,
		size_t npkt, size_t len, enum ip_conntrack_info ctinfo, uint32_t m_time) {

	int rev = CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL ? 1:0;
	ct_ndpi->flinfo.b[rev] += len;
	ct_ndpi->flinfo.p[rev] += npkt;
	ct_ndpi->flinfo.time_end = m_time;
	set_flow_info(ct_ndpi);
	if(ndpi_log_debug > 1)
		pr_info("ndpi: ct_ndpi %pK counter pkt %zu bytes %zu\n",ct_ndpi,npkt,len);
}
		

NDPI_STATIC inline void __ndpi_free_ct_flow(struct nf_ct_ext_ndpi *ct_ndpi) {
	if(ct_ndpi->flow != NULL) {
		ndpi_free_flow(ct_ndpi->flow);
		kmem_cache_free (osdpi_flow_cache, ct_ndpi->flow);
		ct_ndpi->flow = NULL;
		COUNTER(ndpi_flow_d);
		module_put(THIS_MODULE);
	}
}
NDPI_STATIC inline void __ndpi_free_ct_ndpi_id(struct ndpi_net *n, struct nf_ct_ext_ndpi *ct_ndpi) {
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
}

NDPI_STATIC inline void __ndpi_free_ct_proto(struct nf_ct_ext_ndpi *ct_ndpi) {
        if(ct_ndpi->host) {
                kfree(ct_ndpi->host);
                ct_ndpi->host = NULL;
        }
        if(ct_ndpi->ssl) {
                kfree(ct_ndpi->ssl);
                ct_ndpi->ssl = NULL;
        }
}

static void
ct_ndpi_free_flow (struct ndpi_net *n,
		struct nf_ct_ext_labels *ext_l,
		struct nf_ct_ext_ndpi *ct_ndpi,
		int force)
{
	int delete = 0;

	WRITE_ONCE(ext_l->magic, 0);
	WRITE_ONCE(ext_l->ndpi_ext, NULL);
	smp_wmb(); 

	spin_lock_bh(&ct_ndpi->lock);
	__ndpi_free_ct_flow(ct_ndpi);
	__ndpi_free_ct_ndpi_id(n,ct_ndpi);
	if(test_flow_yes(ct_ndpi)) {
	    if(force || !flow_have_info(ct_ndpi)) {
	    	__ndpi_free_ct_proto(ct_ndpi);
	    	clear_flow_info(ct_ndpi);
	    }
	    set_for_delete(ct_ndpi);
	    atomic_inc(&n->acc_rem);
	} else
	    delete = 1;
	spin_unlock_bh(&ct_ndpi->lock);

	if(delete)
		kmem_cache_free (ct_info_cache, ct_ndpi);
}

/* free ndpi info on ndpi_net_exit() */
NDPI_STATIC int
__ndpi_free_flow (struct nf_conn * ct,void *data) {
	struct nf_ct_ext_labels *ext_l = nf_ct_ext_find_label(ct);

	struct nf_ct_ext_ndpi *ct_ndpi = nf_ct_get_ext_ndpi(ext_l);
	struct ndpi_net *n = (struct ndpi_net *)data;

	if(!ct_ndpi) return 1;

	if(ndpi_log_debug > 1)
	    pr_info("ndpi:%s ct_ndpi %pK free_flow\n",n->ns_name, ct_ndpi);

	ct_ndpi_free_flow(n,ext_l,ct_ndpi,1);
	return 1;
}

NDPI_STATIC void
nf_ndpi_free_flow (struct nf_conn * ct)
{
	struct nf_ct_ext_labels *ext_l = nf_ct_ext_find_label(ct);

	struct nf_ct_ext_ndpi *ct_ndpi = nf_ct_get_ext_ndpi(ext_l);

	if(ct_ndpi) {
	    struct ndpi_net *n;
	    n = ndpi_pernet(nf_ct_net(ct));

	    if(ndpi_log_debug > 1)
		pr_info("ndpi:%s free_flow ct %pK ct_ndpi %pk %s\n",
				n->ns_name,ct,ct_ndpi,
				test_flow_yes(ct_ndpi) ? "flow":"");

	    ct_ndpi_free_flow(n,ext_l,ct_ndpi,0);
	}
}

/* must be locked ct_ndpi->lock */
NDPI_STATIC struct ndpi_flow_struct * 
ndpi_alloc_flow (struct nf_ct_ext_ndpi *ct_ndpi)
{
        struct ndpi_flow_struct *flow;

        flow = kmem_cache_zalloc (osdpi_flow_cache, GFP_ATOMIC);
        if (flow == NULL) {
                pr_err("xt_ndpi: couldn't allocate new flow.\n");
                return flow;
        }

	ct_ndpi->proto = proto_null;
	ct_ndpi->flow = flow;
	__module_get(THIS_MODULE);
	COUNTER(ndpi_flow_c);
	if(ndpi_log_debug > 2)
		pr_info("ndpi: ct_ndpi %pK alloc_flow\n", ct_ndpi);
        return flow;
}
#ifndef NF_CT_CUSTOM

NDPI_STATIC void (*ndpi_nf_ct_destroy)(struct nf_conntrack *) __rcu __read_mostly;

NDPI_STATIC void ndpi_destroy_conntrack(struct nf_conntrack *nfct) {
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

NDPI_STATIC void
ndpi_enable_protocols (struct ndpi_net *n)
{
        int i,c=0;

        spin_lock_bh (&n->ipq_lock);
	if(atomic_inc_return(&n->protocols_cnt[0]) == 1) {
		for (i = 1,c=0; i < NDPI_NUM_BITS; i++) {
			if(!ndpi_get_proto_by_id(n->ndpi_struct,i))
				continue;
			if(!n->mark[i].mark && !n->mark[i].mask)
				continue;
			NDPI_ADD_PROTOCOL_TO_BITMASK(n->protocols_bitmask, i);
			c++;
		}
		ndpi_set_protocol_detection_bitmask2(n->ndpi_struct,
				&n->protocols_bitmask);
	}
	spin_unlock_bh (&n->ipq_lock);
}


NDPI_STATIC void add_stat(unsigned long int n) {

	if(n > ndpi_p9) ndpi_p9 = n;
	n /= 10;
	if(n < 0) n = 0;
	if(n > sizeof(ndpi_pl)/sizeof(ndpi_pl[0])-1)
		n = sizeof(ndpi_pl)/sizeof(ndpi_pl[0])-1;
	ndpi_pl[n]++;
}

NDPI_STATIC char *ct_info(const struct nf_conn * ct,char *buf,size_t buf_size,int dir) {
 const struct nf_conntrack_tuple *t = 
	 &ct->tuplehash[!dir ? IP_CT_DIR_ORIGINAL: IP_CT_DIR_REPLY].tuple;
 // fixme ipv6
 snprintf(buf,buf_size,"proto %u %pI4:%d -> %pI4:%d %s",
		t->dst.protonum,
		&t->src.u3.ip, ntohs(t->src.u.all),
		&t->dst.u3.ip, ntohs(t->dst.u.all),
		dir ? "DIR":"REV");
 return buf;
}

NDPI_STATIC void packet_trace(const struct sk_buff *skb,const struct nf_conn * ct, char *msg) {
  const struct iphdr *iph = ip_hdr(skb);
  if(iph && iph->version == 4) {
	if(iph->protocol == IPPROTO_TCP || iph->protocol == IPPROTO_UDP) {
		 struct udphdr *udph = (struct udphdr *)(((const u_int8_t *) iph) + iph->ihl * 4); 
		 pr_info("%s skb %p ct %p proto %d %pI4:%d -> %pI4:%d len %d\n",
			msg ? msg:"",(void *)skb,(void *)ct,
			iph->protocol,&iph->saddr,htons(udph->source),
			&iph->daddr,htons(udph->dest),skb->len);
  	} else
		 pr_info("%s skb %p ct %p proto %d %pI4 -> %pI4 len %d\n",
			msg ? msg:"",(void *)skb,(void *)ct,
			iph->protocol,&iph->saddr, &iph->daddr,skb->len);
  }
}

NDPI_STATIC int check_known_ipv4_service( struct ndpi_net *n,
		union nf_inet_addr *ipaddr, uint16_t port, uint8_t protocol) {

	prefix_t ipx;
	patricia_node_t *node;
	uint16_t app_protocol = NDPI_PROTOCOL_UNKNOWN;
	fill_prefix_any(&ipx,ipaddr,AF_INET);

	spin_lock_bh (&n->ipq_lock);
	node = ndpi_patricia_search_best(n->ndpi_struct->protocols_ptree,&ipx);
	if(node) {
	    if(protocol == IPPROTO_UDP || protocol == IPPROTO_TCP)
		app_protocol = ndpi_check_ipport(node,port,protocol == IPPROTO_TCP);
	}
	spin_unlock_bh (&n->ipq_lock);
	return app_protocol;
}

NDPI_STATIC u32
ndpi_process_packet(struct ndpi_net *n, struct nf_conn * ct, struct nf_ct_ext_ndpi *ct_ndpi,
		    const uint64_t time,
                    const struct sk_buff *skb,int dir)
{
	ndpi_protocol proto = NDPI_PROTOCOL_NULL;
        struct ndpi_id_struct *src, *dst;
        struct ndpi_flow_struct * flow;
	uint32_t low_ip, up_ip, tmp_ip;
	uint16_t low_port, up_port, tmp_port, protocol;
	const struct iphdr *iph = NULL;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	const struct ipv6hdr *ip6h;

	ip6h = ipv6_hdr(skb);
	if(ip6h && ip6h->version != 6) ip6h = NULL;
#endif
	iph = ip_hdr(skb);

	if(iph && iph->version != 4) iph = NULL;

	if(!iph
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		&& !ip6h
#endif
	  ) {
		COUNTER(ndpi_pf);
		return NDPI_PROCESS_ERROR;
	}

	flow = ct_ndpi->flow;
	if (!flow) {
		flow = ndpi_alloc_flow(ct_ndpi);
		if (!flow) {
			COUNTER(ndpi_pg);
			return NDPI_PROCESS_ERROR;
		}
	}

	src = ct_ndpi->src;
	if (!src) {
		src = ndpi_id_search_or_insert (n,
			&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3);
		if (!src) {
			COUNTER(ndpi_ph);
			return NDPI_PROCESS_ERROR;
		}
		ct_ndpi->src = src;
	}
	dst = ct_ndpi->dst;
	if (!dst) {
		dst = ndpi_id_search_or_insert (n,
			&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3);
		if (!dst) {
			COUNTER(ndpi_ph);
			return NDPI_PROCESS_ERROR;
		}
		ct_ndpi->dst = dst;
	}

	/* here the actual detection is performed */
	if(dir) {
		src = ct_ndpi->dst;
		dst = ct_ndpi->src;
	}

	flow->packet_direction = dir;
	if(ndpi_log_debug > 1)
		packet_trace(skb,ct,"process    ");
	proto = ndpi_detection_process_packet(n->ndpi_struct,flow,
#ifdef NDPI_DETECTION_SUPPORT_IPV6
				ip6h ?	(uint8_t *) ip6h :
#endif
					(uint8_t *) iph, 
					 skb->len, time, src, dst);

	if(proto.master_protocol == NDPI_PROTOCOL_UNKNOWN && 
	          proto.app_protocol == NDPI_PROTOCOL_UNKNOWN ) {
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	    if(ip6h) {
		low_ip = 0;
		up_ip = 0;
		protocol = ip6h->nexthdr;
	    } else
#endif
	    {
		low_ip=ntohl(iph->saddr);
		up_ip=ntohl(iph->daddr);
		protocol = iph->protocol;
	    }

	    if(protocol == IPPROTO_TCP || protocol == IPPROTO_UDP) {
		low_port = htons(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.tcp.port);
		up_port  = htons(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.tcp.port);
	    } else {
		low_port = up_port = 0;
	    }
	    if (iph && flow && flow->packet_counter < 3 &&
			!flow->protocol_id_already_guessed) {
		proto.app_protocol = check_known_ipv4_service(n,
				&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3,up_port,protocol);
		if(proto.app_protocol != NDPI_PROTOCOL_UNKNOWN)
			proto.app_protocol = check_known_ipv4_service(n,
				&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3,low_port,protocol);
		if(proto.app_protocol != NDPI_PROTOCOL_UNKNOWN)
			flow->protocol_id_already_guessed = 1;
	    }
	    if(proto.app_protocol == NDPI_PROTOCOL_UNKNOWN) {
		if(low_ip > up_ip) { tmp_ip = low_ip; low_ip=up_ip; up_ip = tmp_ip; }
		if(low_port > up_port) { tmp_port = low_port; low_port=up_port; up_port = tmp_port; }
		proto = ndpi_guess_undetected_protocol (
				n->ndpi_struct,flow,protocol,low_ip,low_port,up_ip,up_port);
	    }
	} else {
		add_stat(flow->packet.parsed_lines);
	}
	if( proto.app_protocol != NDPI_PROTOCOL_UNKNOWN ||
	    proto.master_protocol != NDPI_PROTOCOL_UNKNOWN) {
		ct_ndpi->proto.app_protocol = proto.app_protocol;
		ct_ndpi->proto.master_protocol = proto.master_protocol;
	}

	return proto.app_protocol;
}
NDPI_STATIC inline int can_handle(const struct sk_buff *skb,uint8_t *l4_proto)
{
	const struct iphdr *iph;
	uint32_t l4_len;
	uint8_t proto;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	const struct ipv6hdr *ip6h;

	ip6h = ipv6_hdr(skb);
	if(ip6h->version == 6) {
		COUNTER(ndpi_pa);
		*l4_proto = ip6h->nexthdr;
		// FIXME!
		return 1;
	}
#endif
	iph = ip_hdr(skb);
        if(!iph) { /* not IP */
		COUNTER(ndpi_pb); return 0;
	}
	if(iph->version != 4) {
		COUNTER(ndpi_pb); return 0;
	}
	*l4_proto = proto = iph->protocol;
	COUNTER(ndpi_p0);

	if(ntohs(iph->frag_off) & 0x3fff) {
		COUNTER(ndpi_p1); return 0;
	}
	if(skb->len <= (iph->ihl << 2)) {
		COUNTER(ndpi_p1); return 0; 
	}

	l4_len = skb->len - (iph->ihl << 2);
        if(proto == IPPROTO_TCP) {
		if(l4_len < sizeof(struct tcphdr)) {
			COUNTER(ndpi_p2); return 0;
		}
		return 1;
	}
        if(proto == IPPROTO_UDP) {
		if(l4_len < sizeof(struct udphdr)) {
			COUNTER(ndpi_p2); return 0;
		}
		return 1;
	}
	COUNTER(ndpi_p7);
	return 1;
}

NDPI_STATIC void ndpi_host_ssl(struct nf_ct_ext_ndpi *ct_ndpi) {

    if(ct_ndpi->proto.app_protocol == NDPI_PROTOCOL_UNKNOWN &&
	ct_ndpi->proto.master_protocol == NDPI_PROTOCOL_UNKNOWN ) return;
    if(!ct_ndpi->flow) return;

    if(!ct_ndpi->host) {
	const char *name = ct_ndpi->flow->host_server_name;
	if(*name)
		ct_ndpi->host = kstrndup(name, 
				sizeof(ct_ndpi->flow->host_server_name)-1, GFP_ATOMIC);
    }

    if(!ct_ndpi->ssl && (
		ct_ndpi->proto.app_protocol == NDPI_PROTOCOL_SSL ||
		ct_ndpi->proto.master_protocol == NDPI_PROTOCOL_SSL)) {
    	const char *name_s = ct_ndpi->flow->protos.stun_ssl.ssl.server_certificate;
    	const char *name_c = ct_ndpi->flow->protos.stun_ssl.ssl.client_certificate;
	const size_t s_len = sizeof(ct_ndpi->flow->protos.stun_ssl.ssl.server_certificate);
	const size_t c_len = sizeof(ct_ndpi->flow->protos.stun_ssl.ssl.client_certificate);
	if(*name_s && *name_c) {
		ct_ndpi->ssl = strchr(name_s,'*') ?
			kstrndup(name_c, c_len, GFP_ATOMIC):
			kstrndup(name_s, s_len, GFP_ATOMIC);
	} else if(*name_s) {
		ct_ndpi->ssl = kstrndup(name_s, s_len, GFP_ATOMIC);
	} else if(*name_c) {
	 	ct_ndpi->ssl = kstrndup(name_c, c_len, GFP_ATOMIC);
	}
    }
}

NDPI_STATIC bool ndpi_host_match( const struct xt_ndpi_mtinfo *info,
			     struct nf_ct_ext_ndpi *ct_ndpi) {
bool res = false;

if(!info->hostname[0]) return true;

ndpi_host_ssl(ct_ndpi);

do {
  if(info->host) {
	if(ct_ndpi->host) {
		res = info->re ? ndpi_regexec(info->reg_data,ct_ndpi->host) != 0 :
			strstr(ct_ndpi->host,info->hostname) != NULL;
		if(res) break;
	}
  }

  if(info->ssl && ( ct_ndpi->proto.app_protocol == NDPI_PROTOCOL_SSL ||
		    ct_ndpi->proto.master_protocol == NDPI_PROTOCOL_SSL )) {
	if(ct_ndpi->ssl) {
		res = info->re ? ndpi_regexec(info->reg_data,ct_ndpi->ssl) != 0 :
			strstr(ct_ndpi->ssl,info->hostname) != NULL;
		if(res) break;
	}
  }
} while(0);

if(ndpi_log_debug > 2)
    pr_info("%s: match%s %s %s '%s' %s,%s %d\n", __func__,
	info->re ? "-re":"", info->host ? "host":"", info->ssl ? "ssl":"",
	info->hostname,ct_ndpi->host ? ct_ndpi->host:"-",
	ct_ndpi->ssl ? ct_ndpi->ssl:"-",res);

return res;
}

NDPI_STATIC inline uint16_t get_in_if(const struct net_device *dev) {

	return dev ? dev->ifindex:0;
}


#define NDPI_ID 0x44504900ul

#define pack_proto(proto) ((proto.app_protocol << 16) | proto.master_protocol)

NDPI_STATIC bool
ndpi_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	uint32_t r_proto;
	ndpi_protocol_nf proto = NDPI_PROTOCOL_NULL;
	uint64_t time;
	struct timespec tm;
	const struct xt_ndpi_mtinfo *info = par->matchinfo;

	enum ip_conntrack_info ctinfo;
	struct nf_conn * ct = NULL;
	struct sk_buff *linearized_skb = NULL;
	const struct sk_buff *skb_use = NULL;
	struct nf_ct_ext_ndpi *ct_ndpi = NULL;
	struct ndpi_cb *c_proto;
	uint8_t l4_proto=0;
	bool result=false, host_match = true, is_ipv6=false;
	struct ndpi_net *n;

	char ct_buf[128];

#ifdef NDPI_DETECTION_SUPPORT_IPV6
	const struct ipv6hdr *ip6h;

	ip6h = ipv6_hdr(skb);
	is_ipv6 = ip6h && ip6h->version == 6;
#endif
	n = ndpi_pernet(xt_net(par));
	
	if(!atomic_read(&n->ndpi_ready)) return 0;

	if(!read_trylock(&n->ndpi_busy)) {
		/* ndpi_net_exit started */
		return 0;
	}

	proto.app_protocol = NDPI_PROCESS_ERROR;

	c_proto = skb_get_cproto(skb);

    do {
	if(c_proto->magic == NDPI_ID &&
	   c_proto->proto == NDPI_PROCESS_ERROR) {
		break;
	}
	if(!can_handle(skb,&l4_proto)) {
		proto.app_protocol = NDPI_PROTOCOL_UNKNOWN;
		break;
	}
	if( skb->len > ndpi_mtu && skb_is_nonlinear(skb) ) {
		COUNTER(ndpi_jumbo);
		break;
	}

	if(c_proto->magic != NDPI_ID)
		ct_proto_set_flow(c_proto,NULL,0);

	COUNTER(ndpi_pk);

	getnstimeofday(&tm);

	ct = nf_ct_get (skb, &ctinfo);
	if (ct == NULL) {
		COUNTER(ndpi_p31);
		break;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
	if (nf_ct_is_untracked(ct))
#else
	if(ctinfo == IP_CT_UNTRACKED)	
#endif
	{
		COUNTER(ndpi_p31);
		break;
	}

	{
	    struct nf_ct_ext_labels *ct_label = nf_ct_ext_find_label(ct);
#ifdef NF_CT_CUSTOM
	    if(!ct_label) {
		if(nf_ct_is_confirmed(ct)) {
			COUNTER(ndpi_p31);
			break;
		}
		ct_label = nf_ct_ext_add_ndpi(ct);
		if(ct_label)
			ct_label->magic = 0;

	    }
#endif
	    if(ct_label) {
		if(!ct_label->magic) {
			ct_ndpi = kmem_cache_zalloc (ct_info_cache, GFP_ATOMIC);
			if(ct_ndpi) {
				WRITE_ONCE(ct_label->ndpi_ext, ct_ndpi);
				WRITE_ONCE(ct_label->magic, MAGIC_CT);
				ndpi_init_ct_struct(n,ct_ndpi,l4_proto,ct,is_ipv6,tm.tv_sec);
				if(ndpi_log_debug > 2)
					pr_info("Create  ct_ndpi %p ct %p %s\n",
						(void *)ct_ndpi, (void *)ct, ct_info(ct,ct_buf,sizeof(ct_buf),0));
			}
		} else {
			if(ct_label->magic == MAGIC_CT) {
				ct_ndpi = ct_label->ndpi_ext;
				if(ndpi_log_debug > 2)
					pr_info("Reuse   ct_ndpi %p ct %p %s\n",
						(void *)ct_ndpi, (void *)ct, ct_info(ct,ct_buf,sizeof(ct_buf),0));
			  } else
				COUNTER(ndpi_p34);
		}
	    } else 
		COUNTER(ndpi_p31);
	}

	if(!ct_ndpi) {
		COUNTER(ndpi_p31);
		break;
	}
	if(ndpi_log_debug > 2)
		pr_info("START match ct_ndpi %p ct %p %s\n",
			(void *)ct_ndpi, (void *)ct, ct_info(ct,ct_buf,sizeof(ct_buf),0));

	proto.app_protocol = NDPI_PROTOCOL_UNKNOWN;
	if(ndpi_log_debug > 3)
		packet_trace(skb,ct,"Start      ");

	barrier();
	spin_lock_bh (&ct_ndpi->lock);

	if( test_flow_yes(ct_ndpi) && 
	    !test_nat_done(ct_ndpi) && !ct_proto_get_flow_nat(c_proto))
		ct_proto_set_flow_nat(c_proto,FLOW_NAT_START);

#if defined(CONFIG_NF_CONNTRACK_MARK)
	if(ct->mark && ct->mark != ct_ndpi->flinfo.connmark)
		ct_ndpi->flinfo.connmark = ct->mark;
#endif

#ifdef USE_HACK_USERID
	if(!test_userid(ct_ndpi) &&
	    (skb->userid & 0xffff000000000000ull)
                        == 0xdead000000000000ull) {
        	ct_ndpi->flinfo.ip_snat  = skb->userid & 0xffffffff;
		ct_ndpi->flinfo.sport_nat = htons((skb->userid >> 32) & 0xffff);
		set_userid(ct_ndpi);
		set_nat_done(ct_ndpi);
	}
#endif
	if( c_proto->magic == NDPI_ID ) {
	    if(ct_proto_last(c_proto) == ct) {
		proto.app_protocol = ct_ndpi->proto.app_protocol;
		proto.master_protocol = ct_ndpi->proto.master_protocol;
		if(info->hostname[0])
			host_match = ndpi_host_match(info,ct_ndpi);

		spin_unlock_bh (&ct_ndpi->lock);
		COUNTER(ndpi_pi);
		if(ndpi_log_debug > 1)
		    packet_trace(skb,ct,"cache      ");
		break;
	    } else {
		if(ct_proto_last(c_proto))
		    	COUNTER(ndpi_pi3);
	    }
	} else 
		COUNTER(ndpi_pi4); // new packet

	if(!ct_ndpi->flinfo.ifidx) 
		ct_ndpi->flinfo.ifidx = get_in_if(xt_in(par));
	if(!ct_ndpi->flinfo.ofidx) 
		ct_ndpi->flinfo.ofidx = get_in_if(xt_out(par));
	
	/* don't pass icmp for TCP/UDP to ndpi_process_packet()  */
	if(l4_proto == IPPROTO_ICMP && ct_ndpi->l4_proto != IPPROTO_ICMP) {
		proto.master_protocol = NDPI_PROTOCOL_IP_ICMP;
		proto.app_protocol = NDPI_PROTOCOL_IP_ICMP;
		spin_unlock_bh (&ct_ndpi->lock);
		COUNTER(ndpi_pj);
		ndpi_pjc += skb->len;
		break;
	}
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	if(l4_proto == IPPROTO_ICMPV6 && ct_ndpi->l4_proto != IPPROTO_ICMPV6) {
		proto.master_protocol = NDPI_PROTOCOL_IP_ICMPV6;
		proto.app_protocol = NDPI_PROTOCOL_IP_ICMPV6;
		spin_unlock_bh (&ct_ndpi->lock);
		COUNTER(ndpi_pj);
		ndpi_pjc += skb->len;
		break;
	}
#endif
	if(ndpi_enable_flow && c_proto->magic != NDPI_ID ) {
		ndpi_ct_counters_add(ct_ndpi,1,skb->len, ctinfo, tm.tv_sec);
	}

	if(test_detect_done(ct_ndpi)) {
		proto = ct_ndpi->proto;
		c_proto->magic = NDPI_ID;
		c_proto->proto = pack_proto(proto);

		ct_proto_set_flow(c_proto,ct, !test_flow_yes(ct_ndpi) ? 0:
			(test_nat_done(ct_ndpi) ? FLOW_NAT_END:FLOW_NAT_START));

		if(info->hostname[0])
			host_match = ndpi_host_match(info,ct_ndpi);

		spin_unlock_bh (&ct_ndpi->lock);
		if(ndpi_log_debug > 1)
			packet_trace(skb,ct,"detect_done ");
		break;
	}
	if(ct_ndpi->proto.app_protocol == NDPI_PROTOCOL_UNKNOWN ||
	    ct_ndpi->flow) {
		struct ndpi_net *n;

		if (skb_is_nonlinear(skb)) {
			linearized_skb = skb_copy(skb, GFP_ATOMIC);
			if (linearized_skb == NULL) {
				spin_unlock_bh (&ct_ndpi->lock);
				COUNTER(ndpi_falloc);
				proto.app_protocol = NDPI_PROCESS_ERROR;
				break;
			}
			skb_use = linearized_skb;
			ndpi_nskb += 1;
		} else {
			skb_use = skb;
			ndpi_lskb += 1;
		}

		time = ((uint64_t)tm.tv_sec) * detection_tick_resolution +
			(uint32_t)tm.tv_nsec / (1000000000ul / detection_tick_resolution);

		n = ndpi_pernet(nf_ct_net(ct));
		r_proto = ndpi_process_packet(n, ct,
				ct_ndpi, time, skb_use,
				CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL);

		c_proto->magic = NDPI_ID;
		c_proto->proto = r_proto;

		ct_proto_set_flow(c_proto,ct, !test_flow_yes(ct_ndpi) ? 0:
			(test_nat_done(ct_ndpi) ? FLOW_NAT_END:FLOW_NAT_START));

		COUNTER(ndpi_pd);

		if(r_proto == NDPI_PROCESS_ERROR) {
			// special case for errors
			COUNTER(ndpi_pe);
			c_proto->proto = r_proto;
			proto.app_protocol = r_proto;
			proto.master_protocol = NDPI_PROTOCOL_UNKNOWN;
			if(ct_ndpi->proto.app_protocol == NDPI_PROTOCOL_UNKNOWN)
				ct_ndpi->proto.app_protocol = r_proto;

		} else { // ndpi_process_packet return app_protocol 
			if(r_proto != NDPI_PROTOCOL_UNKNOWN) {
				proto = ct_ndpi->proto;
				c_proto->proto = pack_proto(proto);
				if(test_flow_yes(ct_ndpi))
					ndpi_host_ssl(ct_ndpi);
				atomic_inc(&n->protocols_cnt[proto.app_protocol]);
				if(proto.master_protocol != NDPI_PROTOCOL_UNKNOWN)
					atomic_inc(&n->protocols_cnt[proto.master_protocol]);
			} else { // unknown
				if(ct_ndpi->proto.app_protocol != NDPI_PROTOCOL_UNKNOWN &&
				   ct_ndpi->flow->no_cache_protocol) { // restore proto
					proto = ct_ndpi->proto;
					c_proto->proto = pack_proto(proto);
				} else {
					switch(ct_ndpi->l4_proto) {
					  case IPPROTO_TCP:
						  if(ct_ndpi->flow->packet_counter > max_packet_unk_tcp)
							  set_detect_done(ct_ndpi);
						  break;
					  case IPPROTO_UDP:
						  if(ct_ndpi->flow->packet_counter > max_packet_unk_udp)
							  set_detect_done(ct_ndpi);
						  break;
					  default:
						  if(ct_ndpi->flow->packet_counter > max_packet_unk_other)
							  set_detect_done(ct_ndpi);
					}
					if(test_detect_done(ct_ndpi) && ct_ndpi->flow)
						__ndpi_free_ct_flow(ct_ndpi);
				}
			}
			if(info->hostname[0])
				host_match = ndpi_host_match(info,ct_ndpi);
		}
		spin_unlock_bh (&ct_ndpi->lock);

		if(linearized_skb != NULL)
			kfree_skb(linearized_skb);
	}
    } while(0);

    read_unlock(&n->ndpi_busy);

    if (info->error)
	return (proto.app_protocol == NDPI_PROCESS_ERROR) ^ (info->invert != 0);

    do {
	if (info->have_master) {
		result = proto.master_protocol != NDPI_PROTOCOL_UNKNOWN;
		break;
	}
	if(info->empty) {
		result = true;
		break;
	}
	if (info->m_proto && !info->p_proto) {
		result = NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.master_protocol) != 0;
		break;
	}

	if (!info->m_proto && info->p_proto) {
		result = NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.app_protocol) != 0 ;
		break;
	}

	if (proto.app_protocol != NDPI_PROTOCOL_UNKNOWN) {
		result = NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.app_protocol) != 0;
		if(proto.master_protocol !=  NDPI_PROTOCOL_UNKNOWN)
			result |= NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.master_protocol) != 0;
		break;
	}
	result = NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.master_protocol) != 0;
    } while(0);
    if(ndpi_log_debug > 1)
		packet_trace(skb,ct,"detect_done ");
    return ( result & host_match ) ^ (info->invert != 0);
}


NDPI_STATIC int
ndpi_mt_check(const struct xt_mtchk_param *par)
{
struct xt_ndpi_mtinfo *info = par->matchinfo;

	if (!info->error &&  !info->have_master && !info->hostname[0] &&
	     NDPI_BITMASK_IS_ZERO(info->flags)) {
		pr_info("No selected protocols.\n");
		return -EINVAL;
	}
	info->empty = NDPI_BITMASK_IS_ZERO(info->flags);
	if(info->hostname[0] && info->re) {
		char re_buf[sizeof(info->hostname)];
		int re_len = strlen(info->hostname);
		if(re_len < 3 || info->hostname[0] != '/' || 
				info->hostname[re_len-1] != '/') {
			pr_info("Invalid REGEXP\n");
			return -EINVAL;
		}
		re_len -= 2;
		strncpy(re_buf,&info->hostname[1],re_len);
		re_buf[re_len] = '\0';
		info->reg_data = ndpi_regcomp(re_buf,&re_len);
		if(!info->reg_data) {
			pr_info("regcomp failed\n");
			return -EINVAL;
		}
		if(ndpi_log_debug > 2)
			pr_info("regcomp '%s' success\n",re_buf);
	} else {
		info->reg_data = NULL;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
	{
		int ret;

		ret = nf_ct_netns_get(par->net, par->family);
		if (ret < 0) {
			pr_info("cannot load conntrack support for proto=%u\n",
				par->family);
			return ret;
		}
	}
#endif
	ndpi_enable_protocols (ndpi_pernet(par->net));
	return 0;
}

NDPI_STATIC void 
ndpi_mt_destroy (const struct xt_mtdtor_param *par)
{
struct xt_ndpi_mtinfo *info = par->matchinfo;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
	nf_ct_netns_put(par->net, par->family);
#endif
	if(info->reg_data) kfree(info->reg_data);
}

#ifdef NF_CT_CUSTOM

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,8,0)

NDPI_STATIC char *ndpi_proto_to_str(char *buf,size_t size,ndpi_protocol *p,ndpi_mod_str_t *ndpi_str)
{
const char *t_app,*t_mast;
buf[0] = '\0';
t_app = ndpi_get_proto_by_id(ndpi_str,p->app_protocol);
t_mast= ndpi_get_proto_by_id(ndpi_str,p->master_protocol);
if(p->app_protocol && t_app)
	strncpy(buf,t_app,size);
if(p->master_protocol && t_mast) {
	strncat(buf,",",size);
	strncat(buf,t_mast,size);
}
return buf;
}
NDPI_STATIC unsigned int seq_print_ndpi(struct seq_file *s,
					  const struct nf_conn *ct,
					  int dir)
{

       struct nf_ct_ext_ndpi *ct_ndpi;
       char res_str[64];
       ndpi_mod_str_t *ndpi_str;
       if(dir != IP_CT_DIR_REPLY) return 0;
	
       ct_ndpi = nf_ct_ext_find_ndpi(ct);
       ndpi_str = ndpi_pernet(nf_ct_net(ct))->ndpi_struct;
       if(ct_ndpi && (ct_ndpi->proto.app_protocol || ct_ndpi->proto.master_protocol))
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0)
	    seq_printf(s,"ndpi=%s ",ndpi_proto_to_str(res_str,sizeof(res_str),&ct_ndpi->proto,ndpi_str));
#else
	    return seq_printf(s,"ndpi=%s ",
			    ndpi_proto_to_str(res_str,sizeof(res_str),&ct_ndpi->proto,ndpi_str));
#endif
       return 0;
}
#endif
#endif

NDPI_STATIC u_int32_t ndpi_proto_markmask(struct ndpi_net *n, u_int32_t var,
		const ndpi_protocol *proto, int mode, const struct xt_ndpi_tginfo *info)
{
    if(mode && ( proto->master_protocol >= NDPI_NUM_BITS ||
		 proto->app_protocol >= NDPI_NUM_BITS)) return var;

    switch (mode) {
     case 0:
	var &= ~info->mask;
	var |=  info->mark;
	break;
     case 1:
	var &= ~n->mark[proto->master_protocol].mask;
	var |=  n->mark[proto->master_protocol].mark;
	break;
     case 2:
	var &= ~n->mark[proto->app_protocol].mask;
	var |=  n->mark[proto->app_protocol].mark;
	break;
     case 3:
	if(proto->app_protocol != NDPI_PROTOCOL_UNKNOWN) {
	    var &= ~n->mark[proto->app_protocol].mask;
	    var |=  n->mark[proto->app_protocol].mark;
	} else
	  if(proto->master_protocol != NDPI_PROTOCOL_UNKNOWN) {
		var &= ~n->mark[proto->master_protocol].mask;
		var |=  n->mark[proto->master_protocol].mark;
	  }
	break;
     }
    return var;
}

NDPI_STATIC unsigned int
ndpi_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_ndpi_tginfo *info = par->targinfo;
	ndpi_protocol proto = NDPI_PROTOCOL_NULL;
	struct ndpi_net *n = ndpi_pernet(xt_net(par));
	struct ndpi_cb *c_proto;
	int mode = 0;

	if(!atomic_read(&n->ndpi_ready)) return XT_CONTINUE; // ndpi_net_init() not completed!
	if(!read_trylock(&n->ndpi_busy)) return XT_CONTINUE; // ndpi_net_init() not completed!

	c_proto = skb_get_cproto(skb);

	if(c_proto->magic != NDPI_ID) {
		read_unlock(&n->ndpi_busy);
		if(ndpi_log_debug > 1)
			pr_info("%s: no ndpi magic\n",__func__);
		return XT_CONTINUE;
	}

	if(ndpi_enable_flow && info->flow_yes) {
	    if(ndpi_log_debug > 2)
		pr_info("%s:%s flow_yes flow_nat %u\n",
			__func__,n->ns_name,ct_proto_get_flow_nat(c_proto));
	    do {
		enum ip_conntrack_info ctinfo;
		struct nf_conn * ct = NULL;
		struct nf_ct_ext_ndpi *ct_ndpi;
		char ct_buf[128];

//		if(ct_proto_get_flow_nat(c_proto)) break;
		ct = nf_ct_get (skb, &ctinfo);
		if(!ct) break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
		if (nf_ct_is_untracked(ct)) break;
#else
		if(ctinfo == IP_CT_UNTRACKED) break;
#endif
		ct_ndpi = nf_ct_ext_find_ndpi(ct);
		if(!ct_ndpi) break;
		if(ndpi_log_debug > 2)
			pr_info("START target ct_ndpi %p ct %p %s\n",
				(void *)ct_ndpi, (void *)ct, ct_info(ct,ct_buf,sizeof(ct_buf),0));

		spin_lock_bh (&ct_ndpi->lock);

		if(!test_flow_yes(ct_ndpi))
		    ndpi_ct_list_add(n,ct_ndpi);

		if(!test_nat_done(ct_ndpi) && 
		   !ct_proto_get_flow_nat(c_proto)) {
			ct_proto_set_flow_nat(c_proto,FLOW_NAT_START);
		}
		spin_unlock_bh (&ct_ndpi->lock);
	    } while(0);
	}
	read_unlock(&n->ndpi_busy);

	if(c_proto->proto != NDPI_PROCESS_ERROR) {
		uint32_t tmp_p = READ_ONCE(c_proto->proto);
		/* see pack_proto() */
		proto.master_protocol = tmp_p & 0xffff;
		proto.app_protocol = (tmp_p >> 16) & 0xffff; 
	}

	if(info->t_mark || info->t_clsf) {
		if(info->m_proto_id) mode |= 1;
		if(info->p_proto_id) mode |= 2;
		if(info->any_proto_id) mode |= 3;
	}
	if(info->t_mark)
		skb->mark = ndpi_proto_markmask(n,skb->mark,&proto,mode,info);

	if(info->t_clsf)
		skb->priority =	ndpi_proto_markmask(n,skb->priority,&proto,mode,info);

 return info->t_accept ? NF_ACCEPT : XT_CONTINUE;
}

NDPI_STATIC int
ndpi_tg_check(const struct xt_tgchk_param *par)
{
	const struct xt_ndpi_tginfo *info = par->targinfo;
	if(info->flow_yes && !ndpi_enable_flow)
		return -EINVAL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
	{
		int ret;

		ret = nf_ct_netns_get(par->net, par->family);
		if (ret < 0) {
			pr_info("cannot load conntrack support for proto=%u\n",
				par->family);
			return ret;
		}
	}
#endif
        ndpi_enable_protocols (ndpi_pernet(par->net));
	return nf_ct_l3proto_try_module_get (par->family);
}

NDPI_STATIC void 
ndpi_tg_destroy (const struct xt_tgdtor_param *par)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
	nf_ct_netns_put(par->net, par->family);
#endif
	nf_ct_l3proto_module_put (par->family);
}


NDPI_STATIC struct xt_match
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

NDPI_STATIC struct xt_target ndpi_tg_reg __read_mostly = {
        .name           = "NDPI",
        .revision       = 0,
#ifdef NDPI_DETECTION_SUPPORT_IPV6
        .family         = NFPROTO_UNSPEC,
#else
	.family		= NFPROTO_IPV4,
#endif
        .target         = ndpi_tg,
	.checkentry	= ndpi_tg_check,
	.destroy	= ndpi_tg_destroy,
        .targetsize     = sizeof(struct xt_ndpi_tginfo),
        .me             = THIS_MODULE,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
NDPI_STATIC void bt_port_gc(struct timer_list *t) {
	struct ndpi_net *n = from_timer(n, t, gc);
#else
NDPI_STATIC void bt_port_gc(unsigned long data) {
        struct ndpi_net *n = (struct ndpi_net *)data;
#endif
        struct ndpi_detection_module_struct *ndpi_struct = n->ndpi_struct;
	struct timespec tm;
	int i;
	uint32_t st_j;
	uint32_t en_j;
	
	if(!atomic_read(&n->ndpi_ready)) return; // ndpi_net_init() not completed!
	if(!read_trylock(&n->ndpi_busy)) return; // ndpi_net_exit() started!

	st_j = READ_ONCE(jiffies);
	getnstimeofday(&tm);
	{
	    struct hash_ip4p_table *ht = READ_ONCE(ndpi_struct->bt_ht);
	    if(ht) {
		/* full period 64 seconds */
		for(i=0; i < ht->size/128;i++) {
		    if(n->gc_index < 0 ) n->gc_index = 0;
		    if(n->gc_index >= ht->size-1) n->gc_index = 0;

		    if(ht->tbl[n->gc_index].len)
			n->gc_count += ndpi_bittorrent_gc(ht,n->gc_index,tm.tv_sec);
		    n->gc_index++;
		}
	    }
	}
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	{
	    struct hash_ip4p_table *ht6 = READ_ONCE(ndpi_struct->bt6_ht);
	    if(ht6) {
		for(i=0; i < ht6->size/128;i++) {
		    if(n->gc_index6 < 0 ) n->gc_index6 = 0;
		    if(n->gc_index6 >= ht6->size-1) n->gc_index6 = 0;

		    if(ht6->tbl[n->gc_index6].len)
			n->gc_count += ndpi_bittorrent_gc(ht6,n->gc_index6,tm.tv_sec);
		    n->gc_index6++;
		}
	    }
	}
#endif
	ndpi_bt_gc = n->gc_count;
	en_j = READ_ONCE(jiffies);
	barrier();
	if(en_j > st_j+1 && 0) { 
		pr_info("%s: BT jiffies %u\n",__func__,en_j - st_j);
		st_j = en_j;
	}

	if(ndpi_enable_flow) {

	    if(atomic_read(&n->acc_rem) > n->acc_limit) {
		n->acc_gc = ndpi_delete_acct(n,2) < 0 ?
			jiffies + HZ/5 : jiffies + HZ;
	    } else {
		if(!mutex_is_locked(&n->rem_lock)) {
		    if(time_after(jiffies,n->acc_gc)) {
			if( atomic_read(&n->acc_work) > 0 || 
			    atomic_read(&n->acc_rem)  > 0 )
				ndpi_delete_acct(n,1);
		    }
		    n->acc_gc = jiffies + 5*HZ;
		}
	    }
	}
	en_j = READ_ONCE(jiffies);
	barrier();

	if(en_j > st_j+1 && flow_read_debug) 
		pr_info("%s: FLOW jiffies %u\n",__func__,en_j - st_j);

	read_unlock(&n->ndpi_busy);
	mod_timer(&n->gc,jiffies + HZ/2);
}

int inet_ntop_port(int family,void *ip, u_int16_t port, char *lbuf, size_t bufsize) {
return  family == AF_INET6 ?
		snprintf(lbuf,bufsize-1, "%pI6c.%d",ip, htons(port))
	      :	snprintf(lbuf,bufsize-1, "%pI4n:%d",ip,htons(port));
}

NDPI_STATIC int ninfo_proc_open(struct inode *inode, struct file *file)
{
        return 0;
}

/*
 * all:
 *  1 - delete only for_deleted if reader inactived.
 *  2 - delete only for_deleted, add lost counters if not empty.
 *  3 - remove all ( remove xt_ndpi оr netns )
 */

int ndpi_delete_acct(struct ndpi_net *n,int all) {
	struct nf_ct_ext_ndpi *ct_ndpi,*next,*prev,*flow_h;
	int i2 = 0, del,skip_del;

	if(!ndpi_enable_flow) return 0;

	if(!mutex_trylock(&n->rem_lock)) return -1;

	if(!atomic_read(&n->ndpi_ready)) all = 3;

	skip_del = all != 2 ? 0 : n->acc_limit*3/4;

	if(flow_read_debug) pr_info("%s:%s all=%d rem %d skip_del %d\n",
			__func__,n->ns_name,all,atomic_read(&n->acc_rem),skip_del);

  restart:
	next = prev = NULL;
	smp_rmb();
	ct_ndpi = READ_ONCE(n->flow_h);
	flow_h = ct_ndpi;
	while(ct_ndpi) {
		barrier();
		next = READ_ONCE(ct_ndpi->next);
		if(!spin_trylock_bh(&ct_ndpi->lock)) {
			prev = ct_ndpi;
			ct_ndpi = next;
//			pr_info("%s: skip busy ct %px\n",__func__,prev);
			continue;
		}
		barrier();
		del = 0;
		switch(all) {
		case 1: del = test_for_delete(ct_ndpi);
			break;
		case 2: if(test_for_delete(ct_ndpi)) {
			    if(test_flow_info(ct_ndpi) && skip_del > 0) skip_del--;
				else del = 1;
			}
			break;
		case 3: del = 1;
		}
		
		spin_unlock_bh(&ct_ndpi->lock);

		if(del) {
			if(prev) {
				if(cmpxchg(&prev->next,ct_ndpi,next) != ct_ndpi) {
					pr_err("%s: BUG! prev->next %px != ct_ndpi %px\n",__func__,
							prev->next,ct_ndpi);
					break;
				}
			} else {
				if(cmpxchg(&n->flow_h,flow_h,next) == flow_h)
					flow_h = next;
			  	else {
					pr_info("%s: restart\n",__func__);
					goto restart;
				}
			}
			if(n->flow_l == ct_ndpi) n->flow_l = next;

			if(all == 3) {
				__ndpi_free_ct_ndpi_id(n,ct_ndpi);
				__ndpi_free_ct_flow(ct_ndpi);
			}
			__ndpi_free_ct_proto(ct_ndpi);
			if(all == 2 && test_flow_info(ct_ndpi)) {
				// count lost info
				atomic_add(ct_ndpi->flinfo.p[0]-ct_ndpi->flinfo.p[2],
						&n->acc_i_packets_lost);
				atomic_add(ct_ndpi->flinfo.p[1]-ct_ndpi->flinfo.p[3],
						&n->acc_o_packets_lost);
				atomic64_add(ct_ndpi->flinfo.b[0]-ct_ndpi->flinfo.b[2],
						&n->acc_i_bytes_lost);
				atomic64_add(ct_ndpi->flinfo.b[1]-ct_ndpi->flinfo.b[3],
						&n->acc_o_bytes_lost);
			}
			atomic_dec(&n->acc_work);
			atomic_dec(&n->acc_rem);
			kmem_cache_free (ct_info_cache, ct_ndpi);
			i2++;
			if(all < 3 && (atomic_read(&n->acc_rem) <= 0)) break;
		} else {
			prev = ct_ndpi;
		}
		ct_ndpi=next;

	}

	mutex_unlock(&n->rem_lock);
	if( (all > 1 || i2) && flow_read_debug)
		pr_info("%s:%s Delete %d flows. Active %d, rem %d\n",__func__,n->ns_name,
			i2, atomic_read(&n->acc_work), atomic_read(&n->acc_rem));

	return i2;
}

NDPI_STATIC void ndpi_ct_counter_save(struct nf_ct_ext_ndpi *ct) {
	clear_flow_info(ct);
	ct->flinfo.b[2] = ct->flinfo.b[0];
	ct->flinfo.b[3] = ct->flinfo.b[1];
	ct->flinfo.p[2] = ct->flinfo.p[0];
	ct->flinfo.p[3] = ct->flinfo.p[1];
}

static inline int nflow_str_data(struct ndpi_net *n) {
	int rl = n->str_buf_len;
	if(!rl)  return 0;

	rl -= n->str_buf_offs;
	return rl > 0;
}

static int nflow_put_str(struct ndpi_net *n, char __user *buf,
	int *p, size_t *count, loff_t *ppos) {

	int ro,rl;

	if(!n->str_buf_len)  return 0;

	rl = n->str_buf_len - n->str_buf_offs;
	ro = 0;
	if(rl > *count) {
		ro = rl - *count;
		rl = *count;
	}
	if(!rl) return 1;

	if (!(ACCESS_OK(VERIFY_WRITE, buf+(*p), rl) &&
	      !__copy_to_user(buf+(*p), &n->str_buf[n->str_buf_offs], rl))) {
			n->str_buf_len = 0;
			n->str_buf_offs = 0;
			return -1;
	}
	if(ro) {
		n->str_buf_offs += rl;
	} else {
		n->str_buf_len = 0;
		n->str_buf_offs = 0;
	}
	*count -= rl;
	*p += rl;
	(*ppos) += rl;
	return ro;
}

ssize_t nflow_read(struct ndpi_net *n, char __user *buf,
                              size_t count, loff_t *ppos)
{
	struct nf_ct_ext_ndpi *ct_ndpi,*next,*prev,*flow_h;
	int p,del,r;
	ssize_t sl;
	int cnt_view=0,cnt_del=0,cnt_out=0;
	loff_t st_pos;

	if(!ndpi_enable_flow) return -EINVAL;

	if(n->acc_end) return 0;

	p = 0;
	r = 0;
	if(nflow_str_data(n)) {
		r = nflow_put_str(n,buf,&p,&count,ppos);
		if (r != 0) {
			return r < 0 ? -EINVAL : p;
		}
	}

	if(*ppos == 0) {
		if(flow_read_debug)
		  pr_info("%s:%s Start dump: CT total %d deleted %d\n",
			__func__, n->ns_name, atomic_read(&n->acc_work),atomic_read(&n->acc_rem));
		sl = n->acc_read_mode > 3 ?
			ndpi_dump_start_rec(n->str_buf,sizeof(n->str_buf),n->acc_open_time):
			snprintf(n->str_buf,sizeof(n->str_buf)-1,"TIME %lu\n",n->acc_open_time);

		n->str_buf_len = sl; n->str_buf_offs = 0;
		r = nflow_put_str(n,buf,&p,&count,ppos);

		if (r != 0) {
			return r < 0 ? -EFAULT : p;
		}

		if(atomic_read(&n->acc_i_packets_lost) ||
		   atomic_read(&n->acc_o_packets_lost)) {
			uint32_t cpi = atomic_xchg(&n->acc_i_packets_lost,0);
			uint32_t cpo = atomic_xchg(&n->acc_o_packets_lost,0);
			uint64_t cbi = atomic64_xchg(&n->acc_i_bytes_lost,0);
			uint64_t cbo = atomic64_xchg(&n->acc_o_bytes_lost,0);
			sl = n->acc_read_mode > 3 ? 
				ndpi_dump_lost_rec(n->str_buf,sizeof(n->str_buf),cpi,cpo,cbi,cbo) :
				snprintf(n->str_buf,sizeof(n->str_buf)-1,
					"LOST_TRAFFIC %llu %llu %u %u\n",cbi,cbo,cpi,cpo);

			n->str_buf_len = sl; n->str_buf_offs = 0;
			r = nflow_put_str(n,buf,&p,&count,ppos);

			if (r != 0) {
				return r < 0 ? -EFAULT : p;
			}
		}
	}
	st_pos = *ppos;
	prev = NULL;

    restart:
	if(!n->flow_l) {
		ct_ndpi = READ_ONCE(n->flow_h);
	} else {
		prev = n->flow_l;
		ct_ndpi = prev->next;
	}
	flow_h = ct_ndpi;
	while(ct_ndpi) {

		spin_lock_bh(&ct_ndpi->lock);
		cnt_view++;
		next = ct_ndpi->next;
		del  = test_for_delete(ct_ndpi);

		n->str_buf_len = 0; n->str_buf_offs = 0;

		switch(n->acc_read_mode & 0x3) {
		case 0:
			sl = flow_have_info(ct_ndpi) ?
				ndpi_dump_acct_info(n,ct_ndpi) : 0;
			break;
		case 1:
			sl = del && flow_have_info(ct_ndpi) ?
				ndpi_dump_acct_info(n,ct_ndpi) : 0;
			break;
		case 2:
			sl = !del && test_flow_yes(ct_ndpi) ?
				ndpi_dump_acct_info(n,ct_ndpi) : 0;
			break;
		default:
			sl = 0;
		}
		if(sl && (n->acc_read_mode & 0x3) != 2) {
				ndpi_ct_counter_save(ct_ndpi);
		}
		if(del) {
			if(prev) {
			    if(cmpxchg(&prev->next,ct_ndpi,next) != ct_ndpi) {
					pr_err("%s: BUG! prev->next %px != ct_ndpi %px\n",__func__,
							prev->next,ct_ndpi);
				spin_unlock_bh(&ct_ndpi->lock);
				n->flow_l = NULL;
				n->acc_end = 1;
				n->str_buf_len = 0;
				n->str_buf_offs = 0;
				return -EINVAL;
			    }
			} else {
			    if(cmpxchg(&n->flow_h,flow_h,next) == flow_h)
					flow_h = next;
			    	else {
					spin_unlock_bh(&ct_ndpi->lock);
					pr_info("%s: reread!\n",__func__);
					goto restart;
				}
			}
		}
		spin_unlock_bh(&ct_ndpi->lock);

		if(del) {
			__ndpi_free_ct_proto(ct_ndpi);
			kmem_cache_free (ct_info_cache, ct_ndpi);
			atomic_dec(&n->acc_work);
			atomic_dec(&n->acc_rem);
			cnt_del++;
		} else {
			prev = ct_ndpi;
			n->flow_l = prev;
		}
		ct_ndpi=next;
		if(sl) {
			r = nflow_put_str(n,buf,&p,&count,ppos);
			if(r != 0) {
				n->flow_l = prev;
				if(r < 0) p = -EFAULT;
				break;
			}
			cnt_out++;
		}
	}

	n->cnt_view += cnt_view;
	n->cnt_del  += cnt_del;
	n->cnt_out  += cnt_out;

	if(!ct_ndpi ) {
		n->acc_end = 1;
		n->flow_l = NULL;
		if(flow_read_debug)
		  pr_info("%s:%s End   dump: CT total %d deleted %d\n",
			__func__, n->ns_name, atomic_read(&n->acc_work),atomic_read(&n->acc_rem));
	} else if(flow_read_debug > 1) {
			pr_info("%s:%s pos %7lld view %d dumped %d deleted %d\n",
				__func__, n->ns_name, st_pos, cnt_view, cnt_out, cnt_del);
		}

	n->acc_gc = jiffies + n->acc_wait * HZ;
	
	return p;
}

NDPI_STATIC const char *__acerr2txt[] = {
    [ACERR_SUCCESS] = "OK", /* No error occurred */
    [ACERR_DUPLICATE_PATTERN] = "ERR:DUP", /* Duplicate patterns */
    [ACERR_LONG_PATTERN] = "ERR:LONG", /* Pattern length is longer than AC_PATTRN_MAX_LENGTH */
    [ACERR_ZERO_PATTERN] = "ERR:EMPTY" , /* Empty pattern (zero length) */
    [ACERR_AUTOMATA_CLOSED] = "ERR:CLOSED", /* Automata is closed. */
    [ACERR_ERROR] = "ERROR" /* common error */
};

const char *acerr2txt(AC_ERROR_t r) {
	return r >= ACERR_SUCCESS && r <= ACERR_ERROR ? __acerr2txt[r]:"UNKNOWN";
}


NDPI_STATIC const struct file_operations nproto_proc_fops = {
        .open    = ninfo_proc_open,
        .read    = nproto_proc_read,
        .write   = nproto_proc_write,
	.llseek  = noop_llseek,
	.release = nproto_proc_close
};

NDPI_STATIC const struct file_operations ninfo_proc_fops = {
        .open    = ninfo_proc_open,
        .read    = ninfo_proc_read,
        .write   = ninfo_proc_write,
	.llseek  = noop_llseek,
};

NDPI_STATIC const struct file_operations nflow_proc_fops = {
        .open    = nflow_proc_open,
        .read    = nflow_proc_read,
        .write   = nflow_proc_write,
	.llseek  = nflow_proc_llseek,
	.release = nflow_proc_close
};

#ifdef NDPI_DETECTION_SUPPORT_IPV6
NDPI_STATIC const struct file_operations ninfo6_proc_fops = {
        .open    = ninfo_proc_open,
        .read    = ninfo6_proc_read,
        .write   = ninfo_proc_write,
	.llseek  = noop_llseek,
};
#endif

#ifdef BT_ANNOUNCE
NDPI_STATIC const struct file_operations nann_proc_fops = {
        .open    = ninfo_proc_open,
        .read    = nann_proc_read,
	.llseek  = noop_llseek,
};
#endif

NDPI_STATIC const struct file_operations n_ipdef_proc_fops = {
        .open    = n_ipdef_proc_open,
        .read    = n_ipdef_proc_read,
        .write   = n_ipdef_proc_write,
	.llseek  = noop_llseek,
        .release = n_ipdef_proc_close,
};

NDPI_STATIC const struct file_operations n_hostdef_proc_fops = {
        .open    = n_hostdef_proc_open,
        .read    = n_hostdef_proc_read,
        .write   = n_hostdef_proc_write,
        .llseek  = noop_llseek,
        .release = n_hostdef_proc_close,
};

#if  LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
NDPI_STATIC unsigned int ndpi_nat_do_chain(unsigned int hooknum,
                                         struct sk_buff *skb,
                                         const struct net_device *in,
                                         const struct net_device *out,
                                         int (*okfn)(struct sk_buff *))
{
#elif  LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
NDPI_STATIC unsigned int ndpi_nat_do_chain(const struct nf_hook_ops *ops,
                                         struct sk_buff *skb,
                                         const struct net_device *in,
                                         const struct net_device *out,
                                         int (*okfn)(struct sk_buff *))
{
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0)
NDPI_STATIC unsigned int ndpi_nat_do_chain(const struct nf_hook_ops *priv,
                                         struct sk_buff *skb,
                                         const struct nf_hook_state *state)
{
#else
NDPI_STATIC unsigned int ndpi_nat_do_chain(void *priv,
					 struct sk_buff *skb,
					 const struct nf_hook_state *state)
{
#endif
    struct nf_conn * ct = NULL;
    enum ip_conntrack_info ctinfo;
    struct nf_ct_ext_ndpi *ct_ndpi;
    struct ndpi_cb *c_proto;

    do {

	c_proto = skb_get_cproto(skb);

	if(c_proto->magic != NDPI_ID) break;
	if(c_proto->proto == NDPI_PROCESS_ERROR) break;
	if(ct_proto_get_flow_nat(c_proto) != FLOW_NAT_START) break;

	ct = nf_ct_get (skb, &ctinfo);
	if (ct == NULL) break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
	if (nf_ct_is_untracked(ct)) break;
#else
	if(ctinfo == IP_CT_UNTRACKED) break;
#endif
	if( !(ct_ndpi = nf_ct_ext_find_ndpi(ct)) ) break;
	spin_lock_bh (&ct_ndpi->lock);
	if(!test_nat_done(ct_ndpi)) {
		ndpi_nat_detect(ct_ndpi,ct);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
		if(hooknum != NF_INET_PRE_ROUTING)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
		if(ops->hooknum != NF_INET_PRE_ROUTING)
#else
		if(state->hook != NF_INET_PRE_ROUTING)
#endif
		{
			set_nat_done(ct_ndpi);
			ct_proto_set_flow_nat(c_proto,FLOW_NAT_END);
		}
	}
	spin_unlock_bh (&ct_ndpi->lock);
    } while(0);

    return NF_ACCEPT;
}

NDPI_STATIC struct nf_hook_ops nf_nat_ipv4_ops[] = {
	{
		.hook		= ndpi_nat_do_chain,
		.pf		= NFPROTO_IPV4,
		.hooknum	= NF_INET_PRE_ROUTING,
		.priority	= NF_IP_PRI_NAT_DST + 1,
	},
	{
		.hook		= ndpi_nat_do_chain,
		.pf		= NFPROTO_IPV4,
		.hooknum	= NF_INET_POST_ROUTING,
		.priority	= NF_IP_PRI_NAT_SRC + 1,
	},
	{
		.hook		= ndpi_nat_do_chain,
		.pf		= NFPROTO_IPV4,
		.hooknum	= NF_INET_LOCAL_IN,
		.priority	= NF_IP_PRI_NAT_SRC + 1,
	}
};

NDPI_STATIC void __net_exit ndpi_net_exit(struct net *net)
{
	struct rb_node * next;
	struct osdpi_id_node *id;
	struct ndpi_net *n;

	n = ndpi_pernet(net);
	pr_info("%s:%s\n",__func__,n->ns_name);

	atomic_set(&n->ndpi_ready,0);

	write_lock(&n->ndpi_busy);
	/* ndpi library code not busy */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	del_timer(&n->gc);
#else
	del_timer_sync(&n->gc);
#endif
	write_unlock(&n->ndpi_busy);

	if(ndpi_enable_flow) {
	    nf_unregister_net_hooks(net, nf_nat_ipv4_ops, 
			      ARRAY_SIZE(nf_nat_ipv4_ops));
	}


#ifndef NF_CT_CUSTOM
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
	net->ct.label_words = n->labels_word;
#endif
	net->ct.labels_used--;
#endif

#if   LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
	nf_ct_iterate_cleanup_net(net, __ndpi_free_flow, n, 0 ,0);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0)
	nf_ct_iterate_cleanup(net, __ndpi_free_flow, n, 0 ,0);
#else /* < 3.12 */
	nf_ct_iterate_cleanup(net, __ndpi_free_flow, n);
#endif

	if(ndpi_enable_flow) {
		while(ndpi_delete_acct(n,3) == -1)
			msleep_interruptible(1);
	}

	/* free all objects before destroying caches */
	
	next = rb_first(&n->osdpi_id_root);
	while (next) {
		id = rb_entry(next, struct osdpi_id_node, node);
		next = rb_next(&id->node);
		rb_erase(&id->node, &n->osdpi_id_root);
		kmem_cache_free (osdpi_id_cache, id);
	}
	
	str_hosts_done(n->hosts);
	
	ndpi_exit_detection_module(n->ndpi_struct);

	if(n->pde) {
		if(n->pe_ipdef)
			remove_proc_entry(ipdef_name, n->pde);
		if(n->pe_hostdef)
			remove_proc_entry(hostdef_name, n->pde);
		if(n->pe_info)
			remove_proc_entry(info_name, n->pde);
		if(n->pe_proto)
			remove_proc_entry(proto_name, n->pde);
		if(n->pe_flow)
			remove_proc_entry(flow_name, n->pde);
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

NDPI_STATIC int __net_init ndpi_net_init(struct net *net)
{
	struct ndpi_net *n;
	int i;

	/* init global detection structure */

	n = ndpi_pernet(net);
	snprintf(n->ns_name,sizeof(n->ns_name)-1,"ns%d",net_ns_id);

	rwlock_init(&n->ndpi_busy);
	atomic_set(&n->ndpi_ready,0);

	spin_lock_init(&n->id_lock);
	spin_lock_init(&n->ipq_lock);
	spin_lock_init(&n->w_buff_lock);
	mutex_init(&n->host_lock);
	mutex_init(&n->rem_lock);
	atomic_set(&n->acc_work,0);
	atomic_set(&n->acc_rem,0);
	n->acc_limit = ndpi_flow_limit;
	n->w_buff[W_BUF_IP] = NULL;
	n->w_buff[W_BUF_HOST] = NULL;
	n->w_buff[W_BUF_PROTO] = NULL;

	n->host_ac = NULL;
	n->hosts = str_hosts_alloc();
	n->hosts_tmp = NULL;
	n->host_error = 0;

	parse_ndpi_proto(n,"init");
       	n->osdpi_id_root = RB_ROOT;

	/* init global detection structure */
	n->ndpi_struct = ndpi_init_detection_module();
	if (n->ndpi_struct == NULL) {
		pr_err("xt_ndpi: global structure initialization failed.\n");
                return -ENOMEM;
	}
	n->flow_h = NULL;
	n->ndpi_struct->direction_detect_disable = 1;
	/* disable all protocols */
	NDPI_BITMASK_RESET(n->protocols_bitmask);
	ndpi_set_protocol_detection_bitmask2(n->ndpi_struct, &n->protocols_bitmask);

	n->ndpi_struct->user_data = n;
	for (i = 0; i < NDPI_NUM_BITS; i++) {
                atomic_set (&n->protocols_cnt[i], 0);
        	n->debug_level[i] = 0;
		if(i <= NDPI_LAST_IMPLEMENTED_PROTOCOL) continue;
		n->mark[i].mark = n->mark[i].mask = 0;
        }
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
	pr_info("ndpi_lib_trace %s\n",ndpi_lib_trace ? "Enabled":"Disabled");
	n->ndpi_struct->ndpi_log_level = ndpi_lib_trace;
	set_ndpi_debug_function(n->ndpi_struct, ndpi_lib_trace ? debug_printf:NULL);
#endif

	if(bt_hash_size > 512) bt_hash_size = 512;
	if(bt6_hash_size > 32) bt6_hash_size = 32;
#ifdef BT_ANNOUNCE
	if(bt_log_size > 512) bt_log_size = 512;
	if(bt_log_size < 32 ) bt_log_size = 0;
#else
	bt_log_size = 0;
#endif
	ndpi_bittorrent_init(n->ndpi_struct,
			bt_hash_size*1024,bt6_hash_size*1024,
			bt_hash_tmo,bt_log_size);

	n->n_hash = -1;

	/* Create proc files */
	
	n->pde = proc_mkdir(dir_name, net->proc_net);
	if(!n->pde) {
		ndpi_exit_detection_module(n->ndpi_struct);
		pr_err("xt_ndpi: cant create net/%s\n",dir_name);
		return -ENOMEM;
	}
	do {
		ndpi_protocol_match *hm;
		char *cstr;
		int i2;

		n->pe_info = NULL;
		n->pe_flow = NULL;
		n->pe_proto = NULL;
#ifdef BT_ANNOUNCE
		n->pe_ann = NULL;
#endif
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		n->pe_info6 = NULL;
#endif
		n->pe_ipdef = NULL;
		n->pe_hostdef = NULL;

		n->pe_info = proc_create_data(info_name, S_IRUGO | S_IWUSR,
					 n->pde, &ninfo_proc_fops, n);
		if(!n->pe_info) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,info_name);
			break;
		}
		n->pe_flow = proc_create_data(flow_name, S_IRUGO | S_IWUSR,
					 n->pde, &nflow_proc_fops, n);
		if(!n->pe_flow) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,flow_name);
			break;
		}
		n->pe_proto = proc_create_data(proto_name, S_IRUGO | S_IWUSR,
					 n->pde, &nproto_proc_fops, n);
		if(!n->pe_proto) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,proto_name);
			break;
		}
#ifdef BT_ANNOUNCE
		n->pe_ann = proc_create_data(ann_name, S_IRUGO,
					 n->pde, &nann_proc_fops, n);
		if(!n->pe_ann) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,ann_name);
			break;
		}

#endif
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		n->pe_info6 = proc_create_data(info6_name, S_IRUGO | S_IWUSR,
					 n->pde, &ninfo6_proc_fops, n);
		if(!n->pe_info6) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,info6_name);
			break;
		}

#endif
		n->pe_ipdef = proc_create_data(ipdef_name, S_IRUGO | S_IWUSR,
					 n->pde, &n_ipdef_proc_fops, n);
		if(!n->pe_ipdef) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,ipdef_name);
			break;
		}

		n->pe_hostdef = proc_create_data(hostdef_name, S_IRUGO | S_IWUSR,
					 n->pde, &n_hostdef_proc_fops, n);
		if(!n->pe_hostdef) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,hostdef_name);
			break;
		}
		n->host_ac = ndpi_init_automa();
		if(!n->host_ac) {
			pr_err("xt_ndpi: cant alloc host_ac\n");
			break;
		}
		for(hm = host_match; hm->string_to_match ; hm++) {
			size_t sml;
			ndpi_protocol_match_result s_ret;
			i = hm->protocol_id;
			if(i >= NDPI_NUM_BITS) {
				pr_err("xt_ndpi: bad proto num %d \n",i);
				continue;
			}
			sml = strlen(hm->string_to_match);
			i2 = ndpi_match_string_subprotocol(n->ndpi_struct,
								hm->string_to_match,sml,&s_ret,1);
			if(i2 == NDPI_PROTOCOL_UNKNOWN || i != i2) {
				pr_err("xt_ndpi: Warning! Hostdef '%s' %s! proto_id %u != %u, p:%u. Skipping.\n",
						i != i2 ? "missmatch":"unknown",
						hm->string_to_match,i,i2,s_ret.protocol_id
						);
				continue;
			}
			if(str_collect_look(n->hosts->p[i],hm->string_to_match,sml) >= 0) {
				pr_err("xt_ndpi: Warning! Hostdef '%s' duplicated! Skipping.\n",
						hm->string_to_match);
				continue;
			}
			cstr = str_collect_add(&n->hosts->p[i],hm->string_to_match,sml);
			if(!cstr) {
				hm = NULL; // error
				break;
			}
			{
				AC_ERROR_t r;
				AC_PATTERN_t ac_pattern;
				ac_pattern.astring    = cstr;
				ac_pattern.length     = sml;
				ac_pattern.rep.number = i;
				r = ac_automata_add(n->host_ac, &ac_pattern);
				if(r != ACERR_SUCCESS) {
					str_collect_del(n->hosts_tmp->p[i],cstr,sml);
					if(r != ACERR_DUPLICATE_PATTERN) {
						pr_info("xt_ndpi: add host '%s' proto %x error: %s\n",
							hm->string_to_match,i,acerr2txt(r));
						hm = NULL; // error
						break;
					}
					if(ac_pattern.rep.number != i) {
						pr_info("xt_ndpi: Host '%s' proto %x already defined as %s\n",
							hm->string_to_match,i, 
							ndpi_get_proto_by_id(n->ndpi_struct,
								     ac_pattern.rep.number));
					}
				}
			}
		}
		if(hm) {
			ac_automata_finalize((AC_AUTOMATA_t*)n->host_ac);
			XCHGP(n->ndpi_struct->host_automa.ac_automa,n->host_ac);
			ac_automata_release(n->host_ac);
			n->host_ac = NULL;
		} else break;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
		init_timer(&n->gc);
		n->gc.data = (unsigned long)n;
		n->gc.function = bt_port_gc;
		n->gc.expires = jiffies + HZ/2;
		add_timer(&n->gc);
#else
		timer_setup(&n->gc, bt_port_gc, 0);
		mod_timer(&n->gc, jiffies + HZ/2);
#endif
		n->acc_gc = jiffies;
#ifndef NF_CT_CUSTOM
		/* hack!!! */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
		n->labels_word = ACCESS_ONCE(net->ct.label_words);
		net->ct.label_words = 2;
#endif
		net->ct.labels_used++;
#endif
		if( ndpi_enable_flow && 
		    nf_register_net_hooks(net, nf_nat_ipv4_ops,
	                                   ARRAY_SIZE(nf_nat_ipv4_ops))) break;
		/* All success! */
		atomic_set(&n->ndpi_ready,1);
		pr_info("%s:%s OK\n",__func__,n->ns_name);
		net_ns_id++;
		return 0;
	} while(0);

/* rollback procfs on error */
	str_hosts_done(n->hosts);

	if(n->pe_hostdef)
		remove_proc_entry(hostdef_name,n->pde);
	if(n->pe_ipdef)
		remove_proc_entry(ipdef_name,n->pde);
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	if(n->pe_info6)
		remove_proc_entry(proto_name, n->pde);
#endif
#ifdef BT_ANNOUNCE
	if(n->pe_ann)
		remove_proc_entry(ann_name, n->pde);
#endif
	if(n->pe_proto)
		remove_proc_entry(proto_name,n->pde);
	if(n->pe_info)
		remove_proc_entry(info_name,n->pde);
	if(n->pe_flow)
		remove_proc_entry(flow_name,n->pde);

	PROC_REMOVE(n->pde,net);
	ndpi_exit_detection_module(n->ndpi_struct);

	return -ENOMEM;
}

#ifndef NF_CT_CUSTOM
NDPI_STATIC void replace_nf_destroy(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	void (*destroy)(struct nf_conntrack *);
	rcu_read_lock();
	destroy = rcu_dereference(nf_ct_destroy);
	BUG_ON(destroy == NULL);
	rcu_assign_pointer(ndpi_nf_ct_destroy,destroy);
        RCU_INIT_POINTER(nf_ct_destroy, ndpi_destroy_conntrack);
	rcu_read_unlock();
#else
	struct nf_ct_hook *hook;
	rcu_read_lock();
	hook = rcu_dereference(nf_ct_hook);
	BUG_ON(hook == NULL);
	rcu_assign_pointer(ndpi_nf_ct_destroy,hook->destroy);
	/* This is a hellish hack! */
	hook->destroy = ndpi_destroy_conntrack;
	rcu_read_unlock();

#endif
}

NDPI_STATIC void restore_nf_destroy(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	void (*destroy)(struct nf_conntrack *);
	rcu_read_lock();
	destroy = rcu_dereference(nf_ct_destroy);
	BUG_ON(destroy != ndpi_destroy_conntrack);
	destroy = rcu_dereference(ndpi_nf_ct_destroy);
	BUG_ON(destroy == NULL);
	rcu_assign_pointer(nf_ct_destroy,destroy);
	rcu_read_unlock();
#else
	struct nf_ct_hook *hook;
	rcu_read_lock();
	hook = rcu_dereference(nf_ct_hook);
	BUG_ON(hook == NULL);
	BUG_ON(hook->destroy != ndpi_destroy_conntrack);
	/* This is a hellish hack! */
	hook->destroy = rcu_dereference(ndpi_nf_ct_destroy);
	rcu_assign_pointer(ndpi_nf_ct_destroy,NULL);
	rcu_read_unlock();
#endif
}
#else
NDPI_STATIC struct nf_ct_ext_type ndpi_extend = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,8,0)
       .seq_print = seq_print_ndpi,
#endif
       .destroy   = nf_ndpi_free_flow,
       .len    = sizeof(struct nf_ct_ext_labels),
       .align  = __alignof__(uint32_t),
};
#endif

NDPI_STATIC struct pernet_operations ndpi_net_ops = {
        .init   = ndpi_net_init,
        .exit   = ndpi_net_exit,
        .id     = &ndpi_net_id,
        .size   = sizeof(struct ndpi_net),
};

NDPI_STATIC int __init ndpi_mt_init(void)
{
        int ret;

	ndpi_size_id_struct = sizeof(struct osdpi_id_node);
	ndpi_size_flow_struct = ndpi_detection_get_sizeof_ndpi_flow_struct();
	detection_tick_resolution = HZ;
	set_ndpi_ticks_per_second(detection_tick_resolution);
	set_ndpi_malloc(malloc_wrapper);
	set_ndpi_free(free_wrapper);
	
	if(request_module("nf_conntrack") < 0) {
		pr_err("xt_ndpi: nf_conntrack required!\n");
		return -EOPNOTSUPP;
	}
	if(request_module("ip_tables") < 0) {
		pr_err("xt_ndpi: ip_tables required!\n");
		return -EOPNOTSUPP;
	}
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	if(request_module("ip6_tables") < 0) {
		pr_err("xt_ndpi: ip6_tables required!\n");
		return -EOPNOTSUPP;
	}
#endif
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
        ct_info_cache = kmem_cache_create("ndpi_ctinfo", 
			ALIGN(sizeof(struct nf_ct_ext_ndpi),__SIZEOF_LONG__),
                                             0, 0, NULL);
        if (!ct_info_cache) {
                pr_err("xt_ndpi: error creating ct_info cache.\n");
		goto unreg_target;
        }
        osdpi_flow_cache = kmem_cache_create("ndpi_flows", ndpi_size_flow_struct,
                                             0, 0, NULL);
        if (!osdpi_flow_cache) {
                pr_err("xt_ndpi: error creating flow cache.\n");
		goto free_ctinfo;
        }
        
        osdpi_id_cache = kmem_cache_create("ndpi_ids",
                                           ndpi_size_id_struct,
                                           0, 0, NULL);
        if (!osdpi_id_cache) {
		pr_err("xt_ndpi: error creating id cache.\n");
		goto free_flow;
	}

	ndpi_size_hash_ip4p_node=                sizeof(struct hash_ip4p_node)
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		                                 +12
#endif
	;

        bt_port_cache = kmem_cache_create("ndpi_btport",
				ndpi_size_hash_ip4p_node, 0, 0, NULL);
        if (!bt_port_cache) {
		pr_err("xt_ndpi: error creating port cache.\n");
		goto free_id;
	}
	if(bt_hash_size && bt_hash_size > 512) bt_hash_size = 512;
	if(bt6_hash_size && bt6_hash_size > 32) bt6_hash_size = 32;
	if(!bt_hash_tmo || bt_hash_tmo < 900) bt_hash_tmo = 900;
	if( bt_hash_tmo > 3600) bt_hash_tmo = 3600;

#ifndef NF_CT_CUSTOM
	replace_nf_destroy();
#endif
	pr_info("xt_ndpi v1.2 ndpi %s"
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		" IPv6=YES"
#else
		" IPv6=no"
#endif
#ifdef USE_HACK_USERID
		" USERID=YES"
#endif
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
		" debug_message=YES"
#else
		" debug_message=no"
#endif
		"\n BT: hash_size %luk, hash_expiation %ld sec, log_size %ldkb\n"
		" sizeof hash_ip4p_node=%lu id_struct=%lu PATRICIA_MAXBITS=%zu\n"
		" flow_struct=%lu packet_struct=%zu\n"
		"   flow_tcp_struct=%zu flow_udp_struct=%zu int_one_line_struct=%zu\n"
		" ndpi_ip_addr_t=%zu ndpi_protocol=%zu nf_ct_ext_ndpi=%zu\n"
		" flow_info=%zu spinlock_t=%zu\n"
#ifndef NF_CT_CUSTOM
		" NF_LABEL_ID %d\n",
#else
		" NF_EXT_ID %d\n",
#endif
		NDPI_GIT_RELEASE,
		bt_hash_size, bt_hash_size ? bt_hash_tmo : 0, bt_log_size, 
		ndpi_size_hash_ip4p_node, ndpi_size_id_struct, (size_t)PATRICIA_MAXBITS,
		ndpi_size_flow_struct,
		sizeof(struct ndpi_packet_struct),
		sizeof(struct ndpi_flow_tcp_struct),
		sizeof(struct ndpi_flow_udp_struct),
		sizeof(struct ndpi_int_one_line_struct),
		sizeof(ndpi_ip_addr_t),
		sizeof(ndpi_protocol),
		ALIGN(sizeof(struct nf_ct_ext_ndpi),__SIZEOF_LONG__),
		sizeof(struct flow_info),
		sizeof(spinlock_t),
		nf_ct_ext_id_ndpi);
	pr_info("xt_ndpi: MAX_PROTOCOLS %d LAST_PROTOCOL %d\n",
		NDPI_NUM_BITS,
		NDPI_LAST_IMPLEMENTED_PROTOCOL);
	pr_info("xt_ndpi: flow acctounting %s\n",ndpi_enable_flow ? "ON":"OFF"); 

	return 0;

free_id:
       	kmem_cache_destroy (osdpi_id_cache);
free_flow:
       	kmem_cache_destroy (osdpi_flow_cache);
free_ctinfo:
       	kmem_cache_destroy (ct_info_cache);
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


NDPI_STATIC void __exit ndpi_mt_exit(void)
{
	xt_unregister_target(&ndpi_tg_reg);
	xt_unregister_match(&ndpi_mt_reg);
	unregister_pernet_subsys(&ndpi_net_ops);
#ifdef NF_CT_CUSTOM
	nf_ct_extend_unregister(&ndpi_extend);
#else
	restore_nf_destroy();
#endif
        kmem_cache_destroy (bt_port_cache);
        kmem_cache_destroy (osdpi_id_cache);
        kmem_cache_destroy (osdpi_flow_cache);
        kmem_cache_destroy (ct_info_cache);
}


module_init(ndpi_mt_init);
module_exit(ndpi_mt_exit);

#include "ndpi_strcol.c" 
#include "ndpi_proc_parsers.c" 
#include "ndpi_proc_generic.c"
#include "ndpi_proc_info.c"
#include "ndpi_proc_flow.c" 
#include "ndpi_proc_hostdef.c"
#include "ndpi_proc_ipdef.c"
#include "../libre/regexp.c"
#include "../../src/lib/ndpi_main.c"