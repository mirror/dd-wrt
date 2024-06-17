/*
 * mm.c - netlink implementation of MAC merge layer settings
 *
 * Implementation of "ethtool --show-mm <dev>" and "ethtool --set-mm <dev> ..."
 */

#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "bitset.h"
#include "parser.h"

/* MM_GET */

static const char *
mm_verify_state_to_string(enum ethtool_mm_verify_status state)
{
	switch (state) {
	case ETHTOOL_MM_VERIFY_STATUS_INITIAL:
		return "INITIAL";
	case ETHTOOL_MM_VERIFY_STATUS_VERIFYING:
		return "VERIFYING";
	case ETHTOOL_MM_VERIFY_STATUS_SUCCEEDED:
		return "SUCCEEDED";
	case ETHTOOL_MM_VERIFY_STATUS_FAILED:
		return "FAILED";
	case ETHTOOL_MM_VERIFY_STATUS_DISABLED:
		return "DISABLED";
	default:
		return "UNKNOWN";
	}
}

static int show_mm_stats(const struct nlattr *nest)
{
	const struct nlattr *tb[ETHTOOL_A_MM_STAT_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	static const struct {
		unsigned int attr;
		char *name;
	} stats[] = {
		{ ETHTOOL_A_MM_STAT_REASSEMBLY_ERRORS, "MACMergeFrameAssErrorCount" },
		{ ETHTOOL_A_MM_STAT_SMD_ERRORS, "MACMergeFrameSmdErrorCount" },
		{ ETHTOOL_A_MM_STAT_REASSEMBLY_OK, "MACMergeFrameAssOkCount" },
		{ ETHTOOL_A_MM_STAT_RX_FRAG_COUNT, "MACMergeFragCountRx" },
		{ ETHTOOL_A_MM_STAT_TX_FRAG_COUNT, "MACMergeFragCountTx" },
		{ ETHTOOL_A_MM_STAT_HOLD_COUNT, "MACMergeHoldCount" },
	};
	bool header = false;
	unsigned int i;
	size_t n;
	int ret;

	ret = mnl_attr_parse_nested(nest, attr_cb, &tb_info);
	if (ret < 0)
		return ret;

	open_json_object("statistics");
	for (i = 0; i < ARRAY_SIZE(stats); i++) {
		char fmt[64];

		if (!tb[stats[i].attr])
			continue;

		if (!header && !is_json_context()) {
			printf("Statistics:\n");
			header = true;
		}

		if (mnl_attr_validate(tb[stats[i].attr], MNL_TYPE_U64)) {
			fprintf(stderr, "malformed netlink message (statistic)\n");
			goto err_close_stats;
		}

		n = snprintf(fmt, sizeof(fmt), "  %s: %%" PRIu64 "\n",
			     stats[i].name);
		if (n >= sizeof(fmt)) {
			fprintf(stderr, "internal error - malformed label\n");
			continue;
		}

		print_u64(PRINT_ANY, stats[i].name, fmt,
			  mnl_attr_get_u64(tb[stats[i].attr]));
	}
	close_json_object();

	return 0;

err_close_stats:
	close_json_object();
	return -1;
}

int mm_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_MM_MAX + 1] = {};
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
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_MM_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	if (silent)
		print_nl();

	open_json_object(NULL);

	print_string(PRINT_ANY, "ifname", "MAC Merge layer state for %s:\n",
		     nlctx->devname);

	show_bool("pmac-enabled", "pMAC enabled: %s\n",
		  tb[ETHTOOL_A_MM_PMAC_ENABLED]);
	show_bool("tx-enabled", "TX enabled: %s\n",
		  tb[ETHTOOL_A_MM_TX_ENABLED]);
	show_bool("tx-active", "TX active: %s\n", tb[ETHTOOL_A_MM_TX_ACTIVE]);
	show_u32("tx-min-frag-size", "TX minimum fragment size: ",
		 tb[ETHTOOL_A_MM_TX_MIN_FRAG_SIZE]);
	show_u32("rx-min-frag-size", "RX minimum fragment size: ",
		 tb[ETHTOOL_A_MM_RX_MIN_FRAG_SIZE]);
	show_bool("verify-enabled", "Verify enabled: %s\n",
		  tb[ETHTOOL_A_MM_VERIFY_ENABLED]);
	show_u32("verify-time", "Verify time: ",
		 tb[ETHTOOL_A_MM_VERIFY_TIME]);
	show_u32("max-verify-time", "Max verify time: ",
		 tb[ETHTOOL_A_MM_MAX_VERIFY_TIME]);

	if (tb[ETHTOOL_A_MM_VERIFY_STATUS]) {
		u8 val = mnl_attr_get_u8(tb[ETHTOOL_A_MM_VERIFY_STATUS]);

		print_string(PRINT_ANY, "verify-status", "Verification status: %s\n",
			     mm_verify_state_to_string(val));
	}

	if (tb[ETHTOOL_A_MM_STATS]) {
		ret = show_mm_stats(tb[ETHTOOL_A_MM_STATS]);
		if (ret) {
			fprintf(stderr, "Failed to print stats: %d\n", ret);
			goto err;
		}
	}

	if (!silent)
		print_nl();

	close_json_object();

	return MNL_CB_OK;

err:
	close_json_object();
	return err_ret;
}

int nl_get_mm(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	u32 flags;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_MM_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	flags = get_stats_flag(nlctx, ETHTOOL_MSG_MM_GET, ETHTOOL_A_MM_HEADER);
	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_MM_GET,
				      ETHTOOL_A_MM_HEADER, flags);
	if (ret)
		return ret;

	new_json_obj(ctx->json);
	ret = nlsock_send_get_request(nlsk, mm_reply_cb);
	delete_json_obj();
	return ret;
}

/* MM_SET */

static const struct param_parser mm_set_params[] = {
	{
		.arg		= "verify-enabled",
		.type		= ETHTOOL_A_MM_VERIFY_ENABLED,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "verify-time",
		.type		= ETHTOOL_A_MM_VERIFY_TIME,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-enabled",
		.type		= ETHTOOL_A_MM_TX_ENABLED,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "pmac-enabled",
		.type		= ETHTOOL_A_MM_PMAC_ENABLED,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-min-frag-size",
		.type		= ETHTOOL_A_MM_TX_MIN_FRAG_SIZE,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{}
};

int nl_set_mm(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_MM_SET, false))
		return -EOPNOTSUPP;

	nlctx->cmd = "--set-mm";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_MM_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret)
		return ret;

	if (ethnla_fill_header(msgbuff, ETHTOOL_A_MM_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, mm_set_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret)
		return ret;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return ret;

	ret = nlsock_process_reply(nlsk, nomsg_reply_cb, nlctx);
	if (ret)
		return nlctx->exit_code;

	return 0;
}
