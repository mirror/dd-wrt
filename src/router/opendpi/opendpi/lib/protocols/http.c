/*
 * http.c
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


#include "ipq_protocols.h"

#ifdef IPOQUE_PROTOCOL_HTTP

static void ipoque_int_http_add_connection(struct ipoque_detection_module_struct
										   *ipoque_struct, u32 protocol)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	ipq_connection_detected(ipoque_struct, protocol);
	flow->http_detected = 1;
	flow->server_direction = packet->packet_direction;
}

#ifdef IPOQUE_PROTOCOL_QQ
static void qq_parse_packet_URL_and_hostname(struct ipoque_detection_module_struct
											 *ipoque_struct, struct ipoque_parse_data *pd)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	const u8 *p, *end, *line;
	int len;

	if (packet->payload_packet_len < 100 ||
		/*memcmp(&packet->payload[4], "/qzone", 6) != 0 || packet->host_line.len < 7 || */
		memcmp(&pd->host_line.ptr[pd->host_line.len - 6], "qq.com", 6) != 0) {

		IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "did not find QQ.\n");
		return;
	}
	for (p = packet->payload, end = p + packet->payload_packet_len;
	     get_next_line(&p, end, &line, &len);) {
		if ((len > 22 && memcmp(line, "QzoneAuth: zzpaneluin=", 22) == 0) ||
			(len > 19 && memcmp(line, "Cookie: zzpanelkey=", 19) == 0) ||
			(len > 13 && memcmp(line, "Cookie: adid=", 13) == 0)) {
			IPQ_LOG(IPOQUE_PROTOCOL_QQ, ipoque_struct, IPQ_LOG_DEBUG, "found QQ.\n");
			ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_QQ);
			return;
		}
	}

}
#endif


#ifdef IPOQUE_PROTOCOL_MPEG
static void mpeg_parse_packet_contentline(struct ipoque_detection_module_struct
										  *ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->content_line.len >= 10 && memcmp(pd->content_line.ptr, "audio/mpeg", 10) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MPEG, ipoque_struct, IPQ_LOG_DEBUG, "MPEG: Content-Type: audio/mpeg found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MPEG);
		return;
	}
	if (pd->content_line.len >= 12 && memcmp(pd->content_line.ptr, "audio/x-mpeg", 12) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MPEG, ipoque_struct, IPQ_LOG_DEBUG, "MPEG: Content-Type: audio/x-mpeg found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MPEG);
		return;
	}
	if (pd->content_line.len >= 11 && memcmp(pd->content_line.ptr, "audio/mpeg3", 11) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MPEG, ipoque_struct, IPQ_LOG_DEBUG, "MPEG: Content-Type: audio/mpeg3 found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MPEG);
		return;
	}
	if (pd->content_line.len >= 11 && memcmp(pd->content_line.ptr, "audio/mp4a", 10) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MPEG, ipoque_struct, IPQ_LOG_DEBUG, "MPEG: Content-Type: audio/mp4a found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MPEG);
		return;
	}
	if (pd->content_line.len >= 10 && memcmp(pd->content_line.ptr, "video/mpeg", 10) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MPEG, ipoque_struct, IPQ_LOG_DEBUG, "MPEG: Content-Type: video/mpeg found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MPEG);
		return;
	}
	if (pd->content_line.len >= 9 && memcmp(pd->content_line.ptr, "video/nsv", 9) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MPEG, ipoque_struct, IPQ_LOG_DEBUG, "MPEG: content-type:video/nsv found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MPEG);
		return;
	}
	/* Ultravox */
	if (pd->content_line.len >= 13 && memcmp(pd->content_line.ptr, "misc/ultravox", 13) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MPEG, ipoque_struct, IPQ_LOG_DEBUG, "MPEG: Content-Type: misc/ultravox found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MPEG);
		return;
	}

}
#endif


#ifdef IPOQUE_PROTOCOL_OGG
static void ogg_parse_packet_contentline(struct ipoque_detection_module_struct
										 *ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->content_line.len >= 9 && memcmp(pd->content_line.ptr, "audio/ogg", 9) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_OGG, ipoque_struct, IPQ_LOG_DEBUG, "OGG: Content-Type: audio/ogg found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_OGG);
		return;
	}
	if (pd->content_line.len >= 9 && memcmp(pd->content_line.ptr, "video/ogg", 9) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_OGG, ipoque_struct, IPQ_LOG_DEBUG, "OGG: Content-Type: video/ogg found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_OGG);
		return;
	}
	if (pd->content_line.len >= 15 && memcmp(pd->content_line.ptr, "application/ogg", 15) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_OGG, ipoque_struct, IPQ_LOG_DEBUG, "OGG: content-type: application/ogg found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_OGG);
		return;
	}
}
#endif

#ifdef IPOQUE_PROTOCOL_FLASH
static void flash_parse_packet_contentline(struct ipoque_detection_module_struct
										   *ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->content_line.len >= 9 && memcmp(pd->content_line.ptr, "video/flv", 9) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "FLASH: Content-Type: video/flv found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FLASH);
		return;
	}
	if (pd->content_line.len >= 11 && memcmp(pd->content_line.ptr, "video/x-flv", 11) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "FLASH: Content-Type: video/x-flv found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FLASH);
		return;
	}
	if (pd->content_line.len >= 17 && memcmp(pd->content_line.ptr, "application/x-fcs", 17) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "FLASH: Content-Type: application/x-fcs found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FLASH);
		return;
	}
	if (pd->content_line.len >= 29 && memcmp(pd->content_line.ptr, "application/x-shockwave-flash", 29) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG,
				"FLASH: Content-Type: application/x-shockwave-flash found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FLASH);
		return;
	}
	if (pd->content_line.len >= 11 && memcmp(pd->content_line.ptr, "video/flash", 11) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "FLASH: Content-Type: video/flash found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FLASH);
		return;
	}
	if (pd->content_line.len >= 15 && memcmp(pd->content_line.ptr, "application/flv", 15) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "FLASH: Content-Type: application/flv found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FLASH);
		return;
	}
	if (pd->content_line.len >= 28 && memcmp(pd->content_line.ptr, "flv-application/octet-stream", 28) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG,
				"FLASH: Content-Type: flv-application/octet-stream.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FLASH);
		return;
	}
}
#endif

#ifdef IPOQUE_PROTOCOL_QUICKTIME
static void qt_parse_packet_contentline(struct ipoque_detection_module_struct
										*ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->content_line.len >= 15 && memcmp(pd->content_line.ptr, "video/quicktime", 15) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_QUICKTIME, ipoque_struct, IPQ_LOG_DEBUG,
				"QUICKTIME: Content-Type: video/quicktime found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_QUICKTIME);
		return;
	}
	if (pd->content_line.len >= 9 && memcmp(pd->content_line.ptr, "video/mp4", 9) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_QUICKTIME, ipoque_struct, IPQ_LOG_DEBUG, "QUICKTIME: Content-Type: video/mp4 found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_QUICKTIME);
		return;
	}
	if (pd->content_line.len >= 11 && memcmp(pd->content_line.ptr, "video/x-m4v", 11) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_QUICKTIME, ipoque_struct, IPQ_LOG_DEBUG,
				"QUICKTIME: Content-Type: video/x-m4v found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_QUICKTIME);
		return;
	}
}
#endif

#ifdef IPOQUE_PROTOCOL_REALMEDIA
static void realmedia_parse_packet_contentline(struct ipoque_detection_module_struct
											   *ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->content_line.len >= 20 && memcmp(pd->content_line.ptr, "audio/x-pn-realaudio", 20) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_REALMEDIA, ipoque_struct, IPQ_LOG_DEBUG,
				"REALMEDIA: Content-Type: audio/x-pn-realaudio found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_REALMEDIA);
		return;
	}
	if (pd->content_line.len >= 28 && memcmp(pd->content_line.ptr, "application/vnd.rn-realmedia", 28) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_REALMEDIA, ipoque_struct, IPQ_LOG_DEBUG,
				"REALMEDIA: Content-Type: application/vnd.rn-realmedia found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_REALMEDIA);
		return;
	}
}
#endif

#ifdef IPOQUE_PROTOCOL_WINDOWSMEDIA
static void windowsmedia_parse_packet_contentline(struct ipoque_detection_module_struct
												  *ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->content_line.len >= 14 && ipq_mem_cmp(pd->content_line.ptr, "video/x-ms-", 11) == 0) {
		if (ipq_mem_cmp(&pd->content_line.ptr[11], "wmv", 3) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_WINDOWSMEDIA, ipoque_struct, IPQ_LOG_DEBUG,
					"WINDOWSMEDIA: Content-Type: video/x-ms-wmv found.\n");
			ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_WINDOWSMEDIA);
			return;
		}
		if (ipq_mem_cmp(&pd->content_line.ptr[11], "asf", 3) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_WINDOWSMEDIA, ipoque_struct, IPQ_LOG_DEBUG,
					"WINDOWSMEDIA: Content-Type: video/x-ms-asf found.\n");
			ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_WINDOWSMEDIA);
			return;
		}
		if (ipq_mem_cmp(&pd->content_line.ptr[11], "asx", 3) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_WINDOWSMEDIA, ipoque_struct, IPQ_LOG_DEBUG,
					"WINDOWSMEDIA: Content-Type: video/x-ms-asx found.\n");
			ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_WINDOWSMEDIA);
			return;
		}
	}
	if (pd->content_line.len >= 24 && ipq_mem_cmp(pd->content_line.ptr, "video/x-msvideo", 15) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_WINDOWSMEDIA, ipoque_struct, IPQ_LOG_DEBUG,
				"WINDOWSMEDIA: Content-Type: video/x-msvideo found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_WINDOWSMEDIA);
		return;
	}
	if (pd->content_line.len >= 24 && ipq_mem_cmp(pd->content_line.ptr, "audio/x-wav", 11) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_WINDOWSMEDIA, ipoque_struct, IPQ_LOG_DEBUG,
				"WINDOWSMEDIA: Content-Type: audio/x-wav found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_WINDOWSMEDIA);
		return;
	}
	if (pd->content_line.len >= 32
		&& ipq_mem_cmp(pd->content_line.ptr, "application/vnd.ms.wms-hdr.asfv1", 32) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_WINDOWSMEDIA, ipoque_struct, IPQ_LOG_DEBUG,
				"WINDOWSMEDIA: Content-Type: application/vnd.ms.wms-hdr.asfv1 found.\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_WINDOWSMEDIA);
		return;
	}
}

static void winmedia_parse_packet_useragentline(struct ipoque_detection_module_struct
												*ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->user_agent_line.len >= 9
		&& memcmp(pd->user_agent_line.ptr, "NSPlayer/", 9) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_WINDOWSMEDIA, ipoque_struct, IPQ_LOG_DEBUG, "username NSPlayer found\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_WINDOWSMEDIA);
	}
}
#endif

#ifdef IPOQUE_PROTOCOL_MMS
static void mms_parse_packet_contentline(struct ipoque_detection_module_struct
										 *ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->content_line.len >= 24 && ipq_mem_cmp(pd->content_line.ptr, "application/x-mms-framed", 24) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_MMS, ipoque_struct, IPQ_LOG_DEBUG,
				"MMS: Content-Type: application/x-mms-framed found\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MMS);
	}
}
#endif


#ifdef IPOQUE_PROTOCOL_XBOX
static void xbox_parse_packet_useragentline(struct ipoque_detection_module_struct
											*ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->user_agent_line.len >= 17 && memcmp(pd->user_agent_line.ptr, "Xbox Live Client/", 17) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_XBOX, ipoque_struct, IPQ_LOG_DEBUG, "XBOX: User Agent: Xbox Live Client found\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_XBOX);
	}
}
#endif


#ifdef IPOQUE_PROTOCOL_FLASH
static void flash_check_http_payload(struct ipoque_detection_module_struct
									 *ipoque_struct, struct ipoque_parse_data *pd)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	const u8 *pos;

	if (pd->empty_line_position_set == 0 || (pd->empty_line_position + 10) > (packet->payload_packet_len))
		return;

	pos = &packet->payload[pd->empty_line_position] + 2;


	if (memcmp(pos, "FLV", 3) == 0 && pos[3] == 0x01 && (pos[4] == 0x01 || pos[4] == 0x04 || pos[4] == 0x05)
		&& pos[5] == 0x00 && pos[6] == 0x00 && pos[7] == 0x00 && pos[8] == 0x09) {

		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "Flash content in http detected\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_FLASH);
	}
}
#endif

#ifdef IPOQUE_PROTOCOL_AVI
static void avi_check_http_payload(struct ipoque_detection_module_struct *ipoque_struct, struct ipoque_parse_data *pd)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	IPQ_LOG(IPOQUE_PROTOCOL_AVI, ipoque_struct, IPQ_LOG_DEBUG, "called avi_check_http_payload: %u %u %u\n",
			pd->empty_line_position_set, flow->http_empty_line_seen, pd->empty_line_position);

	if (pd->empty_line_position_set == 0 && flow->http_empty_line_seen == 0)
		return;

	if (pd->empty_line_position_set != 0 && ((pd->empty_line_position + 20) > (packet->payload_packet_len))
		&& flow->http_empty_line_seen == 0) {
		flow->http_empty_line_seen = 1;
		return;
	}

	if (flow->http_empty_line_seen == 1) {
		if (packet->payload_packet_len > 20 && memcmp(packet->payload, "RIFF", 4) == 0
			&& memcmp(packet->payload + 8, "AVI LIST", 8) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_AVI, ipoque_struct, IPQ_LOG_DEBUG, "Avi content in http detected\n");
			ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_AVI);
		}
		flow->http_empty_line_seen = 0;
		return;
	}

	if (pd->empty_line_position_set != 0) {
		// check for avi header
		// for reference see http://msdn.microsoft.com/archive/default.asp?url=/archive/en-us/directx9_c/directx/htm/avirifffilereference.asp
		u32 p = pd->empty_line_position + 2;

		IPQ_LOG(IPOQUE_PROTOCOL_AVI, ipoque_struct, IPQ_LOG_DEBUG, "p = %u\n", p);

		if ((p + 16) <= packet->payload_packet_len && memcmp(&packet->payload[p], "RIFF", 4) == 0
			&& memcmp(&packet->payload[p + 8], "AVI LIST", 8) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_AVI, ipoque_struct, IPQ_LOG_DEBUG, "Avi content in http detected\n");
			ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_AVI);
		}
	}
}
#endif

#ifdef IPOQUE_PROTOCOL_OFF
static void off_parse_packet_contentline(struct ipoque_detection_module_struct
										 *ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->content_line.len >= 4 && memcmp(pd->content_line.ptr, "off/", 4) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_OFF, ipoque_struct, IPQ_LOG_DEBUG, "off: Content-Type: off/ found\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_OFF);
	}
}
#endif

#ifdef IPOQUE_PROTOCOL_MOVE
static void move_parse_packet_contentline(struct ipoque_detection_module_struct
										  *ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->content_line.len == 15
		&& (memcmp(pd->content_line.ptr, "application/qmx", 15) == 0
			|| memcmp(pd->content_line.ptr, "application/qss", 15) == 0)) {
		IPQ_LOG(IPOQUE_PROTOCOL_MOVE, ipoque_struct, IPQ_LOG_DEBUG, "MOVE application qmx or qss detected\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MOVE);
	}
}
#endif

#ifdef IPOQUE_PROTOCOL_RTSP
static void rtsp_parse_packet_acceptline(struct ipoque_detection_module_struct
										 *ipoque_struct, struct ipoque_parse_data *pd)
{
	if (pd->accept_line.len >= 28 && memcmp(pd->accept_line.ptr, "application/x-rtsp-tunnelled", 28) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_RTSP, ipoque_struct, IPQ_LOG_DEBUG, "RTSP accept line detected\n");
		ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_RTSP);
	}
}
#endif

static void http_check_content_type_and_change_protocol(struct ipoque_detection_module_struct
												   *ipoque_struct, struct ipoque_parse_data *pd)
{
#ifdef IPOQUE_PROTOCOL_MPEG
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	const u8 *p, *end, *line;
	int len;
#endif

	if (pd->content_line.ptr != NULL && pd->content_line.len != 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "Content Type Line found %.*s\n",
				pd->content_line.len, pd->content_line.ptr);
#ifdef IPOQUE_PROTOCOL_MPEG
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_MPEG) != 0)
			mpeg_parse_packet_contentline(ipoque_struct, pd);
#endif
#ifdef IPOQUE_PROTOCOL_FLASH
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_FLASH) != 0)
			flash_parse_packet_contentline(ipoque_struct, pd);
#endif
#ifdef IPOQUE_PROTOCOL_QUICKTIME
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_QUICKTIME) != 0)
			qt_parse_packet_contentline(ipoque_struct, pd);
#endif
#ifdef IPOQUE_PROTOCOL_REALMEDIA
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_REALMEDIA) != 0)
			realmedia_parse_packet_contentline(ipoque_struct, pd);
#endif
#ifdef IPOQUE_PROTOCOL_WINDOWSMEDIA
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_WINDOWSMEDIA) != 0)
			windowsmedia_parse_packet_contentline(ipoque_struct, pd);
#endif
#ifdef IPOQUE_PROTOCOL_MMS
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_MMS) != 0)
			mms_parse_packet_contentline(ipoque_struct, pd);
#endif
#ifdef IPOQUE_PROTOCOL_OFF
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_OFF) != 0)
			off_parse_packet_contentline(ipoque_struct, pd);
#endif
#ifdef IPOQUE_PROTOCOL_OGG
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_OGG) != 0)
			ogg_parse_packet_contentline(ipoque_struct, pd);
#endif
#ifdef IPOQUE_PROTOCOL_MOVE
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_MOVE) != 0)
			move_parse_packet_contentline(ipoque_struct, pd);
#endif
	}
	/* check user agent here too */
	if (pd->user_agent_line.ptr != NULL && pd->user_agent_line.len != 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "User Agent Type Line found %.*s\n",
				pd->user_agent_line.len, pd->user_agent_line.ptr);
#ifdef IPOQUE_PROTOCOL_XBOX
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_XBOX) != 0)
			xbox_parse_packet_useragentline(ipoque_struct, pd);
#endif
#ifdef IPOQUE_PROTOCOL_WINDOWSMEDIA
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_WINDOWSMEDIA) != 0)
			winmedia_parse_packet_useragentline(ipoque_struct, pd);
#endif

	}
	/* check for host line */
	if (pd->host_line.ptr != NULL) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HOST Line found %.*s\n",
				pd->host_line.len, pd->host_line.ptr);
#ifdef IPOQUE_PROTOCOL_QQ
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_QQ) != 0) {
			qq_parse_packet_URL_and_hostname(ipoque_struct, pd);
		}
#endif
	}
	/* check for accept line */
	if (pd->accept_line.ptr != NULL) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "Accept Line found %.*s\n",
				pd->accept_line.len, pd->accept_line.ptr);
#ifdef IPOQUE_PROTOCOL_RTSP
		if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_RTSP) != 0) {
			rtsp_parse_packet_acceptline(ipoque_struct, pd);
		}
#endif
	}
	/* search for line startin with "Icy-MetaData" */
#ifdef IPOQUE_PROTOCOL_MPEG
	for (p = packet->payload, end = p + packet->payload_packet_len;
	     get_next_line(&p, end, &line, &len);) {
		if (len > 11 && memcmp(line, "Icy-MetaData", 12) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_MPEG, ipoque_struct, IPQ_LOG_DEBUG, "MPEG: Icy-MetaData found.\n");
			ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_MPEG);
			return;
		}
	}
#ifdef IPOQUE_PROTOCOL_AVI
#endif
#endif

}

static void check_http_payload(struct ipoque_detection_module_struct *ipoque_struct, struct ipoque_parse_data *pd)
{
	IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "called check_http_payload.\n");

#ifdef IPOQUE_PROTOCOL_FLASH
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_FLASH) != 0)
		flash_check_http_payload(ipoque_struct, pd);
#endif
#ifdef IPOQUE_PROTOCOL_AVI
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask, IPOQUE_PROTOCOL_AVI) != 0)
		avi_check_http_payload(ipoque_struct, pd);
#endif
}

static u16 http_request_url_offset(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;

	if (packet->payload_packet_len < 10)
		return 0;

	/* FIRST PAYLOAD PACKET FROM CLIENT */
	/* check if the packet starts with POST or GET */
	if (memcmp(packet->payload, "GET ", 4) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP: GET FOUND\n");
		return 4;
	} else if (memcmp(packet->payload, "POST ", 5) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP: POST FOUND\n");
		return 5;
	} else if (memcmp(packet->payload, "OPTIONS ", 8) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP: OPTIONS FOUND\n");
		return 8;
	} else if (memcmp(packet->payload, "HEAD ", 5) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP: HEAD FOUND\n");
		return 5;
	} else if (memcmp(packet->payload, "PUT ", 4) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP: PUT FOUND\n");
		return 4;
	} else if (memcmp(packet->payload, "DELETE ", 7) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP: DELETE FOUND\n");
		return 7;
	} else if (memcmp(packet->payload, "CONNECT ", 8) == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP: CONNECT FOUND\n");
		return 8;
	}
	return 0;
}

static void http_bitmask_exclude(struct ipoque_flow_struct *flow)
{
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_HTTP);
#ifdef IPOQUE_PROTOCOL_MPEG
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MPEG);
#endif
#ifdef IPOQUE_PROTOCOL_QUICKTIME
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_QUICKTIME);
#endif
#ifdef IPOQUE_PROTOCOL_WINDOWSMEDIA
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_WINDOWSMEDIA);
#endif
#ifdef IPOQUE_PROTOCOL_REALMEDIA
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_REALMEDIA);
#endif
#ifdef IPOQUE_PROTOCOL_AVI
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_AVI);
#endif
#ifdef IPOQUE_PROTOCOL_OGG
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_OGG);
#endif
#ifdef IPOQUE_PROTOCOL_MOVE
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_MOVE);
#endif
#ifdef IPOQUE_PROTOCOL_OFF
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_OFF);
#endif
#ifdef IPOQUE_PROTOCOL_XBOX
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_XBOX);
#endif
}

static void ipoque_search_http_tcp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_parse_data pd;
	const u8 *p, *end, *line;
	int len;
	u16 filename_start;

	IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "search http\n");

	memset(&pd, 0, sizeof(pd));

	/* set client-server_direction */
	if (flow->http_setup_dir == 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "initializes http to stage: 1 \n");
		flow->http_setup_dir = 1 + packet->packet_direction;
	}

	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->generic_http_packet_bitmask, packet->detected_protocol) != 0) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG,
				"protocol might be detected earlier as http jump to payload type detection\n");
		goto http_parse_detection;
	}

	if (flow->http_setup_dir == 1 + packet->packet_direction) {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "http stage: 1\n");
		if (flow->http_stage == 0) {
			filename_start = http_request_url_offset(ipoque_struct);
			if (filename_start == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "filename not found, exclude\n");
				http_bitmask_exclude(flow);
				return;
			}
			// parse packet
			p = packet->payload;
			end = p + packet->payload_packet_len;
			if (!get_next_line(&p, end, &line, &len)) {
				/* parse one more packet .. */
				IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "just one line, search next packet\n");
				flow->http_stage = 1;
				return;
			}
			// parsed_lines > 1 here
			if (len >= (9 + filename_start)
				&& memcmp(&line[len - 9], " HTTP/1.", 8) == 0) {
				ipq_parse_packet_line_info(ipoque_struct, &pd);

				pd.http_url_name.ptr = &packet->payload[filename_start];
				pd.http_url_name.len = len - (filename_start + 9);

				IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "http structure detected, adding\n");

				ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_HTTP);
				http_check_content_type_and_change_protocol(ipoque_struct, &pd);
				/* HTTP found, look for host... */
				if (pd.host_line.ptr != NULL) {
					/* aaahh, skip this direction and wait for a server reply here */
					flow->http_stage = 2;
					IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP START HOST found\n");
					return;
				}
				IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP START HOST found\n");

				/* host not found, check in next packet after */
				flow->http_stage = 1;
				return;
			}
		} else if (flow->http_stage == 1) {
			/* SECOND PAYLOAD TRAFFIC FROM CLIENT, FIRST PACKET MIGHT HAVE BEEN HTTP... */
			/* UNKNOWN TRAFFIC, HERE FOR HTTP again.. */
			// parse packet
			p = packet->payload;
			end = p + packet->payload_packet_len;
			if (!get_next_line(&p, end, &line, &len)) {
				/* stop parsing here */
				IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG,
						"HTTP: PACKET DOES NOT HAVE A LINE STRUCTURE\n");
				http_bitmask_exclude(flow);
				return;
			}

			if (len >= 9 && memcmp(&line[len - 9], " HTTP/1.", 8) == 0) {
				ipq_parse_packet_line_info(ipoque_struct, &pd);
				ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_HTTP);
				http_check_content_type_and_change_protocol(ipoque_struct, &pd);
				IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG,
						"HTTP START HTTP found in 2. packet, check host here...\n");
				/* HTTP found, look for host... */
				flow->http_stage = 2;

				return;
			}
		}
	}
	IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP: REQUEST NOT HTTP CONFORM\n");
	http_bitmask_exclude(flow);
	return;

  http_parse_detection:
	if (flow->http_setup_dir == 1 + packet->packet_direction) {
		/* we have something like http here, so check for host and content type if possible */
		if (flow->http_stage == 0 || flow->http_stage == 3) {
			IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP RUN MAYBE NEXT GET/POST...\n");
			// parse packet
			ipq_parse_packet_line_info(ipoque_struct, &pd);
			/* check for url here */
			filename_start = http_request_url_offset(ipoque_struct);
			p = packet->payload;
			end = p + packet->payload_packet_len;
			if (filename_start != 0 && get_next_line(&p, end, &line, &len) &&
			    len >= (9 + filename_start) && memcmp(line, " HTTP/1.", 8) == 0) {
				pd.http_url_name.ptr = &packet->payload[filename_start];
				pd.http_url_name.len = len - (filename_start + 9);
				IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "next http action, "
						"resetting to http and search for other protocols later.\n");
				ipoque_int_http_add_connection(ipoque_struct, IPOQUE_PROTOCOL_HTTP);
			}
			http_check_content_type_and_change_protocol(ipoque_struct, &pd);
			/* HTTP found, look for host... */
			if (pd.host_line.ptr != NULL) {
				IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG,
						"HTTP RUN MAYBE NEXT HOST found, skipping all packets from this direction\n");
				/* aaahh, skip this direction and wait for a server reply here */
				flow->http_stage = 2;
				return;
			}
			IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG,
					"HTTP RUN MAYBE NEXT HOST NOT found, scanning one more packet from this direction\n");
			flow->http_stage = 1;
		} else if (flow->http_stage == 1) {
			// parse packet and maybe find a packet info with host ptr,...
			ipq_parse_packet_line_info(ipoque_struct, &pd);
			http_check_content_type_and_change_protocol(ipoque_struct, &pd);
			IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP RUN second packet scanned\n");
			/* HTTP found, look for host... */
			flow->http_stage = 2;
		}
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG,
				"HTTP skipping client packets after second packet\n");
		return;
	}
	/* server response */
	if (flow->http_stage > 0) {
		/* first packet from server direction, might have a content line */
		ipq_parse_packet_line_info(ipoque_struct, &pd);
		http_check_content_type_and_change_protocol(ipoque_struct, &pd);


		if (pd.empty_line_position_set != 0 || flow->http_empty_line_seen == 1) {
			IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "empty line. check_http_payload.\n");
			check_http_payload(ipoque_struct, &pd);
		}
		if (flow->http_stage == 2) {
			flow->http_stage = 3;
		} else {
			flow->http_stage = 0;
		}
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG,
				"HTTP response first or second packet scanned,new stage is: %u\n", flow->http_stage);
		return;
	} else {
		IPQ_LOG(IPOQUE_PROTOCOL_HTTP, ipoque_struct, IPQ_LOG_DEBUG, "HTTP response next packet skipped\n");
	}
}
#endif
