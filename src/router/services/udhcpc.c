/*
 * udhcpc scripts
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: udhcpc.c,v 1.4 2005/11/21 15:03:05 seg Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <utils.h>

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

static int
expires (unsigned int in)
{
  struct sysinfo info;
  FILE *fp;

  sysinfo (&info);

  /* Save uptime ranther than system time, because the system time may change */
  if (!(fp = fopen ("/tmp/udhcpc.expires", "w")))
    {
      perror ("/tmp/udhcpd.expires");
      return errno;
    }
  fprintf (fp, "%d", (unsigned int) info.uptime + in);
  fclose (fp);
  return 0;
}

/* 
 * deconfig: This argument is used when udhcpc starts, and when a
 * leases is lost. The script should put the interface in an up, but
 * deconfigured state.
*/
static int
deconfig (void)
{
  char *wan_ifname = safe_getenv ("interface");

  ifconfig (wan_ifname, IFUP, "0.0.0.0", NULL);
  expires (0);

  nvram_set ("wan_ipaddr", "0.0.0.0");
  nvram_set ("wan_netmask", "0.0.0.0");
  nvram_set ("wan_gateway", "0.0.0.0");
  nvram_set ("wan_get_dns", "");
  //nvram_set("wan_wins","0.0.0.0");      // Don't care for linksys spec
  nvram_set ("wan_lease", "0");

  unlink ("/tmp/get_lease_time");
  unlink ("/tmp/lease_time");

  cprintf ("done\n");
  return 0;
}

//==================================================================
static int
update_value (void)
{

  char *value;
  int changed = 0;

  if ((value = getenv ("ip")))
    {
      chomp (value);
      if (nvram_invmatch ("wan_ipaddr", value))
	{
	  nvram_set ("wan_ipaddr", value);
	  changed++;
	}
    }
  if ((value = getenv ("subnet")))
    {
      chomp (value);
      if (nvram_invmatch ("wan_netmask", value))
	{
	  nvram_set ("wan_netmask", value);
	  changed++;
	}
    }
  if ((value = getenv ("router")))
    {
      chomp (value);
      if (nvram_invmatch ("wan_gateway", value))
	{
	  nvram_set ("wan_gateway", value);
	  changed++;
	}
    }
  if ((value = getenv ("dns")))
    {
      chomp (value);
      if (nvram_invmatch ("wan_get_dns", value))
	{
	  nvram_set ("wan_get_dns", value);
	  changed++;
	}
    }
  /*
     if ((value = getenv("wins")))
     nvram_set("wan_wins", value);
     if ((value = getenv("hostname")))
     sethostname(value, strlen(value) + 1);
   */
  if ((value = getenv ("domain")))
    {
      chomp (value);
      if (nvram_invmatch ("wan_get_domain", value))
	{
	  nvram_set ("wan_get_domain", value);
	  changed++;
	}
    }
  if ((value = getenv ("lease")))
    {
      chomp (value);
      if (nvram_invmatch ("wan_lease", value))
	{
	  nvram_set ("wan_lease", value);
	  changed++;
	}
      expires (atoi (value));
    }

  if (changed)
    {
      set_host_domain_name ();
      stop_udhcpd ();
      start_udhcpd ();
    }
  return 0;
}

//=================================================================

/*
 * bound: This argument is used when udhcpc moves from an unbound, to
 * a bound state. All of the paramaters are set in enviromental
 * variables, The script should configure the interface, and set any
 * other relavent parameters (default gateway, dns server, etc).
*/
static int
bound (void)
{
  char *wan_ifname = safe_getenv ("interface");
  char *value;
  char temp_wan_ipaddr[16], temp_wan_netmask[16];
  int changed = 0;

  if ((value = getenv ("ip")))
    {
      chomp (value);
      if (nvram_match ("wan_proto", "pptp")
	  && nvram_match ("pptp_usedhcp", "1"))
	strcpy (temp_wan_ipaddr, value);
      else
	{
	  if (nvram_invmatch ("wan_ipaddr", value))
	    changed = 1;
	}
      nvram_set ("wan_ipaddr", value);
    }
  if ((value = getenv ("subnet")))
    {
      chomp (value);
      if (nvram_match ("wan_proto", "pptp")
	  && nvram_match ("pptp_usedhcp", "1"))
	strcpy (temp_wan_netmask, value);
      else
	{
	  if (nvram_invmatch ("wan_netmask", value))
	    changed = 1;
	  nvram_set ("wan_netmask", value);
	}
    }
  if ((value = getenv ("router")))
    {
      chomp (value);

      if (nvram_invmatch ("wan_gateway", value))
	changed = 1;
      nvram_set ("wan_gateway", value);
    }
  if ((value = getenv ("dns")))
    {
      chomp (value);
//      if (nvram_invmatch("wan_get_dns",value))
//          changed=1;      
      nvram_set ("wan_get_dns", value);
    }
  /*      Don't care for linksys spec
     if ((value = getenv("wins")))
     nvram_set("wan_wins", value);
     if ((value = getenv("hostname")))
     sethostname(value, strlen(value) + 1);
   */
  if ((value = getenv ("domain")))
    {
      chomp (value);
      if (nvram_invmatch ("wan_get_domain", value))
	changed = 1;
      nvram_set ("wan_get_domain", value);	// HeartBeat need to use
    }
  if ((value = getenv ("lease")))
    {
      chomp (value);
      nvram_set ("wan_lease", value);
      expires (atoi (value));
    }
  if (!changed)
    {
      cprintf ("interface havent changed, do nothing\n");
      return 0;
    }
  stop_firewall ();
  cprintf ("configure to IF[%s] , IP[%s], MASK[%s]\n", wan_ifname,
	   nvram_safe_get ("wan_ipaddr"), nvram_safe_get ("wan_netmask"));


  if (nvram_match ("wan_proto", "pptp") && nvram_match ("pptp_usedhcp", "1"))
    ifconfig (wan_ifname, IFUP, temp_wan_ipaddr, temp_wan_netmask);
  else
    ifconfig (wan_ifname, IFUP, nvram_safe_get ("wan_ipaddr"),
	      nvram_safe_get ("wan_netmask"));

  /* We only want to exec bellow functions after dhcp get ip if the wan_proto is heartbeat */
#ifdef HAVE_HEARTBEAT
  if (nvram_match ("wan_proto", "heartbeat"))
    {
      int i = 0;
      /* Delete all default routes */
      while (route_del (wan_ifname, 0, NULL, NULL, NULL) == 0 || i++ < 10);

      /* Set default route to gateway if specified */
      route_add (wan_ifname, 0, "0.0.0.0", nvram_safe_get ("wan_gateway"),
		 "0.0.0.0");

      /* save dns to resolv.conf */
      dns_to_resolv ();
      stop_udhcpd ();
      start_udhcpd ();
      start_firewall ();
      start_wshaper ();
      start_heartbeat_boot ();
    }
#else
  if (0)
    {
      //nothing
    }
#endif
#ifdef HAVE_PPTP
  else if (nvram_match ("wan_proto", "pptp")
	   && nvram_match ("pptp_usedhcp", "1"))
    {
      int i = 0;
      /* Delete all default routes */
      while (route_del (wan_ifname, 0, NULL, NULL, NULL) == 0 || i++ < 10);

      // Add the route to the PPTP server on the wan interface for pptp client to reach it
      route_add (wan_ifname, 0, nvram_safe_get ("pptp_server_ip"), NULL,
		 "255.255.255.255");
    }
#endif
#ifdef HAVE_L2TP
  else if (nvram_match ("wan_proto", "l2tp"))
    {
      int i = 0;
      /* Delete all default routes */
      while (route_del (wan_ifname, 0, NULL, NULL, NULL) == 0 || i++ < 10);

      /* Set default route to gateway if specified */
      route_add (wan_ifname, 0, "0.0.0.0", nvram_safe_get ("wan_gateway"),
		 "0.0.0.0");

      /* Backup the default gateway. It should be used if L2TP connection is broken */
      nvram_set ("wan_gateway_buf", nvram_get ("wan_gateway"));

      /* clear dns from the resolv.conf */
      nvram_set ("wan_get_dns", "");
      dns_to_resolv ();
      start_firewall ();
      start_l2tp_boot ();
    }
#endif
  else
    {
      cprintf ("start wan done\n");
      start_wan_done (wan_ifname);
    }

  cprintf ("done\n");
  return 0;
}

/*
 * renew: This argument is used when a DHCP lease is renewed. All of
 * the paramaters are set in enviromental variables. This argument is
 * used when the interface is already configured, so the IP address,
 * will not change, however, the other DHCP paramaters, such as the
 * default gateway, subnet mask, and dns server may change.
 */
static int
renew (void)
{
  bound ();

  cprintf ("done\n");
  return 0;
}

int
udhcpc_main (int argc, char **argv)
{
  if (check_action () != ACT_IDLE)
    return -1;

  if (!argv[1])
    return EINVAL;
  else if (strstr (argv[1], "deconfig"))
    return deconfig ();
  else if (strstr (argv[1], "bound"))
    return bound ();
  else if (strstr (argv[1], "renew"))
    return renew ();
  else if (strstr (argv[1], "update"))
    return update_value ();
  else
    return EINVAL;
}
