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
#include <asm/percpu.h>

#ifndef CONFIG_NF_CONNTRACK_CUSTOM
#define CONFIG_NF_CONNTRACK_CUSTOM 0
#endif


#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>
#include <net/netfilter/nf_nat.h>
#include <linux/ktime.h>
#include <linux/rcupdate.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
#define IP_CT_UNTRACKED IP_CT_NUMBER
#endif

#define BT_ANNOUNCE

#include "ndpi_config.h"
#undef HAVE_HYPERSCAN
#include "ndpi_api.h"
#include "ndpi_private.h"

#include "xt_ndpi.h"

#include "../lib/third_party/include/ndpi_patricia.h"

NDPI_STATIC ndpi_protocol_match host_match[];
NDPI_STATIC ndpi_debug_function_ptr ndpi_debug_print_init = NULL;
NDPI_STATIC ndpi_log_level_t ndpi_debug_level_init = NDPI_LOG_ERROR;


/* Only for debug! */
//#define NDPI_IPPORT_DEBUG


#define COUNTER(a) atomic_inc((atomic_t *)&a)
#define COUNTER_D(a) atomic_dec((atomic_t *)&a)

#define NDPI_PROCESS_ERROR (NDPI_NUM_BITS+1)
#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF    89
#endif

static char dir_name[]="xt_ndpi";
static char info_name[]="info";
static char ipdef_name[]="ip_proto";
static char ip6def_name[]="ip6_proto";
static char hostdef_name[]="host_proto";
static char flow_name[]="flows";
#ifdef NDPI_DETECTION_SUPPORT_IPV6
static char info6_name[]="info6";
#endif
#ifdef BT_ANNOUNCE
static char ann_name[]="announce";
#endif

static char proto_name[]="proto";
static char debug_name[]="debug";
static char risk_name[]="risks";
static char cfg_name[]="cfg";

#ifdef CONFIG_NF_CONNTRACK_DESTROY_HOOK
#define USE_NF_CONNTRACK_DESTROY_HOOK
#elif LINUX_VERSION_CODE > KERNEL_VERSION(5,19,0)
#ifndef USE_LIVEPATCH
#define USE_LIVEPATCH
#endif
#endif

#ifdef USE_LIVEPATCH
#if IS_ENABLED(CONFIG_LIVEPATCH)
#include <linux/livepatch.h>
#include <linux/rculist_nulls.h>

typedef void (*ndpi_conntrack_destroy_ptr) (struct nf_conntrack *);
ndpi_conntrack_destroy_ptr __rcu nf_conntrack_destroy_cb;
#else
#error "CONFIG_LIVEPATCH not enabled"
#endif
#endif

#define PROC_REMOVE(pde,net) proc_remove(pde)

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

#if !defined(USE_CONNLABELS) && !defined(USE_NF_CONNTRACK_DESTROY_HOOK) && defined(CONFIG_NF_CONNTRACK_CUSTOM) && CONFIG_NF_CONNTRACK_CUSTOM > 0
#define NF_CT_CUSTOM
#else
#ifndef USE_NF_CONNTRACK_DESTROY_HOOK
#undef NF_CT_CUSTOM
#include <net/netfilter/nf_conntrack_labels.h>
#ifndef CONFIG_NF_CONNTRACK_LABELS
#error NF_CONNTRACK_LABELS not defined
#endif
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

#define NDPI_ID 0x44504900ul
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

// static struct nf_ct_ext_ndpi dummy_struct1;
// static struct flow_info dummy_struct2;

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
#define FLOW_FREE_NORM 0
#define FLOW_FREE_FORCE 1
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

static ndpi_protocol_nf proto_null = {NDPI_PROTOCOL_UNKNOWN , NDPI_PROTOCOL_UNKNOWN};

static unsigned long int ndpi_flow_limit=10000000; // 4.3Gb
static unsigned long int ndpi_enable_flow=0;

static char ndpi_flow_opt[NDPI_FLOW_OPT_MAX+1]="";

static unsigned long int ndpi_log_debug=0;
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
static unsigned long  ndpi_lib_trace=0;
#endif
static unsigned long  ndpi_mtu=48000;
static unsigned long  bt_log_size=128;
static unsigned long int bt_hash_size=0;
static unsigned long int bt6_hash_size=0;
static unsigned long int bt_hash_tmo=1200;
static unsigned long int tls_buf_size=8;
static unsigned long int ndpi_stun_cache_opt=0;

static unsigned long  max_packet_unk_tcp=20;
static unsigned long  max_packet_unk_udp=20;
static unsigned long  max_packet_unk_other=20;

static unsigned long  flow_read_debug=0;

static unsigned long  ndpi_size_flow_struct=0;
static unsigned long  ndpi_size_hash_ip4p_node=0;

static unsigned long  ndpi_jumbo=0;
static unsigned long  ndpi_falloc=0;
static unsigned long  ndpi_nskb=0;
static unsigned long  ndpi_lskb=0;
static unsigned long  ndpi_flow_c=0;
static unsigned long  ndpi_bt_gc=0;

static unsigned long  ndpi_p_ipv4=0;
static unsigned long  ndpi_p_err_ip_frag_len=0;
static unsigned long  ndpi_p_err_bad_tcp_udp=0;
static unsigned long  ndpi_p_ct_null=0;
static unsigned long  ndpi_p_ct_untrack=0;
static unsigned long  ndpi_p_ct_confirm=0;
static unsigned long  ndpi_p_ct_nolabel=0;
static unsigned long  ndpi_p_ct_ndpi=0;
static unsigned long  ndpi_p_err_add_ndpi=0;
static unsigned long  ndpi_p_non_tcpudp=0;
static unsigned long  ndpi_p_max_parsed_lines=0;
static unsigned long  ndpi_p_ipv6=0;
static unsigned long  ndpi_p_nonip=0;
static unsigned long  ndpi_p_ndpi=0;
static unsigned long  ndpi_p_err_prot_err=0;
static unsigned long  ndpi_p_err_noiphdr=0;
static unsigned long  ndpi_p_err_alloc_flow=0;
static unsigned long  ndpi_p_err_alloc_id=0;
static unsigned long  ndpi_p_cached=0;
static unsigned long  ndpi_p_c_last_ct_not=0;
static unsigned long  ndpi_p_c_new_pkt=0;
static unsigned long  ndpi_p_c_end_max=0;
static unsigned long  ndpi_p_c_end_fail=0;
static unsigned long  ndpi_p_l4mismatch=0;
static unsigned long  ndpi_p_l4mis_size=0;
static unsigned long  ndpi_p_ndpi_match=0;
static unsigned long  ndpi_p_free_magic=0;

static unsigned long  ndpi_btp_tm[20]={0,};

module_param_named(xt_debug,   ndpi_log_debug, ulong, 0600);
MODULE_PARM_DESC(xt_debug,"Debug level for xt_ndpi (bitmap).");
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
module_param_named(lib_trace,  ndpi_lib_trace, ulong, 0600);
MODULE_PARM_DESC(lib_trace,"Debug level for nDPI library (0-off, 1-error, 2-trace, 3-debug, 4->extra debug");
#endif
module_param_named(mtu, ndpi_mtu, ulong, 0600);
MODULE_PARM_DESC(mtu,"Skip checking nonlinear skbuff larger than MTU");

module_param_named(tls_buf_size, tls_buf_size, ulong, 0600);
MODULE_PARM_DESC(tls_buf_size,"The maximum buffer size in kB for the TLS protocol. default 8, range 2-16");

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

module_param_string(ndpi_flow_opt, ndpi_flow_opt, NDPI_FLOW_OPT_MAX , 0600);
MODULE_PARM_DESC(ndpi_enable_flow,"Enable flow info option. 'S' - JA3S, 'C' - JA3C, 'c' - ja4, 'F' - tls fingerprint sha1, 'L' - level, 'R' - risks");

module_param_named(ndpi_stun_cache,ndpi_stun_cache_opt,ulong,0600);
MODULE_PARM_DESC(ndpi_stun_cache,"STUN cache control (0-1). Disabled by default.");

module_param_named(ndpi_flow_limit, ndpi_flow_limit, ulong, 0400);
MODULE_PARM_DESC(ndpi_flow_limit,"Limit netflow records. Default 10000000 (~4.3Gb RAM)");

module_param_named(max_unk_tcp,max_packet_unk_tcp,ulong, 0600);
module_param_named(max_unk_udp,max_packet_unk_udp,ulong, 0600);
module_param_named(max_unk_other,max_packet_unk_other,ulong, 0600);
module_param_named(x_flow_read_debug,flow_read_debug,ulong, 0600);

module_param_named(ndpi_size_flow_struct,ndpi_size_flow_struct,ulong, 0400);
MODULE_PARM_DESC(ndpi_size_flow_struct,"Sizeof ndpi_size_flow_struct. [info]");
module_param_named(ndpi_size_hash_ip4p_node,ndpi_size_hash_ip4p_node,ulong, 0400);
MODULE_PARM_DESC(ndpi_size_hash_ip4p_node,"Sizeof ndpi_size_hash_ip4p_node. [info]");

module_param_named(err_skb_linear, ndpi_falloc, ulong, 0400);
MODULE_PARM_DESC(err_skb_linear,"Counter of unsuccessful conversions of nonlinear packets. [error]");

module_param_named(c_ndpi_skb_seg,	 ndpi_nskb, ulong, 0400);
MODULE_PARM_DESC(c_ndpi_skb_seg,"Counter nonlinear packets. [info]");
module_param_named(c_ndpi_skb_lin,	 ndpi_lskb, ulong, 0400);
MODULE_PARM_DESC(c_ndpi_skb_lin,"Counter linear packets. [info]");
module_param_named(c_ndpi,	 ndpi_p_ndpi, ulong, 0400);

module_param_named(flow_created, ndpi_flow_c, ulong, 0400);
MODULE_PARM_DESC(flow_created,"Counter of flows. [info]");

module_param_named(bt_gc_count,  ndpi_bt_gc, ulong, 0400);

module_param_named(c_p_ipv4,         ndpi_p_ipv4, ulong, 0400);
module_param_named(c_p_ipv6,         ndpi_p_ipv6, ulong, 0400);
module_param_named(c_p_nonip,        ndpi_p_nonip, ulong, 0400);
module_param_named(max_parsed_lines, ndpi_p_max_parsed_lines, ulong, 0400);

module_param_named(c_ct_untrack,     ndpi_p_ct_untrack, ulong, 0400);
module_param_named(c_non_tcpudp,   ndpi_p_non_tcpudp, ulong, 0400);
module_param_named(c_match,	 ndpi_p_ndpi_match,  ulong, 0400);
module_param_named(c_new_pkt,	 ndpi_p_c_new_pkt, ulong, 0400);
module_param_named(c_cached,	 ndpi_p_cached,  ulong, 0400);
module_param_named(c_end_maxpkt, ndpi_p_c_end_max,  ulong, 0400);
module_param_named(c_end_fail,	 ndpi_p_c_end_fail,  ulong, 0400);

module_param_named(c_l4mismatch,   ndpi_p_l4mismatch,  ulong, 0400);
module_param_named(c_l4mis_size,   ndpi_p_l4mis_size, ulong, 0400);

module_param_named(err_oversize, ndpi_jumbo, ulong, 0400);
MODULE_PARM_DESC(err_oversize,"Counter nonlinear packets bigger than MTU. [info]");
module_param_named(err_ip_frag_len, ndpi_p_err_ip_frag_len, ulong, 0400);
module_param_named(err_bad_tcp_udp, ndpi_p_err_bad_tcp_udp, ulong, 0400);
module_param_named(err_ct_confirm, ndpi_p_ct_confirm, ulong, 0400);
module_param_named(err_ct_nolabel, ndpi_p_ct_nolabel, ulong, 0400);
module_param_named(err_ct_ndpi,    ndpi_p_ct_ndpi, ulong, 0400);
module_param_named(err_add_ndpi,   ndpi_p_err_add_ndpi, ulong, 0400);
module_param_named(err_ct_null,    ndpi_p_ct_null, ulong, 0400);
module_param_named(err_last_ct,  ndpi_p_c_last_ct_not, ulong, 0400);
module_param_named(err_prot_err, ndpi_p_err_prot_err, ulong, 0400);
module_param_named(err_noiphdr,  ndpi_p_err_noiphdr, ulong, 0400);
module_param_named(err_alloc_flow, ndpi_p_err_alloc_flow, ulong, 0400);
module_param_named(err_alloc_id,   ndpi_p_err_alloc_id, ulong, 0400);
module_param_named(err_ct_free_magic, ndpi_p_free_magic, ulong, 0400);

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

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
struct dbg_ipt_names {
	uint32_t mask;
	const char *name;
};

static struct dbg_ipt_names dbg_ipt_names [] = {
	{1 << DBG_TRACE_CT,"ct_start"},
	{1 << DBG_TRACE_PKT,"packet"},
	{1 << DBG_TRACE_NAT,"nat"},
	{1 << DBG_TRACE_CNT,"counters"},
	{1 << DBG_TRACE_FLOW_MEM,"ct_mem"},
	{1 << DBG_TRACE_HOSTNM,"hostname"},
	{1 << DBG_TRACE_TLS,"tls"},
	{1 << DBG_TRACE_JA3,"ja3"},
	{1 << DBG_TRACE_JA3MATCH,"ja3_match"},
	{1 << DBG_TRACE_EXCLUDE,"exclude"},
	{1 << DBG_TRACE_MATCH,"ipt_match"},
	{1 << DBG_TRACE_MATCH2,"ipt_match2"},
	{1 << DBG_TRACE_CACHE,"ct_cache"},
	{1 << DBG_TRACE_GUESSED,"guessed"},
	{1 << DBG_TRACE_DDONE,"dpi_done"},
	{1 << DBG_TRACE_DPI,"dpi"},
	{1 << DBG_TRACE_RE,"regex"},
	{1 << DBG_TRACE_TG1,"tgt_notmagic"},
	{1 << DBG_TRACE_TG2,"tgt_start"},
	{1 << DBG_TRACE_TG3,"tgt_flow_nat"},
	{1 << DBG_TRACE_SPROC,"procfs_set"},
	{1 << DBG_TRACE_SPROC_H,"procfs_set_host"},
	{1 << DBG_TRACE_SPROC_I,"procfs_set_ip"},
	{1 << DBG_TRACE_GPROC,"procfs_get"},
	{1 << DBG_TRACE_GPROC_H,"procfs_get_host"},
	{1 << DBG_TRACE_GPROC_H2,"procfs_get_host2"},
//	{1 << DBG_TRACE_GPROC_I,"procfs_get_ip"},
	{1 << DBG_TRACE_MATCH_CMD,"match_cmd"},
	{1 << DBG_TRACE_NETNS,"netns"},
}; // 27 < 32
#endif

static int net_ns_id=0;
static int ndpi_net_id;
static inline struct ndpi_net *ndpi_pernet(struct net *net)
{
	        return net_generic(net, ndpi_net_id);
}

/* detection */

static	enum nf_ct_ext_id nf_ct_ext_id_ndpi = 0;
static	struct kmem_cache *osdpi_flow_cache = NULL;
static struct kmem_cache *ct_info_cache = NULL;
static struct kmem_cache *bt_port_cache = NULL;

struct ndpi_flow_input_info input_info = {
	.in_pkt_dir = NDPI_IN_PKT_DIR_C_TO_S,
	.seen_flow_beginning = NDPI_FLOW_BEGINNING_UNKNOWN
};

/* debug functions */
static void debug_printf(u_int32_t protocol, struct ndpi_detection_module_struct *id_struct,
		ndpi_log_level_t log_level, const char *file_name,
		const char *func_name, unsigned line_number,
		const char * format, ...)
{
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
	struct ndpi_net *n = id_struct ? id_struct->user_data : NULL;
//	if(!n || protocol >= NDPI_NUM_BITS)
//		pr_info("ndpi_debug n=%d, p=%u, l=%s\n",n != NULL,protocol,
//				log_level < 5 ? dbl_lvl_txt[log_level]:"???");
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
                	pr_err("E: P=%d %s:%d %s",protocol, short_fn, line_number, /*func_name,*/ buf);
			break;
		case NDPI_LOG_TRACE:
                	pr_info("T: P=%d %s:%d %s",protocol, short_fn, line_number, /*func_name,*/ buf);
			break;
		case NDPI_LOG_DEBUG:
                	pr_info("D: P=%d %s:%d %s",protocol, short_fn, line_number, /*func_name,*/ buf);
			break;
		case NDPI_LOG_DEBUG_EXTRA:
                	pr_info("D2: P=%d %s:%d %s",protocol, short_fn, line_number, /*func_name,*/ buf);
			break;
		default:
			;
		}
        }
#endif
}

void set_debug_trace( struct ndpi_net *n) {
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
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
#endif
}

#ifdef NDPI_ENABLE_DEBUG_MESSAGES

#define DBG_NAMES_CNT (sizeof(dbg_ipt_names)/sizeof(dbg_ipt_names[0]))

int dbg_ipt_opt(char *lbuf,size_t count) {
	size_t i,l;
	lbuf[0] = '\0';
	for(i=0,l=0; i < DBG_NAMES_CNT;i++) {
		l += snprintf(&lbuf[l],count - l,"%-20s %c\n",dbg_ipt_names[i].name,
				ndpi_log_debug & dbg_ipt_names[i].mask ? 'Y':'n');
		if(l >= count - 1 && i < DBG_NAMES_CNT-1) {
			pr_err("ndpi:%s buffer %d too small\n",__func__,(int)count);
			break;
		}
	}
	lbuf[l] = '\0';
	return l;
}

uint32_t dbg_ipt_opt_get(const char *lbuf) {
	size_t i;
	if(!strcmp(lbuf,"all")) return 0xffffffff;
	for(i=0; i < DBG_NAMES_CNT;i++) {
		if(!strcmp(lbuf,dbg_ipt_names[i].name)) return dbg_ipt_names[i].mask;
	}
	return 0;
}
#endif

static char *ct_info(const struct nf_conn * ct,char *buf,size_t buf_size,int dir);

static void *malloc_wrapper(size_t size)
{
	if(in_atomic() || irqs_disabled() || in_interrupt())
		return kmalloc(size,GFP_ATOMIC);
	
	if(size > 32*1024) {
		/*
		 * Workarround for 32bit systems
		 * Large memory areas (more than 128 KB) are requested
		 * only during initial initialization.
		 * In this case, we can use kvmalloc() instead of kmalloc().
		 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
		return kvmalloc(size,GFP_KERNEL);
#else
		return vmalloc(size);
#endif
	}
	return kmalloc(size,GFP_KERNEL);
}

static void free_wrapper(void *freeable)
{
	kvfree(freeable);
}

static void fill_prefix_any(ndpi_prefix_t *p, union nf_inet_addr const *ip,int family) {
	memset(p, 0, sizeof(ndpi_prefix_t));
	p->ref_count = 0;
	if(family == AF_INET) {
		memcpy(&p->add.sin, &ip->in, sizeof(ip->in));
		p->family = AF_INET;
		p->bitlen = 32;
		return;
	}
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	if(family == AF_INET6) {
		memcpy(&p->add.sin6, &ip->in6, sizeof(ip->in6));
		p->family = AF_INET6;
		p->bitlen = 128;
	}
#endif
}

#ifdef NF_CT_CUSTOM
static inline void *nf_ct_ext_add_ndpi(struct nf_conn * ct)
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

static inline struct nf_ct_ext_ndpi *nf_ct_get_ext_ndpi(struct nf_ct_ext_labels *ext_l) {
	if(!ext_l) COUNTER(ndpi_p_ct_nolabel);
	  else
	    if(ext_l->magic != 0 && ext_l->magic != MAGIC_CT)
		COUNTER(ndpi_p_free_magic);
	return ext_l && ext_l->magic == MAGIC_CT ? ext_l->ndpi_ext:NULL;
}

static inline struct nf_ct_ext_ndpi *nf_ct_ext_find_ndpi(const struct nf_conn * ct)
{
#if   LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 0)
	struct nf_ct_ext_labels *l = (struct nf_ct_ext_labels *)nf_ct_ext_find(ct,nf_ct_ext_id_ndpi);
#else /* < 5.19 */
	struct nf_ct_ext_labels *l = (struct nf_ct_ext_labels *)__nf_ct_ext_find(ct,nf_ct_ext_id_ndpi);
#endif
	return nf_ct_get_ext_ndpi(l);
}

static inline struct nf_ct_ext_labels *nf_ct_ext_find_label(const struct nf_conn * ct)
{
#if   LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 0)
	return (struct nf_ct_ext_labels *)nf_ct_ext_find(ct,nf_ct_ext_id_ndpi);
#else /* < 5.19 */
	return (struct nf_ct_ext_labels *)__nf_ct_ext_find(ct,nf_ct_ext_id_ndpi);
#endif
}

DEFINE_SPINLOCK(lock_flist);

static inline int ndpi_ct_list_add(struct ndpi_net *n,
			struct nf_ct_ext_ndpi *ct_ndpi) {

	struct nf_ct_ext_ndpi *h;
	int ret = false;
	if(!test_flow_yes(ct_ndpi)) {
	    spin_lock_bh(&lock_flist);
	    h = READ_ONCE(n->flow_h);
	    WRITE_ONCE(ct_ndpi->next,h);
	    WRITE_ONCE(n->flow_h,ct_ndpi);
	    set_flow_yes(ct_ndpi);
	    spin_unlock_bh(&lock_flist);
	    atomic_inc(&n->acc_work);
	    ret = true;
	}
	return ret;
}

static void ndpi_init_ct_struct(struct ndpi_net *n,
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
}

static void ndpi_nat_detect(struct nf_ct_ext_ndpi *ct_ndpi, struct nf_conn * ct) {

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
	if(_DBG_TRACE_NAT)
		pr_info("ndpi: %s ct_ndpi %8p nat\n",__func__,ct_ndpi);
}

static inline void ndpi_ct_counters_add(struct nf_ct_ext_ndpi *ct_ndpi,
		size_t npkt, size_t len, enum ip_conntrack_info ctinfo, uint32_t m_time) {

	int dir = CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL ? 1:0;
	ct_ndpi->flinfo.b[dir] += len;
	ct_ndpi->flinfo.p[dir] += npkt;
	ct_ndpi->flinfo.time_end = m_time;
	set_flow_info(ct_ndpi);
	if(_DBG_TRACE_CNT)
		pr_info("ndpi: ct_ndpi %8p counter pkt %zu bytes %zu\n",ct_ndpi,npkt,len);
}


static inline void ndpi_free_ct_flow(struct nf_ct_ext_ndpi *ct_ndpi) {
	struct ndpi_flow_struct *flow = NULL;
	
	flow = xchg(&ct_ndpi->flow, NULL);
	if(flow) {
		ndpi_free_flow(flow);
		kmem_cache_free (osdpi_flow_cache, flow);
		COUNTER_D(ndpi_flow_c);
		module_put(THIS_MODULE);
	}
}

static inline void ndpi_free_ct_proto(struct nf_ct_ext_ndpi *ct_ndpi) {
	char *ptr = NULL;

	ptr = xchg(&ct_ndpi->host, NULL);
        if(ptr)
                kfree(ptr);
       
	ptr = xchg(&ct_ndpi->flow_opt, NULL);
        if(ptr)
                kfree(ptr);
}
static void
ct_ndpi_free_flow (struct ndpi_net *n,
		struct nf_ct_ext_labels *ext_l,
		struct nf_ct_ext_ndpi *ct_ndpi,
		int force, struct nf_conn * ct)
{
	int delete = 0,x_flow=0,x_info=0,x_ndpiflow=0;

	if(xchg(&ext_l->magic, 0) != MAGIC_CT)
		COUNTER(ndpi_p_free_magic);
	(void *)xchg(&ext_l->ndpi_ext, NULL);
	smp_wmb();
	spin_lock_bh(&ct_ndpi->lock); // FIXME: unneeded ?
	x_ndpiflow = ct_ndpi->flow != NULL;
	ndpi_free_ct_flow(ct_ndpi);
	if(test_flow_yes(ct_ndpi)) {
	    x_flow  = 1;
	    x_info = flow_have_info(ct_ndpi);
	    if(force == FLOW_FREE_FORCE || !flow_have_info(ct_ndpi)) {
	    	ndpi_free_ct_proto(ct_ndpi);
	    	clear_flow_info(ct_ndpi);
	    }
	    set_for_delete(ct_ndpi);
	    atomic_inc(&n->acc_rem);
	} else
	    delete = 1;
	spin_unlock_bh(&ct_ndpi->lock); // FIXME

	if(delete) {
	    	ndpi_free_ct_proto(ct_ndpi);
		kmem_cache_free (ct_info_cache, ct_ndpi);
	}
	if(_DBG_TRACE_FLOW_MEM)
	   pr_info("ndpi:%s free_flow ct %8p ct_ndpi %8p %s%s%s%s%s\n",
			n->ns_name,ct,ct_ndpi,
			x_ndpiflow ? " ndpi_flow":"",
			x_flow ? " flow_yes":"",
			x_info ? " info":"",
			delete ? " delete":"",
			force == FLOW_FREE_FORCE ? " force":"");
}

/* free ndpi info on ndpi_net_exit() */
static int
ndpi_cleanup_flow (struct nf_conn * ct,void *data) {
	struct nf_ct_ext_labels *ext_l = nf_ct_ext_find_label(ct);
	struct nf_ct_ext_ndpi *ct_ndpi = nf_ct_get_ext_ndpi(ext_l);
	struct ndpi_net *n = (struct ndpi_net *)data;

	if(!ct_ndpi) return 1;

	ct_ndpi_free_flow(n,ext_l,ct_ndpi,FLOW_FREE_FORCE,ct);

	return 1;
}

static void
nf_ndpi_free_flow (struct nf_conn * ct)
{
	struct nf_ct_ext_labels *ext_l = nf_ct_ext_find_label(ct);

	struct nf_ct_ext_ndpi *ct_ndpi = nf_ct_get_ext_ndpi(ext_l);

	if(ct_ndpi) {
	    struct ndpi_net *n;
	    n = ndpi_pernet(nf_ct_net(ct));

	    ct_ndpi_free_flow(n,ext_l,ct_ndpi,FLOW_FREE_NORM,ct);
	}
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

	ct_ndpi->proto = proto_null;
	ct_ndpi->flow = flow;
	__module_get(THIS_MODULE);
	COUNTER(ndpi_flow_c);
	if(_DBG_TRACE_FLOW_MEM)
		pr_info(" flow new ct_ndpi %8p\n", ct_ndpi);
        return flow;
}

/*****************************************************************/

static int ndpi_init_host_ac(struct ndpi_net *n) {
	ndpi_protocol_match *hm;
        int i,i2;

	AC_AUTOMATA_t *automa  = n->ndpi_struct->host_automa.ac_automa;
	if(automa->automata_open)
		ac_automata_finalize(automa);

	n->host_ac = ndpi_init_automa();
	if(!n->host_ac) {
		pr_err("xt_ndpi: cant alloc host_ac\n");
		return 0;
	}
	ac_automata_feature(n->host_ac,AC_FEATURE_LC);
	ac_automata_name(n->host_ac,"host",AC_FEATURE_DEBUG);
	for(hm = host_match; hm->string_to_match ; hm++) {
		size_t sml;
		ndpi_protocol_match_result s_ret;
		i = hm->protocol_id;
		if(i >= NDPI_NUM_BITS) {
			pr_err("xt_ndpi: bad proto num %d \n",i);
			continue;
		}
		sml = strlen(hm->string_to_match);
		/* Beginning checking for duplicates */
		i2 = ndpi_match_string_subprotocol(n->ndpi_struct,
							hm->string_to_match,sml,&s_ret);
		if(i2 == NDPI_PROTOCOL_UNKNOWN || i != i2) {
			pr_err("xt_ndpi: Warning! Hostdef missmatch '%s' proto_id %u, subproto %u, p:%u. Skipping.\n",
					hm->string_to_match,i,i2,s_ret.protocol_id
					);
			continue;
		}
		if(str_collect_look(n->hosts->p[i],hm->string_to_match,sml) >= 0) {
			pr_err("xt_ndpi: Warning! Hostdef '%s' duplicated! Skipping.\n",
					hm->string_to_match);
			continue;
		}
		/* Ending checking for duplicates */
		if(str_collect_add(&n->hosts->p[i],hm->string_to_match,sml) == NULL) {
			hm = NULL; // error
			pr_err("xt_ndpi: Error add %s\n",hm->string_to_match);
			break;
		}
	}
	if(hm && str_coll_to_automata(n->ndpi_struct,n->host_ac,n->hosts)) hm = NULL;
	if(!hm)
		pr_err("str_coll_to_automata failed\n");
	if(!hm) return 0;
	ac_automata_release(n->ndpi_struct->host_automa.ac_automa,1);
	n->ndpi_struct->host_automa.ac_automa = n->host_ac;
	n->host_ac = NULL;
	return 1;
}
static int
ndpi_enable_protocols (struct ndpi_net *n)
{
        int i,c=0, ret = 1;

        spin_lock_bh (&n->ipq_lock);
	if(atomic64_inc_return(&n->protocols_cnt[0]) == 1) {
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
		ndpi_bittorrent_init(n->ndpi_struct,
			bt_hash_size*1024,bt6_hash_size*1024,
			bt_hash_tmo,bt_log_size);
		ndpi_finalize_initialization(n->ndpi_struct);
	}
	spin_unlock_bh (&n->ipq_lock);
	return ret;
}


static char *ct_info(const struct nf_conn * ct,char *buf,size_t buf_size,int dir) {
 const struct nf_conntrack_tuple *t;
 if(!ct) {
	 strncpy(buf," null ",buf_size-1);
	 return buf;
 }
 t = &ct->tuplehash[!dir ? IP_CT_DIR_ORIGINAL: IP_CT_DIR_REPLY].tuple;
 if(t->src.l3num == AF_INET6)
 	snprintf(buf,buf_size, "proto %u %u %pI6c.%hu -> %pI6c.%hu %s",
		t->src.l3num,
		t->dst.protonum,
		&t->src.u3.all, ntohs(t->src.u.all),
		&t->dst.u3.all, ntohs(t->dst.u.all),
		!dir ? "DIR":"REV");
    else
 	snprintf(buf,buf_size, "proto %u %u %pI4:%hu -> %pI4:%hu %s",
		t->src.l3num,
		t->dst.protonum,
		&t->src.u3.ip, ntohs(t->src.u.all),
		&t->dst.u3.ip, ntohs(t->dst.u.all),
		!dir ? "DIR":"REV");
 return buf;
}

static void packet_trace(const struct sk_buff *skb,const struct nf_conn * ct,
	struct nf_ct_ext_ndpi *ct_ndpi,int ct_dir,const char *msg, const char *format, ...) {

  const struct iphdr *iph = ip_hdr(skb);
  char ndpi_info[32];
  char buf[256];
  char ct_buf[128];
  va_list args;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  const struct ipv6hdr *ip6h = ipv6_hdr(skb);

  if(ip6h && ip6h->version != 6) ip6h = NULL;
    else iph = NULL;
#endif

  memset(buf, 0, sizeof(buf));
  if(format && strchr(format,'%')) {
	va_start(args, format);
	vsnprintf(buf, sizeof(buf)-1, format, args);
	va_end(args);
  }

  ndpi_info[0] = 0;
  if(ct_ndpi) {
	snprintf(ndpi_info,sizeof(ndpi_info)-1," [%d,%d,%s] ",
		 ct_ndpi->proto.app_protocol, ct_ndpi->proto.master_protocol,
			ndpi_confidence_get_name(ct_ndpi->confidence));
  } else {
	snprintf(ndpi_info,sizeof(ndpi_info)-1,"[!ct_ndpi]");
  }
  ct_info(ct,ct_buf,sizeof(ct_buf)-1,ct_dir);
  if(iph) {
	if(iph->protocol == IPPROTO_TCP || iph->protocol == IPPROTO_UDP) {
		 struct udphdr *udph = (struct udphdr *)(((const u_int8_t *) iph) + iph->ihl * 4);
		 char tcp_flags[8] = "";
		 if(iph->protocol == IPPROTO_TCP) {
			int f_i = 0;
			struct tcphdr *tcph = (struct tcphdr *)udph;
			if(tcph->syn) tcp_flags[f_i++]='S';
			if(tcph->fin) tcp_flags[f_i++]='F';
			if(tcph->rst) tcp_flags[f_i++]='R';
			if(f_i)       tcp_flags[f_i++]=' ';
			tcp_flags[f_i++]='\0';
		 }
		 pr_info("%-13s skb %8p ct %8p %s%slen %d%s%s\n",
			msg ? msg:"",(void *)skb,(void *)ct,ct_buf,tcp_flags,skb->len,ndpi_info,buf);
  	} else
		 pr_info("%-13s skb %8p ct %8p %s len %d%s%s\n",
			msg ? msg:"",(void *)skb,(void *)ct,ct_buf,skb->len,ndpi_info,buf);
  } else if(ip6h) {
	pr_info("%-13s skb %8p ct %8p %s len %d%s%s\n",
		msg ? msg:"",(void *)skb,(void *)ct,ct_buf,skb->len,ndpi_info,buf);

  }
}

static int check_known_ip_service( struct ndpi_net *n,int family,
		union nf_inet_addr const *ipaddr, uint16_t port, uint8_t protocol,int *l_conf) {

	ndpi_prefix_t ipx;
	ndpi_patricia_node_t *node;
	uint16_t app_protocol = NDPI_PROTOCOL_UNKNOWN;
	fill_prefix_any(&ipx,ipaddr,family);

	spin_lock_bh (&n->ipq_lock);
	node = ndpi_patricia_search_best(
			family == AF_INET ? n->ndpi_struct->protocols->v4:
				n->ndpi_struct->protocols->v6,
			&ipx);
	if(node) {
	    if(protocol == IPPROTO_UDP || protocol == IPPROTO_TCP) {
		app_protocol = ndpi_check_ipport(node,port,protocol == IPPROTO_TCP);
		if(app_protocol != NDPI_PROTOCOL_UNKNOWN) {
		    *l_conf = app_protocol & 0x8000 ? NDPI_CONFIDENCE_DPI:NDPI_CONFIDENCE_CUSTOM_RULE;
		    app_protocol &= 0x7fff;
		}
	    }
	    if(app_protocol == NDPI_PROTOCOL_UNKNOWN) {
	    	app_protocol = node->value.u.uv32.user_value;
		if(app_protocol != NDPI_PROTOCOL_UNKNOWN) {
		    *l_conf = app_protocol & 0x8000 ? NDPI_CONFIDENCE_DPI:NDPI_CONFIDENCE_MATCH_BY_IP;
		    app_protocol &= 0x7fff;
		}
	    }
	}
	spin_unlock_bh (&n->ipq_lock);
	return app_protocol;
}

static u32
ndpi_process_packet(struct ndpi_net *n, struct nf_conn * ct, struct nf_ct_ext_ndpi *ct_ndpi,
		    const uint64_t time,
                    const struct sk_buff *skb,int dir, ndpi_protocol *proto)
{
        struct ndpi_flow_struct * flow;
	uint32_t low_ip, up_ip;
	uint16_t low_port = 0, up_port = 0, protocol;
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
		COUNTER(ndpi_p_err_noiphdr);
		return NDPI_PROCESS_ERROR;
	}

	flow = ct_ndpi->flow;
	if (!flow) {
		flow = ndpi_alloc_flow(ct_ndpi);
		if (!flow) {
			COUNTER(ndpi_p_err_alloc_flow);
			return NDPI_PROCESS_ERROR;
		}
	}

	{

	flow->packet_direction = dir;
	preempt_disable();
	*proto = ndpi_detection_process_packet(n->ndpi_struct,flow,
#ifdef NDPI_DETECTION_SUPPORT_IPV6
				ip6h ?	(uint8_t *) ip6h :
#endif
					(uint8_t *) iph,
					 skb->len, time, &input_info);
	}

//	if(proto->master_protocol == NDPI_PROTOCOL_UNKNOWN &&
//	          proto->app_protocol == NDPI_PROTOCOL_UNKNOWN ) 
	if(flow && !flow->ip_port_finished) {
	    int l_conf = NDPI_CONFIDENCE_UNKNOWN;
	    struct nf_conntrack_tuple const *t0 = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;

	    flow->ip_port_finished = 1;

	    protocol = t0->dst.protonum;

	    if(protocol == IPPROTO_TCP || protocol == IPPROTO_UDP) {
		low_port = htons(t0->src.u.tcp.port);
		up_port  = htons(t0->dst.u.tcp.port);
	    }
	    flow->ipdef_proto = check_known_ip_service(n, ip6h ? AF_INET6:AF_INET,
			&t0->dst.u3,up_port,protocol,&l_conf);
	    if(flow->ipdef_proto == NDPI_PROTOCOL_UNKNOWN)
		 flow->ipdef_proto = check_known_ip_service(n,ip6h ? AF_INET6:AF_INET,
			&t0->src.u3,low_port,protocol,&l_conf);
	    flow->ipdef_proto_level = l_conf;

	    if(flow->ipdef_proto != NDPI_PROTOCOL_UNKNOWN ) {
		flow->guessed_protocol_id_by_ip = flow->ipdef_proto;
		if(_DBG_TRACE_DPI || _DBG_TRACE_GUESSED)
			packet_trace(skb,ct,ct_ndpi,dir," check_known",
					" clevel %d [%d]",l_conf,flow->ipdef_proto);
	    }
	    if(0 && l_conf == NDPI_CONFIDENCE_UNKNOWN) {
#ifdef NDPI_DETECTION_SUPPORT_IPV6
			if(ip6h) {
				low_ip = 0;
				up_ip = 0;
			} else
#endif
			if(iph) {
				low_ip=ntohl(iph->saddr);
				up_ip=ntohl(iph->daddr);
			}
			//if(low_ip > up_ip) { uint32_t tmp_ip = low_ip; low_ip=up_ip; up_ip = tmp_ip; }
			//if(low_port > up_port) { uint16_t tmp_port = low_port; low_port=up_port; up_port = tmp_port; }
			*proto = ndpi_guess_undetected_protocol (n->ndpi_struct,flow,protocol);
			if(_DBG_TRACE_GUESSED)
				packet_trace(skb,ct,ct_ndpi,dir," guess_undet "," [%d,%d]",
						proto->app_protocol,proto->master_protocol);
	    }
	}
	preempt_enable();
	ct_ndpi->risk = flow->risk & n->risk_mask;

	return proto->app_protocol != NDPI_PROTOCOL_UNKNOWN ? proto->app_protocol:proto->master_protocol;
}
static inline int can_handle(const struct sk_buff *skb,uint8_t *l4_proto)
{
	const struct iphdr *iph;
	uint32_t l4_len;
	uint8_t proto;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	const struct ipv6hdr *ip6h;

	ip6h = ipv6_hdr(skb);
	if(ip6h->version == 6) {
		COUNTER(ndpi_p_ipv6);
		*l4_proto = ip6h->nexthdr;
		// FIXME!
		return 1;
	}
#endif
	iph = ip_hdr(skb);
        if(!iph) { /* not IP */
		COUNTER(ndpi_p_nonip); return 0;
	}
	if(iph->version != 4) {
		COUNTER(ndpi_p_nonip); return 0;
	}
	*l4_proto = proto = iph->protocol;
	COUNTER(ndpi_p_ipv4);

// FIXME
	if(ntohs(iph->frag_off) & 0x3fff) {
		COUNTER(ndpi_p_err_ip_frag_len); return 0;
	}
	if(skb->len <= (iph->ihl << 2)) {
		COUNTER(ndpi_p_err_ip_frag_len); return 0;
	}

	l4_len = skb->len - (iph->ihl << 2);
        if(proto == IPPROTO_TCP) {
		if(l4_len < sizeof(struct tcphdr)) {
			COUNTER(ndpi_p_err_bad_tcp_udp); return 0;
		}
		return 1;
	}
        if(proto == IPPROTO_UDP) {
		if(l4_len < sizeof(struct udphdr)) {
			COUNTER(ndpi_p_err_bad_tcp_udp); return 0;
		}
		return 1;
	}
	COUNTER(ndpi_p_non_tcpudp);
	return 1;
}
static inline u_int8_t is_ndpi_proto(struct nf_ct_ext_ndpi *ct_ndpi, u_int16_t id) {
    return ct_ndpi->proto.master_protocol == id
           || ct_ndpi->proto.app_protocol == id ? 1:0;
}

static char *ndpi_safe_hostname(const char *name) {
    /* sizeof host_server_name = 80 */
    char buf[160];

    size_t hlen = ndpi_min(strlen(name),sizeof(buf)/2-1);
    int i, j, need_q = 0;

    buf[0]= '\0';
    for(i=0; i < hlen; i++) {
	if(name[i] == '"' || name[i] <= ' ') {
		need_q = 1;
		break;
	}
    }
    if(!need_q) return kstrndup(name,hlen,GFP_ATOMIC);
    buf[j++] = '"';
    for(i=0; i < hlen; i++) {
	char c = name[i];
	if(c < ' ') c = '.';
	if(c == '"') buf[j++] = '\\';
	buf[j++] = c;
    }
    buf[j++] = '"'; buf[j] = '\0';
    return kstrndup(buf,j,GFP_ATOMIC);
}

static void ndpi_host_info(struct nf_ct_ext_ndpi *ct_ndpi) {

    const struct ndpi_flow_struct *flow;
    if(ct_ndpi->proto.app_protocol == NDPI_PROTOCOL_UNKNOWN &&
	ct_ndpi->proto.master_protocol == NDPI_PROTOCOL_UNKNOWN ) return;
    flow = ct_ndpi->flow;
    if(!flow) return;

    if(!ct_ndpi->host) {
	const char *name = flow->host_server_name;
	if(*name) {
		ct_ndpi->host = ndpi_safe_hostname(name);
		if(_DBG_TRACE_HOSTNM)
		    pr_info("%s: set hostname %s\n", __func__,ct_ndpi->host ? ct_ndpi->host:"(null)");
	}
    }

    if(ct_ndpi->flow_opt && test_tlsdone(ct_ndpi)) return;

    if (!(is_ndpi_proto(ct_ndpi,NDPI_PROTOCOL_TLS) ||
	  is_ndpi_proto(ct_ndpi,NDPI_PROTOCOL_QUIC))) return;
    {
	char buf[512];
	size_t l = 0;

       	if(_DBG_TRACE_TLS) 
		pr_info("%s: TLS hello_processed %d, cert_processed %d, extra_packets %d\n",__func__,
				flow->protos.tls_quic.hello_processed,
				flow->tls_quic.certificate_processed,
				flow->extra_packets_func ? 1:0
				);

	if(flow->protos.tls_quic.hello_processed &&
		(flow->tls_quic.certificate_processed || !flow->extra_packets_func))
		set_tlsdone(ct_ndpi);

	if(flow->protos.tls_quic.ja3_server[0]) {
	    ct_ndpi->ja3s = l+1;
	    l += snprintf(&buf[l],sizeof(buf)-1-l,"%s",
			  flow->protos.tls_quic.ja3_server);
	    buf[l++] = 0;
	}
	if(flow->protos.tls_quic.ja3_client[0]) {
	    ct_ndpi->ja3c = l+1;
	    l += snprintf(&buf[l],sizeof(buf)-1-l,"%s",
			  flow->protos.tls_quic.ja3_client);
	    buf[l++] = 0;
	}
	if(flow->protos.tls_quic.ja4_client[0]) {
	    ct_ndpi->ja4c = l+1;
	    l += snprintf(&buf[l],sizeof(buf)-1-l,"%s",
			  flow->protos.tls_quic.ja4_client);
	    buf[l++] = 0;
	}
	if(flow->protos.tls_quic.fingerprint_set) {
	    uint32_t * sha1 = (uint32_t *)flow->protos.tls_quic.sha1_certificate_fingerprint;
	    ct_ndpi->tlsfp = l+1;
	    l += snprintf(&buf[l],sizeof(buf)-1-l,"%08x%08x%08x%08x%08x",
			  htonl(sha1[0]),htonl(sha1[1]),htonl(sha1[2]),htonl(sha1[3]),htonl(sha1[4]));
	    buf[l++] = 0;
	}
	if(flow->protos.tls_quic.ssl_version) {
	    char buf_ver[18];
	    u_int8_t known_tls = 0;
	    char *r = ndpi_ssl_version2str(buf_ver, sizeof(buf_ver)-1,
				    flow->protos.tls_quic.ssl_version, &known_tls);
	    for(;*r;r++) if(*r == ' ') *r = '_';
	    ct_ndpi->tlsv = l+1;
	    l += snprintf(&buf[l],sizeof(buf)-1-l,"%s",buf_ver);
	    buf[l++] = 0;
	}

        if(_DBG_TRACE_JA3)
 	    pr_info("%s: TLS ja3s %s, ja3c %s, ja4c %s, tlsfp %s, tlsv %s\n",
		__func__,
		ct_ndpi->ja3s  ? buf+ct_ndpi->ja3s-1 : "",
		ct_ndpi->ja3c  ? buf+ct_ndpi->ja3c-1 : "",
		ct_ndpi->ja4c  ? buf+ct_ndpi->ja4c-1 : "",
		ct_ndpi->tlsfp ? buf+ct_ndpi->tlsfp-1 : "",
		ct_ndpi->tlsv  ? buf+ct_ndpi->tlsv-1  : "");
       	if(_DBG_TRACE_TLS)
		pr_info("%s: TLS %s\n",__func__,
				test_tlsdone(ct_ndpi) ? "done":"in process");
	if(l != 0) {
	    char *new_flow_opt;
	    buf[l++] = 0;
	    new_flow_opt = kmalloc( l, GFP_ATOMIC);
	    if(new_flow_opt) {
		if(ct_ndpi->flow_opt)
		    kfree(ct_ndpi->flow_opt);
		ct_ndpi->flow_opt = new_flow_opt;
	        memcpy(ct_ndpi->flow_opt,buf,l);
	    }
	}
    }
}

static bool ndpi_host_match( const struct xt_ndpi_mtinfo *info,
			     const struct nf_ct_ext_ndpi *ct_ndpi) {
bool res = false;

if(!info->hostname[0]) return true;

if(info->host && ct_ndpi->host)
    res = info->re ? ndpi_regexec(info->reg_data,ct_ndpi->host) != 0 :
				strstr(ct_ndpi->host,info->hostname) != NULL;
if(_DBG_TRACE_HOSTNM)
    pr_info("%s: match%s %s '%s' %s %d\n", __func__,
	info->re ? "-re":"", info->host ? "host":"",
	info->hostname,ct_ndpi->host ? ct_ndpi->host:"-",res);

return res;
}

static inline uint16_t get_in_if(const struct net_device *dev) {

	return dev ? dev->ifindex:0;
}

static inline int check_excluded_proto(const struct xt_ndpi_mtinfo *info,
    const ndpi_protocol_bitmask_struct_t *excluded, int tls) {
    int i;

    for(i=0; i < NDPI_NUM_FDS_BITS; i++) {
	if(_DBG_TRACE_EXCLUDE)
	    if(info->flags.fds_bits[i]) pr_info("%s: %d: %08x & %08x = %08x\n", __func__,
		    i,info->flags.fds_bits[i],excluded->fds_bits[i],
		    info->flags.fds_bits[i] & excluded->fds_bits[i]);
	if((info->flags.fds_bits[i] & excluded->fds_bits[i]) != info->flags.fds_bits[i])
	    return 0;
    }
    if(tls && tls == 1) return 0;
    return 1;
}

static bool ndpi_j3_match(struct ndpi_detection_module_struct *ndpi_struct,
		const struct xt_ndpi_mtinfo *info,
		const char *prefix,const char *buf) {
	char key[64];
	size_t sml;
	int r;
	ndpi_protocol_match_result s_ret = {.protocol_id = -1};
	strncpy(key,prefix,sizeof(key)-1);
	strncat(key,buf,sizeof(key)-1);
	sml = strlen(key);
	r = ndpi_match_string_subprotocol(ndpi_struct,key,sml,&s_ret);
	if(r == NDPI_PROTOCOL_UNKNOWN) return 0;
	if(_DBG_TRACE_JA3MATCH)
		pr_info("%s: %zd:%s proto %u matched %d\n",__func__,sml,key,s_ret.protocol_id,
				NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,s_ret.protocol_id)!=0);
	return NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,s_ret.protocol_id) != 0;
}

static void ndpi_check_opt(struct ndpi_detection_module_struct *ndpi_struct,
	const struct xt_ndpi_mtinfo *info,
	struct nf_ct_ext_ndpi *ct_ndpi,
	bool *host_matched, bool *ja3s_matched, bool *ja3c_matched,bool *ja4c_matched,
	bool *tlsfp_matched, bool *tlsv_matched)
{
	if(info->host && info->hostname[0])
		*host_matched = ndpi_host_match(info,ct_ndpi);
	if(ct_ndpi->flow_opt) {
	    if(info->ja3s && ct_ndpi->ja3s)
		  *ja3s_matched = ndpi_j3_match(ndpi_struct,info,"JA3S_",ct_ndpi->flow_opt+ct_ndpi->ja3s-1);
	    if(info->ja3c && ct_ndpi->ja3c)
		  *ja3c_matched = ndpi_j3_match(ndpi_struct,info,"JA3C_",ct_ndpi->flow_opt+ct_ndpi->ja3c-1);
	    if(info->ja4c && ct_ndpi->ja4c)
		  *ja4c_matched = ndpi_j3_match(ndpi_struct,info,"JA4C_",ct_ndpi->flow_opt+ct_ndpi->ja4c-1);
	    if(info->tlsfp && ct_ndpi->tlsfp)
		  *tlsfp_matched = ndpi_j3_match(ndpi_struct,info,"TLSFP_",ct_ndpi->flow_opt+ct_ndpi->tlsfp-1);
	    if(info->tlsv && ct_ndpi->tlsv)
		  *tlsv_matched = ndpi_j3_match(ndpi_struct,info,"TLSV_",ct_ndpi->flow_opt+ct_ndpi->tlsv-1);
	    if(_DBG_TRACE_JA3) pr_info("%s: flow_opt %c%s, %c%s, %c%s, %c%s, %c%s\n",__func__,
			*ja3s_matched ? '+':'-',
			 ct_ndpi->ja3s  ? ct_ndpi->flow_opt+ct_ndpi->ja3s-1:"",
			*ja3c_matched ? '+':'-',
			 ct_ndpi->ja3c  ? ct_ndpi->flow_opt+ct_ndpi->ja3c-1:"",
			*ja4c_matched ? '+':'-',
			 ct_ndpi->ja4c  ? ct_ndpi->flow_opt+ct_ndpi->ja4c-1:"",
			*tlsfp_matched ? '+':'-',
			 ct_ndpi->tlsfp ? ct_ndpi->flow_opt+ct_ndpi->tlsfp-1:"",
			*tlsv_matched ? '+':'-',
			 ct_ndpi->tlsv  ? ct_ndpi->flow_opt+ct_ndpi->tlsv-1:""
			);
	}
}
static int check_guessed_protocol(struct nf_ct_ext_ndpi *ct_ndpi,ndpi_protocol *proto) {

	struct ndpi_flow_struct *flow = ct_ndpi->flow;
	int ret = 0;
	if(!flow) return 0;
	if(_DBG_TRACE_GUESSED)
		pr_info("%s: ct_clevel %d, proto.app %d, flow clevel %d, g_host_id %d, g_id %d %s\n",__func__,
				ct_ndpi->confidence,
				proto->app_protocol,
				flow->confidence,
				flow->guessed_protocol_id_by_ip,
				flow->guessed_protocol_id,
				NDPI_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask,
					flow->guessed_protocol_id) != 0 ? "excluded":""
				);
	if(ct_ndpi->confidence >= NDPI_CONFIDENCE_DPI_CACHE) return 0;

	if(proto->app_protocol != NDPI_PROTOCOL_UNKNOWN) return 0;

	if(flow->guessed_protocol_id != NDPI_PROTOCOL_UNKNOWN &&
	   NDPI_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask,
						flow->guessed_protocol_id) == 0) {
		proto->app_protocol = flow->guessed_protocol_id;
		if(_DBG_TRACE_GUESSED)
			pr_info("%s: guessed app_protocol %d\n",__func__,proto->app_protocol);
		ret = 1;
	}
	if(flow->guessed_protocol_id_by_ip != NDPI_PROTOCOL_UNKNOWN &&
	   flow->ipdef_proto_level >= flow->confidence) {
	   	if(proto->app_protocol == NDPI_PROTOCOL_UNKNOWN) {
			proto->app_protocol = flow->guessed_protocol_id_by_ip;
			if(_DBG_TRACE_GUESSED)
			    pr_info("%s: host app_protocol %d\n",__func__,proto->app_protocol);
		} else
		   	if(proto->master_protocol == NDPI_PROTOCOL_UNKNOWN) {
			    proto->master_protocol = flow->guessed_protocol_id_by_ip;
			    if(_DBG_TRACE_GUESSED)
				pr_info("%s: host master_protocol %d\n",__func__,proto->master_protocol);
			}
		flow->confidence = flow->ipdef_proto_level;
		ret = 1;
	}
	return ret;
}
static void check_tls_done(struct nf_ct_ext_ndpi *ct_ndpi,
	uint8_t *detect_complete, uint8_t *tls ) {

	if(ct_ndpi->confidence == NDPI_CONFIDENCE_DPI &&
	    (is_ndpi_proto(ct_ndpi,NDPI_PROTOCOL_TLS) ||
	     is_ndpi_proto(ct_ndpi,NDPI_PROTOCOL_QUIC)) ) {
		if(test_tlsdone(ct_ndpi)) {
			*detect_complete = 1;
			*tls = 2;
		} else {
			*detect_complete = 0;
			*tls = 1;
		}
	}
}

static ndpi_risk fix_unidir_trafic(struct nf_ct_ext_ndpi *ct_ndpi) {

static const ndpi_risk unidir_traf = 1ull << NDPI_UNIDIRECTIONAL_TRAFFIC;

	if((ct_ndpi->risk & unidir_traf) != 0 &&
	    ct_ndpi->flinfo.p[0] != 0 && ct_ndpi->flinfo.p[1] != 0)
		    ct_ndpi->risk ^= unidir_traf;

	return ct_ndpi->risk;
}
static void pr_dc(const char *msg,uint8_t dc,ndpi_protocol_bitmask_struct_t *ex_p) {
    uint32_t *e = &ex_p->fds_bits[0];
    pr_info("%-10s detect done:%d excl:%08x%08x%08x%08x%08x%08x%08x%08x\n",
	msg,dc,e[0],e[1],e[2],e[3],e[4],e[5],e[6],e[7]);
}


#define pack_proto(proto) ((proto.app_protocol << 16) | proto.master_protocol)

static bool
ndpi_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	uint32_t r_proto;
	ndpi_protocol proto = NDPI_PROTOCOL_NULL;
	uint64_t time;
	struct timespec64 tm;
	const struct xt_ndpi_mtinfo *info = par->matchinfo;

	enum ip_conntrack_info ctinfo;
	struct nf_conn * ct = NULL;
	struct sk_buff *linearized_skb = NULL;
	const struct sk_buff *skb_use = NULL;
	struct nf_ct_ext_ndpi *ct_ndpi = NULL;
	struct ndpi_cb *c_proto;
	ndpi_risk risk = 0;
	ndpi_protocol_bitmask_struct_t excluded_proto;
	uint8_t l4_proto=0,ct_dir=0,detect_complete=1,untracked=1,confidence=0,tls=0;
	bool result=false, host_matched = false, is_ipv6=false,
	     ja3s_matched = false, ja3c_matched = false, ja4c_matched = false,
	     tlsfp_matched = false, tlsv_matched = false,
	     ct_create = false, new_packet = false;
	struct ndpi_net *n;

	char ct_buf[128];

#ifdef NDPI_DETECTION_SUPPORT_IPV6
	const struct ipv6hdr *ip6h;

	ip6h = ipv6_hdr(skb);
	is_ipv6 = ip6h && ip6h->version == 6;
#endif
	n = ndpi_pernet(xt_net(par));

	/* ndpi_net_init not completed or ndpi_net_exit started */
	if(!atomic_read(&n->ndpi_ready))
		return 0;

	if(!read_trylock(&n->ndpi_busy))
		return 0;

	/* all excluded by default */

	memset((char *)&excluded_proto,0xff,sizeof(excluded_proto));

	proto.app_protocol = NDPI_PROCESS_ERROR;

	c_proto = skb_get_cproto(skb);

    do {
	if(!n->ndpi_struct || !n->ndpi_struct->finalized)
		break;

	if(READ_ONCE(c_proto->magic) == NDPI_ID &&
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

	COUNTER(ndpi_p_ndpi_match);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,18,0)
	ktime_get_real_ts64(&tm);
#else
	ktime_get_coarse_real_ts64(&tm);
#endif

	ct = nf_ct_get (skb, &ctinfo);
	if (ct == NULL) {
		COUNTER(ndpi_p_ct_null);
		break;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
	if (nf_ct_is_untracked(ct))
#else
	if(ctinfo == IP_CT_UNTRACKED)
#endif
	{
		COUNTER(ndpi_p_ct_untrack);
		break;
	}
	ct_dir = CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL;
	{
	    struct nf_ct_ext_labels *ct_label = nf_ct_ext_find_label(ct);
            if (info->untracked) {
                    untracked = !ct_label || !ct_label->magic || ct_label->magic != MAGIC_CT;
                    break;
            }
#ifdef NF_CT_CUSTOM
	    if(!ct_label) {
		if(nf_ct_is_confirmed(ct)) {
			COUNTER(ndpi_p_ct_confirm);
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
				(void *)xchg(&ct_label->ndpi_ext, ct_ndpi);
				(void)xchg(&ct_label->magic, MAGIC_CT);
	    			ct_create = true;
				ndpi_init_ct_struct(n,ct_ndpi,l4_proto,ct,is_ipv6,tm.tv_sec);
				ct_ndpi->flinfo.ifidx = get_in_if(xt_in(par));
				ct_ndpi->flinfo.ofidx = get_in_if(xt_out(par));
			}
		} else {
			if(ct_label->magic == MAGIC_CT)
				ct_ndpi = ct_label->ndpi_ext;
                           else
				COUNTER(ndpi_p_err_add_ndpi);
		}
	    } else
		COUNTER(ndpi_p_ct_nolabel);
	}

	if(!ct_ndpi) {
		COUNTER(ndpi_p_ct_ndpi);
		break;
	}

	if(c_proto->magic != NDPI_ID) {
		ct_proto_set_flow(c_proto,ct,0);
		WRITE_ONCE(c_proto->magic,NDPI_ID);
		new_packet = true;
	}

	detect_complete = 0;
	proto.app_protocol = NDPI_PROTOCOL_UNKNOWN;
	if(_DBG_TRACE_PKT)
		packet_trace(skb,ct,ct_ndpi,ct_dir,"<START match ",NULL);
	if(_DBG_TRACE_CT)
		pr_info(" %-7s  ct_ndpi %8p ct %8p %s\n",ct_create?"Create":"Reuse",
			(void *)ct_ndpi, (void *)ct, ct_info(ct,ct_buf,sizeof(ct_buf),ct_dir));

	barrier();
	spin_lock_bh (&ct_ndpi->lock);

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
	if( !new_packet ) {
		if(ct_proto_last(c_proto) != ct) {
		    // Bug!
		    if(_DBG_TRACE_CT) pr_info(" %-7s  ct_ndpi %8p ct %8p %s\n","!ct_last",
			(void *)ct_ndpi, (void *)ct, ct_info(ct,ct_buf,sizeof(ct_buf),ct_dir));
		    if(ct_proto_last(c_proto))
			COUNTER(ndpi_p_c_last_ct_not);
		    break;
		}
		risk = ct_ndpi->risk;
		confidence = ct_ndpi->confidence;
		proto.app_protocol = ct_ndpi->proto.app_protocol;
		proto.master_protocol = ct_ndpi->proto.master_protocol;
		detect_complete = test_detect_done(ct_ndpi);
		if((proto.master_protocol != NDPI_PROTOCOL_UNKNOWN ||
		    proto.app_protocol != NDPI_PROTOCOL_UNKNOWN) &&
		    confidence == NDPI_CONFIDENCE_DPI)
			detect_complete = 1;
		if(!detect_complete && ct_ndpi->flow)
			excluded_proto = ct_ndpi->flow->excluded_protocol_bitmask;
		    else
			detect_complete = 1;
		check_tls_done(ct_ndpi,&detect_complete,&tls);
		COUNTER(ndpi_p_cached);
		if(_DBG_TRACE_CACHE || _DBG_TRACE_PKT)
		    packet_trace(skb,ct,ct_ndpi,ct_dir,"cached",NULL);
		break;
	}
	COUNTER(ndpi_p_c_new_pkt); // new packet
	if(_DBG_TRACE_CT) pr_info(" %-7s  ct_ndpi %8p ct %8p %s\n","newpkt",
		(void *)ct_ndpi, (void *)ct, ct_info(ct,ct_buf,sizeof(ct_buf),ct_dir));

	if( test_flow_yes(ct_ndpi) &&
	    !test_nat_done(ct_ndpi) &&
	    !ct_proto_get_flow_nat(c_proto))
		ct_proto_set_flow_nat(c_proto,FLOW_NAT_START);

	/* don't pass icmp for TCP/UDP to ndpi_process_packet()  */
	if(l4_proto == IPPROTO_ICMP && ct_ndpi->l4_proto != IPPROTO_ICMP) {
		proto.master_protocol = NDPI_PROTOCOL_IP_ICMP;
		proto.app_protocol = NDPI_PROTOCOL_IP_ICMP;
		COUNTER(ndpi_p_l4mismatch);
		ndpi_p_l4mis_size += skb->len;
		break;
	}
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	if(l4_proto == IPPROTO_ICMPV6 && ct_ndpi->l4_proto != IPPROTO_ICMPV6) {
		proto.master_protocol = NDPI_PROTOCOL_IP_ICMPV6;
		proto.app_protocol = NDPI_PROTOCOL_IP_ICMPV6;
		COUNTER(ndpi_p_l4mismatch);
		ndpi_p_l4mis_size += skb->len;
		break;
	}
#endif
	if(ndpi_enable_flow && new_packet)
		ndpi_ct_counters_add(ct_ndpi,1,skb->len, ctinfo, tm.tv_sec);

	if( ct_ndpi->flow || ct_create) {
		struct ndpi_flow_struct *flow = NULL;
		if (skb_is_nonlinear(skb)) {
			linearized_skb = skb_copy(skb, GFP_ATOMIC);
			if (linearized_skb == NULL) {
				COUNTER(ndpi_falloc);
				detect_complete = 1;
				proto.app_protocol = NDPI_PROCESS_ERROR;
				break;
			}
			skb_use = linearized_skb;
			ndpi_nskb += 1;
		} else {
			skb_use = skb;
			ndpi_lskb += 1;
		}

		time = (uint64_t)(tm.tv_sec*1000 + tm.tv_nsec/1000000);

		n = ndpi_pernet(nf_ct_net(ct));
		r_proto = ndpi_process_packet(n, ct, ct_ndpi, time, skb_use, ct_dir, &proto);

		if(linearized_skb != NULL)
			kfree_skb(linearized_skb);

		if(_DBG_TRACE_DPI && ct_ndpi->flow)
		   pr_info(" ndpi_process_packet dpi: g_pr:%d g_host_pr:%d m:%d a:%d cl:%s; ct: m:%d a:%d cl:%s r:%llx pcnt %d [%d,%d]%s%s\n",
			ct_ndpi->flow->guessed_protocol_id,
			ct_ndpi->flow->guessed_protocol_id_by_ip,
			proto.master_protocol,
			proto.app_protocol,
			ndpi_confidence_get_name(ct_ndpi->flow->confidence),
			ct_ndpi->proto.master_protocol,
			ct_ndpi->proto.app_protocol,
			ndpi_confidence_get_name(ct_ndpi->confidence),
			(uint64_t)ct_ndpi->risk,
			ct_ndpi->flow->packet_counter,
			ct_ndpi->flow->packet_direction_counter[0],
			ct_ndpi->flow->packet_direction_counter[1],
			ct_ndpi->flow->extra_packets_func ? ", extra_func":"",
			ct_ndpi->flow->fail_with_unknown ? ", end_dpi":"");

		COUNTER(ndpi_p_ndpi);
		flow = ct_ndpi->flow;

		if(r_proto == NDPI_PROCESS_ERROR || !flow) {
		    COUNTER(ndpi_p_err_prot_err);
		    c_proto->proto = r_proto;
		    proto.app_protocol = r_proto;
		    proto.master_protocol = NDPI_PROTOCOL_UNKNOWN;
		    detect_complete = 1;
		    confidence = NDPI_CONFIDENCE_UNKNOWN;
		    break;
		}

		excluded_proto = flow->excluded_protocol_bitmask;
		check_guessed_protocol(ct_ndpi,&proto);
		ct_ndpi->confidence = confidence = flow->confidence;
		ct_ndpi->proto.app_protocol = proto.app_protocol;
		ct_ndpi->proto.master_protocol = proto.master_protocol;
		c_proto->proto = pack_proto(proto);
		risk = fix_unidir_trafic(ct_ndpi);

		ndpi_host_info(ct_ndpi);

		check_tls_done(ct_ndpi,&detect_complete,&tls);

		if(ct_ndpi->confidence != NDPI_CONFIDENCE_UNKNOWN) {
		    if( proto.app_protocol <= NDPI_NUM_BITS)
		    	atomic64_inc(&n->protocols_cnt[proto.app_protocol]);
		    if( proto.master_protocol != proto.app_protocol &&
		        proto.master_protocol <= NDPI_NUM_BITS)
				atomic64_inc(&n->protocols_cnt[proto.master_protocol]);
		}

		if(ct_ndpi->confidence == NDPI_CONFIDENCE_DPI) {
		    if(tls) {
			    detect_complete = tls == 2;
			    if(!detect_complete) break;
		    }
		    detect_complete  = 1;
		    if(_DBG_TRACE_DDONE)
			packet_trace(skb,ct,ct_ndpi,ct_dir,"dpi_done completed","tls %d %s",
		    			tls, flow->extra_packets_func ?
					  " extra_packets":" free_ct_flow");
		    if(!flow->extra_packets_func) {
			set_detect_done(ct_ndpi);
			ndpi_free_ct_flow(ct_ndpi);
		    }
		    break;
		}

		if(ct_ndpi->confidence < NDPI_CONFIDENCE_DPI_CACHE || flow->fail_with_unknown) {
		    int max_packet_unk =
		         (ct_ndpi->l4_proto == IPPROTO_TCP) ? max_packet_unk_tcp:
		         (ct_ndpi->l4_proto == IPPROTO_UDP) ? max_packet_unk_udp : max_packet_unk_other;
		    if( flow->fail_with_unknown || (flow->packet_counter > max_packet_unk && !flow->extra_packets_func)) {
			if(flow->fail_with_unknown)
				COUNTER(ndpi_p_c_end_fail);
			    else
				COUNTER(ndpi_p_c_end_max);
		    	detect_complete = 1;
			if(proto.app_protocol == NDPI_PROTOCOL_UNKNOWN) {
			    u_int8_t proto_guessed;
			    ndpi_protocol p_old = proto;
			    proto = ndpi_detection_giveup(n->ndpi_struct, flow, &proto_guessed);
			    if(_DBG_TRACE_DPI &&
			           (p_old.app_protocol != proto.app_protocol ||
				    p_old.master_protocol != proto.master_protocol ||
				    confidence != flow->confidence))
				packet_trace(skb,ct,ct_ndpi,ct_dir," detection_giveup"," app,master [%u,%u]->[%u,%u] c %u->%u\n",
						p_old.app_protocol,p_old.master_protocol,
						proto.app_protocol,proto.master_protocol,
						confidence,flow->confidence);
			    ct_ndpi->proto.app_protocol = proto.app_protocol;
			    ct_ndpi->proto.master_protocol = proto.master_protocol;
			    ct_ndpi->confidence = confidence = flow->confidence;
			    c_proto->proto = pack_proto(proto);
			}
		    	if(_DBG_TRACE_DDONE)
		    	    packet_trace(skb,ct,ct_ndpi,ct_dir,"dpi_done ","%s %d, free flow",
					    flow->fail_with_unknown ? "fail_with_unknown":"max_packet",max_packet_unk);
		    	set_detect_done(ct_ndpi);
		    	ndpi_free_ct_flow(ct_ndpi);
		    }
		}
	} else { // ct_ndpi->flow == NULL
		proto.app_protocol = ct_ndpi->proto.app_protocol;
		proto.master_protocol = ct_ndpi->proto.master_protocol;
		risk = fix_unidir_trafic(ct_ndpi);
		confidence = ct_ndpi->confidence;
		c_proto->proto = pack_proto(proto);
		check_tls_done(ct_ndpi,&detect_complete,&tls);
		detect_complete = 1;
		if(_DBG_TRACE_PKT || _DBG_TRACE_DDONE)
		    packet_trace(skb,ct,ct_ndpi,ct_dir," nondpi    ",NULL);
	}

    } while(0);

    if(ct_ndpi) {
	if(proto.app_protocol != NDPI_PROCESS_ERROR)
		ndpi_check_opt(n->ndpi_struct,info,ct_ndpi, &host_matched, &ja3s_matched,
				&ja3c_matched, &ja4c_matched, &tlsfp_matched, &tlsv_matched);
	spin_unlock_bh (&ct_ndpi->lock);
    }

    read_unlock(&n->ndpi_busy);

    if(_DBG_TRACE_MATCH2 && !info->error && !info->untracked) {
	    pr_info(" ndpi_match master %d, app %d, host %d, ja3s %d, ja3c %d, ja4c %d, tlsfp %d tlsv %d"
	    	" excluded %d, master_map %d, app_map %d\n",
		proto.master_protocol,proto.app_protocol,
		host_matched,ja3s_matched,ja3c_matched,ja3c_matched,tlsfp_matched,tlsv_matched,
		check_excluded_proto(info,&excluded_proto,tls) != 0,
		NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.master_protocol) != 0,
		NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.app_protocol) != 0);
	    pr_dc(" ndpi_match",detect_complete,&excluded_proto);
    }

    result = true;
    do {
	if(info->error) {
		result = proto.app_protocol == NDPI_PROCESS_ERROR;
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match error: %s\n",result ? "yes":"no");
		break;
	}
	if(info->untracked) {
		result = untracked;
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match untracked: %s\n",result ? "yes":"no");
		break;
	}
	if(info->host) {
		result &= host_matched;
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match host: %s\n",result ? "yes":"no");
		if(!result) break;
	}
	if (info->ja3s) {
		result &= ja3s_matched;
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match ja3s: %s\n",result ? "yes":"no");
	} else
	 if (info->ja3c) {
		result &= ja3c_matched;
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match ja3c: %s\n",result ? "yes":"no");
	 } else
	 if (info->ja4c) {
		result &= ja4c_matched;
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match ja4c: %s\n",result ? "yes":"no");
	 } else
	  if (info->tlsfp) {
		result &= tlsfp_matched;
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match tlsfp: %s\n",result ? "yes":"no");
	  } else
	   if (info->tlsv) {
		result &= tlsv_matched;
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match tlsv: %s\n",result ? "yes":"no");
	   } else
	    if(info->inprogress) {
		result &= detect_complete ? 0 : !check_excluded_proto(info,&excluded_proto,tls);
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match inprogress: %s : detect_complete:%d check_excluded_proto %d\n",
				    result ? "yes":"no",detect_complete,check_excluded_proto(info,&excluded_proto,tls));
	    } else { // protocol
		if(!info->empty) {
		    if (info->m_proto && !info->p_proto)
			result &= NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.master_protocol) != 0;
		      else
			if (!info->m_proto && info->p_proto)
			    result &= NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.app_protocol) != 0;
			  else {
			    if(proto.app_protocol != NDPI_PROTOCOL_UNKNOWN && proto.master_protocol != NDPI_PROTOCOL_UNKNOWN)
				result &= (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.app_protocol) != 0 ||
					   NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.master_protocol) != 0);
			      else
				   if(proto.app_protocol != NDPI_PROTOCOL_UNKNOWN)
					  result &= NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.app_protocol) != 0;
					else
					  result &= NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.master_protocol) != 0;
			}
		    if(_DBG_TRACE_MATCH) {
			    const char *t_master = ndpi_get_proto_by_id(n->ndpi_struct,proto.master_protocol);
			    const char *t_app = ndpi_get_proto_by_id(n->ndpi_struct,proto.app_protocol);
			    pr_info(" ndpi_match protocol: %s : master %s(%d) app %s(%d)\n",
					result ? "yes":"no",
					t_master ? t_master:"???",
					NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.master_protocol) != 0,
					t_app ? t_app : "???",
					NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto.app_protocol) != 0);
		    }
	       }
	}
	if(!result) break;

	if (info->have_master) {
		result &= proto.master_protocol != NDPI_PROTOCOL_UNKNOWN;
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match have_master: %s\n",result ? "yes":"no");
		if(!result) break;
	}
	if(info->clevel) {
		switch(info->clevel_op) {
		     case 1: result &= info->clevel-1 < confidence;
			 break;
		     case 2: result &= info->clevel-1 > confidence;
			 break;
		     default: result &= info->clevel-1 == confidence;
		}
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match confidence: %s : %s\n",
				    result ? "yes":"no",ndpi_confidence_get_name(confidence));
		if(!result) break;
	}
	if(info->risk) {
		result &= (info->risk & risk)  != 0;
		if(_DBG_TRACE_MATCH)
		    pr_info(" ndpi_match risk: %s\n",result ? "yes":"no");
		if(!result) break;
	}
    } while(0);

    if(_DBG_TRACE_MATCH)
	packet_trace(skb,ct,ct_ndpi,ct_dir,">END   match "," result %d", result  ^ (info->invert != 0));

    return result ^ (info->invert != 0);
}


static int
ndpi_mt_check(const struct xt_mtchk_param *par)
{
struct xt_ndpi_mtinfo *info = par->matchinfo;
#if 0
/*
 *  invert:1,error:1,m_proto:1,p_proto:1,have_master:1,
 *  host:1,re:1,empty:1,proto:1,inprogress:1,ja3s:1,ja3c:1,ja4c:1,tlsfp:1,tlsv:1,
 *  untracked:1,clevel:4,clevel_op:2;
 */
	if(_DBG_TRACE_MATCH_CMD) {
		char cbuf[512];
		const char *t_p;
		struct ndpi_net *n;
		int i;
		size_t l = 0;
		cbuf[0] = 0;
		n = ndpi_pernet(par->net);
		for(i=0; l < sizeof(cbuf)-10 && i < NDPI_NUM_BITS; i++) {
			if(NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,i) == 0) continue;
			t_p = ndpi_get_proto_by_id(n->ndpi_struct,i);
			if(t_p) l += snprintf(&cbuf[l],sizeof(cbuf)-l-1,"%s%s",l?",":"",t_p);
			   else l += snprintf(&cbuf[l],sizeof(cbuf)-l-1,"%s_%d",l?",":"",i);
		}
		pr_info("Rule: invert:%d error:%d untracked:%d inprogress:%d have_master:%d p_proto:%d m_proto:%d\n",
				info->invert&1, info->error&1, info->untracked&1, info->inprogress&1,info->have_master&1,
				info->p_proto&1, info->m_proto&1);
		pr_info("      host:%d re:%d empty:%d proto:%d ja3s:%d ja3c:%d ja4c:%d tlsfp:%d tlsv:%d clevel:%d clevel_op:%d\n",
				info->host&1, info->re&1,  info->empty&1, info->proto&1,
				info->ja3s&1, info->ja3c&1, info->ja4c&1,info->tlsfp&1, info->tlsv&1,
				info->clevel&7,info->clevel_op&3);
		pr_info("      hostname:%s protos:%s\n",info->hostname,cbuf);
	}
#endif
	if (!info->error && !info->inprogress && !info->untracked && !info->have_master && !info->hostname[0] &&
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
		if(_DBG_TRACE_RE)
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
	return ndpi_enable_protocols (ndpi_pernet(par->net)) ? 0:-EINVAL;
}

static void
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

char *ndpi_proto_to_str(char *buf,size_t size,ndpi_protocol *p,ndpi_mod_str_t *ndpi_str)
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
static unsigned int seq_print_ndpi(struct seq_file *s,
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

static u_int32_t ndpi_proto_markmask(struct ndpi_net *n, u_int32_t var,
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
     case 4:
	if(proto->app_protocol != NDPI_PROTOCOL_UNKNOWN) {
	    var &= ~n->mark[proto->app_protocol].mask;
	    var |=  n->mark[proto->app_protocol].mark;
	    if(proto->master_protocol != NDPI_PROTOCOL_UNKNOWN &&
		proto->master_protocol != proto->app_protocol) {
		var <<= 16;
		var |=  n->mark[proto->master_protocol].mark & n->mark[proto->master_protocol].mask;
	    }
	} else
	  if(proto->master_protocol != NDPI_PROTOCOL_UNKNOWN) {
		var &= ~n->mark[proto->master_protocol].mask;
		var |=  n->mark[proto->master_protocol].mark;
	  }
	break;
    }
    if(_DBG_TRACE_TG3)
	pr_info("target ret %08x mode %d m:%d a:%d \n",var,mode,
			proto->master_protocol,proto->app_protocol);
    return var;
}

static unsigned int
ndpi_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct xt_ndpi_tginfo *info = par->targinfo;
	ndpi_protocol proto = NDPI_PROTOCOL_NULL;
	struct ndpi_net *n = ndpi_pernet(xt_net(par));
	struct ndpi_cb *c_proto;
	int mode = 0;

	/* ndpi_net_init not completed or ndpi_net_exit started */
	if(!atomic_read(&n->ndpi_ready)) return XT_CONTINUE;
	if(!read_trylock(&n->ndpi_busy)) return XT_CONTINUE;

	c_proto = skb_get_cproto(skb);

	if(c_proto->magic != NDPI_ID) {
		read_unlock(&n->ndpi_busy);
		if(_DBG_TRACE_TG1)
			pr_info("target %s no ndpi magic\n",n->ns_name);
		return XT_CONTINUE;
	}

	if(ndpi_enable_flow && info->flow_yes) {
	    if(_DBG_TRACE_TG2)
		pr_info("target %s flow_yes=1 flow_nat=%u\n",
			 n->ns_name,ct_proto_get_flow_nat(c_proto));
	    do {
		enum ip_conntrack_info ctinfo;
		struct nf_conn * ct = NULL;
		struct nf_ct_ext_ndpi *ct_ndpi;
		char ct_buf[128];
		int ct_dir;

//		if(ct_proto_get_flow_nat(c_proto)) break;
		ct = nf_ct_get (skb, &ctinfo);
		if(!ct) break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
		if (nf_ct_is_untracked(ct)) break;
#else
		if(ctinfo == IP_CT_UNTRACKED) break;
#endif
		ct_dir = CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL;
		ct_ndpi = nf_ct_ext_find_ndpi(ct);
		if(_DBG_TRACE_PKT)
		    packet_trace(skb,ct,ct_ndpi,ct_dir,"target    pkt",NULL);
		if(!ct_ndpi) break;
		{
		    bool flow_add = false, nat_start = false;

		    spin_lock_bh (&ct_ndpi->lock);

		    flow_add = ndpi_ct_list_add(n,ct_ndpi);

		    if(!test_nat_done(ct_ndpi) &&  // atomic
		       !ct_proto_get_flow_nat(c_proto)) { // atomic
			    ct_proto_set_flow_nat(c_proto,FLOW_NAT_START); // atomic
			    nat_start = true;
		    }
		    spin_unlock_bh (&ct_ndpi->lock);
		    if(_DBG_TRACE_TG3 || _DBG_TRACE_NAT)
			pr_info("target START ct_ndpi %8p ct %8p %s%s%s\n",
				(void *)ct_ndpi, (void *)ct, ct_info(ct,ct_buf,sizeof(ct_buf),
					CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL),
				flow_add ? " flow_add":"", nat_start ? " nat_start":"");
		}
	    } while(0);
	}
	read_unlock(&n->ndpi_busy);

	if(c_proto->proto != NDPI_PROCESS_ERROR) {
		uint32_t tmp_p = READ_ONCE(c_proto->proto);
		/* see pack_proto() */
		proto.master_protocol = tmp_p & 0xffff;
		proto.app_protocol = (tmp_p >> 16) & 0xffff;
		if(_DBG_TRACE_TG3)
		    pr_info("target           skb %8p m:%d a:%d\n",
				(void *)skb, proto.master_protocol,proto.app_protocol);
	}

	if(info->t_mark || info->t_clsf) {
		if(info->m_proto_id) mode |= 1;
		if(info->p_proto_id) mode |= 2;
		if(info->any_proto_id) mode |= 3;
	}
	if(info->t_mark2)
		skb->mark = ndpi_proto_markmask(n,0,&proto,4,info);
	  else if(info->t_mark)
			skb->mark = ndpi_proto_markmask(n,skb->mark,&proto,mode,info);

	if(info->t_clsf)
		skb->priority =	ndpi_proto_markmask(n,skb->priority,&proto,mode,info);

 return info->t_accept ? NF_ACCEPT : XT_CONTINUE;
}

static int
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

static void
ndpi_tg_destroy (const struct xt_tgdtor_param *par)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
	nf_ct_netns_put(par->net, par->family);
#endif
	nf_ct_l3proto_module_put (par->family);
}

#if  LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
static unsigned int ndpi_nat_do_chain(const struct nf_hook_ops *ops,
                                         struct sk_buff *skb,
                                         const struct net_device *in,
                                         const struct net_device *out,
                                         int (*okfn)(struct sk_buff *))
{
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0)
static unsigned int ndpi_nat_do_chain(const struct nf_hook_ops *priv,
                                         struct sk_buff *skb,
                                         const struct nf_hook_state *state)
{
#else
static unsigned int ndpi_nat_do_chain(void *priv,
					 struct sk_buff *skb,
					 const struct nf_hook_state *state)
{
#endif
    struct nf_conn * ct = NULL;
    enum ip_conntrack_info ctinfo = IP_CT_UNTRACKED;
    struct nf_ct_ext_ndpi *ct_ndpi=NULL;
    struct ndpi_cb *c_proto;
    const char *nat_info = "skip";
    int ct_dir = 0;

    do {

	c_proto = skb_get_cproto(skb);

	if(_DBG_TRACE_NAT && 0)
	    packet_trace(skb,ct,ct_ndpi, 0 /* ! */, "target nat start","magic %d nat start %d",
			    c_proto->magic == NDPI_ID,
			    ct_proto_get_flow_nat(c_proto));
	if(c_proto->magic != NDPI_ID) break;
	if(c_proto->proto == NDPI_PROCESS_ERROR) break;
	if(ct_proto_get_flow_nat(c_proto) != FLOW_NAT_START) break;

	nat_info = "untracked";
	ct = nf_ct_get (skb, &ctinfo);
	if (ct == NULL) break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
	if (nf_ct_is_untracked(ct)) break;
#else
	if(ctinfo == IP_CT_UNTRACKED) break;
#endif
	ct_dir = CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL;
	ct_ndpi = nf_ct_ext_find_ndpi(ct);
	if( !ct_ndpi ) break;
	spin_lock_bh (&ct_ndpi->lock);
	if(!test_nat_done(ct_ndpi)) {
		nat_info = "check";
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
		if(ops->hooknum != NF_INET_PRE_ROUTING)
#else
		if(state->hook != NF_INET_PRE_ROUTING)
#endif
		{
			ndpi_nat_detect(ct_ndpi,ct);
			ct_proto_set_flow_nat(c_proto,FLOW_NAT_END);
			set_nat_done(ct_ndpi);
			nat_info = "done";
		}
	} else nat_info = "NO";
	spin_unlock_bh (&ct_ndpi->lock);
    } while(0);
    if(_DBG_TRACE_NAT && ct_ndpi)
	packet_trace(skb,ct,ct_ndpi,ct_dir,"target nat chain"," %s",nat_info);

    return NF_ACCEPT;
}

static struct xt_match
ndpi_mt_reg __read_mostly = {
	.name = "ndpi",
	.revision = 1,
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
        .revision       = 1,
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
static void bt_port_gc(struct timer_list *t) {
	struct ndpi_net *n = from_timer(n, t, gc);
#else
static void bt_port_gc(unsigned long data) {
        struct ndpi_net *n = (struct ndpi_net *)data;
#endif
        struct ndpi_detection_module_struct *ndpi_struct = n->ndpi_struct;
	time64_t tm;
	uint32_t now32;
	int i;
	uint32_t st_j;
	uint32_t en_j;

	/* ndpi_net_init not completed or ndpi_net_exit started */
	if(!atomic_read(&n->ndpi_ready)) return;
	/* ndpi_net_exit started */
	if(!read_trylock(&n->ndpi_busy)) return;

	st_j = READ_ONCE(jiffies);
	tm=ktime_get_real_seconds();
	now32 = (uint32_t)tm; // BUG AFTER YAER 2105
	{
	    struct hash_ip4p_table *ht = READ_ONCE(ndpi_struct->bt_ht);
	    if(ht) {
		/* full period 64 seconds */
		for(i=0; i < ht->size/128;i++) {
		    if(n->gc_index < 0 ) n->gc_index = 0;
		    if(n->gc_index >= ht->size-1) n->gc_index = 0;

		    if(ht->tbl[n->gc_index].len)
			n->gc_count += ndpi_bittorrent_gc(ht,n->gc_index,now32);
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
			n->gc_count += ndpi_bittorrent_gc(ht6,n->gc_index6,now32);
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
			n->acc_gc = jiffies + 5*HZ;
		    }
		}
	    }
	}
	en_j = READ_ONCE(jiffies);
	barrier();

	if(en_j > st_j+1 && flow_read_debug)
		pr_info("%s: FLOW jiffies %u\n",__func__,en_j - st_j);

	if(atomic_read(&n->ndpi_ready))
		mod_timer(&n->gc,jiffies + HZ/2);

	read_unlock(&n->ndpi_busy);
}

int inet_ntop_port(int family,void *ip, u_int16_t port, char *lbuf, size_t bufsize) {
return  family == AF_INET6 ?
		snprintf(lbuf,bufsize-1, "%pI6c.%d",ip, htons(port))
	      :	snprintf(lbuf,bufsize-1, "%pI4n:%d",ip,htons(port));
}

AC_ERROR_t ac_automata_add_exact(AC_AUTOMATA_t *thiz, AC_PATTERN_t *ac_pattern) {

	if( ac_pattern->astring[0] == '|') {
		ac_pattern->astring++;
		ac_pattern->length --;
		ac_pattern->rep.from_start = 1;
	}
	if( ac_pattern->length > 4 &&
	    ac_pattern->astring[ac_pattern->length-1] == '|') {
		ac_pattern->length --;
		ac_pattern->rep.at_end = 1;
	}
	return ac_automata_add(thiz,ac_pattern);
}

static int ninfo_proc_open(struct inode *inode, struct file *file)
{
        return 0;
}

static int ninfo_proc_close(struct inode *inode, struct file *file)
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
	struct nf_ct_ext_ndpi *ct_ndpi,*next,*prev;
	int i2 = 0, del,skip_del, needed_unlock = 1;

	if(!ndpi_enable_flow) return 0;

	if(!mutex_trylock(&n->rem_lock)) return -1;

	if(!atomic_read(&n->ndpi_ready)) all = 3;

	skip_del = all != 2 ? 0 : n->acc_limit*3/4;

	if(flow_read_debug) pr_info("%s:%s all=%d rem %d skip_del %d\n",
			__func__,n->ns_name,all,atomic_read(&n->acc_rem),skip_del);

	next = prev = NULL;

	spin_lock_bh(&lock_flist);
	ct_ndpi = READ_ONCE(n->flow_h);

	while(ct_ndpi) {

		if(!spin_trylock_bh(&ct_ndpi->lock)) {
			// skip locked flow
			if(needed_unlock) {
				needed_unlock = 0;
				spin_unlock_bh(&lock_flist);
			}
			prev = ct_ndpi;
			ct_ndpi = READ_ONCE(ct_ndpi->next);
			continue;
		}

		next = READ_ONCE(ct_ndpi->next);
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
		if(needed_unlock && !del) {
			needed_unlock = 0;
			spin_unlock_bh(&lock_flist);
		}
		if(del) {
			if(!prev) { // first element
				WRITE_ONCE(n->flow_h,next);
			} else
			    if(cmpxchg(&prev->next,ct_ndpi,next) != ct_ndpi) BUG();
		}
		spin_unlock_bh(&ct_ndpi->lock);

		if(del) {
			// if nflow_read() is active
			if(n->flow_l == ct_ndpi) n->flow_l = prev;

			ndpi_free_ct_flow(ct_ndpi);

			ndpi_free_ct_proto(ct_ndpi);
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
		} else 
			prev = ct_ndpi;

		ct_ndpi=next;
	}
	if(needed_unlock)
		spin_unlock_bh(&lock_flist);

	mutex_unlock(&n->rem_lock);
	if( (all > 1 || i2) && flow_read_debug)
		pr_info("%s:%s Delete %d flows. Active %d, rem %d\n",__func__,n->ns_name,
			i2, atomic_read(&n->acc_work), atomic_read(&n->acc_rem));

	return i2;
}

static void ndpi_ct_counter_save(struct nf_ct_ext_ndpi *ct) {
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
	struct nf_ct_ext_ndpi *ct_ndpi,*next,*prev;
	int p,del,r,needed_unlock;
	ssize_t sl=0;
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
		sl = n->acc_read_mode >= ACC_READ_BIN ?
			ndpi_dump_start_rec(n->str_buf,NF_STR_LBUF,n->acc_open_time):
			snprintf(n->str_buf,NF_STR_LBUF-1,"TIME %llu\n",n->acc_open_time);

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
			sl = n->acc_read_mode >= ACC_READ_BIN ?
				ndpi_dump_lost_rec(n->str_buf,NF_STR_LBUF,cpi,cpo,cbi,cbo) :
				snprintf(n->str_buf,NF_STR_LBUF-1,
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
	needed_unlock = 0;

	if(!n->flow_l) { // start read list
		spin_lock_bh(&lock_flist);
		needed_unlock = 1;
		ct_ndpi = READ_ONCE(n->flow_h);
	} else { // continue read list
		prev = n->flow_l;
		ct_ndpi = prev->next;
	}
	while(ct_ndpi) {

		n->cnt_view++;
		spin_lock_bh(&ct_ndpi->lock);

		next = READ_ONCE(ct_ndpi->next);

		del  = test_for_delete(ct_ndpi);

		if(needed_unlock && !del) {
			needed_unlock = 0;
			spin_unlock_bh(&lock_flist);
		}

		n->str_buf_len = 0; n->str_buf_offs = 0;

		switch(get_acc_mode(n)) {
		case ACC_READ_NORMAL:
			sl = flow_have_info(ct_ndpi) ?
				ndpi_dump_acct_info(n,ct_ndpi) : 0;
			break;
		case ACC_READ_CLOSED:
			sl = del && flow_have_info(ct_ndpi) ?
				ndpi_dump_acct_info(n,ct_ndpi) : 0;
			break;
		case ACC_READ_MONITOR:
			sl = !del && test_flow_yes(ct_ndpi) ?
				ndpi_dump_acct_info(n,ct_ndpi) : 0;
			break;
		default:
			sl = 0;
		}
		if(sl && get_acc_mode(n) != ACC_READ_MONITOR)
			ndpi_ct_counter_save(ct_ndpi);

		if(del) {
			if(!prev) { // first element
				WRITE_ONCE(n->flow_h,next);
			} else
				if(cmpxchg(&prev->next,ct_ndpi,next) != ct_ndpi) BUG();
		}
		spin_unlock_bh(&ct_ndpi->lock);

		if(del) {
			ndpi_free_ct_proto(ct_ndpi);
			kmem_cache_free (ct_info_cache, ct_ndpi);
			atomic_dec(&n->acc_work);
			atomic_dec(&n->acc_rem);
			n->cnt_del++;
		} else {
			prev = ct_ndpi;
			n->flow_l = prev;
		}
		ct_ndpi=next;

		if(sl) {
			r = nflow_put_str(n,buf,&p,&count,ppos);
			if(r != 0) {
				n->flow_l = prev;
				if(r < 0 && count != 0) {
					p = -EFAULT;
					pr_info("%s:%s cond4 p %d count %zd\n",
						__func__, n->ns_name, p, count);
				}
				break;
			}
			sl = 0;
			n->cnt_out++;
		}
	}
	if(needed_unlock)
		spin_unlock_bh(&lock_flist);

	if(!ct_ndpi ) {
		n->acc_end = 1;
		n->flow_l = NULL;
		if(flow_read_debug)
		  pr_info("%s:%s End   dump: CT total %d deleted %d; view %ld dumped %ld removed %ld\n",
			__func__, n->ns_name, atomic_read(&n->acc_work),atomic_read(&n->acc_rem),
			n->cnt_view, n->cnt_out, n->cnt_del);
	} else {
		if(p < 0) pr_info("%s:%s pos %7lld ERROR %d\n",
				__func__, n->ns_name, st_pos, p);
		if(flow_read_debug > 1 || p < 0)
			pr_info("%s:%s pos %7lld view %ld dumped %ld deleted %ld\n",
				__func__, n->ns_name, st_pos, n->cnt_view, n->cnt_out, n->cnt_del);
	}

	n->acc_gc = jiffies + n->acc_wait * HZ;

	return p;
}

static const char *__acerr2txt[] = {
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

int str_coll_to_automata(struct ndpi_detection_module_struct *ndpi_str,
		void *host_ac,hosts_str_t *hosts) {
str_collect_t *ph;
int np,nh,err=0;

    for(np = 0; np < NDPI_NUM_BITS; np++) {
        ph = hosts->p[np];
        if(!ph) continue;
        for(nh = 0 ; nh < ph->last && ph->s[nh] ; nh += (uint8_t)ph->s[nh] + 2) {
	    if(ndpi_string_to_automa(ndpi_str,(AC_AUTOMATA_t *)host_ac,
			&ph->s[nh+1], np,0,0,0,1) < 0) {
		err++;
		pr_err("%s: error add %s\n",__func__,&ph->s[nh+1]);
	    }
        }
    }
    if(!err) ac_automata_finalize((AC_AUTOMATA_t*)host_ac);
    return err;
}

#if  LINUX_VERSION_CODE < KERNEL_VERSION(5,6,0)
#define PROC_OPS(s,o,r,w,l,d) static const struct file_operations s = { \
        .open    = o , \
        .read    = r , \
        .write   = w , \
	.llseek  = l , \
	.release = d \
}
#else
  #if  LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
    #define PROC_OPS(s,o,r,w,l,d) static const struct proc_ops s = { \
        .proc_open    = o , \
        .proc_read    = r , \
        .proc_write   = w , \
	.proc_lseek   = l , \
	.proc_release = d  \
    }
  #else
    #define PROC_OPS(s,o,r,w,l,d) static const struct proc_ops s = { \
        .proc_open    = o , \
        .proc_read    = r , \
        .proc_write   = w , \
	.proc_release = d \
    }
  #endif
#endif
PROC_OPS(nproto_proc_fops, ninfo_proc_open,nproto_proc_read,nproto_proc_write,noop_llseek,nproto_proc_close);
PROC_OPS(ndebug_proc_fops, ninfo_proc_open,ndebug_proc_read,ndebug_proc_write,noop_llseek,ndebug_proc_close);
PROC_OPS(nrisk_proc_fops, nrisk_proc_open,nrisk_proc_read,nrisk_proc_write,noop_llseek,nrisk_proc_close);
PROC_OPS(ncfg_proc_fops,  ncfg_proc_open, ncfg_proc_read, ncfg_proc_write, noop_llseek,ncfg_proc_close);
PROC_OPS(ninfo_proc_fops, ninfo_proc_open,ninfo_proc_read,ninfo_proc_write,noop_llseek,ninfo_proc_close);
PROC_OPS(nflow_proc_fops, nflow_proc_open,nflow_proc_read,nflow_proc_write,nflow_proc_llseek,nflow_proc_close);

#ifdef NDPI_DETECTION_SUPPORT_IPV6
PROC_OPS(ninfo6_proc_fops, ninfo_proc_open,ninfo6_proc_read,ninfo_proc_write,noop_llseek,ninfo_proc_close);
#endif

#ifdef BT_ANNOUNCE
PROC_OPS(nann_proc_fops, ninfo_proc_open,nann_proc_read,NULL,noop_llseek,ninfo_proc_close);
#endif

PROC_OPS(n_ipdef_proc_fops, n_ipdef_proc_open, n_ipdef_proc_read, n_ipdef_proc_write,noop_llseek,n_ipdef_proc_close);
PROC_OPS(n_ip6def_proc_fops, n_ipdef_proc_open, n_ip6def_proc_read, n_ip6def_proc_write,noop_llseek,n_ip6def_proc_close);
PROC_OPS(n_hostdef_proc_fops,n_hostdef_proc_open,n_hostdef_proc_read,n_hostdef_proc_write,noop_llseek,n_hostdef_proc_close);
static struct nf_hook_ops nf_nat_ipv4_ops[] = {
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

static void __net_exit ndpi_net_exit(struct net *net)
{
	struct ndpi_net *n;

	n = ndpi_pernet(net);
	if(_DBG_TRACE_NETNS)
		pr_info("%s:%s\n",__func__,n->ns_name);

	/* stop processing received packets */
	atomic_set(&n->ndpi_ready,0);

	if(ndpi_enable_flow) {
	    nf_unregister_net_hooks(net, nf_nat_ipv4_ops,
			      ARRAY_SIZE(nf_nat_ipv4_ops));
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	del_timer(&n->gc);
#else
	del_timer_sync(&n->gc);
#endif

	/* wait for the ndpi library code to finish processing packets */
	write_lock(&n->ndpi_busy);

#ifndef NF_CT_CUSTOM
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
	net->ct.label_words = n->labels_word;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(6, 7, 0)
	net->ct.labels_used--;
#else
	atomic_dec(&net->ct.labels_used);
#endif
#endif

#if   LINUX_VERSION_CODE >= KERNEL_VERSION(5, 19, 0)
	struct nf_ct_iter_data iter_data = {
		.net    = net,
		.data   = n,
		.portid = 0,
		.report = 0
	};
	nf_ct_iterate_cleanup_net(ndpi_cleanup_flow, &iter_data);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
	nf_ct_iterate_cleanup_net(net, ndpi_cleanup_flow, n, 0 ,0);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0)
	nf_ct_iterate_cleanup(net, ndpi_cleanup_flow, n, 0 ,0);
#else /* < 3.12 */
	nf_ct_iterate_cleanup(net, ndpi_cleanup_flow, n);
#endif

	write_unlock(&n->ndpi_busy);

	if(ndpi_enable_flow) {
		while(ndpi_delete_acct(n,3) == -1)
			msleep_interruptible(1);
	}

	str_hosts_done(n->hosts);
	kfree(n->str_buf);

	ndpi_exit_detection_module(n->ndpi_struct);
#ifdef USE_GLOBAL_CONTEXT
	if(n->g_ctx)
		kfree(n->g_ctx);
#endif

	if(n->risk_names)
		kfree(n->risk_names);

	if(n->pde) {
		if(n->pe_ipdef)
			remove_proc_entry(ipdef_name, n->pde);
		if(n->pe_hostdef)
			remove_proc_entry(hostdef_name, n->pde);
		if(n->pe_info)
			remove_proc_entry(info_name, n->pde);
		if(n->pe_debug)
			remove_proc_entry(debug_name, n->pde);
		if(n->pe_risk)
			remove_proc_entry(risk_name, n->pde);
		if(n->pe_cfg)
			remove_proc_entry(cfg_name, n->pde);
		if(n->pe_proto)
			remove_proc_entry(proto_name, n->pde);
		if(n->pe_flow)
			remove_proc_entry(flow_name, n->pde);
#ifdef BT_ANNOUNCE
		if(n->pe_ann)
			remove_proc_entry(ann_name, n->pde);
#endif
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		if(n->pe_ip6def)
			remove_proc_entry(ip6def_name, n->pde);
		if(n->pe_info6)
			remove_proc_entry(info6_name, n->pde);
#endif
		PROC_REMOVE(n->pde,net);
	}
}

NDPI_STATIC int ndpi_stun_cache_enable=
#ifndef __KERNEL__
	1;
#else
	0;
#endif

static int __net_init ndpi_net_init(struct net *net)
{
	struct ndpi_net *n;
	struct ndpi_global_context *g_ctx = NULL;
	int i;

	/* init global detection structure */

	n = ndpi_pernet(net);
	snprintf(n->ns_name,sizeof(n->ns_name)-1,"ns%d",net_ns_id);

	rwlock_init(&n->ndpi_busy);
	atomic_set(&n->ndpi_ready,0);

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
	n->w_buff[W_BUF_DEBUG] = NULL;
	n->w_buff[W_BUF_RISK] = NULL;

	n->host_ac = NULL;
	n->hosts = str_hosts_alloc();
	n->hosts_tmp = NULL;
	n->host_error = 0;

	parse_ndpi_proto(n,"init");

	n->str_buf = kmalloc(NF_STR_LBUF,GFP_KERNEL);
	if (n->str_buf == NULL) {
		str_hosts_done(n->hosts);
		pr_err("xt_ndpi: alloc str_buf failed\n");
                return -ENOMEM;
	}
	ndpi_stun_cache_enable = ndpi_stun_cache_opt;
	ndpi_debug_print_init = debug_printf;
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
	ndpi_debug_level_init = ndpi_lib_trace;
#endif
	/* init global detection structure */
#ifdef USE_GLOBAL_CONTEXT
	n->g_ctx = ndpi_calloc(1, sizeof(struct ndpi_global_context));
	if(!n->g_ctx) {
		kfree(n->str_buf);
		str_hosts_done(n->hosts);
		pr_err("xt_ndpi: alloc global context failed\n");
		return -ENOMEM;
	}
#endif
	n->ndpi_struct = ndpi_init_detection_module(g_ctx);
	if (n->ndpi_struct == NULL) {
		pr_err("xt_ndpi: global structure initialization failed.\n");
                return -ENOMEM;
	}
	n->flow_h = NULL;
	n->ndpi_struct->cfg.direction_detect_enabled = 0;
	n->ndpi_struct->cfg.stun_cache_num_entries = ndpi_stun_cache_enable ? 1024:0;
	n->ndpi_struct->cfg.ookla_cache_num_entries = 0;
	/* enable all protocols */
	NDPI_BITMASK_SET_ALL(n->protocols_bitmask);
	ndpi_set_protocol_detection_bitmask2(n->ndpi_struct, &n->protocols_bitmask);
	n->ndpi_struct->user_data = n;
	for (i = 0; i < NDPI_NUM_BITS; i++) {
                atomic64_set (&n->protocols_cnt[i], 0);
        	n->debug_level[i] = 0;
		if(i < NDPI_LAST_IMPLEMENTED_PROTOCOL) continue;
		n->mark[i].mark = n->mark[i].mask = 0;
        }
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
	pr_info("ndpi_lib_trace %s level %d\n",ndpi_lib_trace ? "Enabled":"Disabled",(int)ndpi_lib_trace);
	n->ndpi_struct->cfg.log_level = ndpi_lib_trace;
	set_ndpi_debug_function(n->ndpi_struct, ndpi_lib_trace ? debug_printf:NULL);
#endif
	if(tls_buf_size < 2) tls_buf_size = 2;
	if(tls_buf_size > 16) tls_buf_size = 16;

	n->ndpi_struct->cfg.tls_buf_size_limit = tls_buf_size*1024;

	ndpi_set_config(n->ndpi_struct, "any", "ip_list.load", "1");
	ndpi_set_config(n->ndpi_struct, NULL, "flow_risk_lists.load", "1");
	ndpi_load_ip_lists(n->ndpi_struct);
	ndpi_set_config(n->ndpi_struct, "any", "ip_list.load", "0");
	ndpi_set_config(n->ndpi_struct, NULL, "flow_risk_lists.load", "0");
	ndpi_set_config(n->ndpi_struct, NULL, "tcp_ack_payload_heuristic.load", "1");
	ndpi_init_host_ac(n);

	if(bt_hash_size > 512) bt_hash_size = 512;
	if(bt6_hash_size > 32) bt6_hash_size = 32;
#ifdef BT_ANNOUNCE
	if(bt_log_size > 512) bt_log_size = 512;
	if(bt_log_size < 32 ) bt_log_size = 0;
#else
	bt_log_size = 0;
#endif
	/* ndpi_bittorrent_init(n->ndpi_struct,
			bt_hash_size*1024,bt6_hash_size*1024,
			bt_hash_tmo,bt_log_size); */

	//ndpi_finalize_initialization(n->ndpi_struct);

	n->risk_names_len = risk_names(n,NULL,0);
	n->risk_mask = ~(0ULL);

	n->n_hash = -1;

	/* Create proc files */

	n->pde = proc_mkdir(dir_name, net->proc_net);
	if(!n->pde) {
		ndpi_exit_detection_module(n->ndpi_struct);
		pr_err("xt_ndpi: cant create net/%s\n",dir_name);
		return -ENOMEM;
	}
	do {

		n->pe_info = NULL;
		n->pe_flow = NULL;
		n->pe_proto = NULL;
#ifdef BT_ANNOUNCE
		n->pe_ann = NULL;
#endif
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		n->pe_info6 = NULL;
		n->pe_ip6def = NULL;
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
		n->pe_debug = proc_create_data(debug_name, S_IRUGO | S_IWUSR,
					 n->pde, &ndebug_proc_fops, n);
		if(!n->pe_debug) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,debug_name);
			break;
		}
		n->pe_risk = proc_create_data(risk_name, S_IRUGO,
					 n->pde, &nrisk_proc_fops, n);
		if(!n->pe_risk) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,risk_name);
			break;
		}
		n->pe_cfg = proc_create_data(cfg_name, S_IRUGO,
					 n->pde, &ncfg_proc_fops, n);
		if(!n->pe_cfg) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,cfg_name);
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

		n->pe_ip6def = proc_create_data(ip6def_name, S_IRUGO | S_IWUSR,
					 n->pde, &n_ip6def_proc_fops, n);
		if(!n->pe_ip6def) {
			pr_err("xt_ndpi: cant create net/%s/%s\n",dir_name,ip6def_name);
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
#elif LINUX_VERSION_CODE < KERNEL_VERSION(6, 7, 0)
		net->ct.labels_used++;
#else
		atomic_inc(&net->ct.labels_used);
#endif
#endif
		if( ndpi_enable_flow &&
		    nf_register_net_hooks(net, nf_nat_ipv4_ops,
	                                   ARRAY_SIZE(nf_nat_ipv4_ops))) break;
		/* All success! */
		atomic_set(&n->ndpi_ready,1);
		net_ns_id++;
		if(_DBG_TRACE_NETNS)
			pr_info("%s:%s OK\n",__func__,n->ns_name);
		return 0;
	} while(0);

/* rollback procfs on error */
	if(n->hosts)
		str_hosts_done(n->hosts);
	if(n->host_ac)
		ac_automata_release(n->host_ac,1);

	if(n->pe_hostdef)
		remove_proc_entry(hostdef_name,n->pde);
	if(n->pe_ipdef)
		remove_proc_entry(ipdef_name,n->pde);
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	if(n->pe_ip6def)
		remove_proc_entry(ip6def_name,n->pde);
	if(n->pe_info6)
		remove_proc_entry(proto_name, n->pde);
#endif
#ifdef BT_ANNOUNCE
	if(n->pe_ann)
		remove_proc_entry(ann_name, n->pde);
#endif
	if(n->pe_risk)
		remove_proc_entry(debug_name,n->pde);
	if(n->pe_debug)
		remove_proc_entry(debug_name,n->pde);
	if(n->pe_proto)
		remove_proc_entry(proto_name,n->pde);
	if(n->pe_info)
		remove_proc_entry(info_name,n->pde);
	if(n->pe_flow)
		remove_proc_entry(flow_name,n->pde);

	PROC_REMOVE(n->pde,net);
	if(n->risk_names)
		kfree(n->risk_names);
	ndpi_exit_detection_module(n->ndpi_struct);
#ifdef USE_GLOBAL_CONTEXT
	if(n->g_ctx)
		kfree(n->g_ctx);
#endif

	return -ENOMEM;
}

#if !defined(USE_LIVEPATCH) && !defined(USE_NF_CONNTRACK_DESTROY_HOOK)
static struct nf_ct_ext_type ndpi_extend = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,8,0)
       .seq_print = seq_print_ndpi,
#endif
       .destroy   = nf_ndpi_free_flow,
       .len    = sizeof(struct nf_ct_ext_labels),
       .align  = __alignof__(uint32_t),
       .id     = 0,
};
#elif !defined(USE_NF_CONNTRACK_DESTROY_HOOK)

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,17,0)
#error "not implemented"
#elif LINUX_VERSION_CODE < KERNEL_VERSION(5,18,13)
#include "livepatch/v5.18.1.c"
#elif LINUX_VERSION_CODE < KERNEL_VERSION(6,0,0)
#include "livepatch/v5.18.13.c"
#else
#include "livepatch/v6.0.c"
#endif

static struct klp_func ndpi_funcs[] = {
	{
		.old_name = "nf_ct_destroy",
		.new_func = ndpi_nf_ct_destroy,
	}, { }
};

static struct klp_object ndpi_objs[] = {
	{
		.name = "nf_conntrack",
		.funcs = ndpi_funcs,
	}, { }
};

static struct klp_patch ndpi_patch = {
	.mod = THIS_MODULE,
	.objs = ndpi_objs,
};

#endif

static struct pernet_operations ndpi_net_ops = {
        .init   = ndpi_net_init,
        .exit   = ndpi_net_exit,
        .id     = &ndpi_net_id,
        .size   = sizeof(struct ndpi_net),
};

static int __init ndpi_mt_init(void)
{
        int ret;

	ndpi_size_flow_struct = ndpi_detection_get_sizeof_ndpi_flow_struct();
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
#elif defined(USE_NF_CONNTRACK_DESTROY_HOOK)
	nf_ct_ext_id_ndpi = NF_CT_EXT_LABELS;
	register_nf_ct_destroy_hook(&nf_ndpi_free_flow);
#else
#ifdef USE_LIVEPATCH
	nf_ct_ext_id_ndpi = NF_CT_EXT_LABELS;
#else
	ndpi_extend.id = nf_ct_ext_id_ndpi = NF_CT_EXT_LABELS;
	nf_ct_extend_unregister(&ndpi_extend);
	ret = nf_ct_extend_register(&ndpi_extend);
	if(ret < 0) {
		pr_err("xt_ndpi: can't nf_ct_extend_register.\n");
		return -EBUSY;
	}
#endif
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


	ndpi_size_hash_ip4p_node=                sizeof(struct hash_ip4p_node)
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		                                 +12
#endif
	;

        bt_port_cache = kmem_cache_create("ndpi_btport",
				ndpi_size_hash_ip4p_node, 0, 0, NULL);
        if (!bt_port_cache) {
		pr_err("xt_ndpi: error creating port cache.\n");
		goto free_flow;
	}
	if(bt_hash_size && bt_hash_size > 512) bt_hash_size = 512;
	if(bt6_hash_size && bt6_hash_size > 32) bt6_hash_size = 32;
	if(!bt_hash_tmo || bt_hash_tmo < 900) bt_hash_tmo = 900;
	if( bt_hash_tmo > 3600) bt_hash_tmo = 3600;

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
		" sizeof hash_ip4p_node=%lu flow_struct=%lu packet_struct=%zu\n"
		"   flow_tcp_struct=%zu flow_udp_struct=%zu int_one_line_struct=%zu\n"
		" ndpi_ip_addr_t=%zu ndpi_protocol=%zu nf_ct_ext_ndpi=%zu\n"
		" flow_info=%zu spinlock_t=%zu "
#ifndef NF_CT_CUSTOM
		" NF_LABEL_ID %d\n",
#else
		" NF_EXT_ID %d\n",
#endif
		NDPI_GIT_RELEASE,
		bt_hash_size, bt_hash_size ? bt_hash_tmo : 0, bt_log_size,
		ndpi_size_hash_ip4p_node,
		ndpi_size_flow_struct,
		sizeof(struct ndpi_packet_struct),
		sizeof(struct ndpi_flow_tcp_struct),
		sizeof(struct ndpi_flow_udp_struct),
		sizeof(struct ndpi_int_one_line_struct),
		sizeof(ndpi_ip_addr_t), sizeof(ndpi_protocol),
		ALIGN(sizeof(struct nf_ct_ext_ndpi),__SIZEOF_LONG__),
		sizeof(struct flow_info),
		sizeof(spinlock_t), nf_ct_ext_id_ndpi);
	pr_info("xt_ndpi: MAX_PROTOCOLS %d LAST_PROTOCOL %d\n",
		NDPI_NUM_BITS,
		NDPI_LAST_IMPLEMENTED_PROTOCOL);
	pr_info("xt_ndpi: flow accounting %s\n",ndpi_enable_flow ? "ON":"OFF");
#ifdef USE_LIVEPATCH
	rcu_assign_pointer(nf_conntrack_destroy_cb,nf_ndpi_free_flow);
	return klp_enable_patch(&ndpi_patch);
#else
	return 0;
#endif

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
#if !defined(USE_LIVEPATCH) && !defined(USE_NF_CONNTRACK_DESTROY_HOOK)
	nf_ct_extend_unregister(&ndpi_extend);
#endif
#if defined(USE_NF_CONNTRACK_DESTROY_HOOK)
	unregister_nf_ct_destroy_hook();
#endif
       	return ret;
}


static void __exit ndpi_mt_exit(void)
{
	xt_unregister_target(&ndpi_tg_reg);
	xt_unregister_match(&ndpi_mt_reg);
	unregister_pernet_subsys(&ndpi_net_ops);
#if !defined(USE_LIVEPATCH) && !defined(USE_NF_CONNTRACK_DESTROY_HOOK)
	nf_ct_extend_unregister(&ndpi_extend);
#elif defined(USE_NF_CONNTRACK_DESTROY_HOOK)
	unregister_nf_ct_destroy_hook();
#else
	rcu_assign_pointer(nf_conntrack_destroy_cb,NULL);
#endif
        kmem_cache_destroy (bt_port_cache);
        kmem_cache_destroy (osdpi_flow_cache);
        kmem_cache_destroy (ct_info_cache);
}


module_init(ndpi_mt_init);
module_exit(ndpi_mt_exit);
#ifdef USE_LIVEPATCH
MODULE_INFO(livepatch, "Y");
#endif

#include "ndpi_strcol.c" 
#include "ndpi_proc_parsers.c" 
#include "ndpi_proc_generic.c"
#include "ndpi_proc_info.c"
#include "ndpi_proc_flow.c" 
#include "ndpi_proc_hostdef.c"
#include "ndpi_proc_ipdef.c"
#include "../libre/regexp.c"
#include "../../src/lib/ndpi_main.c"