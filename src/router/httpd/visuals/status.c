/*
 * status.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <unistd.h>
// #ifdef CDEBUG
// #include <utils.h>
// #endif
#include <broadcom.h>
#include <netdb.h>
#include <wlutils.h>

#define	STATUS_RETRY_COUNT	10
#define STATUS_REFRESH_TIME1	5
#define STATUS_REFRESH_TIME2	60

int retry_count = -1;
int refresh_time = STATUS_REFRESH_TIME2;

void ej_show_status_setting(webs_t wp, int argc, char_t ** argv)
{

	do_ej(METHOD_GET, NULL, "Status_Router1.asp", wp);

	return;
}

char *rfctime(const time_t * timep, char *s)
{
	struct tm tm;
	localtime_r(timep, &tm);
	strftime(s, 200, "%a, %d %b %Y %H:%M:%S", &tm);	// spec for linksys
	return s;
}

/*
 * Report time in RFC-822 format 
 */
void ej_localtime(webs_t wp, int argc, char_t ** argv)
{
	time_t tm;

	time(&tm);

	if (time(0) > (unsigned long)31536000)	// 60 * 60 * 24 * 365
	{
		char t[64];

		websWrite(wp, rfctime(&tm, t));
	} else {
		if (argc && !strcmp(argv[0], "1"))
			show_caption_simple(wp, "status_router.notavail");
		else
			websWrite(wp, "%s", live_translate("status_router.notavail"));
	}
}

void ej_dhcp_remaining_time(webs_t wp, int argc, char_t ** argv)
{
	// tofu12

	if (nvram_invmatch("wan_proto", "dhcp"))
		return;

	long exp;
	char buf[128];
	struct sysinfo si;
	long n;

	exp = 0;
	if (file_to_buf("/tmp/udhcpc.expires", buf, sizeof(buf))) {
		n = atol(buf);
		if (n > 0) {
			sysinfo(&si);
			exp = n - si.uptime;
		}
	}
	websWrite(wp, dhcp_reltime(buf, exp));

	return;
}

void ej_nvram_status_get(webs_t wp, int argc, char_t ** argv)
{
	char *type;
	char *wan_ipaddr, *wan_netmask, *wan_gateway;
	char *status1 = "", *status2 = "", *hidden1, *hidden2, *button1 = "";
	char *wan_proto = nvram_safe_get("wan_proto");
	struct dns_lists *dns_list = NULL;
	int wan_link = check_wan_link(0);
	int trans = 0;
	type = argv[0];
	if (argc > 1)
		trans = atoi(argv[1]);
	if (!strcmp(wan_proto, "pptp")) {
		wan_ipaddr = wan_link ? nvram_safe_get("pptp_get_ip") : nvram_safe_get("wan_ipaddr");
		wan_netmask = wan_link ? nvram_safe_get("wan_netmask") : nvram_safe_get("wan_netmask");
		wan_gateway = wan_link ? nvram_safe_get("wan_gateway") : nvram_safe_get("pptp_server_ip");
	} else if (!strcmp(wan_proto, "pppoe")
#ifdef HAVE_PPPOEDUAL
		   || !strcmp(wan_proto, "pppoe_dual")
#endif
#ifdef HAVE_PPPOATM
		   || !strcmp(wan_proto, "pppoa")
#endif
#ifdef HAVE_3G
		   || !strcmp(wan_proto, "3g")
#endif
#ifdef HAVE_IPETH
		   || !strcmp(wan_proto, "iphone")
#endif
	    ) {
		wan_ipaddr = wan_link ? nvram_safe_get("wan_ipaddr") : "0.0.0.0";
		wan_netmask = wan_link ? nvram_safe_get("wan_netmask") : "0.0.0.0";
		wan_gateway = wan_link ? nvram_safe_get("wan_gateway") : "0.0.0.0";
	}
#ifdef HAVE_L2TP
	else if (!strcmp(wan_proto, "l2tp")) {
		wan_ipaddr = wan_link ? nvram_safe_get("l2tp_get_ip") : nvram_safe_get("wan_ipaddr");
		wan_netmask = wan_link ? nvram_safe_get("wan_netmask") : nvram_safe_get("wan_netmask");
		wan_gateway = wan_link ? nvram_safe_get("wan_gateway") : nvram_safe_get("wan_gateway");
	}
#endif
	else {
		wan_ipaddr = nvram_safe_get("wan_ipaddr");
		wan_gateway = nvram_safe_get("wan_gateway");
		wan_netmask = nvram_safe_get("wan_netmask");
	}

	dns_list = get_dns_list();

	if (!strcmp(wan_proto, "pppoe")
	    || !strcmp(wan_proto, "pptp")
#ifdef HAVE_PPPOEDUAL
	    || !strcmp(wan_proto, "pppoe_dual")
#endif
#ifdef HAVE_3G
	    || !strcmp(wan_proto, "3g")
#endif
#ifdef HAVE_IPETH
	    || !strcmp(wan_proto, "iphone")
#endif
#ifdef HAVE_PPPOATM
	    || !strcmp(wan_proto, "pppoa")
#endif
#ifdef HAVE_L2TP
	    || !strcmp(wan_proto, "l2tp")
#endif
	    || !strcmp(wan_proto, "heartbeat")) {
		hidden1 = "";
		hidden2 = "";
		if (wan_link == 0) {
			// submit_button old format is "Connect", new format is
			// "Connect_pppoe" or "Connect_pptp" or "Connect_heartbeat"
			// if(submit_type && !strncmp(submit_type,"Connect",7) &&
			// retry_count != -1){
			if (retry_count != -1) {
				status1 = "share.statu";
				status2 = "share.connecting";
				if (trans)
					button1 = "share.disconnect";
				else
					button1 = "Disconnect";
			} else {
				status1 = "share.statu";
				status2 = "share.disconnected";
				if (trans)
					button1 = "share.connect";
				else
					button1 = "Connect";
			}
		} else {
			retry_count = -1;
			status1 = "share.statu";
			status2 = "share.connected";
			if (trans)
				button1 = "share.disconnect";
			else
				button1 = "Disconnect";
		}
	} else {
		status1 = "share.disable";	// only for nonbrand
		status2 = "&nbsp;";
		hidden1 = "<!--";
		hidden2 = "-->";
	}
	char buf[128];
	if (!strcmp(type, "wan_ipaddr")) {
		if (getWET() || !strcmp(wan_proto, "disabled")) {
			websWrite(wp, "%s", trans == 2 ? tran_string(buf, "share.disabled") : live_translate("share.disabled"));
		} else
			websWrite(wp, "%s", wan_ipaddr);
	} else if (!strcmp(type, "wan_netmask"))
		websWrite(wp, "%s", wan_netmask);
	else if (!strcmp(type, "wan_gateway"))
		websWrite(wp, "%s", wan_gateway);
	else if (!strcmp(type, "wan_dns0")) {
		websWrite(wp, "%s", get_dns_entry(dns_list, 0));
	} else if (!strcmp(type, "wan_dns1")) {
		websWrite(wp, "%s", get_dns_entry(dns_list, 1));
	} else if (!strcmp(type, "wan_dns2")) {
		websWrite(wp, "%s", get_dns_entry(dns_list, 2));
	} else if (!strcmp(type, "status1"))
		websWrite(wp, "%s", trans == 3 ? status1 : trans == 2 ? tran_string(buf, status1) : live_translate(status1));
	else if (!strcmp(type, "status2"))
		websWrite(wp, "%s", trans == 3 ? status2 : trans == 2 ? tran_string(buf, status2) : live_translate(status2));
	else if (!strcmp(type, "button1")) {
		if (trans)
			websWrite(wp, "%s", trans == 3 ? button1 : trans == 2 ? tran_string(buf, button1) : live_translate(button1));
		else
			websWrite(wp, "%s", button1);
	} else if (!strcmp(type, "hidden1"))
		websWrite(wp, "%s", hidden1);
	else if (!strcmp(type, "hidden2"))
		websWrite(wp, "%s", hidden2);
	else if (!strcmp(type, "wan_3g_mode")) {
		if (strlen(nvram_safe_get("wan_3g_mode"))) {
			websWrite(wp, nvram_safe_get("wan_3g_mode"));
		} else {
			websWrite(wp, "n.A.");
		}
	} else if (!strcmp(type, "wan_3g_signal")) {
		if (strlen(nvram_safe_get("wan_3g_signal"))) {
			websWrite(wp, nvram_safe_get("wan_3g_signal"));
		} else {
			websWrite(wp, "n.A.");
		}
	} else if (!strcmp(type, "wan_3g_status")) {
		if (strlen(nvram_safe_get("wan_3g_status"))) {
			websWrite(wp, nvram_safe_get("wan_3g_status"));
		} else {
			websWrite(wp, "n.A.");
		}
	}

	free_dns_list(dns_list);

	return;
}

void ej_show_status(webs_t wp, int argc, char_t ** argv)
{
	char *type;
	char *wan_proto = nvram_safe_get("wan_proto");
	char *submit_type = websGetVar(wp, "submit_type", NULL);
	int wan_link = 0;
	char buf[254];

#ifdef HAVE_DSL_CPE_CONTROL
	if (argc > 0) {
		if (!strcmp(argv[0], "adsl")) {
			websWrite(wp, "{dsl_iface_status::%s}\n", nvram_get("dsl_iface_status"));
			websWrite(wp, "{dsl_datarate_ds::%11.2f}\n", atof(nvram_get("dsl_datarate_ds")));
			websWrite(wp, "{dsl_datarate_us::%11.2f}\n", atof(nvram_get("dsl_datarate_us")));
			websWrite(wp, "{dsl_snr_down::%d}\n", atoi(nvram_get("dsl_snr_down")));
			websWrite(wp, "{dsl_snr_up::%d}\n", atoi(nvram_get("dsl_snr_up")));
		}
	}
#endif
	if (!strcmp(wan_proto, "pppoe")
	    || !strcmp(wan_proto, "pptp")
#ifdef HAVE_PPPOEDUAL
	    || !strcmp(wan_proto, "pppoe_dual")
#endif
#ifdef HAVE_3G
	    || !strcmp(wan_proto, "3g")
#endif
#ifdef HAVE_IPETH
	    || !strcmp(wan_proto, "iphone")
#endif
#ifdef HAVE_PPPOATM
	    || !strcmp(wan_proto, "pppoa")
#endif
#ifdef HAVE_L2TP
	    || !strcmp(wan_proto, "l2tp")
#endif
	    || !strcmp(wan_proto, "heartbeat")) {

		/*
		 * get type [ refresh | reload ] 
		 */
		type = argv[0];
		/*
		 * get ppp status , if /tmp/ppp/link exist, the connection is enabled 
		 */
		wan_link = check_wan_link(0);

		if (!strcmp(type, "init")) {

			/*
			 * press [ Connect | Disconnect ] button 
			 */
			/*
			 * set retry count 
			 */
			if (wp->gozila_action)
				retry_count = STATUS_RETRY_COUNT;	// retry 3 times

			/*
			 * set refresh time 
			 */
			// submit_type old format is "Disconnect", new format is
			// "Disconnect_pppoe" or "Disconnect_pptp" or
			// "Disconnect_heartbeat"
			if (submit_type && !strncmp(submit_type, "Disconnect", 10))	// Disconnect 
				// always 
				// 60 
				// seconds 
				// to 
				// refresh
				retry_count = -1;

			refresh_time = (retry_count <= 0) ? STATUS_REFRESH_TIME2 : STATUS_REFRESH_TIME1;

		} else if (!strcmp(type, "refresh_time")) {

			websWrite(wp, "%d", refresh_time * 1000);
		} else if (!strcmp(type, "onload")) {
			if (retry_count == 0 || (!submit_type && !wan_link && wp->gozila_action)) {	// After refresh 2 times, if the status is
				// disconnect, show Alert message.
				websWrite(wp, "ShowAlert(\"TIMEOUT\");");
				retry_count = -1;
			} else if (file_to_buf("/tmp/ppp/log", buf, sizeof(buf))) {
				websWrite(wp, "ShowAlert(\"%s\");", buf);
				retry_count = -1;
				unlink("/tmp/ppp/log");
			} else {
				websWrite(wp, "Refresh();");
			}

			if (retry_count != -1)
				retry_count--;
		}
	}
	return;
}

void ej_show_wan_domain(webs_t wp, int argc, char_t ** argv)
{
	if (nvram_invmatch("wan_domain", ""))
		tf_webWriteESCNV(wp, "wan_domain");
	else
		tf_webWriteESCNV(wp, "wan_get_domain");
	return;
}

#ifndef HAVE_MADWIFI
void ej_show_wl_mac(webs_t wp, int argc, char_t ** argv)
{
	char wifmac[32];
	char mode[32];

	sprintf(wifmac, "%s_hwaddr", nvram_safe_get("wifi_display"));
	sprintf(mode, "%s_mode", nvram_safe_get("wifi_display"));
	/*
	 * In client mode the WAN MAC is the Wireless MAC 
	 */

	if (nvram_match(mode, "sta") || nvram_match(mode, "apsta"))
		websWrite(wp, "%s", nvram_safe_get("wan_hwaddr"));
	else
		websWrite(wp, "%s", nvram_safe_get(wifmac));

	return;
}
#else
void ej_show_wl_mac(webs_t wp, int argc, char_t ** argv)
{
	char wifmac[32];

	char *ifname = nvram_safe_get("wifi_display");
	if (has_ad(ifname))
		ifname = "ath2";
	sprintf(wifmac, "%s_hwaddr", ifname);

	websWrite(wp, "%s", nvram_safe_get(wifmac));
	return;
}
#endif

/*
 * Return WAN link state 
 */
/*
 * static int ej_link(int eid, webs_t wp, int argc, char_t **argv) { char
 * *name; int s; struct ifreq ifr; struct ethtool_cmd ecmd; FILE *fp;
 * 
 * if (ejArgs(argc, argv, "%s", &name) < 1) { websError(wp, 400,
 * "Insufficient args\n"); return -1; }
 * 
 * // PPPoE connection status if (nvram_match("wan_proto", "pppoe")) { if
 * ((fp = fopen("/tmp/ppp/link", "r"))) { fclose(fp); return websWrite(wp,
 * "Connected"); } else return websWrite(wp, "Disconnected"); }
 * 
 * // Open socket to kernel if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
 * websError(wp, 400, strerror(errno)); return -1; }
 * 
 * // Check for non-zero link speed strncpy(ifr.ifr_name,
 * nvram_safe_get(name), IFNAMSIZ); ifr.ifr_data = (void *) &ecmd; ecmd.cmd = 
 * ETHTOOL_GSET; if (ioctl(s, SIOCETHTOOL, &ifr) < 0) { close(s);
 * websError(wp, 400, strerror(errno)); return -1; }
 * 
 * // Cleanup close(s);
 * 
 * if (ecmd.speed) return websWrite(wp, "Connected"); else return
 * websWrite(wp, "Disconnected"); }
 * 
 */
/*
 * void ej_show_meminfo (webs_t wp, int argc, char_t ** argv) { FILE *fp;
 * char line[254]; if ((fp = popen ("free", "r"))) { while (fgets (line,
 * sizeof (line), fp) != NULL) { websWrite (wp, "%s", line); } pclose (fp); }
 * return; } 
 */
