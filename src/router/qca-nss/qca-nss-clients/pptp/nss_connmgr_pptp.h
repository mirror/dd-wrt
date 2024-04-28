/*
 **************************************************************************
 * Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
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
 * nss_connnmgr_pptp.h
 *	NSS PPTP client definitions
 */

#ifndef _NSS_CONNMGR_PPTP_H_
#define _NSS_CONNMGR_PPTP_H_

 /*
  *		------------------------------------------------------------------
  *		|		|		|		|		 |
  *		| ip header	| GRE header	|ppp header	| ppp payload    |
  *		|		|		|		|  (IP data gram)|
  *		|		|		|		|		 |
  *		|-----------------------------------------------------------------
  *             PPTP over GRE
  */

#define NSS_PPTP_GRE_PROTO	0x880B
#define NSS_PPTP_GRE_VER	0x1
#define NSS_PPTP_GRE_HAS_ACK	0x80
#define NSS_PPTP_GRE_HAS_SEQ	0x10

/*
 * GRE Header Structure
 */
struct nss_pptp_gre_hdr {
	uint8_t  flags;
	uint8_t  flags_ver;
	uint16_t protocol;
	uint16_t payload_len;
	uint16_t call_id;
	uint32_t seq;
	uint32_t ack;
};

/*
 * Structure for PPTP client driver session info
 */
struct nss_connmgr_pptp_session_info {
	uint32_t src_call;
	uint32_t dst_call;
	uint32_t src_ip;
	uint32_t dst_ip;
};

/*
 * Structure for PPTP session entry into HASH table
 */
struct nss_connmgr_pptp_session_entry {
	struct nss_connmgr_pptp_session_info data;
	struct net_device *dev;
	struct net_device *phy_dev;
	struct hlist_node hash_list;
};

#endif
