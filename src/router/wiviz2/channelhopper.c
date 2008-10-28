#include <stdio.h>
#include <pcap.h>
#include <signal.h>
#include <sys/time.h>
#include "wl_access.h"
#include "channelhopper.h"
#include "structs.h"

void ch_sig_handler(int i) {

  }

void channelHopper(wiviz_cfg * cfg) {
  int hopPos;
  int nc;

  //Turn off signal handling from parent process
  signal(SIGUSR1, &ch_sig_handler);
  signal(SIGUSR2, &ch_sig_handler);

  //Start hoppin'!
  hopPos = 0;
  while (1) {
    nc = cfg->channelHopSeq[hopPos];
    hopPos = (hopPos + 1) % cfg->channelHopSeqLen;
    //Set the channel
    fprintf(stderr, "It sets the channel to %i\n", nc);
#ifdef HAVE_MADWIFI
	    sysprintf("iwconfig %s channel %d\n",wl_dev,nc);
#else        
    wl_ioctl(get_wdev(), WLC_SET_CHANNEL, &nc, 4);
#endif
    //Sleep
    usleep(cfg->channelDwellTime * 1000);
    }
  }
