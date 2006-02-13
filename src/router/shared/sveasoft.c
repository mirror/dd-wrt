/*
 *********************************************************
 *   Copyright 2003, Sveasoft AB  All Rights Reserved *
 *********************************************************

 This is UNPUBLISHED PROPRIETARY SOURCE CODE of Sveasoft AB
 the contents of this file MAY BE disclosed to third parties,
 copied or duplicated in any form without the prior written
 permission of Sveasoft AB

 This software should be used as a reference only, and it not
 intended for production use!


 THIS SOFTWARE IS OFFERED "AS IS", AND SVEASOFT GRANTS NO WARRANTIES OF ANY
 KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  SVEASOFT
 SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <proto/ethernet.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <bcmutils.h>

#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlioctl.h>

#include <cy_conf.h>
#include <utils.h>

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

//#define WDS_DEBUG 1
#undef WDS_DEBUG
#ifdef WDS_DEBUG
FILE *fp;
#endif


int
wds_dev_config (int dev, int up)
{
  char wds_var[32] = "";
  char wds_enable_var[32] = "";
  char wds_dev[32] = "";
  char *wds = (void *) 0;
  char wds_gw_var[32] = "";
  char cmd[100] = "";
  char *gw = (void *) 0;
  int s = -1;
  struct ifreq ifr;

#ifdef WDS_DEBUG
  fp = fopen ("/tmp/.wds_debug.log", "a");
#endif

  memset (&ifr, 0, sizeof (struct ifreq));

  snprintf (wds_var, 31, "wl_wds%d", dev);
  snprintf (wds_enable_var, 31, "%s_enable", wds_var);

  if ((wds = nvram_safe_get (wds_enable_var)) == NULL ||
      strcmp (wds, "0") == 0)
    return -1;

  snprintf (wds_dev, 31, "wds0.4915%d", dev + 1);

  snprintf (ifr.ifr_name, IFNAMSIZ, wds_dev);
#ifdef WDS_DEBUG
  fprintf (fp, "opening kernelsocket\n");
#endif
  if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    return -1;

  if (up)
    {
      char wds_hwaddr_var[32] = "";
      char wds_ip_var[32] = "";
      char wds_netmask_var[32] = "";
      char *wds_list = (void *) 0;
      char *hwaddr = (void *) 0;
      char *ip = (void *) 0;
      char *netmask = (void *) 0;

#ifdef WDS_DEBUG
      fprintf (fp, "running up\n");
#endif

      wds_list = nvram_safe_get ("wl0_wds");
      if (wds_list == (void *) 0 || strlen (wds_list) <= 0)
	return 0;

      snprintf (wds_hwaddr_var, 31, "%s_hwaddr", wds_var);
      snprintf (wds_ip_var, 31, "%s_ipaddr", wds_var);
      snprintf (wds_netmask_var, 31, "%s_netmask", wds_var);

      hwaddr = nvram_safe_get (wds_hwaddr_var);
      ip = nvram_safe_get (wds_ip_var);
      netmask = nvram_safe_get (wds_netmask_var);

      if (!strstr (wds_list, hwaddr))
	return -1;

#ifdef WDS_DEBUG
      fprintf (fp, "checking validity\n");
#endif

      if (!sv_valid_hwaddr (hwaddr) || !sv_valid_ipaddr (ip)
	  || !sv_valid_ipaddr (netmask))
	return -1;

#ifdef WDS_DEBUG
      fprintf (fp, "valid mac %s ip %s nm %s\n", hwaddr, ip, netmask);
#endif

      snprintf (cmd, 99, "ifconfig %s down", wds_dev);
      system (cmd);

      snprintf (cmd, 99, "ifconfig %s %s netmask %s up", wds_dev, ip,
		netmask);
      system (cmd);

      snprintf (wds_gw_var, 31, "%s_gw", wds_var);
      gw = nvram_safe_get (wds_gw_var);
      if (strcmp (gw, "0.0.0.0") != 0)
	{
	  get_network (ip, netmask);
	  route_del (wds_dev, 0, ip, gw, netmask);
	  route_add (wds_dev, 0, ip, gw, netmask);
	}

    }
  else
    {
#ifdef WDS_DEBUG
      fprintf (fp, "running down\n");
#endif
      snprintf (cmd, 99, "ifconfig %s down", wds_dev);
      system (cmd);

    }

#ifdef WDS_DEBUG
  fprintf (fp, "running ioctl\n");
  fclose (fp);
#endif

  close (s);

  return 0;
}


int
ishexit (char c)
{

  if (strchr ("01234567890abcdefABCDEF", c) != (char *) 0)
    return 1;

  return 0;
}


/* Example:
 * legal_hwaddr("00:11:22:33:44:aB"); return true;
 * legal_hwaddr("00:11:22:33:44:5"); return false;
 * legal_hwaddr("00:11:22:33:44:HH"); return false;
 */
int
sv_valid_hwaddr (char *value)
{
  unsigned int hwaddr[6];
  int tag = TRUE;
  int i, count;

  /* Check for bad, multicast, broadcast, or null address */
  for (i = 0, count = 0; *(value + i); i++)
    {
      if (*(value + i) == ':')
	{
	  if ((i + 1) % 3 != 0)
	    {
	      tag = FALSE;
	      break;
	    }
	  count++;
	}
      else if (ishexit (*(value + i)))	/* one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F */
	continue;
      else
	{
	  tag = FALSE;
	  break;
	}
    }

  if (!tag || i != 17 || count != 5)	/* must have 17's characters and 5's ':' */
    tag = FALSE;
  else if (sscanf (value, "%x:%x:%x:%x:%x:%x",
		   &hwaddr[0], &hwaddr[1], &hwaddr[2],
		   &hwaddr[3], &hwaddr[4], &hwaddr[5]) != 6)
    {
      tag = FALSE;
    }
  else
    tag = TRUE;
#ifdef WDS_DEBUG
  if (tag == FALSE)
    fprintf (fp, "failed valid_hwaddr\n");
#endif

  return tag;
}


int
sv_valid_range (char *value, int low, int high)
{
  if (!isdigit (value[0]) || atoi (value) < low || atoi (value) > high)
    return FALSE;
  else
    return TRUE;

}

int
sv_valid_statics (char *value)
{
  char ip[16] = { 0 }, mac[18] =
  {
  0}, hostname[255] =
  {
  0}, *p = value;

  if (NULL == value)
    return FALSE;

  do
    {
      while (isspace (*p++) && p - value < strlen (value))
	;

      if (p - value >= strlen (value))
	return FALSE;

      if (sscanf (p, "%15s%17s%254s", ip, mac, hostname) < 3)
	return FALSE;

      if (!sv_valid_ipaddr (ip) || !sv_valid_hwaddr (mac)
	  || strlen (hostname) <= 0)
	return FALSE;

    }
  while ((p = strpbrk (p, "\n\r")) && p - value < strlen (value));

  return TRUE;
}

/* Example:
 * legal_ipaddr("192.168.1.1"); return true;
 * legal_ipaddr("192.168.1.1111"); return false;
 */
int
sv_valid_ipaddr (char *value)
{
  struct in_addr ipaddr;
  int ip[4], ret = 0;

  ret = sscanf (value, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

  if (ret != 4 || !inet_aton (value, &ipaddr))
    return FALSE;
  else
    return TRUE;

}

// note - networl address returned in ipaddr
void
get_network (char *ipaddr, char *netmask)
{
  int ip[4], mask[4];

  if (!ipaddr || !netmask)
    return;

  sscanf (ipaddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
  sscanf (netmask, "%d.%d.%d.%d", &mask[0], &mask[1], &mask[2], &mask[3]);

  ip[0] &= mask[0];
  ip[1] &= mask[1];
  ip[2] &= mask[2];
  ip[3] &= mask[3];

  sprintf (ipaddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#ifdef WDS_DEBUG
  fprintf (fp, "get_network return %s\n", ipaddr);
#endif

}

// note - broadcast addr returned in ipaddr
void
get_broadcast (char *ipaddr, char *netmask)
{
  int ip[4], mask[4];

  if (!ipaddr || !netmask)
    return;

  sscanf (ipaddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
  sscanf (netmask, "%d.%d.%d.%d", &mask[0], &mask[1], &mask[2], &mask[3]);

  ip[0] = (ip[0] & mask[0]) | !mask[0];
  ip[1] = (ip[1] & mask[1]) | !mask[1];
  ip[2] = (ip[2] & mask[2]) | !mask[2];
  ip[3] = (ip[3] & mask[3]) | !mask[3];

  sprintf (ipaddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#ifdef WDS_DEBUG
  fprintf (fp, "get_broadcast return %s\n", value);
#endif

}

/* note: copied from Broadcom code and put in shared via this file */

int
route_manip (int cmd, char *name, int metric, char *dst, char *gateway,
	     char *genmask)
{
  int s;
  struct rtentry rt;

  //dprintf("cmd=[%d] name=[%s] ipaddr=[%s] netmask=[%s] gateway=[%s] metric=[%d]\n",cmd,name,dst,genmask,gateway,metric);

  /* Open a raw socket to the kernel */
  if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    goto err;

  /* Fill in rtentry */
  memset (&rt, 0, sizeof (rt));
  if (dst)
    inet_aton (dst, &sin_addr (&rt.rt_dst));
  if (gateway)
    inet_aton (gateway, &sin_addr (&rt.rt_gateway));
  if (genmask)
    inet_aton (genmask, &sin_addr (&rt.rt_genmask));
  rt.rt_metric = metric;
  rt.rt_flags = RTF_UP;
  if (sin_addr (&rt.rt_gateway).s_addr)
    rt.rt_flags |= RTF_GATEWAY;
  if (sin_addr (&rt.rt_genmask).s_addr == INADDR_BROADCAST)
    rt.rt_flags |= RTF_HOST;
  rt.rt_dev = name;

  /* Force address family to AF_INET */
  rt.rt_dst.sa_family = AF_INET;
  rt.rt_gateway.sa_family = AF_INET;
  rt.rt_genmask.sa_family = AF_INET;

  if (ioctl (s, cmd, &rt) < 0)
    goto err;

  close (s);
  return 0;

err:
  close (s);
  perror (name);
  return errno;
}

int
route_add (char *name, int metric, char *dst, char *gateway, char *genmask)
{
  return route_manip (SIOCADDRT, name, metric, dst, gateway, genmask);
}

int
route_del (char *name, int metric, char *dst, char *gateway, char *genmask)
{
  return route_manip (SIOCDELRT, name, metric, dst, gateway, genmask);
}
