/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2020 embedd.ch 
 *   Copyright (C) 2020 Felix Fietkau <nbd@nbd.name> 
 *   Copyright (C) 2020 John Crispin <john@phrozen.org> 
 */

#ifndef __APMGR_NODE_H
#define __APMGR_NODE_H

#include "usteer.h"

enum local_req_state {
	REQ_IDLE,
	REQ_CLIENTS,
	REQ_STATUS,
	REQ_RRM_SET_LIST,
	REQ_RRM_GET_OWN,
	__REQ_MAX
};

struct usteer_local_node {
	struct usteer_node node;

	struct ubus_subscriber ev;
	struct uloop_timeout update;

	const char *iface;
	int ifindex;
	int wiphy;

	struct ubus_request req;
	struct uloop_timeout req_timer;
	int req_state;

	uint32_t obj_id;

	float load_ewma;
	int load_thr_count;

	uint64_t time, time_busy;

	struct kvlist node_info;

	struct uloop_timeout bss_tm_queries_timeout;
	struct list_head bss_tm_queries;

	int beacon_interval;

	uint16_t band_steering_interval;

	struct {
		bool present;
		struct uloop_timeout update;
	} nl80211;
	struct {
		struct ubus_request req;
		bool req_pending;
		bool status_complete;
	} netifd;

	unsigned int link_measurement_tries;
};

struct interface;

struct usteer_remote_host {
	struct avl_node avl;

	struct list_head nodes;
	struct blob_attr *host_info;
	char *addr;
};

struct usteer_remote_node {
	struct list_head list;
	struct list_head host_list;
	const char *name;

	struct usteer_remote_host *host;
	struct usteer_node node;

	int check;
};

extern struct avl_tree local_nodes;
extern struct list_head remote_nodes;
extern struct avl_tree remote_hosts;

#define for_each_local_node(node)			\
	avl_for_each_element(&local_nodes, node, avl)	\
		if (!node->disabled)

#define for_each_remote_node(rn)			\
	list_for_each_entry(rn, &remote_nodes, list)

#endif
