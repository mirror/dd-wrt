#ifndef __LIBNETLINK_H__
#define __LIBNETLINK_H__ 1

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

struct rtnl_handle
{
	int			fd;
	struct sockaddr_nl	local;
	struct sockaddr_nl	peer;
	__u32			seq;
	__u32			dump;
};

extern int rtnl_open(struct rtnl_handle *rth, unsigned subscriptions);
extern void rtnl_close(struct rtnl_handle *rth);
extern int rtnl_wilddump_request(struct rtnl_handle *rth, int fam, int type);
extern int rtnl_dump_request(struct rtnl_handle *rth, int type, void *req, int len);
extern int rtnl_dump_filter(struct rtnl_handle *rth,
			    int (*filter)(struct sockaddr_nl *, struct nlmsghdr *n, void *),
			    void *arg1,
			    int (*junk)(struct sockaddr_nl *,struct nlmsghdr *n, void *),
			    void *arg2);

extern int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len);

#endif /* __LIBNETLINK_H__ */

