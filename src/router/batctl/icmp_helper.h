/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#ifndef _BATCTL_ICMP_HELPER_H
#define _BATCTL_ICMP_HELPER_H

#include "main.h"

#include <net/ethernet.h>
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <stddef.h>
#include <stdint.h>

#include "batadv_packet.h"
#include "list.h"

struct timeval;

struct icmp_interface {
	char name[IFNAMSIZ];
	uint8_t mac[ETH_ALEN];

	int sock;

	int mark;
	struct list_head list;
};

int icmp_interfaces_init(void);
int icmp_interface_write(struct state *state,
			 struct batadv_icmp_header *icmp_packet, size_t len);
void icmp_interfaces_clean(void);
ssize_t icmp_interface_read(struct batadv_icmp_header *icmp_packet, size_t len,
			    struct timeval *tv);

#endif
