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

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <libubox/usock.h>
#include <libubox/blob.h>
#include <libubox/blobmsg.h>

#include "libubus.h"
#include "libubus-internal.h"

#define STATIC_IOV(_var) { .iov_base = (char *) &(_var), .iov_len = sizeof(_var) }

#define UBUS_MSGBUF_REDUCTION_INTERVAL	16

static const struct blob_attr_info ubus_policy[UBUS_ATTR_MAX] = {
	[UBUS_ATTR_STATUS] = { .type = BLOB_ATTR_INT32 },
	[UBUS_ATTR_OBJID] = { .type = BLOB_ATTR_INT32 },
	[UBUS_ATTR_OBJPATH] = { .type = BLOB_ATTR_STRING },
	[UBUS_ATTR_METHOD] = { .type = BLOB_ATTR_STRING },
	[UBUS_ATTR_ACTIVE] = { .type = BLOB_ATTR_INT8 },
	[UBUS_ATTR_NO_REPLY] = { .type = BLOB_ATTR_INT8 },
	[UBUS_ATTR_SUBSCRIBERS] = { .type = BLOB_ATTR_NESTED },
};

static struct blob_attr *attrbuf[UBUS_ATTR_MAX];

__hidden struct blob_attr **ubus_parse_msg(struct blob_attr *msg, size_t len)
{
	blob_parse_untrusted(msg, len, attrbuf, ubus_policy, UBUS_ATTR_MAX);
	return attrbuf;
}

static void wait_data(int fd, bool write)
{
	struct pollfd pfd = { .fd = fd };

	pfd.events = write ? POLLOUT : POLLIN;
	poll(&pfd, 1, -1);
}

static int writev_retry(int fd, struct iovec *iov, int iov_len, int sock_fd)
{
	uint8_t fd_buf[CMSG_SPACE(sizeof(int))] = { 0 };
	struct msghdr msghdr = { 0 };
	struct cmsghdr *cmsg;
	int len = 0;
	int *pfd;

	msghdr.msg_iov = iov,
	msghdr.msg_iovlen = iov_len,
	msghdr.msg_control = fd_buf;
	msghdr.msg_controllen = sizeof(fd_buf);

	cmsg = CMSG_FIRSTHDR(&msghdr);
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));

	pfd = (int *) CMSG_DATA(cmsg);
	msghdr.msg_controllen = cmsg->cmsg_len;

	do {
		ssize_t cur_len;

		if (sock_fd < 0) {
			msghdr.msg_control = NULL;
			msghdr.msg_controllen = 0;
		} else {
			*pfd = sock_fd;
		}

		cur_len = sendmsg(fd, &msghdr, 0);
		if (cur_len < 0) {
			switch(errno) {
			case EAGAIN:
				wait_data(fd, true);
				break;
			case EINTR:
				break;
			default:
				return -1;
			}
			continue;
		}

		if (len > 0)
			sock_fd = -1;

		len += cur_len;
		while (cur_len >= (ssize_t) iov->iov_len) {
			cur_len -= iov->iov_len;
			iov_len--;
			iov++;
			if (!iov_len)
				return len;
		}
		iov->iov_base += cur_len;
		iov->iov_len -= cur_len;
		msghdr.msg_iov = iov;
		msghdr.msg_iovlen = iov_len;
	} while (1);

	/* Should never reach here */
	return -1;
}

int __hidden ubus_send_msg(struct ubus_context *ctx, uint32_t seq,
			   struct blob_attr *msg, int cmd, uint32_t peer, int fd)
{
	struct ubus_msghdr hdr;
	struct iovec iov[2] = {
		STATIC_IOV(hdr)
	};
	int ret;

	hdr.version = 0;
	hdr.type = cmd;
	hdr.seq = cpu_to_be16(seq);
	hdr.peer = cpu_to_be32(peer);

	if (!msg) {
		blob_buf_init(&b, 0);
		msg = b.head;
	}

	iov[1].iov_base = (char *) msg;
	iov[1].iov_len = blob_raw_len(msg);

	ret = writev_retry(ctx->sock.fd, iov, ARRAY_SIZE(iov), fd);
	if (ret < 0)
		ctx->sock.eof = true;

	if (fd >= 0)
		close(fd);

	return ret;
}

static int recv_retry(struct ubus_context *ctx, struct iovec *iov, bool wait, int *recv_fd)
{
	uint8_t fd_buf[CMSG_SPACE(sizeof(int))] = { 0 };
	struct msghdr msghdr = { 0 };
	struct cmsghdr *cmsg;
	int total = 0;
	int bytes;
	int *pfd;
	int fd;

	fd = ctx->sock.fd;

	msghdr.msg_iov = iov,
	msghdr.msg_iovlen = 1,
	msghdr.msg_control = fd_buf;
	msghdr.msg_controllen = sizeof(fd_buf);

	cmsg = CMSG_FIRSTHDR(&msghdr);
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));

	pfd = (int *) CMSG_DATA(cmsg);

	while (iov->iov_len > 0) {
		if (recv_fd) {
			msghdr.msg_control = fd_buf;
			msghdr.msg_controllen = cmsg->cmsg_len;
		} else {
			msghdr.msg_control = NULL;
			msghdr.msg_controllen = 0;
		}

		*pfd = -1;
		bytes = recvmsg(fd, &msghdr, 0);
		if (!bytes)
			return -1;

		if (bytes < 0) {
			bytes = 0;
			if (errno == EINTR)
				continue;

			if (errno != EAGAIN)
				return -1;
		}
		if (!wait && !bytes)
			return 0;

		if (recv_fd)
			*recv_fd = *pfd;

		recv_fd = NULL;

		wait = true;
		iov->iov_len -= bytes;
		iov->iov_base += bytes;
		total += bytes;

		if (iov->iov_len > 0)
			wait_data(fd, false);
	}

	return total;
}

bool ubus_validate_hdr(struct ubus_msghdr *hdr)
{
	struct blob_attr *data = (struct blob_attr *) (hdr + 1);

	if (hdr->version != 0)
		return false;

	if (blob_raw_len(data) < sizeof(*data))
		return false;

	if (blob_pad_len(data) > UBUS_MAX_MSGLEN)
		return false;

	return true;
}

static bool alloc_msg_buf(struct ubus_context *ctx, int len)
{
	void *ptr;
	int buf_len = ctx->msgbuf_data_len;
	int rem;

	if (!ctx->msgbuf.data)
		buf_len = 0;

	rem = (len % UBUS_MSG_CHUNK_SIZE);
	if (rem > 0)
		len += UBUS_MSG_CHUNK_SIZE - rem;

	if (len < buf_len &&
	    ++ctx->msgbuf_reduction_counter > UBUS_MSGBUF_REDUCTION_INTERVAL) {
		ctx->msgbuf_reduction_counter = 0;
		buf_len = 0;
	}

	if (len <= buf_len)
		return true;

	ptr = realloc(ctx->msgbuf.data, len);
	if (!ptr)
		return false;

	ctx->msgbuf.data = ptr;
	ctx->msgbuf_data_len = len;
	return true;
}

static bool get_next_msg(struct ubus_context *ctx, int *recv_fd)
{
	struct {
		struct ubus_msghdr hdr;
		struct blob_attr data;
	} hdrbuf;
	struct iovec iov = STATIC_IOV(hdrbuf);
	int len;
	int r;

	/* receive header + start attribute */
	r = recv_retry(ctx, &iov, false, recv_fd);
	if (r <= 0) {
		if (r < 0)
			ctx->sock.eof = true;

		return false;
	}

	hdrbuf.hdr.seq = be16_to_cpu(hdrbuf.hdr.seq);
	hdrbuf.hdr.peer = be32_to_cpu(hdrbuf.hdr.peer);

	if (!ubus_validate_hdr(&hdrbuf.hdr))
		return false;

	len = blob_raw_len(&hdrbuf.data);
	if (!alloc_msg_buf(ctx, len))
		return false;

	memcpy(&ctx->msgbuf.hdr, &hdrbuf.hdr, sizeof(hdrbuf.hdr));
	memcpy(ctx->msgbuf.data, &hdrbuf.data, sizeof(hdrbuf.data));

	iov.iov_base = (char *)ctx->msgbuf.data + sizeof(hdrbuf.data);
	iov.iov_len = blob_len(ctx->msgbuf.data);
	if (iov.iov_len > 0 &&
	    recv_retry(ctx, &iov, true, NULL) <= 0)
		return false;

	return true;
}

void __hidden ubus_handle_data(struct uloop_fd *u, unsigned int events)
{
	struct ubus_context *ctx = container_of(u, struct ubus_context, sock);
	int recv_fd = -1;

	while (1) {
		if (!ctx->stack_depth)
			ctx->pending_timer.cb(&ctx->pending_timer);

		if (!get_next_msg(ctx, &recv_fd))
			break;
		ubus_process_msg(ctx, &ctx->msgbuf, recv_fd);
		if (uloop_cancelling() || ctx->cancel_poll)
			break;
	}

	if (!ctx->stack_depth)
		ctx->pending_timer.cb(&ctx->pending_timer);

	if (u->eof)
		ctx->connection_lost(ctx);
}

void __hidden ubus_poll_data(struct ubus_context *ctx, int timeout)
{
	struct pollfd pfd = {
		.fd = ctx->sock.fd,
		.events = POLLIN | POLLERR,
	};

	ctx->cancel_poll = false;
	poll(&pfd, 1, timeout ? timeout : -1);
	ubus_handle_data(&ctx->sock, ULOOP_READ);
}

static void
ubus_auto_sub_lookup(struct ubus_context *ctx, struct ubus_object_data *obj,
		     void *priv)
{
	struct ubus_subscriber *s;

	list_for_each_entry(s, &ctx->auto_subscribers, list)
		if (s->new_obj_cb(ctx, s, obj->path))
			ubus_subscribe(ctx, s, obj->id);
}

static void
ubus_refresh_auto_subscribe(struct ubus_context *ctx)
{
	struct ubus_event_handler *ev = &ctx->auto_subscribe_event_handler;

	if (list_empty(&ctx->auto_subscribers))
		return;

	ubus_register_event_handler(ctx, ev, "ubus.object.add");
	ubus_lookup(ctx, NULL, ubus_auto_sub_lookup, NULL);
}

static void
ubus_refresh_state(struct ubus_context *ctx)
{
	struct ubus_object *obj, *tmp;
	struct ubus_object **objs;
	int n, i = 0;

	/* clear all type IDs, they need to be registered again */
	avl_for_each_element(&ctx->objects, obj, avl)
		if (obj->type)
			obj->type->id = 0;

	/* push out all objects again */
	objs = alloca(ctx->objects.count * sizeof(*objs));
	avl_remove_all_elements(&ctx->objects, obj, avl, tmp) {
		objs[i++] = obj;
		obj->id = 0;
	}

	for (n = i, i = 0; i < n; i++)
		ubus_add_object(ctx, objs[i]);

	ubus_refresh_auto_subscribe(ctx);
}

int ubus_reconnect(struct ubus_context *ctx, const char *path)
{
	struct {
		struct ubus_msghdr hdr;
		struct blob_attr data;
	} hdr;
	struct blob_attr *buf;
	int ret = UBUS_STATUS_UNKNOWN_ERROR;

	if (!path)
		path = UBUS_UNIX_SOCKET;

	if (ctx->sock.fd >= 0) {
		if (ctx->sock.registered)
			uloop_fd_delete(&ctx->sock);

		close(ctx->sock.fd);
	}

	ctx->sock.eof = false;
	ctx->sock.error = false;
	ctx->sock.fd = usock(USOCK_UNIX, path, NULL);
	if (ctx->sock.fd < 0)
		return UBUS_STATUS_CONNECTION_FAILED;

	if (read(ctx->sock.fd, &hdr, sizeof(hdr)) != sizeof(hdr))
		goto out_close;

	if (!ubus_validate_hdr(&hdr.hdr))
		goto out_close;

	if (hdr.hdr.type != UBUS_MSG_HELLO)
		goto out_close;

	buf = calloc(1, blob_raw_len(&hdr.data));
	if (!buf)
		goto out_close;

	memcpy(buf, &hdr.data, sizeof(hdr.data));
	if (read(ctx->sock.fd, blob_data(buf), blob_len(buf)) != (ssize_t) blob_len(buf))
		goto out_free;

	ctx->local_id = hdr.hdr.peer;
	if (!ctx->local_id)
		goto out_free;

	ret = UBUS_STATUS_OK;
	fcntl(ctx->sock.fd, F_SETFL, fcntl(ctx->sock.fd, F_GETFL) | O_NONBLOCK | O_CLOEXEC);

	ubus_refresh_state(ctx);

out_free:
	free(buf);
out_close:
	if (ret)
		close(ctx->sock.fd);

	return ret;
}
