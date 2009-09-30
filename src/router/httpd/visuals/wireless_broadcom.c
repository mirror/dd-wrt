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

#define RSSI_TMP	"/tmp/.rssi"
#define ASSOCLIST_CMD	"wl assoclist"
#define RSSI_CMD	"wl rssi"
#define NOISE_CMD	"wl noise"

int
ej_active_wireless_if(webs_t wp, int argc, char_t ** argv,
		      char *iface, char *visible, int cnt)
{
	int rssi = 0, noise = 0;
	FILE *fp2;
	char *mode;
	char mac[30];
	char line[80];
	int macmask;
	macmask = atoi(argv[0]);
	if (!ifexists(iface))
		return cnt;
	unlink(RSSI_TMP);
	char wlmode[32];

	sprintf(wlmode, "%s_mode", visible);
	mode = nvram_safe_get(wlmode);
	unsigned char buf[WLC_IOCTL_MAXLEN];

	memset(buf, 0, WLC_IOCTL_MAXLEN);	// get_wdev
	int r = getassoclist(iface, buf);

	if (r < 0)
		return cnt;
	struct maclist *maclist = (struct maclist *)buf;
	int i;

	for (i = 0; i < maclist->count; i++) {
		ether_etoa((uint8 *) & maclist->ea[i], mac);

		rssi = 0;
		noise = 0;
		// get rssi value
		if (strcmp(mode, "ap") && strcmp(mode, "apsta")
		    && strcmp(mode, "apstawet"))
			sysprintf("wl -i %s rssi > %s", iface, RSSI_TMP);
		else
			sysprintf("wl -i %s rssi \"%s\" > %s", iface, mac,
				  RSSI_TMP);

		// get noise value if not ap mode
		// if (strcmp (mode, "ap"))
		// snprintf (cmd, sizeof (cmd), "wl -i %s noise >> %s", iface,
		// RSSI_TMP);
		// system2 (cmd); // get RSSI value for mac

		fp2 = fopen(RSSI_TMP, "r");
		if (fgets(line, sizeof(line), fp2) != NULL) {

			// get rssi
			if (sscanf(line, "%d", &rssi) != 1)
				continue;
			noise = getNoise(iface, NULL);
			/*
			 * if (strcmp (mode, "ap") && fgets (line, sizeof (line), fp2) != 
			 * NULL && sscanf (line, "%d", &noise) != 1) continue;
			 */
			// get noise for client/wet mode

			fclose(fp2);
		}
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
		if (cnt)
			websWrite(wp, ",");
		cnt++;
		/*
		 * if (!strcmp (mode, "ap")) { noise = getNoise(iface,NULL); // null
		 * only for broadcom }
		 */
		int qual = rssi * 124 + 11600;
		qual /= 10;
		websWrite(wp, "'%s','%s','N/A','N/A','N/A','%d','%d','%d','%d'",
			  mac, iface, rssi, noise, rssi - noise, qual);
	}
	unlink(RSSI_TMP);

	return cnt;
}

void ej_active_wireless(webs_t wp, int argc, char_t ** argv)
{
	int cnt = 0;
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		char wlif[32];

		sprintf(wlif, "wl%d", i);
		cnt =
		    ej_active_wireless_if(wp, argc, argv,
					  get_wl_instance_name(i), wlif, cnt);
		char *next;
		char var[80];
		char *vifs = nvram_nget("wl%d_vifs", i);

		if (vifs == NULL)
			return;

		foreach(var, vifs, next) {
			cnt =
			    ej_active_wireless_if(wp, argc, argv, var, var,
						  cnt);
		}
	}
}

