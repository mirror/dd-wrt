/*
 * rt2880.c
 *
 * Copyright (C) 2008 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#if defined(HAVE_RT2880) || defined(HAVE_RT61)
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <utils.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>
#include <libbridge.h>
#include <services.h>
#include <wlutils.h>
#include <libbridge.h>
#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

extern int br_add_interface(const char *br, const char *dev);

// returns the number of installed atheros devices/cards

static int need_commit = 0;
void addbssid(FILE *fp, char *prefix)
{
	char *bssid = nvram_nget("%s_bssid", prefix);
	char c_bssid[32];
	strncpy(c_bssid, bssid, 31);
	int i;
	int cnt = 0;
	for (i = 0; i < strlen(c_bssid); i++) {
		if (c_bssid[i] == ' ') {
			continue;
		}
		c_bssid[cnt++] = c_bssid[i];
	}
	c_bssid[cnt] = 0;
	if (strlen(c_bssid) == 17 && strcmp(c_bssid, "00:00:00:00:00:00"))
		fprintf(fp, "\tbssid=%s\n", c_bssid);
}

void setupSupplicant(const char *prefix)
{
	char akm[16];

	char eapol[32];
	sprintf(eapol, "%s_eapol_version", prefix);
	sprintf(akm, "%s_akm", prefix);
	char wmode[16];

	sprintf(wmode, "%s_mode", prefix);
	if (nvram_match(akm, "8021X")) {
		char fstr[32];
		char psk[64];
		char ath[64];

		sprintf(fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
		FILE *fp = fopen(fstr, "wb");

		fprintf(fp, "ap_scan=1\n");
		fprintf(fp, "fast_reauth=1\n");
		fprintf(fp, "eapol_version=%s\n", nvram_default_get(eapol, "1"));
		fprintf(fp, "network={\n");
		sprintf(psk, "%s_ssid", prefix);
		fprintf(fp, "\tssid=\"%s\"\n", nvram_safe_get(psk));
		addbssid(fp, prefix);
		fprintf(fp, "\tscan_ssid=1\n");
		if (nvram_prefix_match("8021xtype", prefix, "tls")) {
			// -> added habeIchVergessen
			char *keyExchng = nvram_nget("%s_tls8021xkeyxchng", prefix);
			char wpaOpts[40];
			if (!*keyExchng)
				nvram_nset("wep", "%s_tls8021xkeyxchng", prefix);
			strcpy(wpaOpts, "");
			keyExchng = nvram_nget("%s_tls8021xkeyxchng", prefix);
			if (strcmp("wpa2", keyExchng) == 0)
				sprintf(wpaOpts, "\tpairwise=CCMP\n\tgroup=CCMP\n");
			if (strcmp("wpa2mixed", keyExchng) == 0)
				sprintf(wpaOpts, "\tpairwise=CCMP TKIP\n\tgroup=CCMP TKIP\n");
			if (strcmp("wpa", keyExchng) == 0)
				sprintf(wpaOpts, "\tpairwise=TKIP\n\tgroup=TKIP\n");
			fprintf(fp, "\tkey_mgmt=%s\n%s", (!*wpaOpts ? "IEEE8021X" : "WPA-EAP"), wpaOpts);
			// <- added habeIchVergessen
			fprintf(fp, "\teap=TLS\n");
			fprintf(fp, "\tidentity=\"%s\"\n", nvram_prefix_get("tls8021xuser", prefix));
			sprintf(psk, "/tmp/%s", prefix);
			mkdir(psk, 0700);
			sprintf(psk, "/tmp/%s/ca.pem", prefix);
			sprintf(ath, "%s_tls8021xca", prefix);
			write_nvram(psk, ath);
			sprintf(psk, "/tmp/%s/user.pem", prefix);
			sprintf(ath, "%s_tls8021xpem", prefix);
			write_nvram(psk, ath);

			sprintf(psk, "/tmp/%s/user.prv", prefix);
			sprintf(ath, "%s_tls8021xprv", prefix);
			write_nvram(psk, ath);
			fprintf(fp, "\tca_cert=/tmp/%s/ca.pem\n", prefix);
			fprintf(fp, "\tclient_cert=/tmp/%s/user.pem\n", prefix);
			fprintf(fp, "\tprivate_key=/tmp/%s/user.prv\n", prefix);
			fprintf(fp, "\tprivate_key_passwd=\"%s\"\n", nvram_prefix_get("tls8021xpasswd", prefix));
			fprintf(fp, "\teapol_flags=3\n");
			if (*(nvram_nget("%s_tls8021xphase2", prefix))) {
				fprintf(fp, "\tphase2=\"%s\"\n", nvram_nget("%s_tls8021xphase2", prefix));
			}
			if (*(nvram_nget("%s_tls8021xanon", prefix))) {
				fprintf(fp, "\tanonymous_identity=\"%s\"\n", nvram_nget("%s_tls8021xanon", prefix));
			}
			if (*(nvram_nget("%s_tls8021xaddopt", prefix))) {
				sprintf(ath, "%s_tls8021xaddopt", prefix);
				fprintf(fp, "\t"); // tab
				fwritenvram(ath, fp);
				fprintf(fp, "\n"); // extra new line at the end
			}
		}
		if (nvram_prefix_match("8021xtype", prefix, "peap")) {
			fprintf(fp, "\tkey_mgmt=WPA-EAP\n");
			fprintf(fp, "\teap=PEAP\n");
			fprintf(fp, "\tpairwise=CCMP TKIP\n");
			fprintf(fp, "\tgroup=CCMP TKIP\n");
			fprintf(fp, "\tphase1=\"peapver=0\"\n");
			fprintf(fp, "\tidentity=\"%s\"\n", nvram_prefix_get("peap8021xuser", prefix));
			fprintf(fp, "\tpassword=\"%s\"\n", nvram_prefix_get("peap8021xpasswd", prefix));
			sprintf(psk, "/tmp/%s", prefix);
			mkdir(psk, 0700);
			sprintf(psk, "/tmp/%s/ca.pem", prefix);
			sprintf(ath, "%s_peap8021xca", prefix);
			if (!nvram_match(ath, "")) {
				write_nvram(psk, ath);
				fprintf(fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix);
			}
			if (*(nvram_nget("%s_peap8021xphase2", prefix))) {
				fprintf(fp, "\tphase2=\"%s\"\n", nvram_nget("%s_peap8021xphase2", prefix));
			}
			if (*(nvram_nget("%s_peap8021xanon", prefix))) {
				fprintf(fp, "\tanonymous_identity=\"%s\"\n", nvram_nget("%s_peap8021xanon", prefix));
			}
			if (*(nvram_nget("%s_peap8021xaddopt", prefix))) {
				sprintf(ath, "%s_peap8021xaddopt", prefix);
				fprintf(fp, "\t"); // tab
				fwritenvram(ath, fp);
				fprintf(fp, "\n"); // extra new line at the end
			}
		}
		if (nvram_prefix_match("8021xtype", prefix, "ttls")) {
			fprintf(fp, "\tkey_mgmt=WPA-EAP\n");
			fprintf(fp, "\teap=TTLS\n");
			fprintf(fp, "\tpairwise=CCMP TKIP\n");
			fprintf(fp, "\tgroup=CCMP TKIP\n");
			fprintf(fp, "\tidentity=\"%s\"\n", nvram_prefix_get("ttls8021xuser", prefix));
			fprintf(fp, "\tpassword=\"%s\"\n", nvram_prefix_get("ttls8021xpasswd", prefix));
			if (*(nvram_nget("%s_ttls8021xca", prefix))) {
				sprintf(psk, "/tmp/%s", prefix);
				mkdir(psk, 0700);
				sprintf(psk, "/tmp/%s/ca.pem", prefix);
				sprintf(ath, "%s_ttls8021xca", prefix);
				write_nvram(psk, ath);
				fprintf(fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix);
			}
			if (*(nvram_nget("%s_ttls8021xphase2", prefix))) {
				fprintf(fp, "\tphase2=\"%s\"\n", nvram_nget("%s_ttls8021xphase2", prefix));
			}
			if (*(nvram_nget("%s_ttls8021xanon", prefix))) {
				fprintf(fp, "\tanonymous_identity=\"%s\"\n", nvram_nget("%s_ttls8021xanon", prefix));
			}
			if (*(nvram_nget("%s_ttls8021xaddopt", prefix))) {
				sprintf(ath, "%s_ttls8021xaddopt", prefix);
				fprintf(fp, "\t"); // tab
				fwritenvram(ath, fp);
				fprintf(fp, "\n"); // extra new line at the end
			}
		}
		if (nvram_prefix_match("8021xtype", prefix, "leap")) {
			fprintf(fp, "\tkey_mgmt=WPA-EAP\n");
			fprintf(fp, "\teap=LEAP\n");
			fprintf(fp, "\tauth_alg=LEAP\n");
			fprintf(fp, "\tproto=WPA RSN\n");
			fprintf(fp, "\tpairwise=CCMP TKIP\n");
			fprintf(fp, "\tgroup=CCMP TKIP\n");
			fprintf(fp, "\tidentity=\"%s\"\n", nvram_prefix_get("leap8021xuser", prefix));
			fprintf(fp, "\tpassword=\"%s\"\n", nvram_prefix_get("leap8021xpasswd", prefix));
			if (*(nvram_nget("%s_leap8021xphase2", prefix))) {
				fprintf(fp, "\tphase2=\"%s\"\n", nvram_nget("%s_leap8021xphase2", prefix));
			}
			if (*(nvram_nget("%s_leap8021xanon", prefix))) {
				fprintf(fp, "\tanonymous_identity=\"%s\"\n", nvram_nget("%s_leap8021xanon", prefix));
			}
			if (*(nvram_nget("%s_leap8021xaddopt", prefix))) {
				sprintf(ath, "%s_leap8021xaddopt", prefix);
				fprintf(fp, "\t"); // tab
				fwritenvram(ath, fp);
				fprintf(fp, "\n"); // extra new line at the end
			}
		}
		fprintf(fp, "}\n");
		fclose(fp);
		sprintf(psk, "-i%s", getRADev(prefix));

		char bvar[32];

		sprintf(bvar, "%s_bridged", prefix);
		if (nvram_matchi(bvar, 1) && (nvram_match(wmode, "wdssta") || nvram_match(wmode, "wet")))
			log_eval("wpa_supplicant", "-b", nvram_safe_get("lan_ifname"), "-B", "-Dralink", psk, "-c", fstr);
		else
			log_eval("wpa_supplicant", "-B", "-Dralink", psk, "-c", fstr);
	}
}

void supplicant_main(int argc, char *argv[])
{
	setupSupplicant(argv[1]);
}

void setupHostAP(const char *prefix, int iswan)
{
}

void setMacFilter(char *iface)
{
	char *next;
	char var[32];
	char nvvar[32];

	eval("iwpriv", getRADev(iface), "set", "ACLClearAll=1");
	eval("iwpriv", getRADev(iface), "set", "AccessPolicy=0");

	sprintf(nvvar, "%s_macmode", iface);
	if (nvram_match(nvvar, "deny")) {
		eval("iwpriv", getRADev(iface), "set", "AccessPolicy=2");
		char nvlist[32];

		sprintf(nvlist, "%s_maclist", iface);

		foreach(var, nvram_safe_get(nvlist), next)
		{
			sysprintf("iwpriv %s set ACLAddEntry=%s", getRADev(iface), var);
		}
	}
	if (nvram_match(nvvar, "allow")) {
		eval("iwpriv", getRADev(iface), "set", "AccessPolicy=1");

		char nvlist[32];

		sprintf(nvlist, "%s_maclist", iface);

		foreach(var, nvram_safe_get(nvlist), next)
		{
			sysprintf("iwpriv %s set ACLAddEntry=%s", getRADev(iface), var);
		}
	}
}

void start_vifs(void)
{
}

void stop_vifs(void)
{
}

extern void adjust_regulatory(int count);

char *getWDSDev(char *wdsdev)
{
	char *newdev = "";

	if (!strcmp(wdsdev, "wds0.1"))
		newdev = "wds0";
	if (!strcmp(wdsdev, "wds0.2"))
		newdev = "wds1";
	if (!strcmp(wdsdev, "wds0.3"))
		newdev = "wds2";
	if (!strcmp(wdsdev, "wds0.4"))
		newdev = "wds3";
	if (!strcmp(wdsdev, "wds0.5"))
		newdev = "wds4";
	if (!strcmp(wdsdev, "wds0.6"))
		newdev = "wds5";
	if (!strcmp(wdsdev, "wds0.7"))
		newdev = "wds6";
	if (!strcmp(wdsdev, "wds0.8"))
		newdev = "wds7";
	if (!strcmp(wdsdev, "wds0.9"))
		newdev = "wds8";
	if (!strcmp(wdsdev, "wds0.10"))
		newdev = "wds9";

	if (!strcmp(wdsdev, "wds1.1"))
		newdev = "wds10";
	if (!strcmp(wdsdev, "wds1.2"))
		newdev = "wds11";
	if (!strcmp(wdsdev, "wds1.3"))
		newdev = "wds12";
	if (!strcmp(wdsdev, "wds1.4"))
		newdev = "wds13";
	if (!strcmp(wdsdev, "wds1.5"))
		newdev = "wds14";
	if (!strcmp(wdsdev, "wds1.6"))
		newdev = "wds15";
	if (!strcmp(wdsdev, "wds1.7"))
		newdev = "wds16";
	if (!strcmp(wdsdev, "wds1.8"))
		newdev = "wds17";
	if (!strcmp(wdsdev, "wds1.9"))
		newdev = "wds18";
	if (!strcmp(wdsdev, "wds1.10"))
		newdev = "wds19";
	return newdev;
}

void deconfigure_wifi(void)
{
}

void start_radius(void)
{
	char psk[64];

	// wrt-radauth $IFNAME $server $port $share $override $mackey $maxun &
	char ifname[32];

	char *prefix = "wl0";

	strcpy(ifname, "ra0");

	if (nvram_nmatch("1", "%s_radauth", prefix) && nvram_nmatch("ap", "%s_mode", prefix)) {
		char *server = nvram_nget("%s_radius_ipaddr", prefix);
		char *port = nvram_nget("%s_radius_port", prefix);
		char *share = nvram_nget("%s_radius_key", prefix);
		char type[32];

		sprintf(type, "%s_radmactype", prefix);
		char *pragma = "";

		if (nvram_default_matchi(type, 0, 0))
			pragma = "-n1 ";
		if (nvram_matchi(type, 1))
			pragma = "-n2 ";
		if (nvram_matchi(type, 2))
			pragma = "-n3 ";
		if (nvram_matchi(type, 3))
			pragma = "";
		sleep(1); // some delay is usefull
		sysprintf("wrt-radauth %s %s %s %s %s %s %s %s &", pragma, ifname, server, port, share,
			  nvram_nget("%s_radius_override", prefix), nvram_nget("%s_radmacpassword", prefix),
			  nvram_nget("%s_max_unauth_users", prefix));
	}

	prefix = "wl1";

	strcpy(ifname, "ba0");

	if (nvram_nmatch("1", "%s_radauth", prefix) && nvram_nmatch("ap", "%s_mode", prefix)) {
		char *server = nvram_nget("%s_radius_ipaddr", prefix);
		char *port = nvram_nget("%s_radius_port", prefix);
		char *share = nvram_nget("%s_radius_key", prefix);
		char type[32];

		sprintf(type, "%s_radmactype", prefix);
		char *pragma = "";

		if (nvram_default_matchi(type, 0, 0))
			pragma = "-n1 ";
		if (nvram_matchi(type, 1))
			pragma = "-n2 ";
		if (nvram_matchi(type, 2))
			pragma = "-n3 ";
		if (nvram_matchi(type, 3))
			pragma = "";
		sleep(1); // some delay is usefull
		sysprintf("wrt-radauth %s %s %s %s %s %s %s %s &", pragma, ifname, server, port, share,
			  nvram_nget("%s_radius_override", prefix), nvram_nget("%s_radmacpassword", prefix),
			  nvram_nget("%s_max_unauth_users", prefix));
	}
}

static int isSTA(int idx)
{
	if (nvram_nmatch("sta", "wl%d_mode", idx) || nvram_nmatch("wet", "wl%d_mode", idx) ||
	    nvram_nmatch("infra", "wl%d_mode", idx))
		return 1;
	return 0;
}

static int startradius[2] = { 0, 0 };

void configure_wifi_single(int idx) // madwifi implementation for atheros based
	// cards
{
	char var[64];
	char *next;

	startradius[idx] = 0;
#ifdef HAVE_DIR810L
	char mac[32];
	char mac5[32];
	strcpy(mac, nvram_default_get("et0macaddr_safe", "00:11:22:33:44:55"));
	strcpy(mac5, nvram_default_get("et0macaddr_safe", "00:11:22:33:44:55"));
	MAC_ADD(mac5);
	MAC_ADD(mac5);

#elif (defined(HAVE_DIR600) || defined(HAVE_AR670W) || defined(HAVE_AR690W) || defined(HAVE_VF803) || defined(HAVE_HAMEA15)) && \
	!defined(HAVE_ALL02310N)
	char mac[32];
	strcpy(mac, nvram_default_get("et0macaddr_safe", "00:11:22:33:44:55"));
	MAC_ADD(mac);
	MAC_ADD(mac);
#endif
	if (idx == 0) {
		stop_process("rt2860apd", "RALINK radius authenticator");
		eval("ifconfig", "ra0", "down");
		eval("ifconfig", "ra1", "down");
		eval("ifconfig", "ra2", "down");
		eval("ifconfig", "ra3", "down");
		eval("ifconfig", "ra4", "down");
		eval("ifconfig", "ra5", "down");
		eval("ifconfig", "ra6", "down");
		eval("ifconfig", "ra7", "down");
		eval("ifconfig", "wds0", "down");
		eval("ifconfig", "wds1", "down");
		eval("ifconfig", "wds2", "down");
		eval("ifconfig", "wds3", "down");
		eval("ifconfig", "wds4", "down");
		eval("ifconfig", "wds5", "down");
		eval("ifconfig", "wds6", "down");
		eval("ifconfig", "wds7", "down");
		eval("ifconfig", "wds8", "down");
		eval("ifconfig", "wds9", "down");
		eval("ifconfig", "apcli0", "down");
		eval("ifconfig", "ba0", "down");
		eval("ifconfig", "ba1", "down");
		eval("ifconfig", "ba2", "down");
		eval("ifconfig", "ba3", "down");
		eval("ifconfig", "ba4", "down");
		eval("ifconfig", "ba5", "down");
		eval("ifconfig", "ba6", "down");
		eval("ifconfig", "ba7", "down");
		eval("ifconfig", "wds10", "down");
		eval("ifconfig", "wds11", "down");
		eval("ifconfig", "wds12", "down");
		eval("ifconfig", "wds13", "down");
		eval("ifconfig", "wds14", "down");
		eval("ifconfig", "wds15", "down");
		eval("ifconfig", "wds16", "down");
		eval("ifconfig", "wds17", "down");
		eval("ifconfig", "wds18", "down");
		eval("ifconfig", "wds19", "down");
		eval("ifconfig", "apcli1", "down");
		rmmod("MT7610_ap");
		rmmod("RTPCI_ap");
		rmmod("mt_wifi");
		rmmod("rlt_wifi");
		rmmod("rt2860v2_ap");
		rmmod("rt2860v2_sta");
		rmmod("rt3062ap");
		rmmod("rt2860ap");
	}
	char *cname = "/tmp/RT2860.dat";
	if (idx == 1)
		cname = "/tmp/RT2860_pci.dat";
	if (nvram_nmatch("disabled", "wl%d_net_mode", idx)) {
		eval("rm", "-f", cname);
		return;
	}

	FILE *fp = fopen(cname,
			 "wb"); // config file for driver (don't ask me, its really the worst config thing i have seen)

	fprintf(fp, "Default\n");
	char wl[32];
	sprintf(wl, "wl%d", idx);
	char *dev = getWifiDeviceName(wl, NULL);
	if (dev && !strcmp(dev, "MT7615 802.11ac"))
		fprintf(fp, "E2pAccessMode=2\n");
	else if (idx == 1)
		fprintf(fp, "E2pAccessMode=2\n");

#ifdef BUFFALO_JP
	fprintf(fp, "CountryRegion=5\n");
	fprintf(fp, "CountryRegionABand=7\n");
	fprintf(fp, "CountryCode=JP\n");
#elif HAVE_BUFFALO
	char *region = nvram_safe_get("region");
	if (!strcmp(region, "EU")) {
		fprintf(fp, "CountryRegion=0\n");
		fprintf(fp, "CountryRegionABand=6\n");
		fprintf(fp, "CountryCode=DE\n");
	} else if (!strcmp(region, "RU")) {
		fprintf(fp, "CountryRegion=1\n");
		fprintf(fp, "CountryRegionABand=6n");
		fprintf(fp, "CountryCode=RU\n");
	} else if (!strcmp(region, "AP")) {
		fprintf(fp, "CountryRegion=1\n");
		fprintf(fp, "CountryRegionABand=7\n");
		fprintf(fp, "CountryCode=KR\n");
	} else if (!strcmp(region, "JP")) {
		fprintf(fp, "CountryRegion=1\n");
		fprintf(fp, "CountryRegionABand=1\n");
		fprintf(fp, "CountryCode=JP\n");
	} else { // US
		fprintf(fp, "CountryRegion=0\n");
		fprintf(fp, "CountryRegionABand=10\n");
		fprintf(fp, "CountryCode=US\n");
	}
#else
	fprintf(fp, "CountryRegion=5\n");
	fprintf(fp, "CountryRegionABand=7\n");
	fprintf(fp, "CountryCode=DE\n");
#endif
	int count = 2;

	char *vifs;

	if (isSTA(idx)) {
		fprintf(fp, "SSID=%s\n", nvram_nget("wl%d_ssid", idx));
		fprintf(fp, "BssidNum=1\n");
		if (nvram_nmatch("sta", "wl%d_mode", idx) || nvram_nmatch("wet", "wl%d_mode", idx))
			fprintf(fp, "NetworkType=Infra\n");
		if (nvram_nmatch("infra", "wl%d_mode", idx))
			fprintf(fp, "NetworkType=Adhoc\n");
		nvram_nset("", "wl%d_vifs", idx);
	} else {
		fprintf(fp, "SSID1=%s\n", nvram_nget("wl%d_ssid", idx));
		vifs = nvram_nget("wl%d_vifs", idx);

		if (vifs != NULL)
			foreach(var, vifs, next)
			{
				fprintf(fp, "SSID%d=%s\n", count, nvram_nget("%s_ssid", var));
				count++;
			}
		fprintf(fp, "BssidNum=%d\n", count - 1);
#ifdef HAVE_ESR6650
		fprintf(fp, "HT_TxStream=1\n");
		fprintf(fp, "HT_RxStream=1\n");
#elif HAVE_TECHNAXX3G
		fprintf(fp, "HT_TxStream=1\n");
		fprintf(fp, "HT_RxStream=1\n");
#elif HAVE_ESR9752
		fprintf(fp, "HT_TxStream=2\n");
		fprintf(fp, "HT_RxStream=2\n");
#elif HAVE_WCRGN
		fprintf(fp, "HT_TxStream=1\n");
		fprintf(fp, "HT_RxStream=1\n");
#elif HAVE_ACXNR22
		fprintf(fp, "HT_TxStream=2\n");
		fprintf(fp, "HT_RxStream=2\n");
#elif HAVE_EAP9550
		fprintf(fp, "HT_TxStream=2\n");
		fprintf(fp, "HT_RxStream=2\n");
#elif HAVE_DIR615
		fprintf(fp, "HT_TxStream=2\n");
		fprintf(fp, "HT_RxStream=2\n");
#elif HAVE_DIR600
		fprintf(fp, "HT_TxStream=1\n");
		fprintf(fp, "HT_RxStream=1\n");
#elif HAVE_RT3352
		fprintf(fp, "HT_TxStream=1\n");
		fprintf(fp, "HT_RxStream=1\n");
#elif HAVE_NEPTUNE
		fprintf(fp, "HT_TxStream=2\n");
		fprintf(fp, "HT_RxStream=2\n");
#elif HAVE_RT10N
		fprintf(fp, "HT_TxStream=1\n");
		fprintf(fp, "HT_RxStream=1\n");
#elif HAVE_W502U
		fprintf(fp, "HT_TxStream=1\n");
		fprintf(fp, "HT_RxStream=1\n");
#elif HAVE_RT3052
		if (idx && dev && !strcmp(dev, "MT7615 802.11ac")) {
			fprintf(fp, "HT_TxStream=4\n");
			fprintf(fp, "HT_RxStream=4\n");
		} else {
			fprintf(fp, "HT_TxStream=2\n");
			fprintf(fp, "HT_RxStream=2\n");
		}
#else
		fprintf(fp, "HT_TxStream=2\n");
		fprintf(fp, "HT_RxStream=3\n");
#endif
		fprintf(fp, "MaxStaNum=%s\n", nvram_nget("wl%d_maxassoc", idx));
	}
	/* suggestion by Jimmy */
	//      fprintf( fp, "HtBw=1\n" );
	char *refif = "ra0";
	if (idx == 1)
		refif = "ba0";

	if (nvram_nmatch("bg-mixed", "wl%d_net_mode", idx))
		fprintf(fp, "WirelessMode=0\n");
	if (nvram_nmatch("b-only", "wl%d_net_mode", idx))
		fprintf(fp, "WirelessMode=1\n");
	if (nvram_nmatch("a-only", "wl%d_net_mode", idx))
		fprintf(fp, "WirelessMode=2\n");
	if (nvram_nmatch("g-only", "wl%d_net_mode", idx))
		fprintf(fp, "WirelessMode=4\n");
	if (nvram_nmatch("n-only", "wl%d_net_mode", idx))
		fprintf(fp, "WirelessMode=6\n");
	if (nvram_nmatch("n2-only", "wl%d_net_mode", idx))
		fprintf(fp, "WirelessMode=6\n");
	if (nvram_nmatch("ng-only", "wl%d_net_mode", idx))
		fprintf(fp, "WirelessMode=7\n");
	if (nvram_nmatch("n5-only", "wl%d_net_mode", idx))
		fprintf(fp, "WirelessMode=11\n");
	if (nvram_nmatch("na-only", "wl%d_net_mode", idx))
		fprintf(fp, "WirelessMode=8\n");

	if (nvram_nmatch("mixed", "wl%d_net_mode", idx)) {
		if (has_5ghz(refif) && has_ac(refif))
			fprintf(fp, "WirelessMode=14\n");
		else
			fprintf(fp, "WirelessMode=9\n");
	}
	if (nvram_nmatch("ac-only", "wl%d_net_mode", idx)) {
		fprintf(fp, "WirelessMode=15\n");
		fprintf(fp, "VHT_DisallowNonVHT=1\n");
	}
	if (nvram_nmatch("acn-mixed", "wl%d_net_mode", idx)) {
		fprintf(fp, "WirelessMode=15\n");
	}

	char hidestr[64];

	hidestr[0] = 0;

	if (nvram_nmatch("1", "wl%d_closed", idx))
		strcat(hidestr, "1");
	else
		strcat(hidestr, "0");

	vifs = nvram_nget("wl%d_vifs", idx);
	if (vifs != NULL)
		foreach(var, vifs, next)
		{
			if (nvram_nmatch("1", "%s_closed", var))
				strcat(hidestr, ";1");
			else
				strcat(hidestr, ";0");
		}
	fprintf(fp, "HideSSID=%s\n", hidestr);
	fprintf(fp, "ShortSlot=%s\n", nvram_nmatch("long", "wl%d_shortslot", idx) ? "0" : "1");
	if (nvram_nmatch("0", "wl%d_channel", idx))
		fprintf(fp, "AutoChannelSelect=2\n");
	else
		fprintf(fp, "AutoChannelSelect=0\n");

	//encryption setup
	fprintf(fp, "IEEE80211H=0\n");
	char keyidstr[64] = { 0 };
	char encryptype[64] = { 0 };
	char authmode[64] = { 0 };
	char radius_server[256] = { 0 };
	char radius_port[256] = { 0 };
	char radius_key[1024] = { 0 };
	char x80211[32] = { 0 };
	char eapifname[64] = { 0 };

	if (nvram_match("wan_proto", "disabled"))
		fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr"));
	else {
		char *wip = get_wan_ipaddr();
		if (*wip)
			fprintf(fp, "own_ip_addr=%s\n", wip);
		else
			fprintf(fp, "own_ip_addr=%s\n", nvram_safe_get("lan_ipaddr"));
	}

	if (nvram_nmatch("", "wl%d_key", idx))
		strcat(keyidstr, "1");
	else
		strcat(keyidstr, nvram_nget("wl%d_key", idx));
	char tmp[256];
	if (idx == 0) {
		if (nvram_nmatch("0", "ra%d_bridged", idx))
			strcat(eapifname, "ra0");
		else
			strcat(eapifname, getBridge("ra0", tmp));
	} else {
		if (nvram_nmatch("0", "ba%d_bridged", idx))
			strcat(eapifname, "ba0");
		else
			strcat(eapifname, getBridge("ba0", tmp));
	}
	if (nvram_nmatch("wep", "wl%d_akm", idx)) {
		if (nvram_nmatch("shared", "wl%d_authmode", idx))
			strcat(authmode, "SHARED");
		else if (nvram_nmatch("auto", "wl%d_authmode", idx))
			strcat(authmode, "WEPAUTO");
		else
			strcat(authmode, "OPEN");
		strcat(encryptype, "WEP");
		strcat(x80211, "0");
		if (isSTA(idx)) {
			fprintf(fp, "Key1Str=%s\n", nvram_nget("wl%d_key1", idx));
			fprintf(fp, "Key2Str=%s\n", nvram_nget("wl%d_key2", idx));
			fprintf(fp, "Key3Str=%s\n", nvram_nget("wl%d_key3", idx));
			fprintf(fp, "Key4Str=%s\n", nvram_nget("wl%d_key4", idx));
			fprintf(fp, "Key1Type=0\n");
			fprintf(fp, "Key2Type=0\n");
			fprintf(fp, "Key3Type=0\n");
			fprintf(fp, "Key4Type=0\n");
		} else {
			fprintf(fp, "Key1Str1=%s\n", nvram_nget("wl%d_key1", idx));
			fprintf(fp, "Key2Str1=%s\n", nvram_nget("wl%d_key2", idx));
			fprintf(fp, "Key3Str1=%s\n", nvram_nget("wl%d_key3", idx));
			fprintf(fp, "Key4Str1=%s\n", nvram_nget("wl%d_key4", idx));
			fprintf(fp, "Key1Type=0\n");
			fprintf(fp, "Key2Type=0\n");
			fprintf(fp, "Key3Type=0\n");
			fprintf(fp, "Key4Type=0\n");
		}
		strcat(radius_server, "0.0.0.0");
		strcat(radius_port, "1812");
		strcat(radius_key, "ralink");
	}
	if (nvram_nmatch("disabled", "wl%d_akm", idx)) {
		strcat(authmode, "OPEN");
		strcat(encryptype, "NONE");
		strcat(x80211, "0");
		strcat(radius_server, "0.0.0.0");
		strcat(radius_port, "1812");
		strcat(radius_key, "ralink");
	}
	if (nvram_nmatch("psk2", "wl%d_akm", idx) || nvram_nmatch("psk", "wl%d_akm", idx) ||
	    nvram_nmatch("psk psk2", "wl%d_akm", idx)) {
		if (isSTA(idx))
			fprintf(fp, "WPAPSK=%s\n", nvram_nget("wl%d_wpa_psk", idx));
		else
			fprintf(fp, "WPAPSK1=%s\n", nvram_nget("wl%d_wpa_psk", idx));
		strcat(radius_server, "0.0.0.0");
		strcat(radius_port, "1812");
		strcat(radius_key, "ralink");
		strcat(x80211, "0");
		if (nvram_nmatch("tkip", "wl%d_crypto", idx))
			strcat(encryptype, "TKIP");
		if (nvram_nmatch("aes", "wl%d_crypto", idx))
			strcat(encryptype, "AES");
		if (nvram_nmatch("tkip+aes", "wl%d_crypto", idx))
			strcat(encryptype, "TKIPAES");
	}
	if (nvram_nmatch("wpa", "wl%d_akm", idx) || nvram_nmatch("wpa2", "wl%d_akm", idx) ||
	    nvram_nmatch("wpa wpa2", "wl%d_akm", idx)) {
		startradius[idx] = 1;
		if (isSTA(idx))
			fprintf(fp, "WPAPSK=%s\n", nvram_nget("wl%d_wpa_psk", idx));
		else
			fprintf(fp, "WPAPSK1=\n");
		strcat(radius_server, nvram_nget("wl%d_radius_ipaddr", idx));
		strcat(radius_port, nvram_nget("wl%d_radius_port", idx));
		strcat(radius_key, nvram_nget("wl%d_radius_key", idx));
		strcat(x80211, "0");
		if (nvram_nmatch("tkip", "wl%d_crypto", idx))
			strcat(encryptype, "TKIP");
		if (nvram_nmatch("aes", "wl%d_crypto", idx))
			strcat(encryptype, "AES");
		if (nvram_nmatch("tkip+aes", "wl%d_crypto", idx))
			strcat(encryptype, "TKIPAES");
	}

	if (nvram_nmatch("infra", "wl%d_mode", idx)) {
		strcat(authmode, "WPANONE");
	} else {
		if (nvram_nmatch("wpa", "wl%d_akm", idx))
			strcat(authmode, "WPA");
		if (nvram_nmatch("wpa2", "wl%d_akm", idx))
			strcat(authmode, "WPA2");
		if (nvram_nmatch("wpa wpa2", "wl%d_akm", idx))
			strcat(authmode, "WPA1WPA2");
		if (nvram_nmatch("psk2", "wl%d_akm", idx))
			strcat(authmode, "WPA2PSK");
		if (nvram_nmatch("psk", "wl%d_akm", idx))
			strcat(authmode, "WPAPSK");
		if (nvram_nmatch("psk psk2", "wl%d_akm", idx))
			strcat(authmode, "WPAPSKWPA2PSK");
	}
	if (nvram_nmatch("radius", "wl%d_akm", idx)) {
		startradius[idx] = 1;
		if (isSTA(idx))
			fprintf(fp, "WPAPSK=\n");
		else
			fprintf(fp, "WPAPSK1=\n");
		strcat(authmode, "OPEN");
		strcat(radius_server, nvram_nget("wl%d_radius_ipaddr", idx));
		strcat(radius_port, nvram_nget("wl%d_radius_port", idx));
		strcat(radius_key, nvram_nget("wl%d_radius_key", idx));
		strcat(x80211, "1");
		strcat(encryptype, "WEP");
	}
	count = 2;
	vifs = nvram_nget("wl%d_vifs", idx);
	if (vifs != NULL)
		foreach(var, vifs, next)
		{
			strcat(eapifname, ";");
			if (nvram_nmatch("0", "%s_bridged", getRADev(var)))
				strcat(eapifname, getRADev(var));
			else
				strcat(eapifname, getBridge(getRADev(var), tmp));
			strcat(keyidstr, ";");
			if (nvram_nmatch("", "%s_key", var))
				strcat(keyidstr, "1");
			else
				strcat(keyidstr, nvram_nget("%s_key", var));
			if (nvram_nmatch("disabled", "%s_akm", var)) {
				strcat(authmode, ";OPEN");
				strcat(encryptype, ";NONE");
				strcat(radius_server, ";0.0.0.0");
				strcat(radius_port, ";1812");
				strcat(radius_key, ";ralink");
				strcat(x80211, ";0");
			}
			if (nvram_nmatch("wep", "%s_akm", var)) {
				sprintf(radius_server, "%s;0.0.0.0", radius_server);
				sprintf(radius_port, "%s;1812", radius_port);
				sprintf(radius_key, "%s;ralink", radius_key);
				if (nvram_nmatch("shared", "%s_authmode", var))
					strcat(authmode, ";SHARED");
				else if (nvram_nmatch("auto", "%s_authmode", var))
					strcat(authmode, ";WEPAUTO");
				else
					strcat(authmode, ";OPEN");
				strcat(encryptype, ";WEP");
				strcat(x80211, ";0");
				fprintf(fp, "Key1Str%d=%s\n", count, nvram_nget("%s_key1", var));
				fprintf(fp, "Key2Str%d=%s\n", count, nvram_nget("%s_key2", var));
				fprintf(fp, "Key3Str%d=%s\n", count, nvram_nget("%s_key3", var));
				fprintf(fp, "Key4Str%d=%s\n", count, nvram_nget("%s_key4", var));
			}
			if (nvram_nmatch("psk", "%s_akm", var) || nvram_nmatch("psk psk2", "%s_akm", var) ||
			    nvram_nmatch("psk2", "%s_akm", var)) {
				sprintf(radius_server, "%s;0.0.0.0", radius_server);
				sprintf(radius_port, "%s;1812", radius_port);
				sprintf(radius_key, "%s;ralink", radius_key);
				strcat(x80211, ";0");
				fprintf(fp, "WPAPSK%d=%s\n", count, nvram_nget("%s_wpa_psk", var));
				if (nvram_nmatch("tkip", "%s_crypto", var))
					strcat(encryptype, ";TKIP");
				if (nvram_nmatch("aes", "%s_crypto", var))
					strcat(encryptype, ";AES");
				if (nvram_nmatch("tkip+aes", "%s_crypto", var))
					strcat(encryptype, ";TKIPAES");
			}
			if (nvram_nmatch("wpa", "%s_akm", var) || nvram_nmatch("wpa2", "%s_akm", var) ||
			    nvram_nmatch("wpa wpa2", "%s_akm", var)) {
				startradius[idx] = 1;
				fprintf(fp, "WPAPSK%d=\n", count);
				sprintf(radius_server, "%s;%s", radius_server, nvram_nget("%s_radius_ipaddr", var));
				sprintf(radius_port, "%s;%s", radius_port, nvram_nget("%s_radius_port", var));
				sprintf(radius_key, "%s;%s", radius_key, nvram_nget("%s_radius_key", var));
				strcat(x80211, ";0");
				if (nvram_nmatch("tkip", "%s_crypto", var))
					strcat(encryptype, ";TKIP");
				if (nvram_nmatch("aes", "%s_crypto", var))
					strcat(encryptype, ";AES");
				if (nvram_nmatch("tkip+aes", "%s_crypto", var))
					strcat(encryptype, ";TKIPAES");
			}
			if (nvram_nmatch("psk", "%s_akm", var))
				strcat(authmode, ";WPAPSK");
			if (nvram_nmatch("psk2", "%s_akm", var))
				strcat(authmode, ";WPA2PSK");
			if (nvram_nmatch("psk psk2", "%s_akm", var))
				strcat(authmode, ";WPAPSKWPA2PSK");
			if (nvram_nmatch("wpa", "%s_akm", var))
				strcat(authmode, ";WPA");
			if (nvram_nmatch("wpa2", "%s_akm", var))
				strcat(authmode, ";WPA2");
			if (nvram_nmatch("wpa wpa2", "%s_akm", var))
				strcat(authmode, ";WPA1WPA2");
			if (nvram_nmatch("radius", "%s_akm", var)) {
				startradius[idx] = 1;
				fprintf(fp, "WPAPSK%d=\n", count);
				strcat(authmode, ";OPEN");
				sprintf(radius_server, "%s;%s", radius_server, nvram_nget("%s_radius_ipaddr", var));
				sprintf(radius_port, "%s;%s", radius_port, nvram_nget("%s_radius_port", var));
				sprintf(radius_key, "%s;%s", radius_key, nvram_nget("%s_radius_key", var));
				strcat(x80211, ";1");
				strcat(encryptype, ";WEP");
			}

			count++;
		}

	fprintf(fp, "DefaultKeyID=%s\n", keyidstr);
	fprintf(fp, "EncrypType=%s\n", encryptype);
	fprintf(fp, "AuthMode=%s\n", authmode);

	fprintf(fp, "RADIUS_Server=%s\n", radius_server);
	fprintf(fp, "RADIUS_Port=%s\n", radius_port);
	fprintf(fp, "RADIUS_Key=%s\n", radius_key);
	fprintf(fp, "IEEE8021X=%s\n", x80211);

	fprintf(fp, "EAPifname=%s\n", eapifname);
	fprintf(fp, "PreAuthifname=%s\n", eapifname);

	//wds entries
	char wdsentries[128] = { 0 };
	int wdscount = 0;
	int s;

	for (s = 1; s <= 10; s++) {
		char wdsvarname[32] = { 0 };
		char wdsdevname[32] = { 0 };
		char wdsmacname[32] = { 0 };
		char *wdsdev;
		char *hwaddr;
		char dev[32];
		sprintf(dev, "wl%d", idx);
		sprintf(wdsvarname, "%s_wds%d_enable", dev, s);
		sprintf(wdsdevname, "%s_wds%d_if", dev, s);
		sprintf(wdsmacname, "%s_wds%d_hwaddr", dev, s);
		wdsdev = nvram_safe_get(wdsdevname);
		if (!*wdsdev)
			continue;
		if (nvram_matchi(wdsvarname, 0))
			continue;
		hwaddr = nvram_safe_get(wdsmacname);
		if (*hwaddr) {
			sprintf(wdsentries, "%s;%s", wdsentries, hwaddr);
			wdscount++;
		}
	}

	if (wdscount) {
		if (nvram_nmatch("1", "wl%d_lazy_wds", idx))
			fprintf(fp, "WdsEnable=4\n"); // 2 is exclusive
		else
			fprintf(fp, "WdsEnable=3\n"); // 2 is exclusive
		fprintf(fp,
			"WdsEncrypType=NONE\n"); //for now we do not support encryption
		fprintf(fp, "WdsList=%s\n", wdsentries);
		fprintf(fp, "WdsKey=\n");
	} else {
		fprintf(fp, "WdsEnable=0\n");
		fprintf(fp, "WdsEncrypType=NONE\n");
		fprintf(fp, "WdsList=\n");
		fprintf(fp, "WdsKey=\n");
	}

	//channel width
	if (nvram_nmatch("20", "wl%d_nbw", idx))
		fprintf(fp, "HT_BW=0\n");
	else if (nvram_nmatch("40", "wl%d_nbw", idx))
		fprintf(fp, "HT_BW=1\n");
	else if (nvram_nmatch("80", "wl%d_nbw", idx)) {
		fprintf(fp, "HT_BW=1\n");
		fprintf(fp, "VHT_BW=1\n");
	} else if (nvram_nmatch("160", "wl%d_nbw", idx)) {
		fprintf(fp, "HT_BW=1\n");
		fprintf(fp, "VHT_BW=2\n");
	} else if (nvram_nmatch("80+80", "wl%d_nbw", idx)) {
		fprintf(fp, "HT_BW=1\n");
		fprintf(fp, "VHT_BW=3\n");
	}
	//VHT_Sec80_Channel for 80+80

	int channel = atoi(nvram_nget("wl%d_channel", idx));

	int ext_chan = 0;
	if (idx == 0) {
		if (nvram_match("wl0_nctrlsb", "ll") || nvram_match("wl0_nctrlsb", "lower") || nvram_match("wl0_nctrlsb", "lu"))
			ext_chan = 1;
		if (channel <= 4)
			ext_chan = 1;
		if (channel >= 10)
			ext_chan = 0;
	} else {
		if (nvram_match("wl1_nctrlsb", "ll") || nvram_match("wl1_nctrlsb", "lower") || nvram_match("wl1_nctrlsb", "lu"))
			ext_chan = 1;
	}
	fprintf(fp, "HT_EXTCHA=%d\n", ext_chan);

	int mcs;
	char gf[32];
	sprintf(gf, "wl%d_greenfield", idx);
	char nmcs[32];
	sprintf(nmcs, "wl%d_nmcsidx", idx);
	if (nvram_default_matchi(gf, 1, 0))
		fprintf(fp, "HT_OpMode=1\n"); // green field mode
	else
		fprintf(fp, "HT_OpMode=0\n");

	mcs = nvram_default_geti(nmcs, -1);
	if (mcs == -1)
		fprintf(fp, "HT_MCS=33\n");
	else
		fprintf(fp, "HT_MCS=%d\n", mcs);

	//txrate
	if (nvram_nmatch("0", "wl%d_rate", idx))
		fprintf(fp, "TxRate=0\n");
	else if (nvram_nmatch("1000000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=1\n");
	else if (nvram_nmatch("2000000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=2\n");
	else if (nvram_nmatch("5500000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=3\n");
	else if (nvram_nmatch("6000000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=5\n");
	else if (nvram_nmatch("9000000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=6\n");
	else if (nvram_nmatch("1100000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=4\n");
	else if (nvram_nmatch("1200000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=7\n");
	else if (nvram_nmatch("1800000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=8\n");
	else if (nvram_nmatch("2400000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=9\n");
	else if (nvram_nmatch("3600000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=10\n");
	else if (nvram_nmatch("4800000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=11\n");
	else if (nvram_nmatch("5400000", "wl%d_rate", idx))
		fprintf(fp, "TxRate=12\n");
	else
		fprintf(fp, "TxRate=0\n");

	if (nvram_nmatch("sta", "wl%d_mode", idx) || nvram_nmatch("wet", "wl%d_mode", idx))
		fprintf(fp, "Channel=0\n");
	else
		fprintf(fp, "Channel=%s\n", nvram_nget("wl%d_channel", idx));
	if (nvram_nmatch("12", "wl%d_rateset", idx))
		fprintf(fp, "BasicRate=3\n");
	if (nvram_nmatch("default", "wl%d_rateset", idx))
		fprintf(fp, "BasicRate=15\n");
	if (nvram_nmatch("all", "wl%d_rateset", idx))
		fprintf(fp, "BasicRate=351\n");
	fprintf(fp, "BeaconPeriod=%s\n", nvram_nget("wl%d_bcn", idx));
	fprintf(fp, "DtimPeriod=%s\n", nvram_nget("wl%d_dtim", idx));
	fprintf(fp, "TxPower=%s\n", nvram_nget("wl%d_txpwr", idx)); // warning. percentage this time
	fprintf(fp, "DisableOLBC=0\n"); //what is this?
	fprintf(fp, "BGProtection=%s\n", nvram_nmatch("auto", "wl%d_gmode_protection", idx) ? "0" : "2");
	fprintf(fp, "TXPreamble=%s\n", nvram_nmatch("long", "wl%d_plcphdr", idx) ? "0" : "1");
	fprintf(fp, "RTSThreshold=%s\n", nvram_nget("wl%d_rts", idx));
	fprintf(fp, "FragThreshold=%s\n", nvram_nget("wl%d_frag", idx));
	fprintf(fp, "TxBurst=%s\n", nvram_nmatch("on", "wl%d_frameburst", idx) ? "0" : "1");
	fprintf(fp, "PktAggregate=0\n"); // ralink propertiery, do not use
	fprintf(fp, "TurboRate=0\n");
	fprintf(fp, "WmmCapable=%s\n", nvram_nmatch("on", "wl%d_wme", idx) ? "1" : "0");
	fprintf(fp, "APAifsn=3;7;1;1\n");
	fprintf(fp, "APCwmin=4;4;3;2\n");
	fprintf(fp, "APCwmax=6;10;4;3\n");
	fprintf(fp, "APTxop=0;0;94;47\n");
	fprintf(fp, "APACM=0;0;0;0\n");
	fprintf(fp, "BSSAifsn=3;7;2;2\n");
	fprintf(fp, "BSSCwmin=4;4;3;2\n");
	fprintf(fp, "BSSCwmax=10;10;4;3\n");
	fprintf(fp, "BSSTxop=0;0;94;47\n");
	fprintf(fp, "BSSACM=0;0;0;0\n");
	fprintf(fp, "AckPolicy=0;0;0;0\n");
	fprintf(fp, "NoForwarding=%s\n", nvram_nget("wl%d_ap_isolate", idx)); //between lan and ap
	fprintf(fp, "NoForwardingBTNBSSID=%s\n", nvram_nget("wl%d_ap_isolate", idx)); // between bssid

	//station

	if (nvram_nmatch("apsta", "wl%d_mode", idx) || nvram_nmatch("apstawet", "wl%d_mode", idx)) {
		char *essid = nvram_nget("wl%d_ssid", idx);
		char *key = NULL;
		fprintf(fp, "ApCliEnable=1\n");
		fprintf(fp, "ApCliSsid=%s\n", nvram_nget("wl%d_ssid", idx));
		if (nvram_nmatch("psk", "wl%d_akm", idx) || nvram_nmatch("psk2", "wl%d_akm", idx) ||
		    nvram_nmatch("psk psk2", "wl%d_akm", idx)) {
			if (nvram_nmatch("psk", "wl%d_akm", idx)) {
				if (nvram_nmatch("tkip", "wl%d_crypto", idx))
					fprintf(fp, "ApCliEncrypType=TKIP\n");
				if (nvram_nmatch("aes", "wl%d_crypto", idx))
					fprintf(fp, "ApCliEncrypType=AES\n");
				if (nvram_nmatch("tkip+aes", "wl%d_crypto", idx))
					fprintf(fp, "ApCliEncrypType=TKIPAES\n");
				fprintf(fp, "ApCliAuthMode=WPAPSK\n");
			}
			if (nvram_nmatch("psk2", "wl%d_akm", idx)) {
				if (nvram_nmatch("tkip", "wl%d_crypto", idx))
					fprintf(fp, "ApCliEncrypType=TKIP\n");
				if (nvram_nmatch("aes", "wl%d_crypto", idx))
					fprintf(fp, "ApCliEncrypType=AES\n");
				if (nvram_nmatch("tkip+aes", "wl%d_crypto", idx))
					fprintf(fp, "ApCliEncrypType=TKIPAES\n");
				fprintf(fp, "ApCliAuthMode=WPA2PSK\n");
			}
			key = nvram_nget("wl%d_wpa_psk", idx);
			fprintf(fp, "ApCliWPAPSK=%s\n", nvram_nget("wl%d_wpa_psk", idx));
		}
		if (nvram_nmatch("disabled", "wl%d_akm", idx)) {
			fprintf(fp, "ApCliEncrypType=NONE\n");
			fprintf(fp, "ApCliAuthMode=OPEN\n");
			key = "";
		}
		if (nvram_nmatch("wep", "wl%d_akm", idx)) {
			fprintf(fp, "ApCliEncrypType=WEP\n");
			if (nvram_nmatch("shared", "wl%d_authmode"))
				fprintf(fp, "ApCliAuthMode=SHARED\n");
			else if (nvram_nmatch("auto", "wl%d_authmode"))
				fprintf(fp, "ApCliAuthMode=WEPAUTO\n");
			else
				fprintf(fp, "ApCliAuthMode=OPEN\n");
			fprintf(fp, "ApCliDefaultKeyID=%s\n", nvram_nget("wl%d_key", idx));
			fprintf(fp, "ApCliKey1Type=0\n");
			fprintf(fp, "ApCliKey2Type=0\n");
			fprintf(fp, "ApCliKey3Type=0\n");
			fprintf(fp, "ApCliKey4Type=0\n");
			fprintf(fp, "ApCliKey1Str=%s\n", nvram_nget("wl%d_key1", idx));
			fprintf(fp, "ApCliKey2Str=%s\n", nvram_nget("wl%d_key2", idx));
			fprintf(fp, "ApCliKey3Str=%s\n", nvram_nget("wl%d_key3", idx));
			fprintf(fp, "ApCliKey4Str=%s\n", nvram_nget("wl%d_key4", idx));
			int keyidx = atoi(nvram_nget("wl%d_key", idx));
			key = nvram_nget("wl%d_key%d", idx, keyidx);
		}
		if (idx == 0)
			log_eval("ap_client", "ra0", "apcli0", essid, key);
		else
			log_eval("ap_client", "ba0", "apcli1", essid, key);
	} else {
		fprintf(fp, "ApCliEnable=0\n");
	}

	fprintf(fp, "CSPeriod=10\n");
	fprintf(fp, "WirelessEvent=0\n");
	fprintf(fp, "PreAuth=0\n");
	fprintf(fp, "RekeyInterval=0\n");
	fprintf(fp, "RekeyMethod=DISABLE\n");
	fprintf(fp, "PMKCachePeriod=10\n");
	fprintf(fp, "HSCounter=0\n");
	fprintf(fp, "AccessPolicy0=0\n");
	fprintf(fp, "AccessControlList0=\n");
	fprintf(fp, "AccessPolicy1=0\n");
	fprintf(fp, "AccessControlList1=\n");
	fprintf(fp, "AccessPolicy2=0\n");
	fprintf(fp, "AccessControlList2=\n");
	fprintf(fp, "AccessPolicy3=0\n");
	fprintf(fp, "AccessControlList3=\n");
	fprintf(fp, "HT_HTC=0\n");
	fprintf(fp, "HT_RDG=1\n");
	fprintf(fp, "HT_LinkAdapt=0\n");
	fprintf(fp, "HT_MpduDensity=5\n");
	fprintf(fp, "HT_AutoBA=1\n");
	fprintf(fp, "HT_AMSDU=0\n");
	fprintf(fp, "HT_BAWinSize=64\n");
	fprintf(fp, "HT_GI=%d\n", nvram_nmatch("1", "wl%d_shortgi", idx) ? 1 : 0);
	fprintf(fp, "HT_STBC=1\n");
	if (has_ac(refif)) {
		if (has_ac(refif) && has_5ghz(refif)) {
			fprintf(fp, "HT_LDPC=1\n");
			fprintf(fp, "VHT_STBC=1\n");
			fprintf(fp, "VHT_LDPC=1\n");
			fprintf(fp, "VHT_SGI=%d\n", nvram_nmatch("1", "wl%d_shortgi", idx) ? 1 : 0);
		}
		fprintf(fp, "G_BAND_256QAM=%d\n", nvram_nmatch("1", "wl%d_turbo_qam", idx) ? 1 : 0);
		if (has_ac(refif) && has_5ghz(refif)) {
			fprintf(fp, "ITxBfEn=%d\n", 1 /*? nvram_nmatch("1", "wl%d_itxbf", idx) ? 1 : 0 */);
			fprintf(fp, "ETxBfEnCond=%d\n", 1 /*? nvram_nmatch("1", "wl%d_txbf", idx) ? 1 : 0 */);
			fprintf(fp, "MUTxRxEnable=%d\n", 1 /* ? nvram_nmatch("1", "wl%d_mubf", idx) ? 1 : 0 */);
		}
	}
	fclose(fp);

	if (isSTA(idx)) {
#if (defined(HAVE_DIR600) || defined(HAVE_AR670W) || defined(HAVE_AR690W) || defined(HAVE_VF803) || defined(HAVE_DIR810L) || \
     defined(HAVE_HAMEA15)) &&                                                                                               \
	!defined(HAVE_ALL02310N)
		if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00") &&
		    nvram_invmatch("def_whwaddr", "")) {
			sysprintf("insmod rt2860v2_sta mac=%s", nvram_safe_get("def_whwaddr"));
		} else {
			sysprintf("insmod rt2860v2_sta mac=%s", mac);
		}
#else
		if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00") &&
		    nvram_invmatch("def_whwaddr", "")) {
			sysprintf("insmod rt2860v2_sta mac=%s", nvram_safe_get("def_whwaddr"));
		} else {
			insmod("rt2860v2_sta");
		}
#endif
		char dev[32];
		sprintf(dev, "wl%d", idx);
		char bridged[32];
		char *raif = get_wl_instance_name(idx);
		sprintf(bridged, "%s_bridged", getRADev(dev));
		if (nvram_default_matchi(bridged, 1, 1)) {
			eval("ifconfig", raif, "0.0.0.0", "up");
			if (nvram_nmatch("infra", "wl%d_mode", idx)) {
				br_add_interface(getBridge(raif, tmp), raif);
			}
		} else {
			eval("ifconfig", raif, "mtu", getMTU(raif));
			eval("ifconfig", raif, "txqueuelen", getTXQ(raif));
			eval("ifconfig", raif, nvram_nget("%s_ipaddr", getRADev(dev)), "netmask",
			     nvram_nget("%s_netmask", getRADev(dev)), "up");
		}
		char vathmac[32];

		sprintf(vathmac, "wl%d_hwaddr", idx);
		char vmacaddr[32];

		getMacAddr(raif, vmacaddr, sizeof(vmacaddr));
		nvram_set(vathmac, vmacaddr);
		setupSupplicant(dev);
	} else {
		if (idx == 0) {
#if (defined(HAVE_DIR600) || defined(HAVE_AR670W) || defined(HAVE_AR690W) || defined(HAVE_VF803) || defined(HAVE_DIR810L) || \
     defined(HAVE_HAMEA15)) &&                                                                                               \
	!defined(HAVE_ALL02310N)
			if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00") &&
			    nvram_invmatch("def_whwaddr", "")) {
				sysprintf("insmod rt2860v2_ap mac=%s", nvram_safe_get("def_whwaddr"));
				if (nvram_matchi("rtchip", 3062))
					sysprintf("insmod /lib/rt3062/rt3062ap.ko mac=%s", nvram_safe_get("def_whwaddr"));
				else
					sysprintf("insmod /lib/rt3062/rt2860ap.ko mac=%s", nvram_safe_get("def_whwaddr"));
			} else {
				if (nvram_matchi("rtchip", 3062))
					sysprintf("insmod /lib/rt3062/rt3062ap.ko mac=%s", mac);
				else
					sysprintf("insmod /lib/rt3062/rt2860ap.ko mac=%s", mac);
				sysprintf("insmod rt2860v2_ap mac=%s", mac);
			}
#else
			if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00") &&
			    nvram_invmatch("def_whwaddr", "")) {
				sysprintf("insmod rt2860v2_ap mac=%s", nvram_safe_get("def_whwaddr"));
			} else {
				insmod("rt2860v2_ap");
			}
#endif
		} else {
			insmod("RTPCI_ap");
			insmod("rlt_wifi");
			insmod("mt_wifi");
#ifdef HAVE_DIR810L
			sysprintf("insmod MT7610_ap mac=%s\n", mac5);
#else
			insmod("MT7610_ap");
#endif
		}
	}
	if (getRouterBrand() == ROUTER_DIR882) {
		eval("iwpriv", "ra0", "set", "led_setting=00-00-00-00-02-00-00-02");
		eval("iwpriv", "ba0", "set", "led_setting=00-00-00-00-02-00-00-02");
	}
}

void init_network(int idx)
{
	char var[64];
	char *next;
	char *vifs;
	char dev[32];
	sprintf(dev, "wl%d", idx);
	char bridged[32];
	char *raif = get_wl_instance_name(idx);
	char apcliif[32];
	int s;
	char tmp[256];

	if (!isSTA(idx)) {
		sprintf(apcliif, "apcli%d", idx);

		sprintf(bridged, "%s_bridged", getRADev(dev));
		if (nvram_default_matchi(bridged, 1, 1)) {
			if (getSTA() || getWET()) {
				eval("ifconfig", raif, "0.0.0.0", "up");
				eval("ifconfig", apcliif, "0.0.0.0", "up");
				br_add_interface(getBridge(apcliif, tmp), raif);
				if (getWET())
					br_add_interface(getBridge(apcliif, tmp), apcliif);
				eval("ifconfig", apcliif, "0.0.0.0", "up");
			} else {
				eval("ifconfig", raif, "0.0.0.0", "up");
				br_add_interface(getBridge(raif, tmp), raif);
				eval("ifconfig", raif, "0.0.0.0", "up");
			}
		} else {
			if (getSTA() || getWET()) {
				eval("ifconfig", raif, "0.0.0.0", "up");
				eval("ifconfig", apcliif, "mtu", getMTU(apcliif));
				eval("ifconfig", apcliif, "txqueuelen", getTXQ(apcliif));
				eval("ifconfig", raif, nvram_nget("%s_ipaddr", getRADev(dev)), "netmask",
				     nvram_nget("%s_netmask", getRADev(dev)), "up");
			} else {
				eval("ifconfig", raif, "0.0.0.0", "up");
				eval("ifconfig", raif, "txqueuelen", getTXQ(raif));
				eval("ifconfig", raif, nvram_nget("%s_ipaddr", getRADev(dev)), "netmask",
				     nvram_nget("%s_netmask", getRADev(dev)), "up");
			}
		}
		char vathmac[32];

		sprintf(vathmac, "wl%d_hwaddr", idx);
		char vmacaddr[32];

		getMacAddr(raif, vmacaddr, sizeof(vmacaddr));
		nvram_set(vathmac, vmacaddr);

		vifs = nvram_nget("wl%d_vifs", idx);
		if (vifs != NULL && *vifs) {
			int count = 1;

			foreach(var, vifs, next)
			{
				sprintf(bridged, "%s_bridged", getRADev(var));
				if (nvram_default_matchi(bridged, 1, 1)) {
					char ra[32];

					sprintf(ra, "ra%d", count + (8 * idx));
					eval("ifconfig", ra, "0.0.0.0", "up");
					br_add_interface(getBridge(getRADev(var), tmp), ra);
				} else {
					char ip[32];
					char mask[32];

					sprintf(ip, "%s_ipaddr", getRADev(var));
					sprintf(mask, "%s_netmask", getRADev(var));
					char raa[32];
					sprintf(raa, "ra%d", count + (8 * idx));

					eval("ifconfig", raa, "mtu", getMTU(raa));
					eval("ifconfig", raa, "txqueuelen", getTXQ(raa));
					eval("ifconfig", raa, nvram_safe_get(ip), "netmask", nvram_safe_get(mask), "up");
				}

				sprintf(vathmac, "%s_hwaddr", var);
				getMacAddr(getRADev(var), vmacaddr, sizeof(vmacaddr));
				nvram_set(vathmac, vmacaddr);

				count++;
			}
		}

		for (s = 1; s <= 10; s++) {
			char wdsvarname[32] = { 0 };
			char wdsdevname[32] = { 0 };
			char wdsmacname[32] = { 0 };
			char *wdsdev;
			char dev[32];
			char *hwaddr;
			sprintf(dev, "wl%d", idx);
			sprintf(wdsvarname, "%s_wds%d_enable", dev, (11 - s));
			sprintf(wdsdevname, "%s_wds%d_if", dev, (11 - s));
			sprintf(wdsmacname, "%s_wds%d_hwaddr", dev, (11 - s));
			wdsdev = nvram_safe_get(wdsdevname);
			if (!*wdsdev)
				continue;
			if (nvram_matchi(wdsvarname, 0))
				continue;
			hwaddr = nvram_safe_get(wdsmacname);
			if (*hwaddr) {
				eval("ifconfig", wdsdev, "0.0.0.0", "down");
				eval("ifconfig", wdsdev, "0.0.0.0", "up");
			}
		}

		/*

		   set macfilter
		 */

		if (startradius[idx]) {
			if (idx == 0)
				log_eval("rt2860apd");
			else
				log_eval("rt2860apd", "-i", "ba");
		}
		setMacFilter(dev);
	}
	vifs = nvram_nget("wl%d_vifs", idx);
	if (vifs != NULL && *vifs) {
		foreach(var, vifs, next)
		{
			setMacFilter(var);
		}
	}

	start_radius();

	int c;

	char br1enable[32];
	char br1ipaddr[32];
	char br1netmask[32];
	char word[256];

	sprintf(br1enable, "wl%d_br1_enable", idx);
	sprintf(br1ipaddr, "wl%d_br1_ipaddr", idx);
	sprintf(br1netmask, "wl%d_br1_netmask", idx);
	if (!nvram_exists(br1enable))
		nvram_seti(br1enable, 0);
	if (!nvram_exists(br1ipaddr))
		nvram_set(br1ipaddr, "0.0.0.0");
	if (!nvram_exists(br1netmask))
		nvram_set(br1netmask, "255.255.255.0");
	if (nvram_matchi(br1enable, 1)) {
		ifconfig("br1", 0, 0, 0);

		// eval ("ifconfig", "br1", "down");
		br_del_bridge("br1");
		br_add_bridge("br1");

		// "br1", "off");
		br_set_bridge_max_age("br1", getBridgeMaxAge("br1"));
		br_set_bridge_forward_delay("br1", getBridgeForwardDelay("br1"));

		/*
		 * Bring up and configure br1 interface 
		 */
		if (nvram_invmatch(br1ipaddr, "0.0.0.0")) {
			ifconfig("br1", IFUP, nvram_safe_get(br1ipaddr), nvram_safe_get(br1netmask));
			br_set_stp_state("br1", getBridgeSTP("br1", word));
			sleep(2);
		}
	}

	for (s = 1; s <= MAX_WDS_DEVS; s++) {
		char wdsvarname[32] = { 0 };
		char wdsdevname[32] = { 0 };
		char *dev;

		char br1enable[32];

		sprintf(wdsvarname, "wl%d_wds%d_enable", idx, s);
		sprintf(wdsdevname, "wl%d_wds%d_if", idx, s);
		sprintf(br1enable, "wl%d_br1_enable", idx);
		if (!nvram_exists(wdsvarname))
			nvram_seti(wdsvarname, 0);
		dev = nvram_safe_get(wdsdevname);
		if (!*dev)
			continue;

		// eval ("ifconfig", dev, "down");
		if (nvram_matchi(wdsvarname, 1)) {
			char *wdsip;
			char *wdsnm;
			char wdsbc[32] = { 0 };
			wdsip = nvram_nget("wl%d_wds%d_ipaddr", idx, s);
			wdsnm = nvram_nget("wl%d_wds%d_netmask", idx, s);

			snprintf(wdsbc, 31, "%s", wdsip);
			get_broadcast(wdsbc, sizeof(wdsbc), wdsnm);
			eval("ifconfig", dev, wdsip, "broadcast", wdsbc, "netmask", wdsnm, "up");
		} else if (nvram_matchi(wdsvarname, 2) && nvram_matchi(br1enable, 1)) {
			eval("ifconfig", dev, "up");
			sleep(1);
			br_add_interface("br1", dev);
		} else if (nvram_matchi(wdsvarname, 3)) {
			ifconfig(dev, IFUP, 0, 0);
			sleep(1);
			br_add_interface(getBridge(dev, tmp), dev);
		}
	}

	reset_hwaddr(nvram_safe_get("lan_ifname"));
}

void start_hostapdwan(void)
{
	int idx;
	stop_process("rt2860apd", "RALINK radius authenticator");
	for (idx = 0; idx < 2; idx++) {
		if (!isSTA(idx) && startradius[idx]) {
			if (idx == 0)
				log_eval("rt2860apd");
			else
				log_eval("rt2860apd", "-i", "ba");
		}
	}
}

void configure_wifi(void)
{
	deconfigure_wifi();
	configure_wifi_single(0);
	if (get_wl_instances() == 2)
		configure_wifi_single(1);
	init_network(0);
	if (get_wl_instances() == 2)
		init_network(1);
	eval("ifconfig", "ra0", "down");
	sleep(1);
	eval("ifconfig", "ra0", "up");
	if (get_wl_instances() == 2) {
		eval("ifconfig", "ba0", "down");
		sleep(1);
		eval("ifconfig", "ba0", "up");
	}
}

void start_configurewifi(void)
{
	configure_wifi();
}

void start_deconfigurewifi(void)
{
	deconfigure_wifi();
}
#endif
