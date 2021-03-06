// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)


#include <stdint.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/if_bridge.h>
#include <errno.h>

#include "list.h"



#include <stdio.h>
#include <stdbool.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <ev.h>
#include <fcntl.h>
#include <getopt.h>
#include <net/if.h>

#include "cfm_netlink.h"
#include "libnetlink.h"

volatile bool quit = false;

static void handle_signal(int sig)
{
    ev_break(EV_DEFAULT, EVBREAK_ALL);;
}

int signal_init(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);

    return 0;
}

static struct rtnl_handle rth;
static ev_io netlink_watcher;

static int netlink_listen(struct rtnl_ctrl_data *who, struct nlmsghdr *n,
			  void *arg)
{
	struct rtattr *aftb[IFLA_BRIDGE_MAX + 1];
	struct rtattr *infotb[IFLA_BRIDGE_CFM_MEP_STATUS_MAX + 1];
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr *tb[IFLA_MAX + 1];
	int len = n->nlmsg_len;
	struct rtattr *i, *list;
	int rem;
	uint32_t instance;

	if (n->nlmsg_type == NLMSG_DONE)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0) {
		fprintf(stderr, "Message too short!\n");
		return -1;
	}

	if (ifi->ifi_family != AF_BRIDGE)
		return 0;

	if (n->nlmsg_type != RTM_NEWLINK)
		return 0;

	parse_rtattr_flags(tb, IFLA_MAX, IFLA_RTA(ifi), len, NLA_F_NESTED);

	if (tb[IFLA_IFNAME] == NULL) {
		printf("No IFLA_IFNAME\n");
		return -1;
	}

	if (!tb[IFLA_AF_SPEC])
		return 0;

	parse_rtattr_flags(aftb, IFLA_BRIDGE_MAX, RTA_DATA(tb[IFLA_AF_SPEC]), RTA_PAYLOAD(tb[IFLA_AF_SPEC]), NLA_F_NESTED);
	if (!aftb[IFLA_BRIDGE_CFM])
		return 0;

	list = aftb[IFLA_BRIDGE_CFM];
	rem = RTA_PAYLOAD(list);

	printf("CFM CC peer status:\n");
	instance = 0xFFFFFFFF;
	for (i = RTA_DATA(list); RTA_OK(i, rem); i = RTA_NEXT(i, rem)) {
		if (i->rta_type != (IFLA_BRIDGE_CFM_CC_PEER_STATUS_INFO | NLA_F_NESTED))
			continue;

		parse_rtattr_flags(infotb, IFLA_BRIDGE_CFM_CC_PEER_STATUS_MAX, RTA_DATA(i), RTA_PAYLOAD(i), NLA_F_NESTED);
		if (!infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_INSTANCE])
			continue;

		if (instance != rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_INSTANCE])) {
			instance = rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_INSTANCE]);
			printf("Instance %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_INSTANCE]));
		}
		printf("    Peer-mep %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_PEER_MEPID]));
		printf("        CCM defect %u\n", rta_getattr_u32(infotb[IFLA_BRIDGE_CFM_CC_PEER_STATUS_CCM_DEFECT]));
		printf("\n");
	}

	return 0;
}

static void netlink_rcv(EV_P_ ev_io *w, int revents)
{
	rtnl_listen(&rth, netlink_listen, stdout);
}

static int netlink_init(void)
{
	int err;

	err = rtnl_open(&rth, RTMGRP_LINK);
	if (err)
		return err;

	fcntl(rth.fd, F_SETFL, O_NONBLOCK);

	ev_io_init(&netlink_watcher, netlink_rcv, rth.fd, EV_READ);
	ev_io_start(EV_DEFAULT, &netlink_watcher);

	return 0;
}

static void netlink_uninit(void)
{
	ev_io_stop(EV_DEFAULT, &netlink_watcher);
	rtnl_close(&rth);
}

int main (void)
{
	if (netlink_init()) {
		printf("netlink init failed!\n");
		return -1;
	}

	ev_run(EV_DEFAULT, 0);

	netlink_uninit();

	return 0;
}

