#ifndef __LIBNETLINK_H__
#define __LIBNETLINK_H__

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

struct rtnl_handle
{
    int fd;
    struct sockaddr_nl local;
    struct sockaddr_nl peer;
    __u32 seq;
    __u32 dump;
};

int rtnl_open(struct rtnl_handle *rth, unsigned subscriptions);
int rtnl_open_byproto(struct rtnl_handle *rth, unsigned subscriptions,
                      int protocol);
void rtnl_close(struct rtnl_handle *rth);
int rtnl_wilddump_request(struct rtnl_handle *rth, int fam, int type);
int rtnl_dump_request(struct rtnl_handle *rth, int type, void *req, int len);

typedef int (*rtnl_filter_t)(const struct sockaddr_nl *, struct nlmsghdr *n,
                             void *);
int rtnl_dump_filter(struct rtnl_handle *rth, rtnl_filter_t filter,
                     void *arg1, rtnl_filter_t junk, void *arg2);
int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n, pid_t peer,
              unsigned groups, struct nlmsghdr *answer, rtnl_filter_t junk,
              void *jarg);
int rtnl_send(struct rtnl_handle *rth, const char *buf, int);

int addattr8(struct nlmsghdr *n, int maxlen, int type, __u8 data);
int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data);
int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
              int alen);
int addraw_l(struct nlmsghdr *n, int maxlen, const void *data, int len);
int rta_addattr32(struct rtattr *rta, int maxlen, int type, __u32 data);
int rta_addattr_l(struct rtattr *rta, int maxlen, int type,
                         const void *data, int alen);

int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len);
int parse_rtattr_byindex(struct rtattr *tb[], int max, struct rtattr *rta,
                         int len);

#define parse_rtattr_nested(tb, max, rta) \
    (parse_rtattr((tb), (max), RTA_DATA(rta), RTA_PAYLOAD(rta)))

int rtnl_listen(struct rtnl_handle *, rtnl_filter_t handler, void *jarg);
int rtnl_from_file(FILE *, rtnl_filter_t handler, void *jarg);

#define NLMSG_TAIL(nmsg) \
    ((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

#endif /* __LIBNETLINK_H__ */
