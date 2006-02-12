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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <typedefs.h>
#include <wlioctl.h>
#include <wlutils.h>

int
wl_ioctl (char *name, int cmd, void *buf, int len)
{
  struct ifreq ifr;
  wl_ioctl_t ioc;
  int ret = 0;
  int s;

  /* open socket to kernel */
  if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("socket");
      return errno;
    }

  /* do it */
  ioc.cmd = cmd;
  ioc.buf = buf;
  ioc.len = len;
  strncpy (ifr.ifr_name, name, IFNAMSIZ);
  ifr.ifr_data = (caddr_t) & ioc;
  if ((ret = ioctl (s, SIOCDEVPRIVATE, &ifr)) < 0)
    if (cmd != WLC_GET_MAGIC)
      perror (ifr.ifr_name);

  /* cleanup */
  close (s);
  return ret;
}

int
wl_hwaddr (char *name, unsigned char *hwaddr)
{
  struct ifreq ifr;
  int ret = 0;
  int s;

  /* open socket to kernel */
  if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("socket");
      return errno;
    }

  /* do it */
  strncpy (ifr.ifr_name, name, IFNAMSIZ);
  if ((ret = ioctl (s, SIOCGIFHWADDR, &ifr)) == 0)
    memcpy (hwaddr, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);

  /* cleanup */
  close (s);
  return ret;
}
