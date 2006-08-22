
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

    route_del (ifname, atoi (metric) + 1, ipaddr, gateway, netmask);
  }

  return 0;
}

int
start_services (void)
{
#ifdef HAVE_SER
  nvram_set ("sipgate", "1");
  //      out=fopen("/tmp/httpd.conf","wb");
  //      fprintf(out,"/:root:%s\n",nvram_get("http_passwd"));
  //      fclose(out);
  //      ret |= eval("httpd2","-h","/sipath","-p","81","-c","/tmp/httpd.conf","-r","DD-WRT Router");
#else
  nvram_set ("sipgate", "0");
#endif

#ifdef HAVE_TELNET
  start_service ("telnetd");
#endif
  start_service ("syslog");
#ifdef HAVE_TFTP
  start_service ("tftpd");
#endif
  start_service ("httpd");
  start_service ("udhcpd");
  start_service ("dnsmasq");
  start_service ("nas_lan");
#ifdef HAVE_MSSID
  start_service ("guest_nas");
#endif
#ifdef HAVE_BIRD
  start_service ("zebra");
#endif
  start_service ("wland");
  start_service ("wshaper");
  start_service ("cron");
  start_service ("radio_timer");

#ifdef HAVE_PPTPD
  start_service ("pptpd");
#endif

#ifdef HAVE_SSHD
  start_service ("sshd");
#endif

#ifdef HAVE_RADVD
  start_service ("radvd");
#endif

#ifdef HAVE_SNMP
  start_service ("snmp");
#endif

#ifdef HAVE_WOL
  start_service ("wol");
#endif

#ifdef HAVE_NOCAT
  start_service ("splashd");
#endif

#ifdef HAVE_UPNP
  start_service ("upnp");
#endif

#ifdef HAVE_NEWMEDIA
  start_service ("openvpnserversys");
#endif



  cprintf ("done\n");
  return 0;
}

int
stop_services (void)
{
  //stop_ses();
  stop_service ("nas");
#ifdef HAVE_UPNP
  stop_service ("upnp");
#endif
  stop_service ("udhcpd");
  stop_service ("dns_clear_resolv");
  stop_service ("cron");
  stop_service ("radio_timer");

#ifdef HAVE_TFTP
  stop_service ("tftpd");
#endif
  stop_service ("syslog");
#ifdef HAVE_BIRD
  stop_service ("zebra");
#endif
  stop_service ("wland");
#ifdef HAVE_TELNET
  stop_service ("telnetd");
#endif
#ifdef HAVE_SSHD
  stop_service ("sshd");
#endif

#ifdef HAVE_RADVD
  stop_service ("radvd");
#endif

#ifdef HAVE_CHILLI
  stop_service ("chilli");
#endif

#ifdef HAVE_SNMP
  stop_service ("snmp");
#endif

#ifdef HAVE_WOL
  stop_service ("wol");
#endif
  stop_service ("wshaper");

#ifdef HAVE_PPTPD
  stop_service ("pptpd");
#endif
#ifdef HAVE_NOCAT
  stop_service ("splashd");
#endif
#ifdef HAVE_NEWMEDIA
  stop_service ("openvpnserversys");
#endif

// end Sveasoft additions
//stop_eou();

  cprintf ("done\n");
  return 0;
}



int
start_single_service (void)
{
  char *service;

  service = nvram_get ("action_service");

  if (!service)
    kill (1, SIGHUP);

  cprintf ("Restart service=[%s]\n", service);
  start_service ("overclocking");

  if (!strcmp (service, "dhcp") || !strcmp (service, "services"))
    {
      startstop ("udhcpd");
    }
  else if (!strcmp (service, "logging"))
    {
      startstop ("firewall");
      startstop ("syslog");
    }
/* Sveasoft addition */
  else if (!strcmp (service, "router"))
    {
#ifdef HAVE_BIRD
      startstop ("zebra");
#endif
    }
  else if (!strcmp (service, "hotspot"))
    {
#ifdef HAVE_NOCAT
      startstop ("splashd");
#endif
#ifdef HAVE_CHILLI
      startstop ("chilli");
#endif
#ifdef HAVE_SPUTNIK_APD
      startstop ("sputnik");
#endif
    }
  else if (!strcmp (service, "services"))
    {
      startstop ("udhcpd");
#ifdef HAVE_TELNET
      startstop ("telnetd");
#endif
#ifdef HAVE_SNMP
      startstop ("snmp");
#endif
#ifdef HAVE_SSHD
      startstop ("sshd");
#endif
      startstop ("firewall");
      startstop ("syslog");
#ifdef HAVE_NEWMEDIA
      startstop ("openvpnserversys");
#endif
    }
  else if (!strcmp (service, "management"))
    {
      if (nvram_match ("wl0_mode", "wet") || nvram_match ("wl0_mode", "sta")
	  || nvram_match ("wl0_mode", "apsta"))
	stop_service ("nas");
#ifdef HAVE_BIRD
      stop_service ("zebra");
#endif
      stop_service ("cron");
      stop_service ("udhcpd");
      start_service ("udhcpd");
      start_service ("cron");
      start_service ("ipv6");
#ifdef HAVE_RADVD
      startstop ("radvd");
#endif
#ifdef HAVE_PPTPD
      startstop ("pptpd");
#endif
#ifdef HAVE_BIRD
      start_service ("zebra");
#endif
      startstop ("firewall");
      startstop ("wshaper");
      startstop ("httpd");

#ifdef HAVE_WOL
      startstop ("wol");
#endif
      if (nvram_match ("wl0_mode", "wet") || nvram_match ("wl0_mode", "sta")
	  || nvram_match ("wl0_mode", "apsta"))
	start_service ("nas_lan");

    }

/* end Sveasoft additon */
  else if (!strcmp (service, "start_pppoe") || !strcmp (service, "start_pptp")
	   || !strcmp (service, "start_l2tp")
	   || !strcmp (service, "start_heartbeat"))
    {
      unlink ("/tmp/ppp/log");
      stop_service ("lan");
      stop_service ("wan");
      start_service ("lan");
      start_service ("wan_boot");
    }
  else if (!strcmp (service, "stop_pppoe") || !strcmp (service, "stop_pptp")
	   || !strcmp (service, "stop_l2tp")
	   || !strcmp (service, "stop_heartbeat"))
    {
      stop_service ("wan");
    }
  else if (!strcmp (service, "filters"))
    {
      stop_service ("cron");
      startstop ("firewall");
      startstop ("wshaper");
      start_service ("cron");
    }
  else if (!strcmp (service, "forward"))
    {
      startstop ("firewall");
      startstop ("wshaper");
    }
  else if (!strcmp (service, "forward_upnp"))
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
  else if (!strcmp (service, "static_route_del"))
    {
      if (nvram_safe_get ("action_service_arg1"))
	{
	  del_routes (nvram_safe_get ("action_service_arg1"));
	}
    }
  else if (!strcmp (service, "ddns"))
    {
      startstop ("ddns");
      nvram_set ("ddns_change", "update");
    }
  else if (!strcmp (service, "start_ping"))
    {
      char *ip = nvram_safe_get ("ping_ip");
      // use Ping.asp as a debugging console
      char cmd[256] = { 0 };
      //snprintf (cmd, sizeof (cmd), "%s > %s 2>&1 &", ip, PING_TMP);
      setenv ("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
//      snprintf (cmd, sizeof (cmd), "%s 2>&1 &", ip);
//      system (cmd);

	      	snprintf (cmd, sizeof (cmd), "alias ping=\'ping -c 3\'; eval \"%s\" > %s 2>&1 &", ip, PING_TMP);
      		system (cmd);

/*
		if(!check_wan_link(0))
			buf_to_file(PING_TMP, "Network is unreachable\n");

		else if(strchr(ip, ' ') || strchr(ip, '`') || strstr(ip, PING_TMP))		// Fix Ping.asp bug, user can execute command ping in Ping.asp
			buf_to_file(PING_TMP, "Invalid IP Address or Domain Name\n");

		else if(nvram_invmatch("ping_times","") && nvram_invmatch("ping_ip","")){
			char cmd[80];
			snprintf(cmd, sizeof(cmd), "ping -c %s -f %s %s &", nvram_safe_get("ping_times"), PING_TMP, ip);
	   	     	printf("cmd=[%s]\n",cmd);
			eval("killall", "ping");
			unlink(PING_TMP);
	        	system(cmd);
		}
*/
    } 
/* OBSOLETE
  else if (!strcmp (service, "start_traceroute"))
    {
      char *ip = nvram_safe_get ("traceroute_ip");
      if (!check_wan_link (0))
	buf_to_file (TRACEROUTE_TMP, "Network is unreachable\n");

      else if (strchr (ip, ' ') || strchr (ip, '`') || strstr (ip, TRACEROUTE_TMP))	// Fix Traceroute.asp bug, users can execute command in Traceroute.asp 
	buf_to_file (TRACEROUTE_TMP, "Invalid IP Address or Domain Name\n");

      else if (nvram_invmatch ("traceroute_ip", ""))
	{
	  // Some site block UDP packets, so we want to use ICMP packets
	  char cmd[80];
	  snprintf (cmd, sizeof (cmd), "/usr/bin/traceroute -f %s %s &",
		    TRACEROUTE_TMP, ip);
	  printf ("cmd=[%s]\n", cmd);
	  //killps("traceroute",NULL);
	  eval ("killall", "traceroute");
	  unlink (TRACEROUTE_TMP);
	  system (cmd);
	}
    } 
END OBSOLETE */
#ifdef HAVE_TFTP
  else if (!strcmp (service, "tftp_upgrade"))
    {
      stop_service ("wan");
      stop_service ("httpd");
#ifdef HAVE_BIRD
      stop_service ("zebra");
#endif
#ifdef HAVE_UPNP
      stop_service ("upnp");
#endif
      stop_service ("cron");
    } 
#endif

  else if (!strcmp (service, "http_upgrade"))
    {
      stop_service ("wan");
#ifdef HAVE_BIRD
      stop_service ("zebra");
#endif
#ifdef HAVE_UPNP
      stop_service ("upnp");
#endif
      stop_service ("cron");
    }
  else if (!strcmp (service, "wireless"))
    {
      eval ("wlconf", nvram_safe_get ("wl0_ifname"), "down");
      stop_services ();
      stop_service ("lan");
#ifndef HAVE_MSSID
      if (nvram_match ("wl_akm", "wpa") ||
	  nvram_match ("wl_akm", "psk") ||
	  nvram_match ("wl_akm", "psk2") ||
	  nvram_match ("wl_akm", "wpa2") ||
	  nvram_match ("wl_akm", "psk psk2") ||
	  nvram_match ("wl_akm", "wpa wpa2") ||
	  nvram_match ("wl_akm", "radius"))
	sleep (4);
#endif
      start_service ("wlconf");
      start_service ("lan");
      start_services ();
    }
  else if (!strcmp (service, "dhcp_release"))
    {
      char sigusr[] = "-XX";
      sprintf (sigusr, "-%d", SIGUSR2);
      //killps("udhcpc",sigusr);
      eval ("killall", sigusr, "udhcpc");
      sleep (1);
    }
  else
    {
      nvram_unset ("action_service");
      nvram_unset ("action_service_arg1");
      kill (1, SIGHUP);
    }

  nvram_set ("action_service", "");
  nvram_set ("action_service_arg1", "");

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
		  stop_service ("pppoe");
		  //killps("pppoecd","-9");
		  eval ("killall", "-9", "pppoecd");
		  sleep (1);
		  start_service ("wan_redial");
		}
	      else
#endif
#ifdef HAVE_PPTP
	      if (nvram_match ("wan_proto", "pptp"))
		{
		  stop_service ("pptp");
		  sleep (1);
		  start_service ("wan_redial");
		}
#endif
#ifdef HAVE_PPTP | HAVE_PPPOE
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
