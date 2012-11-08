/*
 * ndpi_main.c
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

#include "ndpi_main.h"
#include "ndpi_protocols.h"
#include "ndpi_utils.h"
#include "protocols.c"


u_int32_t ndpi_detection_get_sizeof_ndpi_flow_struct(void)
{
  return sizeof(struct ndpi_flow_struct);
}

u_int32_t ndpi_detection_get_sizeof_ndpi_id_struct(void)
{
  return sizeof(struct ndpi_id_struct);
}


struct ndpi_detection_module_struct *ndpi_init_detection_module(u_int32_t ticks_per_second, void
								*(*ndpi_malloc)
								(unsigned
								 long size),
								ndpi_debug_function_ptr ndpi_debug_printf)
{
  struct ndpi_detection_module_struct *ndpi_str;
  ndpi_str = ndpi_malloc(sizeof(struct ndpi_detection_module_struct));

  if (ndpi_str == NULL) {
    ndpi_debug_printf(0, NULL, NDPI_LOG_DEBUG, "ndpi_init_detection_module initial malloc failed\n");
    return NULL;
  }
  memset(ndpi_str, 0, sizeof(struct ndpi_detection_module_struct));


  NDPI_BITMASK_RESET(ndpi_str->detection_bitmask);
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
  ndpi_str->ndpi_debug_printf = ndpi_debug_printf;
  ndpi_str->user_data = NULL;
#endif


  ndpi_str->ticks_per_second = ticks_per_second;
  ndpi_str->tcp_max_retransmission_window_size = NDPI_DEFAULT_MAX_TCP_RETRANSMISSION_WINDOW_SIZE;
  ndpi_str->directconnect_connection_ip_tick_timeout =
    NDPI_DIRECTCONNECT_CONNECTION_IP_TICK_TIMEOUT * ticks_per_second;

  ndpi_str->gadugadu_peer_connection_timeout = NDPI_GADGADU_PEER_CONNECTION_TIMEOUT * ticks_per_second;
  ndpi_str->edonkey_upper_ports_only = NDPI_EDONKEY_UPPER_PORTS_ONLY;
  ndpi_str->ftp_connection_timeout = NDPI_FTP_CONNECTION_TIMEOUT * ticks_per_second;

  ndpi_str->pplive_connection_timeout = NDPI_PPLIVE_CONNECTION_TIMEOUT * ticks_per_second;

  ndpi_str->rtsp_connection_timeout = NDPI_RTSP_CONNECTION_TIMEOUT * ticks_per_second;
  ndpi_str->tvants_connection_timeout = NDPI_TVANTS_CONNECTION_TIMEOUT * ticks_per_second;
  ndpi_str->irc_timeout = NDPI_IRC_CONNECTION_TIMEOUT * ticks_per_second;
  ndpi_str->gnutella_timeout = NDPI_GNUTELLA_CONNECTION_TIMEOUT * ticks_per_second;

  ndpi_str->battlefield_timeout = NDPI_BATTLEFIELD_CONNECTION_TIMEOUT * ticks_per_second;

  ndpi_str->thunder_timeout = NDPI_THUNDER_CONNECTION_TIMEOUT * ticks_per_second;
  ndpi_str->yahoo_detect_http_connections = NDPI_YAHOO_DETECT_HTTP_CONNECTIONS;

  ndpi_str->yahoo_lan_video_timeout = NDPI_YAHOO_LAN_VIDEO_TIMEOUT * ticks_per_second;
  ndpi_str->zattoo_connection_timeout = NDPI_ZATTOO_CONNECTION_TIMEOUT * ticks_per_second;
  ndpi_str->jabber_stun_timeout = NDPI_JABBER_STUN_TIMEOUT * ticks_per_second;
  ndpi_str->jabber_file_transfer_timeout = NDPI_JABBER_FT_TIMEOUT * ticks_per_second;
  ndpi_str->soulseek_connection_ip_tick_timeout = NDPI_SOULSEEK_CONNECTION_IP_TICK_TIMEOUT * ticks_per_second;
  ndpi_str->manolito_subscriber_timeout = NDPI_MANOLITO_SUBSCRIBER_TIMEOUT;
  return ndpi_str;
}

void ndpi_exit_detection_module(struct ndpi_detection_module_struct
				*ndpi_struct, void (*ndpi_free) (void *ptr))
{
  if (ndpi_struct != NULL) {
    ndpi_free(ndpi_struct);
  }
}

void ndpi_set_protocol_detection_bitmask2(struct ndpi_detection_module_struct
					  *ndpi_struct, const NDPI_PROTOCOL_BITMASK * dbm)
{
  NDPI_PROTOCOL_BITMASK detection_bitmask_local;
  NDPI_PROTOCOL_BITMASK *detection_bitmask = &detection_bitmask_local;

  u_int32_t a = 0;

  NDPI_BITMASK_SET(detection_bitmask_local, *dbm);
  NDPI_BITMASK_SET(ndpi_struct->detection_bitmask, *dbm);

  /* set this here to zero to be interrupt safe */
  ndpi_struct->callback_buffer_size = 0;

#ifdef NDPI_PROTOCOL_HTTP
#ifdef NDPI_PROTOCOL_MPEG
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MPEG) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_FLASH
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_FLASH) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_QUICKTIME
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_QUICKTIME) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_REALMEDIA
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_REALMEDIA) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_WINDOWSMEDIA
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_WINDOWSMEDIA) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_MMS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MMS) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_OFF
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_OFF) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_XBOX
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_XBOX) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_WINDOWS_UPDATE
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_WINDOWS_UPDATE) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_QQ
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_QQ) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_AVI
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_AVI) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_OGG
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_OGG) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_MOVE
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MOVE) != 0)
    goto hack_do_http_detection;
#endif
#ifdef NDPI_PROTOCOL_RTSP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_RTSP) != 0)
    goto hack_do_http_detection;
#endif
  /* HTTP DETECTION MUST BE BEFORE DDL BUT AFTER ALL OTHER PROTOCOLS WHICH USE HTTP ALSO */
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_HTTP) != 0) {

  hack_do_http_detection:

    ndpi_struct->callback_buffer[a].func = ndpi_search_http_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_HTTP);

#ifdef NDPI_PROTOCOL_MPEG
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_MPEG);
#endif
#ifdef NDPI_PROTOCOL_FLASH
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_FLASH);
#endif
#ifdef NDPI_PROTOCOL_QUICKTIME
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_QUICKTIME);
#endif
#ifdef NDPI_PROTOCOL_REALMEDIA
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_REALMEDIA);
#endif
#ifdef NDPI_PROTOCOL_WINDOWSMEDIA
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask,
				 NDPI_PROTOCOL_WINDOWSMEDIA);
#endif
#ifdef NDPI_PROTOCOL_MMS
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_MMS);
#endif
#ifdef NDPI_PROTOCOL_OFF
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_OFF);
#endif
#ifdef NDPI_PROTOCOL_XBOX
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_XBOX);
#endif
#ifdef NDPI_PROTOCOL_WINDOWS_UPDATE
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_WINDOWS_UPDATE);
#endif
#ifdef NDPI_PROTOCOL_QQ
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_QQ);
#endif
#ifdef NDPI_PROTOCOL_AVI
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_AVI);
#endif
#ifdef NDPI_PROTOCOL_OGG
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_OGG);
#endif
#ifdef NDPI_PROTOCOL_MOVE
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_MOVE);
#endif
#ifdef NDPI_PROTOCOL_RTSP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_RTSP);
#endif

    NDPI_BITMASK_SET(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
		     ndpi_struct->callback_buffer[a].detection_bitmask);
    NDPI_DEL_PROTOCOL_FROM_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				   NDPI_PROTOCOL_UNKNOWN);

    NDPI_DEL_PROTOCOL_FROM_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				   NDPI_PROTOCOL_QQ);

#ifdef NDPI_PROTOCOL_FLASH
    NDPI_DEL_PROTOCOL_FROM_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				   NDPI_PROTOCOL_FLASH);
#endif

    NDPI_DEL_PROTOCOL_FROM_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				   NDPI_PROTOCOL_MMS);
#ifdef NDPI_PROTOCOL_RTSP
    NDPI_DEL_PROTOCOL_FROM_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				   NDPI_PROTOCOL_RTSP);
#endif
    NDPI_DEL_PROTOCOL_FROM_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				   NDPI_PROTOCOL_XBOX);


    NDPI_BITMASK_SET(ndpi_struct->generic_http_packet_bitmask,
		     ndpi_struct->callback_buffer[a].detection_bitmask);

    NDPI_DEL_PROTOCOL_FROM_BITMASK(ndpi_struct->generic_http_packet_bitmask, NDPI_PROTOCOL_UNKNOWN);

    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_SSL
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SSL) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_ssl_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_SSL);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SSL);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_STUN
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_STUN) != 0
#ifdef NDPI_PROTOCOL_RTP
      || NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_RTP) != 0
#endif
      ) {

    ndpi_struct->callback_buffer[a].func = ndpi_search_stun;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_STUN);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_RTP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_RTP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_rtp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
#ifdef NDPI_PROTOCOL_STUN
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_STUN);
#endif

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_RTP);

    /* consider also real protocol for detection select in main loop */
    ndpi_struct->callback_buffer[a].detection_feature = NDPI_SELECT_DETECTION_WITH_REAL_PROTOCOL;

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_RDP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_RDP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_rdp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_RDP);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_SIP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SIP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_sip;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD; /* Fix courtesy of Miguel Quesada <mquesadab@gmail.com> */

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_SIP);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SIP);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_BITTORRENT
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_BITTORRENT) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_bittorrent;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_BITTORRENT);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_BITTORRENT);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_EDONKEY
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_EDONKEY) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_edonkey;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_EDONKEY);
#ifdef NDPI_PROTOCOL_BITTORRENT
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_BITTORRENT);
#endif
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_EDONKEY);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_FASTTRACK
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_FASTTRACK) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_fasttrack_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_FASTTRACK);
    a++;

  }
#endif
#ifdef NDPI_PROTOCOL_GNUTELLA
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_GNUTELLA) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_gnutella;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

#ifdef NDPI_PROTOCOL_XBOX
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_XBOX);
#endif
#ifdef NDPI_PROTOCOL_WINDOWS_UPDATE
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_WINDOWS_UPDATE);
#endif
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_GNUTELLA);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_WINMX
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_WINMX) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_winmx_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_WINMX);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_DIRECTCONNECT
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_DIRECTCONNECT) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_directconnect;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask,
				 NDPI_PROTOCOL_DIRECTCONNECT);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
			 NDPI_PROTOCOL_DIRECTCONNECT);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_MSN
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MSN) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_msn;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_MSN);
#ifdef NDPI_PROTOCOL_HTTP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_HTTP);
#endif
#ifdef NDPI_PROTOCOL_SSL
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_SSL);
#endif
    NDPI_BITMASK_RESET(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MSN);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_YAHOO
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_YAHOO) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_yahoo;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_YAHOO);
#ifdef NDPI_PROTOCOL_SSL
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_SSL);
#endif
#ifdef NDPI_PROTOCOL_HTTP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_HTTP);
#endif
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_YAHOO);

    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_OSCAR
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_OSCAR) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_oscar;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_OSCAR);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_OSCAR);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_APPLEJUICE
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_APPLEJUICE) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_applejuice_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_APPLEJUICE);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_SOULSEEK
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SOULSEEK) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_soulseek_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_SOULSEEK);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SOULSEEK);


    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_IRC
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_IRC) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_irc_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_IRC);

#ifdef NDPI_PROTOCOL_HTTP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_HTTP);
#endif

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_IRC);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_UNENCRYPED_JABBER
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_UNENCRYPED_JABBER) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_jabber_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask,
				 NDPI_PROTOCOL_UNENCRYPED_JABBER);
#ifdef NDPI_PROTOCOL_SSL
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_SSL);
#endif
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
			 NDPI_PROTOCOL_UNENCRYPED_JABBER);

#ifdef NDPI_PROTOCOL_TRUPHONE
    /* also exlude truphone since this is detected in jabber */
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				 NDPI_PROTOCOL_TRUPHONE);
#endif
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_MAIL_POP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MAIL_POP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_mail_pop_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MAIL_POP);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_MAIL_IMAP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MAIL_IMAP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_mail_imap_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MAIL_IMAP);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_MAIL_SMTP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MAIL_SMTP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_mail_smtp_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MAIL_SMTP);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_FTP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_FTP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_ftp_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_FTP);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_FTP);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_USENET
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_USENET) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_usenet_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_USENET);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_DNS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_DNS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_dns;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;


    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_DNS);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_FILETOPIA
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_FILETOPIA) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_filetopia_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_FILETOPIA);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_MANOLITO
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MANOLITO) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_manolito_tcp_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MANOLITO);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_IMESH
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_IMESH) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_imesh_tcp_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_SSL);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_IMESH);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_MMS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MMS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_mms_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MMS);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_PANDO
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_PANDO) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_pando_tcp_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_PANDO);

    a++;
  }
#endif
#if defined(NDPI_PROTOCOL_IPSEC) || defined(NDPI_PROTOCOL_GRE) || defined(NDPI_PROTOCOL_ICMP) || defined(NDPI_PROTOCOL_IGMP) || defined(NDPI_PROTOCOL_EGP) || defined(NDPI_PROTOCOL_SCTP) || defined(NDPI_PROTOCOL_OSPF) || defined(NDPI_PROTOCOL_IP_IN_IP) || defined(NDPI_PROTOCOL_ICMPV6)
  /* always add non tcp/udp if one protocol is compiled in */
  if (1) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_in_non_tcp_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_IPV4_OR_IPV6;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_BITMASK_RESET(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask);
#ifdef NDPI_PROTOCOL_IPSEC
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				 NDPI_PROTOCOL_IPSEC);
#endif
#ifdef NDPI_PROTOCOL_GRE
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				 NDPI_PROTOCOL_GRE);
#endif
#ifdef NDPI_PROTOCOL_IGMP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				 NDPI_PROTOCOL_ICMP);
#endif
#ifdef NDPI_PROTOCOL_IGMP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				 NDPI_PROTOCOL_IGMP);
#endif
#ifdef NDPI_PROTOCOL_EGP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				 NDPI_PROTOCOL_EGP);
#endif
#ifdef NDPI_PROTOCOL_SCTP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				 NDPI_PROTOCOL_SCTP);
#endif
#ifdef NDPI_PROTOCOL_OSPF
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				 NDPI_PROTOCOL_OSPF);
#endif
#ifdef NDPI_PROTOCOL_IP_IN_IP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				 NDPI_PROTOCOL_IP_IN_IP);
#endif
#ifdef NDPI_PROTOCOL_ICMPV6
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
				 NDPI_PROTOCOL_ICMPV6);
#endif
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_TVANTS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_TVANTS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_tvants_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_TVANTS);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_SOPCAST
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SOPCAST) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_sopcast;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SOPCAST);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_TVUPLAYER
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_TVUPLAYER) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_tvuplayer;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_TVUPLAYER);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_PPSTREAM
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_PPSTREAM) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_ppstream;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_PPSTREAM);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_PPLIVE
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_PPLIVE) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_pplive_tcp_udp;
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_PPLIVE);
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_PPLIVE);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_IAX
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_IAX) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_iax;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_IAX);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_IAX);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_MGCP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MGCP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_mgcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    //NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_MGCP);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MGCP);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_GADUGADU
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_GADUGADU) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_gadugadu;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
#ifdef NDPI_PROTOCOL_HTTP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_HTTP);
#endif
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_GADUGADU);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_GADUGADU);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_ZATTOO
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_ZATTOO) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_zattoo;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_ZATTOO);
#ifdef NDPI_PROTOCOL_FLASH
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_FLASH);
#endif
#ifdef NDPI_PROTOCOL_HTTP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_HTTP);
#endif
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_ZATTOO);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_QQ
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_QQ) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_qq;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_QQ);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_QQ);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_FEIDIAN
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_FEIDIAN) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_feidian;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_FEIDIAN);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_SSH
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SSH) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_ssh_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SSH);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_POPO
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_POPO) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_popo_tcp_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_POPO);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_THUNDER
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_THUNDER) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_thunder;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_THUNDER);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_VNC
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_VNC) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_vnc_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_VNC);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_TEAMVIEWER
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_TEAMVIEWER) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_teamview;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_TEAMVIEWER);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_TEAMVIEWER);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_DHCP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_DHCP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_dhcp_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_DHCP);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_I23V5
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_I23V5) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_i23v5;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_I23V5);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_SOCRATES
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SOCRATES) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_socrates;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SOCRATES);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_STEAM
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_STEAM) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_steam;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_STEAM);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_HALFLIFE2
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_HALFLIFE2) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_halflife2;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_HALFLIFE2);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_XBOX
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_XBOX) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_xbox;

    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_XBOX);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_HTTP_APPLICATION_ACTIVESYNC
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_HTTP_APPLICATION_ACTIVESYNC) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_activesync;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
			 NDPI_PROTOCOL_HTTP_APPLICATION_ACTIVESYNC);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_SMB
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SMB) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_smb_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SMB);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_TELNET
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_TELNET) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_telnet_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_TELNET);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_NTP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_NTP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_ntp_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_NTP);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_NFS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_NFS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_nfs;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_NFS);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_SSDP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SSDP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_ssdp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SSDP);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_WORLDOFWARCRAFT
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_WORLDOFWARCRAFT) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_worldofwarcraft;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
			 NDPI_PROTOCOL_WORLDOFWARCRAFT);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_FLASH
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_FLASH) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_flash;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_FLASH);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_FLASH);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_POSTGRES
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_POSTGRES) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_postgres_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_POSTGRES);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_MYSQL
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MYSQL) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_mysql_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MYSQL);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_BGP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_BGP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_bgp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_BGP);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_QUAKE
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_QUAKE) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_quake;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_QUAKE);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_BATTLEFIELD
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_BATTLEFIELD) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_battlefield;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
			 NDPI_PROTOCOL_BATTLEFIELD);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_SECONDLIFE
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SECONDLIFE) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_secondlife;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

#ifdef NDPI_PROTOCOL_SSL
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_SSL);
#endif

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SECONDLIFE);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_PCANYWHERE
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_PCANYWHERE) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_pcanywhere;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_PCANYWHERE);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_SNMP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SNMP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_snmp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SNMP);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_KONTIKI
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_KONTIKI) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_kontiki;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_KONTIKI);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_ICECAST
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_ICECAST) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_icecast_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
#ifdef	NDPI_PROTOCOL_HTTP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_HTTP);
#endif
#ifdef NDPI_PROTOCOL_MPEG
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_MPEG);
#endif
#ifdef NDPI_PROTOCOL_QUICKTIME
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_QUICKTIME);
#endif
#ifdef NDPI_PROTOCOL_REALMEDIA
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_REALMEDIA);
#endif
#ifdef NDPI_PROTOCOL_WINDOWSMEDIA
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask,
				 NDPI_PROTOCOL_WINDOWSMEDIA);
#endif
#ifdef NDPI_PROTOCOL_AVI
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_AVI);
#endif
#ifdef NDPI_PROTOCOL_OGG
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_OGG);
#endif

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_ICECAST);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_SHOUTCAST
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SHOUTCAST) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_shoutcast_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
#ifdef	NDPI_PROTOCOL_HTTP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_HTTP);
#endif
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SHOUTCAST);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_HTTP_APPLICATION_VEOHTV
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_HTTP_APPLICATION_VEOHTV) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_veohtv_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
#ifdef	NDPI_PROTOCOL_HTTP
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_HTTP);
#endif
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
			 NDPI_PROTOCOL_HTTP_APPLICATION_VEOHTV);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_KERBEROS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_KERBEROS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_kerberos;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_KERBEROS);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_OPENFT
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_OPENFT) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_openft_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_OPENFT);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_SYSLOG
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SYSLOG) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_syslog;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SYSLOG);

    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_TDS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_TDS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_tds_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_TDS);

    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_DIRECT_DOWNLOAD_LINK
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_DIRECT_DOWNLOAD_LINK) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_direct_download_link_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD;


    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask,
				 NDPI_PROTOCOL_DIRECT_DOWNLOAD_LINK);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
			 NDPI_PROTOCOL_DIRECT_DOWNLOAD_LINK);

    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_NETBIOS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_NETBIOS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_netbios;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_NETBIOS);

    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_MDNS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MDNS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_mdns;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;


    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MDNS);

    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_IPP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_IPP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_ipp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_IPP);

    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_LDAP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_LDAP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_ldap;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_LDAP);

    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_WARCRAFT3
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_WARCRAFT3) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_warcraft3;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;


    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_WARCRAFT3);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_WARCRAFT3);

    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_XDMCP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_XDMCP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_xdmcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;


    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_XDMCP);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_XDMCP);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_TFTP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_TFTP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_tftp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_TFTP);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_MSSQL
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MSSQL) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_mssql;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MSSQL);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_PPTP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_PPTP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_pptp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_PPTP);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_STEALTHNET
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_STEALTHNET) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_stealthnet;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_STEALTHNET);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_DHCPV6
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_DHCPV6) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_dhcpv6_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_DHCPV6);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_MEEBO
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MEEBO) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_meebo;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
#ifdef NDPI_PROTOCOL_FLASH
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_FLASH);
#endif

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MEEBO);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_AFP
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_AFP) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_afp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_AFP);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_AIMINI
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_AIMINI) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_aimini;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_AIMINI);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_FLORENSIA
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_FLORENSIA) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_florensia;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_FLORENSIA);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_MAPLESTORY
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_MAPLESTORY) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_maplestory;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_MAPLESTORY);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_DOFUS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_DOFUS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_dofus;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_DOFUS);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_WORLD_OF_KUNG_FU
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_WORLD_OF_KUNG_FU) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_world_of_kung_fu;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask,
			 NDPI_PROTOCOL_WORLD_OF_KUNG_FU);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_FIESTA
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_FIESTA) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_fiesta;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_FIESTA);
    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_CROSSFIRE
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_CROSSFIRE) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_crossfire_tcp_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_CROSSFIRE);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_GUILDWARS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_GUILDWARS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_guildwars_tcp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_GUILDWARS);

    a++;
  }
#endif
#ifdef NDPI_PROTOCOL_ARMAGETRON
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_ARMAGETRON) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_armagetron_udp;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask = NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_ARMAGETRON);

    a++;
  }
#endif


#ifdef NDPI_PROTOCOL_DROPBOX
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_DROPBOX) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_dropbox;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_DROPBOX);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_DROPBOX);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_SKYPE
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SKYPE) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_skype;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_SKYPE);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SKYPE);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_RADIUS
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_RADIUS) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_radius;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_RADIUS);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_RADIUS);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_CITRIX
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_CITRIX) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_citrix;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_CITRIX);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_CITRIX);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_DCERPC
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_DCERPC) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_dcerpc;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_DCERPC);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_DCERPC);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_NETFLOW
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_NETFLOW) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_netflow;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_NETFLOW);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_NETFLOW);
    a++;
  }
#endif

#ifdef NDPI_PROTOCOL_SFLOW
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, NDPI_PROTOCOL_SFLOW) != 0) {
    ndpi_struct->callback_buffer[a].func = ndpi_search_sflow;
    ndpi_struct->callback_buffer[a].ndpi_selection_bitmask =
      NDPI_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_UNKNOWN);
    NDPI_ADD_PROTOCOL_TO_BITMASK(ndpi_struct->callback_buffer[a].detection_bitmask, NDPI_PROTOCOL_SFLOW);
    NDPI_SAVE_AS_BITMASK(ndpi_struct->callback_buffer[a].excluded_protocol_bitmask, NDPI_PROTOCOL_SFLOW);
    a++;
  }
#endif

  ndpi_struct->callback_buffer_size = a;

  NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG,
	   "callback_buffer_size is %u\n", ndpi_struct->callback_buffer_size);

  /* now build the specific buffer for tcp, udp and non_tcp_udp */
  ndpi_struct->callback_buffer_size_tcp_payload = 0;
  ndpi_struct->callback_buffer_size_tcp_no_payload = 0;
  for (a = 0; a < ndpi_struct->callback_buffer_size; a++) {
    if ((ndpi_struct->callback_buffer[a].ndpi_selection_bitmask & (NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP |
								   NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP |
								   NDPI_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC))
	!= 0) {
      NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG,
	       "callback_buffer_tcp_payload, adding buffer %u as entry %u\n", a,
	       ndpi_struct->callback_buffer_size_tcp_payload);

      memcpy(&ndpi_struct->callback_buffer_tcp_payload[ndpi_struct->callback_buffer_size_tcp_payload],
	     &ndpi_struct->callback_buffer[a], sizeof(struct ndpi_call_function_struct));
      ndpi_struct->callback_buffer_size_tcp_payload++;

      if ((ndpi_struct->
	   callback_buffer[a].ndpi_selection_bitmask & NDPI_SELECTION_BITMASK_PROTOCOL_HAS_PAYLOAD) == 0) {
	NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG,
		 "callback_buffer_tcp_no_payload, additional adding buffer %u to no_payload process\n", a);

	memcpy(&ndpi_struct->callback_buffer_tcp_no_payload
	       [ndpi_struct->callback_buffer_size_tcp_no_payload], &ndpi_struct->callback_buffer[a],
	       sizeof(struct ndpi_call_function_struct));
	ndpi_struct->callback_buffer_size_tcp_no_payload++;
      }
    }
  }

  ndpi_struct->callback_buffer_size_udp = 0;
  for (a = 0; a < ndpi_struct->callback_buffer_size; a++) {
    if ((ndpi_struct->callback_buffer[a].ndpi_selection_bitmask & (NDPI_SELECTION_BITMASK_PROTOCOL_INT_UDP |
								   NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP |
								   NDPI_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC))
	!= 0) {
      NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG,
	       "callback_buffer_size_udp, adding buffer %u\n", a);

      memcpy(&ndpi_struct->callback_buffer_udp[ndpi_struct->callback_buffer_size_udp],
	     &ndpi_struct->callback_buffer[a], sizeof(struct ndpi_call_function_struct));
      ndpi_struct->callback_buffer_size_udp++;
    }
  }

  ndpi_struct->callback_buffer_size_non_tcp_udp = 0;
  for (a = 0; a < ndpi_struct->callback_buffer_size; a++) {
    if ((ndpi_struct->callback_buffer[a].ndpi_selection_bitmask & (NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP |
								   NDPI_SELECTION_BITMASK_PROTOCOL_INT_UDP |
								   NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP))
	== 0
	|| (ndpi_struct->
	    callback_buffer[a].ndpi_selection_bitmask & NDPI_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC) != 0) {
      NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG,
	       "callback_buffer_size_non_tcp_udp, adding buffer %u\n", a);

      memcpy(&ndpi_struct->callback_buffer_non_tcp_udp[ndpi_struct->callback_buffer_size_non_tcp_udp],
	     &ndpi_struct->callback_buffer[a], sizeof(struct ndpi_call_function_struct));
      ndpi_struct->callback_buffer_size_non_tcp_udp++;
    }
  }

}

#ifdef NDPI_DETECTION_SUPPORT_IPV6
/* handle extension headers in IPv6 packets
 * arguments:
 * 	l4ptr: pointer to the byte following the initial IPv6 header
 * 	l4len: the length of the IPv6 packet excluding the IPv6 header
 * 	nxt_hdr: next header value from the IPv6 header
 * result:
 * 	l4ptr: pointer to the start of the actual packet payload
 * 	l4len: length of the actual payload
 * 	nxt_hdr: protocol of the actual payload
 * returns 0 upon success and 1 upon failure
 */
static int ndpi_handle_ipv6_extension_headers(struct ndpi_detection_module_struct *ndpi_struct,
					      const u_int8_t ** l4ptr, u_int16_t * l4len, u_int8_t * nxt_hdr)
{
  while ((*nxt_hdr == 0 || *nxt_hdr == 43 || *nxt_hdr == 44 || *nxt_hdr == 60 || *nxt_hdr == 135 || *nxt_hdr == 59)) {
    u_int16_t ehdr_len;

    // no next header
    if (*nxt_hdr == 59) {
      return 1;
    }
    // fragment extension header has fixed size of 8 bytes and the first byte is the next header type
    if (*nxt_hdr == 44) {
      if (*l4len < 8) {
	return 1;
      }
      *nxt_hdr = (*l4ptr)[0];
      *l4len -= 8;
      (*l4ptr) += 8;
      continue;
    }
    // the other extension headers have one byte for the next header type
    // and one byte for the extension header length in 8 byte steps minus the first 8 bytes
    ehdr_len = (*l4ptr)[1];
    ehdr_len *= 8;
    ehdr_len += 8;

    if (*l4len < ehdr_len) {
      return 1;
    }
    *nxt_hdr = (*l4ptr)[0];
    *l4len -= ehdr_len;
    (*l4ptr) += ehdr_len;
  }
  return 0;
}
#endif							/* NDPI_DETECTION_SUPPORT_IPV6 */
static u_int8_t ndpi_iph_is_valid_and_not_fragmented(const struct ndpi_iphdr *iph, const u_int16_t ipsize)
{
#ifdef REQUIRE_FULL_PACKETS
  if (ipsize < iph->ihl * 4 ||
      ipsize < ntohs(iph->tot_len) || ntohs(iph->tot_len) < iph->ihl * 4 || (iph->frag_off & htons(0x1FFF)) != 0) {
    return 0;
  }
#endif

  return 1;
}

static u_int8_t ndpi_detection_get_l4_internal(struct ndpi_detection_module_struct *ndpi_struct,
					       const u_int8_t * l3, u_int16_t l3_len, const u_int8_t ** l4_return, u_int16_t * l4_len_return,
					       u_int8_t * l4_protocol_return, u_int32_t flags)
{
  const struct ndpi_iphdr *iph = NULL;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  const struct ndpi_ipv6hdr *iph_v6 = NULL;
#endif
  u_int16_t l4len = 0;
  const u_int8_t *l4ptr = NULL;
  u_int8_t l4protocol = 0;

  if (l3 == NULL || l3_len < sizeof(struct ndpi_iphdr))
    return 1;

  iph = (const struct ndpi_iphdr *) l3;

  if (iph->version == 4 && iph->ihl >= 5) {
    NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG, "ipv4 header\n");
  }
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  else if (iph->version == 6 && l3_len >= sizeof(struct ndpi_ipv6hdr)) {
    NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG, "ipv6 header\n");
    iph_v6 = (const struct ndpi_ipv6hdr *) iph;
    iph = NULL;
  }
#endif
  else {
    return 1;
  }

  if ((flags & NDPI_DETECTION_ONLY_IPV6) && iph != NULL) {
    NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG, "ipv4 header found but excluded by flag\n");
    return 1;
  }
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  else if ((flags & NDPI_DETECTION_ONLY_IPV4) && iph_v6 != NULL) {
    NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG, "ipv6 header found but excluded by flag\n");
    return 1;
  }
#endif

  if (iph != NULL && ndpi_iph_is_valid_and_not_fragmented(iph, l3_len)) {
    u_int16_t len  = ntohs(iph->tot_len);
    u_int16_t hlen = (iph->ihl * 4);

    l4ptr = (((const u_int8_t *) iph) + iph->ihl * 4);

    if(len == 0) len = l3_len;

    l4len = (len > hlen) ? (len - hlen) : 0;
    l4protocol = iph->protocol;
  }
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  else if (iph_v6 != NULL && (l3_len - sizeof(struct ndpi_ipv6hdr)) >= ntohs(iph_v6->payload_len)) {
    l4ptr = (((const u_int8_t *) iph_v6) + sizeof(struct ndpi_ipv6hdr));
    l4len = ntohs(iph_v6->payload_len);
    l4protocol = iph_v6->nexthdr;

    // we need to handle IPv6 extension headers if present
    if (ndpi_handle_ipv6_extension_headers(ndpi_struct, &l4ptr, &l4len, &l4protocol) != 0) {
      return 1;
    }

  }
#endif
  else {
    return 1;
  }

  if (l4_return != NULL) {
    *l4_return = l4ptr;
  }

  if (l4_len_return != NULL) {
    *l4_len_return = l4len;
  }

  if (l4_protocol_return != NULL) {
    *l4_protocol_return = l4protocol;
  }

  return 0;
}

#if !defined(WIN32)
#define ATTRIBUTE_ALWAYS_INLINE static inline
#else
__forceinline static
#endif
void ndpi_apply_flow_protocol_to_packet(struct ndpi_flow_struct *flow,
					struct ndpi_packet_struct *packet)
{
  memcpy(&packet->detected_protocol_stack[0],
	 &flow->detected_protocol_stack[0], sizeof(packet->detected_protocol_stack));
#if NDPI_PROTOCOL_HISTORY_SIZE > 1
  memcpy(&packet->protocol_stack_info, &flow->protocol_stack_info, sizeof(packet->protocol_stack_info));
#endif
}

static int ndpi_init_packet_header(struct ndpi_detection_module_struct *ndpi_struct, 
				   struct ndpi_flow_struct *flow,
				   unsigned short packetlen)
{
  const struct ndpi_iphdr *decaps_iph = NULL;
  u_int16_t l3len;
  u_int16_t l4len;
  const u_int8_t *l4ptr;
  u_int8_t l4protocol;
  u_int8_t l4_result;

  /* reset payload_packet_len, will be set if ipv4 tcp or udp */
  flow->packet.payload_packet_len = 0;
  flow->packet.l4_packet_len = 0;
  flow->packet.l3_packet_len = packetlen;

  flow->packet.tcp = NULL;
  flow->packet.udp = NULL;
  flow->packet.generic_l4_ptr = NULL;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  flow->packet.iphv6 = NULL;
#endif							/* NDPI_DETECTION_SUPPORT_IPV6 */

  if (flow) {
    ndpi_apply_flow_protocol_to_packet(flow, &flow->packet);
  } else {
    ndpi_int_reset_packet_protocol(&flow->packet);
  }

  l3len =flow->packet.l3_packet_len;

#ifdef NDPI_DETECTION_SUPPORT_IPV6
  if (flow->packet.iph != NULL) {
#endif							/* NDPI_DETECTION_SUPPORT_IPV6 */

    decaps_iph =flow->packet.iph;

#ifdef NDPI_DETECTION_SUPPORT_IPV6
  }
#endif							/* NDPI_DETECTION_SUPPORT_IPV6 */

  if (decaps_iph->version == 4 && decaps_iph->ihl >= 5) {
    NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG, "ipv4 header\n");
  }
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  else if (decaps_iph->version == 6 && l3len >= sizeof(struct ndpi_ipv6hdr) &&
	   (ndpi_struct->ip_version_limit & NDPI_DETECTION_ONLY_IPV4) == 0) {
    NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG, "ipv6 header\n");
    flow->packet.iphv6 = (struct ndpi_ipv6hdr *)flow->packet.iph;
    flow->packet.iph = NULL;
  }
#endif
  else {
    flow->packet.iph = NULL;
    return 1;
  }


  /* needed:
   *  - unfragmented packets
   *  - ip header <= packet len
   *  - ip total length >= packet len
   */


  l4ptr = NULL;
  l4len = 0;
  l4protocol = 0;

  l4_result =
    ndpi_detection_get_l4_internal(ndpi_struct, (const u_int8_t *) decaps_iph, l3len, &l4ptr, &l4len, &l4protocol, 0);

  if (l4_result != 0) {
    return 1;
  }

  flow->packet.l4_protocol = l4protocol;
  flow->packet.l4_packet_len = l4len;

  /* tcp / udp detection */
  if (l4protocol == 6 /* TCP */  &&flow->packet.l4_packet_len >= 20 /* min size of tcp */ ) {
    /* tcp */
    flow->packet.tcp = (struct ndpi_tcphdr *) l4ptr;

    if (flow->packet.l4_packet_len >=flow->packet.tcp->doff * 4) {
      flow->packet.payload_packet_len =
	flow->packet.l4_packet_len -flow->packet.tcp->doff * 4;
      flow->packet.actual_payload_len =flow->packet.payload_packet_len;
      flow->packet.payload = ((u_int8_t *)flow->packet.tcp) + (flow->packet.tcp->doff * 4);

      /* check for new tcp syn packets, here
       * idea: reset detection state if a connection is unknown
       */
      if (flow && flow->packet.tcp->syn != 0
	  && flow->packet.tcp->ack == 0
	  && flow->init_finished != 0
	  && flow->detected_protocol_stack[0] == NDPI_PROTOCOL_UNKNOWN) {

	memset(flow, 0, sizeof(*(flow)));


	NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct,
		 NDPI_LOG_DEBUG,
		 "%s:%u: tcp syn packet for unknown protocol, reset detection state\n", __FUNCTION__, __LINE__);

      }
    } else {
      /* tcp header not complete */
      flow->packet.tcp = NULL;
    }
  } else if (l4protocol == 17 /* udp */  &&flow->packet.l4_packet_len >= 8 /* size of udp */ ) {
    flow->packet.udp = (struct ndpi_udphdr *) l4ptr;
    flow->packet.payload_packet_len =flow->packet.l4_packet_len - 8;
    flow->packet.payload = ((u_int8_t *)flow->packet.udp) + 8;
  } else {
    flow->packet.generic_l4_ptr = l4ptr;
  }
  return 0;
}


#if !defined(WIN32)
static inline
#else
__forceinline static
#endif
void ndpi_connection_tracking(struct ndpi_detection_module_struct *ndpi_struct,
			      struct ndpi_flow_struct *flow)
{
  /* const for gcc code optimisation and cleaner code */
  struct ndpi_packet_struct *packet = &flow->packet;
  const struct ndpi_iphdr *iph = packet->iph;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  const struct ndpi_ipv6hdr *iphv6 = packet->iphv6;
#endif
  const struct ndpi_tcphdr *tcph = packet->tcp;
  //const struct ndpi_udphdr   *udph=flow->packet.udp;

  //struct ndpi_unique_flow_struct      unique_flow;
  //uint8_t                               new_connection;

  u_int8_t proxy_enabled = 0;

  packet->tcp_retransmission = 0;

  packet->packet_direction = 0;


  if (iph != NULL && iph->saddr < iph->daddr)
    packet->packet_direction = 1;

#ifdef NDPI_DETECTION_SUPPORT_IPV6
  if (iphv6 != NULL && NDPI_COMPARE_IPV6_ADDRESS_STRUCTS(&iphv6->saddr, &iphv6->daddr) != 0)
    packet->packet_direction = 1;
#endif

  packet->packet_lines_parsed_complete = 0;
  packet->packet_unix_lines_parsed_complete = 0;
  if (flow == NULL)
    return;

  if (flow->init_finished == 0) {
    flow->init_finished = 1;
    flow->setup_packet_direction = packet->packet_direction;
  }

  if (tcph != NULL) {
    /* reset retried bytes here before setting it */
    packet->num_retried_bytes = 0;

    if (tcph->syn != 0 && tcph->ack == 0 && flow->l4.tcp.seen_syn == 0 && flow->l4.tcp.seen_syn_ack == 0
	&& flow->l4.tcp.seen_ack == 0) {
      flow->l4.tcp.seen_syn = 1;
    }
    if (tcph->syn != 0 && tcph->ack != 0 && flow->l4.tcp.seen_syn == 1 && flow->l4.tcp.seen_syn_ack == 0
	&& flow->l4.tcp.seen_ack == 0) {
      flow->l4.tcp.seen_syn_ack = 1;
    }
    if (tcph->syn == 0 && tcph->ack == 1 && flow->l4.tcp.seen_syn == 1 && flow->l4.tcp.seen_syn_ack == 1
	&& flow->l4.tcp.seen_ack == 0) {
      flow->l4.tcp.seen_ack = 1;
    }
    if ((flow->next_tcp_seq_nr[0] == 0 && flow->next_tcp_seq_nr[1] == 0)
	|| (proxy_enabled && (flow->next_tcp_seq_nr[0] == 0 || flow->next_tcp_seq_nr[1] == 0))) {
      /* initalize tcp sequence counters */
      /* the ack flag needs to be set to get valid sequence numbers from the other
       * direction. Usually it will catch the second packet syn+ack but it works
       * also for asymmetric traffic where it will use the first data packet
       *
       * if the syn flag is set add one to the sequence number,
       * otherwise use the payload length.
       */
      if (tcph->ack != 0) {
	flow->next_tcp_seq_nr[flow->packet.packet_direction] =
	  ntohl(tcph->seq) + (tcph->syn ? 1 : packet->payload_packet_len);
	if (!proxy_enabled) {
	  flow->next_tcp_seq_nr[1 -flow->packet.packet_direction] = ntohl(tcph->ack_seq);
	}
      }
    } else if (packet->payload_packet_len > 0) {
      /* check tcp sequence counters */
      if (((u_int32_t)
	   (ntohl(tcph->seq) -
	    flow->next_tcp_seq_nr[packet->packet_direction])) >
	  ndpi_struct->tcp_max_retransmission_window_size) {

	packet->tcp_retransmission = 1;


	/*CHECK IF PARTIAL RETRY IS HAPPENENING */
	if ((flow->next_tcp_seq_nr[packet->packet_direction] - ntohl(tcph->seq) < packet->payload_packet_len)) {
	  /* num_retried_bytes actual_payload_len hold info about the partial retry
	     analyzer which require this info can make use of this info
	     Other analyzer can use packet->payload_packet_len */
	  packet->num_retried_bytes = (u_int16_t)(flow->next_tcp_seq_nr[packet->packet_direction] - ntohl(tcph->seq));
	  packet->actual_payload_len = packet->payload_packet_len - packet->num_retried_bytes;
	  flow->next_tcp_seq_nr[packet->packet_direction] = ntohl(tcph->seq) + packet->payload_packet_len;
	}
      }
      /*normal path
	actual_payload_len is initialized to payload_packet_len during tcp header parsing itself.
	It will be changed only in case of retransmission */
      else {


	packet->num_retried_bytes = 0;
	flow->next_tcp_seq_nr[packet->packet_direction] = ntohl(tcph->seq) + packet->payload_packet_len;
      }


    }

    if (tcph->rst) {
      flow->next_tcp_seq_nr[0] = 0;
      flow->next_tcp_seq_nr[1] = 0;
    }
  }

  if (flow->packet_counter < MAX_PACKET_COUNTER && packet->payload_packet_len) {
    flow->packet_counter++;
  }

  if (flow->packet_direction_counter[packet->packet_direction] < MAX_PACKET_COUNTER && packet->payload_packet_len) {
    flow->packet_direction_counter[packet->packet_direction]++;
  }

  if (flow->byte_counter[packet->packet_direction] + packet->payload_packet_len >
      flow->byte_counter[packet->packet_direction]) {
    flow->byte_counter[packet->packet_direction] += packet->payload_packet_len;
  }
}

unsigned int ndpi_detection_process_packet(struct ndpi_detection_module_struct *ndpi_struct,
					   struct ndpi_flow_struct *flow,
					   const unsigned char *packet,
					   const unsigned short packetlen,
					   const u_int32_t current_tick, 
					   struct ndpi_id_struct *src,
					   struct ndpi_id_struct *dst)
{
  u_int32_t a;
  NDPI_SELECTION_BITMASK_PROTOCOL_SIZE ndpi_selection_packet;
  NDPI_PROTOCOL_BITMASK detection_bitmask;

  /* need at least 20 bytes for ip header */
  if (packetlen < 20) {
    /* reset protocol which is normally done in init_packet_header */
    ndpi_int_reset_packet_protocol(&flow->packet);

    return NDPI_PROTOCOL_UNKNOWN;
  }
  flow->packet.tick_timestamp = current_tick;

  /* parse packet */
  flow->packet.iph = (struct ndpi_iphdr *) packet;
  /* we are interested in ipv4 packet */

  if (ndpi_init_packet_header(ndpi_struct, flow, packetlen) != 0) {
    return NDPI_PROTOCOL_UNKNOWN;
  }
  /* detect traffic for tcp or udp only */

  flow->src = src, flow->dst = dst;

  ndpi_connection_tracking(ndpi_struct, flow);

  if (flow == NULL && (flow->packet.tcp != NULL || flow->packet.udp != NULL)) {
    return (NDPI_PROTOCOL_UNKNOWN);
  }

  /* build ndpi_selction packet bitmask */
  ndpi_selection_packet = NDPI_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC;
  if (flow->packet.iph != NULL) {
    ndpi_selection_packet |= NDPI_SELECTION_BITMASK_PROTOCOL_IP | NDPI_SELECTION_BITMASK_PROTOCOL_IPV4_OR_IPV6;

  }
  if (flow->packet.tcp != NULL) {
    ndpi_selection_packet |=
      (NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP | NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP);

  }
  if (flow->packet.udp != NULL) {
    ndpi_selection_packet |=
      (NDPI_SELECTION_BITMASK_PROTOCOL_INT_UDP | NDPI_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP);
  }
  if (flow->packet.payload_packet_len != 0) {
    ndpi_selection_packet |= NDPI_SELECTION_BITMASK_PROTOCOL_HAS_PAYLOAD;
  }

  if (flow->packet.tcp_retransmission == 0) {
    ndpi_selection_packet |= NDPI_SELECTION_BITMASK_PROTOCOL_NO_TCP_RETRANSMISSION;

  }
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  if (flow->packet.iphv6 != NULL) {
    ndpi_selection_packet |= NDPI_SELECTION_BITMASK_PROTOCOL_IPV6 | NDPI_SELECTION_BITMASK_PROTOCOL_IPV4_OR_IPV6;

  }
#endif							/* NDPI_DETECTION_SUPPORT_IPV6 */


  NDPI_SAVE_AS_BITMASK(detection_bitmask, flow->packet.detected_protocol_stack[0]);

  if (flow != NULL && flow->packet.tcp != NULL) {
    if (flow->packet.payload_packet_len != 0) {
      for (a = 0; a < ndpi_struct->callback_buffer_size_tcp_payload; a++) {
	if ((ndpi_struct->callback_buffer_tcp_payload[a].ndpi_selection_bitmask & ndpi_selection_packet) ==
	    ndpi_struct->callback_buffer_tcp_payload[a].ndpi_selection_bitmask
	    && NDPI_BITMASK_COMPARE(flow->excluded_protocol_bitmask,
				    ndpi_struct->
				    callback_buffer_tcp_payload[a].excluded_protocol_bitmask) == 0
	    && NDPI_BITMASK_COMPARE(ndpi_struct->callback_buffer_tcp_payload[a].detection_bitmask,
				    detection_bitmask) != 0) {
	  ndpi_struct->callback_buffer_tcp_payload[a].func(ndpi_struct, flow);

	  if(flow->detected_protocol_stack[0] != NDPI_PROTOCOL_UNKNOWN)
	    break; /* Stop after detecting the first protocol */
	}
      }
    } else {				/* no payload */

      for (a = 0; a < ndpi_struct->callback_buffer_size_tcp_no_payload; a++) {
	if ((ndpi_struct->callback_buffer_tcp_no_payload[a].ndpi_selection_bitmask & ndpi_selection_packet) ==
	    ndpi_struct->callback_buffer_tcp_no_payload[a].ndpi_selection_bitmask
	    && NDPI_BITMASK_COMPARE(flow->excluded_protocol_bitmask,
				    ndpi_struct->
				    callback_buffer_tcp_no_payload[a].excluded_protocol_bitmask) == 0
	    && NDPI_BITMASK_COMPARE(ndpi_struct->callback_buffer_tcp_no_payload[a].detection_bitmask,
				    detection_bitmask) != 0) {
	  ndpi_struct->callback_buffer_tcp_no_payload[a].func(ndpi_struct, flow);

	  if(flow->detected_protocol_stack[0] != NDPI_PROTOCOL_UNKNOWN)
	    break; /* Stop after detecting the first protocol */
	}
      }
    }
  } else if (flow != NULL && flow->packet.udp != NULL) {
    for (a = 0; a < ndpi_struct->callback_buffer_size_udp; a++) {
      if ((ndpi_struct->callback_buffer_udp[a].ndpi_selection_bitmask & ndpi_selection_packet) ==
	  ndpi_struct->callback_buffer_udp[a].ndpi_selection_bitmask
	  && NDPI_BITMASK_COMPARE(flow->excluded_protocol_bitmask,
				  ndpi_struct->callback_buffer_udp[a].excluded_protocol_bitmask) == 0
	  && NDPI_BITMASK_COMPARE(ndpi_struct->callback_buffer_udp[a].detection_bitmask,
				  detection_bitmask) != 0) {
	ndpi_struct->callback_buffer_udp[a].func(ndpi_struct, flow);

	if(flow->detected_protocol_stack[0] != NDPI_PROTOCOL_UNKNOWN)
	  break; /* Stop after detecting the first protocol */
      }
    }
  } else {

    for (a = 0; a < ndpi_struct->callback_buffer_size_non_tcp_udp; a++) {
      if ((ndpi_struct->callback_buffer_non_tcp_udp[a].ndpi_selection_bitmask & ndpi_selection_packet) ==
	  ndpi_struct->callback_buffer_non_tcp_udp[a].ndpi_selection_bitmask
	  && (flow == NULL
	      ||
	      NDPI_BITMASK_COMPARE
	      (flow->excluded_protocol_bitmask,
	       ndpi_struct->callback_buffer_non_tcp_udp[a].excluded_protocol_bitmask) == 0)
	  && NDPI_BITMASK_COMPARE(ndpi_struct->callback_buffer_non_tcp_udp[a].detection_bitmask,
				  detection_bitmask) != 0) {

	ndpi_struct->callback_buffer_non_tcp_udp[a].func(ndpi_struct, flow);

	if(flow->detected_protocol_stack[0] != NDPI_PROTOCOL_UNKNOWN)
	  break; /* Stop after detecting the first protocol */
      }
    }
  }

  a = flow->packet.detected_protocol_stack[0];
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, a)
      == 0)
    a = NDPI_PROTOCOL_UNKNOWN;

  return a;
}

static u_int8_t ndpi_detection_build_key_internal(struct ndpi_detection_module_struct *ndpi_struct,
						  const u_int8_t * l3, u_int16_t l3_len, const u_int8_t * l4, u_int16_t l4_len, u_int8_t l4_protocol,
						  struct ndpi_unique_flow_ipv4_and_6_struct *key_return, u_int8_t * dir_return,
						  u_int32_t flags)
{
  const struct ndpi_iphdr *iph = NULL;
  u_int8_t swapped = 0;

  if (key_return == NULL || l3 == NULL)
    return 1;

  if (l3_len < sizeof(*iph))
    return 1;

  iph = (const struct ndpi_iphdr *) l3;

  if (iph->version == 4 && ((iph->ihl * 4) > l3_len || l3_len < ntohs(iph->tot_len)
			    || (iph->frag_off & htons(0x1FFF)) != 0)) {
    return 1;
  }

  if ((flags & NDPI_DETECTION_ONLY_IPV6) && iph->version == 4) {
    return 1;
  }
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  else if ((flags & NDPI_DETECTION_ONLY_IPV4) && iph->version == 6) {
    return 1;
  }
#endif

  //memset( key_return, 0, sizeof( *key_return ) );

  /* needed:
   *  - unfragmented or first part of the fragmented packet
   *  - ip header <= packet len
   *  - ip total length >= packet len
   */

  if (iph->version == 4 && iph->ihl >= 5) {
    NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG, "ipv4 header\n");

    key_return->is_ip_v6 = 0;
    key_return->protocol = l4_protocol;

    if (iph->saddr < iph->daddr) {
      key_return->ip.ipv4.lower_ip = iph->saddr;
      key_return->ip.ipv4.upper_ip = iph->daddr;
    } else {
      key_return->ip.ipv4.upper_ip = iph->saddr;
      key_return->ip.ipv4.lower_ip = iph->daddr;
      swapped = 1;
    }

    key_return->ip.ipv4.dummy[0] = 0;
    key_return->ip.ipv4.dummy[1] = 0;
    key_return->ip.ipv4.dummy[2] = 0;


#ifdef NDPI_DETECTION_SUPPORT_IPV6
  } else if (iph->version == 6 && l3_len >= sizeof(struct ndpi_ipv6hdr)) {
    const struct ndpi_ipv6hdr *ip6h = (const struct ndpi_ipv6hdr *) iph;

    if ((l3_len - sizeof(struct ndpi_ipv6hdr)) < ntohs(ip6h->payload_len)) {
      return 3;
    }

    key_return->is_ip_v6 = 1;
    key_return->protocol = l4_protocol;

    if (NDPI_COMPARE_IPV6_ADDRESS_STRUCTS(&ip6h->saddr, &ip6h->daddr)) {
      key_return->ip.ipv6.lower_ip[0] = ((u_int64_t *) & ip6h->saddr)[0];
      key_return->ip.ipv6.lower_ip[1] = ((u_int64_t *) & ip6h->saddr)[1];
      key_return->ip.ipv6.upper_ip[0] = ((u_int64_t *) & ip6h->daddr)[0];
      key_return->ip.ipv6.upper_ip[1] = ((u_int64_t *) & ip6h->daddr)[1];
    } else {
      key_return->ip.ipv6.lower_ip[0] = ((u_int64_t *) & ip6h->daddr)[0];
      key_return->ip.ipv6.lower_ip[1] = ((u_int64_t *) & ip6h->daddr)[1];
      key_return->ip.ipv6.upper_ip[0] = ((u_int64_t *) & ip6h->saddr)[0];
      key_return->ip.ipv6.upper_ip[1] = ((u_int64_t *) & ip6h->saddr)[1];
      swapped = 1;
    }
#endif
  } else {
    return 5;
  }

  /* tcp / udp detection */
  if (key_return->protocol == 6 /* TCP */  && l4_len >= sizeof(struct ndpi_tcphdr)) {
    const struct ndpi_tcphdr *tcph = (const struct ndpi_tcphdr *) l4;
    if (swapped == 0) {
      key_return->lower_port = tcph->source;
      key_return->upper_port = tcph->dest;
    } else {
      key_return->lower_port = tcph->dest;
      key_return->upper_port = tcph->source;
    }
  } else if (key_return->protocol == 17 /* UDP */  && l4_len >= sizeof(struct ndpi_udphdr)) {
    const struct ndpi_udphdr *udph = (struct ndpi_udphdr *) l4;
    if (swapped == 0) {
      key_return->lower_port = udph->source;
      key_return->upper_port = udph->dest;
    } else {
      key_return->lower_port = udph->dest;
      key_return->upper_port = udph->source;
    }
  } else {
    /* non tcp/udp protocols, one connection between two ip addresses */
    key_return->lower_port = 0;
    key_return->upper_port = 0;
  }

  if (dir_return != NULL) {
    *dir_return = swapped;
  }

  return 0;
}

u_int32_t ndpi_bytestream_to_number(const u_int8_t * str, u_int16_t max_chars_to_read, u_int16_t * bytes_read)
{
  u_int32_t val;
  val = 0;
  // cancel if eof, ' ' or line end chars are reached
  while (*str >= '0' && *str <= '9' && max_chars_to_read > 0) {
    val *= 10;
    val += *str - '0';
    str++;
    max_chars_to_read = max_chars_to_read - 1;
    *bytes_read = *bytes_read + 1;
  }
  return (val);
}

u_int32_t ndpi_bytestream_dec_or_hex_to_number(const u_int8_t * str, u_int16_t max_chars_to_read, u_int16_t * bytes_read)
{
  u_int32_t val;
  val = 0;
  if (max_chars_to_read <= 2 || str[0] != '0' || str[1] != 'x') {
    return ndpi_bytestream_to_number(str, max_chars_to_read, bytes_read);
  } else {
    /*use base 16 system */
    str += 2;
    max_chars_to_read -= 2;
    *bytes_read = *bytes_read + 2;
    while (max_chars_to_read > 0) {

      if (*str >= '0' && *str <= '9') {
	val *= 16;
	val += *str - '0';
      } else if (*str >= 'a' && *str <= 'f') {
	val *= 16;
	val += *str + 10 - 'a';
      } else if (*str >= 'A' && *str <= 'F') {
	val *= 16;
	val += *str + 10 - 'A';
      } else {
	break;
      }
      str++;
      max_chars_to_read = max_chars_to_read - 1;
      *bytes_read = *bytes_read + 1;
    }
  }
  return (val);
}


u_int64_t ndpi_bytestream_to_number64(const u_int8_t * str, u_int16_t max_chars_to_read, u_int16_t * bytes_read)
{
  u_int64_t val;
  val = 0;
  // cancel if eof, ' ' or line end chars are reached
  while (max_chars_to_read > 0 && *str >= '0' && *str <= '9') {
    val *= 10;
    val += *str - '0';
    str++;
    max_chars_to_read = max_chars_to_read - 1;
    *bytes_read = *bytes_read + 1;
  }
  return (val);
}

u_int64_t ndpi_bytestream_dec_or_hex_to_number64(const u_int8_t * str, u_int16_t max_chars_to_read, u_int16_t * bytes_read)
{
  u_int64_t val;
  val = 0;
  if (max_chars_to_read <= 2 || str[0] != '0' || str[1] != 'x') {
    return ndpi_bytestream_to_number64(str, max_chars_to_read, bytes_read);
  } else {
    /*use base 16 system */
    str += 2;
    max_chars_to_read -= 2;
    *bytes_read = *bytes_read + 2;
    while (max_chars_to_read > 0) {

      if (*str >= '0' && *str <= '9') {
	val *= 16;
	val += *str - '0';
      } else if (*str >= 'a' && *str <= 'f') {
	val *= 16;
	val += *str + 10 - 'a';
      } else if (*str >= 'A' && *str <= 'F') {
	val *= 16;
	val += *str + 10 - 'A';
      } else {
	break;
      }
      str++;
      max_chars_to_read = max_chars_to_read - 1;
      *bytes_read = *bytes_read + 1;
    }
  }
  return (val);
}


u_int32_t ndpi_bytestream_to_ipv4(const u_int8_t * str, u_int16_t max_chars_to_read, u_int16_t * bytes_read)
{
  u_int32_t val;
  u_int16_t read = 0;
  u_int16_t oldread;
  u_int32_t c;
  /* ip address must be X.X.X.X with each X between 0 and 255 */
  oldread = read;
  c = ndpi_bytestream_to_number(str, max_chars_to_read, &read);
  if (c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
    return 0;
  read++;
  val = c << 24;
  oldread = read;
  c = ndpi_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
  if (c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
    return 0;
  read++;
  val = val + (c << 16);
  oldread = read;
  c = ndpi_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
  if (c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
    return 0;
  read++;
  val = val + (c << 8);
  oldread = read;
  c = ndpi_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
  if (c > 255 || oldread == read || max_chars_to_read == read)
    return 0;
  val = val + c;

  *bytes_read = *bytes_read + read;

  return htonl(val);
}

/* internal function for every detection to parse one packet and to increase the info buffer */
void ndpi_parse_packet_line_info(struct ndpi_detection_module_struct *ndpi_struct,
				 struct ndpi_flow_struct *flow)
{  
  u_int32_t a;
  struct ndpi_packet_struct *packet = &flow->packet;
  u_int16_t end = packet->payload_packet_len - 1;
  if (packet->packet_lines_parsed_complete != 0)
    return;

  packet->packet_lines_parsed_complete = 1;
  packet->parsed_lines = 0;

  packet->empty_line_position_set = 0;

  packet->host_line.ptr = NULL;
  packet->host_line.len = 0;
  packet->referer_line.ptr = NULL;
  packet->referer_line.len = 0;
  packet->content_line.ptr = NULL;
  packet->content_line.len = 0;
  packet->accept_line.ptr = NULL;
  packet->accept_line.len = 0;
  packet->user_agent_line.ptr = NULL;
  packet->user_agent_line.len = 0;
  packet->http_url_name.ptr = NULL;
  packet->http_url_name.len = 0;
  packet->http_encoding.ptr = NULL;
  packet->http_encoding.len = 0;
  packet->http_transfer_encoding.ptr = NULL;
  packet->http_transfer_encoding.len = 0;
  packet->http_contentlen.ptr = NULL;
  packet->http_contentlen.len = 0;
  packet->http_cookie.ptr = NULL;
  packet->http_cookie.len = 0;
  packet->http_x_session_type.ptr = NULL;
  packet->http_x_session_type.len = 0;
  packet->server_line.ptr = NULL;
  packet->server_line.len = 0;
  packet->http_method.ptr = NULL;
  packet->http_method.len = 0;
  packet->http_response.ptr = NULL;
  packet->http_response.len = 0;

  if (packet->payload_packet_len == 0)
    return;

  packet->line[packet->parsed_lines].ptr = packet->payload;
  packet->line[packet->parsed_lines].len = 0;



  for (a = 0; a < end; a++) {
    if (get_u_int16_t(packet->payload, a) == ntohs(0x0d0a)) {
      packet->line[packet->parsed_lines].len = (u_int16_t)(((unsigned long) &packet->payload[a]) - ((unsigned long) packet->line[packet->parsed_lines].ptr));

      if (packet->parsed_lines == 0 && packet->line[0].len >= NDPI_STATICSTRING_LEN("HTTP/1.1 200 ") &&
	  memcmp(packet->line[0].ptr, "HTTP/1.", NDPI_STATICSTRING_LEN("HTTP/1.")) == 0 &&
	  packet->line[0].ptr[NDPI_STATICSTRING_LEN("HTTP/1.1 ")] > '0' &&
	  packet->line[0].ptr[NDPI_STATICSTRING_LEN("HTTP/1.1 ")] < '6') {
	packet->http_response.ptr = &packet->line[0].ptr[NDPI_STATICSTRING_LEN("HTTP/1.1 ")];
	packet->http_response.len = packet->line[0].len - NDPI_STATICSTRING_LEN("HTTP/1.1 ");
	NDPI_LOG(NDPI_PROTOCOL_UNKNOWN, ndpi_struct, NDPI_LOG_DEBUG,
		 "ndpi_parse_packet_line_info: HTTP response parsed: \"%.*s\"\n",
		 packet->http_response.len, packet->http_response.ptr);
      }
      if (packet->line[packet->parsed_lines].len > NDPI_STATICSTRING_LEN("Server:") + 1
	  && memcmp(packet->line[packet->parsed_lines].ptr, "Server:", NDPI_STATICSTRING_LEN("Server:")) == 0) {
	// some stupid clients omit a space and place the servername directly after the colon
	if (packet->line[packet->parsed_lines].ptr[NDPI_STATICSTRING_LEN("Server:")] == ' ') {
	  packet->server_line.ptr =
	    &packet->line[packet->parsed_lines].ptr[NDPI_STATICSTRING_LEN("Server:") + 1];
	  packet->server_line.len =
	    packet->line[packet->parsed_lines].len - (NDPI_STATICSTRING_LEN("Server:") + 1);
	} else {
	  packet->server_line.ptr = &packet->line[packet->parsed_lines].ptr[NDPI_STATICSTRING_LEN("Server:")];
	  packet->server_line.len = packet->line[packet->parsed_lines].len - NDPI_STATICSTRING_LEN("Server:");
	}
      }

      if (packet->line[packet->parsed_lines].len > 6
	  && memcmp(packet->line[packet->parsed_lines].ptr, "Host:", 5) == 0) {
	// some stupid clients omit a space and place the hostname directly after the colon
	if (packet->line[packet->parsed_lines].ptr[5] == ' ') {
	  packet->host_line.ptr = &packet->line[packet->parsed_lines].ptr[6];
	  packet->host_line.len = packet->line[packet->parsed_lines].len - 6;
	} else {
	  packet->host_line.ptr = &packet->line[packet->parsed_lines].ptr[5];
	  packet->host_line.len = packet->line[packet->parsed_lines].len - 5;
	}
      }

      if (packet->line[packet->parsed_lines].len > 14
	  &&
	  (memcmp
	   (packet->line[packet->parsed_lines].ptr, "Content-Type: ",
	    14) == 0 || memcmp(packet->line[packet->parsed_lines].ptr, "Content-type: ", 14) == 0)) {
	packet->content_line.ptr = &packet->line[packet->parsed_lines].ptr[14];
	packet->content_line.len = packet->line[packet->parsed_lines].len - 14;
      }

      if (packet->line[packet->parsed_lines].len > 13
	  && memcmp(packet->line[packet->parsed_lines].ptr, "content-type:", 13) == 0) {
	packet->content_line.ptr = &packet->line[packet->parsed_lines].ptr[13];
	packet->content_line.len = packet->line[packet->parsed_lines].len - 13;
      }

      if (packet->line[packet->parsed_lines].len > 8
	  && memcmp(packet->line[packet->parsed_lines].ptr, "Accept: ", 8) == 0) {
	packet->accept_line.ptr = &packet->line[packet->parsed_lines].ptr[8];
	packet->accept_line.len = packet->line[packet->parsed_lines].len - 8;
      }

      if (packet->line[packet->parsed_lines].len > 9
	  && memcmp(packet->line[packet->parsed_lines].ptr, "Referer: ", 9) == 0) {
	packet->referer_line.ptr = &packet->line[packet->parsed_lines].ptr[9];
	packet->referer_line.len = packet->line[packet->parsed_lines].len - 9;
      }

      if (packet->line[packet->parsed_lines].len > 12
	  && (memcmp(packet->line[packet->parsed_lines].ptr, "User-Agent: ", 12) == 0 ||
	      memcmp(packet->line[packet->parsed_lines].ptr, "User-agent: ", 12) == 0)) {
	packet->user_agent_line.ptr = &packet->line[packet->parsed_lines].ptr[12];
	packet->user_agent_line.len = packet->line[packet->parsed_lines].len - 12;
      }

      if (packet->line[packet->parsed_lines].len > 18
	  && memcmp(packet->line[packet->parsed_lines].ptr, "Content-Encoding: ", 18) == 0) {
	packet->http_encoding.ptr = &packet->line[packet->parsed_lines].ptr[18];
	packet->http_encoding.len = packet->line[packet->parsed_lines].len - 18;
      }

      if (packet->line[packet->parsed_lines].len > 19
	  && memcmp(packet->line[packet->parsed_lines].ptr, "Transfer-Encoding: ", 19) == 0) {
	packet->http_transfer_encoding.ptr = &packet->line[packet->parsed_lines].ptr[19];
	packet->http_transfer_encoding.len = packet->line[packet->parsed_lines].len - 19;
      }
      if (packet->line[packet->parsed_lines].len > 16
	  && ((memcmp(packet->line[packet->parsed_lines].ptr, "Content-Length: ", 16) == 0)
	      || (memcmp(packet->line[packet->parsed_lines].ptr, "content-length: ", 16) == 0))) {
	packet->http_contentlen.ptr = &packet->line[packet->parsed_lines].ptr[16];
	packet->http_contentlen.len = packet->line[packet->parsed_lines].len - 16;
      }
      if (packet->line[packet->parsed_lines].len > 8
	  && memcmp(packet->line[packet->parsed_lines].ptr, "Cookie: ", 8) == 0) {
	packet->http_cookie.ptr = &packet->line[packet->parsed_lines].ptr[8];
	packet->http_cookie.len = packet->line[packet->parsed_lines].len - 8;
      }
      if (packet->line[packet->parsed_lines].len > 16
	  && memcmp(packet->line[packet->parsed_lines].ptr, "X-Session-Type: ", 16) == 0) {
	packet->http_x_session_type.ptr = &packet->line[packet->parsed_lines].ptr[16];
	packet->http_x_session_type.len = packet->line[packet->parsed_lines].len - 16;
      }


      if (packet->line[packet->parsed_lines].len == 0) {
	packet->empty_line_position = a;
	packet->empty_line_position_set = 1;
      }

      if (packet->parsed_lines >= (NDPI_MAX_PARSE_LINES_PER_PACKET - 1)) {
	return;
      }

      packet->parsed_lines++;
      packet->line[packet->parsed_lines].ptr = &packet->payload[a + 2];
      packet->line[packet->parsed_lines].len = 0;

      if ((a + 2) >= packet->payload_packet_len) {

	return;
      }
      a++;
    }
  }

  if (packet->parsed_lines >= 1) {
    packet->line[packet->parsed_lines].len
      = (u_int16_t)(((unsigned long) &packet->payload[packet->payload_packet_len]) -
      ((unsigned long) packet->line[packet->parsed_lines].ptr));
    packet->parsed_lines++;
  }
}

void ndpi_parse_packet_line_info_unix(struct ndpi_detection_module_struct *ndpi_struct,
				      struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  u_int32_t a;
  u_int16_t end = packet->payload_packet_len;
  if (packet->packet_unix_lines_parsed_complete != 0)
    return;



  packet->packet_unix_lines_parsed_complete = 1;
  packet->parsed_unix_lines = 0;

  if (packet->payload_packet_len == 0)
    return;

  packet->unix_line[packet->parsed_unix_lines].ptr = packet->payload;
  packet->unix_line[packet->parsed_unix_lines].len = 0;

  for (a = 0; a < end; a++) {
    if (packet->payload[a] == 0x0a) {
      packet->unix_line[packet->parsed_unix_lines].len = (u_int16_t)(
	((unsigned long) &packet->payload[a]) -
	((unsigned long) packet->unix_line[packet->parsed_unix_lines].ptr));

      if (packet->parsed_unix_lines >= (NDPI_MAX_PARSE_LINES_PER_PACKET - 1)) {
	break;
      }

      packet->parsed_unix_lines++;
      packet->unix_line[packet->parsed_unix_lines].ptr = &packet->payload[a + 1];
      packet->unix_line[packet->parsed_unix_lines].len = 0;

      if ((a + 1) >= packet->payload_packet_len) {
	break;
      }
      //a++;
    }
  }
}



u_int16_t ndpi_check_for_email_address(struct ndpi_detection_module_struct *ndpi_struct, 
				       struct ndpi_flow_struct *flow, u_int16_t counter)
{

  struct ndpi_packet_struct *packet = &flow->packet;

  NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, "called ndpi_check_for_email_address\n");

  if (packet->payload_packet_len > counter && ((packet->payload[counter] >= 'a' && packet->payload[counter] <= 'z')
					       || (packet->payload[counter] >= 'A' && packet->payload[counter] <= 'Z')
					       || (packet->payload[counter] >= '0' && packet->payload[counter] <= '9')
					       || packet->payload[counter] == '-' || packet->payload[counter] == '_')) {
    NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, "first letter\n");
    counter++;
    while (packet->payload_packet_len > counter
	   && ((packet->payload[counter] >= 'a' && packet->payload[counter] <= 'z')
	       || (packet->payload[counter] >= 'A' && packet->payload[counter] <= 'Z')
	       || (packet->payload[counter] >= '0' && packet->payload[counter] <= '9')
	       || packet->payload[counter] == '-' || packet->payload[counter] == '_'
	       || packet->payload[counter] == '.')) {
      NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, "further letter\n");
      counter++;
      if (packet->payload_packet_len > counter && packet->payload[counter] == '@') {
	NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, "@\n");
	counter++;
	while (packet->payload_packet_len > counter
	       && ((packet->payload[counter] >= 'a' && packet->payload[counter] <= 'z')
		   || (packet->payload[counter] >= 'A' && packet->payload[counter] <= 'Z')
		   || (packet->payload[counter] >= '0' && packet->payload[counter] <= '9')
		   || packet->payload[counter] == '-' || packet->payload[counter] == '_')) {
	  NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, "letter\n");
	  counter++;
	  if (packet->payload_packet_len > counter && packet->payload[counter] == '.') {
	    NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, ".\n");
	    counter++;
	    if (packet->payload_packet_len > counter + 1
		&& ((packet->payload[counter] >= 'a' && packet->payload[counter] <= 'z')
		    && (packet->payload[counter + 1] >= 'a' && packet->payload[counter + 1] <= 'z'))) {
	      NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, "two letters\n");
	      counter += 2;
	      if (packet->payload_packet_len > counter
		  && (packet->payload[counter] == ' ' || packet->payload[counter] == ';')) {
		NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, "whitespace1\n");
		return counter;
	      } else if (packet->payload_packet_len > counter && packet->payload[counter] >= 'a'
			 && packet->payload[counter] <= 'z') {
		NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, "one letter\n");
		counter++;
		if (packet->payload_packet_len > counter
		    && (packet->payload[counter] == ' ' || packet->payload[counter] == ';')) {
		  NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, "whitespace2\n");
		  return counter;
		} else if (packet->payload_packet_len > counter && packet->payload[counter] >= 'a'
			   && packet->payload[counter] <= 'z') {
		  counter++;
		  if (packet->payload_packet_len > counter
		      && (packet->payload[counter] == ' ' || packet->payload[counter] == ';')) {
		    NDPI_LOG(NDPI_PROTOCOL_MSN, ndpi_struct, NDPI_LOG_DEBUG, "whitespace3\n");
		    return counter;
		  } else {
		    return 0;
		  }
		} else {
		  return 0;
		}
	      } else {
		return 0;
	      }
	    } else {
	      return 0;
	    }
	  }
	}
	return 0;
      }
    }
  }
  return 0;
}

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
void ndpi_debug_get_last_log_function_line(struct ndpi_detection_module_struct
					   *ndpi_struct, const char **file, const char **func, u_int32_t * line)
{
  *file = "";
  *func = "";

  if (ndpi_struct->ndpi_debug_print_file != NULL)
    *file = ndpi_struct->ndpi_debug_print_file;

  if (ndpi_struct->ndpi_debug_print_function != NULL)
    *func = ndpi_struct->ndpi_debug_print_function;

  *line = ndpi_struct->ndpi_debug_print_line;
}
#endif
u_int8_t ndpi_detection_get_l4(const u_int8_t * l3, u_int16_t l3_len, const u_int8_t ** l4_return, u_int16_t * l4_len_return,
			       u_int8_t * l4_protocol_return, u_int32_t flags)
{
  return ndpi_detection_get_l4_internal(NULL, l3, l3_len, l4_return, l4_len_return, l4_protocol_return, flags);
}

u_int8_t ndpi_detection_build_key(const u_int8_t * l3, u_int16_t l3_len, const u_int8_t * l4, u_int16_t l4_len, u_int8_t l4_protocol,
				  struct ndpi_unique_flow_ipv4_and_6_struct * key_return, u_int8_t * dir_return, u_int32_t flags)
{
  return ndpi_detection_build_key_internal(NULL, l3, l3_len, l4, l4_len, l4_protocol, key_return, dir_return,
					   flags);
}

void ndpi_int_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
			     struct ndpi_flow_struct *flow,
			     u_int16_t detected_protocol, ndpi_protocol_type_t protocol_type)
{
  struct ndpi_id_struct *src = flow->src;
  struct ndpi_id_struct *dst = flow->dst;

  ndpi_int_change_protocol(ndpi_struct, flow, detected_protocol, protocol_type);

  if (src != NULL) {
    NDPI_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, detected_protocol);
  }
  if (dst != NULL) {
    NDPI_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, detected_protocol);
  }
}

void ndpi_int_change_flow_protocol(struct ndpi_detection_module_struct *ndpi_struct,
				   struct ndpi_flow_struct *flow,
				   u_int16_t detected_protocol, ndpi_protocol_type_t protocol_type)
{
#if NDPI_PROTOCOL_HISTORY_SIZE > 1
  u_int8_t a;
  u_int8_t stack_size;
  u_int8_t new_is_real = 0;
  u_int16_t preserve_bitmask;
#endif

  if (!flow)
    return;

#if NDPI_PROTOCOL_HISTORY_SIZE > 1
  stack_size = flow->protocol_stack_info.current_stack_size_minus_one + 1;

  /* here are the rules for stack manipulations:
   * 1.if the new protocol is a real protocol, insert it at the position
   *   of the top-most real protocol or below the last non-unknown correlated
   *   protocol.
   * 2.if the new protocol is not real, put it on top of stack but if there is
   *   a real protocol in the stack, make sure at least one real protocol remains
   *   in the stack
   */

  if (protocol_type == NDPI_CORRELATED_PROTOCOL) {
    u_int16_t saved_real_protocol = NDPI_PROTOCOL_UNKNOWN;

    if (stack_size == NDPI_PROTOCOL_HISTORY_SIZE) {
      /* check whether we will lost real protocol information due to shifting */
      u_int16_t real_protocol = flow->protocol_stack_info.entry_is_real_protocol;

      for (a = 0; a < stack_size; a++) {
	if (real_protocol & 1)
	  break;
	real_protocol >>= 1;
      }

      if (a == (stack_size - 1)) {
	/* oh, only one real protocol at the end, store it and insert it later */
	saved_real_protocol = flow->detected_protocol_stack[stack_size - 1];
      }
    } else {
      flow->protocol_stack_info.current_stack_size_minus_one++;
      stack_size++;
    }

    /* now shift and insert */
    for (a = stack_size - 1; a > 0; a--) {
      flow->detected_protocol_stack[a] = flow->detected_protocol_stack[a - 1];
    }

    flow->protocol_stack_info.entry_is_real_protocol <<= 1;

    /* now set the new protocol */

    flow->detected_protocol_stack[0] = detected_protocol;

    /* restore real protocol */
    if (saved_real_protocol != NDPI_PROTOCOL_UNKNOWN) {
      flow->detected_protocol_stack[stack_size - 1] = saved_real_protocol;
      flow->protocol_stack_info.entry_is_real_protocol |= 1 << (stack_size - 1);
    }
    /* done */
  } else {
    u_int8_t insert_at = 0;

    if (!(flow->protocol_stack_info.entry_is_real_protocol & 1)) {
      u_int16_t real_protocol = flow->protocol_stack_info.entry_is_real_protocol;

      for (a = 0; a < stack_size; a++) {
	if (real_protocol & 1)
	  break;
	real_protocol >>= 1;
      }

      insert_at = a;
    }

    if (insert_at >= stack_size) {
      /* no real protocol found, insert it at the bottom */

      insert_at = stack_size - 1;
    }

    if (stack_size < NDPI_PROTOCOL_HISTORY_SIZE) {
      flow->protocol_stack_info.current_stack_size_minus_one++;
      stack_size++;
    }

    /* first shift all stacks */
    for (a = stack_size - 1; a > insert_at; a--) {
      flow->detected_protocol_stack[a] = flow->detected_protocol_stack[a - 1];
    }

    preserve_bitmask = (1 << insert_at) - 1;

    new_is_real = (flow->protocol_stack_info.entry_is_real_protocol & (~preserve_bitmask)) << 1;
    new_is_real |= flow->protocol_stack_info.entry_is_real_protocol & preserve_bitmask;

    flow->protocol_stack_info.entry_is_real_protocol = new_is_real;

    /* now set the new protocol */

    flow->detected_protocol_stack[insert_at] = detected_protocol;

    /* and finally update the additional stack information */

    flow->protocol_stack_info.entry_is_real_protocol |= 1 << insert_at;
  }
#else
  flow->detected_protocol_stack[0] = detected_protocol;
  flow->detected_subprotocol_stack[0] = detected_subprotocol;
#endif
}

void ndpi_int_change_packet_protocol(struct ndpi_detection_module_struct *ndpi_struct,
				     struct ndpi_flow_struct *flow,
				     u_int16_t detected_protocol, ndpi_protocol_type_t protocol_type)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  /* NOTE: everything below is identically to change_flow_protocol
   *        except flow->packet If you want to change something here,
   *        don't! Change it for the flow function and apply it here
   *        as well */
#if NDPI_PROTOCOL_HISTORY_SIZE > 1
  u_int8_t a;
  u_int8_t stack_size;
  u_int16_t new_is_real = 0;
  u_int16_t preserve_bitmask;
#endif

  if (!packet)
    return;

#if NDPI_PROTOCOL_HISTORY_SIZE > 1
  stack_size = packet->protocol_stack_info.current_stack_size_minus_one + 1;

  /* here are the rules for stack manipulations:
   * 1.if the new protocol is a real protocol, insert it at the position
   *   of the top-most real protocol or below the last non-unknown correlated
   *   protocol.
   * 2.if the new protocol is not real, put it on top of stack but if there is
   *   a real protocol in the stack, make sure at least one real protocol remains
   *   in the stack
   */

  if (protocol_type == NDPI_CORRELATED_PROTOCOL) {
    u_int16_t saved_real_protocol = NDPI_PROTOCOL_UNKNOWN;

    if (stack_size == NDPI_PROTOCOL_HISTORY_SIZE) {
      /* check whether we will lost real protocol information due to shifting */
      u_int16_t real_protocol = packet->protocol_stack_info.entry_is_real_protocol;

      for (a = 0; a < stack_size; a++) {
	if (real_protocol & 1)
	  break;
	real_protocol >>= 1;
      }

      if (a == (stack_size - 1)) {
	/* oh, only one real protocol at the end, store it and insert it later */
	saved_real_protocol = packet->detected_protocol_stack[stack_size - 1];
      }
    } else {
      packet->protocol_stack_info.current_stack_size_minus_one++;
      stack_size++;
    }

    /* now shift and insert */
    for (a = stack_size - 1; a > 0; a--) {
      packet->detected_protocol_stack[a] = packet->detected_protocol_stack[a - 1];
    }

    packet->protocol_stack_info.entry_is_real_protocol <<= 1;

    /* now set the new protocol */

    packet->detected_protocol_stack[0] = detected_protocol;

    /* restore real protocol */
    if (saved_real_protocol != NDPI_PROTOCOL_UNKNOWN) {
      packet->detected_protocol_stack[stack_size - 1] = saved_real_protocol;
      packet->protocol_stack_info.entry_is_real_protocol |= 1 << (stack_size - 1);
    }
    /* done */
  } else {
    u_int8_t insert_at = 0;

    if (!(packet->protocol_stack_info.entry_is_real_protocol & 1)) {
      u_int16_t real_protocol = packet->protocol_stack_info.entry_is_real_protocol;

      for (a = 0; a < stack_size; a++) {
	if (real_protocol & 1)
	  break;
	real_protocol >>= 1;
      }

      insert_at = a;
    }

    if (insert_at >= stack_size) {
      /* no real protocol found, insert it at the first unknown protocol */

      insert_at = stack_size - 1;
    }

    if (stack_size < NDPI_PROTOCOL_HISTORY_SIZE) {
      packet->protocol_stack_info.current_stack_size_minus_one++;
      stack_size++;
    }

    /* first shift all stacks */
    for (a = stack_size - 1; a > insert_at; a--) {
      packet->detected_protocol_stack[a] = packet->detected_protocol_stack[a - 1];
    }

    preserve_bitmask = (1 << insert_at) - 1;

    new_is_real = (packet->protocol_stack_info.entry_is_real_protocol & (~preserve_bitmask)) << 1;
    new_is_real |= packet->protocol_stack_info.entry_is_real_protocol & preserve_bitmask;

    packet->protocol_stack_info.entry_is_real_protocol = new_is_real;

    /* now set the new protocol */

    packet->detected_protocol_stack[insert_at] = detected_protocol;

    /* and finally update the additional stack information */

    packet->protocol_stack_info.entry_is_real_protocol |= 1 << insert_at;
  }
#else
  packet->detected_protocol_stack[0] = detected_protocol;
  packet->detected_subprotocol_stack[0] = detected_subprotocol;
#endif
}


/*
 * this function returns the real protocol of the flow. Actually it
 * accesses the packet stack since this is what leaves the library but
 * it could also use the flow stack.
 */
u_int16_t ndpi_detection_get_real_protocol_of_flow(struct ndpi_detection_module_struct * ndpi_struct,
						   struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
#if NDPI_PROTOCOL_HISTORY_SIZE > 1
  u_int8_t a;
  u_int8_t stack_size;
  u_int16_t real_protocol;
#endif

  if (!packet)
    return NDPI_PROTOCOL_UNKNOWN;

#if NDPI_PROTOCOL_HISTORY_SIZE > 1
  stack_size = packet->protocol_stack_info.current_stack_size_minus_one + 1;
  real_protocol = packet->protocol_stack_info.entry_is_real_protocol;

  for (a = 0; a < stack_size; a++) {
    if (real_protocol & 1)
      return packet->detected_protocol_stack[a];
    real_protocol >>= 1;
  }

  return NDPI_PROTOCOL_UNKNOWN;
#else
  return packet->detected_protocol_stack[0];
#endif
}

/*
 * this function checks whether a protocol can be found in the
 * history. Actually it accesses the packet stack since this is what
 * leaves the library but it could also use the flow stack.
 */
u_int8_t ndpi_detection_flow_protocol_history_contains_protocol(struct ndpi_detection_module_struct * ndpi_struct,
								struct ndpi_flow_struct *flow,								
								u_int16_t protocol_id)
{
  u_int8_t a;
  u_int8_t stack_size;
  struct ndpi_packet_struct *packet = &flow->packet;

  if (!packet)
    return 0;

#if NDPI_PROTOCOL_HISTORY_SIZE > 1
  stack_size = packet->protocol_stack_info.current_stack_size_minus_one + 1;
#else
  stack_size = 1;
#endif

  for (a = 0; a < stack_size; a++) {
    if (packet->detected_protocol_stack[a] == protocol_id)
      return 1;
  }

  return 0;
}

/* generic function for setting a protocol for a flow
 *
 * what it does is:
 * 1.call ndpi_int_change_protocol
 * 2.set protocol in detected bitmask for src and dst
 */
void ndpi_int_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
			     struct ndpi_flow_struct *flow,
			     u_int16_t detected_protocol, ndpi_protocol_type_t protocol_type);

/* generic function for changing the flow protocol
 *
 * what it does is:
 * 1.update the flow protocol stack with the new protocol
 */
void ndpi_int_change_flow_protocol(struct ndpi_detection_module_struct *ndpi_struct,
				   struct ndpi_flow_struct *flow,
				   u_int16_t detected_protocol, ndpi_protocol_type_t protocol_type);

/* generic function for changing the packetprotocol
 *
 * what it does is:
 * 1.update the packet protocol stack with the new protocol
 */
void ndpi_int_change_packet_protocol(struct ndpi_detection_module_struct *ndpi_struct,
				     struct ndpi_flow_struct *flow,
				     u_int16_t detected_protocol, ndpi_protocol_type_t protocol_type);

/* generic function for changing the protocol
 *
 * what it does is:
 * 1.update the flow protocol stack with the new protocol
 * 2.update the packet protocol stack with the new protocol
 */
void ndpi_int_change_protocol(struct ndpi_detection_module_struct *ndpi_struct, 
			      struct ndpi_flow_struct *flow,
			      u_int16_t detected_protocol,
			      ndpi_protocol_type_t protocol_type)
{
  ndpi_int_change_flow_protocol(ndpi_struct, flow, detected_protocol, protocol_type);
  ndpi_int_change_packet_protocol(ndpi_struct, flow, detected_protocol, protocol_type);
}


/* turns a packet back to unknown */
void ndpi_int_reset_packet_protocol(struct ndpi_packet_struct *packet) {
  packet->detected_protocol_stack[0] = NDPI_PROTOCOL_UNKNOWN;
  
#if NDPI_PROTOCOL_HISTORY_SIZE > 1
  packet->protocol_stack_info.current_stack_size_minus_one = 0;
  packet->protocol_stack_info.entry_is_real_protocol = 0;
#endif
}

void ndpi_int_reset_protocol(struct ndpi_flow_struct *flow)
{
  if (flow) {
    flow->detected_protocol_stack[0] = NDPI_PROTOCOL_UNKNOWN;
	  
#if NDPI_PROTOCOL_HISTORY_SIZE > 1
    flow->protocol_stack_info.current_stack_size_minus_one = 0;
    flow->protocol_stack_info.entry_is_real_protocol = 0;
#endif
  }
}

void ndpi_ip_clear(ndpi_ip_addr_t * ip)
{
  memset(ip, 0, sizeof(ndpi_ip_addr_t));
}

/* NTOP */
int ndpi_ip_is_set(const ndpi_ip_addr_t * ip)
{
  return memcmp(ip, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", sizeof(ndpi_ip_addr_t)) != 0;
}

/* check if the source ip address in packet and ip are equal */
/* NTOP */
int ndpi_packet_src_ip_eql(const struct ndpi_packet_struct *packet, const ndpi_ip_addr_t * ip)
{
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  if (packet->iphv6 != NULL) {
    if (packet->iphv6->saddr.ndpi_v6_u.u6_addr64[0] == ip->ipv6.ndpi_v6_u.u6_addr64[0] &&
	packet->iphv6->saddr.ndpi_v6_u.u6_addr64[1] == ip->ipv6.ndpi_v6_u.u6_addr64[1]) {

      return 1;
    } else {
      return 0;
    }
  }
#endif
  if (packet->iph->saddr == ip->ipv4) {
    return 1;
  }
  return 0;
}

/* check if the destination ip address in packet and ip are equal */
int ndpi_packet_dst_ip_eql(const struct ndpi_packet_struct *packet, const ndpi_ip_addr_t * ip)
{
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  if (packet->iphv6 != NULL) {
    if (packet->iphv6->daddr.ndpi_v6_u.u6_addr64[0] == ip->ipv6.ndpi_v6_u.u6_addr64[0] &&
	packet->iphv6->daddr.ndpi_v6_u.u6_addr64[1] == ip->ipv6.ndpi_v6_u.u6_addr64[1]) {
      return 1;
    } else {
      return 0;
    }
  }
#endif
  if (packet->iph->daddr == ip->ipv4) {
    return 1;
  }
  return 0;
}

/* get the source ip address from packet and put it into ip */
/* NTOP */
void ndpi_packet_src_ip_get(const struct ndpi_packet_struct *packet, ndpi_ip_addr_t * ip)
{
  ndpi_ip_clear(ip);
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  if (packet->iphv6 != NULL) {
    ip->ipv6.ndpi_v6_u.u6_addr64[0] = packet->iphv6->saddr.ndpi_v6_u.u6_addr64[0];
    ip->ipv6.ndpi_v6_u.u6_addr64[1] = packet->iphv6->saddr.ndpi_v6_u.u6_addr64[1];
  } else
#endif
    ip->ipv4 = packet->iph->saddr;
}

/* get the destination ip address from packet and put it into ip */
/* NTOP */
void ndpi_packet_dst_ip_get(const struct ndpi_packet_struct *packet, ndpi_ip_addr_t * ip)
{
  ndpi_ip_clear(ip);
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  if (packet->iphv6 != NULL) {
    ip->ipv6.ndpi_v6_u.u6_addr64[0] = packet->iphv6->daddr.ndpi_v6_u.u6_addr64[0];
    ip->ipv6.ndpi_v6_u.u6_addr64[1] = packet->iphv6->daddr.ndpi_v6_u.u6_addr64[1];
  } else
#endif
    ip->ipv4 = packet->iph->daddr;
}

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
/* get the string representation of ip
 * returns a pointer to a static string
 * only valid until the next call of this function */
char *ndpi_get_ip_string(struct ndpi_detection_module_struct *ndpi_struct,
			 const ndpi_ip_addr_t * ip)
{
  const u_int8_t *a = (const u_int8_t *) &ip->ipv4;

#ifdef NDPI_DETECTION_SUPPORT_IPV6
  if (ip->ipv6.ndpi_v6_u.u6_addr32[1] != 0 || ip->ipv6.ndpi_v6_u.u6_addr64[1] != 0) {
    const u_int16_t *b = ip->ipv6.ndpi_v6_u.u6_addr16;
    snprintf(ndpi_struct->ip_string, NDPI_IP_STRING_SIZE, "%x:%x:%x:%x:%x:%x:%x:%x",
	     ntohs(b[0]), ntohs(b[1]), ntohs(b[2]), ntohs(b[3]),
	     ntohs(b[4]), ntohs(b[5]), ntohs(b[6]), ntohs(b[7]));
    return ndpi_struct->ip_string;
  }
#endif
  snprintf(ndpi_struct->ip_string, NDPI_IP_STRING_SIZE, "%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
  return ndpi_struct->ip_string;
  char *ndpi_get_packet_dst_ip_string(struct ndpi_detection_module_struct *ndpi_struct,
				      const struct ndpi_packet_struct *packet)}


/* get the string representation of the source ip address from packet */
char *ndpi_get_packet_src_ip_string(struct ndpi_detection_module_struct *ndpi_struct,
				    const struct ndpi_packet_struct *packet)
{
  ndpi_ip_addr_t ip;
  ndpi_packet_src_ip_get(packet, &ip);
  return ndpi_get_ip_string(ndpi_struct, &ip);
}

/* get the string representation of the destination ip address from packet */
char *ndpi_get_packet_dst_ip_string(struct ndpi_detection_module_struct *ndpi_struct,
				    const struct ndpi_packet_struct *packet)
{
  ndpi_ip_addr_t ip;
  ndpi_packet_dst_ip_get(packet, &ip);
  return ndpi_get_ip_string(ndpi_struct, &ip);
}
#endif							/* NDPI_ENABLE_DEBUG_MESSAGES */


u_int16_t ntohs_ndpi_bytestream_to_number(const u_int8_t * str, u_int16_t max_chars_to_read, u_int16_t * bytes_read)
{
  u_int16_t val = ndpi_bytestream_to_number(str, max_chars_to_read, bytes_read);
  return ntohs(val);
}


static u_int is_port(u_int16_t sport, u_int16_t dport, u_int16_t match_port) {
  return(((match_port == sport) || (match_port == dport)) ? 1 : 0);
}

/* ****************************************************** */

unsigned int ndpi_find_port_based_protocol(u_int8_t proto,
					   u_int32_t shost, u_int16_t sport,
					   u_int32_t dhost, u_int16_t dport) {
  /* Skyfile (host 193.252.234.246 or host 10.10.102.80) */
  if((shost == 0xC1FCEAF6) || (dhost == 0xC1FCEAF6)
     || (shost == 0x0A0A6650) || (dhost == 0x0A0A6650)) {
    if((sport == 4708) || (dport == 4708)) return(NDPI_PROTOCOL_SKYFILE_PREPAID);
    else if((sport == 4709) || (dport == 4709)) return(NDPI_PROTOCOL_SKYFILE_RUDICS);
    else if((sport == 4710) || (dport == 4710)) return(NDPI_PROTOCOL_SKYFILE_POSTPAID);
  }

  return(NDPI_PROTOCOL_UNKNOWN);
}

/* ****************************************************** */

unsigned int ndpi_guess_undetected_protocol(u_int8_t proto,
					    u_int32_t shost, u_int16_t sport,
					    u_int32_t dhost, u_int16_t dport) {
  //   printf("ndpi_guess_undetected_protocol (proto=%d, %d -> %d)\n", proto, sport, dport);

  if(proto == IPPROTO_UDP) {
    if(is_port(sport, dport, 67) || is_port(sport, dport, 68))          return(NDPI_PROTOCOL_DHCP);
    else if(is_port(sport, dport, 137) || is_port(sport, dport, 138))   return(NDPI_PROTOCOL_NETBIOS);
    else if(is_port(sport, dport, 161) || is_port(sport, dport, 162))   return(NDPI_PROTOCOL_SNMP);
    else if(is_port(sport, dport, 5353) || is_port(sport, dport, 5354)) return(NDPI_PROTOCOL_MDNS);
    else if(is_port(sport, dport, 53)) return(NDPI_PROTOCOL_DNS);
  } else if(proto == IPPROTO_TCP) {
    if(is_port(sport, dport, 443))      return(NDPI_PROTOCOL_SSL);
    else if(is_port(sport, dport, 22))  return(NDPI_PROTOCOL_SSH);
    else if(is_port(sport, dport, 23))  return(NDPI_PROTOCOL_TELNET);
    else if(is_port(sport, dport, 445)) return(NDPI_PROTOCOL_SMB);
    else if(is_port(sport, dport, 80))  return(NDPI_PROTOCOL_HTTP);
    else if(is_port(sport, dport, 3000))  return(NDPI_PROTOCOL_HTTP); /* ntop */
    else if(is_port(sport, dport, 3001))  return(NDPI_PROTOCOL_SSL);  /* ntop */
    else if(is_port(sport, dport, 8080) || is_port(sport, dport, 3128)) return(NDPI_PROTOCOL_HTTP_PROXY);
    else if(is_port(sport, dport, 389)) return(NDPI_PROTOCOL_LDAP);
    else if(is_port(sport, dport, 143) || is_port(sport, dport, 993)) return(NDPI_PROTOCOL_MAIL_IMAP);
    else if(is_port(sport, dport, 25)  || is_port(sport, dport, 465)) return(NDPI_PROTOCOL_MAIL_SMTP);
    else if(is_port(sport, dport, 135))  return(NDPI_PROTOCOL_DCERPC);
    else if(is_port(sport, dport, 1494) || is_port(sport, dport, 2598)) return(NDPI_PROTOCOL_CITRIX); /* http://support.citrix.com/article/CTX104147 */
    else if(is_port(sport, dport, 389))  return(NDPI_PROTOCOL_LDAP);
  }

  return(NDPI_PROTOCOL_UNKNOWN);
}

