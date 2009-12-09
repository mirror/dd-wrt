/*
 * services.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
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

#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 251
#define TXPWR_DEFAULT 70

static int alreadyInHost(char *host)
{
	FILE *in = fopen("/tmp/hosts", "rb");

	if (in == NULL)
		return 0;
	char buf[100];

	while (1) {
		fscanf(in, "%s", buf);
		if (!strcmp(buf, host)) {
			fclose(in);
			return 1;
		}
		if (feof(in)) {
			fclose(in);
			return 0;
		}
	}
}

void addHost(char *host, char *ip)
{
	char buf[100];
	char newhost[100];

	if (host == NULL)
		return;
	if (ip == NULL)
		return;
	strcpy(newhost, host);
	char *domain = nvram_safe_get("lan_domain");

	if (domain != NULL && strlen(domain) > 0 && strcmp(host, "localhost")) {
		sprintf(newhost, "%s.%s", host, domain);
	} else
		sprintf(newhost, "%s", host);

	if (alreadyInHost(newhost))
		return;
	sysprintf("echo \"%s\t%s\">>/tmp/hosts", ip, newhost);
}

/*
 * AhMan March 18 2005 
 */
void start_tmp_ppp(int num);

int write_nvram(char *name, char *nv)
{
	if (nvram_invmatch(nv, "")) {
		FILE *fp = fopen(name, "wb");

		if (fp) {
			fwritenvram(nv, fp);
			fprintf(fp, "\n");
			fclose(fp);
		}
	} else
		return -1;
	return 0;
}

int usejffs = 0;

void stop_dns_clear_resolv(void)
{
	FILE *fp_w;

	if (pidof("dnsmasq") > 0) {
		dd_syslog(LOG_INFO,
			  "dnsmasq : dnsmasq daemon successfully stopped\n");
		killall("dnsmasq", SIGTERM);
	}
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

#if 0
void start_ntpc(void)
{
	char *servers = nvram_safe_get("ntp_server");

	if (!nvram_invmatch("ntpd_enable", "0"))
		return;

	if (strlen(servers)) {
		char *nas_argv[] =
		    { "ntpclient", "-h", servers, "-i", "5", "-l", "-s", "-c",
			"2",
			NULL
		};
		pid_t pid;

		_evalpid(nas_argv, NULL, 0, &pid);
		dd_syslog(LOG_INFO,
			  "ntpclient : ntp client successfully started\n");
	}

	cprintf("done\n");
	return;
}
#endif
void stop_ntpc(void)
{
	if (pidof("ntpclient") > 0) {
		dd_syslog(LOG_INFO,
			  "ntpclient : ntp client successfully stopped\n");
		killall("ntpclient", SIGTERM);
	}
	cprintf("done\n");
	return;
}

// ///////////////////////////////////////////////////
void start_resetbutton(void)
{
	int ret = 0;

	ret = eval("resetbutton");
	dd_syslog(LOG_INFO,
		  "reset button : resetbutton daemon successfully started\n");

	cprintf("done\n");
	return;
}

void stop_resetbutton(void)
{

	if (pidof("resetbutton") > 0) {
		dd_syslog(LOG_INFO,
			  "reset button : resetbutton daemon successfully stopped\n");
		killall("resetbutton", SIGKILL);
	}
	cprintf("done\n");
	return;
}

void start_iptqueue(void)
{
	int ret = 0;

	if (!nvram_invmatch("iptqueue_enable", "0"))
		return;

	ret = eval("iptqueue");
	dd_syslog(LOG_INFO, "iptqueue successfully started\n");

	cprintf("done\n");
	return;
}

void stop_iptqueue(void)
{

	if (pidof("iptqueue") > 0) {
		dd_syslog(LOG_INFO,
			  "iptqueue : iptqueue daemon successfully stopped\n");
		killall("iptqueue", SIGKILL);
	}
	cprintf("done\n");
	return;
}

#ifdef HAVE_IPV6
void start_ipv6(void)
{
	int ret = 0;

	if (!nvram_invmatch("ipv6_enable", "0"))
		return;

	ret = insmod("ipv6");
	dd_syslog(LOG_INFO, "ipv6 successfully started\n");

	cprintf("done\n");
	return;
}
#endif

#ifdef HAVE_3G
void stop_3g(void)
{
	unlink("/tmp/ppp/link");
	if (pidof("pppd") > 0) {
		dd_syslog(LOG_INFO, "3g/umts process successfully stopped\n");
		killall("pppd", SIGTERM);
	}
	cprintf("done\n");
}

#endif

void stop_dhcpc(void)
{

	if (pidof("udhcpc") > 0) {
		dd_syslog(LOG_INFO,
			  "udhcpc : udhcp client process successfully stopped\n");
		killall("udhcpc", SIGTERM);
	}
	cprintf("done\n");
	return;
}

// =====================================================================================================

void stop_wland(void)
{
	if (pidof("wland") > 0) {
		dd_syslog(LOG_INFO,
			  "wland : WLAN daemon successfully stopped\n");
		killall("wland", SIGKILL);
	}
	cprintf("done\n");
	return;
}

void start_wland(void)
{
	int ret;
	pid_t pid;
	char *wland_argv[] = { "wland",
		NULL
	};

	stop_wland();

	// if( nvram_match("apwatchdog_enable", "0") )
	// return 0;

	ret = _evalpid(wland_argv, NULL, 0, &pid);
	dd_syslog(LOG_INFO, "wland : WLAN daemon successfully started\n");
	cprintf("done\n");
	return;
}

void start_process_monitor(void)
{
	if (nvram_match("pmonitor_enable", "0"))
		return;

	pid_t pid;

	char *argv[] = { "process_monitor", NULL };
	int ret = _evalpid(argv, NULL, 0, &pid);

	dd_syslog(LOG_INFO, "process_monitor successfully started\n");

	cprintf("done");

	return;
}

void stop_process_monitor(void)
{

	if (pidof("process_monitor") > 0) {
		dd_syslog(LOG_INFO, "process_monitor successfully stopped\n");
		killall("process_monitor", SIGKILL);
	}
	cprintf("done\n");

	return;
}

void start_radio_timer(void)
{
	if (nvram_match("radio0_timer_enable", "0")
	    && nvram_match("radio1_timer_enable", "0"))
		return;
#ifdef HAVE_MADWIFI
	if (nvram_match("ath0_net_mode", "disabled"))
#else
	if (nvram_match("wl0_net_mode", "disabled")
	    && nvram_match("wl1_net_mode", "disabled"))
#endif
		return;

	pid_t pid;

	char *argv[] = { "radio_timer", NULL };
	int ret = _evalpid(argv, NULL, 0, &pid);

	dd_syslog(LOG_INFO,
		  "radio_timer : radio timer daemon successfully started\n");

	cprintf("done");

	return;
}

void stop_radio_timer(void)
{

	if (pidof("radio_timer") > 0) {
		dd_syslog(LOG_INFO,
			  "radio_timer : radio timer daemon successfully stopped\n");
		killall("radio_timer", SIGKILL);
	}
	cprintf("done\n");

	return;
}

void start_ttraff(void)
{
	if (!nvram_match("ttraff_enable", "1"))
		return;

	if ((nvram_match("ttraff_iface", "") || !nvram_get("ttraff_iface"))
	    && (nvram_match("wan_proto", "disabled")
		|| getWET()))
		return;

	pid_t pid;

	char *argv[] = { "ttraff", NULL };
	int ret = _evalpid(argv, NULL, 0, &pid);

	dd_syslog(LOG_INFO,
		  "ttraff : traffic counter daemon successfully started\n");

	cprintf("done");

	return;
}

void stop_ttraff(void)
{

	if (pidof("ttraff") > 0) {
		dd_syslog(LOG_INFO,
			  "ttraff : traffic counter daemon successfully stopped\n");
		killall("ttraff", SIGKILL);
	}
	cprintf("done\n");

	return;
}

extern void start_heartbeat_boot(void);

/*
 * Trigger Connect On Demand 
 */
void start_force_to_dial(void)
{
	// force_to_dial( char *whichone){
	int ret = 0;
	char dst[50];

	strcpy(&dst[0], nvram_safe_get("wan_gateway"));

	char *ping_argv[] = { "ping",
		"-c", "1",
		dst,
		NULL
	};

	sleep(1);
#ifdef HAVE_L2TP
	if (nvram_match("wan_proto", "l2tp")) {

		sysprintf("echo \"c %s\" >  /var/run/xl2tpd/l2tp-control",
			  nvram_safe_get("l2tp_server_name"));
		return;
	}
#endif
#ifdef HAVE_HEARTBEAT
	if (nvram_match("wan_proto", "heartbeat")) {
		start_heartbeat_boot();
		return;
	}
#endif
	_evalpid(ping_argv, NULL, 3, NULL);

	return;
}
