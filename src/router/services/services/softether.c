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
#define VPNDIR "/var/softethervpn"
#define EXECDIR "/usr/libexec/softethervpn"

void start_softether(void)
{
	int reload = 0;
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
		mkdir(VPNDIR, 0700);
		write_nvram("/tmp/vpn_client.config", "sether_config");
		eval("ln", "-sf", EXECDIR "/hamcore.se2", VPNDIR "/");
		eval("ln", "-sf", EXECDIR "/vpnclient", VPNDIR "/");
		eval("ln", "-sf", EXECDIR "/lang.config", VPNDIR "/");
		eval(VPNDIR "/vpnclient", "start");
		reload |= 1;
	}
	if (nvram_matchi("setherserver_enable", 1)) {
		mkdir(VPNDIR, 0700);
		write_nvram("/tmp/vpn_server.config", "sether_config");
		eval("ln", "-sf", EXECDIR "/hamcore.se2", VPNDIR "/");
		eval("ln", "-sf", EXECDIR "/vpnserver", VPNDIR "/");
		eval("ln", "-sf", EXECDIR "/lang.config", VPNDIR "/");
		eval(VPNDIR "/vpnserver", "start");
		reload |= 2;
	}
	if (nvram_matchi("setherbridge_enable", 1)) {
		mkdir(VPNDIR, 0700);
		write_nvram("/tmp/vpn_bridge.config", "sether_config");
		eval("ln", "-sf", EXECDIR "/hamcore.se2", VPNDIR "/");
		eval("ln", "-sf", EXECDIR "/vpnbridge", VPNDIR "/");
		eval("ln", "-sf", EXECDIR "/lang.config", VPNDIR "/");
		eval(VPNDIR "/vpnbridge", "start");
		reload |= 4;
	}
	if (reload) {
		eval("sleep", "3");
		if (reload & 1)
			eval("/usr/bin/vpncmd", "localhost", "/SERVER", "/CMD", "ConfigSet", "//tmp//vpn_client.config", "quit");
		if (reload & 2)
			eval("/usr/bin/vpncmd", "localhost", "/SERVER", "/CMD", "ConfigSet", "//tmp//vpn_server.config", "quit");
		if (reload & 4)
			eval("/usr/bin/vpncmd", "localhost", "/SERVER", "/CMD", "ConfigSet", "//tmp//vpn_bridge.config", "quit");

	}
	return;
}

static void stop_softetherclient(void)
{
	if (pidof("vpnclient") > 0)
		eval(VPNDIR "/vpnclient", "stop");
	stop_process("vpnclient", "SoftEther Client");
	return;
}

static void stop_softetherbridge(void)
{
	if (pidof("vpnbridge") > 0)
		eval(VPNDIR "/vpnbridge", "stop");
	stop_process("vpnbridge", "SoftEther Bridge");
	return;
}

static void stop_softetherserver(void)
{
	if (pidof("vpnserver") > 0)
		eval(VPNDIR "/vpnserver", "stop");
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
