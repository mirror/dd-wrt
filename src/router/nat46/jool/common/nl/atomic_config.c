#include "mod/common/nl/atomic_config.h"

#include "mod/common/log.h"
#include "mod/common/atomic_config.h"
#include "mod/common/nl/nl_common.h"
#include "mod/common/nl/nl_core.h"

int handle_atomconfig_request(struct sk_buff *skb, struct genl_info *info)
{
	int error;

	LOG_DEBUG("Handling atomic configuration request.");

	error = request_handle_start(info, XT_ANY, NULL, true);
	if (!error)
		error = atomconfig_add(skb, info);
	request_handle_end(NULL);

	return jresponse_send_simple(NULL, info, error);
}
