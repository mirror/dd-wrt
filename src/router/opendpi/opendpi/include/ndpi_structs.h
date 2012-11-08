/*
 * ndpi_structs.h
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


#ifndef __NDPI_STRUCTS_INCLUDE_FILE__
#define __NDPI_STRUCTS_INCLUDE_FILE__

#include "linux_compat.h"

#include "ndpi_define.h"

#ifdef NDPI_DETECTION_SUPPORT_IPV6
struct ndpi_ip6_addr {
  union {
    u_int8_t u6_addr8[16];
    u_int16_t u6_addr16[8];
    u_int32_t u6_addr32[4];
    u_int64_t u6_addr64[2];
  } ndpi_v6_u;

#define ndpi_v6_addr		ndpi_v6_u.u6_addr8
#define ndpi_v6_addr16		ndpi_v6_u.u6_addr16
#define ndpi_v6_addr32		ndpi_v6_u.u6_addr32
#define ndpi_v6_addr64		ndpi_v6_u.u6_addr64
};

struct ndpi_ipv6hdr {
  /* use userspace and kernelspace compatible compile parameters */
#if defined(__LITTLE_ENDIAN__)
  u_int8_t priority:4, version:4;
#elif defined(__BIG_ENDIAN__)
  u_int8_t version:4, priority:4;
#else
# error "Byte order must be defined"
#endif

  u_int8_t flow_lbl[3];

  u_int16_t payload_len;
  u_int8_t nexthdr;
  u_int8_t hop_limit;

  struct ndpi_ip6_addr saddr;
  struct ndpi_ip6_addr daddr;
};
#endif							/* NDPI_DETECTION_SUPPORT_IPV6 */

typedef union {
  u_int32_t ipv4;
  u_int8_t ipv4_u_int8_t[4];
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  struct ndpi_ip6_addr ipv6;
#endif
} ndpi_ip_addr_t;


# define MAX_PACKET_COUNTER 65000

typedef struct ndpi_id_struct {
  /* detected_protocol_bitmask:
   * access this bitmask to find out whether an id has used skype or not
   * if a flag is set here, it will not be resetted
   * to compare this, use:
   * if (NDPI_BITMASK_COMPARE(id->detected_protocol_bitmask,
   *                            NDPI_PROTOCOL_BITMASK_XXX) != 0)
   * {
   *      // protocol XXX detected on this id
   * }
   */
  NDPI_PROTOCOL_BITMASK detected_protocol_bitmask;
#ifdef NDPI_PROTOCOL_FTP
  ndpi_ip_addr_t ftp_ip;
#endif
#ifdef NDPI_PROTOCOL_RTSP
  ndpi_ip_addr_t rtsp_ip_address;
#endif
#ifdef NDPI_PROTOCOL_PPLIVE
  u_int32_t pplive_last_packet_time;
#endif
#ifdef NDPI_PROTOCOL_SIP
#ifdef NDPI_PROTOCOL_YAHOO
  u_int32_t yahoo_video_lan_timer;
#endif
#endif
#ifdef NDPI_PROTOCOL_IRC
  u_int32_t last_time_port_used[16];
#endif
#ifdef NDPI_PROTOCOL_FTP
  u_int32_t ftp_timer;
#endif
#ifdef NDPI_PROTOCOL_IRC
  u_int32_t irc_ts;
#endif
#ifdef NDPI_PROTOCOL_GNUTELLA
  u_int32_t gnutella_ts;
#endif
#ifdef NDPI_PROTOCOL_BATTLEFIELD
  u_int32_t battlefield_ts;
#endif
#ifdef NDPI_PROTOCOL_THUNDER
  u_int32_t thunder_ts;
#endif
#ifdef NDPI_PROTOCOL_RTSP
  u_int32_t rtsp_timer;
#endif
#ifdef NDPI_PROTOCOL_OSCAR
  u_int32_t oscar_last_safe_access_time;
#endif
#ifdef NDPI_PROTOCOL_GADUGADU
  u_int32_t gg_ft_ip_address;
  u_int32_t gg_timeout;
#endif
#ifdef NDPI_PROTOCOL_ZATTOO
  u_int32_t zattoo_ts;
#endif
#ifdef NDPI_PROTOCOL_UNENCRYPED_JABBER
  u_int32_t jabber_stun_or_ft_ts;
#endif
#ifdef NDPI_PROTOCOL_MANOLITO
  u_int32_t manolito_last_pkt_arrival_time;
#endif
#ifdef NDPI_PROTOCOL_DIRECTCONNECT
  u_int32_t directconnect_last_safe_access_time;
#endif
#ifdef NDPI_PROTOCOL_SOULSEEK
  u_int32_t soulseek_last_safe_access_time;
#endif
#ifdef NDPI_PROTOCOL_DIRECTCONNECT
  u_int16_t detected_directconnect_port;
  u_int16_t detected_directconnect_udp_port;
  u_int16_t detected_directconnect_ssl_port;
#endif
#ifdef NDPI_PROTOCOL_PPLIVE
  u_int16_t pplive_vod_cli_port;
#endif
#ifdef NDPI_PROTOCOL_IRC
  u_int16_t irc_port[16];
#endif
#ifdef NDPI_PROTOCOL_GADUGADU
  u_int16_t gg_ft_port;
#endif
#ifdef NDPI_PROTOCOL_UNENCRYPED_JABBER
#define JABBER_MAX_STUN_PORTS 6
  u_int16_t jabber_voice_stun_port[JABBER_MAX_STUN_PORTS];
  u_int16_t jabber_file_transfer_port[2];
#endif
#ifdef NDPI_PROTOCOL_GNUTELLA
  u_int16_t detected_gnutella_port;
#endif
#ifdef NDPI_PROTOCOL_GNUTELLA
  u_int16_t detected_gnutella_udp_port1;
  u_int16_t detected_gnutella_udp_port2;
#endif
#ifdef NDPI_PROTOCOL_SOULSEEK
  u_int16_t soulseek_listen_port;
#endif
#ifdef NDPI_PROTOCOL_IRC
  u_int8_t irc_number_of_port;
#endif
#ifdef NDPI_PROTOCOL_OSCAR
  u_int8_t oscar_ssl_session_id[33];
#endif
#ifdef NDPI_PROTOCOL_GADUGADU
  u_int8_t gg_call_id[2][7];
  u_int8_t gg_fmnumber[8];
#endif
#ifdef NDPI_PROTOCOL_UNENCRYPED_JABBER
  u_int8_t jabber_voice_stun_used_ports;
#endif
#ifdef NDPI_PROTOCOL_SIP
#ifdef NDPI_PROTOCOL_YAHOO
  u_int32_t yahoo_video_lan_dir:1;
#endif
#endif
#ifdef NDPI_PROTOCOL_YAHOO
  u_int32_t yahoo_conf_logged_in:1;
  u_int32_t yahoo_voice_conf_logged_in:1;
#endif
#ifdef NDPI_PROTOCOL_FTP
  u_int32_t ftp_timer_set:1;
#endif
#ifdef NDPI_PROTOCOL_GADUGADU
  u_int32_t gadu_gadu_ft_direction:1;
  u_int32_t gadu_gadu_voice:1;
  u_int32_t gg_next_id:1;
#endif
#ifdef NDPI_PROTOCOL_RTSP
  u_int32_t rtsp_ts_set:1;
#endif
#ifdef NDPI_PROTOCOL_PPLIVE
  u_int32_t pplive_last_packet_time_set:1;
#endif
} ndpi_id_struct;
struct ndpi_flow_tcp_struct {
#ifdef NDPI_PROTOCOL_FLASH
  u_int16_t flash_bytes;
#endif
#ifdef NDPI_PROTOCOL_MAIL_SMTP
  u_int16_t smtp_command_bitmask;
#endif
#ifdef NDPI_PROTOCOL_MAIL_POP
  u_int16_t pop_command_bitmask;
#endif
#ifdef NDPI_PROTOCOL_QQ
  u_int16_t qq_nxt_len;
#endif
#ifdef NDPI_PROTOCOL_TDS
  u_int8_t tds_login_version;
#endif
#ifdef NDPI_PROTOCOL_PPLIVE
  u_int8_t pplive_next_packet_size[2];
#endif
#ifdef NDPI_PROTOCOL_IRC
  u_int8_t irc_stage;
  u_int8_t irc_port;
#endif
#ifdef NDPI_PROTOCOL_GNUTELLA
  u_int8_t gnutella_msg_id[3];
#endif
#ifdef NDPI_PROTOCOL_EDONKEY
  u_int32_t edk_ext:1;
#endif
#ifdef NDPI_PROTOCOL_IRC
  u_int32_t irc_3a_counter:3;
  u_int32_t irc_stage2:5;
  u_int32_t irc_direction:2;
  u_int32_t irc_0x1000_full:1;
#endif
#ifdef NDPI_PROTOCOL_WINMX
  u_int32_t winmx_stage:1;			// 0-1
#endif
#ifdef NDPI_PROTOCOL_SOULSEEK
  u_int32_t soulseek_stage:2;
#endif
#ifdef NDPI_PROTOCOL_FILETOPIA
  u_int32_t filetopia_stage:2;
#endif
#ifdef NDPI_PROTOCOL_MANOLITO
  u_int32_t manolito_stage:4;
#endif
#ifdef NDPI_PROTOCOL_TDS
  u_int32_t tds_stage:3;
#endif
#ifdef NDPI_PROTOCOL_GADUGADU
  u_int32_t gadugadu_stage:2;
#endif
#ifdef NDPI_PROTOCOL_USENET
  u_int32_t usenet_stage:2;
#endif
#ifdef NDPI_PROTOCOL_IMESH
  u_int32_t imesh_stage:4;
#endif
#ifdef NDPI_PROTOCOL_FTP
  u_int32_t ftp_codes_seen:5;
  u_int32_t ftp_client_direction:1;
#endif
#ifdef NDPI_PROTOCOL_HTTP
  u_int32_t http_setup_dir:2;
  u_int32_t http_stage:2;
  u_int32_t http_empty_line_seen:1;
  u_int32_t http_wait_for_retransmission:1;
#endif							// NDPI_PROTOCOL_HTTP
#ifdef NDPI_PROTOCOL_FLASH
  u_int32_t flash_stage:3;
#endif
#ifdef NDPI_PROTOCOL_GNUTELLA
  u_int32_t gnutella_stage:2;		//0-2
#endif
#ifdef NDPI_PROTOCOL_MMS
  u_int32_t mms_stage:2;
#endif
#ifdef NDPI_PROTOCOL_YAHOO
  u_int32_t yahoo_sip_comm:1;
  u_int32_t yahoo_http_proxy_stage:2;
#endif
#ifdef NDPI_PROTOCOL_MSN
  u_int32_t msn_stage:3;
  u_int32_t msn_ssl_ft:2;
#endif
#ifdef NDPI_PROTOCOL_SSH
  u_int32_t ssh_stage:3;
#endif
#ifdef NDPI_PROTOCOL_VNC
  u_int32_t vnc_stage:2;			// 0 - 3
#endif
#ifdef NDPI_PROTOCOL_STEAM
  u_int32_t steam_stage:2;			// 0 - 3
#endif
#ifdef NDPI_PROTOCOL_TELNET
  u_int32_t telnet_stage:2;			// 0 - 2
#endif
#ifdef NDPI_PROTOCOL_SSL
  u_int32_t ssl_stage:2;			// 0 - 3
#endif
#ifdef NDPI_PROTOCOL_POSTGRES
  u_int32_t postgres_stage:3;
#endif
#ifdef NDPI_PROTOCOL_DIRECT_DOWNLOAD_LINK
  u_int32_t ddlink_server_direction:1;
#endif
  u_int32_t seen_syn:1;
  u_int32_t seen_syn_ack:1;
  u_int32_t seen_ack:1;
#ifdef NDPI_PROTOCOL_ICECAST
  u_int32_t icecast_stage:1;
#endif
#ifdef NDPI_PROTOCOL_DOFUS
  u_int32_t dofus_stage:1;
#endif
#ifdef NDPI_PROTOCOL_FIESTA
  u_int32_t fiesta_stage:2;
#endif
#ifdef NDPI_PROTOCOL_WORLDOFWARCRAFT
  u_int32_t wow_stage:2;
#endif
#ifdef NDPI_PROTOCOL_HTTP_APPLICATION_VEOHTV
  u_int32_t veoh_tv_stage:2;
#endif
#ifdef NDPI_PROTOCOL_SHOUTCAST
  u_int32_t shoutcast_stage:2;
#endif
#ifdef NDPI_PROTOCOL_RTP
  u_int32_t rtp_special_packets_seen:1;
#endif
#ifdef NDPI_PROTOCOL_MAIL_POP
  u_int32_t mail_pop_stage:2;
#endif
#ifdef NDPI_PROTOCOL_MAIL_IMAP
  u_int32_t mail_imap_stage:3;
#endif

#ifdef NDPI_PROTOCOL_SKYPE
  u_int8_t skype_packet_id;
#endif

#ifdef NDPI_PROTOCOL_CITRIX
  u_int8_t citrix_packet_id;
#endif

#ifdef NDPI_PROTOCOL_TEAMVIEWER
  u_int8_t teamviewer_stage;
#endif
} 

#if !defined(WIN32)
  __attribute__ ((__packed__))
#endif
  ;

struct ndpi_flow_udp_struct {
#ifdef NDPI_PROTOCOL_BATTLEFIELD
  u_int32_t battlefield_msg_id;
#endif
#ifdef NDPI_PROTOCOL_SNMP
  u_int32_t snmp_msg_id;
#endif
#ifdef NDPI_PROTOCOL_BATTLEFIELD
  u_int32_t battlefield_stage:3;
#endif
#ifdef NDPI_PROTOCOL_SNMP
  u_int32_t snmp_stage:2;
#endif
#ifdef NDPI_PROTOCOL_PPSTREAM
  u_int32_t ppstream_stage:3;		// 0-7
#endif
#ifdef NDPI_PROTOCOL_FEIDIAN
  u_int32_t feidian_stage:1;		// 0-7
#endif
#ifdef NDPI_PROTOCOL_HALFLIFE2
  u_int32_t halflife2_stage:2;		// 0 - 2
#endif
#ifdef NDPI_PROTOCOL_TFTP
  u_int32_t tftp_stage:1;
#endif
#ifdef NDPI_PROTOCOL_AIMINI
  u_int32_t aimini_stage:5;
#endif
#ifdef NDPI_PROTOCOL_XBOX
  u_int32_t xbox_stage:1;
#endif
#ifdef NDPI_PROTOCOL_WINDOWS_UPDATE
  u_int32_t wsus_stage:1;
#endif
#ifdef NDPI_PROTOCOL_SKYPE
  u_int8_t skype_packet_id;
#endif
#ifdef NDPI_PROTOCOL_TEAMVIEWER
  u_int8_t teamviewer_stage;
#endif
}

#if !defined(WIN32)
  __attribute__ ((__packed__))
#endif
  ;

typedef struct ndpi_int_one_line_struct {
  const u_int8_t *ptr;
  u_int16_t len;
} ndpi_int_one_line_struct_t;

typedef struct ndpi_packet_struct {
  const struct ndpi_iphdr *iph;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
  const struct ndpi_ipv6hdr *iphv6;
#endif
  const struct ndpi_tcphdr *tcp;
  const struct ndpi_udphdr *udp;
  const u_int8_t *generic_l4_ptr;	/* is set only for non tcp-udp traffic */
  const u_int8_t *payload;

  u_int32_t tick_timestamp;

  u_int16_t detected_protocol_stack[NDPI_PROTOCOL_HISTORY_SIZE];
  u_int8_t detected_subprotocol_stack[NDPI_PROTOCOL_HISTORY_SIZE];

  /* this is for simple read-only access to the real protocol 
   * used for the main loop */
  u_int16_t real_protocol_read_only;

#if NDPI_PROTOCOL_HISTORY_SIZE > 1
#  if NDPI_PROTOCOL_HISTORY_SIZE > 5
#    error protocol stack size not supported
#  endif

  struct {
    u_int8_t entry_is_real_protocol:5;
    u_int8_t current_stack_size_minus_one:3;
  } 
#if !defined(WIN32)
    __attribute__ ((__packed__))
#endif
    protocol_stack_info;
#endif

  struct ndpi_int_one_line_struct line[NDPI_MAX_PARSE_LINES_PER_PACKET];
  struct ndpi_int_one_line_struct unix_line[NDPI_MAX_PARSE_LINES_PER_PACKET];
  struct ndpi_int_one_line_struct host_line;
  struct ndpi_int_one_line_struct referer_line;
  struct ndpi_int_one_line_struct content_line;
  struct ndpi_int_one_line_struct accept_line;
  struct ndpi_int_one_line_struct user_agent_line;
  struct ndpi_int_one_line_struct http_url_name;
  struct ndpi_int_one_line_struct http_encoding;
  struct ndpi_int_one_line_struct http_transfer_encoding;
  struct ndpi_int_one_line_struct http_contentlen;
  struct ndpi_int_one_line_struct http_cookie;
  struct ndpi_int_one_line_struct http_x_session_type;
  struct ndpi_int_one_line_struct server_line;
  struct ndpi_int_one_line_struct http_method;
  struct ndpi_int_one_line_struct http_response;


  u_int16_t l3_packet_len;
  u_int16_t l4_packet_len;
  u_int16_t payload_packet_len;
  u_int16_t actual_payload_len;
  u_int16_t num_retried_bytes;
  u_int16_t parsed_lines;
  u_int16_t parsed_unix_lines;
  u_int16_t empty_line_position;
  u_int8_t tcp_retransmission;
  u_int8_t l4_protocol;

  u_int8_t packet_lines_parsed_complete;
  u_int8_t packet_unix_lines_parsed_complete;
  u_int8_t empty_line_position_set;
  u_int8_t packet_direction:1;
} ndpi_packet_struct_t;

struct ndpi_detection_module_struct;
struct ndpi_flow_struct;

typedef struct ndpi_call_function_struct {
  NDPI_PROTOCOL_BITMASK detection_bitmask;
  NDPI_PROTOCOL_BITMASK excluded_protocol_bitmask;
  NDPI_SELECTION_BITMASK_PROTOCOL_SIZE ndpi_selection_bitmask;
  void (*func) (struct ndpi_detection_module_struct *, struct ndpi_flow_struct *flow);
  u_int8_t detection_feature;
} ndpi_call_function_struct_t;

typedef struct ndpi_detection_module_struct {
  NDPI_PROTOCOL_BITMASK detection_bitmask;
  NDPI_PROTOCOL_BITMASK generic_http_packet_bitmask;
  
  u_int32_t current_ts;
  u_int32_t ticks_per_second;

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
  void *user_data;
#endif
  /* callback function buffer */
  struct ndpi_call_function_struct callback_buffer[NDPI_MAX_SUPPORTED_PROTOCOLS + 1];
  u_int32_t callback_buffer_size;

  struct ndpi_call_function_struct callback_buffer_tcp_no_payload[NDPI_MAX_SUPPORTED_PROTOCOLS + 1];
  u_int32_t callback_buffer_size_tcp_no_payload;

  struct ndpi_call_function_struct callback_buffer_tcp_payload[NDPI_MAX_SUPPORTED_PROTOCOLS + 1];
  u_int32_t callback_buffer_size_tcp_payload;


  struct ndpi_call_function_struct callback_buffer_udp[NDPI_MAX_SUPPORTED_PROTOCOLS + 1];
  u_int32_t callback_buffer_size_udp;


  struct ndpi_call_function_struct callback_buffer_non_tcp_udp[NDPI_MAX_SUPPORTED_PROTOCOLS + 1];
  u_int32_t callback_buffer_size_non_tcp_udp;

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
  /* debug callback, only set when debug is used */
  ndpi_debug_function_ptr ndpi_debug_printf;
  const char *ndpi_debug_print_file;
  const char *ndpi_debug_print_function;
  u_int32_t ndpi_debug_print_line;
#endif
  /* misc parameters */
  u_int32_t tcp_max_retransmission_window_size;

  u_int32_t edonkey_upper_ports_only:1;
  u_int32_t edonkey_safe_mode:1;
  u_int32_t directconnect_connection_ip_tick_timeout;

  /*gadu gadu*/
  u_int32_t gadugadu_peer_connection_timeout;
  /* pplive params */
  u_int32_t pplive_connection_timeout;
  /* ftp parameters */
  u_int32_t ftp_connection_timeout;
  /* irc parameters */
  u_int32_t irc_timeout;
  /* gnutella parameters */
  u_int32_t gnutella_timeout;
  /* battlefield parameters */
  u_int32_t battlefield_timeout;
  /* thunder parameters */
  u_int32_t thunder_timeout;
  /* SoulSeek parameters */
  u_int32_t soulseek_connection_ip_tick_timeout;
  /* rtsp parameters */
  u_int32_t rtsp_connection_timeout;
  /* tvants parameters */
  u_int32_t tvants_connection_timeout;
  u_int32_t orb_rstp_ts_timeout;
  /* yahoo */
  //      u_int32_t yahoo_http_filetransfer_timeout;
  u_int8_t yahoo_detect_http_connections;
  u_int32_t yahoo_lan_video_timeout;
  u_int32_t zattoo_connection_timeout;
  u_int32_t jabber_stun_timeout;
  u_int32_t jabber_file_transfer_timeout;
  u_int32_t manolito_subscriber_timeout;
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
#define NDPI_IP_STRING_SIZE 40
  char ip_string[NDPI_IP_STRING_SIZE];
#endif
  u_int8_t ip_version_limit;
} ndpi_detection_module_struct_t;

typedef struct ndpi_flow_struct {
  u_int16_t detected_protocol_stack[NDPI_PROTOCOL_HISTORY_SIZE];
#if NDPI_PROTOCOL_HISTORY_SIZE > 1
#  if NDPI_PROTOCOL_HISTORY_SIZE > 5
#    error protocol stack size not supported
#  endif
  
  struct {
    u_int8_t entry_is_real_protocol:5;
    u_int8_t current_stack_size_minus_one:3;
  } 
    
#if !defined(WIN32)
    __attribute__ ((__packed__))
#endif
    protocol_stack_info;
#endif
  
  
  /* init parameter, internal used to set up timestamp,... */
  u_int8_t init_finished:1;
  u_int8_t setup_packet_direction:1;
  /* tcp sequence number connection tracking */
  u_int32_t next_tcp_seq_nr[2];

  /* the tcp / udp / other l4 value union
   * this is used to reduce the number of bytes for tcp or udp protocol states
   * */
  union {
    struct ndpi_flow_tcp_struct tcp;
    struct ndpi_flow_udp_struct udp;
  } l4;


  /* ALL protocol specific 64 bit variables here */

  /* protocols which have marked a connection as this connection cannot be protocol XXX, multiple u_int64_t */
  NDPI_PROTOCOL_BITMASK excluded_protocol_bitmask;

#ifdef NDPI_PROTOCOL_RTP
  u_int32_t rtp_ssid[2];
#endif
#ifdef NDPI_PROTOCOL_I23V5
  u_int32_t i23v5_len1;
  u_int32_t i23v5_len2;
  u_int32_t i23v5_len3;
#endif
  u_int16_t packet_counter;			// can be 0-65000
  u_int16_t packet_direction_counter[2];
  u_int16_t byte_counter[2];
#ifdef NDPI_PROTOCOL_RTP
  u_int16_t rtp_seqnum[2];			/* current highest sequence number (only goes forwards, is not decreased by retransmissions) */
#endif
#ifdef NDPI_PROTOCOL_RTP
  /* tcp and udp */
  u_int8_t rtp_payload_type[2];
#endif

#ifdef NDPI_PROTOCOL_BITTORRENT
  u_int8_t bittorrent_stage;		// can be 0-255
#endif
#ifdef NDPI_PROTOCOL_RTP
  u_int32_t rtp_stage1:2;			//0-3
  u_int32_t rtp_stage2:2;
#endif
#ifdef NDPI_PROTOCOL_EDONKEY
  u_int32_t edk_stage:5;			// 0-17
#endif
#ifdef NDPI_PROTOCOL_DIRECTCONNECT
  u_int32_t directconnect_stage:2;	// 0-1
#endif
#ifdef NDPI_PROTOCOL_SIP
#ifdef NDPI_PROTOCOL_YAHOO
  u_int32_t sip_yahoo_voice:1;
#endif
#endif
#ifdef NDPI_PROTOCOL_HTTP
  u_int32_t http_detected:1;
#endif							// NDPI_PROTOCOL_HTTP
#ifdef NDPI_PROTOCOL_RTSP
  u_int32_t rtsprdt_stage:2;
  u_int32_t rtsp_control_flow:1;
#endif

#ifdef NDPI_PROTOCOL_YAHOO
  u_int32_t yahoo_detection_finished:2;
#endif
#ifdef NDPI_PROTOCOL_PPLIVE
  u_int32_t pplive_stage:3;			// 0-7
#endif

#ifdef NDPI_PROTOCOL_ZATTOO
  u_int32_t zattoo_stage:3;
#endif
#ifdef NDPI_PROTOCOL_QQ
  u_int32_t qq_stage:3;
#endif
#ifdef NDPI_PROTOCOL_THUNDER
  u_int32_t thunder_stage:2;		// 0-3
#endif
#ifdef NDPI_PROTOCOL_OSCAR
  u_int32_t oscar_ssl_voice_stage:3;
  u_int32_t oscar_video_voice:1;
#endif
#ifdef NDPI_PROTOCOL_FLORENSIA
  u_int32_t florensia_stage:1;
#endif

  /* internal structures to save functions calls */
  struct ndpi_packet_struct packet;
  struct ndpi_flow_struct *flow;
  struct ndpi_id_struct *src;
  struct ndpi_id_struct *dst;
} ndpi_flow_struct_t;
		     
#endif							/* __NDPI_STRUCTS_INCLUDE_FILE__ */
