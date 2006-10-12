{lan_mac::<% nvram_get("lan_hwaddr"); %>}
{lan_ip::<% nvram_get("lan_ipaddr"); %>}
{lan_ip_prefix::<% prefix_ip_get("lan_ipaddr",1); %>}
{lan_netmask::<% nvram_get("lan_netmask"); %>}
{lan_gateway::<% nvram_get("lan_gateway"); %>}
{lan_dns::<% nvram_get("sv_localdns"); %>}
{lan_proto::<% nvram_get("lan_proto"); %>}
{dhcp_daemon::<% nvram_else_match("dhcp_dnsmasq", "1", "DNSMasq", "uDHCPd"); %>}
{dhcp_start::<% nvram_get("dhcp_start"); %>}
{dhcp_num::<% nvram_get("dhcp_num"); %>}
{dhcp_lease_time::<% nvram_get("dhcp_lease"); %>}
{dhcp_leases::<% dumpleases(0); %>}
{arp_table::<% dumparptable(0); %>}
