/*
 * L7-filter Supported Protocols 
 */
    /*
     * 2009-05-28 
     */

typedef struct _l7filters {

	char *name;
	int protocol;		// 1=p2p, 0=l7, 2=opendpi
	unsigned char level;
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
	{ "100bao", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "1kxun", NDPI_ONLY, 0 },
	{ "accuweather", NDPI_ONLY, 0 },
	{ "activision", NDPI_ONLY, 0 },
	{ "ads_analytics_track", NDPI_ONLY, 0 },
	{ "adult_content", NDPI_ONLY, 0 },
	{ "afp", NDPI_ONLY, 0 },
#endif
	{ "aim", L7_ONLY, 0 },
	{ "aimwebcontent", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "ajp", NDPI_ONLY, 0 },
	{ "alibaba", NDPI_ONLY, 0 },
	{ "alicloud", NDPI_ONLY, 0 },
	{ "amazon", NDPI_ONLY, 0 },
	{ "amazonalexa", NDPI_ONLY, 0 },
	{ "amazonaws", NDPI_ONLY, 0 },
	{ "amazonvideo", NDPI_ONLY, 0 },
	{ "amongus", NDPI_ONLY, 0 },
	{ "amqp", NDPI_ONLY, 0 },
	{ "anonymous subscriber", NDPI_RISK, 45 },
	{ "anydesk", NDPI_ONLY, 0 },
	{ "apple", NDPI_ONLY, 0 },
	{ "appleicloud", NDPI_ONLY, 0 },
	{ "appleitunes", NDPI_ONLY, 0 },
#endif
#ifdef HAVE_OPENDPI
	{ "applepush", NDPI_ONLY, 0 },
	{ "applesiri", NDPI_ONLY, 0 },
	{ "applestore", NDPI_ONLY, 0 },
	{ "appletvplus", NDPI_ONLY, 0 },
#endif
	{ "ares", PDPI_ONLY, 0 },
	{ "armagetron", DPI, 0 },
	{ "audiogalaxy", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "avast", NDPI_ONLY, 0 },
	{ "avastsecuredns", NDPI_ONLY, 0 },
	{ "azure", NDPI_ONLY, 0 },
	{ "bacnet", NDPI_ONLY, 0 },
	{ "badoo", NDPI_ONLY, 0 },
#endif
	{ "battlefield1942", L7_ONLY, 0 },
	{ "battlefield2", L7_ONLY, 0 },
	{ "battlefield2142", L7_ONLY, 0 },
	{ "bearshare", PDPI_ONLY, 0 },
	{ "bgp", DPI, 0 },
	{ "biff", L7_ONLY, 0 },
	{ "bittorrent", PDPI, 0 },
#ifdef HAVE_OPENDPI
	{ "bjnp", NDPI_ONLY, 0 },
	{ "bloomberg", NDPI_ONLY, 0 },
	{ "cachefly", NDPI_ONLY, 0 },
	{ "capwap", NDPI_ONLY, 0 },
	{ "cassandra", NDPI_ONLY, 0 },
	{ "checkmk", NDPI_ONLY, 0 },
#endif
	{ "chikka", L7_ONLY, 0 },
	{ "cimd", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "ciscoskinny", NDPI_ONLY, 0 },
#endif
	{ "ciscovpn", DPI, 0 },
	{ "citrix", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "clear-text credentials", NDPI_RISK, 36 },
	{ "cloudflare", NDPI_ONLY, 0 },
	{ "cloudflarewarp", NDPI_ONLY, 0 },
#endif
	{ "clubbox", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "cnn", NDPI_ONLY, 0 },
	{ "coap", NDPI_ONLY, 0 },
#endif
	{ "code_red", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "collectd", NDPI_ONLY, 0 },
	{ "corba", NDPI_ONLY, 0 },
#endif
	{ "counterstrike-source", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "cpha", NDPI_ONLY, 0 },
	{ "crashlytics", NDPI_ONLY, 0 },
	{ "crawler/bot", NDPI_RISK, 44 },
	{ "crossfire", NDPI_ONLY, 0 },
	{ "crynet", NDPI_ONLY, 0 },
	{ "csgo", NDPI_ONLY, 0 },
#endif
	{ "cvs", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "cybersec", NDPI_ONLY, 0 },
	{ "datasaver", NDPI_ONLY, 0 },
	{ "dailymotion", NDPI_ONLY, 0 },
#endif
	{ "dayofdefeat-source", L7_ONLY, 0 },
	{ "dazhihui", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "dazn", NDPI_ONLY, 0 },
	{ "deezer", NDPI_ONLY, 0 },
	{ "desktop/file sharing", NDPI_RISK, 30 },
#endif
	{ "dhcp", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "dhcpv6", NDPI_ONLY, 0 },
	{ "diameter", NDPI_ONLY, 0 },
	{ "directv", NDPI_ONLY, 0 },
#endif
#ifdef HAVE_OPENDPI
	{ "discord", NDPI_ONLY, 0 },
	{ "disneyplus", NDPI_ONLY, 0 },
	{ "dnp3", NDPI_ONLY, 0 },
#endif
	{ "dns", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "dnscrypt", NDPI_ONLY, 0 },
	{ "dofus", NDPI_ONLY, 0 },
	{ "doh_dot", NDPI_ONLY, 0 },
#endif
	{ "doom3", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "drda", NDPI_ONLY, 0 },
	{ "dropbox", NDPI_ONLY, 0 },
	{ "dtls", NDPI_ONLY, 0 },
	{ "eaq", NDPI_ONLY, 0 },
	{ "ebay", NDPI_ONLY, 0 },
	{ "edgecast", NDPI_ONLY, 0 },
#endif
	{ "edonkey", PDPI, 0 },
#ifdef HAVE_OPENDPI
	{ "egp", NDPI_ONLY, 0 },
	{ "elasticsearch", NDPI_ONLY, 0 },
	{ "ethernetip", NDPI_ONLY, 0 },
	{ "error code", NDPI_RISK, 43 },
#endif
	{ "exe", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "facebook", NDPI_ONLY, 0 },
	{ "facebookvoip", NDPI_ONLY, 0 },
#endif
#ifdef HAVE_OPENDPI
	{ "fastcgi", NDPI_ONLY, 0 },
#endif
	{ "filetopia", DPI, 0 },
	{ "finger", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "fix", NDPI_ONLY, 0 },
#endif
	{ "flash", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "forticlient", NDPI_ONLY, 0 },
	{ "fragmented dns message", NDPI_RISK, 38 },
#endif
	{ "freegate_dns", L7_ONLY, 0 },
	{ "freegate_http", L7_ONLY, 0 },
	{ "freenet", L7_ONLY, 0 },
	{ "ftp", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "ftp_control", NDPI_ONLY, 0 },
	{ "ftps", NDPI_ONLY, 0 },
	{ "ftp_data", NDPI_ONLY, 0 },
	{ "fuze", NDPI_ONLY, 0 },
	{ "genshinimpact", NDPI_ONLY, 0 },
#endif
	{ "gif", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "git", NDPI_ONLY, 0 },
	{ "github", NDPI_ONLY, 0 },
	{ "gitlab", NDPI_ONLY, 0 },
#endif
	{ "gkrellm", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "gmail", NDPI_ONLY, 0 },
#endif
	{ "gnucleuslan", L7_ONLY, 0 },
	{ "gnutella", PDPI, 0 },
	{ "goboogy", L7_ONLY, 0 },
	{ "gogobox", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "google", NDPI_ONLY, 0 },
	{ "googleclassroom", NDPI_ONLY, 0 },
	{ "googlecloud", NDPI_ONLY, 0 },
	{ "googledocs", NDPI_ONLY, 0 },
	{ "googledrive", NDPI_ONLY, 0 },
	{ "googlehangoutduo", NDPI_ONLY, 0 },
	{ "googlemaps", NDPI_ONLY, 0 },
	{ "googleplus", NDPI_ONLY, 0 },
	{ "googleservices", NDPI_ONLY, 0 },
#endif
	{ "gopher", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "goto", NDPI_ONLY, 0 },
	{ "gre", NDPI_ONLY, 0 },
#endif
	{ "gtalk", L7_ONLY, 0 },
	{ "gtalk1", L7_ONLY, 0 },
	{ "gtalk2", L7_ONLY, 0 },
	{ "gtalk_file", L7_ONLY, 0 },
	{ "gtalk_file_1", L7_ONLY, 0 },
	{ "gtalk_vista", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "gtp", NDPI_ONLY, 0 },
	{ "gtp_c", NDPI_ONLY, 0 },
	{ "gtp_prime", NDPI_ONLY, 0 },
	{ "gtp_u", NDPI_ONLY, 0 },
#endif
	{ "guildwars", DPI, 0 },
	{ "h323", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "halflife2", NDPI_ONLY, 0 },
#endif
	{ "halflife2-deathmatch", L7_ONLY, 0 },
	{ "hamachi1", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "hbo", NDPI_ONLY, 0 },
#endif
	{ "hddtemp", L7_ONLY, 0 },
	{ "hotline", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "heroes_of_the_storm", NDPI_ONLY, 0 },
#endif
#ifdef HAVE_OPENDPI
	{ "hotspotshield", NDPI_ONLY, 0 },
	{ "hp_virtgrp", NDPI_ONLY, 0 },
	{ "hsrp", NDPI_ONLY, 0 },
#else
	{ "hotspot-shield", L7_ONLY, 0 },
#endif
	{ "html", L7_ONLY, 0 },
	{ "http", DPI, 0 },
	{ "http-dap", L7_ONLY, 0 },
	{ "http-freshdownload", L7_ONLY, 0 },
	{ "http-itunes", L7_ONLY, 0 },
	{ "http-rtsp", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "http_connect", NDPI_ONLY, 0 },
	{ "http_proxy", NDPI_ONLY, 0 },
#endif
	{ "httpaudio", L7_ONLY, 0 },
	{ "httpcachehit", L7_ONLY, 0 },
	{ "httpcachemiss", L7_ONLY, 0 },
	{ "httpvideo", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "http obsolete server", NDPI_RISK, 47 },
	{ "http susp content", NDPI_RISK, 25 },
	{ "hulu", NDPI_ONLY, 0 },
	{ "i3d", NDPI_ONLY, 0 },
	{ "iax", NDPI_ONLY, 0 },
	{ "icecast", NDPI_ONLY, 0 },
	{ "icloudprivaterelay", NDPI_ONLY, 0 },
	{ "icmp", NDPI_ONLY, 0 },
	{ "icmpv6", NDPI_ONLY, 0 },
#endif
	{ "icq_file", L7_ONLY, 0 },
	{ "icq_file_1", L7_ONLY, 0 },
	{ "icq_file_2", L7_ONLY, 0 },
	{ "icq_login", L7_ONLY, 0 },
	{ "ident", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "idn domain name", NDPI_RISK, 42 },
	{ "iec60870", NDPI_ONLY, 0 },
	{ "iflix", NDPI_ONLY, 0 },
	{ "igmp", NDPI_ONLY, 0 },
	{ "iheartradio", NDPI_ONLY, 0 },
#endif
	{ "imap", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "imaps", NDPI_ONLY, 0 },
#endif
	{ "imesh", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "imo", NDPI_ONLY, 0 },
	{ "instagram", NDPI_ONLY, 0 },
	{ "ip_in_ip", NDPI_ONLY, 0 },
	{ "ip_pim", NDPI_ONLY, 0 },
#endif
	{ "ipp", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "ipsec", NDPI_ONLY, 0 },
#endif
	{ "irc", DPI, 0 },
	{ "jabber", DPI, 0 },
	{ "jpeg", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "kakaotalk", NDPI_ONLY, 0 },
	{ "kakaotalk_voice", NDPI_ONLY, 0 },
#endif
	{ "kazaa", PDPI_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "kerberos", NDPI_ONLY, 0 },
	{ "kismet", NDPI_ONLY, 0 },
	{ "kontiki", NDPI_ONLY, 0 },
#endif
	{ "kugoo", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "large dns packet", NDPI_RISK, 37 },
	{ "lastfm", NDPI_ONLY, 0 },
	{ "ldap", NDPI_ONLY, 0 },
	{ "likee", NDPI_ONLY, 0 },
	{ "line", NDPI_ONLY, 0 },
	{ "line_call", NDPI_ONLY, 0 },
	{ "linkedin", NDPI_ONLY, 0 },
	{ "lisp", NDPI_ONLY, 0 },
#endif
	{ "live365", L7_ONLY, 0 },
	{ "liveforspeed", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "livestream", NDPI_ONLY, 0 },
	{ "llmnr", NDPI_ONLY, 0 },
	{ "lotusnotes", NDPI_ONLY, 0 },
#endif
	{ "lpd", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "malicious ja3 fingerp.", NDPI_RISK, 28 },
	{ "malicious ssl cert/sha1 fingerp.", NDPI_RISK, 29 },
	{ "maplestory", NDPI_ONLY, 0 },
	{ "mdns", NDPI_ONLY, 0 },
	{ "megaco", NDPI_ONLY, 0 },
	{ "memcached", NDPI_ONLY, 0 },
	{ "meraki_cloud", NDPI_ONLY, 0 },
	{ "messenger", NDPI_ONLY, 0 },
	{ "mgcp", NDPI_ONLY, 0 },
	{ "microsoft", NDPI_ONLY, 0 },
	{ "microsoft365", NDPI_ONLY, 0 },
	{ "mining", NDPI_ONLY, 0 },
	{ "minor issues", NDPI_RISK, 49 },
	{ "missing sni tls extn", NDPI_RISK, 24 },
	{ "modbus", NDPI_ONLY, 0 },
#endif
	{ "mohaa", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "mongodb", NDPI_ONLY, 0 },
	{ "munin", NDPI_ONLY, 0 },
#endif
	{ "mp3", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "mpeg_ts", NDPI_ONLY, 0 },
	{ "mpegdash", NDPI_ONLY, 0 },
	{ "mqtt", NDPI_ONLY, 0 },
	{ "ms_onedrive", NDPI_ONLY, 0 },
#endif
	{ "msn-filetransfer", L7_ONLY, 0 },
	{ "msnmessenger", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "mssql-tds", NDPI_ONLY, 0 },
#endif
	{ "mute", PDPI_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "mysql", NDPI_ONLY, 0 },
#endif
	{ "napster", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "natpmp", NDPI_ONLY, 0 },
	{ "nats", NDPI_ONLY, 0 },
#endif
	{ "nbns", L7_ONLY, 0 },
	{ "ncp", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "nestlogsink", NDPI_ONLY, 0 },
#endif
	{ "netbios", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "netflix", NDPI_ONLY, 0 },
	{ "netflow", NDPI_ONLY, 0 },
	{ "nfs", NDPI_ONLY, 0 },
#endif
	{ "nimda", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "nintendo", NDPI_ONLY, 0 },
#endif
	{ "nntp", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "noe", NDPI_ONLY, 0 },
	{ "ntop", NDPI_ONLY, 0 },
#endif
	{ "ntp", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "ocs", NDPI_ONLY, 0 },
	{ "ocsp", NDPI_ONLY, 0 },
#endif
	{ "ogg", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "oicq", NDPI_ONLY, 0 },
	{ "ookla", NDPI_ONLY, 0 },
	{ "opendns", NDPI_ONLY, 0 },
#endif
#ifdef HAVE_OPENDPI
	{ "openvpn", NDPI_ONLY, 0 },
	{ "oracle", NDPI_ONLY, 0 },
	{ "ospf", NDPI_ONLY, 0 },
	{ "outlook", NDPI_ONLY, 0 },
	{ "pandora", NDPI_ONLY, 0 },
	{ "pastebin", NDPI_ONLY, 0 },
#endif
	{ "pcanywhere", DPI, 0 },
	{ "pdf", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "periodic flow", NDPI_RISK, 48 },
#endif
	{ "perl", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "pgm", NDPI_ONLY, 0 },
	{ "pinterest", NDPI_ONLY, 0 },
	{ "playstation", NDPI_ONLY, 0 },
	{ "playstore", NDPI_ONLY, 0 },
	{ "pluralsight", NDPI_ONLY, 0 },
#endif
	{ "png", L7_ONLY, 0 },
	{ "poco", L7_ONLY, 0 },
	{ "pop3", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "pops", NDPI_ONLY, 0 },
	{ "possible exploit", NDPI_RISK, 40 },
	{ "postgresql", NDPI_ONLY, 0 },
#endif
	{ "postscript", L7_ONLY, 0 },
	{ "pplive", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "ppstream", NDPI_ONLY, 0 },
	{ "pptp", NDPI_ONLY, 0 },
#endif
	{ "pre_icq_login", L7_ONLY, 0 },
	{ "pre_msn_login", L7_ONLY, 0 },
	{ "pre_urlblock", L7_ONLY, 0 },
	{ "pre_yahoo_login", L7_ONLY, 0 },
	{ "pressplay", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "psiphon", NDPI_ONLY, 0 },
#endif
	{ "qianlong", L7_ONLY, 0 },
	{ "qq", DPI, 0 },
	{ "qq_login", L7_ONLY, 0 },
	{ "qq_login_1", L7_ONLY, 0 },
	{ "qq_tcp_file", L7_ONLY, 0 },
	{ "qq_udp_file", L7_ONLY, 0 },
	{ "qqdownload_1", L7_ONLY, 0 },
	{ "qqdownload_2", L7_ONLY, 0 },
	{ "qqdownload_3", L7_ONLY, 0 },
	{ "qqfile", L7_ONLY, 0 },
	{ "qqgame", L7_ONLY, 0 },
	{ "qqlive", DPI, 0 },
	{ "qqlive2", L7_ONLY, 0 },
	{ "quake-halflife", L7_ONLY, 0 },
	{ "quake1", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "quic", NDPI_ONLY, 0 },
#endif
	{ "quicktime", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "radius", NDPI_ONLY, 0 },
#endif
	{ "radmin", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "raknet", NDPI_ONLY, 0 },
#endif
	{ "rar", L7_ONLY, 0 },
	{ "rdp", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "reddit", NDPI_ONLY, 0 },
	{ "redis", NDPI_ONLY, 0 },
#endif
	{ "replaytv-ivs", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "riotgames", NDPI_ONLY, 0 },
	{ "risky asn", NDPI_RISK, 26 },
	{ "risky domain name", NDPI_RISK, 27 },
#endif
	{ "rlogin", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "rpc", NDPI_ONLY, 0 },
#endif
	{ "rpm", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "rsh", NDPI_ONLY, 0 },
	{ "rsync", NDPI_ONLY, 0 },
	{ "rtcp", NDPI_ONLY, 0 },
#endif
	{ "rtf", L7_ONLY, 0 },
	{ "rtmp", DPI, 0 },
	{ "rtp", DPI, 0 },
	{ "rtsp", DPI, 0 },
	{ "runesofmagic", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "rx", NDPI_ONLY, 0 },
	{ "s7comm", NDPI_ONLY, 0 },
	{ "salesforce", NDPI_ONLY, 0 },
	{ "sap", NDPI_ONLY, 0 },
	{ "sctp", NDPI_ONLY, 0 },
	{ "sd-rtn", NDPI_ONLY, 0 },
	{ "sflow", NDPI_ONLY, 0 },
#endif
#ifdef HAVE_OPENDPI
	{ "showtime", NDPI_ONLY, 0 },
	{ "signal", NDPI_ONLY, 0 },
	{ "signalvoip", NDPI_ONLY, 0 },
	{ "sina(weibo)", NDPI_ONLY, 0 },
#endif
	{ "sip", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "siriusxmradio", NDPI_ONLY, 0 },
	{ "skype_teams", NDPI_ONLY, 0 },
	{ "skype_teamscall", NDPI_ONLY, 0 },
#endif
	{ "skypeout", L7_ONLY, 0 },
	{ "skypetoskype", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "slack", NDPI_ONLY, 0 },
#endif
	{ "smb", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "smbv1", NDPI_ONLY, 0 },
	{ "smbv23", NDPI_ONLY, 0 },
	{ "smpp", NDPI_ONLY, 0 },
#endif
	{ "smtp", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "smtps", NDPI_ONLY, 0 },
	{ "snapchat", NDPI_ONLY, 0 },
	{ "snapchatcall", NDPI_ONLY, 0 },
#endif
	{ "snmp", DPI, 0 },
	{ "snmp-mon", L7_ONLY, 0 },
	{ "snmp-trap", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "soap", NDPI_ONLY, 0 },
#endif
	{ "socks", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "softether", NDPI_ONLY, 0 },
	{ "someip", NDPI_ONLY, 0 },
#endif
	{ "soribada", L7_ONLY, 0 },
	{ "soulseek", PDPI, 0 },
#ifdef HAVE_OPENDPI
	{ "soundcloud", NDPI_ONLY, 0 },
	{ "source_engine", NDPI_ONLY, 0 },
	{ "spotify", NDPI_ONLY, 0 },
#endif
	{ "ssdp", DPI, 0 },
	{ "ssh", DPI, 0 },
	{ "ssl", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "starcraft", NDPI_ONLY, 0 },
	{ "steam", NDPI_ONLY, 0 },
#endif
	{ "stun", DPI, 0 },
	{ "subspace", L7_ONLY, 0 },
	{ "subversion", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "susp dns traffic", NDPI_RISK, 23 },
	{ "susp entropy", NDPI_RISK, 35 },
	{ "syncthing", NDPI_ONLY, 0 },
	{ "syslog", NDPI_ONLY, 0 },
	{ "tailscale", NDPI_ONLY, 0 },
#endif
	{ "tar", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "targusdataspeed", NDPI_ONLY, 0 },
	{ "tcp connection issues", NDPI_RISK, 50 },
#endif
	{ "teamfortress2", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "teams", NDPI_ONLY, 0 },
#endif
	{ "teamspeak", DPI, 0 },
	{ "teamviewer", DPI, 0 },
	{ "teamviewer1", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "tencentvideo", NDPI_ONLY, 0 },
	{ "telegram", NDPI_ONLY, 0 },
#endif
	{ "telnet", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "tencent", NDPI_ONLY, 0 },
	{ "teredo", NDPI_ONLY, 0 },
#endif
	{ "tesla", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "text with non-printable chars", NDPI_RISK, 39 },
#endif
	{ "tftp", DPI, 0 },
	{ "thecircle", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "threema", NDPI_ONLY, 0 },
#endif
	{ "thunder5_see", L7_ONLY, 0 },
	{ "thunder5_tcp", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "tidal", NDPI_ONLY, 0 },
	{ "tiktok", NDPI_ONLY, 0 },
	{ "tinc", NDPI_ONLY, 0 },
	{ "tivoconnect", NDPI_ONLY, 0 },
	{ "tls", NDPI_ONLY, 0 },
	{ "tls cert about to expire", NDPI_RISK, 41 },
	{ "tls cert validity too long", NDPI_RISK, 32 },
	{ "tls fatal alert", NDPI_RISK, 34 },
	{ "tls susp extn", NDPI_RISK, 33 },
	{ "tocaboca", NDPI_ONLY, 0 },
#endif
	{ "tonghuashun", L7_ONLY, 0 },
	{ "tor", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "tplink_shp", NDPI_ONLY, 0 },
	{ "truphone", NDPI_ONLY, 0 },
#endif
	{ "tsp", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "tuenti", NDPI_ONLY, 0 },
	{ "tumblr", NDPI_ONLY, 0 },
	{ "tunein", NDPI_ONLY, 0 },
	{ "tunnelbear", NDPI_ONLY, 0 },
	{ "tuya_lp", NDPI_ONLY, 0 },
	{ "tvuplayer", NDPI_ONLY, 0 },
	{ "twitch", NDPI_ONLY, 0 },
	{ "twitter", NDPI_ONLY, 0 },
	{ "ubntac2", NDPI_ONLY, 0 },
	{ "ubuntuone", NDPI_ONLY, 0 },
	{ "ultrasurf", NDPI_ONLY, 0 },
	{ "uncommon tls alpn", NDPI_RISK, 31 },
	{ "unidirectional traffic", NDPI_RISK, 46 },
	{ "unsafe protocol", NDPI_RISK, 22 },
#endif
	{ "unset", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "usenet", NDPI_ONLY, 0 },
#endif
	{ "uucp", L7_ONLY, 0 },
	{ "validcertssl", L7_ONLY, 0 },
	{ "ventrilo", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "vevo", NDPI_ONLY, 0 },
	{ "vhua", NDPI_ONLY, 0 },
	{ "viber", NDPI_ONLY, 0 },
	{ "vimeo", NDPI_ONLY, 0 },
	{ "vk", NDPI_ONLY, 0 },
	{ "vmware", NDPI_ONLY, 0 },
#endif
	{ "vnc", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "vrrp", NDPI_ONLY, 0 },
	{ "vudu", NDPI_ONLY, 0 },
	{ "vxlan", NDPI_ONLY, 0 },
	{ "warcraft3", NDPI_ONLY, 0 },
#endif
	{ "waste", PDPI_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "waze", NDPI_ONLY, 0 },
	{ "webex", NDPI_ONLY, 0 },
#endif
	{ "webmail_163", L7_ONLY, 0 },
	{ "webmail_gmail", L7_ONLY, 0 },
	{ "webmail_hinet", L7_ONLY, 0 },
	{ "webmail_hotmail", L7_ONLY, 0 },
	{ "webmail_pchome", L7_ONLY, 0 },
	{ "webmail_qq", L7_ONLY, 0 },
	{ "webmail_seednet", L7_ONLY, 0 },
	{ "webmail_sina", L7_ONLY, 0 },
	{ "webmail_sohu", L7_ONLY, 0 },
	{ "webmail_tom", L7_ONLY, 0 },
	{ "webmail_url", L7_ONLY, 0 },
	{ "webmail_yahoo", L7_ONLY, 0 },
	{ "webmail_yam", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "websocket", NDPI_ONLY, 0 },
	{ "wechat", NDPI_ONLY, 0 },
	{ "whatsapp", NDPI_ONLY, 0 },
	{ "whatsappcall", NDPI_ONLY, 0 },
	{ "whatsappfiles", NDPI_ONLY, 0 },
#endif
	{ "whois", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "whois-das", NDPI_ONLY, 0 },
	{ "wikipedia", NDPI_ONLY, 0 },
	{ "windowsupdate", NDPI_ONLY, 0 },
#endif
	{ "winmx", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "wireguard", NDPI_ONLY, 0 },
	{ "worldofkungfu", NDPI_ONLY, 0 },
#endif
	{ "worldofwarcraft", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "wsd", NDPI_ONLY, 0 },
#endif
	{ "x11", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "xbox", NDPI_ONLY, 0 },
#endif
	{ "xboxlive", L7_ONLY, 0 },
	{ "xdcc", PDPI_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "xdmcp", NDPI_ONLY, 0 },
	{ "xiaomi", NDPI_ONLY, 0 },
#endif
	{ "xunlei", L7_ONLY, 0 },
	{ "yahoo", DPI, 0 },
	{ "yahoo_camera", L7_ONLY, 0 },
	{ "yahoo_file", L7_ONLY, 0 },
	{ "yahoo_login", L7_ONLY, 0 },
	{ "yahoo_voice", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "yandex", NDPI_ONLY, 0 },
	{ "yandex_cloud", NDPI_ONLY, 0 },
	{ "yandex_direct", NDPI_ONLY, 0 },
	{ "yandex_disk", NDPI_ONLY, 0 },
	{ "yandex_mail", NDPI_ONLY, 0 },
	{ "yandex_market", NDPI_ONLY, 0 },
	{ "yandex_metrika", NDPI_ONLY, 0 },
	{ "yandex_music", NDPI_ONLY, 0 },
#endif
	{ "youtube", DPI, 0 },
#ifdef HAVE_OPENDPI
	{ "youtubeupload", NDPI_ONLY, 0 },
	{ "z3950", NDPI_ONLY, 0 },
	{ "zabbix", NDPI_ONLY, 0 },
	{ "zattoo", NDPI_ONLY, 0 },
	{ "zeromq", NDPI_ONLY, 0 },
#endif
	{ "zip", L7_ONLY, 0 },
	{ "zmaap", L7_ONLY, 0 },
#ifdef HAVE_OPENDPI
	{ "zoom", NDPI_ONLY, 0 },
#endif
	{ 0, 0, 0 },
};
