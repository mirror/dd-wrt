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

static const char *c33_pse_admin_state_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_ADMIN_STATE_UNKNOWN:
		return "unknown";
	case ETHTOOL_C33_PSE_ADMIN_STATE_DISABLED:
		return "disabled";
	case ETHTOOL_C33_PSE_ADMIN_STATE_ENABLED:
		return "enabled";
	default:
		return "unsupported";
	}
}

static const char *c33_pse_pw_d_status_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_PW_D_STATUS_UNKNOWN:
		return "unknown";
	case ETHTOOL_C33_PSE_PW_D_STATUS_DISABLED:
		return "disabled";
	case ETHTOOL_C33_PSE_PW_D_STATUS_SEARCHING:
		return "searching";
	case ETHTOOL_C33_PSE_PW_D_STATUS_DELIVERING:
		return "delivering power";
	case ETHTOOL_C33_PSE_PW_D_STATUS_TEST:
		return "test";
	case ETHTOOL_C33_PSE_PW_D_STATUS_FAULT:
		return "fault";
	case ETHTOOL_C33_PSE_PW_D_STATUS_OTHERFAULT:
		return "otherfault";
	default:
		return "unsupported";
	}
}

static const char *c33_pse_ext_state_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_EXT_STATE_ERROR_CONDITION:
		return "Group of error_condition states";
	case ETHTOOL_C33_PSE_EXT_STATE_MR_MPS_VALID:
		return "Group of mr_mps_valid states";
	case ETHTOOL_C33_PSE_EXT_STATE_MR_PSE_ENABLE:
		return "Group of mr_pse_enable states";
	case ETHTOOL_C33_PSE_EXT_STATE_OPTION_DETECT_TED:
		return "Group of option_detect_ted";
	case ETHTOOL_C33_PSE_EXT_STATE_OPTION_VPORT_LIM:
		return "Group of option_vport_lim states";
	case ETHTOOL_C33_PSE_EXT_STATE_OVLD_DETECTED:
		return "Group of ovld_detected states";
	case ETHTOOL_C33_PSE_EXT_STATE_PD_DLL_POWER_TYPE:
		return "Group of pd_dll_power_type states";
	case ETHTOOL_C33_PSE_EXT_STATE_POWER_NOT_AVAILABLE:
		return "Group of power_not_available states";
	case ETHTOOL_C33_PSE_EXT_STATE_SHORT_DETECTED:
		return "Group of short_detected states";
	default:
		return "unsupported";
	}
}

static const char *c33_pse_ext_substate_mr_mps_valid_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_MR_MPS_VALID_DETECTED_UNDERLOAD:
		return "Underload state";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_MR_MPS_VALID_CONNECTION_OPEN:
		return "Port is not connected";
	default:
		return "unsupported";
	}
}

static const char *c33_pse_ext_substate_error_condition_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_ERROR_CONDITION_NON_EXISTING_PORT:
		return "Non-existing port number";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_ERROR_CONDITION_UNDEFINED_PORT:
		return "Undefined port";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_ERROR_CONDITION_INTERNAL_HW_FAULT:
		return "Internal hardware fault";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_ERROR_CONDITION_COMM_ERROR_AFTER_FORCE_ON:
		return "Communication error after force on";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_ERROR_CONDITION_UNKNOWN_PORT_STATUS:
		return "Unknown port status";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_ERROR_CONDITION_HOST_CRASH_TURN_OFF:
		return "Host crash turn off";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_ERROR_CONDITION_HOST_CRASH_FORCE_SHUTDOWN:
		return "Host crash force shutdown";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_ERROR_CONDITION_CONFIG_CHANGE:
		return "Configuration change";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_ERROR_CONDITION_DETECTED_OVER_TEMP:
		return "Over temperature detected";
	default:
		return "unsupported";
	}
}

static const char *c33_pse_ext_substate_mr_pse_enable_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_MR_PSE_ENABLE_DISABLE_PIN_ACTIVE:
		return "Disable pin active";
	default:
		return "unsupported";
	}
}

static const char *c33_pse_ext_substate_option_detect_ted_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_OPTION_DETECT_TED_DET_IN_PROCESS:
		return "Detection in process";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_OPTION_DETECT_TED_CONNECTION_CHECK_ERROR:
		return "Connection check error";
	default:
		return "unsupported";
	}
}

static const char *c33_pse_ext_substate_option_vport_lim_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_OPTION_VPORT_LIM_HIGH_VOLTAGE:
		return "Main supply voltage is high";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_OPTION_VPORT_LIM_LOW_VOLTAGE:
		return "Main supply voltage is low";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_OPTION_VPORT_LIM_VOLTAGE_INJECTION:
		return "Voltage injection into the port";
	default:
		return "unsupported";
	}
}

static const char *c33_pse_ext_substate_ovld_detected_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_OVLD_DETECTED_OVERLOAD:
		return "Overload state";
	default:
		return "unsupported";
	}
}

static const char *c33_pse_ext_substate_power_not_available_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_POWER_NOT_AVAILABLE_BUDGET_EXCEEDED:
		return "Power budget exceeded for the controller";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_POWER_NOT_AVAILABLE_PORT_PW_LIMIT_EXCEEDS_CONTROLLER_BUDGET:
		return "Configured port power limit exceeded controller power budget";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_POWER_NOT_AVAILABLE_PD_REQUEST_EXCEEDS_PORT_LIMIT:
		return "Power request from PD exceeds port limit";
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_POWER_NOT_AVAILABLE_HW_PW_LIMIT:
		return "Power denied due to Hardware power limit";
	default:
		return "unsupported";
	}
}

static const char *c33_pse_ext_substate_short_detected_name(u32 val)
{
	switch (val) {
	case ETHTOOL_C33_PSE_EXT_SUBSTATE_SHORT_DETECTED_SHORT_CONDITION:
		return "Short condition was detected";
	default:
		return "unsupported";
	}
}

struct c33_pse_ext_substate_desc {
	u32 state;
	const char *(*substate_name)(u32 val);
};

static const struct c33_pse_ext_substate_desc c33_pse_ext_substate_map[] = {
	{ ETHTOOL_C33_PSE_EXT_STATE_ERROR_CONDITION,
	  c33_pse_ext_substate_error_condition_name },
	{ ETHTOOL_C33_PSE_EXT_STATE_MR_MPS_VALID,
	  c33_pse_ext_substate_mr_mps_valid_name },
	{ ETHTOOL_C33_PSE_EXT_STATE_MR_PSE_ENABLE,
	  c33_pse_ext_substate_mr_pse_enable_name },
	{ ETHTOOL_C33_PSE_EXT_STATE_OPTION_DETECT_TED,
	  c33_pse_ext_substate_option_detect_ted_name },
	{ ETHTOOL_C33_PSE_EXT_STATE_OPTION_VPORT_LIM,
	  c33_pse_ext_substate_option_vport_lim_name },
	{ ETHTOOL_C33_PSE_EXT_STATE_OVLD_DETECTED,
	  c33_pse_ext_substate_ovld_detected_name },
	{ ETHTOOL_C33_PSE_EXT_STATE_POWER_NOT_AVAILABLE,
	  c33_pse_ext_substate_power_not_available_name },
	{ ETHTOOL_C33_PSE_EXT_STATE_SHORT_DETECTED,
	  c33_pse_ext_substate_short_detected_name },
	{ /* sentinel */ }
};

static void c33_pse_print_ext_substate(u32 state, u32 substate)
{
	const struct c33_pse_ext_substate_desc *substate_map;

	substate_map = c33_pse_ext_substate_map;
	while (substate_map->state) {
		if (substate_map->state == state) {
			print_string(PRINT_ANY, "c33-pse-extended-substate",
				     "Clause 33 PSE Extended Substate: %s\n",
				     substate_map->substate_name(substate));
			return;
		}
		substate_map++;
	}
}

static int c33_pse_dump_pw_limit_range(const struct nlattr *range)
{
	const struct nlattr *range_tb[ETHTOOL_A_C33_PSE_PW_LIMIT_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(range_tb);
	const struct nlattr *attr;
	u32 min, max;
	int ret;

	ret = mnl_attr_parse_nested(range, attr_cb, &range_tb_info);
	if (ret < 0) {
		fprintf(stderr,
			"malformed netlink message (power limit range)\n");
		return 1;
	}

	attr = range_tb[ETHTOOL_A_C33_PSE_PW_LIMIT_MIN];
	if (!attr || mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
		fprintf(stderr,
			"malformed netlink message (power limit min)\n");
		return 1;
	}
	min = mnl_attr_get_u32(attr);

	attr = range_tb[ETHTOOL_A_C33_PSE_PW_LIMIT_MAX];
	if (!attr || mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
		fprintf(stderr,
			"malformed netlink message (power limit max)\n");
		return 1;
	}
	max = mnl_attr_get_u32(attr);

	print_string(PRINT_ANY, "range", "\trange:\n", NULL);
	print_uint(PRINT_ANY, "min", "\t\tmin %u\n", min);
	print_uint(PRINT_ANY, "max", "\t\tmax %u\n", max);
	return 0;
}

int pse_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_PSE_MAX + 1] = {};
	struct nl_context *nlctx = data;
	const struct nlattr *attr;
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

	if (tb[ETHTOOL_A_C33_PSE_ADMIN_STATE]) {
		u32 val;

		val = mnl_attr_get_u32(tb[ETHTOOL_A_C33_PSE_ADMIN_STATE]);
		print_string(PRINT_ANY, "c33-pse-admin-state",
			     "Clause 33 PSE Admin State: %s\n",
			     c33_pse_admin_state_name(val));
	}

	if (tb[ETHTOOL_A_C33_PSE_PW_D_STATUS]) {
		u32 val;

		val = mnl_attr_get_u32(tb[ETHTOOL_A_C33_PSE_PW_D_STATUS]);
		print_string(PRINT_ANY, "c33-pse-power-detection-status",
			     "Clause 33 PSE Power Detection Status: %s\n",
			     c33_pse_pw_d_status_name(val));
	}

	if (tb[ETHTOOL_A_C33_PSE_EXT_STATE]) {
		u32 val;

		val = mnl_attr_get_u32(tb[ETHTOOL_A_C33_PSE_EXT_STATE]);
		print_string(PRINT_ANY, "c33-pse-extended-state",
			     "Clause 33 PSE Extended State: %s\n",
			     c33_pse_ext_state_name(val));

		if (tb[ETHTOOL_A_C33_PSE_EXT_SUBSTATE]) {
			u32 substate;

			substate = mnl_attr_get_u32(tb[ETHTOOL_A_C33_PSE_EXT_SUBSTATE]);
			c33_pse_print_ext_substate(val, substate);
		}
	}

	if (tb[ETHTOOL_A_C33_PSE_PW_CLASS]) {
		u32 val;

		val = mnl_attr_get_u32(tb[ETHTOOL_A_C33_PSE_PW_CLASS]);
		print_uint(PRINT_ANY, "c33-pse-power-class",
			   "Clause 33 PSE Power Class: %u\n", val);
	}

	if (tb[ETHTOOL_A_C33_PSE_ACTUAL_PW]) {
		u32 val;

		val = mnl_attr_get_u32(tb[ETHTOOL_A_C33_PSE_ACTUAL_PW]);
		print_uint(PRINT_ANY, "c33-pse-actual-power",
			   "Clause 33 PSE Actual Power: %u\n", val);
	}

	if (tb[ETHTOOL_A_C33_PSE_AVAIL_PW_LIMIT]) {
		u32 val;

		val = mnl_attr_get_u32(tb[ETHTOOL_A_C33_PSE_AVAIL_PW_LIMIT]);
		print_uint(PRINT_ANY, "c33-pse-available-power-limit",
			   "Clause 33 PSE Available Power Limit: %u\n", val);
	}

	if (tb[ETHTOOL_A_C33_PSE_PW_LIMIT_RANGES]) {
		print_string(PRINT_ANY, "c33-pse-power-limit-ranges",
			     "Clause 33 PSE Power Limit Ranges:\n", NULL);
		mnl_attr_for_each(attr, nlhdr, GENL_HDRLEN) {
			if (mnl_attr_get_type(attr) == ETHTOOL_A_C33_PSE_PW_LIMIT_RANGES) {
				if (c33_pse_dump_pw_limit_range(attr)) {
					close_json_object();
					return err_ret;
				}
			}
		}
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

static const struct lookup_entry_u32 c33_pse_admin_control_values[] = {
	{ .arg = "enable",	.val = ETHTOOL_C33_PSE_ADMIN_STATE_ENABLED },
	{ .arg = "disable",	.val = ETHTOOL_C33_PSE_ADMIN_STATE_DISABLED },
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
	{
		.arg		= "c33-pse-admin-control",
		.type		= ETHTOOL_A_C33_PSE_ADMIN_CONTROL,
		.handler	= nl_parse_lookup_u32,
		.handler_data	= c33_pse_admin_control_values,
		.min_argc	= 1,
	},
	{
		.arg		= "c33-pse-avail-pw-limit",
		.type		= ETHTOOL_A_C33_PSE_AVAIL_PW_LIMIT,
		.handler	= nl_parse_direct_u32,
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
	if (ethnla_fill_header_phy(msgbuff, ETHTOOL_A_PSE_HEADER,
				   ctx->devname, ctx->phy_index, 0))
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
