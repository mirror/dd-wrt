{router_time::<% localtime(); %>}
{wan_shortproto::<% nvram_get("wan_proto"); %>}
{wan_status::<% nvram_status_get("status2"); %>&nbsp;&nbsp;<input type="button" value="<% nvram_status_get("button1"); %>" onclick="connect(this.form, '<% nvram_status_get("button1"); %>_<% nvram_get("wan_proto"); %>')" />}
{wan_mac::<% nvram_get("wan_hwaddr"); %>}
{wan_ipaddr::<% nvram_status_get("wan_ipaddr"); %>}
{wan_netmask::<% nvram_status_get("wan_netmask"); %>}
{wan_gateway::<% nvram_status_get("wan_gateway"); %>}
{wan_dns0::<% nvram_status_get("wan_dns0"); %>}
{wan_dns1::<% nvram_status_get("wan_dns1"); %>}
{wan_dns2::<% nvram_status_get("wan_dns2"); %>}
{dhcp_remaining::<% dhcp_remaining_time(); %>}
{mem_info::<% dumpmeminfo(); %>}
{uptime::<% get_uptime(); %>}
{ip_conntrack::<% dumpip_conntrack(); %>}
{cpu_temp::<% get_cputemp(); %>}
{voltage::<% get_voltage(); %>}