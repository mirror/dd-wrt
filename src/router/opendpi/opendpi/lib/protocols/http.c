/*
 * http.c
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


#include "ndpi_protocols.h"

#ifdef NDPI_PROTOCOL_HTTP

static void ndpi_int_http_add_connection(struct ndpi_detection_module_struct *ndpi_struct, 
					 struct ndpi_flow_struct *flow,
					 u_int32_t protocol)
{
  

  if (protocol != NDPI_PROTOCOL_HTTP) {
    ndpi_int_add_connection(ndpi_struct, flow, protocol, NDPI_CORRELATED_PROTOCOL);
  } else {
    ndpi_int_reset_protocol(flow);
    ndpi_int_add_connection(ndpi_struct, flow, protocol, NDPI_REAL_PROTOCOL);
  }
  flow->http_detected = 1;
}

#ifdef NDPI_PROTOCOL_QQ
static void qq_parse_packet_URL_and_hostname(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  u_int32_t a;

  if (packet->payload_packet_len < 100 ||
      /*memcmp(&packet->payload[4], "/qzone", 6) != 0 || packet->host_line.len < 7 || */
      memcmp(&packet->host_line.ptr[packet->host_line.len - 6], "qq.com", 6) != 0) {

    NDPI_LOG(NDPI_PROTOCOL_QQ, ndpi_struct, NDPI_LOG_DEBUG, "did not find QQ.\n");
    return;
  }
  for (a = 0; a < packet->parsed_lines; a++) {
    if ((packet->line[a].len > 22 && memcmp(packet->line[a].ptr, "QzoneAuth: zzpaneluin=", 22) == 0) ||
	(packet->line[a].len > 19 && memcmp(packet->line[a].ptr, "Cookie: zzpanelkey=", 19) == 0) ||
	(packet->line[a].len > 13 && memcmp(packet->line[a].ptr, "Cookie: adid=", 13) == 0)) {
      NDPI_LOG(NDPI_PROTOCOL_QQ, ndpi_struct, NDPI_LOG_DEBUG, "found QQ.\n");
      ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_QQ);
      return;
    }
  }

}
#endif


#ifdef NDPI_PROTOCOL_MPEG
static void mpeg_parse_packet_contentline(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->content_line.len >= 10 && memcmp(packet->content_line.ptr, "audio/mpeg", 10) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_MPEG, ndpi_struct, NDPI_LOG_DEBUG, "MPEG: Content-Type: audio/mpeg found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_MPEG);
    return;
  }
  if (packet->content_line.len >= 12 && memcmp(packet->content_line.ptr, "audio/x-mpeg", 12) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_MPEG, ndpi_struct, NDPI_LOG_DEBUG, "MPEG: Content-Type: audio/x-mpeg found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_MPEG);
    return;
  }
  if (packet->content_line.len >= 11 && memcmp(packet->content_line.ptr, "audio/mpeg3", 11) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_MPEG, ndpi_struct, NDPI_LOG_DEBUG, "MPEG: Content-Type: audio/mpeg3 found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_MPEG);
    return;
  }
  if (packet->content_line.len >= 11 && memcmp(packet->content_line.ptr, "audio/mp4a", 10) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_MPEG, ndpi_struct, NDPI_LOG_DEBUG, "MPEG: Content-Type: audio/mp4a found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_MPEG);
    return;
  }
  if (packet->content_line.len >= 10 && memcmp(packet->content_line.ptr, "video/mpeg", 10) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_MPEG, ndpi_struct, NDPI_LOG_DEBUG, "MPEG: Content-Type: video/mpeg found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_MPEG);
    return;
  }
  if (packet->content_line.len >= 9 && memcmp(packet->content_line.ptr, "video/nsv", 9) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_MPEG, ndpi_struct, NDPI_LOG_DEBUG, "MPEG: content-type:video/nsv found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_MPEG);
    return;
  }
  /* Ultravox */
  if (packet->content_line.len >= 13 && memcmp(packet->content_line.ptr, "misc/ultravox", 13) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_MPEG, ndpi_struct, NDPI_LOG_DEBUG, "MPEG: Content-Type: misc/ultravox found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_MPEG);
    return;
  }

}
#endif


#ifdef NDPI_PROTOCOL_OGG
static void ogg_parse_packet_contentline(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->content_line.len >= 9 && memcmp(packet->content_line.ptr, "audio/ogg", 9) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_OGG, ndpi_struct, NDPI_LOG_DEBUG, "OGG: Content-Type: audio/ogg found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_OGG);
    return;
  }
  if (packet->content_line.len >= 9 && memcmp(packet->content_line.ptr, "video/ogg", 9) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_OGG, ndpi_struct, NDPI_LOG_DEBUG, "OGG: Content-Type: video/ogg found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_OGG);
    return;
  }
  if (packet->content_line.len >= 15 && memcmp(packet->content_line.ptr, "application/ogg", 15) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_OGG, ndpi_struct, NDPI_LOG_DEBUG, "OGG: content-type: application/ogg found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_OGG);
    return;
  }
}
#endif

#ifdef NDPI_PROTOCOL_FLASH
static void flash_parse_packet_contentline(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->content_line.len >= 9 && memcmp(packet->content_line.ptr, "video/flv", 9) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG, "FLASH: Content-Type: video/flv found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_FLASH);
    return;
  }
  if (packet->content_line.len >= 11 && memcmp(packet->content_line.ptr, "video/x-flv", 11) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG, "FLASH: Content-Type: video/x-flv found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_FLASH);
    return;
  }
  if (packet->content_line.len >= 17 && memcmp(packet->content_line.ptr, "application/x-fcs", 17) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG, "FLASH: Content-Type: application/x-fcs found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_FLASH);
    return;
  }
  if (packet->content_line.len >= 29 && memcmp(packet->content_line.ptr, "application/x-shockwave-flash", 29) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG,
	    "FLASH: Content-Type: application/x-shockwave-flash found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_FLASH);
    return;
  }
  if (packet->content_line.len >= 11 && memcmp(packet->content_line.ptr, "video/flash", 11) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG, "FLASH: Content-Type: video/flash found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_FLASH);
    return;
  }
  if (packet->content_line.len >= 15 && memcmp(packet->content_line.ptr, "application/flv", 15) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG, "FLASH: Content-Type: application/flv found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_FLASH);
    return;
  }
  if (packet->content_line.len >= 28 && memcmp(packet->content_line.ptr, "flv-application/octet-stream", 28) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG,
	    "FLASH: Content-Type: flv-application/octet-stream.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_FLASH);
    return;
  }
}
#endif

#ifdef NDPI_PROTOCOL_QUICKTIME
static void qt_parse_packet_contentline(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->content_line.len >= 15 && memcmp(packet->content_line.ptr, "video/quicktime", 15) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_QUICKTIME, ndpi_struct, NDPI_LOG_DEBUG,
	    "QUICKTIME: Content-Type: video/quicktime found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_QUICKTIME);
    return;
  }
  if (packet->content_line.len >= 9 && memcmp(packet->content_line.ptr, "video/mp4", 9) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_QUICKTIME, ndpi_struct, NDPI_LOG_DEBUG, "QUICKTIME: Content-Type: video/mp4 found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_QUICKTIME);
    return;
  }
  if (packet->content_line.len >= 11 && memcmp(packet->content_line.ptr, "video/x-m4v", 11) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_QUICKTIME, ndpi_struct, NDPI_LOG_DEBUG,
	    "QUICKTIME: Content-Type: video/x-m4v found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_QUICKTIME);
    return;
  }
}
#endif

#ifdef NDPI_PROTOCOL_REALMEDIA
static void realmedia_parse_packet_contentline(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->content_line.len >= 20 && memcmp(packet->content_line.ptr, "audio/x-pn-realaudio", 20) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_REALMEDIA, ndpi_struct, NDPI_LOG_DEBUG,
	    "REALMEDIA: Content-Type: audio/x-pn-realaudio found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_REALMEDIA);
    return;
  }
  if (packet->content_line.len >= 28 && memcmp(packet->content_line.ptr, "application/vnd.rn-realmedia", 28) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_REALMEDIA, ndpi_struct, NDPI_LOG_DEBUG,
	    "REALMEDIA: Content-Type: application/vnd.rn-realmedia found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_REALMEDIA);
    return;
  }
}
#endif

#ifdef NDPI_PROTOCOL_WINDOWSMEDIA
static void windowsmedia_parse_packet_contentline(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->content_line.len >= 14 && ndpi_mem_cmp(packet->content_line.ptr, "video/x-ms-", 11) == 0) {
    if (ndpi_mem_cmp(&packet->content_line.ptr[11], "wmv", 3) == 0) {
      NDPI_LOG(NDPI_PROTOCOL_WINDOWSMEDIA, ndpi_struct, NDPI_LOG_DEBUG,
	      "WINDOWSMEDIA: Content-Type: video/x-ms-wmv found.\n");
      ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_WINDOWSMEDIA);
      return;
    }
    if (ndpi_mem_cmp(&packet->content_line.ptr[11], "asf", 3) == 0) {
      NDPI_LOG(NDPI_PROTOCOL_WINDOWSMEDIA, ndpi_struct, NDPI_LOG_DEBUG,
	      "WINDOWSMEDIA: Content-Type: video/x-ms-asf found.\n");
      ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_WINDOWSMEDIA);
      return;
    }
    if (ndpi_mem_cmp(&packet->content_line.ptr[11], "asx", 3) == 0) {
      NDPI_LOG(NDPI_PROTOCOL_WINDOWSMEDIA, ndpi_struct, NDPI_LOG_DEBUG,
	      "WINDOWSMEDIA: Content-Type: video/x-ms-asx found.\n");
      ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_WINDOWSMEDIA);
      return;
    }
  }
  if (packet->content_line.len >= 24 && ndpi_mem_cmp(packet->content_line.ptr, "video/x-msvideo", 15) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_WINDOWSMEDIA, ndpi_struct, NDPI_LOG_DEBUG,
	    "WINDOWSMEDIA: Content-Type: video/x-msvideo found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_WINDOWSMEDIA);
    return;
  }
  if (packet->content_line.len >= 24 && ndpi_mem_cmp(packet->content_line.ptr, "audio/x-wav", 11) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_WINDOWSMEDIA, ndpi_struct, NDPI_LOG_DEBUG,
	    "WINDOWSMEDIA: Content-Type: audio/x-wav found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_WINDOWSMEDIA);
    return;
  }
  if (packet->content_line.len >= 32
      && ndpi_mem_cmp(packet->content_line.ptr, "application/vnd.ms.wms-hdr.asfv1", 32) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_WINDOWSMEDIA, ndpi_struct, NDPI_LOG_DEBUG,
	    "WINDOWSMEDIA: Content-Type: application/vnd.ms.wms-hdr.asfv1 found.\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_WINDOWSMEDIA);
    return;
  }
}

static void winmedia_parse_packet_useragentline(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->user_agent_line.len >= 9
      && memcmp(packet->user_agent_line.ptr, "NSPlayer/", 9) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_WINDOWSMEDIA, ndpi_struct, NDPI_LOG_DEBUG, "username NSPlayer found\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_WINDOWSMEDIA);
  }
}
#endif

#ifdef NDPI_PROTOCOL_MMS
static void mms_parse_packet_contentline(struct ndpi_detection_module_struct
					 *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->content_line.len >= 24 && ndpi_mem_cmp(packet->content_line.ptr, "application/x-mms-framed", 24) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_MMS, ndpi_struct, NDPI_LOG_DEBUG,
	    "MMS: Content-Type: application/x-mms-framed found\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_MMS);
  }
}
#endif


#ifdef NDPI_PROTOCOL_XBOX
static void xbox_parse_packet_useragentline(struct ndpi_detection_module_struct
					    *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->user_agent_line.len >= 17 && memcmp(packet->user_agent_line.ptr, "Xbox Live Client/", 17) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_XBOX, ndpi_struct, NDPI_LOG_DEBUG, "XBOX: User Agent: Xbox Live Client found\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_XBOX);
  }
}
#endif

#ifdef NDPI_PROTOCOL_WINDOWS_UPDATE

static void windows_update_packet_useragentline(struct ndpi_detection_module_struct						
						*ndpi_struct, struct ndpi_flow_struct *flow)
{  
  struct ndpi_packet_struct *packet = &flow->packet;

  if(packet->user_agent_line.len >= 20 && memcmp(packet->user_agent_line.ptr, "Windows-Update-Agent", 20) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_WINDOWS_UPDATE, ndpi_struct, NDPI_LOG_DEBUG, "WSUS: User Agent: Windows-Update-Agent\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_WINDOWS_UPDATE);
  }
}
#endif

#ifdef NDPI_PROTOCOL_FLASH
static void flash_check_http_payload(struct ndpi_detection_module_struct
				     *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  const u_int8_t *pos;

  if (packet->empty_line_position_set == 0 || (packet->empty_line_position + 10) > (packet->payload_packet_len))
    return;

  pos = &packet->payload[packet->empty_line_position] + 2;


  if (memcmp(pos, "FLV", 3) == 0 && pos[3] == 0x01 && (pos[4] == 0x01 || pos[4] == 0x04 || pos[4] == 0x05)
      && pos[5] == 0x00 && pos[6] == 0x00 && pos[7] == 0x00 && pos[8] == 0x09) {

    NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG, "Flash content in http detected\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_FLASH);
  }
}
#endif

#ifdef NDPI_PROTOCOL_AVI
static void avi_check_http_payload(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  

  NDPI_LOG(NDPI_PROTOCOL_AVI, ndpi_struct, NDPI_LOG_DEBUG, "called avi_check_http_payload: %u %u %u\n",
	  packet->empty_line_position_set, flow->l4.tcp.http_empty_line_seen, packet->empty_line_position);

  if (packet->empty_line_position_set == 0 && flow->l4.tcp.http_empty_line_seen == 0)
    return;

  if (packet->empty_line_position_set != 0 && ((packet->empty_line_position + 20) > (packet->payload_packet_len))
      && flow->l4.tcp.http_empty_line_seen == 0) {
    flow->l4.tcp.http_empty_line_seen = 1;
    return;
  }

  if (flow->l4.tcp.http_empty_line_seen == 1) {
    if (packet->payload_packet_len > 20 && memcmp(packet->payload, "RIFF", 4) == 0
	&& memcmp(packet->payload + 8, "AVI LIST", 8) == 0) {
      NDPI_LOG(NDPI_PROTOCOL_AVI, ndpi_struct, NDPI_LOG_DEBUG, "Avi content in http detected\n");
      ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_AVI);
    }
    flow->l4.tcp.http_empty_line_seen = 0;
    return;
  }

  if (packet->empty_line_position_set != 0) {
    // check for avi header
    // for reference see http://msdn.microsoft.com/archive/default.asp?url=/archive/en-us/directx9_c/directx/htm/avirifffilereference.asp
    u_int32_t p = packet->empty_line_position + 2;

    NDPI_LOG(NDPI_PROTOCOL_AVI, ndpi_struct, NDPI_LOG_DEBUG, "p = %u\n", p);

    if ((p + 16) <= packet->payload_packet_len && memcmp(&packet->payload[p], "RIFF", 4) == 0
	&& memcmp(&packet->payload[p + 8], "AVI LIST", 8) == 0) {
      NDPI_LOG(NDPI_PROTOCOL_AVI, ndpi_struct, NDPI_LOG_DEBUG, "Avi content in http detected\n");
      ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_AVI);
    }
  }
}
#endif

#ifdef NDPI_PROTOCOL_TEAMVIEWER
static void teamviewer_check_http_payload(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
    struct ndpi_packet_struct *packet = &flow->packet;
    const u_int8_t *pos;
    
    NDPI_LOG(NDPI_PROTOCOL_TEAMVIEWER, ndpi_struct, NDPI_LOG_DEBUG, "called teamviewer_check_http_payload: %u %u %u\n", 
            packet->empty_line_position_set, flow->l4.tcp.http_empty_line_seen, packet->empty_line_position);

    if (packet->empty_line_position_set == 0 || (packet->empty_line_position + 5) > (packet->payload_packet_len))
        return;

    pos = &packet->payload[packet->empty_line_position] + 2;

    if (pos[0] == 0x17 && pos[1] == 0x24) {
        NDPI_LOG(NDPI_PROTOCOL_TEAMVIEWER, ndpi_struct, NDPI_LOG_DEBUG, "TeamViewer content in http detected\n");
        ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_TEAMVIEWER);
    }
}
#endif

#ifdef NDPI_PROTOCOL_OFF
static void off_parse_packet_contentline(struct ndpi_detection_module_struct
					 *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->content_line.len >= 4 && memcmp(packet->content_line.ptr, "off/", 4) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_OFF, ndpi_struct, NDPI_LOG_DEBUG, "off: Content-Type: off/ found\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_OFF);
  }
}
#endif

#ifdef NDPI_PROTOCOL_MOVE
static void move_parse_packet_contentline(struct ndpi_detection_module_struct
					  *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->content_line.len == 15
      && (memcmp(packet->content_line.ptr, "application/qmx", 15) == 0
	  || memcmp(packet->content_line.ptr, "application/qss", 15) == 0)) {
    NDPI_LOG(NDPI_PROTOCOL_MOVE, ndpi_struct, NDPI_LOG_DEBUG, "MOVE application qmx or qss detected\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_MOVE);
  }
}
#endif

#ifdef NDPI_PROTOCOL_RTSP
static void rtsp_parse_packet_acceptline(struct ndpi_detection_module_struct
					 *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  if (packet->accept_line.len >= 28 && memcmp(packet->accept_line.ptr, "application/x-rtsp-tunnelled", 28) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_RTSP, ndpi_struct, NDPI_LOG_DEBUG, "RTSP accept line detected\n");
    ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_RTSP);
  }
}
#endif

/*
 * Find the first occurrence of find in s, where the search is limited to the
 * first slen characters of s.
 */
char* ndpi_strnstr(const char *s, const char *find, size_t slen) {
  char c, sc;
  size_t len;

  if ((c = *find++) != '\0') {
    len = strlen(find);
    do {
      do {
	if (slen-- < 1 || (sc = *s++) == '\0')
	  return (NULL);
      } while (sc != c);
      if (len > slen)
	return (NULL);
    } while (strncmp(s, find, len) != 0);
    s--;
  }
  return ((char *)s);
}


static ndpi_protocol_match host_match[] = {
  { ".twitter.com",      NDPI_PROTOCOL_TWITTER },
  { ".netflix.com",      NDPI_PROTOCOL_NETFLIX },
  { ".twttr.com",        NDPI_PROTOCOL_TWITTER },
  { ".facebook.com",     NDPI_PROTOCOL_FACEBOOK },
  { ".fbcdn.net",        NDPI_PROTOCOL_FACEBOOK },
  { ".dropbox.com",      NDPI_PROTOCOL_DROPBOX },
  { ".gmail.",           NDPI_PROTOCOL_GMAIL },
  { "maps.google.com",   NDPI_PROTOCOL_GOOGLE_MAPS },
  { "maps.gstatic.com",  NDPI_PROTOCOL_GOOGLE_MAPS },
  { ".gstatic.com",      NDPI_PROTOCOL_GOOGLE },
  { ".google.com",       NDPI_PROTOCOL_GOOGLE },
  { ".youtube.",         NDPI_PROTOCOL_YOUTUBE },
  { "itunes.apple.com",  NDPI_PROTOCOL_APPLE_ITUNES },
  { ".apple.com",        NDPI_PROTOCOL_APPLE },
  { ".mzstatic.com",     NDPI_PROTOCOL_APPLE },
  { ".facebook.com",     NDPI_PROTOCOL_FACEBOOK },
  { ".icloud.com",       NDPI_PROTOCOL_APPLE_ICLOUD },
  { ".viber.com",        NDPI_PROTOCOL_VIBER },
  { ".last.fm",          NDPI_PROTOCOL_LASTFM },
  { ".grooveshark.com",  NDPI_PROTOCOL_GROOVESHARK },
  { ".tuenti.com",       NDPI_PROTOCOL_TUENTI },
  { NULL, 0 }
};

int matchStringProtocol(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow, 
			char *string_to_match, u_int string_to_match_len) {
  int i = 0;
  struct ndpi_packet_struct *packet = &flow->packet;

  while(host_match[i].string_to_match != NULL) {
    if(ndpi_strnstr(string_to_match, 
		    host_match[i].string_to_match, 
		    string_to_match_len) != NULL) {
      packet->detected_protocol_stack[0] = host_match[i].protocol_id;
      return(packet->detected_protocol_stack[0]);
    } else
      i++;
  }

#ifdef DEBUG
  string_to_match[string_to_match_len] = '\0';
  printf("[NTOP] Unable to find a match for '%s'\n", string_to_match);
#endif

  return(-1);
}

static void parseHttpSubprotocol(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow) {
  int i = 0;
  struct ndpi_packet_struct *packet = &flow->packet;

  if(packet->iph /* IPv4 only */) {
    /* 
       Twitter Inc. TWITTER-NETWORK (NET-199-59-148-0-1) 199.59.148.0 - 199.59.151.255
       199.59.148.0/22
    */
    if(((ntohl(packet->iph->saddr) & 0xFFFFFC00 /* 255.255.252.0 */) == 0xC73B9400 /* 199.59.148.0 */)
       || ((ntohl(packet->iph->daddr) & 0xFFFFFC00 /* 255.255.252.0 */) == 0xC73B9400 /* 199.59.148.0 */)) {
      packet->detected_protocol_stack[0] = NDPI_PROTOCOL_TWITTER;
      return;
    }


    /* 
       CIDR:           69.53.224.0/19
       OriginAS:       AS2906
       NetName:        NETFLIX-INC
    */
    if(((ntohl(packet->iph->saddr) & 0xFFFFE000 /* 255.255.224.0 */) == 0x4535E000 /* 69.53.224.0 */)
       || ((ntohl(packet->iph->daddr) & 0xFFFFE000 /* 255.255.224.0 */) == 0x4535E000 /* 69.53.224.0 */)) {
      packet->detected_protocol_stack[0] = NDPI_PROTOCOL_NETFLIX;
      return;
    }
  }
    
  if (packet->detected_protocol_stack[0] != NDPI_PROTOCOL_HTTP) 
    return;

  matchStringProtocol(ndpi_struct, flow,
		      (char*)packet->host_line.ptr, 
		      packet->host_line.len);
}

static void check_content_type_and_change_protocol(struct ndpi_detection_module_struct
						   *ndpi_struct, struct ndpi_flow_struct *flow)
{
#ifdef NDPI_PROTOCOL_MPEG
  struct ndpi_packet_struct *packet = &flow->packet;
#endif
#ifdef NDPI_PROTOCOL_AVI
#endif
  //      struct ndpi_id_struct         *src=ndpi_struct->src;
  //      struct ndpi_id_struct         *dst=ndpi_struct->dst;

  u_int8_t a;

  if (packet->content_line.ptr != NULL && packet->content_line.len != 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "Content Type Line found %.*s\n",
	    packet->content_line.len, packet->content_line.ptr);
#ifdef NDPI_PROTOCOL_MPEG
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_MPEG) != 0)
      mpeg_parse_packet_contentline(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_FLASH
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_FLASH) != 0)
      flash_parse_packet_contentline(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_QUICKTIME
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_QUICKTIME) != 0)
      qt_parse_packet_contentline(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_REALMEDIA
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_REALMEDIA) != 0)
      realmedia_parse_packet_contentline(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_WINDOWSMEDIA
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_WINDOWSMEDIA) != 0)
      windowsmedia_parse_packet_contentline(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_MMS
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_MMS) != 0)
      mms_parse_packet_contentline(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_OFF
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_OFF) != 0)
      off_parse_packet_contentline(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_OGG
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_OGG) != 0)
      ogg_parse_packet_contentline(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_MOVE
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_MOVE) != 0)
      move_parse_packet_contentline(ndpi_struct, flow);
#endif
  }
  /* check user agent here too */
  if (packet->user_agent_line.ptr != NULL && packet->user_agent_line.len != 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "User Agent Type Line found %.*s\n",
	    packet->user_agent_line.len, packet->user_agent_line.ptr);
#ifdef NDPI_PROTOCOL_XBOX
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_XBOX) != 0)
      xbox_parse_packet_useragentline(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_WINDOWS_UPDATE    
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_WINDOWS_UPDATE) != 0)
      windows_update_packet_useragentline(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_WINDOWSMEDIA
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_WINDOWSMEDIA) != 0)
      winmedia_parse_packet_useragentline(ndpi_struct, flow);
#endif

  }
  /* check for host line */
  if (packet->host_line.ptr != NULL) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HOST Line found %.*s\n",
	    packet->host_line.len, packet->host_line.ptr);
#ifdef NDPI_PROTOCOL_QQ
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_QQ) != 0) {
      qq_parse_packet_URL_and_hostname(ndpi_struct, flow);
    }
#endif

    parseHttpSubprotocol(ndpi_struct, flow);
  }

  /* check for accept line */
  if (packet->accept_line.ptr != NULL) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "Accept Line found %.*s\n",
	    packet->accept_line.len, packet->accept_line.ptr);
#ifdef NDPI_PROTOCOL_RTSP
    if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_RTSP) != 0) {
      rtsp_parse_packet_acceptline(ndpi_struct, flow);
    }
#endif
  }
  /* search for line startin with "Icy-MetaData" */
#ifdef NDPI_PROTOCOL_MPEG
  for (a = 0; a < packet->parsed_lines; a++) {
    if (packet->line[a].len > 11 && memcmp(packet->line[a].ptr, "Icy-MetaData", 12) == 0) {
      NDPI_LOG(NDPI_PROTOCOL_MPEG, ndpi_struct, NDPI_LOG_DEBUG, "MPEG: Icy-MetaData found.\n");
      ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_MPEG);
      return;
    }
  }
#ifdef NDPI_PROTOCOL_AVI
#endif
#endif

}

static void check_http_payload(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "called check_http_payload.\n");

#ifdef NDPI_PROTOCOL_FLASH
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_FLASH) != 0)
    flash_check_http_payload(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_AVI
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_AVI) != 0)
    avi_check_http_payload(ndpi_struct, flow);
#endif
#ifdef NDPI_PROTOCOL_TEAMVIEWER
  teamviewer_check_http_payload(ndpi_struct, flow);
#endif

}

/**
 * this functions checks whether the packet begins with a valid http request
 * @param ndpi_struct
 * @returnvalue 0 if no valid request has been found
 * @returnvalue >0 indicates start of filename but not necessarily in packet limit
 */
static u_int16_t http_request_url_offset(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  /* FIRST PAYLOAD PACKET FROM CLIENT */
  /* check if the packet starts with POST or GET */
  if (packet->payload_packet_len >= 4 && memcmp(packet->payload, "GET ", 4) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP: GET FOUND\n");
    return 4;
  } else if (packet->payload_packet_len >= 5 && memcmp(packet->payload, "POST ", 5) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP: POST FOUND\n");
    return 5;
  } else if (packet->payload_packet_len >= 8 && memcmp(packet->payload, "OPTIONS ", 8) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP: OPTIONS FOUND\n");
    return 8;
  } else if (packet->payload_packet_len >= 5 && memcmp(packet->payload, "HEAD ", 5) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP: HEAD FOUND\n");
    return 5;
  } else if (packet->payload_packet_len >= 4 && memcmp(packet->payload, "PUT ", 4) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP: PUT FOUND\n");
    return 4;
  } else if (packet->payload_packet_len >= 7 && memcmp(packet->payload, "DELETE ", 7) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP: DELETE FOUND\n");
    return 7;
  } else if (packet->payload_packet_len >= 8 && memcmp(packet->payload, "CONNECT ", 8) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP: CONNECT FOUND\n");
    return 8;
  } else if (packet->payload_packet_len >= 9 && memcmp(packet->payload, "PROPFIND ", 9) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP: PROFIND FOUND\n");
    return 9;
  } else if (packet->payload_packet_len >= 7 && memcmp(packet->payload, "REPORT ", 7) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP: REPORT FOUND\n");
    return 7;
  }

  return 0;
}

static void http_bitmask_exclude(struct ndpi_flow_struct *flow)
{
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_HTTP);
#ifdef NDPI_PROTOCOL_WINDOWS_UPDATE
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_WINDOWS_UPDATE);
#endif
#ifdef NDPI_PROTOCOL_MPEG
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_MPEG);
#endif
#ifdef NDPI_PROTOCOL_QUICKTIME
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_QUICKTIME);
#endif
#ifdef NDPI_PROTOCOL_WINDOWSMEDIA
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_WINDOWSMEDIA);
#endif
#ifdef NDPI_PROTOCOL_REALMEDIA
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_REALMEDIA);
#endif
#ifdef NDPI_PROTOCOL_AVI
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_AVI);
#endif
#ifdef NDPI_PROTOCOL_OGG
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_OGG);
#endif
#ifdef NDPI_PROTOCOL_MOVE
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_MOVE);
#endif
#ifdef NDPI_PROTOCOL_OFF
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_OFF);
#endif
#ifdef NDPI_PROTOCOL_XBOX
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_XBOX);
#endif
}

static void ndpi_search_http_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  
  //      struct ndpi_id_struct         *src=ndpi_struct->src;
  //      struct ndpi_id_struct         *dst=ndpi_struct->dst;

  u_int16_t filename_start;

  NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "search http\n");

  /* set client-server_direction */
  if (flow->l4.tcp.http_setup_dir == 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "initializes http to stage: 1 \n");
    flow->l4.tcp.http_setup_dir = 1 + packet->packet_direction;
  }

  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK
      (ndpi_struct->generic_http_packet_bitmask, packet->detected_protocol_stack[0]) != 0) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG,
	    "protocol might be detected earlier as http jump to payload type detection\n");
    goto http_parse_detection;
  }

  if (flow->l4.tcp.http_setup_dir == 1 + packet->packet_direction) {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "http stage: 1\n");

    if (flow->l4.tcp.http_wait_for_retransmission) {
      if (!packet->tcp_retransmission) {
	if (flow->packet_counter <= 5) {
	  NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "still waiting for retransmission\n");
	  return;
	} else {
	  NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "retransmission not found, exclude\n");
	  http_bitmask_exclude(flow);
	  return;
	}
      }
    }

    if (flow->l4.tcp.http_stage == 0) {
      filename_start = http_request_url_offset(ndpi_struct, flow);
      if (filename_start == 0) {
	NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "filename not found, exclude\n");
	http_bitmask_exclude(flow);
	return;
      }
      // parse packet
      ndpi_parse_packet_line_info(ndpi_struct, flow);

      if (packet->parsed_lines <= 1) {
	/* parse one more packet .. */
	NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "just one line, search next packet\n");
	flow->l4.tcp.http_stage = 1;
	return;
      }
      // parsed_lines > 1 here
      if (packet->line[0].len >= (9 + filename_start)
	  && memcmp(&packet->line[0].ptr[packet->line[0].len - 9], " HTTP/1.", 8) == 0) {
	packet->http_url_name.ptr = &packet->payload[filename_start];
	packet->http_url_name.len = packet->line[0].len - (filename_start + 9);

	packet->http_method.ptr = packet->line[0].ptr;
	packet->http_method.len = filename_start - 1;

	NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "http structure detected, adding\n");

	ndpi_int_http_add_connection(ndpi_struct, flow, (filename_start == 8) ? NDPI_PROTOCOL_HTTP_CONNECT : NDPI_PROTOCOL_HTTP);
	check_content_type_and_change_protocol(ndpi_struct, flow);
	/* HTTP found, look for host... */
	if (packet->host_line.ptr != NULL) {
	  /* aaahh, skip this direction and wait for a server reply here */
	  flow->l4.tcp.http_stage = 2;
	  NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP START HOST found\n");
	  return;
	}
	NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP START HOST found\n");

	/* host not found, check in next packet after */
	flow->l4.tcp.http_stage = 1;
	return;
      }
    } else if (flow->l4.tcp.http_stage == 1) {
      /* SECOND PAYLOAD TRAFFIC FROM CLIENT, FIRST PACKET MIGHT HAVE BEEN HTTP... */
      /* UNKNOWN TRAFFIC, HERE FOR HTTP again.. */
      // parse packet
      ndpi_parse_packet_line_info(ndpi_struct, flow);

      if (packet->parsed_lines <= 1) {

	/* wait some packets in case request is split over more than 2 packets */
	if (flow->packet_counter < 5) {
	  NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG,
		  "line still not finished, search next packet\n");
	  return;
	} else {
	  /* stop parsing here */
	  NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG,
		  "HTTP: PACKET DOES NOT HAVE A LINE STRUCTURE\n");
	  http_bitmask_exclude(flow);
	  return;
	}
      }

      if (packet->line[0].len >= 9 && memcmp(&packet->line[0].ptr[packet->line[0].len - 9], " HTTP/1.", 8) == 0) {
	ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_HTTP);
	check_content_type_and_change_protocol(ndpi_struct, flow);
	NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG,
		"HTTP START HTTP found in 2. packet, check host here...\n");
	/* HTTP found, look for host... */
	flow->l4.tcp.http_stage = 2;

	return;
      }
    }
  }
  NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP: REQUEST NOT HTTP CONFORM\n");
  http_bitmask_exclude(flow);
  return;

 http_parse_detection:
  if (flow->l4.tcp.http_setup_dir == 1 + packet->packet_direction) {
    /* we have something like http here, so check for host and content type if possible */
    if (flow->l4.tcp.http_stage == 0 || flow->l4.tcp.http_stage == 3) {
      NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP RUN MAYBE NEXT GET/POST...\n");
      // parse packet
      ndpi_parse_packet_line_info(ndpi_struct, flow);
      /* check for url here */
      filename_start = http_request_url_offset(ndpi_struct, flow);
      if (filename_start != 0 && packet->parsed_lines > 1 && packet->line[0].len >= (9 + filename_start)
	  && memcmp(&packet->line[0].ptr[packet->line[0].len - 9], " HTTP/1.", 8) == 0) {
	packet->http_url_name.ptr = &packet->payload[filename_start];
	packet->http_url_name.len = packet->line[0].len - (filename_start + 9);

	packet->http_method.ptr = packet->line[0].ptr;
	packet->http_method.len = filename_start - 1;

	NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "next http action, "
		"resetting to http and search for other protocols later.\n");
	ndpi_int_http_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_HTTP);
      }
      check_content_type_and_change_protocol(ndpi_struct, flow);
      /* HTTP found, look for host... */
      if (packet->host_line.ptr != NULL) {
	NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG,
		"HTTP RUN MAYBE NEXT HOST found, skipping all packets from this direction\n");
	/* aaahh, skip this direction and wait for a server reply here */
	flow->l4.tcp.http_stage = 2;
	return;
      }
      NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG,
	      "HTTP RUN MAYBE NEXT HOST NOT found, scanning one more packet from this direction\n");
      flow->l4.tcp.http_stage = 1;
    } else if (flow->l4.tcp.http_stage == 1) {
      // parse packet and maybe find a packet info with host ptr,...
      ndpi_parse_packet_line_info(ndpi_struct, flow);
      check_content_type_and_change_protocol(ndpi_struct, flow);
      NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP RUN second packet scanned\n");
      /* HTTP found, look for host... */
      flow->l4.tcp.http_stage = 2;
    }
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG,
	    "HTTP skipping client packets after second packet\n");
    return;
  }
  /* server response */
  if (flow->l4.tcp.http_stage > 0) {
    /* first packet from server direction, might have a content line */
    ndpi_parse_packet_line_info(ndpi_struct, flow);
    check_content_type_and_change_protocol(ndpi_struct, flow);


    if (packet->empty_line_position_set != 0 || flow->l4.tcp.http_empty_line_seen == 1) {
      NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "empty line. check_http_payload.\n");
      check_http_payload(ndpi_struct, flow);
    }
    if (flow->l4.tcp.http_stage == 2) {
      flow->l4.tcp.http_stage = 3;
    } else {
      flow->l4.tcp.http_stage = 0;
    }
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG,
	    "HTTP response first or second packet scanned,new stage is: %u\n", flow->l4.tcp.http_stage);
    return;
  } else {
    NDPI_LOG(NDPI_PROTOCOL_HTTP, ndpi_struct, NDPI_LOG_DEBUG, "HTTP response next packet skipped\n");
  }
}
#endif
