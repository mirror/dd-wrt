#define NDPI_STATIC
/* 
 * libxt_ndpi.c
 * Copyright (C) 2010-2012 G. Elian Gidoni <geg@gnu.org>
 *               2012 Ed Wildgoose <lists@wildgooses.com>
 * 
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <iptables.h>

//#include <linux/version.h>

#define NDPI_IPTABLES_EXT
#include "xt_ndpi.h"
#include "ndpi_config.h"

#include "regexp.c"

#define true 1
#define false 0

/* copy from ndpi_main.c */

int NDPI_BITMASK_IS_EMPTY(NDPI_PROTOCOL_BITMASK a) {
  int i;

  for(i=0; i<NDPI_NUM_FDS_BITS; i++)
    if(a.fds_bits[i] != 0)
      return(0);

  return(1);
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

//#if NDPI_LAST_IMPLEMENTED_PROTOCOL != NDPI_PROTOCOL_MAXNUM
//#error LAST_IMPLEMENTED_PROTOCOL != PROTOCOL_MAXNUM
//#endif
//

static char *prot_short_str[NDPI_NUM_BITS] = {
	"unknown",
	"ftp_control",
	"pop3",
	"smtp",
	"imap",
	"dns",
	"ipp",
	"http",
	"mdns",
	"ntp",
	"netbios",
	"nfs",
	"ssdp",
	"bgp",
	"snmp",
	"xdmcp",
	"smbv1",
	"syslog",
	"dhcp",
	"postgresql",
	"mysql",
	"outlook",
	"vk",
	"pops",
	"tailscale",
	"yandex",
	"ntop",
	"coap",
	"vmware",
	"smtps",
	"dtls",
	"ubntac2",
	"bfcp",
	"yandex_mail",
	"yandex_music",
	"gnutella",
	"edonkey",
	"bittorrent",
	"skype_teamscall",
	"signal",
	"memcached",
	"smbv23",
	"mining",
	"nestlogsink",
	"modbus",
	"whatsappcall",
	"datasaver",
	"xbox",
	"qq",
	"tiktok",
	"rtsp",
	"imaps",
	"icecast",
	"cpha",
	"iqiyi",
	"zattoo",
	"yandex_market",
	"yandex_disk",
	"discord",
	"adobe_connect",
	"mongodb",
	"pluralsight",
	"yandex_cloud",
	"ocsp",
	"vxlan",
	"irc",
	"meraki_cloud",
	"jabber",
	"nats",
	"amongus",
	"yahoo",
	"disneyplus",
	"hart_ip",
	"vrrp",
	"steam",
	"halflife2",
	"worldofwarcraft",
	"telnet",
	"stun",
	"ipsec",
	"gre",
	"icmp",
	"igmp",
	"egp",
	"sctp",
	"ospf",
	"ip_in_ip",
	"rtp",
	"rdp",
	"vnc",
	"tumblr",
	"tls",
	"ssh",
	"usenet",
	"mgcp",
	"iax",
	"tftp",
	"afp",
	"yandex_metrika",
	"yandex_direct",
	"sip",
	"truphone",
	"icmpv6",
	"dhcpv6",
	"armagetron",
	"crossfire",
	"dofus",
	"ads_analytics_track",
	"adult_content",
	"guildwars",
	"amazonalexa",
	"kerberos",
	"ldap",
	"maplestory",
	"mssql-tds",
	"pptp",
	"warcraft3",
	"worldofkungfu",
	"slack",
	"facebook",
	"twitter",
	"dropbox",
	"gmail",
	"googlemaps",
	"youtube",
	"skype_teams",
	"google",
	"ms_rpch",
	"netflow",
	"sflow",
	"http_connect",
	"http_proxy",
	"citrix",
	"netflix",
	"lastfm",
	"waze",
	"youtubeupload",
	"hulu",
	"checkmk",
	"ajp",
	"apple",
	"webex",
	"whatsapp",
	"appleicloud",
	"viber",
	"appleitunes",
	"radius",
	"windowsupdate",
	"teamviewer",
	"egd",
	"lotusnotes",
	"sap",
	"gtp",
	"wsd",
	"llmnr",
	"tocaboca",
	"spotify",
	"facebook_messenger",
	"h323",
	"openvpn",
	"noe",
	"ciscovpn",
	"teamspeak",
	"tor",
	"ciscoskinny",
	"rtcp",
	"rsync",
	"oracle",
	"corba",
	"ubuntuone",
	"whois-das",
	"sd-rtn",
	"socks",
	"nintendo",
	"rtmp",
	"ftp_data",
	"wikipedia",
	"zeromq",
	"amazon",
	"ebay",
	"cnn",
	"megaco",
	"resp",
	"pinterest",
	"vhua",
	"telegram",
	"cod_mobile",
	"pandora",
	"quic",
	"zoom",
	"eaq",
	"ookla",
	"amqp",
	"kakaotalk",
	"kakaotalk_voice",
	"twitch",
	"doh_dot",
	"wechat",
	"mpeg_ts",
	"snapchat",
	"sina",
	"googlemeet",
	"iflix",
	"github",
	"bjnp",
	"reddit",
	"wireguard",
	"smpp",
	"dnscrypt",
	"tinc",
	"deezer",
	"instagram",
	"microsoft",
	"starcraft",
	"teredo",
	"hotspotshield",
	"imo",
	"googledrive",
	"ocs",
	"microsoft365",
	"cloudflare",
	"ms_onedrive",
	"mqtt",
	"rx",
	"applestore",
	"opendns",
	"git",
	"drda",
	"playstore",
	"someip",
	"fix",
	"playstation",
	"pastebin",
	"linkedin",
	"soundcloud",
	"valve_sdr",
	"lisp",
	"diameter",
	"applepush",
	"googleservices",
	"amazonvideo",
	"googledocs",
	"whatsappfiles",
	"targusdataspeed",
	"dnp3",
	"iec60870",
	"bloomberg",
	"capwap",
	"zabbix",
	"s7comm",
	"teams",
	"websocket",
	"anydesk",
	"soap",
	"applesiri",
	"snapchatcall",
	"hp_virtgrp",
	"genshinimpact",
	"activision",
	"forticlient",
	"z3950",
	"likee",
	"gitlab",
	"avastsecuredns",
	"cassandra",
	"amazonaws",
	"salesforce",
	"vimeo",
	"facebookvoip",
	"signalvoip",
	"fuze",
	"gtp_u",
	"gtp_c",
	"gtp_prime",
	"alibaba",
	"crashlytics",
	"azure",
	"icloudprivaterelay",
	"ethernetip",
	"badoo",
	"accuweather",
	"googleclassroom",
	"hsrp",
	"cybersec",
	"googlecloud",
	"tencent",
	"raknet",
	"xiaomi",
	"edgecast",
	"cachefly",
	"softether",
	"mpegdash",
	"dazn",
	"goto",
	"rsh",
	"1kxun",
	"pgm",
	"ip_pim",
	"collectd",
	"tunnelbear",
	"cloudflarewarp",
	"i3d",
	"riotgames",
	"psiphon",
	"ultrasurf",
	"threema",
	"alicloud",
	"avast",
	"tivoconnect",
	"kismet",
	"fastcgi",
	"ftps",
	"natpmp",
	"syncthing",
	"crynet",
	"line",
	"line_call",
	"appletvplus",
	"directv",
	"hbo",
	"vudu",
	"showtime",
	"dailymotion",
	"livestream",
	"tencentvideo",
	"iheartradio",
	"tidal",
	"tunein",
	"siriusxmradio",
	"munin",
	"elasticsearch",
	"tuya_lp",
	"tplink_shp",
	"source_engine",
	"bacnet",
	"oicq",
	"heroes_of_the_storm",
	"facebook_reel_story",
	"srtp",
	"opera_vpn",
	"epicgames",
	"geforcenow",
	"nvidia",
	"bitcoin",
	"protonvpn",
	"apache_thrift",
	"roblox",
	"service-location",
	"mullvad",
	"http2",
	"haproxy",
	"rmcp",
	"can",
	"protobuf",
	"ethereum",
	"telegram_voip",
	"sina_weibo",
	"tesla_services",
	"ptpv2",
	"rtps",
	"opc_ua",
	"s7comm_plus",
	"fins",
	"ethersio",
	"umas",
	"beckhoff_ads",
	"iso9506_1_mss",
	"ieee_c37118",
	"ethersbus",
	"monero",
	"dcerpc",
	"profinet_io",
	"hislip",
	"uftp",
	"openflow",
	"json_rpc",
	"webdav",
	"apache_kafka",
	"nomachine",
	"iec62056",
	"hl7",
	"ceph",
	"googlechat",
	"roughtime",
	"pia",
	"kcp",
	"dota2",
	"mumble",
	"yojimbo",
	"electronicarts",
	"stomp",
	"radmin",
	"raft",
	"cip",
	"gearman",
	"tencentgames",
	"gaijin",
	"c1222",
	"huawei",
	"huawei_cloud",
	"dlep",
	"bfd",
	"netease_games",
	"path_of_exile",
	"googlecall",
	"pfcp",
	"flute",
	"lol_wild_rift",
	"teso",
	"ldp",
	"knxnet_ip",
	"bluesky",
	"mastodon",
	"threads",
	"viber_voip",
	"zug",
	"jrmi",
	"ripe_atlas",
	"hls",
	"clickhouse",
	"nano",
	"openwire",
	"cnp_ip",
	"atg",
	"trdp",
	"lustre",
	"nordvpn",
	"surfshark",
	"cactusvpn",
	"windscribe",
	NULL,
};

static char  prot_disabled[NDPI_NUM_BITS+1] = { 0, };

#define EXT_OPT_BASE 0
// #define EXT_OPT_BASE NDPI_LAST_IMPLEMENTED_PROTOCOL
enum ndpi_opt_index {
	NDPI_OPT_UNKNOWN=0,
	NDPI_OPT_ALL,
	NDPI_OPT_ERROR,
	NDPI_OPT_PROTO,
	NDPI_OPT_MPROTO,
	NDPI_OPT_APROTO,
	NDPI_OPT_HMASTER,
	NDPI_OPT_HOST,
	NDPI_OPT_INPROGRESS,
	NDPI_OPT_JA3S,
	NDPI_OPT_JA3C,
	NDPI_OPT_TLSFP,
	NDPI_OPT_TLSV,
	NDPI_OPT_UNTRACKED,
	NDPI_OPT_CLEVEL,
	NDPI_OPT_LAST
};

#define FLAGS_ALL 0x1
#define FLAGS_ERR 0x2
#define FLAGS_HMASTER 0x4
#define FLAGS_MPROTO 0x8
#define FLAGS_APROTO 0x10
#define FLAGS_HOST 0x20
#define FLAGS_INPROGRESS 0x40
#define FLAGS_PROTO 0x80
#define FLAGS_JA3S 0x100
#define FLAGS_JA3C 0x200
#define FLAGS_TLSFP 0x400
#define FLAGS_TLSV 0x800
#define FLAGS_UNTRACKED 0x1000
#define FLAGS_CLEVEL 0x2000
#define FLAGS_HPROTO 0x4000

static char *_clevel2str[] = {
 "unknown", "port", "ip", "user",
 "dpart",  "dcpart", "dcache", "dpi" };
static int clevelnc[] = { 2, 1, 1, 2, 3, 3, 3, 3 };

#define clevel2num (sizeof(_clevel2str)/sizeof(_clevel2str[0]))

static const char *clevel2str(int l) {
	return (l > 0 && l < clevel2num) ? _clevel2str[l] : "?";
}
static const char *clevel_op2str(int l) {
	switch(l) {
	  case 1: return "-";
	  case 2: return "+";
	}
	return "";
}
static int str2clevel(const char *s) {
	int i;
	char *e;

	for(i=0; i < clevel2num; i++)
	    if(!strcasecmp(_clevel2str[i],s)) return i;
	for(i=0; i < clevel2num; i++)
	    if(!strncasecmp(_clevel2str[i],s,clevelnc[i])) return i;
	i = strtol(s,&e,0);
	if(*e) return 0;
	return i < 0 || i > 7 ? 0 : i;
}

static void 
_ndpi_mt4_save(const struct ipt_ip *entry, const struct ipt_entry_match *match, int save)
{
	const struct xt_ndpi_mtinfo *info = (const void *)match->data;
	const char *cinv = info->invert ? "! ":"";
	const char *csave = save ? "--":"";
        int i,c,l,t;

        for (t = l = c = i = 0; i < NDPI_NUM_BITS; i++) {
		if (!prot_short_str[i] || prot_disabled[i] || 
				!strncmp(prot_short_str[i],"badproto_",9)) continue;
		t++;
		if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0) {
			l = i; c++;
		}
	}

	if(!save)
		printf(" ndpi");
	if(info->error) {
		printf(" %s%serror",cinv,csave);
		return;
	}
	if(info->untracked) {
		printf(" %s%suntracked",cinv,csave);
		return;
	}
	if(info->invert)
		printf(" !");

	if(info->hostname[0]) {
		printf(" %shost %s",csave,info->hostname);
	}
	if(info->have_master) {
		printf(" %shave-master",csave);
	}
	if(info->clevel) {
		printf(" %sclevel %s%s", csave, clevel_op2str(info->clevel_op),
				clevel2str(info->clevel));
	}
	if(info->m_proto && !info->p_proto)
		printf(" %smatch-m-proto",csave);
	if(!info->m_proto && info->p_proto)
		printf(" %smatch-a-proto",csave);

	if(!c) return;
	printf(" %s",csave);
	if(info->inprogress) {
	  printf("inprogress");
	} else if(info->ja3s) {
	  printf("ja3s");
	} else if(info->ja3c) {
	  printf("ja3c");
	} else if(info->tlsfp) {
	  printf("tlsfp");
	} else if(info->tlsv) {
	  printf("tlsv");
	} else
	  printf("proto");

	if( c == 1) {
		printf(" %s",prot_short_str[l]);
		return;
	}
	if( c == t-1 && 
	    !NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,NDPI_PROTOCOL_UNKNOWN) ) { 
		printf(" all");
		return;
	}

	if(c > t/2 + 1) {
	    printf(" all");
	    for (i = 1; i < NDPI_NUM_BITS; i++) {
                if (prot_short_str[i] && !prot_disabled[i] && NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) == 0)
			printf(",-%s", prot_short_str[i]);
	    }
	    return;
	}
	printf(" ");
        for (l = i = 0; i < NDPI_NUM_BITS; i++) {
                if (prot_short_str[i] && !prot_disabled[i] && NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0)
			printf("%s%s",l++ ? ",":"", prot_short_str[i]);
        }
}

static void 
ndpi_mt4_save(const struct ipt_ip *entry, const struct xt_entry_match *match) {
	_ndpi_mt4_save(entry,match,1);
}

static void 
ndpi_mt4_print(const struct ipt_ip *entry, const struct xt_entry_match *match,
                  int numeric)
{
	_ndpi_mt4_save(entry,match,0);
}



static int 
ndpi_mt4_parse(int c, char **argv, int invert, unsigned int *flags,
                  const struct ipt_entry *entry,unsigned int *nfcache, struct ipt_entry_match **match)
{
	struct xt_ndpi_mtinfo *info = (void *)(*match)->data;
        int i;

	info->invert = invert;

	if(c == NDPI_OPT_ERROR) {
		info->error = 1;
        	*flags |= FLAGS_ERR;
		return true;
	}
	if(c == NDPI_OPT_HMASTER ) {
		info->have_master = 1;
        	*flags |= FLAGS_HMASTER;
		return true;
	}
	if(c == NDPI_OPT_UNTRACKED ) {
		info->untracked = 1;
                *flags |= FLAGS_UNTRACKED;
                return true;
	}
	if(c == NDPI_OPT_MPROTO) {
		info->m_proto = 1;
        	*flags |= FLAGS_MPROTO;
		return true;
	}
	if(c == NDPI_OPT_CLEVEL) {
		int cl = 0;
		info->clevel_op = 0;
		if(optarg[0] == '-') {
			info->clevel_op = 1;
			cl = str2clevel(optarg+1);
		} else if(optarg[0] == '+') {
			info->clevel_op = 2;
			cl = str2clevel(optarg+1);
		} else
			cl = str2clevel(optarg);
		if(!cl) {
			printf("Error: invalid clevel %s\n",optarg);
			return false;
		}
		if(info->clevel == 0 && info->clevel_op == 1) {
			printf("Error: impossible condition '-unknown'\n");
			return false;
		}
		if(info->clevel == 5 && info->clevel_op == 2) {
			printf("Error: impossible condition '+dpi'\n");
			return false;
		}
		info->clevel = cl;
        	*flags |= FLAGS_CLEVEL;
		return true;
	}
	if(c == NDPI_OPT_APROTO) {
		info->p_proto = 1;
        	*flags |= FLAGS_APROTO;
		return true;
	}

	if(c == NDPI_OPT_HOST) {
		char *s;
		int re_len = strlen(optarg);

		if(re_len >= sizeof(info->hostname)-1) {
			printf("Error: host name too long. Allowed %zu chars\n",
					sizeof(info->hostname)-1);
			return false;
		}
		if(!*optarg) {
			printf("Error: empty host name\n");
			return false;
		}
		if(info->hostname[0]) {
			printf("Error: Double --host\n");
			return false;
		}
		strncpy(info->hostname,optarg,sizeof(info->hostname)-1);

		for(s = &info->hostname[0]; *s; s++) *s = tolower(*s);

		info->host = 1;
		*flags |= FLAGS_HOST;

		if(info->hostname[0] == '/') {
			char re_buf[sizeof(info->hostname)];
			regexp *pattern;

			if(re_len < 3 || info->hostname[re_len-1] != '/') {
				printf("Invalid regexp '%s'\n",info->hostname);
				return false;
			}
			re_len -= 2;
			strncpy(re_buf,&info->hostname[1],re_len);
			re_buf[re_len] = '\0';

			pattern = ndpi_regcomp(re_buf, &re_len);

			if(!pattern) {
				printf("Bad regexp '%s' '%s'\n",&info->hostname[1],re_buf);
				return false;
			}
			ndpi_regexec(pattern," "); /* no warning about unused regexec */
			free(pattern);
			info->re = 1;

		} else info->re = 0;

		return true;
	}
	if(c == NDPI_OPT_PROTO || c == NDPI_OPT_INPROGRESS ||
	   c == NDPI_OPT_JA3S  || c == NDPI_OPT_JA3C ||
	   c == NDPI_OPT_TLSFP || c == NDPI_OPT_TLSV) {
		char *np = optarg,*n;
		int num;
		int op;
		while((n = strtok(np,",")) != NULL) {
			num = -1;
			op = 1;
			if(*n == '-') {
				op = 0;
				n++;
			}
			for (i = 0; i < NDPI_NUM_BITS; i++) {
			    if(prot_short_str[i] && !strcasecmp(prot_short_str[i],n)) {
				    num = i;
				    break; 
			    }
			}
			if(num < 0) {
			    if(strcmp(n,"all")) {
				printf("Unknown proto '%s'\n",n);
				return false;
			    }
			    for (i = 1; i < NDPI_NUM_BITS; i++) {
				if(prot_short_str[i] && strncmp(prot_short_str[i],"badproto_",9) && !prot_disabled[i]) {
				    if(op)
					NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags,i);
				     else
					NDPI_DEL_PROTOCOL_FROM_BITMASK(info->flags,i);
				}
			    }
			} else {
			    if(prot_disabled[num]) {
				printf("Disabled proto '%s'\n",n);
				return false;
			    }
			    if(op)
				NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags,num);
			     else
				NDPI_DEL_PROTOCOL_FROM_BITMASK(info->flags,num);
			}
			np = NULL;
		}
		if(c == NDPI_OPT_PROTO) { *flags |= FLAGS_PROTO; info->proto = 1; }
		if(c == NDPI_OPT_JA3S)  { *flags |= FLAGS_JA3S;  info->ja3s = 1; }
		if(c == NDPI_OPT_JA3C)  { *flags |= FLAGS_JA3C;  info->ja3c = 1; }
		if(c == NDPI_OPT_TLSFP) { *flags |= FLAGS_TLSFP; info->tlsfp = 1; }
		if(c == NDPI_OPT_TLSV)  { *flags |= FLAGS_TLSV;  info->tlsv = 1; }
		if(c == NDPI_OPT_INPROGRESS ) { *flags |= FLAGS_INPROGRESS;
						info->inprogress = 1; }
		if(NDPI_BITMASK_IS_EMPTY(info->flags)) {
			info->empty = 1;
			*flags &= ~FLAGS_PROTO;
		} else
			*flags |= FLAGS_HPROTO;

		return true;
	}
	if(c == NDPI_OPT_UNKNOWN) {
		NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags,NDPI_PROTOCOL_UNKNOWN);
		info->proto = 1;
        	*flags |= FLAGS_PROTO | FLAGS_HPROTO;
		return true;
	}
	if(c == NDPI_OPT_ALL) {
		for (i = 1; i < NDPI_NUM_BITS; i++) {
	    	    if(prot_short_str[i] && strncmp(prot_short_str[i],"badproto_",9) && !prot_disabled[i])
			NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags,i);
		}
		info->proto = 1;
        	*flags |= FLAGS_PROTO | FLAGS_HPROTO | FLAGS_ALL;
		return true;
	}
	return false;
}

#ifndef xtables_error
#define xtables_error exit_error
#endif

static void
ndpi_mt_check (unsigned int flags)
{
	int nopt = 0;
	if (!flags)
		xtables_error(PARAMETER_PROBLEM, "xt_ndpi: missing options.");
	
	if (flags & FLAGS_ERR) {
	   if (flags != FLAGS_ERR)
		xtables_error(PARAMETER_PROBLEM, "xt_ndpi: cant use '--error' with other options");
	    else
		return;
	}
	if (flags & FLAGS_UNTRACKED) {
	   if (flags != FLAGS_UNTRACKED)
		xtables_error(PARAMETER_PROBLEM, "xt_ndpi: cant use '--untracked' with other options");
	    else
		return;
	}
	if (flags & FLAGS_HMASTER) {
	   if(flags & (FLAGS_ALL | FLAGS_MPROTO | FLAGS_APROTO))
		 xtables_error(PARAMETER_PROBLEM, "xt_ndpi: cant use '--have-master' with options match-m-proto,match-a-proto,proto");
	      else
		 return;
	}

	if (flags & (FLAGS_APROTO|FLAGS_MPROTO)) {
	    if(!(flags & FLAGS_HPROTO))
		 xtables_error(PARAMETER_PROBLEM, "xt_ndpi: You need to specify at least one protocol");
	}

	if (flags & (FLAGS_PROTO|FLAGS_JA3S|FLAGS_JA3C|FLAGS_TLSFP|FLAGS_TLSV|FLAGS_INPROGRESS)) {
	    if(!(flags & FLAGS_HPROTO))
		 xtables_error(PARAMETER_PROBLEM, "xt_ndpi: You need to specify at least one protocol");
	}
	if(flags & FLAGS_PROTO) nopt++;
	if(flags & FLAGS_JA3S)  nopt++;
	if(flags & FLAGS_JA3C)  nopt++;
	if(flags & FLAGS_TLSFP) nopt++;
	if(flags & FLAGS_TLSV)  nopt++;
	if(flags & FLAGS_INPROGRESS) nopt++;
	if(nopt != 1)
		 xtables_error(PARAMETER_PROBLEM, "xt_ndpi: --proto|--ja3s|--ja3c|--tlsfp|--tlsv|--inprogress %x %d",flags,nopt);
}

static int cmp_pname(const void *p1, const void *p2) {
	const char *a,*b;
	a = *(const char **)p1;
	b = *(const char **)p2;
	if(a && b) {
		return strcmp( a, b);
	}
	if(a)	return -1;
	if(b)	return 1;
	return 0;
}

static int ndpi_print_prot_list(int cond) {
        int i,c,d,l;
	char line[128];
	char *pn[NDPI_NUM_BITS+1];

	bzero((char *)&pn[0],sizeof(pn));

        for (i = 1,d = 0; i < NDPI_NUM_BITS; i++) {
	    if(!prot_short_str[i] || !strncmp(prot_short_str[i],"badproto_",9)) continue;
	    if(prot_disabled[i] != cond) { 
		    d++;
		    continue;
	    }
	    pn[i-1] = prot_short_str[i];
	}
	qsort(&pn[0],NDPI_NUM_BITS,sizeof(pn[0]),cmp_pname);
        for (i = 0,c = 0,l=0; i < NDPI_NUM_BITS; i++) {
	    if(!pn[i]) break;
	    l += snprintf(&line[l],sizeof(line)-1-l,"%-20s ", pn[i]);
	    c++;
	    if(c == 4) {
		    printf("%s\n",line);
		    c = 0; l = 0;
	    }
	}
	if(c > 0) printf("%s\n",line);
	return d;
}

static void
ndpi_mt_help(void)
{
        int d;

	printf( "ndpi match options:\n"
		"  --error            Match error detecting process\n"
		"  --have-master      Match if master protocol detected\n"
		"  --match-master     Match master protocol only\n"
		"  --match-proto      Match protocol only\n"
		"  --host  str        Match server host name\n"
		"  --cert  str        Match SSL server certificate name\n"
		"  --host-or-cert str Match host name or SSL server certificate name\n"
		"                     Use /str/ for regexp match.\n"
		"Special protocol names:\n"
		"  --all              Match any known protocol\n"
		"  --unknown          Match unknown protocol packets\n");
	printf( "Enabled protocols: ( option form '--protoname' or --proto protoname[,protoname...])\n");
	d = ndpi_print_prot_list(0);
	if(!d) return;
	printf( "Disabled protocols:\n");
	ndpi_print_prot_list(1);
}

static struct option ndpi_mt_opts[NDPI_OPT_LAST+2];

static struct iptables_match
ndpi_mt4_reg = {
	.version = IPTABLES_VERSION,
	.name = "ndpi",
	.revision = 0,
//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
//	.family = AF_INET,
//#else
//	.family = NFPROTO_UNSPEC,
//#endif
	.size = IPT_ALIGN(sizeof(struct xt_ndpi_mtinfo)),
	.userspacesize = IPT_ALIGN(sizeof(struct xt_ndpi_mtinfo)),
//	.help = ndpi_mt_help,
	.parse = ndpi_mt4_parse,
	.final_check = ndpi_mt_check,
	.print = ndpi_mt4_print,
	.save = ndpi_mt4_save,
	.extra_opts = ndpi_mt_opts,
};
enum {
        O_SET_VALUE=0,
        O_SET_NDPI,
        O_SET_NDPI_M,
        O_SET_NDPI_P,
        O_SET_MARK,
        O_SET_MARK2,
        O_SET_CLSF,
        O_SET_FLOW,
        O_ACCEPT,
        F_SET_VALUE  = 1 << O_SET_VALUE,
        F_SET_NDPI   = 1 << O_SET_NDPI,
        F_SET_NDPI_M = 1 << O_SET_NDPI_M,
        F_SET_NDPI_P = 1 << O_SET_NDPI_P,
        F_SET_MARK  = 1 << O_SET_MARK,
        F_SET_MARK2 = 1 << O_SET_MARK2,
        F_SET_CLSF = 1 << O_SET_CLSF,
        F_SET_FLOW = 1 << O_SET_FLOW,
        F_ACCEPT   = 1 << O_ACCEPT,
        F_ANY         = F_SET_VALUE | F_SET_NDPI | F_SET_NDPI_M |
			F_SET_NDPI_P |F_SET_MARK | F_SET_CLSF |
			F_ACCEPT,
};

static void NDPI_help(void)
{
        printf(
"NDPI target options:\n"
"  --value value/mask                  Set value = (value & ~mask) | value\n"
"  --ndpi-id                           Set value = (value & ~proto_mask) | proto_mark by any proto\n"
"  --ndpi-id-p                         Set value = (value & ~proto_mask) | proto_mark by proto\n"
"  --ndpi-id-m                         Set value = (value & ~proto_mask) | proto_mark by master protocol\n"
"  --set-mark                          Set nfmark = value\n"
"  --set-clsf                          Set priority = value\n"
"  --flow-info                         Save flow info\n"
"  --accept                            -j ACCEPT\n"
);
}

static struct option NDPI_opts[] = {
        {.name = "value",     .val = O_SET_VALUE,  .has_arg = 1, .flag = 0},
        {.name = "ndpi-id",   .val = O_SET_NDPI,   .has_arg = 0, .flag = 0},
        {.name = "ndpi-id-m", .val = O_SET_NDPI_M, .has_arg = 0, .flag = 0},
        {.name = "ndpi-id-p", .val = O_SET_NDPI_P, .has_arg = 0, .flag = 0},
        {.name = "set-mark",  .val = O_SET_MARK,   .has_arg = 0, .flag = 0},
        {.name = "set-mark2", .val = O_SET_MARK2,  .has_arg = 0, .flag = 0},
        {.name = "set-clsf",  .val = O_SET_CLSF,   .has_arg = 0, .flag = 0},
        {.name = "flow-info", .val = O_SET_FLOW,   .has_arg = 0, .flag = 0},
        {.name = "accept",    .val = O_ACCEPT,     .has_arg = 0, .flag = 0},
        { 0 },
};
static int NDPI_parse_v0(int c, char **argv, int invert, unsigned int *flags,
	 const struct ipt_entry *entry,
	 struct ipt_entry_target **target)
{
        struct xt_ndpi_tginfo *markinfo = (struct xt_ndpi_tginfo *)(*target)->data;
	char *end;
	unsigned long mark = 0;
	unsigned long mask = ~0U;
	unsigned long val;

        switch (c) {
        case O_SET_VALUE:

	mark = strtoul(optarg, &end, 0);
	if (end == optarg)
		exit_error(PARAMETER_PROBLEM, "Bad MARK value (mask) `%s' %ld", optarg,mark);
		
	if (*end == '/') {
	    mask = strtoul(end+1,&end,0);
	    }
	if (*end != '\0')
		exit_error(PARAMETER_PROBLEM, "Bad MARK value (garbage at end) `%s'", optarg);



                markinfo->mark = mark;
                markinfo->mask = ~mask;
                break;
	case O_SET_NDPI:
		markinfo->any_proto_id = 1;
		markinfo->m_proto_id = 0;
		markinfo->p_proto_id = 0;
		break;
	case O_SET_NDPI_M:
		markinfo->m_proto_id = 1;
		if(markinfo->p_proto_id) {
			markinfo->m_proto_id = 0;
			markinfo->p_proto_id = 0;
			markinfo->any_proto_id = 1;
		}
		break;
	case O_SET_NDPI_P:
		markinfo->p_proto_id = 1;
		if(markinfo->m_proto_id) {
			markinfo->m_proto_id = 0;
			markinfo->p_proto_id = 0;
			markinfo->any_proto_id = 1;
		}
		break;
	case O_SET_MARK:
		markinfo->t_mark = 1;
		break;
	case O_SET_MARK2:
		markinfo->t_mark2 = 1;
		break;
	case O_SET_CLSF:
		markinfo->t_clsf = 1;
		break;
	case O_SET_FLOW:
		markinfo->flow_yes = 1;
		break;
	case O_ACCEPT:
		markinfo->t_accept = 1;
		break;
        default:
                exit_error(PARAMETER_PROBLEM,
                           "NDPI target: unknown --%s",
                           optarg);
        }
    return 1;
}

static void NDPI_print_v0(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
        const struct xt_ndpi_tginfo *info =
                (const struct xt_ndpi_tginfo *)target->data;
char buf[128];
int l;
        l = snprintf(buf,sizeof(buf)-1," NDPI");
	if(info->flow_yes)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " NETFLOW");
	if(info->t_mark2)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " set MARK2 ");
	  else
	    if(info->t_mark)
		l += snprintf(&buf[l],sizeof(buf)-l-1, " set MARK ");
	if(info->t_clsf)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " set CLSF ");
	if(info->mask || info->mark) {
	     if(info->mask)
		l += snprintf(&buf[l],sizeof(buf)-l-1," and 0x%x or 0x%x",
				info->mask,info->mark);
	     else
	        l += snprintf(&buf[l],sizeof(buf)-l-1," set 0x%x", info->mark);
	}
	if(info->any_proto_id)
	     l += snprintf(&buf[l],sizeof(buf)-l-1,
			     " by any ndpi-id");
	else {
		if(info->m_proto_id)
		     l += snprintf(&buf[l],sizeof(buf)-l-1,
			     " by master ndpi-id");
		if(info->p_proto_id)
		     l += snprintf(&buf[l],sizeof(buf)-l-1,
			     " by proto ndpi-id");
	}
	if(info->t_accept)
	     l += snprintf(&buf[l],sizeof(buf)-l-1," ACCEPT");
	printf(buf);
}

static void NDPI_save_v0(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
        const struct xt_ndpi_tginfo *info =
                (const struct xt_ndpi_tginfo *)target->data;
	char buf[128];
	int l = 0;
	if(info->mask || info->mark) {
	     l += snprintf(&buf[l],sizeof(buf)-l-1," --value 0x%x", info->mark);
	     if(info->mask)
		l += snprintf(&buf[l],sizeof(buf)-l-1,"/0x%x",~info->mask);
	}
	if(info->any_proto_id)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --ndpi-id");
	else {
		if(info->m_proto_id)
		     l += snprintf(&buf[l],sizeof(buf)-l-1, " --ndpi-id-m");
		if(info->p_proto_id)
		     l += snprintf(&buf[l],sizeof(buf)-l-1, " --ndpi-id-p");
	}
	if(info->t_mark2)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --set-mark2");
	  else
	    if(info->t_mark)
		l += snprintf(&buf[l],sizeof(buf)-l-1, " --set-mark");
	if(info->t_clsf)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --set-clsf");
	if(info->flow_yes)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --flow-info");
	if(info->t_accept)
	     l += snprintf(&buf[l],sizeof(buf)-l-1," --accept");
	printf(buf);

}

static void NDPI_check(unsigned int flags)
{
        if (!(flags & (F_SET_MARK|F_SET_CLSF|F_SET_FLOW))) 
                exit_error(PARAMETER_PROBLEM,
                           "NDPI target: Parameter --set-mark, --set-clsf or --flow-info"
                           " is required");
	if(flags & (F_SET_MARK|F_SET_CLSF))
           if (!(flags & (F_SET_VALUE|F_SET_NDPI|F_SET_NDPI_M|F_SET_NDPI_P)))
                  exit_error(PARAMETER_PROBLEM,
                           "NDPI target: Parameter --value or --ndpi-id[-[mp]]"
                           " is required");
}

static struct iptables_target ndpi_tg_reg =
        {
//                .family        = NFPROTO_UNSPEC,
                .name          = "NDPI",
                .version       = IPTABLES_VERSION,
                .revision      = 0,
                .size          = IPT_ALIGN(sizeof(struct xt_ndpi_tginfo)),
//		.userspacesize = offsetof(struct xt_ndpi_mtinfo, reg_data),
		.userspacesize = IPT_ALIGN(sizeof(struct xt_ndpi_mtinfo)),
//                .userspacesize = IPT_ALIGN(sizeof(struct xt_ndpi_tginfo)),
//                .help          = NDPI_help,
                .print         = NDPI_print_v0,
                .save          = NDPI_save_v0,
                .parse         = NDPI_parse_v0,
                .final_check   = NDPI_check,
                .extra_opts    = NDPI_opts,
};

void _init(void)
{
        int i;
	char buf[128],*c,pname[32],mark[32];
	uint32_t index;
	pname[0] = '\0';
	index = 0;

/*        for (i = 0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++){
                ndpi_mt_opts[i].name = prot_short_str[i];
                ndpi_mt_opts[i].flag = NULL;
		ndpi_mt_opts[i].has_arg = 0;
                ndpi_mt_opts[i].val = i;
        }*/

#define MT_OPT(np,protoname,nargs) { i=(np); \
	ndpi_mt_opts[i].name = protoname; ndpi_mt_opts[i].flag = NULL; \
	ndpi_mt_opts[i].has_arg = nargs;  ndpi_mt_opts[i].val = i; }

	MT_OPT(NDPI_OPT_UNKNOWN,"unknown",0)
	MT_OPT(NDPI_OPT_ALL,"all",0)
	MT_OPT(NDPI_OPT_ERROR,"error",0)
	MT_OPT(NDPI_OPT_PROTO,"proto",1)
	MT_OPT(NDPI_OPT_MPROTO,"match-m-proto",0)
	MT_OPT(NDPI_OPT_APROTO,"match-a-proto",0)
	MT_OPT(NDPI_OPT_HMASTER,"have-master",0)
	MT_OPT(NDPI_OPT_HOST,"host",1)
	MT_OPT(NDPI_OPT_INPROGRESS,"inprogress",1)
	MT_OPT(NDPI_OPT_JA3S,"ja3s",1)
	MT_OPT(NDPI_OPT_JA3C,"ja3c",1)
	MT_OPT(NDPI_OPT_TLSFP,"tlsfp",1)
	MT_OPT(NDPI_OPT_TLSV,"tlsv",1)
	MT_OPT(NDPI_OPT_UNTRACKED,"untracked",0)
	MT_OPT(NDPI_OPT_CLEVEL,"clevel",1)
	MT_OPT(NDPI_OPT_LAST,NULL,0)

	register_match(&ndpi_mt4_reg);
	register_target(&ndpi_tg_reg);
}
