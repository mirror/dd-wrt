// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/types.h>
#include <net/if.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#include "libnetlink.h"
#include "server_cmds.h"
#include "state_machine.h"
#include "list.h"
#include "offload.h"

static struct rtnl_handle rth;
static ev_io netlink_watcher;

int CTL_addmrp(int br_index, int ring_nr, int pport, int sport, int ring_role,
	       uint16_t prio, uint8_t ring_recv, uint8_t react_on_link_change,
	       int in_role, uint16_t in_id, int iport)
{
	return mrp_add(br_index, ring_nr, pport, sport, ring_role, prio,
		       ring_recv, react_on_link_change, in_role, in_id,
		       iport);
}

int CTL_delmrp(int br_index, int ring_nr)
{
	return mrp_del(br_index, ring_nr);
}

int CTL_getmrp(int *count, struct mrp_status *status)
{
	return mrp_get(count, status);
}

static int netlink_listen(struct rtnl_ctrl_data *who, struct nlmsghdr *n,
			  void *arg)
{
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr * tb[IFLA_MAX + 1];
	struct mrp_port *port;
	int len = n->nlmsg_len;
	int af_family;

	if (n->nlmsg_type == NLMSG_DONE)
		return 0;

	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0)
		return -1;

	af_family = ifi->ifi_family;

	port = mrp_get_port(ifi->ifi_index);

	if (af_family != AF_BRIDGE && af_family != AF_UNSPEC)
		return 0;

	if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
		return 0;

	parse_rtattr_flags(tb, IFLA_MAX, IFLA_RTA(ifi), len, NLA_F_NESTED);

	if (tb[IFLA_IFNAME] == NULL) {
		printf("BUG: nil ifname\n");
		return -1;
	}

	if (tb[IFLA_ADDRESS]) {
		mrp_mac_change(ifi->ifi_index,
			       (__u8*)RTA_DATA(tb[IFLA_ADDRESS]));
	}

	if (!port)
		return 0;

	if (tb[IFLA_OPERSTATE]) {
		__u8 state = *(__u8*)RTA_DATA(tb[IFLA_OPERSTATE]);
		if (port->operstate != state) {
			port->operstate = state;

			switch (state) {
			case IF_OPER_UNKNOWN:
			case IF_OPER_NOTPRESENT:
			case IF_OPER_DOWN:
			case IF_OPER_LOWERLAYERDOWN:
			case IF_OPER_TESTING:
			case IF_OPER_DORMANT:
				mrp_port_link_change(port, false);
				break;

			case IF_OPER_UP:
				mrp_port_link_change(port, true);
				break;
			}
		}
	}

	if (tb[IFLA_PROTINFO]) {
		struct rtattr *aftb[IFLA_BRPORT_MAX+1];

		parse_rtattr_nested(aftb, IFLA_BRPORT_MAX, tb[IFLA_PROTINFO]);

		if (aftb[IFLA_BRPORT_MRP_RING_OPEN]) {
			mrp_port_ring_open(port,
					   rta_getattr_u8(aftb[IFLA_BRPORT_MRP_RING_OPEN]));
		}

		if (aftb[IFLA_BRPORT_MRP_IN_OPEN]) {
			mrp_port_in_open(port,
					  rta_getattr_u8(aftb[IFLA_BRPORT_MRP_IN_OPEN]));
		}
	}

	if (!tb[IFLA_MASTER]) {
		mrp_destroy(port->mrp->ifindex, port->mrp->ring_nr, false);
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

int CTL_init(void)
{
	if (netlink_init()) {
		printf("netlink init failed!\n");
		return -1;
	}

	if (if_init()) {
		printf("if init failed\n");
		return -1;
	}

	if (mrp_offload_init()) {
		printf("offload init failed\n");
		return -1;
	}

	return 0;
}

void CTL_cleanup(void)
{
	mrp_offload_uninit();
	netlink_uninit();
	mrp_uninit();
}
