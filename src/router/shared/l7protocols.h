/*
 * L7-filter Supported Protocols 
 */
    /*
     * 2009-05-28 
     */

typedef struct _l7filters	// l7 and p2p filters
{

	char *name;
	int protocol;		// 1=p2p, 0=l7, 2=opendpi

} l7filters;




#ifdef HAVE_OPENDPI
#define DPI 2 //open dpi based
#define PDPI 2 //open dpi based
#else
#define DPI 0 //default l7
#define PDPI 1 //default l7
#endif
//Added ,  (in extra), dazhihui, .

l7filters filters_list[] = {

	{
	 "100bao", 0},		// 100bao - a Chinese P2P protocol/program -
	// http://www.100bao.com
#ifdef HAVE_OPENDPI
	{
	 "afp", 2},
#endif
	{
	 "aim", 0},		// AIM - AOL instant messenger (OSCAR and
#ifdef HAVE_OPENDPI
	{
	 "aimini", 2},
#endif
	// TOC)
	{
	 "aimwebcontent", 0},	// AIM web content - ads/news content
	// downloaded by AOL Instant Messenger
	{
	 "applejuice", 1},	// Apple Juice - P2P filesharing -
	// http://www.applejuicenet.de
	{
	 "ares", 1},		// Ares - P2P filesharing -
	// http://aresgalaxy.sf.net
	{
	 "armagetron", DPI},	// Armagetron Advanced - open source
	// Tron/snake based multiplayer game
	{
	 "audiogalaxy", 0},	// Audiogalaxy - (defunct) Peer to Peer
#ifdef HAVE_OPENDPI
	{
	 "avi", 2},
#endif
	// filesharing
#ifdef HAVE_OPENDPI
	{
	 "battlefield", 2},
#endif
	{
	 "battlefield1942", 0},	// Battlefield 1942 - An EA game
	{
	 "battlefield2", 0},	// Battlefield 2 - An EA game.
	{
	 "battlefield2142", 0},	// Battlefield 2142 - An EA game.
	{
	 "bgp", DPI},		// BGP - Border Gateway Protocol - RFC 1771
	{
	 "biff", 0},		// Biff - new mail notification
	{
	 "bittorrent", PDPI},	// Bittorrent - P2P filesharing / publishing
	// tool - http://www.bittorrent.com
	{
	 "bt", 0},
	{
	 "bt1", 0},
	{
	 "bt2", 0},
	{
	 "bt3", 0},
	{
	 "chikka", 0},		// Chikka - SMS service which can be used
	// without phones - http://chikka.com
	{
	 "cimd", 0},		// Computer Interface to Message
	// Distribution, an SMSC protocol by Nokia
	{
	 "ciscovpn", 0},	// Cisco VPN - VPN client software to a Cisco 
	// VPN server
	{
	 "citrix", DPI},		// Citrix ICA - proprietary remote desktop
#ifdef HAVE_OPENDPI
	{
	 "citrixonline", 2},		// Citrix ICA - proprietary remote desktop
#endif
	// application - http://citrix.com
	{
	 "clubbox", 0},
	{
	 "code_red", 0},	// Code Red - a worm that attacks Microsoft
	// IIS web servers
	{
	 "counterstrike-source", 0},	// Counterstrike (using the new
	// "Source" engine) - network game
#ifdef HAVE_OPENDPI
	{
	 "crossfire", 2},	// Dazhihui - stock analysis and trading; Chinese - http://www.gw.com.cn
#endif
	{
	 "cvs", 0},		// CVS - Concurrent Versions System
	{
	 "dayofdefeat-source", 0},	// Day of Defeat: Source - game
	// (Half-Life 2 mod) -
	// http://www.valvesoftware.com
	{
	 "dazhihui", 0},	// Dazhihui - stock analysis and trading; Chinese - http://www.gw.com.cn
#ifdef HAVE_OPENDPI
	{
	 "dcerpc", 2},	// Dazhihui - stock analysis and trading; Chinese - http://www.gw.com.cn
	{
	 "ddl", 2},	// Dazhihui - stock analysis and trading; Chinese - http://www.gw.com.cn
#endif
	{
	 "dhcp", DPI},		// DHCP - Dynamic Host Configuration Protocol 
#ifdef HAVE_OPENDPI
	{
	 "dhcpv6", 2},	// Dazhihui - stock analysis and trading; Chinese - http://www.gw.com.cn
#endif
	// - RFC 1541
	{
	 "directconnect", PDPI},	// Direct Connect - P2P filesharing -
	// http://www.neo-modus.com
	{
	 "dns", DPI},		// DNS - Domain Name System - RFC 1035
#ifdef HAVE_OPENDPI
	{
	 "dofus", 2},	// Dazhihui - stock analysis and trading; Chinese - http://www.gw.com.cn
#endif
	{
	 "doom3", 0},		// Doom 3 - computer game
#ifdef HAVE_OPENDPI
	{
	 "dropbox", 2},	// Dazhihui - stock analysis and trading; Chinese - http://www.gw.com.cn
#endif
	{
	 "edonkey", PDPI},		// eDonkey2000 - P2P filesharing -
#ifdef HAVE_OPENDPI
	{
	 "egp", 2},
#endif
	// http://edonkey2000.com and others
	{
	 "exe", 0},		// Executable - Microsoft PE file format.
	{
	 "fasttrack", DPI},	// FastTrack - P2P filesharing (Kazaa,
	// Morpheus, iMesh, Grokster, etc)
#ifdef HAVE_OPENDPI
	{
	 "feidian", 2},
	{
	 "fiesta", 2},	// FastTrack - P2P filesharing (Kazaa,
#endif
	{
	 "filetopia", DPI},	// FastTrack - P2P filesharing (Kazaa,
	{
	 "finger", 0},		// Finger - User information server - RFC
	// 1288
	{
	 "flash", 0},		// Flash - Macromedia Flash.
#ifdef HAVE_OPENDPI
	{
	 "florensia", 2},
#endif
	{
	 "freenet", 0},		// Freenet - Anonymous information retrieval
	// - http://freenetproject.org
	{
	 "freegate_dns", 0},
	{
	 "freegate_http", 0},
	{
	 "ftp", DPI},		// FTP - File Transfer Protocol - RFC 959
#ifdef HAVE_OPENDPI
	{
	 "gadugadu", 2},
#endif
	{
	 "gif", 0},		// GIF - Popular Image format.
	{
	 "gkrellm", 0},		// Gkrellm - a system monitor -
	// http://gkrellm.net
	{
	 "gnucleuslan", 0},	// GnucleusLAN - LAN-only P2P filesharing
	{
	 "gnutella", PDPI},	// Gnutella - P2P filesharing
	{
	 "goboogy", 0},		// GoBoogy - a Korean P2P protocol
#ifdef HAVE_OPENDPI
	{
	 "google", 2},
#endif
	{
	 "gogobox", 0},
	{
	 "gopher", 0},		// Gopher - A precursor to HTTP - RFC 1436
#ifdef HAVE_OPENDPI
	{
	 "gre", 2},
#endif
#ifdef HAVE_OPENDPI
	{
	 "groveshark", 2},
#endif
	{
	 "gtalk", 0},		// GTalk, a Jabber (XMPP) client
	{
	 "gtalk1", 0},
	{
	 "gtalk2", 0},
	{
	 "gtalk_file", 0},
	{
	 "gtalk_file_1", 0},
	{
	 "gtalk_vista", 0},
#ifdef HAVE_OPENDPI
	{
	 "gtp", 2},		// LPD - Line Printer Daemon Protocol
#endif
	{
	 "guildwars", DPI},	// Guild Wars - online game - http://guildwars.com
	{
	 "h323", 0},		// H.323 - Voice over IP.
	{
	 "hamachi1", 0},
	{
	 "halflife2-deathmatch", 0},	// Half-Life 2 Deathmatch - popular
	// computer game
	{
	 "hddtemp", 0},		// hddtemp - Hard drive temperature
	// reporting
#ifdef HAVE_OPENDPI
	{
	 "hl2", 2},
#endif
	{
	 "hotline", 0},		// Hotline - An old P2P filesharing protocol
	{
	 "hotspot-shield", 0},
	{
	 "html", 0},		// (X)HTML - (Extensible) Hypertext Markup
	// Language - http://w3.org
#ifdef HAVE_OPENDPI
	{
	 "http_connect", 2},		// HTTP - HyperText Transfer Protocol - RFC
	{
	 "http_proxy", 2},		// HTTP - HyperText Transfer Protocol - RFC
	// 2616
#endif
	{
	 "http-rtsp", 0},	// RTSP tunneled within HTTP
	{
	 "http", DPI},		// HTTP - HyperText Transfer Protocol - RFC
#ifdef HAVE_OPENDPI
	{
	 "httpactivesync", 2},		// HTTP - HyperText Transfer Protocol - RFC
	// 2616
#endif
	{
	 "http-dap", 0},	// HTTP by Download Accelerator Plus -
	// http://www.speedbit.com
	{
	 "http-freshdownload", 0},	// HTTP by Fresh Download -
	// http://www.freshdevices.com
	{
	 "http-itunes", 0},	// HTTP - iTunes (Apple's music program)
	{
	 "httpaudio", 0},	// HTTP - Audio over HyperText Transfer
	// Protocol (RFC 2616)
	{
	 "httpcachehit", 0},	// HTTP - Proxy Cache hit for HyperText
	// Transfer Protocol (RFC 2616)
	{
	 "httpcachemiss", 0},	// HTTP - Proxy Cache miss for HyperText
	// Transfer Protocol (RFC 2616)
	{
	 "httpvideo", 0},	// HTTP - Video over HyperText Transfer
	// Protocol (RFC 2616)
#ifdef HAVE_OPENDPI
	{
	 "i23v5", 2},
	{
	 "iax", 2},
	{
	 "icecast", 2},
	{
	 "igmp", 2},
	{
	 "icmp", 2},
#endif
	{
	 "icq_file", 0},
	{
	 "icq_file_1", 0},
	{
	 "icq_file_2", 0},
	{
	 "icq_login", 0},
	{
	 "ident", 0},		// Ident - Identification Protocol - RFC
	// 1413
	{
	 "imap", DPI},		// IMAP - Internet Message Access Protocol (A 
	// common e-mail protocol)
#ifdef HAVE_OPENDPI
	{
	 "imesh", 2},
	{
	 "imessage_facetime", 2},
#else
	{
	 "imesh", 0},		// iMesh - the native protocol of iMesh, a
#endif
	// P2P application - http://imesh.com
#ifdef HAVE_OPENDPI
	{
	 "ipip", 2},
#endif
	{
	 "ipp", DPI},		// IP printing - a new standard for UNIX
	// printing - RFC 2911
#ifdef HAVE_OPENDPI
	{
	 "ipsec", 2},
#endif
	{
	 "irc", DPI},		// IRC - Internet Relay Chat - RFC 1459
	{
	 "jabber", DPI},		// Jabber (XMPP) - open instant messenger
	// protocol - RFC 3920 - http://jabber.org
	{
	 "jpeg", 0},		// JPEG - Joint Picture Expert Group image
	// format.
#ifdef HAVE_OPENDPI
	{
	 "kerberos", 2},
#endif
	{
	 "kugoo", 0},		// KuGoo - a Chinese P2P program -
#ifdef HAVE_OPENDPI
	{
	 "kontiki", 2},
	{
	 "ldap", 2},
#endif
	// http://www.kugoo.com
	{
	 "live365", 0},		// live365 - An Internet radio site -
	// http://live365.com
	{
	 "liveforspeed", 0},	// Live For Speed - A racing game.
#ifdef HAVE_OPENDPI
	{
	 "lotus_notes", 2},		// LPD - Line Printer Daemon Protocol
#endif

#ifdef HAVE_OPENDPI
	{
	 "llmnr", 2},		// LPD - Line Printer Daemon Protocol
#endif
	{
	 "lpd", 0},		// LPD - Line Printer Daemon Protocol
	// (old-style UNIX printing) - RFC 1179
#ifdef HAVE_OPENDPI
	{
	 "manolito", 2},		// Medal of Honor Allied Assault - an
	{
	 "maplestory", 2},		// Medal of Honor Allied Assault - an
	{
	 "mdns", 2},		// Medal of Honor Allied Assault - an
	{
	 "mgcp", 2},		// Medal of Honor Allied Assault - an
#endif
#ifdef HAVE_OPENDPI
	{
	 "mms", 2},
#endif
	{
	 "mohaa", 0},		// Medal of Honor Allied Assault - an
	// Electronic Arts game
#ifdef HAVE_OPENDPI
	{
	 "move", 2},
#endif
	{
	 "mp3", 0},		// MP3 - Moving Picture Experts Group Audio
#ifdef HAVE_OPENDPI
	{
	 "mpeg", 2},
#endif
	// Layer III
#ifdef HAVE_OPENDPI
	{
	 "msn", 2},
#endif
	{
	 "msn-filetransfer", 0},	// MSN (Micosoft Network) Messenger file
	// transfers (MSNFTP and MSNSLP)
	{
	 "msnmessenger", 0},	// MSN Messenger - Microsoft Network chat
	// client
#ifdef HAVE_OPENDPI
	{
	 "mssql", 2},
#endif
	{
	 "mute", 1},		// MUTE - P2P filesharing -
	// http://mute-net.sourceforge.net
#ifdef HAVE_OPENDPI
	{
	 "mysql", 2},	// Subversion - a version control system
#endif
	{
	 "napster", 0},		// Napster - P2P filesharing
	{
	 "nbns", 0},		// NBNS - NetBIOS name service
	{
	 "ncp", 0},		// NCP - Novell Core Protocol
	{
	 "netbios", DPI},		// NetBIOS - Network Basic Input Output
#ifdef HAVE_OPENDPI
	{
	 "netflix", 2},		// Medal of Honor Allied Assault - an
	{
	 "netflow", 2},		// Medal of Honor Allied Assault - an
#endif
#ifdef HAVE_OPENDPI
	{
	 "nfs", 2},		// Medal of Honor Allied Assault - an
#endif
	// System
	{
	 "nimda", 0},		// Nimda - a worm that attacks Microsoft IIS
	// web servers, and MORE!
	{
	 "nntp", 0},		// NNTP - Network News Transfer Protocol -
	// RFCs 977 and 2980
	{
	 "ntp", DPI},		// (S)NTP - (Simple) Network Time Protocol -
	// RFCs 1305 and 2030
#ifdef HAVE_OPENDPI
	{
	 "off", 2},	// Subversion - a version control system
#endif
	{
	 "ogg", DPI},		// Ogg - Ogg Vorbis music format (not any ogg 
	// file, just vorbis)
	{
	 "openft", DPI},		// OpenFT - P2P filesharing (implemented in
#ifdef HAVE_OPENDPI
	{
	 "oscar", 2},
#endif
	// giFT library)
#ifdef HAVE_OPENDPI
	{
	 "pando", 2},	// Subversion - a version control system
#endif
	{
	 "pcanywhere", DPI},	// pcAnywhere - Symantec remote access
	// program
	{
	 "pdf", 0},		// PDF - Portable Document Format -
	// Postscript-like format by Adobe
	{
	 "perl", 0},		// Perl - A scripting language by Larry
	// Wall.
	{
	 "png", 0},		// PNG - Portable Network Graphics, a popular 
	// image format
	{
	 "poco", 0},		// POCO and PP365 - Chinese P2P filesharing - 
	// http://pp365.com http://poco.cn
	{
	 "pop3", DPI},		// POP3 - Post Office Protocol version 3
#ifdef HAVE_OPENDPI
	{
	 "popo", 2},
	{
	 "postgres", 2},	// Subversion - a version control system
#endif
	{
	 "postscript", 0},	// Postscript - Printing Language
	{
	 "pplive", DPI},		// PPLive - Chinese P2P streaming video - http://pplive.com
#ifdef HAVE_OPENDPI
	{
	 "ppstream", 2},
	{
	 "pptp", 2},
#endif
	{
	 "pre_icq_login", 0},
	{
	 "pre_msn_login", 0},
	{
	 "pre_urlblock", 0},
	{
	 "pre_yahoo_login", 0},
	{
	 "pressplay", 0},	// pressplay - A legal music distribution
	{
	 "qianlong", 0},
	{
	 "qq", DPI},		// Tencent QQ Protocol - Chinese instant
	// messenger protocol - http://www.qq.com
	// site - http://pressplay.com
	{
	 "qq_login", 0},
	{
	 "qq_login_1", 0},
	{
	 "qq_tcp_file", 0},
	{
	 "qq_udp_file", 0},
	{
	 "qqdownload_1", 0},
	{
	 "qqdownload_2", 0},
	{
	 "qqdownload_3", 0},
	{
	 "qqfile", 0},
	{
	 "qqgame", 0},
	{
	 "qqlive", DPI},
	{
	 "qqlive2", 0},
#ifdef HAVE_OPENDPI
	{
	 "quake", 2},
#endif
	{
	 "quake-halflife", 0},	// Half Life 1 engine games (HL 1, Quake
	// 2/3/World, Counterstrike 1.6, etc.)
	{
	 "quake1", 0},		// Quake 1 - A popular computer game.
	{
	 "quicktime", DPI},	// Quicktime HTTP
#ifdef HAVE_OPENDPI
	{
	 "radius", 2},
#endif
	{
	 "radmin", 0},		// Famatech Remote Administrator - remote
	// desktop for MS Windows
	{
	 "rar", 0},		// RAR - The WinRAR archive format
	{
	 "rdp", DPI},		// RDP - Remote Desktop Protocol (used in
	// Windows Terminal Services)
#ifdef HAVE_OPENDPI
	{
	 "realmedia", 2},
	{
	 "remotescan", 2},
#endif
	{
	 "replaytv-ivs", 0},	// ReplayTV Internet Video Sharing - Digital
	// Video Recorder - http://replaytv.com
	{
	 "rlogin", 0},		// rlogin - remote login - RFC 1282
	{
	 "rpm", 0},		// RPM - Redhat Package Management packages
	{
	 "rtf", 0},		// RTF - Rich Text Format - an open document
	// format
	{
	 "rtp", DPI},		// RTP - Real-time Transport Protocol - RFC
	// 3550
	{
	 "rtsp", DPI},		// RTSP - Real Time Streaming Protocol -
	// http://www.rtsp.org - RFC 2326
	{
	 "runesofmagic", 0},	// Runes of Magic - game - http://www.runesofmagic.com
#ifdef HAVE_OPENDPI
	{
	 "sap", 2},		// LPD - Line Printer Daemon Protocol
#endif
#ifdef HAVE_OPENDPI
	{
	 "sctp", 2},
	{
	 "secondlife", 2},
#endif
	{
	 "shoutcast", DPI},	// Shoutcast and Icecast - streaming audio
	{
	 "sip", DPI},		// SIP - Session Initiation Protocol -
	// Internet telephony - RFC 3261
#ifdef HAVE_OPENDPI
	{
	 "skyfile_pre", 2},
	{
	 "skyfile_ru", 2},
	{
	 "skyfile_post", 2},
#endif
#ifdef HAVE_OPENDPI
	{
	 "skype", 2},
#else
	{
	 "skypeout", 0},	// Skype to phone - UDP voice call (program
	// to POTS phone) - http://skype.com
	{
	 "skypetoskype", 0},	// Skype to Skype - UDP voice call (program
#endif
	// to program) - http://skype.com
	{
	 "smb", DPI},		// Samba/SMB - Server Message Block -
	// Microsoft Windows filesharing
	{
	 "smtp", DPI},		// SMTP - Simple Mail Transfer Protocol - RFC 
	// 2821 (See also RFC 1869)
	{
	 "snmp", DPI},		// SNMP - Simple Network Management Protocol
	// - RFC 1157
	{
	 "snmp-mon", 0},	// SNMP Monitoring - Simple Network
	// Management Protocol (RFC1157)
	{
	 "snmp-trap", 0},	// SNMP Traps - Simple Network Management
	// Protocol (RFC1157)
	{
	 "socks", 0},		// SOCKS Version 5 - Firewall traversal
	// protocol - RFC 1928
	{
	 "soribada", 0},	// Soribada - A Korean P2P filesharing
	// program/protocol -
	// http://www.soribada.com
	{
	 "soulseek", PDPI},	// Soulseek - P2P filesharing -
	// http://slsknet.org
	{
	 "ssdp", DPI},		// SSDP - Simple Service Discovery Protocol - 
	// easy discovery of network devices
	{
	 "ssh", DPI},		// SSH - Secure SHell
	{
	 "ssl", DPI},		// SSL and TLS - Secure Socket Layer /
	// Transport Layer Security - RFC 2246
	{
	 "stun", DPI},		// STUN - Simple Traversal of UDP Through NAT 
	// - RFC 3489
	{
	 "subspace", 0},	// Subspace - 2D asteroids-style space game - 
	// http://sscentral.com
	{
	 "subversion", 0},	// Subversion - a version control system
#ifdef HAVE_OPENDPI
	{
	 "steam", 2},	// Subversion - a version control system
	{
	 "stealthnet", 2},	// Subversion - a version control system
	{
	 "socrates", 2},	// Subversion - a version control system
#ifdef HAVE_OPENDPI
	{
	 "sopcast", 2},
#endif
	{
	 "syslog", 2},	// Subversion - a version control system
#endif
	{
	 "tar", 0},		// Tar - tape archive. Standard UNIX file
#ifdef HAVE_OPENDPI
	{
	 "tds", 2},	// Subversion - a version control system
#endif

	// archiver, not just for tapes.
	{
	 "teamfortress2", 0},	// Team Fortress 2 - network game -
	// http://www.valvesoftware.com
	{
	 "teamspeak", 0},	// TeamSpeak - VoIP application -
	{
	 "teamviewer", DPI},
	{
	 "teamviewer1", 0},
	{
	 "telnet", DPI},		// Telnet - Insecure remote login - RFC 854
	{
	 "tesla", 0},		// Tesla Advanced Communication - P2P
	// filesharing (?)
	{
	 "tftp", DPI},		// TFTP - Trivial File Transfer Protocol -
	// used for bootstrapping - RFC 1350
	{
	 "thecircle", 0},	// The Circle - P2P application -
	// http://thecircle.org.au
#ifdef HAVE_OPENDPI
	{
	 "thunder", 2},
#endif
	{
	 "thunder5_see", 0},
	{
	 "thunder5_tcp", 0},
	{
	 "tonghuashun", 0},	// Tonghuashun - stock analysis and trading; Chinese - http://www.10jqka.com.cn
	{
	 "tor", 0},		// Tor - The Onion Router - used for
	// anonymization - http://tor.eff.org
#ifdef HAVE_OPENDPI
	{
	 "truphone", 2},
#endif
	{
	 "tsp", 0},		// TSP - Berkely UNIX Time Synchronization
	// Protocol
#ifdef HAVE_OPENDPI
	{
	 "tvants", 2},
	{
	 "tvuplayer", 2},
	{
	 "twitter", 2},
#endif
	{
	 "unknown", 0},		// -
#ifdef HAVE_OPENDPI
	{
	 "upnp", 2},		// LPD - Line Printer Daemon Protocol
#endif
	{
	 "uucp", 0},		// UUCP - Unix to Unix Copy
#ifdef HAVE_OPENDPI
	{
	 "usenet", 2},
#endif
	{
	 "validcertssl", 0},	// Valid certificate SSL
	{
	 "ventrilo", 0},	// Ventrilo - VoIP - http://ventrilo.com
#ifdef HAVE_OPENDPI
	{
	 "veohtv", 2},
#endif
	{
	 "vnc", DPI},		// VNC - Virtual Network Computing. Also
	// known as RFB - Remote Frame Buffer
#ifdef HAVE_OPENDPI
	{
	 "warcraft3", 2},
#endif
#ifdef HAVE_OPENDPI
	{
	 "webex", 2},
#endif
	{
	 "webmail_163", 0},
	{
	 "webmail_gmail", 0},
	{
	 "webmail_hinet", 0},
	{
	 "webmail_hotmail", 0},
	{
	 "webmail_pchome", 0},
	{
	 "webmail_qq", 0},
	{
	 "webmail_seednet", 0},
	{
	 "webmail_sina", 0},
	{
	 "webmail_sohu", 0},
	{
	 "webmail_tom", 0},
	{
	 "webmail_url", 0},
	{
	 "webmail_yahoo", 0},
	{
	 "webmail_yam", 0},
#ifdef HAVE_OPENDPI
	{
	 "whatOAsapp", 2},
#endif
	{
	 "whois", 0},		// Whois - query/response system, usually
	// used for domain name info - RFC 3912
#ifdef HAVE_OPENDPI
	{
	 "windowsmedia", 2},
#endif
	{
	 "winmx", PDPI},		// Whois - query/response system, usually
#ifdef HAVE_OPENDPI
	{
	 "winupdate", 2},
#endif
#ifdef HAVE_OPENDPI
	{
	 "wokf", 2},
#endif
	{
	 "worldofwarcraft", DPI},	// World of Warcraft - popular network game - 
	// http://blizzard.com/
	{
	 "x11", 0},		// X Windows Version 11 - Networked GUI
	// system used in most Unices
	{
	 "xboxlive", 0},	// XBox Live - Console gaming
	{
	 "xdcc", 1},		// XBox Live - Console gaming
#ifdef HAVE_OPENDPI
	{
	 "xdmcp", 2},		// XBox Live - Console gaming
#endif
	{
	 "xunlei", 0},		// Xunlei - Chinese P2P filesharing -
	// http://xunlei.com
	{
	 "yahoo", DPI},		// Yahoo messenger - an instant messenger
	// protocol - http://yahoo.com
	{
	 "yahoo_camera", 0},
	{
	 "yahoo_file", 0},
	{
	 "yahoo_login", 0},
	{
	 "yahoo_voice", 0},
	{
	 "youtube", 0},
#ifdef HAVE_OPENDPI
	{
	 "zattoo", 2},
#endif
	{
	 "zip", 0},		// ZIP - (PK|Win)Zip archive format
	{
	 "zmaap", 0},		// ZMAAP - Zeroconf Multicast Address
	// Allocation Protocol
	{
	 0, 0}
};
