extern void start_httpd( void );
extern void stop_httpd( void );

#ifdef HAVE_TELNET
extern void start_telnetd( void );
extern void stop_telnetd( void );
#endif

extern void start_resetbutton( void );
extern void stop_resetbutton( void );

extern void start_tftpd( void );
extern void stop_tftpd( void );

extern void start_cron( void );
extern void stop_cron( void );

extern void start_zebra( void );
extern void stop_zebra( void );

extern void start_redial( void );
extern void stop_redial( void );

extern void start_ddns( void );
extern void stop_ddns( void );

#ifdef HAVE_UPNP
extern void start_upnp( void );
extern void stop_upnp( void );
#endif

extern void start_pptp( int status );
extern void stop_pptp( void );

extern void start_syslog( void );
extern void stop_syslog( void );

extern void start_process_monitor( void );
extern void stop_process_monitor( void );

extern void stop_dhcpc(  );

extern void stop_pppoe( void );

void start_custom_script( void );

extern void start_ipv6( void );

#ifdef HAVE_SSHD
extern void start_sshd( void );
extern void stop_sshd( void );
#endif

#ifdef HAVE_RADVD
extern void start_radvd( void );
extern void stop_radvd( void );
#endif
#ifdef HAVE_PPTPD
extern void start_pptpd( void );
extern void stop_pptpd( void );
#endif
#ifdef HAVE_MMC
extern void start_mmc( void );
#endif

#ifdef HAVE_SNMP
extern void start_snmp( void );
extern void stop_snmp( void );
#endif

#ifdef HAVE_WOL
extern void start_wol( void );
extern void stop_wol( void );
#endif

extern void start_shorewall( void );
extern void stop_shorewall( void );

extern void start_wshaper( void );
extern void stop_wshaper( void );

extern void start_wland( void );
extern void stop_wland( void );
extern void start_firewall( void );
extern void stop_firewall( void );

#ifdef HAVE_MULTICAST
extern void start_igmp_proxy( void );
extern void stop_igmp_proxy( void );
#endif
#ifdef HAVE_SPUTNIK_APD
extern void start_sputnik( void );
extern void stop_sputnik( void );
extern void start_sputnik_apd( void );
extern void stop_sputnik_apd( void );
#endif

extern void start_nas( void );
extern void stop_nas( void );
#ifdef HAVE_CHILLI
extern void start_chilli( void );
extern void stop_chilli( void );
#endif
extern void stop_l2tp( void );
extern void start_l2tp_boot( void );
extern void start_udhcpd( void );
extern void stop_udhcpd( void );
extern void start_dnsmasq( void );
extern void stop_dnsmasq( void );
extern void start_dhcpfwd( void );
extern void stop_dhcpfwd( void );
extern void start_ntpc( void );
extern void stop_ntpc( void );
extern void start_lan( void );
extern void stop_lan( void );
extern void start_wan( int status );
extern void start_wan_boot(void);
extern void start_wan_redial(void);
extern void start_wan_done( char *ifname );
extern void stop_wan( void );
extern void start_hotplug_net( void );
#ifdef HAVE_NOCAT
extern void start_splashd( void );
extern void stop_splashd( void );
#endif

#ifdef HAVE_MILKFISH
extern void start_milkfish( void );
extern void stop_milkfish( void );
#endif

extern void start_mkfiles(void);

int write_nvram( char *name, char *nv );
void start_pppoe( int pppoe_num );
void stop_single_pppoe( int pppoe_num );

char *enable_dtag_vlan(int status);
void start_config_vlan(void);
void start_setup_vlans(void);
void start_set_routes(void);
int mk_nocat_conf (void);

void start_wifidog(void);
void stop_wifidog(void);
void start_openvpn(void);
void stop_openvpn(void);
void start_anchorfree(void);
void start_anchorfreednat(void);
void start_openvpnserverwan(void);
void stop_openvpnserverwan(void);

void start_wanup( void );

void stop_anchorfree(void);
