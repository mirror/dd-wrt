/*
 * $Id: libbridge_devif.c,v 1.1 2005/09/28 11:53:38 seg Exp $
 *
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
#include <string.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "libbridge.h"
#include "libbridge_private.h"

int br_device_ioctl(struct bridge *br, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
	unsigned long args[4];
	struct ifreq ifr;

	args[0] = arg0;
	args[1] = arg1;
	args[2] = arg2;
	args[3] = arg3;

	memcpy(ifr.ifr_name, br->ifname, IFNAMSIZ);
	((unsigned long *)(&ifr.ifr_data))[0] = (unsigned long)args;

	return ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
}

int br_add_interface(struct bridge *br, int ifindex)
{
	if (br_device_ioctl(br, BRCTL_ADD_IF, ifindex, 0, 0) < 0)
		return errno;

	return 0;
}

int br_del_interface(struct bridge *br, int ifindex)
{
	if (br_device_ioctl(br, BRCTL_DEL_IF, ifindex, 0, 0) < 0)
		return errno;

	return 0;
}

int br_set_bridge_forward_delay(struct bridge *br, struct timeval *tv)
{
	unsigned long jif = __tv_to_jiffies(tv);

	if (br_device_ioctl(br, BRCTL_SET_BRIDGE_FORWARD_DELAY,
			    jif, 0, 0) < 0)
		return errno;

	return 0;
}

int br_set_bridge_hello_time(struct bridge *br, struct timeval *tv)
{
	unsigned long jif = __tv_to_jiffies(tv);

	if (br_device_ioctl(br, BRCTL_SET_BRIDGE_HELLO_TIME, jif, 0, 0) < 0)
		return errno;

	return 0;
}

int br_set_bridge_max_age(struct bridge *br, struct timeval *tv)
{
	unsigned long jif = __tv_to_jiffies(tv);

	if (br_device_ioctl(br, BRCTL_SET_BRIDGE_MAX_AGE, jif, 0, 0) < 0)
		return errno;

	return 0;
}

int br_set_ageing_time(struct bridge *br, struct timeval *tv)
{
	unsigned long jif = __tv_to_jiffies(tv);

	if (br_device_ioctl(br, BRCTL_SET_AGEING_TIME, jif, 0, 0) < 0)
		return errno;

	return 0;
}

int br_set_gc_interval(struct bridge *br, struct timeval *tv)
{
	unsigned long jif = __tv_to_jiffies(tv);

	if (br_device_ioctl(br, BRCTL_SET_GC_INTERVAL, jif, 0, 0) < 0)
		return errno;

	return 0;
}

int br_set_stp_state(struct bridge *br, int stp_state)
{
	if (br_device_ioctl(br, BRCTL_SET_BRIDGE_STP_STATE, stp_state,
			    0, 0) < 0)
		return errno;

	return 0;
}

int br_set_bridge_priority(struct bridge *br, int bridge_priority)
{
	if (br_device_ioctl(br, BRCTL_SET_BRIDGE_PRIORITY, bridge_priority,
			    0, 0) < 0)
		return errno;

	return 0;
}

int br_set_port_priority(struct port *p, int port_priority)
{
	if (br_device_ioctl(p->parent, BRCTL_SET_PORT_PRIORITY, p->index,
			    port_priority, 0) < 0)
		return errno;

	return 0;
}

int br_set_path_cost(struct port *p, int path_cost)
{
	if (br_device_ioctl(p->parent, BRCTL_SET_PATH_COST, p->index,
			    path_cost, 0) < 0)
		return errno;

	return 0;
}

void __copy_fdb(struct fdb_entry *ent, struct __fdb_entry *f)
{
	memcpy(ent->mac_addr, f->mac_addr, 6);
	ent->port_no = f->port_no;
	ent->is_local = f->is_local;
	__jiffies_to_tv(&ent->ageing_timer_value, f->ageing_timer_value);
}

int br_read_fdb(struct bridge *br, struct fdb_entry *fdbs, int offset, int num)
{
	struct __fdb_entry f[num];
	int i;
	int numread;

	if ((numread = br_device_ioctl(br, BRCTL_GET_FDB_ENTRIES,
				       (unsigned long)f, num, offset)) < 0)
		return errno;

	for (i=0;i<numread;i++)
		__copy_fdb(fdbs+i, f+i);

	return numread;
}
