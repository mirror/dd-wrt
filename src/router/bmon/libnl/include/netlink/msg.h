/*
 * netlink/msg.h             Netlink Message Helpers
 *
 * Copyright (c) 2003-2005 Thomas Graf <tgraf@suug.ch>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef NETLINK_MSG_H_
#define NETLINK_MSG_H_

#include <netlink/netlink.h>

/**
 * Abstract representation of a netlink message
 * @ingroup msg
 */
struct nl_msg
{
	/** Netlink message header with data appended to the tail */
	struct nlmsghdr *nmsg;
};

/**
 * Tail of netlink message.
 * @ingroup msg
 * @arg nlh		netlink message header
 */
#define NLMSG_TAIL(nlh) \
	(((void *) (nlh)) + NLMSG_ALIGN((nlh)->nlmsg_len))

extern struct nl_msg *	  nl_msg_build(struct nlmsghdr *);
extern int		  nl_msg_append_raw(struct nl_msg *, void *, size_t);
extern struct nlmsghdr *  nl_msg_get(struct nl_msg *);
extern void *		  nl_msg_payload(struct nl_msg *);
extern size_t		  nl_msg_payloadlen(struct nl_msg *);
extern void		  nl_msg_free(struct nl_msg *);

extern char *		  nl_nlmsgtype2str(int);
extern char *		  nl_nlmsgtype2str_r(int, char *, size_t);
extern int		  nl_str2nlmsgtype(const char *);

extern char *		  nl_nlmsg_flags2str_r(int, char *, size_t);
extern char *		  nl_nlmsg_flags2str(int);

#endif
