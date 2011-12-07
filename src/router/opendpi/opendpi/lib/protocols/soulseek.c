/*
 * soulseek.c
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

#ifdef IPOQUE_PROTOCOL_SOULSEEK

static void ipoque_int_soulseek_add_connection(struct ipoque_detection_module_struct
											   *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_SOULSEEK, IPOQUE_REAL_PROTOCOL);

	if (src != NULL) {
		src->soulseek_last_safe_access_time = packet->tick_timestamp;
	}
	if (dst != NULL) {
		dst->soulseek_last_safe_access_time = packet->tick_timestamp;
	}

	return;
}

void ipoque_search_soulseek_tcp(struct ipoque_detection_module_struct
								*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG, "Soulseek: search soulseec tcp \n");


	if (packet->detected_protocol_stack[0] == IPOQUE_PROTOCOL_SOULSEEK) {
		IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG, "packet marked as Soulseek\n");
		if (src != NULL)
			IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG,
					"  SRC bitmask: %u, packet tick %llu , last safe access timestamp: %llu\n",
					IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_SOULSEEK)
					!= 0 ? 1 : 0, (u64) packet->tick_timestamp, (u64) src->soulseek_last_safe_access_time);
		if (dst != NULL)
			IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG,
					"  DST bitmask: %u, packet tick %llu , last safe ts: %llu\n",
					IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_SOULSEEK)
					!= 0 ? 1 : 0, (u64) packet->tick_timestamp, (u64) dst->soulseek_last_safe_access_time);

		if (packet->payload_packet_len == 431) {
			if (dst != NULL) {
				dst->soulseek_last_safe_access_time = packet->tick_timestamp;
			}
			return;
		}
		if (packet->payload_packet_len == 12 && get_l32(packet->payload, 4) == 0x02) {
			if (src != NULL) {
				src->soulseek_last_safe_access_time = packet->tick_timestamp;
				if (packet->tcp != NULL && src->soulseek_listen_port == 0) {
					src->soulseek_listen_port = get_l32(packet->payload, 8);
					return;
				}
			}
		}

		if (src != NULL && ((IPOQUE_TIMESTAMP_COUNTER_SIZE)
							(packet->tick_timestamp -
							 src->soulseek_last_safe_access_time) <
							ipoque_struct->soulseek_connection_ip_tick_timeout)) {
			IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG,
					"Soulseek: SRC update last safe access time and SKIP_FOR_TIME \n");
			src->soulseek_last_safe_access_time = packet->tick_timestamp;
		}

		if (dst != NULL && ((IPOQUE_TIMESTAMP_COUNTER_SIZE)
							(packet->tick_timestamp -
							 dst->soulseek_last_safe_access_time) <
							ipoque_struct->soulseek_connection_ip_tick_timeout)) {
			IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG,
					"Soulseek: DST update last safe access time and SKIP_FOR_TIME \n");
			dst->soulseek_last_safe_access_time = packet->tick_timestamp;
		}
	}


	if (dst != NULL && dst->soulseek_listen_port != 0 && dst->soulseek_listen_port == ntohs(packet->tcp->dest)
		&& ((IPOQUE_TIMESTAMP_COUNTER_SIZE)
			(packet->tick_timestamp - dst->soulseek_last_safe_access_time) <
			ipoque_struct->soulseek_connection_ip_tick_timeout)) {
		IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG,
				"Soulseek: Plain detection on Port : %u packet_tick_timestamp: %u soulseeek_last_safe_access_time: %u soulseek_connection_ip_ticktimeout: %u\n",
				dst->soulseek_listen_port, packet->tick_timestamp,
				dst->soulseek_last_safe_access_time, ipoque_struct->soulseek_connection_ip_tick_timeout);
		ipoque_int_soulseek_add_connection(ipoque_struct);
		return;
	}

	if (flow->l4.tcp.soulseek_stage == 0) {

		u32 index = 0;

		if (packet->payload_packet_len >= 12 && packet->payload_packet_len < 300 && get_l32(packet->payload, 4) == 1) {
			while (!get_u16(packet->payload, index + 2)
				   && (index + get_l32(packet->payload, index)) < packet->payload_packet_len - 4) {
				if (get_l32(packet->payload, index) < 8)	/*Minimum soulsek  login msg is 8B */
					break;

				if (index + get_l32(packet->payload, index) + 4 <= index) {
					/* avoid overflow */
					break;
				}

				index += get_l32(packet->payload, index) + 4;
			}
			if (index + get_l32(packet->payload, index) ==
				packet->payload_packet_len - 4 && !get_u16(packet->payload, 10)) {
				/*This structure seems to be soulseek proto */
				index = get_l32(packet->payload, 8) + 12;	// end of "user name"
				if ((index + 4) <= packet->payload_packet_len && !get_u16(packet->payload, index + 2))	// for passwd len
				{
					index += get_l32(packet->payload, index) + 4;	//end of  "Passwd"
					if ((index + 4 + 4) <= packet->payload_packet_len && !get_u16(packet->payload, index + 6))	// to read version,hashlen
					{
						index += get_l32(packet->payload, index + 4) + 8;	// enf of "hash value"
						if (index == get_l32(packet->payload, 0)) {
							IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK,
									ipoque_struct, IPQ_LOG_DEBUG, "Soulseek Login Detected\n");
							ipoque_int_soulseek_add_connection(ipoque_struct);
							return;
						}
					}
				}
			}
		}
		if (packet->payload_packet_len > 8
			&& packet->payload_packet_len < 200 && get_l32(packet->payload, 0) == packet->payload_packet_len - 4) {
			//Server Messages:
			const u32 msgcode = get_l32(packet->payload, 4);

			if (msgcode == 0x7d) {
				flow->l4.tcp.soulseek_stage = 1 + packet->packet_direction;
				IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG, "Soulseek Messages Search\n");
				return;
			} else if (msgcode == 0x02 && packet->payload_packet_len == 12) {
				const u32 soulseek_listen_port = get_l32(packet->payload, 8);

				if (src != NULL) {
					src->soulseek_last_safe_access_time = packet->tick_timestamp;

					if (packet->tcp != NULL && src->soulseek_listen_port == 0) {
						src->soulseek_listen_port = soulseek_listen_port;
						IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct,
								IPQ_LOG_DEBUG, "\n Listen Port Saved : %u", src->soulseek_listen_port);
						ipoque_int_soulseek_add_connection(ipoque_struct);
						return;
					}
				}

			}
			//Peer Messages  : Peer Init Message Detection
			if (get_l32(packet->payload, 0) == packet->payload_packet_len - 4) {
				const u32 typelen = get_l32(packet->payload, packet->payload_packet_len - 9);
				const u8 type = packet->payload[packet->payload_packet_len - 5];
				const u32 namelen = get_l32(packet->payload, 5);
				if (packet->payload[4] == 0x01 && typelen == 1
					&& namelen <= packet->payload_packet_len
					&& (4 + 1 + 4 + namelen + 4 + 1 + 4) ==
					packet->payload_packet_len && (type == 'F' || type == 'P' || type == 'D')) {
					IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG, "soulseek detected\n");
					ipoque_int_soulseek_add_connection(ipoque_struct);
					return;
				}
				IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG, "1\n");
			}
			IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG, "3\n");
			//Peer Message : Pierce Firewall
			if (packet->payload_packet_len == 9 && get_l32(packet->payload, 0) == 5
				&& packet->payload[4] <= 0x10 && get_u32(packet->payload, 5) != 0x00000000) {
				flow->l4.tcp.soulseek_stage = 1 + packet->packet_direction;
				IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_TRACE, "Soulseek Size 9 Pierce Firewall\n");
				return;
			}

		}

		if (packet->payload_packet_len > 25 && packet->payload[4] == 0x01 && !get_u16(packet->payload, 7)
			&& !get_u16(packet->payload, 2)) {
			const u32 usrlen = get_l32(packet->payload, 5);

			if (usrlen <= packet->payload_packet_len - 4 + 1 + 4 + 4 + 1 + 4) {
				const u32 typelen = get_l32(packet->payload, 4 + 1 + 4 + usrlen);
				const u8 type = packet->payload[4 + 1 + 4 + usrlen + 4];
				if (typelen == 1 && (type == 'F' || type == 'P' || type == 'D')) {
					IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct,
							IPQ_LOG_DEBUG, "soulseek detected Pattern command(D|P|F).\n");
					ipoque_int_soulseek_add_connection(ipoque_struct);
					return;
				}
			}
		}

	} else if (flow->l4.tcp.soulseek_stage == 2 - packet->packet_direction) {
		if (packet->payload_packet_len > 8) {
			if ((packet->payload[0] || packet->payload[1]) && get_l32(packet->payload, 4) == 9) {
				/* 9 is search result */
				IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG, "soulseek detected Second Pkt\n");
				ipoque_int_soulseek_add_connection(ipoque_struct);
				return;
			}
			if (get_l32(packet->payload, 0) == packet->payload_packet_len - 4) {
				const u32 msgcode = get_l32(packet->payload, 4);
				if (msgcode == 0x03 && packet->payload_packet_len >= 12)	//Server Message : Get Peer Address
				{
					const u32 usrlen = get_l32(packet->payload, 8);
					if (usrlen <= packet->payload_packet_len && 4 + 4 + 4 + usrlen == packet->payload_packet_len) {
						IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct,
								IPQ_LOG_DEBUG, "Soulseek Request Get Peer Address Detected\n");
						ipoque_int_soulseek_add_connection(ipoque_struct);
						return;
					}
				}
			}
		}

		if (packet->payload_packet_len == 8 && get_l32(packet->payload, 4) == 0x00000004) {
			IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG, "soulseek detected\n");
			ipoque_int_soulseek_add_connection(ipoque_struct);
			return;
		}

		if (packet->payload_packet_len == 4
			&& get_u16(packet->payload, 2) == 0x00 && get_u16(packet->payload, 0) != 0x00) {
			IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG, "soulseek detected\n");
			ipoque_int_soulseek_add_connection(ipoque_struct);
			return;
		} else if (packet->payload_packet_len == 4) {
			flow->l4.tcp.soulseek_stage = 3;
			return;
		}
	} else if (flow->l4.tcp.soulseek_stage == 1 + packet->packet_direction) {
		if (packet->payload_packet_len > 8) {
			if (packet->payload[4] == 0x03 && get_l32(packet->payload, 5) == 0x00000031) {
				IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct,
						IPQ_LOG_DEBUG, "soulseek detected Second Pkt with SIGNATURE :: 0x0331000000 \n");
				ipoque_int_soulseek_add_connection(ipoque_struct);
				return;
			}
		}
	}
	if (flow->l4.tcp.soulseek_stage == 3 && packet->payload_packet_len == 8 && !get_u32(packet->payload, 4)) {

		IPQ_LOG(IPOQUE_PROTOCOL_SOULSEEK, ipoque_struct, IPQ_LOG_DEBUG, "soulseek detected bcz of 8B  pkt\n");
		ipoque_int_soulseek_add_connection(ipoque_struct);
		return;
	}
	if (flow->l4.tcp.soulseek_stage && flow->packet_counter < 11) {
	} else {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SOULSEEK);
	}
}

#endif
