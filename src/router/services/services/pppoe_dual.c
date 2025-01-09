/*      
 * pppoe_dual.c
 *      
 * Copyright (C) 2013 Markus Quint <markus@dd-wrt.com>
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
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <sys/sysinfo.h>

#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#ifdef __UCLIBC__
#include <error.h>
#endif
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <sys/sysinfo.h>

#include <string.h>
#include <linux/version.h>

#include <linux/sockios.h>
//#include <linux/ethtool.h>
//#include <libbridge.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <code_pattern.h>
#include <wlutils.h>
#include <utils.h>
#include <rc.h>
#include <cy_conf.h>
#include <utils.h>
#include <nvparse.h>
#include <etsockio.h>
#include <bcmparams.h>
#include <services.h>

#ifdef HAVE_PPPOEDUAL
void run_pppoe_dual(int status)
{
	FILE *fp;
	char username[80], passwd[80];
	char idletime[20], retry_num[20];
	char *wan_ifname = nvram_safe_get("wan_ifname");

	if (isClient())
		wan_ifname = getSTA();

#ifdef HAVE_PPPOE
	stop_pppoe();
#endif
#ifdef HAVE_PPTP
	stop_pptp();
#endif
#ifdef HAVE_L2TP
	stop_l2tp();
#endif
	stop_pppoe_dual();
	/*
	if (nvram_matchi("pptp_use_dhcp",0)) {
		ifconfig(wan_ifname, IFUP, nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
		struct dns_lists *dns_list = NULL;
		dns_to_resolv();
		dns_list = get_dns_list();
		int i = 0;

		if (dns_list) {
			for (i = 0; i < dns_list->num_servers; i++)
				route_add(wan_ifname, 0, dns_list->dns_server[i].ip, nvram_safe_get("wan_gateway"), "255.255.255.255");
		}
		route_del(wan_ifname, 0, "0.0.0.0", nvram_safe_get("wan_gateway"), "0.0.0.0");
*/
	/*
		if (dns_list) {
			for (i = 0; i < dns_list->num_servers; i++)
				route_del(wan_ifname, 0, dns_list->dns_server[i].ip, nvram_safe_get("l2tp_wan_gateway"), "255.255.255.255");
			free_dns_list(dns_list);
		}
*/
	//      }

	if (nvram_matchi("pptp_use_dhcp", 0)) {
		ifconfig(wan_ifname, IFUP, nvram_safe_get("wan_ipaddr_static"), nvram_safe_get("wan_netmask_static"));
	}

	snprintf(idletime, sizeof(idletime), "%d", nvram_geti("ppp_idletime") * 60);
	snprintf(retry_num, sizeof(retry_num), "%d", (nvram_geti("ppp_redialperiod") / 5) - 1);

	snprintf(username, sizeof(username), "%s", nvram_safe_get("ppp_username"));
	snprintf(passwd, sizeof(passwd), "%s", nvram_safe_get("ppp_passwd"));

	mkdir("/tmp/ppp", 0777);
	int timeout = 5;

	// Lets open option file and enter all the parameters.
	fp = fopen("/tmp/ppp/options.pppoe", "w");
	// rp-pppoe kernelmode plugin
#if defined(HAVE_ADM5120) && !defined(HAVE_WP54G) && !defined(HAVE_NP28G)
	fprintf(fp, "plugin /lib/rp-pppoe.so\n");
#else
	fprintf(fp, "plugin /usr/lib/rp-pppoe.so\n");
#endif
	if (nvram_invmatch("pppoe_service", ""))
		fprintf(fp, " rp_pppoe_service %s", nvram_safe_get("pppoe_service"));
	if (nvram_invmatch("pppoe_host_uniq", ""))
		fprintf(fp, " host-uniq %s", nvram_safe_get("pppoe_host_uniq"));
	fprintf(fp, "\n");

	char vlannic[32];
	char tvnic[32];
	char *pppoe_wan_ifname = nvram_safe_get("wan_ifname");

	if (!strncmp(pppoe_wan_ifname, "vlan", 4)) {
		char *ifn = enable_dtag_vlan(0);

		sprintf(vlannic, "%s.0007", ifn);
		if (ifexists(vlannic))
			eval("vconfig", "rem", vlannic);
		sprintf(vlannic, "%s.0008", ifn);
		if (ifexists(vlannic))
			eval("vconfig", "rem", vlannic);
		fprintf(fp, "nic-%s\n", pppoe_wan_ifname);

	} else {
		sprintf(vlannic, "%s.0008", pppoe_wan_ifname);
		if (ifexists(vlannic))
			eval("vconfig", "rem", vlannic);
		sprintf(vlannic, "%s.0007", pppoe_wan_ifname);
		if (ifexists(vlannic))
			eval("vconfig", "rem", vlannic);
		fprintf(fp, "nic-%s\n", pppoe_wan_ifname);
	}

	// Those are default options we use + user/passwd
	// By using user/password options we dont have to deal with chap/pap
	// secrets files.
	if (nvram_matchi("ppp_compression", 1)) {
		fprintf(fp, "mppc\n");
	} else {
		fprintf(fp, "noccp\n");
		fprintf(fp, "nomppc\n");
	}
	fprintf(fp, "noipdefault\n"
		    "noauth\n"
		    "defaultroute\n"
		    "noaccomp\n"
		    "nobsdcomp\n"
		    "nodeflate\n"
		    // "debug\n"
		    // "maxfail 0\n"
		    // "nocrtscts\n"
		    // "sync\n"
		    // "local\n"
		    // "noixp\n"
		    // "lock\n"
		    // "noproxyarp\n"
		    // "ipcp-accept-local\n"
		    // "ipcp-accept-remote\n"
		    // "nodetach\n"
		    "nopcomp\n");
	// "novj\n"
	// "novjccomp\n");
	if (nvram_invmatch("ppp_mppe", ""))
		fprintf(fp, "%s\n", nvram_safe_get("ppp_mppe"));
	else
		fprintf(fp, "nomppe\n");
	if (nvram_matchi("ppp_mlppp", 1))
		fprintf(fp, "mp\n");
	fprintf(fp,
		"usepeerdns\nuser '%s'\n"
		"password '%s'\n",
		username, passwd);

	// This is a tricky one. When used it could improve speed of PPPoE
	// but not all ISP's can support it.
	// default-asyncmap escapes all control characters. By using asyncmap
	// 0 PPPD will not escape any control characters
	// Not all ISP's can handle this. By default use default-asyncmap
	// and if ppp_asyncmap=1 do not escape
	if (nvram_matchi("ppp_asyncmap", 1))
		fprintf(fp, "asyncmap 0\n");
	else
		fprintf(fp, "default-asyncmap\n");

	// Allow users some control on PPP interface MTU and MRU
	// If pppoe_ppp_mtu > 0 will set mtu of pppX interface to the value
	// in the nvram variable
	// If pppoe_ppp_mru > 0 will set mru of pppX interface to the value
	// in the nvram variable
	// if none is specified PPPD will autonegotiate the values with ISP
	// (sometimes not desirable)
	// Do not forget this should be at least 8 bytes less then physycal
	// interfaces mtu.

	// if MRU is not Auto force MTU/MRU of interface to value selected by
	// theuser on web page
	if (nvram_matchi("mtu_enable", 1)) {
		if (nvram_geti("wan_mtu") > 0) {
			fprintf(fp, "mtu %s\n", nvram_safe_get("wan_mtu"));
			fprintf(fp, "mru %s\n", nvram_safe_get("wan_mtu"));
		}
	} else {
		// If MRU set to Auto we still allow custom MTU/MRU settings for
		// expirienced users
		if (nvram_invmatch("pppoe_ppp_mtu", ""))
			if (nvram_geti("pppoe_ppp_mtu") > 0)
				fprintf(fp, "mtu %s\n", nvram_safe_get("pppoe_ppp_mtu"));
		if (nvram_invmatch("pppoe_ppp_mru", ""))
			if (nvram_geti("pppoe_ppp_mru") > 0)
				fprintf(fp, "mru %s\n", nvram_safe_get("pppoe_ppp_mru"));
	}

	// Allow runtime debugging
	if (nvram_matchi("ppp_debug", 1))
		fprintf(fp, "debug\n");

	fprintf(fp, "persist\n" //
		    "lcp-echo-interval 3\n" //
		    "lcp-echo-failure 20\n" //
		    "lcp-echo-adaptive\n");
#ifdef HAVE_IPV6
	if (nvram_matchi("ipv6_enable", 1))
		fprintf(fp, "ipv6 ,\n");
#endif
	fclose(fp);

	symlink("/sbin/rc", "/tmp/ppp/ip-up");
	symlink("/sbin/rc", "/tmp/ppp/ip-down");
	unlink("/tmp/ppp/log");

	// Clean pppoe linksys client files - Added by ice-man (Wed Jun 1)
	unlink("/tmp/ppp/connect-log");
	unlink("/tmp/ppp/set-pppoepid");

	stop_dhcpc();
#ifdef HAVE_PPTP
	stop_pptp();
#endif
	stop_process("pppd", "daemon");
	start_pppmodules();
	log_eval("pppd", "file", "/tmp/ppp/options.pppoe");

	if (status != REDIAL) {
		start_redial();
	}

	wan_done(nvram_safe_get("pppoe_ifname"));
}

void stop_pppoe_dual()
{
	unlink("/tmp/ppp/link");
	stop_process("pppd", "pppoe daemon");

	return;
}
#endif
