#include "usr/nl/common.h"

#include <netlink/errno.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include "common/config.h"
#include "usr/nl/attribute.h"

struct jool_result joolnl_err_msgsize(void)
{
	return result_from_error(
		-NLE_NOMEM,
		"Cannot build Netlink request: Packet is too small."
	);
}

/* Boilerplate that needs to be done during every foreach response handler. */
struct jool_result joolnl_init_foreach(struct nl_msg *response, bool *done)
{
	struct nlmsghdr *nhdr;
	struct joolnlhdr *jhdr;

	nhdr = nlmsg_hdr(response);
	if (!genlmsg_valid_hdr(nhdr, sizeof(struct joolnlhdr))) {
		return result_from_error(
			-NLE_MSG_TOOSHORT,
			"The kernel module's response lacks headers."
		);
	}

	jhdr = genlmsg_user_hdr(genlmsg_hdr(nhdr));
	*done = !(jhdr->flags & JOOLNLHDR_FLAGS_M);

	return result_success();
}

/* Boilerplate that needs to be done during most foreach response handlers. */
struct jool_result joolnl_init_foreach_list(struct nl_msg *msg,
		char const *what, bool *done)
{
	struct genlmsghdr *ghdr;
	struct jool_result result;

	result = joolnl_init_foreach(msg, done);
	if (result.error)
		return result;

	ghdr = genlmsg_hdr(nlmsg_hdr(msg));
	return jnla_validate_list(
		genlmsg_attrdata(ghdr, sizeof(struct joolnlhdr)),
		genlmsg_attrlen(ghdr, sizeof(struct joolnlhdr)),
		what,
		joolnl_struct_list_policy
	);
}
