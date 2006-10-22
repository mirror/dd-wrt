
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <broadcom.h>
#include <cymac.h>

int clone_wan_mac;
int lan_ip_changed;

void
ej_show_index_setting (int eid, webs_t wp, int argc, char_t ** argv)
{
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

  get_merge_ipaddr (wp, "lan_netmask", lan_netmask);
  get_merge_ipaddr (wp, v->name, lan_ipaddr);

  if (!valid_ipaddr (wp, lan_ipaddr, v))
    return;


  //if(!valid_choice(wp, lan_netmask, &which[0]))
  //      return;

  if (strcmp (nvram_safe_get ("lan_ipaddr"), lan_ipaddr))
    {
      unlink ("/tmp/udhcpd.leases");
      unlink ("/jffs/udhcpd.leases");
      unlink ("/tmp/dnsmasq.leases");
      unlink ("/jffs/dnsmasq.leases");
    }
  if (strcmp (nvram_safe_get ("lan_netmask"), lan_netmask))
    {
      unlink ("/tmp/udhcpd.leases");
      unlink ("/jffs/udhcpd.leases");
      unlink ("/tmp/dnsmasq.leases");
      unlink ("/jffs/dnsmasq.leases");
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


  char *pppoever = websGetVar (wp, "pppoe_ver", NULL);
  if (pppoever)
    nvram_set ("pppoe_ver", pppoever);

  get_merge_ipaddr (wp, "wan_ipaddr", wan_ipaddr);
  get_merge_ipaddr (wp, "wan_netmask", wan_netmask);
  if (!strcmp (wan_proto, "pptp"))
    get_merge_ipaddr (wp, "pptp_server_ip", wan_gateway);
  else
    get_merge_ipaddr (wp, "wan_gateway", wan_gateway);

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
#ifdef HAVE_PORTSETUP
void
validate_portsetup (webs_t wp, char *value, struct variable *v)
{
char *next;
char var[64];
char eths[256];
memset(eths,0,256);
getinterfacelist("eth",eths);
  foreach (var, eths, next)
  {
  char val[64];
  sprintf(val,"%s_bridged",var);
  char *bridged = websGetVar (wp, val, NULL);
  if (bridged)
  nvram_set (val, bridged);
  if (bridged && strcmp(bridged,"0")==0)
    {
  sprintf(val,"%s_ipaddr",var);
  char ipaddr[64];
  if (get_merge_ipaddr (wp, val, ipaddr))
    nvram_set (val, ipaddr);
  sprintf(val,"%s_netmask",var);
  char netmask[64];
  if (get_merge_ipaddr (wp, val, netmask))
    nvram_set (val, netmask);
    }
  }
}
#endif
void
ej_get_wl_max_channel (int eid, webs_t wp, int argc, char_t ** argv)
{

  websWrite (wp, "%s", WL_MAX_CHANNEL);
}

void
ej_get_wl_domain (int eid, webs_t wp, int argc, char_t ** argv)
{

#if COUNTRY == EUROPE
  websWrite (wp, "ETSI");
#elif COUNTRY == JAPAN
  websWrite (wp, "JP");
#else
  websWrite (wp, "US");
#endif
}

int
clone_mac (webs_t wp)
{
  int ret = 0;

  clone_wan_mac = 1;

  return ret;
}

void
ej_get_clone_mac (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *c;
  int mac, which;
  int dofree = 0;

  if (ejArgs (argc, argv, "%d", &which) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
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
      websWrite (wp, "%02X", mac);
      if (dofree)
	free (c);
    }
  else
    websWrite (wp, "00");
}

int
macclone_onload (webs_t wp, char *arg)
{

  if (clone_wan_mac)
    websWrite (wp, arg);

  return 0;
}
