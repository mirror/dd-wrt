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
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>
#ifdef HAVE_MADWIFI
#include "wireless.h"
#endif
#include "wl_access.h"
#include "channelhopper.h"
#include "structs.h"


void ch_sig_handler(int i) {

  }

#ifdef HAVE_MADWIFI
#define IEEE80211_CHAN_2GHZ 1
#define IEEE80211_CHAN_5GHZ 2
u_int ieee80211_ieee2mhz(u_int chan, u_int flags)
{
	if (flags & IEEE80211_CHAN_2GHZ) {	/* 2GHz band */
		if (chan == 14)
			return 2484;
		if (chan < 14)
			return ((2407) + chan * 5);
		else {
			if (chan > 236 && chan < 256) {
				//recalculate offset
				int newchan = chan - 256;
				int newfreq = (2407) + (newchan * 5);
				return newfreq;
			} else
				return ((2512) + ((chan - 15) * 20));
		}
	} else if (flags & IEEE80211_CHAN_5GHZ)	/* 5Ghz band */
		return ((5000) + (chan * 5));
	else {			/* either, guess */
		if (chan == 14)
			return 2484;
		if (chan < 14)	/* 0-13 */
			return ((2407) + chan * 5);
		if (chan < 27)	/* 15-26 */
			return ((2512) + ((chan - 15) * 20));
		if (chan > 236 && chan < 256) {
			//recalculate offset
			int newchan = chan - 256;
			int newfreq = (2407) + (newchan * 5);
			return newfreq;
		} else
			return ((5000) + (chan * 5));
	}
}


int set_channel(char *dev,int channel)
{
    struct iwreq wrq;
    memset( &wrq, 0, sizeof( struct iwreq ) );
    strncpy( wrq.ifr_name, get_monitor(), IFNAMSIZ );
    if (channel>14)
	wrq.u.freq.m = (double) ieee80211_ieee2mhz(channel,2) * 100000;
    else
	wrq.u.freq.m = (double) ieee80211_ieee2mhz(channel,1) * 100000;
    wrq.u.freq.e = (double) 1;
    
    if( ioctl( getsocket(), SIOCSIWFREQ, &wrq ) < 0 )
    {
	return -1;
//        usleep( 10000 ); /* madwifi needs a second chance */

//        if( ioctl( getsocket(), SIOCSIWFREQ, &wrq ) < 0 )
//        {
//            return;
//        }
    }
return 0;
}
#endif
void channelHopper(wiviz_cfg * cfg) {
  int hopPos;
  int nc;

  //Turn off signal handling from parent process
  signal(SIGUSR1, &ch_sig_handler);
  signal(SIGUSR2, &ch_sig_handler);

  //Start hoppin'!
  hopPos = 0;
    printf("set hop %d\n",nc);
  while (1) {
    int hop = cfg->channelHopSeq[hopPos];
    if (hop==0)
	nc++;
    else
    {
    nc = hop;
    hopPos = (hopPos + 1) % cfg->channelHopSeqLen;
    }
#ifdef HAVE_MADWIFI
    if (nc>255)nc=1;
#elif HAVE_RT2880
    if (nc>14)nc=1;
#else
    if (nc>255)nc=1;
#endif
    //Set the channel
#ifdef HAVE_MADWIFI
    printf("set channel %d\n",nc);
    int ret = set_channel(get_wdev(),nc);
    if (ret==-1)
	continue;
#elif HAVE_RT2880
    sysprintf("iwpriv ra0 set Channel=%d",nc);
#else
    char tmp[32];
    sprintf( tmp, "%s_ifname", nvram_safe_get( "wifi_display" ) );
    char *wl_dev = nvram_safe_get( tmp );
    if (wl_ioctl(wl_dev, WLC_SET_CHANNEL, &nc, 4)<0)
	continue;
#endif
    //Sleep
    usleep(cfg->channelDwellTime * 1000);
    }
  }
