/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2007 Sippy Software, Inc., http://www.sippysoft.com
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
 */

#ifndef _RTPP_RECORD_PRIVATE_H_
#define _RTPP_RECORD_PRIVATE_H_

#define	DLT_NULL	0
#define	DLT_EN10MB	1
#define	PCAP_MAGIC	0xa1b2c3d4
#define	PCAP_VER_MAJR	2
#define	PCAP_VER_MINR	4

/* Global PCAP Header */
typedef struct pcap_hdr_s {
    uint32_t magic_number;   /* magic number */
    uint16_t version_major;  /* major version number */
    uint16_t version_minor;  /* minor version number */
    int32_t  thiszone;       /* GMT to local correction */
    uint32_t sigfigs;        /* accuracy of timestamps */
    uint32_t snaplen;        /* max length of captured packets, in octets */
    uint32_t network;        /* data link type */
} pcap_hdr_t;

/* PCAP Packet Header */
typedef struct pcaprec_hdr_s {
    uint32_t ts_sec;         /* timestamp seconds */
    uint32_t ts_usec;        /* timestamp microseconds */
    uint32_t incl_len;       /* number of octets of packet saved in file */
    uint32_t orig_len;       /* actual length of packet */
} pcaprec_hdr_t;

struct udpip {
    struct ip iphdr;
    struct udphdr udphdr;
} __attribute__((__packed__));

/*
 * Recorded data header
 */
struct pkt_hdr_pcap_null {
    pcaprec_hdr_t pcaprec_hdr;
    uint32_t family;
    struct udpip udpip;
} __attribute__((__packed__));

struct pkt_hdr_pcap_en10t {
    pcaprec_hdr_t pcaprec_hdr;
    uint8_t ether_dhost[6];
    uint8_t ether_shost[6];
    uint16_t ether_type;
    struct udpip udpip;
} __attribute__((__packed__));

union pkt_hdr_pcap {
    struct pkt_hdr_pcap_null null;
    struct pkt_hdr_pcap_en10t en10t;
};

/* Stripped down version of sockaddr_in* for saving space */
struct sockaddr_in4_s {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
};

struct sockaddr_in6_s {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in6_addr sin_addr;
};

union sockaddr_in_s {
    struct sockaddr_in4_s in4;
    struct sockaddr_in6_s in6;
};

struct pkt_hdr_adhoc {
    union sockaddr_in_s addr;   /* Source address */
    double time;		/* Time of arrival */
    unsigned short plen;	/* Length of following RTP/RTCP packet */
} __attribute__((__packed__));

#endif
