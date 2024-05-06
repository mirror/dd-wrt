#include "usr/nl/eamt.h"

#include <errno.h>
#include <netlink/genl/genl.h>
#include "usr/nl/attribute.h"
#include "usr/nl/common.h"

struct foreach_args {
	joolnl_eamt_foreach_cb cb;
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
	struct eamt_entry entry;
	struct jool_result result;

	result = joolnl_init_foreach_list(response, "eam", &args->done);
	if (result.error)
		return result;

	foreach_entry(attr, genlmsg_hdr(nlmsg_hdr(response)), rem) {
		result = nla_get_eam(attr, &entry);
		if (result.error)
			return result;

		result = args->cb(&entry, args->args);
		if (result.error)
			return result;

		args->last = entry.prefix4;
	}

	return result_success();
}

struct jool_result joolnl_eamt_foreach(struct joolnl_socket *sk,
		char const *iname, joolnl_eamt_foreach_cb cb, void *_args)
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
		result = joolnl_alloc_msg(sk, iname, JNLOP_EAMT_FOREACH, 0, &msg);
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
		enum joolnl_operation operation,
		struct ipv6_prefix const *p6, struct ipv4_prefix const *p4,
		__u8 flags)
{
	struct nl_msg *msg;
	struct nlattr *root;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, operation, flags, &msg);
	if (result.error)
		return result;

	root = jnla_nest_start(msg, JNLAR_OPERAND);
	if (!root)
		goto nla_put_failure;

	if (nla_put_prefix6(msg, JNLAE_PREFIX6, p6) < 0)
		goto nla_put_failure;
	if (nla_put_prefix4(msg, JNLAE_PREFIX4, p4) < 0)
		goto nla_put_failure;

	nla_nest_end(msg, root);
	return joolnl_request(sk, msg, NULL, NULL);

nla_put_failure:
	nlmsg_free(msg);
	return joolnl_err_msgsize();
}

struct jool_result joolnl_eamt_add(struct joolnl_socket *sk, char const *iname,
		struct ipv6_prefix const *p6, struct ipv4_prefix const *p4,
		bool force)
{
	return __update(sk, iname, JNLOP_EAMT_ADD, p6, p4,
			force ? JOOLNLHDR_FLAGS_FORCE : 0);
}

struct jool_result joolnl_eamt_rm(struct joolnl_socket *sk, char const *iname,
		struct ipv6_prefix const *p6, struct ipv4_prefix const *p4)
{
	return __update(sk, iname, JNLOP_EAMT_RM, p6, p4, 0);
}

struct jool_result joolnl_eamt_flush(struct joolnl_socket *sk, char const *iname)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, JNLOP_EAMT_FLUSH, 0, &msg);
	if (result.error)
		return result;

	return joolnl_request(sk, msg, NULL, NULL);
}
