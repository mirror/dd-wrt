#include <stdio.h>
#include <malloc.h>
#include <wlutils.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>
#include <airbag.h>

struct wifi_info {
	unsigned char mac[6];
	char ifname[32];
	int rssi;
	int noise;
	int rxrate;
	int txrate;
	int uptime;
};

static int _showAssocList(char *base, char *ifname, char *mac, struct wifi_info *rwc, int silent)
{
	char buf[18];
	if (is_mac80211(ifname)) {
		struct mac80211_info *mac80211_info;
		struct wifi_client_info *wc;
		mac80211_info = mac80211_assoclist(base);
		for (wc = mac80211_info->wci; wc; wc = wc->next) {
			if (!strncmp(ifname, wc->ifname, strlen(ifname))) {
				char out[48];
				sprintf(out, "/tmp/snmp_cache/%s", base);
				mkdir(out, 0777);
				sprintf(out, "/tmp/snmp_cache/%s/%s", base, ether_etoa(wc->etheraddr, buf));
				struct wifi_info data;
				memcpy(data.mac, wc->etheraddr, 6);
				strcpy(data.ifname, wc->ifname);
				data.rssi = wc->signal;
				data.noise = wc->noise;
				data.rxrate = wc->rxrate;
				data.txrate = wc->txrate;
				data.uptime = wc->uptime;
				FILE *fp = fopen(out, "wb");
				fwrite(&data, 1, sizeof(data), fp);
				fclose(fp);
				if (!silent)
					fprintf(stdout, "assoclist %s\n", ether_etoa(wc->etheraddr, buf));
			}
		}
		free_wifi_clients(mac80211_info->wci);
		free(mac80211_info);
	} else {

		char *buf = malloc(8192);
		memset(buf, 0, 8192);
		int cnt = getassoclist(ifname, buf);
		int count;
		memcpy(&count, buf, 4);
		unsigned char *p = &buf[4];
		int a;
		int pos = 0;
		for (a = 0; a < cnt; a++) {
			struct wifi_info data;
			memcpy(&data.mac[0], &p[pos], 6);

			strcpy(&data.ifname[0], ifname);
			data.rssi = getRssi(ifname, &data.mac[0]);
			data.noise = getNoise(ifname, &data.mac[0]);
			data.rxrate = getRxRate(ifname, &data.mac[0]);
			data.txrate = getTxRate(ifname, &data.mac[0]);
			data.uptime = getUptime(ifname, &data.mac[0]);
			char mstr[32];
			sprintf(mstr, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", p[pos], p[pos + 1], p[pos + 2], p[pos + 3], p[pos + 4], p[pos + 5]);
			char out[48];
			sprintf(out, "/tmp/snmp_cache/%s", base);
			mkdir(out, 0777);
			sprintf(out, "/tmp/snmp_cache/%s/%s", base, mstr);
			FILE *fp = fopen(out, "wb");
			fwrite(&data, 1, sizeof(data), fp);
			fclose(fp);
			if (!silent)
				fprintf(stdout, "assoclist %s\n", mstr);
			pos += 6;
		}
		free(buf);

	}
	return 1;

}

static int showAssocList(char *base, char *ifname, char *mac, struct wifi_info *rwc)
{
	_showAssocList(base, ifname, mac, rwc, 0);
}

static int matchmac(char *base, char *ifname, char *mac, struct wifi_info *rwc)
{
	unsigned char rmac[32];
	ether_etoa(mac, rmac);
	char out[48];
	char mstr[32];
	int assoclist = 0;
	sprintf(out, "/tmp/snmp_cache/%s/%s", base, rmac);
      retry:;
	FILE *in = fopen(out, "rb");
	if (!in) {
		if (!assoclist) {
			_showAssocList(base, ifname, mac, rwc, 1);
			assoclist = 1;
			goto retry;
		}
		return 0;
	}
	struct wifi_info wc;
	fread(&wc, 1, sizeof(wc), in);
	fclose(in);
	if (!strncmp(wc.ifname, ifname, strlen(ifname))) {
		memcpy(rwc, &wc, sizeof(wc));
		return 1;
	}
	return 0;
}

static int showRssi(char *base, char *ifname, char *rmac, struct wifi_info *wc)
{
	int rssi = wc->rssi;
	if (rssi != 0 && rssi != -1) {
		fprintf(stdout, "rssi is %d\n", rssi);
		return 1;
	}
	return 0;

}

static int showNoise(char *base, char *ifname, char *rmac, struct wifi_info *wc)
{
	int noise = wc->noise;
	if (noise != 0 && noise != -1) {
		fprintf(stdout, "noise is %d\n", noise);
		return 1;
	}
	return 0;

}

static int showRxRate(char *base, char *ifname, char *rmac, struct wifi_info *wc)
{
	int rxrate = wc->rxrate;
	if (rxrate != 0 && rxrate != -1) {
		fprintf(stdout, "rxrate is %d\n", rxrate / 10);
		return 1;
	}
	return 0;

}

static int showTxRate(char *base, char *ifname, char *rmac, struct wifi_info *wc)
{
	int txrate = wc->txrate;
	if (txrate != 0 && txrate != -1) {
		fprintf(stdout, "txrate is %d\n", txrate / 10);
		return 1;
	}
	return 0;

}

static int showIfname(char *base, char *ifname, char *rmac, struct wifi_info *wc)
{
	fprintf(stdout, "ifname is %s\n", ifname);
	return 1;
}

static int showUptime(char *base, char *ifname, char *rmac, struct wifi_info *wc)
{
	int uptime = wc->uptime;
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

static int showUptimeStr(char *base, char *ifname, char *rmac, struct wifi_info *wc)
{
	int uptime = wc->uptime;
	if (uptime != 0 && uptime != -1) {
		fprintf(stdout, "uptime is %s\n", UPTIME(uptime));
		return 1;
	}
	return 0;
}

typedef struct functions {
	char *fname;
	int (*fn) (char *base, char *ifname, char *rmac, struct wifi_info * wc);
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
	int (*fnp) (char *base, char *ifname, char *rmac, struct wifi_info * wc) = NULL;
	struct wifi_info wc;
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
		char *base = strdup(ifdecl);
		char *s = strchr(base, '.');
		if (s)
			*s = 0;
		if (macstr && !matchmac(base, ifdecl, rmac, &wc)) {
			free(base);
			return;
		}
		fnp(base, ifdecl, rmac, &wc);
		free(base);
	} else {
		int ifcount = getdevicecount();

		int c = 0;
		for (c = 0; c < ifcount; c++) {
			char interface[32];
			sprintf(interface, "wlan%d", c);
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
						int r = fnp(interface, var, rmac, &wc);
						if (m && r)
							return;
					}
				}
			}
		}

	}
}
#ifdef HAVE_ATH9K
void special_mac80211_init(void);
#endif
int main(int argc, char *argv[])
{
	airbag_init();
#ifdef HAVE_ATH9K
	special_mac80211_init();
#endif
	mkdir("/tmp/snmp_cache", 0777);
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
