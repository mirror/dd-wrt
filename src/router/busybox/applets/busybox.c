/* vi: set sw=4 ts=4: */
/*
 * BusyBox' main applet dispatcher.
 *
 * Licensed under GPLv2, see file LICENSE in this tarball for details.
 */
#include "busybox.h"

const char *applet_name ATTRIBUTE_EXTERNALLY_VISIBLE;

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

#else
#define install_links(x,y)
#endif /* CONFIG_FEATURE_INSTALLER */

#ifdef HAVE_SSHD
  extern int dropbearkey_main(int argc,char *argv[]);
  extern int dropbearconvert_main(int argc,char *argv[]);
  extern int dropbear_main(int argc,char *argv[]);
  extern int cli_main(int argc,char *argv[]);
  extern int scp_main(int argc,char *argv[]);
#endif
#ifdef HAVE_ARP
  extern int arp_main(int argc,char *argv[]);
#endif
#ifdef HAVE_DHCPFWD
  extern int dhcpforward_main(int argc,char *argv[]);
#endif
#ifdef HAVE_RFLOW
  extern int rflow_main(int argc,char *argv[]);
#endif
#ifdef HAVE_BIRD
  extern int bird_main(int argc,char *argv[]);
#endif
int main(int argc, char **argv)
{
const char *s;
  char *base = strrchr (argv[0], '/');
      
  base = base ? base + 1 : argv[0];

#ifdef HAVE_DHCPFWD

  if (strstr (base, "dhcpfwd"))
    return dhcpforward_main(argc,argv);
#else
  if (strstr (base, "blahblah"))
    return puts(base);
#endif
/*#ifdef HAVE_PPP
  else if (strstr (base, "pppd"))
    return pppd_main(argc,argv);
#endif*/
#ifdef HAVE_RFLOW
  else if (strstr (base, "rflow"))
  {
    return rflow_main(argc,argv);
}
#endif
#ifdef HAVE_DDWRTHTTPD
  extern int httpd_main(int argc,char *argv[]);
  else if (strstr (base, "httpd"))
    return httpd_main(argc,argv);
#endif
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
#ifdef HAVE_BIRD
  else if (strstr (base, "bird"))
    return bird_main(argc, argv);
#endif    
#ifdef HAVE_ARP
  else if (!strcmp (base, "arp"))
    return arp_main(argc,argv);
#endif
//  else if (strstr (base, "dnsmasq"))
//    return dnsmasq_main(argc, argv);    
	applet_name=argv[0];
	if (*applet_name == '-') applet_name++;
	for (s = applet_name; *s ;)
		if (*(s++) == '/') applet_name = s;

	/* Set locale for everybody except 'init' */
	if (ENABLE_LOCALE_SUPPORT && getpid() != 1)
		setlocale(LC_ALL, "");

	run_applet_by_name(applet_name, argc, argv);
	bb_error_msg_and_die("applet not found");
}

int busybox_main(int argc, char **argv)
{
	/*
	 * This style of argument parsing doesn't scale well
	 * in the event that busybox starts wanting more --options.
	 * If someone has a cleaner approach, by all means implement it.
	 */
	if (ENABLE_FEATURE_INSTALLER && argc > 1 && !strcmp(argv[1], "--install")) {
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
// XXX: FIXME: this is broken. Why not just use argv[0] ?
		busybox = xreadlink("/proc/self/exe");
		if (busybox) {
			install_links(busybox, use_symbolic_links);
			free(busybox);
		} else {
			rc = 1;
		}
		return rc;
	}

	/* Deal with --help.  (Also print help when called with no arguments) */

	if (argc==1 || !strcmp(argv[1],"--help") ) {
		if (argc>2) {
			applet_name = argv[2];
			run_applet_by_name(applet_name, 2, argv);
		} else {
			const struct BB_applet *a;
			int col, output_width;

			if (ENABLE_FEATURE_AUTOWIDTH) {
				/* Obtain the terminal width.  */
				get_terminal_width_height(0, &output_width, NULL);
				/* leading tab and room to wrap */
				output_width -= sizeof("start-stop-daemon, ") + 8;
			} else output_width = 80 - sizeof("start-stop-daemon, ") - 8;

			printf("%s\n"
			       "Copyright (C) 1998-2006  Erik Andersen, Rob Landley, and others.\n"
			       "Licensed under GPLv2.  See source distribution for full notice.\n\n"
			       "Usage: busybox [function] [arguments]...\n"
			       "   or: [function] [arguments]...\n\n"
			       "\tBusyBox is a multi-call binary that combines many common Unix\n"
			       "\tutilities into a single executable.  Most people will create a\n"
			       "\tlink to busybox for each function they wish to use and BusyBox\n"
			       "\twill act like whatever it was invoked as!\n"
			       "\nCurrently defined functions:\n", bb_msg_full_version);

			col=0;
			for(a = applets; a->name;) {
				col += printf("%s%s", (col ? ", " : "\t"), (a++)->name);
				if (col > output_width && a->name) {
					printf(",\n");
					col = 0;
				}
			}
			printf("\n\n");
			exit(0);
		}
	} else run_applet_by_name(argv[1], argc-1, argv+1);

	bb_error_msg_and_die("applet not found");
}
