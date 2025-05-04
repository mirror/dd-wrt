#ifndef __LIBNETLINK_H__
#define __LIBNETLINK_H__

#include <stdio.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>

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
int rtnl_add_nl_group(struct rtnl_handle *rth, unsigned int group)
	__attribute__((warn_unused_result));
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

int addattr(struct nlmsghdr *n, int maxlen, int type);
int addattr8(struct nlmsghdr *n, int maxlen, int type, __u8 data);
int addattr16(struct nlmsghdr *n, int maxlen, int type, __u16 data);
int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data);
int addattr64(struct nlmsghdr *n, int maxlen, int type, __u64 data);
int addattrstrz(struct nlmsghdr *n, int maxlen, int type, const char *data);

int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
              int alen);
int addraw_l(struct nlmsghdr *n, int maxlen, const void *data, int len);
struct rtattr *addattr_nest(struct nlmsghdr *n, int maxlen, int type);
int addattr_nest_end(struct nlmsghdr *n, struct rtattr *nest);
struct rtattr *addattr_nest_compat(struct nlmsghdr *n, int maxlen, int type, const void *data, int len);
int addattr_nest_compat_end(struct nlmsghdr *n, struct rtattr *nest);
int rta_addattr8(struct rtattr *rta, int maxlen, int type, __u8 data);
int rta_addattr16(struct rtattr *rta, int maxlen, int type, __u16 data);
int rta_addattr32(struct rtattr *rta, int maxlen, int type, __u32 data);
int rta_addattr64(struct rtattr *rta, int maxlen, int type, __u64 data);
int rta_addattr_l(struct rtattr *rta, int maxlen, int type,
                         const void *data, int alen);

int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len);
int parse_rtattr_byindex(struct rtattr *tb[], int max, struct rtattr *rta,
                         int len);

struct rtattr *rta_nest(struct rtattr *rta, int maxlen, int type);
int rta_nest_end(struct rtattr *rta, struct rtattr *nest);

#define RTA_TAIL(rta) \
		((struct rtattr *) (((void *) (rta)) + \
				    RTA_ALIGN((rta)->rta_len)))

#define parse_rtattr_nested(tb, max, rta) \
    (parse_rtattr((tb), (max), RTA_DATA(rta), RTA_PAYLOAD(rta)))

int rtnl_listen(struct rtnl_handle *, rtnl_filter_t handler, void *jarg);
int rtnl_from_file(FILE *, rtnl_filter_t handler, void *jarg);

#define NLMSG_TAIL(nmsg) \
    ((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

#ifndef BRVLAN_RTA
#define BRVLAN_RTA(r) \
	((struct rtattr *)(((char *)(r)) + NLMSG_ALIGN(sizeof(struct br_vlan_msg))))
#endif

#endif /* __LIBNETLINK_H__ */
