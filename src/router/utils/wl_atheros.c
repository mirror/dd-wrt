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
			fprintf(stdout, "assoclist %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n", p[pos], p[pos + 1], p[pos + 2], p[pos + 3], p[pos + 4], p[pos + 5]);
			pos += 6;
		}
		free(buf);

	}

}

int matchmac(char *base, char *ifname, char *mac)
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

void showRssi(char *base, char *ifname, char *rmac)
{
	int rssi = getRssi(ifname, rmac);
	if (rssi != 0 && rssi != -1) {
		fprintf(stdout, "rssi is %d\n", rssi);
	}

}

void showNoise(char *base, char *ifname, char *rmac)
{

	int noise = getNoise(ifname, rmac);
	if (noise != 0 && noise != -1) {
		fprintf(stdout, "noise is %d\n", noise);
	}

}

void showIfname(char *base, char *ifname, char *rmac)
{
	fprintf(stdout, "ifname is %s\n", ifname);

}

void showUptime(char *base, char *ifname, char *rmac)
{
	int uptime = getUptime(ifname, rmac);
	if (uptime != 0 && uptime != -1) {
		fprintf(stdout, "uptime is %d\n", uptime);
	}
}

typedef struct functions {
	char *fname;
	void (*fn) (char *base, char *ifname, char *rmac);
}

struct functions fn[] = {
	{
	"rssi", &showRssi},	//
	{
	"noise", &showNoise},	//
	{
	"ifname", &showIfname},	//
	{
	"uptime", &showUptime},	//
};

void evaluate(char *keyname, char *ifdecl, char *macstr)
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
			return;
		evaluate(name, ifname, argv[++i]);
		return;

	}
}
