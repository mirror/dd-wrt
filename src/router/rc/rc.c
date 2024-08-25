/*
 * rc.c
 *
 * Copyright (C) 2006 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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


#define start_service(a) eval("startservice", a);
#define start_service_force(a) eval("startservice", a, "-f");
#define start_service_f(a) eval("startservice_f", a);
#define start_service_force_f(a) eval("startservice_f", a, "-f");
#define start_services() eval("startservices");
#define stop_service(a) eval("stopservice", a);
#define stop_service_force(a) eval("stopservice", "-f", a);
#define stop_running(a) eval("stop_running");
#define stop_service_f(a) eval("stopservice_f", a);
#define stop_service_force_f(a) eval("stopservice_f", a, "-f");
#define stop_services() eval("stopservices");
#define restart(a) eval("restart", a);
#define restart_f(a) eval("restart_f", a);

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
#include <shutils.h>
#include <wlutils.h>
#include <cy_conf.h>
#include <arpa/inet.h>

#include <revision.h>
#include <airbag.h>
#include "crc.c"
#include "mtd.c"
#include "nvram.c"
#include "mtd_main.c"
#include "ledtool.c"
#include "check_ps.c"
//#include "resetbutton.c"
#include "listen.c"
#ifdef HAVE_WOL
#include "wol.c"
#endif
#ifdef HAVE_MADWIFI
#include "roaming_daemon.c"
#endif
#ifdef HAVE_GPIOWATCHER
#include "gpiowatcher.c"
#endif
#ifdef HAVE_WPS
#include "wpswatcher.c"
#endif
#ifdef HAVE_WIVIZ
#include "autokill_wiviz.c"
#endif
#ifdef HAVE_QTN
#include "qtn_monitor.c"
#endif
#include "event.c"
#include "gratarp.c"

struct MAIN {
	char *callname;
	char *execname;
	int (*exec)(int argc, char **argv);
};
extern char *getSoftwareRevision(void);
static int softwarerevision_main(int argc, char **argv)
{
	fprintf(stdout, "%s\n", getSoftwareRevision());
	return 0;
}

/* 
 * Call when keepalive mode
 */

static int rc_main(int argc, char *argv[])
{
	int ret = 0;
	if (argv[1]) {
		if (strncmp(argv[1], "start", 5) == 0) {
			ret = kill(1, SIGUSR2);
		} else if (strncmp(argv[1], "stop", 4) == 0) {
			ret = kill(1, SIGINT);
		} else if (strncmp(argv[1], "restart", 7) == 0) {
			ret = kill(1, SIGHUP);
		}
	} else {
		fprintf(stderr, "usage: rc [start|stop|restart]\n");
		ret = EINVAL;
	}

	return ret;
}

static int erase_main(int argc, char *argv[])
{
	int ret = 0;
	/* 
	 * erase [device] 
	 */
	if (argv[1]) {
		if (!strcmp(argv[1], "nvram")) {
			int brand = getRouterBrand();
			if (brand == ROUTER_MOTOROLA || brand == ROUTER_MOTOROLA_V1 || brand == ROUTER_MOTOROLA_WE800G ||
			    brand == ROUTER_RT210W || brand == ROUTER_BUFFALO_WZRRSG54) {
				fprintf(stderr, "Sorry, erasing nvram will turn this router into a brick\n");
			} else {
				nvram_clear();
				nvram_commit();
			}
		} else {
			ret = mtd_erase(argv[1]);
		}
	} else {
		fprintf(stderr, "usage: erase [device]\n");
		ret = EINVAL;
	}
	return ret;
}

static int start_main(char *name, int argc, char **argv)
{
	pid_t pid;
	int status;
	int sig;
	char *args[32] = { "/sbin/service", name, "main", NULL };
	int i;
	for (i = 1; i < argc && i < 30; i++)
		args[i + 2] = argv[i];
	args[2 + i] = NULL;
	switch (pid = fork()) {
	case -1: /* error */
		perror("fork");
		return errno;
	case 0: /* child */
		for (sig = 0; sig < (_NSIG - 1); sig++)
			signal(sig, SIG_DFL);
		execvp(args[0], args);
		perror(argv[0]);
		exit(errno);
	default: /* parent */
		waitpid(pid, &status, 0);
		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		else
			return status;
	}
	return errno;
}

static void start_main_f(char *name, int argc, char **argv)
{
	FORK(start_main(name, argc, argv));
}

static struct MAIN maincalls[] = {
	// {"init", NULL, &main_loop},
	{ "ip-up", "ipup", NULL },
	{ "ip-down", "ipdown", NULL },
	{ "ipdown", "disconnected_pppoe", NULL },
	{ "dhcpc_tv", "dhcpc_tv", NULL },
	{ "dhcpc", "dhcpc", NULL },
	{ "mtd", NULL, mtd_main },
	{ "nvram", NULL, nvram_main },
	{ "filtersync", "filtersync", NULL },
	{ "filter", "filter", NULL },
	{ "setpasswd", "setpasswd", NULL },
	{ "ipfmt", "ipfmt", NULL },
	{ "restart_dns", "restart_dns", NULL },
	{ "ledtool", NULL, ledtool_main },
	{ "check_ps", NULL, check_ps_main },
	{ "listen", NULL, listen_main },
#ifdef HAVE_MADWIFI
	{ "roaming_daemon", NULL, roaming_daemon_main },
#endif
#ifdef HAVE_GPIOWATCHER
	{ "gpiowatcher", NULL, gpiowatcher_main },
#endif
#ifdef HAVE_WPS
	{ "wpswatcher", NULL, wpswatcher_main },
#endif
#ifdef HAVE_PPTPD
	{ "poptop", "pptpd_main", NULL },
#endif
//	{ "redial", NULL, redial_main },
#ifndef HAVE_RB500
// {"resetbutton", NULL, &resetbutton_main},
#endif
	// {"wland", NULL, &wland_main},
	{ "hb_connect", "hb_connect", NULL },
	{ "hb_disconnect", "hb_disconnect", NULL },
	{ "gpio", "gpio", NULL },
	{ "beep", "beep", NULL },
	{ "ledtracking", "ledtracking", NULL },
	// {"listen", NULL, &listen_main},
	// {"check_ps", NULL, &check_ps_main},
	{ "ddns_success", "ddns_success", NULL },
// {"process_monitor", NULL, &process_monitor_main},
// {"radio_timer", NULL, &radio_timer_main},
// {"ttraf", NULL, &ttraff_main},
#ifdef HAVE_WIVIZ
	{ "autokill_wiviz", NULL, &autokill_wiviz_main },
#endif
	{ "site_survey", "site_survey", NULL },
#ifdef HAVE_WOL
	{ "wol", NULL, &wol_main },
#endif
	{ "event", NULL, &event_main },
//    {"switch", "switch", NULL},
#ifdef HAVE_MICRO
	{ "brctl", "brctl", NULL },
#endif
	{ "setportprio", "setportprio", NULL },
#ifdef HAVE_NORTHSTAR
	{ "rtkswitch", "rtkswitch", NULL },
#endif
	{ "setuserpasswd", "setuserpasswd", NULL },
	{ "getbridge", "getbridge", NULL },
	{ "getmask", "getmask", NULL },
	{ "getipmask", "getipmask", NULL },
//	{ "stopservices", NULL, stop_services_main },
#ifdef HAVE_PPPOESERVER
	{ "addpppoeconnected", "addpppoeconnected", NULL },
	{ "delpppoeconnected", "delpppoeconnected", NULL },
	{ "addpppoetime", "addpppoetime", NULL },
#endif
	//	{ "startservices", NULL, start_services_main },
	//	{ "start_single_service", NULL, start_single_service_main },
	//	{ "restart_f", NULL, restart_main_f },
	//	{ "restart", NULL, restart_main },
	//	{ "stop_running", NULL, stop_running_main },
	{ "softwarerevision", NULL, softwarerevision_main },
// {"nvram", NULL, &nvram_main},
#ifdef HAVE_ROAMING
	//      {"roaming_daemon", NULL, &roaming_daemon_main},
	{ "supplicant", "supplicant", NULL },
#endif
	{ "get_wanface", "get_wanface", NULL },
	{ "get_wanip", "get_wanip", NULL },
#ifndef HAVE_XSCALE
// {"ledtool", NULL, &ledtool_main},
#endif
#ifdef HAVE_REGISTER
	{ "regshell", NULL, &reg_main },
#endif
	{ "gratarp", NULL, &gratarp_main },
	{ "get_nfmark", "get_nfmark", NULL },
#ifdef HAVE_IPV6
	{
		"dhcp6c-state",
		"dhcp6c_state",
	},
#endif
#ifdef HAVE_QTN
	{ "qtn_monitor", NULL, &qtn_monitor_main },
#endif
	{ "write", NULL, &write_main },
	//	{ "startservice_f", NULL, &service_main },
	//	{ "startservice", NULL, &service_main },
	//	{ "stopservice_f", NULL, &service_main },
	//	{ "stopservice", NULL, &service_main },
	{ "rc", NULL, &rc_main },
	{ "erase", NULL, &erase_main },
};

int main(int argc, char **argv)
{
	airbag_init();
	char *base = strrchr(argv[0], '/');
	base = base ? base + 1 : argv[0];
	int i;
	for (i = 0; i < sizeof(maincalls) / sizeof(struct MAIN); i++) {
		if (strstr(base, maincalls[i].callname)) {
			if (maincalls[i].execname)
				return start_main(maincalls[i].execname, argc, argv);
			if (maincalls[i].exec)
				return maincalls[i].exec(argc, argv);
		}
	}
	airbag_deinit();
	return 1; // no command found
}
