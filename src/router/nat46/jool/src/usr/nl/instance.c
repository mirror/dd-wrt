#include "usr/nl/instance.h"

#include <errno.h>
#include <netlink/genl/genl.h>
#include "usr/nl/attribute.h"
#include "usr/nl/common.h"

struct foreach_args {
	instance_foreach_cb cb;
	void *args;
	bool done;
	struct instance_entry_usr last;
};

static struct jool_result entry2attr(struct instance_entry_usr *entry,
		int attrtype, struct nl_msg *msg)
{
	struct nlattr *root;

	root = jnla_nest_start(msg, attrtype);
	if (!root)
		goto nla_put_failure;

	NLA_PUT_U32(msg, JNLAIE_NS, entry->ns);
	NLA_PUT_U8(msg, JNLAIE_XF, entry->xf);
	NLA_PUT_STRING(msg, JNLAIE_INAME, entry->iname);

	nla_nest_end(msg, root);
	return result_success();

nla_put_failure:
	return joolnl_err_msgsize();
}

static struct jool_result attr2entry(struct nlattr *root,
		struct instance_entry_usr *entry)
{
	struct nlattr *attrs[JNLAIE_COUNT];
	struct jool_result result;

	result = jnla_parse_nested(attrs, JNLAIE_MAX, root,
			joolnl_instance_entry_policy);
	if (result.error)
		return result;

	entry->ns = nla_get_u32(attrs[JNLAIE_NS]);
	entry->xf = nla_get_u8(attrs[JNLAIE_XF]);
	strcpy(entry->iname, nla_get_string(attrs[JNLAIE_INAME]));
	return result_success();
}

static struct jool_result handle_foreach_response(struct nl_msg *response,
		void *arg)
{
	struct foreach_args *args = arg;
	struct nlattr *attr;
	int rem;
	struct instance_entry_usr entry;
	struct jool_result result;

	result = joolnl_init_foreach_list(response, "instance", &args->done);
	if (result.error)
		return result;

	foreach_entry(attr, genlmsg_hdr(nlmsg_hdr(response)), rem) {
		result = attr2entry(attr, &entry);
		if (result.error)
			return result;

		result = args->cb(&entry, args->args);
		if (result.error)
			return result;

		memcpy(&args->last, &entry, sizeof(entry));
	}

	return result_success();
}

struct jool_result joolnl_instance_foreach(struct joolnl_socket *sk,
		instance_foreach_cb cb, void *_args)
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
		result = joolnl_alloc_msg(sk, NULL, JNLOP_INSTANCE_FOREACH, 0, &msg);
		if (result.error)
			return result;

		if (first_request) {
			first_request = false;
		} else {
			result = entry2attr(&args.last, JNLAR_OFFSET, msg);
			if (result.error) {
				nlmsg_free(msg);
				return result;
			}
		}

		result = joolnl_request(sk, msg, handle_foreach_response, &args);
		if (result.error)
			return result;
	} while (!args.done);

	return result_success();
}

static struct jool_result jool_hello_cb(struct nl_msg *response, void *status)
{
	static struct nla_policy status_policy[JNLAIS_COUNT] = {
		[JNLAIS_STATUS] = { .type = NLA_U8 },
	};
	struct nlattr *attrs[JNLAIS_COUNT];
	struct jool_result result;

	result = jnla_parse_msg(response, attrs, JNLAIS_MAX, status_policy, true);
	if (result.error)
		return result;

	*((enum instance_hello_status *)status) = nla_get_u8(attrs[JNLAIS_STATUS]);
	return result_success();
}

/**
 * If the instance exists, @result will be zero. If the instance does not exist,
 * @result will be 1.
 */
struct jool_result joolnl_instance_hello(struct joolnl_socket *sk,
		char const *iname, enum instance_hello_status *status)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, JNLOP_INSTANCE_HELLO, 0, &msg);
	if (result.error)
		return result;

	return joolnl_request(sk, msg, jool_hello_cb, status);
}

struct jool_result joolnl_instance_add(struct joolnl_socket *sk,
		xlator_framework xf, char const *iname,
		struct ipv6_prefix const *pool6)
{
	struct nl_msg *msg;
	struct nlattr *root;
	struct jool_result result;

	result.error = xf_validate(xf);
	if (result.error)
		return result_from_error(result.error, XF_VALIDATE_ERRMSG);

	result = joolnl_alloc_msg(sk, iname, JNLOP_INSTANCE_ADD, 0, &msg);
	if (result.error)
		return result;

	root = jnla_nest_start(msg, JNLAR_OPERAND);
	if (!root)
		goto nla_put_failure;

	NLA_PUT_U8(msg, JNLAIA_XF, xf);
	if (nla_put_prefix6(msg, JNLAIA_POOL6, pool6) < 0)
		goto nla_put_failure;

	nla_nest_end(msg, root);
	return joolnl_request(sk, msg, NULL, NULL);

nla_put_failure:
	nlmsg_free(msg);
	return joolnl_err_msgsize();
}

struct jool_result joolnl_instance_rm(struct joolnl_socket *sk,
		char const *iname)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, JNLOP_INSTANCE_RM, 0, &msg);
	if (result.error)
		return result;

	return joolnl_request(sk, msg, NULL, NULL);
}

struct jool_result joolnl_instance_flush(struct joolnl_socket *sk)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, NULL, JNLOP_INSTANCE_FLUSH, 0, &msg);
	if (result.error)
		return result;

	return joolnl_request(sk, msg, NULL, NULL);
}
