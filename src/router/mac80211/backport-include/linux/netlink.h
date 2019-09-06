#ifndef __BACKPORT_LINUX_NETLINK_H
#define __BACKPORT_LINUX_NETLINK_H
#include_next <linux/netlink.h>
#include <linux/version.h>

#if LINUX_VERSION_IS_LESS(4,12,0)
#define NETLINK_MAX_COOKIE_LEN  20

struct netlink_ext_ack {
	const char *_msg;
	const struct nlattr *bad_attr;
	u8 cookie[NETLINK_MAX_COOKIE_LEN];
	u8 cookie_len;
};

#define NL_SET_ERR_MSG(extack, msg) do {	\
	static const char _msg[] = (msg);	\
						\
	(extack)->_msg = _msg;			\
} while (0)
#endif

/* this is for patches we apply */
#if LINUX_VERSION_IS_LESS(3,7,0)
#define netlink_notify_portid(__notify) (__notify->pid)
#define NETLINK_CB_PORTID(__skb) NETLINK_CB(__skb).pid
#else
#define netlink_notify_portid(__notify) (__notify->portid)
#define NETLINK_CB_PORTID(__skb) NETLINK_CB(__skb).portid
#endif

#endif /* __BACKPORT_LINUX_NETLINK_H */
