/*
 * ntp.c
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
#include <rc.h>

#include <cy_conf.h>
#include <utils.h>

#define NR_RULES 20

//#define NTP_M_TIMER 3600
//#define NTP_N_TIMER 30

extern void dd_timer_cancel(timer_t timerid);

struct syncservice {
	char *nvram;
	char *service;
};

static struct syncservice service[] = {
	{ "cron_enable", "cron" },
#ifdef HAVE_SNMP
	{ "snmpd_enable", "snmp" },
#endif
#ifdef HAVE_CHILLI
	{ "chilli_enable", "chilli" },	{ "hotss_enable", "chilli" },
#endif
#ifdef HAVE_WIFIDOG
	{ "wd_enable", "wifidog" },
#endif
#ifdef HAVE_UNBOUND
	{ "recursive_dns", "unbound" },
#endif
#ifdef HAVE_DNSCRYPT
	{ "dns_crypt", "dnsmasq" },
#endif
#ifdef HAVE_TOR
	{ "tor_enable", "tor" },
#endif
};

static void sync_daemons(void)
{
	int i;
	for (i = 0; i < sizeof(service) / sizeof(struct syncservice); i++) {
		if (nvram_matchi(service[i].nvram, 1)) {
			dd_syslog(LOG_DEBUG, "Restarting %s (time sync change)\n", service[i].service);
			eval("service", service[i].service, "restart");
		}
	}
}

// <<tofu
static int do_ntp(void) // called from ntp_main and
	// process_monitor_main; (now really) called every hour!
{
	char *servers;
	struct timeval now;
	struct timeval then;

	nvram_seti("ntp_success", 0);
	if (!nvram_matchi("ntp_enable", 1))
		return 0;
	gettimeofday(&now, NULL);
	update_timezone();
	servers = nvram_safe_get("ntp_server");
	if (*servers == 0) {
		servers = "2.pool.ntp.org 212.18.3.19 88.99.174.22";
	}

	char *argv[] = { "ntpclient", servers, NULL };
	if (_evalpid(argv, NULL, 20, NULL) != 0) {
		dd_syslog(LOG_ERR, "cyclic NTP Update failed (servers %s)\n", servers);
		return 1;
	}
	if (nvram_match("ntp_done", "0"))
		nvram_seti("start_time", time(NULL));
	nvram_set("ntp_done", "1");

#if defined(HAVE_VENTANA) || defined(HAVE_NEWPORT) || defined(HAVE_LAGUNA) || defined(HAVE_STORM) || defined(HAVE_PERU) || \
	(defined(HAVE_GATEWORX) && !defined(HAVE_NOP8670))
	eval("hwclock", "-w", "-u");
#endif
	gettimeofday(&then, NULL);
	dd_syslog(LOG_INFO, "Cyclic NTP Update success (servers %s)\n", servers);
	dd_syslog(LOG_INFO, "Local timer delta is %ld\n", (then.tv_sec - now.tv_sec));
	if ((abs(now.tv_sec - then.tv_sec) > 100000000)) {
		int seq;
		//              for (seq = 1; seq <= NR_RULES; seq++)
		//                      sysprintf("/sbin/filter del %d", seq);  // for time scheduled filtering we need to reinitialize the tables here to prevent wrong timed triggers
		eval("restart", "firewall");
		sync_daemons();
		nvram_seti("ntp_success", 1);
		eval("restart", "process_monitor");
	} else {
		eval("filtersync");
	}

	return 0;
}

static void ntp_main(timer_t t, int arg)
{
	if (check_action() != ACT_IDLE)
		return; // don't execute while upgrading

	eval("stopservice", "ntpc", "-f");
	if (do_ntp() == 0) {
		if (arg == FIRST)
			dd_timer_cancel(t);
	}
}
