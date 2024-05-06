#include "usr/nl/session.h"

#include <errno.h>
#include <netlink/genl/genl.h>
#include "usr/nl/attribute.h"
#include "usr/nl/common.h"

struct foreach_args {
	joolnl_session_foreach_cb cb;
	void *args;
	bool done;
	struct session_entry_usr last;
};

static struct jool_result handle_foreach_response(struct nl_msg *response,
		void *arg)
{
	struct foreach_args *args = arg;
	struct nlattr *attr;
	int rem;
	struct session_entry_usr entry;
	struct jool_result result;

	result = joolnl_init_foreach_list(response, "session", &args->done);
	if (result.error)
		return result;

	foreach_entry(attr, genlmsg_hdr(nlmsg_hdr(response)), rem) {
		result = nla_get_session(attr, &entry);
		if (result.error)
			return result;

		result = args->cb(&entry, args->args);
		if (result.error)
			return result;

		memcpy(&args->last, &entry, sizeof(entry));
	}

	return result_success();
}

struct jool_result joolnl_session_foreach(struct joolnl_socket *sk,
		char const *iname, l4_protocol proto,
		joolnl_session_foreach_cb cb, void *_args)
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
		result = joolnl_alloc_msg(sk, iname, JNLOP_SESSION_FOREACH, 0, &msg);
		if (result.error)
			return result;

		if (nla_put_u8(msg, JNLAR_PROTO, proto) < 0)
			goto cancel;

		if (first_request)
			first_request = false;
		else if (nla_put_session(msg, JNLAR_OFFSET, &args.last) < 0)
			goto cancel;

		result = joolnl_request(sk, msg, handle_foreach_response, &args);
		if (result.error)
			return result;
	} while (!args.done);

	return result_success();

cancel:
	nlmsg_free(msg);
	return joolnl_err_msgsize();
}

