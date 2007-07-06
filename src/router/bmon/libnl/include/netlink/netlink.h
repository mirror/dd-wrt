/*
 * netlink/netlink.h         Netlink
 *
 * Copyright (c) 2003-2004 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#ifndef NETLINK_H_
#define NETLINK_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <linux/types.h>
#include <netlink/netlink-compat.h>
#include <netlink/netlink-kernel.h>
#include <netlink/rtnetlink-kernel.h>
#include <netlink/handlers.h>

extern int nl_debug;

/**
 * Netlink handle
 * @ingroup nl
 */
struct nl_handle
{
	/** File descriptor of netlink socket */
	int                 h_fd;
	/** Local netlink address */
	struct sockaddr_nl  h_local;
	/** Netlink address of peer */
	struct sockaddr_nl  h_peer;
	/** Next sequence number to use */
	unsigned int        h_seq_next;
	/** Sequence number of last message not yet acknowledged */
	unsigned int        h_seq_expect;
	/** Flags */
	int		    h_flags;
	/** Set of handlers */
	struct nl_cb        h_cb;
};

/**
 * @name Netlink handle flags
 * @{
 */

/** Set bufsize */
#define NL_SOCK_BUFSIZE_SET 1

/** @} */

/**
 * Initialize a netlink handle.
 * @ingroup nl
 */
#define NL_INIT_HANDLE() {0}

extern void nl_init_handle(struct nl_handle *);

extern void nl_join_groups(struct nl_handle *, int);

extern int  nl_connect(struct nl_handle *, int protocol);
extern void nl_close(struct nl_handle *);

extern int  nl_sendto(struct nl_handle *, unsigned char *, size_t);

extern int  nl_send(struct nl_handle *, struct nlmsghdr *);
extern int  nl_send_auto_complete(struct nl_handle *, struct nlmsghdr *);

extern int  nl_request(struct nl_handle *, int type, int flags);
extern int  nl_request_with_data(struct nl_handle *, int, int,
                                 unsigned char *, size_t);
extern int  nl_recv(struct nl_handle *, struct sockaddr_nl *,
                    unsigned char **buf);

extern int  nl_recvmsgs(struct nl_handle *, struct nl_cb *);
extern int  nl_recvmsgs_def(struct nl_handle *);

extern int  nl_wait_for_ack(struct nl_handle *);

#endif
