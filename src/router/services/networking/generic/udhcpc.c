/*
 * udhcpc.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <utils.h>
#include <services.h>
#include <airbag.h>

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

static int expires(unsigned int in)
{
	struct sysinfo info;
	FILE *fp;

	sysinfo(&info);

	/*
	 * Save uptime ranther than system time, because the system time may
	 * change 
	 */
	if (!(fp = fopen("/tmp/udhcpc.expires", "w"))) {
		perror("/tmp/udhcpd.expires");
		return errno;
	}
	fprintf(fp, "%d", (unsigned int)info.uptime + in);
	fclose(fp);
	return 0;
}

/*
 * deconfig: This argument is used when udhcpc starts, and when a
 * leases is lost. The script should put the interface in an up, but
 * deconfigured state.
 */
static int deconfig(void)
{
	char *wan_ifname = safe_getenv("interface");

	eval("ifconfig", wan_ifname, "0.0.0.0", "up");
	expires(0);

	nvram_set("wan_ipaddr", "0.0.0.0");
	nvram_set("wan_netmask", "0.0.0.0");
	nvram_set("wan_gateway", "0.0.0.0");
	nvram_set("wan_get_dns", "");
	// nvram_set("wan_wins","0.0.0.0"); // Don't care for linksys spec
	nvram_seti("wan_lease", 0);

	unlink("/tmp/get_lease_time");
	unlink("/tmp/lease_time");

	cprintf("done\n");
	return 0;
}

// ==================================================================
static int update_value(void)
{
	char *value;
	int changed = 0;

	if ((value = getenv("ip"))) {
		chomp(value);
		if (nvram_invmatch("wan_ipaddr", value)) {
			nvram_set("wan_ipaddr", value);
			changed++;
		}
	}
	if ((value = getenv("subnet"))) {
		chomp(value);
		if (nvram_invmatch("wan_netmask", value)) {
			nvram_set("wan_netmask", value);
			changed++;
		}
	}
	if ((value = getenv("router"))) {
		chomp(value);
		if (nvram_invmatch("wan_gateway", value)) {
			nvram_set("wan_gateway", value);
			changed++;
		}
	}
	if ((value = getenv("dns"))) {
		chomp(value);
		if (nvram_invmatch("wan_get_dns", value)) {
			nvram_set("wan_get_dns", value);
			changed++;
		}
	}
	/*
	 * if ((value = getenv("wins"))) nvram_set("wan_wins", value); if ((value 
	 * = getenv("hostname"))) sethostname(value, strlen(value) + 1); 
	 */
	if ((value = getenv("domain"))) {
		chomp(value);
		if (nvram_invmatch("wan_get_domain", value)) {
			nvram_set("wan_get_domain", value);
			changed++;
		}
	}
	if ((value = getenv("lease"))) {
		chomp(value);
		if (nvram_invmatch("wan_lease", value)) {
			nvram_set("wan_lease", value);
			changed++;
		}
		expires(atoi(value));
	}

	if (changed) {
		set_host_domain_name();
		stop_unbound();
		start_unbound();
	}
	return 0;
}

static int mbim(void)
{
	stop_firewall();
	cprintf("start wan done\n");
	wan_done(nvram_safe_get("wan_ifname"));
	nvram_set("dhcpc_done", "1");
	cprintf("done\n");
	return 0;
}

// =================================================================

/*
 * bound: This argument is used when udhcpc moves from an unbound, to
 * a bound state. All of the paramaters are set in enviromental
 * variables, The script should configure the interface, and set any
 * other relavent parameters (default gateway, dns server, etc).
 */
#ifdef HAVE_HEARTBEAT
extern void start_heartbeat_boot(void);
#endif
static int bound(void)
{
	nvram_unset("dhcpc_done");
	char *wan_ifname = safe_getenv("interface");
	char *value;
	char temp_wan_ipaddr[16], temp_wan_netmask[16], temp_wan_gateway[16];
	int changed = 0;
	char *cidr;
	if (nvram_match("wan_proto", "iphone"))
		stop_process("ipheth-loop", "IPhone Pairing Daemon");

#ifdef HAVE_BUSYBOX_UDHCPC
	if (wan_ifname) {
		system("/etc/cidrroute.sh /tmp/udhcpstaticroutes");
	}
#else
	cidr = getenv("cidrroute");
	if (cidr && wan_ifname) {
		char *callbuffer = malloc(strlen(cidr) + 128);
		sprintf(callbuffer,
			"export cidrroute=\"%s\";export interface=\"%s\";/etc/cidrroute.sh",
			cidr, wan_ifname);
		system(callbuffer);
		free(callbuffer);
	}
#endif
	if ((value = getenv("ip"))) {
		chomp(value);
		if (nvram_match("wan_proto", "pptp") &&
		    nvram_matchi("pptp_use_dhcp", 1))
			strcpy(temp_wan_ipaddr, value);
		else if (nvram_match("wan_proto", "l2tp") &&
			 nvram_matchi("l2tp_use_dhcp", 1))
			strcpy(temp_wan_ipaddr, value);
		else if (nvram_match("wan_proto", "pppoe_dual") &&
			 nvram_matchi("pptp_use_dhcp", 1))
			strcpy(temp_wan_ipaddr, value);
		else {
			if (nvram_invmatch("wan_ipaddr", value))
				changed = 1;
		}
		nvram_set("wan_ipaddr", value);
	}
	if ((value = getenv("subnet"))) {
		chomp(value);
		if (nvram_match("wan_proto", "pptp") &&
		    nvram_matchi("pptp_use_dhcp", 1))
			strcpy(temp_wan_netmask, value);
		else if (nvram_match("wan_proto", "l2tp") &&
			 nvram_matchi("l2tp_use_dhcp", 1))
			strcpy(temp_wan_netmask, value);
		else if (nvram_match("wan_proto", "pppoe_dual") &&
			 nvram_matchi("pptp_use_dhcp", 1))
			strcpy(temp_wan_netmask, value);
		else {
			if (nvram_invmatch("wan_netmask", value))
				changed = 1;
			nvram_set("wan_netmask", value);
		}
	}
	if ((value = getenv("router"))) {
		chomp(value);
		if (nvram_invmatch("wan_gateway", value))
			changed = 1;
		nvram_set("wan_gateway", value);
	}
	if ((value = getenv("dns"))) {
		chomp(value);
		// if (nvram_invmatch("wan_get_dns",value))
		// changed=1;
		nvram_set("wan_get_dns", value);
	}
	/*
	 * Don't care for linksys spec if ((value = getenv("wins")))
	 * nvram_set("wan_wins", value); if ((value = getenv("hostname")))
	 * sethostname(value, strlen(value) + 1); 
	 */
	if ((value = getenv("domain"))) {
		chomp(value);
		if (nvram_invmatch("wan_get_domain", value))
			changed = 1;
		nvram_set("wan_get_domain", value); // HeartBeat need to use
	}
	if ((value = getenv("lease"))) {
		chomp(value);
		nvram_set("wan_lease", value);
		expires(atoi(value));
	}
	if (!changed) {
		//      dd_loginfo("udhcpc", "dhcp lease info hasn't changed, do nothing\n");
		return 0;
	}
	stop_firewall();

	if (nvram_match("wan_proto", "pptp") &&
	    nvram_matchi("pptp_use_dhcp", 1))
		eval("ifconfig", wan_ifname, temp_wan_ipaddr, "netmask",
		     temp_wan_netmask, "up");
	else if (nvram_match("wan_proto", "l2tp") &&
		 nvram_matchi("l2tp_use_dhcp", 1))
		eval("ifconfig", wan_ifname, temp_wan_ipaddr, "netmask",
		     temp_wan_netmask, "up");
	else if (nvram_match("wan_proto", "pppoe_dual") &&
		 nvram_matchi("pptp_use_dhcp", 1))
		eval("ifconfig", wan_ifname, temp_wan_ipaddr, "netmask",
		     temp_wan_netmask, "up");
	else
		eval("ifconfig", wan_ifname, nvram_safe_get("wan_ipaddr"),
		     "netmask", nvram_safe_get("wan_netmask"), "up");

		/*
	 * We only want to exec bellow functions after dhcp get ip if the
	 * wan_proto is heartbeat 
	 */
#ifdef HAVE_HEARTBEAT
	if (nvram_match("wan_proto", "heartbeat")) {
		int i = 0;

		/*
		 * Delete all default routes 
		 */
		while (route_del(wan_ifname, 0, NULL, NULL, NULL) == 0 ||
		       i++ < 10)
			;

		/*
		 * Set default route to gateway if specified 
		 */
		route_add(wan_ifname, 0, "0.0.0.0",
			  nvram_safe_get("wan_gateway"), "0.0.0.0");

		/*
		 * save dns to resolv.conf 
		 */
		dns_to_resolv();
		start_firewall();
		stop_wland();
		start_wland();
		start_heartbeat_boot();
		stop_unbound();
		start_unbound();
	}
#else
	if (0) {
		// nothing
	}
#endif
#ifdef HAVE_PPTP
	else if (nvram_match("wan_proto", "pptp") &&
		 nvram_matchi("pptp_use_dhcp", 1)) {
		char pptpip[64];
		struct dns_lists *dns_list = NULL;

		dns_to_resolv();

		dns_list = get_dns_list(0);
		int i = 0;

		if (dns_list) {
			for (i = 0; i < dns_list->num_servers; i++)
				route_add(wan_ifname, 0,
					  dns_list->dns_server[i].ip,
					  nvram_safe_get("wan_gateway"),
					  "255.255.255.255");
			free_dns_list(dns_list);
		}
		route_add(wan_ifname, 0, "0.0.0.0",
			  nvram_safe_get("wan_gateway"), "0.0.0.0");

		if (nvram_exists("wan_gateway"))
			nvram_set("wan_gateway_buf",
				  nvram_safe_get("wan_gateway"));
		else
			nvram_unset("wan_gateway_buf");

		getIPFromName(nvram_safe_get("pptp_server_name"), pptpip,
			      sizeof(pptpip));
		if (strcmp(pptpip, "0.0.0.0"))
			nvram_set("pptp_server_ip", pptpip);

		// Add the route to the PPTP server on the wan interface for pptp
		// client to reach it
		if (nvram_match("wan_gateway", "0.0.0.0") ||
		    nvram_match("wan_netmask", "0.0.0.0"))
			route_add(wan_ifname, 0,
				  nvram_safe_get("pptp_server_ip"),
				  nvram_safe_get("wan_gateway_buf"),
				  "255.255.255.255");
		else
			route_add(wan_ifname, 0,
				  nvram_safe_get("pptp_server_ip"),
				  nvram_safe_get("wan_gateway"),
				  nvram_safe_get("wan_netmask"));
	}
#endif
#ifdef HAVE_L2TP
	else if (nvram_match("wan_proto", "l2tp") &&
		 nvram_matchi("l2tp_use_dhcp", 1)) {
		char l2tpip[64];
		struct dns_lists *dns_list = NULL;

		dns_to_resolv();

		dns_list = get_dns_list(0);

		int i = 0;

		if (dns_list) {
			for (i = 0; i < dns_list->num_servers; i++)
				route_add(wan_ifname, 0,
					  dns_list->dns_server[i].ip,
					  nvram_safe_get("wan_gateway"),
					  "255.255.255.255");
			free_dns_list(dns_list);
		}

		/*
		 * Backup the default gateway. It should be used if L2TP connection
		 * is broken 
		 */
		if (nvram_exists("wan_gateway"))
			nvram_set("wan_gateway_buf",
				  nvram_safe_get("wan_gateway"));
		else
			nvram_unset("wan_gateway_buf");

		getIPFromName(nvram_safe_get("l2tp_server_name"), l2tpip,
			      sizeof(l2tpip));
		if (strcmp(l2tpip, "0.0.0.0"))
			nvram_set("l2tp_server_ip", l2tpip);

		//route_add(wan_ifname, 0, nvram_safe_get("l2tp_server_ip"), nvram_safe_get("wan_gateway"), "255.255.255.255");
		route_del(wan_ifname, 0, nvram_safe_get("wan_gateway"), NULL,
			  "255.255.255.255");
		route_add(wan_ifname, 0, nvram_safe_get("l2tp_server_ip"),
			  nvram_safe_get("wan_gateway_buf"), "255.255.255.255");

		start_firewall();
		start_l2tp_boot();
	}
#endif
#ifdef HAVE_PPPOEDUAL
	else if (nvram_match("wan_proto", "pppoe_dual") &&
		 nvram_matchi("pptp_use_dhcp", 1)) {
		struct dns_lists *dns_list = NULL;
		int i;

		dns_to_resolv();

		/*      
		   dns_list = get_dns_list();
		   if (dns_list) {
		   for (i=0; i<dns_list->num_servers; i++)
		   route_add(wan_ifname, 0, dns_list->dns_server[i].ip, nvram_safe_get("wan_gateway"), "255.255.255.255");
		   free_dns_list(dns_list);
		   }
		 */

		start_firewall();
		run_pppoe_dual(BOOT);
	}
#endif
	else {
		cprintf("start wan done\n");
		wan_done(wan_ifname);
	}
	nvram_seti("dhcpc_done", 1);
	cprintf("done\n");
	return 0;
}

/*
 * renew: This argument is used when a DHCP lease is renewed. All of
 * the paramaters are set in enviromental variables. This argument is
 * used when the interface is already configured, so the IP address,
 * will not change, however, the other DHCP paramaters, such as the
 * default gateway, subnet mask, and dns server may change.
 */
static int renew(void)
{
	bound();

	cprintf("done\n");
	return 0;
}

int dhcpc_main(int argc, char **argv)
{
	if (check_action() != ACT_IDLE)
		return -1;

	if (!argv[1])
		return EINVAL;
	else if (strstr(argv[1], "deconfig")) {
		airbag_setpostinfo("dhcpc deconfig");
		return deconfig();
	} else if (strstr(argv[1], "bound")) {
		airbag_setpostinfo("dhcpc bound");
		return bound();
	} else if (strstr(argv[1], "renew")) {
		airbag_setpostinfo("dhcpc renew");
		return renew();
	} else if (strstr(argv[1], "update")) {
		airbag_setpostinfo("dhcpc update");
		return update_value();
	} else if (strstr(argv[1], "mbim")) {
		airbag_setpostinfo("dhcpc renew");
		return mbim();
	} else
		return EINVAL;
}

static int bound_tv(void)
{
	char *ifname = safe_getenv("interface");
	char *ip = safe_getenv("ip");
	char *net = safe_getenv("subnet");
#ifndef HAVE_BUSYBOX_UDHCPC
	char *cidr = safe_getenv("cidrroute");
#endif
	if (ip && net && ifname) {
		char bcast[32];
		strcpy(bcast, ip);
		get_broadcast(bcast, sizeof(bcast), net);
		nvram_set("tvnicaddr", ip);
		eval("ifconfig", ifname, ip, "netmask", net, "broadcast", bcast,
		     "multicast");
	}
#ifdef HAVE_BUSYBOX_UDHCPC
	if (ifname) {
		system("/etc/cidrroute.sh /tmp/tvrouting");
	}
#else
	if (cidr && ifname) {
		char *callbuffer = malloc(strlen(cidr) + 128);
		sprintf(callbuffer,
			"export cidrroute=\"%s\";export interface=\"%s\";/etc/cidrroute.sh",
			cidr, ifname);
		system(callbuffer);
		free(callbuffer);
	}
#endif
	return 0;
}

int dhcpc_tv_main(int argc, char **argv)
{
	if (check_action() != ACT_IDLE)
		return -1;

	if (!argv[1])
		return EINVAL;
	else if (strstr(argv[1], "deconfig"))
		return 0;
	else if (strstr(argv[1], "bound"))
		return bound_tv();
	else if (strstr(argv[1], "renew"))
		return bound_tv();
	else if (strstr(argv[1], "update"))
		return bound_tv();
	else
		return EINVAL;
}
