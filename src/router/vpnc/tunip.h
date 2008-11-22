/* IPSec ESP and AH support.
   Copyright (C) 2005 Maurice Massar

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: tunip.h 312 2008-06-15 18:09:42Z Joerg Mayer $
*/

#ifndef __TUNIP_H__
#define __TUNIP_H__

#include "isakmp.h"

#include <time.h>
#include <net/if.h>

struct lifetime {
	time_t   start;
	uint32_t seconds;
	uint32_t kbytes;
	uint32_t rx;
	uint32_t tx;
};

struct ike_sa {
	uint32_t spi;
	uint32_t seq_id; /* for replay protection (not implemented) */
	
	uint8_t *key;
	uint8_t *key_cry;
	gcry_cipher_hd_t cry_ctx;
	uint8_t *key_md;
	
	/* Description of the packet being processed */
	unsigned char *buf;
	unsigned int bufsize, bufpayload, var_header_size;
	int buflen;
};

struct encap_method; /* private to tunip.c */

enum natt_active_mode_enum{
	NATT_ACTIVE_NONE,
	NATT_ACTIVE_CISCO_UDP, /* isakmp and esp on different ports => never encap */
	NATT_ACTIVE_DRAFT_OLD, /* as in natt-draft 0 and 1 */
	NATT_ACTIVE_RFC        /* draft 2 and RFC3947 / RFC3948 */
};

struct sa_block {
	const char *pidfile;
	
	int tun_fd; /* fd to host via tun/tap */
	char tun_name[IFNAMSIZ];
	uint8_t tun_hwaddr[ETH_ALEN];
	
	struct in_addr dst; /* ip of concentrator, must be set */
	struct in_addr src; /* local ip, from getsockname() */
	
	struct in_addr opt_src_ip; /* configured local ip, can be 0.0.0.0 */
	
	/* these sockets are connect()ed */
	int ike_fd; /* fd over isakmp traffic, and in case of NAT-T esp too */
	int esp_fd; /* raw socket for ip-esp or Cisco-UDP or ike_fd (NAT-T) */
	
	struct {
		int timeout;
		uint8_t *resend_hash;
		uint16_t src_port, dst_port;
		uint8_t i_cookie[ISAKMP_COOKIE_LENGTH];
		uint8_t r_cookie[ISAKMP_COOKIE_LENGTH];
		uint8_t *key; /* ike encryption key */
		size_t keylen;
		uint8_t *initial_iv;
		uint8_t *skeyid_a;
		uint8_t *skeyid_d;
		int auth_algo; /* PSK, PSK+Xauth, Hybrid ToDo: Cert/... */
		int cry_algo, md_algo;
		size_t ivlen, md_len;
		uint8_t current_iv_msgid[4];
		uint8_t *current_iv;
		struct lifetime life;
		int do_dpd;
		int dpd_idle;
		uint32_t dpd_seqno;
		uint32_t dpd_seqno_ack;
		time_t dpd_sent;
		unsigned int dpd_attempts;
	} ike;
	uint8_t our_address[4], our_netmask[4];
	struct {
		int do_pfs;
		int cry_algo, md_algo;
		size_t key_len, md_len;
		size_t blk_len, iv_len;
		uint16_t encap_mode;
		uint16_t peer_udpencap_port;
		enum natt_active_mode_enum natt_active_mode;
		struct lifetime life;
		struct ike_sa rx, tx;
		struct encap_method *em;
		uint16_t ip_id;
	} ipsec;
};

extern int volatile do_kill;
extern void vpnc_doit(struct sa_block *s);

#endif
