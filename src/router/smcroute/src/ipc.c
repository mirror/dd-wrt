/* Daemon IPC API
 *
 * Copyright (C) 2001-2005  Carsten Schill <carsten@cschill.de>
 * Copyright (C) 2006-2009  Julien BLACHE <jb@jblache.org>
 * Copyright (C) 2009       Todd Hayton <todd.hayton@gmail.com>
 * Copyright (C) 2009-2011  Micha Lenk <micha@debian.org>
 * Copyright (C) 2011-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ipc.h"
#include "log.h"
#include "msg.h"
#include "util.h"
#include "socket.h"
#include "mroute.h"

extern char *ident;

static struct sockaddr_un sun;
static int ipc_socket = -1;

/*
 * max word count in one command:
 * smcroutectl add in1 source group out1 out2 .. out32
 */
#define CMD_MAX_WORDS (MAXVIFS + 3)


/* Receive command from the smcroutectl */
static void ipc_read(int sd)
{
	/* since command len must be limited by the max number of oifs
	preallocate ipc_msg only once in advance */  
	char msg_buf[sizeof(struct ipc_msg) + CMD_MAX_WORDS * sizeof(char *)];
	char buf[MX_CMDPKT_SZ];
	int first_call = 1;
	ssize_t pos = 0;

	memset(buf, 0, sizeof(buf));

	/* 
	 * since client message would be big enough and couldn't fit
	 * into buffer we have to make multiple iterations to receive
	 * all data
	 */
	while (1) {
		const char* ptr;
		ssize_t rc;

		rc = ipc_receive(sd, buf + pos, sizeof(buf) - pos - 1, first_call);
		first_call = 0;
		if (rc <= 0) {
			if (errno == EAGAIN)     /* no more data from client */
				return;
			if (errno != ECONNRESET) /* Skip logging client disconnects */
				smclog(LOG_WARNING, "Failed receiving IPC message from client: %s", strerror(errno));
			return;
		}

		pos += rc;
		if (pos > (int)sizeof(buf) - 1) {
			smclog(LOG_WARNING, "Too large IPC message, unsupported.");
			return;
		}

		/* Make sure to always have at least one NUL, for strlen() */
		buf[pos] = 0;

		ptr = buf;
		while (pos > 0) {
			struct ipc_msg* msg = (struct ipc_msg*)msg_buf;

			/* extract one command at a time */
			if (ipc_parse(ptr, pos, msg)) {
				if (EAGAIN == errno) {
					/* 
					 * need more data from client?  move last unused bytes (if any) to
					 * the begging of the buffer and lets try to receive more data
					 */
					memmove(buf, ptr, pos);
					break;
				}
				smclog(LOG_WARNING, "Failed to parse IPC message from client: %s", strerror(errno));
				return;
			}

			if (msg_do(sd, msg)) {
				if (EINVAL == errno)
					smclog(LOG_WARNING, "Unknown or malformed IPC message '%c' from client.", msg->cmd);
				errno = 0;
				ipc_send(sd, log_message, strlen(log_message) + 1);
			} else {
				ipc_send(sd, "", 1);
			}

			/* shift to the next command if any and reduce remaining bytes in buffer */
			ptr += msg->len;
			pos -= msg->len;
		}
	}
}

static void ipc_accept(int sd, void *arg)
{
	socklen_t socklen = 0;
	int client;

	(void)arg;
	client = accept(sd, NULL, &socklen);
	if (client < 0)
		return;

	ipc_read(client);
	close(client);
}

/**
 * ipc_init - Initialise an IPC server socket
 * @path: Path to UNIX domain socket
 *
 * Returns:
 * The socket descriptor, or -1 on error with @errno set.
 */
int ipc_init(char *path)
{
	socklen_t len;
	int sd;

	if (strlen(RUNSTATEDIR) + strlen(ident) + 11 >= sizeof(sun.sun_path)) {
		smclog(LOG_ERR, "Too long socket path, max %zd chars", sizeof(sun.sun_path));
		return -1;
	}

	sd = socket_create(AF_UNIX, SOCK_STREAM, 0, ipc_accept, NULL);
	if (sd < 0) {
		smclog(LOG_WARNING, "Failed creating IPC socket, client disabled: %s", strerror(errno));
		return -1;
	}

#ifdef HAVE_SOCKADDR_UN_SUN_LEN
	sun.sun_len = 0;	/* <- correct length is set by the OS */
#endif
	sun.sun_family = AF_UNIX;
	strlcpy(sun.sun_path, path, sizeof(sun.sun_path));

	unlink(sun.sun_path);
	smclog(LOG_DEBUG, "Binding IPC socket to %s", sun.sun_path);

	len = offsetof(struct sockaddr_un, sun_path) + strlen(sun.sun_path);
	if (bind(sd, (struct sockaddr *)&sun, len) < 0 || listen(sd, 1)) {
		smclog(LOG_WARNING, "Failed binding IPC socket, client disabled: %s", strerror(errno));
		socket_close(sd);
		return -1;
	}
	ipc_socket = sd;

	return sd;
}

/**
 * ipc_exit - Tear down and cleanup IPC communication.
 */
void ipc_exit(void)
{
	if (ipc_socket >= 0)
		socket_close(ipc_socket);
	unlink(sun.sun_path);
}

/**
 * ipc_send - Send message to peer
 * @sd:  Client socket from ipc_accept()
 * @buf: Message to send
 * @len: Message length in bytes of @buf
 *
 * Sends the IPC message in @buf of the size @len to the peer.
 *
 * Returns:
 * Number of bytes successfully sent, or -1 with @errno on failure.
 */
int ipc_send(int sd, const char *buf, size_t len)
{
	if (write(sd, buf, len) != (ssize_t)len)
		return -1;

	return len;
}

/**
 * ipc_server_read - Read IPC message from client
 * @sd:  Client socket from ipc_accept()
 * @buf: Buffer for message
 * @len: Size of @buf in bytes
 * @first_call: non-zero set on first read after accept, 0 - subsequent calls
 *
 * Reads a message(s) from the IPC socket and stores in @buf, respecting
 * the size @len.  Connects and resets connection as necessary.
 *
 * Returns:
 * Size of a successfuly read command packet in @buf, or 0 on error.
 */
ssize_t ipc_receive(int sd, char *buf, size_t len, int first_call)
{
	ssize_t sz;
	/* since we can call this multiple times during receive of multipart
	command lets pass `don't wait` flag to not block forever
	when client finish transmission */
	int flags = first_call ? 0 : MSG_DONTWAIT;

	sz = recv(sd, buf, len - 1, flags);
	if (!sz)
		errno = ECONNRESET;

	return sz;
}

/**
 * ipc_server_parse - Parse IPC message(s) from client
 * @buf: Buffer of message(s)
 * @sz: Size of @buf in bytes
 * @msg_buf: Preallocated ipc_msg
 *
 * Parse message(s) from the IPC socket, respecting
 * the size @sz.
 *
 * Returns:
 * POSIX OK(0) on a successfuly read command in @buf, or non-zero on error.
 */
int ipc_parse(const char *buf, size_t sz, void* msg_buf)
{
	struct ipc_msg* msg;

	/* successful read */
	if (sz >= sizeof(struct ipc_msg)) {
		memcpy(msg_buf, buf, sizeof(struct ipc_msg));
		msg = (struct ipc_msg*)msg_buf;

		/* enough bytes to extract just one message? */
		if (sz >= msg->len) {
			size_t i, count;
			const char *ptr;

			count = msg->count;
			if (count > CMD_MAX_WORDS) {
				errno = EINVAL;
				return 1;
			}

			ptr = buf + offsetof(struct ipc_msg, argv);
			for (i = 0; i < count; i++) {
				/* Verify ptr, attacker may set too large msg->count */
				if (ptr >= (buf + msg->len)) {
					errno = EBADMSG;
					return 1;
				}

				msg->argv[i] = (char*)ptr;
				ptr += strlen(ptr) + 1;
			}

			return 0;
		}
	}

	/* we've parsed all commands or not enough bytes to parse next */
	errno = EAGAIN;
	return 1;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
