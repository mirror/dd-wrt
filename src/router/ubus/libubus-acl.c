/*
 * Copyright (C) 2015 John Cripin <blogic@openwrt.org>
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

#include <unistd.h>

#include <libubox/blob.h>
#include <libubox/blobmsg.h>

#include "libubus.h"
#include <libubox/avl-cmp.h>

static struct ubus_event_handler acl_event;
static struct ubus_request acl_req;
static struct blob_attr *acl_blob;

static int acl_cmp(const void *k1, const void *k2, void *ptr)
{
	const struct ubus_acl_key *key1 = k1;
	const struct ubus_acl_key *key2 = k2;
	int ret = 0;

	if (key1->user && key2->user)
		ret = strcmp(key1->user, key2->user);
	if (ret)
		return ret;

	if (key1->group && key2->group)
		ret = strcmp(key1->group, key2->group);
	if (ret)
		return ret;

	return strcmp(key1->object, key2->object);
}

AVL_TREE(acl_objects, acl_cmp, true, NULL);

enum {
	ACL_OBJ_OBJECT,
	ACL_OBJ_USER,
	ACL_OBJ_GROUP,
	ACL_OBJ_ACL,
	__ACL_OBJ_MAX
};

static const struct blobmsg_policy acl_obj_policy[__ACL_OBJ_MAX] = {
	[ACL_OBJ_OBJECT] = { .name = "obj", .type = BLOBMSG_TYPE_STRING },
	[ACL_OBJ_USER] = { .name = "user", .type = BLOBMSG_TYPE_STRING },
	[ACL_OBJ_GROUP] = { .name = "group", .type = BLOBMSG_TYPE_STRING },
	[ACL_OBJ_ACL] = { .name = "acl", .type = BLOBMSG_TYPE_TABLE },
};

static void
acl_add(struct blob_attr *obj)
{
	struct blob_attr *tb[__ACL_OBJ_MAX];
	struct acl_object *acl;

	blobmsg_parse(acl_obj_policy, __ACL_OBJ_MAX, tb, blobmsg_data(obj),
		      blobmsg_data_len(obj));

	if (!tb[ACL_OBJ_OBJECT] || !tb[ACL_OBJ_ACL])
		return;

	if (!tb[ACL_OBJ_USER] && !tb[ACL_OBJ_GROUP])
		return;

	acl = calloc(1, sizeof(*acl));
	if (!acl)
		return;

	acl->avl.key = &acl->key;
	acl->key.object = blobmsg_get_string(tb[ACL_OBJ_OBJECT]);
	acl->key.user = blobmsg_get_string(tb[ACL_OBJ_USER]);
	acl->key.group = blobmsg_get_string(tb[ACL_OBJ_GROUP]);
	acl->acl = tb[ACL_OBJ_ACL];
	avl_insert(&acl_objects, &acl->avl);
}

enum {
	ACL_POLICY_SEQ,
	ACL_POLICY_ACL,
	__ACL_POLICY_MAX
};

static const struct blobmsg_policy acl_policy[__ACL_POLICY_MAX] = {
	[ACL_POLICY_SEQ] = { .name = "seq", .type = BLOBMSG_TYPE_INT32 },
	[ACL_POLICY_ACL] = { .name = "acl", .type = BLOBMSG_TYPE_ARRAY },
};

static void acl_recv_cb(struct ubus_request *req,
			int type, struct blob_attr *msg)
{
	struct blob_attr *tb[__ACL_POLICY_MAX];
	struct blob_attr *cur;
	size_t rem;

	if (acl_blob) {
		struct acl_object *p, *q;

		avl_for_each_element_safe(&acl_objects, p, avl, q) {
			avl_delete(&acl_objects, &p->avl);
			free(p);
		}
		free(acl_blob);
	}
	acl_blob = blob_memdup(msg);
	blobmsg_parse(acl_policy, __ACL_POLICY_MAX, tb, blobmsg_data(msg),
		      blobmsg_data_len(msg));

	if (!tb[ACL_POLICY_SEQ] && !tb[ACL_POLICY_ACL])
		return;

	blobmsg_for_each_attr(cur, tb[ACL_POLICY_ACL], rem)
		acl_add(cur);
}

static void acl_query(struct ubus_context *ctx)
{
	ubus_invoke_async(ctx, UBUS_SYSTEM_OBJECT_ACL, "query", NULL, &acl_req);
	acl_req.data_cb = acl_recv_cb;
	ubus_complete_request_async(ctx, &acl_req);
}

static void acl_subscribe_cb(struct ubus_context *ctx, struct ubus_event_handler *ev,
		const char *type, struct blob_attr *msg)
{
	if (strcmp(type, "ubus.acl.sequence"))
		return;
	acl_query(ctx);
}

int ubus_register_acl(struct ubus_context *ctx)
{
	int ret;

	acl_event.cb = acl_subscribe_cb;

	ret = ubus_register_event_handler(ctx, &acl_event, "ubus.acl.sequence");
	if (!ret)
		acl_query(ctx);

	return ret;
}
