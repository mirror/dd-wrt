/*
 * Router rc control script
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: rc.c,v 1.12 2005/11/30 11:54:21 seg Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <dirent.h>

#include <epivers.h>
#include <bcmnvram.h>
#include <mtd.h>
#include <shutils.h>
#include <rc.h>
#include <netconf.h>
#include <nvparse.h>
#include <bcmdevs.h>

#include <wlutils.h>
#include <utils.h>
#include <cyutils.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <support.h>
#include <mkfiles.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <wlutils.h>
#include <cy_conf.h>



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


/* States */
enum
{
  RESTART,
  STOP,
  START,
  TIMER,
  USER,
  IDLE,
};
static int state = START;
static int signalled = -1;

/* Signal handling */
static void
rc_signal (int sig)
{
  if (state == IDLE)
    {
      if (sig == SIGHUP)
	{
	  printf ("signalling RESTART\n");
	  signalled = RESTART;
	}
      else if (sig == SIGUSR2)
	{
	  printf ("signalling START\n");
	  signalled = START;
	}
      else if (sig == SIGINT)
	{
	  printf ("signalling STOP\n");
	  signalled = STOP;
	}
      else if (sig == SIGALRM)
	{
	  printf ("signalling TIMER\n");
	  signalled = TIMER;
	}
      else if (sig == SIGUSR1)
	{			// Receive from WEB
	  printf ("signalling USER1\n");
	  signalled = USER;
	}

    }
}

/* Timer procedure */
int
do_timer (void)
{
  //do_ntp();
  return 0;
}

/* Main loop */
void
main_loop (void)
{
  int ret;
  sigset_t sigset;
  pid_t shell_pid = 0;
  uint boardflags;
  int val;
  char tmp[200];
  //setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin", 1);
  //system("/etc/nvram/nvram");
  /* Basic initialization */
  start_service("sysinit");

  /* Setup signal handlers */
  signal_init ();
  signal (SIGHUP, rc_signal);
  signal (SIGUSR1, rc_signal);	// Start single service from WEB, by honor
  signal (SIGUSR2, rc_signal);
  signal (SIGINT, rc_signal);
  signal (SIGALRM, rc_signal);
  sigemptyset (&sigset);

  /* Give user a chance to run a shell before bringing up the rest of the system */
//  if (!noconsole)
//    ddrun_shell (1, 0);

  start_service("nvram");

  /* Restore defaults if necessary */
#ifdef HAVE_SKYTEL
  nvram_set ("vlan0ports", "0 1 2 3 4 5*");
  nvram_set ("vlan1ports", "");
#else

  if (nvram_match ("fullswitch", "1") && nvram_invmatch ("wl_mode", "ap"))
    {
      nvram_set ("vlan0ports", "0 1 2 3 4 5*");
      nvram_set ("vlan1ports", "");
    }
  else
    {
      if (nvram_match ("vlan0ports", "0 1 2 3 4 5*"))
	{
	  nvram_set ("vlan0ports", "");
	  nvram_set ("vlan1ports", "");
	}
    }
#endif
  start_service("restore_defaults");
  

//#ifdef HAVE_MSSID
//create vlans for b44 driver
//robo
#ifndef HAVE_RB500

  system ("echo 1 > /proc/switch/eth0/reset");
  sprintf (tmp, "echo %s > /proc/switch/eth0/vlan/0/ports",
	   nvram_safe_get ("vlan0ports"));
  system (tmp);
  sprintf (tmp, "echo %s > /proc/switch/eth0/vlan/1/ports",
	   nvram_safe_get ("vlan1ports"));
  system (tmp);

//  system("echo 1 > /proc/switch/eth0/reset");

/*  sprintf (tmp, "echo %s > /proc/switch/bcm53xx/vlan/0/ports",
	   nvram_safe_get ("vlan0ports"));
  system (tmp);
  sprintf (tmp, "echo %s > /proc/switch/bcm53xx/vlan/1/ports",
	   nvram_safe_get ("vlan1ports"));
  system (tmp);
//  system("echo 1 > /proc/switch/adm6996/reset");
  sprintf (tmp, "echo %s > /proc/switch/adm6996/vlan/0/ports",
	   nvram_safe_get ("vlan0ports"));
  system (tmp);
  sprintf (tmp, "echo %s > /proc/switch/adm6996/vlan/1/ports",
	   nvram_safe_get ("vlan1ports"));
  system (tmp);*/
#endif
//#endif
  /* Add vlan */
  boardflags = strtoul (nvram_safe_get ("boardflags"), NULL, 0);
  nvram_set ("wanup", "0");
  /* begin Sveasoft add */
  /* force afterburner even for older boards */
//  boardflags |= BFL_AFTERBURNER;
  /* end Sveasoft add */
//  char ab[100];
//  sprintf(ab,"0x%X",boardflags);
//  nvram_set("boardflags",ab);


//      if (boardflags & BFL_ENETVLAN || check_hw_type() == BCM4712_CHIP)
  int brand = getRouterBrand ();
#ifndef HAVE_RB500
  switch (brand)
    {
    case ROUTER_ASUS:
    case ROUTER_MOTOROLA:
    case ROUTER_SIEMENS:
//    case ROUTER_WRT54G:
//    case ROUTER_WRT54G1X:
    start_service("config_vlan");
      break;
    default:
      if (check_vlan_support ())
	{
	start_service("config_vlan");
	}
      break;

    }
#endif

  // Sveasoft add 2004-07-23 - set ip_forwarding, adjust ip_contrack limits, timeouts, enable Westwood+ congestion handling
  set_ip_forward ('1');
  system ("/etc/preinit");	//sets default values for ip_conntrack

#ifndef HAVE_RB500
  char *rwpart = "mtd4";
  int itworked = 0;
  if (nvram_match ("sys_enable_jffs2", "1"))
    {
      if (nvram_match ("sys_clean_jffs2", "1"))
	{
	  nvram_set ("sys_clean_jffs2", "0");
	  nvram_commit ();
	  itworked = mtd_erase (rwpart);
        eval ("insmod", "crc32");
        eval ("insmod", "jffs2");

	  itworked +=
	    mount ("/dev/mtdblock/4", "/jffs", "jffs2", MS_MGC_VAL, NULL);
	  if (itworked)
	    {
	      nvram_set ("jffs_mounted", "0");
	    }
	  else
	    {
	      nvram_set ("jffs_mounted", "1");
	    }



	}
      else
	{
	  itworked = mtd_unlock ("mtd4");
        eval ("insmod", "crc32");
        eval ("insmod", "jffs2");
	  itworked +=
	    mount ("/dev/mtdblock/4", "/jffs", "jffs2", MS_MGC_VAL, NULL);
	  if (itworked)
	    {
	      nvram_set ("jffs_mounted", "0");
	    }
	  else
	    {
	      nvram_set ("jffs_mounted", "1");
	    }


	}
    }
#endif
  //sprintf(tmp,"%s%s",CYBERTAN_VERSION, MINOR_VERSION);
  //setenv("FIRMWARE_VERSION",tmp,1);

//      system("/bin/echo 4096 > /proc/sys/net/ipv4/ip_conntrack_max");
//      system("/bin/echo 3600 > /proc/sys/net/ipv4/ip_conntrack_tcp_timeouts");
//      system("/bin/echo 3600 > /proc/sys/net/ipv4/ip_conntrack_udp_timeouts");
//      system("/bin/echo 1 > /proc/sys/net/ipv4/tcp_westwood");

  // Sveasoft add 2004-01-04 create passwd, groups, misc files
  mkfiles ();
  char *hostname;

  /* set hostname to wan_hostname or router_name */
  if (strlen (nvram_safe_get ("wan_hostname")) > 0)
    hostname = nvram_safe_get ("wan_hostname");
  else if (strlen (nvram_safe_get ("router_name")) > 0)
    hostname = nvram_safe_get ("router_name");
  else
    hostname = "dd-wrt";

  sethostname (hostname, strlen (hostname));
  stop_service("httpd");
  if (brand == ROUTER_SIEMENS)
    {
      start_service("powerled_ctrl_1");
    }

  /* Loop forever */
  for (;;)
    {
      switch (state)
	{
	case USER:		// Restart single service from WEB of tftpd, by honor
	  cprintf ("USER1\n");
	  start_single_service ();
#ifdef HAVE_CHILLI
	  start_chilli ();
#endif

	  state = IDLE;
	  break;
	case RESTART:
	  cprintf ("RESET NVRAM VARS\n");
	  nvram_set ("wl0_lazy_wds", nvram_safe_get ("wl_lazy_wds"));
	  nvram_set ("wl0_akm", nvram_safe_get ("wl_akm"));
	  if (nvram_match ("wl_wep", "tkip"))
	    {
	      nvram_set ("wl_crypto", "tkip");
	    }
	  else if (nvram_match ("wl_wep", "aes"))
	    {
	      nvram_set ("wl_crypto", "aes");
	    }
	  else if (nvram_match ("wl_wep", "tkip+aes"))
	    {
	      nvram_set ("wl_crypto", "tkip+aes");
	    }

	  if (nvram_match ("wl_wep", "restricted"))
	    nvram_set ("wl_wep", "enabled");	// the nas need this value, the "restricted" is no longer need. (20040624 by honor)

	  cprintf ("RESTART\n");
	  if (nvram_match ("wl_akm", "wpa") ||
	      nvram_match ("wl_akm", "psk") ||
	      nvram_match ("wl_akm", "radius") ||
	      nvram_match ("wl_akm", "psk2") ||
	      nvram_match ("wl_akm", "wpa2") ||
	      nvram_match ("wl_akm", "wpa wpa2") ||
	      nvram_match ("wl_akm", "psk psk2"))
	    {
	      eval ("wlconf", nvram_safe_get ("wl0_ifname"), "down");
	      sleep (4);
	      start_service("wlconf");

	    }
	  /* Fall through */
	case STOP:
	  cprintf ("STOP\n");
	  system ("killall -9 udhcpc");
	  setenv ("PATH",
		  "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin",
		  1);
	  setenv ("LD_LIBRARY_PATH",
		  "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:",
		  1);
	  cprintf ("STOP WAN\n");
	  stop_service("wan");
	  cprintf ("STOP SERVICES\n");
	  stop_services ();
	  cprintf ("STOP LAN\n");
	  stop_service("lan");
#ifndef HAVE_RB500
	  cprintf ("STOP RESETBUTTON\n");
	  if ((brand != ROUTER_BELKIN) && (brand != ROUTER_BUFFALO_WBR2G54S) && (brand != ROUTER_SIEMENS) && (brand != ROUTER_BUFFALO_WZRRSG54))	//belkin doesnt like that
	    {
	      stop_service("resetbutton");
	    }
#endif
	  start_service("create_rc_shutdown");
	  system ("/tmp/.rc_shutdown");
	  if (state == STOP)
	    {
	      state = IDLE;
	      break;
	    }
	  /* Fall through */
	case START:
	  nvram_set ("wl0_lazy_wds", nvram_safe_get ("wl_lazy_wds"));
	  nvram_set ("wl0_akm", nvram_safe_get ("wl_akm"));
	  cprintf ("START\n");
	  setenv ("PATH",
		  "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin",
		  1);
	  setenv ("LD_LIBRARY_PATH",
		  "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:",
		  1);
	  start_service("ipv6");
#ifndef HAVE_RB500
	  if ((brand != ROUTER_BELKIN) && (brand != ROUTER_BUFFALO_WBR2G54S) && (brand != ROUTER_SIEMENS) && (brand != ROUTER_BUFFALO_WZRRSG54))	//belkin doesnt like that
	    {
	      start_service("resetbutton");
	    }
#endif
	  start_service("setup_vlans");
	  start_service("lan");
	  eval ("rm", "/tmp/hosts");
	  addHost ("localhost", "127.0.0.1");
	  if (strlen (nvram_safe_get ("wan_hostname")) > 0)
	    addHost (nvram_safe_get ("wan_hostname"),
		     nvram_safe_get ("lan_ipaddr"));
	  else if (strlen (nvram_safe_get ("router_name")) > 0)
	    addHost (nvram_safe_get ("router_name"),
		     nvram_safe_get ("lan_ipaddr"));
	  cprintf ("start services\n");
	  start_services ();
	  cprintf ("start wan boot\n");
	  start_service("wan_boot");
	  cprintf ("diaG STOP LED\n");
	  diag_led (DIAG, STOP_LED);
	  cprintf ("set led release wan control\n");
	  SET_LED (RELEASE_WAN_CONTROL);
	  cprintf ("ifconfig wl up\n");
	  if (nvram_match ("wl_mode", "sta")
	      || nvram_match ("wl_mode", "wet"))
	    {
	      //fix for client mode
	      if (wl_probe ("eth2"))
		eval ("/sbin/ifconfig", "eth1", "up");
	      else
		eval ("/sbin/ifconfig", "eth2", "up");
	    }
	  cprintf ("start nas\n");
	  start_service("nas_wan");
	  cprintf ("create rc file\n");
	  start_service("create_rc_startup");
	  chmod ("/tmp/.rc_startup", 0700);
	  system ("/tmp/.rc_startup");
	  //#ifdef HAVE_SER
	  system ("/etc/init.d/rcS");	// start openwrt startup script (siPath impl)
	  //#endif
	  //start_sambafs();
	  //start_macupd();
	  //start_rflow();
	  //start_radius();
	  cprintf ("start modules\n");
	  start_service("modules");
#ifdef HAVE_CHILLI
          start_service("chilli");
#endif
	  cprintf ("start syslog\n");
	  startstop("syslog");

	  system ("/etc/postinit");
//#ifdef HAVE_SKYTRON   
//eval("ifconfig","vlan1","172.16.1.1","255.255.255.0");
//#endif

/*			if (nvram_match("wl_mode", "bridge"))
				{
				if (wl_probe("eth2"))
				    eval("brctl", "addif", "br0", "eth1"); //create bridge
				else
				    eval("brctl", "addif", "br0", "eth2");
				}*/
	  diag_led (DIAG, STOP_LED);
	  /* Fall through */
	case TIMER:
	  cprintf ("TIMER\n");
	  do_timer ();
	  /* Fall through */
	case IDLE:
	  cprintf ("IDLE\n");
	  state = IDLE;
	  /* Wait for user input or state change */
	  while (signalled == -1)
	    {
	      if ((!shell_pid || kill (shell_pid, 0) != 0))
		shell_pid = ddrun_shell (0, 1);
	      else
		sigsuspend (&sigset);
	    }
	  state = signalled;
	  signalled = -1;
	  break;
	default:
	  cprintf ("UNKNOWN\n");
	  return;
	}
    }

}



int main(int argc,char **argv)
{
  char *base = strrchr (argv[0], '/');
      
  base = base ? base + 1 : argv[0];

  /* init */
  if (strstr (base, "init"))
    {
      main_loop ();
      return 0;
    }

  /* Set TZ for all rc programs */
  setenv ("TZ", nvram_safe_get ("time_zone"), 1);

  /* ppp */
  if (strstr (base, "ip-up"))
    return start_main ("ipup",argc, argv);
  else if (strstr (base, "ip-down"))
    return start_main ("updown",argc, argv);

  /* udhcpc [ deconfig bound renew ] */
  else if (strstr (base, "udhcpc"))
    return start_main("udhcpc",argc, argv);
#ifdef HAVE_PPTPD
  /* poptop [ stop start restart ]  */
  else if (strstr (base, "poptop"))
    return pptpd_main (argc, argv);
#endif
#ifndef HAVE_RB500
  /* erase [device] */
  else if (strstr (base, "erase"))
    {
      int brand = getRouterBrand ();
      if (brand == ROUTER_MOTOROLA)
	{
	  if (argv[1] && strcmp (argv[1], "nvram"))
	    {
	      fprintf (stderr,
		       "Sorry, erasing nvram will get the motorola unit unuseable\n");
	      return 0;
	    }
	}
      else
	{
	  if (argv[1])
	    return mtd_erase (argv[1]);
	  else
	    {
	      fprintf (stderr, "usage: erase [device]\n");
	      return EINVAL;
	    }
	}
    }

  /* write [path] [device] */
  else if (strstr (base, "write"))
    {
      if (argc >= 3)
	return mtd_write (argv[1], argv[2]);
      else
	{
	  fprintf (stderr, "usage: write [path] [device]\n");
	  return EINVAL;
	}
    }
#endif
  /* hotplug [event] */
  else if (strstr (base, "hotplug"))
    {
      if (argc >= 2)
	{
	  if (!strcmp (argv[1], "net"))
	    return start_service("hotplug_net");
	}
      else
	{
	  fprintf (stderr, "usage: hotplug [event]\n");
	  return EINVAL;
	}
    }
  /* rc [stop|start|restart ] */
  else if (strstr (base, "rc"))
    {
      if (argv[1])
	{
	  if (strncmp (argv[1], "start", 5) == 0)
	    return kill (1, SIGUSR2);
	  else if (strncmp (argv[1], "stop", 4) == 0)
	    return kill (1, SIGINT);
	  else if (strncmp (argv[1], "restart", 7) == 0)
	    return kill (1, SIGHUP);
	}
      else
	{
	  fprintf (stderr, "usage: rc [start|stop|restart]\n");
	  return EINVAL;
	}
    }

  //////////////////////////////////////////////////////
  //
  else if (strstr (base, "filtersync"))
    return start_service("filtersync");
  /* filter [add|del] number */
  else if (strstr (base, "filter"))
    {
      if (argv[1] && argv[2])
	{
	  int num = 0;
	  if ((num = atoi (argv[2])) > 0)
	    {
	      if (strcmp (argv[1], "add") == 0)
		return filter_add (num);
	      else if (strcmp (argv[1], "del") == 0)
		return filter_del (num);
	    }
	}
      else
	{
	  fprintf (stderr, "usage: filter [add|del] number\n");
	  return EINVAL;
	}
    }
  else if (strstr (base, "redial"))
    return redial_main (argc, argv);

  else if (strstr (base, "resetbutton"))
    {
#ifndef HAVE_RB500
      int brand = getRouterBrand ();
      if ((brand != ROUTER_BELKIN) && (brand != ROUTER_BUFFALO_WBR2G54S) && (brand != ROUTER_BUFFALO_WZRRSG54) && (brand != ROUTER_SIEMENS))	//belkin doesnt like that
	{
	  return resetbutton_main (argc, argv);
	}
      else
	{
	  fprintf (stderr,
		   "Belkin,Buffalo,Siemens S505 doesnt support the resetbutton!");
	  return 0;
	}
#endif
    }
#ifndef HAVE_MADWIFI
  else if (strstr (base, "wland"))
    return wland_main (argc, argv);
#endif
//  else if (strstr (base, "write_boot"))
//    return write_boot ("/tmp/boot.bin", "pmon");

#ifdef DEBUG_IPTABLE
  else if (strstr (base, "iptable_range"))
    return range_main (argc, argv);
  else if (strstr (base, "iptable_rule"))
    return rule_main (argc, argv);
#endif



  else if (strstr (base, "hb_connect"))
    return start_main("hb_connect",argc, argv);
  else if (strstr (base, "hb_disconnect"))
    return start_main("hb_disconnect",argc, argv);

  else if (strstr (base, "gpio"))
    return start_main("gpio",argc, argv);
  else if (strstr (base, "listen"))
    return listen_main(argc, argv);
  else if (strstr (base, "check_ps"))
    return check_ps_main (argc, argv);
  else if (strstr (base, "ddns_success"))
    return start_main("ddns_success",argc, argv);
//      else if (strstr(base, "eou_status"))
//                return eou_status_main();
  else if (strstr (base, "process_monitor"))
    return process_monitor_main ();
  else if (strstr (base, "restart_dns"))
    {
      stop_service("dnsmasq");
      stop_service("dhcpd");
      start_service("dhcpd");
      start_service("dnsmasq");
    }
  else if (strstr (base, "site_survey"))
    return start_main ("site_survey",argc, argv);
  else if (strstr (base, "setpasswd"))
    mkfiles ();
  else if (strstr (base, "wol"))
    wol_main ();
  else if (strstr (base, "sendudp"))
    return sendudp_main (argc, argv);
  else if (strstr (base, "check_ses_led"))
    return check_ses_led_main (argc, argv);

//  else if (strstr (base, "reboot"))
//    shutdown_system();

}