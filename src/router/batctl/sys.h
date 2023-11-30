/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#ifndef _BATCTL_SYS_H
#define _BATCTL_SYS_H

#include "main.h"

#include <linux/genetlink.h>
#include <netlink/genl/genl.h>
#include <stdbool.h>

#include "batman_adv.h"
#include "netlink.h"

#define VLAN_ID_MAX_LEN		4

struct settings_data {
	void *data;
	int (*parse)(struct state *state, int argc, char *argv[]);
	int (*netlink_get)(struct state *state);
	int (*netlink_set)(struct state *state);
};

struct simple_boolean_data {
	bool val;
};

int handle_sys_setting(struct state *state, int argc, char **argv);
int parse_simple_boolean(struct state *state, int argc, char *argv[]);

int sys_simple_nlquery(struct state *state, enum batadv_nl_commands nl_cmd,
		       nl_recvmsg_msg_cb_t attribute_cb,
		       nl_recvmsg_msg_cb_t callback);
int sys_simple_print_boolean(struct nl_msg *msg, void *arg,
			     enum batadv_nl_attrs attr);

#endif
