#include "usr/nl/joold.h"

#include <stddef.h>
#include <netlink/msg.h>
#include "common/config.h"

static struct jool_result send_to_kernel(struct joolnl_socket *sk,
		struct nl_msg *msg)
{
	int error;

	error = nl_send_auto(sk->sk, msg);
	nlmsg_free(msg);
	if (error < 0) {
		return result_from_error(
			error,
			"Could not dispatch the request to kernelspace: %s",
			nl_geterror(error)
		);
	}

	return result_success();
}

struct jool_result joolnl_joold_add(struct joolnl_socket *sk, char const *iname,
		void const *data, size_t data_len)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, JNLOP_JOOLD_ADD, 0, &msg);
	if (result.error)
		return result;

	result.error = nla_put(msg, NLA_F_NESTED | JNLAR_SESSION_ENTRIES,
			data_len, data);
	if (result.error < 0) {
		nlmsg_free(msg);
		/*
		 * This is fine as long as page size > 1500.
		 * But admittedly, it's not the most elegant implementation.
		 */
		return result_from_error(
			result.error,
			"Can't send joold sessions to kernel: Packet too small."
		);
	}

	return send_to_kernel(sk, msg);
}

struct jool_result joolnl_joold_advertise(struct joolnl_socket *sk,
		char const *iname)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, JNLOP_JOOLD_ADVERTISE, 0, &msg);
	if (result.error)
		return result;

	return send_to_kernel(sk, msg);
}

struct jool_result joolnl_joold_ack(struct joolnl_socket *sk, char const *iname)
{
	struct nl_msg *msg;
	struct jool_result result;

	result = joolnl_alloc_msg(sk, iname, JNLOP_JOOLD_ACK, 0, &msg);
	if (result.error)
		return result;

	return send_to_kernel(sk, msg);
}
