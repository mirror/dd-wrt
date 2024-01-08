/*
 * stabridge.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <utils.h>
#include <services.h>

void start_stabridge(void)
{
	char tmp[256];
#ifdef HAVE_RELAYD
	if (getWET()) {
		char label[32], debug[32], debug_string[32];
		sprintf(label, "%s_relayd_gw_auto", getWET());
		sprintf(debug, "%s_relayd_debug", getWET());
		if (nvram_matchi(debug, 1)) {
			//sprintf(debug_string, " -dd >/tmp/%s_relayd.log 2>&1", getWET());
			sprintf(debug_string, " -dd 2>&1 |/usr/bin/logger");
		} else {
			bzero(debug_string, sizeof(debug_string));
		}
		if (nvram_matchi(label, 0)) {
			sprintf(label, "%s_relayd_gw_ipaddr", getWET());
			sysprintf("relayd -I %s -I %s -G %s -L %s -D -B%s &",
				  getBridge(getWET(), tmp), getWET(),
				  nvram_safe_get(label),
				  nvram_safe_get("lan_ipaddr"), debug_string);
		} else {
			sysprintf("relayd -I %s -I %s -L %s -D -B%s &",
				  getBridge(getWET(), tmp), getWET(),
				  nvram_safe_get("lan_ipaddr"), debug_string);
		}
	}
#else
	if (getWET()) {
		// let packages pass to iptables without ebtables loaded

		writeprocsysnet("bridge/bridge-nf-call-arptables", "1");
		writeprocsysnet("bridge/bridge-nf-call-ip6tables", "1");
		writeprocsysnet("bridge/bridge-nf-call-iptables", "1");
		insmod("ebtables ebtables ebtable_filter ebtable_nat ebtable_broute ebt_arpnat ebt_broute");
		eval("ebtables", "-t", "nat", "-A", "PREROUTING",
		     "--in-interface", getWET(), "-j", "arpnat",
		     "--arpnat-target", "ACCEPT");
		eval("ebtables", "-t", "nat", "-A", "POSTROUTING",
		     "--out-interface", getWET(), "-j", "arpnat",
		     "--arpnat-target", "ACCEPT");
		eval("ebtables", "-t", "broute", "-A", "BROUTING", "--protocol",
		     "0x888e", "--in-interface", getWET(), "-j", "DROP");
	}
#endif
}

void stop_stabridge(void)
{
#ifdef HAVE_RELAYD
	stop_process("relayd", "Client Bridge Relay Daemon");
#else
	if (getWET()) {
		eval("ebtables", "-t", "broute", "-D", "BROUTING", "--protocol",
		     "0x888e", "--in-interface", getWET(), "-j", "DROP");
		eval("ebtables", "-t", "nat", "-D", "POSTROUTING",
		     "--out-interface", getWET(), "-j", "arpnat",
		     "--arpnat-target", "ACCEPT");
		eval("ebtables", "-t", "nat", "-D", "PREROUTING",
		     "--in-interface", getWET(), "-j", "arpnat",
		     "--arpnat-target", "ACCEPT");
	}
	// flush the tables, since getWET will not find the interface
	// in the nvram (if changed from client-bridge to whatever)
	// Fix, cause the rmmod command does not
	// remove the modules (..if rules are in?).
	eval("ebtables", "-t", "broute", "-F");
	eval("ebtables", "-t", "nat", "-F");
	rmmod("ebt_broute ebt_arpnat ebtable_broute ebtable_nat ebtable_filter ebtables");
	// don't let packages pass to iptables without ebtables loaded
	writeprocsysnet("bridge/bridge-nf-call-arptables", "0");
	writeprocsysnet("bridge/bridge-nf-call-ip6tables", "0");
	writeprocsysnet("bridge/bridge-nf-call-iptables", "0");
#endif
}
