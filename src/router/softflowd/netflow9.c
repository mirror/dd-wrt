/*
 * Copyright 2002 Damien Miller <djm@mindrot.org> All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common.h"
#include "log.h"
#include "treetype.h"
#include "softflowd.h"

/* Netflow v.9 */
struct NF9_HEADER {
	u_int16_t version, flows;
	u_int32_t uptime_ms, time_sec;
	u_int32_t package_sequence, source_id;
} __packed;
struct NF9_FLOWSET_HEADER_COMMON {
	u_int16_t flowset_id, length;
} __packed;
struct NF9_TEMPLATE_FLOWSET_HEADER {
	struct NF9_FLOWSET_HEADER_COMMON c;
	u_int16_t template_id, count;
} __packed;
struct NF9_OPTION_TEMPLATE_FLOWSET_HEADER {
	struct NF9_FLOWSET_HEADER_COMMON c;
	u_int16_t template_id, scope_length, option_length;
} __packed;
struct NF9_TEMPLATE_FLOWSET_RECORD {
	u_int16_t type, length;
} __packed;
struct NF9_DATA_FLOWSET_HEADER {
	struct NF9_FLOWSET_HEADER_COMMON c;
} __packed;
#define NF9_TEMPLATE_FLOWSET_ID		0
#define NF9_OPTIONS_FLOWSET_ID		1
#define NF9_MIN_RECORD_FLOWSET_ID	256

/* Flowset record types the we care about */
#define NF9_IN_BYTES			1
#define NF9_IN_PACKETS			2
/* ... */
#define NF9_IN_PROTOCOL			4
/* ... */
#define NF9_TCP_FLAGS			6
#define NF9_L4_SRC_PORT			7
#define NF9_IPV4_SRC_ADDR		8
/* ... */
#define NF9_IF_INDEX_IN			10
#define NF9_L4_DST_PORT			11
#define NF9_IPV4_DST_ADDR		12
/* ... */
#define NF9_IF_INDEX_OUT		14
/* ... */
#define NF9_LAST_SWITCHED		21
#define NF9_FIRST_SWITCHED		22
/* ... */
#define NF9_IPV6_SRC_ADDR		27
#define NF9_IPV6_DST_ADDR		28
/* ... */
#define NF9_SAMPLING_INTERVAL           34
#define NF9_SAMPLING_ALGORITHM          35
/* ... */
#define NF9_IP_PROTOCOL_VERSION		60

/* Stuff pertaining to the templates that softflowd uses */
#define NF9_SOFTFLOWD_TEMPLATE_NRECORDS	13
struct NF9_SOFTFLOWD_TEMPLATE {
	struct NF9_TEMPLATE_FLOWSET_HEADER h;
	struct NF9_TEMPLATE_FLOWSET_RECORD r[NF9_SOFTFLOWD_TEMPLATE_NRECORDS];
} __packed;

#define NF9_SOFTFLOWD_OPTION_TEMPLATE_SCOPE_RECORDS	1
#define NF9_SOFTFLOWD_OPTION_TEMPLATE_NRECORDS	2
struct NF9_SOFTFLOWD_OPTION_TEMPLATE {
	struct NF9_OPTION_TEMPLATE_FLOWSET_HEADER h;
	struct NF9_TEMPLATE_FLOWSET_RECORD s[NF9_SOFTFLOWD_OPTION_TEMPLATE_SCOPE_RECORDS];
	struct NF9_TEMPLATE_FLOWSET_RECORD r[NF9_SOFTFLOWD_OPTION_TEMPLATE_NRECORDS];
} __packed;

/* softflowd data flowset types */
struct NF9_SOFTFLOWD_DATA_COMMON {
	u_int32_t last_switched, first_switched;
	u_int32_t bytes, packets;
	u_int32_t if_index_in, if_index_out;
	u_int16_t src_port, dst_port;
	u_int8_t protocol, tcp_flags, ipproto;
} __packed;

struct NF9_SOFTFLOWD_DATA_V4 {
	u_int32_t src_addr, dst_addr;
	struct NF9_SOFTFLOWD_DATA_COMMON c;
} __packed;

struct NF9_SOFTFLOWD_DATA_V6 {
	u_int8_t src_addr[16], dst_addr[16];
	struct NF9_SOFTFLOWD_DATA_COMMON c;
} __packed;

struct NF9_SOFTFLOWD_OPTION_DATA {
	struct NF9_FLOWSET_HEADER_COMMON c;	
	u_int32_t scope_ifidx;
	u_int32_t sampling_interval;
	u_int8_t sampling_algorithm;
	u_int8_t padding[3];
} __packed;
	
/* Local data: templates and counters */
#define NF9_SOFTFLOWD_MAX_PACKET_SIZE	512
#define NF9_SOFTFLOWD_V4_TEMPLATE_ID	1024
#define NF9_SOFTFLOWD_V6_TEMPLATE_ID	2048
#define NF9_SOFTFLOWD_OPTION_TEMPLATE_ID	256

#define NF9_DEFAULT_TEMPLATE_INTERVAL	16

/* ... */
#define NF9_OPTION_SCOPE_SYSTEM    1
#define NF9_OPTION_SCOPE_INTERFACE 2
#define NF9_OPTION_SCOPE_LINECARD  3
#define NF9_OPTION_SCOPE_CACHE     4
#define NF9_OPTION_SCOPE_TEMPLATE  5
/* ... */
#define NF9_SAMPLING_ALGORITHM_DETERMINISTIC 1
#define NF9_SAMPLING_ALGORITHM_RANDOM        2
/* ... */

static struct NF9_SOFTFLOWD_TEMPLATE v4_template;
static struct NF9_SOFTFLOWD_TEMPLATE v6_template;
static struct NF9_SOFTFLOWD_OPTION_TEMPLATE option_template;
static struct NF9_SOFTFLOWD_OPTION_DATA option_data;
static int nf9_pkts_until_template = -1;

static void
nf9_init_template(void)
{
	bzero(&v4_template, sizeof(v4_template));
	v4_template.h.c.flowset_id = htons(NF9_TEMPLATE_FLOWSET_ID);
	v4_template.h.c.length = htons(sizeof(v4_template));
	v4_template.h.template_id = htons(NF9_SOFTFLOWD_V4_TEMPLATE_ID);
	v4_template.h.count = htons(NF9_SOFTFLOWD_TEMPLATE_NRECORDS);
	v4_template.r[0].type = htons(NF9_IPV4_SRC_ADDR);
	v4_template.r[0].length = htons(4);
	v4_template.r[1].type = htons(NF9_IPV4_DST_ADDR);
	v4_template.r[1].length = htons(4);
	v4_template.r[2].type = htons(NF9_LAST_SWITCHED);
	v4_template.r[2].length = htons(4);
	v4_template.r[3].type = htons(NF9_FIRST_SWITCHED);
	v4_template.r[3].length = htons(4);
	v4_template.r[4].type = htons(NF9_IN_BYTES);
	v4_template.r[4].length = htons(4);
	v4_template.r[5].type = htons(NF9_IN_PACKETS);
	v4_template.r[5].length = htons(4);
	v4_template.r[6].type = htons(NF9_IF_INDEX_IN);
	v4_template.r[6].length = htons(4);
	v4_template.r[7].type = htons(NF9_IF_INDEX_OUT);
	v4_template.r[7].length = htons(4);
	v4_template.r[8].type = htons(NF9_L4_SRC_PORT);
	v4_template.r[8].length = htons(2);
	v4_template.r[9].type = htons(NF9_L4_DST_PORT);
	v4_template.r[9].length = htons(2);
	v4_template.r[10].type = htons(NF9_IN_PROTOCOL);
	v4_template.r[10].length = htons(1);
	v4_template.r[11].type = htons(NF9_TCP_FLAGS);
	v4_template.r[11].length = htons(1);
	v4_template.r[12].type = htons(NF9_IP_PROTOCOL_VERSION);
	v4_template.r[12].length = htons(1);

	bzero(&v6_template, sizeof(v6_template));
	v6_template.h.c.flowset_id = htons(NF9_TEMPLATE_FLOWSET_ID);
	v6_template.h.c.length = htons(sizeof(v6_template));
	v6_template.h.template_id = htons(NF9_SOFTFLOWD_V6_TEMPLATE_ID);
	v6_template.h.count = htons(NF9_SOFTFLOWD_TEMPLATE_NRECORDS);
	v6_template.r[0].type = htons(NF9_IPV6_SRC_ADDR);
	v6_template.r[0].length = htons(16);
	v6_template.r[1].type = htons(NF9_IPV6_DST_ADDR);
	v6_template.r[1].length = htons(16);
	v6_template.r[2].type = htons(NF9_LAST_SWITCHED);
	v6_template.r[2].length = htons(4);
	v6_template.r[3].type = htons(NF9_FIRST_SWITCHED);
	v6_template.r[3].length = htons(4);
	v6_template.r[4].type = htons(NF9_IN_BYTES);
	v6_template.r[4].length = htons(4);
	v6_template.r[5].type = htons(NF9_IN_PACKETS);
	v6_template.r[5].length = htons(4);
	v6_template.r[6].type = htons(NF9_IF_INDEX_IN);
	v6_template.r[6].length = htons(4);
	v6_template.r[7].type = htons(NF9_IF_INDEX_OUT);
	v6_template.r[7].length = htons(4);
	v6_template.r[8].type = htons(NF9_L4_SRC_PORT);
	v6_template.r[8].length = htons(2);
	v6_template.r[9].type = htons(NF9_L4_DST_PORT);
	v6_template.r[9].length = htons(2);
	v6_template.r[10].type = htons(NF9_IN_PROTOCOL);
	v6_template.r[10].length = htons(1);
	v6_template.r[11].type = htons(NF9_TCP_FLAGS);
	v6_template.r[11].length = htons(1);
	v6_template.r[12].type = htons(NF9_IP_PROTOCOL_VERSION);
	v6_template.r[12].length = htons(1);
}

static void
nf9_init_option(u_int16_t ifidx, struct OPTION *option) {
	bzero(&option_template, sizeof(option_template));
	option_template.h.c.flowset_id = htons(NF9_OPTIONS_FLOWSET_ID);
	option_template.h.c.length = htons(sizeof(option_template));
	option_template.h.template_id = htons(NF9_SOFTFLOWD_OPTION_TEMPLATE_ID);
	option_template.h.scope_length = htons(sizeof(option_template.s));
	option_template.h.option_length = htons(sizeof(option_template.r));
	option_template.s[0].type = htons(NF9_OPTION_SCOPE_INTERFACE);
	option_template.s[0].length = htons(sizeof(option_data.scope_ifidx));
	option_template.r[0].type = htons(NF9_SAMPLING_INTERVAL);
	option_template.r[0].length = htons(sizeof(option_data.sampling_interval));
	option_template.r[1].type = htons(NF9_SAMPLING_ALGORITHM);
	option_template.r[1].length = htons(sizeof(option_data.sampling_algorithm));

	bzero(&option_data, sizeof(option_data));
	option_data.c.flowset_id = htons(NF9_SOFTFLOWD_OPTION_TEMPLATE_ID);
	option_data.c.length = htons(sizeof(option_data));
	option_data.scope_ifidx = htonl(ifidx);
	option_data.sampling_interval = htonl(option->sample);
	option_data.sampling_algorithm = NF9_SAMPLING_ALGORITHM_DETERMINISTIC;
}
static int
nf_flow_to_flowset(const struct FLOW *flow, u_char *packet, u_int len,
    u_int16_t ifidx, const struct timeval *system_boot_time, u_int *len_used)
{
	union {
		struct NF9_SOFTFLOWD_DATA_V4 d4;
		struct NF9_SOFTFLOWD_DATA_V6 d6;
	} d[2];
	struct NF9_SOFTFLOWD_DATA_COMMON *dc[2];
	u_int freclen, ret_len, nflows;

	bzero(d, sizeof(d));
	*len_used = nflows = ret_len = 0;
	switch (flow->af) {
	case AF_INET:
		freclen = sizeof(struct NF9_SOFTFLOWD_DATA_V4);
		memcpy(&d[0].d4.src_addr, &flow->addr[0].v4, 4);
		memcpy(&d[0].d4.dst_addr, &flow->addr[1].v4, 4);
		memcpy(&d[1].d4.src_addr, &flow->addr[1].v4, 4);
		memcpy(&d[1].d4.dst_addr, &flow->addr[0].v4, 4);
		dc[0] = &d[0].d4.c;
		dc[1] = &d[1].d4.c;
		dc[0]->ipproto = dc[1]->ipproto = 4;
		break;
	case AF_INET6:
		freclen = sizeof(struct NF9_SOFTFLOWD_DATA_V6);
		memcpy(&d[0].d6.src_addr, &flow->addr[0].v6, 16);
		memcpy(&d[0].d6.dst_addr, &flow->addr[1].v6, 16);
		memcpy(&d[1].d6.src_addr, &flow->addr[1].v6, 16);
		memcpy(&d[1].d6.dst_addr, &flow->addr[0].v6, 16);
		dc[0] = &d[0].d6.c;
		dc[1] = &d[1].d6.c;
		dc[0]->ipproto = dc[1]->ipproto = 6;
		break;
	default:
		return (-1);
	}

	dc[0]->first_switched = dc[1]->first_switched = 
	    htonl(timeval_sub_ms(&flow->flow_start, system_boot_time));
	dc[0]->last_switched = dc[1]->last_switched = 
	    htonl(timeval_sub_ms(&flow->flow_last, system_boot_time));
	dc[0]->bytes = htonl(flow->octets[0]);
	dc[1]->bytes = htonl(flow->octets[1]);
	dc[0]->packets = htonl(flow->packets[0]);
	dc[1]->packets = htonl(flow->packets[1]);
	dc[0]->if_index_in = dc[0]->if_index_out = htonl(ifidx);
	dc[1]->if_index_in = dc[1]->if_index_out = htonl(ifidx);
	dc[0]->src_port = dc[1]->dst_port = flow->port[0];
	dc[1]->src_port = dc[0]->dst_port = flow->port[1];
	dc[0]->protocol = dc[1]->protocol = flow->protocol;
	dc[0]->tcp_flags = flow->tcp_flags[0];
	dc[1]->tcp_flags = flow->tcp_flags[1];

	if (flow->octets[0] > 0) {
		if (ret_len + freclen > len)
			return (-1);
		memcpy(packet + ret_len, &d[0], freclen);
		ret_len += freclen;
		nflows++;
	}
	if (flow->octets[1] > 0) {
		if (ret_len + freclen > len)
			return (-1);
		memcpy(packet + ret_len, &d[1], freclen);
		ret_len += freclen;
		nflows++;
	}

	*len_used = ret_len;
	return (nflows);
}

/*
 * Given an array of expired flows, send netflow v9 report packets
 * Returns number of packets sent or -1 on error
 */
int
send_netflow_v9(struct FLOW **flows, int num_flows, int nfsock, u_int16_t ifidx,
    u_int64_t *flows_exported, struct timeval *system_boot_time,
    int verbose_flag, struct OPTION *option)
{
	struct NF9_HEADER *nf9;
	struct NF9_DATA_FLOWSET_HEADER *dh;
	struct timeval now;
	u_int offset, last_af, i, j, num_packets, inc, last_valid;
	socklen_t errsz;
	int err, r;
	u_char packet[NF9_SOFTFLOWD_MAX_PACKET_SIZE];

	gettimeofday(&now, NULL);

	if (nf9_pkts_until_template == -1) {
		nf9_init_template();
		nf9_pkts_until_template = 0;
		if (option != NULL && option->sample > 1){
			nf9_init_option(ifidx, option);
		}
	}		

	last_valid = num_packets = 0;
	for (j = 0; j < num_flows;) {
		bzero(packet, sizeof(packet));
		nf9 = (struct NF9_HEADER *)packet;

		nf9->version = htons(9);
		nf9->flows = 0; /* Filled as we go, htons at end */
		nf9->uptime_ms = htonl(timeval_sub_ms(&now, system_boot_time));
		nf9->time_sec = htonl(time(NULL));
		nf9->package_sequence = htonl(*flows_exported + j);
		nf9->source_id = 0;
		offset = sizeof(*nf9);

		/* Refresh template headers if we need to */
		if (nf9_pkts_until_template <= 0) {
			memcpy(packet + offset, &v4_template,
			    sizeof(v4_template));
			offset += sizeof(v4_template);
			memcpy(packet + offset, &v6_template,
			    sizeof(v6_template));
			offset += sizeof(v6_template);
			if (option != NULL && option->sample > 1){
				memcpy(packet + offset, &option_template,
				       sizeof(option_template));
				offset += sizeof(option_template);
				memcpy(packet + offset, &option_data,
				       sizeof(option_data));
				offset += sizeof(option_data);
			}

			nf9_pkts_until_template = NF9_DEFAULT_TEMPLATE_INTERVAL;
		}

		dh = NULL;
		last_af = 0;
		for (i = 0; i + j < num_flows; i++) {
			if (dh == NULL || flows[i + j]->af != last_af) {
				if (dh != NULL) {
					if (offset % 4 != 0) {
						/* Pad to multiple of 4 */
						dh->c.length += 4 - (offset % 4);
						offset += 4 - (offset % 4);
					}
					/* Finalise last header */
					dh->c.length = htons(dh->c.length);
				}
				if (offset + sizeof(*dh) > sizeof(packet)) {
					/* Mark header is finished */
					dh = NULL;
					break;
				}
				dh = (struct NF9_DATA_FLOWSET_HEADER *)
				    (packet + offset);
				dh->c.flowset_id =
				    (flows[i + j]->af == AF_INET) ?
				    v4_template.h.template_id : 
				    v6_template.h.template_id;
				last_af = flows[i + j]->af;
				last_valid = offset;
				dh->c.length = sizeof(*dh); /* Filled as we go */
				offset += sizeof(*dh);
			}

			r = nf_flow_to_flowset(flows[i + j], packet + offset,
			    sizeof(packet) - offset, ifidx, system_boot_time, &inc);
			if (r <= 0) {
				/* yank off data header, if we had to go back */
				if (last_valid)
					offset = last_valid;
				break;
			}
			offset += inc;
			dh->c.length += inc;
			nf9->flows += r;
			last_valid = 0; /* Don't clobber this header now */
			if (verbose_flag) {
				logit(LOG_DEBUG, "Flow %d/%d: "
				    "r %d offset %d type %04x len %d(0x%04x) "
				    "flows %d", r, i, j, offset, 
				    dh->c.flowset_id, dh->c.length, 
				    dh->c.length, nf9->flows);
			}
		}
		/* Don't finish header if it has already been done */
		if (dh != NULL) {
			if (offset % 4 != 0) {
				/* Pad to multiple of 4 */
				dh->c.length += 4 - (offset % 4);
				offset += 4 - (offset % 4);
			}
			/* Finalise last header */
			dh->c.length = htons(dh->c.length);
		}
		nf9->flows = htons(nf9->flows);

		if (verbose_flag)
			logit(LOG_DEBUG, "Sending flow packet len = %d", offset);
		errsz = sizeof(err);
		/* Clear ICMP errors */
		getsockopt(nfsock, SOL_SOCKET, SO_ERROR, &err, &errsz); 
		if (send(nfsock, packet, (size_t)offset, 0) == -1)
			return (-1);
		num_packets++;
		nf9_pkts_until_template--;

		j += i;
	}

	*flows_exported += j;
	return (num_packets);
}

void
netflow9_resend_template(void)
{
	if (nf9_pkts_until_template > 0)
		nf9_pkts_until_template = 0;
}
