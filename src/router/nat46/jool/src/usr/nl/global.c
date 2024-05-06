#include "usr/nl/global.h"

#include <errno.h>
#include <stddef.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>

#include "common/constants.h"
#include "usr/util/str_utils.h"
#include "usr/nl/attribute.h"
#include "usr/nl/common.h"
#include "usr/nl/json.h"

struct foreach_args {
	joolnl_global_foreach_cb cb;
	void *args;
	bool done;
	enum joolnl_attr_global last;
};

static struct jool_result handle_foreach_response(struct nl_msg *msg,
		void *_args)
{
	struct foreach_args *args = _args;
	struct genlmsghdr *ghdr;
	struct nlattr *head, *attr;
	int len, rem;
	struct joolnl_global_meta const *meta;
	struct jool_globals globals;
	void *slot;
	struct jool_result result;

	result = joolnl_init_foreach(msg, &args->done);
	if (result.error)
		return result;

	ghdr = nlmsg_data(nlmsg_hdr(msg));
	head = genlmsg_attrdata(ghdr, sizeof(struct joolnlhdr));
	len = genlmsg_attrlen(ghdr, sizeof(struct joolnlhdr));

	nla_for_each_attr(attr, head, len, rem) {
		args->last = nla_type(attr);
		meta = joolnl_global_id2meta(args->last);
		if (!meta) {
			fprintf(stderr, "Warning: The kernel module sent us unknown global id '%u'.\n",
					args->last);
			continue;
		}

		slot = joolnl_global_get(meta, &globals);

		result = joolnl_global_nl2raw(meta, attr, slot);
		if (result.error)
			return result;

		result = args->cb(meta, slot, args->args);
		if (result.error)
			return result;
	}

	return result_success();
}

struct jool_result joolnl_global_foreach(struct joolnl_socket *sk,
		char const *iname, joolnl_global_foreach_cb cb, void *_args)
{
	struct nl_msg *msg;
	struct foreach_args args;
	struct jool_result result;

	args.cb = cb;
	args.args = _args;
	args.done = true;
	args.last = 0;

	do {
		result = joolnl_alloc_msg(sk, iname, JNLOP_GLOBAL_FOREACH, 0, &msg);
		if (result.error)
			return result;

		if (args.last && (nla_put_u8(msg, JNLAR_OFFSET_U8, args.last) < 0)) {
			nlmsg_free(msg);
			return joolnl_err_msgsize();
		}

		result = joolnl_request(sk, msg, handle_foreach_response, &args);
		if (result.error)
			return result;
	} while (!args.done);

	return result_success();
}

struct jool_result joolnl_global_update(struct joolnl_socket *sk,
		char const *iname, struct joolnl_global_meta const *meta,
		char const *value, bool force)
{
	struct nl_msg *msg;
	struct nlattr *root;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, JNLOP_GLOBAL_UPDATE,
			force ? JOOLNLHDR_FLAGS_FORCE : 0, &msg);
	if (result.error)
		return result;

	root = jnla_nest_start(msg, JNLAR_GLOBALS);
	if (!root)
		return joolnl_err_msgsize();

	result = joolnl_global_str2nl(meta, value, msg);
	if (result.error) {
		nlmsg_free(msg);
		return result;
	}

	nla_nest_end(msg, root);
	return joolnl_request(sk, msg, NULL, NULL);
}
