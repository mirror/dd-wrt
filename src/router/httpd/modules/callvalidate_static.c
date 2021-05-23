/*
 * DD-WRT servicemanager.c
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
void validate_port_trigger(webs_t wp, char *value, struct variable *v);
void validate_range(webs_t wp, char *value, struct variable *v);
void validate_reboot(webs_t wp, char *value, struct variable *v);
void validate_remote_ip(webs_t wp, char *value, struct variable *v);
void validate_staticleases(webs_t wp, char *value, struct variable *v);
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

#if 0
static struct callmap ej_map[] = {
	{ "active_wds", &ej_active_wds },
	{ "active_wireless", &ej_active_wireless },
	{ "assoc_count", &ej_assoc_count },
	{ "atmsettings", &ej_atmsettings },
	{ "calcendip", &ej_calcendip },
#ifdef HAVE_ATH9K
	{ "channel_survey", &ej_channel_survey },
#endif
	{ "compile_date", &ej_compile_date },
	{ "compile_time", &ej_compile_time },
	{ "dhcpenabled", &ej_dhcpenabled },
	{ "dhcp_remaining_time", &ej_dhcp_remaining_time },
#ifdef HAVE_MINIDLNA
	{ "dlna_sharepaths", &ej_dlna_sharepaths },
#endif
	{ "do_hpagehead", &ej_do_hpagehead },
	{ "do_menu", &ej_do_menu },
	{ "do_pagehead", &ej_do_pagehead },
	{ "dumparptable", &ej_dumparptable },
#ifdef HAVE_ATH9K
	{ "dump_channel_survey", &ej_dump_channel_survey },
#endif
	{ "dumpip_conntrack", &ej_dumpip_conntrack },
	{ "dumpleases", &ej_dumpleases },
	{ "dumplog", &ej_dumplog },
	{ "dumpmeminfo", &ej_dumpmeminfo },
	{ "dump_ping_log", &ej_dump_ping_log },
#ifdef HAVE_PPPOESERVER
	{ "dumppppoe", &ej_dumppppoe },
#endif
#ifdef HAVE_PPTPD
	{ "dumppptp", &ej_dumppptp },
#endif
	{ "dump_route_table", &ej_dump_route_table },
	{ "dump_site_survey", &ej_dump_site_survey },
#ifdef HAVE_WIVIZ
	{ "dump_wiviz_data", &ej_dump_wiviz_data },
#endif
	{ "else_selmatch", &ej_else_selmatch },
	{ "filter_dport_get", &ej_filter_dport_get },
	{ "filter_getpacketcount", &ej_filter_getpacketcount },
	{ "filter_init", &ej_filter_init },
	{ "filter_ip_get", &ej_filter_ip_get },
	{ "filter_mac_get", &ej_filter_mac_get },
	{ "filter_policy_get", &ej_filter_policy_get },
	{ "filter_policy_select", &ej_filter_policy_select },
	{ "filter_port_get", &ej_filter_port_get },
	{ "filter_port_services_get", &ej_filter_port_services_get },
	{ "filter_summary_show", &ej_filter_summary_show },
	{ "filter_tod_get", &ej_filter_tod_get },
	{ "filter_web_get", &ej_filter_web_get },
	{ "gen_filters", &ej_gen_filters },
	{ "gen_init_timer", &ej_gen_init_timer },
	{ "gen_timer_compute", &ej_gen_timer_compute },
	{ "gen_timer_fields", &ej_gen_timer_fields },
#ifdef HAVE_ATH9K
	{ "get_active", &ej_get_active },
#endif
	{ "get_backup_name", &ej_get_backup_name },
	{ "getboottime", &ej_getboottime },
	{ "get_br1_ip", &ej_get_br1_ip },
	{ "get_br1_netmask", &ej_get_br1_netmask },
#ifdef HAVE_ATH9K
	{ "get_busy", &ej_get_busy },
#endif
	{ "getchipset", &ej_getchipset },
	{ "get_clkfreq", &ej_get_clkfreq },
	{ "get_clone_mac", &ej_get_clone_mac },
	{ "get_clone_wmac", &ej_get_clone_wmac },
#ifdef HAVE_CPUTEMP
	{ "get_cputemp", &ej_get_cputemp },
#endif
	{ "get_curchannel", &ej_get_curchannel },
	{ "get_currate", &ej_get_currate },
	{ "getdefaultindex", &ej_getdefaultindex },
	{ "get_dns_ip", &ej_get_dns_ip },
	{ "getencryptionstatus", &ej_getencryptionstatus },
	{ "get_firmware_svnrev", &ej_get_firmware_svnrev },
	{ "get_firmware_title", &ej_get_firmware_title },
	{ "get_firmware_version", &ej_get_firmware_version },
	{ "gethostnamebyip", &ej_gethostnamebyip },
	{ "get_http_prefix", &ej_get_http_prefix },
	{ "get_model_name", &ej_get_model_name },
	{ "get_mtu", &ej_get_mtu },
	{ "getnumfilters", &ej_getnumfilters },
	{ "get_qosdevs", &ej_get_qosdevs },
	{ "get_qosips", &ej_get_qosips },
	{ "get_qosmacs", &ej_get_qosmacs },
	{ "get_qospkts", &ej_get_qospkts },
	{ "get_qossvcs", &ej_get_qossvcs },
#ifdef HAVE_ATH9K
	{ "get_quality", &ej_get_quality },
#endif
	{ "get_radio_state", &ej_get_radio_state },
	{ "get_radio_statejs", &ej_get_radio_statejs },
	{ "get_service_state", &ej_get_service_state },
	{ "getsetuppage", &ej_getsetuppage },
	{ "get_single_ip", &ej_get_single_ip },
	{ "get_single_mac", &ej_get_single_mac },
	{ "get_single_nm", &ej_get_single_nm },
	{ "get_status_curchannel", &ej_get_status_curchannel },
	{ "get_syskernel", &ej_get_syskernel },
	{ "get_sysmodel", &ej_get_sysmodel },
	{ "get_totaltraff", &ej_get_totaltraff },
	{ "get_txpower", &ej_get_txpower },
	{ "get_uptime", &ej_get_uptime },
	{ "get_wan_uptime", &ej_get_wan_uptime },
	{ "get_wds_gw", &ej_get_wds_gw },
	{ "get_wds_ip", &ej_get_wds_ip },
	{ "get_wds_mac", &ej_get_wds_mac },
	{ "get_wds_netmask", &ej_get_wds_netmask },
	{ "get_wdsp2p", &ej_get_wdsp2p },
	{ "get_web_page_name", &ej_get_web_page_name },
	{ "getWET", &ej_getWET },
	{ "getwirelessmode", &ej_getwirelessmode },
	{ "getwirelessnetmode", &ej_getwirelessnetmode },
	{ "getwirelessssid", &ej_getwirelessssid },
	{ "getwirelessstatus", &ej_getwirelessstatus },
	{ "get_wl_active_mac", &ej_get_wl_active_mac },
	{ "get_wl_domain", &ej_get_wl_domain },
	{ "get_wl_max_channel", &ej_get_wl_max_channel },
	{ "get_wl_value", &ej_get_wl_value },
	{ "has_routing", &ej_has_routing },
	{ "ifdef", &ej_ifdef },
	{ "ifndef", &ej_ifndef },
#ifdef HAVE_WPA_SUPPLICANT
#ifndef HAVE_MICRO
	{ "init_80211x_layers", &ej_init_80211x_layers },
#endif
#endif
	{ "ip_conntrack_table", &ej_ip_conntrack_table },
	{ "list_mac_layers", &ej_list_mac_layers },
	{ "localtime", &ej_localtime },
	{ "make_time_list", &ej_make_time_list },
	{ "no_cache", &ej_no_cache },
	{ "nvc", &ej_nvc },
	{ "nvem", &ej_nvem },
	{ "nvg", &ej_nvg },
	{ "nvim", &ej_nvim },
	{ "nvm", &ej_nvm },
	{ "nvram_checked", &ej_nvram_checked },
	{ "nvram_checked_js", &ej_nvram_checked_js },
	{ "nvram_else_match", &ej_nvram_else_match },
	{ "nvram_else_selmatch", &ej_nvram_else_selmatch },
	{ "nvram_get", &ej_nvram_get },
	{ "nvram_gozila_get", &ej_nvram_gozila_get },
	{ "nvram_invmatch", &ej_nvram_invmatch },
	{ "nvram_list", &ej_nvram_list },
	{ "nvram_mac_get", &ej_nvram_mac_get },
	{ "nvram_match", &ej_nvram_match },
	{ "nvram_real_get", &ej_nvram_real_get },
	{ "nvram_selected", &ej_nvram_selected },
	{ "nvram_selected_js", &ej_nvram_selected_js },
	{ "nvram_selget", &ej_nvram_selget },
	{ "nvram_selmatch", &ej_nvram_selmatch },
	{ "nvram_status_get", &ej_nvram_status_get },
	{ "nvs", &ej_nvs },
	{ "nvsjs", &ej_nvsjs },
	{ "nvsm", &ej_nvsm },
	{ "onload", &ej_onload },
	{ "portsetup", &ej_portsetup },
	{ "port_vlan_table", &ej_port_vlan_table },
	{ "prefix_ip_get", &ej_prefix_ip_get },
	{ "radio_on", &ej_radio_on },
#ifdef HAVE_SAMBA_SERVER
	{ "samba3_sharepaths", &ej_samba3_sharepaths },
	{ "samba3_users", &ej_samba3_users },
#endif
	{ "selchecked", &ej_selchecked },
	{ "show_acktiming", &ej_show_acktiming },
	{ "show_bandwidth", &ej_show_bandwidth },
#ifdef HAVE_BONDING
	{ "show_bondings", &ej_show_bondings },
#endif
#ifdef HAVE_VLANTAGGING
	{ "show_bridgeifnames", &ej_show_bridgeifnames },
	{ "show_bridgenames", &ej_show_bridgenames },
	{ "showbridgesettings", &ej_showbridgesettings },
	{ "show_bridgetable", &ej_show_bridgetable },
#endif
#ifdef HAVE_ATH9K
	{ "show_busy", &ej_show_busy },
#endif
#ifdef HAVE_FREERADIUS
	{ "show_certificate_status", &ej_show_certificate_status },
#endif
#ifdef HAVE_PPPOESERVER
	{ "show_chaps", &ej_show_chaps },
#endif
	{ "show_clocks", &ej_show_clocks },
	{ "show_congestion", &ej_show_congestion },
	{ "show_connectiontype", &ej_show_connectiontype },
	{ "show_control", &ej_show_control },
	{ "show_countrylist", &ej_show_countrylist },
	{ "show_cpucores", &ej_show_cpucores },
	{ "show_cpufeatures", &ej_show_cpufeatures },
	{ "show_cpuinfo", &ej_show_cpuinfo },
#ifdef HAVE_CPUTEMP
	{ "show_cpu_temperature", &ej_show_cpu_temperature },
#endif
	{ "show_ddns_status", &ej_show_ddns_status },
	{ "show_default_level", &ej_show_default_level },
	{ "show_defwpower", &ej_show_defwpower },
	{ "show_dhcpd_settings", &ej_show_dhcpd_settings },
#ifdef HAVE_DNSCRYPT
	{ "show_dnscrypt", &ej_show_dnscrypt },
#endif
	{ "show_dnslist", &ej_show_dnslist },
#ifdef HAVE_EOP_TUNNEL
	{ "show_eop_tunnels", &ej_show_eop_tunnels },
#endif
	{ "show_filterif", &ej_show_filterif },
	{ "show_filters", &ej_show_filters },
	{ "show_forward", &ej_show_forward },
	{ "show_forward_spec", &ej_show_forward_spec },
	{ "show_iflist", &ej_show_iflist },
	{ "show_ifselect", &ej_show_ifselect },
	{ "show_index_setting", &ej_show_index_setting },
	{ "show_infopage", &ej_show_infopage },
#ifdef HAVE_IPV6
	{ "show_ipv6options", &ej_show_ipv6options },
#endif
#ifdef HAVE_IPVS
	{ "show_ipvs", &ej_show_ipvs },
	{ "show_ipvsassignments", &ej_show_ipvsassignments },
#endif
#ifdef HAVE_RADLOCAL
	{ "show_iradius", &ej_show_iradius },
	{ "show_iradius_check", &ej_show_iradius_check },
#endif
#ifdef HAVE_LANGUAGE
	{ "show_languages", &ej_show_languages },
#endif
	{ "show_live_dnslist", &ej_show_live_dnslist },
	{ "show_logo", &ej_show_logo },
	{ "show_macfilter", &ej_show_macfilter },
#ifdef HAVE_VLANTAGGING
	{ "show_mdhcp", &ej_show_mdhcp },
#endif
	{ "show_modules", &ej_show_modules },
#ifdef HAVE_OLSRD
	{ "show_olsrd", &ej_show_olsrd },
#endif
#ifdef HAVE_OPENVPN
	{ "show_openvpn_status", &ej_show_openvpn_status },
#endif
	{ "show_paypal", &ej_show_paypal },
	{ "show_qos_aqd", &ej_show_qos_aqd },
#ifdef HAVE_FREERADIUS
	{ "show_radius_clients", &ej_show_radius_clients },
	{ "show_radius_users", &ej_show_radius_users },
#endif
#ifdef HAVE_RAID
	{ "show_raid", &ej_show_raid },
#endif
	{ "show_routeif", &ej_show_routeif },
	{ "show_routing", &ej_show_routing },
	{ "show_security", &ej_show_security },
	{ "show_staticleases", &ej_show_staticleases },
	{ "show_status", &ej_show_status },
	{ "show_status_setting", &ej_show_status_setting },
	{ "show_styles", &ej_show_styles },
	{ "show_timeoptions", &ej_show_timeoptions },
	{ "show_triggering", &ej_show_triggering },
	{ "show_upgrade_options", &ej_show_upgrade_options },
#ifdef HAVE_USB
	{ "show_usb_diskinfo", &ej_show_usb_diskinfo },
#endif
#ifdef HAVE_CHILLILOCAL
	{ "show_userlist", &ej_show_userlist },
#endif
#ifdef HAVE_VLANTAGGING
	{ "show_vlantagging", &ej_show_vlantagging },
#endif
	{ "show_wan_domain", &ej_show_wan_domain },
	{ "show_wanipinfo", &ej_show_wanipinfo },
	{ "show_wan_to_switch", &ej_show_wan_to_switch },
	{ "show_wds_subnet", &ej_show_wds_subnet },
	{ "show_wifiselect", &ej_show_wifiselect },
	{ "show_wireless", &ej_show_wireless },
	{ "show_wireless_advanced", &ej_show_wireless_advanced },
	{ "show_wl_mac", &ej_show_wl_mac },
	{ "show_wl_wep_setting", &ej_show_wl_wep_setting },
	{ "spectral_scan", &ej_spectral_scan },
#ifdef HAVE_SPUTNIK_APD
	{ "sputnik_apd_status", &ej_sputnik_apd_status },
#endif
	{ "startswith", &ej_startswith },
	{ "statfs", &ej_statfs },
	{ "static_route_setting", &ej_static_route_setting },
	{ "static_route_table", &ej_static_route_table },
	{ "statnv", &ej_statnv },
#ifdef HAVE_NAS_SERVER
	{ "support_fs", &ej_support_fs },
#endif
#ifdef HAVE_SYSCTL_EDIT
	{ "sysctl", &ej_sysctl },
#endif
#ifdef HAVE_UPNP
	{ "tf_upnp", &ej_tf_upnp },
#endif
	{ "tran", &ej_tran },
	{ "update_acktiming", &ej_update_acktiming },
	{ "wan_if_status", &ej_wan_if_status },
	{ "webs_get", &ej_webs_get },
	{ "wireless_active_table", &ej_wireless_active_table },
	{ "wireless_filter_table", &ej_wireless_filter_table },
	{ "wl_packet_get", &ej_wl_packet_get },
	{ "wme_match_op", &ej_wme_match_op },
#ifdef HAVE_MMC
	{ "show_mmc_cardinfo", &ej_show_mmc_cardinfo },
#endif
};
#endif

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
#ifdef HAVE_CHILLILOCAL
	{ "validate_userlist", &validate_userlist },
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
#ifdef HAVE_PPTPD
	{ "delete_pptp", &delete_pptp },
#endif
	{ "save_wifi", &save_wifi },
	{ "save_wifi", &save_wifi },
	{ "dhcp_release", &dhcp_release },
	{ "dhcp_renew", &dhcp_renew },
	{ "stop_ppp", &stop_ppp },
	{ "ttraff_erase", &ttraff_erase },
	{ "save_policy", &save_policy },
	{ "single_delete_policy", &single_delete_policy },
	{ "summary_delete_policy", &summary_delete_policy },
	{ "delete_static_route", &delete_static_route },
#ifndef HAVE_MICRO
	{ "delete_pbr_rule", &delete_pbr_rule },
#endif
	{ "generate_wep_key", &generate_wep_key },
	{ "set_security", &set_security },
	{ "security_save", &security_save },
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
	{ "forward_add", &forward_add },
	{ "forward_remove", &forward_remove },
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
	{ "lease_remove", &lease_remove },
#ifdef HAVE_PPPOESERVER
	{ "chap_user_add", &chap_user_add },
	{ "chap_user_remove", &chap_user_remove },
#endif
#ifdef HAVE_CHILLILOCAL
	{ "user_add", &user_add },
	{ "user_remove", &user_remove },
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
#endif
	{ "add_tunnel", &add_tunnel },
	{ "del_tunnel", &del_tunnel },
	{ "tunnel_save", &tunnel_save },
#endif
	{ "forwardspec_add", &forwardspec_add },
	{ "forwardspec_remove", &forwardspec_remove },
	{ "forwardip_add", &forwardip_add },
	{ "forwardip_remove", &forwardip_remove },
	{ "trigger_add", &trigger_add },
	{ "trigger_remove", &trigger_remove },
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
	dd_debug(DEBUG_HTTPD, "start gozila %s\n", name);
	if (fptr)
		(*fptr) (wp);
	else
		dd_debug(DEBUG_HTTPD, "function %s not found \n", name);
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

	dd_debug(DEBUG_HTTPD, "start validator %s\n", name);
	if (fptr)
		ret = (*fptr) (wp, value, v);
	else
		dd_debug(DEBUG_HTTPD, "function %s not found \n", name);

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
			dd_debug(DEBUG_HTTPD, " %s duration %ld.%06ld\n", name, (long int)r.tv_sec, (long int)r.tv_usec);

		} else {
			dd_debug(DEBUG_HTTPD, " function %s not found (%s)\n", name, "unknown");
		}
	}
	return handle;

}
