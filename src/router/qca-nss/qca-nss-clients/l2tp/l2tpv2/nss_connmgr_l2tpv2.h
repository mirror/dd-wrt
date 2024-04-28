/*
 **************************************************************************
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_connnmgr_l2tpv2.h
 *
 *Header file implementation of nss_connnmgr_l2tpv2.c
 */

#ifndef _NSS_CONNMGR_L2TP_V2_H_
#define _NSS_CONNMGR_L2TP_V2_H_

#include <linux/l2tp.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/types.h>

#define L2TP_V_2 2

#define tunnel_hold(tunnel) refcount_inc(&tunnel->ref_count)
#define tunnel_put(tunnel)  refcount_dec(&tunnel->ref_count)
#define session_hold(session) refcount_inc(&session->ref_count)
#define session_put(session)  refcount_dec(&session->ref_count)

 /*
  *		----------------------------------------------------------------------------------
  *		|		|		|		|		|		 |
  *		| ip header	| udp header	| l2tpv2 header	|ppp header	| ppp payload    |
  *		|		|		|		|		|  (IP data gram)|
  *		|		|		|		|		|		 |
  *		|---------------------------------------------------------------------------------
  *             L2TV2 & L2TPV3 over UDP
  */
struct session_info {
	u32 session_id, peer_session_id;	/* local & remote session id */
	u16 offset;				/* offset to data */
	u16 hdr_len;				/* header length */
	int reorder_timeout;			/* reorder timeout */
	unsigned send_seq:1;			/* enable tx sequence number ?  */
	unsigned recv_seq:1;			/* enable rx sequence number ?  */
};

struct tunnel_info {
	u32 tunnel_id, peer_tunnel_id;		/* local and remote tunnel id */
	u8 udp_csum;				/* enable udp checksum ? */
};

struct udp_info {
	__be16 dport, sport;

};

struct ip_info {
		struct {
			struct in_addr saddr, daddr;
		} v4;
};

struct l2tpv2_info {
	struct tunnel_info tunnel;
	struct session_info session;
};

struct nss_connmgr_l2tpv2_data {
	struct ip_info ip;
	struct udp_info udp;
	struct l2tpv2_info l2tpv2;
};

struct nss_connmgr_l2tpv2_session_data {
	struct nss_connmgr_l2tpv2_data data;
	struct net_device *dev;
	struct hlist_node hash_list;
};

extern int nss_connmgr_l2tpv2_get_data(struct net_device *dev, struct nss_connmgr_l2tpv2_data *data);
extern int nss_connmgr_l2tpv2_does_connmgr_track(const struct net_device *dev);

#endif
