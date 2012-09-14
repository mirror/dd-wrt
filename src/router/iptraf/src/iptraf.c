/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/*
IPTraf
An IP Network Statistics Utility
*/

#define MAIN_MODULE

#include "iptraf-ng-compat.h"

#include "tui/menurt.h"
#include "tui/winops.h"

#include "dirs.h"
#include "deskman.h"
#include "fltdefs.h"
#include "fltselect.h"
#include "fltmgr.h"
#include "fltedit.h"
#include "serv.h"
#include "options.h"
#include "attrs.h"
#include "rvnamed.h"
#include "logvars.h"
#include "detstats.h"
#include "ifstats.h"
#include "itrafmon.h"
#include "pktsize.h"
#include "hostmon.h"

#include "parse-options.h"

#define WITHALL 1
#define WITHOUTALL 0

const char *ALLSPEC = "all";

/*
 * Important globals used throughout the
 * program.
 */
int exitloop = 0;
int daemonized = 0;
int facility_running = 0;
int is_first_instance;
char graphing_filter[80];

extern void about(void);

static void press_enter_to_continue(void)
{
	fprintf(stderr, "Press Enter to continue.\n");
	getchar();
}

static void clearfiles(char *prefix, char *directory)
{
	DIR *dir;
	struct dirent *dir_entry;
	char target_name[80];

	dir = opendir(directory);

	if (dir == NULL) {
		fprintf(stderr, "\nUnable to read directory %s\n%s\n",
			directory, strerror(errno));
		press_enter_to_continue();
		return;
	}

	do {
		dir_entry = readdir(dir);
		if (dir_entry != NULL) {
			if (strncmp(dir_entry->d_name, prefix, strlen(prefix))
			    == 0) {
				snprintf(target_name, 80, "%s/%s", directory,
					 dir_entry->d_name);
				unlink(target_name);
			}
		}
	} while (dir_entry != NULL);

	closedir(dir);
}

static void removetags(void)
{
	clearfiles("iptraf", LOCKDIR);
}

static void remove_sockets(void)
{
	clearfiles(SOCKET_PREFIX, WORKDIR);
}

/*
 * USR2 handler.  Used to normally exit a daemonized facility.
 */

static void term_usr2_handler(int s __unused)
{
	exitloop = 1;
}

static void init_break_menu(struct MENU *break_menu)
{
	tx_initmenu(break_menu, 6, 20, (LINES - 6) / 2, COLS / 2, BOXATTR,
		    STDATTR, HIGHATTR, BARSTDATTR, BARHIGHATTR, DESCATTR);
	tx_additem(break_menu, " By packet ^s^ize",
		   "Displays packet counts by packet size range");
	tx_additem(break_menu, " By TCP/UDP ^p^ort",
		   "Displays packet and byte counts by service port");
	tx_additem(break_menu, NULL, NULL);
	tx_additem(break_menu, " E^x^it menu", "Return to main menu");
}

/*
 * Get the ball rolling: The program interface routine.
 */

static void program_interface(struct OPTIONS *options)
{
	struct MENU menu;
	struct MENU break_menu;

	int endloop = 0;
	int row = 1;
	int break_row = 1;
	int aborted;
	int break_aborted;

	struct filterstate ofilter;
	struct ffnode *fltfiles;

	char ifname[IFNAMSIZ];
	char *ifptr = NULL;
	struct porttab *ports;

	/*
	 * Load saved filter or graphing filter if specified
	 */
	if (graphing_logfile[0] != '\0') {
		loadfilterlist(&fltfiles);
		memset(&ofilter, 0, sizeof(struct filterstate));
		loadfilter(pickfilterbyname(fltfiles, graphing_filter),
			   &(ofilter.fl), FLT_RESOLVE);
	} else {
		loadfilters(&ofilter);
		indicate("");
	}

	loadaddports(&ports);

	tx_initmenu(&menu, 15, 35, (LINES - 16) / 2, (COLS - 35) / 2, BOXATTR,
		    STDATTR, HIGHATTR, BARSTDATTR, BARHIGHATTR, DESCATTR);

	tx_additem(&menu, " IP traffic ^m^onitor",
		   "Displays current IP traffic information");
	tx_additem(&menu, " General interface ^s^tatistics",
		   "Displays some statistics for attached interfaces");
	tx_additem(&menu, " ^D^etailed interface statistics",
		   "Displays more statistics for a selected interface");
	tx_additem(&menu, " Statistical ^b^reakdowns...",
		   "Facilities for traffic counts by packet size or TCP/UDP port");
	tx_additem(&menu, " ^L^AN station monitor",
		   "Displays statistics on detected LAN stations");
	tx_additem(&menu, NULL, NULL);
	tx_additem(&menu, " ^F^ilters...",
		   "Allows you to select traffic display and logging criteria");
	tx_additem(&menu, NULL, NULL);
	tx_additem(&menu, " C^o^nfigure...", "Set various program options");
	tx_additem(&menu, NULL, NULL);
	tx_additem(&menu, " ^A^bout...", "Displays program info");
	tx_additem(&menu, NULL, NULL);
	tx_additem(&menu, " E^x^it", "Exits program");

	endloop = 0;

	do {
		tx_showmenu(&menu);
		tx_operatemenu(&menu, &row, &aborted);

		switch (row) {
		case 1:
			selectiface(ifname, WITHALL, &aborted);
			if (!aborted) {
				if (strcmp(ifname, "") != 0)
					ifptr = ifname;
				else
					ifptr = NULL;

				ipmon(options, &ofilter, 0, ifptr);
			}
			break;
		case 2:
			ifstats(options, &ofilter, 0);
			break;
		case 3:
			selectiface(ifname, WITHOUTALL, &aborted);
			if (!aborted)
				detstats(ifname, options, 0, &ofilter);
			break;
		case 4:
			break_row = 1;
			init_break_menu(&break_menu);
			tx_showmenu(&break_menu);
			tx_operatemenu(&break_menu, &break_row, &break_aborted);

			switch (break_row) {
			case 1:
				selectiface(ifname, WITHOUTALL, &aborted);
				if (!aborted)
					packet_size_breakdown(options, ifname,
							      0, &ofilter);
				break;
			case 2:
				selectiface(ifname, WITHOUTALL, &aborted);
				if (!aborted)
					servmon(ifname, ports, options, 0,
						&ofilter);
				break;
			case 4:
				break;
			}
			tx_destroymenu(&break_menu);
			break;
		case 5:
			selectiface(ifname, WITHALL, &aborted);
			if (!aborted) {
				if (strcmp(ifname, "") != 0)
					ifptr = ifname;
				else
					ifptr = NULL;
				hostmon(options, 0, ifptr, &ofilter);
			}
			break;
		case 7:
			config_filters(&ofilter);
			savefilters(&ofilter);
			break;
		case 9:
			setoptions(options, &ports);
			saveoptions(options);
			break;
		case 11:
			about();
			break;
		case 13:
			endloop = 1;
			break;
		}
	} while (!endloop);

	tx_destroymenu(&menu);

	destroyporttab(ports);
	erase();
	update_panels();
	doupdate();
}

static int first_instance(void)
{
	int fd;

	fd = open(IPTIDFILE, O_RDONLY);

	if (fd < 0)
		return !0;
	else {
		close(fd);
		return 0;
	}
}

static void mark_first_instance(void)
{
	int fd;

	fd = open(IPTIDFILE, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		fprintf(stderr, "\nWarning: unable to tag this process\r\n");
		press_enter_to_continue();
		return;
	}
	close(fd);
}

static const char *const iptraf_ng_usage[] = {
	IPTRAF_NAME " [options]",
	IPTRAF_NAME " [options] -B [-i <iface> | -d <iface> | -s <iface> | -z <iface> | -l <iface> | -g]",
	NULL
};

static int help_opt, f_opt, g_opt, facilitytime, B_opt, I_opt;
static char *i_opt, *d_opt, *s_opt, *z_opt, *l_opt, *L_opt;

static struct options iptraf_ng_options[] = {
	OPT__HELP(&help_opt),
	OPT_GROUP(""),
	OPT_STRING('i', NULL, &i_opt, "iface",
		   "start the IP traffic monitor (use '-i all' for all interfaces)"),
	OPT_STRING('d', NULL, &d_opt, "iface",
		   "start the detailed statistics facility on an interface"),
	OPT_STRING('s', NULL, &s_opt, "iface",
		   "start the TCP and UDP monitor on an interface"),
	OPT_STRING('z', NULL, &z_opt, "iface",
		   "shows the packet size counts on an interface"),
	OPT_STRING('l', NULL, &l_opt, "iface",
		   "start the LAN station monitor (use '-l all' for all LAN interfaces)"),
	OPT_BOOL('g', NULL, &g_opt, "start the general interface statistics"),
	OPT_GROUP(""),
	OPT_BOOL('B', NULL, &B_opt,
		 "run in background (use only with one of the above parameters"),
	OPT_BOOL('f', NULL, &f_opt,
		 "clear all locks and counters"
		 /*. Use with great caution. Normally used to recover from an abnormal termination */
	    ),
	OPT_INTEGER('t', NULL, &facilitytime,
		    "run only for the specified <n> number of minutes"),
	OPT_STRING('L', NULL, &L_opt, "logfile",
		   "specifies an alternate log file"),
	//    OPT_INTEGER('I', NULL, &I_opt, "the log interval for all facilities except the IP traffic monitor. Value is in minutes"),
	OPT_END()
};

static void sanitize_dir(const char *dir)
{
	/* Check whether LOCKDIR exists (/var/run is on a tmpfs in Ubuntu) */
	if (access(dir, F_OK) != 0) {
		if (mkdir(dir, 0700) == -1)
			die("Cannot create %s: %s", dir, strerror(errno));

		if (chown(dir, 0, 0) == -1)
			die("Cannot change owner of %s: %s", dir,
			    strerror(errno));
	}
}

int main(int argc, char **argv)
{
	struct OPTIONS options;
	int command = 0;
	int current_log_interval = 0;

#ifndef ALLOWUSERS
	if (geteuid() != 0)
		die("This program can be run only by the system administrator");
#endif

	/*
	 * Parse command line
	 */

	parse_opts(argc, argv, iptraf_ng_options, iptraf_ng_usage);

	if (help_opt)
		parse_usage_and_die(iptraf_ng_usage, iptraf_ng_options);

	int status = 0;

	status |= (i_opt) ? (1 << 0) : 0;
	status |= (d_opt) ? (1 << 1) : 0;
	status |= (s_opt) ? (1 << 2) : 0;
	status |= (z_opt) ? (1 << 3) : 0;
	status |= (l_opt) ? (1 << 4) : 0;
	status |= (g_opt) ? (1 << 5) : 0;

	if (__builtin_popcount(status) > 1)
		die("only one of -i|-d|-s|-z|-l|-g options must be used");

	strcpy(current_logfile, "");
	strcpy(graphing_logfile, "");
	strcpy(graphing_filter, "");

	if (f_opt) {
		removetags();
		remove_sockets();
	}

	if (B_opt) {
		if (!status)
			die("one of -i|-d|-s|-z|-l|-g option is missing\n");
		daemonized = 1;
		setenv("TERM", "linux", 1);
	}

	if (L_opt) {
		if (strchr(L_opt, '/') != NULL)
			strncpy(current_logfile, L_opt, 80);
		else
			strncpy(current_logfile, get_path(T_LOGDIR, L_opt), 80);
	}
#if 0				/* this could never work */
	/* origin
	   } else if (opt == 'I') {
	   //this could never work
	   current_log_interval = atoi(optarg);
	   if (current_log_interval == 0)
	   fprintf(stderr, "Invalid log interval value\n");

	   exit(1);
	   } else if (opt == 'G') {
	 */
	if (I_opt == 0) {
		fprintf(stderr, "fatal: Invalid log interval value\n");
		exit(1);
	} else
		current_log_interval = I_opt;
#endif

	is_first_instance = first_instance();

	if ((getenv("TERM") == NULL) && (!daemonized))
		die("Your TERM variable is not set.\n"
		    "Please set it to an appropriate value");

#if 0				/* undocumented feature, will take care of it later */
	if (graphing_logfile[0] != '\0' && graphing_filter[0] == '\0') {
		fprintf(stderr, "Specify an IP filter name with -F\n");
		exit(1);
	}
#endif
	loadoptions(&options);

	/*
	 * If a facility is directly invoked from the command line, check for
	 * a daemonization request
	 */

	if ((daemonized) && (command != 0)) {
		switch (fork()) {
		case 0:	/* child */
			setsid();
			freopen("/dev/null", "w", stdout);	/* redirect std output */
			freopen("/dev/null", "r", stdin);	/* redirect std input */
			freopen("/dev/null", "w", stderr);	/* redirect std error */
			signal(SIGUSR2, term_usr2_handler);

			if (graphing_logfile[0] != '\0')
				options.logging = 0;	/* if raw logging is specified */
			else	/* then standard logging is disabled */
				options.logging = 1;
			break;
		case -1:	/* error */
			die("Fork error, %s cannot run in background", IPTRAF_NAME);
		default:	/* parent */
			exit(0);
		}
	}
#ifdef SIMDAEMON
	daemonized = 1;
	freopen("/dev/null", "w", stdout);	/* redirect std output */
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stderr);
#endif

	sanitize_dir(LOCKDIR);
	sanitize_dir(WORKDIR);

	initscr();

	if ((LINES < 24) || (COLS < 80)) {
		endwin();
		die("This program requires a screen size of at least 80 columns by 24 lines\n" "Please resize your window");
	}

	mark_first_instance();

	signal(SIGTSTP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);

	start_color();
	standardcolors(options.color);
	noecho();
	nonl();
	cbreak();

#ifndef DEBUG
	curs_set(0);
#endif

	/*
	 * Set logfilename variable to NULL if -L was specified without an
	 * appropriate facility on the command line.
	 */

	if (command == 0)
		strcpy(current_logfile, "");

	/*
	 * If by this time the logfile is still acceptable, obtain the
	 * logspan from the command line if so specified.
	 */

	if (current_logfile[0] != '\0') {
		options.logging = 1;
		if (current_log_interval != 0) {
			options.logspan = current_log_interval;
		}
	}

	struct filterstate ofilter;
	struct ffnode *fltfiles;
	struct porttab *ports;

	loadaddports(&ports);
	/*
	 * Load saved filter or graphing filter if specified
	 */
	if (graphing_logfile[0] != '\0') {
		loadfilterlist(&fltfiles);
		memset(&ofilter, 0, sizeof(struct filterstate));
		loadfilter(pickfilterbyname(fltfiles, graphing_filter),
			   &(ofilter.fl), FLT_RESOLVE);
	} else {
		loadfilters(&ofilter);
		indicate("");
	}

	/* bad, bad, bad name draw_desktop()
	 * hide all into tui_top_panel(char *msg)
	 * */
	draw_desktop();
	attrset(STATUSBARATTR);
	mvprintw(0, 1, "%s %s", IPTRAF_NAME, IPTRAF_VERSION);

	/* simplify */
	if (g_opt)
		ifstats(&options, &ofilter, facilitytime);
	else if (i_opt)
		if (strcmp(i_opt, "all") == 0)
			ipmon(&options, &ofilter, facilitytime, NULL);
		else
			ipmon(&options, &ofilter, facilitytime, i_opt);
	else if (l_opt)
		if (strcmp(l_opt, "all") == 0)
			hostmon(&options, facilitytime, NULL, &ofilter);
		else
			hostmon(&options, facilitytime, l_opt, &ofilter);
	else if (d_opt)
		detstats(d_opt, &options, facilitytime, &ofilter);
	else if (s_opt)
		servmon(s_opt, ports, &options, facilitytime, &ofilter);
	else if (z_opt)
		packet_size_breakdown(&options, z_opt, facilitytime, &ofilter);
	else
		program_interface(&options);

	endwin();

	if (is_first_instance)
		unlink(IPTIDFILE);

	return (0);
}
