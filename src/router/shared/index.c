
/*
 *********************************************************
 *   Copyright 2003, CyberTAN  Inc.  All Rights Reserved *
 *********************************************************

 This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
 the contents of this file may not be disclosed to third parties,
 copied or duplicated in any form without the prior written
 permission of CyberTAN Inc.

 This software should be used as a reference only, and it not
 intended for production use!


 THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
 KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
 SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <broadcom.h>
#include <cymac.h>

int clone_wan_mac;
int lan_ip_changed;

int
ej_show_index_setting (int eid, webs_t wp, int argc, char_t ** argv)
{
  int ret = 0;
  char *type;

  type = GOZILA_GET ("wan_proto");

  //cprintf("change to %s mode\n",type);

  if (!strcmp (type, "static"))
    do_ej ("index_static.asp", wp);
  else if (!strcmp (type, "pppoe"))
    do_ej ("index_pppoe.asp", wp);
  else if (!strcmp (type, "pptp"))
    do_ej ("index_pptp.asp", wp);
  else if (!strcmp (type, "l2tp"))
    do_ej ("index_l2tp.asp", wp);
  else if (!strcmp (type, "heartbeat"))
    do_ej ("index_heartbeat.asp", wp);

  return ret;
}

void
validate_lan_ipaddr (webs_t wp, char *value, struct variable *v)
{
  char lan_ipaddr[20], lan_netmask[20];
  //struct variable lan_variables[] = {
  //      { longname: "LAN Subnet Mask", argv: ARGV("255.255.255.0","255.255.255.128","255.255.255.192","255.255.255.224","255.255.255.240","255.255.255.248","255.255.255.252") },
  //}, *which;
  //which = &lan_variables[0];

  //lan_netmask = websGetVar(wp, "lan_netmask", NULL);
  //if(!lan_netmask)      return;

  get_merge_ipaddr ("lan_netmask", lan_netmask);
  get_merge_ipaddr (v->name, lan_ipaddr);

  if (!valid_ipaddr (wp, lan_ipaddr, v))
    return;


  //if(!valid_choice(wp, lan_netmask, &which[0]))
  //      return;

  if (strcmp (nvram_safe_get ("lan_ipaddr"), lan_ipaddr))
    {
      unlink ("/tmp/udhcpd.leases");
      unlink ("/jffs/udhcpd.leases");
    }
  if (strcmp (nvram_safe_get ("lan_netmask"), lan_netmask))
    {
      unlink ("/tmp/udhcpd.leases");
      unlink ("/jffs/udhcpd.leases");
    }

  if (strcmp (lan_ipaddr, nvram_safe_get ("lan_ipaddr")) ||
      strcmp (lan_netmask, nvram_safe_get ("lan_netmask")))
    lan_ip_changed = 1;
  else
    lan_ip_changed = 0;

  nvram_set (v->name, lan_ipaddr);
  nvram_set ("lan_netmask", lan_netmask);

}

void
validate_wan_ipaddr (webs_t wp, char *value, struct variable *v)
{
  char wan_ipaddr[20], wan_netmask[20], wan_gateway[20];
  char *wan_proto = websGetVar (wp, "wan_proto", NULL);
  char *pptp_use_dhcp = websGetVar (wp, "pptp_use_dhcp", NULL);

  int pptp_skip_check = FALSE;

  struct variable wan_variables[] = {
  {longname:"WAN IP Address", NULL},
  {longname:"WAN Subnet Mask", NULL},
  {longname: "WAN Gateway", argv:ARGV ("wan_ipaddr", "wan_netmask")},
  }, *which;

  which = &wan_variables[0];


  char *pppoever = websGetVar(wp, "pppoe_ver", NULL);
  if (pppoever)
    nvram_set("pppoe_ver",pppoever);
    
  get_merge_ipaddr ("wan_ipaddr", wan_ipaddr);
  get_merge_ipaddr ("wan_netmask", wan_netmask);
  if (!strcmp (wan_proto, "pptp"))
    get_merge_ipaddr ("pptp_server_ip", wan_gateway);
  else
    get_merge_ipaddr ("wan_gateway", wan_gateway);

  if (!strcmp (wan_proto, "pptp") && !strcmp ("0.0.0.0", wan_ipaddr))
    {				// Sveasoft: allow 0.0.0.0 for pptp IP addr
      pptp_skip_check = TRUE;
      nvram_set ("pptp_usedhcp", "1");
    }
  else
    nvram_set ("pptp_usedhcp", "0");


  if (FALSE == pptp_skip_check && !valid_ipaddr (wp, wan_ipaddr, &which[0]))
    return;

  nvram_set ("wan_ipaddr_buf", nvram_safe_get ("wan_ipaddr"));
  nvram_set ("wan_ipaddr", wan_ipaddr);

  if (FALSE == pptp_skip_check && !valid_netmask (wp, wan_netmask, &which[1]))
    return;

  nvram_set ("wan_netmask", wan_netmask);

  if (!valid_ipaddr (wp, wan_gateway, &which[2]))
    return;

  if (!strcmp (wan_proto, "pptp"))
    nvram_set ("pptp_server_ip", wan_gateway);
  else
    nvram_set ("wan_gateway", wan_gateway);

  if (!strcmp (wan_proto, "pptp") && !strcmp (pptp_use_dhcp, "1"))
    {
      if (!legal_ipaddr (wan_gateway))
	return;
      nvram_set ("pptp_server_ip", wan_gateway);
      return;
    }



}

int
ej_get_wl_max_channel (int eid, webs_t wp, int argc, char_t ** argv)
{
  int ret = 0;

  ret = websWrite (wp, "%s", WL_MAX_CHANNEL);

  return ret;
}

int
ej_get_wl_domain (int eid, webs_t wp, int argc, char_t ** argv)
{
  int ret = 0;

#if COUNTRY == EUROPE
  ret += websWrite (wp, "ETSI");
#elif COUNTRY == JAPAN
  ret += websWrite (wp, "JP");
#else
  ret += websWrite (wp, "US");
#endif

  return ret;
}

int
clone_mac (webs_t wp)
{
  int ret = 0;

  clone_wan_mac = 1;

  return ret;
}

int
ej_get_clone_mac (int eid, webs_t wp, int argc, char_t ** argv)
{
  int ret = 0;
  char *c;
  int mac, which;
  int dofree = 0;

  if (ejArgs (argc, argv, "%d", &which) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  if (clone_wan_mac)
    c = nvram_safe_get ("http_client_mac");
  else
    {
      if (nvram_match ("def_hwaddr", "00:00:00:00:00:00"))
	{
	  c = strdup (nvram_safe_get ("et0macaddr"));
	  if (c)
	    {
	      MAC_ADD (c);
	      dofree = 1;
	    }
	}
      else
	c = nvram_safe_get ("def_hwaddr");
    }

  if (c)
    {
      mac = get_single_mac (c, which);
      ret += websWrite (wp, "%02X", mac);
      if (dofree)
	free (c);
    }
  else
    ret += websWrite (wp, "00");
  return ret;
}

int
macclone_onload (webs_t wp, char *arg)
{
  int ret = 0;

  if (clone_wan_mac)
    ret += websWrite (wp, arg);

  return ret;
}
