
/*
 * L7-filter Supported Protocols 
 */
    /*
     * 2009-05-28 
     */

typedef struct _l7filters {

	char *name;
	unsigned short protocol:2;	// 1=p2p, 0=l7, 2=opendpi
	unsigned short level:14;
	char *matchdep;		// for risk only
} l7filters;
#define L7_ONLY 0
#define PDPI_ONLY 1
#define NDPI_ONLY 2
#define NDPI_RISK 3

#ifdef HAVE_OPENDPI
#define DPI 2			//open dpi based
#define PDPI 2			//open dpi based
#else
#define DPI 0			//default l7
#define PDPI 1			//default p2p
#endif
//Added ,  (in extra), dazhihui, .

l7filters filters_list[] = {
	{ "100bao", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "1kxun", NDPI_ONLY, 0, NULL },
	{ "accuweather", NDPI_ONLY, 0, NULL },
	{ "activision", NDPI_ONLY, 0, NULL },
	{ "ads_analytics_track", NDPI_ONLY, 0, NULL },
	{ "adult_content", NDPI_ONLY, 0, NULL },
	{ "afp", NDPI_ONLY, 0, NULL },
#endif
	{ "aim", L7_ONLY, 0, NULL },
	{ "aimwebcontent", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "ajp", NDPI_ONLY, 0, NULL },
	{ "alibaba", NDPI_ONLY, 0, NULL },
	{ "alicloud", NDPI_ONLY, 0, NULL },
	{ "amazon", NDPI_ONLY, 0, NULL },
	{ "amazonalexa", NDPI_ONLY, 0, NULL },
	{ "amazonaws", NDPI_ONLY, 0, NULL },
	{ "amazonvideo", NDPI_ONLY, 0, NULL },
	{ "amongus", NDPI_ONLY, 0, NULL },
	{ "amqp", NDPI_ONLY, 0, NULL },
	{ "anonymous subscriber", NDPI_RISK, 45, "icloud_private_relay" },
	{ "anydesk", NDPI_ONLY, 0, NULL },
	{ "apache_thrift", NDPI_ONLY, 0, NULL },
	{ "apple", NDPI_ONLY, 0, NULL },
	{ "appleicloud", NDPI_ONLY, 0, NULL },
	{ "appleitunes", NDPI_ONLY, 0, NULL },
#endif
#ifdef HAVE_OPENDPI
	{ "applepush", NDPI_ONLY, 0, NULL },
	{ "applesiri", NDPI_ONLY, 0, NULL },
	{ "applestore", NDPI_ONLY, 0, NULL },
	{ "appletvplus", NDPI_ONLY, 0, NULL },
#endif
	{ "ares", PDPI_ONLY, 0, NULL },
	{ "armagetron", DPI, 0, NULL },
	{ "audiogalaxy", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "avast", NDPI_ONLY, 0, NULL },
	{ "avastsecuredns", NDPI_ONLY, 0, NULL },
	{ "azure", NDPI_ONLY, 0, NULL },
	{ "bacnet", NDPI_ONLY, 0, NULL },
	{ "badoo", NDPI_ONLY, 0, NULL },
#endif
	{ "battlefield1942", L7_ONLY, 0, NULL },
	{ "battlefield2", L7_ONLY, 0, NULL },
	{ "battlefield2142", L7_ONLY, 0, NULL },
	{ "bearshare", PDPI_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "beckhoff_ads", NDPI_ONLY, 0, NULL },
#endif
	{ "bgp", DPI, 0, NULL },
	{ "biff", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "binary app transfer", NDPI_RISK, 4, "http" },
	{ "bitcoin", NDPI_ONLY, 0, NULL },
#endif
	{ "bittorrent", PDPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "bjnp", NDPI_ONLY, 0, NULL },
	{ "bloomberg", NDPI_ONLY, 0, NULL },
	{ "cachefly", NDPI_ONLY, 0, NULL },
	{ "can", NDPI_ONLY, 0, NULL },
	{ "capwap", NDPI_ONLY, 0, NULL },
	{ "cassandra", NDPI_ONLY, 0, NULL },
	{ "checkmk", NDPI_ONLY, 0, NULL },
#endif
	{ "chikka", L7_ONLY, 0, NULL },
	{ "cimd", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "ciscoskinny", NDPI_ONLY, 0, NULL },
#endif
	{ "ciscovpn", DPI, 0, NULL },
	{ "citrix", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "clear-text credentials", NDPI_RISK, 36, "telnet,rsh,imap,smtp,irc,ftp_control,http,pop" },
	{ "cloudflare", NDPI_ONLY, 0, NULL },
	{ "cloudflarewarp", NDPI_ONLY, 0, NULL },
#endif
	{ "clubbox", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "cnn", NDPI_ONLY, 0, NULL },
	{ "coap", NDPI_ONLY, 0, NULL },
#endif
	{ "code_red", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "collectd", NDPI_ONLY, 0, NULL },
	{ "corba", NDPI_ONLY, 0, NULL },
#endif
	{ "counterstrike-source", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "cpha", NDPI_ONLY, 0, NULL },
	{ "crashlytics", NDPI_ONLY, 0, NULL },
	{ "crawler/bot", NDPI_RISK, 44, "http" },
	{ "crossfire", NDPI_ONLY, 0, NULL },
	{ "crynet", NDPI_ONLY, 0, NULL },
	{ "csgo", NDPI_ONLY, 0, NULL },
#endif
	{ "cvs", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "cybersec", NDPI_ONLY, 0, NULL },
	{ "datasaver", NDPI_ONLY, 0, NULL },
	{ "dailymotion", NDPI_ONLY, 0, NULL },
#endif
	{ "dayofdefeat-source", L7_ONLY, 0, NULL },
	{ "dazhihui", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "dazn", NDPI_ONLY, 0, NULL },
	{ "dcerpc", NDPI_ONLY, 0, NULL },
	{ "deezer", NDPI_ONLY, 0, NULL },
	{ "desktop/file sharing", NDPI_RISK, 30, "vnc,rdp,teamviewer" },
#endif
	{ "dhcp", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "dhcpv6", NDPI_ONLY, 0, NULL },
	{ "diameter", NDPI_ONLY, 0, NULL },
	{ "directv", NDPI_ONLY, 0, NULL },
#endif
#ifdef HAVE_OPENDPI
	{ "discord", NDPI_ONLY, 0, NULL },
	{ "disneyplus", NDPI_ONLY, 0, NULL },
	{ "dnp3", NDPI_ONLY, 0, NULL },
#endif
	{ "dns", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "dns message fragmented", NDPI_RISK, 38, "dns" },
	{ "dns packet large", NDPI_RISK, 37, "dns" },
	{ "dns susp dga domain", NDPI_RISK, 16, "dns" },
	{ "dns traffic susp", NDPI_RISK, 23, "dns" },
	{ "dnscrypt", NDPI_ONLY, 0, NULL },
	{ "dofus", NDPI_ONLY, 0, NULL },
	{ "doh_dot", NDPI_ONLY, 0, NULL },
#endif
	{ "doom3", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "drda", NDPI_ONLY, 0, NULL },
	{ "dropbox", NDPI_ONLY, 0, NULL },
	{ "dtls", NDPI_ONLY, 0, NULL },
	{ "eaq", NDPI_ONLY, 0, NULL },
	{ "ebay", NDPI_ONLY, 0, NULL },
	{ "edgecast", NDPI_ONLY, 0, NULL },
#endif
	{ "edonkey", PDPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "egp", NDPI_ONLY, 0, NULL },
	{ "elasticsearch", NDPI_ONLY, 0, NULL },
	{ "epicgames", NDPI_ONLY, 0, NULL },
	{ "ethereum", NDPI_ONLY, 0, NULL },
	{ "ethernetip", NDPI_ONLY, 0, NULL },
	{ "ethersbus", NDPI_ONLY, 0, NULL },
	{ "ethersio", NDPI_ONLY, 0, NULL },
	{ "error code", NDPI_RISK, 43, "dns,snmp,http" },
#endif
	{ "exe", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "facebook", NDPI_ONLY, 0, NULL },
	{ "facebook_reel_story", NDPI_ONLY, 0, NULL },
	{ "facebookvoip", NDPI_ONLY, 0, NULL },
#endif
#ifdef HAVE_OPENDPI
	{ "fastcgi", NDPI_ONLY, 0, NULL },
#endif
	{ "filetopia", L7_ONLY, 0, NULL },
	{ "finger", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "fins", NDPI_ONLY, 0, NULL },
	{ "fix", NDPI_ONLY, 0, NULL },
#endif
	{ "flash", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "forticlient", NDPI_ONLY, 0, NULL },
#endif
	{ "freegate_dns", L7_ONLY, 0, NULL },
	{ "freegate_http", L7_ONLY, 0, NULL },
	{ "freenet", L7_ONLY, 0, NULL },
	{ "ftp", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "ftp_control", NDPI_ONLY, 0, NULL },
	{ "ftps", NDPI_ONLY, 0, NULL },
	{ "ftp_data", NDPI_ONLY, 0, NULL },
	{ "fuze", NDPI_ONLY, 0, NULL },
	{ "genshinimpact", NDPI_ONLY, 0, NULL },
#endif
#ifdef HAVE_OPENDPI
//	{ "gambling", NDPI_ONLY, 0, NULL },
	{ "geforcenow", NDPI_ONLY, 0, NULL },
#endif
	{ "gif", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "git", NDPI_ONLY, 0, NULL },
	{ "github", NDPI_ONLY, 0, NULL },
	{ "gitlab", NDPI_ONLY, 0, NULL },
#endif
	{ "gkrellm", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "gmail", NDPI_ONLY, 0, NULL },
#endif
	{ "gnucleuslan", L7_ONLY, 0, NULL },
	{ "gnutella", PDPI, 0, NULL },
	{ "goboogy", L7_ONLY, 0, NULL },
	{ "gogobox", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "google", NDPI_ONLY, 0, NULL },
	{ "googleclassroom", NDPI_ONLY, 0, NULL },
	{ "googlecloud", NDPI_ONLY, 0, NULL },
	{ "googledocs", NDPI_ONLY, 0, NULL },
	{ "googledrive", NDPI_ONLY, 0, NULL },
	{ "googlehangoutduo", NDPI_ONLY, 0, NULL },
	{ "googlemaps", NDPI_ONLY, 0, NULL },
	{ "googleservices", NDPI_ONLY, 0, NULL },
#endif
	{ "gopher", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "goto", NDPI_ONLY, 0, NULL },
	{ "gre", NDPI_ONLY, 0, NULL },
#endif
	{ "gtalk", L7_ONLY, 0, NULL },
	{ "gtalk1", L7_ONLY, 0, NULL },
	{ "gtalk2", L7_ONLY, 0, NULL },
	{ "gtalk_file", L7_ONLY, 0, NULL },
	{ "gtalk_file_1", L7_ONLY, 0, NULL },
	{ "gtalk_vista", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "gtp", NDPI_ONLY, 0, NULL },
	{ "gtp_c", NDPI_ONLY, 0, NULL },
	{ "gtp_prime", NDPI_ONLY, 0, NULL },
	{ "gtp_u", NDPI_ONLY, 0, NULL },
#endif
	{ "guildwars", DPI, 0, NULL },
	{ "h323", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "halflife2", NDPI_ONLY, 0, NULL },
#endif
	{ "halflife2-deathmatch", L7_ONLY, 0, NULL },
	{ "hamachi1", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "haproxy", NDPI_ONLY, 0, NULL },
	{ "hart_ip", NDPI_ONLY, 0, NULL },
	{ "hbo", NDPI_ONLY, 0, NULL },
#endif
	{ "hddtemp", L7_ONLY, 0, NULL },
	{ "hotline", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "heroes_of_the_storm", NDPI_ONLY, 0, NULL },
	{ "hislip", NDPI_ONLY, 0, NULL },
	{ "hotspotshield", NDPI_ONLY, 0, NULL },
	{ "hp_virtgrp", NDPI_ONLY, 0, NULL },
	{ "hsrp", NDPI_ONLY, 0, NULL },
#else
	{ "hotspot-shield", L7_ONLY, 0, NULL },
#endif
	{ "html", L7_ONLY, 0, NULL },
	{ "http", DPI, 0, NULL },
	{ "http-dap", L7_ONLY, 0, NULL },
	{ "http-freshdownload", L7_ONLY, 0, NULL },
	{ "http-itunes", L7_ONLY, 0, NULL },
	{ "http-rtsp", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "http_connect", NDPI_ONLY, 0, NULL },
	{ "http_proxy", NDPI_ONLY, 0, NULL },
	{ "http2", NDPI_ONLY, 0, NULL },
#endif
	{ "httpaudio", L7_ONLY, 0, NULL },
	{ "httpcachehit", L7_ONLY, 0, NULL },
	{ "httpcachemiss", L7_ONLY, 0, NULL },
	{ "httpvideo", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "http obsolete server", NDPI_RISK, 47, "http" },
	{ "http susp content", NDPI_RISK, 25, "http" },
	{ "http susp header", NDPI_RISK, 14, "http" },
	{ "http susp user-agent", NDPI_RISK, 11, "http" },
	{ "http susp url", NDPI_RISK, 13, "http" },
	{ "http/tls/quic numeric hostname/sni", NDPI_RISK, 12, "http,tls,quic" },
	{ "hulu", NDPI_ONLY, 0, NULL },
	{ "i3d", NDPI_ONLY, 0, NULL },
	{ "iax", NDPI_ONLY, 0, NULL },
	{ "icecast", NDPI_ONLY, 0, NULL },
	{ "icloudprivaterelay", NDPI_ONLY, 0, NULL },
	{ "icmp", NDPI_ONLY, 0, NULL },
	{ "icmpv6", NDPI_ONLY, 0, NULL },
#endif
	{ "icq_file", L7_ONLY, 0, NULL },
	{ "icq_file_1", L7_ONLY, 0, NULL },
	{ "icq_file_2", L7_ONLY, 0, NULL },
	{ "icq_login", L7_ONLY, 0, NULL },
	{ "ident", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "idn domain name", NDPI_RISK, 42, "dns,http,quic,fastcgi,tls,smtp" },
	{ "iec60870", NDPI_ONLY, 0, NULL },
	{ "ieee_c37118", NDPI_ONLY, 0, NULL },
	{ "iflix", NDPI_ONLY, 0, NULL },
	{ "igmp", NDPI_ONLY, 0, NULL },
	{ "iheartradio", NDPI_ONLY, 0, NULL },
#endif
	{ "imap", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "imaps", NDPI_ONLY, 0, NULL },
#endif
	{ "imesh", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "imo", NDPI_ONLY, 0, NULL },
	{ "instagram", NDPI_ONLY, 0, NULL },
	{ "ip_in_ip", NDPI_ONLY, 0, NULL },
	{ "ip_pim", NDPI_ONLY, 0, NULL },
#endif
	{ "ipp", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "ipsec", NDPI_ONLY, 0, NULL },
#endif
	{ "irc", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "iso9506_1_mms", NDPI_ONLY, 0, NULL },
#endif
	{ "jabber", DPI, 0, NULL },
	{ "jpeg", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "kakaotalk", NDPI_ONLY, 0, NULL },
	{ "kakaotalk_voice", NDPI_ONLY, 0, NULL },
#endif
	{ "kazaa", PDPI_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "kerberos", NDPI_ONLY, 0, NULL },
	{ "kismet", NDPI_ONLY, 0, NULL },
	{ "known proto on non std port", NDPI_RISK, 5, "all" },
	{ "kontiki", NDPI_ONLY, 0, NULL },
#endif
	{ "kugoo", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "lastfm", NDPI_ONLY, 0, NULL },
	{ "ldap", NDPI_ONLY, 0, NULL },
	{ "likee", NDPI_ONLY, 0, NULL },
	{ "line", NDPI_ONLY, 0, NULL },
	{ "line_call", NDPI_ONLY, 0, NULL },
	{ "linkedin", NDPI_ONLY, 0, NULL },
	{ "lisp", NDPI_ONLY, 0, NULL },
#endif
	{ "live365", L7_ONLY, 0, NULL },
	{ "liveforspeed", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "livestream", NDPI_ONLY, 0, NULL },
	{ "llmnr", NDPI_ONLY, 0, NULL },
	{ "lotusnotes", NDPI_ONLY, 0, NULL },
#endif
	{ "lpd", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "malformed packet", NDPI_RISK, 17, "icmp,icmpv6,munin,tivoconnect,collectd,ipsec,tls,natpmp,fastcgi,dns,tftp,http" },
	{ "malicious ja3 fingerp.", NDPI_RISK, 28, "tls" },
	{ "malicious ssl cert/sha1 fingerp.", NDPI_RISK, 29, "tls" },
	{ "maplestory", NDPI_ONLY, 0, NULL },
	{ "mdns", NDPI_ONLY, 0, NULL },
	{ "megaco", NDPI_ONLY, 0, NULL },
	{ "memcached", NDPI_ONLY, 0, NULL },
	{ "meraki_cloud", NDPI_ONLY, 0, NULL },
	{ "messenger", NDPI_ONLY, 0, NULL },
	{ "mgcp", NDPI_ONLY, 0, NULL },
	{ "microsoft", NDPI_ONLY, 0, NULL },
	{ "microsoft365", NDPI_ONLY, 0, NULL },
	{ "mining", NDPI_ONLY, 0, NULL },
	{ "minor issues", NDPI_RISK, 49, "dns" },
	{ "missing sni tls extn", NDPI_RISK, 24, "tls" },
	{ "modbus", NDPI_ONLY, 0, NULL },
#endif
	{ "mohaa", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "monero", NDPI_ONLY, 0, NULL },
	{ "mongodb", NDPI_ONLY, 0, NULL },
	{ "mullvad", NDPI_ONLY, 0, NULL },
	{ "munin", NDPI_ONLY, 0, NULL },
#endif
	{ "mp3", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "mpeg_ts", NDPI_ONLY, 0, NULL },
	{ "mpegdash", NDPI_ONLY, 0, NULL },
	{ "mqtt", NDPI_ONLY, 0, NULL },
	{ "ms_onedrive", NDPI_ONLY, 0, NULL },
#endif
	{ "msn-filetransfer", L7_ONLY, 0, NULL },
	{ "msnmessenger", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "mssql-tds", NDPI_ONLY, 0, NULL },
#endif
	{ "mute", PDPI_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "mysql", NDPI_ONLY, 0, NULL },
#endif
	{ "napster", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "natpmp", NDPI_ONLY, 0, NULL },
	{ "nats", NDPI_ONLY, 0, NULL },
#endif
	{ "nbns", L7_ONLY, 0, NULL },
	{ "ncp", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "nestlogsink", NDPI_ONLY, 0, NULL },
#endif
	{ "netbios", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "netflix", NDPI_ONLY, 0, NULL },
	{ "netflow", NDPI_ONLY, 0, NULL },
	{ "nfs", NDPI_ONLY, 0, NULL },
#endif
	{ "nimda", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "nintendo", NDPI_ONLY, 0, NULL },
#endif
	{ "nntp", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "noe", NDPI_ONLY, 0, NULL },
	{ "ntop", NDPI_ONLY, 0, NULL },
#endif
	{ "ntp", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "nvidia", NDPI_ONLY, 0, NULL },
	{ "ocs", NDPI_ONLY, 0, NULL },
	{ "ocsp", NDPI_ONLY, 0, NULL },
#endif
	{ "ogg", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "oicq", NDPI_ONLY, 0, NULL },
	{ "ookla", NDPI_ONLY, 0, NULL },
	{ "opc_ua", NDPI_ONLY, 0, NULL },
	{ "opendns", NDPI_ONLY, 0, NULL },
	{ "openvpn", NDPI_ONLY, 0, NULL },
	{ "opera_vpn", NDPI_ONLY, 0, NULL },
	{ "oracle", NDPI_ONLY, 0, NULL },
	{ "ospf", NDPI_ONLY, 0, NULL },
	{ "outlook", NDPI_ONLY, 0, NULL },
	{ "pandora", NDPI_ONLY, 0, NULL },
	{ "pastebin", NDPI_ONLY, 0, NULL },
#endif
	{ "pcanywhere", L7_ONLY, 0, NULL },
	{ "pdf", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
//      { "periodic flow", NDPI_RISK, 48, NULL }, /* unused */
#endif
	{ "perl", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "pgm", NDPI_ONLY, 0, NULL },
	{ "pinterest", NDPI_ONLY, 0, NULL },
	{ "playstation", NDPI_ONLY, 0, NULL },
	{ "playstore", NDPI_ONLY, 0, NULL },
	{ "pluralsight", NDPI_ONLY, 0, NULL },
#endif
	{ "png", L7_ONLY, 0, NULL },
	{ "poco", L7_ONLY, 0, NULL },
	{ "pop3", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "pops", NDPI_ONLY, 0, NULL },
	{ "possible exploit", NDPI_RISK, 40, "tls,fastcgi,http,quic" },
	{ "postgresql", NDPI_ONLY, 0, NULL },
#endif
	{ "postscript", L7_ONLY, 0, NULL },
	{ "pplive", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "ppstream", NDPI_ONLY, 0, NULL },
	{ "pptp", NDPI_ONLY, 0, NULL },
	{ "ptpv2", NDPI_ONLY, 0, NULL },
#endif
	{ "pre_icq_login", L7_ONLY, 0, NULL },
	{ "pre_msn_login", L7_ONLY, 0, NULL },
	{ "pre_urlblock", L7_ONLY, 0, NULL },
	{ "pre_yahoo_login", L7_ONLY, 0, NULL },
	{ "pressplay", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "profinet_io", NDPI_ONLY, 0, NULL },
	{ "protobuf", NDPI_ONLY, 0, NULL },
	{ "protonvpn", NDPI_ONLY, 0, NULL },
	{ "psiphon", NDPI_ONLY, 0, NULL },
#endif
	{ "qianlong", L7_ONLY, 0, NULL },
	{ "qq", DPI, 0, NULL },
	{ "qq_login", L7_ONLY, 0, NULL },
	{ "qq_login_1", L7_ONLY, 0, NULL },
	{ "qq_tcp_file", L7_ONLY, 0, NULL },
	{ "qq_udp_file", L7_ONLY, 0, NULL },
	{ "qqdownload_1", L7_ONLY, 0, NULL },
	{ "qqdownload_2", L7_ONLY, 0, NULL },
	{ "qqdownload_3", L7_ONLY, 0, NULL },
	{ "qqfile", L7_ONLY, 0, NULL },
	{ "qqgame", L7_ONLY, 0, NULL },
	{ "qqlive", L7_ONLY, 0, NULL },
	{ "qqlive2", L7_ONLY, 0, NULL },
	{ "quake-halflife", L7_ONLY, 0, NULL },
	{ "quake1", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "quic", NDPI_ONLY, 0, NULL },
#endif
	{ "quicktime", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "radius", NDPI_ONLY, 0, NULL },
#endif
	{ "radmin", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "raknet", NDPI_ONLY, 0, NULL },
#endif
	{ "rar", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "rce injection", NDPI_RISK, 3, "http,http_connect,http_proxy" },
#endif
	{ "rdp", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "reddit", NDPI_ONLY, 0, NULL },
	{ "redis", NDPI_ONLY, 0, NULL },
#endif
	{ "replaytv-ivs", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "riotgames", NDPI_ONLY, 0, NULL },
//      { "risky asn", NDPI_RISK, 26, NULL },
	{ "risky domain name", NDPI_RISK, 27, "dns" },
#endif
	{ "rlogin", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "rmcp", NDPI_ONLY, 0, NULL },
	{ "roblox", NDPI_ONLY, 0, NULL },
	{ "rpc", NDPI_ONLY, 0, NULL },
#endif
	{ "rpm", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "rsh", NDPI_ONLY, 0, NULL },
	{ "rsync", NDPI_ONLY, 0, NULL },
	{ "rtcp", NDPI_ONLY, 0, NULL },
#endif
	{ "rtf", L7_ONLY, 0, NULL },
	{ "rtmp", DPI, 0, NULL },
	{ "rtp", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "rtps", NDPI_ONLY, 0, NULL },
#endif
	{ "rtsp", DPI, 0, NULL },
	{ "runesofmagic", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "rx", NDPI_ONLY, 0, NULL },
	{ "s7comm", NDPI_ONLY, 0, NULL },
	{ "s7comm_plus", NDPI_ONLY, 0, NULL },
	{ "salesforce", NDPI_ONLY, 0, NULL },
	{ "sap", NDPI_ONLY, 0, NULL },
	{ "sctp", NDPI_ONLY, 0, NULL },
	{ "sd-rtn", NDPI_ONLY, 0, NULL },
	{ "service-location", NDPI_ONLY, 0, NULL },
	{ "sflow", NDPI_ONLY, 0, NULL },
#endif
#ifdef HAVE_OPENDPI
	{ "showtime", NDPI_ONLY, 0, NULL },
	{ "signal", NDPI_ONLY, 0, NULL },
	{ "signalvoip", NDPI_ONLY, 0, NULL },
	{ "sina", NDPI_ONLY, 0, NULL },
	{ "sina_weibo", NDPI_ONLY, 0, NULL },
#endif
	{ "sip", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "siriusxmradio", NDPI_ONLY, 0, NULL },
	{ "skype_teams", NDPI_ONLY, 0, NULL },
	{ "skype_teamscall", NDPI_ONLY, 0, NULL },
#endif
	{ "skypeout", L7_ONLY, 0, NULL },
	{ "skypetoskype", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "slack", NDPI_ONLY, 0, NULL },
#endif
	{ "smb", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "smb insecure vers", NDPI_RISK, 20, "smb" },
	{ "smbv1", NDPI_ONLY, 0, NULL },
	{ "smbv23", NDPI_ONLY, 0, NULL },
	{ "smpp", NDPI_ONLY, 0, NULL },
#endif
	{ "smtp", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "smtps", NDPI_ONLY, 0, NULL },
	{ "snapchat", NDPI_ONLY, 0, NULL },
	{ "snapchatcall", NDPI_ONLY, 0, NULL },
#endif
	{ "snmp", DPI, 0, NULL },
	{ "snmp-mon", L7_ONLY, 0, NULL },
	{ "snmp-trap", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "soap", NDPI_ONLY, 0, NULL },
#endif
	{ "socks", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "softether", NDPI_ONLY, 0, NULL },
	{ "someip", NDPI_ONLY, 0, NULL },
#endif
	{ "soribada", L7_ONLY, 0, NULL },
	{ "soulseek", PDPI_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "soundcloud", NDPI_ONLY, 0, NULL },
	{ "source_engine", NDPI_ONLY, 0, NULL },
	{ "spotify", NDPI_ONLY, 0, NULL },
	{ "sql injection", NDPI_RISK, 2, "http,http_connect,http_proxy" },
	{ "srtp", NDPI_ONLY, 0},
#endif
	{ "ssdp", DPI, 0, NULL },
	{ "ssh", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "ssh obsolete cli ver/cipher", NDPI_RISK, 18, "ssh" },
	{ "ssh obsolete ser ver/cipher", NDPI_RISK, 19, "ssh" },
#endif
	{ "ssl", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "starcraft", NDPI_ONLY, 0, NULL },
	{ "steam", NDPI_ONLY, 0, NULL },
#endif
	{ "stun", DPI, 0, NULL },
	{ "subspace", L7_ONLY, 0, NULL },
	{ "subversion", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "susp entropy", NDPI_RISK, 35, "icmp" },
	{ "syncthing", NDPI_ONLY, 0, NULL },
	{ "syslog", NDPI_ONLY, 0, NULL },
	{ "tailscale", NDPI_ONLY, 0, NULL },
#endif
	{ "tar", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "targusdataspeed", NDPI_ONLY, 0, NULL },
	{ "tcp connection issues", NDPI_RISK, 50, "all" },
#endif
	{ "teamfortress2", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "teams", NDPI_ONLY, 0, NULL },
#endif
	{ "teamspeak", DPI, 0, NULL },
	{ "teamviewer", DPI, 0, NULL },
	{ "teamviewer1", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "tencentvideo", NDPI_ONLY, 0, NULL },
	{ "telegram", NDPI_ONLY, 0, NULL },
	{ "telegram_voip", NDPI_ONLY, 0, NULL },
#endif
	{ "telnet", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "tencent", NDPI_ONLY, 0, NULL },
	{ "teredo", NDPI_ONLY, 0, NULL },
#endif
	{ "tesla", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "tesla_services", NDPI_ONLY, 0, NULL },
	{ "text with non-printable chars", NDPI_RISK, 39, "tls,fastcgi,quic,http,dns" },
#endif
	{ "tftp", DPI, 0, NULL },
	{ "thecircle", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "threema", NDPI_ONLY, 0, NULL },
#endif
	{ "thunder5_see", L7_ONLY, 0, NULL },
	{ "thunder5_tcp", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "tidal", NDPI_ONLY, 0, NULL },
	{ "tiktok", NDPI_ONLY, 0, NULL },
	{ "tinc", NDPI_ONLY, 0, NULL },
	{ "tivoconnect", NDPI_ONLY, 0, NULL },
	{ "tls", NDPI_ONLY, 0, NULL },
	{ "tls cert about to expire", NDPI_RISK, 41, "tls" },
	{ "tls cert expired", NDPI_RISK, 9, "tls" },
	{ "tls cert mismatch", NDPI_RISK, 10, "tls" },
	{ "tls cert validity too long", NDPI_RISK, 32, "tls" },
	{ "tls fatal alert", NDPI_RISK, 34, "tls" },
	{ "tls not carrying https", NDPI_RISK, 15, "tls" },
	{ "tls obsolete (v1.1 or older)", NDPI_RISK, 7, "tls" },
	{ "tls self signed cert", NDPI_RISK, 6, "tls" },
	{ "tls susp esni usage", NDPI_RISK, 20, "tls" },
	{ "tls susp extn", NDPI_RISK, 33, "tls" },
	{ "tls weak cipher", NDPI_RISK, 8, "tls" },

	{ "tocaboca", NDPI_ONLY, 0, NULL },
#endif
	{ "tonghuashun", L7_ONLY, 0, NULL },
	{ "tor", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "tplink_shp", NDPI_ONLY, 0, NULL },
	{ "truphone", NDPI_ONLY, 0, NULL },
#endif
	{ "tsp", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "tuenti", NDPI_ONLY, 0, NULL },
	{ "tumblr", NDPI_ONLY, 0, NULL },
	{ "tunein", NDPI_ONLY, 0, NULL },
	{ "tunnelbear", NDPI_ONLY, 0, NULL },
	{ "tuya_lp", NDPI_ONLY, 0, NULL },
	{ "tvuplayer", NDPI_ONLY, 0, NULL },
	{ "twitch", NDPI_ONLY, 0, NULL },
	{ "twitter", NDPI_ONLY, 0, NULL },
#endif
	{ "ubnt-telemetry", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "ubntac2", NDPI_ONLY, 0, NULL },
	{ "ubuntuone", NDPI_ONLY, 0, NULL },
	{ "ultrasurf", NDPI_ONLY, 0, NULL },
	{ "umas", NDPI_ONLY, 0, NULL},
	{ "uncommon tls alpn", NDPI_RISK, 31, "tls" },
	{ "unidirectional traffic", NDPI_RISK, 46, "all" },
	{ "unsafe protocol", NDPI_RISK, 22, "all" },
#endif
	{ "unset", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "usenet", NDPI_ONLY, 0, NULL },
#endif
	{ "uucp", L7_ONLY, 0, NULL },
	{ "validcertssl", L7_ONLY, 0, NULL },
	{ "ventrilo", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "vevo", NDPI_ONLY, 0, NULL },
	{ "vhua", NDPI_ONLY, 0, NULL },
	{ "viber", NDPI_ONLY, 0, NULL },
	{ "vimeo", NDPI_ONLY, 0, NULL },
	{ "vk", NDPI_ONLY, 0, NULL },
	{ "vmware", NDPI_ONLY, 0, NULL },
#endif
	{ "vnc", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "vrrp", NDPI_ONLY, 0, NULL },
	{ "vudu", NDPI_ONLY, 0, NULL },
	{ "vxlan", NDPI_ONLY, 0, NULL },
	{ "warcraft3", NDPI_ONLY, 0, NULL },
#endif
	{ "waste", PDPI_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "waze", NDPI_ONLY, 0, NULL },
	{ "webex", NDPI_ONLY, 0, NULL },
#endif
	{ "webmail_163", L7_ONLY, 0, NULL },
	{ "webmail_gmail", L7_ONLY, 0, NULL },
	{ "webmail_hinet", L7_ONLY, 0, NULL },
	{ "webmail_hotmail", L7_ONLY, 0, NULL },
	{ "webmail_pchome", L7_ONLY, 0, NULL },
	{ "webmail_qq", L7_ONLY, 0, NULL },
	{ "webmail_seednet", L7_ONLY, 0, NULL },
	{ "webmail_sina", L7_ONLY, 0, NULL },
	{ "webmail_sohu", L7_ONLY, 0, NULL },
	{ "webmail_tom", L7_ONLY, 0, NULL },
	{ "webmail_url", L7_ONLY, 0, NULL },
	{ "webmail_yahoo", L7_ONLY, 0, NULL },
	{ "webmail_yam", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "websocket", NDPI_ONLY, 0, NULL },
	{ "wechat", NDPI_ONLY, 0, NULL },
	{ "whatsapp", NDPI_ONLY, 0, NULL },
	{ "whatsappcall", NDPI_ONLY, 0, NULL },
	{ "whatsappfiles", NDPI_ONLY, 0, NULL },
#endif
	{ "whois", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "whois-das", NDPI_ONLY, 0, NULL },
	{ "wikipedia", NDPI_ONLY, 0, NULL },
#endif
	{ "windows-telemetry", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "windowsupdate", NDPI_ONLY, 0, NULL },
#endif
	{ "winmx", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "wireguard", NDPI_ONLY, 0, NULL },
	{ "worldofkungfu", NDPI_ONLY, 0, NULL },
#endif
	{ "worldofwarcraft", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "wsd", NDPI_ONLY, 0, NULL },
#endif
	{ "x11", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "xbox", NDPI_ONLY, 0, NULL },
#endif
	{ "xboxlive", L7_ONLY, 0, NULL },
	{ "xdcc", PDPI_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "xdmcp", NDPI_ONLY, 0, NULL },
	{ "xiaomi", NDPI_ONLY, 0, NULL },
	{ "xss attack", NDPI_RISK, 1, "http,http_connect,http_proxy" },
#endif
	{ "xunlei", L7_ONLY, 0, NULL },
	{ "yahoo", DPI, 0, NULL },
	{ "yahoo_camera", L7_ONLY, 0, NULL },
	{ "yahoo_file", L7_ONLY, 0, NULL },
	{ "yahoo_login", L7_ONLY, 0, NULL },
	{ "yahoo_voice", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "yandex", NDPI_ONLY, 0, NULL },
	{ "yandex_cloud", NDPI_ONLY, 0, NULL },
	{ "yandex_direct", NDPI_ONLY, 0, NULL },
	{ "yandex_disk", NDPI_ONLY, 0, NULL },
	{ "yandex_mail", NDPI_ONLY, 0, NULL },
	{ "yandex_market", NDPI_ONLY, 0, NULL },
	{ "yandex_metrika", NDPI_ONLY, 0, NULL },
	{ "yandex_music", NDPI_ONLY, 0, NULL },
#endif
	{ "youtube", DPI, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "youtubeupload", NDPI_ONLY, 0, NULL },
	{ "z3950", NDPI_ONLY, 0, NULL },
	{ "zabbix", NDPI_ONLY, 0, NULL },
	{ "zattoo", NDPI_ONLY, 0, NULL },
	{ "zeromq", NDPI_ONLY, 0, NULL },
#endif
	{ "zip", L7_ONLY, 0, NULL },
	{ "zmaap", L7_ONLY, 0, NULL },
#ifdef HAVE_OPENDPI
	{ "zoom", NDPI_ONLY, 0, NULL },
#endif
	{ 0, 0, 0, NULL },
};
