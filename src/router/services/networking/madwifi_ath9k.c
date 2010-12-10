/*
 * madwifi_ath9k.c 
 * Copyright (C) 2010 Christian Scheele <chris@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifdef HAVE_ATH9K
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

#include <services.h>
void deconfigure_single_ath9k(int count) {
	fprintf (stderr, "ath9k deconfigure_single: phy%d ath%d\n",get_ath9k_phy_idx(count), count);
	static char wif[10];
	sprintf(wif, "phy%d", get_ath9k_phy_idx(count));
	delete_ath9k_devices(wif);
}

void configure_single_ath9k(int count) {
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
	static char rxantenna[32];
	static char txantenna[32];
	static int vapcount = 0;
	char *apm;
	char isolate[32];
	char primary[32] = { 0 };
	if (count == 0)
		vapcount = 0;
	sprintf(wif, "phy%d", get_ath9k_phy_idx(count));
	sprintf(dev, "ath%d", count);
	sprintf(wifivifs, "ath%d_vifs", count);
	fprintf (stderr, "ath9k configure_single: phy%d ath%d\n",get_ath9k_phy_idx(count), count);
	sprintf(channel, "ath%d_channel", count);
	sprintf(power, "ath%d_txpwrdbm", count);
	sprintf(sens, "ath%d_distance", count);
	sprintf(diversity, "ath%d_diversity", count);
	sprintf(athmac, "ath%d_hwaddr", count);
	sprintf(rxantenna, "ath%d_rxantenna", count);
	sprintf(txantenna, "ath%d_txantenna", count);
	// create base device
	cprintf("configure base interface %d\n", count);
	sprintf(net, "%s_net_mode", dev);
	char *netmode = nvram_default_get(net, "mixed");
	if (!strcmp(netmode, "disabled"))
		return;

#ifdef HAVE_REGISTER
	int cpeonly = iscpe();
#else
	int cpeonly = 0;
#endif
#if defined(HAVE_WHRHPGN) || defined(HAVE_DIR615E)
	nvram_default_get(rxantenna, "3");
	nvram_default_get(txantenna, "3");
#else
	nvram_default_get(rxantenna, "7");
	nvram_default_get(txantenna, "5");
#endif
	sysprintf("echo %d > /sys/kernel/debug/ath9k/%s/rx_chainmask",nvram_safe_get(rxantenna),wif);
	sysprintf("echo %d > /sys/kernel/debug/ath9k/%s/tx_chainmask",nvram_safe_get(txantenna),wif);
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

	sprintf(wl, "ath%d_mode", count);
	apm = nvram_default_get(wl, "ap");

	if (!strcmp(apm, "ap") || !strcmp(apm, "wdsap") || !strcmp(apm, "sta") || !strcmp(apm, "wet")) {

		sysprintf
		    ("iw %s interface add %s type managed",
		     wif, dev);

		strcpy(primary, dev);
	}
	else if (!strcmp(apm, "wdssta") ) {
		sysprintf
		    ("iw %s interface add %s type managed 4addr on",
		     wif, dev);

		strcpy(primary, dev);
	}
	else {
		// infra (adhoc) TBD
	}

	char regdomain[16];
	char *country;
	sprintf(regdomain, "%s_regdomain", dev);
	country=nvram_default_get(regdomain, "US");
	sysprintf ("iw reg set %s", getIsoName(country));

	// vifs TBD

	char macaddr[32];
	// interface is created at this point, so that should work
	getMacAddr(dev, macaddr);
	nvram_set(athmac, macaddr);
	int distance = atoi(nvram_default_get(sens, "2000"));	// to meter
	sysprintf
	    ("iw %s set distance %d", wif, distance);

	sprintf(maxassoc, "%s_maxassoc", dev);
	sysprintf("echo TBD maxassoc: %s maxassoc %s", dev,
		  nvram_default_get(maxassoc, "256"));
// das scheint noch aerger zu machen
	sysprintf ("iw dev %s set power_save off", dev);

	cprintf("done()\n");

	cprintf("setup encryption");
	// setup encryption
	if (strcmp(apm, "sta") && strcmp(apm, "wdssta") && strcmp(apm, "wet"))
		setupHostAP(dev, "nl80211", 0);
	else
		setupSupplicant(dev, NULL);
	sleep(3);		//give some time to let hostapd initialize
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
			sysprintf("ifconfig %s %s netmask %s up", dev,
				  nvram_nget("%s_ipaddr", dev),
				  nvram_nget("%s_netmask", dev));
		}
	} else {
#ifdef HAVE_RELAYD
		if (!strcmp(apm, "wet")) {
			sysprintf("ifconfig %s 0.0.0.0 up", dev);
//			sysprintf("relayd -I %s -I %s -D -B", getBridge(dev),
//				  dev);
		}
#endif

		char bridged[32];
		sprintf(bridged, "%s_bridged", dev);
		if (nvram_default_match(bridged, "0", "1")) {
			sysprintf("ifconfig %s mtu %s", dev, getMTU(dev));
			sysprintf("ifconfig %s %s netmask %s up", dev,
				  nvram_nget("%s_ipaddr", dev),
				  nvram_nget("%s_netmask", dev));
		}

	}
}
#endif
