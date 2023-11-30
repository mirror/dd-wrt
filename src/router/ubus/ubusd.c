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

#include <sys/socket.h>
#ifdef FreeBSD
#include <sys/param.h>
#endif

#include "ubusd.h"

#define USES_EXTERNAL_BUFFER ~0U

static struct ubus_msg_buf *ubus_msg_ref(struct ubus_msg_buf *ub)
{
	struct ubus_msg_buf *new_ub;
	if (ub->refcount == USES_EXTERNAL_BUFFER) {
		new_ub = ubus_msg_new(ub->data, ub->len, false);
		if (!new_ub)
			return NULL;
		memcpy(&new_ub->hdr, &ub->hdr, sizeof(struct ubus_msghdr));
		new_ub->fd = ub->fd;
		return new_ub;
	}

	ub->refcount++;
	return ub;
}

struct ubus_msg_buf *ubus_msg_new(void *data, int len, bool shared)
{
	struct ubus_msg_buf *ub;
	int buflen = sizeof(*ub);

	if (!shared)
		buflen += len;

	ub = calloc(1, buflen);
	if (!ub)
		return NULL;

	ub->fd = -1;

	if (shared) {
		ub->refcount = USES_EXTERNAL_BUFFER;
		ub->data = data;
	} else {
		ub->refcount = 1;
		ub->data = (void *) (ub + 1);
		if (data)
			memcpy(ub + 1, data, len);
	}

	ub->len = len;
	return ub;
}

void ubus_msg_free(struct ubus_msg_buf *ub)
{
	switch (ub->refcount) {
	case 1:
	case USES_EXTERNAL_BUFFER:
		if (ub->fd >= 0)
			close(ub->fd);

		free(ub);
		break;
	default:
		ub->refcount--;
		break;
	}
}

ssize_t ubus_msg_writev(int fd, struct ubus_msg_buf *ub, size_t offset)
{
	uint8_t fd_buf[CMSG_SPACE(sizeof(int))] = { 0 };
	static struct iovec iov[2];
	struct msghdr msghdr = { 0 };
	struct ubus_msghdr hdr;
	struct cmsghdr *cmsg;
	ssize_t ret;
	int *pfd;

	msghdr.msg_iov = iov;
	msghdr.msg_iovlen = ARRAY_SIZE(iov);
	msghdr.msg_control = fd_buf;
	msghdr.msg_controllen = sizeof(fd_buf);

	cmsg = CMSG_FIRSTHDR(&msghdr);
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));

	pfd = (int *) CMSG_DATA(cmsg);
	msghdr.msg_controllen = cmsg->cmsg_len;

	*pfd = ub->fd;
	if (ub->fd < 0 || offset) {
		msghdr.msg_control = NULL;
		msghdr.msg_controllen = 0;
	}

	if (offset < sizeof(ub->hdr)) {
		hdr.version = ub->hdr.version;
		hdr.type = ub->hdr.type;
		hdr.seq = cpu_to_be16(ub->hdr.seq);
		hdr.peer = cpu_to_be32(ub->hdr.peer);

		iov[0].iov_base = ((char *) &hdr) + offset;
		iov[0].iov_len = sizeof(hdr) - offset;
		iov[1].iov_base = (char *) ub->data;
		iov[1].iov_len = ub->len;
	} else {
		offset -= sizeof(ub->hdr);
		iov[0].iov_base = ((char *) ub->data) + offset;
		iov[0].iov_len = ub->len - offset;
		msghdr.msg_iovlen = 1;
	}

	do {
		ret = sendmsg(fd, &msghdr, 0);
	} while (ret < 0 && errno == EINTR);

	return ret;
}

void ubus_msg_list_free(struct ubus_msg_buf_list *ubl)
{
	list_del_init(&ubl->list);
	ubus_msg_free(ubl->msg);
	free(ubl);
}

static void ubus_msg_enqueue(struct ubus_client *cl, struct ubus_msg_buf *ub)
{
	struct ubus_msg_buf_list *ubl;

	if (cl->txq_len + ub->len > UBUS_CLIENT_MAX_TXQ_LEN)
		return;

	ubl = calloc(1, sizeof(struct ubus_msg_buf_list));
	if (!ubl)
		return;

	INIT_LIST_HEAD(&ubl->list);
	ubl->msg = ubus_msg_ref(ub);

	list_add_tail(&ubl->list, &cl->tx_queue);
	cl->txq_len += ub->len;
}

/* takes the msgbuf reference */
void ubus_msg_send(struct ubus_client *cl, struct ubus_msg_buf *ub)
{
	ssize_t written;

	if (ub->hdr.type != UBUS_MSG_MONITOR)
		ubusd_monitor_message(cl, ub, true);

	if (list_empty(&cl->tx_queue)) {
		written = ubus_msg_writev(cl->sock.fd, ub, 0);

		if (written < 0)
			written = 0;

		if (written >= (ssize_t) (ub->len + sizeof(ub->hdr)))
			return;

		cl->txq_ofs = written;
		cl->txq_len = -written;

		/* get an event once we can write to the socket again */
		uloop_fd_add(&cl->sock, ULOOP_READ | ULOOP_WRITE | ULOOP_EDGE_TRIGGER);
	}
	ubus_msg_enqueue(cl, ub);
}
