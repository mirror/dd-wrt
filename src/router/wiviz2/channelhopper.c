#include <stdio.h>
#include <pcap.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ddnvram.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>
#ifdef HAVE_MADWIFI
#include "wireless.h"
#endif
#include "wl_access.h"
#include "channelhopper.h"
#include "structs.h"
#include <wlutils.h>

void ch_sig_handler(int i)
{

}

#ifdef HAVE_MADWIFI

char *get_monitor(void);

int set_channel(wiviz_cfg * cfg, char *dev, int channel)
{
	struct iwreq wrq;
	int flags = 0;
	if (is_mac80211(nvram_safe_get("wifi_display"))) {
		char chann[32];
		sprintf(chann, "%d", channel);
		if (cfg && is_ath10k(nvram_safe_get("wifi_display"))) {
			char dwell[32];
			sprintf(dwell, "%d", cfg->channelDwellTime);
			eval("iw", "dev", nvram_safe_get("wifi_display"), "offchannel", chann, dwell);
		} else {
			eval("iw", "dev", dev, "set", "freq", chann);
		}
	} else {
		memset(&wrq, 0, sizeof(struct iwreq));
		strncpy(wrq.ifr_name, get_monitor(), IFNAMSIZ);
		wrq.u.freq.m = channel * 100000;
		wrq.u.freq.e = (double)1;

		if (ioctl(getsocket(), SIOCSIWFREQ, &wrq) < 0) {
			return -1;
//        usleep( 10000 ); /* madwifi needs a second chance */

//        if( ioctl( getsocket(), SIOCSIWFREQ, &wrq ) < 0 )
//        {
//            return;
//        }
		}
	}
	return 0;
}
#endif
#ifdef HAVE_MADWIFI
struct wifi_channels *wifi_channels;
#endif
extern char *wl_dev;

void channelHopper(wiviz_cfg * cfg)
{
	int hopPos;
#ifdef HAVE_MADWIFI
	int nc = 0;
	const char *country = getIsoName(nvram_default_get("wlan0_regdomain", "UNITED_STATES"));
	if (!country)
		country = "DE";
	if (is_mac80211(wl_dev)) {
		wifi_channels = mac80211_get_channels_simple(wl_dev, country, 20, 0xff);
	} else {
		wifi_channels = list_channels(wl_dev);
	}

#else
	int nc = -1;
#endif

	//Turn off signal handling from parent process
	signal(SIGUSR1, &ch_sig_handler);
	signal(SIGUSR2, &ch_sig_handler);
	signal(SIGTERM, &ch_sig_handler);

	//Start hoppin'!
	hopPos = 0;
	printf("set hop %d\n", nc);
	while (1) {
		int hop = cfg->channelHopSeq[hopPos];
		if (hop == 0)
			nc++;
		else {
			nc = hop;
			hopPos = (hopPos + 1) % cfg->channelHopSeqLen;
		}
#ifdef HAVE_MADWIFI
		if (wifi_channels[nc].freq == -1) {
			nc = -1;
			continue;
		}
#elif HAVE_RT2880
		if (nc > 14)
			nc = 1;
#else
		if (nc > 255)
			nc = 1;
#endif
		//Set the channel
#ifdef HAVE_MADWIFI
		{
			printf("set channel %d\n", nc);
			int ret = set_channel(cfg, get_monitor(), wifi_channels[nc].freq);
			if (ret == -1)
				continue;
		}
#elif HAVE_RT2880
		if (nvram_match("wifi_display", "wl0"))
			sysprintf("iwpriv ra0 set Channel=%d", nc);
		else
			sysprintf("iwpriv ba0 set Channel=%d", nc);

#else
		char tmp[32];
		sprintf(tmp, "%s_ifname", nvram_safe_get("wifi_display"));
		char *wl_dev = nvram_safe_get(tmp);
		if (wl_ioctl(wl_dev, WLC_SET_CHANNEL, &nc, 4) < 0)
			continue;
#endif
		//Sleep
		usleep(cfg->channelDwellTime * 1000);
	}
}
