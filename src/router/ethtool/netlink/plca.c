/*
 * plca.c - netlink implementation of plca command
 *
 * Implementation of "ethtool --show-plca <dev>" and
 * "ethtool --set-plca <dev> ..."
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "bitset.h"
#include "parser.h"

/* PLCA_GET_CFG */

int plca_get_cfg_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_PLCA_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	bool silent;
	int idv = 255;
	int err_ret;
	int val;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;

	nlctx->devname = get_dev_name(tb[ETHTOOL_A_PLCA_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	if (silent)
		putchar('\n');

	printf("PLCA settings for %s:\n", nlctx->devname);

	// check if PLCA is enabled
	printf("\tEnabled: ");

	if (!tb[ETHTOOL_A_PLCA_ENABLED]) {
		printf("not supported");
	} else {
		val = mnl_attr_get_u8(tb[ETHTOOL_A_PLCA_ENABLED]);
		printf(val ? "Yes" : "No");
	}
	putchar('\n');

	// get node ID
	printf("\tlocal node ID: ");

	if (!tb[ETHTOOL_A_PLCA_NODE_ID]) {
		printf("not supported");
	} else {
		idv = mnl_attr_get_u32(tb[ETHTOOL_A_PLCA_NODE_ID]);
		printf("%u (%s)", idv,
		       idv == 0 ? "coordinator" :
		       idv == 255 ? "unconfigured" : "follower");
	}
	putchar('\n');

	// get node count
	printf("\tNode count: ");
	if (!tb[ETHTOOL_A_PLCA_NODE_CNT]) {
		printf("not supported");
	} else {
		val = mnl_attr_get_u32(tb[ETHTOOL_A_PLCA_NODE_CNT]);
		printf("%u", val);

		// The node count is ignored by follower nodes. However, it can
		// be pre-set to enable fast coordinator role switchover.
		// Therefore, on a follower node we still wanto to show it,
		// indicating it is not currently used.
		if (tb[ETHTOOL_A_PLCA_NODE_ID] && idv != 0)
			printf(" (ignored)");
	}
	putchar('\n');

	// get TO timer (transmit opportunity timer)
	printf("\tTO timer: ");
	if (!tb[ETHTOOL_A_PLCA_TO_TMR]) {
		printf("not supported");
	} else {
		val = mnl_attr_get_u32(tb[ETHTOOL_A_PLCA_TO_TMR]);
		printf("%u BT", val);
	}
	putchar('\n');

	// get burst count
	printf("\tBurst count: ");
	if (!tb[ETHTOOL_A_PLCA_BURST_CNT]) {
		printf("not supported");
	} else {
		val = mnl_attr_get_u32(tb[ETHTOOL_A_PLCA_BURST_CNT]);
		printf("%u (%s)", val,
		       val > 0 ? "enabled" : "disabled");
	}
	putchar('\n');

	// get burst timer
	printf("\tBurst timer: ");
	if (!tb[ETHTOOL_A_PLCA_BURST_TMR]) {
		printf("not supported");
	} else {
		val = mnl_attr_get_u32(tb[ETHTOOL_A_PLCA_BURST_TMR]);
		printf("%u BT", val);
	}
	putchar('\n');

	return MNL_CB_OK;
}


int nl_plca_get_cfg(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_PLCA_GET_CFG, true))
		return -EOPNOTSUPP;

	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_PLCA_GET_CFG,
				      ETHTOOL_A_PLCA_HEADER, 0);

	if (ret < 0)
		return ret;

	return nlsock_send_get_request(nlsk, plca_get_cfg_reply_cb);
}

/* PLCA_SET_CFG */

static const struct param_parser set_plca_params[] = {
	{
		.arg		= "enable",
		.type		= ETHTOOL_A_PLCA_ENABLED,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "node-id",
		.type		= ETHTOOL_A_PLCA_NODE_ID,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "node-cnt",
		.type		= ETHTOOL_A_PLCA_NODE_CNT,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "to-tmr",
		.type		= ETHTOOL_A_PLCA_TO_TMR,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "burst-cnt",
		.type		= ETHTOOL_A_PLCA_BURST_CNT,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "burst-tmr",
		.type		= ETHTOOL_A_PLCA_BURST_TMR,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{}
};

int nl_plca_set_cfg(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_PLCA_SET_CFG, false))
		return -EOPNOTSUPP;
	if (!ctx->argc) {
		fprintf(stderr,
			"ethtool (--set-plca-cfg): parameters missing\n");
		return 1;
	}

	nlctx->cmd = "--set-plca-cfg";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_PLCA_SET_CFG,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_PLCA_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, set_plca_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		return 1;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return 76;
	ret = nlsock_process_reply(nlsk, nomsg_reply_cb, nlctx);
	if (ret == 0)
		return 0;
	else
		return nlctx->exit_code ?: 76;
}

/* PLCA_GET_STATUS */

int plca_get_status_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_PLCA_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	bool silent;
	int err_ret;
	int ret;
	u8 val;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;

	nlctx->devname = get_dev_name(tb[ETHTOOL_A_PLCA_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	if (silent)
		putchar('\n');

	printf("PLCA status of %s:\n", nlctx->devname);

	// check whether the Open Alliance TC14 standard memory map is supported
	printf("\tStatus: ");

	if (!tb[ETHTOOL_A_PLCA_STATUS]) {
		printf("not supported");
	} else {
		val = mnl_attr_get_u8(tb[ETHTOOL_A_PLCA_STATUS]);
		printf(val ? "on" : "off");
	}
	putchar('\n');

	return MNL_CB_OK;
}


int nl_plca_get_status(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_PLCA_GET_STATUS, true))
		return -EOPNOTSUPP;

	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_PLCA_GET_STATUS,
				      ETHTOOL_A_PLCA_HEADER, 0);

	if (ret < 0)
		return ret;

	return nlsock_send_get_request(nlsk, plca_get_status_reply_cb);
}
