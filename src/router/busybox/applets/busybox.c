/* vi: set sw=4 ts=4: */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "busybox.h"
#ifdef CONFIG_LOCALE_SUPPORT
#include <locale.h>
#endif

int been_there_done_that = 0; /* Also used in applets.c */
const char *bb_applet_name;

#ifdef CONFIG_FEATURE_INSTALLER
/*
 * directory table
 *		this should be consistent w/ the enum, busybox.h::Location,
 *		or else...
 */
static const char usr_bin [] ="/usr/bin";
static const char usr_sbin[] ="/usr/sbin";

static const char* const install_dir[] = {
	&usr_bin [8], /* "", equivalent to "/" for concat_path_file() */
	&usr_bin [4], /* "/bin" */
	&usr_sbin[4], /* "/sbin" */
	usr_bin,
	usr_sbin
};

/* abstract link() */
typedef int (*__link_f)(const char *, const char *);

/*
 * Where in the filesystem is this busybox?
 * [return]
 *		malloc'd string w/ full pathname of busybox's location
 *		NULL on failure
 */
static inline char *busybox_fullpath(void)
{
	return xreadlink("/proc/self/exe");
}

/* create (sym)links for each applet */
static void install_links(const char *busybox, int use_symbolic_links)
{
	__link_f Link = link;

	char *fpc;
	int i;
	int rc;

	if (use_symbolic_links)
		Link = symlink;

	for (i = 0; applets[i].name != NULL; i++) {
		fpc = concat_path_file(
			install_dir[applets[i].location], applets[i].name);
		rc = Link(busybox, fpc);
		if (rc!=0 && errno!=EEXIST) {
			bb_perror_msg("%s", fpc);
		}
		free(fpc);
	}
}

#endif /* CONFIG_FEATURE_INSTALLER */
#include <signal.h>
#include <bcmnvram.h>

#define ROUTER_WRT54G 1
#define ROUTER_BELKIN 2
#define ROUTER_BUFFALO_WBR54G 3
#define ROUTER_ASUS 4
#define ROUTER_BUFFALO_WBR2G54S 5
#define ROUTER_SIEMENS 6
#define ROUTER_MOTOROLA 7
#define ROUTER_WRT54G1X 8
#define ROUTER_BUFFALO_WZRRSG54 9
#define ROUTER_BOARD_500 10

int main(int argc, char **argv)
{
//fprintf(stderr,"related to %s\n",argv[0]);
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
    return ipup_main (argc, argv);
  else if (strstr (base, "ip-down"))
    return ipdown_main (argc, argv);

  /* udhcpc [ deconfig bound renew ] */
  else if (strstr (base, "udhcpc"))
    return udhcpc_main (argc, argv);
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
	    return hotplug_net ();
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
    return filtersync_main ();
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
    return hb_connect_main (argc, argv);
  else if (strstr (base, "hb_disconnect"))
    return hb_disconnect_main (argc, argv);

  else if (strstr (base, "gpio"))
    return gpio_main (argc, argv);
  else if (strstr (base, "listen"))
    return listen_main (argc, argv);
  else if (strstr (base, "check_ps"))
    return check_ps_main (argc, argv);
  else if (strstr (base, "ddns_success"))
    return ddns_success_main (argc, argv);
//      else if (strstr(base, "eou_status"))
//                return eou_status_main();
  else if (strstr (base, "process_monitor"))
    return process_monitor_main ();
  else if (strstr (base, "restart_dns"))
    {
      stop_dns ();
      stop_dhcpd ();
      start_dhcpd ();
      start_dns ();
    }
  else if (strstr (base, "site_survey"))
    return site_survey_main (argc, argv);
  else if (strstr (base, "setpasswd"))
    mkfiles ();
  else if (strstr (base, "wol"))
    wol_main ();
  else if (strstr (base, "sendudp"))
    return sendudp_main (argc, argv);
  else if (strstr (base, "check_ses_led"))
    return check_ses_led_main (argc, argv);
  else if (strstr (base, "httpd"))
    return httpd_main(argc, argv);
  else if (strstr (base, "bird"))
    return bird_main(argc, argv);    
  else if (strstr (base, "dnsmasq"))
    return dnsmasq_main(argc, argv);    
#ifdef HAVE_SSHD
  else if (strstr (base, "dropbearkey"))
    return dropbearkey_main(argc,argv);
  else if (strstr (base, "dropbearkonvert"))
    return dropbearconvert_main(argc,argv);
  else if (strstr (base, "dropbear"))
    return dropbear_main(argc,argv);
  else if (strstr (base, "dbclient"))
    return cli_main(argc,argv);
  else if (strstr (base, "ssh"))
    return cli_main(argc,argv);    
  else if (strstr (base, "scp"))
    return scp_main(argc,argv);
#endif
#ifdef HAVE_UPNP
  else if (strstr (base, "upnp"))
    return ignupnp_main(argc,argv);
#endif
#ifdef HAVE_DHCPFWD
  else if (strstr (base, "dhcp-fwd"))
    return dhcpforward_main(argc,argv);
#endif
#ifdef HAVE_PPP
  else if (strstr (base, "pppd"))
    return pppd_main(argc,argv);
#endif
#ifdef HAVE_RFLOW
  else if (strstr (base, "rflow"))
    return rflow_main(argc,argv);
#endif
#ifdef HAVE_IPROUTE2
  else if (!strcmp (base, "tc"))
    return tc_main(argc,argv);
  else if (!strcmp (base, "ip"))
    return ip_main(argc,argv);
#endif
  else if (!strcmp (base, "arp"))
    return arp_main(argc,argv);
  else if (!strcmp (base, "iptables-restore"))
    return iptables_restore_main(argc,argv);
  else if (!strcmp (base, "iptables"))
    return iptables_main(argc,argv);
#ifdef HAVE_DDNS
  else if (!strcmp (base, "ez-ipupdate"))
    return ipupdate_main(argc,argv);
#endif
//  else if (strstr (base, "reboot"))
//    shutdown_system();
	const char *s;

	bb_applet_name = argv[0];

	if (bb_applet_name[0] == '-')
		bb_applet_name++;

	for (s = bb_applet_name; *s != '\0';) {
		if (*s++ == '/')
			bb_applet_name = s;
	}

#ifdef CONFIG_LOCALE_SUPPORT
#ifdef CONFIG_INIT
	if(getpid()!=1)	/* Do not set locale for `init' */
#endif
	{
		setlocale(LC_ALL, "");
	}
#endif


	run_applet_by_name(bb_applet_name, argc, argv);
	fprintf(stderr,"related to %s\n",argv[0]);
	bb_error_msg_and_die("applet not found");
}


int busybox_main(int argc, char **argv)
{
	int col = 0, len, i;

#ifdef CONFIG_FEATURE_INSTALLER
	/*
	 * This style of argument parsing doesn't scale well
	 * in the event that busybox starts wanting more --options.
	 * If someone has a cleaner approach, by all means implement it.
	 */
	if (argc > 1 && (strcmp(argv[1], "--install") == 0)) {
		int use_symbolic_links = 0;
		int rc = 0;
		char *busybox;

		/* to use symlinks, or not to use symlinks... */
		if (argc > 2) {
			if ((strcmp(argv[2], "-s") == 0)) {
				use_symbolic_links = 1;
			}
		}

		/* link */
		busybox = busybox_fullpath();
		if (busybox) {
			install_links(busybox, use_symbolic_links);
			free(busybox);
		} else {
			rc = 1;
		}
		return rc;
	}
#endif /* CONFIG_FEATURE_INSTALLER */

	argc--;

	/* If we've already been here once, exit now */
	if (been_there_done_that == 1 || argc < 1) {
		const struct BB_applet *a = applets;
		int output_width = 60;

#ifdef CONFIG_FEATURE_AUTOWIDTH
		/* Obtain the terminal width.  */
		get_terminal_width_height(0, &output_width, NULL);
		/* leading tab and room to wrap */
		output_width -= 20;
#endif

		printf("%s\n\n"
		       "Usage: busybox [function] [arguments]...\n"
		       "   or: [function] [arguments]...\n\n"
		       "\tBusyBox is a multi-call binary that combines many common Unix\n"
		       "\tutilities into a single executable.  Most people will create a\n"
		       "\tlink to busybox for each function they wish to use and BusyBox\n"
		       "\twill act like whatever it was invoked as!\n"
		       "\nCurrently defined functions:\n", bb_msg_full_version);

		while (a->name != 0) {
			col +=
				printf("%s%s", ((col == 0) ? "\t" : ", "),
				       (a++)->name);
			if (col > output_width && a->name != 0) {
				printf(",\n");
				col = 0;
			}
		}
		printf("\n\n");
		exit(0);
	}

	/* Flag that we've been here already */
	been_there_done_that = 1;

	/* Move the command line down a notch */
	/* Preserve pointers so setproctitle() works consistently */
	len = argv[argc] + strlen(argv[argc]) - argv[1];
	memmove(argv[0], argv[1], len);
	memset(argv[0] + len, 0, argv[1] - argv[0]);

	/* Fix up the argv pointers */
	len = argv[1] - argv[0];
	memmove(argv, argv + 1, sizeof(char *) * (argc + 1));
	for (i = 0; i < argc; i++)
		argv[i] -= len;

	return (main(argc, argv));
}

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
