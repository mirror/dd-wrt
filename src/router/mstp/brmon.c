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
 * Modified by Vitalii Demianets <vitas@nppfactor.kiev.ua>
 *    for use in MSTP daemon. - 2011-07-18
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <linux/if_bridge.h>

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

static const char *port_states[] =
{
    [BR_STATE_DISABLED] = "disabled",
    [BR_STATE_LISTENING] = "listening",
    [BR_STATE_LEARNING] = "learning",
    [BR_STATE_FORWARDING] = "forwarding",
    [BR_STATE_BLOCKING] = "blocking",
};

static struct rtnl_handle rth;
static struct epoll_event_handler br_handler;

struct rtnl_handle rth_state;

static int dump_msg(const struct sockaddr_nl *who, struct nlmsghdr *n,
                    void *arg)
{
    FILE *fp = arg;
    struct ifinfomsg *ifi = NLMSG_DATA(n);
    struct rtattr * tb[IFLA_MAX + 1];
    int len = n->nlmsg_len;
    char b1[IFNAMSIZ];
    int af_family = ifi->ifi_family;
    bool newlink;
    int br_index;

    if(n->nlmsg_type == NLMSG_DONE)
        return 0;

    len -= NLMSG_LENGTH(sizeof(*ifi));
    if(len < 0)
    {
        return -1;
    }

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
        fprintf(stderr, "BUG: nil ifname\n");
        return -1;
    }

    if(n->nlmsg_type == RTM_DELLINK)
        fprintf(fp, "Deleted ");

    fprintf(fp, "%d: %s ", ifi->ifi_index, (char*)RTA_DATA(tb[IFLA_IFNAME]));

    if(tb[IFLA_OPERSTATE])
    {
        int state = *(int*)RTA_DATA(tb[IFLA_OPERSTATE]);
        switch (state)
        {
            case IF_OPER_UNKNOWN:
                fprintf(fp, "Unknown ");
                break;
            case IF_OPER_NOTPRESENT:
                fprintf(fp, "Not Present ");
                break;
            case IF_OPER_DOWN:
                fprintf(fp, "Down ");
                break;
            case IF_OPER_LOWERLAYERDOWN:
                fprintf(fp, "Lowerlayerdown ");
                break;
            case IF_OPER_TESTING:
                fprintf(fp, "Testing ");
                break;
            case IF_OPER_DORMANT:
                fprintf(fp, "Dormant ");
                break;
            case IF_OPER_UP:
                fprintf(fp, "Up ");
                break;
            default:
                fprintf(fp, "State(%d) ", state);
        }
    }

    if(tb[IFLA_MTU])
        fprintf(fp, "mtu %u ", *(int*)RTA_DATA(tb[IFLA_MTU]));

    if(tb[IFLA_MASTER])
    {
        fprintf(fp, "master %s ",
                if_indextoname(*(int*)RTA_DATA(tb[IFLA_MASTER]), b1));
    }

    if(tb[IFLA_PROTINFO])
    {
        uint8_t state = *(uint8_t *)RTA_DATA(tb[IFLA_PROTINFO]);
        if(state <= BR_STATE_BLOCKING)
            fprintf(fp, "state %s", port_states[state]);
        else
            fprintf(fp, "state (%d)", state);
    }

    fprintf(fp, "\n");
    fflush(fp);

    newlink = (n->nlmsg_type == RTM_NEWLINK);

    if(tb[IFLA_MASTER])
        br_index = *(int*)RTA_DATA(tb[IFLA_MASTER]);
    else if(is_bridge((char*)RTA_DATA(tb[IFLA_IFNAME])))
        br_index = ifi->ifi_index;
    else
        br_index = -1;

    bridge_notify(br_index, ifi->ifi_index, newlink, ifi->ifi_flags);

    return 0;
}

static inline void br_ev_handler(uint32_t events, struct epoll_event_handler *h)
{
    if(rtnl_listen(&rth, dump_msg, stdout) < 0)
    {
        fprintf(stderr, "Error on bridge monitoring socket\n");
        exit(-1);
    }
}

int init_bridge_ops(void)
{
    if(rtnl_open(&rth, RTMGRP_LINK) < 0)
    {
        fprintf(stderr, "Couldn't open rtnl socket for monitoring\n");
        return -1;
    }

    if(rtnl_open(&rth_state, 0) < 0)
    {
        fprintf(stderr, "Couldn't open rtnl socket for setting state\n");
        return -1;
    }

    if(rtnl_wilddump_request(&rth, PF_BRIDGE, RTM_GETLINK) < 0)
    {
        fprintf(stderr, "Cannot send dump request: %m\n");
        return -1;
    }

    if(rtnl_dump_filter(&rth, dump_msg, stdout, NULL, NULL) < 0)
    {
        fprintf(stderr, "Dump terminated\n");
        return -1;
    }

    if(fcntl(rth.fd, F_SETFL, O_NONBLOCK) < 0)
    {
        fprintf(stderr, "Error setting O_NONBLOCK: %m\n");
        return -1;
    }

    br_handler.fd = rth.fd;
    br_handler.arg = NULL;
    br_handler.handler = br_ev_handler;

    if(add_epoll(&br_handler) < 0)
        return -1;

    return 0;
}
