/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2020 embedd.ch 
 *   Copyright (C) 2020 Felix Fietkau <nbd@nbd.name> 
 *   Copyright (C) 2020 John Crispin <john@phrozen.org> 
 */

#include <netinet/udp.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <pcap/pcap.h>

#include <libubox/blobmsg_json.h>

#include "usteer.h"
#include "remote.h"

static pcap_t *pcap;
static int pkt_offset;

/* IP header */
struct ip_header {
	uint8_t ip_vhl;		/* version << 4 | header length >> 2 */
	uint8_t ip_tos;		/* type of service */
	uint16_t ip_len;		/* total length */
	uint16_t ip_id;		/* identification */
	uint16_t ip_off;		/* fragment offset field */
#define IP_RF 0x8000		/* reserved fragment flag */
#define IP_DF 0x4000		/* dont fragment flag */
#define IP_MF 0x2000		/* more fragments flag */
#define IP_OFFMASK 0x1fff	/* mask for fragmenting bits */
	uint8_t ip_ttl;		/* time to live */
	uint8_t ip_p;		/* protocol */
	uint16_t ip_sum;		/* checksum */
	struct in_addr ip_src, ip_dst; /* source and dest address */
};
#define IP_HL(ip)		(((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)		(((ip)->ip_vhl) >> 4)

struct udp_header {
	uint16_t uh_sport;       /* source port */
	uint16_t uh_dport;       /* destination port */
	uint16_t uh_ulen;        /* udp length */
	uint16_t uh_sum;     /* udp checksum */
};


static void
decode_sta(struct blob_attr *data)
{
	struct apmsg_sta msg;

	if (!parse_apmsg_sta(&msg, data))
		return;

	fprintf(stderr, "\t\tSta "MAC_ADDR_FMT" signal=%d connected=%d timeout=%d\n",
		MAC_ADDR_DATA(msg.addr), msg.signal, msg.connected, msg.timeout);
}

static void
decode_node(struct blob_attr *data)
{
	struct apmsg_node msg;
	struct blob_attr *cur;
	int rem;

	if (!parse_apmsg_node(&msg, data))
		return;

	fprintf(stderr, "\tNode %s, freq=%d, n_assoc=%d, noise=%d load=%d max_assoc=%d\n",
		msg.name, msg.freq, msg.n_assoc, msg.noise, msg.load, msg.max_assoc);
	if (msg.rrm_nr) {
		fprintf(stderr, "\t\tRRM:");
		blobmsg_for_each_attr(cur, msg.rrm_nr, rem) {
			if (!blobmsg_check_attr(cur, false))
				continue;
			if (blobmsg_type(cur) != BLOBMSG_TYPE_STRING)
				continue;
			fprintf(stderr, " %s", blobmsg_get_string(cur));
		}
		fprintf(stderr, "\n");
	}

	if (msg.node_info) {
		char *data = blobmsg_format_json(msg.node_info, true);
		fprintf(stderr, "\t\tNode info: %s\n", data);
		free(data);
	}

	blob_for_each_attr(cur, msg.stations, rem)
		decode_sta(cur);
}

static void
decode_packet(struct blob_attr *data)
{
	struct apmsg msg;
	struct blob_attr *cur;
	int rem;

	if (!parse_apmsg(&msg, data)) {
		fprintf(stderr, "missing fields\n");
		return;
	}

	fprintf(stderr, "id=%08x, seq=%d\n", msg.id, msg.seq);
	if (msg.host_info) {
		char *data = blobmsg_format_json(msg.host_info, true);
		fprintf(stderr, "\tHost info: %s\n", data);
		free(data);
	}

	blob_for_each_attr(cur, msg.nodes, rem)
		decode_node(cur);
}

static void
recv_packet(unsigned char *user, const struct pcap_pkthdr *hdr,
	    const unsigned char *packet)
{
	char addr[INET_ADDRSTRLEN];
	struct ip_header *ip;
	struct udp_header *uh;
	struct blob_attr *data;
	int len = hdr->caplen;
	int hdrlen;

	len -= pkt_offset;
	packet += pkt_offset;
	ip = (void *) packet;

	hdrlen = IP_HL(ip) * 4;
	if (hdrlen < 20 || hdrlen >= len)
		return;

	len -= hdrlen;
	packet += hdrlen;

	inet_ntop(AF_INET, &ip->ip_src, addr, sizeof(addr));

	hdrlen = sizeof(*uh);
	if (len <= hdrlen)
		return;

	uh = (void *) packet;
	packet += hdrlen;
	len -= hdrlen;

	if (uh->uh_dport != htons(APMGR_PORT))
		return;

	data = (void *) packet;

	fprintf(stderr, "[%s]: len=%d ", addr, len);

	if (len != blob_pad_len(data)) {
		fprintf(stderr, "invalid data\n");
		return;
	}

	decode_packet(data);
}

int main(int argc, char **argv)
{
	static char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
		return 1;
	}

	pcap = pcap_open_live(argv[1], APMGR_BUFLEN, 1, 1000, errbuf);
	if (!pcap) {
			fprintf(stderr, "Failed to open interface %s: %s\n", argv[1], errbuf);
			return 1;
	}

	pcap_compile(pcap, &fp, "port "APMGR_PORT_STR, 1, PCAP_NETMASK_UNKNOWN);
	pcap_setfilter(pcap, &fp);

	switch (pcap_datalink(pcap)) {
	case DLT_EN10MB:
		pkt_offset = 14;
		break;
	case DLT_RAW:
		pkt_offset = 0;
		break;
	default:
		fprintf(stderr, "Invalid link type\n");
		return -1;
	}

	pcap_loop(pcap, 0, recv_packet, NULL);
	pcap_close(pcap);

	return 0;
}
