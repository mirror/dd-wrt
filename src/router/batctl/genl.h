/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#ifndef _BATCTL_GENL_H
#define _BATCTL_GENL_H

#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

int nl_get_multicast_id(struct nl_sock *sock, const char *family,
			const char *group);

#endif /* _BATCTL_GENL_H */
