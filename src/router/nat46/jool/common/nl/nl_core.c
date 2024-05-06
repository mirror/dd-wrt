#include "mod/common/nl/nl_core.h"

#include <linux/stddef.h>
#include <linux/types.h>

#include "common/config.h"
#include "common/types.h"
#include "mod/common/error_pool.h"
#include "mod/common/log.h"
#include "mod/common/wkmalloc.h"
#include "mod/common/nl/attribute.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/nl/nl_handler.h"

/*
 * Note: If you're working on this module, please keep in mind that there should
 * not be any log_err()s anywhere.
 *
 * If a preparation to send something to userspace failed, then trying to send
 * the error message (via log_err()) to userspace is a fairly lost cause.
 */

int jresponse_init(struct jool_response *response, struct genl_info *info)
{
	response->info = info;
	response->skb = genlmsg_new(GENLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!response->skb) {
		pr_err("genlmsg_new() failed.\n");
		return -ENOMEM;
	}

	response->hdr = genlmsg_put(response->skb, info->snd_portid,
			info->nlhdr->nlmsg_seq, jnl_family(), 0, 0);
	if (!response->hdr) {
		pr_err("genlmsg_put() failed.\n");
		kfree_skb(response->skb);
		return -ENOMEM;
	}

	memcpy(response->hdr, get_jool_hdr(info), sizeof(*response->hdr));
	response->initial_len = response->skb->len;
	return 0;
}

/* Swallows @response. */
int jresponse_send(struct jool_response *response)
{
	int error;

	genlmsg_end(response->skb, response->hdr);

	error = genlmsg_reply(response->skb, response->info);
	if (error)
		pr_err("genlmsg_reply() failed. (errcode %d)\n", error);

	response->skb = NULL;
	return error;
}

void jresponse_cleanup(struct jool_response *response)
{
	kfree_skb(response->skb);
	response->skb = NULL;
}

void jresponse_enable_m(struct jool_response *response)
{
	response->hdr->flags |= JOOLNLHDR_FLAGS_M;
}

int jresponse_send_array(struct xlator *jool, struct jool_response *response,
		int error)
{
	if (error < 0)
		goto cancel;

	/*
	 * Packet empty might happen when the last entry died between foreach
	 * requests.
	 */
	if (error > 0) {
		if (response->skb->len == response->initial_len) {
			report_put_failure();
			error = jresponse_send_simple(jool, response->info,
					-EINVAL);
			goto cancel;
		}
		jresponse_enable_m(response);
	}

	return jresponse_send(response);

cancel:
	jresponse_cleanup(response);
	return error;
}

int jresponse_send_simple(struct xlator *jool, struct genl_info *info,
		int error_code)
{
	struct jool_response response;
	int error;
	char *error_msg;
	size_t error_msg_size;

	if (error_code < 0)
		error_code = abs(error_code);
	else if (error_code > MAX_U16)
		error_code = MAX_U16;

	error = error_pool_get_message(&error_msg, &error_msg_size);
	if (error)
		return error; /* Error msg already printed. */

	error = jresponse_init(&response, info);
	if (error)
		goto revert_msg;

	if (error_code) {
		response.hdr->flags |= JOOLNLHDR_FLAGS_ERROR;

		error = nla_put_u16(response.skb, JNLAERR_CODE, error_code);
		if (error)
			goto revert_response;

		error = nla_put_string(response.skb, JNLAERR_MSG, error_msg);
		if (error) {
			error_msg[128] = '\0';
			error = nla_put_string(response.skb, JNLAERR_MSG,
					error_msg);
			if (error)
				goto revert_response;
		}
		__log_debug(jool, "Sending error code %d to userspace.",
				error_code);
	} else {
		__log_debug(jool, "Sending ACK to userspace.");
	}

	error = jresponse_send(&response);
	/* Fall through. */

revert_response:
	jresponse_cleanup(&response);
revert_msg:
	__wkfree("Error msg out", error_msg);
	return error;
}

