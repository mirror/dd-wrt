/*
 * L7-filter Supported Protocols 
 */
    /*
     * 2009-05-28 
     */

typedef struct _l7filters {

	char *name;
	int protocol;		// 1=p2p, 0=l7, 2=opendpi

} l7filters;
#define L7_ONLY 0
#define PDPI_ONLY 1
#define NDPI_ONLY 2
#define WINDDOWS_SPY 3

#ifdef HAVE_OPENDPI
#define DPI 2			//open dpi based
#define PDPI 2			//open dpi based
#else
#define DPI 0			//default l7
#define PDPI 1			//default p2p
#endif
//Added ,  (in extra), dazhihui, .

l7filters filters_list[] = {
	{ "100bao", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "1kxun", NDPI_ONLY },
	{ "accuweather", NDPI_ONLY },
	{ "activision", NDPI_ONLY },
	{ "ads_analytics_track", NDPI_ONLY },
	{ "adult_content", NDPI_ONLY },
	{ "afp", NDPI_ONLY },
#endif
	{ "aim", L7_ONLY },
	{ "aimwebcontent", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "ajp", NDPI_ONLY },
	{ "alibaba", NDPI_ONLY },
	{ "alicloud", NDPI_ONLY },
	{ "amazon", NDPI_ONLY },
	{ "amazonalexa", NDPI_ONLY },
	{ "amazonaws", NDPI_ONLY },
	{ "amazonvideo", NDPI_ONLY },
	{ "amongus", NDPI_ONLY },
	{ "amqp", NDPI_ONLY },
	{ "anydesk", NDPI_ONLY },
	{ "apple", NDPI_ONLY },
	{ "appleicloud", NDPI_ONLY },
	{ "appleitunes", NDPI_ONLY },
#endif
#ifdef HAVE_OPENDPI
	{ "applepush", NDPI_ONLY },
	{ "applesiri", NDPI_ONLY },
	{ "applestore", NDPI_ONLY },
	{ "appletvplus", NDPI_ONLY },
#endif
	{ "ares", PDPI_ONLY },
	{ "armagetron", DPI },
	{ "audiogalaxy", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "avast", NDPI_ONLY },
	{ "avastsecuredns", NDPI_ONLY },
	{ "azure", NDPI_ONLY },
	{ "bacnet", NDPI_ONLY },
	{ "badoo", NDPI_ONLY },
#endif
	{ "battlefield1942", L7_ONLY },
	{ "battlefield2", L7_ONLY },
	{ "battlefield2142", L7_ONLY },
	{ "bearshare", PDPI_ONLY },
	{ "bgp", DPI },
	{ "biff", L7_ONLY },
	{ "bittorrent", PDPI },
#ifdef HAVE_OPENDPI
	{ "bjnp", NDPI_ONLY },
	{ "bloomberg", NDPI_ONLY },
	{ "cachefly", NDPI_ONLY },
	{ "capwap", NDPI_ONLY },
	{ "cassandra", NDPI_ONLY },
	{ "checkmk", NDPI_ONLY },
#endif
	{ "chikka", L7_ONLY },
	{ "cimd", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "ciscoskinny", NDPI_ONLY },
#endif
	{ "ciscovpn", DPI },
	{ "citrix", DPI },
#ifdef HAVE_OPENDPI
	{ "cloudflare", NDPI_ONLY },
	{ "cloudflarewarp", NDPI_ONLY },
#endif
	{ "clubbox", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "cnn", NDPI_ONLY },
	{ "coap", NDPI_ONLY },
#endif
	{ "code_red", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "collectd", NDPI_ONLY },
	{ "corba", NDPI_ONLY },
#endif
	{ "counterstrike-source", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "cpha", NDPI_ONLY },
	{ "crashlytics", NDPI_ONLY },
	{ "crossfire", NDPI_ONLY },
	{ "crynet", NDPI_ONLY },
	{ "csgo", NDPI_ONLY },
#endif
	{ "cvs", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "cybersec", NDPI_ONLY },
	{ "datasaver", NDPI_ONLY },
	{ "dailymotion", NDPI_ONLY },
#endif
	{ "dayofdefeat-source", L7_ONLY },
	{ "dazhihui", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "dazn", NDPI_ONLY },
	{ "deezer", NDPI_ONLY },
#endif
	{ "dhcp", DPI },
#ifdef HAVE_OPENDPI
	{ "dhcpv6", NDPI_ONLY },
	{ "diameter", NDPI_ONLY },
	{ "directv", NDPI_ONLY },
#endif
#ifdef HAVE_OPENDPI
	{ "discord", NDPI_ONLY },
	{ "disneyplus", NDPI_ONLY },
	{ "dnp3", NDPI_ONLY },
#endif
	{ "dns", DPI },
#ifdef HAVE_OPENDPI
	{ "dnscrypt", NDPI_ONLY },
	{ "dofus", NDPI_ONLY },
	{ "doh_dot", NDPI_ONLY },
#endif
	{ "doom3", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "drda", NDPI_ONLY },
	{ "dropbox", NDPI_ONLY },
	{ "dtls", NDPI_ONLY },
	{ "eaq", NDPI_ONLY },
	{ "ebay", NDPI_ONLY },
	{ "edgecast", NDPI_ONLY },
#endif
	{ "edonkey", PDPI },
#ifdef HAVE_OPENDPI
	{ "egp", NDPI_ONLY },
	{ "elasticsearch", NDPI_ONLY },
	{ "ethernetip", NDPI_ONLY },
#endif
	{ "exe", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "facebook", NDPI_ONLY },
	{ "facebookvoip", NDPI_ONLY },
#endif
#ifdef HAVE_OPENDPI
	{ "fastcgi", NDPI_ONLY },
#endif
	{ "filetopia", DPI },
	{ "finger", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "fix", NDPI_ONLY },
#endif
	{ "flash", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "forticlient", NDPI_ONLY },
#endif
	{ "freegate_dns", L7_ONLY },
	{ "freegate_http", L7_ONLY },
	{ "freenet", L7_ONLY },
	{ "ftp", DPI },
#ifdef HAVE_OPENDPI
	{ "ftp_control", NDPI_ONLY },
	{ "ftps", NDPI_ONLY },
	{ "ftp_data", NDPI_ONLY },
	{ "fuze", NDPI_ONLY },
	{ "genshinimpact", NDPI_ONLY },
#endif
	{ "gif", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "git", NDPI_ONLY },
	{ "github", NDPI_ONLY },
	{ "gitlab", NDPI_ONLY },
#endif
	{ "gkrellm", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "gmail", NDPI_ONLY },
#endif
	{ "gnucleuslan", L7_ONLY },
	{ "gnutella", PDPI },
	{ "goboogy", L7_ONLY },
	{ "gogobox", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "google", NDPI_ONLY },
	{ "googleclassroom", NDPI_ONLY },
	{ "googlecloud", NDPI_ONLY },
	{ "googledocs", NDPI_ONLY },
	{ "googledrive", NDPI_ONLY },
	{ "googlehangoutduo", NDPI_ONLY },
	{ "googlemaps", NDPI_ONLY },
	{ "googleplus", NDPI_ONLY },
	{ "googleservices", NDPI_ONLY },
#endif
	{ "gopher", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "goto", NDPI_ONLY },
	{ "gre", NDPI_ONLY },
#endif
	{ "gtalk", L7_ONLY },
	{ "gtalk1", L7_ONLY },
	{ "gtalk2", L7_ONLY },
	{ "gtalk_file", L7_ONLY },
	{ "gtalk_file_1", L7_ONLY },
	{ "gtalk_vista", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "gtp", NDPI_ONLY },
	{ "gtp_c", NDPI_ONLY },
	{ "gtp_prime", NDPI_ONLY },
	{ "gtp_u", NDPI_ONLY },
#endif
	{ "guildwars", DPI },
	{ "h323", DPI },
#ifdef HAVE_OPENDPI
	{ "halflife2", NDPI_ONLY },
#endif
	{ "halflife2-deathmatch", L7_ONLY },
	{ "hamachi1", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "hbo", NDPI_ONLY },
#endif
	{ "hddtemp", L7_ONLY },
	{ "hotline", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "heroes_of_the_storm", NDPI_ONLY },
#endif
#ifdef HAVE_OPENDPI
	{ "hotspotshield", NDPI_ONLY },
	{ "hp_virtgrp", NDPI_ONLY },
	{ "hsrp", NDPI_ONLY },
#else
	{ "hotspot-shield", L7_ONLY },
#endif
	{ "html", L7_ONLY },
	{ "http", DPI },
	{ "http-dap", L7_ONLY },
	{ "http-freshdownload", L7_ONLY },
	{ "http-itunes", L7_ONLY },
	{ "http-rtsp", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "http_connect", NDPI_ONLY },
	{ "http_proxy", NDPI_ONLY },
#endif
	{ "httpaudio", L7_ONLY },
	{ "httpcachehit", L7_ONLY },
	{ "httpcachemiss", L7_ONLY },
	{ "httpvideo", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "hulu", NDPI_ONLY },
	{ "i3d", NDPI_ONLY },
	{ "iax", NDPI_ONLY },
	{ "icecast", NDPI_ONLY },
	{ "icloudprivaterelay", NDPI_ONLY },
	{ "icmp", NDPI_ONLY },
	{ "icmpv6", NDPI_ONLY },
#endif
	{ "icq_file", L7_ONLY },
	{ "icq_file_1", L7_ONLY },
	{ "icq_file_2", L7_ONLY },
	{ "icq_login", L7_ONLY },
	{ "ident", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "iec60870", NDPI_ONLY },
	{ "iflix", NDPI_ONLY },
	{ "igmp", NDPI_ONLY },
	{ "iheartradio", NDPI_ONLY },
#endif
	{ "imap", DPI },
#ifdef HAVE_OPENDPI
	{ "imaps", NDPI_ONLY },
#endif
	{ "imesh", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "imo", NDPI_ONLY },
	{ "instagram", NDPI_ONLY },
	{ "ip_in_ip", NDPI_ONLY },
	{ "ip_pim", NDPI_ONLY },
#endif
	{ "ipp", DPI },
#ifdef HAVE_OPENDPI
	{ "ipsec", NDPI_ONLY },
#endif
	{ "irc", DPI },
	{ "jabber", DPI },
	{ "jpeg", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "kakaotalk", NDPI_ONLY },
	{ "kakaotalk_voice", NDPI_ONLY },
#endif
	{ "kazaa", PDPI_ONLY },
#ifdef HAVE_OPENDPI
	{ "kerberos", NDPI_ONLY },
	{ "kismet", NDPI_ONLY },
	{ "kontiki", NDPI_ONLY },
#endif
	{ "kugoo", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "lastfm", NDPI_ONLY },
	{ "ldap", NDPI_ONLY },
	{ "likee", NDPI_ONLY },
	{ "line", NDPI_ONLY },
	{ "line_call", NDPI_ONLY },
	{ "linkedin", NDPI_ONLY },
	{ "lisp", NDPI_ONLY },
#endif
	{ "live365", L7_ONLY },
	{ "liveforspeed", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "livestream", NDPI_ONLY },
	{ "llmnr", NDPI_ONLY },
	{ "lotusnotes", NDPI_ONLY },
#endif
	{ "lpd", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "maplestory", NDPI_ONLY },
	{ "mdns", NDPI_ONLY },
	{ "megaco", NDPI_ONLY },
	{ "memcached", NDPI_ONLY },
	{ "meraki_cloud", NDPI_ONLY },
	{ "messenger", NDPI_ONLY },
	{ "mgcp", NDPI_ONLY },
	{ "microsoft", NDPI_ONLY },
	{ "microsoft365", NDPI_ONLY },
	{ "mining", NDPI_ONLY },
	{ "modbus", NDPI_ONLY },
#endif
	{ "mohaa", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "mongodb", NDPI_ONLY },
	{ "munin", NDPI_ONLY },
#endif
	{ "mp3", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "mpeg_ts", NDPI_ONLY },
	{ "mpegdash", NDPI_ONLY },
	{ "mqtt", NDPI_ONLY },
	{ "ms_onedrive", NDPI_ONLY },
#endif
	{ "msn-filetransfer", L7_ONLY },
	{ "msnmessenger", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "mssql-tds", NDPI_ONLY },
#endif
	{ "mute", PDPI_ONLY },
#ifdef HAVE_OPENDPI
	{ "mysql", NDPI_ONLY },
#endif
	{ "napster", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "natpmp", NDPI_ONLY },
	{ "nats", NDPI_ONLY },
#endif
	{ "nbns", L7_ONLY },
	{ "ncp", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "nestlogsink", NDPI_ONLY },
#endif
	{ "netbios", DPI },
#ifdef HAVE_OPENDPI
	{ "netflix", NDPI_ONLY },
	{ "netflow", NDPI_ONLY },
	{ "nfs", NDPI_ONLY },
#endif
	{ "nimda", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "nintendo", NDPI_ONLY },
#endif
	{ "nntp", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "noe", NDPI_ONLY },
	{ "ntop", NDPI_ONLY },
#endif
	{ "ntp", DPI },
#ifdef HAVE_OPENDPI
	{ "ocs", NDPI_ONLY },
	{ "ocsp", NDPI_ONLY },
#endif
	{ "ogg", DPI },
#ifdef HAVE_OPENDPI
	{ "oicq", NDPI_ONLY },
	{ "ookla", NDPI_ONLY },
	{ "opendns", NDPI_ONLY },
#endif
#ifdef HAVE_OPENDPI
	{ "openvpn", NDPI_ONLY },
	{ "oracle", NDPI_ONLY },
	{ "ospf", NDPI_ONLY },
	{ "outlook", NDPI_ONLY },
	{ "pandora", NDPI_ONLY },
	{ "pastebin", NDPI_ONLY },
#endif
	{ "pcanywhere", DPI },
	{ "pdf", L7_ONLY },
	{ "perl", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "pgm", NDPI_ONLY },
	{ "pinterest", NDPI_ONLY },
	{ "playstation", NDPI_ONLY },
	{ "playstore", NDPI_ONLY },
	{ "pluralsight", NDPI_ONLY },
#endif
	{ "png", L7_ONLY },
	{ "poco", L7_ONLY },
	{ "pop3", DPI },
#ifdef HAVE_OPENDPI
	{ "pops", NDPI_ONLY },
	{ "postgresql", NDPI_ONLY },
#endif
	{ "postscript", L7_ONLY },
	{ "pplive", DPI },
#ifdef HAVE_OPENDPI
	{ "ppstream", NDPI_ONLY },
	{ "pptp", NDPI_ONLY },
#endif
	{ "pre_icq_login", L7_ONLY },
	{ "pre_msn_login", L7_ONLY },
	{ "pre_urlblock", L7_ONLY },
	{ "pre_yahoo_login", L7_ONLY },
	{ "pressplay", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "psiphon", NDPI_ONLY },
#endif
	{ "qianlong", L7_ONLY },
	{ "qq", DPI },
	{ "qq_login", L7_ONLY },
	{ "qq_login_1", L7_ONLY },
	{ "qq_tcp_file", L7_ONLY },
	{ "qq_udp_file", L7_ONLY },
	{ "qqdownload_1", L7_ONLY },
	{ "qqdownload_2", L7_ONLY },
	{ "qqdownload_3", L7_ONLY },
	{ "qqfile", L7_ONLY },
	{ "qqgame", L7_ONLY },
	{ "qqlive", DPI },
	{ "qqlive2", L7_ONLY },
	{ "quake-halflife", L7_ONLY },
	{ "quake1", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "quic", NDPI_ONLY },
#endif
	{ "quicktime", DPI },
#ifdef HAVE_OPENDPI
	{ "radius", NDPI_ONLY },
#endif
	{ "radmin", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "raknet", NDPI_ONLY },
#endif
	{ "rar", L7_ONLY },
	{ "rdp", DPI },
#ifdef HAVE_OPENDPI
	{ "reddit", NDPI_ONLY },
	{ "redis", NDPI_ONLY },
#endif
	{ "replaytv-ivs", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "riotgames", NDPI_ONLY },
#endif
	{ "rlogin", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "rpc", NDPI_ONLY },
#endif
	{ "rpm", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "rsh", NDPI_ONLY },
	{ "rsync", NDPI_ONLY },
	{ "rtcp", NDPI_ONLY },
#endif
	{ "rtf", L7_ONLY },
	{ "rtmp", DPI },
	{ "rtp", DPI },
	{ "rtsp", DPI },
	{ "runesofmagic", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "rx", NDPI_ONLY },
	{ "s7comm", NDPI_ONLY },
	{ "salesforce", NDPI_ONLY },
	{ "sap", NDPI_ONLY },
	{ "sctp", NDPI_ONLY },
	{ "sd-rtn", NDPI_ONLY },
	{ "sflow", NDPI_ONLY },
#endif
#ifdef HAVE_OPENDPI
	{ "showtime", NDPI_ONLY },
	{ "signal", NDPI_ONLY },
	{ "signalvoip", NDPI_ONLY },
	{ "sina(weibo)", NDPI_ONLY },
#endif
	{ "sip", DPI },
#ifdef HAVE_OPENDPI
	{ "siriusxmradio", NDPI_ONLY },
	{ "skype_teams", NDPI_ONLY },
	{ "skype_teamscall", NDPI_ONLY },
#endif
	{ "skypeout", L7_ONLY },
	{ "skypetoskype", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "slack", NDPI_ONLY },
#endif
	{ "smb", DPI },
#ifdef HAVE_OPENDPI
	{ "smbv1", NDPI_ONLY },
	{ "smbv23", NDPI_ONLY },
	{ "smpp", NDPI_ONLY },
#endif
	{ "smtp", DPI },
#ifdef HAVE_OPENDPI
	{ "smtps", NDPI_ONLY },
	{ "snapchat", NDPI_ONLY },
	{ "snapchatcall", NDPI_ONLY },
#endif
	{ "snmp", DPI },
	{ "snmp-mon", L7_ONLY },
	{ "snmp-trap", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "soap", NDPI_ONLY },
#endif
	{ "socks", DPI },
#ifdef HAVE_OPENDPI
	{ "softether", NDPI_ONLY },
	{ "someip", NDPI_ONLY },
#endif
	{ "soribada", L7_ONLY },
	{ "soulseek", PDPI },
#ifdef HAVE_OPENDPI
	{ "soundcloud", NDPI_ONLY },
	{ "source_engine", NDPI_ONLY },
	{ "spotify", NDPI_ONLY },
#endif
	{ "ssdp", DPI },
	{ "ssh", DPI },
	{ "ssl", DPI },
#ifdef HAVE_OPENDPI
	{ "starcraft", NDPI_ONLY },
	{ "steam", NDPI_ONLY },
#endif
	{ "stun", DPI },
	{ "subspace", L7_ONLY },
	{ "subversion", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "syncthing", NDPI_ONLY },
	{ "syslog", NDPI_ONLY },
	{ "tailscale", NDPI_ONLY },
#endif
	{ "tar", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "targusdataspeed", NDPI_ONLY },
#endif
	{ "teamfortress2", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "teams", NDPI_ONLY },
#endif
	{ "teamspeak", DPI },
	{ "teamviewer", DPI },
	{ "teamviewer1", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "tencentvideo", NDPI_ONLY },
	{ "telegram", NDPI_ONLY },
#endif
	{ "telnet", DPI },
#ifdef HAVE_OPENDPI
	{ "tencent", NDPI_ONLY },
	{ "teredo", NDPI_ONLY },
#endif
	{ "tesla", L7_ONLY },
	{ "tftp", DPI },
	{ "thecircle", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "threema", NDPI_ONLY },
#endif
	{ "thunder5_see", L7_ONLY },
	{ "thunder5_tcp", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "tidal", NDPI_ONLY },
	{ "tiktok", NDPI_ONLY },
	{ "tinc", NDPI_ONLY },
	{ "tivoconnect", NDPI_ONLY },
	{ "tls", NDPI_ONLY },
	{ "tocaboca", NDPI_ONLY },
#endif
	{ "tonghuashun", L7_ONLY },
	{ "tor", DPI },
#ifdef HAVE_OPENDPI
	{ "tplink_shp", NDPI_ONLY },
	{ "truphone", NDPI_ONLY },
#endif
	{ "tsp", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "tuenti", NDPI_ONLY },
	{ "tumblr", NDPI_ONLY },
	{ "tunein", NDPI_ONLY },
	{ "tunnelbear", NDPI_ONLY },
	{ "tuya_lp", NDPI_ONLY },
	{ "tvuplayer", NDPI_ONLY },
	{ "twitch", NDPI_ONLY },
	{ "twitter", NDPI_ONLY },
	{ "ubntac2", NDPI_ONLY },
	{ "ubuntuone", NDPI_ONLY },
	{ "ultrasurf", NDPI_ONLY },
#endif
	{ "unset", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "usenet", NDPI_ONLY },
#endif
	{ "uucp", L7_ONLY },
	{ "validcertssl", L7_ONLY },
	{ "ventrilo", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "vevo", NDPI_ONLY },
	{ "vhua", NDPI_ONLY },
	{ "viber", NDPI_ONLY },
	{ "vimeo", NDPI_ONLY },
	{ "vk", NDPI_ONLY },
	{ "vmware", NDPI_ONLY },
#endif
	{ "vnc", DPI },
#ifdef HAVE_OPENDPI
	{ "vrrp", NDPI_ONLY },
	{ "vudu", NDPI_ONLY },
	{ "vxlan", NDPI_ONLY },
	{ "warcraft3", NDPI_ONLY },
#endif
	{ "waste", PDPI_ONLY },
#ifdef HAVE_OPENDPI
	{ "waze", NDPI_ONLY },
	{ "webex", NDPI_ONLY },
#endif
	{ "webmail_163", L7_ONLY },
	{ "webmail_gmail", L7_ONLY },
	{ "webmail_hinet", L7_ONLY },
	{ "webmail_hotmail", L7_ONLY },
	{ "webmail_pchome", L7_ONLY },
	{ "webmail_qq", L7_ONLY },
	{ "webmail_seednet", L7_ONLY },
	{ "webmail_sina", L7_ONLY },
	{ "webmail_sohu", L7_ONLY },
	{ "webmail_tom", L7_ONLY },
	{ "webmail_url", L7_ONLY },
	{ "webmail_yahoo", L7_ONLY },
	{ "webmail_yam", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "websocket", NDPI_ONLY },
	{ "wechat", NDPI_ONLY },
	{ "whatsapp", NDPI_ONLY },
	{ "whatsappcall", NDPI_ONLY },
	{ "whatsappfiles", NDPI_ONLY },
#endif
	{ "whois", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "whois-das", NDPI_ONLY },
	{ "wikipedia", NDPI_ONLY },
	{ "windowsupdate", NDPI_ONLY },
#endif
	{ "winmx", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "wireguard", NDPI_ONLY },
	{ "worldofkungfu", NDPI_ONLY },
#endif
	{ "worldofwarcraft", DPI },
#ifdef HAVE_OPENDPI
	{ "wsd", NDPI_ONLY },
#endif
	{ "x11", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "xbox", NDPI_ONLY },
#endif
	{ "xboxlive", L7_ONLY },
	{ "xdcc", PDPI_ONLY },
#ifdef HAVE_OPENDPI
	{ "xdmcp", NDPI_ONLY },
	{ "xiaomi", NDPI_ONLY },
#endif
	{ "xunlei", L7_ONLY },
	{ "yahoo", DPI },
	{ "yahoo_camera", L7_ONLY },
	{ "yahoo_file", L7_ONLY },
	{ "yahoo_login", L7_ONLY },
	{ "yahoo_voice", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "yandex", NDPI_ONLY },
	{ "yandex_cloud", NDPI_ONLY },
	{ "yandex_direct", NDPI_ONLY },
	{ "yandex_disk", NDPI_ONLY },
	{ "yandex_mail", NDPI_ONLY },
	{ "yandex_market", NDPI_ONLY },
	{ "yandex_metrika", NDPI_ONLY },
	{ "yandex_music", NDPI_ONLY },
#endif
	{ "youtube", DPI },
#ifdef HAVE_OPENDPI
	{ "youtubeupload", NDPI_ONLY },
	{ "z3950", NDPI_ONLY },
	{ "zabbix", NDPI_ONLY },
	{ "zattoo", NDPI_ONLY },
	{ "zeromq", NDPI_ONLY },
#endif
	{ "zip", L7_ONLY },
	{ "zmaap", L7_ONLY },
#ifdef HAVE_OPENDPI
	{ "zoom", NDPI_ONLY },
#endif
	{ 0, 0 },
};
