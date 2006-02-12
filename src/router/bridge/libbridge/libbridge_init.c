/*
 * $Id: libbridge_init.c,v 1.1 2005/09/28 11:53:38 seg Exp $
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
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "libbridge.h"
#include "libbridge_private.h"

int br_socket_fd;
struct bridge *bridge_list;

static void __bridge_info_copy(struct bridge_info *info, struct __bridge_info *i)
{
	memcpy(&info->designated_root, &i->designated_root, 8);
	memcpy(&info->bridge_id, &i->bridge_id, 8);
	info->root_path_cost = i->root_path_cost;
	info->topology_change = i->topology_change;
	info->topology_change_detected = i->topology_change_detected;
	info->root_port = i->root_port;
	info->stp_enabled = i->stp_enabled;
	__jiffies_to_tv(&info->max_age, i->max_age);
	__jiffies_to_tv(&info->hello_time, i->hello_time);
	__jiffies_to_tv(&info->forward_delay, i->forward_delay);
	__jiffies_to_tv(&info->bridge_max_age, i->bridge_max_age);
	__jiffies_to_tv(&info->bridge_hello_time, i->bridge_hello_time);
	__jiffies_to_tv(&info->bridge_forward_delay, i->bridge_forward_delay);
	__jiffies_to_tv(&info->ageing_time, i->ageing_time);
	__jiffies_to_tv(&info->gc_interval, i->gc_interval);
	__jiffies_to_tv(&info->hello_timer_value, i->hello_timer_value);
	__jiffies_to_tv(&info->tcn_timer_value, i->tcn_timer_value);
	__jiffies_to_tv(&info->topology_change_timer_value,
			i->topology_change_timer_value);
	__jiffies_to_tv(&info->gc_timer_value, i->gc_timer_value);
}

static void __port_info_copy(struct port_info *info, struct __port_info *i)
{
	memcpy(&info->designated_root, &i->designated_root, 8);
	memcpy(&info->designated_bridge, &i->designated_bridge, 8);
	info->port_id = i->port_id;
	info->designated_port = i->designated_port;
	info->path_cost = i->path_cost;
	info->designated_cost = i->designated_cost;
	info->state = i->state;
	info->top_change_ack = i->top_change_ack;
	info->config_pending = i->config_pending;
	__jiffies_to_tv(&info->message_age_timer_value,
			i->message_age_timer_value);
	__jiffies_to_tv(&info->forward_delay_timer_value,
			i->forward_delay_timer_value);
	__jiffies_to_tv(&info->hold_timer_value,
			i->hold_timer_value);
}

int br_read_info(struct bridge *br)
{
	struct __bridge_info i;

	if (if_indextoname(br->ifindex, br->ifname) == NULL)
		return 1;

	if (br_device_ioctl(br, BRCTL_GET_BRIDGE_INFO,
			    (unsigned long)&i, 0, 0) < 0)
		return 1;

	__bridge_info_copy(&br->info, &i);
	return 0;
}

int br_read_port_info(struct port *p)
{
	struct __port_info i;

	if (br_device_ioctl(p->parent, BRCTL_GET_PORT_INFO,
			    (unsigned long)&i, p->index, 0) < 0)
		return errno;

	__port_info_copy(&p->info, &i);
	return 0;
}

void br_nuke_bridge(struct bridge *b)
{
	struct port *p;

	p = b->firstport;
	while (p != NULL) {
		struct port *pnext;

		pnext = p->next;
		free(p);
		p = pnext;
	}

	free(b);
}

int br_make_port_list(struct bridge *br)
{
	int err;
	int i;
	int ifindices[256];

	if (br_device_ioctl(br, BRCTL_GET_PORT_LIST, (unsigned long)ifindices,
			    0, 0) < 0)
		return errno;

	for (i=255;i>=0;i--) {
		struct port *p;

		if (!ifindices[i])
			continue;

		p = malloc(sizeof(struct port));
		p->index = i;
		p->ifindex = ifindices[i];
		p->parent = br;
		br->ports[i] = p;
		p->next = br->firstport;
		br->firstport = p;
		if ((err = br_read_port_info(p)) != 0)
			goto error_out;
	}

	return 0;

 error_out:
	while (++i < 256)
		free(br->ports[i]);

	return err;
}

int br_make_bridge_list()
{
	int err;
	int i;
	int ifindices[32];
	int num;

	num = br_ioctl(BRCTL_GET_BRIDGES, (unsigned long)ifindices, 32);
	if (num < 0)
		return errno;

	bridge_list = NULL;
	for (i=0;i<num;i++) {
		struct bridge *br;

		br = malloc(sizeof(struct bridge));
		memset(br, 0, sizeof(struct bridge));
		br->ifindex = ifindices[i];
		br->firstport = NULL;
		br->next = bridge_list;
		bridge_list = br;
		if ((err = br_read_info(br)) != 0)
			goto error_out;
		if ((err = br_make_port_list(br)) != 0)
			goto error_out;
	}

	return 0;

 error_out:
	while (bridge_list != NULL) {
		struct bridge *nxt;

		nxt = bridge_list->next;
		br_nuke_bridge(bridge_list);
		bridge_list = nxt;
	}

	return err;
}

int br_init()
{
	int err;

	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return errno;

	if (br_get_version() != BRCTL_VERSION)
		return 12345;

	if ((err = br_make_bridge_list()) != 0)
		return err;

	return 0;
}

int br_refresh()
{
	struct bridge *b;

	b = bridge_list;
	while (b != NULL) {
		struct bridge *bnext;

		bnext = b->next;
		br_nuke_bridge(b);
		b = bnext;
	}

	return br_make_bridge_list();
}
