{lan_mac::<% nvg("lan_hwaddr"); %>}
{lan_ip::<% nvg("lan_ipaddr"); %>}
{lan_ip_prefix::<% prefix_ip_get("lan_ipaddr",1); %>}
{lan_netmask::<% nvg("lan_netmask"); %>}
{lan_gateway::<% nvg("lan_gateway"); %>}
{lan_dns::<% nvg("sv_localdns"); %>}
{lan_proto::<% dhcpenabled("dhcp","static"); %>}
{dhcp_daemon::<% nvem("dhcp_dnsmasq", "1", "DNSMasq", "uDHCPd"); %>}
{dhcp_start::<% nvg("dhcp_start"); %>}
{dhcp_num::<% nvg("dhcp_num"); %>}
{dhcp_lease_time::<% nvg("dhcp_lease"); %>}
{dhcp_leases::<% dumpleases(0); %>}
{pptp_leases::<% dumppptp(); %>}
{pppoe_leases::<% dumppppoe(); %>}
{arp_table::<% dumparptable(0); %>}
{uptime::<% get_uptime(); %>}
{ipinfo::<% show_wanipinfo(); %>}
