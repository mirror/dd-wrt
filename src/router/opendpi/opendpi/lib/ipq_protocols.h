/*
 * ipq_protocols.h
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


#ifndef __IPOQUE_PROTOCOLS_INCLUDE_FILE__
#define __IPOQUE_PROTOCOLS_INCLUDE_FILE__
#include "ipq_main.h"
/* the get_uXX will return raw network packet bytes !! */
#define get_u8(X,O)  (*(u8 *)(((u8 *)X) + O))
#define get_u16(X,O)  (*(u16 *)(((u8 *)X) + O))
#define get_u32(X,O)  (*(u32 *)(((u8 *)X) + O))
#define get_u64(X,O)  (*(u64 *)(((u8 *)X) + O))

/* new definitions to get little endian from network bytes */
#define get_ul8(X,O) get_u8(X,O)

#ifndef OPENDPI_NETFILTER_MODULE
# ifndef __BYTE_ORDER
#  define __BYTE_ORDER BYTE_ORDER
#  define __LITTLE_ENDIAN LITTLE_ENDIAN
#  define __BIG_ENDIAN BIG_ENDIAN
# endif
#else
# ifdef __BIG_ENDIAN
#  define __BYTE_ORDER __BIG_ENDIAN
# else
#  define __BYTE_ORDER __LITTLE_ENDIAN
# endif
#endif


#if defined( __LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN

#define get_l16(X,O)  get_u16(X,O)
#define get_l32(X,O)  get_u32(X,O)

#elif defined( __BIG_ENDIAN) && __BYTE_ORDER == __BIG_ENDIAN

/* convert the bytes from big to little endian */
#ifndef OPENDPI_NETFILTER_MODULE
# define get_l16(X,O) bswap_16(get_u16(X,O))
# define get_l32(X,O) bswap_32(get_u32(X,O))
#else
# define get_l16(X,O) __cpu_to_le16(get_u16(X,O))
# define get_l32(X,O) __cpu_to_le32(get_u32(X,O))
#endif

#else

#error "__BYTE_ORDER MUST BE DEFINED !"

#endif							/* __BYTE_ORDER */



/* define memory callback function */
#define ipq_mem_cmp memcmp
void ipoque_search_bittorrent(struct ipoque_detection_module_struct
							  *ipoque_struct);
/* edonkey entry function*/
void ipoque_search_edonkey(struct ipoque_detection_module_struct
						   *ipoque_struct);

/* fasttrack entry function*/
void ipoque_search_fasttrack_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct);

/* gnutella entry function*/
void ipoque_search_gnutella(struct ipoque_detection_module_struct
							*ipoque_struct);

/* winmx entry function*/
void ipoque_search_winmx_tcp(struct ipoque_detection_module_struct
							 *ipoque_struct);

/* directconnect entry function*/
void ipoque_search_directconnect(struct ipoque_detection_module_struct
								 *ipoque_struct);

/* applejuice entry function*/
void ipoque_search_applejuice_tcp(struct ipoque_detection_module_struct
								  *ipoque_struct);

/* i23v5 entry function */
void ipoque_search_i23v5(struct ipoque_detection_module_struct
						 *ipoque_struct);

void ipoque_search_socrates(struct ipoque_detection_module_struct
							*ipoque_struct);

/* soulseek entry function*/
void ipoque_search_soulseek_tcp(struct ipoque_detection_module_struct
								*ipoque_struct);
/* msn entry function*/
void ipoque_search_msn(struct ipoque_detection_module_struct *ipoque_struct);

/* yahoo entry function*/
void ipoque_search_yahoo(struct ipoque_detection_module_struct
						 *ipoque_struct);

/* oscar entry function*/
void ipoque_search_oscar(struct ipoque_detection_module_struct
						 *ipoque_struct);

/* jabber entry function*/
void ipoque_search_jabber_tcp(struct ipoque_detection_module_struct
							  *ipoque_struct);
/* irc entry function*/
void ipoque_search_irc_tcp(struct ipoque_detection_module_struct
						   *ipoque_struct);

/* sip entry, used for tcp and udp !!! */
void ipoque_search_sip(struct ipoque_detection_module_struct *ipoque_struct);


/* DirectDownloadLink entry */
void ipoque_search_direct_download_link_tcp(struct
											ipoque_detection_module_struct
											*ipoque_struct);

/* Mail POP entry */
void ipoque_search_mail_pop_tcp(struct ipoque_detection_module_struct
								*ipoque_struct);

/* IMAP entry */
void ipoque_search_mail_imap_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct);

/* Mail SMTP entry */
void ipoque_search_mail_smtp_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct);

/* HTTP entry */
void ipoque_search_http_tcp(struct ipoque_detection_module_struct
							*ipoque_struct);

/* FTP entry */
void ipoque_search_ftp_tcp(struct ipoque_detection_module_struct
						   *ipoque_struct);

/* USENET entry */
void ipoque_search_usenet_tcp(struct ipoque_detection_module_struct
							  *ipoque_struct);

/* DNS entry */
void ipoque_search_dns(struct ipoque_detection_module_struct
					   *ipoque_struct);

/* RTSP entry */
void ipoque_search_rtsp_tcp_udp(struct ipoque_detection_module_struct
								*ipoque_struct);

/* filetopia entry */
void ipoque_search_filetopia_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct);

/* manolito entry */
void ipoque_search_manolito_tcp_udp(struct ipoque_detection_module_struct
									*ipoque_struct);

/* imesh entry */
void ipoque_search_imesh_tcp_udp(struct ipoque_detection_module_struct
								 *ipoque_struct);

/* SSL entry */
void ipoque_search_ssl_tcp(struct ipoque_detection_module_struct
						   *ipoque_struct);

/* flash entry */
void ipoque_search_flash(struct ipoque_detection_module_struct
						 *ipoque_struct);

/* mms entry */
void ipoque_search_mms_tcp(struct ipoque_detection_module_struct
						   *ipoque_struct);

/* icecast entry */
void ipoque_search_icecast_tcp(struct ipoque_detection_module_struct
							   *ipoque_struct);

/* shoutcast entry */
void ipoque_search_shoutcast_tcp(struct ipoque_detection_module_struct
								 *ipoque_struct);

/* veohtv entry */
void ipoque_search_veohtv_tcp(struct ipoque_detection_module_struct
							  *ipoque_struct);
/* openft entry */
void ipoque_search_openft_tcp(struct ipoque_detection_module_struct
							  *ipoque_struct);
/* stun entry */
void ipoque_search_stun(struct ipoque_detection_module_struct
						*ipoque_struct);
/* Pando entry */
void ipoque_search_pando_tcp_udp(struct ipoque_detection_module_struct
								 *ipoque_struct);
void ipoque_search_tvants_udp(struct ipoque_detection_module_struct
							  *ipoque_struct);

void ipoque_search_sopcast(struct ipoque_detection_module_struct
						   *ipoque_struct);

void ipoque_search_tvuplayer(struct ipoque_detection_module_struct
							 *ipoque_struct);

void ipoque_search_ppstream(struct ipoque_detection_module_struct
							*ipoque_struct);

void ipoque_search_pplive_tcp_udp(struct ipoque_detection_module_struct
								  *ipoque_struct);

void ipoque_search_iax(struct ipoque_detection_module_struct *ipoque_struct);
void ipoque_search_mgcp(struct ipoque_detection_module_struct
						*ipoque_struct);

void ipoque_search_gadugadu(struct ipoque_detection_module_struct
							*ipoque_struct);

void ipoque_search_zattoo(struct ipoque_detection_module_struct
						  *ipoque_struct);
void ipoque_search_qq(struct ipoque_detection_module_struct *ipoque_struct);
void ipoque_search_feidian(struct ipoque_detection_module_struct
						   *ipoque_struct);

void ipoque_search_ssh_tcp(struct ipoque_detection_module_struct
						   *ipoque_struct);

void ipoque_search_popo_tcp_udp(struct ipoque_detection_module_struct
								*ipoque_struct);

void ipoque_search_thunder(struct ipoque_detection_module_struct
						   *ipoque_struct);
void ipoque_search_activesync(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_in_non_tcp_udp(struct ipoque_detection_module_struct
								  *ipoque_struct);

void ipoque_search_vnc_tcp(struct ipoque_detection_module_struct
						   *ipoque_struct);

void ipoque_search_dhcp_udp(struct ipoque_detection_module_struct
							*ipoque_struct);

void ipoque_search_steam(struct ipoque_detection_module_struct
						 *ipoque_struct);

void ipoque_search_halflife2(struct ipoque_detection_module_struct
							 *ipoque_struct);

void ipoque_search_xbox(struct ipoque_detection_module_struct
						*ipoque_struct);
void ipoque_search_smb_tcp(struct ipoque_detection_module_struct
						   *ipoque_struct);

void ipoque_search_telnet_tcp(struct ipoque_detection_module_struct
							  *ipoque_struct);

void ipoque_search_ntp_udp(struct ipoque_detection_module_struct
						   *ipoque_struct);

void ipoque_search_nfs(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_rtp(struct ipoque_detection_module_struct
					   *ipoque_struct);
void ipoque_search_ssdp(struct ipoque_detection_module_struct
						*ipoque_struct);

void ipoque_search_worldofwarcraft(struct ipoque_detection_module_struct
								   *ipoque_struct);
void ipoque_search_postgres_tcp(struct ipoque_detection_module_struct
								*ipoque_struct);

void ipoque_search_mysql_tcp(struct ipoque_detection_module_struct
							 *ipoque_struct);

void ipoque_search_bgp(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_quake(struct ipoque_detection_module_struct
						 *ipoque_struct);

void ipoque_search_battlefield(struct ipoque_detection_module_struct
							   *ipoque_struct);

void ipoque_search_secondlife(struct ipoque_detection_module_struct
							  *ipoque_struct);

void ipoque_search_pcanywhere(struct ipoque_detection_module_struct
							  *ipoque_struct);

void ipoque_search_rdp(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_snmp(struct ipoque_detection_module_struct
						*ipoque_struct);

void ipoque_search_kontiki(struct ipoque_detection_module_struct
						   *ipoque_struct);

void ipoque_search_syslog(struct ipoque_detection_module_struct
						  *ipoque_struct);

void ipoque_search_tds_tcp(struct ipoque_detection_module_struct
						   *ipoque_struct);

void ipoque_search_netbios(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_mdns(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_ipp(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_ldap(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_warcraft3(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_kerberos(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_xdmcp(struct ipoque_detection_module_struct
						 *ipoque_struct);
void ipoque_search_tftp(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_mssql(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_pptp(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_stealthnet(struct ipoque_detection_module_struct *ipoque_struct);
void ipoque_search_dhcpv6_udp(struct ipoque_detection_module_struct *ipoque_struct);
void ipoque_search_meebo(struct ipoque_detection_module_struct *ipoque_struct);
void ipoque_search_afp(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_aimini(struct ipoque_detection_module_struct *ipoque_struct);
void ipoque_search_florensia(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_maplestory(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_dofus(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_world_of_kung_fu(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_fiesta(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_crossfire_tcp_udp(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_guildwars_tcp(struct ipoque_detection_module_struct *ipoque_struct);

void ipoque_search_armagetron_udp(struct ipoque_detection_module_struct *ipoque_struct);

#endif							/* __IPOQUE_PROTOCOLS_INCLUDE_FILE__ */
