/*
 * nl.c           core netlink implementation
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

/**
 * @defgroup nl Core Netlink API
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>

/**
 * @name General
 * @{
 */

/**
 * Initialize a netlink handle.
 * @arg handle	netlink handle to initialize
 *
 * Initializes a netlink handle by setting the appropriate family, fills
 * out pid if not set already and initializes the ISN and ESN.
 *
 * @note You do not have to call this prior to nl_connect.
 */
void nl_init_handle(struct nl_handle *handle)
{
	handle->h_local.nl_family = AF_NETLINK;
	handle->h_peer.nl_family = AF_NETLINK;

	if (handle->h_local.nl_pid == 0)
		handle->h_local.nl_pid = getpid();

	handle->h_seq_expect = handle->h_seq_next = time(0);
}

/**
 * Set socket buffer size of netlink handle (socket).
 * @arg handle		netlink handle
 * @arg rxbuf		desired receive socket buffer size
 * @arg txbuf		desired transmit socket buffer size
 *
 * Sets the socket buffer size of a netlink handle (socket) to the
 * provided values \c rxbuf and \c txbuf. Providing a value of \c 0
 * assumes a good default value.
 *
 * @note You do not have to call this prior to nl_connect.
 * @return 0 or a negative error code (see setsockopt(2))
 */
int nl_set_buffer_size(struct nl_handle *handle, int rxbuf, int txbuf)
{
	int err;

	if (rxbuf <= 0)
		rxbuf = 32768;

	if (txbuf <= 0)
		txbuf = 32768;
	
	err = setsockopt(handle->h_fd, SOL_SOCKET, SO_SNDBUF, &txbuf, sizeof(txbuf));
	if (err < 0)
		return nl_error(errno, "setsockopt(SO_SNDBUF) failed");

	err = setsockopt(handle->h_fd, SOL_SOCKET, SO_RCVBUF, &rxbuf, sizeof(rxbuf));
	if (err < 0)
		return nl_error(errno, "setsockopt(SO_RCVBUF) failed");

	handle->h_flags |= NL_SOCK_BUFSIZE_SET;

	return 0;
}

void nl_join_groups(struct nl_handle *handle, int groups)
{
	handle->h_local.nl_groups |= groups;
}


/** @} */

/**
 * @name Connection Management
 * @{
 */

/**
 * Create a netlink socket and connect to it kernel.
 * @arg handle		netlink handle
 * @arg protocol	desired netlink protocol
 *
 * Creates a netlink socket of the desired \c protocol and connects it
 * to the kernel.
 *
 * @note nl_handle::local::nl_groups may contain a multicast group mask.
 * @exception EADDRNOTAVAIL Invalid address length
 * @exception EPFNOSUPPORT Address format not supported
 */
int nl_connect(struct nl_handle *handle, int protocol)
{
	int err;
	socklen_t addrlen;

	nl_init_handle(handle);

	if ((handle->h_fd = socket(AF_NETLINK, SOCK_RAW, protocol)) < 0)
		return nl_error(1, "socket(AF_NETLINK, ...) failed");

	if ((err = nl_set_buffer_size(handle, 0, 0)) < 0)
		return err;

	err = bind(handle->h_fd, (struct sockaddr*) &handle->h_local,
			sizeof(handle->h_local));
	if (err < 0)
		return nl_error(1, "bind() failed");

	addrlen = sizeof(handle->h_local);

	err = getsockname(handle->h_fd, (struct sockaddr *) &handle->h_local,
			&addrlen);
	if (err < 0)
		return nl_error(1, "getsockname failed");

	if (addrlen != sizeof(handle->h_local))
		return nl_error(EADDRNOTAVAIL, "Invalid address length");

	if (handle->h_local.nl_family != AF_NETLINK)
		return nl_error(EPFNOSUPPORT, "Address format not supported");

	return 0;
}

/**
 * Close a netlink socket
 * @arg handle		netlink handle
 */
void nl_close(struct nl_handle *handle)
{
	if (handle->h_fd >= 0) {
		close(handle->h_fd);
		handle->h_fd = -1;
	}
}


/** @} */

/**
 * @name Send
 * @{
 */

/**
 * Send raw data over the netlink socket.
 * @arg handle		netlink handle
 * @arg buf		data buffer
 * @arg len		length of data
 * @return See sendto(2)
 */
int nl_sendto(struct nl_handle *handle, unsigned char *buf, size_t len)
{
	return sendto(handle->h_fd, buf, len, 0, (struct sockaddr *)
	    &handle->h_peer, sizeof(handle->h_peer));
}


/**
 * Send a netlink message.
 * @arg handle		netlink handle
 * @arg nmsg		netlink message
 * @return see sendmsg(2)
 */
int nl_send(struct nl_handle *handle, struct nlmsghdr *nmsg)
{
	struct nl_cb *cb;

	struct iovec iov = {
		.iov_base = (void *) nmsg,
		.iov_len = nmsg->nlmsg_len,
	};

	struct msghdr msg = {
		.msg_name = (void *) &handle->h_peer,
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	cb = &handle->h_cb;
	if (cb->cb_msg_out)
		if (cb->cb_msg_out(nmsg, cb->cb_msg_out_arg) != NL_PROCEED)
			return 0;

	return sendmsg(handle->h_fd, &msg, 0);
}

/**
 * Send a netlink message and check & extend needed header values
 * @arg handle		netlink handle
 * @arg nmsg		netlink message
 *
 * Checks the netlink message \c nmsg for completness and extends it
 * as required before sending it out. Checked fields include pid,
 * sequence nr, and flags.
 *
 * @return see sendmsg(2)
 */
int nl_send_auto_complete(struct nl_handle *handle, struct nlmsghdr *nmsg)
{
	if (nmsg->nlmsg_pid == 0)
		nmsg->nlmsg_pid = handle->h_local.nl_pid;

	if (nmsg->nlmsg_seq == 0)
		nmsg->nlmsg_seq = handle->h_seq_next++;
	
	nmsg->nlmsg_flags |= (NLM_F_REQUEST | NLM_F_ACK);

	if (handle->h_cb.cb_send_ow)
		return handle->h_cb.cb_send_ow(handle, nmsg);
	else
		return nl_send(handle, nmsg);
}

/**
 * Send a netlink request message
 * @arg handle		netlink handle
 * @arg type		message type
 * @arg flags		message flags
 *
 * Fills out a netlink request message and sends it out using
 * nl_send_auto_complete()
 *
 * @return See sendmsg(2)
 */
int nl_request(struct nl_handle *handle, int type, int flags)
{
	struct nlmsghdr n = {
		.nlmsg_len = NLMSG_LENGTH(0),
		.nlmsg_type = type,
		.nlmsg_flags = flags,
	};

	return nl_send_auto_complete(handle, &n);
}

/**
 * Send a netlink request message with data.
 * @arg handle		netlink handle
 * @arg type		message type
 * @arg flags		message flags
 * @arg buf		data buffer
 * @arg len		length of data
 *
 * Fills out a netlink request message, appends the data to the tail
 * and sends it out using nl_send_auto_complete().
 *
 * @return See sendmsg(2)
 */
int nl_request_with_data(struct nl_handle *handle, int type, int flags,
			 unsigned char *buf, size_t len)
{
	int err = 0;
	struct nl_msg *m;
	struct nlmsghdr n = {
		.nlmsg_len = NLMSG_LENGTH(0),
		.nlmsg_type = type,
		.nlmsg_flags = flags,
	};

	m = nl_msg_build(&n);
	nl_msg_append_raw(m, buf, len);

	err = nl_send_auto_complete(handle, m->nmsg);
	nl_msg_free(m);
	return err;
}

/** @} */

/**
 * @name Receive
 * @{
 */

/**
 * Receive a netlink message from netlink socket.
 * @arg handle		netlink handle
 * @arg nla		target pointer for peer's netlink address
 * @arg buf		target pointer for message content.
 *
 * Receives a netlink message, allocates a buffer in \c *buf and
 * stores the message content. The peer's netlink address is stored
 * in \c *nla. The caller is responsible for freeing the buffer allocated
 * in \c *buf if a positive value is returned.  Interruped system calls
 * are handled by repeating the read. The input buffer size is determined
 * by peeking before the actual read is done.
 *
 * A non-blocking sockets causes the function to return immediately if
 * no data is available.
 *
 * @return Number of octets read, 0 on EOF or a negative error code.
 */
int nl_recv(struct nl_handle *handle, struct sockaddr_nl *nla, unsigned char **buf)
{
	int n;
	int flags = MSG_PEEK;

	struct iovec iov = {
		.iov_len = 4096,
	};

	struct msghdr msg = {
		.msg_name = (void *) nla,
		.msg_namelen = sizeof(sizeof(struct sockaddr_nl)),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0,
	};

	iov.iov_base = *buf = calloc(1, iov.iov_len);

retry:

	if ((n = recvmsg(handle->h_fd, &msg, flags)) <= 0) {
		if (!n)
			goto abort;
		else if (n < 0) {
			if (errno == EINTR)
				goto retry;
			else if (errno == EAGAIN)
				goto abort;
			else {
				free(*buf);
				return nl_error(errno, "recvmsg failed");
			}
		}
	}
	
	if (iov.iov_len < n) {
		/* Provided buffer is not long enough, enlarge it
		 * and try again. */
		iov.iov_len *= 2;
		iov.iov_base = *buf = realloc(*buf, iov.iov_len);
		goto retry;
	} else if (flags != 0) {
		/* Buffer is big enough, do the actual reading */
		flags = 0;
		goto retry;
	}

	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		free(*buf);
		return nl_error(EADDRNOTAVAIL, "socket address size mismatch");
	}

	return n;

abort:
	free(*buf);
	return 0;
}


/**
 * Receive a set of messages from a netlink socket.
 * @arg handle		netlink handle
 * @arg cb		set of callbacks to control the behaviour.
 *
 * Repeatedly calls nl_recv() and parses the messages as netlink
 * messages. Stops reading if one of the callbacks returns
 * NL_EXIT or nl_recv returns either 0 or a negative error code.
 *
 * A non-blocking sockets causes the function to return immediately if
 * no data is available.
 *
 * @return 0 on success or a negative error code from nl_recv().
 * @see \ref Handlers
 */
int nl_recvmsgs(struct nl_handle *handle, struct nl_cb *cb)
{
	int n, err = 0;
	unsigned char *buf = NULL;
	struct nlmsghdr *hdr;
	struct sockaddr_nl nla = {0};

continue_reading:
	if (cb->cb_recv_ow)
		n = cb->cb_recv_ow(handle, &nla, &buf);
	else
		n = nl_recv(handle, &nla, &buf);

	if (n <= 0)
		return n;

	hdr = (struct nlmsghdr *) buf;
	while (NLMSG_OK(hdr, n)) {

		/* Raw callback is the first, it gives the most control
		 * to the user and he can do his very own parsing. */
		if (cb->cb_msg_in) {
			err = cb->cb_msg_in(&nla, hdr, cb->cb_msg_in_arg);
			if (err == NL_SKIP)
				goto skip;
			else if (err == NL_EXIT || err < 0)
				goto out;
		}

		/* Sequence number checking. The check may be done by
		 * the user, otherwise a very simple check is applied
		 * enforcing strict ordering */
		if (cb->cb_seq_check) {
			err = cb->cb_seq_check(&nla, hdr, cb->cb_seq_check_arg);
			if (err == NL_SKIP)
				goto skip;
			else if (err == NL_EXIT || err < 0)
				goto out;
		} else if (hdr->nlmsg_seq != handle->h_seq_expect) {
			if (cb->cb_invalid) {
				err = cb->cb_invalid(&nla, hdr,
						     cb->cb_invalid_arg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			} else
				goto out;
		}

		if (hdr->nlmsg_type == NLMSG_DONE ||
		    hdr->nlmsg_type == NLMSG_ERROR ||
		    hdr->nlmsg_type == NLMSG_NOOP ||
		    hdr->nlmsg_type == NLMSG_OVERRUN) {
			/* We can't check for !NLM_F_MULTI since some netlink
			 * users in the kernel are broken. */
			handle->h_seq_expect++;
		}
	
		/* Other side wishes to see an ack for this message */
		if (hdr->nlmsg_flags & NLM_F_ACK) {
			if (cb->cb_send_ack) {
				err = cb->cb_send_ack(&nla, hdr,
						      cb->cb_send_ack_arg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			} else {
				/* FIXME: implement */
			}
		}

		/* messages terminates a multpart message, this is
		 * usually the end of a message and therefore we slip
		 * out of the loop by default. the user may overrule
		 * this action by skipping this packet. */
		if (hdr->nlmsg_type == NLMSG_DONE) {
			if (cb->cb_finish) {
				err = cb->cb_finish(&nla, hdr,
						    cb->cb_finish_arg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			}
			goto out;
		}

		/* Message to be ignored, the default action is to
		 * skip this message if no callback is specified. The
		 * user may overrule this action by returning
		 * NL_PROCEED. */
		else if (hdr->nlmsg_type == NLMSG_NOOP) {
			if (cb->cb_skipped) {
				err = cb->cb_skipped(&nla, hdr,
						     cb->cb_skipped_arg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			} else
				goto skip;
		}

		/* Data got lost, report back to user. The default action is to
		 * quit parsing. The user may overrule this action by retuning
		 * NL_SKIP or NL_PROCEED (dangerous) */
		else if (hdr->nlmsg_type == NLMSG_OVERRUN) {
			if (cb->cb_overrun) {
				err = cb->cb_overrun(&nla, hdr,
						     cb->cb_overrun_arg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			} else
				goto out;
		}

		/* Message carries a nlmsgerr */
		else if (hdr->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *e = (struct nlmsgerr*) NLMSG_DATA(hdr);

			if (hdr->nlmsg_len < NLMSG_LENGTH(sizeof(*e))) {
				/* Truncated error message, the default action
				 * is to stop parsing. The user may overrule
				 * this action by returning NL_SKIP or
				 * NL_PROCEED (dangerous) */
				if (cb->cb_invalid) {
					err = cb->cb_invalid(&nla, hdr,
							cb->cb_invalid_arg);
					if (err == NL_SKIP)
						goto skip;
					else if (err == NL_EXIT || err < 0)
						goto out;
				} else
					goto out;
			} else if (e->error) {
				/* Error message reported back from kernel. */
				if (cb->cb_error) {
					err = cb->cb_error(&nla, e,
							   cb->cb_error_arg);
					if (err == NL_SKIP)
						goto skip;
					else if (err == NL_EXIT || err < 0)
						goto out;
				} else
					goto out;
			} else if (cb->cb_ack) {
				/* ACK */
				err = cb->cb_ack(&nla, hdr, cb->cb_ack_arg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			}
		} else {
			/* Valid message (not checking for MULTIPART bit to
			 * get along with broken kernels. NL_SKIP has no
			 * effect on this.  */
			if (cb->cb_valid) {
				err = cb->cb_valid(&nla, hdr, cb->cb_valid_arg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			}
		}
skip:
		hdr = NLMSG_NEXT(hdr, n);
	}
	
	if (buf) {
		free(buf);
		buf = NULL;
	}

	/* Multipart message not yet complete, continue reading */
	goto continue_reading;

out:
	if (buf)
		free(buf);

	return err;
}

/**
 * Receive a set of message from a netlink socket using handlers in nl_handle.
 * @arg handle		netlink handle
 *
 * Calls nl_recvmsgs() with the handlers configured in the netlink handle.
 * 
 * @see \ref Handlers
 */
int nl_recvmsgs_def(struct nl_handle *handle)
{
	if (handle->h_cb.cb_recvmsgs_ow)
		return handle->h_cb.cb_recvmsgs_ow(handle, &handle->h_cb);
	else
		return nl_recvmsgs(handle, &handle->h_cb);
}

static int ack_wait_handler(struct sockaddr_nl *who, struct nlmsghdr *n,
			    void *arg)
{
	return NL_EXIT;
}

/**
 * Wait for ACK.
 * @arg handle		netlink handle
 * @pre The netlink socket must be in blocking state.
 *
 * Waits until an ACK is received for the latest not yet acknoledged
 * netlink message.
 */
int nl_wait_for_ack(struct nl_handle *handle)
{
	struct nl_cb cb;

	memcpy(&cb, &handle->h_cb, sizeof(cb));
	cb.cb_ack = ack_wait_handler;

	return nl_recvmsgs(handle, &cb);
}


/** @} */

/**
 * @name Netlink Family Translations
 * @{
 */

static struct trans_tbl nlfamilies[] = {
	__ADD(NETLINK_ROUTE,route)
	__ADD(NETLINK_SKIP,skip)
	__ADD(NETLINK_USERSOCK,usersock)
	__ADD(NETLINK_FIREWALL,firewall)
	__ADD(NETLINK_TCPDIAG,tcpdiag)
	__ADD(NETLINK_NFLOG,nflog)
	__ADD(NETLINK_XFRM,xfrm)
	__ADD(NETLINK_SELINUX,selinux)
	__ADD(NETLINK_ARPD,arpd)
	__ADD(NETLINK_AUDIT,audit)
	__ADD(NETLINK_ROUTE6,route6)
	__ADD(NETLINK_IP6_FW,ip6_fw)
	__ADD(NETLINK_DNRTMSG,dnrtmsg)
	__ADD(NETLINK_KOBJECT_UEVENT,kobject_uevent)
	__ADD(NETLINK_TAPBASE,tapbase)
};

/**
 * Convert a netlink family to a character string (Reentrant).
 * @arg family		netlink family
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts a netlink family to a character string and stores it in
 * the specified destination buffer.
 *
 * @return The destination buffer or the family encoded in hexidecimal
 *         form if no match was found.
 */
char * nl_nlfamily2str_r(int family, char *buf, size_t len)
{
	return __type2str_r(family, buf, len, nlfamilies,
	    ARRAY_SIZE(nlfamilies));
}

/**
 * Convert a netlink family to a character string.
 * @arg family		netlink family
 *
 * Converts a netlink family to a character string and stores it in a
 * static buffer.
 *
 * @return A static buffer or the family encoded in hexidecimal
 *         form if no match was found.
 * @attention This funnction is NOT thread safe.
 */
char * nl_nlfamily2str(int family)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	return __type2str_r(family, buf, sizeof(buf), nlfamilies,
	    ARRAY_SIZE(nlfamilies));
}

/**
 * Convert a character string to a netlink family
 * @arg name		name of netlink family
 *
 * Converts the provided character string specifying a netlink
 * family to the corresponding numeric value.
 *
 * @return Netlink family negative value if none was found.
 */
int nl_str2nlfamily(const char *name)
{
	return __str2type(name, nlfamilies, ARRAY_SIZE(nlfamilies));
}

/** @} */
/** @} */
