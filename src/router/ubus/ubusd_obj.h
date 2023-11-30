/*
 * Copyright (C) 2011 Felix Fietkau <nbd@openwrt.org>
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

#ifndef __UBUSD_OBJ_H
#define __UBUSD_OBJ_H

#include "ubusd_id.h"

extern struct avl_tree obj_types;
extern struct avl_tree objects;
extern struct avl_tree path;

struct ubus_client;
struct ubus_msg_buf;

struct ubus_object_type {
	struct ubus_id id;
	int refcount;
	struct list_head methods;
};

struct ubus_method {
	struct list_head list;
	const char *name;
	struct blob_attr data[];
};

struct ubus_subscription {
	struct list_head list, target_list;
	struct ubus_object *subscriber, *target;
};

struct ubus_object {
	struct ubus_id id;
	struct list_head list;

	struct list_head events;

	struct list_head subscribers, target_list;

	struct ubus_object_type *type;
	struct avl_node path;

	struct ubus_client *client;
	int (*recv_msg)(struct ubus_client *client, struct ubus_msg_buf *ub,
			const char *method, struct blob_attr *msg);

	int event_seen;
	unsigned int invoke_seq;
};

struct ubus_object *ubusd_create_object(struct ubus_client *cl, struct blob_attr **attr);
struct ubus_object *ubusd_create_object_internal(struct ubus_object_type *type, uint32_t id);
void ubusd_free_object(struct ubus_object *obj);

static inline struct ubus_object *ubusd_find_object(uint32_t objid)
{
	struct ubus_object *obj;
	struct ubus_id *id;

	id = ubus_find_id(&objects, objid);
	if (!id)
		return NULL;

	obj = container_of(id, struct ubus_object, id);
	return obj;
}

void ubus_subscribe(struct ubus_object *obj, struct ubus_object *target);
void ubus_unsubscribe(struct ubus_subscription *s);
void ubus_notify_unsubscribe(struct ubus_subscription *s);
void ubus_notify_subscription(struct ubus_object *obj);

#endif
