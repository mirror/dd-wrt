#include "mod/common/nl/eam.h"

#include "common/types.h"
#include "mod/common/log.h"
#include "mod/common/xlator.h"
#include "mod/common/nl/attribute.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/nl/nl_core.h"
#include "mod/common/db/eam.h"

static int serialize_eam_entry(struct eamt_entry const *entry, void *arg)
{
	return jnla_put_eam(arg, JNLAL_ENTRY, entry) ? 1 : 0;
}

int handle_eamt_foreach(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct jool_response response;
	struct ipv4_prefix offset, *offset_ptr;
	int error;

	error = request_handle_start(info, XT_SIIT, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Sending EAMT to userspace.");

	error = jresponse_init(&response, info);
	if (error)
		goto revert_start;

	offset_ptr = NULL;
	if (info->attrs[JNLAR_OFFSET]) {
		error = jnla_get_prefix4(info->attrs[JNLAR_OFFSET],
				"Iteration offset", &offset);
		if (error)
			goto revert_response;
		offset_ptr = &offset;
		__log_debug(&jool, "Offset: [%pI4/%u]", &offset.addr,
				offset.len);
	}

	error = eamt_foreach(jool.siit.eamt, serialize_eam_entry, response.skb, offset_ptr);
	if (error < 0) {
		log_err("Offset not found.");
		jresponse_cleanup(&response);
		goto revert_response;
	}

	error = jresponse_send_array(&jool, &response, error);
	if (error)
		goto revert_response;

	request_handle_end(&jool);
	return 0;

revert_response:
	jresponse_cleanup(&response);
revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}

int handle_eamt_add(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct eamt_entry addend;
	int error;

	error = request_handle_start(info, XT_SIIT, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Adding EAM entry.");

	error = jnla_get_eam(info->attrs[JNLAR_OPERAND], "Operand", &addend);
	if (error)
		goto revert_start;

	error = eamt_add(jool.siit.eamt, &addend,
			get_jool_hdr(info)->flags & JOOLNLHDR_FLAGS_FORCE, true);
revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}

int handle_eamt_rm(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct nlattr *attrs[JNLAE_COUNT];
	struct ipv6_prefix prefix6, *prefix6_ptr;
	struct ipv4_prefix prefix4, *prefix4_ptr;
	int error;

	error = request_handle_start(info, XT_SIIT, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Removing EAM entry.");

	if (!info->attrs[JNLAR_OPERAND]) {
		log_err("The request is missing the 'Operand' attribute.");
		error = -EINVAL;
		goto revert_start;
	}
	error = jnla_parse_nested(attrs, JNLAE_MAX, info->attrs[JNLAR_OPERAND], eam_policy, "EAM");
	if (error)
		goto revert_start;

	if (!attrs[JNLAE_PREFIX6] && !attrs[JNLAE_PREFIX4]) {
		log_err("The request contains no prefixes.");
		error = -ENOENT;
		goto revert_start;
	}
	prefix6_ptr = NULL;
	if (attrs[JNLAE_PREFIX6]) {
		error = jnla_get_prefix6(attrs[JNLAE_PREFIX6], "IPv6 prefix", &prefix6);
		if (error)
			goto revert_start;
		prefix6_ptr = &prefix6;
	}
	prefix4_ptr = NULL;
	if (attrs[JNLAE_PREFIX4]) {
		error = jnla_get_prefix4(attrs[JNLAE_PREFIX4], "IPv4 prefix", &prefix4);
		if (error)
			goto revert_start;
		prefix4_ptr = &prefix4;
	}

	error = eamt_rm(jool.siit.eamt, prefix6_ptr, prefix4_ptr);
revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}

int handle_eamt_flush(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	int error;

	error = request_handle_start(info, XT_SIIT, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Flushing the EAMT.");

	eamt_flush(jool.siit.eamt);

	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}
