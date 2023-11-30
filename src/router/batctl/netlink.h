/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>, Andrew Lunn <andrew@lunn.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#ifndef _BATCTL_NETLINK_H
#define _BATCTL_NETLINK_H

#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <stdint.h>

struct state;

struct print_opts {
	int read_opt;
	float orig_timeout;
	float watch_interval;
	nl_recvmsg_msg_cb_t callback;
	char *remaining_header;
	const char *static_header;
	uint8_t nl_cmd;
};

struct nlquery_opts {
	int err;
};

struct ether_addr;

int netlink_create(struct state *state);
void netlink_destroy(struct state *state);

char *netlink_get_info(struct state *state, uint8_t nl_cmd, const char *header);
int translate_mac_netlink(struct state *state, const struct ether_addr *mac,
			  struct ether_addr *mac_out);
int get_nexthop_netlink(struct state *state, const struct ether_addr *mac,
			uint8_t *nexthop, char *ifname);
int get_primarymac_netlink(struct state *state, uint8_t *primarymac);
int get_algoname_netlink(struct state *state, unsigned int mesh_ifindex,
			 char *algoname, size_t algoname_len);

extern struct nla_policy batadv_netlink_policy[];

int missing_mandatory_attrs(struct nlattr *attrs[], const int mandatory[],
			    int num);
int netlink_print_common(struct state *state, char *orig_iface, int read_opt,
			 float orig_timeout, float watch_interval,
			 const char *header, uint8_t nl_cmd,
			 nl_recvmsg_msg_cb_t callback);

int netlink_print_common_cb(struct nl_msg *msg, void *arg);
int netlink_stop_callback(struct nl_msg *msg, void *arg);
int netlink_print_error(struct sockaddr_nl *nla, struct nlmsgerr *nlerr,
			void *arg);
void netlink_print_remaining_header(struct print_opts *opts);

int netlink_query_common(struct state *state,
			 unsigned int mesh_ifindex, uint8_t nl_cmd,
			 nl_recvmsg_msg_cb_t callback,
			 nl_recvmsg_msg_cb_t attribute_cb,
			 int flags, struct nlquery_opts *query_opts);

extern char algo_name_buf[256];
extern int last_err;
extern int64_t mcast_flags;
extern int64_t mcast_flags_priv;

#endif /* _BATCTL_NETLINK_H */
