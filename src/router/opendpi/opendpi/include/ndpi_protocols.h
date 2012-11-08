/*
 * ndpi_protocols.h
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


#ifndef __NDPI_PROTOCOLS_INCLUDE_FILE__
#define __NDPI_PROTOCOLS_INCLUDE_FILE__

#include "ndpi_main.h"

/* the get_uXX will return raw network packet bytes !! */
#define get_u_int8_t(X,O)  (*(u_int8_t *)(((u_int8_t *)X) + O))
#define get_u_int16_t(X,O)  (*(u_int16_t *)(((u_int8_t *)X) + O))
#define get_u_int32_t(X,O)  (*(u_int32_t *)(((u_int8_t *)X) + O))
#define get_u_int64_t(X,O)  (*(u_int64_t *)(((u_int8_t *)X) + O))

/* new definitions to get little endian from network bytes */
#define get_ul8(X,O) get_u_int8_t(X,O)


#if defined(__LITTLE_ENDIAN__)
#define get_l16(X,O)  get_u_int16_t(X,O)
#define get_l32(X,O)  get_u_int32_t(X,O)
#elif defined(__BIG_ENDIAN__)
/* convert the bytes from big to little endian */
#ifndef OPENDPI_NETFILTER_MODULE
# define get_l16(X,O) bswap_16(get_u_int16_t(X,O))
# define get_l32(X,O) bswap_32(get_u_int32_t(X,O))
#else
# define get_l16(X,O) __cpu_to_le16(get_u_int16_t(X,O))
# define get_l32(X,O) __cpu_to_le32(get_u_int32_t(X,O))
#endif

#else

#error "__BYTE_ORDER MUST BE DEFINED !"

#endif							/* __BYTE_ORDER */



/* define memory callback function */
#define ndpi_mem_cmp memcmp
static void ndpi_search_bittorrent(struct ndpi_detection_module_struct
			    *ndpi_struct, struct ndpi_flow_struct *flow);
/* edonkey entry function*/
static void ndpi_search_edonkey(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

/* fasttrack entry function*/
static void ndpi_search_fasttrack_tcp(struct ndpi_detection_module_struct
			       *ndpi_struct, struct ndpi_flow_struct *flow);

/* gnutella entry function*/
static void ndpi_search_gnutella(struct ndpi_detection_module_struct
			  *ndpi_struct, struct ndpi_flow_struct *flow);

/* winmx entry function*/
static void ndpi_search_winmx_tcp(struct ndpi_detection_module_struct
			   *ndpi_struct, struct ndpi_flow_struct *flow);

/* directconnect entry function*/
static void ndpi_search_directconnect(struct ndpi_detection_module_struct
			       *ndpi_struct, struct ndpi_flow_struct *flow);

/* applejuice entry function*/
static void ndpi_search_applejuice_tcp(struct ndpi_detection_module_struct
				*ndpi_struct, struct ndpi_flow_struct *flow);

/* i23v5 entry function */
static void ndpi_search_i23v5(struct ndpi_detection_module_struct
		       *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_socrates(struct ndpi_detection_module_struct
			  *ndpi_struct, struct ndpi_flow_struct *flow);

/* soulseek entry function*/
static void ndpi_search_soulseek_tcp(struct ndpi_detection_module_struct
			      *ndpi_struct, struct ndpi_flow_struct *flow);
/* msn entry function*/
static void ndpi_search_msn(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);

/* yahoo entry function*/
static void ndpi_search_yahoo(struct ndpi_detection_module_struct
		       *ndpi_struct, struct ndpi_flow_struct *flow);

/* oscar entry function*/
static void ndpi_search_oscar(struct ndpi_detection_module_struct
		       *ndpi_struct, struct ndpi_flow_struct *flow);

/* jabber entry function*/
static void ndpi_search_jabber_tcp(struct ndpi_detection_module_struct
			    *ndpi_struct, struct ndpi_flow_struct *flow);
/* irc entry function*/
static void ndpi_search_irc_tcp(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

/* sip entry, used for tcp and udp !!! */
static void ndpi_search_sip(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);


/* DirectDownloadLink entry */
static void ndpi_search_direct_download_link_tcp(struct
					  ndpi_detection_module_struct
					  *ndpi_struct, struct ndpi_flow_struct *flow);

/* Mail POP entry */
static void ndpi_search_mail_pop_tcp(struct ndpi_detection_module_struct
			      *ndpi_struct, struct ndpi_flow_struct *flow);

/* IMAP entry */
static void ndpi_search_mail_imap_tcp(struct ndpi_detection_module_struct
			       *ndpi_struct, struct ndpi_flow_struct *flow);

/* Mail SMTP entry */
static void ndpi_search_mail_smtp_tcp(struct ndpi_detection_module_struct
			       *ndpi_struct, struct ndpi_flow_struct *flow);

/* HTTP entry */
static void ndpi_search_http_tcp(struct ndpi_detection_module_struct
			  *ndpi_struct, struct ndpi_flow_struct *flow);

/* FTP entry */
static void ndpi_search_ftp_tcp(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

/* USENET entry */
static void ndpi_search_usenet_tcp(struct ndpi_detection_module_struct
			    *ndpi_struct, struct ndpi_flow_struct *flow);

/* DNS entry */
static void ndpi_search_dns(struct ndpi_detection_module_struct
		     *ndpi_struct, struct ndpi_flow_struct *flow);

/* RTSP entry */
static void ndpi_search_rtsp_tcp_udp(struct ndpi_detection_module_struct
			      *ndpi_struct, struct ndpi_flow_struct *flow);

/* filetopia entry */
static void ndpi_search_filetopia_tcp(struct ndpi_detection_module_struct
			       *ndpi_struct, struct ndpi_flow_struct *flow);

/* manolito entry */
static void ndpi_search_manolito_tcp_udp(struct ndpi_detection_module_struct
				  *ndpi_struct, struct ndpi_flow_struct *flow);

/* imesh entry */
static void ndpi_search_imesh_tcp_udp(struct ndpi_detection_module_struct
			       *ndpi_struct, struct ndpi_flow_struct *flow);

/* SSL entry */
static void ndpi_search_ssl_tcp(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

/* flash entry */
static void ndpi_search_flash(struct ndpi_detection_module_struct
		       *ndpi_struct, struct ndpi_flow_struct *flow);

/* mms entry */
static void ndpi_search_mms_tcp(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

/* icecast entry */
static void ndpi_search_icecast_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);

/* shoutcast entry */
static void ndpi_search_shoutcast_tcp(struct ndpi_detection_module_struct
			       *ndpi_struct, struct ndpi_flow_struct *flow);

/* veohtv entry */
static void ndpi_search_veohtv_tcp(struct ndpi_detection_module_struct
			    *ndpi_struct, struct ndpi_flow_struct *flow);
/* openft entry */
static void ndpi_search_openft_tcp(struct ndpi_detection_module_struct
			    *ndpi_struct, struct ndpi_flow_struct *flow);
/* stun entry */
static void ndpi_search_stun(struct ndpi_detection_module_struct
		      *ndpi_struct, struct ndpi_flow_struct *flow);
/* Pando entry */
static void ndpi_search_pando_tcp_udp(struct ndpi_detection_module_struct
			       *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_tvants_udp(struct ndpi_detection_module_struct
			    *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_sopcast(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_tvuplayer(struct ndpi_detection_module_struct
			   *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_ppstream(struct ndpi_detection_module_struct
			  *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_pplive_tcp_udp(struct ndpi_detection_module_struct
				*ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_iax(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_mgcp(struct ndpi_detection_module_struct
		      *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_gadugadu(struct ndpi_detection_module_struct
			  *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_zattoo(struct ndpi_detection_module_struct*ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_qq(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_feidian(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_ssh_tcp(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_popo_tcp_udp(struct ndpi_detection_module_struct
			      *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_thunder(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_activesync(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_in_non_tcp_udp(struct ndpi_detection_module_struct
				*ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_vnc_tcp(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_dhcp_udp(struct ndpi_detection_module_struct
			  *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_steam(struct ndpi_detection_module_struct
		       *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_halflife2(struct ndpi_detection_module_struct
			   *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_xbox(struct ndpi_detection_module_struct
		      *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_smb_tcp(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_telnet_tcp(struct ndpi_detection_module_struct
			    *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_ntp_udp(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_nfs(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_rtp(struct ndpi_detection_module_struct
		     *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_ssdp(struct ndpi_detection_module_struct
		      *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_worldofwarcraft(struct ndpi_detection_module_struct
				 *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_postgres_tcp(struct ndpi_detection_module_struct
			      *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_mysql_tcp(struct ndpi_detection_module_struct
			   *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_bgp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_quake(struct ndpi_detection_module_struct
		       *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_battlefield(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_secondlife(struct ndpi_detection_module_struct
			    *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_pcanywhere(struct ndpi_detection_module_struct
			    *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_rdp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_snmp(struct ndpi_detection_module_struct
		      *ndpi_struct, struct ndpi_flow_struct *flow);

static void ndpi_search_kontiki(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_syslog(struct ndpi_detection_module_struct*ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_tds_tcp(struct ndpi_detection_module_struct
			 *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_netbios(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_mdns(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_ipp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_ldap(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_warcraft3(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_kerberos(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_xdmcp(struct ndpi_detection_module_struct
		       *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_tftp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_mssql(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_pptp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_stealthnet(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_dhcpv6_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_meebo(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_afp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_aimini(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_florensia(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_maplestory(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_dofus(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_world_of_kung_fu(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_fiesta(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_crossfire_tcp_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_guildwars_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_armagetron_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_dropbox(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_skype(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_citrix(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_dcerpc(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_netflow(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_sflow(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_radius(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_wsus(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);
static void ndpi_search_teamview(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);



#endif							/* __NDPI_PROTOCOLS_INCLUDE_FILE__ */
