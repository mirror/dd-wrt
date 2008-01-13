/*
 *	Userspace interface
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: br_if.c,v 1.6.2.1 2001/12/24 00:59:27 davem Exp $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/if_arp.h>
#include <linux/if_bridge.h>
#include <linux/inetdevice.h>
#include <linux/rtnetlink.h>
#include <linux/brlock.h>
#include <linux/etherdevice.h>
#include <asm/uaccess.h>
#include "br_private.h"

static int br_initial_port_cost(struct net_device *dev)
{
	if (!strncmp(dev->name, "lec", 3))
		return 7;

	if (!strncmp(dev->name, "eth", 3))
		return 100;			/* FIXME handle 100Mbps */

	if (!strncmp(dev->name, "plip", 4))
		return 2500;

	return 100;
}

/* called under BR_NETPROTO_LOCK and bridge lock */
static int __br_del_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p;
	struct net_bridge_port **pptr;

	if ((p = dev->br_port) == NULL || p->br != br)
		return -EINVAL;

	br_stp_disable_port(p);

	dev_set_promiscuity(dev, -1);
	dev->br_port = NULL;

	pptr = &br->port_list;
	while (*pptr != NULL) {
		if (*pptr == p) {
			*pptr = p->next;
			break;
		}

		pptr = &((*pptr)->next);
	}

	br_fdb_delete_by_port(br, p);
	kfree(p);
	dev_put(dev);

	return 0;
}

static void del_ifs(struct net_bridge *br)
{
	br_write_lock_bh(BR_NETPROTO_LOCK);
	write_lock(&br->lock);
	while (br->port_list != NULL)
		__br_del_if(br, br->port_list->dev);
	write_unlock(&br->lock);
	br_write_unlock_bh(BR_NETPROTO_LOCK);
}

static struct net_bridge *new_nb(char *name)
{
	struct net_bridge *br;
	struct net_device *dev;

	if ((br = kmalloc(sizeof(*br), GFP_KERNEL)) == NULL)
		return NULL;

	memset(br, 0, sizeof(*br));
	dev = &br->dev;

	strncpy(dev->name, name, IFNAMSIZ);
	dev->priv = br;
	ether_setup(dev);
	br_dev_setup(dev);

	br->lock = RW_LOCK_UNLOCKED;
	br->hash_lock = RW_LOCK_UNLOCKED;

	br->bridge_id.prio[0] = 0x80;
	br->bridge_id.prio[1] = 0x00;
	memset(br->bridge_id.addr, 0, ETH_ALEN);

	br->stp_enabled = 0;
	br->designated_root = br->bridge_id;
	br->root_path_cost = 0;
	br->root_port = 0;
	br->bridge_max_age = br->max_age = 20 * HZ;
	br->bridge_hello_time = br->hello_time = 2 * HZ;
	br->bridge_forward_delay = br->forward_delay = 15 * HZ;
	br->topology_change = 0;
	br->topology_change_detected = 0;
	br_timer_clear(&br->hello_timer);
	br_timer_clear(&br->tcn_timer);
	br_timer_clear(&br->topology_change_timer);

	br->ageing_time = 300 * HZ;
	br->gc_interval = 4 * HZ;

	return br;
}

/* called under bridge lock */
static struct net_bridge_port *new_nbp(struct net_bridge *br, struct net_device *dev)
{
	int i;
	struct net_bridge_port *p;

	p = kmalloc(sizeof(*p), GFP_ATOMIC);
	if (p == NULL)
		return p;

	memset(p, 0, sizeof(*p));
	p->br = br;
	p->dev = dev;
	p->path_cost = br_initial_port_cost(dev);
	p->priority = 0x80;

	for (i=1;i<255;i++)
		if (br_get_port(br, i) == NULL)
			break;

	if (i == 255) {
		kfree(p);
		return NULL;
	}

	dev->br_port = p;

	p->port_no = i;
	br_init_port(p);
	p->state = BR_STATE_DISABLED;

	p->next = br->port_list;
	br->port_list = p;

	return p;
}

int br_add_bridge(char *name)
{
	struct net_bridge *br;
	struct net_device *dev;
	int err;

	if ((br = new_nb(name)) == NULL)
		return -ENOMEM;

	dev = &br->dev;
	if (strchr(dev->name, '%')) {
		err = dev_alloc_name(dev, dev->name);
		if (err < 0)
			goto  out;
	}

	err = register_netdevice(dev);
	if (err == 0)
		br_inc_use_count();

 out:
	return err;
}

int br_del_bridge(char *name)
{
	struct net_device *dev;
	struct net_bridge *br;

	dev = __dev_get_by_name(name);
	if (!dev)
		return -ENXIO;

	if (dev->hard_start_xmit != br_dev_xmit)
		return -EPERM;

	if (dev->flags & IFF_UP)
		return -EBUSY;

	br = dev->priv;
	BUG_ON(&br->dev != dev);

	del_ifs(br);

	unregister_netdevice(dev);
	kfree(br);
	br_dec_use_count();

	return 0;
}

int br_add_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p;

	if (dev->br_port != NULL)
		return -EBUSY;

	if (dev->flags & IFF_LOOPBACK || dev->type != ARPHRD_ETHER)
		return -EINVAL;

	if (dev->hard_start_xmit == br_dev_xmit)
		return -ELOOP;

	if (!is_valid_ether_addr(dev->dev_addr))
		return -EADDRNOTAVAIL;

	dev_hold(dev);
	write_lock_bh(&br->lock);
	if ((p = new_nbp(br, dev)) == NULL) {
		write_unlock_bh(&br->lock);
		dev_put(dev);
		return -EXFULL;
	}

	dev_set_promiscuity(dev, 1);

	br_stp_recalculate_bridge_id(br);
	br_fdb_insert(br, p, dev->dev_addr, 1);
	if ((br->dev.flags & IFF_UP) && (dev->flags & IFF_UP))
		br_stp_enable_port(p);
	write_unlock_bh(&br->lock);

	return 0;
}

int br_del_if(struct net_bridge *br, struct net_device *dev)
{
	int retval;

	br_write_lock_bh(BR_NETPROTO_LOCK);
	write_lock(&br->lock);
	retval = __br_del_if(br, dev);
	br_stp_recalculate_bridge_id(br);
	write_unlock(&br->lock);
	br_write_unlock_bh(BR_NETPROTO_LOCK);

	return retval;
}

int br_get_bridge_ifindices(int *indices, int num)
{
	struct net_device *dev;
	int i = 0;

	for (dev = dev_base; i < num && dev != NULL; dev = dev->next) {
		if (dev->hard_start_xmit == br_dev_xmit)
			indices[i++] = dev->ifindex;
	}

	return i;
}

/* called under ioctl_lock */
void br_get_port_ifindices(struct net_bridge *br, int *ifindices)
{
	struct net_bridge_port *p;

	p = br->port_list;
	while (p != NULL) {
		ifindices[p->port_no] = p->dev->ifindex;
		p = p->next;
	}
}
