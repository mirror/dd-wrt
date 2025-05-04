/*
 * brmon.c      RTnetlink listener.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 * Authors: Stephen Hemminger <shemminger@osdl.org>
 * Modified by Srinivas Aji <Aji_Srinivas@emc.com>
 *    for use in RSTP daemon. - 2006-09-01
 * Modified by Vitalii Demianets <dvitasgs@gmail.com>
 *    for use in MSTP daemon. - 2011-07-18
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <linux/if_bridge.h>

#include "log.h"
#include "libnetlink.h"
#include "bridge_ctl.h"
#include "netif_utils.h"
#include "epoll_loop.h"

/* RFC 2863 operational status */
enum
{
    IF_OPER_UNKNOWN,
    IF_OPER_NOTPRESENT,
    IF_OPER_DOWN,
    IF_OPER_LOWERLAYERDOWN,
    IF_OPER_TESTING,
    IF_OPER_DORMANT,
    IF_OPER_UP,
};

/* link modes */
enum
{
    IF_LINK_MODE_DEFAULT,
    IF_LINK_MODE_DORMANT, /* limit upward transition to dormant */
};

static struct rtnl_handle rth;
static struct epoll_event_handler nl_handler;

struct rtnl_handle rth_state;

static int dump_br_msg(const struct sockaddr_nl *who, struct nlmsghdr *n,
                       void *arg)
{
    struct ifinfomsg *ifi = NLMSG_DATA(n);
    struct rtattr * tb[IFLA_MAX + 1];
    int len = n->nlmsg_len;
    char b1[IFNAMSIZ];
    int af_family;
    bool newlink;
    int br_index;

    if(n->nlmsg_type == NLMSG_DONE)
        return 0;

    len -= NLMSG_LENGTH(sizeof(*ifi));
    if(len < 0)
    {
        return -1;
    }

    af_family = ifi->ifi_family;

    if(af_family != AF_BRIDGE && af_family != AF_UNSPEC)
        return 0;

    if(n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
        return 0;

    parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);

    /* Check if we got this from bonding */
    if(tb[IFLA_MASTER] && af_family != AF_BRIDGE)
        return 0;

    if(tb[IFLA_IFNAME] == NULL)
    {
        ERROR("BUG: nil ifname\n");
        return -1;
    }

    if(n->nlmsg_type == RTM_DELLINK)
        LOG("Deleted ");

    LOG("%d: %s ", ifi->ifi_index, (char*)RTA_DATA(tb[IFLA_IFNAME]));

    if(tb[IFLA_OPERSTATE])
    {
        __u8 state = *(__u8*)RTA_DATA(tb[IFLA_OPERSTATE]);
        switch (state)
        {
            case IF_OPER_UNKNOWN:
                LOG("Unknown ");
                break;
            case IF_OPER_NOTPRESENT:
                LOG("Not Present ");
                break;
            case IF_OPER_DOWN:
                LOG("Down ");
                break;
            case IF_OPER_LOWERLAYERDOWN:
                LOG("Lowerlayerdown ");
                break;
            case IF_OPER_TESTING:
                LOG("Testing ");
                break;
            case IF_OPER_DORMANT:
                LOG("Dormant ");
                break;
            case IF_OPER_UP:
                LOG("Up ");
                break;
            default:
                LOG("State(%d) ", state);
        }
    }

    if(tb[IFLA_MTU])
        LOG("mtu %u ", *(int*)RTA_DATA(tb[IFLA_MTU]));

    if(tb[IFLA_MASTER])
    {
        LOG("master %s ",
                if_indextoname(*(int*)RTA_DATA(tb[IFLA_MASTER]), b1));
    }

    if(tb[IFLA_PROTINFO])
    {
        uint8_t state = *(uint8_t *)RTA_DATA(tb[IFLA_PROTINFO]);
        if(state <= BR_STATE_BLOCKING)
            LOG("state %s", stp_state_name(state));
        else
            LOG("state (%d)", state);
    }

    newlink = (n->nlmsg_type == RTM_NEWLINK);

    if(tb[IFLA_MASTER])
        br_index = *(int*)RTA_DATA(tb[IFLA_MASTER]);
    else if(is_bridge((char*)RTA_DATA(tb[IFLA_IFNAME])))
        br_index = ifi->ifi_index;
    else
        br_index = -1;

    if(br_index >= 0 && tb[IFLA_LINKINFO])
    {
        struct rtattr *tbli[__IFLA_INFO_MAX + 1];
        char *kind = NULL;

        parse_rtattr_nested(tbli, IFLA_INFO_MAX, tb[IFLA_LINKINFO]);
        if (tbli[IFLA_INFO_KIND])
        {
            kind = (char *)RTA_DATA(tbli[IFLA_INFO_KIND]);
        }

        if (kind && !strcmp("bridge", kind) && tbli[IFLA_INFO_DATA])
        {
            struct rtattr *tbbr[__IFLA_BR_MAX + 1];

            parse_rtattr_nested(tbbr, __IFLA_BR_MAX, tbli[IFLA_INFO_DATA]);

            if (tbbr[IFLA_BR_MULTI_BOOLOPT])
            {
                struct br_boolopt_multi *bm;
                bool mst_en;

                bm = (struct br_boolopt_multi *)RTA_DATA(tbbr[IFLA_BR_MULTI_BOOLOPT]);
                mst_en = !!(bm->optval & (1u << BR_BOOLOPT_MST_ENABLE));

                bridge_mst_notify(br_index, mst_en);
            }

        }
    }

    bridge_notify(br_index, ifi->ifi_index, newlink, ifi->ifi_flags);

    return 0;
}

static int dump_vlan_msg(const struct sockaddr_nl *who, struct nlmsghdr *n,
                    void *arg)
{
    struct br_vlan_msg *bvm = NLMSG_DATA(n);
    int len = n->nlmsg_len;
    struct rtattr *pos;
    int ifindex;

    bool newvlan = n->nlmsg_type == RTM_NEWVLAN;

    len -= NLMSG_LENGTH(sizeof(*bvm));
    if(len < 0)
        return -1;

    if (bvm->family != AF_BRIDGE)
        return 0;

    ifindex = bvm->ifindex;

    LOG("%d: %sVLAN ", ifindex, (newvlan ? "NEW" : "DEL"));

    for(pos = BRVLAN_RTA(bvm); RTA_OK(pos, len); pos = RTA_NEXT(pos, len))
    {
        struct rtattr *tb[BRIDGE_VLANDB_ENTRY_MAX + 1];
        struct bridge_vlan_info *info = NULL;
        __u16 i, range = 0;
	
        if((pos->rta_type & NLA_TYPE_MASK) != BRIDGE_VLANDB_ENTRY)
	  continue;

        parse_rtattr_nested(tb, BRIDGE_VLANDB_ENTRY_MAX, pos);

        if (tb[BRIDGE_VLANDB_ENTRY_INFO])
            info = RTA_DATA(tb[BRIDGE_VLANDB_ENTRY_INFO]);
        if (tb[BRIDGE_VLANDB_ENTRY_RANGE])
            range = *(__u16 *)RTA_DATA(tb[BRIDGE_VLANDB_ENTRY_RANGE]);

        if(!info)
        {
            ERROR("BUG: nil bridge_vlan_info\n");
            continue;
        }

        LOG("%d: info->vid %i, range %i", ifindex, info->vid, range);

        if(!range)
            range = info->vid;

        for(i = info->vid; i <= range; i++)
            bridge_vlan_notify(ifindex, newvlan, i);
    }

    return 0;
}

static int dump_msg(const struct sockaddr_nl *who, struct nlmsghdr *n,
                    void *arg)
{
    switch (n->nlmsg_type)
    {
        case RTM_NEWLINK:
        case RTM_DELLINK:
            return dump_br_msg(who, n, arg);
        case RTM_NEWVLAN:
        case RTM_DELVLAN:
            return dump_vlan_msg(who, n, arg);
        default:
            return 0;
    }
}

static inline void nl_ev_handler(uint32_t events, struct epoll_event_handler *h)
{
    if(rtnl_listen(&rth, dump_msg, stdout) < 0)
    {
        ERROR("Error on bridge monitoring socket\n");
    }
}

int init_netlink_ops(void)
{
    bool have_vlandb = false;

    if(rtnl_open(&rth, RTMGRP_LINK) < 0)
    {
        ERROR("Couldn't open rtnl socket for monitoring\n");
        return -1;
    }

    if(rtnl_open(&rth_state, 0) < 0)
    {
        ERROR("Couldn't open rtnl socket for setting state\n");
        return -1;
    }

    if(rtnl_add_nl_group(&rth, RTNLGRP_BRVLAN) < 0)
    {
        ERROR("Couldn't join RTNLGRP_BRVLAN\n");
    }
    else
    {
        have_vlandb = true;
    }

    if(rtnl_wilddump_request(&rth, PF_BRIDGE, RTM_GETLINK) < 0)
    {
        ERROR("Cannot send dump request: %m\n");
        return -1;
    }

    if(rtnl_dump_filter(&rth, dump_msg, stdout, NULL, NULL) < 0)
    {
        ERROR("Dump terminated\n");
        return -1;
    }

    if(have_vlandb)
    {
        if(rtnl_wilddump_request(&rth, PF_BRIDGE, RTM_GETVLAN) < 0)
        {
            ERROR("Cannot send vlandb dump request: %m\n");
            return -1;
        }

        if(rtnl_dump_filter(&rth, dump_msg, stdout, NULL, NULL) < 0)
        {
            ERROR("Vlandb dump terminated\n");
            return -1;
        }
    }

    if(fcntl(rth.fd, F_SETFL, O_NONBLOCK) < 0)
    {
        ERROR("Error setting O_NONBLOCK: %m\n");
        return -1;
    }

    nl_handler.fd = rth.fd;
    nl_handler.arg = NULL;
    nl_handler.handler = nl_ev_handler;

    if(add_epoll(&nl_handler) < 0)
        return -1;

    return 0;
}
