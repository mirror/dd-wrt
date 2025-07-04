/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* \summary: Broadcom Ethernet switches tag (4 bytes) printer */

#include <config.h>

#include "netdissect-stdinc.h"

#define ND_LONGJMP_FROM_TCHECK
#include "netdissect.h"
#include "addrtoname.h"
#include "extract.h"

#define ETHER_TYPE_LEN		2

#define QCA_TAG_LEN		2

static const struct tok qca_tag_type_values[] = {
	{ 0, "Normal" },
	{ 1, "MiB" },
	{ 2, "Ack"},
	{ 3, "802.1X"},
	{ 4, "Reserved ARL"},
	{ 5, "RIPv1"},
	{ 6, "DHCP"},
	{ 7, "PPPoE Discovery"},
	{ 8, "ARP"},
	{ 9, "Reserved RARP"},
	{ 0xa, "IGMP"},
	{ 0xb, "MLD"},
	{ 0xc, "Reserved Neighbor Discovery"},
	{ 0xd, "Redirect to CPU"},
	{ 0xe, "Normalization"},
	{ 0xf, "Learn Limit"},
	{ 0x10, "IPv4 NAT to CPU"},
	{ 0x11, "SIP not Found"},
	{ 0x12, "NAT not Found"},
	{ 0x13, "APP not Found"},
	{ 0x14, "Routing not Found"},
	{ 0x15, "TTL Exceed"},
	{ 0x16, "MTU Exceed"},
	{ 0x17, "Copy to CPU"},
	{ 0x18, "Mirror to CPU"},
	{ 0x19, "Flooding to CPU"},
	{ 0x1a, "Forwarding to CPU"},
	{ 0, NULL }
};


static void
qca_tag_print(netdissect_options *ndo, const u_char *bp)
{
	uint16_t tag;

	tag = GET_BE_U_2(bp);
	
	ND_PRINT("QCA tag ver: %d", (tag >> 14) & 0x3);
	ND_PRINT(", Type: %s", tok2str(qca_tag_type_values, "unknown", (tag >> 6) & 0x1f));
	ND_PRINT(", Prio: %d", (tag >> 11) & 7);
	ND_PRINT(", Tagged: %s", ((tag >> 3) & 0x1) ? "Yes" : "No");
	ND_PRINT(", Port: %d", tag & 7);
	ND_PRINT(", ");
}

void
qca_tag_if_print(netdissect_options *ndo, const struct pcap_pkthdr *h,
		  const u_char *p)
{
	u_int caplen = h->caplen;
	u_int length = h->len;

	ndo->ndo_protocol = "qca-tag";
	ndo->ndo_ll_hdr_len +=
		ether_switch_tag_print(ndo, p, length, caplen,
				       qca_tag_print, QCA_TAG_LEN);
}
