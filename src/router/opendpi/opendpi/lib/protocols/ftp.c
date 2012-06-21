/*
 * ftp.c
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
#include "ipq_utils.h"

#ifdef IPOQUE_PROTOCOL_FTP


static void ipoque_int_ftp_add_connection(struct ipoque_detection_module_struct
										  *ipoque_struct)
{

	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FTP, IPOQUE_REAL_PROTOCOL);
}

/**
 * checks for possible FTP command
 * not all valid commands are tested, it just need to be 3 or 4 characters followed by a space if the
 * packet is longer
 *
 * this functions is not used to accept, just to not reject
 */
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 ipoque_int_check_possible_ftp_command(const struct ipoque_packet_struct *packet)
{
	if (packet->payload_packet_len < 3)
		return 0;

	if ((packet->payload[0] < 'a' || packet->payload[0] > 'z') &&
		(packet->payload[0] < 'A' || packet->payload[0] > 'Z'))
		return 0;
	if ((packet->payload[1] < 'a' || packet->payload[1] > 'z') &&
		(packet->payload[1] < 'A' || packet->payload[1] > 'Z'))
		return 0;
	if ((packet->payload[2] < 'a' || packet->payload[2] > 'z') &&
		(packet->payload[2] < 'A' || packet->payload[2] > 'Z'))
		return 0;

	if (packet->payload_packet_len > 3) {
		if ((packet->payload[3] < 'a' || packet->payload[3] > 'z') &&
			(packet->payload[3] < 'A' || packet->payload[3] > 'Z') && packet->payload[3] != ' ')
			return 0;

		if (packet->payload_packet_len > 4) {
			if (packet->payload[3] != ' ' && packet->payload[4] != ' ')
				return 0;
		}
	}

	return 1;
}

/**
 * ftp replies are are 3-digit number followed by space or hyphen
 */
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 ipoque_int_check_possible_ftp_reply(const struct ipoque_packet_struct *packet)
{
	if (packet->payload_packet_len < 5)
		return 0;

	if (packet->payload[3] != ' ' && packet->payload[3] != '-')
		return 0;

	if (packet->payload[0] < '0' || packet->payload[0] > '9')
		return 0;
	if (packet->payload[1] < '0' || packet->payload[1] > '9')
		return 0;
	if (packet->payload[2] < '0' || packet->payload[2] > '9')
		return 0;

	return 1;
}

/**
 * check for continuation replies
 * there is no real indication whether it is a continuation message, we just
 * require that there are at least 5 ascii characters
 */
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 ipoque_int_check_possible_ftp_continuation_reply(const struct ipoque_packet_struct *packet)
{
	u16 i;

	if (packet->payload_packet_len < 5)
		return 0;

	for (i = 0; i < 5; i++) {
		if (packet->payload[i] < ' ' || packet->payload[i] > 127)
			return 0;
	}

	return 1;
}

/*
 * these are the commands we tracking and expecting to see
 */
enum {
	FTP_USER_CMD = 1 << 0,
	FTP_FEAT_CMD = 1 << 1,
	FTP_COMMANDS = ((1 << 2) - 1),
	FTP_220_CODE = 1 << 2,
	FTP_331_CODE = 1 << 3,
	FTP_211_CODE = 1 << 4,
	FTP_CODES = ((1 << 5) - 1 - FTP_COMMANDS)
};

/*
  return 0 if nothing has been detected
  return 1 if a pop packet
*/

static u8 search_ftp(struct ipoque_detection_module_struct *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	u8 current_ftp_code = 0;

//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;


	/* initiate client direction flag */
	if (flow->packet_counter == 1) {
		if (flow->l4.tcp.seen_syn) {
			flow->l4.tcp.ftp_client_direction = flow->setup_packet_direction;
		} else {
			/* no syn flag seen so guess */
			if (packet->payload_packet_len > 0) {
				if (packet->payload[0] >= '0' && packet->payload[0] <= '9') {
					/* maybe server side */
					flow->l4.tcp.ftp_client_direction = 1 - packet->packet_direction;
				} else {
					flow->l4.tcp.ftp_client_direction = packet->packet_direction;
				}
			}
		}
	}

	if (packet->packet_direction == flow->l4.tcp.ftp_client_direction) {
		if (packet->payload_packet_len > IPQ_STATICSTRING_LEN("USER ") &&
			(memcmp(packet->payload, "USER ", IPQ_STATICSTRING_LEN("USER ")) == 0 ||
			 memcmp(packet->payload, "user ", IPQ_STATICSTRING_LEN("user ")) == 0)) {

			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP: found USER command\n");
			flow->l4.tcp.ftp_codes_seen |= FTP_USER_CMD;
			current_ftp_code = FTP_USER_CMD;
		} else if (packet->payload_packet_len >= IPQ_STATICSTRING_LEN("FEAT") &&
				   (memcmp(packet->payload, "FEAT", IPQ_STATICSTRING_LEN("FEAT")) == 0 ||
					memcmp(packet->payload, "feat", IPQ_STATICSTRING_LEN("feat")) == 0)) {

			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP: found FEAT command\n");
			flow->l4.tcp.ftp_codes_seen |= FTP_FEAT_CMD;
			current_ftp_code = FTP_FEAT_CMD;
		} else if (!ipoque_int_check_possible_ftp_command(packet)) {
			return 0;
		}
	} else {
		if (packet->payload_packet_len > IPQ_STATICSTRING_LEN("220 ") &&
			(memcmp(packet->payload, "220 ", IPQ_STATICSTRING_LEN("220 ")) == 0 ||
			 memcmp(packet->payload, "220-", IPQ_STATICSTRING_LEN("220-")) == 0)) {

			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP: found 220 reply code\n");
			flow->l4.tcp.ftp_codes_seen |= FTP_220_CODE;
			current_ftp_code = FTP_220_CODE;
		} else if (packet->payload_packet_len > IPQ_STATICSTRING_LEN("331 ") &&
				   (memcmp(packet->payload, "331 ", IPQ_STATICSTRING_LEN("331 ")) == 0 ||
					memcmp(packet->payload, "331-", IPQ_STATICSTRING_LEN("331-")) == 0)) {

			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP: found 331 reply code\n");
			flow->l4.tcp.ftp_codes_seen |= FTP_331_CODE;
			current_ftp_code = FTP_331_CODE;
		} else if (packet->payload_packet_len > IPQ_STATICSTRING_LEN("211 ") &&
				   (memcmp(packet->payload, "211 ", IPQ_STATICSTRING_LEN("211 ")) == 0 ||
					memcmp(packet->payload, "211-", IPQ_STATICSTRING_LEN("211-")) == 0)) {

			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP: found 211reply code\n");
			flow->l4.tcp.ftp_codes_seen |= FTP_211_CODE;
			current_ftp_code = FTP_211_CODE;
		} else if (!ipoque_int_check_possible_ftp_reply(packet)) {
			if ((flow->l4.tcp.ftp_codes_seen & FTP_CODES) == 0 ||
				(!ipoque_int_check_possible_ftp_continuation_reply(packet))) {
				return 0;
			}
		}
	}

	if ((flow->l4.tcp.ftp_codes_seen & FTP_COMMANDS) != 0 && (flow->l4.tcp.ftp_codes_seen & FTP_CODES) != 0) {

		IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP detected\n");
		ipoque_int_ftp_add_connection(ipoque_struct);
		return 1;
	}

	/* if no valid code has been seen for the first packets reject */
	if (flow->l4.tcp.ftp_codes_seen == 0 && flow->packet_counter > 3)
		return 0;

	/* otherwise wait more packets, wait more for traffic on known ftp port */
	if ((packet->packet_direction == flow->setup_packet_direction && packet->tcp && packet->tcp->dest == htons(21)) ||
		(packet->packet_direction != flow->setup_packet_direction && packet->tcp && packet->tcp->source == htons(21))) {
		/* flow to known ftp port */

		/* wait much longer if this was a 220 code, initial messages might be long */
		if (current_ftp_code == FTP_220_CODE) {
			if (flow->packet_counter > 40)
				return 0;
		} else {
			if (flow->packet_counter > 20)
				return 0;
		}
	} else {
		/* wait much longer if this was a 220 code, initial messages might be long */
		if (current_ftp_code == FTP_220_CODE) {
			if (flow->packet_counter > 20)
				return 0;
		} else {
			if (flow->packet_counter > 10)
				return 0;
		}
	}

	return 2;
}


static void search_passive_ftp_mode(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_id_struct *dst = ipoque_struct->dst;
	struct ipoque_id_struct *src = ipoque_struct->src;
	u16 plen;
	u8 i;
	u32 ftp_ip;


// TODO check if normal passive mode also needs adaption for ipv6
	if (packet->payload_packet_len > 3 && ipq_mem_cmp(packet->payload, "227 ", 4) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP passive mode initial string\n");

		plen = 4;				//=4 for "227 "
		while (1) {
			if (plen >= packet->payload_packet_len) {
				IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG,
						"plen >= packet->payload_packet_len, return\n");
				return;
			}
			if (packet->payload[plen] == '(') {
				IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "found (. break.\n");
				break;
			}
/*			if (!isalnum(packet->payload[plen])) {
				IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "no alpha numeric symbol --> break.\n");
				return;
			}*/
			plen++;
		}
		plen++;

		if (plen >= packet->payload_packet_len)
			return;


		ftp_ip = 0;
		for (i = 0; i < 4; i++) {
			u16 oldplen = plen;
			ftp_ip =
				(ftp_ip << 8) +
				ipq_bytestream_to_number(&packet->payload[plen], packet->payload_packet_len - plen, &plen);
			if (oldplen == plen || plen >= packet->payload_packet_len) {
				IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP passive mode %u value parse failed\n",
						i);
				return;
			}
			if (packet->payload[plen] != ',') {

				IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG,
						"FTP passive mode %u value parse failed, char ',' is missing\n", i);
				return;
			}
			plen++;
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG,
					"FTP passive mode %u value parsed, ip is now: %u\n", i, ftp_ip);

		}
		if (dst != NULL) {
			dst->ftp_ip.ipv4 = htonl(ftp_ip);
			dst->ftp_timer = packet->tick_timestamp;
			dst->ftp_timer_set = 1;
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "saved ftp_ip, ftp_timer, ftp_timer_set to dst");
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP PASSIVE MODE FOUND: use  Server %s\n",
					ipq_get_ip_string(ipoque_struct, &dst->ftp_ip));
		}
		if (src != NULL) {
			src->ftp_ip.ipv4 = packet->iph->daddr;
			src->ftp_timer = packet->tick_timestamp;
			src->ftp_timer_set = 1;
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "saved ftp_ip, ftp_timer, ftp_timer_set to src");
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP PASSIVE MODE FOUND: use  Server %s\n",
					ipq_get_ip_string(ipoque_struct, &src->ftp_ip));
		}
		return;
	}

	if (packet->payload_packet_len > 34 && ipq_mem_cmp(packet->payload, "229 Entering Extended Passive Mode", 34) == 0) {
		if (dst != NULL) {
			ipq_packet_src_ip_get(packet, &dst->ftp_ip);
			dst->ftp_timer = packet->tick_timestamp;
			dst->ftp_timer_set = 1;
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "saved ftp_ip, ftp_timer, ftp_timer_set to dst");
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG,
					"FTP Extended PASSIVE MODE FOUND: use Server %s\n", ipq_get_ip_string(ipoque_struct, &dst->ftp_ip));
		}
		if (src != NULL) {
			ipq_packet_dst_ip_get(packet, &src->ftp_ip);
			src->ftp_timer = packet->tick_timestamp;
			src->ftp_timer_set = 1;
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "saved ftp_ip, ftp_timer, ftp_timer_set to src");
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG,
					"FTP Extended PASSIVE MODE FOUND: use Server %s\n", ipq_get_ip_string(ipoque_struct, &src->ftp_ip));
		}
		return;
	}
}


static void search_active_ftp_mode(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	if (packet->payload_packet_len > 5
		&& (ipq_mem_cmp(packet->payload, "PORT ", 5) == 0 || ipq_mem_cmp(packet->payload, "EPRT ", 5) == 0)) {

		//src->local_ftp_data_port = htons(data_port_number);
		if (src != NULL) {
			ipq_packet_dst_ip_get(packet, &src->ftp_ip);
			src->ftp_timer = packet->tick_timestamp;
			src->ftp_timer_set = 1;
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP ACTIVE MODE FOUND, command is %.*s\n", 4,
					packet->payload);
		}
		if (dst != NULL) {
			ipq_packet_src_ip_get(packet, &dst->ftp_ip);
			dst->ftp_timer = packet->tick_timestamp;
			dst->ftp_timer_set = 1;
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "FTP ACTIVE MODE FOUND, command is %.*s\n", 4,
					packet->payload);
		}
	}
	return;
}


void ipoque_search_ftp_tcp(struct ipoque_detection_module_struct *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;



	if (src != NULL && ipq_packet_dst_ip_eql(packet, &src->ftp_ip)
		&& packet->tcp->syn != 0 && packet->tcp->ack == 0
		&& packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_UNKNOWN
		&& IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask,
											  IPOQUE_PROTOCOL_FTP) != 0 && src->ftp_timer_set != 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "possible ftp data, src!= 0.\n");

		if (((IPOQUE_TIMESTAMP_COUNTER_SIZE)
			 (packet->tick_timestamp - src->ftp_timer)) >= ipoque_struct->ftp_connection_timeout) {
			src->ftp_timer_set = 0;
		} else if (ntohs(packet->tcp->dest) > 1024
				   && (ntohs(packet->tcp->source) > 1024 || ntohs(packet->tcp->source) == 20)) {
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "detected FTP data stream.\n");
			ipoque_int_ftp_add_connection(ipoque_struct);
			return;
		}
	}

	if (dst != NULL && ipq_packet_src_ip_eql(packet, &dst->ftp_ip)
		&& packet->tcp->syn != 0 && packet->tcp->ack == 0
		&& packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_UNKNOWN
		&& IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask,
											  IPOQUE_PROTOCOL_FTP) != 0 && dst->ftp_timer_set != 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "possible ftp data; dst!= 0.\n");

		if (((IPOQUE_TIMESTAMP_COUNTER_SIZE)
			 (packet->tick_timestamp - dst->ftp_timer)) >= ipoque_struct->ftp_connection_timeout) {
			dst->ftp_timer_set = 0;

		} else if (ntohs(packet->tcp->dest) > 1024
				   && (ntohs(packet->tcp->source) > 1024 || ntohs(packet->tcp->source) == 20)) {
			IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "detected FTP data stream.\n");
			ipoque_int_ftp_add_connection(ipoque_struct);
			return;
		}
	}
	// ftp data asymmetrically


	/* skip packets without payload */
	if (packet->payload_packet_len == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG,
				"FTP test skip because of data connection or zero byte packet_payload.\n");
		return;
	}
	/* skip excluded connections */

	// we test for FTP connection and search for passive mode
	if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_FTP) {
		IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG,
				"detected ftp command mode. going to test data mode.\n");
		search_passive_ftp_mode(ipoque_struct);

		search_active_ftp_mode(ipoque_struct);
		return;
	}


	if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_UNKNOWN && search_ftp(ipoque_struct) != 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "unknown. need next packet.\n");

		return;
	}
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_FTP);
	IPQ_LOG(IPOQUE_PROTOCOL_FTP, ipoque_struct, IPQ_LOG_DEBUG, "exclude ftp.\n");

}

#endif
