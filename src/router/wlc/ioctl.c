/*
 * Wireless network adapter utilities
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wl.c,v 1.1.1.11 2006/02/27 03:43:20 honor Exp $
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/types.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include <typedefs.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include <wlutils.h>
#ifndef WLC_IOCTL_SMLEN
#define	WLC_IOCTL_SMLEN		256		/* "small" length ioctl buffer required */
#endif
#ifndef BCME_BUFTOOSHORT
#define BCME_BUFTOOSHORT		-14	/* Buffer too short */
#endif

static inline int
wl_iovar_getint(char *ifname, char *iovar, int *val)
{
	return wl_iovar_get(ifname, iovar, val, sizeof(int));
}

int
wl_ioctl(char *name, int cmd, void *buf, int len)
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
	if ((ret = ioctl(s, SIOCDEVPRIVATE, &ifr)) < 0)

	/* cleanup */
	close(s);
	return ret;
}

static inline int
wl_get_dev_type(char *name, void *buf, int len)
{
	int s;
	int ret;
	struct ifreq ifr;
	struct ethtool_drvinfo info;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	/* get device type */
	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t)&info;
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {
		*(char *)buf = '\0';
	} else
		strncpy(buf, info.driver, len);

	close(s);
	return ret;
}

int
wl_probe(char *name)
{
	int ret, val;
	char buf[3];
	if ((ret = wl_get_dev_type(name, buf, 3)) < 0)
		return ret;
	/* Check interface */
	if (strncmp(buf, "wl", 2))
		return -1;
	if ((ret = wl_ioctl(name, WLC_GET_VERSION, &val, sizeof(val))))
		return ret;
	if (val > WLC_IOCTL_VERSION)
		return -1;

	return ret;
}



/*
 * format a bsscfg indexed iovar buffer
 */
static int
wl_bssiovar_mkbuf(char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen,
                  int *plen)
{
	char *prefix = "bsscfg:";
	int8* p;
	uint prefixlen;
	uint namelen;
	uint iolen;

	prefixlen = strlen(prefix);	/* length of bsscfg prefix */
	namelen = strlen(iovar) + 1;	/* length of iovar name + null */
	iolen = prefixlen + namelen + sizeof(int) + paramlen;

	/* check for overflow */
	if (buflen < 0 || iolen > (uint)buflen) {
		*plen = 0;
		return BCME_BUFTOOSHORT;
	}

	p = (int8*)bufptr;

	/* copy prefix, no null */
	memcpy(p, prefix, prefixlen);
	p += prefixlen;

	/* copy iovar name including null */
	memcpy(p, iovar, namelen);
	p += namelen;

	/* bss config index as first param */
	memcpy(p, &bssidx, sizeof(int32));
	p += sizeof(int32);

	/* parameter buffer follows */
	if (paramlen)
		memcpy(p, param, paramlen);

	*plen = iolen;
	return 0;
}

void
wl_printlasterror(char *name)
{
	char err_buf[WLC_IOCTL_SMLEN];
	strcpy(err_buf, "bcmerrstr");

	fprintf(stderr, "Error: ");
	if ( wl_ioctl(name, WLC_GET_VAR, err_buf, sizeof (err_buf)) != 0)
		fprintf(stderr, "Error getting the Errorstring from driver\n");
	else
		fprintf(stderr, err_buf);
}
