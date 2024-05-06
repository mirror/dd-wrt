#include "modsocket.h"

#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>

#include "usr/util/cJSON.h"
#include "usr/util/file.h"
#include "usr/nl/joold.h"
#include "usr/joold/log.h"
#include "usr/joold/netsocket.h"

static struct joolnl_socket jsocket;
static char *iname;

/* Called by the net socket whenever joold receives data from the network. */
void modsocket_send(void *request, size_t request_len)
{
	struct jool_result result;
	result = joolnl_joold_add(&jsocket, iname, request, request_len);
	pr_result(&result);
}

static void do_ack(void)
{
	struct jool_result result;

	result = joolnl_joold_ack(&jsocket, iname);
	if (result.error)
		pr_result(&result);
}

/**
 * Called when joold receives data from kernelspace.
 * This data can be either sessions that should be multicasted to other joolds
 * or a response to something sent by modsocket_send().
 */
static int updated_entries_cb(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *nhdr;
	struct genlmsghdr *ghdr;
	struct joolnlhdr *jhdr;
	struct nlattr *root;
	struct jool_result result;

	syslog(LOG_DEBUG, "Received a packet from kernelspace.");

	nhdr = nlmsg_hdr(msg);
	if (!genlmsg_valid_hdr(nhdr, sizeof(struct joolnlhdr))) {
		syslog(LOG_ERR, "Kernel sent invalid data: Message too short to contain headers");
		goto einval;
	}

	ghdr = genlmsg_hdr(nhdr);

	jhdr = genlmsg_user_hdr(ghdr);
	result = validate_joolnlhdr(jhdr, XT_NAT64);
	if (result.error) {
		pr_result(&result);
		goto fail;
	}
	if (jhdr->flags & JOOLNLHDR_FLAGS_ERROR) {
		result = joolnl_msg2result(msg);
		pr_result(&result);
		goto fail;
	}

	root = genlmsg_attrdata(ghdr, sizeof(struct joolnlhdr));
	if (nla_type(root) != JNLAR_SESSION_ENTRIES) {
		syslog(LOG_ERR, "Kernel sent invalid data: Message lacks a session container");
		goto einval;
	}

	/*
	 * Why do we detach the session container?
	 * Because the Netlink API forces the other end to recreate it.
	 * (See modsocket_send())
	 */
	netsocket_send(nla_data(root), nla_len(root));
	do_ack();
	return 0;

einval:
	result.error = -EINVAL;
fail:
	do_ack(); /* Tell kernel to flush the packet queue anyway. */
	return (result.error < 0) ? result.error : -result.error;
}

static int read_json(int argc, char **argv)
{
	char *file;
	cJSON *json, *child;
	struct jool_result result;

	if (argc < 3) {
		iname = NULL;
		return 0;
	}

	syslog(LOG_INFO, "Opening file %s...", argv[2]);
	result = file_to_string(argv[2], &file);
	if (result.error)
		return pr_result(&result);

	json = cJSON_Parse(file);
	if (!json) {
		syslog(LOG_ERR, "JSON syntax error.");
		syslog(LOG_ERR, "The JSON parser got confused around about here:");
		syslog(LOG_ERR, "%s", cJSON_GetErrorPtr());
		free(file);
		return 1;
	}

	free(file);

	child = cJSON_GetObjectItem(json, "instance");
	if (child) {
		iname = strdup(child->valuestring);
		if (!iname) {
			cJSON_Delete(json);
			return -ENOMEM;
		}
	} else {
		iname = NULL;
	}

	cJSON_Delete(json);
	return 0;
}

static int create_socket(void)
{
	int family_mc_grp;
	struct jool_result result;

	result = joolnl_setup(&jsocket, XT_NAT64);
	if (result.error)
		return pr_result(&result);

	result.error = nl_socket_modify_cb(jsocket.sk, NL_CB_VALID,
			NL_CB_CUSTOM, updated_entries_cb, NULL);
	if (result.error) {
		syslog(LOG_ERR, "Couldn't modify receiver socket's callbacks.");
		goto fail;
	}

	family_mc_grp = genl_ctrl_resolve_grp(jsocket.sk, JOOLNL_FAMILY,
			JOOLNL_MULTICAST_GRP_NAME);
	if (family_mc_grp < 0) {
		syslog(LOG_ERR, "Unable to resolve the Netlink multicast group.");
		result.error = family_mc_grp;
		goto fail;
	}

	result.error = nl_socket_add_membership(jsocket.sk, family_mc_grp);
	if (result.error) {
		syslog(LOG_ERR, "Can't register to the Netlink multicast group.");
		goto fail;
	}

	return 0;

fail:
	joolnl_teardown(&jsocket);
	syslog(LOG_ERR, "Netlink error message: %s", nl_geterror(result.error));
	return result.error;
}

int modsocket_setup(int argc, char **argv)
{
	int error;

	error = read_json(argc, argv);
	if (error)
		return error;

	return create_socket();
}

void modsocket_teardown(void)
{
	free(iname);
	joolnl_teardown(&jsocket);
}

void *modsocket_listen(void *arg)
{
	int error;

	do {
		error = nl_recvmsgs_default(jsocket.sk);
		if (error < 0) {
			syslog(LOG_ERR, "Error receiving packet from kernelspace: %s",
					nl_geterror(error));
		}
	} while (true);

	return 0;
}
