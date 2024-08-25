/*
 * l2tp.c
 *
 * Copyright (C) 2009 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h> /* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <net/route.h> /* AhMan March 18 2005 */
#include <sys/types.h>
#include <signal.h>

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>
#include <nvparse.h>
#include <syslog.h>
#include <services.h>

#ifdef HAVE_L2TP
void run_l2tp(int status)
{
	FILE *fp;
	char *l2tp_argv[] = { "xl2tpd", NULL };
	char username[80], passwd[80];
	char *wan_ifname = nvram_safe_get("wan_ifname");

	if (isClient()) {
		wan_ifname = getSTA();
	}
	// stop_dhcpc();
#ifdef HAVE_PPPOE
	stop_pppoe();
#endif
#ifdef HAVE_PPTP
	stop_pptp();
#endif
	stop_l2tp();

	insmod("n_hdlc");
	if (nvram_matchi("l2tp_use_dhcp", 0)) {
		ifconfig(wan_ifname, IFUP, nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
		struct dns_lists *dns_list = NULL;
		dns_to_resolv();
		dns_list = get_dns_list(0);
		int i = 0;

		if (dns_list) {
			for (i = 0; i < dns_list->num_servers; i++)
				route_add(wan_ifname, 0, dns_list->dns_server[i].ip, nvram_safe_get("l2tp_wan_gateway"),
					  "255.255.255.255");
		}
		route_add(wan_ifname, 0, "0.0.0.0", nvram_safe_get("l2tp_wan_gateway"), "0.0.0.0");
		char pptpip[64];
		getIPFromName(nvram_safe_get("l2tp_server_name"), pptpip, sizeof(pptpip));
		route_del(wan_ifname, 0, "0.0.0.0", nvram_safe_get("l2tp_wan_gateway"), "0.0.0.0");
		if (dns_list) {
			for (i = 0; i < dns_list->num_servers; i++)
				route_del(wan_ifname, 0, dns_list->dns_server[i].ip, nvram_safe_get("l2tp_wan_gateway"),
					  "255.255.255.255");
			free_dns_list(dns_list);
		}

		nvram_set("l2tp_server_ip", pptpip);
		if (!nvram_match("l2tp_wan_gateway", "0.0.0.0"))
			route_add(wan_ifname, 0, nvram_safe_get("l2tp_server_ip"), nvram_safe_get("l2tp_wan_gateway"),
				  "255.255.255.255");
	}

	snprintf(username, sizeof(username), "%s", nvram_safe_get("ppp_username"));
	snprintf(passwd, sizeof(passwd), "%s", nvram_safe_get("ppp_passwd"));

	if (status != REDIAL) {
		start_pppmodules();
		insmod("ipv6 udp_tunnel ip6_udp_tunnel l2tp_core l2tp_netlink l2tp_ppp");
		mkdir("/tmp/ppp", 0777);
		mkdir("/var/run/xl2tpd", 0777);
		mkdir("/tmp/xl2tpd", 0777);
		symlink("/sbin/rc", "/tmp/ppp/ip-up");
		symlink("/sbin/rc", "/tmp/ppp/ip-down");
		symlink("/dev/null", "/tmp/ppp/connect-errors");

		/*
		 * Generate L2TP configuration file 
		 */
		if (!(fp = fopen("/tmp/xl2tpd/xl2tpd.conf", "w"))) {
			perror("/tmp/xl2tpd/xl2tpd.conf");
			return;
		}

		fprintf(fp,
			"[global]\n" // Global section
			"port = 1701\n" // Bind address
			"debug avp = no\n" // TEMP DEBUG
			"debug network = no\n" // TEMP DEBUG
			"debug packet = no\n" // TEMP DEBUG
			"debug state = no\n" // TEMP DEBUG
			"debug tunnel = no\n" // TEMP DEBUG
			"[lac %s]\n" //
			"lns = %s\n" //
			"require chap = %s\n" //
			"refuse pap = %s\n" //
			"redial = yes\n" //
			"redial timeout = 15\n" //
			"require authentication = %s\n" //
			"name = %s\n" //
			"pppoptfile = /tmp/ppp/options.l2tp\n" //
			"length bit = yes\n",
			nvram_safe_get("l2tp_server_name"), nvram_safe_get("l2tp_server_ip"),
			nvram_matchi("l2tp_req_chap", 0) ? "no" : "yes", nvram_matchi("l2tp_ref_pap", 0) ? "no" : "yes",
			nvram_matchi("l2tp_req_auth", 0) ? "no" : "yes", username);
		fclose(fp);

		/*
		 * Generate options file 
		 */
		if (!(fp = fopen("/tmp/ppp/options.l2tp", "w"))) {
			perror("/tmp/ppp/options.l2tp");
			return;
		}

		if (nvram_matchi("mtu_enable", 1)) {
			int wan_mtu = nvram_geti("wan_mtu");
			if (wan_mtu > 0) {
				fprintf(fp,
					"mtu %d\n" //
					"mru %d\n",
					wan_mtu, wan_mtu);
			}
		}

		fprintf(fp,
			"defaultroute\n" // Add a default route to the
			"usepeerdns\n" // Ask the peer for up to 2 DNS
			"user '%s'\n",
			username);

		if (nvram_matchi("ppp_demand", 1)) { // demand mode
			fprintf(fp,
				"idle %d\n" //
				"ipcp-accept-remote\n" //
				"ipcp-accept-local\n" //
				"noipdefault\n" //
				"connect true\n" //
				"ktune\n",
				nvram_matchi("ppp_demand", 1) ? nvram_geti("ppp_idletime") * 60 : 0);
			// to 1 in demand mode if the local
			// address changes
		} else { // keepalive mode
			start_redial();
		}

		fprintf(fp, "default-asyncmap\n" // Disable asyncmap
			    "crtscts\n" // Disable protocol field compression
			    "nopcomp\n" // Disable protocol field compression
			    "refuse-eap\n" // Disable protocol field compression
			    "noaccomp\n"); // Disable Address/Control
		if (nvram_matchi("l2tp_encrypt", 0)) {
			fprintf(fp,
				"nomppe\n" // Disable mppe negotiation
				"noccp\n"); // Disable CCP (Compression Control
			// Protocol)
		} else {
			fprintf(fp, "mppe required,stateless\n"
				    "require-mschap-v2\n");
		}
		fprintf(fp,
			"novj\n" // Disable Van Jacobson style TCP/IP
			"nobsdcomp\n" // Disables BSD-Compress compression
			"nodeflate\n" // Disables Deflate compression
			"lcp-echo-failure 12\n" //
			"lcp-echo-interval 30\n" // echo-request frame to the peer
			"lcp-echo-adaptive\n"
			"lock\n" //
			"noauth\n");
#ifdef HAVE_IPV6
		if (nvram_matchi("ipv6_enable", 1)) {
			fprintf(fp, "+ipv6\n");
		}
#endif
		fclose(fp);

		/*
		 * Generate pap-secrets file 
		 */
		if (!(fp = fopen("/tmp/ppp/pap-secrets", "w"))) {
			perror("/tmp/ppp/pap-secrets");
			return;
		}
		fprintf(fp, "\"%s\" * \"%s\" *\n", username, passwd);
		fclose(fp);
		chmod("/tmp/ppp/pap-secrets", 0600);

		/*
		 * Generate chap-secrets file 
		 */
		if (!(fp = fopen("/tmp/ppp/chap-secrets", "w"))) {
			perror("/tmp/ppp/chap-secrets");
			return;
		}
		fprintf(fp, "\"%s\" * \"%s\" *\n", username, passwd);
		fclose(fp);
		chmod("/tmp/ppp/chap-secrets", 0600);

		/*
		 * Enable Forwarding 
		 */
		if ((fp = fopen("/proc/sys/net/ipv4/ip_forward", "r+"))) {
			fputc('1', fp);
			fclose(fp);
		} else
			perror("/proc/sys/net/ipv4/ip_forward");
	}

	/*
	 * Bring up WAN interface 
	 */
	// ifconfig(nvram_safe_get("wan_ifname"), IFUP,
	// nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));

	_log_evalpid(l2tp_argv, NULL, 0, NULL);
	sleep(1);

	if (nvram_matchi("ppp_demand", 1)) {
		/*
		 * Trigger Connect On Demand if user press Connect button in Status
		 * page 
		 */
		if (nvram_match("action_service", "start_l2tp")) {
			start_force_to_dial();
			nvram_unset("action_service");
		}
		/*
		 * Trigger Connect On Demand if user ping pptp server 
		 */
		else
			eval("listen", nvram_safe_get("lan_ifname"));
	} else {
		sysprintf("echo \"c %s\" >  /var/run/xl2tpd/l2tp-control", nvram_safe_get("l2tp_server_name"));
	}

	cprintf("done\n");
	return;
}

void start_l2tp_redial(void)
{
	run_l2tp(REDIAL);
}

void start_l2tp_boot(void)
{
	run_l2tp(BOOT);
}

void stop_l2tp(void)
{
	route_del(nvram_safe_get("wan_ifname"), 0, nvram_safe_get("l2tp_server_ip"), NULL, NULL);

	unlink("/tmp/ppp/link");

	stop_process("pppd", "daemon");
	stop_process("xl2tpd", "daemon");
	stop_process("listen", "connectivity listener");

	cprintf("done\n");
	return;
}
#endif
