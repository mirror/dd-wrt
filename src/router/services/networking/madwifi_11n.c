/*
 * madwifi.c
 *
 * Copyright (C) 2005 - 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
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

#ifdef HAVE_MADWIFI_MIMO
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

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
#include <bcmutils.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>

#include "wireless.h"
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "net80211/ieee80211_ioctl.h"
//#include <iwlib.h>
#include <services.h>

static int setsysctrl(const char *dev, const char *control, u_long value)
{
	char val[32];
	sprintf(val, "%li", value);
	writevaproc(val, "/proc/sys/dev/%s/%s", dev, control);

	return 0;
}

static void setdistance(char *device, int distance, int chanbw)
{
	if (distance >= 0)
		setsysctrl(device, "distance", distance);
}

// returns the number of installed atheros devices/cards

void deconfigure_single_11n(int count)
{
	char *next;
	char dev[16];
	char var[80];
	char wifivifs[16];

	sprintf(wifivifs, "ath%d_vifs", count);
	sprintf(dev, "ath%d", count);
	char vifs[128];

	sprintf(vifs, "%s.1 %s.2 %s.3 %s.4 %s.5 %s.6 %s.7 %s.8 %s.9", dev, dev, dev, dev, dev, dev, dev, dev, dev);
	int s;

	for (s = 1; s <= 10; s++) {
		sprintf(dev, "ath%d.wds%d", count, s - 1);
		if (ifexists(dev)) {
			br_del_interface("br0", dev);
			sysprintf("ifconfig %s down", dev);
		}
	}
	sprintf(dev, "ath%d", count);
	if (ifexists(dev)) {
		br_del_interface("br0", dev);
		sysprintf("ifconfig %s down", dev);
	}
	foreach(var, vifs, next) {
		if (ifexists(var)) {
			sysprintf("ifconfig %s down", dev);
		}
	}
	sprintf(dev, "ath%d", count);

	if (ifexists(dev))
		sysprintf("80211n_wlanconfig %s destroy", dev);

	foreach(var, vifs, next) {
		if (ifexists(var)) {
			sysprintf("80211n_wlanconfig %s destroy", var);
		}
	}

}

static int need_commit = 0;

static int getMaxPower(char *ifname)
{
	char buf[128];

	sprintf(buf, "iwlist %s txpower|grep \"Maximum Power:\" > /tmp/.power", ifname);
	system2(buf);
	FILE *in = fopen("/tmp/.power", "rb");

	if (in == NULL)
		return 1000;
	char buf2[16];
	int max;

	fscanf(in, "%s %s %d", buf, buf2, &max);
	fclose(in);
	return max;
}

#define SIOCSSCANLIST  		(SIOCDEVPRIVATE+6)
static void set_scanlist(char *dev, char *wif)
{
	char var[32];
	char *next;
	struct iwreq iwr;
	char scanlist[32];
	char list[64];

	sprintf(scanlist, "%s_scanlist", dev);
	char *sl = nvram_default_get(scanlist, "default");
	int c = 0;

	sysprintf("iwpriv %s setscanlist -ALL", dev);
	if (strlen(sl) > 0 && strcmp(sl, "default")) {
		foreach(var, sl, next) {
			sprintf(list, "+%s", var);
			sysprintf("iwpriv %s setscanlist %s", dev, list);
		}
	} else {
		sysprintf("iwpriv %s setscanlist +ALL", dev);
	}
}

static void set_rate(char *dev, char *priv)
{
	char rate[32];
	char maxrate[32];
	char net[32];
	char bw[32];
	char xr[32];

	sprintf(bw, "%s_channelbw", dev);
	sprintf(net, "%s_net_mode", dev);
	sprintf(rate, "%s_minrate", dev);
	sprintf(maxrate, "%s_maxrate", dev);
	sprintf(xr, "%s_xr", dev);
	char *r = nvram_default_get(rate, "0");
	char *mr = nvram_default_get(maxrate, "0");

#ifdef HAVE_WHRAG108
	char *netmode;

	if (!strcmp(dev, "ath0"))
		netmode = nvram_default_get(net, "a-only");
	else
		netmode = nvram_default_get(net, "mixed");
#else
	char *netmode = nvram_default_get(net, "mixed");
#endif

	if (nvram_match(bw, "20") && nvram_match(xr, "0"))
		if (atof(r) == 27.0f || atof(r) == 1.5f || atof(r) == 2.0f || atof(r) == 3.0f || atof(r) == 4.5f || atof(r) == 9.0f || atof(r) == 13.5f) {
			nvram_set(rate, "0");
			r = "0";
		}
	if (nvram_match(bw, "40"))
		if (atof(r) == 27.0f || atof(r) == 1.5f || atof(r) == 2.0f || atof(r) == 3.0f || atof(r) == 4.5f || atof(r) == 9.0f || atof(r) == 13.5f) {
			nvram_set(rate, "0");
			r = "0";
		}
	if (nvram_match(bw, "10"))
		if (atof(r) > 27.0f || atof(r) == 1.5f || atof(r) == 2.0f || atof(r) == 13.5f) {
			nvram_set(rate, "0");
			r = "0";
		}
	if (nvram_match(bw, "5"))
		if (atof(r) > 13.5) {
			nvram_set(rate, "0");
			r = "0";
		}
	if (nvram_match(bw, "2"))
		if (atof(r) > 6.75) {
			nvram_set(rate, "0");
			r = "0";
		}
/*
//must be rewritten

	if (!strcmp(netmode, "b-only"))
		sysprintf("iwconfig %s rate 11M auto", priv);
	else {
		sysprintf("iwconfig %s rate 54M auto", priv);
	}
	if (atol(mr) > 0)
		sysprintf("iwpriv %s maxrate %s", priv, mr);
	if (atoi(r) > 0)
		sysprintf("iwpriv %s minrate %s", priv, r);
*/
}

static void setup_channel(char *dev, char *use)
{
	char *apm;
	char wl[32];
	char channel[32];
	sprintf(channel, "%s_channel", dev);
	sprintf(wl, "%s_mode", dev);
	apm = nvram_default_get(wl, "ap");
	if (strcmp(apm, "sta") && strcmp(apm, "wdssta") && strcmp(apm, "wet")) {
		cprintf("set channel\n");
		char *ch = nvram_default_get(channel, "0");

		if (strcmp(ch, "0") == 0) {
			sysprintf("iwconfig %s channel 0", use);
		} else {
			sysprintf("iwconfig %s freq %sM", use, ch);
		}
	}
}

static void set_netmode(char *wif, char *dev, char *use)
{
	static char net[16];
	static char mode[16];
	static char xr[16];
	static char comp[32];
	static char ff[16];
	static char bw[16];
	static char rxantenna[32];
	static char txantenna[32];

	sprintf(mode, "%s_mode", dev);
	sprintf(net, "%s_net_mode", dev);
	sprintf(bw, "%s_channelbw", dev);
	sprintf(xr, "%s_xr", dev);
	sprintf(txantenna, "%s_txantenna", dev);
	sprintf(rxantenna, "%s_rxantenna", dev);
//    sprintf( comp, "%s_compression", dev );
	sprintf(ff, "%s_ff", dev);
	char *netmode = nvram_default_get(net, "mixed");
	// fprintf (stderr, "set netmode of %s to %s\n", net, netmode);
	cprintf("configure net mode %s\n", netmode);

	{
		{
//                      sysprintf("iwpriv %s xr 0", use);
			if (!strcmp(netmode, "mixed")) {
				sysprintf("iwpriv %s mode 0", use);
				sysprintf("iwpriv %s pureg 0", use);
				sysprintf("iwpriv %s pureb 0", use);
				sysprintf("iwpriv %s puren 0", use);
				setup_channel(dev, use);
			}
			if (!strcmp(netmode, "b-only")) {
				sysprintf("iwpriv %s mode 2", use);
				sysprintf("iwpriv %s pureg 0", use);
				sysprintf("iwpriv %s puren 0", use);
				sysprintf("iwpriv %s pureb 1", use);
				setup_channel(dev, use);
			}
			if (!strcmp(netmode, "g-only")) {
				sysprintf("iwpriv %s mode 3", use);
				sysprintf("iwpriv %s puren 0", use);
				sysprintf("iwpriv %s pureb 0", use);
				sysprintf("iwpriv %s pureg 1", use);
				setup_channel(dev, use);
			}

			if (!strcmp(netmode, "bg-mixed")) {
				sysprintf("iwpriv %s mode 3", use);
				sysprintf("iwpriv %s puren 0", use);
				sysprintf("iwpriv %s pureb 0", use);
				sysprintf("iwpriv %s pureg 0", use);
			}

			if (!strcmp(netmode, "a-only")) {
				sysprintf("iwpriv %s mode 1", use);
				sysprintf("iwpriv %s pureb 0", use);
				sysprintf("iwpriv %s puren 0", use);
				sysprintf("iwpriv %s pureg 0", use);
				setup_channel(dev, use);
			}
		}
	}

	int up = 0;
	char sb[32];
	sprintf(sb, "%s_nctrlsb", dev);
	if (nvram_match(sb, "upper"))
		up = 1;
	if (nvram_default_match(bw, "40", "20")) {
		if (!strcmp(netmode, "g-only")) {
			sysprintf("iwpriv %s mode 6", use);
			sysprintf("iwpriv %s pureb 0", use);
			sysprintf("iwpriv %s puren 0", use);
			sysprintf("iwpriv %s pureg 1", use);
			setup_channel(dev, use);
		}

		if (!strcmp(netmode, "a-only")) {
			sysprintf("iwpriv %s mode 5", use);
			sysprintf("iwpriv %s pureb 0", use);
			sysprintf("iwpriv %s puren 0", use);
			sysprintf("iwpriv %s pureg 0", use);
			setup_channel(dev, use);
		}
		if (!strcmp(netmode, "ng-only") || !strcmp(netmode, "n2-only")) {
			if (up)
				sysprintf("iwpriv %s mode 11nbht40plus", use);
			else
				sysprintf("iwpriv %s mode 11nght40minus", use);
			sysprintf("iwpriv %s pureb 0", use);
			sysprintf("iwpriv %s puren 0", use);
			sysprintf("iwpriv %s pureg 0", use);

			setup_channel(dev, use);
			sysprintf("ifconfig %s txqueuelen 1000", use);
			sysprintf("ifconfig %s txqueuelen 1000", wif);

			sysprintf("iwpriv %s shortgi 1", use);
			sysprintf("iwpriv %s cwmmode 2", use);
			if (up)
				sysprintf("iwpriv %s extoffset 1", use);
			else
				sysprintf("iwpriv %s extoffset -1", use);
			sysprintf("iwpriv %s extprotspac 0", use);
		}
		if (!strcmp(netmode, "na-only") || !strcmp(netmode, "n5-only")) {
			if (up)
				sysprintf("iwpriv %s mode 11naht40plus", use);
			else
				sysprintf("iwpriv %s mode 11naht40minus", use);
			sysprintf("iwpriv %s pureb 0", use);
			sysprintf("iwpriv %s puren 0", use);
			sysprintf("iwpriv %s pureg 0", use);
			setup_channel(dev, use);
			sysprintf("ifconfig %s txqueuelen 1000", use);
			sysprintf("ifconfig %s txqueuelen 1000", wif);
			sysprintf("iwpriv %s shortgi 1", use);
			sysprintf("iwpriv %s cwmmode 2", use);
			if (up)
				sysprintf("iwpriv %s extoffset 1", use);
			else
				sysprintf("iwpriv %s extoffset -1", use);
			sysprintf("iwpriv %s extprotspac 0", use);
		}
	}
	if (nvram_default_match(bw, "2040", "20")) {
		if (!strcmp(netmode, "ng-only") || !strcmp(netmode, "n2-only")) {
			sysprintf("iwpriv %s mode 11nght20", use);
			sysprintf("iwpriv %s pureb 0", use);
			if (!strcmp(netmode, "na-only"))
				sysprintf("iwpriv %s puren 0", use);
			else
				sysprintf("iwpriv %s puren 1", use);
			sysprintf("iwpriv %s pureg 1", use);
			setup_channel(dev, use);
			sysprintf("ifconfig %s txqueuelen 1000", use);
			sysprintf("ifconfig %s txqueuelen 1000", wif);
			sysprintf("iwpriv %s shortgi 1", use);
			sysprintf("iwpriv %s cwmmode 1", use);
			sysprintf("iwpriv %s extoffset -1", use);
			sysprintf("iwpriv %s extprotspac 0", use);
		}
		if (!strcmp(netmode, "na-only") || !strcmp(netmode, "n5-only")) {
			sysprintf("iwpriv %s mode 11naht20", use);
			sysprintf("iwpriv %s pureb 0", use);
			if (!strcmp(netmode, "na-only"))
				sysprintf("iwpriv %s puren 0", use);
			else
				sysprintf("iwpriv %s puren 1", use);
			sysprintf("iwpriv %s pureg 0", use);
			sysprintf("ifconfig %s txqueuelen 1000", use);
			sysprintf("ifconfig %s txqueuelen 1000", wif);
			sysprintf("iwpriv %s shortgi 1", use);
			sysprintf("iwpriv %s cwmmode 1", use);
			sysprintf("iwpriv %s extoffset -1", use);
			sysprintf("iwpriv %s extprotspac 0", use);
		}
	}
	if (nvram_default_match(bw, "20", "20")) {
		if (!strcmp(netmode, "ng-only") || !strcmp(netmode, "n2-only")) {
			sysprintf("iwpriv %s mode 11nght20", use);
			sysprintf("iwpriv %s pureb 0", use);
			if (!strcmp(netmode, "ng-only"))
				sysprintf("iwpriv %s puren 0", use);
			else
				sysprintf("iwpriv %s puren 1", use);
			sysprintf("iwpriv %s pureg 1", use);
			setup_channel(dev, use);
			sysprintf("ifconfig %s txqueuelen 1000", use);
			sysprintf("ifconfig %s txqueuelen 1000", wif);
			sysprintf("iwpriv %s shortgi 1", use);
			sysprintf("iwpriv %s cwmmode 0", use);
			sysprintf("iwpriv %s extoffset -1", use);
			sysprintf("iwpriv %s extprotspac 0", use);
		}
		if (!strcmp(netmode, "na-only") || !strcmp(netmode, "n5-only")) {
			sysprintf("iwpriv %s mode 11naht20", use);
			sysprintf("iwpriv %s pureb 0", use);
			if (!strcmp(netmode, "na-only"))
				sysprintf("iwpriv %s puren 0", use);
			else
				sysprintf("iwpriv %s puren 1", use);
			sysprintf("iwpriv %s pureg 0", use);
			setup_channel(dev, use);
			sysprintf("ifconfig %s txqueuelen 1000", use);
			sysprintf("ifconfig %s txqueuelen 1000", wif);
			sysprintf("iwpriv %s shortgi 1", use);
			sysprintf("iwpriv %s cwmmode 0", use);
			sysprintf("iwpriv %s extoffset -1", use);
			sysprintf("iwpriv %s extprotspac 0", use);
		}
	}

	sysprintf("test -f /proc/sys/dev/ath/htdupieenable && echo 1 > /proc/sys/dev/ath/htdupieenable");
	sysprintf("iwpriv %s ampdu 1", use);
	sysprintf("iwpriv %s ampdulimit 50000", use);
#if defined(HAVE_WHRHPGN) || defined(HAVE_DIR615E) || defined(HAVE_WNR2000)
	sysprintf("iwpriv %s rx_chainmask %s", use, nvram_default_get(rxantenna, "3"));
	sysprintf("iwpriv %s tx_chainmask %s", use, nvram_default_get(txantenna, "3"));
	sysprintf("iwpriv %s tx_cm_legacy %s", use, nvram_default_get(txantenna, "3"));
#else
	sysprintf("iwpriv %s rx_chainmask %s", use, nvram_default_get(rxantenna, "7"));
	sysprintf("iwpriv %s tx_chainmask %s", use, nvram_default_get(txantenna, "5"));
	sysprintf("iwpriv %s tx_cm_legacy %s", use, nvram_default_get(txantenna, "5"));
#endif
}

static void setRTS(char *use)
{
	char rts[32];

	sprintf(rts, "%s_protmode", use);
	nvram_default_get(rts, "None");

	sprintf(rts, "%s_rts", use);
	nvram_default_get(rts, "0");

	sprintf(rts, "%s_rtsvalue", use);
	nvram_default_get(rts, "2346");

	if (nvram_nmatch("1", "%s_rts", use)) {
		sysprintf("iwconfig %s rts %s", use, nvram_nget("%s_rtsvalue", use));
	} else {
		sysprintf("iwconfig %s rts off", use);
	}
/*	if (nvram_nmatch("None", "%s_protmode", use))
		sysprintf("iwpriv %s protmode 0", use);
	if (nvram_nmatch("CTS", "%s_protmode", use))
		sysprintf("iwpriv %s protmode 1", use);
	if (nvram_nmatch("RTS/CTS", "%s_protmode", use))
		sysprintf("iwpriv %s protmode 2", use);*/
}

/*static void set_compression( int count )
{
    char comp[32];
    char wif[32];

    sprintf( wif, "wifi%d", count );
    sprintf( comp, "ath%d_compression", count );
    if( nvram_default_match( comp, "1", "0" ) )
	setsysctrl( wif, "compression", 1 );
    else
	setsysctrl( wif, "compression", 0 );
}
*/
static void setMacFilter(char *iface)
{
	char *next;
	char var[32];

	sysprintf("iwpriv %s maccmd 3", iface);

	char nvvar[32];

	sprintf(nvvar, "%s_macmode", iface);
	if (nvram_match(nvvar, "deny")) {
		sysprintf("iwpriv %s maccmd 2", iface);
//              sysprintf("ifconfig %s up", iface);
		char nvlist[32];

		sprintf(nvlist, "%s_maclist", iface);

		foreach(var, nvram_safe_get(nvlist), next) {
			sysprintf("iwpriv %s addmac %s", iface, var);
		}
	} else if (nvram_match(nvvar, "allow")) {
		sysprintf("iwpriv %s maccmd 1", iface);
//              sysprintf("ifconfig %s up", iface);

		char nvlist[32];

		sprintf(nvlist, "%s_maclist", iface);

		foreach(var, nvram_safe_get(nvlist), next) {
			sysprintf("iwpriv %s addmac %s", iface, var);
		}
	} else {
		//undefined condition
//              sysprintf("ifconfig %s up", iface);
	}

}

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

void configure_single_11n(int count)
{
	char *next;
	static char var[80];
	static char mode[80];
	int cnt = 0;
	static char dev[10];
	static char wif[10];
	static char mtikie[32];
	static char wl[16];
	static char channel[16];
	static char ssid[16];
	static char net[16];
	static char wifivifs[16];
	static char broadcast[16];
	static char power[32];
	static char sens[32];
	static char basedev[16];
	static char diversity[32];
	static char athmac[16];
	static char maxassoc[32];
	static char wl_poll[32];
	static int vapcount = 0;
	if (count == 0)
		vapcount = 0;
	sysprintf("echo 1 >/proc/sys/dev/wifi%d/ledon", count);	//switch off led before configuring to prevent solid led

	sprintf(wif, "wifi%d", count);
	sprintf(dev, "ath%d", count);
	sprintf(wifivifs, "ath%d_vifs", count);
	sprintf(wl, "ath%d_mode", count);
#ifdef HAVE_REGISTER
	int cpeonly = iscpe();
#else
	int cpeonly = 0;
#endif
	if (cpeonly && nvram_match(wl, "ap")) {
		nvram_set(wl, "sta");
	}
	sprintf(channel, "ath%d_channel", count);
	sprintf(power, "ath%d_txpwrdbm", count);
	sprintf(sens, "ath%d_distance", count);
	sprintf(diversity, "ath%d_diversity", count);
	sprintf(athmac, "ath%d_hwaddr", count);

	// create base device
	cprintf("configure base interface %d\n", count);
	sprintf(net, "%s_net_mode", dev);
	if (nvram_match(net, "disabled"))
		return;
//    set_compression( count );
	// create wds interface(s)
	int s;

	char *apm;
	int vif = 0;
	sprintf(wl_poll, "%s_pollingmode", dev);

	setsysctrl(wif, "pollingmode", atoi(nvram_default_get(wl_poll, "0")));

	char *vifs = nvram_safe_get(wifivifs);
	int countvaps = 1;
	foreach(var, vifs, next) {
		countvaps++;
	}
	if (countvaps < 4)
		countvaps = 4;
	if (countvaps > vapcount)
		vapcount = countvaps;

	setsysctrl(wif, "maxvaps", vapcount);

	char primary[32] = { 0 };
	apm = nvram_default_get(wl, "ap");

	if (!strcmp(apm, "ap") || !strcmp(apm, "wdsap")) {

		sysprintf("80211n_wlanconfig %s create wlandev %s wlanmode ap", dev, wif);

		strcpy(primary, dev);
	}

	if (vifs != NULL)
		foreach(var, vifs, next) {
		sprintf(mode, "%s_mode", var);
		char *vapm = nvram_default_get(mode, "ap");
		// create device
		if (strlen(mode) > 0) {
			if (!strcmp(vapm, "wet") || !strcmp(vapm, "sta")
			    || !strcmp(vapm, "wdssta"))
				sysprintf("80211n_wlanconfig %s create wlandev %s wlanmode sta nosbeacon", var, wif);
			else if (!strcmp(vapm, "ap") || !strcmp(vapm, "wdsap"))
				sysprintf("80211n_wlanconfig %s create wlandev %s wlanmode ap", var, wif);
			else
				sysprintf("80211n_wlanconfig %s create wlandev %s wlanmode adhoc nosbeacon", var, wif);
			vif = 1;
			if (strlen(primary) == 0)
				strcpy(primary, var);
			char vathmac[16];

			sprintf(vathmac, "%s_hwaddr", var);
			char vmacaddr[32];

			getMacAddr(var, vmacaddr);
			nvram_set(vathmac, vmacaddr);

		}
		}
	// create original primary interface
	apm = nvram_default_get(wl, "ap");

	if (strcmp(apm, "ap") && strcmp(apm, "wdsap")) {

		if (!strcmp(apm, "wet") || !strcmp(apm, "wdssta")
		    || !strcmp(apm, "sta")) {
			if (vif)
				sysprintf("80211n_wlanconfig %s create wlandev %s wlanmode sta nosbeacon", dev, wif);
			else
				sysprintf("80211n_wlanconfig %s create wlandev %s wlanmode sta", dev, wif);

		} else if (!strcmp(apm, "ap") || !strcmp(apm, "wdsap"))
			sysprintf("80211n_wlanconfig %s create wlandev %s wlanmode ap", dev, wif);
		else
			sysprintf("80211n_wlanconfig %s create wlandev %s wlanmode adhoc nosbeacon", dev, wif);

		if (strlen(primary) == 0)
			strcpy(primary, dev);
	}
#if 0
#endif
	cprintf("detect maxpower\n");
	apm = nvram_default_get(wl, "ap");
	char maxp[16];

	vifs = nvram_safe_get(wifivifs);
	// fprintf(stderr,"vifs %s\n",vifs);
	char *useif = NULL;
	char copyvap[64];

	if (vifs != NULL)
		foreach(var, vifs, next) {
		// fprintf(stderr,"vifs %s, %s\n",vifs, var);
		if (!useif) {
			strcpy(copyvap, var);
			useif = copyvap;
		}
		}
	// config net mode
	if (useif)
		set_netmode(wif, dev, useif);
	set_netmode(wif, dev, dev);

	sysprintf("iwpriv %s wmm 1", dev);
	char doth[32];

	sprintf(doth, "%s_doth", dev);
#ifdef HAVE_BUFFALO
	sysprintf("iwpriv %s doth %s", dev, nvram_default_get(doth, "1"));
#else
	sysprintf("iwpriv %s doth %s", dev, nvram_default_get(doth, "0"));
#endif
	int disablescan = 0;

	set_scanlist(dev, wif);

	// don't call it twice
	// if (useif)
	//      set_netmode(wif, dev, useif);
	// set_netmode(wif, dev, dev);
	setRTS(dev);

	char macaddr[32];

	getMacAddr(dev, macaddr);
	nvram_set(athmac, macaddr);

	cprintf("adjust sensitivity\n");

	int distance = atoi(nvram_default_get(sens, "2000"));	// to meter
	if (nvram_nmatch("1", "%s_pollingmode", var)) {
		setdistance(wif, 100000, 20);
	} else {

		if (distance > 0) {
			setsysctrl(wif, "dynack_count", 0);
			char *chanbw = nvram_nget("%s_channelbw", dev);

			setdistance(wif, distance, atoi(chanbw));	// sets the receiver
			// sensitivity
		} else {
			setsysctrl(wif, "distance", 100000);
			setsysctrl(wif, "dynack_count", 20);
		}
	}

#if 0
	char wl_intmit[32];
	char wl_noise_immunity[32];
	char wl_ofdm_weak_det[32];
	char wl_csma[32];

	sprintf(wl_intmit, "%s_intmit", dev);
	sprintf(wl_noise_immunity, "%s_noise_immunity", dev);
	sprintf(wl_ofdm_weak_det, "%s_ofdm_weak_det", dev);
	sprintf(wl_csma, "%s_csma", dev);

	setsysctrl(wif, "csma", atoi(nvram_default_get(wl_csma, "1")));
	setsysctrl(wif, "intmit", atoi(nvram_default_get(wl_intmit, "-1")));
	int level = atoi(nvram_default_get(wl_noise_immunity, "4"));
	if (level < 0)
		level = 4;
	setsysctrl(wif, "noise_immunity", level);
	setsysctrl(wif, "ofdm_weak_det", atoi(nvram_default_get(wl_ofdm_weak_det, "1")));
#endif

	if (isEMP(dev))		//check this only if the current installed card is usually a emp card. this is made to prevent card destruction
	{
		if (nvram_nmatch("1", "%s_cardtype", dev)) {
			setsysctrl(wif, "powerfix", 7);	//increase outputpower by 7 dbm, we will do this in future for a and b band separate
		}

	}
	// setup vif interfaces first
	char chanshift_s[32];

	sprintf(chanshift_s, "%s_chanshift", dev);
	char *chanshift = nvram_default_get(chanshift_s, "0");

	sprintf(maxassoc, "%s_maxassoc", dev);
	sysprintf("iwpriv %s maxassoc %s", dev, nvram_default_get(maxassoc, "256"));

	switch (atoi(chanshift)) {
	case 15:
		sysprintf("iwpriv %s channelshift -3", dev);
		break;
	case 10:
		sysprintf("iwpriv %s channelshift -2", dev);
		break;
	case 5:
		sysprintf("iwpriv %s channelshift -1", dev);
		break;
	case 0:
		sysprintf("iwpriv %s channelshift 0", dev);
		break;
	case -5:
		sysprintf("iwpriv %s channelshift 1", dev);
		break;
	case -10:
		sysprintf("iwpriv %s channelshift 2", dev);
		break;
	case -15:
		sysprintf("iwpriv %s channelshift 3", dev);
		break;
	default:
		sysprintf("iwpriv %s channelshift 0", dev);
		break;
	}

	if (!strcmp(apm, "wdssta") || !strcmp(apm, "wdsap"))
		sysprintf("iwpriv %s wds 1", dev);

	if (!strcmp(apm, "wdsap"))
		sysprintf("iwpriv %s wdssep 1", dev);
	else
		sysprintf("iwpriv %s wdssep 0", dev);

	vifs = nvram_safe_get(wifivifs);
	if (vifs != NULL)
		foreach(var, vifs, next) {
		sprintf(net, "%s_net_mode", var);
		if (nvram_match(net, "disabled"))
			continue;
		sprintf(ssid, "%s_ssid", var);
		sprintf(mode, "%s_mode", var);
		sprintf(maxassoc, "%s_maxassoc", var);
		sysprintf("iwpriv %s maxassoc %s", var, nvram_default_get(maxassoc, "256"));
		switch (atoi(chanshift)) {
		case 15:
			sysprintf("iwpriv %s channelshift -3", var);
			break;
		case 10:
			sysprintf("iwpriv %s channelshift -2", var);
			break;
		case 5:
			sysprintf("iwpriv %s channelshift -1", var);
			break;
		case 0:
			sysprintf("iwpriv %s channelshift 0", var);
			break;
		case -5:
			sysprintf("iwpriv %s channelshift 1", var);
			break;
		case -10:
			sysprintf("iwpriv %s channelshift 2", var);
			break;
		case -15:
			sysprintf("iwpriv %s channelshift 3", var);
			break;
		default:
			sysprintf("iwpriv %s channelshift 0", var);
			break;
		}
		char *mvap = nvram_default_get(mode, "ap");
		set_scanlist(dev, wif);
		setRTS(var);

#if 0
		if (strcmp(mvap, "sta") && strcmp(mvap, "wdssta")
		    && strcmp(mvap, "wet")) {
			cprintf("set channel\n");
			char *ch = nvram_default_get(channel, "0");

			if (strcmp(ch, "0") == 0) {
				sysprintf("iwconfig %s channel 0", var);
			} else {
				sysprintf("iwconfig %s freq %sM", var, ch);
			}
		}
#endif
		sysprintf("iwpriv %s bgscan 0", var);
#ifdef HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
		sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "default_vap"));
#else
		sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "maksat_vap"));
#endif
#elif defined(HAVE_SANSFIL)
                sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "SANSFIL_vap"));
#elif defined(HAVE_TRIMAX)
		sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "trimax_vap"));
#elif defined(HAVE_WIKINGS)
		sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "Excel Networks_vap"));
#elif defined(HAVE_ESPOD)
		sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "ESPOD Technologies_vap"));
#elif defined(HAVE_NEXTMEDIA)
		sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "nextmedia_vap"));
#elif defined(HAVE_TMK)
		sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "KMT_vap"));
#elif defined(HAVE_CORENET)
		sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "corenet.ap"));
#elif defined(HAVE_ONNET_BLANK)
		sysprintf("iwconfig %s essid -- \"%s\"", var,
			  nvram_default_get(ssid, "Enterprise WIFI_vap"));
#elif defined(HAVE_ONNET)
		sysprintf("iwconfig %s essid -- \"%s\"", var,
			  nvram_default_get(ssid, "OTAi_vap"));
#elif defined(HAVE_KORENRON)
                sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "WBR2000_vap"));
#else
#ifdef HAVE_REGISTER
		if (!isregistered())
			sysprintf("iwconfig %s essid -- need_activation", var);
		else
#endif
			sysprintf("iwconfig %s essid -- \"%s\"", var, nvram_default_get(ssid, "dd-wrt_vap"));
#endif
		cprintf("set broadcast flag vif %s\n", var);	// hide ssid
		sprintf(broadcast, "%s_closed", var);
		sysprintf("iwpriv %s hide_ssid %s", var, nvram_default_get(broadcast, "0"));
		sysprintf("iwpriv %s wmm 1", var);
		char isolate[32];

		sprintf(isolate, "%s_ap_isolate", var);
		if (nvram_default_match(isolate, "1", "0"))
			sysprintf("iwpriv %s ap_bridge 0", var);
		if (!strcmp(mvap, "wdssta") || !strcmp(mvap, "wdsap"))
			sysprintf("iwpriv %s wds 1", var);
		sprintf(mtikie, "%s_mtikie", var);
		if (nvram_default_match(mtikie, "1", "0"))
			sysprintf("iwpriv %s addmtikie 1", var);

#ifdef HAVE_BONDING
		if (!strcmp(mvap, "wdsap") && !isBond(var))
#else
		if (!strcmp(mvap, "wdsap"))
#endif
			sysprintf("iwpriv %s wdssep 1", var);
		else
			sysprintf("iwpriv %s wdssep 0", var);

		// removed hostroaming 0 due to excessive tests and driver research
		// sysprintf("iwpriv %s hostroaming 0", var);
		cnt++;
		}

	sprintf(mtikie, "%s_mtikie", dev);
	if (nvram_default_match(mtikie, "1", "0"))
		sysprintf("iwpriv %s addmtikie 1", dev);

	char isolate[32];

	sprintf(isolate, "%s_ap_isolate", dev);
	if (nvram_default_match(isolate, "1", "0"))
		sysprintf("iwpriv %s ap_bridge 0", dev);
	// removed hostroaming 0 due to excessive tests and driver research
	// sysprintf("iwpriv %s hostroaming 0", dev);

	sprintf(ssid, "ath%d_ssid", count);
	sprintf(broadcast, "ath%d_closed", count);

	memset(var, 0, 80);

	cprintf("set ssid\n");
#ifdef HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
	sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "default"));
#else
	sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "maksat"));
#endif
#elif defined(HAVE_TRIMAX)
	sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "trimax"));
#elif defined(HAVE_WIKINGS)
	sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "Excel Networks"));
#elif defined(HAVE_ESPOD)
	sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "ESPOD Technologies"));
#elif defined(HAVE_NEXTMEDIA)
	sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "nextmedia"));
#elif defined(HAVE_TMK)
	sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "KMT"));
#elif defined(HAVE_CORENET)
	sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "corenet.ap"));
#elif defined(HAVE_KORENRON)
        sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "WBR2000"));
#else
#ifdef HAVE_REGISTER
	if (!isregistered())
		sysprintf("iwconfig %s essid -- need_activation", dev);
	else
#endif
		sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "dd-wrt"));
#endif
	cprintf("set broadcast flag\n");	// hide ssid
	sysprintf("iwpriv %s hide_ssid %s", dev, nvram_default_get(broadcast, "0"));
	sysprintf("iwpriv %s bgscan 0", dev);
	apm = nvram_default_get(wl, "ap");

	char preamble[32];

	if (nvram_match(wl, "g-only") || nvram_match(wl, "b-only")) {
		sprintf(preamble, "%s_preamble", dev);
		if (nvram_default_match(preamble, "1", "0")) {
			sysprintf("iwpriv %s shpreamble 1", dev);
		} else
			sysprintf("iwpriv %s shpreamble 0", dev);
	}

	if (strcmp(apm, "sta") == 0 || strcmp(apm, "infra") == 0 || strcmp(apm, "wet") == 0 || strcmp(apm, "wdssta") == 0) {
		cprintf("set ssid\n");
#ifdef HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
		sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "default"));
#else
		sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "maksat"));
#endif
#elif defined(HAVE_TRIMAX)
		sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "trimax"));
#elif defined(HAVE_WIKINGS)
		sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "Excel Networks"));
#elif defined(HAVE_ESPOD)
		sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "ESPOD Technologies"));
#elif defined(HAVE_NEXTMEDIA)
		sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "nextmedia"));
#elif defined(HAVE_TMK)
		sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "KMT"));
#elif defined(HAVE_CORENET)
		sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "corenet.ap"));
#elif defined(HAVE_KORENRON)
                sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "WBR2000"));
#else
		sysprintf("iwconfig %s essid -- \"%s\"", dev, nvram_default_get(ssid, "dd-wrt"));
#endif
	}

	cprintf("adjust power\n");

	int newpower = atoi(nvram_default_get(power, "16"));

	sysprintf("iwconfig %s txpower %ddBm", dev, newpower);

	cprintf("done()\n");

	cprintf("setup encryption");
	// @todo ifup
	// netconfig

	/*
	 * set_rate (dev);
	 */
	set_rate(dev, dev);
	// don't call it triple
	// set_netmode(wif, dev, dev);

	setMacFilter(dev);
	setupKey(dev);
	if (vifs != NULL && strlen(vifs) > 0) {
		foreach(var, vifs, next) {
			setMacFilter(var);
			setupKey(var);
		}
	}
#if 0

	apm = nvram_default_get(wl, "ap");
	if (strcmp(apm, "sta") && strcmp(apm, "wdssta") && strcmp(apm, "wet")) {
		cprintf("set channel\n");
		char *ch = nvram_default_get(channel, "0");

		if (strcmp(ch, "0") == 0) {
			sysprintf("iwconfig %s channel 0", dev);
		} else {
			sysprintf("iwconfig %s freq %sM", dev, ch);
		}
	}
#endif

	// setup encryption
	if (strcmp(apm, "sta") && strcmp(apm, "wdssta") && strcmp(apm, "wet"))
		setupHostAP(dev, "madwifi", 0);
	else
		setupSupplicant(dev, NULL);
	vifs = nvram_safe_get(wifivifs);
	if (vifs != NULL)
		foreach(var, vifs, next) {
		sprintf(mode, "%s_mode", var);
		char *vapm = nvram_default_get(mode, "ap");
		if (strcmp(vapm, "sta") && strcmp(vapm, "wdssta")
		    && strcmp(vapm, "wet"))
			setupHostAP(var, "madwifi", 0);
		else
			setupSupplicant(var, NULL);
		}
#ifdef HAVE_RELAYD
	if (strcmp(apm, "sta") && strcmp(apm, "wet")) {
#else
	if (strcmp(apm, "sta")) {
#endif
		char bridged[32];

		sprintf(bridged, "%s_bridged", dev);
		if (nvram_default_match(bridged, "1", "1")) {
			sysprintf("ifconfig %s 0.0.0.0 up", dev);
			br_add_interface(getBridge(dev), dev);
			sysprintf("ifconfig %s 0.0.0.0 up", dev);
		} else {
			sysprintf("ifconfig %s mtu %s", dev, getMTU(dev));
			sysprintf("ifconfig %s txqueuelen %s", dev, getTXQ(dev));
			sysprintf("ifconfig %s %s netmask %s up", dev, nvram_nget("%s_ipaddr", dev), nvram_nget("%s_netmask", dev));
		}
	} else {
#ifdef HAVE_RELAYD
		if (!strcmp(apm, "wet")) {
			sysprintf("ifconfig %s 0.0.0.0 up", dev);
//                      sysprintf("relayd -I %s -I %s -D -B", getBridge(dev),
//                                dev);
		}
#endif

		char bridged[32];
		sprintf(bridged, "%s_bridged", dev);
		if (nvram_default_match(bridged, "0", "1")) {
			sysprintf("ifconfig %s mtu %s", dev, getMTU(dev));
			sysprintf("ifconfig %s txqueuelen %s", dev, getTXQ(dev));
			sysprintf("ifconfig %s %s netmask %s up", dev, nvram_nget("%s_ipaddr", dev), nvram_nget("%s_netmask", dev));
		}

	}

	// vif netconfig
	vifs = nvram_safe_get(wifivifs);
	if (vifs != NULL && strlen(vifs) > 0) {
		foreach(var, vifs, next) {
			sprintf(mode, "%s_mode", var);
			char *m2 = nvram_default_get(mode, "ap");

			if (strcmp(m2, "sta")) {
				char bridged[32];

				sprintf(bridged, "%s_bridged", var);
				if (nvram_default_match(bridged, "1", "1")) {
					sysprintf("ifconfig %s 0.0.0.0 up", var);
					br_add_interface(getBridge(var), var);
				} else {
					char ip[32];
					char mask[32];

					sprintf(ip, "%s_ipaddr", var);
					sprintf(mask, "%s_netmask", var);
					sysprintf("ifconfig %s mtu %s", var, getMTU(var));
					sysprintf("ifconfig %s txqueuelen %s", var, getTXQ(var));
					sysprintf("ifconfig %s %s netmask %s up", var, nvram_safe_get(ip), nvram_safe_get(mask));
				}
			}
		}
	}
	if (!strcmp(apm, "ap") || !strcmp(apm, "wdsap")) {

		int hasnawds = 0;

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
			if (strlen(wdsdev) == 0)
				continue;
			if (nvram_match(wdsvarname, "0"))
				continue;
			hwaddr = nvram_get(wdsmacname);
			if (hwaddr != NULL) {
				if (!hasnawds)
					sleep(10);
				hasnawds = 1;
				sysprintf("80211n_wlanconfig %s nawdslist set 1 %s", primary, hwaddr);
			}
		}
		if (hasnawds) {
			sysprintf("iwpriv ath0 wds 1");
			sysprintf("iwpriv ath0 nawds 1");
		}
	}

/*	for (s = 1; s <= 10; s++) {
		char wdsvarname[32] = { 0 };
		char wdsdevname[32] = { 0 };
		char wdsmacname[32] = { 0 };
		char *wdsdev;
		char *hwaddr;

		sprintf(wdsvarname, "%s_wds%d_enable", dev, (11 - s));
		sprintf(wdsdevname, "%s_wds%d_if", dev, (11 - s));
		sprintf(wdsmacname, "%s_wds%d_hwaddr", dev, (11 - s));
		wdsdev = nvram_safe_get(wdsdevname);
		if (strlen(wdsdev) == 0)
			continue;
		if (nvram_match(wdsvarname, "0"))
			continue;
		hwaddr = nvram_get(wdsmacname);
		if (hwaddr != NULL) {
			sysprintf("ifconfig %s 0.0.0.0 up", wdsdev);
		}
	}*/
}

extern void adjust_regulatory(int count);

#endif
