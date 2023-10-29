/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2008-2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cli
 * @defgroup cli_qdisc Queueing Disciplines
 * @{
 */

#include "nl-default.h"

#include <netlink/cli/utils.h>
#include <netlink/cli/qdisc.h>
#include <netlink/route/class.h>

struct rtnl_qdisc *nl_cli_qdisc_alloc(void)
{
	struct rtnl_qdisc *qdisc;

	if (!(qdisc = rtnl_qdisc_alloc()))
		nl_cli_fatal(ENOMEM, "Unable to allocate qdisc object");

	return qdisc;
}

/** @} */
