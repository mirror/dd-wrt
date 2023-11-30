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

#ifndef __LIBUBUS_H
#define __LIBUBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libubox/avl.h>
#include <libubox/list.h>
#include <libubox/blobmsg.h>
#include <libubox/uloop.h>
#include <stdint.h>
#include "ubusmsg.h"
#include "ubus_common.h"

#define UBUS_MAX_NOTIFY_PEERS	16

struct ubus_context;
struct ubus_msg_src;
struct ubus_object;
struct ubus_request;
struct ubus_request_data;
struct ubus_object_data;
struct ubus_event_handler;
struct ubus_subscriber;
struct ubus_notify_request;

struct ubus_msghdr_buf {
	struct ubus_msghdr hdr;
	struct blob_attr *data;
};

typedef void (*ubus_lookup_handler_t)(struct ubus_context *ctx,
				      struct ubus_object_data *obj,
				      void *priv);
typedef int (*ubus_handler_t)(struct ubus_context *ctx, struct ubus_object *obj,
			      struct ubus_request_data *req,
			      const char *method, struct blob_attr *msg);
typedef void (*ubus_state_handler_t)(struct ubus_context *ctx, struct ubus_object *obj);
typedef void (*ubus_remove_handler_t)(struct ubus_context *ctx,
				      struct ubus_subscriber *obj, uint32_t id);
typedef void (*ubus_event_handler_t)(struct ubus_context *ctx, struct ubus_event_handler *ev,
				     const char *type, struct blob_attr *msg);
typedef void (*ubus_data_handler_t)(struct ubus_request *req,
				    int type, struct blob_attr *msg);
typedef void (*ubus_fd_handler_t)(struct ubus_request *req, int fd);
typedef void (*ubus_complete_handler_t)(struct ubus_request *req, int ret);
typedef void (*ubus_notify_complete_handler_t)(struct ubus_notify_request *req,
					       int idx, int ret);
typedef void (*ubus_notify_data_handler_t)(struct ubus_notify_request *req,
					   int type, struct blob_attr *msg);
typedef void (*ubus_connect_handler_t)(struct ubus_context *ctx);
typedef bool (*ubus_new_object_handler_t)(struct ubus_context *ctx, struct ubus_subscriber *sub, const char *path);

#define UBUS_OBJECT_TYPE(_name, _methods)		\
	{						\
		.name = _name,				\
		.id = 0,				\
		.n_methods = ARRAY_SIZE(_methods),	\
		.methods = _methods			\
	}

#define __UBUS_METHOD_NOARG(_name, _handler, _tags)	\
	.name = _name,					\
	.handler = _handler,				\
	.tags = _tags

#define __UBUS_METHOD(_name, _handler, _policy, _tags)	\
	__UBUS_METHOD_NOARG(_name, _handler, _tags),	\
	.policy = _policy,				\
	.n_policy = ARRAY_SIZE(_policy)

#define UBUS_METHOD(_name, _handler, _policy)		\
	{ __UBUS_METHOD(_name, _handler, _policy, 0) }

#define UBUS_METHOD_TAG(_name, _handler, _policy, _tags)\
	{ __UBUS_METHOD(_name, _handler, _policy, _tags) }

#define UBUS_METHOD_MASK(_name, _handler, _policy, _mask) \
	{						\
		__UBUS_METHOD(_name, _handler, _policy, 0),\
		.mask = _mask				\
	}

#define UBUS_METHOD_NOARG(_name, _handler)		\
	{ __UBUS_METHOD_NOARG(_name, _handler, 0) }

#define UBUS_METHOD_TAG_NOARG(_name, _handler, _tags)	\
	{ __UBUS_METHOD_NOARG(_name, _handler, _tags) }

#define UBUS_TAG_STATUS		BIT(0)
#define UBUS_TAG_ADMIN		BIT(1)
#define UBUS_TAG_PRIVATE	BIT(2)

struct ubus_method {
	const char *name;
	ubus_handler_t handler;

	unsigned long mask;
	unsigned long tags;
	const struct blobmsg_policy *policy;
	int n_policy;
};

struct ubus_object_type {
	const char *name;
	uint32_t id;

	const struct ubus_method *methods;
	int n_methods;
};

struct ubus_object {
	struct avl_node avl;

	const char *name;
	uint32_t id;

	const char *path;
	struct ubus_object_type *type;

	ubus_state_handler_t subscribe_cb;
	bool has_subscribers;

	const struct ubus_method *methods;
	int n_methods;
};

struct ubus_subscriber {
	struct list_head list;
	struct ubus_object obj;

	ubus_handler_t cb;
	ubus_remove_handler_t remove_cb;
	ubus_new_object_handler_t new_obj_cb;
};

struct ubus_event_handler {
	struct ubus_object obj;

	ubus_event_handler_t cb;
};

struct ubus_context {
	struct list_head requests;
	struct avl_tree objects;
	struct list_head pending;

	struct uloop_fd sock;
	struct uloop_timeout pending_timer;

	uint32_t local_id;
	uint16_t request_seq;
	bool cancel_poll;
	int stack_depth;

	void (*connection_lost)(struct ubus_context *ctx);
	void (*monitor_cb)(struct ubus_context *ctx, uint32_t seq, struct blob_attr *data);

	struct ubus_msghdr_buf msgbuf;
	uint32_t msgbuf_data_len;
	int msgbuf_reduction_counter;

	struct list_head auto_subscribers;
	struct ubus_event_handler auto_subscribe_event_handler;
};

struct ubus_object_data {
	uint32_t id;
	uint32_t type_id;
	const char *path;
	struct blob_attr *signature;
};

struct ubus_acl_key {
	const char *user;
	const char *group;
	const char *object;
};

struct ubus_request_data {
	uint32_t object;
	uint32_t peer;
	uint16_t seq;

	struct ubus_acl_key acl;

	/* internal use */
	bool deferred;
	int fd;
	int req_fd; /* fd received from the initial request */
};

struct ubus_request {
	struct list_head list;

	struct list_head pending;
	int status_code;
	bool status_msg;
	bool blocked;
	bool cancelled;
	bool notify;

	uint32_t peer;
	uint16_t seq;

	ubus_data_handler_t raw_data_cb;
	ubus_data_handler_t data_cb;
	ubus_fd_handler_t fd_cb;
	ubus_complete_handler_t complete_cb;

	int fd;

	struct ubus_context *ctx;
	void *priv;
};

struct ubus_notify_request {
	struct ubus_request req;

	ubus_notify_complete_handler_t status_cb;
	ubus_notify_complete_handler_t complete_cb;
	ubus_notify_data_handler_t data_cb;

	uint32_t pending;
	uint32_t id[UBUS_MAX_NOTIFY_PEERS + 1];
};

struct ubus_auto_conn {
	struct ubus_context ctx;
	struct uloop_timeout timer;
	const char *path;
	ubus_connect_handler_t cb;
};

struct ubus_context *ubus_connect(const char *path);
int ubus_connect_ctx(struct ubus_context *ctx, const char *path);
void ubus_auto_connect(struct ubus_auto_conn *conn);
int ubus_reconnect(struct ubus_context *ctx, const char *path);

/* call this only for struct ubus_context pointers returned by ubus_connect() */
void ubus_free(struct ubus_context *ctx);

/* call this only for struct ubus_context pointers initialised by ubus_connect_ctx() */
void ubus_shutdown(struct ubus_context *ctx);

static inline void ubus_auto_shutdown(struct ubus_auto_conn *conn)
{
	uloop_timeout_cancel(&conn->timer);
	ubus_shutdown(&conn->ctx);
}

const char *ubus_strerror(int error);

static inline void ubus_add_uloop(struct ubus_context *ctx)
{
	uloop_fd_add(&ctx->sock, ULOOP_BLOCKING | ULOOP_READ);
}

/* call this for read events on ctx->sock.fd when not using uloop */
static inline void ubus_handle_event(struct ubus_context *ctx)
{
	ctx->sock.cb(&ctx->sock, ULOOP_READ);
}

/* ----------- raw request handling ----------- */

/* wait for a request to complete and return its status */
int ubus_complete_request(struct ubus_context *ctx, struct ubus_request *req,
			  int timeout);

/* complete a request asynchronously */
void ubus_complete_request_async(struct ubus_context *ctx,
				 struct ubus_request *req);

/* abort an asynchronous request */
void ubus_abort_request(struct ubus_context *ctx, struct ubus_request *req);

/* ----------- objects ----------- */

int ubus_lookup(struct ubus_context *ctx, const char *path,
		ubus_lookup_handler_t cb, void *priv);

int ubus_lookup_id(struct ubus_context *ctx, const char *path, uint32_t *id);

/* make an object visible to remote connections */
int ubus_add_object(struct ubus_context *ctx, struct ubus_object *obj);

/* remove the object from the ubus connection */
int ubus_remove_object(struct ubus_context *ctx, struct ubus_object *obj);

/* add a subscriber notifications from another object */
int ubus_register_subscriber(struct ubus_context *ctx, struct ubus_subscriber *obj);

static inline int
ubus_unregister_subscriber(struct ubus_context *ctx, struct ubus_subscriber *obj)
{
	if (!list_empty(&obj->list))
		list_del_init(&obj->list);
	return ubus_remove_object(ctx, &obj->obj);
}

int ubus_subscribe(struct ubus_context *ctx, struct ubus_subscriber *obj, uint32_t id);
int ubus_unsubscribe(struct ubus_context *ctx, struct ubus_subscriber *obj, uint32_t id);

int __ubus_monitor(struct ubus_context *ctx, const char *type);

static inline int ubus_monitor_start(struct ubus_context *ctx)
{
	return __ubus_monitor(ctx, "add");
}

static inline int ubus_monitor_stop(struct ubus_context *ctx)
{
	return __ubus_monitor(ctx, "remove");
}


/* ----------- acl ----------- */

struct acl_object {
	struct ubus_acl_key key;
	struct avl_node avl;
	struct blob_attr *acl;
};

extern struct avl_tree acl_objects;
int ubus_register_acl(struct ubus_context *ctx);

#define acl_for_each(o, m) \
	if ((m)->object && (m)->user && (m)->group) \
		avl_for_element_range(avl_find_ge_element(&acl_objects, m, o, avl), avl_find_le_element(&acl_objects, m, o, avl), o, avl)

/* ----------- rpc ----------- */

/* invoke a method on a specific object */
int ubus_invoke_fd(struct ubus_context *ctx, uint32_t obj, const char *method,
		struct blob_attr *msg, ubus_data_handler_t cb, void *priv,
		int timeout, int fd);
static inline int
ubus_invoke(struct ubus_context *ctx, uint32_t obj, const char *method,
	    struct blob_attr *msg, ubus_data_handler_t cb, void *priv,
	    int timeout)
{
	return ubus_invoke_fd(ctx, obj, method, msg, cb, priv, timeout, -1);
}

/* asynchronous version of ubus_invoke() */
int ubus_invoke_async_fd(struct ubus_context *ctx, uint32_t obj, const char *method,
		      struct blob_attr *msg, struct ubus_request *req, int fd);
static inline int
ubus_invoke_async(struct ubus_context *ctx, uint32_t obj, const char *method,
		  struct blob_attr *msg, struct ubus_request *req)
{
	return ubus_invoke_async_fd(ctx, obj, method, msg, req, -1);
}

/* send a reply to an incoming object method call */
int ubus_send_reply(struct ubus_context *ctx, struct ubus_request_data *req,
		    struct blob_attr *msg);

static inline void ubus_defer_request(struct ubus_context *ctx,
				      struct ubus_request_data *req,
				      struct ubus_request_data *new_req)
{
    (void) ctx;
    memcpy(new_req, req, sizeof(*req));
    req->deferred = true;
}

static inline void ubus_request_set_fd(struct ubus_context *ctx,
				       struct ubus_request_data *req, int fd)
{
    (void) ctx;
    req->fd = fd;
}

static inline int ubus_request_get_caller_fd(struct ubus_request_data *req)
{
    int fd = req->req_fd;
    req->req_fd = -1;
    
    return fd;
}

void ubus_complete_deferred_request(struct ubus_context *ctx,
				    struct ubus_request_data *req, int ret);

/*
 * send a notification to all subscribers of an object
 * if timeout < 0, no reply is expected from subscribers
 */
int ubus_notify(struct ubus_context *ctx, struct ubus_object *obj,
		const char *type, struct blob_attr *msg, int timeout);

int ubus_notify_async(struct ubus_context *ctx, struct ubus_object *obj,
		      const char *type, struct blob_attr *msg,
		      struct ubus_notify_request *req);


/* ----------- events ----------- */

int ubus_send_event(struct ubus_context *ctx, const char *id,
		    struct blob_attr *data);

int ubus_register_event_handler(struct ubus_context *ctx,
				struct ubus_event_handler *ev,
				const char *pattern);

static inline int ubus_unregister_event_handler(struct ubus_context *ctx,
						struct ubus_event_handler *ev)
{
    return ubus_remove_object(ctx, &ev->obj);
}

#ifdef __cplusplus
}
#endif

#endif
