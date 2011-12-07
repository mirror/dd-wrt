/*
 * ipq_protocols_osdpi.h
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


#ifndef __IPOQUE_API_INCLUDE_FILE__

#endif

#ifndef __IPQ_PROTOCOLS_DEFAULT_H__
#define __IPQ_PROTOCOLS_DEFAULT_H__

#ifdef __cplusplus
extern "C" {
#endif

//  #define IPOQUE_ENABLE_DEBUG_MESSAGES

#define IPOQUE_DETECTION_SUPPORT_IPV6
#define IPOQUE_PROTOCOL_HISTORY_SIZE 3

#define IPOQUE_PROTOCOL_UNKNOWN                 0
#define IPOQUE_PROTOCOL_FTP                     1
#define IPOQUE_PROTOCOL_MAIL_POP                2
#define IPOQUE_PROTOCOL_MAIL_SMTP               3
#define IPOQUE_PROTOCOL_MAIL_IMAP               4
#define IPOQUE_PROTOCOL_DNS              		5
#define IPOQUE_PROTOCOL_IPP						6
#define IPOQUE_PROTOCOL_HTTP					7
#define IPOQUE_PROTOCOL_MDNS					8
#define IPOQUE_PROTOCOL_NTP						9
#define IPOQUE_PROTOCOL_NETBIOS					10
#define IPOQUE_PROTOCOL_NFS						11
#define IPOQUE_PROTOCOL_SSDP					12
#define IPOQUE_PROTOCOL_BGP						13
#define IPOQUE_PROTOCOL_SNMP					14
#define IPOQUE_PROTOCOL_XDMCP					15
#define IPOQUE_PROTOCOL_SMB						16
#define IPOQUE_PROTOCOL_SYSLOG					17
#define IPOQUE_PROTOCOL_DHCP					18
#define IPOQUE_PROTOCOL_POSTGRES				19
#define IPOQUE_PROTOCOL_MYSQL					20
#define IPOQUE_PROTOCOL_TDS						21
#define IPOQUE_PROTOCOL_DIRECT_DOWNLOAD_LINK	22
#define IPOQUE_PROTOCOL_I23V5					23
#define IPOQUE_PROTOCOL_APPLEJUICE				24
#define IPOQUE_PROTOCOL_DIRECTCONNECT			25
#define IPOQUE_PROTOCOL_SOCRATES				26
#define IPOQUE_PROTOCOL_WINMX					27
#define IPOQUE_PROTOCOL_MANOLITO				28
#define IPOQUE_PROTOCOL_PANDO					29
#define IPOQUE_PROTOCOL_FILETOPIA				30
#define IPOQUE_PROTOCOL_IMESH					31
#define IPOQUE_PROTOCOL_KONTIKI					32
#define IPOQUE_PROTOCOL_OPENFT					33
#define IPOQUE_PROTOCOL_FASTTRACK				34
#define IPOQUE_PROTOCOL_GNUTELLA				35
#define IPOQUE_PROTOCOL_EDONKEY					36
#define IPOQUE_PROTOCOL_BITTORRENT				37
#define IPOQUE_PROTOCOL_OFF						38
#define IPOQUE_PROTOCOL_AVI						39
#define IPOQUE_PROTOCOL_FLASH					40
#define IPOQUE_PROTOCOL_OGG						41
#define	IPOQUE_PROTOCOL_MPEG					42
#define	IPOQUE_PROTOCOL_QUICKTIME				43
#define	IPOQUE_PROTOCOL_REALMEDIA				44
#define	IPOQUE_PROTOCOL_WINDOWSMEDIA			45
#define	IPOQUE_PROTOCOL_MMS						46
#define	IPOQUE_PROTOCOL_XBOX					47
#define	IPOQUE_PROTOCOL_QQ						48
#define	IPOQUE_PROTOCOL_MOVE					49
#define	IPOQUE_PROTOCOL_RTSP					50
#define IPOQUE_PROTOCOL_FEIDIAN					51
#define IPOQUE_PROTOCOL_ICECAST					52
#define IPOQUE_PROTOCOL_PPLIVE					53
#define IPOQUE_PROTOCOL_PPSTREAM				54
#define IPOQUE_PROTOCOL_ZATTOO					55
#define IPOQUE_PROTOCOL_SHOUTCAST				56
#define IPOQUE_PROTOCOL_SOPCAST					57
#define IPOQUE_PROTOCOL_TVANTS					58
#define IPOQUE_PROTOCOL_TVUPLAYER				59
#define IPOQUE_PROTOCOL_HTTP_APPLICATION_VEOHTV 60
#define IPOQUE_PROTOCOL_QQLIVE					61
#define IPOQUE_PROTOCOL_THUNDER					62
#define IPOQUE_PROTOCOL_SOULSEEK				63
#define IPOQUE_PROTOCOL_GADUGADU				64
#define IPOQUE_PROTOCOL_IRC						65
#define IPOQUE_PROTOCOL_POPO					66
#define IPOQUE_PROTOCOL_UNENCRYPED_JABBER		67
#define IPOQUE_PROTOCOL_MSN						68
#define IPOQUE_PROTOCOL_OSCAR					69
#define IPOQUE_PROTOCOL_YAHOO					70
#define IPOQUE_PROTOCOL_BATTLEFIELD				71
#define IPOQUE_PROTOCOL_QUAKE					72
#define IPOQUE_PROTOCOL_SECONDLIFE				73
#define IPOQUE_PROTOCOL_STEAM					74
#define IPOQUE_PROTOCOL_HALFLIFE2				75
#define IPOQUE_PROTOCOL_WORLDOFWARCRAFT			76
#define IPOQUE_PROTOCOL_TELNET					77
#define IPOQUE_PROTOCOL_STUN					78
#define IPOQUE_PROTOCOL_IPSEC					79
#define IPOQUE_PROTOCOL_GRE						80
#define IPOQUE_PROTOCOL_ICMP					81
#define IPOQUE_PROTOCOL_IGMP					82
#define IPOQUE_PROTOCOL_EGP						83
#define IPOQUE_PROTOCOL_SCTP					84
#define IPOQUE_PROTOCOL_OSPF					85
#define IPOQUE_PROTOCOL_IP_IN_IP				86
#define	IPOQUE_PROTOCOL_RTP						87
#define IPOQUE_PROTOCOL_RDP						88
#define IPOQUE_PROTOCOL_VNC						89
#define IPOQUE_PROTOCOL_PCANYWHERE				90
#define IPOQUE_PROTOCOL_SSL						91
#define IPOQUE_PROTOCOL_SSH						92
#define IPOQUE_PROTOCOL_USENET					93
#define IPOQUE_PROTOCOL_MGCP					94
#define IPOQUE_PROTOCOL_IAX						95
#define IPOQUE_PROTOCOL_TFTP					96
#define IPOQUE_PROTOCOL_AFP						97
#define IPOQUE_PROTOCOL_STEALTHNET				98
#define IPOQUE_PROTOCOL_AIMINI					99
#define IPOQUE_PROTOCOL_SIP						100
#define IPOQUE_PROTOCOL_TRUPHONE				101
#define IPOQUE_PROTOCOL_ICMPV6					102
#define IPOQUE_PROTOCOL_DHCPV6					103
#define IPOQUE_PROTOCOL_ARMAGETRON				104
#define IPOQUE_PROTOCOL_CROSSFIRE				105
#define IPOQUE_PROTOCOL_DOFUS					106
#define IPOQUE_PROTOCOL_FIESTA					107
#define IPOQUE_PROTOCOL_FLORENSIA				108
#define IPOQUE_PROTOCOL_GUILDWARS				109
#define IPOQUE_PROTOCOL_HTTP_APPLICATION_ACTIVESYNC		110
#define IPOQUE_PROTOCOL_KERBEROS				111
#define IPOQUE_PROTOCOL_LDAP					112
#define IPOQUE_PROTOCOL_MAPLESTORY				113
#define IPOQUE_PROTOCOL_MSSQL					114
#define IPOQUE_PROTOCOL_PPTP					115
#define IPOQUE_PROTOCOL_WARCRAFT3				116
#define IPOQUE_PROTOCOL_WORLD_OF_KUNG_FU		117
#define IPOQUE_PROTOCOL_MEEBO					118


#define IPOQUE_LAST_IMPLEMENTED_PROTOCOL        118


#define IPOQUE_MAX_SUPPORTED_PROTOCOLS IPOQUE_LAST_IMPLEMENTED_PROTOCOL


#define IPOQUE_PROTOCOL_LONG_STRING "unknown","FTP","Mail_POP","Mail_SMTP","Mail_IMAP","DNS","IPP","HTTP","MDNS","NTP",\
	"NETBIOS","NFS","SSDP","BGP","SNMP","XDMCP","SMB","SYSLOG","DHCP","PostgreSQL","MySQL","TDS","DirectDownloadLink","I23V5",\
	"AppleJuice","DirectConnect","Socrates","WinMX","MANOLITO","PANDO","Filetopia","iMESH","Kontiki","OpenFT","Kazaa/Fasttrack",\
	"Gnutella","eDonkey","Bittorrent","OFF","AVI","Flash","OGG","MPEG","QuickTime","RealMedia","Windowsmedia","MMS","XBOX","QQ",\
	"MOVE","RTSP","Feidian","Icecast","PPLive","PPStream","Zattoo","SHOUTCast","SopCast","TVAnts","TVUplayer","VeohTV",\
	"QQLive","Thunder/Webthunder","Soulseek","GaduGadu","IRC","Popo","Jabber","MSN","Oscar","Yahoo","Battlefield","Quake",\
	"Second Life","Steam","Halflife2","World of Warcraft","Telnet","STUN","IPSEC","GRE","ICMP","IGMP","EGP","SCTP","OSPF",\
	"IP in IP","RTP","RDP","VNC","PCAnywhere","SSL","SSH","USENET","MGCP","IAX","TFTP","AFP","StealthNet","Aimini","SIP","Truphone",\
	"ICMPv6","DHCPv6","Armagetron","CrossFire","Dofus","Fiesta","Florensia","Guildwars","HTTP Application Activesync","Kerberos",\
	"LDAP","MapleStory","msSQL","PPTP","WARCRAFT3","World of Kung Fu","MEEBO"
#define IPOQUE_PROTOCOL_SHORT_STRING "ukn","ftp","pop","smtp","imap","dns","ipp","http","mdns","ntp","netbios","nfs","ssdp",\
	"bgp","snmp","xdmcp","smb","syslog","dhcp","postgres","mysql","tds","ddl","i23v5","apple","directconnect","socrates","winmx",\
	"manolito","pando","filetopia","iMESH","kontiki","openft","fasttrack","gnutella","edonkey","bittorrent","off","avi",\
	"flash","ogg","mpeg","quicktime","realmedia","windowsmedia","mms","xbox","qq","move","rtsp","feidian","icecast","pplive",\
	"ppstream","zattoo","shoutcast","sopcast","tvants","tvuplayer","veohtv","qqlive","thunder","soulseek","gadugadu","irc",\
	"popo","jabber","msn","oscar","yahoo","battlefield","quake","secondlife","steam","hl2","worldofwarcraft","telnet","stun",\
	"ipsec","gre","icmp","igmp","egp","sctp","ospf","ipip","rtp","rdp","vnc","pcanywhere","ssl","ssh","usenet","mgcp","iax",\
	"tftp","afp","stealthnet","aimini","sip","truphone","icmpv6","dhcpv6","armagetron","crossfire","dofus","fiesta","florensia",\
	"guildwars","httpactivesync","kerberos","ldap","maplestory","mssql","pptp","warcraft3","wokf","meebo"

#ifdef __cplusplus
}
#endif
#endif
