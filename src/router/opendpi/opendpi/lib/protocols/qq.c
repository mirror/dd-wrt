/*
 * qq.c
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


#include "ipq_utils.h"

#ifdef IPOQUE_PROTOCOL_QQ

static void ipoque_int_qq_add_connection(struct ipoque_detection_module_struct *ipoque_struct,
										 ipoque_protocol_type_t protocol_type)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_QQ, protocol_type);
}


/*
 * a qq client packet looks like this:
 *
 * TCP packets starts with 16 bit length, then the normal packets follows
 * 
 * 0 1 byte packet tag (usually 0x02)
 * 1 2 byte client tag (client version)
 * 3 2 byte command
 * 5 2 byte sequence number
 * 7 4 byte userid
 * 11 x bytes data
 * LAST 1 byte packet tail (usually 0x03)
 *
 * a qq server packet looks like this:
 *
 * TCP packets starts with 16 bit length, then the normal packets follows
 * 
 * 0 1 byte packet tag (usually 0x02)
 * 1 2 byte source tag (client version, might also be a server id)
 * 3 2 byte command (usually reply to client request, so same command id)
 * 5 2 byte sequence number
 * LAST 1 byte packet tail (usually 0x03)
 *
 * NOTE: there are other qq versions which uses different packet types!
 */

/*
 * these are some currently known client ids (or server ids)
 * new ids might be added here if the traffic is really QQ
 */
static const u16 ipoque_valid_qq_versions[] = {
	0x0100, 0x05a5, 0x062e, 0x06d5, 0x072e, 0x0801, 0x087d, 0x08d2, 0x0961,
	0x0a1d, 0x0b07, 0x0b2f, 0x0b35, 0x0b37, 0x0c0b, 0x0c0d, 0x0c21, 0x0c49,
	0x0d05, 0x0d51, 0x0d55, 0x0d61, 0x0e1b, 0x0e35, 0x0f15, 0x0f4b, 0x0f5f,
	0x1105, 0x111b, 0x111d, 0x1131, 0x113f, 0x115b, 0x1203, 0x1205, 0x120b,
	0x1251, 0x1412, 0x1441, 0x1501, 0x1549, 0x163a, 0x1801, 0x180d, 0x1c27,
	0x1e0d
};

/**
 * this functions checks whether the packet is a valid qq packet
 * it can handle tcp and udp packets
 */
	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 ipoque_is_valid_qq_packet(const struct ipoque_packet_struct *packet)
{
	u8 real_start = 0;
	u16 command;
	u8 ids, found = 0;
	u16 version_id;

	if (packet->payload_packet_len < 9)
		return 0;

	/* for tcp the length is prefixed */
	if (packet->tcp) {
		if (ntohs(get_u16(packet->payload, 0)) != packet->payload_packet_len) {
			return 0;
		}
		real_start = 2;
	}

	/* packet usually starts with 0x02 */
	if (packet->payload[real_start] != 0x02) {
		return 0;
	}

	/* packet usually ends with 0x03 */
	if (packet->payload[packet->payload_packet_len - 1] != 0x03) {
		return 0;
	}

	version_id = ntohs(get_u16(packet->payload, real_start + 1));

	if (version_id == 0) {
		return 0;
	}

	/* check for known version id */
	for (ids = 0; ids < sizeof(ipoque_valid_qq_versions) / sizeof(ipoque_valid_qq_versions[0]); ids++) {
		if (version_id == ipoque_valid_qq_versions[ids]) {
			found = 1;
			break;
		}
	}

	if (!found)
		return 0;

	command = ntohs(get_u16(packet->payload, real_start + 3));

	/* these are some known commands, not all need to be checked
	   since many are used with already established connections */

	switch (command) {
	case 0x0091:				/* get server */
	case 0x00ba:				/* login token */
	case 0x00dd:				/* password verify */
	case 0x00e5:
	case 0x00a4:
	case 0x0030:
	case 0x001d:
	case 0x0001:
	case 0x0062:
	case 0x0002:
	case 0x0022:
	case 0x0029:
		break;
	default:
		return 0;
		break;
	}

	return 1;
}

/*
 * some file transfer packets look like this
 *
 * 0 1 byte packet tag (usually 0x04)
 * 1 2 byte client tag (client version)
 * 3 2 byte length (this is speculative)
 * LAST 1 byte packet tail (usually 0x03)
 *
 */
/**
 * this functions checks whether the packet is a valid qq file transfer packet
 * it can handle tcp and udp packets
 */
	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 ipoque_is_valid_qq_ft_packet(const struct ipoque_packet_struct *packet)
{
	u8 ids, found = 0;
	u16 version_id;

	if (packet->payload_packet_len < 9)
		return 0;

	/* file transfer packets may start with 0x00 (control), 0x03 (data), 0x04 (agent) */

	if (packet->payload[0] != 0x04 && packet->payload[0] != 0x03 && packet->payload[0] != 0x00) {
		return 0;
	}

	version_id = ntohs(get_u16(packet->payload, 1));

	if (version_id == 0) {
		return 0;
	}

	/* check for known version id */
	for (ids = 0; ids < sizeof(ipoque_valid_qq_versions) / sizeof(ipoque_valid_qq_versions[0]); ids++) {
		if (version_id == ipoque_valid_qq_versions[ids]) {
			found = 1;
			break;
		}
	}

	if (!found)
		return 0;

	if (packet->payload[0] == 0x04) {

		if (ntohs(get_u16(packet->payload, 3)) != packet->payload_packet_len) {
			return 0;
		}

		/* packet usually ends with 0x03 */
		if (packet->payload[packet->payload_packet_len - 1] != 0x03) {
			return 0;
		}
	} else if (packet->payload[0] == 0x03) {
		/* TODO currently not detected */
		return 0;
	} else if (packet->payload[0] == 0x00) {

		/* packet length check, there might be other lengths */
		if (packet->payload_packet_len != 84) {
			return 0;
		}

		/* packet usually ends with 0x0c ? */
		if (packet->payload[packet->payload_packet_len - 1] != 0x0c) {
			return 0;
		}
	}
	return 1;
}

static void ipoque_search_qq_udp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	static const u16 p8000_patt_02[12] =	// maybe version numbers
	{ 0x1549, 0x1801, 0x180d, 0x0961, 0x01501, 0x0e35, 0x113f, 0x0b37, 0x1131, 0x163a, 0x1e0d };
	u16 no_of_patterns = 11, index = 0;


	IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "search qq udp.\n");


	if (flow->qq_stage <= 3) {
		if ((packet->payload_packet_len == 27 && ntohs(get_u16(packet->payload, 0)) == 0x0300
			 && packet->payload[2] == 0x01)
			|| (packet->payload_packet_len == 84 && ((ntohs(get_u16(packet->payload, 0)) == 0x000e
													  && packet->payload[2] == 0x35)
													 || (ntohs(get_u16(packet->payload, 0)) == 0x0015
														 && packet->payload[2] == 0x01)
													 || (ntohs(get_u16(packet->payload, 0)) == 0x000b
														 && packet->payload[2] == 0x37)
													 || (ntohs(get_u16(packet->payload, 0)) == 0x0015
														 && packet->payload[2] == 0x49)))
			|| (packet->payload_packet_len > 10
				&& ((get_u16(packet->payload, 0) == htons(0x000b) && packet->payload[2] == 0x37)
					|| (get_u32(packet->payload, 0) == htonl(0x04163a00)
						&& packet->payload[packet->payload_packet_len - 1] == 0x03
						&& packet->payload[4] == packet->payload_packet_len)))) {
			/*
			   if (flow->qq_stage == 3 && flow->detected_protocol == IPOQUE_PROTOCOL_QQ) {
			   if (flow->packet_direction_counter[0] > 0 && flow->packet_direction_counter[1] > 0) {
			   flow->protocol_subtype = IPOQUE_PROTOCOL_QQ_SUBTYPE_AUDIO;
			   return;
			   } else if (flow->packet_counter < 10) {
			   return;
			   }
			   } */
			flow->qq_stage++;
			if (flow->qq_stage == 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
						"found qq udp pattern 030001 or 000e35 four times.\n");
				ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				return;
			}
			return;
		}
		if (packet->payload_packet_len > 2 && (packet->payload[0] == 0x02 || packet->payload[0] == 0x04)) {
			u16 pat = ntohs(get_u16(packet->payload, 1));
			for (index = 0; index < no_of_patterns; index++) {
				if (pat == p8000_patt_02[index] && packet->payload[packet->payload_packet_len - 1] == 0x03) {
					flow->qq_stage++;
					// maybe we can test here packet->payload[4] == packet->payload_packet_len
					if (flow->qq_stage == 3) {
						IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
								"found qq udp pattern 02 ... 03 four times.\n");
						/*
						   if (packet->payload[0] == 0x04) {
						   ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
						   return;
						   } */
						ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
						return;
					}
					return;
				}
			}
		}
		if (packet->payload_packet_len == 84 && (packet->payload[0] == 0 || packet->payload[0] == 0x03)) {
			u16 pat = ntohs(get_u16(packet->payload, 1));
			for (index = 0; index < no_of_patterns; index++) {
				if (pat == p8000_patt_02[index]) {
					flow->qq_stage++;
					/*
					   if (flow->qq_stage == 3 && flow->packet_direction_counter[0] > 0 &&
					   flow->packet_direction_counter[1] > 0) {
					   IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq udp pattern four times.\n");
					   ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
					   return;
					   } else */ if (flow->qq_stage == 3) {
						IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq udp pattern four times.\n");
						ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
						return;
					}
					return;
				}
			}
		}
		if (packet->payload_packet_len > 2 && packet->payload[0] == 0x04
			&& ((ntohs(get_u16(packet->payload, 1)) == 0x1549
				 || ntohs(get_u16(packet->payload, 1)) == 0x1801 || ntohs(get_u16(packet->payload, 1)) == 0x0961)
				||
				(packet->payload_packet_len > 16
				 && (ntohs(get_u16(packet->payload, 1)) == 0x180d || ntohs(get_u16(packet->payload, 1)) == 0x096d)
				 && ntohl(get_u32(packet->payload, 12)) == 0x28000000
				 && ntohs(get_u16(packet->payload, 3)) == packet->payload_packet_len))
			&& packet->payload[packet->payload_packet_len - 1] == 0x03) {
			flow->qq_stage++;
			if (flow->qq_stage == 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
						"found qq udp pattern 04 1159 ... 03 four times.\n");
				ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				return;
			}
			return;
		}
		if (packet->payload_packet_len > 2 && (packet->payload[0] == 0x06 || packet->payload[0] == 0x02)
			&& ntohs(get_u16(packet->payload, 1)) == 0x0100
			&& (packet->payload[packet->payload_packet_len - 1] == 0x00
				|| packet->payload[packet->payload_packet_len - 1] == 0x03)) {
			flow->qq_stage++;
			if (flow->qq_stage == 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
						"found qq udp pattern 02/06 0100 ... 03/00 four times.\n");
				ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				return;
			}
			return;
		}

		if (packet->payload_packet_len > 2 && (packet->payload[0] == 0x02)
			&& ntohs(get_u16(packet->payload, 1)) == 0x1131 && packet->payload[packet->payload_packet_len - 1] == 0x03) {
			flow->qq_stage++;
			if (flow->qq_stage == 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
						"found qq udp pattern 02 1131 ... 03 four times.\n");
				ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				return;
			}
			return;
		}

		if (packet->payload_packet_len > 5 && get_u16(packet->payload, 0) == htons(0x0203) &&
			ntohs(get_u16(packet->payload, 2)) == packet->payload_packet_len &&
			get_u16(packet->payload, 4) == htons(0x0b0b)) {
			flow->qq_stage++;
			if (flow->qq_stage == 3) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
						"found qq udp pattern 0203[packet_length_0b0b] three times.\n");
				ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				return;
			}
			return;
		}

		if (packet->udp->dest == htons(9000) || packet->udp->source == htons(9000)) {
			if (packet->payload_packet_len > 3
				&& ntohs(get_u16(packet->payload, 0)) == 0x0202
				&& ntohs(get_u16(packet->payload, 2)) == packet->payload_packet_len) {
				flow->qq_stage++;
				if (flow->qq_stage == 3) {
					IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
							"found qq udp pattern 02 02 <length> four times.\n");
					ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
					return;
				}
				return;
			}

		}
	}

	if (ipoque_is_valid_qq_packet(packet)) {
		flow->qq_stage++;
		if (flow->qq_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over udp.\n");
			ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq packet stage %d\n", flow->qq_stage);
		return;
	}

	if (ipoque_is_valid_qq_ft_packet(packet)) {
		flow->qq_stage++;
		if (flow->qq_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq ft over udp.\n");
			ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		return;
	}

	if (flow->qq_stage && flow->packet_counter <= 5) {
		return;
	}

	IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "QQ excluded\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_QQ);
}


	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 void ipoque_search_qq_tcp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;



	u16 i = 0;
//  u16 a = 0;

	IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "search qq tcp.\n");

	if (packet->payload_packet_len == 39 && get_u32(packet->payload, 0) == htonl(0x27000000) &&
		get_u16(packet->payload, 4) == htons(0x0014) && get_u32(packet->payload, 11) != 0 &&
		get_u16(packet->payload, packet->payload_packet_len - 2) == htons(0x0000)) {
		if (flow->qq_stage == 4) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp - maybe ft/audio/video.\n");
			ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		flow->qq_stage = 4;
		return;
	}

	if ((packet->payload_packet_len > 4 && ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len
		 && get_u16(packet->payload, 2) == htons(0x0212) && packet->payload[4] == 0x0b)
		|| (packet->payload_packet_len > 6 && packet->payload[0] == 0x02
			&& packet->payload[packet->payload_packet_len - 1] == 0x03
			&& ntohs(get_u16(packet->payload, 1)) == packet->payload_packet_len
			&& (get_u16(packet->payload, 3) == htons(0x0605) || get_u16(packet->payload, 3) == htons(0x0608))
			&& packet->payload[5] == 0x00)
		|| (packet->payload_packet_len > 9 && get_u32(packet->payload, 0) == htonl(0x04154900)
			&& get_l16(packet->payload, 4) == packet->payload_packet_len
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 9 && get_u32(packet->payload, 0) == htonl(0x040e3500)
			&& get_l16(packet->payload, 4) == packet->payload_packet_len
			&& packet->payload[9] == 0x33 && packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 9 && get_u32(packet->payload, 0) == htonl(0x040e0215)
			&& get_l16(packet->payload, 4) == packet->payload_packet_len
			&& packet->payload[9] == 0x33 && packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 6 && get_u32(packet->payload, 2) == htonl(0x020d5500)
			&& ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 6 && get_u16(packet->payload, 0) == htons(0x0418)
			&& packet->payload[2] == 0x01
			&& ntohs(get_u16(packet->payload, 3)) == packet->payload_packet_len
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 6 && get_u16(packet->payload, 0) == htons(0x0411)
			&& packet->payload[2] == 0x31
			&& ntohs(get_u16(packet->payload, 3)) == packet->payload_packet_len
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 6 && ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len
			&& get_u16(packet->payload, 2) == htons(0x0211) && packet->payload[4] == 0x31
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 6 && ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len
			&& get_u16(packet->payload, 2) == htons(0x0218) && packet->payload[4] == 0x01
			&& packet->payload[packet->payload_packet_len - 1] == 0x03)
		|| (packet->payload_packet_len > 10 && get_u32(packet->payload, 0) == htonl(0x04163a00)
			&& packet->payload[packet->payload_packet_len - 1] == 0x03
			&& packet->payload[4] == packet->payload_packet_len)
		) {
		flow->qq_stage++;
		if (flow->qq_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp.\n");
			ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		return;
	}

	if (ipoque_is_valid_qq_packet(packet)) {
		flow->qq_stage++;
		if (flow->qq_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp.\n");
			ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		return;
	}

	if (ipoque_is_valid_qq_ft_packet(packet)) {
		flow->qq_stage++;
		if (flow->qq_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq ft over tcp.\n");
			ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		return;
	}

	if (packet->payload_packet_len == 2) {
		flow->l4.tcp.qq_nxt_len = ntohs(get_u16(packet->payload, 0));
		return;
	}
	if (packet->payload_packet_len > 5 && (((flow->l4.tcp.qq_nxt_len == packet->payload_packet_len + 2)
											&& packet->payload[0] == 0x02
											&& packet->payload[packet->payload_packet_len - 1] == 0x03
											&& get_u16(packet->payload, 1) == htons(0x0f5f))
										   || (ntohs(get_u16(packet->payload, 0)) == packet->payload_packet_len
											   && packet->payload[2] == 0x02
											   && packet->payload[packet->payload_packet_len - 1] == 0x03
											   && get_u16(packet->payload, 3) == htons(0x0f5f)))) {
		flow->qq_stage++;
		if (flow->qq_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq udp pattern 02 ... 03 four times.\n");
			ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		return;

	}
	if (packet->payload_packet_len > 2 && packet->payload[0] == 0x04 && ((get_u16(packet->payload, 1) == htons(0x1549)
																		  || get_u16(packet->payload,
																					 1) == htons(0x1801)
																		  || get_u16(packet->payload,
																					 1) == htons(0x0961))
																		 || (packet->payload_packet_len > 16
																			 && (get_u16(packet->payload, 1) ==
																				 htons(0x180d)
																				 || get_u16(packet->payload,
																							1) == htons(0x096d))
																			 && get_u32(packet->payload,
																						12) == htonl(0x28000000)
																			 && ntohs(get_u16(packet->payload, 3)) ==
																			 packet->payload_packet_len))
		&& packet->payload[packet->payload_packet_len - 1] == 0x03) {
		flow->qq_stage++;
		if (flow->qq_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG,
					"found qq udp pattern 04 1159 ... 03 four times.\n");
			ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
			return;
		}
		return;
	}



	if (packet->payload_packet_len > 100
		&& ((ipq_mem_cmp(packet->payload, "GET", 3) == 0) || (ipq_mem_cmp(packet->payload, "POST", 4) == 0))) {
		IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found GET or POST.\n");
		if (memcmp(packet->payload, "GET /qqfile/qq", 14) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp GET /qqfile/qq.\n");
			ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		}
		ipq_parse_packet_line_info(ipoque_struct);

		if (packet->user_agent_line.ptr != NULL
			&& (packet->user_agent_line.len > 7 && memcmp(packet->user_agent_line.ptr, "QQClient", 8) == 0)) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp GET...QQClient\n");
			ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
			return;
		}
		for (i = 0; i < packet->parsed_lines; i++) {
			if (packet->line[i].len > 3 && memcmp(packet->line[i].ptr, "QQ: ", 4) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp GET...QQ: \n");
				ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				return;
			}
		}
		if (packet->host_line.ptr != NULL) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "host line ptr\n");
			if (packet->host_line.len > 11 && memcmp(&packet->host_line.ptr[0], "www.qq.co.za", 12) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq over tcp Host: www.qq.co.za\n");
				ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_CORRELATED_PROTOCOL);
				return;
			}
		}
	}
	if (flow->qq_stage == 0 && packet->payload_packet_len == 82
		&& get_u32(packet->payload, 0) == htonl(0x0000004e) && get_u32(packet->payload, 4) == htonl(0x01010000)) {
		for (i = 8; i < 82; i++) {
			if (packet->payload[i] != 0x00) {
				break;
			}
			if (i == 81) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq Mail.\n");
				ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				return;
			}
		}
	}
	if (flow->qq_stage == 0 && packet->payload_packet_len == 182 && get_u32(packet->payload, 0) == htonl(0x000000b2)
		&& get_u32(packet->payload, 4) == htonl(0x01020000)
		&& get_u32(packet->payload, 8) == htonl(0x04015151) && get_u32(packet->payload, 12) == htonl(0x4d61696c)) {
		IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq Mail.\n");
		ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
		return;
	}
	if (packet->payload_packet_len == 204 && flow->qq_stage == 0 && get_u32(packet->payload, 200) == htonl(0xfbffffff)) {
		for (i = 0; i < 200; i++) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "i = %u\n", i);
			if (packet->payload[i] != 0) {
				break;
			}
			if (i == 199) {
				IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found qq chat or file transfer\n");
				ipoque_int_qq_add_connection(ipoque_struct, IPOQUE_REAL_PROTOCOL);
				return;
			}
		}
	}
#ifdef IPOQUE_PROTOCOL_HTTP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_HTTP) != 0) {
#endif							/* IPOQUE_PROTOCOL_HTTP */

		IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_QQ);
		IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "QQ tcp excluded; len %u\n",
				packet->payload_packet_len);

#ifdef IPOQUE_PROTOCOL_HTTP
	}
#endif							/* IPOQUE_PROTOCOL_HTTP */

}


void ipoque_search_qq(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;


	if (packet->udp != NULL && flow->detected_protocol_stack[0] != IPOQUE_PROTOCOL_QQ)
		ipoque_search_qq_udp(ipoque_struct);

	if (packet->tcp != NULL && flow->detected_protocol_stack[0] != IPOQUE_PROTOCOL_QQ)
		ipoque_search_qq_tcp(ipoque_struct);
}

#endif
