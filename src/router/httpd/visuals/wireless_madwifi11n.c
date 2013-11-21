/*
 * wireless_madwifi11n.c
 *
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
#ifdef HAVE_MADWIFI_MIMO
#define VISUALSOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>
#include <cymac.h>
#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>

#include "wireless_generic.c"

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

#include "wireless_copy.h"
#include "../../madwifi.dev/madwifi_mimo.dev/core/net80211/ieee80211.h"
#include "../../madwifi.dev/madwifi_mimo.dev/core/net80211/ieee80211_crypto.h"
#include "../../madwifi.dev/madwifi_mimo.dev/core/net80211/ieee80211_ioctl.h"
static const char *ieee80211_ntoa(const uint8_t mac[IEEE80211_ADDR_LEN])
{
	static char a[32];
	int i;

	i = snprintf(a, sizeof(a), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return (i < 17 ? NULL : a);
}

extern unsigned char madbuf[24 * 1024];
int ej_active_wireless_if_11n(webs_t wp, int argc, char_t ** argv, char *ifname, int cnt, int turbo, int macmask)
{

	unsigned char *cp;
	int s, len;
	struct iwreq iwr;
	char nb[32];
	sprintf(nb, "%s_bias", ifname);
	int bias = atoi(nvram_default_get(nb, "0"));
	if (!ifexists(ifname)) {
		printf("IOCTL_STA_INFO ifresolv %s failed!\n", ifname);
		return cnt;
	}
	int state = 0;
	state = get_radiostate(ifname);
	if (state == 0 || state == -1) {
		printf("IOCTL_STA_INFO radio %s not enabled!\n", ifname);
		return cnt;
	}
	s = getsocket();
	if (s < 0) {
		fprintf(stderr, "socket(SOCK_DRAGM)\n");
		return cnt;
	}
	(void)memset(&iwr, 0, sizeof(struct iwreq));
	(void)strncpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));

	iwr.u.data.pointer = (void *)&madbuf[0];
	iwr.u.data.length = 24 * 1024;
	if (ioctl(s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0) {
		fprintf(stderr, "IOCTL_STA_INFO for %s failed!\n", ifname);
		closesocket();
		return cnt;
	}
	len = iwr.u.data.length;
	if (len < sizeof(struct ieee80211req_sta_info)) {
		// fprintf(stderr,"IOCTL_STA_INFO len<struct %s failed!\n",ifname);
		closesocket();
		return cnt;
	}
	cp = madbuf;
	int bufcount = 0;
	do {
		struct ieee80211req_sta_info *si;
		uint8_t *vp;

		si = (struct ieee80211req_sta_info *)cp;
		vp = (u_int8_t *)(si + 1);

		if (cnt)
			websWrite(wp, ",");
		cnt++;
		char mac[32];

		strncpy(mac, ieee80211_ntoa(si->isi_macaddr), 31);
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
		if (si->isi_noise == 0) {
			si->isi_noise = -95;
		}
		int qual = (si->isi_noise + si->isi_rssi) * 124 + 11600;
		qual /= 10;
		int rxrate = si->isi_rxrateKbps / 1000;
		int txrate = si->isi_txrateKbps / 1000;
		if (!rxrate)
			rxrate = si->isi_rates[si->isi_rxrate] & IEEE80211_RATE_VAL;
		if (!txrate)
			txrate = si->isi_rates[si->isi_txrate] & IEEE80211_RATE_VAL;

		char rx[32];
		char tx[32];
		if (rxrate)
			sprintf(rx, "%3dM", rxrate);
		else
			sprintf(rx, "N/A");
		if (txrate)
			sprintf(tx, "%3dM", txrate);
		else
			sprintf(tx, "N/A");
		websWrite(wp, "'%s','%s','%s','%s','%s','%d','%d','%d','%d'", mac, ifname, UPTIME(si->isi_uptime), tx, rx, si->isi_noise + si->isi_rssi + bias, si->isi_noise + bias, si->isi_rssi, qual);
		bufcount += si->isi_len;
		cp += si->isi_len;
		len -= si->isi_len;
	}
	while (len >= sizeof(struct ieee80211req_sta_info)
	       && bufcount < (sizeof(madbuf) - sizeof(struct ieee80211req_sta_info)));

	closesocket();

	return cnt;
}

#endif
