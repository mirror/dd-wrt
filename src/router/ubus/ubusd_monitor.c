/*
 * Copyright (C) 2015 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "ubusd.h"

static struct ubus_object *monitor_obj;
static LIST_HEAD(monitors);

struct ubus_monitor {
	struct list_head list;
	struct ubus_client *cl;
	uint32_t seq;
};

static void
ubusd_monitor_free(struct ubus_monitor *m)
{
	list_del(&m->list);
	free(m);
}

static bool
ubusd_monitor_connect(struct ubus_client *cl, struct ubus_msg_buf *ub)
{
	struct ubus_monitor *m;

	ubusd_monitor_disconnect(cl);

	m = calloc(1, sizeof(*m));
	if (!m)
		return false;

	m->cl = cl;
	list_add_tail(&m->list, &monitors);

	return true;
}

static struct ubus_monitor*
ubusd_monitor_find(struct ubus_client *cl)
{
	struct ubus_monitor *m, *tmp;

	list_for_each_entry_safe(m, tmp, &monitors, list) {
		if (m->cl != cl)
			continue;

		return m;
	}

	return NULL;
}

void
ubusd_monitor_disconnect(struct ubus_client *cl)
{
	struct ubus_monitor *m;

	m = ubusd_monitor_find(cl);
	if (!m)
		return;

	ubusd_monitor_free(m);
}

void
ubusd_monitor_message(struct ubus_client *cl, struct ubus_msg_buf *ub, bool send)
{
	static struct blob_buf mb;
	struct ubus_monitor *m;

	if (list_empty(&monitors))
		return;

	blob_buf_init(&mb, 0);
	blob_put_int32(&mb, UBUS_MONITOR_CLIENT, cl->id.id);
	blob_put_int32(&mb, UBUS_MONITOR_PEER, ub->hdr.peer);
	blob_put_int32(&mb, UBUS_MONITOR_SEQ, ub->hdr.seq);
	blob_put_int32(&mb, UBUS_MONITOR_TYPE, ub->hdr.type);
	blob_put_int8(&mb, UBUS_MONITOR_SEND, send);
	blob_put(&mb, UBUS_MONITOR_DATA, blob_data(ub->data), blob_len(ub->data));

	ub = ubus_msg_new(mb.head, blob_raw_len(mb.head), true);
	ub->hdr.type = UBUS_MSG_MONITOR;

	list_for_each_entry(m, &monitors, list) {
		ub->hdr.seq = ++m->seq;
		ubus_msg_send(m->cl, ub);
	}

	ubus_msg_free(ub);
}

static int
ubusd_monitor_recv(struct ubus_client *cl, struct ubus_msg_buf *ub,
		   const char *method, struct blob_attr *msg)
{
	/* Only root is allowed for now */
	if (cl->uid != 0 || cl->gid != 0)
		return UBUS_STATUS_PERMISSION_DENIED;

	if (!strcmp(method, "add")) {
		if (!ubusd_monitor_connect(cl, ub))
			return UBUS_STATUS_UNKNOWN_ERROR;

		return UBUS_STATUS_OK;
	}

	if (!strcmp(method, "remove")) {
		ubusd_monitor_disconnect(cl);
		return UBUS_STATUS_OK;
	}

	return UBUS_STATUS_METHOD_NOT_FOUND;
}

void
ubusd_monitor_init(void)
{
	monitor_obj = ubusd_create_object_internal(NULL, UBUS_SYSTEM_OBJECT_MONITOR);
	if (monitor_obj != NULL)
		monitor_obj->recv_msg = ubusd_monitor_recv;
}
