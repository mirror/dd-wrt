/*
 * handlers.c           default netlink message handlers
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

/**
 * @ingroup nl
 * @defgroup cb Callbacks
 * 
 * Customization via handlers/callbacks.
 * 
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>

/**
 * @name General
 * @{
 * @anchor Handlers
 */

/**
 * Set all netlink handle handlers to a set of handlers with common suffix.
 * @arg handle		netlink handle
 * @arg suffix		common suffix of all handlers in the set
 *
 * Macro to set all handlers of a netlink handle to a set of handlers
 * with a common suffix, i.e. your handlers must be named like
 * \c nl_valid_handler_mysuffix, \c nl_finish_handler_mysuffix, etc.
 */
#define SET_HANDLERS(handle, suffix)				\
	handle->h_cb.cb_valid   = nl_valid_handler_##suffix;	\
	handle->h_cb.cb_finish  = nl_finish_handler_##suffix;	\
	handle->h_cb.cb_invalid = nl_invalid_handler_##suffix;	\
	handle->h_cb.cb_msg_in  = nl_msg_in_handler_##suffix;	\
	handle->h_cb.cb_overrun = nl_overrun_handler_##suffix;	\
	handle->h_cb.cb_skipped = nl_skipped_handler_##suffix;	\
	handle->h_cb.cb_ack     = nl_ack_handler_##suffix;	\
	handle->h_cb.cb_error   = nl_error_handler_##suffix;	\
	handle->h_cb.cb_msg_out = nl_msg_out_handler_##suffix;

/**
 * Use default handlers
 * @arg handle		netlink handle
 *
 * Sets all callback handlers in \c handle to the default handlers.
 */
void nl_use_default_handlers(struct nl_handle *handle)
{
	SET_HANDLERS(handle, default);
}

static void set_all_cb_args(struct nl_handle *h, void *data)
{
	h->h_cb.cb_valid_arg = data;
	h->h_cb.cb_finish_arg = data;
	h->h_cb.cb_invalid_arg = data;
	h->h_cb.cb_msg_in_arg = data;
	h->h_cb.cb_overrun_arg = data;
	h->h_cb.cb_skipped_arg = data;
	h->h_cb.cb_ack_arg = data;
	h->h_cb.cb_error_arg = data;
	h->h_cb.cb_msg_out_arg = data;
}

/**
 * Use verbose handlers
 * @arg handle		netlink handle
 *
 * Sets all callback handlers in \c handle to the verbose default handlers.
 */
void nl_use_default_verbose_handlers(struct nl_handle *handle)
{
	SET_HANDLERS(handle, verbose);
	set_all_cb_args(handle, (void *) stderr);
}

/**
 * Use debug handlers
 * @arg handle		netlink handle
 *
 * Sets all callback handlers in \c handle to the debugging default handlers.
 */
void nl_use_default_debug_handlers(struct nl_handle *handle)
{
	SET_HANDLERS(handle, debug);
	set_all_cb_args(handle, (void *) stderr);
}

/** @} */


static void print_header_content(FILE *ofd, struct nlmsghdr *n)
{
	char flags[128];
	char type[32];
	
	fprintf(ofd, "type=%s length=%u flags=<%s> sequence-nr=%u pid=%u",
		nl_nlmsgtype2str_r(n->nlmsg_type, type, sizeof(type)),
		n->nlmsg_len, nl_nlmsg_flags2str_r(n->nlmsg_flags, flags,
		sizeof(flags)), n->nlmsg_seq, n->nlmsg_pid);
}

static void raw_dump_msg(FILE *ofd, struct nlmsghdr *n)
{
	int i, a, c;
	char ascii[21] = {0};

	for (i = 0, a = 0, c = 0; i < (n->nlmsg_len - sizeof(*n)); i++) {
		int v = *(uint8_t *) (NLMSG_DATA(n) + i);

		fprintf(ofd, "%02x ", v);
		ascii[a++] = isprint(v) ? v : '.';

		if (c == 19) {
			fprintf(ofd, "%s\n", ascii);
			a = c = 0;
			memset(ascii, 0, sizeof(ascii));
		} else
			c++;
	}

	if (c != 0) {
		for (i = 0; i < (20 - c); i++)
			fprintf(ofd, "   ");
		fprintf(ofd, "%s\n", ascii);
	}
}

/**
 * @name Default Handlers
 * @{
 */

/**
 * nl_valid_handler_default
 *
 * Default handler for valid messages
 */
int nl_valid_handler_default(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_PROCEED;
}

/**
 * nl_finish_handler_default
 *
 * Default handler for 'end of multipart' messages
 */
int nl_finish_handler_default(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_EXIT;
}

/**
 * nl_invalid_handler_default
 *
 * Default handler for invalid/malformed messages.
 */
int nl_invalid_handler_default(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_EXIT;
}

/**
 * nl_msg_in_handler_default
 *
 * Default handler for incoming raw message callback (every message)
 */
int nl_msg_in_handler_default(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_PROCEED;
}

/**
 * nl_msg_out_handler_default
 *
 * Default handler for incoming raw message callback (every message)
 */
int nl_msg_out_handler_default(struct nlmsghdr *n, void *arg)
{
	return NL_PROCEED;
}

/**
 * nl_overrun_handler_default
 *
 * Default handler for overrun messages
 */
int nl_overrun_handler_default(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_EXIT;
}

/**
 * nl_skipped_handler_default
 *
 * Default handler for skipped messages.
 */
int nl_skipped_handler_default(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_SKIP;
}

/**
 * nl_ack_handler_default
 *
 * Default handler for ack requests.
 */
int nl_ack_handler_default(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_PROCEED;
}

/**
 * nl_error_handler_default
 *
 * Default handler for error messages.
 */
int nl_error_handler_default(struct sockaddr_nl *who, struct nlmsgerr *e, void *arg)
{
	return NL_EXIT;
}

/** @} */

/**
 * @name Verbose Handlers
 * @{
 */

/**
 * nl_valid_handler_verbose
 *
 * Verbose default handler for valid messages.
 */
int nl_valid_handler_verbose(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *ofd = arg;

	fprintf(ofd, "-- Warning: unhandled valid message: ");
	print_header_content(ofd, n);
	fprintf(ofd, "\n");

	return NL_PROCEED;
}

/**
 * nl_finish_handler_verbose
 *
 * Verbose default handler for 'end of multipart' messages
 */
int nl_finish_handler_verbose(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_EXIT;
}

/**
 * nl_msg_in_handler_verbose
 *
 * Verbose default handler for incoming raw message callback (every message)
 */
int nl_msg_in_handler_verbose(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_PROCEED;
}

/**
 * nl_invalid_handler_verbose
 *
 * Verbose default handler for invalid/malformed messages.
 */
int nl_invalid_handler_verbose(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *f = (FILE *) arg;
	fprintf(f, "-- Error: Invalid message: ");
	print_header_content(f, n);
	fprintf(f, "\n");

	return NL_EXIT;
}

/**
 * nl_msg_out_handler_verbose
 *
 * Verbose default handler for incoming raw message callback (every message)
 */
int nl_msg_out_handler_verbose(struct nlmsghdr *n, void *arg)
{
	return NL_PROCEED;
}

/**
 * nl_overrun_handler_verbose
 *
 * Verbose default handler for overrun messages
 */
int nl_overrun_handler_verbose(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *ofd = arg;
	fprintf(ofd, "-- Error: Netlink Overrun: ");
	print_header_content(ofd, n);
	fprintf(ofd, "\n");
	
	return NL_EXIT;
}

/**
 * nl_ack_handler_verbose
 *
 * Verbose default handler for ack requests.
 */
int nl_ack_handler_verbose(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_PROCEED;
}

/**
 * nl_skipped_handler_verbose
 *
 * Verbose default handler for skipped messages.
 */
int nl_skipped_handler_verbose(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return NL_SKIP;
}

/**
 * nl_error_handler_verbose
 *
 * Verbose default handler for error messages.
 */
int nl_error_handler_verbose(struct sockaddr_nl *who, struct nlmsgerr *e, void *arg)
{
	FILE *ofd = arg;

	fprintf(ofd, "-- Error received: %s\n", strerror(-e->error));
	fprintf(ofd, "-- Original message: ");
	print_header_content(ofd, &e->msg);
	fprintf(ofd, "\n");
	return NL_EXIT;
}

/** @} */

/**
 * @name Debugging Handlers
 * @{
 */

/**
 * nl_valid_handler_debug
 *
 * Debugging default handler for valid messages.
 */
int nl_valid_handler_debug(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *ofd = arg;

	fprintf(ofd, "-- Debug: Valid message: ");
	print_header_content(ofd, n);
	fprintf(ofd, "\n");

	return NL_PROCEED;
}

/**
 * nl_finish_handler_debug
 *
 * Debugging default handler for 'end of multipart' messages
 */
int nl_finish_handler_debug(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *ofd = arg;
	fprintf(ofd, "-- Debug: End of multipart message block: ");
	print_header_content(ofd, n);
	fprintf(ofd, "\n");
	
	return NL_EXIT;
}

/**
 * nl_invalid_handler_debug
 *
 * Debugging default handler for invalid/malformed messages.
 */
int nl_invalid_handler_debug(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return nl_invalid_handler_verbose(who, n, arg);
}

/**
 * nl_msg_in_handler_debug
 *
 * Debugging default handler for incoming raw message callback (every message)
 */
int nl_msg_in_handler_debug(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *ofd = arg;

	fprintf(ofd, "-- Raw Ingress: ");
	print_header_content(ofd, n);
	fprintf(ofd, "\n");

	raw_dump_msg(ofd, n);
	
	return NL_PROCEED;
}

/**
 * nl_msg_out_handler_debug
 *
 * Debugging default handler for incoming raw message callback (every message)
 */
int nl_msg_out_handler_debug(struct nlmsghdr *n, void *arg)
{
	FILE *ofd = arg;

	fprintf(ofd, "-- Raw Egress: ");
	print_header_content(ofd, n);
	fprintf(ofd, "\n");

	raw_dump_msg(ofd, n);
	
	return NL_PROCEED;
}

/**
 * nl_overrun_handler_debug
 *
 * Debugging default handler for overrun messages
 */
int nl_overrun_handler_debug(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	return nl_overrun_handler_verbose(who, n, arg);
}

/**
 * nl_skipped_handler_debug
 *
 * Debugging default handler for skipped messages.
 */
int nl_skipped_handler_debug(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *ofd = arg;

	fprintf(ofd, "-- Debug: Skipped message: ");
	print_header_content(ofd, n);
	fprintf(ofd, "\n");
	
	return NL_SKIP;
}

/**
 * nl_ack_handler_debug
 *
 * Debugging default handler for ack requests.
 */
int nl_ack_handler_debug(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *ofd = arg;

	fprintf(ofd, "-- Debug: ACK: ");
	print_header_content(ofd, n);
	fprintf(ofd, "\n");
	
	return NL_PROCEED;
}

/**
 * nl_error_handler_debug
 *
 * Debugging default handler for error messages.
 */
int nl_error_handler_debug(struct sockaddr_nl *who, struct nlmsgerr *e, void *arg)
{
	return nl_error_handler_verbose(who, e, arg);
}

/** @} */

/** @} */
