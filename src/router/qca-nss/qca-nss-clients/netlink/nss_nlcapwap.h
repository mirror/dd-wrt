/*
 **************************************************************************
 * Copyright (c) 2014-2015, 2018-2020 The Linux Foundation. All rights reserved.
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
 * nss_nlcapwap.h
 *	NSS Netlink Capwap API definitions
 */
#ifndef __NSS_NLCAPWAP_H
#define __NSS_NLCAPWAP_H

#define NSS_NLCAPWAP_IP_VERS_4 4			/* IP version 4 */
#define NSS_NLCAPWAP_IP_VERS_6 6			/* IP version 6 */
#define NSS_NLCAPWAP_VLAN_TAG_INVALID 0xFFF		/* Invalid vlan tag */
#define NSS_NLCAPWAP_WAN_IFNUM 0			/* WAN interface number */
#define NSS_NLCAPWAP_DATA 0xcc				/* Dummy data */
#define NSS_NLCAPWAP_KALIVE_PAYLOAD_SZ 64		/* Capwap+Dtls keepalive packet size */
#define NSS_NLCAPWAP_KALIVE_TIMER_MSECS 1000		/* Capwap+Dtls keepalive timer */
#define NSS_NLCAPWAP_STATS_MAX 5			/* Maximum number of stats per tunnel */
#define NSS_NLCAPWAP_MAX_STR_LEN 96			/* Maximum length of stats string */
#define NSS_NLCAPWAP_MAX_HEADROOM 128	/* Maximum headroom needed */
#define NSS_NLCAPWAP_MAX_TAILROOM 192	/* Maximum tailroom needed */
#define NSS_NLCAPWAP_MAX_TUNNEL_LONGS BITS_TO_LONGS(NSS_CAPWAPMGR_MAX_TUNNELS)
/*
 * nss_nlcapwap_meta_header_type
 *	Capwap meta header type
 */
enum nss_nlcapwap_meta_header_type {
	NSS_NLCAPWAP_META_HEADER_TYPE_UNKNOWN = -1,	/* Unknown meta header type */
	NSS_NLCAPWAP_META_HEADER_TYPE_IPV4_DATA,	/* capwap meta header type ipv4 */
	NSS_NLCAPWAP_META_HEADER_TYPE_EAPOL,		/* capwap meta header type eapol */
	NSS_NLCAPWAP_META_HEADER_TYPE_MAX		/* Max meta header type */
};

#if (CONFIG_NSS_NLCAPWAP == 1)
bool nss_nlcapwap_init(void);
bool nss_nlcapwap_exit(void);
#define NSS_NLCAPWAP_INIT nss_nlcapwap_init
#define NSS_NLCAPWAP_EXIT nss_nlcapwap_exit
#else
#define NSS_NLCAPWAP_INIT 0
#define NSS_NLCAPWAP_EXIT 0
#endif /* !CONFIG_NSS_NLCAPWAP */

#endif /* __NSS_NLCAPWAP_H */
