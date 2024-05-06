#include "usr/nl/bib.h"

#include <errno.h>
#include <netlink/genl/genl.h>
#include "usr/nl/attribute.h"
#include "usr/nl/common.h"

struct foreach_args {
	joolnl_bib_foreach_cb cb;
	void *args;
	bool done;
	struct bib_entry last;
};

static struct jool_result handle_foreach_response(struct nl_msg *response,
		void *arg)
{
	struct foreach_args *args = arg;
	struct nlattr *attr;
	int rem;
	struct bib_entry entry;
	struct jool_result result;

	result = joolnl_init_foreach_list(response, "bib", &args->done);
	if (result.error)
		return result;

	foreach_entry(attr, genlmsg_hdr(nlmsg_hdr(response)), rem) {
		result = nla_get_bib(attr, &entry);
		if (result.error)
			return result;

		result = args->cb(&entry, args->args);
		if (result.error)
			return result;

		memcpy(&args->last, &entry, sizeof(entry));
	}

	return result_success();
}

struct jool_result joolnl_bib_foreach(struct joolnl_socket *sk, char const *iname,
	l4_protocol proto, joolnl_bib_foreach_cb cb, void *_args)
{
	struct nl_msg *msg;
	struct foreach_args args;
	struct jool_result result;
	bool first_request;

	args.cb = cb;
	args.args = _args;
	args.done = true;
	memset(&args.last, 0, sizeof(args.last));
	first_request = true;

	do {
		result = joolnl_alloc_msg(sk, iname, JNLOP_BIB_FOREACH, 0, &msg);
		if (result.error)
			return result;

		if (first_request) {
			if (nla_put_u8(msg, JNLAR_PROTO, proto) < 0)
				goto cancel;
			first_request = false;

		} else if (nla_put_bib(msg, JNLAR_OFFSET, &args.last) < 0) {
			goto cancel;
		}

		result = joolnl_request(sk, msg, handle_foreach_response, &args);
		if (result.error)
			return result;
	} while (!args.done);

	return result_success();

cancel:
	nlmsg_free(msg);
	return joolnl_err_msgsize();
}

static struct jool_result __update(struct joolnl_socket *sk, char const *iname,
		enum joolnl_operation op,
		struct ipv6_transport_addr const *a6,
		struct ipv4_transport_addr const *a4,
		l4_protocol proto)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, op, 0, &msg);
	if (result.error)
		return result;

	if (nla_put_bib_attrs(msg, JNLAR_OPERAND, a6, a4, proto, true) < 0) {
		nlmsg_free(msg);
		return joolnl_err_msgsize();
	}

	return joolnl_request(sk, msg, NULL, NULL);
}


struct jool_result joolnl_bib_add(struct joolnl_socket *sk,
		char const *iname,
		struct ipv6_transport_addr const *a6,
		struct ipv4_transport_addr const *a4,
		l4_protocol proto)
{
	return __update(sk, iname, JNLOP_BIB_ADD, a6, a4, proto);
}

struct jool_result joolnl_bib_rm(struct joolnl_socket *sk,
		char const *iname,
		struct ipv6_transport_addr const *a6,
		struct ipv4_transport_addr const *a4,
		l4_protocol proto)
{
	return __update(sk, iname, JNLOP_BIB_RM, a6, a4, proto);
}
