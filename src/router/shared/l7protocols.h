/*
 * L7-filter Supported Protocols 
 */
    /*
     * 2009-05-28 
     */

typedef struct _l7filters	// l7 and p2p filters
{

	char *name;
	int protocol;  // 1=p2p, 0=l7

} l7filters;

//Added ,  (in extra), dazhihui, .

l7filters filters_list[] = {

	{
	 "100bao", 0},	// 100bao - a Chinese P2P protocol/program -
	// http://www.100bao.com
	{
	 "aim", 0},		// AIM - AOL instant messenger (OSCAR and
	// TOC)
	{
	 "aimwebcontent", 0},	// AIM web content - ads/news content
	// downloaded by AOL Instant Messenger
	{
	 "applejuice", 1},	// Apple Juice - P2P filesharing -
	// http://www.applejuicenet.de
	{
	 "ares", 1},	// Ares - P2P filesharing -
	// http://aresgalaxy.sf.net
	{
	 "armagetron", 0},	// Armagetron Advanced - open source
	// Tron/snake based multiplayer game
	{
	 "audiogalaxy", 0},	// Audiogalaxy - (defunct) Peer to Peer
	// filesharing
	{
	 "battlefield1942", 0},	// Battlefield 1942 - An EA game
	{
	 "battlefield2", 0},	// Battlefield 2 - An EA game.
	{
	 "battlefield2142", 0},	// Battlefield 2142 - An EA game.
	{
	 "bgp", 0},		// BGP - Border Gateway Protocol - RFC 1771
	{
	 "biff", 0},		// Biff - new mail notification
	{
	 "bittorrent", 1},	// Bittorrent - P2P filesharing / publishing
	// tool - http://www.bittorrent.com
	{
	 "bt", 1},
	{
	 "bt1", 1},
	{
	 "bt2", 1},
	{
	 "bt3", 1},	 
	{
	 "chikka", 0},	// Chikka - SMS service which can be used
	// without phones - http://chikka.com
	{
	 "cimd", 0},		// Computer Interface to Message
	// Distribution, an SMSC protocol by Nokia
	{
	 "ciscovpn", 0},	// Cisco VPN - VPN client software to a Cisco 
	// VPN server
	{
	 "citrix", 0},	// Citrix ICA - proprietary remote desktop
	// application - http://citrix.com
	{
	 "clubbox", 0},
	{
	 "code_red", 0},	// Code Red - a worm that attacks Microsoft
	// IIS web servers
	{
	 "counterstrike-source", 0},	// Counterstrike (using the new
	// "Source" engine) - network game
	{
	 "cvs", 0},		// CVS - Concurrent Versions System
	{
	 "dayofdefeat-source", 0},	// Day of Defeat: Source - game
	// (Half-Life 2 mod) -
	// http://www.valvesoftware.com
	{
	 "dazhihui", 0}, // Dazhihui - stock analysis and trading; Chinese - http://www.gw.com.cn
	{
	 "dhcp", 0},		// DHCP - Dynamic Host Configuration Protocol 
	// - RFC 1541
	{
	 "directconnect", 1},	// Direct Connect - P2P filesharing -
	// http://www.neo-modus.com
	{
	 "dns", 0},		// DNS - Domain Name System - RFC 1035
	{
	 "doom3", 0},	// Doom 3 - computer game
	{
	 "edonkey", 1},	// eDonkey2000 - P2P filesharing -
	// http://edonkey2000.com and others
	{
	 "exe", 0},		// Executable - Microsoft PE file format.
	{
	 "fasttrack", 0},	// FastTrack - P2P filesharing (Kazaa,
	// Morpheus, iMesh, Grokster, etc)
	{
	 "finger", 0},	// Finger - User information server - RFC
	// 1288
	{
	 "flash", 0},	// Flash - Macromedia Flash.
	{
	 "freenet", 0},	// Freenet - Anonymous information retrieval
	// - http://freenetproject.org
	{
	 "freegate_dns", 0},
	{
	 "freegate_http", 0},
	{
	 "ftp", 0},		// FTP - File Transfer Protocol - RFC 959
	{
	 "gif", 0},		// GIF - Popular Image format.
	{
	 "gkrellm", 0},	// Gkrellm - a system monitor -
	// http://gkrellm.net
	{
	 "gnucleuslan", 0},	// GnucleusLAN - LAN-only P2P filesharing
	{
	 "gnutella", 1},	// Gnutella - P2P filesharing
	{
	 "goboogy", 0},	// GoBoogy - a Korean P2P protocol
	{
	 "gogobox", 0},
	{
	 "gopher", 0},	// Gopher - A precursor to HTTP - RFC 1436
	{
	 "gtalk", 0},    // GTalk, a Jabber (XMPP) client
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
	{
	 "guildwars", 0},	// Guild Wars - online game - http://guildwars.com
	{
	 "h323", 0},		// H.323 - Voice over IP.
	{
	 "hamachi1", 0},
	{
	 "halflife2-deathmatch", 0},	// Half-Life 2 Deathmatch - popular
	// computer game
	{
	 "hddtemp", 0},	// hddtemp - Hard drive temperature
	// reporting
	{
	 "hotline", 0},	// Hotline - An old P2P filesharing protocol
	{
	 "hotspot-shield", 0},
	{
	 "html", 0},		// (X)HTML - (Extensible) Hypertext Markup
	// Language - http://w3.org
	{
	 "http-rtsp", 0},	// RTSP tunneled within HTTP
	{
	 "http", 0},		// HTTP - HyperText Transfer Protocol - RFC
	// 2616
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
	{
	 "icq_file", 0},
	{
	 "icq_file_1", 0},
	{
	 "icq_file_2", 0},
	{
	 "icq_login", 0},
	{
	 "ident", 0},	// Ident - Identification Protocol - RFC
	// 1413
	{
	 "imap", 0},		// IMAP - Internet Message Access Protocol (A 
	// common e-mail protocol)
	{
	 "imesh", 0},	// iMesh - the native protocol of iMesh, a
	// P2P application - http://imesh.com
	{
	 "ipp", 0},		// IP printing - a new standard for UNIX
	// printing - RFC 2911
	{
	 "irc", 0},		// IRC - Internet Relay Chat - RFC 1459
	{
	 "jabber", 0},	// Jabber (XMPP) - open instant messenger
	// protocol - RFC 3920 - http://jabber.org
	{
	 "jpeg", 0},		// JPEG - Joint Picture Expert Group image
	// format.
	{
	 "kugoo", 0},	// KuGoo - a Chinese P2P program -
	// http://www.kugoo.com
	{
	 "live365", 0},	// live365 - An Internet radio site -
	// http://live365.com
	{
	 "liveforspeed", 0},	// Live For Speed - A racing game.
	{
	 "lpd", 0},		// LPD - Line Printer Daemon Protocol
	// (old-style UNIX printing) - RFC 1179
	{
	 "mohaa", 0},	// Medal of Honor Allied Assault - an
	// Electronic Arts game
	{
	 "mp3", 0},		// MP3 - Moving Picture Experts Group Audio
	// Layer III
	{
	 "msn-filetransfer", 0},	// MSN (Micosoft Network) Messenger file
	// transfers (MSNFTP and MSNSLP)
	{
	 "msnmessenger", 0},	// MSN Messenger - Microsoft Network chat
	// client
	{
	 "mute", 1},	// MUTE - P2P filesharing -
	// http://mute-net.sourceforge.net
	{
	 "napster", 0},	// Napster - P2P filesharing
	{
	 "nbns", 0},		// NBNS - NetBIOS name service
	{
	 "ncp", 0},		// NCP - Novell Core Protocol
	{
	 "netbios", 0},	// NetBIOS - Network Basic Input Output
	// System
	{
	 "nimda", 0},	// Nimda - a worm that attacks Microsoft IIS
	// web servers, and MORE!
	{
	 "nntp", 0},		// NNTP - Network News Transfer Protocol -
	// RFCs 977 and 2980
	{
	 "ntp", 0},		// (S)NTP - (Simple) Network Time Protocol -
	// RFCs 1305 and 2030
	{
	 "ogg", 0},		// Ogg - Ogg Vorbis music format (not any ogg 
	// file, just vorbis)
	{
	 "openft", 0},	// OpenFT - P2P filesharing (implemented in
	// giFT library)
	{
	 "pcanywhere", 0},	// pcAnywhere - Symantec remote access
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
	 "pop3", 0},		// POP3 - Post Office Protocol version 3
	// (popular e-mail protocol) - RFC 1939
	{
	 "postscript", 0},	// Postscript - Printing Language
	{
	 "pplive", 0},	// PPLive - Chinese P2P streaming video - http://pplive.com
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
	 "qq", 0},		// Tencent QQ Protocol - Chinese instant
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
	 "qqlive", 0},
	{
	 "qqlive2", 0},
	{
	 "quake-halflife", 0},	// Half Life 1 engine games (HL 1, Quake
	// 2/3/World, Counterstrike 1.6, etc.)
	{
	 "quake1", 0},	// Quake 1 - A popular computer game.
	{
	 "quicktime", 0},	// Quicktime HTTP
	{
	 "radmin", 0},	// Famatech Remote Administrator - remote
	// desktop for MS Windows
	{
	 "rar", 0},		// RAR - The WinRAR archive format
	{
	 "rdp", 0},		// RDP - Remote Desktop Protocol (used in
	// Windows Terminal Services)
	{
	 "replaytv-ivs", 0},	// ReplayTV Internet Video Sharing - Digital
	// Video Recorder - http://replaytv.com
	{
	 "rlogin", 0},	// rlogin - remote login - RFC 1282
	{
	 "rpm", 0},		// RPM - Redhat Package Management packages
	{
	 "rtf", 0},		// RTF - Rich Text Format - an open document
	// format
	{
	 "rtp", 0},		// RTP - Real-time Transport Protocol - RFC
	// 3550
	{
	 "rtsp", 0},		// RTSP - Real Time Streaming Protocol -
	// http://www.rtsp.org - RFC 2326
	{
	 "runesofmagic", 0},  // Runes of Magic - game - http://www.runesofmagic.com
	{
	 "shoutcast", 0},	// Shoutcast and Icecast - streaming audio
	{
	 "sip", 0},		// SIP - Session Initiation Protocol -
	// Internet telephony - RFC 3261
	{
	 "skypeout", 0},	// Skype to phone - UDP voice call (program
	// to POTS phone) - http://skype.com
	{
	 "skypetoskype", 0},	// Skype to Skype - UDP voice call (program
	// to program) - http://skype.com
	{
	 "smb", 0},		// Samba/SMB - Server Message Block -
	// Microsoft Windows filesharing
	{
	 "smtp", 0},		// SMTP - Simple Mail Transfer Protocol - RFC 
	// 2821 (See also RFC 1869)
	{
	 "snmp", 0},		// SNMP - Simple Network Management Protocol
	// - RFC 1157
	{
	 "snmp-mon", 0},	// SNMP Monitoring - Simple Network
	// Management Protocol (RFC1157)
	{
	 "snmp-trap", 0},	// SNMP Traps - Simple Network Management
	// Protocol (RFC1157)
	{
	 "socks", 0},	// SOCKS Version 5 - Firewall traversal
	// protocol - RFC 1928
	{
	 "soribada", 0},	// Soribada - A Korean P2P filesharing
	// program/protocol -
	// http://www.soribada.com
	{
	 "soulseek", 1},	// Soulseek - P2P filesharing -
	// http://slsknet.org
	{
	 "ssdp", 0},		// SSDP - Simple Service Discovery Protocol - 
	// easy discovery of network devices
	{
	 "ssh", 0},		// SSH - Secure SHell
	{
	 "ssl", 0},		// SSL and TLS - Secure Socket Layer /
	// Transport Layer Security - RFC 2246
	{
	 "stun", 0},		// STUN - Simple Traversal of UDP Through NAT 
	// - RFC 3489
	{
	 "subspace", 0},	// Subspace - 2D asteroids-style space game - 
	// http://sscentral.com
	{
	 "subversion", 0},	// Subversion - a version control system
	{
	 "tar", 0},		// Tar - tape archive. Standard UNIX file
	// archiver, not just for tapes.
	{
	 "teamfortress2", 0},	// Team Fortress 2 - network game -
	// http://www.valvesoftware.com
	{
	 "teamspeak", 0},	// TeamSpeak - VoIP application -
	{
	 "teamviewer", 0},
	{
	 "teamviewer1", 0},
	{
	 "telnet", 0},	// Telnet - Insecure remote login - RFC 854
	{
	 "tesla", 0},	// Tesla Advanced Communication - P2P
	// filesharing (?)
	{
	 "tftp", 0},		// TFTP - Trivial File Transfer Protocol -
	// used for bootstrapping - RFC 1350
	{
	 "thecircle", 0},	// The Circle - P2P application -
	// http://thecircle.org.au
	{
	 "thunder5_see", 0},
	{
	 "thunder5_tcp", 0},
	{
	 "tonghuashun", 0},  // Tonghuashun - stock analysis and trading; Chinese - http://www.10jqka.com.cn
	{
	 "tor", 0},		// Tor - The Onion Router - used for
	// anonymization - http://tor.eff.org
	{
	 "tsp", 0},		// TSP - Berkely UNIX Time Synchronization
	// Protocol
	{
	 "unknown", 0},	// -
	{
	 "uucp", 0},		// UUCP - Unix to Unix Copy
	{
	 "validcertssl", 0},	// Valid certificate SSL
	{
	 "ventrilo", 0},	// Ventrilo - VoIP - http://ventrilo.com
	{
	 "vnc", 0},		// VNC - Virtual Network Computing. Also
	// known as RFB - Remote Frame Buffer
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
	{
	 "whois", 0},	// Whois - query/response system, usually
	// used for domain name info - RFC 3912
	{
	 "winmx", 1},	// Whois - query/response system, usually
	{
	 "worldofwarcraft", 0},	// World of Warcraft - popular network game - 
	// http://blizzard.com/
	{
	 "x11", 0},		// X Windows Version 11 - Networked GUI
	// system used in most Unices
	{
	 "xboxlive", 0},	// XBox Live - Console gaming
	{
	 "xdcc", 1},	// XBox Live - Console gaming
	{
	 "xunlei", 0},	// Xunlei - Chinese P2P filesharing -
	// http://xunlei.com
	{
	 "yahoo", 0},	// Yahoo messenger - an instant messenger
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
	 "zip", 0},		// ZIP - (PK|Win)Zip archive format
	{
	 "zmaap", 0},	// ZMAAP - Zeroconf Multicast Address
	// Allocation Protocol
	{
	 0, 0}
};
