/*
 *   Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License v2 as published by
 *   the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 */
#ifndef __RELAYD_H
#define __RELAYD_H

#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <linux/if_packet.h>
#include <linux/rtnetlink.h>
#include <linux/neighbour.h>

#include <stdint.h>
#include <stdbool.h>

#include "uloop.h"
#include "list.h"

#define DEBUG
#ifdef DEBUG
#define DPRINTF(level, ...) if (debug >= level) fprintf(stderr, __VA_ARGS__);
#else
#define DPRINTF(...) do {} while(0)
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#define __uc(c) ((unsigned char *)(c))

#define MAC_FMT	"%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_BUF(_c) __uc(_c)[0], __uc(_c)[1], __uc(_c)[2], __uc(_c)[3], __uc(_c)[4], __uc(_c)[5]

#define IP_FMT	"%d.%d.%d.%d"
#define IP_BUF(_c) __uc(_c)[0], __uc(_c)[1], __uc(_c)[2], __uc(_c)[3]

#define DUMMY_IP ((uint8_t *) "\x01\x01\x01\x01")

#define DHCP_FLAG_BROADCAST	(1 << 15)

struct relayd_interface {
	struct list_head list;
	struct uloop_fd fd;
	struct uloop_fd bcast_fd;
	struct sockaddr_ll sll;
	struct sockaddr_ll bcast_sll;
	char ifname[IFNAMSIZ];
	struct list_head hosts;
	uint8_t src_ip[4];
	bool managed;
	int rt_table;
};

struct relayd_host {
	struct list_head list;
	struct list_head routes;
	struct relayd_interface *rif;
	uint8_t lladdr[ETH_ALEN];
	uint8_t ipaddr[4];
	struct uloop_timeout timeout;
	int cleanup_pending;
};

struct relayd_route {
	struct list_head list;
	uint8_t dest[4];
	uint8_t mask;
};

struct arp_packet {
	struct ether_header eth;
	struct ether_arp arp;
} __packed;

struct rtnl_req {
	struct nlmsghdr nl;
	struct rtmsg rt;
} __packed;

extern struct list_head interfaces;
extern int debug;
extern int route_table;
extern uint8_t local_addr[4];
extern int local_route_table;

void rtnl_route_set(struct relayd_host *host, struct relayd_route *route, bool add);

static inline void relayd_add_route(struct relayd_host *host, struct relayd_route *route)
{
	rtnl_route_set(host, route, true);
}

static inline void relayd_del_route(struct relayd_host *host, struct relayd_route *route)
{
	rtnl_route_set(host, route, false);
}

void relayd_add_interface_routes(struct relayd_interface *rif);
void relayd_del_interface_routes(struct relayd_interface *rif);

int relayd_rtnl_init(void);
void relayd_rtnl_done(void);

struct relayd_host *relayd_refresh_host(struct relayd_interface *rif,
					const uint8_t *lladdr,
					const uint8_t *ipaddr);
void relayd_add_host_route(struct relayd_host *host, const uint8_t *ipaddr, uint8_t mask);
void relayd_add_pending_route(const uint8_t *gateway, const uint8_t *dest, uint8_t mask, int timeout);

void relayd_forward_bcast_packet(struct relayd_interface *from_rif, void *packet, int len);
bool relayd_handle_dhcp_packet(struct relayd_interface *rif, void *data, int len, bool forward);

#endif
