/*
 * L7-filter Supported Protocols 
 */  
    /*
     * 2007-11-22 
     */ 
    
typedef struct _l7filters	// l7 and p2p filters
{
    
char *name;
     
char *protocol;
 
} l7filters;

//Added ,  (in extra), dazhihui, .

l7filters filters_list[] =
{
    
    {
    "100bao", "l7"},		// 100bao - a Chinese P2P protocol/program -
				// http://www.100bao.com
    {
    "aim", "l7"},		// AIM - AOL instant messenger (OSCAR and
				// TOC)
    {
    "aimwebcontent", "l7"},	// AIM web content - ads/news content
				// downloaded by AOL Instant Messenger
    {
    "applejuice", "p2p"},	// Apple Juice - P2P filesharing -
				// http://www.applejuicenet.de
    {
    "ares", "p2p"},		// Ares - P2P filesharing -
				// http://aresgalaxy.sf.net
    {
    "armagetron", "l7"},	// Armagetron Advanced - open source
				// Tron/snake based multiplayer game
    {
    "audiogalaxy", "l7"},	// Audiogalaxy - (defunct) Peer to Peer
				// filesharing
    {
    "battlefield1942", "l7"},	// Battlefield 1942 - An EA game
    {
    "battlefield2", "l7"},	// Battlefield 2 - An EA game.
    {
    "battlefield2142", "l7"},	// Battlefield 2142 - An EA game.
    {
    "bgp", "l7"},		// BGP - Border Gateway Protocol - RFC 1771
    {
    "biff", "l7"},		// Biff - new mail notification
    {
    "bittorrent", "p2p"},	// Bittorrent - P2P filesharing / publishing
				// tool - http://www.bittorrent.com
    {
    "chikka", "l7"},		// Chikka - SMS service which can be used
				// without phones - http://chikka.com
    {
    "cimd", "l7"},		// Computer Interface to Message
				// Distribution, an SMSC protocol by Nokia
    {
    "ciscovpn", "l7"},		// Cisco VPN - VPN client software to a Cisco 
				// VPN server
    {
    "citrix", "l7"},		// Citrix ICA - proprietary remote desktop
				// application - http://citrix.com
    {
    "code_red", "l7"},		// Code Red - a worm that attacks Microsoft
				// IIS web servers
    {
    "counterstrike-source", "l7"},	// Counterstrike (using the new
					// "Source" engine) - network game
    {
    "cvs", "l7"},		// CVS - Concurrent Versions System
    {
    "dayofdefeat-source", "l7"},	// Day of Defeat: Source - game
					// (Half-Life 2 mod) -
					// http://www.valvesoftware.com
    {
    "dazhihui","l7"},
    {
    "dhcp", "l7"},		// DHCP - Dynamic Host Configuration Protocol 
				// - RFC 1541
    {
    "directconnect", "p2p"},	// Direct Connect - P2P filesharing -
				// http://www.neo-modus.com
    {
    "dns", "l7"},		// DNS - Domain Name System - RFC 1035
    {
    "doom3", "l7"},		// Doom 3 - computer game
    {
    "edonkey", "p2p"},		// eDonkey2000 - P2P filesharing -
				// http://edonkey2000.com and others
    {
    "exe", "l7"},		// Executable - Microsoft PE file format.
    {
    "fasttrack", "l7"},		// FastTrack - P2P filesharing (Kazaa,
				// Morpheus, iMesh, Grokster, etc)
    {
    "finger", "l7"},		// Finger - User information server - RFC
				// 1288
    {
    "flash", "l7"},		// Flash - Macromedia Flash.
    {
    "freenet", "l7"},		// Freenet - Anonymous information retrieval
				// - http://freenetproject.org
    {
    "ftp", "l7"},		// FTP - File Transfer Protocol - RFC 959
    {
    "gif", "l7"},		// GIF - Popular Image format.
    {
    "gkrellm", "l7"},		// Gkrellm - a system monitor -
				// http://gkrellm.net
    {
    "gnucleuslan", "l7"},	// GnucleusLAN - LAN-only P2P filesharing
    {
    "gnutella", "p2p"},		// Gnutella - P2P filesharing
    {
    "goboogy", "l7"},		// GoBoogy - a Korean P2P protocol
    {
    "gopher", "l7"},		// Gopher - A precursor to HTTP - RFC 1436
    {
    "gtalk", "l7"},
    {
    "guildwars", "l7"},		// Guild Wars - online game - http://guildwars.com
    {
    "h323", "l7"},		// H.323 - Voice over IP.
    {
    "halflife2-deathmatch", "l7"},	// Half-Life 2 Deathmatch - popular
					// computer game
    {
    "hddtemp", "l7"},		// hddtemp - Hard drive temperature
				// reporting
    {
    "hotline", "l7"},		// Hotline - An old P2P filesharing protocol
    {
    "html", "l7"},		// (X)HTML - (Extensible) Hypertext Markup
				// Language - http://w3.org
    {
    "http-rtsp", "l7"},		// RTSP tunneled within HTTP
    {
    "http", "l7"},		// HTTP - HyperText Transfer Protocol - RFC
				// 2616
    {
    "http-dap", "l7"},		// HTTP by Download Accelerator Plus -
				// http://www.speedbit.com
    {
    "http-freshdownload", "l7"},	// HTTP by Fresh Download -
					// http://www.freshdevices.com
    {
    "http-itunes", "l7"},	// HTTP - iTunes (Apple's music program)
    {
    "httpaudio", "l7"},		// HTTP - Audio over HyperText Transfer
				// Protocol (RFC 2616)
    {
    "httpcachehit", "l7"},	// HTTP - Proxy Cache hit for HyperText
				// Transfer Protocol (RFC 2616)
    {
    "httpcachemiss", "l7"},	// HTTP - Proxy Cache miss for HyperText
				// Transfer Protocol (RFC 2616)
    {
    "httpvideo", "l7"},		// HTTP - Video over HyperText Transfer
				// Protocol (RFC 2616)
    {
    "ident", "l7"},		// Ident - Identification Protocol - RFC
				// 1413
    {
    "imap", "l7"},		// IMAP - Internet Message Access Protocol (A 
				// common e-mail protocol)
    {
    "imesh", "l7"},		// iMesh - the native protocol of iMesh, a
				// P2P application - http://imesh.com
    {
    "ipp", "l7"},		// IP printing - a new standard for UNIX
				// printing - RFC 2911
    {
    "irc", "l7"},		// IRC - Internet Relay Chat - RFC 1459
    {
    "jabber", "l7"},		// Jabber (XMPP) - open instant messenger
				// protocol - RFC 3920 - http://jabber.org
    {
    "jpeg", "l7"},		// JPEG - Joint Picture Expert Group image
				// format.
    {
    "kugoo", "l7"},		// KuGoo - a Chinese P2P program -
				// http://www.kugoo.com
    {
    "live365", "l7"},		// live365 - An Internet radio site -
				// http://live365.com
    {
    "liveforspeed", "l7"},	// Live For Speed - A racing game.
    {
    "lpd", "l7"},		// LPD - Line Printer Daemon Protocol
				// (old-style UNIX printing) - RFC 1179
    {
    "mohaa", "l7"},		// Medal of Honor Allied Assault - an
				// Electronic Arts game
    {
    "mp3", "L7"},		// MP3 - Moving Picture Experts Group Audio
				// Layer III
    {
    "msn-filetransfer", "l7"},	// MSN (Micosoft Network) Messenger file
				// transfers (MSNFTP and MSNSLP)
    {
    "msnmessenger", "l7"},	// MSN Messenger - Microsoft Network chat
				// client
    {
    "mute", "p2p"},		// MUTE - P2P filesharing -
				// http://mute-net.sourceforge.net
    {
    "napster", "l7"},		// Napster - P2P filesharing
    {
    "nbns", "l7"},		// NBNS - NetBIOS name service
    {
    "ncp", "l7"},		// NCP - Novell Core Protocol
    {
    "netbios", "l7"},		// NetBIOS - Network Basic Input Output
				// System
    {
    "nimda", "l7"},		// Nimda - a worm that attacks Microsoft IIS
				// web servers, and MORE!
    {
    "nntp", "l7"},		// NNTP - Network News Transfer Protocol -
				// RFCs 977 and 2980
    {
    "ntp", "l7"},		// (S)NTP - (Simple) Network Time Protocol -
				// RFCs 1305 and 2030
    {
    "ogg", "l7"},		// Ogg - Ogg Vorbis music format (not any ogg 
				// file, just vorbis)
    {
    "openft", "l7"},		// OpenFT - P2P filesharing (implemented in
				// giFT library)
    {
    "pcanywhere", "l7"},	// pcAnywhere - Symantec remote access
				// program
    {
    "pdf", "l7"},		// PDF - Portable Document Format -
				// Postscript-like format by Adobe
    {
    "perl", "l7"},		// Perl - A scripting language by Larry
				// Wall.
    {
    "poco", "l7"},		// POCO and PP365 - Chinese P2P filesharing - 
				// http://pp365.com http://poco.cn
    {
    "png", "l7"},		// PNG - Portable Network Graphics, a popular 
				// image format
    {
    "pop3", "l7"},		// POP3 - Post Office Protocol version 3
				// (popular e-mail protocol) - RFC 1939
    {
    "postscript", "l7"},	// Postscript - Printing Language
    {
    "pplive", "l7"},		// PPLive - Chinese P2P streaming video - http://pplive.com
    {
    "pressplay", "l7"},		// pressplay - A legal music distribution
    {
    "qq", "l7"},		// Tencent QQ Protocol - Chinese instant
				// messenger protocol - http://www.qq.com
				// site - http://pressplay.com
    {
    "quake-halflife", "l7"},	// Half Life 1 engine games (HL 1, Quake
				// 2/3/World, Counterstrike 1.6, etc.)
    {
    "quake1", "l7"},		// Quake 1 - A popular computer game.
    {
    "quicktime", "l7"},		// Quicktime HTTP
    {
    "radmin", "l7"},		// Famatech Remote Administrator - remote
				// desktop for MS Windows
    {
    "rar", "l7"},		// RAR - The WinRAR archive format
    {
    "rdp", "l7"},		// RDP - Remote Desktop Protocol (used in
				// Windows Terminal Services)
    {
    "replaytv-ivs", "l7"},	// ReplayTV Internet Video Sharing - Digital
				// Video Recorder - http://replaytv.com
    {
    "rlogin", "l7"},		// rlogin - remote login - RFC 1282
    {
    "rpm", "l7"},		// RPM - Redhat Package Management packages
    {
    "rtf", "l7"},		// RTF - Rich Text Format - an open document
				// format
    {
    "rtp", "l7"},		// RTP - Real-time Transport Protocol - RFC
				// 3550
    {
    "rtsp", "l7"},		// RTSP - Real Time Streaming Protocol -
				// http://www.rtsp.org - RFC 2326
    {
    "runesofmagic", "l7"},    
    {
    "shoutcast", "l7"},		// Shoutcast and Icecast - streaming audio
    {
    "sip", "l7"},		// SIP - Session Initiation Protocol -
				// Internet telephony - RFC 3261
    {
    "skypeout", "l7"},		// Skype to phone - UDP voice call (program
				// to POTS phone) - http://skype.com
    {
    "skypetoskype", "l7"},	// Skype to Skype - UDP voice call (program
				// to program) - http://skype.com
    {
    "smb", "l7"},		// Samba/SMB - Server Message Block -
				// Microsoft Windows filesharing
    {
    "smtp", "l7"},		// SMTP - Simple Mail Transfer Protocol - RFC 
				// 2821 (See also RFC 1869)
    {
    "snmp", "l7"},		// SNMP - Simple Network Management Protocol
				// - RFC 1157
    {
    "snmp-mon", "l7"},		// SNMP Monitoring - Simple Network
				// Management Protocol (RFC1157)
    {
    "snmp-trap", "l7"},		// SNMP Traps - Simple Network Management
				// Protocol (RFC1157)
    {
    "socks", "l7"},		// SOCKS Version 5 - Firewall traversal
				// protocol - RFC 1928
    {
    "soribada", "l7"},		// Soribada - A Korean P2P filesharing
				// program/protocol -
				// http://www.soribada.com
    {
    "soulseek", "p2p"},		// Soulseek - P2P filesharing -
				// http://slsknet.org
    {
    "ssdp", "l7"},		// SSDP - Simple Service Discovery Protocol - 
				// easy discovery of network devices
    {
    "ssh", "l7"},		// SSH - Secure SHell
    {
    "ssl", "l7"},		// SSL and TLS - Secure Socket Layer /
				// Transport Layer Security - RFC 2246
    {
    "stun", "l7"},		// STUN - Simple Traversal of UDP Through NAT 
				// - RFC 3489
    {
    "subspace", "l7"},		// Subspace - 2D asteroids-style space game - 
				// http://sscentral.com
    {
    "subversion", "l7"},	// Subversion - a version control system
    {
    "tar", "l7"},		// Tar - tape archive. Standard UNIX file
				// archiver, not just for tapes.
    {
    "teamfortress2", "l7"},	// Team Fortress 2 - network game -
				// http://www.valvesoftware.com
    {
    "teamspeak", "l7"},		// TeamSpeak - VoIP application -
				// http://goteamspeak.com
    {
    "telnet", "l7"},		// Telnet - Insecure remote login - RFC 854
    {
    "tesla", "l7"},		// Tesla Advanced Communication - P2P
				// filesharing (?)
    {
    "tftp", "l7"},		// TFTP - Trivial File Transfer Protocol -
				// used for bootstrapping - RFC 1350
    {
    "thecircle", "l7"},	// The Circle - P2P application -
				// http://thecircle.org.au
    {"tonghuashun","l7"},
    
    {
    "tor", "l7"},		// Tor - The Onion Router - used for
				// anonymization - http://tor.eff.org
    {
    "tsp", "l7"},		// TSP - Berkely UNIX Time Synchronization
				// Protocol
    {
    "unknown", "l7"},		// -
    {
    "uucp", "l7"},		// UUCP - Unix to Unix Copy
    {
    "validcertssl", "l7"},	// Valid certificate SSL
    {
    "ventrilo", "l7"},		// Ventrilo - VoIP - http://ventrilo.com
    {
    "vnc", "l7"},		// VNC - Virtual Network Computing. Also
				// known as RFB - Remote Frame Buffer
    {
    "whois", "l7"},		// Whois - query/response system, usually
				// used for domain name info - RFC 3912
    {
    "winmx", "p2p"},		// Whois - query/response system, usually

    {
    "worldofwarcraft", "l7"},	// World of Warcraft - popular network game - 
				// http://blizzard.com/
    {
    "x11", "l7"},		// X Windows Version 11 - Networked GUI
				// system used in most Unices
    {
    "xboxlive", "l7"},		// XBox Live - Console gaming
    {
    "xdcc", "p2p"},		// XBox Live - Console gaming
    {
    "xunlei", "l7"},		// Xunlei - Chinese P2P filesharing -
				// http://xunlei.com
    {
    "yahoo", "l7"},		// Yahoo messenger - an instant messenger
				// protocol - http://yahoo.com
    {
    "zip", "l7"},		// ZIP - (PK|Win)Zip archive format
    {
    "zmaap", "l7"},		// ZMAAP - Zeroconf Multicast Address
				// Allocation Protocol
    {
0, 0} 
};
