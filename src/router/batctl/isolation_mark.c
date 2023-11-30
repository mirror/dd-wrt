// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Antonio Quartulli <a@unstable.cc>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "sys.h"

static struct isolation_mark_data {
	uint32_t isolation_mark;
	uint32_t isolation_mask;
} isolation_mark;

static int parse_isolation_mark(struct state *state, int argc, char *argv[])
{
	struct settings_data *settings = state->cmd->arg;
	struct isolation_mark_data *data = settings->data;
	char *mask_ptr;
	char buff[256];
	uint32_t mark;
	uint32_t mask;
	char *endptr;

	if (argc != 2) {
		fprintf(stderr, "Error - incorrect number of arguments (expected 1)\n");
		return -EINVAL;
	}

	strncpy(buff, argv[1], sizeof(buff));
	buff[sizeof(buff) - 1] = '\0';

	/* parse the mask if it has been specified, otherwise assume the mask is
	 * the biggest possible
	 */
	mask = 0xFFFFFFFF;
	mask_ptr = strchr(buff, '/');
	if (mask_ptr) {
		*mask_ptr = '\0';
		mask_ptr++;

		/* the mask must be entered in hex base as it is going to be a
		 * bitmask and not a prefix length
		 */
		mask = strtoul(mask_ptr, &endptr, 16);
		if (!endptr || *endptr != '\0')
			goto inval_format;
	}

	/* the mark can be entered in any base */
	mark = strtoul(buff, &endptr, 0);
	if (!endptr || *endptr != '\0')
		goto inval_format;

	data->isolation_mask = mask;
	/* erase bits not covered by the mask */
	data->isolation_mark = mark & mask;

	return 0;

inval_format:
	fprintf(stderr, "Error - incorrect number of arguments (expected 1)\n");
	fprintf(stderr, "The following formats for mark(/mask) are allowed:\n");
	fprintf(stderr, " * 0x12345678\n");
	fprintf(stderr, " * 0x12345678/0xabcdef09\n");
	return -EINVAL;
}

static int print_isolation_mark(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX + 1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct genlmsghdr *ghdr;
	int *result = arg;

	if (!genlmsg_valid_hdr(nlh, 0))
		return NL_OK;

	ghdr = nlmsg_data(nlh);

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		return NL_OK;
	}

	if (!attrs[BATADV_ATTR_ISOLATION_MARK] ||
	    !attrs[BATADV_ATTR_ISOLATION_MASK])
		return NL_OK;

	printf("0x%08x/0x%08x\n",
	       nla_get_u32(attrs[BATADV_ATTR_ISOLATION_MARK]),
	       nla_get_u32(attrs[BATADV_ATTR_ISOLATION_MASK]));

	*result = 0;
	return NL_STOP;
}

static int get_isolation_mark(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_MESH,
				  NULL, print_isolation_mark);
}

static int set_attrs_isolation_mark(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;
	struct settings_data *settings = state->cmd->arg;
	struct isolation_mark_data *data = settings->data;

	nla_put_u32(msg, BATADV_ATTR_ISOLATION_MARK, data->isolation_mark);
	nla_put_u32(msg, BATADV_ATTR_ISOLATION_MASK, data->isolation_mask);

	return 0;
}

static int set_isolation_mark(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_MESH,
				  set_attrs_isolation_mark, NULL);
}

static struct settings_data batctl_settings_isolation_mark = {
	.data = &isolation_mark,
	.parse = parse_isolation_mark,
	.netlink_get = get_isolation_mark,
	.netlink_set = set_isolation_mark,
};

COMMAND_NAMED(SUBCOMMAND_MIF, isolation_mark, "mark", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_settings_isolation_mark,
	      "[mark]            \tdisplay or modify isolation_mark setting");
