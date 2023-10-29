/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_NETLINK_H_
#define _NET_OVPN_DCO_NETLINK_H_

struct ovpn_struct;
struct ovpn_peer;

int ovpn_netlink_init(struct ovpn_struct *ovpn);
int ovpn_netlink_register(void);
void ovpn_netlink_unregister(void);
int ovpn_netlink_send_packet(struct ovpn_struct *ovpn, const struct ovpn_peer *peer,
			     const u8 *buf, size_t len);
int ovpn_netlink_notify_del_peer(struct ovpn_peer *peer);

#endif /* _NET_OVPN_DCO_NETLINK_H_ */
