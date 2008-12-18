/*
 * brmon.c		RTnetlink listener.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Stephen Hemminger <shemminger@osdl.org>
 *
 *              Modified by Srinivas Aji <Aji_Srinivas@emc.com> for use
 *              in RSTP daemon. - 2006-09-01
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_bridge.h>
#include <string.h>

#include "libnetlink.h"

#include "bridge_ctl.h"

static const char SNAPSHOT[] = "v0.1";


/* RFC 2863 operational status */
enum {
	IF_OPER_UNKNOWN,
	IF_OPER_NOTPRESENT,
	IF_OPER_DOWN,
	IF_OPER_LOWERLAYERDOWN,
	IF_OPER_TESTING,
	IF_OPER_DORMANT,
	IF_OPER_UP,
};

/* link modes */
enum {
	IF_LINK_MODE_DEFAULT,
	IF_LINK_MODE_DORMANT,	/* limit upward transition to dormant */
};

static const char *port_states[] = {
	[BR_STATE_DISABLED] = "disabled",
	[BR_STATE_LISTENING] = "listening",
	[BR_STATE_LEARNING] = "learning",
	[BR_STATE_FORWARDING] = "forwarding",
	[BR_STATE_BLOCKING] = "blocking",
};


static int dump_msg(const struct sockaddr_nl *who, struct nlmsghdr *n,
		    void *arg)
{
	FILE *fp = arg;
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr * tb[IFLA_MAX+1];
	int len = n->nlmsg_len;
	char b1[IFNAMSIZ];
	int af_family = ifi->ifi_family;

        if (n->nlmsg_type == NLMSG_DONE)
          return 0;
        
	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0) {
          return -1;
        }
        
#if 0

	if (filter.ifindex && ifi->ifi_index != filter.ifindex)
		return 0;

	if (filter.up && !(ifi->ifi_flags&IFF_UP))
		return 0;
#endif
        if (ifi->ifi_family != AF_BRIDGE && ifi->ifi_family != AF_UNSPEC)
          return 0;

        if (n->nlmsg_type != RTM_NEWLINK &&
            n->nlmsg_type != RTM_DELLINK)
          return 0;

	parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);

        /* Check if we got this from bonding */
        if (tb[IFLA_MASTER] && af_family != AF_BRIDGE)
          return 0;

	if (tb[IFLA_IFNAME] == NULL) {
		fprintf(stderr, "BUG: nil ifname\n");
		return -1;
	}

	if (n->nlmsg_type == RTM_DELLINK)
		fprintf(fp, "Deleted ");

	fprintf(fp, "%d: %s ", ifi->ifi_index,
		tb[IFLA_IFNAME] ? (char*)RTA_DATA(tb[IFLA_IFNAME]) : "<nil>");


	if (tb[IFLA_OPERSTATE]) {
		int state = *(int*)RTA_DATA(tb[IFLA_OPERSTATE]);
		switch (state) {
		case IF_OPER_UNKNOWN: 
			fprintf(fp, "Unknown "); break;
		case IF_OPER_NOTPRESENT:
			fprintf(fp, "Not Present "); break;
		case IF_OPER_DOWN:
			fprintf(fp, "Down "); break;
		case IF_OPER_LOWERLAYERDOWN:
			fprintf(fp, "Lowerlayerdown "); break;
		case IF_OPER_TESTING:
			fprintf(fp, "Testing "); break;
		case IF_OPER_DORMANT:
			fprintf(fp, "Dormant "); break;
		case IF_OPER_UP:
			fprintf(fp, "Up "); break;
		default:
			fprintf(fp, "State(%d) ", state);
		}
	}
	
	if (tb[IFLA_MTU])
		fprintf(fp, "mtu %u ", *(int*)RTA_DATA(tb[IFLA_MTU]));

	if (tb[IFLA_MASTER]) {
		fprintf(fp, "master %s ", 
			if_indextoname(*(int*)RTA_DATA(tb[IFLA_MASTER]), b1));
	}

	if (tb[IFLA_PROTINFO]) {
		uint8_t state = *(uint8_t *)RTA_DATA(tb[IFLA_PROTINFO]);
		if (state <= BR_STATE_BLOCKING)
			fprintf(fp, "state %s", port_states[state]);
		else
			fprintf(fp, "state (%d)", state);
	}


	fprintf(fp, "\n");
	fflush(fp);
        {
          int newlink = (n->nlmsg_type == RTM_NEWLINK);
          int up = 0;
          if (newlink && tb[IFLA_OPERSTATE]) {
            int state = *(uint8_t*)RTA_DATA(tb[IFLA_OPERSTATE]);
            up = (state == IF_OPER_UP) || (state == IF_OPER_UNKNOWN);
          }

          bridge_notify((tb[IFLA_MASTER]?*(int*)RTA_DATA(tb[IFLA_MASTER]):-1), 
                        ifi->ifi_index, newlink, up);
        }
	return 0;
}

#if 0
static void usage(void)
{
	fprintf(stderr, "Usage: brmon\n");
	exit(-1);
}

static int matches(const char *cmd, const char *pattern)
{
	int len = strlen(cmd);
	if (len > strlen(pattern))
		return -1;
	return memcmp(pattern, cmd, len);
}

int
main(int argc, char **argv)
{
	struct rtnl_handle rth;
	unsigned groups = ~RTMGRP_TC;
	int llink = 0;
	int laddr = 0;

	while (argc > 1) {
		if (matches(argv[1], "-Version") == 0) {
			printf("brmon %s\n", SNAPSHOT);
			exit(0);
		} else if (matches(argv[1], "link") == 0) {
			llink=1;
			groups = 0;
		} else if (matches(argv[1], "bridge") == 0) {
			laddr=1;
			groups = 0;
		} else if (strcmp(argv[1], "all") == 0) {
			groups = ~RTMGRP_TC;
		} else if (matches(argv[1], "help") == 0) {
			usage();
		} else {
			fprintf(stderr, "Argument \"%s\" is unknown, try \"rtmon help\".\n", argv[1]);
			exit(-1);
		}
		argc--;	argv++;
	}

	if (llink)
		groups |= RTMGRP_LINK;

	if (rtnl_open(&rth, groups) < 0)
		exit(1);

	if (rtnl_wilddump_request(&rth, PF_BRIDGE, RTM_GETLINK) < 0) {
		perror("Cannot send dump request");
		exit(1);
	}

	if (rtnl_dump_filter(&rth, dump_msg, stdout, NULL, NULL) < 0) {
		fprintf(stderr, "Dump terminated\n");
		return 1;
	}

	if (rtnl_listen(&rth, dump_msg, stdout) < 0)
		exit(2);

	exit(0);
}
#endif

#include "bridge_ctl.h"
#include "epoll_loop.h"

struct rtnl_handle rth;
struct epoll_event_handler br_handler;

struct rtnl_handle rth_state;

void br_ev_handler(uint32_t events, struct epoll_event_handler *h)
{
  if (rtnl_listen(&rth, dump_msg, stdout) < 0) {
    fprintf(stderr, "Error on bridge monitoring socket\n");
    exit(-1);
  }
}

int init_bridge_ops(void)
{
  if (rtnl_open(&rth, ~RTMGRP_TC) < 0) {
    fprintf(stderr, "Couldn't open rtnl socket for monitoring\n");
    return -1;
  }
  
  if (rtnl_open(&rth_state, 0) < 0) {
    fprintf(stderr, "Couldn't open rtnl socket for setting state\n");
    return -1;
  }

  if (rtnl_wilddump_request(&rth, PF_BRIDGE, RTM_GETLINK) < 0) {
    fprintf(stderr, "Cannot send dump request: %m\n");
    return -1;
  }
  
  if (rtnl_dump_filter(&rth, dump_msg, stdout, NULL, NULL) < 0) {
    fprintf(stderr, "Dump terminated\n");
    return -1;
  }

  if (fcntl(rth.fd, F_SETFL, O_NONBLOCK) < 0) {
    fprintf(stderr, "Error setting O_NONBLOCK: %m\n");
    return -1;
  }

  br_handler.fd = rth.fd;
  br_handler.arg = NULL;
  br_handler.handler = br_ev_handler;
  
  if (add_epoll(&br_handler) < 0)
    return -1;
  
  return 0;
}

/* Send message. Response is through bridge_notify */
void bridge_get_configuration(void)
{
  if (rtnl_wilddump_request(&rth, PF_BRIDGE, RTM_GETLINK) < 0) {
    fprintf(stderr, "Cannot send dump request: %m\n");
  }
}
