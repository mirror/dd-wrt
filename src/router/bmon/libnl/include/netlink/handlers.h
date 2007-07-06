/*
 * netlink/handlers.h         Netlink Handlers
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
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

#ifndef NETLINK_HANDLERS_H_
#define NETLINK_HANDLERS_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <linux/types.h>
#include <netlink/netlink-compat.h>
#include <netlink/netlink-kernel.h>

/**
 * nl_recvmsgs() callback for message processing customization
 * @ingroup cb
 * @arg nla		netlink address of the peer
 * @arg nmsg		netlink message being processed
 * @arg arg		argument passwd on through caller
 */
typedef int (*nl_recvmsg_msg_cb_t)(struct sockaddr_nl *nla, struct nlmsghdr *nmsg,
				   void *arg);

/**
 * nl_recvmsgs() callback for error message processing customization
 * @ingroup cb
 * @arg nla		netlink address of the peer
 * @arg nlerr		netlink error message being processed
 * @arg arg		argument passed on through caller
 */
typedef int (*nl_recvmsg_err_cb_t)(struct sockaddr_nl *nla, struct nlmsgerr *nlerr,
				   void *arg);

/**
 * Callback actions
 * @ingroup cb
 */
enum nl_handler_action {
	/** Proceed with wathever would come next */
	NL_PROCEED,
	/** Skip this message */
	NL_SKIP,
	/** Stop parsing altogether and discard remaining messages */
	NL_EXIT,
};

struct nl_handle;

/**
 * Callback handlers mapping table
 * @ingroup cb
 */
struct nl_cb
{
	/** Called when a valid message was received.  */
	nl_recvmsg_msg_cb_t	cb_valid;
	/** cb_valid argument */
	void *			cb_valid_arg;

	/** Called upon end of a multipart message set */
	nl_recvmsg_msg_cb_t	cb_finish;
	/** cb_finish argument */
	void *			cb_finish_arg;

	/** Called when a netlink overrun occured */
	nl_recvmsg_msg_cb_t	cb_overrun;
	/** cb_overrun argument */
	void *			cb_overrun_arg;

	/** Called for messages requested to be skipped over */
	nl_recvmsg_msg_cb_t	cb_skipped;
	/** cb_skipped argument */
	void *			cb_skipped_arg;

	/** Called when an ACK message is received. */
	nl_recvmsg_msg_cb_t	cb_ack;
	/** cb_ack argument */
	void *			cb_ack_arg;

	/** Called whenever an error message is received */
	nl_recvmsg_err_cb_t	cb_error;
	/** cb_error argument */
	void *			cb_error_arg;

	/** Called for every message that comes in the socket. */
	nl_recvmsg_msg_cb_t	cb_msg_in;
	/** cb_msg_in argument */
	void *			cb_msg_in_arg;

	/** Called for every message that gets out the socket.  */
	int			(*cb_msg_out)(struct nlmsghdr *, void *);
	/** cb_msg_out argument */
	void *			cb_msg_out_arg;

	/** Called upon malformed messages such as messages
	 * with invalid sequence numbers. */
	nl_recvmsg_msg_cb_t	cb_invalid;
	/** cb_invalid argument */
	void *			cb_invalid_arg;

	/** Called for every message and may be used to overwrite
	 * default the sequence number checking. */
	nl_recvmsg_msg_cb_t	cb_seq_check;
	/** cb_seq_check argument */
	void *			cb_seq_check_arg;

	/** Called whenever a ACK must be sent back */
	nl_recvmsg_msg_cb_t	cb_send_ack;
	/** cb_send_ack argument */
	void *			cb_send_ack_arg;

	/** May be used to replace nl_recvmsgs with your own implementation
	 * in all internal calls to nl_recvmsgs. */
	int			(*cb_recvmsgs_ow)(struct nl_handle *,
						  struct nl_cb *);

	/** Overwrite internal calls to nl_recv, must return the number of
	 * octets read and allocate a buffer for the received data. */
	int			(*cb_recv_ow)(struct nl_handle *,
					      struct sockaddr_nl *,
					      unsigned char **);

	/** Overwrites internal calls to nl_send, must send the netlink
	 * message. */
	int			(*cb_send_ow)(struct nl_handle *,
					      struct nlmsghdr *);
};

extern void nl_use_default_handlers(struct nl_handle *);
extern void nl_use_default_verbose_handlers(struct nl_handle *);
extern void nl_use_default_debug_handlers(struct nl_handle *);

extern int nl_valid_handler_default(struct sockaddr_nl *,
                                    struct nlmsghdr *, void *);
extern int nl_valid_handler_verbose(struct sockaddr_nl *,
                                    struct nlmsghdr *, void *);
extern int nl_valid_handler_debug(struct sockaddr_nl *,
                                  struct nlmsghdr *, void *);
extern int nl_finish_handler_default(struct sockaddr_nl *,
                                     struct nlmsghdr *, void *);
extern int nl_finish_handler_verbose(struct sockaddr_nl *,
                                     struct nlmsghdr *, void *);
extern int nl_finish_handler_debug(struct sockaddr_nl *,
                                   struct nlmsghdr *, void *);
extern int nl_invalid_handler_default(struct sockaddr_nl *,
                                      struct nlmsghdr *, void *);
extern int nl_invalid_handler_verbose(struct sockaddr_nl *,
                                      struct nlmsghdr *, void *);
extern int nl_invalid_handler_debug(struct sockaddr_nl *,
                                    struct nlmsghdr *, void *);
extern int nl_msg_in_handler_default(struct sockaddr_nl *,
                                     struct nlmsghdr *, void *);
extern int nl_msg_in_handler_verbose(struct sockaddr_nl *,
                                     struct nlmsghdr *, void *);
extern int nl_msg_in_handler_debug(struct sockaddr_nl *,
                                   struct nlmsghdr *, void *);
extern int nl_overrun_handler_default(struct sockaddr_nl *,
                                      struct nlmsghdr *, void *);
extern int nl_overrun_handler_verbose(struct sockaddr_nl *,
                                      struct nlmsghdr *, void *);
extern int nl_overrun_handler_debug(struct sockaddr_nl *,
                                    struct nlmsghdr *, void *);
extern int nl_skipped_handler_default(struct sockaddr_nl *,
                                      struct nlmsghdr *, void *);
extern int nl_skipped_handler_verbose(struct sockaddr_nl *,
                                      struct nlmsghdr *, void *);
extern int nl_skipped_handler_debug(struct sockaddr_nl *,
                                    struct nlmsghdr *, void *);
extern int nl_ack_handler_default(struct sockaddr_nl *,
                                  struct nlmsghdr *, void *);
extern int nl_ack_handler_verbose(struct sockaddr_nl *,
                                  struct nlmsghdr *, void *);
extern int nl_ack_handler_debug(struct sockaddr_nl *,
                                struct nlmsghdr *, void *);
extern int nl_error_handler_default(struct sockaddr_nl *,
                                    struct nlmsgerr *, void *);
extern int nl_error_handler_verbose(struct sockaddr_nl *,
                                    struct nlmsgerr *, void *);
extern int nl_error_handler_debug(struct sockaddr_nl *,
                                  struct nlmsgerr *, void *);
extern int nl_msg_out_handler_default(struct nlmsghdr *, void *);
extern int nl_msg_out_handler_verbose(struct nlmsghdr *, void *);
extern int nl_msg_out_handler_debug(struct nlmsghdr *, void *);


#endif
