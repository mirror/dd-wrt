#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <ctype.h>

#include <broadcom.h>


void ej_dumppptp(webs_t wp, int argc, char_t ** argv)
{
	FILE *in = fopen("/tmp/pptp_connected","rb");
	if (!in)
	    return;
	char ifname[32];
	char dev[32];
	char speed[32];
	char server[32];
	char local[32];
	char remote[32];
	int count=0;
	while(fscanf(in,"%s %s %s %s %s %s",ifname,dev,speed,server,local,remote)==6)
	    {
	    websWrite(wp,"%c\"%s\",\"%s\",\"%s\",\"%s\",\"%d\"",
					  count ? ',' : ' ',ifname,local,remote);
	    count++;
	    if (feof(in))
		break;
	    }    
	    
	fclose(in);
	return;
}
