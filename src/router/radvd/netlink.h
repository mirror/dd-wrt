/*
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>
 *    Reuben Hawkins		<reubenhwk@gmail.com>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <reubenhwk@gmail.com>.
 *
 */

#pragma once

#include "radvd.h"

int netlink_get_address_lifetimes(struct AdvPrefix const *prefix, unsigned int *preferred_lft, unsigned int *valid_lft);
int netlink_get_device_addr_len(struct Interface *iface);
void process_netlink_msg(int netlink_sock, struct Interface *ifaces, int icmp_sock);
int netlink_socket(void);
int prefix_match (struct AdvPrefix const *prefix, struct in6_addr *addr);
