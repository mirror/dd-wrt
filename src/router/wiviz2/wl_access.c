#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "wl_access.h"
#ifndef HAVE_MADWIFI
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
