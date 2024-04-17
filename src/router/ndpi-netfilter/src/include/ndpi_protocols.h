/*
 * ndpi_protocols.h
 *
 * Copyright (C) 2011-22 - ntop.org
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef __NDPI_PROTOCOLS_H__
#define __NDPI_PROTOCOLS_H__

#include "ndpi_main.h"


NDPI_STATIC ndpi_port_range* ndpi_build_default_ports_range(ndpi_port_range *ports,
						u_int16_t portA_low, u_int16_t portA_high,
						u_int16_t portB_low, u_int16_t portB_high,
						u_int16_t portC_low, u_int16_t portC_high,
						u_int16_t portD_low, u_int16_t portD_high,
						u_int16_t portE_low, u_int16_t portE_high);

NDPI_STATIC ndpi_port_range* ndpi_build_default_ports(ndpi_port_range *ports,
					  u_int16_t portA,
					  u_int16_t portB,
					  u_int16_t portC,
					  u_int16_t portD,
					  u_int16_t portE);

/* TCP/UDP protocols */
#ifdef __cplusplus
extern "C"
#endif
NDPI_STATIC u_int ndpi_search_tcp_or_udp_raw(struct ndpi_detection_module_struct *ndpi_struct,
				 struct ndpi_flow_struct *flow,
				 u_int32_t saddr, u_int32_t daddr);

NDPI_STATIC void ndpi_bittorrent_init(struct ndpi_detection_module_struct *ndpi_struct,
                               u_int32_t size,u_int32_t size6,u_int32_t tmo,int logsize);
NDPI_STATIC void ndpi_bittorrent_done(struct ndpi_detection_module_struct *ndpi_struct);
NDPI_STATIC int  ndpi_bittorrent_gc(struct hash_ip4p_table *ht,int key,time_t now);
/* Applications and other protocols. */
NDPI_STATIC void ndpi_search_diameter(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_bittorrent(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_lisp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_edonkey(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_fasttrack_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_gnutella(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_directconnect(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_applejuice_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_i23v5(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_socrates(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_soulseek_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_msn(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_yahoo(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_oscar(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_jabber_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_irc_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_sip(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_hep(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_direct_download_link_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mail_pop_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mail_imap_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mail_smtp_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_http_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_http_subprotocol_conf(struct ndpi_detection_module_struct *ndpi_struct, char *attr, char *value, int protocol_id);
NDPI_STATIC void ndpi_search_ftp_control(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ftp_data(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_usenet_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_dns(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_rtsp_tcp_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_filetopia_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_vmware(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ssl_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mms_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_icecast_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_shoutcast_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_veohtv_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_openft_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_stun(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_tvants_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_sopcast(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_tvuplayer(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ppstream(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_pplive(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_iax(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mgcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_zattoo(struct ndpi_detection_module_struct*ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_qq(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_feidian(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ssh_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ayiya(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_thunder(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_activesync(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_in_non_tcp_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_vnc_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_dhcp_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_steam(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_halflife2(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_xbox(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_smb_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_telnet_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ntp_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_nfs(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_rtp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ssdp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_worldofwarcraft(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_postgres_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mysql_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_bgp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_quake(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_battlefield(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_secondlife(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_pcanywhere(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_rdp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_snmp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_kontiki(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_syslog(struct ndpi_detection_module_struct*ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_netbios(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mdns(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ipp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ldap(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_warcraft3(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_kerberos(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_xdmcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_tftp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mssql_tds(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_pptp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_stealthnet(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_dhcpv6_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_afp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_checkmk(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_aimini(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_florensia(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_maplestory(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_dofus(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_world_of_kung_fu(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_fiesta(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_crossfire_tcp_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_guildwars_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_armagetron_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_dropbox(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_citrix(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_dcerpc(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_netflow(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_sflow(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_radius(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_wsus(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_teamview(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_lotus_notes(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_gtp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_spotify(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_h323(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_openvpn(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_noe(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ciscovpn(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_viber(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_teamspeak(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_corba(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_collectd(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_oracle(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_rsync(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_rtcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_skinny(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_tor(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_whois_das(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_socks5(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_socks4(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_rtmp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_pando(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_megaco(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_redis(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_zmq(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_vhua(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_telegram(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_quic(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_eaq(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_kakaotalk_voice(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mpegts(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_starcraft(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ubntac2(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_coap(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mqtt (struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_someip (struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_rx(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_git(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_drda(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_bjnp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_smpp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_tinc(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_fix(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_csgo(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ajp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_memcached(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_nest_log_sink(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_amazon_video(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_targus_getdata(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_wireguard(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);


NDPI_STATIC void ndpi_search_among_us(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_amqp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_apple_push(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_capwap(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_cassandra(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_cpha(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC int ndpi_search_dht_again(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_dnp3_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_dnscrypt(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ethernet_ip(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_hangout(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_hsrp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_i3d(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_iec60870_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_imo(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ipsec(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC u_int8_t  ndpi_search_irc_ssl_detect_ninety_percent_but_very_fast(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mining(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_modbus_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mongodb(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_mpegdash_http(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_nats_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_nintendo(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ookla(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_raknet(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_riotgames(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_rsh(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_s7comm_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_sd_rtn(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_skype(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_smpp_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_soap(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_socks(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_teredo(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC int ndpi_search_tls_tcp_memory(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_toca_boca(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_ultrasurf(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_vxlan(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_websocket(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_whatsapp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_wsd(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_zabbix(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_discord(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_avast(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_avast_securedns(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_alicloud(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_activision(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_crynet(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_elasticsearch(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_line(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_munin(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_natpmp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_softether(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_syncthing(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_tivoconnect(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_kismet(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_fastcgi(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_hots(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
NDPI_STATIC void ndpi_search_http2(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);



NDPI_STATIC void init_diameter_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_afp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_armagetron_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_amqp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_bgp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_bittorrent_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC u_int32_t ndpi_bittorrent_hash_funct(u_int32_t ip, u_int16_t port);
NDPI_STATIC void init_lisp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_teredo_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ciscovpn_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_citrix_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_corba_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_crossfire_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_dcerpc_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_dhcp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_dhcpv6_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_dns_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_dofus_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_dropbox_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_eaq_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_edonkey_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ftp_control_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ftp_data_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_gnutella_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_gtp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_hsrp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_guildwars_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_h323_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_halflife2_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_hots_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_http_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_iax_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_icecast_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ipp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_irc_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_jabber_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_kakaotalk_voice_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_kerberos_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_kontiki_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ldap_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_lotus_notes_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mail_imap_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mail_pop_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mail_smtp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_maplestory_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_megaco_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mgcp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mining_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mms_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_nats_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mpegts_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mssql_tds_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mysql_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_netbios_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_netflow_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_nfs_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_noe_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_non_tcp_udp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ntp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_openvpn_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_oracle_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_postgres_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ppstream_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_pptp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_qq_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_quake_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_quic_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_radius_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_rdp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_redis_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_rsync_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_rtcp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_rtmp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_rtp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_rtsp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_sflow_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_shoutcast_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_sip_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_imo_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_skinny_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_skype_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_smb_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_snmp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_socrates_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_socks_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_spotify_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ssh_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_tls_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_starcraft_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_steam_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_stun_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_syslog_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ssdp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_teamspeak_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_teamviewer_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_telegram_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_telnet_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_tftp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_tvuplayer_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_usenet_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_wsd_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_veohtv_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_vhua_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_viber_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_vmware_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_vnc_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_vxlan_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_warcraft3_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_whois_das_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_world_of_warcraft_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_world_of_kung_fu_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_xbox_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_xdmcp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_zattoo_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_zmq_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_stracraft_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ubntac2_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_coap_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mqtt_dissector (struct ndpi_detection_module_struct *ndpi_struct,u_int32_t *id);
NDPI_STATIC void init_someip_dissector (struct ndpi_detection_module_struct *ndpi_struct,u_int32_t *id);
NDPI_STATIC void init_rx_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_git_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_drda_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_bjnp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_smpp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_tinc_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_fix_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_nintendo_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_csgo_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_checkmk_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_cpha_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_apple_push_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_amazon_video_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_whatsapp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ajp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_memcached_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_nest_log_sink_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ookla_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_modbus_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_capwap_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_zabbix_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_wireguard_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_dnp3_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_104_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_s7comm_dissector(struct ndpi_detection_module_struct *ndpi_struct,u_int32_t *id);
NDPI_STATIC void init_websocket_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_soap_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_dnscrypt_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mongodb_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_among_us_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_hpvirtgrp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_genshin_impact_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_z3950_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_avast_securedns_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_cassandra_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ethernet_ip_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_toca_boca_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_sd_rtn_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_raknet_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_xiaomi_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_mpegdash_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_rsh_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ipsec_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_collectd_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_i3d_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_riotgames_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_ultrasurf_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_threema_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_alicloud_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_avast_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_softether_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_activision_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_discord_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_tivoconnect_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_kismet_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_fastcgi_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_natpmp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_syncthing_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_crynet_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_line_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_munin_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_elasticsearch_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_tuya_lp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_tplink_shp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_merakicloud_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_tailscale_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_source_engine_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_bacnet_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_oicq_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_epicgames_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_bitcoin_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_apache_thrift_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_slp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_http2_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);
NDPI_STATIC void init_haproxy_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id);

/* ndpi_main.c */
NDPI_STATIC  u_int32_t ndpi_ip_port_hash_funct(u_int32_t ip, u_int16_t port);

#endif /* __NDPI_PROTOCOLS_H__ */
