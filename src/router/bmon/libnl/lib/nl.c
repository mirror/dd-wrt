/*
 * lib/nl.c		Core Netlink Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @defgroup nl Core Netlink API
 * @brief
 *
 * @par 1) Creating the netlink handle
 * @code
 * struct nl_handle *handle;
 *
 * // Allocate and initialize a new netlink handle
 * handle = nl_handle_new();
 *
 * // Are multiple handles being allocated? You have to provide a unique
 * // netlink process id and overwrite the default local process id.
 * nl_handle_set_pid(handle, MY_UNIQUE_PID);
 *
 * // Is this socket used for event processing? You need to disable sequence
 * // number checking in order to be able to receive messages not explicitely
 * // requested.
 * nl_disable_sequence_check(handle);
 *
 * // Use nl_handle_get_fd() to fetch the file description, for example to
 * // put a socket into non-blocking i/o mode.
 * fcntl(nl_handle_get_fd(handle), F_SETFL, O_NONBLOCK);
 * @endcode
 *
 * @par 2) Joining Groups
 * @code
 * // You may join/subscribe to as many groups as you want, don't forget
 * // to eventually disable sequence number checking. Note: Joining must
 * // be done before connecting/binding the socket.
 * nl_join_groups(handle, GROUP_ID1 | GROUP_ID2);
 * @endcode
 *
 * @par 3) Connecting the socket
 * @code
 * // Bind and connect the socket to a protocol, NETLINK_ROUTE in this example.
 * nl_connect(handle, NETLINK_ROUTE);
 * @endcode
 *
 * @par 4) Sending data
 * @code
 * // The most rudimentary method is to use nl_sendto() simply pushing
 * // a piece of data to the other netlink peer. This method is not
 * // recommended.
 * const char buf[] = { 0x01, 0x02, 0x03, 0x04 };
 * nl_sendto(handle, buf, sizeof(buf));
 *
 * // A more comfortable interface is nl_send() taking a pointer to
 * // a netlink message.
 * struct nl_msg *msg = my_msg_builder();
 * nl_send(handle, nlmsg_hdr(msg));
 *
 * // nl_sendmsg() provides additional control over the sendmsg() message
 * // header in order to allow more specific addressing of multiple peers etc.
 * struct msghdr hdr = { ... };
 * nl_sendmsg(handle, nlmsg_hdr(msg), &hdr);
 *
 * // You're probably too lazy to fill out the netlink pid, sequence number
 * // and message flags all the time. nl_send_auto_complete() automatically
 * // extends your message header as needed with an appropriate sequence
 * // number, the netlink pid stored in the netlink handle and the message
 * // flags NLM_F_REQUEST and NLM_F_ACK
 * nl_send_auto_complete(handle, nlmsg_hdr(msg));
 *
 * // Simple protocols don't require the complex message construction interface
 * // and may favour nl_send_simple() to easly send a bunch of payload
 * // encapsulated in a netlink message header.
 * nl_send_simple(handle, MY_MSG_TYPE, 0, buf, sizeof(buf));
 * @endcode
 *
 * @par 5) Receiving data
 * @code
 * // nl_recv() receives a single message allocating a buffer for the message
 * // content and gives back the pointer to you.
 * struct sockaddr_nl peer;
 * unsigned char *msg;
 * nl_recv(handle, &peer, &msg);
 *
 * // nl_recvmsgs() receives a bunch of messages until the callback system
 * // orders it to state, usually after receving a compolete multi part
 * // message series.
 * nl_recvmsgs(handle, my_callback_configuration);
 *
 * // nl_recvmsgs_def() acts just like nl_recvmsg() but uses the callback
 * // configuration stored in the handle.
 * nl_recvmsgs_def(handle);
 *
 * // In case you want to wait for the ACK to be recieved that you requested
 * // with your latest message, you can call nl_wait_for_ack()
 * nl_wait_for_ack(handle);
 * @endcode
 *
 * @par 6) Cleaning up
 * @code
 * // Close the socket first to release kernel memory
 * nl_close(handle);
 *
 * // Finally destroy the netlink handle
 * nl_handle_destroy(handle);
 * @endcode
 * 
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/handlers.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

/**
 * @name Handle Management
 * @{
 */

/**
 * Allocate and initialize new non-default netlink handle.
 * @arg kind		Kind of callback handler to use per default.
 *
 * Allocates and initializes a new netlink handle, the netlink process id
 * is set to the local process id which may conflict if multiple handles
 * are created, therefore you may have to overwrite it using
 * nl_handle_set_pid(). The initial sequence number is initialized to the
 * current UNIX time.
 *
 * @return Newly allocated netlink handle or NULL.
 */
struct nl_handle *nl_handle_alloc_nondefault(enum nl_cb_kind kind)
{
	struct nl_handle *handle;
	
	handle = calloc(1, sizeof(*handle));
	if (!handle)
		goto errout;

	handle->h_cb = nl_cb_new(kind);
	if (!handle->h_cb)
		goto errout;
	
	handle->h_local.nl_family = AF_NETLINK;
	handle->h_peer.nl_family = AF_NETLINK;
	handle->h_local.nl_pid = getpid();
	handle->h_seq_expect = handle->h_seq_next = time(0);

	return handle;
errout:
	nl_handle_destroy(handle);
	nl_errno(ENOMEM);
	return NULL;
}

/**
 * Allocate and initialize new netlink handle.
 *
 * Allocates and initializes a new netlink handle, the netlink process id
 * is set to the local process id which may conflict if multiple handles
 * are created, therefore you may have to overwrite it using
 * nl_handle_set_pid(). The initial sequence number is initialized to the
 * current UNIX time. The default callback (NL_CB_DEFAULT) handlers are
 * being used.
 *
 * @return Newly allocated netlink handle or NULL.
 */
struct nl_handle *nl_handle_alloc(void)
{
	return nl_handle_alloc_nondefault(NL_CB_DEFAULT);
}

/**
 * Destroy netlink handle.
 * @arg handle		Netlink handle.
 */
void nl_handle_destroy(struct nl_handle *handle)
{
	if (!handle)
		return;

	nl_cb_destroy(handle->h_cb);
	free(handle);
}

/** @} */

/**
 * @name Utilities
 * @{
 */

/**
 * Set socket buffer size of netlink handle.
 * @arg handle		Netlink handle.
 * @arg rxbuf		New receive socket buffer size in bytes.
 * @arg txbuf		New transmit socket buffer size in bytes.
 *
 * Sets the socket buffer size of a netlink handle to the specified
 * values \c rxbuf and \c txbuf. Providing a value of \c 0 assumes a
 * good default value.
 *
 * @note It is not required to call this function prior to nl_connect().
 * @return 0 on sucess or a negative error code.
 */
int nl_set_buffer_size(struct nl_handle *handle, int rxbuf, int txbuf)
{
	int err;

	if (rxbuf <= 0)
		rxbuf = 32768;

	if (txbuf <= 0)
		txbuf = 32768;
	
	err = setsockopt(handle->h_fd, SOL_SOCKET, SO_SNDBUF,
			 &txbuf, sizeof(txbuf));
	if (err < 0)
		return nl_error(errno, "setsockopt(SO_SNDBUF) failed");

	err = setsockopt(handle->h_fd, SOL_SOCKET, SO_RCVBUF,
			 &rxbuf, sizeof(rxbuf));
	if (err < 0)
		return nl_error(errno, "setsockopt(SO_RCVBUF) failed");

	handle->h_flags |= NL_SOCK_BUFSIZE_SET;

	return 0;
}

/**
 * Enable/disable credential passing on netlink handle.
 * @arg handle		Netlink handle
 * @arg state		New state (0 - disabled, 1 - enabled)
 */
int nl_set_passcred(struct nl_handle *handle, int state)
{
	int err;

	err = setsockopt(handle->h_fd, SOL_SOCKET, SO_PASSCRED,
			 &state, sizeof(state));
	if (err < 0)
		return nl_error(errno, "setsockopt(SO_PASSCRED) failed");

	if (state)
		handle->h_flags |= NL_SOCK_PASSCRED;
	else
		handle->h_flags &= ~NL_SOCK_PASSCRED;

	return 0;
}

/**
 * Join multicast groups.
 * @arg handle		Netlink handle.
 * @arg groups		Bitmask of groups to join.
 *
 * @note Joining of groups must be done prior to connecting/binding
 *       the socket (nl_connect()).
 */
void nl_join_groups(struct nl_handle *handle, int groups)
{
	handle->h_local.nl_groups |= groups;
}

#ifndef SOL_NETLINK
#define SOL_NETLINK 270
#endif

int nl_join_group(struct nl_handle *handle, int group)
{
	int err;

	err = setsockopt(handle->h_fd, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP,
			 &group, sizeof(group));
	if (err < 0)
		return nl_error(errno, "setsockopt(NETLINK_ADD_MEMBERSHIP) "
				       "failed");

	return 0;
}

static int noop_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_PROCEED;
}

/**
 * Disable sequence number checking.
 * @arg handle		Netlink handle.
 *
 * Disables checking of sequence numbers on the netlink handle. This is
 * required to allow messages to be processed which were not requested by
 * a preceding request message, e.g. netlink events.
 */
void nl_disable_sequence_check(struct nl_handle *handle)
{
	nl_cb_set(nl_handle_get_cb(handle), NL_CB_SEQ_CHECK,
		  NL_CB_CUSTOM, noop_seq_check, NULL);
}

/** @} */

/**
 * @name Acccess Functions
 * @{
 */

/**
 * Get netlink process identifier of netlink handle.
 * @arg handle		Netlink handle.
 * @return Netlink process identifier.
 */
pid_t nl_handle_get_pid(struct nl_handle *handle)
{
	return handle->h_local.nl_pid;
}

/**
 * Set netlink process identifier of netlink handle.
 * @arg handle		Netlink handle.
 * @arg pid		New netlink process identifier.
 */
void nl_handle_set_pid(struct nl_handle *handle, pid_t pid)
{
	handle->h_local.nl_pid = pid;
}

/**
 * Get netlink process identifier of peer from netlink handle.
 * @arg handle		Netlink handle.
 * @return Netlink process identifier.
 */
pid_t nl_handle_get_peer_pid(struct nl_handle *handle)
{
	return handle->h_peer.nl_pid;
}

/**
 * Set netlink process identifier of peer in netlink handle.
 * @arg handle		Netlink handle.
 * @arg pid		New netlink process identifier.
 */
void nl_handle_set_peer_pid(struct nl_handle *handle, pid_t pid)
{
	handle->h_peer.nl_pid = pid;
}

/**
 * Get file descriptor of netlink handle.
 * @arg handle		Netlink handle.
 * @return File descriptor of netlink socket or -1 if not connected.
 */
int nl_handle_get_fd(struct nl_handle *handle)
{
	return handle->h_fd;
}

/**
 * Get local netlink address of netlink handle.
 * @arg handle		Netlink handle.
 * @return Local netlink address.
 */
struct sockaddr_nl *nl_handle_get_local_addr(struct nl_handle *handle)
{
	return &handle->h_local;
}

/**
 * Get peer netlink address of netlink handle.
 * @arg handle		Netlink handle.
 * @note The peer address is undefined while the socket is unconnected.
 * @return Netlink address of the peer.
 */
struct sockaddr_nl *nl_handle_get_peer_addr(struct nl_handle *handle)
{
	return &handle->h_peer;
}

/**
 * Get callback configuration of netlink handle.
 * @arg handle		Netlink handle.
 * @return Currently active callback configuration or NULL if not available.
 */
struct nl_cb *nl_handle_get_cb(struct nl_handle *handle)
{
	return handle->h_cb;
}

/** @} */

/**
 * @name Connection Management
 * @{
 */

/**
 * Create and connect netlink socket.
 * @arg handle		Netlink handle.
 * @arg protocol	Netlink protocol to use.
 *
 * Creates a netlink socket using the specified protocol, binds the socket
 * and issues a connection attempt.
 *
 * @return 0 on success or a negative error code.
 */
int nl_connect(struct nl_handle *handle, int protocol)
{
	int err;
	socklen_t addrlen;

	handle->h_fd = socket(AF_NETLINK, SOCK_RAW, protocol);
	if (handle->h_fd < 0)
		return nl_error(1, "socket(AF_NETLINK, ...) failed");

	if (!(handle->h_flags & NL_SOCK_BUFSIZE_SET)) {
		err = nl_set_buffer_size(handle, 0, 0);
		if (err < 0)
			return err;
	}

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

	handle->h_proto = protocol;

	return 0;
}

/**
 * Close/Disconnect netlink socket.
 * @arg handle		Netlink handle
 */
void nl_close(struct nl_handle *handle)
{
	if (handle->h_fd >= 0) {
		close(handle->h_fd);
		handle->h_fd = -1;
	}

	handle->h_proto = 0;
}

/** @} */

/**
 * @name Send
 * @{
 */

/**
 * Send raw data over netlink socket.
 * @arg handle		Netlink handle.
 * @arg buf		Data buffer.
 * @arg size		Size of data buffer.
 * @return Number of characters written on success or a negative error code.
 */
int nl_sendto(struct nl_handle *handle, void *buf, size_t size)
{
	int ret;

	ret = sendto(handle->h_fd, buf, size, 0, (struct sockaddr *)
		     &handle->h_peer, sizeof(handle->h_peer));
	if (ret < 0)
		return nl_errno(errno);

	return ret;
}

/**
 * Send netlink message with control over sendmsg() message header.
 * @arg handle		Netlink handle.
 * @arg msg		Netlink message to be sent.
 * @arg hdr		Sendmsg() message header.
 * @return Number of characters sent on sucess or a negative error code.
 */
int nl_sendmsg(struct nl_handle *handle, struct nl_msg *msg, struct msghdr *hdr)
{
	struct nl_cb *cb;
	int ret;

	struct iovec iov = {
		.iov_base = (void *) nlmsg_hdr(msg),
		.iov_len = nlmsg_hdr(msg)->nlmsg_len,
	};

	hdr->msg_iov = &iov;
	hdr->msg_iovlen = 1;

	nlmsg_set_src(msg, &handle->h_local);

	cb = nl_handle_get_cb(handle);
	if (cb->cb_set[NL_CB_MSG_OUT])
		if (nl_cb_call(cb, NL_CB_MSG_OUT, msg) != NL_PROCEED)
			return 0;

	ret = sendmsg(handle->h_fd, hdr, 0);
	if (ret < 0)
		return nl_errno(errno);

	return ret;
}


/**
 * Send netlink message.
 * @arg handle		Netlink handle
 * @arg msg		Netlink message to be sent.
 * @see nl_sendmsg()
 * @return Number of characters sent on success or a negative error code.
 */
int nl_send(struct nl_handle *handle, struct nl_msg *msg)
{
	struct sockaddr_nl *dst;
	struct ucred *creds;
	
	struct msghdr hdr = {
		.msg_name = (void *) &handle->h_peer,
		.msg_namelen = sizeof(struct sockaddr_nl),
	};

	/* Overwrite destination if specified in the message itself, defaults
	 * to the peer address of the handle.
	 */
	dst = nlmsg_get_dst(msg);
	if (dst->nl_family == AF_NETLINK)
		hdr.msg_name = dst;

	/* Add credentials if present. */
	creds = nlmsg_get_creds(msg);
	if (creds != NULL) {
		char buf[CMSG_SPACE(sizeof(struct ucred))];
		struct cmsghdr *cmsg;

		hdr.msg_control = buf;
		hdr.msg_controllen = sizeof(buf);

		cmsg = CMSG_FIRSTHDR(&hdr);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_CREDENTIALS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(struct ucred));
		memcpy(CMSG_DATA(cmsg), creds, sizeof(struct ucred));
	}

	return nl_sendmsg(handle, msg, &hdr);
}

/**
 * Send netlink message and check & extend header values as needed.
 * @arg handle		Netlink handle.
 * @arg msg		Netlink message to be sent.
 *
 * Checks the netlink message \c nlh for completness and extends it
 * as required before sending it out. Checked fields include pid,
 * sequence nr, and flags.
 *
 * @see nl_send()
 * @return Number of characters sent or a negative error code.
 */
int nl_send_auto_complete(struct nl_handle *handle, struct nl_msg *msg)
{
	struct nlmsghdr *nlh;

	nlh = nlmsg_hdr(msg);
	if (nlh->nlmsg_pid == 0)
		nlh->nlmsg_pid = handle->h_local.nl_pid;

	if (nlh->nlmsg_seq == 0)
		nlh->nlmsg_seq = handle->h_seq_next++;
	
	nlh->nlmsg_flags |= (NLM_F_REQUEST | NLM_F_ACK);

	if (handle->h_cb->cb_send_ow)
		return handle->h_cb->cb_send_ow(handle, msg);
	else
		return nl_send(handle, msg);
}

/**
 * Send simple netlink message using nl_send_auto_complete()
 * @arg handle		Netlink handle.
 * @arg type		Netlink message type.
 * @arg flags		Netlink message flags.
 * @arg buf		Data buffer.
 * @arg size		Size of data buffer.
 *
 * Builds a netlink message with the specified type and flags and
 * appends the specified data as payload to the message.
 *
 * @see nl_send_auto_complete()
 * @return Number of characters sent on success or a negative error code.
 */
int nl_send_simple(struct nl_handle *handle, int type, int flags, void *buf,
		   size_t size)
{
	int err;
	struct nl_msg *msg;
	struct nlmsghdr nlh = {
		.nlmsg_len = nlmsg_msg_size(0),
		.nlmsg_type = type,
		.nlmsg_flags = flags,
	};

	msg = nlmsg_build(&nlh);
	if (!msg)
		return nl_errno(ENOMEM);

	if (buf && size)
		nlmsg_append(msg, buf, size, 1);

	err = nl_send_auto_complete(handle, msg);
	nlmsg_free(msg);

	return err;
}

/** @} */

/**
 * @name Receive
 * @{
 */

/**
 * Receive netlink message from netlink socket.
 * @arg handle		Netlink handle.
 * @arg nla		Destination pointer for peer's netlink address.
 * @arg buf		Destination pointer for message content.
 * @arg creds		Destination pointer for credentials.
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
int nl_recv(struct nl_handle *handle, struct sockaddr_nl *nla,
	    unsigned char **buf, struct ucred **creds)
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
	struct cmsghdr *cmsg;

	iov.iov_base = *buf = calloc(1, iov.iov_len);

	if (handle->h_flags & NL_SOCK_PASSCRED) {
		msg.msg_controllen = CMSG_SPACE(sizeof(struct ucred));
		msg.msg_control = calloc(1, msg.msg_controllen);
	}
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
				free(msg.msg_control);
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
	} else if (msg.msg_flags & MSG_CTRUNC) {
		msg.msg_controllen *= 2;
		msg.msg_control = realloc(msg.msg_control, msg.msg_controllen);
		goto retry;
	} else if (flags != 0) {
		/* Buffer is big enough, do the actual reading */
		flags = 0;
		goto retry;
	}

	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		free(msg.msg_control);
		free(*buf);
		return nl_error(EADDRNOTAVAIL, "socket address size mismatch");
	}

	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET &&
		    cmsg->cmsg_type == SCM_CREDENTIALS) {
			*creds = calloc(1, sizeof(struct ucred));
			memcpy(*creds, CMSG_DATA(cmsg), sizeof(struct ucred));
			break;
		}
	}

	free(msg.msg_control);
	return n;

abort:
	free(msg.msg_control);
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
 */
int nl_recvmsgs(struct nl_handle *handle, struct nl_cb *cb)
{
	int n, err = 0;
	unsigned char *buf = NULL;
	struct nlmsghdr *hdr;
	struct sockaddr_nl nla = {0};
	struct nl_msg *msg = NULL;
	struct ucred *creds = NULL;

continue_reading:
	if (cb->cb_recv_ow)
		n = cb->cb_recv_ow(handle, &nla, &buf, &creds);
	else
		n = nl_recv(handle, &nla, &buf, &creds);

	if (n <= 0)
		return n;

	NL_DBG(3, "recvmsgs(%p): Read %d bytes\n", handle, n);

	hdr = (struct nlmsghdr *) buf;
	while (nlmsg_ok(hdr, n)) {
		NL_DBG(3, "recgmsgs(%p): Processing valid message...\n",
		       handle);

		nlmsg_free(msg);
		msg = nlmsg_convert(hdr);
		if (!msg) {
			err = nl_errno(ENOMEM);
			goto out;
		}

		nlmsg_set_proto(msg, handle->h_proto);
		nlmsg_set_src(msg, &nla);
		if (creds)
			nlmsg_set_creds(msg, creds);

		/* Raw callback is the first, it gives the most control
		 * to the user and he can do his very own parsing. */
		if (cb->cb_set[NL_CB_MSG_IN]) {
			err = nl_cb_call(cb, NL_CB_MSG_IN, msg);
			if (err == NL_SKIP)
				goto skip;
			else if (err == NL_EXIT || err < 0)
				goto out;
		}

		/* Sequence number checking. The check may be done by
		 * the user, otherwise a very simple check is applied
		 * enforcing strict ordering */
		if (cb->cb_set[NL_CB_SEQ_CHECK]) {
			err = nl_cb_call(cb, NL_CB_SEQ_CHECK, msg);
			if (err == NL_SKIP)
				goto skip;
			else if (err == NL_EXIT || err < 0)
				goto out;
		} else if (hdr->nlmsg_seq != handle->h_seq_expect) {
			if (cb->cb_set[NL_CB_INVALID]) {
				err = nl_cb_call(cb, NL_CB_INVALID, msg);
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
			NL_DBG(3, "recvmsgs(%p): Increased expected " \
			       "sequence number to %d\n",
			       handle, handle->h_seq_expect);
		}
	
		/* Other side wishes to see an ack for this message */
		if (hdr->nlmsg_flags & NLM_F_ACK) {
			if (cb->cb_set[NL_CB_SEND_ACK]) {
				err = nl_cb_call(cb, NL_CB_SEND_ACK, msg);
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
			if (cb->cb_set[NL_CB_FINISH]) {
				err = nl_cb_call(cb, NL_CB_FINISH, msg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			}
			err = 0;
			goto out;
		}

		/* Message to be ignored, the default action is to
		 * skip this message if no callback is specified. The
		 * user may overrule this action by returning
		 * NL_PROCEED. */
		else if (hdr->nlmsg_type == NLMSG_NOOP) {
			if (cb->cb_set[NL_CB_SKIPPED]) {
				err = nl_cb_call(cb, NL_CB_SKIPPED, msg);
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
			if (cb->cb_set[NL_CB_OVERRUN]) {
				err = nl_cb_call(cb, NL_CB_OVERRUN, msg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			} else
				goto out;
		}

		/* Message carries a nlmsgerr */
		else if (hdr->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *e = nlmsg_data(hdr);

			if (hdr->nlmsg_len < nlmsg_msg_size(sizeof(*e))) {
				/* Truncated error message, the default action
				 * is to stop parsing. The user may overrule
				 * this action by returning NL_SKIP or
				 * NL_PROCEED (dangerous) */
				if (cb->cb_set[NL_CB_INVALID]) {
					err = nl_cb_call(cb, NL_CB_INVALID,
							 msg);
					if (err == NL_SKIP)
						goto skip;
					else if (err == NL_EXIT || err < 0)
						goto out;
				} else
					goto out;
			} else if (e->error) {
				/* Error message reported back from kernel. */
				if (cb->cb_err) {
					err = cb->cb_err(&nla, e,
							   cb->cb_err_arg);
					if (err == NL_SKIP)
						goto skip;
					else if (err == NL_EXIT || err < 0) {
						nl_error(-e->error,
							 "Netlink Error");
						err = e->error;
						goto out;
					}
				} else {
					nl_error(-e->error, "Netlink Error");
					err = e->error;
					goto out;
				}
			} else if (cb->cb_set[NL_CB_ACK]) {
				/* ACK */
				err = nl_cb_call(cb, NL_CB_ACK, msg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			}
		} else {
			/* Valid message (not checking for MULTIPART bit to
			 * get along with broken kernels. NL_SKIP has no
			 * effect on this.  */
			if (cb->cb_set[NL_CB_VALID]) {
				err = nl_cb_call(cb, NL_CB_VALID, msg);
				if (err == NL_SKIP)
					goto skip;
				else if (err == NL_EXIT || err < 0)
					goto out;
			}
		}
skip:
		hdr = nlmsg_next(hdr, &n);
	}
	
	nlmsg_free(msg);
	free(buf);
	free(creds);
	buf = NULL;
	msg = NULL;
	creds = NULL;

	/* Multipart message not yet complete, continue reading */
	goto continue_reading;

out:
	nlmsg_free(msg);
	free(buf);
	free(creds);

	return err;
}

/**
 * Receive a set of message from a netlink socket using handlers in nl_handle.
 * @arg handle		netlink handle
 *
 * Calls nl_recvmsgs() with the handlers configured in the netlink handle.
 */
int nl_recvmsgs_def(struct nl_handle *handle)
{
	if (handle->h_cb->cb_recvmsgs_ow)
		return handle->h_cb->cb_recvmsgs_ow(handle, handle->h_cb);
	else
		return nl_recvmsgs(handle, handle->h_cb);
}

static int ack_wait_handler(struct nl_msg *msg, void *arg)
{
	return NL_EXIT;
}

/**
 * Wait for ACK.
 * @arg handle		netlink handle
 * @pre The netlink socket must be in blocking state.
 *
 * Waits until an ACK is received for the latest not yet acknowledged
 * netlink message.
 */
int nl_wait_for_ack(struct nl_handle *handle)
{
	int err;
	struct nl_cb *cb = nl_cb_clone(nl_handle_get_cb(handle));

	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_wait_handler, NULL);

	err = nl_recvmsgs(handle, cb);
	nl_cb_destroy(cb);

	return err;
}

/** @} */

/**
 * @name Netlink Family Translations
 * @{
 */

static struct trans_tbl nlfamilies[] = {
	__ADD(NETLINK_ROUTE,route)
	__ADD(NETLINK_W1,w1)
	__ADD(NETLINK_USERSOCK,usersock)
	__ADD(NETLINK_FIREWALL,firewall)
	__ADD(NETLINK_INET_DIAG,inetdiag)
	__ADD(NETLINK_NFLOG,nflog)
	__ADD(NETLINK_XFRM,xfrm)
	__ADD(NETLINK_SELINUX,selinux)
	__ADD(NETLINK_ISCSI,iscsi)
	__ADD(NETLINK_AUDIT,audit)
	__ADD(NETLINK_FIB_LOOKUP,fib_lookup)
	__ADD(NETLINK_CONNECTOR,connector)
	__ADD(NETLINK_NETFILTER,netfilter)
	__ADD(NETLINK_IP6_FW,ip6_fw)
	__ADD(NETLINK_DNRTMSG,dnrtmsg)
	__ADD(NETLINK_KOBJECT_UEVENT,kobject_uevent)
	__ADD(NETLINK_GENERIC,generic)
};

/**
 * Convert netlink family to character string.
 * @arg family		Netlink family.
 * @arg buf		Destination buffer.
 * @arg size		Size of destination buffer.
 *
 * Converts a netlink family to a character string and stores it in
 * the specified destination buffer.
 *
 * @return The destination buffer or the family encoded in hexidecimal
 *         form if no match was found.
 */
char * nl_nlfamily2str(int family, char *buf, size_t size)
{
	return __type2str(family, buf, size, nlfamilies,
			  ARRAY_SIZE(nlfamilies));
}

/**
 * Convert character string to netlink family.
 * @arg name		Name of netlink family.
 *
 * Converts the provided character string specifying a netlink
 * family to the corresponding numeric value.
 *
 * @return Numeric netlink family or a negative value if no match was found.
 */
int nl_str2nlfamily(const char *name)
{
	return __str2type(name, nlfamilies, ARRAY_SIZE(nlfamilies));
}

/** @} */
/** @} */
