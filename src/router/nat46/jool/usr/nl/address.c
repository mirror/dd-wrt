#include "usr/nl/address.h"

#include <errno.h>
#include "usr/nl/common.h"
#include "usr/nl/attribute.h"

static struct jool_result handle_method(struct nlattr *attrs[],
		struct address_translation_entry *out)
{
	if (attrs[JNLAAQ_PREFIX6052] && attrs[JNLAAQ_EAM]) {
		return result_from_error(
			-EINVAL,
			"The kernel's response has too many translation methods."
		);
	}

	if (attrs[JNLAAQ_PREFIX6052]) {
		out->method = AXM_RFC6052;
		return nla_get_prefix6(attrs[JNLAAQ_PREFIX6052], &out->prefix6052);
	}
	if (attrs[JNLAAQ_EAM]) {
		out->method = AXM_EAMT;
		return nla_get_eam(attrs[JNLAAQ_EAM], &out->eam);
	}

	return result_from_error(
		-EINVAL,
		"The kernel's response lacks the translation method."
	);
}

static struct jool_result query64_response_cb(struct nl_msg *response, void *args)
{
	static struct nla_policy query64_policy[JNLAAQ_COUNT] = {
		[JNLAAQ_ADDR4] = JOOLNL_ADDR4_POLICY,
		[JNLAAQ_PREFIX6052] = { .type = NLA_NESTED, },
		[JNLAAQ_EAM] = { .type = NLA_NESTED, },
	};
	struct nlattr *attrs[JNLAAQ_COUNT];
	struct jool_result result;
	struct result_addrxlat64 *out = args;

	result = jnla_parse_msg(response, attrs, JNLAAQ_MAX, query64_policy, false);
	if (result.error)
		return result;

	if (!attrs[JNLAAQ_ADDR4]) {
		return result_from_error(
			-ESRCH,
			"The kernel's response lacks the result."
		);
	}

	nla_get_addr4(attrs[JNLAAQ_ADDR4], &out->addr);
	return handle_method(attrs, &out->entry);
}

struct jool_result joolnl_address_query64(struct joolnl_socket *sk,
		char const *iname, struct in6_addr const *addr,
		struct result_addrxlat64 *out)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, JNLOP_ADDRESS_QUERY64, 0, &msg);
	if (result.error)
		return result;

	NLA_PUT(msg, JNLAR_ADDR_QUERY, sizeof(*addr), addr);

	return joolnl_request(sk, msg, query64_response_cb, out);

nla_put_failure:
	return result_from_error(
		-NLE_NOMEM,
		"Cannot build Netlink request: Packet is too small."
	);
}

static struct jool_result query46_response_cb(struct nl_msg *response, void *args)
{
	static struct nla_policy query46_policy[JNLAAQ_COUNT] = {
		[JNLAAQ_ADDR6] = JOOLNL_ADDR6_POLICY,
		[JNLAAQ_PREFIX6052] = { .type = NLA_NESTED, },
		[JNLAAQ_EAM] = { .type = NLA_NESTED, },
	};
	struct nlattr *attrs[JNLAAQ_COUNT];
	struct jool_result result;
	struct result_addrxlat46 *out = args;

	result = jnla_parse_msg(response, attrs, JNLAAQ_MAX, query46_policy, false);
	if (result.error)
		return result;

	if (!attrs[JNLAAQ_ADDR6]) {
		return result_from_error(
			-ESRCH,
			"The kernel's response lacks the result."
		);
	}

	nla_get_addr6(attrs[JNLAAQ_ADDR6], &out->addr);
	return handle_method(attrs, &out->entry);
}

struct jool_result joolnl_address_query46(struct joolnl_socket *sk,
		char const *iname, struct in_addr const *addr,
		struct result_addrxlat46 *out)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, JNLOP_ADDRESS_QUERY46, 0, &msg);
	if (result.error)
		return result;

	NLA_PUT(msg, JNLAR_ADDR_QUERY, sizeof(*addr), addr);

	return joolnl_request(sk, msg, query46_response_cb, out);

nla_put_failure:
	return joolnl_err_msgsize();
}
