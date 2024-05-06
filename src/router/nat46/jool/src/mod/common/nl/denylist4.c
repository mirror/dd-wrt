#include "mod/common/nl/denylist4.h"

#include "common/types.h"
#include "mod/common/log.h"
#include "mod/common/xlator.h"
#include "mod/common/nl/attribute.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/nl/nl_core.h"
#include "mod/common/db/denylist4.h"

static int serialize_bl4_entry(struct ipv4_prefix *prefix, void *arg)
{
	return jnla_put_prefix4(arg, JNLAL_ENTRY, prefix) ? 1 : 0;
}

int handle_denylist4_foreach(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct jool_response response;
	struct ipv4_prefix offset, *offset_ptr;
	int error;

	error = request_handle_start(info, XT_SIIT, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Sending the denylist4 to userspace.");

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

	error = denylist4_foreach(jool.siit.denylist4, serialize_bl4_entry,
			response.skb, offset_ptr);

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

int handle_denylist4_add(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct ipv4_prefix operand;
	int error;

	error = request_handle_start(info, XT_SIIT, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Adding Denylist4 entry.");

	error = jnla_get_prefix4(info->attrs[JNLAR_OPERAND], "Operand", &operand);
	if (error)
		goto revert_start;

	error = denylist4_add(jool.siit.denylist4, &operand,
			get_jool_hdr(info)->flags & JOOLNLHDR_FLAGS_FORCE);
	/* Fall through */

revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}

int handle_denylist4_rm(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct ipv4_prefix operand;
	int error;

	error = request_handle_start(info, XT_SIIT, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Removing Denylist4 entry.");

	error = jnla_get_prefix4(info->attrs[JNLAR_OPERAND], "Operand", &operand);
	if (error)
		goto revert_start;

	error = denylist4_rm(jool.siit.denylist4, &operand);
revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}

int handle_denylist4_flush(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	int error;

	error = request_handle_start(info, XT_SIIT, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Flushing the denylist4...");

	error = denylist4_flush(jool.siit.denylist4);
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}
