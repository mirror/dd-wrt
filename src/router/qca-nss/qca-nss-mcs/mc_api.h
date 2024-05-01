/*
 * Copyright (c) 2012, 2015 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _MC_API_H
#define _MC_API_H

#include <linux/types.h>

#define NETLINK_QCA_MC		  23

#ifndef IFNAMSIZ
#define IFNAMSIZ	(16)
#endif

/* netlink message type */
enum {
	MC_MSG_SET_ENABLE,
	MC_MSG_SET_DEBUG,
	MC_MSG_SET_POLICY,
	MC_MSG_SET_MEMBERSHIP_INTERVAL,
	MC_MSG_SET_RETAG,
	MC_MSG_SET_ROUTER_PORT,
	MC_MSG_SET_ADD_ACL_RULE,
	MC_MSG_SET_FLUSH_ACL_RULE,
	MC_MSG_SET_CONVERT_ALL,
	MC_MSG_SET_TIMEOUT,
	MC_MSG_SET_M2I3_FILTER,
	MC_MSG_SET_TBIT,
	MC_MSG_SET_LOCAL_QUERY_INTERVAL,
	MC_MSG_SET_PSW_ENCAP,
	MC_MSG_SET_PSW_FLOOD,
	MC_MSG_SET_EVENT_PID,
	MC_MSG_GET_ACL,
	MC_MSG_GET_MDB,
	MC_MSG_SET_ROUTER,
	MC_MSG_GET_ROUTER_PORT,
	MC_MSG_MAX
};

/* netlink event type */
enum {
	MC_EVENT_MDB_UPDATED = 100,
	MC_EVENT_MAX,
};

struct __mcctl_msg_header {
	char if_name[IFNAMSIZ];	/* bridge name: br-lan */
	u_int32_t buf_len;	/* not include this header */
	u_int32_t tbl_offsite;	/* how many entries to skip */
	u_int32_t status;
	u_int32_t bytes_written;
	u_int32_t bytes_needed;

} __attribute__ ((packed));

#define MC_MSG_HDRLEN		sizeof(struct __mcctl_msg_header)
#define MC_MSG_DATA(p)		((void *)(((char *)p) + NLMSG_LENGTH(0) + MC_MSG_HDRLEN))

/* define for the status field */
enum {
	MC_STATUS_SUCCESS = 0,
	MC_STATUS_NOT_SUPPORTED = 1,
	MC_STATUS_RESOURCES = 2,
	MC_STATUS_INVALID_PARAMETER = 3,
	MC_STATUS_BUFFER_OVERFLOW = 4,
	MC_STATUS_FAILURE = 5,
	MC_STATUS_NOT_FOUND = 6,
};

struct __mc_param_value {
	u_int32_t val;
};

struct __event_info {
	u_int32_t event_pid;
};

enum {
	MC_POLICY_DROP = 0,
	MC_POLICY_FLOOD
};

struct __mc_param_retag {
	u_int32_t enable;
	u_int32_t dscp;
};

enum {
	MC_RTPORT_FLOOD = 0,
	MC_RTPORT_DROP,
	MC_RTPORT_SPECIFY,
	MC_RTPORT_DEFAULT,
	MC_RTPORT_MAX
};

struct __mc_param_router_port {
	u_int32_t type;
	u_int32_t ifindex;
};

enum {
	MC_ACL_RULE_DISABLE = 0,
	MC_ACL_RULE_MULTICAST,
	MC_ACL_RULE_SWM,	/* system wide management */
	MC_ACL_RULE_MANAGEMENT,
	MC_ACL_RULE_NON_SNOOPING,
	MC_ACL_RULE_MAX
};

enum {
	MC_ACL_PATTERN_IGMP = 0,
	MC_ACL_PATTERN_MLD,
	MC_ACL_PATTERN_MAX
};

#define MC_ACL_RULE_MAX_COUNT 8	/* 8 for IGMP, 8 for MLD */
struct __mc_param_pattern {
	u_int32_t rule;
	u_int8_t mac[6];
	u_int8_t mac_mask[6];
	u_int8_t ip[16];
	u_int8_t ip_mask[16];
} __attribute__ ((packed));

struct __mc_param_acl_rule {
	u_int32_t pattern_type;	/* IGMP or MLD */
	struct __mc_param_pattern pattern;
} __attribute__ ((packed));

enum {
	MC_TIMEOUT_FROM_GROUP_SPECIFIC_QUERIES = 0,
	MC_TIMEOUT_FROM_ALL_SYSTEM_QUERIES,
	MC_TIMEOUT_FROM_GROUP_MEMBERSHIP_INTERVAL
};

struct __mc_param_timeout {
	u_int32_t from;
	u_int32_t enable;
};

#define MC_DEF_SRCS_MAX		 4
#define MC_DEF_RT_SRCS_MAX	 8
#define MC_DEF_DEV_MAX		 16
#define MC_DEF_IF_MAX		 16
#define MC_DEF_IF_NODE_MAX	 8
#define MC_DEF_GROUP_MAX	 256
#define MC_DEF_IP6_SIZE		 16
#define MC_DEF_FILTER_INCLUDE  1
#define MC_DEF_FILTER_EXCLUDE  2
#define MC_DEF_EX_SRCS_INVAL 0xff

struct __mc_group {
	u_int32_t pro;
	union {
		u_int32_t ip4;
		u_int8_t ip6[MC_DEF_IP6_SIZE];
	} u;
};

struct __mc_rtport_entry {
	u_int32_t ifindex;
	u_int32_t ipv4;
} __attribute__ ((packed));

struct __mc_mdb_entry {
	struct __mc_group group;
	u_int32_t ifindex;
	u_int32_t nsrcs;
	u_int8_t srcs[MC_DEF_SRCS_MAX * MC_DEF_IP6_SIZE];
	u_int32_t aging;
	u_int8_t filter_mode;
	u_int8_t fdb_age_out;
	u_int8_t mac[6];
} __attribute__ ((packed));

struct __mc_encaptbl_dev {
	u_int8_t mac[6];
	u_int32_t in_nsrcs;
	u_int8_t in_srcs[MC_DEF_SRCS_MAX * MC_DEF_IP6_SIZE];	/* include sources list */
	u_int32_t ex_nsrcs;
	u_int8_t ex_srcs[MC_DEF_SRCS_MAX * MC_DEF_IP6_SIZE];	/* exclude sources list */
} __attribute__ ((packed));

struct __mc_encaptbl_entry {
	struct __mc_group group;
	u_int32_t dev_cnt;
	struct __mc_encaptbl_dev dev[MC_DEF_DEV_MAX];
} __attribute__ ((packed));

struct __mc_floodtbl_entry {
	struct __mc_group group;
	u_int32_t ifcnt;
	u_int32_t ifindex[MC_DEF_IF_MAX];
} __attribute__ ((packed));

struct __mc_iftbl_node {
	u_int8_t mac[6];
	u_int8_t filter_mode;
	u_int8_t nsrcs;
	u_int8_t srcs[MC_DEF_SRCS_MAX * MC_DEF_IP6_SIZE];
};
struct __mc_iftbl_entry {
	struct __mc_group group;
	u_int32_t node_cnt;
	struct __mc_iftbl_node nodes[MC_DEF_IF_NODE_MAX];
};

int mc_init_cb(void);
int mc_deinit_cb(void);
#endif
