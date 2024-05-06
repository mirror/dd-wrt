#include "usr/nl/pool4.h"

#include <errno.h>
#include <netlink/genl/genl.h>
#include "usr/nl/attribute.h"
#include "usr/nl/common.h"

struct foreach_args {
	joolnl_pool4_foreach_cb cb;
	void *args;
	bool done;
	struct pool4_entry last;
};

static struct jool_result handle_foreach_response(struct nl_msg *response,
		void *arg)
{
	struct foreach_args *args = arg;
	struct nlattr *attr;
	int rem;
	struct pool4_entry entry;
	struct jool_result result;

	result = joolnl_init_foreach_list(response, "pool4", &args->done);
	if (result.error)
		return result;

	foreach_entry(attr, genlmsg_hdr(nlmsg_hdr(response)), rem) {
		result = nla_get_pool4(attr, &entry);
		if (result.error)
			return result;

		result = args->cb(&entry, args->args);
		if (result.error)
			return result;

		memcpy(&args->last, &entry, sizeof(entry));
	}

	return result_success();
}

struct jool_result joolnl_pool4_foreach(struct joolnl_socket *sk,
		char const *iname, l4_protocol proto,
		joolnl_pool4_foreach_cb cb, void *_args)
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
		result = joolnl_alloc_msg(sk, iname, JNLOP_POOL4_FOREACH, 0, &msg);
		if (result.error)
			return result;

		if (first_request) {
			if (nla_put_u8(msg, JNLAR_PROTO, proto) < 0)
				goto cancel;
			first_request = false;

		} else if (nla_put_pool4(msg, JNLAR_OFFSET, &args.last) < 0) {
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
		enum joolnl_operation operation, struct pool4_entry const *entry,
		bool quick)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, operation, quick ? JOOLNLHDR_FLAGS_QUICK : 0, &msg);
	if (result.error)
		return result;

	if (entry && nla_put_pool4(msg, JNLAR_OPERAND, entry) < 0) {
		nlmsg_free(msg);
		return joolnl_err_msgsize();
	}

	return joolnl_request(sk, msg, NULL, NULL);
}

struct jool_result joolnl_pool4_add(struct joolnl_socket *sk, char const *iname,
		struct pool4_entry const *entry)
{
	return __update(sk, iname, JNLOP_POOL4_ADD, entry, false);
}

struct jool_result joolnl_pool4_rm(struct joolnl_socket *sk, char const *iname,
		struct pool4_entry const *entry, bool quick)
{
	return __update(sk, iname, JNLOP_POOL4_RM, entry, quick);
}

struct jool_result joolnl_pool4_flush(struct joolnl_socket *sk,
		char const *iname, bool quick)
{
	return __update(sk, iname, JNLOP_POOL4_FLUSH, NULL, quick);
}
