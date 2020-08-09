// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)

#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <errno.h>
#include <ev.h>

#include "server_cmds.h"
#include "utils.h"

static EV_P;
static ev_io client_watcher;
static int fd;

static int server_socket(void)
{
	struct sockaddr_un sa;
	int s;

	if(0 > (s = socket(PF_UNIX, SOCK_DGRAM, 0)))
	{
		fprintf(stderr, "Couldn't open unix socket: %m");
		return -1;
	}

	set_socket_address(&sa, MRP_SERVER_SOCK_NAME);
	
	if(0 != bind(s, (struct sockaddr *)&sa, sizeof(sa)))
	{
		fprintf(stderr, "Couldn't bind socket: %m");
		close(s);
		return -1;
	}

	return s;
}

static int handle_message(int cmd, void *inbuf, int lin,
                          void *outbuf, int lout)
{
	switch(cmd) {
	SERVER_MESSAGE_CASE(addmrp);
	SERVER_MESSAGE_CASE(delmrp);
	SERVER_MESSAGE_CASE(getmrp);
	default:
		return -1;
	}
}

#define MSG_BUF_LEN 10000
static unsigned char msg_inbuf[MSG_BUF_LEN];
static unsigned char msg_outbuf[MSG_BUF_LEN];

static void ctl_rcv_handler(EV_P_ ev_io *w, int revents)
{
	struct ctl_msg_hdr mhdr;
	struct msghdr msg;
	struct sockaddr_un sa;
	struct iovec iov[2];
	int l;

	msg.msg_name = &sa;
	msg.msg_namelen = sizeof(sa);
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	iov[0].iov_base = &mhdr;
	iov[0].iov_len = sizeof(mhdr);
	iov[1].iov_base = msg_inbuf;
	iov[1].iov_len = MSG_BUF_LEN;
	l = recvmsg(fd, &msg, MSG_NOSIGNAL | MSG_DONTWAIT);

	if ((0 != msg.msg_flags) || (sizeof(mhdr) > l)
	   || (l != sizeof(mhdr) + mhdr.lin)
	   || (MSG_BUF_LEN < mhdr.lout)
	   || (0 > mhdr.cmd)
	  ) {
		fprintf(stderr, "CTL: Unexpected message. Ignoring");
		return;
	}

	mhdr.res = handle_message(mhdr.cmd, msg_inbuf, mhdr.lin,
				  msg_outbuf, mhdr.lout);

	if(0 > mhdr.res)
		memset(msg_outbuf, 0, mhdr.lout);

	iov[1].iov_base = msg_outbuf;
	iov[1].iov_len = mhdr.lout;
	l = sendmsg(fd, &msg, MSG_NOSIGNAL);
	if (l < 0) {
		fprintf(stderr, "CTL: Couldn't send response: %m");
	} else if (l != sizeof(mhdr) + mhdr.lout) {
		fprintf(stderr, "CTL: Couldn't send full response, sent %d bytes instead of %zd.", l, sizeof(mhdr) + mhdr.lout);
	}
}

int ctl_socket_init(void)
{
	int s = server_socket();
	if (0 > s)
		return -1;

	fd = s;
	loop = EV_DEFAULT;
	ev_io_init(&client_watcher, ctl_rcv_handler, fd, EV_READ);
	ev_io_start(loop, &client_watcher);

	CTL_init();

	return 0;
}

void ctl_socket_cleanup(void)
{
	CTL_cleanup();

	ev_io_stop(loop, &client_watcher);
	close(fd);

}
