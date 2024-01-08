/*
 * pppoe.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

/*
 * AhMan March 18 2005 
 */
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

// =========================================tallest============================================
/*
 * AhMan March 18 2005 Start the Original Linksys PPPoE 
 */
/*
 * This function build the pppoe instuction & execute it.
 */
#ifdef HAVE_PPPOE
/*
 * Get the IP, Subnetmask, Geteway from WAN interface
 * and set to NV ram.
 */
void run_tmp_ppp(int num)
{
	int timeout = 5;
	char pppoeifname[15];
	char wanip[2][15] = { "wan_ipaddr", "wan_ipaddr_1" };
	char wanmask[2][15] = { "wan_netmask", "wan_netmask_1" };
	char wangw[2][15] = { "wan_gateway", "wan_gateway_1" };
	// char wanif[2][15]={"wan_ifname","wan_ifname_1"};
	// char *wan_ifname = nvram_safe_get("wan_ifname");
	struct ifreq ifr;
	int s;

	cprintf("start session %d\n", num);

	sprintf(pppoeifname, "pppoe_ifname%d", num);

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;

	/*
	 * Wait for ppp0 to be created 
	 */
	while (ifconfig(nvram_safe_get(pppoeifname), IFUP, NULL, NULL) && timeout--)
		sleep(1);

	strncpy(ifr.ifr_name, nvram_safe_get(pppoeifname), IFNAMSIZ);

	/*
	 * Set temporary IP address 
	 */
	timeout = 3;
	while (ioctl(s, SIOCGIFADDR, &ifr) && timeout--) {
		perror(nvram_safe_get(pppoeifname));
		printf("Wait %s inteface to init (1) ...\n", nvram_safe_get(pppoeifname));
		sleep(1);
	};
	nvram_set(wanip[num], inet_ntoa(sin_addr(&(ifr.ifr_addr))));
	nvram_set(wanmask[num], "255.255.255.255");

	/*
	 * Set temporary P-t-P address 
	 */
	timeout = 3;
	while (ioctl(s, SIOCGIFDSTADDR, &ifr) && timeout--) {
		perror(nvram_safe_get(pppoeifname));
		printf("Wait %s inteface to init (2) ...\n", nvram_safe_get(pppoeifname));
		sleep(1);
	}
	nvram_set(wangw[num], inet_ntoa(sin_addr(&(ifr.ifr_dstaddr))));

	wan_done(nvram_safe_get(pppoeifname));

	// if user press Connect" button from web, we must force to dial
	if (nvram_match("action_service", "start_pppoe") || nvram_match("action_service", "start_pppoe_1")) {
		sleep(3);
		// force_to_dial(nvram_safe_get("action_service"));
		start_force_to_dial();
		nvram_unset("action_service");
	}

	close(s);
	cprintf("done session %d\n", num);
	return;
}

void run_pppoe(int pppoe_num)
{
	char idletime[20], retry_num[20], param[16];
	char username[80], passwd[80];

	char ppp_username[2][20] = { "ppp_username", "ppp_username_1" };
	char ppp_passwd[2][20] = { "ppp_passwd", "ppp_passwd_1" };
	char ppp_demand[2][20] = { "ppp_demand", "ppp_demand_1" };
	char ppp_service[2][20] = { "ppp_service", "ppp_service_1" };
	char ppp_ac[2][10] = { "ppp_ac", "ppp_ac_1" };
	// char wanip[2][15] = { "wan_ipaddr", "wan_ipaddr_1" };
	// char wanmask[2][15] = { "wan_netmask", "wan_netmask_1" };
	// char wangw[2][15] = { "wan_gateway", "wan_gateway_1" };
	char pppoeifname[15];
	char *wan_ifname = nvram_safe_get("wan_ifname");

	if (isClient()) {
		wan_ifname = getSTA();
	}

	pid_t pid;

	sprintf(pppoeifname, "pppoe_ifname%d", pppoe_num);
	nvram_set(pppoeifname, "");

	cprintf("start session %d\n", pppoe_num);
	sprintf(idletime, "%d", nvram_geti("ppp_idletime") * 60);
	snprintf(retry_num, sizeof(retry_num), "%d", (nvram_geti("ppp_redialperiod") / 5) - 1);

	snprintf(username, sizeof(username), "%s", nvram_safe_get(ppp_username[pppoe_num]));
	snprintf(passwd, sizeof(passwd), "%s", nvram_safe_get(ppp_passwd[pppoe_num]));
	sprintf(param, "%d", pppoe_num);
	/*
	 * add here 
	 */
	char *pppoe_argv
		[] = { "pppoecd",
		       wan_ifname,
		       "-u",
		       username,
		       "-p",
		       passwd,
		       "-r",
		       nvram_safe_get("wan_mtu"), // del by honor, add by tallest.
		       "-t",
		       nvram_safe_get("wan_mtu"),
		       "-i",
		       nvram_matchi(ppp_demand[pppoe_num], 1) ? idletime : "0",
		       "-I",
		       "10", // Send an LCP echo-request frame to the
		       // server every 10 seconds
		       "-T",
		       "20", // pppd will presume the server to be dead if
		       // 20 LCP echo-requests are sent without
		       //-> timeout 1 min
		       // receiving a valid LCP echo-reply
		       "-P",
		       param, // PPPOE session number.
		       "-N",
		       retry_num, // To avoid kill pppd when pppd has been
	// connecting.
#if LOG_PPPOE == 2
		       "-d",
#endif
		       "-C",
		       "disconnected_pppoe", // by tallest 0407
		       NULL, /* set default route */
		       NULL,
		       NULL, /* pppoe_service */
		       NULL,
		       NULL, /* pppoe_ac */
		       NULL, /* pppoe_keepalive */
		       NULL },
     **arg;
	/*
	 * Add optional arguments 
	 */
	for (arg = pppoe_argv; *arg; arg++)
		;

	/*
	 * Removed by AhMan 
	 */

	if (pppoe_num == PPPOE0) { // PPPOE0 must set default route.
		*arg++ = "-R";
	}

	if (nvram_invmatch(ppp_service[pppoe_num], "")) {
		*arg++ = "-s";
		*arg++ = nvram_safe_get(ppp_service[pppoe_num]);
	}
	if (nvram_invmatch(ppp_ac[pppoe_num], "")) {
		*arg++ = "-a";
		*arg++ = nvram_safe_get(ppp_ac[pppoe_num]);
	}
	if (nvram_matchi("ppp_static", 1)) {
		*arg++ = "-L";
		*arg++ = nvram_safe_get("ppp_static_ip");
	}
	// if (nvram_matchi("pppoe_demand",1) || nvram_match("pppoe_keepalive",
	// "1"))
	*arg++ = "-k";

	mkdir("/tmp/ppp", 0777);
	symlink("/sbin/rc", "/tmp/ppp/ip-up");
	symlink("/sbin/rc", "/tmp/ppp/ip-down");
	symlink("/sbin/rc", "/tmp/ppp/set-pppoepid"); // tallest 1219
	unlink("/tmp/ppp/log");

	// Clean rpppoe client files - Added by ice-man (Wed Jun 1)
	unlink("/tmp/ppp/options.pppoe");
	unlink("/tmp/ppp/connect-errors");

	start_pppmodules();
	_log_evalpid(pppoe_argv, NULL, 0, &pid);

	if (nvram_matchi(ppp_demand[pppoe_num], 1)) {
		// int timeout = 5;
		run_tmp_ppp(pppoe_num);

		// This should be handled in wan_done
		// while (ifconfig (nvram_safe_get (pppoeifname), IFUP, NULL, NULL)
		// && timeout--)
		// sleep (1);
		// route_add (nvram_safe_get ("wan_iface"), 0, "0.0.0.0",
		// "10.112.112.112",
		// "0.0.0.0");
	}
	cprintf("done. session %d\n", pppoe_num);
	return;
}

void stop_pppoe(void)
{
	unlink("/tmp/ppp/link");
	stop_process("pppd", "pppoe process");
	if (nvram_matchi("wan_vdsl", 1)) {
		eval("ifconfig", nvram_safe_get("wan_iface"), "down");
		eval("vconfig", "rem", nvram_safe_get("wan_iface"));
	}

	cprintf("done\n");
	return;
}

void stop_dns_clear_resolv(void)
{
}

void start_dns_clear_resolv(void)
{
	FILE *fp_w;
	stop_process("dnsmasq", "daemon");
	/*
	 * Save DNS to resolv.conf 
	 */
	if (!(fp_w = fopen(RESOLV_FILE, "w"))) {
		perror(RESOLV_FILE);
		return;
	}
	fprintf(fp_w, " ");
	fclose(fp_w);

	cprintf("done\n");
	return;
}

void single_pppoe_stop(int pppoe_num)
{
	int ret;
	char pppoe_pid[15], pppoe_ifname[15];
	char ppp_unlink[2][20] = { "/tmp/ppp/link", "/tmp/ppp/link_1" };
	char ppp_wan_dns[2][20] = { "wan_get_dns", "wan_get_dns_1" };

	sprintf(pppoe_pid, "pppoe_pid%d", pppoe_num);
	sprintf(pppoe_ifname, "pppoe_ifname%d", pppoe_num);
	dprintf("start! stop pppoe %d, pid %s \n", pppoe_num, nvram_safe_get(pppoe_pid));

	ret = eval("kill", nvram_safe_get(pppoe_pid));
	unlink(ppp_unlink[pppoe_num]);
	nvram_unset(pppoe_ifname);

	nvram_set(ppp_wan_dns[pppoe_num], "");
	stop_dns_clear_resolv();

	dprintf("done\n");
	return;
}

#endif
