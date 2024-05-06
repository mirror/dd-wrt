#include "mod/common/nl/instance.h"

#include "common/types.h"
#include "mod/common/log.h"
#include "mod/common/xlator.h"
#include "mod/common/nl/attribute.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/nl/nl_core.h"

static int parse_instance(struct nlattr *root, struct instance_entry_usr *entry)
{
	struct nlattr *attrs[JNLAIE_COUNT];
	int error;

	error = jnla_parse_nested(attrs, JNLAIE_MAX, root,
			joolnl_instance_entry_policy, "instance");
	if (error)
		return error;

	error = jnla_get_u32(attrs[JNLAIE_NS], "namespace", &entry->ns);
	if (error)
		return error;
	error = jnla_get_u8(attrs[JNLAIE_XF], "framework", &entry->xf);
	if (error)
		return error;
	return jnla_get_str(attrs[JNLAIE_INAME], "instance name",
			INAME_MAX_SIZE, entry->iname);
}

static int serialize_instance(struct xlator *entry, void *arg)
{
	struct sk_buff *skb = arg;
	struct nlattr *root;
	int error;

	root = nla_nest_start(skb, JNLAL_ENTRY);
	if (!root)
		return 1;

	error = nla_put_u32(skb, JNLAIE_NS, ((__u64)entry->ns) & 0xFFFFFFFF);
	if (error)
		goto cancel;
	error = nla_put_u8(skb, JNLAIE_XF, xlator_flags2xf(entry->flags));
	if (error)
		goto cancel;
	error = nla_put_string(skb, JNLAIE_INAME, entry->iname);
	if (error)
		goto cancel;

	nla_nest_end(skb, root);
	return 0;

cancel:
	nla_nest_cancel(skb, root);
	return 1;
}

int handle_instance_foreach(struct sk_buff *skb, struct genl_info *info)
{
	struct instance_entry_usr offset, *offset_ptr;
	struct jool_response response;
	int error;

	LOG_DEBUG("Sending instance table to userspace.");

	error = request_handle_start(info, XT_ANY, NULL, true);
	if (error)
		goto fail;

	offset_ptr = NULL;
	if (info->attrs[JNLAR_OFFSET]) {
		error = parse_instance(info->attrs[JNLAR_OFFSET], &offset);
		if (error)
			goto revert_start;
		offset_ptr = &offset;
		LOG_DEBUG("Offset: [%x %s %u]", offset.ns, offset.iname,
				offset.xf);
	}

	error = jresponse_init(&response, info);
	if (error)
		goto revert_start;

	error = xlator_foreach(get_jool_hdr(info)->xt, serialize_instance,
			response.skb, offset_ptr);

	error = jresponse_send_array(NULL, &response, error);
	if (error)
		goto revert_start;

	request_handle_end(NULL);
	return 0;

revert_start:
	request_handle_end(NULL);
fail:
	return jresponse_send_simple(NULL, info, error);
}

int handle_instance_add(struct sk_buff *skb, struct genl_info *info)
{
	static struct nla_policy add_policy[JNLAIA_COUNT] = {
		[JNLAIA_XF] = { .type = NLA_U8 },
		[JNLAIA_POOL6] = { .type = NLA_NESTED, },
	};
	struct nlattr *attrs[JNLAIA_COUNT];
	struct config_prefix6 pool6;
	__u8 xf;
	int error;

	LOG_DEBUG("Adding Jool instance.");

	error = request_handle_start(info, XT_ANY, NULL, true);
	if (error)
		goto abort;

	if (!info->attrs[JNLAR_OPERAND]) {
		log_err("The request is missing an 'Operand' attribute.");
		error = -EINVAL;
		goto revert_start;
	}

	error = jnla_parse_nested(attrs, JNLAIA_MAX, info->attrs[JNLAR_OPERAND],
			add_policy, "Operand");
	if (error)
		return error;

	error = jnla_get_u8(attrs[JNLAIA_XF], "framework", &xf);
	if (error)
		goto revert_start;
	pool6.set = false;
	if (attrs[JNLAIA_POOL6]) {
		error = jnla_get_prefix6_optional(attrs[JNLAIA_POOL6], "pool6",
				&pool6);
		if (error)
			goto revert_start;
	}

	return jresponse_send_simple(NULL, info, xlator_add(
		xf | get_jool_hdr(info)->xt,
		get_jool_hdr(info)->iname,
		pool6.set ? &pool6.prefix : NULL,
		NULL
	));

revert_start:
	request_handle_end(NULL);
abort:
	return jresponse_send_simple(NULL, info, error);
}

int handle_instance_hello(struct sk_buff *skb, struct genl_info *info)
{
	struct jool_response response;
	int error;

	LOG_DEBUG("Handling instance Hello.");

	error = request_handle_start(info, XT_ANY, NULL, true);
	if (error)
		goto fail;

	error = jresponse_init(&response, info);
	if (error)
		goto revert_start;

	error = xlator_find_current(get_jool_hdr(info)->iname,
			XF_ANY | get_jool_hdr(info)->xt, NULL);
	switch (error) {
	case 0:
		error = nla_put_u8(response.skb, JNLAIS_STATUS, IHS_ALIVE);
		if (error)
			goto put_failure;
		break;
	case -ESRCH:
		error = nla_put_u8(response.skb, JNLAIS_STATUS, IHS_DEAD);
		if (error)
			goto put_failure;
		break;
	default:
		log_err("Unknown status.");
		error = -EINVAL;
		goto revert_start;
	}

	return jresponse_send(&response);

put_failure:
	report_put_failure();
revert_start:
	request_handle_end(NULL);
fail:
	return jresponse_send_simple(NULL, info, error);
}

int handle_instance_rm(struct sk_buff *skb, struct genl_info *info)
{
	int error;

	LOG_DEBUG("Removing Jool instance.");

	error = request_handle_start(info, XT_ANY, NULL, true);
	if (!error)
		error = xlator_rm(get_jool_hdr(info)->xt, get_jool_hdr(info)->iname);
	request_handle_end(NULL);

	return jresponse_send_simple(NULL, info, error);
}

int handle_instance_flush(struct sk_buff *skb, struct genl_info *info)
{
	int error;

	LOG_DEBUG("Flushing all instances from this namespace.");

	error = request_handle_start(info, XT_ANY, NULL, true);
	if (!error)
		error = xlator_flush(get_jool_hdr(info)->xt);
	request_handle_end(NULL);

	return jresponse_send_simple(NULL, info, error);
}
