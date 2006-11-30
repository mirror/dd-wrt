#include <httpd.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <utils.h>
#include <syslog.h>
#include <bcmcvar.h>		//Added by Daniel(2004-07-29) for EZC

/*
#define service_restart() kill(1, SIGUSR1)
#define sys_restart() kill(1, SIGHUP)
#define sys_reboot() kill(1, SIGTERM)
*/
#define service_restart() eval("event","3","1","16")
#define sys_restart() eval("event","3","1","1")
#define sys_reboot() eval("event","3","1","15")

#define sys_stats(url) eval("stats", (url))
#define ARGV(args...) ((char *[]) { args, NULL })
#define STRUCT_LEN(name)    sizeof(name)/sizeof(name[0])
#define GOZILA_GET(name)	gozila_action ? websGetVar(wp, name, NULL) : nvram_safe_get(name);

#define SWAP(AA,BB)  { \
	char *CC; \
	CC = AA; \
	AA = BB; \
	BB = CC; \
}

/* for dhcp */
#define MAX_LEASES 254

/* for filter */
#define FILTER_IP_NUM 5
#define FILTER_PORT_NUM 5
#define FILTER_MAC_NUM 10
#define FILTER_MAC_PAGE 5
#define BLOCKED_SERVICE_NUM 5


// changed by steve
/* for forward */
//#define FORWARDING_NUM 60
#define SPECIAL_FORWARDING_NUM 30
#define UPNP_FORWARDING_NUM 30
//#define PORT_TRIGGER_NUM 50
// end changed by steve


/* for static route */
#define STATIC_ROUTE_PAGE 20

/* for wireless */
#define WL_FILTER_MAC_PAGE 2
#define WL_FILTER_MAC_NUM 64
//#define WL_FILTER_MAC_COUNT 32


#define MAC_LEN 17
#define TMP_PASSWD "d6nw5v1x2pc7st9m"

#define USE_LAN 1
#define USE_WAN 2

extern int gozila_action;
extern int error_value;
extern int debug_value;
extern int filter_id;
extern int generate_key;
extern int clone_wan_mac;
extern char http_client_ip[20];
extern int lan_ip_changed;

/*
struct variable {
	char *name;
	char *longname;
	void (*validate)(webs_t wp, char *value, struct variable *v);
	char **argv;
	int nullok;
};
*/

struct onload
{
  char *name;
  int (*go) (webs_t wp, char *arg);
};

struct lease_t
{
  unsigned char chaddr[16];
  u_int32_t yiaddr;
  u_int32_t expires;
  char hostname[64];
};

struct apply_action
{
  char *name;
  char *service;
  int sleep_time;
  int action;
  int (*go) (webs_t wp);
};

struct gozila_action
{
  char *name;
  char *type;
  char *service;
  int sleep_time;
  int action;
  int (*go) (webs_t wp);
};

enum
{ SET, GET };

enum
{				// return code
  START_FROM = 10,
};

/* SEG addition for dynamic nvram layout */
extern void Initnvramtab (void);
extern void prefix_ip_get (char *name, char *buf, int type);

extern int has_mimo(void);


/* for index */
extern void ej_list_mac_layers(int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_macfilter (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_getregcode (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_index_setting (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_wds_subnet (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_compile_date (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_compile_time (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_wl_max_channel (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_wl_domain (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_clone_mac (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_wan_domain (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_wl_mac (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_ntp (webs_t wp, char *value, struct variable *v);
extern void validate_catchall (webs_t wp, char *value, struct variable *v);
extern void validate_lan_ipaddr (webs_t wp, char *value, struct variable *v);
extern void validate_wan_ipaddr (webs_t wp, char *value, struct variable *v);
extern void validate_portsetup (webs_t wp, char *value, struct variable *v);
extern int clone_mac (webs_t wp);
extern int dhcpfwd (webs_t wp);
extern int wan_proto (webs_t wp);
extern void ej_show_wan_to_switch (int eid, webs_t wp, int argc, char_t ** argv);	/* Added by Botho 10.May.06 */


/* for status */
extern int nvram_selmatch (webs_t wp, char *name, char *match);
extern void ej_portsetup (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_bandwidth (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_paypal (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_routing (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_connectiontype (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_dumpmeminfo (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_clkfreq (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_cpuinfo (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_dhcpd_settings (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_status (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_localtime (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_nvram_status_get (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_nvram_get_len (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_dhcp_remaining_time (int eid, webs_t wp, int argc, char_t ** argv);
extern int dhcp_renew (webs_t wp);
extern int save_macmode (webs_t wp);
extern int dhcp_release (webs_t wp);
extern int stop_ppp (webs_t wp);
extern void ej_show_status_setting (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_dumpip_conntrack (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_ip_conntrack_table (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_gethostnamebyip (int eid, webs_t wp, int argc, char_t ** argv);


/*for dhcp */
extern char *dhcp_reltime (char *buf, time_t t);
extern void ej_dumpleases (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_dhcp (webs_t wp, char *value, struct variable *v);
extern void dhcp_check (webs_t wp, char *value, struct variable *v);
extern int delete_leases (webs_t wp);


/* for log */
extern void ej_dumplog (int eid, webs_t wp, int argc, char_t ** argv);
#ifdef HAVE_SPUTNIK_APD
extern void ej_sputnik_apd_status (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_sputnik (int eid, webs_t wp, int argc, char_t ** argv);
#endif
extern int log_onload (webs_t wp);
extern int filtersummary_onload (webs_t wp, char *arg);


/* for upgrade */
extern void do_upgrade_post (char *url, webs_t stream, int len, char *boundary);
extern void do_upgrade_cgi (char *url, webs_t stream);
extern int sys_restore (char *url, webs_t stream, int *total);
extern void do_restore_post (char *url, webs_t stream, int len, char *boundary);
extern void do_restore_cgi (char *url, webs_t stream);
extern int macclone_onload (webs_t wp, char *arg);


/* for filter */
extern void ej_filter_init (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_filter_summary_show (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_filter_ip_get (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_filter_port_get (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_filter_dport_get (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_filter_mac_get (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_filter_policy_select (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_filter_policy_get (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_filter_tod_get (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_filter_web_get (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_filter_port_services_get (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_filter_policy (webs_t wp, char *value, struct variable *v);
extern void validate_filter_ip_grp (webs_t wp, char *value, struct variable *v);
extern void validate_filter_mac_grp (webs_t wp, char *value, struct variable *v);
extern void validate_filter_dport_grp (webs_t wp, char *value, struct variable *v);
extern void validate_filter_port (webs_t wp, char *value, struct variable *v);
extern void validate_filter_web (webs_t wp, char *value, struct variable *v);
extern void validate_blocked_service (webs_t wp, char *value, struct variable *v);
extern int filter_onload (webs_t wp);
extern int save_policy (webs_t wp);
extern int summary_delete_policy (webs_t wp);
extern int single_delete_policy (webs_t wp);
extern int save_services_port (webs_t wp);


/* for forward */
extern void ej_show_default_level (int eid, webs_t wp, int argc, char_t ** argv);
extern void port_forward_table (webs_t wp, char *type, int which);
extern void port_forward_spec (webs_t wp, char *type, int which);
extern void ej_show_staticleases (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_forward_proto (webs_t wp, char *value, struct variable *v);
extern void validate_staticleases (webs_t wp, char *value, struct variable *v);
extern void validate_forward_spec (webs_t wp, char *value, struct variable *v);
extern void ej_port_trigger_table (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_port_trigger (webs_t wp, char *value, struct variable *v);

// Changed by Steve
//extern int ej_forward_upnp(int eid, webs_t wp, int argc, char_t **argv);			//upnp added 
//extern void validate_forward_upnp(webs_t wp, char *value, struct variable *v);	//upnp added 
// End Changed by Steve


/* for dynamic route */
extern void ej_dump_route_table (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_dynamic_route (webs_t wp, char *value, struct variable *v);
extern int dynamic_route_onload (webs_t wp);


/* for static route */
extern void ej_static_route_setting (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_static_route_table (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_static_route (webs_t wp, char *value, struct variable *v);
extern int delete_static_route (webs_t wp);


/* for wireless */
extern void validate_wl_key (webs_t wp, char *value, struct variable *v);
extern void validate_wl_wep (webs_t wp, char *value, struct variable *v);
extern void validate_wl_auth (webs_t wp, char *value, struct variable *v);
extern void validate_d11_channel (webs_t wp, char *value, struct variable *v);
extern int add_active_mac (webs_t wp);
extern int wl_active_onload (webs_t wp, char *arg);
extern int generate_key_64 (webs_t wp);
extern int generate_key_128 (webs_t wp);
extern void ej_showad (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_wep_value (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_wl_active_mac (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_wl_value (int eid, webs_t wp, int argc, char_t ** argv);
#ifdef HAVE_MSSID
extern int security_save (webs_t wp);
extern void ej_show_wpa_setting (int eid, webs_t wp, int argc, char_t ** argv, char *prefix);
#else
extern void ej_show_wpa_setting (int eid, webs_t wp, int argc, char_t ** argv);
#endif
extern void wl_unit (webs_t wp, char *value, struct variable *v);
extern void validate_wpa_psk (webs_t wp, char *value, struct variable *v);
extern void validate_auth_mode (webs_t wp, char *value, struct variable *v);
extern void validate_security_mode (webs_t wp, char *value, struct variable *v);
extern void ej_wl_ioctl (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_wl_gmode (webs_t wp, char *value, struct variable *v);
extern void validate_wl_net_mode (webs_t wp, char *value, struct variable *v);


/* for nvram save-restore */
extern void nv_file_in (char *url, webs_t stream, int len, char *boundary);
extern void nv_file_out (char *path, webs_t wp);
extern void sr_config_cgi (char *path, webs_t wp);


/* for ddns */
extern int ddns_save_value (webs_t wp);
extern int ddns_update_value (webs_t wp);
extern void ej_show_ddns_status (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_ddns_ip (int eid, webs_t wp, int argc, char_t ** argv);


extern void validate_macmode (webs_t wp, char *value, struct variable *v);
extern void validate_wl_hwaddrs (webs_t wp, char *value, struct variable *v);
extern void ej_wireless_active_table (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_wireless_filter_table (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_wl_wep_setting (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_wl_wep_key (webs_t wp, char *value, struct variable *v);

extern void ej_wl_packet_get (int eid, webs_t wp, int argc, char_t ** argv);


/* Ping and Traceroute */
extern void ej_dump_ping_log (int eid, webs_t wp, int argc, char_t ** argv);
//extern void ej_dump_traceroute_log (int eid, webs_t wp, int argc, char_t ** argv);
extern int diag_ping_start (webs_t wp);
extern int diag_ping_stop (webs_t wp);
extern int diag_ping_clear (webs_t wp);
//extern int diag_traceroute_start (webs_t wp);
//extern int diag_traceroute_stop (webs_t wp);
//extern int diag_traceroute_clear (webs_t wp);
extern int ping_onload (webs_t wp, char *arg);
//extern int traceroute_onload (webs_t wp, char *arg);

/* Added by Botho 21.April.06 */
extern void ej_js_include (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_css_include (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_logout(void);
extern void ej_statfs (int eid, webs_t wp, int argc, char_t ** argv);

/* for all */
extern void ej_onload (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_web_page_name (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_compile_date (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_compile_time (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_model_name (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_firmware_version (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_firmware_title (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_firmware_svnrev (int eid, webs_t wp, int argc, char_t ** argv);
extern char *live_translate (char *tran);  //Eko
extern void rep (char *in, char from, char to);


/* Sveasoft additions */
extern void save_wds (webs_t wp);
extern void validate_iradius (webs_t wp, char *value, struct variable *v);
extern void validate_wds (webs_t wp, char *value, struct variable *v);
extern void validate_statics (webs_t wp, char *value, struct variable *v);
extern void ej_get_wdsp2p (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_active_wireless (int eid, webs_t wp, int argc, char_t ** argv);
#ifdef HAVE_SKYTRON
extern void ej_active_wireless2 (int eid, webs_t wp, int argc, char_t ** argv);
#endif
extern void ej_show_clocks (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_active_wds (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_iradius_check (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_iradius (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_wds_mac (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_wds_ip (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_wds_netmask (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_wds_gw (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_br1_ip (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_br1_netmask (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_curchannel (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_currate (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_uptime (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_qossvcs (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_qosips (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_qosmacs (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_qossvcs2 (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_qosips2 (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_qosmacs2 (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_services_options (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_infopage (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_control (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_clone_wmac (int eid, webs_t wp, int argc, char_t ** argv);
extern int raduser_add (webs_t wp);
extern int qos_add_svc (webs_t wp);
extern int qos_add_ip (webs_t wp);
extern int qos_add_mac (webs_t wp);
extern int qos_save (webs_t wp);
extern int ping_wol (webs_t wp);
extern int ping_startup (webs_t wp);
extern int ping_firewall (webs_t wp);
/* end Sveasoft additions */


#ifdef HAVE_MSSID
extern int add_vifs (webs_t wp);
extern int remove_vifs (webs_t wp);
#endif

//extern int ej_show_virtualssid(int eid, webs_t wp, int argc, char_t **argv);
extern void ej_show_security (int eid, webs_t wp, int argc, char_t ** argv);

extern int reg_validate (webs_t wp);
extern int wireless_save (webs_t wp);
extern int set_security (webs_t wp);
extern int forward_add (webs_t wp);
extern int forward_remove (webs_t wp);
extern int lease_add (webs_t wp);
extern int lease_remove (webs_t wp);
#ifdef HAVE_CHILLILOCAL
extern int user_add (webs_t wp);
extern int user_remove (webs_t wp);
extern void ej_show_userlist (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_userlist (webs_t wp, char *value, struct variable *v);
#endif
extern int forwardspec_add (webs_t wp);
extern int forwardspec_remove (webs_t wp);
extern int trigger_add (webs_t wp);
extern int trigger_remove (webs_t wp);


/* lonewolf additions */
extern void ej_port_vlan_table (int eid, webs_t wp, int argc, char_t ** argv);
extern int port_vlan_table_save (webs_t wp);
extern void ej_if_config_table (int eid, webs_t wp, int argc, char_t ** argv);
extern int if_config_table_save (webs_t wp);
extern int alias_delete_ip (webs_t wp);
/* end lonewolf additions */

extern int valid_ipaddr (webs_t wp, char *value, struct variable *v);
extern int valid_range (webs_t wp, char *value, struct variable *v);
extern int valid_hwaddr (webs_t wp, char *value, struct variable *v);
extern int valid_choice (webs_t wp, char *value, struct variable *v);
extern int valid_netmask (webs_t wp, char *value, struct variable *v);
extern int valid_name (webs_t wp, char *value, struct variable *v);
extern int valid_merge_ipaddrs (webs_t wp, char *value, struct variable *v);
extern int valid_wep_key (webs_t wp, char *value, struct variable *v);

extern int get_dns_ip (char *name, int which, int count);
extern int get_single_ip (char *ipaddr, int which);
extern int get_merge_ipaddr (webs_t wp,char *name, char *ipaddr);
extern int get_merge_mac (webs_t wp,char *name, char *macaddr);
extern char *rfctime (const time_t * timep);
extern int legal_ipaddr (char *value);
extern int legal_hwaddr (char *value);
extern int legal_netmask (char *value);
extern int legal_ip_netmask (char *sip, char *smask, char *dip);
extern int find_pattern (const char *data, size_t dlen,
			 const char *pattern, size_t plen,
			 char term,
			 unsigned int *numoff, unsigned int *numlen);

extern int find_match_pattern (char *name, size_t mlen,
			       const char *data,
			       const char *pattern, char *def);

extern int find_each (char *name, int len,
		      char *data, char *token, int which, char *def);

/* 
 * set type to 1 to replace ' ' with "&nbsp;" and ':' with "&semi;"
 * set type to 2 to replace "&nbsp;" with ' ' and "&semi;" with ':'
 */
extern int httpd_filter_name (char *old_name, char *new_name, size_t size,
			      int type);

/* check the value for a digit (0 through 9) 
 * set flag to 0 to ignore zero-length values
 */
extern int ISDIGIT (char *value, int flag);

/* checks  whether  value  is  a  7-bit unsigned char value that fits into the ASCII character set 
 * set flag to 0 to ignore zero-length values
 */
extern int ISASCII (char *value, int flag);

extern void do_setup_wizard (char *url, webs_t stream);
extern void ej_show_turbo (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_adwifi (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_wireless (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_logo (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_sysinfo (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_miscinfo (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_backup_name (int eid, webs_t wp, int argc, char_t ** argv);
extern struct servent *my_getservbyport (int port, const char *proto);
extern int get_single_mac (char *macaddr, int which);

extern int StopContinueTx (webs_t wp,char *value);
extern int StartContinueTx (webs_t wp,char *value);
extern int Check_TSSI (webs_t wp,char *value);
extern int Get_TSSI (char *value);
extern int Enable_TSSI (char *value);

extern void LOG (int level, const char *fmt, ...);

extern char *num_to_protocol (int num);
extern int protocol_to_num (char *proto);
extern void do_backup (char *path, webs_t stream);

extern void ej_per_port_option (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_port_qos (webs_t wp, char *value, struct variable *v);
extern void ej_qos_sw_default (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_show_meminfo (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_dump_site_survey (int eid, webs_t wp, int argc, char_t ** argv);
extern void ej_get_url (int eid, webs_t wp, int argc, char_t ** argv);
#ifdef FBNFW
extern void ej_list_fbn (int eid, webs_t wp, int argc, char_t ** argv);
#endif
extern void ej_wme_match_op (int eid, webs_t wp, int argc, char_t ** argv);
extern void validate_noack (webs_t wp, char *value, struct variable *v);
extern void validate_wl_wme_params (webs_t wp, char *value, struct variable *v);

extern void validate_choice (webs_t wp, char *value, struct variable *v);
#define MAX_WDS_DEVS 10

int ishexit (char c);
int sv_valid_hwaddr (char *value);
int sv_valid_ipaddr (char *value);
int sv_valid_range (char *value, int low, int high);
int sv_valid_statics (char *value);
void get_network (char *ipaddr, char *netmask);
void get_broadcast (char *ipaddr, char *netmask);
int route_manip (int cmd, char *name, int metric, char *dst, char *gateway, char *genmask);
int route_add (char *name, int metric, char *dst, char *gateway, char *genmask);
int route_del (char *name, int metric, char *dst, char *gateway, char *genmask);
