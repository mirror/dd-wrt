/*-
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: if_ethersubr.h 1441 2006-02-06 16:03:21Z mrenzmann $
 */

#ifndef _NET_IF_ETHERSUBR_H_
#define _NET_IF_ETHERSUBR_H_

#define	ETHER_ADDR_LEN		6	/* length of an Ethernet address */
#define	ETHER_TYPE_LEN		2	/* length of the Ethernet type field */
#define	ETHER_CRC_LEN		4	/* length of the Ethernet CRC */
#define	ETHER_HDR_LEN		(ETHER_ADDR_LEN * 2 + ETHER_TYPE_LEN)
#define	ETHER_MAX_LEN		1518

#define	ETHERMTU	(ETHER_MAX_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)

/*
 * Structure of a 10Mb/s Ethernet header.
 */
struct	ether_header {
	u_char ether_dhost[ETHER_ADDR_LEN];
	u_char ether_shost[ETHER_ADDR_LEN];
	u_short ether_type;
} __packed;

#ifndef ETHERTYPE_PAE
#define	ETHERTYPE_PAE	0x888e		/* EAPOL PAE/802.1x */
#endif
#ifndef ETHERTYPE_IP
#define	ETHERTYPE_IP	0x0800		/* IP protocol */
#endif

/*
 * Structure of a 48-bit Ethernet address.
 */
struct	ether_addr {
	u_char octet[ETHER_ADDR_LEN];
} __packed;

#define	ETHER_IS_MULTICAST(addr) (*(addr) & 0x01) /* is address mcast/bcast? */

#define VLAN_PRI_SHIFT	13		/* Shift to find VLAN user priority */
#define VLAN_PRI_MASK	7		/* Mask for user priority bits in VLAN */


#endif /* _NET_IF_ETHERSUBR_H_ */
