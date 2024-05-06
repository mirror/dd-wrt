#include "mod/common/nl/bib.h"

#include "mod/common/log.h"
#include "mod/common/xlator.h"
#include "mod/common/nl/attribute.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/nl/nl_core.h"
#include "mod/common/db/pool4/db.h"
#include "mod/common/db/bib/db.h"

static int serialize_bib_entry(struct bib_entry const *entry, void *arg)
{
	return jnla_put_bib(arg, JNLAL_ENTRY, entry) ? 1 : 0;
}

int handle_bib_foreach(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct jool_response response;
	struct bib_entry offset, *offset_ptr;
	int error;

	error = request_handle_start(info, XT_NAT64, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Sending BIB to userspace.");

	error = jresponse_init(&response, info);
	if (error)
		goto revert_start;

	if (info->attrs[JNLAR_OFFSET]) {
		error = jnla_get_bib(info->attrs[JNLAR_OFFSET], "Iteration offset", &offset);
		if (error)
			goto revert_response;
		offset_ptr = &offset;
		__log_debug(&jool, "Offset: " BEPP, BEPA(offset_ptr));
	} else if (info->attrs[JNLAR_PROTO]) {
		offset.l4_proto = nla_get_u8(info->attrs[JNLAR_PROTO]);
		offset_ptr = NULL;
	} else {
		log_err("The request is missing a protocol.");
		error = -EINVAL;
		goto revert_response;
	}

	error = bib_foreach(jool.nat64.bib, offset.l4_proto, serialize_bib_entry,
			response.skb, offset_ptr ? &offset_ptr->addr4 : NULL);

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

int handle_bib_add(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct bib_entry new;
	int error;

	error = request_handle_start(info, XT_NAT64, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Adding BIB entry.");

	error = jnla_get_bib(info->attrs[JNLAR_OPERAND], "Operand", &new);
	if (error)
		goto revert_start;

	if (!pool4db_contains(jool.nat64.pool4, jool.ns, new.l4_proto, &new.addr4)) {
		log_err("Transport address '" TA4PP "' does not belong to pool4.\n"
				"Please add it there first.", TA4PA(new.addr4));
		error = -EINVAL;
		goto revert_start;
	}

	error = bib_add_static(&jool, &new);
revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}

int handle_bib_rm(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct nlattr *attrs[JNLAB_COUNT];
	struct bib_entry entry;
	int error;

	error = request_handle_start(info, XT_NAT64, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Removing BIB entry.");

	if (!info->attrs[JNLAR_OPERAND]) {
		log_err("The request lacks an operand attribute.");
		error = -EINVAL;
		goto revert_start;
	}

	error = jnla_parse_nested(attrs, JNLAB_MAX, info->attrs[JNLAR_OPERAND], joolnl_bib_entry_policy, "BIB entry");
	if (error)
		goto revert_start;

	if (!attrs[JNLAB_SRC6] && !attrs[JNLAB_SRC4]) {
		error = -EINVAL;
		goto revert_start;
	}

	if (attrs[JNLAB_SRC6]) {
		error = jnla_get_taddr6(attrs[JNLAB_SRC6], "IPv6 transport address", &entry.addr6);
		if (error)
			goto revert_start;
	}
	if (attrs[JNLAB_SRC4]) {
		error = jnla_get_taddr4(attrs[JNLAB_SRC4], "IPv4 transport address", &entry.addr4);
		if (error)
			goto revert_start;
	}

	if (attrs[JNLAB_PROTO])
		entry.l4_proto = nla_get_u8(attrs[JNLAB_PROTO]);
	if (attrs[JNLAB_STATIC])
		entry.is_static = nla_get_u8(attrs[JNLAB_STATIC]);

	if (!attrs[JNLAB_SRC4])
		error = bib_find6(jool.nat64.bib, entry.l4_proto, &entry.addr6, &entry);
	else if (!attrs[JNLAB_SRC6])
		error = bib_find4(jool.nat64.bib, entry.l4_proto, &entry.addr4, &entry);
	if (error == -ESRCH)
		goto esrch;
	if (error)
		goto revert_start;

	error = bib_rm(&jool, &entry);
	if (error == -ESRCH)
		goto esrch;
	/* Fall through */

revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;

esrch:
	log_err("The entry wasn't in the database.");
	goto revert_start;
}
