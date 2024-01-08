/*
 * Copyright (C) 2000 Lennert Buytenhek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <utils.h>
#include <shutils.h>
#include <syslog.h>
#include <bcmnvram.h>

#include "libbridge.h"
#include "libbridge_private.h"

static int br_socket_fd = -1;
static struct sysfs_class *br_class_net;

static unsigned long __tv_to_jiffies(const struct timeval *tv)
{
	unsigned long long jif;

	jif = 1000000ULL * tv->tv_sec + tv->tv_usec;

	return (HZ * jif) / 1000000;
}

static void __jiffies_to_tv(struct timeval *tv, unsigned long jiffies)
{
	unsigned long long tvusec;

	tvusec = (1000000ULL * jiffies) / HZ;
	tv->tv_sec = tvusec / 1000000;
	tv->tv_usec = tvusec - 1000000 * tv->tv_sec;
}

/* 
 * Only used if sysfs is not available.
 */
static int old_foreach_port(const char *brname, int (*iterator)(const char *br, const char *port, void *arg), void *arg)
{
	int i, err, count;
	struct ifreq ifr;
	char ifname[IFNAMSIZ];
	int ifindices[MAX_PORTS];
	unsigned long args[4] = { BRCTL_GET_PORT_LIST, (unsigned long)ifindices, MAX_PORTS, 0 };

	bzero(ifindices, sizeof(ifindices));
	strncpy(ifr.ifr_name, brname, IFNAMSIZ);
	ifr.ifr_data = (char *)&args;

	err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	if (err < 0) {
		dprintf("list ports for bridge:'%s' failed: %s\n", brname, strerror(errno));
		return -errno;
	}

	count = 0;
	for (i = 0; i < MAX_PORTS; i++) {
		if (!ifindices[i])
			continue;

		if (!if_indextoname(ifindices[i], ifname)) {
			dprintf("can't find name for ifindex:%d\n", ifindices[i]);
			continue;
		}

		++count;
		if (iterator(brname, ifname, arg))
			break;
	}

	return count;
}

/*
 * Iterate over all ports in bridge (using sysfs).
 */
static int br_foreach_port(const char *brname, int (*iterator)(const char *br, const char *port, void *arg), void *arg)
{
#ifdef HAVE_LIBSYSFS
	struct sysfs_class_device *dev;
	DIR *dir;
	struct dirent *dirent;
	int err = 0;
	char path[SYSFS_PATH_MAX];

	if (!br_class_net || !(dev = sysfs_get_class_device(br_class_net, (char *)brname)))
		goto old;

	snprintf(path, sizeof(path), "%s/%s", dev->path, SYSFS_BRIDGE_PORT_SUBDIR);

	dir = opendir(path);
	if (!dir) {
		/* no /sys/class/net/ethX/brif subdirectory
		 * either: old kernel, or not really a bridge
		 */
		goto old;
	}

	err = 0;
	while ((dirent = readdir(dir)) != NULL) {
		if (0 == strcmp(dirent->d_name, "."))
			continue;
		if (0 == strcmp(dirent->d_name, ".."))
			continue;
		++err;
		if (iterator(brname, dirent->d_name, arg))
			break;
	}
	closedir(dir);

	return err;

old:
#endif
	return old_foreach_port(brname, iterator, arg);
}

int br_add_bridge(const char *brname)
{
	int ret;
	char tmp[256];

	dd_loginfo("bridge", "bridge added successfully\n");
	char ipaddr[32];

	sprintf(ipaddr, "%s_ipaddr", brname);
	char netmask[32];

	sprintf(netmask, "%s_netmask", brname);
#ifdef SIOCBRADDBR
	ret = ioctl(br_socket_fd, SIOCBRADDBR, brname);
	if (ret < 0)
#endif
	{
		char _br[IFNAMSIZ];
		unsigned long arg[3] = { BRCTL_ADD_BRIDGE, (unsigned long)_br };

		strncpy(_br, brname, IFNAMSIZ);
		ret = ioctl(br_socket_fd, SIOCSIFBR, arg);
	}

	if (nvram_exists(ipaddr) && nvram_exists(netmask) && !nvram_match(ipaddr, "0.0.0.0") && !nvram_match(netmask, "0.0.0.0")) {
		eval("ifconfig", brname, nvram_safe_get(ipaddr), "netmask", nvram_safe_get(netmask), "mtu",
		     getBridgeMTU(brname, tmp), "promisc", "up");
	} else
		eval("ifconfig", brname, "mtu", getBridgeMTU(brname, tmp), "promisc");

	return ret < 0 ? errno : 0;
}

int br_del_bridge(const char *brname)
{
	int ret;
	if (!ifexists(brname))
		return -1;

#ifdef SIOCBRDELBR
	ret = ioctl(br_socket_fd, SIOCBRDELBR, brname);
	if (ret < 0)
#endif
	{
		char _br[IFNAMSIZ];
		unsigned long arg[3] = { BRCTL_DEL_BRIDGE, (unsigned long)_br };

		strncpy(_br, brname, IFNAMSIZ);
		ret = ioctl(br_socket_fd, SIOCSIFBR, arg);
	}
	dd_loginfo("bridge", "bridge deleted successfully\n");
	return ret < 0 ? errno : 0;
}

int br_add_interface(const char *bridge, const char *dev)
{
	char tmp[256];
	if (!ifexists(dev))
		return -1;
	char ipaddr[32];

	sprintf(ipaddr, "%s_ipaddr", dev);
	char netmask[32];

	sprintf(netmask, "%s_netmask", dev);

	if (strncmp(dev, "wl", 2) != 0) { // this is not an ethernet driver
		eval("ifconfig", dev, "down"); //fixup for some ethernet drivers
	}
	if (nvram_exists(ipaddr) && nvram_exists(netmask) && !nvram_match(ipaddr, "0.0.0.0") && !nvram_match(netmask, "0.0.0.0")) {
		eval("ifconfig", dev, nvram_safe_get(ipaddr), "netmask", nvram_safe_get(netmask), "mtu", getBridgeMTU(bridge, tmp));
	} else
		eval("ifconfig", dev, "mtu", getBridgeMTU(bridge, tmp));

	if (strncmp(dev, "wl", 2) != 0) { // this is not an ethernet driver
		eval("ifconfig", dev, "up");
	}

	dd_loginfo("bridge", "interface added successfully\n");

	struct ifreq ifr;
	int err;
	int ifindex = if_nametoindex(dev);

	if (ifindex == 0)
		return ENODEV;

	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
#ifdef SIOCBRADDIF
	ifr.ifr_ifindex = ifindex;
	err = ioctl(br_socket_fd, SIOCBRADDIF, &ifr);
	if (err < 0)
#endif
	{
		unsigned long args[4] = { BRCTL_ADD_IF, ifindex, 0, 0 };

		ifr.ifr_data = (char *)args;
		err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	}
	return err < 0 ? errno : 0;
}

int br_del_interface(const char *bridge, const char *dev)
{
	if (!ifexists(dev))
		return -1;
	dd_loginfo("bridge", "interface deleted successfully\n");
	struct ifreq ifr;
	int err;
	int ifindex = if_nametoindex(dev);

	if (ifindex == 0)
		return ENODEV;

	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
#ifdef SIOCBRDELIF
	ifr.ifr_ifindex = ifindex;
	err = ioctl(br_socket_fd, SIOCBRDELIF, &ifr);
	if (err < 0)
#endif
	{
		unsigned long args[4] = { BRCTL_DEL_IF, ifindex, 0, 0 };

		ifr.ifr_data = (char *)args;
		err = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	}

	return err < 0 ? errno : 0;
}

static int br_set(const char *bridge, const char *name, unsigned long value, unsigned long oldcode)
{
	int ret = -1;

#ifdef HAVE_LIBSYSFS
	struct sysfs_class_device *dev;

	dev = sysfs_get_class_device(br_class_net, bridge);
	if (dev) {
		struct sysfs_attribute *attr;
		char buf[32];
		char path[SYSFS_PATH_MAX];

		snprintf(buf, sizeof(buf), "%ld\n", value);
		snprintf(path, SYSFS_PATH_MAX, "%s/bridge/%s", dev->path, name);

		attr = sysfs_open_attribute(path);
		if (attr) {
			ret = sysfs_write_attribute(attr, buf, strlen(buf));
			sysfs_close_attribute(attr);
		}
		sysfs_close_class_device(dev);
	} else
#endif
	{
		struct ifreq ifr;
		unsigned long args[4] = { oldcode, value, 0, 0 };

		strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
		ifr.ifr_data = (char *)&args;
		ret = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	}

	return ret < 0 ? errno : 0;
}

int br_set_bridge_forward_delay(const char *br, int sec)
{
	if (!ifexists(br))
		return -1;
	struct timeval tv;

	tv.tv_sec = sec;
	tv.tv_usec = 0;

	return br_set(br, "forward_delay", __tv_to_jiffies(&tv), BRCTL_SET_BRIDGE_FORWARD_DELAY);
}

int br_set_bridge_max_age(const char *br, int sec)
{
	if (!ifexists(br))
		return -1;
	struct timeval tv;

	tv.tv_sec = sec;
	tv.tv_usec = 0;
	return br_set(br, "max_age", __tv_to_jiffies(&tv), BRCTL_SET_BRIDGE_MAX_AGE);
}

int br_set_stp_state(const char *br, int stp_state)
{
	if (!ifexists(br))
		return -1;
	return br_set(br, "stp_state", stp_state, BRCTL_SET_BRIDGE_STP_STATE);
}

int br_set_bridge_prio(const char *br, int prio)
{
	if (!ifexists(br))
		return -1;
	return br_set(br, "priority", prio, BRCTL_SET_BRIDGE_PRIORITY);
}

/*
 * Convert device name to an index in the list of ports in bridge.
 *
 * Old API does bridge operations as if ports were an array
 * inside bridge structure.
 */
static int get_portno(const char *brname, const char *ifname)
{
	int i;
	int ifindex = if_nametoindex(ifname);
	int ifindices[MAX_PORTS];
	unsigned long args[4] = { BRCTL_GET_PORT_LIST, (unsigned long)ifindices, MAX_PORTS, 0 };
	struct ifreq ifr;

	if (ifindex <= 0)
		goto error;

	bzero(ifindices, sizeof(ifindices));
	strncpy(ifr.ifr_name, brname, IFNAMSIZ);
	ifr.ifr_data = (char *)&args;

	if (ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr) < 0) {
		dprintf("get_portno: get ports of %s failed: %s\n", brname, strerror(errno));
		goto error;
	}

	for (i = 0; i < MAX_PORTS; i++) {
		if (ifindices[i] == ifindex)
			return i;
	}

	dprintf("%s is not a in bridge %s\n", ifname, brname);
error:
	return -1;
}

static int port_set(const char *bridge, const char *ifname, const char *name, unsigned long value, unsigned long oldcode)
{
	int ret = -1;
#ifdef HAVE_LIBSYSFS
	struct sysfs_class_device *dev;

	dev = sysfs_get_class_device(br_class_net, ifname);
	if (dev) {
		struct sysfs_attribute *attr;
		char path[SYSFS_PATH_MAX];
		char buf[32];

		sprintf(buf, "%ld", value);
		snprintf(path, SYSFS_PATH_MAX, "%s/brport/%s", dev->path, name);

		attr = sysfs_open_attribute(path);
		if (attr) {
			ret = sysfs_write_attribute(attr, buf, strlen(buf));
			sysfs_close_attribute(attr);
		}
		sysfs_close_class_device(dev);
	} else
#endif
	{
		int index = get_portno(bridge, ifname);

		if (index < 0)
			ret = index;
		else {
			struct ifreq ifr;
			unsigned long args[4] = { oldcode, index, value, 0 };

			strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
			ifr.ifr_data = (char *)&args;
			ret = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
		}
	}

	return ret < 0 ? errno : 0;
}

int br_set_port_hairpin(const char *br, char *port, int on)
{
	if (!ifexists(br))
		return -1;
	if (!ifexists(port))
		return -1;
	return port_set(br, port, "hairpin_mode", on, 0);
}

int br_set_port_prio(const char *bridge, char *port, int prio)
{
	if (!ifexists(bridge))
		return -1;
	if (!ifexists(port))
		return -1;
	return port_set(bridge, port, "priority", prio, BRCTL_SET_PORT_PRIORITY);
}

#define BRCTL_SET_FILTERBPDU 25

int br_set_filterbpdu(const char *bridge, const char *port, int on)
{
	return port_set(bridge, port, "block_bpdu", on, BRCTL_SET_FILTERBPDU);
}

int br_set_path_cost(const char *bridge, const char *port, int cost)
{
	if (!ifexists(bridge))
		return -1;
	if (!ifexists(port))
		return -1;
	return port_set(bridge, port, "path_cost", cost, BRCTL_SET_PATH_COST);
}

static void br_dump_bridge_id(const unsigned char *x)
{
	fprintf(stdout, "%.2x%.2x.%.2x%.2x%.2x%.2x%.2x%.2x", x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7]);
}

static int first;

static int dump_interface(const char *b, const char *p, void *arg)
{
	if (first)
		first = 0;
	else
		fprintf(stdout, "\n\t\t\t\t\t\t\t");

	fprintf(stdout, "%s", p);

	return 0;
}

static void br_dump_interface_list(const char *br)
{
	int err;

	first = 1;
	err = br_foreach_port(br, dump_interface, NULL);
	if (err < 0)
		fprintf(stdout, " can't get port info: %s\n", strerror(-err));
	else
		fprintf(stdout, "\n");
}

/* get information via ioctl */
static int old_get_bridge_info(const char *bridge, struct bridge_info *info)
{
	struct ifreq ifr;
	struct __bridge_info i;
	unsigned long args[4] = { BRCTL_GET_BRIDGE_INFO, (unsigned long)&i, 0, 0 };

	bzero(info, sizeof(*info));
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
	ifr.ifr_data = (char *)&args;

	if (ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr) < 0) {
		dprintf("%s: can't get info %s\n", bridge, strerror(errno));
		return errno;
	}

	memcpy(&info->designated_root, &i.designated_root, 8);
	memcpy(&info->bridge_id, &i.bridge_id, 8);
	info->root_path_cost = i.root_path_cost;
	info->root_port = i.root_port;
	info->topology_change = i.topology_change;
	info->topology_change_detected = i.topology_change_detected;
	info->stp_enabled = i.stp_enabled;
	__jiffies_to_tv(&info->max_age, i.max_age);
	__jiffies_to_tv(&info->hello_time, i.hello_time);
	__jiffies_to_tv(&info->forward_delay, i.forward_delay);
	__jiffies_to_tv(&info->bridge_max_age, i.bridge_max_age);
	__jiffies_to_tv(&info->bridge_hello_time, i.bridge_hello_time);
	__jiffies_to_tv(&info->bridge_forward_delay, i.bridge_forward_delay);
	__jiffies_to_tv(&info->ageing_time, i.ageing_time);
	__jiffies_to_tv(&info->hello_timer_value, i.hello_timer_value);
	__jiffies_to_tv(&info->tcn_timer_value, i.tcn_timer_value);
	__jiffies_to_tv(&info->topology_change_timer_value, i.topology_change_timer_value);
	__jiffies_to_tv(&info->gc_timer_value, i.gc_timer_value);

	return 0;
}

/*
 * Get bridge parameters using either sysfs or old
 * ioctl.
 */
static int br_get_bridge_info(const char *bridge, struct bridge_info *info)
{
#ifdef HAVE_LIBSYSFS
	struct sysfs_class_device *dev;
	char path[SYSFS_PATH_MAX];

	if (!br_class_net)
		goto fallback;

	dev = sysfs_get_class_device(br_class_net, bridge);
	if (!dev) {
		dprintf("get_class_device '%s' failed\n", bridge);
		goto fallback;
	}

	snprintf(path, SYSFS_PATH_MAX, "%s/bridge", dev->path);
	if (sysfs_path_is_dir(path)) {
		dprintf("path '%s' is not a directory\n", path);
		sysfs_close_class_device(dev);
		goto fallback;
	}

	bzero(info, sizeof(*info));
	fetch_id(dev, BRIDGEATTR("root_id"), &info->designated_root);
	fetch_id(dev, BRIDGEATTR("bridge_id"), &info->bridge_id);
	info->root_path_cost = fetch_int(dev, BRIDGEATTR("root_path_cost"));
	fetch_tv(dev, BRIDGEATTR("max_age"), &info->max_age);
	fetch_tv(dev, BRIDGEATTR("hello_time"), &info->hello_time);
	fetch_tv(dev, BRIDGEATTR("forward_delay"), &info->forward_delay);
	fetch_tv(dev, BRIDGEATTR("max_age"), &info->bridge_max_age);
	fetch_tv(dev, BRIDGEATTR("hello_time"), &info->bridge_hello_time);
	fetch_tv(dev, BRIDGEATTR("forward_delay"), &info->bridge_forward_delay);
	fetch_tv(dev, BRIDGEATTR("ageing_time"), &info->ageing_time);
	fetch_tv(dev, BRIDGEATTR("hello_timer"), &info->hello_timer_value);
	fetch_tv(dev, BRIDGEATTR("tcn_timer"), &info->tcn_timer_value);
	fetch_tv(dev, BRIDGEATTR("topology_change_timer"), &info->topology_change_timer_value);
	;
	fetch_tv(dev, BRIDGEATTR("gc_timer"), &info->gc_timer_value);

	info->root_port = fetch_int(dev, BRIDGEATTR("root_port"));
	info->stp_enabled = fetch_int(dev, BRIDGEATTR("stp_state"));
	info->topology_change = fetch_int(dev, BRIDGEATTR("topology_change"));
	info->topology_change_detected = fetch_int(dev, BRIDGEATTR("topology_change_detected"));
	sysfs_close_class_device(dev);

	return 0;

fallback:
#endif
	return old_get_bridge_info(bridge, info);
}

static int show_bridge(const char *name, void *arg)
{
	struct bridge_info info;

	fprintf(stdout, "%s\t\t", name);
	fflush(stdout);

	if (br_get_bridge_info(name, &info)) {
		fprintf(stderr, "can't get info %s\n", strerror(errno));
		return 1;
	}

	br_dump_bridge_id((unsigned char *)&info.bridge_id);
	fprintf(stdout, "\t%s\t\t", info.stp_enabled ? "yes" : "no");

	br_dump_interface_list(name);
	return 0;
}

#ifdef HAVE_LIBSYSFS
/* If /sys/class/net/XXX/bridge exists then it must be a bridge */
static int isbridge(const struct sysfs_class_device *dev)
{
	char path[SYSFS_PATH_MAX];

	snprintf(path, sizeof(path), "%s/bridge", dev->path);
	return !sysfs_path_is_dir(path);
}

/*
 * New interface uses sysfs to find bridges
 */
static int new_foreach_bridge(int (*iterator)(const char *name, void *), void *arg)
{
	struct sysfs_class_device *dev;
	struct dlist *devlist;
	int count = 0;

	if (!br_class_net) {
		dprintf("no class /sys/class/net\n");
		return -EOPNOTSUPP;
	}

	devlist = sysfs_get_class_devices(br_class_net);
	if (!devlist) {
		dprintf("Can't read devices from sysfs\n");
		return -errno;
	}

	dlist_for_each_data(devlist, dev, struct sysfs_class_device)
	{
		if (isbridge(dev)) {
			++count;
			if (iterator(dev->name, arg))
				break;
		}
	}

	return count;
}
#endif

/*
 * Old interface uses ioctl
 */
static int old_foreach_bridge(int (*iterator)(const char *, void *), void *iarg)
{
	int i, ret = 0, num;
	char ifname[IFNAMSIZ];
	int ifindices[MAX_BRIDGES];
	unsigned long args[3] = { BRCTL_GET_BRIDGES, (unsigned long)ifindices, MAX_BRIDGES };

	num = ioctl(br_socket_fd, SIOCGIFBR, args);
	if (num < 0) {
		dprintf("Get bridge indices failed: %s\n", strerror(errno));
		return -errno;
	}

	for (i = 0; i < num; i++) {
		if (!if_indextoname(ifindices[i], ifname)) {
			dprintf("get find name for ifindex %d\n", ifindices[i]);
			return -errno;
		}

		++ret;
		if (iterator(ifname, iarg))
			break;
	}

	return ret;
}

/*
 * Go over all bridges and call iterator function.
 * if iterator returns non-zero then stop.
 */
static int br_foreach_bridge(int (*iterator)(const char *, void *), void *arg)
{
	int ret;
#ifdef HAVE_LIBSYSFS

	ret = new_foreach_bridge(iterator, arg);
	if (ret <= 0)
#endif
		ret = old_foreach_bridge(iterator, arg);

	return ret;
}

int br_cmd_show(void)
{
	fprintf(stdout, "bridge name\tbridge id\t\tSTP enabled\tinterfaces\n");
	br_foreach_bridge(show_bridge, NULL);
	return 0;
}

int br_init(void)
{
	if (br_socket_fd == -1) {
		if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			return errno;

		br_class_net = sysfs_open_class("net");
	}
	return 0;
}

void br_shutdown(void)
{
	if (br_socket_fd != -1) {
		sysfs_close_class(br_class_net);
		br_class_net = NULL;
		close(br_socket_fd);
		br_socket_fd = -1;
	}
}
