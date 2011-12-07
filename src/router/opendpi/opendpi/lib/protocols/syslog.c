/*
 * syslog.c
 * Copyright (C) 2009-2011 by ipoque GmbH
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


#include "ipq_protocols.h"
#ifdef IPOQUE_PROTOCOL_SYSLOG

static void ipoque_int_syslog_add_connection(struct ipoque_detection_module_struct
											 *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_SYSLOG, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_syslog(struct ipoque_detection_module_struct
						  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	u8 i;

	IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG, "search syslog\n");

	if (packet->payload_packet_len > 20 && packet->payload_packet_len <= 1024 && packet->payload[0] == '<') {
		IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG, "checked len>20 and <1024 and first symbol=<.\n");
		i = 1;

		for (;;) {
			if (packet->payload[i] < '0' || packet->payload[i] > '9' || i++ > 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG,
						"read symbols while the symbol is a number.\n");
				break;
			}
		}

		if (packet->payload[i++] != '>') {
			IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG, "there is no > following the number.\n");
			IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SYSLOG);
			return;
		} else {
			IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG, "a > following the number.\n");
		}

		if (packet->payload[i] == 0x20) {
			IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG, "a blank following the >: increment i.\n");
			i++;
		} else {
			IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG, "no blank following the >: do nothing.\n");
		}

		/* check for "last message repeated" */
		if (i + sizeof("last message") - 1 <= packet->payload_packet_len &&
			memcmp(packet->payload + i, "last message", sizeof("last message") - 1) == 0) {

			IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG, "found syslog by 'last message' string.\n");

			ipoque_int_syslog_add_connection(ipoque_struct);

			return;
		} else if (i + sizeof("snort: ") - 1 <= packet->payload_packet_len &&
				   memcmp(packet->payload + i, "snort: ", sizeof("snort: ") - 1) == 0) {

			/* snort events */

			IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG, "found syslog by 'snort: ' string.\n");

			ipoque_int_syslog_add_connection(ipoque_struct);

			return;
		}

		if (ipq_mem_cmp(&packet->payload[i], "Jan", 3) != 0
			&& ipq_mem_cmp(&packet->payload[i], "Feb", 3) != 0
			&& ipq_mem_cmp(&packet->payload[i], "Mar", 3) != 0
			&& ipq_mem_cmp(&packet->payload[i], "Apr", 3) != 0
			&& ipq_mem_cmp(&packet->payload[i], "May", 3) != 0
			&& ipq_mem_cmp(&packet->payload[i], "Jun", 3) != 0
			&& ipq_mem_cmp(&packet->payload[i], "Jul", 3) != 0
			&& ipq_mem_cmp(&packet->payload[i], "Aug", 3) != 0
			&& ipq_mem_cmp(&packet->payload[i], "Sep", 3) != 0
			&& ipq_mem_cmp(&packet->payload[i], "Oct", 3) != 0
			&& ipq_mem_cmp(&packet->payload[i], "Nov", 3) != 0 && ipq_mem_cmp(&packet->payload[i], "Dec", 3) != 0) {


			IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG,
					"no month-shortname following: syslog excluded.\n");

			IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SYSLOG);

			return;

		} else {

			IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG,
					"a month-shortname following: syslog detected.\n");

			ipoque_int_syslog_add_connection(ipoque_struct);

			return;
		}
	}
	IPQ_LOG(IPOQUE_PROTOCOL_SYSLOG, ipoque_struct, IPQ_LOG_DEBUG, "no syslog detected.\n");

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SYSLOG);
}

#endif
