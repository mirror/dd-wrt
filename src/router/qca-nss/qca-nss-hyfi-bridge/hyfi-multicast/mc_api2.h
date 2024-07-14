/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

#define NETLINK_QCA_MC            27
#define NETLINK_QCA_MC_EVENT      28

/* netlink message type */
enum
{
    HYFI_SET_MC_ENABLE,
    HYFI_SET_MC_DEBUG,
    HYFI_SET_MC_POLICY,
    HYFI_SET_MC_MEMBERSHIP_INTERVAL,
    HYFI_SET_MC_RETAG,
    HYFI_SET_MC_ROUTER_PORT,
    HYFI_SET_MC_ADD_ACL_RULE,
    HYFI_SET_MC_FLUSH_ACL_RULE,
    HYFI_SET_MC_CONVERT_ALL,
    HYFI_SET_MC_TIMEOUT,
    HYFI_SET_MC_M2I3_FILTER,
    HYFI_SET_MC_TBIT,
    HYFI_SET_MC_LOCAL_QUERY_INTERVAL,
    HYFI_SET_MC_PSW_ENCAP,
    HYFI_SET_MC_PSW_FLOOD,
    HYFI_SET_MC_EVENT_PID,
    HYFI_GET_MC_ACL,
    HYFI_GET_MC_MDB,
    HYFI_SET_MC_ROUTER,
    HYFI_GET_MC_ROUTER_PORT,
    HYFI_SET_MC_MAX_GROUP,
    HYFI_GET_MC_MAX_GROUP,
};

/* netlink event type */
enum
{
    HYFI_EVENT_MC_MDB_UPDATED = 100,
    HYFI_EVENT_MC_MAX,
};

struct __mc_param_value
{
    u_int32_t val;
};

enum
{
    MC_POLICY_DROP = 0,
    MC_POLICY_FLOOD
};

struct __mc_param_retag
{
    u_int32_t enable;
    u_int32_t dscp;
};

enum
{
    MC_RTPORT_FLOOD = 0,
    MC_RTPORT_DROP,
    MC_RTPORT_SPECIFY,
    MC_RTPORT_DEFAULT,
    MC_RTPORT_MAX
};
struct __mc_param_router_port
{
    u_int32_t type;
    u_int32_t ifindex;
};

enum
{
    MC_ACL_RULE_DISABLE = 0,
    MC_ACL_RULE_MULTICAST,
    MC_ACL_RULE_SWM, /* system wide management */
    MC_ACL_RULE_MANAGEMENT,
    MC_ACL_RULE_NON_SNOOPING,
    MC_ACL_RULE_MAX
};
enum
{
    MC_ACL_PATTERN_IGMP = 0,
    MC_ACL_PATTERN_MLD,
    MC_ACL_PATTERN_MAX
};
#define MC_ACL_RULE_MAX_COUNT 8 /* 8 for IGMP, 8 for MLD */
struct __mc_param_pattern
{
    u_int32_t           rule;
    u_int8_t            mac[6];
    u_int8_t            mac_mask[6];
    u_int8_t            ip[16];
    u_int8_t            ip_mask[16];
}__attribute__ ((packed));

struct __mc_param_acl_rule
{
    u_int32_t                   pattern_type; /* IGMP or MLD */
    struct __mc_param_pattern   pattern;
}__attribute__ ((packed));

enum
{
    MC_TIMEOUT_FROM_GROUP_SPECIFIC_QUERIES = 0,
    MC_TIMEOUT_FROM_ALL_SYSTEM_QUERIES,
    MC_TIMEOUT_FROM_GROUP_MEMBERSHIP_INTERVAL
};

struct __mc_param_timeout
{
    u_int32_t           from;
    u_int32_t           enable;
};

#define HYFI_MC_SRCS_MAX      4
#define HYFI_MC_RT_SRCS_MAX   8
#define HYFI_MC_DEV_MAX       16
#define HYFI_MC_IF_MAX        16
#define HYFI_MC_IF_NODE_MAX   8
#define HYFI_MC_GROUP_MAX     256
#define HYFI_MC_GROUP_MIN     16
#define HYFI_MC_IP6_SIZE      16
#define HYFI_MC_INCLUDE       1
#define HYFI_MC_EXCLUDE       2
#define HYFI_MC_EX_SRCS_INVAL 0xff

struct __mc_group
{
    u_int32_t pro;
    union
    {
        u_int32_t       ip4;
        u_int8_t        ip6[HYFI_MC_IP6_SIZE];
    } u;
};

struct __mc_rtport_entry {
    u_int32_t ifindex;
    u_int32_t ipv4;
}__attribute__ ((packed));

struct __mc_mdb_entry
{
    struct __mc_group   group;
    u_int32_t           ifindex;
    u_int32_t           nsrcs;
    u_int8_t            srcs[HYFI_MC_SRCS_MAX * HYFI_MC_IP6_SIZE];
    u_int32_t           aging;
    u_int8_t            filter_mode;
    u_int8_t            fdb_age_out;
    u_int8_t            mac[6];
}__attribute__ ((packed));

struct __mc_encaptbl_dev
{
    u_int8_t            mac[6];
    u_int32_t           in_nsrcs;
    u_int8_t            in_srcs[HYFI_MC_SRCS_MAX * HYFI_MC_IP6_SIZE]; /* include sources list */
    u_int32_t           ex_nsrcs;
    u_int8_t            ex_srcs[HYFI_MC_SRCS_MAX * HYFI_MC_IP6_SIZE]; /* exclude sources list */
}__attribute__ ((packed));

struct __mc_encaptbl_entry
{
    struct __mc_group           group;
    u_int32_t                   dev_cnt;
    struct __mc_encaptbl_dev    dev[HYFI_MC_DEV_MAX];
}__attribute__ ((packed));

struct __mc_floodtbl_entry
{
    struct __mc_group   group;
    u_int32_t           ifcnt;
    u_int32_t           ifindex[HYFI_MC_IF_MAX];
}__attribute__ ((packed));

struct __mc_iftbl_node
{
    u_int8_t            mac[6];
    u_int8_t            filter_mode;
    u_int8_t            nsrcs;
    u_int8_t            srcs[HYFI_MC_SRCS_MAX * HYFI_MC_IP6_SIZE];
};
struct __mc_iftbl_entry
{
    struct __mc_group       group;
    u_int32_t               node_cnt;
    struct __mc_iftbl_node  nodes[HYFI_MC_IF_NODE_MAX];
};

int mc_init_cb( void );
int mc_deinit_cb( void );

#endif
