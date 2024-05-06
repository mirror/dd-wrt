#include "mod/common/nl/address.h"

#include "mod/common/address_xlat.h"
#include "mod/common/log.h"
#include "mod/common/xlator.h"
#include "mod/common/nl/attribute.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/nl/nl_core.h"

static int jnla_put_entry(struct jool_response *response,
		struct address_translation_entry *entry)
{
	int error;

	switch (entry->method) {
	case AXM_RFC6052:
		error = jnla_put_prefix6(response->skb, JNLAAQ_PREFIX6052, &entry->prefix6052);
		break;
	case AXM_EAMT:
		error = jnla_put_eam(response->skb, JNLAAQ_EAM, &entry->eam);
		break;
	case AXM_RFC6791:
		return 0;
	default:
		log_err("Unknown translation method: %u", entry->method);
		return -EINVAL;
	}

	if (error)
		report_put_failure();
	return error;
}

int handle_address_query64(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct in6_addr request;
	struct result_addrxlat64 result;
	struct addrxlat_result verdict;
	struct jool_response response;
	int error;

	error = request_handle_start(info, XT_SIIT, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Handling 6->4 address translation query.");

	/* Parse request */
	error = jnla_get_addr6(info->attrs[JNLAR_ADDR_QUERY], "IPv6 address", &request);
	if (error)
		goto revert_start;

	/* Perform query */
	verdict = addrxlat_siit64(&jool, &request, &result, true);
	if (verdict.verdict != ADDRXLAT_CONTINUE) {
		log_err("Unable to translate %pI6c: %s", &request, verdict.reason);
		error = -EINVAL;
		goto revert_start;
	}

	/* Build response */
	error = jresponse_init(&response, info);
	if (error)
		goto revert_start;
	error = jnla_put_addr4(response.skb, JNLAAQ_ADDR4, &result.addr);
	if (error) {
		report_put_failure();
		goto drop_response;
	}
	error = jnla_put_entry(&response, &result.entry);
	if (error)
		goto drop_response;

	/* Send response */
	request_handle_end(&jool);
	return jresponse_send(&response);

drop_response:
	jresponse_cleanup(&response);
revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}

int handle_address_query46(struct sk_buff *skb, struct genl_info *info)
{
	struct xlator jool;
	struct in_addr request;
	struct result_addrxlat46 result;
	struct addrxlat_result verdict;
	struct jool_response response;
	int error;

	error = request_handle_start(info, XT_SIIT, &jool, true);
	if (error)
		return jresponse_send_simple(NULL, info, error);

	__log_debug(&jool, "Handling 4->6 address translation query.");

	/* Parse request */
	error = jnla_get_addr4(info->attrs[JNLAR_ADDR_QUERY], "IPv4 address", &request);
	if (error)
		goto revert_start;

	/* Perform query */
	verdict = addrxlat_siit46(&jool, request.s_addr, &result, true, true);
	if (verdict.verdict != ADDRXLAT_CONTINUE) {
		log_err("Unable to translate %pI4: %s", &request, verdict.reason);
		error = -EINVAL;
		goto revert_start;
	}

	/* Build response */
	error = jresponse_init(&response, info);
	if (error)
		goto revert_start;
	error = jnla_put_addr6(response.skb, JNLAAQ_ADDR6, &result.addr);
	if (error) {
		report_put_failure();
		goto drop_response;
	}
	error = jnla_put_entry(&response, &result.entry);
	if (error)
		goto drop_response;

	/* Send response */
	request_handle_end(&jool);
	return jresponse_send(&response);

drop_response:
	jresponse_cleanup(&response);
revert_start:
	error = jresponse_send_simple(&jool, info, error);
	request_handle_end(&jool);
	return error;
}
