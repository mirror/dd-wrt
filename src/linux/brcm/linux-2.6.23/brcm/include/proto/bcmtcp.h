/*
 * Fundamental constants relating to TCP Protocol
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmtcp.h,v 1.3.32.1 2009/10/15 01:38:07 Exp $
 */

#ifndef _bcmtcp_h_
#define _bcmtcp_h_

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>


#define TCP_SRC_PORT_OFFSET	0	/* TCP source port offset */
#define TCP_DEST_PORT_OFFSET	2	/* TCP dest port offset */
#define TCP_CHKSUM_OFFSET	16	/* TCP body checksum offset */

#define	TCP_FLAG_RST		0x0004
#define	TCP_FLAG_SYN		0x0002
#define	TCP_FLAG_FIN		0x0001

/* These fields are stored in network order */
BWL_PRE_PACKED_STRUCT struct bcmtcp_hdr
{
	uint16	src_port;	/* Source Port Address */
	uint16	dst_port;	/* Destination Port Address */
	uint32	seq_num;	/* TCP Sequence Number */
	uint32	ack_num;	/* TCP Sequence Number */
	uint16	hdrlen_rsvd_flags;	/* Header length, reserved bits and flags */
	uint16	tcpwin;		/* TCP window */
	uint16	chksum;		/* Segment checksum with pseudoheader */
	uint16	urg_ptr;	/* Points to seq-num of byte following urg data */
} BWL_POST_PACKED_STRUCT;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif	/* #ifndef _bcmtcp_h_ */
