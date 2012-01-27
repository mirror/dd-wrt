#include <stdio.h>
#include <malloc.h>
#include <wlutils.h>
#include <shutils.h>
#include <bcmnvram.h>

void showinterface(char *base, char *ifname)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		struct mac80211_info *mac80211_info;
		struct wifi_client_info *wc;
		mac80211_info = mac80211_assoclist(base);
		for (wc = mac80211_info->wci; wc; wc = wc->next) {
			if (!strcmp(ifname, wc->ifname))
				fprintf(stdout, "assoclist %s\n", wc->mac);
		}
		free_wifi_clients(mac80211_info->wci);
		free(mac80211_info);
	} else
#endif
	{

		char *buf = malloc(8192);
		memset(buf, 0, 8192);
		int cnt = getassoclist(ifname, buf);
		int count;
		memcpy(&count, buf, 4);
		unsigned char *p = &buf[4];
		int a;
		int pos = 0;
		for (a = 0; a < cnt; a++) {
			fprintf(stdout,
				"assoclist %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",
				p[pos], p[pos + 1], p[pos + 2], p[pos + 3],
				p[pos + 4], p[pos + 5]);
			pos += 6;
		}
		free(buf);

	}

}

int matchmac(char *base, char *ifname, char *mac)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		struct mac80211_info *mac80211_info;
		struct wifi_client_info *wc;
		mac80211_info = mac80211_assoclist(base);
		for (wc = mac80211_info->wci; wc; wc = wc->next) {
			if (!strcmp(ifname, wc->ifname)
			    && !strcmp(mac, wc->mac)) {
				free_wifi_clients(mac80211_info->wci);
				free(mac80211_info);
				return 1;
			}
		}
		return 0;
	} else
#endif
	{
		return 1;
	}

}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "invalid argument\n");
		return 0;
	}
	char *ifname = "ath0";
	int i;
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-i")) {
			ifname = argv[++i];
			continue;
		}
		if (!strcmp(argv[i], "assoclist")) {
			int ifcount = getdevicecount();
			if (strcmp(argv[1], "-i") == 0)
				showinterface(ifname, ifname);
			else {
				int c = 0;
				for (c = 0; c < ifcount; c++) {
					char interface[32];
					sprintf(interface, "ath%d", c);
					showinterface(interface,interface);
					char vif[32];
					sprintf(vif, "%s_vifs", interface);
					char var[80], *next;
					char *vifs = nvram_safe_get(vif);
					if (vifs != NULL) {
						foreach(var, vifs, next) {
							showinterface(interface,
								      var);
						}
					}
				}
			}

		}
		if (!strcmp(argv[i], "rssi")) {
			unsigned char rmac[6];
			ether_atoe(argv[++i], rmac);
			int ifcount = getdevicecount();
			int rssi = -1;
			if (strcmp(argv[1], "-i") == 0)
				rssi = getRssi(ifname, rmac);
			else {
				int c = 0;
				for (c = 0; c < ifcount; c++) {
					char interface[32];
					sprintf(interface, "ath%d", c);
					rssi = getRssi(interface, rmac);
					if (rssi != 0 && rssi != -1
					    && matchmac(interface, interface,
							rmac)) {
						fprintf(stdout, "rssi is %d\n",
							rssi);
						return 0;
					}
					char vif[32];
					sprintf(vif, "%s_vifs", interface);
					char var[80], *next;
					char *vifs = nvram_safe_get(vif);
					if (vifs != NULL) {
						foreach(var, vifs, next) {
							rssi =
							    getRssi(var, rmac);
							if (rssi != 0
							    && rssi != -1
							    &&
							    matchmac(interface,
								     var,
								     rmac)) {
								fprintf(stdout,
									"rssi is %d\n",
									rssi);
								return 0;
							}

						}
					}
				}
			}

			fprintf(stdout, "rssi is %d\n", rssi);
		}
		if (!strcmp(argv[i], "noise")) {
			unsigned char rmac[6];
			ether_atoe(argv[++i], rmac);
			int ifcount = getdevicecount();
			int rssi = -1;
			if (strcmp(argv[1], "-i") == 0)
				rssi = getNoise(ifname, rmac);
			else {
				int c = 0;
				for (c = 0; c < ifcount; c++) {
					char interface[32];
					sprintf(interface, "ath%d", c);

					rssi = getNoise(interface, rmac);
					if (rssi != 0 && rssi != -1
					    && matchmac(interface, interface,
							rmac)) {
						fprintf(stdout, "noise is %d\n",
							rssi);
						return 0;
					}
					char vif[32];
					sprintf(vif, "%s_vifs", interface);
					char var[80], *next;
					char *vifs = nvram_safe_get(vif);
					if (vifs != NULL) {
						foreach(var, vifs, next) {
							rssi =
							    getNoise(var, rmac);
							if (rssi != 0
							    && rssi != -1
							    &&
							    matchmac(interface,
								     var,
								     rmac)) {
								fprintf(stdout,
									"noise is %d\n",
									rssi);
								return 0;
							}
						}
					}
				}
			}
			fprintf(stdout, "noise is %d\n", rssi);
		}
		if (!strcmp(argv[i], "ifname")) {
			unsigned char rmac[6];
			ether_atoe(argv[++i], rmac);
			int ifcount = getdevicecount();
			int rssi = -1;
			int c = 0;
			for (c = 0; c < ifcount; c++) {
				char interface[32];
				sprintf(interface, "ath%d", c);

				rssi = getNoise(interface, rmac);
				if (rssi != 0 && rssi != -1
				    && matchmac(interface, interface, rmac)) {
					fprintf(stdout, "ifname is %s\n",
						interface);
					return 0;
				}
				char vif[32];
				sprintf(vif, "%s_vifs", interface);
				char var[80], *next;
				char *vifs = nvram_safe_get(vif);
				if (vifs != NULL) {
					foreach(var, vifs, next) {
						rssi = getNoise(var, rmac);
						if (rssi != 0 && rssi != -1
						    && matchmac(interface, var,
								rmac)) {
							fprintf(stdout,
								"ifname is %s\n",
								var);
							return 0;
						}
					}
				}
			}
			fprintf(stdout, "ifname is error\n");
		}
		if (!strcmp(argv[i], "uptime")) {
			unsigned char rmac[6];
			ether_atoe(argv[++i], rmac);
			int ifcount = getdevicecount();
			int uptime = -1;
			if (strcmp(argv[1], "-i") == 0)
				uptime = getUptime(ifname, rmac);
			else {
				int c = 0;
				for (c = 0; c < ifcount; c++) {
					char interface[32];
					sprintf(interface, "ath%d", c);

					uptime = getUptime(interface, rmac);
					if (uptime != 0 && uptime != -1
					    && matchmac(interface, interface,
							rmac)) {
						fprintf(stdout,
							"uptime is %d\n",
							uptime);
						return 0;
					}
					char vif[32];
					sprintf(vif, "%s_vifs", interface);
					char var[80], *next;
					char *vifs = nvram_safe_get(vif);
					if (vifs != NULL) {
						foreach(var, vifs, next) {
							uptime =
							    getUptime(var,
								      rmac);
							if (uptime != 0
							    && uptime != -1
							    &&
							    matchmac(interface,
								     var,
								     rmac)) {
								fprintf(stdout,
									"uptime is %d\n",
									uptime);
								return 0;
							}
						}
					}
				}
			}
			fprintf(stdout, "uptime is %d\n", uptime);
		}
	}
}
