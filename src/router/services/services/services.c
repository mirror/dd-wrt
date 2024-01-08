/*
 * services.c
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

#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 1000
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

void addHost(char *host, char *ip, int withdomain);

void genHosts(void)
{
	eval("rm", "/tmp/hosts");
	addHost("localhost", "127.0.0.1", 0);
	if (*(nvram_safe_get("wan_hostname"))) {
		addHost(nvram_safe_get("wan_hostname"),
			nvram_safe_get("lan_ipaddr"), 0);
		addHost(nvram_safe_get("wan_hostname"),
			nvram_safe_get("lan_ipaddr"), 1);
	} else if (*(nvram_safe_get("router_name"))) {
		addHost(nvram_safe_get("router_name"),
			nvram_safe_get("lan_ipaddr"), 0);
		addHost(nvram_safe_get("router_name"),
			nvram_safe_get("lan_ipaddr"), 1);
	}
}

void addHost(char *host, char *ip, int withdomain)
{
	char buf[100];
	char newhost[100];

	if (host == NULL)
		return;
	if (ip == NULL)
		return;
	strcpy(newhost, host);
	char *domain = nvram_safe_get("lan_domain");

	if (withdomain && domain != NULL && *domain &&
	    strcmp(host, "localhost")) {
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

int usejffs = 0;

void stop_ntpc(void)
{
	stop_process("ntpclient", "Network Time Protocol client");
	cprintf("done\n");
	return;
}

// ///////////////////////////////////////////////////
void start_resetbutton(void)
{
	int ret = 0;

	log_eval("resetbutton");

	cprintf("done\n");
	return;
}

void stop_resetbutton(void)
{
	stop_process("resetbutton", "daemon");
	cprintf("done\n");
	return;
}

void start_iptqueue(void)
{
	int ret = 0;

	if (!nvram_invmatchi("iptqueue_enable", 0))
		return;

	log_eval("iptqueue");

	cprintf("done\n");
	return;
}

void stop_iptqueue(void)
{
	stop_process("iptqueue", "daemon");
	cprintf("done\n");
	return;
}

#ifdef HAVE_3G
void stop_3g(void)
{
	unlink("/tmp/ppp/link");
	stop_process("pppd", "3g/umts/lte process");
	cprintf("done\n");
}

void start_check_sierradirectip(void)
{
	eval("/etc/comgt/sierrastatus.sh", nvram_safe_get("3gcontrol"), "dip");
}

void start_check_sierrappp(void)
{
	eval("/etc/comgt/sierrastatus.sh", nvram_safe_get("3gcontrol"));
}

#if defined(HAVE_LIBMBIM) || defined(HAVE_UMBIM)
void start_check_mbim(void)
{
#ifdef HAVE_REGISTER
	if (registered_has_cap(27))
#endif
	{
		dd_loginfo("mbim", "STARTING mbim-status.sh\n");
		eval("/usr/sbin/mbim-status.sh");
	}
}
#endif

#endif

void start_pppmodules(void)
{
	insmod("zlib_inflate zlib_deflate slhc crc-ccitt ppp_generic bsd_comp ppp_deflate ppp_async ppp_synctty ppp_mppe pppox pppoe");
}

void stop_pppmodules(void)
{
	rmmod("pppoe pppox ppp_mppe ppp_synctty ppp_async ppp_deflate bsd_comp ppp_generic slhc crc-ccitt");
}

void stop_dhcpc(void)
{
	FILE *fp = fopen("/var/run/udhcpc.pid", "rb");
	if (fp) {
		int pid;
		fscanf(fp, "%d", &pid);
		fclose(fp);
		dd_loginfo("udhcpc",
			   "udhcp client process successfully stopped\n");
		kill(pid, SIGTERM);
	}
#ifdef HAVE_3G
	if (nvram_match("3gdata", "sierradirectip")) {
		sysprintf("comgt -d %s -s /etc/comgt/hangup-dip.comgt\n",
			  nvram_safe_get("3gcontrol"));
	}
#endif
#ifdef HAVE_LIBMBIM
	if (nvram_match("3gdata", "mbim")) {
		sysprintf(
			"/usr/sbin/mbim-network --profile=/tmp/mbim-net-conf.conf /dev/cdc-wdm0 stop\n");
		sysprintf("echo 0 >/tmp/mbimstatus\n");
	}
#endif
#ifdef HAVE_UQMI
	if (nvram_match("3gdata", "qmi")) {
		sysprintf(
			"/usr/sbin/uqmi -s -d /dev/cdc-wdm0 --stop-network %s\n",
			nvram_safe_get("3g_pdh"));
		sysprintf("echo 0 >/tmp/qmistatus\n");
	}
#endif
	cprintf("done\n");
	return;
}

extern void start_heartbeat_boot(void);

void stop_force_to_dial(void)
{
}

/*
 * Trigger Connect On Demand 
 */
void start_force_to_dial(void)
{
	// force_to_dial( char *whichone){
	int ret = 0;
	char dst[50];

	strcpy(&dst[0], nvram_safe_get("wan_gateway"));

	char *ping_argv[] = { "ping", "-c", "1", dst, NULL };

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
