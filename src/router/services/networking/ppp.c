/*
 * ppp.c
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

#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include "ledcontrol.h"

#include <code_pattern.h>
#include <cy_conf.h>
#include <utils.h>
#include <services.h>

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

char *getenvs(char *env)
{
	static unsigned char r[64];
	char *e = getenv(env);
	if (!e)
		return NULL;
	int c = 0;
	int i;
	for (i = 0; i < strlen(e); i++) {
		if (e[i] != ' ')
			r[c++] = e[i];
	}
	r[c++] = 0;		//terminate string
	return r;
}

/*
 * Called when link comes up
 */
int ipup_main(int argc, char **argv)
{
	FILE *fp;
	char *wan_ifname = safe_getenv("IFNAME");

	// char *wan_proto = nvram_safe_get("wan_proto");
	char *value;
	char buf[256];

	cprintf("%s\n", argv[0]);

	stop_process("listen", "activity listener");
	nvram_set("wan_iface", wan_ifname);
	if (check_action() != ACT_IDLE)
		return -1;

	/*
	 * ipup will receive bellow six arguments 
	 */
	/*
	 * interface-name tty-device speed local-IP-address remote-IP-address
	 * ipparam 
	 */

	/*
	 * Touch connection file 
	 */
	if (!(fp = fopen("/tmp/ppp/link", "a"))) {
		perror("/tmp/ppp/link");
		return errno;
	}
	fprintf(fp, "%s", argv[1]);
	fclose(fp);

	nvram_set("pppd_pppifname", wan_ifname);

	if (nvram_match("wan_proto", "pppoe")
#ifdef HAVE_PPPOEDUAL
	    || nvram_match("wan_proto", "pppoe_dual")
#endif
	   )
		nvram_set("pppoe_ifname", wan_ifname);

	if (getenv("IPLOCAL")) {
		value = getenvs("IPLOCAL");
		ifconfig(wan_ifname, IFUP, value, "255.255.255.255");
		if (nvram_match("wan_proto", "pppoe") 
#ifdef HAVE_PPPOEDUAL
		    || nvram_match("wan_proto", "pppoe_dual")
#endif
		    ) {
			nvram_set("wan_ipaddr_buf", nvram_safe_get("wan_ipaddr"));	// Store 
			nvram_set("wan_ipaddr", value);
			nvram_set("wan_netmask", "255.255.255.255");
#ifdef HAVE_PPPOATM
		} else if (nvram_match("wan_proto", "pppoa")) {
			nvram_set("wan_ipaddr_buf", nvram_safe_get("wan_ipaddr"));	// Store 
			nvram_set("wan_ipaddr", value);
			nvram_set("wan_netmask", "255.255.255.255");
#endif
#ifdef HAVE_PPTP
		} else if (nvram_match("wan_proto", "pptp")) {
			nvram_set("wan_ipaddr_buf", nvram_safe_get("wan_ipaddr"));	// Store 
			nvram_set("pptp_get_ip", value);
#endif
#ifdef HAVE_L2TP
		} else if (nvram_match("wan_proto", "l2tp")) {
			nvram_set("wan_ipaddr_buf", nvram_safe_get("wan_ipaddr"));	// Store 
			nvram_set("l2tp_get_ip", value);
#endif
		}
#ifdef HAVE_3G
		else if (nvram_match("wan_proto", "3g")) {
			nvram_set("wan_ipaddr_buf", nvram_safe_get("wan_ipaddr"));	// Store 
			nvram_set("wan_ipaddr", value);
			nvram_set("wan_netmask", "255.255.255.255");
			// reset 3gnetmodetoggle for mode 5
			nvram_unset("3gnetmodetoggle");
#if defined(HAVE_TMK) || defined(HAVE_BKM)
			char *gpio3g = nvram_get("gpio3g");
			if (gpio3g != NULL)
				set_gpio(atoi(gpio3g), 1);
#endif
		}
#endif
	}

	if (getenv("IPREMOTE")) {
		value = getenvs("IPREMOTE");

		if (nvram_match("wan_proto", "pptp")) {
			nvram_set("wan_gateway", value);
			eval("route", "del", "default");
			route_add(wan_ifname, 0, "0.0.0.0", value, "0.0.0.0");
		} else {
			nvram_set("wan_gateway", value);

		}
	}
	strcpy(buf, "");
	if (getenv("DNS1"))
		sprintf(buf, "%s", getenvs("DNS1"));
	if (getenv("DNS2"))
		sprintf(buf + strlen(buf), "%s%s", strlen(buf) ? " " : "", getenvs("DNS2"));
	nvram_set("wan_get_dns", buf);

	if ((value = getenv("AC_NAME")))
		nvram_set("ppp_get_ac", value);
	if ((value = getenv("SRV_NAME")))
		nvram_set("ppp_get_srv", value);
	if ((value = getenv("MTU")))
		nvram_set("wan_run_mtu", value);
	start_wan_done(wan_ifname);
	cprintf("done\n");
	return 0;
}

/*
 * Called when link goes down
 */
int ipdown_main(int argc, char **argv)
{
	if (check_action() != ACT_IDLE)
		return -1;
	runStartup("/etc/config", ".ipdown");
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
	{
#ifdef HAVE_RB500
		runStartup("/usr/local/etc/config", ".ipdown");
#else
		runStartup("/jffs/etc/config", ".ipdown");
		runStartup("/mmc/etc/config", ".ipdown");
		runStartup("/tmp/etc/config", ".ipdown");
#endif
	}
	stop_ddns();
	stop_ntpc();

	unlink("/tmp/ppp/link");

	if (nvram_match("wan_proto", "l2tp")) {
		/*
		 * clear dns from the resolv.conf 
		 */
		nvram_set("wan_get_dns", "");
		dns_to_resolv();

		// todo
		route_del(nvram_safe_get("wan_ifname"), 0, nvram_safe_get("l2tp_server_ip"), nvram_safe_get("wan_gateway_buf"), "255.255.255.255");
		/*
		 * Restore the default gateway for WAN interface 
		 */
		nvram_set("wan_gateway", nvram_safe_get("wan_gateway_buf"));

		/*
		 * Set default route to gateway if specified 
		 */
		route_add(nvram_safe_get("wan_ifname"), 0, "0.0.0.0", nvram_safe_get("wan_gateway"), "0.0.0.0");
	}
	if (nvram_match("wan_proto", "pptp")) {
		eval("route", "del", "default");
		nvram_set("wan_gateway", nvram_safe_get("wan_gateway_buf"));
		eval("route", "add", "default", "gw", nvram_safe_get("wan_gateway"));
		sysprintf("iptables -t nat -A POSTROUTING -o %s -j MASQUERADE\n", nvram_safe_get("pptp_ifname"));
	}
#ifdef HAVE_3G
#if defined(HAVE_TMK) || defined(HAVE_BKM)
	else if (nvram_match("wan_proto", "3g")) {
		char *gpio3g = nvram_get("gpio3g");
		if (gpio3g != NULL)
			set_gpio(atoi(gpio3g), 0);
	}
#endif
#endif

	nvram_set("pppoe_ifname", "");
	nvram_set("pppd_pppifname", "");

	// write PPP traffic statistics to nvram if wanted
	if (nvram_match("ppp_traffic", "1")) {
		char buffer[64];
		long long old_in, old_out;
		long long in, out;
		char *pin;
		char *pout;
		time_t stamp;

		old_in = atol(nvram_safe_get("ppp_byte_in"));
		old_out = atol(nvram_safe_get("ppp_byte_out"));

		if ((pin = getenv("BYTES_RCVD")))
			in = atol(pin);
		else
			in = 0;

		if ((pout = getenv("BYTES_SENT")))
			out = atol(pout);
		else
			out = 0;

		in += old_in;
		out += old_out;
		snprintf(buffer, 63, "%lld", in);
		nvram_set("ppp_byte_in", buffer);
		snprintf(buffer, 63, "%lld", out);
		nvram_set("ppp_byte_out", buffer);
		if ((stamp = time(NULL)) < 1087818160)	// clock is not set
			// properly
			stamp = 0;
		snprintf(buffer, 63, "%ld", stamp);
		nvram_set("ppp_byte_stamp", buffer);
		nvram_commit();
	}

	if (nvram_match("ppp_demand", "1")
	    && (nvram_match("wan_proto", "pptp")
		|| nvram_match("wan_proto", "l2tp")
		|| nvram_match("wan_proto", "pppoe"))) {
		stop_process("listen", "activity listener");
		eval("listen", nvram_safe_get("lan_ifname"));
	}

	return 1;
}

/*
 * int pppevent_main (int argc, char **argv) { int argn; char *type = NULL;
 * 
 * argn = 1; while (argn < argc && argv[argn][0] == '-') { if (strcmp
 * (argv[argn], "-t") == 0) { ++argn; type = argv[argn]; } ++argn; }
 * 
 * if (!type) return 1;
 * 
 * 
 * if (!strcmp (type, "PAP_AUTH_FAIL") || !strcmp (type, "CHAP_AUTH_FAIL")) {
 * 
 * buf_to_file ("/tmp/ppp/log", type);
 * 
 * if (check_hw_type () == BCM4704_BCM5325F_CHIP) SET_LED (GET_IP_ERROR); }
 * 
 * return 0; }
 * 
 * //=============================================================================
 * int set_pppoepid_to_nv_main (int argc, char **argv) // tallest 1219 { if
 * (!strcmp (argv[1], "0")) { nvram_set ("pppoe_pid0", getenv ("PPPD_PID"));
 * nvram_set ("pppoe_ifname0", getenv ("IFNAME")); } else if (!strcmp (argv[1],
 * "1")) { nvram_set ("pppoe_pid1", getenv ("PPPD_PID")); nvram_set
 * ("pppoe_ifname1", getenv ("IFNAME")); }
 * 
 * dprintf ("done.( IFNAME = %s DEVICE = %s )\n", getenv ("IFNAME"), getenv
 * ("DEVICE")); return 0; }
 */

// by tallest 0407
int disconnected_pppoe_main(int argc, char **argv)
{
	int pppoe_num = atoi(argv[1]);
	char ppp_demand[2][20] = { "ppp_demand", "ppp_demand_1" };

	if (nvram_match(ppp_demand[pppoe_num], "1")
	    && nvram_match("action_service", "")) {
		cprintf("tallest:=====( kill pppoe %d )=====\n", pppoe_num);
		stop_single_pppoe(pppoe_num);
		start_pppoe(pppoe_num);
		dns_to_resolv();

		stop_dnsmasq();
		start_dnsmasq();

		return 0;
	}
	cprintf("tallest:=====( PPPOE Dial On Demand Error!! )=====\n");
	return 0;
}

// =============================================================================
