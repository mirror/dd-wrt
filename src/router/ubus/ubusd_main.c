/*
 * Copyright (C) 2011-2014 Felix Fietkau <nbd@openwrt.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef FreeBSD
#include <sys/param.h>
#endif
#include <string.h>
#include <syslog.h>

#include <libubox/usock.h>

#include "ubusd.h"

static void handle_client_disconnect(struct ubus_client *cl)
{
	struct ubus_msg_buf_list *ubl, *ubl2;
	list_for_each_entry_safe(ubl, ubl2, &cl->tx_queue, list)
		ubus_msg_list_free(ubl);

	ubusd_monitor_disconnect(cl);
	ubusd_proto_free_client(cl);
	if (cl->pending_msg_fd >= 0)
		close(cl->pending_msg_fd);
	uloop_fd_delete(&cl->sock);
	close(cl->sock.fd);
	free(cl);
}

static void ubus_client_cmd_free(struct ubus_client_cmd *cmd)
{
	list_del(&cmd->list);
	ubus_msg_free(cmd->msg);
	free(cmd);
}

static void ubus_client_cmd_queue_process(struct ubus_client *cl)
{
	struct ubus_client_cmd *cmd, *tmp;

	list_for_each_entry_safe(cmd, tmp, &cl->cmd_queue, list) {
		int ret = ubusd_cmd_lookup(cl, cmd);

		/* Stop if the last command caused buffering again */
		if (ret == -2)
			break;

		ubus_client_cmd_free(cmd);
	}
}

static void client_cb(struct uloop_fd *sock, unsigned int events)
{
	struct ubus_client *cl = container_of(sock, struct ubus_client, sock);
	uint8_t fd_buf[CMSG_SPACE(sizeof(int))] = { 0 };
	struct msghdr msghdr = { 0 };
	struct ubus_msg_buf *ub;
	struct ubus_msg_buf_list *ubl, *ubl2;
	static struct iovec iov;
	struct cmsghdr *cmsg;
	int *pfd;

	msghdr.msg_iov = &iov,
	msghdr.msg_iovlen = 1,
	msghdr.msg_control = fd_buf;
	msghdr.msg_controllen = sizeof(fd_buf);

	cmsg = CMSG_FIRSTHDR(&msghdr);
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));

	pfd = (int *) CMSG_DATA(cmsg);
	msghdr.msg_controllen = cmsg->cmsg_len;

	/* first try to tx more pending data */
	list_for_each_entry_safe(ubl, ubl2, &cl->tx_queue, list) {
		ssize_t written;

		ub = ubl->msg;
		written = ubus_msg_writev(sock->fd, ub, cl->txq_ofs);
		if (written < 0) {
			switch(errno) {
			case EINTR:
			case EAGAIN:
				break;
			default:
				goto disconnect;
			}
			break;
		}

		cl->txq_ofs += written;
		cl->txq_len -= written;
		if (cl->txq_ofs < ub->len + sizeof(ub->hdr))
			break;

		cl->txq_ofs = 0;
		ubus_msg_list_free(ubl);
	}

	if (list_empty(&cl->tx_queue) && (events & ULOOP_WRITE)) {
		/* Process queued commands */
		ubus_client_cmd_queue_process(cl);

		/* prevent further ULOOP_WRITE events if we don't have data
		 * to send anymore */
		if (list_empty(&cl->tx_queue))
			uloop_fd_add(sock, ULOOP_READ | ULOOP_EDGE_TRIGGER);
	}

retry:
	if (!sock->eof && cl->pending_msg_offset < (int) sizeof(cl->hdrbuf)) {
		int offset = cl->pending_msg_offset;
		int bytes;

		*pfd = -1;

		iov.iov_base = ((char *) &cl->hdrbuf) + offset;
		iov.iov_len = sizeof(cl->hdrbuf) - offset;

		if (cl->pending_msg_fd < 0) {
			msghdr.msg_control = fd_buf;
			msghdr.msg_controllen = cmsg->cmsg_len;
		} else {
			msghdr.msg_control = NULL;
			msghdr.msg_controllen = 0;
		}

		bytes = recvmsg(sock->fd, &msghdr, 0);
		if (bytes < 0)
			goto out;

		if (*pfd >= 0)
			cl->pending_msg_fd = *pfd;

		cl->pending_msg_offset += bytes;
		if (cl->pending_msg_offset < (int) sizeof(cl->hdrbuf))
			goto out;

		if (blob_raw_len(&cl->hdrbuf.data) < sizeof(struct blob_attr))
			goto disconnect;
		if (blob_pad_len(&cl->hdrbuf.data) > UBUS_MAX_MSGLEN)
			goto disconnect;

		cl->pending_msg = ubus_msg_new(NULL, blob_raw_len(&cl->hdrbuf.data), false);
		if (!cl->pending_msg)
			goto disconnect;

		cl->hdrbuf.hdr.seq = be16_to_cpu(cl->hdrbuf.hdr.seq);
		cl->hdrbuf.hdr.peer = be32_to_cpu(cl->hdrbuf.hdr.peer);

		memcpy(&cl->pending_msg->hdr, &cl->hdrbuf.hdr, sizeof(cl->hdrbuf.hdr));
		memcpy(cl->pending_msg->data, &cl->hdrbuf.data, sizeof(cl->hdrbuf.data));
	}

	ub = cl->pending_msg;
	if (ub) {
		int offset = cl->pending_msg_offset - sizeof(ub->hdr);
		int len = blob_raw_len(ub->data) - offset;
		int bytes = 0;

		if (len > 0) {
			bytes = read(sock->fd, (char *) ub->data + offset, len);
			if (bytes <= 0)
				goto out;
		}

		if (bytes < len) {
			cl->pending_msg_offset += bytes;
			goto out;
		}

		/* accept message */
		ub->fd = cl->pending_msg_fd;
		cl->pending_msg_fd = -1;
		cl->pending_msg_offset = 0;
		cl->pending_msg = NULL;
		ubusd_monitor_message(cl, ub, false);
		ubusd_proto_receive_message(cl, ub);
		goto retry;
	}

out:
	if (!sock->eof || !list_empty(&cl->tx_queue))
		return;

disconnect:
	handle_client_disconnect(cl);
}

static bool get_next_connection(int fd)
{
	struct ubus_client *cl;
	int client_fd;

	client_fd = accept(fd, NULL, 0);
	if (client_fd < 0) {
		switch (errno) {
		case ECONNABORTED:
		case EINTR:
			return true;
		default:
			return false;
		}
	}

	cl = ubusd_proto_new_client(client_fd, client_cb);
	if (cl)
		uloop_fd_add(&cl->sock, ULOOP_READ | ULOOP_EDGE_TRIGGER);
	else
		close(client_fd);

	return true;
}

static void server_cb(struct uloop_fd *fd, unsigned int events)
{
	bool next;

	do {
		next = get_next_connection(fd->fd);
	} while (next);
}

static struct uloop_fd server_fd = {
	.cb = server_cb,
};

static int usage(const char *progname)
{
	fprintf(stderr, "Usage: %s [<options>]\n"
		"Options: \n"
		"  -A <path>:		Set the path to ACL files\n"
		"  -s <socket>:		Set the unix domain socket to listen on\n"
		"\n", progname);
	return 1;
}

static void sighup_handler(int sig)
{
	ubusd_acl_load();
}

static void mkdir_sockdir()
{
	char *ubus_sock_dir, *tmp;

	ubus_sock_dir = strdup(UBUS_UNIX_SOCKET);
	tmp = strrchr(ubus_sock_dir, '/');
	if (tmp) {
		*tmp = '\0';
		mkdir(ubus_sock_dir, 0755);
	}
	free(ubus_sock_dir);
}

#include <libubox/ulog.h>

int main(int argc, char **argv)
{
	const char *ubus_socket = UBUS_UNIX_SOCKET;
	int ret = 0;
	int ch;


	while ((ch = getopt(argc, argv, "A:s:")) != -1) {
		switch (ch) {
		case 's':
			ubus_socket = optarg;
			break;
		case 'A':
			ubusd_acl_dir = optarg;
			break;
		default:
			return usage(argv[0]);
		}
	}

	uloop_handle_sigchld = false;
	switch (fork()) {
	case -1:
		// can't fork
		exit(0);
		break;
	case 0:
		/* 
		 * child process 
		 */
		// fork ok
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, sighup_handler);

	ulog_open(ULOG_KMSG | ULOG_SYSLOG, LOG_DAEMON, "ubusd");
	openlog("ubusd", LOG_PID, LOG_DAEMON);
	uloop_init();
	mkdir_sockdir();
	unlink(ubus_socket);
	umask(0111);
	server_fd.fd = usock(USOCK_UNIX | USOCK_SERVER | USOCK_NONBLOCK, ubus_socket, NULL);
	if (server_fd.fd < 0) {
		perror("usock");
		ret = -1;
		goto out;
	}
	uloop_fd_add(&server_fd, ULOOP_READ | ULOOP_EDGE_TRIGGER);
	ubusd_acl_load();

	uloop_run();
	unlink(ubus_socket);

out:
	uloop_done();
	return ret;
}
