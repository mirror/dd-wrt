{lan_mac::<% nvg("lan_hwaddr"); %>}
{lan_ip::<% get_lan_ipaddr(); %>/<% get_lan_cidr_mask(); %>}
{lan_gateway::<% get_lan_gateway(); %>}
{lan_dns::<% get_lan_dns(); %>}
{lan_proto::<% dhcpenabled("dhcp","static"); %>}
{dhcp_start::<% nvg("dhcp_start"); %>}
{dhcp_end::<% calcendip(); %>}
{dhcp_num::<% nvg("dhcp_num"); %>}
{dhcp_lease_time::<% nvg("dhcp_lease"); %>}
{dhcp_leases::<% dumpleases(0); %>}
{pptp_leases::<% dumppptp(); %>}
{pppoe_leases::<% dumppppoe(); %>}
{arp_table::<% dumparptable(0); %>}
{uptime::<% get_uptime(); %>}
{ipinfo::<% show_wanipinfo(); %>}
