/*
 * ndpi_protocol_ids.h
 *
 * Copyright (C) 2011-15 - ntop.org
 * Copyright (C) 2009-11 - ipoque GmbH
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

#ifndef __NDPI_API_INCLUDE_FILE__

#endif

#ifndef __NDPI_PROTOCOLS_DEFAULT_H__
#define __NDPI_PROTOCOLS_DEFAULT_H__

#define NDPI_DETECTION_SUPPORT_IPV6
#define NDPI_PROTOCOL_HISTORY_SIZE				2

#define NDPI_PROTOCOL_UNKNOWN					0

#define NDPI_PROTOCOL_NO_MASTER_PROTO        NDPI_PROTOCOL_UNKNOWN

#define NDPI_PROTOCOL_IP_ICMPV6					102

#define NDPI_PROTOCOL_HTTP					7
#define NDPI_PROTOCOL_HTTP_APPLICATION_VEOHTV 		        60
#define NDPI_PROTOCOL_SSL_NO_CERT			        64	/* SSL without certificate (Skype, Ultrasurf?) - ntop.org */
#define NDPI_PROTOCOL_SSL					91
#define NDPI_PROTOCOL_HTTP_APPLICATION_ACTIVESYNC		110
#define NDPI_PROTOCOL_HTTP_CONNECT				130
#define NDPI_PROTOCOL_HTTP_PROXY				131
#define NDPI_PROTOCOL_SOCKS					172	/* Tomasz Bujlow <tomasz@skatnet.dk> */

#define NDPI_PROTOCOL_FTP_CONTROL				1	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_MAIL_POP				        2
#define NDPI_PROTOCOL_MAIL_SMTP				        3
#define NDPI_PROTOCOL_MAIL_IMAP			  	        4
#define NDPI_PROTOCOL_DNS              			        5
#define NDPI_PROTOCOL_IPP					6
#define NDPI_PROTOCOL_MDNS					8
#define NDPI_PROTOCOL_NTP					9
#define NDPI_PROTOCOL_NETBIOS					10
#define NDPI_PROTOCOL_NFS					11
#define NDPI_PROTOCOL_SSDP					12
#define NDPI_PROTOCOL_BGP					13
#define NDPI_PROTOCOL_SNMP					14
#define NDPI_PROTOCOL_XDMCP					15
#define NDPI_PROTOCOL_SMBV1					16
#define NDPI_PROTOCOL_SYSLOG					17
#define NDPI_PROTOCOL_DHCP					18
#define NDPI_PROTOCOL_POSTGRES				        19
#define NDPI_PROTOCOL_MYSQL					20
#define NDPI_PROTOCOL_TDS					21
#define NDPI_PROTOCOL_DIRECT_DOWNLOAD_LINK			22
#define NDPI_PROTOCOL_MAIL_POPS				        23
#define NDPI_PROTOCOL_APPLEJUICE				24
#define NDPI_PROTOCOL_DIRECTCONNECT				25
#define NDPI_PROTOCOL_SOCRATES				        26
#define NDPI_PROTOCOL_WINMX					27
#define NDPI_PROTOCOL_VMWARE					28
#define NDPI_PROTOCOL_MAIL_SMTPS				29
#define NDPI_PROTOCOL_FILETOPIA				        30
#define NDPI_PROTOCOL_IMESH					31
#define NDPI_PROTOCOL_KONTIKI					32
#define NDPI_PROTOCOL_OPENFT					33
#define NDPI_PROTOCOL_FASTTRACK				        34
#define NDPI_PROTOCOL_GNUTELLA				        35
#define NDPI_PROTOCOL_EDONKEY					36	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_BITTORRENT				37
#define NDPI_PROTOCOL_EPP					38

#define NDPI_CONTENT_AVI				39
#define NDPI_CONTENT_FLASH				40
#define NDPI_CONTENT_OGG				41
#define	NDPI_CONTENT_MPEG				42
#define	NDPI_CONTENT_QUICKTIME				43
#define	NDPI_CONTENT_REALMEDIA				44
#define	NDPI_CONTENT_WINDOWSMEDIA			45
#define	NDPI_CONTENT_MMS				46

#define	NDPI_PROTOCOL_XBOX					47
#define	NDPI_PROTOCOL_QQ					48
#define	NDPI_PROTOCOL_MOVE					49
#define	NDPI_PROTOCOL_RTSP					50
#define NDPI_PROTOCOL_MAIL_IMAPS				51
#define NDPI_PROTOCOL_ICECAST					52
#define NDPI_PROTOCOL_PPLIVE					53	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_PPSTREAM				        54
#define NDPI_PROTOCOL_ZATTOO					55
#define NDPI_PROTOCOL_SHOUTCAST				        56
#define NDPI_PROTOCOL_SOPCAST					57
#define NDPI_PROTOCOL_TVANTS					58
#define NDPI_PROTOCOL_TVUPLAYER				        59
#define NDPI_PROTOCOL_QQLIVE					61
#define NDPI_PROTOCOL_THUNDER					62
#define NDPI_PROTOCOL_SOULSEEK				        63
#define NDPI_PROTOCOL_IRC					65
#define NDPI_PROTOCOL_AYIYA					66
#define NDPI_PROTOCOL_UNENCRYPTED_JABBER			67
#define NDPI_PROTOCOL_MSN					68
#define NDPI_PROTOCOL_OSCAR					69
#define NDPI_PROTOCOL_YAHOO					70
#define NDPI_PROTOCOL_BATTLEFIELD				71
#define NDPI_PROTOCOL_QUAKE					72
#define NDPI_PROTOCOL_IP_VRRP 				        73
#define NDPI_PROTOCOL_STEAM					74	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_HALFLIFE2				        75
#define NDPI_PROTOCOL_WORLDOFWARCRAFT				76
#define NDPI_PROTOCOL_TELNET					77
#define NDPI_PROTOCOL_STUN					78
#define NDPI_PROTOCOL_IP_IPSEC					79
#define NDPI_PROTOCOL_IP_GRE					80
#define NDPI_PROTOCOL_IP_ICMP					81
#define NDPI_PROTOCOL_IP_IGMP					82
#define NDPI_PROTOCOL_IP_EGP					83
#define NDPI_PROTOCOL_IP_SCTP					84
#define NDPI_PROTOCOL_IP_OSPF					85
#define NDPI_PROTOCOL_IP_IP_IN_IP				86
#define	NDPI_PROTOCOL_RTP					87
#define NDPI_PROTOCOL_RDP					88
#define NDPI_PROTOCOL_VNC					89
#define NDPI_PROTOCOL_PCANYWHERE				90
#define NDPI_PROTOCOL_SSH					92
#define NDPI_PROTOCOL_USENET					93
#define NDPI_PROTOCOL_MGCP					94
#define NDPI_PROTOCOL_IAX					95
#define NDPI_PROTOCOL_TFTP					96
#define NDPI_PROTOCOL_AFP					97
#define NDPI_PROTOCOL_STEALTHNET				98
#define NDPI_PROTOCOL_AIMINI					99
#define NDPI_PROTOCOL_SIP					100
#define NDPI_PROTOCOL_TRUPHONE				        101
#define NDPI_PROTOCOL_DHCPV6					103
#define NDPI_PROTOCOL_ARMAGETRON				104
#define NDPI_PROTOCOL_CROSSFIRE				        105
#define NDPI_PROTOCOL_DOFUS					106
#define NDPI_PROTOCOL_FIESTA					107
#define NDPI_PROTOCOL_FLORENSIA				        108
#define NDPI_PROTOCOL_GUILDWARS				        109
#define NDPI_PROTOCOL_KERBEROS				        111
#define NDPI_PROTOCOL_LDAP					112
#define NDPI_PROTOCOL_MAPLESTORY				113
#define NDPI_PROTOCOL_MSSQL					114
#define NDPI_PROTOCOL_PPTP					115
#define NDPI_PROTOCOL_WARCRAFT3				        116
#define NDPI_PROTOCOL_WORLD_OF_KUNG_FU			        117
#define NDPI_PROTOCOL_MEEBO					118
#define NDPI_PROTOCOL_FACEBOOK				119
#define NDPI_PROTOCOL_TWITTER				120
#define NDPI_PROTOCOL_DROPBOX					121
#define NDPI_PROTOCOL_GMAIL				122
#define NDPI_PROTOCOL_GOOGLE_MAPS			123
#define NDPI_PROTOCOL_YOUTUBE			        124
#define NDPI_PROTOCOL_SKYPE					125
#define NDPI_PROTOCOL_GOOGLE				126
#define NDPI_PROTOCOL_DCERPC					127
#define NDPI_PROTOCOL_NETFLOW					128
#define NDPI_PROTOCOL_SFLOW					129
#define NDPI_PROTOCOL_CITRIX					132
#define NDPI_PROTOCOL_NETFLIX				133
#define NDPI_PROTOCOL_LASTFM				134
#define NDPI_PROTOCOL_WAZE        			135

#define NDPI_PROTOCOL_YOUTUBE_UPLOAD        136 /* Upload files to youtube */
#define NDPI_PROTOCOL_ICQ                   137
#define NDPI_PROTOCOL_CHECKMK               138
#define NDPI_PROTOCOL_CITRIX_ONLINE				139
#define NDPI_PROTOCOL_APPLE				140
#define NDPI_PROTOCOL_WEBEX					141
#define NDPI_PROTOCOL_WHATSAPP				142
#define NDPI_PROTOCOL_APPLE_ICLOUD			143

#define NDPI_PROTOCOL_VIBER					144
#define NDPI_PROTOCOL_APPLE_ITUNES			145

#define NDPI_PROTOCOL_RADIUS					146
#define NDPI_PROTOCOL_WINDOWS_UPDATE				147
#define NDPI_PROTOCOL_TEAMVIEWER				148	/* xplico.org */
#define NDPI_PROTOCOL_TUENTI				149
#define NDPI_PROTOCOL_LOTUS_NOTES				150
#define NDPI_PROTOCOL_SAP					151
#define NDPI_PROTOCOL_GTP					152
#define NDPI_PROTOCOL_UPNP					153
#define NDPI_PROTOCOL_LLMNR					154
#define NDPI_PROTOCOL_REMOTE_SCAN				155
#define NDPI_PROTOCOL_SPOTIFY					156
#define NDPI_CONTENT_WEBM				157
#define NDPI_PROTOCOL_H323					158	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_OPENVPN					159	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_NOE					160	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_CISCOVPN				        161	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_TEAMSPEAK				        162	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_TOR					163	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_SKINNY					164	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_RTCP					165	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_RSYNC					166	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_ORACLE					167	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_CORBA					168	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_UBUNTUONE			  	        169	/* Remy Mudingay <mudingay@ill.fr> */
#define NDPI_PROTOCOL_WHOIS_DAS				        170
#define NDPI_PROTOCOL_COLLECTD				        171
#define NDPI_PROTOCOL_RTMP					174	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_FTP_DATA				        175	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_WIKIPEDIA				176	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_ZMQ                                       177
#define NDPI_PROTOCOL_AMAZON					178	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_EBAY					179	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_CNN					180	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_MEGACO    				181	/* Gianluca Costa <g.costa@xplico.org> */
#define NDPI_PROTOCOL_REDIS                                     182
#define NDPI_PROTOCOL_PANDO					183	/* Tomasz Bujlow <tomasz@skatnet.dk> */
#define NDPI_PROTOCOL_VHUA                                      184
#define NDPI_PROTOCOL_TELEGRAM				        185	/* Gianluca Costa <g.costa@xplico.org> */
#define NDPI_PROTOCOL_VEVO			        186
#define NDPI_PROTOCOL_PANDORA     			187
#define NDPI_PROTOCOL_QUIC				        188	/* Andrea Buscarinu <andrea.buscarinu@gmail.com> - Michele Campus <michelecampus5@gmail.com> */

#define NDPI_PROTOCOL_POPO				189
#define NDPI_PROTOCOL_MANOLITO				190
#define NDPI_PROTOCOL_FEIDIAN				191
#define NDPI_PROTOCOL_GADUGADU				192
#define NDPI_PROTOCOL_I23V5				193
#define NDPI_PROTOCOL_SECONDLIFE			194
#define NDPI_PROTOCOL_WHATSAPP_VOICE			195
#define NDPI_PROTOCOL_EAQ				196
#define NDPI_PROTOCOL_TIMMEU        			197
#define NDPI_PROTOCOL_TORCEDOR        			198
#define NDPI_PROTOCOL_KAKAOTALK        			199	/* KakaoTalk Chat (no voice call) */
#define NDPI_PROTOCOL_KAKAOTALK_VOICE  			200	/* KakaoTalk Voice */
#define NDPI_PROTOCOL_TWITCH                             201	/* Edoardo Dominici <edoaramis@gmail.com>  */
#define NDPI_PROTOCOL_QUICKPLAY                          202	/* Streaming service used by various services such as hooq.tv  */
#define NDPI_PROTOCOL_TIM                                203	/* Traffic for tim.com.br and tim.it */
#define NDPI_PROTOCOL_MPEGTS        			204
#define NDPI_PROTOCOL_SNAPCHAT                           205

#define NDPI_PROTOCOL_SIMET                              206
#define NDPI_PROTOCOL_OPENSIGNAL                         207
#define NDPI_PROTOCOL_99TAXI                             208
#define NDPI_PROTOCOL_EASYTAXI                           209
#define NDPI_PROTOCOL_GLOBOTV                            210
#define NDPI_PROTOCOL_TIMSOMDECHAMADA                    211
#define NDPI_PROTOCOL_TIMMENU                            212
#define NDPI_PROTOCOL_TIMPORTASABERTAS                   213
#define NDPI_PROTOCOL_TIMRECARGA                         214
#define NDPI_PROTOCOL_TIMBETA                            215
#define NDPI_PROTOCOL_DEEZER                             216
#define NDPI_PROTOCOL_INSTAGRAM                          217	/* Andrea Buscarinu <andrea.buscarinu@gmail.com> */
#define NDPI_PROTOCOL_MICROSOFT                          218
#define NDPI_PROTOCOL_BATTLENET                          219	/* Matteo Bracci <matteobracci1@gmail.com> */
#define NDPI_PROTOCOL_STARCRAFT 			220	/* Matteo Bracci <matteobracci1@gmail.com> */
#define NDPI_PROTOCOL_TEREDO 			        221
#define NDPI_PROTOCOL_HOTSPOT_SHIELD                     222
#define NDPI_PROTOCOL_HEP 			        223	/* Sipcapture.org QXIP BV */
#define NDPI_PROTOCOL_UBNTAC2				224	/* Ubiquity UBNT AirControl 2 - Thomas Fjellstrom <thomas+ndpi@fjellstrom.ca> */
#define NDPI_PROTOCOL_MS_LYNC 			        173
#define NDPI_PROTOCOL_OCS                                225
#define NDPI_PROTOCOL_OFFICE_365                         226
#define NDPI_PROTOCOL_CLOUDFLARE                         227
#define NDPI_PROTOCOL_MS_ONE_DRIVE                       228
#define NDPI_PROTOCOL_MQTT								229
#define NDPI_PROTOCOL_RX 			                230	/* RX: RPC protocol used by AFS */
#define NDPI_PROTOCOL_HTTP_DOWNLOAD 			                231	/* RX: RPC protocol used by AFS */
#define NDPI_PROTOCOL_COAP 			                232	/* RX: RPC protocol used by AFS */

#define NDPI_PROTOCOL_APPLESTORE            233
#define NDPI_PROTOCOL_OPENDNS               234
#define NDPI_PROTOCOL_GIT                   235
#define NDPI_PROTOCOL_DRDA                  236
#define NDPI_PROTOCOL_PLAYSTORE             237
#define NDPI_PROTOCOL_SOMEIP                238
#define NDPI_PROTOCOL_FIX                   239
#define NDPI_PROTOCOL_PLAYSTATION           240
#define NDPI_PROTOCOL_PASTEBIN              241	/* Paulo Angelo <pa@pauloangelo.com> */
#define NDPI_PROTOCOL_LINKEDIN              242	/* Paulo Angelo <pa@pauloangelo.com> */
#define NDPI_PROTOCOL_SOUNDCLOUD            243
#define NDPI_PROTOCOL_CSGO                  244	/* Counter-Strike Global Offensive, Dota 2 */
#define NDPI_PROTOCOL_LISP	            245

#define NDPI_PROTOCOL_NINTENDO              246
#define NDPI_PROTOCOL_DNSCRYPT              247
#define NDPI_PROTOCOL_1KXUN                 248
#define NDPI_PROTOCOL_IQIYI                 249
#define NDPI_PROTOCOL_WECHAT                250
#define NDPI_PROTOCOL_GITHUB                251
#define NDPI_PROTOCOL_SINA                  252
#define NDPI_PROTOCOL_SLACK                 253
#define NDPI_PROTOCOL_IFLIX                 254	/* www.vizuamatix.com R&D team & M.Mallawaarachchie <manoj_ws@yahoo.com> */
#define NDPI_PROTOCOL_HOTMAIL               255
#define NDPI_PROTOCOL_GOOGLE_DRIVE          256
#define NDPI_PROTOCOL_OOKLA                 257

#define NDPI_PROTOCOL_HANGOUT               258
#define NDPI_PROTOCOL_BJNP                  259
#define NDPI_PROTOCOL_SMPP                  260	/* Damir Franusic <df@release14.org> */
#define NDPI_PROTOCOL_TINC                  261	/* William Guglielmo <william@deselmo.com> */
#define NDPI_PROTOCOL_AMQP                  262
#define NDPI_PROTOCOL_GOOGLE_PLUS           263
#define NDPI_PROTOCOL_DIAMETER	            264
#define NDPI_PROTOCOL_APPLE_PUSH            265
#define NDPI_PROTOCOL_GOOGLE_SERVICES       266

#define NDPI_PROTOCOL_AMAZON_VIDEO          267
#define NDPI_PROTOCOL_GOOGLE_DOCS           268
#define NDPI_PROTOCOL_WHATSAPP_FILES        269 /* Videos, pictures, voice messages... */
#define NDPI_PROTOCOL_AJP                   270 /* Leonn Paiva <leonn.paiva@gmail.com>*/
#define NDPI_PROTOCOL_NTOP                  271
#define NDPI_PROTOCOL_MESSENGER             272
#define NDPI_PROTOCOL_MUSICALLY             273
#define NDPI_PROTOCOL_FBZERO                274
#define NDPI_PROTOCOL_VIDTO                 275 /* VidTO streaming service */
#define NDPI_PROTOCOL_RAPIDVIDEO            276 /* RapidVideo streaming */
#define NDPI_PROTOCOL_SHOWMAX            277
#define NDPI_PROTOCOL_SMBV23            278
#define NDPI_PROTOCOL_MINING            279
#define NDPI_PROTOCOL_MEMCACHED         280 /* Memcached - Darryl Sokoloski <darryl@egloo.ca> */
#define NDPI_PROTOCOL_SIGNAL            281
#define NDPI_PROTOCOL_NEST_LOG_SINK     282 /* Nest Log Sink (Nest Protect) - Darryl Sokoloski <darryl@egloo.ca> */
#define NDPI_PROTOCOL_MODBUS            283 /* Modbus */
#define NDPI_PROTOCOL_LINE            284 /* Modbus */
#define NDPI_PROTOCOL_TLS            285 /* Modbus */
#define NDPI_PROTOCOL_TLS_NO_CERT            286 /* Modbus */
#define NDPI_PROTOCOL_TARGUS_GETDATA            287

/* UPDATE UPDATE UPDATE UPDATE UPDATE UPDATE UPDATE UPDATE UPDATE (NDPI_PROTOCOL_PANDORA) */
#define NDPI_LAST_IMPLEMENTED_PROTOCOL			287

#define NDPI_MAX_SUPPORTED_PROTOCOLS (NDPI_LAST_IMPLEMENTED_PROTOCOL + 1)
#define NDPI_MAX_NUM_CUSTOM_PROTOCOLS                   (NDPI_NUM_BITS-NDPI_LAST_IMPLEMENTED_PROTOCOL)
#endif
