/*
 * services.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <sebastian.gottschall@blueline-ag.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

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

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <mkfiles.h>
#include <wlutils.h>
#include <nvparse.h>


#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 251
#define TXPWR_DEFAULT 28


#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

/* AhMan  March 18 2005 */
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)


static int
alreadyInHost (char *host)
{
  FILE *in = fopen ("/tmp/hosts", "rb");
  if (in == NULL)
    return 0;
  char buf[100];
  while (1)
    {
      fscanf (in, "%s", buf);
      if (!strcmp (buf, host))
	{
	  fclose (in);
	  return 1;
	}
      if (feof (in))
	{
	  fclose (in);
	  return 0;
	}
    }
}

void
addHost (char *host, char *ip)
{
  char buf[100];
  char newhost[100];
  if (host == NULL)
    return;
  if (ip == NULL)
    return;
  strcpy (newhost, host);
  char *domain = nvram_safe_get ("lan_domain");
  if (domain != NULL && strlen (domain) > 0 && strcmp (host, "localhost"))
    {
      sprintf (newhost, "%s.%s", host, domain);
    }
  else
    sprintf (newhost, "%s", host);

  if (alreadyInHost (newhost))
    return;
  sprintf (buf, "echo \"%s\t%s\">>/tmp/hosts", ip, newhost);
  system (buf);
}

void
start_vpn_modules (void)
{
#ifndef HAVE_RB500
#ifndef HAVE_XSCALE
  if ((nvram_match ("pptp_pass", "1") || nvram_match ("l2tp_pass", "1")
       || nvram_match ("ipsec_pass", "1")))
    {
      eval ("/sbin/insmod", "ip_conntrack_proto_gre");
      eval ("/sbin/insmod", "ip_nat_proto_gre");
    }
#endif
#endif
  if (nvram_match ("pptp_pass", "1"))
    {
      eval ("/sbin/insmod", "ip_conntrack_pptp");
      eval ("/sbin/insmod", "ip_nat_pptp");
    }
}


void
stop_vpn_modules (void)
{
#ifndef HAVE_RB500
#ifndef HAVE_XSCALE
  eval ("/sbin/rmmod", "ip_nat_proto_gre");
#endif
#endif
  eval ("/sbin/rmmod", "ip_nat_pptp");
  eval ("/sbin/rmmod", "ip_conntrack_pptp");
#ifndef HAVE_RB500
#ifndef HAVE_XSCALE
  eval ("/sbin/rmmod", "ip_conntrack_proto_gre");
#endif
#endif
}

#ifdef HAVE_PPTPD
int
start_pptpd (void)
{
  int ret = 0, mss = 0;
  char *lpTemp;
  FILE *fp;

  if (!nvram_invmatch ("pptpd_enable", "0"))
    {
      stop_pptpd ();
      return 0;
    }
//cprintf("stop vpn modules\n");
//  stop_vpn_modules ();


  // Create directory for use by pptpd daemon and its supporting files
  mkdir ("/tmp/pptpd", 0744);
  cprintf ("open options file\n");
  // Create options file that will be unique to pptpd to avoid interference with pppoe and pptp
  fp = fopen ("/tmp/pptpd/options.pptpd", "w");
  cprintf ("adding radius plugin\n");
  if (nvram_match ("pptpd_radius", "1"))
    fprintf (fp, "plugin /usr/lib/pppd/radius.so\n"
	     "radius-config-file /tmp/pptpd/radius/radiusclient.conf\n"
	     "%s%s\n", nvram_get ("pptpd_radavpair") ? "avpair " : "",
	     nvram_get ("pptpd_radavpair") ? nvram_get ("pptpd_radavpair") :
	     "");
  cprintf ("check if wan_wins = zero\n");
  int nowins = 0;
  if (nvram_match ("wan_wins", "0.0.0.0"))
    {
      nvram_set ("wan_wins", "");
      nowins = 1;
    }
  if (strlen (nvram_safe_get ("wan_wins")) == 0)
    nowins = 1;

  cprintf ("write config\n");
  fprintf (fp, "lock\n"
	   "name *\n"
	   "proxyarp\n"
	   "ipcp-accept-local\n"
	   "ipcp-accept-remote\n"
	   "lcp-echo-failure 3\n"
	   "lcp-echo-interval 5\n"
	   "deflate 0\n"
	   "auth\n"
	   "-chap\n"
	   "-mschap\n"
	   "+mschap-v2\n"
	   "mppe stateless\n"
	   "mppc\n"
	   "ms-ignore-domain\n"
	   "chap-secrets /tmp/pptpd/chap-secrets\n"
	   "ip-up-script /tmp/pptpd/ip-up\n"
	   "ip-down-script /tmp/pptpd/ip-down\n"
	   "ms-dns %s\n" "%s%s%s" "%s%s%s" "mtu %s\n" "mru %s\n",
	   // Crude but very effective one-liners. Speed is not an issue as this is only run at startup.
	   // Since we need NULL's returned by nvram_get's we cant use nvram_safe_get
	   nvram_get ("pptpd_dns1") ? nvram_get ("pptpd_dns1") :
	   nvram_safe_get ("lan_ipaddr"),
	   nvram_get ("pptpd_dns2") ? "ms-dns " : "",
	   nvram_get ("pptpd_dns2") ? nvram_get ("pptpd_dns2") : "",
	   nvram_get ("pptpd_dns2") ? "\n" : "", !nowins ? "ms-wins " : "",
	   !nowins ? nvram_get ("wan_wins") : "", !nowins ? "\n" : "",
//         nvram_get ("pptpd_wins2") ? "ms-wins " : "",
//         nvram_get ("pptpd_wins2") ? nvram_get ("pptpd_wins2") : "",
//         nvram_get ("pptpd_wins2") ? "\n" : "",
	   nvram_get ("pptpd_mtu") ? nvram_get ("pptpd_mtu") : "1450",
	   nvram_get ("pptpd_mru") ? nvram_get ("pptpd_mru") : "1450");


  // Following is all crude and need to be revisited once testing confirms that it does work
  // Should be enough for testing..
  if (nvram_match ("pptpd_radius", "1"))
    {
      if (nvram_get ("pptpd_radserver") != NULL
	  && nvram_get ("pptpd_radpass") != NULL)
	{


	  fclose (fp);

	  mkdir ("/tmp/pptpd/radius", 0744);

	  fp = fopen ("/tmp/pptpd/radius/radiusclient.conf", "w");
	  fprintf (fp, "auth_order radius\n"
		   "login_tries 4\n"
		   "login_timeout 60\n"
		   "radius_timeout 10\n"
		   "nologin /etc/nologin\n"
		   "servers /tmp/pptpd/radius/servers\n"
		   "dictionary /etc/radiusclient/dictionary\n"
		   "seqfile /var/run/radius.seq\n"
		   "mapfile /etc/radiusclient/port-id-map\n"
		   "radius_retries 3\n"
		   "authserver %s:%s\n", nvram_get ("pptpd_radserver"),
		   nvram_get ("pptpd_radport") ? nvram_get ("pptpd_radport") :
		   "radius");

	  if (nvram_get ("pptpd_acctserver") != NULL
	      && nvram_get ("pptpd_acctpass") != NULL)
	    fprintf (fp, "acctserver %s:%s\n", nvram_get ("pptpd_acctserver"),
		     nvram_get ("pptpd_acctport") ?
		     nvram_get ("pptpd_acctport") : "radacct");
	  fclose (fp);

	  fp = fopen ("/tmp/pptpd/radius/servers", "w");
	  fprintf (fp, "%s\t%s\n", nvram_get ("pptpd_acctserver"),
		   nvram_get ("pptpd_acctpass"));
	  fprintf (fp, "%s\t%s\n", nvram_get ("pptpd_radserver"),
		   nvram_get ("pptpd_radpass"));
	  fclose (fp);


	}
      else
	fclose (fp);
    }
  else
    fclose (fp);


  // Create pptpd.conf options file for pptpd daemon
  fp = fopen ("/tmp/pptpd/pptpd.conf", "w");
  fprintf (fp, "bcrelay %s\n"
	   "localip %s\n"
	   "remoteip %s\n", nvram_safe_get ("lan_ifname"),
	   nvram_safe_get ("pptpd_lip"), nvram_safe_get ("pptpd_rip"));
  fclose (fp);


  // Create ip-up and ip-down scripts that are unique to pptpd to avoid interference with pppoe and pptp
  /* adjust for tunneling overhead (mtu - 40 byte IP - 108 byte tunnel overhead) */
  if (nvram_match ("mtu_enable", "1"))
    mss = atoi (nvram_safe_get ("wan_mtu")) - 40 - 108;
  else
    mss = 1500 - 40 - 108;

  fp = fopen ("/tmp/pptpd/ip-up", "w");
  fprintf (fp, "#!/bin/sh\n"
	   "/usr/sbin/iptables -I FORWARD -i $1 -p tcp --tcp-flags SYN,RST SYN -m tcpmss --mss %d: -j TCPMSS --set-mss %d\n"
	   "/usr/sbin/iptables -I INPUT -i $1 -j ACCEPT\n"
	   "/usr/sbin/iptables -I FORWARD -i $1 -j ACCEPT\n"
	   "%s\n", mss + 1, mss,
	   nvram_get ("pptpd_ipup_script") ? nvram_get ("pptpd_ipup_script") :
	   "");
  fclose (fp);
  fp = fopen ("/tmp/pptpd/ip-down", "w");
  fprintf (fp, "#!/bin/sh\n"
	   "/usr/sbin/iptables -D FORWARD -i $1 -p tcp --tcp-flags SYN,RST SYN -m tcpmss --mss %d: -j TCPMSS --set-mss %d\n"
	   "/usr/sbin/iptables -D INPUT -i $1 -j ACCEPT\n"
	   "/usr/sbin/iptables -D FORWARD -i $1 -j ACCEPT\n"
	   "%s\n", mss + 1, mss,
	   nvram_get ("pptpd_ipdown_script") ?
	   nvram_get ("pptpd_ipdown_script") : "");
  fclose (fp);
  chmod ("/tmp/pptpd/ip-up", 0744);
  chmod ("/tmp/pptpd/ip-down", 0744);

  // Exctract chap-secrets from nvram and add the default account with routers password
  lpTemp = nvram_safe_get ("pptpd_auth");
  fp = fopen ("/tmp/pptpd/chap-secrets", "w");
//  fprintf (fp, "root\t*\t%s\t*\n", nvram_safe_get ("http_passwd"));
  if (strlen (lpTemp) != 0)
    fprintf (fp, "%s\n", lpTemp);
  fclose (fp);

  chmod ("/tmp/pptpd/chap-secrets", 0600);

  //  Execute pptpd daemon
  ret =
    eval ("/usr/sbin/pptpd", "-c", "/tmp/pptpd/pptpd.conf", "-o",
	  "/tmp/pptpd/options.pptpd");

  return ret;
}

int
stop_pptpd (void)
{
  int ret = 0;

  ret = eval ("killall", "-9", "pptpd");
  eval ("killall", "-9", "bcrelay");
  return ret;
}
#endif

/* AhMan  March 18 2005 */
void start_tmp_ppp (int num);

int
softkill (char *name)
{
  char sigusr1[] = "-XX";
  sprintf (sigusr1, "-%d", SIGUSR1);
  eval ("killall", sigusr1, name);
  sleep (1);
  return eval ("killall", "-9", name);

}


int
adjust_dhcp_range (void)
{
  struct in_addr ipaddr, netaddr, netmask;

  char *lan_ipaddr = nvram_safe_get ("lan_ipaddr");
  char *lan_netmask = nvram_safe_get ("lan_netmask");
  char *dhcp_num = nvram_safe_get ("dhcp_num");
  char *dhcp_start = nvram_safe_get ("dhcp_start");

  int legal_start_ip, legal_end_ip, legal_total_ip, dhcp_start_ip;
  int set_dhcp_start_ip = 0, set_dhcp_num = 0;
  int adjust_ip = 0, adjust_num = 0;

  inet_aton (lan_ipaddr, &netaddr);
  inet_aton (lan_netmask, &netmask);
  inet_aton (dhcp_start, &ipaddr);

  legal_total_ip = 254 - get_single_ip (lan_netmask, 3);
  legal_start_ip =
    (get_single_ip (lan_ipaddr, 3) & get_single_ip (lan_netmask, 3)) + 1;
  legal_end_ip = legal_start_ip + legal_total_ip - 1;
  dhcp_start_ip = atoi (dhcp_start);

  cprintf
    ("legal_total_ip=[%d] legal_start_ip=[%d] legal_end_ip=[%d] dhcp_start_ip=[%d]\n",
     legal_total_ip, legal_start_ip, legal_end_ip, dhcp_start_ip);

  if ((dhcp_start_ip > legal_end_ip) || (dhcp_start_ip < legal_start_ip))
    {
      cprintf ("Illegal DHCP Start IP: We need to adjust DHCP Start ip.\n");
      set_dhcp_start_ip = legal_start_ip;
      adjust_ip = 1;
      if (atoi (dhcp_num) > legal_total_ip)
	{
	  cprintf ("DHCP num is exceed, we need to adjust.");
	  set_dhcp_num = legal_total_ip;
	  adjust_num = 1;
	}
    }
  else
    {
      cprintf ("Legal DHCP Start IP: We need to check DHCP num.\n");
      if ((atoi (dhcp_num) + dhcp_start_ip) > legal_end_ip)
	{
	  cprintf ("DHCP num is exceed, we need to adjust.\n");
	  set_dhcp_num = legal_end_ip - dhcp_start_ip + 1;
	  adjust_num = 1;
	}
    }

  if (adjust_ip)
    {
      char ip[20];
      cprintf ("set_dhcp_start_ip=[%d]\n", set_dhcp_start_ip);
      snprintf (ip, sizeof (ip), "%d", set_dhcp_start_ip);
      nvram_set ("dhcp_start", ip);
    }
  if (adjust_num)
    {
      char num[5];
      cprintf ("set_dhcp_num=[%d]\n", set_dhcp_num);
      snprintf (num, sizeof (num), "%d", set_dhcp_num);
      nvram_set ("dhcp_num", num);
    }

  return 1;
}


int
write_nvram (char *name, char *nv)
{
  if (nvram_invmatch (nv, ""))
    {
      FILE *fp = fopen (name, "wb");

      char *host_key = nvram_safe_get (nv);
      int i = 0;
      do
	{
	  if (host_key[i] != 0x0D)
	    fprintf (fp, "%c", host_key[i]);
	}
      while (host_key[++i]);
      fclose (fp);
    }
  else
    return -1;
  return 0;
}


int
start_dhcpfwd (void)
{
  if (nvram_match ("wl_mode", "wet"))	//dont start any dhcp services in bridge mode
    {
      nvram_set ("lan_proto", "static");
      return 0;
    }
#ifdef HAVE_DHCPFORWARD
  FILE *fp;
  if (nvram_match ("dhcpfwd_enable", "1"))
    {
      mkdir ("/tmp/dhcp-fwd", 0700);
      mkdir ("/var/run/dhcp-fwd", 0700);
      fp = fopen ("/tmp/dhcp-fwd/dhcp-fwd.conf", "wb");
      fprintf (fp, "user		root\n");
      fprintf (fp, "group		root\n");
      fprintf (fp, "chroot		/var/run/dhcp-fwd\n");
      fprintf (fp, "logfile		/tmp/dhcp-fwd.log\n");
      fprintf (fp, "loglevel	1\n");
      fprintf (fp, "pidfile		/var/run/dhcp-fwd.pid\n");
      fprintf (fp, "ulimit core	0\n");
      fprintf (fp, "ulimit stack	64K\n");
      fprintf (fp, "ulimit data	32K\n");
      fprintf (fp, "ulimit rss	200K\n");
      fprintf (fp, "ulimit nproc	0\n");
      fprintf (fp, "ulimit nofile	0\n");
      fprintf (fp, "ulimit as	0\n");
      fprintf (fp, "if	%s	true	false	true\n",
	       nvram_safe_get ("lan_ifname"));

      char *wan_proto = nvram_safe_get ("wan_proto");
      char *wan_ifname = nvram_safe_get ("wan_ifname");
#ifdef HAVE_MADWIFI
      if (nvram_match ("ath0_mode", "sta"))
#else
      if (nvram_match ("wl_mode", "sta"))
#endif
	{
	  wan_ifname = get_wdev ();	//returns eth1/eth2 for broadcom and ath0 for atheros
	}
#ifdef HAVE_PPPOE
      if (strcmp (wan_proto, "pppoe") == 0)
	{
	  fprintf (fp, "if	ppp0	false	true	true\n");

	}
#else
      if (0)
	{
	}
#endif
      else if (strcmp (wan_proto, "dhcp") == 0
	       || strcmp (wan_proto, "static") == 0)
	{
	  fprintf (fp, "if	%s	false	true	true\n",
		   nvram_safe_get ("wan_ifname"));
	}
#ifdef HAVE_PPTP
      else if (strcmp (wan_proto, "pptp") == 0)
	{
	  fprintf (fp, "if	ppp0	false	true	true\n");
	}
#endif
#ifdef HAVE_L2TP
      else if (strcmp (wan_proto, "l2tp") == 0)
	{
	  fprintf (fp, "if	ppp0	false	true	true\n");
	}
#endif
#ifdef HAVE_HEARTBEAT
      else if (strcmp (wan_proto, "heartbeat") == 0)
	{
	  fprintf (fp, "if	ppp0	false	true	true\n");
	}
#endif
      else
	{
	  fprintf (fp, "if	%s	false	true	true\n",
		   nvram_safe_get ("wan_ifname"));
	}

      fprintf (fp, "name	%s	ws-c\n",
	       nvram_safe_get ("lan_ifname"));
      fprintf (fp, "server	ip	%s\n", nvram_safe_get ("dhcpfwd_ip"));
      fclose (fp);
      eval ("dhcpfwd", "-c", "/tmp/dhcp-fwd/dhcp-fwd.conf");
      return 0;
    }
#endif
#ifdef HAVE_DHCPRELAY
  if (nvram_match ("dhcpfwd_enable", "1"))
    {
      eval ("dhcrelay", "-i", nvram_safe_get ("lan_ifname"),
	    nvram_safe_get ("dhcpfwd_ip"));
    }
#endif
  return 0;

}

void
stop_dhcpfwd (void)
{
#ifdef HAVE_DHCPFORWARD
  eval ("killall", "dhcpfwd");	//kill also dhcp forwarder if available
#endif
#ifdef HAVE_DHCPRELAY
  eval ("killall", "dhcrelay");
#endif
}

int usejffs = 0;

int
start_udhcpd (void)
{
  FILE *fp;
  struct dns_lists *dns_list = NULL;
  int i = 0;
  if (nvram_match ("dhcpfwd_enable", "1"))
    {
      return 0;
    }
#ifndef HAVE_RB500
#ifndef HAVE_XSCALE
  if (nvram_match ("wl_mode", "wet"))	//dont start any dhcp services in bridge mode
    {
      nvram_set ("lan_proto", "static");
      return 0;
    }
#endif
#endif

  if (nvram_match ("router_disable", "1")
      || nvram_invmatch ("lan_proto", "dhcp")
      || nvram_match ("dhcp_dnsmasq", "1"))
    {
      stop_udhcpd ();
      return 0;
    }

  /* Automatically adjust DHCP Start IP and num when LAN IP or netmask is changed */
  adjust_dhcp_range ();

  cprintf ("%s %d.%d.%d.%s %s %s\n",
	   nvram_safe_get ("lan_ifname"),
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 2),
	   nvram_safe_get ("dhcp_start"),
	   nvram_safe_get ("dhcp_end"), nvram_safe_get ("lan_lease"));

  /* Touch leases file */

  usejffs = 0;

  if (nvram_match ("dhcpd_usejffs", "1"))
    {
      if (!(fp = fopen ("/jffs/udhcpd.leases", "a")))
	{
	  usejffs = 0;
	}
      else
	{
	  usejffs = 1;
	}
    }
  if (!usejffs)
    if (!(fp = fopen ("/tmp/udhcpd.leases", "a")))
      {
	perror ("/tmp/udhcpd.leases");
	return errno;
      }

  fclose (fp);

  /* Write configuration file based on current information */
  if (!(fp = fopen ("/tmp/udhcpd.conf", "w")))
    {
      perror ("/tmp/udhcpd.conf");
      return errno;
    }
  fprintf (fp, "pidfile /var/run/udhcpd.pid\n");
  fprintf (fp, "start %d.%d.%d.%s\n",
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 2),
	   nvram_safe_get ("dhcp_start"));
  fprintf (fp, "end %d.%d.%d.%d\n",
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 2),
	   atoi (nvram_safe_get ("dhcp_start")) +
	   atoi (nvram_safe_get ("dhcp_num")) - 1);
  int dhcp_max =
    atoi (nvram_safe_get ("dhcp_num")) +
    atoi (nvram_safe_get ("static_leasenum"));
  fprintf (fp, "max_leases %d\n", dhcp_max);
  fprintf (fp, "interface %s\n", nvram_safe_get ("lan_ifname"));
  fprintf (fp, "remaining yes\n");
  fprintf (fp, "auto_time 30\n");	// N seconds to update lease table
  if (usejffs)
    fprintf (fp, "lease_file /jffs/udhcpd.leases\n");
  else
    fprintf (fp, "lease_file /tmp/udhcpd.leases\n");
  fprintf (fp, "statics_file /tmp/udhcpd.statics\n");

  if (nvram_invmatch ("lan_netmask", "")
      && nvram_invmatch ("lan_netmask", "0.0.0.0"))
    fprintf (fp, "option subnet %s\n", nvram_safe_get ("lan_netmask"));

  if (nvram_invmatch ("lan_ipaddr", "")
      && nvram_invmatch ("lan_ipaddr", "0.0.0.0"))
    fprintf (fp, "option router %s\n", nvram_safe_get ("lan_ipaddr"));

  if (nvram_invmatch ("wan_wins", "")
      && nvram_invmatch ("wan_wins", "0.0.0.0"))
    fprintf (fp, "option wins %s\n", nvram_get ("wan_wins"));

  // Wolf add - keep lease within reasonable timeframe
  if (atoi (nvram_safe_get ("dhcp_lease")) < 10)
    {
      nvram_set ("dhcp_lease", "10");
      nvram_commit ();
    }
  if (atoi (nvram_safe_get ("dhcp_lease")) > 5760)
    {
      nvram_set ("dhcp_lease", "5760");
      nvram_commit ();
    }

  fprintf (fp, "option lease %d\n",
	   atoi (nvram_safe_get ("dhcp_lease")) ?
	   atoi (nvram_safe_get ("dhcp_lease")) * 60 : 86400);

  dns_list = get_dns_list ();

  if (!dns_list || dns_list->num_servers == 0)
    {

      if (nvram_invmatch ("lan_ipaddr", ""))
	fprintf (fp, "option dns %s\n", nvram_safe_get ("lan_ipaddr"));

    }
  else if (nvram_match ("local_dns", "1"))
    {

      if (dns_list
	  && (nvram_invmatch ("lan_ipaddr", "")
	      || strlen (dns_list->dns_server[0]) > 0
	      || strlen (dns_list->dns_server[1]) > 0
	      || strlen (dns_list->dns_server[2]) > 0))
	{

	  fprintf (fp, "option dns");

	  if (nvram_invmatch ("lan_ipaddr", ""))
	    fprintf (fp, " %s", nvram_safe_get ("lan_ipaddr"));

	  if (strlen (dns_list->dns_server[0]) > 0)
	    fprintf (fp, " %s", dns_list->dns_server[0]);

	  if (strlen (dns_list->dns_server[1]) > 0)
	    fprintf (fp, " %s", dns_list->dns_server[1]);

	  if (strlen (dns_list->dns_server[2]) > 0)
	    fprintf (fp, " %s", dns_list->dns_server[2]);

	  fprintf (fp, "\n");
	}
    }
  else
    {

      if (dns_list
	  && (strlen (dns_list->dns_server[0]) > 0
	      || strlen (dns_list->dns_server[1]) > 0
	      || strlen (dns_list->dns_server[2]) > 0))
	{

	  fprintf (fp, "option dns");

	  if (strlen (dns_list->dns_server[0]) > 0)
	    fprintf (fp, " %s", dns_list->dns_server[0]);

	  if (strlen (dns_list->dns_server[1]) > 0)
	    fprintf (fp, " %s", dns_list->dns_server[1]);

	  if (strlen (dns_list->dns_server[2]) > 0)
	    fprintf (fp, " %s", dns_list->dns_server[2]);

	  fprintf (fp, "\n");
	}
    }

  if (dns_list)
    free (dns_list);

  /* DHCP Domain */
  if (nvram_match ("dhcp_domain", "wan"))
    {
      if (nvram_invmatch ("wan_domain", ""))
	fprintf (fp, "option domain %s\n", nvram_safe_get ("wan_domain"));
      else if (nvram_invmatch ("wan_get_domain", ""))
	fprintf (fp, "option domain %s\n", nvram_safe_get ("wan_get_domain"));
    }
  else
    {
      if (nvram_invmatch ("lan_domain", ""))
	fprintf (fp, "option domain %s\n", nvram_safe_get ("lan_domain"));
    }

  if (nvram_invmatch ("dhcpd_options", ""))
    {
      char *host_key = nvram_safe_get ("dhcpd_options");
      i = 0;
      do
	{
	  if (host_key[i] != 0x0D)
	    fprintf (fp, "%c", host_key[i]);
	}
      while (host_key[++i]);
    }
  /* end Sveasoft addition */
  fclose (fp);

  /* Sveasoft addition - create static leases file */
  // Static for router
  if (!(fp = fopen ("/tmp/udhcpd.statics", "w")))
    {
      perror ("/tmp/udhcpd.statics");
      return errno;
    }

  if (nvram_match ("local_dns", "1"))
    fprintf (fp, "%s %s %s\n", nvram_safe_get ("lan_ipaddr"),
	     nvram_safe_get ("et0macaddr"), nvram_safe_get ("router_name"));

  int leasenum = atoi (nvram_safe_get ("static_leasenum"));
  if (leasenum > 0)
    {
      char *lease = nvram_safe_get ("static_leases");
      char *leasebuf = (char *) malloc (strlen (lease) + 1);
      char *cp = leasebuf;
      strcpy (leasebuf, lease);
      for (i = 0; i < leasenum; i++)
	{
	  char *mac = strsep (&leasebuf, "=");
	  char *host = strsep (&leasebuf, "=");
	  char *ip = strsep (&leasebuf, " ");
	  if (mac == NULL || host == NULL || ip == NULL)
	    continue;
	  fprintf (fp, "%s %s %s\n", ip, mac, host);
	  addHost (host, ip);
	}
      free (cp);
    }
  //#dhcp-host=bert,192.168.0.70,infinite

  //#dhcp-host=11:22:33:44:55:66,fred,192.168.0.60,45m


/*	if (nvram_invmatch("dhcpd_statics", "")){
		char *host_key = nvram_safe_get("dhcpd_statics");
		i = 0;
		do{
			if(host_key[i] != 0x0D)
				fprintf(fp, "%c", host_key[i]);
		} while(host_key[++i]);
	}*/
  fclose (fp);
  /* end Sveasoft addition */

  dns_to_resolv ();

  eval ("udhcpd", "/tmp/udhcpd.conf");

  /* Dump static leases to udhcpd.leases so they can be read by dnsmasq */
  /* DD-WRT (belanger) : the dump database is now handled directly in udhcpd */
//  sprintf (sigusr1, "-%d", SIGUSR1);
//killps("udhcpd",sigusr1);

//  eval ("killall", sigusr1, "udhcpd");

  cprintf ("done\n");
  return 0;
}

int
stop_udhcpd (void)
{
  softkill ("udhcpd");
  cprintf ("done\n");
  return 0;
}



int
start_dnsmasq (void)
{
  FILE *fp;
  struct dns_lists *dns_list = NULL;
  int ret;
  int i;

  if (nvram_match ("dhcp_dnsmasq", "1") && nvram_match ("lan_proto", "dhcp")
      && nvram_match ("dnsmasq_enable", "0"))
    {
      nvram_set ("dnsmasq_enable", "1");
      nvram_commit ();
    }

  // Sveasoft 2003-12-15 only start if enabled
  if (!nvram_invmatch ("dnsmasq_enable", "0"))
    {
      stop_dnsmasq ();
      return 0;
    }

  usejffs = 0;

  if (nvram_match ("dhcpd_usejffs", "1"))
    {
      if (!(fp = fopen ("/jffs/dnsmasq.leases", "a")))
	{
	  usejffs = 0;
	}
      else
	{
	  fclose (fp);
	  usejffs = 1;
	}
    }

  /* Write configuration file based on current information */
  if (!(fp = fopen ("/tmp/dnsmasq.conf", "w")))
    {
      perror ("/tmp/dnsmasq.conf");
      return errno;
    }

  if (nvram_match ("fon_enable", "1")
      || (nvram_match ("chilli_nowifibridge", "1")
	  && nvram_match ("chilli_enable", "1")))
    {
      fprintf (fp, "interface=%s,br0\n", nvram_safe_get ("wl0_ifname"));
    }
  else
    {
      if (nvram_match ("chilli_enable", "1"))
	fprintf (fp, "interface=%s\n", nvram_safe_get ("wl0_ifname"));
      else if (nvram_match ("pptpd_enable", "1"))
	fprintf (fp, "listen-address=%s,%s\n", "127.0.0.1", nvram_safe_get ("lan_ipaddr"));
      else
	fprintf (fp, "interface=%s\n", nvram_safe_get ("lan_ifname"));
    }

  fprintf (fp, "resolv-file=/tmp/resolv.dnsmasq\n");

  /* Domain */
  if (nvram_match ("dhcp_domain", "wan"))
    {
      if (nvram_invmatch ("wan_domain", ""))
	fprintf (fp, "domain=%s\n", nvram_safe_get ("wan_domain"));
      else if (nvram_invmatch ("wan_get_domain", ""))
	fprintf (fp, "domain=%s\n", nvram_safe_get ("wan_get_domain"));
    }
  else
    {
      if (nvram_invmatch ("lan_domain", ""))
	fprintf (fp, "domain=%s\n", nvram_safe_get ("lan_domain"));
    }

  /* DD-WRT use dnsmasq as DHCP replacement */
  if (!nvram_match ("wl_mode", "wet"))
    if (nvram_match ("dhcp_dnsmasq", "1") && nvram_match ("lan_proto", "dhcp")
	&& nvram_match ("dhcpfwd_enable", "0"))
      {
	if (usejffs)
	  fprintf (fp, "dhcp-leasefile=/jffs/dnsmasq.leases\n");
	else if (nvram_match ("dhcpd_usenvram", "1"))
	  {
	    fprintf (fp, "leasefile-ro\n");
	    fprintf (fp, "dhcp-script=%s\n", "/etc/lease_update.sh");
	  }
	else
	  fprintf (fp, "dhcp-leasefile=/tmp/dnsmasq.leases\n");

	int dhcp_max =
	  atoi (nvram_safe_get ("dhcp_num")) +
	  atoi (nvram_safe_get ("static_leasenum"));
	fprintf (fp, "dhcp-lease-max=%d\n", dhcp_max);
	fprintf (fp, "dhcp-option=3,%s\n", nvram_safe_get ("lan_ipaddr"));
	if (nvram_invmatch ("wan_wins", "")
	    && nvram_invmatch ("wan_wins", "0.0.0.0"))
	  fprintf (fp, "dhcp-option=44,%s\n", nvram_safe_get ("wan_wins"));

	if (nvram_match ("dns_dnsmasq", "0"))
	  {
	    dns_list = get_dns_list ();

	    if (dns_list
		&& (strlen (dns_list->dns_server[0]) > 0
		    || strlen (dns_list->dns_server[1]) > 0
		    || strlen (dns_list->dns_server[2]) > 0))
	      {

		fprintf (fp, "dhcp-option=6");

		if (strlen (dns_list->dns_server[0]) > 0)
		  fprintf (fp, ",%s", dns_list->dns_server[0]);

		if (strlen (dns_list->dns_server[1]) > 0)
		  fprintf (fp, ",%s", dns_list->dns_server[1]);

		if (strlen (dns_list->dns_server[2]) > 0)
		  fprintf (fp, ",%s", dns_list->dns_server[2]);

		fprintf (fp, "\n");
	      }

	    if (dns_list)
	      free (dns_list);
	  }

	if (nvram_match ("auth_dnsmasq", "1"))
	  fprintf (fp, "dhcp-authoritative\n");
	fprintf (fp, "dhcp-range=");
	fprintf (fp, "%d.%d.%d.%s,",
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 2),
		 nvram_safe_get ("dhcp_start"));
	fprintf (fp, "%d.%d.%d.%d,",
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 2),
		 atoi (nvram_safe_get ("dhcp_start")) +
		 atoi (nvram_safe_get ("dhcp_num")) - 1);
	fprintf (fp, "%s,", nvram_safe_get ("lan_netmask"));
	fprintf (fp, "%sm\n", nvram_safe_get ("dhcp_lease"));
	int leasenum = atoi (nvram_safe_get ("static_leasenum"));
	if (leasenum > 0)
	  {
	    char *lease = nvram_safe_get ("static_leases");
	    char *leasebuf = (char *) malloc (strlen (lease) + 1);
	    char *cp = leasebuf;
	    strcpy (leasebuf, lease);
	    for (i = 0; i < leasenum; i++)
	      {
		char *mac = strsep (&leasebuf, "=");
		char *host = strsep (&leasebuf, "=");
		char *ip = strsep (&leasebuf, " ");
		if (mac == NULL || host == NULL || ip == NULL)
		  continue;

		fprintf (fp, "dhcp-host=%s,%s,%s,infinite\n", mac, host, ip);
		addHost (host, ip);
	      }
	    free (cp);
	  }
	//#dhcp-host=bert,192.168.0.70,infinite

	//#dhcp-host=11:22:33:44:55:66,fred,192.168.0.60,45m

      }

  /* Additional options */
  if (nvram_invmatch ("dnsmasq_options", ""))
    {
      char *host_key = nvram_safe_get ("dnsmasq_options");
      i = 0;
      do
	{
	  if (host_key[i] != 0x0D)
	    fprintf (fp, "%c", host_key[i]);
	}
      while (host_key[++i]);
    }
  fclose (fp);

  dns_to_resolv ();

  chmod ("/etc/lease_update.sh", 0700);
  ret = eval ("dnsmasq", "--conf-file", "/tmp/dnsmasq.conf");

  cprintf ("done\n");
  return ret;
}

int
stop_dnsmasq (void)
{
  int ret = softkill ("dnsmasq");
  unlink ("/tmp/resolv.dnsmasq");

  cprintf ("done\n");
  return ret;
}

int
stop_dns_clear_resolv (void)
{
  FILE *fp_w;
  //int ret = killps("dnsmasq",NULL);
  int ret = eval ("killall", "dnsmasq");

  /* Save DNS to resolv.conf */
  if (!(fp_w = fopen (RESOLV_FILE, "w")))
    {
      perror (RESOLV_FILE);
      return errno;
    }
  fprintf (fp_w, " ");
  fclose (fp_w);

  cprintf ("done\n");
  return ret;
}

int
start_httpd (void)
{
  int ret = 0;
  if (nvram_invmatch ("http_enable", "0") && !is_exist ("/var/run/httpd.pid"))
    {
      chdir ("/www");
//      if (chdir ("/tmp/www") == 0)
//      cprintf ("[HTTPD Starting on /tmp/www]\n");
//      else
      cprintf ("[HTTPD Starting on /www]\n");
      ret = eval ("httpd");
      chdir ("/");
    }
#ifdef HAVE_HTTPS
  if (nvram_invmatch ("https_enable", "0")
      && !is_exist ("/var/run/httpsd.pid"))
    {

      // Generate a new certificate
      //if(!is_exist("/tmp/cert.pem") || !is_exist("/tmp/key.pem"))
      //      eval("gencert.sh", BUILD_SECS);         

      chdir ("/www");
      ret = eval ("httpd", "-S");
      chdir ("/");
    }
#endif

  cprintf ("done\n");
  return ret;
}

int
stop_httpd (void)
{
  //int ret = killps("httpd",NULL);
  int ret = eval ("killall", "httpd");

  unlink ("/var/run/httpd.pid");
#ifdef HAVE_HTTPS
  unlink ("/var/run/httpsd.pid");
#endif
  cprintf ("done\n");
  return ret;
}

#ifdef HAVE_UPNP
int
start_upnp (void)
{
  char *wan_ifname = get_wan_face ();
  int ret;

  if (nvram_match ("upnp_enable", "0"))
    {
      stop_upnp ();
      return 0;
    }
  /* Make sure its not running first */
  ret = eval ("killall", "-SIGUSR1", "upnp");
  if (ret != 0)
    {
      ret = eval ("upnp", "-D",
		  "-L", nvram_safe_get ("lan_ifname"),
		  "-W", wan_ifname,
		  "-I", nvram_safe_get ("upnp_ssdp_interval"),
		  "-A", nvram_safe_get ("upnp_max_age"));
    }

  cprintf ("done\n");
  return ret;
}

int
stop_upnp (void)
{
  //int ret = killps("upnp","-USR1");

  eval ("killall", "-USR1", "upnp");

  //killps("upnp",NULL);
  eval ("killall", "upnp");

  cprintf ("done\n");
  return 0;
}
#endif
/*int
start_ses(void)
{
	if(nvram_match("wl_gmode", "-1"))
		return 0;

	if (nvram_match("ses_enable", "1")) {
		eval("ses", "-f");
	}

	return 0;
}

int
stop_ses(void)
{
	int ret = 0;

	if(nvram_match("wl_gmode", "-1")) {
		diag_led(SES_LED1,STOP_LED);
		diag_led(SES_LED2,STOP_LED);
	}

	ret = eval("killall", "ses");

	return ret;
}
*/
static void
convert_wds (void)
{
  char wds_mac[254];
  char buf[254];

  if (nvram_match ("wl_wds", ""))	// For Router, accept all WDS link
    strcpy (wds_mac, "*");
  else				// For AP, assign remote WDS MAC
    strcpy (wds_mac, nvram_safe_get ("wl_wds"));

  /* For WPA-PSK mode, we want to convert wl_wds_mac to wl0_wds0 ... wl0_wds255 */
  if (nvram_match ("security_mode", "psk")
      || nvram_match ("security_mode", "psk2"))
    {
      char wl_wds[] = "wl0_wdsXXX";
      int i = 0;
      int j;
      char mac[254];
      char *next;

      foreach (mac, wds_mac, next)
      {
	snprintf (wl_wds, sizeof (wl_wds), "wl0_wds%d", i);
	snprintf (buf, sizeof (buf), "%s,auto,%s,%s,%s,%s",
		  mac,
		  nvram_safe_get ("wl_crypto"),
		  nvram_safe_get ("security_mode"),
#ifndef HAVE_MSSID
		  nvram_safe_get ("wl_ssid"), nvram_safe_get ("wl_wpa_psk"));
#else
		  nvram_safe_get ("wl0_ssid"), nvram_safe_get ("wl_wpa_psk"));
#endif
	nvram_set (wl_wds, buf);
	i++;
      }

      /* Del unused entry */
      for (j = i; j < MAX_NVPARSE; j++)
	del_wds_wsec (0, j);
    }
}

#ifdef HAVE_MSSID
int
start_guest_nas (void)
{
  char *unbridged_interfaces;
  char *next;
  char name[IFNAMSIZ], lan[IFNAMSIZ];
  int index;

/*	unbridged_interfaces = nvram_get("unbridged_ifnames");
	
	if (unbridged_interfaces)
		foreach(name,unbridged_interfaces,next){
			index = get_ipconfig_index(name);
			if (index < 0) 
				continue;
			snprintf(lan,sizeof(lan),"lan%d",index);
			start_nas(lan);
		}
*/
  return 0;
}
#endif
char *
getSecMode (char *prefix)
{
  char wep[32];
  char crypto[32];
  sprintf (wep, "%s_wep", prefix);
  sprintf (crypto, "%s_crypto", prefix);
  /* BugBug - should we bail when mode is wep ? */
  if (nvram_match (wep, "wep") || nvram_match (wep, "on")
      || nvram_match (wep, "restricted") || nvram_match (wep, "enabled"))
    return "1";
  else if (nvram_match (crypto, "tkip"))
    return "2";
  else if (nvram_match (crypto, "aes"))
    return "4";
  else if (nvram_match (crypto, "tkip+aes"))
    return "6";
  else
    return "0";
}

char *
getAuthMode (char *prefix)
{
  char akm[32];
  sprintf (akm, "%s_akm", prefix);
  if (nvram_match (akm, "disabled") || nvram_get (akm) == NULL
      || strlen (nvram_safe_get (akm)) == 0)
    return NULL;
  if (nvram_match (akm, "radius"))
    return "32";
  else if (nvram_match (akm, "wpa"))
    return "2";
  else if (nvram_match (akm, "psk"))
    return "4";
  else if (nvram_match (akm, "psk2"))
    return "128";
  else if (nvram_match (akm, "psk psk2"))
    return "132";
  else if (nvram_match (akm, "wpa2"))
    return "64";
  else if (nvram_match (akm, "wpa wpa2"))
    return "66";
  else
    return "255";
}

char *
getKey (char *prefix)
{
  char akm[32];
  char psk[32];
  char radius[32];
  sprintf (akm, "%s_akm", prefix);
  sprintf (psk, "%s_wpa_psk", prefix);
  sprintf (radius, "%s_radius_key", prefix);
  if (nvram_match (akm, "wpa") || nvram_match (akm, "radius")
      || nvram_match (akm, "wpa2") || nvram_match (akm, "wpa wpa2"))
    return nvram_safe_get (radius);
  else if (nvram_match (akm, "psk") || nvram_match (akm, "psk2")
	   || nvram_match (akm, "psk psk2"))
    return nvram_safe_get (psk);
  else
    return "";
}

/*
static void start_nas_ap(char *prefix,char *type)
{
      char sec[32];
      sprintf(sec,"%s_security_mode",prefix);
      int i;
      for (i=0;i<strlen(sec);i++)
        if (sec[i]=='.')sec[i]='X';
	
      char *security_mode = nvram_safe_get (sec);

      if (strstr (security_mode, "psk") || strstr (security_mode, "wpa"))
        {
	char auth[32];
	sprintf(auth,"%s_auth",prefix);
	nvram_set (auth, "0");
	}
      convert_wds ();

      if (!type || !*type)
	{
	  if (nvram_match ("wl0_mode", "ap"))
	    type = "lan";
	  else
	    type = "wan";
	}

      snprintf (cfgfile, sizeof (cfgfile), "/tmp/nas.%s.conf", type);
      snprintf (pidfile, sizeof (pidfile), "/tmp/nas.%s.pid", type);

      {
	char *argv[] = { "/usr/sbin/nas", cfgfile, pidfile, type, NULL };
	pid_t pid;

	_eval (argv, NULL, 0, &pid);
	cprintf ("done\n");
      }
}
*/
void
start_nas_lan (void)
{
  start_nas ("lan", "wl0");
#ifdef HAVE_MSSID
  char *next;
  char var[80];
  char *vifs = nvram_safe_get ("wl0_vifs");
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      start_nas ("lan", var);
    }
#endif

}



void
start_nas_wan (void)
{
  start_nas ("wan", "wl0");
#ifdef HAVE_MSSID
  char *next;
  char var[80];
  char *vifs = nvram_safe_get ("wl0_vifs");
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      start_nas ("wan", var);
    }
#endif
}

int
start_nas (char *type, char *prefix)
{

  char cfgfile[64];
  char pidfile[64];
  char *auth_mode = "255";	/* -m N = WPA authorization mode (N = 0: none, 1: 802.1x, 2: WPA PSK, 255: disabled) */
  char *sec_mode = { 0 };	/* -w N = security mode bitmask  (N = 1: WEP, 2: TKIP, 4: AES) */
  char *key = { 0 }, *iface =
  {
  0}, *mode =
  {
  0};

  // Sveasoft 2003-12-15 only start if enabled
  /* if (!nvram_invmatch ("nas_enable", "0")
     || nvram_match ("security_mode", "wep")
     || nvram_match ("security_mode", "disabled"))
     {
     stop_nas ();
     return 0;
     }
   */
  convert_wds ();
  {

    snprintf (pidfile, sizeof (pidfile), "/tmp/nas.%s%s.pid", prefix, type);

    /* Sveasoft rewrite - start nas with explicit parameters */

    char apmode[32];
    sprintf (apmode, "%s_mode", prefix);

    if (!strcmp (type, "wan") && nvram_match (apmode, "ap"))
      {
	return 0;
      }

//    if (!strcmp (type, "lan"))
//      iface = "br0";
//    else

    if (0 == type || 0 == *type)
      type = "lan";
    if (!strcmp (type, "lan") && nvram_invmatch (apmode, "ap"))
      iface = "br0";
    else
      {

	if (!strcmp (prefix, "wl0"))
	  {
	    if (wl_probe ("eth2"))	// identify wireless interface
	      iface = "eth1";
	    else
	      iface = "eth2";
	  }
	else
	  {
	    iface = prefix;
	  }
      }

    sec_mode = getSecMode (prefix);
    auth_mode = getAuthMode (prefix);
    if (auth_mode == NULL)
      return 0;			//no nas required
    if (0 == strcmp (nvram_safe_get (apmode), "ap"))
      mode = "-A";
    else
      mode = "-S";

    char rekey[32];
    char ssid[32];
    char radius[32];
    char port[32];
    char index[32];

    sprintf (rekey, "%s_wpa_gtk_rekey", prefix);
    sprintf (ssid, "%s_ssid", prefix);
    sprintf (radius, "%s_radius_ipaddr", prefix);
    sprintf (port, "%s_radius_port", prefix);
    sprintf (index, "%s_key", prefix);

    key = getKey (prefix);

    {
      //char *argv[] = {"nas", "-P", pidfile, "-l", nvram_safe_get("lan_ifname"), "-H", "34954", "-i", iface, mode, "-m", auth_mode, "-k", key, "-s", nvram_safe_get("wl0_ssid"), "-w", sec_mode, "-g", nvram_safe_get("wl0_wpa_gtk_rekey"), "-h", nvram_safe_get("wl0_radius_ipaddr"), "-p", nvram_safe_get("wl0_radius_port"), NULL};
      pid_t pid;
      FILE *fp = { 0 };
      if (!strcmp (mode, "-S"))
	{
	  char *argv[] =
	    { "nas", "-P", pidfile, "-H", "34954", "-i", iface, mode, "-m",
	    auth_mode, "-k", key, "-s", nvram_safe_get (ssid), "-w",
	    sec_mode, "-g",
	    nvram_safe_get (rekey), NULL
	  };
	  _eval (argv, NULL, 0, &pid);
	}
      else
	{
	  if (!strcmp (auth_mode, "2") || !strcmp (auth_mode, "64")
	      || !strcmp (auth_mode, "66"))
	    {
	      char *argv[] = { "nas", "-P", pidfile, "-H", "34954", "-l",
		nvram_safe_get ("lan_ifname"), "-i", iface, mode, "-m",
		auth_mode, "-r", key, "-s", nvram_safe_get (ssid), "-w",
		sec_mode, "-g", nvram_safe_get (rekey), "-h",
		nvram_safe_get (radius), "-p", nvram_safe_get (port),	// "-t", //radius rekey time
		NULL
	      };
	      _eval (argv, NULL, 0, &pid);
	    }
	  else if (!strcmp (auth_mode, "32"))
	    {
	      int idx = atoi (nvram_safe_get (index));
	      char wepkey[32];
	      sprintf (wepkey, "%s_key%d", prefix, idx);

	      char *argv[] = { "nas", "-P", pidfile, "-H", "34954", "-l",
		nvram_safe_get ("lan_ifname"), "-i", iface, mode, "-m",
		auth_mode, "-r", key, "-s", nvram_safe_get (ssid), "-w",
		sec_mode, "-I", nvram_safe_get (index), "-k",
		nvram_safe_get (wepkey), "-h",
		nvram_safe_get (radius), "-p", nvram_safe_get (port),	// "-t", //radius rekey time
		NULL
	      };
	      _eval (argv, NULL, 0, &pid);

	    }
	  else
	    {

	      char *argv[] = { "nas", "-P", pidfile, "-H", "34954", "-l",
		nvram_safe_get ("lan_ifname"), "-i", iface, mode, "-m",
		auth_mode, "-k", key, "-s", nvram_safe_get (ssid), "-w",
		sec_mode, "-g",
		nvram_safe_get (rekey), NULL
	      };
	      _eval (argv, NULL, 0, &pid);
	    }


	}


      fp = fopen (pidfile, "w");
      if (fp)
	fprintf (fp, "%d", pid);
      fclose (fp);

      cprintf ("done\n");
    }
    return 0;
  }
}

int
stop_nas (void)
{
  /* NAS sometimes won't exit properly on a normal kill */
  //int ret = killps("nas",NULL);
  int ret = eval ("killall", "nas");
  sleep (2);
  //killps("nas","-9");
  eval ("killall", "-9", "nas");

  cprintf ("done\n");
  return ret;
}

#ifdef HAVE_SPUTNIK_APD
/* Sputnik APD Service Handling */
int
start_sputnik (void)
{
  int ret;

  // Only start if enabled
  if (!nvram_invmatch ("apd_enable", "0"))
    return 0;

  ret = eval ("sputnik");
  cprintf ("done\n");
  return ret;
}

int
stop_sputnik (void)
{
  int ret = eval ("killall", "sputnik");

  cprintf ("done\n");
  return ret;
}

/* END Sputnik Service Handling */

#endif
#if 0
int
start_ntpc (void)
{
  char *servers = nvram_safe_get ("ntp_server");

  // Sveasoft 2003-12-15 only start if enabled
  if (!nvram_invmatch ("ntpd_enable", "0"))
    return 0;

  if (strlen (servers))
    {
      char *nas_argv[] =
	{ "ntpclient", "-h", servers, "-i", "5", "-l", "-s", "-c", "2",
	NULL
      };
      pid_t pid;

      _eval (nas_argv, NULL, 0, &pid);
    }

  cprintf ("done\n");
  return 0;
}

int
stop_ntpc (void)
{
  //int ret = killps("ntpclient",NULL);
  int ret = eval ("killall", "ntpclient");

  cprintf ("done\n");
  return ret;
}
#endif


/////////////////////////////////////////////////////
int
start_resetbutton (void)
{
  int ret = 0;

  // Sveasoft 2003-12-15 only start if enabled
  if (!nvram_invmatch ("resetbutton_enable", "0"))
    return 0;

  ret = eval ("resetbutton");

  cprintf ("done\n");
  return ret;
}

int
stop_resetbutton (void)
{
  int ret = 0;

  //ret = killps("resetbutton","-9");
  ret = eval ("killall", "-9", "resetbutton");

  cprintf ("done\n");
  return ret;
}

int
start_iptqueue (void)
{
  int ret = 0;

  // Sveasoft 2003-12-15 only start if enabled
  if (!nvram_invmatch ("iptqueue_enable", "0"))
    return 0;

  ret = eval ("iptqueue");

  cprintf ("done\n");
  return ret;
}

int
stop_iptqueue (void)
{
  int ret = 0;

  //ret = killps("iptqueue","-9");
  ret = eval ("killall", "-9", "iptqueue");

  cprintf ("done\n");
  return ret;
}

#ifdef HAVE_TFTP
int
start_tftpd (void)
{
  int ret = 0;
  pid_t pid;
  char *tftpd_argv[] = { "tftpd",
    "-s", "/tmp",		// chroot to /tmp
    "-c",			// allow new files to be created
    "-l",			// standalone
    NULL
  };
#ifndef ANTI_FLASH
  ret = _eval (tftpd_argv, NULL, 0, &pid);
#endif
  cprintf ("done\n");
  return ret;
}

int
stop_tftpd (void)
{
  int ret;
#ifndef ANTI_FLASH
  //ret = killps("tftpd","-9");
  ret = eval ("killall", "-9", "tftpd");
#endif
  cprintf ("done\n");
  return ret;
}
#endif
int
start_cron (void)
{
  int ret = 0;
  struct stat buf;

  // Sveasoft 2003-12-15 only start if enabled
  if (nvram_match ("cron_enable", "0"))
    return 0;

  /* Create cron's database directory */
  if (stat ("/var/spool", &buf) != 0)
    {
      mkdir ("/var/spool", 0700);
      mkdir ("/var/spool/cron", 0700);
    }
  mkdir ("/tmp/cron.d", 0700);

  buf_to_file ("/tmp/cron.d/check_ps", "*/2 * * * * root /sbin/check_ps\n");
  cprintf ("starting cron\n");
  ret = eval ("/usr/sbin/cron");


  cprintf ("done\n");
  return ret;
}

int
stop_cron (void)
{
  int ret = 0;

  //ret = killps("cron","-9");
  ret = eval ("killall", "-9", "cron");

  cprintf ("done\n");
  return ret;
}

#ifdef HAVE_ZEBRA
int
zebra_init (void)
{
  if (nvram_match ("wk_mode", "gateway"))
    {
      printf ("zebra disabled.\n");
      return 0;
    }
  else if (nvram_match ("wk_mode", "ospf"))
    return zebra_ospf_init ();
  else if (nvram_match ("wk_mode", "router"))
    return zebra_ripd_init ();
  else
    return 0;
}


int
zebra_ospf_init (void)
{

  char *lt = nvram_safe_get ("dr_lan_tx");
  char *lr = nvram_safe_get ("dr_lan_rx");
  char *wt = nvram_safe_get ("dr_wan_tx");
  char *wr = nvram_safe_get ("dr_wan_rx");
  char *lf = nvram_safe_get ("lan_ifname");
  char *wf = get_wan_face ();

  FILE *fp;
  int ret1, ret2, s = 0;

//      printf("Start zebra\n");
  if (!strcmp (lt, "0") && !strcmp (lr, "0") &&
      !strcmp (wt, "0") && !strcmp (wr, "0"))
    {
      printf ("zebra disabled.\n");
      return 0;
    }

  /* Write configuration file based on current information */
  if (!(fp = fopen ("/tmp/zebra.conf", "w")))
    {
      perror ("/tmp/zebra.conf");
      return errno;
    }

  if (strlen (nvram_safe_get ("zebra_conf")) < 1
      || nvram_match ("zebra_copt", "1"))
    {
      if (nvram_match ("zebra_log", "1"))
	{
	  fprintf (fp, "log file /var/log/zebra.log\n");
	}
    }

  if (strlen (nvram_safe_get ("zebra_conf")) > 0)
    {
      fprintf (fp, "%s", nvram_safe_get ("zebra_conf"));
    }

  fclose (fp);

  if (!(fp = fopen ("/tmp/ospfd.conf", "w")))
    {
      perror ("/tmp/ospfd.conf");
      return errno;
    }

  if (strlen (nvram_safe_get ("ospfd_conf")) < 1
      || nvram_match ("ospfd_copt", "1"))
    {
      fprintf (fp, "!\n");
      fprintf (fp, "password %s\n", nvram_safe_get ("http_passwd"));
      fprintf (fp, "enable password %s\n", nvram_safe_get ("http_passwd"));
      fprintf (fp, "!\n!\n!\n");

      fprintf (fp, "interface %s\n!\n", lf);
      fprintf (fp, "interface %s\n", wf);
      fprintf (fp, "passive interface lo\n");

      if (nvram_match ("wl_br1_enable", "1"))
	{
	  fprintf (fp, "!\n! 'Subnet' WDS bridge\n");
	  fprintf (fp, "interface br1\n");
	}

      if (nvram_match ("wl0_mode", "ap"))
	for (s = 1; s <= MAX_WDS_DEVS; s++)
	  {
	    char wdsvarname[32] = { 0 };
	    char wdsdevname[32] = { 0 };
	    char wdsdevlabel[32] = { 0 };
	    char wdsdevospf[32] = { 0 };
	    char *dev;

	    sprintf (wdsvarname, "wl_wds%d_enable", s);
	    sprintf (wdsdevname, "wl_wds%d_if", s);
	    sprintf (wdsdevlabel, "wl_wds%d_desc", s);
	    sprintf (wdsdevospf, "wl_wds%d_ospf", s);
	    dev = nvram_safe_get (wdsdevname);

	    if (nvram_match (wdsvarname, "1"))
	      {
		fprintf (fp, "!\n! WDS: %s\n", nvram_safe_get (wdsdevlabel));
		fprintf (fp, "interface %s\n", dev);

		if (atoi (nvram_safe_get (wdsdevospf)) > 0)
		  fprintf (fp, " ip ospf cost %s\n",
			   nvram_safe_get (wdsdevospf));
	      }
	  }

      fprintf (fp, "!\n");
      fprintf (fp, "router ospf\n");
      fprintf (fp, " ospf router-id %s\n", nvram_safe_get ("lan_ipaddr"));
      fprintf (fp, " redistribute kernel\n");
      fprintf (fp, " redistribute connected\n");
      fprintf (fp, " redistribute static\n");
      fprintf (fp, " network 0.0.0.0/0 area 0\n");	// handle all routing
      fprintf (fp, " default-information originate\n");

      for (s = 1; s <= MAX_WDS_DEVS; s++)
	{
	  char wdsdevospf[32] = { 0 };
	  char wdsdevname[32] = { 0 };

	  sprintf (wdsdevname, "wl_wds%d_if", s);
	  sprintf (wdsdevospf, "wl_wds%d_ospf", s);

	  if (atoi (nvram_safe_get (wdsdevospf)) < 0)
	    fprintf (fp, " passive-interface %s\n",
		     nvram_safe_get (wdsdevname));
	}

      if (nvram_match ("zebra_log", "1"))
	{
	  fprintf (fp, "!\n");
	  fprintf (fp, "log file /var/log/ospf.log\n");
	}

      fprintf (fp, "!\nline vty\n!\n");
    }

  if (strlen (nvram_safe_get ("ospfd_conf")) > 0)
    {
      fprintf (fp, "%s", nvram_safe_get ("ospfd_conf"));
    }

  fflush (fp);
  fclose (fp);

  if (nvram_match ("dyn_default", "1"))
    while (!eval ("/usr/sbin/ip", "route", "del", "default"))
      ;

  ret1 = eval ("/usr/sbin/zebra", "-d", "-f", "/tmp/zebra.conf");
  ret2 = eval ("/usr/sbin/ospfd", "-d", "-f", "/tmp/ospfd.conf");

  return ret1 + ret2;
}

int
zebra_ripd_init (void)
{

  char *lt = nvram_safe_get ("dr_lan_tx");
  char *lr = nvram_safe_get ("dr_lan_rx");
  char *wt = nvram_safe_get ("dr_wan_tx");
  char *wr = nvram_safe_get ("dr_wan_rx");
  char *lf = nvram_safe_get ("lan_ifname");
  char *wf = get_wan_face ();

  FILE *fp;
  int ret1, ret2;

//      printf("Start zebra\n");
  if (!strcmp (lt, "0") && !strcmp (lr, "0") &&
      !strcmp (wt, "0") && !strcmp (wr, "0"))
    {
      printf ("zebra disabled.\n");
      return 0;
    }

  /* Write configuration file based on current information */
  if (!(fp = fopen ("/tmp/zebra.conf", "w")))
    {
      perror ("/tmp/zebra.conf");
      return errno;
    }
  fclose (fp);

  if (!(fp = fopen ("/tmp/ripd.conf", "w")))
    {
      perror ("/tmp/ripd.conf");
      return errno;
    }
  fprintf (fp, "router rip\n");
  fprintf (fp, "  network %s\n", lf);
  fprintf (fp, "  network %s\n", wf);
  fprintf (fp, "redistribute connected\n");
  //fprintf(fp, "redistribute kernel\n");
  //fprintf(fp, "redistribute static\n");

  fprintf (fp, "interface %s\n", lf);
  if (strcmp (lt, "0") != 0)
    fprintf (fp, "  ip rip send version %s\n", lt);
  if (strcmp (lr, "0") != 0)
    fprintf (fp, "  ip rip receive version %s\n", lr);

  fprintf (fp, "interface %s\n", wf);
  if (strcmp (wt, "0") != 0)
    fprintf (fp, "  ip rip send version %s\n", wt);
  if (strcmp (wr, "0") != 0)
    fprintf (fp, "  ip rip receive version %s\n", wr);

  fprintf (fp, "router rip\n");
  if (strcmp (lt, "0") == 0)
    fprintf (fp, "  distribute-list private out %s\n", lf);
  if (strcmp (lr, "0") == 0)
    fprintf (fp, "  distribute-list private in  %s\n", lf);
  if (strcmp (wt, "0") == 0)
    fprintf (fp, "  distribute-list private out %s\n", wf);
  if (strcmp (wr, "0") == 0)
    fprintf (fp, "  distribute-list private in  %s\n", wf);
  fprintf (fp, "access-list private deny any\n");

  //fprintf(fp, "debug rip events\n");
  //fprintf(fp, "log file /tmp/ripd.log\n");
  fflush (fp);
  fclose (fp);

  ret1 = eval ("zebra", "-d", "-f", "/tmp/zebra.conf");
  ret2 = eval ("ripd", "-d", "-f", "/tmp/ripd.conf");

  return ret1 + ret2;
}
#endif

#ifdef HAVE_BIRD
int
bird_init (void)
{
  FILE *fp;
  int ret1;
  /*
     compatibitly for old nvram style (site needs to be enhanced)
   */
  if (nvram_match ("wk_mode", "gateway"))
    return 0;
  nvram_set ("routing_ospf", "off");
  nvram_set ("routing_bgp", "off");
  nvram_set ("routing_rip2", "off");

  if (nvram_match ("wk_mode", "ospf"))
    nvram_set ("routing_ospf", "on");
  if (nvram_match ("wk_mode", "router"))
    nvram_set ("routing_rip2", "on");
  if (nvram_match ("wk_mode", "bgp"))
    nvram_set ("routing_bgp", "on");

  if (nvram_match ("dr_setting", "1"))
    {
      nvram_set ("routing_wan", "on");
      nvram_set ("routing_lan", "off");
    }
  if (nvram_match ("dr_setting", "2"))
    {
      nvram_set ("routing_wan", "off");
      nvram_set ("routing_lan", "on");
    }
  if (nvram_match ("dr_setting", "3"))
    {
      nvram_set ("routing_wan", "on");
      nvram_set ("routing_lan", "on");
    }
  if (nvram_match ("dr_setting", "0"))
    {
      nvram_set ("routing_wan", "off");
      nvram_set ("routing_lan", "off");
    }

  // DD-WRT bird support 
  if (nvram_match ("routing_rip2", "on") ||
      nvram_match ("routing_ospf", "on") || nvram_match ("routing_bgp", "on"))
    {
      mkdir ("/tmp/bird", 0744);
      if (!(fp = fopen ("/tmp/bird/bird.conf", "w")))
	{
	  perror ("/tmp/bird/bird.conf");
	  return errno;
	}
      fprintf (fp, "router id %s;\n", nvram_safe_get ("lan_ipaddr"));
      fprintf (fp,
	       "protocol kernel { learn; persist; scan time 10; import all; export all; }\n");
      fprintf (fp, "protocol device { scan time 10; } \n");
      fprintf (fp, "protocol direct { interface \"*\";}\n");

      if (nvram_match ("routing_rip2", "on"))
	{

	  fprintf (fp, "protocol rip WRT54G_rip {\n");
	  if (nvram_match ("routing_lan", "on"))
	    fprintf (fp, "	interface \"%s\" { };\n",
		     nvram_safe_get ("lan_ifname"));
	  if (nvram_match ("routing_wan", "on"))
	    {
	      if (nvram_match ("wl_mode", "sta")
		  || nvram_match ("wl_mode", "apsta"))
		fprintf (fp, "	interface \"%s\" { };\n", get_wdev ());
	      else
		fprintf (fp, "	interface \"%s\" { };\n",
			 nvram_safe_get ("wan_ifname"));

	    }
	  fprintf (fp, "	honor always;\n");
	  fprintf (fp,
		   "	import filter { print \"importing\"; accept; };\n");
	  fprintf (fp,
		   "	export filter { print \"exporting\"; accept; };\n");
	  fprintf (fp, "}\n");

	}
      if (nvram_match ("routing_ospf", "on"))
	{
	  fprintf (fp, "protocol ospf WRT54G_ospf {\n");
	  fprintf (fp, "area 0 {\n");
	  if (nvram_match ("routing_wan", "on"))
	    fprintf (fp,
		     "interface \"%s\" { cost 1; authentication simple; password \"%s\"; };\n",
		     nvram_safe_get ("wan_ifname"),
		     nvram_safe_get ("http_passwd"));
	  if (nvram_match ("routing_lan", "on"))
	    fprintf (fp,
		     "interface \"%s\" { cost 1; authentication simple; password \"%s\"; };\n",
		     nvram_safe_get ("lan_ifname"),
		     nvram_safe_get ("http_passwd"));
	  fprintf (fp, "};\n}\n");
	}			// if wk_mode = ospf

      if (nvram_match ("routing_bgp", "on"))
	{
	  fprintf (fp, "protocol bgp {\n");
	  fprintf (fp, "local as %s;\n", nvram_safe_get ("routing_bgp_as"));
	  fprintf (fp, "neighbor %s as %s;\n",
		   nvram_safe_get ("routing_bgp_neighbor_ip"),
		   nvram_safe_get ("routing_bgp_neighbor_as"));
	  fprintf (fp, "export all;\n");
	  fprintf (fp, "import all;\n");
	  fprintf (fp, "}\n");
	}
      fflush (fp);
      fclose (fp);

      ret1 = eval ("bird", "-c", "/tmp/bird/bird.conf");
    }
  return 0;

}
#endif /* HAVE_BIRD */
#ifdef HAVE_BIRD
/* Written by Sparq in 2002/07/16 */
int
start_zebra (void)
{

  // Sveasoft 2003-12-15 only start if enabled
  if (!nvram_invmatch ("zebra_enable", "0"))
    return 0;
//      else if ( nvram_invmatch("wk_mode", "ospf") &&
//                !strcmp(lt, "0") && !strcmp(lr, "0") &&
//                !strcmp(wt, "0") && !strcmp(wr, "0") ){
//              printf("routing disabled.\n");
//              return 0;
//      }

#ifdef HAVE_BIRD

  if (bird_init () != 0)
    return -1;

#elif defined(HAVE_ZEBRA)

  if (zebra_init () != 0)
    return -1;

#endif /* HAVE_BIRD */
  return 0;
}

/* Written by Sparq in 2002/07/16 */
int
stop_zebra (void)
{
  int ret1;

#ifdef HAVE_ZEBRA
  int ret2, ret3;
  ret1 = eval ("killall", "zebra");
  ret2 = eval ("killall", "ripd");
  ret3 = eval ("killall", "ospfd");

  while (!
	 (eval ("killall", "zebra") && eval ("killall", "ripd")
	  && eval ("killall", "ospfd")))
    sleep (1);

  cprintf ("done\n");
  return ret1 | ret2 | ret3;

#elif defined(HAVE_BIRD)

  ret1 = eval ("killall", "bird");

  cprintf ("done\n");
  return ret1;
#else
  return -1;
#endif
}

#endif

int
start_syslog (void)
{
  int ret1 = 0, ret2 = 0;

  // Sveasoft 2003-12-15 only start if enabled
  if (!nvram_invmatch ("syslogd_enable", "0"))
    return 0;

  if (strlen (nvram_safe_get ("syslogd_rem_ip")) > 0)
    ret1 = eval ("/sbin/syslogd", "-R", nvram_safe_get ("syslogd_rem_ip"));
  else
    ret1 = eval ("/sbin/syslogd", "-L");

  ret2 = eval ("/sbin/klogd");

  return ret1 | ret2;
}

int
stop_syslog (void)
{
  int ret;

  ret = eval ("killall", "-9", "klogd");
  ret += eval ("killall", "-9", "syslogd");

  cprintf ("done\n");
  return ret;
}


int
start_redial (void)
{
  int ret;
  pid_t pid;
  char *redial_argv[] = { "/tmp/ppp/redial",
    nvram_safe_get ("ppp_redialperiod"),
    NULL
  };

  symlink ("/sbin/rc", "/tmp/ppp/redial");

  ret = _eval (redial_argv, NULL, 0, &pid);

  cprintf ("done\n");
  return ret;
}

int
stop_redial (void)
{
  int ret;

  //ret = killps("redial","-9");
  ret = eval ("killall", "-9", "redial");

  cprintf ("done\n");
  return ret;
}

#ifdef HAVE_RADVD
int
start_radvd (void)
{
  int ret = 0;
  int c = 0;
  char *buf, *buf2;
  int i;
  FILE *fp;
  if (!nvram_match ("radvd_enable", "1"))
    return 0;
  if (!nvram_match ("ipv6_enable", "1"))
    return 0;
  buf = nvram_safe_get ("radvd_conf");
  if (buf != NULL)
    {
      buf2 = (char *) malloc (strlen (buf) + 1);
      memcpy (buf2, buf, strlen (buf));
      buf2[strlen (buf)] = 0;

      i = 0;
      while (buf2[i++] != 0)
	{
	  cprintf (".");
	  if (buf2[i - 1] == '\r')
	    continue;
	  buf2[c++] = buf2[i - 1];
	}
      buf2[c++] = 0;
      fp = fopen ("/tmp/radvd.conf", "wb");
      fwrite (buf2, 1, c - 1, fp);
      fclose (fp);
      free (buf2);
    }
  //nvram2file("radvd_conf", "/tmp/radvd.conf");

  system ("sync");

  ret = eval ("/sbin/radvd");

  cprintf ("done\n");
  return ret;
}

int
stop_radvd (void)
{
  int ret = 0;

  //ret = killps("radvd",NULL);
  ret = eval ("killall", "radvd");

  unlink ("/var/run/radvd.pid");

  cprintf ("done\n");
  return ret;
}
#endif

int
start_ipv6 (void)
{
  int ret = 0;

  if (!nvram_invmatch ("ipv6_enable", "0"))
    return 0;

  ret = eval ("insmod", "ipv6");

  cprintf ("done\n");
  return ret;
}

#ifdef HAVE_CHILLI

int
start_chilli (void)
{
  int ret = 0;
  FILE *fp;
  int i;
  if (!nvram_match ("chilli_enable", "1"))
    return 0;

#ifdef HAVE_CHILLILOCAL

  if (!(fp = fopen ("/tmp/fonusers.local", "w")))
    {
      perror ("/tmp/fonusers.local");
      return errno;
    }
  char *users = nvram_safe_get ("fon_userlist");
  char *u = (char *) malloc (strlen (users) + 1);
  char *o = u;
  strcpy (u, users);
  char *sep = strsep (&u, "=");
  while (sep != NULL)
    {
      fprintf (fp, "%s ", sep);
      char *pass = strsep (&u, " ");
      fprintf (fp, "%s \n", pass != NULL ? pass : "");
      sep = strsep (&u, "=");
    }
  free (o);
  fclose (fp);
#endif


  if (!(fp = fopen ("/tmp/chilli.conf", "w")))
    {
      perror ("/tmp/chilli.conf");
      return errno;
    }

  fprintf (fp, "radiusserver1 %s\n", nvram_get ("chilli_radius"));
  fprintf (fp, "radiusserver2 %s\n", nvram_get ("chilli_backup"));
  fprintf (fp, "radiussecret %s\n", nvram_get ("chilli_pass"));
  if (nvram_match ("chilli_interface", "wlan")
      || nvram_match ("chilli_interface", "wan"))
    {
#ifdef HAVE_MADWIFI
      if (nvram_match("ath0_mode","ap"))
	fprintf (fp, "dhcpif ath0\n");
      else
	fprintf (fp, "dhcpif ath1\n");
#else
#ifndef HAVE_MSSID
      if (wl_probe ("eth2"))
	fprintf (fp, "dhcpif eth1\n");
      else
	fprintf (fp, "dhcpif eth2\n");
#else
      if (nvram_match ("wl0_mode", "apsta"))
	{
	  fprintf (fp, "dhcpif wl0.1\n");
	}
      else
	{
	  if (wl_probe ("eth2"))
	    fprintf (fp, "dhcpif eth1\n");
	  else
	    fprintf (fp, "dhcpif eth2\n");
	}
#endif
#endif
    }
  else
    {
      if (nvram_match ("chilli_interface", "wanwlan"))
	{
	  fprintf (fp, "dhcpif br0\n");
	}
      else
	{
	
	  fprintf (fp, "dhcpif vlan0\n");
	}
    }

  fprintf (fp, "uamserver %s\n", nvram_get ("chilli_url"));
  if (nvram_invmatch ("chilli_dns1", "0.0.0.0")
      && nvram_invmatch ("chilli_dns1", ""))
    {
      fprintf (fp, "dns1 %s\n", nvram_get ("chilli_dns1"));
      if (nvram_invmatch ("sv_localdns", "0.0.0.0")
	  && nvram_invmatch ("sv_localdns", ""))
	fprintf (fp, "dns2 %s\n", nvram_get ("sv_localdns"));
    }
  else if (nvram_invmatch ("sv_localdns", "0.0.0.0")
	   && nvram_invmatch ("sv_localdns", ""))
    fprintf (fp, "dns1 %s\n", nvram_get ("sv_localdns"));

  if (nvram_invmatch ("chilli_uamsecret", ""))
    fprintf (fp, "uamsecret %s\n", nvram_get ("chilli_uamsecret"));
  if (nvram_invmatch ("chilli_uamanydns", "0"))
    fprintf (fp, "uamanydns\n");
  if (nvram_invmatch ("chilli_uamallowed", ""))
    fprintf (fp, "uamallowed %s\n", nvram_get ("chilli_uamallowed"));
  if (nvram_match ("chilli_macauth", "1"))
    fprintf (fp, "macauth\n");
#ifndef HAVE_FON
  if (nvram_match ("fon_enable", "1"))
    {
#endif
      char hyp[32];
      strcpy (hyp, nvram_safe_get ("wl0_hwaddr"));
      for (i = 0; i < strlen (hyp); i++)
	if (hyp[i] == ':')
	  hyp[i] = '-';
      if (i > 0)
	fprintf (fp, "radiusnasid %s\n", hyp);
      nvram_set ("chilli_radiusnasid", hyp);
      fprintf (fp, "interval 300\n");
#ifndef HAVE_FON
    }
  else
    {
      if (nvram_invmatch ("chilli_radiusnasid", ""))
	fprintf (fp, "radiusnasid %s\n", nvram_get ("chilli_radiusnasid"));
    }
#endif

  if (nvram_invmatch ("chilli_additional", ""))
    {
      char *add = nvram_safe_get ("chilli_additional");
      i = 0;
      do
	{
	  if (add[i] != 0x0D)
	    fprintf (fp, "%c", add[i]);
	}
      while (add[++i]);
      i = 0;
      int a = 0;
      char *filter = strdup (add);
      do
	{
	  if (add[i] != 0x0D)
	    filter[a++] = add[i];
	}
      while (add[++i]);

      filter[a] = 0;
      if (strcmp (filter, add))
	{
	  nvram_set ("chilli_additional", filter);
	  nvram_commit ();
	}
      free (filter);
    }
  fflush (fp);
  fclose (fp);
/*  if (nvram_match ("ntp_enable", "1"))
  {
  if (time(0)<1000)
    {
    sleep(10); // wait for ntp connectivity
    }
  }
*/
  ret = eval ("/usr/sbin/chilli", "-c", "/tmp/chilli.conf");

  cprintf ("done\n");
  return ret;
}

int
stop_chilli (void)
{
  int ret = 0;

  //ret = killps("chilli","-9");
  ret = eval ("killall", "-9", "chilli");

  cprintf ("done\n");
  return ret;
}

#endif /* HAVE_CHILLI */
#ifdef HAVE_PPPOE
int
stop_pppoe (void)
{
  int ret;

  unlink ("/tmp/ppp/link");
  //ret = killps("pppoecd",NULL);
  //ret += killps("ip-up",NULL);
  //ret += killps("ip-down",NULL);
  ret = eval ("killall", "pppoecd");
  ret += eval ("killall", "ip-up");
  ret += eval ("killall", "ip-down");

  cprintf ("done\n");
  return ret;
}

int
stop_single_pppoe(int pppoe_num)
{
        int ret;
        char pppoe_pid[15], pppoe_ifname[15];
        char ppp_unlink[2][20]={"/tmp/ppp/link","/tmp/ppp/link_1"};
        char ppp_wan_dns[2][20]={"wan_get_dns","wan_get_dns_1"};
                                                                                                                             
        sprintf(pppoe_pid,"pppoe_pid%d",pppoe_num);
        sprintf(pppoe_ifname,"pppoe_ifname%d",pppoe_num);
        dprintf("start! stop pppoe %d, pid %s \n",pppoe_num,nvram_safe_get(pppoe_pid));
                                                                                                                             
        ret = eval("kill",nvram_safe_get(pppoe_pid));
        unlink(ppp_unlink[pppoe_num]);
        nvram_unset(pppoe_ifname);
                                                                                                                             
        nvram_set(ppp_wan_dns[pppoe_num],"");
        stop_dns_clear_resolv();
                                                                                                                             
        dprintf("done\n");
        return ret ;
}
#endif
int
stop_dhcpc (void)
{
  int ret = 0;

  //ret += killps("udhcpc",NULL);
  ret += eval ("killall", "udhcpc");

  cprintf ("done\n");
  return ret;
}

#ifdef HAVE_PPTP
int
start_pptp (int status)
{
  int ret;
  FILE *fp;
  char *pptp_argv[] = { "pppd",
    NULL
  };
  char username[80], passwd[80];

  // Sveasoft 2003-12-15 only start if enabled
  /* if (!nvram_invmatch("pppd_enable", "0"))
     return 0; */

  stop_dhcpc ();
#ifdef HAVE_PPPOE
  stop_pppoe ();
#endif
  stop_vpn_modules ();

  if (nvram_match ("aol_block_traffic", "0"))
    {
      snprintf (username, sizeof (username), "%s",
		nvram_safe_get ("ppp_username"));
      snprintf (passwd, sizeof (passwd), "%s", nvram_safe_get ("ppp_passwd"));
    }
  else
    {
      if (!strcmp (nvram_safe_get ("aol_username"), ""))
	{
	  snprintf (username, sizeof (username), "%s",
		    nvram_safe_get ("ppp_username"));
	  snprintf (passwd, sizeof (passwd), "%s",
		    nvram_safe_get ("ppp_passwd"));
	}
      else
	{
	  snprintf (username, sizeof (username), "%s",
		    nvram_safe_get ("aol_username"));
	  snprintf (passwd, sizeof (passwd), "%s",
		    nvram_safe_get ("aol_passwd"));
	}
    }

  if (status != REDIAL)
    {
      mkdir ("/tmp/ppp", 0777);
      symlink ("/sbin/rc", "/tmp/ppp/ip-up");
      symlink ("/sbin/rc", "/tmp/ppp/ip-down");
      symlink ("/dev/null", "/tmp/ppp/connect-errors");

      /* Generate options file */
      if (!(fp = fopen ("/tmp/ppp/options", "w")))
	{
	  perror ("/tmp/ppp/options");
	  return -1;
	}
      fprintf (fp, "defaultroute\n");	//Add a default route to the system routing tables, using the peer as the gateway
      fprintf (fp, "usepeerdns\n");	//Ask the peer for up to 2 DNS server addresses
      fprintf (fp, "pty 'pptp %s --nolaunchpppd",
	       nvram_safe_get ("pptp_server_ip"));

      // PPTP client also supports synchronous mode.
      // This should improve the speeds.
      if (nvram_match ("pptp_synchronous", "1"))
	fprintf (fp, " --sync'\nsync\n");
      else
	fprintf (fp, "'\n");

      fprintf (fp, "user '%s'\n", username);
      //fprintf(fp, "persist\n");        // Do not exit after a connection is terminated.

      fprintf (fp, "mtu %s\n", nvram_safe_get ("wan_mtu"));

      if (nvram_match ("ppp_demand", "1"))
	{			//demand mode
	  fprintf (fp, "idle %d\n",
		   nvram_match ("ppp_demand",
				"1") ? atoi (nvram_safe_get ("ppp_idletime"))
		   * 60 : 0);
	  fprintf (fp, "demand\n");	// Dial on demand
	  fprintf (fp, "persist\n");	// Do not exit after a connection is terminated.
	  fprintf (fp, "%s:%s\n", PPP_PSEUDO_IP, PPP_PSEUDO_GW);	// <local IP>:<remote IP>
	  fprintf (fp, "ipcp-accept-remote\n");
	  fprintf (fp, "ipcp-accept-local\n");
	  fprintf (fp, "connect true\n");
	  fprintf (fp, "noipdefault\n");	// Disables  the  default  behaviour when no local IP address is specified
	  fprintf (fp, "ktune\n");	// Set /proc/sys/net/ipv4/ip_dynaddr to 1 in demand mode if the local address changes
	}
      else
	{			// keepalive mode
	  start_redial ();
	}
      if (nvram_match ("pptp_encrypt", "0"))
	fprintf (fp, "nomppe\n");	// Disable mppe negotiation
      fprintf (fp, "default-asyncmap\n");	// Disable  asyncmap  negotiation
      fprintf (fp, "nopcomp\n");	// Disable protocol field compression
      fprintf (fp, "noaccomp\n");	// Disable Address/Control compression
      fprintf (fp, "noccp\n");	// Disable CCP (Compression Control Protocol)
      fprintf (fp, "novj\n");	// Disable Van Jacobson style TCP/IP header compression
      fprintf (fp, "nobsdcomp\n");	// Disables BSD-Compress  compression
      fprintf (fp, "nodeflate\n");	// Disables Deflate compression
      fprintf (fp, "lcp-echo-interval 0\n");	// Don't send an LCP echo-request frame to the peer
      fprintf (fp, "lock\n");
      fprintf (fp, "noauth");

      if (nvram_invmatch ("pptp_extraoptions", ""))
	fprintf (fp, "%s\n", nvram_safe_get ("pptp_extraoptions"));

      fclose (fp);

      /* Generate pap-secrets file */
      if (!(fp = fopen ("/tmp/ppp/pap-secrets", "w")))
	{
	  perror ("/tmp/ppp/pap-secrets");
	  return -1;
	}
      fprintf (fp, "\"%s\" * \"%s\" *\n", username, passwd);
      fclose (fp);
      chmod ("/tmp/ppp/pap-secrets", 0600);

      /* Generate chap-secrets file */
      if (!(fp = fopen ("/tmp/ppp/chap-secrets", "w")))
	{
	  perror ("/tmp/ppp/chap-secrets");
	  return -1;
	}
      fprintf (fp, "\"%s\" * \"%s\" *\n", username, passwd);
      fclose (fp);
      chmod ("/tmp/ppp/chap-secrets", 0600);

      /* Enable Forwarding */
      if ((fp = fopen ("/proc/sys/net/ipv4/ip_forward", "r+")))
	{
	  fputc ('1', fp);
	  fclose (fp);
	}
      else
	perror ("/proc/sys/net/ipv4/ip_forward");
    }


  /* Bring up  WAN interface */
  if (nvram_match ("pptp_use_dhcp", "1"))
    {
//      pid_t pid;
      char *wan_ipaddr;
      char *wan_netmask;
      char *wan_gateway;

      char *pptp_server_ip = nvram_safe_get ("pptp_server_ip");
//      char *wan_hostname = nvram_safe_get ("wan_hostname");
      char *wan_ifname = nvram_safe_get ("wan_ifname");


      start_dhcpc (wan_ifname);
      int timeout;

      for (timeout = 10; nvram_match ("wan_get_dns", "") && timeout > 0;
	   --timeout)
	{			/* wait for info from dhcp server */
	  sleep (1);
	}
      stop_dhcpc ();		/* we don't need dhcp client anymore */

      wan_ipaddr = nvram_safe_get ("wan_ipaddr");	/* store current (dhcp) wan parameters */
      wan_netmask = nvram_safe_get ("wan_netmask");
      wan_gateway = nvram_safe_get ("wan_gateway");
      pptp_server_ip = nvram_safe_get ("pptp_server_ip");

      while (route_del (wan_ifname, 0, NULL, NULL, NULL) == 0);	/* Delete all wan routes */

      /* up wan interface */
      for (timeout = 10;
	   ifconfig (wan_ifname, IFUP, wan_ipaddr, wan_netmask)
	   && timeout > 0; --timeout)
	{
	  sleep (1);
	}
      /* add route to pptp server */
      for (timeout = 10;
	   route_add (wan_ifname, 0, pptp_server_ip, wan_gateway,
		      "255.255.255.255") && timeout > 0; --timeout)
	{
	  sleep (1);
	}
    }
  else
    {
      ifconfig (nvram_safe_get ("wan_ifname"), IFUP,
		nvram_safe_get ("wan_ipaddr"),
		nvram_safe_get ("wan_netmask"));
    }

  ret = _eval (pptp_argv, NULL, 0, NULL);


/*	if(nvram_match("pptp_usedhcp", "1")){
                char *wan_hostname = nvram_get("wan_hostname");
		char *dhcp_argv[] = { "udhcpc",
				      "-i", nvram_safe_get("wan_ifname"),
				      "-p", "/var/run/udhcpc.pid",
				      "-s", "/tmp/udhcpc",
				      wan_hostname && *wan_hostname ? "-H" : NULL,
				      wan_hostname && *wan_hostname ? wan_hostname : NULL,
				      NULL
                };

		ifconfig(nvram_safe_get("wan_ifname"), IFUP, NULL, NULL);
    		
		symlink("/sbin/rc", "/tmp/udhcpc");
    		nvram_set("wan_get_dns","");
		//killps("udhcpc",NULL);
		
		eval("killall","udhcpc");
    		
		_eval(dhcp_argv, NULL, 0, &pid);

		// Give enough time for DHCP to get IP address.
		sleep(2);

	} else
	    ifconfig(nvram_safe_get("wan_ifname"), IFUP, nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));

	// Start pptp client on wan interface
	ret = _eval(pptp_argv, NULL, 0, NULL);
*/
  if (nvram_match ("ppp_demand", "1"))
    {
      /* Trigger Connect On Demand if user press Connect button in Status page */
      if (nvram_match ("action_service", "start_pptp")
	  || nvram_match ("action_service", "start_l2tp"))
	{
	  start_force_to_dial ();
//                      force_to_dial(nvram_safe_get("action_service"));
	  nvram_set ("action_service", "");
	}
      /* Trigger Connect On Demand if user ping pptp server */
      else
	eval ("listen", nvram_safe_get ("lan_ifname"));
    }

  /* Sveasoft - make sure QoS comes up after pptp pppo device */
  start_wshaper ();

  cprintf ("done\n");
  return ret;
}

int
stop_pptp (void)
{
  int ret;
  route_del (nvram_safe_get ("wan_ifname"), 0,
	     nvram_safe_get ("pptp_server_ip"), NULL, NULL);

  unlink ("/tmp/ppp/link");
  //ret = killps("pppd","-9");
  //ret += killps("pptp","-9");
  //ret += killps("listen","-9");
  ret = eval ("killall", "-9", "pppd");
  ret += eval ("killall", "-9", "pptp");
  ret += eval ("killall", "-9", "listen");

  cprintf ("done\n");
  return ret;
}

#endif

//=========================================tallest============================================
/* AhMan  March 18 2005   Start the Original Linksys PPPoE */
/*
 * This function build the pppoe instuction & execute it.
 */
#ifdef HAVE_PPPOE
int
start_pppoe (int pppoe_num)
{
  char idletime[20], retry_num[20], param[4];
  char username[80], passwd[80];

  char ppp_username[2][20] = { "ppp_username", "ppp_username_1" };
  char ppp_passwd[2][20] = { "ppp_passwd", "ppp_passwd_1" };
  char ppp_demand[2][20] = { "ppp_demand", "ppp_demand_1" };
  char ppp_service[2][20] = { "ppp_service", "ppp_service_1" };
  char ppp_ac[2][10] = { "ppp_ac", "ppp_ac_1" };
//  char wanip[2][15] = { "wan_ipaddr", "wan_ipaddr_1" };
//  char wanmask[2][15] = { "wan_netmask", "wan_netmask_1" };
//  char wangw[2][15] = { "wan_gateway", "wan_gateway_1" };
  char pppoeifname[15];
  char *wan_ifname = nvram_safe_get ("wan_ifname");
  if (isClient ())
    {
      wan_ifname = get_wdev ();
    }

  pid_t pid;

  sprintf (pppoeifname, "pppoe_ifname%d", pppoe_num);
  nvram_set (pppoeifname, "");

  cprintf ("start session %d\n", pppoe_num);
  sprintf (idletime, "%d", atoi (nvram_safe_get ("ppp_idletime")) * 60);
  snprintf (retry_num, sizeof (retry_num), "%d",
	    (atoi (nvram_safe_get ("ppp_redialperiod")) / 5) - 1);

  if (nvram_match ("aol_block_traffic", "1") && pppoe_num == PPPOE0)
    {
      if (!strcmp (nvram_safe_get ("aol_username"), ""))
	{
	  snprintf (username, sizeof (username), "%s",
		    nvram_safe_get ("ppp_username"));
	  snprintf (passwd, sizeof (passwd), "%s",
		    nvram_safe_get ("ppp_passwd"));
	}
      else
	{
	  snprintf (username, sizeof (username), "%s",
		    nvram_safe_get ("aol_username"));
	  snprintf (passwd, sizeof (passwd), "%s",
		    nvram_safe_get ("aol_passwd"));
	}

    }
  else
    {
      snprintf (username, sizeof (username), "%s",
		nvram_safe_get (ppp_username[pppoe_num]));
      snprintf (passwd, sizeof (passwd), "%s",
		nvram_safe_get (ppp_passwd[pppoe_num]));
    }
  sprintf (param, "%d", pppoe_num);
  /* add here */
  char *pppoe_argv[] = { "pppoecd",
    wan_ifname,
    "-u", username,
    "-p", passwd,
    "-r", nvram_safe_get ("wan_mtu"),	//del by honor, add by tallest.
    "-t", nvram_safe_get ("wan_mtu"),
    "-i", nvram_match (ppp_demand[pppoe_num], "1") ? idletime : "0",
    "-I", "30",			// Send an LCP echo-request frame to the server every 30 seconds
    "-T", "3",			// pppd will presume the server to be dead if 5 LCP echo-requests are sent without receiving a valid LCP echo-reply
    "-P", param,		// PPPOE session number.
    "-N", retry_num,		// To avoid kill pppd when pppd has been connecting.
#if LOG_PPPOE == 2
    "-d",
#endif
    "-C", "disconnected_pppoe",	//by tallest 0407
    NULL,			/* set default route */
    NULL, NULL,			/* pppoe_service */
    NULL, NULL,			/* pppoe_ac */
    NULL,			/* pppoe_keepalive */
    NULL
  }, **arg;
  /* Add optional arguments */
  for (arg = pppoe_argv; *arg; arg++);

  /* Removed by AhMan */

  if (pppoe_num == PPPOE0)
    {				// PPPOE0 must set default route.
      *arg++ = "-R";
    }

  if (nvram_invmatch (ppp_service[pppoe_num], ""))
    {
      *arg++ = "-s";
      *arg++ = nvram_safe_get (ppp_service[pppoe_num]);
    }
  if (nvram_invmatch (ppp_ac[pppoe_num], ""))
    {
      *arg++ = "-a";
      *arg++ = nvram_safe_get (ppp_ac[pppoe_num]);
    }
  if (nvram_match ("ppp_static", "1"))
    {
      *arg++ = "-L";
      *arg++ = nvram_safe_get ("ppp_static_ip");
    }
  //if (nvram_match("pppoe_demand", "1") || nvram_match("pppoe_keepalive", "1"))
  *arg++ = "-k";

  mkdir ("/tmp/ppp", 0777);
  symlink ("/sbin/rc", "/tmp/ppp/ip-up");
  symlink ("/sbin/rc", "/tmp/ppp/ip-down");
  symlink ("/sbin/rc", "/tmp/ppp/set-pppoepid");	// tallest 1219
  unlink ("/tmp/ppp/log");

  //Clean rpppoe client files - Added by ice-man (Wed Jun 1)
  unlink ("/tmp/ppp/options.pppoe");
  unlink ("/tmp/ppp/connect-errors");

  _eval (pppoe_argv, NULL, 0, &pid);

  if (nvram_match (ppp_demand[pppoe_num], "1"))
    {
      //int timeout = 5;
      start_tmp_ppp (pppoe_num);

// This should be handled in start_wan_done
//      while (ifconfig (nvram_safe_get (pppoeifname), IFUP, NULL, NULL)
//           && timeout--)
//      sleep (1);
//      route_add (nvram_safe_get ("wan_iface"), 0, "0.0.0.0", "10.112.112.112",
//               "0.0.0.0");

    }
  cprintf ("done. session %d\n", pppoe_num);
  return 0;
}
#endif
/* AhMan  March 18 2005 */
/*
 * Get the IP, Subnetmask, Geteway from WAN interface
 * and set to NV ram.
 */
void
start_tmp_ppp (int num)
{

  int timeout = 5;
  char pppoeifname[15];
  char wanip[2][15] = { "wan_ipaddr", "wan_ipaddr_1" };
  char wanmask[2][15] = { "wan_netmask", "wan_netmask_1" };
  char wangw[2][15] = { "wan_gateway", "wan_gateway_1" };
  //char wanif[2][15]={"wan_ifname","wan_ifname_1"};
  //char *wan_ifname = nvram_safe_get("wan_ifname");
  struct ifreq ifr;
  int s;

  cprintf ("start session %d\n", num);

  sprintf (pppoeifname, "pppoe_ifname%d", num);

  if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    return;

  /* Wait for ppp0 to be created */
  while (ifconfig (nvram_safe_get (pppoeifname), IFUP, NULL, NULL)
	 && timeout--)
    sleep (1);

  strncpy (ifr.ifr_name, nvram_safe_get (pppoeifname), IFNAMSIZ);

  /* Set temporary IP address */
  timeout = 3;
  while (ioctl (s, SIOCGIFADDR, &ifr) && timeout--)
    {
      perror (nvram_safe_get (pppoeifname));
      printf ("Wait %s inteface to init (1) ...\n",
	      nvram_safe_get (pppoeifname));
      sleep (1);
    };
  nvram_set (wanip[num], inet_ntoa (sin_addr (&(ifr.ifr_addr))));
  nvram_set (wanmask[num], "255.255.255.255");

  /* Set temporary P-t-P address */
  timeout = 3;
  while (ioctl (s, SIOCGIFDSTADDR, &ifr) && timeout--)
    {
      perror (nvram_safe_get (pppoeifname));
      printf ("Wait %s inteface to init (2) ...\n",
	      nvram_safe_get (pppoeifname));
      sleep (1);
    }
  nvram_set (wangw[num], inet_ntoa (sin_addr (&(ifr.ifr_dstaddr))));

  start_wan_done (nvram_safe_get (pppoeifname));

  // if user press Connect" button from web, we must force to dial
  if (nvram_match ("action_service", "start_pppoe")
      || nvram_match ("action_service", "start_pppoe_1"))
    {
      sleep (3);
      // force_to_dial(nvram_safe_get("action_service"));
      start_force_to_dial ();
      nvram_set ("action_service", "");
    }

  close (s);
  cprintf ("done session %d\n", num);
  return;
}

//=====================================================================================================



int
start_l2tp (int status)
{
  int ret;
  FILE *fp;
  char *l2tp_argv[] = { "l2tpd",
    NULL
  };
  char l2tpctrl[64];
  char username[80], passwd[80];

  //stop_dhcpc();
#ifdef HAVE_PPPOE
  stop_pppoe ();
#endif
#ifdef HAVE_PPTP
  stop_pptp ();
#endif

  if (nvram_match ("aol_block_traffic", "0"))
    {
      snprintf (username, sizeof (username), "%s",
		nvram_safe_get ("ppp_username"));
      snprintf (passwd, sizeof (passwd), "%s", nvram_safe_get ("ppp_passwd"));
    }
  else
    {
      if (!strcmp (nvram_safe_get ("aol_username"), ""))
	{
	  snprintf (username, sizeof (username), "%s",
		    nvram_safe_get ("ppp_username"));
	  snprintf (passwd, sizeof (passwd), "%s",
		    nvram_safe_get ("ppp_passwd"));
	}
      else
	{
	  snprintf (username, sizeof (username), "%s",
		    nvram_safe_get ("aol_username"));
	  snprintf (passwd, sizeof (passwd), "%s",
		    nvram_safe_get ("aol_passwd"));
	}
    }

  if (status != REDIAL)
    {
      mkdir ("/tmp/ppp", 0777);
      symlink ("/sbin/rc", "/tmp/ppp/ip-up");
      symlink ("/sbin/rc", "/tmp/ppp/ip-down");
      symlink ("/dev/null", "/tmp/ppp/connect-errors");

      /* Generate L2TP configuration file */
      if (!(fp = fopen ("/tmp/l2tp.conf", "w")))
	{
	  perror ("/tmp/l2tp.conf");
	  return -1;
	}
      fprintf (fp, "global\n");	// Global section
      fprintf (fp, "load-handler \"sync-pppd.so\"\n");	// Load handlers
      fprintf (fp, "load-handler \"cmd.so\"\n");
      fprintf (fp, "listen-port 1701\n");	// Bind address
      fprintf (fp, "section sync-pppd\n");	// Configure the sync-pppd handler
      fprintf (fp, "section peer\n");	// Peer section
      fprintf (fp, "peer %s\n", nvram_safe_get ("l2tp_server_ip"));
      fprintf (fp, "port 1701\n");
      fprintf (fp, "lac-handler sync-pppd\n");
      fprintf (fp, "section cmd\n");	// Configure the cmd handler
      fclose (fp);

      /* Generate options file */
      if (!(fp = fopen ("/tmp/ppp/options", "w")))
	{
	  perror ("/tmp/ppp/options");
	  return -1;
	}
      fprintf (fp, "defaultroute\n");	//Add a default route to the system routing tables, using the peer as the gateway
      fprintf (fp, "usepeerdns\n");	//Ask the peer for up to 2 DNS server addresses
      //fprintf(fp, "pty 'pptp %s --nolaunchpppd'\n",nvram_safe_get("pptp_server_ip")); 
      fprintf (fp, "user '%s'\n", username);
      //fprintf(fp, "persist\n");        // Do not exit after a connection is terminated.

      if (nvram_match ("mtu_enable", "1"))
	{
	  fprintf (fp, "mtu %s\n", nvram_safe_get ("wan_mtu"));
	}

      if (nvram_match ("ppp_demand", "1"))
	{			//demand mode
	  fprintf (fp, "idle %d\n",
		   nvram_match ("ppp_demand",
				"1") ? atoi (nvram_safe_get ("ppp_idletime"))
		   * 60 : 0);
	  //fprintf(fp, "demand\n");         // Dial on demand
	  //fprintf(fp, "persist\n");        // Do not exit after a connection is terminated.
	  //fprintf(fp, "%s:%s\n",PPP_PSEUDO_IP,PPP_PSEUDO_GW);   // <local IP>:<remote IP>
	  fprintf (fp, "ipcp-accept-remote\n");
	  fprintf (fp, "ipcp-accept-local\n");
	  fprintf (fp, "connect true\n");
	  fprintf (fp, "noipdefault\n");	// Disables  the  default  behaviour when no local IP address is specified
	  fprintf (fp, "ktune\n");	// Set /proc/sys/net/ipv4/ip_dynaddr to 1 in demand mode if the local address changes
	}
      else
	{			// keepalive mode
	  start_redial ();
	}

      fprintf (fp, "default-asyncmap\n");	// Disable  asyncmap  negotiation
      fprintf (fp, "nopcomp\n");	// Disable protocol field compression
      fprintf (fp, "noaccomp\n");	// Disable Address/Control compression 
      fprintf (fp, "noccp\n");	// Disable CCP (Compression Control Protocol)
      fprintf (fp, "novj\n");	// Disable Van Jacobson style TCP/IP header compression
      fprintf (fp, "nobsdcomp\n");	// Disables BSD-Compress  compression
      fprintf (fp, "nodeflate\n");	// Disables Deflate compression
      fprintf (fp, "lcp-echo-interval 0\n");	// Don't send an LCP echo-request frame to the peer
      fprintf (fp, "lock\n");
      fprintf (fp, "noauth\n");

      fclose (fp);

      /* Generate pap-secrets file */
      if (!(fp = fopen ("/tmp/ppp/pap-secrets", "w")))
	{
	  perror ("/tmp/ppp/pap-secrets");
	  return -1;
	}
      fprintf (fp, "\"%s\" * \"%s\" *\n", username, passwd);
      fclose (fp);
      chmod ("/tmp/ppp/pap-secrets", 0600);

      /* Generate chap-secrets file */
      if (!(fp = fopen ("/tmp/ppp/chap-secrets", "w")))
	{
	  perror ("/tmp/ppp/chap-secrets");
	  return -1;
	}
      fprintf (fp, "\"%s\" * \"%s\" *\n", username, passwd);
      fclose (fp);
      chmod ("/tmp/ppp/chap-secrets", 0600);

      /* Enable Forwarding */
      if ((fp = fopen ("/proc/sys/net/ipv4/ip_forward", "r+")))
	{
	  fputc ('1', fp);
	  fclose (fp);
	}
      else
	perror ("/proc/sys/net/ipv4/ip_forward");
    }

  /* Bring up  WAN interface */
  //ifconfig(nvram_safe_get("wan_ifname"), IFUP,
  //       nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));

  ret = _eval (l2tp_argv, NULL, 0, NULL);
  sleep (1);
  snprintf (l2tpctrl, sizeof (l2tpctrl),
	    "/usr/sbin/l2tp-control \"start-session %s\"",
	    nvram_safe_get ("l2tp_server_ip"));
  //system(l2tpctrl);

  if (nvram_match ("ppp_demand", "1"))
    {
      /* Trigger Connect On Demand if user press Connect button in Status page */
      if (nvram_match ("action_service", "start_l2tp"))
	{
	  start_force_to_dial ();
	  nvram_set ("action_service", "");
	}
      /* Trigger Connect On Demand if user ping pptp server */
      else
	eval ("listen", nvram_safe_get ("lan_ifname"));
    }
  else
    system (l2tpctrl);

  cprintf ("done\n");
  return ret;
}

int
start_l2tp_redial (void)
{
  return start_l2tp (REDIAL);
}

int
start_l2tp_boot (void)
{
  return start_l2tp (BOOT);
}

int
stop_l2tp (void)
{
  int ret = 0;

  unlink ("/tmp/ppp/link");
  //ret = killps("pppd","-9");
  //ret += killps("l2tpd","-9");
  //ret += killps("listen","-9");

  ret = eval ("killall", "-9", "pppd");
  ret += eval ("killall", "-9", "l2tpd");
  ret += eval ("killall", "-9", "listen");

  cprintf ("done\n");
  return ret;
}

#ifdef HAVE_MULTICAST
int
start_igmp_proxy (void)
{
  int ret = 0;
  char *igmp_proxy_argv[] = { "igmprt",
    "-f",
    "-i", get_wan_face (),
    NULL
  };

  if (nvram_match ("multicast_pass", "1"))
    ret = _eval (igmp_proxy_argv, NULL, 0, NULL);

  cprintf ("done\n");
  return ret;
}

int
stop_igmp_proxy (void)
{
  //int ret = killps("igmprt","-9");
  int ret = eval ("killall", "-9", "igmprt");

  cprintf ("done\n");
  return ret;
}
#endif



#ifdef HAVE_NOCAT
int
start_splashd (void)
{
  int ret = 0;
  FILE *fp;

  if (!nvram_match ("NC_enable", "1"))
    return 0;

  /* Irving - make sure our WAN link is up first.
     if not, check_ps will start us later */
  if (nvram_match ("wan_ipaddr", "0.0.0.0"))
    return 0;

  mk_nocat_conf ();

  if (!(fp = fopen ("/tmp/start_splashd.sh", "w")))
    {
      perror ("/tmp/start_splashd.sh");
      return errno;
    }
  fprintf (fp, "#!/bin/sh\n");
  fprintf (fp, "sleep 20\n");
  fprintf (fp, "splashd >> /tmp/nocat.log 2>&1 &\n");
  fclose (fp);
  chmod ("/tmp/start_splashd.sh", 0700);
  system ("/tmp/start_splashd.sh&");
  //ret = eval("sh", "/tmp/start_splashd.sh");

  cprintf ("done\n");
  return ret;
}

int
stop_splashd (void)
{
  int ret;
  //ret = killps("splashd",NULL);
  ret = eval ("killall", "splashd");

  cprintf ("done\n");
  return ret;
}

#endif



#ifdef HAVE_TELNET
/* begin Sveasoft additon */
int
start_telnetd (void)
{
  int ret = 0;
  pid_t pid;

  char *telnetd_argv[] = { "/usr/sbin/telnetd", NULL };

  stop_telnetd ();

  if (!nvram_invmatch ("telnetd_enable", "0"))
    return 0;

  ret = _eval (telnetd_argv, NULL, 0, &pid);

  cprintf ("done\n");
  return ret;
}

int
stop_telnetd (void)
{
  int ret;

  //ret = killps("telnetd","-9");
  ret = eval ("killall", "-9", "telnetd");

  cprintf ("done\n");
  return ret;
}
#endif

int
stop_wland (void)
{
  int ret = eval ("killall", "-9", "wland");

  cprintf ("done\n");
  return ret;
}

int
start_wland (void)
{
  int ret;
  pid_t pid;
  char *wland_argv[] = { "/sbin/wland",
    NULL
  };

  stop_wland ();

//  if( nvram_match("apwatchdog_enable", "0") )
//          return 0;

  ret = _eval (wland_argv, NULL, 0, &pid);
  cprintf ("done\n");
  return ret;
}



int
start_process_monitor (void)
{
  if (nvram_match ("pmonitor_enable", "0"))
    return 0;

  pid_t pid;

  char *argv[] = { "process_monitor", NULL };
  int ret = _eval (argv, NULL, 0, &pid);

  cprintf ("done");

  return ret;
}

int
stop_process_monitor (void)
{
  int ret;

  ret = eval ("killall", "process_monitor");

  cprintf ("done\n");

  return ret;
}

int
start_radio_timer (void)
{
  if (nvram_match ("radio_timer_enable", "0"))
    return 0;

  pid_t pid;

  char *argv[] = { "radio_timer", NULL };
  int ret = _eval (argv, NULL, 0, &pid);

  cprintf ("done");

  return ret;
}

int
stop_radio_timer (void)
{
  int ret;

  ret = eval ("killall", "radio_timer");

  cprintf ("done\n");

  return ret;
}

int
stop_ntp (void)
{
  eval ("killall", "-9", "ntpclient");
  return 0;
}

/* Trigger Connect On Demand */
int
start_force_to_dial (void)
{
//force_to_dial( char *whichone){
  int ret = 0;
  char dst[50];

  {
    //sprintf(&dst,"1.1.1.1");
    sprintf (&dst, nvram_safe_get ("wan_gateway"));
  }

  char *ping_argv[] = { "ping",
    "-c", "1",
    dst,
    NULL
  };

  sleep (1);
#ifdef HAVE_L2TP
  if (nvram_match ("wan_proto", "l2tp"))
    {
      char l2tpctrl[64];

      snprintf (l2tpctrl, sizeof (l2tpctrl),
		"/usr/sbin/l2tp-control \"start-session %s\"",
		nvram_safe_get ("l2tp_server_ip"));
      system (l2tpctrl);
      return ret;
    }
#endif
#ifdef HAVE_HEARTBEAT
  if (nvram_match ("wan_proto", "heartbeat"))
    {
      start_heartbeat_boot ();
      return ret;
    }
#endif
  _eval (ping_argv, NULL, 3, NULL);

  return ret;
}
#ifdef HAVE_MEDIASERVER
int
start_hotplug_usb (void)
{
//      char *lan_ifname = nvram_safe_get("lan_ifname");
  char *interface = getenv("INTERFACE"); 
  char *action = getenv("ACTION");
  char *product = getenv("PRODUCT");
  char *devpath = getenv("DEVPATH");
  char *type = getenv("TYPE");
  char *devfs = getenv("DEVFS");
  char *device = getenv("DEVICE");
cprintf("interface %s\n",interface!=NULL?interface:"");
cprintf("action %s\n",action!=NULL?action:"");
cprintf("product %s\n",product!=NULL?product:"");
cprintf("devpath %s\n",devpath!=NULL?devpath:"");
cprintf("type %s\n",type!=NULL?type:"");
cprintf("devfs %s\n",devfs!=NULL?devfs:"");
cprintf("device %s\n",device!=NULL?device:"");


  return 0;
}
#endif
#ifdef HAVE_MICRO
extern int br_add_bridge (const char *brname);
extern int br_del_bridge (const char *brname);
extern int br_add_interface (const char *br, const char *dev);
extern int br_del_interface (const char *br, const char *dev);
extern int br_set_stp_state (const char *br, int stp_state);

int brctl_main(int argc,char **argv)
{
if (argc==1)
    {
    fprintf(stderr,"try to be professional!\n");
    return -1;
    }
br_init();
if (!strcmp(argv[1],"addif"))
    {
    br_add_interface(argv[2],argv[3]);
    }
if (!strcmp(argv[1],"delif"))
    {
    br_del_interface(argv[2],argv[3]);
    }
if (!strcmp(argv[1],"addbr"))
    {
    br_add_bridge(argv[2]);
    }
if (!strcmp(argv[1],"delbr"))
    {
    br_del_bridge(argv[2]);
    }
br_shutdown();
}
#else
int br_add_bridge (const char *brname)
    {
    return eval("/usr/sbin/brctl","addbr",brname);
    }
int br_del_bridge (const char *brname)
    {
    return eval("/usr/sbin/brctl","delbr",brname);
    }
int br_add_interface (const char *br, const char *dev)
    {
    return eval("/usr/sbin/brctl","addif",br,dev);
    }
int br_del_interface (const char *br, const char *dev)
    {
    return eval("/usr/sbin/brctl","delif",br,dev);
    }
int br_set_stp_state (const char *br, int stp_state)
    {
    if (stp_state)
    return eval("/usr/sbin/brctl","stp",br,"on");
    else
    return eval("/usr/sbin/brctl","stp",br,"off");
    }
#endif
