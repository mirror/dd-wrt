/*
 * bittorrent.c
 * Copyright (C) 2009-2010 by ipoque GmbH
 * Copyright (C) 2011 Felix Fietkau <nbd@openwrt.org>
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
#ifdef IPOQUE_PROTOCOL_BITTORRENT

static u8 ipoque_int_search_bittorrent_tcp_zero(struct ipoque_detection_module_struct
												*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	const u8 *p, *end, *line;
	int len;

	if (packet->payload_packet_len > 20) {
		/* test for match 0x13+"BitTorrent protocol" */
		if (packet->payload[0] == 0x13) {
			if (memcmp(&packet->payload[1], "BitTorrent protocol", 19) == 0) {
				IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT,
								   ipoque_struct, IPQ_LOG_TRACE, "BT: plain BitTorrent protocol detected\n");
				ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BITTORRENT);
				return 1;
			}
		}
	}

	if (packet->payload_packet_len > 23 && memcmp(packet->payload, "GET /webseed?info_hash=", 23) == 0) {
		IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
						   IPQ_LOG_TRACE, "BT: plain webseed BitTorrent protocol detected\n");
		ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BITTORRENT);
		return 1;
	}
	/* seen Azureus as server for webseed, possibly other servers existing, to implement */
	/* is Server: hypertracker Bittorrent? */
/* no asymmetric detection possible for answer of pattern "GET /data?fid=". */
	if (packet->payload_packet_len > 60
		&& memcmp(packet->payload, "GET /data?fid=", 14) == 0 && memcmp(&packet->payload[54], "&size=", 6) == 0) {
		IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
						   IPQ_LOG_TRACE, "BT: plain Bitcomet persistent seed protocol detected\n");
		ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BITTORRENT);
		return 1;
	}


	if (packet->payload_packet_len > 50 && memcmp(packet->payload, "GET ", 4) == 0) {
		const u8 *ptr;
		bool bitcomet_check = true;
		int i;
		u16 a = 0;

		for (p = packet->payload, end = p + packet->payload_packet_len, i = 0;
		     get_next_line(&p, end, &line, &len);i++) {

			if (len > 19 && !strncasecmp((const char *) line, "user-agent:", 11)) {
				const u8 *ua = line + 11;
				len -= 11;

				if (*ua == ' ') {
					ua++;
					len--;
				}

				if ((len > 8 && !memcmp(ua, "Azureus ", 8)) ||
				    (len >= 10 && !memcmp(ua, "BitTorrent", 10)) ||
				    (len >= 11 && !memcmp(ua, "BTWebClient", 11)) ||
				    (len >= 9 && !memcmp(ua, "Shareaza ", 9))) {
					IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
							   IPQ_LOG_TRACE, "Azureus /Bittorrent user agent line detected\n");
					ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BITTORRENT);
					return 1;
				}
				if (len < 12 || memcmp(ua, "Mozilla/4.0 ", 12) != 0)
				    bitcomet_check = false;
			}

			if (len > 5 && !strncasecmp((const char *) line, "host:", 5)) {
				const u8 *host = line + 5;
				len -= 5;

				if (*host == ' ') {
					host++;
					len--;
				}
				if (len >= 9 && memcmp(host, "ip2p.com:", 9) == 0) {
					IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT,
							   ipoque_struct, IPQ_LOG_TRACE,
							   "BT: Warez - Plain BitTorrent protocol detected due to Host: ip2p.com: pattern\n");
					ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BITTORRENT);
					return 1;
				}
			}

#define BITCOMET_CHECK(_i, _str) \
	case _i: \
		if (len < sizeof(_str) - 1 || memcmp(line, _str, sizeof(_str) - 1) != 0) \
			bitcomet_check = false; \
		break


			if (bitcomet_check) {
				switch (i) {
				default:
					break;
				BITCOMET_CHECK(2, "Keep-Alive: 300");
				BITCOMET_CHECK(3, "Connection: Keep-alive");
				case 4:
					if (len < 11 ||
					    (memcmp(line, "Accpet: */*", 11) != 0 &&
					     memcmp(line, "Accept: */*", 11) != 0))
						bitcomet_check = false;
					break;
				BITCOMET_CHECK(5, "Range: bytes=");
				BITCOMET_CHECK(7, "Pragma: no-cache");
				BITCOMET_CHECK(8, "Cache-Control: no-cache");
				}
			}
			if (i > 12) {
				bitcomet_check = false;
				break;
			}
		}
		if (bitcomet_check && i >= 9) {
			IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct, IPQ_LOG_TRACE, "Bitcomet LTS detected\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BITTORRENT);
			return 1;
		}

		/* answer to this pattern is not possible to implement asymmetrically */
		len = packet->payload_packet_len - 4;
		ptr = &packet->payload[4];
		while (1) {
			if (len < 50 || ptr[0] == 0x0d) {
				goto ipq_end_bt_tracker_check;
			}
			if (memcmp(ptr, "info_hash=", 10) == 0) {
				break;
			}
			len--;
			ptr++;
		}

		IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
						   IPQ_LOG_TRACE, " BT stat: tracker info hash found\n");

		/* len is > 50, so save operation here */
		len -= 10;
		ptr += 10;

		/* parse bt hash */
		for (a = 0; a < 20; a++) {
			if (len < 3) {
				goto ipq_end_bt_tracker_check;
			}
			if (*ptr == '%') {
				u8 x1 = 0xFF;
				u8 x2 = 0xFF;


				if (ptr[1] >= '0' && ptr[1] <= '9') {
					x1 = ptr[1] - '0';
				}
				if (ptr[1] >= 'a' && ptr[1] <= 'f') {
					x1 = 10 + ptr[1] - 'a';
				}
				if (ptr[1] >= 'A' && ptr[1] <= 'F') {
					x1 = 10 + ptr[1] - 'A';
				}

				if (ptr[2] >= '0' && ptr[2] <= '9') {
					x2 = ptr[2] - '0';
				}
				if (ptr[2] >= 'a' && ptr[2] <= 'f') {
					x2 = 10 + ptr[2] - 'a';
				}
				if (ptr[2] >= 'A' && ptr[2] <= 'F') {
					x2 = 10 + ptr[2] - 'A';
				}

				if (x1 == 0xFF || x2 == 0xFF) {
					goto ipq_end_bt_tracker_check;
				}
				ptr += 3;
				len -= 3;
			} else if (*ptr >= 32 && *ptr < 127) {
				ptr++;
				len--;
			} else {
				goto ipq_end_bt_tracker_check;
			}
		}

		IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
						   IPQ_LOG_TRACE, " BT stat: tracker info hash parsed\n");

		ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BITTORRENT);
		return 1;
	}

  ipq_end_bt_tracker_check:

	if (packet->payload_packet_len == 80) {
		/* Warez 80 Bytes Packet
		 * +----------------+---------------+-----------------+-----------------+
		 * |20 BytesPattern | 32 Bytes Value| 12 BytesPattern | 16 Bytes Data   |
		 * +----------------+---------------+-----------------+-----------------+
		 * 20 BytesPattern : 4c 00 00 00 ff ff ff ff 57 00 00 00 00 00 00 00 20 00 00 00
		 * 12 BytesPattern : 28 23 00 00 01 00 00 00 10 00 00 00
		 * */
		static const char pattern_20_bytes[20] = { 0x4c, 0x00, 0x00, 0x00, 0xff,
			0xff, 0xff, 0xff, 0x57, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00
		};
		static const char pattern_12_bytes[12] = { 0x28, 0x23, 0x00, 0x00, 0x01,
			0x00, 0x00, 0x00, 0x10, 0x00,
			0x00, 0x00
		};

/* did not see this pattern anywhere */
		if ((memcmp(&packet->payload[0], pattern_20_bytes, 20) == 0)
			&& (memcmp(&packet->payload[52], pattern_12_bytes, 12) == 0)) {
			IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
							   IPQ_LOG_TRACE, "BT: Warez - Plain BitTorrent protocol detected\n");
			ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BITTORRENT);
			return 1;
		}
	}

	return 0;
}


/*Search for BitTorrent commands*/
static void ipoque_int_search_bittorrent_tcp(struct ipoque_detection_module_struct
											 *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	if (packet->payload_packet_len == 0) {
		return;
	}

	if (flow->bittorrent_stage == 0 && packet->payload_packet_len != 0) {
		/* exclude stage 0 detection from next run */
		flow->bittorrent_stage = 1;
		if (ipoque_int_search_bittorrent_tcp_zero(ipoque_struct) != 0) {
			IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct, IPQ_LOG_DEBUG,
							   "stage 0 has detected something, returning\n");
			return;
		}

		IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct, IPQ_LOG_DEBUG,
						   "stage 0 has no direct detection, fall through\n");
	}
	return;
}

static void find_bittorrent_metadata(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	const u8 *data = packet->payload;
	int len = packet->payload_packet_len;
	const u8 *end = packet->payload + len;
	const u8 *next_token;
	int n_found = 0;
	int n_open = 0;
	unsigned long ofs;
	int l;

	if (len < 20)
		return;

	/* first three bytes should be null */
	if (data[0] || data[1] || data[2])
		return;

	data += 6;
	next_token = data;
	while (data < end) {
		switch (*data) {
		case 'd':
			/* dictionary */
			data++;
			n_open++;
			continue;

		case 'e':
			/* end of item */
			data++;
			if (--n_open < 0)
				goto out;

			continue;

		case 'i':
			l = 0;
			data++;
			next_token = data;
			while (next_token < end && *next_token >= '0' && *next_token <= '9') {
				next_token++;
				l++;
			}

			if (next_token == end || !l || *next_token != 'e')
				goto out;

			data = next_token + 1;

			n_found++;
			continue;

		default:
			l = 0;
			next_token = data;
			while (next_token < end && *next_token >= '0' && *next_token <= '9') {
				next_token++;
				l++;
				if (next_token - data > 6)
					return;
			}
			if (next_token == end || !l || *next_token != ':')
				goto out;

			ofs = strtoul((const char *) data, NULL, 10);
			if (ofs > end - next_token)
				goto out;

			data = next_token + ofs + 1;
			n_found++;
		}
	}
out:
	if (n_found >= 4)
		ipq_connection_detected(ipoque_struct, IPOQUE_PROTOCOL_BITTORRENT);
}

static void ipoque_search_bittorrent(struct ipoque_detection_module_struct
							  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	if (packet->detected_protocol != IPOQUE_PROTOCOL_BITTORRENT) {
		/* check for tcp retransmission here */

		if ((packet->tcp != NULL)
			&& (packet->tcp_retransmission == 0 || packet->num_retried_bytes)) {
			ipoque_int_search_bittorrent_tcp(ipoque_struct);
		}
	}
	find_bittorrent_metadata(ipoque_struct);
}
#endif
