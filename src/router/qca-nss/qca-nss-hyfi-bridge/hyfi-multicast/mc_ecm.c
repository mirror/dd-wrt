/*
 *  QCA Multicast ECM APIs
 *
 * Copyright (c) 2014, 2016 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define DEBUG_LEVEL HYFI_MC_DEBUG_LEVEL

#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include "hyfi_api.h"
#include "hyfi_bridge.h"
#include "mc_api.h"
#include "mc_private.h"
#include "mc_snooping.h"
#include "mc_ecm.h"

static hyfi_bridge_ipv4_mc_update_callback_t __rcu hyfi_ipv4_mc_event_cb = NULL;
static hyfi_bridge_ipv6_mc_update_callback_t __rcu hyfi_ipv6_mc_event_cb = NULL;


static int hyfi_bridge_if_source_filter(struct mc_mdb_entry *mdb, uint32_t ifindex, struct mc_ip *mc_source)
{

    struct mc_port_group *pg;
    struct hlist_node *pgh;
    int i;

    /*no bridge port joining*/
    if (hlist_empty(&mdb->pslist))
        return 1;

    os_hlist_for_each_entry_rcu(pg, pgh, &mdb->pslist, pslist) {
        struct mc_fdb_group *fg;
        struct hlist_node *fgh;

        if (ifindex != ((struct net_bridge_port *)pg->port)->dev->ifindex)
            continue;

        /*no client joining*/
        if (hlist_empty(&pg->fslist))
            return 1;

        /*anyone who would like to receive stream from the source*/
        os_hlist_for_each_entry_rcu(fg, fgh, &pg->fslist, fslist) {
            if (!fg->filter_mode || (fg->filter_mode == HYFI_MC_EXCLUDE && !fg->a.nsrcs))
                return 0;

            if (fg->filter_mode == HYFI_MC_INCLUDE && !fg->a.nsrcs)
                continue;

            if (mdb->group.pro == htons(ETH_P_IP)) {
                u_int32_t ip4 = mc_source->u.ip4;
                u_int32_t *srcs = (u_int32_t *)fg->a.srcs;
                for (i = 0; i < fg->a.nsrcs; i++) {
                    if (srcs[i] == ip4)
                        break;
                }
            }
#ifdef HYBRID_MC_MLD
            else {
                struct in6_addr *ip6 = &mc_source->u.ip6;
                struct in6_addr *srcs = (struct in6_addr *)fg->a.srcs;
                for (i = 0; i < fg->a.nsrcs; i++) {
                    if (!ipv6_addr_cmp(&srcs[i], ip6))
                        break;
                }
            }
#endif

            if ((fg->filter_mode == HYFI_MC_INCLUDE && i != fg->a.nsrcs) ||
                (fg->filter_mode == HYFI_MC_EXCLUDE && i == fg->a.nsrcs))
                return 0;

        }
    }

    return 1;

}

static int __hyfi_bridge_mc_get_ifs(struct net_device *brdev, struct mc_ip *mc_group,
                       struct mc_ip *mc_source, uint32_t max_dst, uint32_t dst_dev[])
{
    struct hyfi_net_bridge *hyfi_br;
    struct mc_struct *mc;
    struct hlist_head *head;
    struct mc_mdb_entry *mdb;
    int i;
    int ifnum;

    hyfi_br = hyfi_bridge_get_by_dev(brdev);
    if (!hyfi_br){
        printk("The bridge %s is not attached\n", brdev->name);
        return -1;
    }

    mc = MC_DEV(hyfi_br);

    if (!mc || !mc->started ){
        printk("IGMP/MLD snooping is not enabled\n");
        return -1;
    }

    head = &mc->hash[mc_group_hash(mc->salt, mc_group->u.ip4)];
    mdb = mc_mdb_find(head, mc_group);
    if (!mdb || !atomic_read(&mdb->users)) {
        return 0;
    }

    read_lock(&mdb->rwlock);
    for (i = 0, ifnum = 0; i < mdb->flood_ifcnt; i++) {
        if (hyfi_bridge_if_source_filter(mdb, mdb->flood_ifindex[i], mc_source))
        {

            if (mc_group->pro == htons(ETH_P_IP))
                MC_PRINT("Group "MC_IP4_STR" Source "MC_IP4_STR"  ignored for port %d\n",
                    MC_IP4_FMT((u8 *)&mc_group->u.ip4), MC_IP4_FMT((u8 *)&mc_source->u.ip4), mdb->flood_ifindex[i]);
#ifdef HYBRID_MC_MLD
            else
                MC_PRINT("Group "MC_IP6_STR" Source "MC_IP6_STR"  ignored for port %d\n",
                    MC_IP6_FMT((__be16 *)&mc_group->u.ip6), MC_IP6_FMT((__be16 *)&mc_source->u.ip6), mdb->flood_ifindex[i]);
#endif
            continue;
        }

        if ( ifnum + 1 > max_dst) {
            MC_PRINT("Multicast interfaces overflow %d/%d\n", mdb->flood_ifcnt, max_dst);
            ifnum = -1;
            break;
        }

        dst_dev[ifnum] = mdb->flood_ifindex[i];
        ifnum++;

    }
    read_unlock(&mdb->rwlock);

    return ifnum;

}

int hyfi_bridge_ipv4_mc_get_if(struct net_device *brdev, __be32 origin, __be32 group,
                                     uint32_t max_dst, uint32_t dst_dev[])
{
    struct mc_ip mc_group;
    struct mc_ip mc_source;
    int ret;

    memset(&mc_group, 0, sizeof(struct mc_ip));
    mc_group.u.ip4 = group;
    mc_group.pro = htons(ETH_P_IP);

    memset(&mc_source, 0, sizeof(struct mc_ip));
    mc_source.u.ip4 = origin;
    mc_source.pro = htons(ETH_P_IP);

    rcu_read_lock();
    ret =  __hyfi_bridge_mc_get_ifs(brdev, &mc_group, &mc_source, max_dst, dst_dev);
    rcu_read_unlock();

    return ret;
}
EXPORT_SYMBOL(hyfi_bridge_ipv4_mc_get_if);

int hyfi_bridge_ipv4_mc_update_callback_register (hyfi_bridge_ipv4_mc_update_callback_t snoop_event_cb)
{
    hyfi_bridge_ipv4_mc_update_callback_t ipv4_mc_event_cb;

    ipv4_mc_event_cb = rcu_dereference(hyfi_ipv4_mc_event_cb);
    if (ipv4_mc_event_cb &&
        ipv4_mc_event_cb != snoop_event_cb)
    {
        printk("MC callback function is using by another module\n");
        return -1;
    }

    rcu_assign_pointer(hyfi_ipv4_mc_event_cb, snoop_event_cb);
    return 0;
}
EXPORT_SYMBOL(hyfi_bridge_ipv4_mc_update_callback_register);

int hyfi_bridge_ipv4_mc_update_callback_deregister(void)
{
    rcu_assign_pointer(hyfi_ipv4_mc_event_cb, NULL);
    return 0;

}
EXPORT_SYMBOL(hyfi_bridge_ipv4_mc_update_callback_deregister);


hyfi_bridge_ipv4_mc_update_callback_t hyfi_bridge_ipv4_mc_update_callback_get(void)
{
    return rcu_dereference(hyfi_ipv4_mc_event_cb);
}

#ifdef HYBRID_MC_MLD
int hyfi_bridge_ipv6_mc_get_if(struct net_device *brdev, struct in6_addr *origin, struct in6_addr *group,
                                     uint32_t max_dst, uint32_t dst_dev[])
{
    struct mc_ip mc_group;
    struct mc_ip mc_source;
    int ret;

    memset(&mc_group, 0, sizeof(struct mc_ip));
    hyfi_ipv6_addr_copy(&mc_group.u.ip6, group);
    mc_group.pro = htons(ETH_P_IPV6);

    memset(&mc_source, 0, sizeof(struct mc_ip));
    hyfi_ipv6_addr_copy(&mc_source.u.ip6, origin);
    mc_source.pro = htons(ETH_P_IPV6);

    rcu_read_lock();
    ret =  __hyfi_bridge_mc_get_ifs(brdev, &mc_group, &mc_source, max_dst, dst_dev);
    rcu_read_unlock();

    return ret;
}
EXPORT_SYMBOL(hyfi_bridge_ipv6_mc_get_if);

int hyfi_bridge_ipv6_mc_update_callback_register (hyfi_bridge_ipv6_mc_update_callback_t snoop_event_cb)
{
    hyfi_bridge_ipv6_mc_update_callback_t ipv6_mc_event_cb;

    ipv6_mc_event_cb = rcu_dereference(hyfi_ipv6_mc_event_cb);
    if (ipv6_mc_event_cb &&
        ipv6_mc_event_cb != snoop_event_cb)
    {
        printk("MC callback function is using by another module\n");
        return -1;
    }

    rcu_assign_pointer(hyfi_ipv6_mc_event_cb, snoop_event_cb);
    return 0;
}
EXPORT_SYMBOL(hyfi_bridge_ipv6_mc_update_callback_register);

int hyfi_bridge_ipv6_mc_update_callback_deregister(void)
{
    rcu_assign_pointer(hyfi_ipv6_mc_event_cb, NULL);
    return 0;
}
EXPORT_SYMBOL(hyfi_bridge_ipv6_mc_update_callback_deregister);

hyfi_bridge_ipv6_mc_update_callback_t hyfi_bridge_ipv6_mc_update_callback_get(void)
{
    return rcu_dereference(hyfi_ipv6_mc_event_cb);
}

#endif

#ifdef _MC_ECM_TEST
void _test_mc_ipv4_callback(struct net_device *brdev, uint32_t group)
{

}

void _test_mc_ipv6_callback(struct net_device *brdev, struct in6_addr *group)
{

}

void _test_mc_get_ifs(void)
{
    struct net_device *brdev;
    __be32 origin;
    __be32 group;
    struct in6_addr origin6;
    struct in6_addr group6;
    uint32_t max_dst = 16;
    uint32_t dst_dev[16];

    int ifnum;
    int i;


   brdev  = dev_get_by_name(&init_net, "br-lan");
   if (!brdev){
       printk("Can't get bridge\n");
       return;
   }

   /*239.1.2.3*/
   origin = 0;
   group  = htonl(0xef010203);

    ifnum = hyfi_bridge_ipv4_mc_get_if(brdev, origin, group, max_dst, dst_dev);
    if (ifnum < 0)
        printk("Failed to get IPv4 mc interfaces\n");
    else {
        printk("Got IPv4 %d mc interfaces\n", ifnum);
        for( i = 0; i < ifnum; i++)
            printk("%d ", dst_dev[i]);
        printk("\n");
    }

    /*ff15::0001*/
    ipv6_addr_set(&origin6, 0, 0, 0, 0);
    ipv6_addr_set(&group6, htonl(0xff150000), 0, 0, htonl(1));

    ifnum = hyfi_bridge_ipv6_mc_get_if(brdev, &origin6, &group6, max_dst, dst_dev);
    if (ifnum < 0)
        printk("Failed to get IPv4 mc interfaces\n");
    else {
        printk("Got IPv6 %d mc interfaces\n", ifnum);
        for( i = 0; i < ifnum; i++)
            printk("%d ", dst_dev[i]);
        printk("\n");
    }


    dev_put(brdev);
}


int _test_mc_ecm_init(void)
{
    printk("MC callback init\n");

    if (hyfi_bridge_ipv4_mc_update_callback_register(_test_mc_ipv4_callback) < 0)
        printk("Failed to register IPv4 callbak\n");

    if (hyfi_bridge_ipv6_mc_update_callback_register(_test_mc_ipv6_callback) < 0)
        printk("Failed to register IPv6 callbak\n");

    return 0;
}


int _test_mc_ecm_deinit(void)
{
    printk("MC callback deinit\n");

    if (hyfi_bridge_ipv4_mc_update_callback_deregister() < 0)
        printk("Failed to register IPv6 callbak\n");

    if (hyfi_bridge_ipv6_mc_update_callback_deregister() < 0)
        printk("Failed to register IPv6 callbak\n");

    return 0;
}

#endif



