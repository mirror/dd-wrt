/*
 * l2tp.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <sys/ioctl.h>		/* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>
#include <net/route.h>		/* AhMan March 18 2005 */
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
void start_l2tp(int status)
{
	int ret;
	FILE *fp;
	char *l2tp_argv[] = { "xl2tpd",
		NULL
	};
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
	if (nvram_match("l2tp_use_dhcp", "0")) {
		ifconfig(wan_ifname, IFUP, nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
		struct dns_lists *dns_list = NULL;
		dns_to_resolv();
		dns_list = get_dns_list();
		int i = 0;

		if (dns_list) {
			for (i = 0; i < dns_list->num_servers; i++)
				route_add(wan_ifname, 0, dns_list->dns_server[i], nvram_safe_get("l2tp_wan_gateway"), "255.255.255.255");
		}
		route_add(wan_ifname, 0, "0.0.0.0", nvram_safe_get("l2tp_wan_gateway"), "0.0.0.0");
		char pptpip[64];
		getIPFromName(nvram_safe_get("l2tp_server_name"), pptpip);
		route_del(wan_ifname, 0, "0.0.0.0", nvram_safe_get("l2tp_wan_gateway"), "0.0.0.0");
		if (dns_list) {
			for (i = 0; i < dns_list->num_servers; i++)
				route_del(wan_ifname, 0, dns_list->dns_server[i], nvram_safe_get("l2tp_wan_gateway"), "255.255.255.255");
			free(dns_list);
		}

		nvram_set("l2tp_server_ip", pptpip);
		if (!nvram_match("l2tp_wan_gateway", "0.0.0.0"))
			route_add(wan_ifname, 0, nvram_safe_get("l2tp_server_ip"), nvram_safe_get("l2tp_wan_gateway"), "255.255.255.255");
	}

	snprintf(username, sizeof(username), "%s", nvram_safe_get("ppp_username"));
	snprintf(passwd, sizeof(passwd), "%s", nvram_safe_get("ppp_passwd"));

	if (status != REDIAL) {
		insmod("ipv6");
		insmod("l2tp_core");
		insmod("l2tp_netlink");
		insmod("l2tp_ppp");
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
/*[global]
port = 1701
;auth file = /etc/xl2tpd/xl2tp-secrets

[lac fbnl2tpserver]
lns = 10.64.1.237
require chap = yes
refuse pap = yes
require authentication = yes
; Name should be the same as the username in the PPP authentication!
name = dani
ppp debug = yes
pppoptfile = /etc/xl2tpd/options.l2tp
length bit = yes
*/

		fprintf(fp, "[global]\n");	// Global section
		fprintf(fp, "port = 1701\n");	// Bind address
		fprintf(fp, "[lac %s]\n", nvram_safe_get("l2tp_server_name"));
		fprintf(fp, "lns = %s\n", nvram_safe_get("l2tp_server_name"));
		fprintf(fp, "require chap = %s\n", nvram_default_get("l2tp_req_chap", "yes"));
		fprintf(fp, "refuse pap = %s\n", nvram_default_get("l2tp_ref_pap", "yes"));
		fprintf(fp, "redial = yes\n");
		fprintf(fp, "redial timeout = 15\n");
		fprintf(fp, "require authentication = %s\n", nvram_default_get("l2tp_req_auth", "yes"));
		fprintf(fp, "name = %s\n", username);
		fprintf(fp, "pppoptfile = /tmp/ppp/options\n");
		fprintf(fp, "length bit = yes\n");
		fclose(fp);

		/*
		 * Generate options file 
		 */
		if (!(fp = fopen("/tmp/ppp/options", "w"))) {
			perror("/tmp/ppp/options");
			return;
		}

		if (nvram_match("mtu_enable", "1")) {
			if (atoi(nvram_safe_get("wan_mtu")) > 0) {
				fprintf(fp, "mtu %s\n", nvram_safe_get("wan_mtu"));
				fprintf(fp, "mru %s\n", nvram_safe_get("wan_mtu"));
			}

		}

		fprintf(fp, "defaultroute\n");	// Add a default route to the 
		// system routing tables,
		// using the peer as the
		// gateway
		fprintf(fp, "usepeerdns\n");	// Ask the peer for up to 2 DNS
		// server addresses
		// fprintf(fp, "pty 'pptp %s
		// --nolaunchpppd'\n",nvram_safe_get("pptp_server_ip")); 
		fprintf(fp, "user '%s'\n", username);
		// fprintf(fp, "persist\n"); // Do not exit after a connection is
		// terminated.

		if (nvram_match("ppp_demand", "1")) {	// demand mode
			fprintf(fp, "idle %d\n", nvram_match("ppp_demand", "1") ? atoi(nvram_safe_get("ppp_idletime")) * 60 : 0);
			// fprintf(fp, "demand\n"); // Dial on demand
			// fprintf(fp, "persist\n"); // Do not exit after a connection is 
			// terminated.
			// fprintf(fp, "%s:%s\n",PPP_PSEUDO_IP,PPP_PSEUDO_GW); // <local
			// IP>:<remote IP>
			fprintf(fp, "ipcp-accept-remote\n");
			fprintf(fp, "ipcp-accept-local\n");
			fprintf(fp, "connect true\n");
			fprintf(fp, "noipdefault\n");	// Disables the default
			// behaviour when no local IP 
			// address is specified
			fprintf(fp, "ktune\n");	// Set /proc/sys/net/ipv4/ip_dynaddr
			// to 1 in demand mode if the local
			// address changes
		} else {	// keepalive mode
			start_redial();
		}

		fprintf(fp, "default-asyncmap\n");	// Disable asyncmap
		fprintf(fp, "crtscts\n");	// Disable protocol field compression
		// negotiation
		fprintf(fp, "nopcomp\n");	// Disable protocol field compression
		fprintf(fp, "refuse-eap\n");	// Disable protocol field compression
		fprintf(fp, "noaccomp\n");	// Disable Address/Control
		// compression 
		fprintf(fp, "noccp\n");	// Disable CCP (Compression Control
		// Protocol)
		fprintf(fp, "novj\n");	// Disable Van Jacobson style TCP/IP
		// header compression
		fprintf(fp, "nobsdcomp\n");	// Disables BSD-Compress compression
		fprintf(fp, "nodeflate\n");	// Disables Deflate compression
		//fprintf(fp, "lcp-echo-interval 0\n");	// Don't send an LCP
		fprintf(fp, "lcp-echo-failure 20\n");
        	fprintf(fp, "lcp-echo-interval 3\n");   // echo-request frame to the peer	
		fprintf(fp, "lock\n");
		fprintf(fp, "noauth\n");
//              fprintf(fp, "debug\n");

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

	ret = _evalpid(l2tp_argv, NULL, 0, NULL);
	sleep(1);

	if (nvram_match("ppp_demand", "1")) {
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
	start_l2tp(REDIAL);
}

void start_l2tp_boot(void)
{
	start_l2tp(BOOT);
}

void stop_l2tp(void)
{
	route_del(nvram_safe_get("wan_ifname"), 0, nvram_safe_get("l2tp_server_ip"), NULL, NULL);

	unlink("/tmp/ppp/link");

	stop_process("pppd", "ppp daemon");
	stop_process("xl2tpd", "L2TP daemon");
	stop_process("listen", "connectivity listener");

	cprintf("done\n");
	return;
}
#endif
