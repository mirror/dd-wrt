/*
 * milkfish.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#ifdef HAVE_MILKFISH
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <md5.h>

void start_milkfish_boot(void);

void stop_milkfish(void)
{
	if (pidof("rtpproxy") > 0 || pidof("openser") > 0) {
		dd_syslog(LOG_INFO, "Milkfish service successfully stopped\n");

		killall("rtpproxy", SIGTERM);
		killall("openser", SIGTERM);
		// Stop the milkfish services
		eval("milkfish_services", "stop");
	}
}

void start_milkfish(void)
{
	if (nvram_match("milkfish_enabled", "1")) {
		start_milkfish_boot();
		eval("/etc/config/milkfish.netup");	// start rtpproxy and
		// openserctl

		dd_syslog(LOG_INFO, "Milkfish service successfully started\n");
	}
}

void start_milkfish_boot(void)
{
	MD5_CTX MD;

	if (strlen(nvram_safe_get("milkfish_routerid")) != 32) {
		unsigned char hash[32];
		char *et0 = nvram_safe_get("et0macaddr");

		MD5Init(&MD);
		MD5Update(&MD, et0, 17);
		MD5Final((unsigned char *)hash, &MD);
		char request[32];
		int i;

		for (i = 0; i < 16; i++)
			sprintf(&request[2 * i], "%02x", hash[i]);
		nvram_set("milkfish_routerid", request);
		nvram_set("need_commit", "1");
	}
	// Start the milkfish services
	eval("milkfish_services", "start");
	// dbtext module setup
	eval("mkdir", "-p", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/aliases", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/location", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/subscriber", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/version", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/uri", "/var/openser/dbtext/");
	eval("cp", "/etc/openser/dbtext/grp", "/var/openser/dbtext/");

	// restore dbtext parts which may have been saved into nvram
	eval("milkfish_services", "sipdb", "restorenvdd");

	// firewall configuration in networking/firewall.c
	return;
}
#endif
