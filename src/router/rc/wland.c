/*
 * wland.c
 *
 * Copyright (C) 2006 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#undef HAVE_DDLAN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <wait.h>
#include <ctype.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlioctl.h>
#include <rc.h>

#include <cy_conf.h>
#include <utils.h>
#include <wlutils.h>
#include <nvparse.h>

#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 1000
#define TXPWR_DEFAULT 70

#ifdef HAVE_MADWIFI
#define WLAND_INTERVAL 60
#else
#define WLAND_INTERVAL 15
#endif

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

static int do_ap_watchdog(void)
{
	/* 
	 * AP Watchdog - experimental check and fix for hung AP 
	 */
	int val = 0;
	struct stat s;
	static time_t last;
	int interval = nvram_geti("apwatchdog_interval") > WLAND_INTERVAL ? nvram_geti("apwatchdog_interval") : WLAND_INTERVAL;

	system("wl assoclist 2>&1 > /tmp/.assoclist");
	stat("/tmp/.assoclist", &s);
	unlink("/tmp/.assoclist");

	if (s.st_size <= 0 && time(NULL) - last > interval && nvram_matchi("apwatchdog_enable", 1) &&
	    nvram_invmatch("wl_net_mode", "disabled")) {
		time(&last);
		wlconf_down(get_wdev());

		val = nvram_geti("wl0_channel") + 1;
		if (val <= 2 || val >= 14)
			val = 2;

		wl_ioctl(get_wdev(), WLC_SET_CHANNEL, &val, sizeof(val));
		wl_ioctl(get_wdev(), WLC_UP, NULL, 0);

		wlconf_down(get_wdev());
		wlconf_up(get_wdev());
	}

	return 0;
}

#ifdef HAVE_AQOS
int compareNet(char *ip, char *net, char *dest)
{
	if (ip == NULL || net == NULL || dest == NULL)
		return 0;
	// fprintf(stderr,"compare ip%s/net%s with %s\n",ip,net,dest);
	char ips2[32];
	char dest2[32];
	strcpy(ips2, ip);
	strcpy(dest2, dest);
	ip = ips2;
	dest = dest2;

	unsigned int ip1 = atoi(strsep(&ip, "."));
	unsigned int ip2 = atoi(strsep(&ip, "."));
	unsigned int ip3 = atoi(strsep(&ip, "."));
	unsigned int ip4 = atoi(ip);

	unsigned int dip1 = atoi(strsep(&dest, "."));
	unsigned int dip2 = atoi(strsep(&dest, "."));
	unsigned int dip3 = atoi(strsep(&dest, "."));
	unsigned int dip4 = atoi(dest);

	unsigned int fullip = (ip1 << 24) | (ip2 << 16) | (ip3 << 8) | ip4;
	unsigned int dfullip = (dip1 << 24) | (dip2 << 16) | (dip3 << 8) | dip4;
	int bit = atoi(net);
	unsigned long long n = (unsigned long long)1 << (unsigned long long)bit; // convert

	//
	//
	//
	// net
	// to
	// full
	// mask
	int shift = 32 - bit;

	n--;
	n <<= shift;
	/* 
	 * fprintf(stderr, "compare %08X with %08X\n",(unsigned
	 * int)(dfullip&(unsigned int)n),(unsigned int)fullip&(unsigned int)n);
	 * fprintf(stderr, "fullip %08X\n",fullip); fprintf(stderr, "dfullip
	 * %08X\n",dfullip); fprintf(stderr, "n %08X\n",(unsigned int)n);
	 * fprintf(stderr, "nl %08lX\n",n); 
	 */
	if ((unsigned int)(dfullip & (unsigned int)n) == (unsigned int)(fullip & (unsigned int)n))
		return 1;
	return 0;
}

int containsIP(char *ip)
{
	FILE *in;
	char buf_ip[32];
	char *i, *net;
	char cip[32];

	strcpy(cip, ip);
	in = fopen("/tmp/aqos_ips", "rb");
	if (in == NULL)
		return 0;

	while (feof(in) == 0 && fscanf(in, "%s", buf_ip) == 1) {
		i = (char *)&buf_ip[0];
		net = strsep(&i, "/");
		if (compareNet(net, i, cip)) {
			fclose(in);
			return 1;
		}
		memset(buf_ip, 0, 32);
	}
	fclose(in);
	return 0;
}

static int qosidx = 1310;

int containsMAC(char *ip)
{
	FILE *in;
	char buf_ip[32];
	int x;

	in = fopen("/tmp/aqos_macs", "rb");
	if (in == NULL)
		return 0;

	for (x = 0; x < strlen(ip); x++)
		ip[x] = toupper(ip[x]);

	while (feof(in) == 0 && fscanf(in, "%s", buf_ip) == 1) {
		for (x = 0; x < strlen(buf_ip); x++)
			buf_ip[x] = toupper(buf_ip[x]);

		if (!strcmp(buf_ip, ip)) {
			fclose(in);
			return 1;
		}
	}
	fclose(in);
	return 0;
}

static void do_aqos_check(void)
{
	char wan_if_buffer[33];
	if (!nvram_invmatchi("wshaper_enable", 0))
		return;
	if (nvram_matchi("qos_done", 0))
		return;
	if (!nvram_invmatchi("svqos_defaults", 0))
		return;

	FILE *arp = fopen("/proc/net/arp", "rb");
	char ip_buf[32];
	char hw_buf[16];
	char fl_buf[16];
	char mac_buf[32];
	char mask_buf[16];
	char dev_buf[16];
	int cmac;
	int defaulup;
	int defauldown;
	int defaultlan;
	int cip;

	if (arp == NULL) {
		return;
	}
	defaulup = nvram_geti("default_uplevel");
	defauldown = nvram_geti("default_downlevel");
	defaultlan = nvram_geti("default_lanlevel");

	if (!defaulup || defaulup < 0) {
		fclose(arp);
		return;
	}
	if (!defauldown || defauldown < 0) {
		fclose(arp);
		return;
	}
	if (defaultlan < 0) {
		defaultlan = 0;
	}
	while (fgetc(arp) != '\n')
		;

	while (!feof(arp) && fscanf(arp, "%s %s %s %s %s %s", ip_buf, hw_buf, fl_buf, mac_buf, mask_buf, dev_buf) == 6) {
		char *wan = safe_get_wan_face(wan_if_buffer);

		if (wan && *wan && !strcmp(dev_buf, wan))
			continue;

		cmac = containsMAC(mac_buf);
		cip = containsIP(ip_buf);

		if (cip || cmac) {
			continue;
		}

		if (!cip && *ip_buf) {
			char ipnet[32];

			sprintf(ipnet, "%s/32", ip_buf);
			sysprintf("echo \"%s\" >>/tmp/aqos_ips", ipnet);
			if (*mac_buf)
				sysprintf("echo \"%s\" >>/tmp/aqos_macs", mac_buf);

			// create default rule for ip
			add_userip(ipnet, qosidx, defaulup, defauldown, defaultlan);
			qosidx += 10;
			memset(ip_buf, 0, 32);
			memset(mac_buf, 0, 32);
			continue;
		}
		if (!cmac && *mac_buf) {
			char ipnet[32];
			sprintf(ipnet, "%s/32", ip_buf);

			sysprintf("echo \"%s\" >>/tmp/aqos_macs", mac_buf);
			if (*ip_buf)
				sysprintf("echo \"%s\" >>/tmp/aqos_ips", ipnet);

			// create default rule for mac
			add_usermac(mac_buf, qosidx, defaulup, defauldown, defaultlan);
			qosidx += 10;
		}
		memset(ip_buf, 0, 32);
		memset(mac_buf, 0, 32);
	}
	fclose(arp);
}
#endif
#ifndef HAVE_MADWIFI

void start_wds_check(void)
{
	int s, sock;

	if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;

	/* 
	 * logic - if separate ip defined bring it up 
	 */
	/* 
	 * else if flagged for br1 and br1 is enabled add to br1 
	 */
	/* 
	 * else add it to the br0 bridge 
	 */
	int cnt = get_wl_instances();
#ifdef HAVE_QTN
	cnt = 1;
#endif
	int c;

	for (c = 0; c < cnt; c++) {
		for (s = 1; s <= MAX_WDS_DEVS; s++) {
			char *dev;
			struct ifreq ifr;

			dev = nvram_nget("wl%d_wds%d_if", c, s);

			if (nvram_nmatch("0", "wl%d_wds%d_enable", c, s))
				continue;

			memset(&ifr, 0, sizeof(struct ifreq));

			snprintf(ifr.ifr_name, IFNAMSIZ, dev);
			ioctl(sock, SIOCGIFFLAGS, &ifr);

			if ((ifr.ifr_flags & (IFF_RUNNING | IFF_UP)) == (IFF_RUNNING | IFF_UP))
				continue;

			/* 
			 * P2P WDS type 
			 */
			if (nvram_nmatch("1", "wl%d_wds%d_enable", c,
					 s)) // wds_s
			//
			//
			//
			{
				char wdsbc[32] = { 0 };
				char *wdsip = nvram_nget("wl%d_wds%d_ipaddr", c, s);
				char *wdsnm = nvram_nget("wl%d_wds%d_netmask", c, s);

				snprintf(wdsbc, 31, "%s", wdsip);
				get_broadcast(wdsbc, sizeof(wdsbc), wdsnm);
				eval("ifconfig", dev, wdsip, "broadcast", wdsbc, "netmask", wdsnm, "up");
			}
			/* 
			 * Subnet WDS type 
			 */
			else if (nvram_nmatch("2", "wl%d_wds%d_enable", c, s) && nvram_nmatch("1", "wl%d_br1_enable", c)) {
				eval("ifconfig", dev, "up");
				eval("brctl", "addif", "br1", dev);
			}
			/* 
			 * LAN WDS type 
			 */
			else if (nvram_nmatch("3", "wl%d_wds%d_enable", c,
					      s)) // wds_s
			//
			//
			//
			//
			//
			//
			//
			// disabled
			{
				eval("ifconfig", dev, "up");
				eval("brctl", "addif", "br0", dev);

				// notify_nas ("lan", "br0", "up");
			}
		}
	}
	close(sock);
	return;
}

static void do_ap_check(void)
{
	// if (nvram_match ("apwatchdog_enable", "1"))
	// do_ap_watchdog ();
	start_wds_check();
	// do_wds_check ();

	return;
}

int checkbssid(void)
{
	struct ether_addr bssid;
	wl_bss_info_t *bi;
	char buf[WLC_IOCTL_MAXLEN];
	char *ifname = getSTA();

	if (ifname == NULL)
		ifname = getWET();
	if (ifname == NULL)
		return 0;
	if ((WL_IOCTL(ifname, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0) {
		*(uint32 *)buf = WLC_IOCTL_MAXLEN;
		if ((WL_IOCTL(ifname, WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN)) < 0)
			return 0;
		bi = (wl_bss_info_t *)(buf + 4);
		int i;

		for (i = 0; i < 6; i++)
			if (bi->BSSID.octet[i] != 0)
				return 1;
	}
	return 0;
}

/* 
 * for Client/Wet mode 
 */
/* 
 * be nice to rewrite this to use sta_info_t if we had proper Broadcom API
 * specs 
 */
static void do_client_check(void)
{
	FILE *fp = NULL;
	char buf[1024];

	char *ifname = getSTA();
	if (ifname == NULL)
		ifname = getWET();
	if (ifname == NULL)
		return;
	// char mac[512];
	int len;
	int instance = get_wl_instance(ifname);

	char com[64];
	sprintf(com, "wl -i %s assoc 2>&1 > /tmp/.xassocx", ifname);
	system(com);

	if ((fp = fopen("/tmp/.xassocx", "r")) == NULL)
		return;

	len = fread(buf, 1, 1023, fp);

	buf[len] = 0;

	if ((len > 0 && strstr(buf, "Not associated.")) || checkbssid() == 0) {
#ifdef HAVE_DDLAN

		nvram_unset("cur_rssi");
		nvram_unset("cur_noise");
		nvram_unset("cur_bssid");
		nvram_unset("cur_snr");
		nvram_set("cur_state", "<span style=\"background-color: rgb(255, 0, 0);\">Nicht Verbunden</span>");

#endif
		eval("wl", "-i", ifname, "disassoc");
		if (nvram_matchi("roaming_enable", 1)) {
			eval("wl", "-i", ifname, "join", nvram_safe_get("roaming_ssid"));
		} else {
			eval("wl", "-i", ifname, "join", nvram_nget("wl%d_ssid", instance));
		}
		eval("stopservice", "nas");
		eval("startservice_f", "nas");
	} else {
#ifdef HAVE_DDLAN
		nvram_set("cur_state", "<span style=\"background-color: rgb(135, 255, 51);\">Verbunden</span>");
		eval("/sbin/check.sh");
#endif
	}
	fclose(fp);
	return;
}
#endif

#ifdef HAVE_MADWIFI

static int notstarted[32];
static char assoclist[24 * 1024];
static int lastchans[256];
static void do_madwifi_check(void)
{
	// fprintf(stderr,"do wlan check\n");
	int c = getdevicecount();
	char dev[32];
	int i, s;

	for (i = 0; i < c; i++) {
		sprintf(dev, "wlan%d", i);
		if (is_mac80211(dev))
			continue;
		if (nvram_nmatch("disabled", "%s_net_mode", dev))
			continue;
		if (nvram_matchi("wds_watchdog_debug", 1))
			for (s = 1; s <= 10; s++) {
				char wdsvarname[32] = { 0 };
				char wdsdevname[32] = { 0 };
				char wdsmacname[32] = { 0 };
				char *wdsdev;
				char *hwaddr;

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
					int count = getassoclist(wdsdev, &assoclist[0]);

					if (count < 1) {
						eval("ifconfig", wdsdev, "down");
						sleep(1);
						eval("ifconfig", wdsdev, "up");
						eval("startservice", "set_routes", "-f");
					}
				}
			}
		char mode[32];

		sprintf(mode, "%s_mode", dev);
		if (nvram_match(mode, "sta") || nvram_match(mode, "wdssta") || nvram_match(mode, "wet")) {
			int chan = wifi_getchannel(dev);

			// fprintf(stderr,"current channel %d\n",chan);
			if (lastchans[i] == -1 && chan < 1000)
				lastchans[i] = chan;
			else {
				// fprintf(stderr,"current channel %d =
				// %d\n",chan,lastchans[i]);
				if (chan == lastchans[i]) {
					int count = getassoclist(dev, &assoclist[0]);

					if (count == 0 || count == -1) {
						// fprintf(stderr,"get assoclist returns %d, restart
						// ifnames\n",count);
						char *next;
						char var[80];
						char *vifs;
						char mode[32];
						char *m;
						char wifivifs[32];

						sprintf(wifivifs, "%s_vifs", dev);
						vifs = nvram_safe_get(wifivifs);
						if (vifs != NULL && *vifs) {
							foreach(var, vifs, next)
							{
								eval("ifconfig", var, "down");
							}
						}

						notstarted[i] = 0;
						eval("ifconfig", dev, "down");
						sleep(1);
						eval("ifconfig", dev, "up");
						char power[32];
						sprintf(power, "%s_txpwrdbm", dev);
						int newpower = nvram_default_geti(power, 16);
						sysprintf("iwconfig %s txpower %ddBm", dev, newpower);
						eval("startservice", "set_routes", "-f");
						lastchans[i] = -1;
					} else if (!notstarted[i]) {
						notstarted[i] = 1;
						char *next;
						char var[80];
						char *vifs;
						char mode[32];
						char *m;
						char wifivifs[32];

						sprintf(wifivifs, "%s_vifs", dev);
						vifs = nvram_safe_get(wifivifs);
						if (vifs != NULL && *vifs) {
							foreach(var, vifs, next)
							{
								eval("ifconfig", var, "up");
								eval("startservice", "set_routes", "-f");
							}
						}
					}
				}
				lastchans[i] = chan;
			}
		}
	}
	// fprintf(stderr,"do wlancheck end\n");
}
#endif

#ifdef HAVE_MADWIFI
/* 
 * static HAL_MIB_STATS laststats[16]; void detectACK(void) { int count =
 * getdevicecount(); int i; int s = socket(AF_INET, SOCK_DGRAM, 0); for
 * (i=0;i<count;i++) { char wifi[16]; sprintf(wifi,"wifi%d",i); struct ifreq
 * ifr; strcpy(ifr.ifr_name, wifi); ifr.ifr_data = (caddr_t) &laststats[i];
 * if (ioctl(s, SIOCGATHMIBSTATS, &ifr) < 0) { fprintf(stderr,"Error while
 * gettting mib stats\n"); return; } }
 * 
 * close(s); } 
 */
#endif

/* 
 * static void setShortSlot(void) { char *shortslot = nvram_safe_get
 * ("wl0_shortslot");
 * 
 * else if (!strcmp (afterburner, "long")) eval ("wl", "shortslot_override",
 * "0"); else if (!strcmp (afterburner, "short")) eval ("wl",
 * "shortslot_override", "1");
 * 
 * }
 */

static void do_wlan_check(void)
{
#ifdef HAVE_AQOS
	do_aqos_check();
#endif
#ifndef HAVE_DHDAP
#ifndef HAVE_MADWIFI
	if (getSTA() || getWET())
		do_client_check();
	else
		do_ap_check();
#else

	do_madwifi_check();
#endif
#endif
}

int main(int argc, char **argv)
{
	/* 
	 * Run it in the background 
	 */
	switch (fork()) {
	case -1:
		// can't fork
		exit(0);
		break;
	case 0:
		/* 
		 * child process 
		 */
		// fork ok
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}
#ifdef HAVE_AQOS
	qosidx = 1310;
#endif
	/* 
	 * Most of time it goes to sleep 
	 */
#ifdef HAVE_MADWIFI
	memset(lastchans, -1, 256 * 4);
	memset(notstarted, 0, 32 * 4);
#endif
	while (1) {
		sleep(WLAND_INTERVAL);
		do_wlan_check();
		/*#if defined(HAVE_ATH10K) && !defined(HAVE_MVEBU)
		int c = getdevicecount();
		char dev[32];
		int i;
		for (i = 0; i < c; i++) {
			sprintf(dev, "wlan%d", i);
			char dst[32];
			sprintf(dst, "%s_distance", dev);
			if (is_ath10k(dev)) {	// evil hack for QCA 
				set_ath10kdistance(dev, nvram_geti(dst)));
			}
		}
#endif*/
	}

	return 0;
} // end main

/* 
 * void main(int argc, char **argv) { wland_main(argc,argv); } 
 */
