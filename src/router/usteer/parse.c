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

#include "usteer.h"
#include "remote.h"

bool parse_apmsg(struct apmsg *msg, struct blob_attr *data)
{
	static const struct blob_attr_info policy[__APMSG_MAX] = {
		[APMSG_ID] = { .type = BLOB_ATTR_INT32 },
		[APMSG_SEQ] = { .type = BLOB_ATTR_INT32 },
		[APMSG_NODES] = { .type = BLOB_ATTR_NESTED },
		[APMSG_HOST_INFO] = { .type = BLOB_ATTR_NESTED },
	};
	struct blob_attr *tb[__APMSG_MAX];

	blob_parse(data, tb, policy, __APMSG_MAX);
	if (!tb[APMSG_ID] || !tb[APMSG_SEQ] || !tb[APMSG_NODES])
		return false;

	msg->id = blob_get_int32(tb[APMSG_ID]);
	msg->seq = blob_get_int32(tb[APMSG_SEQ]);
	msg->nodes = tb[APMSG_NODES];
	msg->host_info = tb[APMSG_HOST_INFO];

	return true;
}

static int
get_int32(struct blob_attr *attr)
{
	if (!attr)
		return 0;

	return blob_get_int32(attr);
}

bool parse_apmsg_node(struct apmsg_node *msg, struct blob_attr *data)
{
	static const struct blob_attr_info policy[__APMSG_NODE_MAX] = {
		[APMSG_NODE_NAME] = { .type = BLOB_ATTR_STRING },
		[APMSG_NODE_BSSID] = { .type = BLOB_ATTR_BINARY },
		[APMSG_NODE_FREQ] = { .type = BLOB_ATTR_INT32 },
		[APMSG_NODE_N_ASSOC] = { .type = BLOB_ATTR_INT32 },
		[APMSG_NODE_MAX_ASSOC] = { .type = BLOB_ATTR_INT32 },
		[APMSG_NODE_STATIONS] = { .type = BLOB_ATTR_NESTED },
		[APMSG_NODE_NOISE] = { .type = BLOB_ATTR_INT32 },
		[APMSG_NODE_LOAD] = { .type = BLOB_ATTR_INT32 },
		[APMSG_NODE_RRM_NR] = { .type = BLOB_ATTR_NESTED },
		[APMSG_NODE_NODE_INFO] = { .type = BLOB_ATTR_NESTED },
		[APMSG_NODE_CHANNEL] = { .type = BLOB_ATTR_INT32 },
		[APMSG_NODE_OP_CLASS] = { .type = BLOB_ATTR_INT32 },
		[APMSG_NODE_N] = { .type = BLOB_ATTR_INT32 },
		[APMSG_NODE_VHT] = { .type = BLOB_ATTR_INT32 },
		[APMSG_NODE_HE] = { .type = BLOB_ATTR_INT32 },
	};
	struct blob_attr *tb[__APMSG_NODE_MAX];
	struct blob_attr *cur;

	blob_parse(data, tb, policy, __APMSG_NODE_MAX);
	if (!tb[APMSG_NODE_NAME] ||
	    !tb[APMSG_NODE_BSSID] ||
	    blob_len(tb[APMSG_NODE_BSSID]) != 6 ||
	    !tb[APMSG_NODE_FREQ] ||
	    !tb[APMSG_NODE_N_ASSOC] ||
	    !tb[APMSG_NODE_STATIONS] ||
	    !tb[APMSG_NODE_SSID])
		return false;

	msg->name = blob_data(tb[APMSG_NODE_NAME]);
	msg->n_assoc = blob_get_int32(tb[APMSG_NODE_N_ASSOC]);
	msg->freq = blob_get_int32(tb[APMSG_NODE_FREQ]);
	msg->stations = tb[APMSG_NODE_STATIONS];
	msg->ssid = blob_data(tb[APMSG_NODE_SSID]);
	msg->bssid = blob_data(tb[APMSG_NODE_BSSID]);

	msg->noise = get_int32(tb[APMSG_NODE_NOISE]);
	msg->load = get_int32(tb[APMSG_NODE_LOAD]);
	msg->max_assoc = get_int32(tb[APMSG_NODE_MAX_ASSOC]);
	/* not mandatory */
	if (tb[APMSG_NODE_N])
	    msg->n = get_int32(tb[APMSG_NODE_N]);
	if (tb[APMSG_NODE_VHT])
	    msg->vht = get_int32(tb[APMSG_NODE_VHT]);
	if (tb[APMSG_NODE_HE])
	    msg->he = get_int32(tb[APMSG_NODE_HE]);
	if (tb[APMSG_NODE_CW])
	    msg->cw = get_int32(tb[APMSG_NODE_CW]);
	msg->rrm_nr = NULL;

	if (tb[APMSG_NODE_CHANNEL] && tb[APMSG_NODE_OP_CLASS]) {
		msg->channel = blob_get_int32(tb[APMSG_NODE_CHANNEL]);
		msg->op_class = blob_get_int32(tb[APMSG_NODE_OP_CLASS]);
	}

	cur = tb[APMSG_NODE_RRM_NR];
	if (cur && blob_len(cur) >= sizeof(struct blob_attr) &&
	    blob_len(cur) >= blob_pad_len(blob_data(cur))) {
		int rem;

		msg->rrm_nr = blob_data(cur);

		blobmsg_for_each_attr(cur, msg->rrm_nr, rem) {
			if (blobmsg_check_attr(cur, false))
				continue;
			if (blobmsg_type(cur) == BLOBMSG_TYPE_STRING)
				continue;
			msg->rrm_nr = NULL;
			break;
		}

		if (msg->rrm_nr &&
		    blobmsg_type(msg->rrm_nr) != BLOBMSG_TYPE_ARRAY)
			msg->rrm_nr = NULL;
	}

	msg->node_info = tb[APMSG_NODE_NODE_INFO];

	return true;
}

bool parse_apmsg_sta(struct apmsg_sta *msg, struct blob_attr *data)
{
	static const struct blob_attr_info policy[__APMSG_STA_MAX] = {
		[APMSG_STA_ADDR] = { .type = BLOB_ATTR_BINARY },
		[APMSG_STA_SIGNAL] = { .type = BLOB_ATTR_INT32 },
		[APMSG_STA_SEEN] = { .type = BLOB_ATTR_INT32 },
		[APMSG_STA_TIMEOUT] = { .type = BLOB_ATTR_INT32 },
		[APMSG_STA_CONNECTED] = { .type = BLOB_ATTR_INT8 },
		[APMSG_STA_LAST_CONNECTED] = { .type = BLOB_ATTR_INT32 },
	};
	struct blob_attr *tb[__APMSG_STA_MAX];

	blob_parse(data, tb, policy, __APMSG_STA_MAX);
	if (!tb[APMSG_STA_ADDR] ||
	    !tb[APMSG_STA_SIGNAL] ||
	    !tb[APMSG_STA_SEEN] ||
	    !tb[APMSG_STA_TIMEOUT] ||
	    !tb[APMSG_STA_CONNECTED] ||
	    !tb[APMSG_STA_LAST_CONNECTED])
		return false;

	if (blob_len(tb[APMSG_STA_ADDR]) != sizeof(msg->addr))
		return false;

	memcpy(msg->addr, blob_data(tb[APMSG_STA_ADDR]), sizeof(msg->addr));
	msg->signal = blob_get_int32(tb[APMSG_STA_SIGNAL]);
	msg->seen = blob_get_int32(tb[APMSG_STA_SEEN]);
	msg->timeout = blob_get_int32(tb[APMSG_STA_TIMEOUT]);
	msg->connected = blob_get_int8(tb[APMSG_STA_CONNECTED]);
	msg->last_connected = blob_get_int32(tb[APMSG_STA_LAST_CONNECTED]);

	return true;
}
