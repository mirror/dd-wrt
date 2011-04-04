/*
 * OpenDPI_demo.c
 * Copyright (C) 2009-2010 by ipoque GmbH
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * OpenDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * OpenDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <netinet/in.h>

#ifdef __linux__
# include <linux/ip.h>
# include <linux/tcp.h>
# include <linux/udp.h>
# include <linux/if_ether.h>
#else
# include "linux_compat.h"
#endif

#include <pcap.h>

#include "ipq_api.h"

// cli options
static char *_pcap_file = NULL;

// pcap
static char _pcap_error_buffer[PCAP_ERRBUF_SIZE];
static pcap_t *_pcap_handle = NULL;
static int _pcap_datalink_type = 0;

// detection
static struct ipoque_detection_module_struct *ipoque_struct = NULL;
static u32 detection_tick_resolution = 1000;
static char *prot_long_str[] = { IPOQUE_PROTOCOL_LONG_STRING };

#ifdef IPOQUE_ENABLE_DEBUG_MESSAGES
static char *prot_short_str[] = { IPOQUE_PROTOCOL_SHORT_STRING };

static IPOQUE_PROTOCOL_BITMASK debug_messages_bitmask;
#endif

// results
static u64 raw_packet_count = 0;
static u64 ip_packet_count = 0;
static u64 total_bytes = 0;
static u64 protocol_counter[IPOQUE_MAX_SUPPORTED_PROTOCOLS + 1];
static u64 protocol_counter_bytes[IPOQUE_MAX_SUPPORTED_PROTOCOLS + 1];


// id tracking
typedef struct osdpi_id {
	u8 ip[4];
	struct ipoque_id_struct *ipoque_id;
} osdpi_id_t;

static u32 size_id_struct = 0;
#define			MAX_OSDPI_IDS			50000
static struct osdpi_id *osdpi_ids;
static u32 osdpi_id_count = 0;


// flow tracking
typedef struct osdpi_flow {
	u32 lower_ip;
	u32 upper_ip;
	u16 lower_port;
	u16 upper_port;
	u8 protocol;
	struct ipoque_flow_struct *ipoque_flow;

	// result only, not used for flow identification
	u32 detected_protocol;
} osdpi_flow_t;

static u32 size_flow_struct = 0;
#define			MAX_OSDPI_FLOWS			200000
static struct osdpi_flow *osdpi_flows;
static u32 osdpi_flow_count = 0;

#ifdef IPOQUE_ENABLE_DEBUG_MESSAGES
static int string_to_detection_bitmask(char *str, IPOQUE_PROTOCOL_BITMASK * dbm)
{
	u32 a;
	u32 oldptr = 0;
	u32 ptr = 0;
	IPOQUE_BITMASK_RESET(*dbm);

	printf("Protocol parameter given: %s\n", str);

	if (strcmp(str, "all") == 0) {
		printf("Protocol parameter all parsed\n");
		IPOQUE_BITMASK_SET_ALL(*dbm);
		printf("Bitmask is: " IPOQUE_BITMASK_DEBUG_OUTPUT_BITMASK_STRING " \n",
			   IPOQUE_BITMASK_DEBUG_OUTPUT_BITMASK_VALUE(*dbm));
		return 0;
	}
	// parse bitmask
	while (1) {
		if (str[ptr] == 0 || str[ptr] == ' ') {
			printf("Protocol parameter: parsed: %.*s,\n", ptr - oldptr, &str[oldptr]);
			for (a = 1; a <= IPOQUE_MAX_SUPPORTED_PROTOCOLS; a++) {

				if (strlen(prot_short_str[a]) == (ptr - oldptr) &&
					(memcmp(&str[oldptr], prot_short_str[a], ptr - oldptr) == 0)) {
					IPOQUE_ADD_PROTOCOL_TO_BITMASK(*dbm, a);
					printf("Protocol parameter detected as protocol %s\n", prot_long_str[a]);
				}
			}
			oldptr = ptr + 1;
			if (str[ptr] == 0)
				break;
		}
		ptr++;
	}
	return 0;
}
#endif

static void parseOptions(int argc, char **argv)
{
	int opt;

#ifdef IPOQUE_ENABLE_DEBUG_MESSAGES
	IPOQUE_BITMASK_SET_ALL(debug_messages_bitmask);
#endif

	while ((opt = getopt(argc, argv, "f:e:")) != EOF) {
		switch (opt) {
		case 'f':
			_pcap_file = optarg;
			break;
		case 'e':
#ifdef IPOQUE_ENABLE_DEBUG_MESSAGES
			// set debug logging bitmask to all protocols
			if (string_to_detection_bitmask(optarg, &debug_messages_bitmask) != 0) {
				printf("ERROR option -e needs a valid list of protocols");
				exit(-1);
			}

			printf("debug messages Bitmask is: " IPOQUE_BITMASK_DEBUG_OUTPUT_BITMASK_STRING "\n",
				   IPOQUE_BITMASK_DEBUG_OUTPUT_BITMASK_VALUE(debug_messages_bitmask));

#else
			printf("ERROR: option -e : DEBUG MESSAGES DEACTIVATED\n");
			exit(-1);
#endif
			break;
		}
	}

	// check parameters
	if (_pcap_file == NULL || strcmp(_pcap_file, "") == 0) {
		printf("ERROR: no pcap file path provided; use option -f with the path to a valid pcap file\n");
		exit(-1);
	}
}

static void debug_printf(u32 protocol, void *id_struct, ipq_log_level_t log_level, const char *format, ...)
{
#ifdef IPOQUE_ENABLE_DEBUG_MESSAGES
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(debug_messages_bitmask, protocol) != 0) {
		const char *protocol_string;
		const char *file;
		const char *func;
		u32 line;
		va_list ap;
		va_start(ap, format);

		protocol_string = prot_short_str[protocol];

		ipoque_debug_get_last_log_function_line(ipoque_struct, &file, &func, &line);

		printf("\nDEBUG: %s:%s:%u Prot: %s, level: %u packet: %llu :", file, func, line, protocol_string,
			   log_level, raw_packet_count);
		vprintf(format, ap);
		va_end(ap);
	}
#endif
}

static void *malloc_wrapper(unsigned long size)
{
	return malloc(size);
}

static void free_wrapper(void *freeable)
{
	free(freeable);
}

static void *get_id(const u8 * ip)
{
	u32 i;
	for (i = 0; i < osdpi_id_count; i++) {
		if (memcmp(osdpi_ids[i].ip, ip, sizeof(u8) * 4) == 0) {
			return osdpi_ids[i].ipoque_id;
		}
	}
	if (osdpi_id_count == MAX_OSDPI_IDS) {
		printf("ERROR: maximum unique id count (%u) has been exceeded\n", MAX_OSDPI_IDS);
		exit(-1);
	} else {
		struct ipoque_id_struct *ipoque_id;
		memcpy(osdpi_ids[osdpi_id_count].ip, ip, sizeof(u8) * 4);
		ipoque_id = osdpi_ids[osdpi_id_count].ipoque_id;

		osdpi_id_count += 1;
		return ipoque_id;
	}
}

static struct osdpi_flow *get_osdpi_flow(const struct iphdr *iph, u16 ipsize)
{
	u32 i;
	u16 l4_packet_len;
	struct tcphdr *tcph = NULL;
	struct udphdr *udph = NULL;

	u32 lower_ip;
	u32 upper_ip;
	u16 lower_port;
	u16 upper_port;

	if (ipsize < 20)
		return NULL;

	if ((iph->ihl * 4) > ipsize || ipsize < ntohs(iph->tot_len)
		|| (iph->frag_off & htons(0x1FFF)) != 0)
		return NULL;

	l4_packet_len = ntohs(iph->tot_len) - (iph->ihl * 4);

	if (iph->saddr < iph->daddr) {
		lower_ip = iph->saddr;
		upper_ip = iph->daddr;
	} else {
		lower_ip = iph->daddr;
		upper_ip = iph->saddr;
	}

	if (iph->protocol == 6 && l4_packet_len >= 20) {
		// tcp
		tcph = (struct tcphdr *) ((u8 *) iph + iph->ihl * 4);
		if (iph->saddr < iph->daddr) {
			lower_port = tcph->source;
			upper_port = tcph->dest;
		} else {
			lower_port = tcph->dest;
			upper_port = tcph->source;
		}
	} else if (iph->protocol == 17 && l4_packet_len >= 8) {
		// udp
		udph = (struct udphdr *) ((u8 *) iph + iph->ihl * 4);
		if (iph->saddr < iph->daddr) {
			lower_port = udph->source;
			upper_port = udph->dest;
		} else {
			lower_port = udph->dest;
			upper_port = udph->source;
		}
	} else {
		// non tcp/udp protocols
		lower_port = 0;
		upper_port = 0;
	}

	for (i = 0; i < osdpi_flow_count; i++) {
		if (osdpi_flows[i].protocol == iph->protocol &&
			osdpi_flows[i].lower_ip == lower_ip &&
			osdpi_flows[i].upper_ip == upper_ip &&
			osdpi_flows[i].lower_port == lower_port && osdpi_flows[i].upper_port == upper_port) {
			return &osdpi_flows[i];
		}
	}
	if (osdpi_flow_count == MAX_OSDPI_FLOWS) {
		printf("ERROR: maximum flow count (%u) has been exceeded\n", MAX_OSDPI_FLOWS);
		exit(-1);
	} else {
		struct osdpi_flow *flow;
		osdpi_flows[osdpi_flow_count].protocol = iph->protocol;
		osdpi_flows[osdpi_flow_count].lower_ip = lower_ip;
		osdpi_flows[osdpi_flow_count].upper_ip = upper_ip;
		osdpi_flows[osdpi_flow_count].lower_port = lower_port;
		osdpi_flows[osdpi_flow_count].upper_port = upper_port;
		flow = &osdpi_flows[osdpi_flow_count];

		osdpi_flow_count += 1;
		return flow;
	}
}

static void setupDetection(void)
{
	u32 i;
	IPOQUE_PROTOCOL_BITMASK all;

	// init global detection structure
	ipoque_struct = ipoque_init_detection_module(detection_tick_resolution, malloc_wrapper, debug_printf);
	if (ipoque_struct == NULL) {
		printf("ERROR: global structure initialization failed\n");
		exit(-1);
	}
	// enable all protocols
	IPOQUE_BITMASK_SET_ALL(all);
	ipoque_set_protocol_detection_bitmask2(ipoque_struct, &all);

	// allocate memory for id and flow tracking
	size_id_struct = ipoque_detection_get_sizeof_ipoque_id_struct();
	size_flow_struct = ipoque_detection_get_sizeof_ipoque_flow_struct();

	osdpi_ids = malloc(MAX_OSDPI_IDS * sizeof(struct osdpi_id));
	if (osdpi_ids == NULL) {
		printf("ERROR: malloc for osdpi_ids failed\n");
		exit(-1);
	}
	for (i = 0; i < MAX_OSDPI_IDS; i++) {
		memset(&osdpi_ids[i], 0, sizeof(struct osdpi_id));
		osdpi_ids[i].ipoque_id = calloc(1, size_id_struct);
		if (osdpi_ids[i].ipoque_id == NULL) {
			printf("ERROR: malloc for ipoque_id_struct failed\n");
			exit(-1);
		}
	}

	osdpi_flows = malloc(MAX_OSDPI_FLOWS * sizeof(struct osdpi_flow));
	if (osdpi_flows == NULL) {
		printf("ERROR: malloc for osdpi_flows failed\n");
		exit(-1);
	}
	for (i = 0; i < MAX_OSDPI_FLOWS; i++) {
		memset(&osdpi_flows[i], 0, sizeof(struct osdpi_flow));
		osdpi_flows[i].ipoque_flow = calloc(1, size_flow_struct);
		if (osdpi_flows[i].ipoque_flow == NULL) {
			printf("ERROR: malloc for ipoque_flow_struct failed\n");
			exit(-1);
		}
	}

	// clear memory for results
	memset(protocol_counter, 0, (IPOQUE_MAX_SUPPORTED_PROTOCOLS + 1) * sizeof(u64));
	memset(protocol_counter_bytes, 0, (IPOQUE_MAX_SUPPORTED_PROTOCOLS + 1) * sizeof(u64));
}

static void terminateDetection(void)
{
	u32 i;

	ipoque_exit_detection_module(ipoque_struct, free_wrapper);

	for (i = 0; i < MAX_OSDPI_IDS; i++) {
		free(osdpi_ids[i].ipoque_id);
	}
	free(osdpi_ids);
	for (i = 0; i < MAX_OSDPI_FLOWS; i++) {
		free(osdpi_flows[i].ipoque_flow);
	}
	free(osdpi_flows);
}

static unsigned int packet_processing(const uint64_t time, const struct iphdr *iph, uint16_t ipsize, uint16_t rawsize)
{
	struct ipoque_id_struct *src = NULL;
	struct ipoque_id_struct *dst = NULL;
	struct osdpi_flow *flow = NULL;
	struct ipoque_flow_struct *ipq_flow = NULL;
	u32 protocol = 0;


	src = get_id((u8 *) & iph->saddr);
	dst = get_id((u8 *) & iph->daddr);

	flow = get_osdpi_flow(iph, ipsize);
	if (flow != NULL) {
		ipq_flow = flow->ipoque_flow;
	}

	ip_packet_count++;
	total_bytes += rawsize;

#ifndef IPOQUE_ENABLE_DEBUG_MESSAGES
	if (ip_packet_count % 499 == 0) {
		printf("\rip packets scanned: \x1b[33m%-10llu\x1b[0m ip bytes scanned: \x1b[34m%-10llu\x1b[0m",
			   ip_packet_count, total_bytes);
	}
#endif

	// only handle unfragmented packets
	if ((iph->frag_off & htons(0x1FFF)) == 0) {

		// here the actual detection is performed
		protocol = ipoque_detection_process_packet(ipoque_struct, ipq_flow, (uint8_t *) iph, ipsize, time, src, dst);

	} else {
		static u8 frag_warning_used = 0;
		if (frag_warning_used == 0) {
			printf("\n\nWARNING: fragmented ip packets are not supported and will be skipped \n\n");
			sleep(2);
			frag_warning_used = 1;
		}
		return 0;
	}

	protocol_counter[protocol]++;
	protocol_counter_bytes[protocol] += rawsize;

	if (flow != NULL) {
		flow->detected_protocol = protocol;
	}

	return 0;
}

static void printResults(void)
{
	u32 i;

	printf("\x1b[2K\n");
	printf("pcap file contains\n");
	printf("\tip packets:   \x1b[33m%-13llu\x1b[0m of %llu packets total\n", ip_packet_count, raw_packet_count);
	printf("\tip bytes:     \x1b[34m%-13llu\x1b[0m\n", total_bytes);
	printf("\tunique ids:   \x1b[35m%-13u\x1b[0m\n", osdpi_id_count);
	printf("\tunique flows: \x1b[36m%-13u\x1b[0m\n", osdpi_flow_count);

	printf("\n\ndetected protocols:\n");
	for (i = 0; i <= IPOQUE_MAX_SUPPORTED_PROTOCOLS; i++) {
		u32 protocol_flows = 0;
		u32 j;

		// count flows for that protocol
		for (j = 0; j < osdpi_flow_count; j++) {
			if (osdpi_flows[j].detected_protocol == i) {
				protocol_flows++;
			}
		}

		if (protocol_counter[i] > 0) {
			printf("\t\x1b[31m%-20s\x1b[0m packets: \x1b[33m%-13llu\x1b[0m bytes: \x1b[34m%-13llu\x1b[0m "
				   "flows: \x1b[36m%-13u\x1b[0m\n",
				   prot_long_str[i], protocol_counter[i], protocol_counter_bytes[i], protocol_flows);
		}
	}
	printf("\n\n");
}

static void openPcapFile(void)
{
	_pcap_handle = pcap_open_offline(_pcap_file, _pcap_error_buffer);

	if (_pcap_handle == NULL) {
		printf("ERROR: could not open pcap file: %s\n", _pcap_error_buffer);
		exit(-1);
	}
	_pcap_datalink_type = pcap_datalink(_pcap_handle);
}

static void closePcapFile(void)
{
	if (_pcap_handle != NULL) {
		pcap_close(_pcap_handle);
	}
}

// executed for each packet in the pcap file
static void pcap_packet_callback(u_char * args, const struct pcap_pkthdr *header, const u_char * packet)
{
	const struct ethhdr *ethernet = (struct ethhdr *) packet;
	struct iphdr *iph = (struct iphdr *) &packet[sizeof(struct ethhdr)];
	u64 time;
	static u64 lasttime = 0;
	u16 type;

	raw_packet_count++;

	time =
		((uint64_t) header->ts.tv_sec) * detection_tick_resolution +
		header->ts.tv_usec / (1000000 / detection_tick_resolution);
	if (lasttime > time) {
		// printf("\nWARNING: timestamp bug in the pcap file (ts delta: %llu, repairing)\n", lasttime - time);
		time = lasttime;
	}
	lasttime = time;


	type = ethernet->h_proto;

	// just work on Ethernet packets that contain IP
	if (_pcap_datalink_type == DLT_EN10MB && type == htons(ETH_P_IP)
		&& header->caplen >= sizeof(struct ethhdr)) {

		if (header->caplen < header->len) {
			static u8 cap_warning_used = 0;
			if (cap_warning_used == 0) {
				printf
					("\n\nWARNING: packet capture size is smaller than packet size, DETECTION MIGHT NOT WORK CORRECTLY OR EVEN CRASH\n\n");
				sleep(2);
				cap_warning_used = 1;
			}
		}

		if (iph->version != 4) {
			static u8 ipv4_warning_used = 0;
			if (ipv4_warning_used == 0) {
				printf("\n\nWARNING: only IPv4 packets are supported, all other packets will be discarded\n\n");
				sleep(2);
				ipv4_warning_used = 1;
			}
			return;
		}
		// process the packet
		packet_processing(time, iph, header->len - sizeof(struct ethhdr), header->len);
	}

}

static void runPcapLoop(void)
{
	if (_pcap_handle != NULL) {
		pcap_loop(_pcap_handle, -1, &pcap_packet_callback, NULL);
	}
}

int main(int argc, char **argv)
{
	parseOptions(argc, argv);

	setupDetection();

	openPcapFile();
	runPcapLoop();
	closePcapFile();

	printResults();

	terminateDetection();

	return 0;
}
