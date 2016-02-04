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

#ifdef HAVE_OPENDPI
#define DPI 2			//open dpi based
#define PDPI 2			//open dpi based
#else
#define DPI 0			//default l7
#define PDPI 1			//default p2p
#endif
//Added ,  (in extra), dazhihui, .

l7filters filters_list[] = {
#ifdef HAVE_OPENDPI
	{"unknown", NDPI_ONLY},
#endif
	{"100bao", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"99taxi", NDPI_ONLY},
	{"afp", NDPI_ONLY},
#endif
	{"aim", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"aimini", NDPI_ONLY},
#endif
	{"aimwebcontent", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"amazon", NDPI_ONLY},
	{"apple", NDPI_ONLY},
	{"apple_icloud", NDPI_ONLY},
	{"apple_itunes", NDPI_ONLY},
#endif
	{"applejuice", PDPI},
	{"ares", PDPI_ONLY},
	{"armagetron", DPI},
	{"audiogalaxy", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"avi", NDPI_ONLY},
	{"ayiya", NDPI_ONLY},
	{"battlefield", NDPI_ONLY},
#endif
	{"battlefield1942", L7_ONLY},
	{"battlefield2", L7_ONLY},
	{"battlefield2142", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"battlenet", NDPI_ONLY},
#endif
	{"bearshare", PDPI_ONLY},
	{"bgp", DPI},
	{"biff", L7_ONLY},
	{"bittorrent", PDPI},
	{"chikka", L7_ONLY},
	{"cimd", L7_ONLY},
	{"ciscovpn", DPI},
	{"citrix", DPI},
#ifdef HAVE_OPENDPI
	{"citrix_online", NDPI_ONLY},
#endif
	{"clubbox", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"cnn", NDPI_ONLY},
#endif
	{"code_red", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"collectd", NDPI_ONLY},
	{"corba", NDPI_ONLY},
#endif
	{"counterstrike-source", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"crossfire", NDPI_ONLY},
#endif
	{"cvs", L7_ONLY},
	{"dayofdefeat-source", L7_ONLY},
	{"dazhihui", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"dcerpc", NDPI_ONLY},
	{"deezer", NDPI_ONLY},
#endif
	{"dhcp", DPI},
#ifdef HAVE_OPENDPI
	{"dhcpv6", NDPI_ONLY},
	{"direct_download_link", NDPI_ONLY},
#endif
	{"directconnect", PDPI},
	{"dns", DPI},
#ifdef HAVE_OPENDPI
	{"dofus", NDPI_ONLY},
#endif
	{"doom3", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"dropbox", NDPI_ONLY},
	{"easytaxi", NDPI_ONLY},
	{"eaq", NDPI_ONLY},
	{"ebay", NDPI_ONLY},
#endif
	{"edonkey", PDPI},
#ifdef HAVE_OPENDPI
	{"egp", NDPI_ONLY},
	{"epp", NDPI_ONLY},
#endif
	{"exe", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"facebook", NDPI_ONLY},
#endif
	{"fasttrack", DPI},
#ifdef HAVE_OPENDPI
	{"feidian", NDPI_ONLY},
	{"fiesta", NDPI_ONLY},
#endif
	{"filetopia", DPI},
	{"finger", L7_ONLY},
	{"flash", DPI},
#ifdef HAVE_OPENDPI
	{"florensia", NDPI_ONLY},
#endif
	{"freegate_dns", L7_ONLY},
	{"freegate_http", L7_ONLY},
	{"freenet", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"ftp_control", NDPI_ONLY},
	{"ftp_data", NDPI_ONLY},
	{"gadugadu", NDPI_ONLY},
#else
	{"ftp", DPI},
#endif
	{"gif", L7_ONLY},
	{"gkrellm", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"globotv", NDPI_ONLY},
	{"gmail", NDPI_ONLY},
#endif
	{"gnucleuslan", L7_ONLY},
	{"gnutella", PDPI},
	{"goboogy", L7_ONLY},
	{"gogobox", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"google", NDPI_ONLY},
	{"google_maps", NDPI_ONLY},
#endif
	{"gopher", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"gre", NDPI_ONLY},
	{"grooveshark", NDPI_ONLY},
#endif
	{"gtalk", L7_ONLY},
	{"gtalk1", L7_ONLY},
	{"gtalk2", L7_ONLY},
	{"gtalk_file", L7_ONLY},
	{"gtalk_file_1", L7_ONLY},
	{"gtalk_vista", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"gtp", NDPI_ONLY},
#endif
	{"guildwars", DPI},
	{"h323", DPI},
#ifdef HAVE_OPENDPI
	{"halflife2", NDPI_ONLY},
#endif
	{"halflife2-deathmatch", L7_ONLY},
	{"hamachi1", L7_ONLY},
	{"hddtemp", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"hep", NDPI_ONLY},
	{"hl2", NDPI_ONLY},
#endif
	{"hotline", L7_ONLY},
	{"hotspot-shield", DPI},
	{"html", L7_ONLY},
	{"http", DPI},
	{"http-dap", L7_ONLY},
	{"http-freshdownload", L7_ONLY},
	{"http-itunes", L7_ONLY},
	{"http-rtsp", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"http_app_activesync", NDPI_ONLY},
	{"http_app_veohtv", NDPI_ONLY},
	{"http_connect", NDPI_ONLY},
	{"http_proxy", NDPI_ONLY},
	{"httpactivesync", NDPI_ONLY},
#endif
	{"httpaudio", L7_ONLY},
	{"httpcachehit", L7_ONLY},
	{"httpcachemiss", L7_ONLY},
	{"httpvideo", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"i23v5", NDPI_ONLY},
	{"iax", NDPI_ONLY},
	{"icecast", NDPI_ONLY},
	{"icmp", NDPI_ONLY},
#endif
	{"icq_file", L7_ONLY},
	{"icq_file_1", L7_ONLY},
	{"icq_file_2", L7_ONLY},
	{"icq_login", L7_ONLY},
	{"ident", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"igmp", NDPI_ONLY},
#endif
	{"imap", DPI},
#ifdef HAVE_OPENDPI
	{"imap_ssl", NDPI_ONLY},
#endif
	{"imesh", DPI},
#ifdef HAVE_OPENDPI
	{"imessage_facetime", NDPI_ONLY},
	{"instagram", NDPI_ONLY},
	{"ip_egp", NDPI_ONLY},
	{"ip_gre", NDPI_ONLY},
	{"ip_icmp", NDPI_ONLY},
	{"ip_icmpv6", NDPI_ONLY},
	{"ip_igmp", NDPI_ONLY},
	{"ip_ip_in_ip", NDPI_ONLY},
	{"ip_ipsec", NDPI_ONLY},
	{"ip_ospf", NDPI_ONLY},
	{"ip_sctp", NDPI_ONLY},
	{"ip_vrrp", NDPI_ONLY},
	{"ipip", NDPI_ONLY},
#endif
	{"ipp", DPI},
#ifdef HAVE_OPENDPI
	{"ipsec", NDPI_ONLY},
#endif
	{"irc", DPI},
	{"jabber", DPI},
	{"jpeg", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"kakaotalk", NDPI_ONLY},
	{"kakaotalk_voice", NDPI_ONLY},
#endif
	{"kazaa", PDPI_ONLY},
#ifdef HAVE_OPENDPI
	{"kerberos", NDPI_ONLY},
	{"kontiki", NDPI_ONLY},
#endif
	{"kugoo", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"lastfm", NDPI_ONLY},
	{"ldap", NDPI_ONLY},
#endif
	{"live365", L7_ONLY},
	{"liveforspeed", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"llmnr", NDPI_ONLY},
	{"lotus_notes", NDPI_ONLY},
#endif
	{"lpd", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"manolito", NDPI_ONLY},
	{"maplestory", NDPI_ONLY},
	{"mdns", NDPI_ONLY},
	{"meebo", NDPI_ONLY},
	{"megaco", NDPI_ONLY},
	{"meu", NDPI_ONLY},
	{"mgcp", NDPI_ONLY},
	{"microsoft", NDPI_ONLY},
	{"mms", NDPI_ONLY},
#endif
	{"mohaa", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"move", NDPI_ONLY},
#endif
	{"mp3", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"mpeg", NDPI_ONLY},
	{"ms_lync", NDPI_ONLY},
	{"msn", NDPI_ONLY},
#endif
	{"msn-filetransfer", L7_ONLY},
	{"msnmessenger", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"mssql", NDPI_ONLY},
#endif
	{"mute", PDPI_ONLY},
#ifdef HAVE_OPENDPI
	{"mysql", NDPI_ONLY},
#endif
	{"napster", L7_ONLY},
	{"nbns", L7_ONLY},
	{"ncp", L7_ONLY},
	{"netbios", DPI},
#ifdef HAVE_OPENDPI
	{"netflix", NDPI_ONLY},
	{"netflow", NDPI_ONLY},
	{"nfs", NDPI_ONLY},
#endif
	{"nimda", L7_ONLY},
	{"nntp", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"noe", NDPI_ONLY},
#endif
	{"ntp", DPI},
#ifdef HAVE_OPENDPI
	{"off", NDPI_ONLY},
#endif
	{"ogg", DPI},
	{"openft", DPI},
#ifdef HAVE_OPENDPI
	{"opensignal", NDPI_ONLY},
	{"openvpn", NDPI_ONLY},
	{"oracle", NDPI_ONLY},
	{"oscar", NDPI_ONLY},
	{"pando", NDPI_ONLY},
	{"pandora", NDPI_ONLY},
#endif
	{"pcanywhere", DPI},
	{"pdf", L7_ONLY},
	{"perl", L7_ONLY},
	{"png", L7_ONLY},
	{"poco", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"pop", NDPI_ONLY},
#endif
	{"pop3", DPI},
#ifdef HAVE_OPENDPI
	{"pop3_ssl", NDPI_ONLY},
	{"popo", NDPI_ONLY},
	{"postgres", NDPI_ONLY},
#endif
	{"postscript", L7_ONLY},
	{"pplive", DPI},
#ifdef HAVE_OPENDPI
	{"ppstream", NDPI_ONLY},
	{"pptp", NDPI_ONLY},
#endif
	{"pre_icq_login", L7_ONLY},
	{"pre_msn_login", L7_ONLY},
	{"pre_urlblock", L7_ONLY},
	{"pre_yahoo_login", L7_ONLY},
	{"pressplay", L7_ONLY},
	{"qianlong", L7_ONLY},
	{"qq", DPI},
	{"qq_login", L7_ONLY},
	{"qq_login_1", L7_ONLY},
	{"qq_tcp_file", L7_ONLY},
	{"qq_udp_file", L7_ONLY},
	{"qqdownload_1", L7_ONLY},
	{"qqdownload_2", L7_ONLY},
	{"qqdownload_3", L7_ONLY},
	{"qqfile", L7_ONLY},
	{"qqgame", L7_ONLY},
	{"qqlive", DPI},
	{"qqlive2", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"quake", NDPI_ONLY},
#endif
	{"quake-halflife", L7_ONLY},
	{"quake1", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"quic", NDPI_ONLY},
	{"quickplay", NDPI_ONLY},
#endif
	{"quicktime", DPI},
#ifdef HAVE_OPENDPI
	{"radius", NDPI_ONLY},
#endif
	{"radmin", L7_ONLY},
	{"rar", L7_ONLY},
	{"rdp", DPI},
#ifdef HAVE_OPENDPI
	{"realmedia", NDPI_ONLY},
	{"redis", NDPI_ONLY},
	{"remote_scan", NDPI_ONLY},
	{"remotescan", NDPI_ONLY},
#endif
	{"replaytv-ivs", L7_ONLY},
	{"rlogin", L7_ONLY},
	{"rpm", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"rsync", NDPI_ONLY},
	{"rtcp", NDPI_ONLY},
#endif
	{"rtf", L7_ONLY},
	{"rtmp", DPI},
	{"rtp", DPI},
	{"rtsp", DPI},
	{"runesofmagic", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"sap", NDPI_ONLY},
	{"sctp", NDPI_ONLY},
	{"secondlife", NDPI_ONLY},
	{"sflow", NDPI_ONLY},
#endif
	{"shoutcast", DPI},
#ifdef HAVE_OPENDPI
	{"simet", NDPI_ONLY},
#endif
	{"sip", DPI},
#ifdef HAVE_OPENDPI
	{"skinny", NDPI_ONLY},
	{"skyfile_post", NDPI_ONLY},
	{"skyfile_postpaid", NDPI_ONLY},
	{"skyfile_pre", NDPI_ONLY},
	{"skyfile_prepaid", NDPI_ONLY},
	{"skyfile_ru", NDPI_ONLY},
	{"skyfile_rudics", NDPI_ONLY},
	{"skype", NDPI_ONLY},
#endif
	{"skypeout", L7_ONLY},
	{"skypetoskype", L7_ONLY},
	{"smb", DPI},
	{"smtp", DPI},
#ifdef HAVE_OPENDPI
	{"smtp_ssl", NDPI_ONLY},
	{"snapchat", NDPI_ONLY},
#endif
	{"snmp", DPI},
	{"snmp-mon", L7_ONLY},
	{"snmp-trap", L7_ONLY},
	{"socks", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"socks4", NDPI_ONLY},
	{"socks5", NDPI_ONLY},
	{"socrates", NDPI_ONLY},
	{"sopcast", NDPI_ONLY},
#endif
	{"soribada", L7_ONLY},
	{"soulseek", PDPI},
#ifdef HAVE_OPENDPI
	{"spotify", NDPI_ONLY},
#endif
	{"ssdp", DPI},
	{"ssh", DPI},
	{"ssl", DPI},
#ifdef HAVE_OPENDPI
	{"ssl_no_cert", NDPI_ONLY},
	{"starcraft", NDPI_ONLY},
	{"stealthnet", NDPI_ONLY},
	{"steam", NDPI_ONLY},
#endif
	{"stun", DPI},
	{"subspace", L7_ONLY},
	{"subversion", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"syslog", NDPI_ONLY},
#endif
	{"tar", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"tds", NDPI_ONLY},
#endif
	{"teamfortress2", L7_ONLY},
	{"teamspeak", DPI},
	{"teamviewer", DPI},
	{"teamviewer1", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"telegram", NDPI_ONLY},
#endif
	{"telnet", DPI},
#ifdef HAVE_OPENDPI
	{"teredo", NDPI_ONLY},
#endif
	{"tesla", L7_ONLY},
	{"tftp", DPI},
	{"thecircle", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"thunder", NDPI_ONLY},
#endif
	{"thunder5_see", L7_ONLY},
	{"thunder5_tcp", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"timmeu", NDPI_ONLY},
	{"timsomdechamada", NDPI_ONLY},
	{"timmenu", NDPI_ONLY},
	{"timportasabertas", NDPI_ONLY},
	{"timrecarga", NDPI_ONLY},
	{"timbeta", NDPI_ONLY},
#endif
	{"tonghuashun", L7_ONLY},
	{"tor", DPI},
#ifdef HAVE_OPENDPI
	{"torcedor", NDPI_ONLY},
	{"truphone", NDPI_ONLY},
#endif
	{"tsp", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"tuenti", NDPI_ONLY},
	{"tvants", NDPI_ONLY},
	{"tvuplayer", NDPI_ONLY},
	{"twitch", NDPI_ONLY},
	{"twitter", NDPI_ONLY},
	{"ubntac2", NDPI_ONLY},
	{"ubuntuone", NDPI_ONLY},
	{"unencryped_jabber", NDPI_ONLY},
#endif
	{"unset", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"upnp", NDPI_ONLY},
	{"usenet", NDPI_ONLY},
#endif
	{"uucp", L7_ONLY},
	{"validcertssl", L7_ONLY},
	{"ventrilo", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"veohtv", NDPI_ONLY},
	{"vevo", NDPI_ONLY},
	{"vhua", NDPI_ONLY},
	{"viber", NDPI_ONLY},
	{"vmware", NDPI_ONLY},
#endif
	{"vnc", DPI},
#ifdef HAVE_OPENDPI
	{"warcraft3", NDPI_ONLY},
#endif
	{"waste", PDPI_ONLY},
#ifdef HAVE_OPENDPI
	{"webex", NDPI_ONLY},
	{"webm", NDPI_ONLY},
#endif
	{"webmail_163", L7_ONLY},
	{"webmail_gmail", L7_ONLY},
	{"webmail_hinet", L7_ONLY},
	{"webmail_hotmail", L7_ONLY},
	{"webmail_pchome", L7_ONLY},
	{"webmail_qq", L7_ONLY},
	{"webmail_seednet", L7_ONLY},
	{"webmail_sina", L7_ONLY},
	{"webmail_sohu", L7_ONLY},
	{"webmail_tom", L7_ONLY},
	{"webmail_url", L7_ONLY},
	{"webmail_yahoo", L7_ONLY},
	{"webmail_yam", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"whatOAsapp", NDPI_ONLY},
	{"whatsapp", NDPI_ONLY},
#endif
	{"whois", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"whois_das", NDPI_ONLY},
	{"wikipedia", NDPI_ONLY},
	{"windows_update", NDPI_ONLY},
	{"windowsmedia", NDPI_ONLY},
#endif
	{"winmx", PDPI},
#ifdef HAVE_OPENDPI
	{"winupdate", NDPI_ONLY},
	{"wokf", NDPI_ONLY},
	{"world_of_kung_fu", NDPI_ONLY},
#endif
	{"worldofwarcraft", DPI},
	{"x11", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"xbox", NDPI_ONLY},
#endif
	{"xboxlive", L7_ONLY},
	{"xdcc", PDPI_ONLY},
#ifdef HAVE_OPENDPI
	{"xdmcp", NDPI_ONLY},
#endif
	{"xunlei", L7_ONLY},
	{"yahoo", DPI},
	{"yahoo_camera", L7_ONLY},
	{"yahoo_file", L7_ONLY},
	{"yahoo_login", L7_ONLY},
	{"yahoo_voice", L7_ONLY},
	{"youtube", DPI},
#ifdef HAVE_OPENDPI
	{"zattoo", NDPI_ONLY},
	{"zeromq", NDPI_ONLY},
#endif
	{"zip", L7_ONLY},
	{"zmaap", L7_ONLY},
#ifdef HAVE_OPENDPI
	{"zmq", NDPI_ONLY},
#endif
	{0, 0},
};
