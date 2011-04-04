/*
 * ipq_main.c
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


#include "ipq_main.h"
#include "ipq_protocols.h"
u32 ipoque_detection_get_sizeof_ipoque_flow_struct(void)
{
	return sizeof(struct ipoque_flow_struct);
}

u32 ipoque_detection_get_sizeof_ipoque_id_struct(void)
{
	return sizeof(struct ipoque_id_struct);
}


struct ipoque_detection_module_struct *ipoque_init_detection_module(u32 ticks_per_second, void
																	*(*ipoque_malloc)
																	 (unsigned
																	  long size),
																	ipoque_debug_function_ptr ipoque_debug_printf)
{
	struct ipoque_detection_module_struct *ipq_str;
	ipq_str = ipoque_malloc(sizeof(struct ipoque_detection_module_struct));

	if (ipq_str == NULL) {
		ipoque_debug_printf(0, NULL, IPQ_LOG_DEBUG, "ipoque_init_detection_module initial malloc failed\n");
		return NULL;
	}
	memset(ipq_str, 0, sizeof(struct ipoque_detection_module_struct));


	IPOQUE_BITMASK_RESET(ipq_str->detection_bitmask);
#ifdef IPOQUE_ENABLE_DEBUG_MESSAGES
	ipq_str->ipoque_debug_printf = ipoque_debug_printf;
	ipq_str->user_data = NULL;
#endif


	ipq_str->ticks_per_second = ticks_per_second;
	ipq_str->tcp_max_retransmission_window_size = IPOQUE_DEFAULT_MAX_TCP_RETRANSMISSION_WINDOW_SIZE;
	ipq_str->directconnect_connection_ip_tick_timeout =
		IPOQUE_DIRECTCONNECT_CONNECTION_IP_TICK_TIMEOUT * ticks_per_second;

	ipq_str->gadugadu_peer_connection_timeout = IPOQUE_GADGADU_PEER_CONNECTION_TIMEOUT * ticks_per_second;
	ipq_str->edonkey_upper_ports_only = IPOQUE_EDONKEY_UPPER_PORTS_ONLY;
	ipq_str->ftp_connection_timeout = IPOQUE_FTP_CONNECTION_TIMEOUT * ticks_per_second;

	ipq_str->imesh_connection_timeout = IPOQUE_IMESH_CONNECTION_TIMEOUT * ticks_per_second;
	ipq_str->pplive_connection_timeout = IPOQUE_PPLIVE_CONNECTION_TIMEOUT * ticks_per_second;

	ipq_str->rtsp_connection_timeout = IPOQUE_RTSP_CONNECTION_TIMEOUT * ticks_per_second;
	ipq_str->tvants_connection_timeout = IPOQUE_TVANTS_CONNECTION_TIMEOUT * ticks_per_second;
	ipq_str->irc_timeout = IPOQUE_IRC_CONNECTION_TIMEOUT * ticks_per_second;
	ipq_str->gnutella_timeout = IPOQUE_GNUTELLA_CONNECTION_TIMEOUT * ticks_per_second;

	ipq_str->battlefield_timeout = IPOQUE_BATTLEFIELD_CONNECTION_TIMEOUT * ticks_per_second;

	ipq_str->thunder_timeout = IPOQUE_THUNDER_CONNECTION_TIMEOUT * ticks_per_second;
	ipq_str->yahoo_detect_http_connections = IPOQUE_YAHOO_DETECT_HTTP_CONNECTIONS;

	ipq_str->yahoo_lan_video_timeout = IPOQUE_YAHOO_LAN_VIDEO_TIMEOUT * ticks_per_second;
	ipq_str->zattoo_connection_timeout = IPOQUE_ZATTOO_CONNECTION_TIMEOUT * ticks_per_second;
	ipq_str->jabber_stun_timeout = IPOQUE_JABBER_STUN_TIMEOUT * ticks_per_second;
	ipq_str->jabber_file_transfer_timeout = IPOQUE_JABBER_FT_TIMEOUT * ticks_per_second;
	ipq_str->soulseek_connection_ip_tick_timeout = IPOQUE_SOULSEEK_CONNECTION_IP_TICK_TIMEOUT * ticks_per_second;
	ipq_str->manolito_subscriber_timeout = IPOQUE_MANOLITO_SUBSCRIBER_TIMEOUT;
	return ipq_str;
}

void ipoque_exit_detection_module(struct ipoque_detection_module_struct
								  *ipoque_struct, void (*ipoque_free) (void *ptr))
{
	if (ipoque_struct != NULL) {
		ipoque_free(ipoque_struct);
	}
}

void ipoque_set_protocol_detection_bitmask2(struct ipoque_detection_module_struct
											*ipoque_struct, const IPOQUE_PROTOCOL_BITMASK * dbm)
{
	IPOQUE_PROTOCOL_BITMASK detection_bitmask_local;
	IPOQUE_PROTOCOL_BITMASK *detection_bitmask = &detection_bitmask_local;

	u32 a = 0;

	IPOQUE_BITMASK_SET(detection_bitmask_local, *dbm);
	IPOQUE_BITMASK_SET(ipoque_struct->detection_bitmask, *dbm);

	/* set this here to zero to be interrupt safe */
	ipoque_struct->callback_buffer_size = 0;

#ifdef IPOQUE_PROTOCOL_HTTP
#ifdef IPOQUE_PROTOCOL_MPEG
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MPEG) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_FLASH
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_FLASH) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_QUICKTIME
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_QUICKTIME) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_REALMEDIA
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_REALMEDIA) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_WINDOWSMEDIA
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_WINDOWSMEDIA) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_MMS
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MMS) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_OFF
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_OFF) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_XBOX
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_XBOX) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_QQ
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_QQ) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_AVI
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_AVI) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_OGG
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_OGG) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_MOVE
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MOVE) != 0)
		goto hack_do_http_detection;
#endif
#ifdef IPOQUE_PROTOCOL_RTSP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_RTSP) != 0)
		goto hack_do_http_detection;
#endif
	/* HTTP DETECTION MUST BE BEFORE DDL BUT AFTER ALL OTHER PROTOCOLS WHICH USE HTTP ALSO */
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_HTTP) != 0) {

	  hack_do_http_detection:

		ipoque_struct->callback_buffer[a].func = ipoque_search_http_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_HTTP);

#ifdef IPOQUE_PROTOCOL_MPEG
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_MPEG);
#endif
#ifdef IPOQUE_PROTOCOL_FLASH
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_FLASH);
#endif
#ifdef IPOQUE_PROTOCOL_QUICKTIME
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_QUICKTIME);
#endif
#ifdef IPOQUE_PROTOCOL_REALMEDIA
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_REALMEDIA);
#endif
#ifdef IPOQUE_PROTOCOL_WINDOWSMEDIA
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask,
									   IPOQUE_PROTOCOL_WINDOWSMEDIA);
#endif
#ifdef IPOQUE_PROTOCOL_MMS
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_MMS);
#endif
#ifdef IPOQUE_PROTOCOL_OFF
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_OFF);
#endif
#ifdef IPOQUE_PROTOCOL_XBOX
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_XBOX);
#endif
#ifdef IPOQUE_PROTOCOL_QQ
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_QQ);
#endif
#ifdef IPOQUE_PROTOCOL_AVI
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_AVI);
#endif
#ifdef IPOQUE_PROTOCOL_OGG
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_OGG);
#endif
#ifdef IPOQUE_PROTOCOL_MOVE
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_MOVE);
#endif
#ifdef IPOQUE_PROTOCOL_RTSP
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_RTSP);
#endif

		IPOQUE_BITMASK_SET(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
						   ipoque_struct->callback_buffer[a].detection_bitmask);
		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_QQ);

#ifdef IPOQUE_PROTOCOL_FLASH
		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_FLASH);
#endif

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_MMS);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_RTSP);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
										 IPOQUE_PROTOCOL_XBOX);

		IPOQUE_BITMASK_SET(ipoque_struct->generic_http_packet_bitmask,
						   ipoque_struct->callback_buffer[a].detection_bitmask);

		IPOQUE_DEL_PROTOCOL_FROM_BITMASK(ipoque_struct->generic_http_packet_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_SSL
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SSL) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_ssl_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SSL);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_STUN
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_STUN) != 0
#ifdef IPOQUE_PROTOCOL_RTP
		|| IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_RTP) != 0
#endif
		) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_stun_udp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_STUN);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_RTP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_RTP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_rtp_udp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

#ifdef IPOQUE_PROTOCOL_STUN
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_STUN);
#endif

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_RTP);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_SIP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SIP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_sip;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_SIP);
		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SIP);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_BITTORRENT
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_BITTORRENT) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_bittorrent;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_BITTORRENT);
		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_BITTORRENT);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_EDONKEY
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_EDONKEY) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_edonkey;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_EDONKEY);
#ifdef IPOQUE_PROTOCOL_BITTORRENT
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_BITTORRENT);
#endif
		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_EDONKEY);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_FASTTRACK
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_FASTTRACK) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_fasttrack_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_FASTTRACK);
		a++;

	}
#endif
#ifdef IPOQUE_PROTOCOL_GNUTELLA
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_GNUTELLA) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_gnutella;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

#ifdef IPOQUE_PROTOCOL_XBOX
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_XBOX);
#endif

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_GNUTELLA);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_WINMX
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_WINMX) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_winmx_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_WINMX);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_DIRECTCONNECT
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_DIRECTCONNECT) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_directconnect;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask,
									   IPOQUE_PROTOCOL_DIRECTCONNECT);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
							   IPOQUE_PROTOCOL_DIRECTCONNECT);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_MSN
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MSN) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_msn;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_MSN);
#ifdef IPOQUE_PROTOCOL_HTTP
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_HTTP);
#endif
#ifdef IPOQUE_PROTOCOL_SSL
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_SSL);
#endif
		IPOQUE_BITMASK_RESET(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask);
		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,IPOQUE_PROTOCOL_MSN);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_YAHOO
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_YAHOO) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_yahoo;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_YAHOO);
		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_YAHOO);

		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_OSCAR
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_OSCAR) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_oscar;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;
		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_OSCAR);
		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_OSCAR);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_APPLEJUICE
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_APPLEJUICE) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_applejuice_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_APPLEJUICE);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_SOULSEEK
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SOULSEEK) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_soulseek_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_SOULSEEK);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SOULSEEK);


		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_IRC
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_IRC) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_irc_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_IRC);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_IRC);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_UNENCRYPED_JABBER
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_UNENCRYPED_JABBER) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_jabber_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask,
									   IPOQUE_PROTOCOL_UNENCRYPED_JABBER);
#ifdef IPOQUE_PROTOCOL_SSL
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_SSL);
#endif
		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
							   IPOQUE_PROTOCOL_UNENCRYPED_JABBER);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_MAIL_POP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MAIL_POP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_mail_pop_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_MAIL_POP);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_MAIL_IMAP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MAIL_IMAP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_mail_imap_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_MAIL_IMAP);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_MAIL_SMTP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MAIL_SMTP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_mail_smtp_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_MAIL_SMTP);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_FTP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_FTP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_ftp_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_FTP);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_FTP);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_USENET
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_USENET) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_usenet_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_USENET);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_DNS
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_DNS) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_dns;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;


		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_DNS);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_FILETOPIA
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_FILETOPIA) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_filetopia_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_FILETOPIA);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_MANOLITO
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MANOLITO) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_manolito_tcp_udp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_MANOLITO);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_IMESH
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_IMESH) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_imesh_tcp_udp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_IMESH);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_IMESH);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_MMS
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MMS) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_mms_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_MMS);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_PANDO
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_PANDO) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_pando_tcp_udp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_PANDO);

		a++;
	}
#endif
#if defined(IPOQUE_PROTOCOL_IPSEC) || defined(IPOQUE_PROTOCOL_GRE) || defined(IPOQUE_PROTOCOL_ICMP) || defined(IPOQUE_PROTOCOL_IGMP) || defined(IPOQUE_PROTOCOL_EGP) || defined(IPOQUE_PROTOCOL_SCTP) || defined(IPOQUE_PROTOCOL_OSPF) || defined(IPOQUE_PROTOCOL_IP_IN_IP)
	/* always add non tcp/udp if one protocol is compiled in */
	if (1) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_in_non_tcp_udp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_IP;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_BITMASK_RESET(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask);
#ifdef IPOQUE_PROTOCOL_IPSEC
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
									   IPOQUE_PROTOCOL_IPSEC);
#endif
#ifdef IPOQUE_PROTOCOL_GRE
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
									   IPOQUE_PROTOCOL_GRE);
#endif
#ifdef IPOQUE_PROTOCOL_IGMP
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
									   IPOQUE_PROTOCOL_ICMP);
#endif
#ifdef IPOQUE_PROTOCOL_IGMP
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
									   IPOQUE_PROTOCOL_IGMP);
#endif
#ifdef IPOQUE_PROTOCOL_EGP
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
									   IPOQUE_PROTOCOL_EGP);
#endif
#ifdef IPOQUE_PROTOCOL_SCTP
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
									   IPOQUE_PROTOCOL_SCTP);
#endif
#ifdef IPOQUE_PROTOCOL_OSPF
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
									   IPOQUE_PROTOCOL_OSPF);
#endif
#ifdef IPOQUE_PROTOCOL_IP_IN_IP
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
									   IPOQUE_PROTOCOL_IP_IN_IP);
#endif
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_TVANTS
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_TVANTS) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_tvants_udp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_TVANTS);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_SOPCAST
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SOPCAST) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_sopcast;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SOPCAST);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_TVUPLAYER
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_TVUPLAYER) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_tvuplayer;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_TVUPLAYER);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_PPSTREAM
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_PPSTREAM) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_ppstream_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_PPSTREAM);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_PPLIVE
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_PPLIVE) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_pplive_tcp_udp;
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_PPLIVE);
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_PPLIVE);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_IAX
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_IAX) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_iax;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_IAX);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_IAX);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_MGCP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MGCP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_mgcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		//IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_MGCP);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_MGCP);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_GADUGADU
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_GADUGADU) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_gadugadu;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
#ifdef IPOQUE_PROTOCOL_HTTP
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_HTTP);
#endif
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_GADUGADU);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_GADUGADU);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_ZATTOO
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_ZATTOO) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_zattoo_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_ZATTOO);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_ZATTOO);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_QQ
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_QQ) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_qq;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_QQ);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_FEIDIAN
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_FEIDIAN) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_feidian;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_FEIDIAN);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_SSH
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SSH) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_ssh_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SSH);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_POPO
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_POPO) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_popo_tcp_udp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_POPO);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_THUNDER
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_THUNDER) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_thunder;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_THUNDER);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_VNC
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_VNC) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_vnc_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_VNC);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_DHCP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_DHCP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_dhcp_udp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_DHCP);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_I23V5
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_I23V5) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_i23v5;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_I23V5);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_SOCRATES
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SOCRATES) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_socrates;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SOCRATES);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_STEAM
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_STEAM) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_steam;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_STEAM);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_HALFLIFE2
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_HALFLIFE2) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_halflife2;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_HALFLIFE2);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_XBOX
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_XBOX) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_xbox;

		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_XBOX);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_SMB
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SMB) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_smb_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SMB);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_TELNET
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_TELNET) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_telnet_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_TELNET);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_NTP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_NTP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_ntp_udp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_NTP);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_NFS
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_NFS) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_nfs;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_NFS);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_SSDP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SSDP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_ssdp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SSDP);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_WORLDOFWARCRAFT
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_WORLDOFWARCRAFT) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_worldofwarcraft;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
							   IPOQUE_PROTOCOL_WORLDOFWARCRAFT);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_FLASH
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_FLASH) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_flash;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_FLASH);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_POSTGRES
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_POSTGRES) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_postgres_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_POSTGRES);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_MYSQL
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MYSQL) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_mysql_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_MYSQL);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_BGP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_BGP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_bgp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_BGP);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_QUAKE
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_QUAKE) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_quake;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_QUAKE);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_BATTLEFIELD
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_BATTLEFIELD) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_battlefield;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
							   IPOQUE_PROTOCOL_BATTLEFIELD);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_SECONDLIFE
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SECONDLIFE) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_secondlife;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SECONDLIFE);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_PCANYWHERE
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_PCANYWHERE) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_pcanywhere;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_PCANYWHERE);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_RDP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_RDP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_rdp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_RDP);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_SNMP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SNMP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_snmp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SNMP);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_KONTIKI
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_KONTIKI) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_kontiki;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_KONTIKI);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_ICECAST
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_ICECAST) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_icecast_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_ICECAST);
		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_SHOUTCAST
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SHOUTCAST) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_shoutcast_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
#ifdef	IPOQUE_PROTOCOL_HTTP
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_HTTP);
#endif
		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SHOUTCAST);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_VEOHTV
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_VEOHTV) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_veohtv_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_VEOHTV);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_OPENFT
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_OPENFT) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_openft_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_OPENFT);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_SYSLOG
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_SYSLOG) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_syslog;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_SYSLOG);

		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_TDS
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_TDS) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_tds_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_TDS);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_DIRECT_DOWNLOAD_LINK
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_DIRECT_DOWNLOAD_LINK) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_direct_download_link_tcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_TCP_WITH_PAYLOAD;


		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask,
									   IPOQUE_PROTOCOL_DIRECT_DOWNLOAD_LINK);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask,
							   IPOQUE_PROTOCOL_DIRECT_DOWNLOAD_LINK);

		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_NETBIOS
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_NETBIOS) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_netbios;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_NETBIOS);

		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_MDNS
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_MDNS) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_mdns;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_UDP_WITH_PAYLOAD;


		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_MDNS);

		a++;
	}
#endif

#ifdef IPOQUE_PROTOCOL_IPP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_IPP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_ipp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_IPP);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_XDMCP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_XDMCP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_xdmcp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD;


		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_XDMCP);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_XDMCP);

		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_TFTP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_TFTP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_tftp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_TFTP);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_STEALTHNET
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_STEALTHNET) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_stealthnet;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_STEALTHNET);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_AFP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_AFP) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_afp;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask =
			IPQ_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_AFP);
		a++;
	}
#endif
#ifdef IPOQUE_PROTOCOL_AIMINI
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(*detection_bitmask, IPOQUE_PROTOCOL_AIMINI) != 0) {
		ipoque_struct->callback_buffer[a].func = ipoque_search_aimini;
		ipoque_struct->callback_buffer[a].ipq_selection_bitmask = IPQ_SELECTION_BITMASK_PROTOCOL_V4_V6_UDP_WITH_PAYLOAD;

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].detection_bitmask, IPOQUE_PROTOCOL_UNKNOWN);

		IPOQUE_SAVE_AS_BITMASK(ipoque_struct->callback_buffer[a].excluded_protocol_bitmask, IPOQUE_PROTOCOL_AIMINI);
		a++;
	}
#endif
	ipoque_struct->callback_buffer_size = a;

	IPQ_LOG(IPOQUE_PROTOCOL_UNKNOWN, ipoque_struct, IPQ_LOG_DEBUG,
			"callback_buffer_size is %u\n", ipoque_struct->callback_buffer_size);

	/* now build the specific buffer for tcp, udp and non_tcp_udp */
	ipoque_struct->callback_buffer_size_tcp_payload = 0;
	ipoque_struct->callback_buffer_size_tcp_no_payload = 0;
	for (a = 0; a < ipoque_struct->callback_buffer_size; a++) {
		if ((ipoque_struct->callback_buffer[a].ipq_selection_bitmask & (IPQ_SELECTION_BITMASK_PROTOCOL_INT_TCP |
																		IPQ_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP |
																		IPQ_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC))
			!= 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_UNKNOWN, ipoque_struct, IPQ_LOG_DEBUG,
					"callback_buffer_tcp_payload, adding buffer %u as entry %u\n", a,
					ipoque_struct->callback_buffer_size_tcp_payload);

			memcpy(&ipoque_struct->callback_buffer_tcp_payload[ipoque_struct->callback_buffer_size_tcp_payload],
				   &ipoque_struct->callback_buffer[a], sizeof(struct ipq_call_function_struct));
			ipoque_struct->callback_buffer_size_tcp_payload++;

			if ((ipoque_struct->
				 callback_buffer[a].ipq_selection_bitmask & IPQ_SELECTION_BITMASK_PROTOCOL_HAS_PAYLOAD) == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_UNKNOWN, ipoque_struct, IPQ_LOG_DEBUG,
						"callback_buffer_tcp_no_payload, additional adding buffer %u to no_payload process\n", a);

				memcpy(&ipoque_struct->callback_buffer_tcp_no_payload
					   [ipoque_struct->callback_buffer_size_tcp_no_payload], &ipoque_struct->callback_buffer[a],
					   sizeof(struct ipq_call_function_struct));
				ipoque_struct->callback_buffer_size_tcp_no_payload++;
			}
		}
	}

	ipoque_struct->callback_buffer_size_udp = 0;
	for (a = 0; a < ipoque_struct->callback_buffer_size; a++) {
		if ((ipoque_struct->callback_buffer[a].ipq_selection_bitmask & (IPQ_SELECTION_BITMASK_PROTOCOL_INT_UDP |
																		IPQ_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP |
																		IPQ_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC))
			!= 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_UNKNOWN, ipoque_struct, IPQ_LOG_DEBUG,
					"callback_buffer_size_udp, adding buffer %u\n", a);

			memcpy(&ipoque_struct->callback_buffer_udp[ipoque_struct->callback_buffer_size_udp],
				   &ipoque_struct->callback_buffer[a], sizeof(struct ipq_call_function_struct));
			ipoque_struct->callback_buffer_size_udp++;
		}
	}

	ipoque_struct->callback_buffer_size_non_tcp_udp = 0;
	for (a = 0; a < ipoque_struct->callback_buffer_size; a++) {
		if ((ipoque_struct->callback_buffer[a].ipq_selection_bitmask & (IPQ_SELECTION_BITMASK_PROTOCOL_INT_TCP |
																		IPQ_SELECTION_BITMASK_PROTOCOL_INT_UDP |
																		IPQ_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP))
			== 0
			|| (ipoque_struct->
				callback_buffer[a].ipq_selection_bitmask & IPQ_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC) != 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_UNKNOWN, ipoque_struct, IPQ_LOG_DEBUG,
					"callback_buffer_size_non_tcp_udp, adding buffer %u\n", a);

			memcpy(&ipoque_struct->callback_buffer_non_tcp_udp[ipoque_struct->callback_buffer_size_non_tcp_udp],
				   &ipoque_struct->callback_buffer[a], sizeof(struct ipq_call_function_struct));
			ipoque_struct->callback_buffer_size_non_tcp_udp++;
		}
	}
}




static int ipq_init_packet_header(struct ipoque_detection_module_struct *ipoque_struct, unsigned short packetlen)
{
	const struct iphdr *decaps_iph = NULL;
	u16 l3len;
	u16 l4len;
	u8 *l4ptr;
	u8 l4protocol;
	/* reset payload_packet_len, will be set if ipv4 tcp or udp */
	ipoque_struct->packet.payload_packet_len = 0;
	ipoque_struct->packet.l4_packet_len = 0;
	ipoque_struct->packet.l3_packet_len = packetlen;

	ipoque_struct->packet.tcp = NULL;
	ipoque_struct->packet.udp = NULL;
	ipoque_struct->packet.generic_l4_ptr = NULL;

	l3len = ipoque_struct->packet.l3_packet_len;


	decaps_iph = ipoque_struct->packet.iph;


	if (decaps_iph->version == 4 && decaps_iph->ihl >= 5) {
		IPQ_LOG(IPOQUE_PROTOCOL_UNKNOWN, ipoque_struct, IPQ_LOG_DEBUG, "ipv4 header\n");
	} else {
		ipoque_struct->packet.iph = NULL;
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

	if ((decaps_iph->ihl * 4) <= l3len && l3len >= ntohs(decaps_iph->tot_len)
		&& (decaps_iph->frag_off & htons(0x1FFF)) == 0) {
		l4ptr = (((u8 *) decaps_iph) + decaps_iph->ihl * 4);
		l4len = ntohs(decaps_iph->tot_len) - (decaps_iph->ihl * 4);
		l4protocol = decaps_iph->protocol;
	} else {
		return 1;
	}

	ipoque_struct->packet.l4_protocol = l4protocol;
	ipoque_struct->packet.l4_packet_len = l4len;

	/* tcp / udp detection */
	if (l4protocol == 6 /* TCP */  && ipoque_struct->packet.l4_packet_len >= 20 /* min size of tcp */ ) {
		/* tcp */
		ipoque_struct->packet.tcp = (struct tcphdr *) l4ptr;

		if (ipoque_struct->packet.l4_packet_len >= ipoque_struct->packet.tcp->doff * 4) {
			ipoque_struct->packet.payload_packet_len =
				ipoque_struct->packet.l4_packet_len - ipoque_struct->packet.tcp->doff * 4;
			ipoque_struct->packet.actual_payload_len = ipoque_struct->packet.payload_packet_len;
			ipoque_struct->packet.payload = ((u8 *) ipoque_struct->packet.tcp) + (ipoque_struct->packet.tcp->doff * 4);

			/* check for new tcp syn packets, here
			 * idea: reset detection state if a connection is unknown
			 */
			if (ipoque_struct->packet.tcp->syn != 0
				&& ipoque_struct->packet.tcp->ack == 0
				&& ipoque_struct->flow != NULL
				&& ipoque_struct->flow->init_finished != 0
				&& ipoque_struct->flow->detected_protocol == IPOQUE_PROTOCOL_UNKNOWN) {
				memset(ipoque_struct->flow, 0, sizeof(*(ipoque_struct->flow)));
				IPQ_LOG(IPOQUE_PROTOCOL_UNKNOWN, ipoque_struct,
						IPQ_LOG_DEBUG,
						"%s:%u: tcp syn packet for unknown protocol, reset detection state\n", __FUNCTION__, __LINE__);

			}
		} else {
			/* tcp header not complete */
			ipoque_struct->packet.tcp = NULL;
		}
	} else if (l4protocol == 17 /* udp */  && ipoque_struct->packet.l4_packet_len >= 8 /* size of udp */ ) {
		ipoque_struct->packet.udp = (struct udphdr *) l4ptr;
		ipoque_struct->packet.payload_packet_len = ipoque_struct->packet.l4_packet_len - 8;
		ipoque_struct->packet.payload = ((u8 *) ipoque_struct->packet.udp) + 8;
	} else {
		ipoque_struct->packet.generic_l4_ptr = l4ptr;
	}
	return 0;
}

static inline void ipoque_connection_tracking(struct ipoque_detection_module_struct
											  *ipoque_struct)
{
	/* const for gcc code optimisation and cleaner code */
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	const struct iphdr *iph = packet->iph;
	const struct tcphdr *tcph = packet->tcp;
	//const struct udphdr   *udph=ipoque_struct->packet.udp;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	//struct ipoque_unique_flow_struct      unique_flow;
	//uint8_t                               new_connection;

	packet->detected_protocol = 0;

	packet->tcp_retransmission = 0;

	packet->packet_direction = 0;

	if (iph != NULL && iph->saddr < iph->daddr)
		packet->packet_direction = 1;


	packet->packet_lines_parsed_complete = 0;
	packet->packet_unix_lines_parsed_complete = 0;
	if (flow == NULL)
		return;

	if (flow->init_finished == 0) {
		flow->init_finished = 1;
		flow->setup_packet_direction = packet->packet_direction;
	}

	packet->detected_protocol = flow->detected_protocol;
	ipoque_struct->packet.detected_sub_protocol = flow->protocol_subtype;


	if (tcph != NULL) {
		/* reset retried bytes here before setting it */
		packet->num_retried_bytes = 0;

		if (flow->next_tcp_seq_nr[0] == 0 && flow->next_tcp_seq_nr[1] == 0) {
			/* initalize tcp sequence counters */
			if (tcph->seq != 0 && tcph->ack_seq != 0) {
				flow->next_tcp_seq_nr[ipoque_struct->packet.packet_direction] =
					ntohl(tcph->seq) + packet->payload_packet_len;
				flow->next_tcp_seq_nr[1 - ipoque_struct->packet.packet_direction] = ntohl(tcph->ack_seq);
			}
		} else if (packet->payload_packet_len > 0) {
			/* check tcp sequence counters */
			if (((u32)
				 (ntohl(tcph->seq) -
				  flow->next_tcp_seq_nr[packet->packet_direction])) >
				ipoque_struct->tcp_max_retransmission_window_size) {

				packet->tcp_retransmission = 1;


				/*CHECK IF PARTIAL RETRY IS HAPPENENING */
				if ((flow->next_tcp_seq_nr[packet->packet_direction] - ntohl(tcph->seq) < packet->payload_packet_len)) {
					/* num_retried_bytes actual_payload_len hold info about the partial retry
					   analyzer which require this info can make use of this info
					   Other analyzer can use packet->payload_packet_len */
					packet->num_retried_bytes = flow->next_tcp_seq_nr[packet->packet_direction] - ntohl(tcph->seq);
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

}

unsigned int ipoque_detection_process_packet(struct ipoque_detection_module_struct
											 *ipoque_struct, void *flow,
											 const unsigned char *packet,
											 const unsigned short packetlen,
											 const IPOQUE_TIMESTAMP_COUNTER_SIZE current_tick, void *src, void *dst)
{
	u32 a;
	IPQ_SELECTION_BITMASK_PROTOCOL_SIZE ipq_selection_packet;
	IPOQUE_PROTOCOL_BITMASK detection_bitmask;


	/* need at least 20 bytes for ip header */
	if (packetlen < 20) {
		ipoque_struct->packet.detected_protocol = IPOQUE_PROTOCOL_UNKNOWN;
		return IPOQUE_PROTOCOL_UNKNOWN;
	}
	ipoque_struct->packet.tick_timestamp = current_tick;
	ipoque_struct->flow = flow;

	/* parse packet */
	ipoque_struct->packet.iph = (struct iphdr *) packet;
	/* we are interested in ipv4 packet */

	if (ipq_init_packet_header(ipoque_struct, packetlen) != 0) {
		ipoque_struct->packet.detected_protocol = IPOQUE_PROTOCOL_UNKNOWN;
		return IPOQUE_PROTOCOL_UNKNOWN;
	}
	/* detect traffic for tcp or udp only */

	ipoque_struct->src = src;
	ipoque_struct->dst = dst;
	ipoque_connection_tracking(ipoque_struct);

	if (flow == NULL && (ipoque_struct->packet.tcp != NULL || ipoque_struct->packet.udp != NULL)) {
		ipoque_struct->packet.detected_protocol = IPOQUE_PROTOCOL_UNKNOWN;
		return (IPOQUE_PROTOCOL_UNKNOWN);
	}

	/* build ipq_selction packet bitmask */
	ipq_selection_packet = IPQ_SELECTION_BITMASK_PROTOCOL_COMPLETE_TRAFFIC;
	if (ipoque_struct->packet.iph != NULL) {
		ipq_selection_packet |= IPQ_SELECTION_BITMASK_PROTOCOL_IP | IPQ_SELECTION_BITMASK_PROTOCOL_IPV4_OR_IPV6;

	}
	if (ipoque_struct->packet.tcp != NULL) {
		ipq_selection_packet |=
			(IPQ_SELECTION_BITMASK_PROTOCOL_INT_TCP | IPQ_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP);

	}
	if (ipoque_struct->packet.udp != NULL) {
		ipq_selection_packet |=
			(IPQ_SELECTION_BITMASK_PROTOCOL_INT_UDP | IPQ_SELECTION_BITMASK_PROTOCOL_INT_TCP_OR_UDP);
	}
	if (ipoque_struct->packet.payload_packet_len != 0) {
		ipq_selection_packet |= IPQ_SELECTION_BITMASK_PROTOCOL_HAS_PAYLOAD;
	}

	if (ipoque_struct->packet.tcp_retransmission == 0) {
		ipq_selection_packet |= IPQ_SELECTION_BITMASK_PROTOCOL_NO_TCP_RETRANSMISSION;

	}


	IPOQUE_SAVE_AS_BITMASK(detection_bitmask, ipoque_struct->packet.detected_protocol);


	if (flow != NULL && ipoque_struct->packet.tcp != NULL) {
		if (ipoque_struct->packet.payload_packet_len != 0) {
			for (a = 0; a < ipoque_struct->callback_buffer_size_tcp_payload; a++) {
				if ((ipoque_struct->callback_buffer_tcp_payload[a].ipq_selection_bitmask & ipq_selection_packet) ==
					ipoque_struct->callback_buffer_tcp_payload[a].ipq_selection_bitmask
					&& IPOQUE_BITMASK_COMPARE(ipoque_struct->flow->excluded_protocol_bitmask,
											  ipoque_struct->callback_buffer_tcp_payload[a].excluded_protocol_bitmask) == 0
					&& IPOQUE_BITMASK_COMPARE(ipoque_struct->callback_buffer_tcp_payload[a].detection_bitmask,
											  detection_bitmask) != 0) {
					ipoque_struct->callback_buffer_tcp_payload[a].func(ipoque_struct);
				}
			}
		} else {				/* no payload */

			for (a = 0; a < ipoque_struct->callback_buffer_size_tcp_no_payload; a++) {
				if ((ipoque_struct->callback_buffer_tcp_no_payload[a].ipq_selection_bitmask & ipq_selection_packet) ==
					ipoque_struct->callback_buffer_tcp_no_payload[a].ipq_selection_bitmask
					&& IPOQUE_BITMASK_COMPARE(ipoque_struct->flow->excluded_protocol_bitmask,
											  ipoque_struct->callback_buffer_tcp_no_payload[a].excluded_protocol_bitmask) == 0
					&& IPOQUE_BITMASK_COMPARE(ipoque_struct->callback_buffer_tcp_no_payload[a].detection_bitmask,
											  detection_bitmask) != 0) {
					ipoque_struct->callback_buffer_tcp_no_payload[a].func(ipoque_struct);
				}
			}
		}
	} else if (flow != NULL && ipoque_struct->packet.udp != NULL) {
		for (a = 0; a < ipoque_struct->callback_buffer_size_udp; a++) {
			if ((ipoque_struct->callback_buffer_udp[a].ipq_selection_bitmask & ipq_selection_packet) ==
				ipoque_struct->callback_buffer_udp[a].ipq_selection_bitmask
				&& IPOQUE_BITMASK_COMPARE(ipoque_struct->flow->excluded_protocol_bitmask,
										  ipoque_struct->callback_buffer_udp[a].excluded_protocol_bitmask) == 0
				&& IPOQUE_BITMASK_COMPARE(ipoque_struct->callback_buffer_udp[a].detection_bitmask,
										  detection_bitmask) != 0) {
				ipoque_struct->callback_buffer_udp[a].func(ipoque_struct);

			}
		}
	} else {

		for (a = 0; a < ipoque_struct->callback_buffer_size_non_tcp_udp; a++) {
			if ((ipoque_struct->callback_buffer_non_tcp_udp[a].ipq_selection_bitmask & ipq_selection_packet) ==
				ipoque_struct->callback_buffer_non_tcp_udp[a].ipq_selection_bitmask
				&& (ipoque_struct->flow == NULL ||
					IPOQUE_BITMASK_COMPARE(ipoque_struct->flow->excluded_protocol_bitmask,
										   ipoque_struct->callback_buffer_non_tcp_udp[a].excluded_protocol_bitmask) == 0)
				&& IPOQUE_BITMASK_COMPARE(ipoque_struct->callback_buffer_non_tcp_udp[a].detection_bitmask,
										  detection_bitmask) != 0) {

				ipoque_struct->callback_buffer_non_tcp_udp[a].func(ipoque_struct);
			}

		}
	}



	a = ipoque_struct->packet.detected_protocol;
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->detection_bitmask, a)
		== 0)
		a = IPOQUE_PROTOCOL_UNKNOWN;



	return a;
}

u32 ipq_bytestream_to_number(const u8 * str, u16 max_chars_to_read, u16 * bytes_read)
{
	u32 val;
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

u32 ipq_bytestream_dec_or_hex_to_number(const u8 * str, u16 max_chars_to_read, u16 * bytes_read)
{
	u32 val;
	val = 0;
	if (max_chars_to_read <= 2 || str[0] != '0' || str[1] != 'x') {
		return ipq_bytestream_to_number(str, max_chars_to_read, bytes_read);
	} else {
		/*use base 16 system */
		max_chars_to_read += 2;
		while (max_chars_to_read > 0) {

			if (*str >= '0' && *str <= '9') {
				val *= 10;
				val += *str - '0';
			} else if (*str >= 'a' && *str <= 'f') {
				val *= 10;
				val += *str + 10 - 'a';
			} else if (*str >= 'A' && *str <= 'F') {
				val *= 10;
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


u64 ipq_bytestream_to_number64(const u8 * str, u16 max_chars_to_read, u16 * bytes_read)
{
	u64 val;
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

u64 ipq_bytestream_dec_or_hex_to_number64(const u8 * str, u16 max_chars_to_read, u16 * bytes_read)
{
	u64 val;
	val = 0;
	if (max_chars_to_read <= 2 || str[0] != '0' || str[1] != 'x') {
		return ipq_bytestream_to_number64(str, max_chars_to_read, bytes_read);
	} else {
		/*use base 16 system */
		max_chars_to_read += 2;
		while (max_chars_to_read > 0) {

			if (*str >= '0' && *str <= '9') {
				val *= 10;
				val += *str - '0';
			} else if (*str >= 'a' && *str <= 'f') {
				val *= 10;
				val += *str + 10 - 'a';
			} else if (*str >= 'A' && *str <= 'F') {
				val *= 10;
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


u32 ipq_bytestream_to_ipv4(const u8 * str, u16 max_chars_to_read, u16 * bytes_read)
{
	u32 val;
	u16 read = 0;
	u16 oldread;
	u32 c;
	/* ip address must be X.X.X.X with each X between 0 and 255 */
	oldread = read;
	c = ipq_bytestream_to_number(str, max_chars_to_read, &read);
	if (c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
		return 0;
	read++;
	val = c << 24;
	oldread = read;
	c = ipq_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
	if (c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
		return 0;
	read++;
	val = val + (c << 16);
	oldread = read;
	c = ipq_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
	if (c > 255 || oldread == read || max_chars_to_read == read || str[read] != '.')
		return 0;
	read++;
	val = val + (c << 8);
	oldread = read;
	c = ipq_bytestream_to_number(&str[read], max_chars_to_read - read, &read);
	if (c > 255 || oldread == read || max_chars_to_read == read)
		return 0;
	val = val + c;

	*bytes_read = *bytes_read + read;

	return htonl(val);
}

/* internal function for every detection to parse one packet and to increase the info buffer */
void ipq_parse_packet_line_info(struct ipoque_detection_module_struct
								*ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	u32 a;
	u16 end = packet->payload_packet_len - 1;
	if (packet->packet_lines_parsed_complete != 0)
		return;

	packet->packet_lines_parsed_complete = 1;
	packet->parsed_lines = 0;

	packet->empty_line_position_set = 0;

	packet->host_line.ptr = NULL;
	packet->host_line.len = 0;
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

	if (packet->payload_packet_len == 0)
		return;

	packet->line[packet->parsed_lines].ptr = packet->payload;
	packet->line[packet->parsed_lines].len = 0;

	for (a = 0; a < end; a++) {
		if (get_u16(packet->payload, a) == ntohs(0x0d0a)) {
			packet->line[packet->parsed_lines].len =
				((unsigned long) &packet->payload[a]) - ((unsigned long) packet->line[packet->parsed_lines].ptr);

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
				&& memcmp(packet->line[packet->parsed_lines].ptr, "User-Agent: ", 12) == 0) {
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

			if (packet->parsed_lines >= (IPOQUE_MAX_PARSE_LINES_PER_PACKET - 1)) {
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
			=
			((unsigned long) &packet->payload[packet->payload_packet_len]) -
			((unsigned long) packet->line[packet->parsed_lines].ptr);
		packet->parsed_lines++;
	}
}

void ipq_parse_packet_line_info_unix(struct ipoque_detection_module_struct
									 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	u32 a;
	u16 end = packet->payload_packet_len;
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
			packet->unix_line[packet->parsed_unix_lines].len =
				((unsigned long) &packet->payload[a]) -
				((unsigned long) packet->unix_line[packet->parsed_unix_lines].ptr);

			if (packet->parsed_unix_lines >= (IPOQUE_MAX_PARSE_LINES_PER_PACKET - 1)) {
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



u16 ipoque_check_for_email_address(struct ipoque_detection_module_struct *ipoque_struct, u16 counter)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;

	IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, "called ipoque_check_for_email_address\n");

	if (packet->payload_packet_len > counter && ((packet->payload[counter] >= 'a' && packet->payload[counter] <= 'z')
												 || (packet->payload[counter] >= 'A' && packet->payload[counter] <= 'Z')
												 || (packet->payload[counter] >= '0' && packet->payload[counter] <= '9')
												 || packet->payload[counter] == '-' || packet->payload[counter] == '_')) {
		IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, "first letter\n");
		counter++;
		while (packet->payload_packet_len > counter
			   && ((packet->payload[counter] >= 'a' && packet->payload[counter] <= 'z')
				   || (packet->payload[counter] >= 'A' && packet->payload[counter] <= 'Z')
				   || (packet->payload[counter] >= '0' && packet->payload[counter] <= '9')
				   || packet->payload[counter] == '-' || packet->payload[counter] == '_'
				   || packet->payload[counter] == '.')) {
			IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, "further letter\n");
			counter++;
			if (packet->payload_packet_len > counter && packet->payload[counter] == '@') {
				IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, "@\n");
				counter++;
				while (packet->payload_packet_len > counter
					   && ((packet->payload[counter] >= 'a' && packet->payload[counter] <= 'z')
						   || (packet->payload[counter] >= 'A' && packet->payload[counter] <= 'Z')
						   || (packet->payload[counter] >= '0' && packet->payload[counter] <= '9')
						   || packet->payload[counter] == '-' || packet->payload[counter] == '_')) {
					IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, "letter\n");
					counter++;
					if (packet->payload_packet_len > counter && packet->payload[counter] == '.') {
						IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, ".\n");
						counter++;
						if (packet->payload_packet_len > counter + 1
							&& ((packet->payload[counter] >= 'a' && packet->payload[counter] <= 'z')
								&& (packet->payload[counter + 1] >= 'a' && packet->payload[counter + 1] <= 'z'))) {
							IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, "two letters\n");
							counter += 2;
							if (packet->payload_packet_len > counter
								&& (packet->payload[counter] == ' ' || packet->payload[counter] == ';')) {
								IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, "whitespace1\n");
								return counter;
							} else if (packet->payload_packet_len > counter && packet->payload[counter] >= 'a'
									   && packet->payload[counter] <= 'z') {
								IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, "one letter\n");
								counter++;
								if (packet->payload_packet_len > counter
									&& (packet->payload[counter] == ' ' || packet->payload[counter] == ';')) {
									IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, "whitespace2\n");
									return counter;
								} else if (packet->payload_packet_len > counter && packet->payload[counter] >= 'a'
										   && packet->payload[counter] <= 'z') {
									counter++;
									if (packet->payload_packet_len > counter
										&& (packet->payload[counter] == ' ' || packet->payload[counter] == ';')) {
										IPQ_LOG(IPOQUE_PROTOCOL_MSN, ipoque_struct, IPQ_LOG_DEBUG, "whitespace3\n");
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

#ifdef IPOQUE_ENABLE_DEBUG_MESSAGES
void ipoque_debug_get_last_log_function_line(struct ipoque_detection_module_struct
											 *ipoque_struct, const char **file, const char **func, u32 * line)
{
	*file = "";
	*func = "";

	if (ipoque_struct->ipoque_debug_print_file != NULL)
		*file = ipoque_struct->ipoque_debug_print_file;

	if (ipoque_struct->ipoque_debug_print_function != NULL)
		*func = ipoque_struct->ipoque_debug_print_function;

	*line = ipoque_struct->ipoque_debug_print_line;
}
#endif
