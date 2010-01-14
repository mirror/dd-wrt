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
	static char a[18];
	int i;

	i = snprintf(a, sizeof(a), "%02x:%02x:%02x:%02x:%02x:%02x",
		     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return (i < 17 ? NULL : a);
}

int
ej_active_wireless_if_11n(webs_t wp, int argc, char_t ** argv,
			  char *ifname, int cnt, int turbo, int macmask)
{
	// unsigned char buf[24 * 1024];

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
	int state = get_radiostate(ifname);

	if (state == 0 || state == -1) {
		printf("IOCTL_STA_INFO radio %s not enabled!\n", ifname);
		return cnt;
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		fprintf(stderr, "socket(SOCK_DRAGM)\n");
		return cnt;
	}
	(void)memset(&iwr, 0, sizeof(struct iwreq));
	(void)strncpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));
	unsigned char *buf = (unsigned char *)malloc(24 * 1024);

	iwr.u.data.pointer = (void *)buf;
	iwr.u.data.length = 24 * 1024;
	if (ioctl(s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0) {
		fprintf(stderr, "IOCTL_STA_INFO for %s failed!\n", ifname);
		close(s);
		free(buf);
		return cnt;
	}
	len = iwr.u.data.length;
	if (len < sizeof(struct ieee80211req_sta_info)) {
		// fprintf(stderr,"IOCTL_STA_INFO len<struct %s failed!\n",ifname);
		close(s);
		free(buf);
		return cnt;
	}
	cp = buf;
	do {
		struct ieee80211req_sta_info *si;
		uint8_t *vp;

		si = (struct ieee80211req_sta_info *)cp;
		vp = (u_int8_t *)(si + 1);

		if (cnt)
			websWrite(wp, ",");
		cnt++;
		char mac[32];

		strcpy(mac, ieee80211_ntoa(si->isi_macaddr));
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
		if (si->isi_txrateKbps && si->isi_rxrateKbps) {
			websWrite(wp,
				  "'%s','%s','%s','%3dM','%3dM','%d','%d','%d','%d'",
				  mac, ifname, UPTIME(si->isi_uptime),
				  si->isi_txrateKbps, si->isi_rxrateKbps,
				  si->isi_noise + si->isi_rssi + bias,
				  si->isi_noise + bias, si->isi_rssi, qual);
		} else if (si->isi_rates
			   &&
			   ((si->
			     isi_rates[si->isi_txrate] & IEEE80211_RATE_VAL) !=
			    0)
			   &&
			   ((si->
			     isi_rates[si->isi_rxrate] & IEEE80211_RATE_VAL) !=
			    0)) {
			websWrite(wp,
				  "'%s','%s','%s','%3dM','%3dM','%d','%d','%d','%d'",
				  mac, ifname, UPTIME(si->isi_uptime),
				  ((si->
				    isi_rates[si->
					      isi_txrate] & IEEE80211_RATE_VAL)
				   / 2) * turbo,
				  ((si->
				    isi_rates[si->
					      isi_rxrate] & IEEE80211_RATE_VAL)
				   / 2) * turbo,
				  si->isi_noise + si->isi_rssi + bias,
				  si->isi_noise + bias, si->isi_rssi, qual);
		} else {
			websWrite(wp,
				  "'%s','%s','%s','N/A','N/A','%d','%d','%d','%d'",
				  mac, ifname, UPTIME(si->isi_uptime),
				  si->isi_noise + si->isi_rssi + bias,
				  si->isi_noise + bias, si->isi_rssi, qual);
		}
		cp += si->isi_len;
		len -= si->isi_len;
	}
	while (len >= sizeof(struct ieee80211req_sta_info));
	free(buf);
	close(s);

	return cnt;
}

#endif
