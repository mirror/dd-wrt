


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
  dlclose (handle);
// end Sveasoft additions
//stop_eou();

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
#ifndef HAVE_MADWIFI
  stop_service ("nas");
#endif
#ifdef HAVE_MADWIFI
  stop_service ("stabridge");
#endif
#ifdef HAVE_VLANTAGGING
  stop_service ("bridgesif");
  stop_service ("vlantagging");
#endif
#ifdef HAVE_BONDING
  stop_service ("bonding");
#endif
  stop_service ("lan");
#ifdef HAVE_VLANTAGGING
  stop_service ("bridging");
#endif
  stop_service ("wan");
#ifdef HAVE_VLANTAGGING
  start_service ("bridging");
#endif
  start_service ("lan");
#ifdef HAVE_BONDING
  start_service ("bonding");
#endif
#ifdef HAVE_VLANTAGGING
  start_service ("vlantagging");
  start_service ("bridgesif");
#endif
  start_service ("wan_boot");
#ifdef HAVE_MADWIFI
  start_service ("stabridge");
#endif
  startstop ("udhcpd");
  startstop ("dnsmasq");
#ifndef HAVE_MADWIFI
  start_service ("nas");
#ifdef HAVE_MSSID
  start_service ("guest_nas");
#endif
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  startstop ("zebra");
#endif
#ifdef HAVE_OLSRD
  startstop ("olsrd");
#endif
  startstop ("firewall");
  startstop ("httpd");		//httpd will not accept connection anymore on wan/lan ip changes changes
}

static void
handle_router (void)
{
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  startstop ("zebra");
#endif
#ifdef HAVE_OLSRD
  startstop ("olsrd");
#endif
}

static void
handle_hotspot (void)
{
#ifdef HAVE_WIFIDOG
  startstop ("wifidog");
#endif
#ifdef HAVE_NOCAT
  startstop ("splashd");
#endif
#ifdef HAVE_CHILLI
  startstop ("chilli");
#endif
#ifdef HAVE_SPUTNIK_APD
  startstop ("sputnik");
#endif
  eval ("/etc/config/http-redirect.firewall");
  eval ("/etc/config/smtp-redirect.firewall");


}

static void
handle_services (void)
{
#ifdef HAVE_PPPOERELAY
  startstop ("pppoerelay");
#endif
  startstop ("udhcpd");
  startstop ("syslog");
#ifdef HAVE_RSTATS
  startstop ("rstats");
#endif
#ifdef HAVE_NSTX
  startstop ("nstxd");
#endif
#ifdef HAVE_PPPOESERVER
  startstop ("firewall");
  startstop ("pppoeserver");
#endif
  startstop ("dnsmasq");
  startstop ("udhcpd");
#ifdef HAVE_CPUTEMP
  start_service ("hwmon");
#endif
#ifdef HAVE_TELNET
#ifdef HAVE_REGISTER
  if (isregistered ())
#endif
    startstop ("telnetd");
#endif
#ifdef HAVE_SNMP
  startstop ("snmp");
#endif
#ifdef HAVE_OPENVPN
  startstop ("openvpn");
#endif
#ifdef HAVE_PPTPD
  startstop ("pptpd");
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
    startstop ("sshd");
#endif
  startstop ("firewall");
  startstop ("syslog");
#ifdef HAVE_NEWMEDIA
  startstop ("openvpnserversys");
#endif

}

static void
handle_management (void)
{
#ifndef HAVE_MADWIFI
  stop_service ("nas");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  stop_service ("zebra");
#endif
  stop_service ("cron");
  stop_service ("udhcpd");
  start_service ("udhcpd");
  start_service ("cron");
#ifdef HAVE_IPV6
  start_service ("ipv6");
#endif
#ifdef HAVE_RADVD
  startstop ("radvd");
#endif
#ifdef HAVE_PPTPD
  startstop ("pptpd");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  start_service ("zebra");
#endif
  startstop ("firewall");
  startstop ("wshaper");
  startstop ("httpd");

#ifdef HAVE_WOL
  startstop ("wol");
#endif
#ifndef HAVE_MADWIFI
  start_service ("nas");
#ifdef HAVE_MSSID
  start_service ("guest_nas");
#endif
#endif

}

static void
handle_pppoe (void)
{
  unlink ("/tmp/ppp/log");
#ifndef HAVE_MADWIFI
  stop_service ("nas");
#endif
#ifdef HAVE_MADWIFI
  stop_service ("stabridge");
#endif
#ifdef HAVE_VLANTAGGING
  stop_service ("bridgesif");
  stop_service ("vlantagging");
#endif
  stop_service ("lan");
#ifdef HAVE_BONDING
  stop_service ("bonding");
#endif
#ifdef HAVE_VLANTAGGING
  stop_service ("bridging");
#endif
  stop_service ("wan");
#ifdef HAVE_VLANTAGGING
  start_service ("bridging");
#endif
  start_service ("lan");
#ifdef HAVE_BONDING
  start_service ("bonding");
#endif
#ifdef HAVE_VLANTAGGING
  start_service ("vlantagging");
  start_service ("bridgesif");
#endif
  start_service ("wan_boot");
#ifdef HAVE_MADWIFI
  start_service ("stabridge");
#endif
#ifndef HAVE_MADWIFI
  start_service ("nas");
#ifdef HAVE_MSSID
  start_service ("guest_nas");
#endif
#endif

}
static void
handle_spppoe (void)
{
  stop_service ("wan");
}
static void
handle_filters (void)
{
  stop_service ("cron");
  startstop ("firewall");
  startstop ("syslog");
  startstop ("wshaper");
  start_service ("cron");
  startstop ("igmp_proxy");
}
static void
handle_routing (void)
{
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  stop_service ("zebra");
#endif
  startstop ("firewall");
  start_service ("set_routes");
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  start_service ("zebra");
#endif
#ifdef HAVE_OLSRD
  startstop ("olsrd");
#endif

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
  stop_service ("wshaper");
  stop_service ("upnp");
  stop_service ("firewall");
  start_service ("firewall");
  start_service ("upnp");
  start_service ("wshaper");

}
static void
handle_qos (void)
{
  startstop ("wshaper");
}

static void
handle_forwardupnp (void)
{
#ifdef HAVE_UPNP
  stop_service ("upnp");
#endif
  stop_service ("firewall");
#ifdef HAVE_UPNP
  start_service ("upnp");
#endif
  start_service ("firewall");
  startstop ("wshaper");

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
  stop_service ("wan");
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
  stop_service ("zebra");
#endif
#ifdef HAVE_OLSRD
  stop_service ("olsrd");
#endif
#ifdef HAVE_UPNP
  stop_service ("upnp");
#endif
  stop_service ("cron");

}

#ifdef HAVE_MIKLFISH
static void
handle_milkfish (void)
{
  stop_service ("milkfish");
  start_service ("milkfish");

}
#endif
static void
handle_wireless (void)
{
#ifndef HAVE_MADWIFI
  eval ("wlconf", nvram_safe_get ("wl0_ifname"), "down");
#endif

#ifndef HAVE_MADWIFI
  stop_service ("nas");
#endif
#ifdef HAVE_MADWIFI
  stop_service ("stabridge");
#endif
  stop_service ("wan");
#ifdef HAVE_VLANTAGGING
  stop_service ("bridgesif");
  stop_service ("vlantagging");
#endif
#ifdef HAVE_BONDING
  stop_service ("bonding");
#endif
  stop_service ("lan");
#ifdef HAVE_VLANTAGGING
  stop_service ("bridging");
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
  start_service ("wlconf");
#endif
#ifdef HAVE_VLANTAGGING
  start_service ("bridging");
#endif
  start_service ("lan");
#ifdef HAVE_BONDING
  start_service ("bonding");
#endif
#ifdef HAVE_VLANTAGGING
  start_service ("vlantagging");
  start_service ("bridgesif");
#endif
  start_service ("wan");
#ifdef HAVE_MADWIFI
  start_service ("stabridge");
#endif
#ifndef HAVE_MADWIFI
  start_service ("nas");
#ifdef HAVE_MSSID
  start_service ("guest_nas");
#endif
#endif
  startstop ("httpd");		//httpd will not accept connection anymore on wan/lan ip changes changes

}
static void
handle_wireless_2 (void)
{
  stop_service ("radio_timer");
#ifndef HAVE_MADWIFI
  eval ("wlconf", nvram_safe_get ("wl0_ifname"), "down");
#endif

#ifndef HAVE_MADWIFI
  stop_service ("nas");
#endif
#ifdef HAVE_MADWIFI
  stop_service ("stabridge");
#endif
  if (nvram_match ("wl0_mode", "sta")
      || nvram_match ("wl0_mode", "apsta")
      || nvram_match ("wl0_mode", "apstawet"))
    stop_service ("wan");
#ifdef HAVE_VLANTAGGING
  stop_service ("bridgesif");
  stop_service ("vlantagging");
#endif
#ifdef HAVE_BONDING
  stop_service ("bonding");
#endif
  stop_service ("lan");
#ifdef HAVE_VLANTAGGING
  stop_service ("bridging");
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
  start_service ("wlconf");
#endif
#ifdef HAVE_VLANTAGGING
  start_service ("bridging");
#endif
  start_service ("lan");
#ifdef HAVE_BONDING
  start_service ("bonding");
#endif
#ifdef HAVE_VLANTAGGING
  start_service ("vlantagging");
  start_service ("bridgesif");
#endif
  if (nvram_match ("wl0_mode", "sta")
      || nvram_match ("wl0_mode", "apsta")
      || nvram_match ("wl0_mode", "apstawet"))
    start_service ("wan");
#ifdef HAVE_MADWIFI
  start_service ("stabridge");
#endif
#ifndef HAVE_MADWIFI
  start_service ("nas");
#ifdef HAVE_MSSID
  start_service ("guest_nas");
#endif
#endif
  start_service ("radio_timer");
  if (nvram_match ("wl0_mode", "sta")
      || nvram_match ("wl0_mode", "apsta")
      || nvram_match ("wl0_mode", "apstawet"))
    startstop ("httpd");	//httpd will not accept connection anymore on wan/lan ip changes changes
#ifdef HAVE_MADWIFI
  start_service ("hostapdwan");
#endif

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
  {"wireless2", handle_wireless_2},
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
