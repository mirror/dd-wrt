#ifndef SERVICES_H
#define SERVICES_H

extern void start_httpd(void);
extern void stop_httpd(void);

#ifdef HAVE_CHRONY
extern void stop_chronyd(void);
extern void start_chronyd(void);
#endif

#ifdef HAVE_ANTAIRA_AGENT
extern void start_antaira_agent(void);
extern void stop_antaira_agent(void);
#endif

#ifdef HAVE_TELNET
extern void start_telnetd(void);
extern void stop_telnetd(void);
#endif
#ifdef HAVE_MACTELNET
extern void start_mactelnetd(void);
extern void stop_mactelnetd(void);
#endif

extern void start_resetbutton(void);
extern void stop_resetbutton(void);

extern char *vpn_modules_deps(void);
extern void start_vpn_modules(void);
extern void stop_vpn_modules(void);

extern void start_tftpd(void);
extern void stop_tftpd(void);

extern void start_usteer(void);
extern void stop_usteer(void);

extern void load_drivers(int boot);
extern void start_drivers_net(void);

extern void start_cron(void);
extern void stop_cron(void);

extern void start_pppmodules(void);
extern void stop_pppmodules(void);

extern void start_zebra(void);
extern void stop_zebra(void);

extern void start_redial(void);
extern void stop_redial(void);

extern void start_ddns(void);
extern void stop_ddns(void);

#ifdef HAVE_UPNP
extern void start_upnp(void);
extern void stop_upnp(void);
#endif

#ifdef HAVE_IPVS
extern void start_ipvs(void);
extern void stop_ipvs(void);
#endif

#ifdef HAVE_QTN
extern void start_qtn(void);
extern void stop_qtn(void);
#endif

extern void run_pptp(int status);
extern void stop_pptp(void);

extern void start_syslog(void);
extern void stop_syslog(void);

#ifdef HAVE_TOR
extern void start_tor(void);
extern void stop_tor(void);
#endif

extern void start_process_monitor(void);
extern void stop_process_monitor(void);

extern void stop_dhcpc();

extern void stop_pppoe(void);

void start_custom_script(void);

extern void start_ipv6(void);

#ifdef HAVE_SSHD
extern void start_sshd(void);
extern void stop_sshd(void);
#endif

#ifdef HAVE_RADVD
extern void start_radvd(void);
extern void stop_radvd(void);
#endif
#ifdef HAVE_PPTPD
extern void start_pptpd(void);
extern void stop_pptpd(void);
#endif
#ifdef HAVE_PPPOEDUAL
extern void run_pppoe_dual(int status);
extern void stop_pppoe_dual();
#endif
#ifdef HAVE_MMC
extern void start_mmc(void);
#endif

#ifdef HAVE_SNMP
extern void start_snmp(void);
extern void stop_snmp(void);
#endif

#ifdef HAVE_WOL
extern void start_wol(void);
extern void stop_wol(void);
#endif

extern void start_shorewall(void);
extern void stop_shorewall(void);

extern void start_wshaper(void);
extern void stop_wshaper(void);

extern void start_wland(void);
extern void stop_wland(void);
extern void start_firewall(void);
extern void stop_firewall(void);
extern void start_qos(void);
extern void stop_qos(void);

#ifdef HAVE_MULTICAST
extern void start_igmprt(void);
extern void stop_igmprt(void);
#endif

#ifdef HAVE_UNBOUND
extern void start_unbound(void);
extern void stop_unbound(void);
#else
static inline void start_unbound(void)
{
}

static inline void stop_unbound(void)
{
}
#endif

#ifdef HAVE_UDPXY
extern void start_udpxy(void);
extern void stop_udpxy(void);
#endif

#ifdef HAVE_SPUTNIK_APD
extern void start_sputnik(void);
extern void stop_sputnik(void);
extern void start_sputnik_apd(void);
extern void stop_sputnik_apd(void);
#endif

extern void start_nas(void);
extern void stop_nas(void);
#ifdef HAVE_CHILLI
extern void start_chilli(void);
extern void stop_chilli(void);
#endif
extern void run_pptp(int status);
extern void run_l2tp(int status);
extern void stop_l2tp(void);
extern void start_l2tp_boot(void);
extern void start_udhcpd(void);
extern void stop_udhcpd(void);
extern void start_dnsmasq(void);
extern void restart_dnsmasq(void);
extern void stop_dnsmasq(void);
#ifdef HAVE_SMARTDNS
extern void start_smartdns(void);
extern void stop_smartdns(void);
#else
static inline void start_smartdns(void)
{
}

static inline void stop_smartdns(void)
{
}
#endif
extern void start_dhcpfwd(void);
extern void stop_dhcpfwd(void);
extern void start_ntpc(void);
extern void stop_ntpc(void);
extern void start_lan(void);
extern void stop_lan(void);
extern void run_wan(int status);
extern void start_wan_boot(void);
extern void start_wan_redial(void);
extern void wan_done(char *ifname);
extern void stop_wan(void);
extern void start_hotplug_net(void);
#ifdef HAVE_NODOG
extern void start_splashd(void);
extern void stop_splashd(void);
#endif

#ifdef HAVE_MILKFISH
extern void start_milkfish(void);
extern void stop_milkfish(void);
#endif

extern void start_mkfiles(void);
void run_pppoe(int pppoe_num);
void stop_single_pppoe(int pppoe_num);

char *enable_dtag_vlan(int status);
void start_config_vlan(void);
void start_setup_vlans(void);
void start_set_routes(void);
int mk_nodog_conf(void);

#define IDX_IFNAME 0
#define IDX_DHCPON 1
#define IDX_LEASESTART 2
#define IDX_LEASEMAX 3
#define IDX_LEASETIME 4

char *getmdhcp(int count, int index, char *buffer);

void start_wifidog(void);
void stop_wifidog(void);
void start_freeradius(void);
void stop_freeradius(void);
void start_openvpn(void);
void stop_openvpn(void);
void stop_openvpn_wandone(void);
//void start_anchorfree(void);
//void start_anchorfreednat(void);
void start_openvpnserverwan(void);
void stop_openvpnserverwan(void);
void start_openvpnserver(void);
void stop_openvpnserver(void);

void start_atm(void);
void stop_atm(void);

void start_wanup(void);

//void stop_anchorfree(void);

char *getMTU(char *ifname);
char *getTXQ(char *ifname);

int br_add_bridge(const char *brname);
int br_del_bridge(const char *brname);
int br_add_interface(const char *br, const char *dev);
int br_del_interface(const char *br, const char *dev);
int br_set_stp_state(const char *br, int stp_state);
int br_set_port_prio(const char *br, char *port, int prio);
int br_set_port_hairpin(const char *br, char *port, int val);
int br_set_bridge_forward_delay(const char *br, int sec);
int br_set_bridge_prio(const char *br, int prio);
int br_set_port_stp(const char *br, char *port, int on);
int br_set_path_cost(const char *bridge, const char *port, int cost);

void reset_hwaddr(char *ifname);
void start_force_to_dial(void);

int stop_process(char *name, char *desc);
int reload_process(char *name);
int stop_process_timeout(char *name, char *desc, int timeout);
int stop_process_hard(char *name, char *desc);

void network_delay(char *service);

char *getMacAddr(char *ifname, char *mac, size_t len);

#ifdef HAVE_ATH9K
void deconfigure_single_ath9k(int count);
void configure_single_ath9k(int count);
void ath9k_start_supplicant(int count, char *prefix);
#endif
int ifconfig(char *name, int flags, char *addr, char *netmask);

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

#ifdef HAVE_NINTENDO
void start_spotpass(void);
void stop_spotpass(void);
void start_spotpass_defaults(void);
#endif

int wlconf_up(char *name);
#ifdef HAVE_ClOUD4WI
void start_cloud4wi_provisioning(void);
#endif

void start_modules(void);
void start_radio_on(void);
void start_radio_off(void);

void start_radio_timer(void);
void stop_radio_timer(void);
void start_milkfish_boot(void);

void start_bridging(void);
void stop_bridging(void);
void start_bridgesif(void);
void stop_bridgesif(void);
void start_vlantagging(void);
void stop_vlantagging(void);
void start_stabridge(void);
void stop_stabridge(void);
void start_bonding(void);
void stop_bonding(void);
void start_ttraff(void);
void stop_ttraff(void);
void start_run_rc_shutdown(void);
void start_run_rc_startup(void);
void stop_run_rc_startup(void);
void start_run_rc_usb(void);
void stop_run_rc_usb(void);
void start_bonding(void);
void start_overclocking(void);
void stop_mkfiles(void);
void stop_ipv6(void);

void start_wlconf(void);
void stop_wlconf(void);
void start_emf(void);
void stop_emf(void);
void start_sfe(void);
void stop_sfe(void);
void start_notifier(void);
void stop_notifier(void);

void start_nvram(void);
void start_restore_defaults(void);
void start_conntrack(void);
void start_jffs2(void);
void start_speedchecker(void);
void stop_speedchecker(void);
void start_speedchecker_init(void);

void start_mdns(void);
void stop_mdns(void);

void stop_pppoeserver(void);
void start_pppoeserver(void);

void stop_heartbeat_redial(void);
void start_heartbeat_redial(void);

void start_check_sierradirectip(void);
void start_check_sierrappp(void);
void start_check_mbim(void);
void start_check_qmi(void);

void start_postnetwork(void);

void runStartup(char *extension);
char *set_wan_state(int state);
void create_openvpnserverrules(FILE *fp);
void create_openvpnrules(FILE *fp);
void stop_ubus(void);
void start_ubus(void);
void stop_usteer(void);
void start_usteer(void);
void stop_martd(void);
void start_smartd(void);
void start_hostapdwan(void);
void start_duallink(void);
void start_deconfigurewifi(void);
void start_configurewifi(void);

#endif
