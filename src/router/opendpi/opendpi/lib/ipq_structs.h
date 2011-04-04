/*
 * ipq_structs.h
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


#ifndef __IPOQUE_STRUCTS_INCLUDE_FILE__
#define __IPOQUE_STRUCTS_INCLUDE_FILE__


# define MAX_PACKET_COUNTER 65000
typedef union {
	u32 ipv4;
	u8 ipv4_u8[4];
} ipq_ip_addr_t;
typedef struct ipoque_id_struct {
	/* detected_protocol_bitmask:
	 * access this bitmask to find out whether an id has used skype or not
	 * if a flag is set here, it will not be resetted
	 * to compare this, use:
	 * if (IPOQUE_BITMASK_COMPARE(id->detected_protocol_bitmask,
	 *                            IPOQUE_PROTOCOL_BITMASK_XXX) != 0)
	 * {
	 *      // protocol XXX detected on this id
	 * }
	 */
	IPOQUE_PROTOCOL_BITMASK detected_protocol_bitmask;
#ifdef IPOQUE_PROTOCOL_EDONKEY
#endif
#ifdef IPOQUE_PROTOCOL_PPLIVE
	IPOQUE_TIMESTAMP_COUNTER_SIZE pplive_last_packet_time;
#endif
#ifdef IPOQUE_PROTOCOL_YAHOO
	u32 sip_relay_node;
	IPOQUE_TIMESTAMP_COUNTER_SIZE yahoo_video_lan_timer;
#endif
#ifdef IPOQUE_PROTOCOL_IRC
	IPOQUE_TIMESTAMP_COUNTER_SIZE last_time_port_used[16];
#endif
#ifdef IPOQUE_PROTOCOL_FTP
	ipq_ip_addr_t ftp_ip;
	IPOQUE_TIMESTAMP_COUNTER_SIZE ftp_timer;
#endif
#ifdef IPOQUE_PROTOCOL_IMESH
	IPOQUE_TIMESTAMP_COUNTER_SIZE imesh_timer;
#endif
#ifdef IPOQUE_PROTOCOL_IRC
	IPOQUE_TIMESTAMP_COUNTER_SIZE irc_ts;
#endif
#ifdef IPOQUE_PROTOCOL_GNUTELLA
	IPOQUE_TIMESTAMP_COUNTER_SIZE gnutella_ts;
#endif
#ifdef IPOQUE_PROTOCOL_BATTLEFIELD
	IPOQUE_TIMESTAMP_COUNTER_SIZE battlefield_ts;
#endif
#ifdef IPOQUE_PROTOCOL_THUNDER
	IPOQUE_TIMESTAMP_COUNTER_SIZE thunder_ts;
#endif
#ifdef IPOQUE_PROTOCOL_RTSP
	IPOQUE_TIMESTAMP_COUNTER_SIZE rtsp_timer;
	ipq_ip_addr_t rtsp_ip_address;
#endif
#ifdef IPOQUE_PROTOCOL_OSCAR
//  IPOQUE_TIMESTAMP_COUNTER_SIZE oscar_timer;
	IPOQUE_TIMESTAMP_COUNTER_SIZE oscar_last_safe_access_time;
	IPOQUE_TIMESTAMP_COUNTER_SIZE oscar_voice_timestamp[8];
#endif
#ifdef IPOQUE_PROTOCOL_GADUGADU
	u32 gg_ft_ip_address;
	IPOQUE_TIMESTAMP_COUNTER_SIZE gg_timeout;
#endif

#ifdef IPOQUE_PROTOCOL_ZATTOO
	IPOQUE_TIMESTAMP_COUNTER_SIZE zattoo_ts;
#endif
#ifdef IPOQUE_PROTOCOL_UNENCRYPED_JABBER
	IPOQUE_TIMESTAMP_COUNTER_SIZE jabber_stun_or_ft_ts;
#endif
#ifdef IPOQUE_PROTOCOL_MANOLITO
	u32 manolito_last_pkt_arrival_time;
#endif
#ifdef IPOQUE_PROTOCOL_DIRECTCONNECT
	IPOQUE_TIMESTAMP_COUNTER_SIZE directconnect_last_safe_access_time;
#endif
#ifdef IPOQUE_PROTOCOL_SOULSEEK
	IPOQUE_TIMESTAMP_COUNTER_SIZE soulseek_last_safe_access_time;
#endif
#ifdef IPOQUE_PROTOCOL_DIRECTCONNECT
	u16 detected_directconnect_port;
	u16 detected_directconnect_udp_port;
	u16 detected_directconnect_ssl_port;
#endif
#ifdef IPOQUE_PROTOCOL_PPLIVE
	u16 pplive_vod_cli_port;
#endif
#ifdef IPOQUE_PROTOCOL_IRC
	u16 irc_port[16];
#endif
#ifdef IPOQUE_PROTOCOL_GADUGADU
	u16 gg_ft_port;
#endif
#ifdef IPOQUE_PROTOCOL_UNENCRYPED_JABBER
#define JABBER_MAX_STUN_PORTS 6
	u16 jabber_voice_stun_port[JABBER_MAX_STUN_PORTS];
	u16 jabber_file_transfer_port;
#endif
#ifdef IPOQUE_PROTOCOL_GNUTELLA
	u16 detected_gnutella_port;
#endif
#ifdef IPOQUE_PROTOCOL_GNUTELLA
	u16 detected_gnutella_udp_port1;
	u16 detected_gnutella_udp_port2;
#endif
#ifdef IPOQUE_PROTOCOL_SOULSEEK
	u16 soulseek_listen_port;
#endif
#ifdef IPOQUE_PROTOCOL_IRC
	u8 irc_number_of_port;
#endif
#ifdef IPOQUE_PROTOCOL_OSCAR
	u8 oscar_ssl_session_id[33];
	u8 oscar_number_of_voice_port;
#endif
#ifdef IPOQUE_PROTOCOL_GADUGADU
	u8 gg_call_id[2][7];
	u8 gg_fmnumber[8];
#endif
#ifdef IPOQUE_PROTOCOL_UNENCRYPED_JABBER
	u8 jabber_voice_stun_used_ports;
#endif
#ifdef IPOQUE_PROTOCOL_YAHOO
	u32 yahoo_video_lan_dir:1;
#endif
#ifdef IPOQUE_PROTOCOL_YAHOO
	u32 yahoo_conf_logged_in:1;
	u32 yahoo_voice_conf_logged_in:1;
#endif
#ifdef IPOQUE_PROTOCOL_FTP
	u32 ftp_timer_set:1;
#endif
#ifdef IPOQUE_PROTOCOL_GADUGADU
	u32 gadu_gadu_ft_direction:1;
	u32 gadu_gadu_voice:1;
	u32 gg_next_id:1;
#endif
#ifdef IPOQUE_PROTOCOL_RTSP
	u32 rtsp_ts_set:1;
#endif
#ifdef IPOQUE_PROTOCOL_PPLIVE
	u32 pplive_last_packet_time_set:1;
#endif
} ipoque_id_struct;
typedef struct ipoque_flow_struct {


/* ALL 64 bit variables here */

	/* protocols which have marked a connection as this connection cannot be protocol XXX, multiple u64 */
	IPOQUE_PROTOCOL_BITMASK excluded_protocol_bitmask;
	u32 detected_protocol;


#ifdef IPOQUE_PROTOCOL_RTP
	u32 rtp_ssid[2];
#endif
	/* tcp sequence number connection tracking */
	u32 next_tcp_seq_nr[2];
#ifdef IPOQUE_PROTOCOL_BATTLEFIELD
	u32 battlefield_msg_id;
#endif
#ifdef IPOQUE_PROTOCOL_SNMP
	u32 snmp_msg_id;
#endif
#ifdef IPOQUE_PROTOCOL_I23V5
	u32 i23v5_len1;
	u32 i23v5_len2;
	u32 i23v5_len3;
#endif
#ifdef IPOQUE_PROTOCOL_DIRECT_DOWNLOAD_LINK
	u32 hash_id_number;
#endif							// IPOQUE_PROTOCOL_DIRECT_DOWNLOAD_LINK
	/* Count Of Number of payloaded Packets in the Flow */
	u16 packet_counter;			// can be 0-65000
	u16 packet_direction_counter[2];
#ifdef IPOQUE_PROTOCOL_FLASH
	u16 flash_bytes;
#endif
#ifdef IPOQUE_PROTOCOL_RTP
	u16 rtp_seqnum[2];
#endif
#ifdef IPOQUE_PROTOCOL_MAIL_SMTP
	u16 smtp_command_bitmask;
#endif
#ifdef IPOQUE_PROTOCOL_MAIL_POP
	u16 pop_command_bitmask;
#endif

	u8 protocol_subtype;		// protocol subtype fro various protocols
#ifdef IPOQUE_PROTOCOL_RTP
	u8 rtp_payload_type;
#endif
#ifdef IPOQUE_PROTOCOL_TDS
	u8 tds_login_version;
#endif
#ifdef IPOQUE_PROTOCOL_PPLIVE
	u8 pplive_next_packet_size[2];
#endif
#ifdef IPOQUE_PROTOCOL_BITTORRENT
	u8 bittorrent_stage;		// can be 0-255
#endif
#ifdef IPOQUE_PROTOCOL_IRC
	u8 irc_stage;
	u8 irc_port;
#endif
#ifdef IPOQUE_PROTOCOL_GNUTELLA
	u8 gnutella_msg_id[3];
#endif
	/* init parameter, internal used to set up timestamp,... */
	u32 init_finished:1;
	u32 setup_packet_direction:1;
#ifdef IPOQUE_PROTOCOL_IRC
	u32 irc_ssl_min_number_of_packet:4;
	u32 irc_3a_counter:3;
	u32 irc_stage2:5;
	u32 irc_direction:2;
	u32 irc_0x1000_full:1;
#endif
#ifdef IPOQUE_PROTOCOL_RTP
	u32 rtp_stage1:2;			//0-3
	u32 rtp_stage2:2;
#endif
#ifdef IPOQUE_PROTOCOL_EDONKEY
	u32 edk_stage:5;			// 0-17
	u32 edk_ext:1;
	u32 edonkey_added:1;
#endif
#ifdef IPOQUE_PROTOCOL_WINMX
	u32 winmx_stage:1;			// 0-1
#endif
#ifdef IPOQUE_PROTOCOL_DIRECTCONNECT
	u32 directconnect_stage:2;	// 0-1
#endif
#ifdef IPOQUE_PROTOCOL_SOULSEEK
	u32 soulseek_stage:2;
#endif
#ifdef IPOQUE_PROTOCOL_SNMP
	u32 snmp_stage:2;
#endif
#ifdef IPOQUE_PROTOCOL_FILETOPIA
	u32 filetopia_stage:2;
#endif
#ifdef IPOQUE_PROTOCOL_MANOLITO
	u32 manolito_stage:4;
#endif
#ifdef IPOQUE_PROTOCOL_BATTLEFIELD
	u32 battlefield_stage:2;
#endif
#ifdef IPOQUE_PROTOCOL_YAHOO
	u32 sip_yahoo_voice:1;
#endif
#ifdef IPOQUE_PROTOCOL_TDS
	u32 tds_stage:3;
#endif
#ifdef IPOQUE_PROTOCOL_GADUGADU
	u32 gadugadu_stage:2;
#endif
#ifdef IPOQUE_PROTOCOL_USENET
	u32 usenet_stage:2;
#endif
#ifdef IPOQUE_PROTOCOL_IMESH
	u32 imesh_stage:4;
#endif
#ifdef IPOQUE_PROTOCOL_FTP
	u32 ftp_stage:3;
#endif

#ifdef IPOQUE_PROTOCOL_HTTP
	u32 http_setup_dir:2;
	u32 http_stage:2;
	u32 server_direction:1;
	u32 http_empty_line_seen:1;
	u32 http_detected:1;
#endif							// IPOQUE_PROTOCOL_HTTP
#ifdef IPOQUE_PROTOCOL_FLASH
	u32 flash_stage:3;
#endif
#ifdef IPOQUE_PROTOCOL_GNUTELLA
	u32 gnutella_stage:2;		//0-2
#endif
#ifdef IPOQUE_PROTOCOL_MMS
	u32 mms_stage:2;
#endif
#ifdef IPOQUE_PROTOCOL_RTSP
	u32 rtsprdt_stage:2;
	u32 rtsp_control_flow:1;
#endif

#ifdef IPOQUE_PROTOCOL_YAHOO
	u32 yahoo_detection_finished:2;
#ifdef IPOQUE_PROTOCOL_HTTP
	u32 yahoo_http_proxy_stage:2;
#endif
#endif
#ifdef IPOQUE_PROTOCOL_PPLIVE
	u32 pplive_stage:3;			// 0-7
#endif
#ifdef IPOQUE_PROTOCOL_MSN
	u32 msn_stage:3;
	u32 msn_ssl_ft:2;
#endif

#ifdef IPOQUE_PROTOCOL_ZATTOO
	u32 zattoo_stage:3;
#endif
#ifdef IPOQUE_PROTOCOL_PPSTREAM
	u32 ppstream_stage:3;		// 0-2
#endif
#ifdef IPOQUE_PROTOCOL_FEIDIAN
	u32 feidian_stage:1;		// 0-7
#endif
#ifdef IPOQUE_PROTOCOL_QQ
	u32 qq_ssl_stage:4;
	u32 qq_stage:4;
#endif
#ifdef IPOQUE_PROTOCOL_SSH
	u32 ssh_stage:3;
#endif
#ifdef IPOQUE_PROTOCOL_THUNDER
	u32 thunder_stage:2;		// 0-3
#endif

#ifdef IPOQUE_PROTOCOL_OSCAR
	u32 oscar_stage:4;			// 0 - 15
	u32 oscar_cool:1;
	u32 oscar_stun:1;
	u32 oscar_ssl_voice_stage:3;
	u32 oscar_video_voice:1;
#endif

#ifdef IPOQUE_PROTOCOL_VNC
	u32 vnc_stage:2;			// 0 - 3
#endif

#ifdef IPOQUE_PROTOCOL_STEAM
	u32 steam_stage:2;			// 0 - 3
#endif

#ifdef IPOQUE_PROTOCOL_TELNET
	u32 telnet_stage:2;			// 0 - 2
#endif

#ifdef IPOQUE_PROTOCOL_HALFLIFE2
	u32 halflife2_stage:2;		// 0 - 2
#endif

#ifdef IPOQUE_PROTOCOL_SSL
	u32 ssl_stage:2;			// 0 - 2
#endif
#ifdef IPOQUE_PROTOCOL_POSTGRES
	u32 postgres_stage:3;
#endif

#ifdef IPOQUE_PROTOCOL_DIRECT_DOWNLOAD_LINK
	u32 ddlink_server_direction:1;
#endif
#ifdef IPOQUE_PROTOCOL_MAIL_POP
	u32 mail_pop_stage:2;
#endif
#ifdef IPOQUE_PROTOCOL_MAIL_IMAP
	u32 mail_imap_stage:2;
#endif
#ifdef IPOQUE_PROTOCOL_TFTP
	u32 tftp_stage:1;
#endif
#ifdef IPOQUE_PROTOCOL_AIMINI
	u32 aimini_stage:5;
#endif
#ifdef IPOQUE_PROTOCOL_XBOX
	u32 xbox_stage:1;
#endif
} ipoque_flow_struct_t;
#endif							/* __IPOQUE_STRUCTS_INCLUDE_FILE__ */
