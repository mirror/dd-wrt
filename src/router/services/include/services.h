/*
 * services.h
 *
 * Copyright (C) 2008 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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

#ifndef SERVICES_H
#define SERVICES_H
#define DECLARE_SERVICE(name)         \
	extern void start_##name();   \
	extern void stop_##name();    \
	extern void restart_##name(); \
	extern char *name##_deps();   \
	extern char *name##_proc();

DECLARE_SERVICE(backup);
DECLARE_SERVICE(bonding);
DECLARE_SERVICE(bootconfig);
DECLARE_SERVICE(bootconfig_efi);
DECLARE_SERVICE(bootconfig_legacy);
DECLARE_SERVICE(bridgesif);
DECLARE_SERVICE(bridging);
DECLARE_SERVICE(check_qmi);
DECLARE_SERVICE(check_sierradirectip);
DECLARE_SERVICE(check_sierrappp);
DECLARE_SERVICE(checkhostapd);
DECLARE_SERVICE(chilli);
DECLARE_SERVICE(config_vlan);
DECLARE_SERVICE(configurewifi);
DECLARE_SERVICE(conntrack);
DECLARE_SERVICE(crashtest);
DECLARE_SERVICE(cron);
DECLARE_SERVICE(ddns);
DECLARE_SERVICE(deconfigurewifi);
DECLARE_SERVICE(defaults);
DECLARE_SERVICE(detectdrivers);
DECLARE_SERVICE(devinit);
DECLARE_SERVICE(devinit_arch);
DECLARE_SERVICE(dhcp6c);
DECLARE_SERVICE(dhcp6s);
DECLARE_SERVICE(dhcpc);
DECLARE_SERVICE(dhcpfwd);
DECLARE_SERVICE(dlna);
DECLARE_SERVICE(dlna_rescan);
DECLARE_SERVICE(dns_clear_resolv);
DECLARE_SERVICE(dnsmasq);
DECLARE_SERVICE(drivers);
DECLARE_SERVICE(drivers_net);
DECLARE_SERVICE(duallink);
DECLARE_SERVICE(dumprom);
DECLARE_SERVICE(finishupgrade);
DECLARE_SERVICE(firewall);
DECLARE_SERVICE(force_to_dial);
DECLARE_SERVICE(freeradius);
DECLARE_SERVICE(ftpsrv);
DECLARE_SERVICE(gen_radius_cert);
DECLARE_SERVICE(heartbeat_boot);
DECLARE_SERVICE(heartbeat_redial);
DECLARE_SERVICE(hostapdwan);
DECLARE_SERVICE(hostname);
DECLARE_SERVICE(hotplug_block);
DECLARE_SERVICE(hotplug_net);
DECLARE_SERVICE(hotplug_usb);
DECLARE_SERVICE(httpd);
DECLARE_SERVICE(hwmon);
DECLARE_SERVICE(igmprt);
DECLARE_SERVICE(init_restart);
DECLARE_SERVICE(init_start);
DECLARE_SERVICE(init_stop);
DECLARE_SERVICE(initvlans);
DECLARE_SERVICE(iptqueue);
DECLARE_SERVICE(ipv6);
DECLARE_SERVICE(ipvs);
DECLARE_SERVICE(jffs2);
DECLARE_SERVICE(l2tp_boot);
DECLARE_SERVICE(l2tp_redial);
DECLARE_SERVICE(l2tp);
DECLARE_SERVICE(lan);
DECLARE_SERVICE(lighttpd);
DECLARE_SERVICE(lltd);
DECLARE_SERVICE(loadfwmodules);
DECLARE_SERVICE(mactelnetd);
DECLARE_SERVICE(mdns);
DECLARE_SERVICE(mkfiles);
DECLARE_SERVICE(mmc);
DECLARE_SERVICE(modules);
DECLARE_SERVICE(modules_wait);
DECLARE_SERVICE(nfs);
DECLARE_SERVICE(notifier);
DECLARE_SERVICE(nstxd);
DECLARE_SERVICE(ntpc);
DECLARE_SERVICE(nvram);
DECLARE_SERVICE(olsrd);
DECLARE_SERVICE(openvpn);
DECLARE_SERVICE(openvpnserver);
DECLARE_SERVICE(openvpnserversys);
DECLARE_SERVICE(openvpnserverwan);
DECLARE_SERVICE(overclocking);
DECLARE_SERVICE(plex);
DECLARE_SERVICE(postnetwork);
DECLARE_SERVICE(post_sysinit);
DECLARE_SERVICE(pppmodules);
DECLARE_SERVICE(pppoerelay);
DECLARE_SERVICE(pppoeserver);
DECLARE_SERVICE(pppoe_dual);
DECLARE_SERVICE(pppoe);
DECLARE_SERVICE(pptp);
DECLARE_SERVICE(pptpd);
DECLARE_SERVICE(printer);
DECLARE_SERVICE(privoxy);
DECLARE_SERVICE(process_monitor);
DECLARE_SERVICE(qos);
DECLARE_SERVICE(quagga_writememory);
DECLARE_SERVICE(radio_off);
DECLARE_SERVICE(radio_off_0);
DECLARE_SERVICE(radio_off_1);
DECLARE_SERVICE(radio_off_2);
DECLARE_SERVICE(radio_on);
DECLARE_SERVICE(radio_on_0);
DECLARE_SERVICE(radio_on_1);
DECLARE_SERVICE(radio_on_2);
DECLARE_SERVICE(radio_timer);
DECLARE_SERVICE(radvd);
DECLARE_SERVICE(raid);
DECLARE_SERVICE(recover);
DECLARE_SERVICE(redial);
DECLARE_SERVICE(resetbc);
DECLARE_SERVICE(resetbutton);
DECLARE_SERVICE(restarthostapd);
DECLARE_SERVICE(restarthostapd_ifneeded);
DECLARE_SERVICE(restore_defaults);
DECLARE_SERVICE(rsync);
DECLARE_SERVICE(run_rc_shutdown);
DECLARE_SERVICE(run_rc_startup);
DECLARE_SERVICE(run_rc_usb);
DECLARE_SERVICE(samba3);
DECLARE_SERVICE(ses_led_control);
DECLARE_SERVICE(set_routes);
DECLARE_SERVICE(setup_affinity);
DECLARE_SERVICE(setup_vlans);
DECLARE_SERVICE(sfe);
DECLARE_SERVICE(smartd);
DECLARE_SERVICE(snmp);
DECLARE_SERVICE(softether);
DECLARE_SERVICE(speedchecker);
DECLARE_SERVICE(speedchecker_init);
DECLARE_SERVICE(splashd);
DECLARE_SERVICE(sshd);
DECLARE_SERVICE(stabridge);
DECLARE_SERVICE(sysctl_cleanup);
DECLARE_SERVICE(sysctl_config);
DECLARE_SERVICE(sysinit);
DECLARE_SERVICE(syslog);
DECLARE_SERVICE(sysshutdown);
DECLARE_SERVICE(telnetd);
DECLARE_SERVICE(test_ndpi);
DECLARE_SERVICE(tor);
DECLARE_SERVICE(transmission);
DECLARE_SERVICE(ttraff);
DECLARE_SERVICE(ubus);
DECLARE_SERVICE(udpxy);
DECLARE_SERVICE(upnpd);
DECLARE_SERVICE(usteer);
DECLARE_SERVICE(vifs);
DECLARE_SERVICE(vlantagging);
DECLARE_SERVICE(vncrepeater);
DECLARE_SERVICE(vpn_modules);
DECLARE_SERVICE(wan);
DECLARE_SERVICE(wan_boot);
DECLARE_SERVICE(wan_redial);
DECLARE_SERVICE(wan_service);
DECLARE_SERVICE(wanup);
DECLARE_SERVICE(watchdog);
DECLARE_SERVICE(wifidog);
DECLARE_SERVICE(wland);
DECLARE_SERVICE(wlconf);
DECLARE_SERVICE(wol);
DECLARE_SERVICE(zabbix);
DECLARE_SERVICE(zebra);
DECLARE_SERVICE(wifi_drivers);

extern void run_pptp(int status);

#ifdef HAVE_UNBOUND
DECLARE_SERVICE(unbound);
#else
static inline void start_unbound(void)
{
}

static inline void stop_unbound(void)
{
}
#endif

extern void run_pptp(int status);
extern void run_l2tp(int status);
#ifdef HAVE_SMARTDNS
DECLARE_SERVICE(smartdns);
#else
static inline void start_smartdns(void)
{
}

static inline void stop_smartdns(void)
{
}
#endif
extern void run_wan(int status);
extern void wan_done(char *ifname);
void run_pppoe(int pppoe_num);
void run_pppoe_dual(int status);
void stop_single_pppoe(int pppoe_num);
char *enable_dtag_vlan(int status);
int mk_nodog_conf(void);

#define IDX_IFNAME 0
#define IDX_DHCPON 1
#define IDX_LEASESTART 2
#define IDX_LEASEMAX 3
#define IDX_LEASETIME 4

char *getmdhcp(int count, int index, char *buffer);

void stop_openvpn_wandone(void);

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

int wlconf_up(char *name);

void runStartup(char *extension);
char *set_wan_state(int state);
void create_openvpnserverrules(FILE *fp);
void create_openvpnrules(FILE *fp);
void load_drivers(int boot);

#endif
