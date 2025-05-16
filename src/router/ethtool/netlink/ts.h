/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ts.h - netlink timestamping
 *
 * Copyright (c) 2025 Bootlin, Kory Maincent <kory.maincent@bootlin.com>
 */

#ifndef ETHTOOL_NETLINK_TS_H__
#define ETHTOOL_NETLINK_TS_H__

const char *tsinfo_hwprov_qualifier_names(u32 val);
int tsinfo_show_hwprov(const struct nlattr *nest);
int tsinfo_qualifier_parser(struct nl_context *nlctx,
			    uint16_t type __maybe_unused,
			    const void *data __maybe_unused,
			    struct nl_msg_buff *msgbuff __maybe_unused,
			    void *dest);
int tsinfo_dump_list(struct nl_context *nlctx, const struct nlattr *attr,
		     const char *label, const char *if_empty,
		     unsigned int stringset_id);

#endif /* ETHTOOL_NETLINK_TS_H__ */
