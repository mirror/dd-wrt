#include <stdio.h>
#include <malloc.h>
#include <wlutils.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>

#ifndef HAVE_ATH9K
struct wifi_client_info {
    void *dummy;
};
#endif

static int showAssocList(char *base, char *ifname, char *mac, struct wifi_client_info *rwc)
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
			fprintf(stdout, "assoclist %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n", p[pos], p[pos + 1], p[pos + 2], p[pos + 3], p[pos + 4], p[pos + 5]);
			pos += 6;
		}
		free(buf);

	}
	return 1;

}

static int matchmac(char *base, char *ifname, char *mac, struct wifi_client_info *rwc)
{
	unsigned char rmac[32];
	ether_etoa(mac, rmac);
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		struct mac80211_info *mac80211_info;
		struct wifi_client_info *wc;
		mac80211_info = mac80211_assoclist(base);
		for (wc = mac80211_info->wci; wc; wc = wc->next) {
			if (!strcmp(ifname, wc->ifname)
			    && !strcmp(rmac, wc->mac)) {
				memcpy(rwc, wc, sizeof(struct wifi_client_info));
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

static int showRssi(char *base, char *ifname, char *rmac, struct wifi_client_info *wc)
{
	int rssi;
#ifdef HAVE_ATH9K
	if (wc && is_ath9k(ifname))
		rssi = wc->signal;
	else
#endif
		rssi = getRssi(ifname, rmac);
	if (rssi != 0 && rssi != -1) {
		fprintf(stdout, "rssi is %d\n", rssi);
		return 1;
	}
	return 0;

}

static int showNoise(char *base, char *ifname, char *rmac, struct wifi_client_info *wc)
{
	int noise;
#ifdef HAVE_ATH9K
	if (wc && is_ath9k(ifname))
		noise = wc->noise;
	else
#endif
		noise = getNoise(ifname, rmac);
	if (noise != 0 && noise != -1) {
		fprintf(stdout, "noise is %d\n", noise);
		return 1;
	}
	return 0;

}

static int showRxRate(char *base, char *ifname, char *rmac, struct wifi_client_info *wc)
{
	int rxrate;
#ifdef HAVE_ATH9K
	if (wc && is_ath9k(ifname))
		rxrate = wc->rxrate;
	else
#endif
		rxrate = getRxRate(ifname, rmac);
	if (rxrate != 0 && rxrate != -1) {
		fprintf(stdout, "rxrate is %d\n", rxrate / 10);
		return 1;
	}
	return 0;

}

static int showTxRate(char *base, char *ifname, char *rmac, struct wifi_client_info *wc)
{
	int txrate;
#ifdef HAVE_ATH9K
	if (wc && is_ath9k(ifname))
		txrate = wc->txrate;
	else
#endif
		txrate = getTxRate(ifname, rmac);
	if (txrate != 0 && txrate != -1) {
		fprintf(stdout, "txrate is %d\n", txrate / 10);
		return 1;
	}
	return 0;

}

static int showIfname(char *base, char *ifname, char *rmac, struct wifi_client_info *wc)
{
	fprintf(stdout, "ifname is %s\n", ifname);
	return 1;
}

static int  showUptime(char *base, char *ifname, char *rmac, struct wifi_client_info *wc)
{
	int uptime;
#ifdef HAVE_ATH9K
	if (wc && is_ath9k(ifname))
		uptime = wc->uptime;
	else
#endif
		uptime = getUptime(ifname, rmac);
	if (uptime != 0 && uptime != -1) {
		fprintf(stdout, "uptime is %d\n", uptime);
		return 1;
	}
	return 0;
}

static char *UPTIME(int uptime)
{
	int days, minutes;
	static char str[64] = { 0 };
	char str2[64] = { 0 };
	bzero(str, 64);
	bzero(str2, 64);
	days = uptime / (60 * 60 * 24);
	if (days)
		sprintf(str2, "%d day%s, ", days, (days == 1 ? "" : "s"));
	minutes = uptime / 60;
	if (strlen(str2) > 0)
		sprintf(str, "%s %d:%02d:%02d", str2, (minutes / 60) % 24, minutes % 60, uptime % 60);
	else
		sprintf(str, "%d:%02d:%02d", (minutes / 60) % 24, minutes % 60, uptime % 60);
	return str;
}

static int showUptimeStr(char *base, char *ifname, char *rmac, struct wifi_client_info *wc)
{
	int uptime;
#ifdef HAVE_ATH9K
	if (wc && is_ath9k(ifname))
		uptime = wc->uptime;
	else
#endif
		uptime = getUptime(ifname, rmac);
	if (uptime != 0 && uptime != -1) {
		fprintf(stdout, "uptime is %s\n", UPTIME(uptime));
		return 1;
	}
	return 0;
}

typedef struct functions {
	char *fname;
	int (*fn) (char *base, char *ifname, char *rmac, struct wifi_client_info * wc);
	int matchmac;
} FN;

static FN fn[] = {
	{"rssi", &showRssi, 1},	//
	{"noise", &showNoise, 1},	//
	{"ifname", &showIfname, 1},	//
	{"uptime", &showUptime, 1},	//
	{"uptimestr", &showUptimeStr, 1},	//
	{"rxrate", &showRxRate, 1},	//
	{"txrate", &showTxRate, 1},	//
	{"assoclist", &showAssocList, 0}	//
};

static void evaluate(char *keyname, char *ifdecl, char *macstr)
{

	int i;
	int (*fnp) (char *base, char *ifname, char *rmac, struct wifi_client_info * wc);
	struct wifi_client_info wc;
	int m;
	for (i = 0; i < sizeof(fn) / sizeof(fn[0]); i++) {
		if (!strcmp(fn[i].fname, keyname)) {
			fnp = fn[i].fn;
			m = fn[i].matchmac;
		}
	}
	if (!fnp)
		return;
	if (!macstr && m)
		return;
	unsigned char rmac[6];
	if (macstr)
		ether_atoe(macstr, rmac);
	if (ifdecl) {
		fnp(ifdecl, ifdecl, rmac, NULL);
	} else {
		int ifcount = getdevicecount();

		int c = 0;
		for (c = 0; c < ifcount; c++) {
			char interface[32];
			sprintf(interface, "ath%d", c);
			if (!m || matchmac(interface, interface, rmac, &wc)) {
				int r = fnp(interface, interface, rmac, &wc);
				if (m && r)
					return;
			}
			char vif[32];
			sprintf(vif, "%s_vifs", interface);
			char var[80], *next;
			char *vifs = nvram_safe_get(vif);
			if (vifs != NULL) {
				foreach(var, vifs, next) {
					if (!m || matchmac(interface, var, rmac, &wc)) {
						int r  = fnp(interface, var, rmac, &wc);
						if (m && r)
							return;
					}
				}
			}
		}

	}
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "invalid argument\n");
		return 0;
	}
	char *ifname = NULL;
	int i;
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-i")) {
			if (i == (argc - 1))
				return -1;
			ifname = argv[++i];
			continue;
		}
		char *name = argv[i];
		evaluate(name, ifname, argv[++i]);
		return 0;

	}
}
