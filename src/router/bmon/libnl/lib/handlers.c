/*
 * lib/handlers.c	default netlink message handlers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup nl
 * @defgroup cb Callbacks/Customization
 * 
 * Customization via callbacks.
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/msg.h>
#include <netlink/handlers.h>

static void print_header_content(FILE *ofd, struct nlmsghdr *n)
{
	char flags[128];
	char type[32];
	
	fprintf(ofd, "type=%s length=%u flags=<%s> sequence-nr=%u pid=%u",
		nl_nlmsgtype2str(n->nlmsg_type, type, sizeof(type)),
		n->nlmsg_len, nl_nlmsg_flags2str(n->nlmsg_flags, flags,
		sizeof(flags)), n->nlmsg_seq, n->nlmsg_pid);
}

static inline void dump_hex(FILE *ofd, char *start, int len)
{
	int i, a, c, limit;
	char ascii[21] = {0};

	limit = 18;
	fprintf(ofd, "    ");

	for (i = 0, a = 0, c = 0; i < len; i++) {
		int v = *(uint8_t *) (start + i);

		fprintf(ofd, "%02x ", v);
		ascii[a++] = isprint(v) ? v : '.';

		if (c == limit-1) {
			fprintf(ofd, "%s\n", ascii);
			if (i < (len - 1))
				fprintf(ofd, "    ");
			a = c = 0;
			memset(ascii, 0, sizeof(ascii));
		} else
			c++;
	}

	if (c != 0) {
		for (i = 0; i < (limit - c); i++)
			fprintf(ofd, "   ");
		fprintf(ofd, "%s\n", ascii);
	}
}

static void print_hdr(FILE *ofd, struct nl_msg *msg)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct nl_cache_ops *ops;
	char buf[128];

	fprintf(ofd, "    .nlmsg_len = %d\n", nlh->nlmsg_len);

	ops = nl_cache_mngt_associate(nlmsg_get_proto(msg), nlh->nlmsg_type);

	fprintf(ofd, "    .nlmsg_type = %d <%s>\n", nlh->nlmsg_type,
		ops ? nl_cache_mngt_type2name(ops, nlh->nlmsg_type,
					      buf, sizeof(buf))
		: nl_nlmsgtype2str(nlh->nlmsg_type, buf, sizeof(buf)));
	fprintf(ofd, "    .nlmsg_flags = %d <%s>\n", nlh->nlmsg_flags,
		nl_nlmsg_flags2str(nlh->nlmsg_flags, buf, sizeof(buf)));
	fprintf(ofd, "    .nlmsg_seq = %d\n", nlh->nlmsg_seq);
	fprintf(ofd, "    .nlmsg_pid = %d\n", nlh->nlmsg_pid);

}

static void raw_dump_msg(FILE *ofd, struct nl_msg *msg)
{
	struct nlmsghdr *hdr = nlmsg_hdr(msg);
	
	fprintf(ofd, 
	"--------------------------   BEGIN NETLINK MESSAGE "
	"---------------------------\n");

	fprintf(ofd, "  [HEADER] %Zu octets\n", sizeof(struct nlmsghdr));
	print_hdr(ofd, msg);

	if (hdr->nlmsg_type == NLMSG_ERROR &&
	    hdr->nlmsg_len >= nlmsg_msg_size(sizeof(struct nlmsgerr))) {
		struct nl_msg *errmsg;
		struct nlmsgerr *err = nlmsg_data(hdr);

		fprintf(ofd, "  [ERRORMSG] %Zu octets\n", sizeof(*err));
		fprintf(ofd, "    .error = %d \"%s\"\n", err->error,
			strerror(-err->error));
		fprintf(ofd, "  [ORIGINAL MESSAGE] %Zu octets\n", sizeof(*hdr));

		errmsg = nlmsg_build(&err->msg);
		print_hdr(ofd, errmsg);
		nlmsg_free(msg);
	} else if (nlmsg_len(hdr) > 0) {
		struct nl_cache_ops *ops;
		int payloadlen = nlmsg_len(hdr);
		int attrlen = 0;

		ops = nl_cache_mngt_associate(nlmsg_get_proto(msg),
					      hdr->nlmsg_type);
		if (ops) {
			attrlen = nlmsg_attrlen(hdr, ops->co_hdrsize);
			payloadlen -= attrlen;
		}

		fprintf(ofd, "  [PAYLOAD] %d octets\n", payloadlen);
		dump_hex(ofd, nlmsg_data(hdr), payloadlen);

		if (attrlen) {
			int rem, padlen;
			struct nlattr *nla;
	
			nlmsg_for_each_attr(nla, hdr, ops->co_hdrsize, rem) {
				int alen = nla_len(nla);

				fprintf(ofd, "  [ATTR %02d] %d octets\n",
					nla->nla_type, alen);
				dump_hex(ofd, nla_data(nla), alen);

				padlen = nla_padlen(alen);
				if (padlen > 0) {
					fprintf(ofd, "  [PADDING] %d octets\n",
						padlen);
					dump_hex(ofd, nla_data(nla) + alen,
						 padlen);
				}
			}
		}
	}

	fprintf(ofd, 
	"---------------------------  END NETLINK MESSAGE   "
	"---------------------------\n");
}

static int nl_valid_handler_default(struct nl_msg *msg, void *arg)
{
	return NL_PROCEED;
}

static int nl_finish_handler_default(struct nl_msg *msg, void *arg)
{
	return NL_EXIT;
}

static int nl_invalid_handler_default(struct nl_msg *msg, void *arg)
{
	return NL_EXIT;
}

static int nl_msg_in_handler_default(struct nl_msg *msg, void *arg)
{
	return NL_PROCEED;
}

static int nl_msg_out_handler_default(struct nl_msg *msg, void *arg)
{
	return NL_PROCEED;
}

static int nl_overrun_handler_default(struct nl_msg *msg, void *arg)
{
	return NL_EXIT;
}

static int nl_skipped_handler_default(struct nl_msg *msg, void *arg)
{
	return NL_SKIP;
}

static int nl_ack_handler_default(struct nl_msg *msg, void *arg)
{
	return NL_EXIT;
}

static int nl_error_handler_default(struct sockaddr_nl *who,
				    struct nlmsgerr *e, void *arg)
{
	return NL_EXIT;
}

static int nl_valid_handler_verbose(struct nl_msg *msg, void *arg)
{
	FILE *ofd = arg ? arg : stdout;

	fprintf(ofd, "-- Warning: unhandled valid message: ");
	print_header_content(ofd, nlmsg_hdr(msg));
	fprintf(ofd, "\n");

	return NL_PROCEED;
}

static int nl_finish_handler_verbose(struct nl_msg *msg, void *arg)
{
	return NL_EXIT;
}

static int nl_msg_in_handler_verbose(struct nl_msg *msg, void *arg)
{
	return NL_PROCEED;
}

static int nl_invalid_handler_verbose(struct nl_msg *msg, void *arg)
{
	FILE *ofd = arg ? arg : stderr;

	fprintf(ofd, "-- Error: Invalid message: ");
	print_header_content(ofd, nlmsg_hdr(msg));
	fprintf(ofd, "\n");

	return NL_EXIT;
}

static int nl_msg_out_handler_verbose(struct nl_msg *msg, void *arg)
{
	return NL_PROCEED;
}

static int nl_overrun_handler_verbose(struct nl_msg *msg, void *arg)
{
	FILE *ofd = arg ? arg : stderr;

	fprintf(ofd, "-- Error: Netlink Overrun: ");
	print_header_content(ofd, nlmsg_hdr(msg));
	fprintf(ofd, "\n");
	
	return NL_EXIT;
}

static int nl_ack_handler_verbose(struct nl_msg *msg, void *arg)
{
	return NL_EXIT;
}

static int nl_skipped_handler_verbose(struct nl_msg *msg, void *arg)
{
	return NL_SKIP;
}

static int nl_error_handler_verbose(struct sockaddr_nl *who,
				    struct nlmsgerr *e, void *arg)
{
	FILE *ofd = arg ? arg : stderr;

	fprintf(ofd, "-- Error received: %s\n-- Original message: ",
		strerror(-e->error));
	print_header_content(ofd, &e->msg);
	fprintf(ofd, "\n");

	return NL_EXIT;
}

static int nl_valid_handler_debug(struct nl_msg *msg, void *arg)
{
	FILE *ofd = arg ? arg : stderr;

	fprintf(ofd, "-- Debug: Valid message: ");
	print_header_content(ofd, nlmsg_hdr(msg));
	fprintf(ofd, "\n");

	return NL_PROCEED;
}

static int nl_finish_handler_debug(struct nl_msg *msg, void *arg)
{
	FILE *ofd = arg ? arg : stderr;

	fprintf(ofd, "-- Debug: End of multipart message block: ");
	print_header_content(ofd, nlmsg_hdr(msg));
	fprintf(ofd, "\n");
	
	return NL_EXIT;
}

static int nl_invalid_handler_debug(struct nl_msg *msg, void *arg)
{
	return nl_invalid_handler_verbose(msg, arg);
}

static int nl_msg_in_handler_debug(struct nl_msg *msg, void *arg)
{
	FILE *ofd = arg ? arg : stderr;

	fprintf(ofd, "-- Debug: Received Message:\n");
	raw_dump_msg(ofd, msg);
	
	return NL_PROCEED;
}

static int nl_msg_out_handler_debug(struct nl_msg *msg, void *arg)
{
	FILE *ofd = arg ? arg : stderr;

	fprintf(ofd, "-- Debug: Sent Message:\n");
	raw_dump_msg(ofd, msg);

	return NL_PROCEED;
}

static int nl_overrun_handler_debug(struct nl_msg *msg, void *arg)
{
	return nl_overrun_handler_verbose(msg, arg);
}

static int nl_skipped_handler_debug(struct nl_msg *msg, void *arg)
{
	FILE *ofd = arg ? arg : stderr;

	fprintf(ofd, "-- Debug: Skipped message: ");
	print_header_content(ofd, nlmsg_hdr(msg));
	fprintf(ofd, "\n");

	return NL_SKIP;
}

static int nl_ack_handler_debug(struct nl_msg *msg, void *arg)
{
	FILE *ofd = arg ? arg : stderr;

	fprintf(ofd, "-- Debug: ACK: ");
	print_header_content(ofd, nlmsg_hdr(msg));
	fprintf(ofd, "\n");

	return NL_EXIT;
}

static int nl_error_handler_debug(struct sockaddr_nl *who,
				  struct nlmsgerr *e, void *arg)
{
	return nl_error_handler_verbose(who, e, arg);
}

static nl_recvmsg_msg_cb_t cb_def[NL_CB_TYPE_MAX+1][NL_CB_KIND_MAX+1] = {
	[NL_CB_VALID] = {
		[NL_CB_DEFAULT]	= nl_valid_handler_default,
		[NL_CB_VERBOSE]	= nl_valid_handler_verbose,
		[NL_CB_DEBUG]	= nl_valid_handler_debug,
	},
	[NL_CB_FINISH] = {
		[NL_CB_DEFAULT]	= nl_finish_handler_default,
		[NL_CB_VERBOSE]	= nl_finish_handler_verbose,
		[NL_CB_DEBUG]	= nl_finish_handler_debug,
	},
	[NL_CB_INVALID] = {
		[NL_CB_DEFAULT]	= nl_invalid_handler_default,
		[NL_CB_VERBOSE]	= nl_invalid_handler_verbose,
		[NL_CB_DEBUG]	= nl_invalid_handler_debug,
	},
	[NL_CB_MSG_IN] = {
		[NL_CB_DEFAULT]	= nl_msg_in_handler_default,
		[NL_CB_VERBOSE]	= nl_msg_in_handler_verbose,
		[NL_CB_DEBUG]	= nl_msg_in_handler_debug,
	},
	[NL_CB_MSG_OUT] = {
		[NL_CB_DEFAULT]	= nl_msg_out_handler_default,
		[NL_CB_VERBOSE]	= nl_msg_out_handler_verbose,
		[NL_CB_DEBUG]	= nl_msg_out_handler_debug,
	},
	[NL_CB_OVERRUN] = {
		[NL_CB_DEFAULT]	= nl_overrun_handler_default,
		[NL_CB_VERBOSE]	= nl_overrun_handler_verbose,
		[NL_CB_DEBUG]	= nl_overrun_handler_debug,
	},
	[NL_CB_SKIPPED] = {
		[NL_CB_DEFAULT]	= nl_skipped_handler_default,
		[NL_CB_VERBOSE]	= nl_skipped_handler_verbose,
		[NL_CB_DEBUG]	= nl_skipped_handler_debug,
	},
	[NL_CB_ACK] = {
		[NL_CB_DEFAULT]	= nl_ack_handler_default,
		[NL_CB_VERBOSE]	= nl_ack_handler_verbose,
		[NL_CB_DEBUG]	= nl_ack_handler_debug,
	},
};

static nl_recvmsg_err_cb_t cb_err_def[NL_CB_KIND_MAX+1] = {
	[NL_CB_DEFAULT]	= nl_error_handler_default,
	[NL_CB_VERBOSE]	= nl_error_handler_verbose,
	[NL_CB_DEBUG]	= nl_error_handler_debug,
};

/**
 * @name Callback Handle Management
 * @{
 */

/**
 * Allocate a new callback handle
 * @arg kind		callback kind to be used for initialization
 * @return Newly allocated callback handle or NULL
 */
struct nl_cb *nl_cb_new(enum nl_cb_kind kind)
{
	int i;
	struct nl_cb *cb;

	if (kind < 0 || kind > NL_CB_KIND_MAX)
		return NULL;

	cb = calloc(1, sizeof(*cb));
	if (!cb) {
		nl_errno(ENOMEM);
		return NULL;
	}

	for (i = 0; i <= NL_CB_TYPE_MAX; i++)
		nl_cb_set(cb, i, kind, NULL, NULL);

	nl_cb_err(cb, kind, NULL, NULL);

	return cb;
}

/**
 * Destroy a callback handle
 * @arg cb		callback handle
 */
void nl_cb_destroy(struct nl_cb *cb)
{
	free(cb);
}

/**
 * Clone an existing callback handle
 * @arg orig		original callback handle
 * @return Newly allocated callback handle being a duplicate of
 *         orig or NULL
 */
struct nl_cb *nl_cb_clone(struct nl_cb *orig)
{
	struct nl_cb *cb;
	
	cb = nl_cb_new(NL_CB_DEFAULT);
	if (!cb)
		return NULL;

	memcpy(cb, orig, sizeof(*orig));

	return cb;
}

/** @} */

/**
 * @name Callback Setup
 * @{
 */

/**
 * Set up a callback 
 * @arg cb		callback configuration
 * @arg type		which type callback to set
 * @arg kind		kind of callback
 * @arg func		callback function
 * @arg arg		argument to be passwd to callback function
 *
 * @return 0 on success or a negative error code
 */
int nl_cb_set(struct nl_cb *cb, enum nl_cb_type type, enum nl_cb_kind kind,
	      nl_recvmsg_msg_cb_t func, void *arg)
{
	if (type < 0 || type > NL_CB_TYPE_MAX)
		return nl_error(ERANGE, "Callback type out of range");

	if (kind < 0 || kind > NL_CB_KIND_MAX)
		return nl_error(ERANGE, "Callback kind out of range");

	if (kind == NL_CB_CUSTOM) {
		cb->cb_set[type] = func;
		cb->cb_args[type] = arg;
	} else {
		cb->cb_set[type] = cb_def[type][kind];
		cb->cb_args[type] = arg;
	}

	return 0;
}

/**
 * Set up a all callbacks
 * @arg cb		callback configuration
 * @arg kind		kind of callback
 * @arg func		callback function
 * @arg arg		argument to be passwd to callback function
 *
 * @return 0 on success or a negative error code
 */
int nl_cb_set_all(struct nl_cb *cb, enum nl_cb_kind kind,
		  nl_recvmsg_msg_cb_t func, void *arg)
{
	int i, err;

	for (i = 0; i <= NL_CB_TYPE_MAX; i++) {
		err = nl_cb_set(cb, i, kind, func, arg);
		if (err < 0)
			return err;
	}

	return 0;
}

/**
 * Set up an error callback
 * @arg cb		callback configuration
 * @arg kind		kind of callback
 * @arg func		callback function
 * @arg arg		argument to be passed to callback function
 */
int nl_cb_err(struct nl_cb *cb, enum nl_cb_kind kind,
	      nl_recvmsg_err_cb_t func, void *arg)
{
	if (kind < 0 || kind > NL_CB_KIND_MAX)
		return nl_error(ERANGE, "Callback kind out of range");

	if (kind == NL_CB_CUSTOM) {
		cb->cb_err = func;
		cb->cb_err_arg = arg;
	} else {
		cb->cb_err = cb_err_def[kind];
		cb->cb_err_arg = arg;
	}

	return 0;
}

/**
 * Overwrite internal calls to nl_recvmsgs()
 * @arg cb		callback configuration
 * @arg func		replacement callback for nl_recvmsgs()
 */
void nl_cb_overwrite_recvmsgs(struct nl_cb *cb,
			      int (*func)(struct nl_handle *, struct nl_cb *))
{
	cb->cb_recvmsgs_ow = func;
}

/**
 * Overwrite internal calls to nl_recv()
 * @arg cb		callback configuration
 * @arg func		replacement callback for nl_recv()
 */
void nl_cb_overwrite_recv(struct nl_cb *cb,
			  int (*func)(struct nl_handle *, struct sockaddr_nl *,
				      unsigned char **, struct ucred **))
{
	cb->cb_recv_ow = func;
}

/**
 * Overwrite internal calls to nl_send()
 * @arg cb		callback configuration
 * @arg func		replacement callback for nl_send()
 */
void nl_cb_overwrite_send(struct nl_cb *cb,
			  int (*func)(struct nl_handle *, struct nl_msg *))
{
	cb->cb_send_ow = func;
}

/** @} */

/** @} */
