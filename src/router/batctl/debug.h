/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#ifndef _BATCTL_DEBUG_H
#define _BATCTL_DEBUG_H

#include <stddef.h>
#include "main.h"

struct debug_table_data {
	int (*netlink_fn)(struct state *state, char *hard_iface, int read_opt,
			 float orig_timeout, float watch_interval);
	unsigned int option_unicast_only:1;
	unsigned int option_multicast_only:1;
	unsigned int option_timeout_interval:1;
	unsigned int option_orig_iface:1;
};

int handle_debug_table(struct state *state, int argc, char **argv);

#endif
