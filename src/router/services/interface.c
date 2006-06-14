/*
 * Linux network interface code
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: interface.c,v 1.3 2005/11/21 15:03:05 seg Exp $
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
#include <rc.h>

#include <cy_conf.h>
#include <cymac.h>

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

int
ifconfig (char *name, int flags, char *addr, char *netmask)
{
  int s;
  struct ifreq ifr;
  struct in_addr in_addr, in_netmask, in_broadaddr;

  cprintf ("ifconfig(): name=[%s] flags=[%s] addr=[%s] netmask=[%s]\n", name,
	   flags == IFUP ? "IFUP" : "0", addr, netmask);

  /* Open a raw socket to the kernel */
  if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    goto err;

  /* Set interface name */
  strncpy (ifr.ifr_name, name, IFNAMSIZ);

  /* Set interface flags */
  ifr.ifr_flags = flags;
  if (ioctl (s, SIOCSIFFLAGS, &ifr) < 0)
    goto err;

  /* Set IP address */
  if (addr)
    {
      inet_aton (addr, &in_addr);
      sin_addr (&ifr.ifr_addr).s_addr = in_addr.s_addr;
      ifr.ifr_addr.sa_family = AF_INET;
      if (ioctl (s, SIOCSIFADDR, &ifr) < 0)
	goto err;
    }

  /* Set IP netmask and broadcast */
  if (addr && netmask)
    {
      inet_aton (netmask, &in_netmask);
      sin_addr (&ifr.ifr_netmask).s_addr = in_netmask.s_addr;
      ifr.ifr_netmask.sa_family = AF_INET;
      if (ioctl (s, SIOCSIFNETMASK, &ifr) < 0)
	goto err;

      in_broadaddr.s_addr =
	(in_addr.s_addr & in_netmask.s_addr) | ~in_netmask.s_addr;
      sin_addr (&ifr.ifr_broadaddr).s_addr = in_broadaddr.s_addr;
      ifr.ifr_broadaddr.sa_family = AF_INET;
      if (ioctl (s, SIOCSIFBRDADDR, &ifr) < 0)
	goto err;
    }

  close (s);
  return 0;

err:
  close (s);
  perror (name);
  return errno;
}

static int
route_manip (int cmd, char *name, int metric, char *dst, char *gateway,
	     char *genmask)
{
  int s;
  struct rtentry rt;

  cprintf
    ("route(): cmd=[%s] name=[%s] ipaddr=[%s] netmask=[%s] gateway=[%s] metric=[%d]\n",
     cmd == SIOCADDRT ? "ADD" : "DEL", name, dst, genmask, gateway, metric);

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


#define MAX_VLAN_GROUPS	16
#define MAX_DEV_IFINDEX	16

/* configure vlan interface(s) based on nvram settings */
int
start_config_vlan (void)
{
  int s;
  struct ifreq ifr;
  int i, j;
  char ea[ETHER_ADDR_LEN];
  char tmp[200];

// configure ports

  system ("echo 1 > /proc/switch/eth0/reset");
  for (i = 0; i < 16; i++)
    {
      char vlanb[16];
      sprintf (vlanb, "vlan%dports", i);
      if (nvram_get (vlanb) == NULL || nvram_match (vlanb, ""))
	continue;
      sprintf (tmp, "echo %s > /proc/switch/eth0/vlan/%d/ports",
	       nvram_safe_get (vlanb), i);
      system (tmp);
    }


  /* set vlan i/f name to style "vlan<ID>" */

  eval ("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");

  /* create vlan interfaces */
  if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    return errno;

  for (i = 0; i < MAX_VLAN_GROUPS; i++)
    {
      char nvvar_name[16];
      char vlan_id[16];
      char *hwname, *hwaddr;
      snprintf (nvvar_name, sizeof (nvvar_name), "vlan%dhwname", i);
      if (!(hwname = nvram_get (nvvar_name)))
	continue;
      snprintf (nvvar_name, sizeof (nvvar_name), "%smacaddr", hwname);
      if (!(hwaddr = nvram_get (nvvar_name)))
	continue;
      ether_atoe (hwaddr, ea);
      for (j = 1; j <= MAX_DEV_IFINDEX; j++)
	{
	  ifr.ifr_ifindex = j;
	  if (ioctl (s, SIOCGIFNAME, &ifr))
	    continue;
	  if (ioctl (s, SIOCGIFHWADDR, &ifr))
	    continue;
	  if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
	    continue;
	  if (!bcmp (ifr.ifr_hwaddr.sa_data, ea, ETHER_ADDR_LEN))
	    break;
	}
      if (j > MAX_DEV_IFINDEX)
	continue;
      if (ioctl (s, SIOCGIFFLAGS, &ifr))
	continue;
      if (!(ifr.ifr_flags & IFF_UP))
	ifconfig (ifr.ifr_name, IFUP, 0, 0);
      snprintf (vlan_id, sizeof (vlan_id), "%d", i);
      eval ("vconfig", "add", ifr.ifr_name, vlan_id);
    }

  close (s);

  return 0;
}

/* begin lonewolf mods */

int
start_setup_vlans (void)
{
#ifndef HAVE_RB500
  /* VLAN #16 is just a convieniant way of storing tagging info.  There is no VLAN #16 */

  if (!nvram_get ("port5vlans") || nvram_match ("vlans", "0"))
    return 0;			// for some reason VLANs are not set up, and we don't want to disable everything!

  int ports[21][6], i, j, ret = 0, tmp, workaround = 0, found;
  char *vlans, *next, vlan[4], buff[70], buff2[16];
  FILE *fp;

  unsigned char mac[20];;
  struct ifreq ifr;
  int s;

  s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
  strcpy (mac, nvram_safe_get ("et0macaddr"));

//  if (nvram_match ("trunking", "1"))
//    system ("echo 1 > /proc/sys/dev/adm6996/trunk");
//  else
//    system ("echo 0 > /proc/sys/dev/adm6996/trunk");

  if (nvram_match ("boardtype", "bcm94710dev") || nvram_match ("xover", "1"))
    workaround = 1;
  else
    workaround = 0;

  for (i = 0; i < 21; i++)
    for (j = 0; j < 6; j++)
      ports[i][j] = 0;

  for (i = 0; i < 6; i++)
    {
      snprintf (buff, 31, "port%dvlans", i);
      vlans = nvram_safe_get (buff);

      if (vlans)
	{
	  foreach (vlan, vlans, next)
	  {
	    tmp = atoi (vlan);
	    ports[tmp][i] = 1;
	  }
	}
    }

  for (i = 0; i < 16; i++)
    {
      snprintf (buff, 69, "/proc/sys/dev/adm6996/vlan-groups/%d", i);

      if ((fp = fopen (buff, "r+")))
	{
	  snprintf (buff, 69, "%d %d %d %d %d %d", ports[i][0], ports[i][1],
		    ports[i][2], ports[i][3], ports[i][4], ports[i][5]);
	  fputs (buff, fp);
	  fclose (fp);
	}
      else
	{
	  perror (buff);
	}

      if (ports[i][5] == 1)
	{
	  snprintf (buff, 9, "%d", i);
	  eval ("vconfig", "add", "eth0", buff);

	  snprintf (buff, 9, "vlan%d", i);
	  ifconfig (buff, 0, NULL, NULL);
	  snprintf (buff, 9, "vlan%d", i);
	  ifconfig (buff, 0, NULL, NULL);

//                      if(i > 1)
	  {
	    snprintf (buff, 9, "vlan%d", i);
	    strncpy (ifr.ifr_name, buff, IFNAMSIZ);

	    ether_atoe (mac, ifr.ifr_hwaddr.sa_data);
	    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	    if (ioctl (s, SIOCSIFHWADDR, &ifr) == -1)
	      {
		if (errno == EBUSY)
		  {
		    snprintf (buff, 69,
			      "Write vlan%d MAC failed, already in use\n", i);
		    cprintf (buff);
		  }
		else
		  {
		    snprintf (buff, 69,
			      "Write vlan%d MAC FAILED! (Error %d)\n", i,
			      errno);
		    perror (buff);
		  }
	      }
	    else
	      {
		snprintf (buff, 69, "Wrote vlan%d MAC successfully\n", i);
		cprintf (buff);
	      }
	  }
	}
      MAC_ADD (mac);
      if (i == 1)
	MAC_ADD (mac);		// base MAC +2 is taken by wireless...
    }

  for (i = 0; i < 5; i++)
    {
      found = 0;
      for (j = 0; j < 21; j++)
	{
	  if (ports[j][i] == 1 && j < 16 && !found)
	    {
	      snprintf (buff, 69,
			"/proc/sys/dev/adm6996/port%d/vlan-group-mask", i);
	      if ((fp = fopen (buff, "r+")))
		{
		  snprintf (buff, 16, "%d", j);
		  fputs (buff, fp);
		  fclose (fp);
		}
	      else
		{
		  perror (buff);
		}
	      if (workaround)
		ports[20][i] = 1;
	      found = 1;
	    }

	  tmp = 0;

	  switch (j)
	    {
	    case 16:
	      if (ports[16][i] == 1)
		{
		  snprintf (buff, 69, "/proc/sys/dev/adm6996/port%d/tagging",
			    i);
		  tmp = 1;
		  strcpy (buff2, "1");
		}
	      break;
	    case 17:
	      if (ports[17][i] == 1)
		{
		  snprintf (buff, 69,
			    "/proc/sys/dev/adm6996/port%d/auto-negotiating",
			    i);
		  tmp = 1;
		  strcpy (buff2, "0");
		}
	      break;
	    case 18:
	      if (ports[18][i] == 1)
		{
		  snprintf (buff, 69, "/proc/sys/dev/adm6996/port%d/speed",
			    i);
		  tmp = 1;
		  strcpy (buff2, "10");
		}
	      break;
	    case 19:
	      if (ports[19][i] == 1)
		{
		  snprintf (buff, 69, "/proc/sys/dev/adm6996/port%d/duplex",
			    i);
		  tmp = 1;
		  strcpy (buff2, "0");
		}
	      break;
	    case 20:
	      if (ports[20][i] == 1)
		{
		  snprintf (buff, 69,
			    "/proc/sys/dev/adm6996/port%d/crossover", i);
		  tmp = 1;
		  strcpy (buff2, "1");
		}
	      break;
	    default:
	      tmp = 0;
	    }

	  if (tmp)
	    {
	      if ((fp = fopen (buff, "r+")))
		{
		  fputs (buff2, fp);
		  fclose (fp);
		}
	      else
		{
		  perror (buff);
		}
	      if (workaround)
		ports[20][i] = 1;
	    }
	}
    }

  return ret;
#else
  return 0;
#endif
}

int
flush_interfaces (void)
{
  char all_ifnames[256] = { 0 }, *c, *next, buff[32], buff2[32];

  if (wl_probe ("eth2"))
    snprintf (all_ifnames, 255, "%s %s %s", "eth0",
	      nvram_safe_get ("lan_ifnames"), nvram_safe_get ("wan_ifnames"));
  else
    snprintf (all_ifnames, 255, "%s %s %s %s", "eth0", "eth1",
	      nvram_safe_get ("lan_ifnames"), nvram_safe_get ("wan_ifnames"));

  //strcpy(all_ifnames, "eth0 ");
  //strcpy(all_ifnames, "eth0 eth1 "); //James, note: eth1 is the wireless interface on V2/GS's. I think we need a check here.
  //strcat(all_ifnames, nvram_safe_get("lan_ifnames"));
  //strcat(all_ifnames, " ");
  //strcat(all_ifnames, nvram_safe_get("wan_ifnames"));

  c = nvram_safe_get ("port5vlans");
  if (c)
    {
      foreach (buff, c, next)
      {
	if (atoi (buff) > 15)
	  continue;
	snprintf (buff2, sizeof (buff2), " vlan%s", buff);
	strcat (all_ifnames, buff2);
      }
    }

  foreach (buff, all_ifnames, next)
  {
    if (strcmp (buff, "br0") == 0)
      continue;
    ifconfig(buff,0,0,0);
    
//    eval ("ifconfig", buff, "down");
    eval ("ip", "addr", "flush", "dev", buff);
    ifconfig(buff,IFUP,0,0);
    
//    eval ("ifconfig", buff, "up");
  }

  return 0;
}

/*
int
start_nonstd_interfaces (void)
{
  char br_ifnames[256], all_ifnames[256], ifnames[256], buff[256], buff2[256],
    buff3[32];
  char *next, *next2;
  int i, j, k, cidr;

  strcpy (br_ifnames, nvram_safe_get ("lan_ifnames"));
  if (strlen (br_ifnames) > 0)
    strcat (br_ifnames, " ");

  strcat (br_ifnames, nvram_safe_get ("wan_ifname"));
  strcat (br_ifnames, " ");
  strcat (br_ifnames, nvram_safe_get ("wan_ifnames"));

  strcpy (all_ifnames, "eth1");
  strcpy (ifnames, nvram_safe_get ("port5vlans"));
  if (strlen (ifnames) < 1)
    strcat (all_ifnames, " eth0");
  foreach (buff, ifnames, next)
  {
    if (atoi (buff) > 15)
      continue;
    snprintf (buff2, 63, " vlan%s", buff);
    strcat (all_ifnames, buff2);
  }

  strcpy (ifnames, "");

  foreach (buff, all_ifnames, next)
  {
    i = 1;

    foreach (buff2, br_ifnames, next2)
    {
      if (strcmp (buff, buff2) == 0)
	i = 0;
    }

    if (i)
      {
	if (strlen (ifnames) > 1)
	  strcat (ifnames, " ");

	strcat (ifnames, buff);
      }
  }

  i = 1;

  foreach (buff2, ifnames, next)
  {
    snprintf (buff, sizeof (buff), "%s_ipaddr", buff2);
    strcpy (all_ifnames, nvram_safe_get (buff));
    snprintf (buff, sizeof (buff), "%s_netmask", buff2);
    strcpy (br_ifnames, nvram_safe_get (buff));

    next2 = strtok (br_ifnames, ".");
    j = 0;
    cidr = 0;
    while ((next2 != NULL) && (j < 4))
      {
	for (k = 0; k < 8; k++)
	  cidr += ((atoi (next2) << j) & 255) >> 7;

	next2 = strtok (NULL, ".");
	j++;
      }

    snprintf (buff, sizeof (buff), "%s/%d", all_ifnames, cidr);

    eval ("ifconfig", buff2, "up");
    eval ("ip", "addr", "add", buff, "broadcast", "+", "dev", buff2);

    snprintf (buff, sizeof (buff), "%s_alias", buff2);
    strcpy (all_ifnames, nvram_safe_get (buff));
    foreach (buff3, all_ifnames, next2)
    {
      snprintf (buff, sizeof (buff), "%s:%d", buff2, i);
      eval ("ip", "addr", "add", buff3, "broadcast", "+", "label", buff,
	    "dev", buff2);
      i++;
    }
  }
  return 0;
}
*/
