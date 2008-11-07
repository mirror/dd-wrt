#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
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

#include "wl_access.h"
#ifndef HAVE_MADWIFI

#ifdef HAVE_RT2880

char *get_monitor(void)
{
return "ra0";
}

#else
int wl_ioctl(char *name, int cmd, void *buf, int len)
{
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret = 0;
 	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/* do it */
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_data = (caddr_t) &ioc;
	ret = ioctl(s, SIOCDEVPRIVATE, &ifr);

	/* cleanup */
	close(s);
	return ret;
}
#endif
#else


char *get_monitor(void)
{
int devcount;
char *ifname = get_wdev();
sscanf( ifname, "ath%d", &devcount );
static char mon[32];
sprintf(mon,"mon%d",devcount);
return mon;
}
#endif

int get_mac(char *name, void *buf)
{
	struct ifreq ifr;
	int ret = 0;
 	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	//ifr.ifr_data = (caddr_t) buf;
	if ((ret = ioctl(s, SIOCGIFHWADDR, &ifr)) < 0)
			perror(ifr.ifr_name);

	/* cleanup */
	close(s);
	memcpy(buf, &ifr.ifr_hwaddr.sa_data, 6);
	return ret;
}
