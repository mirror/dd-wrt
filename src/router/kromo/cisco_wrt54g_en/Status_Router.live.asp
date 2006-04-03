{router_name::<% nvram_get("router_name"); %>}
{router_model::<% nvram_get("DD_BOARD"); %>}
{router_firmware::<% get_firmware_version(); %>}
{router_time::<% localtime(); %>}
{cpu_info::<% show_cpuinfo(); %>}
{cpu_clock::<% get_clkfreq(); %> MHz}
{wan_shortproto::<% nvram_get("wan_proto"); %>}
{wan_proto::<% nvram_match("wan_proto", "dhcp", "Automatic Configuration - DHCP"); %><% nvram_match("wan_proto", "static", "Static"); %><% nvram_match("wan_proto", "pppoe", "PPPoE"); %><% nvram_match("wan_proto", "pptp", "PPTP"); %><% nvram_match("wan_proto", "l2tp", "L2TP"); %><% nvram_match("wan_proto", "heartbeat", "HeartBeatSignal"); %><% nvram_match("wan_proto", "disabled", "Disabled"); %>}
{wan_status::<% nvram_status_get("status2"); %>&nbsp;&nbsp;<input type="button" value="<% nvram_status_get("button1"); %>" onclick="connect(this.form, '<% nvram_status_get("button1"); %>_<% nvram_get("wan_proto"); %>')" />}
{wan_mac::<% nvram_get("wan_hwaddr"); %>}
{wan_ipaddr::<% nvram_status_get("wan_ipaddr"); %>}
{wan_host::<% nvram_get("wan_hostname"); %>}
{wan_name::<% nvram_get("wan_domain"); %>}
{wan_netmask::<% nvram_status_get("wan_netmask"); %>}
{wan_gateway::<% nvram_status_get("wan_gateway"); %>}
{wan_dns0::<% nvram_status_get("wan_dns0"); %>}
{wan_dns1::<% nvram_status_get("wan_dns1"); %>}
{wan_dns2::<% nvram_status_get("wan_dns2"); %>}
{mem_info::<% dumpmeminfo(); %>}
{uptime::<% get_uptime(); %>}
{ip_conntrack::<% dumpip_conntrack(); %>}