/*
 * defaults.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#define FROM_NVRAM
#include <epivers.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <typedefs.h>
#include <wlioctl.h>
#include <stdio.h>
#include <ezc.h>

#include <code_pattern.h>
#include <cy_conf.h>
#include <revision.h>

#define XSTR(s) STR(s)
#define STR(s) #s

#ifdef HAVE_SKYTEL
#define HAVE_POWERNOC_WORT54G 1
#define HAVE_POWERNOC 1
#endif

#ifdef STORE_DEFAULTS

struct nvram_param srouter_defaults[] = {
	// {"default_init","1",0},
	{ "nvram_ver", "10" },
#ifdef HAVE_GGEW
	{ "router_style", "blue" },
#elif HAVE_OCTAGON
	{ "router_style", "octagon" },
#elif HAVE_CORENET
	{ "router_style", "corenet" },
#elif HAVE_XIOCOM
	{ "router_style", "xiocom" },
#elif HAVE_IMMERSIVE
	{ "router_style", "immersive" },
#elif HAVE_HDWIFI
	{ "router_style", "hdwifi" },
#elif HAVE_DDLAN
	{ "router_style", "blue" },
#elif HAVE_CESAR
	{ "router_style", "cesar" },
#elif HAVE_THOM
	{ "router_style", "orange" },
#elif HAVE_SPOTPASS
	{ "router_style", "nintendo" },
#elif HAVE_BUFFALO
	{ "router_style", "buffalo" },
#elif HAVE_WIKINGS
	{ "router_style", "wikings" },
#elif HAVE_SANSFIL
	{ "router_style", "sansfil" },
#elif HAVE_ESPOD
	{ "router_style", "espod" },
#elif HAVE_NEXTMEDIA
	{ "router_style", "nextmedia" },
#elif HAVE_3COM
	{ "router_style", "3com" },
#elif HAVE_SPUTNIK
	{ "router_style", "red" },
#elif HAVE_ERC
	{ "router_style", "erc-machinery" },
#elif HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
	{ "router_style", "orange" },
#else
	{ "router_style", "maksat" },
#endif
#elif HAVE_ALFA_BRANDING
	{ "router_style", "xirian" },
#elif HAVE_CARLSONWIRELESS
	{ "router_style", "carlson" },
#elif HAVE_IPR
	{ "router_style", "ipr" },
#elif HAVE_ENEO
	{ "router_style", "eneo" },
#elif HAVE_ONNET
	{ "router_style", "onnet" },
#elif HAVE_RAYTRONIK
	{ "router_style", "raytronik" },
#elif HAVE_KORENRON
	{ "router_style", "korenron" },
#elif HAVE_TESTEM
	{ "router_style", "testem" },
#elif HAVE_HOBBIT
	{ "router_style", "hobbit" },
#elif HAVE_UNFY
	{ "router_style", "unfy" },
#elif HAVE_IDEXX
	{ "router_style", "idexx" },
#elif HAVE_ANTAIRA
	{ "router_style", "red" },
#else
	{ "router_style", "elegant" },
#endif
	/*
	 * OS parameters 
	 */
	{ "wait_time", "5" },
	/*
	 * Miscellaneous parameters 
	 */
#ifdef BUFFALO_JP
	{ "time_zone", "Asia/Tokyo" },
#elif HAVE_AXTEL
	{ "time_zone", "America/Mexico City" },
#elif HAVE_HOBBIT
	{ "time_zone", "Europe/Brussels" },
#elif HAVE_ONNET
	{ "time_zone", "Asia/Dubai" },
#elif HAVE_ANTAIRA
	{ "time_zone", "America/Los_Angeles" },
#elif HAVE_CARLSONWIRELESS
	{ "time_zone", "Etc/Zulu" },
#else
	{ "time_zone", "Europe/Berlin" },
#endif
#ifdef HAVE_SKYTRON
	{ "ntp_server", "ntp0.fau.de" },	/* NTP server *//* Modify */
#elif HAVE_DDLAN
	{ "ntp_server", "10.0.0.1" },	/* NTP server *//* Modify */
#elif HAVE_CARLSONWIRELESS
	{ "ntp_server", "pool.ntp.org" },	/* NTP server *//* Modify */
#else
	{ "ntp_server", "" },	/* NTP server *//* Modify */
#endif
	{ "refresh_time", "3" },	/* GUI Auto-Refresh interval */
	{ "auth_limit", "180" },	/* GUI auth limit ask if no config change in 180s ask for auth again */
	{ "log_level", "0" },	/* Bitmask 0:off 1:denied 2:accepted */

#ifdef HAVE_UPNP
#ifdef HAVE_SKYTRON
	{ "upnp_enable", "1" },	/* 0:Disable 1:Enable */
#else
	{ "upnp_enable", "0" },	/* 0:Disable 1:Enable */
#endif
	{ "upnp_config", "1" },	/* Allow Users to Configure. 0:Disable 1:Enable */
	{ "upnp_internet_dis", "0" },	/* Allow Users to Disable Internet Access. 0:Disable 1:Enable */
	{ "upnp_ssdp_interval", "60" },	/* SSDP interval */
	{ "upnp_max_age", "180" },	/* MAX age time */
	{ "upnpcas", "0" },	/* UPnP clear at startup */
#endif

	{ "ezc_enable", "1" },	/* Enable EZConfig updates */
	{ "ezc_version", EZC_VERSION_STR },	/* EZConfig version */
	{ "is_default", "1" },	/* is it default setting: 1:yes 0:no */
	{ "os_server", "" },	/* URL for getting upgrades */
	{ "stats_server", "" },	/* URL for posting stats */
	{ "console_loglevel", "7" },	/* Kernel panics only */

	/*
	 * Big switches 
	 */
	// {"router_disable", "0"}, /* lan_proto=static lan_stp=0
	// wan_proto=disabled */
	{ "fw_disable", "0" },	/* Disable firewall (allow new connections from the WAN) */

	/*
	 * TCP/IP parameters 
	 */
	{ "log_enable", "0" },	/* 0:Disable 1:Eanble *//* Add */
	{ "log_ipaddr", "0" },	/* syslog recipient */

	/*
	 * LAN H/W parameters 
	 */
	{ "lan_ifname", "" },	/* LAN interface name */
	{ "lan_ifnames", "" },	/* Enslaved LAN interfaces */
	{ "lan_hwnames", "" },	/* LAN driver names (e.g. et0) */
	{ "lan_hwaddr", "" },	/* LAN interface MAC address */
//KONG needs to be modified for marvel
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
	{ "wl0_ifname", "wlan0" },	/* LAN interface MAC address */
#else
	{ "wl0_ifname", "eth1" },	/* LAN interface MAC address */
#endif
	/*
	 * LAN TCP/IP parameters 
	 */
#ifdef HAVE_POWERNOC_WOAP54G
	{ "lan_proto", "static" },	/* [static|dhcp] */
#elif HAVE_CARLSONWIRELESS
	{ "lan_proto", "static" },	/* [static|dhcp] */
#elif HAVE_IPR
	{ "lan_proto", "static" },	/* [static|dhcp] */
#elif HAVE_UNFY
	{ "lan_proto", "static" },	/* [static|dhcp] */
#else
	{ "lan_proto", "dhcp" },	/* [static|dhcp] */
#endif
#ifdef HAVE_SKYTRON
	{ "lan_ipaddr", "192.168.0.1" },	/* LAN IP address */
#elif HAVE_DDLAN
#ifdef HAVE_NS5
	{ "wlan0_regdomain", "GERMANY_BFWA" },	/* LAN IP address */
	{ "wlan0_channelbw", "10" },	/* LAN IP address */
#else
	{ "wlan0_regdomain", "GERMANY" },
	{ "wlan1_regdomain", "GERMANY" },
#endif
	{ "lan_ipaddr", "192.168.1.1" },	/* LAN IP address */
#elif HAVE_IDEXX
	{ "lan_ipaddr", "192.168.222.1" },	/* LAN ip address */
	{ "wlan0_regdomain", "UNITED_STATES" },
	{ "wlan1_regdomain", "UNITED_STATES" },
#elif HAVE_ENEO
	{ "wlan0_regdomain", "GERMANY" },
	{ "nocountrysel", "1" },
	{ "wlan0_doth", "1" },
#elif HAVE_BUFFALO
#ifdef BUFFALO_EU
	{ "wlan0_regdomain", "GERMANY" },	/* LAN IP address */
#endif
	{ "lan_ipaddr", "192.168.11.1" },	/* LAN IP address */
	{ "dhcp_start", "192.168.11.2" },	/* DHCP Start IP */
	{ "dhcp_num", "64" },	/* DHCP Start IP */
#elif HAVE_GGEW
#if defined(HAVE_NS5) || defined(HAVE_EOC5610)
	{ "wlan0_regdomain", "GERMANY_BFWA" },	/* LAN IP address */
#elif defined(HAVE_NS2) || defined(HAVE_EOC2610)
	{ "wlan0_regdomain", "GERMANY" },	/* LAN IP address */
#elif defined(HAVE_WHRHPGN)
	{ "wlan0_regdomain", "GERMANY" },	/* LAN IP address */
#endif
	{ "lan_ipaddr", "192.168.1.1" },	/* LAN IP address */
#elif HAVE_CORENET
	{ "wlan0_regdomain", "UNITED_KINGDOM" },	/* LAN IP address */
	{ "lan_ipaddr", "192.168.1.1" },	/* LAN IP address */
#elif HAVE_NEWMEDIA
	{ "lan_ipaddr", "172.31.28.3" },	/* LAN IP address */
#elif HAVE_FON
	{ "lan_ipaddr", "192.168.10.1" },	/* LAN IP address */
#elif HAVE_34TELECOM
	{ "lan_ipaddr", "192.168.1.4" },	/* LAN IP address */
#elif HAVE_SPUTNIK
	{ "lan_ipaddr", "192.168.180.1" },	/* LAN IP address */
#elif HAVE_ERC
	{ "lan_ipaddr", "10.195.0.1" },	/* LAN IP address */
#elif HAVE_BKM
	{ "lan_ipaddr", "192.168.42.1" },	/* LAN IP address */
	{ "wlan0_regdomain", "GERMANY" },	/* LAN IP address */
	{ "wlan1_regdomain", "GERMANY" },	/* LAN IP address */
	{ "wlan2_regdomain", "GERMANY" },	/* LAN IP address */
	{ "wlan3_regdomain", "GERMANY" },	/* LAN IP address */
#elif HAVE_CARLSONWIRELESS
#ifdef HAVE_LAGUNA
	{ "lan_ipaddr", "192.168.3.20" },	/* LAN ip address */
#else
	{ "lan_ipaddr", "192.168.2.20" },	/* LAN ip address */
#endif
	{ "wlan0_regdomain", "UNITED_STATES_(PUBLIC_SAFETY)" },	/* wlan0 regulatory domain */
	{ "wlan1_regdomain", "UNITED_STATES_(PUBLIC_SAFETY)" },	/* wlan0 regulatory domain */
	{ "wlan2_regdomain", "UNITED_STATES_(PUBLIC_SAFETY)" },	/* wlan0 regulatory domain */
#elif HAVE_IPR
	{ "lan_ipaddr", "192.168.14.14" },	/* LAN ip address */
#elif HAVE_KORENRON
	{ "lan_ipaddr", "10.0.0.1" },	/* LAN ip address */
	{ "lan_gateway", "10.0.0.254" },	/* Gateway */
#elif HAVE_HOBBIT
	{ "lan_ipaddr", "192.168.50.254" },	/* LAN ip address */
	{ "lan_gateway", "192.168.50.254" },	/* Gateway */
#elif HAVE_AXTEL
	{ "lan_ipaddr", "192.168.11.1" },	/* LAN IP address */
	{ "wlan0_regdomain", "MEXICO" },	/* LAN IP address */
#elif HAVE_RAYTRONIK
	{ "lan_ipaddr", "10.0.0.1" },	/* LAN IP address */
#elif HAVE_NDTRADE
	{ "lan_ipaddr", "192.168.100.1" },	/* LAN IP address */
#elif HAVE_ONNET
#ifdef HAVE_ONNET_STATION
	{ "lan_ipaddr", "192.168.1.2" },
#else
	{ "lan_ipaddr", "192.168.1.1" },
#endif
	{ "lan_proto", "static" },
#else
	{ "lan_ipaddr", "192.168.1.1" },	/* LAN IP address */
#endif
	{ "lan_netmask", "255.255.255.0" },	/* LAN netmask */
	{ "lan_gateway", "0.0.0.0" },	/* LAN gateway */
	{ "sv_localdns", "0.0.0.0" },	/* Local DNS */
#ifdef HAVE_SKYTRON
	{ "lan_stp", "0" },	/* LAN spanning tree protocol */
#elif HAVE_MAKSAT
	{ "lan_stp", "0" },	/* LAN spanning tree protocol */
#else
	{ "lan_stp", "0" },	/* LAN spanning tree protocol */
#endif
	{ "lan_wins", "" },	/* x.x.x.x x.x.x.x ... */
#ifdef HAVE_SKYTRON
	{ "lan_domain", "local" },	/* LAN domain name */
#else
	{ "lan_domain", "" },	/* LAN domain name */
#endif
	{ "lan_lease", "86400" },	/* LAN lease time in seconds */
	// {"lan_lease", "1440"}, /* LAN lease time in seconds */

	{ "sfe", "1" },

	/*
	 * WAN H/W parameters 
	 */
	{ "wan_dial", "0" },
	{ "wan_ifname", "" },	/* WAN interface name */
	{ "wan_ifname2", "" },	/* WAN interface name (clone) */
	{ "wan_ifnames", "" },	/* WAN interface names */
	{ "wan_default", "" },	/* WAN interface names */
	{ "wan_hwname", "" },	/* WAN driver name (e.g. et1) */
	{ "wan_hwaddr", "" },	/* WAN interface MAC address */

	/*
	 * WAN TCP/IP parameters 
	 */
	{ "wan_vdsl", "0" },
	{ "dtag_vlan8", "0" },
	{ "dtag_bng", "0" },
#ifdef HAVE_SKYTRON
	{ "wan_proto", "static" },	/* [static|dhcp|pppoe|disabled] */
	{ "wan_ipaddr", "10.254.254.254" },	/* WAN IP address */
	{ "wan_netmask", "255.0.0.0" },	/* WAN netmask */
	{ "wan_gateway", "10.0.0.1" },	/* WAN gateway */
	{ "wan_dns", "213.146.232.2 213.146.230.2" },	/* x.x.x.x x.x.x.x
							 * ... */
#elif HAVE_TRIMAX
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */

	{ "wan_ipaddr", "0.0.0.0" },	/* WAN IP address */
	{ "wan_netmask", "0.0.0.0" },	/* WAN netmask */
	{ "wan_gateway", "0.0.0.0" },	/* WAN gateway */
	{ "wan_dns", "" },	/* x.x.x.x x.x.x.x ... */
#elif HAVE_DDLAN
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */

	{ "wan_ipaddr", "0.0.0.0" },	/* WAN IP address */
	{ "wan_netmask", "0.0.0.0" },	/* WAN netmask */
	{ "wan_gateway", "0.0.0.0" },	/* WAN gateway */
	{ "wan_dns", "" },	/* x.x.x.x x.x.x.x ... */
#elif HAVE_SANSFIL
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_ANTAIRA
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif defined(HAVE_GGEW) && defined(HAVE_NS5)
	{ "wan_proto", "pptp" },	/* [static|dhcp|pppoe|disabled] */
#elif defined(HAVE_GGEW) && defined(HAVE_EOC5610)
	{ "wan_proto", "pptp" },	/* [static|dhcp|pppoe|disabled] */
#elif defined(HAVE_GGEW) && defined(HAVE_NS2)
	{ "wan_proto", "pptp" },	/* [static|dhcp|pppoe|disabled] */
#elif defined(HAVE_GGEW) && defined(HAVE_EOC2610)
	{ "wan_proto", "pptp" },	/* [static|dhcp|pppoe|disabled] */
#elif defined(HAVE_GGEW)
	{ "wan_proto", "pptp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_X86
#ifdef HAVE_GW700
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#else
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#endif
#elif HAVE_JWAP606
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WA901v1
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WA901
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WA901V3
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WZRG450
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WR710
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_DIR632
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_E380AC
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_AP120C
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_DAP3662
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_DAP2230
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_DIR862
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_MMS344
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WLAEAG300N
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_RS
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_LAGUNA
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_EROUTER
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_NEWPORT
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_IPQ6018
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_VENTANA
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_MAGICBOX
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_RB600
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_TW6600
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_XSCALE
#ifdef HAVE_SPUTNIK
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_XIOCOM
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_CARLSONWIRELESS
	{ "wan_proto", "disabled", 0 }	/* [static|dhcp|pppoe|disabled] */
#else
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#endif
#elif HAVE_HAMEA15
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_EAP9550
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WNR2000
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WHRHPGN
#ifdef HAVE_ONNET
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#else
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#endif
#elif HAVE_DIR615E
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_DIR400
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WRT54G2
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_RTG32
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_DIR300
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WR741
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_FONERA
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_NS2
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_LS5
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_PICO2
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_PICO2HP
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_PICO5
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_LS2
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_CA8
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_RS
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_AP83
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_AP96
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_AP94
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_JA76PF
#if defined(HAVE_MAKSAT) || defined(HAVE_ESPOD)
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#else
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#endif
#elif HAVE_JJAP93
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_JJAP005
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_JJAP501
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_AC722
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_AC622
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_JWAP003
#ifdef HAVE_MAKSAT
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#else
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#endif
#elif HAVE_LSX
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_DANUBE
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_STORM
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_OPENRISC
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_WP54G
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_NP28G
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_ECB9750
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_TECHNAXX3G
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#elif HAVE_ADM5120
	{ "wan_proto", "disabled" },	/* [static|dhcp|pppoe|disabled] */
#else
	{ "wan_proto", "dhcp" },	/* [static|dhcp|pppoe|disabled] */

#endif
	{ "ignore_wan_dns", "0" },
	{ "wan_ipaddr", "0.0.0.0" },	/* WAN IP address */
	{ "wan_netmask", "0.0.0.0" },	/* WAN netmask */
	{ "wan_gateway", "0.0.0.0" },	/* WAN gateway */
	{ "pptp_wan_gateway", "0.0.0.0" },
	{ "l2tp_wan_gateway", "0.0.0.0" },
#ifdef HAVE_IDEXX
	{ "wan_dns", "208.67.222.222 8.8.8.8 209.244.0.3" },	/* x.x.x.x x.x.x.x ... */
#else
	{ "wan_dns", "" },	/* x.x.x.x x.x.x.x ... */
#endif

	{ "wan_wins", "0.0.0.0" },	/* x.x.x.x x.x.x.x ... */
	{ "wan_dualaccess", "0" },	/* dual-access mode, eg for russia */
#ifdef HAVE_SKYTRON
	{ "wan_hostname", "skymax254b" },	/* WAN hostname */
	{ "wan_domain", "local" },	/* WAN domain name */
#else
	{ "wan_hostname", "" },	/* WAN hostname */
	{ "wan_domain", "" },	/* WAN domain name */
#endif
	{ "wan_lease", "86400" },	/* WAN lease time in seconds */
	{ "static_route", "" },	/* Static routes
				 * (ipaddr:netmask:gateway:metric:ifname ...) 
				 */
	{ "static_route_name", "" },	/* Static routes name ($NAME:name) */

	{ "ses_enable", "1" },	/* enable ses */
	{ "ses_event", "2" },	/* initial ses event */
	{ "ses_button", "0" },	/* Affect custom actions to SES Button - 0-3: 
				 * nothing, reboot, enable/disable wireless,
				 * custom script */
	{ "ses_script", "" },	/* Custom script to launch when pushing SES
				 * Button */

	{ "wan_primary", "1" },	/* Primary wan connection */
	{ "wan_unit", "0" },	/* Last configured connection */

	/*
	 * Filters 
	 */
	{ "filter_maclist", "" },	/* xx:xx:xx:xx:xx:xx ... */
	{ "filter_macmode", "deny" },	/* "allow" only, "deny" only, or
					 * "disabled" (allow all) */
	{ "filter_client0", "" },	/* [lan_ipaddr0-lan_ipaddr1|*]:lan_port0-lan_port1,proto,enable,day_start-day_end,sec_start-sec_end,desc 
					 */

	{ "filter_port", "" },	/* [lan_ipaddr|*]:lan_port0-lan_port1 */
	{ "filter_rule1", "" },	/* $STAT: $NAME:$$ */
	{ "filter_rule2", "" },	/* $STAT: $NAME:$$ */
	{ "filter_rule3", "" },	/* $STAT: $NAME:$$ */
	{ "filter_rule4", "" },	/* $STAT: $NAME:$$ */
	{ "filter_rule5", "" },	/* $STAT: $NAME:$$ */
	{ "filter_rule6", "" },	/* $STAT: $NAME:$$ */
	{ "filter_rule7", "" },	/* $STAT: $NAME:$$ */
	{ "filter_rule8", "" },	/* $STAT: $NAME:$$ */
	{ "filter_rule9", "" },	/* $STAT: $NAME:$$ */
	{ "filter_rule10", "" },	/* $STAT: $NAME:$$ */
	{ "filter_tod1", "" },	/* Filter Time of the day */
	{ "filter_tod2", "" },	/* Filter Time of the day */
	{ "filter_tod3", "" },	/* Filter Time of the day */
	{ "filter_tod4", "" },	/* Filter Time of the day */
	{ "filter_tod5", "" },	/* Filter Time of the day */
	{ "filter_tod6", "" },	/* Filter Time of the day */
	{ "filter_tod7", "" },	/* Filter Time of the day */
	{ "filter_tod8", "" },	/* Filter Time of the day */
	{ "filter_tod9", "" },	/* Filter Time of the day */
	{ "filter_tod10", "" },	/* Filter Time of the day */
	{ "filter_tod_buf1", "" },	/* Filter Time of the day */
	{ "filter_tod_buf2", "" },	/* Filter Time of the day */
	{ "filter_tod_buf3", "" },	/* Filter Time of the day */
	{ "filter_tod_buf4", "" },	/* Filter Time of the day */
	{ "filter_tod_buf5", "" },	/* Filter Time of the day */
	{ "filter_tod_buf6", "" },	/* Filter Time of the day */
	{ "filter_tod_buf7", "" },	/* Filter Time of the day */
	{ "filter_tod_buf8", "" },	/* Filter Time of the day */
	{ "filter_tod_buf9", "" },	/* Filter Time of the day */
	{ "filter_tod_buf10", "" },	/* Filter Time of the day */
	{ "filter_ip_grp1", "" },	/* Filter IP group 1 */
	{ "filter_ip_grp2", "" },	/* Filter IP group 1 */
	{ "filter_ip_grp3", "" },	/* Filter IP group 1 */
	{ "filter_ip_grp4", "" },	/* Filter IP group 1 */
	{ "filter_ip_grp5", "" },	/* Filter IP group 1 */
	{ "filter_ip_grp6", "" },	/* Filter IP group 1 */
	{ "filter_ip_grp7", "" },	/* Filter IP group 1 */
	{ "filter_ip_grp8", "" },	/* Filter IP group 1 */
	{ "filter_ip_grp9", "" },	/* Filter IP group 1 */
	{ "filter_ip_grp10", "" },	/* Filter IP group 1 */
	{ "filter_mac_grp1", "" },	/* Filter MAC group 1 */
	{ "filter_mac_grp2", "" },	/* Filter MAC group 1 */
	{ "filter_mac_grp3", "" },	/* Filter MAC group 1 */
	{ "filter_mac_grp4", "" },	/* Filter MAC group 1 */
	{ "filter_mac_grp5", "" },	/* Filter MAC group 1 */
	{ "filter_mac_grp6", "" },	/* Filter MAC group 1 */
	{ "filter_mac_grp7", "" },	/* Filter MAC group 1 */
	{ "filter_mac_grp8", "" },	/* Filter MAC group 1 */
	{ "filter_mac_grp9", "" },	/* Filter MAC group 1 */
	{ "filter_mac_grp10", "" },	/* Filter MAC group 1 */
	{ "filter_web_host1", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host2", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host3", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host4", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host5", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host6", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host7", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host8", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host9", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host10", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host11", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host12", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host13", "" },	/* Website Blocking by URL Address */
	{ "filter_web_host14", "" },	/* Website Blocking by URL Address */
	{ "filter_web_url1", "" },	/* Website Blocking by keyword */
	{ "filter_web_url2", "" },	/* Website Blocking by keyword */
	{ "filter_web_url3", "" },	/* Website Blocking by keyword */
	{ "filter_web_url4", "" },	/* Website Blocking by keyword */
	{ "filter_web_url5", "" },	/* Website Blocking by keyword */
	{ "filter_web_url6", "" },	/* Website Blocking by keyword */
	{ "filter_web_url7", "" },	/* Website Blocking by keyword */
	{ "filter_web_url8", "" },	/* Website Blocking by keyword */
	{ "filter_web_url9", "" },	/* Website Blocking by keyword */
	{ "filter_web_url10", "" },	/* Website Blocking by keyword */
	{ "filter_web_url11", "" },	/* Website Blocking by keyword */
	{ "filter_web_url12", "" },	/* Website Blocking by keyword */
	{ "filter_web_url13", "" },	/* Website Blocking by keyword */
	{ "filter_web_url14", "" },	/* Website Blocking by keyword */
	{ "filter_web_url15", "" },	/* Website Blocking by keyword */
	{ "filter_port_grp1", "" },	/* Blocked Services */
	{ "filter_port_grp2", "" },	/* Blocked Services */
	{ "filter_port_grp3", "" },	/* Blocked Services */
	{ "filter_port_grp4", "" },	/* Blocked Services */
	{ "filter_port_grp5", "" },	/* Blocked Services */
	{ "filter_port_grp6", "" },	/* Blocked Services */
	{ "filter_port_grp7", "" },	/* Blocked Services */
	{ "filter_port_grp8", "" },	/* Blocked Services */
	{ "filter_port_grp9", "" },	/* Blocked Services */
	{ "filter_port_grp10", "" },	/* Blocked Services */
	{ "filter_dport_grp1", "" },	/* Blocked Services */
	{ "filter_dport_grp2", "" },	/* Blocked Services */
	{ "filter_dport_grp3", "" },	/* Blocked Services */
	{ "filter_dport_grp4", "" },	/* Blocked Services */
	{ "filter_dport_grp5", "" },	/* Blocked Services */
	{ "filter_dport_grp6", "" },	/* Blocked Services */
	{ "filter_dport_grp7", "" },	/* Blocked Services */
	{ "filter_dport_grp8", "" },	/* Blocked Services */
	{ "filter_dport_grp9", "" },	/* Blocked Services */
	{ "filter_dport_grp10", "" },	/* Blocked Services */

	/*
	 * Services List 
	 */
	{ "filter_services", "" },	/* only user defined filters */
	{ "filter_services_1", "" },

	/*
	 * Port forwards 
	 */
	{ "dmz_enable", "0" },	/* Enable (1) or Disable (0) */
	{ "dmz_ipaddr", "0" },	/* x.x.x.x (equivalent to
				 * 0-60999>dmz_ipaddr:0-60999) */
	{ "autofw_port0", "" },	/* out_proto:out_port,in_proto:in_port0-in_port1>to_port0-to_port1,enable,desc 
				 */

	/*
	 * DHCP server parameters 
	 */
#ifdef HAVE_IDEXX
	{ "dhcp_start", "192.168.1.2" },	/* First assignable DHCP address */
#elif HAVE_CARLSONWIRELESS
#ifdef HAVE_LAGUNA
	{ "dhcp_start", "192.168.2.30" },	/* First assignable DHCP address */
#else
	{ "dhcp_start", "192.168.2.30" },	/* First assignable DHCP address */
#endif
#elif HAVE_NDTRADE
	{ "dhcp_start", "192.168.100.30" },	/* First assignable DHCP address */
#else
	{ "dhcp_start", "192.168.1.64" },	/* First assignable DHCP address */
#endif
	// { "dhcp_end", "150", 0 }, /* Last assignable DHCP address */ /* Remove 
	// 
	// 
	// */
#ifdef HAVE_IDEXX
	{ "dhcp_num", "199" },	/* Number of DHCP Users *//* Add */
#else
	{ "dhcp_num", "190" },	/* Number of DHCP Users *//* Add */
#endif
#ifdef HAVE_SKYTRON
	{ "dhcp_lease", "10" },	/* LAN lease time in minutes */
#elif HAVE_IDEXX
	{ "dhcp_lease", "10080" },	/* LAN lease time in minutes */
#else
	{ "dhcp_lease", "1440" },	/* LAN lease time in minutes */
#endif
	{ "dhcp_domain", "wan" },	/* Use WAN domain name first if available
					 * (wan|lan) */
	{ "dhcp_wins", "wan" },	/* Use WAN WINS first if available (wan|lan) */
	{ "wan_get_dns", "" },	/* DNS IP address which get by dhcpc *//* Add */

	/*
	 * Web server parameters 
	 */
#ifdef HAVE_POWERNOC
	{ "http_username", "bJz7PcC1rCRJQ" },	/* Username */
#elif HAVE_ERC
	{ "http_username", "$1$OIw4f9TB$/dcveO2p0zs7eH0gHgsyw0" },
#elif HAVE_CARLSONWIRELESS
	{ "http_username", "$1$y5qEiLaV$/2cQErs8qxs./J3pl2l2F." },	/* HTTP username) */
#elif defined(HAVE_IAS) || defined(HAVE_AXTEL)
	{ "http_username", "$1$LJZEFe0/$TMujOR/zbGMDwxgb3KP0J." },
#elif HAVE_KORENRON
	{ "http_username", "$1$9thN/f9/$nnZ35gSQvAaV0EPh.WJs8." },	/* HTTP username */
#elif HAVE_IDEXX
	{ "http_username", "$1$IpR13S3g$E1hg4idP4TZmfQeHIX20L/" },	/* Password */
#else
	{ "http_username", DEFAULT_USER },	/* Username */
#endif

#ifdef HAVE_SKYTRON
	{ "skyhttp_username", "bJkMQXH.mZhZo" },	/* Username */
	{ "skyhttp_passwd", "bJkMQXH.mZhZo" },	/* Password */
	{ "http_passwd", "bJe0C3lwF.z0c" },	/* Password */
#elif HAVE_NEWMEDIA
#ifdef HAVE_GGEW
	{ "http_passwd", "bJz7PcC1rCRJQ" },	/* Password */
#elif HAVE_KODATA
	{ "http_passwd", "bJDLObifZlIRQ" },	/* Password */
#else
	{ "http_passwd", "bJxJZz5DYRGxI" },	/* Password */
#endif
#elif defined(HAVE_IAS) || defined(HAVE_AXTEL)
	{ "http_passwd", "$1$LJZEFe0/$yHSTW.W0nkBqSkWfcUnww." },
#elif HAVE_CORENET
	{ "http_passwd", "$1$YwPEyUx/$LLV6oaeof4WDEdpHPEMpA." },	/* Username */
	{ "http_username", "$1$9wWnpX1Q$1fobI1HcfeXewVtWCnhxh." },	/* Password */
#elif HAVE_DDLAN
	{ "http_passwd", "4DC5smu4lEiiQ" },	/* Password */
#elif HAVE_ERC
	{ "http_passwd", "$1$o.4B3QRb$KB7.8AOgnesREpnv8Zhfx1" },
#elif HAVE_BKM
	{ "http_passwd", "$1$sur0onKC$Ltnjj7PBVQtmVTNYPb5XF0" },
#elif HAVE_CARLSONWIRELESS
	{ "http_passwd", "$1$y5qEiLaV$KNvLd5jrLCfYko/e6e7lZ1" },	/* HTTP password) */
#elif HAVE_IPR
	{ "http_passwd", "$1$E1quEPpk$d.nw/cqhR1qsi.ECGT5ed0" },	/* HTTP password) */
#elif HAVE_KORENRON
	{ "http_passwd", "$1$9thN/f9/$nnZ35gSQvAaV0EPh.WJs8." },	/* HTTP password */
#elif HAVE_UNFY
	{ "http_passwd", "$1$HHwZAUaN$Xj8iAs4882IkDVfBzO9GI1" },	/* Password */
#elif HAVE_RAYTRONIK
	{ "http_passwd", "$1$iuIXI3we$ZM4nJ4QVrzGySwLd/9PDF0" },	/* Password */
#else
	{ "http_passwd", DEFAULT_PASS },	/* Password */
#endif

	{ "remote_ip_any", "1" },	/* allowed remote ip */
	{ "remote_ip", "0.0.0.0 0" },	/* allowed remote ip range */
	{ "http_wanport", "8080" },	/* WAN port to listen on */
	{ "http_lanport", "80" },	/* LAN port to listen on */
	{ "https_lanport", "443" },	/* LAN port to listen on */
#if defined(HAVE_IPR) || defined(HAVE_UNFY)
#ifdef HAVE_HTTPS
	{ "https_enable", "1" },	/* HTTPS server enable/disable */
	{ "http_enable", "0" },	/* HTTP server enable/disable */
#else
	{ "http_enable", "1" },	/* HTTP server enable/disable */
#endif
#else
	{ "http_enable", "1" },	/* HTTP server enable/disable */
#ifdef HAVE_HTTPS
	{ "https_enable", "0" },	/* HTTPS server enable/disable */
#endif
#endif
	{ "http_method", "post" },	/* HTTP method */
	/*
	 * PPPoE parameters 
	 */
	{ "pppoe_ifname", "" },	/* PPPoE enslaved interface */
	{ "ppp_username", "" },	/* PPP username */
	{ "ppp_passwd", "" },	/* PPP password */
	{ "ppp_idletime", "5" },	/* Dial on demand max idle time (mins) */
	{ "ppp_keepalive", "0" },	/* Restore link automatically */
	{ "ppp_demand", "0" },	/* Dial on demand */
	{ "ppp_redialperiod", "30" },	/* Redial Period (seconds) */
	{ "ppp_service", "" },	/* PPPoE service name */
	{ "ppp_ac", "" },	/* PPPoE access concentrator name */
	{ "ppp_static", "0" },	/* Enable / Disable Static IP */
	{ "ppp_static_ip", "" },	/* PPPoE Static IP */
	{ "ppp_get_ac", "" },	/* PPPoE Server ac name */
	{ "ppp_get_srv", "" },	/* PPPoE Server service name */
	{ "ppp_compression", "0" },	/* PPPoE compr. */
	{ "ppp_mppe", "" },	/* PPPoE mppe parameters */
	{ "ppp_mlppp", "0" },	/* ML-PPP */

	/*
	 * Wireless parameters 
	 */
	{ "wl0_nbw_cap", "1" },
	{ "wl1_nbw_cap", "1" },
	{ "wl2_nbw_cap", "1" },
	{ "wl0_nbw", "20" },	/* N-BW */
	{ "wl1_nbw", "20" },	/* N-BW */
	{ "wl2_nbw", "20" },	/* N-BW */
	{ "wl0_bw_cap", "255" },	/* N-BW */
	{ "wl1_bw_cap", "255" },	/* N-BW */
	{ "wl2_bw_cap", "255" },	/* N-BW */
	{ "wl0_nctrlsb", "none" },	/* N-CTRL SB */
	{ "wl1_nctrlsb", "none" },	/* N-CTRL SB */
	{ "wl2_nctrlsb", "none" },	/* N-CTRL SB */
	{ "wl0_nband", "0" },	/* N-BAND */
	{ "wl1_nband", "1" },	/* N-BAND */
	{ "wl2_nband", "1" },	/* N-BAND */
	{ "wl0_nmcsidx", "-1" },	/* N-MCS Index - rate */
	{ "wl1_nmcsidx", "-1" },	/* N-MCS Index - rate */
	{ "wl2_nmcsidx", "-1" },	/* N-MCS Index - rate */
	{ "wl0_nmode", "-1" },	/* N-mode */
	{ "wl1_nmode", "-1" },	/* N-mode */
	{ "wl2_nmode", "-1" },	/* N-mode */
	{ "wl0_nreqd", "0" },	/* Require 802.11n support */
	{ "wl1_nreqd", "0" },	/* Require 802.11n support */
	{ "wl2_nreqd", "0" },	/* Require 802.11n support */
	{ "wl0_vlan_prio_mode", "off" },	/* VLAN Priority support */
	{ "wl1_vlan_prio_mode", "off" },	/* VLAN Priority support */
	{ "wl2_vlan_prio_mode", "off" },	/* VLAN Priority support */
	{ "wl0_leddc", "0x640000" },	/* 100% duty cycle for LED on router */
	{ "wl_rxstreams", "0" },	/* 802.11n Rx Streams, 0 is invalid, WLCONF will
					 * change it to a radio appropriate default
					 */
	{ "wl_txstreams", "0" },	/* 802.11n Tx Streams 0, 0 is invalid, WLCONF will
					 * change it to a radio appropriate default
					 */

#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
#ifdef HAVE_UNIWIP
	{ "wlan0_bgscan_mode", "simple" },
	{ "wlan1_bgscan_mode", "simple" },
	{ "wlan2_bgscan_mode", "simple" },
#else
	{ "wlan0_bgscan_mode", "off" },
	{ "wlan1_bgscan_mode", "off" },
	{ "wlan2_bgscan_mode", "off" },
#endif
#endif

#ifdef HAVE_80211AC
	{ "wl0_wmf_bss_enable", "0" },	/* 0= off 1= on */
	{ "wl1_wmf_bss_enable", "0" },
	{ "wl2_wmf_bss_enable", "0" },

	{ "wl0_wmf_ucigmp_query", "0" },	/* Disable Converting IGMP Query to ucast (default) */
	{ "wl0_wmf_mdata_sendup", "0" },	/* Disable Sending Multicast Data to host  (default) */
	{ "wl0_wmf_ucast_upnp", "1" },	/* Disable Converting upnp to ucast (default) */
	{ "wl0_wmf_igmpq_filter", "0" },	/* Disable igmp query filter */

	{ "wl1_wmf_ucigmp_query", "0" },	/* Disable Converting IGMP Query to ucast (default) */
	{ "wl1_wmf_mdata_sendup", "0" },	/* Disable Sending Multicast Data to host  (default) */
	{ "wl1_wmf_ucast_upnp", "1" },	/* Disable Converting upnp to ucast (default) */
	{ "wl1_wmf_igmpq_filter", "0" },	/* Disable igmp query filter */

	{ "wl2_wmf_ucigmp_query", "0" },	/* Disable Converting IGMP Query to ucast (default) */
	{ "wl2_wmf_mdata_sendup", "0" },	/* Disable Sending Multicast Data to host  (default) */
	{ "wl2_wmf_ucast_upnp", "1" },	/* Disable Converting upnp to ucast (default) */
	{ "wl2_wmf_igmpq_filter", "0" },	/* Disable igmp query filter */
	/* Airtime fairness */
	{ "wl0_atf", "0" },	/* 0= off 1= on */
	{ "wl1_atf", "0" },
	{ "wl2_atf", "0" },

	{ "wl0_txbf", "0" },
	{ "wl1_txbf", "0" },
	{ "wl2_txbf", "0" },

	{ "wl0_txbf_imp", "0" },
	{ "wl1_txbf_imp", "0" },
	{ "wl2_txbf_imp", "0" },

	{ "wl0_txbf_bfr_cap", "1" },
	{ "wl0_txbf_bfe_cap", "1" },

	{ "wl1_txbf_bfr_cap", "1" },
	{ "wl1_txbf_bfe_cap", "1" },

	{ "wl2_txbf_bfr_cap", "1" },
	{ "wl2_txbf_bfe_cap", "1" },

	{ "wl0_turbo_qam", "1" },	/* RIFS mode advertisement */
	{ "wl1_turbo_qam", "1" },	/* RIFS mode advertisement */
	{ "wl2_turbo_qam", "1" },	/* RIFS mode advertisement */

	{ "wl0_rxchain_pwrsave_enable", "0" },
	{ "wl1_rxchain_pwrsave_enable", "0" },
	{ "wl2_rxchain_pwrsave_enable", "0" },

	{ "wl0_txchain_pwrsave_enable", "0" },
	{ "wl1_txchain_pwrsave_enable", "0" },
	{ "wl2_txchain_pwrsave_enable", "0" },
	{ "wl0_lbr_aggr_en_mask", "0" },	/* per tid/ac mask disable by default */
	{ "wl0_lbr_aggr_len", "16" },	/* default aggregate len */
	{ "wl0_lbr_aggr_release_timeout", "10" },	/* default release timeout in msec */
	{ "wl1_lbr_aggr_en_mask", "0" },	/* per tid/ac mask disable by default */
	{ "wl1_lbr_aggr_len", "16" },	/* default aggregate len */
	{ "wl1_lbr_aggr_release_timeout", "10" },	/* default release timeout in msec */
	{ "wl2_lbr_aggr_en_mask", "0" },	/* per tid/ac mask disable by default */
	{ "wl2_lbr_aggr_len", "16" },	/* default aggregate len */
	{ "wl2_lbr_aggr_release_timeout", "10" },	/* default release timeout in msec */

#endif
#ifdef HAVE_BCMMODERN
	{ "wl_rifs_advert", "auto" },	/* RIFS mode advertisement */
	{ "wl0_rifs_advert", "auto" },	/* RIFS mode advertisement */
	{ "wl1_rifs_advert", "auto" },	/* RIFS mode advertisement */
	{ "wl2_rifs_advert", "auto" },	/* RIFS mode advertisement */
	{ "wl_stbc_tx", "auto" },	/* Default STBC TX setting */
	{ "wl_stbc_rx", "1" },	/* Default STBC RX setting */
	{ "wl_ampdu", "auto" },	/* Default AMPDU setting */
	/* Default AMPDU retry limit per-tid setting */
	{ "wl_ampdu_rtylimit_tid", "5 5 5 5 5 5 5 5" },
	/* Default AMPDU regular rate retry limit per-tid setting */
	{ "wl_ampdu_rr_rtylimit_tid", "2 2 2 2 2 2 2 2" },
	{ "wl_ampdu_mpdu", "0" },
	{ "wl_ampdu_rts", "1" },
	{ "wl_amsdu", "auto" },	/* Disable AMSDU Tx by default */
	{ "wl_rx_amsdu_in_ampdu", "auto" },	/* Disable AMSDU Rx by default */
	{ "wl_cal_period", "0" },	/* Disable periodic cal */
	{ "wl_obss_coex", "0" },	/* Default OBSS Coexistence setting - 0=OFF 1=ON */

	{ "wl0_stbc_tx", "auto" },	/* Default STBC TX setting */
	{ "wl0_stbc_rx", "1" },	/* Default STBC RX setting */
	{ "wl0_ampdu", "auto" },	/* Default AMPDU setting */
	/* Default AMPDU retry limit per-tid setting */
	{ "wl0_ampdu_rtylimit_tid", "7 7 7 7 7 7 7 7" },
	/* Default AMPDU regular rate retry limit per-tid setting */
	{ "wl0_ampdu_rr_rtylimit_tid", "3 3 3 3 3 3 3 3" },
	{ "wl0_ampdu_rts", "1" },
	{ "wl0_ampdu_mpdu", "0" },
	{ "wl0_amsdu", "auto" },	/* Default AMSDU setting */
	{ "wl0_rx_amsdu_in_ampdu", "auto" },	/* Disable AMSDU Rx by default */
	{ "wl0_cal_period", "0" },	/* Disable periodic cal */
	{ "wl0_obss_coex", "0" },	/* Default OBSS Coexistence setting - OFF */
	{ "wl0_bss_opmode_cap_reqd", "0" },
	{ "wl1_stbc_tx", "auto" },	/* Default STBC TX setting */
	{ "wl1_stbc_rx", "1" },	/* Default STBC RX setting */
	{ "wl1_ampdu", "auto" },	/* Default AMPDU setting */
	/* Default AMPDU retry limit per-tid setting */
	{ "wl1_ampdu_rtylimit_tid", "7 7 7 7 7 7 7 7" },
	/* Default AMPDU regular rate retry limit per-tid setting */
	{ "wl1_ampdu_rr_rtylimit_tid", "3 3 3 3 3 3 3 3" },
	{ "wl1_ampdu_rts", "1" },
	{ "wl1_ampdu_mpdu", "0" },
	{ "wl1_amsdu", "auto" },	/* Default AMSDU setting */
	{ "wl1_rx_amsdu_in_ampdu", "auto" },	/* Disable AMSDU Rx by default */
	{ "wl1_cal_period", "0" },	/* Disable periodic cal */
	{ "wl1_obss_coex", "0" },	/* Default OBSS Coexistence setting - OFF */
	{ "wl1_bss_opmode_cap_reqd", "0" },
	{ "wl2_stbc_tx", "auto" },	/* Default STBC TX setting */
	{ "wl2_stbc_rx", "1" },	/* Default STBC RX setting */
	{ "wl2_ampdu", "auto" },	/* Default AMPDU setting */
	/* Default AMPDU retry limit per-tid setting */
	{ "wl2_ampdu_rtylimit_tid", "7 7 7 7 7 7 7 7" },
	/* Default AMPDU regular rate retry limit per-tid setting */
	{ "wl2_ampdu_rr_rtylimit_tid", "3 3 3 3 3 3 3 3" },
	{ "wl2_ampdu_rts", "1" },
	{ "wl2_ampdu_mpdu", "0" },
	{ "wl2_amsdu", "auto" },	/* Default AMSDU setting */
	{ "wl2_rx_amsdu_in_ampdu", "auto" },	/* Disable AMSDU Rx by default */
	{ "wl2_cal_period", "0" },	/* Disable periodic cal */
	{ "wl2_obss_coex", "0" },	/* Default OBSS Coexistence setting - OFF */
	{ "wl2_bss_opmode_cap_reqd", "0" },
	/* Tx Beamforming */
#endif

	{ "wl0_sta_retry_time", "5" },	/* 100% duty cycle for LED on router */
	{ "wl_ifname", "" },	/* Interface name */
	{ "wl_hwaddr", "" },	/* MAC address */
	{ "wl_phytype", "g" },	/* Current wireless band ("a" (5 GHz), "b" *
	 * * (2.4 GHz), or "g" (2.4 GHz)) *//*
	 * Modify 
	 */
	{ "wl_corerev", "" },	/* Current core revision */
	{ "wl_phytypes", "" },	/* List of supported wireless bands (e.g.
				 * "ga") */
	{ "wl_radioids", "" },	/* List of radio IDs */
	{ "wl_shortslot", "auto" },
	{ "wl1_shortslot", "auto" },
	{ "wl2_shortslot", "auto" },
#ifdef HAVE_WTS
	{ "wl_ssid", "www.wts.com.ve" },	/* Service set ID (network name) */
#elif HAVE_DDLAN
	{ "wl_ssid", "www.ddlan.de" },	/* Service set ID (network name) */
#elif HAVE_SKYTEL
	{ "wl_ssid", "skytel" },	/* Service set ID (network name) */
#elif HAVE_POWERNOC
	{ "wl_ssid", "powernoc" },	/* Service set ID (network name) */
#elif HAVE_SKYTRON
	{ "wl_ssid", "SKYTRON Network" },	/* Service set ID (network name) */
#elif HAVE_SAGAR
	{ "wl_ssid", "hotspot-internet" },	/* Service set ID (network name) */
#elif HAVE_CORENET
	{ "wl_ssid", "corenet" },	/* Service set ID (network name) */
#elif defined(HAVE_GGEW) && !defined(HAVE_NS5) && !defined(HAVE_NS2) && !defined(HAVE_EOC5610) && !defined(HAVE_BUFFALO_BL_DEFAULTS)
	{ "wl_ssid", "GGEWnet-WLAN" },	/* Service set ID (network name) */
#elif HAVE_NEWMEDIA  && !defined(HAVE_NS5) && !defined(HAVE_NS2) && !defined(HAVE_EOC5610) && !defined(HAVE_BUFFALO_BL_DEFAULTS)
	{ "wl_ssid", "changeme" },	/* Service set ID (network name) */
#elif HAVE_MAKSAT
#if defined(HAVE_DEFREGDOMAIN)
	{ "wlan0_regdomain", HAVE_DEFREGDOMAIN },
#endif
#ifdef HAVE_MAKSAT_BLANK
	{ "wl_ssid", "default" },	/* Service set ID (network name) */
#else
	{ "wl_ssid", "maksat" },	/* Service set ID (network name) */
#endif
#elif HAVE_TMK
	{ "wl_ssid", "KMT" },	/* Service set ID (network name) */
#if defined(HAVE_UBNTM) && !defined(HAVE_UBNTXW)
	{ "wlan0_bias", "10" },
#endif
#elif HAVE_BKM
	{ "wl_ssid", "BKM-HSDL" },	/* Service set ID (network name) */
#elif HAVE_ERC
	{ "wl_ssid", "RemoteEngineer" },	/* Service set ID (network name) */
#elif HAVE_34TELECOM
	{ "wl_ssid", "Lobo" },	/* Service set ID (network name) */
#elif HAVE_KORENRON
	{ "wl_ssid", "WBR2000" },	/* Service set ID (network name) */
#elif HAVE_HOBBIT
	{ "wl_ssid", "HQ-NDS200" },
#else

#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
#ifdef HAVE_MAKSAT
	{ "wlan0_regulatory", "0" },
	{ "wlan1_regulatory", "0" },
	{ "wlan2_regulatory", "0" },
	{ "wlan3_regulatory", "0" },
#ifdef HAVE_MAKSAT_BLANK
	{ "wlan0_ssid", "default" },	/* Service set ID (network name) */
#else
	{ "wlan0_ssid", "maksat" },	/* Service set ID (network name) */
#endif
#else
#if defined(HAVE_TRIMAX)
	{ "wl0_ssid", "M2M" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "M2M" },	/* Service set ID (network name) */
#elif defined(HAVE_WIKINGS)
	{ "wl0_ssid", "Excel Networks" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "Excel Networks" },	/* Service set ID (network name) */
	{ "wkregdomain", "IR" },
#elif defined(HAVE_ESPOD)
	{ "wl0_ssid", "ESPOD" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "ESPOD" },	/* Service set ID (network name) */
	{ "wl1_ssid", "ESPOD" },	/* Service set ID (network name) */
	{ "wlan1_ssid", "ESPOD" },	/* Service set ID (network name) */
	{ "wl2_ssid", "ESPOD" },	/* Service set ID (network name) */
	{ "wlan2_ssid", "ESPOD" },	/* Service set ID (network name) */
#elif defined(HAVE_NEXTMEDIA)
	{ "wl0_ssid", "nextmedia" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "nextmedia" },	/* Service set ID (network name) */
#elif defined(HAVE_CARLSONWIRELESS)
	{ "wl0_ifname", "wlan0" },	/* Wireless interface name) */
	{ "wl0_ssid", "Carlson-STv2" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "Carlson-STv2" },	/* Service set ID (network name) */
	{ "wlan0_nctrlsb", "upper" },	/* wlan0 11n sub channel */
	{ "wlan0_ccmp", "1" },	/* wlan0 encryption type */
	{ "wlan0_security_mode", "psk" },	/* wlan0 encryption type */
	{ "wlan0_akm", "psk2" },	/* wlan0 encryption type */
	{ "wlan0_psk2", "1" },	/* wlan0 encryption type */
	{ "wlan0_txpwrdbm", "19" },	/* wlan0 transmit power */
#elif defined(HAVE_IMMERSIVE)
	{ "wlan0_ssid", "imm" },
	{ "wlan1_ssid", "imm_1" },
	{ "wlan2_ssid", "imm_2" },
#elif defined(HAVE_HDWIFI)
	{ "wlan0_ssid", "hdwifi1" },
	{ "wlan1_ssid", "hdwifi2" },
	{ "wlan2_ssid", "hdwifi3" },
	{ "wlan3_ssid", "hdwifi4" },
#elif defined(HAVE_ONNET_BLANK)
#ifdef HAVE_MMS344
#ifdef HAVE_CPE880
	{ "wlan0_ssid", "OTAi5.8" },
#else
	{ "wlan0_ssid", "OTAi2.4" },
	{ "wlan1_ssid", "OTAi5.8" },
#endif
#else
	{ "wlan0_ssid", "Enterprise WIFI" },
	{ "wlan1_ssid", "Enterprise WIFI_1" },
	{ "wlan2_ssid", "Enterprise WIFI_2" },
#endif
#elif defined(HAVE_ONNET)
#ifdef HAVE_MMS344
#ifdef HAVE_CPE880
	{ "wlan0_ssid", "OTAi5.8" },
#else
	{ "wlan0_ssid", "OTAi2.4" },
	{ "wlan1_ssid", "OTAi5.8" },
#endif
#else
	{ "wlan0_ssid", "OTAi" },
	{ "wlan1_ssid", "OTAi_1" },
#endif
#elif defined(HAVE_RAYTRONIK)
	{ "wlan0_ssid", "raytronik" },
	{ "wlan1_ssid", "raytronik" },
#elif defined(HAVE_GGEW) && defined(HAVE_NS5)
	{ "wlan0_ssid", "GGEWnet-WLAN" },	/* Service set ID (network name) */
#elif defined(HAVE_GGEW) && defined(HAVE_EOC5610)
	{ "wlan0_ssid", "GGEWnet-WLAN" },	/* Service set ID (network name) */
#elif defined(HAVE_GGEW) && defined(HAVE_NS2)
	{ "wlan0_ssid", "GGEWnet-WLAN" },	/* Service set ID (network name) */
#elif defined(HAVE_GGEW) && defined(HAVE_EOC2610)
	{ "wlan0_ssid", "GGEWnet-WLAN" },	/* Service set ID (network name) */
#elif defined(HAVE_CORENET)
	{ "wlan0_ssid", "corenet" },	/* Service set ID (network name) */
#elif defined(HAVE_DDLAN)
	{ "wl0_ssid", "www.ddlan.de" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "www.ddlan.de" },	/* Service set ID (network name) */
#elif defined(HAVE_TMK)
	{ "wl0_ssid", "KMT" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "KMT" },	/* Service set ID (network name) */
	{ "wlan0_intmit", "1" }, /* enable noise immunity */
#elif defined(HAVE_BKM)
	{ "wl0_ssid", "BKM-HSDL" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "BKM-HSDL" },	/* Service set ID (network name) */
#elif defined(HAVE_SANSFIL)
	{ "wlan0_ssid", "SANSFIL" },
#elif defined(HAVE_ERC)
	{ "wl0_ssid", "ERC" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "ERC" },	/* Service set ID (network name) */
#elif defined(HAVE_NDTRADE)
	{ "wlan0_ssid", "ND Trade 2G +201114899000" },	/* Service set ID (network name) */
	{ "wlan1_ssid", "ND Trade 5G +201114899000" },	/* Service set ID (network name) */
#elif defined(HAVE_IPR)
	{ "wl0_ssid", "IPR" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "IPR" },	/* Service set ID (network name) */
	{ "wlan0_regulatory", "1" },
	{ "wlan0_channel", "6000" },	/* 6000/chan 200 -wlan0 frequency */
	{ "wlan0_txpwrdbm", "6" },
	{ "wlan0_ccmp", "aes" },	/* wlan0 encryption type */
	{ "wlan0_security_mode", "psk" },	/* wlan0 encryption type */
	{ "wlan0_akm", "psk2" },	/* wlan0 encryption type */
	{ "wlan0_psk2", "1" },	/* wlan0 encryption type */
	{ "wlan0_wpa_psk", "marcomarco14" },	/* wlan0 encryption key */
#elif defined(HAVE_KORENRON)
	{ "wl0_ssid", "WBR2000" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "WBR2000" },	/* Service set ID (network name) */
#elif HAVE_IDEXX
	{ "wl0_ssid", "IDEXXw1" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "IDEXXw1" },	/* Service set ID (network name) */
	{ "wlan0_ccmp", "1" },	/* wlan0 encryption type */
	{ "wlan0_tkip", "1" },	/* wlan0 encryption type */
	{ "wl1_ssid", "IDEXXw2" },	/* Service set ID (network name) */
	{ "wlan1_ccmp", "1" },	/* wlan0 encryption type */
	{ "wlan1_tkip", "1" },	/* wlan0 encryption type */
	{ "wlan1_ssid", "IDEXXw2" },	/* Service set ID (network name) */
	{ "wlan0_vifs", "wlan0.1" },
	{ "wlan0.1_ccmp", "1" },	/* wlan0 encryption type */
	{ "wlan0.1_tkip", "1" },	/* wlan0 encryption type */
	{ "wlan0.1_ssid", "LabStation Guest" },	/* Service set ID (network name) */
#else
#if !defined(HAVE_BUFFALO) && !defined(HAVE_AXTEL) && !defined(HAVE_ANTAIRA)
	{ "wl0_ssid", "dd-wrt" },	/* Service set ID (network name) */
	{ "wlan0_ssid", "dd-wrt" },	/* Service set ID (network name) */
	{ "wlan0_akm", "disabled" },	/* Service set ID (network name) */
#endif
#endif

#endif
#ifdef HAVE_IDEXX
	{ "wlan0_wpa_psk", "IDEXXwlan1234" },	/* wlan0 encryption key */
	{ "wlan0.1_ssid", "LabStation Guest" },	/* Service set ID (network name) */
#else
	{ "wlan0.1_ssid", "" },	/* Service set ID (network name) */
	{ "wlan0.2_ssid", "" },	/* Service set ID (network name) */
	{ "wlan0.3_ssid", "" },	/* Service set ID (network name) */
	{ "wlan0_bridged", "1" },	/* Service set ID (network name) */
	{ "wlan0.1_bridged", "1" },	/* Service set ID (network name) */
	{ "wlan0.2_bridged", "1" },	/* Service set ID (network name) */
	{ "wlan0.3_bridged", "1" },	/* Service set ID (network name) */
	{ "wlan0_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wlan0.1_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wlan0.2_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wlan0.3_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wlan0_netmask", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wlan0.1_netmask", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wlan0.2_netmask", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wlan0.3_netmask", "0.0.0.0" },	/* Service set ID (network name) */
#endif
#else
#ifndef HAVE_BUFFALO
	{ "wl_ssid", "dd-wrt" },	/* Service set ID (network name) */
	{ "wl0_ssid", "dd-wrt" },	/* Service set ID (network name) */
	{ "wl1_ssid", "dd-wrt" },	/* Service set ID (network name) */
	{ "wl2_ssid", "dd-wrt" },	/* Service set ID (network name) */
#endif
	{ "wl0.1_ssid", "" },	/* Service set ID (network name) */
	{ "wl0.2_ssid", "" },	/* Service set ID (network name) */
	{ "wl0.3_ssid", "" },	/* Service set ID (network name) */
	{ "wl0_bridged", "1" },	/* Service set ID (network name) */
	{ "wl0.1_bridged", "1" },	/* Service set ID (network name) */
	{ "wl0.2_bridged", "1" },	/* Service set ID (network name) */
	{ "wl0.3_bridged", "1" },	/* Service set ID (network name) */
	{ "wl0_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wl0.1_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wl0.2_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wl0.3_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wl0_netmask", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wl0.1_netmask", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wl0.2_netmask", "0.0.0.0" },	/* Service set ID (network name) */
	{ "wl0.3_netmask", "0.0.0.0" },	/* Service set ID (network name) */
#endif
#endif

#ifdef HAVE_NEWMEDIA
	{ "wl_radio", "1" },	/* Enable (1) or disable (0) radio */
#else
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
#ifdef HAVE_IDEXX
	{ "wlan1_closed", "0" },	/* Closed (hidden) network */
#else
	{ "wlan0_radio", "1" },	/* Enable (1) or disable (0) radio */
	{ "wlan0_closed", "0" },	/* Closed (hidden) network */
	{ "wlan1_radio", "1" },	/* Enable (1) or disable (0) radio */
	{ "wlan1_closed", "0" },	/* Closed (hidden) network */
	{ "wlan2_radio", "1" },	/* Enable (1) or disable (0) radio */
	{ "wlan2_closed", "0" },	/* Closed (hidden) network */
	{ "wlan3_radio", "1" },	/* Enable (1) or disable (0) radio */
	{ "wlan3_closed", "0" },	/* Closed (hidden) network */
	{ "wlan4_radio", "1" },	/* Enable (1) or disable (0) radio */
	{ "wlan4_closed", "0" },	/* Closed (hidden) network */
	{ "wlan5_radio", "1" },	/* Enable (1) or disable (0) radio */
	{ "wlan5_closed", "0" },	/* Closed (hidden) network */
#endif
#else
	{ "wl_radio", "1" },	/* Enable (1) or disable (0) radio */
	{ "wl0_radio", "1" },	/* Enable (1) or disable (0) radio */
	{ "wl0_closed", "0" },	/* Closed (hidden) network */
	{ "wl0.1_closed", "0" },	/* Closed (hidden) network */
	{ "wl0.2_closed", "0" },	/* Closed (hidden) network */
	{ "wl0.3_closed", "0" },	/* Closed (hidden) network */
#ifdef HAVE_RT2880
	{ "wl1_radio", "1" },	/* Enable (1) or disable (0) radio */
	{ "wl1_closed", "0" },	/* Closed (hidden) network */
#endif
#endif

#endif

#ifdef HAVE_SAGAR
	{ "wl_ap_isolate", "1" },	/* AP isolate mode */
#elif HAVE_FON
	{ "wl_ap_isolate", "1" },	/* AP isolate mode */
#else
	{ "wl_ap_isolate", "0" },	/* AP isolate mode */
	{ "wl1_ap_isolate", "0" },
	{ "wl2_ap_isolate", "0" },
#endif
#ifdef HAVE_POWERNOC_WORT54G
	{ "wl_mode", "sta" },	/* AP mode (ap|sta|wet|infra) */
	{ "wl0_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif HAVE_SKYTRON
	{ "wl_mode", "sta" },
	{ "wl0_mode", "sta" },
#elif HAVE_GGEW && !defined(HAVE_NS5) && !defined(HAVE_NS2) && !defined(HAVE_EOC2610) && !defined(HAVE_EOC5610)
	{ "wl_mode", "sta" },
	{ "wl0_mode", "sta" },
#else

#if !defined(HAVE_MADWIFI) && !defined(HAVE_ATH9K)
	{ "wl_mode", "ap" },	/* AP mode (ap|sta|wet|infra) */
	{ "wl0_mode", "ap" },	/* AP mode (ap|sta|wet|infra) */
	{ "wl1_mode", "ap" },	/* AP mode (ap|sta|wet|infra) */
	{ "wl2_mode", "ap" },	/* AP mode (ap|sta|wet|infra) */
#else
#ifdef HAVE_DDLAN
	{ "wl_mode", "sta" },	/* AP mode (ap|sta|wet|infra) */
#elif defined(HAVE_GGEW) && defined(HAVE_NS5)
	{ "wl_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif defined(HAVE_GGEW) && defined(HAVE_EOC5610)
	{ "wl_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif defined(HAVE_GGEW) && defined(HAVE_NS2)
	{ "wl_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif defined(HAVE_GGEW) && defined(HAVE_EOC2610)
	{ "wl_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif defined(HAVE_GGEW) && defined(HAVE_WHRHPGN)
	{ "wl_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif HAVE_TRIMAX
	{ "wl_mode", "ap" },	/* AP mode (ap|sta|wet|infra) */
#else
	{ "wl_mode", "ap" },	/* AP mode (ap|sta|wet|infra) */
#endif
#ifdef HAVE_CARLSONWIRELESS
	{ "wlan0_channelbw", "40" },	/* wlan0 channel bandwidth */
#elif defined(HAVE_ONNET)
#ifdef HAVE_CPE880
	{ "wlan0_channelbw", "40" },
#else
	{ "wlan0_channelbw", "2040" },
	{ "wlan1_channelbw", "2040" },
#endif
#elif defined(HAVE_AXTEL)
	{ "wlan0_channelbw", "2040" },
#else
	{ "wlan0_channelbw", "20" },	/* AP mode (ap|sta|wds) */
	{ "wlan1_channelbw", "20" },	/* AP mode (ap|sta|wds) */
	{ "wlan2_channelbw", "20" },	/* AP mode (ap|sta|wds) */
	{ "wlan3_channelbw", "20" },	/* AP mode (ap|sta|wds) */
	{ "wlan4_channelbw", "20" },	/* AP mode (ap|sta|wds) */
	{ "wlan5_channelbw", "20" },	/* AP mode (ap|sta|wds) */
#endif

#ifdef HAVE_DDLAN
	{ "wlan0_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif defined(HAVE_GGEW) && defined(HAVE_NS5)
	{ "wlan0_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif defined(HAVE_GGEW) && defined(HAVE_EOC5610)
	{ "wlan0_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif defined(HAVE_GGEW) && defined(HAVE_NS2)
	{ "wlan0_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif defined(HAVE_GGEW) && defined(HAVE_EOC2610)
	{ "wlan0_mode", "sta" },	/* AP mode (ap|sta|wds) */
#elif defined(HAVE_GGEW) && defined(HAVE_WHRHPGN)
	{ "wlan0_mode", "ap" },	/* AP mode (ap|sta|wds) */
#elif HAVE_TRIMAX
	{ "wlan0_mode", "ap" },	/* AP mode (ap|sta|wds) */
#elif HAVE_CARLSONWIRELES
#ifdef HAVE_LAGUNA
	{ "wlan0_mode", "ap" },	/* AP mode (wdsap) */
#else
	{ "wlan0_mode", "wdsap" },	/* AP mode (wdsap) */
#endif
#elif HAVE_RAYTRONIK
	{ "wlan0_mode", "wdsap" },	/* AP mode (wdsap) */
	{ "wlan0_channelbw", "2040" },	/* LAN IP address */
#elif HAVE_ONNET
#ifdef HAVE_ONNET_STATION
	{ "wlan0_mode", "wdssta" },
#else
	{ "wlan0_mode", "wdsap" },
#endif
#else
	{ "wlan0_mode", "ap" },	/* AP mode (ap|sta|wds) */
	{ "wlan1_mode", "ap" },	/* AP mode (ap|sta|wds) */
	{ "wlan2_mode", "ap" },	/* AP mode (ap|sta|wds) */
	{ "wlan3_mode", "ap" },	/* AP mode (ap|sta|wds) */
	{ "wlan4_mode", "ap" },	/* AP mode (ap|sta|wds) */
	{ "wlan5_mode", "ap" },	/* AP mode (ap|sta|wds) */
#endif
	{ "wlan0_xr", "0" },	/* AP mode (ap|sta|wds) */
	{ "wlan1_xr", "0" },	/* AP mode (ap|sta|wds) */
	{ "wlan2_xr", "0" },	/* AP mode (ap|sta|wds) */
	{ "wlan3_xr", "0" },	/* AP mode (ap|sta|wds) */
	{ "wlan4_xr", "0" },	/* AP mode (ap|sta|wds) */
	{ "wlan5_xr", "0" },	/* AP mode (ap|sta|wds) */
#endif
#endif
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
	{ "wlan0_lazywds", "0" },	/* Enable "lazy" WDS mode (0|1) */
	{ "wlan1_lazywds", "0" },	/* Enable "lazy" WDS mode (0|1) */
	{ "wlan2_lazywds", "0" },	/* Enable "lazy" WDS mode (0|1) */
	{ "wlan3_lazywds", "0" },	/* Enable "lazy" WDS mode (0|1) */
#else
	{ "wl_lazywds", "0" },	/* Enable "lazy" WDS mode (0|1) */
#endif
	{ "wl_wds", "" },	/* xx:xx:xx:xx:xx:xx ... */
	{ "wl_wep", "disabled" },	/* Data encryption (off|wep|tkip|aes) */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_ATH9K)
#ifndef HAVE_BUFFALO
	{ "wl_crypto", "off" },	/* Data encryption (off|wep|tkip|aes) */
	{ "wl_auth", "0" },	/* Shared key authentication optional (0) or
				 * required (1) */
	{ "wl0_crypto", "off" },	/* Data encryption (off|wep|tkip|aes) */
#endif
	{ "wl0_auth", "0" },	/* Shared key authentication optional (0) or required (1) */
	{ "wl1_auth", "0" },
	{ "wl0_key", "1" },	/* Current WEP key */
	{ "wl0_key1", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0_key2", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0_key3", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0_key4", "" },	/* 5/13 char ASCII or 10/26 char hex */

	{ "wl0.1_crypto", "off" },	/* Data encryption (off|wep|tkip|aes) */
	{ "wl0.1_auth", "0" },	/* Shared key authentication optional (0) or
				 * required (1) */
	{ "wl0.1_key", "1" },	/* Current WEP key */
	{ "wl0.1_key1", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0.1_key2", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0.1_key3", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0.1_key4", "" },	/* 5/13 char ASCII or 10/26 char hex */

	{ "wl0.2_crypto", "off" },	/* Data encryption (off|wep|tkip|aes) */
	{ "wl0.2_auth", "0" },	/* Shared key authentication optional (0) or
				 * required (1) */
	{ "wl0.2_key", "1" },	/* Current WEP key */
	{ "wl0.2_key1", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0.2_key2", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0.2_key3", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0.2_key4", "" },	/* 5/13 char ASCII or 10/26 char hex */

	{ "wl0.3_crypto", "off" },	/* Data encryption (off|wep|tkip|aes) */
	{ "wl0.3_auth", "0" },	/* Shared key authentication optional (0) or
				 * required (1) */
	{ "wl0.3_key", "1" },	/* Current WEP key */
	{ "wl0.3_key1", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0.3_key2", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0.3_key3", "" },	/* 5/13 char ASCII or 10/26 char hex */
	{ "wl0.3_key4", "" },	/* 5/13 char ASCII or 10/26 char hex */
#endif
	{ "wl_macmode", "disabled" },	/* "allow" only, "deny" only, or
					 * "disabled" (allow all) */
	{ "wl_macmode1", "disabled" },	/* "disabled" or "other" for WEBB *//* Add */
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
#ifdef HAVE_CARLSONWIRELESS
	{ "wlan0_channel", "5180" },	/* 5275wlan0 frequency */
	{ "wlan0_rxantenna", "3" },
	{ "wlan0_txantenna", "3" },
#elif HAVE_IMMERSIVE
	{ "wlan0_channel", "2412" },
	{ "wlan1_channel", "2437" },
	{ "wlan2_channel", "2462" },
#elif HAVE_HDWIFI
	{ "wlan0_channel", "2412" },
	{ "wlan1_channel", "2437" },
	{ "wlan2_channel", "2462" },
#elif HAVE_NDTRADE
	{ "wlan0_channel", "2437" },	/* 6000/chan 200 -wlan0 frequency */
	{ "wlan1_channel", "5180" },	/* 6000/chan 200 -wlan0 frequency */
#else
	{ "wlan0_channel", "0" },	/* Channel number */
	{ "wlan1_channel", "0" },	/* Channel number */
#endif
#else

	{ "wl0_channel", "6" },	/* Channel number */

#endif
	{ "wl_reg_mode", "off" },	/* Regulatory: 802.11H(h)/802.11D(d)/off(off) 
					 */
	{ "wl_dfs_preism", "60" },	/* 802.11H pre network CAC time */
	{ "wl_dfs_postism", "60" },	/* 802.11H In Service Monitoring CAC
					 * time */
	{ "wl_rate", "0" },	/* Rate (bps, 0 for auto) */
	{ "wl1_rate", "0" },	/* Rate (bps, 0 for auto) */
	{ "wl_mrate", "0" },	/* Mcast Rate (bps, 0 for auto) */
	{ "wl_rateset", "default" },	/* "default" or "all" or "12" */
	{ "wl1_rateset", "default" },	/* "default" or "all" or "12" */
	{ "wl_frag", "2346" },	/* Fragmentation threshold */
	{ "wl1_frag", "2346" },
#ifdef HAVE_POWERNOC_WORT54G
	{ "wl_rts", "65" },	/* RTS threshold */
#else
	{ "wl_rts", "2347" },	/* RTS threshold */
	{ "wl1_rts", "2347" },
#endif
	{ "wl_dtim", "1" },	/* DTIM period (3.11.5) *//* It is best value for WiFi test */
	{ "wl1_dtim", "1" },

	{ "wl_bcn", "100" },	/* Beacon interval */
	{ "wl1_bcn", "100" },
	{ "wl_plcphdr", "long" },	/* 802.11b PLCP preamble type */
	{ "wl1_plcphdr", "long" },

#ifdef HAVE_GGEW
#if defined(HAVE_NS5) || defined(HAVE_EOC5610)
	{ "wlan0_net_mode", "a-only" },
#elif defined(HAVE_EOC2610) || defined(HAVE_NS2)
	{ "wlan0_net_mode", "mixed" },
#else
	{ "wl0_net_mode", "b-only" },	/* Wireless mode (mixed|g-only|b-only|disable) */
#endif
#elif HAVE_NEWMEDIA
	{ "wl_net_mode", "disabled" },	/* Wireless mode
					 * (mixed|g-only|b-only|disable) */
	{ "wl0_net_mode", "disabled" },	/* Wireless mode
					 * (mixed|g-only|b-only|disable) */
#elif HAVE_DDLAN
#ifdef HAVE_NS5
	{ "wlan0_net_mode", "a-only" },
	{ "wl0_net_mode", "a-only" },	/* Wireless mode
					 * (mixed|g-only|b-only|disable) */
#else
	{ "wlan0_net_mode", "b-only" },
	{ "wl0_net_mode", "b-only" },	/* Wireless mode
					 * (mixed|g-only|b-only|disable) */
#endif
#elif HAVE_CARLSONWIRELESS
	{ "wlan0_net_mode", "n5-only" },	/* wlan0 wireless mode */
#elif HAVE_ONNET
#ifdef HAVE_CPE880
	{ "wlan0_net_mode", "n5-only" },	/* wlan0 wireless mode */
#else
	{ "wlan0_net_mode", "mixed" },	/* wlan0 wireless mode */
#endif
#else
	{ "wl_net_mode", "mixed" },	/* Wireless mode
					 * (mixed|g-only|b-only|disable) */
	{ "wl0_net_mode", "mixed" },	/* Wireless mode
					 * (mixed|g-only|b-only|disable) */
	{ "wl1_net_mode", "mixed" },
	{ "wl2_net_mode", "mixed" },
#endif

#ifdef HAVE_SAGAR
	{ "wl0_gmode", XSTR(GMODE_LEGACY_B) },	/* 54g mode */
#elif HAVE_GGEW
	{ "wl0_gmode", "0" },	/* 54g mode */
#elif HAVE_NEWMEDIA
	{ "wl0_gmode", "-1" },	/* 54g mode */
#else
	{ "wl0_gmode", XSTR(GMODE_AUTO) },	/* 54g mode */
#endif

	{ "wl_gmode_protection", "auto" },	/* 802.11g RTS/CTS protection (off|auto) */
	{ "wl1_gmode_protection", "auto" },
	{ "wl2_gmode_protection", "auto" },
	{ "wl_nmode_protection", "auto" },	/* 802.11g RTS/CTS protection (off|auto) */
#ifdef HAVE_SKYTEL
	{ "wl_frameburst", "on" },	/* BRCM Frambursting mode (off|on) */
#elif HAVE_GGEW
	{ "wl_frameburst", "on" },	/* BRCM Frambursting mode (off|on) */
#else
	{ "wl_frameburst", "off" },	/* BRCM Frambursting mode (off|on) */
	{ "wl1_frameburst", "off" },
	{ "wl2_frameburst", "off" },
#endif

	{ "wl_infra", "1" },	/* Network Type (BSS/IBSS) */

	{ "wl_passphrase", "" },	/* Passphrase *//* Add */
	{ "wl_wep_bit", "64" },	/* WEP encryption [64 | 128] *//* Add */
	{ "wl_wep_buf", "" },	/* save all settings for web *//* Add */
	{ "wl_wep_gen", "" },	/* save all settings for generate button *//* Add */
	{ "wl_wep_last", "" },	/* Save last wl_wep mode *//* Add */
	{ "wl_active_mac", "" },	/* xx:xx:xx:xx:xx:xx ... *//* Add */
	{ "wl_mac_list", "" },	/* filter MAC *//* Add */
	{ "wl_mac_deny", "" },	/* filter MAC *//* Add */

	/*
	 * WPA parameters 
	 */
	{ "security_mode", "disabled" },	/* WPA mode
	 * * * WEB *//*
	 * Add 
	 */
	{ "security_mode_last", "" },	/* Save last WPA mode *//* Add */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_ATH9K)
	{ "wl0_auth_mode", "disabled" },	/* WPA mode (disabled|radius|wpa|psk) 
						 */
#ifndef HAVE_BUFFALO
	{ "wl0_akm", "disabled" },
	{ "wl0_wpa_psk", "" },	/* WPA pre-shared key */
#endif

#ifdef HAVE_IDEXX
	{ "wl0_akm", "psk psk2" },
	{ "wl0_wpa_psk", "IDEXXwlan1234" },	/* WPA pre-shared key */
#endif
	{ "wl0_wpa_gtk_rekey", "3600" },	/* WPA GTK rekey interval *//* Modify */
	{ "wl0_radius_port", "1812" },	/* RADIUS server UDP port */
	{ "wl0_radius_ipaddr", "" },	/* RADIUS server IP address */
	{ "wl0_radius_key", "" },	/* RADIUS shared secret */
#ifndef HAVE_BUFFALO
	{ "wl0_security_mode", "disabled" },	/* WPA mode */
#elif HAVE_IDEXX
	{ "wl0_security_mode", "psk psk2" },	/* WPA mode */
#endif

	{ "wl0.1_auth_mode", "disabled" },	/* WPA mode (disabled|radius|wpa|psk) 
						 */
	{ "wl0.1_akm", "disabled" },
	{ "wl0.1_wpa_psk", "" },	/* WPA pre-shared key */
	{ "wl0.1_wpa_gtk_rekey", "3600" },	/* WPA GTK rekey interval *//* Modify */
	{ "wl0.1_radius_port", "1812" },	/* RADIUS server UDP port */
	{ "wl0.1_radius_ipaddr", "" },	/* RADIUS server IP address */
	{ "wl0.1_radius_key", "" },	/* RADIUS shared secret */

	{ "wl0.2_auth_mode", "disabled" },	/* WPA mode (disabled|radius|wpa|psk) 
						 */
	{ "wl0.2_akm", "disabled" },
	{ "wl0.2_wpa_psk", "" },	/* WPA pre-shared key */
	{ "wl0.2_wpa_gtk_rekey", "3600" },	/* WPA GTK rekey interval *//* Modify */
	{ "wl0.2_radius_port", "1812" },	/* RADIUS server UDP port */
	{ "wl0.2_radius_ipaddr", "" },	/* RADIUS server IP address */
	{ "wl0.2_radius_key", "" },	/* RADIUS shared secret */

	{ "wl0.3_auth_mode", "disabled" },	/* WPA mode (disabled|radius|wpa|psk) 
						 */
	{ "wl0.3_akm", "disabled" },
	{ "wl0.3_wpa_psk", "" },	/* WPA pre-shared key */
	{ "wl0.3_wpa_gtk_rekey", "3600" },	/* WPA GTK rekey interval *//* Modify */
	{ "wl0.3_radius_port", "1812" },	/* RADIUS server UDP port */
	{ "wl0.3_radius_ipaddr", "" },	/* RADIUS server IP address */
	{ "wl0.3_radius_key", "" },	/* RADIUS shared secret */

#else
#ifdef HAVE_IDEXX
	{ "wlan0_security_mode", "psk" },	/* WPA mode */
	{ "wlan0_akm", "psk psk2" },	/* WPA mode */
	{ "wlan0_psk", "1" },	/* WPA mode */
	{ "wlan0_psk2", "1" },	/* WPA mode */
	{ "wlan0_auth_mode", "psk psk2" },	/* WPA mode (disabled|radius|wpa|psk)  */
	{ "wlan0.1_security_mode", "psk psk2" },	/* WPA mode */
	{ "wlan0X1_security_mode", "psk psk2" },	/* WPA mode */
	{ "wlan1_security_mode", "psk" },	/* WPA mode */
	{ "wlan1_akm", "psk psk2" },	/* WPA mode */
	{ "wlan1_psk", "1" },	/* WPA mode */
	{ "wlan1_psk2", "1" },	/* WPA mode */
#else
	{ "wlan0_auth_mode", "disabled" },	/* WPA mode (disabled|radius|wpa|psk)  */
#endif
#if !defined(HAVE_BUFFALO) && !defined(HAVE_AXTEL)
#ifdef HAVE_CARLSONWIRELESS
	{ "wlan0_akm", "psk2" },
	{ "wlan0_psk2", "1" },
	{ "wlan0_wpa_psk", "7078227000" },	/* wlan0 encryption key */
#elif defined(HAVE_ANTAIRA)
	{ "wlan0_security_mode", "psk2" },
	{ "wlan0_akm", "psk2" },
	{ "wlan0_psk2", "1" },
	{ "wlan0_wpa_psk", "12345678" },
	{ "wlan0_ccmp", "1" },
	{ "wlan1_security_mode", "psk2" },
	{ "wlan1_akm", "psk2" },
	{ "wlan1_psk2", "1" },
	{ "wlan1_wpa_psk", "12345678" },
	{ "wlan1_ccmp", "1" },
	{ "wlan2_security_mode", "psk2" },
	{ "wlan2_akm", "psk2" },
	{ "wlan2_psk2", "1" },
	{ "wlan2_wpa_psk", "12345678" },
	{ "wlan2_ccmp", "1" },
#else
#ifndef HAVE_BUFFALO
	{ "wlan1_akm", "disabled" },
	{ "wlan1_wpa_psk", "" },	/* WPA pre-shared key */
#elif defined(HAVE_IDEXX)
	{ "wlan1_akm", "psk psk2" },
	{ "wlan1_wpa_psk", "IDEXXwlan1234" },	/* WPA pre-shared key */
#else
	{ "wlan0_akm", "disabled" },
	{ "wlan0_wpa_psk", "" },	/* WPA pre-shared key */
	{ "wlan0_wpa_gtk_rekey", "3600" },	/* WPA GTK rekey interval *//* Modify */
	{ "wlan0_radius_port", "1812" },	/* RADIUS server UDP port */
	{ "wlan0_radius_ipaddr", "" },	/* RADIUS server IP address */
	{ "wlan0_radius_key", "" },	/* RADIUS shared secret */

	{ "wlan1_wpa_gtk_rekey", "3600" },	/* WPA GTK rekey interval *//* Modify */
	{ "wlan1_radius_port", "1812" },	/* RADIUS server UDP port */
	{ "wlan1_auth_mode", "disabled" },	/* WPA mode (disabled|radius|wpa|psk) 
						 */
	{ "wlan1_radius_ipaddr", "" },	/* RADIUS server IP address */
	{ "wlan1_radius_key", "" },	/* RADIUS shared secret */

	{ "wlan2_wpa_gtk_rekey", "3600" },	/* WPA GTK rekey interval *//* Modify */
	{ "wlan2_radius_port", "1812" },	/* RADIUS server UDP port */
	{ "wlan2_auth_mode", "disabled" },	/* WPA mode (disabled|radius|wpa|psk) 
						 */
	{ "wlan2_akm", "disabled" },
	{ "wlan2_wpa_psk", "" },	/* WPA pre-shared key */
	{ "wlan2_radius_ipaddr", "" },	/* RADIUS server IP address */
	{ "wlan2_radius_key", "" },	/* RADIUS shared secret */

	{ "wlan3_wpa_gtk_rekey", "3600" },	/* WPA GTK rekey interval *//* Modify */
	{ "wlan3_radius_port", "1812" },	/* RADIUS server UDP port */
	{ "wlan3_auth_mode", "disabled" },	/* WPA mode (disabled|radius|wpa|psk) 
						 */
	{ "wlan3_akm", "disabled" },
	{ "wlan3_wpa_psk", "" },	/* WPA pre-shared key */
	{ "wlan3_radius_ipaddr", "" },	/* RADIUS server IP address */
	{ "wlan3_radius_key", "" },	/* RADIUS shared secret */

	{ "wlan4_wpa_gtk_rekey", "3600" },	/* WPA GTK rekey interval *//* Modify */
	{ "wlan4_radius_port", "1812" },	/* RADIUS server UDP port */
	{ "wlan4_auth_mode", "disabled" },	/* WPA mode (disabled|radius|wpa|psk) 
						 */
	{ "wlan4_akm", "disabled" },
	{ "wlan4_wpa_psk", "" },	/* WPA pre-shared key */
	{ "wlan4_radius_ipaddr", "" },	/* RADIUS server IP address */
	{ "wlan4_radius_key", "" },	/* RADIUS shared secret */
	{ "wlan5_wpa_gtk_rekey", "3600" },	/* WPA GTK rekey interval *//* Modify */
	{ "wlan5_radius_port", "1812" },	/* RADIUS server UDP port */
	{ "wlan5_auth_mode", "disabled" },	/* WPA mode (disabled|radius|wpa|psk) 
						 */
	{ "wlan5_akm", "disabled" },
	{ "wlan5_wpa_psk", "" },	/* WPA pre-shared key */
	{ "wlan5_radius_ipaddr", "" },	/* RADIUS server IP address */
	{ "wlan5_radius_key", "" },	/* RADIUS shared secret */
#endif
#endif
#endif
#ifdef HAVE_IDEXX
	{ "wlan0_akm", "psk psk2" },
	{ "wlan0_psk", "1" },
	{ "wlan0_psk2", "1" },
	{ "wlan0_wpa_psk", "IDEXXwlan1234" },	/* wlan0 encryption key */
	{ "wlan0.1_akm", "psk psk2" },
	{ "wlan0.1_wpa_psk", "IDEXXguest1234" },	/* wlan0 encryption key */
#endif
#ifdef HAVE_GGEW
	{ "wlan0_8021xtype", "ttls" },
	{ "wlan0_ttls8021xanon", "anonymous" },
	{ "wlan0_ttls8021xphase2", "auth=PAP" },
	{ "wlan0_ttls8021xca", "-----BEGIN CERTIFICATE-----\n"
	 "MIICfTCCAeYCCQC/0xTqd3htwDANBgkqhkiG9w0BAQUFADCBgjELMAkGA1UEBhMC\n"
	 "REUxDzANBgNVBAgTBkhlc3NlbjERMA8GA1UEBxMIQmVuc2hlaW0xFTATBgNVBAoT\n"
	 "DEdHRVduZXQgR21iSDEXMBUGA1UEAxMOY2EuZ2dldy1uZXQuZGUxHzAdBgkqhkiG\n"
	 "9w0BCQEWEGluZm9AZ2dldy1uZXQuZGUwHhcNMDgxMjEzMTIzNjU3WhcNMzcwOTEy\n"
	 "MTIzNjU3WjCBgjELMAkGA1UEBhMCREUxDzANBgNVBAgTBkhlc3NlbjERMA8GA1UE\n"
	 "BxMIQmVuc2hlaW0xFTATBgNVBAoTDEdHRVduZXQgR21iSDEXMBUGA1UEAxMOY2Eu\n"
	 "Z2dldy1uZXQuZGUxHzAdBgkqhkiG9w0BCQEWEGluZm9AZ2dldy1uZXQuZGUwgZ8w\n"
	 "DQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBAPoMxn2kv8u/im4rt5nJjV1rYpWn4Xzi\n"
	 "CME3aus+ZgRw6nrgWZSX8Zu1B4ZRpGD0I10UAgrjlkNHNVqiBkCxQd8MZDUsnzd+\n"
	 "i4fZfYBqHliJUE4tCLWbBzMLiZTfuSb6TRaGhCnesXWQ6iIgjI/LJk274Wtq+zc8\n"
	 "ENGTIghlKJH/AgMBAAEwDQYJKoZIhvcNAQEFBQADgYEAe7Q6yWGdMX5f6GDAbFVR\n"
	 "xEZSLgIM6TIazKARcgoV1fD5ymfb9bkWHt2/gXp9EGKVH97nwlkxvR4oYCCVQ9Cp\n" "hyMc/KTqX9P9M6ZTxwIBN+bkgIIbmArzkHRMrONYOgxAW1oGV+mnHPmgo3rF7fuI\n" "kSlc2ZFwN5KCX2+3TdcNnVk=\n" "-----END CERTIFICATE-----\n" },
#else
	{ "wlan0_8021xtype", "peap" },
#endif
	{ "wlan1_8021xtype", "peap" },
	{ "wlan2_8021xtype", "peap" },
	{ "wlan3_8021xtype", "peap" },
	{ "wlan4_8021xtype", "peap" },
	{ "wlan5_8021xtype", "peap" },
#endif
	{ "wl0_radius_override", "1" },	// overrides radius if server is
	// unavailable
	{ "wl0_max_unauth_users", "0" },	// overrides radius if server is
	// unavailable
	{ "wl0_radmacpassword", "0" },	// overrides radius if server is
	// unavailable
#ifdef HAVE_SKYTEL
	{ "wl_afterburner", "auto" },	/* Afterburner/Speedbooster */
#else
	{ "wl_afterburner", "off" },	/* Afterburner/Speedbooster */
	{ "wl1_afterburner", "off" },
#endif
	{ "wl_unit", "0" },	/* Last configured interface */

	/*
	 * Restore defaults 
	 */
	{ "restore_defaults", "0" },	/* Set to 0 to not restore defaults
					 * on boot */

	// //////////////////////////////////////
#ifdef HAVE_WTS
	{ "router_name", "WTS" },	/* Router name string */
#elif  HAVE_SKYTEL
	{ "router_name", "ST54G" },
#elif  HAVE_CORENET
	{ "router_name", "core-packet" },
#elif  HAVE_POWERNOC_WORT54G
	{ "router_name", "WORT54G" },
#elif  HAVE_POWERNOC_WOAP54G
	{ "router_name", "WOAP54G" },
#elif  HAVE_SKYTRON
	{ "router_name", "skymax254b" },
#elif  HAVE_34TELECOM
	{ "router_name", "MiuraBasic" },
#elif  HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
	{ "router_name", "default" },
#else
	{ "router_name", "MAKSAT" },
#endif
#elif  HAVE_SANSFIL
	{ "router_name", "SANSFIL" },
#elif  HAVE_TRIMAX
	{ "router_name", "M2M" },
#elif  HAVE_WIKINGS
	{ "router_name", "Excel Networks" },
#elif  HAVE_ESPOD
	{ "router_name", "ESPOD" },
#elif  HAVE_IMMERSIVE
	{ "router_name", "IMMERSIVE" },
#elif  HAVE_HDWIFI
	{ "router_name", "HDWIFI" },
#elif  HAVE_NEXTMEDIA
	{ "router_name", "NEXTMEDIA" },
#elif  HAVE_ONNET_BLANK
	{ "router_name", "Enterprise AP" },
#elif  HAVE_ONNET
	{ "router_name", "OTAi" },
#elif  HAVE_RAYTRONIK
	{ "router_name", "RN-150M" },
#elif  HAVE_DDLAN
	{ "router_name", "WDSL-Modem XXX" },
#elif  HAVE_TMK
	{ "router_name", "KMT-WAS" },
	{ "wlan0_ssid", "KMT" },
	{ "wlan0_intmit", "1" }, /* enable noise immunity */
	{ "wlan1_ssid", "KMT" },
	{ "wlan1_intmit", "1" }, /* enable noise immunity */
#elif  HAVE_NDTRADE
	{ "router_name", "ND-TRADE" },
#elif  HAVE_ANTAIRA
#ifdef HAVE_HABANERO
	{ "wlan0_ssid", "Antaira-2" },
	{ "wlan1_ssid", "Antaira-5" },
#else
	{ "wlan0_ssid", "Antaira" },
	{ "wlan1_ssid", "Antaira" },
#endif
	{ "wlan2_ssid", "Antaira" },
	{ "router_name", "Antaira" },
	{ "radius_country", "US" },
	{ "radius_state", "California" },
	{ "radius_locality", "none" },
	{ "radius_organisation", "Antaira" },
	{ "radius_email", "info@antaira.com" },
	{ "radius_common", "Antaira FreeRadius Certificate" },
	{ "radius_passphrase", "none" },
	{ "wan_default", "eth0" },
	{ "resetbutton_enable", "0" },
	{ "menu_nosecurity", "1" },
	{ "menu_noaccrestriction", "1" },
	{ "sv_restore_defaults", "0" },
	{ "xor_backup", "1" },
#elif  HAVE_BKM
	{ "router_name", "BKM-HSDL" },
#elif  HAVE_ERC
	{ "router_name", "RemoteEngineer" },
	{ "ree_resetme", "1" },
#ifdef HAVE_CARAMBOLA
	{ "erc_reset", "1" },
#endif
#ifdef HAVE_RUT500
	{ "erc_reset", "1" },
#endif
#elif  HAVE_CARLSONWIRELESS
#ifdef HAVE_LAGUNA
	{ "router_name", "CWT-LH-STv2" },	/* Router name) */
#else
	{ "router_name", "CWT" },	/* Router name) */
#endif
#elif HAVE_IPR
	{ "router_name", "IPR" },
#elif HAVE_KORENRON
	{ "router_name", "KORENRON" },
#elif HAVE_HOBBIT
	{ "router_name", "HQ-NDS200" },
#else
	{ "router_name", MODEL_NAME },	/* Router name string */
#endif
	{ "ntp_mode", "auto" },	/* NTP server [manual | auto] */
	{ "pptp_server_ip", "" },	/* as same as WAN gateway */
	{ "pptp_get_ip", "" },	/* IP Address assigned by PPTP server */

	/*
	 * for firewall 
	 */

#ifdef HAVE_SKYTRON
	{ "filter", "off" },	/* Firewall Protection [on|off] */
	{ "block_wan", "0" },	/* Block WAN Request [1|0] */
	{ "block_ident", "0" },	/* Block IDENT passthrough [1|0] */
	{ "block_proxy", "0" },	/* Block Proxy [1|0] */
	{ "block_java", "0" },	/* Block Java [1|0] */
	{ "block_activex", "0" },	/* Block ActiveX [1|0] */
	{ "block_cookie", "0" },	/* Block Cookie [1|0] */
	{ "block_multicast", "1" },	/* Multicast Pass Through [1|0] */
	{ "block_loopback", "0" },	/* Block NAT loopback [1|0] */
	{ "ipsec_pass", "1" },	/* IPSec Pass Through [1|0] */
	{ "pptp_pass", "1" },	/* PPTP Pass Through [1|0] */
	{ "l2tp_pass", "1" },	/* L2TP Pass Through [1|0] */
	{ "remote_management", "1" },	/* Remote Management [1|0] */
#ifndef HAVE_MICRO
//      {"limit_http", "0"}, /* Impede DDoS/Brutforce [1|0] */
	{ "limit_ssh", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_telnet", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_pptp", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_ftp", "0" },	/* Impede DDoS/Brutforce [1|0] */
#endif
#elif HAVE_SAGAR
	{ "filter", "off" },	/* Firewall Protection [on|off] */
	{ "block_wan", "0" },	/* Block WAN Request [1|0] */
	{ "block_ident", "0" },	/* Block IDENT passthrough [1|0] */
	{ "block_proxy", "0" },	/* Block Proxy [1|0] */
	{ "block_java", "0" },	/* Block Java [1|0] */
	{ "block_activex", "0" },	/* Block ActiveX [1|0] */
	{ "block_cookie", "0" },	/* Block Cookie [1|0] */
	{ "block_multicast", "1" },	/* Multicast Pass Through [1|0] */
	{ "block_loopback", "0" },	/* Block NAT loopback [1|0] */
	{ "ipsec_pass", "1" },	/* IPSec Pass Through [1|0] */
	{ "pptp_pass", "1" },	/* PPTP Pass Through [1|0] */
	{ "l2tp_pass", "1" },	/* L2TP Pass Through [1|0] */
	{ "remote_management", "1" },	/* Remote Management [1|0] */
#ifndef HAVE_MICRO
//      {"limit_http", "0"}, /* Impede DDoS/Brutforce [1|0] */
	{ "limit_ssh", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_telnet", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_pptp", "0" },	/* Impede DDoS/Brutforce [1|0] */
#endif
#elif HAVE_DDLAN
	{ "filter", "off" },	/* Firewall Protection [on|off] */
	{ "block_wan", "0" },	/* Block WAN Request [1|0] */
	{ "block_ident", "0" },	/* Block IDENT passthrough [1|0] */
	{ "block_proxy", "0" },	/* Block Proxy [1|0] */
	{ "block_java", "0" },	/* Block Java [1|0] */
	{ "block_activex", "0" },	/* Block ActiveX [1|0] */
	{ "block_cookie", "0" },	/* Block Cookie [1|0] */
	{ "block_multicast", "1" },	/* Multicast Pass Through [1|0] */
	{ "block_loopback", "0" },	/* Block NAT loopback [1|0] */
	{ "ipsec_pass", "1" },	/* IPSec Pass Through [1|0] */
	{ "pptp_pass", "1" },	/* PPTP Pass Through [1|0] */
	{ "l2tp_pass", "1" },	/* L2TP Pass Through [1|0] */
	{ "remote_management", "1" },	/* Remote Management [1|0] */
#ifndef HAVE_MICRO
//      {"limit_http", "0"}, /* Impede DDoS/Brutforce [1|0] */
	{ "limit_ssh", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_telnet", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_pptp", "0" },	/* Impede DDoS/Brutforce [1|0] */
#endif
#elif HAVE_MAKSAT
	{ "filter", "off" },	/* Firewall Protection [on|off] */
	{ "block_wan", "0" },	/* Block WAN Request [1|0] */
	{ "block_ident", "0" },	/* Block IDENT passthrough [1|0] */
	{ "block_proxy", "0" },	/* Block Proxy [1|0] */
	{ "block_java", "0" },	/* Block Java [1|0] */
	{ "block_activex", "0" },	/* Block ActiveX [1|0] */
	{ "block_cookie", "0" },	/* Block Cookie [1|0] */
	{ "block_multicast", "1" },	/* Multicast Pass Through [1|0] */
	{ "block_loopback", "0" },	/* Block NAT loopback [1|0] */
	{ "ipsec_pass", "1" },	/* IPSec Pass Through [1|0] */
	{ "pptp_pass", "1" },	/* PPTP Pass Through [1|0] */
	{ "l2tp_pass", "1" },	/* L2TP Pass Through [1|0] */
#ifndef HAVE_MICRO
//      {"limit_http", "0"}, /* Impede DDoS/Brutforce [1|0] */
	{ "limit_ssh", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_telnet", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_pptp", "0" },	/* Impede DDoS/Brutforce [1|0] */
#endif
#elif HAVE_XIOCOM
	{ "filter", "off" },	/* Firewall Protection [on|off] */
	{ "block_wan", "0" },	/* Block WAN Request [1|0] */
	{ "block_ident", "0" },	/* Block IDENT passthrough [1|0] */
	{ "block_proxy", "0" },	/* Block Proxy [1|0] */
	{ "block_java", "0" },	/* Block Java [1|0] */
	{ "block_activex", "0" },	/* Block ActiveX [1|0] */
	{ "block_cookie", "0" },	/* Block Cookie [1|0] */
	{ "block_multicast", "1" },	/* Multicast Pass Through [1|0] */
	{ "block_loopback", "0" },	/* Block NAT loopback [1|0] */
	{ "ipsec_pass", "1" },	/* IPSec Pass Through [1|0] */
	{ "pptp_pass", "1" },	/* PPTP Pass Through [1|0] */
	{ "l2tp_pass", "1" },	/* L2TP Pass Through [1|0] */
	{ "remote_management", "0" },	/* Remote Management [1|0] */
#ifndef HAVE_MICRO
//      {"limit_http", "0"}, /* Impede DDoS/Brutforce [1|0] */
	{ "limit_ssh", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_telnet", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_pptp", "0" },	/* Impede DDoS/Brutforce [1|0] */
#endif
#elif HAVE_IDEXX
	{ "filter", "on" },	/* Firewall Protection [on|off] */
	{ "block_wan", "1" },	/* Block WAN Request [1|0] */
	{ "block_ident", "1" },	/* Block IDENT passthrough [1|0] */
	{ "block_proxy", "0" },	/* Block Proxy [1|0] */
	{ "block_java", "0" },	/* Block Java [1|0] */
	{ "block_activex", "0" },	/* Block ActiveX [1|0] */
	{ "block_cookie", "0" },	/* Block Cookie [1|0] */
	{ "block_multicast", "0" },	/* Multicast Pass Through [1|0] */
	{ "block_loopback", "0" },	/* Block NAT loopback [1|0] */
	{ "block_snmp", "1" },	/* Block WAN SNMP access [1|0] */
	{ "ipsec_pass", "1" },	/* IPSec Pass Through [1|0] */
	{ "pptp_pass", "1" },	/* PPTP Pass Through [1|0] */
	{ "l2tp_pass", "1" },	/* L2TP Pass Through [1|0] */
#ifndef HAVE_MICRO
//      {"limit_http", "0"}, /* Impede DDoS/Brutforce [1|0] */
	{ "limit_ssh", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_telnet", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_pptp", "0" },	/* Impede DDoS/Brutforce [1|0] */
#endif
#elif HAVE_NDTRADE
	{ "remote_management", "1" },	/* Remote Management [1|0] */
	{ "http_wanport", "80" },	/* Remote Management Port */
#else
	{ "filter", "on" },	/* Firewall Protection [on|off] */
	{ "block_wan", "1" },	/* Block WAN Request [1|0] */
	{ "block_ident", "1" },	/* Block IDENT passthrough [1|0] */
	{ "block_proxy", "0" },	/* Block Proxy [1|0] */
	{ "block_java", "0" },	/* Block Java [1|0] */
	{ "block_activex", "0" },	/* Block ActiveX [1|0] */
	{ "block_cookie", "0" },	/* Block Cookie [1|0] */
	{ "block_multicast", "1" },	/* Multicast Pass Through [1|0] */
	{ "block_loopback", "0" },	/* Block NAT loopback [1|0] */
	{ "block_snmp", "1" },	/* Block WAN SNMP access [1|0] */
	{ "ipsec_pass", "1" },	/* IPSec Pass Through [1|0] */
	{ "pptp_pass", "1" },	/* PPTP Pass Through [1|0] */
	{ "l2tp_pass", "1" },	/* L2TP Pass Through [1|0] */
#ifndef HAVE_MICRO
//      {"limit_http", "0"}, /* Impede DDoS/Brutforce [1|0] */
	{ "limit_ssh", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_telnet", "0" },	/* Impede DDoS/Brutforce [1|0] */
	{ "limit_pptp", "0" },	/* Impede DDoS/Brutforce [1|0] */
#endif
	{ "arp_spoofing", "1" },
	{ "filter_tos", "0" },
	{ "filter_invalid", "1" },
#ifdef HAVE_DDLAN
	{ "remote_management", "1" },	/* Remote Management [1|0] */
#elif HAVE_GGEW
	{ "remote_management", "0" },	/* Remote Management [1|0] */
#elif HAVE_TW6600
	{ "remote_management", "1" },	/* Remote Management [1|0] */
#else
	{ "remote_management", "0" },	/* Remote Management [1|0] */
#endif
#endif
#ifdef HAVE_SAGAR
	{ "remote_mgt_https", "1" },	/* Remote Management use https [1|0] */// add
#elif HAVE_HTTPS
	{ "remote_mgt_https", "0" },	/* Remote Management use https [1|0] */// add
#endif

	{ "mtu_enable", "0" },	/* WAN MTU [1|0] */
	{ "wan_mtu", "1500" },	/* Negotiate MTU to the smaller of this value 
				 * or the peer MRU */

	/*
	 * for forward 
	 */
	{ "forward_port", "" },	/* name:[on|off]:[tcp|udp|both]:wan_port>lan_ipaddr:lan_port0 
				 */
	{ "forward_spec", "" },	/* name:[on|off]:[tcp|udp|both]:wan_port>lan_ipaddr:lan_port0 
				 */

	{ "port_trigger", "" },	/* name:[on|off]:[tcp|udp|both]:wan_port0-wan_port1>lan_port0-lan_port1 
				 */

	/*
	 * for dynamic route 
	 */
#ifdef HAVE_DDLAN
	{ "wk_mode", "zero" },	/* Network mode [gateway|router] */
#elif HAVE_MAKSAT
#ifdef HAVE_ESR6650
	{ "wk_mode", "gateway" },	/* Network mode [gateway|router] */
#elif HAVE_HORNET
	{ "wk_mode", "gateway" },	/* Network mode [gateway|router] */
#else
	{ "wk_mode", "router" },	/* Network mode [gateway|router] */
#endif				// HAVE_ESR6650
#elif HAVE_ONNET
#ifdef HAVE_HORNET
	{ "wk_mode", "gateway" },	/* Network mode [gateway|router] */
#else
	{ "wk_mode", "router" },	/* Network mode [gateway|router] */
#endif				// HAVE_ESR6650
#else
	{ "wk_mode", "gateway" },	/* Network mode [gateway|router] */
#endif
	{ "dr_setting", "0" },	/* [ Disable | WAN | LAN | Both ] */
	{ "dr_lan_tx", "0" },	/* Dynamic-Routing LAN out */
	{ "dr_lan_rx", "0" },	/* Dynamic-Routing LAN in */
	{ "dr_wan_tx", "0" },	/* Dynamic-Routing WAN out */
	{ "dr_wan_rx", "0" },	/* Dynamic-Routing WAN in */

	/*
	 * for mac clone 
	 */
	{ "mac_clone_enable", "0" },	/* User define WAN interface MAC
					 * address */
	{ "def_hwaddr", "00:00:00:00:00:00" },	/* User define WAN interface
						 * MAC address */

	/*
	 * for mac addresses 
	 */
	{ "port_swap", "0" },	/* used to set mac addresses from et0macaddr
				 * or et1macaddr */

	/*
	 * for DDNS 
	 */
	// for dyndns
	{ "ddns_enable", "0" },	/* 0:Disable 1:dyndns 2:afraid 3:zoneedit
				 * 4:no-ip 5:custom 6:3322.org */
	{ "ddns_ssl", "0" },	/* 0:Disable 1:dyndns 2:afraid 3:zoneedit
				 * 4:no-ip 5:custom 6:3322.org */
	{ "ddns_wan_ip", "1" },
#ifdef HAVE_IPV6
	{ "ddns_ipv6", "0" },
	{ "ddns_ipv6_only", "0" },
#endif
	/*
	 * for last value 
	 */
	{ "ddns_force", "10" },

	{ "skip_amd_check", "0" },	/* 0:Disable 1:Enable */
	{ "skip_intel_check", "0" },	/* 0:Disable 1:Enable */

	{ "l2tp_use_dhcp", "1" },	/* l2tp will use dhcp to obtain ip address, netmask and gateway */
	{ "l2tp_server_ip", "" },	/* L2TP auth server (IP Address) */
	{ "l2tp_server_name", "" },	/* L2TP auth server (IP Address) */
	{ "l2tp_get_ip", "" },	/* IP Address assigned by L2TP server */
	{ "l2tp_req_chap", "1" },	/* L2TP require chap */
	{ "l2tp_ref_pap", "1" },	/* L2TP refuse pap */
	{ "l2tp_req_auth", "1" },	/* L2TP require authentication */
	{ "l2tp_encrypt", "0" },
	{ "wan_gateway_buf", "0.0.0.0" },	/* save the default gateway for DHCP */

	{ "hb_server_ip", "" },	/* heartbeat auth server (IP Address) */
	{ "hb_server_domain", "" },	/* heartbeat auth server (domain
					 * name) */
	// #ifdef HAVE_SAMBA
	{ "samba_mount", "0" },	// leave this in all versions, or will
	// produce error on info or status_router
	// pages; Eko
#ifdef HAVE_SAMBA
	{ "samba_share", "//yourserverip/yourshare" },
	{ "samba_user", "username/computer" },
	{ "samba_password", "iwer573495u7340" },
	{ "samba_script", "yourscript" },
#endif
	{ "rflow_enable", "0" },
#ifdef HAVE_ERC
	{ "info_passwd", "1" },
#else
	{ "info_passwd", "0" },
#endif
	{ "macupd_enable", "0" },
	{ "wl_radauth", "0" },
	{ "rc_startup", "" },
	{ "rc_usb", "" },
	{ "rc_firewall", "" },
	{ "rc_custom", "" },
	{ "rc_shutdown", "" },
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
#ifdef HAVE_XIOCOM
	{ "wlan0_txpwrdbm", "17" },
	{ "wlan1_txpwrdbm", "17" },
	{ "wlan2_txpwrdbm", "17" },
	{ "wlan3_txpwrdbm", "17" },
	{ "wlan4_txpwrdbm", "17" },
	{ "wlan5_txpwrdbm", "17" },
#elif defined(HAVE_GGEW) && defined(HAVE_EOC5610)
	{ "wlan0_txpwrdbm", "28" },
#elif HAVE_WZR450HP2
	{ "wlan0_txpwrdbm", "30" },
#else
#ifdef HAVE_ANTAIRA
#ifdef HAVE_HABANERO
	{ "wlan0_txpwrdbm", "25" },
	{ "wlan1_txpwrdbm", "25" },
#else
	{ "wlan0_txpwrdbm", "20" },
	{ "wlan1_txpwrdbm", "20" },
#endif
#else
	{ "wlan0_txpwrdbm", "20" },
	{ "wlan1_txpwrdbm", "20" },
	{ "wlan2_txpwrdbm", "20" },
	{ "wlan3_txpwrdbm", "20" },
	{ "wlan4_txpwrdbm", "20" },
	{ "wlan5_txpwrdbm", "20" },
#endif
#endif
#else
#ifdef HAVE_POWERNOC
	{ "wl0_txpwr", "200" },
#elif HAVE_DDLAN
	{ "wl0_txpwr", "100" },
#elif HAVE_SKYTRON
	{ "wl0_txpwr", "251" },
#elif HAVE_SAGAR
	{ "wl0_txpwr", "100" },
#else
#ifdef HAVE_RT2880
	{ "wl0_txpwr", "100" },
	{ "wl1_txpwr", "100" },
#else
	{ "wl0_txpwr", "71" },
	{ "wl1_txpwr", "71" },
	{ "wl2_txpwr", "71" },
	{ "wl0_txpwrusr", "1" },
	{ "wl1_txpwrusr", "1" },
	{ "wl2_txpwrusr", "1" },
#endif
#endif

#endif
#ifdef HAVE_GGEW
	{ "txant", "0" },
	{ "wl_antdiv", "0" },
#elif HAVE_BUFFALO
	{ "wl0_txant", "0" },
	{ "wl0_antdiv", "3" },
	{ "wl1_txant", "0" },
	{ "wl1_antdiv", "3" },
#elif HAVE_ALLNETWRT
	{ "wl0_txant", "0" },
	{ "wl0_antdiv", "0" },
#else
	{ "wl0_txant", "3" },
	{ "wl0_antdiv", "3" },
	{ "wl1_txant", "3" },
	{ "wl1_antdiv", "3" },
	{ "wl2_txant", "3" },
	{ "wl2_antdiv", "3" },
#endif

	{ "apwatchdog_enable", "0" },
	{ "apwatchdog_interval", "15" },
	{ "boot_wait", "on" },
#ifdef HAVE_SKYTEL
	{ "cron_enable", "0" },
#elif HAVE_WRK54G
	{ "cron_enable", "0" },
#else
	{ "cron_enable", "1" },
#endif
	{ "cron_jobs", "" },
	{ "dhcpd_options", "" },
	{ "dhcpd_usenvram", "0" },
	{ "dns_crypt", "0" },
	{ "dnssec", "0" },
	{ "dnssec_cu", "0" },
	{ "dnssec_proxy", "0" },
	{ "dnsmasq_no_dns_rebind", "1" },
	{ "dnsmasq_strict", "0" },
	{ "dnsmasq_rc", "0" },
	{ "dnsmasq_cachesize", "1500" },
	{ "dnsmasq_forward_max", "150" },
	{ "dnsmasq_add_mac", "0" },
	{ "dhcpd_usejffs", "0" },
#ifdef HAVE_POWERNOC_WOAP54G
	{ "dnsmasq_enable", "0" },
#elif HAVE_FON
	{ "dnsmasq_enable", "0" },
#elif HAVE_WRK54G
	{ "dnsmasq_enable", "0" },
	// #elif HAVE_ADM5120
	// {"dnsmasq_enable", "0"},
#else
	{ "dnsmasq_enable", "1" },
#endif
	{ "dnsmasq_options", "" },
#ifdef HAVE_IPV6
	{ "dns_ipv6_enable", "0" },
#endif
	// #ifdef HAVE_DDLAN
	// {"ntp_enable", "0"},
	// #else
	{ "ntp_enable", "1" },
	{ "ntp_timer", "3600" },
	// #endif
	{ "pptpd_enable", "0" },
	{ "pptpd_forcemppe", "1" },
	{ "pptpd_bcrelay", "0" },
	{ "pptpd_lip", "" },
	{ "pptpd_rip", "" },
	{ "pptpd_auth", "" },
	{ "pptpd_radius", "0" },
	{ "pptpd_radserver", "0.0.0.0" },
	{ "pptpd_radport", "1812" },
	{ "pptpd_acctport", "1813" },
	{ "pptpd_radpass", "" },
	{ "pptpd_conn", "64" },
	{ "pptp_reorder", "0" },
	{ "pptp_iptv", "0" },
	{ "pptp_extraoptions", "" },
	{ "pptpd_mtu", "1436" },
	{ "pptpd_mru", "1436" },
#ifdef HAVE_GGEW
	{ "pptp_encrypt", "1" },
#else
	{ "pptp_encrypt", "0" },
#endif
	{ "resetbutton_enable", "1" },

#ifdef HAVE_SKYTRON
	{ "telnetd_enable", "0" },
#elif HAVE_GGEW
	{ "telnetd_enable", "0" },
#elif HAVE_UNFY
	{ "telnetd_enable", "0" },
#elif HAVE_AXTEL
	{ "telnetd_enable", "0" },
#elif HAVE_WRK54G
	{ "telnetd_enable", "0" },
#elif defined(HAVE_ADM5120) && !defined(HAVE_WP54G)
	{ "telnetd_enable", "0" },
#elif defined(HAVE_NDTRADE)
	{ "telnetd_enable", "0" },
#else
	{ "telnetd_enable", "1" },
#endif
#ifdef HAVE_IPV6
	{ "ipv6_enable", "0" },
	{ "ipv6_pf_len", "64" },
	{ "ipv6_mtu", "1452" },
	{ "ipv6_tun_client_addr_pref", "64" },
	{ "ipv6_tun_upd_url", "See tunnelbroker account" },
	{ "radvd_enable", "1" },
	{ "radvd_custom", "0" },
	{ "radvd_conf", "" },
	{ "dhcp6c_custom", "0" },
	{ "dhcp6c_conf", "" },
	{ "dhcp6c_norelease", "0" },
	{ "dhcp6s_enable", "0" },
	{ "dhcp6s_seq_ips", "0" },
	{ "dhcp6s_custom", "0" },
	{ "dhcp6s_conf", "" },
#endif
#ifdef HAVE_CHILLI
	{ "chilli_net", "192.168.182.0/24" },
	{ "chilli_enable", "0" },
	{ "chilli_url", "" },
	{ "chilli_radius", "0.0.0.0" },
	{ "chilli_backup", "0.0.0.0" },
	{ "chilli_pass", "" },
	{ "chilli_dns1", "0.0.0.0" },
	{ "chilli_interface", "" },
	{ "chilli_radiusnasid", "" },
	{ "chilli_uamsecret", "" },
	{ "chilli_uamanydns", "0" },
	{ "chilli_uamallowed", "" },
	{ "chilli_macauth", "0" },
	{ "chilli_macpasswd", "password" },
	{ "chilli_802.1Xauth", "0" },
	{ "chilli_additional", "" },
#endif
#ifdef HAVE_HOTSPOT
	{ "hotss_enable", "0" },
	{ "hotss_uamenable", "0" },
	{ "hotss_loginonsplash", "0" },
	{ "hotss_customsplash", "0" },
	{ "hotss_uamallowed", "" },
	{ "hotss_operatorid", "" },
	{ "hotss_locationid", "" },
	{ "hotss_interface", "" },
	{ "hotss_net", "192.168.182.0/24" },
	{ "hotss_customuam", "" },
	{ "hotss_customuamproto", "https" },
	{ "hotss_remotekey", "" },
#endif
#ifdef HAVE_SSHD

#ifdef HAVE_SKYTEL
	{ "sshd_enable", "0" },
#elif HAVE_GGEW
	{ "sshd_enable", "1" },
#elif HAVE_XIOCOM
	{ "sshd_enable", "1" },
#elif HAVE_34TELECOM
	{ "sshd_enable", "1" },
#elif HAVE_POWERNOC
	{ "sshd_enable", "1" },
#elif HAVE_SAGAR
	{ "sshd_enable", "1" },
#elif HAVE_SKYTRON
	{ "sshd_enable", "1" },
#elif HAVE_NEWMEDIA
	{ "sshd_enable", "1" },
#elif HAVE_MAKSAT
	{ "sshd_enable", "1" },
#elif HAVE_TMK
	{ "sshd_enable", "1" },
#elif HAVE_UNFY
	{ "sshd_enable", "1" },
#elif HAVE_IDEXX
	{ "sshd_enable", "1" },
#else
	{ "sshd_enable", "0" },
#endif
	{ "sshd_forwarding", "0" },
	{ "sshd_port", "22" },
	{ "sshd_rw", "24576" },
	{ "sshd_passwd_auth", "1" },
	{ "sshd_rsa_host_key", "" },
	{ "sshd_dss_host_key", "" },
	{ "sshd_authorized_keys", "" },
#ifdef HAVE_GGEW
	{ "remote_mgt_ssh", "0" },
#elif HAVE_MAKSAT
	{ "remote_mgt_ssh", "1" },
#elif HAVE_TW6600
	{ "remote_mgt_ssh", "0" },
#else
	{ "remote_mgt_ssh", "0" },
#endif
	{ "sshd_wanport", "22" },	/* Botho 03-05-2006 : WAN port to listen on */
#endif				/* micro build - use telnet remote mgmt */
	{ "remote_mgt_telnet", "0" },
	{ "telnet_wanport", "23" },	/* WAN port to listen on */
	{ "syslogd_enable", "0" },
#ifdef HAVE_JFFS2
	{ "syslogd_jffs2", "0" },
#endif
	{ "klogd_enable", "0" },
	{ "syslogd_rem_ip", "" },
	{ "sshd_replace", "0" },

#ifdef HAVE_ONNET
	{ "tcp_congestion_control", "vegas" },
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_ATH9K)
	{ "wl0_wds1_enable", "0" },
	{ "wl0_wds2_enable", "0" },
	{ "wl0_wds3_enable", "0" },
	{ "wl0_wds4_enable", "0" },
	{ "wl0_wds5_enable", "0" },
	{ "wl0_wds6_enable", "0" },
	{ "wl0_wds7_enable", "0" },
	{ "wl0_wds8_enable", "0" },
	{ "wl0_wds9_enable", "0" },
	{ "wl0_wds10_enable", "0" },

	{ "wl0_wds1_hwaddr", "" },
	{ "wl0_wds2_hwaddr", "" },
	{ "wl0_wds3_hwaddr", "" },
	{ "wl0_wds4_hwaddr", "" },
	{ "wl0_wds5_hwaddr", "" },
	{ "wl0_wds6_hwaddr", "" },
	{ "wl0_wds7_hwaddr", "" },
	{ "wl0_wds8_hwaddr", "" },
	{ "wl0_wds9_hwaddr", "" },
	{ "wl0_wds10_hwaddr", "" },

	{ "wl0_wds1_ipaddr", "" },
	{ "wl0_wds2_ipaddr", "" },
	{ "wl0_wds3_ipaddr", "" },
	{ "wl0_wds4_ipaddr", "" },
	{ "wl0_wds5_ipaddr", "" },
	{ "wl0_wds6_ipaddr", "" },
	{ "wl0_wds7_ipaddr", "" },
	{ "wl0_wds8_ipaddr", "" },
	{ "wl0_wds9_ipaddr", "" },
	{ "wl0_wds10_ipaddr", "" },

	{ "wl0_wds1_netmask", "" },
	{ "wl0_wds2_netmask", "" },
	{ "wl0_wds3_netmask", "" },
	{ "wl0_wds4_netmask", "" },
	{ "wl0_wds5_netmask", "" },
	{ "wl0_wds6_netmask", "" },
	{ "wl0_wds7_netmask", "" },
	{ "wl0_wds8_netmask", "" },
	{ "wl0_wds9_netmask", "" },
	{ "wl0_wds10_netmask", "" },

	{ "wl0_wds1_desc", "" },
	{ "wl0_wds2_desc", "" },
	{ "wl0_wds3_desc", "" },
	{ "wl0_wds4_desc", "" },
	{ "wl0_wds5_desc", "" },
	{ "wl0_wds6_desc", "" },
	{ "wl0_wds7_desc", "" },
	{ "wl0_wds8_desc", "" },
	{ "wl0_wds9_desc", "" },
	{ "wl0_wds10_desc", "" },

	{ "wl0_wds1_ospf", "" },
	{ "wl0_wds2_ospf", "" },
	{ "wl0_wds3_ospf", "" },
	{ "wl0_wds4_ospf", "" },
	{ "wl0_wds5_ospf", "" },
	{ "wl0_wds6_ospf", "" },
	{ "wl0_wds7_ospf", "" },
	{ "wl0_wds8_ospf", "" },
	{ "wl0_wds9_ospf", "" },
	{ "wl0_wds10_ospf", "" },

	{ "wl1_wds1_enable", "0" },
	{ "wl1_wds2_enable", "0" },
	{ "wl1_wds3_enable", "0" },
	{ "wl1_wds4_enable", "0" },
	{ "wl1_wds5_enable", "0" },
	{ "wl1_wds6_enable", "0" },
	{ "wl1_wds7_enable", "0" },
	{ "wl1_wds8_enable", "0" },
	{ "wl1_wds9_enable", "0" },
	{ "wl1_wds10_enable", "0" },

	{ "wl1_wds1_hwaddr", "" },
	{ "wl1_wds2_hwaddr", "" },
	{ "wl1_wds3_hwaddr", "" },
	{ "wl1_wds4_hwaddr", "" },
	{ "wl1_wds5_hwaddr", "" },
	{ "wl1_wds6_hwaddr", "" },
	{ "wl1_wds7_hwaddr", "" },
	{ "wl1_wds8_hwaddr", "" },
	{ "wl1_wds9_hwaddr", "" },
	{ "wl1_wds10_hwaddr", "" },

	{ "wl1_wds1_ipaddr", "" },
	{ "wl1_wds2_ipaddr", "" },
	{ "wl1_wds3_ipaddr", "" },
	{ "wl1_wds4_ipaddr", "" },
	{ "wl1_wds5_ipaddr", "" },
	{ "wl1_wds6_ipaddr", "" },
	{ "wl1_wds7_ipaddr", "" },
	{ "wl1_wds8_ipaddr", "" },
	{ "wl1_wds9_ipaddr", "" },
	{ "wl1_wds10_ipaddr", "" },

	{ "wl1_wds1_netmask", "" },
	{ "wl1_wds2_netmask", "" },
	{ "wl1_wds3_netmask", "" },
	{ "wl1_wds4_netmask", "" },
	{ "wl1_wds5_netmask", "" },
	{ "wl1_wds6_netmask", "" },
	{ "wl1_wds7_netmask", "" },
	{ "wl1_wds8_netmask", "" },
	{ "wl1_wds9_netmask", "" },
	{ "wl1_wds10_netmask", "" },

	{ "wl1_wds1_desc", "" },
	{ "wl1_wds2_desc", "" },
	{ "wl1_wds3_desc", "" },
	{ "wl1_wds4_desc", "" },
	{ "wl1_wds5_desc", "" },
	{ "wl1_wds6_desc", "" },
	{ "wl1_wds7_desc", "" },
	{ "wl1_wds8_desc", "" },
	{ "wl1_wds9_desc", "" },
	{ "wl1_wds10_desc", "" },

	{ "wl1_wds1_ospf", "" },
	{ "wl1_wds2_ospf", "" },
	{ "wl1_wds3_ospf", "" },
	{ "wl1_wds4_ospf", "" },
	{ "wl1_wds5_ospf", "" },
	{ "wl1_wds6_ospf", "" },
	{ "wl1_wds7_ospf", "" },
	{ "wl1_wds8_ospf", "" },
	{ "wl1_wds9_ospf", "" },
	{ "wl1_wds10_ospf", "" },

#

#endif

	{ "wl0_br1_enable", "0" },
	{ "wl0_br1_nat", "0" },
	{ "wl1_br1_enable", "0" },
	{ "wl1_br1_nat", "0" },
#if !defined(HAVE_MADWIFI) && !defined(HAVE_ATH9K)

	{ "wl0_wds", "" },
	{ "wl0_wds0", "" },
	{ "wl0_wds1", "" },
	{ "wl0_wds2", "" },
	{ "wl0_wds3", "" },
	{ "wl0_wds4", "" },
	{ "wl0_wds5", "" },
	{ "wl0_wds6", "" },
	{ "wl0_wds7", "" },
	{ "wl0_wds8", "" },
	{ "wl0_wds9", "" },
	{ "wl1_wds", "" },
	{ "wl1_wds0", "" },
	{ "wl1_wds1", "" },
	{ "wl1_wds2", "" },
	{ "wl1_wds3", "" },
	{ "wl1_wds4", "" },
	{ "wl1_wds5", "" },
	{ "wl1_wds6", "" },
	{ "wl1_wds7", "" },
	{ "wl1_wds8", "" },
	{ "wl1_wds9", "" },
	{ "wl0_wds0_if", "" },
	{ "wl0_wds1_if", "" },
	{ "wl0_wds2_if", "" },
	{ "wl0_wds3_if", "" },
	{ "wl0_wds4_if", "" },
	{ "wl0_wds5_if", "" },
	{ "wl0_wds6_if", "" },
	{ "wl0_wds7_if", "" },
	{ "wl0_wds8_if", "" },
	{ "wl0_wds9_if", "" },
	{ "wl0_wds10_if", "" },

	{ "wl1_wds0_if", "" },
	{ "wl1_wds1_if", "" },
	{ "wl1_wds2_if", "" },
	{ "wl1_wds3_if", "" },
	{ "wl1_wds4_if", "" },
	{ "wl1_wds5_if", "" },
	{ "wl1_wds6_if", "" },
	{ "wl1_wds7_if", "" },
	{ "wl1_wds8_if", "" },
	{ "wl1_wds9_if", "" },
	{ "wl1_wds10_if", "" },

	{ "wds0.1", "" },
	{ "wds0.2", "" },
	{ "wds0.3", "" },
	{ "wds0.4", "" },
	{ "wds0.5", "" },
	{ "wds0.6", "" },
	{ "wds0.7", "" },
	{ "wds0.8", "" },
	{ "wds0.9", "" },
	{ "wds0.10", "" },
	{ "wds0.11", "" },
	{ "wds0.12", "" },
	{ "wds0.13", "" },
	{ "wds0.14", "" },
	{ "wds0.15", "" },
	{ "wds0.16", "" },

	{ "wds1.1", "" },
	{ "wds1.2", "" },
	{ "wds1.3", "" },
	{ "wds1.4", "" },
	{ "wds1.5", "" },
	{ "wds1.6", "" },
	{ "wds1.7", "" },
	{ "wds1.8", "" },
	{ "wds1.9", "" },
	{ "wds1.10", "" },
	{ "wds1.11", "" },
	{ "wds1.12", "" },
	{ "wds1.13", "" },
	{ "wds1.14", "" },
	{ "wds1.15", "" },
	{ "wds1.16", "" },
#endif

#ifdef HAVE_SKYTRON
	{ "wshaper_enable", "1" },
#else
	{ "wshaper_enable", "0" },
#endif
	{ "wshaper_dev", "WAN" },
#ifdef HAVE_SKYTRON
	{ "wshaper_downlink", "800" },
	{ "wshaper_uplink", "800" },
#else
	{ "wshaper_downlink", "0" },
	{ "wshaper_uplink", "0" },
#endif
	{ "wshaper_nopriohostsrc", "" },
	{ "wshaper_nopriohostdst", "" },
	{ "wshaper_noprioportsrc", "" },
	{ "wshaper_noprioportdst", "" },
	{ "qos_type", "0" },
	{ "svqos_svcs", "" },
	{ "svqos_ips", "" },
	{ "svqos_macs", "" },

	{ "svqos_port1bw", "FULL" },
	{ "svqos_port2bw", "FULL" },
	{ "svqos_port3bw", "FULL" },
	{ "svqos_port4bw", "FULL" },

	{ "svqos_port1prio", "10" },
	{ "svqos_port2prio", "10" },
	{ "svqos_port3prio", "10" },
	{ "svqos_port4prio", "10" },
#ifdef HAVE_SNMP
#ifdef HAVE_SAGAR
	{ "snmpd_enable", "1" },
#elif HAVE_DDLAN
	{ "snmpd_enable", "1" },
#else
	{ "snmpd_enable", "0" },
#endif
	{ "snmpd_syslocation", "Unknown" },
	{ "snmpd_syscontact", "root" },
#ifdef HAVE_BRANDING
	{ "snmpd_sysname", "anonymous" },
#elif defined(HAVE_TRIMAX)
	{ "snmpd_sysname", "m2m" },
#elif defined(HAVE_WIKINGS)
	{ "snmpd_sysname", "Excel Networks" },
#elif defined(HAVE_ESPOD)
	{ "snmpd_sysname", "ESPOD Technologies" },
#elif defined(HAVE_NEXTMEDIA)
	{ "snmpd_sysname", "nextmedia" },
#elif defined(HAVE_KORENRON)
	{ "snmpd_sysname", "KORENRON" },
#elif defined(HAVE_HOBBIT)
	{ "snmpd_sysname", "HQ-NDS" },
#else
	{ "snmpd_sysname", "dd-wrt" },
#endif
	{ "snmpd_rocommunity", "public" },
	{ "snmpd_rwcommunity", "private" },
	{ "snmpd_conf",
	 "See http://www.net-snmp.org for expert snmpd.conf options" },
#endif
	{ "wol_enable", "0" },
	{ "wol_interval", "86400" },
	{ "wol_hostname", "" },
	{ "wol_macs", "" },
	{ "wol_passwd", "" },

	{ "hs_enable", "" },
	{ "hs_exempt", "" },
	{ "hs_urls", "" },
	{ "hs_redirect", "" },
	{ "hs_html", "" },
	{ "hs_image", "" },

	{ "def_whwaddr", "00:00:00:00:00:00" },	/* User define wireless
						 * interface MAC address */

	{ "sv_restore_defaults", "0" },	// fix for vlan stuff side effects

	{ "ospfd_conf", "" },
	{ "zebra_conf", "" },
	{ "ospfd_copt", "0" },
	{ "zebra_copt", "0" },
	{ "zebra_log", "0" },
	{ "dyn_default", "0" },

	{ "altdns1", "" },
	{ "altdns2", "" },
	{ "altdns3", "" },

	{ "log_accepted", "0" },	/* 0:Disable 1:Eanble */
	{ "log_dropped", "0" },	/* 0:Disable 1:Eanble */
	{ "log_rejected", "0" },	/* 0:Disable 1:Eanble */

	/*
	 * start lonewolf mods 
	 */
	{ "vlans", "0" },
	{ "trunking", "0" },
	/*
	 * end lonewolf mods 
	 */
	// DD-WRT start
	{ "manual_boot_nv", "0" },
#ifdef HAVE_WTS
	{ "status_auth", "0" },
#elif HAVE_ERC
	{ "status_auth", "0" },
#elif HAVE_IPR
	{ "status_auth", "0" },
#else
	{ "status_auth", "1" },
#endif
#ifdef HAVE_UNFY
	{ "enable_jffs2", "1" },
	{ "clean_jffs2", "1" },
#else
	{ "enable_jffs2", "0" },
	{ "clean_jffs2", "0" },
#endif
#ifdef HAVE_KAID
	{ "kaid_enable", "0" },
	{ "kaid_macs", "" },
	{ "kaid_uibind", "34522" },
	{ "kaid_orbport", "34525" },
	{ "kaid_orbdeepport", "34523" },
#endif
#ifdef HAVE_WTS
	{ "language", "spanish" },
#elif defined(DEFAULT_LANGUAGE)
	{ "language", DEFAULT_LANGUAGE },
#else
	{ "language", "english" },
#endif
	{ "macupd_ip", "0.0.0.0" },
	{ "macupd_port", "2056" },
	{ "macupd_interval", "10" },
	{ "mmc_enable", "0" },
	{ "mmc_enable0", "0" },
#ifdef HAVE_MMC
	{ "mmc_gpio", "0" },
	{ "mmc_di", "0" },
	{ "mmc_do", "0" },
	{ "mmc_clk", "0" },
	{ "mmc_cs", "0" },
#endif
#ifdef HAVE_RB500
	{ "ip_conntrack_max", "16384" },
#elif HAVE_WRT300NV2
	{ "ip_conntrack_max", "4096" },
#elif HAVE_XSCALE
	{ "ip_conntrack_max", "16384" },
#elif HAVE_X86
#ifdef HAVE_NOWIFI
	{ "ip_conntrack_max", "4096" },
#else
	{ "ip_conntrack_max", "32768" },
#endif
#elif HAVE_MAGICBOX
	{ "ip_conntrack_max", "16384" },
#elif HAVE_NORTHSTAR
	{ "ip_conntrack_max", "32768" },
#elif HAVE_MVEBU
	{ "ip_conntrack_max", "32768" },
#elif HAVE_IPQ806X
	{ "ip_conntrack_max", "32768" },
#elif HAVE_LAGUNA
	{ "ip_conntrack_max", "32768" },
#elif HAVE_RB600
	{ "ip_conntrack_max", "32768" },
#elif HAVE_MERAKI
	{ "ip_conntrack_max", "16384" },
#elif HAVE_FONERA
	{ "ip_conntrack_max", "4096" },
#elif HAVE_BUFFALO
	{ "ip_conntrack_max", "4096" },
#elif HAVE_LS2
	{ "ip_conntrack_max", "4096" },
#elif HAVE_LS5
	{ "ip_conntrack_max", "4096" },
#elif HAVE_WHRAG108
	{ "ip_conntrack_max", "16384" },
#elif HAVE_TW6600
	{ "ip_conntrack_max", "4096" },
#elif HAVE_CA8
	{ "ip_conntrack_max", "16384" },
#elif HAVE_MICRO
	{ "ip_conntrack_max", "1024" },
#else
	{ "ip_conntrack_max", "4096" },
#endif
#ifdef HAVE_80211AC
#ifndef HAVE_BUFFALO
	{ "wl_regdomain", "UNITED_STATES" },
	{ "wl0_country_code", "US" },
	{ "wl1_country_code", "US" },
	{ "wl0_country_rev", "0" },
	{ "wl1_country_rev", "0" },
#elif HAVE_HOBBIT
	{ "wl_regdomain", "EUROPE" },
#endif
#endif
#ifdef HAVE_MICRO
	{ "ip_conntrack_tcp_timeouts", "300" },
#else
	{ "ip_conntrack_tcp_timeouts", "3600" },
#endif
	{ "ip_conntrack_udp_timeouts", "120" },
	{ "rflow_ip", "0.0.0.0" },
	{ "rflow_port", "2055" },
	{ "rflow_if", "br0" },
#ifdef HAVE_PPPOERELAY
#ifdef HAVE_DDLAN
	{ "pppoerelay_enable", "1" },
#else
	{ "pppoerelay_enable", "0" },
#endif
#endif
	{ "schedule_enable", "0" },
	{ "schedule_time", "3600" },
	{ "schedule_hour_time", "1" },
	{ "schedule_minutes", "0" },
	{ "schedule_hours", "0" },
	{ "schedule_weekdays", "00" },
	{ "smtp_redirect_enable", "0" },
	{ "smtp_redirect_destination", "0.0.0.0" },
	{ "smtp_source_network", "0.0.0.0" },
	{ "wds_watchdog_enable", "0" },
	{ "wds_watchdog_interval_sec", "1000" },
	{ "wds_watchdog_ips", "" },
	{ "wds_watchdog_mode", "0" },
	{ "wds_watchdog_timeout", "10" },
	{ "dhcpfwd_enable", "0" },
	{ "dhcpfwd_ip", "0.0.0.0" },
#ifdef HAVE_NODOG
	{ "ND_enable", "0" },
#ifdef HAVE_BRANDING
	{ "ND_GatewayName", "GATEWAY" },
#else
	{ "ND_GatewayName", "DD-WRT" },
#endif
#ifdef HAVE_BRANDING
	{ "ND_HomePage", "" },
#else
	{ "ND_HomePage", "http://www.dd-wrt.com" },
#endif
	{ "ND_ExcludePorts", "" },
	{ "ND_IncludePorts", "" },
	{ "ND_LoginTimeout", "120" },
#ifdef HAVE_BRANDING
	{ "ND_AllowedWebHosts", "google.com" },
#else
	{ "ND_AllowedWebHosts", "" },
#endif
#ifdef HAVE_RAMSKOV
	{ "ND_DocumentRoot", "/tmp" },
#else
	{ "ND_DocumentRoot", "/www" },
#endif
	{ "ND_MACWhiteList", "" },
	{ "ND_MaxClients", "250" },
	{ "ND_GatewayPort", "2050" },
	{ "ND_ForcedRedirect", "0" },
	{ "ND_IdleTimeout", "0" },
	{ "ND_PreAuthIdleTimeout", "30" },
	{ "ND_CheckInterval", "600" },
	{ "ND_GatewayIPRange_mask","0"},
	{ "ND_GatewayIPRange","0.0.0.0"},
	{ "ND_dl", "0" },
	{ "ND_ul", "0" },
#else
	{ "NC_enable", "0" },
#ifdef HAVE_BRANDING
	{ "NC_GatewayName", "GATEWAY" },
#else
	{ "NC_GatewayName", "DD-WRT" },
#endif
#ifdef HAVE_BRANDING
	{ "NC_HomePage", "" },
#else
	{ "NC_HomePage", "http://www.dd-wrt.com" },
#endif
	{ "NC_ExcludePorts", "25" },
	{ "NC_IncludePorts", "" },
	{ "NC_Verbosity", "2" },
	{ "NC_LoginTimeout", "86400" },
#ifdef HAVE_BRANDING
	{ "NC_AllowedWebHosts", "google.com" },
#else
	{ "NC_AllowedWebHosts", "" },
#endif
#ifdef HAVE_RAMSKOV
	{ "NC_RouteOnly", "0" },
	{ "NC_DocumentRoot", "/tmp" },
#else
	{ "NC_RouteOnly", "0" },
	{ "NC_DocumentRoot", "/www" },
#endif
	{ "NC_SplashURL", "" },
	{ "NC_SplashURLTimeout", "21600" },
	{ "NC_MACWhiteList", "" },
	{ "NC_GatewayPort", "5280" },
	{ "NC_GatewayMode", "Open" },
	{ "NC_extifname", "auto" },
	{ "NC_ForcedRedirect", "0" },
	{ "NC_IdleTimeout", "0" },
	{ "NC_MaxMissedARP", "5" },
	{ "NC_RenewTimeout", "0" },

#endif
	{ "wl_wme", "on" },	/* WME mode (off|on) */
	{ "wl1_wme", "on" },	/* WME mode (off|on) */
	/*
	 * WME parameters 
	 */
	/*
	 * EDCA parameters for STA 
	 */
	{ "wl_wme_sta_bk", "15 1023 7 0 0 off off" },	/* WME STA AC_BK paramters */
	{ "wl_wme_sta_be", "15 1023 3 0 0 off off" },	/* WME STA AC_BE paramters */
	{ "wl_wme_sta_vi", "7 15 2 6016 3008 off off" },	/* WME STA AC_VI
								 * paramters */
	{ "wl_wme_sta_vo", "3 7 2 3264 1504 off off" },	/* WME STA AC_VO
							 * paramters */

	/*
	 * EDCA parameters for AP 
	 */
	{ "wl_wme_ap_bk", "15 1023 7 0 0 off off" },	/* WME AP AC_BK paramters */
	{ "wl_wme_ap_be", "15 63 3 0 0 off off" },	/* WME AP AC_BE paramters */
	{ "wl_wme_ap_vi", "7 15 1 6016 3008 off off" },	/* WME AP AC_VI
							 * paramters */
	{ "wl_wme_ap_vo", "3 7 1 3264 1504 off off" },	/* WME AP AC_VO paramters */
	{ "wl_wme_no_ack", "off" },	/* WME No-Acknowledgmen mode */
	{ "wl_wme_apsd", "on" },	/* WME APSD mode */

	{ "wl1_wme_sta_bk", "15 1023 7 0 0 off off" },	/* WME STA AC_BK paramters */
	{ "wl1_wme_sta_be", "15 1023 3 0 0 off off" },	/* WME STA AC_BE paramters */
	{ "wl1_wme_sta_vi", "7 15 2 6016 3008 off off" },	/* WME STA AC_VI                                                         * paramters */
	{ "wl1_wme_sta_vo", "3 7 2 3264 1504 off off" },	/* WME STA AC_VO
								 * paramters */
	{ "wl2_wme_sta_bk", "15 1023 7 0 0 off off" },	/* WME STA AC_BK paramters */
	{ "wl2_wme_sta_be", "15 1023 3 0 0 off off" },	/* WME STA AC_BE paramters */
	{ "wl2_wme_sta_vi", "7 15 2 6016 3008 off off" },	/* WME STA AC_VI                                                         * paramters */
	{ "wl2_wme_sta_vo", "3 7 2 3264 1504 off off" },	/* WME STA AC_VO
								 * paramters */

	/*
	 * EDCA parameters for AP 
	 */
	{ "wl1_wme_ap_bk", "15 1023 7 0 0 off off" },	/* WME AP AC_BK paramters */
	{ "wl1_wme_ap_be", "15 63 3 0 0 off off" },	/* WME AP AC_BE paramters */
	{ "wl1_wme_ap_vi", "7 15 1 6016 3008 off off" },	/* WME AP AC_VI                                                  * paramters */
	{ "wl1_wme_ap_vo", "3 7 1 3264 1504 off off" },	/* WME AP AC_VO
							 * paramters */
	{ "wl1_wme_no_ack", "off" },	/* WME No-Acknowledgmen mode */
	{ "wl1_wme_apsd", "on" },	/* WME APSD mode */

	{ "wl2_wme_ap_bk", "15 1023 7 0 0 off off" },	/* WME AP AC_BK paramters */
	{ "wl2_wme_ap_be", "15 63 3 0 0 off off" },	/* WME AP AC_BE paramters */
	{ "wl2_wme_ap_vi", "7 15 1 6016 3008 off off" },	/* WME AP AC_VI                                                  * paramters */
	{ "wl2_wme_ap_vo", "3 7 1 3264 1504 off off" },	/* WME AP AC_VO
							 * paramters */
	{ "wl2_wme_no_ack", "off" },	/* WME No-Acknowledgmen mode */
	{ "wl2_wme_apsd", "on" },	/* WME APSD mode */

	{ "wl_maxassoc", "128" },	/* Max associations driver could support */
	{ "wl0_maxassoc", "128" },	/* Max associations driver could support */
	{ "wl1_maxassoc", "128" },	/* Max associations driver could support */
	{ "wl2_maxassoc", "128" },	/* Max associations driver could support */

	/* Per AC Tx parameters */

	{ "wl_wme_txp_be", "7 3 4 2 0" },	/* WME AC_BE Tx parameters */
	{ "wl_wme_txp_bk", "7 3 4 2 0" },	/* WME AC_BK Tx parameters */
	{ "wl_wme_txp_vi", "7 3 4 2 0" },	/* WME AC_VI Tx parameters */
	{ "wl_wme_txp_vo", "7 3 4 2 0" },	/* WME AC_VO Tx parameters */

	{ "wl1_wme_txp_be", "7 3 4 2 0" },	/* WME AC_BE Tx parameters */
	{ "wl1_wme_txp_bk", "7 3 4 2 0" },	/* WME AC_BK Tx parameters */
	{ "wl1_wme_txp_vi", "7 3 4 2 0" },	/* WME AC_VI Tx parameters */
	{ "wl1_wme_txp_vo", "7 3 4 2 0" },	/* WME AC_VO Tx parameters */

	{ "wl2_wme_txp_be", "7 3 4 2 0" },	/* WME AC_BE Tx parameters */
	{ "wl2_wme_txp_bk", "7 3 4 2 0" },	/* WME AC_BK Tx parameters */
	{ "wl2_wme_txp_vi", "7 3 4 2 0" },	/* WME AC_VI Tx parameters */
	{ "wl2_wme_txp_vo", "7 3 4 2 0" },	/* WME AC_VO Tx parameters */

#ifdef HAVE_ZEROIP
	{ "shat_enable", "0" },
	{ "shat_range", "192.168.1.79+20" },
	{ "shat_shield", "" },
#endif
#ifdef HAVE_IDEXX
	{ "dns_dnsmasq", "0" },
#else
	{ "dns_dnsmasq", "1" },
#endif
#ifdef HAVE_IDEXX
	{ "auth_dnsmasq", "1" },
#else
	{ "auth_dnsmasq", "1" },
#endif
	{ "dns_redirect", "0" },
#ifdef HAVE_GGEW
	{ "ral", "217.113.177.185 172.16.0.0/28" },
	{ "pptp_use_dhcp", "1" },	/* pptp will use dhcp to obtain ip address, netmask and gateway */
	{ "pptp_server_name", "proxy2.wlan.ggew-net.de" },
#else
	{ "pptp_use_dhcp", "1" },	/* pptp will use dhcp to obtain ip address, netmask and gateway */
	{ "pptp_server_name", "" },
#endif

	{ "forward_entries", "0" },
	{ "forwardspec_entries", "0" },
	{ "forwardip_entries", "0" },
	{ "trigger_entries", "0" },
#ifdef HAVE_SKYTRON
	{ "sip_port", "5060" },
	{ "sip_domain", "sip.skytron.de" },
#else
	{ "sip_port", "5060" },	/* MILKFISH SETTING */// this setting is not evaluated/used by the
	// milkfish scripts for milkfish-dd 1.0 -
	// fs070712
	{ "sip_domain", "192.168.1.1" },	/* MILKFISH SETTING */// set "192.168.1.1" as default - not 
	// 
	// 
	// setting at all disables dbtextctl
	// script - fs070712
#endif
#ifdef HAVE_AQOS
	{ "default_uplevel", "100000" },	// set a useful value to prevent
	// deadlock
	{ "default_downlevel", "100000" },	// set a useful value to prevent
	// deadlock
#endif
	{ "static_leases", "" },
	{ "static_leasenum", "0" },
	{ "dhcpc_vendorclass", "" },	// vendor class id for client
	{ "dhcpc_121", "1" },
	// (optional)
	{ "dhcpc_requestip", "" },	// request ip (optional)
#ifdef HAVE_DDLAN
	{ "maskmac", "0" },
#else
	{ "maskmac", "1" },
#endif
#ifdef HAVE_OPENVPN
	{ "openvpncl_enable", "0" },
	{ "openvpncl_mit", "1" },
	{ "openvpncl_remoteip", "0.0.0.0" },
	{ "openvpncl_remoteport", "1194" },
	{ "openvpncl_ca", "" },
	{ "openvpncl_client", "" },
	{ "openvpncl_key", "" },
	{ "openvpncl_static", "" },
	{ "openvpncl_pkcs12", "" },
	{ "openvpncl_pkcs", "0" },
	{ "openvpncl_lzo", "off" },
	{ "openvpncl_proto", "udp4" },
	{ "openvpncl_mtu", "1400" },
	{ "openvpncl_mssfix", "0" },
	{ "openvpncl_fragment", "" },
	{ "openvpncl_certtype", "0" },
	{ "openvpncl_tuntap", "tun" },
	{ "openvpncl_nat", "1" },
	{ "openvpncl_config", "" },
	{ "openvpncl_tlsauth", "" },
	{ "openvpncl_cipher", "" },
	{ "openvpncl_auth", "" },
	{ "openvpncl_adv", "0" },
	{ "openvpncl_bridge", "0" },
	{ "openvpncl_tlscip", "0" },
	{ "openvpncl_route", "" },
	{ "openvpncl_ip", "" },
	{ "openvpncl_mask", "" },
	{ "openvpncl_scramble", "off" },
	{ "openvpncl_scrmblpw", "o54a72ReutDK" },
	{ "openvpncl_tls_btn", "3" },
	{ "openvpncl_fw", "1" },
	{ "openvpncl_dc1", "CHACHA20-POLY1305" },
	{ "openvpncl_dc2", "AES-128-GCM" },
	{ "openvpncl_dc3", "AES-256-GCM" },
	{ "openvpncl_killswitch", "0" },
	{ "openvpncl_spbr", "0" },
	{ "openvpncl_splitdns", "0" },
	{ "openvpncl_upauth", "0" },
	{ "openvpncl_multirem", "0" },
	{ "openvpncl_randomsrv", "0" },
	{ "openvpncl_blockmulticast", "0" },
	{ "openvpncl_wdog", "0" },
	{ "openvpncl_wdog_pingip", "8.8.8.8" },
	{ "openvpncl_wdog_sleept", "30" },
	{ "openvpn_enable", "0" },
	{ "openvpn_mit", "1" },
	{ "openvpn_net", "10.8.0.0" },
#ifdef HAVE_IPV6
	{ "openvpn_v6netmask", "fddb:c50f:f9bc:4ba5::/64" },
#endif
	{ "openvpn_mask", "0.0.0.0" },
	{ "openvpn_tunmask", "255.255.255.0" },
	{ "openvpn_gateway", "0.0.0.0" },
	{ "openvpn_startip", "0.0.0.0" },
	{ "openvpn_endip", "0.0.0.0" },
	{ "openvpn_port", "1194" },
	{ "openvpn_ca", "" },
	{ "openvpn_crl", "" },
	{ "openvpn_crt", "" },
	{ "openvpn_client", "" },	//old and invalid just to be compatible to old config
	{ "openvpn_key", "" },
	{ "openvpn_pkcs12", "" },
	{ "openvpn_static", "" },
	{ "openvpn_pkcs", "0" },
	{ "openvpn_lzo", "off" },
	{ "openvpn_proto", "udp4" },
	{ "openvpn_mtu", "1400" },
	{ "openvpn_mssfix", "0" },
	{ "openvpn_fragment", "" },
	{ "openvpn_config", "" },
	{ "openvpn_dh", "" },
	{ "openvpn_tlsauth", "" },
	{ "openvpn_tuntap", "tun" },
	{ "openvpn_cl2cl", "1" },
	{ "openvpn_dupcn", "0" },
	{ "openvpn_onwan", "0" },
	{ "openvpn_switch", "1" },	//switch between old/new style config
	{ "openvpn_cipher", "" },
	{ "openvpn_auth", "" },
	{ "openvpn_redirgate", "1" },
	{ "openvpn_adv", "0" },
	{ "openvpn_tlscip", "0" },
	{ "openvpn_proxy", "0" },
	{ "openvpn_clcon", "" },
	{ "openvpn_cldiscon", "" },
	{ "openvpn_ccddef", "" },
	{ "openvpn_dhcpbl", "0" },
	{ "openvpn_scramble", "off" },
	{ "openvpn_scrmblpw", "o54a72ReutDK" },
	{ "openvpn_tls_btn", "3" },
	{ "openvpn_dc1", "CHACHA20-POLY1305" },
	{ "openvpn_dc2", "AES-128-GCM" },
	{ "openvpn_dc3", "AES-256-GCM" },
	{ "openvpn_dh_btn", "1" },
	{ "openvpn_allowcnwan", "1" },
	{ "openvpn_allowcnlan", "1" },
	{ "openvpn_blockmulticast", "0" },
	{ "openvpn_userpassnum", "0" },
	{ "openvpn_enuserpass", "0" },

#endif
#ifdef HAVE_ANTAIRA_AGENT
	{ "antaira_agent_enable", "0" },
	{ "antaira_agent_retry_min", "5" },
	{ "antaira_agent_retry_max", "10" },
	{ "antaira_agent_cloud_url", "https://vpn.antaira.com:8443" },
	{ "antaira_agent_connect_url", "/v2/connect" },
	{ "antaira_agent_configuration_url", "/v2/configuration" },
	{ "antaira_agent_wan_ifname", "br0" },
	{ "chronyd_enable", "0" },
#endif
#ifdef HAVE_CHRONY
        { "chronyd_enable", "0" },
#endif
#ifdef HAVE_KODATA
	{ "newhttp_username", "bJ/GddyoJuiU2" },
	{ "newhttp_passwd", "bJDLObifZlIRQ" },
#endif
#ifdef HAVE_34TELECOM
	{ "newhttp_passwd", "hdslklas9a" },
#endif
#ifdef HAVE_ERC
	{ "newhttp_passwd", "$1$2XwGZVRQ$H35EZ6yHCEZiG42sY1QSJ1" },
//      {"newhttp_passwd", "$1$.V44ffYt$6ttOdlItuYV6uvi..vvoO/"},
#endif
#ifdef HAVE_CARLSONWIRELESS
	{ "newhttp_username", "$1$y5qEiLaV$/2cQErs8qxs./J3pl2l2F." },	/* HTTP username) */
	{ "newhttp_passwd", "$1$y5qEiLaV$KNvLd5jrLCfYko/e6e7lZ1" },	/* HTTP password) */
#endif
#ifdef HAVE_IPR
	{ "newhttp_passwd", "$1$hFOmcfz/$eYEVGPdzfrkGcA6MbUukF." },
#endif
#ifdef HAVE_MVEBU
	{ "wlan0_regdomain", "UNITED_STATES" },
	{ "wlan1_regdomain", "UNITED_STATES" },
	{ "wlan0_txpwrdbm", "30" },
	{ "wlan1_txpwrdbm", "30" },
#endif
#ifdef HAVE_SPUTNIK_APD

#ifdef HAVE_SPUTNIK
	{ "sputnik_mjid_type", "0" },
	{ "sputnik_mjid", "sputnik@wifi.sputnik.com" },
#if defined(HAVE_XSCALE) || defined(HAVE_X86)
	{ "sputnik_mode", "pro" },
#else
	{ "sputnik_mode", "standard" },
#endif
	{ "sputnik_done", "0" },
	{ "sputnik_rereg", "1" },
	{ "apd_enable", "1" },
#else
	{ "sputnik_mjid_type", "0" },
	{ "sputnik_mjid", "" },
	{ "sputnik_mode", "standard" },
	{ "sputnik_done", "0" },
	{ "apd_enable", "0" },
#endif
#endif
#ifdef HAVE_FONERA
	{ "upgrade_delay", "1200" },
#elif HAVE_MERAKI
	{ "upgrade_delay", "600" },
#elif HAVE_LS2
	{ "upgrade_delay", "600" },
#else
	{ "upgrade_delay", "300" },
#endif
#ifdef HAVE_WIFIDOG
	{ "wd_enable", "0" },
	{ "wd_gwid", "default" },
	{ "wd_url", "" },
	{ "wd_gwport", "2060" },
	{ "wd_httpdname", "WiFiDog" },
	{ "wd_httpdcon", "10" },
	{ "wd_interval", "60" },
	{ "wd_timeout", "5" },
	{ "wd_maclist", "" },
	{ "wd_hostname", "" },
	{ "wd_sslavailable", "0" },
	{ "wd_sslport", "443" },
	{ "wd_httpport", "80" },
	{ "wd_path", "/wifidog" },
	{ "wd_auth", "0" },
	{ "wd_username", "" },
	{ "wd_password", "" },
	{ "wd_iface", "" },
	{ "wd_extiface", "" },
#endif

#ifdef HAVE_CHILLI
#ifdef HAVE_CHILLILOCAL
	{ "fon_usernames", "0" },
	{ "fon_userlist", "" },
#endif
#endif
	{ "fon_enable", "0" },
	{ "pptpd_client_enable", "" },
	{ "pptpd_client_srvip", "" },
	{ "pptpd_client_srvsub", "" },
	{ "pptpd_client_srvsubmsk", "" },
	{ "pptpd_client_srvuser", "" },
	{ "pptpd_client_srvpass", "" },
	{ "pptpd_client_ipparam", "" },
	{ "pptpd_client_mtu", "1436" },
	{ "pptpd_client_mru", "1436" },
#if defined(HAVE_AOSS) || defined(HAVE_WPS)
	{ "radiooff_button", "2" },
	{ "radiooff_boot_off", "0" },
#else
	{ "radiooff_button", "0" },
	{ "radiooff_boot_off", "0" },
#endif
	{ "radio0_on_time", "111111111111111111111111" },	/* Radio timer,always on */
	{ "radio0_timer_enable", "0" },
	{ "radio1_on_time", "111111111111111111111111" },	/* Radio timer,always on */
	{ "radio1_timer_enable", "0" },
	{ "radio2_on_time", "111111111111111111111111" },	/* Radio timer,always on */
	{ "radio2_timer_enable", "0" },
#ifdef HAVE_CPUTEMP
#ifdef HAVE_MVEBU
	{ "hwmon_temp_max", "65" },
	{ "hwmon_temp_hyst", "60" },
#else
	{ "hwmon_temp_max", "60" },
	{ "hwmon_temp_hyst", "50" },
#endif
#endif
#ifdef HAVE_RSTATS
	{ "rstats_enable", "0" },
	{ "rstats_path", "" },
	{ "rstats_stime", "48" },
	{ "rstats_data", "" },
#endif
#ifdef HAVE_NSTX
	{ "nstxd_enable", "0" },
	{ "nstx_ipenable", "0" },
	{ "nstx_ip", "0.0.0.0" },
	{ "nstx_log", "0" },
#endif
#ifdef HAVE_PORTSETUP
	{ "br0_mcast", "0" },	/* IGMP Snooping enabled */
	{ "br1_mcast", "0" },	/* IGMP Snooping enabled */

	{ "eth0_bridged", "1" },	/* Service set ID (network name) */
	{ "eth0_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth0_netmask", "0.0.0.0" },	/* Service set ID (network name) */

	{ "eth1_bridged", "1" },	/* Service set ID (network name) */
	{ "eth1_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth1_netmask", "0.0.0.0" },	/* Service set ID (network name) */

	{ "eth2_bridged", "1" },	/* Service set ID (network name) */
	{ "eth2_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth2_netmask", "0.0.0.0" },	/* Service set ID (network name) */

	{ "eth3_bridged", "1" },	/* Service set ID (network name) */
	{ "eth3_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth3_netmask", "0.0.0.0" },	/* Service set ID (network name) */

	{ "eth4_bridged", "1" },	/* Service set ID (network name) */
	{ "eth4_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth4_netmask", "0.0.0.0" },	/* Service set ID (network name) */

	{ "eth5_bridged", "1" },	/* Service set ID (network name) */
	{ "eth5_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth5_netmask", "0.0.0.0" },	/* Service set ID (network name) */

	{ "eth6_bridged", "1" },	/* Service set ID (network name) */
	{ "eth6_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth6_netmask", "0.0.0.0" },	/* Service set ID (network name) */

	{ "eth7_bridged", "1" },	/* Service set ID (network name) */
	{ "eth7_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth7_netmask", "0.0.0.0" },	/* Service set ID (network name) */

	{ "eth8_bridged", "1" },	/* Service set ID (network name) */
	{ "eth8_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth8_netmask", "0.0.0.0" },	/* Service set ID (network name) */

	{ "eth9_bridged", "1" },	/* Service set ID (network name) */
	{ "eth9_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth9_netmask", "0.0.0.0" },	/* Service set ID (network name) */

	{ "eth10_bridged", "1" },	/* Service set ID (network name) */
	{ "eth10_ipaddr", "0.0.0.0" },	/* Service set ID (network name) */
	{ "eth10_netmask", "0.0.0.0" },	/* Service set ID (network name) */
#endif
#ifdef HAVE_EOP
	{ "oet_tunnels", "0" },
#endif
	{ "wifi_bonding", "0" },
#ifdef HAVE_RADLOCAL
	{ "iradius_enable", "0" },
#endif
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
	{ "wifi_display", "wlan0" },
#else
	{ "wifi_display", "wl0" },
#endif
#ifdef HAVE_USB
	{ "usb_enable", "0" },
	{ "usb_uhci", "0" },
	{ "usb_ohci", "0" },
	{ "usb_usb2", "0" },
	{ "usb_storage", "0" },
	{ "usb_printer", "0" },
	{ "usb_automnt", "0" },
	{ "usb_ses_umount", "0" },
	{ "usb_mntpoint", "mnt" },
	{ "usb_runonmount", "" },
#endif
#ifdef HAVE_UNFY
	{ "ttraff_enable", "0" },
#else
	{ "ttraff_enable", "1" },
#endif
	{ "ttraff_iface", "" },
#ifdef HAVE_PPPOESERVER
	{ "pppoeserver_enabled", "0" },
	{ "pppoeserver_interface", "br0" },
	{ "pppoeradius_enabled", "0" },
	{ "pppoeserver_bsdcomp", "0" },
	{ "pppoeserver_deflate", "0" },
	{ "pppoeserver_lzs", "0" },
	{ "pppoeserver_mppc", "0" },
	{ "pppoeserver_encryption", "0" },
	{ "pppoeserver_lcpechoint", "5" },
	{ "pppoeserver_lcpechofail", "12" },
	{ "pppoeserver_sessionlimit", "0" },
	{ "pppoeserver_chaps", "" },
	{ "pppoeserver_chapsnum", "0" },
	{ "pppoeserver_idle", "0" },
	{ "pppoeserver_authserverip", "192.168.1.1" },
	{ "pppoeserver_authserverip_backup", "" },
	{ "pppoeserver_authserverport", "1812" },
	{ "pppoeserver_authserverport_backup", "1812" },
	{ "pppoeserver_acctserverport", "1813" },
	{ "pppoeserver_acctserverport_backup", "1813" },
	{ "pppoeserver_sharedkey", "" },
	{ "pppoeserver_sharedkey_backup", "" },
	{ "pppoeserver_pool", "192.168.1.100" },
	{ "pppoeserver_clip", "local" },
	{ "pppoeserver_clcount", "64" },
	{ "pppoeserver_mtu", "1492" },
	{ "pppoeserver_mru", "1492" },
#endif
#ifdef HAVE_MILKFISH
	{ "milkfish_enabled", "0" },	/* MILKFISH enable=1|disable=0 */
	{ "openser_cfg", "/var/openser/milkfish_openser.cfg" },	/* MILKFISH
								 * SETTING */
	{ "milkfish_fromdomain", "" },	/* MILKFISH SETTING */
	{ "milkfish_fromswitch", "off" },	/* MILKFISH SETTING */
	{ "milkfish_username", "" },	/* MILKFISH SETTING */
	{ "milkfish_password", "" },	/* MILKFISH SETTING */
	{ "milkfish_routerid", "" },	/* MILKFISH SETTING */
	{ "milkfish_ppptime", "off" },	/* MILKFISH SETTING - keep always
					 * "off" on dd-wrt ! */
	{ "milkfish_audit", "off" },	/* MILKFISH SETTING */
	{ "milkfish_dynsip", "off" },	/* MILKFISH SETTING */
	{ "milkfish_siptrace", "off" },	/* MILKFISH SETTING */
	{ "milkfish_ddsubscribers", "" },	/* MILKFISH SETTING */
	{ "milkfish_ddsubscribersnum", "0" },	/* MILKFISH SETTING */
	{ "milkfish_ddaliases", "" },	/* MILKFISH SETTING */
	{ "milkfish_ddaliasesnum", "0" },	/* MILKFISH SETTING */
	{ "milkfish_ddactive", "" },	/* MILKFISH SETTING */
	{ "milkfish_ddactivenum", "0" },	/* MILKFISH SETTING */
	{ "milkfish_dsusername", "" },	/* MILKFISH SETTING */
	{ "milkfish_dspassword", "" },	/* MILKFISH SETTING */
#endif
#ifdef HAVE_OLSRD
	{ "olsrd_pollsize", "0.1" },
	{ "olsrd_redundancy", "2" },
	{ "olsrd_coverage", "7" },
	{ "olsrd_gateway", "0" },
	{ "olsrd_lqfisheye", "1" },
	{ "olsrd_lqaging", "0.1" },
	{ "olsrd_lqdijkstramin", "0" },
	{ "olsrd_lqdijkstramax", "5.0" },
	{ "olsrd_lqlevel", "2" },
	{ "olsrd_hysteresis", "0" },
	{ "olsrd_smartgw", "0" },
#endif
	{ "reconnect_enable", "0" },
	{ "reconnect_hours", "0" },
	{ "reconnect_minutes", "0" },
/*	{"af_enable", "0"},
	{"af_email", ""},
	{"af_ssid", "0"},
	{"af_ssid_name", "AnchorFree WiFi"},
	{"af_address", ""},
	{"af_address_2", ""},
	{"af_city", ""},
	{"af_zip", ""},
	{"af_state", ""},
	{"af_country", ""},
	{"af_category", "0"},
	{"af_publish", "1"},
	{"af_agree", "0"},*/
#ifdef HAVE_WAVESAT
	{ "ofdm_mode", "disabled" },
	{ "ofdm_upstream", "3525000" },
	{ "ofdm_downstream", "3450000" },
	{ "ofdm_width", "7" },
	{ "ofdm_duplex", "TDD" },
#endif
#ifdef HAVE_FTP
	{ "proftpd_enable", "0" },
	{ "proftpd_wan", "0" },
	{ "proftpd_port", "21" },
	{ "proftpd_anon", "0" },
	{ "proftpd_anon_subdir", "" },
	{ "proftpd_rad", "0" },
	{ "proftpd_authserverip", "" },
	{ "proftpd_authserverport", "1812" },
	{ "proftpd_acctserverport", "1813" },
	{ "proftpd_sharedkey", "" },
#endif
#ifdef HAVE_NFS
	{ "nfs_enable", "0" },
#endif
#ifdef HAVE_RSYNC
	{ "rsyncd_enable", "0" },
#endif
#ifdef HAVE_SAMBA3
	{ "samba3_enable", "0" },
	{ "samba3_pub", "0" },
	{ "samba3_dirpath", "/jffs" },
	{ "samba3_pubacl", "1" },
	{ "samba3_advanced", "0" },
#ifdef HAVE_SMBD
	{ "samba3_min_proto", "SMB2_10" },
	{ "samba3_max_proto", "SMB3_11" },
	{ "samba3_guest", "bad user" },
#else
	{ "samba3_min_proto", "SMB2" },
	{ "samba3_max_proto", "SMB2" },
#endif
	{ "samba3_encrypt", "auto" },
#endif
#ifdef HAVE_MINIDLNA
	{ "dlna_enable", "0" },
	{ "dlna_thumb", "0" },
	{ "dlna_merge", "0" },
	{ "dlna_album_art", "0" },
	{ "dlna_no_art", "0" },
	{ "dlna_subtitles", "0" },
	{ "dlna_cleandb", "0" },
	{ "dlna_metadata", "0" },
	{ "dlna_rescan", "0" },
#endif
#ifdef HAVE_VNCREPEATER
	{ "vncr_enable", "0" },
#endif
#ifdef HAVE_AP_SERV
	{ "apserv_enable", "1" },
#endif
#ifdef HAVE_AOSS
	{ "aoss_enable", "1" },
	{ "aoss_tkip", "0" },
	{ "aoss_aes", "1" },
	{ "aoss_wep", "0" },
#endif
#ifdef HAVE_WPS
	{ "aoss_enable", "0" },
	{ "wps_enabled", "1" },
#ifdef HAVE_IDEXX
	{ "wps_registrar", "0" },
#else
	{ "wps_registrar", "1" },
#endif
#endif

#ifdef HAVE_LLTD
	{ "lltd_enabled", "0" },
#endif
	{ "warn_enabled", "0" },
	{ "warn_connlimit", "500" },

#ifdef HAVE_USBIP
	{ "usb_ip", "0" },
#endif
#ifdef HAVE_FREECWMP
#ifdef HAVE_ETISALAT
	{ "freecwmp_enable", "1" },
	{ "freecwmp_acs_username", "softathome" },
	{ "freecwmp_acs_password", "softathome" },
	{ "freecwmp_acs_hostname", "86.96.241.17" },
	{ "freecwmp_acs_port", "7547" },
	{ "freecwmp_acs_path", "/ACS-server/ACS" },
	{ "freecwmp_acs_periodic_enable", "1" },
	{ "freecwmp_acs_periodic_interval", "172800" },
	{ "freecwmp_local_port", "51005" },
	{ "freecwmp_local_auth_enable", "1" },
	{ "freecwmp_local_username", "softathome" },
	{ "freecwmp_local_password", "softathome" },
#elif HAVE_AXTEL

#else
	{ "freecwmp_enable", "0" },
	{ "freecwmp_local_auth_enable", "0" },
#endif
	{ "freecwmp_bootevents", "bootstrap EmptyKey" },
#endif
#ifdef HAVE_ZABBIX
	{ "zabbix_enable", "0" },
#endif
#ifdef HAVE_UNFY
	{ "gpiovpn", "-13" },
#endif
#ifdef HAVE_PRIVOXY
	{ "privoxy_enable", "0" },
	{ "privoxy_advanced", "0" },
	{ "privoxy_pac_enable", "0" },
	{ "privoxy_transp_enable", "0" },
	{ "privoxy_maxclient", "128" },
#endif
#ifdef HAVE_SOFTETHER
	{ "setherclient_enable", "0" },
	{ "setherbridge_enable", "0" },
	{ "setherserver_enable", "0" },
#endif
#ifdef HAVE_TOR
	{ "tor_enable", "0" },
	{ "tor_relayonly", "0" },
	{ "tor_dir", "0" },
	{ "tor_relay", "0" },
	{ "tor_bridge", "0" },
	{ "tor_transparent", "0" },
	{ "tor_bwrate", "100" },
	{ "tor_bwburst", "200" },
	{ "tor_strict", "0" },
#endif
#ifdef HAVE_UDPXY
	{ "udpxy_enable", "0" },
	{ "udpxy_nicfrom", "" },
	{ "udpxy_listenif", "" },
	{ "udpxy_listenport", "4022" },
#endif
#ifdef HAVE_WEBSERVER
	{ "lighttpd_enable", "0" },
	{ "lighttpd_sslport", "443" },
	{ "lighttpd_port", "8000" },
	{ "lighttpd_root", "/jffs/www" },
	{ "lighttpd_wan", "0" },
#endif
#ifdef HAVE_GPSI
	{ "gps", "0" },
	{ "gps_gpsd", "0" },
	{ "gps_gpsd_port", "2947" },
#endif
#ifdef HAVE_TRANSMISSION
	{ "transmission_enable", "0" },
	{ "transmission_dir", "/mnt/sda/transmission_config" },
	{ "transmission_download", "/mnt/sda" },
	{ "transmission_rpc", "9091" },
	{ "transmission_style", "transmission-web-control" },
#endif
#ifdef HAVE_PLEX
	{ "plex_enable", "0" },
	{ "plex_appdir", "/jffs/plex" },
#endif
#ifdef HAVE_MACTELNET
	{ "mactelnetd_enable", "0" },
#endif
#ifdef HAVE_ROUTERSTYLE
	{ "router_style_dark", "0" },
	{ "sticky_footer", "1" },
#endif
#if defined(HAVE_ONNET) || defined(HAVE_RAYTRONIK)
	{ "radius_country", "ARE" },
	{ "radius_state", "none" },
	{ "radius_locality", "none" },
	{ "radius_organisation", "none" },
	{ "radius_email", "none" },
	{ "radius_common", "none" },
	{ "radius_passphrase", "none" },
#endif
#ifdef HAVE_SPEEDCHECKER
	{ "shownf", "1" },
	{ "speedchecker_enable", "0" },
#endif
#ifdef HAVE_USB_ADV
	{ "drive_ra", "256" },	// read ahead default per physical drive
#endif
#ifdef HAVE_SMARTDNS
	{ "smartdns", "0" },	// read ahead default per physical drive
	{ "smartdns_dualstack_ip_selection", "0" },	// prefer ipv4 over ipv6 if ipv6 is significant slower
	{ "smartdns_prefetch_domain", "1" },	// do cyclic background resolv and measurement
	{ "smartdns_serve_expired", "1" },	// 
	{ "smartdns_use_dns", "0" },	// 0 = use dns-list from router e.g. from WAN, Static DNS, WireGuard or VPN; 1 = use only SmartDNS servers from Addtional Options
#endif
#ifdef HAVE_MDNS
	{ "mdns_enable", "0" },
	{ "mdns_domain", "local" },
	{ "mdns_reflector", "0" },
	{ "mdns_interfaces", "br0" },
#endif
#ifdef HAVE_X86
	{ "boot_disable_msi", "0" },
	{ "boot_noaer", "0" },
	{ "boot_noari", "0" },
	{ "boot_noacpi", "0" },
	{ "boot_pcie_tune", "default" },
	{ "boot_mds", "0" },
	{ "boot_tsx_async_abort", "0" },
	{ "boot_srbds", "0" },
	{ "boot_nospectre_v1", "0" },
	{ "boot_nospectre_v2", "0" },
	{ "boot_l1tf", "0" },
	{ "boot_nospec_store_bypass_disable", "0" },
	{ "boot_nopti", "0" },
	{ "boot_pstate", "0" },
#endif
	{ "no_bootfails", "0" },
	{ "boot_fail_open", "0" },
	{ "boot_fail_keepip", "0" },
	{ "switch_leds", "1" },
#ifdef HAVE_IRQBALANCE
	{ "irqbalance_enabled", "1" },
#endif
	{ 0, 0 }
};
#else
/*struct nvram_param *srouter_defaults = NULL;
static unsigned char **values;
static unsigned int defaultnum;
static unsigned int stores;
void load_defaults(void)
{
	FILE *in = fopen("/etc/defaults.bin", "rb");
	if (in == NULL)
		return;
	unsigned int i;
	defaultnum = (unsigned int)getc(in);
	defaultnum |= (unsigned int)getc(in) << 8;
	defaultnum |= (unsigned int)getc(in) << 16;
	defaultnum |= (unsigned int)getc(in) << 24;
	stores = getc(in);	// count of unique values

	unsigned char *index;
	index = malloc(sizeof(char) * defaultnum);
	fread(index, defaultnum, 1, in);	// read string index table

	values = malloc(sizeof(char *) * stores);
	for (i = 0; i < stores; i++) {
		char temp[4096];
		int c;
		int a = 0;
		while ((c = getc(in)) != 0) {
			temp[a++] = c;
		}
		temp[a] = 0;

		values[i] = strdup(temp);
	}
	srouter_defaults = (struct nvram_param *)malloc(sizeof(struct nvram_param) * (defaultnum + 1));
	memset(srouter_defaults, 0, sizeof(struct nvram_param) * (defaultnum + 1));
	for (i = 0; i < defaultnum; i++) {
		char temp[4096];
		int c;
		int a = 0;
		while ((c = getc(in)) != 0) {
			temp[a++] = c;
		}
		temp[a++] = 0;
		srouter_defaults[i].name = strdup(temp);
		srouter_defaults[i].value = values[index[i]];
	}
	free(index);
	fclose(in);
}

void free_defaults(void)
{
	int i;
	for (i = defaultnum - 1; i > -1; i--) {
		if (srouter_defaults[i].name) {
			free(srouter_defaults[i].name);
		}
	}
	free(srouter_defaults);
	for (i = stores - 1; i > -1; i--) {
		free(values[i]);
	}
	free(values);

}*/
#endif

#ifdef HAVE_SKYTEL
#undef HAVE_POWERNOC_WORT54G
#undef HAVE_POWERNOC
#endif
