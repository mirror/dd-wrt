#include <sys/types.h>

#include <time.h>

#ifdef WINDOWS
#include <winsock2.h>

#else
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "peerlist.h"
#include "tracker.h"
#include "btcontent.h"

void Downloader()
{
  int nfds,maxfd,r;
  struct timeval timeout;
  time_t now;
  fd_set rfd;
  fd_set wfd;

  for(;;){
    time(&now);
    if( BTCONTENT.SeedTimeout(&now) ) break;
    
    FD_ZERO(&rfd); FD_ZERO(&wfd);
    maxfd = Tracker.IntervalCheck(&now,&rfd, &wfd);
    r = WORLD.FillFDSET(&now,&rfd,&wfd);
    if( r > maxfd ) maxfd = r;

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    nfds = select(maxfd + 1,&rfd,&wfd,(fd_set*) 0,&timeout);

	if(nfds > 0){
      if(T_FREE != Tracker.GetStatus()) Tracker.SocketReady(&rfd,&wfd,&nfds);
	  if( nfds ) WORLD.AnyPeerReady(&rfd,&wfd,&nfds);
	}
  }/* end for(;;) */
}
