/*
 * signal_watchdog.c
 *
 * Copyright (C) 2006 - 2026 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils.h>
#include <wlutils.h>
#include <errno.h>
#include <ddnvram.h>
#include <shutils.h>

int isregistered_real(void);
int isregistered(void);

/* check signal code, its unused now, we keep it if we need it later again */
static unsigned char zerocount[8][17];
static void check_signal(const char *var, int interface, int vap)
{
	struct mac80211_info *mac80211_info;
	int clientcount = 0;
	mac80211_info = mac80211_assoclist(var);
	if (mac80211_info && mac80211_info->wci) {
		struct wifi_client_info *wc;
		for (wc = mac80211_info->wci; wc; wc = wc->next) {
			if (wc) {
				clientcount++;
				char mac[32];
				ether_etoa(wc->etheraddr, mac);
				if (!(wc->signal - wc->noise)) {
					zerocount[interface][vap]++;
					if (zerocount[interface][vap] > 20)
						dd_logerror("ath11k_watchdog", "zero signal issue detected on interface %s (%s)\n",
							    wc->ifname, mac);
					if (zerocount[interface][vap] == 100) {
						dd_logerror("ath11k_watchdog", "20 consecutive signal fails detected on %s (%s)\n",
							    wc->ifname, mac);
						sys_reboot();
					}
				
				} else {
					if (zerocount[interface][vap]) {
						if (zerocount[interface][vap] > 20)
							dd_logerror("ath11k_watchdog",
								    "signal measurement received. reset failcount %s (%s)\n",
								    wc->ifname, mac);
						int i;
						for (i = 0; i < 17; i++)
							zerocount[interface][i] = 0;
					}
				}
			}
		}
		if (!clientcount)
			zerocount[interface][vap] = 0;
		free_wifi_clients(mac80211_info->wci);
	}
	if (mac80211_info)
		free(mac80211_info);
}
static void check_wifi(void)
{
	int ifcount = getdevicecount();
	int c = 0;
	int vap = 0;
	for (c = 0; c < ifcount; c++) {
		char interface[32];
		sprintf(interface, "wlan%d", c);
		if (nvram_nmatch("disabled", "%s_net_mode", interface))
			continue;
		if (nvram_nmatch("disabled", "%s_mode", interface))
			continue;

		if (is_ath11k(interface)) {
			check_signal(interface, c, 0);
			char vifs[32];
			char var[32];
			const char *next;
			sprintf(vifs, "wlan%d_vifs", c);
			char *vaps = nvram_safe_get(vifs);
			int vap = 1;
			foreach(var, vaps, next) {
				if (nvram_nmatch("disabled", "%s_net_mode", var))
					continue;
				if (nvram_nmatch("disabled", "%s_mode", var))
					continue;
				check_signal(var, c, vap++);
			}
		}
	}
}

static void watchdog(void)
{
	while (1) {
		check_wifi();
		sleep(10);
	}
}

int main(int argc, char *argv[])
{
	memset(zerocount, 0, sizeof(zerocount));
	dd_daemon();
	watchdog();
	return 0;
}

