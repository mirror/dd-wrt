/* SPDX-License-Identifier: GPL-2.0-only */
/* OpenVPN data channel accelerator
 *
 *  Copyright (C) 2019-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_OVPN_H_
#define _NET_OVPN_DCO_OVPN_H_

#include "main.h"
#include "peer.h"
#include "sock.h"
#include "ovpnstruct.h"

#include <linux/workqueue.h>
#include <linux/types.h>
#include <net/sock.h>

struct ovpn_struct;
struct net_device;

int ovpn_struct_init(struct net_device *dev);

u16 ovpn_select_queue(struct net_device *dev, struct sk_buff *skb,
		      struct net_device *sb_dev);

void ovpn_keepalive_xmit(struct ovpn_peer *peer);
void ovpn_explicit_exit_notify_xmit(struct ovpn_peer *peer);

netdev_tx_t ovpn_net_xmit(struct sk_buff *skb, struct net_device *dev);

int ovpn_recv(struct ovpn_struct *ovpn, struct ovpn_peer *peer, struct sk_buff *skb);

void ovpn_encrypt_work(struct work_struct *work);
void ovpn_decrypt_work(struct work_struct *work);
int ovpn_napi_poll(struct napi_struct *napi, int budget);

int ovpn_send_data(struct ovpn_struct *ovpn, u32 peer_id, const u8 *data, size_t len);

#endif /* _NET_OVPN_DCO_OVPN_H_ */
