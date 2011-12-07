/* 
 * main.c
 * Copyright (C) 2010 Gerardo E. Gidoni <gerel@gnu.org>
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
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
#include <linux/netfilter.h>
#include <linux/netfilter/x_tables.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/if_ether.h>
#include <linux/rbtree.h>
#include <linux/kref.h>
#include <linux/time.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_ecache.h>

#include "ipq_api.h"
#include "xt_opendpi.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gerardo E. Gidoni <gerel@gnu.org>");
MODULE_DESCRIPTION("OpenDPI wrapper");
MODULE_ALIAS("ipt_opendpi");

/* flow tracking */
struct osdpi_flow_node {
        struct rb_node node;
        struct nf_conn * ct;
	/* result only, not used for flow identification */
	u32 detected_protocol;
        /* last pointer assigned at run time */
	struct ipoque_flow_struct *ipoque_flow;
};

/* id tracking */
struct osdpi_id_node {
        struct rb_node node;
        struct kref refcnt;
	union nf_inet_addr ip;
        /* last pointer assigned at run time */
	struct ipoque_id_struct *ipoque_id;
};

static u32 size_id_struct = 0;
static u32 size_flow_struct = 0;

static struct rb_root osdpi_flow_root = RB_ROOT;
static struct rb_root osdpi_id_root = RB_ROOT;

static struct kmem_cache *osdpi_flow_cache __read_mostly;
static struct kmem_cache *osdpi_id_cache __read_mostly;

static IPOQUE_PROTOCOL_BITMASK protocols_bitmask;
static atomic_t protocols_cnt[IPOQUE_MAX_SUPPORTED_PROTOCOLS];

DEFINE_SPINLOCK(flow_lock);
DEFINE_SPINLOCK(id_lock);
DEFINE_SPINLOCK(ipq_lock);


/* detection */
static struct ipoque_detection_module_struct *ipoque_struct = NULL;
static u32 detection_tick_resolution = 1000;


static void debug_printf(u32 protocol, void *id_struct,
                         ipq_log_level_t log_level, const char *format, ...)
{
        /* do nothing */
}


static void *malloc_wrapper(unsigned long size)
{
	return kmalloc(size, GFP_KERNEL);
}


static void free_wrapper(void *freeable)
{
	kfree(freeable);
}

static struct osdpi_flow_node *
opendpi_flow_search(struct rb_root *root, struct nf_conn *ct)
{
        struct osdpi_flow_node *data;
  	struct rb_node *node = root->rb_node;

  	while (node) {
                data = rb_entry(node, struct osdpi_flow_node, node);

		if (ct < data->ct)
  			node = node->rb_left;
		else if (ct > data->ct)
  			node = node->rb_right;
		else
  			return data;
	}

	return NULL;
}


static int
opendpi_flow_insert(struct rb_root *root, struct osdpi_flow_node *data)
{
        struct osdpi_flow_node *this;
  	struct rb_node **new = &(root->rb_node), *parent = NULL;

  	while (*new) {
                this = rb_entry(*new, struct osdpi_flow_node, node);

		parent = *new;
  		if (data->ct < this->ct)
  			new = &((*new)->rb_left);
  		else if (data->ct > this->ct)
  			new = &((*new)->rb_right);
  		else
  			return 0;
  	}
  	rb_link_node(&data->node, parent, new);
  	rb_insert_color(&data->node, root);

	return 1;
}


static struct osdpi_id_node *
opendpi_id_search(struct rb_root *root, union nf_inet_addr *ip)
{
        int res;
        struct osdpi_id_node *data;
  	struct rb_node *node = root->rb_node;

  	while (node) {
                data = rb_entry(node, struct osdpi_id_node, node);
		res = memcmp(ip, &data->ip, sizeof(union nf_inet_addr));

		if (res < 0)
  			node = node->rb_left;
		else if (res > 0)
  			node = node->rb_right;
		else
  			return data;
	}

	return NULL;
}


static int
opendpi_id_insert(struct rb_root *root, struct osdpi_id_node *data)
{
        int res;
        struct osdpi_id_node *this;
  	struct rb_node **new = &(root->rb_node), *parent = NULL;

  	while (*new) {
                this = rb_entry(*new, struct osdpi_id_node, node);
		res = memcmp(&data->ip, &this->ip, sizeof(union nf_inet_addr));

		parent = *new;
  		if (res < 0)
  			new = &((*new)->rb_left);
  		else if (res > 0)
  			new = &((*new)->rb_right);
  		else
  			return 0;
  	}
  	rb_link_node(&data->node, parent, new);
  	rb_insert_color(&data->node, root);

	return 1;
}


static void
opendpi_id_release(struct kref *kref)
{
        struct osdpi_id_node * id;

        id = container_of (kref, struct osdpi_id_node, refcnt);
        rb_erase(&id->node, &osdpi_id_root);
        kmem_cache_free (osdpi_id_cache, id);
}


static struct osdpi_flow_node *
opendpi_alloc_flow (struct nf_conn * ct)
{
        struct osdpi_flow_node *flow;

        spin_lock_bh (&flow_lock);
        flow = opendpi_flow_search (&osdpi_flow_root, ct);
        if (flow != NULL){
                spin_unlock_bh (&flow_lock);
                return flow;
        }
        flow = kmem_cache_zalloc (osdpi_flow_cache, GFP_ATOMIC);
        if (flow == NULL){
                pr_err("xt_opendpi: couldn't allocate new flow.\n");
                spin_unlock_bh (&flow_lock);
                return NULL;
        }
        flow->ct = ct;
        flow->ipoque_flow = (struct ipoque_flow_struct *)
                ((char*)&flow->ipoque_flow+sizeof(flow->ipoque_flow));
        opendpi_flow_insert (&osdpi_flow_root, flow);
        spin_unlock_bh (&flow_lock);

        return flow;
}


static void
opendpi_free_flow (struct nf_conn * ct)
{
        struct osdpi_flow_node * flow;

        spin_lock_bh (&flow_lock);
        flow = opendpi_flow_search (&osdpi_flow_root, ct);
        if (flow != NULL){
                rb_erase (&flow->node, &osdpi_flow_root);
                kmem_cache_free (osdpi_flow_cache, flow);
        }
        spin_unlock_bh (&flow_lock);
}


static struct osdpi_id_node *
opendpi_alloc_id (union nf_inet_addr * ip)
{
        struct osdpi_id_node *id;

        spin_lock_bh (&id_lock);
        id = opendpi_id_search (&osdpi_id_root, ip);
        if (id != NULL){
                kref_get (&id->refcnt);
        }else{
                id = kmem_cache_zalloc (osdpi_id_cache, GFP_ATOMIC);

                if (id == NULL){
                        pr_err("xt_opendpi: couldn't allocate new id.\n");
                        spin_unlock_bh (&id_lock);
                        return NULL;
                }
                memcpy(&id->ip, ip, sizeof(union nf_inet_addr));
                id->ipoque_id = (struct ipoque_id_struct *)
                        ((char*)&id->ipoque_id+sizeof(id->ipoque_id));
                kref_init (&id->refcnt);
                opendpi_id_insert (&osdpi_id_root, id);
        }
        spin_unlock_bh (&id_lock);

        return id;
}


static void
opendpi_free_id (union nf_inet_addr * ip)
{
        struct osdpi_id_node *id;

        spin_lock_bh (&id_lock);
        id = opendpi_id_search (&osdpi_id_root, ip);
        if (id != NULL)
                kref_put (&id->refcnt, opendpi_id_release);
        spin_unlock_bh (&id_lock);
}


static void
opendpi_enable_protocols (const struct xt_opendpi_mtinfo*info)
{
        int i;

        for (i = 1; i <= IPOQUE_MAX_SUPPORTED_PROTOCOLS; i++){
                if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0){
                        spin_lock_bh (&ipq_lock);
                        atomic_inc(&protocols_cnt[i-1]);
                        IPOQUE_ADD_PROTOCOL_TO_BITMASK(protocols_bitmask, i);
                        ipoque_set_protocol_detection_bitmask2
                                (ipoque_struct,&protocols_bitmask);
                        spin_unlock_bh (&ipq_lock);
                }
        }
}


static void
opendpi_disable_protocols (const struct xt_opendpi_mtinfo*info)
{
        int i;

        for (i = 1; i <= IPOQUE_MAX_SUPPORTED_PROTOCOLS; i++){
                if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0){
                        spin_lock_bh (&ipq_lock);
                        if (atomic_dec_and_test(&protocols_cnt[i-1])){
                                IPOQUE_DEL_PROTOCOL_FROM_BITMASK(protocols_bitmask, i);
                                ipoque_set_protocol_detection_bitmask2
                                        (ipoque_struct, &protocols_bitmask);
                        }
                        spin_unlock_bh (&ipq_lock);
                }
        }
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
static int
opendpi_conntrack_event (struct notifier_block *this, unsigned long ev,
                         void * data)
{
        struct nf_conn * ct = (struct nf_conn *) data;
        union nf_inet_addr *src, *dst;

        if (ct == &nf_conntrack_untracked)
                return NOTIFY_DONE;

        if (ev & IPCT_DESTROY){
                src = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3;
                dst = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3;

                opendpi_free_id (src);
                opendpi_free_id (dst);
                opendpi_free_flow (ct);
        }

        return NOTIFY_DONE;
}

static struct notifier_block
osdpi_notifier = {
        .notifier_call = opendpi_conntrack_event,
};

#else
static int
opendpi_conntrack_event(unsigned int events, struct nf_ct_event *item)
{
        struct nf_conn * ct = item->ct;
        union nf_inet_addr *src, *dst;

        if (ct == &nf_conntrack_untracked)
                return 0;

        if (events & (1 << IPCT_DESTROY)){
                src = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3;
                dst = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3;

                opendpi_free_id (src);
                opendpi_free_id (dst);
                opendpi_free_flow (ct);
        }

        return 0;
}

static struct nf_ct_event_notifier
osdpi_notifier = {
        .fcn = opendpi_conntrack_event,
};

#endif



static u32
opendpi_process_packet(struct nf_conn * ct, const uint64_t time,
                       const struct iphdr *iph, uint16_t ipsize)
{
	u32 proto = IPOQUE_PROTOCOL_UNKNOWN;
        union nf_inet_addr *ipsrc, *ipdst;
        struct osdpi_id_node *src, *dst;
        struct osdpi_flow_node * flow;

        spin_lock_bh (&flow_lock);
        flow = opendpi_flow_search (&osdpi_flow_root, ct);
        spin_unlock_bh (&flow_lock);
        if (flow == NULL){
                flow = opendpi_alloc_flow(ct);
                if (flow == NULL)
                        return proto;
        }

        ipsrc = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3;

        spin_lock_bh (&id_lock);
        src = opendpi_id_search (&osdpi_id_root, ipsrc);
        spin_unlock_bh (&id_lock);
	if (src == NULL) {
                src = opendpi_alloc_id(ipsrc);
                if (src == NULL)
                        return proto;
	}

        ipdst = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3;

        spin_lock_bh (&id_lock);
        dst = opendpi_id_search (&osdpi_id_root, ipdst);
        spin_unlock_bh (&id_lock);
	if (dst == NULL) {
                dst = opendpi_alloc_id(ipdst);
                if (dst == NULL)
                        return proto;
	}

        /* here the actual detection is performed */
        spin_lock_bh (&ipq_lock);
        proto = ipoque_detection_process_packet(ipoque_struct,flow->ipoque_flow,
                                                (uint8_t *) iph, ipsize, time,
                                                src->ipoque_id, dst->ipoque_id);
        flow->detected_protocol = proto;
        spin_unlock_bh (&ipq_lock);

	return proto;
}



#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
static bool 
opendpi_mt (const struct sk_buff *skb,
            const struct net_device *in,
            const struct net_device *out,
            const struct xt_match *match,
            const void *matchinfo,
            int offset,
            unsigned int protoff,
            bool *hotdrop)

#else
static bool
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
opendpi_mt(const struct sk_buff *skb, const struct xt_match_param *par)
#else
opendpi_mt(const struct sk_buff *skb, struct xt_action_param *par)
#endif
#endif
{
	u32 proto;
	u64 time;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
	const struct xt_opendpi_mtinfo *info = matchinfo;
#else
	const struct xt_opendpi_mtinfo *info = par->matchinfo;
#endif

	enum ip_conntrack_info ctinfo;
	struct nf_conn * ct;
	struct timeval tv;
	struct sk_buff *linearized_skb = NULL;
	const struct sk_buff *skb_use = NULL;


	if (skb_is_nonlinear(skb)){
		linearized_skb = skb_copy(skb, GFP_ATOMIC);
		if (linearized_skb == NULL) {
			pr_info ("xt_opendpi: linearization failed.\n");
			return false;
		}
		skb_use = linearized_skb;
	} else {
		skb_use = skb;
	}

	ct = nf_ct_get (skb_use, &ctinfo);

	if (ct == NULL){

		if(linearized_skb != NULL){
			kfree_skb(linearized_skb);
		}

		return false;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	if (nf_ct_is_untracked(skb))
#else
	if (nf_ct_is_untracked(ct))
#endif	
	{
		pr_info ("xt_opendpi: ignoring untracked sk_buff.\n");
		return false;               
	}

	do_gettimeofday(&tv);

	time = ((uint64_t) tv.tv_sec) * detection_tick_resolution +
		tv.tv_usec / (1000000 / detection_tick_resolution);

	/* process the packet */
	proto = opendpi_process_packet(ct, time, ip_hdr(skb_use), skb_use->len);

	if(linearized_skb != NULL){
		kfree_skb(linearized_skb);
	}


	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(info->flags,proto) != 0)
		return true;

	return false;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
static bool 
opendpi_mt_check(const char *tablename,
                 const void *ip,
                 const struct xt_match *match,
                 void *matchinfo,
                 unsigned int hook_mask)

{
	const struct xt_opendpi_mtinfo *info = matchinfo;

	if (IPOQUE_BITMASK_IS_ZERO(info->flags)){
		pr_info("None selected protocol.\n");
		return false;
	}

        opendpi_enable_protocols (info);

	return nf_ct_l3proto_try_module_get (match->family) == 0;
}

#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
static bool
#else
static int
#endif
opendpi_mt_check(const struct xt_mtchk_param *par)
{
	const struct xt_opendpi_mtinfo *info = par->matchinfo;

	if (IPOQUE_BITMASK_IS_ZERO(info->flags)){
		pr_info("None selected protocol.\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
		return false;
#else
		return -EINVAL;
#endif
	}

        opendpi_enable_protocols (info);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	return nf_ct_l3proto_try_module_get (par->family) == 0;
#else
	return nf_ct_l3proto_try_module_get (par->family);
#endif
}
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
static void 
opendpi_mt_destroy (const struct xt_match *match, void *matchinfo)
{
	const struct xt_opendpi_mtinfo *info = matchinfo;

        opendpi_disable_protocols (info);
	nf_ct_l3proto_module_put (match->family);
}

#else
static void 
opendpi_mt_destroy (const struct xt_mtdtor_param *par)
{
	const struct xt_opendpi_mtinfo *info = par->matchinfo;

        opendpi_disable_protocols (info);
	nf_ct_l3proto_module_put (par->family);
}

#endif



static void opendpi_cleanup(void)
{
        struct rb_node * next;
        struct osdpi_id_node *id;
        struct osdpi_flow_node *flow;

        ipoque_exit_detection_module(ipoque_struct, free_wrapper);
        
        nf_conntrack_unregister_notifier (&osdpi_notifier);
        
        /* free all objects before destroying caches */
        next = rb_first(&osdpi_flow_root);
        while (next){
                flow = rb_entry(next, struct osdpi_flow_node, node);
                next = rb_next(&flow->node);
                rb_erase(&flow->node, &osdpi_flow_root);
                kmem_cache_free (osdpi_flow_cache, flow);
        }
        kmem_cache_destroy (osdpi_flow_cache);
        
        next = rb_first(&osdpi_id_root);
        while (next){
                id = rb_entry(next, struct osdpi_id_node, node);
                next = rb_next(&id->node);
                rb_erase(&id->node, &osdpi_id_root);
                kmem_cache_free (osdpi_id_cache, id);
        }
        kmem_cache_destroy (osdpi_id_cache);
}


static struct xt_match
opendpi_mt_reg __read_mostly = {
	.name = "opendpi",
	.revision = 0,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
	.family = AF_INET,
#else
	.family = NFPROTO_IPV4,
#endif
	.match = opendpi_mt,
	.checkentry = opendpi_mt_check,
	.destroy = opendpi_mt_destroy,
	.matchsize = sizeof(struct xt_opendpi_mtinfo),
	.me = THIS_MODULE,
};


static int __init opendpi_mt_init(void)
{
        int ret, i;

	pr_info("xt_opendpi 0.1 (OpenDPI wrapper module).\n");
	/* init global detection structure */
	ipoque_struct = ipoque_init_detection_module(detection_tick_resolution,
                                                     malloc_wrapper, debug_printf);
	if (ipoque_struct == NULL) {
		pr_err("xt_opendpi: global structure initialization failed.\n");
                ret = -ENOMEM;
                goto err_out;
	}
        
        for (i = 0; i < IPOQUE_MAX_SUPPORTED_PROTOCOLS; i++){
                atomic_set (&protocols_cnt[i], 0);
        }

	/* disable all protocols */
	IPOQUE_BITMASK_RESET(protocols_bitmask);
	ipoque_set_protocol_detection_bitmask2(ipoque_struct, &protocols_bitmask);
        
	/* allocate memory for id and flow tracking */
	size_id_struct = ipoque_detection_get_sizeof_ipoque_id_struct();
	size_flow_struct = ipoque_detection_get_sizeof_ipoque_flow_struct();
        
        osdpi_flow_cache = kmem_cache_create("xt_opendpi_flows",
                                             sizeof(struct osdpi_flow_node) +
                                             size_flow_struct,
                                             0, 0, NULL);
        if (!osdpi_flow_cache){
                pr_err("xt_opendpi: error creating flow cache.\n");
                ret = -ENOMEM;
                goto err_ipq;
        }
        
        osdpi_id_cache = kmem_cache_create("xt_opendpi_ids",
                                           sizeof(struct osdpi_id_node) +
                                           size_id_struct,
                                           0, 0, NULL);
        if (!osdpi_id_cache){
                pr_err("xt_opendpi: error creating ids cache.\n");
                ret = -ENOMEM;
                goto err_flow;
        }
        
        ret = nf_conntrack_register_notifier(&osdpi_notifier);
        if (ret < 0){
                pr_err("xt_opendpi: error registering notifier.\n");
                goto err_id;
        }

        ret = xt_register_match(&opendpi_mt_reg);
        if (ret != 0){
                pr_err("xt_opendpi: error registering opendpi match.\n");
                opendpi_cleanup();
        }

        return ret;

err_id:
        kmem_cache_destroy (osdpi_id_cache);
err_flow:
        kmem_cache_destroy (osdpi_flow_cache);
err_ipq:
        ipoque_exit_detection_module(ipoque_struct, free_wrapper);
err_out:
        return ret;
}


static void __exit opendpi_mt_exit(void)
{
	pr_info("xt_opendpi 0.1 unload.\n");

	xt_unregister_match(&opendpi_mt_reg);

        opendpi_cleanup();
}


module_init(opendpi_mt_init);
module_exit(opendpi_mt_exit);
