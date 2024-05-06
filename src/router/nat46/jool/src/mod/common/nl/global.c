#include "mod/common/nl/global.h"

#include "common/constants.h"
#include "mod/common/log.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/nl/nl_core.h"
#include "mod/common/nl/attribute.h"
#include "mod/common/db/eam.h"
#include "mod/common/db/global.h"

static int serialize_global(struct joolnl_global_meta const *meta, void *global,
		void *skb)
{
	return joolnl_global_raw2nl(meta, global, skb) ? 1 : 0;
}

int handle_global_foreach(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct jool_response response;
	enum joolnl_attr_global offset;
	int error;

	error = request_handle_start(info, XT_ANY, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Returning 'Global' options.");

	error = jresponse_init(&response, info);
	if (error)
		goto revert_start;

	offset = 0;
	if (info->attrs[JNLAR_OFFSET_U8]) {
		offset = nla_get_u8(info->attrs[JNLAR_OFFSET_U8]);
		__log_debug(&jool, "Offset: [%u]", offset);
	}

	error = globals_foreach(&jool.globals, xlator_get_type(&jool),
			serialize_global, response.skb, offset);

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

int handle_global_update(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	int error;

	/*
	 * This is implemented as an atomic configuration run with only a single
	 * globals modification.
	 *
	 * Why?
	 *
	 * First, I can't just modify the value directly because ongoing
	 * translations could be using it. Making those values atomic is
	 * awkward because not all of them are basic data types.
	 *
	 * Protecting the values by a spinlock is feasible but dirty and not
	 * very performant. The translating code wants to query the globals
	 * often and I don't think that locking all the time is very healthy.
	 *
	 * [Also, the global values should ideally]
	 * remain constant through a translation, because bad things might
	 * happen if a value is queried at the beginning of the pipeline, some
	 * stuff is done based on it, and a different value pops when queried
	 * later. We could ask the code to query every value just once, but
	 * really that's not intuitive and won't sit well for new coders.
	 *
	 * So really, we don't want to edit values. We want to replace the
	 * entire structure via RCU so ongoing translations will keep their
	 * current configurations and future ones will have the new config from
	 * the beginning.
	 *
	 * Which leads to the second point: I don't want to protect
	 * xlator.globals with RCU either because I don't want to lock RCU every
	 * single time I want to query a global. Now I know that RCU-locking is
	 * faster than spin-locking, but hear me out:
	 *
	 * We could just change the entire xlator. Not only because we already
	 * need to support that for atomic configuration, but also because it
	 * literally does not impose any additional synchronization rules
	 * whatsoever for the translating code. The only overhead over replacing
	 * only xlator.globals is that we need to allocate an extra xlator
	 * during this operation. Which is not a recurrent operation at all.
	 *
	 * So let's STFU and do that.
	 */

	error = request_handle_start(info, XT_ANY, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Updating 'Global' value.");

	if (!info->attrs[JNLAR_GLOBALS]) {
		log_err("Request is missing a globals container.");
		error = -EINVAL;
		goto revert_start;
	}

	error = global_update(&jool.globals, get_jool_hdr(info)->xt,
			get_jool_hdr(info)->flags & JOOLNLHDR_FLAGS_FORCE,
			info->attrs[JNLAR_GLOBALS]);
	if (error)
		goto revert_start;

	/*
	 * Notice that this @jool is also a clone and we're the only thread
	 * with access to it.
	 */
	error = xlator_replace(&jool);

revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}

int global_update(struct jool_globals *cfg, xlator_type xt, bool force,
		struct nlattr *root)
{
	const struct nla_policy *policy;
	struct nlattr *attrs[JNLAG_COUNT];
	struct joolnl_global_meta const *meta;
	enum joolnl_attr_global id;
	int error;

	switch (xt) {
	case XT_SIIT:
		policy = siit_globals_policy;
		break;
	case XT_NAT64:
		policy = nat64_globals_policy;
		break;
	default:
		log_err(XT_VALIDATE_ERRMSG);
		return -EINVAL;
	}

	error = jnla_parse_nested(attrs, JNLAG_MAX, root, policy, "Globals Container");
	if (error)
		return error;

	joolnl_global_foreach_meta(meta) {
		if (!(joolnl_global_meta_xt(meta) & xt))
			continue;
		id = joolnl_global_meta_id(meta);
		if (!attrs[id])
			continue;

		error = joolnl_global_nl2raw(meta, attrs[id],
				joolnl_global_get(meta, cfg),
				force);
		if (error)
			return error;
	}

	return 0;
}
