/*
 * ndpi_protocols_osdpi.h
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


#ifndef __NDPI_API_INCLUDE_FILE__

#endif

#ifndef __NDPI_PROTOCOLS_DEFAULT_H__
#define __NDPI_PROTOCOLS_DEFAULT_H__

#ifdef __cplusplus
extern "C" {
#endif

//  #define NDPI_ENABLE_DEBUG_MESSAGES

#define NDPI_DETECTION_SUPPORT_IPV6
#define NDPI_PROTOCOL_HISTORY_SIZE            3 

#define NDPI_PROTOCOL_UNKNOWN                 0
#define NDPI_PROTOCOL_FTP                     1
#define NDPI_PROTOCOL_MAIL_POP                2
#define NDPI_PROTOCOL_MAIL_SMTP               3
#define NDPI_PROTOCOL_MAIL_IMAP               4
#define NDPI_PROTOCOL_DNS              	      5
#define NDPI_PROTOCOL_IPP						6
#define NDPI_PROTOCOL_HTTP					7
#define NDPI_PROTOCOL_MDNS					8
#define NDPI_PROTOCOL_NTP						9
#define NDPI_PROTOCOL_NETBIOS					10
#define NDPI_PROTOCOL_NFS						11
#define NDPI_PROTOCOL_SSDP					12
#define NDPI_PROTOCOL_BGP						13
#define NDPI_PROTOCOL_SNMP					14
#define NDPI_PROTOCOL_XDMCP					15
#define NDPI_PROTOCOL_SMB						16
#define NDPI_PROTOCOL_SYSLOG					17
#define NDPI_PROTOCOL_DHCP					18
#define NDPI_PROTOCOL_POSTGRES				19
#define NDPI_PROTOCOL_MYSQL					20
#define NDPI_PROTOCOL_TDS						21
#define NDPI_PROTOCOL_DIRECT_DOWNLOAD_LINK	22
#define NDPI_PROTOCOL_I23V5					23
#define NDPI_PROTOCOL_APPLEJUICE				24
#define NDPI_PROTOCOL_DIRECTCONNECT			25
#define NDPI_PROTOCOL_SOCRATES				26
#define NDPI_PROTOCOL_WINMX					27
#define NDPI_PROTOCOL_MANOLITO				28
#define NDPI_PROTOCOL_PANDO					29
#define NDPI_PROTOCOL_FILETOPIA				30
#define NDPI_PROTOCOL_IMESH					31
#define NDPI_PROTOCOL_KONTIKI					32
#define NDPI_PROTOCOL_OPENFT					33
#define NDPI_PROTOCOL_FASTTRACK				34
#define NDPI_PROTOCOL_GNUTELLA				35
#define NDPI_PROTOCOL_EDONKEY					36
#define NDPI_PROTOCOL_BITTORRENT				37
#define NDPI_PROTOCOL_OFF						38
#define NDPI_PROTOCOL_AVI						39
#define NDPI_PROTOCOL_FLASH					40
#define NDPI_PROTOCOL_OGG						41
#define	NDPI_PROTOCOL_MPEG					42
#define	NDPI_PROTOCOL_QUICKTIME				43
#define	NDPI_PROTOCOL_REALMEDIA				44
#define	NDPI_PROTOCOL_WINDOWSMEDIA			45
#define	NDPI_PROTOCOL_MMS						46
#define	NDPI_PROTOCOL_XBOX					47
#define	NDPI_PROTOCOL_QQ						48
#define	NDPI_PROTOCOL_MOVE					49
#define	NDPI_PROTOCOL_RTSP					50
#define NDPI_PROTOCOL_FEIDIAN					51
#define NDPI_PROTOCOL_ICECAST					52
#define NDPI_PROTOCOL_PPLIVE					53
#define NDPI_PROTOCOL_PPSTREAM				54
#define NDPI_PROTOCOL_ZATTOO					55
#define NDPI_PROTOCOL_SHOUTCAST				56
#define NDPI_PROTOCOL_SOPCAST					57
#define NDPI_PROTOCOL_TVANTS					58
#define NDPI_PROTOCOL_TVUPLAYER				59
#define NDPI_PROTOCOL_HTTP_APPLICATION_VEOHTV 60
#define NDPI_PROTOCOL_QQLIVE					61
#define NDPI_PROTOCOL_THUNDER					62
#define NDPI_PROTOCOL_SOULSEEK				63
#define NDPI_PROTOCOL_GADUGADU				64
#define NDPI_PROTOCOL_IRC						65
#define NDPI_PROTOCOL_POPO					66
#define NDPI_PROTOCOL_UNENCRYPED_JABBER		67
#define NDPI_PROTOCOL_MSN						68
#define NDPI_PROTOCOL_OSCAR					69
#define NDPI_PROTOCOL_YAHOO					70
#define NDPI_PROTOCOL_BATTLEFIELD				71
#define NDPI_PROTOCOL_QUAKE					72
#define NDPI_PROTOCOL_SECONDLIFE				73
#define NDPI_PROTOCOL_STEAM					74
#define NDPI_PROTOCOL_HALFLIFE2				75
#define NDPI_PROTOCOL_WORLDOFWARCRAFT			76
#define NDPI_PROTOCOL_TELNET					77
#define NDPI_PROTOCOL_STUN					78
#define NDPI_PROTOCOL_IPSEC					79
#define NDPI_PROTOCOL_GRE						80
#define NDPI_PROTOCOL_ICMP					81
#define NDPI_PROTOCOL_IGMP					82
#define NDPI_PROTOCOL_EGP						83
#define NDPI_PROTOCOL_SCTP					84
#define NDPI_PROTOCOL_OSPF					85
#define NDPI_PROTOCOL_IP_IN_IP				86
#define	NDPI_PROTOCOL_RTP						87
#define NDPI_PROTOCOL_RDP						88
#define NDPI_PROTOCOL_VNC						89
#define NDPI_PROTOCOL_PCANYWHERE				90
#define NDPI_PROTOCOL_SSL						91
#define NDPI_PROTOCOL_SSH						92
#define NDPI_PROTOCOL_USENET					93
#define NDPI_PROTOCOL_MGCP					94
#define NDPI_PROTOCOL_IAX						95
#define NDPI_PROTOCOL_TFTP					96
#define NDPI_PROTOCOL_AFP						97
#define NDPI_PROTOCOL_STEALTHNET				98
#define NDPI_PROTOCOL_AIMINI					99
#define NDPI_PROTOCOL_SIP					100
#define NDPI_PROTOCOL_TRUPHONE				101
#define NDPI_PROTOCOL_ICMPV6					102
#define NDPI_PROTOCOL_DHCPV6					103
#define NDPI_PROTOCOL_ARMAGETRON				104
#define NDPI_PROTOCOL_CROSSFIRE				105
#define NDPI_PROTOCOL_DOFUS					106
#define NDPI_PROTOCOL_FIESTA					107
#define NDPI_PROTOCOL_FLORENSIA				108
#define NDPI_PROTOCOL_GUILDWARS				109
#define NDPI_PROTOCOL_HTTP_APPLICATION_ACTIVESYNC		110
#define NDPI_PROTOCOL_KERBEROS				111
#define NDPI_PROTOCOL_LDAP					112
#define NDPI_PROTOCOL_MAPLESTORY				113
#define NDPI_PROTOCOL_MSSQL					114
#define NDPI_PROTOCOL_PPTP					115
#define NDPI_PROTOCOL_WARCRAFT3				116
#define NDPI_PROTOCOL_WORLD_OF_KUNG_FU		        117
#define NDPI_PROTOCOL_MEEBO					118

typedef struct {
  char *string_to_match;
  int protocol_id;
} ndpi_protocol_match;

#define NDPI_PROTOCOL_FACEBOOK				        119
#define NDPI_PROTOCOL_TWITTER				        120
#define NDPI_PROTOCOL_DROPBOX				        121
#define NDPI_PROTOCOL_GMAIL				        122
#define NDPI_PROTOCOL_GOOGLE_MAPS			        123
#define NDPI_PROTOCOL_YOUTUBE				        124
#define NDPI_PROTOCOL_SKYPE				        125
#define NDPI_PROTOCOL_GOOGLE				        126
#define NDPI_PROTOCOL_DCERPC				        127
#define NDPI_PROTOCOL_NETFLOW				        128
#define NDPI_PROTOCOL_SFLOW				        129
#define NDPI_PROTOCOL_HTTP_CONNECT				130
#define NDPI_PROTOCOL_HTTP_PROXY				131
#define NDPI_PROTOCOL_CITRIX				        132
#define NDPI_PROTOCOL_NETFLIX				        133
#define NDPI_PROTOCOL_LASTFM                                    134
#define NDPI_PROTOCOL_GROOVESHARK                               135
#define NDPI_PROTOCOL_SKYFILE_PREPAID                           136
#define NDPI_PROTOCOL_SKYFILE_RUDICS                            137
#define NDPI_PROTOCOL_SKYFILE_POSTPAID                          138
#define NDPI_PROTOCOL_CITRIX_ONLINE                             139
#define NDPI_PROTOCOL_APPLE                                     140
#define NDPI_PROTOCOL_WEBEX                                     141
#define NDPI_PROTOCOL_WHATSAPP                                  142
#define NDPI_PROTOCOL_APPLE_ICLOUD                              143
#define NDPI_PROTOCOL_VIBER                                     144
#define NDPI_PROTOCOL_APPLE_ITUNES                              145
#define NDPI_PROTOCOL_RADIUS                                    146
#define NDPI_PROTOCOL_WINDOWS_UPDATE                            147 /* Thierry Laurion */
#define NDPI_PROTOCOL_TEAMVIEWER                                148 /* xplico.org */
#define NDPI_PROTOCOL_TUENTI					149

/* NOTE: REMEMBER TO UPDATE NDPI_PROTOCOL_LONG_STRING / NDPI_PROTOCOL_SHORT_STRING */

#define NDPI_LAST_IMPLEMENTED_PROTOCOL                        149


#define NDPI_MAX_SUPPORTED_PROTOCOLS (NDPI_LAST_IMPLEMENTED_PROTOCOL + 1)


#define NDPI_PROTOCOL_LONG_STRING "Unknown","FTP","Mail_POP","Mail_SMTP","Mail_IMAP","DNS","IPP","HTTP","MDNS","NTP",\
    "NETBIOS","NFS","SSDP","BGP","SNMP","XDMCP","SMB","SYSLOG","DHCP","PostgreSQL","MySQL","TDS","DirectDownloadLink","I23V5", \
    "AppleJuice","DirectConnect","Socrates","WinMX","MANOLITO","PANDO","Filetopia","iMESH","Kontiki","OpenFT","Kazaa/Fasttrack", \
    "Gnutella","eDonkey","Bittorrent","OFF","AVI","Flash","OGG","MPEG","QuickTime","RealMedia","Windowsmedia","MMS","XBOX","QQ", \
    "MOVE","RTSP","Feidian","Icecast","PPLive","PPStream","Zattoo","SHOUTCast","SopCast","TVAnts","TVUplayer","VeohTV",	\
    "QQLive","Thunder/Webthunder","Soulseek","GaduGadu","IRC","Popo","Jabber","MSN","Oscar","Yahoo","Battlefield","Quake", \
    "Second Life","Steam","Halflife2","World of Warcraft","Telnet","STUN","IPSEC","GRE","ICMP","IGMP","EGP","SCTP","OSPF", \
    "IP in IP","RTP","RDP","VNC","PCAnywhere","SSL","SSH","USENET","MGCP","IAX","TFTP","AFP","StealthNet","Aimini","SIP","Truphone", \
    "ICMPv6","DHCPv6","Armagetron","CrossFire","Dofus","Fiesta","Florensia","Guildwars","HTTP Application Activesync","Kerberos", \
    "LDAP","MapleStory","msSQL","PPTP","WARCRAFT3","World of Kung Fu","MEEBO", \
    "FaceBook","Twitter","DropBox","Gmail","Google Maps","YouTube","Skype","Google","DCE RPC","NetFlow_IPFIX","sFlow", \
    "HTTP Connect","HTTP Proxy","Citrix","Netflix","Last.fm","Grooveshark", \
    "Skyfile_prepaid","Skyfile_rudics","Skyfile_postpaid","CitrixOnline_GotoMeeting","Apple","Webex",\
    "WhatsApp","Apple_iCloud","Viber","Apple_iTunes","Radius","WindowsUpdate","TeamViewer","Tuenti"
#define NDPI_PROTOCOL_SHORT_STRING "ukn","ftp","pop","smtp","imap","dns","ipp","http","mdns","ntp","netbios","nfs","ssdp", \
    "bgp","snmp","xdmcp","smb","syslog","dhcp","postgres","mysql","tds","ddl","i23v5","apple","directconnect","socrates","winmx", \
    "manolito","pando","filetopia","iMESH","kontiki","openft","fasttrack","gnutella","edonkey","bittorrent","off","avi", \
    "flash","ogg","mpeg","quicktime","realmedia","windowsmedia","mms","xbox","qq","move","rtsp","feidian","icecast","pplive", \
    "ppstream","zattoo","shoutcast","sopcast","tvants","tvuplayer","veohtv","qqlive","thunder","soulseek","gadugadu","irc", \
    "popo","jabber","msn","oscar","yahoo","battlefield","quake","secondlife","steam","hl2","worldofwarcraft","telnet","stun", \
    "ipsec","gre","icmp","igmp","egp","sctp","ospf","ipip","rtp","rdp","vnc","pcanywhere","ssl","ssh","usenet","mgcp","iax", \
    "tftp","afp","stealthnet","aimini","sip","truphone","icmpv6","dhcpv6","armagetron","crossfire","dofus","fiesta","florensia", \
    "guildwars","httpactivesync","kerberos","ldap","maplestory","mssql","pptp","warcraft3","wokf","meebo",\
    "facebook","twitter","dropbox","gmail","gmaps","youtube","skype","google","dcerpc","netflow","sflow", \
    "http_connect","http_proxy","Citrix","Netflix","Last.fm","Grooveshark",\
    "Skyfile_pre", "Skyfile_ru","Skyfile_post","CitrixOnline","iMessage_Facetime","Webex","WhatsApp","iCloud","Viber","iTunes", \
    "Radius","WinUpdate","TeamViewer","tuenti"

#ifdef __cplusplus
}
#endif
#endif
