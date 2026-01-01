/*
 * smcroute.c
 *
 * Copyright (C) 2025 EGC
 *
 *Github: https://github.com/troglobit/smcroute 
 *
 * man page: https://man.troglobit.com/man8/smcrouted.8.html
 * man page : https://man.freebsd.org/cgi/man.cgi?query=smcroute&sektion=8&manpath=FreeBSD+13.2-RELEASE+and+Ports/1000
 *
 * Setup: https://an0n-r0.medium.com/making-dlna-through-site-to-site-vpn-work-f393629f4ce0
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

#ifdef HAVE_SMCROUTE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <ddnvram.h>
#include <shutils.h>
#include <services.h>

void stop_smcrouted(void);

void start_smcrouted(void)
{
	char path[64];
	char conffile[64];
	if (!nvram_matchi("smcrouted_enable", 1)) {
		stop_smcrouted();
		return;
	}

	/*
	char *loglevel = nvram_safe_get("smcrouted_loglevel");
	if (!loglevel || loglevel[0] == '\0') {
		loglevel = "info";
	}
	*/

	FILE *fp = fopencreate("/tmp/smcroute/smcroute.conf", "w+");
	if (fp == NULL)
		return;
	if (nvram_invmatch("smcrouted_conf", ""))
		fprintf(fp, nvram_get("smcrouted_conf"));

	if (reload_process("smcrouted")) {
		snprintf(conffile, sizeof(conffile), getdefaultconfig(NULL, path, sizeof(path), "smcroute/smcroute.conf"));
		//log_eval("smcrouted", "-s", "-f", conffile, "-P", "smcrouted", "-N", "-l", loglevel);   //default log level is notice > info > debug
		//log_eval("smcrouted", "-s", "-f", conffile, "-P", "smcrouted", "-N", "-l", !strcmp(nvram_safe_get("smcrouted_loglevel"), "") ? "info" : nvram_safe_get("smcrouted_loglevel"));   //default log level is notice > info > debug
		log_eval("smcrouted", "-s", "-f", conffile, "-P", "smcrouted", "-N", "-c",
			 nvram_invmatch("smcrouted_flushrtsec", "") ? nvram_safe_get("smcrouted_flushrtsec") : "0", "-l",
			 nvram_invmatch("smcrouted_loglevel", "") ? nvram_safe_get("smcrouted_loglevel") :
								    "info"); //default log level is notice > info > debug
	}
	//dd_loginfo("done\n");
	return;
}

void stop_smcrouted(void)
{
	stop_process("smcrouted", "smcroute daemon");
	unlink("/var/run/smcrouted.pid"); // remove pid file if it exists, otherwise smcroute wont restart if killed with SIGKILL
	//log_eval("smcroute", "-k");  //note this is the /usr/bin/smcroute script, not the daemon
	//log_eval("smcroutectl", "kill");  //note this is the /usr/bin/smcroutectl stub
	//unlink("/tmp/smcroute.conf");
	//dd_loginfo("done\n");
	return;
}

void restart_smcrouted(void)
{
	stop_smcrouted();
	start_smcrouted();
	return;
}

#endif
