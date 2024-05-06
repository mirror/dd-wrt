#include "mod/common/nl/session.h"

#include "mod/common/log.h"
#include "mod/common/xlator.h"
#include "mod/common/nl/attribute.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/nl/nl_core.h"
#include "mod/common/db/bib/db.h"

static int parse_offset(struct nlattr *root, struct session_foreach_offset *entry)
{
	struct nlattr *attrs[JNLASE_COUNT];
	int error;

	error = jnla_parse_nested(attrs, JNLASE_MAX, root, joolnl_session_entry_policy, "session entry");
	if (error)
		return error;

	memset(entry, 0, sizeof(*entry));

	if (attrs[JNLASE_SRC4]) {
		error = jnla_get_taddr4(attrs[JNLASE_SRC4], "IPv4 source address", &entry->offset.src);
		if (error)
			return error;
	}
	if (attrs[JNLASE_DST4]) {
		error = jnla_get_taddr4(attrs[JNLASE_DST4], "IPv4 destination address", &entry->offset.dst);
		if (error)
			return error;
	}

	entry->include_offset = false;
	return 0;
}

static int serialize_session_entry(struct session_entry const *entry, void *arg)
{
	return jnla_put_session(arg, JNLAL_ENTRY, entry) ? 1 : 0;
}

int handle_session_foreach(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct jool_response response;
	struct session_foreach_offset offset, *offset_ptr;
	l4_protocol proto;
	int error;

	error = request_handle_start(info, XT_NAT64, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Sending session to userspace.");

	error = jresponse_init(&response, info);
	if (error)
		goto revert_start;

	if (!info->attrs[JNLAR_PROTO]) {
		log_err("The request is missing a transport protocol.");
		error = -EINVAL;
		goto revert_response;
	} else {
		proto = nla_get_u8(info->attrs[JNLAR_PROTO]);
	}

	if (!info->attrs[JNLAR_OFFSET]) {
		offset_ptr = NULL;
	} else {
		error = parse_offset(info->attrs[JNLAR_OFFSET], &offset);
		if (error)
			goto revert_response;
		offset_ptr = &offset;
		__log_debug(&jool, "Offset: [%pI4/%u %pI4/%u]",
				&offset.offset.src.l3, offset.offset.src.l4,
				&offset.offset.dst.l3, offset.offset.dst.l4);
	}

	error = bib_foreach_session(&jool, proto, serialize_session_entry,
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
