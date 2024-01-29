/*
 * Wireless network adapter utilities (linux-specific)
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wl_linux.c,v 1.2 2005/11/11 09:26:19 seg Exp $
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/types.h>

#ifdef __UCLIBC__
typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#else
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

#endif
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include <typedefs.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <utils.h>

static int s_socket = -1;
int getsocket(void)
{
	if (s_socket < 0) {
		s_socket = socket(AF_INET, SOCK_DGRAM, 0);
		if (s_socket < 0)
			perror("socket(SOCK_DGRAM)");
	}
	return s_socket;
}

void closesocket(void)
{
	if (s_socket >= 0) {
		close(s_socket);
		s_socket = -1;
	}
}

int wl_ioctl(char *name, int cmd, void *buf, int len)
{
	struct ifreq ifr;
	wl_ioctl_t ioc;
	int ret = 0;
	int s;

	/*
	 * open socket to kernel 
	 */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/*
	 * do it 
	 */
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;

	/* initializing the remaining fields */
	ioc.set = FALSE;
	ioc.used = 0;
	ioc.needed = 0;

	strlcpy(ifr.ifr_name, name, IFNAMSIZ - 1);
	ifr.ifr_data = (caddr_t)&ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0)
		if (cmd != WLC_GET_MAGIC)
			perror(ifr.ifr_name);

	/*
	 * cleanup 
	 */
	close(s);
	return ret;
}

int wl_hwaddr(char *name, unsigned char *hwaddr)
{
	struct ifreq ifr;
	int ret = 0;
	int s;
	if (is_wil6210(name))
		name = "giwifi0";
#if defined(HAVE_DIR862) && !defined(HAVE_DAP2680)
	if (!strcmp(name, "wlan1"))
		name = "wlan0";
#endif

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/*
	 * do it 
	 */
	strlcpy(ifr.ifr_name, name, IFNAMSIZ - 1);
	if ((ret = ioctl(s, SIOCGIFHWADDR, &ifr)) == 0)
		memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);

	/*
	 * cleanup 
	 */
	close(s);
	return ret;
}

int wl_bssid(char *name, unsigned char *hwaddr)
{
	wl_ioctl(name, WLC_GET_BSSID, hwaddr, ETHER_ADDR_LEN);
	return 0;
}

int wl_get_dev_type(char *name, void *buf, int len)
{
	int s;
	int ret;
	struct ifreq ifr;
	struct ethtool_drvinfo info;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "socket");
		return -1;
	}

	/* get device type */
	bzero(&info, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t)&info;
	strlcpy(ifr.ifr_name, name, IFNAMSIZ - 1);
	if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {
		/* print a good diagnostic if not superuser */
		if (errno == EPERM)
			fprintf(stderr, "wl_get_dev_type");

		*(char *)buf = '\0';
	} else
		strncpy(buf, info.driver, len);

	close(s);
	return ret;
}
