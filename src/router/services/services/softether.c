/*
 * softether.c
 *
 * Copyright (C) 2016 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_SOFTETHER
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

static void stop_softetherclient(void);
static void stop_softetherbridge(void);
static void stop_softetherserver(void);

void start_softether(void)
{
	if (!nvram_matchi("setherclient_enable", 1)) {
		stop_softetherclient();
	}

	if (!nvram_matchi("setherserver_enable", 1)) {
		stop_softetherserver();
	}

	if (!nvram_matchi("setherbridge_enable", 1)) {
		stop_softetherbridge();
	}

	if (nvram_matchi("setherclient_enable", 1)) {
		write_nvram("/tmp/vpn_client.config", "sether_config");
		eval("ln", "-s", "/usr/bin/hamcore.se2", "/tmp/");
		eval("ln", "-s", "/usr/bin/vpnclient", "/tmp/");
		eval("/tmp/vpnclient", "start");
		eval("sleep", "15");
		eval("/usr/bin/vpncmd", "localhost", "/CLIENT", "/CMD", "ConfigSet", "//tmp//vpn_server.config", "quit");
	}
	if (nvram_matchi("setherserver_enable", 1)) {
		write_nvram("/tmp/vpn_server.config", "sether_config");
		eval("ln", "-s", "/usr/bin/hamcore.se2", "/tmp/");
		eval("ln", "-s", "/usr/bin/vpnserver", "/tmp/");
		eval("/tmp/vpnserver", "start");
		eval("sleep", "15");
		eval("/usr/bin/vpncmd", "localhost", "/SERVER", "/CMD", "ConfigSet", "//tmp//vpn_server.config", "quit");
	}
	if (nvram_matchi("setherbridge_enable", 1)) {
		write_nvram("/tmp/vpn_bridge.config", "sether_config");
		eval("ln", "-s", "/usr/bin/hamcore.se2", "/tmp/");
		eval("ln", "-s", "/usr/bin/vpnbridge", "/tmp/");
		eval("/tmp/vpnbridge", "start");
	}
	return;
}

static void stop_softetherclient(void)
{
	if (pidof("vpnclient") > 0)
		eval("vpnclient", "stop");
	stop_process("vpnclient", "SoftEther Client");
	return;
}

static void stop_softetherbridge(void)
{
	if (pidof("vpnbridge") > 0)
		eval("vpnbridge", "stop");
	stop_process("vpnbridge", "SoftEther Bridge");
	return;
}

static void stop_softetherserver(void)
{
	if (pidof("vpnserver") > 0)
		eval("vpnserver", "stop");
	stop_process("vpnserver", "SoftEther Server");
	return;
}

void stop_softether(void)
{
	stop_softetherserver();
	stop_softetherbridge();
	stop_softetherclient();

}

#endif
