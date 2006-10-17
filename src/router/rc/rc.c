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
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <wlutils.h>
#include <cy_conf.h>

#include <revision.h>



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
static int noconsole = 0;

/* Main loop */
void
main_loop (void)
{
  sigset_t sigset;
  pid_t shell_pid = 0;
  uint boardflags;
  //setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin", 1);
  //system("/etc/nvram/nvram");
  /* Basic initialization */
  if (console_init ())
    noconsole = 1;

  start_service ("sysinit");

  /* Setup signal handlers */
  signal_init ();
  signal (SIGHUP, rc_signal);
  signal (SIGUSR1, rc_signal);	// Start single service from WEB, by honor
  signal (SIGUSR2, rc_signal);
  signal (SIGINT, rc_signal);
  signal (SIGALRM, rc_signal);
  sigemptyset (&sigset);

  /* Give user a chance to run a shell before bringing up the rest of the system */

  if (!noconsole)
    ddrun_shell (1, 0);

  start_service ("nvram");

  /* Restore defaults if necessary */
#ifdef HAVE_SKYTEL
  nvram_set ("vlan0ports", "0 1 2 3 4 5*");
  nvram_set ("vlan1ports", "");
#else

  if (nvram_match ("fullswitch", "1")
      && (nvram_invmatch ("wl_mode", "ap")
	  || nvram_match ("wan_proto", "disabled")))
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
  start_service ("restore_defaults");


  /* Add vlan */
  boardflags = strtoul (nvram_safe_get ("boardflags"), NULL, 0);
  nvram_set ("wanup", "0");


  int brand = getRouterBrand ();
#ifndef HAVE_RB500
  switch (brand)
    {
    case ROUTER_ASUS:
    case ROUTER_MOTOROLA:
    case ROUTER_SIEMENS:
    case ROUTER_BELKIN_F5D7230:
      start_service ("config_vlan");
      break;
    default:
      if (check_vlan_support ())
	{
	  start_service ("config_vlan");
	}
      break;

    }
#endif

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
#ifdef HAVE_MMC
  if (nvram_match ("mmc_enable", "1"))
    {
      if (!eval ("insmod", "mmc"))
	{
	  //device detected
	  eval ("insmod", "ext2");
	  if (mount
	      ("/dev/mmc/disc0/part1", "/mmc", "ext2", MS_MGC_VAL, NULL))
	    {
	      //device not formated
	      eval ("/sbin/mke2fs", "-F", "-b", "1024",
		    "/dev/mmc/disc0/part1");
	      mount ("/dev/mmc/disc0/part1", "/mmc", "ext2", MS_MGC_VAL,
		     NULL);
	    }
	}
    }

#endif


  start_service ("mkfiles");
  char *hostname;

  /* set hostname to wan_hostname or router_name */
  if (strlen (nvram_safe_get ("wan_hostname")) > 0)
    hostname = nvram_safe_get ("wan_hostname");
  else if (strlen (nvram_safe_get ("router_name")) > 0)
    hostname = nvram_safe_get ("router_name");
  else
    hostname = "dd-wrt";

  sethostname (hostname, strlen (hostname));
  stop_service ("httpd");
  if (brand == ROUTER_SIEMENS)
    {
      start_service ("powerled_ctrl_1");
    }
//create loginprompt
  FILE *fp = fopen ("/tmp/loginprompt", "wb");

#ifndef HAVE_MSSID
#ifdef DIST
  if (strlen (DIST) > 0)
    fprintf (fp,
	     "DD-WRT v23 SP3 %s (c) 2006 NewMedia-NET GmbH\nRelease: "
	     BUILD_DATE " (SVN revision: %s)\n", DIST, SVN_REVISION);
  else
    fprintf (fp,
	     "DD-WRT v23 SP3 custom (c) 2006 NewMedia-NET GmbH\nRelease: "
	     BUILD_DATE " (SVN revision: %s)\n", SVN_REVISION);
#else
  fprintf (fp,
	   "DD-WRT v23 SP3 custom (c) 2006 NewMedia-NET GmbH\nRelease: "
	   BUILD_DATE " (SVN revision: %s)\n", SVN_REVISION);
#endif
#else
#ifdef DIST
  if (strlen (DIST) > 0)
    fprintf (fp,
	     "DD-WRT v24 %s (c) 2006 NewMedia-NET GmbH\nRelease: " BUILD_DATE
	     " (SVN revision: %s)\n", DIST, SVN_REVISION);
  else
    fprintf (fp,
	     "DD-WRT v24 custom (c) 2006 NewMedia-NET GmbH\nRelease: "
	     BUILD_DATE " (SVN revision: %s)\n", SVN_REVISION);
#else
  fprintf (fp,
	   "DD-WRT v24 custom (c) 2006 NewMedia-NET GmbH\nRelease: "
	   BUILD_DATE " (SVN revision: %s)\n", SVN_REVISION);
#endif
#endif


  fclose (fp);

  /* Loop forever */
  for (;;)
    {
      switch (state)
	{
	case USER:		// Restart single service from WEB of tftpd, by honor
	  cprintf ("USER1\n");
	  start_single_service ();
#ifdef HAVE_CHILLI
	  start_service ("chilli");
#endif
#ifdef HAVE_WIFIDOG
	  start_service ("wifidog");
#endif

	  state = IDLE;
	  break;
	case RESTART:
	  start_service ("overclocking");
	  cprintf ("RESET NVRAM VARS\n");
	  nvram_set ("wl0_lazy_wds", nvram_safe_get ("wl_lazy_wds"));
#ifndef HAVE_MSSID
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
#endif

	  cprintf ("RESTART\n");
#ifndef HAVE_MSSID

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
	      start_service ("wlconf");

	    }
#endif
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
	  stop_service ("wan");
	  cprintf ("STOP SERVICES\n");
	  stop_services ();
	  cprintf ("STOP LAN\n");
          stop_service ("stabridge");
	  stop_service ("lan");
#ifndef HAVE_RB500
	  cprintf ("STOP RESETBUTTON\n");
	  if ((brand & 0x000f) != 0x000f)
/*	  
	  	  if ((brand == ROUTER_WRT54G) ||
	      (brand == ROUTER_WRT54G1X) ||
	      (brand == ROUTER_WRTSL54GS) ||
	      (brand == ROUTER_LINKSYS_WRT55AG) ||
	      (brand == ROUTER_ASUS) ||
	      (brand == ROUTER_BUFFALO_WBR54G) ||
	      (brand == ROUTER_BUFFALO_WHRG54S) ||
	      (brand == ROUTER_MOTOROLA_V1) ||
	      (brand == ROUTER_BOARD_500) ||
	      (brand == ROUTER_BUFFALO_WZRRSG54) ||
	      (brand == ROUTER_BUFFALO_WBR2G54S))  */
	    {
	      stop_service ("resetbutton");
	    }
#endif
	  start_service ("create_rc_shutdown");
	  system ("/tmp/.rc_shutdown");
	  if (state == STOP)
	    {
	      state = IDLE;
	      break;
	    }
	  /* Fall through */
	case START:
	  nvram_set ("wl0_lazy_wds", nvram_safe_get ("wl_lazy_wds"));
#ifndef HAVE_MSSID
	  nvram_set ("wl0_akm", nvram_safe_get ("wl_akm"));
#endif
	  cprintf ("START\n");
	  setenv ("PATH",
		  "/sbin:/bin:/usr/sbin:/usr/bin:/jffs/sbin:/jffs/bin:/jffs/usr/sbin:/jffs/usr/bin",
		  1);
	  setenv ("LD_LIBRARY_PATH",
		  "/lib:/usr/lib:/jffs/lib:/jffs/usr/lib:/mmc/lib:/mmc/usr/lib:",
		  1);
	  start_service ("ipv6");
#ifndef HAVE_RB500
	  if ((brand & 0x000f) != 0x000f)
/*
	  if ((brand == ROUTER_WRT54G) ||
	      (brand == ROUTER_WRT54G1X) ||
	      (brand == ROUTER_WRTSL54GS) ||
	      (brand == ROUTER_LINKSYS_WRT55AG) ||
	      (brand == ROUTER_ASUS) ||
	      (brand == ROUTER_BUFFALO_WBR54G) ||
	      (brand == ROUTER_BUFFALO_WHRG54S) ||
	      (brand == ROUTER_MOTOROLA_V1) ||
	      (brand == ROUTER_BOARD_500) ||
	      (brand == ROUTER_BUFFALO_WZRRSG54) ||
	      (brand == ROUTER_BUFFALO_WBR2G54S))  */
	    {
	      start_service ("resetbutton");
	    }
#endif
	  start_service ("setup_vlans");
	  start_service ("lan");
          start_service ("stabridge");
	  cprintf ("start services\n");
	  start_services ();
	  cprintf ("start wan boot\n");
	  start_service ("wan_boot");
	  cprintf ("diaG STOP LED\n");
	  diag_led (DIAG, STOP_LED);
	  cprintf ("set led release wan control\n");
	  SET_LED (RELEASE_WAN_CONTROL);
	  cprintf ("ifconfig wl up\n");
	  if (nvram_match ("wl_mode", "sta")
	      || nvram_match ("wl_mode", "wet")
	      || nvram_match ("wl_mode", "apsta"))
	    {
	      //fix for client mode
	      if (wl_probe ("eth2"))
		eval ("/sbin/ifconfig", "eth1", "up");
	      else
		eval ("/sbin/ifconfig", "eth2", "up");
	    }
	  cprintf ("start nas\n");
	  start_service ("nas_wan");
	  cprintf ("create rc file\n");
#ifdef HAVE_REGISTER
  if (isregistered())
#endif
	{
	  start_service ("create_rc_startup");
	  chmod ("/tmp/.rc_startup", 0700);
	  system ("/tmp/.rc_startup");
	  system ("/etc/init.d/rcS");	// start openwrt startup script (siPath impl)
	  cprintf ("start modules\n");
	  start_service ("modules");
	}
#ifdef HAVE_CHILLI
	  start_service ("chilli");
#endif
#ifdef HAVE_WIFIDOG
	  start_service ("wifidog");
#endif
	  cprintf ("start syslog\n");
	  startstop ("syslog");

	  system ("/etc/postinit");

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
	      if (!noconsole && (!shell_pid || kill (shell_pid, 0) != 0))
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



int
main (int argc, char **argv)
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

  if (strstr (base, "startservice"))
    {
      if (argc < 2)
	{
	  puts ("try to be professional\n");
	  return 0;
	}
      return start_service (argv[1]);
    }
  if (strstr (base, "stopservice"))
    {
      if (argc < 2)
	{
	  puts ("try to be professional\n");
	  return 0;
	}
      return stop_service (argv[1]);
    }

  /* ppp */
  if (strstr (base, "ip-up"))
    return start_main ("ipup", argc, argv);
  else if (strstr (base, "ip-down"))
    return start_main ("ipdown", argc, argv);
//  else if (strstr(base, "set-pppoepid")) //tallest 1219
//     return start_main("set_pppoepid_to_nv",argc, argv);
  else if (strstr (base, "disconnected_pppoe"))	//by tallest 0407
    return start_main ("disconnected_pppoe", argc, argv);
//  else if (strstr(base, "ppp_event")) 
//    return start_main("pppevent_main",argc, argv);

  /* udhcpc [ deconfig bound renew ] */
  else if (strstr (base, "udhcpc"))
    return start_main ("udhcpc", argc, argv);
#ifdef HAVE_PPTP
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
	  cprintf ("hotplug %s\n", argv[1]);
	  if (!strcmp (argv[1], "net"))
	    return start_service ("hotplug_net");
#ifdef HAVE_MEDIASERVER
	  if (!strcmp (argv[1], "usb"))
	    return start_service ("hotplug_usb");
#endif
#ifdef HAVE_XSCALE
	  if (!strcmp (argv[1], "firmware"))
	    return eval ("/etc/upload", argv[1]);
#endif
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
    return start_service ("filtersync");
  /* filter [add|del] number */
  else if (strstr (base, "filter"))
    {
      if (argv[1] && argv[2])
	{
	  int num = 0;
	  if ((num = atoi (argv[2])) > 0)
	    {
	      if (strcmp (argv[1], "add") == 0)
		return start_servicei ("filter_add", num);
	      else if (strcmp (argv[1], "del") == 0)
		return start_servicei ("filter_del", num);
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
      if ((brand & 0x000f) != 0x000f)
	{
	  return resetbutton_main (argc, argv);
	}
      else
	{
	  fprintf (stderr,
		   "Your router model doesnt support the resetbutton!\n");
	  return 0;
	}
/*	  	
      int brand = getRouterBrand ();
//      fprintf(stderr,"brand = %d\n",brand);
      switch (brand)
	{
	case ROUTER_WRT54G:
	case ROUTER_WRT54G1X:
	case ROUTER_WRTSL54GS:
	case ROUTER_LINKSYS_WRT55AG:
	case ROUTER_ASUS:
	case ROUTER_BUFFALO_WBR54G:
	case ROUTER_BUFFALO_WHRG54S:
	case ROUTER_MOTOROLA_V1:
	case ROUTER_BUFFALO_WBR2G54S:
	case ROUTER_BUFFALO_WZRRSG54:
	  return resetbutton_main (argc, argv);
	default:
	  fprintf (stderr,
		   "Your router model doesnt support the resetbutton!\n");
	  return 0;
	}
*/
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
    return start_main ("hb_connect", argc, argv);
  else if (strstr (base, "hb_disconnect"))
    return start_main ("hb_disconnect", argc, argv);
#ifndef HAVE_XSCALE
  else if (strstr (base, "gpio"))
    return start_main ("gpio", argc, argv);
#endif
  else if (strstr (base, "listen"))
    return listen_main (argc, argv);
  else if (strstr (base, "check_ps"))
    return check_ps_main (argc, argv);
  else if (strstr (base, "ddns_success"))
    return start_main ("ddns_success", argc, argv);
//      else if (strstr(base, "eou_status"))
//                return eou_status_main();
  else if (strstr (base, "process_monitor"))
    return process_monitor_main ();
  else if (strstr (base, "radio_timer"))
    return radio_timer_main ();
  else if (strstr (base, "restart_dns"))
    {
      stop_service ("dnsmasq");
      stop_service ("udhcpd");
      start_service ("udhcpd");
      start_service ("dnsmasq");
    }
  else if (strstr (base, "site_survey"))
    return start_main ("site_survey", argc, argv);
  else if (strstr (base, "setpasswd"))
    start_service ("mkfiles");
#ifdef HAVE_WOL
  else if (strstr (base, "wol"))
    wol_main ();
#endif
  else if (strstr (base, "sendudp"))
    return sendudp_main (argc, argv);
  else if (strstr (base, "check_ses_led"))
    return check_ses_led_main (argc, argv);
#ifdef HAVE_MICRO
  else if (strstr (base, "brctl"))
    return start_main ("brctl", argc, argv);
#endif
  else if (strstr (base, "watchdog"))
    return watchdog_main (argc, argv);
//  else if (strstr (base, "reboot"))
//    shutdown_system();

}
