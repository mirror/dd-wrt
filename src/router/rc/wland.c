
/*
 *********************************************************
 *   Copyright 2003, 2004 Sveasoft AB                    *
 *   All Rights Reserved                                 *
 *********************************************************

 This is PROPRIETARY SOURCE CODE developed by Sveasoft AB

 THIS SOFTWARE IS OFFERED "AS IS", AND SVEASOFT GRANTS NO WARRANTIES OF ANY
 KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  SVEASOFT
 SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE

 COPYRIGHT 2004 Sveasoft AB

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlioctl.h>
#include <rc.h>

#include <cy_conf.h>
#include <bcmutils.h>
#include <utils.h>
#include <nvparse.h>



#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 251
#define TXPWR_DEFAULT 28


#define WLAND_INTERVAL 15

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

char *
get_wdev ()
{
  if (check_hw_type () == BCM4702_CHIP)
    return "eth2";
  else
    return "eth1";

}

static int
notify_nas (char *type, char *ifname, char *action)
{
  char *argv[] = { "nas4not", type, ifname, action,
    NULL,			/* role */
    NULL,			/* crypto */
    NULL,			/* auth */
    NULL,			/* passphrase */
    NULL,			/* ssid */
    NULL
  };
  char *str = NULL;
  int retries = 10;
  char tmp[100], prefix[] = "wlXXXXXXXXXX_";
  int unit;
  char remote[ETHER_ADDR_LEN];
  char ssid[48], pass[80], auth[16], crypto[16], role[8];
  int i;

  /* the wireless interface must be configured to run NAS */
  wl_ioctl (ifname, WLC_GET_INSTANCE, &unit, sizeof (unit));
  snprintf (prefix, sizeof (prefix), "wl%d_", unit);
  if (nvram_match (strcat_r (prefix, "akm", tmp), "") &&
      nvram_match (strcat_r (prefix, "auth_mode", tmp), "none"))
    return 0;

  /* find WDS link configuration */
  wl_ioctl (ifname, WLC_WDS_GET_REMOTE_HWADDR, remote, ETHER_ADDR_LEN);
  for (i = 0; i < MAX_NVPARSE; i++)
    {
      char mac[ETHER_ADDR_STR_LEN];
      uint8 ea[ETHER_ADDR_LEN];

      if (get_wds_wsec (unit, i, mac, role, crypto, auth, ssid, pass) &&
	  ether_atoe (mac, ea) && !bcmp (ea, remote, ETHER_ADDR_LEN))
	{
	  argv[4] = role;
	  argv[5] = crypto;
	  argv[6] = auth;
	  argv[7] = pass;
	  argv[8] = ssid;
	  break;
	}
    }

  /* did not find WDS link configuration, use wireless' */
  if (i == MAX_NVPARSE)
    {
      /* role */
      argv[4] = "auto";
      /* crypto */
      argv[5] = nvram_safe_get (strcat_r (prefix, "crypto", tmp));
      /* auth mode */
      argv[6] = nvram_safe_get (strcat_r (prefix, "akm", tmp));
      /* passphrase */
      argv[7] = nvram_safe_get (strcat_r (prefix, "wpa_psk", tmp));
      /* ssid */
      argv[8] = nvram_safe_get (strcat_r (prefix, "ssid", tmp));
    }

  /* wait till nas is started */
  while (retries-- > 0 && !(str = file2str ("/tmp/nas.lan.pid")))
    sleep (1);
  if (str)
    {
      int pid;
      free (str);
      return _eval (argv, ">/dev/console", 0, &pid);
    }
  return -1;
}

static int
do_wds_check (void)
{
  int s = 0;

  /* Sveasoft - Bring up and configure wds interfaces */
  /* logic - if separate ip defined bring it up */
  /*         else if flagged for br1 and br1 is enabled add to br1 */
  /*         else add it to the br0 bridge */
  for (s = 1; s <= MAX_WDS_DEVS; s++)
    {
      char wdsvarname[32] = { 0 };
      char wdsdevname[32] = { 0 };
      char *dev;
      struct ifreq ifr;


      sprintf (wdsvarname, "wl_wds%d_enable", s);
      sprintf (wdsdevname, "wl_wds%d_if", s);
      dev = nvram_safe_get (wdsdevname);

      if (nvram_invmatch (wdsvarname, "1"))
	continue;

      memset (&ifr, 0, sizeof (struct ifreq));

      snprintf (ifr.ifr_name, IFNAMSIZ, wdsdevname);
      ioctl (s, SIOCGIFFLAGS, &ifr);

      if ((ifr.ifr_flags & (IFF_RUNNING | IFF_UP)) == (IFF_RUNNING | IFF_UP))
	continue;

      /* P2P WDS type */
      if (nvram_match (wdsvarname, "1"))
	{
	  char wdsip[32] = { 0 };
	  char wdsbc[32] = { 0 };
	  char wdsnm[32] = { 0 };

	  snprintf (wdsip, 31, "wl_wds%d_ipaddr", s);
	  snprintf (wdsnm, 31, "wl_wds%d_netmask", s);

	  snprintf (wdsbc, 31, "%s", nvram_safe_get (wdsip));
	  get_broadcast (wdsbc, nvram_safe_get (wdsnm));
	  eval ("ifconfig", dev, nvram_safe_get (wdsip), "broadcast", wdsbc,
		"netmask", nvram_safe_get (wdsnm), "up");
	}
      /* Subnet WDS type */
      else if (nvram_match (wdsvarname, "2")
	       && nvram_match ("wl_br1_enable", "1"))
	{
	  eval ("ifconfig", dev, "up");
	  eval ("brctl", "addif", "br1", dev);
	}
      /* LAN WDS type */
      else if (nvram_match (wdsvarname, "3"))
	{
	  eval ("ifconfig", dev, "up");
	  eval ("brctl", "addif", "br0", dev);
	}

    }

  if (nvram_match ("router_disable", "1") || nvram_match ("lan_stp", "0"))
    system ("/usr/sbin/brctl stp br0 disable stp");

  return 0;
}


static int
do_ap_watchdog (void)
{

  /* AP Watchdog - experimental check and fix for hung AP */
  int val;
  struct stat s;
  static time_t last;
  int interval =
    atoi (nvram_safe_get ("apwatchdog_interval")) >
    WLAND_INTERVAL ? atoi (nvram_safe_get ("apwatchdog_interval")) :
    WLAND_INTERVAL;

  system ("/usr/sbin/wl assoclist 2>&1 > /tmp/.assoclist");
  stat ("/tmp/.assoclist", &s);
  unlink ("/tmp/.assoclist");

  if (s.st_size <= 0 &&
      time (NULL) - last > interval &&
      nvram_match ("apwatchdog_enable", "1") &&
      nvram_invmatch ("wl_net_mode", "disabled"))
    {

      int val = 0;

      time (&last);
      cprintf ("resetting ap radio\n");
      eval ("/usr/sbin/wlconf", get_wdev (), "down");

      val = atoi (nvram_safe_get ("wl0_channel")) + 1;
      if (val <= 2 || val >= 14)
	val = 2;

      wl_ioctl (get_wdev (), WLC_SET_CHANNEL, &val, sizeof (val));
      wl_ioctl (get_wdev (), WLC_UP, NULL, 0);

      eval ("/usr/sbin/wlconf", get_wdev (), "down");
      wlconf_up (get_wdev ());

    }

  return 0;

}

#ifdef HAVE_AQOS
int
compareNet (char *ip, char *net, char *dest)
{
  unsigned int ip1 = atoi (strsep (&ip, "."));
  unsigned int ip2 = atoi (strsep (&ip, "."));
  unsigned int ip3 = atoi (strsep (&ip, "."));
  unsigned int ip4 = atoi (ip);

  unsigned int dip1 = atoi (strsep (&dest, "."));
  unsigned int dip2 = atoi (strsep (&dest, "."));
  unsigned int dip3 = atoi (strsep (&dest, "."));
  unsigned int dip4 = atoi (dest);

  unsigned int fullip = (ip1 << 24) + (ip2 << 16) + (ip3 << 8) + ip4;
  unsigned int dfullip = (dip1 << 24) + (dip2 << 16) + (dip3 << 8) + dip4;
  int n = 1 << atoi (net);	//convert net to full mask
  if ((dfullip & n) == (fullip & n))
    return 1;
  return 0;
}

int
containsIP (char *ip)
{
  FILE *in;
  char buf_ip[20];
  char *i, *net;
  in = fopen ("/tmp/aqos_ips", "rb");
  if (in == NULL)
    return 0;
//cprintf("scan for ip %s\n",ip);
  while (fscanf (in, "%s", buf_ip) != EOF)
    {
      i = (char *) &buf_ip[0];
      net = strsep (&i, "/");
      cprintf ("found %s/%s\n", net, i);
      if (compareNet (net, i, ip))
	{
	  printf ("%s/%s fits to %s\n", net, i, ip);
	  return 1;
	}
      printf ("%s/%s dosnt fit to %s\n", net, i, ip);
    }
  fclose (in);
//cprintf("no ip found\n");
  return 0;
}

int
containsMAC (char *ip)
{
  FILE *in;
  char buf_ip[16];
  in = fopen ("/tmp/aqos_macs", "rb");
  if (in == NULL)
    return 0;
//cprintf("scan for mac %s \n",ip);
  while (fscanf (in, "%s", buf_ip) != EOF)
    {
      if (!strcmp (buf_ip, ip))
	return 1;
    }
  fclose (in);
//cprintf("no mac found\n");
  return 0;
}


static int
do_aqos_check (void)
{
  FILE *arp = fopen ("/proc/net/arp", "rb");
  char ip_buf[16];
  char hw_buf[16];
  char cmd[1024];
  char fl_buf[16];
  char mac_buf[18];
  char mask_buf[16];
  char dev_buf[16];
  char *wdev = get_wshaper_dev ();
  int cmac;
  int defaultlevel;
  char *defaul;
  int cip;
  if (arp == NULL)
    {
      cprintf ("/proc/net/arp missing, check kernel config\n");
      return;
    }
  defaul = nvram_safe_get ("default_level");

  if (defaul == NULL || strlen (defaul) == 0)
    return;
  defaultlevel = atoi (defaul) * 1000 / 8;
  while (fgetc (arp) != '\n');

//fscanf(arp,"%s %s %s %s %s %s",ip_buf,hw_buf,fl_buf,mac_buf,mask_buf,dev_buf); //skip first line
  while (fscanf
	 (arp, "%s %s %s %s %s %s", ip_buf, hw_buf, fl_buf, mac_buf, mask_buf,
	  dev_buf) != EOF)
    {
      cmac = containsMAC (mac_buf);
      cip = containsIP (ip_buf);

      snprintf (cmd, 1023,
		"/usr/sbin/iptables -t mangle -D PREROUTING -i br0 -m mac --mac-source %s -m connrate --connrate ! 0:%d -j DROP > /dev/null",
		mac_buf, defaultlevel);
      system (cmd);
      snprintf (cmd, 1023,
		"/usr/sbin/iptables -t mangle -D PREROUTING -i br0 -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		ip_buf, defaultlevel);
      system (cmd);
      snprintf (cmd, 1023,
		"/usr/sbin/iptables -t mangle -D PREROUTING -i br0 -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		ip_buf, defaultlevel);
      system (cmd);
      snprintf (cmd, 1023,
		"/usr/sbin/iptables -t mangle -D POSTROUTING -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		ip_buf, defaultlevel);
      system (cmd);
      snprintf (cmd, 1023,
		"/usr/sbin/iptables -t mangle -D POSTROUTING -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		ip_buf, defaultlevel);
      system (cmd);
//snprintf(cmd, 1023, "/usr/sbin/iptables -t nat -D PREROUTING -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null", mac_buf, default_level);
//system(cmd);
//snprintf(cmd, 1023, "/usr/sbin/iptables -t nat -D POSTROUTING -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null", mac_buf, default_level);
//system(cmd);

      if (cip || cmac)
	continue;
//cprintf("nothing found for %s %s\n",ip_buf,mac_buf);

      if (!cmac)
	{
	  //create default rule for mac
	  snprintf (cmd, 1023,
		    "/usr/sbin/iptables -t mangle -A PREROUTING -i br0 -m mac --mac-source %s -m connrate --connrate ! 0:%d -j DROP > /dev/null",
		    mac_buf, defaultlevel);
	  system (cmd);

	  //   snprintf(cmd, 1023, "/usr/sbin/iptables -t nat -A PREROUTING -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null", data, ilevel);
	  //    system(cmd);


//    snprintf(cmd, 1023, "/usr/sbin/iptables -t nat -A POSTROUTING -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null", data, ilevel);
//    system(cmd);

	}
      if (!cip)
	{
	  //create default rule for ip
	  snprintf (cmd, 1023,
		    "/usr/sbin/iptables -t mangle -A PREROUTING -i br0 -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		    ip_buf, defaultlevel);
	  system (cmd);

	  snprintf (cmd, 1023,
		    "/usr/sbin/iptables -t mangle -A PREROUTING -i br0 -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		    ip_buf, defaultlevel);
	  system (cmd);

	  snprintf (cmd, 1023,
		    "/usr/sbin/iptables -t mangle -A POSTROUTING -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		    ip_buf, defaultlevel);
	  system (cmd);

	  snprintf (cmd, 1023,
		    "/usr/sbin/iptables -t mangle -A POSTROUTING -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		    ip_buf, defaultlevel);
	  system (cmd);

	}
    }

  fclose (arp);

}
#endif

static int
do_ap_check (void)
{

//  if (nvram_match ("apwatchdog_enable", "1"))
//    do_ap_watchdog ();

  do_wds_check ();

  return 0;
}

/* for Client/Wet mode */
/* be nice to rewrite this to use sta_info_t if we had proper Broadcom API specs */
static int
do_client_check (void)
{
  FILE *fp = NULL;
  char buf[1024];
  char mac[512];
  int len;

  system ("/usr/sbin/wl assoc 2>&1 > /tmp/.xassocx");
  if ((fp = fopen ("/tmp/.xassocx", "r")) == NULL)
    return -1;

  len = fread (buf, 1, 1023, fp);

  buf[len] = 0;

  if (len > 0 && strstr (buf, "Not associated."))
    {

      /*if (nvram_match("wl_mode", "wet"))
         {

         system("/bin/rm /tmp/bridged");
         if (wl_probe("eth2"))
         eval("brctl", "delif", "br0", "eth1"); //create bridge
         else
         eval("brctl", "delif", "br0", "eth2");
         eval("wl","wet","1");
         } */
      /* let wl do this for us (no use in reinventing the wheel) */
      //eval("/usr/sbin/wlconf", get_wdev(), "down");
      //eval("/usr/sbin/wlconf", get_wdev(), "up"); 
      eval ("wl", "join", nvram_safe_get ("wl0_ssid"));
      fclose (fp);
    }				/*else if (nvram_match("wl_mode","bridge"))
				   {
				   FILE *in;
				   in=fopen("/tmp/bridged","rb");

				   if (in==NULL)
				   {
				   if (nvram_match("security_mode","wep") || nvram_match("security_mode","disabled"))
				   {
				   if (wl_probe("eth2"))
				   eval("brctl", "addif", "br0", "eth1"); //create bridge
				   else
				   eval("brctl", "addif", "br0", "eth2");
				   system("/bin/echo bridged>/tmp/bridged");

				   if (fp!=NULL)fclose(fp);
				   return 0;
				   }

				   system("/usr/sbin/wl assoc|grep BSSID:>/tmp/.associnfo");
				   in=fopen("/tmp/.associnfo","rb");
				   fscanf(in,"%s",mac);
				   fscanf(in,"%s",mac);
				   fclose(in);
				   sprintf(buf,"/usr/sbin/wl sta_info %s>/tmp/.sta_info",mac);
				   system(buf);
				   fclose(fp);
				   fp = NULL;
				   if( (fp = fopen("/tmp/.sta_info", "r")) == NULL)
				   return -1;
				   len = fread(buf, 1, 1023, fp);
				   buf[len] = 0;
				   fclose(fp);
				   //eval("/usr/sbin/wl","sta_info",mac,">/tmp/.sta_info");
				   in=fopen("/tmp/bridged","rb");

				   if (len>0 && strstr(buf,"AUTHORIZED") && in==NULL)
				   {
				   if (wl_probe("eth2"))
				   eval("brctl", "addif", "br0", "eth1"); //create bridge
				   else
				   eval("brctl", "addif", "br0", "eth2");
				   }
				   system("/bin/echo bridged>/tmp/bridged");

				   }
				   if (in!=NULL)fclose(in);
				   if (fp!=NULL)fclose(fp);

				   }

				   //unlink("/tmp/.xassocx");
				 */
  fclose (fp);
  return 0;
}

static void
do_wlan_check (void)
{

  if (nvram_invmatch ("wl0_mode", "ap"))
    do_client_check ();
  else
    do_ap_check ();
#ifdef HAVE_AQOS
  do_aqos_check ();
#endif

}


int
wland_main (int argc, char **argv)
{
  /* Run it in the background */
  switch (fork ())
    {
    case -1:
      // can't fork
      exit (0);
      break;
    case 0:
      /* child process */
      // fork ok
      (void) setsid ();
      break;
    default:
      /* parent process should just die */
      _exit (0);
    }

  /* Most of time it goes to sleep */
  while (1)
    {
      do_wlan_check ();

      sleep (WLAND_INTERVAL);
    }

  return 0;
}				// end main

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

//      if( nvram_match("apwatchdog_enable", "0") )
//          return 0;

  ret = _eval (wland_argv, NULL, 0, &pid);
  cprintf ("done\n");
  return ret;
}
