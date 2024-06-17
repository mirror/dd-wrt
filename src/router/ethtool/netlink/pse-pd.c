/*
 * pse.c - netlink implementation of pse commands
 *
 * Implementation of "ethtool --show-pse <dev>" and
 * "ethtool --set-pse <dev> ..."
 */

#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "parser.h"

/* PSE_GET */

static const char *podl_pse_admin_state_name(u32 val)
{
	switch (val) {
	case ETHTOOL_PODL_PSE_ADMIN_STATE_UNKNOWN:
		return "unknown";
	case ETHTOOL_PODL_PSE_ADMIN_STATE_DISABLED:
		return "disabled";
	case ETHTOOL_PODL_PSE_ADMIN_STATE_ENABLED:
		return "enabled";
	default:
		return "unsupported";
	}
}

static const char *podl_pse_pw_d_status_name(u32 val)
{
	switch (val) {
	case ETHTOOL_PODL_PSE_PW_D_STATUS_UNKNOWN:
		return "unknown";
	case ETHTOOL_PODL_PSE_PW_D_STATUS_DISABLED:
		return "disabled";
	case ETHTOOL_PODL_PSE_PW_D_STATUS_SEARCHING:
		return "searching";
	case ETHTOOL_PODL_PSE_PW_D_STATUS_DELIVERING:
		return "delivering power";
	case ETHTOOL_PODL_PSE_PW_D_STATUS_SLEEP:
		return "sleep";
	case ETHTOOL_PODL_PSE_PW_D_STATUS_IDLE:
		return "idle";
	case ETHTOOL_PODL_PSE_PW_D_STATUS_ERROR:
		return "error";
	default:
		return "unsupported";
	}
}

int pse_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_PSE_MAX + 1] = {};
	struct nl_context *nlctx = data;
	DECLARE_ATTR_TB_INFO(tb);
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_PSE_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	if (silent)
		print_nl();

	open_json_object(NULL);

	print_string(PRINT_ANY, "ifname", "PSE attributes for %s:\n",
		     nlctx->devname);

	if (tb[ETHTOOL_A_PODL_PSE_ADMIN_STATE]) {
		u32 val;

		val = mnl_attr_get_u32(tb[ETHTOOL_A_PODL_PSE_ADMIN_STATE]);
		print_string(PRINT_ANY, "podl-pse-admin-state",
			     "PoDL PSE Admin State: %s\n",
			     podl_pse_admin_state_name(val));
	}

	if (tb[ETHTOOL_A_PODL_PSE_PW_D_STATUS]) {
		u32 val;

		val = mnl_attr_get_u32(tb[ETHTOOL_A_PODL_PSE_PW_D_STATUS]);
		print_string(PRINT_ANY, "podl-pse-power-detection-status",
			     "PoDL PSE Power Detection Status: %s\n",
			     podl_pse_pw_d_status_name(val));
	}

	close_json_object();

	return MNL_CB_OK;
}

int nl_gpse(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_PSE_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	nlsk = nlctx->ethnl_socket;
	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_PSE_GET,
				      ETHTOOL_A_PSE_HEADER, 0);
	if (ret < 0)
		return ret;

	new_json_obj(ctx->json);
	ret = nlsock_send_get_request(nlsk, pse_reply_cb);
	delete_json_obj();

	return ret;
}

/* PSE_SET */

static const struct lookup_entry_u32 podl_pse_admin_control_values[] = {
	{ .arg = "enable",	.val = ETHTOOL_PODL_PSE_ADMIN_STATE_ENABLED },
	{ .arg = "disable",	.val = ETHTOOL_PODL_PSE_ADMIN_STATE_DISABLED },
	{}
};

static const struct param_parser spse_params[] = {
	{
		.arg		= "podl-pse-admin-control",
		.type		= ETHTOOL_A_PODL_PSE_ADMIN_CONTROL,
		.handler	= nl_parse_lookup_u32,
		.handler_data	= podl_pse_admin_control_values,
		.min_argc	= 1,
	},
	{}
};

int nl_spse(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_PSE_SET, false))
		return -EOPNOTSUPP;
	if (!ctx->argc) {
		fprintf(stderr, "ethtool (--set-pse): parameters missing\n");
		return 1;
	}

	nlctx->cmd = "--set-pse";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_PSE_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_PSE_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, spse_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		return 1;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return 83;
	ret = nlsock_process_reply(nlsk, nomsg_reply_cb, nlctx);
	if (ret == 0)
		return 0;
	else
		return nlctx->exit_code ?: 83;
}
