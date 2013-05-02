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
	FILE *in = fopen("/tmp/pptp_connected", "rb");
	if (!in)
		return;
	char pid[32];
	char ifname[32];
	char local[32];
	char remote[32];
	char peer[64];
	int count = 0;
	while (fscanf(in, "%s %s %s %s %s", pid, ifname, local, remote, peer) == 5) {
		websWrite(wp, "%c\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"", count ? ',' : ' ', ifname, peer, local, remote, pid);
		count++;
		if (feof(in))
			break;
	}

	fclose(in);
	return;
}
