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

#ifndef __APMGR_REMOTE_H
#define __APMGR_REMOTE_H

#include <libubox/blob.h>

enum {
	APMSG_ID,
	APMSG_SEQ,
	APMSG_NODES,
	APMSG_HOST_INFO,
	__APMSG_MAX
};

struct apmsg {
	uint32_t id;
	uint32_t seq;
	struct blob_attr *nodes;
	struct blob_attr *host_info;
};

enum {
	APMSG_NODE_NAME,
	APMSG_NODE_FREQ,
	APMSG_NODE_N_ASSOC,
	APMSG_NODE_STATIONS,
	APMSG_NODE_NOISE,
	APMSG_NODE_LOAD,
	APMSG_NODE_SSID,
	APMSG_NODE_MAX_ASSOC,
	APMSG_NODE_RRM_NR,
	APMSG_NODE_NODE_INFO,
	APMSG_NODE_BSSID,
	APMSG_NODE_CHANNEL,
	APMSG_NODE_OP_CLASS,
	__APMSG_NODE_MAX
};

struct apmsg_node {
	const char *name;
	const char *ssid;
	const char *bssid;
	int freq;
	int channel;
	int op_class;
	int n_assoc;
	int max_assoc;
	int noise;
	int load;
	struct blob_attr *stations;
	struct blob_attr *rrm_nr;
	struct blob_attr *node_info;
};

enum {
	APMSG_STA_ADDR,
	APMSG_STA_SIGNAL,
	APMSG_STA_TIMEOUT,
	APMSG_STA_SEEN,
	APMSG_STA_CONNECTED,
	APMSG_STA_LAST_CONNECTED,
	__APMSG_STA_MAX
};

struct apmsg_sta {
	uint8_t addr[6];

	bool connected;
	int signal;
	int timeout;
	int seen;
	int last_connected;
};

bool parse_apmsg(struct apmsg *msg, struct blob_attr *data);
bool parse_apmsg_node(struct apmsg_node *msg, struct blob_attr *data);
bool parse_apmsg_sta(struct apmsg_sta *msg, struct blob_attr *data);

#endif
