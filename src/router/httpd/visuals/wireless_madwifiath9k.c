/*
 * wireless_madwifiath9k.c 
 *
 * Copyright (C) 2010 Christian Scheele <chris@dd-wrt.com>
 * Copyright (C) 2005 - 2013 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_ATH9K
#define VISUALSOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
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

#include <broadcom.h>
#include <bcmparams.h>
#include <utils.h>
#include <wlutils.h>
#include "wireless_generic.c"
int ej_active_wireless_if_ath9k(webs_t wp, int argc, char_t ** argv, char *ifname, int cnt, int turbo, int macmask)
{
	char mac[32];
	struct mac80211_info *mac80211_info;
	struct wifi_client_info *wc;
	char nb[32];
	int bias, qual, it, div = 1, mul = 1;
	int co = 0;
	sprintf(nb, "%s_bias", ifname);
	bias = nvram_default_geti(nb, 0);

	if (is_ath5k(ifname)) {
		sprintf(nb, "%s_channelbw", ifname);
		int channelbw = atoi(nvram_default_get(nb, "0"));
		if (channelbw == 40)
			mul = 2;
		if (channelbw == 10)
			div = 2;
		if (channelbw == 5)
			div = 4;
	}
	// sprintf(it, "inactivity_time", ifname);
//      it = nvram_default_geti("inacttime", 300000);

	mac80211_info = mac80211_assoclist(ifname);
	for (wc = mac80211_info->wci; wc; wc = wc->next) {
		strncpy(mac, wc->mac, 31);
		if (nvram_matchi("maskmac", 1) && macmask) {
			mac[0] = 'x';
			mac[1] = 'x';
			mac[3] = 'x';
			mac[4] = 'x';
			mac[6] = 'x';
			mac[7] = 'x';
			mac[9] = 'x';
			mac[10] = 'x';
		}
		int signal = wc->signal;
		if (signal >= -50)
			qual = 1000;
		else if (signal <= -100)
			qual = 0;
		else
			qual = (signal + 100) * 20;

		if (cnt)
			websWrite(wp, ",");

		int ht = 0;
		int sgi = 0;
		int vht = 0;
		char info[32];

		if (!wc->rx_is_ht)
			ht = 5;
		if (wc->rx_is_40mhz || wc->is_40mhz)
			ht = 1;
		if (wc->rx_is_80mhz || wc->is_80mhz)
			ht = 2;
		if (wc->rx_is_160mhz || wc->is_160mhz)
			ht = 3;
		if (wc->rx_is_80p80mhz || wc->is_80p80mhz)
			ht = 4;
		if (wc->rx_is_vht || wc->is_vht)
			vht = 1;
		if (wc->rx_is_short_gi || wc->is_short_gi)
			sgi = 1;

		if (ht == 5 && sgi)
			ht = 0;
		if (ht == 5 && vht)
			ht = 0;
		if (ht == 5)
			strcpy(info, "LEGACY");
		else
			strcpy(info, vht ? "VHT" : "HT");
		char *bwinfo[] = { "20", "40", "80", "160", "80+80" };
		if (ht < 5 && ht >= 0)
			sprintf(info, "%s%s", info, bwinfo[ht]);
		if (wc->ht40intol)
			sprintf(info, "%si", info);
		if (sgi)
			sprintf(info, "%s%s", info, "SGI");
		if (wc->islzo)
			sprintf(info, "%s %s", info, "LZ");
		char str[64] = { 0 };

		websWrite(wp, "'%s','%s','%s','%dM','%dM','%s','%d','%d','%d','%d'", mac, wc->ifname, UPTIME(wc->uptime, str), wc->txrate / 10 * mul / div, wc->rxrate / 10 * mul / div, info, wc->signal + bias,
			  wc->noise + bias, wc->signal - wc->noise, qual);
		cnt++;
//              }
	}
	free_wifi_clients(mac80211_info->wci);
	free(mac80211_info);
	return cnt;
}
#endif
