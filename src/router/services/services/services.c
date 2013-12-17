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

#if 0
void start_ntpc(void)
{
	char *servers = nvram_safe_get("ntp_server");

	if (!nvram_invmatch("ntpd_enable", "0"))
		return;

	if (strlen(servers)) {
		char *nas_argv[] = { "ntpclient", "-h", servers, "-i", "5", "-l", "-s", "-c",
			"2",
			NULL
		};
		pid_t pid;

		_evalpid(nas_argv, NULL, 0, &pid);
		dd_syslog(LOG_INFO, "ntpclient : ntp client successfully started\n");
	}

	cprintf("done\n");
	return;
}
#endif
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

	ret = eval("resetbutton");
	dd_syslog(LOG_INFO, "reset button : resetbutton daemon successfully started\n");

	cprintf("done\n");
	return;
}

void stop_resetbutton(void)
{
	stop_process("resetbutton", "resetbutton daemon");
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
	stop_process("iptqueue", "iptqueue daemon");
	cprintf("done\n");
	return;
}

#ifdef HAVE_IPV6
void start_ipv6(void)
{
	int ret = 0;

	if (!nvram_invmatch("ipv6_enable", "0"))
		return;

	insmod("ipv6");
	insmod("tunnel4");
	insmod("ip_tunnel");
	insmod("sit");
	insmod("xfrm_algo");
	insmod("esp6");
	insmod("ah6");
	insmod("mip6");
	insmod("tunnel6");
	insmod("ip6_tunnel");
	insmod("xfrm6_mode_beet");
	insmod("xfrm6_mode_ro");
	insmod("xfrm6_mode_transport");
	insmod("xfrm6_mode_tunnel");
	insmod("xfrm6_tunnel");
	insmod("xfrm_ipcomp");
	insmod("ipcomp6");

	dd_syslog(LOG_INFO, "ipv6 successfully started\n");

	cprintf("done\n");
	return;
}
#endif

#ifdef HAVE_3G
void stop_3g(void)
{
	unlink("/tmp/ppp/link");
	stop_process("pppd", "3g/umts process");
	cprintf("done\n");
}

#endif

void stop_dhcpc(void)
{
	FILE *fp = fopen("/var/run/udhcpc.pid", "rb");
	if (fp) {
		int pid;
		fscanf(fp, "%d", &pid);
		fclose(fp);
		dd_syslog(LOG_INFO, "udhcpc : udhcp client process successfully stopped\n");
		kill(pid, SIGTERM);
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

		sysprintf("echo \"c %s\" >  /var/run/xl2tpd/l2tp-control", nvram_safe_get("l2tp_server_name"));
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
