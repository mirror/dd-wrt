/*
 * menu.c
 *
 * Copyright (C) 2005 - 2022 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

//this is undefined behavior but works for gcc compilers since strings are deduplicated
static inline int strcmp_pnt(const char *s1, const char *s2)
{
	return !(s1 == s2);
}

struct menucontext {
	char *menu[8][13];
	char *menuname[8][14];
};
#define MENU_INDEX 0
#define MENU_WIRELESS 1
#define MENU_SERVICES 2
#define MENU_FIREWALL 3
#define MENU_FILTERS 4
#define MENU_QOS 5
#define MENU_ADMIN 6
#define MENU_STATUS 7

static struct menucontext *init_menu(webs_t wp)
{
	static struct menucontext *m = NULL;
	if (!m) {
		m = malloc(sizeof(struct menucontext));
		bzero(m, sizeof(struct menucontext));
	}
#ifdef HAVE_ERC
	static char *menu_s[8][13] = {
		{ "index.asp", "DDNS.asp", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "Wireless_Basic.asp", "WL_WPATable.asp", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "ForwardSpec.asp", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "Filters.asp", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "Management.asp", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "", "", "", "", "", "", "", "", "", "", "", "", "" }	//
	};
	/*
	 * real name is bmenu.menuname[i][j]
	 */
	static char *menuname_s[8][14] = {
		{ "setup", "setupbasic", "setupddns", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "wireless", "wirelessBasic", "wirelessSecurity", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "applications", "applicationspforwarding", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "accrestriction", "webaccess", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "admin", "adminManagement", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "", "", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "", "", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "", "", "", "", "", "", "", "", "", "", "", "", "", "" },	//
	};

#elif HAVE_IPR

#endif

	static char *menu_t[8][13] = {
		{ "index.asp", "IPV6.asp", "DDNS.asp", "WanMAC.asp", "Routing.asp", "Vlan.asp", "Networking.asp", "eop-tunnel.asp", "", "", "", "", "" },	//
		{ "Wireless_Basic.asp", "SuperChannel.asp", "WiMAX.asp", "Wireless_radauth.asp", "WL_WPATable.asp", "Roaming.asp", "AOSS.asp", "Wireless_MAC.asp", "Wireless_Advanced.asp", "Wireless_WDS.asp", "", "", "" },	//
		{ "Services.asp", "FreeRadius.asp", "PPPoE_Server.asp", "PPTP.asp", "USB.asp", "NAS.asp", "Hotspot.asp", "Nintendo.asp", "Milkfish.asp", "Privoxy.asp", "Speedchecker.asp", "", "" },	//
		{ "Firewall.asp", "VPN.asp", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "Filters.asp", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "ForwardSpec.asp", "Forward.asp", "ForwardIP.asp", "Triggering.asp", "UPnP.asp", "DMZ.asp", "QoS.asp", "P2P.asp", "", "", "", "", "" },	//
		{ "Management.asp", "Alive.asp", "Sysctl.asp", "Diagnostics.asp", "Wol.asp", "Factory_Defaults.asp", "Upgrade.asp", "config.asp", "", "", "", "", "" },	//
		{ "Status_Router.asp", "Status_Internet.asp", "Status_Lan.asp", "Status_Wireless.asp", "Status_SputnikAPD.asp", "Status_OpenVPN.asp", "Status_Bandwidth.asp", "Syslog.asp", "Info.htm", "register.asp", "MyPage.asp", "Gpio.asp", "Status_CWMP.asp" }	//
	};
	/*
	 * real name is bmenu.menuname[i][j]
	 */
	static char *menuname_t[8][14] = {
		{ "setup", "setupbasic", "setupipv6", "setupddns", "setupmacclone", "setuprouting", "setupvlan", "networking", "setupeop", "", "", "", "", "" },	//
		{ "wireless", "wirelessBasic", "wirelessSuperchannel", "wimax", "wirelessRadius", "wirelessSecurity", "wirelessRoaming",	//
#if defined(HAVE_AOSS) && defined(HAVE_WPS)
		 "wirelessAossWPS",
#elif defined(HAVE_AOSS) && !defined(HAVE_WPS)
		 "wirelessAoss",
#elif !defined(HAVE_AOSS) && defined(HAVE_WPS)
		 "wirelessWPS",
#else
		 "",		// place holder
#endif
		 "wirelessMac", "wirelessAdvanced", "wirelessWds", "", "", ""},	//
		{ "services", "servicesServices", "servicesRadius", "servicesPppoesrv", "servicesPptp", "servicesUSB", "servicesNAS", "servicesHotspot", "servicesNintendo", "servicesMilkfish", "servicesPrivoxy", "servicesSpeedchecker", "", "" },	//
		{ "security", "firwall", "vpn", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "accrestriction", "webaccess", "", "", "", "", "", "", "", "", "", "", "", "" },	//
		{ "applications", "applicationspforwarding", "applicationsprforwarding", "applicationsipforwarding", "applicationsptriggering", "applicationsUpnp", "applicationsDMZ", "applicationsQoS", "applicationsP2P", "", "", "", "", "" },	//
		{ "admin", "adminManagement", "adminAlive", "adminSysctl", "adminDiag", "adminWol", "adminFactory", "adminUpgrade", "adminBackup", "", "", "", "", "" },	//
		{ "statu", "statuRouter", "statuInet", "statuLAN", "statuWLAN", "statuSputnik", "statuVPN", "statuBand", "statuSyslog", "statuSysInfo", "statuActivate", "statuMyPage", "statuGpio", "statuCWMP" }	//
	};
	int x, y;
	for (x = 0; x < 8; x++) {
		for (y = 0; y < 14; y++) {
			m->menuname[x][y] = menuname_t[x][y];
			if (y < 13) {
				m->menu[x][y] = menu_t[x][y];
			}
		}
	}
#ifdef HAVE_ERC
	if (!wp->userid) {
		for (x = 0; x < 8; x++) {
			for (y = 0; y < 14; y++) {
				m->menuname[x][y] = menuname_s[x][y];
				if (y < 13) {
					m->menu[x][y] = menu_s[x][y];
				}
			}
		}
	}
#endif
#ifdef HAVE_IPR
	if (!wp->userid) {
		m->menu[MENU_INDEX][2] = NULL;	// setup - mac cloning
		//menu[MENU_INDEX][4] = NULL;  // setup - routing / test!
		m->menu[MENU_SERVICES][4] = NULL;	// services - USB
		m->menu[MENU_SERVICES][5] = NULL;	// services - NAS
		m->menu[MENU_SERVICES][6] = NULL;	// services - Hotspot
		m->menu[MENU_ADMIN][3] = NULL;	// administration - commands
		m->menu[MENU_ADMIN][6] = NULL;	// administration - upgrade
	}
	m->menu[MENU_SERVICES][9] = NULL;	// services - anchorfree
#endif

#ifdef HAVE_CORENET
	m->menuname[MENU_INDEX][0] = "setupnetw";
	m->menuname[MENU_ADMIN][0] = "adminman";
#endif
#ifdef HAVE_MADWIFI
	static char *wdsmenu[4] = { "Wireless_WDS-wlan0.asp", "Wireless_WDS-wlan1.asp", "Wireless_WDS-wlan2.asp", "Wireless_WDS-wlan3.asp" };
	static char *tran_wdsmenu[4] = { "wirelessWds0", "wirelessWds1", "wirelessWds2", "wirelessWds3" };
#if defined(HAVE_BUFFALO) && !defined(HAVE_ATH9K)
	m->menu[MENU_WIRELESS][8] = NULL;
	m->menuname[MENU_WIRELESS][9] = NULL;
#else
	// fill up WDS
	int ifcount = getdevicecount();
	if (ifcount > 4)
		ifcount = 4;	//truncate to max of 4
	int a;
	int count = 0;
	for (a = 0; a < ifcount; a++) {
		char check[32];
		sprintf(check, "wlan%d", a);
		if (has_ad(check))
			continue;
		m->menu[MENU_WIRELESS][count + 8] = wdsmenu[a];
		if (ifcount == 1)
			m->menuname[MENU_WIRELESS][count + 9] = "wirelessWds";
		else
			m->menuname[MENU_WIRELESS][count + 9] = tran_wdsmenu[a];
		count++;
	}
#endif
#else
#ifdef HAVE_ERC
	if (wp->userid) {
#endif

		int ifcount = get_wl_instances();
		int a;
		static char *wdsmenu[4] = { "Wireless_WDS-wl0.asp", "Wireless_WDS-wl1.asp", "Wireless_WDS-wl2.asp", "Wireless_WDS-wl3.asp" };
		static char *tran_wdsmenu[4] = { "wirelessWdswl0", "wirelessWdswl1", "wirelessWdswl2", "wirelessWdswl3" };

		static char *advmenu[4] = { "Wireless_Advanced-wl0.asp", "Wireless_Advanced-wl1.asp", "Wireless_Advanced-wl2.asp", "Wireless_Advanced-wl3.asp" };
		static char *tran_advmenu[4] = { "wirelessAdvancedwl0", "wirelessAdvancedwl1", "wirelessAdvancedwl2", "wirelessAdvancedwl3" };

		for (a = 0; a < ifcount; a++) {
			m->menu[MENU_WIRELESS][a * 2 + 7] = advmenu[a];
			m->menu[MENU_WIRELESS][a * 2 + 8] = wdsmenu[a];
			if (ifcount == 1) {
				m->menuname[MENU_WIRELESS][a * 2 + 8] = "wirelessAdvanced";
				m->menuname[MENU_WIRELESS][a * 2 + 9] = "wirelessWds";
			} else {
				m->menuname[MENU_WIRELESS][a * 2 + 8] = tran_advmenu[a];
				m->menuname[MENU_WIRELESS][a * 2 + 9] = tran_wdsmenu[a];
			}
		}
#ifdef HAVE_ERC
	}
#endif
#endif

#ifdef HAVE_ANTAIRA_MINI
	m->menu[MENU_INDEX][1] = NULL;	// setup - ipv6
	//m->menu[MENU_INDEX][2] = NULL;        // setup - ddns
	//m->menu[MENU_INDEX][3] = NULL;        // setup - macclone
	m->menu[MENU_INDEX][4] = NULL;	// setup - routing
	m->menu[MENU_INDEX][5] = NULL;	// setup - vlan
	//m->menu[MENU_INDEX][6] = NULL;        // setup - networking
	m->menu[MENU_INDEX][7] = NULL;	// setup - setupeop

	m->menu[MENU_WIRELESS][1] = NULL;	// wireless - superchannel
	m->menu[MENU_WIRELESS][2] = NULL;	// wireless - wimax
	m->menu[MENU_WIRELESS][3] = NULL;	// wireless - radius
	//m->menu[MENU_WIRELESS][4] = NULL;     // wireless - security
	m->menu[MENU_WIRELESS][5] = NULL;	// wireless - wps
	//m->menu[MENU_WIRELESS][6] = NULL;     // wireless - macfilter
	m->menu[MENU_WIRELESS][7] = NULL;	// wireless - advanced
	//m->menu[MENU_WIRELESS][8] = NULL;     // wireless - wds

	m->menu[MENU_SERVICES][1] = NULL;	// services - Radius
	m->menu[MENU_SERVICES][2] = NULL;	// services - PPPOED
	m->menu[MENU_SERVICES][3] = NULL;	// services - PPTPD
	m->menu[MENU_SERVICES][4] = NULL;	// services - USB
	m->menu[MENU_SERVICES][5] = NULL;	// services - NAS
	m->menu[MENU_SERVICES][6] = NULL;	// services - hotspot

	m->menu[MENU_FIREWALL][0] = NULL;	// security
	m->menu[MENU_FILTERS][0] = NULL;	// Access Restriction

	m->menu[MENU_QOS][2] = NULL;	// applications/NAT/QOS - porttrigger
	m->menu[MENU_QOS][4] = NULL;	// applications/NAT/QOS - dmz
	m->menu[MENU_QOS][5] = NULL;	// applications/NAT/QOS - qos
	m->menu[MENU_QOS][6] = NULL;	// applications/NAT/QOS - p2p

	//m->menu[MENU_ADMIN][1] = NULL;        // admin - keepalive
#if !defined(HAVE_PERU)
	m->menu[MENU_ADMIN][3] = NULL;	// admin - diag
#endif
	//m->menu[MENU_ADMIN][4] = NULL;        // admin - wol

	m->menu[MENU_STATUS][4] = NULL;	// status - sputnik
	m->menu[MENU_STATUS][5] = NULL;	// status - vpn
	m->menu[MENU_STATUS][7] = NULL;	// status - syslog
	m->menu[MENU_STATUS][8] = NULL;	// status - info
#endif				/*HAVE_ANTAIRA */
	return m;
}

EJ_VISIBLE void ej_do_menu(webs_t wp, int argc, char_t ** argv)
{
	char *mainmenu, *submenu;

	mainmenu = argv[0];
	submenu = argv[1];

	int vlan_supp = check_vlan_support();
	if (getRouterBrand() == ROUTER_UBNT_UNIFIAC)
		vlan_supp = 1;
	if (getRouterBrand() == ROUTER_UBNT_NANOAC)
		vlan_supp = 1;

#ifdef HAVE_SPUTNIK_APD
	int sputnik = nvram_matchi("apd_enable", 1);
#else
	int sputnik = 0;
#endif
	int openvpn = nvram_matchi("openvpn_enable", 1) | nvram_matchi("openvpncl_enable", 1);
	int auth = nvram_matchi("status_auth", 1);
	int registered = 1;
#ifdef HAVE_REGISTER
	if (!wp->isregistered_real)
		registered = 0;
	int cpeonly = iscpe();
#else
	int cpeonly = 0;
#endif

#ifdef HAVE_MADWIFI
#ifdef HAVE_NOWIFI
	int wifi = 0;
#else
	int wifi = haswifi();
#endif
#endif
#ifdef HAVE_MADWIFI
	int wimaxwifi = 0;
#endif

	struct menucontext *m = init_menu(wp);
	int i, j;

	websWrite(wp, "<div id=\"menu\">\n");
	websWrite(wp, "<div id=\"menuMain\">\n");
	websWrite(wp, "<ul id=\"menuMainList\">\n");
#ifdef HAVE_WAVESAT
	wimaxwifi = 1;
#endif

#define MAXMENU 8
#define MAXSUBMENU 13

	for (i = 0; i < MAXMENU; i++) {
		if (m->menu[i][0] == NULL)
			continue;
#ifdef HAVE_MADWIFI
		if (!wifi && !wimaxwifi && !strcmp_pnt(m->menu[i][0], "Wireless_Basic.asp"))
			i++;
#endif
#ifdef HAVE_CORENET
		if (!strcmp_pnt(m->menu[i][0], "Firewall.asp") || !strcmp_pnt(m->menu[i][0], "Filters.asp") || !strcmp_pnt(m->menu[i][0], "ForwardSpec.asp"))	// jump over
			// Corenet
			i++;
#endif
		if (i >= MAXMENU)
			break;
		if (!strcmp(m->menu[i][0], mainmenu)) {
//fprintf(stderr,"%s->%d\n",__func__,__LINE__);
#ifdef HAVE_MADWIFI
			if (!wifi && wimaxwifi && !strcmp_pnt(m->menu[i][0], "Wireless_Basic.asp"))
				websWrite(wp, "<li class=\"current\"><span><script type=\"text/javascript\">Capture(bmenu.wimax)</script></span>\n");
			else
#endif
				websWrite(wp, "<li class=\"current\"><span><script type=\"text/javascript\">Capture(bmenu.%s)</script></span>\n", m->menuname[i][0]);
//fprintf(stderr,"%s->%d\n",__func__,__LINE__);
			websWrite(wp, "<div id=\"menuSub\">\n");

			websWrite(wp, "<ul id=\"menuSubList\">\n");
			websWrite(wp,
				  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<a id=\\\"logout\\\" title=\\\"\" + share.logout + \"\\\" aria-label=\\\"\" + share.logout + \"\\\" href=\\\"dologout.asp\\\">\");\n//]]>\n</script>\n");
			websWrite(wp, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"16\" height=\"16\" fill=\"currentColor\" class=\"logout\" viewBox=\"0 0 16 16\">\n");
			websWrite(wp,
				  "<path fill-rule=\"evenodd\" d=\"M10 12.5a.5.5 0 0 1-.5.5h-8a.5.5 0 0 1-.5-.5v-9a.5.5 0 0 1 .5-.5h8a.5.5 0 0 1 .5.5v2a.5.5 0 0 0 1 0v-2A1.5 1.5 0 0 0 9.5 2h-8A1.5 1.5 0 0 0 0 3.5v9A1.5 1.5 0 0 0 1.5 14h8a1.5 1.5 0 0 0 1.5-1.5v-2a.5.5 0 0 0-1 0v2z\"></path>\n");
			websWrite(wp,
				  "<path fill-rule=\"evenodd\" d=\"M15.854 8.354a.5.5 0 0 0 0-.708l-3-3a.5.5 0 0 0-.708.708L14.293 7.5H5.5a.5.5 0 0 0 0 1h8.793l-2.147 2.146a.5.5 0 0 0 .708.708l3-3z\"></path></svg></a>");

			for (j = 0; j < MAXSUBMENU; j++) {
//fprintf(stderr,"%s->%d %d %d\n",__func__,__LINE__,i, j);
				if (!m->menu[i][j] || !(*m->menu[i][j]))
					continue;
#ifdef HAVE_MADWIFI
				if (!wifi && !strncmp(m->menu[i][j], "Wireless_Basic.asp", 8))
					goto skip;
#ifndef HAVE_SUPERCHANNEL
				if (!strcmp_pnt(m->menu[i][j], "SuperChannel.asp"))	// jump over
					// PPTP in
					// micro
					// build
					goto skip;
#else
				if (!strcmp_pnt(m->menu[i][j], "SuperChannel.asp") && (wp->issuperchannel || !wifi))	// jump
					// over
					// PPTP
					// in
					// micro
					// build
					goto skip;
#endif
#else
				if (!strcmp_pnt(m->menu[i][j], "SuperChannel.asp"))	// jump over
					// PPTP in
					// micro
					// build
					goto skip;
#endif
#ifndef HAVE_WAVESAT
				if (!strcmp_pnt(m->menu[i][j], "WiMAX.asp"))	// jump over
					// WiMAX
					goto skip;
#else
				if (!wimaxwifi && !strcmp_pnt(m->menu[i][j], "WiMAX.asp"))	// jump
					// over
					// WiMAX
					goto skip;
#endif
#if !defined(HAVE_AOSS) && !defined(HAVE_WPS)
				if (!strcmp_pnt(m->menu[i][j], "AOSS.asp"))	// jump over
					// AOSS
					goto skip;
#endif
#if defined(HAVE_WPS) && !defined(HAVE_IDEXX)
				if (!strcmp_pnt(m->menu[i][j], "AOSS.asp"))	// jump over
					// AOSS
					goto skip;
#endif
#ifdef HAVE_MADWIFI
				if (!wifi && !strcmp_pnt(m->menu[i][j], "WL_WPATable.asp"))	// jump
					// over
					// PPTP
					// in
					// micro
					// build
					goto skip;
#ifndef HAVE_WPA3
				if (!strcmp_pnt(m->menu[i][j], "Roaming.asp"))	// jump
					// over
					// PPTP
					// in
					// micro
					// build
					goto skip;
#else
				if (!wifi && !strcmp_pnt(m->menu[i][j], "Roaming.asp"))	// jump
					// over
					// PPTP
					// in
					// micro
					// build
					goto skip;
#endif
				if (!strcmp_pnt(m->menu[i][j], "Wireless_radauth.asp"))
					goto skip;
				if (!wifi && !strncmp(m->menu[i][j], "Wireless_MAC.asp", 8))
					goto skip;
				if (!strncmp(m->menu[i][j], "Wireless_Advanced", 17))
					goto skip;
				if ((!wifi || cpeonly)
				    && !strncmp(m->menu[i][j], "Wireless_WDS", 12))
					goto skip;
				if (!wifi && !strcmp_pnt(m->menu[i][j], "Status_Wireless.asp"))
					goto skip;

#endif
				if ((!vlan_supp) && !strcmp_pnt(m->menu[i][j], "Vlan.asp"))	// jump
					// over
					// VLANs
					// if
					// vlan
					// not
					// supported
					goto skip;
#ifndef HAVE_FREERADIUS
				if (!strcmp_pnt(m->menu[i][j], "FreeRadius.asp"))
					goto skip;
#endif
#ifndef HAVE_PPPOESERVER
				if (!strcmp_pnt(m->menu[i][j], "PPPoE_Server.asp"))
					goto skip;
#endif
#ifdef HAVE_MICRO
				if (!strcmp_pnt(m->menu[i][j], "PPTP.asp"))	// jump over PPTP in
					// micro build
					goto skip;
#endif
#ifndef HAVE_USB
				if (!strcmp_pnt(m->menu[i][j], "USB.asp"))	// jump over USB
					goto skip;
#endif
#ifndef HAVE_SYSCTL_EDIT
				if (!strcmp_pnt(m->menu[i][j], "Sysctl.asp"))	// jump over sysctl editor
					goto skip;
#endif
#ifndef HAVE_NAS_SERVER
				if (!strcmp_pnt(m->menu[i][j], "NAS.asp"))	// jump over NAS
					goto skip;
#endif
#ifdef HAVE_GLAUCO
				if (!strcmp_pnt(m->menu[i][j], "Factory_Defaults.asp"))
					goto skip;
				if (!strcmp_pnt(m->menu[i][j], "Upgrade.asp"))
					goto skip;
#endif
#ifdef HAVE_SANSFIL
				if (!strcmp_pnt(m->menu[i][j], "Hotspot.asp"))
					goto skip;
#endif
#ifndef HAVE_SPOTPASS
				if (!strcmp_pnt(m->menu[i][j], "Nintendo.asp"))	// jump over
					// Nintendo
					goto skip;
#endif
#ifndef HAVE_MILKFISH
				if (!strcmp_pnt(m->menu[i][j], "Milkfish.asp"))
					goto skip;
#endif
#ifndef HAVE_IPV6
				if (!strcmp_pnt(m->menu[i][j], "IPV6.asp"))
					goto skip;
#endif
//#ifdef HAVE_WIKINGS
//                              if (!strcmp_pnt(m->menu[i][j], "AnchorFree.asp"))
//                                      goto skip;
//#endif
#ifndef HAVE_PRIVOXY
				if (!strcmp_pnt(m->menu[i][j], "Privoxy.asp"))
					goto skip;
#endif
#ifndef HAVE_SPEEDCHECKER
				if (!strcmp_pnt(m->menu[i][j], "Speedchecker.asp"))
					goto skip;
#endif
//#ifdef HAVE_ESPOD
//                              if (!strcmp_pnt(m->menu[i][j], "AnchorFree.asp"))
//                                      goto skip;
//#endif
//#ifdef HAVE_CARLSONWIRELESS
//                              if (!strcmp_pnt(m->menu[i][j], "AnchorFree.asp"))
//                                      goto skip;
//#endif
#ifndef HAVE_WOL
				if (!strcmp_pnt(m->menu[i][j], "Wol.asp"))
					goto skip;
#endif
#ifndef HAVE_EOP_TUNNEL
				if (!strcmp_pnt(m->menu[i][j], "eop-tunnel.asp"))
					goto skip;
#endif
#ifndef HAVE_VLANTAGGING
				if (!strcmp_pnt(m->menu[i][j], "Networking.asp"))
					goto skip;
#endif
#ifndef HAVE_CTORRENT
				if (!strcmp_pnt(m->menu[i][j], "P2P.asp"))
					goto skip;
#endif
				if ((!sputnik) && !strcmp_pnt(m->menu[i][j], "Status_SputnikAPD.asp"))	// jump
					// over
					// Sputnik
					goto skip;
				if ((!openvpn) && !strcmp_pnt(m->menu[i][j], "Status_OpenVPN.asp"))	// jump
					// over
					// OpenVPN
					goto skip;
				if ((!auth) && !strcmp_pnt(m->menu[i][j], "Info.htm"))	// jump
					// over
					// Sys-Info
					goto skip;
				if ((registered) && !cpeonly && !strcmp_pnt(m->menu[i][j], "register.asp"))	// jump
					// over
					// register.asp
					goto skip;
				if ((!*(nvram_safe_get("mypage_scripts"))) && !strcmp_pnt(m->menu[i][j], "MyPage.asp"))	// jump
					// over
					// MyPage.asp
					goto skip;
#ifndef HAVE_STATUS_GPIO
				if (!strcmp_pnt(m->menu[i][j], "Gpio.asp"))
					goto skip;
#endif
#ifndef HAVE_FREECWMP
				if (!strcmp_pnt(m->menu[i][j], "Status_CWMP.asp"))
					goto skip;
#endif
#ifndef HAVE_STATUS_SYSLOG
				if (!strcmp_pnt(m->menu[i][j], "Syslog.asp"))
					goto skip;
#endif
#ifdef HAVE_MADWIFI
				if (!strcmp(m->menu[i][j], submenu)
				    && (*(m->menu[i][j])
					&& !strcmp_pnt(m->menu[i][j], "Wireless_Basic.asp")
					&& !wifi && wimaxwifi)) {
					websWrite(wp, "<li><span><script type=\"text/javascript\">Capture(bmenu.wimax)</script></span></li>\n");
				}
#endif
				else if (!strcmp(m->menu[i][j], submenu)
					 && (*(m->menu[i][j]))) {
					websWrite(wp, "<li><span><script type=\"text/javascript\">Capture(bmenu.%s)</script></span></li>\n", m->menuname[i][j + 1]);
				}
#ifdef HAVE_MATRIXSSL
				else if (DO_SSL(wp) && (*(m->menu[i][j]) != 0)
					 && ((!strcmp_pnt(m->menu[i][j], "Upgrade.asp")
					      || (!strcmp_pnt(m->menu[i][j], "config.asp"))))) {
					websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
					websWrite(wp,
						  "document.write(\"<li><a style=\\\"cursor:pointer\\\" title=\\\"\" + errmsg.err46 + \"\\\" onclick=\\\"alert(errmsg.err45)\\\" ><em>\" + bmenu.%s + \"</em></a></li>\");\n",
						  m->menuname[i][j + 1]);
					websWrite(wp, "\n//]]>\n</script>\n");
				}
#endif				/* < */
#ifdef HAVE_MADWIFI
				else if (*(m->menu[i][j])
					 && !strcmp_pnt(m->menu[i][j], "Wireless_Basic.asp")
					 && !wifi && wimaxwifi) {
					websWrite(wp, "<li><a href=\"WiMAX.asp\"><script type=\"text/javascript\">Capture(bmenu.wimax)</script></a></li>\n");
				}
#endif
				else if (*(m->menu[i][j])) {
					websWrite(wp, "<li><a href=\"%s\"><script type=\"text/javascript\">Capture(bmenu.%s)</script></a></li>\n", m->menu[i][j], m->menuname[i][j + 1]);
				}
			      skip:;
			}
			websWrite(wp, "</ul>\n");
			websWrite(wp, "</div>\n");
			websWrite(wp, "</li>\n");
		}
#ifdef HAVE_MADWIFI
		else if (!strcmp_pnt(m->menu[i][0], "Wireless_Basic.asp") && !wifi && wimaxwifi) {
			websWrite(wp, "<li><a href=\"WiMAX.asp\"><script type=\"text/javascript\">Capture(bmenu.wimax)</script></a></li>\n");
		}
#endif
		else {
			websWrite(wp, "<li><a href=\"%s\"><script type=\"text/javascript\">Capture(bmenu.%s)</script></a></li>\n", m->menu[i][0], m->menuname[i][0]);
		}
	}
	websWrite(wp, "</ul>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "</div>\n");
	return;
}
