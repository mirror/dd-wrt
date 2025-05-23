#ifndef BROADCOM_H_
#define BROADCOM_H_

#include <httpd.h>
#include <typedefs.h>
#include <ddnvram.h>
#include <shutils.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <utils.h>
#include <syslog.h>
#include <bcmcvar.h> // Added by Daniel(2004-07-29) for EZC

#ifndef STATIC_BUILD
#define EJ_VISIBLE __attribute__((used)) __attribute__((__visibility__("default")))
#define EJ_HIDDEN __attribute__((__visibility__("hidden")))
#else
#define EJ_VISIBLE
#define EJ_HIDDEN
#endif
//#define EJ_VISIBLE __attribute__((__visibility__("default")))
//#define EJ_HIDDEN __attribute__((__visibility__("hidden")))

EJ_VISIBLE void wan_proto(webs_t wp);
EJ_VISIBLE void dhcpfwd(webs_t wp);
EJ_VISIBLE void ias_save_admincard(webs_t wp);
EJ_VISIBLE void execute(webs_t wp);
EJ_VISIBLE void clone_mac(webs_t wp);
EJ_VISIBLE void delete_leases(webs_t wp);
EJ_VISIBLE void static_leases(webs_t wp);
EJ_VISIBLE void delete_pptp(webs_t wp);
EJ_VISIBLE void import_vpntunnel(webs_t wp);
EJ_VISIBLE void ssh_downloadkey(webs_t wp);
EJ_VISIBLE void save_wifi(webs_t wp);
EJ_VISIBLE void save_wifi(webs_t wp);
EJ_VISIBLE void dhcp_release(webs_t wp);
EJ_VISIBLE void dhcp_renew(webs_t wp);
EJ_VISIBLE void stop_ppp(webs_t wp);
EJ_VISIBLE void ttraff_erase(webs_t wp);
EJ_VISIBLE void save_policy(webs_t wp);
EJ_VISIBLE void sel_filter(webs_t wp);
EJ_VISIBLE void single_delete_policy(webs_t wp);
EJ_VISIBLE void summary_delete_policy(webs_t wp);
EJ_VISIBLE void delete_static_route(webs_t wp);
EJ_VISIBLE void delete_pbr_rule(webs_t wp);
EJ_VISIBLE void generate_wep_key(webs_t wp);
EJ_VISIBLE void set_security(webs_t wp);
EJ_VISIBLE void security_save(webs_t wp);
EJ_VISIBLE void roaming_save(webs_t wp);
EJ_VISIBLE void add_active_mac(webs_t wp);
EJ_VISIBLE void ping_wol(webs_t wp);
EJ_VISIBLE void save_wds(webs_t wp);
EJ_VISIBLE void save_wireless_advanced(webs_t wp);
EJ_VISIBLE void save_startup(webs_t wp);
EJ_VISIBLE void save_shutdown(webs_t wp);
EJ_VISIBLE void save_firewall(webs_t wp);
EJ_VISIBLE void save_custom(webs_t wp);
EJ_VISIBLE void save_usb(webs_t wp);
EJ_VISIBLE void qos_add_svc(webs_t wp);
EJ_VISIBLE void qos_add_ip(webs_t wp);
EJ_VISIBLE void qos_add_mac(webs_t wp);
EJ_VISIBLE void qos_add_dev(webs_t wp);
EJ_VISIBLE void qos_save(webs_t wp);
EJ_VISIBLE void qossvcs_del(webs_t wp);
EJ_VISIBLE void qosips_del(webs_t wp);
EJ_VISIBLE void qosmacs_del(webs_t wp);
EJ_VISIBLE void qosdevs_del(webs_t wp);
EJ_VISIBLE void forward_remove(webs_t wp);
EJ_VISIBLE void filter_add(webs_t wp);
EJ_VISIBLE void filter_remove(webs_t wp);
EJ_VISIBLE void add_vifs(webs_t wp);
EJ_VISIBLE void remove_vifs(webs_t wp);
EJ_VISIBLE void copy_if(webs_t wp);
EJ_VISIBLE void paste_if(webs_t wp);
EJ_VISIBLE void radius_generate_certificate(webs_t wp);
EJ_VISIBLE void add_radius_user(webs_t wp);
EJ_VISIBLE void del_radius_user(webs_t wp);
EJ_VISIBLE void add_radius_client(webs_t wp);
EJ_VISIBLE void del_radius_client(webs_t wp);
EJ_VISIBLE void save_radius_user(webs_t wp);
EJ_VISIBLE void add_poker_user(webs_t wp);
EJ_VISIBLE void del_poker_user(webs_t wp);
EJ_VISIBLE void save_poker_user(webs_t wp);
EJ_VISIBLE void poker_loaduser(webs_t wp);
EJ_VISIBLE void poker_checkout(webs_t wp);
EJ_VISIBLE void poker_buy(webs_t wp);
EJ_VISIBLE void poker_credit(webs_t wp);
EJ_VISIBLE void poker_back(webs_t wp);
EJ_VISIBLE void add_bond(webs_t wp);
EJ_VISIBLE void del_bond(webs_t wp);
EJ_VISIBLE void add_olsrd(webs_t wp);
EJ_VISIBLE void del_olsrd(webs_t wp);
EJ_VISIBLE void add_vlan(webs_t wp);
EJ_VISIBLE void add_vlanfilter(webs_t wp);
EJ_VISIBLE void add_bridge(webs_t wp);
EJ_VISIBLE void add_bridgeif(webs_t wp);
EJ_VISIBLE void del_vlan(webs_t wp);
EJ_VISIBLE void del_vlanfilter(webs_t wp);
EJ_VISIBLE void del_bridge(webs_t wp);
EJ_VISIBLE void del_bridgeif(webs_t wp);
EJ_VISIBLE void save_networking(webs_t wp);
EJ_VISIBLE void add_mdhcp(webs_t wp);
EJ_VISIBLE void del_mdhcp(webs_t wp);
EJ_VISIBLE void add_ipvs(webs_t wp);
EJ_VISIBLE void del_ipvs(webs_t wp);
EJ_VISIBLE void add_ipvstarget(webs_t wp);
EJ_VISIBLE void del_ipvstarget(webs_t wp);
EJ_VISIBLE void wireless_save(webs_t wp);
EJ_VISIBLE void set_wiviz(webs_t wp);
EJ_VISIBLE void reg_validate(webs_t wp);
EJ_VISIBLE void changepass(webs_t wp);
EJ_VISIBLE void superchannel_validate(webs_t wp);
EJ_VISIBLE void lease_add(webs_t wp);
EJ_VISIBLE void lease_del(webs_t wp);
EJ_VISIBLE void userpass_add(webs_t wp);
EJ_VISIBLE void userpass_del(webs_t wp);
EJ_VISIBLE void chap_user_add(webs_t wp);
EJ_VISIBLE void chap_user_remove(webs_t wp);
EJ_VISIBLE void user_add(webs_t wp);
EJ_VISIBLE void user_remove(webs_t wp);
EJ_VISIBLE void raduser_add(webs_t wp);
EJ_VISIBLE void mdnsif_save(webs_t wp);
EJ_VISIBLE void gen_wg_key(webs_t wp);
EJ_VISIBLE void gen_wg_psk(webs_t wp);
EJ_VISIBLE void gen_wg_client(webs_t wp);
EJ_VISIBLE void del_wg_client(webs_t wp);
EJ_VISIBLE void add_peer(webs_t wp);
EJ_VISIBLE void del_peer(webs_t wp);
EJ_VISIBLE void import_tunnel(webs_t wp);
EJ_VISIBLE void add_tunnel(webs_t wp);
EJ_VISIBLE void del_tunnel(webs_t wp);
EJ_VISIBLE void tunnel_save(webs_t wp);
EJ_VISIBLE void trigger_add(webs_t wp);
EJ_VISIBLE void forwardip_add(webs_t wp);
EJ_VISIBLE void forwardspec_add(webs_t wp);
EJ_VISIBLE void forward_add(webs_t wp);
EJ_VISIBLE void trigger_del(webs_t wp);
EJ_VISIBLE void forwardip_del(webs_t wp);
EJ_VISIBLE void forwardspec_del(webs_t wp);
EJ_VISIBLE void forward_del(webs_t wp);

EJ_VISIBLE void forwardspec_remove(webs_t wp);
EJ_VISIBLE void forwardip_remove(webs_t wp);
EJ_VISIBLE void trigger_remove(webs_t wp);
EJ_VISIBLE void save_services_port(webs_t wp);
EJ_VISIBLE void save_services_port(webs_t wp);
EJ_VISIBLE void diag_ping_start(webs_t wp);
EJ_VISIBLE void diag_ping_stop(webs_t wp);
EJ_VISIBLE void diag_ping_clear(webs_t wp);
EJ_VISIBLE void milkfish_user_add(webs_t wp);
EJ_VISIBLE void milkfish_user_remove(webs_t wp);
EJ_VISIBLE void milkfish_alias_add(webs_t wp);
EJ_VISIBLE void milkfish_alias_remove(webs_t wp);
EJ_VISIBLE void milkfish_sip_message(webs_t wp);
EJ_VISIBLE void gpios_save(webs_t wp);
EJ_VISIBLE void setupassistant_save(webs_t wp);
EJ_VISIBLE void generate_wep_key(webs_t wp);
EJ_VISIBLE void set_security(webs_t wp);
EJ_VISIBLE void security_save(webs_t wp);
EJ_VISIBLE void get_airstation_upgrades(webs_t wp);
EJ_VISIBLE void internetatstart(webs_t wp);
EJ_VISIBLE void intatstart_ajax(webs_t wp);
EJ_VISIBLE void nintendo_save(webs_t wp);
EJ_VISIBLE void wireless_join(webs_t wp);
EJ_VISIBLE void nassrv_save(webs_t wp);
EJ_VISIBLE void add_raid(webs_t wp);
EJ_VISIBLE void del_raid(webs_t wp);
EJ_VISIBLE void add_raid_member(webs_t wp);
EJ_VISIBLE void del_raid_member(webs_t wp);
EJ_VISIBLE void format_raid(webs_t wp);
EJ_VISIBLE void format_drive(webs_t wp);
EJ_VISIBLE void raid_save(webs_t wp);
EJ_VISIBLE void zfs_scrub(webs_t wp);
EJ_VISIBLE void sysctl_save(webs_t wp);
EJ_VISIBLE void ddns_save_value(webs_t wp);
EJ_VISIBLE void portvlan_add(webs_t wp);
EJ_VISIBLE void portvlan_remove(webs_t wp);
EJ_VISIBLE void port_vlan_table_save(webs_t wp);
EJ_VISIBLE void save_wireless_advanced(webs_t wp);
EJ_VISIBLE void save_macmode(webs_t wp);
EJ_VISIBLE void sysctl_save(webs_t wp);
EJ_VISIBLE void hotspot_save(webs_t wp);
EJ_VISIBLE void tf_upnp(webs_t wp);
EJ_VISIBLE void aoss_save(webs_t wp);
EJ_VISIBLE void aoss_start(webs_t wp);
EJ_VISIBLE void wps_register(webs_t wp);
EJ_VISIBLE void wps_ap_register(webs_t wp);
EJ_VISIBLE void wps_forcerelease(webs_t wp);
EJ_VISIBLE void wps_configure(webs_t wp);

void showRadioNoDef(webs_t wp, char *propname, char *nvname, int val);
void showInputNum(webs_t wp, char *propname, char *nvname, int size, int maxsize, int def);
void showInput(webs_t wp, char *propname, char *nvname, int size, int maxsize, char *def);

#define showRadioDefaultOn(wp, propname, nvname)                                     \
	do {                                                                         \
		showRadioNoDef(wp, propname, nvname, nvram_default_geti(nvname, 1)); \
	} while (0)

#define showRadioDefaultOff(wp, propname, nvname)                                    \
	do {                                                                         \
		showRadioNoDef(wp, propname, nvname, nvram_default_geti(nvname, 0)); \
	} while (0)

#define showRadioDefaultOnPrefix(wp, propname, nv, prefix)                           \
	do {                                                                         \
		char nvname[32];                                                     \
		sprintf(nvname, "%s_%s", prefix, nv);                                \
		showRadioNoDef(wp, propname, nvname, nvram_default_geti(nvname, 1)); \
	} while (0)

#define showRadioDefaultOffPrefix(wp, propname, nv, prefix)                          \
	do {                                                                         \
		char nvname[64];                                                     \
		sprintf(nvname, "%s_%s", prefix, nv);                                \
		showRadioNoDef(wp, propname, nvname, nvram_default_geti(nvname, 0)); \
	} while (0)

void showRadio(webs_t wp, char *propname, char *nvname);
void showRadioInv(webs_t wp, char *propname, char *nvname);
void showRadioPrefix(webs_t wp, char *propname, char *nv, char *prefix);
void showInputNumPrefix(webs_t wp, char *propname, char *nv, char *prefix, int size, int maxsize, int def);
void showInputPrefix(webs_t wp, char *propname, char *nv, char *prefix, int size, int maxsize, char *def);

void showRadioInvPrefix(webs_t wp, char *propname, char *nv, char *prefix);

#define service_restart() kill(1, SIGUSR1)
/*
 * #define sys_restart() kill(1, SIGHUP) #define sys_reboot() kill(1,
 * SIGTERM) 
 */
// #define service_restart() eval("event","3","1","16")

#ifdef HAVE_WRK54G
#define sys_restart() kill(1, 1)
#define sys_reboot() kill(1, 15)
#else
#define sys_restart() eval("event", "0", "1", "1")
#define sys_reboot() eval("event", "3", "1", "15")
#endif

#define sys_stats(url) eval("stats", (url))
#define ARGV(args...) ((char *[]){ args, NULL })
#define STRUCT_LEN(name) sizeof(name) / sizeof(name[0])
char *GOZILA_GET(webs_t wp, char *name);

#define SWAP(AA, BB)      \
	{                 \
		char *CC; \
		CC = AA;  \
		AA = BB;  \
		BB = CC;  \
	}

extern void do_vsp_info_page(unsigned char method, struct mime_handler *handler, char *url, webs_t stream);
/*
 * for dhcp 
 */
#define MAX_LEASES 254

/*
 * for filter 
 */
#define FILTER_IP_NUM 5
#define FILTER_PORT_NUM 5
#define FILTER_MAC_NUM 10
#define FILTER_MAC_PAGE 5
#define BLOCKED_SERVICE_NUM 5

// changed by steve
/*
 * for forward 
 */
// #define FORWARDING_NUM 60
#define SPECIAL_FORWARDING_NUM 30
#define UPNP_FORWARDING_NUM 30
// #define PORT_TRIGGER_NUM 50
// end changed by steve

/*
 * for static route 
 */
#define STATIC_ROUTE_PAGE 64

/*
 * for wireless 
 */
#define WL_FILTER_MAC_PAGE 2
#define WL_FILTER_MAC_NUM 128
// #define WL_FILTER_MAC_COUNT 32

#define MAC_LEN 17
#define TMP_PASSWD "d6nw5v1x2pc7st9m"

/*
 * struct variable { char *name; char *longname; void (*validate)(webs_t wp,
 * char *value, struct variable *v); char **argv; int nullok; }; 
 */

struct onload {
	char *name;
	void (*go)(webs_t wp, char *arg);
};

struct lease_t {
	unsigned char chaddr[16];
	u_int32_t yiaddr;
	u_int32_t expires;
	char hostname[64];
};

struct apply_action {
	char *name;
	char *service;
	int action;
	char *goname;
};

struct gozila_action {
	char *name;
	char *type;
	char *service;
	int action;
	char *goname;
};

enum { SET, GET };

enum { // return code
	START_FROM = 10,
};

extern void addAction(char *action);
extern void registerCustom(char *action);

/*
 * SEG addition for dynamic nvram layout 
 */
extern void Initnvramtab(void);
extern void prefix_ip_get(char *name, char *buf, int type);

/*
 * for index 
 */
extern void ej_show_routeif(webs_t wp, int argc, char_t **argv);
extern void ej_show_olsrd(webs_t wp, int argc, char_t **argv);
extern void ej_list_mac_layers(webs_t wp, int argc, char_t **argv);
extern void ej_show_macfilter(webs_t wp, int argc, char_t **argv);
extern void ej_getregcode(webs_t wp, int argc, char_t **argv);
extern void ej_show_index_setting(webs_t wp, int argc, char_t **argv);
extern void ej_show_wds_subnet(webs_t wp, int argc, char_t **argv);
extern void ej_compile_date(webs_t wp, int argc, char_t **argv);
extern void ej_compile_time(webs_t wp, int argc, char_t **argv);
extern void ej_get_wl_max_channel(webs_t wp, int argc, char_t **argv);
extern void ej_get_wl_domain(webs_t wp, int argc, char_t **argv);
extern void ej_get_clone_mac(webs_t wp, int argc, char_t **argv);
extern void ej_show_wan_domain(webs_t wp, int argc, char_t **argv);
extern void ej_show_wl_mac(webs_t wp, int argc, char_t **argv);
extern void ej_show_acktiming(webs_t wp, int argc, char_t **argv);
extern void ej_update_acktiming(webs_t wp, int argc, char_t **argv);
extern void validate_ntp(webs_t wp, char *value, struct variable *v);
extern void validate_catchall(webs_t wp, char *value, struct variable *v);
extern void validate_lan_ipaddr(webs_t wp, char *value, struct variable *v);
extern void validate_wan_ipaddr(webs_t wp, char *value, struct variable *v);
extern void validate_remote_ip(webs_t wp, char *value, struct variable *v);
extern void validate_portsetup(webs_t wp, char *value, struct variable *v);
extern void clone_mac(webs_t wp);
extern void dhcpfwd(webs_t wp);
extern void set_wiviz(webs_t wp);
extern void wan_proto(webs_t wp);
extern void ej_show_wan_to_switch(webs_t wp, int argc, char_t **argv); /* Added 
									 * by 
									 * Botho 
									 * 10.May.06 
									 */
extern void ej_show_countrylist(webs_t wp, int argc, char_t **argv);

void convert_wl_gmode(char *value, char *prefix);

/*
 * for status 
 */
extern int nvram_selmatch(webs_t wp, char *name, char *match);
extern void ej_show_wifiselect(webs_t wp, int argc, char_t **argv);
extern void ej_show_ifselect(webs_t wp, int argc, char_t **argv);
extern void ej_portsetup(webs_t wp, int argc, char_t **argv);
extern void ej_bandwidth(webs_t wp, int argc, char_t **argv);
extern void ej_show_paypal(webs_t wp, int argc, char_t **argv);
extern void ej_show_vlantagging(webs_t wp, int argc, char_t **argv);
extern void ej_show_mdhcp(webs_t wp, int argc, char_t **argv);
extern void ej_show_bondings(webs_t wp, int argc, char_t **argv);
extern void ej_show_bridgenames(webs_t wp, int argc, char_t **argv);
extern void ej_show_bridgetable(webs_t wp, int argc, char_t **argv);
extern void ej_show_bridgeifnames(webs_t wp, int argc, char_t **argv);
extern void ej_show_routing(webs_t wp, int argc, char_t **argv);
extern void ej_show_connectiontype(webs_t wp, int argc, char_t **argv);
extern void ej_dumpmeminfo(webs_t wp, int argc, char_t **argv);
extern void ej_get_clkfreq(webs_t wp, int argc, char_t **argv);
extern void ej_show_cpuinfo(webs_t wp, int argc, char_t **argv);
extern void ej_show_dhcpd_settings(webs_t wp, int argc, char_t **argv);
extern void ej_show_status(webs_t wp, int argc, char_t **argv);
extern void ej_localtime(webs_t wp, int argc, char_t **argv);
extern void ej_nvram_status_get(webs_t wp, int argc, char_t **argv);
extern void ej_nvram_get_len(webs_t wp, int argc, char_t **argv);
extern void ej_dhcp_remaining_time(webs_t wp, int argc, char_t **argv);
extern void dhcp_renew(webs_t wp);
extern void add_olsrd(webs_t wp);
extern void del_olsrd(webs_t wp);
extern void save_macmode(webs_t wp);
extern void dhcp_release(webs_t wp);
extern void stop_ppp(webs_t wp);
extern void add_bridge(webs_t wp);
extern void add_vlan(webs_t wp);
extern void add_mdhcp(webs_t wp);
extern void add_bond(webs_t wp);
extern void add_bridgeif(webs_t wp);
extern void del_bridge(webs_t wp);
extern void del_vlan(webs_t wp);
extern void del_mdhcp(webs_t wp);
extern void del_bond(webs_t wp);
extern void del_bridgeif(webs_t wp);
extern void save_networking(webs_t wp);
extern void ej_show_status_setting(webs_t wp, int argc, char_t **argv);
extern void ej_dumpip_conntrack(webs_t wp, int argc, char_t **argv);
extern void ej_ip_conntrack_table(webs_t wp, int argc, char_t **argv);
extern void ej_gethostnamebyip(webs_t wp, int argc, char_t **argv);

/*
 * for pppoeserver
 */
extern void validate_chaps(webs_t wp, char *value, struct variable *v);
extern void ej_show_chaps(webs_t wp, int argc, char_t **argv);

/*
 * for dhcp 
 */
extern void ej_dumpleases(webs_t wp, int argc, char_t **argv);
extern void validate_dhcp(webs_t wp, char *value, struct variable *v);
extern void validate_dhcp_check(webs_t wp, char *value, struct variable *v);
extern void delete_leases(webs_t wp);
extern void static_leases(webs_t wp);

/*
 * for log 
 */
extern void ej_dumplog(webs_t wp, int argc, char_t **argv);

#ifdef HAVE_SPUTNIK_APD
extern void ej_sputnik_apd_status(webs_t wp, int argc, char_t **argv);
extern void ej_show_sputnik(webs_t wp, int argc, char_t **argv);
#endif
extern void log_onload(webs_t wp);
extern void filtersummary_onload(webs_t wp, char *arg);

/*
 * for upgrade 
 */
extern int sys_restore(char *url, webs_t stream, int *total);
extern void do_restore_post(char *url, webs_t stream, int len, char *boundary);
extern void do_restore_cgi(char *url, webs_t stream);
extern void macclone_onload(webs_t wp, char *arg);
extern void do_olupgrade_post(char *url, webs_t stream, int len, char *boundary);
extern void do_olupgrade_cgi(unsigned char method, struct mime_handler *handler, char *url, webs_t stream);
/*
 * for filter 
 */
extern void ej_filter_init(webs_t wp, int argc, char_t **argv);
extern void ej_filter_summary_show(webs_t wp, int argc, char_t **argv);
extern void ej_filter_ip_get(webs_t wp, int argc, char_t **argv);
extern void ej_filter_port_get(webs_t wp, int argc, char_t **argv);
extern void ej_filter_dport_get(webs_t wp, int argc, char_t **argv);
extern void ej_filter_mac_get(webs_t wp, int argc, char_t **argv);
extern void ej_filter_policy_select(webs_t wp, int argc, char_t **argv);
extern void ej_filter_policy_get(webs_t wp, int argc, char_t **argv);
extern void ej_filter_tod_get(webs_t wp, int argc, char_t **argv);
extern void ej_filter_web_get(webs_t wp, int argc, char_t **argv);
extern void filter_port_services_get(webs_t wp, char *type, int which);
extern void validate_filter_policy(webs_t wp, char *value, struct variable *v);
extern void validate_filter_ip_grp(webs_t wp, char *value, struct variable *v);
extern void validate_filter_mac_grp(webs_t wp, char *value, struct variable *v);
extern void validate_filter_dport_grp(webs_t wp, char *value, struct variable *v);
extern void validate_filter_port(webs_t wp, char *value, struct variable *v);
extern void validate_filter_web(webs_t wp, char *value, struct variable *v);
extern void validate_blocked_service(webs_t wp, char *value, struct variable *v);
extern void filter_onload(webs_t wp);
extern void save_policy(webs_t wp);
extern void sel_filter(webs_t wp);
extern void summary_delete_policy(webs_t wp);
extern void single_delete_policy(webs_t wp);
extern void save_services_port(webs_t wp);

/*
 * for forward 
 */
extern void ej_show_default_level(webs_t wp, int argc, char_t **argv);
extern void port_forward_table(webs_t wp, char *type, int which);
extern void port_forward_spec(webs_t wp, char *type, int which);
extern void ej_show_staticleases(webs_t wp, int argc, char_t **argv);
extern void validate_forward_proto(webs_t wp, char *value, struct variable *v);
extern void validate_staticleases(webs_t wp, char *value, struct variable *v);
extern void validate_forward_spec(webs_t wp, char *value, struct variable *v);
extern void port_trigger_table(webs_t wp, char *which, int type);
extern void validate_port_trigger(webs_t wp, char *value, struct variable *v);

// Changed by Steve
// extern int ej_forward_upnp( webs_t wp, int argc, char_t **argv); //upnp
// added
// extern void validate_forward_upnp(webs_t wp, char *value, struct variable
// *v); //upnp added
// End Changed by Steve

/*
 * for dynamic route 
 */
extern void ej_dump_route_table(webs_t wp, int argc, char_t **argv);
extern void validate_dynamic_route(webs_t wp, char *value, struct variable *v);
extern void dynamic_route_onload(webs_t wp);

/*
 * for static route 
 */
extern void ej_static_route_setting(webs_t wp, int argc, char_t **argv);
extern void ej_static_route_table(webs_t wp, int argc, char_t **argv);
extern void validate_static_route(webs_t wp, char *value, struct variable *v);
extern void delete_static_route(webs_t wp);
extern void delete_pbr_rule(webs_t wp);

/*
 * for wireless 
 */
extern void validate_wl_key(webs_t wp, char *value, struct variable *v);
extern void validate_wl_wep(webs_t wp, char *value, struct variable *v);
extern void validate_wl_auth(webs_t wp, char *value, struct variable *v);
extern void validate_d11_channel(webs_t wp, char *value, struct variable *v);
extern void add_active_mac(webs_t wp);
extern void wl_active_onload(webs_t wp, char *arg);
extern void generate_key_64(webs_t wp);
extern void generate_key_128(webs_t wp);
extern void ej_showad(webs_t wp, int argc, char_t **argv);
extern void ej_get_wep_value(webs_t wp, int argc, char_t **argv);
extern void ej_get_wl_active_mac(webs_t wp, int argc, char_t **argv);
extern void ej_get_wl_value(webs_t wp, int argc, char_t **argv);

extern void security_save(webs_t wp);
extern void ej_show_wpa_setting(webs_t wp, int argc, char_t **argv, char *prefix);
extern void wl_unit(webs_t wp, char *value, struct variable *v);
extern void validate_wpa_psk(webs_t wp, char *value, struct variable *v);
extern void validate_auth_mode(webs_t wp, char *value, struct variable *v);
extern void validate_security_mode(webs_t wp, char *value, struct variable *v);
extern void ej_wl_ioctl(webs_t wp, int argc, char_t **argv);
extern void validate_wl_gmode(webs_t wp, char *value, struct variable *v);
extern void validate_wl_net_mode(webs_t wp, char *value, struct variable *v);

/*
 * for nvram save-restore 
 */
/*
 * for traff data save-restore 
 */
extern void ttraff_erase(webs_t wp);

/*
 * for ddns 
 */
extern void ddns_save_value(webs_t wp);
extern void ddns_update_value(webs_t wp);
extern void ej_show_ddns_status(webs_t wp, int argc, char_t **argv);
extern void ej_show_ddns_ip(webs_t wp, int argc, char_t **argv);

extern void validate_macmode(webs_t wp, char *value, struct variable *v);
extern void validate_wl_hwaddrs(webs_t wp, char *value, struct variable *v);
extern void ej_wireless_active_table(webs_t wp, int argc, char_t **argv);
extern void ej_wireless_filter_table(webs_t wp, int argc, char_t **argv);
extern void ej_show_wl_wep_setting(webs_t wp, int argc, char_t **argv);
extern void validate_wl_wep_key(webs_t wp, char *value, struct variable *v);

extern void ej_wl_packet_get(webs_t wp, int argc, char_t **argv);

/*
 * for usb
 */
extern void ej_show_usb_diskinfo(webs_t wp, int argc, char_t **argv);
extern void ej_show_mmc_cardinfo(webs_t wp, int argc, char_t **argv);

/*
 * Ping and Traceroute 
 */
extern void ej_dump_ping_log(webs_t wp, int argc, char_t **argv);

// extern void ej_dump_traceroute_log ( webs_t wp, int argc, char_t ** argv);
extern void diag_ping_start(webs_t wp);
extern void diag_ping_stop(webs_t wp);
extern void diag_ping_clear(webs_t wp);

// extern int diag_traceroute_start (webs_t wp);
// extern int diag_traceroute_stop (webs_t wp);
// extern int diag_traceroute_clear (webs_t wp);
extern void ping_onload(webs_t wp, char *arg);

// extern int traceroute_onload (webs_t wp, char *arg);

/*
 * Added by Botho 21.April.06 
 */
extern void ej_statfs(webs_t wp, int argc, char_t **argv);
extern void ej_statnv(webs_t wp, int argc, char_t **argv);

/*
 * for all 
 */
extern void ej_onload(webs_t wp, int argc, char_t **argv);
extern void ej_get_web_page_name(webs_t wp, int argc, char_t **argv);
extern void ej_compile_date(webs_t wp, int argc, char_t **argv);
extern void ej_compile_time(webs_t wp, int argc, char_t **argv);
extern void ej_get_model_name(webs_t wp, int argc, char_t **argv);
extern void ej_get_firmware_version(webs_t wp, int argc, char_t **argv);
extern void ej_get_firmware_title(webs_t wp, int argc, char_t **argv);
extern void ej_get_firmware_svnrev(webs_t wp, int argc, char_t **argv);
extern void rep(char *in, char from, char to);

extern void save_wds(webs_t wp);
extern void save_wireless_advanced(webs_t wp);
extern void validate_iradius(webs_t wp, char *value, struct variable *v);
extern void validate_wds(webs_t wp, char *value, struct variable *v);
extern void ej_get_wdsp2p(webs_t wp, int argc, char_t **argv);
extern void ej_active_wireless(webs_t wp, int argc, char_t **argv);

#ifdef HAVE_SKYTRON
extern void ej_active_wireless2(webs_t wp, int argc, char_t **argv);
#endif
extern void ej_show_clocks(webs_t wp, int argc, char_t **argv);
extern void ej_active_wds(webs_t wp, int argc, char_t **argv);
extern void ej_show_iradius_check(webs_t wp, int argc, char_t **argv);
extern void ej_show_iradius(webs_t wp, int argc, char_t **argv);
extern void ej_get_wds_mac(webs_t wp, int argc, char_t **argv);
extern void ej_get_wds_ip(webs_t wp, int argc, char_t **argv);
extern void ej_get_wds_netmask(webs_t wp, int argc, char_t **argv);
extern void ej_get_wds_gw(webs_t wp, int argc, char_t **argv);
extern void ej_get_br1_ip(webs_t wp, int argc, char_t **argv);
extern void ej_get_br1_netmask(webs_t wp, int argc, char_t **argv);
extern void ej_get_curchannel(webs_t wp, int argc, char_t **argv);
extern void ej_get_currate(webs_t wp, int argc, char_t **argv);
extern void ej_get_uptime(webs_t wp, int argc, char_t **argv);
extern void ej_get_wan_uptime(webs_t wp, int argc, char_t **argv);
extern void ej_get_qossvcs(webs_t wp, int argc, char_t **argv);
extern void ej_get_qosips(webs_t wp, int argc, char_t **argv);
extern void ej_get_qosmacs(webs_t wp, int argc, char_t **argv);
extern void ej_get_qossvcs2(webs_t wp, int argc, char_t **argv);
extern void ej_get_qosips2(webs_t wp, int argc, char_t **argv);
extern void ej_get_qosmacs2(webs_t wp, int argc, char_t **argv);
extern void ej_show_defwpower(webs_t wp, int argc, char_t **argv);

// extern void ej_get_services_options (webs_t wp, int argc, char_t ** argv);
extern void ej_show_infopage(webs_t wp, int argc, char_t **argv);
extern void ej_show_control(webs_t wp, int argc, char_t **argv);
extern void ej_get_clone_wmac(webs_t wp, int argc, char_t **argv);
extern void raduser_add(webs_t wp);
extern void qos_add_svc(webs_t wp);
extern void qos_add_ip(webs_t wp);
extern void qos_add_mac(webs_t wp);
extern void qos_save(webs_t wp);
extern void ping_wol(webs_t wp);
extern void ping_startup(webs_t wp);
extern void ping_firewall(webs_t wp);
extern void ping_custom(webs_t wp);

extern void add_vifs(webs_t wp);
extern void remove_vifs(webs_t wp);

// extern int ej_show_virtualssid( webs_t wp, int argc, char_t **argv);
extern void ej_show_security(webs_t wp, int argc, char_t **argv);

extern void reg_validate(webs_t wp);
extern void superchannel_validate(webs_t wp);
extern void wireless_save(webs_t wp);
extern void set_security(webs_t wp);
extern void forward_add(webs_t wp);
extern void forward_del(webs_t wp);
extern void forward_remove(webs_t wp);
extern void lease_add(webs_t wp);
extern void lease_del(webs_t wp);
extern void chap_user_add(webs_t wp);
extern void chap_user_remove(webs_t wp);

#ifdef HAVE_CHILLILOCAL
extern void user_add(webs_t wp);
extern void user_remove(webs_t wp);
extern void ej_show_userlist(webs_t wp, int argc, char_t **argv);
extern void validate_userlist(webs_t wp, char *value, struct variable *v);
#endif
extern void forwardspec_add(webs_t wp);
extern void forwardspec_del(webs_t wp);
extern void forwardspec_remove(webs_t wp);
extern void forwardip_add(webs_t wp);
extern void forwardip_del(webs_t wp);
extern void forwardip_remove(webs_t wp);
extern void trigger_add(webs_t wp);
extern void trigger_del(webs_t wp);
extern void trigger_remove(webs_t wp);

/*
 * lonewolf additions 
 */
extern void ej_port_vlan_table(webs_t wp, int argc, char_t **argv);
extern void port_vlan_table_save(webs_t wp);
extern void ej_if_config_table(webs_t wp, int argc, char_t **argv);
extern void if_config_table_save(webs_t wp);
extern void alias_delete_ip(webs_t wp);

/*
 * end lonewolf additions 
 */

extern int valid_ipaddr(webs_t wp, char *value, struct variable *v);

#ifndef VALIDSOURCE
#define valid_range(wp, value, v) start_validator("valid_range", wp, value, v)
#define valid_hwaddr(wp, value, v) start_validator("valid_hwaddr", wp, value, v)
#define valid_netmask(wp, value, v) start_validator("valid_netmask", wp, value, v)
#define valid_wep_key(wp, value, v) start_validator("valid_wep_key", wp, value, v)
#define valid_choice(wp, value, v) start_validator("valid_choice", wp, value, v)
#else

extern int valid_range(webs_t wp, char *value, struct variable *v);
extern int valid_hwaddr(webs_t wp, char *value, struct variable *v);
extern int valid_netmask(webs_t wp, char *value, struct variable *v);
extern int valid_wep_key(webs_t wp, char *value, struct variable *v);
extern int valid_choice(webs_t wp, char *value, struct variable *v);
#endif
extern void validate_statics(webs_t wp, char *value, struct variable *v);
extern int valid_name(webs_t wp, char *value, struct variable *v, int unwanted);
extern int valid_merge_ipaddrs(webs_t wp, char *value, struct variable *v);

extern int get_dns_ip(char *name, int which, int count);
extern int get_single_ip(char *ipaddr, int which);

// extern int get_merge_ipaddr (webs_t wp, char *name, char *ipaddr);
extern int get_merge_mac(webs_t wp, char *name, char *macaddr);
extern char *rfctime(const time_t *timep, char *t);
extern int legal_ipaddr(char *value);
extern int legal_hwaddr(char *value);
extern int legal_netmask(char *value);
extern int legal_ip_netmask(char *sip, char *smask, char *dip);
extern int find_pattern(const char *data, size_t dlen, const char *pattern, size_t plen, char term, unsigned int *numoff,
			unsigned int *numlen);
extern int tf_webWriteESCNV(webs_t wp, const char *nvname);
extern int tf_webWriteESC(webs_t wp, const char *value);

extern int find_match_pattern(char *name, size_t mlen, const char *data, const char *pattern, char *def);

extern int find_each(char *name, int len, char *data, char *token, int which, char *def);

char *tran_string(char *buf, size_t len, char *str);

/*
 * set type to 1 to replace ' ' with "&nbsp;" and ':' with "&semi;"
 * set type to 2 to replace "&nbsp;" with ' ' and "&semi;" with ':'
 */
extern int httpd_filter_name(char *old_name, char *new_name, size_t size, int type);

/*
 * check the value for a digit (0 through 9) set flag to 0 to ignore
 * zero-length values 
 */
extern int ISDIGIT(char *value, int flag);

/*
 * checks whether value is a 7-bit unsigned char value that fits into the
 * ASCII character set set flag to 0 to ignore zero-length values 
 */
extern int ISASCII(char *value, int flag, int unwanted);
extern int tf_webWriteJS(webs_t wp, const char *s);

extern void do_setup_wizard(char *url, webs_t stream);
extern void ej_show_turbo(webs_t wp, int argc, char_t **argv);
extern void ej_show_adwifi(webs_t wp, int argc, char_t **argv);
extern void ej_show_wireless(webs_t wp, int argc, char_t **argv);
extern void ej_show_logo(webs_t wp, int argc, char_t **argv);
extern void ej_show_sysinfo(webs_t wp, int argc, char_t **argv);
extern void ej_show_miscinfo(webs_t wp, int argc, char_t **argv);
extern void ej_get_backup_name(webs_t wp, int argc, char_t **argv);
extern struct servent *my_getservbyport(int port, const char *proto);
extern int get_single_mac(char *macaddr, int which);

extern int StopContinueTx(webs_t wp, char *value);
extern int StartContinueTx(webs_t wp, char *value);
extern int Check_TSSI(webs_t wp, char *value);
extern int Get_TSSI(char *value);
extern int Enable_TSSI(char *value);

extern void LOG(int level, const char *fmt, ...);

extern char *num_to_protocol(int num);
extern int protocol_to_num(char *proto);
extern void do_backup(char *path, webs_t stream);

extern void ej_per_port_option(webs_t wp, int argc, char_t **argv);
extern void validate_port_qos(webs_t wp, char *value, struct variable *v);
extern void ej_qos_sw_default(webs_t wp, int argc, char_t **argv);
extern void ej_show_meminfo(webs_t wp, int argc, char_t **argv);
extern void ej_dump_site_survey(webs_t wp, int argc, char_t **argv);
extern void ej_dump_wiviz_data(webs_t wp, int argc, char_t **argv);
extern void ej_get_url(webs_t wp, int argc, char_t **argv);
extern char *getNetworkLabel(webs_t wp, char *var);

#ifdef FBNFW
extern void ej_list_fbn(webs_t wp, int argc, char_t **argv);
#endif
extern void ej_wme_match_op(webs_t wp, int argc, char_t **argv);
extern void validate_noack(webs_t wp, char *value, struct variable *v);
extern void validate_wl_wme_params(webs_t wp, char *value, struct variable *v);

extern void validate_choice(webs_t wp, char *value, struct variable *v);

#define MAX_WDS_DEVS 10

int ishexit(char c);
int sv_valid_hwaddr(char *value);
int sv_valid_ipaddr(char *value);
int sv_valid_range(char *value, int low, int high);
int sv_valid_statics(char *value);
void get_network(char *ipaddr, char *netmask);
void get_broadcast(char *ipaddr, size_t len, char *netmask);
int route_manip(int cmd, char *name, int metric, char *dst, char *gateway, char *genmask);
int route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
int route_del(char *name, int metric, char *dst, char *gateway, char *genmask);

extern void show_preshared(webs_t wp, char *prefix);
extern void show_radius(webs_t wp, char *prefix, int showmac, int backup);
extern void show_wparadius(webs_t wp, char *prefix);
extern void show_wep(webs_t wp, char *prefix);
extern char *get_wep_value(webs_t wp, char *buf, char *type, char *_bit, char *prefix);

#ifdef HAVE_MILKFISH
extern void ej_exec_milkfish_service(webs_t wp, int argc, char_t **argv);
extern void ej_exec_milkfish_phonebook(webs_t wp, int argc, char_t **argv);
extern void ej_show_subscribers(webs_t wp, int argc, char_t **argv);
extern void milkfish_user_add(webs_t wp);
extern void milkfish_user_remove(webs_t wp);
extern void validate_subscribers(webs_t wp, char *value, struct variable *v);
extern void ej_show_aliases(webs_t wp, int argc, char_t **argv);
extern void milkfish_alias_add(webs_t wp);
extern void milkfish_alias_remove(webs_t wp);
extern void validate_aliases(webs_t wp, char *value, struct variable *v);
extern void milkfish_sip_message(webs_t wp);
extern void ej_show_registrations(webs_t wp, int argc, char_t **argv);
#endif

#ifdef HAVE_RFLOW
extern void ej_show_rflowif(webs_t wp, int argc, char_t **argv);
#endif

#ifdef HAVE_OPENVPN
//extern void ej_show_openvpnuserpass(webs_t wp, int argc, char_t ** argv);   //see show_staticleases
extern void validate_openvpnuserpass(webs_t wp, char *value, struct variable *v); //see validate_staticleases
extern void userpass_add(webs_t wp);
extern void userpass_del(webs_t wp);
#endif

#ifndef BASEDEF
void ej_showbridgesettings(webs_t wp, int argc, char_t **argv);

void do_ej_buffer(char *buffer, webs_t stream);

char *websGetVar(webs_t wp, char *var, char *d);

char *websGetSaneVar(webs_t wp, char *var, char *d);

int websGetVari(webs_t wp, char *var, int d);

char *validate_websGetVar(webs_t wp, char *var, char *d);

int validate_websGetVari(webs_t wp, char *var, int d);

size_t websWrite(webs_t wp, char *fmt, ...) __attribute__((format(printf, 2, 3)));

void do_ej(unsigned char method, struct mime_handler *handler, char *path, webs_t stream);

FILE *getWebsFile(webs_t wp, char *path);
FILE *_getWebsFile(webs_t wp, char *path, size_t *len);

int wfputs(char *buf, webs_t fp);

char *live_translate(webs_t wp, const char *tran);

char *GOZILA_GET(webs_t wp, char *name);

void validate_cgi(webs_t fp);

#endif
#endif
