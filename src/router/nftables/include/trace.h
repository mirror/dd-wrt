#ifndef NFTABLES_TRACE_H
#define NFTABLES_TRACE_H
#include <linux/netlink.h>

struct netlink_mon_handler;
int netlink_events_trace_cb(const struct nlmsghdr *nlh, int type,
			    struct netlink_mon_handler *monh);
#endif /* NFTABLES_TRACE_H */
