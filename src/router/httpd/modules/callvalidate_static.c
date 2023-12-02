/*
 * DD-WRT callvalidate_static.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License
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
#ifdef WEBS
#include <uemf.h>
#include <ej.h>
#else				/* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif				/* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <dd_defs.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>
#include <airbag.h>

#include <dlfcn.h>
#include <stdio.h>
// #include <shutils.h>
void validate_auth_mode(webs_t wp, char *value, struct variable *v);
void validate_blocked_service(webs_t wp, char *value, struct variable *v);
void validate_catchall(webs_t wp, char *value, struct variable *v);
void validate_chaps(webs_t wp, char *value, struct variable *v);
void validate_choice(webs_t wp, char *value, struct variable *v);
void validate_dns(webs_t wp, char *value, struct variable *v);
void validate_dynamic_route(webs_t wp, char *value, struct variable *v);
void validate_filter_dport_grp(webs_t wp, char *value, struct variable *v);
void validate_filter_ip_grp(webs_t wp, char *value, struct variable *v);
void validate_filter_mac_grp(webs_t wp, char *value, struct variable *v);
void validate_filter_policy(webs_t wp, char *value, struct variable *v);
void validate_filter_port(webs_t wp, char *value, struct variable *v);
void validate_filter_web(webs_t wp, char *value, struct variable *v);
void validate_forward_proto(webs_t wp, char *value, struct variable *v);
void validate_forward_spec(webs_t wp, char *value, struct variable *v);
void validate_forward_ip(webs_t wp, char *value, struct variable *v);
void validate_hwaddr(webs_t wp, char *value, struct variable *v);
void validate_hwaddrs(webs_t wp, char *value, struct variable *v);
void validate_ipaddr(webs_t wp, char *value, struct variable *v);
void validate_ipaddrs(webs_t wp, char *value, struct variable *v);
void validate_iradius(webs_t wp, char *value, struct variable *v);
void validate_lan_ipaddr(webs_t wp, char *value, struct variable *v);
void validate_merge_dhcpstart(webs_t wp, char *value, struct variable *v);
void validate_merge_ipaddrs(webs_t wp, char *value, struct variable *v);
void validate_merge_mac(webs_t wp, char *value, struct variable *v);
void validate_merge_netmask(webs_t wp, char *value, struct variable *v);
void validate_name(webs_t wp, char *value, struct variable *v);
void validate_netmask(webs_t wp, char *value, struct variable *v);
void validate_noack(webs_t wp, char *value, struct variable *v);
void validate_password(webs_t wp, char *value, struct variable *v);
void validate_password2(webs_t wp, char *value, struct variable *v);
void validate_portsetup(webs_t wp, char *value, struct variable *v);
void validate_avahi(webs_t wp, char *value, struct variable *v);
void validate_port_trigger(webs_t wp, char *value, struct variable *v);
void validate_range(webs_t wp, char *value, struct variable *v);
void validate_reboot(webs_t wp, char *value, struct variable *v);
void validate_remote_ip(webs_t wp, char *value, struct variable *v);
void validate_staticleases(webs_t wp, char *value, struct variable *v);
void validate_openvpnuserpass(webs_t wp, char *value, struct variable *v);
void validate_static_route(webs_t wp, char *value, struct variable *v);
void validate_pbr_rule(webs_t wp, char *value, struct variable *v);
void validate_statics(webs_t wp, char *value, struct variable *v);
void validate_userlist(webs_t wp, char *value, struct variable *v);
void validate_wan_ipaddr(webs_t wp, char *value, struct variable *v);
void validate_wds(webs_t wp, char *value, struct variable *v);
void validate_wl_auth(webs_t wp, char *value, struct variable *v);
void validate_wl_gmode(webs_t wp, char *value, struct variable *v);
void validate_wl_hwaddrs(webs_t wp, char *value, struct variable *v);
void validate_wl_key(webs_t wp, char *value, struct variable *v);
void validate_wl_net_mode(webs_t wp, char *value, struct variable *v);
void validate_wl_wep(webs_t wp, char *value, struct variable *v);
void validate_wl_wep_key(webs_t wp, char *value, struct variable *v);
void validate_wl_wme_params(webs_t wp, char *value, struct variable *v);
void validate_wl_wme_tx_params(webs_t wp, char *value, struct variable *v);
void validate_wpa_psk(webs_t wp, char *value, struct variable *v);

#include "ej_proto.h"

struct callmap {
	char *name;
	void *func;
};

static struct callmap ej_map[] = {
#include "ejtable.h"
};

static struct callmap validate_map[] = {
#ifdef HAVE_MILKFISH
	{ "validate_subscribers", &validate_subscribers },
	{ "validate_aliases", &validate_aliases },
#endif
	{ "validate_dhcp_check", &validate_dhcp_check },
	{ "validate_auth_mode", &validate_auth_mode },
	{ "validate_blocked_service", &validate_blocked_service },
	{ "validate_catchall", &validate_catchall },
#ifdef HAVE_PPPOESERVER
	{ "validate_chaps", &validate_chaps },
#endif
	{ "validate_choice", &validate_choice },
	{ "validate_dns", &validate_dns },
	{ "validate_dynamic_route", &validate_dynamic_route },
	{ "validate_filter_dport_grp", &validate_filter_dport_grp },
	{ "validate_filter_ip_grp", &validate_filter_ip_grp },
	{ "validate_filter_mac_grp", &validate_filter_mac_grp },
	{ "validate_filter_policy", &validate_filter_policy },
	{ "validate_filter_port", &validate_filter_port },
	{ "validate_filter_web", &validate_filter_web },
	{ "validate_forward_proto", &validate_forward_proto },
	{ "validate_forward_spec", &validate_forward_spec },
	{ "validate_forward_ip", &validate_forward_ip },
	{ "validate_hwaddr", &validate_hwaddr },
	{ "validate_hwaddrs", &validate_hwaddrs },
	{ "validate_ipaddr", &validate_ipaddr },
	{ "validate_ipaddrs", &validate_ipaddrs },
#ifdef HAVE_RADLOCAL
	{ "validate_iradius", &validate_iradius },
#endif
	{ "validate_lan_ipaddr", &validate_lan_ipaddr },
	{ "validate_merge_dhcpstart", &validate_merge_dhcpstart },
	{ "validate_merge_ipaddrs", &validate_merge_ipaddrs },
	{ "validate_merge_mac", &validate_merge_mac },
	{ "validate_merge_netmask", &validate_merge_netmask },
	{ "validate_name", &validate_name },
	{ "validate_netmask", &validate_netmask },
	{ "validate_noack", &validate_noack },
	{ "validate_password", &validate_password },
	{ "validate_password2", &validate_password2 },
#ifdef HAVE_PORTSETUP
	{ "validate_portsetup", &validate_portsetup },
#endif
#ifdef HAVE_MDNS
	{ "validate_avahi", &validate_avahi },
#endif
#ifdef HAVE_OPENVPN
	{ "validate_openvpnuserpass", &validate_openvpnuserpass },
#endif

	{ "validate_port_trigger", &validate_port_trigger },
	{ "validate_range", &validate_range },
	{ "validate_reboot", &validate_reboot },
	{ "validate_remote_ip", &validate_remote_ip },
	{ "validate_staticleases", &validate_staticleases },
	{ "validate_static_route", &validate_static_route },
#ifndef HAVE_MICRO
	{ "validate_pbr_rule", &validate_pbr_rule },
#endif
	{ "validate_statics", &validate_statics },
#ifdef HAVE_CHILLI
#ifdef HAVE_CHILLILOCAL
	{ "validate_userlist", &validate_userlist },
#endif
#endif
	{ "validate_wan_ipaddr", &validate_wan_ipaddr },
	{ "validate_wds", &validate_wds },
	{ "validate_wl_auth", &validate_wl_auth },
	{ "validate_wl_gmode", &validate_wl_gmode },
	{ "validate_wl_hwaddrs", &validate_wl_hwaddrs },
	{ "validate_wl_key", &validate_wl_key },
	{ "validate_wl_net_mode", &validate_wl_net_mode },
	{ "validate_wl_wep", &validate_wl_wep },
	{ "validate_wl_wep_key", &validate_wl_wep_key },
	{ "validate_wl_wme_params", &validate_wl_wme_params },
	{ "validate_wl_wme_tx_params", &validate_wl_wme_tx_params },
	{ "validate_wpa_psk", &validate_wpa_psk },
};

static struct callmap gozila_map[] = {
	{ "wan_proto", &wan_proto },
	{ "dhcpfwd", &dhcpfwd },
#ifdef HAVE_IAS
	{ "ias_save_admincard", &ias_save_admincard },
#endif
#ifdef HAVE_CCONTROL
	{ "execute", &execute },
#endif
	{ "clone_mac", &clone_mac },
	{ "delete_leases", &delete_leases },
	{ "static_leases", &static_leases },
#ifdef HAVE_PPTPD
	{ "delete_pptp", &delete_pptp },
#endif
#ifdef HAVE_OPENVPN
	{ "import_vpntunnel", &import_vpntunnel },
	{ "userpass_add", &userpass_add },
	{ "userpass_del", &userpass_del },
#endif
#ifdef HAVE_SSHD
	{ "ssh_downloadkey", &ssh_downloadkey },
#endif
	{ "save_wifi", &save_wifi },
	{ "save_wifi", &save_wifi },
	{ "dhcp_release", &dhcp_release },
	{ "dhcp_renew", &dhcp_renew },
	{ "stop_ppp", &stop_ppp },
	{ "ttraff_erase", &ttraff_erase },
	{ "save_policy", &save_policy },
	{ "sel_filter", &sel_filter },
	{ "single_delete_policy", &single_delete_policy },
	{ "summary_delete_policy", &summary_delete_policy },
	{ "delete_static_route", &delete_static_route },
#ifndef HAVE_MICRO
	{ "delete_pbr_rule", &delete_pbr_rule },
#endif
	{ "generate_wep_key", &generate_wep_key },
	{ "set_security", &set_security },
	{ "security_save", &security_save },
#ifdef HAVE_80211R
	{ "roaming_save", &security_save },
#endif
	{ "add_active_mac", &add_active_mac },
	{ "ping_wol", &ping_wol },
	{ "save_wds", &save_wds },
	{ "save_wireless_advanced", &save_wireless_advanced },
	{ "save_startup", &save_startup },
	{ "save_shutdown", &save_shutdown },
	{ "save_firewall", &save_firewall },
	{ "save_custom", &save_custom },
	{ "save_usb", &save_usb },
	{ "qos_add_svc", &qos_add_svc },
	{ "qos_add_ip", &qos_add_ip },
	{ "qos_add_mac", &qos_add_mac },
	{ "qos_add_dev", &qos_add_dev },
	{ "qos_save", &qos_save },
	{ "qosips_del", &qosips_del },
	{ "qossvcs_del", &qossvcs_del },
	{ "qosmacs_del", &qosmacs_del },
	{ "qosdevs_del", &qosdevs_del },
	{ "forward_add", &forward_add },
	{ "forward_del", &forward_del },
	{ "filter_add", &filter_add },
	{ "filter_remove", &filter_remove },
	{ "add_vifs", &add_vifs },
	{ "remove_vifs", &remove_vifs },
	{ "copy_if", &copy_if },
	{ "paste_if", &paste_if },
#ifdef HAVE_FREERADIUS
	{ "radius_generate_certificate", &radius_generate_certificate },
	{ "add_radius_user", &add_radius_user },
	{ "del_radius_user", &del_radius_user },
	{ "add_radius_client", &add_radius_client },
	{ "del_radius_client", &del_radius_client },
	{ "save_radius_user", &save_radius_user },
#endif
#ifdef HAVE_POKER
	{ "add_poker_user", &add_poker_user },
	{ "del_poker_user", &del_poker_user },
	{ "save_poker_user", &save_poker_user },
	{ "poker_loaduser", &poker_loaduser },
	{ "poker_checkout", &poker_checkout },
	{ "poker_buy", &poker_buy },
	{ "poker_credit", &poker_credit },
	{ "poker_back", &poker_back },
#endif
#ifdef HAVE_BONDING
	{ "add_bond", &add_bond },
	{ "del_bond", &del_bond },
#endif
#ifdef HAVE_OLSRD
	{ "add_olsrd", &add_olsrd },
	{ "del_olsrd", &del_olsrd },
#endif
#ifdef HAVE_VLANTAGGING
	{ "add_vlan", &add_vlan },
	{ "add_bridge", &add_bridge },
	{ "add_bridgeif", &add_bridgeif },
	{ "del_vlan", &del_vlan },
	{ "del_bridge", &del_bridge },
	{ "del_bridgeif", &del_bridgeif },
	{ "save_networking", &save_networking },
	{ "add_mdhcp", &add_mdhcp },
	{ "del_mdhcp", &del_mdhcp },
#endif
#ifdef HAVE_IPVS
	{ "add_ipvs", &add_ipvs },
	{ "del_ipvs", &del_ipvs },
	{ "add_ipvstarget", &add_ipvstarget },
	{ "del_ipvstarget", &del_ipvstarget },
#endif
	{ "wireless_save", &wireless_save },
#ifdef HAVE_WIVIZ
	{ "set_wiviz", &set_wiviz },
#endif
#ifdef HAVE_REGISTER
	{ "reg_validate", &reg_validate },
#endif
	{ "changepass", &changepass },
#ifdef HAVE_SUPERCHANNEL
	{ "superchannel_validate", &superchannel_validate },
#endif
	{ "lease_add", &lease_add },
	{ "lease_del", &lease_del },
#ifdef HAVE_PPPOESERVER
	{ "chap_user_add", &chap_user_add },
	{ "chap_user_remove", &chap_user_remove },
#endif
#ifdef HAVE_CHILLI
#ifdef HAVE_CHILLILOCAL
	{ "user_add", &user_add },
	{ "user_remove", &user_remove },
#endif
#endif
#ifdef HAVE_RADLOCAL
	{ "raduser_add", &raduser_add },
#endif
#ifdef HAVE_EOP_TUNNEL
#ifdef HAVE_WIREGUARD
	{ "gen_wg_key", &gen_wg_key },
	{ "gen_wg_psk", &gen_wg_psk },
	{ "gen_wg_client", &gen_wg_client },
	{ "del_wg_client", &del_wg_client },
	{ "add_peer", &add_peer },
	{ "del_peer", &del_peer },
	{ "import_tunnel", &import_tunnel },
#endif
	{ "add_tunnel", &add_tunnel },
	{ "del_tunnel", &del_tunnel },
	{ "tunnel_save", &tunnel_save },
#endif
	{ "forwardspec_add", &forwardspec_add },
	{ "forwardspec_del", &forwardspec_del },
	{ "forwardip_add", &forwardip_add },
	{ "forwardip_del", &forwardip_del },
	{ "trigger_add", &trigger_add },
	{ "trigger_del", &trigger_del },
	{ "save_services_port", &save_services_port },
	{ "save_services_port", &save_services_port },
	{ "diag_ping_start", &diag_ping_start },
	{ "diag_ping_stop", &diag_ping_stop },
	{ "diag_ping_clear", &diag_ping_clear },
#ifdef HAVE_MILKFISH
	{ "milkfish_user_add", &milkfish_user_add },
	{ "milkfish_user_remove", &milkfish_user_remove },
	{ "milkfish_alias_add", &milkfish_alias_add },
	{ "milkfish_alias_remove", &milkfish_alias_remove },
	{ "milkfish_sip_message", &milkfish_sip_message },
#endif
#ifdef HAVE_STATUS_GPIO
	{ "gpios_save", &gpios_save },
#endif
#ifdef HAVE_BUFFALO
	{ "setupassistant_save", &setupassistant_save },
	{ "generate_wep_key", &generate_wep_key },
	{ "set_security", &set_security },
	{ "security_save", &security_save },
	{ "get_airstation_upgrades", &get_airstation_upgrades },
#ifdef HAVE_IAS
	{ "internetatstart", &internetatstart },
	{ "intatstart_ajax", &intatstart_ajax },
#endif
#ifdef HAVE_SPOTPASS
	{ "nintendo_save", &nintendo_save },
#endif
#endif
	{ "wireless_join", &wireless_join },
#ifdef HAVE_NAS_SERVER
	{ "nassrv_save", &nassrv_save },
#endif
#ifdef HAVE_RAID
	{ "add_raid", &add_raid },
	{ "del_raid", &del_raid },
	{ "add_raid_member", &add_raid_member },
	{ "del_raid_member", &del_raid_member },
	{ "format_raid", &format_raid },
	{ "format_drive", &format_drive },
	{ "raid_save", &raid_save },
#ifdef HAVE_ZFS
	{ "zfs_scrub", &zfs_scrub },
#endif
#endif
#if defined(HAVE_WPS) || defined(HAVE_AOSS)
	{ "aoss_save", &aoss_save },
	{ "aoss_start", &aoss_start },
#ifdef HAVE_WPS
	{ "wps_register", &wps_register },
	{ "wps_ap_register", &wps_ap_register },
	{ "wps_forcerelease", &wps_forcerelease },
	{ "wps_configure", &wps_configure },
#endif
#endif
#ifdef HAVE_SYSCTL_EDIT
	{ "sysctl_save", &sysctl_save },
#endif
	{ "ddns_save_value", &ddns_save_value },
	{ "port_vlan_table_save", &port_vlan_table_save },
	{ "portvlan_add", &portvlan_add },
	{ "portvlan_remove", &portvlan_remove },
	{ "save_wireless_advanced", &save_wireless_advanced },
	{ "save_wireless_advanced", &save_wireless_advanced },
	{ "save_wireless_advanced", &save_wireless_advanced },
	{ "save_macmode", &save_macmode },
#ifdef HAVE_SYSCTL_EDIT
	{ "sysctl_save", &sysctl_save },
#endif
	{ "hotspot_save", &hotspot_save },
#ifdef HAVE_UPNP
	{ "tf_upnp", &tf_upnp },
#endif
};

#define cprintf(fmt, args...)

#ifndef cprintf
#define cprintf(fmt, args...) do { \
	FILE *fp = fopen("/dev/console", "w"); \
	if (fp) { \
		fprintf(fp,"%s (%d):%s ",__FILE__,__LINE__,__func__); \
		fprintf(fp, fmt, ## args); \
		fclose(fp); \
	} \
} while (0)
#endif
size_t websWrite(webs_t wp, char *fmt, ...);
size_t vwebsWrite(webs_t wp, char *fmt, va_list args);
char *websGetVar(webs_t wp, char *var, char *d)
{
	return get_cgi(wp, var) ? : d;
}

char *websGetSaneVar(webs_t wp, char *var, char *d)
{
	char *sanevar = websGetVar(wp, var, d);
	if (d && sanevar && !*var)
	    sanevar = d;
	return sanevar;
}

int websGetVari(webs_t wp, char *var, int d)
{
	char *res = get_cgi(wp, var);
	return res ? atoi(res) : d;
}

char *GOZILA_GET(webs_t wp, char *name)
{
	if (!name)
		return NULL;
	if (!wp)
		return nvram_safe_get(name);
	return wp->gozila_action ? websGetVar(wp, name, NULL) : nvram_safe_get(name);
}

char *live_translate(webs_t wp, const char *tran);
void validate_cgi(webs_t wp);

static void start_gozila(char *name, webs_t wp)
{
	int (*fptr)(webs_t wp) = NULL;
	int i;
	for (i = 0; i < sizeof(gozila_map) / sizeof(gozila_map[0]); i++) {
		if (gozila_map[i].name[0] == name[0] && !strcmp(gozila_map[i].name, name)) {
			fptr = (int (*)(webs_t wp))gozila_map[i].func;
			break;
		}
	}
	dd_logdebug("httpd", "start gozila %s\n", name);
	if (fptr)
		(*fptr) (wp);
	else
		dd_logdebug("httpd", "function %s not found \n", name);
}

static int start_validator(char *name, webs_t wp, char *value, struct variable *v)
{

	int (*fptr)(webs_t wp, char *value, struct variable * v) = NULL;

	int i;
	int ret;
	for (i = 0; i < sizeof(validate_map) / sizeof(validate_map[0]); i++) {
		if (validate_map[i].name[0] == name[0] && !strcmp(validate_map[i].name, name)) {
			fptr = (int (*)(webs_t wp, char *value, struct variable * v))validate_map[i].func;
			break;
		}
	}

	dd_logdebug("httpd", "start validator %s\n", name);
	if (fptr)
		ret = (*fptr) (wp, value, v);
	else
		dd_logdebug("httpd", "function %s not found \n", name);

	cprintf("start_sevice done()\n");
	return ret;
}

static void *start_validator_nofree(char *name, void *handle, webs_t wp, char *value, struct variable *v)
{
	start_validator(name, wp, value, v);
	return handle;
}

static void *call_ej(char *name, void *handle, webs_t wp, int argc, char_t ** argv)
{
	struct timeval before, after, r;

	airbag_setpostinfo(name);
	if (nvram_matchi("httpd_debug", 1)) {
		fprintf(stderr, "call_ej %s", name);
		int i = 0;
		for (i = 0; i < argc; i++)
			fprintf(stderr, " %s", argv[i]);
	}
	void (*fptr)(webs_t wp, int argc, char_t ** argv) = NULL;
	int i;
	for (i = 0; i < sizeof(ej_map) / sizeof(ej_map[0]); i++) {
		if (!strcmp(ej_map[i].name, name)) {
			fptr = (void (*)(webs_t wp, int argc, char_t ** argv))ej_map[i].func;
			break;
		}
	}
	cprintf("found. pointer is %p\n", fptr);
	{
		memdebug_enter();
		if (fptr) {
			gettimeofday(&before, NULL);
			(*fptr) (wp, argc, argv);
			gettimeofday(&after, NULL);
			timersub(&after, &before, &r);
			dd_logdebug("httpd", " %s duration %ld.%06ld\n", name, (long int)r.tv_sec, (long int)r.tv_usec);

		} else {
			dd_logdebug("httpd", " function %s not found (%s)\n", name, "unknown");
		}
	}
	return handle;

}
