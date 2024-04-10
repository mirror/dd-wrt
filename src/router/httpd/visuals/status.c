/*
 * status.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#define STATUS_RETRY_COUNT 10
#define STATUS_REFRESH_TIME1 5
#define STATUS_REFRESH_TIME2 60

void show_caption_simple(webs_t wp, const char *caption);

int retry_count = -1;
int refresh_time = STATUS_REFRESH_TIME2;

EJ_VISIBLE void ej_show_status_setting(webs_t wp, int argc, char_t **argv)
{
	do_ej(METHOD_GET, NULL, "Status_Router1.asp", wp);

	return;
}

char *rfctime(const time_t *timep, char *s)
{
	struct tm tm;
	localtime_r(timep, &tm);
	strftime(s, 200, "%a, %d %b %Y %H:%M:%S", &tm); // spec for linksys
	return s;
}

/*
 * Report time in RFC-822 format 
 */
EJ_VISIBLE void ej_localtime(webs_t wp, int argc, char_t **argv)
{
	time_t tm;

	time(&tm);

	if (time(0) > (unsigned long)31536000) // 60 * 60 * 24 * 365
	{
		char t[64];

		websWrite(wp, rfctime(&tm, t));
	} else {
		if (argc && !strcmp(argv[0], "1"))
			show_caption_simple(wp, "status_router.notavail");
		else
			websWrite(wp, "%s", live_translate(wp, "status_router.notavail"));
	}
}

void nvram_status_get(webs_t wp, char *type, int trans)
{
	char wan_if_buffer[33];
	char *wan_ipaddr, *wan_netmask, *wan_gateway;
	char *status1 = "", *status2 = "", *hidden1, *hidden2, *button1 = "";
	char *wan_proto = nvram_safe_get("wan_proto");
	struct dns_lists *dns_list = NULL;
	int wan_link = check_wan_link(0);
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
		   || !strcmp(wan_proto, "android")
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

	if (!strcmp(wan_proto, "pppoe") || !strcmp(wan_proto, "pptp")
#ifdef HAVE_PPPOEDUAL
	    || !strcmp(wan_proto, "pppoe_dual")
#endif
#ifdef HAVE_3G
	    || !strcmp(wan_proto, "3g")
#endif
#ifdef HAVE_IPETH
	    || !strcmp(wan_proto, "iphone")
	    || !strcmp(wan_proto, "android")
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
		status1 = "share.disable"; // only for nonbrand
		status2 = "\"&nbsp;\"";
		button1 = "dummy";
		hidden1 = "<!--";
		hidden2 = "-->";
	}
	char buf[128];
	if (!strcmp(type, "wan_ipaddr")) {
		if (getWET() || !strcmp(wan_proto, "disabled")) {
			websWrite(wp, "%s",
				  trans == 2 ? tran_string(buf, sizeof(buf), "share.disabled") :
					       live_translate(wp, "share.disabled"));
		} else
			websWrite(wp, "%s/%d", wan_ipaddr, getmask(wan_netmask));
#ifdef HAVE_IPV6
	} else if (!strcmp(type, "wan_ipv6addr")) {
		const char *ipv6addr = NULL;
		char buf[INET6_ADDRSTRLEN];
		char strbuf[128];
		if (nvram_match("ipv6_typ", "ipv6native"))
			ipv6addr = getifaddr(buf, safe_get_wan_face(wan_if_buffer), AF_INET6, 0);
		if (nvram_match("ipv6_typ", "ipv6in4"))
			ipv6addr = getifaddr(buf, "ip6tun", AF_INET6, 0);
		if (nvram_match("ipv6_typ", "ipv6pd"))
			ipv6addr = getifaddr(buf, nvram_safe_get("lan_ifname"), AF_INET6, 0);
		if (!ipv6addr)
			ipv6addr = getifaddr(buf, safe_get_wan_face(wan_if_buffer), AF_INET6,
					     0); // try wan if all other fails
		if (!ipv6addr || getWET() || !strcmp(wan_proto, "disabled")) {
			websWrite(wp, "%s",
				  trans == 2 ? tran_string(strbuf, sizeof(strbuf), "share.disabled") :
					       live_translate(wp, "share.disabled"));
		} else {
			websWrite(wp, "%s", ipv6addr);
		}
#endif
	} else if (!strcmp(type, "wan_netmask"))
		websWrite(wp, "%s", wan_netmask);
	else if (!strcmp(type, "wan_gateway"))
		websWrite(wp, "%s", wan_gateway);
	else if (!strncmp(type, "wan_dns", 7)) {
		dns_list = get_dns_list(1);
		int index = atoi(type + 7);
		struct dns_entry *entry = get_dns_entry(dns_list, index);
		if (entry)
			websWrite(wp, "%s", entry->ip);
		free_dns_list(dns_list);
	} else if (!strcmp(type, "status1"))
		websWrite(wp, "%s",
			  trans == 3 ? status1 :
			  trans == 2 ? tran_string(buf, sizeof(buf), status1) :
				       live_translate(wp, status1));
	else if (!strcmp(type, "status2"))
		websWrite(wp, "%s",
			  trans == 3 ? status2 :
			  trans == 2 ? tran_string(buf, sizeof(buf), status2) :
				       live_translate(wp, status2));
	else if (!strcmp(type, "button1")) {
		if (trans)
			websWrite(wp, "%s",
				  trans == 3 ? button1 :
				  trans == 2 ? tran_string(buf, sizeof(buf), button1) :
					       live_translate(wp, button1));
		else
			websWrite(wp, "%s", button1);
	} else if (!strcmp(type, "hidden1"))
		websWrite(wp, "%s", hidden1);
	else if (!strcmp(type, "hidden2"))
		websWrite(wp, "%s", hidden2);
	else if (!strcmp(type, "wan_3g_mode")) {
		if (*(nvram_safe_get("wan_3g_mode"))) {
			websWrite(wp, nvram_safe_get("wan_3g_mode"));
		} else {
			websWrite(wp, "n.A.");
		}
	} else if (!strcmp(type, "wan_3g_signal")) {
		if (*(nvram_safe_get("wan_3g_signal"))) {
			websWrite(wp, nvram_safe_get("wan_3g_signal"));
		} else {
			websWrite(wp, "n.A.");
		}
	} else if (!strcmp(type, "wan_3g_status")) {
		if (*(nvram_safe_get("wan_3g_status"))) {
			websWrite(wp, nvram_safe_get("wan_3g_status"));
		} else {
			websWrite(wp, "n.A.");
		}
	}

	return;
}

EJ_VISIBLE void ej_nvram_status_get(webs_t wp, int argc, char_t **argv)
{
	char *type;
	int trans = 0;
	type = argv[0];
	if (argc > 1)
		trans = atoi(argv[1]);
	nvram_status_get(wp, type, trans);
}

EJ_VISIBLE void ej_show_dnslist(webs_t wp, int argc, char_t **argv)
{
	int i = 0;
	struct dns_lists *dns_list = NULL;
	struct dns_entry *entry;
	dns_list = get_dns_list(1);
	int ipv4count = 0;
	int ipv6count = 0;
	while ((entry = get_dns_entry(dns_list, i)) != NULL) {
		websWrite(wp, "<div class=\"setting\">\n");
		char buf[64];
		if (entry->type && !entry->ipv6)
			websWrite(wp, "<div class=\"label\">IPv4 DNS %d (%s)</div>\n", ipv4count++,
				  tran_string(buf, sizeof(buf), "share.sttic"));
		if (entry->type && entry->ipv6)
			websWrite(wp, "<div class=\"label\">IPv6 DNS %d (%s)</div>\n", ipv6count++,
				  tran_string(buf, sizeof(buf), "share.sttic"));
		if (!entry->type && !entry->ipv6)
			websWrite(wp, "<div class=\"label\">IPv4 DNS %d</div>\n", ipv4count++);
		if (!entry->type && entry->ipv6)
			websWrite(wp, "<div class=\"label\">IPv6 DNS %d</div>\n", ipv6count++);
		websWrite(wp, "<span id=\"wan_dns%d\">%s</span>&nbsp;\n", i, entry->ip);
		websWrite(wp, "</div>\n");
		i++;
	}
	free_dns_list(dns_list);
}

EJ_VISIBLE void ej_show_live_dnslist(webs_t wp, int argc, char_t **argv)
{
	int i = 0;
	struct dns_entry *entry;
	struct dns_lists *dns_list = NULL;
	dns_list = get_dns_list(1);
	while ((entry = get_dns_entry(dns_list, i)) != NULL) {
		websWrite(wp, "{wan_dns%d::%s}\n", i, entry->ip);
		i++;
	}
	free_dns_list(dns_list);
}

EJ_VISIBLE void ej_show_status(webs_t wp, int argc, char_t **argv)
{
	char *type;
	char *wan_proto = nvram_safe_get("wan_proto");
	char *submit_type = websGetVar(wp, "submit_type", NULL);
	int wan_link = 0;
	char buf[254];

#ifdef HAVE_DSL_CPE_CONTROL
	if (argc > 0) {
		if (!strcmp(argv[0], "adsl")) {
			websWrite(wp, "{dsl_iface_status::%s}\n", nvram_safe_get("dsl_iface_status"));
			websWrite(wp, "{dsl_datarate_ds::%11.2f}\n", atof(nvram_safe_get("dsl_datarate_ds")));
			websWrite(wp, "{dsl_datarate_us::%11.2f}\n", atof(nvram_safe_get("dsl_datarate_us")));
			websWrite(wp, "{dsl_snr_down::%d}\n", atoi(nvram_safe_get("dsl_snr_down")));
			websWrite(wp, "{dsl_snr_up::%d}\n", atoi(nvram_safe_get("dsl_snr_up")));
		}
	}
#endif
	if (!strcmp(wan_proto, "pppoe") || !strcmp(wan_proto, "pptp")
#ifdef HAVE_PPPOEDUAL
	    || !strcmp(wan_proto, "pppoe_dual")
#endif
#ifdef HAVE_3G
	    || !strcmp(wan_proto, "3g")
#endif
#ifdef HAVE_IPETH
	    || !strcmp(wan_proto, "iphone")
	    || !strcmp(wan_proto, "android")
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
				retry_count = STATUS_RETRY_COUNT; // retry 3 times

			/*
			 * set refresh time 
			 */
			// submit_type old format is "Disconnect", new format is
			// "Disconnect_pppoe" or "Disconnect_pptp" or
			// "Disconnect_heartbeat"
			if (submit_type && !strncmp(submit_type, "Disconnect",
						    10)) // Disconnect
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
			if (retry_count == 0 ||
			    (!submit_type && !wan_link && wp->gozila_action)) { // After refresh 2 times, if the status is
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

EJ_VISIBLE void ej_show_wan_domain(webs_t wp, int argc, char_t **argv)
{
	if (nvram_invmatch("wan_domain", ""))
		tf_webWriteESCNV(wp, "wan_domain");
	else
		tf_webWriteESCNV(wp, "wan_get_domain");
	return;
}

#ifndef HAVE_MADWIFI
EJ_VISIBLE void ej_show_wl_mac(webs_t wp, int argc, char_t **argv)
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
EJ_VISIBLE void ej_show_wl_mac(webs_t wp, int argc, char_t **argv)
{
	char wifmac[32];

	char *ifname = nvram_safe_get("wifi_display");
	if (has_ad(ifname))
		ifname = "wlan2";
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
 * EJ_VISIBLE void ej_show_meminfo (webs_t wp, int argc, char_t ** argv) { FILE *fp;
 * char line[254]; if ((fp = popen ("free", "r"))) { while (fgets (line,
 * sizeof (line), fp) != NULL) { websWrite (wp, "%s", line); } pclose (fp); }
 * return; } 
 */
