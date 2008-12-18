/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.

  This program is free software; you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the Free 
  Software Foundation; either version 2 of the License, or (at your option) 
  any later version.
  
  This program is distributed in the hope that it will be useful, but WITHOUT 
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
  more details.
  
  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59 
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  
  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>

******************************************************************************/

#include "ctl_socket.h"
#include "ctl_socket_server.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>

#include "epoll_loop.h"
#include "log.h"

int server_socket(void)
{
	struct sockaddr_un sa;
	int s;

	TST(strlen(RSTP_SERVER_SOCK_NAME) < sizeof(sa.sun_path), -1);

	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (s < 0) {
		ERROR("Couldn't open unix socket: %m");
		return -1;
	}

	set_socket_address(&sa, RSTP_SERVER_SOCK_NAME);

	if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
		ERROR("Couldn't bind socket: %m");
		close(s);
		return -1;
	}

	return s;
}

int handle_message(int cmd, void *inbuf, int lin, void *outbuf, int lout)
{
	switch (cmd) {
		SERVER_MESSAGE_CASE(enable_bridge_rstp);
		SERVER_MESSAGE_CASE(get_bridge_status);
		SERVER_MESSAGE_CASE(set_bridge_config);
		SERVER_MESSAGE_CASE(get_port_status);
		SERVER_MESSAGE_CASE(set_port_config);
		SERVER_MESSAGE_CASE(port_mcheck);
		SERVER_MESSAGE_CASE(set_debug_level);

	default:
		ERROR("CTL: Unknown command %d", cmd);
		return -1;
	}
}

int ctl_in_handler = 0;
static unsigned char msg_logbuf[1024];
static unsigned int msg_log_offset;
void _ctl_err_log(char *fmt, ...)
{
	if (msg_log_offset >= sizeof(msg_logbuf) - 1)
		return;
	int r;
	va_list ap;
	va_start(ap, fmt);
	r = vsnprintf((char *)msg_logbuf + msg_log_offset,
		      sizeof(msg_logbuf) - msg_log_offset,
		      fmt, ap);
	va_end(ap);
	msg_log_offset += r;
	if (msg_log_offset == sizeof(msg_logbuf)) {
		msg_log_offset = sizeof(msg_logbuf) - 1;
		msg_logbuf[sizeof(msg_logbuf) - 1] = 0;
	}
}


#define msg_buf_len 1024
static unsigned char msg_inbuf[1024];
static unsigned char msg_outbuf[1024];

void ctl_rcv_handler(uint32_t events, struct epoll_event_handler *p)
{
	struct ctl_msg_hdr mhdr;
	struct msghdr msg;
	struct sockaddr_un sa;
	struct iovec iov[3];
	int l;

	msg.msg_name = &sa;
	msg.msg_namelen = sizeof(sa);
	msg.msg_iov = iov;
	msg.msg_iovlen = 3;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	iov[0].iov_base = &mhdr;
	iov[0].iov_len = sizeof(mhdr);
	iov[1].iov_base = msg_inbuf;
	iov[1].iov_len = msg_buf_len;
	iov[2].iov_base = NULL;
	iov[2].iov_len = 0;
	l = recvmsg(p->fd, &msg, MSG_NOSIGNAL | MSG_DONTWAIT);
	TST(l > 0,);
	if (msg.msg_flags != 0 || l < sizeof(mhdr) ||
	    l != sizeof(mhdr) + mhdr.lin ||
	    mhdr.lout > msg_buf_len || mhdr.cmd < 0) {
		ERROR("CTL: Unexpected message. Ignoring");
		return;
	}

	msg_log_offset = 0;
	ctl_in_handler = 1;

	mhdr.res = handle_message(mhdr.cmd, msg_inbuf, mhdr.lin,
				  msg_outbuf, mhdr.lout);

	ctl_in_handler = 0;
	if (mhdr.res < 0)
		memset(msg_outbuf, 0, mhdr.lout);
	if (msg_log_offset < mhdr.llog)
		mhdr.llog = msg_log_offset;
	
	iov[1].iov_base = msg_outbuf;
	iov[1].iov_len = mhdr.lout;
	iov[2].iov_base = msg_logbuf;
	iov[2].iov_len = mhdr.llog;
	l = sendmsg(p->fd, &msg, MSG_NOSIGNAL);
	if (l < 0)
		ERROR("CTL: Couldn't send response: %m");
	else if (l != sizeof(mhdr) + mhdr.lout) {
		ERROR
		    ("CTL: Couldn't send full response, sent %d bytes instead of %zd.",
		     l, sizeof(mhdr) + mhdr.lout);
	}
}

struct epoll_event_handler ctl_handler;

int ctl_socket_init(void)
{
	int s = server_socket();
	if (s < 0)
		return -1;

	ctl_handler.fd = s;
	ctl_handler.handler = ctl_rcv_handler;

	TST(add_epoll(&ctl_handler) == 0, -1);
	return 0;
}

void ctl_socket_cleanup(void)
{
	remove_epoll(&ctl_handler);
	close(ctl_handler.fd);
}
