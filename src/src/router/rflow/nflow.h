/*-
 * Copyright (c) 2004 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: nflow.h,v 1.2 2004/03/19 08:12:31 vlm Exp $
 */
#ifndef	__NFLOW_H__
#define	__NFLOW_H__

/* NetFlow Version 1 Header */
typedef struct NFv1H {
	u_int16_t Version;	/* NetFlow export format version number */
	u_int16_t Count;	/* Number of flows exported in this packet */
	u_int32_t SysUptime;	/* Number of milliseconds since reboot */
	u_int32_t unix_secs;	/* Number of seconds since epoch */
	u_int32_t unix_nsecs;	/* Number of residual nanoseconds since epoch */
} NFv1_Header;

/* NetFlow Version 5 Header */
typedef struct NFv5H {
	u_int16_t Version;	/* NetFlow export format version number */
	u_int16_t Count;	/* Number of flows exported in this packet */
	u_int32_t SysUptime;	/* Number of milliseconds since reboot */
	u_int32_t unix_secs;	/* Number of seconds since epoch */
	u_int32_t unix_nsecs;	/* Number of residual nanoseconds since epoch */
	u_int32_t flow_sequence;/* Sequence counter of total flows seen */
	u_int8_t  engine_type;	/* Type of flow switching engine */
	u_int8_t  engine_id;	/* ID number of the flow switching engine */
	/*
	 * Sampling mode and the sampling interval information.
	 * The first two bits of this field indicates the sampling mode:
	 * 00 = No sampling mode is configured
	 * 01 = 'Packet Interval' sampling mode is configured
	 * (One of every x packet is selected and placed in the NetFlow cache)
	 * 10 = Reserved
	 * 11 = Reserved
	 * The remaining 14 bits hold the value of the sampling interval.
	 */
	u_int16_t sampling_interval;
} NFv5_Header;

/* NetFlow Version 1 Record Format */
typedef struct NFv1R {
	u_int32_t srcaddr;	/* Source IP address */
	u_int32_t dstaddr;	/* Destination IP address */
	u_int32_t nexthop;	/* IP address of the next hop router */
	u_int16_t input;	/* SNMP index of the input interface */
	u_int16_t output;	/* SNMP index of the output interface */
	u_int32_t dPkts;	/* Packets in the flow */
	u_int32_t dOctets;	/* Total number of Layer 3 bytes */
	u_int32_t First;	/* SysUptime at start of flow. */
	u_int32_t Last;		/* SysUptime when the last packet was rcvd. */
	u_int16_t srcport;	/* TCP/UDP source port number */
	u_int16_t dstport;	/* TCP/UDP destination port number */
	u_int16_t pad1;		/* Unused */
	u_int8_t  prot;		/* IP protocol */
	u_int8_t  tos;		/* IP ToS */
	u_int8_t  flags;	/* Cumulative OR of TCP flags */
	u_int8_t  tcp_retx_cnt;	/* Number of delayed mis-sequenced packets */
	u_int8_t  tcp_retx_secs;/* Cumulative seconds between m-s.p */
	u_int8_t  tcp_misseq_cnt;/* Number of mis-sequenced packets seen */
	u_int32_t pad2;		/* Unused */
} NFv1_Record;

/* NetFlow Version 5 Record Format */
typedef struct NFv5R {
	u_int32_t srcaddr;	/* Source IP address */
	u_int32_t dstaddr;	/* Destination IP address */
	u_int32_t nexthop;	/* IP address of the next hop router */
	u_int16_t input;	/* SNMP index of the input interface */
	u_int16_t output;	/* SNMP index of the output interface */
	u_int32_t dPkts;	/* Packets in the flow */
	u_int32_t dOctets;	/* Total number of Layer 3 bytes */
	u_int32_t First;	/* SysUptime at start of flow. */
	u_int32_t Last;		/* SysUptime when the last packet was rcvd. */
	u_int16_t srcport;	/* TCP/UDP source port number */
	u_int16_t dstport;	/* TCP/UDP destination port number */
	u_int8_t  pad1;		/* Unused */
	u_int8_t  tcp_flags;	/* Cumulative OR of TCP flags */
	u_int8_t  prot;		/* IP protocol */
	u_int8_t  tos;		/* IP ToS */
	u_int16_t src_as;	/* AS of the source address */
	u_int16_t dst_as;	/* AS of the destination address */
	u_int8_t  src_mask;	/* Source address prefix mask bits */
	u_int8_t  dst_mask;	/* Source address prefix mask bits */
	u_int16_t pad2;		/* Unused */
} NFv5_Record;

/*
 * Supported NetFlow export format versions.
 */
static struct NetFlow_VersionDescriptor {
	int header_size;
	int record_size;
	int max_records;
	int tcp_flags_offset;
	int ip_masks_offset;
} NFVers[10] __attribute__ ((unused)) = {
	/* [0] = */ { 0, 0, 0, 0, 0 },
	/* [1] = */ { sizeof(NFv1_Header), sizeof(NFv1_Record), 24,
			offsetof(NFv1_Record, flags), 0 },
	/* [2] = */ { 0, 0, 0, 0, 0 },
	/* [3] = */ { 0, 0, 0, 0, 0 },
	/* [4] = */ { 0, 0, 0, 0, 0 },
	/* [5] = */ { sizeof(NFv5_Header), sizeof(NFv1_Record), 30,
			offsetof(NFv5_Record, tcp_flags), 0 },
};

#endif	/* __NFLOW_H__ */
