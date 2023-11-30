/*
 * Copyright (C) 2011-2014 Felix Fietkau <nbd@openwrt.org>
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

#include <arpa/inet.h>
#include <unistd.h>

#include "ubusd.h"

struct blob_buf b;
static struct avl_tree clients;

static struct blob_attr *attrbuf[UBUS_ATTR_MAX];

typedef int (*ubus_cmd_cb)(struct ubus_client *cl, struct ubus_msg_buf *ub, struct blob_attr **attr);

static const struct blob_attr_info ubus_policy[UBUS_ATTR_MAX] = {
	[UBUS_ATTR_SIGNATURE] = { .type = BLOB_ATTR_NESTED },
	[UBUS_ATTR_OBJTYPE] = { .type = BLOB_ATTR_INT32 },
	[UBUS_ATTR_OBJPATH] = { .type = BLOB_ATTR_STRING },
	[UBUS_ATTR_OBJID] = { .type = BLOB_ATTR_INT32 },
	[UBUS_ATTR_STATUS] = { .type = BLOB_ATTR_INT32 },
	[UBUS_ATTR_METHOD] = { .type = BLOB_ATTR_STRING },
	[UBUS_ATTR_USER] = { .type = BLOB_ATTR_STRING },
	[UBUS_ATTR_GROUP] = { .type = BLOB_ATTR_STRING },
};

struct blob_attr **ubus_parse_msg(struct blob_attr *msg, size_t len)
{
	blob_parse_untrusted(msg, len, attrbuf, ubus_policy, UBUS_ATTR_MAX);
	return attrbuf;
}

static void ubus_msg_close_fd(struct ubus_msg_buf *ub)
{
	if (ub->fd < 0)
		return;

	close(ub->fd);
	ub->fd = -1;
}

static void ubus_msg_init(struct ubus_msg_buf *ub, uint8_t type, uint16_t seq, uint32_t peer)
{
	ub->hdr.version = 0;
	ub->hdr.type = type;
	ub->hdr.seq = seq;
	ub->hdr.peer = peer;
}

static struct ubus_msg_buf *ubus_msg_from_blob(bool shared)
{
	return ubus_msg_new(b.head, blob_raw_len(b.head), shared);
}

static struct ubus_msg_buf *ubus_reply_from_blob(struct ubus_msg_buf *ub, bool shared)
{
	struct ubus_msg_buf *new;

	new = ubus_msg_from_blob(shared);
	if (!new)
		return NULL;

	ubus_msg_init(new, UBUS_MSG_DATA, ub->hdr.seq, ub->hdr.peer);
	return new;
}

void
ubus_proto_send_msg_from_blob(struct ubus_client *cl, struct ubus_msg_buf *ub,
			uint8_t type)
{
	/* keep the fd to be passed if it is UBUS_MSG_INVOKE */
	int fd = ub->fd;
	ub = ubus_reply_from_blob(ub, true);
	if (!ub)
		return;

	ub->hdr.type = type;
	ub->fd = fd;

	ubus_msg_send(cl, ub);
	ubus_msg_free(ub);
}

static bool ubusd_send_hello(struct ubus_client *cl)
{
	struct ubus_msg_buf *ub;

	blob_buf_init(&b, 0);
	ub = ubus_msg_from_blob(true);
	if (!ub)
		return false;

	ubus_msg_init(ub, UBUS_MSG_HELLO, 0, cl->id.id);
	ubus_msg_send(cl, ub);
	ubus_msg_free(ub);
	return true;
}

static int ubusd_send_pong(struct ubus_client *cl, struct ubus_msg_buf *ub, struct blob_attr **attr)
{
	ub->hdr.type = UBUS_MSG_DATA;
	ubus_msg_send(cl, ub);
	return 0;
}

static int ubusd_handle_remove_object(struct ubus_client *cl, struct ubus_msg_buf *ub, struct blob_attr **attr)
{
	struct ubus_object *obj;

	if (!attr[UBUS_ATTR_OBJID])
		return UBUS_STATUS_INVALID_ARGUMENT;

	obj = ubusd_find_object(blob_get_u32(attr[UBUS_ATTR_OBJID]));
	if (!obj)
		return UBUS_STATUS_NOT_FOUND;

	if (obj->client != cl)
		return UBUS_STATUS_PERMISSION_DENIED;

	blob_buf_init(&b, 0);
	blob_put_int32(&b, UBUS_ATTR_OBJID, obj->id.id);

	/* check if we're removing the object type as well */
	if (obj->type && obj->type->refcount == 1)
		blob_put_int32(&b, UBUS_ATTR_OBJTYPE, obj->type->id.id);

	ubus_proto_send_msg_from_blob(cl, ub, UBUS_MSG_DATA);
	ubusd_free_object(obj);

	return 0;
}

static int ubusd_handle_add_object(struct ubus_client *cl, struct ubus_msg_buf *ub, struct blob_attr **attr)
{
	struct ubus_object *obj;

	obj = ubusd_create_object(cl, attr);
	if (!obj)
		return UBUS_STATUS_INVALID_ARGUMENT;

	blob_buf_init(&b, 0);
	blob_put_int32(&b, UBUS_ATTR_OBJID, obj->id.id);
	if (attr[UBUS_ATTR_SIGNATURE] && obj->type)
		blob_put_int32(&b, UBUS_ATTR_OBJTYPE, obj->type->id.id);

	ubus_proto_send_msg_from_blob(cl, ub, UBUS_MSG_DATA);
	return 0;
}

static void ubusd_send_obj(struct ubus_client *cl, struct ubus_msg_buf *ub, struct ubus_object *obj)
{
	struct ubus_method *m;
	int all_cnt = 0, cnt = 0;
	void *s;

	if (!obj->type)
		return;

	blob_buf_init(&b, 0);

	blob_put_string(&b, UBUS_ATTR_OBJPATH, obj->path.key);
	blob_put_int32(&b, UBUS_ATTR_OBJID, obj->id.id);
	blob_put_int32(&b, UBUS_ATTR_OBJTYPE, obj->type->id.id);

	s = blob_nest_start(&b, UBUS_ATTR_SIGNATURE);
	list_for_each_entry(m, &obj->type->methods, list) {
		all_cnt++;
		if (!ubusd_acl_check(cl, obj->path.key, blobmsg_name(m->data), UBUS_ACL_ACCESS)) {
			blobmsg_add_blob(&b, m->data);
			cnt++;
		}
	}
	blob_nest_end(&b, s);

	if (cnt || !all_cnt)
		ubus_proto_send_msg_from_blob(cl, ub, UBUS_MSG_DATA);
}

static int ubus_client_cmd_queue_add(struct ubus_client *cl,
					struct ubus_msg_buf *msg,
					struct ubus_object *obj)
{
	struct ubus_client_cmd *cmd = malloc(sizeof(*cmd));

	if (cmd) {
		cmd->msg = msg;
		cmd->obj = obj;
		list_add_tail(&cmd->list, &cl->cmd_queue);
		return -2;
	}
	return UBUS_STATUS_UNKNOWN_ERROR;
}

static int __ubusd_handle_lookup(struct ubus_client *cl,
				struct ubus_msg_buf *ub,
				struct blob_attr **attr,
				struct ubus_client_cmd *cmd)
{
	struct ubus_object *obj = NULL;
	char *objpath;
	bool found = false;
	int len;

	if (!attr[UBUS_ATTR_OBJPATH]) {
		if (cmd)
			obj = cmd->obj;

		/* Start from beginning or continue from the last object */
		if (obj == NULL)
			obj = avl_first_element(&path, obj, path);

		avl_for_element_range(obj, avl_last_element(&path, obj, path), obj, path) {
			/* Keep sending objects until buffering starts */
			if (list_empty(&cl->tx_queue)) {
				ubusd_send_obj(cl, ub, obj);
			} else {
				/* Queue command and continue on the next call */
				int ret;

				if (cmd == NULL) {
					ret = ubus_client_cmd_queue_add(cl, ub, obj);
				} else {
					cmd->obj = obj;
					ret = -2;
				}
				return ret;
			}
		}
		return 0;
	}

	objpath = blob_data(attr[UBUS_ATTR_OBJPATH]);
	len = strlen(objpath);
	if (objpath[len - 1] != '*') {
		obj = avl_find_element(&path, objpath, obj, path);
		if (!obj)
			return UBUS_STATUS_NOT_FOUND;

		ubusd_send_obj(cl, ub, obj);
		return 0;
	}

	objpath[--len] = 0;

	obj = avl_find_ge_element(&path, objpath, obj, path);
	if (!obj)
		return UBUS_STATUS_NOT_FOUND;

	while (!strncmp(objpath, obj->path.key, len)) {
		found = true;
		ubusd_send_obj(cl, ub, obj);
		if (obj == avl_last_element(&path, obj, path))
			break;
		obj = avl_next_element(obj, path);
	}

	if (!found)
		return UBUS_STATUS_NOT_FOUND;

	return 0;
}

static int ubusd_handle_lookup(struct ubus_client *cl, struct ubus_msg_buf *ub, struct blob_attr **attr)
{
	int rc;

	if (list_empty(&cl->tx_queue))
		rc = __ubusd_handle_lookup(cl, ub, attr, NULL);
	else
		rc = ubus_client_cmd_queue_add(cl, ub, NULL);

	return rc;
}

int ubusd_cmd_lookup(struct ubus_client *cl, struct ubus_client_cmd *cmd)
{
	struct ubus_msg_buf *ub = cmd->msg;
	struct blob_attr **attr;
	int ret;

	attr = ubus_parse_msg(ub->data, blob_raw_len(ub->data));
	ret = __ubusd_handle_lookup(cl, ub, attr, cmd);

	if (ret != -2) {
		struct ubus_msg_buf *retmsg = cl->retmsg;
		int *retmsg_data = blob_data(blob_data(retmsg->data));

		retmsg->hdr.seq = ub->hdr.seq;
		retmsg->hdr.peer = ub->hdr.peer;

		*retmsg_data = htonl(ret);
		ubus_msg_send(cl, retmsg);
	}
	return ret;
}

static void
ubusd_forward_invoke(struct ubus_client *cl, struct ubus_object *obj,
		     const char *method, struct ubus_msg_buf *ub,
		     struct blob_attr *data)
{
	blob_put_int32(&b, UBUS_ATTR_OBJID, obj->id.id);
	blob_put_string(&b, UBUS_ATTR_METHOD, method);
	if (cl->user)
		blob_put_string(&b, UBUS_ATTR_USER, cl->user);
	if (cl->group)
		blob_put_string(&b, UBUS_ATTR_GROUP, cl->group);
	if (data)
		blob_put(&b, UBUS_ATTR_DATA, blob_data(data), blob_len(data));

	ubus_proto_send_msg_from_blob(obj->client, ub, UBUS_MSG_INVOKE);
}

static int ubusd_handle_invoke(struct ubus_client *cl, struct ubus_msg_buf *ub, struct blob_attr **attr)
{
	struct ubus_object *obj = NULL;
	struct ubus_id *id;
	const char *method;

	if (!attr[UBUS_ATTR_METHOD] || !attr[UBUS_ATTR_OBJID])
		return UBUS_STATUS_INVALID_ARGUMENT;

	id = ubus_find_id(&objects, blob_get_u32(attr[UBUS_ATTR_OBJID]));
	if (!id)
		return UBUS_STATUS_NOT_FOUND;

	obj = container_of(id, struct ubus_object, id);

	method = blob_data(attr[UBUS_ATTR_METHOD]);

	if (ubusd_acl_check(cl, obj->path.key, method, UBUS_ACL_ACCESS))
		return UBUS_STATUS_PERMISSION_DENIED;

	if (!obj->client)
		return obj->recv_msg(cl, ub, method, attr[UBUS_ATTR_DATA]);

	ub->hdr.peer = cl->id.id;
	blob_buf_init(&b, 0);

	ubusd_forward_invoke(cl, obj, method, ub, attr[UBUS_ATTR_DATA]);

	return -1;
}

static int ubusd_handle_notify(struct ubus_client *cl, struct ubus_msg_buf *ub, struct blob_attr **attr)
{
	struct ubus_object *obj = NULL;
	struct ubus_subscription *s;
	struct ubus_id *id;
	const char *method;
	bool no_reply = false;
	void *c;

	if (!attr[UBUS_ATTR_METHOD] || !attr[UBUS_ATTR_OBJID])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (attr[UBUS_ATTR_NO_REPLY])
		no_reply = blob_get_int8(attr[UBUS_ATTR_NO_REPLY]);

	id = ubus_find_id(&objects, blob_get_u32(attr[UBUS_ATTR_OBJID]));
	if (!id)
		return UBUS_STATUS_NOT_FOUND;

	obj = container_of(id, struct ubus_object, id);
	if (obj->client != cl)
		return UBUS_STATUS_PERMISSION_DENIED;

	if (!no_reply) {
		blob_buf_init(&b, 0);
		blob_put_int32(&b, UBUS_ATTR_OBJID, id->id);
		c = blob_nest_start(&b, UBUS_ATTR_SUBSCRIBERS);
		list_for_each_entry(s, &obj->subscribers, list) {
			blob_put_int32(&b, 0, s->subscriber->id.id);
		}
		blob_nest_end(&b, c);
		blob_put_int32(&b, UBUS_ATTR_STATUS, 0);
		ubus_proto_send_msg_from_blob(cl, ub, UBUS_MSG_STATUS);
	}

	ub->hdr.peer = cl->id.id;
	method = blob_data(attr[UBUS_ATTR_METHOD]);
	list_for_each_entry(s, &obj->subscribers, list) {
		blob_buf_init(&b, 0);
		if (no_reply)
			blob_put_int8(&b, UBUS_ATTR_NO_REPLY, 1);
		ubusd_forward_invoke(cl, s->subscriber, method, ub, attr[UBUS_ATTR_DATA]);
	}

	return -1;
}

static struct ubus_client *ubusd_get_client_by_id(uint32_t id)
{
	struct ubus_id *clid;

	clid = ubus_find_id(&clients, id);
	if (!clid)
		return NULL;

	return container_of(clid, struct ubus_client, id);
}

static int ubusd_handle_response(struct ubus_client *cl, struct ubus_msg_buf *ub, struct blob_attr **attr)
{
	struct ubus_object *obj;

	if (!attr[UBUS_ATTR_OBJID] ||
	    (ub->hdr.type == UBUS_MSG_STATUS && !attr[UBUS_ATTR_STATUS]) ||
	    (ub->hdr.type == UBUS_MSG_DATA && !attr[UBUS_ATTR_DATA]))
		goto out;

	obj = ubusd_find_object(blob_get_u32(attr[UBUS_ATTR_OBJID]));
	if (!obj)
		goto out;

	if (cl != obj->client)
		goto out;

	cl = ubusd_get_client_by_id(ub->hdr.peer);
	if (!cl)
		goto out;

	ub->hdr.peer = blob_get_u32(attr[UBUS_ATTR_OBJID]);
	ubus_msg_send(cl, ub);
out:
	return -1;
}

static int ubusd_handle_add_watch(struct ubus_client *cl, struct ubus_msg_buf *ub, struct blob_attr **attr)
{
	struct ubus_object *obj, *target;

	if (!attr[UBUS_ATTR_OBJID] || !attr[UBUS_ATTR_TARGET])
		return UBUS_STATUS_INVALID_ARGUMENT;

	obj = ubusd_find_object(blob_get_u32(attr[UBUS_ATTR_OBJID]));
	if (!obj)
		return UBUS_STATUS_NOT_FOUND;

	if (cl != obj->client)
		return UBUS_STATUS_INVALID_ARGUMENT;

	target = ubusd_find_object(blob_get_u32(attr[UBUS_ATTR_TARGET]));
	if (!target || !target->client)
		return UBUS_STATUS_NOT_FOUND;

	if (cl == target->client)
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (!target->path.key) {
		if (strcmp(target->client->user, cl->user) && strcmp(target->client->group, cl->group))
			return UBUS_STATUS_NOT_FOUND;
	} else if (ubusd_acl_check(cl, target->path.key, NULL, UBUS_ACL_SUBSCRIBE)) {
		return UBUS_STATUS_NOT_FOUND;
	}

	ubus_subscribe(obj, target);
	return 0;
}

static int ubusd_handle_remove_watch(struct ubus_client *cl, struct ubus_msg_buf *ub, struct blob_attr **attr)
{
	struct ubus_object *obj;
	struct ubus_subscription *s;
	uint32_t id;

	if (!attr[UBUS_ATTR_OBJID] || !attr[UBUS_ATTR_TARGET])
		return UBUS_STATUS_INVALID_ARGUMENT;

	obj = ubusd_find_object(blob_get_u32(attr[UBUS_ATTR_OBJID]));
	if (!obj)
		return UBUS_STATUS_NOT_FOUND;

	if (cl != obj->client)
		return UBUS_STATUS_INVALID_ARGUMENT;

	id = blob_get_u32(attr[UBUS_ATTR_TARGET]);
	list_for_each_entry(s, &obj->target_list, target_list) {
		if (s->target->id.id != id)
			continue;

		ubus_unsubscribe(s);
		return 0;
	}

	return UBUS_STATUS_NOT_FOUND;
}

static const ubus_cmd_cb handlers[__UBUS_MSG_LAST] = {
	[UBUS_MSG_PING] = ubusd_send_pong,
	[UBUS_MSG_ADD_OBJECT] = ubusd_handle_add_object,
	[UBUS_MSG_REMOVE_OBJECT] = ubusd_handle_remove_object,
	[UBUS_MSG_LOOKUP] = ubusd_handle_lookup,
	[UBUS_MSG_INVOKE] = ubusd_handle_invoke,
	[UBUS_MSG_STATUS] = ubusd_handle_response,
	[UBUS_MSG_DATA] = ubusd_handle_response,
	[UBUS_MSG_SUBSCRIBE] = ubusd_handle_add_watch,
	[UBUS_MSG_UNSUBSCRIBE] = ubusd_handle_remove_watch,
	[UBUS_MSG_NOTIFY] = ubusd_handle_notify,
};

void ubusd_proto_receive_message(struct ubus_client *cl, struct ubus_msg_buf *ub)
{
	ubus_cmd_cb cb = NULL;
	int ret;
	struct ubus_msg_buf *retmsg = cl->retmsg;
	int *retmsg_data = blob_data(blob_data(retmsg->data));

	retmsg->hdr.seq = ub->hdr.seq;
	retmsg->hdr.peer = ub->hdr.peer;

	if (ub->hdr.type < __UBUS_MSG_LAST)
		cb = handlers[ub->hdr.type];

	if (ub->hdr.type != UBUS_MSG_STATUS && ub->hdr.type != UBUS_MSG_INVOKE)
		ubus_msg_close_fd(ub);

	/* Note: no callback should free the `ub` buffer
	         that's always done right after the callback finishes */
	if (cb)
		ret = cb(cl, ub, ubus_parse_msg(ub->data, blob_raw_len(ub->data)));
	else
		ret = UBUS_STATUS_INVALID_COMMAND;

	/* Command has not been completed yet and got queued */
	if (ret == -2)
		return;

	ubus_msg_free(ub);

	if (ret == -1)
		return;

	*retmsg_data = htonl(ret);
	ubus_msg_send(cl, retmsg);
}

static int ubusd_proto_init_retmsg(struct ubus_client *cl)
{
	struct blob_buf *b = &cl->b;

	blob_buf_init(&cl->b, 0);
	blob_put_int32(&cl->b, UBUS_ATTR_STATUS, 0);

	/* we make the 'retmsg' buffer shared with the blob_buf b, to reduce mem duplication */
	cl->retmsg = ubus_msg_new(b->head, blob_raw_len(b->head), true);
	if (!cl->retmsg)
		return -1;

	cl->retmsg->hdr.type = UBUS_MSG_STATUS;
	return 0;
}

struct ubus_client *ubusd_proto_new_client(int fd, uloop_fd_handler cb)
{
	struct ubus_client *cl;

	cl = calloc(1, sizeof(*cl));
	if (!cl)
		return NULL;

	if (ubusd_acl_init_client(cl, fd))
		goto free;

	INIT_LIST_HEAD(&cl->objects);
	INIT_LIST_HEAD(&cl->cmd_queue);
	INIT_LIST_HEAD(&cl->tx_queue);
	cl->sock.fd = fd;
	cl->sock.cb = cb;
	cl->pending_msg_fd = -1;

	if (!ubus_alloc_id(&clients, &cl->id, 0))
		goto free;

	if (ubusd_proto_init_retmsg(cl))
		goto free;

	if (!ubusd_send_hello(cl))
		goto delete;

	return cl;

delete:
	ubus_free_id(&clients, &cl->id);
free:
	free(cl);
	return NULL;
}

void ubusd_proto_free_client(struct ubus_client *cl)
{
	struct ubus_object *obj, *tmp;

	list_for_each_entry_safe(obj, tmp, &cl->objects, list) {
		ubusd_free_object(obj);
	}

	ubus_msg_free(cl->retmsg);
	blob_buf_free(&cl->b);

	ubusd_acl_free_client(cl);
	ubus_free_id(&clients, &cl->id);
}

void ubus_notify_subscription(struct ubus_object *obj)
{
	bool active = !list_empty(&obj->subscribers);
	struct ubus_msg_buf *ub;

	blob_buf_init(&b, 0);
	blob_put_int32(&b, UBUS_ATTR_OBJID, obj->id.id);
	blob_put_int8(&b, UBUS_ATTR_ACTIVE, active);

	ub = ubus_msg_from_blob(false);
	if (!ub)
		return;

	ubus_msg_init(ub, UBUS_MSG_NOTIFY, ++obj->invoke_seq, 0);
	ubus_msg_send(obj->client, ub);
	ubus_msg_free(ub);
}

void ubus_notify_unsubscribe(struct ubus_subscription *s)
{
	struct ubus_msg_buf *ub;

	blob_buf_init(&b, 0);
	blob_put_int32(&b, UBUS_ATTR_OBJID, s->subscriber->id.id);
	blob_put_int32(&b, UBUS_ATTR_TARGET, s->target->id.id);

	ub = ubus_msg_from_blob(false);
	if (ub != NULL) {
		ubus_msg_init(ub, UBUS_MSG_UNSUBSCRIBE, ++s->subscriber->invoke_seq, 0);
		ubus_msg_send(s->subscriber->client, ub);
		ubus_msg_free(ub);
	}

	ubus_unsubscribe(s);
}

static void __constructor ubusd_proto_init(void)
{
	ubus_init_id_tree(&clients);
}
