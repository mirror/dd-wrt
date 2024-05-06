#include "usr/nl/denylist4.h"

#include <errno.h>
#include <netlink/genl/genl.h>
#include "usr/nl/attribute.h"
#include "usr/nl/common.h"

struct foreach_args {
	joolnl_denylist4_foreach_cb cb;
	void *args;
	bool done;
	struct ipv4_prefix last;
};

static struct jool_result handle_foreach_response(struct nl_msg *response,
		void *arg)
{
	struct foreach_args *args = arg;
	struct nlattr *attr;
	int rem;
	struct ipv4_prefix entry;
	struct jool_result result;

	result = joolnl_init_foreach_list(response, "denylist4", &args->done);
	if (result.error)
		return result;

	foreach_entry(attr, genlmsg_hdr(nlmsg_hdr(response)), rem) {
		result = nla_get_prefix4(attr, &entry);
		if (result.error)
			return result;

		result = args->cb(&entry, args->args);
		if (result.error)
			return result;

		memcpy(&args->last, &entry, sizeof(entry));
	}

	return result_success();
}

struct jool_result joolnl_denylist4_foreach(struct joolnl_socket *sk,
		char const *iname, joolnl_denylist4_foreach_cb cb, void *_args)
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
		result = joolnl_alloc_msg(sk, iname, JNLOP_BL4_FOREACH, 0, &msg);
		if (result.error)
			return result;

		if (first_request) {
			first_request = false;

		} else if (nla_put_prefix4(msg, JNLAR_OFFSET, &args.last) < 0) {
			nlmsg_free(msg);
			return joolnl_err_msgsize();
		}

		result = joolnl_request(sk, msg, handle_foreach_response, &args);
		if (result.error)
			return result;
	} while (!args.done);

	return result_success();
}

static struct jool_result __update(struct joolnl_socket *sk, char const *iname,
		enum joolnl_operation operation, struct ipv4_prefix const *prefix,
		__u8 force)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, operation, force, &msg);
	if (result.error)
		return result;

	if (nla_put_prefix4(msg, JNLAR_OPERAND, prefix) < 0) {
		nlmsg_free(msg);
		return joolnl_err_msgsize();
	}

	return joolnl_request(sk, msg, NULL, NULL);
}

struct jool_result joolnl_denylist4_add(struct joolnl_socket *sk,
		char const *iname, struct ipv4_prefix const *prefix, bool force)
{
	return __update(sk, iname, JNLOP_BL4_ADD, prefix,
			force ? JOOLNLHDR_FLAGS_FORCE : 0);
}

struct jool_result joolnl_denylist4_rm(struct joolnl_socket *sk,
		char const *iname, struct ipv4_prefix const *prefix)
{
	return __update(sk, iname, JNLOP_BL4_RM, prefix, 0);
}

struct jool_result joolnl_denylist4_flush(struct joolnl_socket *sk,
		char const *iname)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, JNLOP_BL4_FLUSH, 0, &msg);
	if (result.error)
		return result;

	return joolnl_request(sk, msg, NULL, NULL);
}
