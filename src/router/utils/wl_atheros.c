#include <stdio.h>
#include <malloc.h>
#include <wlutils.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>

static void showinterface(char *base, char *ifname)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		struct mac80211_info *mac80211_info;
		struct wifi_client_info *wc;
		mac80211_info = mac80211_assoclist(base);
		for (wc = mac80211_info->wci; wc; wc = wc->next) {
			if (!strcmp(ifname, wc->ifname))
				// dd_syslog(LOG_INFO, "assoclist %s\n", wc->mac);
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
			// dd_syslog(LOG_INFO, "assoclist %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n", p[pos], p[pos + 1], p[pos + 2], p[pos + 3], p[pos + 4], p[pos + 5]);
			fprintf(stdout, "assoclist %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n", p[pos], p[pos + 1], p[pos + 2], p[pos + 3], p[pos + 4], p[pos + 5]);
			pos += 6;
		}
		free(buf);

	}

}

static int matchmac(char *base, char *ifname, char *mac)
{
	unsigned char rmac[32];
	ether_etoa(mac, rmac);
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		struct mac80211_info *mac80211_info;
		struct wifi_client_info *wc;
		mac80211_info = mac80211_assoclist(base);
		for (wc = mac80211_info->wci; wc; wc = wc->next) {
//                      fprintf(stderr,"%s == %s\n",wc->ifname,ifname);
//                      fprintf(stderr,"%s == %s\n",wc->mac, mac);
			if (!strcmp(ifname, wc->ifname)
			    && !strcmp(rmac, wc->mac)) {
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

static void showRssi(char *base, char *ifname, char *rmac)
{
	int rssi = getRssi(ifname, rmac);
	if (rssi != 0 && rssi != -1) {
		// dd_syslog(LOG_INFO, "rssi %d\n",rssi);
		fprintf(stdout, "rssi is %d\n", rssi);
	}

}

static void showNoise(char *base, char *ifname, char *rmac)
{

	int noise = getNoise(ifname, rmac);
	if (noise != 0 && noise != -1) {
		// dd_syslog(LOG_INFO, "noise %d\n",noise);
		fprintf(stdout, "noise is %d\n", noise);
	}

}

static void showRxRate(char *base, char *ifname, char *rmac)
{

	int rxrate = getRxRate(ifname, rmac);
	if (rxrate != 0 && rxrate != -1) {
		// dd_syslog(LOG_INFO, "rxrate %d\n",rxrate);
		fprintf(stdout, "rxrate is %d\n", rxrate);
	}

}

static void showTxRate(char *base, char *ifname, char *rmac)
{

	int txrate = getTxRate(ifname, rmac);
	if (txrate != 0 && txrate != -1) {
		// dd_syslog(LOG_INFO, "txrate %d\n",txrate);
		fprintf(stdout, "txrate is %d\n", txrate);
	}

}

static void showIfname(char *base, char *ifname, char *rmac)
{
	fprintf(stdout, "ifname is %s\n", ifname);

}

static void showUptime(char *base, char *ifname, char *rmac)
{
	int uptime = getUptime(ifname, rmac);
	if (uptime != 0 && uptime != -1) {
		// dd_syslog(LOG_INFO, "uptime %d\n",uptime);
		fprintf(stdout, "uptime is %d\n", uptime);
	}
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

static void showUptimeStr(char *base, char *ifname, char *rmac)
{
	int uptime = getUptime(ifname, rmac);
	if (uptime != 0 && uptime != -1) {
		// dd_syslog(LOG_INFO, "uptime %s\n",UPTIME(uptime));
		fprintf(stdout, "uptime is %s\n", UPTIME(uptime));
	}
}

typedef struct functions {
	char *fname;
	void (*fn) (char *base, char *ifname, char *rmac);
} FN;

FN fn[] = {
	{
	 "rssi", &showRssi},	//
	{
	 "noise", &showNoise},	//
	{
	 "ifname", &showIfname},	//
	{
	 "uptime", &showUptime},	//
	{
	 "uptimestr", &showUptimeStr},	//
	{
	 "rxrate", &showRxRate},	//
	{
	 "txrate", &showTxRate}	//
};

static void evaluate(char *keyname, char *ifdecl, char *macstr)
{

	int i;
	void (*fnp) (char *base, char *ifname, char *rmac);

	for (i = 0; i < sizeof(fn) / sizeof(fn[0]); i++) {
		if (!strcmp(fn[i].fname, keyname))
			fnp = fn[i].fn;
	}
	if (!fnp)
		return;

	unsigned char rmac[6];
	ether_atoe(macstr, rmac);

	if (ifdecl) {
		fnp(ifdecl, ifdecl, rmac);
	} else {
		int ifcount = getdevicecount();

		int c = 0;
		for (c = 0; c < ifcount; c++) {
			char interface[32];
			sprintf(interface, "ath%d", c);
			if (matchmac(interface, interface, rmac)) {
				fnp(interface, interface, rmac);
				return;
			}
			char vif[32];
			sprintf(vif, "%s_vifs", interface);
			char var[80], *next;
			char *vifs = nvram_safe_get(vif);
			if (vifs != NULL) {
				foreach(var, vifs, next) {
					if (matchmac(interface, var, rmac)) {
						fnp(interface, var, rmac);
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
	// dd_syslog(LOG_INFO, "Call wl_atheros, arguments %d\n",argc);
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-i")) {
			ifname = argv[++i];
			// dd_syslog(LOG_INFO, "interface %s\n",ifname);
			continue;
		}
		
		// dd_syslog(LOG_INFO, "call %s\n",argv[i]);
		if (!strcmp(argv[i], "assoclist")) {
			int ifcount = getdevicecount();
			if (strcmp(argv[1], "-i") == 0)
				showinterface(ifname, ifname);
			else {
				int c = 0;
				for (c = 0; c < ifcount; c++) {
					char interface[32];
					sprintf(interface, "ath%d", c);
					showinterface(interface, interface);
					char vif[32];
					sprintf(vif, "%s_vifs", interface);
					char var[80], *next;
					char *vifs = nvram_safe_get(vif);
					if (vifs != NULL) {
						foreach(var, vifs, next) {
							showinterface(interface, var);
						}
					}
				}
			}

		}
		char *name = argv[i];
		if (i == (argc - 1))
			return -1;
		evaluate(name, ifname, argv[++i]);
		return 0;

	}
}
