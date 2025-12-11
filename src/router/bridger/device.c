// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Felix Fietkau <nbd@nbd.name>
 */
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <glob.h>
#include <utils.h>
#include <uloop.h>
#include "bridger.h"

static int device_avl_cmp(const void *k1, const void *k2, void *ptr)
{
	return (uintptr_t)k1 - (uintptr_t)k2;
}

static void bridger_device_update_cb(struct uloop_timeout *t);

AVL_TREE(devices, device_avl_cmp, false, NULL);
static struct uloop_timeout update_timer = {
	.cb = bridger_device_update_cb,
};
static bool init_done = false;

static const char * const device_types[__DEVICE_TYPE_MAX] = {
	[DEVICE_TYPE_ETHERNET] = "ethernet",
	[DEVICE_TYPE_VLAN] = "vlan",
	[DEVICE_TYPE_BRIDGE] = "bridge",
};

enum device_type device_lookup_type(const char *type)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(device_types); i++)
		if (!strcmp(type, device_types[i]))
			return i;

	return DEVICE_TYPE_ETHERNET;
}

struct device *device_get(int ifindex)
{
	struct device *dev;

	if (!ifindex)
		return NULL;

	return avl_find_element(&devices, (void *)(uintptr_t)ifindex, dev, node);
}

struct device *device_get_by_name(const char *name)
{
	unsigned int ifindex;

	ifindex = if_nametoindex(name);
	if (!ifindex)
		return NULL;

	return device_get(ifindex);
}

int device_set_redirect(struct device *dev, unsigned int ifindex)
{
	if (dev->redirect_dev == ifindex)
		return 0;

	dev->redirect_dev = ifindex;
	if (dev->attached)
		device_update(dev);

	return 0;
}

int device_vlan_get_input(struct device *dev, uint16_t bpf_vlan)
{
	struct bridge *br = device_get_br(dev);
	int i;

	if (!br || !br->vlan_enabled)
		return 0;

	if (!(bpf_vlan & BRIDGER_VLAN_PRESENT) ||
	    !!(bpf_vlan & BRIDGER_VLAN_TYPE_AD) !=
	    (br->vlan_proto == ETH_P_8021AD)) {
		if (!dev->pvid)
			return -1;

		for (i = 0; i < dev->n_vlans; i++)
			if (dev->pvid == dev->vlan[i].id)
				return dev->pvid;

		return -1;
	}

	bpf_vlan &= BRIDGER_VLAN_ID;
	for (i = 0; i < dev->n_vlans; i++)
		if (bpf_vlan == dev->vlan[i].id)
			return bpf_vlan;

	return -1;
}

int device_vlan_get_output(struct device *dev, int vid)
{
	struct bridge *br = device_get_br(dev);
	uint16_t flags = 0;
	int i;

	if (!br)
		return 0;

	if (!br->vlan_enabled)
		return 0;

	for (i = 0; i < dev->n_vlans; i++) {
		if (vid != dev->vlan[i].id)
			continue;

		if (dev->vlan[i].untagged)
			return 0;

		flags = BRIDGER_VLAN_PRESENT;
		if (br->vlan_proto == ETH_P_8021AD)
			flags |= BRIDGER_VLAN_TYPE_AD;

		return vid | flags;
	}

	return -1;
}

bool device_vlan_state_forwarding(struct device *dev, int vid)
{
	int i;

	if (!dev->vlan)
		return true;

	for (i = 0; i < dev->n_vlans; i++) {
		if (vid != dev->vlan[i].id)
			continue;

		return dev->vlan[i].forwarding;
	}

	return false;
}

bool device_vlan_has_tunnel(struct device *dev, int vid)
{
	int i;

	if (!dev->vlan)
		return false;

	for (i = 0; i < dev->n_vlans; i++) {
		if (vid != dev->vlan[i].id)
			continue;

		return dev->vlan[i].tunnel;
	}

	return false;
}

static void
device_set_attached(struct device *dev, bool val)
{
	if (dev->attached == val)
		return;

	if (!val)
		device_set_redirect(dev, 0);

	D("%s device %s\n", val ? "attach" : "detach", dev->ifname);
	dev->attached = val;

	if (val)
		bridger_nl_device_attach(dev, false);
	else
		bridger_nl_device_detach(dev, false);
}

static void
device_set_tx_attached(struct device *dev, bool val)
{
	if (dev->tx_attached == val)
		return;

	D("%s device tx %s\n", val ? "attach" : "detach", dev->ifname);
	dev->tx_attached = val;

	if (val)
		bridger_nl_device_attach(dev, true);
	else
		bridger_nl_device_detach(dev, true);
}

struct device *device_create(int ifindex, enum device_type type, const char *name)
{
	struct device *dev;

	dev = device_get(ifindex);
	if (dev) {
		if (dev->type == type)
			goto out;

		device_free(dev);
	}


	D("Create %s device %s, ifindex=%d\n", device_types[type], name, ifindex);
	dev = calloc(1, sizeof(*dev));
	dev->type = type;
	dev->node.key = (void *)(uintptr_t)ifindex;
	avl_insert(&devices, &dev->node);
	INIT_LIST_HEAD(&dev->fdb_entries);
	INIT_LIST_HEAD(&dev->member_list);
	if (!init_done)
		dev->cleanup = true;

	if (type == DEVICE_TYPE_BRIDGE) {
		struct bridge *br = calloc(1, sizeof(*br));

		INIT_LIST_HEAD(&br->members);
		fdb_init(br);
		dev->br = br;
		br->dev = dev;
	}

out:
	snprintf(dev->ifname, sizeof(dev->ifname), "%s", name);
	return dev;
}

void device_clear_flows(struct device *dev)
{
	struct fdb_entry *f;

	list_for_each_entry(f, &dev->fdb_entries, dev_list)
		fdb_clear_flows(f);
}

void device_free(struct device *dev)
{
	D("Free device %s\n", dev->ifname);

	avl_delete(&devices, &dev->node);
	device_clear_flows(dev);
	device_set_attached(dev, false);
	if (dev->master)
		list_del(&dev->member_list);
	free(dev->vlan);
	free(dev);
}

static struct device *device_get_offload_dev(struct device *dev)
{
	char path[128];
	char *name;
	glob_t g;
	int index;

	snprintf(path, sizeof(path), "/sys/class/net/%s/lower_*", dev->ifname);
	dev = NULL;
	glob(path, GLOB_NOSORT, NULL, &g);
	if (g.gl_pathc != 1)
		goto out;

	name = strrchr(g.gl_pathv[0], '/');
	if (!name)
		goto out;

	name += 7;
	index = if_nametoindex(name);
	if (!index)
		goto out;

	dev = device_get(index);

out:
	globfree(&g);
	return dev;
}

void device_update(struct device *dev)
{
	dev->update = true;

	if (init_done)
		uloop_timeout_set(&update_timer, 1);
}

static void __device_update(struct device *dev)
{
	struct device *master, *odev;
	bool attach;

	if (!dev->update)
		return;

	dev->update = false;
	device_clear_flows(dev);
	master = device_get(dev->master_ifindex);
	if (dev->master != master) {
		if (!list_empty(&dev->member_list))
			list_del_init(&dev->member_list);
	}

	if (master) {
		if (master->br)
			list_add_tail(&dev->member_list, &master->br->members);
		else
			master = NULL;
	}

	if (dev->master != master)
		D("Set device %s master to %s\n", dev->ifname,
		  master ? master->ifname : "(none)");

	dev->master = master;

	attach = (dev->master || dev->br) && !bridger_ubus_dev_blacklisted(dev);
	device_set_attached(dev, attach);
	bridger_bpf_dev_policy_set(dev);
	device_set_tx_attached(dev, attach && dev->redirect_dev);

	odev = device_get_offload_dev(dev);
	if (odev != dev->offload_dev)
		D("Set device %s offload device to %s\n", dev->ifname,
		  odev ? odev->ifname : "(none)");

	dev->offload_dev = odev;
}

static void bridger_device_update_cb(struct uloop_timeout *t)
{
	struct device *dev;

	avl_for_each_element(&devices, dev, node)
		__device_update(dev);
}

void device_reset_offload_update(void)
{
	struct device *dev;

	avl_for_each_element(&devices, dev, node)
		dev->offload_update = true;
}

int bridger_device_init(void)
{
	init_done = true;

	uloop_timeout_set(&update_timer, 1);

	return 0;
}

void bridger_device_stop(void)
{
	struct device *dev, *tmp;

	avl_for_each_element_safe(&devices, dev, node, tmp)
		device_free(dev);
}
