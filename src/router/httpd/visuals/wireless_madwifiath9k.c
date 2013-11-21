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
	int bias, qual, it;
	int co = 0;
	sprintf(nb, "%s_bias", ifname);
	bias = atoi(nvram_default_get(nb, "0"));

	// sprintf(it, "inactivity_time", ifname);
//      it = atoi(nvram_default_get("inacttime", "300000"));

	mac80211_info = mac80211_assoclist(ifname);
	for (wc = mac80211_info->wci; wc; wc = wc->next) {
		strncpy(mac, wc->mac, 31);
		if (nvram_match("maskmac", "1") && macmask) {
			mac[0] = 'x';
			mac[1] = 'x';
			mac[3] = 'x';
			mac[4] = 'x';
			mac[6] = 'x';
			mac[7] = 'x';
			mac[9] = 'x';
			mac[10] = 'x';
		}
		qual = wc->signal * 124 + 11600;
		qual /= 10;
//              if (wc->inactive_time < it) {
		if (cnt)
			websWrite(wp, ",");
		websWrite(wp, "'%s','%s','%s','%dM','%dM','%d','%d','%d','%d'", mac, wc->ifname, UPTIME(wc->uptime), wc->txrate / 10, wc->rxrate / 10, wc->signal + bias, wc->noise + bias, wc->signal - wc->noise, qual);
		cnt++;
//              }
	}
	free_wifi_clients(mac80211_info->wci);
	free(mac80211_info);
	return cnt;
}
#endif
