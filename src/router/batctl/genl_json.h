/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Alexander Sarmanow <asarmanow@gmail.com>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#ifndef _BATCTL_GENLJSON_H
#define _BATCTL_GENLJSON_H

#include <stdint.h>

#include "batman_adv.h"
#include "netlink.h"

struct json_opts {
	uint8_t is_first:1;
	struct nlquery_opts query_opts;
};

struct json_query_data {
	int nlm_flags;
	enum batadv_nl_commands cmd;
};

void netlink_print_json_entries(struct nlattr *attrs[], struct json_opts *json_opts);
int handle_json_query(struct state *state, int argc, char **argv);

#endif /* _BATCTL_GENLJSON_H */
