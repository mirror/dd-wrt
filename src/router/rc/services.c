


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>		/* AhMan  March 18 2005 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>
#include <net/route.h>		/* AhMan  March 18 2005 */
#include <sys/types.h>
#include <signal.h>
#include <dlfcn.h>

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>
#include <nvparse.h>


#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 251
#define TXPWR_DEFAULT 28


#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

/* AhMan  March 18 2005 */
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

/* AhMan  March 18 2005 */
void start_tmp_ppp (int num);

void
del_routes (char *route)
{
  char word[80], *tmp;
  char *ipaddr, *netmask, *gateway, *metric, *ifname;

  foreach (word, route, tmp)
  {
    netmask = word;
    ipaddr = strsep (&netmask, ":");
    if (!ipaddr || !netmask)
      continue;
    gateway = netmask;
    netmask = strsep (&gateway, ":");
    if (!netmask || !gateway)
      continue;
    metric = gateway;
    gateway = strsep (&metric, ":");
    if (!gateway || !metric)
      continue;
    ifname = metric;
    metric = strsep (&ifname, ":");
    if (!metric || !ifname)
      continue;

    if (!strcmp (ipaddr, "0.0.0.0"))
      eval ("route", "del", "default", "gw", gateway);

    route_del (ifname, atoi (metric) + 1, ipaddr, gateway, netmask);
  }
}

int
start_services (void)
{
  void *handle = NULL;
  nvram_set ("qos_done", "0");
#ifdef HAVE_CPUTEMP
  handle = start_service_nofree ("hwmon", handle);
#endif
#ifdef HAVE_TELNET
#ifdef HAVE_REGISTER
  if (isregistered ())
#endif
    handle = start_service_nofree ("telnetd", handle);
#endif
  handle = start_service_nofree ("syslog", handle);
#ifdef HAVE_TFTP
  handle = start_service_nofree ("tftpd", handle);
#endif
  handle = start_service_nofree ("httpd", handle);
  handle = start_service_nofree ("udhcpd", handle);
  handle = start_service_nofree ("dnsmasq", handle);

#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  handle = start_service_nofree ("zebra", handle);
#endif
#ifdef HAVE_OLSRD
  handle = start_service_nofree ("olsrd", handle);
#endif

  handle = start_service_nofree ("wland", handle);
  handle = start_service_nofree ("wshaper", handle);
  handle = start_service_nofree ("cron", handle);
  handle = start_service_nofree ("radio_timer", handle);

#ifdef HAVE_PPTPD
  handle = start_service_nofree ("pptpd", handle);
#endif

#ifdef HAVE_SSHD
#ifdef HAVE_REGISTER
  if (isregistered ())
#endif
    handle = start_service_nofree ("sshd", handle);
#endif

#ifdef HAVE_RADVD
  handle = start_service_nofree ("radvd", handle);
#endif

#ifdef HAVE_SNMP
  handle = start_service_nofree ("snmp", handle);
#endif

#ifdef HAVE_PPPOESERVER
  handle = start_service_nofree ("pppoeserver", handle);
#endif

#ifdef HAVE_WOL
  handle = start_service_nofree ("wol", handle);
#endif

#ifdef HAVE_NOCAT
  handle = start_service_nofree ("splashd", handle);
#endif

#ifdef HAVE_UPNP
  handle = start_service_nofree ("upnp", handle);
#endif

#ifdef HAVE_NEWMEDIA
  handle = start_service_nofree ("openvpnserversys", handle);
#endif
#ifdef HAVE_RSTATS
  handle = start_service_nofree ("rstats", handle);
#endif
#ifdef HAVE_NSTX
  handle = start_service_nofree ("nstxd", handle);
#endif
#ifdef HAVE_PPPOERELAY
  handle = start_service_nofree ("pppoerelay", handle);
#endif
#ifdef HAVE_MILKFISH
  handle = start_service_nofree ("milkfish", handle);
#endif
  dlclose (handle);

  cprintf ("done\n");
  return 0;
}

int
stop_services (void)
{
  void *handle = NULL;
  //stop_ses();
#ifdef HAVE_PPPOERELAY
  handle = stop_service_nofree ("pppoerelay", handle);
#endif
#ifdef HAVE_RSTATS
  handle = stop_service_nofree ("rstats", handle);
#endif
#ifdef HAVE_NSTX
  handle = stop_service_nofree ("nstxd", handle);
#endif

#ifdef HAVE_UPNP
  handle = stop_service_nofree ("upnp", handle);
#endif
  handle = stop_service_nofree ("udhcpd", handle);
  handle = stop_service_nofree ("dns_clear_resolv", handle);
  handle = stop_service_nofree ("cron", handle);
  handle = stop_service_nofree ("radio_timer", handle);

#ifdef HAVE_TFTP
  handle = stop_service_nofree ("tftpd", handle);
#endif
  handle = stop_service_nofree ("syslog", handle);
#ifdef HAVE_OLSRD
  handle = stop_service_nofree ("olsrd", handle);
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  handle = stop_service_nofree ("zebra", handle);
#endif
  handle = stop_service_nofree ("wland", handle);
#ifdef HAVE_TELNET
#ifdef HAVE_REGISTER
  if (isregistered ())
#endif
    handle = stop_service_nofree ("telnetd", handle);
#endif
#ifdef HAVE_SSHD
#ifdef HAVE_REGISTER
  if (isregistered ())
#endif
    handle = stop_service_nofree ("sshd", handle);
#endif

#ifdef HAVE_RADVD
  handle = stop_service_nofree ("radvd", handle);
#endif

#ifdef HAVE_WIFIDOG
  handle = stop_service_nofree ("wifidog", handle);
#endif

#ifdef HAVE_CHILLI
  handle = stop_service_nofree ("chilli", handle);
#endif


#ifdef HAVE_PPPOESERVER
  handle = stop_service_nofree ("pppoeserver", handle);
#endif

#ifdef HAVE_SNMP
  handle = stop_service_nofree ("snmp", handle);
#endif

#ifdef HAVE_WOL
  handle = stop_service_nofree ("wol", handle);
#endif
  handle = stop_service_nofree ("wshaper", handle);

#ifdef HAVE_PPTPD
  handle = stop_service_nofree ("pptpd", handle);
#endif
#ifdef HAVE_NOCAT
  handle = stop_service_nofree ("splashd", handle);
#endif
#ifdef HAVE_NEWMEDIA
  handle = stop_service_nofree ("openvpnserversys", handle);
#endif
  if (handle)
    dlclose (handle);

  cprintf ("done\n");
  return 0;
}


static void
handle_dhcpd (void)
{
  startstop ("udhcpd");
}
static void
handle_index (void)
{
  unlink ("/tmp/ppp/log");
  void *handle = NULL;
#ifndef HAVE_MADWIFI
  handle = stop_service_nofree ("nas", handle);
#endif
#ifdef HAVE_MADWIFI
  handle = stop_service_nofree ("stabridge", handle);
#endif
#ifdef HAVE_VLANTAGGING
  handle = stop_service_nofree ("bridgesif", handle);
  handle = stop_service_nofree ("vlantagging", handle);
#endif
#ifdef HAVE_BONDING
  handle = stop_service_nofree ("bonding", handle);
#endif
  handle = stop_service_nofree ("lan", handle);
#ifdef HAVE_VLANTAGGING
  handle = stop_service_nofree ("bridging", handle);
#endif
  handle = stop_service_nofree ("wan", handle);
#ifdef HAVE_VLANTAGGING
  handle = start_service_nofree ("bridging", handle);
#endif
  handle = start_service_nofree ("lan", handle);
#ifdef HAVE_BONDING
  handle = start_service_nofree ("bonding", handle);
#endif
#ifdef HAVE_VLANTAGGING
  handle = start_service_nofree ("vlantagging", handle);
  handle = start_service_nofree ("bridgesif", handle);
#endif
  handle = start_service_nofree ("wan_boot", handle);
#ifdef HAVE_MADWIFI
  handle = start_service_nofree ("stabridge", handle);
#endif
  handle = startstop_nofree ("udhcpd", handle);
  handle = startstop_nofree ("dnsmasq", handle);
#ifndef HAVE_MADWIFI
  handle = start_service_nofree ("nas", handle);
#ifdef HAVE_MSSID
  handle = start_service_nofree ("guest_nas", handle);
#endif
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  handle = startstop_nofree ("zebra", handle);
#endif
#ifdef HAVE_OLSRD
  handle = startstop_nofree ("olsrd", handle);
#endif
  handle = startstop_nofree ("firewall", handle);
  handle = startstop_nofree ("httpd", handle);	//httpd will not accept connection anymore on wan/lan ip changes changes
  if (handle)
    dlclose (handle);
}

static void
handle_router (void)
{
  void *handle = NULL;
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  handle = startstop_nofree ("zebra", handle);
#endif
#ifdef HAVE_OLSRD
  handle = startstop_nofree ("olsrd", handle);
#endif
  if (handle)
    dlclose (handle);
}

static void
handle_hotspot (void)
{
  void *handle = NULL;

#ifdef HAVE_WIFIDOG
  handle = startstop_nofree ("wifidog", handle);
#endif
#ifdef HAVE_NOCAT
  handle = startstop_nofree ("splashd", handle);
#endif
#ifdef HAVE_CHILLI
  handle = startstop_nofree ("chilli", handle);
#endif
#ifdef HAVE_SPUTNIK_APD
  handle = startstop_nofree ("sputnik", handle);
#endif
  if (handle)
    dlclose (handle);
  eval ("/etc/config/http-redirect.firewall");
  eval ("/etc/config/smtp-redirect.firewall");


}

static void
handle_services (void)
{
  void *handle = NULL;

#ifdef HAVE_PPPOERELAY
  handle = startstop_nofree ("pppoerelay", handle);
#endif
  handle = startstop_nofree ("udhcpd", handle);
  handle = startstop_nofree ("syslog", handle);
#ifdef HAVE_RSTATS
  handle = startstop_nofree ("rstats", handle);
#endif
#ifdef HAVE_NSTX
  handle = startstop_nofree ("nstxd", handle);
#endif
#ifdef HAVE_PPPOESERVER
  handle = startstop_nofree ("firewall", handle);
  handle = startstop_nofree ("pppoeserver", handle);
#endif
  handle = startstop_nofree ("dnsmasq", handle);
  handle = startstop_nofree ("udhcpd", handle);
#ifdef HAVE_CPUTEMP
  handle = start_service_nofree ("hwmon", handle);
#endif
#ifdef HAVE_TELNET
#ifdef HAVE_REGISTER
  if (isregistered ())
#endif
    handle = startstop_nofree ("telnetd", handle);
#endif
#ifdef HAVE_SNMP
  handle = startstop_nofree ("snmp", handle);
#endif
#ifdef HAVE_OPENVPN
  handle = startstop_nofree ("openvpn", handle);
#endif
#ifdef HAVE_PPTPD
  handle = startstop_nofree ("pptpd", handle);
#endif
#ifdef HAVE_PPTP
  eval ("/etc/config/pptpd_client.startup");
#endif
#ifdef HAVE_RFLOW
  eval ("/etc/config/rflow.startup");
#endif
#ifdef HAVE_KAID
  eval ("/etc/config/kaid.startup");
#endif
#ifdef HAVE_SSHD
#ifdef HAVE_REGISTER
  if (isregistered ())
#endif
    handle = startstop_nofree ("sshd", handle);
#endif
  handle = startstop_nofree ("firewall", handle);
  handle = startstop_nofree ("syslog", handle);
#ifdef HAVE_NEWMEDIA
  handle = startstop_nofree ("openvpnserversys", handle);
#endif
  if (handle)
    dlclose (handle);

}

static void
handle_management (void)
{
  void *handle = NULL;

#ifndef HAVE_MADWIFI
  handle = stop_service_nofree ("nas", handle);
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  handle = stop_service_nofree ("zebra", handle);
#endif
  handle = stop_service_nofree ("cron", handle);
  handle = stop_service_nofree ("udhcpd", handle);
  handle = start_service_nofree ("udhcpd", handle);
  handle = start_service_nofree ("cron", handle);
#ifdef HAVE_IPV6
  handle = start_service_nofree ("ipv6", handle);
#endif
#ifdef HAVE_RADVD
  handle = startstop_nofree ("radvd", handle);
#endif
#ifdef HAVE_PPTPD
  handle = startstop_nofree ("pptpd", handle);
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  handle = start_service_nofree ("zebra", handle);
#endif
  handle = startstop_nofree ("firewall", handle);
  handle = startstop_nofree ("wshaper", handle);
  handle = startstop_nofree ("httpd", handle);

#ifdef HAVE_WOL
  handle = startstop_nofree ("wol", handle);
#endif
#ifndef HAVE_MADWIFI
  handle = start_service_nofree ("nas", handle);
#ifdef HAVE_MSSID
  handle = start_service_nofree ("guest_nas", handle);
#endif
#endif
  if (handle)
    dlclose (handle);

}

static void
handle_pppoe (void)
{
  unlink ("/tmp/ppp/log");
  void *handle = NULL;
#ifndef HAVE_MADWIFI
  handle = stop_service_nofree ("nas", handle);
#endif
#ifdef HAVE_MADWIFI
  handle = stop_service_nofree ("stabridge", handle);
#endif
#ifdef HAVE_VLANTAGGING
  handle = stop_service_nofree ("bridgesif", handle);
  handle = stop_service_nofree ("vlantagging", handle);
#endif
  handle = stop_service_nofree ("lan", handle);
#ifdef HAVE_BONDING
  handle = stop_service_nofree ("bonding", handle);
#endif
#ifdef HAVE_VLANTAGGING
  handle = stop_service_nofree ("bridging", handle);
#endif
  handle = stop_service_nofree ("wan", handle);
#ifdef HAVE_VLANTAGGING
  handle = start_service_nofree ("bridging", handle);
#endif
  handle = start_service_nofree ("lan", handle);
#ifdef HAVE_BONDING
  handle = start_service_nofree ("bonding", handle);
#endif
#ifdef HAVE_VLANTAGGING
  handle = start_service_nofree ("vlantagging", handle);
  handle = start_service_nofree ("bridgesif", handle);
#endif
  handle = start_service_nofree ("wan_boot", handle);
#ifdef HAVE_MADWIFI
  handle = start_service_nofree ("stabridge", handle);
#endif
#ifndef HAVE_MADWIFI
  handle = start_service_nofree ("nas", handle);
#ifdef HAVE_MSSID
  handle = start_service_nofree ("guest_nas", handle);
#endif
#endif
  if (handle)
    dlclose (handle);

}
static void
handle_spppoe (void)
{
  stop_service ("wan");
}
static void
handle_filters (void)
{
  void *handle = NULL;
  handle = stop_service_nofree ("cron", handle);
  handle = startstop_nofree ("firewall", handle);
  handle = startstop_nofree ("syslog", handle);
  handle = startstop_nofree ("wshaper", handle);
  handle = start_service_nofree ("cron", handle);
  handle = startstop_nofree ("igmp_proxy", handle);
  if (handle)
    dlclose (handle);
}
static void
handle_routing (void)
{
  void *handle = NULL;
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  handle = stop_service_nofree ("zebra", handle);
#endif
  handle = startstop_nofree ("firewall", handle);
  handle = start_service_nofree ("set_routes", handle);
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  handle = start_service_nofree ("zebra", handle);
#endif
#ifdef HAVE_OLSRD
  handle = startstop_nofree ("olsrd", handle);
#endif
  if (handle)
    dlclose (handle);

}

static void
handle_alive (void)
{
  eval ("/etc/config/wdswatchdog.startup");
  eval ("/etc/config/schedulerb.startup");
  eval ("/etc/config/proxywatchdog.startup");

}

static void
handle_forward (void)
{
  void *handle = NULL;
  handle = stop_service_nofree ("wshaper", handle);
  handle = stop_service_nofree ("upnp", handle);
  handle = stop_service_nofree ("firewall", handle);
  handle = start_service_nofree ("firewall", handle);
  handle = start_service_nofree ("upnp", handle);
  handle = start_service_nofree ("wshaper", handle);
  if (handle)
    dlclose (handle);

}
static void
handle_qos (void)
{
  startstop ("wshaper");
}

static void
handle_forwardupnp (void)
{
  void *handle = NULL;
#ifdef HAVE_UPNP
  handle = stop_service_nofree ("upnp", handle);
#endif
  handle = stop_service_nofree ("firewall", handle);
#ifdef HAVE_UPNP
  handle = start_service_nofree ("upnp", handle);
#endif
  handle = start_service_nofree ("firewall", handle);
  handle = startstop_nofree ("wshaper", handle);
  if (handle)
    dlclose (handle);

}

static void
handle_routedel (void)
{
  del_routes (nvram_safe_get ("action_service_arg1"));

}
struct SERVICES
{
  char *servicename;
  void (*service) (void);
};

static void
handle_ddns (void)
{
  startstop ("ddns");
  nvram_set ("ddns_change", "update");

}
static void
handle_ping (void)
{
  char *ip = nvram_safe_get ("ping_ip");
  // use Ping.asp as a debugging console
  char cmd[256] = { 0 };
  //snprintf (cmd, sizeof (cmd), "%s > %s 2>&1 &", ip, PING_TMP);
  setenv ("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
//      snprintf (cmd, sizeof (cmd), "%s 2>&1 &", ip);
//      system (cmd);

  snprintf (cmd, sizeof (cmd),
	    "alias ping=\'ping -c 3\'; eval \"%s\" > %s 2>&1 &", ip,
	    PING_TMP);
  system (cmd);

}

static void
handle_upgrade (void)
{
  void *handle = NULL;
  handle = stop_service_nofree ("wan", handle);
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  handle = stop_service_nofree ("zebra", handle);
#endif
#ifdef HAVE_OLSRD
  handle = stop_service_nofree ("olsrd", handle);
#endif
#ifdef HAVE_UPNP
  handle = stop_service_nofree ("upnp", handle);
#endif
  handle = stop_service_nofree ("cron", handle);
  if (handle)
    dlclose (handle);

}

#ifdef HAVE_MIKLFISH
static void
handle_milkfish (void)
{
  startstop ("milkfish");
}
#endif
static void
handle_wireless (void)
{
  void *handle = NULL;
#ifndef HAVE_MADWIFI
  eval ("wlconf", nvram_safe_get ("wl0_ifname"), "down");
#endif

#ifndef HAVE_MADWIFI
  handle = stop_service_nofree ("nas", handle);
#endif
#ifdef HAVE_MADWIFI
  handle = stop_service_nofree ("stabridge", handle);
#endif
  handle = stop_service_nofree ("wan", handle);
#ifdef HAVE_VLANTAGGING
  handle = stop_service_nofree ("bridgesif", handle);
  handle = stop_service_nofree ("vlantagging", handle);
#endif
#ifdef HAVE_BONDING
  handle = stop_service_nofree ("bonding", handle);
#endif
  handle = stop_service_nofree ("lan", handle);
#ifdef HAVE_VLANTAGGING
  handle = stop_service_nofree ("bridging", handle);
#endif
#ifndef HAVE_MSSID
  if (nvram_match ("wl_akm", "wpa") ||
      nvram_match ("wl_akm", "psk") ||
      nvram_match ("wl_akm", "psk2") ||
      nvram_match ("wl_akm", "wpa2") ||
      nvram_match ("wl_akm", "psk psk2") ||
      nvram_match ("wl_akm", "wpa wpa2") || nvram_match ("wl_akm", "radius"))
    sleep (4);
#endif
#ifndef HAVE_MADWIFI
  handle = start_service_nofree ("wlconf", handle);
#endif
#ifdef HAVE_VLANTAGGING
  handle = start_service_nofree ("bridging", handle);
#endif
  handle = start_service_nofree ("lan", handle);
#ifdef HAVE_BONDING
  handle = start_service_nofree ("bonding", handle);
#endif
#ifdef HAVE_VLANTAGGING
  handle = start_service_nofree ("vlantagging", handle);
  handle = start_service_nofree ("bridgesif", handle);
#endif
  handle = start_service_nofree ("wan", handle);
#ifdef HAVE_MADWIFI
  handle = start_service_nofree ("stabridge", handle);
#endif
#ifndef HAVE_MADWIFI
  handle = start_service_nofree ("nas", handle);
#ifdef HAVE_MSSID
  handle = start_service_nofree ("guest_nas", handle);
#endif
#endif
  startstop ("httpd");		//httpd will not accept connection anymore on wan/lan ip changes changes
  if (handle)
    dlclose (handle);

}
static void
handle_wireless_2 (void)
{
  void *handle = NULL;
  handle = stop_service_nofree ("radio_timer", handle);
#ifndef HAVE_MADWIFI
  eval ("wlconf", nvram_safe_get ("wl0_ifname"), "down");
#endif

#ifndef HAVE_MADWIFI
  handle = stop_service_nofree ("nas", handle);
#endif
#ifdef HAVE_MADWIFI
  handle = stop_service_nofree ("stabridge", handle);
#endif
  if (nvram_match ("wl0_mode", "sta")
      || nvram_match ("wl0_mode", "apsta")
      || nvram_match ("wl0_mode", "apstawet"))
    handle = stop_service_nofree ("wan", handle);
#ifdef HAVE_VLANTAGGING
  handle = stop_service_nofree ("bridgesif", handle);
  handle = stop_service_nofree ("vlantagging", handle);
#endif
#ifdef HAVE_BONDING
  handle = stop_service_nofree ("bonding", handle);
#endif
  handle = stop_service_nofree ("lan", handle);
#ifdef HAVE_VLANTAGGING
  handle = stop_service_nofree ("bridging", handle);
#endif
#ifndef HAVE_MSSID
  if (nvram_match ("wl_akm", "wpa") ||
      nvram_match ("wl_akm", "psk") ||
      nvram_match ("wl_akm", "psk2") ||
      nvram_match ("wl_akm", "wpa2") ||
      nvram_match ("wl_akm", "psk psk2") ||
      nvram_match ("wl_akm", "wpa wpa2") || nvram_match ("wl_akm", "radius"))
    sleep (4);
#endif
#ifndef HAVE_MADWIFI
  handle = start_service_nofree ("wlconf", handle);
#endif
#ifdef HAVE_VLANTAGGING
  handle = start_service_nofree ("bridging", handle);
#endif
  handle = start_service_nofree ("lan", handle);
#ifdef HAVE_BONDING
  handle = start_service_nofree ("bonding", handle);
#endif
#ifdef HAVE_VLANTAGGING
  handle = start_service_nofree ("vlantagging", handle);
  handle = start_service_nofree ("bridgesif", handle);
#endif
  if (nvram_match ("wl0_mode", "sta")
      || nvram_match ("wl0_mode", "apsta")
      || nvram_match ("wl0_mode", "apstawet"))
    handle = start_service_nofree ("wan", handle);
#ifdef HAVE_MADWIFI
  handle = start_service_nofree ("stabridge", handle);
#endif
#ifndef HAVE_MADWIFI
  handle = start_service_nofree ("nas", handle);
#ifdef HAVE_MSSID
  handle = start_service_nofree ("guest_nas", handle);
#endif
#endif
  handle = start_service_nofree ("radio_timer", handle);
  if (nvram_match ("wl0_mode", "sta")
      || nvram_match ("wl0_mode", "apsta")
      || nvram_match ("wl0_mode", "apstawet"))
    startstop ("httpd");	//httpd will not accept connection anymore on wan/lan ip changes changes
#ifdef HAVE_MADWIFI
  handle = start_service_nofree ("hostapdwan", handle);
#endif
  if (handle)
    dlclose (handle);


}

static void
handle_dhcp_release (void)
{
  killall ("udhcpc", SIGUSR2);
  sleep (1);

}

#ifdef HAVE_EOP_TUNNEL
static void
handle_eop (void)
{
  eval ("/etc/config/eop-tunnel.startup");
  eval ("/etc/config/eop-tunnel.firewall");

}
#endif
static struct SERVICES services_def[] = {
  {"dhcp", handle_dhcpd},
  {"index", handle_index},
  {"router", handle_router},
  {"hotspot", handle_hotspot},
  {"services", handle_services},
  {"management", handle_management},
  {"start_pppoe", handle_pppoe},
  {"start_pptp", handle_pppoe},
  {"start_l2tp", handle_pppoe},
  {"start_heartbeat", handle_pppoe},
  {"stop_pppoe", handle_spppoe},
  {"stop_pptp", handle_spppoe},
  {"stop_l2tp", handle_spppoe},
  {"stop_heartbeat", handle_spppoe},
  {"filters", handle_filters},
  {"routing", handle_routing},
  {"alive", handle_alive},
  {"forward", handle_forward},
  {"qos", handle_qos},
  {"forward_upnp", handle_forwardupnp},
  {"static_route_del", handle_routedel},
  {"ddns", handle_ddns},
  {"start_ping", handle_ping},
  {"http_upgrade", handle_upgrade},
#ifdef HAVE_MIKLFISH
  {"milkfish", handle_milkfish},
#endif
  {"wireless", handle_wireless},
  {"wireless_2", handle_wireless_2},
  {"dhcp_release", handle_dhcp_release},
#ifdef HAVE_EOP_TUNNEL
  {"eop", handle_eop},
#endif
  {NULL, NULL}
};

int
start_single_service (void)
{

  sleep (3);
  start_service ("overclocking");
  char *next;
  char service[80];
  char *services = nvram_safe_get ("action_service");
  foreach (service, services, next)
  {
    cprintf ("Restart service=[%s]\n", service);
    int servicecount = 0;
    while (services_def[servicecount].servicename != NULL)
      {
	if (!strcmp (services_def[servicecount].servicename, service))
	  services_def[servicecount].service ();
	servicecount++;
      }
  }

  nvram_unset ("action_service");
  nvram_unset ("action_service_arg1");
  return 0;
}

int
is_running (char *process_name)
{
  DIR *dir;
  struct dirent *next;
  int retval = 0;

  dir = opendir ("/proc");
  if (!dir)
    {
      perror ("Cannot open /proc");
      return 0;
    }

  while ((next = readdir (dir)) != NULL)
    {
      FILE *status;
      char filename[100];
      char buffer[100];
      char name[100];

      if (strcmp (next->d_name, "..") == 0)
	continue;
      if (!isdigit (*next->d_name))
	continue;

      sprintf (filename, "/proc/%s/status", next->d_name);

      if (!(status = fopen (filename, "r")))
	continue;

      if (fgets (buffer, 99, status) == NULL)
	{
	  fclose (status);
	  continue;
	}
      fclose (status);

      sscanf (buffer, "%*s %s", name);

      if (strcmp (name, process_name) == 0)
	retval++;
    }

  closedir (dir);
  return retval;
}


/*
 * Call when keepalive mode
 */
int
redial_main (int argc, char **argv)
{
  int need_redial = 0;
  int status;
  pid_t pid;
  int count = 1;
  int num;

  while (1)
    {

      sleep (atoi (argv[1]));
      num = 0;
      count++;

      //fprintf(stderr, "check PPPoE %d\n", num);
      if (!check_wan_link (num))
	{
	  //fprintf(stderr, "PPPoE %d need to redial\n", num);
	  need_redial = 1;
	}
      else
	{
	  //fprintf(stderr, "PPPoE %d not need to redial\n", num);
	  continue;
	}


#if 0
      cprintf ("Check pppx if exist: ");
      if ((fp = fopen ("/proc/net/dev", "r")) == NULL)
	{
	  return -1;
	}

      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  if (strstr (line, "ppp"))
	    {
	      match = 1;
	      break;
	    }
	}
      fclose (fp);
      cprintf ("%s", match == 1 ? "have exist\n" : "ready to dial\n");
#endif

      if (need_redial)
	{
	  pid = fork ();
	  switch (pid)
	    {
	    case -1:
	      perror ("fork failed");
	      exit (1);
	    case 0:
#ifdef HAVE_PPPOE
	      if (nvram_match ("wan_proto", "pppoe"))
		{
		  sleep (1);
		  start_service ("wan_redial");
		}
#endif
#ifdef HAVE_PPTP
	      else if (nvram_match ("wan_proto", "pptp"))
		{
		  stop_service ("pptp");
		  sleep (1);
		  start_service ("wan_redial");
		}
#endif
#if defined(HAVE_PPTP) || defined(HAVE_PPPOE)
	      else
#endif
	      if (nvram_match ("wan_proto", "l2tp"))
		{
		  stop_service ("l2tp");
		  sleep (1);
		  start_service ("l2tp_redial");
		}
//      Moded by Boris Bakchiev
//      We dont need this at all.
//      But if this code is executed by any of pppX programs we might have to do this.

	      else if (nvram_match ("wan_proto", "heartbeat"))
		{
		  if (is_running ("bpalogin") == 0)
		    {
		      stop_service ("heartbeat");
		      sleep (1);
		      start_service ("heartbeat_redial");
		    }

		}

	      exit (0);
	      break;
	    default:
	      waitpid (pid, &status, 0);
	      //dprintf("parent\n");
	      break;
	    }			// end switch
	}			// end if
    }				// end while
}				// end main
