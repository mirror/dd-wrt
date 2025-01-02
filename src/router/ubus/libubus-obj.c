/*
 * Copyright (C) 2011-2012 Felix Fietkau <nbd@openwrt.org>
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
#include "libubus.h"
#include "libubus-internal.h"

static void
ubus_process_unsubscribe(struct ubus_context *ctx, struct ubus_msghdr *hdr,
			 struct ubus_object *obj, struct blob_attr **attrbuf, int fd)
{
	struct ubus_subscriber *s;

	if (!obj || !attrbuf[UBUS_ATTR_TARGET])
		return;

	if (obj->methods != &watch_method)
		return;

	s = container_of(obj, struct ubus_subscriber, obj);
	if (s->remove_cb)
		s->remove_cb(ctx, s, blob_get_u32(attrbuf[UBUS_ATTR_TARGET]));

	if (fd >= 0)
		close(fd);
}

static void
ubus_process_notify(struct ubus_context *ctx, struct ubus_msghdr *hdr,
		    struct ubus_object *obj, struct blob_attr **attrbuf, int fd)
{
	if (!obj || !attrbuf[UBUS_ATTR_ACTIVE])
		return;

	obj->has_subscribers = blob_get_u8(attrbuf[UBUS_ATTR_ACTIVE]);
	if (obj->subscribe_cb)
		obj->subscribe_cb(ctx, obj);

	if (fd >= 0)
		close(fd);
}
static void
ubus_process_invoke(struct ubus_context *ctx, struct ubus_msghdr *hdr,
		    struct ubus_object *obj, struct blob_attr **attrbuf, int fd)
{
	struct ubus_request_data req = {
		.fd = -1,
		.req_fd = fd,
	};
	ubus_handler_t handler;
	int method;
	int ret;
	bool no_reply = false;

	if ((!obj && !ubus_context_is_channel(ctx)) ||
	    (!ctx->request_handler && ubus_context_is_channel(ctx))) {
		ret = UBUS_STATUS_NOT_FOUND;
		goto send;
	}

	if (!attrbuf[UBUS_ATTR_METHOD]) {
		ret = UBUS_STATUS_INVALID_ARGUMENT;
		goto send;
	}

	if (attrbuf[UBUS_ATTR_NO_REPLY])
		no_reply = blob_get_int8(attrbuf[UBUS_ATTR_NO_REPLY]);

	req.peer = hdr->peer;
	req.seq = hdr->seq;

	if (ubus_context_is_channel(ctx)) {
		handler = ctx->request_handler;
		goto found;
	}

	req.object = obj->id;
	if (attrbuf[UBUS_ATTR_USER] && attrbuf[UBUS_ATTR_GROUP]) {
		req.acl.user = blobmsg_get_string(attrbuf[UBUS_ATTR_USER]);
		req.acl.group = blobmsg_get_string(attrbuf[UBUS_ATTR_GROUP]);
		req.acl.object = obj->name;
	}
	for (method = 0; method < obj->n_methods; method++)
		if (!obj->methods[method].name ||
		    !strcmp(obj->methods[method].name,
		            blob_data(attrbuf[UBUS_ATTR_METHOD]))) {
			handler = obj->methods[method].handler;
			goto found;
		}

	/* not found */
	ret = UBUS_STATUS_METHOD_NOT_FOUND;
	goto send;

found:
	if (!attrbuf[UBUS_ATTR_DATA]) {
		ret = UBUS_STATUS_INVALID_ARGUMENT;
		goto send;
	}

	ret = handler(ctx, obj, &req, blob_data(attrbuf[UBUS_ATTR_METHOD]),
		      attrbuf[UBUS_ATTR_DATA]);
	if (req.req_fd >= 0)
		close(req.req_fd);
	if (req.deferred || no_reply)
		return;

send:
	ubus_complete_deferred_request(ctx, &req, ret);
}


void __hidden ubus_process_obj_msg(struct ubus_context *ctx, struct ubus_msghdr_buf *buf, int fd)
{
	void (*cb)(struct ubus_context *, struct ubus_msghdr *,
		   struct ubus_object *, struct blob_attr **, int fd);
	struct ubus_msghdr *hdr = &buf->hdr;
	struct blob_attr **attrbuf;
	struct ubus_object *obj;
	uint32_t objid;
	void *prev_data = NULL;
	attrbuf = ubus_parse_msg(buf->data, blob_raw_len(buf->data));
	if (!attrbuf[UBUS_ATTR_OBJID])
		return;

	objid = blob_get_u32(attrbuf[UBUS_ATTR_OBJID]);
	obj = avl_find_element(&ctx->objects, &objid, obj, avl);

	switch (hdr->type) {
	case UBUS_MSG_INVOKE:
		cb = ubus_process_invoke;
		break;
	case UBUS_MSG_UNSUBSCRIBE:
		cb = ubus_process_unsubscribe;
		break;
	case UBUS_MSG_NOTIFY:
		cb = ubus_process_notify;
		break;
	default:
		return;
	}

	if (buf == &ctx->msgbuf) {
		prev_data = buf->data;
		buf->data = NULL;
	}

	cb(ctx, hdr, obj, attrbuf, fd);

	if (prev_data) {
		if (buf->data)
			free(prev_data);
		else
			buf->data = prev_data;
	}
}

static void ubus_add_object_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct ubus_object *obj = req->priv;
	struct blob_attr **attrbuf = ubus_parse_msg(msg, blob_raw_len(msg));

	if (!attrbuf[UBUS_ATTR_OBJID])
		return;

	obj->id = blob_get_u32(attrbuf[UBUS_ATTR_OBJID]);

	if (attrbuf[UBUS_ATTR_OBJTYPE])
		obj->type->id = blob_get_u32(attrbuf[UBUS_ATTR_OBJTYPE]);

	obj->avl.key = &obj->id;
	avl_insert(&req->ctx->objects, &obj->avl);
}

static void ubus_push_method_data(const struct ubus_method *m)
{
	void *mtbl;
	int i;

	mtbl = blobmsg_open_table(&b, m->name);

	for (i = 0; i < m->n_policy; i++) {
		if (m->mask && !(m->mask & (1 << i)))
			continue;

		blobmsg_add_u32(&b, m->policy[i].name, m->policy[i].type);
	}

	blobmsg_close_table(&b, mtbl);
}

static bool ubus_push_object_type(const struct ubus_object_type *type)
{
	void *s;
	int i;

	s = blob_nest_start(&b, UBUS_ATTR_SIGNATURE);

	for (i = 0; i < type->n_methods; i++)
		ubus_push_method_data(&type->methods[i]);

	blob_nest_end(&b, s);

	return true;
}

int ubus_add_object(struct ubus_context *ctx, struct ubus_object *obj)
{
	struct ubus_request req;
	int ret;

	if (ubus_context_is_channel(ctx))
		return UBUS_STATUS_INVALID_ARGUMENT;

	blob_buf_init(&b, 0);

	if (obj->name && obj->type) {
		blob_put_string(&b, UBUS_ATTR_OBJPATH, obj->name);

		if (obj->type->id)
			blob_put_int32(&b, UBUS_ATTR_OBJTYPE, obj->type->id);
		else if (!ubus_push_object_type(obj->type))
			return UBUS_STATUS_INVALID_ARGUMENT;
	}

	if (ubus_start_request(ctx, &req, b.head, UBUS_MSG_ADD_OBJECT, 0) < 0)
		return UBUS_STATUS_INVALID_ARGUMENT;

	req.raw_data_cb = ubus_add_object_cb;
	req.priv = obj;
	ret = ubus_complete_request(ctx, &req, 0);
	if (ret)
		return ret;

	if (!obj->id)
		return UBUS_STATUS_NO_DATA;

	return 0;
}

static void ubus_remove_object_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct ubus_object *obj = req->priv;
	struct blob_attr **attrbuf = ubus_parse_msg(msg, blob_raw_len(msg));

	if (!attrbuf[UBUS_ATTR_OBJID])
		return;

	avl_delete(&req->ctx->objects, &obj->avl);

	obj->id = 0;

	if (attrbuf[UBUS_ATTR_OBJTYPE] && obj->type)
		obj->type->id = 0;
}

int ubus_remove_object(struct ubus_context *ctx, struct ubus_object *obj)
{
	struct ubus_request req;
	int ret;

	if (ubus_context_is_channel(ctx))
		return UBUS_STATUS_INVALID_ARGUMENT;

	blob_buf_init(&b, 0);
	blob_put_int32(&b, UBUS_ATTR_OBJID, obj->id);

	if (ubus_start_request(ctx, &req, b.head, UBUS_MSG_REMOVE_OBJECT, 0) < 0)
		return UBUS_STATUS_INVALID_ARGUMENT;

	req.raw_data_cb = ubus_remove_object_cb;
	req.priv = obj;
	ret = ubus_complete_request(ctx, &req, 0);
	if (ret)
		return ret;

	if (obj->id)
		return UBUS_STATUS_NO_DATA;

	return 0;
}
