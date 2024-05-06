#include "mod/common/nl/joold.h"

#include "mod/common/log.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/nl/nl_core.h"
#include "mod/common/joold.h"

int handle_joold_add(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	int error;

	error = request_handle_start(info, XT_NAT64, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Handling joold add.");

	error = joold_sync(&jool, info->attrs[JNLAR_SESSION_ENTRIES]);
	if (error)
		goto revert_start;

	request_handle_end(&jool);
	/*
	 * Do not bother userspace with an ACK; it's not
	 * waiting nor has anything to do with it.
	 */
	return 0;

revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}

int handle_joold_advertise(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	int error;

	error = request_handle_start(info, XT_NAT64, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Handling joold advertise.");

	error = joold_advertise(&jool);

	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}

int handle_joold_ack(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	int error;

	error = request_handle_start(info, XT_NAT64, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Handling joold ack.");

	joold_ack(&jool);

	request_handle_end(&jool);
	return 0; /* Do not ack the ack. */
}
